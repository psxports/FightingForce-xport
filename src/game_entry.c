#include <stdlib.h>

#include "game.h"
#include "psx_gpu.h"

// XPORT REVISION: 2026-09-29T20:00:00Z
void xport_main(void)
{
#if defined(_WIN32) && defined(_DEBUG)
    xport_set_exit_code(game_run_arguments(__argc, __argv));
#else
    char *arguments[] = {"FF", NULL};
    xport_set_exit_code(game_run_arguments(1, arguments));
#endif
}
