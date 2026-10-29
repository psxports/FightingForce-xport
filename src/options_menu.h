#ifndef OPTIONS_MENU_H
#define OPTIONS_MENU_H

#include "xport.h"

typedef enum OPTIONS_MENU_ITEM
{
    OPTION_STAGE,
    OPTION_INVULNERABILITY,
    OPTION_DIFFICULTY,
    OPTION_MUSIC_VOLUME,
    OPTION_EFFECTS_VOLUME,
    OPTION_STEREO,
    OPTION_CONTROLLER,
    OPTION_LOAD_GAME,
    OPTION_LOAD_SETTINGS,
    OPTION_SAVE_SETTINGS,
    OPTION_SCREEN,
    OPTION_SCORES,
    OPTION_EXIT,
    OPTION_COUNT
} OPTIONS_MENU_ITEM;

enum
{
    OPTION_ROW_DISABLED = 999
};

typedef struct OPTIONS_MENU
{
    uint32 sel_row;
    uint32 row[OPTION_COUNT];
    uint32 row_count;
    uint32 phase;
} OPTIONS_MENU;

extern OPTIONS_MENU options_menu;
extern const OPTIONS_MENU options_menu_defaults;

sint32 options_menu_update(void);

uint32 options_draw_slider(sint32 x, sint32 y, sint32 val);

#endif
