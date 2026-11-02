#include <stddef.h>

#include "game.h"

/* A retained 64-byte SDK packet, including untouched serialized tail bytes */
_Static_assert(sizeof(SpuVoiceAttr) == SOUND_VOICE_ATTRIBUTE_BYTES, "SpuVoiceAttr packet size");
_Static_assert(sizeof(SOUND_VOICE_ATTRIBUTE_PACKET) == SOUND_VOICE_ATTRIBUTE_BYTES, "SPU packet view size");
_Static_assert(offsetof(SpuVoiceAttr, voice) == 0, "SPU voice offset");
_Static_assert(offsetof(SpuVoiceAttr, mask) == 4, "SPU mask offset");
_Static_assert(offsetof(SpuVoiceAttr, volume.left) == 8, "SPU volume.left offset");
_Static_assert(offsetof(SpuVoiceAttr, volume.right) == 10, "SPU volume.right offset");
_Static_assert(offsetof(SpuVoiceAttr, volmode.left) == 12, "SPU volmode.left offset");
_Static_assert(offsetof(SpuVoiceAttr, volmode.right) == 14, "SPU volmode.right offset");
_Static_assert(offsetof(SpuVoiceAttr, volumex) == 16, "SPU volumex offset");
_Static_assert(offsetof(SpuVoiceAttr, pitch) == 20, "SPU pitch offset");
_Static_assert(offsetof(SpuVoiceAttr, note) == 22, "SPU note offset");
_Static_assert(offsetof(SpuVoiceAttr, sample_note) == 24, "SPU sample_note offset");
_Static_assert(offsetof(SpuVoiceAttr, envx) == 26, "SPU envx offset");
_Static_assert(offsetof(SpuVoiceAttr, addr) == 28, "SPU addr offset");
_Static_assert(offsetof(SpuVoiceAttr, loop_addr) == 32, "SPU loop_addr offset");
_Static_assert(offsetof(SpuVoiceAttr, a_mode) == 36, "SPU a_mode offset");
_Static_assert(offsetof(SpuVoiceAttr, s_mode) == 40, "SPU s_mode offset");
_Static_assert(offsetof(SpuVoiceAttr, r_mode) == 44, "SPU r_mode offset");
_Static_assert(offsetof(SpuVoiceAttr, ar) == 48, "SPU ar offset");
_Static_assert(offsetof(SpuVoiceAttr, dr) == 50, "SPU dr offset");
_Static_assert(offsetof(SpuVoiceAttr, sr) == 52, "SPU sr offset");
_Static_assert(offsetof(SpuVoiceAttr, rr) == 54, "SPU rr offset");
_Static_assert(offsetof(SpuVoiceAttr, sl) == 56, "SPU sl offset");
_Static_assert(offsetof(SpuVoiceAttr, adsr1) == 58, "SPU adsr1 offset");
_Static_assert(offsetof(SpuVoiceAttr, adsr2) == 60, "SPU adsr2 offset");

SOUND_VOICES sound_voices;
SOUND_ATTRIBUTE_RING sound_attributes;
static sint16 paused_voice_volume[SOUND_VOICE_COUNT][2];

/* Return the old packet after advancing and wrapping the signed cursor */
SpuVoiceAttr *sound_voice_next_attributes(void)
{
    FUNCTION_MARKER(0x80056640, "GAME.EXE");
    uint32 old = sound_attributes.cur, next = old + 1;
    sound_attributes.cur = next;
    if ((sint32)next >= SOUND_VOICE_ATTRIBUTE_COUNT)
        sound_attributes.cur = 0;
    XASSERT(old < SOUND_VOICE_ATTRIBUTE_COUNT);
    return &sound_attributes.packet[old].attr;
}

/* Search circularly past reserved voices; all-reserved has no original termination */
sint32 sound_voice_play(sint32 effect, sint32 volume, sint32 pitch)
{
    FUNCTION_MARKER(0x80056670, "GAME.EXE");
    uint32 master = sound_settings.effects_volume;
    SpuVoiceAttr *attr = sound_voice_next_attributes();
    uint32 voice, sample;
    do
    {
        voice = sound_voices.cur + 1;
        sound_voices.cur = voice;
        if ((sint32)voice >= SOUND_VOICE_COUNT)
            sound_voices.cur = 0;
        voice = sound_voices.cur;
        XASSERT(voice < SOUND_VOICE_COUNT);
    } while (sound_voices.voice[voice].mode == 2);
    XASSERT(effect >= 0 && effect < SOUND_SAMPLE_COUNT);
    const SOUND_SAMPLE *sample_desc = sound_samples.sample + effect;
    sample = sample_desc->offset;
    if (sample == 0xffffffffu)
        sound_voice_stop(voice);
    else
    {
        sint32 level = (sint32)((volume >> 1) * master) >> 8;
        sint32 scaled = pitch * 1536u;
        uint32 note = (scaled / 1024) - 1536u;
        sound_voices.voice[voice].mode = sample_desc->mode;
        voice = sound_voices.cur;
        XASSERT(voice < SOUND_VOICE_COUNT);
        SOUND_VOICE *channel = sound_voices.voice + voice;
        channel->right = level;
        channel->left = level;
        voice = sound_voices.cur;
        attr->mask = 227;
        attr->volume.left = (uint32)level << 7;
        attr->volume.right = (uint32)level << 7;
        attr->sample_note = 21584;
        attr->note = ((note << 1) & 0xffffff00u) + (note & 127);
        attr->voice = 1u << (voice & 31);
        sample = sound_samples.spu_base;
        sample += sample_desc->offset;
        attr->addr = sample;
        XASSERT(game_services.spu_key_on_attributes);
        game_services.spu_key_on_attributes(attr);
    }
    return sound_voices.cur;
}

/* Clear mode before muting and key-off */
sint32 sound_voice_stop(sint32 voice)
{
    FUNCTION_MARKER(0x800567f8, "GAME.EXE");
    XASSERT(voice >= 0 && voice < SOUND_VOICE_COUNT);
    sound_voices.voice[voice].mode = 0;
    sound_voice_volume(voice, 0);
    XASSERT(game_services.spu_key);
    return game_services.spu_key(0, 1u << (voice & 31u));
}

/* Divide the wrapped signed pitch product toward zero */
sint32 sound_voice_pitch(sint32 voice, sint32 pitch)
{
    FUNCTION_MARKER(0x80056840u, "GAME.EXE");
    SpuVoiceAttr *attr = sound_voice_next_attributes();
    uint32 note = ((sint32)(pitch * 1536u) / 1024) - 1536u;
    attr->voice = 1u << (voice & 31u);
    attr->mask = 32;
    attr->note = ((note << 1) & 0xffffff00u) + (note & 127);
    XASSERT(game_services.spu_voice_attributes);
    return game_services.spu_voice_attributes(attr);
}

/* Cache signed channel bytes before publishing spatial SPU attributes */
sint32 sound_voice_pan(sint32 voice_idx, sint32 volume, sint32 pan)
{
    FUNCTION_MARKER(0x800568c8, "GAME.EXE");
    uint32 master = sound_settings.effects_volume;
    SpuVoiceAttr *attr = sound_voice_next_attributes();
    uint32 idx = (uint32)pan << 2;
    sint32 left = (sint32)((sin_table[idx]) * (uint32)volume) >> 15;
    sint32 right = (sint32)((sin_table[MATH_QUARTER_TURN + idx]) * (uint32)volume) >> 15;
    left = (sint32)(left * master) >> 8;
    right = (sint32)(right * master) >> 8;
    XASSERT(voice_idx >= 0 && voice_idx < SOUND_VOICE_COUNT);
    SOUND_VOICE *voice = sound_voices.voice + voice_idx;
    voice->left = left;
    voice->right = right;
    attr->mask = 3;
    attr->voice = 1u << (voice_idx & 31u);
    attr->volume.left = (uint32)left << 6;
    attr->volume.right = (uint32)right << 6;
    XASSERT(game_services.spu_voice_attributes);
    return game_services.spu_voice_attributes(attr);
}

/* Stop every SPU voice, then clear modes without resetting cached volumes */
void sound_voices_stop_all(void)
{
    FUNCTION_MARKER(0x80056ecc, "GAME.EXE");
    uint32 i;
    XASSERT(game_services.spu_key);
    game_services.spu_key(0, 0xffffffu);
    for (i = 0; i < SOUND_VOICE_COUNT; i++)
        sound_voices.voice[i].mode = 0;
}

/* Publish SPU attributes before caching signed channel bytes */
sint32 sound_voice_volume(sint32 voice_idx, sint32 volume)
{
    FUNCTION_MARKER(0x80056f08, "GAME.EXE");
    sint32 product = (uint32)volume * sound_settings.effects_volume, result;
    SpuVoiceAttr *attr = sound_voice_next_attributes();
    uint32 level = (uint32)(product >> 8) << 6;
    attr->mask = 3;
    attr->voice = 1u << (voice_idx & 31u);
    attr->volume.left = level;
    attr->volume.right = level;
    XASSERT(game_services.spu_voice_attributes);
    result = game_services.spu_voice_attributes(attr);
    XASSERT(voice_idx >= 0 && voice_idx < SOUND_VOICE_COUNT);
    SOUND_VOICE *voice = sound_voices.voice + voice_idx;
    voice->left = product >> 9;
    voice->right = product >> 9;
    return result;
}

/* Preserve the original two successive master products; the second argument is ignored */
sint32 sound_voice_restore_volume(sint32 voice_idx, sint32 unused, sint32 right)
{
    FUNCTION_MARKER(0x80056f94, "GAME.EXE");
    uint32 master = sound_settings.effects_volume;
    SpuVoiceAttr *attr;
    sint32 left_product, right_product, result;
    left_product = right * master;
    attr = sound_voice_next_attributes();
    right_product = (left_product >> 8) * master;
    attr->mask = 3;
    attr->volume.left = (uint32)(left_product >> 8) << 6;
    attr->voice = 1u << (voice_idx & 31u);
    attr->volume.right = (uint32)(right_product >> 8) << 6;
    XASSERT(game_services.spu_voice_attributes);
    result = game_services.spu_voice_attributes(attr);
    XASSERT(voice_idx >= 0 && voice_idx < SOUND_VOICE_COUNT);
    SOUND_VOICE *voice = sound_voices.voice + voice_idx;
    voice->left = left_product >> 9;
    voice->right = right_product >> 9;
    return result;
}

sint32 sound_voices_pause(void)
{
    FUNCTION_MARKER(0x8005703c, "GAME.EXE");
    sint32 i, type;
    SpuVoiceAttr *attr;
    for (i = 0; i < SOUND_VOICE_COUNT; i++)
    {
        type = sound_voices.voice[i].mode;
        if (type == 1)
            sound_voice_stop(i);
        else if (type == 2)
        {
            SpuVoiceAttr current = {.voice = 1u << i};
            SpuGetVoiceAttr(&current);
            paused_voice_volume[i][0] = current.volume.left;
            paused_voice_volume[i][1] = current.volume.right;
            attr = sound_voice_next_attributes();
            attr->mask = 3;
            attr->voice = 1u << i;
            attr->volume.right = 0;
            attr->volume.left = 0;
            XASSERT(game_services.spu_voice_attributes);
            game_services.spu_voice_attributes(attr);
        }
    }
    return 0;
}

sint32 sound_voices_resume(void)
{
    FUNCTION_MARKER(0x800570f0, "GAME.EXE");
    sint32 i;
    for (i = 0; i < SOUND_VOICE_COUNT; i++)
        if (sound_voices.voice[i].mode == 2)
        {
            // LEGACY FIX: restore both saved channels without applying master volume again
            SpuVoiceAttr *attr = sound_voice_next_attributes();
            attr->voice = 1u << i;
            attr->mask = 3;
            attr->volume.left = paused_voice_volume[i][0];
            attr->volume.right = paused_voice_volume[i][1];
            XASSERT(game_services.spu_voice_attributes);
            game_services.spu_voice_attributes(attr);
        }
    return 0;
}

/* Startup phase of GAME.EXE:80056BC8, not a separate original function */
sint32 sound_voice_defaults(SpuVoiceAttr *desc)
{
    uint32 i;
    desc->mask = 0xff1f;
    desc->pitch = 0x1000;
    desc->voice = 0xffffff;
    desc->volume.left = 0x3fff;
    desc->volume.right = 0x3fff;
    desc->volmode.right = 0;
    desc->volmode.left = 0;
    desc->a_mode = 1;
    desc->s_mode = 1;
    desc->r_mode = 3;
    desc->ar = 0;
    desc->dr = 0;
    desc->sr = 0;
    desc->rr = 0;
    desc->sl = 0;
    XASSERT(game_services.spu_voice_attributes && game_services.spu_key);
    game_services.spu_voice_attributes(desc);
    for (i = 0; i < SOUND_VOICE_COUNT; i++)
        game_services.spu_key(0, 1u << i);
    /* The original final loop condition leaves zero in the result register */
    return 0;
}
