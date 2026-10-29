#include <stdio.h>

#include "game.h"

ITEM_PICKUP item_pickup;

/* Select the first eligible slot in original pool order */
uint32 item_pickup_select(sint32 idx, sint32 radius)
{
    FUNCTION_MARKER(0x80017fe0, "GAME.EXE");
    uint32 type, i;
    /* TODO: Investigate original pickup actor inputs outside the native pool */
    XASSERT(idx >= 0 && idx < ACTOR_COUNT);
    const FIXED_VEC3 *actor_pos = actor_positions + idx;
    sint32 actor_type = actor_types[idx];
    uint32 actor_x = actor_pos->x.bits;
    sint32 x = (sint32)actor_x >> 16;
    uint32 actor_z = actor_pos->z.bits;
    sint32 z = (sint32)actor_z >> 16;
    uint32 left = actor_items[idx].slot[ACTOR_ITEM_LEFT];
    sint32 right = actor_items[idx].slot[ACTOR_ITEM_RIGHT];
    sint32 yaw = actor_orientation[idx].yaw;
    /* TODO: Investigate original pickup sine reads beyond the native table */
    XASSERT(yaw >= 0 && yaw < MATH_SIN_COUNT);
    sint32 fx = (sint32)(actor_x + 192u * (sin_table[yaw])) >> 16;
    /* TODO: Investigate original pickup cosine reads beyond the native table */
    XASSERT(yaw >= 0 && yaw < MATH_SIN_COUNT - MATH_QUARTER_TURN);
    sint32 fz = (sint32)(actor_z - 192u * (sin_table[MATH_QUARTER_TURN + yaw])) >> 16;
    uint32 sel = (uint32)(-1);
    sint32 limit;
    for (i = 0; i < DYNAMIC_OBJECT_COUNT; i++)
    {
        if (i == left || i == (uint32)right)
            continue;
        if (game_session.player_count == 2)
        {
            /* TODO: Investigate pickup calls outside the two-player slot domain */
            XASSERT(idx < ITEM_PICKUP_PLAYER_COUNT);
            if (i == (uint32)(sint32)actor_items[1 - idx].slot[ACTOR_ITEM_LEFT] || i == (uint32)(sint32)actor_items[1 - idx].slot[ACTOR_ITEM_RIGHT] || i == item_pickup.nearby[1 - idx])
                continue;
        }
        const MODEL_OBJECT *item = dynamic_objects + i;
        type = item->kind;
        if (type - 587u < 10u && math_distance_approx(x, z, item->pos.x.whole, item->pos.z.whole) < radius)
        {
            sel = i;
            break;
        }
        if (actor_items[idx].bits != 0xffff ||
            (item->payload_kind == MODEL_PAYLOAD_UNMIGRATED      ? item->pending_motion.flags
             : item->payload_kind == MODEL_PAYLOAD_TRAIN         ? item->train.flags
             : item->payload_kind == MODEL_PAYLOAD_SUSPENDED     ? item->suspended.flags
             : item->payload_kind == MODEL_PAYLOAD_WAVE          ? item->wave.flags
             : item->payload_kind == MODEL_PAYLOAD_PANEL         ? item->panel.retire_ticks
             : item->payload_kind == MODEL_PAYLOAD_SIDE_PANEL    ? item->panel.roll_step
             : item->payload_kind == MODEL_PAYLOAD_SPRITE        ? item->spr.frame
             : item->payload_kind == MODEL_PAYLOAD_BALLISTIC     ? item->ballistic.pitch_step
             : item->payload_kind == MODEL_PAYLOAD_DRIFT         ? item->drift.flags
             : item->payload_kind == MODEL_PAYLOAD_SHAKE         ? item->shake.ticks
             : item->payload_kind == MODEL_PAYLOAD_GATE_FALL     ? item->gate_fall.pitch_speed
             : item->payload_kind == MODEL_PAYLOAD_GATE_FRAGMENT ? item->gate_fragment.pitch_speed
             : item->payload_kind == MODEL_PAYLOAD_ROTATION      ? item->rotation.phase
             : item->payload_kind == MODEL_PAYLOAD_BOUNCING_DROP ? item->drop.flags
             : item->payload_kind == MODEL_PAYLOAD_BOUNCING_PART ? item->bounce.bounces
             : item->payload_kind == MODEL_PAYLOAD_TRAFFIC       ? item->traffic.flags
             : item->payload_kind == MODEL_PAYLOAD_TANK          ? item->tank.flags
             : item->payload_kind == MODEL_PAYLOAD_OSCILLATION   ? item->oscillation.roll_speed
             : item->payload_kind == MODEL_PAYLOAD_COMPANION     ? item->companion.flags
             : item->payload_kind == MODEL_PAYLOAD_LAB_TANK      ? item->lab_tank.flags
             : item->payload_kind == MODEL_PAYLOAD_WOBBLE        ? item->wobble.ticks
             : item->payload_kind == MODEL_PAYLOAD_PHYSICS       ? item->physics.life
             : item->payload_kind == MODEL_PAYLOAD_ARMY_TRUCK    ? item->truck.flags
                                                                 : (item && item->payload_kind == MODEL_PAYLOAD_AIR_BIKE ? item->air_bike.target[0].x : (item && item->payload_kind == MODEL_PAYLOAD_SCENE_VEHICLE ? item->scene_vehicle.flags : (item && item->payload_kind == MODEL_PAYLOAD_SCENE_HELICOPTER ? item->scene_helicopter.flags : (item && item->payload_kind == MODEL_PAYLOAD_COMBAT_HELICOPTER ? item->helicopter.bob_phase : item->patrol.voice_plus_one))))) < 0 ||
            right != -1 || item->pos.y.bits || type - 477u >= 86u)
            continue;
        if ((sint32)type < 521)
        {
            if (math_distance_approx(x, z, item->pos.x.whole, item->pos.z.whole) < radius)
            {
                sel = i;
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
            if (math_distance_approx(fx, fz, item->pos.x.whole, item->pos.z.whole) < limit)
            {
                sel = i;
                break;
            }
        }
    }
    /* TODO: Investigate original pickup selection writes beyond the player array */
    XASSERT(idx < ITEM_PICKUP_PLAYER_COUNT);
    item_pickup.nearby[idx] = sel;
    return 4u * idx;
}

void item_pickup_draw(MODEL_OBJECT *item)
{
    FUNCTION_MARKER(0x8005d820, "GAME.EXE");
    /* TODO: Investigate an absent native dynamic draw receiver */
    XASSERT(item);
    uint32 type = item->kind, slot, sel;
    sint32 flags, timer;
    transform_push();
    object_transform_camera(item, NULL);
    /* TODO: Investigate a dynamic draw without its original native payload */
    XASSERT((type != 302 || item->payload_kind == MODEL_PAYLOAD_TRAIN) && (type != 896 || item->payload_kind == MODEL_PAYLOAD_SUSPENDED) && (type != 125 && type != 126 || item->payload_kind == MODEL_PAYLOAD_PANEL) && (type - 129u >= 3u && type - 350u >= 9u && type - 758u >= 12u || item->payload_kind == MODEL_PAYLOAD_SIDE_PANEL) && (type - 575u >= 2u && type - 862u >= 5u && type != 869 && type - 881u >= 5u || item->payload_kind == MODEL_PAYLOAD_SPRITE) && (type - 679u >= 4u || item->payload_kind == MODEL_PAYLOAD_BALLISTIC) && (type - 675u >= 3u || item->payload_kind == MODEL_PAYLOAD_DRIFT) && (type != 70 && type != 71 && type - 74u >= 3u && type != 124 && type - 127u >= 2u && type - 348u >= 2u || item->payload_kind == MODEL_PAYLOAD_SHAKE) && (type != 77 || item->payload_kind == MODEL_PAYLOAD_GATE_FALL) && (type - 72u >= 2u || item->payload_kind == MODEL_PAYLOAD_GATE_FRAGMENT) && (type != 696 || item->payload_kind == MODEL_PAYLOAD_ROTATION) &&
            (type - 473u >= 3u || item->payload_kind == MODEL_PAYLOAD_BOUNCING_DROP) && (type != 67 && type - 85u >= 4u && type != 92 && type - 99u >= 2u || item->payload_kind == MODEL_PAYLOAD_BOUNCING_PART) && (type - 78u >= 7u || item->payload_kind == MODEL_PAYLOAD_TRAFFIC) && (type != 776 || item->payload_kind == MODEL_PAYLOAD_TANK) && (type != 17 && type - 62u >= 3u && type != 66 && type != 90 && type - 94u >= 3u && type - 108u >= 3u && type - 720u >= 3u && type != 739 && type != 731 && type != 204 && type != 693 && type != 405 && type != 137 || item->payload_kind == MODEL_PAYLOAD_OSCILLATION) && (type != 65 || item->payload_kind == MODEL_PAYLOAD_COMPANION) && (type != 433 || item->payload_kind == MODEL_PAYLOAD_LAB_TANK) &&
            (type - 295u >= 7u && type != 326 && type != 367 && type - 371u >= 12u && type - 409u >= 24u && type - 608u >= 6u && type - 642u >= 5u && type != 649 && type - 655u >= 2u && type - 672u >= 3u && type - 718u >= 2u && type != 737 && type != 748 && type - 786u >= 12u && type - 804u >= 8u || item->payload_kind == MODEL_PAYLOAD_WOBBLE) && (type - 587u >= 10u && type != 69 && type != 105 && type != 724 && type != 448 && type != 717 && type - 269u >= 2u && type != 392 && type != 359 && type != 695 && type - 708u >= 3u && type != 408 && type - 572u >= 3u && type - DYNAMIC_ITEM_TYPE_FIRST >= DYNAMIC_ITEM_TYPE_COUNT && type != 713 && type != 715 && type - 735u >= 2u && type - 784u >= 2u && type - 828u >= 4u || item->payload_kind == MODEL_PAYLOAD_PHYSICS) && (type - 781u >= 2u || item->payload_kind == MODEL_PAYLOAD_ARMY_TRUCK) && (type - 460u >= 4u || item->payload_kind == MODEL_PAYLOAD_AIR_BIKE) &&
            (type != 160 && type != 166 && type != 168 && type != 740 && type != 749 && type - 175u >= 8u && type != 754 || item->payload_kind == MODEL_PAYLOAD_SCENE_VEHICLE) && (type != 341 && type != 146 || item->payload_kind == MODEL_PAYLOAD_SCENE_HELICOPTER) && (type != 687 || item->payload_kind == MODEL_PAYLOAD_COMBAT_HELICOPTER) && (type != 268 || item->payload_kind == MODEL_PAYLOAD_PATROL_HELICOPTER));
    /* TODO: Investigate dynamic draw payload tags outside the native owner catalog */
    XASSERT(item->payload_kind <= MODEL_PAYLOAD_SUSPENDED);
    /* Keep the signed flag word of each original payload variant */
    if (item->payload_kind == MODEL_PAYLOAD_UNMIGRATED)
        flags = item->pending_motion.flags;
    else if (item->payload_kind == MODEL_PAYLOAD_TRAIN)
        flags = item->train.flags;
    else if (item->payload_kind == MODEL_PAYLOAD_SUSPENDED)
        flags = item->suspended.flags;
    else if (item->payload_kind == MODEL_PAYLOAD_WAVE)
        flags = item->wave.flags;
    else if (item->payload_kind == MODEL_PAYLOAD_PANEL)
        flags = item->panel.retire_ticks;
    else if (item->payload_kind == MODEL_PAYLOAD_SIDE_PANEL)
        flags = item->panel.roll_step;
    else if (item->payload_kind == MODEL_PAYLOAD_SPRITE)
        flags = item->spr.frame;
    else if (item->payload_kind == MODEL_PAYLOAD_BALLISTIC)
        flags = item->ballistic.pitch_step;
    else if (item->payload_kind == MODEL_PAYLOAD_DRIFT)
        flags = item->drift.flags;
    else if (item->payload_kind == MODEL_PAYLOAD_SHAKE)
        flags = item->shake.ticks;
    else if (item->payload_kind == MODEL_PAYLOAD_GATE_FALL)
        flags = item->gate_fall.pitch_speed;
    else if (item->payload_kind == MODEL_PAYLOAD_GATE_FRAGMENT)
        flags = item->gate_fragment.pitch_speed;
    else if (item->payload_kind == MODEL_PAYLOAD_ROTATION)
        flags = item->rotation.phase;
    else if (item->payload_kind == MODEL_PAYLOAD_BOUNCING_DROP)
        flags = item->drop.flags;
    else if (item->payload_kind == MODEL_PAYLOAD_BOUNCING_PART)
        flags = item->bounce.bounces;
    else if (item->payload_kind == MODEL_PAYLOAD_TRAFFIC)
        flags = item->traffic.flags;
    else if (item->payload_kind == MODEL_PAYLOAD_TANK)
        flags = item->tank.flags;
    else if (item->payload_kind == MODEL_PAYLOAD_OSCILLATION)
        flags = item->oscillation.roll_speed;
    else if (item->payload_kind == MODEL_PAYLOAD_COMPANION)
        flags = item->companion.flags;
    else if (item->payload_kind == MODEL_PAYLOAD_LAB_TANK)
        flags = item->lab_tank.flags;
    else if (item->payload_kind == MODEL_PAYLOAD_WOBBLE)
        flags = item->wobble.ticks;
    else if (item->payload_kind == MODEL_PAYLOAD_PHYSICS)
        flags = item->physics.life;
    else if (item->payload_kind == MODEL_PAYLOAD_ARMY_TRUCK)
        flags = item->truck.flags;
    else
        flags = item && item->payload_kind == MODEL_PAYLOAD_AIR_BIKE ? item->air_bike.target[0].x : (item && item->payload_kind == MODEL_PAYLOAD_SCENE_VEHICLE ? item->scene_vehicle.flags : (item && item->payload_kind == MODEL_PAYLOAD_SCENE_HELICOPTER ? item->scene_helicopter.flags : (item && item->payload_kind == MODEL_PAYLOAD_COMBAT_HELICOPTER ? item->helicopter.bob_phase : item->patrol.voice_plus_one)));
    if (flags < 0 && (flags & 2))
    {
        transform_pop();
        return;
    }
    if (type - 496u < 2u && (item->physics.life) > 0)
        item_draw_explosive_countdown(item);
    if (!item->pos.y.bits && !(item->payload_kind == MODEL_PAYLOAD_UNMIGRATED                                                ? item->pending_motion.speed
                               : item->payload_kind == MODEL_PAYLOAD_TRAIN                                                   ? item->train.speed
                               : item->payload_kind == MODEL_PAYLOAD_SUSPENDED                                               ? item->suspended.actor_idx
                               : item->payload_kind == MODEL_PAYLOAD_WAVE                                                    ? item->wave.speed
                               : item->payload_kind == MODEL_PAYLOAD_SPRITE                                                  ? item->spr.speed
                               : item->payload_kind == MODEL_PAYLOAD_PANEL || item->payload_kind == MODEL_PAYLOAD_SIDE_PANEL ? item->panel.speed
                               : item->payload_kind == MODEL_PAYLOAD_BALLISTIC                                               ? item->ballistic.speed
                               : item->payload_kind == MODEL_PAYLOAD_DRIFT                                                   ? item->drift.speed
                               : item->payload_kind == MODEL_PAYLOAD_SHAKE                                                   ? item->shake.speed
                               : item->payload_kind == MODEL_PAYLOAD_GATE_FALL                                               ? item->gate_fall.speed
                               : item->payload_kind == MODEL_PAYLOAD_GATE_FRAGMENT                                           ? item->gate_fragment.speed
                               : item->payload_kind == MODEL_PAYLOAD_ROTATION                                                ? item->rotation.amplitude
                               : item->payload_kind == MODEL_PAYLOAD_BOUNCING_DROP                                           ? item->drop.speed
                               : item->payload_kind == MODEL_PAYLOAD_BOUNCING_PART                                           ? item->bounce.speed
                               : item->payload_kind == MODEL_PAYLOAD_TRAFFIC                                                 ? item->traffic.speed
                               : item->payload_kind == MODEL_PAYLOAD_TANK                                                    ? item->tank.speed
                               : item->payload_kind == MODEL_PAYLOAD_OSCILLATION                                             ? item->oscillation.speed
                               : item->payload_kind == MODEL_PAYLOAD_COMPANION                                               ? item->companion.speed
                               : item->payload_kind == MODEL_PAYLOAD_LAB_TANK                                                ? item->lab_tank.speed
                               : item->payload_kind == MODEL_PAYLOAD_WOBBLE                                                  ? item->wobble.speed
                               : item->payload_kind == MODEL_PAYLOAD_PHYSICS                                                 ? item->physics.speed
                               : item->payload_kind == MODEL_PAYLOAD_ARMY_TRUCK                                              ? item->truck.speed
                                                                                                                             : (item && item->payload_kind == MODEL_PAYLOAD_AIR_BIKE ? item->air_bike.target[0].z : (item && item->payload_kind == MODEL_PAYLOAD_SCENE_VEHICLE ? item->scene_vehicle.speed : (item && item->payload_kind == MODEL_PAYLOAD_SCENE_HELICOPTER ? item->scene_helicopter.speed : (item && item->payload_kind == MODEL_PAYLOAD_COMBAT_HELICOPTER ? item->helicopter.shot_distance : item->patrol.speed))))))
    {
        if (type == 477)
            transform_translate(0, 0, -3670016);
        if (type == 490 || type == 683)
            transform_translate(0, 0, -2621440);
        if (type == 508)
            transform_translate(0, 2621440, 0);
        if (type == 515)
            transform_translate(0, 2621440, -2097152);
        slot = item_pickup.nearby[0];
        /* TODO: Investigate pickup selections outside the native dynamic pool */
        XASSERT(slot == 0xffffffffu || slot < DYNAMIC_OBJECT_COUNT);
        sel = slot != 0xffffffffu && dynamic_objects + slot == item;
        if (!sel)
        {
            slot = item_pickup.nearby[1];
            /* TODO: Investigate pickup selections outside the native dynamic pool */
            XASSERT(slot == 0xffffffffu || slot < DYNAMIC_OBJECT_COUNT);
            sel = slot != 0xffffffffu && dynamic_objects + slot == item;
        }
        if (sel && !(sint8)game_pause_flags && !(hud.tick & 31))
            render_style.color.rgbc = 0x2cffffff;
        model_draw_static(model_refs[type], 0, 0);
    }
    else
    {
        model_draw_static(model_refs[type], 0, 0);
        /* TODO: Investigate a timed attachment draw with a foreign native owner */
        XASSERT(type != 683 || item->payload_kind == MODEL_PAYLOAD_UNMIGRATED);
        if (type == 683 && (timer = item->pending_motion.render_ticks) > 0)
        {
            item->pending_motion.render_ticks = (uint16)(timer - 1);
            transform_roll(8192);
            transform_translate(0, 0, 9437184);
            model_draw_static(model_refs[685u + (item->pending_motion.render_ticks)], 0, 0);
        }
    }
    transform_pop();
}

const OBJECT_CALLBACK callback_default_model_draw = {OBJECT_CALL_VOID_MUTABLE_MODEL, {.no_result_mutable_model = item_pickup_draw}, 0};

const OBJECT_CALLBACK callback_model_draw = {OBJECT_CALL_VOID_MUTABLE_MODEL, {.no_result_mutable_model = item_pickup_draw}, 0};

uint32 item_draw_explosive_countdown(const MODEL_OBJECT *item)
{
    FUNCTION_MARKER(0x8002bdd0u, "GAME.EXE");
    PROJECTION_POINT pos;
    /* TODO: Investigate explosive callbacks without a compatible native owner */
    XASSERT(item && item->payload_kind == MODEL_PAYLOAD_PHYSICS);
    sint32 timer = item->physics.life;
    sint32 grp = (timer + 29) / 30;
    /* Groups 2 and above take the unconditional path */
    if (grp < 2 && (grp != 1 || !(game_session.tick & 2)))
        return 2;
    proj_local(0, 0, 0, &pos);
    if (pos.depth >= 65)
        pos.depth = pos.depth - 64u;
    if (pos.status)
        return (uint32)grp << 1;
    return spr_draw(&pos, tex_map[(121u + (grp))]);
}

void item_draw_explosive(const MODEL_OBJECT *item)
{
    FUNCTION_MARKER(0x8005dad4u, "GAME.EXE");
    PROJECTION_POINT pos;
    /* TODO: Investigate a dynamic callback outside its native pool */
    XASSERT(item);
    transform_push();
    object_transform_camera(item, NULL);
    /* TODO: Investigate explosive callbacks without a compatible native owner */
    XASSERT(item && item->payload_kind == MODEL_PAYLOAD_PHYSICS);
    model_draw_static(model_refs[item->kind], 0, 0);
    if ((item->physics.life) > 0)
    {
        proj_local(0, 64, 0, &pos);
        if (!pos.status)
        {
            pos.size = game_session.tick & 1u ? 448 : 256;
            render_style.color.rgbc = 0x2cffffffu;
            spr_draw_scaled(&pos, tex_map[66]);
        }
        if ((item->physics.life) > 0)
            item_draw_explosive_countdown(item);
    }
    transform_pop();
}

sint32 item_pickup_find_for_npc(sint32 x, sint32 z)
{
    FUNCTION_MARKER(0x80055b68u, "GAME.EXE");
    uint32 i;
    sint32 kind;
    x >>= 16;
    z >>= 16;
    for (i = 0; i < DYNAMIC_OBJECT_COUNT; i++)
    {
        const MODEL_OBJECT *model = dynamic_objects + i;
        kind = model->kind;
        if (kind < 522 || kind >= 563)
            continue;
        /* TODO: Investigate an NPC pickup candidate with a foreign native owner */
        XASSERT(model->payload_kind == MODEL_PAYLOAD_PHYSICS);
        if (!model->physics.speed && model->pos.y.bits != 1 && math_distance_approx(x, z, model->pos.x.whole, model->pos.z.whole) < 64)
            return i;
    }
    return -1;
}
