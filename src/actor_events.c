#include <stdio.h>

#include "game.h"

const ACTOR_EVENT *actor_events[ACTOR_EVENT_SLOT_COUNT];

const ACTOR_EVENT actor_event_noop = {ACTOR_EVENT_NONE, {.none = actor_ignore_event}};

const ACTOR_EVENT actor_event_attack_voice = {ACTOR_EVENT_SINT32, {.s32_idx = actor_attack_voice_event}};

const ACTOR_EVENT actor_event_ground_effect = {ACTOR_EVENT_INTPTR, {.intptr_idx = actor_ground_effect_event}};

const ACTOR_EVENT actor_event_ground_effect_voice = {ACTOR_EVENT_INTPTR, {.intptr_idx = actor_ground_effect_voice_event}};

const ACTOR_EVENT actor_event_attack_fixed_voice = {ACTOR_EVENT_SINT32, {.s32_idx = actor_attack_fixed_voice_event}};

const ACTOR_EVENT actor_event_attack_effects = {ACTOR_EVENT_INTPTR, {.intptr_idx = actor_process_attack_effects}};

const ACTOR_EVENT actor_event_hit_repeat = {ACTOR_EVENT_SINT32, {.s32_idx = actor_hit_repeat_event}};

const ACTOR_EVENT actor_event_running_attack = {ACTOR_EVENT_EFFECT, {.effect_idx = actors_emit_running_attack}};

const ACTOR_EVENT actor_event_hit_begin = {ACTOR_EVENT_SINT32, {.s32_idx = actor_hit_begin_event}};

const ACTOR_EVENT actor_event_heavy_hit = {ACTOR_EVENT_SINT32, {.s32_idx = actor_heavy_hit_event}};

const ACTOR_EVENT actor_event_hit_end = {ACTOR_EVENT_SINT32, {.s32_idx = actor_hit_end_event}};

const ACTOR_EVENT actor_event_hit_attachment = {ACTOR_EVENT_SINT32, {.s32_idx = actor_hit_attachment_event}};

const ACTOR_EVENT actor_event_hit_impact = {ACTOR_EVENT_SINT32, {.s32_idx = actor_hit_impact_event}};

const ACTOR_EVENT actor_event_hit_flash = {ACTOR_EVENT_SINT32, {.s32_idx = actor_hit_flash_event}};

const ACTOR_EVENT actor_event_attack_attachment = {ACTOR_EVENT_SINT32, {.s32_idx = actor_attack_attachment_event}};

const ACTOR_EVENT actor_event_attachment_impact = {ACTOR_EVENT_UINT32, {.u32_idx = actor_attachment_impact_event}};

const ACTOR_EVENT actor_event_impact_voice = {ACTOR_EVENT_SINT32, {.s32_idx = actor_impact_voice_event}};

const ACTOR_EVENT actor_event_attachment_particle0 = {ACTOR_EVENT_UINT32, {.u32_idx = actor_attachment_particle_event0}};

const ACTOR_EVENT actor_event_attachment_particle1 = {ACTOR_EVENT_UINT32, {.u32_idx = actor_attachment_particle_event1}};

const ACTOR_EVENT actor_event_burst_attachment = {ACTOR_EVENT_INTPTR, {.intptr_idx = actor_burst_attachment_event}};

const ACTOR_EVENT actor_event_pickup = {ACTOR_EVENT_PICKUP, {.pickup = regions_update_pickup}};

const ACTOR_EVENT actor_event_release_item = {ACTOR_EVENT_SINT32, {.s32_idx = actor_release_item_event}};

const ACTOR_EVENT actor_event_shock_attack = {ACTOR_EVENT_INTPTR, {.intptr_idx = actor_process_shock_attack}};

/* Existing animation-event dispatch, with results discarded by all callers */
void actor_event_call(const ACTOR_EVENT *event, sint32 idx)
{
    if (event)
    {
        switch (event->signature)
        {
            case ACTOR_EVENT_NONE:
                event->fn.none();
                return;
            case ACTOR_EVENT_SINT32:
                event->fn.s32_idx(idx);
                return;
            case ACTOR_EVENT_UINT32:
                event->fn.u32_idx(idx);
                return;
            case ACTOR_EVENT_PICKUP:
                event->fn.pickup(idx, NULL);
                return;
            case ACTOR_EVENT_INTPTR:
                event->fn.intptr_idx(idx);
                return;
            case ACTOR_EVENT_EFFECT:
                event->fn.effect_idx(idx);
                return;
        }
    }
    /* TODO: Investigate an invalid native event instead of calling an encoded address */

    XASSERT(0);
}

GDB_CALL sint32 actor_hit_effects(sint32 idx)
{
    FUNCTION_MARKER(0x800455e4u, "GAME.EXE");
    if ((sint16)actor_animations[idx].event_step)
        effect_spawn_at_attachment(idx);
    else
    {
        actor_apply_animation_damage((sint32)actor_contacts[idx].last_attacker, idx, actor_animations[idx].id);
        if (idx >= 2)
            actors_rec_hit(actor_contacts[idx].last_attacker, idx);
    }
    if (!(actor_animations[idx].motion.events & 0x800u))
        return 0;
    effect_spawn(606, actor_positions[idx].x.bits, 0, actor_positions[idx].z.bits);
    sound_play_hit_voice(idx);
    actor_apply_animation_damage((sint32)actor_contacts[idx].last_attacker, idx, actor_animations[idx].id);
    if (idx < 2)
        return 1;
    return actors_rec_hit(actor_contacts[idx].last_attacker, idx);
}

GDB_CALL void actor_ignore_event(void)
{
    FUNCTION_MARKER(0x80032ed8u, "GAME.EXE");
}

GDB_CALL sint32 actor_attack_voice_event(sint32 idx)
{
    FUNCTION_MARKER(0x80032f38u, "GAME.EXE");
    sint32 type = actor_types[idx];
    if (actor_animations[idx].motion.events & 0x8000u)
    {
        if (idx >= 2)
            return 1;
        actor_trails[idx].phase = 1;
        actor_trails[idx].mode = 2;
        return character_trail_clear(idx + 867);
    }
    if (type == 0 || type == 2)
        return sound_voice_play(48, 80, 6144);
    if (type == 1 || type == 3)
        return sound_voice_play(70, 160, 6144);
    if (type == 49)
        return sound_voice_play(169, 160, 6144);
    return 49;
}

GDB_CALL sint32 actor_attack_fixed_voice_event(sint32 idx)
{
    FUNCTION_MARKER(0x800330ccu, "GAME.EXE");
    if (actor_animations[idx].motion.events & 0x8000u)
    {
        if (idx >= 2)
            return 1;
        actor_trails[idx].phase = 1;
        actor_trails[idx].mode = 2;
        return character_trail_clear(idx + 867);
    }
    return sound_voice_play(22, 255, 6144);
}

GDB_CALL sint32 actor_hit_repeat_event(sint32 idx)
{
    FUNCTION_MARKER(0x800332bcu, "GAME.EXE");
    uint32 x, z, state;
    sint32 attacker;
    x = actor_positions[idx].x.bits;
    z = actor_positions[idx].z.bits;
    effect_spawn(606, x, 0, z);
    attacker = actor_contacts[idx].last_attacker;
    state = actor_animations[idx].id;
    actor_apply_animation_damage(attacker, idx, state);
    if (idx >= 2)
        actors_rec_hit(actor_contacts[idx].last_attacker, idx);
    sound_play_hit_voice(idx);
    attacker = actor_contacts[idx].last_attacker;
    state = actor_animations[idx].id;
    actor_apply_animation_damage(attacker, idx, state);
    return idx < 2 ? 1 : actors_rec_hit(actor_contacts[idx].last_attacker, idx);
}

GDB_CALL sint32 actor_hit_begin_event(sint32 idx)
{
    FUNCTION_MARKER(0x80033488u, "GAME.EXE");
    uint32 x, z, state;
    sint32 attacker;
    if ((sint16)actor_animations[idx].event_step)
        return effect_spawn_at_attachment(idx);
    x = actor_positions[idx].x.bits;
    z = actor_positions[idx].z.bits;
    effect_spawn(606, x, 0, z);
    sound_play_hit_voice(idx);
    attacker = actor_contacts[idx].last_attacker;
    state = actor_animations[idx].id;
    actor_apply_animation_damage(attacker, idx, state);
    if (idx < 2)
        return 1;
    return actors_rec_hit(actor_contacts[idx].last_attacker, idx);
}

GDB_CALL sint32 actor_heavy_hit_event(sint32 idx)
{
    FUNCTION_MARKER(0x80033534u, "GAME.EXE");
    uint32 angle = actor_orientation[idx].yaw, x = actor_positions[idx].x.bits, y = actor_positions[idx].y.bits, z = actor_positions[idx].z.bits;
    uint32 state, effect_x, effect_z;
    sint32 attacker;
    int i;
    if ((sint16)actor_animations[idx].event_step)
        effect_spawn_at_attachment(idx);
    else
    {
        if (actor_animations[idx].id == 189)
        {
            for (i = 0; i < 12; i++)
            {
                effect_spawn_actor_hit_reverse(idx, x, y, z, angle);
                effect_spawn_actor_hit(idx, x, y, z, angle);
            }
            disp_buffers.flash_ticks = 4;
            disp_buffers.shake_ticks = 8;
            sound_voice_play(78, 255, 5632);
        }
        attacker = actor_contacts[idx].last_attacker;
        state = actor_animations[idx].id;
        actor_apply_animation_damage(attacker, idx, state);
        if (idx >= 2)
            actors_rec_hit(actor_contacts[idx].last_attacker, idx);
    }
    if (!(actor_animations[idx].motion.events & 0x800u))
        return 0;
    effect_x = actor_positions[idx].x.bits;
    effect_z = actor_positions[idx].z.bits;
    effect_spawn(606, effect_x, 0, effect_z);
    sound_play_hit_voice(idx);
    attacker = actor_contacts[idx].last_attacker;
    state = actor_animations[idx].id;
    actor_apply_animation_damage(attacker, idx, state);
    if (idx < 2)
        return 1;
    return actors_rec_hit(actor_contacts[idx].last_attacker, idx);
}

GDB_CALL sint32 actor_hit_end_event(sint32 idx)
{
    FUNCTION_MARKER(0x800336ccu, "GAME.EXE");
    uint32 x, z, state;
    sint32 attacker;
    x = actor_positions[idx].x.bits;
    z = actor_positions[idx].z.bits;
    effect_spawn(606, x, 0, z);
    sound_play_hit_voice(idx);
    attacker = actor_contacts[idx].last_attacker;
    state = actor_animations[idx].id;
    actor_apply_animation_damage(attacker, idx, state);
    if (idx < 2)
        return 1;
    return actors_rec_hit(actor_contacts[idx].last_attacker, idx);
}

GDB_CALL sint32 actor_hit_attachment_event(sint32 idx)
{
    FUNCTION_MARKER(0x8003375cu, "GAME.EXE");
    uint32 angle;
    FIXED_VEC3 point;
    uint32 state, effect_x, effect_z;
    sint32 attacker;
    int i;
    if ((sint16)actor_animations[idx].event_step)
        effect_spawn_at_attachment(idx);
    else
    {
        attacker = actor_contacts[idx].last_attacker;
        state = actor_animations[idx].id;
        actor_apply_animation_damage(attacker, idx, state);
        if (idx >= 2)
            actors_rec_hit(actor_contacts[idx].last_attacker, idx);
        actor_attachment_pos(idx, 1, point.bits);
        sound_voice_play(95, 255, 6144);
        dynamic_objects_spawn_transient(865, point.x.bits, point.y.bits, point.z.bits);
        angle = actor_orientation[idx].yaw;
        for (i = 0; i < 4; i++)
            effect_spawn_actor_hit_reverse(idx, point.x.bits, point.y.bits, point.z.bits, angle);
        for (i = 0; i < 12; i++)
            effect_spawn_actor_hit(idx, point.x.bits, point.y.bits, point.z.bits, angle);
    }
    if (!(actor_animations[idx].motion.events & 0x800u))
        return 0;
    effect_x = actor_positions[idx].x.bits;
    effect_z = actor_positions[idx].z.bits;
    effect_spawn(606, effect_x, 0, effect_z);
    return sound_play_hit_voice(idx);
}

GDB_CALL sint32 actor_hit_impact_event(sint32 idx)
{
    FUNCTION_MARKER(0x800338e0u, "GAME.EXE");
    FIXED_VEC3 point;
    uint32 state, effect_x, effect_z;
    sint32 attacker;
    int i;
    if ((sint16)actor_animations[idx].event_step)
        return effect_spawn_at_attachment(idx);
    effect_x = actor_positions[idx].x.bits;
    effect_z = actor_positions[idx].z.bits;
    effect_spawn(606, effect_x, 0, effect_z);
    if (actor_animations[idx].id == 187)
    {
        actor_attachment_pos(idx, 1, point.bits);
        sound_voice_play(35, 255, 6144);
        sound_play_pain(idx);
        disp_buffers.shake_ticks = 16;
        disp_buffers.flash_ticks = 4;
        for (i = 0; i < 12; i++)
            effect_spawn_actor_hit_reverse(idx, point.x.bits, point.y.bits, point.z.bits, actor_orientation[idx].yaw);
        for (i = 0; i < 12; i++)
            effect_spawn_actor_hit(idx, point.x.bits, point.y.bits, point.z.bits, actor_orientation[idx].yaw);
    }
    sound_play_hit_voice(idx);
    attacker = actor_contacts[idx].last_attacker;
    state = actor_animations[idx].id;
    actor_apply_animation_damage(attacker, idx, state);
    if (idx < 2)
        return 1;
    return actors_rec_hit(actor_contacts[idx].last_attacker, idx);
}

GDB_CALL sint32 actor_hit_flash_event(sint32 idx)
{
    FUNCTION_MARKER(0x80033a6cu, "GAME.EXE");
    uint32 angle;
    FIXED_VEC3 point;
    uint32 state, effect_x, effect_z;
    sint32 attacker, attacker_type, sample;
    int i;
    if ((sint16)actor_animations[idx].event_step)
        effect_spawn_at_attachment(idx);
    else
    {
        if (actor_animations[idx].id - 141u < 2u)
        {
            disp_buffers.flash_ticks = 4;
            actor_attachment_pos(idx, 1, point.bits);
            attacker_type = actor_types[(uint32)(sint32)actor_contacts[idx].last_attacker];
            sample = actor_animations[idx].id == 142 && attacker_type == 49 ? 172 : 78;
            sound_voice_play(sample, 255, 6144);
            dynamic_objects_spawn_transient(865, point.x.bits, point.y.bits, point.z.bits);
            angle = actor_orientation[idx].yaw;
            for (i = 0; i < 4; i++)
                effect_spawn_actor_hit_reverse(idx, point.x.bits, point.y.bits, point.z.bits, angle);
            for (i = 0; i < 12; i++)
                effect_spawn_actor_hit(idx, point.x.bits, point.y.bits, point.z.bits, angle);
        }
        attacker = actor_contacts[idx].last_attacker;
        state = actor_animations[idx].id;
        actor_apply_animation_damage(attacker, idx, state);
        actors_rec_hit(actor_contacts[idx].last_attacker, idx);
    }
    if (!(actor_animations[idx].motion.events & 0x800u))
        return 0;
    effect_x = actor_positions[idx].x.bits;
    effect_z = actor_positions[idx].z.bits;
    effect_spawn(606, effect_x, 0, effect_z);
    return sound_play_hit_voice(idx);
}

GDB_CALL sint32 actor_attack_attachment_event(sint32 idx)
{
    FUNCTION_MARKER(0x80033c44u, "GAME.EXE");
    uint32 angle, i;
    FIXED_VEC3 point;
    sint32 kind = actor_types[idx];
    if (!((sint16)actor_animations[idx].event_step))
        return sound_play_attack_voice(idx);
    if (((sint16)actor_animations[idx].event_step) != 1 || kind != 3)
        return 3;
    actor_attachment_pos(idx, 4, point.bits);
    sound_voice_play(95, 255, 6144);
    dynamic_objects_spawn_transient(865, point.x.bits, point.y.bits, point.z.bits);
    angle = actor_orientation[idx].yaw;
    for (i = 0; i < 4; i++)
        effect_spawn_actor_hit_reverse(actor_contacts[idx].last_target, point.x.bits, point.y.bits, point.z.bits, angle);
    for (i = 0; i < 12; i++)
        effect_spawn_actor_hit(actor_contacts[idx].last_target, point.x.bits, point.y.bits, point.z.bits, angle);
    return 0;
}

GDB_CALL uint32 actor_attachment_impact_event(sint32 idx)
{
    FUNCTION_MARKER(0x80033d7cu, "GAME.EXE");
    sint32 count = (sint16)actor_animations[idx].event_step;
    return count ? count : effect_spawn_attachment_particle(idx, 577);
}

GDB_CALL sint32 actor_impact_voice_event(sint32 idx)
{
    FUNCTION_MARKER(0x80033dc8u, "GAME.EXE");
    sint32 val = (sint16)actor_animations[idx].event_step;
    return val ? val : sound_voice_play(28, 255, 6144);
}

GDB_CALL uint32 actor_attachment_particle_event0(sint32 idx)
{
    FUNCTION_MARKER(0x80033e18u, "GAME.EXE");
    return effect_spawn_attachment_particle(idx, 578);
}

GDB_CALL uint32 actor_attachment_particle_event1(sint32 idx)
{
    FUNCTION_MARKER(0x80033e38u, "GAME.EXE");
    return effect_spawn_attachment_particle(idx, 578);
}

GDB_CALL intptr actor_burst_attachment_event(sint32 idx)
{
    FUNCTION_MARKER(0x80033e58u, "GAME.EXE");
    uint32 angle, i;
    FIXED_VEC3 point;
    if (actor_animations[idx].motion.events & 0x800u)
        return (intptr)actor_emit_ground_effect(idx);
    actor_attachment_pos(idx, 8, point.bits);
    dynamic_objects_spawn_transient(865, point.x.bits, point.y.bits, point.z.bits);
    angle = actor_orientation[idx].yaw;
    for (i = 0; i < 4; i++)
        effect_spawn_actor_hit_reverse(idx, point.x.bits, point.y.bits, point.z.bits, angle);
    for (i = 0; i < 12; i++)
        effect_spawn_actor_hit(idx, point.x.bits, point.y.bits, point.z.bits, angle);
    return (uint32)sound_voice_play(59, 255, 6144);
}

GDB_CALL sint32 actor_release_item_event(sint32 idx)
{
    FUNCTION_MARKER(0x800341a0u, "GAME.EXE");
    sint32 slot;
    /* TODO: Investigate an item release outside the native actor pool */
    XASSERT(idx >= 0 && idx < ACTOR_COUNT);
    if (!(actor_animations[idx].motion.events & 0x800u))
        return sound_voice_play(123, 255, 6144);
    slot = actor_items[idx].slot[ACTOR_ITEM_RIGHT];
    if (slot == -1)
        return -1;
    /* TODO: Investigate an item release outside the native dynamic pool */
    XASSERT(slot >= 0 && slot < DYNAMIC_OBJECT_COUNT);
    MODEL_OBJECT *item = dynamic_objects + slot;
    item->kind = item->kind + 1u;
    return actor_items_drop(idx);
}
