#ifndef LOADING_H
#define LOADING_H

#include "xport.h"

typedef struct LOADING
{
    sint16 bar_x;
    sint16 total;
    uint32 count;
    uint32 active;
} LOADING;

extern const LOADING loading_defaults;
extern LOADING loading;

GDB_CALL uint32 loading_clear(void);
GDB_CALL uint32 loading_begin(uint32 steps);
GDB_CALL uint32 loading_draw(void);

#endif
