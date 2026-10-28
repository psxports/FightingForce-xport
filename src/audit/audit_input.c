/* Controlled native input comparison, never used by the game frame loop */
#include <setjmp.h>
#include <stdio.h>
#include <string.h>

#include "game.h"

#if !defined(NDEBUG) && !defined(__EMSCRIPTEN__)
enum
{
    INPUT_TRACE_WORDS = 7,
    INPUT_STATE_WORDS = 29
};

enum
{
    INPUT_OBSERVATION_WORDS = INPUT_TRACE_WORDS + INPUT_STATE_WORDS + 24 + 66 + 3 * SOUND_VOICE_COUNT
};

static jmp_buf audit_jump;
static uint32 trace[INPUT_TRACE_WORDS];

static uint32 record_key(sint32 enabled, uint32 channels)
{
    trace[1] = enabled;
    trace[2] = channels;
    trace[5] = trace[5] * 4 + 1;
    trace[6]++;
    return 0;
}

static void record_frontend_return(void)
{
    trace[3] = 1;
    trace[5] = trace[5] * 4 + 2;
    trace[6]++;
    longjmp(audit_jump, 1);
}

extern void diagnostic_end(void);

int audit_input_run(void)
{
    FILE *jobs, *out;
    uint32 j[10], voice_idx, i, used;
    uint32 observation[INPUT_OBSERVATION_WORDS];
    const uint32 header[4] = {0x32494e46, 1, INPUT_OBSERVATION_WORDS, 0};
    size_t got;
    int result = 0;
    jobs = xport_fopen("../status/menu/input-jobs.bin", "rb");
    out = xport_fopen("../status/menu/input-native.bin", "wb");
    if (!jobs || !out)
    {
        if (jobs)
            fclose(jobs);
        if (out)
            fclose(out);
        return 4;
    }
    if (fwrite(header, sizeof(header), 1, out) != 1)
        result = 4;
    game_services.spu_key = record_key;
    game_services.return_to_frontend = record_frontend_return;
    while (!result && (got = fread(j, 1, sizeof(j), jobs)) == sizeof(j))
    {
        if (j[1] >= PAD_PLAYER_COUNT)
        {
            result = 5;
            break;
        }
        /* Match the original job's zeroed owners and explicit sentinels */
        memset(&sound_settings, 0, sizeof(sound_settings));
        memset(&sound_voices, 0, sizeof(sound_voices));
        memset(&game_session, 0, sizeof(game_session));
        memset(&disp_buffers, 0, sizeof(disp_buffers));
        game_settings.scr_offset.x = 0;
        game_settings.scr_offset.y = 0;
        text_hint = TEXT_HINT_SELECT_CANCEL;
        memset(&action_history, 0, sizeof(action_history));
        memset(&pad_decode, 0, sizeof(pad_decode));
        memset(&pad_action, 0, sizeof(pad_action));
        memset(&pad_stream, 0, sizeof(pad_stream));
        memset(&resource_io, 0, sizeof(resource_io));
        memset(&resource_archive, 0, sizeof(resource_archive));
        memset(&pad_game, 0, sizeof(pad_game));
        memset(pad_reports, 0, sizeof(pad_reports));
        memcpy(&pad_reports[0], j + 2, 8);
        memcpy(&pad_reports[1], j + 4, 8);
        pad_decode.disconnect[j[1]] = j[6];
        pad_decode.unsupported[j[1]] = j[7];
        pad_decode.reset_count[j[1]] = j[8];
        pad_decode.reset_blocked = j[9];
        pad_decode.x = 0x12345678;
        pad_decode.y = (sint32)0x87654321u;
        for (voice_idx = 0; voice_idx < SOUND_VOICE_COUNT; voice_idx++)
            sound_voices.voice[voice_idx].mode = (sint8)0xaa;
        memset(trace, 0, sizeof(trace));
        if (setjmp(audit_jump))
            trace[0] = 0x7ffffffe;
        else if (j[0] == 0x8005f5ac)
            trace[0] = pad_decode_buttons(j[1]);
        else if (j[0] == 0x800573a8)
            trace[0] = pad_count_devices();
        else if (j[0] == 0x8005ec90)
            trace[0] = pad_refresh_ports();
        else
        {
            result = 5;
            break;
        }
        uint32 state[] = {
            pad_decode.type, pad_decode.reset_blocked, pad_decode.x, pad_decode.y, pad_decode.reset_count[0], pad_decode.reset_count[1], pad_decode.disconnect[0], pad_decode.disconnect[1], pad_decode.unsupported[0], pad_decode.unsupported[1], pad_decode.count, pad_game.port[0], pad_game.port[1], pad_game.sample[0], pad_game.sample[1], pad_game.prev[0], pad_game.prev[1], text_hint, sound_settings.flags, sound_settings.effects_volume, sound_settings.music_volume, game_settings.scr_offset.x, game_settings.scr_offset.y, disp_buffers.shake_ticks, game_session.stage, game_session.mode, game_session.scene_phase, game_session.tick, game_session.player_count,
        };
        _Static_assert(sizeof(state) == INPUT_STATE_WORDS * sizeof(uint32), "Input state wire words");
        _Static_assert(SOUND_VOICE_COUNT == 24, "Original input reset voice count");
        memcpy(observation, trace, sizeof(trace));
        used = INPUT_TRACE_WORDS;
        memcpy(observation + used, state, sizeof(state));
        used += INPUT_STATE_WORDS;
        for (i = 0; i < 2; i++)
        {
            const DISPENV *env = disp_buffers.env + i;
            uint32 fields[] = {env->disp.x, env->disp.y, env->disp.w, env->disp.h, env->screen.x, env->screen.y, env->screen.w, env->screen.h, env->isinter, env->isrgb24, env->pad0, env->pad1};
            _Static_assert(sizeof(fields) == 12 * sizeof(uint32), "Display observation words");
            memcpy(observation + used, fields, sizeof(fields));
            used += 12;
        }
        for (i = 0; i < PAD_PLAYER_COUNT; i++)
        {
            const PAD_REPORT *pad = pad_reports + i;
            uint32 fields[] = {pad->status, pad->id, pad->buttons, pad->right_x, pad->right_y, pad->left_x, pad->left_y};
            uint32 tail;
            _Static_assert(sizeof(fields) == 7 * sizeof(uint32), "PAD observation words");
            memcpy(observation + used, fields, sizeof(fields));
            used += 7;
            for (tail = 0; tail < sizeof(pad->transport_tail); tail++)
                observation[used++] = pad->transport_tail[tail];
        }
        for (voice_idx = 0; voice_idx < SOUND_VOICE_COUNT; voice_idx++)
        {
            const SOUND_VOICE *voice = sound_voices.voice + voice_idx;
            observation[used++] = (uint8)voice->left;
            observation[used++] = (uint8)voice->mode;
            observation[used++] = (uint8)voice->right;
        }
        if (used != INPUT_OBSERVATION_WORDS)
        {
            result = 5;
            break;
        }
        if (fwrite(observation, sizeof(observation), 1, out) != 1)
            result = 4;
    }
    if (!result && (got || ferror(jobs)))
        result = 4;
    if (fclose(jobs))
        result = 4;
    if (fclose(out))
        result = 4;
    if (!result)
        diagnostic_end();
    return result;
}
#endif
