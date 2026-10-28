#ifndef SCENE_PLAYBACK_H
#define SCENE_PLAYBACK_H

#include "xport.h"

enum
{
    SCENE_STREAM_COUNT = 8
};

typedef struct SCENE_STREAM
{
    const uint8 *data;
    uint32 byte_len;
} SCENE_STREAM;

typedef struct SCENE_PLAYBACK
{
    uint32 camera_frames;
    uint32 end_tick;
    /* Distinguish Start from the other accepted scene-skip button */
    uint32 start_skip;
    SCENE_STREAM stream[SCENE_STREAM_COUNT];
} SCENE_PLAYBACK;

extern SCENE_PLAYBACK scene_playback;

/* Temporary numeric result ABI until loader and diagnostic contracts migrate */
GDB_CALL uint32 scene_load_resources0(void);
GDB_CALL uint32 scene_load_resources1(void);
GDB_CALL uint32 scene_load_resources2(void);
GDB_CALL uint32 scene_load_resources3(void);
GDB_CALL uint32 scene_load_resources4(void);
GDB_CALL uint32 scene_load_resources5(void);
GDB_CALL uint32 scene_load_resources6(void);
GDB_CALL uint32 scene_load_resources7(void);
GDB_CALL uint32 scene_load_resources8(void);

GDB_CALL sint32 scene_apply_camera_frame(sint32 frame);
GDB_CALL uint32 scene_apply_object_frame(sint32 frame, uint32 object_idx, uint32 stream);

GDB_CALL sint32 scene_update_prison_ending(uint32 frame);

GDB_CALL uint32 scene_update_hovercraft_fleet(sint32 frame);

GDB_CALL uint32 scene_update_wing(sint32 frame);

GDB_CALL uint32 scene_update_island_fleet(sint32 frame);

GDB_CALL sint32 scene_update_traffic_explosions(uint32 frame);

GDB_CALL uint32 scene_update_wing_destruction(uint32 frame);

GDB_CALL uint32 scene_update_tower_destruction(uint32 frame);

GDB_CALL uint32 scene_update_intro_helicopter(sint32 frame);

GDB_CALL sint32 sequence_run(sint32 sequence, void (*prepare_frame)(void));

GDB_CALL uint32 scene_step(void);

GDB_CALL sint32 scene_run_ending_disp(void);

#endif
