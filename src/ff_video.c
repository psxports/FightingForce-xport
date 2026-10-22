#include "ff.h"
#include "ff_video.h"
#include "ff_audio.h"
#include "psx_spu.h"
#include "psx_stream.h"
#include "psx_gpu.h"
#include <stdio.h>
#include <string.h>
#if defined(_WIN32)
    #include <windows.h>
#endif

/* Required SDK data bindings from reviewed GAME GPU/CD entries */
const uint32 xport_gpu_graph_type_address = 0x800872ecu;
const uint32 xport_cd_ready_callback_address = 0x8008b6ccu;
const uint32 xport_cd_sync_callback_address = 0x8008b6c8u;
const uint32 xport_cd_status_address = 0x8008b6d8u;
const uint32 xport_cd_setloc_table_address = 0x8008b640u;
static sint32 movie_smoke_index = -1;
static uint32 movie_smoke_buttons;

static uint32 movie_u32(const uint8 *p)
{
    return (uint32)p[0] | ((uint32)p[1] << 8) | ((uint32)p[2] << 16) | ((uint32)p[3] << 24);
}

/* Bind original DATA movies to the shared raw-sector transport */
sint32 ff_movie_search(CdlFILE *file, const char *name)
{
    FILE *input = NULL;
    uint8 header[12], chunk[8];
    uint32 remaining, bytes = 0, sectors = 0;
    const char *asset;
    sint32 success = 0;
    if (!file || !name)
        return 0;
    if (!strcmp(name, "\\LOGO.FMV;1"))
        asset = "DATA/LOGO.FMV";
    else if (!strcmp(name, "\\FFNEW7.FMV;1"))
        asset = "DATA/FFNEW7.FMV";
    else
        return 0;
    input = xport_fopen(asset, "rb");
    if (!input || fread(header, 1, sizeof(header), input) != sizeof(header) || memcmp(header, "RIFF", 4) || memcmp(header + 8, "CDXA", 4))
        goto done;
    remaining = movie_u32(header + 4);
    if (remaining < 4)
        goto done;
    remaining -= 4;
    while (remaining >= 8)
    {
        if (fread(chunk, 1, 8, input) != 8)
            goto done;
        remaining -= 8;
        bytes = movie_u32(chunk + 4);
        if (bytes > remaining)
            goto done;
        if (!memcmp(chunk, "data", 4))
            break;
        if (bytes + (bytes & 1u) > remaining || fseek(input, bytes + (bytes & 1u), SEEK_CUR))
            goto done;
        remaining -= bytes + (bytes & 1u);
    }
    if (remaining < 8 || !bytes || bytes % 2352u || memcmp(chunk, "data", 4))
        goto done;
    sectors = bytes / 2352u;
    if (!cd_mount_stream_file(asset))
        goto done;
    memset(file, 0, sizeof(*file));
    /* The mounted subset maps its first original sector to transport LBA zero */
    CdIntToPos(0, &file->pos);
    file->size = sectors * 2048u;
    strncpy(file->name, asset + 5, sizeof(file->name) - 1);
    success = 1;
done:
    if (input)
        fclose(input);
    if (!success)
        FF_DIAGNOSTIC(fprintf(stderr, "Cannot open original CDXA movie: %s\n", asset));
    return success;
}

void ff_movie_wait(void)
{
    psx_native_poll_events();
#if defined(_WIN32)
    Sleep(1);
#endif
}

static uint32 movie_service(uint32 address, uint32 a, uint32 b, uint32 c, uint32 d, uint32 e)
{
    switch (address)
    {
        case 0x80012bbc:
            return ResetCallbackPSX();
        case 0x8001772c:
            return CdInit();
        case 0x80013808:
        {
            uint32 result = InitPAD(a, b, c, d);
            if (movie_smoke_index >= 0)
                xport_input_override(1, movie_smoke_buttons);
            return result;
        }
        case 0x800138a4:
        {
            uint32 result = StartPAD();
            /* The original movie reader accepts only digital PAD reports */
            PadSetMainMode(0, 0, 3);
            PadSetMainMode(0x10, 0, 3);
            return result;
        }
        case 0x800138d4:
            PadStopCom();
            return 0;
        case 0x80014658:
            return ResetGraph((sint32)a);
        case 0x800148f0:
            return SetGraphDebug((sint32)a);
        case 0x80014a7c:
            SetDispMask((sint32)a);
            return 0;
        case 0x80012cb0:
            return StopCallback();
        case 0x800136ac:
            FlushCache();
            return 0;
        default:
            ff_wip_stop(__FUNCTION__, __FILE__, __LINE__);
            return 0;
    }
}

static void movie_boot_data(void)
{
    /* Reviewed SLUS initialized config at 800302F0 and movie selection at 80011050 */
    w_u16(0x800302f0, 1);
    memcpy(psx_addr(0x800302fc, 12), "\\LOGO.FMV;1", 12);
    memcpy(psx_addr(0x80030308, 14), "\\FFNEW7.FMV;1", 14);
    memset(psx_addr(0x8003032c, 32), 0, 32);
    w_u16(0x80030330, 1);
    w_u16(0x80030332, 1);
    w_u16(0x80030334, 640);
    w_u32(0x80030340, 18000);
    w_u16(0x80030348, 127);
}

void ff_movie_intro(void)
{
    movie_boot_data();
    w_u32(0x80030460, 638);
    if (FUN_SLUS_8001287C(0x800302fc, movie_service) == 1)
    {
        w_u32(0x80030460, 483);
        FUN_SLUS_8001287C(0x80030308, movie_service);
    }
    cd_unmount_image();
}

void ff_movie_frame(void)
{
    if (movie_smoke_index >= 0 && r_u32(0x8003036c) == 60)
        gpu_save_frame(movie_smoke_index ? "../status/movie-ffnew7.bgrx" : "../status/movie-logo.bgrx");
}

sint32 ff_movie_smoke(void)
{
    sint32 result, i;
    FILE *report;
    sint32 success = 1;
    static const uint32 names[2] = {0x800302fc, 0x80030308};
    static const uint32 skip_buttons[2] = {PADstart, PADRdown};
    if (!ff_load_game_image())
        return 2;
    gpu_init_empty();
    ff_audio_init_empty();
    xport_set_headless(1);
    ResetGraph(0);
    if (!xport_audio_init())
        return 3;
    report = xport_fopen("../status/movie-smoke.log", "wb");
    if (!report)
    {
        xport_audio_shutdown();
        return 3;
    }
    movie_boot_data();
    /* Bound the normal translated playback using its caller-supplied end frame */
    w_u32(0x80030340, 60);
    for (i = 0; i < 2; ++i)
    {
        uint32 audio_before = g_xport_audio_nonzero_buffers;
        movie_smoke_index = i;
        result = FUN_SLUS_8001287C(names[i], movie_service);
        FF_DIAGNOSTIC(fprintf(report, "movie_smoke %d result %d frames %u width %u height %u audio_nonzero %u peak %u\n", i, result, r_u32(0x8003036c), r_u32(0x80030400), r_u32(0x80030404), g_xport_audio_nonzero_buffers - audio_before, g_xport_audio_peak));
        FF_DIAGNOSTIC(fflush(report));
        if (result != 1 || r_u32(0x8003036c) != 61 || g_xport_audio_nonzero_buffers == audio_before)
        {
            success = 0;
            break;
        }
    }
    w_u32(0x80030340, 18000);
    for (i = 0; success && i < 2; ++i)
    {
        movie_smoke_buttons = skip_buttons[i];
        result = FUN_SLUS_8001287C(names[0], movie_service);
        FF_DIAGNOSTIC(fprintf(report, "movie_skip %08X result %d frames %u fade %d restored_cd %d %d\n", movie_smoke_buttons, result, r_u32(0x8003036c), r_s16(0x801fe6a0), ff_spu_common_state[2], ff_spu_common_state[3]));
        FF_DIAGNOSTIC(fflush(report));
        if (result != (i ? 1 : 2) || r_u32(0x8003036c) >= 638 || ff_spu_common_state[2] || ff_spu_common_state[3])
            success = 0;
    }
    movie_smoke_index = -1;
    movie_smoke_buttons = 0;
    xport_input_override(0, 0);
    fclose(report);
    cd_unmount_image();
    xport_audio_shutdown();
    return success ? 0 : 4;
}
