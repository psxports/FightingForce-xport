#ifndef CARD_DIALOG_H
#define CARD_DIALOG_H

#include "xport.h"

enum
{
    CARD_TEXT_CAPACITY = 256
};

typedef struct CARD_LOAD_TEXT
{
    char prefix;
    char wrapped[CARD_TEXT_CAPACITY];
    char description[CARD_TEXT_CAPACITY + 1];
} CARD_LOAD_TEXT;

typedef struct CARD_SAVE_TEXT
{
    char prefix;
    char wrapped[CARD_TEXT_CAPACITY];
    char description[CARD_TEXT_CAPACITY];
} CARD_SAVE_TEXT;

typedef struct CARD_TEXT_WORKSPACES
{
    char settings[261];
    char status[258];
    char confirm[257];
    char notice[257];
    CARD_LOAD_TEXT load;
    CARD_SAVE_TEXT save;
    char list[258];
    char title[CARD_TEXT_CAPACITY];
} CARD_TEXT_WORKSPACES;

typedef enum CARD_PROMPT_ID
{
    CARD_PROMPT_DELETE,
    CARD_PROMPT_FULL,
    CARD_PROMPT_OVERWRITE,
    CARD_PROMPT_SAVE,
    CARD_PROMPT_SAVE_RETRY,
    CARD_PROMPT_LOAD_RETRY,
    CARD_PROMPT_COUNT
} CARD_PROMPT_ID;

typedef struct CARD_PROMPT
{
    const char *text;
    /* Original saved S3 is independent of the borrowed native string */
    uint32 scene_context;
} CARD_PROMPT;

extern CARD_PROMPT card_prompts[CARD_PROMPT_COUNT];

typedef struct CARD_DIALOG
{
    uint32 settings_only;
    uint32 scene_backdrop;
} CARD_DIALOG;

extern CARD_TEXT_WORKSPACES card_text;
extern CARD_DIALOG card_dialog;

sint32 card_menu_load_game(uint32 retained_s3);
sint32 card_menu_load_settings(uint32 retained_s3);
sint32 card_menu_save_settings(void);

sint32 card_format_description(char *out, uint32 capacity);

sint32 card_save_dialog(void);
uint32 card_settings_load_dialog(void);
sint32 card_confirm_dialog(CARD_PROMPT prompt, sint32 choice, sint32 monitor_card);
sint32 card_draw_pending_notice(uint32 retained_s3);

#endif
