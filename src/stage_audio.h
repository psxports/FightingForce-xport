#ifndef STAGE_AUDIO_H
#define STAGE_AUDIO_H

#include "xport.h"

enum
{
    STAGE_AUDIO_SLOT_COUNT = 8
};

/* Original contexts reuse one slot for a voice, encoded voice or engine level */
typedef union STAGE_AUDIO_SLOT
{
    sint32 voice;
    uint32 voice_plus_one;
    uint32 level;
} STAGE_AUDIO_SLOT;

extern STAGE_AUDIO_SLOT stage_audio_slots[STAGE_AUDIO_SLOT_COUNT];

sint32 stage_audio_start(void);

#endif
