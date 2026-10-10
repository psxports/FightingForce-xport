#ifndef FF_VIDEO_H
#define FF_VIDEO_H

#include "psx.h"

sint32 ff_movie_search(CdlFILE *file, const char *name);
void ff_movie_wait(void);
void ff_movie_frame(void);
void ff_movie_intro(void);
sint32 ff_movie_smoke(void);

#endif
