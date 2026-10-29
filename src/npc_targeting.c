#include <stdio.h>

#include "game.h"

/* The biased original base addresses six NPC records, not player storage */
sint32 npc_target_priorities[NPC_TARGET_PRIORITY_COUNT];
NPC_TARGET npc_targets[NPC_BEHAVIOR_ACTOR_COUNT];
sint32 npc_target_distances[NPC_BEHAVIOR_ACTOR_COUNT];

sint32 npc_find_target_conflict(sint32 idx, sint32 target)
{
    FUNCTION_MARKER(0x80048844, "GAME.EXE");
    sint32 i, count = 0, role;
    uint32 wanted = (uint32)target - NPC_TARGET_ITEM_FIRST;
    for (i = NPC_TARGET_PLAYER_COUNT; i < NPC_BEHAVIOR_ACTOR_COUNT; i++)
    {
        if (i == idx || actor_types[i] == ACTOR_TYPE_NONE || actor_health[i].hp <= 0)
            continue;
        if (target < NPC_TARGET_PLAYER_COUNT)
        {
            role = npc_targets[i].role;
            if ((role == 1 || role == 3) && npc_targets[i].idx == (uint32)target)
                ++count;
            if (count >= (sint32)level_wave.attacker_limit)
                return i;
        }
        else
        {
            if ((uint32)(sint32)actor_items[i].slot[ACTOR_ITEM_RIGHT] == wanted)
                return i;
            role = npc_targets[i].role;
            if (role == 4 && npc_targets[i].idx == wanted)
                return i;
        }
    }
    return -1;
}

void npc_reset_target_priorities(sint32 idx)
{
    FUNCTION_MARKER(0x80048964, "GAME.EXE");
    uint32 i;
    XASSERT((uint32)idx - NPC_TARGET_ACTOR_FIRST < NPC_TARGET_ACTOR_COUNT);
    sint32 *rank = npc_target_priorities + NPC_TARGET_COUNT * ((uint32)idx - NPC_TARGET_ACTOR_FIRST);
    for (i = 0; i < NPC_TARGET_COUNT; i++)
        rank[i] = -1;
}

sint32 npc_select_target(sint32 idx)
{
    FUNCTION_MARKER(0x8004899c, "GAME.EXE");
    sint32 i, best = 0, result = -1, val;
    XASSERT((uint32)idx - NPC_TARGET_ACTOR_FIRST < NPC_TARGET_ACTOR_COUNT);
    const sint32 *rank = npc_target_priorities + NPC_TARGET_COUNT * ((uint32)idx - NPC_TARGET_ACTOR_FIRST);
    for (i = 0; i < NPC_TARGET_COUNT; i++)
    {
        val = rank[i];
        if (best < val)
        {
            best = val;
            result = i;
        }
    }
    return result;
}

sint32 npc_rank_targets(sint32 idx)
{
    FUNCTION_MARKER(0x800489f4, "GAME.EXE");
    uint32 config;
    uint32 bias;
    uint32 bits;
    uint32 angle;
    sint32 numerator;
    const NPC_AI_CAPABILITY *capability;
    sint32 i, type, denominator, actor_x, actor_z, other_x, other_z;
    XASSERT((uint32)idx - NPC_TARGET_ACTOR_FIRST < NPC_TARGET_ACTOR_COUNT);
    uint32 first = NPC_TARGET_COUNT * ((uint32)idx - NPC_TARGET_ACTOR_FIRST);
    sint32 *rank = npc_target_priorities + first;
    for (i = 0; i < (sint32)game_session.player_count; i++)
    {
        XASSERT((uint32)i < NPC_TARGET_PRIORITY_COUNT - first);
        /* TODO: Investigate invalid player counts before expanding actor capacity */
        XASSERT(i < ACTOR_COUNT);
        if (actor_types[i] == ACTOR_TYPE_NONE || actor_health[i].hp <= 0)
            rank[i] = -1;
        else
        {
            actor_x = actor_positions[idx].x.whole;
            actor_z = actor_positions[idx].z.whole;
            other_x = actor_positions[i].x.whole;
            other_z = actor_positions[i].z.whole;
            math_dir16(actor_x, actor_z, other_x, other_z);
            rank[i] = 0x7fffu - dir_distance;
        }
    }
    config = npc_behavior_actors[idx].behavior;
    NPC_BEHAVIOR_REFERENCE profile = npc_behavior_actors[idx].profile;
    config = (profile.first + config) & NPC_BEHAVIOR_ROW_MASK;
    XASSERT(profile.rows && config < profile.row_count);
    const NPC_BEHAVIOR_ROW *behavior_row = profile.rows + config;
    XASSERT(behavior_row->capability < NPC_AI_CAPABILITY_COUNT);
    capability = npc_ai_capabilities + behavior_row->capability;
    bias = 8u * behavior_row->pickup_bias - 1024u;
    for (i = 0; i < DYNAMIC_OBJECT_COUNT; i++)
    {
        const MODEL_OBJECT *item = dynamic_objects + i;
        if (actor_items[idx].slot[ACTOR_ITEM_RIGHT] != -1 || actor_items[idx].slot[ACTOR_ITEM_LEFT] != -1)
            goto ineligible;
        type = item->kind;
        if (type < 477 || type >= 563)
            goto ineligible;
        /* TODO: Investigate a pickup candidate with a foreign native owner */
        XASSERT(item->payload_kind == MODEL_PAYLOAD_PHYSICS);
        if (item->physics.speed || item->pos.y.bits == 1 || item->physics.life < 0)
            goto ineligible;
        bits = item_properties[type - DYNAMIC_ITEM_TYPE_FIRST].packed;
        if ((bits & capability->weapon_flags) != (bits & 65535u) || npc_find_target_conflict(idx, i + NPC_TARGET_ITEM_FIRST) != -1)
            goto ineligible;
        actor_x = actor_positions[idx].x.whole;
        actor_z = actor_positions[idx].z.whole;
        other_x = item->pos.x.whole;
        other_z = item->pos.z.whole;
        angle = math_dir16(actor_x, actor_z, other_x, other_z);
        angle -= (uint32)((uint16)actor_orientation[idx].yaw) << 5;
        if (angle & 0x8000u)
            angle = (uint32)(-(sint64)(angle));
        numerator = (0x7fffu - dir_distance) << 13;
        denominator = ((sint32)(angle << 16) >> 20) + 8192;
        /* Signed16 angular fold bounds the denominator to 6144..10239 */
        rank[NPC_TARGET_ITEM_FIRST + i] = (numerator / denominator) + bias;
        if (dir_distance < 769)
            continue;
    ineligible:
        rank[NPC_TARGET_ITEM_FIRST + i] = -1;
    }
    return 0;
}

sint32 npc_update_target(sint32 idx)
{
    FUNCTION_MARKER(0x800531c4, "GAME.EXE");
    NPC_TARGET *choice = npc_targets + idx;
    sint32 target, role;
    npc_rank_targets(idx);
    target = npc_select_target(idx);
    if (target == -1)
    {
        choice->idx = 0xffffffffu;
        return 1;
    }
    if (target >= NPC_TARGET_ITEM_FIRST)
    {
        choice->idx = (uint32)target - NPC_TARGET_ITEM_FIRST;
        choice->role = 4;
        return 4;
    }
    choice->idx = target;
    role = npc_find_target_conflict(idx, target) == -1 ? 1 : 2;
    choice->role = role;
    return role;
}

/* Preserve X-only distance and the current-actor candidate comparison */
uint32 npc_balance_targets(void)
{
    FUNCTION_MARKER(0x8004a4cc, "GAME.EXE");
    uint32 i, j, target, sel;
    sint32 x, tx, distance;
    for (i = 2; i < 8; i++)
    {
        if (actor_types[i] == ACTOR_TYPE_NONE || npc_targets[i].role - 1u >= 2u)
            continue;
        target = npc_targets[i].idx;
        if (target == 0xffffffffu)
            continue;
        x = actor_positions[i].x.whole;
        tx = actor_positions[target].x.whole;
        npc_target_distances[i] = math_distance_approx(x, x, tx, tx);
    }
    for (i = 2; i < 8; i++)
    {
        if (actor_types[i] == ACTOR_TYPE_NONE || npc_targets[i].role != 1 || npc_targets[i].idx == 0xffffffffu)
            continue;
        sel = i;
        distance = npc_target_distances[i];
        for (j = 2; j < 8; j++)
        {
            if (j != i && actor_types[j] != ACTOR_TYPE_NONE && npc_targets[j].role == 2 && npc_targets[j].idx == npc_targets[i].idx && npc_target_distances[j] < distance)
            {
                distance = npc_target_distances[i];
                sel = j;
            }
        }
        if (sel != i)
        {
            npc_targets[i].role = 2;
            npc_targets[sel].role = 1;
        }
    }
    return 0;
}
