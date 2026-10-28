#ifndef AUDIT_SCENE_MODEL_VIEWS_H
#define AUDIT_SCENE_MODEL_VIEWS_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "../resources.h"

#if !defined(NDEBUG) && !defined(__EMSCRIPTEN__)
/* Audit-only input boundary for the complete renderer fixture */
static int diagnostic_scene_models_read(FILE *input, RESOURCE_FILE *owner)
{
    uint32 header[6], records[46][7], i, j;
    MODEL_REFERENCE *views;
    uint8 *data;
    if (!input || !owner || owner->data || owner->capacity || owner->byte_len || fread(header, sizeof(header), 1, input) != 1 || header[0] != 0x35564d46 || header[1] != 1 || header[2] != 86964 || header[3] <= header[2] || header[3] > RESOURCE_STORAGE_CAPACITY || header[4] != 46 || header[5] != 4 || fread(records, sizeof(records), 1, input) != 1)
        return 0;
    views = calloc(MODEL_TYPE_COUNT, sizeof(*views));
    data = malloc(header[3]);
    if (!views || !data || fread(data, header[3], 1, input) != 1 || fgetc(input) != EOF)
    {
        free(views);
        free(data);
        return 0;
    }
    for (i = 0; i < 46; i++)
    {
        uint32 *row = records[i];
        if (row[0] >= MODEL_TYPE_COUNT || (i && records[i - 1][0] >= row[0]) || row[1] > header[3] || row[2] < sizeof(MODEL_BODY_HEADER) || row[2] > header[3] - row[1] || row[3] > header[3] || row[4] > header[3] - row[3] || row[5] > header[3] || row[6] > header[3] - row[5] || (row[1] & 3) || (row[3] & 3) || (row[5] & 3) || row[3] < row[1] || row[5] < row[1] || row[3] > row[1] + row[2] || row[5] > row[1] + row[2] || row[4] != row[1] + row[2] - row[3] || row[6] != row[1] + row[2] - row[5])
            break;
        MODEL_REFERENCE *model = views + row[0];
        model->header = (MODEL_BODY_HEADER *)(data + row[1]);
        model->byte_len = row[2];
        model->vertices = (SVECTOR *)(data + row[3]);
        model->vertex_byte_len = row[4];
        model->faces = (MODEL_FACE_RECORD *)(data + row[5]);
        model->face_byte_len = row[6];
        if (model->header->vertex_count < 0 || model->header->face_count < 0 || (uint32)model->header->vertex_count > row[4] / sizeof(SVECTOR) || (uint32)model->header->face_count > row[6] / sizeof(MODEL_FACE_RECORD))
            break;
        if (row[0] < 4)
        {
            if (row[1] < header[2] || row[2] < sizeof(MODEL_RIG_HEADER))
                break;
        }
        else if (row[1] >= header[2] || row[2] > header[2] - row[1])
            break;
    }
    if (i != 46)
    {
        free(data);
        free(views);
        return 0;
    }
    for (j = 0; j < MODEL_TYPE_COUNT; j++)
        if (model_refs[j].header || model_refs[j].vertices || model_refs[j].faces)
            break;
    if (j != MODEL_TYPE_COUNT)
    {
        free(data);
        free(views);
        return 0;
    }
    memcpy(model_refs, views, sizeof(*views) * MODEL_TYPE_COUNT);
    free(views);
    owner->data = data;
    owner->byte_len = owner->capacity = header[3];
    return 1;
}
#endif
#endif
