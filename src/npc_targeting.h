#ifndef NPC_TARGETING_H
#define NPC_TARGETING_H

#include "dynamic_objects.h"
#include "npc_behavior.h"

enum
{
    NPC_TARGET_PLAYER_COUNT = 2,
    NPC_TARGET_ITEM_FIRST = NPC_TARGET_PLAYER_COUNT,
    NPC_TARGET_ACTOR_FIRST = NPC_TARGET_PLAYER_COUNT,
    NPC_TARGET_ACTOR_COUNT = NPC_BEHAVIOR_ACTOR_COUNT - NPC_TARGET_ACTOR_FIRST,
    NPC_TARGET_COUNT = NPC_TARGET_ITEM_FIRST + DYNAMIC_OBJECT_COUNT,
    NPC_TARGET_PRIORITY_COUNT = NPC_TARGET_ACTOR_COUNT * NPC_TARGET_COUNT
};

typedef struct NPC_TARGET
{
    uint32 role;
    /* Actor or item slot according to role, with full32 NONE encoding */
    uint32 idx;
} NPC_TARGET;

extern NPC_TARGET npc_targets[NPC_BEHAVIOR_ACTOR_COUNT];
extern sint32 npc_target_distances[NPC_BEHAVIOR_ACTOR_COUNT];
extern sint32 npc_target_priorities[NPC_TARGET_PRIORITY_COUNT];

GDB_CALL sint32 npc_find_target_conflict(sint32 idx, sint32 target);
GDB_CALL void npc_reset_target_priorities(sint32 idx);
GDB_CALL sint32 npc_select_target(sint32 idx);
GDB_CALL sint32 npc_rank_targets(sint32 idx);
GDB_CALL sint32 npc_update_target(sint32 idx);
GDB_CALL uint32 npc_balance_targets(void);

#endif
