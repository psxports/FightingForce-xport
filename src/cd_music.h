#ifndef CD_MUSIC_H
#define CD_MUSIC_H

#include "psx.h"

enum
{
    CD_MUSIC_TOC_COUNT = 28
};

typedef struct CD_MUSIC
{
    uint32 track_count;
    uint32 track;
    uint32 async_mode;
    uint32 paused;
    CdlLOC toc[CD_MUSIC_TOC_COUNT];
} CD_MUSIC;

extern CD_MUSIC cd_music;

sint32 cd_music_init(void);
sint32 cd_music_set_volume(sint32 volume);
sint32 cd_music_volume(sint32 volume);
sint32 cd_music_play(sint32 asynchronous, sint32 track);
sint32 cd_music_start_track(sint32 track);
sint32 cd_music_stop(void);
sint32 cd_music_pause(void);
sint32 cd_music_resume(void);
sint32 cd_music_pause_playback(void);
sint32 cd_music_resume_playback(void);
sint32 cd_music_poll(void);

sint32 cd_music_start_scene(sint32 track);

#endif
