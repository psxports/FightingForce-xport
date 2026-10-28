#ifndef ACTOR_POSE_H
#define ACTOR_POSE_H

#include "xport.h"
#include "psx.h"
#include "resources.h"
#include "actors.h"

enum
{
    ACTOR_POSE_ACTOR_COUNT = ACTOR_COUNT,
    ACTOR_POSE_TRAIL0 = ACTOR_POSE_ACTOR_COUNT,
    ACTOR_POSE_TRAIL1,
    ACTOR_POSE_COUNT,
    ACTOR_POSE_MATRIX_COUNT = 4,
    ACTOR_POSE_POINT_COUNT = 7,
    ACTOR_POSE_BONE_COUNT = 48
};

typedef union POSE_POINT
{
    struct
    {
        sint32 x, y, z;
    };

    sint32 v[3];
    /* Unsigned view for low-word arithmetic and existing vector outputs */
    uint32 bits[3];
} POSE_POINT;

typedef union POSE_ANGLES
{
    struct
    {
        sint16 roll, yaw, pitch;
    };

    sint16 v[3];
} POSE_ANGLES;

/* Game matrix packet: Q16 translations and the complete fifth COP2 word */
typedef struct POSE_MATRIX
{
    union
    {
        struct
        {
            sint16 m[3][3];
            uint16 rotation_tail;
        };

        uint32 rotation_words[5];
    };

    sint32 t[3];
} POSE_MATRIX;

typedef struct ACTOR_POSE
{
    uint32 sway_yaw;
    /* Matrix translations retain signed Q16 units */
    POSE_MATRIX matrix[ACTOR_POSE_MATRIX_COUNT];
    POSE_POINT point[ACTOR_POSE_POINT_COUNT];
    POSE_ANGLES angles[ACTOR_POSE_BONE_COUNT];
} ACTOR_POSE;

extern ACTOR_POSE actor_poses[ACTOR_POSE_COUNT];

GDB_CALL void actor_pose_load_initial_motion(sint32 idx);
GDB_CALL void actor_pose_decode(MODEL_REFERENCE model, sint32 idx, ACTOR_POSE *pose);
GDB_CALL void actor_attachment_pos(sint32 idx, sint32 attachment, uint32 output[3]);
GDB_CALL uint32 actor_place_attachment(FIXED_VEC3 *pos, sint32 idx, sint32 attachment);
void actor_pose_copy(ACTOR_POSE *to, const ACTOR_POSE *from);

#endif
