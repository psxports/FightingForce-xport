#ifndef PROJECTION_H
#define PROJECTION_H

#include "xport.h"

typedef union PROJECTION_POINT
{
    struct
    {
        sint32 x, y, depth;

        /* Sprite callers reuse the projection status slot as their size */
        union
        {
            sint32 status;
            sint32 size;
        };
    };

    /* Indexed access retains explicit four-word copies and diagnostic output */
    sint32 v[4];
} PROJECTION_POINT;

extern uint32 proj_horizon;

GDB_CALL sint32 proj_local(sint32 x, sint32 y, sint32 z, PROJECTION_POINT *dest);
GDB_CALL void proj_update_horizon(void);

#endif
