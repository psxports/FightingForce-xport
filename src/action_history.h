#ifndef ACTION_HISTORY_H
#define ACTION_HISTORY_H

#include "xport.h"

enum
{
    ACTION_HISTORY_PLAYER_COUNT = 2,
    ACTION_HISTORY_LENGTH = 16
};

typedef struct ACTION_HISTORY
{
    uint8 action[ACTION_HISTORY_PLAYER_COUNT][ACTION_HISTORY_LENGTH];
    uint8 predicted[ACTION_HISTORY_PLAYER_COUNT];
    uint8 matched[ACTION_HISTORY_PLAYER_COUNT];
} ACTION_HISTORY;

extern ACTION_HISTORY action_history;

uint8 *action_history_reset(void);
uint8 action_history_rec(sint32 player, sint32 action);
uint8 action_history_matched(sint32 target);

#endif
