#ifndef EFFECTS_H
#define EFFECTS_H

#include "xport.h"
#include "psx.h"
#include "geometry_math.h"

struct EFFECT;
struct MODEL_OBJECT;

enum
{
    EFFECT_COUNT = 128,
    EFFECT_NORMAL_FIRST = 1,
    EFFECT_NORMAL_END = 112,
    EFFECT_ROTATE_COUNT = 16
};

typedef struct EFFECT_LINE
{
    FIXED_VEC3 pos;

    /* Signed whole-unit endpoint displacement after Q16 subtraction */
    struct
    {
        sint16 x, y, z;
    } delta;

    sint16 tex;

    union
    {
        uint32 size_bits;

        struct
        {
            sint16 size0, size1;
        };
    };

    RGBC color;
} EFFECT_LINE;

void effect_draw_line(const struct EFFECT *effect);
void effect_draw_spr(struct MODEL_OBJECT *item, struct EFFECT *effect);

typedef struct EFFECT_RIBBON
{
    FIXED_VEC3 pos;

    /* Endpoint displacement is signed when projected, not an angle */
    struct
    {
        sint16 x, y, z;
    } delta;

    sint16 frame;
} EFFECT_RIBBON;

void effect_draw_ribbon(const struct EFFECT *effect);
uint32 effect_remove_line(struct EFFECT *effect);
uint32 effect_remove_ribbon(struct EFFECT *effect);

typedef struct EFFECT_MOVING_RIBBON
{
    EFFECT_RIBBON ribbon;
    sint16 age;
    sint16 owner;
} EFFECT_MOVING_RIBBON;

sint32 effect_update_moving_ribbon(struct EFFECT *effect);
void effect_draw_moving_ribbon(const struct EFFECT *effect);

typedef struct EFFECT_SPRITE
{
    FIXED_VEC3 pos;

    /* Original motion words also supply the shared transform rotations */
    struct
    {
        sint16 x, y, z;
    } motion;

    sint16 frame;
    sint16 rise;
    /* Shockwave source actor, reloaded after nested collision calls */
    sint16 owner;
} EFFECT_SPRITE;

uint32 effect_update_dust_short(struct MODEL_OBJECT *item, struct EFFECT *effect);
uint32 effect_update_dust_long(struct EFFECT *effect);
uint32 effect_update_falling_spr(struct EFFECT *effect);
sint32 effect_update_drifting_spr(struct EFFECT *effect);
intptr effect_update_landing_spr(struct EFFECT *effect);
uint32 effect_update_spr_short(struct EFFECT *effect);
uint32 effect_update_spr_long(struct EFFECT *effect);

typedef struct EFFECT_PARTICLE
{
    FIXED_VEC3 pos;
    sint16 yaw, pitch, roll;
    sint16 radial_speed;
    uint16 age;
    /* Signed for point physics, unsigned before Q16 shifts for model debris */
    uint16 vertical_speed;
    sint16 life;
    sint16 bounce_count;
} EFFECT_PARTICLE;

/* The tag selects one payload for each native effect slot */
enum
{
    EFFECT_PAYLOAD_NONE,
    EFFECT_PAYLOAD_LINE,
    EFFECT_PAYLOAD_RIBBON,
    EFFECT_PAYLOAD_MOVING_RIBBON,
    EFFECT_PAYLOAD_SPRITE,
    EFFECT_PAYLOAD_PARTICLE
};

typedef struct EFFECT
{
    uint32 kind;
    uint8 payload_kind;

    union
    {
        EFFECT_LINE line;
        EFFECT_RIBBON ribbon;
        EFFECT_MOVING_RIBBON moving_ribbon;
        EFFECT_SPRITE spr;
        EFFECT_PARTICLE particle;
    };
} EFFECT;

extern EFFECT effects[EFFECT_COUNT];

extern uint32 effect_rotate_cursor;

EFFECT *effect_alloc(void);
EFFECT *effect_alloc_rotating(void);
uint32 effect_spawn_hit(uint32 x, uint32 y, uint32 z, uint32 angle);
uint32 effect_spawn_hit_reverse(uint32 x, uint32 y, uint32 z, uint32 angle);
uint32 effect_spawn_actor_hit(sint32 idx, uint32 x, uint32 y, uint32 z, uint32 angle);
uint32 effect_spawn_actor_hit_reverse(sint32 idx, uint32 x, uint32 y, uint32 z, uint32 angle);
uint32 effect_spawn_rotating_particle(uint32 x, uint32 y, uint32 z, uint32 angle);
sint32 effects_spawn_falling_glass(uint32 x, uint32 y, uint32 z);
uint32 effect_spawn_spark(uint32 x, uint32 y, uint32 z);
uint32 effect_spawn_glass_fragment(uint32 kind, uint32 x, uint32 y, uint32 z, uint32 angle);
uint32 effect_spawn_slow_fragment(uint32 x, uint32 y, uint32 z, uint32 angle);
uint32 effect_spawn_gate_fragment(uint32 x, uint32 y, uint32 z, uint32 angle);
uint32 effect_spawn_at_attachment(sint32 idx);
uint32 effect_spawn_spark_reverse(uint32 x, uint32 y, uint32 z, uint32 angle);
uint32 effect_spawn_point(uint32 x, uint32 y, uint32 z, uint32 angle);
uint32 effect_spawn_attachment_particle(sint32 idx, uint32 kind);
EFFECT *effect_spawn_timed(uint32 kind, uint32 x, uint32 y, uint32 z, sint32 age, sint32 a, sint32 b);
EFFECT *effect_spawn(uint32 kind, uint32 x, uint32 y, uint32 z);
intptr effect_spawn_debris(uint32 kind, uint32 x, uint32 y, uint32 z);
intptr effect_spawn_debris_aimed(uint32 kind, uint32 x, uint32 y, uint32 z, uint32 angle);
EFFECT *effect_spawn_motion(uint32 kind, uint32 x, uint32 y, uint32 z, uint32 dx, uint32 dy, uint32 dz);
sint32 effect_spawn_line(uint32 style, uint32 x, uint32 y, uint32 z, uint32 end_x, uint32 end_y, uint32 end_z, uint32 size, uint32 color);

sint32 effect_spawn_velocity(uint32 kind, uint32 x, uint32 y, uint32 z, sint32 yaw, sint32 pitch, sint32 speed);

intptr effect_update_debris(EFFECT *effect);
intptr effect_update_debris_low_gravity(EFFECT *effect);
intptr effect_update_debris_quarter_speed(EFFECT *effect);

sint32 effects_update(void);
sint32 effects_draw(void);
void effect_draw_model(const struct EFFECT *effect);
void effect_draw_oriented_model(const struct EFFECT *effect);

uint32 effect_update_blast_large(struct EFFECT *effect);

uint32 effect_update_blast(struct EFFECT *effect);

void effect_draw_blast_large(const struct EFFECT *effect);

void effect_draw_blast(const struct EFFECT *effect);

uint32 effect_update_rising_spr_short(struct MODEL_OBJECT *item, struct EFFECT *effect);

uint32 effect_update_rising_spr_long(struct MODEL_OBJECT *item, struct EFFECT *effect);

uint32 effect_update_rising_smoke(struct MODEL_OBJECT *item, struct EFFECT *effect);

uint32 effect_update_ring_large(struct EFFECT *effect);

uint32 effect_update_ring(struct EFFECT *effect);

uint32 effect_update_ring_short(struct EFFECT *effect);

void effect_draw_ring_large(const struct EFFECT *effect);

void effect_draw_ring(const struct EFFECT *effect);

void effect_draw_ring_small(const struct EFFECT *effect);

void effect_draw_rising_smoke(const struct EFFECT *effect);

sint32 effect_update_shockwave(struct EFFECT *effect);

sint32 effect_update_particle(struct EFFECT *effect);

void effect_draw_hit_particle(const struct EFFECT *effect);

void effect_draw_spark(const struct EFFECT *effect);

void effect_draw_point(const struct EFFECT *effect);

uint32 effect_update_falling_glass(struct EFFECT *effect);

void effect_draw_glass(const struct EFFECT *effect);

uint32 effect_update_slowing_particle(struct EFFECT *effect);

uint32 effect_update_stationary_particle(struct EFFECT *effect);

uint32 effect_update_short_particle(struct EFFECT *effect);

uint32 effect_update_long_particle(struct EFFECT *effect);

uint32 effect_remove_short_particle(struct EFFECT *effect);

uint32 effect_remove_long_particle(struct EFFECT *effect);

void effect_draw_rotating_particle(void);

void effect_draw_short_particle(void);

void effect_draw_long_particle(void);

sint32 effect_update_bouncing_particle(struct MODEL_OBJECT *item, struct EFFECT *effect);

void effect_draw_double_point(const struct EFFECT *effect);

void effect_draw_attachment_model(const struct EFFECT *effect);

uint32 effect_update_delayed_fragment(struct EFFECT *effect);

uint32 effect_update_fragment(struct EFFECT *effect);

uint32 effect_update_gate_fragment(struct EFFECT *effect);

uint32 effect_update_gate_fragment_velocity(struct EFFECT *effect, uint32 incoming_v1);

#endif
