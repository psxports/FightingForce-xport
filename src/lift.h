#ifndef LIFT_H
#define LIFT_H

#include "xport.h"

typedef struct LIFT
{
    uint32 pos;
    uint32 speed;
    uint32 cycle;
    uint32 stop_phase;
    uint32 travel;
    /* TODO: Determine the consumer of the original sample242 event mark */
    uint16 sound_mark;
} LIFT;

extern LIFT lift;

#endif
