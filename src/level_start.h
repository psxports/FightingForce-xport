#ifndef LEVEL_START_H
#define LEVEL_START_H

#include "xport.h"

enum
{
    LEVEL_START_COUNT = 36
};

typedef struct LEVEL_START
{
    sint16 x;
    sint16 z;
    sint16 angle;
} LEVEL_START;

extern LEVEL_START level_starts[LEVEL_START_COUNT];

uint32 level_start_players(void);
void level_reset(void);

void level_start_playback(void);

uint32 level_initialize(void);

#endif
