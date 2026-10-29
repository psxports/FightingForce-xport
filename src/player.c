#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "game.h"
#include "diagnostics.h"

PLAYER_SCORE player_scores[PLAYER_COUNT];
uint32 player_repeat_counts[PLAYER_COUNT];

sint32 player_step_blocked(sint32 idx)
{
    FUNCTION_MARKER(0x8001a268u, "GAME.EXE");
    uint32 state = actor_animations[idx].id, distance, x, z, other_idx, dx, dz;
    uint32 s, c, count;
    sint32 yaw;
    if (state == 87)
        distance = 352;
    else if (state == 82 || state == 69 || state - 198u < 3u)
        distance = 280;
    else
        return 0;
    yaw = actor_orientation[idx].yaw;
    s = sin_table[yaw];
    c = sin_table[MATH_QUARTER_TURN + yaw];
    x = actor_positions[idx].x.bits;
    z = actor_positions[idx].z.bits;
    count = game_session.player_count;
    x += s * distance;
    z -= c * distance;
    if (count == 2)
    {
        other_idx = 1u - idx;
        dx = x - actor_positions[other_idx].x.bits;
        dz = z - actor_positions[other_idx].z.bits;
        if (actor_health[idx].hp <= 0 || actor_health[other_idx].hp <= 0)
        {
            dx = 0;
            dz = 0;
        }
        if (dx + 0x04000000u > 0x08000000u || dz + 0x04000000u > 0x08000000u)
            return 1;
    }
    return regions_find_obstacle(x, z);
}

/* 199AC: first containing region, including rotated bounds and local-coordinate globals. */

/* 23A48: choose a living state252/253 NPC by wrapped angular distance. */
sint32 player_start_pair_motion(sint32 idx)
{
    FUNCTION_MARKER(0x80023a48u, "GAME.EXE");
    /* TODO: Investigate pair motion outside the native actor pool */
    XASSERT((uint32)idx < ACTOR_COUNT);
    ACTOR_PAIR_MOTION *pair = actor_pair_motion + idx;
    uint32 table, delta;
    sint32 type = actor_types[idx], best = -1, best_angle = 8192, distance, mode = 8;
    int i;
    /* TODO: Investigate pair motion without a valid animation type */
    XASSERT((uint32)type < ANIMATION_TYPE_COUNT);
    table = animation_type_banks[(uint32)type];
    for (i = 2; i < 8; i++)
    {
        if (actor_health[i].hp <= 0 || actor_animations[i].id - 252u >= 2u)
            continue;
        delta = math_dir16(actor_positions[idx].x.whole, actor_positions[idx].z.whole, actor_positions[i].x.whole, actor_positions[i].z.whole);
        delta -= ((uint32)(uint16)actor_orientation[idx].yaw << 5);
        if (dir_distance >= 513)
            continue;
        if (delta & 0x8000u)
            delta = (uint32)(-(sint64)(delta));
        distance = (sint16)delta;
        if (distance < best_angle)
        {
            best_angle = distance;
            best = i;
        }
    }
    if (best == -1)
        return -16;
    actor_orientation[idx].yaw = (uint16)math_dir11(actor_positions[idx].x.whole, actor_positions[idx].z.whole, actor_positions[best].x.whole, actor_positions[best].z.whole);
    pair->x = (uint16)((actor_positions[best].x.whole + actor_positions[idx].x.whole) >> 1);
    pair->z = (uint16)((actor_positions[best].z.whole + actor_positions[idx].z.whole) >> 1);
    pair->span = (uint16)math_distance_newton(actor_positions[idx].x.whole, actor_positions[idx].z.whole, actor_positions[best].x.whole, actor_positions[best].z.whole);
    if (type == 0)
        mode = 9;
    else if (type == 1)
        mode = 15;
    else if (type == 2)
        mode = 13;
    else if (type == 3)
        mode = 14;
    pair->duration = (uint16)mode;
    return animation_start_held(idx, table, 65);
}

/* 1BBE8: strict contact bounds and first matching stage callback. */

/* Projectile contact scan retains actor hit, death and forced-drop reactions */

/* 32F38: attack animation callback, sample selection or attachment reset. */

/* 330CC: same attachment reset as32F38, fixed sample22 on the other event. */

/* 2976C: character voice sample and one RNG draw for pitch. */

/* 29854/32EE0: landing voice, then ground effect607. */

/* 23FD0: preserve19718's region lookup side effects even on a zero result. */

/* Identical effects at24F38..24F78 and26578..265AC, in MIPS call order. */
static void player_special_attack(sint32 idx, uint32 table)
{
    sint32 health = actor_health[idx].hp;
    if (health < 33)
        return;
    actor_health[idx].hp = (uint16)(health - 32);
    actor_combat[idx].special_hits = 0;
    sound_voice_play(46, 255, 6144);
    animation_start_held(idx, table, 33);
    pad_action_clear(idx);
    actor_items_drop(idx);
}

/* 24E24..24FEC: Circle action, including original combination priority. */
static void player_action1(sint32 idx, uint32 table, sint32 state)
{
    sint32 slot;
    uint8 *actions = pad_action.latch[idx];
    if (state != 69 && state != 82 && state != 87 && state != 87)
        return;
    slot = actor_items[idx].slot[ACTOR_ITEM_RIGHT];
    /* TODO: Investigate an invalid player item at the original access */
    XASSERT(slot == -1 || slot >= 0 && slot < DYNAMIC_OBJECT_COUNT);
    if (slot != -1 && (sint32)dynamic_objects[slot].kind >= 522)
    {
        pad_action_clear(idx);
        animation_start_held(idx, table, 161);
        return;
    }
    slot = actor_items[idx].slot[ACTOR_ITEM_LEFT];
    /* TODO: Investigate an invalid player item at the original access */
    XASSERT(slot == -1 || slot >= 0 && slot < DYNAMIC_OBJECT_COUNT);
    if (slot != -1 && (sint32)dynamic_objects[slot].kind >= 522)
    {
        pad_action_clear(idx);
        actor_items_drop(idx);
        return;
    }
    if (actions[0])
    {
        player_special_attack(idx, table);
        return;
    }
    if (actions[2])
    {
        if (!(animation_stream(table, 39).kind != ACTOR_ANIMATION_SOURCE_NONE))
            return;
        animation_start_held(idx, table, 39);
    }
    else if (state != 87 && actor_animations[idx].id != 87)
    {
        actor_movement[idx].speed = 0;
        animation_start_held(idx, table, 158);
    }
    pad_action_clear(idx);
}

/* 25014..2519C: Square action; simultaneous Cross selects animation28. */
static void player_action2(sint32 idx, uint32 table, sint32 state)
{
    uint32 target_state = 0, s, c, x, z;
    sint32 yaw, animation = 26;
    if (state == 69 || state == 82)
    {
        if (pad_action.latch[idx][0])
            animation = 28;
        else
        {
            if (actor_targeting.front.available == 1)
            {
                /* TODO: Investigate a front target outside the native actor pool */
                XASSERT(actor_targeting.actor_idx < ACTOR_COUNT);
                target_state = actor_animations[actor_targeting.actor_idx].id;
            }
            yaw = actor_orientation[idx].yaw;
            /* TODO: Investigate player action headings outside the sine table */
            XASSERT(yaw >= 0 && yaw < MATH_SIN_COUNT);
            s = (uint32)(sint32)sin_table[yaw];
            x = actor_positions[idx].x.bits;
            /* TODO: Investigate player action headings outside the cosine view */
            XASSERT(yaw >= -MATH_QUARTER_TURN && yaw < MATH_ANGLE_COUNT);
            c = (uint32)(sint32)sin_table[MATH_QUARTER_TURN + yaw];
            z = actor_positions[idx].z.bits;
            if (regions_query_alternate_attack(x + (s << 9), z - (c << 9)))
                animation = 28;
            else if (actor_targeting.front.available == 1 && (uint16)((uint16)actor_targeting.front.angle + 16383u) < 32767u && (target_state - 252u < 2u || target_state - 237u < 2u || target_state == 239))
                animation = 28;
        }
        animation_start_held(idx, table, animation);
        pad_action_clear(idx);
    }
    else if (state == 26 && (actor_animations[idx].motion.events & 512u))
    {
        animation_start_held(idx, table, 27);
        pad_action_clear(idx);
    }
}

static void player_action0(sint32 idx, uint32 table, sint32 state)
{
    uint32 target_state = 0, sel, s, c, x, z;
    sint32 animation, slot, ammo, yaw;
    /* 251C4..251E0: running Cross chooses67, without clearing buffered input. */
    if (state == 87)
    {
        animation_start_held(idx, table, 67);
        return;
    }
    if (state == 69 || state == 82)
    {
        item_pickup_select(idx, 96);
        if (col_regions.actor[idx].flags & 15)
        {
            if ((animation_stream(table, 130).kind != ACTOR_ANIMATION_SOURCE_NONE))
            {
                actor_items_drop(idx);
                animation_start_held(idx, table, 130);
            }
            return;
        }
        if (actor_items[idx].bits == 0xffff)
        {
            sel = item_pickup.nearby[idx];
            if (sel != 0xffffffffu)
            {
                /* TODO: Investigate an invalid player item at the original access */
                XASSERT(sel < DYNAMIC_OBJECT_COUNT);
                uint32 kind = dynamic_objects[sel].kind;
                item_pickup.pending[idx] = (sint8)sel; /* 25298 branch delay executes for every kind */
                if (kind - 586u < 12u)
                    return;
                animation = (sint32)kind >= 522 ? 121 : kind - 506u < 2u ? 123 : kind == 515 ? 126 : kind == 504 ? 128 : 118;
                animation_start_held(idx, table, animation);
                pad_action_clear(idx);
                return;
            }
        }
        slot = actor_items[idx].slot[ACTOR_ITEM_RIGHT];
        if (slot != -1)
        {
            /* 2530C..25504: signed halfword kind selects the held-item Cross action. */
            /* TODO: Investigate an invalid player item at the original access */
            XASSERT(slot >= 0 && slot < DYNAMIC_OBJECT_COUNT);
            MODEL_OBJECT *item = dynamic_objects + slot;
            sint32 kind = (sint16)(uint16)item->kind;
            if (kind >= 522)
                animation = 161;
            else if ((uint16)(kind - 496) < 2u)
            {
                animation_start_held(idx, table, 159);
                /* TODO: Investigate an action item without a compatible native owner */
                XASSERT(item->payload_kind == MODEL_PAYLOAD_PHYSICS);
                if (!(item->physics.life))
                {
                    item->physics.life = 270;
                    effect_spawn_attachment_particle(idx, 577);
                }
                pad_action_clear(idx);
                return;
            }
            else if (kind == 513)
                animation = 159;
            else if (kind == 515 || (uint16)(kind - 499) < 5u)
            {
                /* TODO: Investigate an action item without a compatible native owner */
                XASSERT(item->payload_kind == MODEL_PAYLOAD_PHYSICS);
                ammo = item->physics.life;
                if (ammo > 0)
                {
                    item->physics.life = ammo - 1;
                    if (kind == 515)
                    {
                        animation_start_held(idx, table, 193);
                        actor_items_fire_ray(idx);
                    }
                    else
                    {
                        actor_items_fire_spread(idx);
                        animation_start_held(idx, table, 192);
                    }
                }
                else
                    sound_voice_play(26, 255, 6144);
                pad_action_clear(idx);
                return;
            }
            else if (kind == 505)
            {
                /* TODO: Investigate an action item without a compatible native owner */
                XASSERT(item->payload_kind == MODEL_PAYLOAD_PHYSICS);
                ammo = item->physics.life;
                if (ammo > 0)
                {
                    item->physics.life = ammo - 1;
                    actor_items_fire_projectile(idx);
                    animation_start_held(idx, table, 192);
                    actor_animations[idx].event_step = 1;
                }
                else
                    sound_voice_play(26, 255, 6144);
                pad_action_clear(idx);
                return;
            }
            else if ((uint16)(kind - 486) < 3u || kind == 479 || kind == 481 || kind == 483)
                animation = 43;
            else if ((uint16)(kind - 506) < 2u)
            {
                /* TODO: Investigate an action item without a compatible native owner */
                XASSERT(item->payload_kind == MODEL_PAYLOAD_PHYSICS);
                ammo = item->physics.life;
                if (ammo > 0)
                {
                    item->physics.life = ammo - 1;
                    animation_start_held(idx, table, 195);
                    actor_items_fire_projectile(idx);
                }
                else
                    sound_voice_play(26, 255, 6144);
                pad_action_clear(idx);
                return;
            }
            else
                animation = 35;
            animation_start_held(idx, table, animation);
            pad_action_clear(idx);
            return;
        }
        slot = actor_items[idx].slot[ACTOR_ITEM_LEFT];
        if (slot != -1)
        {
            /* TODO: Investigate an invalid player item at the original access */
            XASSERT(slot >= 0 && slot < DYNAMIC_OBJECT_COUNT);
            MODEL_OBJECT *item = dynamic_objects + slot;
            sint32 kind = (sint16)(uint16)item->kind;
            if (kind == 515 || kind == 504)
            {
                /* TODO: Investigate an action item without a compatible native owner */
                XASSERT(item->payload_kind == MODEL_PAYLOAD_PHYSICS);
                ammo = item->physics.life;
                if (ammo > 0)
                {
                    item->physics.life = ammo - 1;
                    if (kind == 515)
                    {
                        animation_start_held(idx, table, 193);
                        actor_items_fire_ray(idx);
                    }
                    else
                    {
                        actor_items_fire_spread(idx);
                        animation_start_held(idx, table, 196);
                    }
                }
                else
                    sound_voice_play(26, 255, 6144);
            }
            else
                animation_start_held(idx, table, 35);
            pad_action_clear(idx);
            return;
        }
        if (actor_targeting.front.available == 1)
        {
            /* TODO: Investigate a front target outside the native actor pool */
            XASSERT(actor_targeting.actor_idx < ACTOR_COUNT);
            target_state = actor_animations[actor_targeting.actor_idx].id;
        }
        yaw = actor_orientation[idx].yaw;
        /* TODO: Investigate player action headings outside the sine table */
        XASSERT(yaw >= 0 && yaw < MATH_SIN_COUNT);
        s = (uint32)(sint32)sin_table[yaw];
        x = actor_positions[idx].x.bits;
        /* TODO: Investigate player action headings outside the cosine view */
        XASSERT(yaw >= -MATH_QUARTER_TURN && yaw < MATH_ANGLE_COUNT);
        c = (uint32)(sint32)sin_table[MATH_QUARTER_TURN + yaw];
        z = actor_positions[idx].z.bits;
        animation = 0;
        if (regions_query_alternate_attack(x + (s << 9), z - (c << 9)))
            animation = 28;
        else if (actor_targeting.front.available == 1 && (uint16)((uint16)actor_targeting.front.angle + 16383u) < 32767u && (target_state - 252u < 2u || target_state - 237u < 2u || target_state == 239))
            animation = 28;
        animation_start_held(idx, table, animation);
        pad_action_clear(idx);
        return;
    }
    if (state == 0 || state == 1)
    {
        if (actor_animations[idx].motion.events & 512)
        {
            animation_start_held(idx, table, state + 1);
            pad_action_clear(idx);
        }
        return;
    }
    /* 256F4..25808: only0/1 and held-item states35/37/43 can chain.
  * All remaining states (including hurt133) preserve buffered input. */
    if ((state == 35 || state == 37 || state == 43) && (actor_animations[idx].motion.events & 512u) && actor_items[idx].slot[ACTOR_ITEM_RIGHT] != -1)
    {
        animation_start_held(idx, table, state == 35 ? 37 : state == 37 ? 38 : 44);
        pad_action_clear(idx);
    }
    return;
}

/* 262B4..262FC: one RNG draw per footstep, followed by the real sound allocator. */
void player_play_footstep(sint32 type)
{
    static const sint32 bases[4] = {52, 72, 87, 72};
    if ((uint32)type >= 4)
        XASSERT(0 && "Original fatal branch 0x800262b4");
    sound_voice_play(bases[type] + (sint32)(random_next() & 3), 255, 6144);
}

/* 1A07C: predict whether this player's step increases separation beyond the
 * two-player boundary. The initial distance call also occurs in one-player mode.
 * Unlike1AEF4's movement bound, this uses inclusive +/-0x4000000. */
sint32 player_step_exceeds_bounds(sint32 idx)
{
    FUNCTION_MARKER(0x8001a07c, "GAME.EXE");
    uint32 other_idx, step, x, z, dx, dz;
    sint32 yaw;
    sint32 prev, state, distance;
    prev = math_distance_newton(actor_positions[0].x.whole, actor_positions[0].z.whole, actor_positions[1].x.whole, actor_positions[1].z.whole);
    state = animation_movement_state(actor_animations[idx].id);
    if (state == 87)
        step = 352;
    else if (state == 82 || state == 69)
        step = 280;
    else if (state == 158)
        step = actor_movement[idx].speed;
    else
        return 0;
    if (game_session.player_count != 2)
        return 0;
    yaw = actor_orientation[idx].yaw;
    other_idx = 1u - idx;
    x = actor_positions[idx].x.bits + (sin_table[yaw]) * step;
    z = actor_positions[idx].z.bits - (sin_table[MATH_QUARTER_TURN + yaw]) * step;
    dx = x - actor_positions[other_idx].x.bits;
    dz = z - actor_positions[other_idx].z.bits;
    distance = math_distance_newton(actor_positions[other_idx].x.whole, actor_positions[other_idx].z.whole, (sint32)x >> 16, (sint32)z >> 16);
    if (actor_health[idx].hp <= 0 || distance < prev || actor_health[other_idx].hp <= 0)
    {
        dx = 0;
        dz = 0;
    }
    return dx + 0x4000000u > 0x8000000u || dz + 0x4000000u > 0x8000000u;
}

/* Digital-pad block 247A4..24994, split out for original-MIPS differential.
 * Returns local B8 (facing) in bit0 and local C0 (backward) in bit1.
 * Includes analog24720 and digital247A4; movement dispatch remains at24AD8. */
static uint32 update_dir(sint32 idx, sint32 state, uint32 *buttons, uint32 locked, uint32 retained[3])
{
    ACTOR_MOVEMENT *movement = actor_movement + idx;
    uint32 dir = *buttons & 240u, desired = movement->desired_yaw, flags = 0, offset, raw, analog_x, analog_y;
    sint32 delta, pad = (sint8)pad_decode.type;
    if (pad == 22)
    {
        if (!pad_decode.x && !pad_decode.y)
        {
            *buttons &= 0xffffff0fu;
            dir = 0;
        }
        else
        {
            *buttons |= 240u;
            dir = 240u;
        }
    }
    if (dir)
    {
        if (pad == 22)
        {
            analog_y = pad_decode.y;
            analog_x = pad_decode.x;
            desired = (uint32)math_dir16(0, 0, analog_x, (uint32)(-(sint64)(analog_y)));
        }
        else
            switch (dir)
            {
                case 16:
                    desired = (uint32)-32768;
                    break;
                case 48:
                    desired = 24576;
                    break;
                case 32:
                    desired = 16384;
                    break;
                case 96:
                    desired = 8192;
                    break;
                case 64:
                    desired = 0;
                    break;
                case 192:
                    desired = (uint32)-8192;
                    break;
                case 128:
                    desired = (uint32)-16384;
                    break;
                case 144:
                    desired = (uint32)-24576;
                    break;
            }
        desired += camera.frame_angles.yaw_bits;
        offset = ((uint32)(uint16)actor_orientation[idx].yaw << 5) - desired;
        if ((uint16)(offset + 8191u) < 16383u)
            flags |= 1;
        if ((uint16)(offset - 24577u) < 16383u)
            flags |= 2;
        raw = actor_animations[idx].id;
        if (raw == 147 || raw - 149u < 2u || raw == 166 || (state == 158 && (sint32)actor_animations[idx].frame < 8))
            desired = movement->desired_yaw;
    }
    /* 49864 saves these caller S0/S1/S2 at entry-SP-40/-36/-32. */
    if (retained)
    {
        retained[0] = desired;
        /* Preserve the original saved S1 word as numeric pickup Y */
        retained[1] = 0x800b4318u + 244u * (uint32)idx;
        retained[2] = dir;
    }
    if (locked)
    {
        *buttons &= 0xffffff0fu;
        return flags;
    }
    if ((flags & 2) && state == 69)
        return flags;
    movement->desired_yaw = (uint16)desired;
    delta = (sint32)((desired - ((uint32)(uint16)actor_orientation[idx].yaw << 5)) << 16) >> 23;
    if (delta > 128)
        delta = 128;
    if (delta < -128)
        delta = -128;
    /* s2 is a nonnegative digital mask: zero delta with held direction adds1. */
    if (!delta && dir)
        delta = 1;
    if (retained)
        retained[0] = (uint32)delta;
    if (dir)
        actor_orientation[idx].yaw = (uint16)(((uint32)(uint16)actor_orientation[idx].yaw + (uint32)delta) & 2047);
    return flags;
}

uint32 player_update_dir(sint32 idx, sint32 state, uint32 *buttons, uint32 locked)
{
    return update_dir(idx, state, buttons, locked, NULL);
}

/* 26108..262A4: movement requested by the animation event2000. */
void player_move_input(sint32 idx, uint32 buttons)
{
    ACTOR_MOVEMENT *movement = actor_movement + idx;
    uint32 desired = movement->desired_yaw, dir = buttons & 240u, speed, offset, x, z, analog_x, analog_y, s, c;
    if ((sint8)pad_decode.type == 22)
    {
        analog_y = pad_decode.y;
        analog_x = pad_decode.x;
        desired = (uint32)math_dir16(0, 0, analog_x, (uint32)(-(sint64)(analog_y)));
        desired += camera.frame_angles.yaw_bits;
        dir = 1;
        analog_x = pad_decode.x;
        analog_y = pad_decode.y;
        speed = math_distance_approx(0, 0, analog_x, analog_y) >> 1;
    }
    else
    {
        if (dir)
        {
            speed = movement->speed;
            if ((sint32)speed < 64)
                movement->speed = speed + 8;
        }
        switch (dir)
        {
            case 16:
                desired = 0xffff8000u;
                break;
            case 48:
                desired = 24576;
                break;
            case 32:
                desired = 16384;
                break;
            case 96:
                desired = 8192;
                break;
            case 64:
                desired = 0;
                break;
            case 192:
                desired = 0xffffe000u;
                break;
            case 128:
                desired = 0xffffc000u;
                break;
            case 144:
                desired = 0xffffa000u;
                break;
        }
        desired += camera.frame_angles.yaw_bits;
        speed = movement->speed;
    }
    if (!dir)
        return;
    offset = (desired >> 4) & 4094u;
    s = (uint32)(sint32)sin_table[offset >> 1];
    x = actor_positions[idx].x.bits + s * speed;
    c = (uint32)(sint32)sin_table[MATH_QUARTER_TURN + (offset >> 1)];
    z = actor_positions[idx].z.bits - c * speed;
    actor_correct_obstacles(idx, &x, &z);
    if (!player_step_exceeds_bounds(idx))
    {
        actor_positions[idx].x.bits = x;
        actor_positions[idx].z.bits = z;
    }
}

static void player_target_turn(sint32 idx, const ACTOR_TARGET_DIRECTION *dir)
{
    if (dir->available)
    {
        uint32 angle = (uint16)dir->angle;
        if ((uint16)(angle + 8191u) < 16383u)
            actor_orientation[idx].yaw = (uint16)(((uint32)((uint16)actor_orientation[idx].yaw) + (uint32)((sint32)(angle << 16) >> 24)) & 2047);
    }
}

/* 24008 live unarmed idle and133/134 hit-reaction control paths.
 * Reaction path follows the same no-input dispatch until animation completion;
 * 2758C then branches to27CB0 and returns to state69 via237CC.
 * Evidence: player-hit186-trace.json records all97 reached transfers.
 * Conditions outside the translated
 * path stop at a named original PC; this is not complete player logic.
 * Evidence: player-first-path.lst, actual MIPS and first-call differential. */
/* 23DC0: acquire a facing, eligible NPC for the grapple animation pair. */
sint32 player_try_grapple(sint32 idx)
{
    FUNCTION_MARKER(0x80023dc0u, "GAME.EXE");
    uint32 other_idx, table;
    sint32 type = actor_types[idx], state, result, sample = -1;
    table = animation_type_banks[(uint32)type];
    if ((uint16)((uint16)actor_targeting.front.angle + 0x2000u) >= 0x4001u)
        return 0;
    other_idx = actor_targeting.actor_idx;
    if (actor_types[other_idx] == 50 || actor_companions[other_idx] != -1)
        return -1;
    state = npc_base_animation(other_idx);
    result = actor_health[other_idx].hp;
    if (result <= 0)
        return result;
    result = actor_contacts[other_idx].captor;
    if (result != -1)
        return result;
    if ((uint32)state != npc_behavior_actors[other_idx].idle_animation && state - 57u >= 8u && state != 236 && state != 82 && state != 103 && state != 113 && state != 108)
        return 108;
    actor_items_drop(actor_targeting.actor_idx);
    actor_items_drop(idx);
    actor_contacts[idx].grapple_mode = 0;
    actor_contacts[other_idx].captor = (uint8)idx;
    actor_contacts[other_idx].grapple_mode = 0;
    actor_contacts[idx].captive = (uint8)actor_targeting.actor_idx;
    actor_contacts[other_idx].escape_ticks = 50;
    if (type == 0)
        sample = 47;
    else if (type == 1)
        sample = 67;
    else if (type == 2)
        sample = 83;
    else if (type == 3)
        sample = 103;
    if (sample >= 0)
        sound_voice_play(sample, 255, 6144);
    animation_start_held(idx, table, 166);
    return animation_start_held(other_idx, animation_type_banks[(uint32)actor_types[other_idx]], 171);
}

/* 27A8C..27B14: retained S6, not a fresh partner lookup after callbacks. */
static void player_grapple_pair_end(sint32 idx, uint32 table, sint32 partner, sint32 state)
{
    if (state == 167)
    {
        animation_start_held(partner, animation_type_banks[(uint32)actor_types[partner]], 77);
        animation_start_held(idx, table, 169);
        actors_capture_pair_offset(idx, partner);
    }
    else
    {
        animation_start_held(idx, table, 169);
        animation_start_held(partner, animation_type_banks[(uint32)actor_types[partner]], 77);
    }
}

/* 26DF8..27240: paired attack completion, including signed wrapped combo counts.
 * Called only for states3/4/34/41,10..17,19..23,40. Callee order and link reloads matter. */
static void player_grapple_combo_end(sint32 idx, uint32 table, sint32 state)
{
    ACTOR_COMBAT *combat = actor_combat + idx;
    sint32 partner;
    sint32 other_idx;
    uint32 count;
    uint8 *actions = pad_action.latch[idx];
    if (state == 3 || state == 4 || state == 34 || state == 41)
    {
        animation_start_held(idx, table, 69);
        return;
    }
    if (state == 14 || state == 15)
    {
        sint32 pressed = actions[0];
        if (pressed)
            partner = actor_contacts[idx].captive;
        if (pressed && partner != -1 && actor_health[partner].hp > 0)
        {
            if (state == 14)
            {
                pad_action_clear(idx);
                animation_start_held(idx, table, 15);
                return;
            }
            count = (uint16)(combat->combo_step + 1);
            combat->combo_step = (uint16)count;
            if ((sint16)count < 2)
            {
                pad_action_clear(idx);
                animation_start_held(idx, table, 14);
                return;
            }
        }
        animation_start_held(idx, table, state == 14 ? 16 : 17);
        return;
    }
    partner = actor_contacts[idx].captive;
    if (state == 16 || state == 17)
    {
        if (partner != -1)
        {
            other_idx = partner;
            actor_contacts[other_idx].captor = ACTOR_SLOT_NONE;
            actor_contacts[other_idx].grapple_mode = 0;
            animation_start_held(other_idx, animation_type_banks[(uint32)actor_types[other_idx]], 69);
        }
        actor_contacts[idx].captive = ACTOR_SLOT_NONE;
        animation_start_held(idx, table, 69);
        return;
    }
    if (partner == -1)
    {
        animation_start_held(idx, table, 69);
        return;
    }
    if (state == 10 || state == 19)
    {
        animation_start_held(idx, table, state == 10 ? 11 : 20);
        return;
    }
    if (state == 11 || state == 20)
    {
        if (actions[0])
        {
            animation_start_held(idx, table, state == 11 ? 12 : 21);
            pad_action_clear(idx);
        }
        else
            animation_start_held(idx, table, state == 11 ? 13 : 22);
        return;
    }
    if (state == 12)
    {
        count = (uint16)(combat->combo_step + 1);
        combat->combo_step = (uint16)count;
        if ((sint16)count < 3)
        {
            animation_start_held(idx, table, 11);
            return;
        }
        other_idx = actor_contacts[idx].captive;
        animation_start_held(other_idx, animation_type_banks[(uint32)actor_types[other_idx]], 171);
        animation_start_held(idx, table, 34);
        actor_contacts[idx].captive = ACTOR_SLOT_NONE;
        actor_contacts[other_idx].captor = ACTOR_SLOT_NONE;
        actor_status[(other_idx)].deadline = game_session.tick + 15u;
        return;
    }
    if (state == 21)
    {
        if (!actions[0])
        {
            animation_start_held(idx, table, 23);
            return;
        }
        count = (uint16)(combat->combo_step + 1);
        combat->combo_step = (uint16)count;
        if ((sint16)count < 2)
            animation_start_held(idx, table, 20);
        else
        {
            animation_start_held(idx, table, 23);
            other_idx = actor_contacts[idx].captive;
            actor_contacts[other_idx].captor = ACTOR_SLOT_NONE;
            actor_contacts[idx].captive = ACTOR_SLOT_NONE;
        }
        pad_action_clear(idx);
        return;
    }
    /* 13/40/22/23 return to the hold while still linked. */
    animation_start_held(idx, table, 74);
}

/* 26634..26754: state169 paired hold; Square is reloaded after Cross clears input. */
static void player_grapple169(sint32 idx, uint32 table, sint32 type, sint32 partner)
{
    uint8 *actions = pad_action.latch[idx];
    if (partner == -1)
    {
        animation_start_held(idx, table, 69);
        return;
    }
    actors_apply_pair_offset(idx, partner);
    if (actions[0])
    {
        pad_action_clear(idx);
        if (type == 2)
        {
            animation_start_held(idx, table, 178);
            actor_contacts[idx].captive = ACTOR_SLOT_NONE;
            animation_start_held(partner, animation_type_banks[(uint32)actor_types[partner]], 184);
            actor_contacts[partner].captor = ACTOR_SLOT_NONE;
            actor_contacts[partner].last_attacker = (uint8)idx;
        }
    }
    if (actions[2])
    {
        pad_action_clear(idx);
        if (type == 2)
        {
            animation_start_held(idx, table, 179);
            actor_contacts[idx].captive = ACTOR_SLOT_NONE;
            animation_start_held(partner, animation_type_banks[(uint32)actor_types[partner]], 189);
            actor_contacts[partner].captor = ACTOR_SLOT_NONE;
            actor_contacts[partner].last_attacker = (uint8)idx;
        }
    }
}

/* 26764..26D50: state74 grapple controls; input priority3,1,0,2. */
static void player_grapple74(sint32 idx, uint32 table, sint32 type, uint32 directions, sint32 partner)
{
    sint32 mine = 0, theirs = 0;
    uint8 *actions = pad_action.latch[idx];
    if (partner == -1)
    {
        animation_start_held(idx, table, 69);
        return;
    }
    if (directions & 2u)
    {
        animation_start_held(idx, table, 69);
        actor_contacts[partner].captor = ACTOR_SLOT_NONE;
        animation_start_held(partner, animation_type_banks[(uint32)actor_types[partner]], 69);
        actor_contacts[idx].captive = ACTOR_SLOT_NONE;
        return;
    }
    if (actions[3])
    {
        pad_action_clear(idx);
        if (type == 2)
        {
            actor_contacts[partner].grapple_mode = 1;
            animation_start_held(partner, animation_type_banks[(uint32)actor_types[partner]], 172);
            animation_start_held(idx, table, 167);
            return;
        }
        if (type == 1)
        {
            theirs = (directions & 1u) ? 188 : 186;
            mine = (directions & 1u) ? 179 : 178;
        }
        else if (type == 3)
        {
            theirs = (directions & 1u) ? 142 : 185;
            mine = (directions & 1u) ? 179 : 178;
        }
        else if (type == 0)
        {
            if (!(directions & 1u))
            {
                animation_start_held(partner, animation_type_banks[(uint32)actor_types[partner]], 171);
                animation_start_held(idx, table, 34);
                actor_contacts[idx].captive = ACTOR_SLOT_NONE;
                actor_contacts[partner].captor = ACTOR_SLOT_NONE;
                actor_status[(partner)].deadline = game_session.tick + 15u;
                return;
            }
            theirs = 187;
            mine = 179;
        }
        else
            return;
    }
    else if (actions[1])
    {
        pad_action_clear(idx);
        actor_contacts[idx].captive = ACTOR_SLOT_NONE;
        theirs = type == 2 ? 181 : type == 1 ? 182 : type == 3 ? 183 : 180;
        animation_start_held(partner, animation_type_banks[(uint32)actor_types[partner]], theirs);
        if (type == 3)
        {
            actor_contacts[idx].last_target = (uint8)partner;
            actor_contacts[idx].target_ticks = 0;
        }
        animation_start_held(idx, table, 177);
        actor_contacts[partner].captor = ACTOR_SLOT_NONE;
        actor_contacts[partner].last_attacker = (uint8)idx;
        return;
    }
    else if (actions[0])
    {
        pad_action_clear(idx);
        actor_combat[idx].combo_step = 0; /* branch delay for every type */
        if (type == 0 || type == 1)
        {
            animation_start_held(idx, table, type == 0 ? 10 : 19);
            return;
        }
        if (type == 2)
        {
            animation_start_held(partner, animation_type_banks[(uint32)actor_types[partner]], 141);
            animation_start_held(idx, table, 3);
            actor_contacts[partner].last_attacker = (uint8)idx;
            actor_contacts[partner].captor = ACTOR_SLOT_NONE;
            actor_contacts[idx].captive = ACTOR_SLOT_NONE;
            actor_contacts[idx].last_target = (uint8)partner;
            actor_contacts[idx].target_ticks = 0;
            return;
        }
        if (type == 3)
        {
            animation_start_held(partner, animation_type_banks[(uint32)actor_types[partner]], 174);
            animation_start_held(idx, table, 168);
            actor_contacts[idx].grapple_mode = 2;
        }
        return;
    }
    else if (actions[2])
    {
        pad_action_clear(idx);
        if (type == 0)
        {
            animation_start_held(idx, table, 40);
            return;
        }
        if (type == 1 || type == 3)
        {
            theirs = 140;
            mine = 41;
        }
        else if (type == 2)
        {
            theirs = 142;
            mine = 4;
        }
        else
            return;
    }
    else
        return;
    animation_start_held(partner, animation_type_banks[(uint32)actor_types[partner]], theirs);
    animation_start_held(idx, table, mine);
    actor_contacts[partner].last_attacker = (uint8)idx;
    actor_contacts[partner].captor = ACTOR_SLOT_NONE;
    actor_contacts[idx].captive = ACTOR_SLOT_NONE;
}

/* Extracted 24408..2452C block; item kind 587..596 was checked by caller. */
static void collect_item(MODEL_OBJECT *item, sint32 idx, uint32 kind)
{
    sint32 amount = 0, health = actor_health[idx].hp, maximum;
    if (health <= 0)
        return;
    if (kind == 587)
        amount = 160;
    else if (kind == 588)
        amount = 128;
    else if (kind == 589)
        amount = 96;
    else if (kind == 590 || kind == 591)
        amount = 64;
    if (amount)
    {
        if (kind != 587)
            health = (uint16)actor_health[idx].hp;
        actor_health[idx].hp = (uint16)(health + amount);
    }
    if (kind - 593u < 4u)
    {
        XASSERT(idx >= 0 && idx < PLAYER_COUNT);
        player_scores[idx].val += 500u * (kind - 592u);
        hud_format_score(idx);
    }
    item->kind = 0xffffffffu;
    sound_voice_play(46, 255, 7168);
    health = actor_health[idx].hp;
    maximum = actor_health[idx].max;
    if (health >= maximum)
        actor_health[idx].hp = (uint16)(maximum - 4);
}

/* 25A8C..25E58: hit sounds depend on S0 (saved state), not a reloaded pose. */
static void player_contact_sound(sint32 type, sint32 state, sint32 attachment)
{
    sint32 sample = -1;
    if (attachment == 2)
    {
        if (type == 0)
        {
            if (state == 1)
                sample = 34;
            else if (state == 11)
                sample = 44;
            else if (state == 24)
                sample = 33;
            else if (state == 25)
                sample = 35;
        }
        else if (type == 1)
        {
            if (state == 1)
                sample = 58;
            else if (state == 11 || state == 24)
                sample = 57;
            else if (state == 25)
                sample = 59;
        }
        else if (type == 2)
        {
            if (state == 1 || state == 66)
                sample = 77;
            else if (state == 11 || state == 24)
                sample = 76;
            else if (state == 25)
                sample = 78;
        }
        else if (type == 3)
        {
            if (state == 1 || state == 15)
                sample = 94;
            else if (state == 11 || state == 24)
                sample = 93;
            else if (state == 25)
                sample = 95;
        }
    }
    else
    {
        if (type == 0)
        {
            if (state == 0)
                sample = 33;
            else if (state == 2)
                sample = 35;
            else if (state == 25)
                sample = 34;
        }
        else if (type == 1)
        {
            if (state == 0 || state == 20)
                sample = 57;
            else if (state == 2)
                sample = 59;
            else if (state == 25 || state == 21)
                sample = 58;
        }
        else if (type == 2)
        {
            if (state == 0)
                sample = 76;
            else if (state == 2)
                sample = 78;
            else if (state == 25 || state == 66)
                sample = 77;
        }
        else if (type == 3)
        {
            if (state == 0 || state == 14)
                sample = 93;
            else if (state == 2)
                sample = 95;
            else if (state == 25)
                sample = 94;
        }
    }
    if (sample >= 0)
        sound_voice_play(sample, 255, 6144);
}

/* 245DC..246FC: the original lock predicate, including the state169 override. */
static uint32 player_dir_locked(sint32 idx, sint32 state)
{
    uint32 n = state;
    uint32 locked = state == 147 || n - 149u < 2u || n - 177u < 2u || state == 179 || state == 118 || state == 121 || n - 123u < 2u || state == 126 || n - 66u < 2u || state == 65 || state == 92 || state == 34 || state == 41 || n - 3u < 2u || state == 33 || n - 170u < 2u || state == 173 || n - 180u < 2u || n - 182u < 2u || n - 184u < 2u || state == 186 || n - 130u < 2u || actor_contacts[idx].captive != -1 || actor_health[idx].hp <= 0;
    return state == 169 ? 0 : locked;
}

/* 2588C..25A68: events and state are captured before any contact callback. */
static void early_contacts(sint32 idx, sint32 type, sint32 state, uint32 events)
{
    sint32 sample;
    if (events & 0x800u)
        actor_animations[idx].hit_flags = 0;
    if (events & 1u)
        if (actor_process_attachment_hits(idx, 1) == 1)
            if ((uint32)type < 4u)
                sound_voice_play(43, 255, 6144);
    if (events & 32u)
        if (actor_process_attachment_hits(idx, 6) == 1)
        {
            sample = -1;
            if (state == 24)
            {
                if (type == 0)
                    sample = 33;
                else if (type == 1)
                    sample = 57;
                else if (type == 2)
                    sample = 76;
                else if (type == 3)
                    sample = 93;
            }
            else if (type == 2 && state == 66)
                sample = 77;
            if (sample >= 0)
                sound_voice_play(sample, 255, 6144);
        }
    if (events & 64u)
        if (actor_process_attachment_hits(idx, 7) == 1)
        {
            sample = -1;
            if (state == 25)
            {
                if (type == 0)
                    sample = 34;
                else if (type == 1)
                    sample = 58;
                else if (type == 2)
                    sample = 77;
                else if (type == 3)
                    sample = 94;
            }
            else if (type == 2 && state == 66)
                sample = 77;
            else if (type == 3 && state == 2)
                sample = 95;
            if (sample >= 0)
                sound_voice_play(sample, 255, 6144);
        }
}

sint32 player_update(void)
{
    FUNCTION_MARKER(0x80024008, "GAME.EXE");
    sint32 idx = 0, count = game_session.player_count, type, partner, state, timer, pose_step, contact_state;
    uint32 table, buttons, dir_flags, contact_events, dir_locked;
    uint8 *actions;
    uint32 pickup_words[3], pickup_context, pickup_state;
    uint32 retained_partner_idx, completion_ready;
    if (count <= 0)
        return count;
    do
    {
        /* TODO: Investigate a player count outside the native actor pool */
        XASSERT((uint32)idx < ACTOR_COUNT);
        ACTOR_CONTACT *contact = actor_contacts + idx;
        ACTOR_LIFECYCLE *life = actor_lifecycle + idx;
        ACTOR_STATUS *status = actor_status + idx;
        ACTOR_COMBAT *combat = actor_combat + idx;
        type = actor_types[idx];
        table = animation_type_banks[(uint32)type];
        partner = contact->captive;
        if (type == 2 && actor_animations[idx].id != 67)
        {
            sint32 handle = combat->run_voice;
            if (handle > 0)
            {
                sound_voice_stop(handle - 1);
                combat->run_voice = 0;
            }
        }
        if (actor_types[idx] == ACTOR_TYPE_NONE)
            goto next_actor;
        timer = life->delay;
        if (timer)
            life->delay = (uint16)(timer - 1);
        /* Resolve death and captivity before accepting player actions */
        if ((sint32)game_session.player_count >= 2)
        {
            timer = (sint8)life->death_ticks;
            if (timer)
            {
                timer = (sint8)(timer + 1);
                life->death_ticks = (uint8)timer;
                if (timer >= 63)
                {
                    life->death_ticks = 62;
                    actor_animations[idx].src = (ACTOR_ANIMATION_SOURCE){ACTOR_ANIMATION_SOURCE_NONE, {0}};
                }
                goto next_actor;
            }
            if (actor_health[idx].hp <= 0)
            {
                timer = contact->captive;
                if (timer != -1)
                {
                    actor_contacts[(uint32)timer].captor = ACTOR_SLOT_NONE;
                    contact->captive = ACTOR_SLOT_NONE;
                }
                life->death_ticks = 1;
                goto next_actor;
            }
        }
        if (status->flags & ACTOR_ELECTRIC)
        {
            actors_emit_arc(idx);
            if ((sint32)status->deadline < (sint32)game_session.tick)
                status->flags = status->flags & 0xfffffffeu;
        }
        /* 241C8..242E4: held actor handles escape input, then finishes this actor. */
        if (contact->captor != -1)
        {
            actor_items_drop(idx);
            actor_trails[idx].phase = 0;
            if (contact->grapple_mode == 1)
                goto next_actor;
            actors_update_grapple(contact->captor, idx);
            if (actor_animations[idx].frame == 0xffffffffu)
                animation_start_held(idx, animation_type_banks[(uint32)type], 171);
            buttons = pad_game_buttons(pad_game.port[idx]);
            pad_action_press(idx, ~pad_game.prev[idx] & buttons);
            pad_action_hold(idx, buttons);
            if (pad_action.latch[idx][0])
            {
                timer = contact->captor;
                if (timer != -1)
                {
                    contact->captor = ACTOR_SLOT_NONE;
                    actor_contacts[(uint32)timer].captive = ACTOR_SLOT_NONE;
                }
            }
            goto next_actor;
        }
        /* 242E8..24340 initializes saved S6, including the slot7 fallback. */
        timer = contact->captive;
        retained_partner_idx = timer == -1 ? ACTOR_COUNT - 1 : (uint32)timer;
        if (timer != -1 && actor_health[retained_partner_idx].hp <= 0)
        {
            contact->captive = ACTOR_SLOT_NONE;
            animation_start_held(idx, table, 69);
        }
        /* Resolve nearby equipment before decoding this frame of input */
        item_pickup_select(idx, 96);
        {
            sint32 sel = item_pickup.nearby[idx];
            if (sel >= 0)
            {
                /* TODO: Investigate an invalid player item at the original access */
                XASSERT(sel < DYNAMIC_OBJECT_COUNT);
                MODEL_OBJECT *nearby = dynamic_objects + sel;
                uint32 kind = nearby->kind;
                /* 2438C/24400: ordinary equipment stays selected for the action dispatch. */
                if (kind == 598)
                {
                    sint32 right = actor_items[idx].slot[ACTOR_ITEM_RIGHT];
                    if (right != -1)
                    {
                        /* TODO: Investigate an invalid player item at the original access */
                        XASSERT(right >= 0 && right < DYNAMIC_OBJECT_COUNT);
                        MODEL_OBJECT *weapon = dynamic_objects + right;
                        if (weapon->kind - 499 < 5)
                        {
                            /* TODO: Investigate a player item access with a foreign native owner */
                            XASSERT(weapon->payload_kind == MODEL_PAYLOAD_PHYSICS);
                            if (weapon->physics.life < 5)
                            {
                                weapon->physics.life = 5;
                                sound_voice_play(28, 255, 6144);
                                nearby->kind = 0xffffffff;
                            }
                        }
                    }
                }
                if (kind - 587u < 10u)
                    collect_item(nearby, idx, kind);
            }
        }
        buttons = pad_game_buttons(pad_game.port[idx]);
        if ((sint32)level_transition.route_exit_tick > 0 || (game_session.mode == 1 && (sint32)game_session.tick < 60) || (game_session.stage == 4 && (sint32)game_session.tick < 660))
        {
            buttons = 0;
            pad_decode.y = 0;
            pad_decode.x = 0;
        }
        /* Preserve input sampling and action-history order */
        state = animation_movement_state(actor_animations[idx].id);
        dir_locked = player_dir_locked(idx, state);
        dir_flags = update_dir(idx, state, &buttons, dir_locked, pickup_words);
        pad_action_press(idx, ~pad_game.prev[idx] & buttons);
        pad_action_hold(idx, buttons);
        actions = pad_action.latch[idx];
        if (level_transition.finish_tick)
        {
            actions[3] = 0;
            actions[2] = 0;
            actions[0] = 0;
        }
        pickup_state = actor_animations[idx].id;
        pickup_context = pickup_state - 130u < 2u && dir_locked && partner == -1 && actor_items[idx].slot[ACTOR_ITEM_LEFT] == -1 && actor_items[idx].slot[ACTOR_ITEM_RIGHT] == -1;
        /* Facing targets are selected from the pre-dispatch actor state */
        actors_find_facing_targets(idx);
        if (partner != -1 || dir_locked)
        {
            actor_targeting.rear.available = 0;
            actor_targeting.front.available = 0;
        }
        player_target_turn(idx, &actor_targeting.front);
        player_target_turn(idx, &actor_targeting.rear);
        if ((buttons & 240) && (state = animation_movement_state(actor_animations[idx].id)) == 69)
        {
            if (actions[5])
            {
                actor_animations[idx].id = 87;
                if (!player_step_exceeds_bounds(idx))
                {
                    actor_animations[idx].id = state;
                    animation_start_held(idx, table, 87);
                    if (animation_movement_state(actor_animations[idx].id) != 87)
                        animation_start_held(idx, table, 82);
                }
            }
            else
            {
                actor_animations[idx].id = 82;
                if (dir_flags & 2)
                    animation_start_held(idx, table, 92);
                else if (!player_step_exceeds_bounds(idx))
                {
                    animation_start_held(idx, table, 82);
                    /* Original24BDC loop deliberately advances the same pose twelve times. */
                    if (type == 0 || type == 2)
                        for (pose_step = 0; pose_step < 12; pose_step++)
                            actor_pose_decode(model_refs[(uint32)type], idx, &actor_poses[idx]);
                }
            }
        }
        state = animation_movement_state(actor_animations[idx].id);
        if ((sint16)actor_trails[idx].phase)
        {
            sint32 attachment = pickup_words[2];
            if (type == 0)
                attachment = 4;
            else if (type == 1)
                attachment = 5;
            else if (type == 3)
                attachment = actor_animations[idx].id == 39 ? 4 : 5;
            else if (type == 2)
                attachment = 3;
            character_trail_append(idx, attachment);
            {
                sint32 selector = actor_trails[idx].mode;
                uint32 trail = actor_trails[idx].phase;
                static const uint16 steps[4] = {0, 1, 2, 2};
                uint32 step = steps[selector];
                actor_trails[idx].phase = (uint16)(trail + step);
            }
        }
        /* 24CB0..24CE8: held right item only enters this dispatch on Triangle/Square. */
        if (partner == -1 && actor_items[idx].slot[ACTOR_ITEM_RIGHT] != -1 && (actions[3] || actions[2]))
        {
            if (animation_movement_state(actor_animations[idx].id) == 69)
            {
                sint32 animation = actor_animations[idx].id == 70 ? 161 : 159;
                pad_action_clear(idx);
                animation_start_held(idx, table, animation);
            }
        }
        /* 24D48..24DFC: left item uses a separate drop/put-away animation. */
        if (partner == -1 && actor_items[idx].slot[ACTOR_ITEM_LEFT] != -1 && (actions[3] || actions[2]))
        {
            if (animation_movement_state(actor_animations[idx].id) == 69)
            {
                if (actor_animations[idx].id == 78)
                {
                    pad_action_clear(idx);
                    animation_start_held(idx, table, 163);
                }
                else
                {
                    pad_action_clear(idx);
                    actor_items_drop(idx);
                    animation_start_held(idx, table, 69);
                }
            }
        }
        if (actions[1])
            player_action1(idx, table, state);
        if (actions[2])
            player_action2(idx, table, state);
        if (actions[0])
            player_action0(idx, table, state);
        /* 25828..2587C: Triangle idle24, then buffered event200 chain25. */
        if (actions[3])
        {
            if (state == 69)
            {
                pad_action_clear(idx);
                animation_start_held(idx, table, 24);
            }
            else if (state == 24 && (actor_animations[idx].motion.events & 512u))
            {
                pad_action_clear(idx);
                animation_start_held(idx, table, 25);
            }
        }
        contact_state = actor_animations[idx].id;
        contact_events = actor_animations[idx].motion.events; /* 25880/25884: saved across calls. */
        early_contacts(idx, type, contact_state, contact_events);
        if (contact_events & 2u)
            if (actor_process_attachment_hits(idx, 2) == 1)
                player_contact_sound(type, contact_state, 2);
        if (contact_events & 4u)
            if (actor_process_attachment_hits(idx, 3) == 1)
                player_contact_sound(type, contact_state, 3);
        /* 25E60: attachment4 contact event precedes animation sound callbacks. */
        if (contact_events & 8u)
            if (actor_process_attachment_hits(idx, 4) == 1)
            {
                sint32 sample = -1;
                if (type == 0)
                    sample = contact_state == 26 ? 36 : 37;
                else if (type == 1 || type == 3)
                    sample = 60;
                else if (type == 2)
                    sample = 36;
                if (sample >= 0)
                    sound_voice_play(sample, 255, 6144);
            }
        /* 25ECC..25F24: attachment5 follows attachment4; sound only on contact. */
        if (contact_events & 16u)
            if (actor_process_attachment_hits(idx, 5) == 1)
            {
                sint32 sample = -1;
                if (type == 0 || type == 2)
                    sample = 37;
                else if (type == 1 || type == 3)
                    sample = 61;
                if (sample >= 0)
                    sound_voice_play(sample, 255, 6144);
            }
        /* 25F28..25FEC: attachment8 follows attachment5. State40 contact sound. */
        if (contact_events & 0x80u)
            if (actor_process_attachment_hits(idx, 8) == 1)
                if ((uint32)type < 4u && actor_animations[idx].id == 40)
                    sound_voice_play(45, 255, 6144);
        /* 25FEC..25FFC: attachment9 after attachment8, without a contact sound. */
        if (contact_events & 0x100u)
            actor_process_attachment_hits(idx, 9);
        /* 26000..260B8: try full reach, then half reach only after a miss. */
        if (contact_events & 0x400u)
        {
            sint32 contact = actor_items_hit_weapon(idx, 0);
            if (contact == 3)
                sound_voice_play(33 + (sint32)random_next() % 3, 255, 6144);
            if (!contact && actor_items_hit_weapon(idx, 1) == 3)
                sound_voice_play(33 + (sint32)random_next() % 3, 255, 6144);
        }
        if (contact_events & 0x1000u)
        {
            uint32 event_state = actor_animations[idx].id & 0x3fffffff;
            XASSERT(event_state < ACTOR_EVENT_SLOT_COUNT);
            const ACTOR_EVENT *callback = actor_events[event_state];
            /* Trail geometry uses separate scratchpad matrices and stack locals
             * Pickup retains the S0/S1/S2 words saved by49864 */
            if (callback == &actor_event_pickup && pickup_context && actor_animations[idx].id == pickup_state && contact_events == 0x1800u && actor_items[idx].slot[ACTOR_ITEM_LEFT] == -1 && actor_items[idx].slot[ACTOR_ITEM_RIGHT] == -1)
                regions_update_pickup(idx, pickup_words);
            else
                actor_event_call(callback, idx);
            actor_animations[idx].event_step = (uint16)(actor_animations[idx].event_step + 1);
        }
        if (contact_events & 0x2000u)
            player_move_input(idx, buttons);
        if (contact_events & 0x4000)
            player_play_footstep(type);
        state = animation_movement_state(actor_animations[idx].id);
        completion_ready = 0;
        if (state == 82)
        {
            if (!(buttons & 240))
                animation_start_held(idx, table, 69);
            if (player_step_exceeds_bounds(idx))
                animation_start_held(idx, table, 69);
            if (actions[5])
                animation_start_held(idx, table, 87);
            if (actions[3] && !(dir_flags & 2) && actor_targeting.front.available == 1 && actor_targeting.near_distance < 192)
            {
                player_try_grapple(idx);
                pad_action_clear(idx); /* 26410 -> 265EC, even if acquisition failed. */
            }
        }
        else if (state == 87)
        {
            if (!(buttons & 240))
                animation_start_held(idx, table, 69);
            if (player_step_exceeds_bounds(idx))
                animation_start_held(idx, table, 69);
            if (actions[2] && actor_movement[idx].prev_state == 87)
            {
                animation_start_held(idx, table, 66);
                pad_action_clear(idx);
            }
        }
        else if (state == 39)
        {
            /* 2649C..264D4: event2000 invokes environmental contact. */
            if (actor_animations[idx].motion.events & 0x2000u)
                world_objects_break_near_attachment(idx, type == 2 ? 8 : 4);
        }
        else if (state == 158)
        {
            /* 264E0..26524: acquisition is followed by input consumption even on failure. */
            if ((actor_animations[idx].motion.events & 512u) && actions[3])
            {
                player_start_pair_motion(idx);
                pad_action_clear(idx);
            }
            sint32 frame = actor_animations[idx].frame;
            if (frame < 8)
            {
                if (frame == -1)
                    completion_ready = 1;
                else if (actions[0])
                    player_special_attack(idx, table);
                else if (actions[2])
                {
                    /* 265D8..265F8: Circle-to-Square transition, actual Alana tick173. */
                    actor_combat[idx].special_hits = 0;
                    animation_start_held(idx, table, 39);
                    pad_action_clear(idx);
                }
            }
        }
        else if (state == 13)
        {
            if (contact->captive == -1)
                animation_start_held(idx, table, 69);
        }
        else if (state - 14u < 2u)
        {
            if (contact->captive == -1)
                animation_start_held(idx, table, 18);
        }
        else if (state == 74 || state == 169)
        {
            /* 2665C/267B0 refresh S6 only when a partner is present. */
            sint32 linked = contact->captive;
            if (linked != -1)
                retained_partner_idx = linked;
            if (state == 74)
                player_grapple74(idx, table, type, dir_flags, linked);
            else
                player_grapple169(idx, table, type, linked);
        }
        /* Other states fall through 2675C ->26D54 in the original, including123/251. */
        if (completion_ready || actor_animations[idx].frame == 0xffffffffu)
        {
            item_pickup_select(idx, 128);
            state = animation_movement_state(actor_animations[idx].id);
            if (state == 82)
            {
                /* 274C4..27534: restart walk while held; sprint may fall back to walk. */
                if (buttons & 240)
                {
                    if (actions[5])
                    {
                        animation_start_held(idx, table, 87);
                        if (animation_movement_state(actor_animations[idx].id) != 87)
                            animation_start_held(idx, table, 82);
                    }
                    else
                        animation_start_held(idx, table, 82);
                }
                else
                    animation_start_held(idx, table, 69);
            }
            else if (state == 87)
            {
                animation_start_held(idx, table, (buttons & 240) ? (actions[5] ? 87 : 82) : 69);
            }
            else if ((uint32)(state - 10) < 8u || (uint32)(state - 19) < 5u || state == 40 || state == 3 || state == 4 || state == 34 || state == 41)
            {
                player_grapple_combo_end(idx, table, state);
            }
            else if (state == 167 || state == 169)
            {
                player_grapple_pair_end(idx, table, retained_partner_idx, state);
            }
            else if (state == 166 || state == 74)
            {
                /* 27A64..27A80: enter/restart the grapple hold, not idle69. */
                animation_start_held(idx, table, 74);
            }
            else if (state == 168)
            {
                /* 26DE0..26DF4: Alana paired action continues at14. */
                animation_start_held(idx, table, 14);
            }
            else if (state == 177 || state == 178 || state == 179)
            {
                /* 26DAC..26DDC and27B30..27B54: type-specific facing corrections. */
                if (state == 177 && type != 1 && (uint32)(type - 2) >= 2u)
                {
                    uint16 facing = (uint16)actor_orientation[idx].yaw ^ 1024u;
                    actor_orientation[idx].yaw = facing;
                    actor_movement[idx].desired_yaw = (uint16)(facing << 5);
                }
                else if (state == 179 && type == 1)
                    actor_orientation[idx].yaw = (uint16)(((uint16)actor_orientation[idx].yaw - 512u) & 2047u);
                animation_start_held(idx, table, 69);
            }
            else if (state == 197)
            {
                --player_repeat_counts[idx];
                if (buttons & 0x4000)
                    player_repeat_counts[idx] = 4;
                if ((sint32)player_repeat_counts[idx] > 0)
                {
                    animation_start_held(idx, table, 197);
                    sint32 slot = actor_items[idx].slot[ACTOR_ITEM_RIGHT];
                    /* TODO: Investigate an invalid player item at the original access */
                    XASSERT(slot >= 0 && slot < DYNAMIC_OBJECT_COUNT);
                    MODEL_OBJECT *item = dynamic_objects + slot;
                    /* TODO: Investigate a player item access with a foreign native owner */
                    XASSERT(item->payload_kind == MODEL_PAYLOAD_PHYSICS);
                    item->physics.actor_idx = 2;
                }
                else
                    animation_start_held(idx, table, 69);
            }
            else if (state == 190)
                animation_start_held(idx, table, combat->repeat_ticks ? 190 : 69);
            else if (state == 159)
            {
                /* Release the right item from attachment3 */
                sint32 sel;
                FIXED_VEC3 point;
                animation_start_held(idx, table, 160);
                sel = actor_items[idx].slot[ACTOR_ITEM_RIGHT];
                actor_items[idx].slot[ACTOR_ITEM_RIGHT] = -1;
                /* TODO: Investigate an invalid release item before expanding the pool */
                XASSERT((uint32)sel < DYNAMIC_OBJECT_COUNT);
                MODEL_OBJECT *item = dynamic_objects + sel;
                /* TODO: Investigate a player item access with a foreign native owner */
                XASSERT(item->payload_kind == MODEL_PAYLOAD_PHYSICS);
                item->physics.speed = 128;
                item->physics.vertical_speed = 64;
                item->physics.spin = 6528;
                item->angles.yaw_bits = (uint16)(((uint16)actor_orientation[idx].yaw) << 5);
                actor_attachment_pos(idx, 3, point.bits);
                item->pos.x.bits = point.x.bits;
                item->pos.y.bits = point.y.bits;
                item->pos.z.bits = point.z.bits;
                /* TODO: Investigate a player item access with a foreign native owner */
                XASSERT(item->payload_kind == MODEL_PAYLOAD_PHYSICS);
                item->physics.actor_idx = idx;
            }
            else if (state == 161)
            {
                /* Release the right item with vertical speed32 */
                sint32 sel;
                FIXED_VEC3 point;
                animation_start_held(idx, table, 162);
                sel = actor_items[idx].slot[ACTOR_ITEM_RIGHT];
                actor_items[idx].slot[ACTOR_ITEM_RIGHT] = -1;
                /* TODO: Investigate an invalid release item before expanding the pool */
                XASSERT((uint32)sel < DYNAMIC_OBJECT_COUNT);
                MODEL_OBJECT *item = dynamic_objects + sel;
                /* TODO: Investigate a player item access with a foreign native owner */
                XASSERT(item->payload_kind == MODEL_PAYLOAD_PHYSICS);
                item->physics.speed = 128;
                item->physics.vertical_speed = 32;
                item->physics.spin = 6528;
                item->angles.yaw_bits = (uint16)(((uint16)actor_orientation[idx].yaw) << 5);
                actor_attachment_pos(idx, 3, point.bits);
                item->pos.x.bits = point.x.bits;
                item->pos.y.bits = point.y.bits;
                item->pos.z.bits = point.z.bits;
                /* TODO: Investigate a player item access with a foreign native owner */
                XASSERT(item->payload_kind == MODEL_PAYLOAD_PHYSICS);
                item->physics.actor_idx = idx;
            }
            else if (state == 163)
            {
                /* 273FC..274C0: release left item from attachment2 */
                sint32 sel;
                FIXED_VEC3 point;
                animation_start_held(idx, table, 164);
                sel = actor_items[idx].slot[ACTOR_ITEM_LEFT];
                actor_items[idx].slot[ACTOR_ITEM_LEFT] = -1;
                /* TODO: Investigate an invalid release item before expanding the pool */
                XASSERT((uint32)sel < DYNAMIC_OBJECT_COUNT);
                MODEL_OBJECT *item = dynamic_objects + sel;
                /* TODO: Investigate a player item access with a foreign native owner */
                XASSERT(item->payload_kind == MODEL_PAYLOAD_PHYSICS);
                item->physics.speed = 128;
                item->physics.vertical_speed = 32;
                item->physics.spin = 6528;
                item->angles.yaw_bits = (uint16)(((uint16)actor_orientation[idx].yaw) << 5);
                actor_attachment_pos(idx, 2, point.bits);
                item->pos.x.bits = point.x.bits;
                item->pos.y.bits = point.y.bits;
                item->pos.z.bits = point.z.bits;
                /* TODO: Investigate a player item access with a foreign native owner */
                XASSERT(item->payload_kind == MODEL_PAYLOAD_PHYSICS);
                item->physics.actor_idx = idx;
            }
            else if (state == 164)
            {
                /* State164 follows the original default completion to idle69 */
                animation_start_held(idx, table, 69);
            }
            else if (state == 118)
            {
                /* 276CC..27798: claim the selected item after the pickup animation. */
                sint32 sel = item_pickup.pending[idx];
                /* TODO: Investigate an invalid claim item before expanding the pool */
                XASSERT((uint32)sel < DYNAMIC_OBJECT_COUNT);
                uint32 kind;
                MODEL_OBJECT *item = dynamic_objects + sel;
                if (item->pos.y.bits == 1)
                    animation_start_held(idx, table, 119);
                else
                {
                    sint32 animation;
                    kind = item->kind;
                    if (kind == 515)
                    {
                        actor_items[idx].slot[type == 2 ? ACTOR_ITEM_RIGHT : ACTOR_ITEM_LEFT] = (sint8)sel;
                        animation = 126;
                    }
                    else
                    {
                        actor_items[idx].slot[ACTOR_ITEM_RIGHT] = (sint8)sel;
                        animation = kind - 499u < 5u ? 125 : 119;
                    }
                    animation_start_held(idx, table, animation);
                    item->pos.y.bits = 1;
                    item_pickup.nearby[idx] = 0xffffffffu;
                }
            }
            else if (state == 123)
            {
                /* 277AC..2781C: reserve the right-hand item, then enter state124. */
                sint32 sel = item_pickup.pending[idx];
                /* TODO: Investigate an invalid claim item before expanding the pool */
                XASSERT((uint32)sel < DYNAMIC_OBJECT_COUNT);
                MODEL_OBJECT *item = dynamic_objects + sel;
                if (item->pos.y.bits == 1)
                    animation_start_held(idx, table, 119);
                else
                {
                    actor_items[idx].slot[ACTOR_ITEM_RIGHT] = (sint8)sel;
                    item->pos.y.bits = 1;
                    item_pickup.nearby[idx] = 0xffffffffu;
                    animation_start_held(idx, table, 124);
                }
            }
            else if (state == 121)
            {
                /* 27964..279FC: reserve right-hand item; convert522/524 once. */
                sint32 sel = item_pickup.pending[idx];
                /* TODO: Investigate an invalid claim item before expanding the pool */
                XASSERT((uint32)sel < DYNAMIC_OBJECT_COUNT);
                uint32 kind;
                MODEL_OBJECT *item = dynamic_objects + sel;
                if (item->pos.y.bits == 1)
                    animation_start_held(idx, table, 119);
                else
                {
                    actor_items[idx].slot[ACTOR_ITEM_RIGHT] = (sint8)sel;
                    kind = item->kind;
                    if (kind == 522)
                    {
                        item->kind = 523;
                        kind = item->kind;
                    }
                    if (kind == 524)
                        item->kind = 525;
                    item->pos.y.bits = 1;
                    item_pickup.nearby[idx] = 0xffffffffu;
                    animation_start_held(idx, table, 122);
                }
            }
            else if (state == 126)
            {
                /* TODO: Review original 278D8 item claim before restoring the missing branch */

                XASSERT(0);
            }
            else if (state == 128)
            {
                /* 27A08..27A60: reserve left-hand item; selection is not cleared here. */
                sint32 sel = item_pickup.pending[idx];
                /* TODO: Investigate an invalid claim item before expanding the pool */
                XASSERT((uint32)sel < DYNAMIC_OBJECT_COUNT);
                MODEL_OBJECT *item = dynamic_objects + sel;
                if (item->pos.y.bits == 1)
                    animation_start_held(idx, table, 119);
                else
                {
                    actor_items[idx].slot[ACTOR_ITEM_LEFT] = (sint8)sel;
                    item->pos.y.bits = 1;
                    animation_start_held(idx, table, 129);
                }
            }
            else if (state == 183 && type == 2)
                animation_start_held(idx, table, actor_health[idx].hp > 0 ? 69 : 154);
            else if (state == 147 || state == 154 || state == 251 || (uint32)(state - 180) < 10 || (uint32)(state - 140) < 3)
            {
                uint16 facing;
                if (state == 181 || (state == 140 && type == 0))
                {
                    facing = actor_orientation[idx].yaw;
                    facing ^= 1024;
                    actor_orientation[idx].yaw = facing;
                    actor_movement[idx].desired_yaw = (uint16)(facing << 5);
                }
                if (state == 184 || state == 141)
                {
                    facing = actor_orientation[idx].yaw;
                    facing = (uint16)((facing - 512) & 2047);
                    actor_orientation[idx].yaw = facing;
                    if (state == 141)
                        actor_orientation[idx].yaw = facing ^ 1024;
                }
                if (state == 188 || state == 189)
                {
                    facing = actor_orientation[idx].yaw;
                    actor_orientation[idx].yaw = (uint16)((facing + 512) & 2047);
                }
                if (state != 147 && state != 154 && state != 251 && state != 180)
                {
                    uint32 landed = 147;
                    if (state == 182 || state == 188 || state == 189 || state == 142 || (state == 183 && (type == 1 || type == 2)) || (state == 140 && type != 2))
                        landed = 154;
                    actor_animations[idx].id = landed;
                }
                /* Recovery tests the original S3 snapshot, not the new actor state */
                actor_health[idx].stun = 0;
                if (actor_health[idx].hp > 0)
                {
                    status->deadline = game_session.tick + 45;
                    animation_start_held(idx, table, state == 154 ? 150 : 149);
                }
                else if (game_session.player_count == 1)
                {
                    if (!level_transition.continue_phase)
                    {
                        level_transition.continue_phase = 1;
                        level_transition.continue_timer = 0;
                    }
                }
                else if (actor_health[0].hp <= 0 && actor_health[1].hp <= 0 && !level_transition.continue_phase)
                {
                    level_transition.continue_phase = 1;
                    level_transition.continue_timer = 0;
                }
            }
            else if (state == 119)
            {
                /* 27820..278C4: finish pickup; kind512 consumes a healing item. */
                sint32 sel;
                animation_start_held(idx, table, 69);
                sel = actor_items[idx].slot[ACTOR_ITEM_RIGHT];
                if (sel != -1)
                {
                    /* TODO: Investigate an invalid player item at the original access */
                    XASSERT(sel >= 0 && sel < DYNAMIC_OBJECT_COUNT);
                    MODEL_OBJECT *item = dynamic_objects + sel;
                    if (item->kind == 512)
                    {
                        sint32 health, maximum;
                        sound_voice_play(122, 255, 6144);
                        /* TODO: Investigate a player item access with a foreign native owner */
                        XASSERT(item->payload_kind == MODEL_PAYLOAD_PHYSICS);
                        item->physics.bounce_count = 0;
                        animation_start_held(idx, table, 132);
                        health = (sint16)(((uint16)actor_health[idx].hp) + 32);
                        actor_health[idx].hp = (uint16)health;
                        maximum = actor_health[idx].max;
                        if (health >= maximum)
                            actor_health[idx].hp = (uint16)(maximum - 4);
                    }
                }
            }
            else
            {
                /* 26D80 ->26DC4: states25/92 finish with a half-turn for every type. */
                if (state == 25 || state == 92 || state == 150)
                {
                    uint16 facing = (uint16)actor_orientation[idx].yaw ^ 1024u;
                    actor_orientation[idx].yaw = facing;
                    actor_movement[idx].desired_yaw = (uint16)(facing << 5);
                }
                /* All remaining states take the original default at27CB0 */
                animation_start_held(idx, table, 69);
            }
        }
        /* Publish cooldowns and previous buttons after action dispatch */
        timer = combat->repeat_ticks;
        if (timer > 0)
            combat->repeat_ticks = timer - 1;
        timer = contact->target_ticks;
        if (timer > 0)
            contact->target_ticks = (uint8)(timer - 1);
        timer = actor_movement[idx].speed;
        if (timer > 0)
            actor_movement[idx].speed = timer - 1;
        pad_game.prev[idx] = buttons;
    next_actor:
        ++idx;
    } while (idx < (sint32)game_session.player_count);
    return 0;
}
