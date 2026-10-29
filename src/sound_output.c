#include <stdio.h>

#include "game.h"
#include "sound_device.h"

typedef enum SOUND_REVERB_PRESET_ID
{
    SOUND_REVERB_PRESET_DEFAULT,
    SOUND_REVERB_PRESET_MODE1_ENABLED,
    SOUND_REVERB_PRESET_MODE5_FULL,
    SOUND_REVERB_PRESET_MODE5_REDUCED,
    SOUND_REVERB_PRESET_COUNT
} SOUND_REVERB_PRESET_ID;

typedef struct SOUND_REVERB_PRESET
{
    sint32 enabled;
    uint32 left;
    uint32 right;
    sint32 mode;
    sint32 delay;
    sint32 feedback;
} SOUND_REVERB_PRESET;

static const SOUND_REVERB_PRESET presets[SOUND_REVERB_PRESET_COUNT] = {{0, 2816, 2816, 1, 8191, 16383}, {1, 8191, 8191, 1, 8191, 16383}, {1, 16383, 16383, 5, 16383, 32767}, {1, 8191, 8191, 5, 12287, 32767}};

const SOUND_SETTINGS sound_settings_defaults = {SOUND_FLAG_STEREO, 255, 255};
SOUND_SETTINGS sound_settings;
uint32 sound_reverb_configured;

sint32 sound_reverb_reset(void)
{
    FUNCTION_MARKER(0x800569dc, "GAME.EXE");
    SpuReverbAttr attr = {1, 0, {0, 0}, 0, 0};
    SpuHeapBinding binding = {0};
    SpuReverbControl reverb;
    sint32 result;
    if (!sound_reverb_configured)
        return 0;
    /* TODO: Investigate a reverb operation without a valid native heap owner */
    if (sound_heap.records)
    {
        uintptr_t ptr = (uintptr_t)sound_heap.records;
        uintptr_t base = (uintptr_t)sound_heap_recs;
        uint32 capacity = sizeof(sound_heap_recs);
        XASSERT(sound_heap.count < PSX_DRAM_SIZE / sizeof(SpuMallocRecord) && !(ptr & 3));
        XASSERT(base && ptr >= base && ptr - base <= capacity && (sound_heap.count + 1) * sizeof(SpuMallocRecord) <= capacity - (ptr - base));
        binding.entries = (uint32 *)sound_heap.records;
        binding.last_index = &sound_heap.last_index;
        binding.capacity = sound_heap.count + 1;
        binding.limit = sound_heap.count;
        binding.address_shift = SPU_RAM_ADDRESS_SHIFT;
        binding.alignment_mask = SPU_RAM_ADDRESS_MASK;
        spu_reverb_export(&reverb);
        binding.reserved_bytes = reverb.reserved ? (0x10000u - reverb.work_address) << SPU_RAM_ADDRESS_SHIFT : 0;
    }
    else
        XASSERT(!sound_heap.count && !sound_heap.last_index);
    xport_audio_lock();
    {
        sint32 checked_result = spu_bind_heap(binding.entries ? &binding : NULL);
        XASSERT(checked_result);
    }
    xport_audio_unlock();
    SpuSetReverbModeParam(&attr);
    result = SpuSetReverb(0);
    xport_audio_lock();
    spu_bind_heap(NULL);
    xport_audio_unlock();
    return result;
}

/* Configure mode before depth, reservation, output enable and voice channels */
sint32 sound_reverb_configure(void)
{
    FUNCTION_MARKER(0x80056a18, "GAME.EXE");
    uint32 stage = game_session.stage;
    const SOUND_REVERB_PRESET *preset = presets + SOUND_REVERB_PRESET_DEFAULT;
    SpuReverbAttr attr = {0};
    SpuHeapBinding binding = {0};
    SpuReverbControl reverb;
    sound_reverb_configured = 1;
    if (stage == 1 || stage == 2 || stage == 3 || stage == 7 || stage == 10 || stage == 11 || stage == 17)
        preset = presets + SOUND_REVERB_PRESET_MODE1_ENABLED;
    else if (stage == 16 || stage == 21 || stage == 24)
        preset = presets + SOUND_REVERB_PRESET_MODE5_FULL;
    else if (stage == 12 || stage == 13 || stage == 22 || stage == 23)
        preset = presets + SOUND_REVERB_PRESET_MODE5_REDUCED;
    attr.mask = 0x19;
    attr.mode = preset->mode;
    attr.delay = preset->delay;
    attr.feedback = preset->feedback;
    /* TODO: Investigate a reverb operation without a valid native heap owner */
    if (sound_heap.records)
    {
        uintptr_t ptr = (uintptr_t)sound_heap.records;
        uintptr_t base = (uintptr_t)sound_heap_recs;
        uint32 capacity = sizeof(sound_heap_recs);
        XASSERT(sound_heap.count < PSX_DRAM_SIZE / sizeof(SpuMallocRecord) && !(ptr & 3));
        XASSERT(base && ptr >= base && ptr - base <= capacity && (sound_heap.count + 1) * sizeof(SpuMallocRecord) <= capacity - (ptr - base));
        binding.entries = (uint32 *)sound_heap.records;
        binding.last_index = &sound_heap.last_index;
        binding.capacity = sound_heap.count + 1;
        binding.limit = sound_heap.count;
        binding.address_shift = SPU_RAM_ADDRESS_SHIFT;
        binding.alignment_mask = SPU_RAM_ADDRESS_MASK;
        spu_reverb_export(&reverb);
        binding.reserved_bytes = reverb.reserved ? (0x10000u - reverb.work_address) << SPU_RAM_ADDRESS_SHIFT : 0;
    }
    else
        XASSERT(!sound_heap.count && !sound_heap.last_index);
    xport_audio_lock();
    {
        sint32 checked_result = spu_bind_heap(binding.entries ? &binding : NULL);
        XASSERT(checked_result);
    }
    xport_audio_unlock();
    SpuSetReverbModeParam(&attr);
    attr.mask = 6;
    attr.depth.left = preset->left;
    attr.depth.right = preset->right;
    SpuSetReverbModeParam(&attr);
    SpuReserveReverbWorkArea(preset->enabled);
    SpuSetReverb(preset->enabled);
    SpuSetReverbVoice(preset->enabled, 0xffffff);
    xport_audio_lock();
    spu_bind_heap(NULL);
    xport_audio_unlock();
    return game_session.stage == 17 ? sound_reverb_disable_channels() : 17;
}

GDB_CALL sint32 sound_reverb_enable_channels(void)
{
    FUNCTION_MARKER(0x80056b78u, "GAME.EXE");
    return SpuSetReverbVoice(1, 0xffffff);
}

sint32 sound_reverb_disable_channels(void)
{
    FUNCTION_MARKER(0x80056ba0, "GAME.EXE");
    return SpuSetReverbVoice(0, 0xffffff);
}

sint32 sound_output_init(void)
{
    FUNCTION_MARKER(0x80056bc8, "GAME.EXE");
    SpuVoiceAttr *slot = sound_voice_next_attributes();
    SpuCommonAttr common = {0x2c3, {0x3fff, 0x3fff}, {0, 0}, {0, 0}, {{0x3fff, 0x3fff}, 0, 1}, {{0, 0}, 0, 0}};
    sound_reverb_reset();
    sound_device_init();
    sound_heap_init(SOUND_HEAP_LIMIT, sound_heap_recs);
    SpuSetCommonAttr(&common);
    SpuSetTransferMode(0);
    sound_voice_defaults(slot);
    return sound_reverb_configure();
}

GDB_CALL uint32 sound_device_init(void)
{
    FUNCTION_MARKER(0x8007676cu, "GAME.EXE");
    return sound_device_reset(); /* 7678C mode0, existing native SPU reset. */
}
