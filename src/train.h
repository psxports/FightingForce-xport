#ifndef TRAIN_H
#define TRAIN_H

#include "xport.h"
#include "model_object.h"

typedef enum TRAIN_SIDE
{
    TRAIN_SIDE_LOW_X,
    TRAIN_SIDE_HIGH_X,
    TRAIN_SIDE_COUNT
} TRAIN_SIDE;

extern sint16 train_accel[TRAIN_SIDE_COUNT];

uint32 train_update(MODEL_OBJECT *model);

#endif
