#ifndef FF_WIP_H
#define FF_WIP_H
int ff_wip_options(int *argc, char **argv);
int ff_wip_visit(unsigned int pc, const char *subsystem, const char *function, const char *file, int line, int recoverable, const char *fallback);
__declspec(noreturn) void ff_wip_stop(const char *function, const char *file, int line);
#endif
