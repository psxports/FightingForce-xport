#include "ff.h"
#include <stdio.h>

GDB_CALL uint32 FUN_80057788(uint32 first, uint32 second)
{
    FUNCTION_MARKER(0x80057788u, "GAME.EXE");
    uint32 start[3], end[3], step[3], current[3], point[3], tx, ty, tz;
    sint32 previous[4], projected[4];
    uint16 packet[9];
    sint32 count, index, depth, old_depth, width, new_width;
    int i, j;
    static const uint16 styles[4] = {1373, 827, 1647, 553};
    for (i = 0; i < 3; ++i)
    {
        start[i] = r_u32(first + 4 * i);
        current[i] = start[i];
    }
    for (i = 0; i < 3; ++i)
        end[i] = r_u32(second + 4 * i);
    count = FUN_800163F4((sint32)start[0] >> 16, (sint32)start[2] >> 16, (sint32)end[0] >> 16, (sint32)end[2] >> 16) >> 8;
    if (count < 3)
        count = 3;
    for (i = 0; i < 3; ++i)
        step[i] = (uint32)((sint32)(end[i] - start[i]) / count) << 1;
    FUN_80010040();
    tx = r_u32(0x8008d4a4);
    ty = r_u32(0x8008d4a8);
    tz = r_u32(0x8008d4ac);
    FUN_800100C4((sint32)(start[0] - tx), (sint32)(start[1] - ty), (sint32)(start[2] - tz));
    ff_project_local(0, 0, 0, previous);
    FUN_80010028();
    for (index = 1; index <= count; ++index)
    {
        for (i = 0; i < 3; ++i)
            current[i] += step[i];
        if (index == count)
        {
            for (i = 0; i < 3; ++i)
                point[i] = end[i];
        }
        else
            for (i = 0; i < 3; ++i)
                point[i] = current[i] + (((ff_rand_8006D348() & 127) - 64u) << 17);
        FUN_80010040();
        tx = r_u32(0x8008d4a4);
        ty = r_u32(0x8008d4a8);
        tz = r_u32(0x8008d4ac);
        FUN_800100C4((sint32)(point[0] - tx), (sint32)(point[1] - ty), (sint32)(point[2] - tz));
        ff_project_local(0, 0, 0, projected);
        FUN_80010028();
        if (previous[3] < 4 && projected[3] < 4)
        {
            old_depth = previous[2];
            depth = projected[2];
            if (!old_depth || !depth)
                ff_wip_stop(__FUNCTION__, __FILE__, __LINE__); /* Original57A2C/57A60 BREAK */
            width = 65536 / old_depth;
            new_width = 65536 / depth;
            packet[2] = (uint16)previous[0];
            packet[3] = (uint16)((uint16)previous[1] - (uint32)width);
            packet[4] = (uint16)projected[0];
            packet[5] = (uint16)((uint16)projected[1] - (uint32)new_width);
            packet[6] = (uint16)projected[0];
            packet[7] = (uint16)((uint16)projected[1] + (uint32)new_width);
            packet[0] = (uint16)previous[0];
            packet[1] = (uint16)((uint16)previous[1] + (uint32)width);
            if (index != 1 && index != count)
                ff_rand_8006D348(); /* Unused result is still an RNG advance */
            packet[8] = 831;
            depth = projected[2] >> 2;
            packet[0] = (uint16)projected[0];
            packet[1] = (uint16)projected[1];
            for (j = 0; j < 4; ++j)
            {
                int target = j & 1 ? 0 : 2, base = j & 1 ? 2 : 0;
                sint32 delta;
                delta = FUN_80057700(depth);
                packet[target] = (uint16)(packet[base] + (uint32)delta);
                delta = FUN_80057700(depth);
                packet[target + 1] = (uint16)(packet[base + 1] + (uint32)delta);
                packet[8] = styles[j];
                ff_stack_color_80064638(packet);
            }
        }
        for (i = 0; i < 4; ++i)
            previous[i] = projected[i];
    }
    return 1;
}

GDB_CALL uint32 FUN_80046EF0(uint32 object)
{
    FUNCTION_MARKER(0x80046ef0u, "GAME.EXE");
    uint32 kind = r_u32(object), bias, state;
    sint32 angle = r_s16(object + 16), x, z;
    if (angle)
        FUN_8001033C(angle);
    angle = r_s16(object + 20);
    if (angle)
        FUN_800104A0(angle);
    angle = r_s16(object + 18);
    if (angle)
        FUN_800101CC(angle);
    bias = kind - 207u < 4u || kind == 214 || kind == 211 ? 512 : 0;
    state = r_u32(0x80093d60);
    if (!state)
    {
        z = (sint32)r_u32(object + 12);
        if (z <= (sint32)0xf2900000)
            goto draw_model;
        state = r_u32(0x80093d60);
        if (!state)
        {
            if (r_u32(0x80094674))
                goto test_front;
            state = r_u32(0x80093d60);
        }
    }
    if (state - 1 < 2)
    {
        z = (sint32)r_u32(object + 12);
        goto test_front;
    }
    goto select_middle;

test_front:
    if (z > (sint32)0xf2900000)
    {
        x = (sint32)r_u32(object + 4);
        if (x > 0x06000000)
            goto draw_model;
    }
select_middle:
    state = r_u32(0x80093d60);
    if ((state == 2 && r_u32(0x80094674)) || state - 3 < 2 || state == 5)
    {
        x = (sint32)r_u32(object + 4);
        if ((uint32)x + 0x0dffffff <= 0x13fffffe)
            goto draw_model;
    }
    state = r_u32(0x80093d60);
    if (state == 4 && r_u32(0x80094674))
        goto test_left;
    state = r_u32(0x80093d60);
    if (state - 5 >= 2)
        return 0;
test_left:
    x = (sint32)r_u32(object + 4);
    if (x >= (sint32)0xf2000000)
        return 0;

draw_model:
    return ff_static_model_80010B18(r_u32(0x80081720 + 4u * r_u32(object)), 0, bias);
}

GDB_CALL uint32 FUN_8002A640(uint32 object)
{
    FUNCTION_MARKER(0x8002a640u, "GAME.EXE");
    sint32 amplitude, scale;
    uint32 offset;
    if (r_u32(0x80093d58) == 18 && (sint32)r_u32(object + 4) < (sint32)0xe5300000u)
        return 1;
    FUN_80010040();
    FUN_800100C4(0, 0x02000000, 0);
    FUN_8001033C(r_s16(object + 16));
    FUN_800104A0(-r_s16(object + 20));
    FUN_800101CC(r_s16(object + 18));
    amplitude = (sint32)(0u - r_u32(object + 8)) >> 18;
    if (amplitude > 64)
        amplitude = 64;
    offset = (r_u32(0x80093dd0) << 4) & 0xff0u;
    FUN_800100C4(0, (sint32)((uint32)(sint32)r_s16(0x800802d0 + offset) * (uint32)amplitude), 0);
    ff_static_model_80010B18(r_u32(0x80081c74), 0, 0);
    FUN_800100C4(0, 0x00100000, 0);
    FUN_8001033C((sint32)((uint32)r_u16(0x80093dd0) << 12));
    ff_static_model_80010B18(r_u32(0x80081ff0), 0, 0);
    FUN_80010028();
    if (r_u32(0x80093d58) == 18)
        return 18;
    FUN_80010040();
    FUN_8001033C(r_s16(object + 16));
    FUN_800100C4(0, (sint32)r_u32(object + 8), (sint32)0xfec00000u);
    scale = ((sint32)r_u32(object + 8) >> 14) + 16384;
    FUN_80010F94(scale, scale, scale);
    ff_static_model_80010B18(r_u32(0x80082428), 0, 0);
    return FUN_80010028();
}

GDB_CALL uint32 FUN_8002C55C(uint32 object)
{
    FUNCTION_MARKER(0x8002c55cu, "GAME.EXE");
    uint32 x, y, z, color, offset, camera_x, camera_y, camera_z;
    sint32 shade, wave, angle;
    FUN_80010040();
    y = r_u32(object + 8);
    offset = ((uint32)(sint32)r_s16(object + 26) << 3) & 0xff8;
    wave = r_s16(0x800806d0 + offset);
    z = r_u32(object + 12) - ((uint32)wave << 9);
    x = r_u32(object + 4);
    camera_x = r_u32(0x8008d4a4);
    camera_y = r_u32(0x8008d4a8);
    camera_z = r_u32(0x8008d4ac);
    FUN_800100C4((sint32)(x - camera_x), (sint32)(y - camera_y), (sint32)(z - camera_z));
    FUN_800101CC(32768);
    angle = r_s16(object + 18);
    if (angle)
        FUN_800101CC(angle);
    angle = r_s16(object + 20);
    if (angle)
        FUN_800104A0(angle);
    shade = ((sint32)z >> 19) - 450;
    if (shade < 0)
        shade = 0;
    else
        shade >>= 2;
    if (shade > 128)
        shade = 128;
    color = 128u - (uint32)shade;
    w_u32(0x8008d4bc, 0x2c000000u + color * 0x10101u);
    ff_static_model_80010B18(r_u32(0x80081720 + 4u * r_u32(object)), 0, 0);
    return FUN_80010028();
}

GDB_CALL uint32 FUN_800454E0(uint32 object)
{
    FUNCTION_MARKER(0x800454e0u, "GAME.EXE");
    sint32 angle;
    FUN_8001033C(r_s16(object + 16));
    angle = r_s16(object + 20);
    if (angle)
        FUN_800104A0(angle);
    angle = r_s16(object + 18);
    if (angle)
        FUN_800101CC(angle);
    FUN_800100C4((sint32)0xfb000000u, 0, 0);
    FUN_80011594(r_u32(0x80081720 + 4u * r_u32(object)), 0, 0);
    FUN_800100C4(0x05000000, 0, 0);
    FUN_80011594(r_u32(0x80081720 + 4u * r_u32(object)), 0, 0);
    FUN_800100C4(0x05000000, 0, 0);
    return FUN_80011594(r_u32(0x80081720 + 4u * r_u32(object)), 0, 0);
}

GDB_CALL uint32 FUN_8005B404(uint32 object)
{
    FUNCTION_MARKER(0x8005b404u, "GAME.EXE");
    sint32 phase = r_s16(object + 28), height, angle;
    if (phase >= 3)
        phase = 2;
    height = r_s16(0x800802d0 + ((uint32)phase << 9)) >> 9;
    w_u16(object + 26, 0);
    w_u16(object + 22, 0);
    angle = r_s16(object + 16);
    if (angle)
        FUN_8001033C(angle);
    ff_static_model_80010B18(r_u32(0x80081830), 0, 8192);
    FUN_800100C4(0, (sint32)((64u - (uint32)height) << 16), 0);
    angle = r_s16(object + 20);
    if (angle)
        FUN_800104A0(angle);
    angle = r_s16(object + 18);
    if (angle)
        FUN_800101CC(angle);
    return ff_static_model_80010B18(r_u32(0x80081720 + 4u * r_u32(object)), 0, 0);
}

GDB_CALL uint32 FUN_80032C4C(uint32 object)
{
    FUNCTION_MARKER(0x80032c4cu, "GAME.EXE");
    FUN_80010040();
    if ((sint32)r_u32(object + 4) > 0x13000000)
        FUN_8001033C(32768);
    FUN_80011594(r_u32(0x80081720 + 4u * r_u32(object)), 0, 0);
    FUN_800100C4(0, 0, (sint32)0xf1a00000u);
    FUN_80011594(r_u32(0x80081bdc), 0, 0);
    FUN_800100C4(0, 0, 0x07300000);
    FUN_80011594(r_u32(0x80081be0), 0, 0);
    FUN_800100C4(0, 0, 0x0e600000);
    FUN_80011594(r_u32(0x80081bdc), 0, 0);
    FUN_800100C4(0, 0, 0x07300000);
    FUN_80011594(r_u32(0x80081be0), 0, 0);
    return FUN_80010028();
}

GDB_CALL uint32 FUN_8005B598(uint32 object)
{
    FUNCTION_MARKER(0x8005b598u, "GAME.EXE");
    sint32 phase = r_s16(object + 28), height, angle;
    if (phase >= 3)
        phase = 2;
    height = (5 * r_s16(0x800802d0 + ((uint32)phase << 9))) >> 12;
    w_u16(object + 26, 0);
    w_u16(object + 22, 0);
    angle = r_s16(object + 16);
    if (angle)
        FUN_8001033C(angle);
    FUN_800100C4(0, (sint32)((40u - (uint32)height) << 16), 0);
    angle = r_s16(object + 20);
    if (angle)
        FUN_800104A0(angle);
    angle = r_s16(object + 18);
    if (angle)
        FUN_800101CC(angle);
    return ff_static_model_80010B18(r_u32(0x80081720 + 4u * r_u32(object)), 0, 0);
}

GDB_CALL uint32 FUN_8005AD54(uint32 object)
{
    FUNCTION_MARKER(0x8005ad54u, "GAME.EXE");
    uint32 kind = r_u32(object);
    sint32 angle = r_s16(object + 16);
    if (angle)
        FUN_8001033C(angle);
    FUN_80010040();
    FUN_800100C4(0, (sint32)r_u32(object + 8), 0);
    ff_static_model_80010B18(r_u32(0x80081720 + 4u * (kind == 87 ? 89u : 106u)), 0, 8192);
    FUN_80010028();
    angle = r_s16(object + 20);
    if (angle)
        FUN_800104A0(angle);
    angle = r_s16(object + 18);
    if (angle)
        FUN_800101CC(angle);
    return ff_static_model_80010B18(r_u32(0x80081720 + 4u * r_u32(object)), 0, 0);
}

GDB_CALL uint32 FUN_8002CAC4(uint32 object)
{
    FUNCTION_MARKER(0x8002cac4u, "GAME.EXE");
    FUN_80010040();
    FUN_8001033C((sint32)((uint32)(sint32)r_s16(object + 16) << 5));
    FUN_800104A0(-r_s16(object + 20));
    FUN_800101CC(r_s16(object + 18));
    ff_static_model_80010B18(r_u32(0x80081968), 0, 0);
    FUN_80010040();
    FUN_800101CC((sint32)((uint32)(sint32)r_s16(object + 18) << 3));
    ff_static_model_80010B18(r_u32(0x8008196c), 0, 0);
    FUN_80010028();
    if ((sint32)r_u32(0x80093dd0) >= 631)
    {
        FUN_800100C4(0, (sint32)0xfe9a0000u, 0x1000000);
        FUN_8001033C(16384);
        ff_static_model_80010B18(r_u32(0x80081970), 0, 0);
    }
    return FUN_80010028();
}

static void object_rotations(uint32 object);

GDB_CALL uint32 FUN_80032818(uint32 object)
{
    FUNCTION_MARKER(0x80032818u, "GAME.EXE");
    uint32 kind = r_u32(object), state, mode;
    sint32 x, z, bias;
    object_rotations(object);
    bias = (kind - 207u < 4u || kind == 214 || kind == 211) ? 512 : 0;
    state = r_u32(0x80093d60);
    if (!state)
    {
        z = (sint32)r_u32(object + 12);
        if (z <= (sint32)0xf2900000)
            goto draw_model;
        state = r_u32(0x80093d60);
        if (!state)
        {
            mode = r_u32(0x80094674);
            if (mode)
                goto test_front;
        }
    }
    state = r_u32(0x80093d60);
    if (state - 1 < 2 || state == 3)
    {
        z = (sint32)r_u32(object + 12);
        goto test_front;
    }
    goto select_middle;

test_front:
    if (z > (sint32)0xf2900000)
    {
        x = (sint32)r_u32(object + 4);
        if (x > 0x06000000)
            goto draw_model;
    }
    state = r_u32(0x80093d60);
select_middle:
    if (state == 2)
    {
        mode = r_u32(0x80094674);
        if (mode)
            goto test_middle;
    }
    state = r_u32(0x80093d60);
    if (state - 3 < 2 || state == 5)
        goto test_middle;
    goto select_left;

test_middle:
    x = (sint32)r_u32(object + 4);
    if ((uint32)x + 0x0dffffff <= 0x13fffffe)
        goto draw_model;
    state = r_u32(0x80093d60);
select_left:
    if (state == 4)
    {
        mode = r_u32(0x80094674);
        if (mode)
            goto test_left;
    }
    state = r_u32(0x80093d60);
    if (state - 5 >= 2)
        return 0;
test_left:
    x = (sint32)r_u32(object + 4);
    if (x >= (sint32)0xf2000000)
        return 0;

draw_model:
    return ff_static_model_80010B18(r_u32(0x80081720 + 4u * r_u32(object)), 0, bias);
}

GDB_CALL uint32 FUN_80032A48(uint32 object)
{
    FUNCTION_MARKER(0x80032a48u, "GAME.EXE");
    sint32 x = r_s16(object + 6);
    sint32 z = r_s16(object + 14);
    sint32 player_x = r_s16(0x800b4342);
    sint32 player_z = r_s16(0x800b434a);
    sint16 angle = (sint16)FUN_8001614C(x, z, player_x, player_z);
    w_u16(object + 16, (uint16)angle);
    FUN_8001033C(angle);
    return FUN_80032818(object);
}

GDB_CALL uint32 FUN_80032D70(uint32 object)
{
    FUNCTION_MARKER(0x80032d70u, "GAME.EXE");
    if ((sint32)r_u32(0x80094674) <= 0 && !FUN_800473B0((sint32)0xee000000u))
        return 0;
    object_rotations(object);
    return ff_static_model_80010B18(r_u32(0x80081720 + 4u * r_u32(object)), 0, 0);
}

GDB_CALL sint32 FUN_80032E20(uint32 object)
{
    FUNCTION_MARKER(0x80032e20u, "GAME.EXE");
    sint32 result, angle = r_s16(object + 16);
    if (angle)
        FUN_8001033C(angle);
    FUN_80011594(r_u32(0x80081720 + 4u * r_u32(object)), 0, 0);
    FUN_800100C4(0, (sint32)((uint32)(sint32)r_s16(object + 26) << 16), 0x00d00000);
    FUN_80011594(r_u32(0x80081d44), 0, 0);
    result = (sint16)r_u16(0x800802d0 + ((r_u32(0x80093dd0) << 5) & 0x7e0u)) >> 7;
    w_u16(object + 26, (uint16)result);
    return result;
}

static uint32 object_faded_model(uint32 object)
{
    object_rotations(object);
    return FUN_80011594(r_u32(0x80081720 + 4u * r_u32(object)), 0, 0);
}

GDB_CALL uint32 FUN_8005B71C(uint32 object)
{
    FUNCTION_MARKER(0x8005b71cu, "GAME.EXE");
    return object_faded_model(object);
}

GDB_CALL uint32 FUN_8005C0E8(uint32 object)
{
    FUNCTION_MARKER(0x8005c0e8u, "GAME.EXE");
    FUN_80012190();
    return object_faded_model(object);
}

GDB_CALL uint32 FUN_800428EC(uint32 object)
{
    FUNCTION_MARKER(0x800428ecu, "GAME.EXE");
    uint32 state = r_u32(0x80093d60);
    if ((sint32)state <= 0)
        return state;
    return object_faded_model(object);
}

GDB_CALL uint32 FUN_80042848(uint32 object)
{
    FUNCTION_MARKER(0x80042848u, "GAME.EXE");
    if (r_u32(0x80093d60) >= 2u)
        return 0;
    return object_faded_model(object);
}

GDB_CALL uint32 FUN_80032D44(void)
{
    FUNCTION_MARKER(0x80032d44u, "GAME.EXE");
    return FUN_80011594(r_u32(0x80081b84), 0, 1024);
}

GDB_CALL uint32 FUN_8005B844(uint32 object)
{
    FUNCTION_MARKER(0x8005b844u, "GAME.EXE");
    sint32 brightness = ((sint16)r_u16(0x800802d0 + ((r_u32(0x80093dd0) << 5) & 0x7e0u)) >> 9) + 65, point[4], distance, amount, scale;
    uint32 cutoff;
    ff_project_local(0, 0, 0, point);
    cutoff = r_u32(0x80011590);
    distance = (sint32)((uint32)point[2] - cutoff);
    if (distance > 0)
        return cutoff;
    distance = (sint32)((uint32)distance + 2048u);
    if (distance < 0)
        distance = 0;
    brightness = (sint32)((128u - (uint32)(distance >> 4)) * (uint32)brightness) >> 7;
    amount = (sint32)r_u32(0x80094338);
    if (amount > 0)
    {
        scale = (sint32)((uint32)amount << 14) / 624;
        FUN_80010040();
        FUN_80010F94(16384, scale, 16384);
        w_u32(0x8008d4bc, 0x2c000000u + (uint32)brightness * 0x10101u);
        ff_static_model_80010B18(r_u32(0x80081720 + 4u * r_u32(object)), 0, -64);
        FUN_80010028();
    }
    else
        scale = (sint32)((0u - r_u32(0x80094338)) << 14) / 624;
    FUN_800100C4(0, 0x02700000, 0);
    FUN_80010F94(16384, (sint32)(16384u - (uint32)scale), 16384);
    w_u32(0x8008d4bc, 0x2c000000u + ((uint32)brightness << 16) + (uint32)(brightness >> 1) * 257u);
    return ff_static_model_80010B18(r_u32(0x80081de8), 0, -64);
}

GDB_CALL uint32 FUN_8005BA28(uint32 object)
{
    FUNCTION_MARKER(0x8005ba28u, "GAME.EXE");
    sint32 point[4], distance;
    uint32 phase, x, z, cutoff, color;
    FUN_80010040();
    phase = (uint32)(sint32)r_s16(object + 26) + r_u32(0x80093dd0);
    x = ((uint32)(sint32)r_s16(0x800802d0 + ((phase << 6) & 0xfc0u)) << 5) + r_u32(object + 4);
    z = ((uint32)(sint32)r_s16(0x800802d0 + ((phase << 5) & 0xfe0u)) << 5) + r_u32(object + 12);
    FUN_800100C4((sint32)(x - r_u32(0x8008d4a4)), (sint32)(r_u32(object + 8) - r_u32(0x8008d4a8)), (sint32)(z - r_u32(0x8008d4ac)));
    ff_project_local(0, 0, 0, point);
    FUN_80010028();
    if (point[3])
        return (uint32)point[3];
    cutoff = r_u32(0x80011590);
    distance = (sint32)((uint32)point[2] - cutoff);
    if (distance > 0)
        return cutoff;
    distance = (sint32)((uint32)distance + 2048u);
    if (distance < 0)
        distance = 0;
    color = (16u - (uint32)(distance >> 7)) * 1057u;
    if (point[2] > 2560)
        return FUN_800121A8(point[0], point[1], point[2], color);
    if (point[2] < 1536)
        return FUN_800122E0(point[0], point[1], point[2], color);
    return FUN_80012240(point[0], point[1], point[2], color);
}

GDB_CALL uint32 FUN_8005CEE4(uint32 object)
{
    FUNCTION_MARKER(0x8005cee4u, "GAME.EXE");
    sint32 a[4], b[4];
    FUN_80010040();
    FUN_800100C4((sint32)(r_u32(object + 4) - r_u32(0x8008d4a4)), (sint32)(r_u32(object + 8) - r_u32(0x8008d4a8)), (sint32)(r_u32(object + 12) - r_u32(0x8008d4ac)));
    ff_project_local(0, 0, 0, a);
    if (!a[3])
    {
        ff_project_local(r_s16(object + 16), r_s16(object + 18), r_s16(object + 20), b);
        if (!b[3])
        {
            w_u32(0x8008d4bc, 0x2c303040);
            a[3] = 128;
            b[3] = 256;
            FUN_8006A1B4(b, a, r_s16(0x8009adce + 2u * (uint32)(sint32)r_s16(object + 22)));
            w_u32(0x8008d4bc, 0x2c181820);
            a[3] = 176;
            b[3] = 336;
            FUN_8006A1B4(b, a, r_s16(0x8009adce + 2u * (uint32)(sint32)r_s16(object + 22)));
        }
    }
    return FUN_80010028();
}

GDB_CALL uint32 FUN_8005D024(uint32 object)
{
    FUNCTION_MARKER(0x8005d024u, "GAME.EXE");
    sint32 a[4], b[4];
    uint32 color;
    FUN_80010040();
    FUN_800100C4((sint32)(r_u32(object + 4) - r_u32(0x8008d4a4)), (sint32)(r_u32(object + 8) - r_u32(0x8008d4a8)), (sint32)(r_u32(object + 12) - r_u32(0x8008d4ac)));
    ff_project_local(0, 0, 0, a);
    ff_project_local(r_s16(object + 16), r_s16(object + 18), r_s16(object + 20), b);
    a[2] = (sint32)((uint32)a[2] - 32u);
    b[2] = (sint32)((uint32)b[2] - 32u);
    color = r_u32(object + 28);
    w_u32(0x8008d4bc, color);
    w_u32(0x8008d4c0, (uint32)((sint32)color >> 24));
    w_u32(0x8008d4bc, (color & 0xffffffu) | 0x2c000000u);
    a[3] = r_s16(object + 24);
    b[3] = r_s16(object + 26);
    FUN_8006A1B4(a, b, r_s16(0x8009acc8 + 2u * (uint32)(sint32)r_s16(object + 22)));
    return FUN_80010028();
}

/* Original 72480/724F0 sequence, including COP2 bank and RGB FIFO effects */
static void ff_normalize_screen(sint32 x, sint32 y, sint16 output[2])
{
    sint32 sum, scaled, shift, factor;
    uint32 leading;
    int64_t total;
    SVECTOR input;
    VECTOR product;
    xport_gte_write_data(9, (uint32)x);
    xport_gte_write_data(10, (uint32)y);
    xport_gte_write_data(11, 0);
    xport_gte_execute(0xa00428);
    total = (int64_t)(sint32)xport_gte_read_data(25);
    total += (sint32)xport_gte_read_data(26);
    total += (sint32)xport_gte_read_data(27);
    if (total > 2147483647)
        ff_wip_stop(__FUNCTION__, __FILE__, __LINE__);
    sum = (sint32)total;
    xport_gte_write_data(30, (uint32)sum);
    leading = xport_gte_read_data(31) & ~1;
    shift = (31 - (sint32)leading) >> 1;
    scaled = leading >= 24 ? (sint32)((uint32)sum << ((leading - 24) & 31)) : sum >> ((24 - leading) & 31);
    factor = r_s16(0x800874b4 + ((uint32)scaled - 64) * 2);
    input.vx = (sint16)x;
    input.vy = (sint16)y;
    input.vz = 0;
    input.pad = 0;
    gte_gpf0(&input, factor, &product);
    output[0] = (sint16)(product.vx >> ((uint32)shift & 31));
    output[1] = (sint16)(product.vy >> ((uint32)shift & 31));
}

GDB_CALL uint32 FUN_8006A1B4(const sint32 *a, const sint32 *b, sint32 sprite)
{
    FUNCTION_MARKER(0x8006a1b4u, "GAME.EXE");
    uint32 packet = r_u32(0x8008d4b0), texture = 0x800a8768 + 12u * (uint32)sprite, depth = (uint32)((sint32)((uint32)a[2] + (uint32)b[2]) >> 3), color, texture_flags;
    sint16 normal[2];
    sint32 numerator, width, dx, dy, ot, uv_end, uv_start;
    uint32 pass;
    const sint32 *point;
    texture_flags = r_u32(texture);
    color = r_u32(0x8008d4bc);
    if (texture_flags & 1u)
        color |= 0x02000000u;
    w_u32(packet + 4, color);
    w_u32(packet + 12, r_u32(texture) & 0xfffffffeu);
    w_u32(0x8008d4bc, 0x2d000000);
    if (depth >= 2048)
        return 0;
    ff_normalize_screen((sint32)((uint32)b[0] - (uint32)a[0]), (sint32)((uint32)b[1] - (uint32)a[1]), normal);
    w_u32(0x8008d4b0, r_u32(0x8008d4b0) + 40);
    w_u32(packet + 20, r_u32(texture + 4));
    w_u16(packet + 28, (uint16)r_s16(texture + 8));
    w_u16(packet + 36, (uint16)r_s16(texture + 10));
    if (r_u32(0x8008d4c0))
    {
        uint32 packet_mode = r_u16(packet + 22);
        uint32 blend_mode = r_u16(0x8008d4c0);
        w_u16(packet + 22, (uint16)((packet_mode & 0xff9fu) | ((blend_mode - 1u) << 5)));
        w_u32(packet + 4, r_u32(packet + 4) | 0x02000000u);
    }
    for (pass = 0; pass < 2; pass++)
    {
        uint32 uv = texture + (pass ? 8u : 0u), p = packet + (pass ? 24u : 8u);
        point = pass ? b : a;
        uv_end = r_s8(uv + (pass ? 2u : 4u));
        uv_start = r_s8(uv);
        width = (uv_end - uv_start) >> 1;
        numerator = (sint32)((uint32)width * (uint32)point[3]);
        if (!point[2] || (point[2] == -1 && (uint32)numerator == 0x80000000u))
            ff_wip_stop(__FUNCTION__, __FILE__, __LINE__);
        width = numerator / point[2];
        dx = (sint32)((uint32)width * (uint32)(sint32)normal[1]) >> 12;
        dy = (sint32)((uint32)width * (uint32) - (sint32)normal[0]) >> 12;
        w_u16(p, (uint16)((uint32)point[0] - (uint32)dx));
        w_u16(p + 2, (uint16)((uint32)point[1] - (uint32)dy));
        w_u16(p + 8, (uint16)((uint32)point[0] + (uint32)dx));
        w_u16(p + 10, (uint16)((uint32)point[1] + (uint32)dy));
    }
    ot = r_s16(r_u32(0x8008d4d8) + 2 * depth) >> 2;
    return FUN_80011E28(r_u32(0x8008d4b4) + 4u * (uint32)ot, packet, 0x09000000);
}

GDB_CALL uint32 FUN_8005CDBC(uint32 object)
{
    FUNCTION_MARKER(0x8005cdbcu, "GAME.EXE");
    sint32 a[4], b[4];
    FUN_80010040();
    FUN_800100C4((sint32)(r_u32(object + 4) - r_u32(0x8008d4a4)), (sint32)(r_u32(object + 8) - r_u32(0x8008d4a8)), (sint32)(r_u32(object + 12) - r_u32(0x8008d4ac)));
    ff_project_local(0, 0, 0, a);
    ff_project_local(r_s16(object + 16), r_s16(object + 18), r_s16(object + 20), b);
    w_u32(0x8008d4bc, 0x2c303040);
    a[3] = 128;
    b[3] = 256;
    FUN_8006A1B4(b, a, r_s16(0x8009ad9c + 2u * (uint32)(sint32)r_s16(object + 22)));
    w_u32(0x8008d4bc, 0x2c181820);
    a[3] = 176;
    b[3] = 336;
    FUN_8006A1B4(b, a, r_s16(0x8009ad9c + 2u * (uint32)(sint32)r_s16(object + 22)));
    return FUN_80010028();
}

GDB_CALL uint32 FUN_8005DAD4(uint32 object)
{
    FUNCTION_MARKER(0x8005dad4u, "GAME.EXE");
    sint32 position[4];
    FUN_80010040();
    FUN_8005C740(object);
    ff_static_model_80010B18(r_u32(0x80081720 + 4u * r_u32(object)), 0, 0);
    if (r_s16(object + 22) > 0)
    {
        ff_project_local(0, 64, 0, position);
        if (!position[3])
        {
            position[3] = r_u32(0x80093dd0) & 1u ? 448 : 256;
            w_u32(0x8008d4bc, 0x2cffffffu);
            FUN_80012594(position, r_s16(0x8009ad4c));
        }
        if (r_s16(object + 22) > 0)
            FUN_8002BDD0(object);
    }
    return FUN_80010028();
}

GDB_CALL uint32 FUN_8005D1FC(uint32 object)
{
    FUNCTION_MARKER(0x8005d1fcu, "GAME.EXE");
    sint32 position[4];
    uint32 kind;
    FUN_80010040();
    FUN_8005C740(object);
    ff_project_local(0, 0, 0, position);
    if (!position[3])
    {
        kind = r_u32(object);
        if (kind == 892)
            position[3] = 2048;
        else if (kind == 893)
            position[3] = 1024;
        else if (kind == 894)
            position[3] = 256;
        else if (kind == 895)
            position[3] = 128;
        w_u32(0x8008d4bc, 0x2c404040u);
        FUN_80012594(position, r_s16(0x8009ad5c + 2u * (uint32)(r_s16(object + 22) >> 1)));
    }
    return FUN_80010028();
}

GDB_CALL uint32 FUN_8005C598(uint32 object)
{
    FUNCTION_MARKER(0x8005c598u, "GAME.EXE");
    sint32 position[4];
    FUN_80010040();
    FUN_800100C4((sint32)(r_u32(object + 4) - r_u32(0x8008d4a4)), (sint32)(r_u32(object + 8) - r_u32(0x8008d4a8)), (sint32)(r_u32(object + 12) - r_u32(0x8008d4ac)));
    ff_project_local(0, 0, 0, position);
    if (!position[3])
        FUN_800121A8(position[0], position[1], position[2], (uint32)(sint32)r_s16(0x800800b0 + 2u * (uint32)(r_s16(object + 26) >> 2)));
    return FUN_80010028();
}

GDB_CALL uint32 FUN_8005D53C(uint32 object)
{
    FUNCTION_MARKER(0x8005d53cu, "GAME.EXE");
    sint32 position[4];
    uint32 color;
    FUN_80010040();
    FUN_8005C740(object);
    ff_project_local(0, 0, 0, position);
    if (!position[3])
    {
        position[3] = 256;
        w_u32(0x8008d4bc, 0x2c404040u);
        color = 64u - ((uint32)(sint32)r_s16(object + 22) << 2);
        w_u32(0x8008d4bc, 0x2c000000u + (color << 16) + (color << 8) + color);
        position[3] = (sint32)(((uint32)(sint32)r_s16(object + 22) << 4) + 256u);
        FUN_80012594(position, r_s16(0x8009ad5a));
    }
    return FUN_80010028();
}

GDB_CALL uint32 FUN_8005B4F8(uint32 object)
{
    FUNCTION_MARKER(0x8005b4f8u, "GAME.EXE");
    sint32 angle = r_s16(object + 16);
    if (angle)
        FUN_8001033C(angle);
    FUN_800100C4(0, 0x280000, 0);
    angle = r_s16(object + 20);
    if (angle)
        FUN_800104A0(angle);
    angle = r_s16(object + 18);
    if (angle)
        FUN_800101CC(angle);
    return ff_static_model_80010B18(r_u32(0x80081720 + 4u * r_u32(object)), 0, 0);
}

GDB_CALL uint32 FUN_8005B350(uint32 object)
{
    FUNCTION_MARKER(0x8005b350u, "GAME.EXE");
    sint32 angle = r_s16(object + 16);
    if (angle)
        FUN_8001033C(angle);
    ff_static_model_80010B18(r_u32(0x80081830), 0, 8192);
    FUN_800100C4(0, 0x400000, 0);
    angle = r_s16(object + 20);
    if (angle)
        FUN_800104A0(angle);
    angle = r_s16(object + 18);
    if (angle)
        FUN_800101CC(angle);
    return ff_static_model_80010B18(r_u32(0x80081720 + 4u * r_u32(object)), 0, 0);
}

GDB_CALL uint32 FUN_80044CD4(uint32 object)
{
    FUNCTION_MARKER(0x80044cd4u, "GAME.EXE");
    sint32 angle;
    FUN_80010040();
    FUN_8001033C(r_s16(object + 16));
    angle = r_s16(object + 20);
    if (angle)
        FUN_800104A0(angle);
    angle = r_s16(object + 18);
    if (angle)
        FUN_800101CC(angle);
    ff_static_model_80010B18(r_u32(0x80081720 + 4u * r_u32(object)), 0, 0);
    FUN_8001033C((sint32)((uint32)r_u16(0x80093dd0) << 12));
    ff_static_model_80010B18(r_u32(0x80081ff0), 0, 0);
    return FUN_80010028();
}

GDB_CALL uint32 FUN_8005CD1C(uint32 object)
{
    FUNCTION_MARKER(0x8005cd1cu, "GAME.EXE");
    sint32 position[4];
    FUN_80010040();
    FUN_8005C740(object);
    ff_project_local(0, 0, 0, position);
    if (!position[3])
    {
        position[3] = r_u32(0x80093dd0) & 1u ? 704 : 512;
        w_u32(0x8008d4bc, 0x2cffffffu);
        FUN_80012594(position, r_s16(0x8009ad4c));
    }
    return FUN_80010028();
}

GDB_CALL uint32 FUN_8005BBAC(uint32 object)
{
    FUNCTION_MARKER(0x8005bbacu, "GAME.EXE");
    sint32 position[4];
    FUN_80010040();
    FUN_800100C4((sint32)(r_u32(object + 4) - r_u32(0x8008d4a4)), (sint32)(r_u32(object + 8) - r_u32(0x8008d4a8)), (sint32)(r_u32(object + 12) - r_u32(0x8008d4ac)));
    ff_project_local(0, 0, 0, position);
    if (!position[3])
    {
        FUN_800121A8(position[0], position[1], position[2], 0x4210);
        FUN_800121A8((sint32)((uint32)position[0] + 1u), position[1], position[2], 0x4210);
    }
    return FUN_80010028();
}

/* 11594: distance-faded quads and lines; GTE projection uses the port boundary. */
GDB_CALL uint32 FUN_80011590(uint32 model, uint32 unused, sint32 bias)
{
    FUNCTION_MARKER(0x80011590u, "GAME.EXE");
    /* No resolved call targets11590; the word is normally read as cutoff data.
  * If entered as code, SLL V0,ZERO,16 falls through11594. V0 is overwritten
  * at11598 before any read, so the existing11594 body is the exact successor. */
    return FUN_80011594(model, unused, bias);
}

static uint32 ff_matrix_pointer_add(uint32 a, sint32 delta);

GDB_CALL uint32 FUN_80011594(uint32 model, uint32 unused, sint32 bias)
{
    FUNCTION_MARKER(0x80011594u, "GAME.EXE");
    uint32 matrix = r_u32(0x8008d494), words[8], vertices, xy, depths, xy_output, depth_output;
    uint32 face, left, p, ot, depth_table, saved_depths[3];
    sint32 count, below127, below193, below385 = 0, cutoff, depth_carrier;
    int i;
    for (i = 0; i < 5; i++)
        words[i] = r_u32(matrix + 4 * i);
    for (i = 0; i < 5; i++)
        xport_gte_write_control(i, words[i]);
    for (i = 5; i < 8; i++)
        words[i] = r_u32(matrix + 4 * i);
    for (i = 5; i < 8; i++)
        xport_gte_write_control(i, (uint32)((sint32)words[i] >> 16));
    depth_carrier = (sint32)words[7] >> 16;
    count = r_s32(model + 4);
    vertices = r_u32(model);
    below127 = (sint32)ff_matrix_pointer_add((uint32)count, -127);
    below193 = (sint32)ff_matrix_pointer_add((uint32)count, -193);
    if (below127 >= 0)
        below385 = (sint32)ff_matrix_pointer_add((uint32)count, -385);
    xy = below127 < 0 || below193 < 0 ? 0x1f800000 : 0x1fe800;
    depths = below127 < 0 ? 0x1f800200 : below193 < 0 ? 0x1fe400 : below385 < 0 ? 0x1f800000 : 0x1fe400;
    xy_output = ff_matrix_pointer_add(xy, 12);
    depth_output = ff_matrix_pointer_add(depths, 6);
    for (i = 0; i < 6; i++)
        xport_gte_write_data(i, r_u32(vertices + 4 * i));
    xport_gte_execute(0x280030);
    vertices = ff_matrix_pointer_add(vertices, 24);
    count = (sint32)ff_matrix_pointer_add((uint32)count, -3);
    for (i = 0; i < 3; i++)
        w_u32(xy_output - 12 + 4 * i, xport_gte_read_data(12 + i));
    for (i = 0; i < 3; i++)
        saved_depths[i] = xport_gte_read_data(17 + i);
    /* The original always projects a second triple before storing the first depths */
    do
    {
        for (i = 0; i < 6; i++)
            xport_gte_write_data(i, r_u32(vertices + 4 * i));
        xport_gte_execute(0x280030);
        vertices = ff_matrix_pointer_add(vertices, 24);
        for (i = 0; i < 3; i++)
            w_u16(depth_output - 6 + 2 * i, (uint16)saved_depths[i]);
        xy_output = ff_matrix_pointer_add(xy_output, 12);
        depth_output = ff_matrix_pointer_add(depth_output, 6);
        count = (sint32)ff_matrix_pointer_add((uint32)count, -3);
        for (i = 0; i < 3; i++)
            w_u32(xy_output - 12 + 4 * i, xport_gte_read_data(12 + i));
        for (i = 0; i < 3; i++)
            saved_depths[i] = xport_gte_read_data(17 + i);
    } while (count > 0);
    for (i = 0; i < 3; i++)
        w_u16(depth_output - 6 + 2 * i, (uint16)saved_depths[i]);
    /* 11590 is also read as the 0x1400 distance cutoff; see boundary audit. */
    face = r_u32(model + 8);
    left = r_u32(model + 12);
    cutoff = (sint32)r_u32(0x80011590);
    depth_table = r_u32(0x8008d4d8);
    ot = r_u32(0x8008d4b4);
    p = r_u32(0x8008d4b0);
    do
    {
        uint32 second = r_u32(face + 4), packed = r_u32(face), i0 = packed & 65535, i1 = packed >> 16;
        sint32 d0 = r_s16(ff_matrix_pointer_add(depths, (sint32)(i0 >> 1))), depth;
        if ((sint32)second < 0)
        {
            sint32 sum = d0 + r_s16(ff_matrix_pointer_add(depths, (sint32)(i1 >> 1)));
            uint32 first_xy, second_xy;
            left = ff_matrix_pointer_add(left, -1);
            /* The dead ADD checks the previous T7, not the current depth sum */
            ff_matrix_pointer_add((uint32)depth_carrier, bias);
            depth_carrier = depth = sum >> 3;
            first_xy = ff_matrix_pointer_add(xy, (sint32)i0);
            if (depth > 8188)
                goto next_face;
            second_xy = ff_matrix_pointer_add(xy, (sint32)i1);
            if (depth > 0 && depth <= 8188)
            {
                uint32 command = r_u32(face + 8), s0 = r_u32(first_xy), s1 = r_u32(second_xy);
                sint32 offset = r_s16(ff_matrix_pointer_add(depth_table, depth << 1));
                depth_carrier = offset;
                uint32 bucket, next;
                w_u32(p + 4, command);
                w_u32(p + 8, s0);
                w_u32(p + 12, s1);
                bucket = ff_matrix_pointer_add(ot, offset);
                uint32 link = r_u32(bucket);
                face = ff_matrix_pointer_add(face, 12);
                w_u32(p, link | 0x03000000u);
                next = ff_matrix_pointer_add(p, 16);
                w_u32(bucket, p);
                p = next;
                continue;
            }
        }
        else
        {
            left = ff_matrix_pointer_add(left, -1);
            if (d0 <= 0)
                goto next_face;
            uint32 i2 = second & 65535u, i3 = second >> 16, color = 0x2d000000u;
            sint32 d1 = r_s16(ff_matrix_pointer_add(depths, (sint32)(i2 >> 1)));
            if (d1 > 0)
            {
                sint32 delta, fade;
                sint64 difference;
                depth = (sint32)ff_matrix_pointer_add((uint32)(d0 + d1), bias) >> 1;
                difference = (sint64)depth - cutoff;
                if (difference < -2147483647LL - 1 || difference > 2147483647LL)
                    ff_wip_stop(__FUNCTION__, __FILE__, __LINE__);
                delta = (sint32)difference;
                fade = (sint32)ff_matrix_pointer_add((uint32)delta, 2048);
                depth_carrier = fade;
                if (delta <= 0)
                {
                    if (fade >= 0)
                    {
                        uint32 shade = 128u - (uint32)(fade >> 4);
                        color = 0x2c000000u | shade | (shade << 8) | (shade << 16);
                    }
                    depth_carrier = depth >>= 2;
                    uint32 first_xy = ff_matrix_pointer_add(xy, (sint32)i0);
                    if (depth <= 8192)
                    {
                        uint32 s0 = r_u32(first_xy), s1 = r_u32(ff_matrix_pointer_add(xy, (sint32)i1));
                        uint32 s2 = r_u32(ff_matrix_pointer_add(xy, (sint32)i2)), s3 = r_u32(ff_matrix_pointer_add(xy, (sint32)i3));
                        if ((s0 & 65535u) < 320 || (s1 & 65535u) < 320 || (s2 & 65535u) < 320 || (s3 & 65535u) < 320)
                        {
                            uint32 flags;
                            sint32 offset, area;
                            xport_gte_write_data(12, s0);
                            xport_gte_write_data(14, s2);
                            if ((sint32)s0 <= 0 && (sint32)s1 <= 0 && (sint32)s2 <= 0 && (sint32)s3 < 0)
                                goto next_face;
                            xport_gte_write_data(13, s1);
                            offset = r_s16(ff_matrix_pointer_add(depth_table, (sint32)((uint32)depth << 1)));
                            depth_carrier = offset;
                            xport_gte_execute(0x140006);
                            flags = r_u32(face + 8);
                            area = (sint32)xport_gte_read_data(24);
                            if ((flags & 1u) || area >= 0)
                            {
                                uint32 texture = ff_matrix_pointer_add(0x800a8768, (sint32)(flags >> 16)), uv, mode, end, semi, bucket, next;
                                if (flags & 2u)
                                {
                                    w_u32(p + 8, s1);
                                    w_u32(p + 16, s0);
                                    w_u32(p + 24, s2);
                                    w_u32(p + 32, s3);
                                }
                                else
                                {
                                    w_u32(p + 8, s0);
                                    w_u32(p + 16, s1);
                                    w_u32(p + 24, s3);
                                    w_u32(p + 32, s2);
                                }
                                uv = r_u32(texture);
                                mode = r_u32(texture + 4);
                                end = r_u32(texture + 8);
                                semi = uv & 1u;
                                w_u32(p + 12, uv - semi);
                                w_u32(p + 20, mode);
                                w_u16(p + 28, (uint16)end);
                                w_u16(p + 36, (uint16)(end >> 16));
                                w_u32(p + 4, (semi << 25) | color);
                                bucket = ff_matrix_pointer_add(ot, offset);
                                w_u32(p, r_u32(bucket) | 0x09000000u);
                                next = ff_matrix_pointer_add(p, 40);
                                w_u32(bucket, p & 0xffffffu);
                                p = next;
                            }
                        }
                    }
                }
            }
        }
    next_face:
        face = ff_matrix_pointer_add(face, 12);
    } while (left);
    w_u32(0x8008d4bc, 0x2d000000u);
    w_u32(0x8008d4b0, p);
    return 0x8008d4b0;
}

GDB_CALL uint32 FUN_800450F8(uint32 object)
{
    FUNCTION_MARKER(0x800450f8u, "GAME.EXE");
    sint32 position[4], x, y, z;
    uint32 i, camera_x, camera_y, camera_z;
    FUN_80010040();
    z = r_s16(object + 14);
    camera_x = r_u32(0x8008d4a4);
    camera_y = r_u32(0x8008d4a8);
    camera_z = r_u32(0x8008d4ac);
    FUN_800100C4((sint32)(0 - camera_x), (sint32)(0 - camera_y), (sint32)(0 - camera_z));
    x = r_s16(object + 6);
    y = r_s16(object + 10);
    ff_project_local(x, y, z, position);
    for (i = 0; i < 32; i++)
        FUN_80044D88();
    return FUN_80010028();
}

GDB_CALL uint32 FUN_8004277C(uint32 object)
{
    FUNCTION_MARKER(0x8004277cu, "GAME.EXE");
    if ((sint32)r_u32(object + 8) <= (sint32)0xe0000000u)
        return 0;
    FUN_80010040();
    FUN_8001033C(r_s16(object + 16));
    FUN_800104A0(r_s16(object + 20));
    FUN_800101CC(r_s16(object + 18));
    ff_static_model_80010B18(r_u32(0x800821dc), 0, 0);
    FUN_8001033C((sint32)((uint32)r_u16(0x80093dd0) << 12));
    ff_static_model_80010B18(r_u32(0x80081ff0), 0, 0);
    return FUN_80010028();
}

GDB_CALL uint32 FUN_8005B684(uint32 object)
{
    FUNCTION_MARKER(0x8005b684u, "GAME.EXE");
    sint32 position[4];
    uint32 model, color;
    ff_project_local(0, 0, 0, position);
    model = r_u32(object);
    if ((uint32)position[2] - 1u >= 0x2fffu)
        return 0;
    color = 128u - (uint32)(position[2] >> 7);
    w_u32(0x8008d4bc, 0x2c000000u + (color << 16) + (color << 8) + color);
    return ff_static_model_80010B18(r_u32(0x80081720 + 4u * model), 0, 0);
}

GDB_CALL uint32 FUN_8005B7AC(uint32 object)
{
    FUNCTION_MARKER(0x8005b7acu, "GAME.EXE");
    sint32 position[4], shade;
    uint32 color;
    ff_project_local(0, 0, 0, position);
    shade = (position[2] >> 2) - 32;
    if (shade <= 0)
        return 1;
    if (shade < 128)
    {
        color = (uint32)shade;
        w_u32(0x8008d4bc, 0x2c000000u + (color << 16) + (color << 8) + color);
    }
    return ff_static_model_80010B18(r_u32(0x80081720 + 4u * r_u32(object)), 0, 0);
}

GDB_CALL uint32 FUN_8005D160(uint32 object)
{
    FUNCTION_MARKER(0x8005d160u, "GAME.EXE");
    sint32 position[4];
    FUN_80010040();
    FUN_8005C740(object);
    ff_project_local(0, 0, 0, position);
    if (!position[3])
    {
        position[3] = 4096;
        w_u32(0x8008d4bc, 0x2c404040);
        FUN_80012594(position, r_s16(0x8009ad5c + 2u * (uint32)(r_s16(object + 22) >> 2)));
    }
    return FUN_80010028();
}

GDB_CALL uint32 FUN_8005D608(uint32 object)
{
    FUNCTION_MARKER(0x8005d608u, "GAME.EXE");
    sint32 position[4];
    FUN_80010040();
    FUN_8005C740(object);
    ff_project_local(0, 0, 0, position);
    if (!position[3])
    {
        position[3] = 512;
        FUN_80012594(position, r_s16(0x8009ad4c));
    }
    return FUN_80010028();
}

GDB_CALL uint32 FUN_8005D7A4(uint32 object)
{
    FUNCTION_MARKER(0x8005d7a4u, "GAME.EXE");
    if ((uint32)FUN_80012190() - 257u >= 0xeffu)
        return 0;
    ff_static_model_80010B18(r_u32(0x80081720 + 4u * r_u32(object)), 0, 0);
    return ff_static_model_80010B18(r_u32(0x80081724 + 4u * r_u32(object)), 0, 0);
}

GDB_CALL uint32 FUN_8005BD7C(uint32 object)
{
    FUNCTION_MARKER(0x8005bd7cu, "GAME.EXE");
    sint32 phase = (sint32)r_u32(0x80093da8) >> 4;
    uint32 color, height, i;
    if (phase < 433)
    {
        phase -= 416;
        if (phase < 0)
            phase = 0;
        color = 4u * (uint32)phase + 64u;
        w_u32(0x8008d4bc, 0x2c000000u + (color << 16) + (color << 8) + color);
    }
    ff_static_model_80010B18(r_u32(0x80081720 + 4u * r_u32(object)), 0, 2048);
    height = 0u - r_u32(0x80093da8);
    for (i = 0; i < 4; i++)
    {
        FUN_8005BD04(440, height);
        height += 1472u;
    }
    FUN_8005BD04(439, height);
    for (i = 0; i < 8; i++)
    {
        height += 3456u;
        FUN_8005BD04(437u + (i & 1u), height);
    }
    height += 3456u;
    FUN_8005BD04(441, height);
    return FUN_8005BD04(442, height + 3456u);
}

GDB_CALL void FUN_80047108(uint32 object)
{
    FUNCTION_MARKER(0x80047108u, "GAME.EXE");
    object_rotations(object);
}

static void object_rotations(uint32 object)
{
    sint32 angle = r_s16(object + 16);
    if (angle)
        FUN_8001033C(angle);
    angle = r_s16(object + 20);
    if (angle)
        FUN_800104A0(angle);
    angle = r_s16(object + 18);
    if (angle)
        FUN_800101CC(angle);
}

GDB_CALL uint32 FUN_8005AB18(uint32 object)
{
    FUNCTION_MARKER(0x8005ab18u, "GAME.EXE");
    sint32 angle;
    if (!FUN_80011E5C())
        return 0;
    angle = r_s16(object + 16);
    if (angle)
        FUN_8001033C(angle);
    return ff_static_model_80010B18(r_u32(0x80081720 + 4u * r_u32(object)), 0, 0);
}

GDB_CALL uint32 FUN_8005BD04(uint32 model, uint32 height)
{
    FUNCTION_MARKER(0x8005bd04u, "GAME.EXE");
    if (height + 0x17ffu >= 0x1fffu)
        return 0;
    FUN_80010040();
    FUN_800100C4(0, (sint32)(height << 16), 0);
    ff_static_model_80010B18(r_u32(0x80081720 + 4u * model), 0, 2048);
    return FUN_80010028();
}

GDB_CALL uint32 FUN_8002A858(void)
{
    FUNCTION_MARKER(0x8002a858u, "GAME.EXE");
    if ((sint32)r_u32(0x80094004) > 0x0fffffff)
        return 1;
    return ff_static_model_80010B18(r_u32(0x80081cc0), 0, 0);
}

GDB_CALL uint32 FUN_8002A8FC(uint32 object)
{
    FUNCTION_MARKER(0x8002a8fcu, "GAME.EXE");
    uint32 angle;
    sint32 x;
    FUN_800100C4(0, 0x02800000, 0);
    x = (sint32)r_u32(object + 4);
    angle = r_u32(0x80093dd0);
    if (x > 0)
        angle = 0u - angle;
    FUN_800104A0((sint32)(angle << 12));
    return ff_static_model_80010B18(r_u32(0x80081d60), 0, 0);
}

static uint32 rising_particle(uint32 object, sint32 end_age)
{
    uint16 age, velocity;
    uint32 delta;
    if (!r_s16(object + 22))
        w_u16(object + 26, (uint16)(ff_rand_8006D348() & 3u));
    age = (uint16)((uint16)r_s16(object + 22) + 1u);
    w_u16(object + 22, age);
    if ((sint16)age == end_age)
        w_u32(object, 0xffffffffu);
    velocity = (uint16)((uint16)r_s16(object + 26) + 1u);
    w_u16(object + 26, velocity);
    delta = (uint32)((sint32)((uint32)velocity << 16) >> 1);
    w_u32(object + 8, r_u32(object + 8) - delta);
    return delta;
}

GDB_CALL uint32 FUN_8002ACE8(uint32 object)
{
    FUNCTION_MARKER(0x8002ace8u, "GAME.EXE");
    return rising_particle(object, 16);
}

GDB_CALL uint32 FUN_8002AD7C(uint32 object)
{
    FUNCTION_MARKER(0x8002ad7cu, "GAME.EXE");
    return rising_particle(object, 64);
}

GDB_CALL uint32 FUN_8002B150(uint32 object)
{
    FUNCTION_MARKER(0x8002b150u, "GAME.EXE");
    return rising_particle(object, 16);
}

GDB_CALL uint32 FUN_80032680(uint32 object)
{
    FUNCTION_MARKER(0x80032680u, "GAME.EXE");
    if ((uint16)(r_u16(0x8008d490) + 16384) <= 32768)
        return 0;
    return ff_static_model_80010B18(r_u32(0x80081720 + 4u * r_u32(object)), 0, 0);
}

GDB_CALL uint32 FUN_800326DC(uint32 object)
{
    FUNCTION_MARKER(0x800326dcu, "GAME.EXE");
    uint32 kind = r_u32(object);
    return ff_static_model_80010B18(r_u32(0x80081720 + 4u * kind), 0, kind - 133u < 2u ? 1024 : 512);
}

GDB_CALL uint32 FUN_80032728(uint32 object)
{
    FUNCTION_MARKER(0x80032728u, "GAME.EXE");
    if (!r_u32(0x80094674))
        return 0;
    return ff_static_model_80010B18(r_u32(0x80081720 + 4u * r_u32(object)), 0, 512);
}

GDB_CALL uint32 FUN_800327B0(void)
{
    FUNCTION_MARKER(0x800327b0u, "GAME.EXE");
    sint32 phase = (sint32)(r_u32(0x80093dd0) - 270u);
    if (phase < 0)
        phase = 0;
    else if (phase > 128)
        phase = 128;
    FUN_80010F94(16384, 16384, phase << 7);
    return ff_static_model_80010B18(r_u32(0x800821b8), 0, 0);
}

GDB_CALL uint32 FUN_80032AA4(uint32 object)
{
    FUNCTION_MARKER(0x80032aa4u, "GAME.EXE");
    uint32 offset = r_u32(0x80094674) << 3;
    if (r_u32(object) == 467)
        offset = 0u - offset;
    FUN_800100C4((sint32)(offset << 16), 0, 0);
    return ff_static_model_80010B18(r_u32(0x80081720 + 4u * r_u32(object)), 0, 256);
}

GDB_CALL uint32 FUN_80032B20(uint32 object)
{
    FUNCTION_MARKER(0x80032b20u, "GAME.EXE");
    uint32 offset = r_u32(0x80094674) << 3;
    if (r_u32(object) == 470)
        offset = 0u - offset;
    FUN_800100C4(0, 0, (sint32)(offset << 16));
    return ff_static_model_80010B18(r_u32(0x80081720 + 4u * r_u32(object)), 0, 256);
}

GDB_CALL uint32 FUN_80033068(sint32 index)
{
    FUNCTION_MARKER(0x80033068u, "GAME.EXE");
    if (r_u32(0x800b4334 + 244u * (uint32)index) & 0x800u)
        return FUN_80032EE0(index);
    return (uint32)FUN_80056670(187, 255, 6144);
}

GDB_CALL sint32 FUN_80033DC8(sint32 index)
{
    FUNCTION_MARKER(0x80033dc8u, "GAME.EXE");
    sint32 value = r_s16(0x800b436c + 244u * (uint32)index);
    return value ? value : FUN_80056670(28, 255, 6144);
}

GDB_CALL uint32 FUN_8002A00C(uint32 object)
{
    FUNCTION_MARKER(0x8002a00cu, "GAME.EXE");
    return ff_static_model_80010B18(r_u32(0x80081720 + 4u * r_u32(object)), 0, 256);
}

GDB_CALL uint32 FUN_80032774(uint32 object)
{
    FUNCTION_MARKER(0x80032774u, "GAME.EXE");
    return ff_static_model_80010B18(r_u32(0x80081720 + 4u * r_u32(object)), 0, 512);
}

GDB_CALL uint32 FUN_8002A7DC(uint32 object)
{
    FUNCTION_MARKER(0x8002a7dcu, "GAME.EXE");
    FUN_8001033C(r_s16(object + 16));
    ff_static_model_80010B18(r_u32(0x80082340), 0, 0);
    FUN_800100C4(0, 0x016a0000, -10485760);
    FUN_8001033C(r_s16(object + 26));
    return ff_static_model_80010B18(r_u32(0x80082344), 0, 0);
}

GDB_CALL uint32 FUN_8002A89C(void)
{
    FUNCTION_MARKER(0x8002a89cu, "GAME.EXE");
    ff_static_model_80010B18(r_u32(0x80082360), 0, 0);
    FUN_800100C4(0x04e00000, 0x00f00000, 0);
    FUN_8001033C(16384);
    return ff_static_model_80010B18(r_u32(0x80081f2c), 0, 0);
}

GDB_CALL uint32 FUN_8002AA6C(uint32 object)
{
    FUNCTION_MARKER(0x8002aa6cu, "GAME.EXE");
    uint32 x, y, z, camera_x, camera_y, camera_z, kind, height;
    FUN_80010040();
    x = r_u32(object + 4);
    camera_x = r_u32(0x8008d4a4);
    kind = r_u32(object);
    y = r_u32(object + 8);
    height = r_u32(0x800990d8 + 4 * kind);
    y += height;
    camera_y = r_u32(0x8008d4a8);
    z = r_u32(object + 12);
    camera_z = r_u32(0x8008d4ac);
    FUN_800100C4((sint32)(x - camera_x), (sint32)(y - camera_y), (sint32)(z - camera_z));
    FUN_800101CC(32768);
    FUN_8001033C(r_s16(object + 16));
    FUN_800104A0(-r_s16(object + 20));
    FUN_800101CC(r_s16(object + 18));
    ff_static_model_80010B18(r_u32(0x80081720 + 4u * r_u32(object)), 0, 0);
    return FUN_80010028();
}

GDB_CALL uint32 FUN_8002ABE0(uint32 object)
{
    FUNCTION_MARKER(0x8002abe0u, "GAME.EXE");
    FUN_80010040();
    FUN_800100C4((sint32)(r_u32(object + 4) - r_u32(0x8008d4a4)), (sint32)(r_u32(object + 8) - r_u32(0x8008d4a8)), (sint32)(r_u32(object + 12) - r_u32(0x8008d4ac)));
    FUN_800104A0(r_s16(object + 20));
    FUN_8001033C(r_s16(object + 18));
    FUN_800101CC(r_s16(object + 16));
    ff_static_model_80010B18(r_u32(0x80081720 + 4u * r_u32(object)), 0, 0);
    return FUN_80010028();
}

GDB_CALL uint32 FUN_8002C294(uint32 object)
{
    FUNCTION_MARKER(0x8002c294u, "GAME.EXE");
    uint32 kind = r_u32(object);
    if (r_u32(0x80093d60) == 2)
        ++kind;
    return ff_static_model_80010B18(r_u32(0x80081720 + 4u * kind), 0, 0);
}

GDB_CALL uint32 FUN_80032B98(uint32 object)
{
    FUNCTION_MARKER(0x80032b98u, "GAME.EXE");
    uint32 offset = r_u32(0x80094794);
    FUN_80010040();
    offset <<= 19;
    FUN_800100C4(0, 0, (sint32)offset);
    ff_static_model_80010B18(r_u32(0x80081720 + 4u * r_u32(object)), 0, 256);
    FUN_80010028();
    FUN_8001033C(32768);
    FUN_800100C4(0, 0, (sint32)offset);
    return ff_static_model_80010B18(r_u32(0x80081720 + 4u * r_u32(object)), 0, 256);
}

GDB_CALL uint32 FUN_8003EE7C(uint32 object)
{
    FUNCTION_MARKER(0x8003ee7cu, "GAME.EXE");
    FUN_8001033C(49152);
    return ff_static_model_80010B18(r_u32(0x80081720 + 4u * r_u32(object)), 0, 0);
}

GDB_CALL uint32 FUN_8005BC64(uint32 object)
{
    FUNCTION_MARKER(0x8005bc64u, "GAME.EXE");
    FUN_80010040();
    FUN_800100C4((sint32)(r_u32(object + 4) - r_u32(0x8008d4a4)), (sint32)(r_u32(object + 8) - r_u32(0x8008d4a8)), (sint32)(r_u32(object + 12) - r_u32(0x8008d4ac)));
    FUN_8001033C((sint32)(16384u - ((uint32)(sint32)r_s16(object + 16) << 5)));
    ff_static_model_80010B18(r_u32(0x80081720 + 4u * r_u32(object)), 0, 0);
    return FUN_80010028();
}

GDB_CALL uint32 FUN_8005BF0C(uint32 object)
{
    FUNCTION_MARKER(0x8005bf0cu, "GAME.EXE");
    FUN_80010040();
    FUN_8001033C(r_s16(object + 16));
    w_u32(0x8008d4bc, 0x2c404040);
    ff_static_model_80010B18(r_u32(0x80081720 + 4u * r_u32(object)), 0, 0);
    return FUN_80010028();
}

GDB_CALL uint32 FUN_8005BF7C(uint32 object)
{
    FUNCTION_MARKER(0x8005bf7cu, "GAME.EXE");
    FUN_80010040();
    FUN_8001033C(r_s16(object + 16));
    FUN_800101CC(r_s16(object + 18));
    FUN_800104A0(-r_s16(object + 20));
    ff_static_model_80010B18(r_u32(0x80081720 + 4u * r_u32(object)), 0, 0);
    return FUN_80010028();
}

GDB_CALL uint32 FUN_8005BFF4(uint32 object)
{
    FUNCTION_MARKER(0x8005bff4u, "GAME.EXE");
    FUN_80010040();
    FUN_8001033C(r_s16(object + 16));
    FUN_800101CC(r_s16(object + 18));
    FUN_800104A0(-r_s16(object + 20));
    ff_static_model_80010B18(r_u32(0x80081720 + 4u * r_u32(object)), 0, -256);
    FUN_800100C4(-3145728, 2621440, 0);
    FUN_8001033C((sint32)((uint32)r_u16(0x80093dd0) << 12));
    ff_static_model_80010B18(r_u32(0x80081ff0), 0, -256);
    FUN_80010028();
    FUN_80010040();
    FUN_800100C4(0, (sint32)r_u32(object + 8), 0);
    FUN_8001033C(r_s16(object + 16));
    ff_static_model_80010B18(r_u32(0x80082428), 0, -2048);
    return FUN_80010028();
}

GDB_CALL uint32 FUN_8005D2E4(uint32 object)
{
    FUNCTION_MARKER(0x8005d2e4u, "GAME.EXE");
    FUN_80010040();
    FUN_8005C740(object);
    ff_static_model_80010B18(r_u32(0x80081720 + 4u * r_u32(object)), 0, 0);
    return FUN_80010028();
}

GDB_CALL uint32 FUN_8005D340(uint32 object)
{
    FUNCTION_MARKER(0x8005d340u, "GAME.EXE");
    uint32 scale, color;
    FUN_80010040();
    FUN_8005C740(object);
    scale = (uint32)(sint32)r_s16(object + 22) << 12;
    FUN_80010F94((sint32)scale, (sint32)scale, (sint32)scale);
    color = 64u - 2u * (uint32)(sint32)r_s16(object + 22);
    w_u32(0x8008d4bc, 0x2c000000u + (color << 16) + (color << 8) + color);
    ff_static_model_80010B18(r_u32(0x80082088), 0, 0);
    return FUN_80010028();
}

GDB_CALL sint32 FUN_80012190(void)
{
    FUNCTION_MARKER(0x80012190u, "GAME.EXE");
    return r_s16(r_u32(0x8008d494) + 30);
}

static uint32 ff_matrix_pointer_add(uint32 a, sint32 delta);

/* 124A0: signed UV subtraction precedes truncation; OT offsets remain signed */
GDB_CALL uint32 FUN_800124A0(const sint32 *position, uint32 texture)
{
    FUNCTION_MARKER(0x800124a0u, "GAME.EXE");
    uint32 packet = r_u32(0x8008d4b0), uv;
    sint32 x = (sint16)position[0], y = (sint16)position[1];
    uint32 first, second, last, mode, base, extent, right, bottom, ot, depth, lookup, link, next;
    sint64 difference;
    uv = ff_matrix_pointer_add(0x800a8768, (sint32)(texture << 2));
    uv = ff_matrix_pointer_add(uv, (sint32)(texture << 3));
    first = r_u32(uv);
    second = r_u32(uv + 4);
    last = r_u32(uv + 8);
    mode = first & 1;
    base = first - mode;
    difference = (sint64)(last >> 16) - (sint32)base;
    if (difference < -2147483647LL - 1 || difference > 2147483647LL)
        ff_wip_stop(__FUNCTION__, __FILE__, __LINE__);
    extent = (uint32)difference & 65535;
    right = (extent & 255) + (uint32)x;
    bottom = (extent >> 8) + (uint32)y;
    w_u32(packet + 12, base);
    w_u32(packet + 20, second);
    w_u16(packet + 28, (uint16)last);
    w_u16(packet + 36, (uint16)(last >> 16));
    w_u32(packet + 4, 0x2d000000u | (mode << 25));
    w_u16(packet + 8, (uint16)x);
    w_u16(packet + 24, (uint16)x);
    w_u16(packet + 10, (uint16)y);
    w_u16(packet + 18, (uint16)y);
    w_u16(packet + 16, (uint16)right);
    w_u16(packet + 32, (uint16)right);
    w_u16(packet + 26, (uint16)bottom);
    w_u16(packet + 34, (uint16)bottom);
    ot = r_u32(0x8008d4b4);
    depth = (uint32)(position[2] >> 2);
    lookup = r_u32(0x8008d4d8);
    lookup = ff_matrix_pointer_add(lookup, (sint32)(depth << 1));
    link = ff_matrix_pointer_add(ot, r_s16(lookup));
    w_u32(packet, r_u32(link) | 0x09000000u);
    next = ff_matrix_pointer_add(packet, 40);
    w_u32(link, packet & 0xffffffu);
    w_u32(0x8008d4b0, next);
    return 0x8008d4b0;
}

/* Exact original leaf, including its return delay slot; see leaf-batch.json. */
GDB_CALL uint32 FUN_80011E28(uint32 head, uint32 packet, uint32 flags)
{
    FUNCTION_MARKER(0x80011e28u, "GAME.EXE");
    uint32 old = r_u32(head);
    w_u32(head, packet);
    w_u32(packet, old | flags);
    return old;
}

/* Exact original leaf, including its return delay slot; see leaf-batch.json. */
GDB_CALL uint32 FUN_8001F510(void)
{
    FUNCTION_MARKER(0x8001f510u, "GAME.EXE");
    return 0;
}

/* Exact original leaf, including its return delay slot; see leaf-batch.json. */
GDB_CALL uint32 FUN_8002A154(void)
{
    FUNCTION_MARKER(0x8002a154u, "GAME.EXE");
    w_u16(r_u32(0x800947c8) + 28, 1);
    return 1;
}

/* Exact original leaf, including its return delay slot; see leaf-batch.json. */
GDB_CALL void FUN_8002A974(void)
{
    FUNCTION_MARKER(0x8002a974u, "GAME.EXE");
}

/* Exact original leaf, including its return delay slot; see leaf-batch.json. */
GDB_CALL uint32 FUN_8002C1DC(uint32 object)
{
    FUNCTION_MARKER(0x8002c1dcu, "GAME.EXE");
    w_u32(object, 0xffffffffu);
    return 0xffffffffu;
}

/* Exact original leaf, including its return delay slot; see leaf-batch.json. */
GDB_CALL uint32 FUN_8002C27C(uint32 object)
{
    FUNCTION_MARKER(0x8002c27cu, "GAME.EXE");
    w_u32(object, 0xffffffffu);
    return 0xffffffffu;
}

/* Exact original leaf, including its return delay slot; see leaf-batch.json. */
GDB_CALL uint32 FUN_8002EB58(uint32 object)
{
    FUNCTION_MARKER(0x8002eb58u, "GAME.EXE");
    uint32 value = (uint16)r_s16(object + 16) + 1004u;
    w_u16(object + 16, (uint16)value);
    return value;
}

/* Exact original leaf, including its return delay slot; see leaf-batch.json. */
GDB_CALL void FUN_80032ED8(void)
{
    FUNCTION_MARKER(0x80032ed8u, "GAME.EXE");
}

/* Exact original leaf, including its return delay slot; see leaf-batch.json. */
GDB_CALL void FUN_8003B774(void)
{
    FUNCTION_MARKER(0x8003b774u, "GAME.EXE");
}

/* Exact original leaf, including its return delay slot; see leaf-batch.json. */
GDB_CALL uint32 FUN_800415F4(uint32 object)
{
    FUNCTION_MARKER(0x800415f4u, "GAME.EXE");
    w_u32(object, 0xffffffffu);
    return 0xffffffffu;
}

/* Exact original leaf, including its return delay slot; see leaf-batch.json. */
GDB_CALL uint32 FUN_80041600(uint32 object)
{
    FUNCTION_MARKER(0x80041600u, "GAME.EXE");
    w_u32(object, 0xffffffffu);
    return 0xffffffffu;
}

/* Exact original leaf, including its return delay slot; see leaf-batch.json. */
GDB_CALL void FUN_80041B4C(void)
{
    FUNCTION_MARKER(0x80041b4cu, "GAME.EXE");
}

/* Exact original leaf, including its return delay slot; see leaf-batch.json. */
GDB_CALL void FUN_8004483C(void)
{
    FUNCTION_MARKER(0x8004483cu, "GAME.EXE");
}

/* Exact original leaf, including its return delay slot; see leaf-batch.json. */
GDB_CALL void FUN_80044D88(void)
{
    FUNCTION_MARKER(0x80044d88u, "GAME.EXE");
}

/* Exact original leaf, including its return delay slot; see leaf-batch.json. */
GDB_CALL void FUN_800455DC(void)
{
    FUNCTION_MARKER(0x800455dcu, "GAME.EXE");
}

/* Exact original leaf, including its return delay slot; see leaf-batch.json. */
GDB_CALL void FUN_8004EF64(void)
{
    FUNCTION_MARKER(0x8004ef64u, "GAME.EXE");
}

/* Exact original leaf, including its return delay slot; see leaf-batch.json. */
GDB_CALL void FUN_800582DC(void)
{
    FUNCTION_MARKER(0x800582dcu, "GAME.EXE");
}

/* Exact original leaf, including its return delay slot; see leaf-batch.json. */
GDB_CALL void FUN_8005862C(void)
{
    FUNCTION_MARKER(0x8005862cu, "GAME.EXE");
}

/* Exact original leaf, including its return delay slot; see leaf-batch.json. */
GDB_CALL void FUN_8005C654(void)
{
    FUNCTION_MARKER(0x8005c654u, "GAME.EXE");
}

/* Exact original leaf, including its return delay slot; see leaf-batch.json. */
GDB_CALL void FUN_8005C65C(void)
{
    FUNCTION_MARKER(0x8005c65cu, "GAME.EXE");
}

/* Exact original leaf, including its return delay slot; see leaf-batch.json. */
GDB_CALL void FUN_8005C664(void)
{
    FUNCTION_MARKER(0x8005c664u, "GAME.EXE");
}

/* Exact original leaf, including its return delay slot; see leaf-batch.json. */
GDB_CALL void FUN_8005D150(void)
{
    FUNCTION_MARKER(0x8005d150u, "GAME.EXE");
}

/* Exact original leaf, including its return delay slot; see leaf-batch.json. */
GDB_CALL void FUN_8005D158(void)
{
    FUNCTION_MARKER(0x8005d158u, "GAME.EXE");
}

/* Exact original leaf, including its return delay slot; see leaf-batch.json. */
GDB_CALL void FUN_8005DACC(void)
{
    FUNCTION_MARKER(0x8005daccu, "GAME.EXE");
}

/* Exact original leaf, including its return delay slot; see leaf-batch.json. */
GDB_CALL void FUN_8005E138(void)
{
    FUNCTION_MARKER(0x8005e138u, "GAME.EXE");
}

/* Exact original leaf, including its return delay slot; see leaf-batch.json. */
GDB_CALL uint32 FUN_8005F050(uint32 value)
{
    FUNCTION_MARKER(0x8005f050u, "GAME.EXE");
    return value;
}

/* Exact original leaf, including its return delay slot; see leaf-batch.json. */
GDB_CALL uint32 FUN_8005F234(uint32 value)
{
    FUNCTION_MARKER(0x8005f234u, "GAME.EXE");
    return value;
}

/* Exact original leaf, including its return delay slot; see leaf-batch.json. */
GDB_CALL void FUN_80064620(void)
{
    FUNCTION_MARKER(0x80064620u, "GAME.EXE");
}

/* Exact original leaf, including its return delay slot; see leaf-batch.json. */
GDB_CALL uint32 FUN_8006467C(void)
{
    FUNCTION_MARKER(0x8006467cu, "GAME.EXE");
    return 0;
}

/* Exact original leaf, including its return delay slot; see leaf-batch.json. */
GDB_CALL void FUN_80066ABC(void)
{
    FUNCTION_MARKER(0x80066abcu, "GAME.EXE");
}

/* Exact original leaf, including its return delay slot; see leaf-batch.json. */
GDB_CALL void FUN_8006A0F8(void)
{
    FUNCTION_MARKER(0x8006a0f8u, "GAME.EXE");
}

#include <stdlib.h>
#include <string.h>

/* Positive finite software-float paths used only by6A50C. These preserve
 * the original SDK rounding; they are not general IEEE754 replacements. */
static uint32 ff_table_float_div(uint32 a, uint32 b)
{
    sint32 exponent;
    uint32 numerator, denominator, bit, quotient = 0;
    if ((a | b) & 0x80000000u || !b)
        ff_wip_stop(__FUNCTION__, __FILE__, __LINE__);
    if (!a)
        return 0;
    exponent = (sint32)((a >> 23) & 255) - (sint32)((b >> 23) & 255) + 126;
    numerator = (a & 0x7fffff) | 0x800000;
    denominator = (b & 0x7fffff) | 0x800000;
    if (numerator < denominator)
    {
        numerator <<= 1;
        --exponent;
    }
    for (bit = 0x1000000; bit; bit >>= 1)
    {
        if (numerator >= denominator)
        {
            quotient |= bit;
            numerator -= denominator;
        }
        numerator <<= 1;
    }
    if (exponent < 0 || exponent >= 254)
        ff_wip_stop(__FUNCTION__, __FILE__, __LINE__);
    return ((uint32)(exponent + 1) << 23) | (((quotient + 1) >> 1) & 0xff7fffff);
}

static uint32 ff_table_float_add(uint32 a, uint32 b)
{
    uint32 ea = (a >> 23) & 255, eb = (b >> 23) & 255, x, y, sum;
    if ((a | b) & 0x80000000u)
        ff_wip_stop(__FUNCTION__, __FILE__, __LINE__);
    if (!a)
        return b;
    if (!b)
        return a;
    if (eb + 25 < ea)
        return a;
    if (ea + 25 < eb)
        return b;
    x = ((a & 0x7fffff) | 0x800000) << 6;
    y = ((b & 0x7fffff) | 0x800000) << 6;
    if (eb < ea)
        y >>= ea - eb;
    else
    {
        x >>= eb - ea;
        ea = eb;
    }
    sum = x + y;
    while (!(sum & 0xe0000000))
    {
        sum <<= 1;
        --ea;
    }
    if (sum & 0x40000000)
    {
        sum >>= 1;
        ++ea;
    }
    sum += (sum & 0x40) ? 32 : 31;
    if (sum & 0x40000000)
    {
        sum >>= 1;
        ++ea;
    }
    if (ea >= 255)
        ff_wip_stop(__FUNCTION__, __FILE__, __LINE__);
    return (ea << 23) | ((sum >> 6) & 0xff7fffff);
}

static uint32 ff_table_int_float(uint32 value)
{
    uint32 exponent = 127, top = value;
    if (!value)
        return 0;
    if (value > 8191)
        ff_wip_stop(__FUNCTION__, __FILE__, __LINE__);
    while (top >> 1)
    {
        top >>= 1;
        ++exponent;
    }
    return (exponent << 23) | ((value << (150 - exponent)) & 0x7fffff);
}

static uint32 ff_table_float_int(uint32 value)
{
    uint32 exponent = (value >> 23) & 255;
    if (!value || exponent < 126)
        return 0;
    if (exponent > 157 || value & 0x80000000u)
        ff_wip_stop(__FUNCTION__, __FILE__, __LINE__);
    return (((value & 0x7fffff) | 0x800000) << 7) >> (157 - exponent);
}

uint32 FUN_8006A50C(void)
{
    uint32 i, state = 0x3f800000;
    for (i = 0; i < 8192; i++)
    {
        w_u16(r_u32(0x8008d4d8) + 2 * i, (uint16)(ff_table_float_int(ff_table_float_div(0x44ffe000, state)) << 2));
        state = ff_table_float_add(state, ff_table_float_div(ff_table_int_float(i), 0x46000000));
    }
    w_u16(r_u32(0x8008d4d8), 8188);
    return 8188;
}

/* 80011D20: eight word stores include matrix padding and translations. */
GDB_CALL sint32 FUN_80011D20(void)
{
    uint32 a = r_u32(0x8008d494);
    int i;
    for (i = 0; i < 8; i++)
        w_u32(a + 4u * i, (i == 0 || i == 2 || i == 4) ? 4096 : 0);
    return 4096;
}

/* Rotation columns retain the original MVMVA and IR read pipeline */
static void ff_matrix_scratch_columns(uint32 a)
{
    uint32 words[5], first[3], second[3], third_x, third_y, x, y, z;
    int i;
    for (i = 0; i < 5; i++)
        words[i] = r_u32(a + 4 * i);
    for (i = 0; i < 5; i++)
        xport_gte_write_control(i, words[i]);
    y = r_u16(0x1f8003e6);
    x = r_u16(0x1f8003e0);
    z = r_u32(0x1f8003ec);
    xport_gte_write_data(0, x | (y << 16));
    xport_gte_write_data(1, z);
    xport_gte_execute(0x486012);
    y = r_u16(0x1f8003e8);
    x = r_u16(0x1f8003e2);
    z = (uint32)(sint32)r_s16(0x1f8003ee);
    for (i = 0; i < 3; i++)
        first[i] = xport_gte_read_data(9 + i);
    xport_gte_write_data(0, x | (y << 16));
    xport_gte_write_data(1, z);
    xport_gte_execute(0x486012);
    y = r_u16(0x1f8003ea);
    x = r_u16(0x1f8003e4);
    z = r_u32(0x1f8003f0);
    for (i = 0; i < 3; i++)
        second[i] = xport_gte_read_data(9 + i);
    xport_gte_write_data(0, x | (y << 16));
    xport_gte_write_data(1, z);
    xport_gte_execute(0x486012);
    w_u32(a, (first[0] & 65535) | (second[0] << 16));
    w_u32(a + 12, (first[2] & 65535) | (second[2] << 16));
    third_x = xport_gte_read_data(9);
    third_y = xport_gte_read_data(10);
    w_u32(a + 4, (third_x & 65535) | (first[1] << 16));
    w_u32(a + 8, (second[1] & 65535) | (third_y << 16));
    w_u32(a + 16, xport_gte_read_data(11));
}

/* Table pitch uses three rotation-only MVMVA columns */
GDB_CALL void ff_pitch_index_800101E0(uint32 index)
{
    uint32 a, pair;
    pair = r_u32(0x80087640 + index);
    a = r_u32(0x8008d494);
    w_u32(0x1f8003e0, 4096);
    w_u32(0x1f8003e4, 0);
    w_u16(0x1f8003e8, (uint16)(pair >> 16));
    w_u16(0x1f8003ea, (uint16)pair);
    w_u16(0x1f8003ec, 0);
    w_u16(0x1f8003ee, (uint16)(0u - (pair & 65535)));
    w_u32(0x1f8003f0, pair >> 16);
    ff_matrix_scratch_columns(a);
}

GDB_CALL void FUN_800101CC(sint32 angle)
{
    uint32 index;
    if ((uint32)angle == 0x80000000u)
        ff_wip_stop(__FUNCTION__, __FILE__, __LINE__);
    index = ((0u - (uint32)angle) >> 2) & 0x3ffc;
    if (index)
        ff_pitch_index_800101E0(index);
}

/* RTPT retains V1/V2 and exposes V0 through SXY0/SZ1 */
GDB_CALL sint32 ff_project_local(sint32 x, sint32 y, sint32 z, sint32 dest[4])
{
    uint32 a = r_u32(0x8008d494), words[8], i, packed, depth;
    sint32 sx, sy, result;
    for (i = 0; i < 8; ++i)
        words[i] = r_u32(a + i * 4);
    for (i = 0; i < 5; ++i)
        xport_gte_write_control(i, words[i]);
    for (i = 5; i < 8; ++i)
        xport_gte_write_control(i, (uint32)((sint32)words[i] >> 16));
    xport_gte_write_data(0, ((uint32)y << 16) | ((uint32)x & 65535));
    xport_gte_write_data(1, (uint32)z);
    xport_gte_execute(0x280030);
    depth = xport_gte_read_data(17) & 65535;
    packed = xport_gte_read_data(12);
    sx = (sint16)packed;
    sy = (sint16)(packed >> 16);
    result = depth < 40 ? 4 : ((uint16)sx < 320 && (uint16)sy < 256 ? 0 : 1);
    dest[0] = sx;
    dest[1] = sy;
    dest[2] = (sint16)depth;
    dest[3] = result;
    return result;
}

GDB_CALL uint32 FUN_800112A4(uint32 unused, uint32 column, uint32 row)
{
    FUNCTION_MARKER(0x800112a4u, "GAME.EXE");
    uint32 r, n, packed, xy = 0x001fe800, depths = 0x1f800000, p, ot, head, offset = 0, cell, texture, sx[4], uv[3], color;
    uint32 matrix, words[8], projected_depths[3];
    sint32 z = -2048, cutoff, depth, delta, fade;
    sint64 difference;
    int i;
    column <<= 1;
    row <<= 7;
    matrix = r_u32(0x8008d494);
    for (i = 0; i < 5; i++)
        words[i] = r_u32(matrix + 4 * i);
    for (i = 0; i < 5; i++)
        xport_gte_write_control(i, words[i]);
    for (i = 5; i < 8; i++)
        words[i] = r_u32(matrix + 4 * i);
    for (i = 5; i < 8; i++)
        xport_gte_write_control(i, (uint32)((sint32)words[i] >> 16));
    /* Six RTPT triples per row with a seventeen-point stride */
    for (r = 0; r < 17; ++r, z += 256)
    {
        xport_gte_write_data(1, (uint32)z);
        xport_gte_write_data(3, (uint32)z);
        xport_gte_write_data(5, (uint32)z);
        packed = 0xf800;
        for (n = 0; n < 6; n++)
        {
            for (i = 0; i < 3; i++)
            {
                xport_gte_write_data(2 * i, packed);
                packed += 256;
            }
            xport_gte_execute(0x280030);
            for (i = 0; i < 3; i++)
                w_u32(xy + 4 * i, xport_gte_read_data(12 + i));
            for (i = 0; i < 3; i++)
                projected_depths[i] = xport_gte_read_data(17 + i);
            for (i = 0; i < 3; i++)
                w_u16(depths + 2 * i, (uint16)projected_depths[i]);
            xy += 12;
            depths += 6;
        }
        xy -= 4;
        depths -= 2;
    }
    color = r_u32(0x8008d4bc);
    ot = r_u32(0x8008d4b4) + 8;
    p = r_u32(0x8008d4b0);
    head = r_u32(ot);
    cutoff = (sint32)r_u32(0x80011590);
    for (r = 0; r < 16; ++r)
    {
        for (n = 0; n < 16; ++n)
        {
            column &= 0x7e;
            row &= 0x1f80;
            cell = r_u16(0x8009f870 + column + row);
            if (cell != 0xfffc && r_s16(0x1f800000 + (offset >> 1)) > 0)
            {
                depth = r_s16(0x1f800024 + (offset >> 1));
                if (depth > 0)
                {
                    sx[0] = r_u32(0x001fe800 + offset);
                    sx[1] = r_u32(0x001fe804 + offset);
                    color = 0x2d000000;
                    difference = (sint64)depth - cutoff;
                    if (difference < -2147483647 - 1 || difference > 2147483647)
                        ff_wip_stop(__FUNCTION__, __FILE__, __LINE__);
                    delta = (sint32)difference;
                    /* The ADDI in the rejection branch delay slot still executes */
                    difference += 2048;
                    if (difference > 2147483647)
                        ff_wip_stop(__FUNCTION__, __FILE__, __LINE__);
                    fade = (sint32)difference;
                    if (delta > 0)
                        goto next_faded_tile;
                    if (fade >= 0)
                    {
                        uint32 shade = 128 - (uint32)(fade >> 4);
                        color = 0x2c000000 | shade | (shade << 8) | (shade << 16);
                    }
                    sx[2] = r_u32(0x001fe844 + offset);
                    sx[3] = r_u32(0x001fe848 + offset);
                    /* Horizontal rejection still executes this delay-slot store */
                    w_u32(p + 8, sx[0]);
                    if ((sx[0] & 65535) < 320 || (sx[1] & 65535) < 320 || (sx[2] & 65535) < 320 || (sx[3] & 65535) < 320)
                    {
                        w_u32(p + 16, sx[1]);
                        w_u32(p + 24, sx[2]);
                        w_u32(p + 32, sx[3]);
                        texture = 0x800a8768 + 4 * cell;
                        uv[0] = r_u32(texture);
                        uv[1] = r_u32(texture + 4);
                        uv[2] = r_u32(texture + 8);
                        w_u32(p + 12, uv[0]);
                        w_u32(p + 20, uv[1]);
                        w_u16(p + 28, (uint16)uv[2]);
                        w_u16(p + 36, (uint16)(uv[2] >> 16));
                        w_u32(p + 4, color);
                        w_u32(p, head | 0x09000000);
                        head = p;
                        p += 40;
                    }
                }
            }
        next_faded_tile:
            column -= 2;
            offset += 4;
        }
        column += 32;
        row -= 128;
        offset += 4;
    }
    w_u32(ot, head);
    w_u32(0x8008d4bc, 0x2d000000);
    w_u32(0x8008d4b0, p);
    return 0x8008d4b0;
}

void ff_sky_occluder_80065B3C(void)
{
    static const uint32 xs[4] = {0xfe000000u, 0x02000000, 0xfa000000u, 0x06000000};
    sint32 points[4][4];
    uint32 i, p;
    FUN_80011D20();
    FUN_800101CC(r_s16(0x8008d492));
    FUN_8001033C(r_s16(0x8008d490));
    for (i = 0; i < 4; ++i)
    {
        uint32 camera_x, camera_y, camera_z = 0;
        FUN_80010040();
        camera_x = r_u32(0x8008d4a4);
        camera_y = r_u32(0x8008d4a8);
        if (i < 2)
            camera_z = r_u32(0x8008d4ac);
        FUN_800100C4((sint32)(xs[i] - camera_x), (sint32)(0 - camera_y), (sint32)(i < 2 ? 0x07000000 - camera_z : 0x0fff0000));
        ff_project_local(0, 0, 0, points[i]);
        FUN_80010028();
    }
    p = r_u32(0x8008d4b0);
    w_u32(0x8008d4b0, p + 24);
    setPolyF4(psx_addr(p, 24));
    w_u8(p + 4, 0);
    w_u8(p + 5, 0);
    w_u8(p + 6, 0);
    for (i = 0; i < 4; ++i)
    {
        w_u16(p + 8 + 4 * i, (uint16)points[i][0]);
        w_u16(p + 10 + 4 * i, (uint16)((uint32)points[i][1] - (i < 2 ? 1 : 0)));
    }
    FUN_80011E40(r_u32(0x8008d4b4), p);
}

GDB_CALL uint32 FUN_80064D34(uint32 left, uint32 right)
{
    FUNCTION_MARKER(0x80064d34u, "GAME.EXE");
    sint32 points[4][4];
    uint32 i, p;
    FUN_80011D20();
    FUN_800101CC(r_s16(0x8008d492));
    FUN_8001033C(r_s16(0x8008d490));
    for (i = 0; i < 4; ++i)
    {
        uint32 camera_x, camera_y;
        FUN_80010040();
        camera_x = r_u32(0x8008d4a4);
        camera_y = r_u32(0x8008d4a8);
        FUN_800100C4((sint32)(((i & 1) ? right : left) - camera_x), (sint32)(0 - camera_y), i < 2 ? 0x7fff0000 : 0x0fff0000);
        ff_project_local(0, 0, 0, points[i]);
        FUN_80010028();
    }
    p = r_u32(0x8008d4b0);
    w_u32(0x8008d4b0, p + 24);
    setPolyF4(psx_addr(p, 24));
    w_u8(p + 4, 0);
    w_u8(p + 5, 0);
    w_u8(p + 6, 0);
    for (i = 0; i < 4; ++i)
    {
        w_u16(p + 8 + 4 * i, (uint16)points[i][0]);
        w_u16(p + 10 + 4 * i, (uint16)points[i][1]);
    }
    return FUN_80011E40(r_u32(0x8008d4b4), p);
}

GDB_CALL sint32 FUN_800120BC(sint32 x, sint32 y, sint32 z, uint32 dest)
{
    sint32 position[4], result = ff_project_local(x, y, z, position);
    w_u32(dest, (uint32)position[0]);
    w_u32(dest + 4, (uint32)position[1]);
    w_u32(dest + 8, (uint32)position[2]);
    w_u32(dest + 12, (uint32)position[3]);
    return result;
}

/* Original 58844..5887C horizon projection uses native caller-local output */
GDB_CALL void frame_project_horizon(void)
{
    sint32 position[4];
    FUN_80011D20();
    FUN_800101CC(r_s16(0x8008d492));
    ff_project_local(0, 0, 32767, position);
    w_u32(0x8008d4d0, (uint32)position[1]);
}

/* 80011D50: forward OT, 2048 entries; raw pointer stores retain alias bits.
 * Valid RAM ranges cannot trigger the original signed ADDI overflow. */
GDB_CALL sint32 FUN_80011D50(uint32 table)
{
    uint32 i;
    psx_addr(table, 8192);
    for (i = 0; i < 2047; i++)
        w_u32(table + 4u * i, table + 4u * (i + 1));
    w_u32(table + 8188, 0x10018);
    return 0x10018;
}

/* 80011D9C: skip empty links, retaining the predecessor node. In the final
 * empty run, the store uses the last node address, not the terminal's address.
 * Well-formed DMA lists are required, as in the original. */
GDB_CALL sint32 FUN_80011D9C(void)
{
    uint32 current = r_u32(0x8008d4b4), previous, tag, next, node;
    for (;;)
    {
        previous = current;
        tag = r_u32(current);
        next = tag & 0xffffff;
        if (next == 0x10018)
            return 0x10018;
        current = next;
        if (tag & 0xff000000)
            continue;
        for (;;)
        {
            node = current;
            tag = r_u32(current);
            next = tag & 0xffffff;
            if (next == 0x10018)
            {
                w_u32(previous, current);
                return 0x10018;
            }
            current = next;
            if (tag & 0xff000000)
                break;
        }
        w_u32(previous, (r_u32(previous) & 0xff000000) | node);
    }
}

static uint32 ff_matrix_pointer_add(uint32 a, sint32 delta)
{
    sint64 n = (sint64)(sint32)a + delta;
    if (n < -2147483647LL - 1 || n > 2147483647LL)
        ff_wip_stop(__FUNCTION__, __FILE__, __LINE__);
    return (uint32)n;
}

static void ff_matrix_load_gte(const uint32 words[8])
{
    int i;
    for (i = 0; i < 5; i++)
        xport_gte_write_control(i, words[i]);
    for (i = 5; i < 8; i++)
        xport_gte_write_control(i, (uint32)((sint32)words[i] >> 16));
}

/* 80011F74: load all eight words before stores, then reload COP2 controls. */
GDB_CALL void FUN_80011F74(uint32 source)
{
    uint32 target = r_u32(0x8008d494), words[8];
    int i;
    for (i = 0; i < 8; i++)
        words[i] = r_u32(source + 4 * i);
    for (i = 0; i < 8; i++)
        w_u32(target + 4 * i, words[i]);
    ff_matrix_load_gte(words);
}

/* 80010170: reload geometry registers from the current matrix, without moving
 * its pointer or modifying the matrix. Translation fractions are discarded. */
GDB_CALL void FUN_80010170(void)
{
    uint32 source = r_u32(0x8008d494), words[8];
    int i;
    for (i = 0; i < 8; i++)
        words[i] = r_u32(source + 4 * i);
    ff_matrix_load_gte(words);
}

/* 80010028 only advances the pointer: it does not reload GTE registers. */
GDB_CALL uint32 FUN_80010028(void)
{
    uint32 a = ff_matrix_pointer_add(r_u32(0x8008d494), 32);
    w_u32(0x8008d494, a);
    return a;
}

/* 80010040 snapshots all eight source words before writing the lower slot. */
GDB_CALL uint32 FUN_80010040(void)
{
    uint32 a = r_u32(0x8008d494), b, words[8];
    int i;
    for (i = 0; i < 8; i++)
        words[i] = r_u32(a + 4 * i);
    b = ff_matrix_pointer_add(a, -32);
    for (i = 0; i < 8; i++)
        w_u32(b + 4 * i, words[i]);
    ff_matrix_load_gte(words);
    w_u32(0x8008d494, b);
    return b;
}

/* 800100C4: MVMVA results use MAC1..3, not saturated IR1..3. Translation
 * fractions are discarded before the operation; stores wrap after SLL 16. */
GDB_CALL uint32 FUN_800100C4(sint32 x, sint32 y, sint32 z)
{
    uint32 a = r_u32(0x8008d494), words[8], result[3];
    int i;
    for (i = 0; i < 8; i++)
        words[i] = r_u32(a + 4 * i);
    ff_matrix_load_gte(words);
    xport_gte_write_data(0, ((uint32)x >> 16) | ((uint32)y & 0xffff0000));
    xport_gte_write_data(1, (uint32)(z >> 16));
    xport_gte_execute(0x480012);
    for (i = 0; i < 3; i++)
        result[i] = xport_gte_read_data(25 + i) << 16;
    for (i = 0; i < 3; i++)
        w_u32(a + 20 + 4 * i, result[i]);
    return a;
}

/* 8001033C: same column multiply as pitch, with a different scratch matrix. */
GDB_CALL void ff_yaw_index_80010350(uint32 index)
{
    uint32 pair, a;
    pair = r_u32(0x80087640 + index);
    a = r_u32(0x8008d494);
    w_u32(0x1f8003e0, pair >> 16);
    w_u32(0x1f8003e4, (0u - pair) & 65535);
    w_u32(0x1f8003e8, 4096);
    w_u32(0x1f8003ec, pair & 65535);
    w_u32(0x1f8003f0, pair >> 16);
    ff_matrix_scratch_columns(a);
}

GDB_CALL void FUN_8001033C(sint32 angle)
{
    uint32 index;
    if ((uint32)angle == 0x80000000u)
        ff_wip_stop(__FUNCTION__, __FILE__, __LINE__);
    index = ((0u - (uint32)angle) >> 2) & 0x3ffc;
    if (index)
        ff_yaw_index_80010350(index);
}

/* 800104B4 is also called directly by the model stream with a table offset. */
GDB_CALL void ff_roll_index_800104B4(uint32 index)
{
    uint32 pair, a;
    pair = r_u32(0x80087640 + index);
    a = r_u32(0x8008d494);
    w_u16(0x1f8003e0, (uint16)(pair >> 16));
    w_u16(0x1f8003e2, (uint16)(0u - (pair & 65535)));
    w_u16(0x1f8003e4, 0);
    w_u16(0x1f8003e6, (uint16)pair);
    w_u32(0x1f8003e8, pair >> 16);
    w_u32(0x1f8003ec, 0);
    w_u32(0x1f8003f0, 4096);
    ff_matrix_scratch_columns(a);
}

GDB_CALL void FUN_800104A0(sint32 angle)
{
    uint32 index;
    if ((uint32)angle == 0x80000000u)
        ff_wip_stop(__FUNCTION__, __FILE__, __LINE__);
    index = ((0u - (uint32)angle) >> 2) & 0x3ffc;
    if (index)
        ff_roll_index_800104B4(index);
}

static void ff_model_rotations(uint32 source, int halfwords)
{
    uint32 roll, yaw, pitch;
    roll = halfwords ? (uint32)(sint32)r_s16(source) : r_u32(source);
    if (roll)
        ff_roll_index_800104B4(roll << 2);
    yaw = halfwords ? (uint32)(sint32)r_s16(source + 2) : r_u32(source + 4);
    if (yaw)
    {
        if (yaw == 0x80000000u)
            ff_wip_stop(__FUNCTION__, __FILE__, __LINE__);
        ff_yaw_index_80010350(((0u - yaw) & 0xfff) << 2);
    }
    pitch = halfwords ? (uint32)(sint32)r_s16(source + 4) : r_u32(source + 8);
    if (pitch)
    {
        if (pitch == 0x80000000u)
            ff_wip_stop(__FUNCTION__, __FILE__, __LINE__);
        ff_pitch_index_800101E0(((0u - pitch) & 0xfff) << 2);
    }
}

/* Model stream transform phase through 80010AB8
 * FUN_8001060C performs the final matrix pop and shared packet phase */
GDB_CALL void ff_model_transform_8001060C(uint32 model, uint32 pose)
{
    uint32 stream, counts = model + 32, vertices, index = 0, command;
    sint32 angle;
    r_u32(model + 4);
    vertices = r_u32(model);
    stream = model + r_u32(model + 24);
    FUN_80010040();
    angle = r_s16(pose + 220);
    if (angle)
        ff_pitch_index_800101E0(((uint32)angle & 0xfff) << 2);
    angle = r_s16(pose + 218);
    if (angle)
        ff_yaw_index_80010350(((uint32)angle & 0xfff) << 2);
    angle = r_s16(pose + 216);
    if (angle)
        ff_roll_index_800104B4(((uint32)angle & 0xfff) << 2);
    command = r_u32(stream);
    stream += 4;
    for (;;)
    {
        uint32 flag, bone, count, matrix, out = 0, words[8], xyz[3];
        int i;
        if (command == 1)
        {
            FUN_80010040();
            goto next_command;
        }
        if (command == 2)
        {
            FUN_80010028();
            goto next_command;
        }
        flag = r_u32(stream);
        matrix = r_u32(0x8008d494);
        for (i = 0; i < 5; i++)
            words[i] = r_u32(matrix + 4 * i);
        for (i = 0; i < 5; i++)
            xport_gte_write_control(i, words[i]);
        for (i = 5; i < 8; i++)
            words[i] = r_u32(matrix + 4 * i);
        for (i = 5; i < 8; i++)
            xport_gte_write_control(i, (uint32)((sint32)words[i] >> 16));
        for (i = 0; i < 3; i++)
            xyz[i] = r_u32(stream + 4 + 4 * i);
        stream += 16;
        xport_gte_write_data(0, (xyz[0] >> 16) | (xyz[1] & 0xffff0000));
        xport_gte_write_data(1, (uint32)((sint32)xyz[2] >> 16));
        xport_gte_execute(0x480012);
        matrix = r_u32(0x8008d494);
        for (i = 0; i < 3; i++)
            xyz[i] = xport_gte_read_data(25 + i) << 16;
        for (i = 0; i < 3; i++)
            w_u32(matrix + 20 + 4 * i, xyz[i]);
        ff_model_rotations(stream, 0);
        stream += 12;
        bone = pose + 6 * r_u32(stream);
        stream += 4;
        ff_model_rotations(bone + 216, 1);
        if (flag == 1)
        {
            ff_yaw_index_80010350(r_u32(pose) << 2);
            out = 204;
        }
        else if (flag == 2 || flag == 3 || flag == 10 || flag == 11)
        {
            out = flag == 2 ? 4 : flag == 3 ? 36 : flag == 10 ? 68 : 100;
            matrix = r_u32(0x8008d494);
            for (i = 0; i < 8; i++)
                words[i] = r_u32(matrix + 4 * i);
            for (i = 0; i < 8; i++)
                w_u32(pose + out + 4 * i, words[i]);
            out = 0;
        }
        else if (flag >= 4 && flag <= 9)
            out = 132 + 12 * (flag - 4);
        if (out)
        {
            matrix = r_u32(0x8008d494);
            for (i = 0; i < 3; i++)
                xyz[i] = r_u32(matrix + 20 + 4 * i);
            for (i = 0; i < 3; i++)
                w_u32(pose + out + 4 * i, xyz[i]);
        }
        count = r_u32(counts);
        counts += 4;
        if (count)
        {
            uint32 vertex, xy, depth_out, remaining = count, depths[3];
            matrix = r_u32(0x8008d494);
            for (i = 0; i < 8; i++)
                words[i] = r_u32(matrix + 4 * i);
            for (i = 0; i < 5; i++)
                xport_gte_write_control(i, words[i]);
            for (i = 5; i < 8; i++)
                xport_gte_write_control(i, (uint32)((sint32)words[i] >> 16));
            vertex = vertices + 8 * index;
            xy = 0x1fe800 + 4 * index;
            depth_out = 0x1fe400 + 2 * index;
            index += count;
            do
            {
                for (i = 0; i < 6; i++)
                    xport_gte_write_data(i, r_u32(vertex + 4 * i));
                xport_gte_execute(0x280030);
                vertex += 24;
                if ((sint32)remaining < -2147483645)
                    ff_wip_stop(__FUNCTION__, __FILE__, __LINE__);
                remaining -= 3;
                for (i = 0; i < 3; i++)
                    w_u32(xy + 4 * i, xport_gte_read_data(12 + i));
                for (i = 0; i < 3; i++)
                    depths[i] = xport_gte_read_data(17 + i);
                for (i = 0; i < 3; i++)
                    w_u16(depth_out + 2 * i, (uint16)depths[i]);
                xy += 12;
                depth_out += 6;
            } while ((sint32)remaining > 0);
        }
        ff_model_rotations(stream, 0);
        stream += 12;
    next_command:
        command = r_u32(stream);
        stream += 4;
        if (!command)
            break;
    }
}

/* Shared polygon phase. Signed packed XY tests and line bucket alias behavior
 * deliberately follow MIPS rather than a generic polygon clipping policy. */
GDB_CALL uint32 ff_model_packets_80010CA0(uint32 count, uint32 records, uint32 xy, uint32 depths, sint32 bias, sint32 depth_carrier)
{
    uint32 packet, ot, table, color, blend, blend_override;
    table = r_u32(0x8008d4d8);
    color = r_u32(0x8008d4bc);
    blend = r_u32(0x8008d4c0);
    blend_override = ff_matrix_pointer_add(blend, -1) << 21;
    ot = r_u32(0x8008d4b4);
    packet = r_u32(0x8008d4b0);
    if (!count)
        ff_wip_stop(__FUNCTION__, __FILE__, __LINE__);
    do
    {
        uint32 first, second, i0, i1, i2, i3;
        sint32 depth;
        uint32 a, b, c, d, bucket, texture, uv0, uv1, uv2, flags;
        second = r_u32(records + 4);
        first = r_u32(records);
        i0 = first & 65535;
        i1 = first >> 16;
        i2 = second & 65535;
        i3 = second >> 16;
        if ((sint32)second < 0)
        {
            sint32 z0 = r_s16(ff_matrix_pointer_add(depths, (sint32)(i0 >> 1)));
            sint32 z1 = r_s16(ff_matrix_pointer_add(depths, (sint32)(i1 >> 1)));
            uint32 first_xy, second_xy, link, next;
            count = ff_matrix_pointer_add(count, -1);
            /* The overwritten ADD still checks the previous T7 for overflow */
            ff_matrix_pointer_add((uint32)depth_carrier, bias);
            depth_carrier = depth = (z0 + z1) >> 3;
            first_xy = ff_matrix_pointer_add(xy, (sint32)i0);
            if (depth > 8188)
                goto next_record;
            second_xy = ff_matrix_pointer_add(xy, (sint32)i1);
            if (depth <= 0)
                goto next_record;
            flags = r_u32(records + 8);
            a = r_u32(first_xy);
            b = r_u32(second_xy);
            depth_carrier = r_s16(ff_matrix_pointer_add(table, depth << 1));
            w_u32(packet + 4, flags);
            w_u32(packet + 8, a);
            w_u32(packet + 12, b);
            bucket = ff_matrix_pointer_add(ot, depth_carrier);
            link = r_u32(bucket);
            records = ff_matrix_pointer_add(records, 12);
            w_u32(packet, link | 0x03000000);
            next = ff_matrix_pointer_add(packet, 16);
            w_u32(bucket, packet);
            packet = next;
            continue;
        }
        else
        {
            sint32 z0, z2, clip;
            uint32 first_xy, next;
            z0 = r_s16(ff_matrix_pointer_add(depths, (sint32)(i0 >> 1)));
            count = ff_matrix_pointer_add(count, -1);
            if (z0 <= 0)
                goto next_record;
            z2 = r_s16(ff_matrix_pointer_add(depths, (sint32)(i2 >> 1)));
            if (z2 <= 0)
                goto next_record;
            depth_carrier = depth = (sint32)ff_matrix_pointer_add((uint32)(z0 + z2), bias) >> 3;
            first_xy = ff_matrix_pointer_add(xy, (sint32)i0);
            if (depth > 8192)
                goto next_record;
            a = r_u32(first_xy);
            b = r_u32(ff_matrix_pointer_add(xy, (sint32)i1));
            c = r_u32(ff_matrix_pointer_add(xy, (sint32)i2));
            d = r_u32(ff_matrix_pointer_add(xy, (sint32)i3));
            if ((a & 65535) >= 320 && (b & 65535) >= 320 && (c & 65535) >= 320 && (d & 65535) >= 320)
                goto next_record;
            xport_gte_write_data(12, a);
            xport_gte_write_data(14, c);
            if ((sint32)a <= 0 && (sint32)b <= 0 && (sint32)c <= 0 && (sint32)d < 0)
                goto next_record;
            xport_gte_write_data(13, b);
            depth_carrier = r_s16(ff_matrix_pointer_add(table, (sint32)((uint32)depth << 1)));
            xport_gte_execute(0x140006);
            flags = r_u32(records + 8);
            clip = (sint32)xport_gte_read_data(24);
            if (!(flags & 1) && clip < 0)
                goto next_record;
            texture = ff_matrix_pointer_add(0x800a8768, (sint32)(flags >> 16));
            if (flags & 2)
            {
                w_u32(packet + 8, b);
                w_u32(packet + 16, a);
                w_u32(packet + 24, c);
                w_u32(packet + 32, d);
            }
            else
            {
                w_u32(packet + 8, a);
                w_u32(packet + 16, b);
                w_u32(packet + 24, d);
                w_u32(packet + 32, c);
            }
            uv0 = r_u32(texture);
            uv1 = r_u32(texture + 4);
            uv2 = r_u32(texture + 8);
            if (blend)
                uv1 = (uv1 & 0xff9fffff) | blend_override;
            w_u32(packet + 12, uv0 - (uv0 & 1));
            w_u32(packet + 20, uv1);
            w_u16(packet + 28, (uint16)uv2);
            w_u16(packet + 36, (uint16)(uv2 >> 16));
            w_u32(packet + 4, color | ((uv0 & 1) << 25));
            bucket = ff_matrix_pointer_add(ot, depth_carrier);
            w_u32(packet, r_u32(bucket) | 0x09000000);
            next = ff_matrix_pointer_add(packet, 40);
            w_u32(bucket, packet & 0xffffff);
            packet = next;
        }
    next_record:
        records = ff_matrix_pointer_add(records, 12);
    } while (count);
    w_u32(0x8008d4bc, 0x2d000000);
    w_u32(0x8008d4c0, 0);
    w_u32(0x8008d4b0, packet);
    return 0x8008d4b0;
}

GDB_CALL uint32 FUN_8001060C(uint32 model, uint32 pose)
{
    uint32 records, count;
    ff_model_transform_8001060C(model, pose);
    FUN_80010028();
    records = r_u32(model + 8);
    count = r_u32(model + 12);
    /* With zero bias the overwritten ADD cannot observe the incoming carrier */
    return ff_model_packets_80010CA0(count, records, 0x1fe800, 0x1fe400, 0, 0);
}

/* Static entry pipelines depth stores behind the next RTPT triple
 * The original always projects at least two triples */
GDB_CALL uint32 ff_static_model_80010B18(uint32 model, uint32 unused, sint32 bias)
{
    sint32 count, below127, below193, below385 = 0;
    uint32 matrix, words[8], xy, depths, vertices, xy_out, depth_out;
    uint32 previous_depths[3], remaining, records, packets;
    int i;
    matrix = r_u32(0x8008d494);
    for (i = 0; i < 5; i++)
        words[i] = r_u32(matrix + 4 * i);
    for (i = 0; i < 5; i++)
        xport_gte_write_control(i, words[i]);
    for (i = 5; i < 8; i++)
        words[i] = r_u32(matrix + 4 * i);
    for (i = 5; i < 8; i++)
        xport_gte_write_control(i, (uint32)((sint32)words[i] >> 16));
    count = r_s32(model + 4);
    vertices = r_u32(model);
    below127 = (sint32)ff_matrix_pointer_add((uint32)count, -127);
    below193 = (sint32)ff_matrix_pointer_add((uint32)count, -193);
    if (below127 >= 0)
        below385 = (sint32)ff_matrix_pointer_add((uint32)count, -385);
    xy = below127 < 0 || below193 < 0 ? 0x1f800000 : 0x1fe800;
    depths = below127 < 0 ? 0x1f800200 : below193 < 0 ? 0x1fe400 : below385 < 0 ? 0x1f800000 : 0x1fe400;
    xy_out = ff_matrix_pointer_add(xy, 12);
    depth_out = ff_matrix_pointer_add(depths, 6);
    for (i = 0; i < 6; i++)
        xport_gte_write_data(i, r_u32(vertices + 4 * i));
    xport_gte_execute(0x280030);
    vertices = ff_matrix_pointer_add(vertices, 24);
    remaining = ff_matrix_pointer_add((uint32)count, -3);
    for (i = 0; i < 3; i++)
        w_u32(xy_out - 12 + 4 * i, xport_gte_read_data(12 + i));
    for (i = 0; i < 3; i++)
        previous_depths[i] = xport_gte_read_data(17 + i);
    do
    {
        for (i = 0; i < 6; i++)
            xport_gte_write_data(i, r_u32(vertices + 4 * i));
        xport_gte_execute(0x280030);
        vertices = ff_matrix_pointer_add(vertices, 24);
        for (i = 0; i < 3; i++)
            w_u16(depth_out - 6 + 2 * i, (uint16)previous_depths[i]);
        xy_out = ff_matrix_pointer_add(xy_out, 12);
        depth_out = ff_matrix_pointer_add(depth_out, 6);
        remaining = ff_matrix_pointer_add(remaining, -3);
        for (i = 0; i < 3; i++)
            w_u32(xy_out - 12 + 4 * i, xport_gte_read_data(12 + i));
        for (i = 0; i < 3; i++)
            previous_depths[i] = xport_gte_read_data(17 + i);
    } while ((sint32)remaining > 0);
    for (i = 0; i < 3; i++)
        w_u16(depth_out - 6 + 2 * i, (uint16)previous_depths[i]);
    records = r_u32(model + 8);
    packets = r_u32(model + 12);
    return ff_model_packets_80010CA0(packets, records, xy, depths, bias, (sint32)words[7] >> 16);
}

GDB_CALL uint32 FUN_80067B8C(uint32 model, uint32 unused, sint32 bias)
{
    return ff_static_model_80010B18(model, unused, bias);
}

/* 80011C60: Reset matrix stack to SCRATCHPAD, then pitch followed by yaw
 * The original uses unsigned halfword angle loads. Return is not consumed. */
GDB_CALL void FUN_80011C60(void)
{
    int i;
    w_u32(0x8008d494, 0x1f8003c0);
    for (i = 0; i < 8; i++)
        w_u32(0x1f8003c0 + 4 * i, (i == 0 || i == 2 || i == 4) ? 4096 : 0);
    FUN_800101CC((uint16)r_s16(0x8008d492));
    FUN_8001033C((uint16)r_s16(0x8008d490));
}

/* Terrain grid uses six RTPT triples per row and a 17-point row stride
 * Packed X increments carry into Y at zero as in the original */
GDB_CALL uint32 FUN_8001100C(sint32 unused, sint32 tile_x, sint32 tile_z)
{
    uint32 row, col, n, packed, p, ot, head, color, matrix, words[8], depths[3];
    uint32 sx = (uint32)tile_x << 1, sz = (uint32)tile_z << 7;
    int i;
    matrix = r_u32(0x8008d494);
    for (i = 0; i < 5; i++)
        words[i] = r_u32(matrix + 4 * i);
    for (i = 0; i < 5; i++)
        xport_gte_write_control(i, words[i]);
    for (i = 5; i < 8; i++)
        words[i] = r_u32(matrix + 4 * i);
    for (i = 5; i < 8; i++)
        xport_gte_write_control(i, (uint32)((sint32)words[i] >> 16));
    for (row = 0; row < 17; row++)
    {
        uint32 z = (uint32)-2048 + row * 256;
        xport_gte_write_data(1, z);
        xport_gte_write_data(3, z);
        xport_gte_write_data(5, z);
        packed = 0xf800;
        for (col = 0; col < 18; col += 3)
        {
            for (i = 0; i < 3; i++)
            {
                xport_gte_write_data(2 * i, packed);
                packed += 256;
            }
            xport_gte_execute(0x280030);
            n = row * 17 + col;
            for (i = 0; i < 3; i++)
                w_u32(0x1fe800 + 4 * (n + i), xport_gte_read_data(12 + i));
            for (i = 0; i < 3; i++)
                depths[i] = xport_gte_read_data(17 + i);
            for (i = 0; i < 3; i++)
                w_u16(0x1f800000 + 2 * (n + i), (uint16)depths[i]);
        }
    }
    color = r_u32(0x8008d4bc);
    ot = r_u32(0x8008d4b4);
    p = r_u32(0x8008d4b0);
    ot += 4;
    head = r_u32(ot);
    for (row = 0; row < 16; row++)
    {
        for (col = 0; col < 16; col++)
        {
            sint32 tile;
            uint32 v0, v1, v2, v3, t, uv, descriptor0, descriptor1;
            sx &= 0x7e;
            sz &= 0x1f80;
            tile = r_s16(0x8009f870 + sx + sz);
            n = row * 17 + col;
            if (tile != -4 && r_s16(0x1f800000 + 2 * n) > 0 && r_s16(0x1f800024 + 2 * n) > 0)
            {
                v0 = r_u32(0x1fe800 + 4 * n);
                v1 = r_u32(0x1fe804 + 4 * n);
                v2 = r_u32(0x1fe844 + 4 * n);
                v3 = r_u32(0x1fe848 + 4 * n);
                w_u32(p + 8, v0); /* branch delay slot also executes for horizontal rejection */
                if ((uint16)v0 < 320 || (uint16)v1 < 320 || (uint16)v2 < 320 || (uint16)v3 < 320)
                {
                    w_u32(p + 16, v1);
                    w_u32(p + 24, v2);
                    w_u32(p + 32, v3);
                    t = 0x800a8768 + ((uint32)tile << 2);
                    descriptor0 = r_u32(t);
                    descriptor1 = r_u32(t + 4);
                    uv = r_u32(t + 8);
                    w_u32(p + 12, descriptor0);
                    w_u32(p + 20, descriptor1);
                    w_u16(p + 28, (uint16)uv);
                    w_u16(p + 36, (uint16)(uv >> 16));
                    w_u32(p + 4, color);
                    w_u32(p, head | 0x09000000);
                    head = p;
                    p += 40;
                }
            }
            sx -= 2;
        }
        sx += 32;
        sz -= 128;
    }
    w_u32(ot, head);
    w_u32(0x8008d4bc, 0x2d000000);
    w_u32(0x8008d4b0, p);
    return 0x8008d4b0;
}

/* 80064150: update ten alternating 320-byte scanlines. MULT uses low 32 bits
 * before SAR, and division by 160 applies after wrapped SLL 14. */
static sint32 ff_bg_product(sint32 a, sint32 b)
{
    return (sint32)((uint32)a * (uint32)b) >> 8;
}

GDB_CALL sint32 FUN_80064150(void)
{
    uint32 parity, p = 0x800a1990;
    sint32 base_x = ~(sint32)r_s16(0x8008d4a6);
    sint32 height = r_s16(0x8008d4aa);
    sint32 base_z = ~(sint32)r_s16(0x8008d4ae);
    sint32 angle = (sint16)r_u16(0x8008d490) >> 5;
    sint32 sn = r_s16(0x800802d0 + 2 * ((uint32)angle & 2047));
    sint32 cs = r_s16(0x800802d0 + 2 * ((uint32)(angle + 512) & 2047));
    uint32 sn160 = (uint32)sn * 160, cs160 = (uint32)cs * 160, sn256 = (uint32)sn << 8, cs256 = (uint32)cs << 8;
    sint32 ax = (sint32)(0u - cs160 - sn256) >> 15, az = (sint32)(cs256 - sn160) >> 15;
    sint32 bx = (sint32)(cs160 - sn256) >> 15, bz = (sint32)(sn160 + cs256) >> 15;
    sint32 h = (sint32)((uint32)height << 8), row, col;
    parity = r_u32(0x8008d4c4) & 1;
    p += 320 * parity;
    for (row = 1; row < 20; row += 2)
    {
        sint32 q, x, z;
        parity = r_u32(0x8008d4c4) & 1;
        q = h / (row + (sint32)parity);
        x = ff_bg_product(q, ax);
        z = ff_bg_product(q, az);
        uint32 dx = (uint32)((sint32)(((uint32)ff_bg_product(q, bx) - (uint32)x) << 14) / 160) << 1;
        uint32 dz = (uint32)((sint32)(((uint32)ff_bg_product(q, bz) - (uint32)z) << 14) / 160) << 1;
        uint32 u = ((uint32)x + (uint32)base_x) << 16, v = ((uint32)z + (uint32)base_z) << 16;
        for (col = 0; col < 320; col++)
        {
            uint32 index = ((uint32)((sint32)u >> 23) & 127) + ((uint32)((sint32)v >> 16) & 0x3f80);
            w_u8(p++, r_u8(0x800a3290 + index));
            u += dx;
            v += dz;
        }
        p += 320;
    }
    return 0;
}

/* 80063EEC: four terrain grids and optional distant texture/quads. */
GDB_CALL sint32 FUN_80063EEC(void)
{
    uint32 cx, cz, x, z, tx, tz, direction, position[4];
    int i;
    cx = r_u32(0x8008d4a4);
    x = cx & 0xffffff;
    cz = r_u32(0x8008d4ac);
    z = cz & 0xffffff;
    direction = (uint32)((r_s16(0x8008d490) + 2048) >> 12) & 15;
    tx = (uint32)((sint32)~cx >> 24);
    tz = (uint32)((sint32)~cz >> 24);
    if (r_s16(0x8008d492) < 8192)
    {
        uint32 dx, dz;
        dx = r_u32(0x80093a90 + 4 * direction);
        x += dx << 23;
        dz = r_u32(0x80093ad0 + 4 * direction);
        z += dz << 23;
        tx += (uint32)((sint32)dx >> 1);
        tz += (uint32)((sint32)dz >> 1);
    }
    FUN_80011C60();
    for (i = 0; i < 4; i++)
    {
        FUN_80010040();
        FUN_800100C4((sint32)((i & 1 ? 0xf8010000u : 0x07ff0000u) - x), (sint32)(0u - r_u32(0x8008d4a8)), (sint32)((i & 2 ? 0xf8010000u : 0x07ff0000u) - z));
        FUN_8001100C(0, (sint32)(tx + (i & 1 ? 16 : 0)), (sint32)(tz + (i & 2 ? 16 : 0)));
        FUN_80010028();
    }
    if (r_u32(0x80093d58) == 0xffffffff)
        return -1;
    FUN_80064150();
    position[0] = 0;
    position[1] = r_u32(0x8008d4d0);
    position[2] = 32767;
    position[3] = 21;
    ff_stack_height_sprite_800128F4(position, r_s16(0x800af4b2) + 256);
    position[0] += 256;
    return (sint32)ff_stack_height_sprite_800128F4(position, r_s16(0x800af4b2) + 257);
}

/* 80058E24: four menu actors, their shadows and selected-player rings. */
GDB_CALL sint32 FUN_80058E24(uint32 selected, sint32 players)
{
    uint32 i, a, phase, fade, actor_x, actor_y, actor_z;
    uint32 camera_x, camera_y, camera_z, height, blue, green;
    int slot, changed;
    sint32 red, value, type;
    if (r_s16(0x8008d492) >= 4097)
        return 0;
    FUN_80011D20();
    FUN_800101CC(r_s16(0x8008d492));
    FUN_8001033C(r_s16(0x8008d490));
    w_u32(0x80093580, (r_u32(0x80093580) + 64) & 2047);
    for (i = 0; i < 4; i++)
    {
        a = 0x800b4318 + 244 * i;
        changed = 0;
        slot = players > 0 && i == r_u32(selected) ? 0 : players >= 2 && i == r_u32(selected + 4) ? 1 : -1;
        if (slot >= 0)
        {
            if (!(r_u32(0x8009272c) & (1u << slot)))
            {
                phase = r_u32(0x80093580);
                value = (sint16)r_u16(0x800802d0 + 2 * phase) >> 10;
                changed = 1;
                w_u8(a + 159, (uint8)value);
                w_u8(a + 158, (uint8)value);
                w_u8(a + 157, (uint8)value);
            }
            else
            {
                red = r_s8(a + 157);
                if (red < 32)
                {
                    value = (sint8)(red + 4);
                    w_u8(a + 157, (uint8)value);
                    if (value > 32)
                        w_u8(a + 157, 32);
                    value = r_u8(a + 157);
                    w_u8(a + 159, (uint8)value);
                    w_u8(a + 158, (uint8)value);
                    changed = 1;
                }
            }
        }
        else
        {
            fade = r_u32(0x8009278c);
            if (fade)
            {
                fade <<= 2;
                value = (sint32)fade >= 129 ? -128 : (sint32)(0u - fade);
                w_u8(a + 159, (uint8)value);
                w_u8(a + 158, (uint8)value);
                w_u8(a + 157, (uint8)value);
                changed = 1;
            }
            else
            {
                red = r_s8(a + 157);
                if (red > 0)
                {
                    w_u8(a + 157, (uint8)(red - 1));
                    w_u8(a + 158, r_u8(a + 158) - 1);
                    w_u8(a + 159, r_u8(a + 159) - 1);
                    w_u8(a + 156, r_u8(a + 156) | 1);
                    red = r_s8(a + 157);
                }
                if (red < 0)
                {
                    w_u8(a + 157, (uint8)(red + 1));
                    w_u8(a + 158, r_u8(a + 158) + 1);
                    w_u8(a + 159, r_u8(a + 159) + 1);
                    changed = 1;
                }
            }
        }
        if (changed)
            w_u8(a + 156, r_u8(a + 156) | 1);
        FUN_80010040();
        actor_x = r_u32(a + 40);
        camera_x = r_u32(0x8008d4a4);
        type = r_s16(a + 52);
        actor_y = r_u32(a + 44);
        height = r_u32(0x800990d8 + 4 * (uint32)type);
        camera_y = r_u32(0x8008d4a8);
        actor_z = r_u32(a + 48);
        camera_z = r_u32(0x8008d4ac);
        FUN_800100C4((sint32)(actor_x - camera_x), (sint32)(actor_y + height - camera_y), (sint32)(actor_z - camera_z));
        FUN_8001033C((sint16)(0u - 32u * (uint32)(sint32)r_s16(a + 56)) + 32768);
        actor_y = r_u32(a + 16);
        actor_z = r_u32(a + 32);
        FUN_800100C4(0, (sint32)actor_y, (sint32)actor_z);
        blue = (uint32)(r_s8(a + 159) + 128);
        green = (uint32)(r_s8(a + 158) + 128);
        red = r_s8(a + 157);
        w_u32(0x8008d4bc, 0x2c000000u + (uint32)(red + 128) + (green << 8) + (blue << 16));
        w_u32(0x8008d4c0, 0);
        FUN_8001060C(r_u32(0x80081720 + 4 * (sint32)r_s16(a + 52)), 0x800ba210 + 504 * i);
        FUN_80010028();
        FUN_80010040();
        actor_x = r_u32(a + 40);
        camera_x = r_u32(0x8008d4a4);
        camera_y = r_u32(0x8008d4a8);
        actor_z = r_u32(a + 48);
        camera_z = r_u32(0x8008d4ac);
        FUN_800100C4((sint32)(actor_x - camera_x), (sint32)(0u - camera_y), (sint32)(actor_z - camera_z));
        FUN_800101CC(32768);
        FUN_8001033C((sint16)(32u * (uint32)(sint32)r_s16(a + 56)));
        FUN_80067B8C(r_u32(0x80082490), 0, 0);
        FUN_80010028();
        w_u32(0x8009357c, (r_u32(0x8009357c) + 8) & 2047);
        slot = players > 0 && i == r_u32(selected) ? 0 : players >= 2 && i == r_u32(selected + 4) ? 1 : -1;
        if (slot >= 0)
        {
            FUN_80010040();
            actor_x = r_u32(a + 40);
            camera_x = r_u32(0x8008d4a4);
            camera_y = r_u32(0x8008d4a8);
            actor_z = r_u32(a + 48);
            camera_z = r_u32(0x8008d4ac);
            FUN_800100C4((sint32)(actor_x - camera_x), (sint32)(0u - camera_y), (sint32)(actor_z - camera_z));
            FUN_800101CC(32768);
            FUN_8001033C((sint16)(32u * r_u32(0x8009357c)));
            FUN_80067B8C(r_u32(slot == 0 ? 0x800824fc : 0x80082500), 0, 0);
            FUN_80010028();
        }
    }
    return 0;
}

/* Shared rotation sequence with 8005B71C; that twin uses a different renderer. */
static void ff_object_rotations(uint32 object)
{
    sint32 angle = r_s16(object + 16);
    if (angle)
        FUN_8001033C(angle);
    angle = r_s16(object + 20);
    if (angle)
        FUN_800104A0(angle);
    angle = r_s16(object + 18);
    if (angle)
        FUN_800101CC(angle);
}

GDB_CALL uint32 FUN_8002A0C4(uint32 object)
{
    FUNCTION_MARKER(0x8002a0c4, "GAME.EXE");
    ff_object_rotations(object);
    return ff_static_model_80010B18(r_u32(0x80081720 + 4 * r_u32(object)), 0, 0);
}

GDB_CALL uint32 FUN_80029F48(void)
{
    FUNCTION_MARKER(0x80029f48, "GAME.EXE");
    uint32 model;
    if (r_s32(0x80093d60) < 3 || ((r_u16(0x8008d490) + 12288) & 65535) >= 24577)
        ff_static_model_80010B18(r_u32(0x8008180c), 0, 0);
    if (r_s32(0x80093d60) < 3)
        model = 56;
    else
        model = ((r_u16(0x8008d490) + 16384) & 65535) > 32768 ? 58 : 57;
    return ff_static_model_80010B18(r_u32(0x80081720 + 4u * model), 0, 0);
}

GDB_CALL uint32 FUN_8003262C(uint32 object)
{
    FUNCTION_MARKER(0x8003262c, "GAME.EXE");
    FUN_8001033C(r_s16(object + 16));
    return ff_static_model_80010B18(r_u32(0x80081720 + 4u * r_u32(object)), 0, 128);
}

GDB_CALL uint32 FUN_8002A048(uint32 object)
{
    FUNCTION_MARKER(0x8002a048, "GAME.EXE");
    sint32 angle;
    if (((r_u16(0x8008d490) + 16383) & 65535) >= 32767)
        return 0;
    angle = r_s16(object + 16);
    if (angle)
        FUN_8001033C(angle);
    return ff_static_model_80010B18(r_u32(0x80081720 + 4u * r_u32(object)), 0, 0);
}

GDB_CALL uint32 FUN_8002AB40(uint32 object)
{
    FUNCTION_MARKER(0x8002ab40, "GAME.EXE");
    uint32 phase, tick;
    if ((sint32)r_u32(0x80093dd0) < 2)
        w_u16(object + 26, (uint16)ff_rand_8006D348());
    phase = (uint32)(sint32)r_s16(object + 26);
    tick = r_u32(0x80093dd0);
    phase = ((phase + tick) << 5) & 0xfe0;
    FUN_800100C4(0, 0, 8 * (sint32)r_s16(0x800802d0 + phase));
    return ff_static_model_80010B18(r_u32(0x80081720 + 4u * r_u32(object)), 0, 0);
}

GDB_CALL sint32 FUN_80011E5C(void)
{
    FUNCTION_MARKER(0x80011e5c, "GAME.EXE");
    uint32 matrix = r_u32(0x8008d494);
    uint32 words[8];
    sint32 x, z;
    int i;
    for (i = 0; i < 8; i++)
        words[i] = r_u32(matrix + 4 * i);
    for (i = 0; i < 5; i++)
        xport_gte_write_control(i, words[i]);
    for (i = 5; i < 8; i++)
        xport_gte_write_control(i, (uint32)((sint32)words[i] >> 16));
    xport_gte_write_data(0, 0);
    xport_gte_write_data(1, 0);
    xport_gte_execute(0x480012);
    x = (sint32)xport_gte_read_data(25);
    xport_gte_read_data(26);
    z = (sint32)xport_gte_read_data(27);
    /* Zero-vector MACs equal signed 16-bit translations, so ADD/SUB cannot overflow */
    if (z - 80 < 0)
        return 0;
    if (x <= 0)
        x = -x;
    x += (uint32)z < 800 ? z >> 2 : z >> 3;
    return z - x >= 0;
}

GDB_CALL uint32 FUN_8005AB84(uint32 object)
{
    FUNCTION_MARKER(0x8005ab84, "GAME.EXE");
    sint16 angle;
    sint32 object_x, object_z, player_x, player_z;
    if (!FUN_80011E5C())
        return 0;
    object_x = r_s16(object + 6);
    object_z = r_s16(object + 14);
    player_x = r_s16(0x800b4342);
    player_z = r_s16(0x800b434a);
    angle = (sint16)FUN_8001614C(object_x, object_z, player_x, player_z);
    w_u16(object + 16, (uint16)angle);
    FUN_8001033C(angle);
    return ff_static_model_80010B18(r_u32(0x80081720 + 4u * r_u32(object)), 0, 0);
}

GDB_CALL uint32 FUN_8002A97C(uint32 object)
{
    FUNCTION_MARKER(0x8002a97c, "GAME.EXE");
    FUN_80010040();
    FUN_800100C4(0, 0x800000, 0);
    FUN_8001033C(r_s16(object + 16));
    FUN_800104A0(-r_s16(object + 20));
    FUN_800101CC(r_s16(object + 18));
    ff_static_model_80010B18(r_u32(0x800818f8), 0, 0);
    FUN_800100C4(0, 0x400000, 0);
    FUN_8001033C((sint32)((uint32)r_u16(0x80093dd0) << 12));
    ff_static_model_80010B18(r_u32(0x80081ff0), 0, 0);
    FUN_80010028();
    FUN_80010040();
    FUN_800100C4(0, (sint32)r_u32(object + 8), 0);
    FUN_8001033C(r_s16(object + 16));
    ff_static_model_80010B18(r_u32(0x80082428), 0, 0);
    return FUN_80010028();
}

/* 5AE3C: model transforms and six/two flashing sprites, actual MIPS order. */
GDB_CALL uint32 ff_car_render_8005AE3C(uint32 object)
{
    static const sint32 lights[6][3] = {{80, 36, -376}, {-80, 36, -376}, {128, 36, 384}, {-128, 36, 384}, {140, 68, -176}, {-140, 68, -176}};
    uint32 original_kind = r_u32(object), phase, shade, color, result;
    sint32 position[4], angle;
    int i;
    angle = r_s16(object + 16);
    if (angle)
        FUN_8001033C(angle);
    ff_static_model_80010B18(r_u32(0x800818c8), 0, 8192);
    FUN_800100C4(0, 2621440, 0);
    angle = r_s16(object + 20);
    if (angle)
        FUN_800104A0(angle);
    angle = r_s16(object + 18);
    if (angle)
        FUN_800101CC(angle);
    ff_static_model_80010B18(r_u32(0x80081720 + 4u * r_u32(object)), 0, 0);
    if (!r_s16(object + 30))
        return 0;
    phase = r_u32(0x80093dd0) & 15u;
    shade = phase < 4 ? 128u : phase < 12 ? 128u - 16u * (phase - 4u) : 0;
    color = 0x2c000000u + (shade << 16) + (shade << 8) + shade;
    result = 0;
    for (i = original_kind == 94 ? 0 : 4; i < 6; i++)
    {
        result = (uint32)ff_project_local(lights[i][0], lights[i][1], lights[i][2], position);
        if (!result)
        {
            position[2] = (sint32)((uint32)position[2] - (i < 4 ? 32u : 48u));
            position[3] = 640;
            w_u32(0x8008d4bc, color);
            result = FUN_80012594(position, r_s16(0x8009ad4c));
        }
    }
    return result;
}

/* 800451A0: compound scene model, spinning component and ground shadow. */
GDB_CALL uint32 FUN_800451A0(uint32 object)
{
    FUNCTION_MARKER(0x800451a0, "GAME.EXE");
    sint32 angle;
    if ((sint32)r_u32(object + 8) <= -536870912)
        return 0;
    FUN_80010040();
    FUN_8001033C(r_s16(object + 16));
    angle = r_s16(object + 20);
    if (angle)
        FUN_800104A0(angle);
    FUN_800101CC(r_s16(object + 18) + 4096);
    ff_static_model_80010B18(r_u32(0x80082424), 0, 0);
    FUN_8001033C((sint32)((uint32)r_u16(0x80093dd0) << 12));
    ff_static_model_80010B18(r_u32(0x80081ff0), 0, 0);
    FUN_80010028();
    FUN_80010040();
    FUN_800100C4(0, (sint32)r_u32(object + 8), 0);
    FUN_8001033C(r_s16(object + 16));
    FUN_80067B8C(r_u32(0x80082428), 0, 0);
    return FUN_80010028();
}

/* 800452AC: static object with three phase-offset effect attachments. */
GDB_CALL uint32 FUN_800452AC(uint32 object)
{
    FUNCTION_MARKER(0x800452ac, "GAME.EXE");
    uint32 phase, i, p;
    sint32 angle;
    FUN_80010040();
    FUN_8001033C(r_s16(object + 16));
    angle = r_s16(object + 20);
    if (angle)
        FUN_800104A0(angle);
    if (r_s16(object + 18))
        FUN_800101CC(4096);
    ff_static_model_80010B18(r_u32(0x80081720 + 4 * r_u32(object)), 0, 0);
    phase = r_u32(0x80093dd0) & 31;
    if (r_u32(object) == 836)
        phase = (phase + 19) & 31;
    FUN_800100C4(-4194304, 16252928, 0);
    for (i = 0; i < 3; i++)
    {
        if (i)
            FUN_800100C4(4194304, 0, 0);
        p = (phase + 8 * i) & 31;
        if (p < 6)
            ff_static_model_80010B18(r_u32(p < 2 ? 0x80082440 : p < 4 ? 0x8008243c : 0x80082438), 0, -256);
    }
    FUN_80010028();
    FUN_80010040();
    FUN_800100C4(0, (sint32)r_u32(object + 8), 0);
    FUN_8001033C(r_s16(object + 16));
    FUN_80067B8C(r_u32(0x80082434), 0, 0);
    return FUN_80010028();
}

GDB_CALL uint32 FUN_80059394(void)
{
    uint32 shade = 64u - 16u * (r_u32(0x80093dd0) & 1u);
    w_u32(0x8008d4bc, 0x2c000000u + (shade << 16) + (shade << 8) + shade);
    return ff_static_model_80010B18(r_u32(0x8008257c), 0, 4096);
}

/* 5AC08 compound roadside model, depth shade and alternating shadow. */
GDB_CALL uint32 FUN_8005AC08(uint32 object)
{
    FUNCTION_MARKER(0x8005ac08, "GAME.EXE");
    uint32 type = r_u32(object), position[3];
    sint32 depth, shade;
    if (!FUN_80011E5C())
        return 0;
    FUN_800614F4(0, 0, 0, position);
    FUN_8001033C(r_s16(object + 16));
    depth = (sint16)(position[2] >> 16);
    if (depth > 8192)
    {
        shade = (depth - 8192) >> 7;
        if (shade > 96)
            shade = 96;
        shade = 128 - shade;
        w_u32(0x8008d4bc, 0x2c000000u + ((uint32)shade << 16) + ((uint32)shade << 8) + (uint32)shade);
    }
    ff_static_model_80010B18(r_u32(0x80081720 + 4u * type), 0, 0);
    ff_static_model_80010B18(r_u32(type == 80 ? 0x80081884 : 0x800818c8), 0, 8192);
    FUN_8001033C(-16384);
    FUN_800100C4(-25165824, 0, 0);
    if (type == 78)
        FUN_800100C4(-8388608, 0, 0);
    if (type == 80)
        FUN_800100C4(-16777216, 0, 0);
    return FUN_80059394();
}

/* 8005C17C: object render dispatch. Unported targets are explicit WIP failures. */
GDB_CALL sint32 FUN_8005C17C(void)
{
    sint32 i, count;
    uint32 object = 0x800b89f0, type, target, x, y, z;
    FUN_80011C60();
    count = (sint32)r_u32(0x8009403c);
    if (count <= 0)
        return count;
    i = 0;
    do
    {
        type = r_u32(object);
        if (type != 55)
        {
            if (!r_u32(0x80081720 + 4 * type))
                ff_wip_stop(__FUNCTION__, __FILE__, __LINE__); /* Original fatal loop */
            FUN_80010040();
            x = r_u32(object + 4);
            x -= r_u32(0x8008d4a4);
            y = r_u32(object + 8);
            y -= r_u32(0x8008d4a8);
            z = r_u32(object + 12);
            z -= r_u32(0x8008d4ac);
            FUN_800100C4((sint32)x, (sint32)y, (sint32)z);
            FUN_800101CC(32768);
            target = r_u32(0x80097408 + 4 * type);
            if (target == 0x8002a0c4)
                FUN_8002A0C4(object);
            else if (target == 0x80029f48)
                FUN_80029F48();
            else if (target == 0x8003262c)
                FUN_8003262C(object);
            else if (target == 0x8002a048)
                FUN_8002A048(object);
            else if (target == 0x8002ab40)
                FUN_8002AB40(object);
            else if (target == 0x8005ab84)
                FUN_8005AB84(object);
            else if (target == 0x8005ac08)
                FUN_8005AC08(object);
            else if (target == 0x8002a97c)
                FUN_8002A97C(object);
            else if (target == 0x8005b178)
                FUN_8005B178(object);
            else if (target == 0x8005b350)
                FUN_8005B350(object);
            else if (target == 0x8005ae3c)
                ff_car_render_8005AE3C(object);
            else if (target == 0x800451a0)
                FUN_800451A0(object);
            else if (target == 0x800452ac)
                FUN_800452AC(object);
            else
                ff_object_call(target, object);
            FUN_80010028();
        }
        i++;
        count = (sint32)r_u32(0x8009403c);
        object += 36;
    } while (i < count);
    return 0;
}

/* 2B2E4 item motion, impact and lifetime. Incoming a1 is overwritten by
 * LH object+24 in actual MIPS; IDA's single-argument signature is valid here. */
GDB_CALL sint32 FUN_8002B2E4(uint32 object)
{
    uint32 type = r_u32(object);
    sint32 speed = r_s16(object + 24), life;
    if (speed > 0)
    {
        uint32 angle = r_u16(object + 16) >> 5, x, z, height, point[4];
        sint32 sine = (sint16)r_u16(0x800802d0 + 2 * angle);
        sint32 cosine = (sint16)r_u16(0x800806d0 + 2 * angle);
        sint32 dx = (sint32)((uint32)speed * (uint32)sine) >> 1;
        sint32 dz = (sint32)((uint32)speed * (uint32)cosine) >> 1;
        uint32 reach_x = (uint32)sine << 7, reach_z = (uint32)cosine << 7;
        sint32 value, spin;
        int step;
        x = r_u32(object + 4);
        z = r_u32(object + 12);
        if ((r_u16(object + 34) & 1) && !FUN_80019718(x, z))
            w_u16(object + 34, (uint16)(r_u16(object + 34) - 1));
        for (step = 0; step < 2; step++)
        {
            if (!(r_u16(object + 34) & 1))
            {
                if (r_s16(object + 24) >= 13 && type - 586u >= 12u)
                {
                    point[0] = x + reach_x;
                    point[1] = r_u32(object + 8);
                    point[2] = z - reach_z;
                    point[3] = 1;
                    {
                        sint32 owner = r_s16(object + 28);
                        uint32 kind = r_u32(object);
                        sint32 hit = FUN_8001BFA0(owner, point, kind);
                        if (hit)
                        {
                            if (type == 526)
                                FUN_80056670(30, 255, 6144);
                            if (type == 478 || type == 480 || type == 482)
                            {
                                int particle;
                                w_u32(object, type + 1);
                                FUN_80056670(113, 255, 6400);
                                for (particle = 0; particle < 12; particle++)
                                {
                                    uint32 particle_angle = r_u16(object + 16) >> 5;
                                    uint32 particle_x = r_u32(object + 4);
                                    uint32 particle_y = r_u32(object + 8);
                                    uint32 particle_z = r_u32(object + 12);
                                    FUN_800315A4(599 + (type - 478) / 2, particle_x, particle_y, particle_z, particle_angle);
                                }
                            }
                            if (type - 499u >= 5u && type != 504 && type != 506 && type != 515 && type - 496u >= 2u && type != 505)
                            {
                                value = (sint16)(r_u16(object + 22) - 1);
                                w_u16(object + 22, (uint16)value);
                                if (value <= 0)
                                    w_u16(object + 22, 65535);
                            }
                            if (hit == 2)
                            {
                                dx = dz = 0;
                                w_u16(object + 24, 1);
                            }
                            else
                                w_u16(object + 24, (uint16)((sint16)r_u16(object + 24) >> 1));
                        }
                    }
                }
                if (!(r_u16(object + 34) & 1))
                {
                    if (FUN_80019718(x + reach_x, z - reach_z) & 1)
                    {
                        dx = dz = 0;
                        w_u16(object + 24, 1);
                    }
                    if (!(r_u16(object + 34) & 1) && (FUN_80019718(x + 2u * (uint32)dx, z - 2u * (uint32)dz) & 1))
                    {
                        dx = dz = 0;
                        w_u16(object + 24, 1);
                    }
                }
            }
            x += (uint32)dx;
            z -= (uint32)dz;
        }
        w_u32(object + 4, x);
        w_u32(object + 12, z);
        value = r_u16(object + 32) - 16;
        w_u16(object + 32, (uint16)value);
        height = r_u32(object + 8);
        w_u32(object + 8, height - (uint32)((sint32)((uint32)(uint16)value << 16) >> 1));
        spin = (sint16)r_u16(object + 26) >> 1;
        value = r_u16(object + 18);
        w_u16(object + 18, (uint16)(value + spin));
        height = r_u32(object + 8);
        if ((sint32)height > 393216)
        {
            /* 2B714..2B8CC: material sound and diminishing volume on each bounce. */
            sint32 bounce = r_s16(object + 30);
            sint32 volume = 255 >> ((uint32)bounce & 31), sample = 29, pitch = 6144;
            int stop_spin = 0;
            w_u16(object + 30, (uint16)(bounce + 1));
            if (type == 526)
            {
                sample = 31;
                stop_spin = 1;
            }
            else if (type == 525 || type == 506 || type == 515 || type == 540 || type == 532 || type == 524)
            {
                sample = 219;
                pitch = 4096;
            }
            else if (type - 543u < 2u || type - 553u < 2u || type - 555u < 2u || type == 535 || type - 522u < 2u)
                sample = 221;
            else if (type == 484 || type == 492 || type == 495)
                sample = 218;
            else if (type - 499u < 5u || type == 505)
            {
                sample = 220;
                pitch = 7168;
            }
            else if (type == 497)
                sample = 52;
            else if (type == 504)
                sample = 220;
            else if (type == 477 || type == 485 || type == 487 || type == 489 || type - 493u < 2u || type == 491 || type == 511 || type == 520)
                sample = 219;
            else if (type - 473u < 3u || type - 478u < 6u)
            {
                sample = 222;
                stop_spin = 1;
            }
            else if ((sint32)type >= 522)
            {
                sample = 31;
                stop_spin = 1;
            }
            if (!stop_spin || r_s16(object + 26))
                FUN_80056670(sample, volume, pitch);
            if (stop_spin)
            {
                w_u16(object + 26, 0);
                w_u16(object + 32, 0);
            }
        }
        if ((sint32)height > 0)
        {
            w_u16(object + 34, r_u16(object + 34) & 65534);
            w_u16(object + 18, 0);
            w_u32(object + 8, 0);
            w_u16(object + 32, (uint16)(-r_s16(object + 32) >> 1));
        }
        value = (sint16)(r_u16(object + 24) - 3);
        w_u16(object + 24, (uint16)value);
        if (value < 0)
            w_u16(object + 24, 0);
        height = r_u32(object + 8);
        if ((height == 1 || (sint32)height < 0) && !r_s16(object + 24))
            w_u16(object + 24, 3);
    }
    else if (r_u32(object + 8) != 1)
    {
        w_u16(object + 30, 0);
        w_u16(object + 18, 0);
        w_u32(object + 8, 0);
        if (!r_s16(object + 22) && (type == 506 || type == 515 || type - 499u < 5u || type - 504u < 2u))
            w_u16(object + 22, 65535);
    }
    type = r_u32(object);
    if (type - 587 < 11)
    {
        life = r_s16(object + 22);
        if (life >= 0)
        {
            w_u16(object + 22, (uint16)(life - 1));
            life = r_s16(object + 22);
        }
    }
    else
        life = r_s16(object + 22);
    if (life >= 0)
        return life;
    life = (sint16)(r_u16(object + 22) - 1);
    w_u16(object + 22, (uint16)life);
    if (life < -60)
        w_u32(object, 0xffffffffu);
    return -1;
}

GDB_CALL sint32 FUN_8002BA48(uint32 object)
{
    return FUN_8002B2E4(object);
}

/* 2BA68 supplies a1=0 instead of1024;2B2E4 overwrites it before any use. */
GDB_CALL sint32 FUN_8002BA68(uint32 object)
{
    return FUN_8002B2E4(object);
}

/* 121A8/12240/122E0: shared RGB555 and depth-table OT insertion.
 * Actual MIPS retains complete pointer/tag words, including alias bits. */
static uint32 ff_particle_packet(sint32 x, sint32 y, sint32 z, uint32 color, uint32 shape)
{
    uint32 p = r_u32(0x8008d4b0), xy = ((uint32)x & 65535) | ((uint32)y << 16);
    uint32 ot, depth_table, depth_address, bucket;
    sint32 offset;
    w_u32(p + 4, (shape == 2 ? 0x28000000u : 0x40000000u) | ((color & 31) << 3) | ((color & 0x3e0) << 6) | ((color & 0x7c00) << 9));
    w_u32(p + 8, xy);
    if (shape == 2)
    {
        xy = ff_matrix_pointer_add(xy, 2);
        w_u32(p + 12, xy);
        xy = ff_matrix_pointer_add(xy, 0x20000);
        w_u32(p + 20, xy);
        xy = ff_matrix_pointer_add(xy, -2);
        w_u32(p + 16, xy);
    }
    else
    {
        if (shape == 1)
            xy = ff_matrix_pointer_add(xy, 0x10000);
        w_u32(p + 12, xy);
    }
    ot = r_u32(0x8008d4b4);
    depth_table = r_u32(0x8008d4d8);
    depth_address = ff_matrix_pointer_add(depth_table, (sint32)((uint32)(z >> 2) << 1));
    offset = r_s16(depth_address);
    bucket = ff_matrix_pointer_add(ot, offset);
    w_u32(p, r_u32(bucket) | (shape == 2 ? 0x05000000u : 0x03000000u));
    w_u32(bucket, p);
    w_u32(0x8008d4b0, ff_matrix_pointer_add(p, shape == 2 ? 24 : 16));
    return 0x8008d4b0;
}

GDB_CALL uint32 FUN_800121A8(sint32 x, sint32 y, sint32 z, uint32 color)
{
    FUNCTION_MARKER(0x800121a8, "GAME.EXE");
    return ff_particle_packet(x, y, z, color, 0);
}

GDB_CALL uint32 FUN_80012240(sint32 x, sint32 y, sint32 z, uint32 color)
{
    FUNCTION_MARKER(0x80012240, "GAME.EXE");
    return ff_particle_packet(x, y, z, color, 1);
}

GDB_CALL uint32 FUN_800122E0(sint32 x, sint32 y, sint32 z, uint32 color)
{
    FUNCTION_MARKER(0x800122e0, "GAME.EXE");
    return ff_particle_packet(x, y, z, color, 2);
}

/* Project a particle using the kind and language palette */
GDB_CALL uint32 FUN_8005C2F0(uint32 object)
{
    uint32 table, color, x, y, z, camera_x, camera_y, camera_z;
    sint32 age, position[4];
    FUN_80010040();
    x = r_u32(object + 4);
    camera_x = r_u32(0x8008d4a4);
    y = r_u32(object + 8);
    camera_y = r_u32(0x8008d4a8);
    z = r_u32(object + 12);
    camera_z = r_u32(0x8008d4ac);
    FUN_800100C4((sint32)(x - camera_x), (sint32)(y - camera_y), (sint32)(z - camera_z));
    ff_project_local(0, 0, 0, position);
    if (!position[3])
    {
        table = r_u32(object) == 1 ? 0x80080050 : r_u32(0x80093d70) == 2 ? 0x80080070 : 0x80080030;
        age = (sint16)r_u16(object + 26);
        color = (uint32)(sint32)r_s16(table + 2u * (uint32)(age >> 2));
        age = r_s16(object + 26);
        if (age >= 17)
            FUN_800121A8(position[0], position[1], position[2], color);
        else if (r_s16(object + 26) >= 9)
            FUN_80012240(position[0], position[1], position[2], color);
        else
            FUN_800122E0(position[0], position[1], position[2], color);
    }
    return FUN_80010028();
}

/* Fixed palette80080090, signed age/4 and size thresholds9/17 */
GDB_CALL uint32 FUN_8005C480(uint32 object)
{
    FUNCTION_MARKER(0x8005c480u, "GAME.EXE");
    uint32 color, x, y, z, camera_x, camera_y, camera_z;
    sint32 age, position[4];
    FUN_80010040();
    x = r_u32(object + 4);
    camera_x = r_u32(0x8008d4a4);
    y = r_u32(object + 8);
    camera_y = r_u32(0x8008d4a8);
    z = r_u32(object + 12);
    camera_z = r_u32(0x8008d4ac);
    FUN_800100C4((sint32)(x - camera_x), (sint32)(y - camera_y), (sint32)(z - camera_z));
    ff_project_local(0, 0, 0, position);
    if (!position[3])
    {
        age = (sint16)r_u16(object + 26);
        color = (uint32)(sint32)r_s16(0x80080090 + 2 * (uint32)(age >> 2));
        if (age >= 17)
            FUN_800121A8(position[0], position[1], position[2], color);
        else if (r_s16(object + 26) >= 9)
            FUN_80012240(position[0], position[1], position[2], color);
        else
            FUN_800122E0(position[0], position[1], position[2], color);
    }
    return FUN_80010028();
}

/* 2BEC0: signed table products, gravity, ground removal and saturating age.
 * The return still increments when age >=63 and its store is skipped. */
GDB_CALL sint32 FUN_8002BEC0(uint32 object)
{
    sint32 speed = r_s16(object + 24), angle = r_s16(object + 16), vertical, age;
    uint32 y;
    w_u32(object + 4, r_u32(object + 4) + (uint32)((speed * r_s16(0x800802d0 + 2u * (uint32)angle)) >> 1));
    w_u32(object + 12, r_u32(object + 12) - (uint32)((speed * r_s16(0x800806d0 + 2u * (uint32)angle)) >> 1));
    vertical = r_s16(object + 32);
    if (vertical >= -255)
        w_u16(object + 32, (uint16)(vertical - 2));
    y = r_u32(object + 8) - ((uint32)(sint32)r_s16(object + 32) << 15);
    w_u32(object + 8, y);
    if ((sint32)y > 0)
        w_u32(object, 0xffffffff);
    age = r_s16(object + 26);
    if (age < 63)
        w_u16(object + 26, (uint16)(age + 1));
    return age + 1;
}

/* 2B28C: remove the impact sprite at age6, then increment its unsigned age. */
GDB_CALL uint32 FUN_8002B28C(uint32 object)
{
    uint32 result;
    if (r_s16(object + 22) == 6)
        w_u32(object, 0xffffffff);
    result = (uint32)(uint16)r_s16(object + 22) + 1;
    w_u16(object + 22, (uint16)result);
    return result;
}

/* Native target mapping; additional game callbacks remain explicit WIP. */
/* 2AF78: four ordered environment probes; expiry does not skip this update. */
GDB_CALL sint32 FUN_8002AF78(uint32 object)
{
    FUNCTION_MARKER(0x8002af78u, "GAME.EXE");
    uint32 actor = 0x800b4318 + 244u * (uint32)(sint32)r_s16(object + 28), point[4], angle, dx, dz;
    sint16 age = (sint16)((uint16)r_s16(object + 22) + 1u);
    sint32 result;
    int i;
    w_u16(object + 22, (uint16)age);
    if (age >= 16)
        w_u32(object, 0xffffffffu);
    angle = ((uint32)(uint16)r_s16(object + 22) << 8) & 0xf00u;
    dx = ((uint32)(sint32)r_s16(0x800802d0 + angle) * (uint32)(sint32)r_s16(object + 22)) << 7;
    dz = ((uint32)(sint32)r_s16(0x800806d0 + angle) * (uint32)(sint32)r_s16(object + 22)) << 7;
    point[1] = 0;
    point[3] = 0;
    for (i = 0; i < 4; i++)
    {
        point[0] = (i & 1) ? r_u32(actor + 40) - dx : r_u32(actor + 40) + dx;
        point[2] = (i == 0 || i == 3) ? r_u32(actor + 48) - dz : r_u32(actor + 48) + dz;
        result = FUN_8001BBE8(point, r_s16(object + 28));
        if (result)
            return result;
    }
    return result;
}

/* 2B120: signed halfword lifetime; v0 is -1 on both branches. */
GDB_CALL uint32 FUN_8002B120(uint32 object)
{
    sint16 age = (sint16)((uint16)r_s16(object + 22) + 1u);
    w_u16(object + 22, (uint16)age);
    if (age >= 8)
        w_u32(object, 0xffffffffu);
    return 0xffffffffu;
}

/* 5D3D8: signed age scaling and unclamped wrapping grayscale, MIPS exact. */
GDB_CALL uint32 FUN_8005D3D8(uint32 object)
{
    FUNCTION_MARKER(0x8005d3d8u, "GAME.EXE");
    uint32 shade, kind;
    sint32 scale;
    FUN_80010040();
    FUN_8005C740(object);
    scale = (sint32)((uint32)(sint32)r_s16(object + 22) << 12);
    FUN_80010F94(scale, scale, scale);
    shade = 64u - ((uint32)(sint32)r_s16(object + 22) << 2);
    w_u32(0x8008d4bc, 0x2c000000u + (shade << 16) + (shade << 8) + shade);
    kind = r_u32(object);
    ff_static_model_80010B18(r_u32(0x80081720 + 4u * kind), 0, kind == 604 ? 0 : 8192);
    return FUN_80010028();
}

/* 5D488: expanding ground effect; scale and grayscale follow actual MIPS. */
GDB_CALL uint32 FUN_8005D488(uint32 object)
{
    uint32 shade;
    sint32 scale;
    FUN_80010040();
    FUN_8005C740(object);
    scale = (sint32)((uint32)(sint32)r_s16(object + 22) << 13);
    if (r_u32(object) == 607)
        scale >>= 1;
    FUN_80010F94(scale, scale, scale);
    shade = 64u - ((uint32)(sint32)r_s16(object + 22) << 3);
    w_u32(0x8008d4bc, 0x2c000000u + (shade << 16) + (shade << 8) + shade);
    ff_static_model_80010B18(r_u32(0x80081720 + 4u * r_u32(object)), 0, 0);
    return FUN_80010028();
}

/* 2AE64: dust age28 expiry; signed acceleration from wrapped halfword.
 * Indexed twin2AE10 has a different lifetime threshold; keep it distinct. */
GDB_CALL uint32 FUN_8002AE64(uint32 object)
{
    uint16 age = (uint16)((uint16)r_s16(object + 22) + 1u);
    uint16 speed = (uint16)((uint16)r_s16(object + 26) + 1u);
    uint32 delta = (uint32)((sint32)((uint32)speed << 16) >> 1);
    w_u16(object + 22, age);
    if ((sint16)age >= 28)
        w_u32(object, 0xffffffffu);
    w_u16(object + 26, speed);
    w_u32(object + 8, r_u32(object + 8) - delta);
    return delta;
}

/* 2AE10: indexed twin2AE64; only signed expiry threshold differs (14/28). */
GDB_CALL uint32 FUN_8002AE10(uint32 object)
{
    FUNCTION_MARKER(0x8002ae10u, "GAME.EXE");
    uint16 age = (uint16)((uint16)r_s16(object + 22) + 1u), speed;
    uint32 delta;
    w_u16(object + 22, age);
    if ((sint16)age >= 14)
        w_u32(object, 0xffffffffu);
    speed = (uint16)((uint16)r_s16(object + 26) + 1u);
    delta = (uint32)((sint32)((uint32)speed << 16) >> 1);
    w_u16(object + 26, speed);
    w_u32(object + 8, r_u32(object + 8) - delta);
    return delta;
}

/* 11FEC: original JR RA/NOP; no result or side effects. */
GDB_CALL void FUN_80011FEC(void)
{
    FUNCTION_MARKER(0x80011fecu, "GAME.EXE");
}

GDB_CALL uint32 ff_object_call(uint32 target, uint32 object)
{
    if (target == 0x800420f8)
        return FUN_800420F8(object, 0);

    if (target == 0x8002d7c8)
        return FUN_8002D7C8(object);
    if (target == 0x8002dba0)
        return FUN_8002DBA0(object);
    if (target == 0x8002da3c)
        return FUN_8002DA3C(object);
    if (target == 0x8002cde0)
        return FUN_8002CDE0(object);
    if (target == 0x8002d5d4)
        return FUN_8002D5D4(object);
    if (target == 0x8002f59c)
        return FUN_8002F59C(object);
    if (target == 0x80043978)
        return FUN_80043978(object);
    if (target == 0x8002a168)
        return FUN_8002A168(object);
    if (target == 0x8003fdcc)
        return FUN_8003FDCC(object);
    if (target == 0x8003f9c0)
        return FUN_8003F9C0(object);
    if (target == 0x80044844)
        return FUN_80044844(object);
    if (target == 0x80042ba0)
        return FUN_80042BA0(object);
    if (target == 0x80044334)
        return FUN_80044334(object);
    switch (target)
    {
        case 0x8002c060:
            return FUN_8002C060(object);
        case 0x8002a858:
            return FUN_8002A858();
        case 0x8002a8fc:
            return FUN_8002A8FC(object);
        case 0x8002ace8:
            return FUN_8002ACE8(object);
        case 0x8002ad7c:
            return FUN_8002AD7C(object);
        case 0x8002b150:
            return FUN_8002B150(object);
        case 0x80032680:
            return FUN_80032680(object);
        case 0x800326dc:
            return FUN_800326DC(object);
        case 0x80032728:
            return FUN_80032728(object);
        case 0x800327b0:
            return FUN_800327B0();
        case 0x80032aa4:
            return FUN_80032AA4(object);
        case 0x80032b20:
            return FUN_80032B20(object);
        case 0x8002a00c:
            return FUN_8002A00C(object);
        case 0x8002a7dc:
            return FUN_8002A7DC(object);
        case 0x8002a89c:
            return FUN_8002A89C();
        case 0x8002aa6c:
            return FUN_8002AA6C(object);
        case 0x8002abe0:
            return FUN_8002ABE0(object);
        case 0x8002c294:
            return FUN_8002C294(object);
        case 0x80032774:
            return FUN_80032774(object);
        case 0x80032b98:
            return FUN_80032B98(object);
        case 0x8003ee7c:
            return FUN_8003EE7C(object);
        case 0x8005bc64:
            return FUN_8005BC64(object);
        case 0x8005bf0c:
            return FUN_8005BF0C(object);
        case 0x8005bf7c:
            return FUN_8005BF7C(object);
        case 0x80032818:
            return FUN_80032818(object);
        case 0x80032a48:
            return FUN_80032A48(object);
        case 0x8005c598:
            return FUN_8005C598(object);
        case 0x8005d53c:
            return FUN_8005D53C(object);
        case 0x8002e37c:
            return (uint32)FUN_8002E37C(object);
        case 0x80032d70:
            return FUN_80032D70(object);
        case 0x80032e20:
            return (uint32)FUN_80032E20(object);
        case 0x8002bfa8:
            return FUN_8002BFA8(object);
        case 0x8005b4f8:
            return FUN_8005B4F8(object);
        case 0x8005b350:
            return FUN_8005B350(object);
        case 0x80044cd4:
            return FUN_80044CD4(object);
        case 0x8005cd1c:
            return FUN_8005CD1C(object);
        case 0x8005bbac:
            return FUN_8005BBAC(object);
        case 0x8002ed7c:
            return FUN_8002ED7C(object);
        case 0x8005b71c:
            return FUN_8005B71C(object);
        case 0x8005c0e8:
            return FUN_8005C0E8(object);
        case 0x800428ec:
            return FUN_800428EC(object);
        case 0x80042848:
            return FUN_80042848(object);
        case 0x80032d44:
            return FUN_80032D44();
        case 0x800450f8:
            return FUN_800450F8(object);
        case 0x8004277c:
            return FUN_8004277C(object);
        case 0x8005b684:
            return FUN_8005B684(object);
        case 0x8005b7ac:
            return FUN_8005B7AC(object);
        case 0x8005d160:
            return FUN_8005D160(object);
        case 0x8005d608:
            return FUN_8005D608(object);
        case 0x8005d7a4:
            return FUN_8005D7A4(object);
        case 0x8005bd7c:
            return FUN_8005BD7C(object);
        case 0x8005ab18:
            return FUN_8005AB18(object);
        case 0x80041d60:
            return FUN_80041D60(object);
        case 0x80043680:
            return FUN_80043680(object);
        case 0x8003f088:
            return FUN_8003F088(object);
        case 0x800403dc:
            return FUN_800403DC(object);
        case 0x8002efdc:
            return FUN_8002EFDC(object);
        case 0x8002fa90:
            return FUN_8002FA90(object);
        case 0x8002f2e0:
            return FUN_8002F2E0(object);
        case 0x8002bc1c:
            return FUN_8002BC1C(object);
        case 0x800469a4:
            return FUN_800469A4(object);
        case 0x8002e11c:
            return FUN_8002E11C(object);
        case 0x8004322c:
            return FUN_8004322C(object);
        case 0x80046c08:
            return FUN_80046C08(object);
        case 0x80046ef0:
            return FUN_80046EF0(object);
        case 0x8002eb6c:
            return FUN_8002EB6C(object);
        case 0x8005b844:
            return FUN_8005B844(object);
        case 0x8002a640:
            return FUN_8002A640(object);
        case 0x80042f80:
            return FUN_80042F80(object);
        case 0x800429f0:
            return FUN_800429F0(object);
        case 0x8004298c:
            return (uint32)FUN_8004298C();
        case 0x8002ff30:
            return FUN_8002FF30(object);
        case 0x8002ee60:
            return FUN_8002EE60(object);
        case 0x8005ba28:
            return FUN_8005BA28(object);
        case 0x8002c2e4:
            return FUN_8002C2E4(object);
        case 0x800419d4:
            return FUN_800419D4(object);
        case 0x80041b54:
            return FUN_80041B54(object, 0xffffffffu);
        case 0x8002e9fc:
            return FUN_8002E9FC(object);
        case 0x8002d678:
            return FUN_8002D678(object);
        case 0x8002e8a4:
            return FUN_8002E8A4(object);
        case 0x8005cee4:
            return FUN_8005CEE4(object);
        case 0x8002f950:
            return FUN_8002F950(object);
        case 0x8002c55c:
            return FUN_8002C55C(object);
        case 0x8005d024:
            return FUN_8005D024(object);
        case 0x8003f6ec:
            return FUN_8003F6EC(object);
        case 0x8005cdbc:
            return FUN_8005CDBC(object);
        case 0x8004471c:
            return FUN_8004471C(object);
        case 0x8002d8fc:
            return FUN_8002D8FC(object);
        case 0x800300bc:
            return FUN_800300BC(object);
        case 0x800416e0:
            return FUN_800416E0(object);
        case 0x800454e0:
            return FUN_800454E0(object);
        case 0x80043128:
            return FUN_80043128(object);
        case 0x8002c454:
            return FUN_8002C454(object);
        case 0x8005b404:
            return FUN_8005B404(object);
        case 0x80032c4c:
            return FUN_80032C4C(object);
        case 0x8005b598:
            return FUN_8005B598(object);
        case 0x8005dad4:
            return FUN_8005DAD4(object);
        case 0x80041e6c:
            return FUN_80041E6C(object);
        case 0x8005ad54:
            return FUN_8005AD54(object);
        case 0x8005d1fc:
            return FUN_8005D1FC(object);
        case 0x80046e1c:
            FUN_80046E1C(object);
            return 0; /* Callback's return is unused; original leaves v0 undefined on early exit. */
        case 0x80041894:
            return FUN_80041894(object);
        case 0x8002cac4:
            return FUN_8002CAC4(object);
        case 0x80041dd8:
            return FUN_80041DD8(object);
        case 0x8004160c:
            return FUN_8004160C(object);
        case 0x80047108:
            FUN_80047108(object);
            return 0;
        case 0x8005bff4:
            return FUN_8005BFF4(object);
        case 0x8005d2e4:
            return FUN_8005D2E4(object);
        case 0x8005d340:
            return FUN_8005D340(object);
        case 0x8002c154:
            return FUN_8002C154(object);
        case 0x8002c1f4:
            return FUN_8002C1F4(object);
        case 0x8002ccbc:
            return FUN_8002CCBC(object);
        case 0x8002ac8c:
            return FUN_8002AC8C(object);
        case 0x8002b1e4:
            return FUN_8002B1E4(object);
        case 0x8002d218:
            return FUN_8002D218(object);
        case 0x8002e444:
            return FUN_8002E444(object);
        case 0x8002c69c:
            return FUN_8002C69C(object);
        case 0x80044d90:
            return FUN_80044D90(object);
        case 0x8002cef8:
            return (uint32)FUN_8002CEF8(object);
        case 0x8002d2a4:
            return FUN_8002D2A4(object);
        case 0x8002df00:
            return FUN_8002DF00(object);
        case 0x8002dfa4:
            return FUN_8002DFA4(object);
        case 0x8003f388:
            return FUN_8003F388(object);
        case 0x8003f410:
            return FUN_8003F410(object);
        case 0x800417ec:
            return FUN_800417EC(object);
        case 0x8002e7c4:
            return FUN_8002E7C4(object);
        case 0x8002aeb8:
            return FUN_8002AEB8(object);
        case 0x8002aee8:
            return FUN_8002AEE8(object);
        case 0x8002af18:
            return FUN_8002AF18(object);
        case 0x8002af48:
            return FUN_8002AF48(object);
        case 0x8002b2b4:
            return FUN_8002B2B4(object);
        case 0x80041974:
            return FUN_80041974(object);
        case 0x800419a4:
            return FUN_800419A4(object);
        case 0x8002da0c:
            return FUN_8002DA0C(object);
        case 0x8002e7f0:
            return FUN_8002E7F0(object);
        case 0x8002e870:
            return FUN_8002E870(object);
        case 0x8004348c:
            return FUN_8004348C(object);
        case 0x8002cba0:
            return FUN_8002CBA0(object);
        case 0x8002ce6c:
            return FUN_8002CE6C(object);
        case 0x8002e038:
            return FUN_8002E038(object);
        case 0x8002c1dc:
            return FUN_8002C1DC(object);
        case 0x8002c27c:
            return FUN_8002C27C(object);
        case 0x800415f4:
            return FUN_800415F4(object);
        case 0x80041600:
            return FUN_80041600(object);
        case 0x8002eb58:
            return FUN_8002EB58(object);
        case 0x8002a974:
            FUN_8002A974();
            return 0;
        case 0x80041b4c:
            FUN_80041B4C();
            return 0;
        case 0x8004483c:
            FUN_8004483C();
            return 0;
        case 0x800455dc:
            FUN_800455DC();
            return 0;
        case 0x8005c654:
            FUN_8005C654();
            return 0;
        case 0x8005c65c:
            FUN_8005C65C();
            return 0;
        case 0x8005c664:
            FUN_8005C664();
            return 0;
        case 0x8005d150:
            FUN_8005D150();
            return 0;
        case 0x8005d158:
            FUN_8005D158();
            return 0;
        case 0x8005dacc:
            FUN_8005DACC();
            return 0;
        case 0x8005d67c:
            return FUN_8005D67C(object);
        case 0x8002ddb8:
            return FUN_8002DDB8(object);
        case 0x8002af78:
            return (uint32)FUN_8002AF78(object);
        case 0x8002ae10:
            return FUN_8002AE10(object);
        case 0x8002ae64:
            return FUN_8002AE64(object);
        case 0x8002b120:
            return FUN_8002B120(object);
        case 0x8005d3d8:
            return FUN_8005D3D8(object);
        case 0x8005d488:
            return FUN_8005D488(object);
        case 0x8005c480:
            return FUN_8005C480(object);
        case 0x8005c2f0:
            return FUN_8005C2F0(object);
        case 0x8005c808:
            return FUN_8005C808(object);
        case 0x8002bec0:
            return (uint32)FUN_8002BEC0(object);
        case 0x8002f828:
            return FUN_8002F828(object);
        case 0x8002f700:
            return FUN_8002F700(object);
        case 0x8002b28c:
            return FUN_8002B28C(object);
        case 0x8002ba48:
            return (uint32)FUN_8002BA48(object);
        case 0x8002ba68:
            return (uint32)FUN_8002BA68(object);
        case 0x8005da8c:
        case 0x8005daac:
            return FUN_8005D820(object);
        case 0x8002a0c4:
            return FUN_8002A0C4(object);
        case 0x800451a0:
            return FUN_800451A0(object);
        case 0x800452ac:
            return FUN_800452AC(object);
        case 0x80043520:
            return (uint32)FUN_80043520(object);
        case 0x8005c66c:
            return FUN_8005C66C(object);
        default:
            FF_DIAGNOSTIC(fprintf(stderr, "XPORT_CALLBACK target=%08X object=%08X kind=%08X tick=%u\n", target, object, r_u32(object), r_u32(0x80093dd0)));
            ff_wip_stop(__FUNCTION__, __FILE__, __LINE__);
    }
}

/* 8001EDB8: all128 effect slots are visited; absent callbacks are skipped. */
GDB_CALL sint32 FUN_8001EDB8(void)
{
    uint32 i, object, type, target;
    for (i = 0, object = 0x8009d1d0; i < 128; i++, object += 36)
    {
        type = r_u32(object);
        if (type == 0xffffffff)
            continue;
        target = r_u32(0x800be5e0 + 4 * type);
        if (target)
            ff_object_call(target, object);
    }
    return 0;
}

/* 8005C66C: default effect model with height offset and yaw/pitch/roll order. */
GDB_CALL uint32 FUN_8005C66C(uint32 object)
{
    uint32 x, y, z, camera_x, camera_y, camera_z, type, height;
    FUN_80010040();
    x = r_u32(object + 4);
    camera_x = r_u32(0x8008d4a4);
    type = r_u32(object);
    y = r_u32(object + 8);
    height = r_u32(0x800990d8 + 4 * type);
    y += height;
    camera_y = r_u32(0x8008d4a8);
    z = r_u32(object + 12);
    camera_z = r_u32(0x8008d4ac);
    FUN_800100C4((sint32)(x - camera_x), (sint32)(y - camera_y), (sint32)(z - camera_z));
    FUN_800101CC(32768);
    FUN_8001033C(r_s16(object + 16));
    FUN_800101CC(r_s16(object + 18));
    FUN_800104A0(r_s16(object + 20));
    ff_static_model_80010B18(r_u32(0x80081720 + 4 * r_u32(object)), 0, 0);
    return FUN_80010028();
}

/* 8001EF68: render128 effect slots, falling back when table entry is null. */
GDB_CALL sint32 FUN_8001EF68(void)
{
    uint32 i, object, type, target;
    for (i = 0, object = 0x8009d1d0; i < 128; i++, object += 36)
    {
        type = r_u32(object);
        if (type == 0xffffffff)
            continue;
        target = r_u32(0x80098270 + 4 * type);
        if (target)
            ff_object_call(target, object);
        else
            FUN_8005C66C(object);
    }
    return 0;
}

/* 8001ED1C:64 dynamic object updates with original callback stack provenance */
GDB_CALL sint32 FUN_8001ED1C(void)
{
    uint32 i, object, type, target, previous_car = 0, retained_angle = 0x800bbf78;
    for (i = 0, object = 0x8009a3c8; i < 64; i++, object += 36)
    {
        type = r_u32(object);
        if (type == 0xffffffff)
            continue;
        target = r_u32(0x800bbf78 + 4 * type);
        if (target == 0x800420f8)
        {
            scene_helicopter_call(object, previous_car, retained_angle);
            previous_car = 0;
        }
        else if (target == 0x8004471c)
        {
            ff_object_call(target, object);
            previous_car = object;
            retained_angle = 0x800bbf78;
        }
        else if (!target || target == 0x8002ba48 || target == 0x8002ba68)
        {
            /* Physics preserves height and saves incoming S0 as angle */
            ff_object_call(target ? target : 0x8002ba48, object);
            retained_angle = object;
        }
        else
        {
            ff_object_call(target ? target : 0x8002ba48, object);
            previous_car = 0;
        }
    }
    return 0;
}

/* 8001EE44: hide dynamic records referenced by any actor attachment. */
GDB_CALL sint32 FUN_8001EE44(void)
{
    uint8 attached[64] = {0};
    uint32 i, j, object, type, target;
    sint32 index;
    for (i = 0; i < 8; i++)
        for (j = 0; j < 2; j++)
        {
            index = r_s8(0x800b4395 + 244 * i + j);
            if (index != -1)
            {
                if ((uint32)index >= 64)
                    ff_wip_stop(__FUNCTION__, __FILE__, __LINE__);
                attached[index] = 1;
            }
        }
    for (i = 0, object = 0x8009a3c8; i < 64; i++, object += 36)
    {
        type = r_u32(object);
        if (type == 0xffffffff || attached[i])
            continue;
        target = r_u32(0x800bff00 + 4 * type);
        ff_object_call(target ? target : 0x8005da8c, object);
    }
    return 0;
}

/* 80064684: original double-buffer OT and packet arena selection. */
GDB_CALL uint32 FUN_80064684(void)
{
    uint32 parity = r_u32(0x8008d4c4) & 1;
    w_u32(0x8008d4b8, parity ? 0xe5140 : 0xe7140);
    parity = r_u32(0x8008d4c4) & 1;
    w_u32(0x8008d4b4, parity ? 0xe7140 : 0xe5140);
    parity = r_u32(0x8008d4c4) & 1;
    w_u32(0x8008d4b0, parity ? 0xff8a0 : 0xe9140);
    return parity;
}

/* 614F4: original RotTrans(MVMVA), MAC outputs shifted to16.16. */
GDB_CALL uint32 FUN_800614F4(sint32 x, sint32 y, sint32 z, uint32 output[3])
{
    SVECTOR v;
    VECTOR q;
    sint32 flags;
    FUN_80010170();
    v.vx = (sint16)(x >> 16);
    v.vy = (sint16)(y >> 16);
    v.vz = (sint16)(z >> 16);
    v.pad = 0;
    RotTrans(&v, &q, &flags);
    output[0] = (uint32)q.vx << 16;
    output[1] = (uint32)q.vy << 16;
    output[2] = (uint32)q.vz << 16;
    return output[2];
}

/* 1B9F4 valid attachment selectors. Original unspecified selectors read
 * uninitialized stack; reject them rather than invent attachment geometry. */
GDB_CALL uint32 FUN_8001B9F4(sint32 index, sint32 attachment, uint32 output[3])
{
    uint32 pose = 0x800ba210 + 504u * (uint32)index, offset, point[3], x, y, z;
    int i;
    FUN_80010040();
    FUN_80011D20();
    FUN_8001033C(-r_s16(0x8008d490));
    FUN_800101CC(-r_s16(0x8008d492));
    if (attachment == 2)
    {
        FUN_800100C4(0, -786432, 0);
        offset = 24;
    }
    else if (attachment == 3 || attachment == 11)
    {
        FUN_800100C4(attachment == 11 ? -2097152 : 0, -786432, 0);
        offset = 56;
    }
    else if (attachment >= 4 && attachment <= 9)
        offset = 132 + 12u * (uint32)(attachment - 4);
    else if (attachment == 1)
        offset = 204;
    else
    {
        ff_wip_stop(__FUNCTION__, __FILE__, __LINE__);
        return 0;
    }
    x = r_u32(pose + offset);
    y = r_u32(pose + offset + 4);
    z = r_u32(pose + offset + 8);
    FUN_800614F4((sint32)x, (sint32)y, (sint32)z, point);
    for (i = 0; i < 3; i++)
        output[i] = r_u32(0x8008d4a4 + 4u * i) + point[i];
    return FUN_80010028();
}
