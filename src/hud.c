#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "game.h"
#include "sound_device.h"

static const char continue_prompt_format[] = "]%s";
static const char ammo_text[] = "AMMO";
static const char destruction_format[] = "%d x 250 = %d";
static const char game_over_text[] = " GAME  OVER";

HUD hud;

/* Five original signed quotient phases; the eighth byte is retained */
GDB_CALL sint32 hud_format_score(sint32 player)
{
    FUNCTION_MARKER(0x8005e9e8, "GAME.EXE");
    uint32 started = 0;
    sint32 val, q;
    uint8 digit;
    if ((uint32)player >= HUD_PLAYER_COUNT)
        return 0;
    val = (sint32)player_scores[player].val;
    q = val / 100000;
    digit = (uint32)q + 48;
    if (!started && digit == 48)
        digit = 32;
    else
        started = 1;
    hud.score_text[player][0] = digit;
    val = (uint32)val - (uint32)q * 100000u;
    q = val / 10000;
    digit = (uint32)q + 48;
    if (!started && digit == 48)
        digit = 32;
    else
        started = 1;
    hud.score_text[player][1] = digit;
    val = (uint32)val - (uint32)q * 10000u;
    q = val / 1000;
    digit = (uint32)q + 48;
    if (!started && digit == 48)
        digit = 32;
    else
        started = 1;
    hud.score_text[player][2] = digit;
    val = (uint32)val - (uint32)q * 1000u;
    q = val / 100;
    digit = (uint32)q + 48;
    if (!started && digit == 48)
        digit = 32;
    else
        started = 1;
    hud.score_text[player][3] = digit;
    val = (uint32)val - (uint32)q * 100u;
    q = val / 10;
    digit = (uint32)q + 48;
    if (!started && digit == 48)
        digit = 32;
    hud.score_text[player][4] = digit;
    hud.score_text[player][5] = 48;
    hud.score_text[player][6] = 0;
    return 48;
}

/* Original 61E6C keeps its formatted continue prompt on the native stack */
GDB_CALL uint32 hud_continue_prompt(uint32 player)
{
    uint32 partner;
    const char *name;
    sint32 x;
    char text[40];
    FUNCTION_MARKER(0x80061e6c, "GAME.EXE");
    if (game_session.player_count == 1)
        return 0;
    /* TODO: Investigate a continue prompt outside the native actor pool */
    if (player >= ACTOR_COUNT)
    {
        DIAGNOSTIC(fprintf(stderr, "Invalid continue actor %u players %u stage %u tick %u\n", player, game_session.player_count, game_session.stage, game_session.tick));
        diagnostic_stop(__FUNCTION__, __FILE__, __LINE__);
    }
    if (!((sint8)actor_lifecycle[player].death_ticks))
        return 0;
    /* TODO: Investigate a continue prompt whose partner is not a player */
    if (player >= PLAYER_COUNT)
    {
        DIAGNOSTIC(fprintf(stderr, "Invalid continue partner for actor %u death ticks %u players %u stage %u tick %u\n", player, (uint32)actor_lifecycle[player].death_ticks, game_session.player_count, game_session.stage, game_session.tick));
        diagnostic_stop(__FUNCTION__, __FILE__, __LINE__);
    }
    partner = 1 - player;
    if (actor_health[partner].hp <= 0 || (sint32)level_transition.continues <= 0)
        return 0;
    if (player >= CHARACTER_NAME_ACTOR_COUNT)
        diagnostic_stop(__FUNCTION__, __FILE__, __LINE__);
    name = character_name_text(character_actor_names[player]);
    if (player)
    {
        x = 314 - (uint32)text_width(name);
        text_draw(name, x, 16);
        x = (uint32)x - 10 - (uint32)text_width(hud.score_text[1]);
        text_draw_digits(hud.score_text[1], HUD_SCORE_TEXT_CAPACITY, x, 16);
    }
    else
    {
        x = text_draw(name, 16, 16);
        text_draw_digits(hud.score_text[0], HUD_SCORE_TEXT_CAPACITY, x, 16);
    }
    sprintf(text, continue_prompt_format, localization.text[LOCAL_TEXT_CONTINUE]);
    x = player ? (sint32)(314 - (uint32)text_width(text)) : 16;
    if (game_session.tick & 8)
        text_draw(text, x, 26);
    if (!(pad_decode_buttons(pad_game.port[player]) & 0x4000))
        return 0;
    level_transition.continues = level_transition.continues - 1;
    actor_health[player].hp = 380;
    actor_lifecycle[player].death_ticks = 0;
    if (player >= PLAYER_COUNT)
        diagnostic_stop(__FUNCTION__, __FILE__, __LINE__);
    player_scores[player].val = 0;
    player_scores[player].continue_threshold = 50000;
    hud_format_score(player);
    sound_play_chord();
    actor_positions[player].x.bits = actor_positions[partner].x.bits;
    actor_positions[player].z.bits = actor_positions[partner].z.bits;
    actor_col.pass_count = 0;
    actors_resolve_overlaps(player);
    return 0;
}

static void hud_ammo(uint32 player, ACTOR_ITEM_HAND hand)
{
    /* TODO: Investigate an ammo display outside the native player or hand domain */
    if (player >= PLAYER_COUNT || hand < 0 || hand >= ACTOR_ITEM_HAND_COUNT)
    {
        DIAGNOSTIC(fprintf(stderr, "Invalid ammo display player %u hand %d stage %u tick %u\n", player, (sint32)hand, game_session.stage, game_session.tick));
        diagnostic_stop(__FUNCTION__, __FILE__, __LINE__);
    }
    sint32 idx = actor_items[player].slot[hand];
    uint32 type;
    int show;
    sint32 x;
    sint32 ammo;
    char text[3];
    if (idx == -1)
        return;
    /* TODO: Investigate an ammo item outside the native dynamic pool */
    if (idx < 0 || idx >= DYNAMIC_OBJECT_COUNT)
    {
        DIAGNOSTIC(fprintf(stderr, "Invalid ammo item player %u hand %d slot %d stage %u tick %u\n", player, (sint32)hand, idx, game_session.stage, game_session.tick));
        diagnostic_stop(__FUNCTION__, __FILE__, __LINE__);
    }
    const MODEL_OBJECT *model = dynamic_objects + idx;
    type = model->kind;
    show = hand == ACTOR_ITEM_LEFT ? (type == 515 || type == 504) : (type == 515 || type == 506 || (!player && type == 505) || type - 499u < 5u);
    if (!show)
        return;
    /* TODO: Investigate a displayed weapon with a foreign native owner */
    if (model->payload_kind != MODEL_PAYLOAD_PHYSICS)
    {
        DIAGNOSTIC(fprintf(stderr, "Invalid ammo item owner player %u hand %d slot %d kind %u owner %u stage %u tick %u\n", player, (sint32)hand, idx, type, (uint32)model->payload_kind, game_session.stage, game_session.tick));
        diagnostic_stop(__FUNCTION__, __FILE__, __LINE__);
    }
    /* Original 61E0C decimal bytes in the caller's native local buffer */
    ammo = model->physics.life;
    text_format_digits(ammo, text);
    x = text_draw(ammo_text, player ? 224 : 16, 200);
    text_draw(text, x, 200);
}

/* Left health bar with a textured border */
GDB_CALL uint32 hud_draw_health_left(uint32 xy, uint32 idx)
{
    FUNCTION_MARKER(0x80063100, "GAME.EXE");
    const ACTOR_HEALTH *health = actor_health + idx;
    sint32 maximum = health->max >> 2, old = health->trail >> 2;
    sint32 shown = health->shown >> 2, hp = health->hp >> 2;
    uint32 desc_idx = (uint32)tex_map[(59 + ((uint32)tex_grps[TEXTURE_GROUP_GAME_FONT].first_tex))];
    if (desc_idx >= TEXTURE_DESCRIPTOR_CAPACITY)
        diagnostic_stop(__FUNCTION__, __FILE__, __LINE__);
    const TEXTURE_DESCRIPTOR *d = tex_desc.item + desc_idx;
    uint32 x = xy & 65535, y = (uint32)((sint32)xy >> 16), p, bucket, prev, i;
    int wide = maximum > 128, special;
    POLY_G4 *poly;
    POLY_F4 *flash;
    POLY_FT4 *border;
    uint32 *ot;
    sint32 delta, fade;
    if (wide)
    {
        maximum >>= 1;
        old >>= 1;
        shown >>= 1;
        hp >>= 1;
    }
    p = render_buffers.packet_addr;
    render_buffers.packet_addr = p + sizeof(*poly);
    poly = psx_addr(p, sizeof(*poly));
    setPolyG4(poly);
    special = actor_types[idx] == 53;
    for (i = 0; i < 4; i++)
    {
        poly->p[i].rgbc.r = i < 2 ? (wide ? 0 : 128) : 255;
        poly->p[i].rgbc.g = i < 2 ? (wide ? 128 : 0) : (special ? 128 : 255);
        poly->p[i].rgbc.b = i < 2 ? (special ? 128 : 0) : (special ? 255 : 128);
        poly->p[i].xy.x = x + (i < 2 ? 0 : (uint32)shown);
        poly->p[i].xy.y = y + 2 + ((i & 1) ? 4 : 0);
    }
    render_ot_insert(render_buffers.ot_addr + 8188, p);
    delta = old - hp;
    if (delta < 0)
        delta = -delta;
    if (delta > 0)
    {
        fade = 255 * (shown - hp);
        if (fade < 0)
            fade = -fade;
        fade /= delta;
        p = render_buffers.packet_addr;
        render_buffers.packet_addr = p + sizeof(*flash);
        flash = psx_addr(p, sizeof(*flash));
        setPolyF4(flash);
        setRGB0(flash, fade, 0, 0);
        for (i = 0; i < 4; i++)
        {
            flash->p[i].x = x + (uint32)(i < 2 ? shown : old);
            flash->p[i].y = y + 2 + ((i & 1) ? 4 : 0);
        }
        render_ot_insert(render_buffers.ot_addr + 8188, p);
    }
    p = render_buffers.packet_addr;
    render_buffers.packet_addr = p + sizeof(*border);
    border = psx_addr(p, sizeof(*border));
    border->rgbc.rgbc = 0x2d808080;
    border->p[0].uv_attr = d->words[0] & ~1u;
    border->p[1].uv_attr = d->words[1];
    border->p[2].uv = d->uv2;
    border->p[3].uv = d->uv3;
    for (i = 0; i < 4; i++)
    {
        border->p[i].xy.x = x + ((i & 1) ? (uint32)maximum : 0);
        border->p[i].xy.y = y + (i < 2 ? 0 : 11);
    }
    /* Preserve the full OT link and OR its existing tag bits with length */
    bucket = render_buffers.ot_addr + 8188;
    ot = psx_addr(bucket, sizeof(*ot));
    prev = *ot;
    *ot = p;
    border->tag = prev | 0x09000000;
    return prev;
}

/* Right health bar with a segmented sprite border */
GDB_CALL uint32 hud_draw_health_right(uint32 xy, uint32 idx)
{
    FUNCTION_MARKER(0x8006351c, "GAME.EXE");
    const ACTOR_HEALTH *health = actor_health + idx;
    sint32 maximum = health->max >> 2, old = health->trail >> 2;
    sint32 shown = health->shown >> 2, hp = health->hp >> 2;
    uint32 x = xy & 65535, y = (uint32)((sint32)xy >> 16), p, result, i;
    PROJECTION_POINT pos = {.v = {(sint32)x, (sint32)y, 0, 0}};
    int wide = maximum > 128, special;
    POLY_G4 *poly;
    POLY_F4 *flash;
    sint32 delta, fade;
    if (wide)
    {
        maximum >>= 1;
        old >>= 1;
        shown >>= 1;
        hp >>= 1;
    }
    p = render_buffers.packet_addr;
    render_buffers.packet_addr = p + sizeof(*poly);
    poly = psx_addr(p, sizeof(*poly));
    setPolyG4(poly);
    special = actor_types[idx] == 53;
    for (i = 0; i < 4; i++)
    {
        poly->p[i].rgbc.r = i < 2 ? (wide ? 0 : 128) : 255;
        poly->p[i].rgbc.g = i < 2 ? (wide ? 128 : 0) : (special ? 128 : 255);
        poly->p[i].rgbc.b = i < 2 ? (special ? 128 : 0) : (special ? 255 : 128);
    }
    for (i = 0; i < 4; i++)
    {
        poly->p[i].xy.x = x - (i < 2 ? 0 : (uint32)shown);
        poly->p[i].xy.y = y + 2 + ((i & 1) ? 4 : 0);
    }
    result = render_ot_insert(render_buffers.ot_addr + 8188, p);
    delta = old - hp;
    if (delta < 0)
        delta = -delta;
    if (delta > 0)
    {
        fade = 255 * (shown - hp);
        if (fade < 0)
            fade = -fade;
        fade /= delta;
        p = render_buffers.packet_addr;
        render_buffers.packet_addr = p + sizeof(*flash);
        flash = psx_addr(p, sizeof(*flash));
        setPolyF4(flash);
        setRGB0(flash, fade, 0, 0);
        for (i = 0; i < 4; i++)
        {
            flash->p[i].x = x - (uint32)(i < 2 ? shown : old);
            flash->p[i].y = y + 2 + ((i & 1) ? 4 : 0);
        }
        result = render_ot_insert(render_buffers.ot_addr + 8188, p);
    }
    for (i = maximum >> 3; (sint32)i > 0; i--)
    {
        pos.x = (sint32)((uint32)pos.x - 8u);
        result = spr_draw(&pos, tex_map[(59 + ((uint32)tex_grps[TEXTURE_GROUP_GAME_FONT].first_tex))]);
    }
    return result;
}

/* Original 620BC player HUD, branch choices and completion display */
GDB_CALL uint32 hud_draw(void)
{
    uint32 i;
    const char *name;
    sint32 hp, shown, mode, x, idx;
    FUNCTION_MARKER(0x800620bc, "GAME.EXE");
    for (i = 0; i < 8; i++)
    {
        ACTOR_HEALTH *health = actor_health + i;
        if (actor_types[i] == ACTOR_TYPE_NONE)
            continue;
        hp = health->hp;
        shown = health->shown;
        if (hp < shown)
        {
            shown = (sint16)(shown - 4);
            health->shown = (uint16)shown;
            if (hp >= shown)
            {
                health->shown = (uint16)hp;
                health->trail = (uint16)hp;
            }
        }
        else if (shown < hp)
        {
            shown = (sint16)(shown + 4);
            health->shown = (uint16)shown;
            health->trail = (uint16)hp;
            if (shown >= hp)
                health->shown = (uint16)hp;
        }
    }
    for (i = 0; i < 2; i++)
    {
        ACTOR_HEALTH *health = actor_health + i;
        ACTOR_CONTACT *contact = actor_contacts + i;
        if (actor_types[i] == ACTOR_TYPE_NONE)
            continue;
        mode = (sint32)game_session.mode;
        if ((i && game_session.player_count != 2) || !(mode == 2 || (mode < 2 && health->hp > 0)))
        {
            hud_continue_prompt(i);
            continue;
        }
        name = character_name_text(character_actor_names[i]);
        x = i ? 314 - text_width(name) : 16;
        if (i)
            text_draw(name, x, 16);
        else
            x = text_draw(name, x, 16);
        if (mode < 2)
        {
            if (i)
                x = (uint32)x - 10 - (uint32)text_width(hud.score_text[1]);
            text_draw_digits(hud.score_text[i], HUD_SCORE_TEXT_CAPACITY, x, 16);
        }
        if (i)
            hud_draw_health_right(0x001a0130, i);
        else
            hud_draw_health_left(0x001a0010, i);
        hud_ammo(i, ACTOR_ITEM_RIGHT);
        hud_ammo(i, ACTOR_ITEM_LEFT);
        idx = contact->last_target;
        if (contact->target_ticks && idx != -1)
        {
            /* TODO: Investigate a retained HUD target outside the actor pool */
            if (idx < 0 || idx >= ACTOR_COUNT)
            {
                DIAGNOSTIC(fprintf(stderr, "Invalid HUD target player %u target %d ticks %u stage %u tick %u\n", i, idx, (uint32)contact->target_ticks, game_session.stage, game_session.tick));
                diagnostic_stop(__FUNCTION__, __FILE__, __LINE__);
            }
            if (actor_health[idx].hp <= 0 || actor_types[idx] == ACTOR_TYPE_NONE)
                contact->target_ticks = 0;
            else
            {
                if (idx < 0 || idx >= CHARACTER_NAME_ACTOR_COUNT)
                    diagnostic_stop(__FUNCTION__, __FILE__, __LINE__);
                name = character_name_text(character_actor_names[idx]);
                text_draw(name, i ? 314 - text_width(name) : 16, 36);
                if (i)
                    hud_draw_health_right(0x002e0130, idx);
                else
                    hud_draw_health_left(0x002e0010, idx);
            }
        }
    }
    if (game_session.stage == 6 && game_session.scene_phase && !level_transition.finish_tick)
    {
        COLLISION_REGION *object0 = col_regions.named[0], *object1 = col_regions.named[1], *object2 = col_regions.named[2];
        uint32 choice;
        sint16 angle;
        sint32 width;
        object2->col_kind = 0;
        object1->col_kind = 0;
        object0->col_kind = 0;
        if (game_session.tick & 8)
            text_center(localization.text[LOCAL_TEXT_CHOOSE_ROUTE], 40);
        camera_update_focus();
        text_gradient_update();
        angle = (sint16)math_dir16(-1536, 512, camera.tracking.center.x.whole, (sint32)(0 - camera.tracking.center.z.bits) >> 16);
        choice = angle < -8192 ? 0 : angle >= 8193 ? 1 : 2;
        width = text_width(localization.text[LOCAL_TEXT_ROUTE_BRONX]);
        width = (sint32)(300 - (uint32)width - (uint32)text_width(localization.text[LOCAL_TEXT_ROUTE_PARK])) >> 1;
        text_draw_gradient(localization.text[LOCAL_TEXT_ROUTE_BRONX], width, 75, choice == 0 ? &text_active_gradient : &text_inactive_gradient);
        x = text_width(localization.text[LOCAL_TEXT_ROUTE_BRONX]);
        text_draw_gradient(localization.text[LOCAL_TEXT_ROUTE_PARK], (uint32)width + (uint32)x + 30, 75, choice == 1 ? &text_active_gradient : &text_inactive_gradient);
        x = (sint32)(320 - (uint32)text_width(localization.text[LOCAL_TEXT_ROUTE_MALL])) >> 1;
        text_draw_gradient(localization.text[LOCAL_TEXT_ROUTE_MALL], x, 55, choice == 2 ? &text_active_gradient : &text_inactive_gradient);
        level_transition.route_choice = (uint8)choice;
    }
    if (game_session.stage == 13 && level_transition.exit_requested && (sint32)stage_animation.phase[0] > 0 && !level_transition.finish_tick)
    {
        uint32 choice;
        sint32 width;
        if (game_session.tick & 8)
            text_center(localization.text[LOCAL_TEXT_CHOOSE_ROUTE], 40);
        camera_update_focus();
        text_gradient_update();
        choice = (sint32)camera.tracking.center.x.bits > 0x14000000;
        width = text_width(localization.text[LOCAL_TEXT_ROUTE_NAVAL_BASE]);
        width = (sint32)(320 - (uint32)width - (uint32)text_width(localization.text[LOCAL_TEXT_ROUTE_AIRBASE])) >> 1;
        text_draw_gradient(localization.text[LOCAL_TEXT_ROUTE_AIRBASE], width, 54, !choice ? &text_active_gradient : &text_inactive_gradient);
        x = text_width(localization.text[LOCAL_TEXT_ROUTE_AIRBASE]);
        text_draw_gradient(localization.text[LOCAL_TEXT_ROUTE_NAVAL_BASE], (uint32)width + (uint32)x + 10, 54, choice ? &text_active_gradient : &text_inactive_gradient);
    }
    {
        uint32 transition = level_transition.finish_tick, result = transition & 8;
        char text[96];
        if ((sint32)transition >= 16)
        {
            if (result)
                text_center(localization.text[LOCAL_TEXT_STAGE_CLEARED], 60);
            text_center(localization.text[LOCAL_TEXT_DESTRUCTION_BONUS], 100);
            sprintf(text, continue_count_format, localization.text[LOCAL_TEXT_CONTINUE], (sint32)level_transition.continues);
            x = 160 - (uint32)(text_width(text) >> 1);
            text_draw(text, x, 190);
            if (actor_health[0].hp > 0)
            {
                sprintf(text, destruction_format, (sint16)actor_bonus_counts[0], 250 * (sint16)actor_bonus_totals[0]);
                text_draw(text, 16, 120);
            }
            result = 2;
            if (game_session.player_count == 2)
            {
                result = (uint32)(sint32)actor_health[1].hp;
                if ((sint32)result > 0)
                {
                    sprintf(text, destruction_format, (sint16)actor_bonus_counts[1], 250 * (sint16)actor_bonus_totals[1]);
                    x = 314 - (uint32)text_width(text);
                    result = (uint32)text_draw(text, x, 120);
                }
            }
        }
        return result;
    }
}

/* Original 58634 scene tail, shared by gameplay, menu and all sequences */
GDB_CALL uint32 frame_draw_scene(void)
{
    sint32 fade;
    sint32 r, g, b;
    uint32 tick;
    if ((sint32)game_session.stage < 26)
        hud_draw();
    if ((sint32)level_transition.continue_phase >= 192)
        text_center(game_over_text, 110);
    r = disp_buffers.fade.r;
    g = disp_buffers.fade.g;
    b = disp_buffers.fade.b;
    if (r + g + b)
        disp_draw_fade();
    if (disp_buffers.flash_ticks)
        disp_draw_flash();
    proj_update_horizon();
    background_draw();
    tick = game_session.tick;
    if ((sint32)tick >= 33)
        return 0;
    fade = (tick << 3) - 256u;
    return disp_set_fade(fade, fade, fade);
}

GDB_CALL uint32 hud_update(void)
{
    FUNCTION_MARKER(0x80058C14, "GAME.EXE");
    int i, k;
    uint32 color_idx = 0, flags, result;
    hud.tick++;
    for (i = 0; i < 3; i++)
    {
        uint16 first = tex_effect_pals.cycle[color_idx];
        for (k = 0; k < 4; k++, color_idx++)
            tex_effect_pals.cycle[color_idx] = tex_effect_pals.cycle[color_idx + 1];
        tex_effect_pals.cycle[color_idx] = first;
        color_idx++;
    }
    /* Copy the fifteen colors without the original adjacent-RAM overread */
    for (i = 0; i < EFFECT_CYCLE_COLORS; i++)
        tex_pals.color[TEXTURE_PAL_EFFECTS][16 + i] = tex_effect_pals.cycle[i];
    flags = game_pause_flags;
    if (flags & 15)
    {
        if (game_session.player_count == 1)
        {
            if (flags & 3)
                text_center(localization.text[LOCAL_TEXT_PAUSED], 115);
        }
        else
        {
            if (game_pause_flags & 1)
                text_center(localization.text[LOCAL_TEXT_PAUSED1P], 115);
            if (game_pause_flags & 2)
                text_center(localization.text[LOCAL_TEXT_PAUSED2P], 115);
        }
    }
    if (game_pause_flags & 4)
        text_center(localization.text[LOCAL_TEXT_CONTROLLER1_REMOVED], 130);
    if (game_pause_flags & 8)
        text_center(localization.text[LOCAL_TEXT_CONTROLLER2_REMOVED], 130);
    result = disp_buffers.frame + 1;
    disp_buffers.frame = result;
    return result;
}
