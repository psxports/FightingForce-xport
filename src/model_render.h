#ifndef MODEL_RENDER_H
#define MODEL_RENDER_H

#include "resources.h"
#include "actor_pose.h"

GDB_CALL void model_transform_pose(MODEL_REFERENCE model, ACTOR_POSE *pose);

GDB_CALL uint32 model_draw_pose(MODEL_REFERENCE model, ACTOR_POSE *pose);

GDB_CALL uint32 model_draw_static(MODEL_REFERENCE model, uint32 unused, sint32 bias);

GDB_CALL uint32 model_draw_menu(MODEL_REFERENCE model, uint32 unused, sint32 bias);

GDB_CALL uint32 model_draw_pulsing_marker(void);

GDB_CALL uint32 model_draw_packets(uint32 count, const MODEL_FACE_RECORD *recs, const uint32 *xy, const sint16 *depths, sint32 bias, sint32 depth_carrier);

GDB_CALL uint32 model_draw_faded(MODEL_REFERENCE model, uint32 unused, sint32 bias);

#endif
