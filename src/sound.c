#include <stdlib.h>

#include "game.h"
#include "sound_device.h"
#include "platform.h"

/* Camera-relative pan and sine-table distance attenuation */
GDB_CALL sint32 sound_spatial(const FIXED_VEC3 *pos, uint32 *volume, uint32 *pan)
{
    FUNCTION_MARKER(0x80046884, "GAME.EXE");
    /* TODO: Investigate missing native spatial-sound arguments */
    XASSERT(pos && volume && pan);
    sint32 x, z, px, pz, distance, p, v;
    uint32 angle, camera_angle;
    x = camera.pos.x.whole;
    z = camera.pos.z.whole;
    px = pos->x.whole;
    pz = pos->z.whole;
    angle = math_dir16(x, z, px, pz);
    camera_angle = camera.angles.yaw_bits;
    x = camera.pos.x.whole;
    z = camera.pos.z.whole;
    px = pos->x.whole;
    pz = pos->z.whole;
    angle -= camera_angle;
    distance = math_distance_approx(x, z, px, pz) >> 5;
    if (distance >= 512)
        distance = 511;
    p = 127 - (sin_table[((angle >> 4) & 0xffe) >> 1] >> 9);
    if (p < 0)
        p = 0;
    else if ((sint16)p >= 256)
        p = 255;
    *pan = (sint16)p;
    v = (sin_table[511 - distance] >> 7) & 255;
    *volume = v;
    return v;
}

/* 293D8 selection prefix. Keep RNG calls separate and ordered before56670. */
GDB_CALL void sound_pain_parameters(sint32 idx, sint32 parameters[3])
{
    sint32 type = actor_types[(uint32)idx], effect;
    switch (type)
    {
        case 0:
            effect = 40;
            break;
        case 1:
        case 12:
        case 29:
        case 30:
        case 38:
            effect = 64 + (random_next() & 1);
            break;
        case 2:
        case 49:
            effect = 84;
            break;
        case 3:
            effect = 99 + (random_next() & 1);
            break;
        case 48:
            effect = 190 + (random_next() & 1);
            break;
        case 32:
            effect = 159;
            break;
        case 14:
            effect = 208;
            break;
        default:
            effect = (sint32)random_next() % 3 + 9;
            break;
    }
    parameters[0] = effect;
    parameters[1] = 255 - (random_next() & 63);
    parameters[2] = 5952 + (random_next() & 255);
}

GDB_CALL sint32 sound_play_pain(sint32 idx)
{
    FUNCTION_MARKER(0x800293d8u, "GAME.EXE");
    sint32 p[3];
    sound_pain_parameters(idx, p);
    return sound_voice_play(p[0], p[1], p[2]);
}

GDB_CALL uint32 sound_volume_adjust(uint32 val, sint32 delta)
{
    FUNCTION_MARKER(0x80042820u, "GAME.EXE");
    /* Preserve volume-word wrapping before signed saturation */
    sint32 sum = val + delta;
    return sum <= 0 ? 0u : sum < 255 ? sum : 255u;
}

GDB_CALL sint32 sound_play_attack_voice(sint32 idx)
{
    FUNCTION_MARKER(0x80029944u, "GAME.EXE");
    sint32 kind = actor_types[(uint32)idx], sound = 51, pitch = 6144;
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
    return sound_voice_play(sound, 255, pitch - (sint32)(random_next() & 255u));
}

GDB_CALL sint32 sound_play_heavy_hit_voice(sint32 idx)
{
    FUNCTION_MARKER(0x80029a40u, "GAME.EXE");
    sint32 kind = actor_types[(uint32)idx], sample, pitch = 6144;
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
    return sound_voice_play(sample, 255, pitch - (sint32)(random_next() & 255u));
}

GDB_CALL sint32 sound_play_chord(void)
{
    FUNCTION_MARKER(0x800149ecu, "GAME.EXE");
    sound_voice_play(46, 255, 7168);
    sound_voice_play(46, 255, 6912);
    return sound_voice_play(46, 255, 6656);
}

GDB_CALL sint32 sound_play_action_voice(sint32 idx)
{
    FUNCTION_MARKER(0x8002976cu, "GAME.EXE");
    sint32 type = actor_types[(uint32)idx], sample = 38;
    switch (type)
    {
        case 1:
        case 12:
        case 29:
        case 30:
        case 38:
            sample = 62;
            break;
        case 2:
        case 14:
        case 32:
        case 49:
            sample = 79;
            break;
        case 3:
            sample = 98;
            break;
        case 48:
            sample = 189;
            break;
    }
    return sound_voice_play(sample, 255, 6144 - (sint32)(random_next() & 255u));
}

GDB_CALL sint32 sound_play_strike_voice(sint32 idx)
{
    FUNCTION_MARKER(0x80052878u, "GAME.EXE");
    sint32 type = actor_types[(uint32)idx], sample;
    if (type == 49)
        sample = 177;
    else if (type == 14)
        sample = 210;
    else
        sample = (type == 32 ? 76 : 33) + (sint32)random_next() % 3;
    return sound_voice_play(sample, 255, 6144);
}

GDB_CALL uint32 sound_play_death_voice(sint32 idx)
{
    FUNCTION_MARKER(0x80029530u, "GAME.EXE");
    sint32 type = actor_types[(uint32)idx], sound, pitch = 5952;
    sint32 volume = 192 - (random_next() & 63u);
    switch (type)
    {
        case 0:
            sound = 42;
            break;
        case 1:
            sound = 69;
            break;
        case 2:
            sound = 82;
            break;
        case 3:
            sound = 104;
            break;
        case 48:
            sound = 193;
            break;
        case 12:
        case 29:
        case 30:
        case 38:
            sound = 14;
            break;
        case 49:
        case 50:
            sound = 174;
            volume = 255;
            break;
        case 32:
            sound = 160;
            volume = 255;
            break;
        case 14:
            sound = 174;
            volume = 255;
            pitch = 5696;
            break;
        default:
            sound = (random_next() & 1u) + 12;
            break;
    }
    pitch += (sint32)(random_next() & 255u);
    return sound_voice_play(sound, volume, pitch);
}

GDB_CALL uint32 sound_play_hit_voice(sint32 idx)
{
    FUNCTION_MARKER(0x80029684u, "GAME.EXE");
    sint32 type = actor_types[(uint32)idx], sound = 50;
    switch (type)
    {
        case 1:
        case 12:
        case 48:
        case 29:
        case 30:
        case 38:
            sound = 68;
            break;
        case 3:
            sound = 101;
            break;
        case 49:
            sound = 168;
            break;
        case 14:
            sound = 209;
            break;
    }
    return sound_voice_play(sound, 255, 6144 - (sint32)(random_next() & 255u));
}

GDB_CALL sint32 sound_play_effort_voice(sint32 idx)
{
    FUNCTION_MARKER(0x800292f8u, "GAME.EXE");
    sint32 type = actor_types[(uint32)idx], sound = 47, pitch = 6144;
    switch (type)
    {
        case 1:
        case 12:
        case 29:
        case 30:
        case 38:
            sound = 67;
            break;
        case 2:
        case 32:
            sound = 83;
            break;
        case 3:
            sound = 103;
            break;
        case 48:
            sound = 195;
            break;
        case 49:
            sound = 176;
            break;
        case 14:
            sound = 83;
            pitch = 5888;
            break;
    }
    return sound_voice_play(sound, 255, pitch);
}

GDB_CALL sint32 sound_play_ground_voice(sint32 idx)
{
    FUNCTION_MARKER(0x80029854u, "GAME.EXE");
    sint32 type = actor_types[(uint32)idx], sample = 39;
    switch (type)
    {
        case 1:
        case 3:
        case 12:
        case 29:
        case 30:
        case 38:
            sample = 63;
            break;
        case 2:
        case 14:
            sample = 80;
            break;
        case 48:
            sample = 197;
            break;
        case 32:
            sample = 165;
            break;
        case 49:
            sample = 173;
            break;
    }
    return sound_voice_play(sample, 255, 6144 - (sint32)(random_next() & 255u));
}
