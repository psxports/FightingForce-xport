#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "game.h"
#include "diagnostics.h"

int diagnostic_options(int *argc, char **argv)
{
    int i, out = 1;
    for (i = 1; i < *argc; i++)
    {
        if (!strcmp(argv[i], "--debug"))
        {
#if defined(_WIN32) && !defined(NDEBUG)
            _set_abort_behavior(0, _WRITE_ABORT_MSG | _CALL_REPORTFAULT);
#endif
        }
        else
            argv[out++] = argv[i];
    }
    *argc = out;
    argv[out] = NULL;
    return 1;
}
