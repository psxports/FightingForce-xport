#include "ff.h"
#include "psx.h"
#include <string.h>

/* 5F558: raw gameplay buttons; the first mapping wins if both IDs match. */
GDB_CALL uint32 FUN_8005F558(uint32 controller)
{
    uint32 value;
    if (controller == r_u32(0x80094288))
        value = r_u32(0x80093a10);
    else if (controller == r_u32(0x8009428c))
        value = r_u32(0x80093a14);
    else
        return 0;
    w_u8(0x80094114, (uint8)(value >> 16));
    return value & 65535;
}

static uint32 gameplay_action_address(uint32 player, uint32 mapping)
{
    sint32 config = r_s8(0x80094480 + player);
    sint32 action = r_s8(mapping + 6u * (uint32)config);
    return 0x80094100 + 6u * player + (uint32)action;
}

/* 5F8F0: decay six action bytes, then latch new face-button edges for12 ticks.
 * Preserve write order and the original address-or-zero return register. */
GDB_CALL uint32 FUN_8005F8F0(uint32 player, uint32 buttons)
{
    uint32 base = 0x80094100 + 6u * player, i, address;
    uint8 value;
    for (i = 0; i < 6; i++)
    {
        value = r_u8(base + i);
        if (value)
            w_u8(base + i, (uint8)(value - 1));
    }
    if (buttons & 0x8000)
    {
        address = gameplay_action_address(player, 0x80093d7a);
        w_u8(address, 12);
    }
    if (buttons & 0x4000)
    {
        address = gameplay_action_address(player, 0x80093d78);
        w_u8(address, 12);
    }
    if (buttons & 0x2000)
    {
        address = gameplay_action_address(player, 0x80093d79);
        w_u8(address, 12);
    }
    if (buttons & 0x1000)
    {
        address = gameplay_action_address(player, 0x80093d7b);
        w_u8(address, 12);
        return address;
    }
    return 0;
}

GDB_CALL uint32 FUN_8005FA58(uint32 player, uint32 buttons)
{
    uint32 address;
    if (buttons & 0x400)
    {
        address = gameplay_action_address(player, 0x80093d7c);
        w_u8(address, 1);
    }
    if (buttons & 0x800)
    {
        address = gameplay_action_address(player, 0x80093d7d);
        w_u8(address, 1);
        return address;
    }
    return 0;
}

uint32 ff_host_pad_state[6] = {0x80094748, 34, 0x80094770, 34, 1, 0};

/* InitPAD host boundary: register neutral disconnected buffers; host frame
 * polling supplies actual digital input before any game input decoder runs. */
void ff_host_pad_init(uint32 first, uint32 first_size, uint32 second, uint32 second_size)
{
    PadInit(0);
    PadInitDirect(first_size >= 8 ? psx_addr(first, 8) : 0, second_size >= 8 ? psx_addr(second, 8) : 0);
    ff_host_pad_state[0] = first;
    ff_host_pad_state[1] = first_size;
    ff_host_pad_state[2] = second;
    ff_host_pad_state[3] = second_size;
    ff_host_pad_state[4] = 1;
    ff_host_pad_state[5] = 0;
}

void ff_host_pad_publish(uint32 connected, uint32 first, uint32 second)
{
    uint32 i, buttons[2] = {first, second}, packed = 0xffffffffu;
    if (!ff_host_pad_state[4])
        return;
    for (i = 0; i < 2; ++i)
    {
        pad_publish((sint32)i, connected > i, (uint16)~buttons[i]);
        if (connected > i)
        {
            uint32 bits = (~(((buttons[i] & 255) << 8) | ((buttons[i] >> 8) & 255))) & 65535;
            packed = (packed & ~(65535u << (16 * i))) | (bits << (16 * i));
        }
    }
    if (ff_host_pad_state[5])
        w_u32(ff_host_pad_state[5], packed);
}

static sint32 host_pad_high_init(uint32 type, uint32 buffer)
{
    if (type != 0x20000000 && type != 0x20000001)
        return 0;
    ff_host_pad_init(0x801fd000, 34, 0x801fd040, 34);
    ff_host_pad_state[5] = buffer;
    return 2;
}

GDB_CALL void FUN_8006CE70(void)
{
    FUNCTION_MARKER(0x8006ce70u, "GAME.EXE");
    ff_host_pad_state[4] = 0;
}

GDB_CALL sint32 FUN_8006D1A0(uint32 first, uint32 first_size, uint32 second, uint32 second_size)
{
    FUNCTION_MARKER(0x8006d1a0u, "GAME.EXE");
    ff_host_pad_init(first, first_size, second, second_size);
    return 1;
}

GDB_CALL void FUN_8006D1B0(void)
{
    FUNCTION_MARKER(0x8006d1b0u, "GAME.EXE");
    ff_host_pad_state[4] = 1;
}

GDB_CALL sint32 FUN_8006D1C0(uint32 type, uint32 buffer)
{
    FUNCTION_MARKER(0x8006d1c0u, "GAME.EXE");
    return host_pad_high_init(type, buffer);
}

GDB_CALL sint32 FUN_SLUS_80013A6C(uint32 first, uint32 first_size, uint32 second, uint32 second_size)
{
    FUNCTION_MARKER(0x80013a6cu, "SLUS_004.33");
    ff_host_pad_init(first, first_size, second, second_size);
    return 1;
}

GDB_CALL void FUN_SLUS_80013A7C(void)
{
    FUNCTION_MARKER(0x80013a7cu, "SLUS_004.33");
    ff_host_pad_state[4] = 1;
}

GDB_CALL void FUN_SLUS_80013A8C(void)
{
    FUNCTION_MARKER(0x80013a8cu, "SLUS_004.33");
    ff_host_pad_state[4] = 0;
}

GDB_CALL sint32 FUN_SLUS_80013A9C(uint32 type, uint32 buffer)
{
    FUNCTION_MARKER(0x80013a9cu, "SLUS_004.33");
    return host_pad_high_init(type, buffer);
}

uint32 FUN_8005FB00(sint32 player)
{
    uint32 address = 0x80094100 + 6u * (uint32)player;
    if (address)
        memset(psx_addr(address, 6), 0, 6);
    return address;
}

/* 80068114: mapped controller edges, deliberately does not write 800927A0. */
GDB_CALL uint32 FUN_80068114(sint32 player)
{
    FUNCTION_MARKER(0x80068114, "GAME.EXE");
    if (ff_services.audit_input_event)
        ff_services.audit_input_event(2);
    uint32 offset = (uint32)player * 4, raw = (uint32)FUN_8005F5AC((sint32)r_u32(0x80094288 + offset));
    uint32 edge = (raw ^ r_u32(0x80092790 + offset)) & raw;
    w_u32(0x80092798 + offset, edge);
    w_u32(0x80092790 + offset, raw);
    return r_u32(0x80092798 + offset);
}

#include <stdlib.h>
#include <string.h>
FF_SERVICES ff_services;

/* GAME.EXE:800573A8. The second pad is counted only if the first is valid. */
GDB_CALL sint32 FUN_800573A8(void)
{
    FUNCTION_MARKER(0x800573a8, "GAME.EXE");
    uint32 pad = r_u32(0x8009355c);
    if (((r_u8(pad + 1) >> 4) != 4 && (r_u8(pad + 1) >> 4) != 7 && (r_u8(pad + 1) >> 4) != 5) || r_u8(r_u32(0x8009355c)) == 255)
        return 0;
    pad = r_u32(0x80093560);
    return (((r_u8(pad + 1) >> 4) == 4 || (r_u8(pad + 1) >> 4) == 7 || (r_u8(pad + 1) >> 4) == 5) && r_u8(r_u32(0x80093560)) != 255) ? 2 : 1;
}

/* GAME.EXE:8005EC90. Return is always one, not the controller count. */
GDB_CALL sint32 FUN_8005EC90(void)
{
    w_u32(0x8009400c, (uint32)FUN_800573A8());
    w_u32(0x80094288, 0);
    w_u32(0x8009428c, 1);
    return 1;
}

/* GAME.EXE:80056ECC. SDK SpuSetKey(0, all voices), then memset of voice flags.
 * Runtime SPU binding is WIP; no sound dummy is installed. */
GDB_CALL uint32 FUN_80056ECC(void)
{
    FUNCTION_MARKER(0x80056ecc, "GAME.EXE");
    uint32 i;
    if (!ff_services.spu_key)
        ff_wip_stop(__FUNCTION__, __FILE__, __LINE__);
    ff_services.spu_key(0, 0xffffffu);
    for (i = 0; i < 24; i++)
        w_u8(0x80094708 + i, 0);
    return 0x80094708;
}

/* GAME.EXE:8005F5AC. Complete input decoder including grace counters and reset.
 * The reset path does not return: IDA's post-longjmp store is unreachable. */
GDB_CALL sint32 FUN_8005F5AC(sint32 controller)
{
    FUNCTION_MARKER(0x8005f5ac, "GAME.EXE");
    ff_audit_input_packet(controller);
    /* Apply recorded buttons at the original decode-time load */
    if (ff_services.audit_input_event)
        ff_services.audit_input_event(0);
    uint32 index = (uint32)controller, pad = controller ? 0x80094770 : 0x80094748;
    uint32 disconnect = 0x80093a1c + index, unsupported = 0x80093a20 + index;
    sint16 buttons;
    sint32 decoded_type;
    uint8 type = r_u8(pad + 1) >> 4;
    w_u8(0x80094114, type);
    buttons = (sint16)~r_u16(pad + 2);
    if (buttons == -1)
        buttons = 0;
    if (r_u8(pad) == 255)
    {
        type = 255;
        if (r_s8(disconnect) < 0)
        {
            w_u8(disconnect, (uint8)(r_u8(disconnect) + 1));
            type = 4;
        }
        w_u8(0x80094114, type);
        buttons = 0;
    }
    else
        w_u8(disconnect, 0);
    decoded_type = r_s8(0x80094114);
    if (decoded_type == 4)
        w_u8(unsupported, 0);
    else if (decoded_type == 7 || decoded_type == 5)
    {
        sint32 raw_x, x, y;
        w_u8(unsupported, 0);
        w_u8(0x80094114, 22);
        raw_x = r_u8(pad + 6);
        x = raw_x - 128;
        w_u32(0x800945d4, (uint32)x);
        y = (sint32)r_u8(pad + 7) - 128;
        w_u32(0x800945f8, (uint32)y);
        if (x > 0)
        {
            x = raw_x - 160;
            w_u32(0x800945d4, (uint32)x);
            if (x < 0)
                w_u32(0x800945d4, 0);
        }
        else
        {
            x = (sint32)(r_u32(0x800945d4) + 32);
            w_u32(0x800945d4, (uint32)x);
            if (x > 0)
                w_u32(0x800945d4, 0);
        }
        y = r_s32(0x800945f8);
        if (y > 0)
        {
            y = (sint32)((uint32)y - 32);
            w_u32(0x800945f8, (uint32)y);
            if (y < 0)
                w_u32(0x800945f8, 0);
        }
        else
        {
            y = (sint32)(r_u32(0x800945f8) + 32);
            w_u32(0x800945f8, (uint32)y);
            if (y > 0)
                w_u32(0x800945f8, 0);
        }
        x = (sint32)(r_u32(0x800945d4) * 341) >> 8;
        w_u32(0x800945d4, (uint32)x);
        y = (sint32)(r_u32(0x800945f8) * 341) >> 8;
        w_u32(0x800945f8, (uint32)y);
    }
    else
    {
        type = 255;
        if (r_s8(unsupported) < 15)
        {
            w_u8(unsupported, (uint8)(r_u8(unsupported) + 1));
            type = 4;
        }
        w_u8(0x80094114, type);
        buttons = 0;
    }
    if ((buttons & 9) == 9 && !r_s8(0x80093cf8))
    {
        sint16 count = (sint16)(r_u16(0x80093a18 + 2 * index) + 1);
        w_u16(0x80093a18 + 2u * index, (uint16)count);
        if (count >= 31)
        {
            w_u16(0x80093a1a, 0);
            w_u16(0x80093a18, 0);
            FUN_80056ECC();
            w_u32(0x80093c14, 0xffffffffu);
            w_u8(0x80093cf8, 1);
            if (!ff_services.game_longjmp)
                ff_wip_stop(__FUNCTION__, __FILE__, __LINE__);
            ff_services.game_longjmp(0x80094298, 0);
            ff_wip_stop(__FUNCTION__, __FILE__, __LINE__);
        }
    }
    else
        w_u16(0x80093a18 + 2u * index, 0);
    return buttons;
}
