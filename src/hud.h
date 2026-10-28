#ifndef HUD_H
#define HUD_H

#include "xport.h"
#include "player.h"

enum
{
    HUD_PLAYER_COUNT = PLAYER_COUNT,
    HUD_SCORE_TEXT_CAPACITY = 8
};

typedef struct HUD
{
    char score_text[HUD_PLAYER_COUNT][HUD_SCORE_TEXT_CAPACITY];
    uint32 tick;
} HUD;

extern HUD hud;

GDB_CALL sint32 hud_format_score(sint32 player);
GDB_CALL uint32 hud_draw(void);
GDB_CALL uint32 hud_draw_health_left(uint32 xy, uint32 idx);
GDB_CALL uint32 hud_draw_health_right(uint32 xy, uint32 idx);
GDB_CALL uint32 hud_continue_prompt(uint32 player);
GDB_CALL uint32 hud_update(void);

#endif
