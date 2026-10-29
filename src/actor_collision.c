#include <stdio.h>

#include "game.h"

ACTOR_COLLISION actor_col;

/* Original1C35C reaction slice; preserve animation and displacement order */
static void contact_reaction(sint32 victim, uint32 table, sint32 attacker, sint32 dir)
{
    FIXED_VEC3 *pos = actor_positions + victim;
    uint32 attack = actor_animations[attacker].id, yaw;
    sint32 animation = 133, scale = 0;
    if (dir >= 0)
    {
        animation_start(victim, table, 152);
        return;
    }
    switch (attack)
    {
        case 11:
            animation = 139;
            if (table == ANIMATION_BANK_SHARED1 || table == ANIMATION_BANK_SHARED2 || table == ANIMATION_BANK_SHARED4 || table == ANIMATION_BANK_SHARED8)
                scale = 24;
            else
                scale = table == ANIMATION_BANK_SHARED7 ? 64 : -64;
            break;
        case 14:
            animation = 145;
            break;
        case 15:
            animation = 146;
            break;
        case 20:
            animation = 143;
            break;
        case 21:
            animation = 144;
            break;
        case 40:
            animation = 139;
            scale = -112;
            break;
        default:
            break; /* attack2 and the default both select133. */
    }
    animation_start(victim, table, animation);
    if (scale)
    {
        yaw = (uint32)(sint32)actor_orientation[victim].yaw;
        pos->x.bits = pos->x.bits + (uint32)((sint32)sin_table[yaw] * scale);
        pos->z.bits = pos->z.bits - (uint32)((sint32)sin_table[MATH_QUARTER_TURN + yaw] * scale);
    }
}

/* Original1C35C finisher slice */
static void finisher_contact(sint32 victim, sint32 idx, uint32 x, uint32 y, uint32 z, uint32 angle)
{
    sint32 j;
    for (j = 0; j < 12; j++)
    {
        effect_spawn_actor_hit_reverse(idx, x, y, z, angle);
        effect_spawn_actor_hit(idx, x, y, z, angle);
    }
    disp_buffers.flash_ticks = 4;
    disp_buffers.shake_ticks = 8;
    actor_health[victim].stun = 127;
    sound_voice_play(78, 255, 5632);
}

GDB_CALL sint32 actor_process_hits(sint32 idx, COLLISION_POINT *contact)
{
    FUNCTION_MARKER(0x8001c35c, "GAME.EXE");
    uint32 hit_x = contact->pos.bits[0], hit_y = contact->pos.bits[1], hit_z = contact->pos.bits[2];
    uint32 table, angle;
    sint32 dir, blocked, j, i, type, state, result, x = (sint32)hit_x >> 16, z = (sint32)hit_z >> 16;
    contact->impact_sound = 0;
    result = actor_process_stage_contact(contact, idx);
    if (result)
        return result;
    ACTOR_ANIMATION *attack = actor_animations + idx;
    for (i = 0; i < 8; i++)
    {
        ACTOR_HEALTH *health = actor_health + i;
        ACTOR_ANIMATION *playback = actor_animations + i;
        ACTOR_CONTACT *links = actor_contacts + i;
        FIXED_VEC3 *pos = actor_positions + i;
        if ((!actor_col.player_hits_enabled && i < 2) || i == idx)
            continue;
        type = actor_types[i];
        if (type == -1 || health->hp <= 0 || type == 17 || type == 51 || (sint32)pos->y.bits < 0 || actor_lifecycle[i].delay || ((((sint16)attack->hit_flags) >> i) & 1) || actor_animation_blocks_hits(playback->id))
            continue;
        math_dir16(x, z, pos->x.whole, pos->z.whole);
        if (actor_col.radius[type] < dir_distance)
            continue;
        table = animation_type_banks[(uint32)type];
        angle = (sint16)math_dir16(pos->x.whole, pos->z.whole, actor_positions[idx].x.whole, actor_positions[idx].z.whole);
        dir = (sint16)((((uint16)actor_orientation[i].yaw) << 5) - angle - 0x4000u);
        state = i < 2 ? (sint32)playback->id : npc_base_animation(i);
        if (state - 147u < 2u || state == 154 || state == 191)
            continue;
        attack->hit_flags = (uint16)(((sint16)attack->hit_flags) | (1u << i));
        links->last_attacker = (uint8)idx;
        blocked = 0;
        if (state == 149 || state == 151)
        {
            dynamic_objects_spawn_transient(866, hit_x, hit_y, hit_z);
            blocked = 16;
        }
        if (state - 230u < 2u || state == 232)
        {
            dynamic_objects_spawn_transient(866, hit_x, hit_y, hit_z);
            blocked = 16;
        }
        if (playback->id == 67 && (state == 208 || state - 209u < 2u))
        {
            dynamic_objects_spawn_transient(866, hit_x, hit_y, hit_z);
            blocked = 16;
        }
        if (state == 57 || state - 59u < 2u)
        {
            dynamic_objects_spawn_transient(866, hit_x, hit_y, hit_z);
            blocked = 16;
            if (state != 59 || playback->frame == 0xffffffffu)
                animation_start(i, table, 59);
        }
        if (blocked)
            actor_apply_damage(idx, i, blocked);
        else
        {
            sound_play_pain(i);
            actor_items_drop_on_hit(i);
            uint32 behavior = npc_behavior_actors[i].behavior;
            NPC_BEHAVIOR_REFERENCE profile = npc_behavior_actors[i].profile;
            uint32 row_idx = (profile.first + behavior) & NPC_BEHAVIOR_ROW_MASK;
            XASSERT(profile.rows && row_idx < profile.row_count);
            uint32 delay = profile.rows[row_idx].hit_delay;
            uint32 tick = game_session.tick;
            actor_status[i].deadline = tick + delay;
            dynamic_objects_spawn_transient(865, hit_x, hit_y, hit_z);
            for (j = 0; j < 4; j++)
                effect_spawn_actor_hit_reverse(i, hit_x, hit_y, hit_z, angle);
            for (j = 0; j < 12; j++)
                effect_spawn_actor_hit(i, hit_x, hit_y, hit_z, angle);
            actor_apply_animation_damage(idx, i, attack->id);
        }
        /* 1C78C..1C850: special-attack hit count, bonus label and score. */
        if (i >= 2 && attack->id == 33)
        {
            ACTOR_COMBAT *combat = actor_combat + idx;
            combat->special_hits = (uint8)(combat->special_hits + 1u);
            if (((sint8)combat->special_hits) == 2)
            {
                dynamic_objects_spawn_transient(862, pos->x.bits, 0xff400000u, pos->z.bits);
                XASSERT(idx >= 0 && idx < PLAYER_COUNT);
                player_scores[idx].val += 50u;
                hud_format_score(idx);
            }
            if (((sint8)combat->special_hits) == 3)
            {
                dynamic_objects_spawn_transient(863, pos->x.bits, 0xff400000u, pos->z.bits);
                XASSERT(idx >= 0 && idx < PLAYER_COUNT);
                player_scores[idx].val += 100u;
                hud_format_score(idx);
            }
            if (((sint8)combat->special_hits) == 4)
            {
                dynamic_objects_spawn_transient(864, pos->x.bits, 0xff400000u, pos->z.bits);
                XASSERT(idx >= 0 && idx < PLAYER_COUNT);
                player_scores[idx].val += 250u;
                hud_format_score(idx);
            }
        }
        if (attack->id == 34)
            finisher_contact(i, i, hit_x, hit_y, hit_z, angle);
        if (health->hp <= 0)
        {
            actors_rec_death(i);
            animation_start(i, table, state == 253 || state == 156 ? 156 : state == 252 || state == 155 ? 155 : dir < 0 ? 147 : 154);
            health->hp = 0;
            actors_detach_companion(i);
        }
        else if (!blocked)
        {
            attack->hit_flags = (uint16)(((sint16)attack->hit_flags) | (1u << i));
            if (state == 253 || state == 156)
            {
                health->stun = 0;
                animation_start(i, table, 156);
            }
            else if (state == 252 || state == 155)
            {
                health->stun = 0;
                animation_start(i, table, 155);
            }
            else if (state - 141u < 2u)
                health->stun = 0;
            else if (health->stun < 49 && attack->id - 66u >= 2u)
            {
                if (links->captor == -1 && health->hp < 32 && health->stun >= 2 && (animation_stream(table, 234).kind != ACTOR_ANIMATION_SOURCE_NONE) && (animation_stream(table, 235).kind != ACTOR_ANIMATION_SOURCE_NONE))
                {
                    actor_items_drop(i);
                    animation_start(i, table, dir < 0 ? 234 : 235);
                    actor_col.reaction_deadline = game_session.tick + 60u;
                }
                else
                    contact_reaction(i, table, idx, dir);
            }
            else if (links->captor == -1)
            {
                actor_items_drop(i);
                health->stun = 0;
                animation_start(i, table, dir < 0 ? 147 : 154);
            }
            else
                contact_reaction(i, table, idx, dir);
            actors_detach_companion(i);
        }
        if ((sint32)game_session.mode < 2)
            actors_rec_hit(idx, i);
        return 1;
    }
    return 0;
}

/* Collision event latch retains the original actor callback carrier */
GDB_CALL sint32 actor_col_event(COLLISION_REGION *obstacle, sint32 idx)
{
    FUNCTION_MARKER(0x8001a3f8, "GAME.EXE");
    ACTOR_ANIMATION *playback = actor_animations + idx;
    uint32 flags = playback->hit_flags, animation = playback->id;
    COLLISION_POINT point;
    COLLISION_REFERENCE target;
    sint32 handler_idx;
    if (flags & 0x4000)
        return 0x4000;
    if (animation - 180u >= 10 && animation - 147u >= 2 && animation != 154)
        return 154;
    if (!obstacle->object.present)
        return 0x80010000u;
    /* TODO: Investigate a collision event outside the world object pool */
    XASSERT(obstacle->object.present == 1 && obstacle->object.idx < WORLD_OBJECT_COUNT);
    MODEL_OBJECT *model = world_objects.object + obstacle->object.idx;
    point.pos.bits[0] = model->pos.x.bits;
    point.pos.bits[1] = 0x80010000u;
    point.pos.bits[2] = model->pos.z.bits;
    /* Fix the original uninitialized impact sound selector */
    point.impact_sound = 0;
    playback->hit_flags |= 0x4000;
    handler_idx = obstacle->col_kind;
    XASSERT(handler_idx >= 0 && handler_idx < COLLISION_SLOT_COUNT);
    target = col_callbacks[handler_idx];
    return col_call(target, obstacle, model, &point, idx);
}

static sint32 side_product(sint32 a, sint32 b)
{
    return (uint32)a * (uint32)b;
}

static sint32 circle_probe_distance(COLLISION_REGION *obstacle, uint32 x, uint32 z, uint32 *table_offset)
{
    sint32 cx, cz, angle, sn, cs;
    cx = obstacle->geometry.circle.center_x;
    cz = obstacle->geometry.circle.center_z;
    angle = math_dir11((sint32)x >> 16, (sint32)z >> 16, cx, cz);
    cx = obstacle->geometry.circle.center_x;
    *table_offset = (uint32)(sint32)(sint16)angle * 2;
    sn = sin_table[(*table_offset) >> 1];
    cs = sin_table[MATH_QUARTER_TURN + ((*table_offset) >> 1)];
    cz = obstacle->geometry.circle.center_z;
    return math_distance_approx((sint32)(x + ((uint32)sn << 8)) >> 16, (sint32)(z - ((uint32)cs << 8)) >> 16, cx, cz);
}

/* Correct obstacles in order, retaining the clear latch across records */
static sint32 correct_obstacles(sint32 idx, uint32 *native_x, uint32 *native_z)
{
    FUNCTION_MARKER(0x8001a4d0, "GAME.EXE");
    uint32 x;
    uint32 z;
    sint32 dx;
    sint32 dz;
    COLLISION_REGION *o;
    sint32 xi, zi, oldx, oldz, i, clear = 1, changed = 0;
    if (actor_lifecycle[idx].delay)
        return 1;
    x = *native_x;
    z = *native_z;
    xi = (sint32)x >> 16;
    zi = (sint32)z >> 16;
    oldx = actor_positions[idx].x.whole;
    oldz = actor_positions[idx].z.whole;
    dx = x - oldx;
    /* TODO: Review original whole-X reuse in both Q16 differences */
    dz = z - oldx;
    for (i = 0, o = col_regions.data; i < (sint32)col_regions.count; i++, ++o)
    {
        sint32 type;
        uint32 sn;
        uint32 cs;
        XASSERT(o != col_regions.data + COLLISION_REGION_COUNT);
        if (o->col_kind < 2)
            continue;
        type = o->shape;
        if (type == 1)
        {
            /* TODO: Investigate an oriented obstacle outside the world object pool */
            XASSERT(o->object.present == 1 && o->object.idx < WORLD_OBJECT_COUNT);
            const MODEL_OBJECT *model = world_objects.object + o->object.idx;
            uint32 offset = (model->angles.yaw_bits >> 5) * 2;
            uint32 cx;
            uint32 cz;
            sint32 lx;
            sint32 lz;
            sint32 angle;
            sint32 k;
            uint32 rx, rz;
            cx = model->pos.x.whole;
            cs = sin_table[MATH_QUARTER_TURN + (offset >> 1)];
            rx = ((sint32)x >> 16) - cx;
            cz = model->pos.z.whole;
            sn = sin_table[offset >> 1];
            rz = ((sint32)z >> 16) - cz;
            lx = (sint32)(rx * cs + rz * sn) >> 15;
            lz = (sint32)(rz * cs - rx * sn) >> 15;
            angle = (sint16)math_dir11(lx, lz, 0, 0);
            for (k = 0; k < 2048; k += 256)
            {
                sint32 sx, sz, hx, hz, px, pz, corner, encoded_angle;
                sx = sin_table[k] >> 9;
                sz = sin_table[MATH_QUARTER_TURN + k] >> 9;
                hx = o->geometry.box.half_x;
                px = lx + (uint32)sx;
                pz = lz + (uint32)sz;
                if (px <= -hx || px >= hx)
                    continue;
                hz = o->geometry.box.half_z;
                if (pz <= -hz || pz >= hz)
                    continue;
                clear = 0;
                changed = 1;
                corner = (sint16)math_dir11(0, 0, hx, -hz);
                if (angle < corner || 2048 - corner < angle)
                {
                    hz = o->geometry.box.half_z;
                    encoded_angle = model->angles.yaw;
                    lz = hz - sz;
                }
                else if (angle < 1024 - corner)
                {
                    encoded_angle = model->angles.yaw;
                    hx = o->geometry.box.half_x;
                    lx = -hx - sx;
                }
                else if (angle < 1024 + corner)
                {
                    hz = o->geometry.box.half_z;
                    encoded_angle = model->angles.yaw;
                    lz = -hz - sz;
                }
                else
                {
                    encoded_angle = model->angles.yaw;
                    hx = o->geometry.box.half_x;
                    lx = hx - sx;
                }
                offset = ((-encoded_angle) >> 4) & 0xffeu;
                cs = sin_table[MATH_QUARTER_TURN + (offset >> 1)];
                sn = sin_table[offset >> 1];
                x = (cx + ((sint32)(lx * cs + lz * sn) >> 15)) << 16;
                z = (cz + ((sint32)(lz * cs - lx * sn) >> 15)) << 16;
                break;
            }
        }
        else if (type)
        {
            uint32 offset, scale, penetration;
            sint32 distance = circle_probe_distance(o, x, z, &offset);
            sint32 radius = o->geometry.circle.radius;
            if (distance < radius)
            {
                penetration = dir_distance;
                sn = sin_table[offset >> 1];
                scale = ((radius - penetration) << 1) + 256;
                cs = sin_table[MATH_QUARTER_TURN + (offset >> 1)];
                clear = 0;
                changed = 1;
                x -= sn * scale;
                z += cs * scale;
            }
        }
        else
        {
            sint32 left, right, bottom, top, side = 0;
            bottom = o->geometry.axis.min_z - 32;
            right = o->geometry.axis.max_x + 32;
            top = o->geometry.axis.max_z + 32;
            left = o->geometry.axis.min_x - 32;
            if (xi < left || xi > right || zi < bottom || zi > top)
                continue;
            clear = 0;
            changed = 1;
            if (oldx < left)
                side = 1;
            else if (oldx > right)
                side = 2;
            if (oldz < bottom)
                side |= 4;
            else if (oldz > top)
                side |= 8;
            if (side == 5)
                side = side_product(bottom - oldz, dx) < side_product(dz, left - oldx) ? 4 : 1;
            else if (side == 6)
                side = side_product(bottom - oldz, dx) < side_product(dz, right - oldx) ? 2 : 4;
            else if (side == 9)
                side = side_product(top - oldz, dx) < side_product(dz, left - oldx) ? 1 : 8;
            else if (side == 10)
                side = side_product(top - oldz, dx) < side_product(dz, right - oldx) ? 8 : 2;
            if (side == 1)
                xi = left - 1;
            else if (side == 2)
                xi = right + 1;
            else if (side == 4)
                zi = bottom - 1;
            else if (side == 8)
                zi = top + 1;
            x = (uint32)xi << 16;
            z = (uint32)zi << 16;
        }
        if (!clear)
            actor_col_event(o, idx);
    }
    if (changed)
        for (i = 0, o = col_regions.data; i < (sint32)col_regions.count; i++, ++o)
        {
            sint32 type, reject = 0;
            XASSERT(o != col_regions.data + COLLISION_REGION_COUNT);
            if (o->col_kind < 2)
                continue;
            type = o->shape;
            if (type == 1)
                continue;
            if (!type)
            {
                xi = (sint32)x >> 16;
                zi = (sint32)z >> 16;
                {
                    sint32 bottom, right, top, left;
                    bottom = o->geometry.axis.min_z - 32;
                    right = o->geometry.axis.max_x + 32;
                    top = o->geometry.axis.max_z + 32;
                    left = o->geometry.axis.min_x - 32;
                    reject = xi >= left && xi <= right && zi >= bottom && zi <= top;
                }
            }
            else if (type == 2)
            {
                uint32 offset;
                sint32 distance = circle_probe_distance(o, x, z, &offset);
                sint32 radius = o->geometry.circle.radius;
                reject = distance <= radius;
            }
            if (reject)
            {
                x = actor_positions[idx].x.bits;
                *native_x = x;
                z = actor_positions[idx].z.bits;
                *native_z = z;
                return 0;
            }
        }
    *native_x = x;
    *native_z = z;
    return clear;
}

GDB_CALL void actor_correct_obstacles(sint32 idx, uint32 *x, uint32 *z)
{
    correct_obstacles(idx, x, z);
}

static int excluded_animation(uint32 n)
{
    return n - 177u < 13 || n - 10u < 4 || n - 14u < 5 || n - 19u < 5 || n - 252u < 2 || n - 155u < 2 || n - 140u < 2 || n == 142 || n == 148 || n - 145u < 2 || n == 139 || n == 171 || n == 74 || n == 166 || n == 77 || n == 169 || n == 174 || n == 168 || n == 172 || n == 167 || n == 173 || n == 170 || n == 65 || n - 3u < 2 || n == 205 || n == 203 || n == 281 || n == 269;
}

/* 8001B2C4: ordered pair separation. The global call counter is not decremented
 * on return. Candidate actor stores occur only in the second-distance branch */
GDB_CALL sint32 actors_resolve_overlaps(sint32 idx)
{
    FUNCTION_MARKER(0x8001b2c4, "GAME.EXE");
    uint32 count, animation;
    sint32 gate, frame, react, i;
    ACTOR_ANIMATION *playback = actor_animations + idx;
    FIXED_VEC3 *pos = actor_positions + idx;
    gate = actor_lifecycle[idx].delay;
    if (gate)
        return gate;
    count = actor_col.pass_count + 1;
    actor_col.pass_count = count;
    if ((sint32)count >= 6)
        return -1;
    frame = playback->frame;
    animation = playback->id;
    react = frame != -1 && ((animation == 180 && frame < 20) || (animation == 147 && frame < 12) || animation == 148);
    for (i = 0; i < ACTOR_COUNT; i++)
    {
        ACTOR_ANIMATION *other_playback = actor_animations + i;
        FIXED_VEC3 *other_pos = actor_positions + i;
        sint32 type, radius, r, distance;
        uint32 n;
        sint32 dx;
        sint32 dz;
        uint32 x;
        uint32 z;
        uint32 table;
        uint32 ix;
        uint32 iz;
        if (actor_companions[idx] == i || i == idx)
            continue;
        type = actor_types[i];
        if (type == -1 || type == 50 || (sint8)actor_lifecycle[i].death_ticks > 0)
            continue;
        n = other_playback->id;
        if (excluded_animation(n))
            continue;
        radius = actor_col.radius[type];
        r = actor_col.radius[actor_types[idx]];
        if (radius < r)
            radius = r;
        dx = other_pos->x.bits - pos->x.bits;
        dz = other_pos->z.bits - pos->z.bits;
        ix = dx >> 16;
        iz = dz >> 16;
        distance = math_sqrt_odd(ix * ix + iz * iz);
        if (!distance)
            distance = 1;
        if (distance >= radius)
            continue;
        /* The real bit-pair helper returns 1..65535, so signed division cannot trap. */
        if (actor_lifecycle[i].delay)
        {
            x = other_pos->x.bits;
            z = other_pos->z.bits;
        }
        else
        {
            x = pos->x.bits + radius * (uint32)(dx / distance);
            z = pos->z.bits + radius * (uint32)(dz / distance);
            actor_correct_obstacles(i, &x, &z);
        }
        type = actor_types[i];
        if (type != 17 && type != 51)
        {
            table = animation_type_banks[(uint32)type];
            if (n - 50u >= 2 && n - 52u >= 2 && (animation_stream(table, 147).kind != ACTOR_ANIMATION_SOURCE_NONE) && i >= 2 && actor_companions[i] == -1 && react && other_playback->id - 133u >= 4)
                animation_start(i, table, (sint32)(random_next() & 3) + 133);
            dx = pos->x.bits - x;
            dz = pos->z.bits - z;
            ix = dx >> 16;
            iz = dz >> 16;
            distance = math_sqrt_odd(ix * ix + iz * iz);
            if (!distance)
                distance = 1;
            if (distance < radius)
            {
                other_pos->x.bits = x;
                other_pos->z.bits = z;
                x += radius * (uint32)(dx / distance);
                z += radius * (uint32)(dz / distance);
                actor_correct_obstacles(idx, &x, &z);
                pos->x.bits = x;
                pos->z.bits = z;
            }
        }
        actors_resolve_overlaps(i);
    }
    return 0;
}

/* First strict hit, then push out; negative-Z exit retains its zero result */
static uint32 collide_vehicle(COLLISION_REGION *region, const MODEL_OBJECT *model, const VEHICLE_COLLISION_SHAPE *vehicle)
{
    uint32 i, angle, yaw, pos, offset, dx, dz, sn, cs, pos_x, pos_z;
    sint32 type, x, z, width, depth, bearing, actor_x, actor_z, object_x, object_z, wave;
    for (i = 0; i < ACTOR_COUNT; i++)
    {
        type = actor_types[i];
        if (type == -1 || type == 17 || actor_health[i].hp <= 0 || actor_animations[i].id == 147 || actor_lifecycle[i].delay)
            continue;
        actor_x = actor_positions[i].x.whole;
        actor_z = actor_positions[i].z.whole;
        /* TODO: Investigate vehicle or movement callbacks without a native position */
        XASSERT(vehicle || model);
        object_x = vehicle ? (sint32)vehicle->x >> 16 : model->pos.x.whole;
        object_z = vehicle ? (sint32)vehicle->z >> 16 : model->pos.z.whole;
        bearing = math_dir11(actor_x, actor_z, object_x, object_z);
        /* Q15 products wrap before the signed shift */
        offset = (sint32)((uint32)bearing << 16) >> 15;
        sn = (uint32)(sint32)sin_table[offset >> 1];
        pos_x = actor_positions[i].x.bits;
        cs = (uint32)(sint32)sin_table[MATH_QUARTER_TURN + (offset >> 1)];
        pos_z = actor_positions[i].z.bits;
        object_x = vehicle ? (sint32)vehicle->x >> 16 : model->pos.x.whole;
        object_z = vehicle ? (sint32)vehicle->z >> 16 : model->pos.z.whole;
        dx = (uint32)((sint32)(pos_x + (sn << 4)) >> 16) - (uint32)object_x;
        dz = (uint32)((sint32)(pos_z - (cs << 4)) >> 16) - (uint32)object_z;
        yaw = vehicle ? vehicle->yaw : model->angles.yaw_bits;
        offset = 2 * (yaw >> 5);
        sn = (uint32)(sint32)sin_table[offset >> 1];
        cs = (uint32)(sint32)sin_table[MATH_QUARTER_TURN + (offset >> 1)];
        x = (sint32)(dx * cs + dz * sn) >> 15;
        z = (sint32)(dz * cs - dx * sn) >> 15;
        /* TODO: Investigate the original one-past region read into terrain */
        XASSERT(region != col_regions.data + COLLISION_REGION_COUNT);
        width = region->geometry.box.half_x;
        if (x <= -width || x >= width)
            continue;
        depth = region->geometry.box.half_z;
        if (z <= -depth || z >= depth)
            continue;
        if ((sint32)((((uint32)bearing << 5) - yaw) << 16) > 0)
            angle = (uint32)(sint32)(sint16)yaw + 16384;
        else
            angle = (uint32)(sint32)(vehicle ? (sint16)vehicle->yaw : model->angles.yaw) - 16384;
        angle >>= 5;
        actor_movement[i].desired_yaw = (uint16)angle;
        actor_orientation[i].yaw = (uint16)angle;
        if (game_session.stage == 18)
            actor_orientation[i].yaw = 0;
        actor_orientation[i].yaw = ((uint16)actor_orientation[i].yaw) & 2047;
        actor_contacts[i].captor = ACTOR_SLOT_NONE;
        actor_health[i].stun = 0;
        actor_items_drop(i);
        actor_apply_damage(-1, i, -100);
        actor_contacts[i].last_attacker = ACTOR_SLOT_NONE;
        animation_start(i, animation_type_banks[(uint32)actor_types[i]], 147);
        sound_play_pain(i);
        actor_pose_decode(model_refs[(uint32)actor_types[i]], i, &actor_poses[i]);
        actor_col.pass_count = 0;
        actors_resolve_overlaps(i);
        for (;;)
        {
            angle = 2 * (uint32)(sint32)actor_orientation[i].yaw;
            /* TODO: Investigate invalid yaw after the original hit callbacks */
            XASSERT(angle < 2u * MATH_ANGLE_COUNT);
            wave = sin_table[angle >> 1];
            pos = actor_positions[i].x.bits;
            actor_positions[i].x.bits = pos - ((uint32)wave << 7);
            wave = sin_table[MATH_QUARTER_TURN + (angle >> 1)];
            pos = actor_positions[i].z.bits + ((uint32)wave << 7);
            actor_positions[i].z.bits = pos;
            actor_x = actor_positions[i].x.whole;
            object_x = vehicle ? (sint32)vehicle->x >> 16 : model->pos.x.whole;
            object_z = vehicle ? (sint32)vehicle->z >> 16 : model->pos.z.whole;
            bearing = math_dir11(actor_x, (sint32)pos >> 16, object_x, object_z);
            /* Q15 products wrap before the signed shift */
            offset = (sint32)((uint32)bearing << 16) >> 15;
            sn = (uint32)(sint32)sin_table[offset >> 1];
            pos_x = actor_positions[i].x.bits;
            cs = (uint32)(sint32)sin_table[MATH_QUARTER_TURN + (offset >> 1)];
            pos_z = actor_positions[i].z.bits;
            object_x = vehicle ? (sint32)vehicle->x >> 16 : model->pos.x.whole;
            object_z = vehicle ? (sint32)vehicle->z >> 16 : model->pos.z.whole;
            dx = (uint32)((sint32)(pos_x + (sn << 4)) >> 16) - (uint32)object_x;
            dz = (uint32)((sint32)(pos_z - (cs << 4)) >> 16) - (uint32)object_z;
            yaw = vehicle ? vehicle->yaw : model->angles.yaw_bits;
            offset = 2 * (yaw >> 5);
            sn = (uint32)(sint32)sin_table[offset >> 1];
            cs = (uint32)(sint32)sin_table[MATH_QUARTER_TURN + (offset >> 1)];
            x = (sint32)(dx * cs + dz * sn) >> 15;
            z = (sint32)(dz * cs - dx * sn) >> 15;
            width = region->geometry.box.half_x;
            if (x < -width || x > width)
                return 1;
            depth = region->geometry.box.half_z;
            if (z < -depth)
                return depth < z;
            if (z > depth)
                return 1;
        }
    }
    return 0;
}

GDB_CALL uint32 actors_collide_vehicle(COLLISION_REGION *region, const struct MODEL_OBJECT *model)
{
    FUNCTION_MARKER(0x80029afc, "GAME.EXE");
    return collide_vehicle(region, model, NULL);
}

GDB_CALL uint32 actors_collide_vehicle_shape(COLLISION_REGION *region, const VEHICLE_COLLISION_SHAPE *vehicle)
{
    return collide_vehicle(region, NULL, vehicle);
}

static sint32 projectile_impact(uint32 world_x, uint32 world_z, const MODEL_OBJECT *model, const PROJECTILE_SOURCE *projectile, const EFFECT *effect)
{
    FUNCTION_MARKER(0x800322c8u, "GAME.EXE");
    /* TODO: Investigate missing, ambiguous or unpublished projectile sources */
    XASSERT((!!model + !!projectile + !!effect) == 1 && (!effect || effect->payload_kind == EFFECT_PAYLOAD_MOVING_RIBBON && (effect->kind == 876 || effect->kind == 0xffffffffu)) && (!model || model->payload_kind == MODEL_PAYLOAD_PHYSICS || model->payload_kind == MODEL_PAYLOAD_AIR_BIKE || model->payload_kind == MODEL_PAYLOAD_SCENE_VEHICLE || model->payload_kind == MODEL_PAYLOAD_SCENE_HELICOPTER || model->payload_kind == MODEL_PAYLOAD_COMBAT_HELICOPTER || model->payload_kind == MODEL_PAYLOAD_PATROL_HELICOPTER));
    const EFFECT_MOVING_RIBBON *ribbon = effect ? &effect->moving_ribbon : NULL;
    const FIXED_VEC3 *pos = ribbon ? &ribbon->ribbon.pos : model ? &model->pos : NULL;
    uint32 y = projectile ? projectile->height : pos->y.bits, kind, animation, state;
    ANIMATION_BANK_ID resource;
    int i, j;
    sint32 type, owner, actor_x, actor_z, x = (sint32)world_x >> 16, z = (sint32)world_z >> 16;
    for (i = 0; i < ACTOR_COUNT; i++)
    {
        if (actor_lifecycle[i].delay)
            continue;
        type = actor_types[i];
        if (type == -1 || type == 17 || type == 51)
            continue;
        state = actor_animations[i].id;
        if (state == 205 || state == 203 || actor_health[i].hp <= 0)
            continue;
        actor_x = actor_positions[i].x.whole;
        actor_z = actor_positions[i].z.whole;
        if (math_distance_approx(x, z, actor_x, actor_z) >= 64)
            continue;
        actor_items_drop_on_hit(i);
        /* TODO: Investigate cross-variant projectile payload changes */
        XASSERT((!effect || effect->payload_kind == EFFECT_PAYLOAD_MOVING_RIBBON && (effect->kind == 876 || effect->kind == 0xffffffffu)) && (!model || model->payload_kind == MODEL_PAYLOAD_PHYSICS || model->payload_kind == MODEL_PAYLOAD_AIR_BIKE || model->payload_kind == MODEL_PAYLOAD_SCENE_VEHICLE || model->payload_kind == MODEL_PAYLOAD_SCENE_HELICOPTER || model->payload_kind == MODEL_PAYLOAD_COMBAT_HELICOPTER || model->payload_kind == MODEL_PAYLOAD_PATROL_HELICOPTER));
        kind = projectile ? projectile->kind : (effect ? effect->kind : model->kind);
        if (kind - 499 < 5)
            actor_apply_damage(projectile ? projectile->owner : ribbon ? ribbon->owner : model && model->payload_kind == MODEL_PAYLOAD_PHYSICS ? model->physics.actor_idx : (model && model->payload_kind == MODEL_PAYLOAD_AIR_BIKE ? model->air_bike.target[1].z : (model && model->payload_kind == MODEL_PAYLOAD_SCENE_VEHICLE ? model->scene_vehicle.voice_plus_one[0] : (model && model->payload_kind == MODEL_PAYLOAD_SCENE_HELICOPTER ? model->scene_helicopter.active : (model && model->payload_kind == MODEL_PAYLOAD_COMBAT_HELICOPTER ? model->helicopter.voice_plus_one[0] : model->patrol.rotor_angle)))), i, -60);
        else
        {
            kind = projectile ? projectile->kind : (effect ? effect->kind : model->kind);
            if (kind - 504 < 2 || kind == 515)
                actor_apply_damage(projectile ? projectile->owner : ribbon ? ribbon->owner : model && model->payload_kind == MODEL_PAYLOAD_PHYSICS ? model->physics.actor_idx : (model && model->payload_kind == MODEL_PAYLOAD_AIR_BIKE ? model->air_bike.target[1].z : (model && model->payload_kind == MODEL_PAYLOAD_SCENE_VEHICLE ? model->scene_vehicle.voice_plus_one[0] : (model && model->payload_kind == MODEL_PAYLOAD_SCENE_HELICOPTER ? model->scene_helicopter.active : (model && model->payload_kind == MODEL_PAYLOAD_COMBAT_HELICOPTER ? model->helicopter.voice_plus_one[0] : model->patrol.rotor_angle)))), i, -100);
            else
            {
                owner = projectile ? projectile->owner : ribbon ? ribbon->owner : model && model->payload_kind == MODEL_PAYLOAD_PHYSICS ? model->physics.actor_idx : (model && model->payload_kind == MODEL_PAYLOAD_AIR_BIKE ? model->air_bike.target[1].z : (model && model->payload_kind == MODEL_PAYLOAD_SCENE_VEHICLE ? model->scene_vehicle.voice_plus_one[0] : (model && model->payload_kind == MODEL_PAYLOAD_SCENE_HELICOPTER ? model->scene_helicopter.active : (model && model->payload_kind == MODEL_PAYLOAD_COMBAT_HELICOPTER ? model->helicopter.voice_plus_one[0] : model->patrol.rotor_angle))));
                kind = projectile ? projectile->kind : (effect ? effect->kind : model->kind);
                object_apply_damage(owner, i, kind);
            }
        }
        /* TODO: Investigate cross-variant projectile payload changes */
        XASSERT((!effect || effect->payload_kind == EFFECT_PAYLOAD_MOVING_RIBBON && (effect->kind == 876 || effect->kind == 0xffffffffu)) && (!model || model->payload_kind == MODEL_PAYLOAD_PHYSICS || model->payload_kind == MODEL_PAYLOAD_AIR_BIKE || model->payload_kind == MODEL_PAYLOAD_SCENE_VEHICLE || model->payload_kind == MODEL_PAYLOAD_SCENE_HELICOPTER || model->payload_kind == MODEL_PAYLOAD_COMBAT_HELICOPTER || model->payload_kind == MODEL_PAYLOAD_PATROL_HELICOPTER));
        owner = projectile ? projectile->owner : ribbon ? ribbon->owner : model && model->payload_kind == MODEL_PAYLOAD_PHYSICS ? model->physics.actor_idx : (model && model->payload_kind == MODEL_PAYLOAD_AIR_BIKE ? model->air_bike.target[1].z : (model && model->payload_kind == MODEL_PAYLOAD_SCENE_VEHICLE ? model->scene_vehicle.voice_plus_one[0] : (model && model->payload_kind == MODEL_PAYLOAD_SCENE_HELICOPTER ? model->scene_helicopter.active : (model && model->payload_kind == MODEL_PAYLOAD_COMBAT_HELICOPTER ? model->helicopter.voice_plus_one[0] : model->patrol.rotor_angle))));
        /* TODO: Investigate original hit credit outside the actor pool */
        XASSERT(owner >= 0 && owner < ACTOR_COUNT);
        actors_rec_hit(owner, i);
        /* TODO: Investigate cross-variant projectile payload changes */
        XASSERT((!effect || effect->payload_kind == EFFECT_PAYLOAD_MOVING_RIBBON && (effect->kind == 876 || effect->kind == 0xffffffffu)) && (!model || model->payload_kind == MODEL_PAYLOAD_PHYSICS || model->payload_kind == MODEL_PAYLOAD_AIR_BIKE || model->payload_kind == MODEL_PAYLOAD_SCENE_VEHICLE || model->payload_kind == MODEL_PAYLOAD_SCENE_HELICOPTER || model->payload_kind == MODEL_PAYLOAD_COMBAT_HELICOPTER || model->payload_kind == MODEL_PAYLOAD_PATROL_HELICOPTER));

        kind = projectile ? projectile->kind : (effect ? effect->kind : model->kind);
        if (kind - 499 < 5 || kind - 504 < 2 || kind == 515)
        {
            if (actor_animations[i].id != 251)
            {
                actor_orientation[i].yaw = (uint16)(((projectile ? projectile->angle : ribbon ? (uint16)ribbon->ribbon.delta.x : model->angles.yaw_bits) >> 5) ^ 1024);
                dynamic_objects_spawn_transient(865, world_x, y, world_z);
                for (j = 0; j < 4; j++)
                    effect_spawn_actor_hit_reverse(i, world_x, y, world_z, projectile ? (sint16)projectile->angle : ribbon ? ribbon->ribbon.delta.x : model->angles.yaw);
                for (j = 0; j < 12; j++)
                    effect_spawn_actor_hit(i, world_x, y, world_z, projectile ? (sint16)projectile->angle : ribbon ? ribbon->ribbon.delta.x : model->angles.yaw);
                sound_play_pain(i);
                actor_health[i].stun = (uint16)(((uint16)actor_health[i].stun) + 32);
                if ((projectile ? projectile->kind : (effect ? effect->kind : model->kind)) == 504)
                    actor_health[i].stun = 127;
                if (actor_health[i].stun < 49 && actor_health[i].hp > 0)
                    animation = (random_next() & 1) + 249;
                else
                {
                    actor_items_drop(i);
                    animation = 251;
                }
                type = actor_types[i];
                resource = animation_type_banks[(uint32)type];
                animation_start(i, resource, animation);
            }
            return actors_detach_companion(i);
        }
        actor_x = actor_positions[i].x.whole;
        actor_z = actor_positions[i].z.whole;
        actor_orientation[i].yaw = (uint16)(math_dir11(x, z, actor_x, actor_z) ^ 1024);
        actor_items_drop(i);
        type = actor_types[i];
        resource = animation_type_banks[(uint32)type];
        animation_start(i, resource, 147);
        actors_detach_companion(i);
    }
    return 0;
}

GDB_CALL sint32 npc_process_weapon_hits(sint32 idx)
{
    FUNCTION_MARKER(0x8004930cu, "GAME.EXE");
    /* TODO: Investigate the original out-of-pool NPC operation */
    XASSERT(idx >= 0 && idx < ACTOR_COUNT);
    sint32 slot = actor_items[idx].slot[ACTOR_ITEM_RIGHT];
    sint32 src_type = actor_types[idx];
    uint32 type;
    sint32 angle;
    sint32 dir;
    sint32 distance;
    sint32 victim_idx;
    /* TODO: Investigate the original out-of-pool NPC operation */
    XASSERT(slot >= 0 && slot < DYNAMIC_OBJECT_COUNT);
    MODEL_OBJECT *item = dynamic_objects + slot;
    uint32 kind = item->kind, debris;
    FIXED_VEC3 point;
    int i, detach;
    actor_items_calc_weapon_point(idx, 0, point.bits);
    detach = src_type == 32 || src_type - 49u < 2u || src_type == 48 || src_type == 14;
    for (victim_idx = 0; victim_idx < (sint32)game_session.player_count; ++victim_idx)
    {
        /* TODO: Investigate the original out-of-pool NPC operation */
        XASSERT(victim_idx < ACTOR_COUNT);
        if (((((sint16)actor_animations[idx].hit_flags) >> (victim_idx & 31)) & 1) || actor_animation_blocks_hits(actor_animations[victim_idx].id))
            continue;
        type = actor_types[victim_idx];
        distance = math_distance_approx((sint32)point.x.bits >> 16, (sint32)point.z.bits >> 16, actor_positions[victim_idx].x.whole, actor_positions[victim_idx].z.whole);
        angle = actor_col.radius[actor_types[victim_idx]];
        dir_distance = distance;
        if (distance >= angle)
            continue;
        angle = (sint16)math_dir16(actor_positions[victim_idx].x.whole, actor_positions[victim_idx].z.whole, actor_positions[idx].x.whole, actor_positions[idx].z.whole);
        dir = (sint16)(((uint32)(uint16)actor_orientation[victim_idx].yaw << 5) - angle - 16384u);
        angle = actor_orientation[victim_idx].yaw;
        dynamic_objects_spawn_transient(865, point.x.bits, point.y.bits, point.z.bits);
        actor_items_drop_on_hit(victim_idx);
        sound_play_pain(victim_idx);
        angle ^= 1024;
        if (kind == 478 || kind == 480 || kind == 482)
        {
            item->kind = kind + 1;
            sound_voice_play(113, 255, 6400);
            debris = 599 + (kind - 478) / 2;
            for (i = 0; i < 12; ++i)
                effect_spawn_glass_fragment(debris, point.x.bits, point.y.bits, point.z.bits, angle - 512u);
        }
        actor_contacts[victim_idx].last_attacker = (uint8)idx;
        for (i = 0; i < 8; ++i)
            effect_spawn_hit_reverse(point.x.bits, point.y.bits, point.z.bits, angle);
        for (i = 0; i < 32; ++i)
            effect_spawn_hit(point.x.bits, point.y.bits, point.z.bits, angle);
        object_apply_damage(actor_contacts[victim_idx].last_attacker, victim_idx, item->kind);
        if (actor_health[victim_idx].hp <= 0)
        {
            if (detach)
                sound_play_heavy_hit_voice(idx);
            actor_items_drop(victim_idx);
            actors_rec_death(victim_idx);
            animation_start(victim_idx, animation_type_banks[type], 147);
            actor_health[victim_idx].hp = 0;
        }
        else
        {
            actor_animations[idx].hit_flags = (uint16)((uint16)((sint16)actor_animations[idx].hit_flags) | (1u << (victim_idx & 31)));
            if (actor_health[victim_idx].stun < 49)
                animation_start(victim_idx, animation_type_banks[type], dir < 0 ? 133 : 152);
            else
            {
                if (detach)
                    sound_play_heavy_hit_voice(idx);
                animation_start(victim_idx, animation_type_banks[type], dir < 0 ? 147 : 154);
                actor_items_drop(victim_idx);
                actor_health[victim_idx].stun = 0;
            }
        }
        return 1;
    }
    return 0;
}

GDB_CALL sint32 actors_apply_nearby_contact(sint32 attacker, uint32 world_x, uint32 world_z)
{
    FUNCTION_MARKER(0x80055964u, "GAME.EXE");
    sint32 x = (sint32)world_x >> 16, z = (sint32)world_z >> 16, idx, type, slot;
    uint32 state, angle;
    for (idx = 0; idx < 8; idx++)
    {
        if (actor_lifecycle[idx].delay || actor_contacts[idx].captor != -1)
            continue;
        type = actor_types[idx];
        if (type == -1 || type == 17 || type == 51)
            continue;
        state = actor_animations[idx].id;
        if (state == 147 || state == 167 || state == 148 || actor_health[idx].hp <= 0)
            continue;
        if (math_distance_approx(x, z, actor_positions[idx].x.whole, actor_positions[idx].z.whole) >= 64)
            continue;
        actor_apply_damage(-1, idx, -100);
        actors_rec_hit(attacker, idx);
        /* TODO: Investigate an invalid attacker before reading its position */
        XASSERT((uint32)attacker < ACTOR_COUNT);
        angle = math_dir11(actor_positions[attacker].x.whole, actor_positions[attacker].z.whole, actor_positions[idx].x.whole, actor_positions[idx].z.whole) ^ 512u;
        actor_orientation[idx].yaw = (uint16)angle;
        actor_items_drop(idx);
        slot = actor_contacts[idx].captive;
        if (slot != -1)
        {
            actor_contacts[idx].captive = ACTOR_SLOT_NONE;
            /* TODO: Investigate an invalid captive after the original detach store */
            XASSERT((uint32)slot < ACTOR_COUNT);
            actor_contacts[slot].captor = ACTOR_SLOT_NONE;
            if (state == 169)
            {
                type = actor_types[slot];
                /* TODO: Investigate an invalid captive type after detachment */
                XASSERT((uint32)type < ANIMATION_TYPE_COUNT);
                animation_start(slot, animation_type_banks[(uint32)type], 148);
            }
        }
        type = actor_types[idx];
        /* TODO: Investigate a changed victim type after drop and captive animation */
        XASSERT((uint32)type < ANIMATION_TYPE_COUNT);
        animation_start(idx, animation_type_banks[(uint32)type], 147);
    }
    return 0;
}

GDB_CALL sint32 actor_process_stage_contact(COLLISION_POINT *contact, sint32 idx)
{
    FUNCTION_MARKER(0x8001bbe8, "GAME.EXE");
    MODEL_OBJECT *target_model = NULL;
    uint32 angle, flags;
    COLLISION_REGION *p;
    sint32 i, type, shape, x = contact->pos.x >> 16, z = contact->pos.z >> 16;
    /* TODO: Investigate stage contact from an invalid actor slot */
    XASSERT(idx >= 0 && idx < ACTOR_COUNT);
    if ((uint16)((sint16)actor_animations[idx].hit_flags) & 0x8000u)
        return 0;
    for (i = 0; i < (sint32)col_regions.count; i++)
    {
        XASSERT((uint32)i < COLLISION_REGION_COUNT);
        p = col_regions.data + i;
        type = p->col_kind;
        if ((type < 3 && type != 1) || p->hits < 0)
            continue;
        shape = p->shape;
        if (!shape)
        {
            if (!(p->geometry.axis.min_x < x && x < p->geometry.axis.max_x && p->geometry.axis.min_z < z && z < p->geometry.axis.max_z))
                continue;
        }
        else if (shape == 1)
        {
            uint32 dx, dz, s, c;
            sint32 local_x, local_z, width, depth;
            /* TODO: Investigate an oriented region without a valid world owner */
            XASSERT(p->object.present == 1 && p->object.idx < WORLD_OBJECT_COUNT);
            uint32 world_idx = p->object.idx;
            const MODEL_OBJECT *model = world_objects.object + world_idx;
            target_model = world_objects.object + world_idx;
            angle = model->angles.yaw_bits >> 5;
            dx = x - (uint32)(sint32)model->pos.x.whole;
            c = sin_table[MATH_QUARTER_TURN + angle];
            dz = z - (uint32)(sint32)model->pos.z.whole;
            s = sin_table[angle];
            local_x = (sint32)(dx * c + dz * s) >> 15;
            local_z = (sint32)(dz * c - dx * s) >> 15;
            width = p->geometry.box.half_x;
            if (local_x <= -width || local_x >= width)
                continue;
            depth = p->geometry.box.half_z;
            if (local_z <= -depth || local_z >= depth)
                continue;
            {
                sint32 a = (sint16)math_dir11(local_x, local_z, 0, 0);
                sint32 b;
                uint32 side;
                depth = p->geometry.box.half_z;
                width = p->geometry.box.half_x;
                b = (sint16)math_dir11(0, 0, width, -depth);
                if (a < b || a > 2048 - b)
                    side = 0;
                else if (a < 1024 - b)
                    side = 1;
                else
                    side = a < b + 1024 ? 2 : 3;
                col_regions.side = side;
            }
        }
        else
        {
            sint32 center_x = p->geometry.circle.center_x, center_z = p->geometry.circle.center_z;
            sint32 distance = math_distance_approx(x, z, center_x, center_z);
            if (distance >= p->geometry.circle.radius)
                continue;
        }
        flags = actor_animations[idx].hit_flags;
        if (shape != 1)
            target_model = p->object.present == 1 && p->object.idx < WORLD_OBJECT_COUNT ? world_objects.object + p->object.idx : NULL;
        actor_animations[idx].hit_flags = (uint16)(flags | 0x8000);
        sint32 handler_idx = p->col_kind;
        XASSERT(handler_idx >= 0 && handler_idx < COLLISION_SLOT_COUNT);
        return col_call(col_callbacks[handler_idx], p, target_model, contact, idx);
    }
    return 0;
}

GDB_CALL sint32 actor_projectile_contact(sint32 owner, COLLISION_POINT *contact, uint32 kind)
{
    FUNCTION_MARKER(0x8001bfa0, "GAME.EXE");
    sint32 i, type, other_x, other_z, range_type, distance, radius;
    uint32 state, x, y, z, height, offset, angle;
    /* TODO: Investigate a projectile contact without native point storage */
    XASSERT(contact);
    x = contact->pos.bits[0];
    y = contact->pos.bits[1];
    contact->impact_sound = 0;
    z = contact->pos.bits[2];
    if (actor_process_stage_contact(contact, owner))
        return 2;
    for (i = 0; i < 8; i++)
    {
        if (game_session.mode == 2 && i >= 2)
            return 0;
        if (i == owner)
            continue;
        type = actor_types[i];
        if (type == -1 || actor_lifecycle[i].delay || type == 17 || type == 51)
            continue;
        height = actor_positions[i].y.bits;
        offset = actor_animations[i].motion.bits[0];
        if ((sint32)(height + offset) < -16777216)
            continue;
        if (actor_animation_blocks_hits(actor_animations[i].id))
            continue;
        other_x = actor_positions[i].x.whole;
        other_z = actor_positions[i].z.whole;
        math_dir16((sint32)x >> 16, (sint32)z >> 16, other_x, other_z);
        range_type = actor_types[i];
        distance = dir_distance;
        /* TODO: Investigate a reloaded projectile target type outside the radius table */
        XASSERT((uint32)range_type < ACTOR_TYPE_COUNT);
        radius = actor_col.radius[range_type];
        if (radius < distance)
            continue;
        angle = (uint32)(sint32)actor_orientation[i].yaw << 5;
        state = actor_animations[i].id;
        if (state == 253 || state == 156 || state == 252 || state == 155 || state == 147 || state == 154 || state == 191 || state - 149u < 2u || state == 148)
            continue;
        {
            uint32 table, has_animation;
            int j;
            sound_voice_play(76, 255, 6144);
            sound_play_pain(i);
            actor_items_drop_on_hit(i);
            actor_contacts[i].last_attacker = (uint8)owner;
            dynamic_objects_spawn_transient(865, x, y, z);
            for (j = 0; j < 4; j++)
                effect_spawn_actor_hit_reverse(i, x, y, z, angle);
            for (j = 0; j < 12; j++)
                effect_spawn_actor_hit(i, x, y, z, angle);
            object_apply_damage(owner, i, kind);
            if (actor_health[i].hp <= 0)
            {
                actors_rec_death(i);
                /* TODO: Investigate a saved projectile type before death animation */
                XASSERT((uint32)type < ANIMATION_TYPE_COUNT);
                table = animation_type_banks[(uint32)type];
                animation_start(i, table, 147);
                actors_detach_companion(i);
                actor_health[i].hp = 0;
            }
            else
            {
                if ((sint32)kind >= 522 || kind == 695 || kind - 708u < 2u || kind == 710 || kind == 408)
                {
                    actor_contacts[i].captor = ACTOR_SLOT_NONE;
                    actor_health[i].stun = 0;
                    actor_items_drop(i);
                    /* TODO: Investigate a saved projectile type before drop animation */
                    XASSERT((uint32)type < ANIMATION_TYPE_COUNT);
                    table = animation_type_banks[(uint32)type];
                    if ((animation_stream(table, 147).kind != ACTOR_ANIMATION_SOURCE_NONE))
                    {
                        table = animation_type_banks[(uint32)type];
                        animation_start(i, table, 147);
                    }
                    else
                    {
                        table = animation_type_banks[(uint32)type];
                        has_animation = animation_stream(table, 234).kind != ACTOR_ANIMATION_SOURCE_NONE;
                        table = animation_type_banks[(uint32)type];
                        animation_start(i, table, has_animation ? 234 : 133);
                    }
                }
                else
                {
                    /* TODO: Investigate a saved projectile type before reaction animation */
                    XASSERT((uint32)type < ANIMATION_TYPE_COUNT);
                    table = animation_type_banks[(uint32)type];
                    animation_start(i, table, 133);
                }
            }
            actors_detach_companion(i);
            if ((sint32)game_session.mode < 2)
                actors_rec_hit(owner, i);
            return 1;
        }
    }
    return 0;
}

GDB_CALL sint32 actor_process_attachment_hits(sint32 idx, sint32 attachment)
{
    FUNCTION_MARKER(0x8001ced0u, "GAME.EXE");
    COLLISION_POINT point;
    actor_attachment_pos(idx, attachment, point.pos.bits);
    return actor_process_hits(idx, &point);
}

GDB_CALL sint32 actor_projectile_impact(uint32 world_x, uint32 world_z, const struct MODEL_OBJECT *model, const struct EFFECT *effect)
{
    return projectile_impact(world_x, world_z, model, NULL, effect);
}

GDB_CALL sint32 actor_projectile_impact_src(uint32 world_x, uint32 world_z, const PROJECTILE_SOURCE *projectile)
{
    return projectile_impact(world_x, world_z, NULL, projectile, NULL);
}

GDB_CALL sint32 actors_apply_radial_damage(uint32 world_x, uint32 world_z, const struct EFFECT *effect, const struct MODEL_OBJECT *model)
{
    FUNCTION_MARKER(0x80031f64u, "GAME.EXE");
    /* TODO: Investigate a missing, ambiguous or cross-format radial source */
    XASSERT((effect || model) && (!effect || !model) && (!effect || effect->payload_kind == EFFECT_PAYLOAD_SPRITE) && (!effect || effect->kind - 870u < 5u || effect->kind == 877 || effect->kind == 879 || effect->kind == 880 || effect->kind - 891u < 5u || effect->kind - 575u < 2u || effect->kind - 602u < 6u || effect->kind == 0xffffffffu) && (!model || model->payload_kind == MODEL_PAYLOAD_PHYSICS));
    const EFFECT_SPRITE *spr = effect ? &effect->spr : NULL;
    const FIXED_VEC3 *pos = spr ? &spr->pos : &model->pos;
    uint32 y = pos->y.bits, kind, animation;
    sint32 owner;
    int i, j;
    sint32 radius, type, x = (sint32)world_x >> 16, z = (sint32)world_z >> 16;
    for (i = 0; i < ACTOR_COUNT; i++)
    {
        /* TODO: Investigate cross-format reuse during original radial damage */
        XASSERT((!effect || effect->payload_kind == EFFECT_PAYLOAD_SPRITE) && (!model || model->payload_kind == MODEL_PAYLOAD_PHYSICS));
        if ((spr ? spr->owner : model->physics.actor_idx) == i)
            continue;
        if (actor_lifecycle[i].delay)
            continue;
        type = actor_types[i];
        if (type == -1 || type == 17 || type == 51)
            continue;
        if (actor_positions[i].x.bits == world_x && actor_positions[i].z.bits == world_z)
            continue;
        if (actor_animations[i].id == 205 || actor_animations[i].id == 203 || actor_health[i].hp <= 0)
            continue;
        kind = effect ? effect->kind : model->kind;
        radius = (kind == 515 || kind == 574) ? 256 : 512;
        if (math_distance_approx(x, z, actor_positions[i].x.whole, actor_positions[i].z.whole) >= radius)
            continue;
        kind = effect ? effect->kind : model->kind;
        if (kind == 572)
            actor_apply_damage(spr ? spr->owner : model->physics.actor_idx, i, -160);
        else if (kind == 574)
            actor_apply_damage(spr ? spr->owner : model->physics.actor_idx, i, -120);
        else
            object_apply_damage(spr ? spr->owner : model->physics.actor_idx, i, effect ? effect->kind : model->kind);
        /* TODO: Investigate cross-format reuse during original radial damage */
        XASSERT((!effect || effect->payload_kind == EFFECT_PAYLOAD_SPRITE) && (!model || model->payload_kind == MODEL_PAYLOAD_PHYSICS));
        owner = spr ? spr->owner : model->physics.actor_idx;
        /* TODO: Investigate original invalid hit-credit actor indices */
        XASSERT(owner >= 0 && owner < ACTOR_COUNT);
        actors_rec_hit(owner, i);
        /* TODO: Investigate cross-format reuse during original radial damage */
        XASSERT((!effect || effect->payload_kind == EFFECT_PAYLOAD_SPRITE) && (!model || model->payload_kind == MODEL_PAYLOAD_PHYSICS));
        kind = effect ? effect->kind : model->kind;
        if (kind - 499u < 5u)
        {
            /* TODO: Investigate raw item kind publication over native sprite data */
            XASSERT(!spr);
            if (actor_animations[i].id != 147)
            {
                actor_orientation[i].yaw = (uint16)(((uint16)model->angles.yaw >> 5) ^ 1024u);
                dynamic_objects_spawn_transient(865, world_x, y, world_z);
                for (j = 0; j < 4; j++)
                    effect_spawn_actor_hit_reverse(i, world_x, y, world_z, (sint32)model->angles.yaw);
                for (j = 0; j < 12; j++)
                    effect_spawn_actor_hit(i, world_x, y, world_z, (sint32)model->angles.yaw);
                sound_play_pain(i);
                if (actor_health[i].stun < 49 && actor_health[i].hp > 0)
                    animation = (random_next() & 1u) + 249u;
                else
                    animation = 251;
                animation_start(i, animation_type_banks[(uint32)actor_types[i]], animation);
            }
            return actors_detach_companion(i);
        }
        actor_orientation[i].yaw = (uint16)(math_dir11(x, z, actor_positions[i].x.whole, actor_positions[i].z.whole) ^ 1024u);
        actor_items_drop(i);
        animation_start(i, animation_type_banks[(uint32)actor_types[i]], 147);
        actors_detach_companion(i);
    }
    return 0;
}

GDB_CALL sint32 actor_animation_blocks_hits(uint32 state)
{
    FUNCTION_MARKER(0x80048cfcu, "GAME.EXE");
    return state == 147 || state == 154 || state - 149u < 2u || state == 41 || state == 132 || state - 177u < 3u || state - 180u < 10u || state - 130u < 2u || state == 77 || state == 167 || state == 169 || state == 33 || state == 148 || state == 205 || state == 203 || state == 172 || state == 36;
}
