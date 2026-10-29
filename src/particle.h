#ifndef PARTICLE_H
#define PARTICLE_H

#include "xport.h"

uint32 particle_draw_point(sint32 x, sint32 y, sint32 z, uint32 color);
uint32 particle_draw_line(sint32 x, sint32 y, sint32 z, uint32 color);
uint32 particle_draw_quad(sint32 x, sint32 y, sint32 z, uint32 color);

#endif
