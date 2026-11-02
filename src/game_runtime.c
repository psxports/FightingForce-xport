#include <string.h>
#include <setjmp.h>

#include "game.h"
#include "psx_gpu.h"
#include "sound_device.h"
#include "movies.h"
#include "xport.h"
#include "platform.h"
#include "hd.h"

GAME_SESSION game_session;

static jmp_buf menu_exit;
static jmp_buf gameplay_exit;
static int gameplay_exit_active;
static uint32 menu_scene_context = 0xffffffff;

static struct
{
    uint64 field;
    int active, paused, hd;
} frame_clock;

void frame_clock_start(void)
{
    frame_clock.field = xport_frame();
    frame_clock.active = 1;
    frame_clock.paused = 0;
    frame_clock.hd = xport_hd;
    hd_history_reset();
}

void frame_clock_stop(void)
{
    frame_clock.active = 0;
}

static float frame_clock_sample(uint64 *field)
{
    float phase = xport_frame_fract(field);
    if (frame_clock.paused || frame_clock.hd != xport_hd || *field < frame_clock.field)
    {
        frame_clock.field = *field;
        frame_clock.paused = 0;
        frame_clock.hd = xport_hd;
        hd_history_reset();
    }
    else if (*field - frame_clock.field > 16)
    {
        /* retain at most eight pending steps after a long host stall */
        frame_clock.field = *field - 16;
        hd_history_reset();
    }
    return phase;
}

sint32 frame_update(void)
{
    uint64 field;
    sint32 result;
    /* service SDK events without an automatic software presentation */
    gpu_frame_begin();
    psx_native_poll_events();
    gpu_frame_end();
    xport_poll();
    if (xport_isquit())
    {
        frame_clock_stop();
        longjmp(menu_exit, 1);
    }
    float phase = frame_clock_sample(&field);
    for (uint32 steps = 0; steps < 8 && field - frame_clock.field >= 2; steps++)
    {
        frame_clock.field += 2;
        result = game_update_frame_prefix();
        if (result)
            return result;
        result = level_transition_update();
        if (result)
            return result;
        phase = frame_clock_sample(&field);
    }
    phase = ((float)(field - frame_clock.field) + phase) * 0.5f;
    hd_present(phase > 1 ? 1 : phase);
    return 0;
}

enum
{
    FRONTEND_GAME_READY = 2
};

static void return_to_frontend(void)
{
    frame_clock_stop();
    if (gameplay_exit_active)
        longjmp(gameplay_exit, 1);
    longjmp(menu_exit, 2);
}

static void poll_host_pads(void)
{
    uint32 pad0 = PadRead(0), pad1 = PadRead(1);
    if (xport_isquit())
    {
        frame_clock_stop();
        longjmp(menu_exit, 1);
    }
    pad_host_publish(2, ((pad0 & 255) << 8) | ((pad0 >> 8) & 255), ((pad1 & 255) << 8) | ((pad1 >> 8) & 255));
}

void pause_refresh_pad(void)
{
    /* service paused fields now so SDK audio and callbacks cannot accumulate */
    gpu_frame_begin();
    psx_native_poll_events();
    gpu_frame_end();
    xport_poll();
    poll_host_pads();
}

static void title_host_frame(void)
{
    poll_host_pads();
}

static void run_high_scores(uint32 score0, uint32 score1)
{
    void (*frame)(void) = game_services.frontend_frame;
    /* gameplay menus need the same host pad publisher as the frontend */
    game_services.frontend_frame = title_host_frame;
    high_scores_run(score0, score1);
    game_services.frontend_frame = frame;
}

uint32 frame_prepare(void)
{
    uint32 parity;
    DISPENV *disp;
    PSX_RECT effects_rect = {0, 499, TEXTURE_PAL_COLORS, 1};
    FUNCTION_MARKER(0x80058634, "GAME.EXE");
    poll_host_pads();
    hd_frame_begin(1);
    DrawSync(0);
    if (LoadImagePSX(&effects_rect, tex_pals.words + TEXTURE_PAL_EFFECTS * TEXTURE_PAL_WORDS) != 0)
    {
        frame_clock_stop();
        longjmp(menu_exit, 1);
    }
    DrawSync(0);
    PSX_RECT *strip_rect = &tex_upload_rects.terrain_strip;
    if (strip_rect->w < 0 || strip_rect->h < 0 || strip_rect->w * (uint32)strip_rect->h > sizeof(background_img.pix) / sizeof(uint16) || LoadImagePSX(strip_rect, background_img.words) != 0)
    {
        frame_clock_stop();
        longjmp(menu_exit, 1);
    }
    cd_music_poll();
    render_ot_compact();
    if (!frame_clock.active)
        hd_vsync();
    if (frame_clock.active && game_pause_flags)
        frame_clock.paused = 1;
    parity = disp_buffers.frame & 1;
    disp = disp_buffers.env + parity;
    PutDispEnv(disp);
    render_buffers_select();
    parity = disp_update_offsets();
    gpu_begin();
    gpu_draw_env(0x800b8938 + 92 * parity, 320 * parity, 0);
    DrawOTag((uint32 *)psx_addr(render_buffers.prev_ot_addr, sizeof(uint32)));
    gpu_display_offset(disp->screen.x, disp->screen.y);
    hd_collect_seal(disp->screen.x, disp->screen.y);
    hd_frame_end(frame_clock.active && !game_pause_flags && !frame_clock.paused ? HD_PRESENT_DEFERRED : 1, parity);
    render_ot_init(render_buffers.ot_addr);
    hd_collect_begin((float)(xport_gte_read_control(26) & 65535u), (sint32)xport_gte_read_control(24) / 65536.0f, (sint32)xport_gte_read_control(25) / 65536.0f, disp->disp.w, disp->disp.h);
    return frame_draw_scene();
}

static void sequence_prepare_frame(void)
{
    frame_prepare();
}

static int gameplay_continue(int result)
{
    uint32 stage, score0, score1, continues, players;
    sint32 branch;
    disp_buffers.shake_ticks = 0;
    loading_clear();
    if (result == 2)
    {
        stage = game_session.stage;
        switch (stage)
        {
            case 5:
                sequence_run(1, sequence_prepare_frame);
                game_session.stage = 6;
                break;
            case 17:
                sequence_run(5, sequence_prepare_frame);
                game_session.stage = 22;
                break;
            case 18:
                sequence_run(4, sequence_prepare_frame);
                game_session.stage = 22;
                break;
            case 6:
                branch = level_transition.route_choice;
                if (branch == 0)
                    game_session.stage = 10;
                if (branch == 1)
                    game_session.stage = 8;
                if (branch == 2)
                    game_session.stage = 7;
                break;
            case 7:
            case 9:
            case 11:
                game_session.stage = 12;
                break;
            case 14:
                game_session.stage = level_transition.route_choice ? 19 : 15;
                break;
            case 15:
                game_session.stage = 20;
                break;
            case 16:
                game_session.stage = 21;
                break;
            default:
                stage = game_session.stage;
                switch (stage)
                {
                    case 19:
                        game_session.stage = 16;
                        break;
                    case 20:
                        sequence_run(3, sequence_prepare_frame);
                        game_session.stage = 17;
                        break;
                    case 21:
                        sequence_run(2, sequence_prepare_frame);
                        game_session.stage = 18;
                        break;
                    case 24:
                        sequence_run(6, sequence_prepare_frame);
                        sequence_run(7, sequence_prepare_frame);
                        score0 = player_scores[0].val;
                        score1 = player_scores[1].val;
                        run_high_scores(score0, score1);
                        game_session.stage = 25;
                        card_dialog.settings_only = 0;
                        card_save_dialog();
                        return 1;
                    default:
                        game_session.stage = stage + 1;
                        break;
                }
                break;
        }
        stage = game_session.stage;
        if (stage == 5 || stage - 7 < 2 || stage == 10 || stage == 14 || stage == 22)
        {
            card_dialog.settings_only = 0;
            card_save_dialog();
        }
    }
    if (result == 4)
    {
        if (level_transition.continue_timer == 329)
        {
            score0 = player_scores[0].val;
            score1 = player_scores[1].val;
            run_high_scores(score0, score1);
            return 1;
        }
        actors_reset_health();
        continues = level_transition.continues;
        players = game_session.player_count;
        level_transition.continues = continues - players;
    }
    return result == 1;
}

static int run_native_levels(void)
{
    int result;
    for (;;)
    {
        if (game_session.stage == 25)
        {
            sequence_run(6, sequence_prepare_frame);
            sequence_run(7, sequence_prepare_frame);
            return 1;
        }
        if (setjmp(gameplay_exit))
        {
            gameplay_exit_active = 0;
            return 1;
        }
        gameplay_exit_active = 1;
        result = level_run();
        gameplay_exit_active = 0;
        if (gameplay_continue(result))
            return 1;
    }
}

static int run_loaded_game(int sequence)
{
    if (sequence)
        sequence_run(0, sequence_prepare_frame);
    return run_native_levels();
}

static void game_initialize(void)
{
    /* Original nullsub_47 is only JR RA/NOP */
    depth_table_build();
    memset(item_corrections, 0, sizeof(item_corrections));
    game_tables_init();
    game_tables_init_vehicles();
    game_tables_init_extra();
    disp_initialize();
    render_buffers_select();
    localization_load();
    card_backend_read(0, NULL, 0);
    pad_refresh_ports();
}

int game_run(void)
{
    FUNCTION_MARKER(0x80015c6c, "GAME.EXE");
    gameplay_exit_active = 0;
    game_initialize();
    for (;;)
    {
        if (setjmp(menu_exit) == 1)
            return 9;
        sound_voices_stop_all();
        disp_buffers.shake_ticks = 0;
        loading_clear();
        game_services.frontend_frame = title_host_frame;
        if (menu_run())
            demo_select_next();
        if (game_session.stage == 0 && demo.cur_idx == 0xffffffff)
            sequence_run(0, sequence_prepare_frame);
        for (;;)
        {
            int result;
            if (game_session.stage == 25)
            {
                sequence_run(6, sequence_prepare_frame);
                sequence_run(7, sequence_prepare_frame);
                break;
            }
            result = level_run();
            if (gameplay_continue(result))
                break;
        }
    }
}

static int finish_game_to_menu(void)
{
    int result;
    sound_voices_stop_all();
    disp_buffers.shake_ticks = 0;
    loading_clear();
    result = menu_run() ? 1 : FRONTEND_GAME_READY;
    game_services.frontend_frame = NULL;
    return result;
}

static int frontend_run_impl(void)
{
    int result = 0;
    menu_scene_context = 0xffffffff;
    if (!game_cold_reset())
        return 2;
    gpu_init_empty();
    sound_core_init();
    xport_set_headless(0);
    xport_input_override(0, 0);
    ResetGraph(0);
    InitGeom();
    if (!xport_audio_init())
    {
        result = 5;
        goto done;
    }
#if !defined(AP_WEB)
    movie_play_intro();
    if (xport_isquit())
        goto done;
    /* Reset native owners after the original loader's movie handoff */
    if (!game_cold_reset())
    {
        result = 2;
        goto done;
    }
    gpu_init_empty();
    sound_core_init();
    ResetCallbackPSX();
    ResetGraph(0);
    InitGeom();
#endif
    game_services.return_to_frontend = return_to_frontend;
    menu.callback = &menu_callback_main;
    switch (setjmp(menu_exit))
    {
        case 0:
            break;
        case 2:
            gameplay_exit_active = 0;
            game_services.frontend_frame = title_host_frame;
            result = finish_game_to_menu();
            goto frontend_ready;
        default:
            gameplay_exit_active = 0;
            result = 9;
            goto done;
    }
    result = game_run();
    goto done;
frontend_ready:
    while (!xport_isquit())
    {
        int cleanup;
        if (result == FRONTEND_GAME_READY)
            goto start_game;
        if (result == 1)
        {
            demo_select_next();
            goto start_game;
        }
        menu_prepare_frame();
        cleanup = menu_update_selection_frame(&menu_scene_context);
        if (cleanup)
        {
            pad_stream_reset();
            sound_voices_stop_all();
        start_game:
            game_services.frontend_frame = title_host_frame;
            result = run_loaded_game(game_session.stage == 0 && demo.cur_idx == 0xffffffffu);
            if (result == 1 || result == 4)
            {
                result = finish_game_to_menu();
                if (result != 0 && result != 1 && result != FRONTEND_GAME_READY)
                    break;
                continue;
            }
            break;
        }
    }
done:
    frame_clock_stop();
    game_services.frontend_frame = NULL;
    xport_audio_shutdown();
    return result;
}

int frontend_run(void)
{
    int result = frontend_run_impl();
    /* A host quit is not a failed game initialization */
    return result == 9 && xport_isquit() ? 0 : result;
}
