#ifndef HELICOPTER_H
#define HELICOPTER_H

#include "actor_collision.h"

GDB_CALL uint32 helicopter_update_intro(struct MODEL_OBJECT *model);
GDB_CALL void helicopter_draw_intro(const struct MODEL_OBJECT *item);
GDB_CALL void helicopter_draw(const struct MODEL_OBJECT *model);
GDB_CALL uint32 helicopter_update_scene(struct MODEL_OBJECT *model);
GDB_CALL void helicopter_draw_scene(const struct MODEL_OBJECT *model);

GDB_CALL uint32 helicopter_update(struct MODEL_OBJECT *item, const PROJECTILE_SOURCE *retained_projectile);

/* Retained numeric projectile values from the world/dynamic dispatchers */
uint32 helicopter_update_retained(struct MODEL_OBJECT *item, uint32 prev_car, uint32 retained_angle);

GDB_CALL uint32 helicopter_update_patrol(struct MODEL_OBJECT *model);
GDB_CALL void helicopter_draw_patrol(const struct MODEL_OBJECT *item);

#endif
