#ifndef GAME_PAUSE_H
#define GAME_PAUSE_H

#include "xport.h"

extern uint8 game_pause_flags;

GDB_CALL sint32 game_pause_run(void);

#endif
