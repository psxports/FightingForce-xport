#ifndef CHARACTER_MENU_H
#define CHARACTER_MENU_H

#include "xport.h"

enum
{
    CHARACTER_SELECTION_PLAYER_COUNT = 2,
    CHARACTER_MENU_ACTOR_COUNT = 4
};

typedef struct CHARACTER_MENU
{
    uint32 sel[CHARACTER_SELECTION_PLAYER_COUNT];
    uint32 player_count;
    uint32 ready_mask;
    uint32 confirmed;
    uint32 fade_tick;
    uint32 ring_phase;
    uint32 pulse_phase;
} CHARACTER_MENU;

extern CHARACTER_MENU character_menu;
extern const CHARACTER_MENU character_menu_defaults;
extern const uint32 character_menu_types[CHARACTER_MENU_ACTOR_COUNT];

sint32 character_menu_update(void);
sint32 character_menu_draw(const uint32 *sel, sint32 players);

sint32 character_menu_advance_actors(void);

#endif
