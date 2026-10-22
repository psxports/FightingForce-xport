/* Native PsyQ/SPU adapter: menu3/E3 and startupFF1F attribute masks. */
#include "ff_audio.h"
#include "psx_spu.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
static int audio_initialized;
uint16 ff_spu_common_state[5];
uint16 ff_spu_reverb_registers[32];
uint16 ff_spu_reverb_state[3];
uint16 ff_spu_reverb_channels_state[2];
static uint32 ff_spu_dma_state[5]; /* transfer units, DPCR, MADR, BCR, CHCR */

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

void ff_spu_voice_snapshot(sint32 voice, uint16 registers[7])
{
    SpuVoiceAttr attr;
    voice_get(voice, &attr);
    registers[0] = (uint16)attr.volume.left;
    registers[1] = (uint16)attr.volume.right;
    registers[2] = attr.pitch;
    registers[3] = (uint16)(attr.addr >> 3);
    registers[4] = attr.adsr1;
    registers[5] = attr.adsr2;
    registers[6] = (uint16)(attr.loop_addr >> 3);
}

void ff_spu_voice_seed(sint32 voice, const uint16 registers[7])
{
    SpuVoiceAttr attr;
    memset(&attr, 0, sizeof(attr));
    attr.volume.left = (sint16)registers[0];
    attr.volume.right = (sint16)registers[1];
    attr.pitch = registers[2];
    attr.addr = (uint32)registers[3] << 3;
    attr.adsr1 = registers[4];
    attr.adsr2 = registers[5];
    attr.loop_addr = (uint32)registers[6] << 3;
    voice_set(voice, &attr);
}

static uint16 slus_spu_control(void)
{
    uint32 p = r_u32(0x80030294) + 0x1aa;
    return (p & 0x1fffffffu) == 0x1f801daa ? ff_spu_common_state[4] : (uint16)r_s16(p);
}

static void slus_spu_control_write(uint16 value)
{
    uint32 p = r_u32(0x80030294) + 0x1aa;
    if ((p & 0x1fffffffu) == 0x1f801daa)
        ff_spu_common_state[4] = value;
    else
        w_u16(p, value);
}

GDB_CALL uint32 FUN_SLUS_8001CF10(uint32 entry_v0, FF_SLUS_SERVICE service)
{
    FUNCTION_MARKER(0x8001cf10u, "SLUS_004.33");
    uint32 address = r_u32(0x80030294) + 0x1aa, attempts, callback;
    int hardware = (address & 0x1fffffff) == 0x1f801daa;
    uint16 control = hardware ? ff_spu_common_state[4] : r_u16(address);
    /* 1D5D4 is a pure caller-stack delay; native device timing is handled by the adapter */
    control &= 0xffcf;
    if (hardware)
        ff_spu_common_state[4] = control;
    else
        w_u16(address, control);
    control = hardware ? ff_spu_common_state[4] : r_u16(address);
    if (control & 0x30)
    {
        attempts = 1;
        while (attempts < 0xf01)
        {
            control = hardware ? ff_spu_common_state[4] : r_u16(address);
            ++attempts;
            if (!(control & 0x30))
                break;
        }
    }
    callback = r_u32(0x800302cc);
    if (callback)
    {
        callback = r_u32(0x800302cc);
        return service(callback, 0xf0000000u, 0, 0, 0, 0);
    }
    return service(0x8001783c, 0xf0000009u, 0x20, 0, 0, 0); /* DeliverEvent, not a fabricated callback result. */
}

static void slus_spu_upload(uint32 source, uint32 bytes, uint32 target)
{
    uint32 chunk;
    if (!audio_initialized)
    {
        SpuInit();
        audio_initialized = 1;
    }
    xport_audio_lock();
    source &= 0x1fffff;
    target &= 0x7ffff;
    while (bytes)
    {
        chunk = bytes;
        if (chunk > 0x200000 - source)
            chunk = 0x200000 - source;
        if (chunk > 0x80000 - target)
            chunk = 0x80000 - target;
        if (!spu_upload(target, psx_addr(source, chunk), chunk))
            ff_wip_stop(__FUNCTION__, __FILE__, __LINE__);
        source = (source + chunk) & 0x1fffff;
        target = (target + chunk) & 0x7ffff;
        bytes -= chunk;
    }
    ff_spu_dma_state[0] = target >> 3;
    xport_audio_unlock();
}

static void spu_dma_word(uint32 index, uint32 address, uint32 expected, uint32 value)
{
    if ((address & 0x1fffffffu) == expected)
        ff_spu_dma_state[index] = value;
    else
        w_u32(address, value);
}

GDB_CALL uint32 FUN_SLUS_8001D30C(uint32 source, uint32 bytes, uint32 entry_v0)
{
    FUNCTION_MARKER(0x8001d30cu, "SLUS_004.33");
    uint32 base = r_u32(0x80030294), units = (uint16)r_s16(0x800302ac), shift, address, value, blocks, remaining, chunk, i, position;
    if (!entry_v0)
    {
        shift = r_u32(0x800302bc) & 31;
        units = (units << shift) >> shift;
        w_u16(0x800302ac, (uint16)units);
        address = base + 0x1a6;
        if ((address & 0x1fffffffu) == 0x1f801da6)
            ff_spu_dma_state[0] = units;
        else
            w_u16(address, (uint16)units);
        w_u32(0x800302e4, 0);
        slus_spu_control_write((slus_spu_control() & 0xffcf) | 0x20);
        address = r_u32(0x800302a8);
        value = (address & 0x1fffffffu) == 0x1f8010f0 ? ff_spu_dma_state[1] : r_u32(address);
        spu_dma_word(1, address, 0x1f8010f0, (value & 0xf0ffffffu) | 0x20000000u);
        blocks = (bytes >> 6) + ((bytes & 63) != 0);
        w_u32(0x800302e8, source);
        w_u32(0x800302ec, blocks);
        spu_dma_word(2, r_u32(0x80030298), 0x1f8010c0, source);
        spu_dma_word(3, r_u32(0x8003029c), 0x1f8010c4, (blocks << 16) | 16);
        address = r_u32(0x800302a0);
        spu_dma_word(4, address, 0x1f8010c8, 0x01000201);
        if ((address & 0x1fffffffu) == 0x1f8010c8)
        {
            slus_spu_upload(source & 0x1ffffc, (blocks & 65535) * 64, (units << 3) & 0x7ffff);
            ff_spu_dma_state[4] &= ~0x01000000u;
        }
    }
    else
    {
        address = base + 0x1a6;
        if ((address & 0x1fffffffu) == 0x1f801da6)
            ff_spu_dma_state[0] = units;
        else
            w_u16(address, (uint16)units);
        remaining = bytes;
        position = (units << 3) & 0x7ffff;
        while (remaining)
        {
            chunk = remaining < 64 ? remaining : 64;
            for (i = 0; i < chunk; i += 2)
            {
                value = (uint16)r_s16(source + i);
                if (((base + 0x1a8) & 0x1fffffffu) != 0x1f801da8)
                    w_u16(base + 0x1a8, (uint16)value);
            }
            if (((base + 0x1a8) & 0x1fffffffu) == 0x1f801da8)
                slus_spu_upload(source, (chunk + 1) & ~1u, position);
            source += (chunk + 1) & ~1u;
            position = (position + ((chunk + 1) & ~1u)) & 0x7ffff;
            remaining -= chunk;
            slus_spu_control_write((slus_spu_control() & 0xffcf) | 0x10);
        }
        slus_spu_control_write(slus_spu_control() & 0xffcf);
    }
    return bytes; /* No SpuWrite cap in this split entry. */
}

static uint32 spu_dma_read_entry(uint32 destination, uint32 address_units, uint32 blocks, uint32 entry_v0, uint32 globals)
{
    uint32 address, value, remaining, source, chunk, target;
    address = entry_v0 + 0x1a6;
    if ((address & 0x1fffffffu) == 0x1f801da6)
        ff_spu_dma_state[0] = (uint16)address_units;
    else
        w_u16(address, (uint16)address_units);
    /* SKIP7744C delays have no externally visible effect with synchronous DMA. */
    address = r_u32(globals) + 0x1aa;
    if ((address & 0x1fffffffu) == 0x1f801daa)
        ff_spu_common_state[4] |= 0x30;
    else
        w_u16(address, (uint16)r_s16(address) | 0x30);
    address = r_u32(globals + 20);
    value = (address & 0x1fffffffu) == 0x1f8010f0 ? ff_spu_dma_state[1] : r_u32(address);
    spu_dma_word(1, address, 0x1f8010f0, (value & 0xf0ffffffu) | 0x22000000u);
    spu_dma_word(2, r_u32(globals + 4), 0x1f8010c0, destination);
    spu_dma_word(3, r_u32(globals + 8), 0x1f8010c4, (blocks << 16) | 16);
    address = r_u32(globals + 12);
    w_u32(globals + 80, 1);
    spu_dma_word(4, address, 0x1f8010c8, 0x01000200);
    if ((address & 0x1fffffffu) == 0x1f8010c8)
    {
        /* CHCR bit0=0 is SPU-to-main-RAM. This is SpuRead, not an upload. */
        source = (ff_spu_dma_state[0] << 3) & 0x7ffff;
        target = ff_spu_dma_state[2] & 0x1ffffc;
        remaining = ff_spu_dma_state[3] >> 16;
        /* Zero in the DMA block-count field represents 65536 blocks */
        remaining = (remaining ? remaining : 65536) * (ff_spu_dma_state[3] & 65535u) * 4;
        if (!audio_initialized)
        {
            SpuInit();
            audio_initialized = 1;
        }
        xport_audio_lock();
        while (remaining)
        {
            chunk = remaining;
            if (chunk > 0x80000 - source)
                chunk = 0x80000 - source;
            if (chunk > 0x200000 - target)
                chunk = 0x200000 - target;
            if (!spu_download(source, psx_addr(target, chunk), chunk))
                ff_wip_stop(__FUNCTION__, __FILE__, __LINE__);
            source = (source + chunk) & 0x7ffff;
            target = (target + chunk) & 0x1fffff;
            remaining -= chunk;
        }
        ff_spu_dma_state[0] = source >> 3;
        ff_spu_dma_state[4] &= ~0x01000000u;
        xport_audio_unlock();
    }
    return 1;
}

GDB_CALL uint32 FUN_80076E38(uint32 destination, uint32 address_units, uint32 blocks, uint32 entry_v0)
{
    FUNCTION_MARKER(0x80076e38u, "GAME.EXE");
    return spu_dma_read_entry(destination, address_units, blocks, entry_v0, 0x8008cf90);
}

GDB_CALL uint32 FUN_SLUS_8001CFD0(uint32 destination, uint32 address_units, uint32 blocks, uint32 entry_v0)
{
    FUNCTION_MARKER(0x8001cfd0u, "SLUS_004.33");
    /* Manual semantic peer of GAME76E38: relocated globals, two rather than
  * four delay calls. Synchronous boundary shares data/register operations. */
    return spu_dma_read_entry(destination, address_units, blocks, entry_v0, 0x80030294);
}

/* 786F8/7871C: deferred mode updates the SDK queue, not live registers */
uint32 ff_spu_reverb_channels(sint32 enabled, uint32 mask)
{
    uint16 low = (uint16)mask, high = (uint16)((mask >> 16) & 255);
    if (r_u32(0x8008cf88) & 1)
    {
        uint32 result = ((uint32)(uint16)r_s16(0x800adbda) & 255) << 16;
        result |= (uint16)r_s16(0x800adbd8);
        if (enabled == 1)
        {
            w_u16(0x800adbd8, (uint16)((uint16)r_s16(0x800adbd8) | low));
            w_u16(0x800adbda, (uint16)((uint16)r_s16(0x800adbda) | high));
            w_u32(0x8008cb54, r_u32(0x8008cb54) | 8);
            result |= mask & 0xffffff;
        }
        else if (enabled == 0)
        {
            w_u16(0x800adbd8, (uint16)((uint16)r_s16(0x800adbd8) & (uint16)~low));
            w_u16(0x800adbda, (uint16)((uint16)r_s16(0x800adbda) & (uint16)~high));
            w_u32(0x8008cb54, r_u32(0x8008cb54) | 8);
            result &= ~(mask & 0xffffff);
        }
        return result & 0xffffff;
    }
    if (enabled == 1)
    {
        ff_spu_reverb_channels_state[0] |= low;
        ff_spu_reverb_channels_state[1] |= high;
        SpuSetReverbVoice(SPU_ON, mask);
    }
    else if (enabled == 0)
    {
        ff_spu_reverb_channels_state[0] &= (uint16)~low;
        ff_spu_reverb_channels_state[1] &= (uint16)~high;
        SpuSetReverbVoice(SPU_OFF, mask);
    }
    return ff_spu_reverb_channels_state[0] | ((uint32)(ff_spu_reverb_channels_state[1] & 255) << 16);
}

/* Native completion consumes the BIOS event after the synchronous DMA upload */
static sint32 ff_spu_clear_event(void *context)
{
    DeliverEvent(0xf0000009, 0x20);
    return TestEvent(*(sint32 *)context);
}

/* 78924 clear uses the original source buffer and preserves normal completion */
static sint32 ff_spu_clear_reverb_work_area(uint32 mode)
{
    SpuTransferBinding binding = {0};
    sint32 event, result;
    if (mode >= 10 || ff_spu_reverb_conflict(r_u32(0x8008cff8 + 4 * mode)))
        return -1;
    event = OpenEvent(0xf0000009, 0x20, 0x2000, NULL);
    if (event < 1)
        ff_wip_stop(__FUNCTION__, __FILE__, __LINE__);
    if (!EnableEvent(event))
    {
        CloseEvent(event);
        ff_wip_stop(__FUNCTION__, __FILE__, __LINE__);
    }
    binding.normalized_mode = (uint32 *)psx_addr(0x8008cfac, 4);
    binding.address_units = (uint16 *)psx_addr(0x8008cfa8, 2);
    binding.completion_callback = (uint32 *)psx_addr(0x8008cfc8, 4);
    binding.source_address = (uint32 *)psx_addr(0x8008cfe4, 4);
    binding.dma_blocks = (uint32 *)psx_addr(0x8008cfe8, 4);
    binding.address_shift = r_u32(0x8008cfb8) & 31;
    binding.block_bytes = 64;
    binding.complete_event = ff_spu_clear_event;
    binding.guest_address = NULL;
    binding.context = &event;
    spu_bind_reverb_presets((const uint32 *)psx_addr(0x8008cff8, 40), (const uint8 *)psx_addr(0x8008d048, 680), binding.address_shift);
    result = spu_clear_reverb_transfer((sint32)mode, psx_addr(0x8008cb88, 1024), &binding, (uint32 *)psx_addr(0x8008cfe0, 4));
    if (!CloseEvent(event))
        ff_wip_stop(__FUNCTION__, __FILE__, __LINE__);
    if (result)
        ff_wip_stop(__FUNCTION__, __FILE__, __LINE__);
    return 0;
}

/* 77CC8 parameter state and synchronous native work-area transfer */
sint32 ff_spu_reverb_attributes(uint32 a)
{
    uint32 values[5];
    memcpy(values, psx_addr(a, 20), 20);
    return ff_spu_reverb_attributes_host(values);
}

sint32 ff_spu_reverb_attributes_host(const uint32 a[5])
{
    uint32 mask = a[0], change = !mask || (mask & 1), mode = r_u32(0x8008cb40), control = 0, clear = 0;
    SpuReverbAttr attr;
    SpuReverbControl state;
    if (change)
    {
        mode = a[1];
        clear = mode & 0x100;
        mode &= ~0x100;
        if (mode >= 10)
            return -1;
        if (ff_spu_reverb_conflict(r_u32(0x8008cff8 + 4 * mode)))
            return -1;
        w_u32(0x8008cb40, mode);
        w_u32(0x8008cb38, r_u32(0x8008cff8 + 4 * mode));
        w_u32(0x8008cb4c, mode == 7 ? 127 : 0);
        w_u32(0x8008cb48, mode == 7 || mode == 8 ? 127 : 0);
    }
    if (!mask || (mask & 8))
    {
        w_u32(0x8008cb48, mode == 7 || mode == 8 ? a[3] : 0);
    }
    if (!mask || (mask & 16))
    {
        w_u32(0x8008cb4c, mode == 7 || mode == 8 ? a[4] : 0);
    }
    if (change)
    {
        if (!spu_mmio_read(NULL, 0x1f801daa, 2, &control))
            ff_wip_stop(__FUNCTION__, __FILE__, __LINE__);
        ff_spu_common_state[4] = (uint16)(control & 0xff7f);
        if ((control & 0x80) && !spu_mmio_write(NULL, 0x1f801daa, 2, control & 0xff7f, 3))
            ff_wip_stop(__FUNCTION__, __FILE__, __LINE__);
        ff_spu_reverb_state[0] = ff_spu_reverb_state[1] = 0;
        w_u16(0x8008cb44, 0);
        w_u16(0x8008cb46, 0);
        ff_spu_reverb_state[2] = (uint16)r_u32(0x8008cb38);
    }
    else
    {
        if (mask & 2)
        {
            ff_spu_reverb_state[0] = (uint16)a[2];
            w_u16(0x8008cb44, ff_spu_reverb_state[0]);
        }
        if (mask & 4)
        {
            ff_spu_reverb_state[1] = (uint16)(a[2] >> 16);
            w_u16(0x8008cb46, ff_spu_reverb_state[1]);
        }
    }
    spu_bind_reverb_presets((const uint32 *)psx_addr(0x8008cff8, 40), (const uint8 *)psx_addr(0x8008d048, 680), r_u32(0x8008cfb8));
    attr.mask = mask;
    attr.mode = (sint32)(change ? mode : a[1]);
    attr.depth.left = (sint16)a[2];
    attr.depth.right = (sint16)(a[2] >> 16);
    attr.delay = (sint32)a[3];
    attr.feedback = (sint32)a[4];
    if (SpuSetReverbModeParam(&attr))
        ff_wip_stop(__FUNCTION__, __FILE__, __LINE__);
    spu_reverb_export(&state);
    memcpy(ff_spu_reverb_registers, state.parameters, sizeof(ff_spu_reverb_registers));
    if (clear)
        ff_spu_clear_reverb_work_area(mode);
    if (change && (control & 0x80))
    {
        if (!spu_mmio_read(NULL, 0x1f801daa, 2, &control) || !spu_mmio_write(NULL, 0x1f801daa, 2, control | 0x80, 3))
            ff_wip_stop(__FUNCTION__, __FILE__, __LINE__);
        ff_spu_common_state[4] = (uint16)(control | 0x80);
    }
    return 0;
}

/* 781D8: mask0 writes every word; all other masks select individual words. */
uint32 ff_spu_reverb_register_write(uint32 a)
{
    uint32 mask = r_u32(a), i;
    for (i = 0; i < 32; i++)
        if (!mask || (mask & (1u << i)))
            ff_spu_reverb_registers[i] = (uint16)r_s16(a + 4 + 2 * i);
    /* The final two branches determine v0, regardless of earlier writes. */
    if (!mask || (mask & 0x80000000u))
        return ff_spu_reverb_registers[31];
    if (mask & 0x40000000u)
        return ff_spu_reverb_registers[30];
    return 0;
}

/* 77C3C checks whether any allocated range overlaps the proposed work area. */
sint32 ff_spu_reverb_conflict(uint32 units)
{
    uint32 address = units << (r_u32(0x8008cfb8) & 31), table = r_u32(0x8008cff4), record, start;
    if (!table)
        return 0;
    for (;; table += 8)
    {
        record = r_u32(table);
        if (record & 0x80000000u)
            continue;
        if (record & 0x40000000u)
            return 0;
        start = record & 0x0fffffffu;
        if (start >= address || address < start + r_u32(table + 4))
            return 1;
    }
}

sint32 ff_spu_reverb_reserve(sint32 enabled)
{
    uint32 result = enabled && !ff_spu_reverb_conflict(r_u32(0x8008cb38));
    w_u32(0x8008cb34, result);
    return (sint32)result;
}

sint32 ff_spu_reverb_enable(sint32 enabled)
{
    uint32 result, control;
    if (enabled != 0 && enabled != 1)
        return (sint32)r_u32(0x8008cb30);
    result = enabled && (r_u32(0x8008cb34) == 1 || !ff_spu_reverb_conflict(r_u32(0x8008cb38)));
    if (!spu_mmio_read(NULL, 0x1f801daa, 2, &control))
        ff_wip_stop(__FUNCTION__, __FILE__, __LINE__);
    w_u32(0x8008cb30, result);
    ff_spu_common_state[4] = (uint16)((control & 0xff7f) | (result ? 0x80 : 0));
    if (!spu_mmio_write(NULL, 0x1f801daa, 2, ff_spu_common_state[4], 3))
        ff_wip_stop(__FUNCTION__, __FILE__, __LINE__);
    return (sint32)result;
}

/* 7676C/7678C mode0 and76900 reset use the existing shared SPU
 * Host scheduling replaces BIOS event installation and MMIO waits */
uint32 ff_spu_reset_startup(void)
{
    static const uint32 clear[] = {0x8008cfac, 0x8008cfb0, 0x8008cfc8, 0x8008cfcc, 0x8008cb30, 0x8008cb34, 0x8008cb40, 0x8008cb44, 0x8008cb48, 0x8008cb4c, 0x8008cfec, 0x8008cff0, 0x8008cff4, 0x8008cb2c, 0x8008cb28, 0x8008cb54, 0x8008cb50, 0x8008cf88};
    static const uint32 reset_registers[] = {0x1f801d84, 0x1f801d86, 0x1f801d98, 0x1f801d9a, 0x1f801d90, 0x1f801d92, 0x1f801d94, 0x1f801d96, 0x1f801db4, 0x1f801db6};
    uint32 i;
    SpuVoiceAttr attr;
    SpuCommonAttr common;
    if (!audio_initialized)
        ff_wip_stop(__FUNCTION__, __FILE__, __LINE__);
    if (!spu_mmio_write(NULL, 0x1f801daa, 2, 0, 3))
        ff_wip_stop(__FUNCTION__, __FILE__, __LINE__);
    for (i = 0; i < sizeof(clear) / sizeof(clear[0]); i++)
        w_u32(clear[i], 0);
    w_u16(0x8008cfa8, 0x200);
    w_u32(0x8008cfb4, 2);
    w_u32(0x8008cfb8, 3);
    w_u32(0x8008cfbc, 8);
    w_u32(0x8008cfc0, 7);
    w_u32(0x8008cfc4, 1);
    for (i = 0; i < 10; i++)
        w_u16(0x800adbc8 + 2 * i, 0);
    for (i = 0; i < 24; i++)
        w_u16(0x8008cb58 + 2 * i, 0xc000);
    w_u32(0x8008cb38, r_u32(0x8008cff8));
    ff_spu_reverb_state[0] = ff_spu_reverb_state[1] = 0;
    ff_spu_reverb_state[2] = (uint16)r_u32(0x8008cff8);
    ff_spu_reverb_channels_state[0] = ff_spu_reverb_channels_state[1] = 0;
    xport_audio_lock();
    if (!spu_mmio_write(NULL, 0x1f801dac, 2, 4, 3))
        ff_wip_stop(__FUNCTION__, __FILE__, __LINE__);
    /* Reset live effects and external inputs, not only their guest shadows */
    for (i = 0; i < sizeof(reset_registers) / sizeof(reset_registers[0]); i++)
        if (!spu_mmio_write(NULL, reset_registers[i], 2, 0, 3))
            ff_wip_stop(__FUNCTION__, __FILE__, __LINE__);
    if (!spu_upload(0x1000, psx_addr(0x8008cfd0, 16), 16))
        ff_wip_stop(__FUNCTION__, __FILE__, __LINE__);
    for (i = 0; i < 24; i++)
    {
        voice_get((sint32)i, &attr);
        attr.volume.left = attr.volume.right = 0;
        attr.pitch = 0x3fff;
        attr.addr = 0x1000;
        attr.adsr1 = attr.adsr2 = 0;
        voice_set((sint32)i, &attr);
    }
    SpuSetKey(SPU_ON, SPU_ALLCH);
    SpuSetKey(SPU_OFF, SPU_ALLCH);
    memset(&common, 0, sizeof(common));
    common.mask = SPU_COMMON_MVOLL | SPU_COMMON_MVOLR | SPU_COMMON_CDVOLL | SPU_COMMON_CDVOLR;
    SpuSetCommonAttr(&common);
    ff_spu_common_state[0] = ff_spu_common_state[1] = 0;
    ff_spu_common_state[2] = ff_spu_common_state[3] = 0;
    ff_spu_common_state[4] = 0xc000;
    if (!spu_mmio_write(NULL, 0x1f801daa, 2, 0xc000, 3))
        ff_wip_stop(__FUNCTION__, __FILE__, __LINE__);
    if (!spu_mmio_write(NULL, 0x1f801da2, 2, ff_spu_reverb_state[2], 3))
        ff_wip_stop(__FUNCTION__, __FILE__, __LINE__);
    xport_audio_unlock();
    /* 7726C returns the guest register address, not the written value. */
    return r_u32(0x8008cf90) + 0x1a2;
}

sint32 ff_spu_common_startup(uint32 a)
{
    uint32 values[10];
    memcpy(values, psx_addr(a, 40), 40);
    return ff_spu_common_startup_host(values);
}

void ff_spu_cd_volume_host(sint16 left, sint16 right)
{
    SpuCommonAttr attr;
    /* SLUS1C618 mode0 -> SpuSetCommonAttr maskC0 */
    if (left >= 128)
        left = 127;
    if (right >= 128)
        right = 127;
    if (!audio_initialized)
        ff_wip_stop(__FUNCTION__, __FILE__, __LINE__);
    xport_audio_lock();
    ff_spu_common_state[2] = (uint16)((sint32)left * 258);
    ff_spu_common_state[3] = (uint16)((sint32)right * 258);
    memset(&attr, 0, sizeof(attr));
    attr.mask = SPU_COMMON_CDVOLL | SPU_COMMON_CDVOLR;
    attr.cd.volume.left = (sint16)ff_spu_common_state[2];
    attr.cd.volume.right = (sint16)ff_spu_common_state[3];
    SpuSetCommonAttr(&attr);
    xport_audio_unlock();
}

sint32 ff_spu_common_startup_host(const uint32 a[10])
{
    SpuCommonAttr attr;
    if (a[0] != 0x2c3 || !audio_initialized)
        ff_wip_stop(__FUNCTION__, __FILE__, __LINE__);
    xport_audio_lock();
    ff_spu_common_state[0] = (uint16)a[1] & 0x7fff;
    ff_spu_common_state[1] = (uint16)(a[1] >> 16) & 0x7fff;
    ff_spu_common_state[2] = (uint16)a[4];
    ff_spu_common_state[3] = (uint16)(a[4] >> 16);
    ff_spu_common_state[4] = (ff_spu_common_state[4] & 0xfffe) | (a[6] ? 1 : 0);
    memset(&attr, 0, sizeof(attr));
    attr.mask = SPU_COMMON_MVOLL | SPU_COMMON_MVOLR | SPU_COMMON_CDVOLL | SPU_COMMON_CDVOLR | SPU_COMMON_CDMIX;
    attr.mvol.left = (sint16)ff_spu_common_state[0];
    attr.mvol.right = (sint16)ff_spu_common_state[1];
    attr.cd.volume.left = (sint16)ff_spu_common_state[2];
    attr.cd.volume.right = (sint16)ff_spu_common_state[3];
    attr.cd.mix = a[6] != 0;
    SpuSetCommonAttr(&attr);
    xport_audio_unlock();
    return 0;
}

/* SDK774D0 CPU allocation-table initialization; no SPU register access. */
sint32 ff_spu_transfer_mode(sint32 mode)
{
    uint32 result = mode == 1;
    w_u32(0x8008cb2c, (uint32)mode);
    w_u32(0x8008cfac, result);
    return (sint32)result;
}

sint32 ff_spu_allocation_init(sint32 entries, uint32 table)
{
    if (entries <= 0)
        return 0;
    w_u32(table, 0x40001010);
    w_u32(0x8008cff4, table);
    w_u32(0x8008cff0, 0);
    w_u32(0x8008cfec, (uint32)entries);
    w_u32(table + 4, (0x10000u << (r_u32(0x8008cfb8) & 31)) - 0x1010);
    return entries;
}

/* 77524/777EC use the shared allocator over the original caller-owned records */
sint32 ff_spu_allocate_startup(uint32 bytes)
{
    SpuHeapBinding binding;
    uint32 count = r_u32(0x8008cfec), table = r_u32(0x8008cff4);
    sint32 result, failed;
    if (count >= PSX_DRAM_SIZE / 8)
        ff_wip_stop(__FUNCTION__, __FILE__, __LINE__);
    memset(&binding, 0, sizeof(binding));
    binding.entries = (uint32 *)psx_addr(table, (size_t)(count + 1) * 8);
    binding.last_index = (uint32 *)psx_addr(0x8008cff0, 4);
    binding.capacity = count + 1;
    binding.limit = count;
    binding.address_shift = r_u32(0x8008cfb8) & 31;
    binding.alignment_mask = r_u32(0x8008cfc0);
    if (r_u32(0x8008cb34))
        binding.reserved_bytes = (0x10000 - r_u32(0x8008cb38)) << binding.address_shift;
    xport_audio_lock();
    if (!spu_bind_heap(&binding))
        ff_wip_stop(__FUNCTION__, __FILE__, __LINE__);
    result = SpuMalloc((sint32)bytes);
    failed = spu_heap_failed();
    /* No heap binding survives the call or needs checkpoint hydration */
    spu_bind_heap(NULL);
    xport_audio_unlock();
    if (failed)
        ff_wip_stop(__FUNCTION__, __FILE__, __LINE__);
    return result;
}

/* SDK GAME:800798B0: integer table conversion, MIPS shift counts modulo 32. */
GDB_CALL uint16 ff_note_to_pitch(sint32 base, uint16 fine, sint32 note, uint16 note_fine)
{
    sint32 fraction = ((uint32)fine + note_fine) >> 3, carry = 0, semitones, octave, shift, index;
    uint32 pitch;
    while (fraction >= 16)
    {
        fraction -= 16;
        ++carry;
    }
    semitones = (sint16)((uint32)carry + (uint32)note + 60u - (uint32)base);
    octave = semitones / 12;
    index = (sint16)(semitones - 12 * octave) * 16 + fraction;
    pitch = (uint16)r_s16(0x8008d2f0u + (uint32)index * 2u);
    shift = (sint16)(octave - 5);
    if (shift > 0)
        pitch <<= ((uint32)shift & 31);
    else if (shift < 0)
        pitch >>= ((0u - (uint32)shift) & 31);
    return (uint16)pitch;
}

static sint32 key(sint32 enabled, uint32 channels)
{
    uint32 mask = channels & 0xffffff, result, opposite, high, deferred;
    if (enabled != 0 && enabled != 1)
        return 1;
    xport_audio_lock();
    deferred = r_u32(0x8008cf88) & 1;
    opposite = 0;
    if (deferred)
    {
        opposite = r_u16(enabled ? 0x800adbce : 0x800adbca);
    }
    spu_bind_key_guest(0x8008cb28, 0x8008cf88, 0x8008cb54, 0x8008cb50, 0x800adbc8);
    SpuSetKey(enabled, mask);
    spu_bind_key_guest(0, 0, 0, 0, 0);
    high = mask >> 16;
    /* 78AD0 returns the last high-word cancellation result in deferred mode */
    result = deferred ? ((opposite & high) ? opposite & ~high : 0) : r_u32(0x8008cb28);
    xport_audio_unlock();
    return (sint32)result;
}

static sint32 attributes(uint32 a)
{
    uint32 mask = r_u32(a + 4), voices = r_u32(a);
    int i;
    if (mask != 3 && mask != 0x20 && mask != 0xe3 && mask != 0xff1f)
        ff_wip_stop(__FUNCTION__, __FILE__, __LINE__);
    /* Startup uses fixed volume, not hardware volume sweeps. */
    if (mask == 0xff1f && (r_s16(a + 12) || r_s16(a + 14)))
        ff_wip_stop(__FUNCTION__, __FILE__, __LINE__);
    xport_audio_lock();
    for (i = 0; i < 24; i++)
        if (voices & (1u << i))
        {
            SpuVoiceAttr attr;
            voice_get(i, &attr);
            if (mask & 1)
                attr.volume.left = (sint16)((uint16)r_s16(a + 8) & 0x7fff);
            if (mask & 2)
                attr.volume.right = (sint16)((uint16)r_s16(a + 10) & 0x7fff);
            if (mask == 0x20)
            {
                /* 79254:79348..79390 uses the cached sample note; it does not
    * overwrite volume, ADSR, sample address or the sample-note cache. */
                uint16 sample_note = (uint16)r_s16(0x8008cb58 + 2u * i), note = (uint16)r_s16(a + 22);
                attr.pitch = ff_note_to_pitch(sample_note >> 8, sample_note & 255, note >> 8, note & 255);
            }
            if (mask == 0xff1f)
            {
                uint32 ar = (uint16)r_s16(a + 48), dr = (uint16)r_s16(a + 50), sr = (uint16)r_s16(a + 52);
                uint32 rr = (uint16)r_s16(a + 54), sl = (uint16)r_s16(a + 56), sm = r_u32(a + 40), sbits;
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
                attr.pitch = (uint16)r_s16(a + 20);
                attr.adsr1 = (uint16)(((ar | (r_u32(a + 36) == 5 ? 128 : 0)) << 8) | (dr << 4) | sl);
                attr.adsr2 = (uint16)(((sr | sbits) << 6) | rr | (r_u32(a + 44) == 7 ? 32 : 0));
            }
            if (mask == 0xe3)
            {
                uint16 sample_note = (uint16)r_s16(a + 24), note = (uint16)r_s16(a + 22);
                uint32 address = r_u32(a + 28);
                w_u16(0x8008cb58 + 2u * i, sample_note);
                attr.pitch = ff_note_to_pitch(sample_note >> 8, sample_note & 255, note >> 8, note & 255);
                if (r_u32(0x8008cfb4))
                {
                    uint32 alignment = r_u32(0x8008cfbc);
                    if (!alignment)
                        ff_wip_stop(__FUNCTION__, __FILE__, __LINE__);
                    if (address % alignment)
                        address = (address + alignment) & ~r_u32(0x8008cfc0);
                }
                attr.addr = (address >> (r_u32(0x8008cfb8) & 31)) << 3;
            }
            voice_set(i, &attr);
        }
    xport_audio_unlock();
    return 0;
}

static sint32 key_on_attributes(uint32 a)
{
    attributes(a);
    return key(1, r_u32(a));
}

/* Empty host core for original-resource startup; CPU SDK initialization is
 * the caller's responsibility. No RAM or voice-register fixture is read. */
void ff_audio_init_empty(void)
{
    memset(ff_spu_dma_state, 0, sizeof(ff_spu_dma_state));
    SpuInit();
    audio_initialized = 1;
    ff_spu_common_state[0] = ff_spu_common_state[1] = 0x3fff;
    ff_spu_common_state[2] = ff_spu_common_state[3] = ff_spu_common_state[4] = 0;
    memset(ff_spu_reverb_registers, 0, sizeof(ff_spu_reverb_registers));
    memset(ff_spu_reverb_state, 0, sizeof(ff_spu_reverb_state));
    memset(ff_spu_reverb_channels_state, 0, sizeof(ff_spu_reverb_channels_state));
    ff_services.spu_key = key;
    ff_services.spu_voice_attributes = attributes;
    ff_services.spu_key_on_attributes = key_on_attributes;
    ff_services.play_effect = FUN_80056670;
}

uint32 ff_spu_transfer_address(uint32 address)
{
    uint32 alignment, units;
    if (address - 0x1010u > 0x7efe8u)
        return 0;
    if (r_u32(0x8008cfb4))
    {
        alignment = r_u32(0x8008cfbc);
        if (!alignment)
            ff_wip_stop(__FUNCTION__, __FILE__, __LINE__);
        if (address % alignment)
            address = (address + alignment) & ~r_u32(0x8008cfc0);
    }
    units = (address >> (r_u32(0x8008cfb8) & 31)) & 65535;
    w_u16(0x8008cfa8, (uint16)units);
    return units;
}

uint32 ff_spu_transfer_write(uint32 source, uint32 bytes)
{
    uint32 units, address, count, chunk, dma;
    if (bytes > 0x7eff0)
        bytes = 0x7eff0;
    if (!audio_initialized)
        ff_wip_stop(__FUNCTION__, __FILE__, __LINE__);
    dma = !r_u32(0x8008cfac);
    if (dma)
    {
        uint32 shift;
        units = r_u16(0x8008cfa8);
        shift = r_u32(0x8008cfb8) & 31;
        units = (units << shift) >> shift;
        w_u16(0x8008cfa8, units);
        w_u32(0x8008cfe0, 0);
        w_u32(0x8008cfe4, source);
        count = (bytes + 63) >> 6;
        w_u32(0x8008cfe8, count);
        /* Zero in the DMA block-count field represents 65536 blocks */
        count = (count ? count : 65536) << 6;
    }
    else
    {
        units = r_u16(0x8008cfa8);
        count = (bytes + 1) & ~1;
    }
    /* Native synchronous transport retains DMA64 and PIO2 guest reads */
    address = units << 3;
    if (dma)
        slus_spu_upload(source & 0x1ffffc, count, address);
    else
    {
        xport_audio_lock();
        while (count)
        {
            chunk = 0x80000 - address;
            if (chunk > count)
                chunk = count;
            if (!spu_upload(address, psx_addr(source, chunk), chunk))
                ff_wip_stop(__FUNCTION__, __FILE__, __LINE__);
            source += chunk;
            count -= chunk;
            address = 0;
        }
        xport_audio_unlock();
    }
    if (!r_u32(0x8008cfc8))
        w_u32(0x8008cfc4, 0);
    return bytes;
}

void ff_spu_transfer_wait(void)
{
    /* Synchronous host upload replaces the original completion-event wait. */
    if (r_u32(0x8008cb2c) != 1 && r_u32(0x8008cfc4) != 1)
        w_u32(0x8008cfc4, 1);
}

sint32 ff_audio_init(void)
{
    static uint8 sample_ram[0x80000];
    uint16 regs[194];
    SpuCommonAttr common;
    FILE *f;
    int i;
    f = xport_fopen("FF-menu.spu", "rb");
    if (!f)
        return 0;
    if (fread(sample_ram, 1, sizeof(sample_ram), f) != sizeof(sample_ram))
    {
        fclose(f);
        return 0;
    }
    fclose(f);
    f = xport_fopen("FF-menu.spu-regs", "rb");
    if (!f)
        return 0;
    if (fread(regs, 1, sizeof(regs), f) != sizeof(regs))
    {
        fclose(f);
        return 0;
    }
    fclose(f);
    SpuInit();
    audio_initialized = 1;
    spu_upload(0, sample_ram, sizeof(sample_ram));
    memset(&common, 0, sizeof(common));
    common.mask = SPU_COMMON_MVOLL | SPU_COMMON_MVOLR;
    common.mvol.left = (sint16)regs[0];
    common.mvol.right = (sint16)regs[1];
    SpuSetCommonAttr(&common);
    for (i = 0; i < 24; i++)
    {
        uint16 *v = regs + 2 + i * 8;
        uint16 voice_registers[7] = {v[0], v[1], v[2], v[3], v[4], v[5], v[7]};
        ff_spu_voice_seed(i, voice_registers);
    }
    ff_services.spu_key = key;
    ff_services.spu_voice_attributes = attributes;
    ff_services.spu_key_on_attributes = key_on_attributes;
    ff_services.play_effect = FUN_80056670;
    return 1;
}

#include "diagnostic_state.h"

int ff_audio_state_io(FILE *f, int load)
{
    int result;
    if (!audio_initialized)
        return 0;
    xport_audio_lock();
    result = FF_STATE(f, ff_spu_common_state, load) && FF_STATE(f, ff_spu_reverb_registers, load) && FF_STATE(f, ff_spu_reverb_state, load) && FF_STATE(f, ff_spu_reverb_channels_state, load) && FF_STATE(f, ff_spu_dma_state, load) && spu_state_io(f, load);
    xport_audio_unlock();
    return result;
}
