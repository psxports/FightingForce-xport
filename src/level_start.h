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

GDB_CALL uint32 level_start_players(void);
GDB_CALL void level_reset(void);

GDB_CALL void level_start_playback(void);

GDB_CALL uint32 level_initialize(void);

#endif
