#include "game.h"
#include "platform.h"

/* Player pause bits0/1 and disconnected-controller bits2/3 */
uint8 game_pause_flags;

static void pause_frame(void)
{
    pause_refresh_pad();
    frame_prepare();
    disp_draw_scene();
    hud_update();
}

static void pause_connection(uint32 player)
{
    uint32 flags = game_pause_flags;
    if ((sint8)pad_decode.type == -1)
        flags |= 4u << player;
    else
        flags &= ~(4u << player);
    game_pause_flags = (uint8)flags;
}

static sint32 pause_choice(uint32 buttons, sint32 choice)
{
    if ((buttons & 128u) && choice == 1)
    {
        choice = 0;
        menu_play_move_sound();
    }
    if ((buttons & 32u) && !choice)
    {
        choice = 1;
        menu_play_move_sound();
    }
    return choice;
}

/* Original5DD18..5E03C menu/quit phases retain read-before-draw order */
static sint32 pause_select(uint32 player, uint32 pad)
{
    uint32 buttons;
    sint32 choice = 0, left, right, x;
    do
    {
        pause_refresh_pad();
        buttons = (uint32)pad_decode_buttons(pad);
        pause_connection(player);
        choice = pause_choice(buttons, choice);
        text_gradient_update();
        frame_prepare();
        disp_draw_scene();
        right = text_width(localization.text[LOCAL_TEXT_QUIT]);
        left = text_width(localization.text[LOCAL_TEXT_CONTINUE]);
        x = (sint32)(320u - (uint32)right - (uint32)left) >> 1;
        text_draw_gradient(localization.text[LOCAL_TEXT_CONTINUE], x, 160, choice ? &text_inactive_gradient : &text_active_gradient);
        left = text_width(localization.text[LOCAL_TEXT_CONTINUE]);
        text_draw_gradient(localization.text[LOCAL_TEXT_QUIT], (uint32)x + (uint32)left + 10u, 160, choice == 1 ? &text_active_gradient : &text_inactive_gradient);
        text_center(localization.hint[TEXT_HINT_CHOOSE_SELECT], 214);
        hud_update();
    } while (!(buttons & 0x4000u));
    menu_play_confirm_sound();
    do
    {
        pause_refresh_pad();
        buttons = (uint32)pad_decode_buttons(pad);
    } while (buttons & 0x4000u);
    if (choice != 1)
    {
        game_pause_flags = 0;
        return 0;
    }
    do
    {
        pause_refresh_pad();
        buttons = (uint32)pad_decode_buttons(pad);
        pause_connection(player);
        choice = pause_choice(buttons, choice);
        text_gradient_update();
        frame_prepare();
        disp_draw_scene();
        text_center(localization.text[LOCAL_TEXT_QUIT_ASK], 150);
        text_draw_gradient(localization.text[LOCAL_TEXT_YES], 110, 160, choice ? &text_inactive_gradient : &text_active_gradient);
        text_draw_gradient(localization.text[LOCAL_TEXT_NO], 170, 160, choice == 1 ? &text_active_gradient : &text_inactive_gradient);
        text_center(localization.hint[TEXT_HINT_CHOOSE_SELECT], 214);
        hud_update();
    } while (!(buttons & 0x4000u));
    menu_play_confirm_sound();
    if (!choice)
        return 1;
    return 0;
}

sint32 game_pause_run(void)
{
    FUNCTION_MARKER(0x8005DBBC, "GAME.EXE");
    uint32 player = (game_pause_flags & 2u) != 0, pad = pad_game.port[player], buttons, flags;
    cd_music_pause_playback();
    sound_voices_pause();
    do
    {
        pause_frame();
        buttons = pad_decode_buttons(pad);
    } while (buttons & 8u);
    do
    {
        pause_frame();
        buttons = pad_decode_buttons(pad);
        flags = game_pause_flags;
        if ((sint8)pad_decode.type == -1)
            flags |= 4u << player;
        else
            flags &= ~(4u << player);
        if (buttons & 8u)
            flags &= ~(1u << player);
        game_pause_flags = (uint8)flags;
        if ((buttons & 1u) && pause_select(player, pad))
            return 1;
    } while ((sint8)game_pause_flags);
    do
    {
        pause_frame();
        buttons = pad_decode_buttons(pad);
        if ((sint8)pad_decode.type == -1)
            game_pause_flags = game_pause_flags | (4u << player);
    } while (buttons & 8u);
    pause_frame();
    cd_music_resume_playback();
    sound_voices_resume();
    return 0;
}
