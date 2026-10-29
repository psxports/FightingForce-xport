#ifndef BACKGROUND_H
#define BACKGROUND_H

#include "xport.h"

typedef struct BACKGROUND
{
    uint32 first_color, last_color;
    /* Wrapped phase; its signed high halfword controls sprite scrolling */
    uint32 wave_phase;
} BACKGROUND;

extern BACKGROUND background;

void background_draw(void);
uint32 background_draw_ground(void);
uint32 background_draw_gradient_sky(void);
uint32 background_draw_black_to_horizon(void);
uint32 background_draw_lower_black(void);
uint32 background_draw_tall_sky(void);
uint32 background_draw_split_sky(void);
uint32 background_draw_two_row_sky(void);
uint32 background_draw_black_to_horizon_alt(void);
sint32 background_draw_lightning(void);
uint32 background_draw_wave_sky(void);
uint32 background_draw_black(void);
uint32 background_draw_lift_sky(void);
uint32 background_draw_high_sky(void);
uint32 background_draw_fixed_gradient(void);
uint32 background_draw_parallax_sky(void);

#endif
