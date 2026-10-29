#include <stdio.h>

#include "game.h"

ACTOR_ITEMS actor_items[ACTOR_COUNT];

sint32 actor_items_drop_on_hit(sint32 idx)
{
    FUNCTION_MARKER(0x80017c78, "GAME.EXE");
    MODEL_OBJECT *item;
    FIXED_VEC3 pos;
    sint32 slot, type;
    /* TODO: Investigate an item operation outside the actor pool */
    XASSERT(idx >= 0 && idx < ACTOR_COUNT);
    slot = actor_items[idx].slot[ACTOR_ITEM_RIGHT];
    if (slot != -1)
    {
        /* TODO: Investigate an invalid held item before accessing its payload */
        XASSERT(slot >= 0 && slot < DYNAMIC_OBJECT_COUNT);
        item = dynamic_objects + slot;
        type = item->kind;
        if (type >= 522 || type == 512)
        {
            actor_attachment_pos(idx, 3, pos.bits);
            item->pos.x.bits = pos.x.bits;
            item->pos.y.bits = pos.y.bits;
            item->pos.z.bits = pos.z.bits;
            actor_items[idx].slot[ACTOR_ITEM_RIGHT] = -1;
            /* TODO: Investigate an item transfer with a foreign native owner */
            XASSERT(item->payload_kind == MODEL_PAYLOAD_PHYSICS);
            item->physics.speed = 4;
        }
    }
    slot = actor_items[idx].slot[ACTOR_ITEM_LEFT];
    if (slot == -1)
        return -8;
    /* TODO: Investigate an invalid held item before accessing its payload */
    XASSERT(slot >= 0 && slot < DYNAMIC_OBJECT_COUNT);
    item = dynamic_objects + slot;
    if ((sint32)item->kind < 522)
        return 1;
    actor_attachment_pos(idx, 2, pos.bits);
    item->pos.x.bits = pos.x.bits;
    item->pos.y.bits = pos.y.bits;
    item->pos.z.bits = pos.z.bits;
    actor_items[idx].slot[ACTOR_ITEM_LEFT] = -1;
    /* TODO: Investigate an item transfer with a foreign native owner */
    XASSERT(item->payload_kind == MODEL_PAYLOAD_PHYSICS);
    item->physics.speed = 4;
    return 4;
}

uint32 actor_items_drop(sint32 idx)
{
    FUNCTION_MARKER(0x80017e30, "GAME.EXE");
    FIXED_VEC3 point;
    uint32 result;
    /* TODO: Investigate drop actors outside the native equipment domain */
    XASSERT(idx >= 0 && idx < ACTOR_COUNT);
    sint32 slot = actor_items[idx].slot[ACTOR_ITEM_RIGHT];
    if (slot != -1)
    {
        actor_attachment_pos(idx, 3, point.bits);
        /* TODO: Investigate original held-item references outside the dynamic pool */
        XASSERT(slot >= 0 && slot < DYNAMIC_OBJECT_COUNT);
        MODEL_OBJECT *item = dynamic_objects + slot;
        item->pos.x.bits = point.x.bits;
        item->pos.y.bits = point.y.bits;
        item->pos.z.bits = point.z.bits;
        actor_items[idx].slot[ACTOR_ITEM_RIGHT] = -1;
        if (item->payload_kind == MODEL_PAYLOAD_PANEL || item->payload_kind == MODEL_PAYLOAD_SIDE_PANEL)
            item->panel.speed = 4;
        else if (item->payload_kind == MODEL_PAYLOAD_BALLISTIC)
            item->ballistic.speed = 4;
        else if (item->payload_kind == MODEL_PAYLOAD_DRIFT)
            item->drift.speed = 4;
        else if (item->payload_kind == MODEL_PAYLOAD_SHAKE)
            item->shake.speed = 4;
        else if (item->payload_kind == MODEL_PAYLOAD_GATE_FALL)
            item->gate_fall.speed = 4;
        else if (item->payload_kind == MODEL_PAYLOAD_GATE_FRAGMENT)
            item->gate_fragment.speed = 4;
        else if (item->payload_kind == MODEL_PAYLOAD_ROTATION)
            item->rotation.amplitude = 4;
        else if (item->payload_kind == MODEL_PAYLOAD_BOUNCING_DROP)
            item->drop.speed = 4;
        else if (item->payload_kind == MODEL_PAYLOAD_BOUNCING_PART)
            item->bounce.speed = 4;
        else if (item->payload_kind == MODEL_PAYLOAD_TRAFFIC)
            item->traffic.speed = 4;
        else if (item->payload_kind == MODEL_PAYLOAD_TANK)
            item->tank.speed = 4;
        else if (item->payload_kind == MODEL_PAYLOAD_OSCILLATION)
            item->oscillation.speed = 4;
        else if (item->payload_kind == MODEL_PAYLOAD_COMPANION)
            item->companion.speed = 4;
        else if (item->payload_kind == MODEL_PAYLOAD_LAB_TANK)
            item->lab_tank.speed = 4;
        else if (item->payload_kind == MODEL_PAYLOAD_WOBBLE)
            item->wobble.speed = 4;
        else if (item->payload_kind == MODEL_PAYLOAD_PHYSICS)
            item->physics.speed = 4;
        else if (item->payload_kind == MODEL_PAYLOAD_ARMY_TRUCK)
            item->truck.speed = 4;
        else if (item->payload_kind == MODEL_PAYLOAD_AIR_BIKE)
            item->air_bike.target[0].z = 4;
        else if (item->payload_kind == MODEL_PAYLOAD_SCENE_VEHICLE)
            item->scene_vehicle.speed = 4;
        else if (item->payload_kind == MODEL_PAYLOAD_SCENE_HELICOPTER)
            item->scene_helicopter.speed = 4;
        else if (item->payload_kind == MODEL_PAYLOAD_COMBAT_HELICOPTER)
            item->helicopter.shot_distance = 4;
        else if (item->payload_kind == MODEL_PAYLOAD_PATROL_HELICOPTER)
            item->patrol.speed = 4;
        else if (item->payload_kind == MODEL_PAYLOAD_TRAIN)
            item->train.speed = 4;
        else if (item->payload_kind == MODEL_PAYLOAD_SUSPENDED)
            item->suspended.actor_idx = 4;
        else if (item->payload_kind == MODEL_PAYLOAD_UNMIGRATED)
            item->pending_motion.speed = 4;
        else if (item->payload_kind == MODEL_PAYLOAD_SPRITE)
            item->spr.speed = 4;
        else if (item->payload_kind == MODEL_PAYLOAD_WAVE)
            item->wave.speed = 4;
        else
        {
            /* TODO: Investigate an unpublished generic model field owner */

            XASSERT(0);
        }
    }
    slot = actor_items[idx].slot[ACTOR_ITEM_LEFT];
    result = 0xffffffff;
    if (slot != -1)
    {
        actor_attachment_pos(idx, 2, point.bits);
        /* TODO: Investigate original held-item references outside the dynamic pool */
        XASSERT(slot >= 0 && slot < DYNAMIC_OBJECT_COUNT);
        result = 36u * slot;
        MODEL_OBJECT *item = dynamic_objects + slot;
        item->pos.x.bits = point.x.bits;
        item->pos.y.bits = point.y.bits;
        item->pos.z.bits = point.z.bits;
        actor_items[idx].slot[ACTOR_ITEM_LEFT] = -1;
        if (item->payload_kind == MODEL_PAYLOAD_PANEL || item->payload_kind == MODEL_PAYLOAD_SIDE_PANEL)
            item->panel.speed = 4;
        else if (item->payload_kind == MODEL_PAYLOAD_BALLISTIC)
            item->ballistic.speed = 4;
        else if (item->payload_kind == MODEL_PAYLOAD_DRIFT)
            item->drift.speed = 4;
        else if (item->payload_kind == MODEL_PAYLOAD_SHAKE)
            item->shake.speed = 4;
        else if (item->payload_kind == MODEL_PAYLOAD_GATE_FALL)
            item->gate_fall.speed = 4;
        else if (item->payload_kind == MODEL_PAYLOAD_GATE_FRAGMENT)
            item->gate_fragment.speed = 4;
        else if (item->payload_kind == MODEL_PAYLOAD_ROTATION)
            item->rotation.amplitude = 4;
        else if (item->payload_kind == MODEL_PAYLOAD_BOUNCING_DROP)
            item->drop.speed = 4;
        else if (item->payload_kind == MODEL_PAYLOAD_BOUNCING_PART)
            item->bounce.speed = 4;
        else if (item->payload_kind == MODEL_PAYLOAD_TRAFFIC)
            item->traffic.speed = 4;
        else if (item->payload_kind == MODEL_PAYLOAD_TANK)
            item->tank.speed = 4;
        else if (item->payload_kind == MODEL_PAYLOAD_OSCILLATION)
            item->oscillation.speed = 4;
        else if (item->payload_kind == MODEL_PAYLOAD_COMPANION)
            item->companion.speed = 4;
        else if (item->payload_kind == MODEL_PAYLOAD_LAB_TANK)
            item->lab_tank.speed = 4;
        else if (item->payload_kind == MODEL_PAYLOAD_WOBBLE)
            item->wobble.speed = 4;
        else if (item->payload_kind == MODEL_PAYLOAD_PHYSICS)
            item->physics.speed = 4;
        else if (item->payload_kind == MODEL_PAYLOAD_ARMY_TRUCK)
            item->truck.speed = 4;
        else if (item->payload_kind == MODEL_PAYLOAD_TRAIN)
            item->train.speed = 4;
        else if (item->payload_kind == MODEL_PAYLOAD_SUSPENDED)
            item->suspended.actor_idx = 4;
        else if (item->payload_kind == MODEL_PAYLOAD_UNMIGRATED)
            item->pending_motion.speed = 4;
        else if (item->payload_kind == MODEL_PAYLOAD_SPRITE)
            item->spr.speed = 4;
        else if (item->payload_kind == MODEL_PAYLOAD_AIR_BIKE)
            item->air_bike.target[0].z = 4;
        else if (item->payload_kind == MODEL_PAYLOAD_SCENE_VEHICLE)
            item->scene_vehicle.speed = 4;
        else if (item->payload_kind == MODEL_PAYLOAD_SCENE_HELICOPTER)
            item->scene_helicopter.speed = 4;
        else if (item->payload_kind == MODEL_PAYLOAD_COMBAT_HELICOPTER)
            item->helicopter.shot_distance = 4;
        else if (item->payload_kind == MODEL_PAYLOAD_PATROL_HELICOPTER)
            item->patrol.speed = 4;
        else if (item->payload_kind == MODEL_PAYLOAD_WAVE)
            item->wave.speed = 4;
        else
        {
            /* TODO: Investigate an unpublished generic model field owner */

            XASSERT(0);
        }
    }
    return result;
}

uint32 actor_items_calc_weapon_point(sint32 idx, sint32 half, uint32 out[3])
{
    FUNCTION_MARKER(0x8001cf08, "GAME.EXE");
    /* TODO: Investigate invalid weapon references at the original access */
    XASSERT(idx >= 0 && idx < ACTOR_COUNT);
    sint32 slot = actor_items[idx].slot[ACTOR_ITEM_RIGHT];
    /* TODO: Investigate invalid weapon references at the original access */
    XASSERT(slot >= -1 && slot < DYNAMIC_OBJECT_COUNT);
    /* Preserve the original zero length for an empty hand without reading before the pool */
    uint32 kind = slot == -1 ? 0u : dynamic_objects[slot].kind;
    sint32 len = 0;
    transform_push();
    transform_load(&actor_poses[idx].matrix[1]);
    transform_yaw(6144);
    if (kind == 477 || kind == 491 || kind - 493u < 2u || kind == 492 || kind - 484u < 2u || kind == 519 || kind == 495 || kind == 520)
        len = 6291456;
    if (kind == 486 || kind == 488)
        len = 3145728;
    if (kind == 487)
        len = 6291456;
    if (kind == 489)
        len = 9437184;
    if (kind - 478u < 2u || kind - 480u < 2u || kind - 482u < 2u)
        len = 3145728;
    if (half)
        len >>= 1;
    transform_point(0, 0, len, out);
    transform_identity();
    transform_yaw(-camera.angles.yaw);
    transform_pitch(-camera.angles.pitch);
    transform_point(out[0], out[1], out[2], out);
    transform_pop();
    out[0] += camera.pos.x.bits;
    out[1] += camera.pos.y.bits;
    out[2] += camera.pos.z.bits;
    return out[2];
}

sint32 actor_items_hit_weapon(sint32 idx, sint32 half)
{
    FUNCTION_MARKER(0x8001d10c, "GAME.EXE");
    uint32 kind, x, y, z;
    COLLISION_POINT point;
    sint32 result, i, type, distance, radius, other_x, other_z, actor_x, actor_z;
    uint32 wear;
    ANIMATION_BANK_ID bank;
    /* TODO: Investigate invalid weapon references at the original access */
    XASSERT(idx >= 0 && idx < ACTOR_COUNT);
    sint32 slot = actor_items[idx].slot[ACTOR_ITEM_RIGHT];
    /* A broken weapon can leave hit events in the remaining animation frames */
    if (slot == -1)
        return 0;
    /* TODO: Investigate invalid weapon references at the original access */
    XASSERT(slot >= 0 && slot < DYNAMIC_OBJECT_COUNT);
    MODEL_OBJECT *item = dynamic_objects + slot;
    kind = item->kind;
    actor_items_calc_weapon_point(idx, half, point.pos.bits);
    x = point.pos.bits[0];
    y = point.pos.bits[1];
    z = point.pos.bits[2];
    point.impact_sound = 1;
    result = actor_process_stage_contact(&point, idx);
    if (result)
    {
        /* TODO: Investigate a weapon wear access with a foreign native owner */
        XASSERT(item->payload_kind == MODEL_PAYLOAD_PHYSICS);
        wear = ((uint16)item->physics.life) - 1;
        item->physics.life = (uint16)wear;
        if ((sint16)wear <= 0)
        {
            actor_items_drop(idx);
            /* TODO: Investigate a weapon wear access with a foreign native owner */
            XASSERT(item->payload_kind == MODEL_PAYLOAD_PHYSICS);
            item->physics.life = -1;
        }
        return result;
    }
    point.impact_sound = 0;
    for (i = 2; i < 8; i++)
    {
        if (i == idx)
            continue;
        type = actor_types[i];
        if (type == -1 || actor_lifecycle[i].delay || type == 17 || type == 51)
            continue;
        if ((((sint16)actor_animations[idx].hit_flags) >> i) & 1)
            continue;
        if (actor_animation_blocks_hits(actor_animations[i].id))
            continue;
        /* TODO: Investigate an invalid victim type before animation-bank selection */
        XASSERT(type >= 0 && type < ANIMATION_TYPE_COUNT);
        bank = animation_type_banks[type];
        if ((sint32)actor_positions[i].y.bits < 0)
            continue;
        other_x = actor_positions[i].x.whole;
        other_z = actor_positions[i].z.whole;
        distance = math_distance_approx((sint32)x >> 16, (sint32)z >> 16, other_x, other_z);
        radius = actor_types[i];
        /* TODO: Investigate an invalid reloaded victim type before collision-radius selection */
        XASSERT(radius >= 0 && radius < ACTOR_TYPE_COUNT);
        radius = actor_col.radius[radius];
        dir_distance = distance;
        if (distance < radius)
        {
            uint32 yaw;
            sint16 dir;
            int j;
            sint16 bearing;
            other_x = actor_positions[i].x.whole;
            other_z = actor_positions[i].z.whole;
            actor_x = actor_positions[idx].x.whole;
            actor_z = actor_positions[idx].z.whole;
            bearing = (sint16)math_dir16(other_x, other_z, actor_x, actor_z);
            yaw = (uint16)actor_orientation[i].yaw;
            dir = (sint16)((yaw << 5) - (bearing)-0x4000u);
            yaw = (sint16)yaw;
            dynamic_objects_spawn_transient(865, x, y, z);
            sound_play_pain(i);
            actor_items_drop_on_hit(i);
            sound_voice_play(23, 255, 6144);
            /* TODO: Investigate a weapon wear access with a foreign native owner */
            XASSERT(item->payload_kind == MODEL_PAYLOAD_PHYSICS);
            wear = ((uint16)item->physics.life) - 1;
            item->physics.life = (uint16)wear;
            yaw ^= 0x400u;
            if ((sint16)wear <= 0)
            {
                actor_items_drop(idx);
                /* TODO: Investigate a weapon wear access with a foreign native owner */
                XASSERT(item->payload_kind == MODEL_PAYLOAD_PHYSICS);
                item->physics.life = -1;
            }
            if (kind == 478 || kind == 480 || kind == 482)
            {
                uint32 debris = kind / 2u + 360u;
                item->kind = kind + 1u;
                sound_voice_play(113, 255, 6400);
                for (j = 0; j < 12; j++)
                    effect_spawn_glass_fragment(debris, x, y, z, yaw - 512u);
            }
            actor_contacts[i].last_attacker = (uint8)idx;
            for (j = 0; j < 8; j++)
                effect_spawn_actor_hit_reverse(i, x, y, z, yaw);
            for (j = 0; j < 32; j++)
                effect_spawn_actor_hit(i, x, y, z, yaw);
            object_apply_damage(idx, i, kind);
            if (actor_health[i].hp <= 0)
            {
                actors_rec_death(i);
                animation_start(i, bank, 147);
                actor_health[i].hp = 0;
            }
            else
            {
                actor_animations[idx].hit_flags = (uint16)(actor_animations[idx].hit_flags | (1u << i));
                if (actor_health[i].stun >= 49)
                {
                    animation_start(i, bank, dir < 0 ? 147 : 154);
                    actor_health[i].stun = 0;
                    actor_items_drop(i);
                }
                else
                    animation_start(i, bank, dir < 0 ? 133 : ((random_next() & 1u) | 152u));
            }
            actors_rec_hit(idx, i);
            actors_detach_companion(i);
            return 3;
        }
    }
    return 0;
}

sint32 animation_start_held(sint32 idx, ANIMATION_BANK_ID table, sint32 animation)
{
    FUNCTION_MARKER(0x800237CC, "GAME.EXE");
    sint32 slot;
    uint32 type;
    /* TODO: Investigate held-animation actors outside the native pool */
    XASSERT(idx >= 0 && idx < ACTOR_COUNT);
    actor_movement[idx].prev_state = (uint16)animation_movement_state(actor_animations[idx].id);
    slot = actor_items[idx].slot[ACTOR_ITEM_LEFT];
    if (slot != -1)
    {
        /* TODO: Investigate original held-item references outside the dynamic pool */
        XASSERT(slot >= 0 && slot < DYNAMIC_OBJECT_COUNT);
        type = dynamic_objects[slot].kind;
        if (type == 515)
        {
            if (animation == 69)
                animation = 73;
            if (animation == 82)
                animation = 83;
            if (animation == 87)
                animation = 88;
            if (animation == 92)
                animation = 95;
        }
        else if (type == 504)
        {
            if (animation == 69)
                animation = 78;
            if (animation == 82)
                animation = 86;
            if (animation == 87)
                animation = 90;
            if (animation == 92)
                animation = 97;
        }
    }
    slot = actor_items[idx].slot[ACTOR_ITEM_RIGHT];
    if (slot != -1)
    {
        /* TODO: Investigate original held-item references outside the dynamic pool */
        XASSERT(slot >= 0 && slot < DYNAMIC_OBJECT_COUNT);
        type = dynamic_objects[slot].kind;
        if (type == 515)
        {
            if (animation == 69)
                animation = 73;
            if (animation == 82)
                animation = 83;
            if (animation == 87)
                animation = 88;
            if (animation == 92)
                animation = 95;
        }
        else if (type - 499u < 5u || type == 505)
        {
            if (animation == 69)
                animation = 72;
            if (animation == 92)
                animation = 94;
        }
        else if (type == 477 || type == 491 || type == 493 || type - 484u < 2u || type - 494u < 2u)
        {
            if (animation == 69 && (animation_stream(table, 71).kind != ACTOR_ANIMATION_SOURCE_NONE))
                animation = 71;
        }
        else if (type == 509 || type - 506u < 2u)
        {
            if (animation == 69)
                animation = 76;
            if (animation == 82)
                animation = 84;
            if (animation == 87)
                animation = 89;
            if (animation == 92)
                animation = 96;
        }
        else if ((sint32)type >= 522)
        {
            if (animation == 69)
                animation = 70;
            if (animation == 82)
                animation = 85;
            if (animation == 92)
                animation = 93;
            if (animation == 87 || animation == 35)
                return 35;
        }
    }
    return animation_start(idx, table, animation);
}

sint32 actor_items_fire_projectile(sint32 idx)
{
    FUNCTION_MARKER(0x800301ccu, "GAME.EXE");
    uint32 kind;
    FIXED_VEC3 point;
    sint32 held_slot, slot;
    /* TODO: Investigate an item operation outside the actor pool */
    XASSERT(idx >= 0 && idx < ACTOR_COUNT);
    held_slot = actor_items[idx].slot[ACTOR_ITEM_RIGHT];
    slot = dynamic_objects_allocate();
    if (slot < 0)
        return 0;
    sound_voice_play(21, 255, 6144);
    actor_attachment_pos(idx, 3, point.bits);
    /* TODO: Investigate an invalid held item before accessing its payload */
    XASSERT(held_slot >= 0 && held_slot < DYNAMIC_OBJECT_COUNT);
    const MODEL_OBJECT *held = dynamic_objects + held_slot;
    MODEL_OBJECT *projectile = dynamic_objects + slot;
    kind = held->kind == 505 ? 574u : 572u;
    projectile->kind = kind;
    projectile->payload_kind = MODEL_PAYLOAD_PHYSICS;
    projectile->pos.x.bits = point.x.bits;
    projectile->pos.y.bits = point.y.bits;
    projectile->pos.z.bits = point.z.bits;
    projectile->angles.pitch_bits = 0;
    projectile->angles.roll_bits = 0;
    projectile->angles.yaw_bits = (uint16)(((uint16)actor_orientation[idx].yaw) << 5);
    projectile->physics.speed = kind == 574 ? 128 : 256;
    projectile->physics.target_idx = 0;
    projectile->physics.vertical_speed = 0;
    projectile->physics.actor_idx = (uint16)idx;
    projectile->physics.target_idx = actor_targeting.front.available == 1 ? (uint16)actor_targeting.actor_idx : 65535;
    return 0;
}

sint32 actor_items_fire_ray(sint32 idx)
{
    FUNCTION_MARKER(0x80030310u, "GAME.EXE");
    const MODEL_OBJECT *held;
    uint32 kind;
    FIXED_VEC3 point, pos;
    uint16 yaw;
    /* TODO: Investigate invalid weapon references at the original access */
    XASSERT(idx >= 0 && idx < ACTOR_COUNT);
    sint32 slot = actor_items[idx].slot[ACTOR_ITEM_RIGHT];
    if (slot == -1)
    {
        slot = actor_items[idx].slot[ACTOR_ITEM_LEFT];
        if (slot == -1)
            return 0;
        slot = actor_items[idx].slot[ACTOR_ITEM_LEFT];
    }
    sound_voice_play(19, 160, 7168);
    actor_attachment_pos(idx, actor_types[idx] == 2 ? 2 : 3, point.bits);
    /* TODO: Investigate invalid weapon references at the original access */
    XASSERT(slot >= 0 && slot < DYNAMIC_OBJECT_COUNT);
    held = dynamic_objects + slot;
    kind = held->kind;
    pos.x.bits = actor_positions[idx].x.bits;
    pos.z.bits = actor_positions[idx].z.bits;
    pos.y.bits = point.y.bits;
    yaw = ((uint16)actor_orientation[idx].yaw) << 5;
    projectile_trace_weapon(pos.bits, kind, yaw, idx);
    return 0;
}

sint32 actor_items_fire_spread(sint32 idx)
{
    FUNCTION_MARKER(0x80030440u, "GAME.EXE");
    const MODEL_OBJECT *held;
    uint32 kind;
    FIXED_VEC3 point, pos;
    uint16 yaw;
    /* TODO: Investigate invalid weapon references at the original access */
    XASSERT(idx >= 0 && idx < ACTOR_COUNT);
    sint32 slot = actor_items[idx].slot[ACTOR_ITEM_RIGHT];
    if (slot == -1)
        slot = actor_items[idx].slot[ACTOR_ITEM_LEFT];
    /* TODO: Investigate invalid weapon references at the original access */
    XASSERT(slot >= 0 && slot < DYNAMIC_OBJECT_COUNT);
    held = dynamic_objects + slot;
    if (held->kind == 504)
        sound_voice_play(27, 255, 5376);
    else
        sound_voice_play(24, 255, 6144);
    sound_voice_play(19, 128, 8192);
    actor_attachment_pos(idx, 3, point.bits);
    kind = held->kind;
    kind = kind - 499 < 5 || kind - 504 < 2 ? held->kind : 499;
    pos.x.bits = actor_positions[idx].x.bits;
    pos.z.bits = actor_positions[idx].z.bits;
    pos.y.bits = point.y.bits;
    yaw = ((uint16)actor_orientation[idx].yaw) << 5;
    if (!projectile_trace_weapon(pos.bits, kind, yaw, idx))
    {
        yaw = yaw - 384;
        pos.x.bits = actor_positions[idx].x.bits;
        pos.z.bits = actor_positions[idx].z.bits;
        if (!projectile_trace_weapon(pos.bits, kind, yaw, idx))
        {
            yaw = yaw + 768;
            pos.x.bits = actor_positions[idx].x.bits;
            pos.z.bits = actor_positions[idx].z.bits;
            projectile_trace_weapon(pos.bits, kind, yaw, idx);
        }
    }
    return 0;
}
