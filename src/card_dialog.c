#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "game.h"

static sint32 format_dialog(uint32 retained_s3);
static sint32 draw_notice(const char *text, uint32 retained_s3);

CARD_TEXT_WORKSPACES card_text;
CARD_DIALOG card_dialog;
CARD_PROMPT card_prompts[CARD_PROMPT_COUNT];

GDB_CALL sint32 card_format_description(char *out, uint32 capacity)
{
    FUNCTION_MARKER(0x8006ba58, "GAME.EXE");
    sint32 stage = card_storage.preview.stage;
    const char *format;
    const char *name0, *name1, *level;
    sint32 type0, type1, result;
    XASSERT(out && capacity);
    if (stage == -1 || stage == -2)
    {
        format = stage == -1 ? localization.text[LOCAL_TEXT_CARD_SEARCH] : localization.text[LOCAL_TEXT_CARD_MISSING];
        result = snprintf(out, capacity, format);
    }
    else if (card_storage.preview.player_count == 1)
    {
        type0 = card_storage.preview.character_type[0];
        format = localization.text[LOCAL_TEXT_CARD_SAVE_INFO1];
        XASSERT(stage >= 0 && stage < LEVEL_NAME_COUNT);
        level = level_names[stage];
        XASSERT(type0 >= 0 && type0 < CHARACTER_NAME_TYPE_COUNT);
        name0 = character_name_text(character_name_types[type0]);
        result = snprintf(out, capacity, format, name0, level);
    }
    else
    {
        XASSERT(stage >= 0 && stage < LEVEL_NAME_COUNT);
        level = level_names[stage];
        type0 = card_storage.preview.character_type[0];
        format = localization.text[LOCAL_TEXT_CARD_SAVE_INFO2];
        type1 = card_storage.preview.character_type[1];
        XASSERT(type0 >= 0 && type0 < CHARACTER_NAME_TYPE_COUNT && type1 >= 0 && type1 < CHARACTER_NAME_TYPE_COUNT);
        name0 = character_name_text(character_name_types[type0]);
        name1 = character_name_text(character_name_types[type1]);
        /* The original level string is the fifth argument at SP+10 */
        result = snprintf(out, capacity, format, name0, name1, level);
    }
    XASSERT(result >= 0 && (uint32)result < capacity);
    return result;
}

GDB_CALL uint32 card_settings_load_dialog(void)
{
    FUNCTION_MARKER(0x80069554u, "GAME.EXE");
    uint8 saved[260];
    uint32 buttons, result;
    const char *text;
    memcpy(saved, card_text.settings + 1, sizeof(saved));
    DrawSync(0);
    sound_samples_load(197);
    resource_load(3, 0x116000, NULL, TEXTURE_ARCHIVE_CAPACITY, &tex_file, NULL);
    tex_load_grps(0);
    tex_upload_pals();
    disp_set_fade(-256, -256, -256);
    draw_notice(localization.text[LOCAL_TEXT_CARD_ACCESS], 2);
    for (;;)
    {
        menu_draw_dialog(2);
        buttons = pad_menu_poll(0);
        if (game_session.player_count == 2)
            buttons |= pad_menu_poll(1);
        card_update();
        card_wait();
        if (!card_poll_status(&card_storage.status))
        {
            text = localization.text[LOCAL_TEXT_CARD_SAVE_CAUTION];
            goto prompt;
        }
        if ((card_storage.status & 3) == 2)
        {
            text_wrap(localization.text[LOCAL_TEXT_CARD_INSERT_FORMATTED_OR_CONTINUE], card_text.settings + 1, CARD_TEXT_CAPACITY);
            text_draw_lines(-1);
            text_center(localization.hint[TEXT_HINT_CONTINUE], 214);
            if (buttons & 0x4000)
                break;
        }
        if ((card_storage.status & 3) != 1)
            goto present;
        if (card_read_save(0, card_save_name, CARD_SAVE_NAME_SIZE, card_files.save.bytes, CARD_FILE_SIZE) == 1)
        {
            game_settings_restore_card();
            break;
        }
        if ((card_storage.status & 3) != 1 || card_file_exists(0, card_save_name, CARD_SAVE_NAME_SIZE) == 1)
            goto present;
        card_list_files(0, &card_files.directory);
        card_update();
        card_wait();
        if (!card_poll_status(&card_storage.status))
            card_storage.free_blocks = 0;
        if (card_storage.free_blocks)
            break;
        text = localization.text[LOCAL_TEXT_CARD_FULL_MANAGER];
    prompt:
        text_wrap(text, card_text.settings + 1, CARD_TEXT_CAPACITY);
        text_draw_lines(-1);
        text_center(localization.hint[TEXT_HINT_CONTINUE], 214);
        if (buttons & 0x4000)
            break;
    present:
        menu_present();
    }
    result = loading_clear();
    memcpy(card_text.settings + 1, saved, sizeof(saved));
    return result;
}

static uint32 card_dialog_buttons(void)
{
    uint32 buttons = pad_menu_poll(0);
    if (game_session.player_count == 2)
        buttons |= pad_menu_poll(1);
    return buttons;
}

static sint32 status_dialog(uint32 *retained_status, uint32 retained_s3)
{
    FUNCTION_MARKER(0x80068180u, "GAME.EXE");
    uint8 saved[257];
    uint32 buttons, prev_status, cur_status;
    draw_notice(localization.text[LOCAL_TEXT_CARD_ACCESS], retained_s3);
    /* Stop before consuming the original unwritten status local */
    {
        sint32 checked_result = card_wait_status(retained_status);
        XASSERT(checked_result);
    }
    prev_status = *retained_status;
    cur_status = card_storage.status;
    if (prev_status == cur_status)
        return 0;
    memcpy(saved, card_text.status + 1, 257);
    text_hint = TEXT_HINT_CONTINUE;
    do
    {
        menu_draw_dialog(retained_s3);
        text_wrap(localization.text[LOCAL_TEXT_CARD_REMOVED], card_text.status + 1, CARD_TEXT_CAPACITY);
        text_draw_lines(-1);
        menu_present();
        buttons = card_dialog_buttons();
    } while (!(buttons & 0xf000));
    menu_play_confirm_sound();
    menu_end_dialog(retained_s3);
    memcpy(card_text.status + 1, saved, 257);
    return 1;
}

/* The retained status cell is independent of caller S3 */
static sint32 card_status_changed_with_context(uint32 retained_s3)
{
    return status_dialog(&card_storage.prev_status, retained_s3);
}

static sint32 load_dialog(uint32 retained_s3)
{
    FUNCTION_MARKER(0x800690d8u, "GAME.EXE");
    CARD_LOAD_TEXT saved = card_text.load;
    uint8 ch;
    uint32 buttons, i = 0, status;
    const char *text;
    sint32 connected, result;
    text = card_dialog.settings_only ? localization.text[LOCAL_TEXT_LOAD_SETTINGS] : localization.text[LOCAL_TEXT_CARD_LOAD_TITLE];
    if (text)
    {
        do
        {
            ch = (uint8)text[i];
            XASSERT(i < CARD_TEXT_CAPACITY);
            card_text.title[i] = ch;
            ++i;
        } while (ch);
    }
    card_dialog.scene_backdrop = 1;
    DrawSync(0);
    menu_end_dialog(retained_s3);
    do
    {
        pause_refresh_pad();
        buttons = card_dialog_buttons();
    } while (buttons);
    draw_notice(localization.text[LOCAL_TEXT_CARD_ACCESS], retained_s3);
    card_storage.preview.stage = -1;
    retained_s3 = 3;
    card_update();
    card_wait();
    text_hint = TEXT_HINT_NONE;
    for (;;)
    {
    reload_card:
        menu_draw_dialog(retained_s3);
        pad_refresh_ports();
        card_update();
        status = card_poll_status(&card_storage.status);
        buttons = card_dialog_buttons();
        if (!status || card_storage.status == 2)
        {
            text = !status ? localization.text[LOCAL_TEXT_CARD_INSERT_SAVED] : localization.text[LOCAL_TEXT_CARD_INSERT_FORMATTED];
            text_wrap(text, card_text.load.wrapped, CARD_TEXT_CAPACITY);
            text_draw_lines(-1);
            text_hint = TEXT_HINT_EXIT;
        }
        else
        {
            card_format_description(card_text.load.description, CARD_TEXT_CAPACITY);
            text_wrap(card_text.load.description, card_text.load.wrapped, CARD_TEXT_CAPACITY);
            text_draw_lines(-1);
            connected = pad_decode.count;
            if (!connected)
                connected = 1;
            if ((card_storage.status & 3) == 1 && card_storage.preview.stage >= 0 && connected >= card_storage.preview.player_count)
                text_hint = TEXT_HINT_LOAD_EXIT;
            else
                text_hint = TEXT_HINT_EXIT;
            if ((buttons & 0x4000) && text_hint == TEXT_HINT_LOAD_EXIT)
            {
                card_wait();
                if (card_file_exists(0, card_save_name, CARD_SAVE_NAME_SIZE))
                {
                    menu_play_confirm_sound();
                    menu_end_dialog(retained_s3);
                    for (;;)
                    {
                        if (card_status_changed_with_context(retained_s3))
                            goto reload_card;
                        draw_notice(localization.text[LOCAL_TEXT_CARD_LOADING], retained_s3);
                        if (card_read_save(0, card_save_name, CARD_SAVE_NAME_SIZE, card_files.save.bytes, CARD_FILE_SIZE) == 1)
                            break;
                        if (card_status_changed_with_context(retained_s3) || card_confirm_dialog(card_prompts[CARD_PROMPT_LOAD_RETRY], 0, 1))
                            goto reload_card;
                    }
                    game_settings_restore_card();
                    if (!card_dialog.settings_only)
                    {
                        menu.loaded_game.stage = card_files.save.data.stage;
                        menu.loaded_game.player_count = card_files.save.data.player_count;
                        ch = card_files.save.data.character_type[0];
                        actor_types[0] = (uint16)(sint16)(sint8)ch;
                        ch = card_files.save.data.character_type[1];
                        actor_types[1] = (uint16)(sint16)(sint8)ch;
                        actor_health[0].hp = card_files.save.data.health[0];
                        actor_health[1].hp = card_files.save.data.health[1];
                        player_scores[0].val = card_files.save.data.score[0];
                        player_scores[1].val = card_files.save.data.score[1];
                        player_scores[0].continue_threshold = card_files.save.data.continue_threshold[0];
                        player_scores[1].continue_threshold = card_files.save.data.continue_threshold[1];
                        level_transition.continues = card_files.save.data.continues;
                        level_transition.route_choice = card_files.save.data.route_choice;
                        menu.loaded_game.ready = 1;
                    }
                    text_hint = TEXT_HINT_CONTINUE;
                    do
                    {
                        menu_draw_dialog(retained_s3);
                        text_center(localization.text[LOCAL_TEXT_CARD_LOAD_COMPLETE], 120);
                        menu_present();
                        buttons = card_dialog_buttons();
                    } while (!(buttons & 0xf000));
                    break;
                }
            }
        }
        if (buttons & 0x1000)
            break;
        menu_present();
    }
    menu_play_confirm_sound();
    result = menu_end_dialog(retained_s3);
    memcpy(card_text.load.wrapped, saved.wrapped, sizeof(saved.wrapped));
    memcpy(card_text.load.description, saved.description, sizeof(saved.description));
    return result;
}

static sint32 save_file_dialog(uint32 retained_s3)
{
    FUNCTION_MARKER(0x80068850u, "GAME.EXE");
    uint8 saved[257];
    uint32 buttons, entry_idx;
    DIRENTRY *entry;
    const TEXT_GRADIENT *pal;
    sint32 count, sel = 0, scroll = 0, i, y, last, result = 2;
    if (card_status_changed_with_context(retained_s3))
        return 2;
    count = card_list_files(0, &card_files.directory);
    if (card_file_exists(0, card_save_name, CARD_SAVE_NAME_SIZE))
        card_storage.free_blocks = card_storage.free_blocks + 1;
    if (card_storage.free_blocks)
    {
        if (card_status_changed_with_context(retained_s3))
            return 2;
        if (card_file_exists(0, card_save_name, CARD_SAVE_NAME_SIZE))
        {
            if (card_confirm_dialog(card_prompts[CARD_PROMPT_OVERWRITE], 1, 1))
                return 2;
            if (card_status_changed_with_context(retained_s3))
                return 2;
        }
        if (card_dialog.settings_only == 1)
            game_settings_capture_card();
        else
        {
            game_settings_capture_card();
            card_files.save.data.stage = (uint8)game_session.stage;
            card_files.save.data.player_count = (uint8)game_session.player_count;
            card_files.save.data.character_type[0] = (uint8)actor_types[0];
            card_files.save.data.character_type[1] = (uint8)actor_types[1];
            card_files.save.data.health[0] = actor_health[0].hp;
            card_files.save.data.health[1] = actor_health[1].hp;
            card_files.save.data.score[0] = player_scores[0].val;
            card_files.save.data.score[1] = player_scores[1].val;
            card_files.save.data.continue_threshold[0] = player_scores[0].continue_threshold;
            card_files.save.data.continue_threshold[1] = player_scores[1].continue_threshold;
            card_files.save.data.continues = level_transition.continues;
            card_files.save.data.route_choice = level_transition.route_choice;
        }
        /* Preserve B22C's saved S3 independently of the native filename */
        result = card_write_save(0, card_save_name, CARD_SAVE_NAME_SIZE, card_save_title, CARD_SAVE_TITLE_SIZE, 0x80093c04, card_files.save.bytes, CARD_FILE_SIZE);
        if (result != 1)
            return result;
        memset(card_files.verify.bytes, 255, CARD_FILE_SIZE);
        result = card_read_save(0, card_save_name, CARD_SAVE_NAME_SIZE, card_files.verify.bytes, CARD_FILE_SIZE);
        if (result != 1)
            return result;
        for (i = 0; i < CARD_DATA_SIZE; i++)
        {
            uint8 save_byte = card_files.save.data.bytes[i];
            uint8 verify_byte = card_files.verify.data.bytes[i];
            if (save_byte != verify_byte)
            {
                save_byte = card_files.save.data.bytes[i];
                verify_byte = card_files.verify.data.bytes[i];
                return save_byte == verify_byte;
            }
        }
        return 1;
    }
    if (card_confirm_dialog(card_prompts[CARD_PROMPT_FULL], 1, 1))
        return 2;
    if (card_status_changed_with_context(retained_s3))
        return 2;
    text_hint = TEXT_HINT_CHOOSE_SELECT_CANCEL;
    last = count - 1u;
    memcpy(saved, card_text.list + 1, 257);
    for (;;)
    {
        card_update();
        if (!card_poll_status(&card_storage.status))
        {
            menu_end_dialog(retained_s3);
            break;
        }
        buttons = card_dialog_buttons();
        menu_draw_dialog(retained_s3);
        text_wrap(localization.text[LOCAL_TEXT_CARD_DELETE_SELECT], card_text.list + 1, CARD_TEXT_CAPACITY);
        text_draw_lines(70);
        y = 12 * text_layout.line_count + 80;
        for (i = 0; i < 10 && (sint32)(i + (uint32)scroll) < count; ++i, y = y + 10u)
        {
            uint32 name_idx = scroll + (uint32)i;
            char *name;
            XASSERT(name_idx < CARD_DIRECTORY_FILE_COUNT);
            name = card_files.directory.titles + name_idx * CARD_DIRECTORY_TITLE_STRIDE;
            pal = sel == i ? &text_active_gradient : &text_inactive_gradient;
            name[23] = 0;
            snprintf(card_text.list + 1, 257, "%d.", (sint32)(i + (uint32)scroll + 1));
            text_draw_gradient(card_text.list + 1, 50 - text_width(card_text.list + 1), y, pal);
            text_draw_gradient(name, 40, y, pal);
        }
        retained_s3 = y;
        menu_present();
        if (buttons & 0x40)
        {
            if (sel < 9 && (sint32)(scroll + (uint32)sel) < last)
            {
                ++sel;
                menu_play_move_sound();
            }
            else if (sel == 9 && (sint32)(scroll + 9u) < last)
            {
                ++scroll;
                menu_play_move_sound();
            }
        }
        if (buttons & 0x10)
        {
            if (sel > 0)
            {
                --sel;
                menu_play_move_sound();
            }
            else if (!sel && scroll > 0)
            {
                --scroll;
                menu_play_move_sound();
            }
        }
        if (buttons & 0x1000)
        {
            menu_play_confirm_sound();
            menu_end_dialog(retained_s3);
            break;
        }
        if (!(buttons & 0x4000))
            continue;
        menu_play_confirm_sound();
        if (card_confirm_dialog(card_prompts[CARD_PROMPT_DELETE], 1, 1))
            break;
        menu_end_dialog(retained_s3);
        entry_idx = scroll + (uint32)sel;
        XASSERT(entry_idx < CARD_DIRECTORY_FILE_COUNT);
        entry = card_files.directory.entry + entry_idx;
        if (!card_status_changed_with_context(retained_s3))
            card_delete_file(0, (const char *)entry, CARD_DIRECTORY_NAME_SPAN);
        break;
    }
    memcpy(card_text.list + 1, saved, 257);
    return 2;
}

GDB_CALL sint32 card_save_dialog(void)
{
    FUNCTION_MARKER(0x80068d68u, "GAME.EXE");
    CARD_SAVE_TEXT saved = card_text.save;
    uint8 ch;
    uint32 buttons, i;
    const char *text;
    sint32 state, result;
    if (card_dialog.settings_only == 1)
    {
        text = localization.text[LOCAL_TEXT_SAVE_SETTINGS];
        if (text)
        {
            i = 0;
            do
            {
                ch = (uint8)text[i];
                XASSERT(i < CARD_TEXT_CAPACITY);
                card_text.title[i] = ch;
                ++i;
            } while (ch);
        }
        card_dialog.scene_backdrop = 1;
    }
    else
    {
        text = localization.text[LOCAL_TEXT_CARD_SAVE_TITLE];
        if (text)
        {
            i = 0;
            do
            {
                ch = (uint8)text[i];
                XASSERT(i < CARD_TEXT_CAPACITY);
                card_text.title[i] = ch;
                ++i;
            } while (ch);
        }
        card_dialog.scene_backdrop = 0;
        DrawSync(0);
        resource_arenas.persistent = resource_arenas.text_end;
        sound_samples_load(197);
        resource_load(3, 0x116000, NULL, TEXTURE_ARCHIVE_CAPACITY, &tex_file, NULL);
        tex_load_grps(0);
        tex_upload_pals();
        disp_set_fade(-256, -256, -256);
        if (card_confirm_dialog(card_prompts[CARD_PROMPT_SAVE], 1, 0))
        {
            result = -1;
            goto finished_save_dialog;
        }
    }
    card_storage.preview.stage = -1;
    if (card_dialog.settings_only == 1)
    {
        memset(card_files.save.data.bytes, 0, CARD_DATA_SIZE);
        card_files.save.data.player_count = 1;
        card_files.save.data.health[0] = 380;
    }
    text_hint = TEXT_HINT_NONE;
    for (;;)
    {
        buttons = card_dialog_buttons();
        card_update();
        state = card_poll_status(&card_storage.status);
        menu_draw_dialog(2);
        if (!state)
        {
            text_wrap(localization.text[LOCAL_TEXT_CARD_INSERT], card_text.save.wrapped, CARD_TEXT_CAPACITY);
            text_draw_lines(-1);
            text_hint = TEXT_HINT_EXIT;
        }
        else
        {
            if (card_storage.status == 2)
            {
                text_wrap(localization.text[LOCAL_TEXT_CARD_UNFORMATTED], card_text.save.wrapped, CARD_TEXT_CAPACITY);
                text_draw_lines(-1);
                text_hint = TEXT_HINT_FORMAT_EXIT;
            }
            else
            {
                text_hint = TEXT_HINT_SAVE_EXIT;
                card_format_description(card_text.save.description, CARD_TEXT_CAPACITY);
                text_wrap(card_text.save.description, card_text.save.wrapped, CARD_TEXT_CAPACITY);
                text_draw_lines(-1);
            }
            if (buttons & 0x4000)
            {
                menu_play_confirm_sound();
                menu_play_confirm_sound();
                menu_end_dialog(2);
                if ((card_storage.status & 3) != 1)
                {
                    format_dialog(2);
                    continue;
                }
                for (;;)
                {
                    state = save_file_dialog(2);
                    if (state > 0)
                        break;
                    if (card_confirm_dialog(card_prompts[CARD_PROMPT_SAVE_RETRY], 0, 1))
                    {
                        state = 2;
                        break;
                    }
                }
                if (state != 1)
                    continue;
                text_hint = TEXT_HINT_CONTINUE;
                do
                {
                    menu_draw_dialog(2);
                    text_center(localization.text[LOCAL_TEXT_CARD_SAVE_COMPLETE], 120);
                    menu_present();
                } while (!(card_dialog_buttons() & 0xf000));
                menu_play_confirm_sound();
                result = menu_end_dialog(2);
                break;
            }
        }
        menu_present();
        if (buttons & 0x1000)
        {
            menu_play_confirm_sound();
            if (card_dialog.settings_only == 1)
            {
                result = 1;
                break;
            }
            result = card_confirm_dialog(card_prompts[CARD_PROMPT_SAVE], 1, 0);
            if (result)
                break;
        }
    }
finished_save_dialog:
    card_text.save = saved;
    return result;
}

static void card_dialog_choices(sint32 choice)
{
    text_draw_gradient(localization.text[LOCAL_TEXT_YES], 110, 170, choice ? &text_inactive_gradient : &text_active_gradient);
    text_draw_gradient(localization.text[LOCAL_TEXT_NO], 170, 170, choice ? &text_active_gradient : &text_inactive_gradient);
}

GDB_CALL sint32 card_confirm_dialog(CARD_PROMPT prompt, sint32 choice, sint32 monitor_card)
{
    FUNCTION_MARKER(0x80068520u, "GAME.EXE");
    uint8 saved[257];
    uint32 buttons;
    sint32 result;
    memcpy(saved, card_text.confirm, 257);
    text_hint = TEXT_HINT_SELECT_CANCEL;
    for (;;)
    {
        buttons = card_dialog_buttons();
        card_update();
        result = card_poll_status(&card_storage.status);
        if (monitor_card == 1 && !result)
        {
            menu_end_dialog(prompt.scene_context);
            result = 1;
            break;
        }
        menu_draw_dialog(prompt.scene_context);
        text_wrap(prompt.text, card_text.confirm + 1, CARD_TEXT_CAPACITY);
        text_draw_lines(-1);
        card_dialog_choices(choice);
        menu_present();
        if (buttons & 0x80)
        {
            choice = 0;
            menu_play_move_sound();
        }
        if (buttons & 0x20)
        {
            choice = 1;
            menu_play_move_sound();
        }
        if (buttons & 0x4000)
        {
            menu_play_confirm_sound();
            menu_end_dialog(prompt.scene_context);
            result = choice;
            break;
        }
        if (buttons & 0x1000)
        {
            menu_play_confirm_sound();
            menu_end_dialog(prompt.scene_context);
            result = 1;
            break;
        }
    }
    memcpy(card_text.confirm, saved, 257);
    return result;
}

static sint32 format_dialog(uint32 retained_s3)
{
    FUNCTION_MARKER(0x80068258u, "GAME.EXE");
    uint8 saved[257];
    uint32 buttons, prev, timer;
    sint32 choice = 1, result = -1;
    memcpy(saved, card_text.confirm, 257);
    for (;;)
    {
        text_hint = TEXT_HINT_SELECT_CANCEL;
        buttons = card_dialog_buttons();
        menu_draw_dialog(retained_s3);
        text_wrap(localization.text[LOCAL_TEXT_CARD_FORMAT_ASK], card_text.confirm + 1, CARD_TEXT_CAPACITY);
        text_draw_lines(-1);
        card_dialog_choices(choice);
        menu_present();
        if (buttons & 0x80)
        {
            choice = 0;
            menu_play_move_sound();
        }
        if (buttons & 0x20)
        {
            choice = 1;
            menu_play_move_sound();
        }
        if (buttons & 0x4000)
            break;
        if (buttons & 0x1000)
        {
            menu_play_confirm_sound();
            menu_end_dialog(retained_s3);
            goto finished;
        }
    }
    menu_play_confirm_sound();
    menu_end_dialog(retained_s3);
    if (choice == 1)
        goto finished;
    prev = card_storage.status;
    text_hint = TEXT_HINT_NONE;
    draw_notice(localization.text[LOCAL_TEXT_CARD_ACCESS], retained_s3);
    card_wait_status(&card_storage.status);
    if (prev != card_storage.status)
        goto finished;
    draw_notice(localization.text[LOCAL_TEXT_CARD_FORMATTING], retained_s3);
    card_format(0);
    card_wait_status(&card_storage.status);
    prev = card_storage.status;
    text_hint = TEXT_HINT_CONTINUE;
    timer = 0;
    if ((prev & 3) != 1)
    {
        do
        {
            menu_draw_dialog(retained_s3);
            text_wrap(localization.text[LOCAL_TEXT_CARD_FORMAT_FAILED], card_text.confirm + 1, CARD_TEXT_CAPACITY);
            text_draw_lines(-1);
            menu_present();
            buttons = card_dialog_buttons();
            ++timer;
        } while (!(buttons & 0xf000) && timer < 150);
        menu_end_dialog(retained_s3);
    }
    else
    {
        do
        {
            menu_draw_dialog(retained_s3);
            text_center(localization.text[LOCAL_TEXT_CARD_FORMAT_COMPLETE], 120);
            menu_present();
            buttons = card_dialog_buttons();
            if (buttons & 0xf000)
                break;
            ++timer;
        } while (timer < 150);
        menu_end_dialog(retained_s3);
        result = 0;
    }
finished:
    memcpy(card_text.confirm, saved, 257);
    return result;
}

static sint32 message_dialog(const char *text, uint32 retained_s3)
{
    FUNCTION_MARKER(0x800686a8u, "GAME.EXE");
    uint32 i, buttons;
    DrawSync(0);
    resource_load(3, 0x116000, NULL, TEXTURE_ARCHIVE_CAPACITY, &tex_file, NULL);
    tex_load_grps(0);
    tex_upload_pals();
    disp_set_fade(-256, -256, -256);
    text_hint = TEXT_HINT_CONTINUE;
    for (i = 0; i < 2; ++i)
    {
        menu_draw_dialog(retained_s3);
        text_center(text, 125);
        menu_present();
    }
    do
    {
        pause_refresh_pad();
        buttons = card_dialog_buttons();
    } while (!(buttons & 0x5000));
    menu_end_dialog(retained_s3);
    text_hint = TEXT_HINT_NONE;
    return -1;
}

/* The prefix retains the original overlong-first-word terminator */
static sint32 draw_wrapped_dialog(const char *text, int reload_text, uint32 retained_s3)
{
    uint8 saved[257];
    uint32 i;
    char *workspace = card_text.notice;
    memcpy(saved, workspace, sizeof(saved));
    text_hint = TEXT_HINT_NONE;
    for (i = 0; i < 2; ++i)
    {
        menu_draw_dialog(retained_s3);
        text_wrap(reload_text ? localization.text[LOCAL_TEXT_CARD_SAVING] : text, card_text.notice + 1, CARD_TEXT_CAPACITY);
        text_draw_lines(-1);
        menu_present();
    }
    // TODO Fix workspace lifetime for pointers published by text_wrap
    memcpy(workspace, saved, sizeof(saved));
    return 0;
}

sint32 card_draw_pending_notice(uint32 retained_s3)
{
    FUNCTION_MARKER(0x80068788u, "GAME.EXE");
    return draw_wrapped_dialog(0, 1, retained_s3);
}

static sint32 draw_notice(const char *text, uint32 retained_s3)
{
    FUNCTION_MARKER(0x800687e8u, "GAME.EXE");
    return draw_wrapped_dialog(text, 0, retained_s3);
}

sint32 card_menu_load_game(uint32 retained_s3)
{
    FUNCTION_MARKER(0x80052428u, "GAME.EXE");
    card_dialog.settings_only = 0;
    load_dialog(retained_s3);
    menu.skip_scene = 1;
    return 1;
}

sint32 card_menu_load_settings(uint32 retained_s3)
{
    FUNCTION_MARKER(0x80052458u, "GAME.EXE");
    card_dialog.settings_only = 1;
    return load_dialog(retained_s3);
}

GDB_CALL sint32 card_menu_save_settings(void)
{
    FUNCTION_MARKER(0x80052484u, "GAME.EXE");
    card_dialog.settings_only = 1;
    return card_save_dialog();
}
