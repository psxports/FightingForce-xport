#include <string.h>

#include "game.h"

sint32 text_hint = TEXT_HINT_NONE;
TEXT_LAYOUT text_layout;
TEXT_GRADIENT text_active_gradient;
const TEXT_GRADIENT text_gradient_defaults = TEXT_GRADIENT_ACTIVE_INIT;
const TEXT_GRADIENT text_inactive_gradient = {
    .color = {{.v = {248, 248, 248}}, {.v = {248, 248, 248}}, {.v = {248, 248, 248}}, {.v = {248, 248, 248}}},
};
uint32 text_gradient_phase;

enum
{
    TEXT_FONT_WIDTH_COUNT = 60,
    TEXT_LARGE_WIDTH_COUNT = 2,
    TEXT_DIGIT_WIDTH_COUNT = 100
};

static const sint8 font_width[TEXT_FONT_WIDTH_COUNT] = {
    11, 10, 12, 11, 11, 9, 12, 10, 4, 10, 11, 11, 12, 9, 10, 10, 10, 10, 10, 9, 11, 11, 12, 12, 10, 10, 12, 12, 12, 12, 10, 5, 15, 15, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 11, 10, 11, 10, 4, 4, 6, 10, 10, 10, 10, 0,
};
static const sint16 large_width[TEXT_LARGE_WIDTH_COUNT] = {
    144,
    112,
};
static const sint8 digit_width[TEXT_DIGIT_WIDTH_COUNT] = {
    11, 10, 9, 8, 11, 9, 9, 10, 10, 9, 5, 4, 3, 3, 6, 4, 4, 6, 4, 4, 10, 9, 9, 8, 11, 10, 10, 10, 9, 9, 10, 8, 8, 7, 10, 9, 8, 10, 9, 8, 9, 8, 8, 7, 10, 9, 8, 10, 8, 8, 11, 10, 10, 9, 12, 10, 10, 12, 10, 10, 12, 11, 11, 10, 13, 11, 11, 13, 11, 11, 8, 7, 7, 7, 9, 8, 6, 10, 7, 7, 11, 9, 9, 8, 11, 10, 9, 11, 10, 9, 12, 10, 9, 9, 12, 10, 10, 11, 11, 10,
};

GDB_CALL sint32 text_wrap(const char *text, char *buffer, uint32 capacity)
{
    FUNCTION_MARKER(0x80061800, "GAME.EXE");
    char *p = buffer, *word = buffer;
    uint32 line_width = 0, word_width = 0, i;
    uint8 ch;
    memset(text_layout.line, 0, sizeof(text_layout.line));
    if (text && buffer)
    {
        i = 0;
        do
        {
            ch = (uint8)text[i];
            XASSERT(i < capacity);
            buffer[i++] = ch;
        } while (ch);
    }
    text_layout.line_count = 0;
    text_layout.line[0] = buffer;
    for (;;)
    {
        XASSERT(p < buffer + capacity);
        sint32 letter = (sint8)*p;
        if (letter == 32 || !letter)
        {
            if ((sint32)(line_width + word_width) >= 273)
            {
                line_width = word_width;
                word_width = 0;
                i = ++text_layout.line_count;
                XASSERT(i <= TEXT_LINE_COUNT);
                if (text_layout.line[i - 1] != word)
                {
                    XASSERT(i != TEXT_LINE_COUNT);
                    text_layout.line[i] = word;
                }
                word[-1] = 0;
                if (!(sint8)*p)
                    break;
                /* The overflowing separator follows the original glyph path */
            }
            else
            {
                line_width += 10 + word_width;
                word_width = 0;
                if (!(sint8)*p)
                    break;
                word = ++p;
                continue;
            }
        }
        letter = (sint8)*p++;
        sint32 glyph = text_glyph(letter);
        XASSERT(glyph >= 0 && glyph < TEXT_FONT_WIDTH_COUNT);
        word_width += font_width[glyph];
    }
    text_layout.line_count = 0;
    for (;;)
    {
        i = text_layout.line_count;
        if (!text_layout.line[i])
            return i + 1;
        text_layout.line_count = ++i;
        if (i >= TEXT_LINE_COUNT)
            return 0;
    }
}

GDB_CALL sint32 text_draw_intro(const char *text, sint32 x, sint32 y, sint32 phase)
{
    FUNCTION_MARKER(0x8006159cu, "GAME.EXE");
    PROJECTION_POINT pos;
    sint32 letter;
    sint32 spr;
    sint32 offset;
    uint32 step;
    sint32 width;
    /* TODO: Investigate missing native intro text at the original first read */
    XASSERT(text);
    uint8 character = (uint8)*text;
    pos.y = y;
    pos.depth = 128;
    if (!character)
        return 0;
    do
    {
        letter = (sint8)character;
        text++;
        if (letter == 32)
            x = x + 6u;
        else
        {
            letter -= 65;
            spr = letter + tex_map[(uint32)tex_grps[TEXTURE_GROUP_GAME_FONT].first_tex];
            if (phase < 0)
                offset = 400;
            else
            {
                step = phase > 16 ? 16 : phase;
                offset = (32767 - sin_table[step << 5]) >> 5;
            }
            pos.x = x + (uint32)offset;
            pos.size = 3 * offset + 64;
            if (offset != 400)
                spr_draw_scaled(&pos, spr);
            XASSERT(letter >= 0 && letter < TEXT_FONT_WIDTH_COUNT);
            width = font_width[letter];
            x = x + (uint32)(width + (width >> 3));
        }
        character = (uint8)*text;
        letter = (sint8)*text;
        phase = phase - 3u;
    } while (letter);
    return 0;
}

GDB_CALL sint32 text_draw_large_intro(const char *text, sint32 x, sint32 y, sint32 phase)
{
    FUNCTION_MARKER(0x800616dcu, "GAME.EXE");
    PROJECTION_POINT pos;
    sint32 letter;
    sint32 spr;
    sint32 offset;
    uint32 step;
    /* TODO: Investigate missing native intro text at the original first read */
    XASSERT(text);
    uint8 character = (uint8)*text;
    pos.y = y;
    pos.depth = 128;
    if (!character)
        return 0;
    do
    {
        letter = (sint8)character - 65;
        text++;
        spr = letter + tex_map[(60u + (tex_grps[TEXTURE_GROUP_GAME_FONT].first_tex))];
        if (phase < 0)
            offset = 400;
        else
        {
            step = phase > 16 ? 16 : phase;
            offset = (32767 - sin_table[step << 5]) >> 5;
        }
        pos.x = x + (uint32)offset;
        pos.size = 3 * offset + 64;
        if (offset != 400)
            spr_draw_scaled(&pos, spr);
        XASSERT(letter >= 0 && letter < TEXT_LARGE_WIDTH_COUNT);
        x = (uint32)x + large_width[letter];
        character = (uint8)*text;
        letter = (sint8)*text;
        phase = phase - 4u;
    } while (letter);
    return 0;
}

GDB_CALL sint32 text_draw_lines(sint32 y)
{
    FUNCTION_MARKER(0x800619e8, "GAME.EXE");
    sint32 count = text_layout.line_count;
    uint32 i = 0;
    if (count <= 0)
        return count;
    if (y == -1)
    {
        do
        {
            XASSERT(i < TEXT_LINE_COUNT);
            text_center(text_layout.line[i], 16 * i + 120 - 8u * count);
            ++i;
            count = text_layout.line_count;
        } while ((sint32)i < count);
        return 4 * i;
    }
    do
    {
        XASSERT(i < TEXT_LINE_COUNT);
        text_center(text_layout.line[i], y + 10 * i);
        ++i;
    } while ((sint32)i < (sint32)text_layout.line_count);
    return 0;
}

GDB_CALL sint32 text_draw_right(const char *text, sint32 x, sint32 y)
{
    FUNCTION_MARKER(0x80061cb8u, "GAME.EXE");
    uint32 width = text_width(text);
    return text_draw(text, x + 10u - width, y);
}

/* Original stack-local XYZ crosses the host ABI as a native pointer,
 * just like the existing 12594 sprite path; no emulated RAM scratch alias. */

/* 80062AB4: all colors precede coordinates; allocation padding is preserved */

#include <stdlib.h>

/* Original GAME.EXE:80018670..8001886C. IDA draft audited against all MIPS branches.
 * The unsigned lowercase-range comparison precedes signed special-character cases. */
GDB_CALL sint32 text_glyph(sint32 character)
{
    FUNCTION_MARKER(0x80018670, "GAME.EXE");
    if (character - 97u < 26u)
        character = character - 32u;
    switch (character)
    {
        case 91:
            return 26;
        case 93:
            return 27;
        case 123:
            return 28;
        case 125:
            return 29;
        case 63:
            return 30;
        case 46:
            return 31;
        case 40:
            return 32;
        case 41:
            return 33;
        case -93:
            return 34;
        case 36:
            return 35;
        case 37:
            return 36;
        case 94:
            return 37;
        case -60:
            return 48;
        case -42:
            return 49;
        case -36:
            return 50;
        case 45:
            return 51;
        case 44:
            return 52;
        case 39:
            return 53;
        case 33:
            return 54;
        case 95:
            return 55;
        case 60:
            return 56;
        case 35:
            return 57;
        case 61:
            return 58;
        default:
            return character - (character >= 65 ? 65u : 10u);
    }
}

/* Signed glyph widths and text bytes retain the original ten-pixel margin */
GDB_CALL sint32 text_width(const char *text)
{
    FUNCTION_MARKER(0x8005efb8, "GAME.EXE");
    uint32 width = 0;
    /* TODO: Investigate missing native text at the original first read */
    XASSERT(text);
    uint8 character = (uint8)*text;
    if (character)
    {
        do
        {
            sint32 ch = (sint8)character;
            text++;
            if (ch == 32)
                width += 6;
            else
            {
                sint32 glyph = text_glyph(ch);
                XASSERT(glyph >= 0 && glyph < TEXT_FONT_WIDTH_COUNT);
                width += font_width[glyph];
            }
            character = (uint8)*text;
        } while (character);
    }
    return width + 10;
}

/* Original GAME.EXE:80061B7C..80061C68. Signed glyph widths and font table LH. */
GDB_CALL sint32 text_draw(const char *text, sint32 x, sint32 y)
{
    FUNCTION_MARKER(0x80061b7c, "GAME.EXE");
    PROJECTION_POINT pos = {.v = {x, y, 0, 0}};
    /* TODO: Investigate missing native text at the original first read */
    XASSERT(text);
    uint8 character = (uint8)*text;
    if (!character)
        return pos.x + 10u;
    do
    {
        sint32 ch = (sint8)character;
        text++;
        if (ch == 32)
            pos.x = pos.x + 6u;
        else
        {
            sint32 glyph = text_glyph(ch);
            uint32 font = tex_grps[TEXTURE_GROUP_GAME_FONT].first_tex;
            spr_draw(&pos, (uint32)glyph + tex_map[font]);
            XASSERT(glyph >= 0 && glyph < TEXT_FONT_WIDTH_COUNT);
            pos.x = (uint32)pos.x + font_width[glyph];
        }
        character = (uint8)*text;
    } while (character);
    return pos.x + 10u;
}

/* Original GAME.EXE:80061ADC..80061B28. SRA width/2, not C signed division. */
GDB_CALL sint32 text_center(const char *text, sint32 y)
{
    FUNCTION_MARKER(0x80061adc, "GAME.EXE");
    sint32 width = text_width(text);
    return text_draw(text, 160u - (width >> 1), y);
}

/* GAME.EXE:80061C68. Logo font bank, fixed depth 16. */
GDB_CALL uint32 text_draw_logo(sint32 x, sint32 y)
{
    FUNCTION_MARKER(0x80061c68, "GAME.EXE");
    PROJECTION_POINT pos = {.v = {x, y, 16, 0}};
    return spr_draw(&pos, tex_map[(uint32)tex_grps[TEXTURE_GROUP_LOGO].first_tex]);
}

/* GAME.EXE:80012A00. Descriptor end coordinates are inclusive sprite limits;
 * the polygon path deliberately does not add the sprite path's +1. */

/* GAME.EXE:80011E40. Preserve the original full pointer store to the OT;
 * GPU traversal masks to 24 bits. A host pointer must never enter this chain. */

/* Signed byte deltas and late color reloads retain original interpolation */
GDB_CALL sint32 text_draw_gradient(const char *text, sint32 x, sint32 y, const TEXT_GRADIENT *colors)
{
    FUNCTION_MARKER(0x80062c0c, "GAME.EXE");
    sint32 width = text_width(text);
    uint32 advance = 0;
    sint8 delta[6];
    uint8 prev[6], character;
    int i;
    if (!width)
        return 0;
    for (i = 0; i < 6; i++)
    {
        int row = i / 3 * 2, channel = i % 3;
        uint8 right = colors->color[row + 1].v[channel];
        prev[i] = colors->color[row].v[channel];
        delta[i] = right - prev[i];
    }
    character = *text;
    if (!character)
        return x + 170u;
    do
    {
        sint32 ch = (sint8)character;
        text++;
        if (ch == 32)
            advance += 6;
        else
        {
            uint8 next[6];
            sint32 glyph = text_glyph(ch);
            uint32 packet = render_buffers.packet_addr;
            POLY_GT4 *poly = psx_addr(packet, sizeof(*poly));
            render_buffers.packet_addr = packet + 52;
            setPolyGT4(poly);
            for (i = 0; i < 6; i++)
            {
                sint32 product = (delta[i]) * advance;
                int row = i / 3 * 2, channel = i % 3;
                /* Original BREAK 6 on signed division overflow */
                XASSERT(width != -1 || (uint32)product != 0x80000000u);
                uint8 base = colors->color[row].v[channel];
                next[i] = base + (uint32)(product / width);
            }
            setRGB0(poly, (uint8)(prev[0] * 8u - 128u), (uint8)(prev[1] * 8u - 128u), (uint8)(prev[2] * 8u - 128u));
            setRGB1(poly, (uint8)(next[0] * 8u - 128u), (uint8)(next[1] * 8u - 128u), (uint8)(next[2] * 8u - 128u));
            setRGB2(poly, (uint8)(next[3] * 8u - 128u), (uint8)(next[4] * 8u - 128u), (uint8)(next[5] * 8u - 128u));
            setRGB3(poly, (uint8)(prev[3] * 8u - 128u), (uint8)(prev[4] * 8u - 128u), (uint8)(prev[5] * 8u - 128u));
            spr_set_gradient_quad(packet, (uint32)glyph + tex_map[(uint32)tex_grps[TEXTURE_GROUP_GAME_FONT].first_tex], x + advance, y);
            XASSERT(glyph >= 0 && glyph < TEXT_FONT_WIDTH_COUNT);
            advance += font_width[glyph];
            render_ot_insert(render_buffers.ot_addr + 8188, packet);
            for (i = 0; i < 6; i++)
                prev[i] = next[i];
        }
        character = *text;
    } while (character);
    return advance + x + 170u;
}

/* Score digits retain separate carried-character and continuation reads */
GDB_CALL uint32 text_draw_digits(const char *text, uint32 capacity, sint32 x, sint32 y)
{
    FUNCTION_MARKER(0x80061d0c, "GAME.EXE");
    PROJECTION_POINT pos = {.v = {x, y, 0, 0}};
    uint32 character, idx = 0;
    XASSERT(text && capacity);
    character = (uint8)text[idx];
    if (character)
        do
        {
            sint32 digit = (sint8)character;
            ++idx;
            if (digit == 32)
                pos.x = pos.x + 6u;
            else
            {
                digit -= 48;
                spr_draw(&pos, digit + tex_map[(38u + (tex_grps[TEXTURE_GROUP_GAME_FONT].first_tex))]);
                XASSERT(idx < capacity);
                uint32 pair = 10u * digit + ((uint8)text[idx] & 15);
                XASSERT(pair < TEXT_DIGIT_WIDTH_COUNT);
                pos.x = (uint32)pos.x + digit_width[pair];
            }
            XASSERT(idx < capacity);
            character = (uint8)text[idx];
        } while ((sint8)text[idx]);
    return character << 24;
}

/* Negative values use one character; retain the full32 return and unwritten bytes */
GDB_CALL uint32 text_format_digits(sint32 val, char *dst)
{
    FUNCTION_MARKER(0x80061e0c, "GAME.EXE");
    uint32 result = val + 48u;
    if (val < 10)
    {
        dst[0] = (char)result;
        dst[1] = 0;
    }
    else
    {
        dst[0] = (char)(val / 10 + 48);
        result = (val % 10) + 48;
        dst[1] = (char)result;
        dst[2] = 0;
    }
    return result;
}

/* 61B28: centered text, arithmetic width/2, fixed palette93BE4. */
GDB_CALL sint32 text_center_gradient(const char *text, sint32 y)
{
    FUNCTION_MARKER(0x80061b28u, "GAME.EXE");
    sint32 width = text_width(text);
    return text_draw_gradient(text, 160u - (width >> 1), y, &text_active_gradient);
}

GDB_CALL sint32 text_gradient_update(void)
{
    FUNCTION_MARKER(0x80067d3c, "GAME.EXE");
    uint32 phase = text_gradient_phase, i, j;
    const uint32 order[4] = {0, 1, 3, 2};
    sint32 val = 0;
    for (i = 0; i < 4; i++)
    {
        uint32 angle = i ? ((phase + 512u * i) & 2047u) : phase;
        val = sin_table[angle] >> 11;
        for (j = 0; j < 3; j++)
            text_active_gradient.color[order[i]].v[2 - j] = (uint8)val;
    }
    text_gradient_phase = (phase + 32u) & 2047u;
    return val;
}
