#ifndef COLLISION_H
#define COLLISION_H

#include "xport.h"
#include "actor_pose.h"
#include "regions.h"

struct MODEL_OBJECT;

enum
{
    COLLISION_SLOT_COUNT = 26,
    COLLISION_HANDLER_COUNT = 23
};

typedef struct COLLISION_DROPS
{
    uint32 angle_cursor;
    uint32 type_cursor;
} COLLISION_DROPS;

extern COLLISION_DROPS col_drops;

uint32 col_region_build(COLLISION_REGION *region, sint32 model_type, sint32 hits, sint32 kind, sint32 padding);
uint32 col_region_configure(COLLISION_REGION *region, sint32 model_type);

typedef struct COLLISION_POINT
{
    POSE_POINT pos;
    /* Retained numeric bits or a native pointer used as the sound gate */
    uintptr_t impact_sound;
} COLLISION_POINT;

typedef enum COLLISION_SIGNATURE
{
    COLLISION_NO_ARGS,
    COLLISION_CONTACT,
    COLLISION_MUTABLE_CONTACT
} COLLISION_SIGNATURE;

typedef struct COLLISION_HANDLER
{
    COLLISION_SIGNATURE signature;

    union
    {
        uint32 (*no_args)(void);
        sint32 (*contact)(COLLISION_REGION *, struct MODEL_OBJECT *, const COLLISION_POINT *, sint32);
        sint32 (*mutable_contact)(COLLISION_REGION *, struct MODEL_OBJECT *, COLLISION_POINT *, sint32);
    } fn;
} COLLISION_HANDLER;

typedef const COLLISION_HANDLER *COLLISION_REFERENCE;

extern COLLISION_REFERENCE col_callbacks[COLLISION_SLOT_COUNT];
extern const COLLISION_HANDLER col_handlers[COLLISION_HANDLER_COUNT];

/* Actor indices are 0..ACTOR_COUNT-1, or -1 when the original source has no actor */
sint32 col_call(COLLISION_REFERENCE target, COLLISION_REGION *obstacle, struct MODEL_OBJECT *model, COLLISION_POINT *contact, sint32 actor_idx);

/* Exact original leaf, including its return delay slot; see leaf-batch.json. */
uint32 col_noop(void);

#endif
