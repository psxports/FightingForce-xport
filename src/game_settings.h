#ifndef GAME_SETTINGS_H
#define GAME_SETTINGS_H

#include "xport.h"

enum
{
    GAME_FLAG_INVULNERABILITY = 1,
    GAME_FLAG_EXTENDED_OPTIONS = 2,
    GAME_FLAG_SKIP_TARGET_REACTION = 4
};

typedef enum GAME_LANGUAGE
{
    GAME_LANGUAGE_ENGLISH,
    GAME_LANGUAGE_FRENCH,
    GAME_LANGUAGE_GERMAN,
    GAME_LANGUAGE_COUNT
} GAME_LANGUAGE;

typedef enum GAME_DIFFICULTY
{
    GAME_DIFFICULTY_EASY,
    GAME_DIFFICULTY_NORMAL,
    GAME_DIFFICULTY_HARD,
    GAME_DIFFICULTY_COUNT
} GAME_DIFFICULTY;

typedef struct GAME_SETTINGS
{
    uint32 flags;
    uint32 language;
    uint32 start_stage;

    struct
    {
        sint16 x, y;
    } scr_offset;

    union
    {
        sint8 difficulty;
        uint8 difficulty_bits;
    };
} GAME_SETTINGS;

extern GAME_SETTINGS game_settings;
extern const GAME_SETTINGS game_settings_defaults;

GDB_CALL uint32 game_settings_restore_card(void);
GDB_CALL uint32 game_settings_capture_card(void);

#endif
