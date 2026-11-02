#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "game.h"
#include "hd.h"

static const sint32 effects_preview_voice = 19;

static const char invulnerability_on[] = "INVULNERABILITY ON";
static const char invulnerability_off[] = "INVULNERABILITY OFF";
static const char difficulty_format[] = "%s %s";
static const char numbered_label_format[] = "%s %d";
static const char action_label_format[] = "%c%s";
static const char back_fist_german[] = "HINTEN";
static const char back_fist_french[] = "RETOURNE";
static const char stereo_text[] = "STEREO";
static const char mono_text[] = "MONO";
static const sint8 pad_glyph[] = {93, 91, 125, 123, 40, 41};

OPTIONS_MENU options_menu;
const OPTIONS_MENU options_menu_defaults = {
    .row = {0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12},
    .row_count = OPTION_COUNT,
};

/* 80052624: draw before input; use existing controller count without refresh. */
sint32 scores_menu_update(void)
{
    FUNCTION_MARKER(0x80052624, "GAME.EXE");
    uint32 i = 0, raw, edge;
    sint32 result;
    high_scores_draw();
    result = game_session.player_count;
    if (result <= 0)
        return result;
    for (;;)
    {
        raw = pad_decode_buttons(pad_game.port[i]);
        edge = (raw ^ pad_menu.prev[i]) & raw;
        pad_menu.pressed[i] = edge;
        pad_menu.prev[i] = raw;
        edge = pad_menu.pressed[i];
        pad_menu.last_raw = raw;
        ++i;
        if (edge & 0x1000)
        {
            menu.callback = &menu_callback_options;
            return menu_play_confirm_sound();
        }
        result = (sint32)i < (sint32)game_session.player_count;
        if (!result)
            return result;
    }
}

/* 800519D4: display both player headings, then each connected player's scheme.
 * Preserve redundant decoder calls: they update disconnect/reset state. */
sint32 pad_menu_update(void)
{
    FUNCTION_MARKER(0x800519d4, "GAME.EXE");
    char buffer[256];
    int player, k, x, y;
    uint32 edge, available;
    pad_refresh_ports();
    available = pad_decode.count;
    game_session.player_count = available;
    if (!available)
        return menu_draw_primary_pad_notice(98);
    text_center(localization.text[LOCAL_TEXT_CONTROLLER_SETTINGS], 20);
    for (player = 0, x = 32; player < 2; player++, x += 150)
    {
        snprintf(buffer, 256, numbered_label_format, localization.text[LOCAL_TEXT_PLAYER], player + 1);
        text_draw(buffer, x, 60);
        pad_decode_buttons(player);
        if ((sint8)pad_decode.type != -1)
        {
            sint32 scheme = (sint8)pad_action.cfg[player];
            const char *label = localization.text[LOCAL_TEXT_SETTING];
            snprintf(buffer, 256, numbered_label_format, label, scheme + 1);
            text_draw(buffer, x - 10, 70);
            for (k = 0, y = 90; k < 6; k++, y += 16)
            {
                sint32 action, prefix;
                scheme = (sint8)pad_action.cfg[player];
                action = pad_bindings[scheme][k];
                prefix = pad_glyph[k];
                XASSERT(action >= 0 && action < LOCALIZED_ACTION_COUNT);
                label = localization.action[action];
                snprintf(buffer, 256, action_label_format, prefix, label);
                text_draw(buffer, x - 16, y);
                if (action == 3)
                {
                    uint32 language = game_settings.language;
                    if (language == 2)
                    {
                        y += 10;
                        text_draw(back_fist_german, x + 65, y);
                        language = game_settings.language;
                    }
                    if (language == 1)
                    {
                        y += 10;
                        text_draw(back_fist_french, x + 25, y);
                    }
                }
            }
        }
    }
    for (player = 0; player < 2; player++)
    {
        pad_decode_buttons(player);
        if ((sint8)pad_decode.type != -1)
        {
            uint8 *scheme = &pad_action.cfg[player];
            edge = pad_menu_poll(player);
            if (edge & 0x80)
            {
                menu_play_confirm_sound();
                --*scheme;
            }
            if (edge & 0x20)
            {
                menu_play_confirm_sound();
                ++*scheme;
            }
            *scheme &= 7;
            if (edge & 0x1000)
            {
                menu_play_confirm_sound();
                menu.callback = &menu_callback_options;
            }
        }
    }
    return text_center(localization.hint[TEXT_HINT_CHANGE_EXIT], 214);
}

/* 80051C98: adjustment edges from either pad, in left/right/up/down order.
 * The inclusive left guard permits -10 -> -11, as in actual MIPS. */
sint32 scr_menu_update(void)
{
    FUNCTION_MARKER(0x80051c98, "GAME.EXE");
    uint32 edge, players;
    sint32 offset;
    pad_refresh_ports();
    players = pad_decode.count;
    game_session.player_count = players;
    if (!players)
        return menu_draw_primary_pad_notice(98);
    text_center(localization.text[LOCAL_TEXT_SCREEN_ADJUST], 20);
    edge = pad_menu_poll(0);
    if (game_session.player_count == 2)
        edge |= pad_menu_poll(1);
    if (edge & 0x80)
    {
        offset = game_settings.scr_offset.x;
        if (offset >= -10)
            game_settings.scr_offset.x = offset - 1;
    }
    if (edge & 0x20)
    {
        offset = game_settings.scr_offset.x;
        if (offset < 16)
            game_settings.scr_offset.x = offset + 1;
    }
    if (edge & 0x10)
    {
        offset = game_settings.scr_offset.y;
        if (offset > 0)
            game_settings.scr_offset.y = offset - 1;
    }
    if (edge & 0x40)
    {
        offset = game_settings.scr_offset.y;
        if (offset < 16)
            game_settings.scr_offset.y = offset + 1;
    }
    disp_apply_offsets();
    if (edge & 0x1000)
    {
        menu_play_confirm_sound();
        menu.callback = &menu_callback_options;
    }
    return text_center(localization.hint[TEXT_HINT_MOVE_CHANGE_EXIT], 214);
}

static void home(void)
{
    menu.callback = &menu_callback_main;
    camera_capture(camera_menu_dst.bits, 0, -12032);
}

/* Original option text and both palettes remain caller-local */
sint32 options_menu_update(void)
{
    FUNCTION_MARKER(0x80050d98, "GAME.EXE");
    char buffer[256];
    TEXT_GRADIENT active = text_gradient_defaults;
    const TEXT_GRADIENT inactive = text_inactive_gradient;
    uint32 flags = game_settings.flags, phase, n, edge, raw, sel, val, a, b;
    sint32 count, i, k, y, width = 0, result;
    const uint32 phases[4] = {0, 512, 1024, 1536};
    const uint32 color_offsets[4] = {0, 3, 9, 6};
    if (flags & GAME_FLAG_EXTENDED_OPTIONS)
    {
        for (i = OPTION_INVULNERABILITY; i < OPTION_COUNT; i++)
            options_menu.row[i] = i;
        options_menu.row[OPTION_STAGE] = 0;
        n = OPTION_COUNT;
    }
    else
    {
        options_menu.row[OPTION_STAGE] = OPTION_ROW_DISABLED;
        options_menu.row[OPTION_INVULNERABILITY] = OPTION_ROW_DISABLED;
        for (i = OPTION_MUSIC_VOLUME; i < OPTION_COUNT; i++)
            options_menu.row[i] = i - 2;
        options_menu.row[OPTION_DIFFICULTY] = 0;
        n = OPTION_COUNT - 2;
    }
    options_menu.row_count = n;
    phase = options_menu.phase;
    for (i = 0; i < 4; i++)
    {
        uint32 sample = i ? (phase + phases[i]) & 2047 : phase;
        uint8 shade = (uint8)(sin_table[sample] >> 11);
        if (i == 3)
            options_menu.phase = (phase + 32) & 2047;
        for (k = 0; k < 3; k++)
            active.color[color_offsets[i] / 3].v[k] = shade;
    }
    pad_refresh_ports();
    val = pad_decode.count;
    game_session.player_count = val;
    if (!val)
        return menu_draw_primary_pad_notice(98);
    text_center(localization.text[LOCAL_TEXT_OPTIONS], 20);
    sel = options_menu.sel_row;
    text_center((sel == options_menu.row[OPTION_LOAD_GAME] || sel == options_menu.row[OPTION_LOAD_SETTINGS] || sel == options_menu.row[OPTION_SAVE_SETTINGS] || sel == options_menu.row[OPTION_CONTROLLER] || sel == options_menu.row[OPTION_SCREEN] || sel == options_menu.row[OPTION_SCORES] || sel == options_menu.row[OPTION_EXIT]) ? localization.hint[TEXT_HINT_CHOOSE_SELECT_EXIT] : localization.hint[TEXT_HINT_CHOOSE_CHANGE_EXIT], 214);
    count = options_menu.row_count;
    y = 128 - 6u * count;
    if (game_settings.language == GAME_LANGUAGE_GERMAN)
        y = y - 6u;
    for (i = 0; i < count; i++)
    {
        uint32 row = i;
        const char *native_text = NULL;
        int slider = 0, has_label = 1;
        if (row == options_menu.row[OPTION_STAGE])
        {
            XASSERT(game_settings.start_stage < LEVEL_NAME_COUNT);
            native_text = level_names[game_settings.start_stage];
        }
        else if (row == options_menu.row[OPTION_INVULNERABILITY])
            native_text = game_settings.flags & GAME_FLAG_INVULNERABILITY ? invulnerability_on : invulnerability_off;
        else if (row == options_menu.row[OPTION_DIFFICULTY])
        {
            sint32 difficulty = game_settings.difficulty;
            const char *label = localization.text[LOCAL_TEXT_DIFFICULTY];
            XASSERT(difficulty >= 0 && difficulty < GAME_DIFFICULTY_COUNT);
            const char *name = localization.difficulty[difficulty];
            snprintf(buffer, sizeof(buffer), difficulty_format, label, name);
            native_text = buffer;
        }
        else if (row == options_menu.row[OPTION_EFFECTS_VOLUME])
        {
            native_text = localization.text[LOCAL_TEXT_EFFECTS_VOLUME];
            width = text_width(native_text);
            a = game_settings.language;
            b = sound_settings.effects_volume;
            width = width + 64u;
            slider = 1;
            if (a == 2)
            {
                options_draw_slider(240, y + 12u, b);
                width = width - 64u;
            }
            else
                options_draw_slider((width >> 1) + 96u, y + 1u, b);
        }
        else if (row == options_menu.row[OPTION_MUSIC_VOLUME])
        {
            native_text = localization.text[LOCAL_TEXT_MUSIC_VOLUME];
            width = text_width(native_text) + 64u;
            slider = 1;
            options_draw_slider((width >> 1) + 96u, y + 2u, sound_settings.music_volume);
        }
        else if (row == options_menu.row[OPTION_STEREO])
            native_text = sound_settings.flags & SOUND_FLAG_STEREO ? stereo_text : mono_text;
        else
        {
            const LOCALIZED_TEXT_ID labels[7] = {LOCAL_TEXT_CONTROLLER_SETTINGS, LOCAL_TEXT_LOAD_CARD_DATA, LOCAL_TEXT_LOAD_SETTINGS, LOCAL_TEXT_SAVE_SETTINGS, LOCAL_TEXT_SCREEN_ADJUST, LOCAL_TEXT_HIGH_SCORES, LOCAL_TEXT_EXIT};
            has_label = 0;
            for (k = 0; k < 7; k++)
                if (row == options_menu.row[OPTION_CONTROLLER + k])
                {
                    native_text = localization.text[labels[k]];
                    has_label = 1;
                    break;
                }
        }
        if (!slider && has_label)
            width = text_width(native_text);
        text_draw_gradient(native_text, 160u - (width >> 1), y, options_menu.sel_row == row ? &active : &inactive);
        y = y + 12u;
        if (row == options_menu.row[OPTION_EFFECTS_VOLUME] && game_settings.language == GAME_LANGUAGE_GERMAN)
            y = y + 6u;
        count = options_menu.row_count;
    }
    count = game_session.player_count;
    result = count;
    for (i = 0; i < count; i++)
    {
        uint32 *pressed = &pad_menu.pressed[i];
        uint32 *prev = &pad_menu.prev[i];
        raw = pad_decode_buttons(pad_game.port[i]);
        edge = (raw ^ *prev) & raw;
        *pressed = edge;
        *prev = raw;
        edge = *pressed;
        pad_menu.last_raw = raw;
        if (edge & 0x1000)
        {
            home();
            return menu_play_confirm_sound();
        }
        if (edge & 0x10)
        {
            menu_play_move_sound();
            val = options_menu.sel_row - 1;
            options_menu.sel_row = val;
            if ((sint32)val < 0)
                options_menu.sel_row = options_menu.row_count - 1;
        }
        if (*pressed & 0x40)
        {
            menu_play_move_sound();
            val = options_menu.sel_row;
            n = options_menu.row_count;
            val++;
            options_menu.sel_row = val;
            if ((sint32)(n - 1) < (sint32)val)
                options_menu.sel_row = 0;
        }
        sel = options_menu.sel_row;
        if (sel == options_menu.row[OPTION_STAGE])
        {
            if (*pressed & 0x80)
            {
                val = game_settings.start_stage;
                if ((sint32)val > 0)
                    game_settings.start_stage = val - 1;
            }
            if (*pressed & 0x20)
            {
                val = game_settings.start_stage;
                if ((sint32)val < LEVEL_NAME_COUNT - 1)
                    game_settings.start_stage = val + 1;
            }
        }
        else if (sel == options_menu.row[OPTION_INVULNERABILITY])
        {
            if (*pressed & 0x80)
            {
                game_settings.flags &= ~GAME_FLAG_INVULNERABILITY;
                menu_play_confirm_sound();
            }
            if (*pressed & 0x20)
            {
                game_settings.flags |= GAME_FLAG_INVULNERABILITY;
                menu_play_confirm_sound();
            }
        }
        else if (sel == options_menu.row[OPTION_DIFFICULTY])
        {
            if (*pressed & 0x80)
            {
                val = game_settings.difficulty;
                if ((sint32)val > 0)
                {
                    game_settings.difficulty_bits = val - 1;
                    menu_play_confirm_sound();
                }
            }
            if (*pressed & 0x20)
            {
                val = game_settings.difficulty;
                if ((sint32)val < 2)
                {
                    game_settings.difficulty_bits = val + 1;
                    menu_play_confirm_sound();
                }
            }
        }
        else if (sel == options_menu.row[OPTION_MUSIC_VOLUME])
        {
            sint32 v;
            raw = *prev;
            v = sound_settings.music_volume;
            if (raw & 0x80)
                v -= 2;
            if (raw & 0x20)
                v += 2;
            if (v < 0)
                v = 0;
            if (v > 255)
                v = 255;
            cd_music_volume(v);
            sound_settings.music_volume = v;
        }
        else if (sel == options_menu.row[OPTION_EFFECTS_VOLUME])
        {
            sint32 v;
            raw = *prev;
            v = sound_settings.effects_volume;
            if (raw & 0x80)
                v -= 2;
            if (raw & 0x20)
                v += 2;
            if (v < 0)
                v = 0;
            if (v > 255)
                v = 255;
            SOUND_SETTINGS settings = sound_settings;
            a = effects_preview_voice;
            settings.effects_volume = v;
            sound_settings = settings;
            sound_voice_volume(a, 128);
        }
        else if (sel == options_menu.row[OPTION_STEREO])
        {
            if (*pressed & 0x80)
            {
                SOUND_SETTINGS settings = sound_settings;
                if (settings.flags & SOUND_FLAG_STEREO)
                {
                    settings.flags &= ~SOUND_FLAG_STEREO;
                    sound_settings = settings;
                    menu_play_confirm_sound();
                }
            }
            if (*pressed & 0x20)
            {
                SOUND_SETTINGS settings = sound_settings;
                if (!(settings.flags & SOUND_FLAG_STEREO))
                {
                    settings.flags |= SOUND_FLAG_STEREO;
                    sound_settings = settings;
                    menu_play_confirm_sound();
                }
            }
        }
        else if (sel == options_menu.row[OPTION_CONTROLLER])
        {
            if (*pressed & 0x4000)
            {
                menu_play_confirm_sound();
                menu.callback = &menu_callback_pad;
            }
        }
        else if (sel == options_menu.row[OPTION_SCREEN])
        {
            if (*pressed & 0x4000)
            {
                menu_play_confirm_sound();
                menu.callback = &menu_callback_scr;
            }
        }
        else if (sel == options_menu.row[OPTION_LOAD_GAME])
        {
            if (*pressed & 0x4000)
            {
                menu_play_confirm_sound();
                card_menu_load_game(i);
            }
        }
        else if (sel == options_menu.row[OPTION_LOAD_SETTINGS])
        {
            if (*pressed & 0x4000)
            {
                menu_play_confirm_sound();
                card_menu_load_settings(i);
            }
        }
        else
        {
            sel = options_menu.sel_row;
            if (sel == options_menu.row[OPTION_SAVE_SETTINGS])
            {
                if (*pressed & 0x4000)
                {
                    menu_play_confirm_sound();
                    card_menu_save_settings();
                }
            }
            else if (sel == options_menu.row[OPTION_SCORES])
            {
                if (*pressed & 0x4000)
                {
                    menu.callback = &menu_callback_scores;
                    menu_play_confirm_sound();
                }
            }
            else if (sel == options_menu.row[OPTION_EXIT])
            {
                if (*pressed & 0x4000)
                {
                    home();
                    menu_play_confirm_sound();
                }
            }
        }
        count = game_session.player_count;
        result = i + 1 < count;
    }
    return result;
}

uint32 options_draw_slider(sint32 x, sint32 y, sint32 val)
{
    FUNCTION_MARKER(0x80062ab4u, "GAME.EXE");
    sint32 width = val >> 2;
    uint32 a, ux = x, uy = y;
    int i;
    if (width > 0)
    {
        a = render_buffers.packet_addr;
        render_buffers.packet_addr = a + 36;
        POLY_G4 *poly = psx_addr(a, sizeof(*poly));
        setPolyG4(poly);
        for (i = 0; i < 4; i++)
        {
            poly->p[i].rgbc.r = i < 2 ? 0 : 255;
            poly->p[i].rgbc.g = i < 2 ? 0 : 255;
            poly->p[i].rgbc.b = i < 2 ? 255 : 0;
        }
        poly->p[0].xy.x = (uint16)(ux + 1);
        poly->p[0].xy.y = (uint16)(uy + 1);
        poly->p[1].xy.x = (uint16)(ux + 1);
        poly->p[1].xy.y = (uint16)(uy + 5);
        poly->p[2].xy.x = (uint16)(ux + width + 1);
        poly->p[2].xy.y = (uint16)(uy + 1);
        poly->p[3].xy.x = (uint16)(ux + width + 1);
        poly->p[3].xy.y = (uint16)(uy + 5);
        const HD_UI_VERTEX fill[4] = {{(sint16)(ux + 1), (sint16)(uy + 1), 0, 0, 0xff0000}, {(sint16)(ux + width + 1), (sint16)(uy + 1), 0, 0, 0x00ffff}, {(sint16)(ux + 1), (sint16)(uy + 5), 0, 0, 0xff0000}, {(sint16)(ux + width + 1), (sint16)(uy + 5), 0, 0, 0x00ffff}};
        hd_collect_ui(fill, 0, 0, 0, 2047, HD_UI_OVERLAY);
        render_ot_insert(render_buffers.ot_addr + 8188, a);
    }
    a = render_buffers.packet_addr;
    render_buffers.packet_addr = a + 24;
    POLY_F4 *poly = psx_addr(a, sizeof(*poly));
    setPolyF4(poly);
    poly->rgbc.r = 0;
    poly->rgbc.g = 0;
    poly->rgbc.b = 0;
    for (i = 0; i < 4; i++)
    {
        poly->p[i].x = (uint16)(ux + (i < 2 ? 66 : 0));
        poly->p[i].y = (uint16)(uy + (i & 1 ? 6 : 0));
    }
    const HD_UI_VERTEX border[4] = {{(sint16)ux, (sint16)uy, 0, 0, 0}, {(sint16)(ux + 66), (sint16)uy, 0, 0, 0}, {(sint16)ux, (sint16)(uy + 6), 0, 0, 0}, {(sint16)(ux + 66), (sint16)(uy + 6), 0, 0, 0}};
    hd_collect_ui(border, 0, 0, 0, 2047, HD_UI_OVERLAY);
    return render_ot_insert(render_buffers.ot_addr + 8188, a);
}
