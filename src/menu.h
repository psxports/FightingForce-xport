#ifndef MENU_H
#define MENU_H

#include "xport.h"
#include "text.h"

enum
{
    MENU_PLAYER_COUNT = 2,
    MAIN_MENU_MODE_COUNT = 2
};

typedef struct MENU_CALLBACK
{
    sint32 (*update)(void);
} MENU_CALLBACK;

typedef const MENU_CALLBACK *MENU_CALLBACK_REFERENCE;

typedef struct MENU_TEXT
{
    char *data;
    uint32 capacity;
} MENU_TEXT;

extern MENU_TEXT menu_text;

typedef struct MENU_LOAD
{
    sint8 ready, stage, player_count;
} MENU_LOAD;

typedef struct MENU
{
    uint32 sel_row;
    uint32 skip_scene;
    uint32 last_activity[MENU_PLAYER_COUNT];
    uint32 last_combo_break[MENU_PLAYER_COUNT];
    uint32 color_phase;
    sint32 ambient_voice[MENU_PLAYER_COUNT];
    MENU_CALLBACK_REFERENCE callback;
    MENU_CALLBACK_REFERENCE message_return;
    MENU_CALLBACK_REFERENCE question_accept;
    MENU_CALLBACK_REFERENCE question_cancel;
    uint32 message_phase;
    uint32 question_phase;
    uint8 question_choice;
    TEXT_GRADIENT main_gradient;
    TEXT_GRADIENT message_gradient;
    TEXT_GRADIENT question_gradient;
    uint32 main_row_count[MAIN_MENU_MODE_COUNT];
    MENU_LOAD loaded_game;
} MENU;

extern const MENU_CALLBACK menu_callback_main;
extern const MENU_CALLBACK menu_callback_character;
extern const MENU_CALLBACK menu_callback_options;
extern const MENU_CALLBACK menu_callback_pad;
extern const MENU_CALLBACK menu_callback_scr;
extern const MENU_CALLBACK menu_callback_scores;

extern MENU menu;
extern const MENU menu_defaults;

uint32 language_menu_draw(void);
uint32 language_menu_run(void);
sint32 main_menu_update(void);
sint32 pad_menu_update(void);
sint32 scr_menu_update(void);
sint32 scores_menu_update(void);
sint32 menu_message_update(void);
sint32 menu_question_update(void);
uint32 menu_message_open(const char *message, uint32 message_capacity, MENU_CALLBACK_REFERENCE callback);
uint32 menu_question_open(const char *message, uint32 message_capacity, uint32 kind, MENU_CALLBACK_REFERENCE accept, MENU_CALLBACK_REFERENCE cancel);
uint32 menu_message_why_not(void);
uint32 menu_message_cannot(void);

sint32 menu_draw_dialog(uint32 retained_s3);

sint32 menu_end_dialog(uint32 retained_s3);

uint32 menu_begin_dim_frame(void);

sint32 menu_play_effect(sint32 id, sint32 volume);

sint32 menu_play_move_sound(void);

sint32 menu_play_confirm_sound(void);

sint32 menu_draw_primary_pad_notice(sint32 y);

uint32 menu_present_dim(void);

uint32 menu_draw_title_backdrop(void);

uint32 title_menu_run(void);

sint32 menu_finish_selection(sint32 skip_scene);

#endif
