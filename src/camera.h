#ifndef CAMERA_H
#define CAMERA_H

#include "xport.h"
#include "geometry_math.h"

typedef union CAMERA_ANGLES
{
    struct
    {
        sint16 yaw, pitch;
    };

    struct
    {
        uint16 yaw_bits, pitch_bits;
    };

    sint16 v[2];
    uint16 bits[2];
} CAMERA_ANGLES;

typedef struct CAMERA_MOTION
{
    FIXED_VEC3 origin;
    FIXED_VEC3 delta;
    FIXED_VEC3 target;
    CAMERA_ANGLES origin_angles;
    CAMERA_ANGLES delta_angles;
    CAMERA_ANGLES target_angles;

    union
    {
        sint16 phase;
        uint16 phase_bits;
    };
} CAMERA_MOTION;

typedef struct CAMERA_TRACKING
{
    FIXED_XZ center;
    uint32 radius;

    union
    {
        sint32 pitch_control;
        uint32 pitch_control_bits;
    };

    uint32 visibility_mask;

    union
    {
        sint32 hold;
        uint32 hold_bits;
    };

    uint32 biker_idx;
} CAMERA_TRACKING;

typedef struct CAMERA
{
    FIXED_VEC3 pos;
    CAMERA_ANGLES angles;
    FIXED_VEC3 target;
    CAMERA_ANGLES target_angles;
    FIXED_VEC3 velocity;
    CAMERA_MOTION motion;
    CAMERA_TRACKING tracking;
    FIXED_VEC3 frame_pos;
    CAMERA_ANGLES frame_angles;

    /* Original camera update publishes (-x, y, -z) shifted right by 12 */
    struct
    {
        sint32 x, y, z;
    } scaled_pos;
} CAMERA;

enum
{
    CAMERA_STAGE_COUNT = 36
};

typedef enum CAMERA_HANDLER_ID
{
    CAMERA_HANDLER_STAGE0,
    CAMERA_HANDLER_STAGE1,
    CAMERA_HANDLER_STAGE2,
    CAMERA_HANDLER_STAGE3,
    CAMERA_HANDLER_STAGE4,
    CAMERA_HANDLER_STAGE5,
    CAMERA_HANDLER_STAGE6,
    CAMERA_HANDLER_STAGE7,
    CAMERA_HANDLER_STAGE8,
    CAMERA_HANDLER_STAGE9,
    CAMERA_HANDLER_STAGE10,
    CAMERA_HANDLER_STAGE11,
    CAMERA_HANDLER_STAGE12,
    CAMERA_HANDLER_ACTOR2,
    CAMERA_HANDLER_STAGE14,
    CAMERA_HANDLER_STAGE15,
    CAMERA_HANDLER_STAGE16,
    CAMERA_HANDLER_STAGE17,
    CAMERA_HANDLER_STAGE18,
    CAMERA_HANDLER_STAGE19,
    CAMERA_HANDLER_STAGE20,
    CAMERA_HANDLER_STAGE21,
    CAMERA_HANDLER_STAGE22,
    CAMERA_HANDLER_STAGE23,
    CAMERA_HANDLER_STAGE24,
    CAMERA_HANDLER_FRONTEND,
    CAMERA_HANDLER_COUNT
} CAMERA_HANDLER_ID;

typedef struct CAMERA_STAGE_HANDLER
{
    sint32 (*update)(void);
    CAMERA_HANDLER_ID id;
} CAMERA_STAGE_HANDLER;

typedef const CAMERA_STAGE_HANDLER *CAMERA_STAGE_REFERENCE;

extern const CAMERA_STAGE_HANDLER camera_stage_handlers[CAMERA_HANDLER_COUNT];
extern CAMERA_STAGE_REFERENCE camera_stage_refs[CAMERA_STAGE_COUNT];

GDB_CALL sint32 camera_update_stage(CAMERA_STAGE_REFERENCE ref);
GDB_CALL sint32 camera_update_stage0(void);

extern CAMERA camera;
extern const CAMERA camera_defaults;
extern const FIXED_VEC3 camera_menu_dst;
extern const FIXED_VEC3 camera_options_dst;

GDB_CALL sint32 camera_update(void);
GDB_CALL sint32 camera_update_target(void);

GDB_CALL sint32 camera_update_focus(void);
GDB_CALL sint32 camera_update_transition(void);
GDB_CALL sint32 camera_update_motion(uint32 x_speed);
GDB_CALL sint32 camera_capture(const uint32 *pos, sint32 yaw, sint32 pitch);
GDB_CALL sint32 camera_orbit(void);

#endif
