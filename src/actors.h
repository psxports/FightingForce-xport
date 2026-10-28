#ifndef ACTORS_H
#define ACTORS_H

#include "xport.h"
#include "geometry_math.h"

struct EFFECT;

enum
{
    ACTOR_TYPE_NONE = -1,
    ACTOR_COUNT = 8,
    ACTOR_TYPE_COUNT = 54
};

enum
{
    ACTOR_SLOT_NONE = -1
};

/* Type -1 marks a free slot; other values select game model/behavior tables */
extern sint16 actor_types[ACTOR_COUNT];

extern FIXED_VEC3 actor_positions[ACTOR_COUNT];

typedef struct ACTOR_ORIENTATION
{
    /* Yaw uses 2048 units per turn; roll uses 65536 */
    sint16 yaw, roll;
    /* Van turning and damped body rocking */
    sint16 yaw_rate;
    /* Roll integrates roll_rate >> 2 */
    sint16 roll_rate;
} ACTOR_ORIENTATION;

extern ACTOR_ORIENTATION actor_orientation[ACTOR_COUNT];

typedef struct ACTOR_HEALTH
{
    /* Bars use signed health values shifted right by two */
    sint16 hp, shown, trail, max;
    sint16 stun;
} ACTOR_HEALTH;

extern ACTOR_HEALTH actor_health[ACTOR_COUNT];

typedef struct ACTOR_MOVEMENT
{
    /* Digital input accelerates this low-word speed; player ticks decay it */
    uint32 speed;
    /* Input yaw retains sixteen bits; facing uses eleven */
    uint16 desired_yaw;
    /* Normalized state before the held-item animation switch */
    uint16 prev_state;
    sint16 van_speed, van_heading;
} ACTOR_MOVEMENT;

extern ACTOR_MOVEMENT actor_movement[ACTOR_COUNT];

typedef struct ACTOR_CONTACT
{
    sint8 last_target, last_attacker;
    /* Incoming and outgoing links have independent clearing rules */
    sint8 captor, captive;
    sint8 target_ticks, grapple_mode;
    uint8 escape_ticks;
} ACTOR_CONTACT;

extern ACTOR_CONTACT actor_contacts[ACTOR_COUNT];

typedef struct ACTOR_LIFECYCLE
{
    /* Signed delay gates motion and contact during scripted entry/actions */
    sint16 delay;
    /* Signed tick comparisons and blink bit retain the original byte bits */
    uint8 death_ticks;
    /* Low byte of the creation argument, cleared after the last van passenger */
    uint8 spawn_flag;
} ACTOR_LIFECYCLE;

extern ACTOR_LIFECYCLE actor_lifecycle[ACTOR_COUNT];

/* Pending stage-end awards retain sixteen-bit wrap and signed consumption */
extern uint16 actor_bonus_counts[ACTOR_COUNT];

/* Stage-end snapshot stays unchanged while pending awards count down */
extern uint16 actor_bonus_totals[ACTOR_COUNT];

/* Scripted vehicle/passenger links use a signed slot or ACTOR_SLOT_NONE */
extern sint16 actor_companions[ACTOR_COUNT];

typedef struct ACTOR_PAIR_MOTION
{
    /* Shared local offset or world midpoint, according to the current action */
    sint16 x, z;
    /* Signed separation and interpolation frame count */
    sint16 span, duration;
} ACTOR_PAIR_MOTION;

extern ACTOR_PAIR_MOTION actor_pair_motion[ACTOR_COUNT];

typedef struct ACTOR_TRAIL
{
    /* Signed Q2 age reads share the wrapping sixteen-bit phase */
    uint16 phase;
    sint16 mode;
} ACTOR_TRAIL;

extern ACTOR_TRAIL actor_trails[ACTOR_COUNT];

typedef struct ACTOR_COMBAT
{
    /* Grapple repetitions use signed comparisons after wrapping sixteen bits */
    uint16 combo_step;
    /* Special-attack bonus thresholds reread this signed byte */
    uint8 special_hits;
    sint8 repeat_ticks;
    /* One-based voice read as signed sixteen bits */
    sint16 run_voice;
} ACTOR_COMBAT;

extern ACTOR_COMBAT actor_combat[ACTOR_COUNT];

enum
{
    ACTOR_ELECTRIC = 1,
    ACTOR_CULLED = 2
};

typedef struct ACTOR_STATUS
{
    /* Shared deadline for AI action delays and electric effect expiry */
    uint32 deadline;
    /* Retain all bits, including the asymmetric NPC/player culling stores */
    uint32 flags;
} ACTOR_STATUS;

extern ACTOR_STATUS actor_status[ACTOR_COUNT];

/* Type-specific spacing for paired grapple positions */
extern sint16 actor_grapple_distance[ACTOR_TYPE_COUNT];
extern uint32 actor_score_vals[ACTOR_TYPE_COUNT];

GDB_CALL uint32 actors_init_score_vals(void);

typedef union ACTOR_LIGHTING
{
    struct
    {
        uint8 flags;
        sint8 r, g, b;
    };

    /* Original arc effects combine all four bytes as one numeric word */
    uint32 bits;
} ACTOR_LIGHTING;

extern ACTOR_LIGHTING actor_lighting[ACTOR_COUNT];
extern uint32 actor_ambient_shade;

typedef struct ACTOR_TARGET_DIRECTION
{
    uint32 available;
    sint16 angle;
} ACTOR_TARGET_DIRECTION;

typedef struct ACTOR_TARGETING
{
    uint32 actor_idx;
    ACTOR_TARGET_DIRECTION front, rear;
    /* Player search stores whole units; NPC search stores Q16 units */
    sint32 near_distance;
    uint32 player_idx;
    sint32 player_distance;
} ACTOR_TARGETING;

extern ACTOR_TARGETING actor_targeting;

GDB_CALL uint32 actors_find_facing_targets(sint32 idx);
GDB_CALL sint32 actors_find_nearest_targets(sint32 idx);

uint32 actors_reset(void);
uint32 actors_reset_health(void);
GDB_CALL uint32 actor_apply_damage(sint32 attacker, sint32 victim, sint32 damage);
GDB_CALL sint32 actors_update_grapple(sint32 idx, sint32 victim);
GDB_CALL sint32 actors_try_grapple(sint32 idx, sint32 victim);
GDB_CALL sint32 actors_rec_hit(sint32 idx, sint32 victim);
GDB_CALL sint32 actors_rec_death(sint32 idx);
GDB_CALL sint32 actors_detach_companion(sint32 idx);
GDB_CALL sint32 actors_capture_pair_offset(sint32 idx, sint32 partner);
GDB_CALL uint32 actors_apply_pair_offset(sint32 idx, sint32 partner);
GDB_CALL sint32 actors_update_pair_motion(sint32 idx);
GDB_CALL sint32 character_trail_clear(sint32 resource);
GDB_CALL uint32 character_trail_append(sint32 idx, sint32 attachment);
GDB_CALL struct EFFECT *actors_emit_running_attack(sint32 idx);
GDB_CALL uint32 actors_emit_arc(sint32 idx);
GDB_CALL intptr actors_emit_link(sint32 idx, sint32 other_idx);

GDB_CALL intptr actor_process_attack_effects(sint32 idx);
GDB_CALL intptr actor_process_shock_attack(sint32 idx);

GDB_CALL struct EFFECT *actor_emit_ground_effect(sint32 idx);
GDB_CALL intptr actor_ground_effect_event(sint32 idx);
GDB_CALL intptr actor_ground_effect_voice_event(sint32 idx);

GDB_CALL uint32 players_past_x(sint32 x);

GDB_CALL uint32 players_before_x(sint32 x);

GDB_CALL uint32 players_before_z(sint32 z);

GDB_CALL sint32 actor_is_defeated(sint32 idx);

GDB_CALL sint32 players_past_z(sint32 z);

GDB_CALL uint32 actor_apply_animation_damage(uint32 attacker, uint32 victim, uint32 state);

#endif
