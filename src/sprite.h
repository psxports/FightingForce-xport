#ifndef SPRITE_H
#define SPRITE_H

#include "xport.h"
#include "projection.h"

GDB_CALL uint32 spr_draw(const PROJECTION_POINT *pos, sint32 spr_idx);
GDB_CALL uint32 spr_draw_quad(const PROJECTION_POINT *pos, uint32 tex);
GDB_CALL uint32 spr_draw_scaled(const PROJECTION_POINT *pos, sint32 spr);
GDB_CALL uint32 spr_draw_height(const PROJECTION_POINT *pos, sint32 spr);

GDB_CALL uint32 spr_draw_ribbon(const PROJECTION_POINT *a, const PROJECTION_POINT *b, sint32 spr);

GDB_CALL void spr_set_gradient_quad(uint32 packet, sint32 spr, sint32 x, sint32 y);

#endif
