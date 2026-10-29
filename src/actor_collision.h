#ifndef ACTOR_COLLISION_H
#define ACTOR_COLLISION_H

#include "actors.h"
#include "regions.h"
#include "collision.h"

struct EFFECT;
struct MODEL_OBJECT;

typedef struct ACTOR_COLLISION
{
    sint16 radius[ACTOR_TYPE_COUNT];
    /* Counts calls across recursion and is not decremented on return */
    uint32 pass_count;
    uint32 player_hits_enabled;
    /* Published after the low-health reaction; no original reader is identified */
    uint32 reaction_deadline;
} ACTOR_COLLISION;

extern ACTOR_COLLISION actor_col;

void actor_correct_obstacles(sint32 idx, uint32 *x, uint32 *z);

sint32 actors_resolve_overlaps(sint32 idx);
sint32 actor_process_hits(sint32 idx, COLLISION_POINT *contact);
sint32 actor_col_event(COLLISION_REGION *obstacle, sint32 idx);

/* Transient vehicle shape: Q16 coordinates and a full-turn sixteen-bit yaw */
typedef struct VEHICLE_COLLISION_SHAPE
{
    uint32 x, z;
    uint16 yaw;
} VEHICLE_COLLISION_SHAPE;

uint32 actors_collide_vehicle(COLLISION_REGION *region, const struct MODEL_OBJECT *model);
uint32 actors_collide_vehicle_shape(COLLISION_REGION *region, const VEHICLE_COLLISION_SHAPE *vehicle);

/* Native caller-owned projectile values; no pool storage or wire padding */
typedef struct PROJECTILE_SOURCE
{
    uint32 kind, height;
    uint16 angle;
    sint16 owner;
} PROJECTILE_SOURCE;

sint32 actor_projectile_impact(uint32 world_x, uint32 world_z, const struct MODEL_OBJECT *model, const struct EFFECT *effect);
sint32 actor_projectile_impact_src(uint32 world_x, uint32 world_z, const PROJECTILE_SOURCE *projectile);

sint32 actors_apply_radial_damage(uint32 world_x, uint32 world_z, const struct EFFECT *effect, const struct MODEL_OBJECT *model);

sint32 actor_process_stage_contact(COLLISION_POINT *contact, sint32 idx);

sint32 actor_projectile_contact(sint32 owner, COLLISION_POINT *contact, uint32 kind);

sint32 actor_process_attachment_hits(sint32 idx, sint32 attachment);

sint32 npc_process_weapon_hits(sint32 idx);

sint32 actors_apply_nearby_contact(sint32 attacker, uint32 world_x, uint32 world_z);

sint32 actor_animation_blocks_hits(uint32 state);

#endif
