#include "game.h"

uint32 credits_run(void)
{
    FUNCTION_MARKER(0x8005a280u, "GAME.EXE");
    RESOURCE_VIEW credits = resource_find("CREDITS.TXT");
    uint32 scroll = 0, frame = 0, p, dst, shade, result;
    const uint8 *src, *end = credits.data + credits.byte_len;
    char line[128];
    PROJECTION_POINT pos;
    sint32 fade, slide, spr, next, y;
    sint32 r, g, b;
    int i;
    uint8 c;
    cd_music_start_track(12);
    for (;; ++frame)
    {
        fade = (frame & 255) - 192;
        if (fade < 0)
            fade = 0;
        fade *= 2;
        slide = ((sint32)frame >> 8) % 5;
        if ((sint32)frame < 17)
            disp_fade_from_origin(frame << 4);
        text_gradient_update();
        disp_begin_frame();
        r = disp_buffers.fade.r;
        g = disp_buffers.fade.g;
        b = disp_buffers.fade.b;
        if (r + g + b)
            disp_draw_fade();
        p = render_buffers.packet_addr;
        render_buffers.packet_addr = p + 24;
        POLY_F4 *poly = psx_addr(p, sizeof(*poly));
        setPolyF4(poly); /* SKIP6E798/6E6F8(0). */
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
        spr = tex_map[(uint32)tex_grps[slide + 1].first_tex];
        pos.x = 96;
        pos.y = 128;
        pos.depth = 256;
        pos.size = 128;
        shade = 128u - fade;
        shade = (shade << 16) + (shade << 8) + 0x2c000000u + shade;
        render_style.color.rgbc = shade;
        spr_draw_scaled(&pos, spr);
        pos.x += 127;
        render_style.color.rgbc = shade;
        spr_draw_scaled(&pos, spr + 1);
        pos.x -= 127;
        next = slide == 4 ? tex_map[(uint32)tex_grps[1].first_tex] : spr + 2;
        shade = ((uint32)fade << 16) + ((uint32)fade << 8) + 0x2c000000u + fade;
        render_style.color.rgbc = shade;
        spr_draw_scaled(&pos, next);
        pos.x += 127;
        render_style.color.rgbc = shade;
        spr_draw_scaled(&pos, next + 1);
        if ((sint32)scroll < 2062)
            ++scroll;
        if (scroll == 2062)
            text_center(localization.hint[TEXT_HINT_CONTINUE], 214);
        src = credits.data;
        y = 256 - ((sint32)scroll >> 1);
        for (;;)
        {
            XASSERT(src != end);
            c = *src;
            if (c == '<')
                break;
            dst = 0;
            do
            {
                XASSERT(dst < sizeof(line));
                line[dst++] = (char)c;
                ++src;
                XASSERT(src != end);
                c = *src;
            } while (c != 10 && c != '<');
            c = *src;
            ++src;
            if (c == '<')
                break;
            line[dst - 1] = 0; /* Strip CR before LF, as5A5B4 */
            if (y + 9u < 265u)
            {
                if ((sint8)line[0] == '@')
                    text_center_gradient(line + 1, y);
                else
                    text_center(line, y);
            }
            y = y + 10u;
        }
        menu_present();
        if (scroll == 2062)
            for (i = 0; i < (sint32)game_session.player_count; ++i)
            {
                result = pad_decode_buttons(pad_game.port[i]) & 0x4008;
                if (result)
                {
                    return result;
                }
            }
    }
}
