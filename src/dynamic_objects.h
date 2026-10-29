#ifndef DYNAMIC_OBJECTS_H
#define DYNAMIC_OBJECTS_H

#include "xport.h"
#include "model_object.h"

enum
{
    DYNAMIC_OBJECT_COUNT = 64
};

enum
{
    DYNAMIC_ITEM_TYPE_FIRST = 476,
    DYNAMIC_ITEM_TYPE_COUNT = 87
};

typedef union ITEM_PROPERTIES
{
    uint32 packed;

    struct
    {
        uint16 capabilities;
        sint16 damage;
    };
} ITEM_PROPERTIES;

extern const ITEM_PROPERTIES item_properties[DYNAMIC_ITEM_TYPE_COUNT];

typedef struct ITEM_CORRECTION
{
    sint16 pitch, roll, yaw;
    sint16 x, y, z;
} ITEM_CORRECTION;

extern ITEM_CORRECTION item_corrections[DYNAMIC_ITEM_TYPE_COUNT];
extern sint8 dynamic_item_uses[DYNAMIC_ITEM_TYPE_COUNT];
extern MODEL_OBJECT dynamic_objects[DYNAMIC_OBJECT_COUNT];

uint32 object_update_expiring(MODEL_OBJECT *model);

sint32 object_update_physics(MODEL_OBJECT *item);

uint32 object_apply_damage(sint32 attacker, sint32 victim, uint32 kind);

sint32 dynamic_objects_spawn(uint32 kind, uint32 x, uint32 y, uint32 z);
sint32 dynamic_objects_spawn_randomized(sint32 kind, uint32 x, uint32 y, uint32 z, uint32 angle);
sint32 dynamic_objects_spawn_item(sint32 type, uint32 x, uint32 y, uint32 z, uint32 angle);
sint32 dynamic_objects_spawn_oriented(uint32 kind, uint32 x, uint32 y, uint32 z, uint32 angle);
uint32 dynamic_objects_spawn_transient(uint32 type, uint32 x, uint32 y, uint32 z);
void dynamic_objects_settle_debris(MODEL_OBJECT *item);
sint32 dynamic_objects_update(void);
sint32 dynamic_objects_draw(void);

sint32 object_update_bouncing_drop(MODEL_OBJECT *model);

void object_update_rotation_shake(MODEL_OBJECT *model);
void object_apply_rotations(const MODEL_OBJECT *model);

sint32 dynamic_objects_allocate(void);

sint32 object_update_ballistic_spin(MODEL_OBJECT *model);

uint32 object_update_drifting_spin(MODEL_OBJECT *model);

sint32 object_update_falling_panel(MODEL_OBJECT *model);

uint32 object_update_falling_panel_sideways(MODEL_OBJECT *model);

uint32 object_update_ground_bounce(MODEL_OBJECT *model);

uint32 object_update_shake(MODEL_OBJECT *model);

uint32 object_update_sliding_bounce(MODEL_OBJECT *model);

uint32 object_update_spinning_landing(MODEL_OBJECT *model);

uint32 object_update_stationary_bounce(MODEL_OBJECT *model);

uint32 object_update_gate_shake(MODEL_OBJECT *model);

uint32 object_update_gate_panels_shake(MODEL_OBJECT *model);

uint32 object_start_gate_panels_shake(MODEL_OBJECT *model);

uint32 object_update_falling_gate(MODEL_OBJECT *model);

sint32 object_update_gate_fragment(MODEL_OBJECT *model);

uint32 object_update_oscillation(MODEL_OBJECT *model);

uint32 object_update_compression(MODEL_OBJECT *model);

sint32 object_update_wobble(MODEL_OBJECT *model);

intptr object_update_wobble_debris(MODEL_OBJECT *model);

sint32 object_update_wobble_particles(MODEL_OBJECT *model);

intptr object_update_smoke_emitter(MODEL_OBJECT *model);

sint32 object_update_explosive(MODEL_OBJECT *item);

sint32 object_update_island_fragment(MODEL_OBJECT *model);

uint32 dynamic_objects_spawn_spinning(uint32 kind, uint32 x, uint32 y, uint32 z, uint32 angle);

uint32 object_update_rising_spr(MODEL_OBJECT *model);

uint32 object_update_oscillating_spr(MODEL_OBJECT *model);

uint32 object_update_orbit(MODEL_OBJECT *model);

uint32 object_update_wall_fragment(MODEL_OBJECT *model);

#endif
