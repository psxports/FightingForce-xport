#ifndef SOUND_SAMPLES_H
#define SOUND_SAMPLES_H

#include "xport.h"
#include "resources.h"

/* Original sound scratch window ends at the two-MiB RAM boundary */
enum
{
    SOUND_SAMPLE_COUNT = 246,
    SOUND_ARCHIVE_CAPACITY = 0xea000
};

typedef struct SOUND_SAMPLE
{
    uint32 offset;
    uint8 mode;
} SOUND_SAMPLE;

typedef struct SOUND_SAMPLES
{
    uint32 spu_base;
    SOUND_SAMPLE sample[SOUND_SAMPLE_COUNT];
    /* CPU archive lifetime is independent of uploaded SPU samples */
    RESOURCE_FILE file;
} SOUND_SAMPLES;

extern SOUND_SAMPLES sound_samples;

sint32 sound_samples_load(sint32 archive);

#endif
