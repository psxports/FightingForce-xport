#include <stdio.h>

#include "game.h"

const char continue_count_format[] = "%sS = %d";

LEVEL_TRANSITION level_transition;

GDB_CALL sint32 level_transition_at_exit(void)
{
    FUNCTION_MARKER(0x8001886C, "GAME.EXE");
    uint32 mask = 1, stage = game_session.stage, i;
    sint32 result = 1, coord;
    if (game_session.player_count != 1)
    {
        mask = (actor_health[0].hp > 0 ? 1u : 0u) | (actor_health[1].hp > 0 ? 2u : 0u);
        if (!mask)
        {
            if (!level_transition.continue_phase)
                level_transition.continue_phase = 1;
            return 0;
        }
    }
    if (stage == 4 || stage == 12 || stage == 14 || (stage > 14 && stage != 22))
        return 0;
    if (stage == 22)
        return players_before_z(0x1e900000) == 0;
    if (stage == 13)
    {
        for (i = 0; i < 2; i++)
            if (mask & (1u << i))
            {
                coord = actor_positions[i].x.whole;
                if (coord >= 7168)
                    level_transition.route_choice = 1;
                if (coord < 3073)
                    level_transition.route_choice = 0;
                if ((uint32)(coord - 3073) < 0xfffu)
                    result = 0;
            }
        coord = level_transition.route_exit_tick;
        if (!coord && result == 1)
        {
            level_transition.route_exit_tick = 1;
            coord = 1;
        }
        return coord >= 211;
    }
    for (i = 0; i < 2; i++)
        if (mask & (1u << i))
        {
            switch (stage)
            {
                case 0:
                    if ((sint32)actor_positions[i].z.bits <= 0x09ffffff)
                        result = 0;
                    break;
                case 1:
                    if ((sint32)actor_positions[i].z.bits > (sint32)0xf9000000u)
                        result = 0;
                    break;
                case 2:
                    if ((sint32)actor_positions[i].z.bits <= 0x031fffff)
                        result = 0;
                    break;
                case 3:
                    if ((sint32)actor_positions[i].z.bits <= 0x07bfffff)
                        result = 0;
                    break;
                case 5:
                    if ((sint32)actor_positions[i].z.bits <= 0x090fffff)
                        result = 0;
                    break;
                case 6:
                    if ((actor_positions[i].x.bits + 0x0c4fffffu) <= 0x0c27fffeu && (sint32)actor_positions[i].z.bits <= 0x04a7ffff)
                        result = 0;
                    break;
                case 7:
                    if ((sint32)actor_positions[i].x.bits > (sint32)0xe2f00000u)
                        result = 0;
                    break;
                case 8:
                    if ((sint32)actor_positions[i].z.bits <= 0x073fffff)
                        result = 0;
                    break;
                case 9:
                    if ((sint32)actor_positions[i].z.bits <= 0x0e9fffff)
                        result = 0;
                    break;
                case 10:
                    if ((sint32)actor_positions[i].z.bits <= 0x1adfffff)
                        result = 0;
                    break;
                case 11:
                    if ((sint32)actor_positions[i].z.bits <= 0x1c5fffff)
                        result = 0;
                    break;
                default:
                    return 0;
            }
        }
    return result;
}

GDB_CALL sint32 level_transition_continue_prompt(void)
{
    FUNCTION_MARKER(0x8005FB44, "GAME.EXE");
    uint32 val = level_transition.continue_phase, step, buttons;
    const TEXT_GRADIENT *pal;
    char text[128];
    sint32 r = 0;
    sint32 g;
    sint32 timer;
    uint32 width;
    sint32 x;
    if ((sint32)val <= 0)
        return val;
    step = val - 128u;
    if ((sint32)step < 0)
        step = 0;
    g = (uint32)(-(sint64)(step << 1));
    if (g < -256)
        g = -256;
    if ((sint32)step >= 65)
    {
        r = (uint32)(-(sint64)((step - 64u) << 1));
        if (r < -256)
            r = -256;
    }
    disp_set_fade(r, g, g);
    level_prepare_frame_noop();
    if ((sint32)level_transition.continues < (sint32)game_session.player_count)
    {
        level_transition.continue_timer = 329;
        goto advance;
    }
    if (level_transition.continue_phase != 191)
        goto advance;
    text_gradient_update();
    sprintf(text, "%s ?", localization.text[LOCAL_TEXT_CONTINUE]);
    text_center(text, 100);
    timer = level_transition.continue_timer;
    sprintf(text, "%d", (sint32)(10u - (timer / 30)));
    timer = level_transition.continue_timer;
    if (timer % 30 < 4)
        text_center_gradient(text, 130);
    else
        text_center(text, 130);
    if (game_settings.language == GAME_LANGUAGE_GERMAN)
        sprintf(text, "ANZAHL DER CONTINUES = %d", (sint32)level_transition.continues);
    else
    {
        const char *label = localization.text[LOCAL_TEXT_CONTINUE];
        sint32 continues = level_transition.continues;
        sprintf(text, continue_count_format, label, continues);
    }
    text_center(text, 190);
    width = text_width(localization.text[LOCAL_TEXT_NO]);
    x = text_width(localization.text[LOCAL_TEXT_YES]);
    x = (sint32)(320u - width - x) >> 1;
    pal = level_transition.continue_choice ? &text_inactive_gradient : &text_active_gradient;
    text_draw_gradient(localization.text[LOCAL_TEXT_YES], x, 160, pal);
    pal = level_transition.continue_choice == 1 ? &text_active_gradient : &text_inactive_gradient;
    width = text_width(localization.text[LOCAL_TEXT_YES]);
    text_draw_gradient(localization.text[LOCAL_TEXT_NO], x + width + 10u, 160, pal);
    buttons = pad_decode_buttons(pad_game.port[0]);
    if (game_session.player_count == 2)
        buttons |= (uint32)pad_decode_buttons(pad_game.port[1]);
    if (buttons & 128)
        level_transition.continue_choice = 0;
    if (buttons & 32)
        level_transition.continue_choice = 1;
    val = level_transition.continue_timer + 1u;
    level_transition.continue_timer = val;
    if (val == 329)
        level_transition.continue_phase = level_transition.continue_phase + 2u;
    if (!(buttons & 16384))
        return 1;
    if (level_transition.continue_choice != 1)
    {
        level_transition.continue_phase = 384;
        return 384;
    }
    level_transition.continue_timer = 329;
    level_transition.continues = 0;
advance:
    val = level_transition.continue_phase + 2u;
    level_transition.continue_phase = val;
    return val;
}

/* Original main-loop phase15038..1556C,not a separate MIPS entry */
GDB_CALL sint32 level_transition_update(void)
{
    uint32 i, pad, transition = level_transition.finish_tick;
    sint32 type, result, fade;
    if (transition)
    {
        fade = transition << 3;
        if (transition == 1)
        {
            uint32 players, stage;
            for (i = 0; i < 2; i++)
                actor_bonus_totals[i] = actor_bonus_counts[i];
            players = game_session.player_count;
            if ((sint32)players > 0)
            {
                stage = game_session.stage;
                for (i = 0; (sint32)i < (sint32)players; i++)
                {
                    /* TODO: Investigate finish recovery outside the native actor pool */
                    XASSERT(i < ACTOR_COUNT);
                    sint32 hp = actor_health[i].hp, maximum, next;
                    if (hp <= 0)
                        continue;
                    maximum = actor_health[i].max;
                    next = hp + (maximum >> 2);
                    if (stage == 6 || stage == 13 || stage - 20 < 2)
                        next = maximum;
                    if (next > maximum - 4)
                        next = maximum - 4;
                    actor_health[i].hp = (uint16)next;
                }
            }
            fade = level_transition.finish_tick << 3;
        }
        if (fade > 32)
            fade = 32;
        disp_fade_from_origin(255 - (uint32)fade);
        transition = level_transition.finish_tick;
        if ((sint32)transition >= 91 && !(transition & 3))
        {
            sint32 changed = 0;
            i = 0;
            if ((sint32)game_session.player_count > 0)
            {
                do
                {
                    /* TODO: Investigate unsupported finish-phase player counts */
                    XASSERT(i < ACTOR_COUNT);
                    if (actor_health[i].hp > 0)
                    {
                        sint32 count = (sint16)actor_bonus_counts[i];
                        uint32 score;
                        if (count > 0)
                        {
                            actor_bonus_counts[i] = count - 1;
                            XASSERT(i < PLAYER_COUNT);
                            player_scores[i].val += 250;
                            changed = 1;
                            hud_format_score(i);
                        }
                        XASSERT(i < PLAYER_COUNT);
                        score = player_scores[i].val;
                        if ((sint32)player_scores[i].continue_threshold < (sint32)score)
                        {
                            level_transition.continues = level_transition.continues + 1;
                            player_scores[i].continue_threshold = score + 50000;
                            sound_play_chord();
                            disp_buffers.flash_ticks = 4;
                        }
                    }
                    if (changed)
                        sound_voice_play(46, 255, 7168);
                    ++i;
                } while ((sint32)i < (sint32)game_session.player_count);
                transition = level_transition.finish_tick;
            }
        }
        ++transition;
        level_transition.finish_tick = transition;
        if (transition == 240 || level_transition_at_exit())
        {
            if (level_transition_at_exit() && game_session.stage != 13)
            {
                level_transition.bonus_item_pending = 1;
                sound_play_chord();
                disp_buffers.flash_ticks = 4;
            }
            for (fade = 224; fade > 0; fade -= 16)
                level_draw_fade_step(fade);
            loading_clear();
            sound_voices_stop_all();
            pad_stream_disable();
            return 2;
        }
    }
    else
    {
        result = level_script_update();
        if (game_session.stage == 23 && game_session.scene_phase && !level_transition.finish_tick)
            level_transition.finish_tick = 1;
        if (result == -1)
        {
            for (i = 2; i < 8; i++)
            {
                type = actor_types[i];
                if (type != -1 && type != 17)
                    break;
            }
            if (i == 8 && !level_transition.finish_tick)
                level_transition.finish_tick = 1;
        }
        if (level_transition_at_exit() && !level_transition.finish_tick)
            level_transition.finish_tick = 1;
    }
    if ((sint32)game_session.tick >= 33 && !level_transition.continue_phase && demo.cur_idx == 0xffffffff && !level_transition.finish_tick)
    {
        i = 0;
        if ((sint32)game_session.player_count > 0)
        {
            do
            {
                if (actor_health[i].hp)
                {
                    pad = (uint32)pad_decode_buttons(pad_game.port[i]);
                    if ((sint8)pad_decode.type == -1)
                        game_pause_flags = (uint8)(game_pause_flags | ((uint32)5 << (i & 31)));
                    if (pad & 8)
                        game_pause_flags = (uint8)(game_pause_flags | ((uint32)1 << (i & 31)));
                    if ((sint8)game_pause_flags)
                        goto pause_run;
                }
                ++i;
            } while ((sint32)i < (sint32)game_session.player_count);
        }
    }
    if (!(sint8)game_pause_flags)
        return 0;
pause_run:
    if (game_pause_run())
    {
        level_transition_noop();
        sound_voices_stop_all();
        pad_stream_disable();
        return 1;
    }
    game_pause_flags = 0;
    return 0;
}

GDB_CALL uint32 level_draw_fade_step(sint32 level)
{
    FUNCTION_MARKER(0x8001453cu, "GAME.EXE");
    frame_prepare();
    disp_draw_scene();
    stage_update_world();
    hud_update();
    return disp_fade_from_origin(level);
}

GDB_CALL void level_transition_noop(void)
{
    FUNCTION_MARKER(0x8005e138u, "GAME.EXE");
}

GDB_CALL void level_prepare_frame_noop(void)
{
    FUNCTION_MARKER(0x80011fecu, "GAME.EXE");
}
