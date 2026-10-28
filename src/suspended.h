#ifndef SUSPENDED_H
#define SUSPENDED_H

#include "xport.h"
#include "model_object.h"

enum
{
    SUSPENDED_SEGMENT_COUNT = 32
};

typedef struct SUSPENDED_SEGMENT
{
    /* The collapse phase reuses the angle as a falling speed */
    union
    {
        sint16 angle;
        sint16 fall_speed;
    };

    sint16 angular_velocity;
    sint16 x;
    sint16 y;
} SUSPENDED_SEGMENT;

typedef struct SUSPENDED
{
    uint32 mode;
    SUSPENDED_SEGMENT segs[SUSPENDED_SEGMENT_COUNT];
} SUSPENDED;

extern SUSPENDED suspended;

GDB_CALL uint32 suspended_update(MODEL_OBJECT *model);
GDB_CALL void suspended_draw(const MODEL_OBJECT *model);

GDB_CALL void suspended_noop(void);

#endif
