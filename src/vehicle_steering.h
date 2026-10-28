#ifndef VEHICLE_STEERING_H
#define VEHICLE_STEERING_H

#include "xport.h"
#include "geometry_math.h"
#include "model_object.h"

typedef enum VEHICLE_PROBE_ID
{
    VEHICLE_PROBE_NEAR_LEFT,
    VEHICLE_PROBE_NEAR_RIGHT,
    VEHICLE_PROBE_FAR_LEFT,
    VEHICLE_PROBE_FAR_RIGHT,
    VEHICLE_PROBE_COUNT
} VEHICLE_PROBE_ID;

enum
{
    VEHICLE_HIT_NEAR = 1,
    VEHICLE_HIT_FAR_LEFT = 2,
    VEHICLE_HIT_FAR_RIGHT = 4
};

typedef struct VEHICLE_STEERING
{
    uint32 hit_flags;
    /* Near targets retain the preceding update's probe vectors */
    FIXED_XZ probe[VEHICLE_PROBE_COUNT];
    FIXED_XZ target;
    sint16 turn_error;
} VEHICLE_STEERING;

extern VEHICLE_STEERING vehicle_steering;

GDB_CALL uint32 vehicle_steering_update(MODEL_OBJECT *model);

GDB_CALL sint32 vehicle_steering_move(MODEL_OBJECT *model);
GDB_CALL sint32 vehicle_steering_draw(MODEL_OBJECT *item);

#endif
