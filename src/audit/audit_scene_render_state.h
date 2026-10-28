#ifndef AUDIT_SCENE_RENDER_STATE_H
#define AUDIT_SCENE_RENDER_STATE_H

#include "../game.h"
#include <stdio.h>
#include <string.h>

#if !defined(NDEBUG) && !defined(__EMSCRIPTEN__)
/* Renderer audit input boundary; the caller restores actual GTE input */
static int diagnostic_scene_render_state_read(FILE *input, uint32 gte[64])
{
    uint32 header[8], seed[6], words[8], record[2], i, j, previous = 0;
    if (!input || !gte || fread(header, sizeof(header), 1, input) != 1 || header[0] != 0x35524746 || header[1] != 1 || header[2] != GEOMETRY_MATRIX_COUNT || header[3] != GEOMETRY_MODEL_MATRIX_COUNT || header[4] != DEPTH_TABLE_COUNT || header[5] != TEXTURE_MAP_COUNT || !header[6] || header[6] > TEXTURE_DESCRIPTOR_CAPACITY * 3 || header[7] != 6 || fread(seed, sizeof(seed), 1, input) != 1 || seed[4] > TEXTURE_MAP_COUNT || seed[5] > TEXTURE_DESCRIPTOR_CAPACITY || fread(gte, 64 * sizeof(uint32), 1, input) != 1)
        return 0;
    for (i = 0; i < GEOMETRY_MATRIX_COUNT + GEOMETRY_MODEL_MATRIX_COUNT; i++)
    {
        POSE_MATRIX *matrix = i < GEOMETRY_MATRIX_COUNT ? geometry_workspace.scratch.matrix + i : geometry_workspace.model.matrix + (i - GEOMETRY_MATRIX_COUNT);
        if (fread(words, sizeof(words), 1, input) != 1)
            return 0;
        for (j = 0; j < 5; j++)
            matrix->rotation_words[j] = words[j];
        for (j = 0; j < 3; j++)
            matrix->t[j] = (sint32)words[j + 5];
    }
    if (fread(depth_table, sizeof(depth_table), 1, input) != 1 || fread(tex_map, sizeof(tex_map), 1, input) != 1)
        return 0;
    memset(&tex_desc, 0xa5, sizeof(tex_desc));
    for (i = 0; i < header[6]; i++)
    {
        if (fread(record, sizeof(record), 1, input) != 1 || record[0] >= TEXTURE_DESCRIPTOR_CAPACITY * 3 || (i && record[0] <= previous))
            return 0;
        previous = record[0];
        tex_desc.words[record[0]] = record[1];
    }
    if (fgetc(input) != EOF)
        return 0;
    render_style.color.rgbc = seed[0];
    render_style.blend_mode = seed[1];
    random_seed = seed[2];
    dir_distance = (sint32)seed[3];
    tex_count = seed[4];
    tex_desc_count = seed[5];
    geometry_workspace.matrix = NULL;
    geometry_workspace.matrix_storage = GEOMETRY_MATRIX_NULL;
    return 1;
}

/* Observe numeric workspace fields and actual owner/slot identity */
static int diagnostic_scene_render_state_write(FILE *output)
{
    const uint32 header[4] = {0x354f4746, 1, GEOMETRY_MATRIX_COUNT, GEOMETRY_MODEL_MATRIX_COUNT};
    uint32 seed[6] = {render_style.color.rgbc, render_style.blend_mode, random_seed, (uint32)dir_distance, 2, 0xffffffff};
    uint32 words[8], i, j;
    if (geometry_workspace.matrix)
    {
        for (i = 0; i < GEOMETRY_MATRIX_COUNT; i++)
            if (geometry_workspace.matrix == geometry_workspace.scratch.matrix + i)
            {
                seed[4] = GEOMETRY_MATRIX_SCRATCH;
                seed[5] = i;
            }
        for (i = 0; i < GEOMETRY_MODEL_MATRIX_COUNT; i++)
            if (geometry_workspace.matrix == geometry_workspace.model.matrix + i)
            {
                seed[4] = GEOMETRY_MATRIX_MODEL;
                seed[5] = i;
            }
        if (seed[4] == GEOMETRY_MATRIX_NULL)
            return 0;
    }
    if (geometry_workspace.matrix_storage != seed[4] || !output || fwrite(header, sizeof(header), 1, output) != 1 || fwrite(seed, sizeof(seed), 1, output) != 1)
        return 0;
    for (i = 0; i < GEOMETRY_MATRIX_COUNT + GEOMETRY_MODEL_MATRIX_COUNT; i++)
    {
        const POSE_MATRIX *matrix = i < GEOMETRY_MATRIX_COUNT ? geometry_workspace.scratch.matrix + i : geometry_workspace.model.matrix + (i - GEOMETRY_MATRIX_COUNT);
        for (j = 0; j < 5; j++)
            words[j] = matrix->rotation_words[j];
        for (j = 0; j < 3; j++)
            words[j + 5] = (uint32)matrix->t[j];
        if (fwrite(words, sizeof(words), 1, output) != 1)
            return 0;
    }
    return 1;
}
#endif
#endif
