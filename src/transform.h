#ifndef TRANSFORM_H
#define TRANSFORM_H

#include "xport.h"
#include "actor_pose.h"

enum
{
    GEOMETRY_MATRIX_COUNT = 32,
    GEOMETRY_XY_COUNT = 256,
    GEOMETRY_DEPTH_COUNT = 512
};

typedef union GEOMETRY_SCRATCH
{
    POSE_MATRIX matrix[GEOMETRY_MATRIX_COUNT];
    uint32 xy[GEOMETRY_XY_COUNT];
    sint16 depth[GEOMETRY_DEPTH_COUNT];
} GEOMETRY_SCRATCH;

enum
{
    GEOMETRY_MODEL_DEPTH_PREFIX_COUNT = 512,
    GEOMETRY_MODEL_XY_COUNT = 1536,
    GEOMETRY_MODEL_DEPTH_COUNT = 3584,
    GEOMETRY_MODEL_MATRIX_COUNT = 224
};

/* The large depth view overlaps the packed XY view after its prefix */
typedef union GEOMETRY_MODEL_BUFFER
{
    sint16 depth[GEOMETRY_MODEL_DEPTH_COUNT];
    POSE_MATRIX matrix[GEOMETRY_MODEL_MATRIX_COUNT];

    struct
    {
        sint16 depth_prefix[GEOMETRY_MODEL_DEPTH_PREFIX_COUNT];
        uint32 xy[GEOMETRY_MODEL_XY_COUNT];
    } proj;
} GEOMETRY_MODEL_BUFFER;

typedef enum GEOMETRY_MATRIX_STORAGE
{
    GEOMETRY_MATRIX_SCRATCH = 0,
    GEOMETRY_MATRIX_NULL = 2,
    GEOMETRY_MATRIX_MODEL = 3
} GEOMETRY_MATRIX_STORAGE;

typedef struct GEOMETRY_WORKSPACE
{
    GEOMETRY_SCRATCH scratch;
    GEOMETRY_MODEL_BUFFER model;
    POSE_MATRIX *matrix;
    GEOMETRY_MATRIX_STORAGE matrix_storage;
} GEOMETRY_WORKSPACE;

extern GEOMETRY_WORKSPACE geometry_workspace;

/* Matrix, packed XY and depth views share the same scratch storage */
void transform_pop(void);
void transform_push(void);
void transform_translate(sint32 x, sint32 y, sint32 z);
void transform_reload(void);
uint32 transform_point(sint32 x, sint32 y, sint32 z, uint32 output[3]);
void transform_pitch(sint32 angle);
void transform_pitch_idx(uint32 idx);
void transform_yaw(sint32 angle);
void transform_yaw_idx(uint32 idx);
void transform_roll(sint32 angle);
void transform_roll_idx(uint32 idx);
void transform_scale(sint32 x, sint32 y, sint32 z);
void transform_reset(void);
sint32 transform_identity(void);
sint32 transform_origin_visible(void);
void transform_load(const POSE_MATRIX *src);
sint16 transform_origin_depth(void);

#endif
