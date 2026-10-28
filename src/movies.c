#include <stdio.h>

#include "game.h"
#include "movies.h"

/* Existing SDK native GPU and CD owners require no game RAM bindings */
const uint32 xport_gpu_graph_type_address = 0u;
const uint32 xport_cd_ready_callback_address = 0u;
const uint32 xport_cd_sync_callback_address = 0u;
const uint32 xport_cd_status_address = 0u;
const uint32 xport_cd_setloc_table_address = 0u;

static sint32 movie_play(const char *path, sint32 video_y)
{
    FUNCTION_MARKER(0x8001287cu, "SLUS_004.33");
    const XportFmvPresentation presentation = {.width = 320, .height = 240, .video_y = video_y, .screen_height = 240, .interlace = 1, .controllers = 1, .skip_buttons = PADstart | PADRdown, .skip_held = 1};
    sint32 result = xport_fmv_play_presented(path, 0, 0, &presentation);
    xport_fmv_discard_audio();
    return result;
}

void movie_play_intro(void)
{
    static const char *const paths[] = {"DATA/LOGO.FMV", "DATA/FFNEW7.FMV"};
    static const sint32 video_y[] = {0, 24};
    uint32 i;
    for (i = 0; i < sizeof(paths) / sizeof(paths[0]); ++i)
    {
        sint32 result = movie_play(paths[i], video_y[i]);
        if (xport_isquit())
            return;
        if (result < 0)
        {
            DIAGNOSTIC(fprintf(stderr, "Fullscreen movie failed: %s\n", paths[i]));
            diagnostic_stop(__FUNCTION__, __FILE__, __LINE__);
        }
        if (result != 1)
            return;
    }
}
