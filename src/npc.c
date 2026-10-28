#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "game.h"

const NPC_ATTACK_GEOMETRY npc_attack_geometry_defaults = {
    .origin = {.x = {.val = -1179648}, .y = {.val = 1114112}, .z = {.val = -2424832}},
    .offset = {.x = {.val = -524288}, .y = {.val = 5832704}, .z = {.val = -786432}},
};

NPC_ATTACK_GEOMETRY npc_attack_geometry;
const sint32 npc_fall_speed_limit_default = 64;
sint32 npc_fall_speed_limit;
uint16 npc_drop_phase;
uint32 npc_update_idx;

static const sint8 drop_pattern[8] = {0, 0, 1, 0, 0, 1, 0, 1};

enum
{
    NPC_PROFILE_NONE = 0xffff
};

static const uint16 profile_first[NPC_BEHAVIOR_TYPE_COUNT] = {
    NPC_PROFILE_NONE, NPC_PROFILE_NONE, NPC_PROFILE_NONE, NPC_PROFILE_NONE, NPC_PROFILE_NONE, 0, 10, 20, 5, 15, 25, 40, 30, 35, 105, 110, 115, 0, 45, 50, 55, 60, 65, 70, 75, 80, 85, 90, 0, 95, 100, 120, 125, 130, 135, 140, 145, 150, 155, 160, 165, 170, 175, 180, 185, 190, 195, 200, 205, 210, 215, 0, 220, 225,
};

enum
{
    NPC_BEAM_COUNT = 3
};

static const FIXED_VEC3 beam_points[NPC_BEAM_COUNT][2] = {
    {
        {.x = {.val = 0}, .y = {.val = -6291456}, .z = {.val = 917504}},
        {.x = {.val = 0}, .y = {.val = -12582912}, .z = {.val = 917504}},
    },
    {
        {.x = {.val = -851968}, .y = {.val = -6291456}, .z = {.val = -458752}},
        {.x = {.val = -851968}, .y = {.val = -12582912}, .z = {.val = -458752}},
    },
    {
        {.x = {.val = 851968}, .y = {.val = -6291456}, .z = {.val = -458752}},
        {.x = {.val = 851968}, .y = {.val = -12582912}, .z = {.val = -458752}},
    },
};

typedef struct NPC_BASE_ANIMATION
{
    uint32 idle;
    uint32 move;
} NPC_BASE_ANIMATION;

static const NPC_BASE_ANIMATION base_animations[NPC_BEHAVIOR_TYPE_COUNT] = {
    {69, 82}, {69, 82}, {69, 82}, {69, 82}, {69, 82}, {69, 82}, {69, 82}, {69, 82}, {69, 82}, {69, 82}, {69, 82}, {69, 82}, {69, 82}, {69, 82}, {69, 82}, {69, 82}, {69, 82}, {69, 82}, {69, 82}, {69, 82}, {69, 82}, {69, 82}, {69, 82}, {211, 82}, {69, 82}, {69, 82}, {69, 82}, {69, 82}, {69, 82}, {69, 82}, {69, 82}, {69, 82}, {69, 82}, {69, 82}, {69, 82}, {69, 82}, {69, 82}, {69, 82}, {69, 82}, {69, 82}, {69, 82}, {69, 82}, {69, 82}, {69, 82}, {69, 82}, {69, 82}, {69, 82}, {69, 82}, {69, 82}, {69, 82}, {69, 82}, {277, 82}, {285, 82}, {69, 82},
};

static sint32 bike_partner_idx(sint32 idx)
{
    sint32 partner = actor_companions[idx];
    /* TODO: Investigate the original post-detach companion access */
    if (partner < 0 || partner >= ACTOR_COUNT)
    {
        DIAGNOSTIC(fprintf(stderr, "Invalid bike companion %d for actor slot %d\n", partner, idx));
        diagnostic_stop(__FUNCTION__, __FILE__, __LINE__);
    }
    return partner;
}

static void bike_partner_state(sint32 idx, uint32 state)
{
    sint32 partner_idx = bike_partner_idx(idx);
    animation_start(partner_idx, animation_type_banks[(uint32)actor_types[partner_idx]], state);
}

GDB_CALL intptr npc_update_bike(sint32 idx)
{
    FUNCTION_MARKER(0x80055c30u, "GAME.EXE");
    NPC_SPECIAL *special = &npc_behavior_actors[idx].special;
    uint32 table, angle, speed, pitch, target, state, partner, x, y, z, result;
    FIXED_VEC3 point;
    uint32 spatial_volume, pan, candidate_x, candidate_z, random_offset;
    sint32 volume, delta, yaw;
    int i;
    yaw = actor_orientation[idx].yaw;
    if (yaw < 0 || yaw >= MATH_SIN_COUNT)
    {
        /* TODO: Investigate bike headings outside the original sine table */
        DIAGNOSTIC(fprintf(stderr, "Invalid bike sine heading %d actor %d stage %u tick %u\n", yaw, idx, game_session.stage, game_session.tick));
        diagnostic_stop(__FUNCTION__, __FILE__, __LINE__);
    }
    x = (uint32)(sint32)sin_table[yaw];
    speed = special->bike.speed;
    target = (uint32)actor_types[idx];
    yaw = actor_orientation[idx].yaw;
    if (target >= ANIMATION_TYPE_COUNT)
    {
        /* TODO: Investigate a bike type outside the animation bank table */
        DIAGNOSTIC(fprintf(stderr, "Invalid bike type %d actor %d stage %u tick %u\n", (sint32)target, idx, game_session.stage, game_session.tick));
        diagnostic_stop(__FUNCTION__, __FILE__, __LINE__);
    }
    table = animation_type_banks[target];
    x = actor_positions[idx].x.bits + x * speed;
    actor_positions[idx].x.bits = x;
    if (yaw < -MATH_QUARTER_TURN || yaw >= MATH_ANGLE_COUNT)
    {
        /* TODO: Investigate bike headings outside the original cosine view */
        DIAGNOSTIC(fprintf(stderr, "Invalid bike cosine heading %d actor %d stage %u tick %u\n", yaw, idx, game_session.stage, game_session.tick));
        diagnostic_stop(__FUNCTION__, __FILE__, __LINE__);
    }
    z = (uint32)(sint32)sin_table[MATH_QUARTER_TURN + yaw];
    speed = special->bike.speed;
    pitch = special->bike.speed << 2;
    z = actor_positions[idx].z.bits - z * speed;
    actor_positions[idx].z.bits = z;
    random_offset = (random_next() & 15) ? 6144 : 4096;
    sound_voice_pitch(special->bike.voice[0] - 1, pitch + random_offset);
    sound_voice_pitch(special->bike.voice[1] - 1, pitch + 5120);
    sound_spatial(actor_positions + idx, &spatial_volume, &pan);
    volume = (sint32)(spatial_volume * (96u - (uint32)((sint32)special->bike.speed >> 1))) / 176;
    volume = sound_volume_adjust(volume, 60);
    sound_voice_pan(special->bike.voice[0] - 1, volume, pan);
    volume = (sint32)(spatial_volume * (special->bike.speed - 64u)) / 48;
    volume = sound_volume_adjust(volume, 0);
    sound_voice_pan(special->bike.voice[1] - 1, volume, pan);
    if (actor_companions[idx] != -1)
    {
        actors_find_nearest_targets(idx);
        target = actor_targeting.player_idx;
        x = (uint32)(sint32)actor_positions[idx].x.whole;
        z = (uint32)(sint32)actor_positions[idx].z.whole;
        npc_targets[idx].idx = target;
        if (target >= ACTOR_COUNT)
        {
            /* TODO: Investigate bike targeting without an available live player */
            DIAGNOSTIC(fprintf(stderr, "Invalid bike target %d actor %d player count %u stage %u tick %u\n", (sint32)target, idx, game_session.player_count, game_session.stage, game_session.tick));
            diagnostic_stop(__FUNCTION__, __FILE__, __LINE__);
        }
        candidate_x = (uint32)(sint32)actor_positions[target].x.whole;
        candidate_z = (uint32)(sint32)actor_positions[target].z.whole;
        angle = math_dir16(x, z, candidate_x, candidate_z);
        delta = (sint16)(angle - ((uint32)((uint16)actor_orientation[idx].yaw) << 5));
        state = actor_animations[idx].id;
        if (state == 278)
        {
            speed = special->bike.speed;
            if ((sint32)speed > 0)
                special->bike.speed = speed - 4;
            if (actor_animations[idx].frame == 0xffffffffu)
            {
                speed = special->bike.speed;
                angle = (uint16)actor_orientation[idx].yaw;
                special->bike.speed = 0u - speed;
                actor_orientation[idx].yaw = (uint16)angle ^ 1024;
                animation_start(idx, table, 277);
                sound_voice_stop(special->bike.voice[2] - 1);
                special->bike.voice[2] = 0;
                sound_voice_play(142, 159, 5888);
                bike_partner_state(idx, 279);
            }
        }
        else if (state == 277)
        {
            speed = special->bike.speed;
            if ((sint32)speed < 0)
                special->bike.speed = speed + 4;
            else if ((sint32)speed < 96)
                special->bike.speed = speed + 1;
            if ((uint16)(delta + 16384) <= 32768)
            {
                if (delta < -256)
                    delta = -256;
                if (delta > 256)
                    delta = 256;
                actor_orientation[idx].yaw = (uint16)((((uint16)actor_orientation[idx].yaw) + (delta >> 5)) & 2047);
            }
            else
            {
                animation_start(idx, table, 278);
                pitch = (random_next() & 255) + 5504;
                sound_voice_play(230, 95, pitch);
                pitch = (random_next() & 255) | 5888;
                sound_voice_play(230, 127, pitch);
                bike_partner_state(idx, 280);
            }
            if (actor_animations[idx].frame == 0xffffffffu)
                animation_start(idx, table, 277);
        }
        actor_attachment_pos(idx, 1, point.bits);
        x = point.x.bits;
        z = point.z.bits;
        candidate_x = x;
        candidate_z = z;
        actor_correct_obstacles(bike_partner_idx(idx), &candidate_x, &candidate_z);
        if (candidate_x == x && candidate_z == z)
        {
            actor_positions[bike_partner_idx(idx)].x.bits = x;
            actor_positions[bike_partner_idx(idx)].y.bits = 0x680000;
            actor_positions[bike_partner_idx(idx)].z.bits = candidate_z;
        }
        else
        {
            bike_partner_state(idx, 281);
            actor_companions[bike_partner_idx(idx)] = ACTOR_SLOT_NONE;
            actor_companions[idx] = ACTOR_SLOT_NONE;
        }
        state = actor_animations[idx].id;
        result = 277;
        /* The companion lookup diagnoses the original post-detach invalid index */
        if (state == 278 || state == 277)
        {
            partner = bike_partner_idx(idx);
            angle = (uint16)actor_orientation[idx].yaw;
            actor_orientation[partner].yaw = (uint16)angle ^ (state == 278 ? 1024 : 0);
            point.x.bits = 0;
            point.y.bits = state == 278 ? 0x400000 : 0x100000;
            point.z.bits = state == 278 ? 0xff780000 : 0x1000000;
            actor_transform_point(&actor_poses[idx].matrix[0], point.bits);
            if (state == 278)
                return (intptr)effect_spawn_timed(871, point.x.bits, point.y.bits, point.z.bits, 0, 0, 0);
            else
            {
                actors_apply_nearby_contact(idx, point.x.bits, point.z.bits);
                result = item_pickup_find_for_npc(point.x.bits, point.z.bits);
                if (result != 0xffffffffu)
                {
                    bike_partner_state(idx, 281);
                    partner = bike_partner_idx(idx);
                    result = 244u * partner;
                    actor_companions[partner] = ACTOR_SLOT_NONE;
                    actor_companions[idx] = ACTOR_SLOT_NONE;
                }
            }
        }
    }
    else
    {
        sound_voice_stop(special->bike.voice[0] - 1);
        sound_voice_stop(special->bike.voice[1] - 1);
        target = special->bike.voice[2];
        if (target)
            sound_voice_stop(target - 1);
        sound_voice_play(19, 255, 6144);
        x = actor_positions[idx].x.bits;
        y = actor_positions[idx].y.bits;
        z = actor_positions[idx].z.bits;
        angle = (uint16)actor_orientation[idx].yaw;
        effect_spawn(893, x, y, z);
        for (i = 0; i < 2; ++i)
        {
            random_offset = random_next() << 8;
            x = actor_positions[idx].x.bits + random_offset - 0x40000;
            random_offset = random_next() << 8;
            z = actor_positions[idx].z.bits;
            y = actor_positions[idx].y.bits;
            z += random_offset - 0x40000;
            effect_spawn(893, x, y, z);
        }
        angle = (uint32)(sint32)(sint16)(angle << 5);
        x = actor_positions[idx].x.bits;
        z = actor_positions[idx].z.bits;
        dynamic_objects_spawn_oriented(580, x, 0, z, angle);
        x = actor_positions[idx].x.bits;
        z = actor_positions[idx].z.bits;
        dynamic_objects_spawn_item(532, x, 0xffb00000, z, angle + 16384);
        x = actor_positions[idx].x.bits;
        z = actor_positions[idx].z.bits;
        dynamic_objects_spawn_randomized(520, x, 0xffb00000, z, angle + 32768);
        x = actor_positions[idx].x.bits;
        z = actor_positions[idx].z.bits;
        dynamic_objects_spawn_randomized(531, x, 0xffb00000, z, angle);
        x = actor_positions[idx].x.bits;
        z = actor_positions[idx].z.bits;
        dynamic_objects_spawn_randomized(531, x, 0xffb00000, z, angle + 32768);
        for (i = 0; i < 16; ++i)
        {
            target = (random_next() & 3) + 902;
            x = actor_positions[idx].x.bits;
            y = actor_positions[idx].y.bits;
            z = actor_positions[idx].z.bits;
            effect_spawn_debris(target, x, y, z);
        }
        disp_buffers.shake_ticks = 16;
        disp_buffers.flash_ticks = 4;
        actor_animations[idx].src = (ACTOR_ANIMATION_SOURCE){ACTOR_ANIMATION_SOURCE_NONE, {0}};
        actor_types[idx] = ACTOR_TYPE_NONE;
        result = 0xffffffffu;
    }
    return (sint32)result;
}

GDB_CALL sint32 npc_update_attack_motion(sint32 idx)
{
    FUNCTION_MARKER(0x8004d2d4u, "GAME.EXE");
    NPC_SPECIAL *special = &npc_behavior_actors[idx].special;
    uint32 table = animation_type_banks[(uint32)actor_types[idx]], state, next = 0, result;
    sint32 drive;
    if (npc_move_oscillating(idx) < 0)
        return npc_update_combat(idx);
    drive = (sint32)special->attack.phase;
    if (drive >= 257)
    {
        state = actor_animations[idx].id;
        special->attack.effect_height = 0x590000;
        if (state == 255 && actor_animations[idx].frame == 0xffffffff)
            npc_start_animation(idx, table, 254);
        state = actor_animations[idx].id; /* 4D38C reload after the first transition. */
        if (state == 254)
        {
            if (actor_animations[idx].frame == 0xffffffff)
                next = 254;
        }
        else if (state == 257 || state == 258)
        {
            if (actor_animations[idx].frame == 0xffffffff)
                next = 259;
        }
        else if (state == 256 || state == 259)
        {
            if (actor_animations[idx].frame == 0xffffffff)
                next = 255;
        }
        else if (state == 266)
            next = 255;
    }
    else if (drive < -320)
    {
        state = actor_animations[idx].id;
        special->attack.effect_height = 0x290000;
        if (state == 255 || state == 254)
        {
            if (actor_animations[idx].frame == 0xffffffff)
                next = 256;
        }
        else if (state == 258 || state == 257)
        {
            if (actor_animations[idx].frame == 0xffffffff)
                next = 257;
        }
        else if (state == 256 || state == 259)
        {
            if (actor_animations[idx].frame == 0xffffffff)
                next = 258;
        }
        else if (state == 266)
            next = 258;
    }
    else
    {
        state = actor_animations[idx].id;
        special->attack.effect_height = 0x90000;
        if (state == 254 || state == 255)
        {
            if (actor_animations[idx].frame != 0xffffffff)
                return -1;
            next = 256;
        }
        else if (state == 257 || state == 258)
        {
            if (actor_animations[idx].frame != 0xffffffff)
                return -1;
            next = 259;
        }
        else if (state == 256 || state == 259)
        {
            if (actor_animations[idx].frame != 0xffffffff)
                return -1;
            next = 266;
        }
        else if (state == 266)
        {
            sint32 deadline = ((sint32)special->attack.rest_until);
            sint32 tick = (sint32)game_session.tick;
            if (tick >= deadline)
                next = 268;
            else
            {
                if (actor_animations[idx].frame != 0xffffffff)
                    return -1;
                next = 266;
            }
        }
        else
            return 266;
        return npc_start_animation(idx, table, next);
    }
    if (next)
        npc_start_animation(idx, table, next);
    result = game_session.tick + 20;
    special->attack.rest_until = result;
    return result;
}

static intptr npc_action_call(NPC_ACTION_REFERENCE callback, sint32 idx);

GDB_CALL intptr npc_update_vulkan(sint32 idx)
{
    FUNCTION_MARKER(0x8004df2cu, "GAME.EXE");
    uint32 events, level, volume = 0, result, state, handle;
    sint32 timer, partner;
    int bit, change = 0;
    timer = actor_health[idx].hp;
    npc_update_idx = (uint32)idx;
    if (timer > 0)
        npc_attack_electric(idx);
    if (stage_audio_slots[0].voice == -1)
    {
        stage_audio_slots[0].voice = sound_voice_play(181, 0, 5632);
        stage_audio_slots[2].voice = sound_voice_play(184, 255, 6144);
        stage_audio_slots[3].voice = sound_voice_play(188, 0, 6400);
    }
    state = actor_animations[idx].id;
    level = stage_audio_slots[1].level;
    if (state - 270u < 2u)
    {
        if ((sint32)level < 248)
        {
            level += 8;
            change = 1;
        }
    }
    else if ((sint32)level > 0)
    {
        level -= 8;
        change = 1;
    }
    if (change)
    {
        handle = stage_audio_slots[0].voice;
        stage_audio_slots[1].level = level;
        sound_voice_pitch(handle, (level << 2) + 5120);
        handle = stage_audio_slots[0].voice;
        level = stage_audio_slots[1].level;
        sound_voice_volume(handle, level);
    }
    if (actor_status[idx].flags & ACTOR_ELECTRIC)
    {
        actors_emit_arc(idx);
        volume = 255;
        actor_status[idx].flags = actor_status[idx].flags & ~ACTOR_ELECTRIC;
    }
    sound_voice_volume(stage_audio_slots[2].voice, volume);
    timer = (sint8)actor_lifecycle[idx].death_ticks;
    if (timer)
    {
        timer = (sint8)(timer + 1);
        actor_lifecycle[idx].death_ticks = (uint8)timer;
        if (timer >= 61)
        {
            actor_animations[idx].src = (ACTOR_ANIMATION_SOURCE){ACTOR_ANIMATION_SOURCE_NONE, {0}};
            actor_types[idx] = ACTOR_TYPE_NONE;
        }
        return -1;
    }
    if (actor_health[idx].hp <= 0)
    {
        partner = actor_contacts[idx].captive;
        if (partner != -1)
        {
            actor_contacts[(uint32)partner].captor = ACTOR_SLOT_NONE;
            actor_contacts[idx].captive = ACTOR_SLOT_NONE;
        }
        return actors_rec_death(idx);
    }
    if (actor_lifecycle[idx].delay > 0)
        return npc_update_delay(idx);
    events = actor_animations[idx].motion.events;
    if (events & 0x800)
        actor_animations[idx].hit_flags = 0;
    for (bit = 0; bit < 9; ++bit)
        if (events & (1u << bit))
            if (npc_attack_players(idx, bit + 1))
                sound_play_strike_voice(idx);
    if ((events & 0x400) && npc_process_weapon_hits(idx))
    {
        result = (random_next() & 1) + 33;
        sound_voice_play(result, 255, 6144);
    }
    if (events & 0x1000)
    {
        uint32 event_state = actor_animations[idx].id & 0x3fffffff;
        if (event_state >= ACTOR_EVENT_SLOT_COUNT)
            diagnostic_stop(__FUNCTION__, __FILE__, __LINE__);
        actor_event_call(actor_events[event_state], idx);
        actor_animations[idx].event_step = (uint16)(actor_animations[idx].event_step + 1);
    }
    result = (uint32)npc_react_to_target(idx);
    if (result)
        return (sint32)result;
    state = actor_animations[idx].id;
    if ((uint32)idx >= NPC_BEHAVIOR_ACTOR_COUNT)
        diagnostic_stop(__FUNCTION__, __FILE__, __LINE__);
    NPC_ACTION_BANK_REFERENCE bank = npc_action_actors[idx];
    uint32 action_idx = (bank.first + state) & 0x3fffffff;
    if (!bank.actions || action_idx >= NPC_ACTION_SLOT_COUNT)
        diagnostic_stop(__FUNCTION__, __FILE__, __LINE__);
    return npc_action_call(bank.actions[action_idx], idx);
}

GDB_CALL sint32 npc_attack_beams(sint32 idx)
{
    FUNCTION_MARKER(0x8004e7e8u, "GAME.EXE");
    static const uint32 styles[NPC_BEAM_COUNT] = {108, 106, 107};
    NPC_BEHAVIOR_ACTOR *behavior = npc_behavior_actors + idx;
    sint32 type = actor_types[idx];
    uint32 flags = actor_animations[idx].motion.events;
    uint32 table = animation_type_banks[(uint32)type];
    FIXED_VEC3 points[2];
    uint32 target, x, z, delta, result, state = 272;
    const POSE_MATRIX *matrix;
    int i;
    for (i = 0; i < NPC_BEAM_COUNT; ++i)
    {
        if (i)
            flags = actor_animations[idx].motion.events;
        if (flags & (0x2000 << i))
        {
            if (!(actor_status[idx].flags & ACTOR_CULLED))
            {
                points[0] = beam_points[i][0];
                points[1] = beam_points[i][1];
                matrix = &actor_poses[idx].matrix[2];
                actor_transform_point(matrix, points[0].bits);
                actor_transform_point(matrix, points[1].bits);
                effect_spawn_line(styles[i], points[1].x.bits, points[1].y.bits, points[1].z.bits, points[0].x.bits, points[0].y.bits, points[0].z.bits, 0x01000080, 0x00c0c0c0);
            }
            sound_voice_play(24, 128, 6144);
            result = (uint32)npc_fire_fan(idx);
            behavior->progress.projectile_hits += result;
        }
    }
    if ((sint32)behavior->progress.projectile_hits >= 6)
        behavior->progress.projectile_hits = 0;
    else
    {
        if (actor_animations[idx].frame != 0xffffffff)
            return -1;
        random_next(); /* Advance the original unused RNG draw */
        if (npc_targets[idx].role == 1)
        {
            target = npc_targets[idx].idx;
            if (target != 0xffffffff)
            {
                x = actor_positions[target].x.bits;
                z = actor_positions[target].z.bits;
                delta = (uint32)npc_turn_to_point(idx, x, z);
                if ((uint16)(delta + 2047) < 4095)
                    state = 271;
            }
        }
    }
    return npc_start_animation(idx, table, state);
}

GDB_CALL uint32 npc_build_attack_vectors(sint32 idx, uint32 scale, uint32 output[16])
{
    FUNCTION_MARKER(0x8004d64cu, "GAME.EXE");
    int i;
    if (scale)
    {
        npc_attack_geometry.offset.x.bits = 0u - 8u * scale;
        npc_attack_geometry.offset.y.bits = 89u * scale;
        npc_attack_geometry.offset.z.bits = 0u - 12u * scale;
    }
    else
    {
        npc_attack_geometry.offset.x.bits = 0xfff80000;
        npc_attack_geometry.offset.z.bits = 0xfff40000;
        npc_attack_geometry.offset.y.bits = npc_behavior_actors[idx].special.attack.effect_height;
    }
    transform_push();
    transform_load(&actor_poses[idx].matrix[2]);
    transform_point(npc_attack_geometry.origin.x.bits, npc_attack_geometry.origin.y.bits, npc_attack_geometry.origin.z.bits, output);
    transform_point(npc_attack_geometry.origin.x.bits + npc_attack_geometry.offset.x.bits, npc_attack_geometry.origin.y.bits + npc_attack_geometry.offset.y.bits, npc_attack_geometry.origin.z.bits + npc_attack_geometry.offset.z.bits, output + 4);
    transform_point(0u - npc_attack_geometry.origin.x.bits, npc_attack_geometry.origin.y.bits, npc_attack_geometry.origin.z.bits, output + 8);
    transform_point(0u - (npc_attack_geometry.origin.x.bits + npc_attack_geometry.offset.x.bits), npc_attack_geometry.origin.y.bits + npc_attack_geometry.offset.y.bits, npc_attack_geometry.origin.z.bits + npc_attack_geometry.offset.z.bits, output + 12);
    transform_identity();
    transform_yaw(-camera.angles.yaw);
    transform_pitch(-camera.angles.pitch);
    for (i = 0; i < 16; i += 4)
        transform_point(output[i], output[i + 1], output[i + 2], output + i);
    transform_pop();
    output[4] -= output[0];
    output[5] -= output[1];
    output[6] -= output[2];
    output[0] += camera.pos.x.bits;
    output[1] += camera.pos.y.bits;
    output[12] -= output[8];
    output[14] -= output[10];
    output[13] -= output[9];
    output[2] += camera.pos.z.bits;
    output[8] += camera.pos.x.bits;
    output[9] += camera.pos.y.bits;
    output[10] += camera.pos.z.bits;
    return output[10];
}

GDB_CALL sint32 npc_emit_attack_effects(sint32 idx)
{
    FUNCTION_MARKER(0x8004d914u, "GAME.EXE");
    NPC_SPECIAL *special = &npc_behavior_actors[idx].special;
    uint32 flags = actor_animations[idx].motion.events, point[16];
    int i;
    if (flags & 0x1000u)
    {
        if (flags & 0x800u)
            actor_emit_ground_effect(idx);
        else
            sound_voice_play(187, 255, 6144);
    }
    if (flags & 0xe000u)
    {
        uint32 control = actor_status[idx].flags;
        special->attack.effect_height = 0x00c90000;
        if (!(control & 2u))
        {
            npc_build_attack_vectors(idx, 0x28000, point);
            for (i = 0; i < 2; i++)
                if (flags & (0x2000u << i))
                {
                    uint32 *p = point + 8 * i;
                    effect_spawn_motion(876, p[0], p[1], p[2], p[4], p[5], p[6]);
                    effect_spawn_motion(876, p[0], p[1], p[2], p[4] << 1, p[5], p[6] << 1);
                }
            if (flags & 0x8000u)
            {
                npc_build_attack_vectors(idx, 0x18000, point);
                for (i = 0; i < 2; i++)
                {
                    uint32 *p = point + 8 * i;
                    effect_spawn_motion(877, p[0], p[1], p[2], p[4], p[5], p[6]);
                }
            }
        }
    }
    else
        special->attack.effect_height = 0x00590000;
    if (actor_animations[idx].frame == 0xffffffffu)
    {
        special->attack.effect_height = 0x00090000;
        return npc_update_combat(idx);
    }
    return 0x00090000;
}

GDB_CALL intptr npc_update_stage4_departure(sint32 idx)
{
    FUNCTION_MARKER(0x8003b8e4u, "GAME.EXE");
    uint32 yaw, result;
    sint32 tick, target, delta, x, z, action = -1;
    if (game_session.stage != 4)
        return npc_update_actor(idx);
    actor_lifecycle[idx].delay = 1;
    tick = (sint32)game_session.tick;
    if (tick >= 301)
    {
        target = tick >= 571 ? 5200 : 4800;
        x = actor_positions[idx].x.whole;
        z = actor_positions[idx].z.whole;
        if (math_distance_approx(-128, target, x, z) >= 257)
        {
            yaw = (uint32)(uint16)actor_orientation[idx].yaw << 5;
            x = actor_positions[idx].x.whole;
            z = actor_positions[idx].z.whole;
            delta = (sint16)((uint32)math_dir16(x, z, -128, target) - yaw + 2u);
            delta >>= 2;
            if (delta > 512)
                delta = 512;
            else if (delta < -512)
                delta = -512;
            actor_orientation[idx].yaw = (uint16)(yaw + (uint32)delta) >> 5;
            if (actor_animations[idx].frame != 0xffffffffu && actor_animations[idx].id != 69)
                goto finish;
            if ((uint32)(uint16)actor_orientation[idx].yaw - 961u < 127u && dir_distance >= 513)
                action = 87;
            else if (actor_orientation[idx].yaw < 1536)
                action = 82;
        }
    }
    if (action == -1 && actor_animations[idx].frame == 0xffffffffu)
        action = 69;
    if (action != -1)
        animation_start(idx, animation_type_banks[(uint32)actor_types[idx]], action);
finish:
    if (game_session.tick != 630)
        return -1;
    actor_types[idx] = ACTOR_TYPE_NONE;
    result = level_wave.remaining_enemies - 1u;
    level_wave.remaining_enemies = result;
    return (sint32)result;
}

GDB_CALL sint32 npc_attack_electric(sint32 idx)
{
    FUNCTION_MARKER(0x8004db94u, "GAME.EXE");
    uint32 state;
    sint32 i, heading;
    int hit = 0;
    /* TODO: Investigate an electric attack outside the native actor pool */
    if ((uint32)idx >= ACTOR_COUNT)
    {
        DIAGNOSTIC(fprintf(stderr, "Invalid electric attacker %d players %u stage %u tick %u\n", idx, game_session.player_count, game_session.stage, game_session.tick));
        diagnostic_stop(__FUNCTION__, __FILE__, __LINE__);
    }
    if ((sint32)(actor_positions[idx].y.bits + actor_animations[idx].motion.bits[0]) < (sint32)0xff000000u)
        return 1;
    state = actor_animations[idx].id;
    if (state - 274u < 2u || state == 276)
        return 276;
    ACTOR_STATUS *status = actor_status + idx;
    for (i = 0; i < (sint32)game_session.player_count; i++)
    {
        /* TODO: Investigate an electric target beyond the native actor pool */
        if ((uint32)i >= ACTOR_COUNT)
        {
            DIAGNOSTIC(fprintf(stderr, "Invalid electric target %d attacker %d players %u stage %u tick %u\n", i, idx, game_session.player_count, game_session.stage, game_session.tick));
            diagnostic_stop(__FUNCTION__, __FILE__, __LINE__);
        }
        if (actor_types[i] == ACTOR_TYPE_NONE || actor_health[i].hp <= 0)
            continue;
        heading = (sint16)math_dir16(actor_positions[idx].x.whole, actor_positions[idx].z.whole, actor_positions[i].x.whole, actor_positions[i].z.whole);
        heading >>= 5;
        if (dir_distance < 1280 && (sint32)status->deadline < (sint32)(game_session.tick + 20u))
            status->flags = status->flags | ACTOR_ELECTRIC;
        if (dir_distance >= 448)
            continue;
        state = actor_animations[i].id;
        if (state == 147 || state == 148 || state == 252 || state == 251 || (sint32)game_session.tick < (sint32)status->deadline)
            continue;
        actors_emit_link(idx, i);
        sint32 type = actor_types[i];
        /* TODO: Investigate a changed electric target type after link creation */
        if ((uint32)type >= ANIMATION_TYPE_COUNT)
        {
            DIAGNOSTIC(fprintf(stderr, "Invalid electric target type %d target %d attacker %d stage %u tick %u\n", type, i, idx, game_session.stage, game_session.tick));
            diagnostic_stop(__FUNCTION__, __FILE__, __LINE__);
        }
        animation_start(i, animation_type_banks[(uint32)type], 251);
        actor_orientation[i].yaw = (uint16)(heading ^ 1024);
        actor_apply_damage(idx, i, 20);
        sound_voice_play(185, 255, 6144);
        hit = 1;
        actor_status[i].flags = actor_status[i].flags | ACTOR_ELECTRIC;
        actor_status[i].deadline = game_session.tick + 20u;
    }
    if (hit)
    {
        status->deadline = game_session.tick + 80u;
        return npc_update_ranged_attack(idx);
    }
    return 0;
}

GDB_CALL sint32 npc_move_oscillating(sint32 idx)
{
    FUNCTION_MARKER(0x8004cfe4u, "GAME.EXE");
    NPC_SPECIAL *special = &npc_behavior_actors[idx].special;
    uint32 x, z, yaw, phase, offset;
    sint32 slot, delta, reach, speed;
    slot = (sint32)npc_targets[idx].idx;
    if (npc_targets[idx].role == 1 && slot != -1)
    {
        /* TODO: Investigate an oscillating NPC with an invalid actor target */
        if ((uint32)slot >= ACTOR_COUNT)
        {
            DIAGNOSTIC(fprintf(stderr, "Invalid oscillating NPC actor %d target %d stage %u tick %u\n", idx, slot, game_session.stage, game_session.tick));
            diagnostic_stop(__FUNCTION__, __FILE__, __LINE__);
        }
        x = actor_positions[slot].x.bits;
        z = actor_positions[slot].z.bits;
    }
    else
    {
        if (npc_targets[idx].role != 4 || slot == -1)
            return -1;
        /* TODO: Investigate an oscillating NPC with an invalid item target */
        if ((uint32)slot >= DYNAMIC_OBJECT_COUNT)
        {
            DIAGNOSTIC(fprintf(stderr, "Invalid oscillating NPC actor %d item %d stage %u tick %u\n", idx, slot, game_session.stage, game_session.tick));
            diagnostic_stop(__FUNCTION__, __FILE__, __LINE__);
        }
        const MODEL_OBJECT *model = dynamic_objects + slot;
        x = model->pos.x.bits;
        z = model->pos.z.bits;
    }
    delta = math_dir16(actor_positions[idx].x.whole, actor_positions[idx].z.whole, (sint32)x >> 16, (sint32)z >> 16);
    yaw = (uint16)actor_orientation[idx].yaw;
    delta = (sint16)((uint32)delta - (yaw << 5));
    yaw += delta < -1024 ? (uint32)-32 : delta > 1024 ? 32u : (uint32)(delta >> 5);
    actor_orientation[idx].yaw = (uint16)yaw;
    reach = (uint32)dir_distance - 384u;
    actor_orientation[idx].yaw = (uint16)(yaw & 2047);
    if (reach > 384)
        reach = 384;
    phase = special->attack.phase;
    phase += (uint32)((sint32)(((uint32)reach << 2) - phase) >> 3);
    special->attack.phase = phase;
    offset = (uint32)((sint32)phase >> 5) & 2047u;
    special->attack.speed = offset;
    speed = sin_table[offset] >> 6;
    special->attack.speed = (uint32)speed;
    if (speed > 48)
        speed = 48;
    else if (speed < -48)
        speed = -48;
    special->attack.speed = (uint32)speed;
    offset = 2u * (uint32)(sint32)actor_orientation[idx].yaw;
    actor_positions[idx].x.bits = actor_positions[idx].x.bits + (uint32)(speed * sin_table[offset >> 1]);
    actor_positions[idx].z.bits = actor_positions[idx].z.bits - (uint32)(speed * sin_table[MATH_QUARTER_TURN + (offset >> 1)]);
    return dir_distance;
}

GDB_CALL sint32 npc_release_captive(sint32 idx)
{
    FUNCTION_MARKER(0x80054598u, "GAME.EXE");
    uint32 random, other_table;
    sint32 type = actor_types[idx];
    uint32 frame = actor_animations[idx].frame;
    uint32 table = animation_type_banks[(uint32)type];
    sint32 slot, action, other_type;
    if (frame != 0xffffffffu)
        return -1;
    slot = actor_contacts[idx].captive;
    if (slot == -1)
        return npc_update_ranged_attack(idx);
    if (table == ANIMATION_BANK_SHARED8 || table == ANIMATION_BANK_SHARED2 || table == ANIMATION_BANK_SHARED4 || table == ANIMATION_BANK_SHARED1)
        action = 181;
    else
    {
        random = random_next() & 3u;
        if (!random)
            return npc_start_animation(idx, table, 10);
        if (random == 1)
            return npc_start_animation(idx, table, 40);
        /* The MIPS fallback at54764 is unreachable after ANDI3 and the 0/1 gates. */
        action = table == ANIMATION_BANK_SHARED7 ? 182 : 180;
    }
    actor_contacts[idx].captive = ACTOR_SLOT_NONE;
    if (action == 181)
    {
        other_type = actor_types[slot];
        actor_contacts[slot].captor = ACTOR_SLOT_NONE;
        actor_contacts[slot].last_attacker = (uint8)idx;
    }
    else
    {
        actor_contacts[slot].captor = ACTOR_SLOT_NONE;
        actor_contacts[slot].last_attacker = (uint8)idx;
        other_type = actor_types[slot];
    }
    other_table = animation_type_banks[(uint32)other_type];
    animation_start_held(slot, other_table, action);
    return npc_start_animation(idx, table, 177);
}

GDB_CALL sint32 npc_finish_grapple(sint32 idx)
{
    FUNCTION_MARKER(0x8004cb88u, "GAME.EXE");

    enum
    {
        NPC_GRAPPLE_PAIR_COUNT = 3
    };

    static const struct NPC_GRAPPLE_PAIR
    {
        uint32 actor_animation;
        uint32 captive_animation;
    } pairs[NPC_GRAPPLE_PAIR_COUNT] = {{177, 181}, {3, 141}, {4, 142}};
    const struct NPC_GRAPPLE_PAIR *pair;
    uint32 table, state, angle, heading, action;
    sint32 type, slot, random, x, z, distance;
    type = actor_types[idx];
    state = actor_animations[idx].frame;
    table = animation_type_banks[(uint32)type];
    if (state != 0xffffffff)
        return -1;
    slot = actor_contacts[idx].captive;
    if (slot == -1)
        return npc_update_ranged_attack(idx);
    x = actor_positions[idx].x.whole;
    z = actor_positions[idx].z.whole;
    angle = (uint32)math_dir16(x, z, -1216, 432);
    heading = (uint16)actor_orientation[idx].yaw;
    distance = dir_distance;
    angle -= heading << 5;
    if (distance >= 513 && (uint16)(angle + 4096) >= 8193)
    {
        animation_start(idx, table, 167);
        actor_contacts[0].grapple_mode = 1;
        type = actor_types[slot];
        table = animation_type_banks[(uint32)type];
        return animation_start_held(slot, table, 172);
    }
    actor_contacts[idx].captive = ACTOR_SLOT_NONE;
    actor_contacts[slot].captor = ACTOR_SLOT_NONE;
    actor_contacts[slot].last_attacker = (uint8)idx;
    random = (sint32)random_next();
    pair = pairs + random % NPC_GRAPPLE_PAIR_COUNT;
    action = pair->actor_animation;
    animation_start(idx, table, action);
    type = actor_types[slot];
    action = pair->captive_animation;
    table = animation_type_banks[(uint32)type];
    return animation_start_held(slot, table, action);
}

GDB_CALL sint32 npc_fire_fan(sint32 idx)
{
    FUNCTION_MARKER(0x8004e6acu, "GAME.EXE");
    FIXED_VEC3 pos;
    uint32 angle;
    int ray;
    sint32 hit = 0;
    for (ray = 0; ray < 3; ray++)
    {
        pos.x.bits = actor_positions[idx].x.bits;
        pos.y.bits = actor_positions[idx].y.bits - 0x400000;
        pos.z.bits = actor_positions[idx].z.bits;
        angle = (uint16)actor_orientation[idx].yaw;
        if (ray == 1)
            angle -= 384;
        else if (ray == 2)
            angle += 384;
        hit = projectile_trace_npc(pos.bits, angle << 5, idx);
        if (hit)
            break;
    }
    return hit != 0;
}

GDB_CALL sint32 npc_carry_captive_to_point(sint32 idx)
{
    FUNCTION_MARKER(0x8004ce48u, "GAME.EXE");
    uint32 table, frame, angle, heading;
    sint32 type, slot, x, z, distance;
    type = actor_types[idx];
    slot = actor_contacts[idx].captive;
    table = animation_type_banks[(uint32)type];
    if (slot == -1)
        return npc_update_ranged_attack(idx);
    npc_turn_to_point(idx, 0xfb400000, 0x01b00000);
    actors_apply_pair_offset(idx, slot);
    frame = actor_animations[idx].frame;
    if (frame != 0xffffffff)
        return frame;
    x = actor_positions[idx].x.whole;
    z = actor_positions[idx].z.whole;
    angle = (uint32)math_dir16(x, z, -1216, 432);
    heading = (uint16)actor_orientation[idx].yaw;
    distance = dir_distance;
    angle -= heading << 5;
    if (distance >= 257 && (uint16)(angle + 2048) >= 4097)
    {
        animation_start_held(idx, table, 169);
        type = actor_types[slot];
        table = animation_type_banks[(uint32)type];
        return animation_start_held(slot, table, 77);
    }
    actor_contacts[idx].captive = ACTOR_SLOT_NONE;
    actor_contacts[slot].captor = ACTOR_SLOT_NONE;
    actor_contacts[slot].last_attacker = (uint8)idx;
    animation_start(idx, table, 178);
    type = actor_types[slot];
    table = animation_type_banks[(uint32)type];
    return animation_start_held(slot, table, 184);
}

GDB_CALL sint32 npc_throw_items(sint32 idx)
{
    FUNCTION_MARKER(0x800555dcu, "GAME.EXE");
    FIXED_VEC3 point;
    /* TODO: Investigate NPC item actions outside the native actor pool */
    if (idx < 0 || idx >= ACTOR_COUNT)
    {
        DIAGNOSTIC(fprintf(stderr, "Invalid NPC item actor %d stage %u tick %u\n", idx, game_session.stage, game_session.tick));
        diagnostic_stop(__FUNCTION__, __FILE__, __LINE__);
    }
    sint32 type = actor_types[idx];
    uint32 frame = actor_animations[idx].frame;
    ANIMATION_BANK_ID bank = animation_type_banks[(uint32)type];
    sint32 slot;
    if (frame != 0xffffffffu)
        return -1;
    npc_start_animation(idx, bank, 160);
    slot = actor_items[idx].slot[ACTOR_ITEM_RIGHT];
    if (slot != -1)
    {
        actor_items[idx].slot[ACTOR_ITEM_RIGHT] = -1;
        /* TODO: Investigate the original right-hand item slot before its first payload write */
        if (slot < 0 || slot >= DYNAMIC_OBJECT_COUNT)
        {
            DIAGNOSTIC(fprintf(stderr, "Invalid NPC throw actor %d right-hand source slot %d stage %u tick %u\n", idx, slot, game_session.stage, game_session.tick));
            diagnostic_stop(__FUNCTION__, __FILE__, __LINE__);
        }
        MODEL_OBJECT *item = dynamic_objects + slot;
        /* TODO: Investigate an NPC item access with a foreign native owner */
        if (item->payload_kind != MODEL_PAYLOAD_PHYSICS)
        {
            DIAGNOSTIC(fprintf(stderr, "Invalid NPC item owner actor %d slot %d kind %u owner %u phase throw-motion stage %u tick %u\n", idx, slot, item->kind, (uint32)item->payload_kind, game_session.stage, game_session.tick));
            diagnostic_stop(__FUNCTION__, __FILE__, __LINE__);
        }
        item->physics.speed = 128;
        item->physics.vertical_speed = 64;
        item->physics.spin = 6528;
        item->angles.yaw_bits = (uint16)((uint32)((uint16)actor_orientation[idx].yaw) << 5);
        actor_attachment_pos(idx, 3, point.bits);
        item->pos.x.bits = point.x.bits;
        item->pos.y.bits = point.y.bits;
        /* TODO: Investigate an NPC item access with a foreign native owner */
        if (item->payload_kind != MODEL_PAYLOAD_PHYSICS)
        {
            DIAGNOSTIC(fprintf(stderr, "Invalid NPC item owner actor %d slot %d kind %u owner %u phase throw-after-attachment stage %u tick %u\n", idx, slot, item->kind, (uint32)item->payload_kind, game_session.stage, game_session.tick));
            diagnostic_stop(__FUNCTION__, __FILE__, __LINE__);
        }
        item->physics.actor_idx = idx;
        item->pos.z.bits = point.z.bits;
    }
    if (actor_items[idx].slot[ACTOR_ITEM_LEFT] == -1)
        return -1;
    /* Original 800556D8 reloads slot 126, even after clearing it above. */
    slot = actor_items[idx].slot[ACTOR_ITEM_RIGHT];
    actor_items[idx].slot[ACTOR_ITEM_LEFT] = -1;
    /* TODO: Investigate the original left-hand item slot before its first payload write */
    if (slot < 0 || slot >= DYNAMIC_OBJECT_COUNT)
    {
        DIAGNOSTIC(fprintf(stderr, "Invalid NPC throw actor %d left-hand source slot %d stage %u tick %u\n", idx, slot, game_session.stage, game_session.tick));
        diagnostic_stop(__FUNCTION__, __FILE__, __LINE__);
    }
    MODEL_OBJECT *item = dynamic_objects + slot;
    /* TODO: Investigate an NPC item access with a foreign native owner */
    if (item->payload_kind != MODEL_PAYLOAD_PHYSICS)
    {
        DIAGNOSTIC(fprintf(stderr, "Invalid NPC item owner actor %d slot %d kind %u owner %u phase throw-motion stage %u tick %u\n", idx, slot, item->kind, (uint32)item->payload_kind, game_session.stage, game_session.tick));
        diagnostic_stop(__FUNCTION__, __FILE__, __LINE__);
    }
    item->physics.speed = 128;
    item->physics.vertical_speed = 64;
    item->physics.spin = 6528;
    item->angles.yaw_bits = (uint16)((uint32)((uint16)actor_orientation[idx].yaw) << 5);
    actor_attachment_pos(idx, 2, point.bits);
    item->pos.x.bits = point.x.bits;
    item->pos.y.bits = point.y.bits;
    /* TODO: Investigate an NPC item access with a foreign native owner */
    if (item->payload_kind != MODEL_PAYLOAD_PHYSICS)
    {
        DIAGNOSTIC(fprintf(stderr, "Invalid NPC item owner actor %d slot %d kind %u owner %u phase throw-after-attachment stage %u tick %u\n", idx, slot, item->kind, (uint32)item->payload_kind, game_session.stage, game_session.tick));
        diagnostic_stop(__FUNCTION__, __FILE__, __LINE__);
    }
    item->physics.actor_idx = idx;
    item->pos.z.bits = point.z.bits;
    return (sint32)point.z.bits;
}

GDB_CALL sint32 npc_advance_charge(sint32 idx)
{
    FUNCTION_MARKER(0x8004edb8u, "GAME.EXE");
    sint32 type = actor_types[idx];
    uint32 role = npc_targets[idx].role;
    uint32 table = animation_type_banks[(uint32)type];
    uint32 slot = 0xffffffff, x, z, heading_x, heading_z, dx, dz, frame;
    if (role == 1)
        slot = npc_targets[idx].idx;
    if (slot == 0xffffffff)
    {
        if (actor_animations[idx].frame != 0xffffffffu)
            return -1;
        return npc_start_animation(idx, table, 276);
    }
    x = actor_positions[slot].x.bits;
    z = actor_positions[slot].z.bits;
    npc_turn_to_point(idx, x, z);
    heading_x = (uint32)(sint32)actor_orientation[idx].yaw;
    heading_z = (uint32)(sint32)actor_orientation[idx].yaw;
    dx = (uint32)(sint32)sin_table[heading_x] << 5;
    x = actor_positions[idx].x.bits;
    actor_positions[idx].x.bits = x + dx;
    dz = (uint32)(sint32)sin_table[MATH_QUARTER_TURN + heading_z] << 5;
    z = actor_positions[idx].z.bits;
    frame = actor_animations[idx].frame;
    actor_positions[idx].z.bits = z - dz;
    if (frame != 0xffffffff)
        return dz;
    if (dir_distance < 448)
    {
        sound_voice_volume(stage_audio_slots[3].voice, 0);
        return npc_start_animation(idx, table, 276);
    }
    return npc_start_animation(idx, table, 275);
}

GDB_CALL intptr npc_update_jetpack_sound(sint32 idx)
{
    FUNCTION_MARKER(0x8004ddecu, "GAME.EXE");
    uint32 state = actor_animations[idx].id, volume, pan;
    sint32 level;
    level = (state - 257u < 2u || state - 254u < 2u || state - 266u < 2u || state == 31 || state == 3) ? 16 : -16;
    level = (uint32)level + stage_audio_slots[1].level;
    if (level < 64)
        level = 64;
    if (level > 192)
        level = 192;
    stage_audio_slots[1].level = level;
    sound_spatial(actor_positions + idx, &volume, &pan);
    volume *= stage_audio_slots[1].level;
    sound_voice_pan(stage_audio_slots[0].voice, (sint32)volume >> 8, pan);
    sound_voice_pitch(stage_audio_slots[0].voice, level * 8 + 5120);
    return npc_update_actor(idx);
}

GDB_CALL sint32 npc_finish_aimed_charge(sint32 idx)
{
    FUNCTION_MARKER(0x8004e574u, "GAME.EXE");
    sint32 type = actor_types[idx];
    sint32 deadline = ((sint32)actor_status[idx].deadline);
    sint32 tick = (sint32)game_session.tick;
    uint32 table = animation_type_banks[(uint32)type], slot, frame, x, z;
    int attack = 0;
    if (tick >= deadline && npc_targets[idx].role == 1)
    {
        slot = npc_targets[idx].idx;
        if (slot != 0xffffffff)
        {
            x = actor_positions[slot].x.bits;
            z = actor_positions[slot].z.bits;
            if ((uint16)((uint32)npc_turn_to_point(idx, x, z) + 2047) < 4095 && dir_distance < 1281)
                attack = 1;
        }
    }
    frame = actor_animations[idx].frame;
    if (frame != 0xffffffffu)
        return attack ? (sint32)frame : -1;
    return npc_start_animation(idx, table, attack ? 271 : 272);
}

GDB_CALL sint32 npc_update_fall(sint32 idx)
{
    FUNCTION_MARKER(0x800553f0u, "GAME.EXE");
    uint32 slot, height;
    NPC_BEHAVIOR_ACTOR *behavior = npc_behavior_actors + idx;
    sint32 type = actor_types[idx];
    sint32 speed = behavior->fall_speed, limit, next_speed;
    ANIMATION_BANK_ID table = animation_type_banks[(uint32)type];
    next_speed = (uint8)behavior->fall_speed + 4;
    height = actor_positions[idx].y.bits;
    behavior->fall_speed = (sint8)(uint8)next_speed;
    limit = npc_fall_speed_limit;
    next_speed = (sint8)(uint8)next_speed;
    actor_positions[idx].y.bits = height + ((uint32)speed << 16);
    if (limit < next_speed)
        behavior->fall_speed = (sint8)(uint8)limit;
    if ((sint32)actor_positions[idx].y.bits >= 0)
    {
        slot = behavior->progress.landing_item_idx;
        actor_positions[idx].y.bits = 0;
        if (slot != 0xffffffffu)
        {
            /* TODO: Investigate a landing item index outside the original pool */
            if (slot >= DYNAMIC_OBJECT_COUNT)
            {
                DIAGNOSTIC(fprintf(stderr, "Invalid landing item actor %d slot %u type %d state %u stage %u tick %u\n", idx, slot, type, actor_animations[idx].id, game_session.stage, game_session.tick));
                diagnostic_stop(__FUNCTION__, __FILE__, __LINE__);
            }
            MODEL_OBJECT *item = dynamic_objects + slot;
            /* TODO: Investigate the shared progress word selecting a foreign owner */
            if (item->payload_kind != MODEL_PAYLOAD_PHYSICS && item->payload_kind != MODEL_PAYLOAD_UNMIGRATED)
            {
                DIAGNOSTIC(fprintf(stderr, "Invalid landing item owner actor %d slot %u kind %u owner %u stage %u tick %u\n", idx, slot, item->kind, (uint32)item->payload_kind, game_session.stage, game_session.tick));
                diagnostic_stop(__FUNCTION__, __FILE__, __LINE__);
            }
            if (item->payload_kind == MODEL_PAYLOAD_PHYSICS)
                item->physics.spin = 1;
            else
                item->pending_motion.pitch_speed = 1;
        }
        npc_start_animation(idx, table, actor_animations[idx].id == 203 ? 204 : 206);
        if (game_session.stage == 8)
            sound_voice_play(229, 255, 6144);
    }
    if (actor_animations[idx].frame == 0xffffffffu)
        return npc_start_animation(idx, table, actor_animations[idx].id);
    return -1;
}

GDB_CALL sint32 npc_wait_target_animation(sint32 idx)
{
    FUNCTION_MARKER(0x800551f0u, "GAME.EXE");
    uint32 slot;
    sint32 type = actor_types[idx];
    uint32 frame = actor_animations[idx].frame;
    uint32 table = animation_type_banks[(uint32)type];
    sint32 state;
    if (frame != 0xffffffffu)
        return -1;
    state = npc_base_animation(idx);
    if (state == 208)
        return npc_start_animation(idx, table, 210);
    if (state == 210)
    {
        slot = npc_targets[idx].idx;
        if (slot != 0xffffffffu && npc_targets[idx].role != 4 && actor_animations[slot].id == 67)
            return npc_start_animation(idx, table, 210);
        return npc_start_animation(idx, table, 209);
    }
    if (state == 209)
        return npc_update_combat(idx);
    return 209;
}

GDB_CALL sint32 npc_claim_ranged_item(sint32 idx)
{
    FUNCTION_MARKER(0x80053cecu, "GAME.EXE");
    /* TODO: Investigate NPC item actions outside the native actor pool */
    if (idx < 0 || idx >= ACTOR_COUNT)
    {
        DIAGNOSTIC(fprintf(stderr, "Invalid NPC item actor %d stage %u tick %u\n", idx, game_session.stage, game_session.tick));
        diagnostic_stop(__FUNCTION__, __FILE__, __LINE__);
    }
    sint32 type = actor_types[idx];
    uint32 frame = actor_animations[idx].frame;
    uint32 table = animation_type_banks[(uint32)type];
    if (frame != 0xffffffffu)
        return 4;
    if (npc_targets[idx].role == 4)
    {
        uint32 slot = npc_targets[idx].idx;
        if (slot != 0xffffffffu)
        {
            /* TODO: Investigate NPC pickup targets outside the native item pool */
            if (slot >= DYNAMIC_OBJECT_COUNT)
            {
                DIAGNOSTIC(fprintf(stderr, "Invalid NPC pickup actor %d target slot %u stage %u tick %u\n", idx, slot, game_session.stage, game_session.tick));
                diagnostic_stop(__FUNCTION__, __FILE__, __LINE__);
            }
            MODEL_OBJECT *model = dynamic_objects + slot;
            if (!model->pos.y.bits)
            {
                uint32 kind;
                actor_items[idx].slot[ACTOR_ITEM_RIGHT] = (sint8)slot;
                kind = model->kind;
                if (kind == 522)
                {
                    model->kind = 523;
                    kind = model->kind;
                }
                if (kind == 524)
                    model->kind = 525;
                model->pos.y.bits = 1;
            }
        }
    }
    return npc_start_animation(idx, table, 122);
}

GDB_CALL sint32 npc_begin_captive_carry(sint32 idx)
{
    FUNCTION_MARKER(0x8004cd5cu, "GAME.EXE");
    sint32 type = actor_types[idx];
    uint32 frame = actor_animations[idx].frame;
    uint32 table = animation_type_banks[(uint32)type];
    sint32 slot;
    if (frame != 0xffffffffu)
        return -1;
    slot = actor_contacts[idx].captive;
    if (slot == -1)
        return npc_update_ranged_attack(idx);
    animation_start_held(slot, animation_type_banks[(uint32)actor_types[slot]], 77);
    animation_start_held(idx, table, 169);
    return actors_capture_pair_offset(idx, slot);
}

GDB_CALL sint32 npc_finish_vulkan_combo(sint32 idx)
{
    FUNCTION_MARKER(0x8004e484u, "GAME.EXE");
    sint32 type = actor_types[idx];
    uint32 frame = actor_animations[idx].frame;
    uint32 table = animation_type_banks[(uint32)type];
    sint32 state;
    if (frame != 0xffffffffu)
        return -1;
    state = npc_base_animation(idx);
    if (state == 57)
    {
        actor_animations[idx].frame = 0xffffffffu;
        return npc_start_animation(idx, table, 60);
    }
    if (state == 60)
        return npc_start_animation(idx, table, 58);
    if (state == 59)
        return npc_start_animation(idx, table, 60);
    if (state == 58)
        return npc_continue_alternate_combat(idx);
    return 58;
}

GDB_CALL intptr npc_update_stage4_idle(sint32 idx)
{
    FUNCTION_MARKER(0x8003b83cu, "GAME.EXE");
    sint32 type, action;
    uint32 table;
    if (game_session.stage != 4 || (sint32)game_session.tick >= 660)
        return npc_update_actor(idx);
    if (actor_animations[idx].frame != 0xffffffffu)
        return -1;
    type = actor_types[idx];
    table = animation_type_banks[(uint32)type];
    action = (sint32)npc_behavior_actors[idx].idle_animation;
    return npc_start_animation(idx, table, action);
}

GDB_CALL sint32 npc_finish_turn(sint32 idx)
{
    FUNCTION_MARKER(0x80054988u, "GAME.EXE");
    uint32 table;
    if (actor_animations[idx].frame != 0xffffffffu)
        return -1;
    table = animation_type_banks[(uint32)actor_types[idx]];
    if (table != ANIMATION_BANK_SHARED8 && table != ANIMATION_BANK_SHARED4 && table != ANIMATION_BANK_SHARED1 && table != ANIMATION_BANK_SHARED2)
        actor_orientation[idx].yaw = ((uint16)actor_orientation[idx].yaw) ^ 1024;
    return npc_update_ranged_attack(idx);
}

GDB_CALL sint32 npc_finish_captive_strike(sint32 idx)
{
    FUNCTION_MARKER(0x80054840u, "GAME.EXE");
    sint32 type = actor_types[idx];
    uint32 frame = actor_animations[idx].frame;
    uint32 table = animation_type_banks[(uint32)type];
    if (frame != 0xffffffffu)
        return -1;
    if (actor_contacts[idx].captive == -1)
        return npc_update_ranged_attack(idx);
    return npc_start_animation(idx, table, random_next() & 1u ? 12 : 13);
}

GDB_CALL sint32 npc_choose_held_item_action(sint32 idx)
{
    FUNCTION_MARKER(0x80053a70u, "GAME.EXE");
    /* TODO: Investigate the original out-of-pool NPC operation */
    if (idx < 0 || idx >= ACTOR_COUNT)
    {
        DIAGNOSTIC(fprintf(stderr, "Invalid NPC actor %d in %s stage %u tick %u\n", idx, __FUNCTION__, game_session.stage, game_session.tick));
        diagnostic_stop(__FUNCTION__, __FILE__, __LINE__);
    }
    sint32 type = actor_types[idx];
    uint32 frame = actor_animations[idx].frame;
    uint32 table = animation_type_banks[(uint32)type];
    sint32 slot;
    if (frame != 0xffffffffu)
        return -1;
    slot = actor_items[idx].slot[ACTOR_ITEM_RIGHT];
    if (slot == -1)
        return npc_update_combat(idx);
    /* TODO: Investigate the original out-of-pool NPC operation */
    if (slot < 0 || slot >= DYNAMIC_OBJECT_COUNT)
    {
        DIAGNOSTIC(fprintf(stderr, "Invalid NPC action item actor %d slot %d in %s stage %u tick %u\n", idx, slot, __FUNCTION__, game_session.stage, game_session.tick));
        diagnostic_stop(__FUNCTION__, __FILE__, __LINE__);
    }
    if (dynamic_objects[slot].kind - 499u >= 5u)
        return npc_update_combat(idx);
    return npc_start_animation(idx, table, (sint32)(random_next() & 1u) + 247);
}

GDB_CALL sint32 npc_claim_right_item0(sint32 idx)
{
    FUNCTION_MARKER(0x800539c0u, "GAME.EXE");
    /* TODO: Investigate NPC item operations outside the native actor pool */
    if (idx < 0 || idx >= ACTOR_COUNT)
    {
        DIAGNOSTIC(fprintf(stderr, "Invalid NPC item actor %d in %s stage %u tick %u\n", idx, __FUNCTION__, game_session.stage, game_session.tick));
        diagnostic_stop(__FUNCTION__, __FILE__, __LINE__);
    }
    uint32 type = (uint32)actor_types[idx];
    uint32 frame = actor_animations[idx].frame;
    uint32 table = animation_type_banks[type];
    if (frame != 0xffffffffu)
        return 4;
    if (npc_targets[idx].role == 4)
    {
        uint32 slot = npc_targets[idx].idx;
        if (slot != 0xffffffffu)
        {
            /* TODO: Investigate an NPC claim outside the item pool */
            if (slot >= DYNAMIC_OBJECT_COUNT)
            {
                DIAGNOSTIC(fprintf(stderr, "Invalid NPC claim actor %d hand %u slot %u action %d stage %u tick %u\n", idx, (uint32)ACTOR_ITEM_RIGHT, slot, 119, game_session.stage, game_session.tick));
                diagnostic_stop(__FUNCTION__, __FILE__, __LINE__);
            }
            MODEL_OBJECT *model = dynamic_objects + slot;
            if (!model->pos.y.bits)
            {
                actor_items[idx].slot[ACTOR_ITEM_RIGHT] = (sint8)slot;
                model->pos.y.bits = 1;
            }
        }
    }
    return npc_start_animation(idx, table, 119);
}

GDB_CALL sint32 npc_claim_right_item1(sint32 idx)
{
    FUNCTION_MARKER(0x80053b40u, "GAME.EXE");
    /* TODO: Investigate NPC item operations outside the native actor pool */
    if (idx < 0 || idx >= ACTOR_COUNT)
    {
        DIAGNOSTIC(fprintf(stderr, "Invalid NPC item actor %d in %s stage %u tick %u\n", idx, __FUNCTION__, game_session.stage, game_session.tick));
        diagnostic_stop(__FUNCTION__, __FILE__, __LINE__);
    }
    uint32 type = (uint32)actor_types[idx];
    uint32 frame = actor_animations[idx].frame;
    uint32 table = animation_type_banks[type];
    if (frame != 0xffffffffu)
        return 4;
    if (npc_targets[idx].role == 4)
    {
        uint32 slot = npc_targets[idx].idx;
        if (slot != 0xffffffffu)
        {
            /* TODO: Investigate an NPC claim outside the item pool */
            if (slot >= DYNAMIC_OBJECT_COUNT)
            {
                DIAGNOSTIC(fprintf(stderr, "Invalid NPC claim actor %d hand %u slot %u action %d stage %u tick %u\n", idx, (uint32)ACTOR_ITEM_RIGHT, slot, 124, game_session.stage, game_session.tick));
                diagnostic_stop(__FUNCTION__, __FILE__, __LINE__);
            }
            MODEL_OBJECT *model = dynamic_objects + slot;
            if (!model->pos.y.bits)
            {
                actor_items[idx].slot[ACTOR_ITEM_RIGHT] = (sint8)slot;
                model->pos.y.bits = 1;
            }
        }
    }
    return npc_start_animation(idx, table, 124);
}

GDB_CALL sint32 npc_claim_left_item(sint32 idx)
{
    FUNCTION_MARKER(0x80053bf0u, "GAME.EXE");
    /* TODO: Investigate NPC item operations outside the native actor pool */
    if (idx < 0 || idx >= ACTOR_COUNT)
    {
        DIAGNOSTIC(fprintf(stderr, "Invalid NPC item actor %d in %s stage %u tick %u\n", idx, __FUNCTION__, game_session.stage, game_session.tick));
        diagnostic_stop(__FUNCTION__, __FILE__, __LINE__);
    }
    uint32 type = (uint32)actor_types[idx];
    uint32 frame = actor_animations[idx].frame;
    uint32 table = animation_type_banks[type];
    if (frame != 0xffffffffu)
        return 4;
    if (npc_targets[idx].role == 4)
    {
        uint32 slot = npc_targets[idx].idx;
        if (slot != 0xffffffffu)
        {
            /* TODO: Investigate an NPC claim outside the item pool */
            if (slot >= DYNAMIC_OBJECT_COUNT)
            {
                DIAGNOSTIC(fprintf(stderr, "Invalid NPC claim actor %d hand %u slot %u action %d stage %u tick %u\n", idx, (uint32)ACTOR_ITEM_LEFT, slot, 127, game_session.stage, game_session.tick));
                diagnostic_stop(__FUNCTION__, __FILE__, __LINE__);
            }
            MODEL_OBJECT *model = dynamic_objects + slot;
            if (!model->pos.y.bits)
            {
                actor_items[idx].slot[ACTOR_ITEM_LEFT] = (sint8)slot;
                model->pos.y.bits = 1;
            }
        }
    }
    return npc_start_animation(idx, table, 127);
}

GDB_CALL sint32 npc_wait_captor_release(sint32 idx)
{
    FUNCTION_MARKER(0x80052f70u, "GAME.EXE");
    sint32 type = actor_types[idx];
    uint32 state = actor_animations[idx].id;
    uint32 table = animation_type_banks[(uint32)type];
    if (state == 171 && actor_contacts[idx].captor == -1)
        return npc_start_animation(idx, table, npc_behavior_actors[idx].idle_animation);
    return -1;
}

GDB_CALL intptr npc_update_action(sint32 idx)
{
    FUNCTION_MARKER(0x8004be84u, "GAME.EXE");
    uint32 state;
    sint32 age;
    npc_update_idx = (uint32)idx;
    age = (sint8)actor_lifecycle[idx].death_ticks;
    if (age)
    {
        age = (sint8)(age + 1);
        actor_lifecycle[idx].death_ticks = (uint8)age;
        if (age >= 61)
        {
            actor_animations[idx].src = (ACTOR_ANIMATION_SOURCE){ACTOR_ANIMATION_SOURCE_NONE, {0}};
            actor_types[idx] = ACTOR_TYPE_NONE;
            return -1;
        }
    }
    else if (actor_health[idx].hp <= 0)
        return actors_rec_death(idx);
    state = actor_animations[idx].id;
    if ((uint32)idx >= NPC_BEHAVIOR_ACTOR_COUNT)
        diagnostic_stop(__FUNCTION__, __FILE__, __LINE__);
    NPC_ACTION_BANK_REFERENCE bank = npc_action_actors[idx];
    uint32 action_idx = (bank.first + state) & 0x3fffffff;
    if (!bank.actions || action_idx >= NPC_ACTION_SLOT_COUNT)
        diagnostic_stop(__FUNCTION__, __FILE__, __LINE__);
    return npc_action_call(bank.actions[action_idx], idx);
}

static sint32 npc_finished_to(sint32 idx, sint32 action)
{
    uint32 type = (uint32)actor_types[idx];
    uint32 frame = actor_animations[idx].frame;
    uint32 table = animation_type_banks[type];
    if (frame != 0xffffffffu)
        return -1;
    return npc_start_animation(idx, table, action);
}

GDB_CALL sint32 npc_finish_to_idle(sint32 idx)
{
    FUNCTION_MARKER(0x8004bf3cu, "GAME.EXE");
    return npc_finished_to(idx, 69);
}

GDB_CALL sint32 npc_finish_charge_start(sint32 idx)
{
    FUNCTION_MARKER(0x8004ed54u, "GAME.EXE");
    return npc_finished_to(idx, 275);
}

GDB_CALL sint32 npc_begin_attack_wait(sint32 idx)
{
    FUNCTION_MARKER(0x8004d20cu, "GAME.EXE");
    NPC_SPECIAL *special = &npc_behavior_actors[idx].special;
    sint32 type = actor_types[idx];
    uint32 frame = actor_animations[idx].frame;
    uint32 table = animation_type_banks[(uint32)type];
    special->attack.effect_height = 0x390000;
    if (frame != 0xffffffffu)
        return -1;
    special->attack.rest_until = game_session.tick + 20u;
    return npc_start_animation(idx, table, 266);
}

GDB_CALL sint32 npc_update_delay(sint32 idx)
{
    FUNCTION_MARKER(0x8004e3d8u, "GAME.EXE");
    uint32 table = animation_type_banks[(uint32)actor_types[idx]], result;
    if (actor_lifecycle[idx].delay <= 0)
        return npc_continue_alternate_combat(idx);
    if (actor_animations[idx].frame == 0xffffffffu)
        npc_start_animation(idx, table, actor_animations[idx].id);
    result = (uint16)actor_lifecycle[idx].delay - 1u;
    actor_lifecycle[idx].delay = (uint16)result;
    result <<= 16;
    return result ? (sint32)result : npc_continue_alternate_combat(idx);
}

GDB_CALL sint32 npc_begin_charge_loop(sint32 idx)
{
    FUNCTION_MARKER(0x8004ece0u, "GAME.EXE");
    sint32 type = actor_types[idx];
    uint32 frame = actor_animations[idx].frame;
    uint32 table = animation_type_banks[(uint32)type];
    if (frame != 0xffffffff)
        return -1;
    npc_start_animation(idx, table, 274);
    return sound_voice_play(187, 255, 4096);
}

GDB_CALL sint32 npc_update_knockdown_pose(sint32 idx)
{
    FUNCTION_MARKER(0x80053f4cu, "GAME.EXE");
    sint32 type = actor_types[idx];
    uint32 frame = actor_animations[idx].frame;
    uint32 table = animation_type_banks[(uint32)type];
    if (frame != 0xffffffffu)
        return 156;
    return npc_start_animation(idx, table, actor_animations[idx].id == 156 ? 253 : 252);
}

static sint32 npc_after_hold(sint32 idx)
{
    uint32 type = (uint32)actor_types[idx];
    uint32 frame = actor_animations[idx].frame;
    uint32 table = animation_type_banks[type];
    if (frame != 0xffffffffu)
        return -1;
    if (actor_contacts[idx].captive == -1)
        return npc_update_ranged_attack(idx);
    return npc_start_animation(idx, table, 11);
}

GDB_CALL sint32 npc_finish_captive_hold0(sint32 idx)
{
    FUNCTION_MARKER(0x800547b8u, "GAME.EXE");
    return npc_after_hold(idx);
}

GDB_CALL sint32 npc_finish_captive_hold1(sint32 idx)
{
    FUNCTION_MARKER(0x80054900u, "GAME.EXE");
    return npc_after_hold(idx);
}

GDB_CALL sint32 npc_choose_animation_variant(sint32 idx)
{
    FUNCTION_MARKER(0x8005531cu, "GAME.EXE");
    sint32 type = actor_types[idx];
    uint32 frame = actor_animations[idx].frame;
    uint32 table = animation_type_banks[(uint32)type];
    if (frame != 0xffffffffu)
        return -1;
    return npc_start_animation(idx, table, (sint32)(random_next() & 3u) + 243);
}

GDB_CALL sint32 npc_set_stun_recovery_deadline(sint32 idx)
{
    FUNCTION_MARKER(0x80055574u, "GAME.EXE");
    if (actor_animations[idx].frame != 0xffffffffu)
        return -1;
    uint32 duration = ((uint16)actor_health[idx].stun);
    uint32 tick = game_session.tick;
    sint32 delay = (sint32)(duration << 16) >> 18;
    actor_status[idx].deadline = tick + (uint32)delay;
    return npc_update_combat(idx);
}

GDB_CALL intptr npc_update_companion_wait(sint32 idx)
{
    FUNCTION_MARKER(0x80056530u, "GAME.EXE");
    sint32 count = actor_lifecycle[idx].delay;
    if (count)
        actor_lifecycle[idx].delay = (uint16)(count - 1);
    if (actor_companions[idx] == -1)
        return npc_update_actor(idx);
    return -1;
}

GDB_CALL sint32 npc_resume_approach0(sint32 idx)
{
    FUNCTION_MARKER(0x8004c134u, "GAME.EXE");
    if (actor_animations[idx].frame == 0xffffffffu)
        return npc_approach_target(idx);
    return -1;
}

GDB_CALL sint32 npc_resume_approach1(sint32 idx)
{
    FUNCTION_MARKER(0x8004c180u, "GAME.EXE");
    if (actor_animations[idx].frame == 0xffffffffu)
        return npc_approach_target(idx);
    return -1;
}

GDB_CALL sint32 npc_resume_combat0(sint32 idx)
{
    FUNCTION_MARKER(0x8004d288u, "GAME.EXE");
    if (actor_animations[idx].frame == 0xffffffffu)
        return npc_update_combat(idx);
    return -1;
}

GDB_CALL sint32 npc_resume_alternate_combat5(sint32 idx)
{
    FUNCTION_MARKER(0x8004e38cu, "GAME.EXE");
    if (actor_animations[idx].frame == 0xffffffffu)
        return npc_continue_alternate_combat(idx);
    return -1;
}

GDB_CALL sint32 npc_resume_alternate_combat0(sint32 idx)
{
    FUNCTION_MARKER(0x8004ec48u, "GAME.EXE");
    if (actor_animations[idx].frame == 0xffffffffu)
        return npc_update_alternate_combat(idx);
    return -1;
}

GDB_CALL sint32 npc_resume_alternate_combat1(sint32 idx)
{
    FUNCTION_MARKER(0x8004ec94u, "GAME.EXE");
    if (actor_animations[idx].frame == 0xffffffffu)
        return npc_update_alternate_combat(idx);
    return -1;
}

GDB_CALL sint32 npc_resume_alternate_combat2(sint32 idx)
{
    FUNCTION_MARKER(0x8004ef18u, "GAME.EXE");
    if (actor_animations[idx].frame == 0xffffffffu)
        return npc_update_alternate_combat(idx);
    return -1;
}

GDB_CALL sint32 npc_resume_alternate_combat3(sint32 idx)
{
    FUNCTION_MARKER(0x8004ef6cu, "GAME.EXE");
    if (actor_animations[idx].frame == 0xffffffffu)
        return npc_update_alternate_combat(idx);
    return -1;
}

GDB_CALL sint32 npc_resume_alternate_combat4(sint32 idx)
{
    FUNCTION_MARKER(0x8004f038u, "GAME.EXE");
    if (actor_animations[idx].frame == 0xffffffffu)
        return npc_update_alternate_combat(idx);
    return -1;
}

GDB_CALL sint32 npc_resume_combat1(sint32 idx)
{
    FUNCTION_MARKER(0x80053ca0u, "GAME.EXE");
    if (actor_animations[idx].frame == 0xffffffffu)
        return npc_update_combat(idx);
    return -1;
}

GDB_CALL sint32 npc_resume_combat2(sint32 idx)
{
    FUNCTION_MARKER(0x80053dd0u, "GAME.EXE");
    if (actor_animations[idx].frame == 0xffffffffu)
        return npc_update_combat(idx);
    return -1;
}

GDB_CALL sint32 npc_resume_combat3(sint32 idx)
{
    FUNCTION_MARKER(0x800553a4u, "GAME.EXE");
    if (actor_animations[idx].frame == 0xffffffffu)
        return npc_update_combat(idx);
    return -1;
}

GDB_CALL sint32 npc_resume_combat4(sint32 idx)
{
    FUNCTION_MARKER(0x80055528u, "GAME.EXE");
    if (actor_animations[idx].frame == 0xffffffffu)
        return npc_update_combat(idx);
    return -1;
}

GDB_CALL sint32 npc_resume_combat5(sint32 idx)
{
    FUNCTION_MARKER(0x80055918u, "GAME.EXE");
    if (actor_animations[idx].frame == 0xffffffffu)
        return npc_update_combat(idx);
    return -1;
}

static uint32 approach_can_stop(sint32 idx)
{
    FUNCTION_MARKER(0x8004c4a4u, "GAME.EXE");
    uint32 action;
    if (actor_animations[idx].frame == 0xffffffffu)
        return 1;
    action = actor_animations[idx].id;
    return action == 82u || action == 87u;
}

/* 4BFA0: approach thresholds depend on current action (216 / 264). */
GDB_CALL sint32 npc_approach_target(sint32 idx)
{
    FUNCTION_MARKER(0x8004bfa0u, "GAME.EXE");
    uint32 target, table;
    sint32 type, x0, z0, x1, z1, distance, result;
    type = actor_types[idx];
    target = npc_targets[idx].idx;
    table = animation_type_banks[(uint32)type];
    if (target == 0xffffffffu)
    {
        result = approach_can_stop(idx);
        if (!result)
            return result;
        return npc_start_animation(idx, table, 69);
    }
    const FIXED_VEC3 *target_pos = actor_positions + target;
    x0 = actor_positions[idx].x.val;
    z0 = actor_positions[idx].z.val;
    x1 = target_pos->x.val;
    z1 = target_pos->z.val;
    distance = (sint32)math_distance_approx(x0, z0, x1, z1) >> 16;
    x1 = target_pos->x.val;
    z1 = target_pos->z.val;
    npc_turn_to_point(idx, x1, z1);
    if (distance < (actor_animations[idx].id ? 216 : 264))
    {
        if ((uint32)idx >= NPC_BEHAVIOR_ACTOR_COUNT)
            diagnostic_stop(__FUNCTION__, __FILE__, __LINE__);
        npc_behavior_actors[idx].command = 0;
        npc_targets[idx].role = 3;
        return npc_start_animation(idx, table, 0);
    }
    if (distance < 640)
    {
        if (actor_animations[idx].frame != 0xffffffffu && !animation_matches_stream(idx, 69))
            return 0;
        return npc_start_animation(idx, table, 82);
    }
    if (actor_animations[idx].frame != 0xffffffffu)
        return -1;
    return npc_start_animation(idx, table, (animation_stream(table, 87).kind != ACTOR_ANIMATION_SOURCE_NONE) ? 87 : 82);
}

/* 4F084: preserve v0 after distance tests, not just the movement status. */
GDB_CALL sint32 npc_update_alternate_combat(sint32 idx)
{
    /* TODO: Investigate target owner outside its native pool */
    if (idx < 0 || idx >= ACTOR_COUNT)
    {
        DIAGNOSTIC(fprintf(stderr, "Invalid NPC target owner value %u actor %d stage %u tick %u\n", (uint32)idx, idx, game_session.stage, game_session.tick));
        diagnostic_stop(__FUNCTION__, __FILE__, __LINE__);
    }
    FUNCTION_MARKER(0x8004f084u, "GAME.EXE");
    sint32 type = actor_types[idx];
    uint32 flags = actor_animations[idx].motion.events;
    uint32 table = animation_type_banks[(uint32)type];
    uint32 target, mode, other_idx, desired, x, z;
    sint32 old, result, slot, item, moved, distance, x0, z0, x1, z1;
    const FIXED_VEC3 *other_pos;
    if (flags & 0x4000)
        sound_voice_play(17, 240, 6144);
    old = npc_base_animation(idx);
    npc_update_target(idx);
    target = npc_targets[idx].idx;
    if (target != 0xffffffffu)
    {
        mode = npc_targets[idx].role;
        if (mode == 2)
        {
            other_idx = target;
            /* TODO: Investigate actor target outside its native pool */
            if (other_idx >= ACTOR_COUNT)
            {
                DIAGNOSTIC(fprintf(stderr, "Invalid NPC actor target value %u actor %d stage %u tick %u\n", (uint32)other_idx, idx, game_session.stage, game_session.tick));
                diagnostic_stop(__FUNCTION__, __FILE__, __LINE__);
            }
            other_pos = actor_positions + other_idx;
            x = other_pos->x.bits;
            z = other_pos->z.bits;
            result = npc_steer_to_point(idx, x, z, 262);
            if (result)
                return result;
        }
        else if (mode == 1)
        {
            slot = actor_items[idx].slot[ACTOR_ITEM_RIGHT];
            if (slot != -1)
            {
                /* TODO: Investigate held item outside its native pool */
                if (slot < 0 || slot >= DYNAMIC_OBJECT_COUNT)
                {
                    DIAGNOSTIC(fprintf(stderr, "Invalid NPC held item value %u actor %d stage %u tick %u\n", (uint32)slot, idx, game_session.stage, game_session.tick));
                    diagnostic_stop(__FUNCTION__, __FILE__, __LINE__);
                }
            }
            item = slot == -1 ? -1 : (sint32)dynamic_objects[slot].kind;
            if (item == -1)
            {
                slot = actor_items[idx].slot[ACTOR_ITEM_LEFT];
                if (slot != -1)
                {
                    /* TODO: Investigate held item outside its native pool */
                    if (slot < 0 || slot >= DYNAMIC_OBJECT_COUNT)
                    {
                        DIAGNOSTIC(fprintf(stderr, "Invalid NPC held item value %u actor %d stage %u tick %u\n", (uint32)slot, idx, game_session.stage, game_session.tick));
                        diagnostic_stop(__FUNCTION__, __FILE__, __LINE__);
                    }
                }
                if (slot != -1)
                    item = (sint32)dynamic_objects[slot].kind;
            }
            other_idx = npc_targets[idx].idx;
            /* TODO: Investigate actor target outside its native pool */
            if (other_idx >= ACTOR_COUNT)
            {
                DIAGNOSTIC(fprintf(stderr, "Invalid NPC actor target value %u actor %d stage %u tick %u\n", (uint32)other_idx, idx, game_session.stage, game_session.tick));
                diagnostic_stop(__FUNCTION__, __FILE__, __LINE__);
            }
            other_pos = actor_positions + other_idx;
            if (item == -1)
            {
                x = other_pos->x.bits;
                z = other_pos->z.bits;
                moved = npc_steer_to_point(idx, x, z, 257);
                other_idx = npc_targets[idx].idx;
                x0 = actor_positions[idx].x.whole;
                z0 = actor_positions[idx].z.whole;
                /* TODO: Investigate late actor target outside its native pool */
                if (other_idx >= ACTOR_COUNT)
                {
                    DIAGNOSTIC(fprintf(stderr, "Invalid NPC late actor target value %u actor %d stage %u tick %u\n", (uint32)other_idx, idx, game_session.stage, game_session.tick));
                    diagnostic_stop(__FUNCTION__, __FILE__, __LINE__);
                }
                other_pos = actor_positions + other_idx;
                x1 = other_pos->x.whole;
                z1 = other_pos->z.whole;
                distance = math_distance_approx(x0, z0, x1, z1);
                result = distance < 641;
                if (distance >= 1281)
                {
                    result = npc_start_animation(idx, table, 273);
                    moved = 4;
                }
                else if (distance >= 641)
                {
                    npc_start_animation(idx, table, 270);
                    result = sound_voice_play(179, 192, 6144);
                    moved = 4;
                }
            }
            else
            {
                x = other_pos->x.bits;
                z = other_pos->z.bits;
                result = npc_steer_to_point(idx, x, z, 261);
                moved = result;
            }
            if (!moved)
                return npc_update_ranged_attack(idx);
            return result;
        }
        else if (mode == 4)
        {
            other_idx = target;
            /* TODO: Investigate item target outside its native pool */
            if (other_idx >= DYNAMIC_OBJECT_COUNT)
            {
                DIAGNOSTIC(fprintf(stderr, "Invalid NPC item target value %u actor %d stage %u tick %u\n", (uint32)other_idx, idx, game_session.stage, game_session.tick));
                diagnostic_stop(__FUNCTION__, __FILE__, __LINE__);
            }
            other_pos = &dynamic_objects[other_idx].pos;
            x = other_pos->x.bits;
            z = other_pos->z.bits;
            result = npc_steer_to_point(idx, x, z, 256);
            if (result)
                return result;
            other_idx = npc_targets[idx].idx;
            /* TODO: Investigate late item target outside its native pool */
            if (other_idx >= DYNAMIC_OBJECT_COUNT)
            {
                DIAGNOSTIC(fprintf(stderr, "Invalid NPC late item target value %u actor %d stage %u tick %u\n", (uint32)other_idx, idx, game_session.stage, game_session.tick));
                diagnostic_stop(__FUNCTION__, __FILE__, __LINE__);
            }
            item = (sint32)dynamic_objects[other_idx].kind;
            if (item >= 522)
                return npc_start_animation(idx, table, 121);
            if (item == 509 || (uint32)item - 507u < 2u || item == 506)
                return npc_start_animation(idx, table, 123);
            return npc_start_animation(idx, table, item == 515 || item == 504 ? 126 : 118);
        }
    }
    desired = npc_behavior_actors[idx].idle_animation;
    if ((uint32)old == desired)
    {
        result = (sint32)actor_animations[idx].frame;
        if (result != -1)
            return result;
    }
    return npc_start_animation(idx, table, desired);
}

/* IDA misses forwarded a0 in this call-only entry. */
GDB_CALL sint32 npc_continue_alternate_combat(sint32 idx)
{
    FUNCTION_MARKER(0x8004f000u, "GAME.EXE");
    return npc_update_alternate_combat(idx);
}

#define npc_wip_fatal(pc) \
    do \
    { \
        diagnostic_branch((pc), "npc", __FUNCTION__, __FILE__, __LINE__, 0, "abort"); \
        npc_wip_abort(pc); \
    } while (0)

static void npc_wip_abort(uint32 pc)
{
    DIAGNOSTIC(fprintf(stderr, "WIP NPC branch %08X\n", pc));
    abort();
}

/* 54474: completed recovery150 turns around, then resumes normal NPC logic. */
GDB_CALL sint32 npc_finish_recovery_turn(sint32 idx)
{
    FUNCTION_MARKER(0x80054474u, "GAME.EXE");
    if (actor_animations[idx].frame != 0xffffffffu)
        return 150;
    if (actor_animations[idx].id == 150)
        actor_orientation[idx].yaw = ((uint16)actor_orientation[idx].yaw) ^ 1024;
    return npc_update_combat(idx);
}

/* 54398: signed deadline; loop animation while waiting, then recover. */
GDB_CALL sint32 npc_wait_recovery_deadline(sint32 idx)
{
    FUNCTION_MARKER(0x80054398u, "GAME.EXE");
    uint32 table = animation_type_banks[(uint32)actor_types[idx]];
    sint32 state = npc_base_animation(idx);
    sint32 deadline = ((sint32)actor_status[idx].deadline);
    sint32 tick = (sint32)game_session.tick;
    if (tick < deadline)
    {
        if (actor_animations[idx].frame == 0xffffffffu)
            return animation_start(idx, table, state);
        return -1;
    }
    return npc_start_animation(idx, table, state == 253 ? 150 : (sint32)((random_next() & 2u) | 149u));
}

GDB_CALL sint32 npc_base_animation(sint32 idx)
{
    FUNCTION_MARKER(0x8004c9b0, "GAME.EXE");
    const NPC_BEHAVIOR_ACTOR *behavior = npc_behavior_actors + idx;
    sint32 state = (sint32)actor_animations[idx].id;
    switch (state)
    {
        case 79:
        case 80:
        case 81:
        case 70:
        case 73:
        case 74:
        case 76:
            return (sint32)behavior->idle_animation;
        case 85:
        case 83:
            return (sint32)behavior->move_animation;
        case 100:
        case 102:
            return 98;
        case 105:
        case 107:
            return 103;
        case 115:
        case 117:
            return 113;
        case 110:
        case 112:
            return 108;
        case 161:
            return 159;
        case 162:
            return 160;
        case 61:
            return 57;
        case 63:
            return 59;
        case 64:
            return 60;
        case 62:
            return 58;
        case 49:
            return 36;
        default:
            return state;
    }
}

GDB_CALL sint32 npc_start_animation(sint32 idx, ANIMATION_BANK_ID table, sint32 state)
{
    FUNCTION_MARKER(0x8004c4d8, "GAME.EXE");
    /* TODO: Investigate NPC item operations outside the native actor pool */
    if (idx < 0 || idx >= ACTOR_COUNT)
    {
        DIAGNOSTIC(fprintf(stderr, "Invalid NPC item actor %d in %s stage %u tick %u\n", idx, __FUNCTION__, game_session.stage, game_session.tick));
        diagnostic_stop(__FUNCTION__, __FILE__, __LINE__);
    }
    NPC_BEHAVIOR_ACTOR *behavior = npc_behavior_actors + idx;
    sint32 slot, deadline, base_state;
    uint32 type, tick;
    if ((state == 113 || state == 117) && behavior->transition_state == 108)
    {
        deadline = (sint32)behavior->transition_until;
        tick = game_session.tick;
        if ((sint32)tick < deadline)
            state = (sint32)behavior->idle_animation;
    }
    if ((state == 108 || state == 112) && behavior->transition_state == 113)
    {
        deadline = (sint32)behavior->transition_until;
        tick = game_session.tick;
        if ((sint32)tick < deadline)
            state = (sint32)behavior->idle_animation;
    }
    if (actor_types[idx] == 12 && state == 36)
        state = 49;
    slot = actor_items[idx].slot[ACTOR_ITEM_RIGHT];
    if (slot != -1)
    {
        /* TODO: Investigate a held NPC item outside the native object pool */
        if (slot < 0 || slot >= DYNAMIC_OBJECT_COUNT)
        {
            DIAGNOSTIC(fprintf(stderr, "Invalid NPC animation item actor %d slot %d state %d stage %u tick %u\n", idx, slot, state, game_session.stage, game_session.tick));
            diagnostic_stop(__FUNCTION__, __FILE__, __LINE__);
        }
        type = dynamic_objects[slot].kind;
        if (type - 499u < 5u)
        {
            if (state == (sint32)behavior->idle_animation && animation_stream(table, 79).kind != ACTOR_ANIMATION_SOURCE_NONE)
                state = 79;
        }
        else if (type == 477 || type == 491 || type == 493)
        {
            if (state == (sint32)behavior->idle_animation && animation_stream(table, 80).kind != ACTOR_ANIMATION_SOURCE_NONE)
                state = 80;
        }
        else if (type == 490)
        {
            if (state == (sint32)behavior->idle_animation && animation_stream(table, 81).kind != ACTOR_ANIMATION_SOURCE_NONE)
                state = 81;
        }
        else if (type == 509 || type - 507u < 2u || type == 506)
        {
            if (state == (sint32)behavior->idle_animation && animation_stream(table, 76).kind != ACTOR_ANIMATION_SOURCE_NONE)
                state = 76;
        }
        else if ((sint32)type >= 522)
        {
            if (state == (sint32)behavior->idle_animation && animation_stream(table, 70).kind != ACTOR_ANIMATION_SOURCE_NONE)
                state = 70;
            if (state == (sint32)behavior->move_animation && animation_stream(table, 85).kind != ACTOR_ANIMATION_SOURCE_NONE)
                state = 85;
            if (state == 87 && animation_stream(table, 85).kind != ACTOR_ANIMATION_SOURCE_NONE)
                state = 85;
            if (state == 98 && animation_stream(table, 102).kind != ACTOR_ANIMATION_SOURCE_NONE)
                state = 102;
            if (state == 103 && animation_stream(table, 107).kind != ACTOR_ANIMATION_SOURCE_NONE)
                state = 107;
            if (state == 113 && animation_stream(table, 117).kind != ACTOR_ANIMATION_SOURCE_NONE)
                state = 117;
            if (state == 108 && animation_stream(table, 112).kind != ACTOR_ANIMATION_SOURCE_NONE)
                state = 112;
            if (state == 159 && animation_stream(table, 161).kind != ACTOR_ANIMATION_SOURCE_NONE)
                state = 161;
            if (state == 160 && animation_stream(table, 162).kind != ACTOR_ANIMATION_SOURCE_NONE)
                state = 162;
        }
    }
    if (actor_items[idx].slot[ACTOR_ITEM_LEFT] != -1)
    {
        if (state == (sint32)behavior->idle_animation && animation_stream(table, 73).kind != ACTOR_ANIMATION_SOURCE_NONE)
            state = 73;
        if (state == (sint32)behavior->move_animation && animation_stream(table, 83).kind != ACTOR_ANIMATION_SOURCE_NONE)
            state = 83;
        if (state == 87 && animation_stream(table, 83).kind != ACTOR_ANIMATION_SOURCE_NONE)
            state = 83;
        if (state == 98 && animation_stream(table, 100).kind != ACTOR_ANIMATION_SOURCE_NONE)
            state = 100;
        if (state == 103 && animation_stream(table, 105).kind != ACTOR_ANIMATION_SOURCE_NONE)
            state = 105;
        if (state == 113 && animation_stream(table, 115).kind != ACTOR_ANIMATION_SOURCE_NONE)
            state = 115;
        if (state == 108 && animation_stream(table, 110).kind != ACTOR_ANIMATION_SOURCE_NONE)
            state = 110;
    }
    if (state == 87 && !(animation_stream(table, 87).kind != ACTOR_ANIMATION_SOURCE_NONE))
        state = (sint32)behavior->move_animation;
    if (state == 113 || state == 117)
    {
        tick = game_session.tick;
        behavior->transition_state = 113;
        behavior->transition_until = tick + 120;
    }
    if (state == 108 || state == 112)
    {
        tick = game_session.tick;
        behavior->transition_state = 108;
        behavior->transition_until = tick + 120;
    }
    if (actor_animations[idx].id == (uint32)state)
    {
        base_state = npc_base_animation(idx);
        if (base_state == (sint32)behavior->idle_animation && actor_animations[idx].frame != 0xffffffffu)
            return -1;
    }
    return animation_start(idx, table, state);
}

/* 4AF18 consumes seven arguments; IDA's eleven-argument draft is incorrect.
 * Stack slots +10/+14/+18 are flag117, timer80 and field8 respectively. */
GDB_CALL sint32 npc_create(sint32 type, uint32 x, uint32 z, sint32 yaw, sint32 flag, sint32 timer, uint32 extra)
{
    FUNCTION_MARKER(0x8004af18, "GAME.EXE");
    uint32 idx, hp, table, move_animation, color, idle_animation;
    sint32 stored_type;
    for (idx = 2; idx < ACTOR_COUNT; idx++)
    {
        if (actor_types[idx] == ACTOR_TYPE_NONE)
            break;
    }
    if (idx == ACTOR_COUNT)
        return -1;
    actor_animations[idx].src = (ACTOR_ANIMATION_SOURCE){ACTOR_ANIMATION_SOURCE_NONE, {0}};
    actor_animations[idx].frame = 0;
    actor_animations[idx].selector = 0;
    actor_animations[idx].id = 0;
    actor_animations[idx].motion = (ACTOR_FRAME_MOTION){0};
    actor_movement[idx] = (ACTOR_MOVEMENT){0};
    actor_positions[idx] = (FIXED_VEC3){0};
    actor_types[idx] = 0;
    actor_orientation[idx] = (ACTOR_ORIENTATION){0};
    actor_health[idx] = (ACTOR_HEALTH){0};
    actor_lifecycle[idx] = (ACTOR_LIFECYCLE){0};
    actor_animations[idx].hit_flags = 0;
    actor_animations[idx].event_step = 0;
    actor_combat[idx] = (ACTOR_COMBAT){0};
    actor_companions[idx] = 0;
    actor_pair_motion[idx] = (ACTOR_PAIR_MOTION){0};
    actor_trails[idx] = (ACTOR_TRAIL){0};
    col_regions.actor[idx] = (ACTOR_REGION_CONTACT){0};
    actor_contacts[idx] = (ACTOR_CONTACT){0};
    actor_items[idx].slot[ACTOR_ITEM_LEFT] = 0;
    actor_items[idx].slot[ACTOR_ITEM_RIGHT] = 0;
    item_pickup.pending[idx] = 0;
    actor_bonus_counts[idx] = 0;
    actor_bonus_totals[idx] = 0;
    npc_targets[idx] = (NPC_TARGET){0};
    actor_status[idx] = (ACTOR_STATUS){0};
    actor_lighting[idx].bits = 0;
    col_regions.actor[idx].region = NULL;
    character_actor_names[idx] = CHARACTER_NAME_NONE;
    npc_behavior_actors[idx] = (NPC_BEHAVIOR_ACTOR){0};
    npc_action_actors[idx] = (NPC_ACTION_BANK_REFERENCE){0};
    actor_types[idx] = (uint16)type;
    stored_type = actor_types[idx];
    actor_contacts[idx].captive = ACTOR_SLOT_NONE;
    actor_contacts[idx].captor = ACTOR_SLOT_NONE;
    actor_items[idx].slot[ACTOR_ITEM_LEFT] = -1;
    actor_items[idx].slot[ACTOR_ITEM_RIGHT] = -1;
    actor_status[idx].flags = 0;
    npc_behavior_actors[idx].command = 0;
    if (stored_type < 0 || stored_type >= NPC_BEHAVIOR_TYPE_COUNT)
        diagnostic_stop(__FUNCTION__, __FILE__, __LINE__);
    uint32 first = profile_first[stored_type];
    npc_behavior_actors[idx].profile = first == NPC_PROFILE_NONE ? (NPC_BEHAVIOR_REFERENCE){NULL, 0, 0} : (NPC_BEHAVIOR_REFERENCE){npc_behavior_rows, NPC_BEHAVIOR_ROW_COUNT, first};
    NPC_BEHAVIOR_REFERENCE profile = npc_behavior_actors[idx].profile;
    npc_behavior_actors[idx].progress.landing_item_idx = 0xffffffffu;
    if (!profile.rows || profile.first >= profile.row_count)
        diagnostic_stop(__FUNCTION__, __FILE__, __LINE__);
    hp = 8u * profile.rows[profile.first].health_units;
    actor_lifecycle[idx].spawn_flag = flag;
    actor_lifecycle[idx].death_ticks = 0;
    actor_lifecycle[idx].delay = (uint16)timer;
    actor_animations[idx].selector = extra;
    actor_health[idx].hp = (uint16)(hp - 4u);
    actor_movement[idx].van_speed = 0;
    actor_health[idx].shown = (uint16)hp;
    actor_health[idx].max = (uint16)hp;
    table = animation_type_banks[(uint32)type];
    actor_orientation[idx].yaw = (uint16)yaw;
    actor_positions[idx].x.bits = x;
    actor_positions[idx].y.bits = 0;
    actor_positions[idx].z.bits = z;
    actor_companions[idx] = ACTOR_SLOT_NONE;
    if (type == 17)
    {
        actor_animations[idx].src = (ACTOR_ANIMATION_SOURCE){ACTOR_ANIMATION_SOURCE_NONE, {0}};
        actor_animations[idx].id = 0xffffffffu;
        actor_animations[idx].frame = 0xffffffffu;
        npc_behavior_actors[idx].special.van_voice = 0;
        npc_behavior_actors[idx].fall_speed = 0;
    }
    else if (type == 51)
    {
        npc_start_animation(idx, table, 277);
        npc_behavior_actors[idx].special.bike.speed = 0;
        npc_behavior_actors[idx].fall_speed = 0;
        npc_behavior_actors[idx].special.bike.voice[0] = (uint32)sound_voice_play(143, 0, 6144) + 1u;
        npc_behavior_actors[idx].special.bike.voice[1] = (uint32)sound_voice_play(144, 0, 6144) + 1u;
    }
    else
        npc_start_animation(idx, table, 69);
    npc_reset_target_priorities(idx);
    type = actor_types[idx];
    idle_animation = base_animations[type].idle;
    type = actor_types[idx];
    npc_behavior_actors[idx].idle_animation = idle_animation;
    move_animation = base_animations[type].move;
    color = (uint8)actor_ambient_shade;
    actor_lighting[idx].b = color;
    actor_lighting[idx].g = color;
    actor_lighting[idx].r = color;
    npc_behavior_actors[idx].move_animation = move_animation;
    regions_apply_actor(idx);
    type = actor_types[idx];
    if (type < 0 || type >= CHARACTER_NAME_TYPE_COUNT)
        diagnostic_stop(__FUNCTION__, __FILE__, __LINE__);
    CHARACTER_NAME_ID name_ref = character_name_types[type];
    if (name_ref == CHARACTER_NAME_POOL0 || name_ref == CHARACTER_NAME_POOL1)
    {
        uint32 pool = name_ref == CHARACTER_NAME_POOL0 ? 0 : 1;
        uint32 start = pool ? CHARACTER_NAME_POOL1_START : 0;
        uint32 pos = character_name_cursor[pool];
        if (pos >= CHARACTER_NAME_POOL_WORD_COUNT - start)
            diagnostic_stop(__FUNCTION__, __FILE__, __LINE__);
        character_actor_names[idx] = character_name_pools[start + pos];
        ++pos;
        if (pos >= CHARACTER_NAME_POOL_WORD_COUNT - start)
            diagnostic_stop(__FUNCTION__, __FILE__, __LINE__);
        CHARACTER_NAME_ID next = character_name_pools[start + pos];
        character_name_cursor[pool] = pos;
        if (next == CHARACTER_NAME_NONE)
            character_name_cursor[pool] = 0;
    }
    else
        character_actor_names[idx] = name_ref;
    return idx;
}

/* Actual MIPS53034, including real expiry dispatch53278->53558. */
GDB_CALL sint32 npc_update_entry_delay(sint32 idx)
{
    FUNCTION_MARKER(0x80053034, "GAME.EXE");
    uint32 table = animation_type_banks[(uint32)actor_types[idx]];
    uint32 state = actor_animations[idx].id, y, timer;
    sint32 yaw;
    if (state == 287)
    {
        y = actor_positions[idx].y.bits + (lift.speed << 16);
        actor_positions[idx].y.bits = y;
        if ((sint32)y >= 0)
        {
            actor_lifecycle[idx].delay = 28;
            actor_positions[idx].y.bits = 0;
            return npc_start_animation(idx, table, 288);
        }
        actor_lifecycle[idx].delay = 2;
    }
    else if (state == 288)
    {
        yaw = actor_orientation[idx].yaw;
        if (yaw < 0 || yaw >= MATH_SIN_COUNT)
        {
            /* TODO: Investigate entry motion headings outside the sine table */
            DIAGNOSTIC(fprintf(stderr, "Invalid NPC entry sine heading %d actor %d stage %u tick %u\n", yaw, idx, game_session.stage, game_session.tick));
            diagnostic_stop(__FUNCTION__, __FILE__, __LINE__);
        }
        actor_positions[idx].x.bits += (uint32)(sint32)sin_table[yaw] << 5;
        if (yaw < -MATH_QUARTER_TURN || yaw >= MATH_ANGLE_COUNT)
        {
            /* TODO: Investigate entry motion headings outside the cosine view */
            DIAGNOSTIC(fprintf(stderr, "Invalid NPC entry cosine heading %d actor %d stage %u tick %u\n", yaw, idx, game_session.stage, game_session.tick));
            diagnostic_stop(__FUNCTION__, __FILE__, __LINE__);
        }
        actor_positions[idx].z.bits -= (uint32)(sint32)sin_table[MATH_QUARTER_TURN + yaw] << 5;
    }
    if (!actor_lifecycle[idx].delay)
        return npc_continue_combat(idx);
    if (actor_animations[idx].frame == 0xffffffffu)
    {
        state = actor_animations[idx].id;
        if (state == 282)
            state = 283;
        else if (state == 283)
            state = 284;
        else if (state == 284)
            state = npc_behavior_actors[idx].idle_animation;
        npc_start_animation(idx, table, state);
    }
    timer = (uint32)(uint16)actor_lifecycle[idx].delay - 1u;
    actor_lifecycle[idx].delay = (uint16)timer;
    timer <<= 16;
    if (!timer)
        return npc_continue_combat(idx);
    return timer;
}

GDB_CALL sint32 npc_turn_to_point(sint32 idx, uint32 x, uint32 z)
{
    FUNCTION_MARKER(0x800483bc, "GAME.EXE");
    uint32 yaw, difference;
    sint32 delta, step, actor_x, actor_z;
    npc_base_animation(idx);
    actor_x = actor_positions[idx].x.whole;
    actor_z = actor_positions[idx].z.whole;
    difference = (uint32)math_dir16(actor_x, actor_z, (sint32)x >> 16, (sint32)z >> 16);
    yaw = (uint16)actor_orientation[idx].yaw;
    difference -= yaw << 5;
    delta = (sint16)(uint16)difference;
    step = delta < -1024 ? -32 : delta > 1024 ? 32 : (delta >> 5);
    actor_orientation[idx].yaw = (uint16)(yaw + (uint32)step);
    yaw = (uint16)actor_orientation[idx].yaw;
    actor_orientation[idx].yaw = (uint16)(yaw & 2047);
    return delta;
}

/* Steer toward a Q16 point with obstacle probes and neighboring NPC avoidance */
/* Original6A0F8 calls have no effect: JR RA / NOP */
GDB_CALL sint32 npc_steer_to_point(sint32 idx, uint32 x, uint32 z, uint32 flags)
{
    FUNCTION_MARKER(0x80047CE4, "GAME.EXE");
    /* TODO: Investigate steering requested outside the native actor pool */
    if ((uint32)idx >= ACTOR_COUNT)
    {
        DIAGNOSTIC(fprintf(stderr, "Invalid steering actor %d flags %08X stage %u tick %u\n", idx, flags, game_session.stage, game_session.tick));
        diagnostic_stop(__FUNCTION__, __FILE__, __LINE__);
    }
    sint32 type = actor_types[idx];
    /* TODO: Investigate steering without a valid animation type */
    if ((uint32)type >= ANIMATION_TYPE_COUNT)
    {
        DIAGNOSTIC(fprintf(stderr, "Invalid steering type %d actor %d flags %08X stage %u tick %u\n", type, idx, flags, game_session.stage, game_session.tick));
        diagnostic_stop(__FUNCTION__, __FILE__, __LINE__);
    }
    uint32 table = animation_type_banks[(uint32)type];
    uint32 angle, dx0, dz0, dx1, dz1, mask, diff, weight, sum = 0, threshold;
    sint32 state, distance, nearby, base, i, fold, product;
    state = npc_base_animation(idx);
    if ((state == 113 || state == 108 || state == 98 || state == 103 || actor_animations[idx].id == 91) && actor_animations[idx].frame != 0xffffffffu)
    {
        npc_turn_to_point(idx, x, z);
        return 4;
    }
    distance = math_distance_approx(actor_positions[idx].x.whole, actor_positions[idx].z.whole, (sint32)x >> 16, (sint32)z >> 16);
    if (!distance)
        distance = 1;
    if (distance >= 129)
    {
        angle = ((uint32)(sint32)actor_orientation[idx].yaw - 176u) & 2047u;
        dx0 = (uint32)(sint32)sin_table[angle] << 8;
        dz0 = 0u - ((uint32)(sint32)sin_table[MATH_QUARTER_TURN + angle] << 8);
        mask = regions_find_obstacle(actor_positions[idx].x.bits + dx0, actor_positions[idx].z.bits + dz0) ? 2u : 0u;
        angle = ((uint32)(sint32)actor_orientation[idx].yaw + 176u) & 2047u;
        dx1 = (uint32)(sint32)sin_table[angle] << 8;
        dz1 = 0u - ((uint32)(sint32)sin_table[MATH_QUARTER_TURN + angle] << 8);
        if (regions_find_obstacle(actor_positions[idx].x.bits + dx1, actor_positions[idx].z.bits + dz1))
            mask |= 4u;
        if (mask)
        {
            flags &= ~4u;
            if (mask == 4 || mask == 5 || mask == 6)
            {
                x = actor_positions[idx].x.bits + dx0;
                z = actor_positions[idx].z.bits + dz0;
            }
            else
            {
                x = actor_positions[idx].x.bits + dx1;
                z = actor_positions[idx].z.bits + dz1;
            }
            npc_turn_to_point(idx, x, z);
        }
    }
    npc_turn_to_point(idx, x, z);
    if (!(flags & 256u) && (npc_behavior_actors[idx].transition_state != 32768u || (sint32)game_session.tick >= (sint32)npc_behavior_actors[idx].transition_until))
    {
        angle = (uint32)math_dir16(actor_positions[idx].x.whole, actor_positions[idx].z.whole, (sint32)x >> 16, (sint32)z >> 16);
        nearby = dir_distance;
        if (nearby < 640 && npc_targets[idx].role != 4 && npc_targets[idx].idx != 0xffffffffu)
        {
            base = nearby ? nearby : 1;
            for (i = 2; i < 8; i++)
            {
                if (i == idx || actor_types[i] == ACTOR_TYPE_NONE || actor_health[i].hp <= 0 || npc_targets[i].role == 4 || npc_targets[i].idx == 0xffffffffu)
                    continue;
                diff = angle - (uint32)math_dir16(actor_positions[idx].x.whole, actor_positions[idx].z.whole, actor_positions[i].x.whole, actor_positions[i].z.whole);
                fold = (sint16)(uint16)((diff & 0x8000u) ? diff - 16384u : 16384u - diff);
                weight = (uint32)base - (uint32)dir_distance;
                if ((sint32)weight < 0)
                    weight = 0;
                product = (uint32)fold * weight;
                if (base == -1 && (uint32)product == 0x80000000u)
                    npc_wip_fatal(0x80048100);
                sum += (uint32)(product / base);
            }
            mask = ((uint32)(uint16)actor_orientation[idx].yaw + 512u) & 2047u;
            dx0 = (uint32)(sint32)sin_table[mask] << 8;
            dz0 = 0u - ((uint32)(sint32)sin_table[MATH_QUARTER_TURN + mask] << 8);
            if ((sint32)sum < -768)
            {
                if (!regions_find_obstacle(actor_positions[idx].x.bits - dx0, actor_positions[idx].z.bits - dz0))
                {
                    npc_start_animation(idx, table, 108);
                    return 3;
                }
            }
            else if ((sint32)sum >= 769)
            {
                if (!regions_find_obstacle(actor_positions[idx].x.bits + dx0, actor_positions[idx].z.bits + dz0))
                {
                    npc_start_animation(idx, table, 113);
                    return 3;
                }
            }
        }
    }
    if (actor_types[idx] == 48 && distance >= 641 && npc_targets[idx].idx != 0xffffffffu)
        npc_start_animation(idx, table, 267);
    mask = flags & 3u;
    threshold = !mask ? 108u : mask == 1 ? ((flags & 256u) ? 344u : 216u) : mask == 2 ? 640u : 1280u;
    if ((sint32)game_session.tick >= (sint32)actor_status[idx].deadline && !(flags & 256u) && distance < (sint32)(threshold >> 1))
    {
        npc_start_animation(idx, table, 103);
        return 3;
    }
    if (distance < (sint32)threshold)
        return 0;
    if (distance < 640 && (flags & 4u))
    {
        if (actor_animations[idx].frame == 0xffffffffu || (uint32)state != npc_behavior_actors[idx].move_animation)
            npc_start_animation(idx, table, npc_behavior_actors[idx].move_animation);
        return 1;
    }
    if (actor_animations[idx].frame != 0xffffffffu)
        return 4;
    npc_start_animation(idx, table, (animation_stream(table, 87).kind != ACTOR_ANIMATION_SOURCE_NONE) ? 87 : (sint32)npc_behavior_actors[idx].move_animation);
    return 2;
}

/* Actual48488: reject grapple/attachment and unavailable target states. */
static sint32 target_available(sint32 idx)
{
    FUNCTION_MARKER(0x80048488, "GAME.EXE");
    sint32 state;
    if (actor_contacts[idx].captor != -1 || actor_contacts[idx].captive != -1)
        return 0;
    state = animation_movement_state(actor_animations[idx].id);
    return state != 33 && state != 158 && state != 39 && state != 149 && state != 150 && state != 151;
}

GDB_CALL sint32 npc_choose_action(sint32 idx)
{
    FUNCTION_MARKER(0x80048530, "GAME.EXE");
    uint32 table, cursor, next, state, frame, target = 0xffffffff;
    const NPC_AI_CAPABILITY *script;
    const uint32 *commands;
    uint32 capacity, choice;
    uint32 behavior, deadline, capability, tick, delay;
    sint32 type, slot, pick, count, random, eligible, result;
    /* TODO: Investigate the original out-of-pool NPC operation */
    if (idx < 0 || idx >= ACTOR_COUNT)
    {
        DIAGNOSTIC(fprintf(stderr, "Invalid NPC actor %d in %s stage %u tick %u\n", idx, __FUNCTION__, game_session.stage, game_session.tick));
        diagnostic_stop(__FUNCTION__, __FILE__, __LINE__);
    }
    type = actor_types[idx];
    behavior = npc_behavior_actors[idx].behavior;
    NPC_BEHAVIOR_REFERENCE profile = npc_behavior_actors[idx].profile;
    deadline = actor_status[idx].deadline;
    behavior = (profile.first + behavior) & NPC_BEHAVIOR_ROW_MASK;
    if (!profile.rows || behavior >= profile.row_count)
        diagnostic_stop(__FUNCTION__, __FILE__, __LINE__);
    capability = profile.rows[behavior].capability;
    table = animation_type_banks[(uint32)type];
    tick = game_session.tick;
    if (capability >= NPC_AI_CAPABILITY_COUNT)
        diagnostic_stop(__FUNCTION__, __FILE__, __LINE__);
    script = npc_ai_capabilities + capability;
    eligible = (sint32)tick >= (sint32)deadline && npc_targets[idx].role != 4;
    if (eligible)
    {
        target = npc_targets[idx].idx;
        eligible = target != 0xffffffff;
    }
    if (!eligible)
    {
        state = actor_animations[idx].id;
        next = npc_behavior_actors[idx].idle_animation;
        if (state == next && actor_animations[idx].frame != 0xffffffffu)
            return -1;
        return npc_start_animation(idx, table, next);
    }
    if (!target_available((sint32)target))
    {
        state = actor_animations[idx].id;
        next = npc_behavior_actors[idx].idle_animation;
        if (state == next)
        {
            frame = actor_animations[idx].frame;
            if (frame != 0xffffffffu)
                return frame;
        }
        return npc_start_animation(idx, table, next);
    }
    random = random_next() & 3u;
    cursor = npc_behavior_actors[idx].command;
    commands = script->code;
    capacity = script->code_capacity;
    if (!commands || cursor >= capacity)
        diagnostic_stop(__FUNCTION__, __FILE__, __LINE__);
    if (random < (sint32)commands[cursor])
        npc_behavior_actors[idx].command = cursor + 1u;
    else
        for (;;)
        {
            slot = actor_items[idx].slot[ACTOR_ITEM_RIGHT];
            if (slot == -1)
                slot = actor_items[idx].slot[ACTOR_ITEM_LEFT];
            random = (sint32)random_next();
            count = (sint32)script->choice_count;
            if (!count || (random == (-2147483647 - 1) && count == -1))
                npc_wip_fatal(0x800486bc);
            choice = random % count;
            const uint32 *choices = script->choices;
            if (!choices || choice >= script->choice_capacity)
                diagnostic_stop(__FUNCTION__, __FILE__, __LINE__);
            pick = (sint32)choices[choice];
            npc_behavior_actors[idx].command = (uint32)pick;
            if (pick < 0)
            {
                npc_behavior_actors[idx].command = (uint16)pick;
                if (slot == -1)
                    npc_behavior_actors[idx].command = 0xffffffffu;
                else
                {
                    /* TODO: Investigate the original out-of-pool NPC operation */
                    if (slot < 0 || slot >= DYNAMIC_OBJECT_COUNT)
                    {
                        DIAGNOSTIC(fprintf(stderr, "Invalid NPC command item actor %d slot %d in %s stage %u tick %u\n", idx, slot, __FUNCTION__, game_session.stage, game_session.tick));
                        diagnostic_stop(__FUNCTION__, __FILE__, __LINE__);
                    }
                    uint32 kind = dynamic_objects[slot].kind;
                    uint32 item_idx = kind - DYNAMIC_ITEM_TYPE_FIRST, bits;
                    if (item_idx < DYNAMIC_ITEM_TYPE_COUNT)
                        bits = item_properties[item_idx].packed;
                    else if (item_idx == DYNAMIC_ITEM_TYPE_COUNT)
                        bits = npc_ai_capabilities[0].choice_count;
                    else
                        diagnostic_stop(__FUNCTION__, __FILE__, __LINE__);
                    if (!(bits & ((uint32)(pick >> 16) & 0x3fffu)))
                        npc_behavior_actors[idx].command = 0xffffffffu;
                }
            }
            else if (slot != -1)
                npc_behavior_actors[idx].command = 0xffffffffu;
            else
            {
                commands = script->code;
                capacity = script->code_capacity;
                if (!commands || (uint32)pick >= capacity)
                    diagnostic_stop(__FUNCTION__, __FILE__, __LINE__);
                if (commands[(uint32)pick] == 166 && !actors_try_grapple(idx, npc_targets[idx].idx))
                    npc_behavior_actors[idx].command = 0xffffffffu;
            }
            if (npc_behavior_actors[idx].command != 0xffffffffu)
                break;
        }
    cursor = npc_behavior_actors[idx].command;
    npc_behavior_actors[idx].command = cursor + 1u;
    commands = script->code;
    capacity = script->code_capacity;
    if (!commands || cursor >= capacity)
        diagnostic_stop(__FUNCTION__, __FILE__, __LINE__);
    next = commands[cursor];
    if (next == 69)
    {
        state = actor_animations[idx].id;
        next = npc_behavior_actors[idx].idle_animation;
        if (state != next || actor_animations[idx].frame == 0xffffffffu)
            npc_start_animation(idx, table, next);
    }
    else
        npc_start_animation(idx, table, next);
    cursor = npc_behavior_actors[idx].command;
    npc_behavior_actors[idx].command = cursor + 1u;
    commands = script->code;
    tick = game_session.tick;
    capacity = script->code_capacity;
    if (!commands || cursor >= capacity)
        diagnostic_stop(__FUNCTION__, __FILE__, __LINE__);
    delay = commands[cursor];
    result = tick + delay;
    actor_status[idx].deadline = (uint32)result;
    return result;
}

GDB_CALL sint32 npc_update_ranged_attack(sint32 idx)
{
    FUNCTION_MARKER(0x800532b8, "GAME.EXE");
    /* TODO: Investigate the original out-of-pool NPC operation */
    if (idx < 0 || idx >= ACTOR_COUNT)
    {
        DIAGNOSTIC(fprintf(stderr, "Invalid NPC actor %d in %s stage %u tick %u\n", idx, __FUNCTION__, game_session.stage, game_session.tick));
        diagnostic_stop(__FUNCTION__, __FILE__, __LINE__);
    }
    ANIMATION_BANK_ID bank = animation_type_banks[(uint32)actor_types[idx]];
    uint32 target, type;
    uint32 x, z, tick, deadline;
    sint32 yaw, slot, ammo;
    npc_base_animation(idx);
    target = npc_targets[idx].idx;
    /* TODO: Investigate the original out-of-pool NPC operation */
    if (target >= ACTOR_COUNT)
    {
        DIAGNOSTIC(fprintf(stderr, "Invalid NPC facing target actor %d slot %u in %s stage %u tick %u\n", idx, target, __FUNCTION__, game_session.stage, game_session.tick));
        diagnostic_stop(__FUNCTION__, __FILE__, __LINE__);
    }
    x = actor_positions[target].x.bits;
    z = actor_positions[target].z.bits;
    yaw = npc_turn_to_point(idx, x, z);
    if ((uint16)((uint32)yaw + 4096) >= 8193)
    {
        actor_status[idx].deadline = game_session.tick + 5;
        return npc_choose_action(idx);
    }
    slot = actor_items[idx].slot[ACTOR_ITEM_RIGHT];
    if (slot != -1)
    {
        deadline = actor_status[idx].deadline;
        tick = game_session.tick;
        if ((sint32)tick >= (sint32)deadline && (uint16)((uint32)yaw + 2047) < 4095)
        {
            /* TODO: Investigate the original out-of-pool NPC operation */
            if (slot < 0 || slot >= DYNAMIC_OBJECT_COUNT)
            {
                DIAGNOSTIC(fprintf(stderr, "Invalid NPC firing item actor %d slot %d in %s stage %u tick %u\n", idx, slot, __FUNCTION__, game_session.stage, game_session.tick));
                diagnostic_stop(__FUNCTION__, __FILE__, __LINE__);
            }
            MODEL_OBJECT *item = dynamic_objects + slot;
            type = item->kind;
            if (type - 499 < 5)
            {
                /* TODO: Investigate an NPC item access with a foreign native owner */
                if (item->payload_kind != MODEL_PAYLOAD_PHYSICS)
                {
                    DIAGNOSTIC(fprintf(stderr, "Invalid NPC item owner actor %d slot %d kind %u owner %u phase ammo stage %u tick %u\n", idx, slot, item->kind, (uint32)item->payload_kind, game_session.stage, game_session.tick));
                    diagnostic_stop(__FUNCTION__, __FILE__, __LINE__);
                }
                ammo = item->physics.life;
                if (ammo <= 0)
                    return npc_start_animation(idx, bank, 159);
                item->physics.life = ammo - 1;
                return npc_start_animation(idx, bank, 270);
            }
            if (type == 506 || type == 509 || type - 507 < 2)
            {
                /* TODO: Investigate an NPC item access with a foreign native owner */
                if (item->payload_kind != MODEL_PAYLOAD_PHYSICS)
                {
                    DIAGNOSTIC(fprintf(stderr, "Invalid NPC item owner actor %d slot %d kind %u owner %u phase ammo stage %u tick %u\n", idx, slot, item->kind, (uint32)item->payload_kind, game_session.stage, game_session.tick));
                    diagnostic_stop(__FUNCTION__, __FILE__, __LINE__);
                }
                ammo = item->physics.life;
                if (ammo <= 0)
                    return npc_start_animation(idx, bank, 159);
                item->physics.life = ammo - 1;
                npc_start_animation(idx, bank, 195);
                actor_items_fire_projectile(idx);
                deadline = game_session.tick + 150;
                actor_status[idx].deadline = deadline;
                return deadline;
            }
        }
    }
    slot = actor_items[idx].slot[ACTOR_ITEM_LEFT];
    if (slot != -1)
    {
        deadline = actor_status[idx].deadline;
        tick = game_session.tick;
        if ((sint32)tick >= (sint32)deadline && (uint16)((uint32)yaw + 2047) < 4095)
        {
            /* TODO: Investigate the original out-of-pool NPC operation */
            if (slot < 0 || slot >= DYNAMIC_OBJECT_COUNT)
            {
                DIAGNOSTIC(fprintf(stderr, "Invalid NPC firing item actor %d slot %d in %s stage %u tick %u\n", idx, slot, __FUNCTION__, game_session.stage, game_session.tick));
                diagnostic_stop(__FUNCTION__, __FILE__, __LINE__);
            }
            MODEL_OBJECT *item = dynamic_objects + slot;
            if (item->kind == 515)
            {
                /* TODO: Investigate an NPC item access with a foreign native owner */
                if (item->payload_kind != MODEL_PAYLOAD_PHYSICS)
                {
                    DIAGNOSTIC(fprintf(stderr, "Invalid NPC item owner actor %d slot %d kind %u owner %u phase ammo stage %u tick %u\n", idx, slot, item->kind, (uint32)item->payload_kind, game_session.stage, game_session.tick));
                    diagnostic_stop(__FUNCTION__, __FILE__, __LINE__);
                }
                ammo = item->physics.life;
                if (ammo <= 0)
                    return npc_start_animation(idx, bank, 159);
                item->physics.life = ammo - 1;
                animation_start_held(idx, bank, 193);
                actor_items_fire_ray(idx);
                deadline = game_session.tick + 150;
                actor_status[idx].deadline = deadline;
                return deadline;
            }
        }
    }
    return npc_choose_action(idx);
}

/* 5392C: state270 starts the shot once, then waits for its animation. */
GDB_CALL sint32 npc_fire_spread_and_wait(sint32 idx)
{
    FUNCTION_MARKER(0x8005392cu, "GAME.EXE");
    sint32 type = actor_types[idx];
    uint32 frame = actor_animations[idx].frame;
    uint32 table = animation_type_banks[(uint32)type], deadline;
    if (frame != 0xffffffff)
        return -1;
    npc_start_animation(idx, table, 192);
    actor_items_fire_spread(idx);
    deadline = game_session.tick + 120;
    actor_status[idx].deadline = deadline;
    return deadline;
}

GDB_CALL sint32 npc_finish_attack_lighting(sint32 idx)
{
    FUNCTION_MARKER(0x80054a40u, "GAME.EXE");
    sint32 type = actor_types[idx];
    uint32 state = actor_animations[idx].id;
    uint32 table = animation_type_banks[(uint32)type];
    if ((state == 51 || state == 53) && (actor_animations[idx].motion.events & 0x3000u) == 0x3000u)
    {
        if ((uint32)idx >= ACTOR_COUNT)
            diagnostic_stop(__FUNCTION__, __FILE__, __LINE__);
        actor_lighting[idx].flags |= 8;
    }
    if (actor_animations[idx].frame != 0xffffffffu)
        return 28;
    state = actor_animations[idx].id;
    if (state == 28 && (table == ANIMATION_BANK_SHARED1 || table == ANIMATION_BANK_SHARED2 || table == ANIMATION_BANK_SHARED4 || table == ANIMATION_BANK_SHARED8))
        return npc_start_animation(idx, table, 149);
    return npc_update_ranged_attack(idx);
}

/* Original behavior selection and close-range attack dispatch */
GDB_CALL sint32 npc_update_combat(sint32 idx)
{
    FUNCTION_MARKER(0x80053558u, "GAME.EXE");
    /* TODO: Investigate target owner outside its native pool */
    if (idx < 0 || idx >= ACTOR_COUNT)
    {
        DIAGNOSTIC(fprintf(stderr, "Invalid NPC target owner value %u actor %d stage %u tick %u\n", (uint32)idx, idx, game_session.stage, game_session.tick));
        diagnostic_stop(__FUNCTION__, __FILE__, __LINE__);
    }
    uint32 table = animation_type_banks[(uint32)actor_types[idx]], target, role, other_idx, type, x, z, frame;
    sint32 state, result, slot, next;
    const FIXED_VEC3 *other_pos;
    state = npc_base_animation(idx);
    npc_update_target(idx);
    target = npc_targets[idx].idx;
    if (target != 0xffffffffu)
    {
        role = npc_targets[idx].role;
        if (role == 2)
        {
            other_idx = target;
            /* TODO: Investigate actor target outside its native pool */
            if (other_idx >= ACTOR_COUNT)
            {
                DIAGNOSTIC(fprintf(stderr, "Invalid NPC actor target value %u actor %d stage %u tick %u\n", (uint32)other_idx, idx, game_session.stage, game_session.tick));
                diagnostic_stop(__FUNCTION__, __FILE__, __LINE__);
            }
            other_pos = actor_positions + other_idx;
            x = other_pos->x.bits;
            z = other_pos->z.bits;
            result = npc_steer_to_point(idx, x, z, 6);
            if (result)
                return result;
        }
        else if (role == 1)
        {
            type = 0xffffffffu;
            slot = actor_items[idx].slot[ACTOR_ITEM_RIGHT];
            if (slot != -1)
            {
                /* TODO: Investigate held item outside its native pool */
                if (slot < 0 || slot >= DYNAMIC_OBJECT_COUNT)
                {
                    DIAGNOSTIC(fprintf(stderr, "Invalid NPC held item value %u actor %d stage %u tick %u\n", (uint32)slot, idx, game_session.stage, game_session.tick));
                    diagnostic_stop(__FUNCTION__, __FILE__, __LINE__);
                }
            }
            if (slot != -1)
                type = dynamic_objects[slot].kind;
            if (type == 0xffffffffu)
            {
                slot = actor_items[idx].slot[ACTOR_ITEM_LEFT];
                if (slot != -1)
                {
                    /* TODO: Investigate held item outside its native pool */
                    if (slot < 0 || slot >= DYNAMIC_OBJECT_COUNT)
                    {
                        DIAGNOSTIC(fprintf(stderr, "Invalid NPC held item value %u actor %d stage %u tick %u\n", (uint32)slot, idx, game_session.stage, game_session.tick));
                        diagnostic_stop(__FUNCTION__, __FILE__, __LINE__);
                    }
                }
                if (slot != -1)
                    type = dynamic_objects[slot].kind;
            }
            other_idx = npc_targets[idx].idx;
            /* TODO: Investigate actor target outside its native pool */
            if (other_idx >= ACTOR_COUNT)
            {
                DIAGNOSTIC(fprintf(stderr, "Invalid NPC actor target value %u actor %d stage %u tick %u\n", (uint32)other_idx, idx, game_session.stage, game_session.tick));
                diagnostic_stop(__FUNCTION__, __FILE__, __LINE__);
            }
            other_pos = actor_positions + other_idx;
            if (type - 499u < 5u || type == 515 || type == 506 || type == 509 || type - 507u < 2u || type - 521u >= 42u)
            {
                x = other_pos->x.bits;
                z = other_pos->z.bits;
                result = npc_steer_to_point(idx, x, z, 5);
                if (!result)
                    return npc_update_ranged_attack(idx);
                return result;
            }
            x = other_pos->x.bits;
            z = other_pos->z.bits;
            result = npc_steer_to_point(idx, x, z, 6);
            if (result)
                return result;
            other_idx = npc_targets[idx].idx;
            /* TODO: Investigate actor target outside its native pool */
            if (other_idx >= ACTOR_COUNT)
            {
                DIAGNOSTIC(fprintf(stderr, "Invalid NPC actor target value %u actor %d stage %u tick %u\n", (uint32)other_idx, idx, game_session.stage, game_session.tick));
                diagnostic_stop(__FUNCTION__, __FILE__, __LINE__);
            }
            other_pos = actor_positions + other_idx;
            x = other_pos->x.bits;
            z = other_pos->z.bits;
            result = ((uint32)npc_turn_to_point(idx, x, z) + 2047u) & 65535u;
            result = (uint32)result < 4095u;
            if (result)
                result = npc_start_animation(idx, table, 159);
            return result;
        }
        else if (role == 4)
        {
            other_idx = target;
            /* TODO: Investigate item target outside its native pool */
            if (other_idx >= DYNAMIC_OBJECT_COUNT)
            {
                DIAGNOSTIC(fprintf(stderr, "Invalid NPC item target value %u actor %d stage %u tick %u\n", (uint32)other_idx, idx, game_session.stage, game_session.tick));
                diagnostic_stop(__FUNCTION__, __FILE__, __LINE__);
            }
            other_pos = &dynamic_objects[other_idx].pos;
            x = other_pos->x.bits;
            z = other_pos->z.bits;
            result = npc_steer_to_point(idx, x, z, 0);
            if (result)
                return result;
            other_idx = npc_targets[idx].idx;
            /* TODO: Investigate late item target outside its native pool */
            if (other_idx >= DYNAMIC_OBJECT_COUNT)
            {
                DIAGNOSTIC(fprintf(stderr, "Invalid NPC late item target value %u actor %d stage %u tick %u\n", (uint32)other_idx, idx, game_session.stage, game_session.tick));
                diagnostic_stop(__FUNCTION__, __FILE__, __LINE__);
            }
            type = dynamic_objects[other_idx].kind;
            next = (sint32)type >= 522 ? 121 : type == 509 || type - 507u < 2u || type == 506 ? 123 : type == 515 || type == 504 ? 126 : 118;
            return npc_start_animation(idx, table, next);
        }
    }
    next = (sint32)npc_behavior_actors[idx].idle_animation;
    if (state == next)
    {
        frame = actor_animations[idx].frame;
        if (frame != 0xffffffffu)
            return frame;
    }
    return npc_start_animation(idx, table, next);
}

/* Forwarding wrapper; normalized twins differ in callee targets. */
GDB_CALL sint32 npc_continue_combat(sint32 idx)
{
    FUNCTION_MARKER(0x80053278u, "GAME.EXE");
    return npc_update_combat(idx);
}

/* 29530: death voice. Actual MIPS uses two RNG draws except fallback
 * voice selection, which consumes a third. No indexed reuse candidates. */

/* Hit reactions retain the original lethal, high-stun and directional paths */
GDB_CALL sint32 npc_attack_players(sint32 idx, sint32 attachment)
{
    FUNCTION_MARKER(0x80048dbc, "GAME.EXE");
    FIXED_VEC3 pos;
    uint32 x, z, table, angle, state, step;
    uint32 flags, tick, random;
    sint32 yaw, yaw_z;
    sint32 i, type, j, partner, victim_x, victim_z, src_x, src_z, range_type, distance, radius, s, c;
    sint16 dir;
    actor_attachment_pos(idx, attachment, pos.bits);
    for (i = 0; i < 2; i++)
    {
        if (i == idx)
            continue;
        type = actor_types[i];
        if (type == -1 || actor_animation_blocks_hits(actor_animations[i].id))
            continue;
        victim_x = actor_positions[i].x.whole;
        victim_z = actor_positions[i].z.whole;
        math_dir16(victim_x, victim_z, (sint32)pos.x.bits >> 16, (sint32)pos.z.bits >> 16);
        range_type = actor_types[i];
        distance = dir_distance;
        if (range_type < 0 || range_type >= ACTOR_TYPE_COUNT)
        {
            /* TODO: Investigate hit victims outside the collision radius table */
            DIAGNOSTIC(fprintf(stderr, "Invalid player hit radius type %d source %d victim %d stage %u tick %u\n", range_type, idx, i, game_session.stage, game_session.tick));
            diagnostic_stop(__FUNCTION__, __FILE__, __LINE__);
        }
        radius = actor_col.radius[range_type];
        if (radius < distance)
            continue;
        victim_x = actor_positions[i].x.whole;
        victim_z = actor_positions[i].z.whole;
        src_x = actor_positions[idx].x.whole;
        src_z = actor_positions[idx].z.whole;
        dir = (sint16)math_dir16(victim_x, victim_z, src_x, src_z);
        x = actor_positions[i].x.bits;
        angle = (uint16)actor_orientation[i].yaw;
        z = actor_positions[i].z.bits;
        dir = (angle << 5) - (uint32)(sint32)dir - 16384;
        dynamic_objects_spawn_transient(865, x, pos.y.bits, z);
        actor_items_drop_on_hit(i);
        sound_play_pain(i);
        angle = (uint32)(sint32)actor_orientation[i].yaw ^ 1024u;
        for (j = 0; j < 4; j++)
            effect_spawn_hit_reverse(x, pos.y.bits, z, angle);
        for (j = 0; j < 4; j++)
            effect_spawn_hit(x, pos.y.bits, z, angle);
        actor_trails[i].phase = 0;
        actor_apply_animation_damage(idx, i, actor_animations[idx].id);
        if (attachment == 3 && (uint32)((uint16)actor_types[idx]) - 40 < 4)
        {
            actor_health[i].stun = 127;
            sound_voice_play(185, 255, 6144);
            sound_voice_play(171, 255, 6144);
            flags = actor_status[i].flags;
            tick = game_session.tick;
            actor_status[i].flags = flags | 1;
            actor_status[i].deadline = tick + 20;
        }
        if (actor_health[i].hp <= 0)
        {
            sound_play_death_voice(i);
            actor_health[i].hp = 0;
            game_session.defeated_player = (uint32)i;
            actor_items_drop(i);
            animation_start(i, animation_type_banks[(uint32)type], dir < 0 ? 147 : 154);
            partner = actor_contacts[i].captive;
            if (partner != -1)
            {
                if (partner < 0 || partner >= ACTOR_COUNT)
                {
                    /* TODO: Investigate an invalid captive after a lethal player hit */
                    DIAGNOSTIC(fprintf(stderr, "Invalid lethal hit captive %d source %d victim %d stage %u tick %u\n", partner, idx, i, game_session.stage, game_session.tick));
                    diagnostic_stop(__FUNCTION__, __FILE__, __LINE__);
                }
                actor_contacts[partner].captor = ACTOR_SLOT_NONE;
                actor_contacts[i].captive = ACTOR_SLOT_NONE;
            }
            return 1;
        }
        if (actor_health[i].stun >= 49)
        {
            sound_play_heavy_hit_voice(idx);
            actor_health[i].stun = 0;
            actor_items_drop(i);
            table = animation_type_banks[(uint32)type];
            animation_start(i, table, dir < 0 ? 147 : 154);
        }
        else if (dir < 0)
        {
            state = actor_animations[idx].id;
            if (state == 11 || state == 40)
            {
                table = animation_type_banks[(uint32)type];
                animation_start(i, table, 139);
                step = state == 11 ? 64 : 112;
                yaw = actor_orientation[i].yaw;
                x = actor_positions[i].x.bits;
                yaw_z = actor_orientation[i].yaw;
                if (yaw < 0 || yaw >= MATH_SIN_COUNT)
                {
                    /* TODO: Investigate hit reaction headings outside the sine table */
                    DIAGNOSTIC(fprintf(stderr, "Invalid player hit sine heading %d source %d victim %d stage %u tick %u\n", yaw, idx, i, game_session.stage, game_session.tick));
                    diagnostic_stop(__FUNCTION__, __FILE__, __LINE__);
                }
                s = sin_table[yaw];
                actor_positions[i].x.bits = x - step * (uint32)s;
                if (yaw_z < -MATH_QUARTER_TURN || yaw_z >= MATH_ANGLE_COUNT)
                {
                    /* TODO: Investigate hit reaction headings outside the cosine view */
                    DIAGNOSTIC(fprintf(stderr, "Invalid player hit cosine heading %d source %d victim %d stage %u tick %u\n", yaw_z, idx, i, game_session.stage, game_session.tick));
                    diagnostic_stop(__FUNCTION__, __FILE__, __LINE__);
                }
                c = sin_table[MATH_QUARTER_TURN + yaw_z];
                z = actor_positions[i].z.bits;
                actor_positions[i].z.bits = z + step * (uint32)c;
            }
            else
            {
                random = random_next();
                table = animation_type_banks[(uint32)type];
                animation_start(i, table, (random & 1) + 133);
            }
        }
        else
        {
            random = random_next();
            table = animation_type_banks[(uint32)type];
            animation_start(i, table, (random & 1) | 152);
        }
        partner = actor_contacts[i].captive;
        if (partner != -1)
        {
            if (partner < 0 || partner >= ACTOR_COUNT)
            {
                /* TODO: Investigate an invalid captive after a player hit */
                DIAGNOSTIC(fprintf(stderr, "Invalid hit captive %d source %d victim %d stage %u tick %u\n", partner, idx, i, game_session.stage, game_session.tick));
                diagnostic_stop(__FUNCTION__, __FILE__, __LINE__);
            }
            actor_contacts[partner].captor = ACTOR_SLOT_NONE;
            actor_contacts[i].captive = ACTOR_SLOT_NONE;
        }
        return 1;
    }
    return 0;
}

/* 52944 entrance and event-free52FE8 dispatch. Other events/callbacks remain WIP. */
/* 53E1C: detach, wait for animation completion, then recover or die.
 * The no-partner/no-completion return is -16 (SLL in branch delay slot). */
GDB_CALL sint32 npc_update_detached(sint32 idx)
{
    FUNCTION_MARKER(0x80053e1c, "GAME.EXE");
    uint32 tick;
    sint32 type = actor_types[idx];
    sint32 partner = actor_contacts[idx].captive;
    uint32 table = animation_type_banks[(uint32)type];
    sint32 result = (uint32)partner << 4;
    if (partner != -1)
    {
        result = 244u * (uint32)partner;
        actor_contacts[partner].captor = ACTOR_SLOT_NONE;
        actor_contacts[idx].captive = ACTOR_SLOT_NONE;
    }
    if (actor_animations[idx].frame != 0xffffffffu)
        return result;
    if (actor_health[idx].hp <= 0)
    {
        npc_behavior_cycle.progress = npc_behavior_cycle.progress + 1u;
        actor_items_drop(idx);
        result = npc_start_animation(idx, table, 147);
        npc_targets[idx].idx = 0xffffffffu;
        return result;
    }
    if (actor_health[idx].stun < 33)
        return npc_update_combat(idx);
    npc_start_animation(idx, table, 103);
    tick = game_session.tick;
    npc_targets[idx].idx = 0xffffffffu;
    result = tick + 30;
    actor_status[idx].deadline = (uint32)result;
    return result;
}

/* 53FC4: knockdown bookkeeping and recovery transitions.
 * Evidence: status/gameplay/npc-53fc4-mips-audit.md. */
GDB_CALL sint32 npc_update_knockdown(sint32 idx)
{
    FUNCTION_MARKER(0x80053fc4, "GAME.EXE");
    uint32 table, count, behavior, factor;
    sint32 type, partner, state, hp, timer, product, divisor, result;
    uint16 angle;
    uint32 tick;
    type = actor_types[idx];
    count = npc_behavior_cycle.progress;
    table = animation_type_banks[(uint32)type];
    npc_behavior_cycle.progress = count + 1u;
    npc_targets[idx].idx = 0xffffffffu;
    state = npc_base_animation(idx);
    partner = actor_contacts[idx].captive;
    if (partner != -1)
    {
        actor_contacts[(uint32)partner].captor = ACTOR_SLOT_NONE;
        actor_contacts[idx].captive = ACTOR_SLOT_NONE;
    }
    result = (uint32)actor_items[idx].bits << 8;
    if (result != 0xffff00)
        result = actor_items_drop(idx);
    if (actor_animations[idx].frame != 0xffffffffu)
        return result;
    hp = actor_health[idx].hp;
    if (hp <= 0)
        return hp;
    timer = (sint32)actor_status[idx].deadline;
    tick = game_session.tick;
    if (timer < (sint32)tick)
    {
        if ((uint32)idx >= NPC_BEHAVIOR_ACTOR_COUNT)
            diagnostic_stop(__FUNCTION__, __FILE__, __LINE__);
        behavior = npc_behavior_actors[idx].behavior;
        NPC_BEHAVIOR_REFERENCE profile = npc_behavior_actors[idx].profile;
        npc_targets[idx].idx = 0xffffffffu; /* 540D8: repeat after the callees. */
        hp = actor_health[idx].hp;
        divisor = (sint32)actor_health[idx].max;
        behavior = (profile.first + behavior) & NPC_BEHAVIOR_ROW_MASK;
        if (!profile.rows || behavior >= profile.row_count)
            diagnostic_stop(__FUNCTION__, __FILE__, __LINE__);
        factor = profile.rows[behavior].recovery_delay;
        product = (sint32)factor * (divisor - hp);
        if (!divisor)
            npc_wip_fatal(0x80054110);
        if (divisor == -1 && (uint32)product == 0x80000000u)
            npc_wip_fatal(0x80054128);
        result = tick + (uint32)((product / divisor) + 1);
        actor_status[idx].deadline = (uint32)result;
        return result;
    }
    if (state == 0xb5)
    {
        angle = (uint16)actor_orientation[idx].yaw;
        actor_orientation[idx].yaw = angle ^ 0x400u;
        return npc_start_animation(idx, table, 0xfc);
    }
    if (state == 0xb7)
    {
        if (table == ANIMATION_BANK_SHARED1 || table == ANIMATION_BANK_SHARED2 || table == ANIMATION_BANK_SHARED4 || table == ANIMATION_BANK_SHARED8)
            return npc_update_combat(idx);
        return npc_start_animation(idx, table, 0xfd);
    }
    if (state == 0x8e)
    {
        if (table == ANIMATION_BANK_SHARED1 || table == ANIMATION_BANK_SHARED2 || table == ANIMATION_BANK_SHARED4 || table == ANIMATION_BANK_SHARED8)
            return npc_start_animation(idx, table, 0xfc);
        return npc_start_animation(idx, table, 0xfd);
    }
    if (state == 0x8c)
    {
        if (table == ANIMATION_BANK_SHARED1 || table == ANIMATION_BANK_SHARED2 || table == ANIMATION_BANK_SHARED4 || table == ANIMATION_BANK_SHARED8)
            return npc_start_animation(idx, table, 0xfc);
        actor_orientation[idx].yaw = (uint16)actor_orientation[idx].yaw ^ 0x400u;
        return npc_start_animation(idx, table, 0xfd);
    }
    if (state == 0x9a || ((uint32)state - 0xb6u < 2u) || state == 0xf1)
        return npc_start_animation(idx, table, 0xfd);
    if (state == 0x8d || state == 0xb8)
    {
        angle = (uint16)actor_orientation[idx].yaw;
        angle = ((uint32)(angle - 0x200u)) & 0x7ffu;
        actor_orientation[idx].yaw = angle;
        if (state == 0x8d)
            actor_orientation[idx].yaw = angle ^ 0x400u;
        return npc_start_animation(idx, table, 0xfc);
    }
    if (state == 0xbc || state == 0x94)
    {
        angle = (uint16)actor_orientation[idx].yaw;
        angle = ((uint32)(angle + 0x200u)) & 0x7ffu;
        actor_orientation[idx].yaw = angle;
        return npc_start_animation(idx, table, 0xfd);
    }
    if (state == 0xbd)
    {
        actor_orientation[idx].yaw = ((uint16)actor_orientation[idx].yaw - 0x200u) & 0x7ffu;
        return npc_start_animation(idx, table, 0xfc);
    }
    if (state == 0x119)
    {
        actor_positions[idx].y.bits = 0u;
        return npc_start_animation(idx, table, 0xfc);
    }
    return npc_start_animation(idx, table, 0xfc);
}

/* 55100: defensive animation chain. Indexed twin4E484 differs final callee;
 * this mapping follows current MIPS, including return58 on other endedstates. */
GDB_CALL sint32 npc_finish_combat_combo(sint32 idx)
{
    FUNCTION_MARKER(0x80055100u, "GAME.EXE");
    sint32 type = actor_types[idx];
    uint32 frame = actor_animations[idx].frame;
    uint32 table = animation_type_banks[(uint32)type];
    sint32 state;
    if (frame != 0xffffffffu)
        return -1;
    state = npc_base_animation(idx);
    if (state == 57)
    {
        actor_animations[idx].frame = 0xffffffffu;
        return npc_start_animation(idx, table, 60);
    }
    if (state == 60)
        return npc_start_animation(idx, table, 58);
    if (state == 59)
        return npc_start_animation(idx, table, 60);
    if (state == 58)
        return npc_update_combat(idx);
    return 58;
}

/* 55770: low-health stagger/recovery, actual signed timer comparison and
 * halfword HP addition. The return retains the pre-clamp HP value. */
GDB_CALL sint32 npc_update_recovery(sint32 idx)
{
    FUNCTION_MARKER(0x80055770, "GAME.EXE");
    uint32 table, state, frame, behavior, factor, deadline, tick;
    sint32 type, maximum, product, val;
    type = actor_types[idx];
    frame = actor_animations[idx].frame;
    table = animation_type_banks[(uint32)type];
    if (frame != 0xffffffffu)
        return -1;
    state = actor_animations[idx].id;
    if (state - 234u < 2u)
    {
        if ((uint32)idx >= NPC_BEHAVIOR_ACTOR_COUNT)
            diagnostic_stop(__FUNCTION__, __FILE__, __LINE__);
        behavior = npc_behavior_actors[idx].behavior;
        NPC_BEHAVIOR_REFERENCE profile = npc_behavior_actors[idx].profile;
        maximum = actor_health[idx].max;
        npc_targets[idx].idx = 0xffffffffu;
        val = actor_health[idx].hp;
        behavior = (profile.first + behavior) & NPC_BEHAVIOR_ROW_MASK;
        if (!profile.rows || behavior >= profile.row_count)
            diagnostic_stop(__FUNCTION__, __FILE__, __LINE__);
        factor = profile.rows[behavior].recovery_delay;
        product = factor * (uint32)(maximum - val);
        if (!maximum)
            npc_wip_fatal(0x80055818); /* Original BREAK7 division trap. */
        if (maximum == -1 && (uint32)product == 0x80000000u)
            npc_wip_fatal(0x80055830);
        actor_status[idx].deadline = game_session.tick + (uint32)(product / maximum) + 1u;
        return npc_start_animation(idx, table, 236);
    }
    if (state == 236)
        return npc_start_animation(idx, table, 237);
    if (state == 237)
        return npc_start_animation(idx, table, 238);
    if (state == 238)
    {
        deadline = actor_status[idx].deadline;
        tick = game_session.tick;
        if ((sint32)tick < (sint32)deadline)
            return npc_start_animation(idx, table, 238);
        npc_start_animation(idx, table, 239);
        val = (uint16)actor_health[idx].hp;
        maximum = actor_health[idx].max;
        val = (sint16)((uint32)val + 32u);
        actor_health[idx].hp = (uint16)val;
        if (maximum < val)
            actor_health[idx].hp = (uint16)maximum;
        return val;
    }
    if (state == 239)
        return npc_update_combat(idx);
    return 239;
}

/* 29684: body-impact sound. Exact type map and one RNG pitch draw. */

/* 332BC intentionally applies damage/attribution twice, around impact sound. */

/* 52C0C..52CE8: captured actor maintenance; continue at52CEC afterwards. */
static void npc_captured_update(sint32 idx, ANIMATION_BANK_ID table)
{
    uint32 state, deadline, tick;
    sint32 expired = 0;
    if (actor_items[idx].bits != 0xffff)
        actor_items_drop(idx);
    deadline = actor_status[idx].deadline;
    tick = game_session.tick;
    if ((sint32)tick >= (sint32)deadline)
    {
        uint8 counter = (uint8)(actor_contacts[idx].escape_ticks - 1u);
        actor_contacts[idx].escape_ticks = counter;
        expired = (counter & 128) != 0;
    }
    state = actor_animations[idx].id;
    if (expired && state == 171)
    {
        npc_start_animation(idx, table, 36);
        return;
    }
    if (state != 139)
        actors_update_grapple(actor_contacts[idx].captor, idx);
    if (actor_animations[idx].frame != 0xffffffffu)
        return;
    state = actor_animations[idx].id;
    if (state == 172)
        npc_start_animation(idx, table, 77);
    else if (state == 77)
        actor_animations[idx].frame = 0;
    else
        npc_start_animation(idx, table, 171);
}

GDB_CALL intptr npc_update_actor(sint32 idx)
{
    FUNCTION_MARKER(0x80052944, "GAME.EXE");
    uint32 events, point[16], table, x, z, state;
    sint32 type = actor_types[idx], effect_type = actor_types[idx], hp, result, age;
    npc_update_idx = (uint32)idx;
    table = animation_type_banks[(uint32)type];
    if (effect_type == 48 && !(actor_status[idx].flags & ACTOR_CULLED))
    {
        npc_build_attack_vectors(idx, 0, point);
        effect_spawn_motion(875, point[0], point[1], point[2], point[4], point[5], point[6]);
        effect_spawn_motion(875, point[8], point[9], point[10], point[12], point[13], point[14]);
    }
    age = (sint8)actor_lifecycle[idx].death_ticks;
    if (age)
    {
        /* 52A4C..52B44: signed byte death age; skip ordinary AI/health paths. */
        age = (sint8)(age + 1);
        actor_lifecycle[idx].death_ticks = (uint8)age;
        if (age >= 61)
        {
            uint16 phase = npc_drop_phase;
            uint32 kind;
            actor_animations[idx].src = (ACTOR_ANIMATION_SOURCE){ACTOR_ANIMATION_SOURCE_NONE, {0}};
            actor_types[idx] = ACTOR_TYPE_NONE;
            if (phase & 1u)
            {
                if (drop_pattern[(phase >> 1) & 7u])
                    kind = 590u + (random_next() & 1u);
                else
                    kind = (uint32)((sint32)random_next() % 4) + 593u;
                x = actor_positions[idx].x.bits;
                z = actor_positions[idx].z.bits;
                dynamic_objects_spawn(kind, x, 0, z);
            }
            ++npc_drop_phase;
        }
        if (actor_animations[idx].motion.events & 0x1000u)
        {
            uint32 event_state = actor_animations[idx].id & 0x3fffffff;
            if (event_state >= ACTOR_EVENT_SLOT_COUNT)
                diagnostic_stop(__FUNCTION__, __FILE__, __LINE__);
            actor_event_call(actor_events[event_state], idx);
            result = actor_animations[idx].event_step + 1;
            actor_animations[idx].event_step = (uint16)result;
            return result;
        }
        return 0; /* ANDI event1000 result on direct52F4C path. */
    }
    hp = actor_health[idx].hp;
    if (hp <= 0)
    {
        sint32 partner = actor_contacts[idx].captive;
        if (partner != -1)
        {
            actor_contacts[(uint32)partner].captor = ACTOR_SLOT_NONE;
            actor_contacts[idx].captive = ACTOR_SLOT_NONE;
        }
        return actors_rec_death(idx); /* 52B58..52B98: detach before death processing. */
    }
    if (type == 53 && !(game_session.tick & 3u) && hp < (sint32)actor_health[idx].max - 1)
        actor_health[idx].hp = (uint16)(hp + 1);
    if (actor_lifecycle[idx].delay > 0)
        return npc_update_entry_delay(idx);
    if (actor_contacts[idx].captor != -1)
        npc_captured_update(idx, table);
    events = actor_animations[idx].motion.events;
    if (events & 2048u)
        actor_animations[idx].hit_flags = 0;
    {
        uint32 bit;
        for (bit = 0; bit < 9; bit++)
            if (events & (1u << bit))
                if (npc_attack_players(idx, (sint32)bit + 1))
                    sound_play_strike_voice(idx);
    }
    if ((events & 0x400u) && npc_process_weapon_hits(idx))
        sound_voice_play((sint32)(random_next() & 1u) + 33, 255, 6144);
    /* 52EA4..52EE0: live impact callback precedes voice and eligibility. */
    if (events & 0x1000u)
    {
        uint32 event_state = actor_animations[idx].id & 0x3fffffff;
        if (event_state >= ACTOR_EVENT_SLOT_COUNT)
            diagnostic_stop(__FUNCTION__, __FILE__, __LINE__);
        actor_event_call(actor_events[event_state], idx);
        actor_animations[idx].event_step = (uint16)(actor_animations[idx].event_step + 1);
    }
    /* 52EE4: one RNG draw, type49 selects alternate sample bank/volume. */
    if (events & 0x4000u)
        sound_voice_play((type == 49 ? 87 : 52) + (sint32)(random_next() & 3u), type == 49 ? 255 : 96, 6144);
    result = npc_react_to_target(idx);
    if (result)
        return result;
    state = actor_animations[idx].id;
    if ((uint32)idx >= NPC_BEHAVIOR_ACTOR_COUNT)
        diagnostic_stop(__FUNCTION__, __FILE__, __LINE__);
    NPC_ACTION_BANK_REFERENCE bank = npc_action_actors[idx];
    uint32 action_idx = (bank.first + state) & 0x3fffffff;
    if (!bank.actions || action_idx >= NPC_ACTION_SLOT_COUNT)
        diagnostic_stop(__FUNCTION__, __FILE__, __LINE__);
    return npc_action_call(bank.actions[action_idx], idx);
}

static intptr npc_action_call(NPC_ACTION_REFERENCE callback, sint32 idx)
{
    if (callback)
    {
        const NPC_ACTION *action = callback;
        switch (action->signature)
        {
            case NPC_ACTION_SINT32:
                return action->fn.s32_idx(idx);
            case NPC_ACTION_UINT32:
                return (sint32)action->fn.u32_idx(idx);
            case NPC_ACTION_INTPTR:
                return action->fn.intptr_idx(idx);
            case NPC_ACTION_NONE:
                /* Preserve the original retained V0 as data, not a host pointer */
                action->fn.none();
                return action->retained_result;
        }
    }
    DIAGNOSTIC(fprintf(stderr, "Unknown NPC action=%p actor=%d stage=%u tick=%u\n", (const void *)callback, idx, game_session.stage, game_session.tick));
    diagnostic_stop(__FUNCTION__, __FILE__, __LINE__);
    return 0;
}

GDB_CALL sint32 npc_animation_allows_target_reaction(uint32 state)
{
    FUNCTION_MARKER(0x80054b44u, "GAME.EXE");
    return !(state <= 5u || state - 26u < 5u || state - 234u < 4u);
}

GDB_CALL sint32 npc_target_reaction_class(uint32 state)
{
    FUNCTION_MARKER(0x80054bc8u, "GAME.EXE");
    if (state <= 5u || state == 32 || state - 35u < 5u || state == 66)
        return 1;
    if (state == 24 || state == 25)
        return 2;
    if (state == 166)
        return 3;
    if (state == 26 || state == 27)
        return 4;
    return state == 67 ? 5 : 0;
}

/* 54C90 reaction eligibility; accepted angle/distance gates precede reaction. */
GDB_CALL sint32 npc_react_to_target(sint32 idx)
{
    FUNCTION_MARKER(0x80054c90, "GAME.EXE");
    uint32 target, angle, table, deadline, tick;
    sint32 state, reaction, type, x0, z0, x1, z1;
    if (game_settings.flags & GAME_FLAG_SKIP_TARGET_REACTION)
        return 0;
    /* TODO: Investigate a reaction requested outside the native actor pool */
    if ((uint32)idx >= ACTOR_COUNT)
    {
        DIAGNOSTIC(fprintf(stderr, "Invalid NPC reaction actor %d stage %u tick %u\n", idx, game_session.stage, game_session.tick));
        diagnostic_stop(__FUNCTION__, __FILE__, __LINE__);
    }
    type = actor_types[idx];
    deadline = npc_behavior_actors[idx].reaction_until;
    tick = game_session.tick;
    /* TODO: Investigate a reaction without a valid animation type */
    if ((uint32)type >= ANIMATION_TYPE_COUNT)
    {
        DIAGNOSTIC(fprintf(stderr, "Invalid NPC reaction type %d actor %d stage %u tick %u\n", type, idx, game_session.stage, game_session.tick));
        diagnostic_stop(__FUNCTION__, __FILE__, __LINE__);
    }
    table = animation_type_banks[(uint32)type];
    if ((sint32)tick < (sint32)deadline)
        return 0;
    state = npc_base_animation(idx);
    if (state == 57 || state == 59 || state == 208 || state == 210)
        return 0;
    if ((state == 230 || state == 231 || state == 232) && !(actor_animations[idx].motion.events & 512u))
        return 0;
    target = npc_targets[idx].idx;
    if (target == 0xffffffffu || npc_targets[idx].role == 4)
        return 0;
    if (!npc_animation_allows_target_reaction(actor_animations[idx].id))
        return 0;
    /* TODO: Investigate a reaction target outside the native actor pool */
    if (target >= ACTOR_COUNT)
    {
        DIAGNOSTIC(fprintf(stderr, "Invalid NPC reaction target %u actor %d role %u stage %u tick %u\n", target, idx, (uint32)npc_targets[idx].role, game_session.stage, game_session.tick));
        diagnostic_stop(__FUNCTION__, __FILE__, __LINE__);
    }
    reaction = npc_target_reaction_class(actor_animations[target].id);
    if (!reaction)
        return 0;
    x0 = actor_positions[target].x.whole;
    z0 = actor_positions[target].z.whole;
    x1 = actor_positions[idx].x.whole;
    z1 = actor_positions[idx].z.whole;
    angle = (uint32)math_dir16(x0, z0, x1, z1);
    angle -= (uint32)((uint16)actor_orientation[target].yaw) << 5;
    if (reaction == 2)
        angle ^= 0x8000u;
    if ((uint16)(angle + 12288u) >= 24577u)
        return 0;
    x0 = actor_positions[idx].x.whole;
    z0 = actor_positions[idx].z.whole;
    x1 = actor_positions[target].x.whole;
    z1 = actor_positions[target].z.whole;
    angle = (uint32)math_dir16(x0, z0, x1, z1);
    angle -= (uint32)((uint16)actor_orientation[idx].yaw) << 5;
    if ((uint16)(angle + 12288u) >= 24577u)
        return 0;
    if (dir_distance >= 449)
        return 0;
    /* 54EC4..550D4: target eligibility, probability, reaction and shared counters. */
    target = npc_targets[idx].idx;
    if (!action_history_matched(target))
        return 0;
    {
        uint32 roll = random_next() & 255u;
        if ((uint32)idx >= NPC_BEHAVIOR_ACTOR_COUNT)
            diagnostic_stop(__FUNCTION__, __FILE__, __LINE__);
        NPC_BEHAVIOR_REFERENCE profile = npc_behavior_actors[idx].profile;
        if (!profile.rows || profile.first >= profile.row_count)
            diagnostic_stop(__FUNCTION__, __FILE__, __LINE__);
        uint32 skill = profile.rows[profile.first].reaction_threshold;
        uint32 old;
        sint32 animation = -1, alternative;
        if (skill < roll)
        {
            npc_behavior_actors[idx].reaction_until = game_session.tick + ((255u - skill) >> 5);
            return 0;
        }
        if (state == 60)
        {
            if (actor_animations[idx].frame == 0xffffffffu)
                npc_start_animation(idx, table, 60);
            return 1;
        }
        alternative = npc_targets[idx].role == 1 && npc_targets[idx].idx != 0xffffffffu;
        if (reaction == 1)
            animation = alternative && !(random_next() & 1u) ? 230 : 57;
        else if (reaction == 2)
            animation = 57;
        else if (reaction == 3)
            animation = 231;
        else if (reaction == 4)
            animation = alternative && !(random_next() & 1u) ? 232 : 57;
        else if (reaction == 5)
        {
            if (actor_items[idx].bits != 0xffff)
                actor_items_drop(idx);
            animation = 208;
        }
        if (animation != -1)
            npc_start_animation(idx, table, animation);
        old = level_wave.attacker_trigger_count;
        level_wave.attacker_trigger_count = old + 1u;
        if ((sint32)(old + 1u) >= 3)
        {
            uint32 level = level_wave.attacker_limit, limit = level_wave.base_attacker_limit;
            level_wave.attacker_trigger_count = old - 1u;
            level += 1;
            limit += 1;
            level_wave.attacker_limit = level;
            if ((sint32)level > (sint32)limit)
                level_wave.attacker_limit = limit;
            level_wave.attacker_decay_tick = game_session.tick + 100u;
        }
    }
    return 1;
}

GDB_CALL sint32 npc_resume_combat6(sint32 idx)
{
    FUNCTION_MARKER(0x80052fe8u, "GAME.EXE");
    if (actor_animations[idx].frame == 0xffffffffu)
        return npc_continue_combat(idx);
    return -1;
}

/* 4A704 NPC dispatch and stage-specific environment response */
GDB_CALL sint32 npc_update(void)
{
    FUNCTION_MARKER(0x8004a704, "GAME.EXE");
    static const uint16 near_yaw_stage0[] = {0, 0};
    static const uint16 far_yaw_stage0[] = {16384, 49152};
    static const uint16 near_yaw_stage1[] = {16384, 49152, 16384, 16384, 0, 32768};
    static const uint16 far_yaw_stage1[] = {16384, 0, 16384, 0, 0, 32768};
    static const uint16 near_yaw_stage3[] = {16384, 16384, 49152, 49152};
    uint32 deadline = level_wave.attacker_decay_tick, tick = game_session.tick;
    uint32 stage, desired = 0, old, val, level, x, z, ax, az;
    MODEL_OBJECT *item;
    sint32 sound_played;
    sint32 i, j, type, nearest, distance, result, population, count;
    if (deadline == tick)
    {
        level = level_wave.attacker_limit;
        val = level_wave.base_attacker_limit;
        if ((sint32)val < (sint32)level)
        {
            level_wave.attacker_limit = level - 1;
            level_wave.attacker_decay_tick = deadline + 30;
            level_wave.attacker_trigger_count = 0;
        }
    }
    if (!(game_session.tick & 15u))
        npc_behavior_cycle.progress = npc_behavior_cycle.progress + 1u;
    val = npc_behavior_cycle.progress;
    if ((sint32)val < 49)
        npc_behavior_cycle.advance = 0;
    else
    {
        npc_behavior_cycle.advance = 1;
        npc_behavior_cycle.progress = val - 48;
    }
    npc_balance_targets();
    for (i = 2; i < ACTOR_COUNT; i++)
    {
        type = actor_types[i];
        if (type == -1)
            continue;
        if (((sint8)actor_lifecycle[i].death_ticks))
            npc_behavior_cycle.progress = npc_behavior_cycle.progress + 1u;
        if (npc_behavior_cycle.advance)
        {
            val = npc_behavior_actors[i].behavior;
            if ((sint32)val < 4)
            {
                npc_behavior_actors[i].behavior = val + 1;
                npc_behavior_actors[i].command = 0;
            }
        }
        uint32 update_idx = (uint32)type & 0x3fffffff;
        if (update_idx >= NPC_BEHAVIOR_TYPE_COUNT)
            diagnostic_stop(__FUNCTION__, __FILE__, __LINE__);
        NPC_ACTION_REFERENCE update = npc_updates[update_idx];
        if (update)
            npc_action_call(update, i);
    }
    stage = game_session.stage;
    if (stage >= 2u && stage != 3 && stage != 17)
        return 17;
    result = (sint32)world_objects.tracked_count;
    if (result <= 0)
        return result;
    i = 0;
    for (;;)
    {
        /* TODO: Investigate tracked-list indices outside the original eight references */
        if (i < 0 || i >= WORLD_TRACKED_COUNT)
        {
            DIAGNOSTIC(fprintf(stderr, "Invalid tracked NPC index %d count %u stage %u\n", i, world_objects.tracked_count, game_session.stage));
            diagnostic_stop(__FUNCTION__, __FILE__, __LINE__);
        }
        nearest = 32767;
        for (j = 2; j < ACTOR_COUNT; j++)
        {
            if (actor_types[j] == ACTOR_TYPE_NONE)
                continue;
            /* TODO: Investigate absent or invalid tracked references before object access */
            if (world_objects.tracked[i].present != 1 || world_objects.tracked[i].idx >= WORLD_OBJECT_COUNT)
            {
                DIAGNOSTIC(fprintf(stderr, "Invalid tracked world reference %u slot %u present %u\n", (uint32)(i), world_objects.tracked[i].idx, (uint32)world_objects.tracked[i].present));
                diagnostic_stop(__FUNCTION__, __FILE__, __LINE__);
            }
            const MODEL_OBJECT *model = world_objects.object + world_objects.tracked[i].idx;
            ax = (uint32)(sint32)actor_positions[j].x.whole;
            az = (uint32)(sint32)actor_positions[j].z.whole;
            x = (uint32)(sint32)model->pos.x.whole;
            z = (uint32)(sint32)model->pos.z.whole;
            distance = math_distance_approx(x, z, ax, az);
            /* Original tests entrance timer80 here, not HP66. */
            if (distance < nearest && actor_lifecycle[j].delay > 0)
                nearest = distance;
        }
        if (level_transition.exit_requested == 1)
        {
            population = (sint32)game_session.player_count;
            if (population > 0)
            {
                stage = game_session.stage;
                for (j = 0; j < population; j++)
                {
                    /* TODO: Investigate an exit population outside the actor pool */
                    if ((uint32)j >= ACTOR_COUNT)
                    {
                        DIAGNOSTIC(fprintf(stderr, "Invalid NPC exit player %d population %d tracked %d stage %u tick %u\n", j, population, i, game_session.stage, game_session.tick));
                        diagnostic_stop(__FUNCTION__, __FILE__, __LINE__);
                    }
                    uint32 pos = actor_positions[(uint32)j].z.bits;
                    if ((stage == 0 && (sint32)pos <= 0x0a5fffff) || (stage == 1 && pos + 0x077fffffu <= 0x02fffffeu))
                        nearest = 0;
                }
            }
        }
        stage = game_session.stage;
        if (stage == 3 && (sint32)game_session.scene_phase > 0)
            nearest = 512;
        else if (stage == 17 && (sint32)game_session.scene_phase > 0)
            nearest = 0;
        stage = game_session.stage;
        if (nearest < 384)
        {
            if (stage == 0)
            {
                /* TODO: Investigate original out-of-table object yaw reads */
                if (i < 0 || (uint32)i >= sizeof(near_yaw_stage0) / sizeof(*near_yaw_stage0))
                {
                    DIAGNOSTIC(fprintf(stderr, "Invalid NPC object yaw stage=%u index=%d\n", stage, i));
                    diagnostic_stop(__FUNCTION__, __FILE__, __LINE__);
                }
                desired = near_yaw_stage0[i];
            }
            else if (stage == 1)
            {
                /* TODO: Investigate original out-of-table object yaw reads */
                if (i < 0 || (uint32)i >= sizeof(near_yaw_stage1) / sizeof(*near_yaw_stage1))
                {
                    DIAGNOSTIC(fprintf(stderr, "Invalid NPC object yaw stage=%u index=%d\n", stage, i));
                    diagnostic_stop(__FUNCTION__, __FILE__, __LINE__);
                }
                desired = near_yaw_stage1[i];
                if (i == 4)
                {
                    val = stage_animation.phase[0];
                    stage_animation.hold_ticks = 60;
                    if (!val)
                        sound_voice_play(124, 255, 0x1900);
                    val = stage_animation.phase[0];
                    if ((sint32)val < 32)
                        stage_animation.phase[0] = val + 2;
                }
                else
                {
                    /* TODO: Investigate absent or invalid tracked references before object access */
                    if (world_objects.tracked[i].present != 1 || world_objects.tracked[i].idx >= WORLD_OBJECT_COUNT)
                    {
                        DIAGNOSTIC(fprintf(stderr, "Invalid tracked world reference %u slot %u present %u\n", (uint32)(i), world_objects.tracked[i].idx, (uint32)world_objects.tracked[i].present));
                        diagnostic_stop(__FUNCTION__, __FILE__, __LINE__);
                    }
                    item = world_objects.object + world_objects.tracked[i].idx;
                    switch (item->payload_kind)
                    {
                        case MODEL_PAYLOAD_UNMIGRATED:
                            sound_played = item->pending_motion.sound_played;
                            break;
                        case MODEL_PAYLOAD_OSCILLATION:
                            sound_played = item->oscillation.sound_played;
                            break;
                        default:
                            /* TODO: Investigate a tracked sound flag with a foreign payload */
                            DIAGNOSTIC(fprintf(stderr, "Invalid tracked sound owner slot %u kind %u payload %u stage %u tick %u\n", world_objects.tracked[i].idx, item->kind, (uint32)item->payload_kind, game_session.stage, game_session.tick));
                            diagnostic_stop(__FUNCTION__, __FILE__, __LINE__);
                    }
                    if (!sound_played)
                    {
                        sound_voice_play(108, 127, 0x1b00);
                        /* TODO: Investigate absent or invalid tracked references before object access */
                        if (world_objects.tracked[i].present != 1 || world_objects.tracked[i].idx >= WORLD_OBJECT_COUNT)
                        {
                            DIAGNOSTIC(fprintf(stderr, "Invalid tracked world reference %u slot %u present %u\n", (uint32)(i), world_objects.tracked[i].idx, (uint32)world_objects.tracked[i].present));
                            diagnostic_stop(__FUNCTION__, __FILE__, __LINE__);
                        }
                        item = world_objects.object + world_objects.tracked[i].idx;
                        switch (item->payload_kind)
                        {
                            case MODEL_PAYLOAD_UNMIGRATED:
                                item->pending_motion.sound_played = 1;
                                break;
                            case MODEL_PAYLOAD_OSCILLATION:
                                item->oscillation.sound_played = 1;
                                break;
                            default:
                                /* TODO: Investigate a tracked sound store with a foreign payload */
                                DIAGNOSTIC(fprintf(stderr, "Invalid tracked sound store slot %u kind %u payload %u stage %u tick %u\n", world_objects.tracked[i].idx, item->kind, (uint32)item->payload_kind, game_session.stage, game_session.tick));
                                diagnostic_stop(__FUNCTION__, __FILE__, __LINE__);
                        }
                    }
                }
            }
            else if (stage == 3)
            {
                /* TODO: Investigate absent or invalid tracked references before object access */
                if (world_objects.tracked[i].present != 1 || world_objects.tracked[i].idx >= WORLD_OBJECT_COUNT)
                {
                    DIAGNOSTIC(fprintf(stderr, "Invalid tracked world reference %u slot %u present %u\n", (uint32)(i), world_objects.tracked[i].idx, (uint32)world_objects.tracked[i].present));
                    diagnostic_stop(__FUNCTION__, __FILE__, __LINE__);
                }
                item = world_objects.object + world_objects.tracked[i].idx;
                switch (item->payload_kind)
                {
                    case MODEL_PAYLOAD_UNMIGRATED:
                        sound_played = item->pending_motion.sound_played;
                        break;
                    case MODEL_PAYLOAD_OSCILLATION:
                        sound_played = item->oscillation.sound_played;
                        break;
                    default:
                        /* TODO: Investigate a tracked sound flag with a foreign payload */
                        DIAGNOSTIC(fprintf(stderr, "Invalid tracked sound owner slot %u kind %u payload %u stage %u tick %u\n", world_objects.tracked[i].idx, item->kind, (uint32)item->payload_kind, game_session.stage, game_session.tick));
                        diagnostic_stop(__FUNCTION__, __FILE__, __LINE__);
                }
                if (!sound_played)
                {
                    sound_voice_play(108, 127, 0x1b00);
                    /* TODO: Investigate absent or invalid tracked references before object access */
                    if (world_objects.tracked[i].present != 1 || world_objects.tracked[i].idx >= WORLD_OBJECT_COUNT)
                    {
                        DIAGNOSTIC(fprintf(stderr, "Invalid tracked world reference %u slot %u present %u\n", (uint32)(i), world_objects.tracked[i].idx, (uint32)world_objects.tracked[i].present));
                        diagnostic_stop(__FUNCTION__, __FILE__, __LINE__);
                    }
                    item = world_objects.object + world_objects.tracked[i].idx;
                    switch (item->payload_kind)
                    {
                        case MODEL_PAYLOAD_UNMIGRATED:
                            item->pending_motion.sound_played = 1;
                            break;
                        case MODEL_PAYLOAD_OSCILLATION:
                            item->oscillation.sound_played = 1;
                            break;
                        default:
                            /* TODO: Investigate a tracked sound store with a foreign payload */
                            DIAGNOSTIC(fprintf(stderr, "Invalid tracked sound store slot %u kind %u payload %u stage %u tick %u\n", world_objects.tracked[i].idx, item->kind, (uint32)item->payload_kind, game_session.stage, game_session.tick));
                            diagnostic_stop(__FUNCTION__, __FILE__, __LINE__);
                    }
                }
                /* TODO: Investigate original out-of-table object yaw reads */
                if (i < 0 || (uint32)i >= sizeof(near_yaw_stage3) / sizeof(*near_yaw_stage3))
                {
                    DIAGNOSTIC(fprintf(stderr, "Invalid NPC object yaw stage=%u index=%d\n", stage, i));
                    diagnostic_stop(__FUNCTION__, __FILE__, __LINE__);
                }
                desired = near_yaw_stage3[i];
            }
            else if (stage == 17)
            {
                val = stage_animation.phase[0];
                stage_animation.hold_ticks = 60;
                if ((sint32)val < 64)
                    stage_animation.phase[0] = val + 8;
            }
        }
        else
        {
            if (stage == 0)
            {
                /* TODO: Investigate original out-of-table object yaw reads */
                if (i < 0 || (uint32)i >= sizeof(far_yaw_stage0) / sizeof(*far_yaw_stage0))
                {
                    DIAGNOSTIC(fprintf(stderr, "Invalid NPC object yaw stage=%u index=%d\n", stage, i));
                    diagnostic_stop(__FUNCTION__, __FILE__, __LINE__);
                }
                desired = far_yaw_stage0[i];
            }
            else if (stage == 1)
            {
                /* TODO: Investigate absent or invalid tracked references before object access */
                if (world_objects.tracked[i].present != 1 || world_objects.tracked[i].idx >= WORLD_OBJECT_COUNT)
                {
                    DIAGNOSTIC(fprintf(stderr, "Invalid tracked world reference %u slot %u present %u\n", (uint32)(i), world_objects.tracked[i].idx, (uint32)world_objects.tracked[i].present));
                    diagnostic_stop(__FUNCTION__, __FILE__, __LINE__);
                }
                item = world_objects.object + world_objects.tracked[i].idx;
                /* TODO: Investigate original out-of-table object yaw reads */
                if (i < 0 || (uint32)i >= sizeof(far_yaw_stage1) / sizeof(*far_yaw_stage1))
                {
                    DIAGNOSTIC(fprintf(stderr, "Invalid NPC object yaw stage=%u index=%d\n", stage, i));
                    diagnostic_stop(__FUNCTION__, __FILE__, __LINE__);
                }
                desired = far_yaw_stage1[i];
                old = item->angles.yaw_bits;
                if (i == 4)
                {
                    val = stage_animation.hold_ticks;
                    if ((sint32)val > 0)
                        stage_animation.hold_ticks = val - 1;
                    if (!stage_animation.hold_ticks)
                    {
                        if (stage_animation.phase[0] == 32)
                            sound_voice_play(124, 255, (random_next() & 0x1ffu) | 0x1800u);
                        val = stage_animation.phase[0];
                        if ((sint32)val > 0)
                            stage_animation.phase[0] = val - 2;
                    }
                }
                else
                {
                    switch (item->payload_kind)
                    {
                        case MODEL_PAYLOAD_UNMIGRATED:
                            sound_played = item->pending_motion.sound_played;
                            break;
                        case MODEL_PAYLOAD_OSCILLATION:
                            sound_played = item->oscillation.sound_played;
                            break;
                        default:
                            /* TODO: Investigate a tracked sound flag with a foreign payload */
                            DIAGNOSTIC(fprintf(stderr, "Invalid tracked sound owner slot %u kind %u payload %u stage %u tick %u\n", world_objects.tracked[i].idx, item->kind, (uint32)item->payload_kind, game_session.stage, game_session.tick));
                            diagnostic_stop(__FUNCTION__, __FILE__, __LINE__);
                    }
                    if (sound_played == 1 && ((desired - old + 511u) & 0xffffu) < 0x3ffu)
                    {
                        sound_voice_play(109, 79, (random_next() & 0x1ffu) | 0x1a00u);
                        /* TODO: Investigate absent or invalid tracked references before object access */
                        if (world_objects.tracked[i].present != 1 || world_objects.tracked[i].idx >= WORLD_OBJECT_COUNT)
                        {
                            DIAGNOSTIC(fprintf(stderr, "Invalid tracked world reference %u slot %u present %u\n", (uint32)(i), world_objects.tracked[i].idx, (uint32)world_objects.tracked[i].present));
                            diagnostic_stop(__FUNCTION__, __FILE__, __LINE__);
                        }
                        item = world_objects.object + world_objects.tracked[i].idx;
                        switch (item->payload_kind)
                        {
                            case MODEL_PAYLOAD_UNMIGRATED:
                                item->pending_motion.sound_played = 0;
                                break;
                            case MODEL_PAYLOAD_OSCILLATION:
                                item->oscillation.sound_played = 0;
                                break;
                            default:
                                /* TODO: Investigate a tracked sound store with a foreign payload */
                                DIAGNOSTIC(fprintf(stderr, "Invalid tracked sound store slot %u kind %u payload %u stage %u tick %u\n", world_objects.tracked[i].idx, item->kind, (uint32)item->payload_kind, game_session.stage, game_session.tick));
                                diagnostic_stop(__FUNCTION__, __FILE__, __LINE__);
                        }
                    }
                }
            }
            else if (stage == 3)
            {
                /* TODO: Investigate absent or invalid tracked references before object access */
                if (world_objects.tracked[i].present != 1 || world_objects.tracked[i].idx >= WORLD_OBJECT_COUNT)
                {
                    DIAGNOSTIC(fprintf(stderr, "Invalid tracked world reference %u slot %u present %u\n", (uint32)(i), world_objects.tracked[i].idx, (uint32)world_objects.tracked[i].present));
                    diagnostic_stop(__FUNCTION__, __FILE__, __LINE__);
                }
                item = world_objects.object + world_objects.tracked[i].idx;
                old = item->angles.yaw_bits;
                switch (item->payload_kind)
                {
                    case MODEL_PAYLOAD_UNMIGRATED:
                        sound_played = item->pending_motion.sound_played;
                        break;
                    case MODEL_PAYLOAD_OSCILLATION:
                        sound_played = item->oscillation.sound_played;
                        break;
                    default:
                        /* TODO: Investigate a tracked sound flag with a foreign payload */
                        DIAGNOSTIC(fprintf(stderr, "Invalid tracked sound owner slot %u kind %u payload %u stage %u tick %u\n", world_objects.tracked[i].idx, item->kind, (uint32)item->payload_kind, game_session.stage, game_session.tick));
                        diagnostic_stop(__FUNCTION__, __FILE__, __LINE__);
                }
                type = sound_played;
                desired = 0;
                if (type == 1 && ((511u - old) & 0xffffu) < 0x3ffu)
                {
                    sound_voice_play(109, 79, (random_next() & 0x1ffu) | 0x1a00u);
                    /* TODO: Investigate absent or invalid tracked references before object access */
                    if (world_objects.tracked[i].present != 1 || world_objects.tracked[i].idx >= WORLD_OBJECT_COUNT)
                    {
                        DIAGNOSTIC(fprintf(stderr, "Invalid tracked world reference %u slot %u present %u\n", (uint32)(i), world_objects.tracked[i].idx, (uint32)world_objects.tracked[i].present));
                        diagnostic_stop(__FUNCTION__, __FILE__, __LINE__);
                    }
                    item = world_objects.object + world_objects.tracked[i].idx;
                    switch (item->payload_kind)
                    {
                        case MODEL_PAYLOAD_UNMIGRATED:
                            item->pending_motion.sound_played = 0;
                            break;
                        case MODEL_PAYLOAD_OSCILLATION:
                            item->oscillation.sound_played = 0;
                            break;
                        default:
                            /* TODO: Investigate a tracked sound store with a foreign payload */
                            DIAGNOSTIC(fprintf(stderr, "Invalid tracked sound store slot %u kind %u payload %u stage %u tick %u\n", world_objects.tracked[i].idx, item->kind, (uint32)item->payload_kind, game_session.stage, game_session.tick));
                            diagnostic_stop(__FUNCTION__, __FILE__, __LINE__);
                    }
                }
            }
            else if (stage == 17)
            {
                val = stage_animation.hold_ticks;
                if ((sint32)val > 0)
                    stage_animation.hold_ticks = val - 1;
                if (!stage_animation.hold_ticks)
                {
                    val = stage_animation.phase[0];
                    if ((sint32)val > 0)
                        stage_animation.phase[0] = val - 8;
                }
            }
        }
        /* TODO: Investigate absent or invalid tracked references before object access */
        if (world_objects.tracked[i].present != 1 || world_objects.tracked[i].idx >= WORLD_OBJECT_COUNT)
        {
            DIAGNOSTIC(fprintf(stderr, "Invalid tracked world reference %u slot %u present %u\n", (uint32)(i), world_objects.tracked[i].idx, (uint32)world_objects.tracked[i].present));
            diagnostic_stop(__FUNCTION__, __FILE__, __LINE__);
        }
        item = world_objects.object + world_objects.tracked[i].idx;
        count = (sint32)world_objects.tracked_count;
        old = item->angles.yaw_bits;
        result = (sint32)((desired - old) << 16) >> 18;
        item->angles.yaw_bits = (uint16)(old + (uint32)result);
        if (++i >= count)
            return result;
    }
}

/* 544E4: face stored target only in mode1; reload animation after facing. */
GDB_CALL sint32 npc_finish_facing_target(sint32 idx)
{
    FUNCTION_MARKER(0x800544e4u, "GAME.EXE");
    uint32 target, x, z;
    if (npc_targets[idx].role == 1)
    {
        target = npc_targets[idx].idx;
        if (target != 0xffffffffu)
        {
            x = actor_positions[target].x.bits;
            z = actor_positions[target].z.bits;
            npc_turn_to_point(idx, x, z);
        }
    }
    if (actor_animations[idx].frame == 0xffffffffu)
        return npc_update_combat(idx);
    return -1;
}

/* 33D7C: signed impact count; inherited A2/A3 are unused by31A28. */

/* 49DB0: AEE90 is an immediate table address, not a word loaded there. */
GDB_CALL sint32 npc_start_jump_off(sint32 idx, sint32 mode)
{
    FUNCTION_MARKER(0x80049db0u, "GAME.EXE");
    npc_start_animation(idx, animation_type_banks[(uint32)actor_types[idx]], 269);
    actor_lifecycle[idx].delay = 25;
    if ((uint32)idx >= NPC_BEHAVIOR_ACTOR_COUNT)
        diagnostic_stop(__FUNCTION__, __FILE__, __LINE__);
    npc_action_actors[idx] = (NPC_ACTION_BANK_REFERENCE){npc_actions[NPC_ACTION_DEFAULT], 0};
    npc_behavior_actors[idx].behavior = 0;
    npc_behavior_actors[idx].command = 0;
    actors_find_nearest_targets(idx);
    npc_targets[idx].idx = actor_targeting.player_idx;
    npc_targets[idx].role = mode ? 1 : 2;
    return mode ? 1 : 2;
}

/* Van arrival, collision region, braking and four passenger spawns */
GDB_CALL uint32 npc_update_van(sint32 idx)
{
    FUNCTION_MARKER(0x80049e6cu, "GAME.EXE");
    NPC_SPECIAL *special = &npc_behavior_actors[idx].special;
    ACTOR_ANIMATION *playback = actor_animations + idx;
    FIXED_VEC3 *pos = actor_positions + idx;
    ACTOR_ORIENTATION *orientation = actor_orientation + idx;
    ACTOR_MOVEMENT *movement = actor_movement + idx;
    FIXED_VEC3 point;
    uint32 x, z, n, phase, state, wave, volume, pan, angle;
    COLLISION_REGION *region;
    VEHICLE_COLLISION_SHAPE vehicle;
    sint32 speed, heading, yaw, tilt, roll, rate, val, child;
    int i;
    static const sint32 spawn_frame[4] = {13, 16, 28, 31}, spawn_kind[4] = {5, 6, 7, 5};
    state = playback->id;
    speed = movement->van_speed;
    heading = movement->van_heading;
    if (!state)
        heading = 1024;
    if (heading < 0 || heading >= MATH_SIN_COUNT)
    {
        /* TODO: Investigate van headings outside the original sine table */
        DIAGNOSTIC(fprintf(stderr, "Invalid van sine heading %d actor %d animation %u stage %u tick %u\n", heading, idx, playback->id, game_session.stage, game_session.tick));
        diagnostic_stop(__FUNCTION__, __FILE__, __LINE__);
    }
    wave = (uint32)(sint32)sin_table[heading];
    x = pos->x.bits;
    pos->x.bits = x + (uint32)((sint32)(wave * (uint32)speed) >> 1);
    if (heading < -MATH_QUARTER_TURN || heading >= MATH_ANGLE_COUNT)
    {
        /* TODO: Investigate van headings outside the original cosine view */
        DIAGNOSTIC(fprintf(stderr, "Invalid van cosine heading %d actor %d animation %u stage %u tick %u\n", heading, idx, playback->id, game_session.stage, game_session.tick));
        diagnostic_stop(__FUNCTION__, __FILE__, __LINE__);
    }
    wave = (uint32)(sint32)sin_table[MATH_QUARTER_TURN + heading];
    n = col_regions.count;
    z = pos->z.bits;
    if (n >= COLLISION_REGION_COUNT)
        diagnostic_stop(__FUNCTION__, __FILE__, __LINE__);
    region = col_regions.data + n;
    pos->z.bits = z - (uint32)((sint32)(wave * (uint32)speed) >> 1);
    col_region_configure(region, 66);
    vehicle.yaw = (uint16)heading;
    vehicle.x = pos->x.bits;
    n = col_regions.count;
    vehicle.z = pos->z.bits;
    col_regions.count = n - 1;
    actors_collide_vehicle_shape(region, &vehicle);
    yaw = orientation->yaw;
    if (heading != yaw)
        heading -= 16;
    if (playback->id)
    {
        if (!speed)
            special->van_voice = (uint32)sound_voice_play(132, 0, 6400) + 1u;
        sound_spatial(pos, &volume, &pan);
        sound_voice_pan(special->van_voice - 1, volume, pan);
        if (speed == 240)
            sound_voice_play(110, 255, 5888);
        if (speed < 256)
            speed += 16;
        if ((sint32)pos->x.bits <= 100663295)
        {
            yaw = orientation->yaw;
            if (yaw != 1024)
                orientation->yaw = (uint16)((yaw - 32) & 2047);
            else if ((sint32)pos->z.bits > -100663296)
            {
                playback->id = 0;
                playback->frame = 0;
                playback->src = (ACTOR_ANIMATION_SOURCE){ACTOR_ANIMATION_SOURCE_PROCEDURAL, {0}};
            }
        }
    }
    else
    {
        rate = orientation->yaw_rate;
        if (yaw >= 385)
        {
            if (rate >= -31)
                orientation->yaw_rate = (uint16)(rate - 2);
            orientation->roll_rate = 1536;
        }
        else if (rate < 0)
            orientation->yaw_rate = (uint16)(rate + 4);
        angle = (uint16)orientation->yaw;
        rate = (uint16)orientation->yaw_rate;
        roll = orientation->roll;
        tilt = orientation->roll_rate;
        orientation->yaw = (uint16)(angle + rate);
        val = (uint16)orientation->roll;
        tilt = (sint16)(tilt + ((-roll - tilt) >> 3));
        orientation->roll_rate = (uint16)tilt;
        roll = (sint16)(val + (tilt >> 2));
        orientation->roll = (uint16)roll;
        if (roll >= 1537)
            orientation->roll = 1536;
        if (orientation->roll < -1536)
            orientation->roll = (uint16)-1536;
        speed -= 8;
        if (speed < 0)
            speed = 0;
        if (!speed)
        {
            phase = playback->frame + 1u;
            playback->frame = phase;
            val = (sint32)stage_animation.phase[0];
            if ((sint32)phase < 39)
            {
                if (val < 28)
                    stage_animation.phase[0] = (uint32)val + 1u;
            }
            else if (val > 0)
                stage_animation.phase[0] = (uint32)val - 4u;
        }
        if ((sint32)playback->frame >= 2)
        {
            x = pos->x.bits;
            z = pos->z.bits;
            yaw = orientation->yaw;
            transform_push();
            transform_identity();
            transform_pitch(32768);
            transform_yaw((uint32)yaw << 5);
            phase = playback->frame;
            for (i = 0; i < 4; i++)
            {
                if (i == 2)
                    phase = playback->frame;
                if (phase == (uint32)spawn_frame[i])
                {
                    transform_point((i & 1) ? -5242880 : 5242880, 0, i < 2 ? 524288 : -6815744, point.bits);
                    child = npc_create(spawn_kind[i], x + point.x.bits, z + point.z.bits, (yaw + ((i & 1) ? -512 : 512)) & 2047, 0, 0, (i & 1) ? 0 : 1);
                    if (child < 0)
                    {
                        /* Keep this passenger pending until an NPC slot becomes free */
                        playback->frame = (uint32)spawn_frame[i] - 1;
                        phase = (uint32)spawn_frame[i] - 1;
                        continue;
                    }
                    npc_start_jump_off(child, i == 0);
                    if (i)
                        level_wave.remaining_enemies = level_wave.remaining_enemies + 1u;
                    if (i == 3)
                        actor_lifecycle[idx].spawn_flag = 0;
                    if (i == 0 || i == 2)
                        phase = playback->frame;
                }
            }
            if (playback->frame == 47)
            {
                uint32 world_idx = world_objects.count;
                n = col_regions.count;
                if (n >= COLLISION_REGION_COUNT)
                    diagnostic_stop(__FUNCTION__, __FILE__, __LINE__);
                region = col_regions.data + n;
                actor_types[idx] = ACTOR_TYPE_NONE;
                /* TODO: Investigate the original van append beyond the world pool */
                if (world_idx >= WORLD_OBJECT_COUNT)
                {
                    DIAGNOSTIC(fprintf(stderr, "Invalid van world slot %u actor %d stage %u tick %u\n", world_idx, idx, game_session.stage, game_session.tick));
                    diagnostic_stop(__FUNCTION__, __FILE__, __LINE__);
                }
                MODEL_OBJECT *world = world_objects.object + world_idx;
                /* Publish the retained motion under its concrete runtime owner */
                if (world->payload_kind != MODEL_PAYLOAD_UNMIGRATED)
                {
                    /* TODO: Investigate van construction over another native payload */
                    DIAGNOSTIC(fprintf(stderr, "Invalid van reserve owner slot %u payload %u stage %u tick %u\n", world_idx, world->payload_kind, game_session.stage, game_session.tick));
                    diagnostic_stop(__FUNCTION__, __FILE__, __LINE__);
                }
                world->oscillation = world->pending_motion;
                world->payload_kind = MODEL_PAYLOAD_OSCILLATION;
                world->kind = 66;
                world->pos.x.bits = x;
                world->pos.y.bits = 0;
                world->pos.z.bits = z;
                world->angles.yaw_bits = (uint16)((uint32)yaw << 5);
                val = (sint32)special->van_voice;
                state = game_session.scene_phase;
                world->oscillation.voice_plus_one = (sint16)val;
                region->object = (WORLD_OBJECT_REFERENCE){world_idx, 1};
                region->geometry.coord[2] = x >> 16;
                region->geometry.coord[0] = x >> 16;
                region->geometry.coord[3] = z >> 16;
                region->geometry.coord[1] = z >> 16;
                phase = world->kind;
                col_regions.count = n + 1;
                game_session.scene_phase = state + 1;
                col_region_configure(region, phase);
                n = world_objects.count;
                /* TODO: Investigate an exhausted world pool after the first van append */
                if (world_idx + 1 >= WORLD_OBJECT_COUNT)
                {
                    DIAGNOSTIC(fprintf(stderr, "Invalid second van world slot %u actor %d stage %u tick %u\n", world_idx + 1, idx, game_session.stage, game_session.tick));
                    diagnostic_stop(__FUNCTION__, __FILE__, __LINE__);
                }
                world = world_objects.object + world_idx + 1;
                /* The companion retains the original flags, speed and voice words */
                if (world->payload_kind != MODEL_PAYLOAD_UNMIGRATED)
                {
                    /* TODO: Investigate van construction over another native companion */
                    DIAGNOSTIC(fprintf(stderr, "Invalid van companion reserve slot %u payload %u stage %u tick %u\n", world_idx + 1, world->payload_kind, game_session.stage, game_session.tick));
                    diagnostic_stop(__FUNCTION__, __FILE__, __LINE__);
                }
                world->companion = (COMPANION_MODEL){.flags = world->pending_motion.roll_speed, .speed = world->pending_motion.speed, .physics_spin = world->pending_motion.pitch_speed, .physics_actor_idx = world->pending_motion.phase, .voice = world->pending_motion.voice_plus_one, .physics_vertical_speed = world->pending_motion.sound_clock, .physics_obstacle_flags = world->pending_motion.physics_obstacle_flags};
                world->payload_kind = MODEL_PAYLOAD_COMPANION;
                world->kind = 65;
                world->pos.x.bits = x;
                world->pos.y.bits = 0;
                world->pos.z.bits = z;
                world->angles.yaw_bits = (uint16)((uint32)yaw << 5);
                world_objects.count = n + 2u;
            }
            transform_pop();
        }
    }
    movement->van_heading = (uint16)(heading & 2047);
    movement->van_speed = (uint16)speed;
    return (uint32)heading & 2047;
}

GDB_CALL uint32 npc_finish_animation(sint32 idx)
{
    FUNCTION_MARKER(0x8004efb8u, "GAME.EXE");
    uint32 result = 0xffffffffu;
    if (actor_animations[idx].frame == 0xffffffffu)
    {
        result = actor_status[idx].flags & 0xfffffffeu;
        actor_status[idx].flags = result;
    }
    return result;
}

GDB_CALL sint32 npc_continue_attack_behavior(sint32 idx)
{
    FUNCTION_MARKER(0x80053298u, "GAME.EXE");
    return npc_update_combat(idx);
}
