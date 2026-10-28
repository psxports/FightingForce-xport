#include "game.h"
#include "platform.h"
#include "psx_gpu.h"

DISPLAY_BUFFERS disp_buffers;

/* Existing setup phase of original5748C, not a new original function */
GDB_CALL void disp_setup(void)
{
    TILE clear = {0};
    uint16 offset;
    gpu_set_def_draw_env(0x800b8938, 0, 0, 320, 256);
    gpu_set_def_draw_env(0x800b8994, 320, 0, 320, 256);
    SetDefDispEnv(&disp_buffers.env[0], 320, 0, 320, 256);
    SetDefDispEnv(&disp_buffers.env[1], 0, 0, 320, 256);
    offset = (uint16)game_settings.scr_offset.x;
    disp_buffers.env[0].screen.x = offset;
    disp_buffers.env[1].screen.x = offset;
    offset = (uint16)game_settings.scr_offset.y;
    disp_buffers.env[0].screen.y = offset;
    disp_buffers.env[1].screen.y = offset;
    clear.tag = 0x03000000;
    clear.rgbc.code = 2;
    clear.w = 640;
    clear.h = 256;
    SetDispMask(1);
    DrawPrim(&clear);
    render_ot_init(0xe7140);
    render_ot_init(0xe5140);
    VSync(0);
    disp_buffers.frame = 0;
    InitGeom();
    SetGeomScreen(256);
    SetGeomOffset(160, 120);
    transform_reset();
}

/* Original VBlank leaf has no effects and its return value is discarded */
static void vblank_callback(void)
{
    FUNCTION_MARKER(0x8006467c, "GAME.EXE");
}

/* 5717C. IRQ/card BIOS lifecycle calls do not execute on Windows; card events
 * are WIP. Host polling replaces InitPAD/StartPAD and ChangeClearPAD(0).
 * _bu_init and InitCARD/StartCARD have no storage behind them. */
GDB_CALL sint32 game_init_devices(void)
{
    FUNCTION_MARKER(0x8005717c, "GAME.EXE");
    static const uint32 spec[4] = {4, 0x8000, 0x100, 0x2000};
    uint32 i;
    sint32 result = 0;
    ResetCallback();              /* 75C14: native scheduling belongs to the window loop. */
    ++platform_boundary_calls[1]; /* WIP729DC CdInit: host assets need no drive reset. */
    CdSetDebug(0);
    card_backend_reset_events();
    for (i = 0; i < 8; i++)
        card_storage.event[i / CARD_EVENT_COUNT][i % CARD_EVENT_COUNT] = card_backend_open_event(i < 4 ? 0xf4000001 : 0xf0000011, spec[i & 3], 0x2000, 0);
    card_backend_init(0);
    pad_host_init();
    card_backend_start();
    ++platform_boundary_calls[0]; /* WIP _bu_init. */
    pad_host.enabled = 1;         /* SKIP6D178 StartPAD wrapper */
    if (ResetGraph(0) < 0)
        diagnostic_stop(__FUNCTION__, __FILE__, __LINE__);
    gpu_reset_graph_state();
    SetGraphDebug(0);
    for (i = 0; i < 8; i++)
        result = card_backend_enable_event(card_storage.event[i / CARD_EVENT_COUNT][i % CARD_EVENT_COUNT]);
    return result;
}

GDB_CALL void disp_initialize(void)
{
    FUNCTION_MARKER(0x8005748C, "GAME.EXE");
    game_init_devices();
    psx_bios_vsync_callback(vblank_callback);
    cd_music_init();
    disp_setup();
}

GDB_CALL uint32 disp_apply_offsets(void)
{
    FUNCTION_MARKER(0x800575f8, "GAME.EXE");
    uint16 x = (uint16)game_settings.scr_offset.x, y;
    disp_buffers.env[0].screen.x = x;
    disp_buffers.env[1].screen.x = x;
    y = (uint16)game_settings.scr_offset.y;
    disp_buffers.env[0].screen.y = y;
    disp_buffers.env[1].screen.y = y;
    return y;
}

/* Existing offset phase of original58634, retaining the captured parity */
GDB_CALL uint32 disp_update_offsets(void)
{
    uint32 shake = disp_buffers.shake_ticks, parity;
    sint32 denominator = (sint32)(18u - shake), delta;
    if (!denominator)
        diagnostic_stop(__FUNCTION__, __FILE__, __LINE__); /* original BREAK7 */
    delta = (sint32)((shake & 1) << 5) / denominator;
    parity = disp_buffers.frame & 1;
    disp_buffers.env[parity].screen.x = (uint16)((uint32)delta + (uint16)game_settings.scr_offset.x);
    disp_buffers.env[parity].screen.y = (uint16)((uint32)delta + (uint16)game_settings.scr_offset.y);
    return parity;
}

uint32 disp_begin_frame(void)
{
    FUNCTION_MARKER(0x80067284, "GAME.EXE");
    uint32 parity;
    DISPENV *disp;
    sint32 offset_x, offset_y;
    DrawSync(0);
    if (!loading.active)
        VSync(2);
    parity = disp_buffers.frame & 1;
    disp = disp_buffers.env + parity;
    disp->screen.x = (uint16)game_settings.scr_offset.x;
    disp->screen.y = (uint16)game_settings.scr_offset.y;
    PutDrawEnv(psx_addr((0x80000000u | RENDER_DRAW_ENV_BASE) + sizeof(DRAWENV) * parity, sizeof(DRAWENV)));
    parity = disp_buffers.frame & 1;
    disp = disp_buffers.env + parity;
    PutDispEnv(disp);
    offset_x = disp->screen.x;
    offset_y = disp->screen.y;
    gpu_display_offset(offset_x, offset_y);
    render_buffers_select();
    return render_ot_init(render_buffers.ot_addr);
}

/* 80011CCC: NEG/ADDI are trapping instructions; retain that domain explicitly */
GDB_CALL uint32 disp_set_fade(sint32 r, sint32 g, sint32 b)
{
    FUNCTION_MARKER(0x80011ccc, "GAME.EXE");
    sint32 vals[3] = {r, g, b};
    int i;
    for (i = 0; i < 3; i++)
    {
        sint64 n = -(sint64)vals[i];
        if (n > 2147483647 || n - 255 < -2147483647 - 1)
            diagnostic_stop(__FUNCTION__, __FILE__, __LINE__);
        vals[i] = (sint32)((n >= 255 ? 255 : (uint32)n) & 255);
    }
    for (i = 0; i < 3; i++)
        disp_buffers.fade.channel[i] = vals[i];
    return 0x8001000c;
}

GDB_CALL uint32 disp_fade_from_origin(sint32 level)
{
    FUNCTION_MARKER(0x8005ee58, "GAME.EXE");
    sint32 color[3];
    uint32 delta = (uint32)level - 256u;
    int i;
    for (i = 0; i < 3; i++)
    {
        color[i] = (sint32)((uint32)(sint32)disp_buffers.fade_origin.channel[i] + delta);
        if (color[i] < -256)
            color[i] = -256;
    }
    return disp_set_fade(color[0], color[1], color[2]);
}

/* 8006471C/80064820 share the same audited full-screen packet tail.
 * SDK shape setters reuse PsyQ; draw mode follows the guest GPU model */
static uint32 begin_overlay(void)
{
    uint32 p = render_buffers.packet_addr;
    POLY_F4 *poly;
    render_buffers.packet_addr = p + 24;
    poly = psx_addr(p, sizeof(*poly));
    setPolyF4(poly);
    setSemiTrans(poly, 1);
    return p;
}

static uint32 finish_overlay(uint32 p, uint16 page)
{
    POLY_F4 *poly = psx_addr(p, sizeof(*poly));
    setXY4(poly, 0, 0, 320, 0, 0, 256, 320, 256);
    render_ot_insert(render_buffers.ot_addr + 8188, p);
    return render_insert_tex_page(page, 2047);
}

uint32 disp_draw_fade(void)
{
    uint32 p;
    FUNCTION_MARKER(0x8006471c, "GAME.EXE");
    p = begin_overlay();
    POLY_F4 *poly = psx_addr(p, sizeof(*poly));
    setRGB0(poly, (uint8)disp_buffers.fade.r, (uint8)disp_buffers.fade.g, (uint8)disp_buffers.fade.b);
    return finish_overlay(p, 206);
}

uint32 disp_draw_flash(void)
{
    uint32 phase, p;
    uint8 color;
    FUNCTION_MARKER(0x80064820, "GAME.EXE");
    phase = disp_buffers.flash_ticks - 6u;
    color = (sint32)phase > 0 ? 255u - 85u * phase : 255u + 42u * phase;
    p = begin_overlay();
    POLY_F4 *poly = psx_addr(p, sizeof(*poly));
    setRGB0(poly, color, color, color);
    return finish_overlay(p, 174);
}

GDB_CALL sint32 disp_update_pulse(void)
{
    FUNCTION_MARKER(0x80058238u, "GAME.EXE");
    uint32 phase;
    sint32 shade;
    if (!disp_buffers.pulse_active && !(uint8)disp_buffers.pulse_phase)
        return 0;
    phase = disp_buffers.pulse_phase + 2u;
    disp_buffers.pulse_phase = phase;
    shade = (sint32)((2u * phase + 128u) & 255u) - 128;
    if (shade > 0)
        shade = -shade;
    disp_set_fade(0, shade, shade);
    disp_buffers.fade_origin.r = (sint16)(0);
    disp_buffers.fade_origin.b = (sint16)((uint16)shade);
    disp_buffers.fade_origin.g = (sint16)((uint16)shade);
    return shade;
}

GDB_CALL sint32 disp_draw_scene(void)
{
    FUNCTION_MARKER(0x8005c2a0u, "GAME.EXE");
    world_objects_draw();
    dynamic_objects_draw();
    if ((sint32)game_session.stage < 27)
        characters_draw();
    return effects_draw();
}

GDB_CALL sint32 disp_wait_vblank(void)
{
    FUNCTION_MARKER(0x80061120u, "GAME.EXE");
    return VSync(0);
}

uint32 disp_prepare_scene(void)
{
    FUNCTION_MARKER(0x80057630u, "GAME.EXE");
    uint32 i = 0, count;
    tex_upload_pals();
    menu_present_dim();
    menu_present_dim();
    menu_present_dim();
    /* Resume ordinary presentation only after the loading handoff */
    loading.active = 0;
    count = game_session.player_count;
    if ((sint32)count <= 0)
        return count;
    do
    {
        hud_format_score(i);
        i++;
    } while ((sint32)i < (sint32)game_session.player_count);
    return 0;
}
