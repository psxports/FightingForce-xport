#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include "game.h"

ACTOR_ANIMATION actor_animations[ACTOR_COUNT];

uint16 animation_flags[ANIMATION_COUNT];
uint32 animation_fall_exit;
ANIMATION_BANK animation_banks[ANIMATION_BANK_COUNT];
ANIMATION_BANK_ID animation_type_banks[ANIMATION_TYPE_COUNT];

GDB_CALL ACTOR_ANIMATION_SOURCE animation_stream(ANIMATION_BANK_ID table, sint32 animation)
{
    FUNCTION_MARKER(0x8001a068, "GAME.EXE");
    /* TODO: Resolve a missing animation bank after the full CDB playthrough */
    XASSERT(table != ANIMATION_BANK_NONE);
    XASSERT(table < ANIMATION_BANK_COUNT && (uint32)animation < ANIMATION_COUNT);
    return animation_banks[table].stream[animation];
}

/* 8001AE7C: conditional vertical scaling for animation 269. Return remains
 * four times the old negative value on unsupported stages, without a store. */
GDB_CALL sint32 animation_scale_vertical_motion(sint32 idx)
{
    FUNCTION_MARKER(0x8001ae7c, "GAME.EXE");
    ACTOR_ANIMATION *playback = actor_animations + idx;
    uint32 stage, v, result;
    if (playback->id != 269)
        return 269;
    ACTOR_FRAME_MOTION *motion = &playback->motion;
    v = motion->bits[0];
    if ((sint32)v >= 0)
        return 269;
    stage = game_session.stage;
    if (!stage)
        result = v << 1;
    else if (stage == 15)
        result = (v + ((sint32)v >> 1)) << 1;
    else
    {
        result = v << 2;
        if (stage - 17u >= 2 && stage != 23)
            return result;
    }
    motion->bits[0] = result;
    return result;
}

/* Start a stream and publish initial playback state */
GDB_CALL sint32 animation_start(sint32 idx, ANIMATION_BANK_ID table, sint32 animation)
{
    FUNCTION_MARKER(0x80019F00, "GAME.EXE");
    ACTOR_ANIMATION *playback = actor_animations + idx;
    uint32 old = playback->id, flags;
    if (old == 281 && playback->frame != 0xffffffff)
        return -1;
    ACTOR_FRAME_MOTION *motion = &playback->motion;
    if ((old == 205 || old == 203) && (sint32)actor_positions[idx].y.bits < 0 && old != (uint32)animation)
        animation_fall_exit = 1;
    playback->id = animation;
    ACTOR_ANIMATION_SOURCE src = animation_stream(table, animation);
    playback->frame = 0;
    playback->hit_flags = 0;
    motion->forward_offset_bits = 0;
    motion->events = 0;
    playback->event_step = 0;
    playback->src = src;
    XASSERT((uint32)animation < ANIMATION_COUNT);
    if ((uint32)idx < 2)
    {
        flags = animation_flags[animation];
        if ((flags >> 3) & 15)
            action_history_rec(idx, (flags >> 3) & 15);
    }
    flags = animation_flags[animation];
    if (!(flags & 2) && actor_types[idx] != 17)
    {
        actor_pose_load_initial_motion(idx);
        return animation_scale_vertical_motion(idx);
    }
    return 17;
}

/* Toggle the retained actor selector before attempting animation startup */
GDB_CALL sint32 animation_start_toggle(sint32 idx, ANIMATION_BANK_ID table, sint32 animation)
{
    FUNCTION_MARKER(0x8001a03cu, "GAME.EXE");
    ACTOR_ANIMATION *playback = actor_animations + idx;
    playback->selector ^= 1u;
    return animation_start(idx, table, animation);
}

/* 8002371C: map animation variants to the state used by actor movement. */
GDB_CALL sint32 animation_movement_state(uint32 state)
{
    FUNCTION_MARKER(0x8002371c, "GAME.EXE");
    if (state == 198 || state == 70 || state - 71u < 2 || state == 73 || state == 76 || state == 78)
        state = 69;
    if (state - 96u < 2 || state - 93u < 2 || state == 95)
        state = 92;
    if (state - 83u < 2 || state - 85u < 2)
        state = 82;
    if (state - 88u < 2 || state == 90)
        state = 87;
    return state;
}

/* 8001AEF4: velocity comes from the old pose, even though decoding advances
 * the stream and replaces velocity fields before movement is applied. */
GDB_CALL sint32 animation_update_motion(sint32 idx)
{
    FUNCTION_MARKER(0x8001aef4, "GAME.EXE");
    uint32 stage = game_session.stage;
    sint32 side;
    uint32 forward;
    uint32 state;
    uint32 x;
    uint32 z;
    uint32 other_idx;
    sint32 prev_distance;
    sint32 limit;
    uint32 sn;
    uint32 cs;
    sint32 distance;
    sint32 yaw;
    uint32 dx, dz;
    ACTOR_ANIMATION *playback = actor_animations + idx;
    ACTOR_FRAME_MOTION *motion = &playback->motion;
    FIXED_VEC3 *pos = actor_positions + idx;
    if (stage == 4 || stage == 18)
    {
        limit = stage == 4 ? 47185920 : 78643200;
        if ((sint32)pos->x.bits < -limit)
            pos->x.bits = -limit;
        if ((sint32)pos->x.bits > limit)
            pos->x.bits = limit;
    }
    prev_distance = math_distance_newton(actor_positions[0].x.whole, actor_positions[0].z.whole, actor_positions[1].x.whole, actor_positions[1].z.whole);
    side = motion->bits[1];
    forward = motion->bits[2];
    actor_pose_decode(model_refs[(uint32)actor_types[idx]], idx, &actor_poses[idx]);
    state = animation_movement_state(playback->id);
    if (actor_contacts[idx].captor != -1 && state != 139 && state != 172 && state != 77)
        return 77;
    XASSERT(state < ANIMATION_COUNT);
    if (animation_flags[state] & 1)
    {
        motion->forward_offset_bits += forward;
        actor_correct_obstacles(idx, &pos->x.bits, &pos->z.bits);
        actor_col.pass_count = 0;
        return actors_resolve_overlaps(idx);
    }
    yaw = actor_orientation[idx].yaw;
    /* TODO: Investigate animation motion headings outside the sine table */
    XASSERT(yaw >= 0 && yaw < MATH_SIN_COUNT);
    sn = sin_table[yaw];
    /* TODO: Investigate animation motion headings outside the cosine view */
    XASSERT(yaw >= -MATH_QUARTER_TURN && yaw < MATH_ANGLE_COUNT);
    cs = sin_table[MATH_QUARTER_TURN + yaw];
    x = pos->x.bits + sn * ((sint32)forward >> 15) + cs * (side >> 15);
    z = pos->z.bits - cs * ((sint32)forward >> 15) + sn * (side >> 15);
    animation_scale_vertical_motion(idx);
    if (playback->id == 65 && actors_update_pair_motion(idx))
    {
        x = pos->x.bits;
        z = pos->z.bits;
    }
    if (!actor_lifecycle[idx].delay && actor_types[idx] != 51 && playback->id != 130)
        actor_correct_obstacles(idx, &x, &z);
    if ((sint32)game_session.player_count >= 2 && idx < 2)
    {
        other_idx = 1u - idx;
        distance = math_distance_newton((sint32)x >> 16, (sint32)z >> 16, actor_positions[other_idx].x.whole, actor_positions[other_idx].z.whole);
        dx = x - actor_positions[other_idx].x.bits;
        dz = z - actor_positions[other_idx].z.bits;
        if (distance < prev_distance || actor_health[other_idx].hp <= 0)
        {
            dx = 0;
            dz = 0;
        }
        if (dx + 0x3ffffffu > 0x7fffffeu || dz + 0x3ffffffu > 0x7fffffeu)
            goto movement_done;
    }
    pos->x.bits = x;
    pos->z.bits = z;
movement_done:
    if (actor_contacts[idx].captor != -1)
        return -1;
    actor_col.pass_count = 0;
    return actors_resolve_overlaps(idx);
}

/* Compare actual stream locations without merging distinct original address-space views */
GDB_CALL uint32 animation_matches_stream(sint32 idx, uint32 action)
{
    FUNCTION_MARKER(0x8004c464, "GAME.EXE");
    sint32 type = actor_types[idx];
    uint32 cur_id = actor_animations[idx].id;
    ANIMATION_BANK_ID table = animation_type_banks[type];
    ACTOR_ANIMATION_SOURCE cur = animation_stream(table, cur_id);
    ACTOR_ANIMATION_SOURCE next = animation_stream(table, action);
    if (cur.kind != next.kind)
        return 0;
    if (cur.kind != ACTOR_ANIMATION_SOURCE_STREAM)
        return 1;
    if (cur.stream.domain != ANIMATION_STREAM_NATIVE && next.stream.domain != ANIMATION_STREAM_NATIVE && cur.stream.domain != next.stream.domain)
        return 0;
    return cur.stream.header == next.stream.header;
}
