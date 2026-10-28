#ifndef DIAGNOSTICS_H
#define DIAGNOSTICS_H
int diagnostic_options(int *argc, char **argv);
int diagnostic_branch(unsigned int pc, const char *subsystem, const char *function, const char *file, int line, int recoverable, const char *fallback);
__declspec(noreturn) void diagnostic_stop(const char *function, const char *file, int line);

#if !defined(NDEBUG) && !defined(__EMSCRIPTEN__)
/* Optional one-shot audit observation; the fatal stop still aborts */
extern void (*diagnostic_stop_observer)(const char *function, const char *file, int line);
#endif

/* Release omits diagnostic text and preserves control arguments */
#if defined(NDEBUG)
    #define diagnostic_stop(function, file, line) diagnostic_stop(0, 0, (line))
    #define diagnostic_branch(pc, subsystem, function, file, line, recoverable, fallback) diagnostic_branch((pc), 0, 0, 0, (line), (recoverable), 0)
#endif

#endif
