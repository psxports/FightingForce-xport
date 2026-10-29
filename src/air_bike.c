#include "game.h"

sint32 air_bike_update_approach(MODEL_OBJECT *model)
{
    FUNCTION_MARKER(0x8002efdcu, "GAME.EXE");
    /* TODO: Investigate common motion fields outside the native object pools */
    XASSERT(model);
    /* TODO: Investigate an air bike callback with a foreign payload owner */
    XASSERT(model->payload_kind == MODEL_PAYLOAD_AIR_BIKE);
    uint32 yaw = (uint16)model->angles.yaw, pitch = (uint16)model->angles.pitch, roll = (uint16)model->angles.roll, volume, pan, distance, offset, y, result;
    sint32 delta, speed, slot;
    if (model->air_bike.voice_plus_one)
    {
        /* TODO: Investigate motion callbacks without a native world or dynamic position */
        XASSERT(model);
        sound_spatial(&model->pos, &volume, &pan);
        sound_voice_pan(model->air_bike.voice_plus_one - 1, volume, pan);
        sound_voice_pitch(model->air_bike.voice_plus_one - 1, 6144u - (dir_distance >> 2));
    }
    else
        model->air_bike.voice_plus_one = (uint16)(sound_voice_play(6, 255, 6144) + 1u);
    /* TODO: Investigate motion callbacks without a native world or dynamic position */
    XASSERT(model);
    delta = (sint16)(math_dir16(model->pos.x.whole, model->pos.z.whole, model->air_bike.target[0].x, model->air_bike.target[0].z) - yaw);
    if (delta > 8192)
        delta = 8192;
    else if (delta < -8192)
        delta = -8192;
    roll -= (uint32)(((sint16)roll + delta) >> 4);
    yaw -= (uint32)((sint16)roll >> 4);
    distance = dir_distance;
    offset = (((model->air_bike.phase) + game_session.tick) << 5) & 0xfe0u;
    y = ((uint32)(-(sint64)(distance))) * 17408u + ((sint32)((sin_table[offset >> 1]) * distance) >> 4);
    model->pos.y.bits = y;
    if ((sint32)y > (sint32)0xff580000u)
        model->pos.y.bits = 0xff580000;
    distance = dir_distance - 1024u;
    distance = (sint32)distance > 1024 ? 2048u : distance << 1;
    pitch += (uint32)((sint32)(distance - ((sint16)pitch)) >> 5);
    speed = sin_table[((pitch >> 1) & 0xffeu) >> 1] >> 8;
    offset = (yaw >> 4) & 0xffeu;
    model->pos.x.bits = model->pos.x.bits + (speed * sin_table[offset >> 1]);
    model->pos.z.bits = model->pos.z.bits - (speed * sin_table[MATH_QUARTER_TURN + (offset >> 1)]);
    model->angles.yaw_bits = (uint16)yaw;
    model->angles.pitch_bits = (uint16)pitch;
    model->angles.roll_bits = (uint16)roll;
    if (dir_distance >= 768)
        return 0;
    model->kind = model->kind + 1u;
    sound_voice_play(199, 255, 6144);
    slot = npc_create(47, model->pos.x.bits, model->pos.z.bits, ((model->angles.yaw >> 5) - 512) & 2047, 0, 0, 0);
    /* TODO: Investigate NPC pool exhaustion during the moving object transition */
    XASSERT(slot >= 0 && slot < ACTOR_COUNT);
    npc_start_jump_off(slot, 1);
    animation_update_motion(slot);
    model->air_bike.actor_idx = (uint16)slot;
    result = level_wave.remaining_enemies + 1u;
    level_wave.remaining_enemies = result;
    level_wave.paused = 0;
    return result;
}

sint32 air_bike_update_departure(MODEL_OBJECT *model)
{
    FUNCTION_MARKER(0x8002f2e0u, "GAME.EXE");
    uint32 volume, pan, yaw, pitch, roll, distance, offset, y, frame;
    sint32 delta, speed, actor_idx;
    /* TODO: Investigate motion callbacks without a native world or dynamic position */
    XASSERT(model);
    /* TODO: Investigate an air bike callback with a foreign payload owner */
    XASSERT(model->payload_kind == MODEL_PAYLOAD_AIR_BIKE);
    sound_spatial(&model->pos, &volume, &pan);
    sound_voice_pan(model->air_bike.voice_plus_one - 1, volume, pan);
    yaw = (uint16)model->angles.yaw;
    pitch = (uint16)model->angles.pitch;
    roll = (uint16)model->angles.roll;
    math_dir16(model->pos.x.whole, model->pos.z.whole, model->air_bike.target[0].x, model->air_bike.target[0].z);
    distance = dir_distance;
    offset = (((model->air_bike.phase) + game_session.tick) << 5) & 0xfe0u;
    y = ((uint32)(-(sint64)(distance))) * 17408u + ((sint32)((sin_table[offset >> 1]) * distance) >> 4);
    model->pos.y.bits = y;
    if ((sint32)y > (sint32)0xff580000u)
        model->pos.y.bits = 0xff580000;
    delta = (sint16)(math_dir16(model->pos.x.whole, model->pos.z.whole, model->air_bike.target[1].x, model->air_bike.target[1].z) - yaw);
    if (delta > 8192)
        delta = 8192;
    else if (delta < -8192)
        delta = -8192;
    roll -= (uint32)(((sint16)roll + delta) >> 4);
    yaw -= (uint32)((sint16)roll >> 4);
    distance = dir_distance - 1024u;
    distance = (sint32)distance > 1024 ? 2048u : distance << 1;
    pitch += (uint32)((sint32)(distance - ((sint16)pitch)) >> 5);
    speed = sin_table[((pitch >> 1) & 0xffeu) >> 1] >> 8;
    offset = (yaw >> 4) & 0xffeu;
    model->pos.x.bits = model->pos.x.bits + (speed * sin_table[offset >> 1]);
    model->pos.z.bits = model->pos.z.bits - (speed * sin_table[MATH_QUARTER_TURN + (offset >> 1)]);
    model->angles.yaw_bits = (uint16)yaw;
    model->angles.pitch_bits = (uint16)pitch;
    model->angles.roll_bits = (uint16)roll;
    if (dir_distance < 1024)
    {
        sound_voice_stop(model->air_bike.voice_plus_one - 1);
        model->air_bike.voice_plus_one = 0;
        model->kind = 0xffffffffu;
    }
    actor_idx = model->air_bike.actor_idx;
    /* TODO: Investigate an invalid actor retained by the moving object */
    XASSERT(actor_idx >= 0 && actor_idx < ACTOR_COUNT);
    if (actor_animations[actor_idx].id != 269)
        return 269;
    frame = actor_animations[actor_idx].frame;
    if ((sint32)frame >= 12 || frame == 0xffffffffu)
        return -1;
    actor_positions[actor_idx].x.bits = model->pos.x.bits;
    y = model->pos.z.bits;
    actor_positions[actor_idx].z.bits = y;
    return y;
}
