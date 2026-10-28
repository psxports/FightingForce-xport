#ifndef REGIONS_H
#define REGIONS_H

#include "xport.h"
#include "actors.h"
#include "world_objects.h"

enum
{
    COLLISION_REGION_COUNT = 128,
    COLLISION_NAMED_REGION_COUNT = 11
};

typedef union REGION_GEOMETRY
{
    struct
    {
        sint16 min_x, min_z, max_x, max_z;
    } axis;

    struct
    {
        sint16 half_x, half_z, center_x, center_z;
    } box;

    /* The second halfword retains the original map edge, but is not a radius */
    struct
    {
        sint16 radius, edge_z, center_x, center_z;
    } circle;

    /* Shared coordinate stores precede shape-specific configuration and decoding */
    sint16 coord[4];
} REGION_GEOMETRY;

typedef struct COLLISION_REGION
{
    REGION_GEOMETRY geometry;
    sint16 trigger_kind;
    sint16 shape;
    sint16 col_kind;
    sint16 model_type;
    sint16 hits;
    sint16 alternate_attack;
    WORLD_OBJECT_REFERENCE object;
} COLLISION_REGION;

typedef struct ACTOR_REGION_CONTACT
{
    COLLISION_REGION *region;
    /* Flags accumulate during the current region scan; heading retains its last value */
    uint16 flags;
    sint16 heading;
} ACTOR_REGION_CONTACT;

typedef struct COLLISION_REGIONS
{
    COLLISION_REGION data[COLLISION_REGION_COUNT];
    uint32 count;
    COLLISION_REGION *sel;
    COLLISION_REGION *named[COLLISION_NAMED_REGION_COUNT];
    ACTOR_REGION_CONTACT actor[ACTOR_COUNT];
    uint32 local_x, local_z;
    uint32 side;
} COLLISION_REGIONS;

extern COLLISION_REGIONS col_regions;

typedef enum REGION_TRIGGER_ID
{
    REGION_TRIGGER_LIGHT_WARM,
    REGION_TRIGGER_LIGHT_SHADE,
    REGION_TRIGGER_LIGHT_RADIAL,
    REGION_TRIGGER_LIGHT_GREEN,
    REGION_TRIGGER_LIGHT_RED,
    REGION_TRIGGER_LIGHT_STRIP,
    REGION_TRIGGER_LIGHT_DARK,
    REGION_TRIGGER_LIGHT_ADD,
    REGION_TRIGGER_FACE_ANGLE0,
    REGION_TRIGGER_FACE_ANGLE1024,
    REGION_TRIGGER_FACE_ANGLE512,
    REGION_TRIGGER_FACE_ANGLE1536,
    REGION_TRIGGER_USE_ITEM,
    REGION_TRIGGER_LIGHT_PULSE,
    REGION_TRIGGER_COUNT
} REGION_TRIGGER_ID;

typedef struct REGION_TRIGGER
{
    /* The actor index addresses the native actor-domain metadata */
    uint32 (*call)(COLLISION_REGION *region, sint32 actor);
} REGION_TRIGGER;

typedef const REGION_TRIGGER *REGION_TRIGGER_REFERENCE;

enum
{
    REGION_KIND_COUNT = 40
};

extern REGION_TRIGGER_REFERENCE region_triggers[REGION_KIND_COUNT];
extern const REGION_TRIGGER region_trigger_catalog[REGION_TRIGGER_COUNT];

GDB_CALL sint32 regions_find_obstacle(uint32 x, uint32 z);
GDB_CALL uint32 regions_apply_actor(sint32 idx);
GDB_CALL uint32 regions_update_pickup(sint32 idx, uint32 retained_xyz[3]);

GDB_CALL void regions_disable_object(const MODEL_OBJECT *object);

GDB_CALL sint32 regions_find_projectile_obstacle(uint32 world_x, uint32 world_z);

GDB_CALL sint32 regions_query_alternate_attack(uint32 x, uint32 z);

#endif
