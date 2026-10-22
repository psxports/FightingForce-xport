#include "ff.h"
#include "wip.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static int continue_wip = 1;

__declspec(noreturn) void ff_wip_stop(const char *function, const char *file, int line)
{
    FF_DIAGNOSTIC(fprintf(stderr, "Fatal guard or unclassified WIP: %s (%s:%d)\n", function, file, line));
    abort();
}

int ff_wip_options(int *argc, char **argv)
{
    int i, out = 1;
    for (i = 1; i < *argc; i++)
    {
        if (!strcmp(argv[i], "--debug"))
        {
            continue_wip = 0;
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

int ff_wip_visit(unsigned int pc, const char *subsystem, const char *function, const char *file, int line, int recoverable, const char *fallback)
{
    int resume = continue_wip && recoverable;
    if (!resume)
        FF_DIAGNOSTIC(fprintf(stderr, "Fatal guard or unclassified WIP: %s pc %08X (%s:%d), subsystem %s, fallback %s\n", function, pc, file, line, subsystem, fallback));
    return resume;
}
