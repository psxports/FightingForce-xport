#include <stdlib.h>
#include <string.h>

#include "game.h"
#include "psx_gpu.h"

// XPORT REVISION: 2026-09-29T20:00:00Z
void xport_main(void)
{
#if defined(_WIN32) && !defined(NDEBUG)
    int idx, count = 0, result;
    char **arguments = malloc(((size_t)__argc + 1) * sizeof(*arguments));
    if (!arguments)
    {
        xport_set_exit_code(2);
        return;
    }
    /* The host consumes collect; retain the remaining FF diagnostic arguments */
    for (idx = 0; idx < __argc; ++idx)
        if (!idx || strcmp(__argv[idx], "--collect"))
            arguments[count++] = __argv[idx];
    arguments[count] = NULL;
    result = game_run_arguments(count, arguments);
    free(arguments);
    xport_set_exit_code(result);
#else
    char *arguments[] = {"FF", NULL};
    xport_set_exit_code(game_run_arguments(1, arguments));
#endif
}
