#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "game.h"
#include "diagnostics.h"

#undef diagnostic_stop
#undef diagnostic_branch

static int continue_wip = 1;

#if !defined(NDEBUG) && !defined(__EMSCRIPTEN__)
void (*diagnostic_stop_observer)(const char *function, const char *file, int line);
#endif

__declspec(noreturn) void diagnostic_stop(const char *function, const char *file, int line)
{
    DIAGNOSTIC(fprintf(stderr, "Fatal guard or unclassified WIP: %s (%s:%d)\n", function, file, line));
#if !defined(NDEBUG) && !defined(__EMSCRIPTEN__)
    void (*observer)(const char *, const char *, int) = diagnostic_stop_observer;
    diagnostic_stop_observer = NULL;
    if (observer)
        observer(function, file, line);
#endif
    abort();
}

int diagnostic_options(int *argc, char **argv)
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

int diagnostic_branch(unsigned int pc, const char *subsystem, const char *function, const char *file, int line, int recoverable, const char *fallback)
{
    int resume = continue_wip && recoverable;
    if (!resume)
        DIAGNOSTIC(fprintf(stderr, "Fatal guard or unclassified WIP: %s pc %08X (%s:%d), subsystem %s, fallback %s\n", function, pc, file, line, subsystem, fallback));
    return resume;
}
