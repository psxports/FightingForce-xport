#include "game.h"

STAGE_AUDIO_SLOT stage_audio_slots[STAGE_AUDIO_SLOT_COUNT];

sint32 stage_audio_start(void)
{
    FUNCTION_MARKER(0x80014584, "GAME.EXE");
    uint32 stage = game_session.stage;
    if (stage - 8u < 2u)
        return sound_voice_play(141, 64, 6144);
    if (stage == 0 || stage == 5 || stage - 10u < 2u || stage == 15)
        return sound_voice_play(141, 170, 6144);
    if (stage == 6)
    {
        sound_voice_play(154, 128, 6144);
        sound_voice_play(156, 255, 6144);
        sound_voice_play(155, 255, 6144);
        sound_voice_play(158, 80, 6144);
        return sound_voice_play(141, 96, 6144);
    }
    if (stage == 19)
    {
        sound_voice_play(118, 255, 6144);
        return sound_voice_play(235, 80, 6144);
    }
    if (stage == 1 || stage == 3 || stage == 7 || stage - 12u < 2u || stage == 24)
        return sound_voice_play(118, 255, 6144);
    if (stage == 2)
    {
        sint32 result = sound_voice_play(126, 96, 5120);
        stage_audio_slots[0].voice = result;
        return result;
    }
    if (stage == 14)
    {
        sint32 result = sound_voice_play(215, 80, 6144);
        stage_audio_slots[0].voice = result;
        result = sound_voice_play(184, 0, 6144);
        stage_audio_slots[1].voice = result;
        return result;
    }
    if (stage == 16 || stage == 21)
    {
        sint32 result = sound_voice_play(235, 128, 6144);
        stage_audio_slots[0].voice = result;
        return result;
    }
    if (stage == 17)
    {
        sint32 result = sound_voice_play(214, 255, 6144);
        stage_audio_slots[0].voice = result;
        return result;
    }
    stage = game_session.stage;
    if (stage == 18)
    {
        sint32 result = sound_voice_play(235, 128, 6144);
        stage_audio_slots[0].voice = result;
        result = sound_voice_play(236, 64, 6144);
        stage_audio_slots[1].voice = result;
        return result;
    }
    if (stage == 20)
    {
        sint32 result;
        sound_voice_play(141, 170, 6144);
        result = sound_voice_play(188, 0, 5120);
        stage_audio_slots[0].voice = result;
        stage_audio_slots[1].level = 0;
        return result;
    }
    if (stage == 22)
    {
        sound_voice_play(239, 90, 5120);
        return sound_voice_play(240, 90, 3840);
    }
    if (stage == 23)
    {
        sint32 result = sound_voice_play(126, 96, 5120);
        stage_audio_slots[0].voice = result;
        result = sound_voice_play(235, 64, 6144);
        stage_audio_slots[1].voice = result;
        return result;
    }
    return 23;
}
