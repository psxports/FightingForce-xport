#include "game.h"

BACKGROUND background;

static const sint8 lightning_pattern[40] = {
    1, 0, 0, 0, 1, 0, 0, 1, 0, 0, 1, 1, 0, 0, 1, 1, 0, 0, 1, 1, 1, 0, 1, 1, 1, 0, 1, 1, 1, 0, 1, 0, 1, 0, 0, 1, 0, 0, 0, 1,
};

static uint32 draw_black_quad(uint16 top, uint16 bottom, int horizon)
{
    POLY_F4 *poly;
    uint32 p = render_buffers.packet_addr;
    render_buffers.packet_addr = p + 24;
    poly = psx_addr(p, sizeof(*poly));
    setPolyF4(poly);
    setRGB0(poly, 0, 0, 0);
    poly->p[0].x = 0;
    poly->p[0].y = top;
    poly->p[1].x = 320;
    poly->p[1].y = top;
    poly->p[2].x = 0;
    if (horizon)
        bottom = (uint16)proj_horizon + 20;
    poly->p[2].y = bottom;
    poly->p[3].x = 320;
    poly->p[3].y = bottom;
    return render_ot_insert(render_buffers.ot_addr, p);
}

static void draw_gradient(uint32 top, uint32 bottom, uint32 height_bias, int reload_horizon)
{
    POLY_G4 *poly;
    uint8 r = (uint8)background.first_color << 3;
    uint32 color = background.first_color;
    uint8 g = (uint8)(((sint32)color >> 2) & ~7), b = ((sint32)color >> 7) & ~7;
    uint32 p = render_buffers.packet_addr;
    render_buffers.packet_addr = p + 36;
    poly = psx_addr(p, sizeof(*poly));
    setPolyG4(poly);
    setRGB0(poly, 0, 0, 0);
    setRGB1(poly, 0, 0, 0);
    setRGB2(poly, r, g, b);
    setRGB3(poly, r, g, b);
    poly->p[0].xy.x = 0;
    if (reload_horizon)
    {
        uint32 h = (uint16)proj_horizon;
        top = h - 256;
        bottom = h - height_bias + (reload_horizon == 1 ? 1 : 0);
    }
    poly->p[0].xy.y = top;
    poly->p[1].xy.x = 320;
    poly->p[1].xy.y = top;
    poly->p[2].xy.x = 0;
    poly->p[2].xy.y = bottom;
    poly->p[3].xy.x = 320;
    poly->p[3].xy.y = bottom;
    render_ot_insert(render_buffers.ot_addr, p);
}

static void draw_spr(uint32 x, uint32 y, uint32 v, uint32 height, int reload_horizon)
{
    SPRT *spr;
    uint32 p = render_buffers.packet_addr;
    render_buffers.packet_addr = p + 20;
    spr = psx_addr(p, sizeof(*spr));
    setSprt(spr);
    spr->p[0].attr = 0x7c00;
    setShadeTex(spr, 1);
    spr->p[0].xy.x = x;
    if (reload_horizon)
        y = (uint16)proj_horizon - y;
    spr->p[0].xy.y = y;
    setUV0(spr, 0, (uint8)v);
    setWH(spr, 256, (uint16)height);
    render_ot_insert(render_buffers.ot_addr, p);
}

static void draw_wave_occluder(void)
{
    static const uint32 xs[4] = {0xfe000000u, 0x02000000, 0xfa000000u, 0x06000000};
    PROJECTION_POINT points[4];
    uint32 i, p;
    POLY_F4 *poly;
    transform_identity();
    transform_pitch(camera.angles.pitch);
    transform_yaw(camera.angles.yaw);
    for (i = 0; i < 4; ++i)
    {
        uint32 camera_x, camera_y, camera_z = 0;
        transform_push();
        camera_x = camera.pos.x.bits;
        camera_y = camera.pos.y.bits;
        if (i < 2)
            camera_z = camera.pos.z.bits;
        transform_translate(xs[i] - camera_x, (uint32)(-(sint64)(camera_y)), i < 2 ? 0x07000000 - camera_z : 0x0fff0000);
        proj_local(0, 0, 0, &points[i]);
        transform_pop();
    }
    p = render_buffers.packet_addr;
    render_buffers.packet_addr = p + 24;
    poly = psx_addr(p, sizeof(*poly));
    setPolyF4(poly);
    setRGB0(poly, 0, 0, 0);
    for (i = 0; i < 4; ++i)
    {
        poly->p[i].x = points[i].x;
        poly->p[i].y = (uint32)points[i].y - (i < 2 ? 1 : 0);
    }
    render_ot_insert(render_buffers.ot_addr, p);
}

static uint32 draw_occluder(uint32 left, uint32 right)
{
    FUNCTION_MARKER(0x80064d34u, "GAME.EXE");
    PROJECTION_POINT points[4];
    uint32 i, p;
    POLY_F4 *poly;
    transform_identity();
    transform_pitch(camera.angles.pitch);
    transform_yaw(camera.angles.yaw);
    for (i = 0; i < 4; ++i)
    {
        uint32 camera_x, camera_y;
        transform_push();
        camera_x = camera.pos.x.bits;
        camera_y = camera.pos.y.bits;
        transform_translate(((i & 1) ? right : left) - camera_x, (uint32)(-(sint64)(camera_y)), i < 2 ? 0x7fff0000 : 0x0fff0000);
        proj_local(0, 0, 0, &points[i]);
        transform_pop();
    }
    p = render_buffers.packet_addr;
    render_buffers.packet_addr = p + 24;
    poly = psx_addr(p, sizeof(*poly));
    setPolyF4(poly);
    setRGB0(poly, 0, 0, 0);
    for (i = 0; i < 4; ++i)
    {
        poly->p[i].x = points[i].x;
        poly->p[i].y = points[i].y;
    }
    return render_ot_insert(render_buffers.ot_addr, p);
}

uint32 background_draw_ground(void)
{
    POLY_F4 *poly;
    FUNCTION_MARKER(0x80064380u, "GAME.EXE");
    uint32 x = camera.pos.x.bits, z = camera.pos.z.bits, rx = x & 0xffffff, rz = z & 0xffffff;
    uint32 column = (sint32)~x >> 24, row = (sint32)~z >> 24, heading = ((camera.angles.yaw + 2048) >> 12) & 15;
    uint32 i, p;
    uint16 horizon;
    if (camera.angles.pitch < 8192)
    {
        uint32 dx = terrain_view_offset[heading].x, dz = terrain_view_offset[heading].z;
        rx += dx << 23;
        rz += dz << 23;
        column += (uint32)((sint32)dx >> 1);
        row += (uint32)((sint32)dz >> 1);
    }
    transform_reset();
    for (i = 0; i < 4; ++i)
    {
        transform_push();
        transform_translate(((i & 1) ? 0xf8000000u : 0x08000000u) - rx, (uint32)(-(sint64)(camera.pos.y.bits)), ((i & 2) ? 0xf8000000u : 0x08000000u) - rz);
        terrain_draw_faded_grid(0, column + ((i & 1) ? 16 : 0), row + ((i & 2) ? 16 : 0));
        transform_pop();
    }
    p = render_buffers.packet_addr;
    render_buffers.packet_addr = p + 24;
    poly = psx_addr(p, sizeof(*poly));
    setlen(poly, 5);
    poly->rgbc.rgbc = 0x28000000;
    poly->p[0].x = 0;
    horizon = (sint16)proj_horizon;
    poly->p[0].y = horizon;
    poly->p[1].x = 320;
    poly->p[1].y = horizon;
    poly->p[2].x = 0;
    horizon += 26;
    poly->p[2].y = horizon;
    poly->p[3].x = 320;
    poly->p[3].y = horizon;
    return render_ot_insert(render_buffers.ot_addr, p);
}

/* 6494C: sky gradient and three horizontally wrapping sprites. MIPS keeps
 * coordinates in 32 bits until SH; the stage-4 horizon offset is +132. */
uint32 background_draw_gradient_sky(void)
{
    POLY_G4 *poly;
    SPRT *spr;
    DR_MODE *mode;
    FUNCTION_MARKER(0x8006494c, "GAME.EXE");
    uint32 horizon = proj_horizon, bottom, a, color, x, v, i;
    if (game_session.stage == 4)
        horizon += 132u;
    bottom = horizon - 128u;
    if ((sint32)bottom > 0)
    {
        color = background.first_color;
        a = render_buffers.packet_addr;
        render_buffers.packet_addr = a + 36;
        poly = psx_addr(a, sizeof(*poly));
        setPolyG4(poly);
        for (i = 0; i < 4; i++)
        {
            poly->p[i].rgbc.r = i < 2 ? 0 : (uint8)(color << 3);
            poly->p[i].rgbc.g = i < 2 ? 0 : (uint8)(((sint32)color >> 2) & ~7);
            poly->p[i].rgbc.b = i < 2 ? 0 : (uint8)(((sint32)color >> 7) & ~7);
            poly->p[i].xy.x = (uint16)((i & 1) ? 320 : 0);
            poly->p[i].xy.y = (uint16)(i < 2 ? horizon - 288u : bottom);
        }
        render_ot_insert(render_buffers.ot_addr, a);
    }
    x = (camera.angles.yaw >> 5) + 160u;
    v = (x & 256u) >> 1;
    x &= 255u;
    for (i = 0; i < 3; i++)
    {
        a = render_buffers.packet_addr;
        render_buffers.packet_addr = a + 20;
        spr = psx_addr(a, sizeof(*spr));
        setSprt(spr);
        spr->p[0].attr = 0x7c00;
        setShadeTex(spr, 1);
        spr->p[0].xy.x = x + 256u * i - 256u;
        spr->p[0].xy.y = bottom;
        setUV0(spr, 0, (uint8)(i == 1 ? 128u - v : v));
        setWH(spr, 256, 128);
        render_ot_insert(render_buffers.ot_addr, a);
    }
    a = render_buffers.packet_addr;
    render_buffers.packet_addr = a + 12;
    mode = psx_addr(a, sizeof(*mode));
    setDrawMode(mode, 1, 0, 142, NULL);
    if ((uint32)(GetGraphType() - 1) < 2)
        mode->code[0] = 0xe100108e;
    return render_ot_insert(render_buffers.ot_addr, a);
}

uint32 background_draw_black_to_horizon(void)
{
    FUNCTION_MARKER(0x80064c1cu, "GAME.EXE");
    return draw_black_quad(0, 0, 1);
}

uint32 background_draw_lower_black(void)
{
    FUNCTION_MARKER(0x80064cacu, "GAME.EXE");
    return draw_black_quad(160, 255, 0);
}

uint32 background_draw_tall_sky(void)
{
    FUNCTION_MARKER(0x80064f38u, "GAME.EXE");
    uint32 bias = 160, scroll, i, stage;
    if (game_session.stage - 8 < 2)
    {
        draw_occluder(0xf0000000u, 0);
        draw_occluder(0, 0x10000000);
    }
    stage = game_session.stage;
    if (stage == 10)
    {
        draw_occluder(0xfb000000u, 0x05000000);
        stage = game_session.stage;
    }
    if (stage == 4)
    {
        bias = 64;
        background.first_color = 4198;
    }
    if ((sint32)(proj_horizon - bias) > 0)
        draw_gradient(0, 0, bias, 1);
    scroll = ((uint16)camera.angles.yaw >> 5) & 255;
    for (i = 0; i < 3; ++i)
        draw_spr(scroll + 256 * i - 256, bias, 0, 256, 1);
    return render_insert_tex_page(142, 0);
}

uint32 background_draw_split_sky(void)
{
    SPRT *spr;
    FUNCTION_MARKER(0x80065274u, "GAME.EXE");
    uint32 horizon = proj_horizon, top, middle, angle, scroll, v, i;
    if (game_session.stage - 8 < 2)
        horizon += 24;
    top = horizon - 256;
    middle = horizon - 128;
    if ((sint32)top > 0)
        top = 0;
    if ((sint32)middle > 0)
        draw_gradient(top, middle, 0, 0);
    draw_black_quad((uint16)horizon, 256, 0);
    angle = (camera.angles.yaw >> 5) + 160;
    scroll = angle & 255;
    v = (angle & 256) >> 1;
    for (i = 0; i < 3; ++i)
    {
        uint32 p = render_buffers.packet_addr;
        render_buffers.packet_addr = p + 20;
        spr = psx_addr(p, sizeof(*spr));
        setSprt(spr);
        spr->p[0].attr = 0x7c00;
        setShadeTex(spr, 1);
        spr->p[0].xy.x = scroll + 256 * i - 256;
        spr->p[0].xy.y = middle;
        setUV0(spr, 0, (uint8)(i == 1 ? 128 - v : v));
        setWH(spr, 256, 128);
        render_ot_insert(render_buffers.ot_addr, p);
    }
    return render_insert_tex_page(142, 0);
}

uint32 background_draw_two_row_sky(void)
{
    FUNCTION_MARKER(0x800655c0u, "GAME.EXE");
    uint32 scroll = ((camera.angles.yaw >> 5) + 160) & 255, i;
    for (i = 0; i < 3; ++i)
        draw_spr(scroll + 256 * i - 256, 128, 128, 128, 1);
    if ((sint32)(proj_horizon - 128) > 0)
        for (i = 0; i < 3; ++i)
            draw_spr(scroll + 256 * i - 256, 256, 0, 128, 1);
    return render_insert_tex_page(142, 0);
}

uint32 background_draw_black_to_horizon_alt(void)
{
    FUNCTION_MARKER(0x80065918u, "GAME.EXE");
    return draw_black_quad(0, 0, 1);
}

sint32 background_draw_lightning(void)
{
    POLY_F4 *poly;
    FUNCTION_MARKER(0x800659a8u, "GAME.EXE");
    uint32 p = render_buffers.packet_addr;
    sint32 phase, volume = 0;
    render_buffers.packet_addr = p + 24;
    poly = psx_addr(p, sizeof(*poly));
    setlen(poly, 5);
    poly->rgbc.rgbc = 0x28000000;
    poly->p[0].xy = 0;
    poly->p[1].xy = 320;
    poly->p[2].xy = 160u << 16;
    poly->p[3].xy = (160u << 16) | 320;
    render_ot_insert(render_buffers.ot_addr, p);
    phase = (game_session.tick & 511) - 472;
    if (phase >= 0 && lightning_pattern[phase])
    {
        p = render_buffers.packet_addr;
        render_buffers.packet_addr = p + 24;
        poly = psx_addr(p, sizeof(*poly));
        setlen(poly, 5);
        poly->rgbc.rgbc = 0x2a808080;
        poly->p[0].xy = 0;
        poly->p[1].xy = 320;
        poly->p[2].xy = 256u << 16;
        poly->p[3].xy = (256u << 16) | 320;
        render_ot_insert(render_buffers.ot_addr + 8188, p);
        render_insert_tex_page(206, 2047);
        volume = 255;
    }
    return sound_voice_volume(stage_audio_slots[1].voice, volume);
}

uint32 background_draw_wave_sky(void)
{
    POLY_F4 *poly;
    FUNCTION_MARKER(0x80065b3cu, "GAME.EXE");
    uint32 angle;
    uint32 horizon;
    uint32 p;
    sint32 color;
    uint32 scroll;
    uint32 v;
    uint32 i;
    uint32 wave_delta;
    uint8 r, g, b;
    if ((sint32)camera.pos.z.bits < (sint32)0xf4000000u)
        draw_wave_occluder();
    angle = (uint16)camera.angles.yaw >> 5;
    wave_delta = (uint32)(sint32)sin_table[angle] << 3;
    background.wave_phase -= wave_delta;
    horizon = proj_horizon;
    if ((sint32)(horizon - 84) > 0)
        draw_gradient(horizon - 212, horizon - 84, 0, 0);
    r = (uint8)background.last_color << 3;
    color = background.last_color;
    g = (color >> 2) & ~7;
    b = (color >> 7) & ~7;
    p = render_buffers.packet_addr;
    render_buffers.packet_addr = p + 24;
    poly = psx_addr(p, sizeof(*poly));
    setPolyF4(poly);
    setRGB0(poly, r, g, b);
    setXY4(poly, 0, (uint16)(horizon + 44), 320, (uint16)(horizon + 44), 0, 255, 320, 255);
    render_ot_insert(render_buffers.ot_addr, p);
    angle = (camera.angles.yaw >> 5) + (uint32)(sint32)(sint16)(background.wave_phase >> 16);
    scroll = angle & 255;
    v = (angle & 256) >> 1;
    for (i = 0; i < 3; ++i)
        draw_spr(scroll + 256 * i - 256, horizon - 84, i == 1 ? 128 - v : v, 128, 0);
    return render_insert_tex_page(142, 0);
}

uint32 background_draw_black(void)
{
    FUNCTION_MARKER(0x800660dcu, "GAME.EXE");
    return draw_black_quad(0, 256, 0);
}

uint32 background_draw_lift_sky(void)
{
    POLY_G4 *poly0;
    SPRT *spr;
    POLY_F4 *poly1;
    FUNCTION_MARKER(0x80066160u, "GAME.EXE");
    uint32 height = ((sint32)lift.travel >> 9) + 64u + proj_horizon;
    sint32 top = height - 256, middle = height - 128;
    sint32 color;
    uint32 p;
    uint32 i;
    uint32 angle;
    uint32 scroll;
    uint32 v;
    uint8 r, g, b;
    if (top > 0)
        top = 0;
    if (middle > 0)
    {
        r = (uint8)background.first_color << 3;
        color = background.first_color;
        g = (color >> 2) & ~7;
        b = (color >> 7) & ~7;
        p = render_buffers.packet_addr;
        render_buffers.packet_addr = p + 36;
        poly0 = psx_addr(p, sizeof(*poly0));
        setPolyG4(poly0);
        setRGB0(poly0, 0, 0, 0);
        setRGB1(poly0, 0, 0, 0);
        setRGB2(poly0, r, g, b);
        setRGB3(poly0, r, g, b);
        poly0->p[0].xy.x = 0;
        poly0->p[0].xy.y = top;
        poly0->p[1].xy.x = 320;
        poly0->p[1].xy.y = top;
        poly0->p[2].xy.x = 0;
        poly0->p[2].xy.y = middle;
        poly0->p[3].xy.x = 320;
        poly0->p[3].xy.y = middle;
        render_ot_insert(render_buffers.ot_addr, p);
    }
    angle = (camera.angles.yaw >> 5) + 256;
    scroll = angle & 255;
    v = (angle & 256) >> 1;
    for (i = 0; i < 3; ++i)
    {
        p = render_buffers.packet_addr;
        render_buffers.packet_addr = p + 20;
        spr = psx_addr(p, sizeof(*spr));
        setSprt(spr);
        spr->p[0].attr = 0x7c00;
        setShadeTex(spr, 1);
        spr->p[0].xy.x = scroll + 256 * i - 256;
        spr->p[0].xy.y = height - 128;
        setUV0(spr, 0, (uint8)(i == 1 ? 128 - v : v));
        setWH(spr, 256, 128);
        render_ot_insert(render_buffers.ot_addr, p);
    }
    render_insert_tex_page(0x8e, 0);
    r = (uint8)background.last_color << 3;
    color = background.last_color;
    g = (color >> 2) & ~7;
    b = (color >> 7) & ~7;
    p = render_buffers.packet_addr;
    render_buffers.packet_addr = p + 24;
    poly1 = psx_addr(p, sizeof(*poly1));
    setPolyF4(poly1);
    setRGB0(poly1, r, g, b);
    setXY4(poly1, 0, 256, 320, 256, 0, (uint16)(height - 256), 320, (uint16)(height - 256));
    return render_ot_insert(render_buffers.ot_addr, p);
}

uint32 background_draw_high_sky(void)
{
    FUNCTION_MARKER(0x800664f0u, "GAME.EXE");
    uint32 scroll, i;
    if ((sint32)(proj_horizon - 128) > 0)
        draw_gradient(0, 0, 128, 2);
    scroll = ((uint16)camera.angles.yaw >> 5) & 255;
    for (i = 0; i < 3; ++i)
        draw_spr(scroll + 256 * i - 256, 128, 0, 256, 1);
    return render_insert_tex_page(142, 0);
}

uint32 background_draw_fixed_gradient(void)
{
    POLY_G4 *poly;
    FUNCTION_MARKER(0x800667b8u, "GAME.EXE");
    uint32 p = render_buffers.packet_addr;
    render_buffers.packet_addr = p + 36;
    poly = psx_addr(p, sizeof(*poly));
    setPolyG4(poly);
    setRGB0(poly, 0, 0, 32);
    setRGB1(poly, 0, 0, 32);
    setRGB2(poly, 64, 32, 16);
    setRGB3(poly, 64, 32, 16);
    poly->p[0].xy.x = 0;
    poly->p[0].xy.y = 0;
    poly->p[1].xy.x = 320;
    poly->p[1].xy.y = 0;
    poly->p[2].xy.x = 0;
    poly->p[2].xy.y = 128;
    poly->p[3].xy.x = 320;
    poly->p[3].xy.y = 128;
    return render_ot_insert(render_buffers.ot_addr, p);
}

/* Original signed shifts and low-halfword parallax coordinates */
uint32 background_draw_parallax_sky(void)
{
    POLY_F4 *poly;
    SPRT *spr;
    DR_MODE *mode;
    FUNCTION_MARKER(0x8006686c, "GAME.EXE");
    sint32 camera_x = camera.pos.x.val;
    uint32 yaw = camera.angles.yaw;
    sint32 x = (sint32)((camera_x >> 8) - ((yaw << 10) + 0x200000)) >> 16;
    uint32 horizon = proj_horizon - 256u, a, color, r;
    if ((sint32)horizon > 0)
    {
        r = ((uint8)background.first_color) << 3;
        color = background.first_color;
        a = render_buffers.packet_addr;
        render_buffers.packet_addr = a + 24;
        poly = psx_addr(a, sizeof(*poly));
        setPolyF4(poly);
        setRGB0(poly, (uint8)r, (uint8)(((sint32)color >> 2) & ~7), (uint8)(((sint32)color >> 7) & ~7));
        poly->p[0].x = 0;
        poly->p[0].y = 0;
        poly->p[1].x = 320;
        poly->p[1].y = 0;
        poly->p[2].x = 0;
        horizon = ((uint16)proj_horizon) - 256;
        poly->p[2].y = horizon;
        poly->p[3].x = 320;
        poly->p[3].y = horizon;
        render_ot_insert(render_buffers.ot_addr, a);
    }
    a = render_buffers.packet_addr;
    render_buffers.packet_addr = a + 20;
    spr = psx_addr(a, sizeof(*spr));
    setSprt(spr);
    spr->p[0].attr = 0x7c00;
    setShadeTex(spr, 1);
    spr->p[0].xy.x = (uint32)(-(sint64)(x));
    horizon = ((uint16)proj_horizon) - 256;
    spr->p[0].xy.y = horizon;
    setUV0(spr, 0, 0);
    setWH(spr, 256, 256);
    render_ot_insert(render_buffers.ot_addr, a);
    a = render_buffers.packet_addr;
    render_buffers.packet_addr = a + 12;
    mode = psx_addr(a, sizeof(*mode));
    setDrawMode(mode, 1, 0, 142, NULL);
    /* Original SDK supports alternate GPU revisions with the DFE bit at 12. */
    if ((uint32)(GetGraphType() - 1) < 2)
        mode->code[0] = 0xe100108e;
    return render_ot_insert(render_buffers.ot_addr, a);
}

/* 58634: audited background dispatch 5887C..58BC8. The caller performs
 * horizon projection and the final fade, so this helper emits no entry marker. */
void background_draw(void)
{
    uint32 stage = game_session.stage;
    if (!stage)
        goto base_background;
    stage = game_session.stage;
    if (stage == 1)
        goto horizon_background;
    if (stage == 2)
    {
        background_draw_black();
        return;
    }
    if (stage == 3)
    {
        background_draw_black_to_horizon();
        goto horizon_background;
    }
    stage = game_session.stage;
    if (stage == 4)
    {
        background_draw_tall_sky();
        goto horizon_background;
    }
    if (stage == 7)
    {
        background_draw_two_row_sky();
        goto horizon_background;
    }
    stage = game_session.stage;
    if (stage == 10)
    {
        background_draw_tall_sky();
        goto extra_background;
    }
    if (stage - 8 < 2 || stage == 11)
    {
        background_draw_split_sky();
        goto extra_background;
    }
    stage = game_session.stage;
    if (stage - 12 < 2)
    {
        background_draw_black_to_horizon_alt();
        goto extra_background;
    }
    stage = game_session.stage;
    if (stage == 14)
    {
        background_draw_lightning();
        goto extra_background;
    }
    if (stage == 17)
    {
        background_draw_wave_sky();
        goto extra_background;
    }
    stage = game_session.stage;
    if (stage == 18)
    {
        uint32 x = camera.pos.x.bits, y = camera.pos.y.bits, z = camera.pos.z.bits, tick, px, py, pz;
        background_draw_gradient_sky();
        tick = game_session.tick;
        pz = tick << 23;
        camera.pos.z.bits = pz;
        px = (uint32)(sint32)sin_table[((tick << 5) & 0xfe0) >> 1] << 4;
        camera.pos.x.bits = px;
        py = camera.pos.y.bits + 0xfcec0000u;
        camera.pos.y.bits = py;
        camera.pos.x.bits = (uint32)((sint32)px >> 2);
        camera.pos.y.bits = (uint32)((sint32)py >> 2);
        camera.pos.z.bits = (uint32)((sint32)pz >> 2);
        terrain_draw();
        camera.pos.x.bits = x;
        camera.pos.y.bits = y;
        camera.pos.z.bits = z;
        return;
    }
    if (stage == 19)
    {
        background_draw_black_to_horizon();
        goto extra_background;
    }
    stage = game_session.stage;
    if (stage == 22)
    {
        background_draw_black();
        goto extra_background;
    }
    if (stage == 23)
    {
        background_draw_lift_sky();
        return;
    }
    stage = game_session.stage;
    if (stage == 24)
    {
        background_draw_black_to_horizon();
        goto extra_background;
    }
    if (stage == 26)
    {
        background_draw_parallax_sky();
        goto horizon_background;
    }
    stage = game_session.stage;
    if (stage - 31 < 2 || stage == 33)
    {
        background_draw_high_sky();
        goto horizon_background;
    }
    if (stage == 34)
    {
        if ((sint32)game_session.tick >= 301)
        {
            background_draw_fixed_gradient();
            return;
        }
        background_draw_black();
        goto horizon_background;
    }
base_background:
    background_draw_gradient_sky();
horizon_background:
    terrain_draw();
    return;
extra_background:
    background_draw_ground();
}
