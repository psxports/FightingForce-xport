#ifndef SOUND_OUTPUT_H
#define SOUND_OUTPUT_H

#include "xport.h"

enum
{
    SOUND_FLAG_STEREO = 1
};

typedef struct SOUND_SETTINGS
{
    uint16 flags;
    uint8 effects_volume;
    uint8 music_volume;
} SOUND_SETTINGS;

extern const SOUND_SETTINGS sound_settings_defaults;
extern SOUND_SETTINGS sound_settings;
extern uint32 sound_reverb_configured;

sint32 sound_reverb_reset(void);
sint32 sound_reverb_configure(void);
sint32 sound_reverb_enable_channels(void);
sint32 sound_reverb_disable_channels(void);
sint32 sound_output_init(void);

#endif
