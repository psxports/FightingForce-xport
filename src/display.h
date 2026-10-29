#ifndef DISPLAY_H
#define DISPLAY_H

#include "xport.h"
#include "psx.h"

enum
{
    DISPLAY_ENV_COUNT = 2
};

typedef union DISPLAY_COLOR
{
    struct
    {
        sint16 r, g, b;
    };

    sint16 channel[3];
} DISPLAY_COLOR;

typedef struct DISPLAY_BUFFERS
{
    DISPENV env[DISPLAY_ENV_COUNT];
    uint32 frame;
    uint32 shake_ticks;
    uint32 flash_ticks;
    DISPLAY_COLOR fade;
    DISPLAY_COLOR fade_origin;
    sint8 pulse_active;
    uint32 pulse_phase;
} DISPLAY_BUFFERS;

extern DISPLAY_BUFFERS disp_buffers;

void disp_initialize(void);
void disp_setup(void);
uint32 disp_apply_offsets(void);
uint32 disp_update_offsets(void);
uint32 disp_begin_frame(void);
sint32 disp_draw_scene(void);
sint32 disp_wait_vblank(void);

uint32 disp_set_fade(sint32 r, sint32 g, sint32 b);
uint32 disp_fade_from_origin(sint32 level);
uint32 disp_draw_fade(void);
uint32 disp_draw_flash(void);

sint32 disp_update_pulse(void);

uint32 disp_prepare_scene(void);

#endif
