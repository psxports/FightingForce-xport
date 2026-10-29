#include <stdio.h>
#include <string.h>

#include "game.h"

ACTION_HISTORY action_history;

uint8 *action_history_reset(void)
{
    FUNCTION_MARKER(0x800526F0, "GAME.EXE");
    memset(action_history.action, 0, sizeof(action_history.action));
    memset(action_history.predicted, 0, sizeof(action_history.predicted));
    /* Original reset leaves the previous match result unchanged */
    return action_history.predicted;
}

uint8 action_history_matched(sint32 target)
{
    FUNCTION_MARKER(0x80052744, "GAME.EXE");
    /* TODO: Investigate a non-player target reading beyond the match array */
    XASSERT((uint32)target < ACTION_HISTORY_PLAYER_COUNT);
    return action_history.matched[target];
}

uint8 action_history_rec(sint32 player, sint32 action)
{
    FUNCTION_MARKER(0x80052758, "GAME.EXE");
    uint8 *history;
    int end, start, len, byte;
    sint32 difference;
    uint8 val, left, right;
    XASSERT((uint32)player < ACTION_HISTORY_PLAYER_COUNT);
    history = action_history.action[player];
    for (byte = 0; byte < ACTION_HISTORY_LENGTH - 1; byte++)
        history[byte] = history[byte + 1];
    history[ACTION_HISTORY_LENGTH - 1] = action;
    /* Compare the full argument, not its truncated stored byte */
    action_history.matched[player] = action_history.predicted[player] == action;
    for (end = 12, len = 4; end < ACTION_HISTORY_LENGTH; end++, len--)
        for (start = end - 1; start >= 0; start--)
        {
            difference = 0;
            for (byte = 0; byte < len; byte++)
            {
                left = history[start + byte];
                right = history[end + byte];
                if (left != right)
                {
                    left = history[start + byte];
                    right = history[end + byte];
                    difference = left - right;
                    break;
                }
            }
            if (!difference)
            {
                val = history[start + len];
                action_history.predicted[player] = val;
                return val;
            }
        }
    return 0;
}
