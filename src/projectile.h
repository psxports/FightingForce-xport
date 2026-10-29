#ifndef PROJECTILE_H
#define PROJECTILE_H

#include "xport.h"

struct MODEL_OBJECT;
struct PROJECTILE_SOURCE;

sint32 projectile_query_model(uint32 world_x, uint32 world_z, const struct MODEL_OBJECT *model);
sint32 projectile_query_src(uint32 world_x, uint32 world_z, const struct PROJECTILE_SOURCE *projectile);
sint32 projectile_contact_model(sint32 idx, const struct MODEL_OBJECT *model, uint32 retained_word);
sint32 projectile_contact_vals(sint32 idx, uint32 kind, const uint32 pos[3], uint32 retained_word);
sint32 projectile_trace_model(struct MODEL_OBJECT *model, sint32 idx);
sint32 projectile_trace_npc(uint32 pos[3], uint16 angle, sint32 idx);
/* Caller-local weapon ray for original kinds 499 through 505 */
sint32 projectile_trace_weapon(uint32 pos[3], uint32 kind, uint16 angle, sint32 idx);

intptr projectile_update_accelerating(struct MODEL_OBJECT *model);
uint32 projectile_update_ballistic(struct MODEL_OBJECT *model, uint32 retained_s3);
void projectile_draw_glow(const struct MODEL_OBJECT *item);

#endif
