#include <stdio.h>
#include <string.h>

#include "game.h"

HIGH_SCORE high_scores[HIGH_SCORE_COUNT];
HIGH_SCORE_WORKSPACE high_score_workspace;
const HIGH_SCORE high_scores_defaults[HIGH_SCORE_COUNT] = {
    {500000, {.text = "MAC"}, 0}, {400000, {.text = "NMT"}, 0}, {300000, {.text = "DAV"}, 0}, {200000, {.text = "ROB"}, 0}, {100000, {.text = "GAZ"}, 0}, {80000, {.text = "JER"}, 0}, {60000, {.text = "ADY"}, 0}, {40000, {.text = "BRI"}, 0}, {20000, {.text = "DAZ"}, 0}, {10000, {.text = "KEN"}, 0},
};
static const HIGH_SCORE_NAME default_name = {.text = "???"};

enum
{
    HIGH_SCORE_CHARACTER_COUNT = 40
};

static const uint8 characters[HIGH_SCORE_CHARACTER_COUNT] = {'#', '<', 'A', 'B', 'C', 'D', 'E', 'F', 'G', 'H', 'I', 'J', 'K', 'L', 'M', 'N', 'O', 'P', 'Q', 'R', 'S', 'T', 'U', 'V', 'W', 'X', 'Y', 'Z', '0', '1', '2', '3', '4', '5', '6', '7', '8', '9', '!', '?'};
static const char score_format[4] = "%u";
static const char score_mark[4] = "?";

GDB_CALL sint32 high_scores_insert(uint32 score)
{
    FUNCTION_MARKER(0x8006aa70, "GAME.EXE");
    uint32 idx = 0, i;
    while (idx < HIGH_SCORE_COUNT && high_scores[idx].score >= score)
        ++idx;
    if (idx == HIGH_SCORE_COUNT)
        return -1;
    for (i = HIGH_SCORE_COUNT - 1; i > idx; --i)
        high_scores[i] = high_scores[i - 1];
    HIGH_SCORE *entry = high_scores + idx;
    entry->score = score;
    entry->flags = game_settings.flags != 0;
    entry->name = default_name;
    return idx;
}

GDB_CALL sint32 high_scores_draw(void)
{
    FUNCTION_MARKER(0x8006a968, "GAME.EXE");
    char buffer[32];
    sint32 i; uint32 width;
    text_center(localization.text[LOCAL_TEXT_HIGH_SCORES], 32);
    for (i = 0; i < HIGH_SCORE_COUNT; i++)
    {
        const HIGH_SCORE *entry = high_scores + i;
#if defined(_DEBUG)
        const void *name_end = memchr(entry->name.text, 0, sizeof(entry->name.text));
        XASSERT(name_end);
#endif
        text_draw(entry->name.text, 70, 56 + 14 * i);
        snprintf(buffer, 32, score_format, entry->score);
        width = text_width(buffer);
        if (entry->flags & 1)
            strcat(buffer, score_mark);
        text_draw(buffer, 250 - width, 56 + 14 * i);
    }
    return text_center(localization.hint[TEXT_HINT_EXIT], 214);
}

static sint32 name_draw(uint32 rank, uint32 score, const char *name)
{
    FUNCTION_MARKER(0x8006a870u, "GAME.EXE");
    uint8 saved[129];
    sint32 width, result;
    const char *text = rank == 0 ? localization.text[LOCAL_TEXT_CONGRATULATIONS] : (rank == 1 ? localization.text[LOCAL_TEXT_CONGRATULATIONS1P] : localization.text[LOCAL_TEXT_CONGRATULATIONS2P]);
    memcpy(saved, high_score_workspace.text, 129);
    text_wrap(text, high_score_workspace.text + 1, sizeof(high_score_workspace.text) - 1);
    text_draw_lines(32);
    text_wrap(localization.text[LOCAL_TEXT_TOP_TEN], high_score_workspace.text + 1, sizeof(high_score_workspace.text) - 1);
    text_draw_lines(64);
    text_center(localization.text[LOCAL_TEXT_ENTER_NAME], 112);
    text_center(localization.hint[TEXT_HINT_CHOOSE_SELECT], 214);
    sprintf(high_score_workspace.text + 1, score_format, score);
    text_center(high_score_workspace.text + 1, 88);
    width = text_width(name);
    result = text_draw_gradient(name, 160u - (width >> 1), 144, &text_active_gradient);
    memcpy(high_score_workspace.text, saved, 129);
    return result;
}

static void name_edit(uint32 player, uint32 score, char *name)
{
    FUNCTION_MARKER(0x8006adf8u, "GAME.EXE");
    sint32 pos = 0, glyph = 2, fade;
    uint32 buttons, tick;
    disp_set_fade(-256, -256, -256);
    game_session.tick = 0;
    for (;;)
    {
        tick = game_session.tick;
        if ((sint32)tick < 33)
            disp_fade_from_origin(tick << 3);
        game_session.tick = game_session.tick + 1;
        name[pos] = characters[glyph];
        name[pos + 1] = 0;
        menu_begin_dim_frame();
        name_draw(player, score, name);
        buttons = pad_menu_poll(player == 2);
        if (buttons & 0x80)
        {
            --glyph;
            menu_play_move_sound();
        }
        if (buttons & 0x20)
        {
            ++glyph;
            menu_play_move_sound();
        }
        if (pos < 3)
        {
            if (glyph < 0)
                glyph += HIGH_SCORE_CHARACTER_COUNT;
            else if (glyph >= HIGH_SCORE_CHARACTER_COUNT)
                glyph -= HIGH_SCORE_CHARACTER_COUNT;
        }
        else
        {
            if (glyph < 0)
                glyph += 2;
            else if (glyph >= 2)
                glyph -= 2;
        }
        if (buttons & 0x4000)
        {
            if (!glyph)
            {
                if (pos > 0)
                {
                    --pos;
                    menu_play_confirm_sound();
                }
                else
                    sound_voice_play(5, 223, 7936);
            }
            else if (glyph == 1)
                break;
            else
            {
                ++pos;
                if (pos == 3)
                    glyph = 1;
                menu_play_confirm_sound();
            }
        }
        menu_present();
    }
    menu_play_confirm_sound();
    menu_present();
    for (fade = 256; fade > 0; fade -= 8)
    {
        game_session.tick = game_session.tick + 1;
        disp_fade_from_origin(fade);
        menu_begin_dim_frame();
        name_draw(player, score, name);
        menu_present();
    }
    if (pos)
        name[pos] = 0;
    else
        memcpy(name, default_name.text, HIGH_SCORE_NAME_BYTES);
}

GDB_CALL uint32 high_scores_run(uint32 score0, uint32 score1)
{
    FUNCTION_MARKER(0x8006ab80u, "GAME.EXE");
    uint8 saved[8];
    uint32 player = 0, tmp;
    sint32 idx;
    memcpy(saved, high_score_workspace.name.text, 8);
    DrawSync(0);
    resource_arenas.persistent = resource_arenas.text_end;
    sound_samples_load(197);
    resource_load(3, 0x116000, NULL, TEXTURE_ARCHIVE_CAPACITY, &tex_file, NULL);
    cd_music_start_track(24);
    tex_load_grps(0);
    tex_upload_pals();
    if (game_session.mode)
    {
        if ((sint32)score1 < (sint32)score0)
            player = 1;
        else
        {
            tmp = score0;
            score0 = score1;
            score1 = tmp;
            player = 2;
        }
        idx = high_scores_insert(score0);
        if (idx != -1)
        {
            name_edit(player, score0, high_score_workspace.name.text);
            high_scores[idx].name.bits = high_score_workspace.name.bits;
        }
        idx = high_scores_insert(score1);
        if (idx != -1)
        {
            name_edit(3 - player, score1, high_score_workspace.name.text);
            high_scores[idx].name.bits = high_score_workspace.name.bits;
        }
    }
    else
    {
        idx = high_scores_insert(score0);
        if (idx != -1)
        {
            name_edit(0, score0, high_score_workspace.name.text);
            high_scores[idx].name.bits = high_score_workspace.name.bits;
        }
    }
    disp_set_fade(-256, -256, -256);
    game_session.tick = 0;
    return high_scores_resume(saved);
}

GDB_CALL uint32 high_scores_resume(const uint8 *saved)
{
    uint32 buttons, tick, result;
    sint32 fade;
    for (;;)
    {
        tick = game_session.tick;
        if ((sint32)tick < 33)
            disp_fade_from_origin(tick << 3);
        game_session.tick = game_session.tick + 1;
        menu_begin_dim_frame();
        high_scores_draw();
        buttons = pad_menu_poll(0);
        if ((sint32)game_session.player_count >= 2)
            buttons |= pad_menu_poll(1);
        if (buttons & 0x1000)
            break;
        menu_present();
    }
    menu_play_confirm_sound();
    menu_present();
    for (fade = 256; fade > 0; fade -= 8)
    {
        game_session.tick = game_session.tick + 1;
        disp_fade_from_origin(fade);
        menu_begin_dim_frame();
        high_scores_draw();
        result = menu_present();
    }
    memcpy(high_score_workspace.name.text, saved, 8);
    return result;
}
