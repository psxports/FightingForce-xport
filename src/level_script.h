#ifndef LEVEL_SCRIPT_H
#define LEVEL_SCRIPT_H

#include "xport.h"

enum
{
    LEVEL_SCRIPT_STAGE_COUNT = 36,
    LEVEL_SCRIPT_WORD_COUNT = 7692
};

#define LEVEL_SCRIPT_NONE 0xffffffffu

typedef enum SCENE_OBJECT_POOL
{
    SCENE_OBJECT_NONE,
    SCENE_OBJECT_WORLD,
    SCENE_OBJECT_DYNAMIC
} SCENE_OBJECT_POOL;

typedef struct SCENE_OBJECT_REFERENCE
{
    uint32 idx;
    /* The pool tag distinguishes absence from either valid slot0 */
    uint8 pool;
} SCENE_OBJECT_REFERENCE;

typedef struct LEVEL_SCRIPT_SCENE
{
    sint16 target_x;
    sint16 target_z;
    uint32 remaining_landings;
    sint32 ox;
    sint32 oz;
    uint32 event_timer;
    sint8 relative;
    SCENE_OBJECT_REFERENCE object;
    /* Initialized damping accumulators survive stage and original BSS resets */
    uint32 height_velocity;
    sint16 yaw_step;
} LEVEL_SCRIPT_SCENE;

extern LEVEL_SCRIPT_SCENE level_script_scene;

typedef struct LEVEL_WAVE
{
    uint32 remaining_enemies;
    uint32 pending_spawns;
    uint32 spawn_delay;
    uint32 population_gate;
    uint32 attacker_limit;
    uint32 base_attacker_limit;
    uint32 attacker_decay_tick;
    uint32 attacker_trigger_count;
    sint8 paused;
} LEVEL_WAVE;

extern LEVEL_WAVE level_wave;
extern const LEVEL_WAVE level_wave_defaults;

extern uint32 level_script_starts[LEVEL_SCRIPT_STAGE_COUNT];
extern uint32 level_script_cursor;

GDB_CALL sint32 level_script_count_enemies(void);
GDB_CALL sint32 level_script_update(void);

#endif
