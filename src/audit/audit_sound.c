/* Controlled native sound comparison, never used by the game frame loop */
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include "game.h"
#include "psx_spu.h"

#if !defined(NDEBUG) && !defined(__EMSCRIPTEN__)
enum
{
    SOUND_BASE_WORDS = 852,
    SOUND_OBSERVATION_WORDS = 64 + SOUND_BASE_WORDS + 2 + SOUND_VOICE_COUNT
};

static uint32 trace[64];

static uint32 *event(uint32 kind)
{
    uint32 n = trace[1]++;
    if (n >= 3)
        abort();
    trace[2 + 20 * n] = kind;
    return trace + 2 + 20 * n;
}

static uint32 attribute_slot(const SpuVoiceAttr *packet)
{
    uint32 slot;
    for (slot = 0; slot < SOUND_VOICE_ATTRIBUTE_COUNT; slot++)
        if (packet == &sound_attributes.packet[slot].attr)
            return slot;
    abort();
}

static uint32 key(sint32 enable, uint32 mask)
{
    uint32 *e = event(1);
    e[1] = enable;
    e[2] = mask;
    return 0x1357;
}

static sint32 attr(SpuVoiceAttr *a)
{
    uint32 *e = event(2);
    e[1] = attribute_slot(a);
    memcpy(e + 3, a, SOUND_VOICE_ATTRIBUTE_BYTES);
    return 0x2468;
}

static sint32 on(SpuVoiceAttr *a)
{
    uint32 *e = event(3);
    e[1] = attribute_slot(a);
    memcpy(e + 3, a, SOUND_VOICE_ATTRIBUTE_BYTES);
    return 0x3579;
}

/* This finite test codec contains metadata and SDK packets, never a game RAM image */
static void install_baseline(const uint32 *words, MODEL_OBJECT *model)
{
    uint32 i, used = 0;
    sound_samples.spu_base = words[used++];
    sound_settings.effects_volume = words[used++];
    camera.pos.x.bits = words[used++];
    camera.pos.y.bits = words[used++];
    camera.pos.z.bits = words[used++];
    camera.angles.yaw_bits = words[used++];
    menu.ambient_voice[0] = words[used++];
    menu.ambient_voice[1] = words[used++];
    model->kind = words[used++];
    model->pos.x.bits = words[used++];
    model->pos.y.bits = words[used++];
    model->pos.z.bits = words[used++];
    model->angles.yaw_bits = words[used++];
    model->angles.pitch_bits = words[used++];
    model->angles.roll_bits = words[used++];
    dir_distance = words[used++];
    for (i = 0; i < SOUND_SAMPLE_COUNT; i++)
    {
        sound_samples.sample[i].offset = words[used++];
        sound_samples.sample[i].mode = words[used++];
    }
    for (i = 0; i < SOUND_VOICE_COUNT; i++)
    {
        sound_voices.voice[i].left = words[used++];
        sound_voices.voice[i].right = words[used++];
    }
    for (i = 0; i < WORLD_REGION_KIND_COUNT; i++)
        world_region_kinds[i].flags = words[used++];
    for (i = 0; i < SOUND_VOICE_ATTRIBUTE_COUNT; i++)
    {
        memcpy(sound_attributes.packet[i].wire, words + used, SOUND_VOICE_ATTRIBUTE_BYTES);
        used += SOUND_VOICE_ATTRIBUTE_BYTES / sizeof(uint32);
    }
    if (used != SOUND_BASE_WORDS)
        abort();
}

static void observe_baseline(uint32 *words, const MODEL_OBJECT *model)
{
    uint32 i, used = 0;
    words[used++] = sound_samples.spu_base;
    words[used++] = sound_settings.effects_volume;
    words[used++] = camera.pos.x.bits;
    words[used++] = camera.pos.y.bits;
    words[used++] = camera.pos.z.bits;
    words[used++] = camera.angles.yaw_bits;
    words[used++] = menu.ambient_voice[0];
    words[used++] = menu.ambient_voice[1];
    words[used++] = model->kind;
    words[used++] = model->pos.x.bits;
    words[used++] = model->pos.y.bits;
    words[used++] = model->pos.z.bits;
    words[used++] = model->angles.yaw_bits;
    words[used++] = model->angles.pitch_bits;
    words[used++] = model->angles.roll_bits;
    words[used++] = dir_distance;
    for (i = 0; i < SOUND_SAMPLE_COUNT; i++)
    {
        words[used++] = sound_samples.sample[i].offset;
        words[used++] = sound_samples.sample[i].mode;
    }
    for (i = 0; i < SOUND_VOICE_COUNT; i++)
    {
        words[used++] = (uint8)sound_voices.voice[i].left;
        words[used++] = (uint8)sound_voices.voice[i].right;
    }
    for (i = 0; i < WORLD_REGION_KIND_COUNT; i++)
        words[used++] = (uint8)world_region_kinds[i].flags;
    for (i = 0; i < SOUND_VOICE_ATTRIBUTE_COUNT; i++)
    {
        memcpy(words + used, sound_attributes.packet[i].wire, SOUND_VOICE_ATTRIBUTE_BYTES);
        used += SOUND_VOICE_ATTRIBUTE_BYTES / sizeof(uint32);
    }
    if (used != SOUND_BASE_WORDS)
        abort();
}

extern void diagnostic_end(void);

int audit_sound_run(void)
{
    FILE *jobs, *out, *input;
    uint32 j[8], baseline[SOUND_BASE_WORDS], input_header[4], i, used;
    uint32 observation[SOUND_OBSERVATION_WORDS];
    const uint32 baseline_header[4] = {0x32425346, 1, SOUND_BASE_WORDS, 0};
    const uint32 header[4] = {0x32534e46, 1, SOUND_OBSERVATION_WORDS, 0};
    size_t got = 0;
    int result = 0;
    MODEL_OBJECT model, unchanged_model;
    input = xport_fopen("../status/menu/sound-baseline.bin", "rb");
    if (!input)
        return 4;
    if (fread(input_header, sizeof(input_header), 1, input) != 1 || memcmp(input_header, baseline_header, sizeof(input_header)) || fread(baseline, sizeof(baseline), 1, input) != 1 || fgetc(input) != EOF || ferror(input))
        result = 4;
    if (fclose(input))
        result = 4;
    if (result)
        return result;
    jobs = xport_fopen("../status/menu/sound-jobs.bin", "rb");
    out = xport_fopen("../status/menu/sound-native.bin", "wb");
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
    game_services.spu_key = key;
    game_services.spu_voice_attributes = attr;
    game_services.spu_key_on_attributes = on;
    while (!result && (got = fread(j, 1, sizeof(j), jobs)) == sizeof(j))
    {
        memset(&model, 0, sizeof(model));
        install_baseline(baseline, &model);
        sound_attributes.cur = j[4];
        sound_voices.cur = j[5];
        for (i = 0; i < SOUND_VOICE_COUNT; i++)
            sound_voices.voice[i].mode = (j[6] & (1u << i)) ? 2 : 0;
        if (j[7])
        {
            if (j[1] >= SOUND_SAMPLE_COUNT)
            {
                result = 5;
                break;
            }
            sound_samples.sample[j[1]].offset = 0xffffffff;
        }
        if (j[0] == 0x80043520)
        {
            if (j[1] != 0x800b8b10)
            {
                result = 5;
                break;
            }
            model.angles.pitch_bits = j[2];
        }
        memcpy(&unchanged_model, &model, sizeof(model));
        memset(trace, 0, sizeof(trace));
        if (j[0] == 0x80056640)
            trace[0] = attribute_slot(sound_voice_next_attributes());
        else if (j[0] == 0x80056670)
            trace[0] = sound_voice_play(j[1], j[2], j[3]);
        else if (j[0] == 0x800567f8)
            trace[0] = sound_voice_stop(j[1]);
        else if (j[0] == 0x80056840)
            trace[0] = sound_voice_pitch(j[1], j[2]);
        else if (j[0] == 0x80056f08)
            trace[0] = sound_voice_volume(j[1], j[2]);
        else if (j[0] == 0x800568c8)
            trace[0] = sound_voice_pan(j[1], j[2], j[3]);
        else if (j[0] == 0x80043520)
            trace[0] = object_update_orbit_sound(&model);
        else
        {
            result = 5;
            break;
        }
        /* Common fields are observed; every other native model byte must remain unchanged */
        unchanged_model.kind = model.kind;
        unchanged_model.pos = model.pos;
        unchanged_model.angles = model.angles;
        if (memcmp(&unchanged_model, &model, sizeof(model)))
            abort();
        memcpy(observation, trace, sizeof(trace));
        observe_baseline(observation + 64, &model);
        used = 64 + SOUND_BASE_WORDS;
        observation[used++] = sound_attributes.cur;
        observation[used++] = sound_voices.cur;
        for (i = 0; i < SOUND_VOICE_COUNT; i++)
            observation[used++] = (uint8)sound_voices.voice[i].mode;
        if (used != SOUND_OBSERVATION_WORDS)
            abort();
        if (fwrite(observation, sizeof(observation), 1, out) != 1)
            result = 4;
    }
    if (ferror(jobs) || (!result && got))
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
