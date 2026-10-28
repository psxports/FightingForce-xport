#ifndef LEVEL_TRANSITION_H
#define LEVEL_TRANSITION_H

#include "xport.h"

typedef struct LEVEL_TRANSITION
{
    uint32 finish_tick;
    sint8 exit_requested;
    uint32 restart_timer;
    uint32 route_exit_tick;
    sint8 route_choice;
    uint32 bonus_item_pending;
    uint32 continue_phase;
    uint32 continue_timer;
    sint8 continue_choice;
    uint32 continues;
} LEVEL_TRANSITION;

extern LEVEL_TRANSITION level_transition;
extern const char continue_count_format[];

GDB_CALL sint32 level_transition_at_exit(void);
GDB_CALL sint32 level_transition_continue_prompt(void);
GDB_CALL sint32 level_transition_update(void);

GDB_CALL uint32 level_draw_fade_step(sint32 level);

GDB_CALL void level_transition_noop(void);

GDB_CALL void level_prepare_frame_noop(void);

#endif
