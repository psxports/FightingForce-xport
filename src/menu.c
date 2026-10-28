#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "game.h"
#include "psx_gpu.h"
#include "platform.h"

static const char cheat_text[] = "CHEAT MODE";
static const char message_ok[] = "OK";
static const char message_why_not[] = "WHY NOT";
static const char message_cannot[] = "WELL YOU CANT";
static const char language_english[] = "ENGLISH";
static const char language_french[] = "FRANCAIS";
static const char language_german[] = "DEUTSCH";
static const char language_heading[] = "SELECT LANGUAGE";
static const char language_hint[] = "] SELECT";

const MENU_CALLBACK menu_callback_main = {main_menu_update};
const MENU_CALLBACK menu_callback_character = {character_menu_update};
const MENU_CALLBACK menu_callback_options = {options_menu_update};
const MENU_CALLBACK menu_callback_pad = {pad_menu_update};
const MENU_CALLBACK menu_callback_scr = {scr_menu_update};
const MENU_CALLBACK menu_callback_scores = {scores_menu_update};
static const MENU_CALLBACK callback_message = {menu_message_update};
static const MENU_CALLBACK callback_question = {menu_question_update};

MENU menu;
MENU_TEXT menu_text;
const MENU menu_defaults = {
    .ambient_voice = {18, 19},
    .main_row_count = {2, 3},
    .main_gradient = TEXT_GRADIENT_ACTIVE_INIT,
    .message_gradient = TEXT_GRADIENT_ACTIVE_INIT,
    .question_gradient = TEXT_GRADIENT_ACTIVE_INIT,
    .callback = &menu_callback_main,
    .message_return = &menu_callback_main,
    .question_accept = &menu_callback_main,
    .question_cancel = &menu_callback_main,
};

GDB_CALL uint32 language_menu_draw(void)
{
    const TEXTURE_DESCRIPTOR *desc;
    FUNCTION_MARKER(0x800697a4u, "GAME.EXE");
    uint32 p;
    const TEXT_GRADIENT *pal;
    int i;
    sint32 width, light_x, light_y, light_z;
    text_gradient_update();
    disp_begin_frame();
    light_x = disp_buffers.fade.r;
    light_y = disp_buffers.fade.g;
    light_z = disp_buffers.fade.b;
    if (light_x + light_y + light_z)
        disp_draw_fade();
    uint32 desc_idx = (uint32)tex_map[(uint32)tex_grps[TEXTURE_GROUP_LANGUAGE].first_tex];
    if (desc_idx > TEXTURE_DESCRIPTOR_CAPACITY - 3)
        diagnostic_stop(__FUNCTION__, __FILE__, __LINE__);
    desc = tex_desc.item + desc_idx;
    for (i = 0; i < 3; ++i, desc++)
    {
        p = render_buffers.packet_addr;
        render_buffers.packet_addr = p + 20;
        SPRT *spr = psx_addr(p, sizeof(*spr));
        setSprt(spr);
        spr->p[0].uv.u = desc->uv0.u;
        spr->p[0].uv.v = desc->uv0.v;
        spr->w = i == 2 ? 64 : 128;
        spr->h = 255;
        spr->p[0].xy.x = (uint16)(128 * i);
        spr->p[0].xy.y = 65529;
        spr->rgbc.r = 128;
        spr->rgbc.g = 128;
        spr->rgbc.b = 128;
        render_ot_insert(render_buffers.ot_addr + 4, p);
        render_insert_tex_page((sint32)(sint16)desc->tpage, 1);
    }
    p = render_buffers.packet_addr;
    render_buffers.packet_addr = p + 24;
    POLY_F4 *poly = psx_addr(p, sizeof(*poly));
    setPolyF4(poly);
    poly->rgbc.r = 0;
    poly->rgbc.g = 0;
    poly->rgbc.b = 0;
    poly->p[0].x = 0;
    poly->p[0].y = 0;
    poly->p[1].x = 320;
    poly->p[1].y = 0;
    poly->p[2].x = 0;
    poly->p[2].y = 256;
    poly->p[3].x = 320;
    poly->p[3].y = 256;
    render_ot_insert(render_buffers.ot_addr, p);
    pal = game_settings.language == GAME_LANGUAGE_ENGLISH ? &text_active_gradient : &text_inactive_gradient;
    text_draw_gradient(language_english, 24, 175, pal);
    pal = game_settings.language == GAME_LANGUAGE_FRENCH ? &text_active_gradient : &text_inactive_gradient;
    width = text_width(language_french);
    text_draw_gradient(language_french, 160 - (width >> 1), 175, pal);
    pal = game_settings.language == GAME_LANGUAGE_GERMAN ? &text_active_gradient : &text_inactive_gradient;
    text_draw_gradient(language_german, 220, 175, pal);
    return text_center(language_heading, 30);
}

GDB_CALL uint32 language_menu_run(void)
{
    FUNCTION_MARKER(0x80069b34u, "GAME.EXE");
    uint32 buttons, tick, selection;
    sint32 fade;
    DrawSync(0);
    resource_load(2, 0x116000, NULL, TEXTURE_ARCHIVE_CAPACITY, &tex_file, NULL);
    tex_load_grps(0);
    tex_upload_pals();
    disp_set_fade(-256, -256, -256);
    cd_music_start_track(24);
    game_session.tick = 0;
    for (;;)
    {
        tick = game_session.tick;
        if ((sint32)tick < 33)
            disp_fade_from_origin(tick << 3);
        language_menu_draw();
        text_center(language_hint, 214);
        menu_present();
        buttons = pad_menu_poll(0);
        if (game_session.player_count == 2)
            buttons |= pad_menu_poll(1);
        if (buttons & 128)
        {
            selection = game_settings.language;
            if ((sint32)selection > 0)
                game_settings.language = selection - 1;
        }
        if (buttons & 32)
        {
            selection = game_settings.language;
            if ((sint32)selection < 2)
                game_settings.language = selection + 1;
        }
        if (buttons & 0x4000)
            break;
        game_session.tick = game_session.tick + 1;
    }
    for (fade = 256; fade > 0; fade -= 8)
    {
        game_session.tick = game_session.tick + 1;
        disp_fade_from_origin(fade);
        language_menu_draw();
        menu_present();
    }
    return loading_clear();
}

static sint32 update_scene(uint32 retained_s3)
{
    FUNCTION_MARKER(0x800524b0u, "GAME.EXE");
    uint32 i, object, yaw, pitch, camera_x, camera_y, camera_z, tick, prev_car = 0;
    const OBJECT_CALLBACK *target;
    camera_update_transition();
    yaw = camera.angles.yaw_bits;
    pitch = camera.angles.pitch_bits;
    camera_x = camera.pos.x.bits;
    camera_y = camera.pos.y.bits;
    tick = game_session.tick;
    camera_z = camera.pos.z.bits;
    camera.frame_angles.yaw_bits = yaw;
    camera.frame_angles.pitch_bits = pitch;
    camera.frame_pos.x.bits = camera_x;
    camera.frame_pos.y.bits = camera_y;
    camera.frame_pos.z.bits = camera_z;
    game_session.tick = tick + 1;
    frame_prepare();
    stage_update();
    character_menu_advance_actors();
    character_menu_draw(character_menu.sel, character_menu.player_count);
    for (i = 0; (sint32)i < (sint32)world_objects.count; ++i)
    {
        object = 0x800b89f0 + 36 * i;
        /* TODO: Investigate menu dispatch beyond the native world pool */
        if (i >= WORLD_OBJECT_COUNT)
        {
            DIAGNOSTIC(fprintf(stderr, "Invalid menu object %u count %u in %s stage %u tick %u\n", i, world_objects.count, __FUNCTION__, game_session.stage, game_session.tick));
            diagnostic_stop(__FUNCTION__, __FILE__, __LINE__);
        }
        const MODEL_OBJECT *item = world_objects.object + i;
        uint32 callback_type = item->kind;
        /* TODO: Investigate a menu kind outside the native callback table */
        if (callback_type >= MODEL_TYPE_COUNT)
        {
            DIAGNOSTIC(fprintf(stderr, "Invalid menu kind %u slot %u in %s stage %u tick %u\n", callback_type, i, __FUNCTION__, game_session.stage, game_session.tick));
            diagnostic_stop(__FUNCTION__, __FILE__, __LINE__);
        }
        target = object_callbacks[OBJECT_CALLBACK_UPDATE][callback_type];
        if (target == &callback_helicopter_update)
        {
            /* Same callback-entry stack and S2 table as the reviewed car writer */
            helicopter_update_retained(world_objects.object + i, prev_car, 0x800bbf78);
            prev_car = 0;
        }
        else if (target == &callback_projectile_update_ballistic)
        {
            projectile_update_ballistic(world_objects.object + i, retained_s3);
            prev_car = 0;
        }
        else if (target)
        {
            object_call(target, world_objects.object + i, NULL);
            prev_car = target == &callback_scene_vehicle_update_traffic ? object : 0;
        }
    }
    world_objects_draw();
    dynamic_objects_draw();
    transform_identity();
    transform_pitch(camera.angles.pitch);
    transform_yaw(camera.angles.yaw);
    effects_draw();
    effects_update();
    return dynamic_objects_update();
}

static uint32 menu_dim_backdrop(void)
{
    const TEXTURE_DESCRIPTOR *desc;
    uint32 p, i;
    uint32 desc_idx = (uint32)tex_map[(uint32)tex_grps[TEXTURE_GROUP_TITLE_ALT].first_tex];
    if (desc_idx > TEXTURE_DESCRIPTOR_CAPACITY - 3)
        diagnostic_stop(__FUNCTION__, __FILE__, __LINE__);
    desc = tex_desc.item + desc_idx;
    for (i = 0; i < 3; ++i, desc++)
    {
        p = render_buffers.packet_addr;
        render_buffers.packet_addr = p + 20;
        SPRT *spr = psx_addr(p, sizeof(*spr));
        setSprt(spr);
        spr->p[0].uv.u = desc->uv0.u;
        spr->p[0].uv.v = desc->uv0.v;
        spr->w = i == 2 ? 64 : 128;
        spr->h = 255;
        spr->p[0].xy.x = (uint16)(i * 128);
        spr->p[0].xy.y = (uint16)-7;
        spr->rgbc.r = 32;
        spr->rgbc.g = 32;
        spr->rgbc.b = 32;
        render_ot_insert(render_buffers.ot_addr + 4, p);
        render_insert_tex_page((sint32)(sint16)desc->tpage, 1);
    }
    p = render_buffers.packet_addr;
    render_buffers.packet_addr = p + 24;
    POLY_F4 *poly = psx_addr(p, sizeof(*poly));
    setPolyF4(poly);
    poly->rgbc.code = poly->rgbc.code & 0xfd;
    poly->rgbc.r = 0;
    poly->rgbc.g = 0;
    poly->rgbc.b = 0;
    poly->p[0].x = 0;
    poly->p[0].y = 0;
    poly->p[1].x = 320;
    poly->p[1].y = 0;
    poly->p[2].x = 0;
    poly->p[2].y = 256;
    poly->p[3].x = 320;
    poly->p[3].y = 256;
    return render_ot_insert(render_buffers.ot_addr, p);
}

GDB_CALL uint32 menu_begin_dim_frame(void)
{
    FUNCTION_MARKER(0x8006a5b8u, "GAME.EXE");
    sint32 r, g, b;
    text_gradient_update();
    disp_begin_frame();
    r = disp_buffers.fade.r;
    g = disp_buffers.fade.g;
    b = disp_buffers.fade.b;
    if (r + g + b)
        disp_draw_fade();
    return menu_dim_backdrop();
}

sint32 menu_draw_dialog(uint32 retained_s3)
{
    FUNCTION_MARKER(0x80067e20u, "GAME.EXE");
    text_gradient_update();
    if (card_dialog.scene_backdrop)
        update_scene(retained_s3);
    else
    {
        disp_begin_frame();
        menu_dim_backdrop();
    }
    return text_center(card_text.title, 30);
}

sint32 menu_end_dialog(uint32 retained_s3)
{
    FUNCTION_MARKER(0x800680ccu, "GAME.EXE");
    uint32 i;
    text_hint = TEXT_HINT_NONE;
    for (i = 0; i < 2; ++i)
    {
        menu_draw_dialog(retained_s3);
        menu_present();
    }
    return 0;
}

GDB_CALL sint32 menu_question_update(void)
{
    FUNCTION_MARKER(0x800520acu, "GAME.EXE");
    static const uint32 color_offsets[4] = {0, 3, 9, 6};
    uint32 phase = menu.question_phase, offset, color, raw, edge, selection;
    sint32 width, count;
    int i, j;
    for (i = 0; i < 4; i++)
    {
        offset = i ? (phase + 512u * i) & 2047u : phase;
        color = (uint8)(sin_table[offset] >> 11);
        if (i == 3)
            menu.question_phase = (phase + 32u) & 2047u;
        for (j = 2; j >= 0; j--)
            menu.question_gradient.color[color_offsets[i] / 3].v[j] = (uint8)color;
    }
    width = text_width(menu_text.data);
    text_draw(menu_text.data, 160 - (width >> 1), 78);
    width = text_width(localization.text[LOCAL_TEXT_NO]);
    selection = menu.question_choice;
    text_draw_gradient(localization.text[LOCAL_TEXT_NO], 120 - (width >> 1), 108, selection ? &text_inactive_gradient : &menu.question_gradient);
    width = text_width(localization.text[LOCAL_TEXT_YES]);
    selection = menu.question_choice;
    text_draw_gradient(localization.text[LOCAL_TEXT_YES], 200 - (width >> 1), 108, selection == 1 ? &menu.question_gradient : &text_inactive_gradient);
    count = (sint32)game_session.player_count;
    if (count <= 0)
        return count;
    i = 0;
    do
    {
        raw = pad_decode_buttons(pad_game.port[i]);
        edge = (raw ^ pad_menu.prev[i]) & raw;
        pad_menu.pressed[i] = edge;
        pad_menu.prev[i] = raw;
        edge = pad_menu.pressed[i];
        pad_menu.last_raw = raw;
        if (edge & 0x4000u)
        {
            menu_play_confirm_sound();
            selection = menu.question_choice;
            menu.callback = selection ? menu.question_cancel : menu.question_accept;
        }
        if ((pad_menu.pressed[i] & 0x80u) && menu.question_choice)
            menu.question_choice = 0;
        if ((pad_menu.pressed[i] & 0x20u) && menu.question_choice != 1)
            menu.question_choice = 1;
        count = (sint32)game_session.player_count;
        i++;
    } while (i < count);
    return 0;
}

GDB_CALL sint32 menu_message_update(void)
{
    FUNCTION_MARKER(0x80051e50u, "GAME.EXE");
    uint32 phase = menu.message_phase, count, i, raw, pressed;
    uint8 color;
    sint32 width;
    color = sin_table[phase] >> 11;
    menu.message_gradient.color[0].b = color;
    menu.message_gradient.color[0].g = color;
    menu.message_gradient.color[0].r = color;
    color = sin_table[(phase + 512u) & 2047u] >> 11;
    menu.message_gradient.color[1].b = color;
    menu.message_gradient.color[1].g = color;
    menu.message_gradient.color[1].r = color;
    color = sin_table[(phase + 1024u) & 2047u] >> 11;
    menu.message_gradient.color[3].b = color;
    menu.message_gradient.color[3].g = color;
    menu.message_gradient.color[3].r = color;
    color = sin_table[(phase + 1536u) & 2047u] >> 11;
    menu.message_phase = (phase + 32u) & 2047u;
    menu.message_gradient.color[2].b = color;
    menu.message_gradient.color[2].g = color;
    menu.message_gradient.color[2].r = color;
    width = text_width(menu_text.data);
    text_draw(menu_text.data, 160u - (uint32)(width >> 1), 78);
    width = text_width(message_ok);
    text_draw_gradient(message_ok, 160u - (uint32)(width >> 1), 108, &menu.message_gradient);
    count = game_session.player_count;
    if ((sint32)count <= 0)
        return count;
    i = 0;
    do
    {
        raw = (uint32)pad_decode_buttons(pad_game.port[i]);
        uint32 *edge = &pad_menu.pressed[i];
        *edge = (raw ^ pad_menu.prev[i]) & raw;
        pad_menu.prev[i] = raw;
        pressed = *edge;
        pad_menu.last_raw = raw;
        if (pressed & 0x4000u)
        {
            menu_play_confirm_sound();
            menu.callback = menu.message_return;
        }
        count = game_session.player_count;
        i++;
    } while ((sint32)i < (sint32)count);
    return 0;
}

/* Original 4FEF4..4FF70 */
GDB_CALL void menu_prepare_frame(void)
{
    uint32 yaw, pitch, x, y, z, tick;
    hud_update();
    camera_update_transition();
    yaw = camera.angles.yaw_bits;
    pitch = camera.angles.pitch_bits;
    x = camera.pos.x.bits;
    y = camera.pos.y.bits;
    tick = game_session.tick;
    z = camera.pos.z.bits;
    camera.frame_angles.yaw_bits = (uint16)yaw;
    camera.frame_angles.pitch_bits = (uint16)pitch;
    camera.frame_pos.x.bits = x;
    camera.frame_pos.y.bits = y;
    camera.frame_pos.z.bits = z;
    game_session.tick = tick + 1;
    frame_prepare();
}

/* Original 4FF70..50238, ending before the caller's cleanup */
sint32 menu_update_selection_frame(uint32 *retained_s3)
{
    uint32 skip_scene, i, prev_car = 0;
    sint32 result;
    if (!character_menu.confirmed)
    {
        MENU_CALLBACK_REFERENCE ref = menu.callback;
        if (!ref)
            diagnostic_stop(__FUNCTION__, __FILE__, __LINE__);
        ref->update();
    }
    skip_scene = menu.skip_scene;
    if (!skip_scene)
    {
        stage_update();
        character_menu_advance_actors();
        character_menu_draw(character_menu.sel, character_menu.player_count);
        for (i = 0; (sint32)i < (sint32)world_objects.count; i++)
        {
            uint32 object = 0x800b89f0 + 36 * i;
            /* TODO: Investigate menu dispatch beyond the native world pool */
            if (i >= WORLD_OBJECT_COUNT)
            {
                DIAGNOSTIC(fprintf(stderr, "Invalid menu object %u count %u in %s stage %u tick %u\n", i, world_objects.count, __FUNCTION__, game_session.stage, game_session.tick));
                diagnostic_stop(__FUNCTION__, __FILE__, __LINE__);
            }
            const MODEL_OBJECT *item = world_objects.object + i;
            uint32 callback_type = item->kind;
            /* TODO: Investigate a menu kind outside the native callback table */
            if (callback_type >= MODEL_TYPE_COUNT)
            {
                DIAGNOSTIC(fprintf(stderr, "Invalid menu kind %u slot %u in %s stage %u tick %u\n", callback_type, i, __FUNCTION__, game_session.stage, game_session.tick));
                diagnostic_stop(__FUNCTION__, __FILE__, __LINE__);
            }
            const OBJECT_CALLBACK *target = object_callbacks[OBJECT_CALLBACK_UPDATE][callback_type];
            if (target == &callback_helicopter_update)
            {
                helicopter_update_retained(world_objects.object + i, prev_car, 0x800bbf78);
                prev_car = 0;
            }
            else if (target == &callback_projectile_update_ballistic)
            {
                projectile_update_ballistic(world_objects.object + i, *retained_s3);
                prev_car = 0;
            }
            else if (target)
            {
                object_call(target, world_objects.object + i, NULL);
                prev_car = target == &callback_scene_vehicle_update_traffic ? object : 0;
            }
        }
        world_objects_draw();
        dynamic_objects_draw();
        transform_identity();
        transform_pitch(camera.angles.pitch);
        transform_yaw(camera.angles.yaw);
        effects_draw();
        effects_update();
        dynamic_objects_update();
    }
    result = menu_finish_selection(skip_scene != 0);
    /* Preserve original S3 bits for later scene callback arguments */
    if (!skip_scene)
        *retained_s3 = 0x80092774;
    return result;
}

GDB_CALL sint32 menu_update_frame(void)
{
    uint32 retained_s3 = 0xffffffff;
    return menu_update_selection_frame(&retained_s3);
}

GDB_CALL sint32 menu_finish_selection(sint32 skip_scene)
{
    int i;
    uint32 count;
    if (skip_scene)
        menu.skip_scene = 0;
    else
    {
        for (i = 0; i < 2; i++)
        {
            uint32 raw = (uint32)pad_decode_buttons(pad_game.port[i]);
            if (raw & 65535)
                menu.last_activity[i] = game_session.tick;
            if ((raw & 0x8680) != 0x8680)
                menu.last_combo_break[i] = game_session.tick;
        }
        if (menu.loaded_game.ready)
        {
            demo.cur_idx = 0xffffffff;
            game_session.stage = menu.loaded_game.stage;
            game_session.player_count = menu.loaded_game.player_count;
            return 1;
        }
    }
    if (!character_menu.confirmed)
        return 0;
    count = character_menu.fade_tick;
    character_menu.fade_tick = count + 1;
    {
        sint32 phase = (sint32)(count - 15u), level;
        if (phase < 0)
            phase = 0;
        level = (sint32)(256u - ((uint32)phase << 3));
        if (level < 0)
            level = 0;
        disp_fade_from_origin(level);
    }
    if (character_menu.fade_tick != 48)
        return 0;
    count = character_menu.player_count;
    for (i = 0; i < (sint32)count; i++)
        actor_types[i] = (uint16)character_menu_types[character_menu.sel[i]];
    game_session.stage = (game_settings.flags & GAME_FLAG_EXTENDED_OPTIONS) ? game_settings.start_stage : 0;
    demo.cur_idx = 0xffffffff;
    game_session.player_count = character_menu.player_count;
    return 1;
}

/* Normalized twin pair 8004F468/8004F490: same effect dispatch, adapted ID/volume.
 * The underlying GAME:80056670 sound allocator is a required WIP service. */
sint32 menu_play_effect(sint32 id, sint32 volume)
{
    if (!game_services.play_effect)
        diagnostic_stop(__FUNCTION__, __FILE__, __LINE__);
    return game_services.play_effect(id, volume, 0x1f00);
}

GDB_CALL sint32 menu_play_move_sound(void)
{
    FUNCTION_MARKER(0x8004f468u, "GAME.EXE");
    return menu_play_effect(3, 255);
}

GDB_CALL sint32 menu_play_confirm_sound(void)
{
    FUNCTION_MARKER(0x8004f490u, "GAME.EXE");
    return menu_play_effect(4, 127);
}

GDB_CALL sint32 menu_draw_primary_pad_notice(sint32 y)
{
    FUNCTION_MARKER(0x8004f4f8u, "GAME.EXE");
    return text_center(localization.text[LOCAL_TEXT_CONTROLLER1_REMOVED], y);
}

/* Draw the main menu before processing edge-triggered input */
GDB_CALL sint32 main_menu_update(void)
{
    FUNCTION_MARKER(0x80050270, "GAME.EXE");
    uint32 phase = menu.color_phase, mode, frame, menu_count, other_count, stamp;
    uint32 *counts;
    int i, k;
    sint32 result;
    const uint32 phases[4] = {0, 512, 1536, 1024};
    const uint32 order[4] = {0, 1, 3, 2};
    for (i = 0; i < 4; i++)
    {
        uint32 slot = order[i], sample = slot ? ((phase + phases[slot]) & 2047) : phase;
        uint8 color = (uint8)(sin_table[sample] >> 11);
        if (i == 3)
        {
            character_menu.player_count = 0;
            menu.color_phase = (phase + 32) & 2047;
        }
        for (k = 2; k >= 0; k--)
            menu.main_gradient.color[slot].v[k] = color;
    }
    pad_refresh_ports();
    menu_count = pad_decode.count;
    game_session.player_count = menu_count;
    if (!menu_count)
        return menu_draw_primary_pad_notice(98);
    text_draw_logo(96, 16);
    text_center(localization.hint[TEXT_HINT_CHOOSE_SELECT_BACK], 214);
    mode = game_session.player_count != 1;
    menu_count = menu_defaults.main_row_count[0];
    other_count = menu_defaults.main_row_count[1];
    counts = menu.main_row_count + mode;
    menu.main_row_count[0] = menu_count;
    menu.main_row_count[1] = other_count;
    menu_count = *counts;
    if ((sint32)menu_count > 0)
    {
        uint32 y = 138 - 10 * menu_count;
        i = 0;
        do
        {
            const char *text;
            const TEXT_GRADIENT *color;
            if (mode >= MAIN_MENU_MODE_COUNT || i < 0 || i >= LOCALIZED_MAIN_ROW_COUNT)
                diagnostic_stop(__FUNCTION__, __FILE__, __LINE__);
            text = localization.main_menu[mode][i];
            sint32 x = (sint32)(160u - (uint32)(text_width(text) >> 1));
            color = menu.sel_row == (uint32)i ? &menu.main_gradient : &text_inactive_gradient;
            text = localization.main_menu[mode][i];
            text_draw_gradient(text, x, y, color);
            y += 20;
            menu_count = *counts;
            i++;
        } while (i < (sint32)menu_count);
    }
    stamp = menu.last_combo_break[0];
    frame = game_session.tick;
    if ((sint32)(stamp + 60) < (sint32)frame || (sint32)(menu.last_combo_break[1] + 60u) < (sint32)frame)
    {
        uint32 flags = game_settings.flags;
        menu.last_combo_break[1] = frame;
        menu.last_combo_break[0] = frame;
        game_settings.flags = flags ^ GAME_FLAG_EXTENDED_OPTIONS;
    }
    if (game_settings.flags & GAME_FLAG_EXTENDED_OPTIONS)
        text_draw(cheat_text, 160u - (uint32)(text_width(cheat_text) >> 1), 200);
    result = (sint32)game_session.player_count;
    if (result <= 0)
        return result;
    i = 0;
    do
    {
        uint32 raw = (uint32)pad_decode_buttons(pad_game.port[i]);
        uint32 *edge = &pad_menu.pressed[i];
        uint32 buttons, selection;
        *edge = (raw ^ pad_menu.prev[i]) & raw;
        pad_menu.prev[i] = raw;
        buttons = *edge;
        pad_menu.last_raw = raw;
        if (buttons & 0x10)
        {
            menu_play_move_sound();
            selection = menu.sel_row - 1;
            menu.sel_row = selection;
            if ((sint32)selection < 0)
                menu.sel_row = *counts - 1;
        }
        if (*edge & 0x40)
        {
            menu_play_move_sound();
            selection = menu.sel_row;
            menu_count = *counts;
            selection++;
            menu.sel_row = selection;
            if ((sint32)(menu_count - 1) < (sint32)selection)
                menu.sel_row = 0;
        }
        buttons = *edge;
        if (buttons & 0x1000)
        {
            if (!game_services.return_to_frontend)
                diagnostic_stop(__FUNCTION__, __FILE__, __LINE__);
            game_services.return_to_frontend();
            diagnostic_stop(__FUNCTION__, __FILE__, __LINE__);
        }
        if (buttons & 0x4000)
        {
            uint32 start = 0, options = 0;
            selection = menu.sel_row;
            if (!selection && !i)
                start = 1;
            else
            {
                if (mode)
                {
                    selection = menu.sel_row;
                    if (selection == 1)
                        start = 2;
                }
                if (!start)
                {
                    if (!mode)
                    {
                        selection = menu.sel_row;
                        if (selection == 1)
                            options = 1;
                    }
                    if (!options && mode)
                    {
                        selection = menu.sel_row;
                        if (selection == 2)
                            options = 1;
                    }
                }
            }
            if (start)
            {
                menu_play_confirm_sound();
                character_menu.player_count = start;
                game_session.mode = start - 1;
                menu.callback = &menu_callback_character;
                character_menu.ready_mask = 0;
                camera_orbit();
            }
            else if (options)
            {
                FIXED_VEC3 pos;
                for (k = 0; k < 3; k++)
                    pos.bits[k] = camera_options_dst.bits[k];
                menu_play_confirm_sound();
                menu.callback = &menu_callback_options;
                camera_capture(pos.bits, 0, 0x4000);
            }
        }
        menu_count = game_session.player_count;
        i++;
        result = i < (sint32)menu_count;
    } while (result);
    return result;
}

uint32 menu_present(void)
{
    FUNCTION_MARKER(0x80067358, "GAME.EXE");
    sint32 idx = text_hint;
    uint32 result;
    if (idx != TEXT_HINT_NONE)
    {
        if (idx < 0 || idx >= TEXT_HINT_COUNT)
            diagnostic_stop(__FUNCTION__, __FILE__, __LINE__);
        text_center(localization.hint[idx], 216);
    }
    DrawOTag((uint32 *)psx_addr(render_buffers.ot_addr, sizeof(uint32)));
    if (!loading.active)
        gpu_present();
    result = disp_buffers.frame + 1;
    disp_buffers.frame = result;
    if (game_services.frontend_frame)
        game_services.frontend_frame();
    return result;
}

uint32 menu_present_dim(void)
{
    FUNCTION_MARKER(0x80063e50u, "GAME.EXE");
    uint32 p;
    POLY_F4 *poly;
    disp_begin_frame();
    p = render_buffers.packet_addr;
    render_buffers.packet_addr = p + 24;
    poly = psx_addr(p, sizeof(*poly));
    setPolyF4(poly);
    setSemiTrans(poly, 1);
    poly->rgbc.r = 0;
    poly->rgbc.g = 0;
    poly->rgbc.b = 0;
    poly->p[0].x = 0;
    poly->p[0].y = 0;
    poly->p[1].x = 320;
    poly->p[1].y = 0;
    poly->p[2].x = 0;
    poly->p[2].y = 256;
    poly->p[3].x = 320;
    poly->p[3].y = 256;
    render_ot_insert(render_buffers.ot_addr, p);
    return menu_present();
}

uint32 menu_draw_title_backdrop(void)
{
    FUNCTION_MARKER(0x80069cc8u, "GAME.EXE");
    const TEXTURE_DESCRIPTOR *desc;
    uint32 i, p;
    sint32 color_sum;
    SPRT *spr;
    POLY_F4 *poly;
    disp_begin_frame();
    color_sum = disp_buffers.fade.r;
    color_sum += disp_buffers.fade.g;
    color_sum += disp_buffers.fade.b;
    if (color_sum)
        disp_draw_fade();
    uint32 desc_idx = (uint32)tex_map[(uint32)tex_grps[TEXTURE_GROUP_TITLE].first_tex];
    if (desc_idx > TEXTURE_DESCRIPTOR_CAPACITY - 3)
        diagnostic_stop(__FUNCTION__, __FILE__, __LINE__);
    desc = tex_desc.item + desc_idx;
    for (i = 0; i < 3; i++, desc++)
    {
        p = render_buffers.packet_addr;
        render_buffers.packet_addr = p + 20;
        spr = psx_addr(p, sizeof(*spr));
        setSprt(spr);
        spr->p[0].uv.u = desc->uv0.u;
        spr->p[0].uv.v = desc->uv0.v;
        spr->w = i == 2 ? 64 : 128;
        spr->h = 255;
        spr->p[0].xy.x = 128 * i;
        spr->p[0].xy.y = 65529;
        spr->rgbc.r = 128;
        spr->rgbc.g = 128;
        spr->rgbc.b = 128;
        render_ot_insert(render_buffers.ot_addr + 4, p);
        render_insert_tex_page((sint16)desc->tpage, 1);
    }
    /* Opaque background is linked last in bucket 0; submission is the caller's job. */
    p = render_buffers.packet_addr;
    render_buffers.packet_addr = p + 24;
    poly = psx_addr(p, sizeof(*poly));
    setPolyF4(poly);
    setSemiTrans(poly, 0);
    poly->rgbc.r = 0;
    poly->rgbc.g = 0;
    poly->rgbc.b = 0;
    poly->p[0].x = 0;
    poly->p[0].y = 0;
    poly->p[1].x = 320;
    poly->p[1].y = 0;
    poly->p[2].x = 0;
    poly->p[2].y = 256;
    poly->p[3].x = 320;
    poly->p[3].y = 256;
    return render_ot_insert(render_buffers.ot_addr, p);
}

uint32 title_menu_run(void)
{
    FUNCTION_MARKER(0x80069f78, "GAME.EXE");
    uint32 edge, tick;
    sint32 level;
    disp_buffers.flash_ticks = 0;
    disp_buffers.shake_ticks = 0;
    disp_apply_offsets();
    DrawSync(0);
    resource_load(170, 0x116000, NULL, TEXTURE_ARCHIVE_CAPACITY, &tex_file, NULL);
    tex_load_grps(0);
    tex_upload_pals();
    disp_set_fade(-256, -256, -256);
    cd_music_start_track(24);
    pad_menu.prev[0] = 0xffffffff;
    game_session.tick = 0;
    for (;;)
    {
        tick = game_session.tick;
        if ((sint32)tick < 17)
            disp_fade_from_origin(tick << 4);
        menu_draw_title_backdrop();
        if (game_session.tick & 16)
            text_center(localization.text[LOCAL_TEXT_PRESS_START], 170);
        menu_present();
        edge = pad_menu_poll(0);
        if (game_session.player_count == 2)
            edge |= pad_menu_poll(1);
        if (edge & 8)
            break;
        tick = game_session.tick;
        if (tick == 600)
            break;
        game_session.tick = tick + 1;
    }
    for (level = 256; level > 0; level -= 16)
    {
        disp_fade_from_origin(level);
        menu_draw_title_backdrop();
        menu_present();
    }
    loading_clear();
    return game_session.tick == 600;
}

/* 4FBCC..4FEF4: return 0 at the first frame prefix, 1 on title timeout. */
uint32 menu_initialize(void)
{
    const RESOURCE_LOADER *tex;
    uint32 slot, yaw, stage;
    sint32 idx, s, type;
    sint32 angle = -192;
    game_settings.flags = 0;
    memset(actor_animations, 0, sizeof(actor_animations));
    memset(actor_positions, 0, sizeof(actor_positions));
    memset(actor_orientation, 0, sizeof(actor_orientation));
    memset(actor_health, 0, sizeof(actor_health));
    memset(actor_types, 0, sizeof(actor_types));
    memset(actor_movement, 0, sizeof(actor_movement));
    memset(actor_contacts, 0, sizeof(actor_contacts));
    memset(actor_items, 0, sizeof(actor_items));
    memset(item_pickup.pending, 0, sizeof(item_pickup.pending));
    memset(actor_lifecycle, 0, sizeof(actor_lifecycle));
    memset(actor_bonus_counts, 0, sizeof(actor_bonus_counts));
    memset(actor_bonus_totals, 0, sizeof(actor_bonus_totals));
    memset(actor_companions, 0, sizeof(actor_companions));
    memset(actor_pair_motion, 0, sizeof(actor_pair_motion));
    memset(actor_trails, 0, sizeof(actor_trails));
    memset(actor_combat, 0, sizeof(actor_combat));
    memset(actor_status, 0, sizeof(actor_status));
    memset(actor_lighting, 0, sizeof(actor_lighting));
    memset(col_regions.actor, 0, sizeof(col_regions.actor));
    memset(character_actor_names, 0, sizeof(character_actor_names));
    memset(npc_behavior_actors, 0, sizeof(npc_behavior_actors));
    memset(npc_targets, 0, sizeof(npc_targets));
    memset(npc_action_actors, 0, sizeof(npc_action_actors));
    actors_reset_health();
    level_transition.continues = 0;
    pad_refresh_ports();
    if (title_menu_run())
        return 1;
    loading_begin(64);
    level_reset();
    character_resources_begin();
    actors_reset();
    game_session.stage = 26;
    resource_load(31, 0x116000, NULL, TEXTURE_ARCHIVE_CAPACITY, &tex_file, NULL);
    stage = game_session.stage;
    if (stage >= RESOURCE_STAGE_COUNT)
        diagnostic_stop(__FUNCTION__, __FILE__, __LINE__);
    tex = stage_resource_loaders[STAGE_RESOURCE_TEX][stage];
    world_object_active_types = world_object_type_grps[26];
    resource_loader_call(RESOURCE_LOADER_TEXTURE, tex);
    tex_load_grps(1);
    sound_samples_load(197);
    actor_types[0] = 3;
    actor_types[2] = 1;
    actor_types[1] = 0;
    actor_types[3] = 2;
    resource_load_frontend_models();
    character_menu.player_count = game_session.player_count;
    /* The original address range contains exactly four menu actors */
    for (slot = 0; slot < CHARACTER_MENU_ACTOR_COUNT; slot++, angle += 128)
    {
        ACTOR_ANIMATION *playback = actor_animations + slot;
        FIXED_VEC3 *pos = actor_positions + slot;
        ACTOR_ORIENTATION *orientation = actor_orientation + slot;
        ACTOR_MOVEMENT *movement = actor_movement + slot;
        yaw = (uint32)angle & 2047;
        orientation->yaw = (uint16)yaw;
        yaw = (uint16)orientation->yaw;
        idx = orientation->yaw;
        actor_contacts[slot].captive = ACTOR_SLOT_NONE;
        orientation->roll_rate = 0;
        orientation->roll = 0;
        orientation->yaw_rate = 0;
        movement->desired_yaw = (uint16)yaw;
        s = sin_table[idx];
        idx = orientation->yaw;
        pos->x.bits = (uint32)s << 10;
        pos->z.bits = 0xf3000000 - ((uint32)(sint32)sin_table[((uint32)idx + 512) & 2047] << 10);
        playback->src = (ACTOR_ANIMATION_SOURCE){ACTOR_ANIMATION_SOURCE_NONE, {0}};
        type = actor_types[slot];
        playback->frame = 0xffffffff;
        playback->selector = 0;
        playback->id = 69;
        animation_start(slot, animation_type_banks[(uint32)type], 69);
    }
    pad_refresh_ports();
    pad_action_clear(0);
    pad_action_clear(1);
    depth_table_build();
    disp_prepare_scene();
    camera.angles.pitch_bits = 0xd100;
    camera.pos.y.bits = 0xfef00000;
    camera.target_angles.pitch_bits = 0;
    camera.target_angles.yaw_bits = 0;
    camera.velocity.z.bits = 0;
    camera.velocity.y.bits = 0;
    camera.velocity.x.bits = 0;
    camera.angles.yaw_bits = 0;
    camera.pos.x.bits = 0;
    camera.pos.z.bits = 0xef800000;
    camera_capture(camera.pos.bits, 0, -12032);
    cd_music_start_track(24);
    character_menu.confirmed = 0;
    character_menu.fade_tick = 0;
    pad_decode.reset_blocked = 0;
    menu.ambient_voice[0] = sound_voice_play(0, 0, 7936);
    menu.ambient_voice[1] = sound_voice_play(1, 0, 7936);
    pad_menu.prev[1] = 0xffffffff;
    pad_menu.prev[0] = 0xffffffff;
    menu.callback = &menu_callback_main;
    menu.last_activity[1] = 0;
    menu.last_activity[0] = 0;
    menu.last_combo_break[1] = 0;
    menu.last_combo_break[0] = 0;
    menu.loaded_game.ready = 0;
    menu.sel_row = 0;
    character_menu.ready_mask = 0;
    menu.skip_scene = 0;
    return 0;
}

GDB_CALL uint32 menu_run(void)
{
    uint32 timeout, retained_s3 = 0xffffffff;
    FUNCTION_MARKER(0x8004fbcc, "GAME.EXE");
    timeout = menu_initialize();
    game_services.frontend_frame = NULL;
    if (timeout)
        return 1;
    do
    {
        menu_prepare_frame();
    } while (!menu_update_selection_frame(&retained_s3));
    pad_stream_reset();
    sound_voices_stop_all();
    return 0;
}

/* Keep the original forward copy and retained bytes after the new terminator */
static void copy_text(const char *src, uint32 src_capacity)
{
    const char *end;
    uint32 count, i = 0;
    uint8 byte;
    if (!src)
        return;
    if (src_capacity > 0x7fffffff)
        diagnostic_stop(__FUNCTION__, __FILE__, __LINE__);
    end = memchr(src, 0, src_capacity);
    if (!end)
        diagnostic_stop(__FUNCTION__, __FILE__, __LINE__);
    count = end - src + 1;
    if (count > menu_text.capacity)
    {
        char *data = realloc(menu_text.data, count);
        if (!data)
            diagnostic_stop(__FUNCTION__, __FILE__, __LINE__);
        menu_text.data = data;
        menu_text.capacity = count;
    }
    do
    {
        byte = (uint8)src[i];
        menu_text.data[i++] = byte;
    } while (byte);
}

GDB_CALL uint32 menu_message_open(const char *message, uint32 message_capacity, MENU_CALLBACK_REFERENCE callback)
{
    FUNCTION_MARKER(0x80051e14u, "GAME.EXE");
    menu.message_return = callback;
    copy_text(message, message_capacity);
    menu.callback = &callback_message;
    /* Preserve original V0 bits, not a host callback pointer */
    return 0x80051e50;
}

GDB_CALL uint32 menu_question_open(const char *message, uint32 message_capacity, uint32 kind, MENU_CALLBACK_REFERENCE accept, MENU_CALLBACK_REFERENCE cancel)
{
    FUNCTION_MARKER(0x8005205cu, "GAME.EXE");
    menu.question_accept = accept;
    menu.question_cancel = cancel;
    copy_text(message, message_capacity);
    menu.question_choice = (uint8)kind;
    menu.callback = &callback_question;
    /* Preserve original V0 bits, not a host callback pointer */
    return 0x800520ac;
}

GDB_CALL uint32 menu_message_why_not(void)
{
    FUNCTION_MARKER(0x800523c8u, "GAME.EXE");
    return menu_message_open(message_why_not, sizeof(message_why_not), &menu_callback_options);
}

GDB_CALL uint32 menu_message_cannot(void)
{
    FUNCTION_MARKER(0x800523f8u, "GAME.EXE");
    return menu_message_open(message_cannot, sizeof(message_cannot), &menu_callback_options);
}
