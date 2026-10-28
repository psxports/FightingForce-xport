#ifndef RENDER_STYLE_H
#define RENDER_STYLE_H

#include "xport.h"
#include "psx.h"

/* Original instruction word at 80011590 also supplies this depth cutoff */
enum
{
    RENDER_FADE_CUTOFF = 0x1400
};

typedef struct RENDER_STYLE
{
    /* Color includes the packed GPU command byte */
    RGBC color;
    uint32 blend_mode;
} RENDER_STYLE;

extern RENDER_STYLE render_style;

#endif
