#include <stdio.h>

#include "game.h"

sint32 lab_tank_level;

static sint32 update(MODEL_OBJECT *item)
{
    FUNCTION_MARKER(0x8002eb6cu, "GAME.EXE");
    uint32 phase, offset, radius, x, z, result;
    /* TODO: Investigate a lab tank callback without its native owner */
    if (!item || item->payload_kind != MODEL_PAYLOAD_LAB_TANK)
    {
        DIAGNOSTIC(fprintf(stderr, "Invalid lab tank payload object %p stage %u tick %u\n", (const void *)item, game_session.stage, game_session.tick));
        diagnostic_stop(__FUNCTION__, __FILE__, __LINE__);
    }
    if (lab_tank_level < -622)
    {
        if (item->lab_tank.voice_plus_one[0])
            sound_voice_stop(item->lab_tank.voice_plus_one[0] - 1);
        if (item->lab_tank.voice_plus_one[1])
            sound_voice_stop(item->lab_tank.voice_plus_one[1] - 1);
        lab_tank_level = -624;
        /* TODO: Investigate a model callback outside its native object domain */
        if (!item)
        {
            DIAGNOSTIC(fprintf(stderr, "Invalid model receiver %p in %s stage %u tick %u\n", (const void *)item, __FUNCTION__, game_session.stage, game_session.tick));
            diagnostic_stop(__FUNCTION__, __FILE__, __LINE__);
        }
        item->kind = 55;
        return 55;
    }
    if (!((uint32)lab_tank_level & 0xfffffffeu))
        level_wave.paused = 0;
    else if (((uint32)lab_tank_level & 0xfffffffeu) == (uint32)-150)
    {
        col_regions.named[5]->col_kind = 0;
        col_regions.named[6]->col_kind = 0;
    }
    if (!(random_next() & 3u))
    {
        offset = (random_next() & 2047u) * 2u;
        radius = random_next() & 127u;
        radius += radius >> 3;
        /* TODO: Investigate a model callback outside its native object domain */
        if (!item)
        {
            DIAGNOSTIC(fprintf(stderr, "Invalid model receiver %p in %s stage %u tick %u\n", (const void *)item, __FUNCTION__, game_session.stage, game_session.tick));
            diagnostic_stop(__FUNCTION__, __FILE__, __LINE__);
        }
        x = (uint32)(sint32)sin_table[offset >> 1] * radius + item->pos.x.bits;
        z = (uint32)(sint32)sin_table[MATH_QUARTER_TURN + (offset >> 1)] * radius + item->pos.z.bits;
        effect_spawn(579, x, 0xfff00000u, z);
    }
    phase = lab_tank_level;
    result = phase - 2u;
    if ((sint32)phase >= 624)
        return result;
    lab_tank_level = (sint32)result;
    if (!item->lab_tank.voice_plus_one[0])
        item->lab_tank.voice_plus_one[0] = (uint16)((uint32)sound_voice_play(154, 79, 6144) + 1u);
    if ((uint32)lab_tank_level >= 2u)
        return 0;
    if (item->lab_tank.voice_plus_one[0])
        sound_voice_stop(item->lab_tank.voice_plus_one[0] - 1);
    item->lab_tank.voice_plus_one[0] = (uint16)((uint32)sound_voice_play(237, 95, 5888) + 1u);
    result = (uint32)sound_voice_play(237, 111, 5920) + 1u;
    item->lab_tank.voice_plus_one[1] = (uint16)result;
    return result;
}

static uint32 update_effect(EFFECT *effect)
{
    FUNCTION_MARKER(0x8002ed7cu, "GAME.EXE");
    /* TODO: Investigate a lab particle callback outside its native pool */
    if (!effect || effect->payload_kind != EFFECT_PAYLOAD_PARTICLE)
    {
        DIAGNOSTIC(fprintf(stderr, "Invalid lab particle owner object %p callback %s stage %u tick %u\n", (const void *)effect, __FUNCTION__, game_session.stage, game_session.tick));
        diagnostic_stop(__FUNCTION__, __FILE__, __LINE__);
    }
    EFFECT_PARTICLE *particle = &effect->particle;
    uint32 speed, result;
    if (!particle->age)
        particle->vertical_speed = (random_next() & 7u) + 8u;
    speed = (uint32)(sint32)(sint16)particle->vertical_speed;
    particle->pos.y.bits -= speed << 14;
    ++particle->vertical_speed;
    if ((sint32)particle->pos.y.bits < (sint32)((0u - (uint32)lab_tank_level) << 16))
        effect->kind = 0xffffffffu;
    result = particle->age + 1u;
    particle->age = result;
    return result;
}

static sint32 start(void)
{
    FUNCTION_MARKER(0x8004298cu, "GAME.EXE");
    if ((sint32)game_session.scene_phase >= 3)
        return 0;
    if (!players_past_z(0x04300000))
        return 3;
    game_session.scene_phase = 3;
    col_regions.named[3]->col_kind = 3;
    lab_tank_level = 607;
    return 607;
}

static sint32 wait_npc(sint32 idx)
{
    FUNCTION_MARKER(0x800565a4u, "GAME.EXE");
    sint32 type = actor_types[idx];
    sint32 limit = lab_tank_level;
    uint32 table = animation_type_banks[(uint32)type];
    if (limit < 17)
    {
        npc_start_animation(idx, table, npc_behavior_actors[idx].idle_animation);
        actor_lifecycle[idx].delay = 60;
        return 60;
    }
    if (actor_animations[idx].frame == 0xffffffffu)
        return animation_start(idx, table, actor_animations[idx].id);
    return -1;
}

static uint32 draw(const MODEL_OBJECT *item)
{
    FUNCTION_MARKER(0x8005b844u, "GAME.EXE");
    PROJECTION_POINT point;
    sint32 brightness = (sin_table[((game_session.tick << 5) & 0x7e0u) >> 1] >> 9) + 65, distance, amount, scale;
    uint32 cutoff;
    proj_local(0, 0, 0, &point);
    cutoff = RENDER_FADE_CUTOFF;
    distance = (uint32)point.depth - cutoff;
    if (distance > 0)
        return cutoff;
    distance = (uint32)distance + 2048u;
    if (distance < 0)
        distance = 0;
    brightness = (sint32)((128u - (uint32)(distance >> 4)) * (uint32)brightness) >> 7;
    amount = lab_tank_level;
    if (amount > 0)
    {
        scale = (sint32)((uint32)amount << 14) / 624;
        transform_push();
        transform_scale(16384, scale, 16384);
        render_style.color.rgbc = 0x2c000000u + (uint32)brightness * 0x10101u;
        /* TODO: Investigate a model callback outside its native object domain */
        if (!item)
        {
            DIAGNOSTIC(fprintf(stderr, "Invalid model receiver %p in %s stage %u tick %u\n", (const void *)item, __FUNCTION__, game_session.stage, game_session.tick));
            diagnostic_stop(__FUNCTION__, __FILE__, __LINE__);
        }
        model_draw_static(model_refs[item->kind], 0, -64);
        transform_pop();
    }
    else
        scale = (sint32)((0u - (uint32)lab_tank_level) << 14) / 624;
    transform_translate(0, 0x02700000, 0);
    transform_scale(16384, 16384u - (uint32)scale, 16384);
    render_style.color.rgbc = 0x2c000000u + ((uint32)brightness << 16) + (uint32)(brightness >> 1) * 257u;
    return model_draw_static(model_refs[434], 0, -64);
}

const OBJECT_CALLBACK callback_lab_tank_update = {OBJECT_CALL_S32_MUTABLE_MODEL, {.s32_mutable_model = update}, 0};

const OBJECT_CALLBACK callback_lab_tank_update_effect = {OBJECT_CALL_U32_EFFECT, {.u32_effect = update_effect}, 0};

const OBJECT_CALLBACK callback_lab_tank_start = {OBJECT_CALL_S32_NONE, {.s32_none = start}, 0};

const OBJECT_CALLBACK callback_lab_tank_draw = {OBJECT_CALL_U32_MODEL, {.u32_model = draw}, 0};

const NPC_ACTION npc_action_lab_tank_wait = {NPC_ACTION_SINT32, {.s32_idx = wait_npc}, 0};

static uint32 draw_effect(const EFFECT *effect)
{
    FUNCTION_MARKER(0x8005ba28u, "GAME.EXE");
    /* TODO: Investigate a lab particle callback outside its native pool */
    if (!effect || effect->payload_kind != EFFECT_PAYLOAD_PARTICLE)
    {
        DIAGNOSTIC(fprintf(stderr, "Invalid lab particle owner object %p callback %s stage %u tick %u\n", (const void *)effect, __FUNCTION__, game_session.stage, game_session.tick));
        diagnostic_stop(__FUNCTION__, __FILE__, __LINE__);
    }
    const EFFECT_PARTICLE *particle = &effect->particle;
    PROJECTION_POINT point;
    sint32 distance;
    uint32 phase, x, z, cutoff, color;
    transform_push();
    phase = (uint32)(sint32)(sint16)particle->age + game_session.tick;
    x = ((uint32)(sint32)sin_table[((phase << 6) & 0xfc0u) >> 1] << 5) + particle->pos.x.bits;
    z = ((uint32)(sint32)sin_table[((phase << 5) & 0xfe0u) >> 1] << 5) + particle->pos.z.bits;
    transform_translate(x - camera.pos.x.bits, particle->pos.y.bits - camera.pos.y.bits, z - camera.pos.z.bits);
    proj_local(0, 0, 0, &point);
    transform_pop();
    if (point.status)
        return (uint32)point.status;
    cutoff = RENDER_FADE_CUTOFF;
    distance = (uint32)point.depth - cutoff;
    if (distance > 0)
        return cutoff;
    distance = (uint32)distance + 2048u;
    if (distance < 0)
        distance = 0;
    color = (16u - (uint32)(distance >> 7)) * 1057u;
    if (point.depth > 2560)
        return particle_draw_point(point.x, point.y, point.depth, color);
    if (point.depth < 1536)
        return particle_draw_quad(point.x, point.y, point.depth, color);
    return particle_draw_line(point.x, point.y, point.depth, color);
}

const OBJECT_CALLBACK callback_lab_tank_draw_effect = {OBJECT_CALL_U32_CONST_EFFECT, {.u32_const_effect = draw_effect}, 0};
