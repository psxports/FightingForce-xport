#ifndef GAME_SESSION_H
#define GAME_SESSION_H

#include "xport.h"

typedef struct GAME_SESSION
{
    uint32 stage;
    uint32 mode;
    uint32 scene_phase;
    /* Shared menu/scene/level tick resets at the original transition phases */
    uint32 tick;
    uint32 player_count;
    /* Stage0 updates accumulate independently of the resettable game tick */
    uint32 stage0_ticks;
    /* Victim slot published by npc_attack_players after a lethal hit */
    uint32 defeated_player;
} GAME_SESSION;

extern GAME_SESSION game_session;

#endif
