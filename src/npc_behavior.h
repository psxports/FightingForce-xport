#ifndef NPC_BEHAVIOR_H
#define NPC_BEHAVIOR_H

#include "xport.h"
#include "geometry_math.h"
#include "animation.h"

enum
{
    NPC_AI_CAPABILITY_COUNT = 46,
    NPC_AI_STORAGE_WORD_COUNT = 1536,
    NPC_AI_WORD_MASK = 0x3fffffff,
    NPC_BEHAVIOR_TYPE_COUNT = 54,
    NPC_BEHAVIOR_NPC_PROFILE_COUNT = 46,
    NPC_BEHAVIOR_PLAYER_COUNT = 2,
    NPC_BEHAVIOR_ROWS_PER_PROFILE = 5,
    NPC_BEHAVIOR_ROW_COUNT = (NPC_BEHAVIOR_NPC_PROFILE_COUNT + NPC_BEHAVIOR_PLAYER_COUNT) * NPC_BEHAVIOR_ROWS_PER_PROFILE,
    NPC_BEHAVIOR_ACTOR_COUNT = 8,
    /* Preserve low32 byte-offset wrapping in row-index units */
    NPC_BEHAVIOR_ROW_MASK = 0x1fffffff
};

typedef struct NPC_BEHAVIOR_ROW
{
    uint8 capability;
    uint8 hit_delay;
    uint8 recovery_delay;
    uint8 reaction_threshold;
    uint8 pickup_bias;
    uint8 damage_out;
    uint8 health_units;
    uint8 damage_in;
} NPC_BEHAVIOR_ROW;

typedef struct NPC_BEHAVIOR_REFERENCE
{
    NPC_BEHAVIOR_ROW *rows;
    uint32 row_count;
    uint32 first;
} NPC_BEHAVIOR_REFERENCE;

typedef union NPC_ACTION_PROGRESS
{
    uint32 projectile_hits;
    uint32 landing_item_idx;
} NPC_ACTION_PROGRESS;

typedef union NPC_SPECIAL
{
    struct
    {
        uint32 speed;
        uint32 voice[3];
    } bike;

    struct
    {
        uint32 speed;
        uint32 phase;
        uint32 rest_until;
        uint32 effect_height;
    } attack;

    uint32 van_voice;
} NPC_SPECIAL;

typedef struct NPC_ATTACK_GEOMETRY
{
    FIXED_VEC3 origin;
    FIXED_VEC3 offset;
} NPC_ATTACK_GEOMETRY;

extern const NPC_ATTACK_GEOMETRY npc_attack_geometry_defaults;
extern NPC_ATTACK_GEOMETRY npc_attack_geometry;
extern const sint32 npc_fall_speed_limit_default;
extern sint32 npc_fall_speed_limit;
extern uint16 npc_drop_phase;
extern uint32 npc_update_idx;

typedef struct NPC_BEHAVIOR_ACTOR
{
    NPC_BEHAVIOR_REFERENCE profile;
    uint32 behavior;
    uint32 command;
    uint32 idle_animation;
    uint32 move_animation;
    /* Separate from the shared action/electric deadline */
    uint32 transition_until;
    uint32 transition_state;
    NPC_ACTION_PROGRESS progress;
    sint8 fall_speed;
    uint32 reaction_until;
    NPC_SPECIAL special;
} NPC_BEHAVIOR_ACTOR;

typedef struct NPC_BEHAVIOR_CYCLE
{
    uint32 progress;
    uint32 advance;
} NPC_BEHAVIOR_CYCLE;

extern NPC_BEHAVIOR_CYCLE npc_behavior_cycle;

/* Mutable MVS bytecode views share one retained resource backing */
typedef struct NPC_AI_CAPABILITY
{
    uint32 choice_count;
    uint32 *choices;
    uint32 choice_capacity;
    uint32 *code;
    uint32 code_capacity;
    uint32 weapon_flags;
} NPC_AI_CAPABILITY;

/* Serialized source file remains distinct from mutable decoded command streams */
extern uint32 npc_behavior_file[NPC_AI_STORAGE_WORD_COUNT];
extern uint32 npc_ai_storage[NPC_AI_STORAGE_WORD_COUNT];
extern NPC_AI_CAPABILITY npc_ai_capabilities[NPC_AI_CAPABILITY_COUNT];

extern NPC_BEHAVIOR_ROW npc_behavior_rows[NPC_BEHAVIOR_ROW_COUNT];
extern NPC_BEHAVIOR_ACTOR npc_behavior_actors[NPC_BEHAVIOR_ACTOR_COUNT];

void npc_behavior_load(void);
sint32 npc_base_animation(sint32 idx);
sint32 npc_start_animation(sint32 idx, ANIMATION_BANK_ID table, sint32 state);

#endif
