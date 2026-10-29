#include <stdio.h>

#include "game.h"

static const char pair_names_french[] = "%s ET %s";
static const char pair_names_german[] = "%s UND %s";
static const char pair_names_english[] = "%s AND %s";

CHARACTER_MENU character_menu;
const CHARACTER_MENU character_menu_defaults = {{1, 2}, 0, 0, 0, 0, 0, 0};
const uint32 character_menu_types[CHARACTER_MENU_ACTOR_COUNT] = {3, 0, 1, 2};

/* Return one only if the candidate is absent */
static sint32 character_menu_available(sint32 candidate, const uint32 *sel, sint32 count)
{
    FUNCTION_MARKER(0x8004f4b8, "GAME.EXE");
    sint32 i;
    for (i = 0; i < count; i++)
        if (sel[i] == (uint32)candidate)
            return 0;
    return 1;
}

/* Original stack-local name string stays local through packet emission */
static void draw_pair_names(const char *format, const char *name0, const char *name1)
{
    char buffer[72];
    snprintf(buffer, 72, format, name0, name1);
    text_center(buffer, 50);
}

/* Cancel clears ready bits but keeps processing same-frame input */
GDB_CALL sint32 character_menu_update(void)
{
    FUNCTION_MARKER(0x80050798, "GAME.EXE");
    sint32 i;
    uint32 actor1, actor0, available, players, ready;
    actor1 = character_menu.sel[1];
    actor0 = character_menu.sel[0];
    if (actor1 == actor0)
        character_menu.sel[1] = (actor1 + 1) & 3;
    text_center(localization.hint[TEXT_HINT_MOVE_CHOOSE_BACK], 214);
    pad_refresh_ports();
    available = pad_decode.count;
    players = character_menu.player_count;
    game_session.player_count = available;
    if ((sint32)available < (sint32)players)
    {
        menu.callback = &menu_callback_main;
        camera_capture(camera_menu_dst.bits, 0, -12032);
        return menu_play_confirm_sound();
    }
    if (players == 1)
    {
        text_center(localization.text[LOCAL_TEXT_SELECT_FIGHTER], 28);
        uint32 sel_idx = character_menu.sel[0];
        sint32 type = actor_types[sel_idx];
        XASSERT(type >= 0 && type < CHARACTER_NAME_TYPE_COUNT);
        text_center(character_name_text(character_name_types[type]), 50);
    }
    else
    {
        uint32 language, type0, type1;
        const char *format, *name0, *name1;
        text_center(localization.text[LOCAL_TEXT_SELECT_FIGHTERS], 28);
        language = game_settings.language;
        format = language == 1 ? pair_names_french : language == 2 ? pair_names_german : pair_names_english;
        actor0 = character_menu.sel[0];
        actor1 = character_menu.sel[1];
        type0 = actor_types[actor0];
        type1 = actor_types[actor1];
        XASSERT(type0 < CHARACTER_NAME_TYPE_COUNT && type1 < CHARACTER_NAME_TYPE_COUNT);
        name0 = character_name_text(character_name_types[type0]);
        name1 = character_name_text(character_name_types[type1]);
        draw_pair_names(format, name0, name1);
    }
    for (i = 0; i < (sint32)character_menu.player_count; i++)
    {
        uint32 raw = pad_decode_buttons(pad_game.port[i]), cancel;
        uint32 *sel = character_menu.sel + i;
        uint32 *edge = &pad_menu.pressed[i];
        *edge = (raw ^ pad_menu.prev[i]) & raw;
        pad_menu.prev[i] = raw;
        cancel = *edge;
        pad_menu.last_raw = raw;
        if (cancel & 0x1000)
        {
            menu.callback = &menu_callback_main;
            character_menu.ready_mask = 0;
            camera_capture(camera_menu_dst.bits, 0, -12032);
            menu_play_confirm_sound();
        }
        if (!((character_menu.ready_mask >> (i & 31u)) & 1))
        {
            if (*edge & 0x80)
            {
                sint32 sel_val = *sel;
                sint32 candidate = sel_val - 1u;
                if (sel_val <= 0)
                    goto select_next;
                for (; candidate >= 0; candidate--)
                    if (character_menu_available(candidate, character_menu.sel, character_menu.player_count))
                    {
                        *sel = candidate;
                        camera_orbit();
                        menu_play_move_sound();
                        break;
                    }
                if (candidate == -1)
                    menu_play_effect(5, 223);
            }
        select_next:
            if (*edge & 0x20)
            {
                sint32 sel_val = *sel;
                sint32 candidate = sel_val + 1u;
                if (sel_val >= 3)
                    goto select_confirm;
                for (; candidate < 4; candidate++)
                    if (character_menu_available(candidate, character_menu.sel, character_menu.player_count))
                    {
                        *sel = candidate;
                        camera_orbit();
                        menu_play_move_sound();
                        break;
                    }
                if (candidate == 4)
                    menu_play_effect(5, 223);
            }
        select_confirm:
            if (*edge & 0x4000)
            {
                menu_play_confirm_sound();
                players = character_menu.player_count;
                ready = character_menu.ready_mask;
                game_session.player_count = players;
                character_menu.ready_mask = ready | (1u << (i & 31u));
            }
        }
    }
    players = character_menu.player_count;
    if (players == 1)
    {
        ready = character_menu.ready_mask;
        if (ready == players)
        {
            character_menu.confirmed = ready;
            players = character_menu.player_count;
        }
    }
    if (players == 2)
    {
        if (character_menu.ready_mask == 3)
            character_menu.confirmed = 1;
        return 1;
    }
    return 3;
}

/* Draw four menu actors, their shadows and selected-player rings */
GDB_CALL sint32 character_menu_draw(const uint32 *sel, sint32 players)
{
    FUNCTION_MARKER(0x80058e24, "GAME.EXE");
    uint32 i, phase, fade, actor_x, actor_y, actor_z;
    uint32 camera_x, camera_y, camera_z, height, b, g;
    int slot, changed;
    sint32 r;
    sint32 val;
    uint32 type;
    if (camera.angles.pitch >= 4097)
        return 0;
    transform_identity();
    transform_pitch(camera.angles.pitch);
    transform_yaw(camera.angles.yaw);
    character_menu.pulse_phase = (character_menu.pulse_phase + 64) & 2047;
    for (i = 0; i < 4; i++)
    {
        ACTOR_LIGHTING *light = actor_lighting + i;
        ACTOR_FRAME_MOTION *motion = &actor_animations[i].motion;
        const FIXED_VEC3 *pos = actor_positions + i;
        const ACTOR_ORIENTATION *orientation = actor_orientation + i;
        changed = 0;
        slot = players > 0 && i == sel[0] ? 0 : players >= 2 && i == sel[1] ? 1 : -1;
        if (slot >= 0)
        {
            if (!(character_menu.ready_mask & (1u << slot)))
            {
                phase = character_menu.pulse_phase;
                val = sin_table[phase] >> 10;
                changed = 1;
                light->b = val;
                light->g = val;
                light->r = val;
            }
            else
            {
                r = light->r;
                if (r < 32)
                {
                    val = (sint8)(r + 4);
                    light->r = val;
                    if (val > 32)
                        light->r = 32;
                    val = (uint8)light->r;
                    light->b = val;
                    light->g = val;
                    changed = 1;
                }
            }
        }
        else
        {
            fade = character_menu.fade_tick;
            if (fade)
            {
                fade <<= 2;
                val = (sint32)fade >= 129 ? -128 : (sint32)((uint32)(-(sint64)(fade)));
                light->b = val;
                light->g = val;
                light->r = val;
                changed = 1;
            }
            else
            {
                r = light->r;
                if (r > 0)
                {
                    light->r = r - 1;
                    --light->g;
                    --light->b;
                    light->flags |= 1;
                    r = light->r;
                }
                if (r < 0)
                {
                    light->r = r + 1;
                    ++light->g;
                    ++light->b;
                    changed = 1;
                }
            }
        }
        if (changed)
            light->flags |= 1;
        transform_push();
        actor_x = pos->x.bits;
        camera_x = camera.pos.x.bits;
        type = actor_types[i];
        actor_y = pos->y.bits;
        height = model_height[type];
        camera_y = camera.pos.y.bits;
        actor_z = pos->z.bits;
        camera_z = camera.pos.z.bits;
        transform_translate(actor_x - camera_x, actor_y + height - camera_y, actor_z - camera_z);
        transform_yaw((sint16)((uint32)(-(sint64)(32u * (orientation->yaw)))) + 32768);
        actor_y = motion->bits[0];
        actor_z = motion->forward_offset_bits;
        transform_translate(0, actor_y, actor_z);
        b = light->b + 128;
        g = light->g + 128;
        r = light->r;
        render_style.color.rgbc = 0x2c000000u + (r + 128) + (g << 8) + (b << 16);
        render_style.blend_mode = 0;
        model_draw_pose(model_refs[actor_types[i]], &actor_poses[i]);
        transform_pop();
        transform_push();
        actor_x = pos->x.bits;
        camera_x = camera.pos.x.bits;
        camera_y = camera.pos.y.bits;
        actor_z = pos->z.bits;
        camera_z = camera.pos.z.bits;
        transform_translate(actor_x - camera_x, (uint32)(-(sint64)(camera_y)), actor_z - camera_z);
        transform_pitch(32768);
        transform_yaw((sint16)(32u * (orientation->yaw)));
        model_draw_menu(model_refs[860], 0, 0);
        transform_pop();
        character_menu.ring_phase = (character_menu.ring_phase + 8) & 2047;
        slot = players > 0 && i == sel[0] ? 0 : players >= 2 && i == sel[1] ? 1 : -1;
        if (slot >= 0)
        {
            transform_push();
            actor_x = pos->x.bits;
            camera_x = camera.pos.x.bits;
            camera_y = camera.pos.y.bits;
            actor_z = pos->z.bits;
            camera_z = camera.pos.z.bits;
            transform_translate(actor_x - camera_x, (uint32)(-(sint64)(camera_y)), actor_z - camera_z);
            transform_pitch(32768);
            transform_yaw((sint16)(32u * character_menu.ring_phase));
            model_draw_menu(model_refs[slot == 0 ? 887 : 888], 0, 0);
            transform_pop();
        }
    }
    return 0;
}

/* Advance all four menu actors independently of the selected player count */
GDB_CALL sint32 character_menu_advance_actors(void)
{
    FUNCTION_MARKER(0x80050cc8, "GAME.EXE");
    sint32 i;
    for (i = 0; i < CHARACTER_MENU_ACTOR_COUNT; i++)
    {
        ACTOR_ANIMATION *playback = actor_animations + i;
        if (playback->src.kind == ACTOR_ANIMATION_SOURCE_NONE)
            playback->frame = 0xffffffffu;
        if (playback->frame == 0xffffffffu)
            animation_start(i, animation_type_banks[(uint32)actor_types[i]], 69);
        if (playback->frame != 0xffffffffu)
            animation_update_motion(i);
    }
    return 0;
}
