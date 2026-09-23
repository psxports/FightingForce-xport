/* Native integration of audited callbacks. WIP: fixture bootstrap and missing
 * outer stage/scene tail. Unsupported callbacks stop explicitly. */
#include "ff.h"
#include "ff_gpu.h"
#include "ff_audio.h"
#include "audio_waveout.h"
#include "platform_dummy.h"
#include <stdio.h>
#include <string.h>
#include <setjmp.h>
#include <stdlib.h>
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
static jmp_buf menu_exit;
#include "diagnostic_state.h"
static uint32 diag_segments[36000][3], diag_decode_segments[36000][3], diag_count, diag_decode_count, diag_active, diag_title, diag_enabled, diag_end_tick = 900;
static uint16 diag_pad = 65535, diag_decode_pad = 65535;
static FILE *diag_inputs;
static FILE *diag_sound;
static FILE *diag_phases;
static int diag_phase_failed;
static uint32 diag_phase_count, diag_phase_limit;
static FILE *diag_input_calls;
static uint32 diag_input_ordinal;

typedef struct DIAG_INPUT_HISTORY
{
    uint32 ordinal, tick, controller, expected_tick, expected_controller, scope;
} DIAG_INPUT_HISTORY;

static DIAG_INPUT_HISTORY diag_input_history[16];
static uint32 diag_input_history_count, diag_input_scope;
static uint32 diag_connected = 1;
static uint32 diag_phase_base, diag_sequence_stage, diag_resume_pc, diag_menu_jobs;
static uint32 diag_highscore_saved[2];
static const char *diag_phase_directory;
static uint32 diag_phase_interval = 300;
static int diagnostic_checkpoint(const char *path, int load);
static int diagnostic_phase_checkpoint(uint32 pc);

static int diagnostic_prime_input_packets(void)
{
    uint32 expected[2], tick = ff_u32(0x80093dd0);
    uint8 packet[34];
    long position;
    if (!diag_input_calls)
        return 1;
    position = ftell(diag_input_calls);
    if (position < 0)
        return 0;
    while (fread(expected, sizeof(expected), 1, diag_input_calls) == 1)
    {
        if (fread(packet, sizeof(packet), 1, diag_input_calls) != 1 || expected[1] > 1)
            return 0;
        if (expected[0] != tick)
            break;
        memcpy(ff_ptr(expected[1] ? 0x80094770 : 0x80094748, 34), packet, 34);
    }
    clearerr(diag_input_calls);
    return fseek(diag_input_calls, position, SEEK_SET) == 0;
}

void ff_audit_sequence_context(uint32 old_stage)
{
    diag_sequence_stage = old_stage;
}

void ff_audit_highscore_context(const uint8 *saved)
{
    memcpy(diag_highscore_saved, saved, 8);
}

static int diagnostic_phase_gpu(uint32 pc)
{
    uint32 *payload = (uint32 *)malloc(0x200000), used = 1, p = ff_u32(0x8008d4b4) & 0x1fffff, guard = 0, tag, words, header[3];
    if (!payload)
        return 0;
    payload[0] = pc;
    while (p != 0x10018)
    {
        if (p >= 0x200000 || (p & 3) || ++guard > 131072)
        {
            free(payload);
            return 0;
        }
        tag = ff_u32(p);
        words = tag >> 24;
        if (p + 4 + words * 4 > 0x200000 || used + words + 2 > 0x80000)
        {
            free(payload);
            return 0;
        }
        if (words)
        {
            payload[used++] = words;
            memcpy(payload + used, ff_ptr(p + 4, words * 4), words * 4);
            used += words;
        }
        p = tag & 0xffffff;
    }
    payload[used++] = 0;
    header[0] = 17;
    header[1] = ff_u32(0x80093dd0);
    header[2] = used * 4;
    if (fwrite(header, sizeof(header), 1, diag_phases) != 1 || fwrite(payload, used * 4, 1, diag_phases) != 1)
    {
        free(payload);
        return 0;
    }
    free(payload);
    return 1;
}

void ff_audit_input_packet(sint32 controller)
{
    uint32 expected[2] = {0xffffffffu, 0xffffffffu}, i, count;
    uint8 packet[34];
    DIAG_INPUT_HISTORY *entry;
    if (!diag_input_calls)
        return;
    entry = &diag_input_history[diag_input_history_count++ & 15u];
    entry->ordinal = diag_input_ordinal;
    entry->tick = ff_u32(0x80093dd0);
    entry->controller = (uint32)controller;
    entry->expected_tick = expected[0];
    entry->expected_controller = expected[1];
    entry->scope = diag_input_scope;
    if (fread(expected, sizeof(expected), 1, diag_input_calls) != 1 || fread(packet, sizeof(packet), 1, diag_input_calls) != 1 || expected[0] != entry->tick || expected[1] != (uint32)controller || controller < 0 || controller > 1)
    {
        entry->expected_tick = expected[0];
        entry->expected_controller = expected[1];
        fprintf(stderr, "Diagnostic input call mismatch at ordinal %u tick %u controller %d expected_tick %u expected_controller %u scope %u\n", diag_input_ordinal, entry->tick, controller, expected[0], expected[1], diag_input_scope);
        count = diag_input_history_count < 16u ? diag_input_history_count : 16u;
        for (i = 0; i < count; i++)
        {
            entry = &diag_input_history[(diag_input_history_count - count + i) & 15u];
            fprintf(stderr, "Diagnostic input history ordinal %u tick %u controller %u expected_tick %u expected_controller %u scope %u\n", entry->ordinal, entry->tick, entry->controller, entry->expected_tick, entry->expected_controller, entry->scope);
        }
        longjmp(menu_exit, 1);
    }
    entry->expected_tick = expected[0];
    entry->expected_controller = expected[1];
    memcpy(ff_ptr(controller ? 0x80094770 : 0x80094748, 34), packet, 34);
    diag_input_ordinal++;
}

int ff_audit_phase_boundary(uint32 pc)
{
    uint32 header[3] = {15, ff_u32(0x80093dd0), 2228};
    uint32 context[8];
    if (!diag_phases)
        return 0;
    if (!diagnostic_prime_input_packets())
    {
        diag_phase_failed = 1;
        longjmp(menu_exit, 1);
    }
    context[0] = pc;
    context[1] = (uint32)VSync(-1);
    context[2] = 0;
    context[3] = 0;
    context[4] = ff_u32(0x80093d58);
    context[5] = ff_u32(0x800927ec);
    context[6] = ff_u32(0x80094748);
    context[7] = ff_u32(0x80094770);
    if (diag_phase_directory && (diag_phase_count == 0 || (diag_phase_base + diag_phase_count) % diag_phase_interval == 0))
    {
        if (!diagnostic_phase_checkpoint(pc))
        {
            diag_phase_failed = 1;
            longjmp(menu_exit, 1);
        }
    }
    if (fwrite(header, sizeof(header), 1, diag_phases) != 1 || fwrite(context, sizeof(context), 1, diag_phases) != 1 || fwrite(ff_ptr(0x8009272c, 212), 212, 1, diag_phases) != 1 || fwrite(ff_ptr(0x800b4318, 1952), 1952, 1, diag_phases) != 1 || fwrite(ff_ptr(0x8008d490, 32), 32, 1, diag_phases) != 1)
        diag_phase_failed = 1;
    if (!diagnostic_phase_gpu(pc))
        diag_phase_failed = 1;
    ++diag_phase_count;
    return diag_phase_limit && diag_phase_count == diag_phase_limit;
}

static int diag_input_failed;

static void diagnostic_sound(uint32 address, uint32 a, uint32 b, uint32 c)
{
    uint32 record[5] = {address, ff_u32(0x80093dd0), a, b, c};
    if (diag_sound && fwrite(record, sizeof(record), 1, diag_sound) != 1)
        diag_input_failed = 1;
    if (diag_phases && diag_phase_count)
    {
        uint32 payload[4] = {address, a, b, c}, length = address == 0x800567f8 ? 8u : (address == 0x80056f08 ? 12u : 16u);
        uint32 header[3] = {4, record[1], length};
        if (fwrite(header, sizeof(header), 1, diag_phases) != 1 || fwrite(payload, length, 1, diag_phases) != 1)
            diag_phase_failed = 1;
    }
}

static LONGLONG diag_gpu_ticks;

static void diagnostic_input(uint32 event)
{
    uint32 i, lo, hi, tick = ff_u32(0x80093dd0);
    if (event == 1)
        diag_input_scope = 1;
    if (event == 4)
        diag_input_scope = 0;
    if (event == 4 && diag_inputs)
    {
        uint32 record[3] = {tick, (uint32)(diag_decode_pad ^ 65535u), ff_u32(0x80093a10) & 65535u};
        if (fwrite(record, sizeof(record), 1, diag_inputs) != 1)
            diag_input_failed = 1;
    }
    if (event == 3)
        diag_title = 0;
    if (event == 2 && diag_title)
        ff_w16(0x8009474a, tick == 2 ? 0xfff7 : 0xffff);
    if (event == 1)
    {
        diag_active = 1;
        diag_pad = 65535;
        lo = 0;
        hi = diag_count;
        while (lo < hi)
        {
            i = lo + (hi - lo) / 2;
            if (diag_segments[i][0] <= tick)
                lo = i + 1;
            else
                hi = i;
        }
        if (lo && tick <= diag_segments[lo - 1][1])
            diag_pad = (uint16)~diag_segments[lo - 1][2];
        diag_decode_pad = diag_pad;
        lo = 0;
        hi = diag_decode_count;
        while (lo < hi)
        {
            i = lo + (hi - lo) / 2;
            if (diag_decode_segments[i][0] <= tick)
                lo = i + 1;
            else
                hi = i;
        }
        if (lo && tick <= diag_decode_segments[lo - 1][1])
            diag_decode_pad = (uint16)~diag_decode_segments[lo - 1][2];
    }
    if (diag_active && (event == 0 || event == 1))
    {
        ff_w16(0x8009474a, event == 0 ? diag_decode_pad : diag_pad);
    }
}

static int diagnostic_load_pad_schedule(const char *path, uint32 segments[36000][3], uint32 *count)
{
    FILE *file;
    uint32 i;
    if (!path)
    {
        *count = 0;
        return 1;
    }
    file = fopen(path, "rb");
    if (!file)
        return 0;
    *count = (uint32)fread(segments, 1, sizeof(diag_segments), file);
    if (*count % 12 || fgetc(file) != EOF || ferror(file))
    {
        fclose(file);
        return 0;
    }
    fclose(file);
    *count /= 12;
    for (i = 0; i < *count; i++)
        if (segments[i][0] > segments[i][1] || segments[i][1] >= diag_end_tick || segments[i][2] > 65535 || (i && segments[i][0] <= segments[i - 1][1]))
            return 0;
    return 1;
}

static int diagnostic_init(void)
{
    const char *path = getenv("FF_AUDIT_PAD_SCHEDULE"), *decode_path = getenv("FF_AUDIT_DECODE_PAD_SCHEDULE"), *end = getenv("FF_AUDIT_END_TICK");
    char *tail;
    diag_enabled = 0;
    ff_services.audit_input_event = NULL;
    if (!path)
        return 1;
    diag_end_tick = 900;
    if (end)
    {
        unsigned long value = strtoul(end, &tail, 10);
        if (!*end || *tail || value == 0 || value > 1000000)
            return 0;
        diag_end_tick = (uint32)value;
    }
    if (!diagnostic_load_pad_schedule(path, diag_segments, &diag_count) || !diagnostic_load_pad_schedule(decode_path, diag_decode_segments, &diag_decode_count))
        return 0;
    diag_enabled = 1;
    diag_title = 1;
    diag_active = 0;
    diag_pad = 65535;
    diag_decode_pad = 65535;
    ff_services.audit_input_event = diagnostic_input;
    return 1;
}

static int diagnostic_checkpoint(const char *path, int load)
{
    static const char checkpoint_abi[64] = "FightingForce native checkpoint ABI 1";
    FILE *f;
    int ok;
    uint32 magic = 0x31434646;
    char actual[64];
    /* Stop the output worker before snapshotting RAM and SPU together. */
    waveout_shutdown();
    f = fopen(path, load ? "rb" : "wb");
    if (!f)
        return 0;
    if (load)
    {
        ok = fread(&magic, 4, 1, f) == 1 && magic == 0x31434646 && fread(actual, 1, 64, f) == 64 && !memcmp(checkpoint_abi, actual, 64);
    }
    else
        ok = fwrite(&magic, 4, 1, f) == 1 && fwrite(checkpoint_abi, 1, 64, f) == 64;
    ok = ok && ff_state_block(f, ff_ram, sizeof(ff_ram), load) && ff_state_block(f, ff_ptr(0x1f800000, 1024), 1024, load) && psx_state_io(f, load) && ff_gpu_state_io(f, load) && ff_audio_state_io(f, load) && FF_STATE(f, ff_dummy_calls, load) && FF_STATE(f, ff_dummy_card_pending, load) && FF_STATE(f, ff_sdk_interrupts_enabled, load) && FF_STATE(f, ff_host_pad_state, load) && FF_STATE(f, ff_sdk_counter_state, load);
    if (load && fgetc(f) != EOF)
        ok = 0;
    if (fclose(f))
        ok = 0;
    if (!waveout_init())
        ok = 0;
    return ok;
}

static const char *diagnostic_output(void)
{
    const char *p = diag_enabled ? getenv("FF_AUDIT_OUTPUT") : NULL;
    return p ? p : "../status/gameplay/frontend-game-loop-final.ram";
}

static int diagnostic_phase_checkpoint(uint32 pc)
{
    char path[1024], context_path[1040];
    FILE *file;
    int length;
    uint32 context[12] = {0x32504346, pc, diag_phase_base + diag_phase_count, diag_input_ordinal, diag_connected, diag_sequence_stage, diag_active, diag_title, diag_pad, diag_menu_jobs - (pc == 0x8004fef4 ? 1u : 0u), diag_highscore_saved[0], diag_highscore_saved[1]};
    length = snprintf(path, sizeof(path), "%s/phase-%010u.ffcp", diag_phase_directory, context[2]);
    if (length < 0 || (size_t)length >= sizeof(path))
        return 0;
    snprintf(context_path, sizeof(context_path), "%s.ctx", path);
    file = fopen(path, "rb");
    if (file)
    {
        fclose(file);
        return 0;
    }
    file = fopen(context_path, "rb");
    if (file)
    {
        fclose(file);
        return 0;
    }
    if (!diagnostic_checkpoint(path, 0))
        return 0;
    file = fopen(context_path, "wbx");
    if (!file)
        return 0;
    length = fwrite(context, sizeof(context), 1, file) == 1;
    if (fclose(file))
        length = 0;
    return length;
}

static int diagnostic_phase_restore(const char *path, FILE *menu_input)
{
    char context_path[1040];
    uint32 context[12];
    FILE *file;
    int length;
    length = snprintf(context_path, sizeof(context_path), "%s.ctx", path);
    if (length < 0 || (size_t)length >= sizeof(context_path))
        return 0;
    file = fopen(context_path, "rb");
    if (!file)
        return 0;
    length = fread(context, sizeof(context), 1, file) == 1 && fgetc(file) == EOF && !ferror(file);
    if (fclose(file))
        length = 0;
    if (!length || context[0] != 0x32504346 || (context[1] != 0x8004fef4 && context[1] != 0x8001559c && context[1] != 0x80014df8 && context[1] != 0x8006acf4) || context[2] > 36000 || context[3] > 1000000 || context[4] > 2 || context[6] > 1 || context[7] > 1 || context[8] > 65535 || context[9] > 36000)
        return 0;
    if (!diag_input_calls || !menu_input || fseek(diag_input_calls, 4 + 42 * (long)context[3], SEEK_SET) || fseek(menu_input, 12 * (long)context[9], SEEK_SET))
        return 0;
    if (!diagnostic_checkpoint(path, 1))
        return 0;
    if ((context[1] == 0x8004fef4 && ff_u32(0x80093d58) != 26) || (context[1] == 0x8001559c && ff_u32(0x80093d58) != 27))
        return 0;
    diag_resume_pc = context[1];
    diag_phase_base = context[2];
    diag_input_ordinal = context[3];
    diag_connected = context[4];
    diag_sequence_stage = context[5];
    diag_active = context[6];
    diag_title = context[7];
    diag_pad = (uint16)context[8];
    diag_decode_pad = diag_pad;
    diag_menu_jobs = context[9];
    diag_highscore_saved[0] = context[10];
    diag_highscore_saved[1] = context[11];
    return 1;
}

/* Physical CD/card services remain dummy; route game-side menu logic through
 * the translated entries so their markers and UI branches are retained. */
static sint32 menu_aux(uint32 function, sint32 a, sint32 b)
{
    if (function == 0x800569bc)
        return FUN_800569BC(a);
    if (function == 0x80052428)
        return FUN_80052428();
    if (function == 0x80052458)
        return FUN_80052458();
    if (function == 0x80052484)
        return FUN_80052484();
    fprintf(stderr, "WIP: untranslated menu service %08X\n", function);
    longjmp(menu_exit, 1);
}

static void exit_menu(uint32 buffer, sint32 value)
{
    fprintf(stderr, "WIP: original longjmp %08X (%d), startup continuation pending.\n", buffer, value);
    longjmp(menu_exit, 1);
}

extern void ff_audit_end(void);

/* Native equivalent of asynchronous PAD refresh in the original busy wait. */
GDB_CALL void ff_pause_refresh_pad(void)
{
    uint32 pad = PadRead(0), buttons = ((pad & 255) << 8) | ((pad >> 8) & 255);
    if (psx_quit_requested())
        longjmp(menu_exit, 1);
    ff_host_pad_publish(diag_enabled ? diag_connected : 1, buttons, 0);
}

static void title_host_frame(void)
{
    uint32 i, pad = PadRead(0), buttons = ((pad & 255) << 8) | ((pad >> 8) & 255);
    if (psx_quit_requested())
        longjmp(menu_exit, 1);
    ff_host_pad_publish(diag_enabled ? diag_connected : 1, buttons, 0);
    VSync(0);
}

static uint32 prepare_host_frame(int gameplay)
{
    LARGE_INTEGER gpu_begin, gpu_end;
    uint32 i, pad = PadRead(0), buttons = ((pad & 255) << 8) | ((pad >> 8) & 255), parity, display;
    if (psx_quit_requested())
        longjmp(menu_exit, 1);
    ff_host_pad_publish(diag_enabled ? diag_connected : 1, buttons, 0);
    if (!ff_gpu_load_clut(0x800b50b8, 0, 499) || !ff_gpu_load_image(0x80093568, 0x800a1990))
        longjmp(menu_exit, 1);
    FUN_80067820();
    FUN_80011D9C();
    VSync(2);
    parity = ff_u32(0x8008d4c4) & 1;
    display = 0x800947a0 + 20 * parity;
    FUN_80064684();
    ff_display_offsets_800586D0();
    ff_gpu_begin();
    ff_gpu_draw_env(0x800b8938 + 92 * parity, 320 * parity, 0);
    if (diag_enabled)
        QueryPerformanceCounter(&gpu_begin);
    if (ff_gpu_ot(ff_u32(0x8008d4b8)) < 0)
        longjmp(menu_exit, 1);
    ff_gpu_display_offset(ff_s16(display + 8), ff_s16(display + 10));
    ff_gpu_present();
    FUN_80011D50(ff_u32(0x8008d4b4));
    if (diag_enabled)
    {
        QueryPerformanceCounter(&gpu_end);
        diag_gpu_ticks += gpu_end.QuadPart - gpu_begin.QuadPart;
    }
    if (gameplay == 1)
        return ff_prepare_scene_800587A8_stage0();
    if ((sint32)ff_u32(0x80093dd4) >= 192)
        FUN_80061ADC(0x80093570, 110);
    if (ff_s16(0x8001000c) + ff_s16(0x8001000e) + ff_s16(0x80010010))
        FUN_8006471C();
    if (ff_u32(0x80094190))
        FUN_80064820();
    ff_menu_horizon_80058844();
    ff_draw_stage_background();
    if ((sint32)ff_u32(0x80093dd0) < 33)
    {
        sint32 fade = (sint32)((ff_u32(0x80093dd0) << 3) - 256u);
        FUN_80011CCC(fade, fade, fade);
    }
    return 0;
}

static void sequence_prepare_frame(void)
{
    prepare_host_frame(0);
}

GDB_CALL uint32 ff_game_prepare_80058634_stage0(void)
{
    return prepare_host_frame(1);
}

GDB_CALL uint32 ff_menu_prepare_80058634_stage26(void)
{
    return prepare_host_frame(2);
}

int ff_game_prefix_preview(void)
{
    PSX_CONFIG config;
    FILE *f;
    int result = 9;
    if (!ff_load_ram("../status/gameplay/sequence-level-entry.ram"))
        return 2;
    f = fopen("../status/gameplay/sequence-level-entry.scratch", "rb");
    if (!f)
        return 3;
    if (fread(ff_ptr(0x1f800000, 1024), 1, 1024, f) != 1024)
    {
        fclose(f);
        return 4;
    }
    fclose(f);
    ff_gpu_init_empty();
    ff_audio_init_empty();
    memset(&config, 0, sizeof(config));
    config.window_title = "Fighting Force gameplay audit";
    config.window_width = 960;
    config.window_height = 720;
    config.refresh_rate = 60;
    config.headless = 1;
    psx_configure(&config);
    ResetGraph(0);
    InitGeom();
    if (!waveout_init())
        return 5;
    ff_services.game_longjmp = exit_menu;
    ff_services.menu_aux = menu_aux;
    ff_services.frontend_frame = title_host_frame;
    if (!setjmp(menu_exit))
    {
        ff_level_init_80014A34_stage0();
        ff_level_ready_80014B70_stage0();
        result = ff_game_frame_prefix_80014DF8_stage0();
    }
    ff_services.frontend_frame = NULL;
    waveout_shutdown();
    printf("game_prefix_result %d tick %u\n", result, ff_u32(0x80093dd0));
    ff_audit_end();
    return result;
}

/* Native continuation, no RAM/VRAM/SPU fixture loads or host reset. */
static int save_actor_boundary(FILE *f)
{
    return fwrite(ff_ptr(0x80093dd0, 4), 1, 4, f) == 4 && fwrite(ff_ptr(0x800b4318, 1952), 1, 1952, f) == 1952 && fwrite(ff_ptr(0x8008d490, 32), 1, 32, f) == 32;
}

/* Optional full memory evidence for a narrow diagnostic interval */
static int save_detail_boundary(FILE *f, uint32 begin, uint32 end)
{
    uint32 tick = ff_u32(0x80093dd0);
    if (!f || tick < begin || tick > end)
        return 1;
    return fwrite(&tick, 4, 1, f) == 1 && fwrite(ff_ptr(0x80000000, 0x200000), 1, 0x200000, f) == 0x200000 && fwrite(ff_ptr(0x1f800000, 1024), 1, 1024, f) == 1024;
}

/* Evidence stream: tick,scene count,world pools/script/RNG, then OT payloads.
 * Skip empty links and addresses, retain every GPU command/payload word. */
static int save_world_boundary(FILE *f)
{
    uint32 count = ff_u32(0x8009403c), p = ff_u32(0x8008d4b4) & 0xffffff, tag, n, guard = 0, zero = 0;
    if (fwrite(ff_ptr(0x80093dd0, 4), 4, 1, f) != 1 || fwrite(&count, 4, 1, f) != 1)
        return 0;
    if (fwrite(ff_ptr(0x8009a3c8, 2304), 1, 2304, f) != 2304 || fwrite(ff_ptr(0x8009d1d0, 4608), 1, 4608, f) != 4608)
        return 0;
    if (fwrite(ff_ptr(0x800b89f0, 36 * count), 36, count, f) != count || fwrite(ff_ptr(0x800bcde0, 6144), 1, 6144, f) != 6144)
        return 0;
    if (fwrite(ff_ptr(0x800946b8, 4), 4, 1, f) != 1 || fwrite(ff_ptr(0x80094e88, 4), 4, 1, f) != 1)
        return 0;
    while (p != 0x10018)
    {
        if (++guard > 100000)
            return 0;
        tag = ff_u32(p);
        n = tag >> 24;
        if (n && (fwrite(&n, 4, 1, f) != 1 || fwrite(ff_ptr(p + 4, 4 * n), 4, n, f) != n))
            return 0;
        p = tag & 0xffffff;
    }
    return fwrite(&zero, 4, 1, f) == 1;
}

static int run_loaded_game(int sequence, uint32 limit, const char *output)
{
    FILE *f, *actors = NULL, *world = NULL, *detail = NULL;
    char actor_path[260];
    int result = 9;
    uint32 frame, vblank_start, audio_start, nonzero_start, detail_from = 0, detail_to = 0;
    LARGE_INTEGER started, finished, frequency;
    int phase_aligned = diag_enabled && getenv("FF_AUDIT_PHASE_ALIGNED") && strcmp(getenv("FF_AUDIT_PHASE_ALIGNED"), "1") == 0;
    const char *resume = diag_enabled ? getenv("FF_CHECKPOINT_LOAD") : NULL, *checkpoint = diag_enabled ? getenv("FF_CHECKPOINT_SAVE") : NULL;
    const char *checkpoint_tick_text = diag_enabled ? getenv("FF_CHECKPOINT_TICK") : NULL;
    uint32 checkpoint_tick = checkpoint_tick_text ? (uint32)strtoul(checkpoint_tick_text, NULL, 10) : 0;
    const char *checkpoint_directory = diag_enabled ? getenv("FF_CHECKPOINT_DIRECTORY") : NULL;
    const char *interval_text = getenv("FF_CHECKPOINT_INTERVAL");
    uint32 checkpoint_interval = 300, next_checkpoint = 0;
    int stage_soak = diag_enabled && getenv("FF_STAGE_SOAK") && !strcmp(getenv("FF_STAGE_SOAK"), "1");
    char periodic_path[1024], *interval_end;
    if (checkpoint_directory && interval_text)
    {
        unsigned long interval = strtoul(interval_text, &interval_end, 10);
        if (!*interval_text || *interval_end || !interval || interval > 18000)
            return 15;
        checkpoint_interval = (uint32)interval;
    }
    int restored_game = diag_resume_pc == 0x80014df8;
    LONGLONG prefix_ticks = 0, tail_ticks = 0, record_ticks = 0;
    LARGE_INTEGER p0, p1, p2, p3;
    if (diag_resume_pc == 0x8001559c)
    {
        diag_resume_pc = 0;
        result = ff_sequence_resume_8001559C_stage27(sequence_prepare_frame, diag_sequence_stage);
        if (diag_phase_limit && diag_phase_count == diag_phase_limit)
            return 0;
        sequence = 0;
    }
    if (restored_game)
    {
        diag_resume_pc = 0;
        sequence = 0;
    }
    if (sequence && !resume)
    {
        result = ff_sequence_run_80015720_stage0(sequence_prepare_frame);
        if (diag_phase_limit && diag_phase_count == diag_phase_limit)
            return 0;
        printf("continuous_sequence_result %d tick %u stage %u\n", result, ff_u32(0x80093dd0), ff_u32(0x80093d58));
        fflush(stdout);
        /* 15EC0..15EE8 ignores15720's result, including user skip0. */
        result = 9;
    }
    if (resume)
    {
        if (!diagnostic_checkpoint(resume, 1))
            return 15;
        /* Restore diagnostic pad ownership from the loaded game buffers */
        diag_connected = (uint32)FUN_800573A8();
        diag_pad = (uint16)ff_s16(0x8009474a);
        diag_title = 0;
    }
    else if (!restored_game)
    {
        ff_level_init_80014A34_stage0();
        ff_level_ready_80014B70_stage0();
    }
    next_checkpoint = ff_u32(0x80093dd0);
    if (checkpoint && checkpoint_tick == ff_u32(0x80093dd0) && !diagnostic_checkpoint(checkpoint, 0))
        return 15;
    /* A resumed audit retains absolute game ticks and ends at the same boundary. */
    if ((resume || restored_game) && limit)
    {
        uint32 tick = ff_u32(0x80093dd0);
        if (tick >= limit)
            return 15;
        limit -= tick;
    }
    if (limit)
    {
        snprintf(actor_path, sizeof(actor_path), "%s.actors", output);
        actors = fopen(actor_path, "wb");
        if (!actors)
            return 7;
        if (!phase_aligned && !save_actor_boundary(actors))
        {
            fclose(actors);
            return 8;
        }
    }
    if (limit && getenv("FF_TRACE_WORLD"))
    {
        snprintf(actor_path, sizeof(actor_path), "%s.world", output);
        world = fopen(actor_path, "wb");
        if (!world || (!phase_aligned && !save_world_boundary(world)))
            return 8;
    }
    if (limit && diag_enabled && getenv("FF_DETAIL_FROM") && getenv("FF_DETAIL_TO"))
    {
        detail_from = (uint32)strtoul(getenv("FF_DETAIL_FROM"), NULL, 10);
        detail_to = (uint32)strtoul(getenv("FF_DETAIL_TO"), NULL, 10);
        if (detail_to < detail_from || detail_to - detail_from > 120)
            return 8;
        snprintf(actor_path, sizeof(actor_path), "%s.detail", output);
        detail = fopen(actor_path, "wbx");
        if (!detail || !save_detail_boundary(detail, detail_from, detail_to))
            return 8;
    }
    diag_gpu_ticks = 0;
    QueryPerformanceFrequency(&frequency);
    QueryPerformanceCounter(&started);
    diag_input_failed = 0;
    if (limit && diag_enabled)
    {
        snprintf(actor_path, sizeof(actor_path), "%s.inputs", output);
        diag_inputs = fopen(actor_path, "wb");
        if (!diag_inputs)
            return 7;
        snprintf(actor_path, sizeof(actor_path), "%s.sound", output);
        diag_sound = fopen(actor_path, "wb");
        if (!diag_sound)
        {
            fclose(diag_inputs);
            diag_inputs = NULL;
            return 7;
        }
        ff_audit_sound_event = diagnostic_sound;
    }
    vblank_start = (uint32)VSync(-1);
    audio_start = g_waveout_submitted_buffers;
    nonzero_start = g_waveout_nonzero_buffers;
    for (frame = 0; !limit || frame < limit; frame++)
    {
        if (checkpoint_directory && ff_u32(0x80093dd0) == next_checkpoint)
        {
            FILE *existing;
            if (snprintf(periodic_path, sizeof(periodic_path), "%s/tick-%010u.ffcp", checkpoint_directory, next_checkpoint) < 0)
                return 15;
            existing = fopen(periodic_path, "rb");
            if (existing)
            {
                fclose(existing);
                return 15;
            }
            if (!diagnostic_checkpoint(periodic_path, 0))
                return 15;
            printf("periodic_checkpoint_saved tick %u path %s\n", next_checkpoint, periodic_path);
            next_checkpoint += checkpoint_interval;
        }
        if (checkpoint && checkpoint_tick != 0 && checkpoint_tick == ff_u32(0x80093dd0))
        {
            if (!diagnostic_checkpoint(checkpoint, 0))
                return 15;
            printf("checkpoint_saved tick %u path %s\n", checkpoint_tick, checkpoint);
            if (getenv("FF_CHECKPOINT_STOP"))
                break;
        }
        if (!diag_enabled)
        {
            printf("game_frame_begin %u\n", frame);
            fflush(stdout);
        }
        /* Original PAD polling has published this tick before the game boundary */
        if (diag_enabled)
            diagnostic_input(1);
        QueryPerformanceCounter(&p0);
        /* Capture the incoming stage boundary before executing its first frame */
        if (phase_aligned)
        {
            if (actors && !save_actor_boundary(actors))
            {
                fclose(actors);
                return 8;
            }
            if (world && !save_world_boundary(world))
            {
                fclose(world);
                return 8;
            }
        }
        if (ff_audit_phase_boundary(0x80014df8))
        {
            result = 0;
            break;
        }
        /* Keep idle stage-loading diagnostics out of the game-over frontend */
        if (stage_soak)
            ff_w32(0x80093dd4, 0);
        result = ff_game_frame_prefix_80014DF8_stage0();
        QueryPerformanceCounter(&p1);
        if (!result)
            result = ff_game_frame_tail_80015038_stage0();
        QueryPerformanceCounter(&p2);
        if (diag_input_failed)
        {
            ff_audit_sound_event = NULL;
            fclose(diag_sound);
            diag_sound = NULL;
            fclose(diag_inputs);
            diag_inputs = NULL;
            return 8;
        }
        if (!phase_aligned && actors && !save_actor_boundary(actors))
        {
            fclose(actors);
            return 8;
        }
        if (!phase_aligned && world && !save_world_boundary(world))
        {
            fclose(world);
            return 8;
        }
        if (!save_detail_boundary(detail, detail_from, detail_to))
            return 8;
        QueryPerformanceCounter(&p3);
        prefix_ticks += p1.QuadPart - p0.QuadPart;
        tail_ticks += p2.QuadPart - p1.QuadPart;
        record_ticks += p3.QuadPart - p2.QuadPart;
        if (!diag_enabled)
        {
            printf("game_frame_complete %u tick %u\n", frame, ff_u32(0x80093dd0));
            fflush(stdout);
        }
        if (result == 2 && diag_enabled && ff_u32(0x80093d58) == 0)
        {
            /* 15EF0..1607C: fade the completed stage, advance, then enter the next stage */
            ff_w32(0x800941a8, 0);
            FUN_8005ED78();
            ff_w32(0x80093d58, 1);
            ff_level_init_80014A34_stage0();
            ff_level_ready_80014B70_stage0();
            result = 0;
            if (getenv("FF_TRANSITION_CHECKPOINT_SAVE") && !diagnostic_checkpoint(getenv("FF_TRANSITION_CHECKPOINT_SAVE"), 0))
                return 15;
        }
        else if (result)
        {
            ++frame;
            break;
        }
        if (frame == 299)
            ff_gpu_save_frame("../status/gameplay/game-frame-300.bgrx");
        if (frame == 599)
            ff_gpu_save_frame("../status/gameplay/game-frame-600.bgrx");
    }
    QueryPerformanceCounter(&finished);
    if (diag_enabled)
        printf("diagnostic_profile prefix_ms %.3f tail_ms %.3f record_ms %.3f gpu_submit_ms %.3f\n", 1000.0 * prefix_ticks / frequency.QuadPart, 1000.0 * tail_ticks / frequency.QuadPart, 1000.0 * record_ticks / frequency.QuadPart, 1000.0 * diag_gpu_ticks / frequency.QuadPart);
    if (actors)
        fclose(actors);
    if (world)
        fclose(world);
    if (detail && fclose(detail))
        return 8;
    ff_audit_sound_event = diag_phases ? diagnostic_sound : NULL;
    if (diag_sound)
    {
        int error = fclose(diag_sound);
        diag_sound = NULL;
        if (error)
            return 8;
    }
    if (diag_inputs)
    {
        int error = fclose(diag_inputs);
        diag_inputs = NULL;
        if (error)
            return 8;
    }
    printf("gameplay_metrics frames %u vblanks %u seconds %.6f audio_buffers %u nonzero %u overruns %u\n", frame, (uint32)VSync(-1) - vblank_start, (double)(finished.QuadPart - started.QuadPart) / (double)frequency.QuadPart, g_waveout_submitted_buffers - audio_start, g_waveout_nonzero_buffers - nonzero_start, g_waveout_callback_overruns);
    ff_gpu_save_frame("../status/gameplay/game-frame-900.bgrx");
    f = fopen(output, "wb");
    if (!f)
    {
        return 7;
    }
    if (fwrite(ff_ptr(0x80000000, 0x200000), 1, 0x200000, f) != 0x200000)
    {
        fclose(f);
        return 8;
    }
    fclose(f);
    return result;
}

static int finish_game_to_menu(int result)
{
    /* 15EF0..15F00 preserves the game return across the fade call */
    ff_w32(0x800941a8, 0);
    FUN_8005ED78();
    if (result == 4)
    {
        if (ff_u32(0x80093d4c) != 329)
        {
            fprintf(stderr, "WIP: game-over retry continuation\n");
            return 11;
        }
        FUN_8006AB80(ff_u32(0x80094030), ff_u32(0x80094034));
        if (diag_phase_limit && diag_phase_count == diag_phase_limit)
            return 0;
    }
    FUN_80056ECC();
    ff_w32(0x800941a8, 0);
    FUN_8005ED78();
    result = (int)ff_menu_startup_8004FBCC();
    ff_services.frontend_frame = NULL;
    return result;
}

static int game_loop_preview(int sequence)
{
    PSX_CONFIG config;
    FILE *f;
    int result = 9;
    if (!ff_load_ram(sequence ? "../status/gameplay/sequence-start.ram" : "../status/gameplay/sequence-level-entry.ram"))
        return 2;
    f = fopen(sequence ? "../status/gameplay/sequence-start.scratch" : "../status/gameplay/sequence-level-entry.scratch", "rb");
    if (!f)
        return 3;
    if (fread(ff_ptr(0x1f800000, 1024), 1, 1024, f) != 1024)
    {
        fclose(f);
        return 4;
    }
    fclose(f);
    ff_gpu_init_empty();
    if (sequence)
    {
        if (!ff_audio_init())
            return 3;
    }
    else
        ff_audio_init_empty();
    memset(&config, 0, sizeof(config));
    config.window_title = "Fighting Force gameplay loop audit";
    config.window_width = 960;
    config.window_height = 720;
    config.refresh_rate = 60;
    config.headless = 1;
    psx_configure(&config);
    ResetGraph(0);
    InitGeom();
    if (!waveout_init())
        return 5;
    ff_services.game_longjmp = exit_menu;
    ff_services.menu_aux = menu_aux;
    ff_services.frontend_frame = title_host_frame;
    if (!setjmp(menu_exit))
    {
        result = run_loaded_game(sequence, 900, sequence ? "../status/gameplay/sequence-game-loop-final.ram" : "../status/gameplay/game-loop-final.ram");
    }
    ff_services.frontend_frame = NULL;
    waveout_shutdown();
    printf("game_loop_result %d tick %u\n", result, ff_u32(0x80093dd0));
    ff_audit_end();
    return result;
}

int ff_game_loop_preview(void)
{
    return game_loop_preview(0);
}

int ff_sequence_game_loop_preview(void)
{
    return game_loop_preview(1);
}

/* Isolated sequence fixture audit; frontend uses run_loaded_game directly. */
int ff_sequence_preview(int headless)
{
    PSX_CONFIG config;
    int result = 9;
    if (!ff_load_ram("../status/gameplay/sequence-start.ram"))
        return 2;
    ff_gpu_init_empty();
    if (!ff_audio_init())
        return 3;
    memset(&config, 0, sizeof(config));
    config.window_title = "Fighting Force - sequence WIP";
    config.window_width = 960;
    config.window_height = 720;
    config.refresh_rate = 60;
    config.headless = headless;
    psx_configure(&config);
    ResetGraph(0);
    InitGeom();
    if (!waveout_init())
        return 5;
    ff_services.game_longjmp = exit_menu;
    ff_services.menu_aux = menu_aux;
    ff_services.frontend_frame = title_host_frame;
    if (!setjmp(menu_exit))
        result = ff_sequence_run_80015720_stage0(sequence_prepare_frame);
    ff_services.frontend_frame = NULL;
    waveout_shutdown();
    printf("sequence_result %d tick %u stage %u\n", result, ff_u32(0x80093dd0), ff_u32(0x80093d58));
    ff_audit_end();
    return result == 1 ? 0 : result;
}

/* Interactive title preview. Initial configuration still comes from fixtures. */
int ff_title_runtime(void)
{
    PSX_CONFIG config;
    uint32 result = 0;
    if (!ff_load_ram("FF-menu.ram") || !ff_gpu_load_vram("FF-menu.vram"))
        return 2;
    memset(&config, 0, sizeof(config));
    config.window_title = "Fighting Force - title WIP";
    config.window_width = 960;
    config.window_height = 720;
    config.refresh_rate = 60;
    psx_configure(&config);
    ResetGraph(0);
    InitGeom();
    ff_w32(0x800940b0, 1);
    ff_w16(0x80093566, 0);
    ff_w32(0x80093c14, 0xffffffff);
    ff_services.game_longjmp = exit_menu;
    ff_services.frontend_frame = title_host_frame;
    if (!setjmp(menu_exit))
    {
        title_host_frame();
        result = FUN_80069F78();
    }
    ff_services.frontend_frame = NULL;
    printf("title_result %u timer %u\n", result, ff_u32(0x80093dd0));
    ff_audit_end();
    return 0;
}

/* Scripted menu jobs end at confirmation; subsequent gameplay has no input.
 * Keep physical keyboard input for the ordinary interactive runtime. */
static uint32 scripted_neutral_pad(void *user, sint32 controller)
{
    return 0;
}

static int menu_runtime_impl(const char *script, uint32 limit, int headless, int startup)
{
    PSX_CONFIG config;
    FILE *input = NULL, *trace = NULL;
    uint32 frame = 0;
    int result = 0, i;
    const char *menu_load = getenv("FF_MENU_CHECKPOINT_LOAD"), *menu_save = getenv("FF_MENU_CHECKPOINT_SAVE");
    const char *phase_load = getenv("FF_PHASE_CHECKPOINT_LOAD");
    const char *menu_tick_text = getenv("FF_MENU_CHECKPOINT_TICK");
    uint32 menu_tick = 520;
    int menu_saved = 0;
    diag_phase_count = 0;
    diag_phase_limit = 0;
    diag_phase_base = 0;
    diag_resume_pc = 0;
    diag_menu_jobs = 0;
    diag_phase_directory = getenv("FF_PHASE_CHECKPOINT_DIRECTORY");
    diag_phase_interval = 300;
    if (!diagnostic_init())
        return 14;
    if ((menu_load || menu_save) && !diag_enabled)
        return 14;
    if ((phase_load || diag_phase_directory) && (!diag_enabled || !getenv("FF_TRACE_INPUT_CALLS")))
        return 14;
    if (phase_load && (menu_load || getenv("FF_CHECKPOINT_LOAD")))
        return 14;
    if (getenv("FF_PHASE_CHECKPOINT_INTERVAL"))
    {
        char *end;
        const char *value = getenv("FF_PHASE_CHECKPOINT_INTERVAL");
        unsigned long count = strtoul(value, &end, 10);
        if (!*value || *end || !count || count > 36000)
            return 14;
        diag_phase_interval = (uint32)count;
    }
    if (menu_tick_text)
    {
        char *end;
        unsigned long value = strtoul(menu_tick_text, &end, 10);
        if (!*menu_tick_text || *end || value > 1000000)
            return 14;
        menu_tick = (uint32)value;
    }
    if (menu_load && getenv("FF_CHECKPOINT_LOAD"))
        return 14;
    if (getenv("FF_MENU_PHASE_COUNT"))
    {
        char *end;
        const char *value = getenv("FF_MENU_PHASE_COUNT");
        unsigned long count = strtoul(value, &end, 10);
        if (!*value || *end || !count || count > 36001 || (!menu_load && !phase_load) || !getenv("FF_TRACE_PHASE_PATH"))
            return 14;
        diag_phase_limit = (uint32)count;
    }
    if (getenv("FF_TRACE_INPUT_CALLS") && ((!menu_load && !phase_load) || !diag_phase_limit))
        return 14;
    if (diag_enabled && (getenv("FF_CHECKPOINT_LOAD") || menu_load || phase_load))
    {
    }
    else if (startup == 2)
    {
        if (!ff_load_game_image("GAME.EXE"))
            return 2;
    }
    else if (!ff_load_ram("FF-menu.ram"))
        return 2;
    if (startup)
    {
        ff_gpu_init_empty();
        ff_audio_init_empty();
    }
    else if (!ff_gpu_load_vram("FF-menu.vram") || !ff_audio_init())
        return 2;
    if (script)
    {
        input = fopen(script, "rb");
        if (!input)
            return 4;
        trace = fopen("../status/menu/runtime-trace.bin", "wb");
    }
    memset(&config, 0, sizeof(config));
    config.window_title = "Fighting Force - menu WIP";
    config.window_width = 960;
    config.window_height = 720;
    config.refresh_rate = 60;
    config.headless = headless;
    if (script)
        config.host.pad_read = scripted_neutral_pad;
    psx_configure(&config);
    ResetGraph(0);
    InitGeom();
    if (!waveout_init())
        return 5;
    if (getenv("FF_TRACE_PHASE_PATH"))
    {
        uint32 header[3] = {0x31504646, 3, 2228};
        diag_phases = fopen(getenv("FF_TRACE_PHASE_PATH"), "wbx");
        diag_phase_failed = 0;
        if (!diag_phases)
            return 14;
        if (fwrite(header, sizeof(header), 1, diag_phases) != 1)
        {
            fclose(diag_phases);
            diag_phases = NULL;
            return 14;
        }
        ff_audit_sound_event = diagnostic_sound;
    }
    ff_services.game_longjmp = exit_menu;
    ff_services.menu_aux = menu_aux;
    /* Resume at the reference callback entry; host pads supply original active-low bytes. */
    ff_w32(0x800927ec, 0x80050270);
    ff_w32(0x8009355c, 0x80094748);
    ff_w32(0x80093560, 0x80094770);
    if (setjmp(menu_exit))
    {
        result = 9;
        goto done;
    }
    if (getenv("FF_TRACE_INPUT_CALLS"))
    {
        uint32 magic;
        diag_input_ordinal = 0;
        diag_input_calls = fopen(getenv("FF_TRACE_INPUT_CALLS"), "rb");
        if (!diag_input_calls || fread(&magic, sizeof(magic), 1, diag_input_calls) != 1 || magic != 0x31494646)
        {
            result = 14;
            goto done;
        }
    }
    if (menu_load)
    {
        if (!diagnostic_checkpoint(menu_load, 1) || ff_u32(0x80093d58) != 26)
        {
            result = 15;
            goto done;
        }
        startup = 0;
        frame = 1;
        diag_title = 0;
    }
    if (phase_load)
    {
        if (!diagnostic_phase_restore(phase_load, input))
        {
            result = 15;
            goto done;
        }
        startup = 0;
        frame = 1;
        if (diag_resume_pc != 0x8004fef4)
        {
            ff_services.frontend_frame = title_host_frame;
            if (diag_resume_pc == 0x8006acf4)
            {
                diag_resume_pc = 0;
                ff_highscore_resume_8006ACF4((const uint8 *)diag_highscore_saved);
                if (diag_phase_limit && diag_phase_count == diag_phase_limit)
                    goto done;
                FUN_80056ECC();
                ff_w32(0x800941a8, 0);
                FUN_8005ED78();
                result = (int)ff_menu_startup_8004FBCC();
                ff_services.frontend_frame = NULL;
            }
            else
            {
                result = run_loaded_game(0, diag_end_tick, diagnostic_output());
                if (result != 1 && result != 4)
                    goto done;
                result = finish_game_to_menu(result);
            }
            if (result || (diag_phase_limit && diag_phase_count == diag_phase_limit))
                goto done;
        }
        diag_resume_pc = 0;
    }
    if (diag_enabled && getenv("FF_CHECKPOINT_LOAD"))
    {
        ff_services.frontend_frame = title_host_frame;
        result = run_loaded_game(0, diag_end_tick, diagnostic_output());
        printf("frontend_game_result %d tick %u\n", result, ff_u32(0x80093dd0));
        goto done;
    }
    if (startup)
    {
        if (startup == 2)
        {
            FUN_8006A50C();
            memset(ff_ptr(0x8009e858, 1044), 0, 1044);
            FUN_8003EE4C();
            FUN_8003EF78();
            FUN_80046854();
        }
        FUN_8005717C();
        FUN_800675FC();
        ff_display_setup_800574B4();
        ff_services.frontend_frame = title_host_frame;
        title_host_frame();
        if (startup == 2)
        {
            FUN_80064684();
            FUN_80018E50();
            /* WIP69554: no memory-card startup UI or saved configuration is loaded. */
            ff_dummy_memory_card_read(0, NULL, 0);
            FUN_8005EC90();
            FUN_80056ECC();
            ff_w32(0x800941a8, 0);
            FUN_8005ED78();
        }
        result = (int)ff_menu_startup_8004FBCC();
        ff_services.frontend_frame = NULL;
        if (result)
            goto done;
    }
    while (!psx_quit_requested() && (!limit || frame < limit))
    {
        uint32 job[3] = {1, 0, 0}, callback, display;
        int cleanup;
        sint32 display_x, display_y;
        if (input)
        {
            if (fread(job, sizeof(job), 1, input) != 1)
                break;
        }
        else
        {
            uint32 pad = PadRead(0);
            job[1] = ((pad & 255) << 8) | ((pad >> 8) & 255);
        }
        ff_host_pad_publish(job[0], job[1], job[2]);
        ++diag_menu_jobs;
        diag_connected = job[0];

        /* Fixture PC is already at the first callback, after this prefix. */
        if (frame || startup)
        {
            if (menu_save && !menu_saved && ff_u32(0x80093dd0) == menu_tick)
            {
                FILE *existing = fopen(menu_save, "rb");
                if (existing)
                {
                    fclose(existing);
                    result = 15;
                    break;
                }
                if (!diagnostic_checkpoint(menu_save, 0))
                {
                    result = 15;
                    break;
                }
                menu_saved = 1;
            }
            if (ff_audit_phase_boundary(0x8004fef4))
                break;
            FUN_80058C14();
            FUN_8004F520();
            ff_w16(0x80093d34, (uint16)ff_s16(0x8008d490));
            ff_w16(0x80093d36, (uint16)ff_s16(0x8008d492));
            for (i = 0; i < 3; i++)
                ff_w32(0x80093d28 + 4u * i, ff_u32(0x8008d4a4 + 4u * i));
            ff_w32(0x80093dd0, ff_u32(0x80093dd0) + 1);
            /* 80058650..80058660 uploads the current 256-color palette first. */
            if (!ff_gpu_load_clut(0x800b50b8, 0, 499) || !ff_gpu_load_image(0x80093568, 0x800a1990))
            {
                result = 12;
                break;
            }
            FUN_80011D9C();
        }
        display = 0x800947a0 + 20 * (ff_u32(0x8008d4c4) & 1);
        display_x = ff_s16(display + 8);
        display_y = ff_s16(display + 10);
        FUN_80064684();
        ff_display_offsets_800586D0();
        /* Original5868C..587A4: compress prior OT before swapping, submit it,
   * then initialize the new current OT. Fixture frame0 was already compressed. */
        ff_gpu_begin();
        /* 8005876C..80058794: DrawOTagEnv(previous OT, DrawEnv[parity]).
   * Save State 1 uses horizontal VRAM draw surfaces at x=0 and x=320. */
        ff_gpu_draw_env(0x800b8938 + 92 * (ff_u32(0x8008d4c4) & 1), 320 * (ff_u32(0x8008d4c4) & 1), 0);
        if (ff_gpu_ot(ff_u32(0x8008d4b8)) < 0)
        {
            result = 7;
            break;
        }
        ff_gpu_display_offset(display_x, display_y);
        ff_gpu_present();
        FUN_80011D50(ff_u32(0x8008d4b4));
        if (frame || startup)
        {
            if (ff_s16(0x8001000c) + ff_s16(0x8001000e) + ff_s16(0x80010010))
                FUN_8006471C();
            if (ff_u32(0x80094190))
                FUN_80064820();
            ff_menu_horizon_80058844();
        }
        /* Frame zero retains the entry fixture; subsequent horizons are projected. */
        FUN_8006686C();
        FUN_80063EEC();
        /* 58BD0..58BF4 runs after packet construction; affects the next frame. */
        if ((frame || startup) && (sint32)ff_u32(0x80093dd0) < 33)
        {
            sint32 fade = (sint32)((ff_u32(0x80093dd0) << 3) - 256u);
            FUN_80011CCC(fade, fade, fade);
        }
        callback = ff_u32(0x800927ec);
        if (!ff_u32(0x80092788))
        {
            if (callback == 0x80050270)
                FUN_80050270();
            else if (callback == 0x80051e50)
                FUN_80051E50();
            else if (callback == 0x800520ac)
                FUN_800520AC();
            else if (callback == 0x80050798)
                FUN_80050798();
            else if (callback == 0x80050d98)
                FUN_80050D98();
            else if (callback == 0x80051c98)
                FUN_80051C98();
            else if (callback == 0x800519d4)
                FUN_800519D4();
            else if (callback == 0x80052624)
                FUN_80052624();
            else
            {
                fprintf(stderr, "WIP: untranslated menu callback %08X\n", callback);
                result = 10;
                break;
            }
        }
        /* 8004FFB4..8004FFCC: update then draw actors with original selections/count. */
        if (!ff_u32(0x80092734))
        {
            ff_menu_stage_8001E168();
            FUN_80050CC8();
            FUN_80058E24(0x8009277c, (sint32)ff_u32(0x80092784));
            /* 8004FFD0..80050034: scene update callbacks, followed by object rendering. */
            for (i = 0; i < (sint32)ff_u32(0x8009403c); i++)
            {
                uint32 object = 0x800b89f0 + 36u * i, target = ff_u32(0x800bbf78 + 4 * ff_u32(object));
                if (target)
                    ff_object_call(target, object);
            }
            FUN_8005C17C();
            FUN_8001EE44();
            /* 80050040..8005007C: camera matrix, effect rendering, then effect update.
    * Dynamic object rendering has run immediately before this matrix setup. */
            FUN_80011D20();
            FUN_800101CC(ff_s16(0x8008d492));
            FUN_8001033C(ff_s16(0x8008d490));
            FUN_8001EF68();
            FUN_8001EDB8();
            FUN_8001ED1C();
        }
        cleanup = ff_menu_tail_80050090(ff_u32(0x80092734) != 0);
        if (trace)
        {
            uint32 row[8] = {frame, callback, ff_u32(0x800927ec), ff_u32(0x80092730), ff_u32(0x8009277c), ff_u32(0x80092780), ff_u32(0x8009272c), ff_u32(0x80092788)};
            fwrite(row, sizeof(row), 1, trace);
        }
        ++frame;
        if (cleanup)
        {
            FUN_8005F214();
            FUN_80056ECC();
            if (ff_u32(0x80093d1c) != 0xffffffffu)
            {
                fprintf(stderr, "WIP: unsupported menu continuation\n");
                result = 11;
                break;
            }
            ff_services.frontend_frame = title_host_frame;
            /* 80015E90..80015EBC runs the opening sequence only for a normal stage-zero start */
            result = run_loaded_game(ff_u32(0x80093d58) == 0 && ff_u32(0x80093d1c) == 0xffffffffu, script ? (diag_enabled ? diag_end_tick : 900) : 0, diagnostic_output());
            printf("frontend_game_result %d tick %u\n", result, ff_u32(0x80093dd0));
            if (result == 1 || result == 4)
            {
                result = finish_game_to_menu(result);
                if (diag_phase_limit && diag_phase_count == diag_phase_limit)
                    break;
                printf("frontend_menu_return %d stage %u\n", result, ff_u32(0x80093d58));
                fflush(stdout);
                if (result)
                    break;
                continue;
            }
            break;
        }
        VSync(0);
    }
done:
    ff_audit_sound_event = NULL;
    if (diag_phase_limit && diag_phase_count != diag_phase_limit && result == 0)
        result = 14;
    if (diag_input_calls)
    {
        const char *cursor_text = getenv("FF_TRACE_INPUT_CALLS_END");
        if (cursor_text)
        {
            char *end;
            unsigned long cursor = strtoul(cursor_text, &end, 10);
            /* Narrow replay retains future packets for boundary priming */
            if (!*cursor_text || *end || cursor > 1000000 || diag_input_ordinal != (uint32)cursor || ferror(diag_input_calls))
            {
                if (!result)
                    result = 14;
            }
        }
        else if (fgetc(diag_input_calls) != EOF || ferror(diag_input_calls))
        {
            if (!result)
                result = 14;
        }
        if (fclose(diag_input_calls) && !result)
            result = 14;
        diag_input_calls = NULL;
        printf("diagnostic_input_calls %u\n", diag_input_ordinal);
    }
    if (diag_phases)
    {
        uint32 footer[4] = {5, ff_u32(0x80093dd0), 4, (uint32)(!diag_phase_failed && result == 0)};
        if (fwrite(footer, sizeof(footer), 1, diag_phases) != 1)
            diag_phase_failed = 1;
        if (fclose(diag_phases))
            diag_phase_failed = 1;
        diag_phases = NULL;
        if (diag_phase_failed)
            result = 14;
    }
    ff_services.frontend_frame = NULL;
    waveout_shutdown();
    if (input)
        fclose(input);
    if (trace)
        fclose(trace);
    ff_gpu_save_frame("../status/menu/runtime-frame.bgrx");
    printf("menu_frames %u result %d audio_buffers %u nonzero %u peak %u\n", frame, result, g_waveout_submitted_buffers, g_waveout_nonzero_buffers, g_waveout_peak);
    ff_audit_end();
    return result;
}

int ff_menu_runtime(const char *script, uint32 limit, int headless)
{
    return menu_runtime_impl(script, limit, headless, 0);
}

int ff_menu_startup_runtime(uint32 limit, int headless)
{
    return menu_runtime_impl(NULL, limit, headless, 1);
}

int ff_menu_cold_runtime(uint32 limit, int headless)
{
    return menu_runtime_impl(NULL, limit, headless, 2);
}

int ff_menu_cold_script_runtime(const char *script, int headless)
{
    return menu_runtime_impl(script, 0, headless, 2);
}

int ff_menu_startup_script_runtime(const char *script, int headless)
{
    return menu_runtime_impl(script, 0, headless, 1);
}
