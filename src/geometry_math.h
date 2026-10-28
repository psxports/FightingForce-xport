#ifndef GEOMETRY_MATH_H
#define GEOMETRY_MATH_H

#include "xport.h"

typedef union FIXED_COORD
{
    sint32 val;
    uint32 bits;

    struct
    {
        uint16 fraction;
        sint16 whole;
    };

    uint16 half[2];
} FIXED_COORD;

typedef union FIXED_VEC3
{
    struct
    {
        FIXED_COORD x, y, z;
    };

    FIXED_COORD v[3];
    sint32 val[3];
    uint32 bits[3];
} FIXED_VEC3;

typedef union FIXED_XZ
{
    struct
    {
        FIXED_COORD x, z;
    };

    FIXED_COORD v[2];
    uint32 bits[2];
} FIXED_XZ;

enum
{
    MATH_ANGLE_COUNT = 2048,
    MATH_QUARTER_TURN = MATH_ANGLE_COUNT / 4,
    MATH_SIN_COUNT = MATH_ANGLE_COUNT + MATH_QUARTER_TURN
};

/* Q15 samples include the repeated quarter used by cosine lookups */
extern const sint16 sin_table[MATH_SIN_COUNT];

/* Signed distance output also has deliberate unsigned-bit consumers */
extern sint32 dir_distance;

uint32 math_add_checked(uint32 val, sint32 delta);
sint32 math_divide(sint32 dividend, sint32 divisor);
GDB_CALL sint32 math_sqrt_odd(uint32 val);
GDB_CALL sint32 math_dir16(sint32 x0, sint32 y0, sint32 x1, sint32 y1);
GDB_CALL sint32 math_dir11(sint32 x0, sint32 z0, sint32 x1, sint32 z1);
GDB_CALL sint32 math_distance_approx(sint32 x0, sint32 z0, sint32 x1, sint32 z1);
GDB_CALL sint32 math_distance_newton(sint32 x0, sint32 z0, sint32 x1, sint32 z1);
GDB_CALL sint32 math_sqrt_newton(sint32 val);

#endif
