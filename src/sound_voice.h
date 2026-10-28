#ifndef SOUND_VOICE_H
#define SOUND_VOICE_H

#include "psx.h"

enum
{
    SOUND_VOICE_COUNT = 24,
    SOUND_VOICE_ATTRIBUTE_COUNT = 16,
    SOUND_VOICE_ATTRIBUTE_BYTES = 64
};

/* Binary SDK packet view; game operations use attr fields */
typedef union SOUND_VOICE_ATTRIBUTE_PACKET
{
    SpuVoiceAttr attr;
    uint8 wire[SOUND_VOICE_ATTRIBUTE_BYTES];
} SOUND_VOICE_ATTRIBUTE_PACKET;

typedef struct SOUND_ATTRIBUTE_RING
{
    uint32 cur;
    SOUND_VOICE_ATTRIBUTE_PACKET packet[SOUND_VOICE_ATTRIBUTE_COUNT];
} SOUND_ATTRIBUTE_RING;

extern SOUND_ATTRIBUTE_RING sound_attributes;
GDB_CALL SpuVoiceAttr *sound_voice_next_attributes(void);
GDB_CALL sint32 sound_voice_defaults(SpuVoiceAttr *attributes);

typedef struct SOUND_VOICE
{
    sint8 mode;
    sint8 left;
    sint8 right;
} SOUND_VOICE;

typedef struct SOUND_VOICES
{
    uint32 cur;
    SOUND_VOICE voice[SOUND_VOICE_COUNT];
} SOUND_VOICES;

extern SOUND_VOICES sound_voices;

GDB_CALL sint32 sound_voice_play(sint32 effect, sint32 volume, sint32 pitch);
GDB_CALL sint32 sound_voice_stop(sint32 voice);
GDB_CALL sint32 sound_voice_pitch(sint32 voice, sint32 pitch);
GDB_CALL sint32 sound_voice_pan(sint32 voice, sint32 volume, sint32 pan);
GDB_CALL void sound_voices_stop_all(void);
GDB_CALL sint32 sound_voice_volume(sint32 voice, sint32 volume);
GDB_CALL sint32 sound_voice_restore_volume(sint32 voice, sint32 unused, sint32 right);
GDB_CALL sint32 sound_voices_pause(void);
GDB_CALL sint32 sound_voices_resume(void);

#endif
