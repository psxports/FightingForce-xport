#ifndef SCENE_VEHICLE_H
#define SCENE_VEHICLE_H

#include "xport.h"
#include "model_object.h"

GDB_CALL intptr scene_vehicle_update_taxi(MODEL_OBJECT *model);
GDB_CALL uint32 scene_vehicle_update_truck(MODEL_OBJECT *model);
GDB_CALL uint32 scene_vehicle_update_car(MODEL_OBJECT *model);

GDB_CALL uint32 scene_vehicle_start_departure(MODEL_OBJECT *model);
GDB_CALL sint32 scene_vehicle_update_departure(MODEL_OBJECT *model);

GDB_CALL uint32 scene_vehicle_update_traffic(MODEL_OBJECT *model);

GDB_CALL sint32 scene_vehicle_update_accelerating_truck(MODEL_OBJECT *model);

#endif
