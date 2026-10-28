#ifndef TEXT_H
#define TEXT_H

#include "xport.h"

enum
{
    TEXT_HINT_NONE = -1,
    TEXT_HINT_SELECT_CANCEL = 0,
    TEXT_HINT_CONTINUE = 1,
    TEXT_HINT_SELECT_EXIT = 2,
    TEXT_HINT_EXIT = 3,
    TEXT_HINT_LOAD_EXIT = 4,
    TEXT_HINT_FORMAT_EXIT = 5,
    TEXT_HINT_SAVE_EXIT = 6,
    TEXT_HINT_CHOOSE_SELECT = 7,
    TEXT_HINT_CHOOSE_SELECT_BACK = 8,
    TEXT_HINT_MOVE_CHOOSE_BACK = 9,
    TEXT_HINT_CHOOSE_SELECT_EXIT = 10,
    TEXT_HINT_CHOOSE_CHANGE_EXIT = 11,
    TEXT_HINT_CHANGE_EXIT = 12,
    TEXT_HINT_MOVE_CHANGE_EXIT = 13,
    TEXT_HINT_CHOOSE_SELECT_CANCEL = 14,
    TEXT_HINT_COUNT = 15
};

enum
{
    TEXT_GRADIENT_COLOR_COUNT = 4,
    TEXT_COLOR_CHANNEL_COUNT = 3
};

typedef union TEXT_COLOR
{
    struct
    {
        uint8 r, g, b;
    };

    uint8 v[TEXT_COLOR_CHANNEL_COUNT];
} TEXT_COLOR;

typedef struct TEXT_GRADIENT
{
    TEXT_COLOR color[TEXT_GRADIENT_COLOR_COUNT];
} TEXT_GRADIENT;

#define TEXT_GRADIENT_ACTIVE_INIT \
    { \
        .color = { {.v = {240, 240, 240}}, {.v = {0, 0, 0}}, {.v = {15, 15, 15}}, {.v = {0, 0, 0}} } \
    }

extern TEXT_GRADIENT text_active_gradient;
extern const TEXT_GRADIENT text_gradient_defaults;
extern const TEXT_GRADIENT text_inactive_gradient;
extern uint32 text_gradient_phase;

GDB_CALL sint32 text_gradient_update(void);
GDB_CALL sint32 text_center_gradient(const char *text, sint32 y);
GDB_CALL sint32 text_draw_gradient(const char *text, sint32 x, sint32 y, const TEXT_GRADIENT *colors);

GDB_CALL uint32 text_format_digits(sint32 val, char *dst);
GDB_CALL sint32 text_width(const char *text);
GDB_CALL sint32 text_draw(const char *text, sint32 x, sint32 y);
GDB_CALL sint32 text_center(const char *text, sint32 y);
GDB_CALL sint32 text_draw_intro(const char *text, sint32 x, sint32 y, sint32 phase);
GDB_CALL sint32 text_draw_large_intro(const char *text, sint32 x, sint32 y, sint32 phase);
GDB_CALL uint32 text_draw_logo(sint32 x, sint32 y);
GDB_CALL sint32 text_draw_right(const char *text, sint32 x, sint32 y);
GDB_CALL uint32 text_draw_digits(const char *text, uint32 capacity, sint32 x, sint32 y);

enum
{
    TEXT_LINE_COUNT = 10
};

typedef struct TEXT_LAYOUT
{
    char *line[TEXT_LINE_COUNT];
    uint32 line_count;
} TEXT_LAYOUT;

extern TEXT_LAYOUT text_layout;

GDB_CALL sint32 text_glyph(sint32 character);
GDB_CALL sint32 text_wrap(const char *text, char *buffer, uint32 capacity);
GDB_CALL sint32 text_draw_lines(sint32 y);

extern sint32 text_hint;

#endif
