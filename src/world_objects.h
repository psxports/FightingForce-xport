#ifndef WORLD_OBJECTS_H
#define WORLD_OBJECTS_H

#include "xport.h"
#include "resources.h"
#include "model_object.h"

struct EFFECT;

enum
{
    WORLD_OBJECT_STAGE_COUNT = 36,
    WORLD_OBJECT_COUNT = 128,
    WORLD_TRACKED_COUNT = 8
};

typedef struct WORLD_OBJECTS
{
    uint32 count;
    /* Retains localization context until the linked group publishes its index */
    uint32 linked_first;
    uint32 tracked_count;
    WORLD_OBJECT_REFERENCE tracked[WORLD_TRACKED_COUNT];
    WORLD_OBJECT_REFERENCE relative_scene;
    MODEL_OBJECT object[WORLD_OBJECT_COUNT];
} WORLD_OBJECTS;

extern WORLD_OBJECTS world_objects;

typedef enum WORLD_OBJECT_TYPE_GROUP_ID
{
    WORLD_OBJECT_TYPES_NONE,
    WORLD_OBJECT_TYPES_STAGE0,
    WORLD_OBJECT_TYPES_STAGE1,
    WORLD_OBJECT_TYPES_STAGE2,
    WORLD_OBJECT_TYPES_STAGE3,
    WORLD_OBJECT_TYPES_STAGE4,
    WORLD_OBJECT_TYPES_STAGE5,
    WORLD_OBJECT_TYPES_STAGE6,
    WORLD_OBJECT_TYPES_STAGE7,
    WORLD_OBJECT_TYPES_STAGE8,
    WORLD_OBJECT_TYPES_STAGE9,
    WORLD_OBJECT_TYPES_STAGE10,
    WORLD_OBJECT_TYPES_STAGE11,
    WORLD_OBJECT_TYPES_STAGE12,
    WORLD_OBJECT_TYPES_STAGE14,
    WORLD_OBJECT_TYPES_STAGE15,
    WORLD_OBJECT_TYPES_STAGE16,
    WORLD_OBJECT_TYPES_STAGE17,
    WORLD_OBJECT_TYPES_STAGE18,
    WORLD_OBJECT_TYPES_STAGE19,
    WORLD_OBJECT_TYPES_STAGE22,
    WORLD_OBJECT_TYPES_STAGE23,
    WORLD_OBJECT_TYPES_STAGE24,
    WORLD_OBJECT_TYPES_SEQUENCE1,
    WORLD_OBJECT_TYPES_SEQUENCE4,
    WORLD_OBJECT_TYPES_SEQUENCE6,
    WORLD_OBJECT_TYPES_SEQUENCE7,
    WORLD_OBJECT_TYPES_FRONTEND,
    WORLD_OBJECT_TYPES_COUNT
} WORLD_OBJECT_TYPE_GROUP_ID;

extern WORLD_OBJECT_TYPE_GROUP_ID world_object_type_grps[WORLD_OBJECT_STAGE_COUNT];
extern WORLD_OBJECT_TYPE_GROUP_ID world_object_active_types;

enum
{
    WORLD_REGION_KIND_COUNT = 40
};

typedef struct WORLD_REGION_KIND
{
    sint8 mode;
    sint8 flags;
} WORLD_REGION_KIND;

extern sint16 world_object_height[MODEL_TYPE_COUNT];
extern WORLD_REGION_KIND world_region_kinds[WORLD_REGION_KIND_COUNT];

GDB_CALL uint32 world_objects_bob(MODEL_OBJECT *item);
GDB_CALL uint32 world_objects_update_traffic(MODEL_OBJECT *model);
GDB_CALL uint32 world_objects_draw_tank(const MODEL_OBJECT *item);
GDB_CALL void world_objects_roll(MODEL_OBJECT *item);
GDB_CALL uint32 world_objects_load_map(RESOURCE_VIEW map);
GDB_CALL uint32 world_objects_expand(uint32 next_idx, uint32 type, uint32 x, uint32 z);
GDB_CALL uint32 world_objects_adjust(uint32 next_idx, uint32 type, uint32 x, uint32 z);
GDB_CALL uint32 world_objects_init_special(uint32 next_idx, uint32 type);

GDB_CALL uint32 world_objects_build_regions(void);
GDB_CALL sint32 world_objects_draw(void);

GDB_CALL uint32 object_draw_base_compressed(MODEL_OBJECT *item);

GDB_CALL uint32 object_draw_compressed(MODEL_OBJECT *item);

GDB_CALL uint32 world_objects_draw_compressed_car(MODEL_OBJECT *item);

GDB_CALL uint32 world_objects_draw_car(const MODEL_OBJECT *model);

GDB_CALL sint32 world_objects_start_army_trucks(void);
GDB_CALL sint32 world_objects_update_army_truck(MODEL_OBJECT *model);
GDB_CALL uint32 world_objects_update_army_truck_wreck(MODEL_OBJECT *model);

GDB_CALL sint32 world_objects_break_near_attachment(sint32 idx, sint32 attachment);

GDB_CALL uint32 world_objects_update_moving_emitter(MODEL_OBJECT *model);
GDB_CALL uint32 world_objects_draw_moving_emitter(const MODEL_OBJECT *item);

GDB_CALL intptr world_objects_update_burning_bin(MODEL_OBJECT *model);

GDB_CALL sint32 world_objects_update_water_cannon(MODEL_OBJECT *model);
GDB_CALL intptr world_objects_update_truck_fire(MODEL_OBJECT *model);

GDB_CALL sint32 world_objects_update_falling_door(MODEL_OBJECT *item);

GDB_CALL uint32 world_objects_update_bus_wreck(MODEL_OBJECT *model);

GDB_CALL sint32 world_objects_update_lifting_door(MODEL_OBJECT *model);

GDB_CALL uint32 world_objects_update_aircraft(MODEL_OBJECT *model);

GDB_CALL uint32 world_objects_update_scrolling_wall(MODEL_OBJECT *model);

GDB_CALL uint32 world_objects_update_radar(MODEL_OBJECT *item);

GDB_CALL struct EFFECT *world_objects_update_hydrant_spray(const MODEL_OBJECT *model);

#endif
