#ifndef DEPTH_TABLE_H
#define DEPTH_TABLE_H

#include "xport.h"

enum
{
    DEPTH_TABLE_COUNT = 8192
};

extern sint16 depth_table[DEPTH_TABLE_COUNT];

uint32 depth_table_build(void);

#endif
