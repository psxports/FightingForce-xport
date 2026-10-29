#ifndef PLAYER_H
#define PLAYER_H

#include "xport.h"

enum
{
    PLAYER_COUNT = 2
};

typedef struct PLAYER_SCORE
{
    uint32 val;
    uint32 continue_threshold;
} PLAYER_SCORE;

extern PLAYER_SCORE player_scores[PLAYER_COUNT];
extern uint32 player_repeat_counts[PLAYER_COUNT];

uint32 player_update_dir(sint32 idx, sint32 state, uint32 *buttons, uint32 locked);
void player_move_input(sint32 idx, uint32 buttons);
sint32 player_step_exceeds_bounds(sint32 idx);
sint32 player_step_blocked(sint32 idx);
sint32 player_try_grapple(sint32 idx);
sint32 player_start_pair_motion(sint32 idx);

void player_play_footstep(sint32 type);

#endif
