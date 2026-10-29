/* Native PsyQ voice services and SPU lifecycle */
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "game.h"
#include "sound_device.h"
#include "psx_spu.h"

_Static_assert(sizeof(SpuCommonAttr) == 40, "SPU common attribute size");
_Static_assert(offsetof(SpuCommonAttr, mask) == 0, "SPU common mask offset");
_Static_assert(offsetof(SpuCommonAttr, mvol) == 4, "SPU common master volume offset");
_Static_assert(offsetof(SpuCommonAttr, mvolmode) == 8, "SPU common master mode offset");
_Static_assert(offsetof(SpuCommonAttr, mvolx) == 12, "SPU common current master offset");
_Static_assert(offsetof(SpuCommonAttr, cd) == 16, "SPU common CD offset");
_Static_assert(offsetof(SpuCommonAttr, cd.mix) == 24, "SPU common CD mix offset");
_Static_assert(offsetof(SpuCommonAttr, ext) == 28, "SPU common external offset");
_Static_assert(sizeof(SpuReverbAttr) == 20, "SPU reverb attribute size");
_Static_assert(offsetof(SpuReverbAttr, mask) == 0, "SPU reverb mask offset");
_Static_assert(offsetof(SpuReverbAttr, mode) == 4, "SPU reverb mode offset");
_Static_assert(offsetof(SpuReverbAttr, depth) == 8, "SPU reverb depth offset");
_Static_assert(offsetof(SpuReverbAttr, delay) == 12, "SPU reverb delay offset");
_Static_assert(offsetof(SpuReverbAttr, feedback) == 16, "SPU reverb feedback offset");

static int audio_initialized;
SpuMallocState sound_heap;
SpuMallocRecord sound_heap_recs[SOUND_HEAP_RECORD_COUNT];

static void voice_get(sint32 voice, SpuVoiceAttr *attr)
{
    memset(attr, 0, sizeof(*attr));
    attr->voice = SPU_KEYCH(voice);
    SpuGetVoiceAttr(attr);
}

static void voice_set(sint32 voice, SpuVoiceAttr *attr)
{
    attr->voice = SPU_KEYCH(voice);
    attr->mask = SPU_VOICE_VOLL | SPU_VOICE_VOLR | SPU_VOICE_PITCH | SPU_VOICE_WDSA | SPU_VOICE_LSAX | SPU_VOICE_ADSR_ADSR1 | SPU_VOICE_ADSR_ADSR2;
    SpuSetVoiceAttr(attr);
}

/* 7676C/7678C mode0 and76900 reset use the existing shared SPU
 * Host scheduling replaces BIOS event installation and MMIO waits */
uint32 sound_device_reset(void)
{
    uint32 i;
    SpuReverbControl reverb;
    XASSERT(audio_initialized);
    xport_audio_lock();
    _spu_init(0);
    for (i = 0; i < SPU_VOICE_COUNT; ++i)
        spu_sample_notes[i] = SPU_INITIAL_SAMPLE_NOTE;
    spu_reverb_export(&reverb);
    reverb.enabled = 0;
    reverb.control &= ~0x80u;
    reverb.reserved = 0;
    reverb.mode = 0;
    reverb.depth_left = 0;
    reverb.depth_right = 0;
    reverb.delay = 0;
    reverb.feedback = 0;
    reverb.work_address = spu_reverb_default_work[0];
    {
        sint32 checked_result = spu_reverb_import(&reverb, NULL, NULL);
        XASSERT(checked_result);
    }
    {
        sint32 checked_result = spu_mmio_write(NULL, SPU_REGISTER_BASE + 0x1a2, 2, spu_reverb_default_work[0], 3);
        XASSERT(checked_result);
    }
    spu_bind_heap(NULL);
    sound_heap = (SpuMallocState){0};
    spu_native_transfer_state->requested_mode = 0;
    spu_native_transfer_state->normalized_mode = 0;
    spu_key_control->key_state = 0;
    spu_key_control->event_flags = 0;
    spu_key_control->pending_state = 0;
    spu_key_control->event_mode = 0;
    xport_audio_unlock();
    return SPU_REGISTER_BASE + 0x1a2;
}

/* SDK774D0 CPU allocation-table initialization without SPU register access */
sint32 sound_heap_init(sint32 entries, SpuMallocRecord *recs)
{
    sint32 result;
    if (entries <= 0)
        return 0;
    /* TODO: Investigate invalid caller workspace without redirecting allocation */
    XASSERT(recs && !((uintptr_t)recs & 3));
    xport_audio_lock();
    result = SpuInitMalloc(entries, (char *)recs);
#if defined(_DEBUG)
    sint32 heap_failed = result == entries ? spu_heap_failed() : 0;
    XASSERT(result == entries && !heap_failed);
#endif
    /* The game binds its own table only for each allocation */
    spu_bind_heap(NULL);
    sound_heap.records = recs;
    sound_heap.last_index = 0;
    sound_heap.count = entries;
    xport_audio_unlock();
    return result;
}

/* 77524/777EC use the shared allocator over the original caller-owned records */
sint32 sound_heap_allocate(uint32 bytes)
{
    SpuHeapBinding binding;
    SpuReverbControl reverb;
    uint32 count = sound_heap.count;
    sint32 result, failed;
    /* TODO: Investigate invalid heap metadata before the shared allocator */
    XASSERT(count < PSX_DRAM_SIZE / sizeof(SpuMallocRecord) && sound_heap.records && !((uintptr_t)sound_heap.records & 3));
    {
        uintptr_t ptr = (uintptr_t)sound_heap.records;
        uintptr_t base = (uintptr_t)sound_heap_recs;
        uint32 span = (sound_heap.count + 1) * sizeof(SpuMallocRecord);
        uint32 capacity = sizeof(sound_heap_recs);
        /* TODO: Investigate metadata exceeding the actual caller-owned workspace */
        XASSERT(ptr >= base && ptr - base <= capacity && span <= capacity - (ptr - base));
    }
    memset(&binding, 0, sizeof(binding));
    binding.entries = (uint32 *)sound_heap.records;
    binding.last_index = &sound_heap.last_index;
    binding.capacity = count + 1;
    binding.limit = count;
    binding.address_shift = SPU_RAM_ADDRESS_SHIFT;
    binding.alignment_mask = SPU_RAM_ADDRESS_MASK;
    spu_reverb_export(&reverb);
    if (reverb.reserved)
        binding.reserved_bytes = (0x10000u - reverb.work_address) << binding.address_shift;
    xport_audio_lock();
    {
        sint32 checked_result = spu_bind_heap(&binding);
        XASSERT(checked_result);
    }
    result = SpuMalloc(bytes);
    failed = spu_heap_failed();
    /* No heap binding survives the call */
    spu_bind_heap(NULL);
    xport_audio_unlock();
    XASSERT(!failed);
    return result;
}

static sint32 attributes(SpuVoiceAttr *desc)
{
    /* TODO: Investigate any SDK pitch-table stop during the full CDB playthrough */
    uint32 mask = desc->mask, voices = desc->voice;
    int i;
    XASSERT(mask == 3 || mask == 0x20 || mask == 0xe3 || mask == 0xff1f);
    /* Startup uses fixed volume, not hardware volume sweeps. */
    XASSERT(mask != 0xff1f || !desc->volmode.left && !desc->volmode.right);
    xport_audio_lock();
    for (i = 0; i < 24; i++)
        if (voices & (1u << i))
        {
            SpuVoiceAttr attr;
            voice_get(i, &attr);
            if (mask & 1)
                attr.volume.left = (sint16)((uint16)desc->volume.left & 0x7fff);
            if (mask & 2)
                attr.volume.right = (sint16)((uint16)desc->volume.right & 0x7fff);
            if (mask == 0x20)
            {
                /* 79254:79348..79390 uses the cached sample note; it does not
    * overwrite volume, ADSR, sample address or the sample-note cache. */
                uint16 sample_note = spu_sample_notes[i], note = desc->note;
                attr.pitch = _spu_note2pitch(sample_note >> 8, sample_note & 255, note >> 8, note & 255);
            }
            if (mask == 0xff1f)
            {
                uint32 ar = desc->ar, dr = desc->dr, sr = desc->sr;
                uint32 rr = desc->rr, sl = desc->sl, sm = desc->s_mode, sbits;
                if (ar > 127)
                    ar = 127;
                if (dr > 15)
                    dr = 15;
                if (sr > 127)
                    sr = 127;
                if (rr > 31)
                    rr = 31;
                if (sl > 15)
                    sl = 15;
                sbits = sm == 1 ? 0 : sm == 5 ? 512 : sm == 7 ? 768 : 256;
                attr.pitch = desc->pitch;
                attr.adsr1 = (uint16)(((ar | (desc->a_mode == 5 ? 128 : 0)) << 8) | (dr << 4) | sl);
                attr.adsr2 = (uint16)(((sr | sbits) << 6) | rr | (desc->r_mode == 7 ? 32 : 0));
            }
            if (mask == 0xe3)
            {
                uint16 sample_note = desc->sample_note, note = desc->note;
                uint32 address = desc->addr;
                spu_sample_notes[i] = sample_note;
                attr.pitch = _spu_note2pitch(sample_note >> 8, sample_note & 255, note >> 8, note & 255);
                if (address & SPU_RAM_ADDRESS_MASK)
                    address = (address + SPU_RAM_ADDRESS_UNIT) & ~SPU_RAM_ADDRESS_MASK;
                attr.addr = (address >> SPU_RAM_ADDRESS_SHIFT) << 3;
            }
            voice_set(i, &attr);
        }
    xport_audio_unlock();
    return 0;
}

static sint32 key_on_attributes(SpuVoiceAttr *desc)
{
    attributes(desc);
    return (sint32)spu_set_key(1, desc->voice);
}

/* Initialize the core while preserving SDK CPU state */
void sound_core_init(void)
{
    /* Preserve current transfer, key and note state */
    SpuNativeTransferState transfer_state = *spu_native_transfer_state;
    SpuKeyControl key_control = *spu_key_control;
    uint16 sample_notes[SPU_VOICE_COUNT];
    memcpy(sample_notes, spu_sample_notes, sizeof(sample_notes));
    SpuInit();
    *spu_key_control = key_control;
    memcpy(spu_sample_notes, sample_notes, sizeof(sample_notes));
    *spu_native_transfer_state = transfer_state;
    audio_initialized = 1;
    game_services.spu_key = spu_set_key;
    game_services.spu_voice_attributes = attributes;
    game_services.spu_key_on_attributes = key_on_attributes;
    game_services.play_effect = sound_voice_play;
}

uint32 sound_transfer_write(const void *src, uint32 bytes, uint32 src_bytes)
{
    SpuNativeTransferState *state = spu_native_transfer_state;
    uint32 mode, rounded, shift, written, upload_bytes;
    if (bytes > 0x7eff0)
        bytes = 0x7eff0;
    XASSERT(audio_initialized);
    mode = state->normalized_mode;
    shift = SPU_RAM_ADDRESS_SHIFT;
    /* TODO: Investigate a transfer outside the reset driver's DMA/PIO contract */
    XASSERT(mode <= 1 && shift == 3 && (mode || bytes) && !state->completion_callback);
    rounded = mode ? (bytes + 1) & ~1 : (bytes + 63) & ~63;
    /* Keep actual sample reads inside their native owner */
    XASSERT(src && !((uintptr_t)src & (mode ? 1 : 3)) && bytes <= src_bytes);
    /* Preserve available archive padding and let the SDK zero-fill the missing tail */
    upload_bytes = src_bytes < rounded ? src_bytes : rounded;
    written = SpuWrite(src, upload_bytes);
#if defined(_DEBUG)
    sint32 transfer_failed = spu_transfer_failed();
    XASSERT(!transfer_failed && written == upload_bytes);
#endif
    /* Original78D08 clears completion only without a completion callback */
    if (!state->completion_callback)
        state->completion = 0;
    return bytes;
}

void sound_transfer_wait(void)
{
    /* The native binding preserves requested-mode and completion-event semantics */
    sint32 completion_result = SpuIsTransferCompleted(1);
#if defined(_DEBUG)
    sint32 transfer_failed = completion_result == 1 ? spu_transfer_failed() : 0;
    XASSERT(completion_result == 1 && !transfer_failed);
#endif
}
