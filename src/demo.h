#ifndef DEMO_H
#define DEMO_H

#include "resources.h"

enum
{
    DEMO_COUNT = 4
};

typedef struct DEMO_CONFIG
{
    uint32 mode;
    uint32 stage;
    uint32 player_type[2];
    uint32 file_idx;
} DEMO_CONFIG;

typedef struct DEMO_STATE
{
    uint32 next_idx;
    uint32 cur_idx;
    sint32 count;
} DEMO_STATE;

extern const DEMO_CONFIG demo_config[DEMO_COUNT];
extern DEMO_STATE demo;
extern RESOURCE_FILE demo_file;
extern const DEMO_STATE demo_defaults;

GDB_CALL void demo_select_next(void);

#endif
