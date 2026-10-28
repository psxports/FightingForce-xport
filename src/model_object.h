#ifndef MODEL_OBJECT_H
#define MODEL_OBJECT_H

#include "geometry_math.h"

typedef struct WORLD_OBJECT_REFERENCE
{
    uint32 idx;
    /* Absence is distinct from world slot0 */
    uint8 present;
} WORLD_OBJECT_REFERENCE;

typedef union OBJECT_ANGLES
{
    struct
    {
        sint16 yaw, pitch, roll;
    };

    struct
    {
        uint16 yaw_bits, pitch_bits, roll_bits;
    };

    sint16 v[3];
    uint16 bits[3];
} OBJECT_ANGLES;

typedef struct FALLING_PANEL
{
    sint16 initialized;
    sint16 speed;
    sint16 vertical_speed;
    sint16 pitch_step;
    sint16 roll_step;
    /* Only the normal panel uses this signed retirement/blink timer */
    sint16 retire_ticks;
    /* Shared generic physics owns these additional motion fields */
    uint16 physics_obstacle_flags;
    sint16 physics_bounce_count;
} FALLING_PANEL;

typedef struct MODEL_SPRITE
{
    sint16 frame;
    sint16 speed;
    sint16 rise;
    sint16 vertical_speed;
    /* Shared generic physics owns these additional motion fields */
    sint16 physics_actor_idx;
    sint16 physics_bounce_count;
    uint16 physics_obstacle_flags;
} MODEL_SPRITE;

typedef struct BALLISTIC_DEBRIS
{
    sint16 speed;
    sint16 vertical_speed;
    sint16 pitch_step;
    sint16 roll_step;
    /* Shared generic physics owns these additional motion fields */
    sint16 physics_actor_idx;
    sint16 physics_bounce_count;
    uint16 physics_obstacle_flags;
} BALLISTIC_DEBRIS;

typedef struct DRIFTING_DEBRIS
{
    /* The shared pickup/default draw reads the original signed flag word */
    sint16 flags;
    sint16 speed;
    sint16 phase;
    /* Shared generic physics owns these additional motion fields */
    sint16 physics_actor_idx;
    sint16 physics_bounce_count;
    sint16 physics_vertical_speed;
    uint16 physics_obstacle_flags;
} DRIFTING_DEBRIS;

typedef struct OBJECT_SHAKE
{
    sint16 ticks;
    /* The shared dynamic renderer also consumes this signed speed */
    sint16 speed;
    sint16 ox;
    sint16 oz;
    /* Shared generic physics owns these additional motion fields */
    sint16 physics_bounce_count;
    sint16 physics_vertical_speed;
    uint16 physics_obstacle_flags;
} OBJECT_SHAKE;

typedef struct FALLING_GATE
{
    sint16 pitch_speed;
    sint16 speed;
    sint16 bounces;
    /* Retained shake origin until the original reset or reuse */
    sint16 ox;
    /* Shared generic physics owns these additional motion fields */
    sint16 physics_bounce_count;
    sint16 physics_vertical_speed;
    uint16 physics_obstacle_flags;
} FALLING_GATE;

typedef struct GATE_FRAGMENT
{
    sint16 pitch_speed;
    sint16 speed;
    /* Shared generic physics owns these additional motion fields */
    sint16 physics_spin;
    sint16 physics_actor_idx;
    sint16 physics_bounce_count;
    sint16 physics_vertical_speed;
    uint16 physics_obstacle_flags;
} GATE_FRAGMENT;

typedef struct ROTATION_SHAKE
{
    sint16 phase;
    sint16 amplitude;
    sint16 axis;
    sint16 phase_step;
    WORLD_OBJECT_REFERENCE linked;
    /* Shared generic physics owns these additional motion fields */
    sint16 physics_vertical_speed;
    uint16 physics_obstacle_flags;
} ROTATION_SHAKE;

typedef struct BOUNCING_DROP
{
    /* The shared dynamic picker and renderer read this signed word */
    sint16 flags;
    sint16 speed;
    sint16 released;
    sint16 actor_idx;
    sint16 bounces;
    sint16 vertical_speed;
    uint16 obstacle_flags;
} BOUNCING_DROP;

typedef struct BOUNCING_PART
{
    sint16 bounces;
    sint16 speed;
    sint16 vertical_speed;
    /* Shared generic physics owns these additional motion fields */
    sint16 physics_spin;
    sint16 physics_actor_idx;
    sint16 physics_bounce_count;
    uint16 physics_obstacle_flags;
} BOUNCING_PART;

typedef struct TRAFFIC_VEHICLE
{
    /* Shared dynamic readers consume the signed original flag word */
    sint16 flags;
    sint16 speed;
    /* Shared generic physics owns these additional motion fields */
    sint16 physics_spin;
    sint16 physics_actor_idx;
    sint16 physics_bounce_count;
    sint16 physics_vertical_speed;
    uint16 physics_obstacle_flags;
} TRAFFIC_VEHICLE;

typedef struct TANK_OBJECT
{
    /* Shared dynamic readers retain the signed flag and speed words */
    sint16 flags;
    sint16 speed;
    sint16 turret_yaw;
    /* Shared generic physics owns these additional motion fields */
    sint16 physics_actor_idx;
    sint16 physics_bounce_count;
    sint16 physics_vertical_speed;
    uint16 physics_obstacle_flags;
} TANK_OBJECT;

typedef struct OSCILLATING_OBJECT
{
    /* Moving emitter stores its path origin in the common motion words */
    union
    {
        sint16 roll_speed, base_x, spray_phase, flags, retire_ticks;
    };

    union
    {
        sint16 speed, base_z, door_height;
    };

    /* Truck fire retains the original three-word point selection */
    union
    {
        sint16 pitch_speed, point_word, door_phase;
    };

    union
    {
        sint16 phase, render_ticks;
    };

    union
    {
        sint16 voice_plus_one, sound_played;
    };

    /* Timer in the common update, then pitch in the compression update */
    sint16 sound_clock;
    /* Shared generic physics owns these additional motion fields */
    uint16 physics_obstacle_flags;
} OSCILLATING_OBJECT;

typedef struct COMPANION_MODEL
{
    sint16 flags;
    sint16 speed;
    /* The compound destruction passes this value directly to stop */
    sint16 voice;
    /* Shared generic physics owns these additional motion fields */
    sint16 physics_spin;
    sint16 physics_actor_idx;
    sint16 physics_vertical_speed;
    uint16 physics_obstacle_flags;
} COMPANION_MODEL;

typedef struct LAB_TANK_OBJECT
{
    sint16 flags;
    sint16 speed;
    /* Retain the generic spinning launch words independently of sound channels */
    sint16 pitch_speed;
    sint16 vertical_speed;
    /* The primary sound is original30, the second sound is original28 */
    sint16 voice_plus_one[2];
    /* Shared generic physics owns these additional motion fields */
    uint16 physics_obstacle_flags;
} LAB_TANK_OBJECT;

typedef struct WOBBLING_OBJECT
{
    /* The shared dynamic flag reader consumes this signed timer too */
    sint16 ticks;
    sint16 speed;
    sint16 emit_ticks;
    /* Kind410 retains the predecessor fields and adds this sound channel */
    sint16 voice_plus_one;
    /* Shared generic physics owns these additional motion fields */
    sint16 physics_actor_idx;
    sint16 physics_vertical_speed;
    uint16 physics_obstacle_flags;
} WOBBLING_OBJECT;

typedef struct PHYSICS_OBJECT
{
    /* Settling debris reuses its pitch speed as item life at the kind transition */
    union
    {
        sint16 life, pitch_speed, fall_speed, reverse_ticks, wait_ticks, roll_step;
    };

    sint16 speed;

    /* Kind448 sliding speed and projectile target share the generic spin word */
    union
    {
        sint16 spin, target_idx, slide_speed, impact_count, blast_tick, fork_height, flight_phase;
    };

    sint16 actor_idx;

    union
    {
        sint16 bounce_count, reverse;
    };

    union
    {
        sint16 vertical_speed, heading;
    };

    uint16 obstacle_flags;
} PHYSICS_OBJECT;

typedef struct ARMY_TRUCK
{
    sint16 flags;
    sint16 speed;
    sint16 active;
    sint16 turn_step;
    /* Retain the generic dynamic-physics words at the shared allocation boundary */
    sint16 bounce_count;
    sint16 vertical_speed;
    uint16 obstacle_flags;
} ARMY_TRUCK;

typedef struct AIR_BIKE
{
    struct
    {
        sint16 x, z;
    } target[2];

    sint16 voice_plus_one;
    sint16 phase;
    sint16 actor_idx;
} AIR_BIKE;

typedef struct SCENE_VEHICLE
{
    /* Retain the shared flag and launch words at generic object boundaries */
    sint16 flags;
    sint16 speed;

    union
    {
        sint16 start_tick, departure_step, motion_active;
    };

    sint16 voice_plus_one[2];
    sint16 vertical_speed;
    uint16 obstacle_flags;
} SCENE_VEHICLE;

typedef struct SCENE_HELICOPTER
{
    /* Generic launch words retain their own meanings outside the scene callback */
    sint16 flags;
    sint16 speed;
    sint16 spin;

    /* Scenario341 activation and intro146 steering share this native word */
    union
    {
        sint16 active, yaw_step;
    };

    sint16 voice_plus_one;
    sint16 vertical_speed;
    uint16 obstacle_flags;
} SCENE_HELICOPTER;

typedef struct COMBAT_HELICOPTER
{
    sint16 bob_phase;
    sint16 shot_distance;
    sint16 phase;
    sint16 voice_plus_one[2];
    sint16 hover_height;
    sint16 shot_passes;
} COMBAT_HELICOPTER;

typedef struct PATROL_HELICOPTER
{
    sint16 voice_plus_one;
    sint16 speed;
    sint16 phase;
    sint16 rotor_angle;
    sint16 mode;
    /* Generic motion and obstacle words retain their boundary meanings */
    sint16 vertical_speed;
    uint16 obstacle_flags;
} PATROL_HELICOPTER;

enum
{
    MODEL_PAYLOAD_UNMIGRATED,
    MODEL_PAYLOAD_PANEL,
    MODEL_PAYLOAD_SIDE_PANEL,
    MODEL_PAYLOAD_SPRITE,
    MODEL_PAYLOAD_BALLISTIC,
    MODEL_PAYLOAD_DRIFT,
    MODEL_PAYLOAD_SHAKE,
    MODEL_PAYLOAD_GATE_FALL,
    MODEL_PAYLOAD_GATE_FRAGMENT,
    MODEL_PAYLOAD_ROTATION,
    MODEL_PAYLOAD_BOUNCING_DROP,
    MODEL_PAYLOAD_BOUNCING_PART,
    MODEL_PAYLOAD_TRAFFIC,
    MODEL_PAYLOAD_TANK,
    MODEL_PAYLOAD_OSCILLATION,
    MODEL_PAYLOAD_COMPANION,
    MODEL_PAYLOAD_LAB_TANK,
    MODEL_PAYLOAD_WOBBLE,
    MODEL_PAYLOAD_PHYSICS,
    MODEL_PAYLOAD_ARMY_TRUCK,
    MODEL_PAYLOAD_AIR_BIKE,
    MODEL_PAYLOAD_SCENE_VEHICLE,
    MODEL_PAYLOAD_SCENE_HELICOPTER,
    MODEL_PAYLOAD_COMBAT_HELICOPTER,
    MODEL_PAYLOAD_PATROL_HELICOPTER,
    MODEL_PAYLOAD_WAVE,
    MODEL_PAYLOAD_TRAIN,
    MODEL_PAYLOAD_SUSPENDED
};

typedef struct WAVE_MODEL
{
    sint16 flags;
    sint16 speed;
    sint16 phase;
    /* Shared generic physics owns these additional motion fields */
    sint16 physics_actor_idx;
    sint16 physics_bounce_count;
    sint16 physics_vertical_speed;
    uint16 physics_obstacle_flags;
} WAVE_MODEL;

typedef struct TRAIN_MODEL
{
    union
    {
        sint16 wait_ticks, flags;
    };

    sint16 speed;
    /* Generic spinning construction retains its launch fields */
    sint16 spin;
    sint16 state;
    sint16 voice_plus_one;
    sint16 vertical_speed;
    /* Shared generic physics owns these additional motion fields */
    uint16 physics_obstacle_flags;
} TRAIN_MODEL;

typedef struct SUSPENDED_MODEL
{
    sint16 flags;
    /* Generic construction and drawing share the original speed word */
    sint16 actor_idx;
    sint16 phase;
    /* Generic scene activation writes the original word28 */
    sint16 scene_active;
    sint16 vertical_speed;
    /* Shared generic physics owns these additional motion fields */
    sint16 physics_bounce_count;
    uint16 physics_obstacle_flags;
} SUSPENDED_MODEL;

typedef struct MODEL_OBJECT
{
    uint32 kind;
    FIXED_VEC3 pos;
    /* Preserve the angle units of each original producer and consumer */
    OBJECT_ANGLES angles;
    uint8 payload_kind;

    union
    {
        FALLING_PANEL panel;
        MODEL_SPRITE spr;
        BALLISTIC_DEBRIS ballistic;
        DRIFTING_DEBRIS drift;
        OBJECT_SHAKE shake;
        FALLING_GATE gate_fall;
        GATE_FRAGMENT gate_fragment;
        ROTATION_SHAKE rotation;
        BOUNCING_DROP drop;
        BOUNCING_PART bounce;
        TRAFFIC_VEHICLE traffic;
        TANK_OBJECT tank;
        /* Own common motion until a concrete payload is published */
        OSCILLATING_OBJECT pending_motion;
        OSCILLATING_OBJECT oscillation;
        COMPANION_MODEL companion;
        LAB_TANK_OBJECT lab_tank;
        WOBBLING_OBJECT wobble;
        PHYSICS_OBJECT physics;
        ARMY_TRUCK truck;
        AIR_BIKE air_bike;
        SCENE_VEHICLE scene_vehicle;
        SCENE_HELICOPTER scene_helicopter;
        COMBAT_HELICOPTER helicopter;
        PATROL_HELICOPTER patrol;
        WAVE_MODEL wave;
        TRAIN_MODEL train;
        SUSPENDED_MODEL suspended;
    };
} MODEL_OBJECT;

#endif
