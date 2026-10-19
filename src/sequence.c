#include "ff.h"
#include <string.h>
#include <stdlib.h>

GDB_CALL uint32 FUN_80043978(uint32 object)
{
    FUNCTION_MARKER(0x80043978u, "GAME.EXE");
    sint32 start = r_s16(object + 26);
    sint32 age = (sint32)(r_u32(0x80093dd0) - (uint32)start), pitch, count, voice, wave;
    uint32 volume, pan, angle, multiplier, point[3], target, end, random, x, y, z, result = 0, region;
    int i;
    if (r_s16(object + 30) > 0)
    {
        ff_spatial_sound_80046884(object + 4, &volume, &pan);
        if (age > 0)
        {
            FUN_80056840(r_s16(object + 30) - 1, ((uint32)(age >> 3) << 3) + 6400);
            if (age < 65)
                volume = (uint32)((sint32)((uint32)age * volume) >> 6);
        }
        volume = (uint32)FUN_80042820((sint32)volume, -80);
        FUN_800568C8(r_s16(object + 30) - 1, (sint32)volume, (sint32)pan);
    }
    if (r_s16(object + 26) == -1)
    {
        if (!r_s8(0x80093dd9) || r_u32(0x80093d60) != 1 || FUN_80047434(0xec000000u))
            goto region_update;
        w_u16(object + 26, r_u16(0x80093dd0));
        w_u16(object + 30, (uint16)(FUN_80056670(132, 0, 6400) + 1));
        w_u32(0x80093d60, r_u32(0x80093d60) + 1);
        /* Age was captured before the start timestamp changed. */
    }
    if (age < 10)
        return 1;
    if (age < 90)
    {
        angle = r_u16(object + 16);
        if (age < 40)
        {
            angle = (angle >> 5) * 2;
            multiplier = 128;
        }
        else
        {
            angle += age < 70 ? 192u : (age < 80 ? 0xfffffa00u : 0xffffff80u);
            w_u16(object + 16, (uint16)angle);
            angle = (angle >> 4) & 4094;
            multiplier = age < 70 ? 160 : (age < 80 ? 192 : 256);
        }
        wave = r_s16(0x800802d0 + angle);
        x = r_u32(object + 4);
        w_u32(object + 4, x - (uint32)wave * multiplier);
        wave = r_s16(0x800806d0 + angle);
        z = r_u32(object + 12);
        w_u32(object + 12, z + (uint32)wave * multiplier);
        if (age == 69)
            w_u16(object + 28, (uint16)(FUN_80056670(131, 255, 5632) + 1));
        if (age >= 70 && (age & 1))
            for (i = 0; i < 2; ++i)
            {
                FUN_80010040();
                FUN_80011D20();
                FUN_800101CC(32768);
                FUN_8001033C(r_s16(object + 16));
                FUN_800614F4(i ? -128 : 128, 0, 256, point);
                x = r_u32(object + 4);
                y = r_u32(object + 8);
                z = r_u32(object + 12);
                FUN_80031B20(871, (point[0] << 16) + x, y + 196608, (point[2] << 16) + z, 0, 0, 0);
                FUN_80010028();
            }
    }
    else if (age < 98)
    {
        if (age == 90)
        {
            voice = r_s16(object + 28);
            if (voice)
            {
                FUN_800567F8(voice - 1);
                w_u16(object + 28, 0);
            }
            voice = r_s16(object + 30);
            if (voice)
            {
                FUN_800567F8(voice - 1);
                w_u16(object + 30, 0);
            }
            target = 0x800b89f0;
            end = target + 36u * r_u32(0x8009403c);
            while (target < end && r_u32(target) != 162)
                target += 36;
            if (target < end)
            {
                FUN_80056670(134, 255, 6144);
                w_u32(0x800941a8, 16);
                w_u32(0x80094190, 4);
                for (i = -4; i <= 4; ++i)
                {
                    random = ff_rand_8006D348() & 0x17f;
                    y = r_u32(target + 8);
                    if (i)
                        x = r_u32(target + 4);
                    z = r_u32(target + 12);
                    /* The original unrolled center particle reads X after Z */
                    if (!i)
                        x = r_u32(target + 4);
                    FUN_80031B20(871, x + ((uint32)i << 20), y - (random << 16), z + 0xfe810000, 0, 0, 0);
                }
                w_u32(target, 163);
            }
        }
        pitch = -((sint32)((uint32)r_u16(0x800802d0 + ((uint32)(age - 90) << 8)) << 16) >> 20);
        w_u16(object + 18, (uint16)pitch);
        w_u32(object + 8, (uint32)(pitch >> 6) << 15);
    }
    else if (age < 107)
    {
        pitch = -((sint32)((uint32)r_u16(0x800802d0 + ((uint32)(age - 98) << 8)) << 16) >> 21);
        w_u16(object + 18, (uint16)pitch);
        w_u32(object + 8, (uint32)(pitch >> 6) << 14);
    }
    else
    {
        for (i = 0; i < 2; ++i)
        {
            x = ((ff_rand_8006D348() & 31) << 16) + r_u32(object + 4);
            random = ff_rand_8006D348();
            z = r_u32(object + 12);
            y = r_u32(object + 8);
            result = FUN_80031B20(871, x, y - 6291456, ((random & 31) << 16) + z, 0, 0, 0);
        }
        return result;
    }
region_update:
    region = 0x8009ec70;
    count = (sint32)r_u32(0x8009479c);
    if (count > 0)
        count = (sint32)r_u32(0x8009479c);
    for (i = 0; i < count; ++i, region += 24)
        if (r_u32(region + 20) == object && r_s16(region + 10) == 1)
            break;
    return FUN_80029AFC(region, object); /* End pointer is used even if no match. */
}

static sint32 helicopter_turn_delta(uint32 value)
{
    sint32 delta = (sint16)value;
    if (delta > 4096)
        delta = 4096;
    if (delta < -4096)
        delta = -4096;
    return delta;
}

static void helicopter_bob(uint32 object)
{
    uint32 phase = (r_u16(object + 22) + 32) & 2047;
    uint32 height;
    sint32 sine;
    w_u16(object + 22, (uint16)phase);
    height = (uint32)(sint32)r_s16(object + 32) << 16;
    sine = r_s16(0x800802d0 + 2 * phase);
    w_u32(object + 8, height + ((uint32)sine << 4));
}

GDB_CALL uint32 FUN_800420F8(uint32 object, const FF_STACK_PROJECTILE *retained_projectile)
{
    FUNCTION_MARKER(0x800420f8u, "GAME.EXE");
    sint32 player_x = r_s16(0x800b4342), player_z = r_s16(0x800b434a), state, delta, distance, pitch, volume, voice, x, z;
    uint32 angle, phase, dx, dz, speed, sample, result, volume_word, pan, world_x, world_z;
    if ((sint32)r_u32(0x80093d60) < 3)
        return 1;
    if (r_s16(object + 28))
    {
        ff_spatial_sound_80046884(object + 4, &volume_word, &pan);
        volume = FUN_80042820(volume_word, 128);
        FUN_800568C8(r_s16(object + 28) - 1, volume, pan);
    }
    else
        w_u16(object + 28, (uint16)(FUN_80056670(243, 255, 6144) + 1));
    w_u32(0x800947c8, object);
    state = r_s16(object + 26);
    if (!state)
    {
        FUN_80041F5C(object, player_x, player_z);
        if ((sint32)r_u32(0x800941a0) < 3072)
        {
            w_u16(object + 26, 1);
            w_u16(object + 22, 0);
            w_u16(object + 32, (uint16)r_s16(object + 10));
        }
    }
    else if ((state = r_s16(object + 26)) == 1)
    {
        x = r_s16(object + 6);
        z = r_s16(object + 14);
        angle = FUN_8001614C(x, z, player_x, player_z) + 8192;
        delta = helicopter_turn_delta(angle - r_u16(object + 16));
        w_u16(object + 16, (uint16)(r_u16(object + 16) + (delta >> 4)));
        pitch = r_u16(object + 18);
        w_u16(object + 18, (uint16)(pitch - ((sint16)pitch >> 3)));
        helicopter_bob(object);
        if ((uint16)(delta + 1023) < 2047)
        {
            w_u16(object + 26, 2);
            w_u16(object + 24, (uint16)((sint32)r_u32(0x800941a0) >> 1));
            w_u16(object + 30, (uint16)(FUN_80056670(180, 255, 6144) + 1));
        }
    }
    else if (state == 2)
    {
        x = r_s16(object + 6);
        z = r_s16(object + 14);
        angle = FUN_8001614C(x, z, player_x, player_z) + 8192;
        phase = ((angle + (ff_rand_8006D348() & 512) - 8448) << 16) >> 21;
        delta = helicopter_turn_delta(angle - r_u16(object + 16));
        w_u16(object + 16, (uint16)(r_u16(object + 16) + (delta >> 3)));
        pitch = r_u16(object + 18);
        w_u16(object + 18, (uint16)(pitch - ((sint16)pitch >> 3)));
        helicopter_bob(object);
        if (!(r_u32(0x80093dd0) & 1))
        {
            FF_STACK_PROJECTILE projectile;
            speed = r_u16(object + 24);
            dx = ((uint32)(sint32)(sint16)speed * (uint32)(sint32)r_s16(0x800802d0 + 2 * phase)) << 1;
            dz = ((uint32)(sint32)(sint16)speed * (uint32)(sint32)r_s16(0x800806d0 + 2 * phase)) << 1;
            w_u16(object + 24, (uint16)(speed + 128));
            distance = (sint32)r_u32(0x800941a0);
            if ((sint32)((uint32)(distance >> 1) + (uint32)distance) < (sint16)(speed + 128))
            {
                w_u16(object + 24, (uint16)(distance >> 1));
                w_u16(object + 34, (uint16)(r_u16(object + 34) + 1));
            }
            world_x = r_u32(object + 4);
            world_z = r_u32(object + 12);
            FUN_80031B20(879, dx + world_x, 0, world_z - dz, 0, 0, 0);
            projectile.kind = 499;
            projectile.owner = -1;
            /* Preserve reviewed stack carriers or the agreed ground-shot correction */
            projectile.height = retained_projectile ? retained_projectile->height : 0;
            projectile.angle = retained_projectile ? retained_projectile->angle : (uint16)(phase << 5);
            world_x = r_u32(object + 4);
            world_z = r_u32(object + 12);
            if (ff_stack_projectile_query(dx + world_x, world_z - dz, &projectile) >= 0)
            {
                world_x = r_u32(object + 4);
                world_z = r_u32(object + 12);
                ff_stack_projectile_impact(dx + world_x, world_z - dz, &projectile);
            }
            if (!(ff_rand_8006D348() & 3))
            {
                sample = (ff_rand_8006D348() & 3) + 136;
                volume = (ff_rand_8006D348() & 63) + 16;
                FUN_80056670(sample, volume, 6272);
            }
        }
        if ((sint32)r_u32(0x800941a0) >= 4097)
            w_u16(object + 26, 3);
        if (r_s16(object + 34) >= 5)
        {
            w_u16(object + 26, 4);
            w_u8(0x80093dd9, 0);
        }
        voice = r_s16(object + 30);
        if (voice && r_s16(object + 26) != 2)
            FUN_800567F8(voice - 1);
    }
    else if ((state = r_s16(object + 26)) == 3)
    {
        x = r_s16(object + 6);
        z = r_s16(object + 14);
        angle = FUN_8001614C(x, z, player_x, player_z);
        delta = helicopter_turn_delta(angle - r_u16(object + 16));
        w_u16(object + 16, (uint16)(r_u16(object + 16) + (delta >> 3)));
        distance = (sint32)(r_u32(0x800941a0) - 3072);
        if (distance > 3072)
            distance = 3072;
        pitch = r_s16(object + 18);
        w_u16(object + 18, (uint16)((uint32)pitch + (uint32)((sint32)(((uint32)distance << 2) - (uint32)pitch) >> 4)));
        helicopter_bob(object);
        if ((uint16)(delta + 1023) < 2047)
        {
            w_u16(object + 26, 0);
            w_u32(object + 8, (uint32)(sint32)r_s16(object + 32) << 16);
        }
    }
    else if (state == 4)
    {
        w_u32(0x800947c8, 0);
        FUN_80041F5C(object, 0, 28672);
        if ((sint32)r_u32(0x800941a0) < 3072)
        {
            w_u32(object, 55);
            voice = r_s16(object + 30);
            if (voice)
                FUN_800567F8(voice - 1);
            voice = r_s16(object + 28);
            if (voice)
                FUN_800567F8(voice - 1);
        }
    }
    result = r_u32(object + 4);
    if ((sint32)result < (sint32)0xeff00000)
    {
        w_u32(object + 4, 0xeff00000);
        return 1;
    }
    result = r_u32(object + 4);
    if ((sint32)result > 0)
        w_u32(object + 4, 0);
    return result;
}

GDB_CALL uint32 FUN_80033F7C(sint32 index, uint32 retained_xyz[3])
{
    FUNCTION_MARKER(0x80033f7cu, "GAME.EXE");
    uint32 actor = 0x800b4318 + 244u * (uint32)index, region = r_u32(actor + 108), object = r_u32(region + 20), kind, type, x, z;
    uint32 point[3];
    sint32 turn;
    w_u16(region + 8, 17);
    turn = r_s16(actor + 104);
    if (r_u32(actor + 28) & 0x800)
    {
        kind = r_u32(object);
        if (kind == 627)
            type = 493;
        else if (kind == 663)
            type = 494;
        else if (r_u32(object) == 813)
            type = 511;
        else
            return 813;
        if (!retained_xyz)
        {
            /* Fix original unwritten coordinates using the pickup attachment */
            FUN_8001B9F4(index, 3, point);
            retained_xyz = point;
        }
        w_u16(actor + 138, (uint16)((uint16)r_s16(actor + 138) + 1));
        w_u32(object, kind == 663 ? 664 : r_u32(object) + 1);
        /* Preserve caller coordinates when their original carrier is known */
        w_u8(actor + 126, (uint8)FUN_800167C4(type, retained_xyz[0], retained_xyz[1], retained_xyz[2], 0));
        return FUN_80056670(32, 255, 6144);
    }
    if (r_s16(actor + 84))
        return FUN_800292F8(index);
    if (!retained_xyz)
        retained_xyz = point;
    w_u16(actor + 56, (uint16)turn);
    FUN_8001B9F4(index, 3, retained_xyz);
    x = r_u32(object + 4) - (retained_xyz[0] - r_u32(actor + 40));
    z = r_u32(object + 12) - (retained_xyz[2] - r_u32(actor + 48));
    if ((uint32)(uint16)r_s16(actor + 52) - 1u < 2u)
    {
        x -= (uint32)(sint32)r_s16(0x800802d0 + 2u * (uint32)turn) << 4;
        z += (uint32)(sint32)r_s16(0x800806d0 + 2u * (uint32)turn) << 4;
    }
    if (turn == 512 || turn == 1536)
        w_u32(actor + 40, x);
    if (!turn || turn == 1024)
        w_u32(actor + 48, z);
    w_u32(actor + 20, 0);
    w_u32(actor + 24, 0);
    return 1024;
}

GDB_CALL uint32 FUN_80041B54(uint32 object, uint32 retained_s3)
{
    FUNCTION_MARKER(0x80041b54u, "GAME.EXE");
    uint32 actor, angle, speed, x, y, z, dx, dz, result;
    sint32 hit, contact, target, source_x, source_z, target_x, target_z;
    x = r_u32(object + 4);
    y = r_u32(object + 8);
    z = r_u32(object + 12);
    FUN_80016654(575, x, y, z);
    target = r_s16(object + 26);
    if (target != -1)
    {
        actor = 0x800b4318 + 244u * (uint32)target;
        source_x = r_s16(object + 6);
        source_z = r_s16(object + 14);
        target_x = r_s16(actor + 42);
        target_z = r_s16(actor + 50);
        w_u16(object + 16, (uint16)FUN_8001614C(source_x, source_z, target_x, target_z));
    }
    angle = (r_u16(object + 16) >> 5) * 2;
    speed = (uint32)(sint32)(sint16)r_u16(object + 24);
    dx = speed * (uint32)(sint32)r_s16(0x800802d0 + angle);
    dz = speed * (uint32)(sint32)r_s16(0x800806d0 + angle);
    x = r_u32(object + 4) + dx;
    w_u32(object + 4, x);
    z = r_u32(object + 12) - dz;
    w_u32(object + 12, z);
    x = r_u32(object + 4);
    hit = FUN_800199AC(x, z);
    contact = hit;
    if (hit < 0)
    {
        x = r_u32(object + 4);
        z = r_u32(object + 12);
        contact = FUN_80019BA8(x, z, object);
    }
    if (contact >= 0)
    {
        FUN_80056670(19, 255, 6144);
        x = r_u32(object + 4);
        y = r_u32(object + 8);
        z = r_u32(object + 12);
        FUN_80031C50(893, x, y, z);
        w_u32(0x800941a8, 16);
        w_u32(0x80094190, 8);
        /* 31C50 saves inherited S3 at SP-12; 1F004 reads that retained word. */
        if (hit >= 0)
            FUN_8001F004(hit, object, retained_s3);
        x = r_u32(object + 4);
        z = r_u32(object + 12);
        FUN_80031F64(x, z, object);
        w_u32(object, 0xffffffffu);
        return 0xffffffffu;
    }
    result = r_u16(object + 32) - 1;
    w_u16(object + 32, (uint16)result);
    result = (uint32)((sint32)(result << 16) >> 1);
    y = r_u32(object + 8) - result;
    w_u32(object + 8, y);
    if ((sint32)y <= 0)
        return result;
    w_u32(object, 0xffffffffu);
    return 0xffffffffu;
}

GDB_CALL uint32 FUN_8002A168(uint32 object)
{
    FUNCTION_MARKER(0x8002a168u, "GAME.EXE");
    uint32 region = 0x8009ec70, i = 0, yaw, pitch, roll, angle, target, value, offset, result, volume, pan, height, velocity, wave, position;
    sint32 count = (sint32)r_u32(0x8009479c), vertical, difference, speed, turn, x, z, target_x, target_z, voice;
    while ((sint32)i < count)
    {
        if (r_u32(region + 20) == object)
            break;
        ++i;
        region += 24;
    }
    yaw = r_u16(object + 16);
    pitch = r_u16(object + 18);
    roll = r_u16(object + 20);
    if (r_u32(0x80093d58) == 15 && r_u32(0x80093d60) == 2 && r_u32(0x80093dfc) == 1)
    {
        FUN_800292A4(r_u32(0x800940ac));
        w_u32(r_u32(0x800940ac), 55);
    }
    if (r_s16(object + 30))
    {
        ff_spatial_sound_80046884(object + 4, &volume, &pan);
        volume = (uint32)FUN_80042820((sint32)volume, 64);
        voice = r_s16(object + 30) - 1;
        FUN_800568C8(voice, (sint32)volume, (sint32)pan);
    }
    else
        w_u16(object + 30, (uint16)(FUN_80056670(243, 0, 6144) + 1));
    if (!r_s16(object + 28) && !r_u32(0x80093df0))
        return pitch << 16;
    vertical = (sint16)pitch >> 8;
    if (vertical == -1)
        vertical = 0;
    if ((sint32)r_u32(object + 8) > (sint32)0xff400000u)
    {
        w_u16(region + 12, 3);
        FUN_80029AFC(region, object);
    }
    else
        w_u16(region + 12, 0);
    x = r_s16(object + 6);
    z = r_s16(object + 14);
    target_x = r_s16(0x80093dec);
    target_z = r_s16(0x80093dee);
    angle = (uint32)FUN_8001614C(x, z, target_x, target_z);
    target = 0xfd000000;
    if ((sint32)r_u32(0x800941a0) < 2304 && !vertical)
        target = (uint16)(yaw - 0x3801) < 0xfff ? 0 : 0xfe800000;
    height = r_u32(object + 8);
    velocity = r_u32(0x8008e510);
    value = velocity + (uint32)((sint32)(target - height - velocity) >> 4);
    w_u32(0x8008e510, value);
    value = (uint32)((sint32)(value + 16) >> 5) + r_u32(object + 8);
    w_u32(object + 8, value);
    if ((sint32)value <= (sint32)0xfd800000u)
    {
        difference = (sint16)(angle - yaw);
        if (difference > 8192)
            difference = 8192;
        if (difference < -8192)
            difference = -8192;
        roll -= (uint32)(((sint16)roll + difference) >> 4);
        yaw -= (uint32)((sint16)roll >> 4);
        difference = (sint32)(r_u32(0x800941a0) - 2048);
        if (difference > 2048)
            difference = 2048;
        pitch += (uint32)((sint32)((uint32)difference - (uint32)(sint32)(sint16)pitch) >> 5);
        speed = (sint16)r_u16(0x800802d0 + ((pitch >> 1) & 0xffe)) >> 8;
        offset = (yaw >> 4) & 0xffe;
        wave = (uint32)(sint32)r_s16(0x800802d0 + offset);
        position = r_u32(object + 4);
        w_u32(object + 4, position + (uint32)speed * wave);
        wave = (uint32)(sint32)r_s16(0x800806d0 + offset);
        position = r_u32(object + 12);
        w_u32(object + 12, position - (uint32)speed * wave);
    }
    else
    {
        if ((sint32)r_u32(0x800941a0) < 2304 && (uint16)(yaw - 0x3801) < 0xfff)
        {
            if (r_s16(object + 10) == -1)
            {
                w_u32(0x80094198, 0);
                w_u16(object + 28, 0);
                if (r_u32(0x80093d58) != 18 || (sint32)r_u32(object + 4) > (sint32)0xe7000000u || r_u32(0x80093d58) != 18)
                    w_u8(0x80093dd9, 0);
                value = r_u32(0x80093df0);
                if ((sint32)value > 0)
                {
                    value--;
                    w_u32(0x80093df0, value);
                    if (!value && r_u32(0x80093d58) == 18)
                    {
                        w_u16(0x80093dec, 0xe0c8);
                        w_u16(0x80093dee, 1600);
                        w_u8(0x80093e00, 0);
                        w_u16(object + 28, 1);
                    }
                }
            }
        }
        else
        {
            roll -= (uint32)((sint16)roll >> 4);
            pitch -= (uint32)((sint16)pitch >> 4);
        }
        turn = r_s16(0x8008e514);
        turn += (0x4000 - (sint16)yaw - turn) >> 4;
        w_u16(0x8008e514, (uint16)turn);
        yaw += (uint32)(((sint16)turn + 16) >> 5);
    }
    w_u32(0x80093df4, (uint32)(sint32)r_s16(object + 6));
    result = (uint32)(sint32)r_s16(object + 14);
    w_u32(0x80093df8, result);
    w_u16(object + 16, (uint16)yaw);
    w_u16(object + 18, (uint16)pitch);
    w_u16(object + 20, (uint16)roll);
    return result;
}

static void sequence_train_door_sound(void)
{
    FUN_80056670(124, 127, 5632);
    FUN_80056670(150, 255, 6144);
}

GDB_CALL uint32 FUN_8003FDCC(uint32 object)
{
    FUNCTION_MARKER(0x8003fdccu, "GAME.EXE");
    sint32 side, maximum, state, distance, value, volume, denominator, numerator;
    uint32 acceleration, z, result;
    if (!r_s16(object + 30))
        w_u16(object + 30, (uint16)(FUN_80056670(145, 0, 6656) + 1));
    side = (sint32)r_u32(object + 4) > 0x0f9fffff;
    maximum = side ? 16640 : 19200;
    value = r_s16(object + 24);
    z = r_u32(object + 12);
    w_u32(object + 12, side ? z - ((uint32)value << 9) : z + ((uint32)value << 9));
    acceleration = 0x80093d00 + 2u * side;
    value = r_u16(object + 24);
    value = (sint16)(value + r_u16(acceleration));
    w_u16(object + 24, (uint16)value);
    if (value > maximum)
        w_u16(object + 24, (uint16)maximum);
    if (r_s16(object + 24) < 0)
    {
        w_u16(object + 24, 0);
        w_u16(acceleration, 0);
        w_u16(object + 28, 2);
        sequence_train_door_sound();
        FUN_80056670(241, 255, 6144);
    }
    state = r_s16(object + 28);
    if (state == 0 || state == 1)
    {
        z = r_u32(object + 12);
        distance = (sint32)(side ? z - 0x0d800000u : 0x0d800000u - z);
        value = (sint32)(distance < 0 ? (uint32)distance + 0x3e800000u : 0x3e800000u - (uint32)distance);
        value = (value >> 8) / 16000;
        volume = (sint32)((uint32)value * (uint32)(sint32)r_s16(object + 24)) / (maximum >> 1);
        if (volume < 0)
            volume = 0;
        if (volume > 255)
            volume = 255;
        FUN_80056F08(r_s16(object + 30) - 1, volume);
        if (state == 1)
            goto finish;
        value = (sint16)r_u16(object + 24);
        if (value >= maximum && (uint32)distance - 1u <= 0x1f3ffffe)
        {
            value >>= 7;
            numerator = (sint32)(0u - ((uint32)value * (uint32)value << 7));
            denominator = (sint32)((uint32)(distance >> 16) << 1);
            if (!denominator || (denominator == -1 && numerator == (sint32)0x80000000u))
                ff_wip_stop(__FUNCTION__, __FILE__, __LINE__); /* Original400CC/400E4 BREAK. */
            w_u16(acceleration, (uint16)(numerator / denominator));
            w_u16(object + 28, 1);
            FUN_80056670(242, 255, 5632);
        }
        if (!r_s16(acceleration))
        {
            if (!r_u8(0x800941a4))
                w_u16(object + 24, 65535);
            if (!r_s16(acceleration) && r_u8(0x800941a4))
            {
                w_u16(object + 28, 2);
                sequence_train_door_sound();
            }
        }
    }
    else if (state == 2)
    {
        if (side)
        {
            value = (sint32)r_u32(0x80094674);
            if (value < 32)
                w_u32(0x80094674, (uint32)value + 1);
        }
        if ((sint32)r_u32(0x80094674) >= 9)
        {
            if (r_u8(0x800941a4) && r_u32(0x80093d58) == 13)
            {
                w_u16(r_u32(0x800941c8) + 12, 0);
                w_u16(r_u32(0x800941cc) + 12, 0);
            }
            else if ((sint32)r_u32(0x80093d60) >= 2)
                w_u8(0x80093dd9, 0);
        }
        if ((sint32)r_u32(0x80094674) >= 31)
        {
            if (!r_u8(0x800941a4))
            {
                w_u16(object + 28, 3);
                w_u16(object + 22, 100);
            }
            else
            {
                if ((sint32)r_u32(0x80093e04) > 0)
                {
                    w_u16(object + 28, 4);
                    sequence_train_door_sound();
                }
                w_u32(0x80094674, 32);
            }
        }
    }
    else if (state == 3)
    {
        value = (sint16)(r_u16(object + 22) - 1);
        w_u16(object + 22, (uint16)value);
        if (value < 0)
        {
            w_u16(object + 28, 4);
            sequence_train_door_sound();
        }
    }
    else if (state == 4)
    {
        if (!r_u32(0x80094674))
        {
            value = (sint32)r_u32(0x80093d60);
            w_u16(object + 28, 0);
            w_u16(acceleration, 50);
            if (value >= 2)
                w_u8(0x80093dd9, 1);
        }
        if (side)
        {
            value = (sint32)r_u32(0x80094674);
            if (value > 0)
                w_u32(0x80094674, (uint32)value - 1);
        }
    }
finish:
    result = (uint32)(sint32)r_s8(0x80093de0);
    if ((uint32)side != result)
        return result;
    result = r_u32(0x80094674);
    if (result)
        return result;
    value = (sint32)r_u32(0x80093e04);
    result = (uint32)value + 1;
    if (value > 0)
    {
        z = r_u32(object + 12);
        w_u32(0x8009479c, 0);
        w_u32(0x80093e04, result);
        w_u32(0x800b443c, z);
        w_u32(0x800b4348, z);
    }
    return result;
}

GDB_CALL uint32 FUN_8003F9C0(uint32 object)
{
    FUNCTION_MARKER(0x8003f9c0u, "GAME.EXE");
    static const sint32 turns[4] = {-64, 64, -112, 112};
    static const uint32 flags[4] = {1, 1, 2, 4};
    uint32 x = r_u32(0x800b4340), z = r_u32(0x800b4348), p, offset, dx, dz, scale;
    sint32 distance;
    int i;
    w_u32(0x80091a18, 0);
    w_u32(0x80091a3c, x);
    w_u32(0x80091a40, z);
    distance = FUN_800163F4((sint32)x >> 16, (sint32)z >> 16, r_s16(object + 6), r_s16(object + 14));
    FUN_8003F6EC(object);
    if (distance >= 65)
        for (i = 0; i < 4; ++i)
        {
            p = 0x80091a1c + 8 * i;
            offset = 2u * ((uint32)((r_s16(object + 16) >> 5) + turns[i]) & 2047);
            scale = i < 2 ? 512 : 900;
            dx = (uint32)(sint32)r_s16(0x800802d0 + offset) * scale;
            dz = 0u - (uint32)(sint32)r_s16(0x800806d0 + offset) * scale;
            w_u32(p, dx);
            w_u32(p + 4, dz);
            if (r_s16(object + 30) == 1)
            {
                w_u32(p, 0u - dx);
                w_u32(p + 4, 0u - dz);
            }
            if (FUN_8003F670(object, r_u32(p), r_u32(p + 4)))
                w_u32(0x80091a18, r_u32(0x80091a18) | flags[i]);
        }
    FUN_8003F81C(object);
    if (!r_s16(object + 30))
        for (i = 0; i < 4; ++i)
        {
            p = 0x80091a1c + 8 * i;
            FUN_8003F5A0(0x800b4318, object, r_u32(object + 4) + r_u32(p), r_u32(object + 12) + r_u32(p + 4));
        }
    /* Four debug points call the original empty6A0F8. The final call leaves
  * V0 loaded with object Z, not the last collision result. */
    return r_u32(object + 12);
}

GDB_CALL uint32 FUN_80044844(uint32 object)
{
    FUNCTION_MARKER(0x80044844u, "GAME.EXE");
    sint32 start = r_s16(object + 26);
    sint32 elapsed = (sint32)(r_u32(0x80093dd0) - (uint32)start), count, i, bounce, voice, wave;
    uint32 volume, pan, angle, offset, speed, p, end, region, position;
    if (r_s16(object + 30))
    {
        ff_spatial_sound_80046884(object + 4, &volume, &pan);
        volume = FUN_80042820((sint32)volume, (sint32)((uint32)elapsed - 130u));
        FUN_800568C8(r_s16(object + 30) - 1, (sint32)volume, (sint32)pan);
        if (elapsed >= 96)
            FUN_80056840(r_s16(object + 30) - 1, 6912u - 8u * ((uint32)elapsed - 95u));
    }
    if (r_s16(object + 26) == -1)
    {
        if (!r_s8(0x80093dd9) || r_u32(0x80093d60) != 4 || FUN_80047434((sint32)0xf4000000u))
            goto region_update;
        w_u16(object + 30, (uint16)(FUN_80056670(132, 0, 6912) + 1));
        w_u16(object + 26, r_u16(0x80093dd0));
        w_u32(0x800947c8, object);
    }
    if (elapsed < 0)
        goto region_update;
    if (elapsed < 16)
    {
        offset = 2 * (r_u16(object + 16) >> 5);
        speed = 96;
    }
    else if (elapsed < 59)
    {
        angle = r_u16(object + 16) - 256;
        w_u16(object + 16, (uint16)angle);
        offset = (angle >> 4) & 0xffe;
        speed = 129;
    }
    else if (elapsed < 112)
    {
        wave = r_s16(object + 24);
        if (wave < 192)
            w_u16(object + 24, (uint16)(wave + 8));
        offset = 2 * (r_u16(object + 16) >> 5);
        speed = (uint32)(sint32)r_s16(object + 24);
    }
    else
    {
        if (elapsed == 112)
        {
            end = 0x800b89f0 + 36u * r_u32(0x8009403c);
            for (p = 0x800b89f0; p < end; p += 36)
                if (r_u32(p) == 173)
                    w_u32(p, 174);
            voice = r_s16(object + 28);
            if (voice)
            {
                FUN_800567F8(voice - 1);
                w_u16(object + 28, 0);
            }
            voice = r_s16(object + 30);
            if (voice)
            {
                FUN_800567F8(voice - 1);
                w_u16(object + 30, 0);
            }
            FUN_80056670(135, 255, 6144);
            FUN_80056670(154, 160, 6144);
            w_u32(0x800941a8, 16);
            w_u32(0x80094190, 4);
        }
        else if (elapsed < 120)
        {
            bounce = -((sint32)((uint32)r_u16(0x800802d0 + (((uint32)elapsed - 112) << 8)) << 16) >> 20);
            w_u16(object + 18, (uint16)bounce);
            w_u32(object + 8, (uint32)(bounce >> 6) << 15);
        }
        else if (elapsed < 129)
        {
            bounce = -((sint32)((uint32)r_u16(0x800802d0 + (((uint32)elapsed - 120) << 8)) << 16) >> 21);
            w_u16(object + 18, (uint16)bounce);
            w_u32(object + 8, (uint32)(bounce >> 6) << 14);
        }
        else
        {
            if (elapsed != 129)
                return 129;
            w_u8(0x80093dd9, 0);
            w_u32(0x800947c8, 0);
        }
        goto region_update;
    }
    wave = r_s16(0x800802d0 + offset);
    position = r_u32(object + 4);
    w_u32(object + 4, position - speed * (uint32)wave);
    if (elapsed >= 59)
        speed = (uint32)(sint32)r_s16(object + 24);
    wave = r_s16(0x800806d0 + offset);
    position = r_u32(object + 12);
    w_u32(object + 12, position + speed * (uint32)wave);
    if (elapsed >= 16 && elapsed < 59)
        w_u16(object + 24, 129);
    if (elapsed == 90)
        w_u16(object + 28, (uint16)(FUN_80056670(131, 255, 5632) + 1));
region_update:
    region = 0x8009ec70;
    count = (sint32)r_u32(0x8009479c);
    if (count <= 0)
        return (uint32)count;
    for (i = 0; i < count; ++i, region += 24)
        if (r_u32(region + 20) == object)
            break;
    if (i < (sint32)r_u32(0x8009479c))
        return FUN_80029AFC(region, object);
    return 0;
}

static void sequence_exhaust_pair(uint32 object, uint32 width_address)
{
    uint32 r, x, y, z, width, z_offset;
    int i;
    for (i = 0; i < 2; i++)
    {
        r = ff_rand_8006D348();
        x = r_u32(object + 4);
        width = r_u32(width_address);
        x = i ? x + width + ((r & 63) << 16) : x - width - ((r & 63) << 16);
        z = r_u32(object + 12);
        z_offset = r_u32(0x8007f320);
        y = r_u32(object + 8);
        FUN_80031B20(871, x, y, z + z_offset, 0, 0, 0);
    }
}

GDB_CALL uint32 FUN_80042BA0(uint32 object)
{
    FUNCTION_MARKER(0x80042ba0u, "GAME.EXE");
    uint32 region = 0x8009ec70, volume, pan, quiet, position[3], step, z;
    sint32 voice;
    sint32 tick, count = (sint32)r_u32(0x8009479c), i;
    for (i = 0; i < count; ++i, region += 24)
        if (r_u32(region + 20) == object)
            break;
    tick = (sint32)r_u32(0x80093dd0);
    if (tick == 1)
    {
        w_u32(0x800944b8, FUN_80056670(147, 0, 6144));
        w_u32(0x800944c0, FUN_80056670(132, 0, 6400));
        position[0] = r_u32(object + 4);
        position[1] = r_u32(object + 8);
        position[2] = 0x01000000;
        ff_stack_spatial_sound_80046884(position, 0x80091bf4, 0x80091bf0);
        volume = FUN_80042820((sint32)r_u32(0x80091bf4), -80);
        w_u32(0x80091bf4, volume);
        quiet = FUN_80042820((sint32)volume, -128);
        voice = (sint32)r_u32(0x800944b8);
        volume = r_u32(0x80091bf4);
        pan = r_u32(0x80091bf0);
        FUN_800568C8(voice, (sint32)volume, (sint32)pan);
        voice = (sint32)r_u32(0x800944c0);
        pan = r_u32(0x80091bf0);
        FUN_800568C8(voice, (sint32)quiet, (sint32)pan);
    }
    else if (tick >= 100)
    {
        if (tick < 130)
        {
            if (tick & 1)
                sequence_exhaust_pair(object, 0x8007f31c);
            if (r_u32(0x80093dd0) == 110)
                w_u32(0x800944bc, FUN_80056670(146, 0, 4608));
            if ((sint32)r_u32(0x80093dd0) >= 111)
            {
                ff_spatial_sound_80046884(object + 4, &volume, &pan);
                FUN_800568C8((sint32)r_u32(0x800944bc), (sint32)volume, (sint32)pan);
            }
        }
        else if ((sint32)r_u32(0x80093dd0) < 200)
        {
            ff_spatial_sound_80046884(object + 4, &volume, &pan);
            FUN_800568C8((sint32)r_u32(0x800944bc), (sint32)volume, (sint32)pan);
            if (r_u32(0x80093dd0) & 2)
                sequence_exhaust_pair(object, 0x8007f324);
            step = (uint32)(sint32)r_s16(object + 26) * 196608;
            z = r_u32(object + 12);
            w_u32(object + 12, z - step);
            w_u16(object + 26, (uint16)(r_u16(object + 26) + 1));
        }
        else
        {
            FUN_800567F8((sint32)r_u32(0x800944bc));
            w_u32(object, 749);
            w_u32(object + 12, 0x01000000);
            FUN_8003B77C(region, (sint32)r_u32(object));
            w_u32(0x8009479c, r_u32(0x8009479c) - 1);
        }
    }
    return FUN_80029AFC(region, object);
}

GDB_CALL uint32 FUN_80044334(uint32 object)
{
    FUNCTION_MARKER(0x80044334u, "GAME.EXE");
    sint32 start = r_s16(object + 26), elapsed = (sint32)(r_u32(0x80093dd0) - (uint32)start), count, i, voice, wave;
    uint32 angle, offset, speed, point[3], region, x, y, z;
    if (start == -1)
    {
        if (r_u32(0x80093d60) != 2)
            goto region_update;
        w_u16(object + 26, r_u16(0x80093dd0));
    }
    /* MIPS keeps elapsed computed with the old -1 start on the arming frame. */
    if (elapsed < 15)
        return elapsed < 50;
    if (elapsed < 50)
    {
        w_u32(0x800947c8, object);
        offset = 2 * (r_u16(object + 16) >> 5);
        speed = 88;
    }
    else if (elapsed < 60)
    {
        if (elapsed == 50)
            FUN_80056670(233, 120, 7168);
        angle = r_u16(object + 16) - 512;
        w_u16(object + 16, (uint16)angle);
        offset = (angle >> 4) & 0xffe;
        speed = 160;
    }
    else if (elapsed < 140)
    {
        if (elapsed == 65)
            FUN_80056670(233, 162, 7168);
        angle = r_u16(object + 16) - 256;
        w_u16(object + 16, (uint16)angle);
        offset = (angle >> 4) & 0xffe;
        speed = 160u - 2u * ((uint32)elapsed - 60u);
    }
    else
    {
        if (elapsed != 140)
            return 140;
        voice = r_s16(object + 28);
        if (voice)
            FUN_800567F8(voice - 1);
        w_u8(0x80093dd9, 0);
        goto region_update;
    }
    wave = r_s16(0x800802d0 + offset);
    x = r_u32(object + 4);
    w_u32(object + 4, x - speed * (uint32)wave);
    wave = r_s16(0x800806d0 + offset);
    z = r_u32(object + 12);
    w_u32(object + 12, z + speed * (uint32)wave);
    if (elapsed >= 60)
    {
        if (elapsed == 85)
            FUN_80056670(230, 95, 5632);
        if (elapsed >= 86 && (elapsed & 1))
            for (i = 0; i < 2; ++i)
            {
                FUN_80010040();
                FUN_80011D20();
                FUN_800101CC(32768);
                FUN_8001033C(r_s16(object + 16));
                FUN_800614F4(i ? -128 : 128, 0, -256, point);
                x = r_u32(object + 4);
                y = r_u32(object + 8);
                z = r_u32(object + 12);
                FUN_80031B20(871, (point[0] << 16) + x, y + 196608, (point[2] << 16) + z, 0, 0, 0);
                FUN_80010028();
            }
    }
region_update:
    region = 0x8009ec70;
    count = (sint32)r_u32(0x8009479c);
    if (count <= 0)
        return (uint32)count;
    for (i = 0; i < count; ++i, region += 24)
        if (r_u32(region + 20) == object)
            break;
    if (i < (sint32)r_u32(0x8009479c))
        return FUN_80029AFC(region, object);
    return 0;
}

static void sequence_tower_smoke(uint32 kind, uint32 y)
{
    uint32 offset = 2u * (ff_rand_8006D348() & 2047);
    FUN_80016654(kind, r_u32(0x800b8a18) + ((uint32)(sint32)r_s16(0x800802d0 + offset) << 9), y + 0xfa800000u, r_u32(0x800b8a20) - ((uint32)(sint32)r_s16(0x800806d0 + offset) << 9));
}

GDB_CALL uint32 FUN_8003AED8(uint32 frame)
{
    FUNCTION_MARKER(0x8003aed8u, "GAME.EXE");
    uint32 tick, y, kind, value;
    int i, j;
    static const uint32 xs[] = {0xffb00000, 0x00b80000, 0xffb00000, 0x00380000};
    static const uint32 zs[] = {0xfa800000, 0xfb580000, 0xfc700000, 0xfda00000};
    static const uint32 blast_y[] = {0xfd200000, 0xfcc00000, 0xfc800000, 0xfc400000, 0xfc000000};
    static const uint32 pieces[][3] = {{0x00880000, 0x02880000, 0x2000}, {0x00380000, 0x02d80000, 0xa000}, {0x00900000, 0x02e00000, 0x6000}, {0x00380000, 0x02800000, 0xe000}};
    w_u32(0x8009403c, 6);
    tick = r_u32(0x80093dd0);
    w_u32(0x800b89f0, (sint32)tick < 500 ? 444 : 446);
    if (tick - 501u < 129u)
    {
        value = r_u32(0x800b8a1c) + ((tick - 500u) << 13);
        w_u32(0x800b8a1c, value);
        if ((sint32)value > 0x02f00000)
            w_u32(0x800b8a1c, 0x02f00000);
        w_u16(0x800b8a28, (uint16)((uint16)r_s16(0x800b8a28) + ((sint32)(r_u32(0x80093dd0) - 500u) >> 1)));
    }
    y = r_u32(0x800b8a1c);
    if (!(r_u32(0x80093dd0) & 1))
        for (i = 0; i < 4; ++i)
            FUN_80016654(575, xs[i], 0xff400000, zs[i]);
    if (!(r_u32(0x80093dd0) & 31))
        for (i = 0; i < 4; ++i)
            FUN_80016654(576, xs[i], 0xff400000, zs[i]);
    if (r_u32(0x80093dd0) - 201u < 294u)
        sequence_tower_smoke(575, y);
    if (r_u32(0x80093dd0) - 496u < 134u)
    {
        sequence_tower_smoke(575, y);
        sequence_tower_smoke(575, y);
        sequence_tower_smoke(576, y);
    }
    w_u32(0x800b8a14, (sint32)r_u32(0x80093dd0) < 632 ? 447 : 55);
    for (i = 0; i < 5; ++i)
        if (r_u32(0x80093dd0) == 470u + 5u * i)
        {
            FUN_80056670(19, 192, 6144);
            FUN_80031C50(893, 0x00600000, blast_y[i], 0x02b00000);
        }
    if (r_u32(0x80093dd0) == 500)
    {
        FUN_80056670(19, 255, 6144);
        FUN_80031C50(893, 0x00600000, 0xfc800000, 0x02b00000);
        FUN_80031C50(603, 0x00600000, 0xfc800000, 0x02b00000);
        w_u32(0x800941a8, 16);
        w_u32(0x80094190, 16);
        for (j = 0; j < 2; ++j)
            for (i = 0; i < 4; ++i)
                FUN_800168BC(448, pieces[i][0], j ? 0xfd200000 : 0xfc400000, pieces[i][1], pieces[i][2]);
    }
    tick = r_u32(0x80093dd0);
    if (tick == 600 || tick == 620)
    {
        FUN_80056670(19, 192, 6144);
        FUN_80031C50(892, 0x02000000, y + 0xfb000000u, r_u32(0x800b8a20));
        FUN_80031C50(603, 0x02000000, y + 0xfb000000u, r_u32(0x800b8a20));
        w_u32(0x800941a8, 8);
        w_u32(0x80094190, 8);
    }
    if (r_u32(0x80093dd0) == 630)
    {
        FUN_80056670(19, 255, 6144);
        FUN_80031C50(891, 0x02800000, y + 0xfb000000u, r_u32(0x800b8a20));
        FUN_80031C50(602, 0x02800000, y + 0xfb000000u, r_u32(0x800b8a20));
        /* S5 originally holds the input frame, then becomes the debris counter. */
        for (frame = 0; frame < 64; ++frame)
        {
            kind = (ff_rand_8006D348() & 3) + 898;
            FUN_80031CD8(kind, 0x02800000, y + 0xfb000000u, r_u32(0x800b8a20));
        }
        w_u32(0x800941a8, 16);
        w_u32(0x80094190, 16);
    }
    if ((sint32)r_u32(0x80093dd0) >= (sint32)r_u32(0x800941c0))
        return 0;
    w_u32(0x800b8a38, 854);
    FUN_80047A40((sint32)frame, 0x800b8a38, 2);
    w_u32(0x800b8a5c, 858);
    FUN_80047A40((sint32)frame, 0x800b8a5c, 3);
    w_u32(0x800b8a80, 858);
    FUN_80047A40((sint32)frame, 0x800b8a80, 4);
    w_u32(0x800b8aa4, 855);
    return FUN_80047A40((sint32)frame, 0x800b8aa4, 5);
}

GDB_CALL uint32 FUN_8003AB18(uint32 frame)
{
    FUNCTION_MARKER(0x8003ab18u, "GAME.EXE");
    uint32 point[3], x, y, z, kind, tick;
    int i;
    w_u32(0x8009403c, 4);
    FUN_80047A40(frame, 0x800b8a14, 1);
    FUN_80010040();
    FUN_80011D20();
    FUN_800104A0(r_s16(0x800b8a28));
    FUN_800101CC(r_s16(0x800b8a26));
    FUN_8001033C(-r_s16(0x800b8a24));
    if (frame == 136 || frame == 168)
    {
        FUN_800614F4(frame == 168 ? (sint32)0xff080000u : 0x00f80000, 0, (sint32)0xfea80000u, point);
        x = r_u32(0x800b8a18) + point[0];
        y = r_u32(0x800b8a1c) + point[1];
        z = r_u32(0x800b8a20) + point[2];
        FUN_80031C50(893, x, y, z);
        for (frame = 0; frame < 8; ++frame)
        {
            kind = (ff_rand_8006D348() & 3) + 898;
            FUN_80031CD8(kind, x, y, z);
        }
        w_u32(0x800941a8, 8);
        w_u32(0x80094190, 8);
    }
    /* S2 really is reused as the particle counter; keep its new value for
  * subsequent tests AND the final two47A40 calls. */
    if (frame - 137u < 262u)
    {
        if (!(frame & 1))
            for (i = 0; i < 5; ++i)
            {
                x = ((ff_rand_8006D348() & 63) - 32) << 19;
                FUN_800614F4((sint32)x, -1048576, -22544384, point);
                FUN_80016654(575, r_u32(0x800b8a18) + point[0], r_u32(0x800b8a1c) + point[1], r_u32(0x800b8a20) + point[2]);
            }
        if (!(frame & 3))
        {
            FUN_800614F4(16252928, -1048576, -22544384, point);
            FUN_80016654(576, r_u32(0x800b8a18) + point[0], r_u32(0x800b8a1c) + point[1], r_u32(0x800b8a20) + point[2]);
            if ((sint32)frame >= 185)
            {
                FUN_800614F4(-16252928, -1048576, -22544384, point);
                FUN_80016654(576, r_u32(0x800b8a18) + point[0], r_u32(0x800b8a1c) + point[1], r_u32(0x800b8a20) + point[2]);
            }
        }
    }
    if (frame == 320 || frame == 352)
    {
        FUN_80056670(19, 128, 6144);
        FUN_80031C50(892, r_u32(0x800b8a18), r_u32(0x800b8a1c), r_u32(0x800b8a20));
        for (frame = 0; frame < 8; ++frame)
        {
            kind = (ff_rand_8006D348() & 3) + 898;
            FUN_80031CD8(kind, r_u32(0x800b8a18), r_u32(0x800b8a1c), r_u32(0x800b8a20));
        }
        w_u32(0x80094190, 4);
    }
    if (frame == 399)
    {
        FUN_80031C50(891, r_u32(0x800b8a18), 0, r_u32(0x800b8a20));
        FUN_80031C50(602, r_u32(0x800b8a18), 0, r_u32(0x800b8a20));
        for (frame = 0; frame < 64; ++frame)
        {
            kind = (ff_rand_8006D348() & 3) + 898;
            FUN_80031CD8(kind, r_u32(0x800b8a18), r_u32(0x800b8a1c), r_u32(0x800b8a20));
        }
        w_u32(0x800941a8, 8);
        w_u32(0x80094190, 8);
    }
    FUN_80010028();
    tick = r_u32(0x80093dd0);
    kind = 856;
    if ((sint32)tick >= 144 && ((sint32)tick >= 192 || !(tick & 8)))
        kind = 857;
    w_u32(0x800b8a14, kind);
    w_u32(0x800b8a38, 858);
    FUN_80047A40(frame, 0x800b8a38, 2);
    w_u32(0x800b8a5c, 859);
    return FUN_80047A40(frame, 0x800b8a5c, 3);
}

GDB_CALL uint32 FUN_80044D90(uint32 object)
{
    FUNCTION_MARKER(0x80044d90u, "GAME.EXE");
    uint32 p, x, y, total = 0, offset, phase;
    sint32 floor, angle, drag, wave, amplitude;
    int i, above = 0;
    if (r_u32(0x80091bf8) == 2)
    {
        for (i = 0; i < 32; ++i)
        {
            p = 0x800b9f40 + 8u * i;
            w_u16(p, (uint16)((ff_rand_8006D348() & 4095) + 12288));
            w_u16(p + 2, 0);
            w_u16(p + 4, 0);
            w_u16(p + 6, 0);
        }
        w_u32(object + 8, 0xf4000000);
        w_u16(object + 26, 0);
        w_u32(0x80091bf8, 3);
    }
    if (!r_u32(0x80091bf8))
        return 0;
    x = (uint16)r_s16(object + 6);
    y = (uint16)r_s16(object + 10);
    floor = r_s16(0x800b4346 + 244u * (uint32)(sint32)r_s16(object + 24));
    for (i = 0; i < 32; ++i)
    {
        p = 0x800b9f40 + 8u * i;
        if (!r_s16(object + 26))
        {
            w_u16(p, (uint16)((uint16)r_s16(p) + (uint16)r_s16(p + 2)));
            if ((sint16)y < floor)
            {
                w_u16(p + 2, 0);
                w_u16(p, 0);
            }
            phase = (uint16)r_s16(p);
            angle = (sint16)phase;
            if (angle < 0)
                angle = -angle;
            drag = (r_s16(p + 2) * (32768 - angle)) / 262144;
            wave = (96 * r_s16(0x800802d0 + 2 * (phase >> 5))) / 8192;
            w_u16(p + 2, (uint16)((uint16)r_s16(p + 2) - (uint32)(drag + wave)));
            amplitude = (sint32)((sint32)total < 0 ? total + 196608u : 196608u - total);
            if (amplitude <= 0)
                w_u16(p + 2, 0);
            else
                w_u16(p + 2, (uint16)((sint32)((uint32)(sint32)r_s16(p + 2) * (uint32)amplitude) / 196608));
            total += (uint32)(sint32)r_s16(p);
            offset = ((uint32)((sint32)total >> 4)) & 0xffe;
            x += (uint32)((3 * r_s16(0x800802d0 + offset)) >> 10);
            y += (uint32)((3 * r_s16(0x800806d0 + offset)) >> 10);
            w_u16(p + 4, (uint16)x);
            w_u16(p + 6, (uint16)y);
        }
        else if (r_s16(object + 26) == 1)
            w_u16(p, 0);
        else
        {
            w_u16(p + 6, (uint16)((uint16)r_s16(p + 6) - (uint16)r_s16(p)));
            w_u16(p, (uint16)((uint16)r_s16(p) + 4));
            if (r_s16(p) >= 65)
                w_u16(p, 64);
            angle = r_s16(object + 10);
            if (r_s16(p + 6) < angle)
                w_u16(p + 6, (uint16)angle);
            else
                ++above;
        }
    }
    phase = (uint32)(sint32)r_s16(object + 26);
    if (phase == 2 && !above)
    {
        w_u32(object, 0xffffffff);
        phase = (uint32)(sint32)r_s16(object + 26);
    }
    if (phase == 1)
        w_u16(object + 26, 2);
    return 2;
}

GDB_CALL uint32 FUN_8001DA84(void)
{
    FUNCTION_MARKER(0x8001da84u, "GAME.EXE");
    static const uint32 limits[3] = {0xed180000, 0xfa610000, 0x07c10000};
    uint32 state = r_u32(0x80093d60), p, next, a, b, result, value;
    sint32 first = -2, second = -2, z;
    int i, sound = 0;
    if (state < 3)
    {
        if (r_u8(0x80093dd9))
            w_u16(r_u32(0x800941dc + 4 * state) + 12, 0);
        if (FUN_800473B0((sint32)limits[state]))
        {
            next = r_u32(0x80093d60) + 1;
            p = r_u32(0x800941d0 + 4 * state);
            w_u8(0x80093dd9, 0);
            w_u32(0x80093d60, next);
            w_u16(p + 12, 3);
        }
    }
    for (i = 0; i < 8; ++i)
        if (r_s16(0x800b435a + 244 * i) > 0)
        {
            z = r_s16(0x800b434a + 244 * i);
            if ((uint32)z + 4862u < 894u || (uint32)z - 1986u < 958u)
                first = 2;
            if ((uint32)z + 1438u < 926u)
                second = 2;
        }
    value = r_u32(0x80094674);
    if ((!value && first > 0) || (value == 64 && first < 0))
        sound = 2;
    value = r_u32(0x80094674) + (uint32)first;
    w_u32(0x80094674, value);
    if ((sint32)value < 0)
        w_u32(0x80094674, 0);
    if ((sint32)r_u32(0x80094674) >= 65)
        w_u32(0x80094674, 64);
    value = r_u32(0x80094794);
    if ((!value && second > 0) || (value == 64 && second < 0))
        sound = 1;
    value = r_u32(0x80094794) + (uint32)second;
    w_u32(0x80094794, value);
    if ((sint32)value < 0)
        w_u32(0x80094794, 0);
    if ((sint32)r_u32(0x80094794) >= 65)
        w_u32(0x80094794, 64);
    if (sound)
        FUN_80056670(124, 255, 5888);
    FUN_80058374();
    value = r_u32(0x80094674);
    a = r_u32(0x800941e8);
    b = r_u32(0x800941f0);
    next = r_u32(0x80094794);
    w_u16(b + 12, value == 64 ? 0 : 3);
    w_u16(a + 12, value == 64 ? 0 : 3);
    result = r_u32(0x800941ec);
    w_u16(result + 12, next == 64 ? 0 : 3);
    return result;
}

GDB_CALL uint32 FUN_8001DDBC(void)
{
    FUNCTION_MARKER(0x8001ddbcu, "GAME.EXE");
    static const uint32 xs[6] = {0xfd800000, 0xfe800000, 0xff800000, 0x00800000, 0x01800000, 0x02800000};
    uint32 kind, x, y, r0, r1, result;
    int row, col, i;
    if (r_u32(0x80093dd0) == 210)
    {
        w_u16(0x8009463c, (uint16)FUN_80056670(130, 255, 6144));
        w_u32(0x800941a8, 16);
        w_u32(0x80094190, 4);
        FUN_80056670(19, 255, 6144);
        FUN_80056670(127, 255, 6400);
        w_u32(0x800b89f0, r_u32(0x800b89f0) + 1);
        for (row = -168; row >= -840; row -= 336)
        {
            y = (uint32)row << 16;
            for (col = 0; col < 6; ++col)
            {
                kind = (ff_rand_8006D348() & 3) + 679;
                FUN_800168BC(kind, xs[col], y, 0x0dc80000, 0);
            }
        }
        for (i = 0; i < 96; ++i)
        {
            r0 = ff_rand_8006D348();
            r1 = ff_rand_8006D348();
            x = (r0 & 127) + (r1 & 63) - 96;
            r0 = ff_rand_8006D348();
            r1 = ff_rand_8006D348();
            y = (0u - (r0 & 511) - (r1 & 255)) << 16;
            FUN_80031968(x << 19, y, 0x0dc80000, x & 2047);
        }
        for (i = 0; i < 16; ++i)
        {
            kind = (uint32)((sint32)ff_rand_8006D348() % 3 + 675);
            x = ((ff_rand_8006D348() & 31) - 16) << 19;
            r0 = ff_rand_8006D348();
            FUN_80016654(kind, x, 0xff800000, ((r0 & 15) + 384) << 19);
        }
        for (i = 0; i < 16; ++i)
        {
            r0 = ff_rand_8006D348();
            r1 = ff_rand_8006D348();
            x = (r0 & 127) + (r1 & 63) - 96;
            kind = (uint32)((sint32)ff_rand_8006D348() % 3 + 675);
            y = (0u - (ff_rand_8006D348() & 511)) << 16;
            FUN_80016654(kind, x << 19, y, 0x08000000);
        }
    }
    if ((sint32)r_u32(0x80093dd0) < 211)
        return 1;
    result = ff_rand_8006D348() & 31;
    if (result)
        return result;
    kind = (uint32)((sint32)ff_rand_8006D348() % 3 + 675);
    r0 = ff_rand_8006D348();
    r1 = ff_rand_8006D348();
    x = ((r0 & 127) + (r1 & 63) - 96) << 19;
    y = (0u - (ff_rand_8006D348() & 511)) << 16;
    FUN_80016654(kind, x, y, 0x08000000);
    kind = (ff_rand_8006D348() & 1) + 128;
    result = (ff_rand_8006D348() & 511) + 5888;
    return (uint32)FUN_80056670(kind, 255, result);
}

GDB_CALL uint32 FUN_8002E444(uint32 object)
{
    FUNCTION_MARKER(0x8002e444u, "GAME.EXE");
    sint32 state = r_s16(object + 26);
    uint32 x = 0, volume = 0, result, kind;
    int i;
    if (state == 1)
    {
        if (FUN_800473B0((sint32)0xf9000000u))
            w_u16(object + 26, 2);
        return 2;
    }
    if (!state)
        return 2;
    switch (state)
    {
        case 2:
            x = 0xfe000000;
            volume = 128;
            break;
        case 8:
            x = 0xff000000;
            volume = 148;
            break;
        case 16:
            x = 0;
            volume = 168;
            break;
        case 24:
            x = 0x01000000;
            volume = 148;
            break;
        case 32:
            x = 0x02000000;
            volume = 128;
            break;
    }
    if (volume)
    {
        FUN_80056670(19, volume, 6144);
        FUN_80031C50(893, x, 0xff000000, 0xfdd80000);
        for (i = 0; i < 2; ++i)
        {
            kind = (ff_rand_8006D348() & 3) + 902;
            FUN_80031CD8(kind, x, 0xff800000, 0xff000000);
        }
        w_u32(0x800941a8, 8);
        w_u32(0x80094190, 4);
    }
    else if (state == 40)
    {
        FUN_80056670(19, 255, 6144);
        FUN_80031C50(892, r_u32(object + 4), 0xff000000, r_u32(object + 12));
        FUN_80031C50(603, r_u32(object + 4), 0, r_u32(object + 12));
        w_u32(0x800941a8, 16);
        w_u32(0x80094190, 8);
        for (i = 0; i < 48; ++i)
        {
            kind = (ff_rand_8006D348() & 3) + 902;
            FUN_80031CD8(kind, r_u32(object + 4), r_u32(object + 8), r_u32(object + 12));
        }
        /* IDA omits the fifth argument; the last two objects face 0x8000. */
        FUN_80031EB4(528, 0xfe200000, 0, 0xfe380000, 0);
        FUN_80031EB4(528, 0x02200000, 0, 0xfe380000, 0);
        FUN_80031EB4(528, 0xfe200000, 0, 0xffa80000, 32768);
        FUN_80031EB4(528, 0x02200000, 0, 0xffa80000, 32768);
        w_u8(0x80093dd9, 0);
        w_u32(object, 270);
    }
    result = (uint32)(uint16)r_s16(object + 26) + 1;
    w_u16(object + 26, (uint16)result);
    return result;
}

GDB_CALL uint32 FUN_8002C69C(uint32 object)
{
    FUNCTION_MARKER(0x8002c69cu, "GAME.EXE");
    uint32 volume, pan, tick, yaw, pitch, roll, delta, offset, value;
    sint32 angle, target, speed;
    if (r_s16(object + 30))
    {
        ff_spatial_sound_80046884(object + 4, &volume, &pan);
        volume = FUN_80042820(volume, 160);
        FUN_800568C8(r_s16(object + 30) - 1, (sint32)volume, (sint32)pan);
        FUN_80056840(r_s16(object + 30) - 1, 2 * r_s16(object + 24) + 6144);
    }
    else
        w_u16(object + 30, (uint16)((uint32)FUN_80056670(214, 0, 6144) + 1));
    tick = r_u32(0x80093dd0);
    if (tick - 420u < 210u)
    {
        yaw = (uint32)(uint16)r_s16(object + 16) << 5;
        angle = ((sint16)(16384u - yaw)) >> 5;
        target = r_s16(object + 28);
        target += (angle - target) >> 4;
        w_u16(object + 28, (uint16)target);
        yaw = ((yaw + (uint32)target) << 16) >> 21;
        w_u16(object + 16, (uint16)yaw);
        if (yaw - 481u < 63u)
        {
            value = r_u32(object + 4);
            w_u32(object + 4, value + (uint32)((sint32)(0xfe800000u - value) >> 5));
            target = -((sint32)(0x13780000u - r_u32(object + 12)) >> 13);
            angle = r_s16(object + 20);
            angle += (target - angle) >> 4;
            w_u16(object + 20, (uint16)angle);
            w_u32(object + 12, r_u32(object + 12) - (uint32)((sint32)((uint32)angle << 16) >> 8));
            speed = r_s16(object + 20);
            if (speed < 0)
                speed = -speed;
            w_u16(object + 24, (uint16)(speed >> 6));
        }
    }
    else
    {
        if ((sint32)tick < 2)
        {
            w_u32(object + 4, 0x30000000);
            w_u32(object + 12, 0x60000000);
            w_u16(0x80093dec, 0xf000);
            w_u16(0x80093dee, 0x3000);
            tick = r_u32(0x80093dd0);
        }
        if (tick == 120)
        {
            w_u16(0x80093dec, 0xf400);
            w_u16(0x80093dee, 0x3000);
            tick = r_u32(0x80093dd0);
        }
        if (tick == 240)
        {
            w_u16(0x80093dec, 0xff00);
            w_u16(0x80093dee, 0x1400);
        }
        if (r_u32(0x80093dd0) == 630)
        {
            w_u16(0x80093dec, 0xa000);
            w_u16(0x80093dee, 0x6000);
        }
        yaw = (uint16)r_s16(object + 16);
        pitch = (uint16)r_s16(object + 18);
        roll = (uint16)r_s16(object + 20);
        delta = (uint32)FUN_8001614C(r_s16(object + 6), r_s16(object + 14), r_s16(0x80093dec), r_s16(0x80093dee)) - (yaw << 5);
        angle = (sint16)delta;
        if (angle > 8192)
            angle = 8192;
        if (angle < -8192)
            angle = -8192;
        if (r_u32(0x80093dd0) - 361u < 239u)
            angle = 0;
        roll -= (uint32)(((sint16)roll + angle) >> 4);
        yaw = (yaw - (uint32)((sint16)roll >> 10)) & 2047;
        target = (sint32)(r_u32(0x800941a0) - 1024u);
        if (target > 1024)
            target = 1024;
        if (r_u32(0x80093dd0) - 270u >= 360u)
            target = (sint32)((uint32)target << 1);
        pitch += (uint32)((sint32)((uint32)target - (uint32)(sint32)(sint16)pitch) >> 5);
        speed = r_s16(0x800802d0 + ((pitch >> 4) & 0xffe)) >> 5;
        w_u16(object + 24, (uint16)speed);
        offset = yaw * 2;
        w_u32(object + 4, r_u32(object + 4) + (uint32)(speed * r_s16(0x800802d0 + offset)));
        w_u32(object + 12, r_u32(object + 12) - (uint32)(speed * r_s16(0x800806d0 + offset)));
        w_u16(object + 16, (uint16)yaw);
        w_u16(object + 18, (uint16)pitch);
        w_u16(object + 20, (uint16)roll);
    }
    tick = r_u32(0x80093dd0);
    w_u32(object + 8, ((uint32)(sint32)r_s16(0x800802d0 + ((tick << 4) & 0xff0)) << 4) + 0xfe900000u);
    if (tick == 1050)
    {
        w_u32(object, 55);
        return (uint32)FUN_800567F8(r_s16(object + 30) - 1);
    }
    return 55;
}

/* Shared falling-panel prefix, instruction-audited independently at
 * 2CEF8/2D2A4. Rotation seed field and movement angle differ. */
static void panel_fall_step(uint32 object, uint32 spin_field, int sideways)
{
    uint32 offset, random;
    if (!r_s16(object + 26))
    {
        w_u16(object + 26, 1);
        w_u16(object + spin_field, (uint16)((ff_rand_8006D348() & 4095u) + 135));
        w_u16(object + 28, (uint16)((ff_rand_8006D348() & 4095u) + 135));
        w_u16(object + 16, (uint16)((uint16)r_s16(object + 16) - 2048));
        random = ff_rand_8006D348() & 4095;
        w_u16(object + 16, (uint16)((uint16)r_s16(object + 16) + random));
        w_u16(object + 32, (uint16)((ff_rand_8006D348() & 15) + 32));
        w_u16(object + 24, (uint16)((ff_rand_8006D348() & 63) + 64));
    }
    if (sideways)
    {
        offset = (uint32)((r_s16(object + 16) >> 5) + 512) & 2047;
        if ((sint32)r_u32(object + 4) < 0)
            offset ^= 1024;
    }
    else
        offset = (uint16)r_s16(object + 16) >> 5;
    offset *= 2;
    w_u32(object + 4, r_u32(object + 4) + (uint32)((r_s16(object + 24) * r_s16(0x800802d0 + offset)) >> 1));
    w_u32(object + 12, r_u32(object + 12) - (uint32)((r_s16(object + 24) * r_s16(0x800806d0 + offset)) >> 1));
    w_u16(object + 24, (uint16)((uint16)r_s16(object + 24) - 2));
    if (r_s16(object + 24) < 0)
        w_u16(object + 24, 0);
    w_u32(object + 8, r_u32(object + 8) - ((uint32)(sint32)r_s16(object + 32) << 15));
    w_u16(object + 32, (uint16)((uint16)r_s16(object + 32) - 8));
    FUN_80010040();
    FUN_80011D20();
    FUN_800101CC(32768);
    FUN_8001033C(r_s16(object + 16));
    FUN_800101CC(r_s16(object + 18));
    FUN_800104A0(r_s16(object + 20));
}

GDB_CALL sint32 FUN_8002CEF8(uint32 object)
{
    FUNCTION_MARKER(0x8002cef8u, "GAME.EXE");
    uint32 point[3];
    sint32 height = 0, angle, timer;
    int i;
    panel_fall_step(object, 30, 0);
    for (i = 0; i < 4; ++i)
    {
        FUN_800614F4(i < 2 ? 0x00600000 : (sint32)0xffa00000u, i & 1 ? 0x005c0000 : (sint32)0xffa40000u, 0, point);
        if (height < (sint32)point[1])
            height = (sint32)point[1];
    }
    FUN_80010028();
    if ((sint32)r_u32(object + 8) >= (sint32)(0u - (uint32)height))
    {
        w_u32(object + 8, 0u - (uint32)height);
        if (r_s16(object + 28))
            w_u16(object + 22, 65535);
        w_u16(object + 32, 0);
        w_u16(object + 30, 0);
        w_u16(object + 28, 0);
        angle = (sint16)(16384u - (uint16)r_s16(object + 18));
        w_u16(object + 18, (uint16)((uint16)r_s16(object + 18) + (angle >> 1)));
        angle = r_s16(object + 20);
        w_u16(object + 20, (uint16)(angle + ((-angle) >> 1)));
        w_u16(object + 32, (uint16)((-r_s16(object + 32)) >> 1));
    }
    w_u16(object + 20, (uint16)((uint16)r_s16(object + 20) + (uint16)r_s16(object + 30)));
    w_u16(object + 18, (uint16)((uint16)r_s16(object + 18) + (uint16)r_s16(object + 28)));
    timer = r_s16(object + 22);
    if (timer < 0)
    {
        timer = (sint16)(timer - 1);
        w_u16(object + 22, (uint16)timer);
        if (timer < -60)
            w_u32(object, 0xffffffff);
        return -1;
    }
    return timer;
}

GDB_CALL uint32 FUN_8002D2A4(uint32 object)
{
    FUNCTION_MARKER(0x8002d2a4u, "GAME.EXE");
    uint32 point[3], result;
    sint32 height = 0, angle, extent_z = 0x600000, extent_y = 0x5c0000;
    int i;
    panel_fall_step(object, 22, 1);
    if ((sint32)r_u32(object) >= 350)
    {
        extent_z = 0x800000;
        extent_y = 0x500000;
    }
    for (i = 0; i < 4; ++i)
    {
        FUN_800614F4(0, i & 1 ? extent_y : -extent_y, i < 2 ? extent_z : -extent_z, point);
        if (height < (sint32)point[1])
            height = (sint32)point[1];
    }
    FUN_80010028();
    if ((sint32)r_u32(object + 8) >= (sint32)(0u - (uint32)height))
    {
        w_u32(object + 8, 0u - (uint32)height);
        w_u16(object + 32, 0);
        w_u16(object + 22, 0);
        w_u16(object + 28, 0);
        angle = (sint16)(16384u - (uint16)r_s16(object + 20));
        w_u16(object + 20, (uint16)((uint16)r_s16(object + 20) + (angle >> 1)));
        angle = r_s16(object + 18);
        w_u16(object + 18, (uint16)(angle + ((-angle) >> 1)));
        w_u16(object + 32, (uint16)((-r_s16(object + 32)) >> 1));
    }
    w_u16(object + 20, (uint16)((uint16)r_s16(object + 20) + (uint16)r_s16(object + 22)));
    result = (uint32)(uint16)r_s16(object + 18) + (uint16)r_s16(object + 28);
    w_u16(object + 18, (uint16)result);
    return result;
}

GDB_CALL uint32 FUN_80043680(uint32 object)
{
    FUNCTION_MARKER(0x80043680u, "GAME.EXE");
    uint32 offset, point[3], result;
    sint32 height = 0, angle;
    int i;
    if (!r_s16(object + 26))
    {
        w_u16(object + 26, 1);
        w_u16(object + 22, (uint16)((ff_rand_8006D348() & 4095u) + 135u));
        w_u16(object + 28, (uint16)((ff_rand_8006D348() & 4095u) + 135u));
        w_u16(object + 16, (uint16)((uint16)r_s16(object + 16) - 2048u));
        result = ff_rand_8006D348() & 4095u;
        w_u16(object + 16, (uint16)((uint16)r_s16(object + 16) + result));
        w_u16(object + 32, (uint16)((ff_rand_8006D348() & 15u) + 32u));
        w_u16(object + 24, (uint16)((ff_rand_8006D348() & 63u) + 64u));
    }
    offset = (uint32)((r_s16(object + 16) >> 5) + 512) & 2047u;
    if ((sint32)r_u32(object + 4) < 0)
        offset ^= 1024u;
    offset *= 2u;
    w_u32(object + 4, r_u32(object + 4) + (uint32)((r_s16(object + 24) * r_s16(0x800802d0 + offset)) >> 1));
    w_u32(object + 12, r_u32(object + 12) - (uint32)((r_s16(object + 24) * r_s16(0x800806d0 + offset)) >> 1));
    w_u16(object + 24, (uint16)((uint16)r_s16(object + 24) - 2u));
    if (r_s16(object + 24) < 0)
        w_u16(object + 24, 0);
    w_u32(object + 8, r_u32(object + 8) - ((uint32)(sint32)r_s16(object + 32) << 15));
    w_u16(object + 32, (uint16)((uint16)r_s16(object + 32) - 8u));
    FUN_80010040();
    FUN_80011D20();
    FUN_800101CC(32768);
    FUN_8001033C(r_s16(object + 16));
    FUN_800101CC(r_s16(object + 18));
    FUN_800104A0(r_s16(object + 20));
    for (i = 0; i < 4; i++)
    {
        FUN_800614F4(0, i & 1 ? 0x005c0000u : 0xffa40000u, i < 2 ? 0x00600000u : 0xffa00000u, point);
        if (height < (sint32)point[1])
            height = (sint32)point[1];
    }
    FUN_80010028();
    if ((sint32)r_u32(object + 8) >= (sint32)(0u - (uint32)height))
    {
        w_u32(object + 8, 0u - (uint32)height);
        w_u16(object + 32, 0);
        w_u16(object + 22, 0);
        w_u16(object + 28, 0);
        angle = (sint16)(16384u - (uint16)r_s16(object + 20));
        w_u16(object + 20, (uint16)((uint16)r_s16(object + 20) + (angle >> 1)));
        angle = r_s16(object + 18);
        w_u16(object + 18, (uint16)(angle + ((-angle) >> 1)));
        w_u16(object + 32, (uint16)((-r_s16(object + 32)) >> 1));
    }
    w_u16(object + 20, (uint16)((uint16)r_s16(object + 20) + (uint16)r_s16(object + 22)));
    result = (uint32)(uint16)r_s16(object + 18) + (uint16)r_s16(object + 28);
    w_u16(object + 18, (uint16)result);
    return result;
}

GDB_CALL sint32 FUN_8003F088(uint32 object)
{
    FUNCTION_MARKER(0x8003f088u, "GAME.EXE");
    uint32 offset, z, point[3], y, variant, region;
    sint32 turn, count;
    int i, side;
    if (r_s16(object + 26))
    {
        turn = r_s16(object + 28);
        w_u16(object + 16, (uint16)((uint16)r_s16(object + 16) + (uint16)r_s16(object + 28)));
        if (turn < 100)
            w_u16(object + 28, (uint16)(turn + 25));
        offset = ((uint16)r_s16(object + 16) >> 5) * 2u;
        w_u32(object + 4, r_u32(object + 4) + (uint32)(r_s16(object + 24) * r_s16(0x800802d0 + offset)));
        offset = ((uint32)((r_s16(object + 16) >> 5) + 512) & 2047u) * 2u;
        z = r_u32(object + 12) - (uint32)(r_s16(object + 24) * r_s16(0x800802d0 + offset));
        w_u32(object + 12, z);
        if ((sint32)z > (sint32)0xede00000u)
        {
            if (r_s16(object + 24) < 120)
                w_u16(object + 24, (uint16)(r_s16(object + 24) + 4));
        }
        else
        {
            w_u16(object + 24, (uint16)((uint16)r_s16(object + 24) - 5u));
            if (r_s16(object + 24) < 0)
            {
                w_u16(object + 24, 0);
                w_u16(object + 26, 0);
                w_u8(0x80093dd9, 0);
                FUN_80010040();
                FUN_80011D20();
                FUN_800101CC(32768);
                FUN_8001033C(r_s16(object + 16));
                FUN_800614F4(0, 0, 0xfebb0000, point);
                FUN_80010028();
            }
            w_u16(object + 16, (uint16)((uint16)r_s16(object + 16) + 380u));
            FUN_80010040();
            for (i = 0; i < 8; i++)
                for (side = 0; side < 2; side++)
                {
                    FUN_80011D20();
                    FUN_800101CC(32768);
                    FUN_8001033C(r_s16(object + 16));
                    FUN_800614F4(side ? 0xff380000u : 0x00c80000u, 0, 0xfebb0000, point);
                    y = (0u - (ff_rand_8006D348() & 63u)) << 16;
                    variant = ff_rand_8006D348() & 3u;
                    FUN_80031B20(871, r_u32(object + 4) + point[0], y, r_u32(object + 12) + point[2], variant, 0, 0);
                }
            FUN_80010028();
        }
        w_u32(0x80093df4, (uint32)(sint32)r_s16(object + 6));
        w_u32(0x80093df8, (uint32)(sint32)r_s16(object + 14));
    }
    if (!r_s16(object + 24))
        return 0;
    count = (sint32)r_u32(0x8009479c);
    region = 0x8009ec70;
    for (i = 0; i < count; i++, region += 24u)
        if (r_u32(region + 20) == object)
            break;
    return FUN_80029AFC(region, object);
}

GDB_CALL uint32 FUN_800403DC(uint32 object)
{
    FUNCTION_MARKER(0x800403dcu, "GAME.EXE");
    uint32 volume, pan, offset, result;
    sint32 mode, phase, angle, speed;
    if (r_s16(object + 22))
    {
        ff_spatial_sound_80046884(object + 4, &volume, &pan);
        FUN_800568C8(r_s16(object + 22) - 1, volume, pan);
    }
    else
        w_u16(object + 22, (uint16)(FUN_80056670(243, 255, 6144) + 1u));
    mode = r_s16(object + 30);
    if (mode == 1)
    {
        angle = (r_s16(object + 16) + 24576) >> 5;
        w_u32(object + 8, 0xfa240000);
        speed = r_s16(0x800802d0 + 2u * ((uint32)angle & 2047u));
        w_u32(object + 4, r_u32(0x800b4340) + (uint32)(25000 * r_s16(0x800802d0 + 2u * ((uint32)(angle + 512) & 2047u))));
        w_u16(object + 20, 63488);
        w_u16(object + 16, (uint16)((uint16)r_s16(object + 16) + 64u));
        w_u32(object + 12, r_u32(0x800b4348) + (uint32)(25000 * speed));
    }
    else if (mode == 2)
    {
        w_u32(object + 8, 0xfa240000);
        w_u32(object + 4, r_u32(object + 4) + 0x00780000u);
        w_u16(object + 18, 1024);
        w_u16(object + 16, 16384);
        w_u32(object + 12, r_u32(0x800b4348) + 0x0fa00000u);
    }
    else
    {
        phase = r_s16(object + 26);
        w_u32(object + 8, 0xf4480000);
        w_u16(object + 24, 200);
        if (phase < 2)
        {
            if (r_s16(object + 20) < 0)
                w_u16(object + 20, (uint16)(r_s16(object + 20) + 1024));
            if (!r_s16(object + 26))
            {
                if ((sint32)r_u32(object + 12) < (sint32)0xf6a00000u)
                    w_u16(object + 26, 2);
            }
            else if (r_s16(object + 26) == 1 && (sint32)r_u32(object + 12) > 0x3e800000)
                w_u16(object + 26, 2);
        }
        else if (phase == 2)
        {
            angle = r_s16(object + 20);
            w_u16(object + 16, (uint16)((uint16)r_s16(object + 16) + 256u));
            if (angle >= -4095)
                w_u16(object + 20, (uint16)(angle - 320));
            offset = (uint16)r_s16(object + 16) >> 5;
            if (!offset)
                w_u16(object + 26, 0);
            if (offset == 1024)
                w_u16(object + 26, 1);
        }
        angle = r_s16(object + 16) >> 5;
        speed = r_s16(object + 24);
        w_u32(object + 4, r_u32(object + 4) + (uint32)(speed * r_s16(0x800802d0 + 2u * ((uint32)angle & 2047u))));
        w_u32(object + 12, r_u32(object + 12) - (uint32)(speed * r_s16(0x800802d0 + 2u * ((uint32)(angle + 512) & 2047u))));
    }
    result = (uint32)(uint16)r_s16(object + 28) + 256u;
    w_u16(object + 28, (uint16)result);
    return result;
}

GDB_CALL sint32 FUN_8002EFDC(uint32 object)
{
    FUNCTION_MARKER(0x8002efdcu, "GAME.EXE");
    uint32 yaw = (uint16)r_s16(object + 16), pitch = (uint16)r_s16(object + 18), roll = (uint16)r_s16(object + 20), volume, pan, distance, offset, y, result;
    sint32 delta, speed, slot;
    if (r_s16(object + 30))
    {
        ff_spatial_sound_80046884(object + 4, &volume, &pan);
        FUN_800568C8(r_s16(object + 30) - 1, volume, pan);
        FUN_80056840(r_s16(object + 30) - 1, 6144u - (uint32)((sint32)r_u32(0x800941a0) >> 2));
    }
    else
        w_u16(object + 30, (uint16)(FUN_80056670(6, 255, 6144) + 1u));
    delta = (sint16)((uint32)FUN_8001614C(r_s16(object + 6), r_s16(object + 14), r_s16(object + 22), r_s16(object + 24)) - yaw);
    if (delta > 8192)
        delta = 8192;
    else if (delta < -8192)
        delta = -8192;
    roll -= (uint32)(((sint16)roll + delta) >> 4);
    yaw -= (uint32)((sint16)roll >> 4);
    distance = r_u32(0x800941a0);
    offset = (((uint32)(sint32)r_s16(object + 32) + r_u32(0x80093dd0)) << 5) & 0xfe0u;
    y = (0u - distance) * 17408u + (uint32)((sint32)((uint32)(sint32)r_s16(0x800802d0 + offset) * distance) >> 4);
    w_u32(object + 8, y);
    if ((sint32)y > (sint32)0xff580000u)
        w_u32(object + 8, 0xff580000);
    distance = r_u32(0x800941a0) - 1024u;
    distance = (sint32)distance > 1024 ? 2048u : distance << 1;
    pitch += (uint32)((sint32)(distance - (uint32)(sint32)(sint16)pitch) >> 5);
    speed = r_s16(0x800802d0 + ((pitch >> 1) & 0xffeu)) >> 8;
    offset = (yaw >> 4) & 0xffeu;
    w_u32(object + 4, r_u32(object + 4) + (uint32)(speed * r_s16(0x800802d0 + offset)));
    w_u32(object + 12, r_u32(object + 12) - (uint32)(speed * r_s16(0x800806d0 + offset)));
    w_u16(object + 16, (uint16)yaw);
    w_u16(object + 18, (uint16)pitch);
    w_u16(object + 20, (uint16)roll);
    if ((sint32)r_u32(0x800941a0) >= 768)
        return 0;
    w_u32(object, r_u32(object) + 1u);
    FUN_80056670(199, 255, 6144);
    slot = FUN_8004AF18(47, r_u32(object + 4), r_u32(object + 12), ((r_s16(object + 16) >> 5) - 512) & 2047, 0, 0, 0);
    FUN_80049DB0(slot, 1);
    FUN_8001AEF4(slot);
    w_u16(object + 34, (uint16)slot);
    result = r_u32(0x80093dbc) + 1u;
    w_u32(0x80093dbc, result);
    w_u8(0x80093dd9, 0);
    return (sint32)result;
}

GDB_CALL sint32 FUN_8002FA90(uint32 object)
{
    FUNCTION_MARKER(0x8002fa90u, "GAME.EXE");
    uint32 target, offset, x, y, z, dx, dy, dz, result;
    sint32 slot, speed, hit;
    int i;
    FUN_80016654(575, r_u32(object + 4), r_u32(object + 8), r_u32(object + 12));
    slot = r_s16(object + 26);
    if (slot != -1)
    {
        target = 0x800b4318 + 244u * slot;
        w_u16(object + 16, (uint16)FUN_8001614C(r_s16(object + 6), r_s16(object + 14), r_s16(target + 42), r_s16(target + 50)));
    }
    offset = ((uint16)r_s16(object + 16) >> 5) * 2u;
    speed = r_s16(object + 24);
    dx = (uint32)((speed * r_s16(0x800802d0 + offset)) >> 2);
    dz = (uint32)((speed * r_s16(0x800806d0 + offset)) >> 2);
    dy = (uint32)((speed * r_s16(0x800802d0 + ((uint16)r_s16(object + 18) >> 5) * 2u)) >> 2);
    x = r_u32(object + 4);
    y = r_u32(object + 8);
    z = r_u32(object + 12);
    for (i = 0; i < 4; i++)
    {
        if (r_s16(object + 24) > 100)
        {
            hit = FUN_800199AC(x, z);
            if (hit >= 0)
            {
                FUN_80056670(19, 255, 6144);
                FUN_80031C50(893, x, r_u32(object + 8), z);
                w_u32(0x800941a8, 16);
                w_u32(0x80094190, 8);
                /* 31C50 saves caller S3=x at entry SP-12, read by 1F004. */
                FUN_8001F004(hit, object, x);
                FUN_80031F64(x, z, object);
                w_u32(object, 0xffffffffu);
                return -1;
            }
            if (FUN_80019BA8(x, z, object) >= 0 || (sint32)r_u32(object + 8) > 0)
            {
                FUN_80056670(19, 255, 6144);
                FUN_80031C50(893, x, r_u32(object + 8), z);
                w_u32(0x800941a8, 16);
                w_u32(0x80094190, 8);
                FUN_80031F64(x, z, object);
                w_u32(object, 0xffffffffu);
                result = r_u32(object + 8);
                if ((sint32)result > 0)
                    return FUN_80031C50(603, r_u32(object + 4), 0, r_u32(object + 12));
                return (sint32)result;
            }
        }
        x += dx;
        y -= dy;
        z -= dz;
        w_u32(object + 4, x);
        w_u32(object + 8, y);
        w_u32(object + 12, z);
        if (i & 1)
            FUN_80016654(575, r_u32(object + 4), r_u32(object + 8), z);
    }
    w_u16(object + 32, (uint16)((uint16)r_s16(object + 32) - 4u));
    w_u32(object + 8, r_u32(object + 8) - (uint32)((sint32)((uint32)(uint16)r_s16(object + 32) << 16) >> 1));
    result = (uint32)(uint16)r_s16(object + 24) + 64u;
    w_u16(object + 24, (uint16)result);
    return (sint32)result;
}

GDB_CALL sint32 FUN_8002F2E0(uint32 object)
{
    FUNCTION_MARKER(0x8002f2e0u, "GAME.EXE");
    uint32 volume, pan, yaw, pitch, roll, distance, offset, y, actor, frame;
    sint32 delta, speed;
    ff_spatial_sound_80046884(object + 4, &volume, &pan);
    FUN_800568C8(r_s16(object + 30) - 1, volume, pan);
    yaw = (uint16)r_s16(object + 16);
    pitch = (uint16)r_s16(object + 18);
    roll = (uint16)r_s16(object + 20);
    FUN_8001614C(r_s16(object + 6), r_s16(object + 14), r_s16(object + 22), r_s16(object + 24));
    distance = r_u32(0x800941a0);
    offset = (((uint32)(sint32)r_s16(object + 32) + r_u32(0x80093dd0)) << 5) & 0xfe0u;
    y = (0u - distance) * 17408u + (uint32)((sint32)((uint32)(sint32)r_s16(0x800802d0 + offset) * distance) >> 4);
    w_u32(object + 8, y);
    if ((sint32)y > (sint32)0xff580000u)
        w_u32(object + 8, 0xff580000);
    delta = (sint16)((uint32)FUN_8001614C(r_s16(object + 6), r_s16(object + 14), r_s16(object + 26), r_s16(object + 28)) - yaw);
    if (delta > 8192)
        delta = 8192;
    else if (delta < -8192)
        delta = -8192;
    roll -= (uint32)(((sint16)roll + delta) >> 4);
    yaw -= (uint32)((sint16)roll >> 4);
    distance = r_u32(0x800941a0) - 1024u;
    distance = (sint32)distance > 1024 ? 2048u : distance << 1;
    pitch += (uint32)((sint32)(distance - (uint32)(sint32)(sint16)pitch) >> 5);
    speed = r_s16(0x800802d0 + ((pitch >> 1) & 0xffeu)) >> 8;
    offset = (yaw >> 4) & 0xffeu;
    w_u32(object + 4, r_u32(object + 4) + (uint32)(speed * r_s16(0x800802d0 + offset)));
    w_u32(object + 12, r_u32(object + 12) - (uint32)(speed * r_s16(0x800806d0 + offset)));
    w_u16(object + 16, (uint16)yaw);
    w_u16(object + 18, (uint16)pitch);
    w_u16(object + 20, (uint16)roll);
    if ((sint32)r_u32(0x800941a0) < 1024)
    {
        FUN_800567F8(r_s16(object + 30) - 1);
        w_u16(object + 30, 0);
        w_u32(object, 0xffffffffu);
    }
    actor = 0x800b4318 + 244u * (uint32)(sint32)r_s16(object + 34);
    if (r_u32(actor + 12) != 269)
        return 269;
    frame = r_u32(actor + 4);
    if ((sint32)frame >= 12 || frame == 0xffffffffu)
        return -1;
    w_u32(actor + 40, r_u32(object + 4));
    y = r_u32(object + 12);
    w_u32(actor + 48, y);
    return (sint32)y;
}

GDB_CALL sint32 FUN_8003A700(uint32 source)
{
    FUNCTION_MARKER(0x8003a700u, "GAME.EXE");
    static const uint16 kinds[7] = {848, 850, 849, 848, 847, 846, 846};
    uint32 object;
    int i;
    sint32 result = 0;
    w_u32(0x8008d4a8, (uint32)((sint32)r_u32(0x8008d4a8) >> 1));
    if ((sint32)r_u32(0x80093dd0) >= (sint32)r_u32(0x800941c0))
        return 0;
    w_u32(0x8009403c, 33);
    for (i = 0; i < 7; i++)
    {
        object = 0x800b8d98 + 36u * i;
        w_u32(object, kinds[i]);
        result = FUN_80047A40(source, object, i + 1);
        if ((i == 0 && r_u32(0x80093dd0) == 202) || (i == 3 && r_u32(0x80093dd0) == 298) || (i == 4 && r_u32(0x80093dd0) == 254))
        {
            FUN_80031C50(892, r_u32(object + 4), 0xffc00000, r_u32(object + 12));
            FUN_80031C50(603, r_u32(object + 4), 0, r_u32(object + 12));
            w_u32(0x800941a8, 8);
            w_u32(0x80094190, 4);
        }
    }
    return result;
}

GDB_CALL sint32 FUN_8002BA88(uint32 object, sint32 radius, uint32 retained_word)
{
    FUNCTION_MARKER(0x8002ba88u, "GAME.EXE");
    uint32 point[4], region = 0x8009ec70;
    sint32 i, type, x, z, px, pz, count;
    point[0] = r_u32(object + 4);
    point[1] = r_u32(object + 8);
    point[2] = r_u32(object + 12);
    point[3] = retained_word;
    count = (sint32)r_u32(0x8009479c);
    if (count <= 0)
        return count;
    px = (sint32)point[0] >> 16;
    pz = (sint32)point[2] >> 16;
    for (i = 0; i < (sint32)r_u32(0x8009479c); i++, region += 24u)
    {
        type = r_s16(region + 12);
        if ((type < 3 && type != 1) || (uint32)(uint16)type - 23u < 2u || r_s16(region + 16) < 0)
            continue;
        if (r_s16(region + 10))
        {
            x = r_s16(region + 4);
            z = r_s16(region + 6);
        }
        else
        {
            x = (r_s16(region) + r_s16(region + 4)) >> 1;
            z = (r_s16(region + 2) + r_s16(region + 6)) >> 1;
        }
        if (FUN_800163F4(px, pz, x, z) >= radius)
            continue;
        w_u16(region + 16, 65535);
        ff_collision_dispatch(r_u32(0x80094040 + 4u * (uint32)(sint32)r_s16(region + 12)), region, r_u32(region + 20), point, 0);
    }
    return 0;
}

GDB_CALL sint32 FUN_8002BC1C(uint32 object)
{
    FUNCTION_MARKER(0x8002bc1cu, "GAME.EXE");
    uint32 actor, state, result;
    sint32 timer, i, type, distance;
    FUN_8002B2E4(object);
    timer = r_s16(object + 22);
    if (timer <= 0)
        return timer;
    w_u16(object + 22, (uint16)(timer - 1));
    result = (uint32)(timer - 1) << 16;
    if (result)
        return (sint32)result;
    FUN_80056670(19, 255, 6144);
    FUN_80031C50(892, r_u32(object + 4), r_u32(object + 8), r_u32(object + 12));
    FUN_80031C50(603, r_u32(object + 4), 0, r_u32(object + 12));
    w_u32(0x800941a8, 16);
    w_u32(0x80094190, 4);
    /* 2BA88's fourth point word is caller SP-44: 31C50 -> 30F44 saved
  * RA=80031C80 there; the subsequent leaf memset never changes the stack. */
    FUN_8002BA88(object, 1024, 0x80031c80);
    for (i = 0; i < 8; i++)
    {
        actor = 0x800b4318 + 244u * i;
        type = r_s16(actor + 52);
        if (type == -1 || r_s16(actor + 80) || type == 17)
            continue;
        state = r_u32(actor + 12);
        if (state == 205 || state == 203 || r_s16(actor + 66) <= 0)
            continue;
        distance = FUN_800163F4(r_s16(object + 6), r_s16(object + 14), r_s16(actor + 42), r_s16(actor + 50));
        if (distance >= 1024)
            continue;
        w_u16(actor + 66, (uint16)((uint32)(uint16)r_s16(actor + 66) - 128u + (uint32)(distance >> 4)));
        w_u16(actor + 56, (uint16)((uint32)FUN_8001627C(r_s16(object + 6), r_s16(object + 14), r_s16(actor + 42), r_s16(actor + 50)) ^ 1024u));
        FUN_80017E30(i);
        FUN_80019F00(actor, r_u32(0x800b3be8 + 4u * (uint32)(sint32)r_s16(actor + 52)), 147);
    }
    w_u32(object, 0xffffffffu);
    return -1;
}

GDB_CALL sint32 FUN_800469A4(uint32 object)
{
    FUNCTION_MARKER(0x800469a4u, "GAME.EXE");
    uint32 region = 0x8009ec70, offset, radius, x, z, point[4], result, coordinate, angle, step, kind;
    uint32 count = r_u32(0x8009479c), i = 0;
    sint32 dx, dz, object_x, object_z, region_x, region_z, speed, sine, cosine, owner;
    result = count;
    for (; (sint32)i < (sint32)count; i++, region += 24)
        if (r_u32(region + 20) == object)
            break;
    if ((sint32)count > 0)
        result = r_u32(0x8009479c);
    if (i == result)
        return (sint32)result;
    speed = r_s16(object + 24);
    if (speed <= 0)
        return speed;
    object_x = r_s16(object + 6);
    region_x = r_s16(region);
    dx = object_x - region_x;
    object_z = r_s16(object + 14);
    region_z = r_s16(region + 2);
    dz = object_z - region_z;
    radius = (uint32)FUN_80012B5C((uint32)dx * (uint32)dx + (uint32)dz * (uint32)dz) << 1;
    offset = 2 * (uint32)(sint32)r_s16(object + 32);
    sine = r_s16(0x800802d0 + offset);
    x = r_u32(object + 4);
    x += (uint32)sine * radius;
    cosine = r_s16(0x800806d0 + offset);
    z = r_u32(object + 12);
    z -= (uint32)cosine * radius;
    point[0] = x;
    point[1] = r_u32(object + 8);
    point[2] = z;
    point[3] = 1;
    owner = r_s16(object + 28);
    kind = r_u32(object);
    FUN_8001BFA0(owner, point, kind);
    if (FUN_80019718(x, z))
    {
        result = r_u32(0x80093d04);
        if (result != region)
        {
            w_u16(object + 24, 0);
            return (sint32)result;
        }
    }
    angle = r_u16(object + 16);
    step = r_u16(object + 22);
    w_u16(object + 16, angle + step);
    offset = 2 * (uint32)(sint32)r_s16(object + 32);
    sine = r_s16(0x800802d0 + offset);
    speed = r_s16(object + 24);
    x = r_u32(object + 4);
    w_u32(object + 4, x + (uint32)sine * (uint32)speed);
    offset = 2 * (uint32)(sint32)r_s16(object + 32);
    cosine = r_s16(0x800806d0 + offset);
    speed = r_s16(object + 24);
    z = r_u32(object + 12);
    w_u32(object + 12, z - (uint32)cosine * (uint32)speed);
    step = r_u16(object + 24);
    w_u16(object + 24, step - 1);
    coordinate = r_u16(object + 6);
    w_u16(region + 4, coordinate);
    w_u16(region, coordinate);
    coordinate = r_u16(object + 14);
    w_u16(region + 6, coordinate);
    w_u16(region + 2, coordinate);
    kind = r_u32(object);
    FUN_8003B77C(region, (sint32)kind);
    result = r_u32(0x8009479c) - 1u;
    w_u32(0x8009479c, result);
    return (sint32)result;
}

GDB_CALL sint32 FUN_80028114(void)
{
    FUNCTION_MARKER(0x80028114u, "GAME.EXE");
    uint32 tick, players, z, limit, offset, product, x;
    sint32 result;
    w_u16(0x8009419c, (uint16)((sint32)(0u - r_u32(0x80094004)) >> 12));
    FUN_800476E8();
    if (r_u32(0x80093dd0) == 150)
        FUN_80056670(207, 255, (ff_rand_8006D348() & 255u) + 5632u);
    tick = r_u32(0x80093dd0);
    if (tick - 61u < 179u)
    {
        w_u32(0x80094660, 0);
        w_u32(0x80094664, 0xff000000);
        w_u32(0x80094668, 0x09000000);
        w_u16(0x800941a6, 64512);
    }
    tick = r_u32(0x80093dd0);
    if (tick - 240u < 390u)
    {
        if ((sint32)tick < 361)
        {
            w_u32(0x80094660, 0);
            w_u32(0x80094664, 0xfe000000);
            w_u32(0x80094668, 0x0a000000);
        }
        else
        {
            w_u32(0x80094660, 0xff800000);
            w_u32(0x80094664, 0xfec00000);
            w_u32(0x80094668, 0x0c000000);
            w_u16(0x800941a6, 1024);
        }
    }
    players = r_u32(0x800940b0);
    tick = r_u32(0x80093dd0);
    if ((sint32)tick < 660)
    {
        w_u32(0x800940b0, 0);
        w_u16(0x8009419c, (sint32)tick < 240 ? 0 : 2048);
    }
    else
    {
        z = r_u32(0x80094668);
        if ((sint32)z > 0x09c00000)
            limit = 0x09c00000;
        else
        {
            z = r_u32(0x80094668);
            limit = (sint32)z <= 0x067fffff ? 0x06800000 : z;
        }
        if (limit != z)
        {
            offset = ((uint16)r_s16(0x8009419c) >> 5) * 2u;
            product = (uint32)(sint32)r_s16(0x800802d0 + offset) * (uint32)((sint32)(z - limit) >> 15);
            x = r_u32(0x80094660);
            w_u32(0x80094660, x + product);
            w_u32(0x80094668, limit);
        }
    }
    result = FUN_800474B8();
    w_u32(0x800940b0, players);
    return result;
}

GDB_CALL sint32 FUN_8002E11C(uint32 object)
{
    FUNCTION_MARKER(0x8002e11cu, "GAME.EXE");
    uint32 offset = ((uint16)r_s16(object + 16) >> 5) * 2u, y, kind;
    sint32 bounce, volume, result;
    w_u32(object + 4, r_u32(object + 4) + (uint32)((r_s16(object + 24) * r_s16(0x800802d0 + offset)) >> 1));
    w_u32(object + 12, r_u32(object + 12) - (uint32)((r_s16(object + 24) * r_s16(0x800806d0 + offset)) >> 1));
    w_u16(object + 24, (uint16)((uint16)r_s16(object + 24) - 1u));
    if (r_s16(object + 24) < 0)
        w_u16(object + 24, 0);
    w_u16(object + 32, (uint16)((uint16)r_s16(object + 32) - (r_u32(object) - 512u < 2u ? 16u : 5u)));
    y = r_u32(object + 8) - ((uint32)(sint32)r_s16(object + 32) << 15);
    w_u32(object + 8, y);
    if ((sint32)y > 0)
    {
        w_u32(object + 8, 0);
        bounce = (-r_s16(object + 32)) >> 1;
        w_u16(object + 32, (uint16)bounce);
        if (bounce >= 17)
        {
            w_u16(object + 30, (uint16)((uint16)r_s16(object + 30) + 1u));
            volume = 255 - 64 * r_s16(object + 30);
            if (volume < 0)
                volume = 0;
            kind = r_u32(object);
            if (kind == 577)
                FUN_80056670(29, volume, 7168);
            if (kind == 514)
                FUN_80056670(29, volume, 6144);
            if (kind == 578)
                FUN_80056670(29, volume, 5632);
            if (kind - 512u < 2u)
            {
                FUN_80056670((sint32)ff_rand_8006D348() % 3 + 119, volume, 6144);
                if (kind == 513 && !r_s16(object + 22))
                    w_u16(object + 22, 65535);
            }
        }
    }
    result = r_s16(object + 22);
    if (result < 0)
    {
        w_u16(object + 22, (uint16)(result - 1));
        if (r_s16(object + 22) < -60)
            w_u32(object, 0xffffffffu);
        return -1;
    }
    return result;
}

GDB_CALL uint32 FUN_8004322C(uint32 object)
{
    FUNCTION_MARKER(0x8004322cu, "GAME.EXE");
    uint32 tick, factor, offset, base, x, y, z, volume, pan, result, angle, item;
    sint32 voice, slot;
    if (!r_s16(object + 30))
        w_u16(object + 30, (uint16)(FUN_80056670(145, 0, 6144) + 1u));
    tick = r_u32(0x80093dd0);
    base = (uint32)(sint32)r_s16(object + 22) << 16;
    factor = tick & 256u ? 256u - (tick & 255u) : r_u8(0x80093dd0);
    offset = ((uint32)((r_s16(object + 16) >> 5) + 512) & 2047u) * 2u;
    x = base + factor * ((uint32)(sint32)r_s16(0x800802d0 + offset) << 7);
    w_u32(object + 4, x);
    base = (uint32)(sint32)r_s16(object + 24) << 16;
    offset = ((uint16)r_s16(object + 16) >> 5) * 2u;
    z = base + factor * ((uint32)(sint32)r_s16(0x800802d0 + offset) << 7) + (tick & 256u ? 0x02000000u : 0u);
    w_u32(object + 12, z);
    ff_spatial_sound_80046884(object + 4, &volume, &pan);
    voice = r_s16(object + 30);
    if (voice)
        FUN_800568C8(voice - 1, volume, pan);
    result = r_u32(0x80094198);
    if (!result)
        return result;
    result = r_u32(0x80093dd0) & 15u;
    if (result)
        return result;
    result = r_u32(object + 4) + 0x072fffffu;
    if (result > 0x072ffffeu)
        return result;
    angle = (ff_rand_8006D348() & 8191u) - 4096u;
    x = r_u32(object + 4);
    y = r_u32(object + 8);
    z = r_u32(object + 12);
    slot = FUN_80016748(497, x, y, z, angle);
    if (slot >= 0)
    {
        item = 0x8009a3c8 + 36 * (uint32)slot;
        w_u16(item + 22, 90);
        w_u16(item + 24, 190);
    }
    return 190;
}

GDB_CALL sint32 FUN_8003B5F0(uint32 source)
{
    FUNCTION_MARKER(0x8003b5f0u, "GAME.EXE");
    uint32 tick = r_u32(0x80093dd0), object;
    int i;
    sint32 result = 0;
    if ((sint32)tick < 300)
    {
        w_u32(0x8009403c, 7);
        for (i = 1; i <= 6; i++)
        {
            object = 0x800b89f0 + 36u * i;
            w_u32(object, i <= 2 ? 854 : 452);
            result = FUN_80047A40(source, object, i);
        }
        return result;
    }
    w_u32(0x80094348, r_u32(0x80094364));
    FUN_8004799C((sint32)(tick - 300u));
    w_u32(0x8009403c, 2);
    for (i = 0; i < 18; i++)
        w_u32(0x800b89f0 + 4u * i, 0);
    w_u32(0x800b89f0, 449);
    w_u32(0x800b89f4, 0);
    w_u32(0x800b89fc, 0);
    w_u32(0x800b8a14, 453);
    w_u32(0x800b8a18, 0x00f00000);
    w_u32(0x800b8a1c, 0x00280000);
    w_u32(0x800b8a20, 0);
    w_u16(0x800b8a24, 8192);
    return 8192;
}

GDB_CALL sint32 FUN_80028D08(void)
{
    FUNCTION_MARKER(0x80028d08u, "GAME.EXE");
    sint32 volume = (sint32)(0xf0000000u - r_u32(0x80094008)) >> 18, result, pitch;
    uint32 i, object, wave, y;
    if (volume < 72)
        volume = 72;
    if (volume > 192)
        volume = 192;
    FUN_80056F08(r_u32(0x800944b8), volume);
    pitch = r_s16(0x800802d0 + ((r_u32(0x80093dd0) << 4) & 0xff0u)) >> 9;
    FUN_80056840(r_u32(0x800944b8), pitch + 6144);
    w_u16(0x8009419c, (uint16)((sint32)(0u - r_u32(0x80094004)) >> 13));
    FUN_80028C08();
    if ((sint32)r_u32(0x80094198) > 0)
    {
        if ((sint32)r_u32(0x80094668) < (sint32)0xe2500000u)
            w_u32(0x80094668, 0xe2500000);
        return FUN_800474B8();
    }
    FUN_800476E8();
    wave = (uint32)(sint32)r_s16(0x800802d0 + ((r_u32(0x80093dd0) << 4) & 0xff0u));
    y = r_u32(0x80094664);
    w_u32(0x80094664, y + (wave << 6));
    if ((sint32)r_u32(0x80094668) < (sint32)0xe2500000u)
        w_u32(0x80094668, 0xe2500000);
    FUN_800474B8();
    if (r_u32(0x80093d60) != 1)
        return 1;
    result = r_s8(0x80093dd9);
    if (result != 1)
        return result;
    result = FUN_800473B0((sint32)0xee480000u);
    if (!result)
        return result;
    w_u32(0x80093d60, r_u32(0x80093d60) + 1u);
    w_u8(0x80093dd9, 0);
    for (i = 0; i < 64; i++)
    {
        object = 0x8009a3c8 + 36u * i;
        if (r_u32(object) != 0xffffffffu && (sint32)r_u32(object + 12) < (sint32)0xee300000u)
            w_u32(object, 0xffffffffu);
    }
    w_u16(r_u32(0x800941cc) + 12, 3);
    return FUN_80056B78();
}

GDB_CALL sint32 FUN_80046C08(uint32 object)
{
    FUNCTION_MARKER(0x80046c08u, "GAME.EXE");
    uint32 offset, x, z, y, kind;
    sint32 speed, result, bounces;
    if (r_s16(object + 24) <= 0)
    {
        if (!r_s16(object + 26))
            return 473;
        kind = r_u32(object);
        kind = kind == 473 ? 479 : (kind == 474 || kind == 475 ? 481 : 55);
        if (kind != 55)
            FUN_800167C4(kind, r_u32(object + 4), 0, r_u32(object + 12), r_s16(object + 16));
        w_u32(object, 55);
        return 55;
    }
    offset = ((uint16)r_s16(object + 16) >> 5) * 2u;
    if (((uint16)r_s16(object + 34) & 1u) && !FUN_80019718((sint32)r_u32(object + 4), (sint32)r_u32(object + 12)))
        w_u16(object + 34, (uint16)((uint16)r_s16(object + 34) - 1u));
    speed = r_s16(object + 24);
    x = r_u32(object + 4) + ((uint32)(speed * r_s16(0x800802d0 + offset)) << 1);
    z = r_u32(object + 12) - ((uint32)(speed * r_s16(0x800806d0 + offset)) << 1);
    if (r_s16(object + 34) || !FUN_80019718((sint32)x, (sint32)z))
    {
        w_u32(object + 4, x);
        w_u32(object + 12, z);
    }
    w_u32(object + 8, r_u32(object + 8) + ((uint32)(sint32)r_s16(object + 32) << 16));
    w_u16(object + 32, (uint16)((uint16)r_s16(object + 32) + 1u));
    bounces = r_s16(object + 30);
    y = r_u32(object + 8);
    if ((sint32)y <= 0)
        return (sint32)y;
    if (!bounces)
        FUN_80056670(222, 255 >> (bounces & 31), 6144);
    w_u16(object + 30, (uint16)((uint16)r_s16(object + 30) + 1u));
    w_u32(object + 8, 0);
    w_u16(object + 32, (uint16)(0u - (uint32)(r_s16(object + 32) >> 1)));
    result = r_s16(object + 24) >> 2;
    w_u16(object + 24, (uint16)result);
    return result;
}

GDB_CALL sint32 FUN_8002EB6C(uint32 object)
{
    FUNCTION_MARKER(0x8002eb6cu, "GAME.EXE");
    uint32 phase, offset, radius, x, z, result;
    if ((sint32)r_u32(0x80094338) < -622)
    {
        if (r_s16(object + 30))
            FUN_800567F8(r_s16(object + 30) - 1);
        if (r_s16(object + 28))
            FUN_800567F8(r_s16(object + 28) - 1);
        w_u32(0x80094338, (uint32)-624);
        w_u32(object, 55);
        return 55;
    }
    if (!(r_u32(0x80094338) & 0xfffffffeu))
        w_u8(0x80093dd9, 0);
    else if ((r_u32(0x80094338) & 0xfffffffeu) == (uint32)-150)
    {
        w_u16(r_u32(0x800941dc) + 12, 0);
        w_u16(r_u32(0x800941e0) + 12, 0);
    }
    if (!(ff_rand_8006D348() & 3u))
    {
        offset = (ff_rand_8006D348() & 2047u) * 2u;
        radius = ff_rand_8006D348() & 127u;
        radius += radius >> 3;
        x = (uint32)(sint32)r_s16(0x800802d0 + offset) * radius + r_u32(object + 4);
        z = (uint32)(sint32)r_s16(0x800806d0 + offset) * radius + r_u32(object + 12);
        FUN_80031C50(579, x, 0xfff00000u, z);
    }
    phase = r_u32(0x80094338);
    result = phase - 2u;
    if ((sint32)phase >= 624)
        return (sint32)result;
    w_u32(0x80094338, result);
    if (!r_s16(object + 30))
        w_u16(object + 30, (uint16)((uint32)FUN_80056670(154, 79, 6144) + 1u));
    if (r_u32(0x80094338) >= 2u)
        return 0;
    if (r_s16(object + 30))
        FUN_800567F8(r_s16(object + 30) - 1);
    w_u16(object + 30, (uint16)((uint32)FUN_80056670(237, 95, 5888) + 1u));
    result = (uint32)FUN_80056670(237, 111, 5920) + 1u;
    w_u16(object + 28, (uint16)result);
    return (sint32)result;
}

GDB_CALL sint32 FUN_80058374(void)
{
    FUNCTION_MARKER(0x80058374u, "GAME.EXE");
    uint32 offset = 2u * (uint32)(sint32)r_s16(0x800af440), source = 0x800a8768 + 12u * (uint32)(sint32)r_s16(0x8009ade8 + offset), target = 0x800a8768 + 12u * (uint32)(sint32)r_s16(0x8009adf8 + offset), v[4], phase;
    int i, j;
    /* Four load-before-store chunks preserve overlap behavior of both MIPS copy paths. */
    for (i = 0; i < 4; i++)
    {
        for (j = 0; j < 4; j++)
            v[j] = r_u32(source + 16u * i + 4u * j);
        for (j = 0; j < 4; j++)
            w_u32(target + 16u * i + 4u * j, v[j]);
    }
    /* Aligned LWL/LWR and SWL/SWR pairs each access the full word twice */
    v[0] = r_u32(source + 64);
    v[0] = r_u32(source + 64);
    v[1] = r_u32(source + 68);
    v[1] = r_u32(source + 68);
    w_u32(target + 64, v[0]);
    w_u32(target + 64, v[0]);
    w_u32(target + 68, v[1]);
    w_u32(target + 68, v[1]);
    for (i = 0; i < 6; i++, source += 12, target += 12)
    {
        phase = (uint32)(sint32)r_s16(0x800945d8 + 2u * i) + r_u32(i < 3 ? 0x80094674 : 0x80094794);
        w_u8(source + 5, (uint8)phase);
        w_u8(source + 1, (uint8)phase);
        w_u8(source + 11, (uint8)(phase + 31u));
        w_u8(source + 9, (uint8)(phase + 31u));
        phase += 32u;
        w_u8(target + 5, (uint8)phase);
        w_u8(target + 1, (uint8)phase);
        w_u8(target + 11, (uint8)(phase + 31u));
        w_u8(target + 9, (uint8)(phase + 31u));
    }
    return 0;
}

GDB_CALL sint32 FUN_8003F81C(uint32 object)
{
    FUNCTION_MARKER(0x8003f81cu, "GAME.EXE");
    uint32 input = r_u32(0x80091a18), offset, delta;
    sint32 error, result;
    if (input)
    {
        if ((input & 1u) || (input & 6u) == 6u)
        {
            if (r_s16(object + 30) == 1 && ((uint16)r_s16(object + 22) & 1u))
                w_u16(object + 24, 0);
            else
            {
                w_u16(object + 30, (uint16)((uint16)r_s16(object + 30) ^ 1u));
                if (r_s16(object + 30) == 1)
                    w_u16(object + 22, 35);
            }
        }
        else if (input & 6u)
        {
            offset = input & 2u ? 0x80091a34u : 0x80091a2cu;
            delta = r_u32(object + 4) + r_u32(offset);
            w_u32(0x80091a3c, delta);
            w_u32(0x80091a40, r_u32(object + 12) + r_u32(offset + 4));
        }
    }
    delta = (uint32)FUN_8001614C(r_s16(object + 6), r_s16(object + 14), r_s16(0x80091a3e), r_s16(0x80091a42)) - (uint16)r_s16(object + 16);
    w_u16(0x80091a44, (uint16)delta);
    error = (sint16)delta;
    if (error < 0)
        error = -error;
    if (error > 1024 && (sint32)r_u32(0x800941a0) < 300 && !r_s16(object + 30))
    {
        w_u16(object + 22, 25);
        w_u16(object + 30, 1);
        w_u16(object + 24, 30);
    }
    result = r_s16(0x80091a44) >> (r_u32(0x80091a18) ? 4 : 6);
    w_u16(object + 16, (uint16)((uint32)(uint16)r_s16(object + 16) + (uint32)result));
    return result;
}

GDB_CALL sint32 FUN_80040FAC(sint32 index)
{
    FUNCTION_MARKER(0x80040facu, "GAME.EXE");
    uint32 actor = 0x800b4318 + 244u * (uint32)index, item, points[16], matrix, x, z;
    int i;
    sint32 result;
    if (r_u32(actor + 28) & 0x800u)
    {
        x = r_u32(actor + 40);
        z = r_u32(actor + 48);
        item = FUN_80031C50(604, x, 0, z);
        w_u16(item + 28, (uint16)index);
        FUN_80056670(19, 128, 6144);
        w_u32(0x800941a8, 16);
        x = r_u32(actor + 40);
        z = r_u32(actor + 48);
        FUN_80031F64(x, z, item);
    }
    if (r_u32(actor + 28) & 0x2000u)
        w_u32(actor + 232, r_u32(actor + 232) | 1u);
    if (!(r_u32(actor + 28) & 0x4000u))
        return 0;
    for (i = 0; i < 16; i++)
        points[i] = r_u32(0x80091bac + 4u * i);
    if (r_u32(actor + 232) & 2u)
        return 2;
    for (i = 0; i < 3; i++)
    {
        points[4 + i] += points[i];
        points[12 + i] += points[8 + i];
    }
    matrix = 0x800ba274 + 504u * (uint32)index;
    ff_transform_stack_vector_800234E8(matrix, points);
    ff_transform_stack_vector_800234E8(matrix, points + 4);
    ff_transform_stack_vector_800234E8(matrix, points + 8);
    ff_transform_stack_vector_800234E8(matrix, points + 12);
    FUN_8004141C(877, points[0], points[1], points[2], points[4] - points[0], points[5] - points[1], points[6] - points[2]);
    result = (sint32)FUN_8004141C(877, points[8], points[9], points[10], points[12] - points[8], points[13] - points[9], points[14] - points[10]);
    return result;
}

GDB_CALL sint32 FUN_80033A6C(sint32 index)
{
    FUNCTION_MARKER(0x80033a6cu, "GAME.EXE");
    uint32 actor = 0x800b4318 + 244u * (uint32)index, point[3], angle;
    uint32 state, effect_x, effect_z;
    sint32 attacker, attacker_type, sample;
    int i;
    if (r_s16(actor + 84))
        FUN_800317D0(index);
    else
    {
        if (r_u32(actor + 12) - 141u < 2u)
        {
            w_u32(0x80094190, 4);
            FUN_8001B9F4(index, 1, point);
            attacker_type = r_s16(0x800b434c + 244u * (uint32)(sint32)r_s8(actor + 129));
            sample = r_u32(actor + 12) == 142 && attacker_type == 49 ? 172 : 78;
            FUN_80056670(sample, 255, 6144);
            FUN_8001697C(865, point[0], point[1], point[2]);
            angle = (uint32)(sint32)r_s16(actor + 56);
            for (i = 0; i < 4; i++)
                FUN_8003124C(index, point[0], point[1], point[2], angle);
            for (i = 0; i < 12; i++)
                FUN_80031164(index, point[0], point[1], point[2], angle);
        }
        attacker = r_s8(actor + 129);
        state = r_u32(actor + 12);
        FUN_8001F41C(attacker, index, state);
        FUN_8001B850(r_s8(actor + 129), index);
    }
    if (!(r_u32(actor + 28) & 0x800u))
        return 0;
    effect_x = r_u32(actor + 40);
    effect_z = r_u32(actor + 48);
    FUN_80031C50(606, effect_x, 0, effect_z);
    return FUN_80029684(index);
}

GDB_CALL sint32 FUN_80033534(sint32 index)
{
    FUNCTION_MARKER(0x80033534u, "GAME.EXE");
    uint32 actor = 0x800b4318 + 244u * (uint32)index, angle = (uint32)(sint32)r_s16(actor + 56), x = r_u32(actor + 40), y = r_u32(actor + 44), z = r_u32(actor + 48);
    uint32 state, effect_x, effect_z;
    sint32 attacker;
    int i;
    if (r_s16(actor + 84))
        FUN_800317D0(index);
    else
    {
        if (r_u32(actor + 12) == 189)
        {
            for (i = 0; i < 12; i++)
            {
                FUN_8003124C(index, x, y, z, angle);
                FUN_80031164(index, x, y, z, angle);
            }
            w_u32(0x80094190, 4);
            w_u32(0x800941a8, 8);
            FUN_80056670(78, 255, 5632);
        }
        attacker = r_s8(actor + 129);
        state = r_u32(actor + 12);
        FUN_8001F41C(attacker, index, state);
        if (index >= 2)
            FUN_8001B850(r_s8(actor + 129), index);
    }
    if (!(r_u32(actor + 28) & 0x800u))
        return 0;
    effect_x = r_u32(actor + 40);
    effect_z = r_u32(actor + 48);
    FUN_80031C50(606, effect_x, 0, effect_z);
    FUN_80029684(index);
    attacker = r_s8(actor + 129);
    state = r_u32(actor + 12);
    FUN_8001F41C(attacker, index, state);
    if (index < 2)
        return 1;
    return FUN_8001B850(r_s8(actor + 129), index);
}

/* Table BB9B0 is shared by player and NPC animation events; callers discard V0. */
void ff_actor_event_call(uint32 target, sint32 index)
{
    switch (target)
    {
        case 0x80033f7c:
            /* Only the spawning arm requires retained caller stack words */
            FUN_80033F7C(index, 0);
            return;

        case 0x80032ed8:
            FUN_80032ED8();
            return;
        case 0x80032f38:
            FUN_80032F38(index);
            return;
        case 0x80033008:
            FUN_80033008(index);
            return;
        case 0x80033068:
            FUN_80033068(index);
            return;
        case 0x800330cc:
            FUN_800330CC(index);
            return;
        case 0x8003314c:
            FUN_8003314C(index);
            return;
        case 0x800332bc:
            FUN_800332BC(index);
            return;
        case 0x80033374:
            FUN_80033374(index);
            return;
        case 0x80033488:
            FUN_80033488(index);
            return;
        case 0x80033534:
            FUN_80033534(index);
            return;
        case 0x800336cc:
            FUN_800336CC(index);
            return;
        case 0x8003375c:
            FUN_8003375C(index);
            return;
        case 0x800338e0:
            FUN_800338E0(index);
            return;
        case 0x80033a6c:
            FUN_80033A6C(index);
            return;
        case 0x80033c44:
            FUN_80033C44(index);
            return;
        case 0x80033d7c:
            FUN_80033D7C(index);
            return;
        case 0x80033dc8:
            FUN_80033DC8(index);
            return;
        case 0x80033e18:
            FUN_80033E18(index);
            return;
        case 0x80033e38:
            FUN_80033E38(index);
            return;
        case 0x80033e58:
            FUN_80033E58(index);
            return;
        case 0x800341a0:
            FUN_800341A0(index);
            return;
        case 0x80040fac:
            FUN_80040FAC(index);
            return;
    }
    fprintf(stderr, "XPORT_ACTOR_EVENT target=%08X index=%d\n", target, index);
    ff_wip_stop(__FUNCTION__, __FILE__, __LINE__);
}

GDB_CALL sint32 FUN_8003375C(sint32 index)
{
    FUNCTION_MARKER(0x8003375cu, "GAME.EXE");
    uint32 actor = 0x800b4318 + 244u * (uint32)index, point[3], angle;
    uint32 state, effect_x, effect_z;
    sint32 attacker;
    int i;
    if (r_s16(actor + 84))
        FUN_800317D0(index);
    else
    {
        attacker = r_s8(actor + 129);
        state = r_u32(actor + 12);
        FUN_8001F41C(attacker, index, state);
        if (index >= 2)
            FUN_8001B850(r_s8(actor + 129), index);
        FUN_8001B9F4(index, 1, point);
        FUN_80056670(95, 255, 6144);
        FUN_8001697C(865, point[0], point[1], point[2]);
        angle = (uint32)(sint32)r_s16(actor + 56);
        for (i = 0; i < 4; i++)
            FUN_8003124C(index, point[0], point[1], point[2], angle);
        for (i = 0; i < 12; i++)
            FUN_80031164(index, point[0], point[1], point[2], angle);
    }
    if (!(r_u32(actor + 28) & 0x800u))
        return 0;
    effect_x = r_u32(actor + 40);
    effect_z = r_u32(actor + 48);
    FUN_80031C50(606, effect_x, 0, effect_z);
    return FUN_80029684(index);
}

GDB_CALL sint32 FUN_800338E0(sint32 index)
{
    FUNCTION_MARKER(0x800338e0u, "GAME.EXE");
    uint32 actor = 0x800b4318 + 244u * (uint32)index, point[3];
    uint32 state, effect_x, effect_z;
    sint32 attacker;
    int i;
    if (r_s16(actor + 84))
        return FUN_800317D0(index);
    effect_x = r_u32(actor + 40);
    effect_z = r_u32(actor + 48);
    FUN_80031C50(606, effect_x, 0, effect_z);
    if (r_u32(actor + 12) == 187)
    {
        FUN_8001B9F4(index, 1, point);
        FUN_80056670(35, 255, 6144);
        FUN_800293D8(index);
        w_u32(0x800941a8, 16);
        w_u32(0x80094190, 4);
        for (i = 0; i < 12; i++)
            FUN_8003124C(index, point[0], point[1], point[2], r_s16(actor + 56));
        for (i = 0; i < 12; i++)
            FUN_80031164(index, point[0], point[1], point[2], r_s16(actor + 56));
    }
    FUN_80029684(index);
    attacker = r_s8(actor + 129);
    state = r_u32(actor + 12);
    FUN_8001F41C(attacker, index, state);
    if (index < 2)
        return 1;
    return FUN_8001B850(r_s8(actor + 129), index);
}

GDB_CALL sint32 FUN_80042F80(uint32 object)
{
    FUNCTION_MARKER(0x80042f80u, "GAME.EXE");
    uint32 region, count, i, z, volume, pan;
    region = 0x8009ec70;
    count = r_u32(0x8009479c);
    for (i = 0; (sint32)i < (sint32)count; i++, region += 24)
        if (r_u32(region + 20) == object)
            break;
    FUN_80029AFC(region, object);
    z = r_u32(object + 12) - 0x800000u;
    w_u32(object + 12, z);
    if (z == 0xc7000000u)
    {
        FUN_80056840(r_u32(0x800944b8), 6128);
        FUN_80056840(r_u32(0x800944c0), 6336);
    }
    else if (r_u32(object + 12) == 0xdf000000u)
    {
        FUN_80056840(r_u32(0x800944b8), 6112);
        FUN_80056840(r_u32(0x800944c0), 6272);
    }
    ff_spatial_sound_80046884(object + 4, &volume, &pan);
    volume = FUN_80042820(volume, (uint32)-80);
    FUN_800568C8(r_u32(0x800944b8), (sint32)volume, (sint32)pan);
    volume = FUN_80042820(volume, (uint32)-48);
    FUN_800568C8(r_u32(0x800944c0), (sint32)volume, (sint32)pan);
    region = 0x8009ec70;
    count = r_u32(0x8009479c);
    for (i = 0; (sint32)i < (sint32)count; i++, region += 24)
        if (r_u32(region + 20) == object)
            break;
    FUN_80029AFC(region, object);
    if ((sint32)r_u32(object + 12) < (sint32)0xbb000000u)
    {
        w_u32(object, 55);
        FUN_800567F8(r_u32(0x800944b8));
        return FUN_800567F8(r_u32(0x800944c0));
    }
    return 55;
}

GDB_CALL sint32 FUN_800429F0(uint32 object)
{
    FUNCTION_MARKER(0x800429f0u, "GAME.EXE");
    sint32 state, result;
    uint32 y;
    if (!r_s8(0x80093dd9) && !r_u32(0x80093d60))
        return 0;
    if (!r_u32(0x80093d60) && !FUN_800473B0((sint32)0xf7650000u))
        return 0;
    if (!r_s16(object + 26))
    {
        w_u16(r_u32(0x800941cc) + 12, 3);
        w_u32(0x80093d60, 1);
        w_u16(object + 26, 1);
        FUN_80056670(244, 255, 6144);
        result = FUN_80056670(245, 255, 6144);
        w_u8(0x80093dd9, 0);
        return result;
    }
    state = r_s16(object + 26);
    if (state == 1 || state == 2)
    {
        w_u16(object + 24, (uint16)((uint16)r_s16(object + 24) + 8u));
        if (state == 1 && r_s16(object + 24) > 320)
        {
            w_u16(r_u32(0x800941d0) + 12, 0);
            w_u16(object + 26, 2);
        }
        else if (state == 2 && r_s16(object + 24) > 768)
        {
            w_u16(object + 24, 768);
            w_u16(object + 26, 3);
        }
        y = (0u - (uint32)(sint32)r_s16(object + 24)) << 16;
        w_u32(object + 8, y);
        return (sint32)y;
    }
    if (r_s16(object + 26) == 3 && FUN_800473B0((sint32)0xfa300000u))
    {
        w_u16(r_u32(0x800941d0) + 12, 3);
        w_u32(0x80093d60, 2);
        w_u32(object, 55);
        return 55;
    }
    return 3;
}

GDB_CALL uint32 FUN_80040E14(sint32 index, sint32 other_index)
{
    FUNCTION_MARKER(0x80040e14u, "GAME.EXE");
    uint32 actor = 0x800b4318 + 244u * (uint32)index, other = 0x800b4318 + 244u * (uint32)other_index, offset, start[3], end[3], flags, mask;
    if (r_u32(actor + 232) & 2u)
        return 2;
    if (r_u32(other + 232) & 2u)
        return 2;
    offset = (ff_rand_8006D348() & 15u) * 16u;
    FUN_8001B9F4(index, (sint32)r_u32(0x8007f114 + offset), start);
    FUN_8001B9F4(other_index, (sint32)r_u32(0x8007f118 + offset), end);
    FUN_8004150C(r_u32(0x8007f11c + offset), start[0], start[1], start[2], end[0], end[1], end[2], 0x02000200u, 0x028090f0u);
    flags = r_u32(actor + 156);
    mask = r_u32(0x8007f120 + offset);
    w_u32(actor + 156, flags | mask);
    flags = r_u32(other + 156);
    mask = r_u32(0x8007f120 + offset);
    w_u32(other + 156, flags | mask);
    return FUN_80031B20(871, end[0], end[1], end[2], 0, 0, 0);
}

GDB_CALL sint32 FUN_80041F5C(uint32 object, sint32 target_x, sint32 target_z)
{
    FUNCTION_MARKER(0x80041f5cu, "GAME.EXE");
    sint32 yaw = (sint16)r_u16(object + 16) >> 5, pitch = r_s16(object + 18), roll = r_s16(object + 20), error, distance, speed;
    sint32 x = r_s16(object + 6), z = r_s16(object + 14), trig;
    uint32 delta, result, offset, position;
    delta = (uint32)FUN_8001614C(x, z, target_x, target_z) - ((uint32)yaw << 5);
    if ((uint16)(delta + 8192) >= 16385)
    {
        result = r_u16(object + 16) + (uint32)((sint16)delta >> 4);
        w_u16(object + 16, (uint16)result);
        return (sint32)result;
    }
    error = (sint16)delta;
    if (error > 4096)
        error = 4096;
    if (error < -4096)
        error = -4096;
    roll -= (roll + error) >> 4;
    yaw = (yaw - (roll >> 8)) & 2047;
    distance = (sint32)(r_u32(0x800941a0) - 3072);
    if (distance > 3072)
        distance = 3072;
    pitch = (sint32)((uint32)pitch + (uint32)((sint32)(((uint32)distance << 2) - (uint32)pitch) >> 4));
    speed = (sint16)r_u16(0x800802d0 + ((uint32)(pitch >> 4) & 0xffe)) >> 7;
    offset = (uint32)yaw * 2;
    trig = r_s16(0x800802d0 + offset);
    position = r_u32(object + 4);
    w_u32(object + 4, position + (uint32)(speed * trig));
    trig = r_s16(0x800806d0 + offset);
    position = r_u32(object + 12);
    w_u32(object + 12, position - (uint32)(speed * trig));
    result = (uint32)yaw << 5;
    w_u16(object + 16, (uint16)result);
    w_u16(object + 18, (uint16)pitch);
    w_u16(object + 20, (uint16)roll);
    return (sint32)result;
}

GDB_CALL sint32 FUN_8003F4C4(uint32 object, uint32 x, uint32 y, uint32 z)
{
    FUNCTION_MARKER(0x8003f4c4u, "GAME.EXE");
    sint32 hit = FUN_800199AC((sint32)x, (sint32)z);
    uint32 target, position[3];
    if (hit < 0)
        return (sint32)((uint32)hit << 1);
    target = r_u32(0x8009ec84 + 24u * (uint32)hit);
    if (!target || r_u32(target) != 367)
        return 367;
    position[0] = x;
    position[1] = y;
    position[2] = z;
    FUN_80031C50(894, x, y, z);
    /* 31C50 saves S3=x at entry-SP-12, consumed by 1F004 */
    ff_stack_projectile_contact(hit, 0, position, x);
    w_u16(object + 22, 40);
    w_u16(object + 30, 1);
    w_u16(object + 24, 15);
    return 15;
}

GDB_CALL sint32 FUN_8003F670(uint32 object, uint32 dx, uint32 dz)
{
    FUNCTION_MARKER(0x8003f670u, "GAME.EXE");
    if (!FUN_80019718((sint32)(r_u32(object + 4) + dx), (sint32)(r_u32(object + 12) + dz)))
        return 0;
    FUN_8003F4C4(object, r_u32(object + 4) + dx, r_u32(object + 8), r_u32(object + 12) + dz);
    return 1;
}

GDB_CALL sint32 FUN_8002FF30(uint32 object)
{
    FUNCTION_MARKER(0x8002ff30u, "GAME.EXE");
    uint32 offset = ((uint16)r_s16(object + 16) >> 5) * 2u, x = r_u32(object + 4), z = r_u32(object + 12), dx, dz, value;
    sint32 i, hit, result, speed = r_s16(object + 24);
    dx = (uint32)((speed * r_s16(0x800802d0 + offset)) >> 1);
    dz = (uint32)((speed * r_s16(0x800806d0 + offset)) >> 1);
    for (i = 0; i < 4; i++)
    {
        hit = FUN_800199AC((sint32)x, (sint32)z);
        if (hit >= 0)
        {
            FUN_80056670(19, 192, 6144);
            FUN_80031C50(894, r_u32(object + 4), r_u32(object + 8), r_u32(object + 12));
            /* 31C50 saves caller S3 (this loop index) at entry-SP-12. */
            result = FUN_8001F004(hit, object, (uint32)i);
            w_u32(object, 0xffffffffu);
            return result;
        }
        x += dx;
        z -= dz;
        w_u32(object + 4, x);
        w_u32(object + 12, z);
    }
    value = (uint32)(uint16)r_s16(object + 32) - 2u;
    w_u16(object + 32, (uint16)value);
    w_u32(object + 8, r_u32(object + 8) - (uint32)((sint32)(value << 16) >> 1));
    w_u16(object + 18, (uint16)((uint32)(uint16)r_s16(object + 18) + (uint32)(r_s16(object + 26) >> 1)));
    if ((sint32)r_u32(object + 8) > 0)
        w_u32(object, 0xffffffffu);
    return -1;
}

GDB_CALL sint32 FUN_80030DBC(uint32 region, sint32 index)
{
    FUNCTION_MARKER(0x80030dbcu, "GAME.EXE");
    uint32 actor = 0x800b4318 + 244u * (uint32)index, item, object, cursor, count, i, value;
    sint32 slot = r_s8(actor + 126);
    if (slot == -1)
        return (sint32)(244u * (uint32)index);
    item = 0x8009a3c8 + 36u * (uint32)slot;
    if (r_u32(item) != 514)
        return 299;
    object = r_u32(region + 20);
    if (r_u32(object) != 299)
        return 299;
    FUN_80056670(151, 192, 6144);
    w_u32(0x80094190, 4);
    w_u32(object, 300);
    w_u8(actor + 126, 255);
    w_u32(item, 0xffffffffu);
    w_u16(region + 12, 0);
    w_u16(region + 8, 17);
    count = r_u32(0x8009479c);
    if ((sint32)count <= 0)
        return (sint32)count;
    cursor = 0x8009ec70;
    i = 0;
    do
    {
        if (r_u32(cursor + 20) == object && cursor != region)
        {
            value = (uint32)(sint32)r_s16(object + 6) + 32u;
            w_u16(cursor + 4, (uint16)value);
            w_u16(cursor, (uint16)value);
            value = r_u16(object + 14);
            w_u16(cursor + 6, (uint16)value);
            w_u16(cursor + 2, (uint16)value);
            FUN_8003B77C(cursor, 300);
            value = r_u32(0x8009479c) - 1u;
            w_u32(0x8009479c, value);
            return (sint32)value;
        }
        i++;
        count = r_u32(0x8009479c);
        cursor += 24;
    } while ((sint32)i < (sint32)count);
    return 0;
}

GDB_CALL sint32 FUN_8002EE60(uint32 object)
{
    FUNCTION_MARKER(0x8002ee60u, "GAME.EXE");
    uint32 offset = ((uint16)r_s16(object + 16) >> 5) * 2u, y, result;
    if (!((uint16)r_s16(object + 22) & 63u))
        FUN_80016654(575, r_u32(object + 4), r_u32(object + 8), r_u32(object + 12));
    if (!r_s16(object + 22))
        w_u16(object + 26, (uint16)((ff_rand_8006D348() & 31u) + 256u));
    w_u32(object + 4, r_u32(object + 4) + (uint32)((r_s16(0x800802d0 + offset) * r_s16(object + 26)) >> 3));
    w_u32(object + 12, r_u32(object + 12) - (uint32)((r_s16(0x800806d0 + offset) * r_s16(object + 26)) >> 3));
    w_u16(object + 22, (uint16)((uint16)r_s16(object + 22) + 32u));
    w_u16(object + 18, (uint16)((uint32)(uint16)r_s16(object + 18) + (uint32)(r_s16(object + 22) >> 1)));
    if (r_s16(object + 18) > 20480)
        w_u16(object + 18, 20480);
    y = r_u32(object + 8) + ((uint32)(sint32)r_s16(object + 22) << 9);
    w_u32(object + 8, y);
    if ((sint32)y > (sint32)0xfda00000u)
    {
        w_u32(object + 8, 0xfda00000);
        w_u16(object + 18, 20480);
    }
    result = (uint32)(uint16)r_s16(object + 26) - 16u;
    w_u16(object + 26, (uint16)result);
    result <<= 16;
    if ((sint32)result < 0)
        w_u16(object + 26, 0);
    return (sint32)result;
}

GDB_CALL sint32 FUN_8002C2E4(uint32 object)
{
    FUNCTION_MARKER(0x8002c2e4u, "GAME.EXE");
    sint32 velocity;
    uint32 y, offset;
    if (!r_s16(object + 26))
    {
        w_u16(object + 22, (uint16)((ff_rand_8006D348() & 2047u) + 512u));
        w_u16(object + 26, (uint16)((ff_rand_8006D348() & 511u) + 128u));
        w_u16(object + 16, (uint16)(((sint32)(0u - r_u32(object + 4)) >> 19) & 2047));
        w_u16(object + 24, (uint16)-256);
        w_u16(object + 32, (uint16)(ff_rand_8006D348() & 63u));
    }
    offset = (uint32)(sint32)r_s16(object + 16) * 2u;
    w_u32(object + 4, r_u32(object + 4) + (uint32)((r_s16(object + 24) * r_s16(0x800802d0 + offset)) >> 1));
    offset = (uint32)(sint32)r_s16(object + 16) * 2u;
    w_u32(object + 12, r_u32(object + 12) - (uint32)((r_s16(object + 24) * r_s16(0x800806d0 + offset)) >> 1));
    w_u16(object + 18, (uint16)((uint16)r_s16(object + 18) + (uint16)r_s16(object + 22)));
    w_u16(object + 20, (uint16)((uint16)r_s16(object + 20) + (uint16)r_s16(object + 26)));
    velocity = r_s16(object + 32);
    if (velocity >= -255)
        w_u16(object + 32, (uint16)(velocity - 3));
    y = r_u32(object + 8) - ((uint32)(sint32)r_s16(object + 32) << 15);
    w_u32(object + 8, y);
    if ((sint32)y > 0x02000000)
    {
        w_u32(object, 0xffffffffu);
        return -1;
    }
    return 0;
}

GDB_CALL uint32 FUN_800419D4(uint32 object)
{
    FUNCTION_MARKER(0x800419d4u, "GAME.EXE");
    uint32 volume, pan, random, offset, x, z, result;
    if (r_s16(object + 30))
    {
        ff_spatial_sound_80046884(object + 4, &volume, &pan);
        volume = FUN_80042820(volume, (uint32)-48);
        FUN_800568C8(r_s16(object + 30) - 1, (sint32)volume, (sint32)pan);
    }
    else
        w_u16(object + 30, (uint16)((uint32)FUN_80056670(156, 0, 6144) + 1u));
    random = ff_rand_8006D348();
    offset = (random & 2047u) * 2u;
    x = r_u32(object + 4) + (uint32)(sint32)r_s16(0x800802d0 + offset) * (random & 31u);
    z = r_u32(object + 12) - (uint32)(sint32)r_s16(0x800806d0 + offset) * (random & 31u);
    FUN_8004141C(876, x, r_u32(object + 8) - r_u32(0x8007f214), z, 0, 0u - r_u32(0x8007f218), 0);
    result = r_u32(0x80093dd0) & 3u;
    if (result)
        return result;
    return FUN_80031B20(871, x, r_u32(object + 8) - r_u32(0x8007f214) - r_u32(0x8007f21c), z, 0, 0, 0);
}

GDB_CALL uint32 FUN_8002E9FC(uint32 object)
{
    FUNCTION_MARKER(0x8002e9fcu, "GAME.EXE");
    uint32 volume, pan, x, z, result;
    if (r_s16(object + 30))
    {
        ff_spatial_sound_80046884(object + 4, &volume, &pan);
        volume = FUN_80042820(volume, (uint32)-128);
        FUN_800568C8(r_s16(object + 30) - 1, (sint32)volume, (sint32)pan);
    }
    else
    {
        uint32 random = ff_rand_8006D348();
        w_u16(object + 30, (uint16)((uint32)FUN_80056670(154, 0, (sint32)(random & 63u) + 6144) + 1u));
    }
    if (r_u32(0x80093dd0) & 1u)
        return 410;
    x = r_u32(object + 4);
    if (r_u32(object) == 410)
        x -= 0x600000u;
    x += ((ff_rand_8006D348() & 31u) - 16u) << 16;
    z = ((ff_rand_8006D348() & 31u) - 16u) << 16;
    z += r_u32(object + 12);
    result = FUN_80031B20(872, x, 0xff100000u, z, 0, 0, 0);
    w_u16(result + 26, (uint16)((32767 - r_s16(0x800802d0 + ((r_u32(0x80093dd0) << 3) & 0x7f8u))) >> 10));
    return result;
}

GDB_CALL sint32 FUN_80028704(void)
{
    FUNCTION_MARKER(0x80028704u, "GAME.EXE");
    sint32 angle, excess;
    uint32 offset, x, z, product;
    if (!FUN_800473B0(0x12000000))
        angle = (sint32)(0xfd000000u - r_u32(0x80094004)) >> 13;
    else if (FUN_8004732C((sint32)0xfa000000u))
        angle = (sint32)(16384u - (uint32)((sint32)(r_u32(0x80094008) - 0x16800000u) >> 13));
    else if (FUN_800472A8(0))
        angle = (sint32)((uint32)((sint32)(r_u32(0x80094008) - 0x16800000u) >> 13) - 16384u);
    else
        angle = (sint32)(0xfd000000u - r_u32(0x80094004)) >> 13;
    w_u16(0x8009419c, (uint16)angle);
    FUN_800476E8();
    x = r_u32(0x80094660);
    if ((sint32)x > 0x03000000)
    {
        offset = 2u * ((uint32)((r_s16(0x8009419c) >> 5) + 512) & 2047u);
        product = (uint32)(sint32)r_s16(0x800802d0 + offset);
        excess = (sint32)(x - 0x03000000u) >> 15;
        product *= (uint32)excess;
        z = r_u32(0x80094668);
        w_u32(0x80094668, z - product);
        w_u32(0x80094660, 0x03000000);
    }
    w_u32(0x80094198, r_u32(0x80094668) + 0x0c7fffffu <= 0x0aeffffeu ? 512 : 0);
    return FUN_800474B8();
}

GDB_CALL sint32 FUN_8002D678(uint32 object)
{
    FUNCTION_MARKER(0x8002d678u, "GAME.EXE");
    sint32 phase = r_s16(object + 22), angle = (r_s16(0x800802d0 + (((uint32)phase & 7u) << 9)) * phase) >> 9, count;
    uint32 kind, x, z;
    w_u16(object + (r_u32(object) == 788 ? 20u : 18u), (uint16)angle);
    w_u32(object + 8, (0u - (uint32)(sint32)r_s16(0x800802d0 + (((uint32)(uint16)r_s16(object + 18) >> 4) & 0x7feu))) << 8);
    if (r_s16(object + 22) > 0)
        w_u16(object + 22, (uint16)(r_s16(object + 22) - 1));
    count = r_s16(object + 26);
    if (count <= 0)
        return count;
    w_u16(object + 26, (uint16)(count - 1));
    kind = r_u32(object) == 428 ? 872 : 871;
    if ((uint16)r_s16(object + 26) & 1u)
        return 1;
    x = ((ff_rand_8006D348() & 63u) - 32u) << 16;
    x += r_u32(object + 4);
    z = ((ff_rand_8006D348() & 63u) - 32u) << 16;
    z += r_u32(object + 12);
    return (sint32)FUN_80031B20(kind, x, 0xffc00000u, z, 0, 0, 0);
}

GDB_CALL sint32 FUN_8002E8A4(uint32 object)
{
    FUNCTION_MARKER(0x8002e8a4u, "GAME.EXE");
    uint32 kind, x, z;
    sint32 height, phase, angle;
    if (!(r_u32(0x80093dd0) & 3u))
    {
        kind = r_u32(object);
        height = kind == 809 || kind == 811 ? -224 : -112;
        x = r_u32(object + 4);
        if (kind == 805)
            x += 0x300000u;
        if (r_u32(object) == 807)
            x -= 0x300000u;
        x += ((ff_rand_8006D348() & 31u) - 16u) << 16;
        z = ((ff_rand_8006D348() & 31u) - 16u) << 16;
        z += r_u32(object + 12);
        FUN_80031B20(871, x, (uint32)height << 16, z, 0, 0, 0);
    }
    phase = r_s16(object + 22);
    angle = (r_s16(0x800802d0 + (((uint32)phase & 7u) << 9)) * phase) >> 9;
    w_u16(object + 18, (uint16)angle);
    w_u32(object + 8, (0u - (uint32)(sint32)r_s16(0x800802d0 + (((uint32)angle >> 4) & 0x7feu))) << 8);
    phase = r_s16(object + 22);
    if (phase > 0)
    {
        phase--;
        w_u16(object + 22, (uint16)phase);
    }
    return phase;
}

GDB_CALL uint32 FUN_8002F950(uint32 object)
{
    FUNCTION_MARKER(0x8002f950u, "GAME.EXE");
    uint32 offset, value, x, y, z;
    sint32 speed, trig;
    speed = r_s16(object + 24);
    offset = 2 * (uint32)(sint32)r_s16(object + 16);
    trig = r_s16(0x800802d0 + offset);
    x = r_u32(object + 4);
    w_u32(object + 4, x - (uint32)((speed * trig) >> 3));
    speed = r_s16(object + 24);
    offset = 2 * (uint32)(sint32)r_s16(object + 16);
    trig = r_s16(0x800806d0 + offset);
    z = r_u32(object + 12);
    w_u32(object + 12, z + (uint32)((speed * trig) >> 3));
    w_u16(object + 18, (uint16)(r_u16(object + 18) + 4480));
    w_u16(object + 20, (uint16)(r_u16(object + 20) + 7296));
    value = (uint32)r_u16(object + 32) - 6;
    w_u16(object + 32, (uint16)value);
    y = r_u32(object + 8) - (uint32)((sint32)(value << 16) >> 3);
    w_u32(object + 8, y);
    if ((sint32)y > 0)
    {
        w_u32(object, 0xffffffffu);
        x = r_u32(object + 4);
        z = r_u32(object + 12);
        return FUN_80031C50(895, x, 0, z);
    }
    value = (uint32)r_u16(object + 26) + 1;
    w_u16(object + 26, (uint16)value);
    if (value & 1)
        return 1;
    x = r_u32(object + 4);
    y = r_u32(object + 8);
    z = r_u32(object + 12);
    return FUN_80031B20(870, x, y, z, 0, 0, 0);
}

GDB_CALL sint32 FUN_800284D4(void)
{
    FUNCTION_MARKER(0x800284d4u, "GAME.EXE");
    sint32 state = (sint32)r_u32(0x80093d60), angle, x, limit;
    if (state >= 5)
        angle = (sint32)(16384u - (uint32)((sint32)(r_u32(0x80094008) + 0x04800000u) >> 14));
    else
    {
        x = (sint32)r_u32(0x80094004);
        if (x <= 0x074fffff && (sint32)r_u32(0x80093d60) >= 3)
            angle = (sint32)(16384u - (uint32)((sint32)(r_u32(0x80094008) + 0x04800000u) >> 12));
        else
        {
            x = (sint32)((uint32)x - 0x0f000000u);
            if (x < (sint32)0xfab00000u)
                x = (sint32)0xfab00000u;
            if (x > 0x05500000)
                x = 0x05500000;
            angle = (sint32)(0u - (uint32)x) >> 12;
            angle = (sint32)((uint32)angle - (uint32)((sint16)angle >> 2));
        }
    }
    w_u16(0x8009419c, (uint16)angle);
    FUN_800476E8();
    state = (sint32)r_u32(0x80093d60);
    if (state == 4 || state == 6)
    {
        limit = state == 4 ? 0x06430000 : (sint32)0xf2500000u;
        if ((sint32)r_u32(0x80094660) > limit)
            w_u32(0x80094660, (uint32)limit);
    }
    return FUN_800474B8();
}

GDB_CALL sint32 FUN_80033C44(sint32 index)
{
    FUNCTION_MARKER(0x80033c44u, "GAME.EXE");
    uint32 actor = 0x800b4318 + 244u * (uint32)index, point[3], angle, i;
    sint32 kind = r_s16(actor + 52);
    if (!r_s16(actor + 84))
        return FUN_80029944(index);
    if (r_s16(actor + 84) != 1 || kind != 3)
        return 3;
    FUN_8001B9F4(index, 4, point);
    FUN_80056670(95, 255, 6144);
    FUN_8001697C(865, point[0], point[1], point[2]);
    angle = (uint32)(sint32)r_s16(actor + 56);
    for (i = 0; i < 4; i++)
        FUN_8003124C(r_s8(actor + 116), point[0], point[1], point[2], angle);
    for (i = 0; i < 12; i++)
        FUN_80031164(r_s8(actor + 116), point[0], point[1], point[2], angle);
    return 0;
}

GDB_CALL sint32 FUN_8003F6EC(uint32 object)
{
    FUNCTION_MARKER(0x8003f6ecu, "GAME.EXE");
    uint32 angle = (uint32)(r_s16(object + 16) >> 5) & 2047u, dx, dz;
    sint32 speed = r_s16(object + 24), result;
    dx = (uint32)speed * (uint32)(sint32)r_s16(0x800802d0 + 2 * angle);
    dz = (uint32)speed * (uint32)(sint32)r_s16(0x800802d0 + 2 * ((angle + 512u) & 2047u));
    if (r_s16(object + 30) == 1)
    {
        w_u32(object + 4, r_u32(object + 4) - dx);
        w_u16(object + 22, (uint16)(r_s16(object + 22) - 1));
        result = (sint32)((uint32)(uint16)r_s16(object + 22) << 16);
        w_u32(object + 12, r_u32(object + 12) + dz);
        if (result < 0)
            w_u16(object + 30, 0);
        return result;
    }
    w_u32(object + 4, r_u32(object + 4) + dx);
    w_u32(object + 12, r_u32(object + 12) - dz);
    result = speed + 2;
    if (speed < 30)
        w_u16(object + 24, (uint16)result);
    return result;
}

GDB_CALL uint32 FUN_8004471C(uint32 object)
{
    FUNCTION_MARKER(0x8004471cu, "GAME.EXE");
    uint32 angle, position, region = 0x8009ec70, i = 0;
    sint32 count, wave;
    if (r_u32(object + 4) + 0x18000000u > 0x30000000u)
    {
        w_u16(object + 16, r_u16(object + 16) ^ 0x8000);
        w_u32(object, r_u32(0x8007f328 + 4u * (ff_rand_8006D348() & 7u)));
    }
    angle = 2 * (r_u16(object + 16) >> 5);
    wave = r_s16(0x800802d0 + angle);
    position = r_u32(object + 4);
    w_u32(object + 4, position - 192 * (uint32)wave);
    wave = r_s16(0x800806d0 + angle);
    position = r_u32(object + 12);
    w_u32(object + 12, position + 192 * (uint32)wave);
    count = (sint32)r_u32(0x8009479c);
    while ((sint32)i < count)
    {
        if (r_u32(region + 20) == object)
            break;
        i++;
        region += 24;
    }
    return FUN_80029AFC(region, object);
}

GDB_CALL uint32 FUN_80033E58(sint32 index)
{
    FUNCTION_MARKER(0x80033e58u, "GAME.EXE");
    uint32 actor = 0x800b4318 + 244u * (uint32)index, point[3], angle, i;
    if (r_u32(actor + 28) & 0x800u)
        return FUN_80032EE0(index);
    FUN_8001B9F4(index, 8, point);
    FUN_8001697C(865, point[0], point[1], point[2]);
    angle = (uint32)(sint32)r_s16(actor + 56);
    for (i = 0; i < 4; i++)
        FUN_8003124C(index, point[0], point[1], point[2], angle);
    for (i = 0; i < 12; i++)
        FUN_80031164(index, point[0], point[1], point[2], angle);
    return (uint32)FUN_80056670(59, 255, 6144);
}

GDB_CALL sint32 FUN_8002D8FC(uint32 object)
{
    FUNCTION_MARKER(0x8002d8fcu, "GAME.EXE");
    uint32 angle;
    sint32 result;
    w_u16(object + 22, (uint16)(r_s16(object + 22) + (r_u32(object) == 72 ? 96 : 64)));
    w_u16(object + 18, (uint16)(r_s16(object + 18) - r_s16(object + 22)));
    angle = 2u * ((uint32)(uint16)r_s16(object + 16) >> 5);
    w_u32(object + 4, r_u32(object + 4) - (uint32)((r_s16(object + 24) * r_s16(0x800802d0 + angle)) >> 1));
    w_u32(object + 12, r_u32(object + 12) + (uint32)((r_s16(object + 24) * r_s16(0x800806d0 + angle)) >> 1));
    w_u16(object + 24, (uint16)(r_s16(object + 24) - 64));
    if (r_s16(object + 24) < 0)
        w_u16(object + 24, 0);
    if (r_s16(object + 18) >= -16384)
        return -16384;
    w_u16(object + 18, (uint16)-16384);
    result = -(r_s16(object + 22) >> 1);
    w_u16(object + 22, (uint16)result);
    return result;
}

GDB_CALL sint32 FUN_800300BC(uint32 object)
{
    FUNCTION_MARKER(0x800300bcu, "GAME.EXE");
    sint32 angle, step;
    if (!r_u8(0x800941a4))
    {
        w_u16(object + 30, 0);
        return 0;
    }
    angle = r_s16(r_u32(0x800940d0) + 16);
    if (angle < 16384)
    {
        step = (angle + 33) >> 4;
        if (step >= 49 && !r_s16(object + 30))
        {
            FUN_80056670(124, 255, 4608);
            w_u16(object + 30, 1);
        }
        if (step >= 257)
            step = 256;
        angle += step;
    }
    if (angle >= 16385)
    {
        FUN_80056670(20, 255, 5632);
        w_u32(0x800941a8, 8);
        angle = 16384;
        w_u16(r_u32(0x800941c8) + 12, 0);
    }
    w_u16(r_u32(0x800940d0) + 16, (uint16)angle);
    w_u16(r_u32(0x800940d4) + 16, (uint16)-angle);
    return -angle;
}

GDB_CALL sint32 FUN_80028874(void)
{
    FUNCTION_MARKER(0x80028874u, "GAME.EXE");
    sint32 angle = (sint32)(0x13000000u - r_u32(0x80094004)) >> 13, x, camera_x, camera_z;
    w_u16(0x8009419c, (uint16)((uint32)angle - (uint32)((sint16)angle >> 3)));
    FUN_800476E8();
    if ((sint32)r_u32(0x80094660) < 0x10000000)
        w_u32(0x80094660, 0x10000000);
    if ((sint32)r_u32(0x80094660) > 0x18000000)
        w_u32(0x80094660, 0x18000000);
    if ((sint32)r_u32(0x80093e04) >= 2)
    {
        x = r_s8(0x80093de0) ? 6144 : 4096;
        camera_x = r_s16(0x80094006);
        camera_z = r_s16(0x8009400a);
        w_u16(0x8009419c, (uint16)((uint32)FUN_8001614C(x, 3584, camera_x, camera_z) - 32768u));
        w_u32(0x80094660, (uint32)x << 16);
        w_u32(0x80094668, 0x0e000000);
    }
    return FUN_800474B8();
}

GDB_CALL sint32 FUN_800313FC(uint32 x, uint32 y, uint32 z)
{
    FUNCTION_MARKER(0x800313fcu, "GAME.EXE");
    uint32 i, object, angle, radius;
    for (i = 0; i < 8; i++)
    {
        object = FUN_80030F44();
        if (!object)
            continue;
        w_u32(object, 897);
        angle = (ff_rand_8006D348() & 2047u) * 2;
        radius = ff_rand_8006D348() & 255u;
        w_u32(object + 4, x + (uint32)(sint32)r_s16(0x800802d0 + angle) * radius);
        w_u32(object + 12, z - (uint32)(sint32)r_s16(0x800806d0 + angle) * radius);
        w_u32(object + 8, y);
        w_u16(object + 16, (uint16)ff_rand_8006D348());
        w_u16(object + 18, (uint16)ff_rand_8006D348());
        w_u16(object + 20, (uint16)ff_rand_8006D348());
        w_u16(object + 32, (uint16)(ff_rand_8006D348() & 15u));
    }
    return 0;
}

GDB_CALL sint32 FUN_80028A94(void)
{
    FUNCTION_MARKER(0x80028a94u, "GAME.EXE");
    sint32 angle, x, z;
    int target = r_s8(0x80093e00) != 0;
    if (FUN_800472A8(0x10000000) && (z = (sint32)r_u32(0x80094008)) < (sint32)0xf6000000u)
    {
        angle = (sint32)((uint32)z + 0x0e000000u) >> 13;
        w_u16(0x8009419c, (uint16)((uint32)angle - 16384u));
    }
    else if (target)
    {
        x = (sint32)r_u32(0x80093df4);
        z = (sint32)r_u32(0x80093df8);
        FUN_80027DB4((sint32)((uint32)x << 16), (sint32)((uint32)z << 16));
        return FUN_800474B8();
    }
    else
    {
        angle = (sint32)(0x04000000u - r_u32(0x80094004)) >> 13;
        if (angle > 16384)
            angle = 16384;
        if (angle < -16384)
            angle = -16384;
        w_u16(0x8009419c, (uint16)angle);
    }
    FUN_800476E8();
    return FUN_800474B8();
}

GDB_CALL sint32 FUN_80028F30(void)
{
    FUNCTION_MARKER(0x80028f30u, "GAME.EXE");
    int orbit = 1;
    sint32 x, z, camera_x, camera_z;
    uint32 wave, y;
    if (r_s8(0x80093e00))
    {
        x = (sint32)r_u32(0x80093df4);
        z = (sint32)r_u32(0x80093df8);
        camera_x = r_s16(0x80094006);
        camera_z = r_s16(0x8009400a);
        w_u16(0x8009419c, (uint16)FUN_8001614C(x, z, camera_x, camera_z));
    }
    else
    {
        w_u16(0x8009419c, (uint16)(0xffff8000u - (uint32)((sint32)r_u32(0x80094004) >> 13)));
        FUN_80028C08();
        if ((sint32)r_u32(0x80094198) > 0)
            orbit = 0;
    }
    if (orbit)
    {
        FUN_800476E8();
        wave = (uint32)(sint32)r_s16(0x800802d0 + ((r_u32(0x80093dd0) << 4) & 0xff0u));
        y = r_u32(0x80094664);
        w_u32(0x80094664, y + (wave << 6));
    }
    if ((sint32)r_u32(0x80094668) > 0x0dc00000)
        w_u32(0x80094668, 0x0dc00000);
    return FUN_800474B8();
}

GDB_CALL sint32 FUN_8002837C(void)
{
    FUNCTION_MARKER(0x8002837cu, "GAME.EXE");
    uint32 object;
    sint32 angle = (sint32)(0xf9000000u - r_u32(0x80094004)) >> 13;
    sint32 x, z, camera_x, camera_z;
    w_u16(0x8009419c, (uint16)angle);
    if ((sint16)angle > 14336)
        w_u16(0x8009419c, 14336);
    if (r_s16(0x8009419c) < -14336)
        w_u16(0x8009419c, (uint16)-14336);
    object = r_u32(0x800947c8);
    if (object)
    {
        x = r_s16(object + 6);
        z = r_s16(object + 14);
        camera_x = r_s16(0x80094006);
        camera_z = r_s16(0x8009400a);
        w_u16(0x8009419c, (uint16)FUN_8001614C(x, z, camera_x, camera_z));
    }
    FUN_800476E8();
    if ((sint32)r_u32(0x80094660) < (sint32)0xee800000u)
        w_u32(0x80094660, 0xee800000u);
    if (r_u32(0x80093d60) == 3 && r_s8(0x80093dd9) == 1)
        w_u16(0x800941a6, 1024);
    return FUN_800474B8();
}

GDB_CALL uint32 FUN_800416E0(uint32 object)
{
    FUNCTION_MARKER(0x800416e0u, "GAME.EXE");
    uint32 result, r;
    w_u16(object + 22, (uint16)(r_s16(object + 22) + 1));
    if (r_s16(object + 22) >= 8)
        w_u32(object, 0xffffffffu);
    w_u32(object + 4, r_u32(object + 4) + ((uint32)(sint32)r_s16(object + 16) << 15));
    w_u32(object + 8, r_u32(object + 8) + ((uint32)(sint32)r_s16(object + 18) << 15));
    w_u32(object + 12, r_u32(object + 12) + ((uint32)(sint32)r_s16(object + 20) << 15));
    result = r_u32(object + 8);
    if ((sint32)result > 0)
    {
        r = ff_rand_8006D348();
        result = FUN_80031B20(871, r_u32(object + 4) + (((r & 127u) - 64u) << 16), 0u - (((r >> 12) & 3u) << 16), r_u32(object + 12) + ((((r >> 6) & 127u) - 64u) << 16), 0, 0, 0);
        w_u32(object + 8, 0);
    }
    return result;
}

GDB_CALL sint32 FUN_80043128(uint32 object)
{
    FUNCTION_MARKER(0x80043128u, "GAME.EXE");
    uint32 volume, pan;
    if (!r_s16(object + 26))
    {
        if (FUN_80047434(0x06000000))
            return 1;
        w_u16(object + 26, 1);
        w_u16(object + 24, 0);
        w_u32(0x800944c8, (uint32)FUN_80056670(146, 0, 4608));
    }
    w_u16(object + 24, (uint16)(r_s16(object + 24) + 1));
    if (r_s16(object + 24) >= 49)
        w_u16(object + 24, 48);
    w_u32(object + 4, r_u32(object + 4) + ((uint32)(sint32)r_s16(object + 24) << 16));
    ff_spatial_sound_80046884(object + 4, &volume, &pan);
    volume = FUN_80042820(volume, 24);
    FUN_800568C8((sint32)r_u32(0x800944c8), (sint32)volume, (sint32)pan);
    if ((sint32)r_u32(object + 4) > 0x15000000)
    {
        w_u32(object, 55);
        return FUN_800567F8((sint32)r_u32(0x800944c8));
    }
    return 55;
}

GDB_CALL uint32 FUN_8002C454(uint32 object)
{
    FUNCTION_MARKER(0x8002c454u, "GAME.EXE");
    uint32 result;
    if (!r_s16(object + 26))
    {
        w_u16(object + 26, (uint16)ff_rand_8006D348());
        w_u16(object + 18, (uint16)ff_rand_8006D348());
        w_u16(object + 20, (uint16)ff_rand_8006D348());
        w_u16(object + 24, 32);
    }
    w_u32(object + 12, r_u32(object + 12) + ((uint32)(sint32)r_s16(object + 24) << 16));
    w_u16(object + 18, (uint16)(r_s16(object + 18) + 1120));
    w_u16(object + 20, (uint16)(r_s16(object + 20) + 1824));
    result = r_u32(object + 8) - ((uint32)(sint32)r_s16(0x800802d0 + (((uint32)(sint32)r_s16(object + 26) << 3) & 0xff8u)) << 4);
    w_u32(object + 8, result);
    if ((sint32)result > 0)
        w_u32(object + 8, 0);
    if ((sint32)r_u32(object + 12) > 0x40000000)
        w_u32(object, 0xffffffffu);
    w_u16(object + 26, (uint16)(r_s16(object + 26) + 1));
    result = (uint32)(uint16)r_s16(object + 24) + 1u;
    w_u16(object + 24, (uint16)result);
    return result;
}

GDB_CALL sint32 FUN_80058524(void)
{
    FUNCTION_MARKER(0x80058524u, "GAME.EXE");
    uint32 index = 2u * (uint32)(sint32)r_s16(0x800af446), source = 0x800a8768 + 12u * (uint32)(sint32)r_s16(0x8009ad80 + index), dest = 0x800a8768 + 12u * (uint32)(sint32)r_s16(0x8009ad90 + index), i, v;
    uint8 u;
    for (i = 0; i < 2; i++, source += 12, dest += 12)
    {
        v = (uint32)(sint32)r_s16(0x800945d8 + 2 * i) + r_u32(0x80094674);
        w_u8(source + 5, (uint8)v);
        w_u8(source + 1, (uint8)v);
        w_u8(source + 11, (uint8)(v + 31));
        w_u8(source + 9, (uint8)(v + 31));
        u = r_u8(source);
        w_u8(dest + 8, u);
        w_u8(dest, u);
        u = r_u8(source + 4);
        w_u8(dest + 10, u);
        w_u8(dest + 4, u);
        w_u8(dest + 5, (uint8)(v + 32));
        w_u8(dest + 1, (uint8)(v + 32));
        w_u8(dest + 11, (uint8)(v + 63));
        w_u8(dest + 9, (uint8)(v + 63));
        w_u16(dest + 6, (uint16)r_s16(source + 6));
    }
    return 0;
}

GDB_CALL sint32 FUN_80029944(sint32 index)
{
    FUNCTION_MARKER(0x80029944u, "GAME.EXE");
    sint32 kind = r_s16(0x800b434c + 244u * (uint32)index), sound = 51, pitch = 6144;
    switch (kind)
    {
        case 1:
        case 12:
        case 48:
        case 29:
        case 30:
        case 38:
            sound = 66;
            break;
        case 2:
        case 49:
            sound = 81;
            break;
        case 3:
            sound = 102;
            break;
        case 32:
            sound = 164;
            break;
        case 14:
            sound = 81;
            pitch = 5888;
            break;
    }
    return FUN_80056670(sound, 255, pitch - (sint32)(ff_rand_8006D348() & 255u));
}

GDB_CALL uint32 FUN_8004141C(uint32 kind, uint32 x, uint32 y, uint32 z, uint32 dx, uint32 dy, uint32 dz)
{
    FUNCTION_MARKER(0x8004141cu, "GAME.EXE");
    uint32 object = FUN_80030F44(), i;
    if (!object)
        return 0;
    for (i = 0; i < 36; i++)
        w_u8(object + i, 0);
    ff_rand_8006D348();
    w_u16(object + 22, (uint16)(ff_rand_8006D348() & (kind == 877 ? 1u : 7u)));
    w_u32(object, kind);
    w_u32(object + 4, x);
    w_u32(object + 8, y);
    w_u32(object + 12, z);
    w_u16(object + 16, (uint16)(dx >> 16));
    w_u16(object + 18, (uint16)(dy >> 16));
    w_u16(object + 20, (uint16)(dz >> 16));
    return object;
}

GDB_CALL uint32 FUN_80041E6C(uint32 object)
{
    FUNCTION_MARKER(0x80041e6cu, "GAME.EXE");
    sint32 random, divisor;
    uint32 slot, offset, x, y, z;
    if (r_u32(0x80093dd0) & 1u)
        return 1;
    random = (sint32)ff_rand_8006D348();
    divisor = (sint32)r_u32(0x80091bec);
    if (!divisor || (divisor == -1 && (uint32)random == 0x80000000u))
        ff_wip_stop(__FUNCTION__, __FILE__, __LINE__);
    slot = 3u * (uint32)(random % divisor);
    w_u16(object + 26, (uint16)slot);
    offset = (uint32)((sint32)(slot << 16) >> 14);
    x = r_u32(object + 4);
    x += r_u32(0x8007f220 + offset);
    y = r_u32(object + 8);
    y += r_u32(0x8007f224 + offset);
    z = r_u32(object + 12);
    z += r_u32(0x8007f228 + offset);
    y += ((uint32)(random >> 8) & 63u) << 16;
    return FUN_80031B20(880, x, y, z, 0, 0, 0);
}

GDB_CALL sint32 FUN_800455E4(sint32 index)
{
    FUNCTION_MARKER(0x800455e4u, "GAME.EXE");
    uint32 actor = 0x800b4318 + 244u * (uint32)index;
    if (r_s16(actor + 84))
        FUN_800317D0(index);
    else
    {
        FUN_8001F41C((uint32)(sint32)r_s8(actor + 129), (uint32)index, r_u32(actor + 12));
        if (index >= 2)
            FUN_8001B850(r_s8(actor + 129), index);
    }
    if (!(r_u32(actor + 28) & 0x800u))
        return 0;
    FUN_80031C50(606, r_u32(actor + 40), 0, r_u32(actor + 48));
    FUN_80029684(index);
    FUN_8001F41C((uint32)(sint32)r_s8(actor + 129), (uint32)index, r_u32(actor + 12));
    if (index < 2)
        return 1;
    return FUN_8001B850(r_s8(actor + 129), index);
}

GDB_CALL void FUN_80046E1C(uint32 object)
{
    FUNCTION_MARKER(0x80046e1cu, "GAME.EXE");
    sint32 amplitude = r_s16(object + 24), value;
    if (amplitude <= 0)
        return;
    if (r_s16(object + 26))
    {
        value = (sint32)((uint32)(sint32)r_s16(0x800802d0 + 2u * (uint32)(sint32)r_s16(object + 22)) * (uint32)amplitude) >> 12;
        w_u16(object + 18, (uint16)value);
        w_u16(object + 20, 0);
    }
    else
    {
        w_u16(object + 18, 0);
        value = (sint32)((uint32)(sint32)r_s16(0x800802d0 + 2u * (uint32)(sint32)r_s16(object + 22)) * (uint32)(sint32)r_s16(object + 24)) >> 12;
        w_u16(object + 20, (uint16)value);
    }
    w_u16(object + 22, (uint16)(((uint32)(uint16)r_s16(object + 22) + (uint32)(r_s16(object + 28) >> 7)) & 2047u));
    w_u16(object + 28, (uint16)(r_s16(object + 28) + 511));
    w_u16(object + 24, (uint16)(r_s16(object + 24) - 2));
}

GDB_CALL sint32 FUN_80041894(uint32 object)
{
    FUNCTION_MARKER(0x80041894u, "GAME.EXE");
    sint32 result;
    w_u16(object + 22, (uint16)(r_s16(object + 22) + 1));
    if (r_s16(object + 22) >= 28)
        w_u32(object, 0xffffffffu);
    if (r_s16(object + 22) == 11)
    {
        w_u16(object + 16, (uint16)((ff_rand_8006D348() & 15u) - 8u));
        w_u16(object + 20, (uint16)((ff_rand_8006D348() & 15u) - 8u));
    }
    w_u32(object + 4, r_u32(object + 4) + ((uint32)(sint32)r_s16(object + 16) << 16));
    w_u32(object + 12, r_u32(object + 12) + ((uint32)(sint32)r_s16(object + 20) << 16));
    w_u16(object + 26, (uint16)(r_s16(object + 26) - 13));
    result = (sint32)((uint32)(sint32)r_s16(object + 26) << 16) >> 2;
    w_u32(object + 8, r_u32(object + 8) - (uint32)result);
    return result;
}

GDB_CALL sint32 FUN_800412A8(uint32 kind, uint32 x, uint32 y, uint32 z, sint32 yaw, sint32 pitch, sint32 speed)
{
    FUNCTION_MARKER(0x800412a8u, "GAME.EXE");
    uint32 object = FUN_80030F44(), r, i, angle;
    sint32 scale, result;
    if (!object)
        return 0;
    for (i = 0; i < 36; i++)
        w_u8(object + i, 0);
    w_u32(object, kind);
    w_u32(object + 4, x);
    w_u32(object + 8, y);
    w_u32(object + 12, z);
    r = ff_rand_8006D348();
    scale = (sint32)((uint32)speed - 1u + (r & 3u));
    angle = ((sint32)((uint32)yaw + (uint32)((sint32)r >> 6)) >> 5) & 2047u;
    w_u16(object + 16, (uint16)((sint32)((uint32)scale * (uint32)(sint32)r_s16(0x800802d0 + 2 * angle)) >> 15));
    w_u16(object + 20, (uint16)((sint32)((uint32)scale * (uint32)-r_s16(0x800806d0 + 2 * angle)) >> 15));
    angle = ((uint32)(pitch >> 5) & 2047u) * 2 + 0x800802d0;
    w_u16(object + 18, (uint16)((sint32)((uint32)scale * (uint32)-r_s16(angle)) >> 15));
    w_u16(object + 16, (uint16)((sint32)((uint32)(sint32)r_s16(object + 16) * (uint32)(sint32)r_s16(angle)) >> 15));
    result = (sint32)((uint32)(sint32)r_s16(object + 20) * (uint32)(sint32)r_s16(angle)) >> 15;
    w_u16(object + 20, (uint16)result);
    return result;
}

GDB_CALL sint32 FUN_80041DD8(uint32 object)
{
    FUNCTION_MARKER(0x80041dd8u, "GAME.EXE");
    uint32 angle;
    sint32 result;
    FUN_800412A8(874, r_u32(object + 4), r_u32(object + 8) - 0x500000u, r_u32(object + 12), r_s16(object + 16), 8192, 32);
    angle = ((uint32)(uint16)r_s16(object + 22) + 8u) & 2047u;
    w_u16(object + 22, (uint16)angle);
    result = (r_s16(0x800802d0 + 2 * angle) >> 2) + 1024;
    w_u16(object + 16, (uint16)result);
    return result;
}

GDB_CALL sint32 FUN_8004160C(uint32 object)
{
    FUNCTION_MARKER(0x8004160cu, "GAME.EXE");
    w_u16(object + 24, (uint16)(r_s16(object + 24) + 1));
    if (r_s16(object + 24) >= 5)
        w_u32(object, 0xffffffffu);
    w_u32(object + 4, r_u32(object + 4) + ((uint32)(sint32)r_s16(object + 16) << 14));
    w_u32(object + 8, r_u32(object + 8) + ((uint32)(sint32)r_s16(object + 18) << 14));
    w_u32(object + 12, r_u32(object + 12) + ((uint32)(sint32)r_s16(object + 20) << 14));
    if (r_s16(object + 24) >= 4)
        FUN_800322C8(r_u32(object + 4), r_u32(object + 12), object);
    if ((sint32)r_u32(object + 8) > 0)
        w_u32(object, 0xffffffffu);
    return -1;
}

GDB_CALL sint32 FUN_8003F5A0(uint32 actor, uint32 object, sint32 x, sint32 z)
{
    FUNCTION_MARKER(0x8003f5a0u, "GAME.EXE");
    sint32 first, second;
    x >>= 16;
    z >>= 16;
    first = FUN_800163F4(x, z, r_s16(actor + 42), r_s16(actor + 50));
    second = FUN_800163F4(x, z, r_s16(actor + 42), r_s16(actor + 50));
    if (first >= 80 && second >= 80)
        return 0;
    if (r_u32(actor + 12) == 147)
        return 147;
    FUN_800237CC(actor, r_u32(0x800b3be8 + 4u * (uint32)(sint32)r_s16(actor + 52)), 147);
    w_u16(object + 22, 60);
    w_u16(object + 30, 1);
    w_u16(object + 24, 10);
    return 10;
}

GDB_CALL sint32 FUN_8003EFEC(void)
{
    FUNCTION_MARKER(0x8003efecu, "GAME.EXE");
    sint32 count = (sint32)r_u32(0x8009403c);
    uint32 i = 0, object = 0x800b89f0;
    if (count <= 0)
        return count;
    do
    {
        if (r_u32(object) == 781)
        {
            w_u16(object + 26, 1);
            FUN_80056670(110, 255, 5936);
        }
        ++i;
        object += 36;
    } while ((sint32)i < (sint32)r_u32(0x8009403c));
    return 0;
}

GDB_CALL sint32 FUN_8002E37C(uint32 object)
{
    FUNCTION_MARKER(0x8002e37cu, "GAME.EXE");
    uint32 speed = (uint32)(uint16)r_s16(object + 22) + 64u;
    sint32 result, count;
    w_u16(object + 22, (uint16)speed);
    w_u16(object + 18, (uint16)((uint32)(uint16)r_s16(object + 18) - speed));
    if (r_s16(object + 18) >= -16384)
        return 0;
    count = r_s16(object + 26);
    if (!count)
    {
        FUN_800292A4(object);
        count = r_s16(object + 26);
    }
    if (count < 2)
    {
        w_u16(object + 26, (uint16)(count + 1));
        w_u32(0x800941a8, 6);
        FUN_80056670(117, 255, 6144);
    }
    w_u16(object + 18, 0xc000);
    result = -(r_s16(object + 22) >> 2);
    w_u16(object + 22, (uint16)result);
    return result;
}

GDB_CALL sint32 FUN_800289C0(void)
{
    FUNCTION_MARKER(0x800289c0u, "GAME.EXE");
    sint32 angle;
    if (!r_u8(0x80093dd0) && !r_u32(0x800941a8))
        w_u32(0x800941a8, 8);
    angle = (sint32)(0u - r_u32(0x80094004)) >> 13;
    w_u16(0x8009419c, (uint16)angle);
    angle += r_s16(0x800802d0 + ((r_u32(0x80093dd0) << 4) & 0xff0u)) >> 6;
    w_u16(0x8009419c, (uint16)angle);
    FUN_800476E8();
    if ((sint32)r_u32(0x80094668) < (sint32)0xe0c00000u)
        w_u32(0x80094668, 0xe0c00000u);
    w_u32(0x80094660, 0);
    return FUN_800474B8();
}

GDB_CALL sint32 FUN_80027FA4(void)
{
    FUNCTION_MARKER(0x80027fa4u, "GAME.EXE");
    sint32 z = r_u32(0x80093d60) == 2 ? 256 : 768;
    sint32 camera_x = r_s16(0x80094006), camera_z = r_s16(0x8009400a);
    uint32 angle = (uint32)FUN_8001614C(0, z, camera_x, camera_z) - 32768u;
    w_u16(0x8009419c, (uint16)angle);
    FUN_800476E8();
    if ((sint32)r_u32(0x80094660) > 0x02000000)
        w_u32(0x80094660, 0x02000000);
    if ((sint32)r_u32(0x80094660) < (sint32)0xfe000000u)
        w_u32(0x80094660, 0xfe000000u);
    return FUN_800474B8();
}

GDB_CALL sint32 FUN_80029A40(sint32 index)
{
    FUNCTION_MARKER(0x80029a40u, "GAME.EXE");
    sint32 kind = r_s16(0x800b434c + 244u * (uint32)index), sample, pitch = 6144;
    switch (kind)
    {
        case 32:
        case 50:
            sample = 163;
            break;
        case 48:
            sample = 194;
            break;
        case 49:
            sample = 175;
            break;
        case 14:
            sample = 207;
            pitch = 5632;
            break;
        default:
            return 14;
    }
    return FUN_80056670(sample, 255, pitch - (sint32)(ff_rand_8006D348() & 255u));
}

GDB_CALL uint32 FUN_80031710(uint32 x, uint32 y, uint32 z, uint32 angle)
{
    FUNCTION_MARKER(0x80031710u, "GAME.EXE");
    uint32 object = FUN_80030F44(), result;
    if (!object)
        return 0;
    w_u32(object, (ff_rand_8006D348() & 3u) + 72u);
    w_u32(object + 4, x);
    w_u32(object + 8, y);
    w_u32(object + 12, z);
    w_u16(object + 16, (uint16)((angle + (ff_rand_8006D348() & 511u) - 256u) & 2047u));
    w_u16(object + 24, (uint16)((ff_rand_8006D348() & 63u) + 64u));
    result = ff_rand_8006D348() & 63u;
    w_u16(object + 32, (uint16)(64u - result));
    w_u16(object + 26, 0);
    return result;
}

GDB_CALL uint32 FUN_80031968(uint32 x, uint32 y, uint32 z, uint32 angle)
{
    FUNCTION_MARKER(0x80031968u, "GAME.EXE");
    uint32 object = FUN_80030F44(), result;
    if (!object)
        return 7;
    w_u32(object, 7);
    w_u32(object + 4, x);
    w_u32(object + 8, y);
    w_u32(object + 12, z);
    w_u16(object + 16, (uint16)angle);
    w_u16(object + 32, (uint16)((ff_rand_8006D348() & 31u) + 16u));
    w_u16(object + 24, (uint16)((ff_rand_8006D348() & 31u) + 64u));
    w_u16(object + 32, (uint16)(r_s16(object + 32) >> 1));
    result = (uint32)(r_s16(object + 24) >> 1);
    w_u16(object + 24, (uint16)result);
    w_u16(object + 26, 0);
    return result;
}

GDB_CALL uint32 FUN_80031338(uint32 x, uint32 y, uint32 z, uint32 angle)
{
    FUNCTION_MARKER(0x80031338u, "GAME.EXE");
    uint32 object = FUN_80030F44(), result;
    if (!object)
        return 4;
    w_u32(object, 4);
    w_u32(object + 4, x);
    w_u32(object + 8, y);
    w_u32(object + 12, z);
    w_u16(object + 16, (uint16)((angle + (ff_rand_8006D348() & 255u) - 128u) & 2047u));
    w_u16(object + 18, (uint16)(ff_rand_8006D348() & 2047u));
    w_u16(object + 20, (uint16)(ff_rand_8006D348() & 2047u));
    w_u16(object + 32, (uint16)(0u - (ff_rand_8006D348() & 15u)));
    result = (ff_rand_8006D348() & 31u) + 32u;
    w_u16(object + 24, (uint16)result);
    w_u16(object + 26, 0);
    return result;
}

GDB_CALL uint32 FUN_8003A934(sint32 frame)
{
    FUNCTION_MARKER(0x8003a934u, "GAME.EXE");
    if ((sint32)r_u32(0x8008d4a8) > (sint32)0xff300000u)
        w_u32(0x8008d4a8, 0xff300000u);
    w_u32(0x8009403c, 4);
    w_u32(0x800b89f0, 851);
    FUN_80047A40(frame, 0x800b89f0, 1);
    w_u32(0x800b8a14, 854);
    FUN_80047A40(frame, 0x800b8a14, 2);
    w_u32(0x800b8a38, 854);
    FUN_80047A40(frame, 0x800b8a38, 3);
    w_u32(0x800b8a5c, 853);
    return FUN_80047A40(frame, 0x800b8a5c, 4);
}

GDB_CALL uint32 FUN_8003AA54(sint32 frame)
{
    FUNCTION_MARKER(0x8003aa54u, "GAME.EXE");
    w_u32(0x8009403c, 6);
    w_u32(0x800b8a14, 852);
    FUN_80047A40(frame, 0x800b8a14, 1);
    w_u32(0x800b8a38, 854);
    FUN_80047A40(frame, 0x800b8a38, 2);
    w_u32(0x800b8a5c, 854);
    FUN_80047A40(frame, 0x800b8a5c, 3);
    w_u32(0x800b8a80, 853);
    FUN_80047A40(frame, 0x800b8a80, 4);
    w_u32(0x800b8aa4, 854);
    return FUN_80047A40(frame, 0x800b8aa4, 5);
}

GDB_CALL uint32 FUN_8002BFA8(uint32 object)
{
    FUNCTION_MARKER(0x8002bfa8u, "GAME.EXE");
    uint32 result, i;
    w_u16(object + 32, (uint16)((uint32)(uint16)r_s16(object + 32) + 5u));
    w_u16(object + 16, (uint16)((uint32)(uint16)r_s16(object + 16) + 480u));
    w_u16(object + 18, (uint16)((uint32)(uint16)r_s16(object + 18) + 1120u));
    w_u16(object + 20, (uint16)((uint32)(uint16)r_s16(object + 20) + 1824u));
    result = r_u32(object + 8) + ((uint32)(sint32)r_s16(object + 32) << 16);
    w_u32(object + 8, result);
    if ((sint32)result > 0)
    {
        w_u32(object, 0xffffffffu);
        for (i = 0; i < 4; i++)
            FUN_80031510(r_u32(object + 4), 0, r_u32(object + 12));
        return 0;
    }
    return result;
}

static sint32 region_light_value(uint32 region, uint32 actor, uint32 scale)
{
    sint32 radius = r_s16(region), value, x, z, center_x, center_z, distance;
    x = r_s16(actor + 42);
    z = r_s16(actor + 50);
    center_x = r_s16(region + 4);
    center_z = r_s16(region + 6);
    distance = FUN_800163F4(x, z, center_x, center_z);
    value = (sint32)(((uint32)radius - (uint32)distance) * scale);
    if (!radius || (radius == -1 && (uint32)value == 0x80000000u))
        ff_wip_stop(__FUNCTION__, __FILE__, __LINE__);
    return value / radius;
}

static uint32 region_light_flag(uint32 actor)
{
    uint32 result = r_u8(actor + 156) | 1u;
    w_u8(actor + 156, (uint8)result);
    return result;
}

GDB_CALL uint32 FUN_800306D4(uint32 region, sint32 index)
{
    FUNCTION_MARKER(0x800306d4u, "GAME.EXE");
    uint32 actor = 0x800b4318 + 244u * (uint32)index;
    sint32 value = region_light_value(region, actor, 80);
    if (value >= 49)
        value = 48;
    w_u8(actor + 157, (uint8)(0xffffffd0u - (uint32)value));
    w_u8(actor + 158, (uint8)(0xffffffd0u - (uint32)value));
    w_u8(actor + 159, (uint8)(0xffffffd0u - (uint32)value));
    return region_light_flag(actor);
}

GDB_CALL uint32 FUN_80047174(uint32 region, sint32 index)
{
    FUNCTION_MARKER(0x80047174u, "GAME.EXE");
    uint32 actor = 0x800b4318 + 244u * (uint32)index;
    sint32 radius = r_s16(region), value, x, z, center_x, center_z, scale;
    x = r_s16(actor + 42);
    z = r_s16(actor + 50);
    center_x = r_s16(region + 4);
    center_z = r_s16(region + 6);
    value = FUN_800163F4(x, z, center_x, center_z);
    scale = r_s16(0x8007f354);
    value = (sint32)(((uint32)radius - (uint32)value) * (uint32)scale);
    if (!radius || (radius == -1 && (uint32)value == 0x80000000u))
        ff_wip_stop(__FUNCTION__, __FILE__, __LINE__);
    value /= radius;
    w_u8(actor + 157, 192);
    w_u8(actor + 158, (uint8)((uint32)((sint32)((uint32)value * 192u) >> 7) - 64u));
    w_u8(actor + 159, 192);
    return region_light_flag(actor);
}

GDB_CALL uint32 FUN_80030B94(uint32 region, sint32 index)
{
    FUNCTION_MARKER(0x80030b94u, "GAME.EXE");
    uint32 actor = 0x800b4318 + 244u * (uint32)index;
    sint32 value = region_light_value(region, actor, 32);
    w_u8(actor + 159, (uint8)(value));
    w_u8(actor + 158, (uint8)(value));
    w_u8(actor + 157, (uint8)(value));
    return region_light_flag(actor);
}

GDB_CALL uint32 FUN_80030AD4(uint32 region, sint32 index)
{
    FUNCTION_MARKER(0x80030ad4u, "GAME.EXE");
    uint32 actor = 0x800b4318 + 244u * (uint32)index;
    sint32 value = region_light_value(region, actor, 128);
    w_u8(actor + 159, (uint8)(0u - (uint32)value));
    w_u8(actor + 158, (uint8)(0u - (uint32)value));
    w_u8(actor + 157, (uint8)(0u - (uint32)value));
    return region_light_flag(actor);
}

GDB_CALL uint32 FUN_800307AC(uint32 region, sint32 index)
{
    FUNCTION_MARKER(0x800307acu, "GAME.EXE");
    uint32 actor = 0x800b4318 + 244u * (uint32)index;
    sint32 value = region_light_value(region, actor, 80);
    w_u8(actor + 157, (uint8)((uint32)value - 48u));
    w_u8(actor + 158, (uint8)((uint32)value - 48u));
    w_u8(actor + 159, (uint8)((uint32)value - 48u));
    return region_light_flag(actor);
}

GDB_CALL uint32 FUN_80030874(uint32 region, sint32 index)
{
    FUNCTION_MARKER(0x80030874u, "GAME.EXE");
    uint32 actor = 0x800b4318 + 244u * (uint32)index;
    sint32 value = region_light_value(region, actor, 80);
    w_u8(actor + 157, (uint8)((uint32)(value >> 1) - 48u));
    w_u8(actor + 158, (uint8)((uint32)value - 48u));
    w_u8(actor + 159, (uint8)((uint32)(value >> 1) - 48u));
    return region_light_flag(actor);
}

GDB_CALL uint32 FUN_80030944(uint32 region, sint32 index)
{
    FUNCTION_MARKER(0x80030944u, "GAME.EXE");
    uint32 actor = 0x800b4318 + 244u * (uint32)index;
    sint32 value = region_light_value(region, actor, 80);
    w_u8(actor + 157, (uint8)((uint32)value - 48u));
    w_u8(actor + 158, (uint8)((uint32)(value >> 2) - 48u));
    w_u8(actor + 159, (uint8)((uint32)(value >> 2) - 48u));
    return region_light_flag(actor);
}

GDB_CALL uint32 FUN_800317D0(sint32 index)
{
    FUNCTION_MARKER(0x800317d0u, "GAME.EXE");
    uint32 object = FUN_80030F44(), actor, result = (uint32)index << 4;
    if (!object)
        return result;
    actor = 0x800b4318 + 244u * (uint32)index;
    w_u32(object, r_s16(actor + 52) == 53);
    FUN_80017F88(object, index, 1);
    w_u16(object + 16, (uint16)r_s16(actor + 56) & 2047u);
    result = (ff_rand_8006D348() & 15u) + 12u;
    w_u16(object + 32, (uint16)result);
    w_u16(object + 24, 0);
    w_u16(object + 26, 0);
    return result;
}

GDB_CALL sint32 FUN_80033488(sint32 index)
{
    FUNCTION_MARKER(0x80033488u, "GAME.EXE");
    uint32 actor = 0x800b4318 + 244u * (uint32)index, x, z, state;
    sint32 attacker;
    if (r_s16(actor + 84))
        return (sint32)FUN_800317D0(index);
    x = r_u32(actor + 40);
    z = r_u32(actor + 48);
    FUN_80031C50(606, x, 0, z);
    FUN_80029684(index);
    attacker = r_s8(actor + 129);
    state = r_u32(actor + 12);
    FUN_8001F41C((uint32)attacker, (uint32)index, state);
    if (index < 2)
        return 1;
    return FUN_8001B850(r_s8(actor + 129), index);
}

GDB_CALL sint32 FUN_80057700(sint32 divisor)
{
    FUNCTION_MARKER(0x80057700u, "GAME.EXE");
    uint32 random;
    sint32 value;
    if (divisor < 0)
        divisor = (sint32)(0u - (uint32)divisor);
    random = ff_rand_8006D348();
    value = (sint32)random >> 1;
    value = (sint32)((uint32)value + (uint32)(value >> 1));
    if (!divisor || (divisor == -1 && (uint32)value == 0x80000000u))
        ff_wip_stop(__FUNCTION__, __FILE__, __LINE__);
    value /= divisor;
    return random & 1u ? (sint32)(0u - (uint32)value) : value;
}

GDB_CALL uint32 FUN_8005F058(uint32 stream)
{
    FUNCTION_MARKER(0x8005f058u, "GAME.EXE");
    uint32 result;
    w_u32(0x80093d08, 1);
    w_u32(0x80093d0c, stream);
    w_u32(0x80094e88, FUN_8005F050(r_u32(stream + 4))); /* PsyQ srand */
    result = FUN_8005F050(r_u32(r_u32(0x80093d0c)));
    w_u32(0x80093d10, result);
    w_u32(0x80093d14, 1);
    return result;
}

GDB_CALL uint32 FUN_8005F0D8(uint32 stream)
{
    FUNCTION_MARKER(0x8005f0d8u, "GAME.EXE");
    uint32 random;
    w_u32(0x80093d08, 2);
    w_u32(0x80093d0c, stream);
    w_u32(stream, 0);
    do
    {
        random = ff_rand_8006D348() & 65535u;
        w_u32(r_u32(0x80093d0c) + 4, random);
    } while (!r_u32(r_u32(0x80093d0c) + 4));
    w_u32(0x80094e88, r_u32(r_u32(0x80093d0c) + 4)); /* PsyQ srand */
    w_u32(0x80093d10, 1);
    w_u32(0x80093d14, 1);
    return 1;
}

GDB_CALL sint32 FUN_800341A0(sint32 index)
{
    FUNCTION_MARKER(0x800341a0u, "GAME.EXE");
    uint32 actor = 0x800b4318 + 244u * (uint32)index, object;
    sint32 slot;
    if (!(r_u32(actor + 28) & 0x800u))
        return FUN_80056670(123, 255, 6144);
    slot = r_s8(actor + 126);
    if (slot == -1)
        return -1;
    object = 0x8009a3c8 + 36u * (uint32)slot;
    w_u32(object, r_u32(object) + 1u);
    return (sint32)FUN_80017E30(index);
}

GDB_CALL uint32 FUN_8002ED7C(uint32 object)
{
    FUNCTION_MARKER(0x8002ed7cu, "GAME.EXE");
    uint32 speed, result;
    if (!r_s16(object + 26))
        w_u16(object + 32, (uint16)((ff_rand_8006D348() & 7u) + 8u));
    speed = (uint32)(sint32)r_s16(object + 32);
    w_u32(object + 8, r_u32(object + 8) - (speed << 14));
    w_u16(object + 32, (uint16)((uint32)(uint16)r_s16(object + 32) + 1u));
    if ((sint32)r_u32(object + 8) < (sint32)((0u - r_u32(0x80094338)) << 16))
        w_u32(object, 0xffffffffu);
    result = (uint32)(uint16)r_s16(object + 26) + 1u;
    w_u16(object + 26, (uint16)result);
    return result;
}

GDB_CALL uint32 FUN_80031EB4(uint32 kind, uint32 x, uint32 y, uint32 z, uint32 angle)
{
    FUNCTION_MARKER(0x80031eb4u, "GAME.EXE");
    uint32 object = FUN_800165EC();
    if (!object)
        return 0;
    angle = angle - 4096u + (ff_rand_8006D348() & 8191u);
    w_u32(object, kind);
    w_u32(object + 4, x);
    w_u32(object + 8, y);
    w_u32(object + 12, z);
    w_u16(object + 16, (uint16)angle);
    w_u16(object + 18, 0);
    w_u16(object + 20, 0);
    w_u16(object + 24, 128);
    w_u16(object + 32, 64);
    w_u16(object + 26, 6528);
    return 6528;
}

GDB_CALL sint32 FUN_80058238(void)
{
    FUNCTION_MARKER(0x80058238u, "GAME.EXE");
    uint32 phase;
    sint32 shade;
    if (!r_s8(0x80093dda) && !r_u8(0x80093ddc))
        return 0;
    phase = r_u32(0x80093ddc) + 2u;
    w_u32(0x80093ddc, phase);
    shade = (sint32)((2u * phase + 128u) & 255u) - 128;
    if (shade > 0)
        shade = -shade;
    FUN_80011CCC(0, shade, shade);
    w_u16(0x80093e10, 0);
    w_u16(0x80093e14, (uint16)shade);
    w_u16(0x80093e12, (uint16)shade);
    return shade;
}

GDB_CALL uint32 FUN_8003AA00(sint32 frame)
{
    FUNCTION_MARKER(0x8003aa00u, "GAME.EXE");
    w_u16(0x8008d492, (uint16)((uint32)(uint16)r_s16(0x8008d492) - 2048u));
    w_u32(0x8009403c, 17);
    w_u32(0x800b8c30, 856);
    return FUN_80047A40(frame, 0x800b8c30, 1);
}

GDB_CALL uint32 FUN_80031510(uint32 x, uint32 y, uint32 z)
{
    FUNCTION_MARKER(0x80031510u, "GAME.EXE");
    uint32 object = FUN_80030F44(), result = 6;
    if (!object)
        return result;
    w_u32(object, 6);
    w_u32(object + 4, x);
    w_u32(object + 8, y);
    w_u32(object + 12, z);
    w_u16(object + 16, (uint16)(ff_rand_8006D348() & 2047u));
    w_u16(object + 32, (uint16)((ff_rand_8006D348() & 15u) + 8u));
    result = (ff_rand_8006D348() & 15u) + 16u;
    w_u16(object + 24, (uint16)result);
    w_u16(object + 26, 0);
    return result;
}

GDB_CALL sint32 FUN_800336CC(sint32 index)
{
    FUNCTION_MARKER(0x800336ccu, "GAME.EXE");
    uint32 actor = 0x800b4318 + 244u * (uint32)index, x, z, state;
    sint32 attacker;
    x = r_u32(actor + 40);
    z = r_u32(actor + 48);
    FUN_80031C50(606, x, 0, z);
    FUN_80029684(index);
    attacker = r_s8(actor + 129);
    state = r_u32(actor + 12);
    FUN_8001F41C((uint32)attacker, (uint32)index, state);
    if (index < 2)
        return 1;
    return FUN_8001B850(r_s8(actor + 129), index);
}

GDB_CALL sint32 FUN_8004298C(void)
{
    FUNCTION_MARKER(0x8004298cu, "GAME.EXE");
    if ((sint32)r_u32(0x80093d60) >= 3)
        return 0;
    if (!FUN_800473B0(0x04300000))
        return 3;
    w_u32(0x80093d60, 3);
    w_u16(r_u32(0x800941d4) + 12, 3);
    w_u32(0x80094338, 607);
    return 607;
}

GDB_CALL uint32 FUN_80041D60(uint32 object)
{
    FUNCTION_MARKER(0x80041d60u, "GAME.EXE");
    uint32 x = (ff_rand_8006D348() & 31u) << 16, z;
    x += r_u32(object + 4);
    z = (ff_rand_8006D348() & 31u) << 16;
    z += r_u32(object + 12);
    return FUN_80031B20(873, x, r_u32(object + 8) - 0x100000u, z, 0, 0, 0);
}

GDB_CALL sint32 FUN_80028C08(void)
{
    FUNCTION_MARKER(0x80028c08u, "GAME.EXE");
    sint32 timer = (sint32)r_u32(0x80094198), x, z, camera_x, camera_z;
    uint32 i, object, type;
    if (timer > 0)
        w_u32(0x80094198, (uint32)timer - 1u);
    for (i = 0; i < 64; i++)
    {
        object = 0x8009a3c8 + 36u * i;
        type = r_u32(object);
        if (type == 460 || type == 462)
        {
            x = r_s16(object + 6);
            z = r_s16(object + 14);
            camera_x = r_s16(0x80094006);
            camera_z = r_s16(0x8009400a);
            if (FUN_800163F4(x, z, camera_x, camera_z) < 32767)
            {
                w_u32(0x80094198, 60);
                w_u32(0x8008e50c, i);
            }
        }
    }
    timer = (sint32)r_u32(0x80094198);
    if (timer <= 0)
        return timer;
    object = 0x8009a3c8 + 36u * r_u32(0x8008e50c);
    x = (sint32)r_u32(object + 4);
    z = (sint32)r_u32(object + 12);
    return FUN_80027DB4(x, z);
}

GDB_CALL sint32 FUN_800291E8(void)
{
    FUNCTION_MARKER(0x800291e8u, "GAME.EXE");
    w_u16(0x8009419c, (uint16)((sint32)(0x04800000u - r_u32(0x80094004)) >> 13));
    FUN_80028C08();
    if ((sint32)r_u32(0x80094198) <= 0)
        FUN_800476E8();
    if ((sint32)r_u32(0x80094668) < (sint32)0xf4c00000u)
        w_u32(0x80094668, 0xf4c00000u);
    return FUN_800474B8();
}

static sint32 camera_floor(uint32 origin, unsigned shift, uint32 floor)
{
    w_u16(0x8009419c, (uint16)((sint32)(origin - r_u32(0x80094004)) >> shift));
    FUN_800476E8();
    if ((sint32)r_u32(0x80094668) < (sint32)floor)
        w_u32(0x80094668, floor);
    return FUN_800474B8();
}

GDB_CALL sint32 FUN_8002804C(void)
{
    FUNCTION_MARKER(0x8002804cu, "GAME.EXE");
    return camera_floor(0, 12, 0xfc400000u);
}

GDB_CALL sint32 FUN_800280B0(void)
{
    FUNCTION_MARKER(0x800280b0u, "GAME.EXE");
    return camera_floor(0, 12, 0xfa300000u);
}

GDB_CALL sint32 FUN_800286A0(void)
{
    FUNCTION_MARKER(0x800286a0u, "GAME.EXE");
    return camera_floor(0, 13, 0xe1100000u);
}

GDB_CALL sint32 FUN_80028BA0(void)
{
    FUNCTION_MARKER(0x80028ba0u, "GAME.EXE");
    return camera_floor(0xff000000u, 15, 0xea200000u);
}

GDB_CALL sint32 FUN_80029180(void)
{
    FUNCTION_MARKER(0x80029180u, "GAME.EXE");
    return camera_floor(0xfe800000u, 13, 0xe6600000u);
}

GDB_CALL sint32 FUN_80029110(void)
{
    FUNCTION_MARKER(0x80029110u, "GAME.EXE");
    sint32 x = (sint32)r_u32(0x800b4528), z = (sint32)r_u32(0x800b4530);
    FUN_80027D68(x, z);
    x = (sint32)r_u32(0x80094660);
    if (x < (sint32)0xef900000u)
        w_u32(0x80094660, 0xef900000u);
    else if ((sint32)r_u32(0x80094660) > 0x0ec00000)
        w_u32(0x80094660, 0x0ec00000);
    return FUN_800474B8();
}

GDB_CALL sint32 FUN_80029090(void)
{
    FUNCTION_MARKER(0x80029090u, "GAME.EXE");
    if (r_s8(0x80093dd9) == 1)
    {
        w_u16(0x8009419c, (uint16)(0x4000u - (uint32)((sint32)(r_u32(0x80094008) + 0x05a00000u) >> 13)));
        FUN_800476E8();
    }
    else
    {
        sint32 x = (sint32)r_u32(0x800b4528), z = (sint32)r_u32(0x800b4530);
        FUN_80027D68(x, z);
    }
    return FUN_800474B8();
}

/* PsyQ strcpy boundary: preserve null handling and byte order for RAM strings. */
static void dialog_copy(uint32 source)
{
    uint32 target = 0x80094848;
    uint8 byte;
    if (!source)
        return;
    do
    {
        byte = r_u8(source++);
        w_u8(target++, byte);
    } while (byte);
}

GDB_CALL uint32 FUN_80051E14(uint32 message, uint32 callback)
{
    FUNCTION_MARKER(0x80051e14u, "GAME.EXE");
    w_u32(0x800927f0, callback);
    dialog_copy(message);
    w_u32(0x800927ec, 0x80051e50);
    return 0x80051e50;
}

GDB_CALL uint32 FUN_8005205C(uint32 message, uint32 kind, uint32 accept, uint32 cancel)
{
    FUNCTION_MARKER(0x8005205cu, "GAME.EXE");
    w_u32(0x800927f8, accept);
    w_u32(0x800927f4, cancel);
    dialog_copy(message);
    w_u8(0x800927fc, (uint8)kind);
    w_u32(0x800927ec, 0x800520ac);
    return 0x800520ac;
}

GDB_CALL uint32 FUN_800523C8(void)
{
    FUNCTION_MARKER(0x800523c8u, "GAME.EXE");
    return FUN_80051E14(0x80092918, 0x80050d98);
}

GDB_CALL uint32 FUN_800523F8(void)
{
    FUNCTION_MARKER(0x800523f8u, "GAME.EXE");
    return FUN_80051E14(0x80092920, 0x80050d98);
}

GDB_CALL uint32 FUN_8006BD9C(uint32 output)
{
    FUNCTION_MARKER(0x8006bd9cu, "GAME.EXE");
    uint32 value = r_u32(0x80093cec);
    if (!value)
        return 0;
    w_u32(output, value);
    return 1;
}

GDB_CALL uint32 FUN_80030A14(uint32 region, sint32 index)
{
    FUNCTION_MARKER(0x80030a14u, "GAME.EXE");
    uint32 actor = 0x800b4318 + 244u * (uint32)index, result;
    sint32 start = r_s16(region + 2);
    sint32 end = r_s16(region + 6), mid = (start + end) >> 1;
    sint32 delta = r_s16(actor + 50) - mid, radius = end - mid, value;
    if (delta < 0)
        delta = -delta;
    value = (sint32)(80u * ((uint32)radius - (uint32)delta));
    if (!radius || (radius == -1 && (uint32)value == 0x80000000u))
        ff_wip_stop(__FUNCTION__, __FILE__, __LINE__);
    value /= radius;
    if (value > 32)
        value = 32;
    result = 0xffffffd0u - (uint32)value;
    w_u8(actor + 159, (uint8)result);
    w_u8(actor + 158, (uint8)result);
    w_u8(actor + 157, (uint8)result);
    result = r_u8(actor + 156) | 1u;
    w_u8(actor + 156, (uint8)result);
    return result;
}

sint32 ff_camera_callback(uint32 target)
{
    switch (target)
    {
        case 0x800289c0:
            return FUN_800289C0();
        case 0x80028114:
            return FUN_80028114();
        case 0x80028d08:
            return FUN_80028D08();
        case 0x80028704:
            return FUN_80028704();
        case 0x800284d4:
            return FUN_800284D4();
        case 0x80028874:
            return FUN_80028874();
        case 0x80028a94:
            return FUN_80028A94();
        case 0x80028f30:
            return FUN_80028F30();
        case 0x8002837c:
            return FUN_8002837C();
        case 0x80027fa4:
            return FUN_80027FA4();
        case 0x800291e8:
            return FUN_800291E8();
        case 0x8002804c:
            return FUN_8002804C();
        case 0x800280b0:
            return FUN_800280B0();
        case 0x800286a0:
            return FUN_800286A0();
        case 0x80028ba0:
            return FUN_80028BA0();
        case 0x80029180:
            return FUN_80029180();
        case 0x80029110:
            return FUN_80029110();
        case 0x80029090:
            return FUN_80029090();
        case 0x80027df8:
            return FUN_80027DF8();
        case 0x8002848c:
            return FUN_8002848C();
        case 0x80028610:
            return FUN_80028610();
        case 0x80028658:
            return FUN_80028658();
        case 0x80028988:
            return FUN_80028988();
        case 0x8002903c:
            return FUN_8002903C();
        case 0x8002926c:
            return FUN_8002926C();
        case 0x80040a48:
            return FUN_80040A48();
        default:
            ff_wip_stop(__FUNCTION__, __FILE__, __LINE__);
            return 0;
    }
}

/* IDA omits the fifth argument loaded from caller SP+10. */
GDB_CALL sint32 FUN_80016748(sint32 kind, uint32 x, uint32 y, uint32 z, uint32 angle)
{
    FUNCTION_MARKER(0x80016748u, "GAME.EXE");
    sint32 slot = FUN_800167C4(kind, x, y, z, angle);
    uint32 object, value = ff_rand_8006D348();
    if (slot >= 0)
    {
        object = 0x8009a3c8 + 36 * (uint32)slot;
        w_u16(object + 24, (uint16)((value & 15) + 50));
    }
    value = ff_rand_8006D348();
    if (slot >= 0)
    {
        w_u16(object + 32, (uint16)((value & 15) + 60));
        w_u16(object + 34, 1);
    }
    return slot;
}

GDB_CALL sint32 FUN_80027D68(sint32 x, sint32 z)
{
    FUNCTION_MARKER(0x80027d68u, "GAME.EXE");
    sint32 camera_x = r_s16(0x80094006), camera_z = r_s16(0x8009400a);
    w_u16(0x8009419c, (uint16)FUN_8001614C(x >> 16, z >> 16, camera_x, camera_z));
    FUN_800476E8();
    return FUN_800474B8();
}

GDB_CALL sint32 FUN_80027DB4(sint32 x, sint32 z)
{
    FUNCTION_MARKER(0x80027db4u, "GAME.EXE");
    sint32 camera_x = r_s16(0x80094006), camera_z = r_s16(0x8009400a);
    w_u16(0x8009419c, (uint16)FUN_8001614C(x >> 16, z >> 16, camera_x, camera_z));
    return FUN_800476E8();
}

GDB_CALL sint32 FUN_8002848C(void)
{
    FUNCTION_MARKER(0x8002848cu, "GAME.EXE");
    w_u16(0x8009419c, (uint16)((sint32)(0xfa000000u - r_u32(0x80094004)) >> 14));
    FUN_800476E8();
    return FUN_800474B8();
}

GDB_CALL sint32 FUN_80028610(void)
{
    FUNCTION_MARKER(0x80028610u, "GAME.EXE");
    w_u16(0x8009419c, (uint16)((sint32)(0xfa000000u - r_u32(0x80094004)) >> 13));
    FUN_800476E8();
    return FUN_800474B8();
}

GDB_CALL sint32 FUN_80028658(void)
{
    FUNCTION_MARKER(0x80028658u, "GAME.EXE");
    w_u16(0x8009419c, (uint16)((sint32)(0xfe000000u - r_u32(0x80094004)) >> 13));
    FUN_800476E8();
    return FUN_800474B8();
}

GDB_CALL sint32 FUN_80028988(void)
{
    FUNCTION_MARKER(0x80028988u, "GAME.EXE");
    sint32 x = (sint32)r_u32(0x800b4528), z = (sint32)r_u32(0x800b4530);
    FUN_80027D68(x, z);
    return FUN_800474B8();
}

GDB_CALL sint32 FUN_8002903C(void)
{
    FUNCTION_MARKER(0x8002903cu, "GAME.EXE");
    w_u32(0x80094664, 0xfec00000u);
    w_u16(0x8009419c, (uint16)((sint32)(0xfe800000u - r_u32(0x80094004)) >> 13));
    FUN_800476E8();
    return FUN_800474B8();
}

GDB_CALL sint32 FUN_8002926C(void)
{
    FUNCTION_MARKER(0x8002926cu, "GAME.EXE");
    sint32 x = (sint32)r_u32(0x800b4528), z = (sint32)r_u32(0x800b4530);
    FUN_80027D68(x, z);
    return FUN_800474B8();
}

GDB_CALL sint32 FUN_80040A48(void)
{
    FUNCTION_MARKER(0x80040a48u, "GAME.EXE");
    w_u16(0x800941a6, 0);
    w_u32(0x80094664, 0xfdc00000u);
    FUN_800476E8();
    w_u16(0x8009419c, 0);
    w_u32(0x80094660, 0xfc000000u);
    return FUN_800474B8();
}

/* IDA split the epilogue into 2C1DC/2C27C. A surviving particle returns
 * its signed age; only the exact age 64 path returns -1. */
static uint32 particle_age64(uint32 object, uint32 increment)
{
    uint32 angle = (uint32)(sint32)r_s16(object + 16), product;
    uint16 age;
    product = (uint32)(sint32)r_s16(object + 24) * (uint32)(sint32)r_s16(0x800802d0 + 2u * angle);
    w_u32(object + 4, r_u32(object + 4) + product);
    angle = (uint32)(sint32)r_s16(object + 16);
    product = (uint32)(sint32)r_s16(object + 24) * (uint32)(sint32)r_s16(0x800806d0 + 2u * angle);
    w_u32(object + 12, r_u32(object + 12) - product);
    age = (uint16)((uint16)r_s16(object + 26) + increment);
    w_u16(object + 26, age);
    if ((sint16)age == 64)
    {
        w_u32(object, 0xffffffffu);
        return 0xffffffffu;
    }
    return (uint32)(sint32)(sint16)age;
}

GDB_CALL uint32 FUN_8002C154(uint32 object)
{
    FUNCTION_MARKER(0x8002c154u, "GAME.EXE");
    return particle_age64(object, 4);
}

GDB_CALL uint32 FUN_8002C1F4(uint32 object)
{
    FUNCTION_MARKER(0x8002c1f4u, "GAME.EXE");
    return particle_age64(object, 2);
}

static uint32 particle_2CD84(uint32 object)
{
    uint32 y = r_u32(object + 8) - ((uint32)(sint32)r_s16(object + 32) << 15);
    w_u32(object + 8, y);
    if ((sint32)y > 0)
    {
        w_u32(object + 8, 0);
        w_u16(object + 32, (uint16)(-(r_s16(object + 32) >> 1)));
    }
    if (r_s16(object + 24) < 0)
        w_u32(object, 0xffffffffu);
    return 0xffffffffu;
}

/* Nonstandard direct suffix entry consumes v1 from its caller. */
GDB_CALL uint32 FUN_8002CD78(uint32 object, uint32 incoming_v1)
{
    FUNCTION_MARKER(0x8002cd78u, "GAME.EXE");
    w_u16(object + 32, (uint16)(incoming_v1 - 6u));
    return particle_2CD84(object);
}

GDB_CALL uint32 FUN_8002CCBC(uint32 object)
{
    FUNCTION_MARKER(0x8002ccbcu, "GAME.EXE");
    uint32 angle = (uint32)(sint32)r_s16(object + 16), product;
    sint32 velocity;
    product = (uint32)(sint32)r_s16(object + 24) * (uint32)(sint32)r_s16(0x800802d0 + 2u * angle);
    w_u32(object + 4, r_u32(object + 4) + (uint32)((sint32)product >> 1));
    angle = (uint32)(sint32)r_s16(object + 16);
    product = (uint32)(sint32)r_s16(object + 24) * (uint32)(sint32)r_s16(0x800806d0 + 2u * angle);
    w_u32(object + 12, r_u32(object + 12) - (uint32)((sint32)product >> 1));
    w_u16(object + 18, (uint16)((uint16)r_s16(object + 18) + 280u));
    w_u16(object + 20, (uint16)((uint16)r_s16(object + 20) + 456u));
    w_u16(object + 24, (uint16)((uint16)r_s16(object + 24) - 4u));
    velocity = r_s16(object + 32);
    if (velocity >= -255)
        w_u16(object + 32, (uint16)(velocity - 6));
    return particle_2CD84(object);
}

/* Search miss intentionally clears the entry immediately beyond the list. */
GDB_CALL uint32 FUN_800292A4(uint32 owner)
{
    FUNCTION_MARKER(0x800292a4u, "GAME.EXE");
    uint32 entry = 0x8009ec70, result = r_u32(0x8009479c);
    sint32 count = (sint32)result, i = 0;
    while (i < count)
    {
        result = r_u32(entry + 20);
        if (result == owner)
            break;
        ++i;
        result = i < count;
        entry += 24;
    }
    w_u16(entry + 12, 0);
    return result;
}

GDB_CALL uint32 FUN_8002AC8C(uint32 object)
{
    FUNCTION_MARKER(0x8002ac8cu, "GAME.EXE");
    w_u16(object + 16, (uint16)r_s16(0x8008d490));
    w_u32(object + 8, r_u32(object + 8) - ((uint32)(sint32)r_s16(object + 32) << 16));
    w_u16(object + 32, (uint16)((uint16)r_s16(object + 32) + 1u));
    if ((sint32)r_u32(object + 8) < -536870912)
        w_u32(object, 0xffffffffu);
    return 0xffffffffu;
}

GDB_CALL uint32 FUN_8002B1E4(uint32 object)
{
    FUNCTION_MARKER(0x8002b1e4u, "GAME.EXE");
    uint16 age = (uint16)((uint16)r_s16(object + 22) + 1u);
    uint32 phase, delta, x, result;
    w_u16(object + 22, age);
    if ((sint16)age == 40)
        w_u32(object, 0xffffffffu);
    phase = (uint32)(sint32)r_s16(object + 22) << 3;
    delta = (uint32)(sint32)r_s16(0x800802d0 + 2u * phase) << 4;
    w_u32(object + 8, r_u32(object + 8) - delta);
    phase <<= 3;
    if ((sint32)phase >= 513)
        phase = 512;
    delta = (uint32)(sint32)r_s16(0x800806d0 + 2u * phase) << 6;
    x = r_u32(object + 4);
    if ((sint32)x > 0)
    {
        delta = 0u - delta;
        x = r_u32(object + 4);
    }
    result = x + delta;
    w_u32(object + 4, result);
    return result;
}

GDB_CALL uint32 FUN_8002D218(uint32 object)
{
    FUNCTION_MARKER(0x8002d218u, "GAME.EXE");
    sint32 timer = r_s16(object + 22);
    uint32 odd;
    if (!timer)
    {
        w_u16(object + 28, (uint16)r_s16(object + 6));
        w_u16(object + 26, (uint16)r_s16(object + 14));
        timer = r_s16(object + 22);
    }
    if (timer > 0)
        w_u16(object + 22, (uint16)(timer - 1));
    odd = (uint16)r_s16(object + 22) & 1u;
    w_u32(object + 4, ((uint32)(sint32)r_s16(object + 28) + (odd << 3)) << 16);
    odd = (uint16)r_s16(object + 22) & 1u;
    w_u32(object + 12, ((uint32)(sint32)r_s16(object + 26) + (odd << 4)) << 16);
    return odd << 4;
}

GDB_CALL uint32 FUN_8002DF00(uint32 object)
{
    FUNCTION_MARKER(0x8002df00u, "GAME.EXE");
    uint16 velocity;
    uint32 result;
    sint32 count;
    w_u32(object + 8, r_u32(object + 8) + ((uint32)(sint32)r_s16(object + 32) << 15));
    velocity = (uint16)((uint16)r_s16(object + 32) + 8u);
    w_u16(object + 32, velocity);
    if ((sint32)r_u32(object + 8) > 0 && (sint16)velocity > 0)
    {
        w_u32(object + 8, 0);
        w_u16(object + 32, (uint16)((-r_s16(object + 32)) >> 2));
        count = r_s16(object + 22);
        if (count >= 0)
        {
            count = (sint16)(count - 1);
            w_u16(object + 22, (uint16)count);
        }
        if (count < 0)
            w_u16(object + 32, 0);
    }
    result = (uint32)((sint32)r_u32(object + 8) >> 13);
    w_u16(object + 18, (uint16)result);
    return result;
}

GDB_CALL uint32 FUN_8002DFA4(uint32 object)
{
    FUNCTION_MARKER(0x8002dfa4u, "GAME.EXE");
    sint32 result;
    w_u32(object + 8, r_u32(object + 8) + ((uint32)(sint32)r_s16(object + 32) << 15));
    w_u16(object + 32, (uint16)((uint16)r_s16(object + 32) + 8u));
    if (r_s16(object + 22) > 0)
    {
        w_u16(object + 18, (uint16)((uint16)r_s16(object + 18) + 64u));
        w_u16(object + 20, (uint16)((uint16)r_s16(object + 20) + 1024u));
    }
    result = (sint32)r_u32(object + 8) > -16777216;
    if (result)
    {
        result = r_s16(object + 32);
        if (result > 0)
        {
            w_u32(object + 8, 0xff000000u);
            w_u16(object + 32, 0);
            w_u16(object + 22, 0);
        }
    }
    return (uint32)result;
}

GDB_CALL uint32 FUN_80030C50(uint32 region, sint32 index)
{
    FUNCTION_MARKER(0x80030c50u, "GAME.EXE");
    uint32 actor = 0x800b4318 + 244u * (uint32)index, result = (uint32)r_u16(actor + 56) - 128u < 1793u;
    if (!result)
    {
        result = r_u16(actor + 102) | 1u;
        w_u16(actor + 102, (uint16)result);
        w_u32(actor + 108, region);
        w_u16(actor + 104, 0);
    }
    return result;
}

GDB_CALL uint32 FUN_80030CA8(uint32 region, sint32 index)
{
    FUNCTION_MARKER(0x80030ca8u, "GAME.EXE");
    uint32 actor = 0x800b4318 + 244u * (uint32)index, result = (uint32)r_u16(actor + 56) - 897u < 255u;
    if (result)
    {
        w_u16(actor + 102, r_u16(actor + 102) | 2u);
        w_u32(actor + 108, region);
        w_u16(actor + 104, 1024);
        result = 1024;
    }
    return result;
}

GDB_CALL uint32 FUN_8003F388(uint32 object)
{
    FUNCTION_MARKER(0x8003f388u, "GAME.EXE");
    uint32 angle = ((uint16)r_s16(object + 16) >> 5) * 2u, speed = (uint32)(sint32)r_s16(object + 24), delta, result, z;
    delta = speed * (uint32)(sint32)r_s16(0x800802d0 + angle);
    speed = (uint32)(sint32)r_s16(object + 24);
    w_u32(object + 4, r_u32(object + 4) + delta);
    delta = speed * (uint32)(sint32)r_s16(0x800806d0 + angle);
    result = (uint16)r_s16(object + 24) - 8u;
    z = r_u32(object + 12);
    w_u16(object + 24, (uint16)result);
    result <<= 16;
    w_u32(object + 12, z - delta);
    if ((sint32)result < 0)
        w_u16(object + 24, 0);
    return result;
}

GDB_CALL uint32 FUN_8003F410(uint32 object)
{
    FUNCTION_MARKER(0x8003f410u, "GAME.EXE");
    sint32 state = r_s16(object + 26);
    uint32 result, y;
    uint16 count;
    if (!state)
    {
        y = r_u32(object + 8);
        w_u32(object + 4, r_u32(object + 4) - 0x780000u);
        if ((sint32)y < 0)
        {
            w_u32(object + 8, y + 0xf0000u);
            w_u16(object + 18, 0);
        }
        result = (sint32)r_u32(object + 4) > 0x1c200000;
        if (result && (sint32)r_u32(object + 8) >= 0)
        {
            w_u16(object + 26, 1);
            result = 500;
            w_u16(object + 22, 500);
        }
        return result;
    }
    if (state != 1)
        return 1;
    count = (uint16)((uint16)r_s16(object + 22) - 1u);
    w_u16(object + 22, count);
    if (count)
        return 0x1c200000;
    w_u32(object + 4, 0x1c200000);
    w_u32(object + 12, 0x1c200000);
    w_u16(object + 26, 0);
    w_u32(object + 8, 0xfc6b0000u);
    w_u16(object + 18, 0xf000);
    return 0xfffff000u;
}

GDB_CALL uint32 FUN_800417EC(uint32 object)
{
    FUNCTION_MARKER(0x800417ecu, "GAME.EXE");
    uint16 age = (uint16)((uint16)r_s16(object + 22) + 1u);
    uint32 result;
    w_u16(object + 22, age);
    if ((sint16)age >= 56)
        w_u16(object + 22, 55);
    w_u32(object + 4, r_u32(object + 4) + ((uint32)(sint32)r_s16(object + 16) << 16));
    w_u32(object + 8, r_u32(object + 8) + ((uint32)(sint32)r_s16(object + 18) << 16));
    w_u32(object + 12, r_u32(object + 12) + ((uint32)(sint32)r_s16(object + 20) << 16));
    if ((sint32)r_u32(object + 8) > 0)
    {
        w_u32(object + 8, 0);
        w_u32(object, 0xffffffffu);
    }
    result = (uint16)r_s16(object + 18) + 1u;
    w_u16(object + 18, (uint16)result);
    return result;
}

GDB_CALL uint32 FUN_800472A8(sint32 x)
{
    FUNCTION_MARKER(0x800472a8u, "GAME.EXE");
    uint32 result = 1;
    if (r_u32(0x800940b0) == 1)
        return x < (sint32)r_u32(0x800b4340);
    if (r_s16(0x800b435a) > 0)
        result = (sint32)r_u32(0x800b4340) >= x;
    if (r_s16(0x800b444e) > 0 && (sint32)r_u32(0x800b4434) < x)
        return 0;
    return result;
}

GDB_CALL uint32 FUN_8004732C(sint32 x)
{
    FUNCTION_MARKER(0x8004732cu, "GAME.EXE");
    uint32 result = 1;
    if (r_u32(0x800940b0) == 1)
        return (sint32)r_u32(0x800b4340) < x;
    if (r_s16(0x800b435a) > 0)
        result = x >= (sint32)r_u32(0x800b4340);
    if (r_s16(0x800b444e) > 0 && x < (sint32)r_u32(0x800b4434))
        return 0;
    return result;
}

GDB_CALL uint32 FUN_80047434(sint32 z)
{
    FUNCTION_MARKER(0x80047434u, "GAME.EXE");
    uint32 result = 1;
    if (r_u32(0x800940b0) == 1)
        return (sint32)r_u32(0x800b4348) < z;
    if (r_s16(0x800b435a) > 0)
        result = z >= (sint32)r_u32(0x800b4348);
    if (r_s16(0x800b444e) > 0 && z < (sint32)r_u32(0x800b443c))
        return 0;
    return result;
}

GDB_CALL uint32 FUN_8004EFB8(sint32 index)
{
    FUNCTION_MARKER(0x8004efb8u, "GAME.EXE");
    uint32 actor = 0x800b4318 + 244u * (uint32)index, result = 0xffffffffu;
    if (r_u32(actor + 4) == 0xffffffffu)
    {
        result = r_u32(actor + 232) & 0xfffffffeu;
        w_u32(actor + 232, result);
    }
    return result;
}

static uint32 animate_four_uv(uint32 selection, uint32 table)
{
    uint32 index = (uint32)(sint32)r_s16(table + 2u * (uint32)(sint32)r_s16(selection));
    uint32 uv = 0x800a8768 + 12u * index, i, value;
    for (i = 0; i < 4; i++, uv += 12)
    {
        value = (uint32)(sint32)r_s16(0x800945d8 + 2u * i) + r_u32(0x80094674);
        w_u8(uv + 5, (uint8)value);
        w_u8(uv + 1, (uint8)value);
        value += 31;
        w_u8(uv + 11, (uint8)value);
        w_u8(uv + 9, (uint8)value);
    }
    return 0;
}

GDB_CALL uint32 FUN_800581A8(void)
{
    FUNCTION_MARKER(0x800581a8u, "GAME.EXE");
    return animate_four_uv(0x800af42a, 0x8009ad18);
}

GDB_CALL uint32 FUN_800582E4(void)
{
    FUNCTION_MARKER(0x800582e4u, "GAME.EXE");
    return animate_four_uv(0x800af4a4, 0x8009add8);
}

GDB_CALL uint32 FUN_8001453C(sint32 level)
{
    FUNCTION_MARKER(0x8001453cu, "GAME.EXE");
    frame_prepare();
    FUN_8005C2A0();
    ff_sequence_update_8001D640();
    FUN_80058C14();
    return FUN_8005EE58(level);
}

GDB_CALL sint32 FUN_800149EC(void)
{
    FUNCTION_MARKER(0x800149ecu, "GAME.EXE");
    FUN_80056670(46, 255, 7168);
    FUN_80056670(46, 255, 6912);
    return FUN_80056670(46, 255, 6656);
}

GDB_CALL sint32 FUN_800163AC(uint32 mesh, uint32 first, uint32 second)
{
    FUNCTION_MARKER(0x800163acu, "GAME.EXE");
    uint32 vertices = r_u32(mesh), a = vertices + 8u * first, b = vertices + 8u * second;
    return (sint16)FUN_8001614C(r_s16(a), r_s16(a + 4), r_s16(b), r_s16(b + 4));
}

GDB_CALL sint32 FUN_80016438(uint32 x0, uint32 z0, uint32 x1, uint32 z1)
{
    FUNCTION_MARKER(0x80016438u, "GAME.EXE");
    uint32 dx = x1 - x0, dz = z1 - z0;
    return FUN_80012B5C(dx * dx + dz * dz);
}

GDB_CALL sint32 FUN_8001A03C(uint32 actor, uint32 table, sint32 animation)
{
    FUNCTION_MARKER(0x8001a03cu, "GAME.EXE");
    w_u32(actor + 8, r_u32(actor + 8) ^ 1u);
    return FUN_80019F00(actor, table, animation);
}

GDB_CALL sint32 FUN_8001A4B0(void)
{
    FUNCTION_MARKER(0x8001a4b0u, "GAME.EXE");
    return FUN_80061120();
}

/* 31A28 consumes only a0/a1; IDA spuriously lists a2/a3 as arguments. */
GDB_CALL uint32 FUN_80033E18(sint32 index)
{
    FUNCTION_MARKER(0x80033e18u, "GAME.EXE");
    return FUN_80031A28(index, 578);
}

GDB_CALL uint32 FUN_80033E38(sint32 index)
{
    FUNCTION_MARKER(0x80033e38u, "GAME.EXE");
    return FUN_80031A28(index, 578);
}

GDB_CALL sint32 FUN_80053298(sint32 index)
{
    FUNCTION_MARKER(0x80053298u, "GAME.EXE");
    return FUN_80053558(index);
}

GDB_CALL uint32 FUN_80060374(uint32 name)
{
    FUNCTION_MARKER(0x80060374u, "GAME.EXE");
    return FUN_800611F8(name) + 8u;
}

GDB_CALL uint32 FUN_80042820(uint32 value, uint32 delta)
{
    FUNCTION_MARKER(0x80042820u, "GAME.EXE");
    sint32 sum = (sint32)(value + delta);
    return sum <= 0 ? 0u : sum < 255 ? (uint32)sum : 255u;
}

GDB_CALL uint32 FUN_8002E7C4(uint32 object)
{
    FUNCTION_MARKER(0x8002e7c4u, "GAME.EXE");
    uint32 result = r_u32(object + 12) - 0x00600000u;
    w_u32(object + 12, result);
    if ((sint32)result <= -536870912)
    {
        result = 0x16000000;
        w_u32(object + 12, result);
    }
    return result;
}

/* 8005E190: IDA call order, decoded MIPS arguments/stores/delay slots. */
GDB_CALL uint32 FUN_8005E190(void)
{
    FUNCTION_MARKER(0x8005e190u, "GAME.EXE");
    uint32 v0, v1;
    v0 = FUN_80067108(0x800935c0u, 0x800935ccu); /* 8005E1A8 */
    v1 = FUN_80063900(0x800935d8u, 0x800935e4u); /* 8005E1C0 */
    return v1;
}

/* 8005E1D8: IDA call order, decoded MIPS arguments/stores/delay slots. */
GDB_CALL uint32 FUN_8005E1D8(void)
{
    FUNCTION_MARKER(0x8005e1d8u, "GAME.EXE");
    uint32 v0, v1;
    v0 = FUN_80067108(0x800935f0u, 0x800935fcu); /* 8005E1F0 */
    v1 = FUN_80063900(0x80093608u, 0x80093614u); /* 8005E208 */
    return v1;
}

/* 8005E220: IDA call order, decoded MIPS arguments/stores/delay slots. */
GDB_CALL uint32 FUN_8005E220(void)
{
    FUNCTION_MARKER(0x8005e220u, "GAME.EXE");
    uint32 v0;
    v0 = FUN_80063900(0x80093620u, 0x8009362cu); /* 8005E238 */
    return v0;
}

/* 8005E250: IDA call order, decoded MIPS arguments/stores/delay slots. */
GDB_CALL uint32 FUN_8005E250(void)
{
    FUNCTION_MARKER(0x8005e250u, "GAME.EXE");
    uint32 v0, v1;
    v0 = FUN_80067108(0x80093638u, 0x80093644u); /* 8005E268 */
    v1 = FUN_80063900(0x80093650u, 0x8009365cu); /* 8005E280 */
    return v1;
}

/* 8005E298: IDA call order, decoded MIPS arguments/stores/delay slots. */
GDB_CALL uint32 FUN_8005E298(void)
{
    FUNCTION_MARKER(0x8005e298u, "GAME.EXE");
    uint32 v0, v1;
    v0 = FUN_80067108(0x80093668u, 0x80093674u); /* 8005E2B0 */
    v1 = FUN_80063900(0x80093680u, 0x8009368cu); /* 8005E2C8 */
    return v1;
}

/* 8005E2E0: IDA call order, decoded MIPS arguments/stores/delay slots. */
GDB_CALL uint32 FUN_8005E2E0(void)
{
    FUNCTION_MARKER(0x8005e2e0u, "GAME.EXE");
    uint32 v0, v1;
    v0 = FUN_80067108(0x80093668u, 0x80093674u); /* 8005E2F8 */
    v1 = FUN_80063900(0x80093698u, 0x800936a4u); /* 8005E310 */
    return v1;
}

/* 8005E328: IDA call order, decoded MIPS arguments/stores/delay slots. */
GDB_CALL uint32 FUN_8005E328(void)
{
    FUNCTION_MARKER(0x8005e328u, "GAME.EXE");
    uint32 v0, v1;
    v0 = FUN_80067108(0x800936b0u, 0x800936bcu); /* 8005E340 */
    v1 = FUN_80063900(0x800936c8u, 0x800936d4u); /* 8005E358 */
    return v1;
}

/* 8005E370: IDA call order, decoded MIPS arguments/stores/delay slots. */
GDB_CALL uint32 FUN_8005E370(void)
{
    FUNCTION_MARKER(0x8005e370u, "GAME.EXE");
    uint32 v0, v1;
    v0 = FUN_80067108(0x800936e0u, 0x800936ecu); /* 8005E388 */
    v1 = FUN_80063900(0x800936f8u, 0x80093704u); /* 8005E3A0 */
    return v1;
}

/* 8005E3B8: IDA call order, decoded MIPS arguments/stores/delay slots. */
GDB_CALL uint32 FUN_8005E3B8(void)
{
    FUNCTION_MARKER(0x8005e3b8u, "GAME.EXE");
    uint32 v0, v1;
    v0 = FUN_80067108(0x800936e0u, 0x800936ecu); /* 8005E3D0 */
    v1 = FUN_80063900(0x80093710u, 0x8009371cu); /* 8005E3E8 */
    return v1;
}

/* 8005E400: IDA call order, decoded MIPS arguments/stores/delay slots. */
GDB_CALL uint32 FUN_8005E400(void)
{
    FUNCTION_MARKER(0x8005e400u, "GAME.EXE");
    uint32 v0, v1;
    v0 = FUN_80067108(0x80093728u, 0x80093734u); /* 8005E418 */
    v1 = FUN_80063900(0x80093740u, 0x8009374cu); /* 8005E430 */
    return v1;
}

/* 8005E448: IDA call order, decoded MIPS arguments/stores/delay slots. */
GDB_CALL uint32 FUN_8005E448(void)
{
    FUNCTION_MARKER(0x8005e448u, "GAME.EXE");
    uint32 v0, v1;
    v0 = FUN_80067108(0x80093758u, 0x80093764u); /* 8005E460 */
    v1 = FUN_80063900(0x80093770u, 0x8009377cu); /* 8005E478 */
    return v1;
}

/* 8005E490: IDA call order, decoded MIPS arguments/stores/delay slots. */
GDB_CALL uint32 FUN_8005E490(void)
{
    FUNCTION_MARKER(0x8005e490u, "GAME.EXE");
    uint32 v0, v1;
    v0 = FUN_80067108(0x80093758u, 0x80093764u); /* 8005E4A8 */
    v1 = FUN_80063900(0x80093788u, 0x80093794u); /* 8005E4C0 */
    return v1;
}

/* 8005E4D8: IDA call order, decoded MIPS arguments/stores/delay slots. */
GDB_CALL uint32 FUN_8005E4D8(void)
{
    FUNCTION_MARKER(0x8005e4d8u, "GAME.EXE");
    uint32 v0, v1;
    v0 = FUN_80067108(0x80093590u, 0x8009359cu); /* 8005E4F0 */
    v1 = FUN_80063900(0x800937a0u, 0x800937acu); /* 8005E508 */
    return v1;
}

/* 8005E520: IDA call order, decoded MIPS arguments/stores/delay slots. */
GDB_CALL uint32 FUN_8005E520(void)
{
    FUNCTION_MARKER(0x8005e520u, "GAME.EXE");
    uint32 v0, v1;
    v0 = FUN_80067108(0x800937b8u, 0x800937c4u); /* 8005E538 */
    v1 = FUN_80063900(0x800937d0u, 0x800937dcu); /* 8005E550 */
    return v1;
}

/* 8005E568: IDA call order, decoded MIPS arguments/stores/delay slots. */
GDB_CALL uint32 FUN_8005E568(void)
{
    FUNCTION_MARKER(0x8005e568u, "GAME.EXE");
    uint32 v0, v1;
    v0 = FUN_80067108(0x800937e8u, 0x800937f4u); /* 8005E580 */
    v1 = FUN_80063900(0x80093800u, 0x8009380cu); /* 8005E598 */
    return v1;
}

/* 8005E5B0: IDA call order, decoded MIPS arguments/stores/delay slots. */
GDB_CALL uint32 FUN_8005E5B0(void)
{
    FUNCTION_MARKER(0x8005e5b0u, "GAME.EXE");
    uint32 v0, v1;
    v0 = FUN_80067108(0x80093818u, 0x80093824u); /* 8005E5C8 */
    v1 = FUN_80063900(0x80093830u, 0x8009383cu); /* 8005E5E0 */
    return v1;
}

/* 8005E5F8: IDA call order, decoded MIPS arguments/stores/delay slots. */
GDB_CALL uint32 FUN_8005E5F8(void)
{
    FUNCTION_MARKER(0x8005e5f8u, "GAME.EXE");
    uint32 v0, v1;
    v0 = FUN_80067108(0x80093818u, 0x80093824u); /* 8005E610 */
    v1 = FUN_80063900(0x80093848u, 0x80093854u); /* 8005E628 */
    return v1;
}

/* 8005E640: IDA call order, decoded MIPS arguments/stores/delay slots. */
GDB_CALL uint32 FUN_8005E640(void)
{
    FUNCTION_MARKER(0x8005e640u, "GAME.EXE");
    uint32 v0, v1;
    v0 = FUN_80067108(0x800937e8u, 0x800937f4u); /* 8005E658 */
    v1 = FUN_80063900(0x80093860u, 0x8009386cu); /* 8005E670 */
    return v1;
}

/* 8005E688: IDA call order, decoded MIPS arguments/stores/delay slots. */
GDB_CALL uint32 FUN_8005E688(void)
{
    FUNCTION_MARKER(0x8005e688u, "GAME.EXE");
    uint32 v0, v1;
    v0 = FUN_80067108(0x800937b8u, 0x800937c4u); /* 8005E6A0 */
    v1 = FUN_80063900(0x80093878u, 0x80093884u); /* 8005E6B8 */
    return v1;
}

/* 8005E6D0: IDA call order, decoded MIPS arguments/stores/delay slots. */
GDB_CALL uint32 FUN_8005E6D0(void)
{
    FUNCTION_MARKER(0x8005e6d0u, "GAME.EXE");
    uint32 v0, v1;
    v0 = FUN_80067108(0x800937e8u, 0x800937f4u); /* 8005E6E8 */
    v1 = FUN_80063900(0x80093890u, 0x8009389cu); /* 8005E700 */
    return v1;
}

/* 8005E718: IDA call order, decoded MIPS arguments/stores/delay slots. */
GDB_CALL uint32 FUN_8005E718(void)
{
    FUNCTION_MARKER(0x8005e718u, "GAME.EXE");
    uint32 v0;
    v0 = FUN_80063900(0x800938a8u, 0x800938b4u); /* 8005E730 */
    return v0;
}

/* 8005E748: IDA call order, decoded MIPS arguments/stores/delay slots. */
GDB_CALL uint32 FUN_8005E748(void)
{
    FUNCTION_MARKER(0x8005e748u, "GAME.EXE");
    uint32 v0, v1;
    v0 = FUN_80067108(0x800938c0u, 0x800938ccu); /* 8005E760 */
    v1 = FUN_80063900(0x800938d8u, 0x800938e4u); /* 8005E778 */
    return v1;
}

/* 8005E790: IDA call order, decoded MIPS arguments/stores/delay slots. */
GDB_CALL uint32 FUN_8005E790(void)
{
    FUNCTION_MARKER(0x8005e790u, "GAME.EXE");
    uint32 v0;
    v0 = FUN_80063900(0x800938f0u, 0x800938fcu); /* 8005E7A8 */
    return v0;
}

/* 8005E808: IDA call order, decoded MIPS arguments/stores/delay slots. */
GDB_CALL uint32 FUN_8005E808(void)
{
    FUNCTION_MARKER(0x8005e808u, "GAME.EXE");
    uint32 v0, v1;
    v0 = FUN_80067108(0x80093668u, 0x80093674u); /* 8005E820 */
    v1 = FUN_80063900(0x80093938u, 0x80093944u); /* 8005E838 */
    return v1;
}

/* 8005E850: IDA call order, decoded MIPS arguments/stores/delay slots. */
GDB_CALL uint32 FUN_8005E850(void)
{
    FUNCTION_MARKER(0x8005e850u, "GAME.EXE");
    uint32 v0, v1;
    v0 = FUN_80067108(0x80093950u, 0x8009395cu); /* 8005E868 */
    v1 = FUN_80063900(0x80093968u, 0x80093974u); /* 8005E880 */
    return v1;
}

/* 8005E898: IDA call order, decoded MIPS arguments/stores/delay slots. */
GDB_CALL uint32 FUN_8005E898(void)
{
    FUNCTION_MARKER(0x8005e898u, "GAME.EXE");
    uint32 v0, v1;
    v0 = FUN_80067108(0x80093980u, 0x8009398cu); /* 8005E8B0 */
    v1 = FUN_80063900(0x80093998u, 0x800939a4u); /* 8005E8C8 */
    return v1;
}

/* 8005E8E0: IDA call order, decoded MIPS arguments/stores/delay slots. */
GDB_CALL uint32 FUN_8005E8E0(void)
{
    FUNCTION_MARKER(0x8005e8e0u, "GAME.EXE");
    uint32 v0, v1;
    v0 = FUN_80067108(0x800939b0u, 0x800939bcu); /* 8005E8F8 */
    v1 = FUN_80063900(0x800939c8u, 0x800939d4u); /* 8005E910 */
    return v1;
}

/* 8005E928: IDA call order, decoded MIPS arguments/stores/delay slots. */
GDB_CALL uint32 FUN_8005E928(void)
{
    FUNCTION_MARKER(0x8005e928u, "GAME.EXE");
    uint32 v0, v1;
    v0 = FUN_80067108(0x800939b0u, 0x800939bcu); /* 8005E940 */
    v1 = FUN_80063900(0x800939e0u, 0x800939ecu); /* 8005E958 */
    return v1;
}

/* 8005E970: IDA call order, decoded MIPS arguments/stores/delay slots. */
GDB_CALL uint32 FUN_8005E970(void)
{
    FUNCTION_MARKER(0x8005e970u, "GAME.EXE");
    uint32 v0, v1;
    v0 = FUN_80067108(0x80093590u, 0x8009359cu); /* 8005E988 */
    v1 = FUN_80063900(0x800939f8u, 0x80093a04u); /* 8005E9A0 */
    return v1;
}

uint32 ff_stage_textures_call(uint32 target)
{
    switch (target)
    {
        case 0x80058374u:
            return FUN_80058374();
        case 0x80058524u:
            return FUN_80058524();
        case 0x8005e148u:
            return FUN_8005E148();
        case 0x8005e7c0u:
            return FUN_8005E7C0();
        case 0x8005e190u:
            return FUN_8005E190();
        case 0x8005e1d8u:
            return FUN_8005E1D8();
        case 0x8005e220u:
            return FUN_8005E220();
        case 0x8005e250u:
            return FUN_8005E250();
        case 0x8005e298u:
            return FUN_8005E298();
        case 0x8005e2e0u:
            return FUN_8005E2E0();
        case 0x8005e328u:
            return FUN_8005E328();
        case 0x8005e370u:
            return FUN_8005E370();
        case 0x8005e3b8u:
            return FUN_8005E3B8();
        case 0x8005e400u:
            return FUN_8005E400();
        case 0x8005e448u:
            return FUN_8005E448();
        case 0x8005e490u:
            return FUN_8005E490();
        case 0x8005e4d8u:
            return FUN_8005E4D8();
        case 0x8005e520u:
            return FUN_8005E520();
        case 0x8005e568u:
            return FUN_8005E568();
        case 0x8005e5b0u:
            return FUN_8005E5B0();
        case 0x8005e5f8u:
            return FUN_8005E5F8();
        case 0x8005e640u:
            return FUN_8005E640();
        case 0x8005e688u:
            return FUN_8005E688();
        case 0x8005e6d0u:
            return FUN_8005E6D0();
        case 0x8005e718u:
            return FUN_8005E718();
        case 0x8005e748u:
            return FUN_8005E748();
        case 0x8005e790u:
            return FUN_8005E790();
        case 0x8005e808u:
            return FUN_8005E808();
        case 0x8005e850u:
            return FUN_8005E850();
        case 0x8005e898u:
            return FUN_8005E898();
        case 0x8005e8e0u:
            return FUN_8005E8E0();
        case 0x8005e928u:
            return FUN_8005E928();
        case 0x8005e970u:
            return FUN_8005E970();
        default:
            ff_wip_stop(__FUNCTION__, __FILE__, __LINE__);
            return 0;
    }
}

GDB_CALL uint32 FUN_8002DA0C(uint32 object)
{
    FUNCTION_MARKER(0x8002da0cu, "GAME.EXE");
    w_u32(object + 4, ((r_u32(0x80093db0) << 5) - 240u) << 16);
    w_u32(object + 8, 0xfe300000u);
    w_u32(object + 12, 0x01a00000u);
    return 0x01a00000u;
}

GDB_CALL uint32 FUN_8002E7F0(uint32 object)
{
    FUNCTION_MARKER(0x8002e7f0u, "GAME.EXE");
    uint32 tick = r_u32(0x80093dd0), height, delta;
    w_u16(object + 16, (uint16)(2u * (uint16)r_s16(0x800802d0 + 2u * (tick & 2047u))));
    height = (uint32)(sint32)r_s16(0x800a7290 + 2u * r_u32(object)) << 16;
    delta = (uint32)(sint32)r_s16(0x800802d0 + ((tick << 5) & 0xfe0u)) << 5;
    w_u32(object + 8, height + delta);
    return delta;
}

GDB_CALL uint32 FUN_8002E870(uint32 object)
{
    FUNCTION_MARKER(0x8002e870u, "GAME.EXE");
    sint32 result = r_s16(0x800802d0 + ((r_u32(0x80093dd0) << 4) & 0xff0u)) >> 1;
    w_u16(object + 16, (uint16)result);
    return (uint32)result;
}

GDB_CALL uint32 FUN_8004348C(uint32 object)
{
    FUNCTION_MARKER(0x8004348cu, "GAME.EXE");
    uint32 angle = (uint16)r_s16(object + 16), delta, result;
    delta = (uint32)(sint32)r_s16(0x800802d0 + 2u * (angle >> 5)) << 8;
    w_u32(object + 4, r_u32(object + 4) + delta);
    angle = ((uint32)(r_s16(object + 16) >> 5) + 512u) & 2047u;
    delta = (uint32)(sint32)r_s16(0x800802d0 + 2u * angle) << 8;
    w_u32(object + 12, r_u32(object + 12) - delta);
    result = (uint16)r_s16(object + 16) + 32u;
    w_u16(object + 16, (uint16)result);
    return result;
}

GDB_CALL uint32 FUN_8004C464(uint32 actor, uint32 action)
{
    FUNCTION_MARKER(0x8004c464u, "GAME.EXE");
    uint32 current = r_u32(actor + 12), table = r_u32(0x800b3be8 + 4u * (uint32)(sint32)r_s16(actor + 52));
    return r_u32(table + 4u * current) == r_u32(table + 4u * action);
}

GDB_CALL uint32 FUN_8005813C(void)
{
    FUNCTION_MARKER(0x8005813cu, "GAME.EXE");
    uint32 index = (uint32)(sint32)r_s16(0x8009ad44 + 2u * (uint32)(sint32)r_s16(0x800af428));
    uint32 uv = 0x800a8768 + 12u * index, value = (uint32)(sint32)r_s16(0x800945d8) + r_u32(0x80094674);
    w_u8(uv + 5, (uint8)value);
    w_u8(uv + 1, (uint8)value);
    value += 31;
    w_u8(uv + 11, (uint8)value);
    w_u8(uv + 9, (uint8)value);
    return value;
}

/* IDA splits each entry at a branch delay slot. Helpers below implement the
 * shared continuations; public suffix entries preserve their own entry path. */
static uint32 particle_2C080(uint32 object)
{
    uint32 angle, delta;
    sint32 vertical;
    uint16 age;
    angle = (uint32)(sint32)r_s16(object + 16);
    delta = (uint32)(sint32)r_s16(object + 24) * (uint32)(sint32)r_s16(0x800802d0 + 2u * angle);
    w_u32(object + 4, r_u32(object + 4) + delta);
    angle = (uint32)(sint32)r_s16(object + 16);
    delta = (uint32)(sint32)r_s16(object + 24) * (uint32)(sint32)r_s16(0x800806d0 + 2u * angle);
    w_u32(object + 12, r_u32(object + 12) - delta);
    vertical = r_s16(object + 32);
    if (vertical >= -255)
        w_u16(object + 32, (uint16)(vertical - 1));
    w_u32(object + 8, r_u32(object + 8) - ((uint32)(sint32)r_s16(object + 32) << 16));
    age = (uint16)((uint16)r_s16(object + 26) + 1u);
    w_u16(object + 26, age);
    if ((sint16)age >= 193)
        w_u32(object, 0xffffffffu);
    return 0xffffffffu;
}

GDB_CALL uint32 FUN_8002C060(uint32 object)
{
    FUNCTION_MARKER(0x8002c060u, "GAME.EXE");
    uint16 speed = (uint16)((uint16)r_s16(object + 24) - 1u);
    w_u16(object + 24, speed);
    if ((sint16)speed < 0)
        w_u16(object + 24, 0);
    return particle_2C080(object);
}

GDB_CALL uint32 FUN_8002C078(uint32 object)
{
    FUNCTION_MARKER(0x8002c078u, "GAME.EXE");
    w_u16(object + 24, 0);
    return particle_2C080(object);
}

static uint32 particle_2CBB4(uint32 object)
{
    uint32 angle, product, y, result;
    sint32 vertical;
    angle = (uint32)(sint32)r_s16(object + 16);
    product = (uint32)(sint32)r_s16(object + 24) * (uint32)(sint32)r_s16(0x800802d0 + 2u * angle);
    w_u32(object + 4, r_u32(object + 4) + (uint32)((sint32)product >> 1));
    angle = (uint32)(sint32)r_s16(object + 16);
    product = (uint32)(sint32)r_s16(object + 24) * (uint32)(sint32)r_s16(0x800806d0 + 2u * angle);
    w_u32(object + 12, r_u32(object + 12) - (uint32)((sint32)product >> 1));
    w_u16(object + 18, (uint16)((uint16)r_s16(object + 18) + 1120u));
    w_u16(object + 20, (uint16)((uint16)r_s16(object + 20) + 1824u));
    w_u16(object + 24, (uint16)((uint16)r_s16(object + 24) - 4u));
    vertical = r_s16(object + 32);
    if (vertical >= -255)
        w_u16(object + 32, (uint16)(vertical - 6));
    y = r_u32(object + 8) - ((uint32)(sint32)r_s16(object + 32) << 15);
    w_u32(object + 8, y);
    if ((sint32)y > 0)
        w_u32(object, 0xffffffffu);
    result = (uint16)r_s16(object + 26) + 1u;
    w_u16(object + 26, (uint16)result);
    return result;
}

GDB_CALL uint32 FUN_8002CBA0(uint32 object)
{
    FUNCTION_MARKER(0x8002cba0u, "GAME.EXE");
    uint32 result;
    if (r_s16(object + 26) < 3)
    {
        result = (uint16)r_s16(object + 26) + 1u;
        w_u16(object + 26, (uint16)result);
        return result;
    }
    return particle_2CBB4(object);
}

GDB_CALL uint32 FUN_8002CBB0(uint32 object)
{
    FUNCTION_MARKER(0x8002cbb0u, "GAME.EXE");
    return particle_2CBB4(object);
}

/* Twin of 2CDE0: X odd offset is 8, Z odd offset is 16. */
static uint32 gate_2CE78(uint32 object, int initialize)
{
    sint32 timer = r_s16(object + 22);
    uint32 odd;
    if (initialize)
    {
        w_u16(object + 28, (uint16)r_s16(object + 6));
        w_u16(object + 26, (uint16)r_s16(object + 14));
        timer = r_s16(object + 22);
    }
    if (timer > 0)
        w_u16(object + 22, (uint16)(timer - 1));
    odd = (uint16)r_s16(object + 22) & 1u;
    w_u32(object + 4, ((uint32)(sint32)r_s16(object + 28) + (odd << 3)) << 16);
    w_u32(object + 12, ((uint32)(sint32)r_s16(object + 26) + (odd << 4)) << 16);
    return odd << 4;
}

GDB_CALL uint32 FUN_8002CE6C(uint32 object)
{
    FUNCTION_MARKER(0x8002ce6cu, "GAME.EXE");
    return gate_2CE78(object, r_s16(object + 22) == 0);
}

GDB_CALL uint32 FUN_8002CE78(uint32 object)
{
    FUNCTION_MARKER(0x8002ce78u, "GAME.EXE");
    return gate_2CE78(object, 1);
}

static uint32 debris_2E050(uint32 object, sint32 speed)
{
    uint32 y, result;
    uint16 vertical;
    sint32 count;
    w_u16(object + 24, (uint16)speed);
    if (r_s16(object + 16) == 16384)
        speed = -speed;
    w_u32(object + 4, r_u32(object + 4) + ((uint32)speed << 16));
    y = r_u32(object + 8) + ((uint32)(sint32)r_s16(object + 32) << 14);
    w_u32(object + 8, y);
    vertical = (uint16)((uint16)r_s16(object + 32) + 10u);
    w_u16(object + 32, vertical);
    if ((sint32)y > 0 && (sint16)vertical > 0)
    {
        w_u32(object + 8, 0);
        w_u16(object + 32, (uint16)((-r_s16(object + 32)) >> 2));
        count = r_s16(object + 22);
        if (count >= 0)
        {
            w_u16(object + 22, (uint16)(count - 1));
            count = (sint16)(count - 1);
        }
        if (count < 0)
            w_u16(object + 32, 0);
    }
    result = (uint32)((sint32)r_u32(object + 8) >> 13);
    w_u16(object + 18, (uint16)result);
    return result;
}

GDB_CALL uint32 FUN_8002E038(uint32 object)
{
    FUNCTION_MARKER(0x8002e038u, "GAME.EXE");
    sint32 speed = r_s16(object + 24) - 4;
    return debris_2E050(object, speed < 0 ? 0 : speed);
}

GDB_CALL uint32 FUN_8002E048(uint32 object)
{
    FUNCTION_MARKER(0x8002e048u, "GAME.EXE");
    return debris_2E050(object, 0);
}

#include <stdio.h>
#include <string.h>

/* 49864: nearest eligible front/rear target. Preserve the original right-hand
 * test: only505/506 extend reach; the signed comparison against -503 is dead. */
GDB_CALL uint32 FUN_80049864(sint32 index)
{
    uint32 actor = 0x800b4318 + 244u * (uint32)index, i, other, type, angle;
    sint32 reach = 0x2400000, cone = 16384, front = 0x7fffffff, rear = 0x7fffffff, slot, dx, dz, distance, mode;
    if (r_s8(actor + 125) != -1)
    {
        reach = 0x3fff0000;
        cone = 8192;
    }
    slot = r_s8(actor + 126);
    if (slot != -1)
    {
        type = r_u32(0x8009a3c8 + 36u * (uint32)slot);
        if (type - 505u < 2u)
        {
            reach = 0x3fff0000;
            cone = 8192;
        }
    }
    w_u32(0x80094118, 0);
    w_u32(0x80094110, 0);
    for (i = 0; i < 8; i++)
    {
        if (i == (uint32)index)
            continue;
        mode = (sint32)r_u32(0x80093d5c);
        if (mode < 2 && i < 2)
            continue;
        if (mode == 2 && i >= 2)
            return 0;
        other = 0x800b4318 + 244u * i;
        slot = r_s16(other + 52);
        if (slot == -1 || slot == 17 || r_s16(other + 66) <= 0 || r_u32(other + 44))
            continue;
        dx = (sint32)(r_u32(actor + 40) - r_u32(other + 40));
        if (dx < 0)
            dx = (sint32)(0u - (uint32)dx);
        dz = (sint32)(r_u32(actor + 48) - r_u32(other + 48));
        if (dz < 0)
            dz = (sint32)(0u - (uint32)dz);
        distance = dz < dx ? (sint32)((uint32)dx + (uint32)(dz >> 1)) : (sint32)((uint32)dz + (uint32)(dx >> 1));
        if (distance >= reach)
            continue;
        angle = (uint32)FUN_8001614C(r_s16(actor + 42), r_s16(actor + 50), r_s16(other + 42), r_s16(other + 50)) - 32u * (uint32)(uint16)r_s16(actor + 56);
        if (distance < front && (sint16)angle < cone && -cone < (sint16)angle)
        {
            front = distance;
            w_u32(0x80094118, 1);
            w_u32(0x80094010, i);
            w_u16(0x8009483c, (uint16)angle);
            w_u32(0x8009418c, (uint32)(distance >> 16));
            w_u32(0x80094110, 0);
        }
        if (!r_u32(0x80094118) && distance < rear && (uint16)(angle + 16383u) >= 32767u)
        {
            rear = distance;
            w_u32(0x80094110, 1);
            w_u32(0x80094010, i);
            w_u16(0x8009410c, (uint16)(angle - 32768u));
        }
    }
    return 0;
}

/* 17FE0: first eligible nearby item, not the nearest. */
GDB_CALL uint32 FUN_80017FE0(sint32 index, sint32 radius)
{
    uint32 actor = 0x800b4318 + 244u * (uint32)index, other = 0x800b4318 + 244u * (1u - (uint32)index), item, type, i;
    sint32 actor_type = r_s16(actor + 52), left = r_s8(actor + 125), right = r_s8(actor + 126);
    sint32 x = (sint32)r_u32(actor + 40) >> 16, z = (sint32)r_u32(actor + 48) >> 16;
    uint32 angle = 2u * (uint32)(sint32)r_s16(actor + 56);
    sint32 fx = (sint32)(r_u32(actor + 40) + 192u * (uint32)(sint32)r_s16(0x800802d0 + angle)) >> 16;
    sint32 fz = (sint32)(r_u32(actor + 48) - 192u * (uint32)(sint32)r_s16(0x800806d0 + angle)) >> 16;
    sint32 selected = -1, limit;
    for (i = 0; i < 64; i++)
    {
        if (i == (uint32)left || i == (uint32)right)
            continue;
        if (r_u32(0x800940b0) == 2 && (i == (uint32)(sint32)r_s8(other + 125) || i == (uint32)(sint32)r_s8(other + 126) || i == r_u32(0x80094550 + 4u * (1u - (uint32)index))))
            continue;
        item = 0x8009a3c8 + 36u * i;
        type = r_u32(item);
        if (type - 587u < 10u && FUN_800163F4(x, z, r_s16(item + 6), r_s16(item + 14)) < radius)
        {
            selected = (sint32)i;
            break;
        }
        if ((r_u32(actor + 124) & 0xffff00) != 0xffff00 || r_s16(item + 22) < 0 || right != -1 || r_u32(item + 8) || type - 477u >= 86u)
            continue;
        if ((sint32)type < 521)
        {
            if (FUN_800163F4(x, z, r_s16(item + 6), r_s16(item + 14)) < radius)
            {
                selected = (sint32)i;
                break;
            }
        }
        else
        {
            limit = radius;
            if (type - 533u < 2u)
                limit = actor_type == 2 ? 144 : 0;
            else if (type == 539 && actor_type != 0 && actor_type != 2)
                limit = 0;
            if (FUN_800163F4(fx, fz, r_s16(item + 6), r_s16(item + 14)) < limit)
            {
                selected = (sint32)i;
                break;
            }
        }
    }
    w_u32(0x80094550 + 4u * (uint32)index, (uint32)selected);
    return 4u * (uint32)index;
}

/* 4A4CC: preserve two original quirks: distance uses X twice, and the
 * candidate comparison keeps the current actor's distance (not the best). */
GDB_CALL uint32 FUN_8004A4CC(void)
{
    FUNCTION_MARKER(0x8004a4cc, "GAME.EXE");
    uint32 i, j, actor, target, other, selected;
    sint32 x, tx, distance;
    for (i = 2; i < 8; i++)
    {
        actor = 0x800b4318 + 244u * i;
        if (r_s16(actor + 52) == -1 || r_u32(actor + 164) - 1u >= 2u)
            continue;
        target = r_u32(actor + 168);
        if (target == 0xffffffffu)
            continue;
        x = r_s16(actor + 42);
        tx = r_s16(0x800b4342 + 244u * target);
        w_u32(0x80093f40 + 4u * i, (uint32)FUN_800163F4(x, x, tx, tx));
    }
    for (i = 2; i < 8; i++)
    {
        actor = 0x800b4318 + 244u * i;
        if (r_s16(actor + 52) == -1 || r_u32(actor + 164) != 1 || r_u32(actor + 168) == 0xffffffffu)
            continue;
        selected = i;
        distance = (sint32)r_u32(0x80093f40 + 4u * i);
        for (j = 2; j < 8; j++)
        {
            other = 0x800b4318 + 244u * j;
            if (j != i && r_s16(other + 52) != -1 && r_u32(other + 164) == 2 && r_u32(other + 168) == r_u32(actor + 168) && (sint32)r_u32(0x80093f40 + 4u * j) < distance)
            {
                distance = (sint32)r_u32(0x80093f40 + 4u * i);
                selected = j;
            }
        }
        if (selected != i)
        {
            w_u32(actor + 164, 2);
            w_u32(0x800b43bc + 244u * selected, 1);
        }
    }
    return 0;
}

/* Original 15720 uses indexed resources and stage-dependent sequence setup */
GDB_CALL sint32 sequence_run(sint32 sequence, void (*prepare_frame)(void))
{
    uint32 old_stage, stage, resource, texture, world, i;
    sint32 sound;
    sint32 track = 8;
    FUNCTION_MARKER(0x80015720, "GAME.EXE");
    sound = r_s16(0x8007a11c + 2 * (uint32)sequence);
    old_stage = r_u32(0x80093d58);
    ff_audit_sequence_context(old_stage);
    FUN_8005ECCC((uint32)sound);
    world = r_u32(0x80093cfc);
    w_u32(0x80093d58, (uint32)sequence + 27);
    w_u32(0x80094844, world);
    FUN_80018318();
    FUN_80013A30();
    FUN_80061448((uint32)sequence + 32, 0x116000);
    resource = r_u32(0x800b3e6c + 4 * (uint32)sequence);
    texture = r_u32(0x800a8744 + 4 * (uint32)sequence);
    w_u32(0x800946bc, resource);
    ff_stage_textures_call(texture);
    FUN_80060394(0);
    stage = r_u32(0x80093d58);
    if (stage == 31 || stage == 33)
    {
        FUN_80056CCC(171);
        stage = r_u32(0x80093d58);
    }
    if (stage == 27)
        FUN_80039578();
    stage = r_u32(0x80093d58);
    if (stage == 28)
    {
        FUN_80039A10();
        stage = r_u32(0x80093d58);
    }
    if (stage == 29)
        FUN_80039C88();
    stage = r_u32(0x80093d58);
    if (stage == 30)
    {
        FUN_80039E18();
        stage = r_u32(0x80093d58);
    }
    if (stage == 31)
        FUN_80039FDC();
    stage = r_u32(0x80093d58);
    if (stage == 32)
    {
        FUN_8003A12C();
        stage = r_u32(0x80093d58);
    }
    if (stage == 33)
        FUN_8003A2C0();
    if (r_u32(0x80093d58) == 34)
        FUN_8003A4D4();
    if (r_u32(0x80093d58) == 35)
        FUN_8003A664();
    FUN_80057630();
    if (r_u32(0x80093d58) == 34 && r_u32(0x8008d4dc))
    {
        FUN_8005A280();
        w_u32(0x80093d58, old_stage);
        return 0;
    }
    stage = r_u32(0x80093d58);
    w_u32(0x8008d4dc, 0);
    if (stage == 34 && FUN_8005A690())
    {
        FUN_8005A280();
        w_u32(0x80093d58, old_stage);
        return 0;
    }
    for (i = 8; i > 0; i--)
        w_u32(0x800b431c + 244 * (i - 1), 0xffffffff);
    stage = r_u32(0x80093d58);
    if (stage == 27)
        track = 6;
    if (stage == 28)
        track = 7;
    if (stage == 29)
        track = 3;
    if (stage == 30)
        track = 8;
    if (stage == 31)
        track = 9;
    if (stage == 32)
        track = 10;
    if (stage == 33)
        track = 11;
    if (stage == 34 || stage == 35)
        track = 4;
    if (stage != 34)
        FUN_8005ED78();
    FUN_800638B8(track);
    if (r_u32(0x80093d58) == 28)
        for (i = 0; i < 13; i++)
            VSync(2);
    FUN_8005ED78();
    FUN_8005EE58(0);
    return ff_sequence_resume_8001559C_stage27(prepare_frame, old_stage);
}

GDB_CALL sint32 ff_sequence_resume_8001559C_stage27(void (*prepare_frame)(void), uint32 old_stage)
{
    uint32 i, yaw, pitch, camera_x, camera_y, camera_z, end_tick, tick;
    sint32 remaining;
    for (;;)
    {
        if (ff_audit_phase_boundary(0x8001559c))
            return 0;
        sequence_step();
        yaw = r_u16(0x8008d490);
        pitch = r_u16(0x8008d492);
        camera_x = r_u32(0x8008d4a4);
        camera_y = r_u32(0x8008d4a8);
        camera_z = r_u32(0x8008d4ac);
        w_u16(0x80093d34, (uint16)yaw);
        w_u16(0x80093d36, (uint16)pitch);
        w_u32(0x80093d28, camera_x);
        w_u32(0x80093d2c, camera_y);
        w_u32(0x80093d30, camera_z);
        prepare_frame();
        FUN_8005C17C();
        FUN_8001EF68();
        FUN_8001EE44();
        if (r_u32(0x80093d58) == 27 && (sint32)r_u32(0x80093dd0) >= 401)
            FUN_8005A8F4();
        stage_update();
        ff_sequence_update_8001D640();
        FUN_8001EDB8();
        FUN_8001ED1C();
        FUN_80058C14();
        end_tick = r_u32(0x800942cc);
        tick = r_u32(0x80093dd0);
        remaining = (sint32)(end_tick - tick);
        if (remaining < 16)
            FUN_8005EE58((sint32)((uint32)remaining << 4));
        for (i = 0; (sint32)i < (sint32)r_u32(0x800940b0); i++)
        {
            uint32 pad = (uint16)FUN_8005F5AC(r_u32(0x80094288 + 4u * i));
            if (pad & 0x4008)
            {
                w_u32(0x8008d4dc, (pad & 8) != 0);
                FUN_8005ED78();
                FUN_80056ECC();
                if (r_u32(0x80093d58) == 34)
                    FUN_8005A280();
                w_u32(0x80093d58, old_stage);
                return 0;
            }
        }
        tick = r_u32(0x80093dd0);
        end_tick = r_u32(0x800942cc);
        if (tick == end_tick)
            break;
    }
    /* Preserve the two 1453C(0) calls through the host frame adapter */
    for (i = 0; i < 2; i++)
    {
        prepare_frame();
        FUN_8005C2A0();
        ff_sequence_update_8001D640();
        FUN_80058C14();
        FUN_8005EE58(0);
    }
    FUN_8005ED78();
    for (i = 0; i < 30; i++)
        VSync(2);
    FUN_80056ECC();
    if (r_u32(0x80093d58) == 34)
        FUN_8005A280();
    w_u32(0x80093d58, old_stage);
    w_u32(0x8008d4dc, 0);
    return 1;
}

GDB_CALL uint32 FUN_8005E148(void)
{
    FUN_80067108(0x80093590, 0x8009359c);
    return FUN_80063900(0x800935a8, 0x800935b4);
}

GDB_CALL sint32 FUN_800638B8(sint32 track)
{
    FUN_80067640(0);
    FUN_80067670(0, track);
    return FUN_800569BC(r_u8(0x80093d6e));
}

/* 5A8F4: characters and shadows become visible from sequence tick401. */
GDB_CALL sint32 FUN_8005A8F4(void)
{
    uint32 i, actor, type, table;
    sint32 result;
    FUN_80011D20();
    FUN_800101CC(r_s16(0x8008d492));
    FUN_8001033C(r_s16(0x8008d490));
    result = (sint32)r_u32(0x800940b0);
    for (i = 0; (sint32)i < (sint32)r_u32(0x800940b0); i++)
    {
        actor = 0x800b4318 + 244u * i;
        type = (uint32)(sint32)r_s16(actor + 52);
        table = r_u32(0x800b3be8 + 4u * type);
        w_u16(actor + 56, 512);
        if (r_u32(actor + 4) == 0xffffffffu)
            FUN_80019F00(actor, table, 69);
        if (r_u32(actor + 4) != 0xffffffffu)
            FUN_8001AEF4((sint32)i);
        FUN_80010040();
        type = (uint32)(sint32)r_s16(actor + 52);
        FUN_800100C4((sint32)(r_u32(actor + 40) - r_u32(0x8008d4a4)), (sint32)(r_u32(actor + 44) + r_u32(0x800990d8 + 4u * type) - r_u32(0x8008d4a8)), (sint32)(r_u32(actor + 48) - r_u32(0x8008d4ac)));
        FUN_8001033C((sint32)(sint16)(0u - 32u * (uint32)(sint32)r_s16(actor + 56)) + 32768);
        FUN_800100C4(0, (sint32)r_u32(actor + 16), (sint32)r_u32(actor + 32));
        FUN_8001060C(r_u32(0x80081720 + 4u * (uint32)(sint32)r_s16(actor + 52)), 0x800ba210 + 504u * i);
        FUN_80010028();
        FUN_80010040();
        FUN_800100C4((sint32)(r_u32(actor + 40) - r_u32(0x8008d4a4)), (sint32)(0u - r_u32(0x8008d4a8)), (sint32)(r_u32(actor + 48) - r_u32(0x8008d4ac)));
        FUN_800101CC(32768);
        FUN_8001033C((sint16)(32u * (uint32)(sint32)r_s16(actor + 56)));
        FUN_80067B8C(r_u32(0x80082490), 0, 0);
        FUN_80010028();
        result = (sint32)(i + 1) < (sint32)r_u32(0x800940b0);
    }
    return result;
}

/* 580AC: two texture V windows, byte wrapping is original SB behavior. */
GDB_CALL uint32 FUN_800580AC(void)
{
    uint32 descriptor = 0x800a8768u + 12u * (uint32)(sint32)r_s16(0x8009acd4u + 2u * (uint32)(sint32)r_s16(0x800af4a4)), i, v;
    for (i = 0; i < 2; i++, descriptor += 12)
    {
        v = (uint32)(sint32)r_s16(0x800945d8 + 2 * i) + r_u32(0x80094674);
        w_u8(descriptor + 5, (uint8)v);
        w_u8(descriptor + 1, (uint8)v);
        w_u8(descriptor + 11, (uint8)(v + 31));
        w_u8(descriptor + 9, (uint8)(v + 31));
    }
    return 0;
}

/* Original 1E168 shared timer and complete stage dispatch */
GDB_CALL uint32 stage_update(void)
{
    uint32 stage = r_u32(0x80093d58), phase, step, base, i, object;
    sint32 actor;
    uint32 y;
    FUNCTION_MARKER(0x8001e168, "GAME.EXE");
    if ((sint32)r_u32(0x80093dfc) > 0)
        w_u32(0x80093dfc, r_u32(0x80093dfc) - 1u);
    if (stage == 0)
        return FUN_800580AC();
    if (stage == 1)
        return FUN_8005813C();
    if (stage == 2)
    {
        phase = r_u32(0x80093da8);
        step = r_u32(0x80093dac);
        if (!(r_u32(0x80093db0) & 1u))
        {
            if (!phase)
            {
                if (!r_u32(0x80093db4))
                {
                    w_u32(0x80093dac, 0);
                    w_u8(0x80093dd9, 1);
                    if (!r_u32(0x80094674))
                    {
                        FUN_80056670(124, 255, 5888);
                        FUN_80056670(234, 255, 6144);
                    }
                    if ((r_u32(0x80093db0) & 2) || (sint32)r_u32(0x80093dd0) >= 61)
                    {
                        uint32 timer = r_u32(0x80094674) + 1;
                        w_u32(0x80094674, timer);
                        if (timer == 32)
                        {
                            w_u8(0x80093dd9, 0);
                            w_u32(0x80093db4, 1);
                        }
                    }
                }
                else if (r_u32(0x80093db4) == 1u)
                {
                    w_u8(0x80093dd9, 0);
                    if ((sint32)r_u32(0x80093dbc) > 0)
                        w_u32(0x80093db4, 2);
                }
                else if (r_u32(0x80093db4) == 2 && (sint32)r_u32(0x80093dd0) >= 61)
                {
                    if (r_u32(0x80094674) == 30u)
                        FUN_80056670(124, 255, 5888);
                    for (actor = 0; actor < 8; actor++)
                        if (!FUN_800196A4(actor) && (sint32)r_u32(0x800b4348 + 244u * (uint32)actor) > 0x02000000)
                            break;
                    if (r_s8(0x80094670) == 1)
                    {
                        if (actor == 8 && (sint32)r_u32(0x80094674) > 0)
                            w_u32(0x80094674, r_u32(0x80094674) - 1u);
                        if (r_s8(0x80093dd9) == 1)
                        {
                            if ((sint32)r_u32(0x80094674) > 0)
                                w_u32(0x80094674, r_u32(0x80094674) - 1u);
                            if (!r_u32(0x80094674))
                            {
                                w_u32(0x80093db4, 0);
                                w_u32(0x80093da8, r_u32(0x80093da8) + 1);
                            }
                        }
                    }
                    else
                    {
                        w_u8(0x800941a4, 1);
                        w_u16(r_u32(0x800941c8) + 12u, 0);
                    }
                }
                phase = r_u32(0x80093da8);
                step = r_u32(0x80093dac);
            }
            if ((sint32)phase > 0 && (sint32)step < 64)
                w_u32(0x80093dac, step + 1);
        }
        else if ((sint32)phase > 2048 && (sint32)step >= 2)
            w_u32(0x80093dac, step - 1);
        step = r_u32(0x80093dac);
        phase = r_u32(0x80093da8) + step;
        w_u32(0x80093da8, phase);
        FUN_80056840((sint32)r_u32(0x800944b8), (sint32)(4u * step + 5120u));
        phase = r_u32(0x80093da8);
        if ((sint32)phase >= 4096)
        {
            phase -= 4096;
            w_u32(0x80093da8, phase);
            w_u32(0x80093db0, r_u32(0x80093db0) + 1u);
        }
        y = ((uint32)((sint32)((phase * 3) << 9) / 4096) - 3600) << 15;
        base = r_u32(0x80094620);
        for (i = 0; i < 4; i++, y += 0x03000000)
        {
            object = 0x800b89f0 + 36u * (base + i);
            w_u32(object + 8, (uint32)y);
            w_u32(object + 8 + 36u * 4u, (uint32)y);
        }
        return FUN_800581A8();
    }
    if (stage == 3)
    {
        uint32 result, side, kind, height;
        if (r_u32(0x80093d60) == 1 && FUN_800473B0(0x00200000))
        {
            FUN_80056670(19, 255, 6144);
            FUN_80056670(200, 255, 6144);
            w_u32(0x80093d60, 2);
            w_u8(0x80093dda, 1);
            w_u32(0x80093ddc, 0);
            for (side = 0; side < 2; side++)
            {
                uint32 x = side ? 0x03000000 : 0xfd000000;
                FUN_80031C50(893, x, 0xfe800000, 0x03c00000);
                FUN_80031C50(893, x, 0xff000000, 0x03c00000);
                FUN_80031C50(893, x, 0xff800000, 0x03c00000);
                FUN_80031C50(603, x, 0, 0x03c00000);
            }
            w_u32(0x800941a8, 16);
            w_u32(0x80094190, 4);
            FUN_80056670(19, 192, 4864);
            for (side = 0; side < 2; side++)
                for (i = 0; i < 16; i++)
                {
                    kind = (ff_rand_8006D348() & 3) + 902;
                    height = (0 - (ff_rand_8006D348() & 255)) << 16;
                    FUN_80031DA8(kind, side ? 0x03000000 : 0xfd000000, height, 0x03c00000, side ? 512 : 1536);
                }
            FUN_80056670(19, 128, 3584);
            w_u8(0x80093dd9, 0);
        }
        result = (uint32)FUN_80058238();
        FUN_800582DC();
        return result;
    }
    if (stage == 4)
        return FUN_8001DDBC();
    if (stage == 6)
    {
        uint32 result, hits = 0;
        FUN_8005862C();
        result = (sint32)r_u32(0x80093dd0) < 61;
        if (result)
            return result;
        result = r_u32(0x80093dd0) & 7;
        if (!r_u32(0x80093d60) || result)
            return result;
        result = r_u32(0x8009403c);
        if ((sint32)result <= 0)
            return result;
        for (i = 0, object = 0x800b89f0;; i++, object += 36)
        {
            if (r_u32(object) - 200 < 2)
            {
                FUN_80056670(19, 255, 6144);
                w_u32(0x800941a8, 16);
                w_u32(0x80094190, 4);
                for (base = 0; base < 4; base++)
                {
                    uint32 kind = (ff_rand_8006D348() & 3) + 902;
                    FUN_80031CD8(kind, r_u32(object + 4), 0, r_u32(object + 12));
                }
                w_u32(object, 55);
                if (++hits >= 5)
                    return 0;
            }
            result = (sint32)(i + 1) < (sint32)r_u32(0x8009403c);
            if (!result)
                return result;
        }
    }
    if (stage == 7)
    {
        uint32 result, timer, next, target;
        if (r_s8(0x800941a4) == 1)
        {
            if (r_u32(0x80094794) == 48)
                w_u16(r_u32(0x800941c8) + 12, 0);
            else
                w_u32(0x80094794, r_u32(0x80094794) + 2);
        }
        if (r_s8(0x80093dd9) != 1)
            return 1;
        phase = r_u32(0x80093d60);
        if (!phase)
        {
            timer = r_u32(0x80094674);
            if (timer)
            {
                timer++;
                w_u32(0x80094674, timer);
                if (timer != 32)
                    return 32;
                result = r_u32(0x800941cc);
                w_u32(0x80093d60, 1);
                w_u16(result + 12, 0);
                return result;
            }
            result = (uint32)FUN_800473B0((sint32)0xf1280000);
            if (result)
            {
                w_u32(0x80094674, 1);
                w_u16(r_u32(0x800941d0) + 12, 3);
                result = (uint32)FUN_80056670(124, 192, 5632);
            }
            return result;
        }
        if (phase == 2 || phase == 4)
        {
            next = phase + 1;
            timer = r_u32(0x80094674);
            if (timer)
            {
                timer++;
                w_u32(0x80094674, timer);
                if (timer == 32)
                {
                    target = r_u32(phase == 2 ? 0x800941dc : 0x800941d4);
                    w_u32(0x80093d60, next);
                    w_u16(target + 12, 0);
                }
                return next;
            }
            result = (uint32)FUN_8004732C(phase == 2 ? 0x07800000 : (sint32)0xf3d00000);
            if (result)
            {
                w_u16(r_u32(phase == 2 ? 0x800941e0 : 0x800941d8) + 12, 3);
                result = (uint32)FUN_80056670(124, 192, 5632);
                if (phase == 4)
                {
                    result = (uint32)FUN_80056670(231, 112, 6144) + 1;
                    w_u32(0x800944b8, result);
                }
                w_u32(0x80094674, 1);
            }
            return result;
        }
        if (phase == 1)
        {
            result = (uint32)FUN_800473B0((sint32)0xf6280000);
            target = 0x800941cc;
        }
        else if (phase == 3)
        {
            result = (uint32)FUN_8004732C(0x04200000);
            target = 0x800941dc;
        }
        else if (phase == 5)
        {
            result = (uint32)FUN_8004732C((sint32)0xf1780000);
            target = 0x800941d4;
        }
        else
            return 5;
        if (result)
        {
            result = r_u32(target);
            w_u32(0x80093d60, phase + 1);
            w_u32(0x80094674, 0);
            w_u8(0x80093dd9, 0);
            w_u16(result + 12, 3);
        }
        return result;
    }
    if (stage == 10)
    {
        uint32 result = (uint32)(sint32)r_s8(0x80093dd9);
        if (result)
        {
            result = r_u32(0x80093d60);
            if (result == 1)
            {
                result = (uint32)FUN_800473B0((sint32)0xe6800000);
                if (result)
                {
                    w_u8(0x80093dd9, 0);
                    w_u32(0x80093d60, 2);
                }
            }
        }
        return result;
    }
    if (stage == 11)
    {
        uint32 result = (uint32)(sint32)r_s8(0x80093dd9);
        if (!result)
            return result;
        result = r_u32(0x80093d60);
        if (result != 1u)
            return result;
        result = 3;
        if (FUN_800473B0(0x07800000))
        {
            object = r_u32(0x800941d0);
            w_u32(0x80093d60, 2);
            w_u8(0x80093dd9, 0);
            w_u16(object + 12, 3);
        }
        return result;
    }
    if (stage - 12u < 2u)
    {
        if (r_s8(0x80093dd9) && !r_u32(0x80093d60) && FUN_800473B0(0))
        {
            object = r_u32(0x800941d0);
            phase = r_u32(0x80093d60);
            w_u8(0x80093dd9, 0);
            w_u16(object + 12, 3);
            w_u32(0x80093d60, phase + 1);
        }
        return FUN_800582E4();
    }
    if (stage == 14)
        return FUN_8001DA84();
    if (stage == 17)
        return (uint32)FUN_80058524();
    if (stage == 20)
    {
        uint32 result = (uint32)(sint32)r_s8(0x80093dd9);
        if (result)
        {
            result = 3;
            if (FUN_8004732C(0x11d00000))
            {
                w_u8(0x80093dd9, 0);
                w_u16(r_u32(0x800941d0) + 12u, 3);
            }
        }
        return result;
    }
    return 20;
}

/* 1D640: shared actor/object update, including stage23 timers and stage1 exit. */
uint32 scene_helicopter_call(uint32 object, uint32 previous_car, uint32 callback_table)
{
    FF_STACK_PROJECTILE projectile;
    if (!previous_car)
        return FUN_800420F8(object, 0);
    /* Nested 4471C to 29AFC saves S0 and S2 into these two retained fields */
    projectile.kind = 499;
    projectile.height = previous_car;
    projectile.angle = (uint16)callback_table;
    projectile.owner = -1;
    return FUN_800420F8(object, &projectile);
}

GDB_CALL uint32 ff_sequence_update_8001D640(void)
{
    FUNCTION_MARKER(0x8001d640u, "GAME.EXE");
    uint32 i, object, target, result, tick = 0, stage, phase, previous_car = 0;
    sint32 count, timer;
    timer = (sint32)r_u32(0x80094190);
    if (timer > 0)
        w_u32(0x80094190, (uint32)timer - 1);
    timer = (sint32)r_u32(0x800941a8);
    if (timer > 0)
        w_u32(0x800941a8, (uint32)timer - 1);
    count = (sint32)r_u32(0x800940b0);
    if (count > 0)
        tick = r_u32(0x80093dd0);
    for (i = 0; (sint32)i < count; i++)
    {
        uint32 pose = 0x800ba210 + 504u * i;
        sint32 goal = 0, value;
        if (r_u32(0x800b4324 + 244u * i) == 69)
            goal = (sint32)((uint32)r_u16(0x800802d0 + ((tick << 5) & 0xfe0)) << 16) >> 22;
        value = (sint32)(r_u32(pose) << 20) >> 20;
        w_u32(pose, ((uint32)value + (uint32)((goal - value) >> 3)) & 0xffeu);
    }
    for (i = 0; i < 8; i++)
    {
        object = 0x800b4318 + 244u * i;
        if (r_s16(object + 52) == -1)
            continue;
        timer = r_s16(object + 74);
        if (timer > 0)
            w_u16(object + 74, (uint16)(timer - 1));
        if (!r_u32(object))
            w_u32(object + 4, 0xffffffffu);
        if (r_u32(object + 4) != 0xffffffffu && r_s16(object + 52) != 17)
            FUN_8001AEF4((sint32)i);
        w_u16(object + 102, 0);
        FUN_80019CA4((sint32)i);
    }
    stage = r_u32(0x80093d58);
    if (!stage)
        w_u32(0x80094470, r_u32(0x80094470) + 1u);
    if (stage == 23)
    {
        uint32 speed;
        if ((sint32)r_u32(0x80093dd0) >= 33)
        {
            speed = r_u32(0x80093dac);
            if ((sint32)speed < 16)
                w_u32(0x80093dac, speed + 1);
        }
        result = r_u32(0x80093db8);
        speed = r_u32(0x80093dac);
        result += speed;
        w_u32(0x80093db8, result);
        if ((sint32)result > 81408)
            w_u32(0x80093db8, 81408);
        result = r_u32(0x80093da8) + speed;
        w_u32(0x80093da8, result);
        if ((sint32)result > 40704)
            w_u32(0x80093da8, 40704);
        if (!r_u32(0x80093d60))
        {
            result = r_u32(0x80093da8);
            if ((sint32)result > 35776)
                w_u32(0x80093da8, result - 6912);
        }
        if (!r_u8(0x80093dd0))
        {
            FUN_80056670(242, 127, (sint32)(ff_rand_8006D348() & 1023u) + 4864);
            w_u16(0x8008e4f0, 30);
        }
    }
    count = (sint32)r_u32(0x8009403c);
    for (i = 0; (sint32)i < count; i++)
    {
        object = 0x800b89f0 + 36u * i;
        target = r_u32(0x800bbf78 + 4u * r_u32(object));
        /* Original 1D730 initializes S3=-1 before this callback loop. */
        if (target == 0x80041b54)
        {
            FUN_80041B54(object, 0xffffffffu);
            previous_car = 0;
        }
        else if (target == 0x800420f8)
        {
            scene_helicopter_call(object, previous_car, 0x800bbf78);
            previous_car = 0;
        }
        else if (target)
        {
            ff_object_call(target, object);
            previous_car = target == 0x8004471c ? object : 0;
        }
        count = (sint32)r_u32(0x8009403c);
    }
    result = 1;
    if (r_s8(0x80093dd9) == 1 && r_u32(0x80093d58) == 1)
    {
        result = r_u32(0x80093d60);
        if (result != 1)
            return result;
        result = (uint32)FUN_80047434((sint32)0xff000000u);
        if (!result)
            return result;
        phase = r_u32(0x80093d60);
        count = (sint32)(r_u32(0x8009403c) - 1u);
        w_u8(0x80093dd9, 0);
        w_u32(0x80093d60, phase + 1);
        w_u32(0x8009403c, (uint32)count);
        for (i = 0; (sint32)i < count; i++)
        {
            object = 0x800b89f0 + 36u * i;
            if (r_u32(object) == 124)
                w_u32(object, 55);
        }
        count = (sint32)r_u32(0x8009479c);
        result = 0x8009ec70;
        for (i = 0; (sint32)i < count; i++)
        {
            object = 0x8009ec7c + 24u * i;
            result = (uint32)(sint32)r_s16(object);
            if (!result)
                w_u16(object, 3);
        }
    }
    return result;
}

/* Normalized twins 30D04/30D60 share the same stores and differ in the
 * unsigned angular window, flag and snapped angle. */
static uint32 sequence_region_angle(uint32 region, sint32 index, uint32 base, uint16 flag, uint16 angle)
{
    uint32 actor = 0x800b4318 + 244u * (uint32)index;
    if ((uint32)r_u16(actor + 56) - base >= 255u)
        return 0;
    w_u16(actor + 102, r_u16(actor + 102) | flag);
    w_u32(actor + 108, region);
    w_u16(actor + 104, angle);
    return angle;
}

GDB_CALL uint32 FUN_80030D04(uint32 region, sint32 index)
{
    return sequence_region_angle(region, index, 385, 4, 512);
}

GDB_CALL uint32 FUN_80030D60(uint32 region, sint32 index)
{
    return sequence_region_angle(region, index, 1409, 8, 1536);
}

/* 3060C radial light region; preserve signed division traps and SB wrap. */
GDB_CALL uint32 FUN_8003060C(uint32 region, sint32 index)
{
    uint32 actor = 0x800b4318 + 244u * (uint32)index, result;
    sint32 radius = r_s16(region), value, x, z, center_x, center_z, distance;
    x = r_s16(actor + 42);
    z = r_s16(actor + 50);
    center_x = r_s16(region + 4);
    center_z = r_s16(region + 6);
    distance = FUN_800163F4(x, z, center_x, center_z);
    value = (sint32)(((uint32)radius - (uint32)distance) << 6);
    if (!radius || (radius == -1 && (uint32)value == 0x80000000))
        ff_wip_stop(__FUNCTION__, __FILE__, __LINE__);
    value /= radius;
    w_u8(actor + 159, (uint8)((value >> 2) - 48));
    w_u8(actor + 158, (uint8)((uint32)value - 48u));
    w_u8(actor + 157, (uint8)((uint32)value - 48u));
    result = r_u8(actor + 156) | 1u;
    w_u8(actor + 156, (uint8)result);
    return result;
}

static uint32 sequence_region_call(uint32 target, uint32 region, sint32 actor)
{
    if (target == 0x80030b94)
        return FUN_80030B94(region, actor);
    if (target == 0x80030ad4)
        return FUN_80030AD4(region, actor);
    if (target == 0x800307ac)
        return FUN_800307AC(region, actor);
    if (target == 0x800306d4)
        return FUN_800306D4(region, actor);
    if (target == 0x80047174)
        return FUN_80047174(region, actor);
    if (target == 0x80030874)
        return FUN_80030874(region, actor);
    if (target == 0x80030944)
        return FUN_80030944(region, actor);
    if (target == 0x80030a14)
        return FUN_80030A14(region, actor);
    if (target == 0x80030c50)
        return FUN_80030C50(region, actor);
    if (target == 0x80030ca8)
        return FUN_80030CA8(region, actor);
    if (target == 0x80030dbc)
        return FUN_80030DBC(region, actor);
    if (target == 0x8003060c)
        return FUN_8003060C(region, actor);
    if (target == 0x80030d04)
        return FUN_80030D04(region, actor);
    if (target == 0x80030d60)
        return FUN_80030D60(region, actor);
    /* Known initializer targets are covered; other target domains remain unresolved */
    ff_wip_stop(__FUNCTION__, __FILE__, __LINE__);
    return 0;
}

GDB_CALL uint32 FUN_80019CA4(sint32 index)
{
    uint32 actor = 0x800b4318 + 244u * (uint32)index, region = 0x8009ec70, result = actor;
    sint32 left = (sint32)r_u32(0x8009479c);
    sint32 x = r_s16(actor + 42);
    sint32 z = r_s16(actor + 50);
    for (; left > 0; left--, region += 24)
    {
        uint32 table, target;
        sint32 shape;
        result = 4u * (uint32)(sint32)r_s16(region + 8);
        table = 0x8009cc98 + result;
        target = r_u32(table);
        if (!target)
            continue;
        shape = r_s16(region + 10);
        if (!shape)
        {
            result = r_s16(region) < x;
            if (!result)
                continue;
            result = x < r_s16(region + 4);
            if (!result)
                continue;
            result = r_s16(region + 2) < z;
            if (!result)
                continue;
            result = z < r_s16(region + 6);
            if (!result)
                continue;
            result = sequence_region_call(target, region, index);
            continue;
        }
        result = 2;
        if (shape == 1)
        {
            uint32 object = r_u32(region + 20);
            uint32 angle = r_u16(object + 16) >> 5;
            uint32 dx = (uint32)(x - r_s16(object + 6));
            sint32 cosine = r_s16(0x800806d0 + 2 * angle);
            uint32 dz = (uint32)(z - r_s16(object + 14));
            sint32 sine = r_s16(0x800802d0 + 2 * angle);
            sint32 width = r_s16(region), depth;
            sint32 rx = (sint32)(dx * (uint32)cosine + dz * (uint32)sine) >> 15;
            sint32 rz = (sint32)(dz * (uint32)cosine - dx * (uint32)sine) >> 15;
            w_u32(0x80094290, (uint32)rx);
            w_u32(0x80094294, (uint32)rz);
            /* Both upper comparisons execute in lower-bound branch delay slots. */
            result = rx < width;
            if (rx <= -width || !result)
                continue;
            depth = r_s16(region + 2);
            result = rz < depth;
            if (rz <= -depth || !result)
                continue;
        }
        else if (shape == 2)
        {
            sint32 center_x = r_s16(region + 4);
            sint32 center_z = r_s16(region + 6);
            sint32 distance = FUN_800163F4(x, z, center_x, center_z);
            sint32 radius = r_s16(region);
            result = distance < radius;
            if (!result)
                continue;
        }
        else
            continue;
        result = sequence_region_call(r_u32(table), region, index);
    }
    return result;
}

/* Original 2CDE0..2CE6C spans the IDA split at branch delay slot 2CDEC */
GDB_CALL uint32 FUN_8002CDE0(uint32 object)
{
    sint32 timer = r_s16(object + 22), coordinate;
    uint32 odd;
    if (!timer)
    {
        w_u16(object + 28, (uint16)r_s16(object + 6));
        w_u16(object + 26, (uint16)r_s16(object + 14));
        timer = r_s16(object + 22);
    }
    if (timer > 0)
        w_u16(object + 22, (uint16)(timer - 1));
    coordinate = r_s16(object + 28);
    odd = r_u16(object + 22) & 1;
    w_u32(object + 4, ((uint32)coordinate + (odd << 4)) << 16);
    coordinate = r_s16(object + 26);
    odd = r_u16(object + 22) & 1;
    w_u32(object + 12, ((uint32)coordinate + (odd << 3)) << 16);
    return odd << 3;
}

GDB_CALL uint32 FUN_8002DA3C(uint32 object)
{
    sint32 angle, velocity, timer;
    uint32 result = 720;
    angle = r_s16(object + 18);
    velocity = r_s16(object + 26);
    velocity += (-angle - velocity) >> 3;
    w_u16(object + 26, (uint16)velocity);
    angle = (sint16)(angle + ((sint16)velocity >> 2));
    w_u16(object + 18, (uint16)angle);
    if (angle > 1024)
        w_u16(object + 18, 1024);
    if (r_s16(object + 18) < -1024)
        w_u16(object + 18, (uint16)-1024);
    angle = r_s16(object + 20);
    velocity = r_s16(object + 22);
    velocity += (-angle - velocity) >> 3;
    w_u16(object + 22, (uint16)velocity);
    angle = (sint16)(angle + ((sint16)velocity >> 2));
    w_u16(object + 20, (uint16)angle);
    if (angle > 1536)
        w_u16(object + 20, 1536);
    if (r_s16(object + 20) < -1536)
        w_u16(object + 20, (uint16)-1536);
    timer = r_s16(object + 32);
    if (timer > 0)
    {
        w_u16(object + 32, (uint16)(timer - 1));
        if (timer == 1)
        {
            FUN_800567F8(r_s16(object + 30) - 1);
            w_u16(object + 30, 0);
        }
    }
    if (r_u32(object) == 720)
    {
        w_u16(object + 16, (uint16)((uint16)r_s16(object + 16) + 256u));
        result = (uint16)r_s16(object + 52) + 256u;
        w_u16(object + 52, (uint16)result);
    }
    return result;
}

/* 4799C: signed upper clamp only; all fixed-point shifts use unsigned bits
 * to preserve MIPS wrapping without shifting negative signed C values. */
GDB_CALL sint32 FUN_8004799C(sint32 frame)
{
    uint32 p, yaw;
    if (frame >= (sint32)r_u32(0x800941c0))
        frame = (sint32)(r_u32(0x800941c0) - 1u);
    p = r_u32(0x80094348) + 2u + 12u * (uint32)frame;
    w_u32(0x8008d4a4, (uint32)(sint32)r_s16(p) << 16);
    w_u32(0x8008d4a8, (0u - (uint32)(sint32)r_s16(p + 2)) << 16);
    w_u32(0x8008d4ac, (0u - (uint32)(sint32)r_s16(p + 4)) << 16);
    w_u16(0x8008d490, (uint16)r_s16(p + 6));
    yaw = 0u - (uint32)(uint16)r_s16(p + 8);
    w_u16(0x8008d492, (uint16)yaw);
    return (sint32)yaw;
}

/* 47A40 does not clamp the frame; its final halfword store is in jr's slot. */
GDB_CALL uint32 FUN_80047A40(sint32 frame, uint32 object, uint32 stream)
{
    uint32 p = r_u32(0x80094348 + 4u * stream) + 2u + 12u * (uint32)frame, result;
    w_u32(object + 4, (uint32)(sint32)r_s16(p) << 16);
    w_u32(object + 8, (0u - (uint32)(sint32)r_s16(p + 2)) << 16);
    w_u32(object + 12, (0u - (uint32)(sint32)r_s16(p + 4)) << 16);
    w_u16(object + 16, (uint16)r_s16(p + 6));
    w_u16(object + 18, (uint16)r_s16(p + 8));
    result = (uint16)r_s16(p + 10);
    w_u16(object + 20, (uint16)result);
    return result;
}

GDB_CALL uint32 FUN_8003A6AC(sint32 frame)
{
    w_u16(0x8008d492, (uint16)((uint32)(uint16)r_s16(0x8008d492) - 1024u));
    w_u32(0x8009403c, 65);
    w_u32(0x800b92f0, 118);
    return FUN_80047A40(frame, 0x800b92f0, 1);
}

/* Original 1559C reloads stage and tick after each sequence callback */
GDB_CALL uint32 sequence_step(void)
{
    uint32 frame;
    FUNCTION_MARKER(0x8001559c, "GAME.EXE");
    FUN_8004799C((sint32)r_u32(0x80093dd0));
    if (r_u32(0x80093d58) == 27)
        FUN_8003A6AC((sint32)r_u32(0x80093dd0));
    if (r_u32(0x80093d58) == 28)
        FUN_8003A700(r_u32(0x80093dd0));
    if (r_u32(0x80093d58) == 29)
        FUN_8003A934((sint32)r_u32(0x80093dd0));
    if (r_u32(0x80093d58) == 30)
        FUN_8003AA00((sint32)r_u32(0x80093dd0));
    if (r_u32(0x80093d58) == 31)
        FUN_8003AA54((sint32)r_u32(0x80093dd0));
    if (r_u32(0x80093d58) == 32)
        FUN_8003AB18(r_u32(0x80093dd0));
    if (r_u32(0x80093d58) == 33)
        FUN_8003AED8(r_u32(0x80093dd0));
    if (r_u32(0x80093d58) == 34)
        FUN_8003B5F0(r_u32(0x80093dd0));
    if (r_u32(0x80093d58) == 35)
        FUN_8003B774();
    frame = r_u32(0x80093dd0) + 1;
    w_u32(0x80093dd0, frame);
    return frame;
}

/* Sequence 0 resources, reached by sequence_run for stage 27 */
GDB_CALL uint32 FUN_80039578(void)
{
    FUNCTION_MARKER(0x80039578, "GAME.EXE");
    uint32 result, arena, size;
    FUN_80061448(67, r_u32(0x80094844));
    arena = r_u32(0x80094844);
    size = r_u32(0x800947d0);
    w_u32(0x80094844, arena + size);
    FUN_8005FF2C(0x800904D0u, 860, 0);  /* SHADOW.MOD */
    FUN_8005FF2C(0x800904DCu, 106, 0);  /* CAR1SHAD.MOD */
    FUN_8005FF2C(0x8008E564u, 56, 7);   /* HQ1.MOD */
    FUN_8005FF2C(0x8008E57Cu, 59, 7);   /* STG1BUSH.MOD */
    FUN_8005FF2C(0x8008E58Cu, 70, 7);   /* STG1GATE.MOD */
    FUN_8005FF2C(0x8008E5FCu, 60, 7);   /* STG1WALL.MOD */
    FUN_8005FF2C(0x8008E60Cu, 614, 7);  /* LAMP.MOD */
    FUN_8005FF2C(0x8008E618u, 615, 7);  /* LAMP2.MOD */
    FUN_8005FF2C(0x8008E624u, 61, 7);   /* STG1BAR.MOD */
    FUN_8005FF2C(0x8008E630u, 618, 7);  /* BUILD1.MOD */
    FUN_8005FF2C(0x8008E63Cu, 619, 7);  /* BUILD2.MOD */
    FUN_8005FF2C(0x8008E648u, 620, 7);  /* BUILD3.MOD */
    FUN_8005FF2C(0x8008E654u, 621, 7);  /* BUILD4.MOD */
    FUN_8005FF2C(0x8008E660u, 622, 7);  /* BUILD5.MOD */
    FUN_8005FF2C(0x8008E66Cu, 623, 7);  /* TREE.MOD */
    FUN_8005FF2C(0x8008E678u, 624, 7);  /* TREE2.MOD */
    FUN_8005FF2C(0x8008E684u, 625, 7);  /* BUSH.MOD */
    FUN_8005FF2C(0x8008E690u, 464, 7);  /* STG1DOOR.MOD */
    FUN_8005FF2C(0x8008E6A0u, 919, 0);  /* CARLIGHT.MOD */
    FUN_8005FF2C(0x8008E6BCu, 94, 70);  /* CAR1BOD1.MOD */
    FUN_8005FF2C(0x8008E6F8u, 93, 70);  /* CAR1WHLS.MOD */
    FUN_8005FF2C(0x8008E720u, 108, 70); /* PCARBOD1.MOD */
    FUN_8005FF2C(0x8008E7C0u, 84, 70);  /* TAXI.MOD */
    FUN_8005FF2C(0x8008E7CCu, 78, 70);  /* PICKUP2.MOD */
    FUN_8005FF2C(0x8008E7D8u, 79, 70);  /* JEEP1.MOD */
    FUN_8005FF2C(0x8008E7E4u, 80, 70);  /* BUS.MOD */
    FUN_8005FF2C(0x8008E7ECu, 81, 70);  /* CARRED1.MOD */
    FUN_8005FF2C(0x8008E7F8u, 82, 70);  /* CARBLUE1.MOD */
    FUN_8005FF2C(0x8008E808u, 83, 70);  /* POLICAR.MOD */
    FUN_8005FF2C(0x8008E894u, 627, 70); /* BARRIER3.MOD */
    FUN_8005FF2C(0x8008E8C4u, 526, 70); /* BOXMETL1.MOD */
    FUN_8005FF2C(0x8008E8E4u, 630, 70); /* PARKLAMP.MOD */
    FUN_8005FF2C(0x8008E8F4u, 633, 70); /* SECURCAM.MOD */
    FUN_8005FF2C(0x8008E904u, 641, 70); /* PRKFENCE.MOD */
    FUN_8005FF2C(0x8008E914u, 113, 70); /* POLCEBAR.MOD */
    FUN_8005FF2C(0x8008E924u, 114, 70); /* 2DCOP1.MOD */
    FUN_8005FF2C(0x8008E930u, 115, 70); /* 2DCOP2.MOD */
    FUN_8005FF2C(0x8008E93Cu, 116, 70); /* 2DCOP3.MOD */
    FUN_8005FF2C(0x8008E948u, 117, 70); /* 2DCOP4.MOD */
    FUN_8005FF2C(0x8008ED34u, 118, 70); /* HELICOP.MOD */
    FUN_8005FF2C(0x8008EED4u, 564, 71); /* ROTOR.MOD */
    FUN_8005FF2C(0x8008FC88u, 834, 8);  /* HELISHAD.MOD */
    FUN_8001721C(0x800904ECu, 0);       /* HAWK.ADT */
    FUN_8001721C(0x800904F8u, 1);       /* MACE.ADT */
    FUN_8001721C(0x80090504u, 2);       /* SMASHER.ADT */
    FUN_8001721C(0x80090510u, 3);       /* ALANA.ADT */
    FUN_80060110(0x8009051Cu, 0, 36);   /* HAWK.ACH */
    FUN_80060110(0x80090528u, 1, 37);   /* MACE.ACH */
    FUN_80060110(0x80090534u, 2, 38);   /* SMASHER.ACH */
    FUN_80060110(0x80090540u, 3, 39);   /* ALANA.ACH */
    w_u32(0x800b4340, 0xfa000000u);
    w_u32(0x800b4348, 0xf2680000u);
    w_u32(0x800b4434, 0xfa180000u);
    w_u32(0x800b443c, 0xf1b00000u);
    w_u32(0x800941c0, 550);
    w_u32(0x800942cc, 550);
    w_u32(0x80094348, FUN_800611F8(0x8009054c));
    result = FUN_800611F8(0x80090558);
    w_u32(0x8009434c, result);
    return result;
}

/* 2D5D4: MIPS MFLO/SRA9 wobble, unsigned angle lookup, signed timer. */
GDB_CALL sint32 FUN_8002D5D4(uint32 object)
{
    sint32 timer = r_s16(object + 22), sine = r_s16(0x800802d0 + (((uint32)timer & 7u) << 9));
    sint32 angle = (sint32)((uint32)sine * (uint32)timer) >> 9;
    w_u16(object + (r_u32(object) - 786u < 2u ? 20 : 18), (uint16)angle);
    sine = r_s16(0x800802d0 + (((uint32)(uint16)r_s16(object + 18) >> 4) & 0x7fe));
    w_u32(object + 8, (0u - (uint32)sine) << 8);
    timer = r_s16(object + 22);
    if (timer > 0)
    {
        --timer;
        w_u16(object + 22, (uint16)timer);
    }
    return timer;
}

/* 1F230: low32 damage products, signed SRA8 and halfword HP wrapping.
 * The final branch delay returns FFFFFFF0 for no grapple partner. */
GDB_CALL uint32 FUN_8001F230(sint32 attacker, sint32 victim, sint32 damage)
{
    uint32 actor = 0x800b4318 + 244 * (uint32)victim, source = 0, amount, table, entry, base, stun, mode;
    uint16 hp;
    sint32 coefficient, difficulty, partner;
    if ((uint32)victim >= 8)
        return 0;
    if (damage > 0)
    {
        source = 0x800b4318 + 244 * (uint32)attacker;
        if (attacker < 2 && victim >= 2)
        {
            entry = r_u32(source + 184);
            base = r_u32(source + 180);
            difficulty = r_s8(0x80093d74);
            coefficient = r_u8(base + 8 * entry + 5);
            coefficient = (sint32)((uint32)coefficient * (256 + ((1 - (uint32)difficulty) << 7))) >> 8;
        }
        else if (attacker >= 2 && victim < 2)
        {
            difficulty = r_s8(0x80093d74);
            entry = r_u32(source + 184);
            base = r_u32(source + 180);
            coefficient = r_u8(base + 8 * entry + 5);
            coefficient = (sint32)((uint32)coefficient * (256 - ((1 - (uint32)difficulty) << 7))) >> 8;
        }
        else
        {
            entry = r_u32(source + 184);
            base = r_u32(source + 180);
            coefficient = r_u8(base + 8 * entry + 5);
        }
        if (coefficient <= 0)
            coefficient = 1;
        amount = (uint32)((sint32)((uint32)damage * (uint32)coefficient) >> 8);
    }
    else
        amount = 0 - (uint32)damage;
    entry = r_u32(actor + 184);
    base = r_u32(actor + 180);
    table = base + 8 * entry;
    amount = (uint32)((sint32)(amount * (uint32)r_u8(table + 7)) >> 8);
    if ((sint32)amount <= 0)
        amount = 1;
    if (source)
        w_u8(source + 116, (uint8)victim);
    stun = r_u16(actor + 74);
    w_u8(actor + 129, (uint8)attacker);
    mode = r_u32(0x80093d5c);
    w_u16(actor + 74, (uint16)(stun + amount));
    if (mode == 2 || victim >= 2 || !(r_u32(0x80093d68) & 1))
    {
        hp = (uint16)(r_u16(actor + 66) - amount);
        w_u16(actor + 66, hp);
        if ((sint16)hp < 0)
            w_u16(actor + 66, 0);
    }
    partner = r_s8(actor + 122);
    if (partner == -1)
        return 0xfffffff0;
    w_u8(0x800b4390 + 244 * (uint32)partner, 255);
    w_u8(actor + 122, 255);
    return 244 * (uint32)partner;
}

/* 17E30: right-hand then left-hand drop, held slots cleared after XYZ stores. */
GDB_CALL uint32 FUN_80017E30(sint32 index)
{
    uint32 actor = 0x800b4318 + 244u * (uint32)index, point[3], item, result = 0xffffffffu, i;
    int slot;
    sint32 held;
    for (slot = 126; slot >= 125; slot--)
    {
        held = r_s8(actor + slot);
        if (held == -1)
            continue;
        FUN_8001B9F4(index, slot == 126 ? 3 : 2, point);
        item = 0x8009a3c8 + 36u * (uint32)held;
        for (i = 0; i < 3; i++)
            w_u32(item + 4 + 4 * i, point[i]);
        w_u8(actor + slot, 255);
        w_u16(item + 24, 4);
        if (slot == 125)
            result = 36u * (uint32)held;
    }
    return result;
}

/* Shared29AFC vehicle-local point. All sums/products wrap before SRA. */
static uint32 vehicle_local_point(uint32 actor, uint32 object, const FF_STACK_VEHICLE *vehicle, sint32 bearing, sint32 *x, sint32 *z)
{
    uint32 offset = (uint32)((sint32)((uint32)bearing << 16) >> 15), dx, dz, sn, cs, position_x, position_z, yaw;
    sint32 object_x, object_z;
    sn = (uint32)(sint32)r_s16(0x800802d0 + offset);
    position_x = r_u32(actor + 40);
    cs = (uint32)(sint32)r_s16(0x800806d0 + offset);
    position_z = r_u32(actor + 48);
    object_x = vehicle ? (sint32)vehicle->x >> 16 : r_s16(object + 6);
    object_z = vehicle ? (sint32)vehicle->z >> 16 : r_s16(object + 14);
    dx = (uint32)((sint32)(position_x + (sn << 4)) >> 16) - (uint32)object_x;
    dz = (uint32)((sint32)(position_z - (cs << 4)) >> 16) - (uint32)object_z;
    yaw = vehicle ? vehicle->angle : r_u16(object + 16);
    offset = 2 * (yaw >> 5);
    sn = (uint32)(sint32)r_s16(0x800802d0 + offset);
    cs = (uint32)(sint32)r_s16(0x800806d0 + offset);
    *x = (sint32)(dx * cs + dz * sn) >> 15;
    *z = (sint32)(dz * cs - dx * sn) >> 15;
    return yaw;
}

/* 29AFC: first eligible actor strictly inside vehicle region; apply hit,
 * then move it until strictly outside. Negative-Z exit returns zero due
 * branch-delay comparison, despite an actual collision. */
static uint32 vehicle_collision_80029AFC(uint32 region, uint32 object, const FF_STACK_VEHICLE *vehicle)
{
    uint32 i, actor, angle, yaw, position;
    sint32 type, x, z, width, depth, bearing, actor_x, actor_z, object_x, object_z, wave;
    for (i = 0; i < 8; i++)
    {
        actor = 0x800b4318 + 244 * i;
        type = r_s16(actor + 52);
        if (type == -1 || type == 17 || r_s16(actor + 66) <= 0 || r_u32(actor + 12) == 147 || r_s16(actor + 80))
            continue;
        actor_x = r_s16(actor + 42);
        actor_z = r_s16(actor + 50);
        object_x = vehicle ? (sint32)vehicle->x >> 16 : r_s16(object + 6);
        object_z = vehicle ? (sint32)vehicle->z >> 16 : r_s16(object + 14);
        bearing = FUN_8001627C(actor_x, actor_z, object_x, object_z);
        yaw = vehicle_local_point(actor, object, vehicle, bearing, &x, &z);
        width = r_s16(region);
        if (x <= -width || x >= width)
            continue;
        depth = r_s16(region + 2);
        if (z <= -depth || z >= depth)
            continue;
        if ((sint32)((((uint32)bearing << 5) - yaw) << 16) > 0)
            angle = (uint32)(sint32)(sint16)yaw + 16384;
        else
            angle = (uint32)(sint32)(vehicle ? (sint16)vehicle->angle : r_s16(object + 16)) - 16384;
        angle >>= 5;
        w_u16(actor + 62, (uint16)angle);
        w_u16(actor + 56, (uint16)angle);
        if (r_u32(0x80093d58) == 18)
            w_u16(actor + 56, 0);
        w_u16(actor + 56, r_u16(actor + 56) & 2047);
        w_u8(actor + 120, 255);
        w_u16(actor + 74, 0);
        FUN_80017E30((sint32)i);
        FUN_8001F230(-1, (sint32)i, -100);
        w_u8(actor + 129, 255);
        FUN_80019F00(actor, r_u32(0x800b3be8 + 4u * (uint32)(sint32)r_s16(actor + 52)), 147);
        FUN_800293D8((sint32)i);
        FUN_80011A50(r_u32(0x80081720 + 4u * (uint32)(sint32)r_s16(actor + 52)), actor, 0x800ba210 + 504 * i);
        w_u32(0x80093d54, 0);
        FUN_8001B2C4((sint32)i);
        for (;;)
        {
            angle = 2 * (uint32)(sint32)r_s16(actor + 56);
            wave = r_s16(0x800802d0 + angle);
            position = r_u32(actor + 40);
            w_u32(actor + 40, position - ((uint32)wave << 7));
            wave = r_s16(0x800806d0 + angle);
            position = r_u32(actor + 48) + ((uint32)wave << 7);
            w_u32(actor + 48, position);
            actor_x = r_s16(actor + 42);
            object_x = vehicle ? (sint32)vehicle->x >> 16 : r_s16(object + 6);
            object_z = vehicle ? (sint32)vehicle->z >> 16 : r_s16(object + 14);
            bearing = FUN_8001627C(actor_x, (sint32)position >> 16, object_x, object_z);
            vehicle_local_point(actor, object, vehicle, bearing, &x, &z);
            width = r_s16(region);
            if (x < -width || x > width)
                return 1;
            depth = r_s16(region + 2);
            if (z < -depth)
                return depth < z;
            if (z > depth)
                return 1;
        }
    }
    return 0;
}

GDB_CALL uint32 FUN_80029AFC(uint32 region, uint32 object)
{
    return vehicle_collision_80029AFC(region, object, NULL);
}

GDB_CALL uint32 ff_stack_vehicle_collision(uint32 region, const FF_STACK_VEHICLE *vehicle)
{
    return vehicle_collision_80029AFC(region, 0, vehicle);
}

/* 2F59C moving traffic. Original region search may finish one-past-end;
 * preserve address progression. Three ordered RNG calls only on wrapping. */
GDB_CALL uint32 FUN_8002F59C(uint32 object)
{
    uint32 region = 0x8009ec70, i, x, step;
    sint32 count = (sint32)r_u32(0x8009479c);
    int wrap;
    for (i = 0; (sint32)i < count; i++, region += 24)
        if (r_u32(region + 20) == object)
            break;
    if (r_s16(object + 16) == 16384)
    {
        step = (uint32)(sint32)r_s16(object + 24) << 16;
        x = r_u32(object + 4) - step;
        w_u32(object + 4, x);
        wrap = (sint32)x < -1073741824;
        if (wrap)
            w_u32(object + 4, ((ff_rand_8006D348() & 8191) + 16384u) << 16);
    }
    else
    {
        step = (uint32)(sint32)r_s16(object + 24) << 16;
        x = r_u32(object + 4) + step;
        w_u32(object + 4, x);
        wrap = (sint32)x > 1073741824;
        if (wrap)
            w_u32(object + 4, (0xffffc000u - (ff_rand_8006D348() & 8191)) << 16);
    }
    if (wrap)
    {
        w_u32(object, (uint32)(sint32)r_s16(0x8008e518 + 2u * (ff_rand_8006D348() & 15)));
        w_u16(object + 24, (uint16)(128 + (ff_rand_8006D348() & 63)));
        FUN_8003B77C(region, (sint32)r_u32(object));
        w_u32(0x8009479c, r_u32(0x8009479c) - 1u);
    }
    return FUN_80029AFC(region, object);
}

GDB_CALL sint32 FUN_800196A4(sint32 index)
{
    uint32 actor = 0x800b4318 + 244u * (uint32)index;
    if (index < 2)
    {
        if (r_s16(actor + 52) == -1)
            return 1;
        if (r_s16(actor + 66) != 0)
            return 0;
    }
    if (r_s16(actor + 52) == -1)
        return 1;
    return r_s16(actor + 66) <= 0;
}

GDB_CALL sint32 FUN_80047600(void)
{
    uint32 mask = (r_s16(0x800b435a) > 0 ? 1u : 0u) + (r_s16(0x800b444e) > 0 ? 2u : 0u), actor;
    sint32 result;
    w_u32(0x80094840, 0);
    if (mask - 1u < 2u)
    {
        actor = 0x800b4318 + 244u * (mask - 1u);
        w_u32(0x80094004, r_u32(actor + 40));
        result = (sint32)r_u32(actor + 48);
        w_u32(0x80094008, (uint32)result);
        return result;
    }
    if (mask != 3)
        return 3;
    w_u32(0x80094004, ((uint32)(sint32)r_s16(0x800b4342) + (uint32)(sint32)r_s16(0x800b4436)) << 15);
    w_u32(0x80094008, ((uint32)(sint32)r_s16(0x800b434a) + (uint32)(sint32)r_s16(0x800b443e)) << 15);
    result = FUN_800163F4(r_s16(0x800b4342), r_s16(0x800b434a), r_s16(0x800b4436), r_s16(0x800b443e));
    w_u32(0x80094840, (uint32)result);
    return result;
}

GDB_CALL sint32 FUN_800474B8(void)
{
    uint32 target_x = r_u32(0x80094660);
    uint32 x = r_u32(0x8008d4a4);
    uint32 vx = r_u32(0x800942c8);
    uint32 target_y = r_u32(0x80094664);
    uint32 y = r_u32(0x8008d4a8);
    uint32 target_z = r_u32(0x80094668);
    uint32 z = r_u32(0x8008d4ac);
    uint32 vy = r_u32(0x800942d0);
    uint32 vz = r_u32(0x800942d8);
    uint32 target_yaw, yaw, target_pitch, pitch;
    sint32 result;
    vx += (uint32)((sint32)(target_x - x - vx) >> 1);
    w_u32(0x800942c8, vx);
    x += (uint32)((sint32)vx >> 2);
    vy += (uint32)((sint32)(target_y - y - vy) >> 4);
    w_u32(0x800942d0, vy);
    y += (uint32)((sint32)vy >> 5);
    target_yaw = r_u16(0x8009419c);
    yaw = r_u16(0x8008d490);
    vz += (uint32)((sint32)(target_z - z - vz) >> 1);
    w_u32(0x800942d8, vz);
    z += (uint32)((sint32)vz >> 2);
    target_pitch = r_u16(0x800941a6);
    pitch = r_u16(0x8008d492);
    w_u32(0x8008d4a4, x);
    w_u32(0x80094334, (uint32)((sint32)(0 - x) >> 12));
    w_u32(0x8008d4a8, y);
    w_u32(0x8008d4ac, z);
    w_u32(0x8009433c, (uint32)((sint32)y >> 12));
    w_u32(0x80094340, (uint32)((sint32)(0 - z) >> 12));
    yaw += (uint32)((sint32)((target_yaw - yaw) << 16) >> 19);
    result = (sint32)((target_pitch - pitch + 15) << 16) >> 21;
    w_u16(0x8008d490, (uint16)yaw);
    w_u16(0x8008d492, (uint16)(pitch + (uint32)result));
    return result;
}

GDB_CALL sint32 FUN_800476E8(void)
{
    uint32 radius = r_u32(0x80094840) + 1280;
    uint32 angle = r_u16(0x8008d490) >> 5;
    uint32 actor, diff, bit, x, z, bearing;
    sint32 i, timer, override, sine, cosine, player_x, player_z, actor_x, actor_z;
    int raised;
    if (r_u32(0x80093d58) != 14 && r_u32(0x800940b0) == 1 && r_s8(0x80094104))
        radius = 2048;
    sine = r_s16(0x800802d0 + 2 * angle);
    cosine = r_s16(0x800806d0 + 2 * angle);
    z = r_u32(0x80094008);
    x = r_u32(0x80094004);
    w_u32(0x80094660, x + (uint32)sine * radius);
    w_u32(0x80094668, z - (uint32)cosine * radius);
    for (i = 2; i < 8; i++)
    {
        bit = 1 << i;
        if (FUN_800196A4(i))
        {
            w_u32(0x800944d8, r_u32(0x800944d8) & ~bit);
            continue;
        }
        actor = 0x800b4318 + 244 * (uint32)i;
        player_x = r_s16(0x800b4342);
        player_z = r_s16(0x800b434a);
        actor_x = r_s16(actor + 42);
        actor_z = r_s16(actor + 50);
        bearing = (uint32)FUN_8001614C(player_x, player_z, actor_x, actor_z);
        diff = bearing - r_u16(0x8008d490);
        if ((sint32)r_u32(0x800941a0) < 384 && ((uint16)(diff + 6143) < 12287 || (uint16)(diff + 26624) > 53248))
            w_u32(0x800944d8, r_u32(0x800944d8) | bit);
        if ((sint32)r_u32(0x800941a0) >= 513 || (uint16)(diff - 7169) < 18431 || (uint16)(diff + 25599) < 18431)
            w_u32(0x800944d8, r_u32(0x800944d8) & ~bit);
    }
    if (r_u32(0x800944d8))
    {
        w_u32(0x80094410, 60);
        raised = 1;
    }
    else
    {
        timer = (sint32)r_u32(0x80094410);
        if (timer > 0)
        {
            w_u32(0x80094410, (uint32)timer - 1);
            raised = 1;
        }
        else
            raised = (sint32)r_u32(0x80094198) < 0;
    }
    if (raised)
    {
        if (r_u32(0x80093d58) == 14)
        {
            w_u32(0x80094664, 0xfec00000);
            w_u16(0x800941a6, 3584);
        }
        else
        {
            w_u32(0x80094664, 0xfe400000);
            w_u16(0x800941a6, 4608);
        }
    }
    else
    {
        w_u32(0x80094664, 0xfec00000);
        w_u16(0x800941a6, 3072);
    }
    override = (sint32)r_u32(0x80094198);
    if (override > 0)
    {
        w_u32(0x80094664, 0xfec00000);
        w_u16(0x800941a6, (uint16) override);
    }
    return (sint32)0xfec00000;
}

/* 473B0: one-player strict boundary; two-player living actors use inclusive Z. */
GDB_CALL sint32 FUN_800473B0(sint32 z)
{
    sint32 result = 1;
    if (r_u32(0x800940b0) == 1)
        return z < (sint32)r_u32(0x800b4348);
    if (r_s16(0x800b435a) > 0)
        result = (sint32)r_u32(0x800b4348) >= z;
    if (r_s16(0x800b444e) > 0 && (sint32)r_u32(0x800b443c) < z)
        return 0;
    return result;
}

GDB_CALL sint32 FUN_80027DF8(void)
{
    uint32 mode = r_u32(0x80093d60);
    sint32 x, z, camera_x, camera_z;
    if (!mode)
    {
        x = 2240;
        z = ((sint32)(((0xf9000000u - r_u32(0x800b4348)) << 1) + 0xf9000000u)) >> 16;
    }
    else if ((mode = r_u32(0x80093d60)) == 1)
    {
        uint32 i, object;
        camera_x = r_s16(0x80094006);
        camera_z = r_s16(0x8009400a);
        w_u16(0x8009419c, (uint16)FUN_8001614C(256, 3392, camera_x, camera_z));
        FUN_800476E8();
        if (FUN_800473B0((sint32)0xfa000000u) && r_s8(0x80093dd9) == 1)
        {
            w_u32(0x80093d60, r_u32(0x80093d60) + 1u);
            w_u8(0x80093dd9, 0);
            for (i = 0, object = 0x8009a3c8; i < 64; i++, object += 36)
                if (r_u32(object) != 0xffffffffu && (sint32)r_u32(object + 12) < (sint32)0xf9000000u)
                    w_u32(object, 0xffffffffu);
            w_u16(r_u32(0x800941cc) + 12, 3);
        }
        return FUN_800474B8();
    }
    else if (r_u32(0x80093d60) == 2)
    {
        x = 256;
        z = -1792;
    }
    else if (r_s8(0x800941a4))
    {
        x = 128;
        z = 3392;
    }
    else
    {
        x = 0;
        z = -384;
    }
    camera_x = r_s16(0x80094006);
    camera_z = r_s16(0x8009400a);
    w_u16(0x8009419c, (uint16)FUN_8001614C(x, z, camera_x, camera_z));
    FUN_800476E8();
    return FUN_800474B8();
}

GDB_CALL uint32 FUN_8001697C(uint32 type, uint32 x, uint32 y, uint32 z)
{
    uint32 i, object = 0x8009a3c8, angle;
    for (i = 0; i < 64; i++, object += 36)
        if (r_u32(object) == 0xffffffffu)
        {
            memset(psx_addr(object, 36), 0, 36);
            angle = (uint16)r_s16(0x8008d490);
            w_u32(object, type);
            w_u32(object + 4, x);
            w_u32(object + 8, y);
            w_u32(object + 12, z);
            w_u16(object + 16, (uint16)angle);
            return angle;
        }
    return 0;
}

GDB_CALL sint32 FUN_80017C78(sint32 index)
{
    uint32 actor = 0x800b4318 + 244u * (uint32)index, item, position[3];
    sint32 slot, type;
    slot = r_s8(actor + 126);
    if (slot != -1)
    {
        item = 0x8009a3c8 + 36u * (uint32)slot;
        type = (sint32)r_u32(item);
        if (type >= 522 || type == 512)
        {
            FUN_8001B9F4(index, 3, position);
            w_u32(item + 4, position[0]);
            w_u32(item + 8, position[1]);
            w_u32(item + 12, position[2]);
            w_u8(actor + 126, 255);
            w_u16(item + 24, 4);
        }
    }
    slot = r_s8(actor + 125);
    if (slot == -1)
        return -8;
    item = 0x8009a3c8 + 36u * (uint32)slot;
    if ((sint32)r_u32(item) < 522)
        return 1;
    FUN_8001B9F4(index, 2, position);
    w_u32(item + 4, position[0]);
    w_u32(item + 8, position[1]);
    w_u32(item + 12, position[2]);
    w_u8(actor + 125, 255);
    w_u16(item + 24, 4);
    return 4;
}

/* 301CC: projectile kind and speed depend on the held item; exhausted pool is silent. */
GDB_CALL sint32 FUN_800301CC(sint32 index)
{
    FUNCTION_MARKER(0x800301ccu, "GAME.EXE");
    uint32 actor = 0x800b4318 + 244u * (uint32)index, held, p, point[3], kind;
    held = 0x8009a3c8 + 36u * (uint32)(sint32)r_s8(actor + 126);
    p = FUN_800165EC();
    if (!p)
        return 0;
    FUN_80056670(21, 255, 6144);
    FUN_8001B9F4(index, 3, point);
    kind = r_u32(held) == 505 ? 574u : 572u;
    w_u32(p, kind);
    w_u32(p + 4, point[0]);
    w_u32(p + 8, point[1]);
    w_u32(p + 12, point[2]);
    w_u16(p + 18, 0);
    w_u16(p + 20, 0);
    w_u16(p + 16, (uint16)((uint32)(uint16)r_s16(actor + 56) << 5));
    w_u16(p + 24, kind == 574 ? 128 : 256);
    w_u16(p + 26, 0);
    w_u16(p + 32, 0);
    w_u16(p + 28, (uint16)index);
    w_u16(p + 26, r_u32(0x80094118) == 1 ? (uint16)r_u32(0x80094010) : 65535);
    return 0;
}

/* 4150C: nine actual arguments, not the thirteen inferred by IDA. */
GDB_CALL sint32 FUN_8004150C(uint32 style, uint32 x, uint32 y, uint32 z, uint32 end_x, uint32 end_y, uint32 end_z, uint32 size, uint32 color)
{
    FUNCTION_MARKER(0x8004150cu, "GAME.EXE");
    uint32 p = FUN_80030F44();
    sint32 dz;
    if (!p)
        return 0;
    memset(psx_addr(p, 36), 0, 36);
    w_u32(p, 878);
    w_u16(p + 22, (uint16)style);
    w_u32(p + 24, size);
    w_u32(p + 28, color);
    w_u32(p + 4, x);
    w_u32(p + 8, y);
    w_u32(p + 12, z);
    w_u16(p + 16, (uint16)((sint32)(end_x - x) >> 16));
    w_u16(p + 18, (uint16)((sint32)(end_y - y) >> 16));
    dz = (sint32)(end_z - z) >> 16;
    w_u16(p + 20, (uint16)dz);
    return dz;
}

GDB_CALL uint32 FUN_80040CF4(sint32 index)
{
    FUNCTION_MARKER(0x80040cf4u, "GAME.EXE");
    uint32 actor = 0x800b4318 + 244u * (uint32)index, v = r_u32(actor + 232) & 2u, offset, start[3], end[3], mask;
    if (v)
        return v;
    offset = 16u * (ff_rand_8006D348() & 15u);
    FUN_8001B9F4(index, (sint32)r_u32(0x8007f014 + offset), start);
    FUN_8001B9F4(index, (sint32)r_u32(0x8007f018 + offset), end);
    FUN_8004150C(r_u32(0x8007f01c + offset), start[0], start[1], start[2], end[0], end[1], end[2], 0x02000200u, 0x00c0c0c0u);
    v = r_u32(actor + 156);
    mask = r_u32(0x8007f020 + offset);
    v |= mask;
    w_u32(actor + 156, v);
    return v;
}

/* 165EC: zero the first free item and return its PSX address, not its index. */
GDB_CALL uint32 FUN_800165EC(void)
{
    FUNCTION_MARKER(0x800165ecu, "GAME.EXE");
    uint32 i, byte, p = 0x8009a3c8;
    for (i = 0; i < 64; i++, p += 36)
        if (r_u32(p) == 0xffffffffu)
        {
            for (byte = 0; byte < 36; byte++)
                w_u8(p + byte, 0);
            return p;
        }
    return 0;
}

/* 168BC: angle is the fifth argument at entry SP+16 (IDA calls it a9). */
GDB_CALL sint32 FUN_800168BC(uint32 kind, uint32 x, uint32 y, uint32 z, uint32 angle)
{
    FUNCTION_MARKER(0x800168bcu, "GAME.EXE");
    sint32 i;
    uint32 p = 0x8009a3c8, byte;
    for (i = 0; i < 64; i++, p += 36)
        if (r_u32(p) == 0xffffffffu)
        {
            for (byte = 0; byte < 36; byte++)
                w_u8(p + byte, 0);
            w_u32(p, kind);
            w_u32(p + 4, x);
            w_u32(p + 8, y);
            w_u32(p + 12, z);
            w_u16(p + 16, (uint16)angle);
            return i;
        }
    return -1;
}

GDB_CALL uint32 FUN_80017F88(uint32 object, sint32 index, sint32 attachment)
{
    FUNCTION_MARKER(0x80017f88u, "GAME.EXE");
    uint32 point[3];
    FUN_8001B9F4(index, attachment, point);
    w_u32(object + 4, point[0]);
    w_u32(object + 8, point[1]);
    w_u32(object + 12, point[2]);
    return point[2];
}

/* 16654: item pool0..63, distinct from the effect pool30F44. */
GDB_CALL sint32 FUN_80016654(uint32 kind, uint32 x, uint32 y, uint32 z)
{
    FUNCTION_MARKER(0x80016654, "GAME.EXE");
    sint32 index;
    uint32 p = 0x8009a3c8, byte;
    for (index = 0; index < 64; index++, p += 36)
    {
        if (r_u32(p) != 0xffffffffu)
            continue;
        for (byte = 0; byte < 36; byte++)
            w_u8(p + byte, 0);
        w_u32(p, kind);
        w_u32(p + 4, x);
        w_u32(p + 8, y);
        w_u32(p + 12, z);
        w_u16(p + 16, r_u16(0x8008d490));
        if (kind == 869)
            w_u16(p + 22, (uint16)(ff_rand_8006D348() & 3u));
        if (kind - 587u < 10u)
            w_u16(p + 22, 300);
        return index;
    }
    return -1;
}

/* 30F44 scans slots1..111; slot0 and112..127 are not available here. */
GDB_CALL uint32 FUN_80030F44(void)
{
    FUNCTION_MARKER(0x80030f44, "GAME.EXE");
    uint32 i, byte, p = 0x8009d1f4;
    for (i = 1; i < 112; i++, p += 36)
        if (r_u32(p) == 0xffffffffu)
        {
            for (byte = 0; byte < 36; byte++)
                w_u8(p + byte, 0);
            return p;
        }
    return 0;
}

/* 30FAC: rotating 16-entry pool; always clears the chosen slot. */
GDB_CALL uint32 FUN_80030FAC(void)
{
    FUNCTION_MARKER(0x80030facu, "GAME.EXE");
    uint32 i = (r_u32(0x8008e538) + 1u) & 15u, p, byte;
    w_u32(0x8008e538, i);
    p = 0x8009e190 + 36u * i;
    for (byte = 0; byte < 36; byte++)
        w_u8(p + byte, 0);
    return p;
}

/* 315A4: fifth word argument is angle; return is the third RNG low six bits. */
GDB_CALL uint32 FUN_800315A4(uint32 kind, uint32 x, uint32 y, uint32 z, uint32 angle)
{
    FUNCTION_MARKER(0x800315a4u, "GAME.EXE");
    uint32 p = FUN_80030F44(), v;
    if (!p)
        return 0;
    w_u32(p, kind);
    w_u32(p + 4, x);
    w_u32(p + 8, y);
    w_u32(p + 12, z);
    w_u16(p + 16, (uint16)((angle + (ff_rand_8006D348() & 255u) - 128u) & 2047u));
    w_u16(p + 24, (uint16)((ff_rand_8006D348() & 63u) + 128u));
    v = ff_rand_8006D348() & 63u;
    w_u16(p + 32, (uint16)(64u - v));
    w_u16(p + 26, 0);
    return v;
}

/* 31C50: allocate and initialize one generic effect, including zero-fill. */
GDB_CALL uint32 FUN_80031C50(uint32 kind, uint32 x, uint32 y, uint32 z)
{
    uint32 p = FUN_80030F44(), i;
    if (!p)
        return 0;
    for (i = 0; i < 36; ++i)
        w_u8(p + i, 0);
    w_u32(p, kind);
    w_u32(p + 4, x);
    w_u32(p + 8, y);
    w_u32(p + 12, z);
    return p;
}

/* Shared actual3100C/310B8 structure, constants retain each variant's RNG range. */
static uint32 hit_particle(uint32 x, uint32 y, uint32 z, uint32 angle, int reverse, uint32 kind, uint32 exhausted)
{
    uint32 p = FUN_80030F44(), v;
    if (!p)
        return exhausted;
    w_u32(p, kind);
    w_u32(p + 4, x);
    w_u32(p + 8, y);
    w_u32(p + 12, z);
    v = angle + (reverse ? 1024u : 0u) + (ff_rand_8006D348() & (reverse ? 511u : 255u)) - (reverse ? 256u : 128u);
    w_u16(p + 16, (uint16)(v & 2047u));
    v = (ff_rand_8006D348() & (reverse ? 15u : 31u)) + (reverse ? 12u : 16u);
    w_u16(p + 32, (uint16)v);
    v = (ff_rand_8006D348() & (reverse ? 15u : 31u)) + (reverse ? 32u : 64u);
    w_u16(p + 24, (uint16)v);
    w_u16(p + 26, 0);
    return v;
}

GDB_CALL uint32 FUN_8003100C(uint32 x, uint32 y, uint32 z, uint32 angle)
{
    return hit_particle(x, y, z, angle, 0, 0, 0);
}

GDB_CALL uint32 FUN_800310B8(uint32 x, uint32 y, uint32 z, uint32 angle)
{
    return hit_particle(x, y, z, angle, 1, 0, 0);
}

/* 31660: kind583 debris; unlike hit_particle, Y speed precedes radial speed
 * in the RNG stream, and allocation failure returns 583. */
GDB_CALL uint32 FUN_80031660(uint32 x, uint32 y, uint32 z, uint32 angle)
{
    FUNCTION_MARKER(0x80031660u, "GAME.EXE");
    uint32 p = FUN_80030F44(), v;
    if (!p)
        return 583;
    w_u32(p, 583);
    w_u32(p + 4, x);
    w_u32(p + 8, y);
    w_u32(p + 12, z);
    w_u16(p + 16, (uint16)((angle + (ff_rand_8006D348() & 511u) - 256u) & 2047u));
    w_u16(p + 24, (uint16)((ff_rand_8006D348() & 31u) + 32u));
    v = ff_rand_8006D348() & 31u;
    w_u16(p + 32, (uint16)(32u - v));
    w_u16(p + 26, 0);
    return v;
}

/* 31894: reverse-facing kind6, three RNG calls, then signed speed halving. */
GDB_CALL uint32 FUN_80031894(uint32 x, uint32 y, uint32 z, uint32 angle)
{
    FUNCTION_MARKER(0x80031894u, "GAME.EXE");
    uint32 p = FUN_80030F44(), v;
    if (!p)
        return 6;
    w_u32(p, 6);
    w_u32(p + 4, x);
    w_u32(p + 8, y);
    w_u32(p + 12, z);
    w_u16(p + 16, (uint16)((angle + 1024u + (ff_rand_8006D348() & 511u) - 256u) & 2047u));
    w_u16(p + 32, (uint16)((ff_rand_8006D348() & 31u) + 16u));
    w_u16(p + 24, (uint16)((ff_rand_8006D348() & 31u) + 64u));
    w_u16(p + 32, (uint16)(r_s16(p + 32) >> 1));
    v = (uint32)(sint32)(r_s16(p + 24) >> 1);
    w_u16(p + 24, (uint16)v);
    w_u16(p + 26, 0);
    return v;
}

/* Direct pair MIPS-0de99b50766aac743a42: only gravity and vertical SRA differ. */
static uint32 chunk_motion(uint32 p, uint32 gravity, uint32 shift)
{
    uint32 angle, v, x, y, z;
    sint32 speed, trig;
    speed = r_s16(p + 24);
    angle = 2 * (uint32)(sint32)r_s16(p + 16);
    trig = r_s16(0x800802d0 + angle);
    x = r_u32(p + 4);
    w_u32(p + 4, x - (uint32)((speed * trig) >> 1));
    speed = r_s16(p + 24);
    angle = 2 * (uint32)(sint32)r_s16(p + 16);
    trig = r_s16(0x800806d0 + angle);
    z = r_u32(p + 12);
    w_u32(p + 12, z + (uint32)((speed * trig) >> 1));
    w_u16(p + 18, (uint16)(r_u16(p + 18) + 0x1180));
    w_u16(p + 20, (uint16)(r_u16(p + 20) + 0x1c80));
    v = (uint32)r_u16(p + 32) - gravity;
    w_u16(p + 32, (uint16)v);
    y = r_u32(p + 8) - (uint32)((sint32)(v << 16) >> shift);
    w_u32(p + 8, y);
    if ((sint32)y > 0)
    {
        w_u32(p, 0xffffffffu);
        x = r_u32(p + 4);
        z = r_u32(p + 12);
        return FUN_80031C50(894, x, 0, z);
    }
    v = (uint32)r_u16(p + 26) + 1;
    w_u16(p + 26, (uint16)v);
    if (v & 1u)
        return 1;
    x = r_u32(p + 4);
    y = r_u32(p + 8);
    z = r_u32(p + 12);
    return FUN_80031C50(605, x, y, z);
}

GDB_CALL uint32 FUN_8002F828(uint32 p)
{
    FUNCTION_MARKER(0x8002f828u, "GAME.EXE");
    return chunk_motion(p, 6, 1);
}

GDB_CALL uint32 FUN_8002F700(uint32 p)
{
    FUNCTION_MARKER(0x8002f700u, "GAME.EXE");
    return chunk_motion(p, 3, 2);
}

/* 31CD8: five RNG draws and two real2F828 updates, age reset after each. */
GDB_CALL uint32 FUN_80031CD8(uint32 kind, uint32 x, uint32 y, uint32 z)
{
    FUNCTION_MARKER(0x80031cd8u, "GAME.EXE");
    uint32 p = FUN_80030F44(), v;
    if (!p)
        return 0;
    v = ff_rand_8006D348();
    w_u32(p, kind);
    w_u32(p + 4, x);
    w_u32(p + 8, y);
    w_u32(p + 12, z);
    w_u16(p + 16, (uint16)(v & 2047u));
    w_u16(p + 18, (uint16)(ff_rand_8006D348() << 1));
    w_u16(p + 20, (uint16)(ff_rand_8006D348() << 1));
    w_u16(p + 24, (uint16)((ff_rand_8006D348() & 63u) + 80u));
    w_u16(p + 32, (uint16)((ff_rand_8006D348() & 63u) + 80u));
    FUN_8002F828(p);
    w_u16(p + 26, 0);
    v = FUN_8002F828(p);
    w_u16(p + 26, 0);
    return v;
}

/* 31A28: attachment-dependent particle in the rotating pool. */
GDB_CALL uint32 FUN_80031A28(sint32 index, uint32 kind)
{
    FUNCTION_MARKER(0x80031a28u, "GAME.EXE");
    uint32 actor = 0x800b4318 + 244u * (uint32)index, p = FUN_80030FAC(), bias, v;
    w_u32(p, kind);
    if (r_s16(actor + 52) == 2)
    {
        FUN_80017F88(p, index, 3);
        bias = 0xfffffdc0u;
    }
    else
    {
        FUN_80017F88(p, index, 2);
        bias = 576;
    }
    w_u32(p + 8, r_u32(p + 8) - 0x100000u);
    v = ff_rand_8006D348() & 127u;
    w_u16(p + 16, (uint16)((((uint16)r_s16(actor + 56) - v + bias) & 2047u) << 5));
    w_u16(p + 32, (uint16)((ff_rand_8006D348() & 7u) + 32u));
    v = (ff_rand_8006D348() & 7u) + 64u;
    w_u16(p + 24, (uint16)v);
    w_u16(p + 26, 0);
    return v;
}

/* 31DA8: aimed variant, wider radial speed for kinds902..905. */
GDB_CALL uint32 FUN_80031DA8(uint32 kind, uint32 x, uint32 y, uint32 z, uint32 angle)
{
    FUNCTION_MARKER(0x80031da8u, "GAME.EXE");
    uint32 p = FUN_80030F44(), v;
    if (!p)
        return 0;
    v = (angle - 256u + (ff_rand_8006D348() & 511u)) & 2047u;
    w_u32(p, kind);
    w_u32(p + 4, x);
    w_u32(p + 8, y);
    w_u32(p + 12, z);
    w_u16(p + 16, (uint16)v);
    w_u16(p + 18, (uint16)(ff_rand_8006D348() << 1));
    w_u16(p + 20, (uint16)(ff_rand_8006D348() << 1));
    w_u16(p + 24, (uint16)((ff_rand_8006D348() & 63u) + (kind - 902u < 4u ? 160u : 80u)));
    w_u16(p + 32, (uint16)((ff_rand_8006D348() & 63u) + 80u));
    FUN_8002F828(p);
    w_u16(p + 26, 0);
    v = FUN_8002F828(p);
    w_u16(p + 26, 0);
    return v;
}

/* 31164/3124C: actual fifth stack argument is a full word angle, not IDA a9.
 * Same particle fields/RNG as 3100C/310B8; actor type53 selects kind1.
 * Allocation failure returns the branch-delay result index<<4 without RNG. */
GDB_CALL uint32 FUN_80031164(sint32 index, uint32 x, uint32 y, uint32 z, uint32 angle)
{
    return hit_particle(x, y, z, angle, 0, r_s16(0x800b434c + 244u * (uint32)index) == 53, (uint32)index << 4);
}

GDB_CALL uint32 FUN_8003124C(sint32 index, uint32 x, uint32 y, uint32 z, uint32 angle)
{
    return hit_particle(x, y, z, angle, 1, r_s16(0x800b434c + 244u * (uint32)index) == 53, (uint32)index << 4);
}

/* 31B20: seven actual MIPS arguments; Hex-Rays draft stack arguments are shifted. */
GDB_CALL uint32 FUN_80031B20(uint32 kind, uint32 x, uint32 y, uint32 z, sint32 age, sint32 a, sint32 b)
{
    uint32 p = FUN_80030F44(), v, i;
    if (!p)
        return 0;
    for (i = 0; i < 36; ++i)
        w_u8(p + i, 0);
    w_u32(p, kind);
    w_u32(p + 4, x);
    w_u32(p + 8, y);
    w_u32(p + 12, z);
    w_u16(p + 16, (uint16)a);
    w_u16(p + 20, (uint16)b);
    if (kind - 871u < 2u)
        v = (ff_rand_8006D348() & 7u) + (uint32)age;
    else if (kind == 870)
        v = (ff_rand_8006D348() & 3u) + (uint32)age;
    else if (kind == 879)
        v = (uint32)age;
    else if (kind == 880)
        v = 0;
    else
    {
        v = (ff_rand_8006D348() & 7u) + (uint32)age;
        w_u16(p + 22, (uint16)v);
        w_u16(p + 26, 216);
        return p;
    }
    w_u16(p + 22, (uint16)v);
    return p;
}

/* 2D7C8: accelerate hinged object, bounce at -15360, four dust bursts. */
GDB_CALL uint32 FUN_8002D7C8(uint32 object)
{
    FUNCTION_MARKER(0x8002d7c8u, "GAME.EXE");
    uint16 speed = (uint16)((uint16)r_s16(object + 22) + 128u), angle;
    sint32 count, offset;
    uint32 i, z, result;
    w_u16(object + 22, speed);
    angle = (uint16)((uint16)r_s16(object + 18) - speed);
    w_u16(object + 18, angle);
    if ((sint16)angle >= -15360)
        return 0;
    count = r_s16(object + 26);
    if (count < 4)
    {
        w_u16(object + 26, (uint16)(count + 1));
        w_u32(0x800941a8, 6);
        if (r_s16(object + 26) == 1)
            FUN_80056670(116, 255, 6144);
        for (i = 0; i < 32; i++)
        {
            offset = (sint32)(ff_rand_8006D348() & 1023) - 512;
            offset += offset >> 3;
            z = ((ff_rand_8006D348() & 255) + 320u) << 16;
            FUN_80031B20(870, ((uint32)offset << 16) + r_u32(object + 4), 0, z + r_u32(object + 12), 0, 0, 0);
        }
    }
    w_u16(object + 18, 0xc400);
    result = 0u - (uint32)(r_s16(object + 22) >> 2);
    w_u16(object + 22, (uint16)result);
    return result;
}

/* 2DBA0: falling pitch and paired debris emission on phase2. */
GDB_CALL uint32 FUN_8002DBA0(uint32 object)
{
    FUNCTION_MARKER(0x8002dba0u, "GAME.EXE");
    sint32 voice, pitch, offset, phase;
    uint32 i, x, z, y, age, point[3];
    FUN_8002DA3C(object);
    if (!r_s16(object + 28))
        w_u16(object + 32, 6144);
    voice = r_s16(object + 30);
    if (voice > 0)
    {
        FUN_80056840(voice - 1, r_s16(object + 32));
        pitch = (sint16)((uint16)r_s16(object + 32) - 16u);
        w_u16(object + 32, (uint16)pitch);
        if (pitch < 4096)
        {
            FUN_800567F8(r_s16(object + 30) - 1);
            w_u16(object + 30, 0);
        }
    }
    if (r_s16(object + 28) == 2)
    {
        FUN_80010040();
        FUN_80011D20();
        FUN_800101CC(32768);
        FUN_8001033C(r_s16(object + 16));
        x = r_u32(object + 4);
        z = r_u32(object + 12);
        w_u32(0x800941a8, 8);
        for (i = 0; i < 24; i++)
        {
            offset = (sint32)(ff_rand_8006D348() & 511) - 256;
            offset += offset >> 2;
            y = (0xffffff70u - (ff_rand_8006D348() & 31)) << 16;
            FUN_800614F4((sint32)y, 0, (sint32)((uint32)offset << 16), point);
            y = (0u - (ff_rand_8006D348() & 31)) << 16;
            age = ff_rand_8006D348() & 3;
            FUN_80031B20(871, x + point[0], y, z + point[2], (sint32)age, 0, 0);
            offset = (sint32)(ff_rand_8006D348() & 511) - 256;
            y = ((ff_rand_8006D348() & 31) + 144u) << 16;
            FUN_800614F4((sint32)y, 0, (sint32)((uint32)offset << 16), point);
            y = (0u - (ff_rand_8006D348() & 31)) << 16;
            age = ff_rand_8006D348() & 3;
            FUN_80031B20(871, x + point[0], y, z + point[2], (sint32)age, 0, 0);
        }
        FUN_80010028();
    }
    phase = r_s16(object + 28);
    if (phase < 4)
        w_u16(object + 28, (uint16)(phase + 1));
    return (uint32)(phase + 1);
}

/* 2DDB8: halfword bounce, settle into kind528/529/530 and shift its origin. */
GDB_CALL uint32 FUN_8002DDB8(uint32 object)
{
    FUNCTION_MARKER(0x8002ddb8u, "GAME.EXE");
    uint32 speed = (uint16)r_s16(object + 22) + 256u, angle, result, kind, point[3];
    sint32 bounce, z;
    w_u16(object + 22, (uint16)speed);
    angle = (uint16)r_s16(object + 18) - speed;
    w_u16(object + 18, (uint16)angle);
    result = angle << 16;
    if ((sint32)result >= 0)
        return result;
    w_u16(object + 18, 0);
    bounce = -(r_s16(object + 22) >> 1);
    w_u16(object + 22, (uint16)bounce);
    if ((uint16)(bounce + 255) >= 511)
        return 724; /* Delay slot2DE20. */
    kind = r_u32(object);
    kind = kind == 724 ? 530 : kind == 69 ? 529 : 528;
    w_u32(object, kind);
    w_u16(object + 22, (uint16)(sint16)r_s8(0x8009439c + r_u32(object)));
    FUN_80010040();
    FUN_80011D20();
    FUN_800101CC(32768);
    FUN_8001033C(r_s16(object + 16));
    kind = r_u32(object);
    z = kind == 529 ? -3932160 : kind == 530 ? -3670016 : -3145728;
    FUN_800614F4(0, 0, z, point);
    w_u32(object + 4, r_u32(object + 4) + point[0]);
    w_u32(object + 12, r_u32(object + 12) + point[2]);
    return FUN_80010028();
}
