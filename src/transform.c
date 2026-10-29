#include <stdio.h>
#include <stddef.h>
#include <string.h>

#include "game.h"

GEOMETRY_WORKSPACE geometry_workspace;

_Static_assert(sizeof(GEOMETRY_SCRATCH) == sizeof(geometry_workspace.scratch.depth), "Complete scratch scalar span");
_Static_assert(sizeof(GEOMETRY_MODEL_BUFFER) == sizeof(geometry_workspace.model.depth), "Complete model projection scalar span");
_Static_assert(offsetof(GEOMETRY_MODEL_BUFFER, proj.xy) == sizeof(geometry_workspace.model.proj.depth_prefix), "Model projection view overlap");

/* Rotation columns retain the original MVMVA and IR read pipeline */
static void rotate_columns(POSE_MATRIX *matrix)
{
    const POSE_MATRIX *rotation = &geometry_workspace.scratch.matrix[GEOMETRY_MATRIX_COUNT - 1];
    uint32 rotation_words[5], col0[3], col1[3], col2_x, col2_y, x, y, z;
    int i;
    for (i = 0; i < 5; i++)
        rotation_words[i] = matrix->rotation_words[i];
    for (i = 0; i < 5; i++)
        xport_gte_write_control(i, rotation_words[i]);
    y = (uint16)rotation->m[1][0];
    x = (uint16)rotation->m[0][0];
    z = rotation->rotation_words[3];
    xport_gte_write_data(0, x | (y << 16));
    xport_gte_write_data(1, z);
    xport_gte_execute(0x486012);
    y = (uint16)rotation->m[1][1];
    x = (uint16)rotation->m[0][1];
    z = (uint32)(sint32)rotation->m[2][1];
    for (i = 0; i < 3; i++)
        col0[i] = xport_gte_read_data(9 + i);
    xport_gte_write_data(0, x | (y << 16));
    xport_gte_write_data(1, z);
    xport_gte_execute(0x486012);
    y = (uint16)rotation->m[1][2];
    x = (uint16)rotation->m[0][2];
    z = rotation->rotation_words[4];
    for (i = 0; i < 3; i++)
        col1[i] = xport_gte_read_data(9 + i);
    xport_gte_write_data(0, x | (y << 16));
    xport_gte_write_data(1, z);
    xport_gte_execute(0x486012);
    matrix->rotation_words[0] = (col0[0] & 65535) | (col1[0] << 16);
    matrix->rotation_words[3] = (col0[2] & 65535) | (col1[2] << 16);
    col2_x = xport_gte_read_data(9);
    col2_y = xport_gte_read_data(10);
    matrix->rotation_words[1] = (col2_x & 65535) | (col0[1] << 16);
    matrix->rotation_words[2] = (col1[1] & 65535) | (col2_y << 16);
    matrix->rotation_words[4] = xport_gte_read_data(11);
}

static void load_gte(const uint32 words[8])
{
    int i;
    for (i = 0; i < 5; i++)
        xport_gte_write_control(i, words[i]);
    for (i = 5; i < 8; i++)
        xport_gte_write_control(i, (sint32)words[i] >> 16);
}

/* 80010028 only advances the pointer: it does not reload GTE registers. */
GDB_CALL void transform_pop(void)
{
    FUNCTION_MARKER(0x80010028, "GAME.EXE");
    /* TODO: Investigate geometry entered before its current matrix is published */
    XASSERT(geometry_workspace.matrix);
    geometry_workspace.matrix++;
}

/* 80010040 snapshots all eight source words before writing the lower slot. */
GDB_CALL void transform_push(void)
{
    FUNCTION_MARKER(0x80010040, "GAME.EXE");
    uint32 words[8];
    /* TODO: Investigate geometry entered before its current matrix is published */
    XASSERT(geometry_workspace.matrix);
    const POSE_MATRIX *src = geometry_workspace.matrix;
    POSE_MATRIX *dst;
    int i;
    for (i = 0; i < 8; i++)
        words[i] = i < 5 ? src->rotation_words[i] : src->t[i - 5];
    dst = geometry_workspace.matrix - 1;
    for (i = 0; i < 5; i++)
        dst->rotation_words[i] = words[i];
    for (i = 5; i < 8; i++)
        dst->t[i - 5] = words[i];
    load_gte(words);
    geometry_workspace.matrix = dst;
}

/* 800100C4: MVMVA results use MAC1..3, not saturated IR1..3. Translation
 * fractions are discarded before the operation; stores wrap after SLL 16. */
GDB_CALL void transform_translate(sint32 x, sint32 y, sint32 z)
{
    FUNCTION_MARKER(0x800100c4, "GAME.EXE");
    /* TODO: Investigate geometry entered before its current matrix is published */
    XASSERT(geometry_workspace.matrix);
    POSE_MATRIX *matrix = geometry_workspace.matrix;
    uint32 words[8], result[3];
    int i;
    for (i = 0; i < 8; i++)
        words[i] = i < 5 ? matrix->rotation_words[i] : matrix->t[i - 5];
    load_gte(words);
    xport_gte_write_data(0, ((uint32)x >> 16) | (y & 0xffff0000));
    xport_gte_write_data(1, z >> 16);
    xport_gte_execute(0x480012);
    for (i = 0; i < 3; i++)
        result[i] = xport_gte_read_data(25 + i) << 16;
    for (i = 0; i < 3; i++)
        matrix->t[i] = result[i];
}

/* 80010170: reload geometry registers from the current matrix, without moving
 * its pointer or modifying the matrix. Translation fractions are discarded. */
GDB_CALL void transform_reload(void)
{
    FUNCTION_MARKER(0x80010170, "GAME.EXE");
    /* TODO: Investigate geometry entered before its current matrix is published */
    XASSERT(geometry_workspace.matrix);
    const POSE_MATRIX *src = geometry_workspace.matrix;
    uint32 words[8];
    int i;
    for (i = 0; i < 8; i++)
        words[i] = i < 5 ? src->rotation_words[i] : src->t[i - 5];
    load_gte(words);
}

GDB_CALL void transform_pitch(sint32 angle)
{
    FUNCTION_MARKER(0x800101cc, "GAME.EXE");
    uint32 idx;
    XASSERT((uint32)angle != 0x80000000u);
    idx = (((uint32)(-(sint64)(angle))) >> 2) & 0x3ffc;
    if (idx)
        transform_pitch_idx(idx);
}

GDB_CALL void transform_yaw(sint32 angle)
{
    FUNCTION_MARKER(0x8001033c, "GAME.EXE");
    uint32 idx;
    XASSERT((uint32)angle != 0x80000000u);
    idx = (((uint32)(-(sint64)(angle))) >> 2) & 0x3ffc;
    if (idx)
        transform_yaw_idx(idx);
}

GDB_CALL void transform_roll(sint32 angle)
{
    FUNCTION_MARKER(0x800104a0, "GAME.EXE");
    uint32 idx;
    XASSERT((uint32)angle != 0x80000000u);
    idx = (((uint32)(-(sint64)(angle))) >> 2) & 0x3ffc;
    if (idx)
        transform_roll_idx(idx);
}

/* Table pitch uses three rotation-only MVMVA columns */
GDB_CALL void transform_pitch_idx(uint32 idx)
{
    POSE_MATRIX *matrix;
    POSE_MATRIX *rotation = &geometry_workspace.scratch.matrix[GEOMETRY_MATRIX_COUNT - 1];
    sint32 s, c;
    /* Reject invalid packed Q12 offsets before accessing the table */
    XASSERT((idx & 3u) == 0 && idx <= 16380u);
    rsincos(idx >> 2, &s, &c);
    /* TODO: Investigate geometry entered before its current matrix is published */
    XASSERT(geometry_workspace.matrix);
    matrix = geometry_workspace.matrix;
    rotation->rotation_words[0] = ONE;
    rotation->rotation_words[1] = 0;
    rotation->m[1][1] = (sint16)c;
    rotation->m[1][2] = (sint16)s;
    rotation->m[2][0] = 0;
    rotation->m[2][1] = (sint16)-s;
    /* The original packed cosine word zero-extends its signed halfword */
    rotation->rotation_words[4] = (uint16)c;
    rotate_columns(matrix);
}

/* 8001033C: same column multiply as pitch, with a different scratch matrix. */
GDB_CALL void transform_yaw_idx(uint32 idx)
{
    POSE_MATRIX *matrix;
    POSE_MATRIX *rotation = &geometry_workspace.scratch.matrix[GEOMETRY_MATRIX_COUNT - 1];
    sint32 s, c;
    /* Reject invalid packed Q12 offsets before accessing the table */
    XASSERT((idx & 3u) == 0 && idx <= 16380u);
    rsincos(idx >> 2, &s, &c);
    /* TODO: Investigate geometry entered before its current matrix is published */
    XASSERT(geometry_workspace.matrix);
    matrix = geometry_workspace.matrix;
    /* Packed halfwords remain zero-extended in the original word stores */
    rotation->rotation_words[0] = (uint16)c;
    rotation->rotation_words[1] = (uint16)-s;
    rotation->rotation_words[2] = ONE;
    rotation->rotation_words[3] = (uint16)s;
    rotation->rotation_words[4] = (uint16)c;
    rotate_columns(matrix);
}

/* 800104B4 is also called directly by the model stream with a table offset. */
GDB_CALL void transform_roll_idx(uint32 idx)
{
    POSE_MATRIX *matrix;
    POSE_MATRIX *rotation = &geometry_workspace.scratch.matrix[GEOMETRY_MATRIX_COUNT - 1];
    sint32 s, c;
    /* Reject invalid packed Q12 offsets before accessing the table */
    XASSERT((idx & 3u) == 0 && idx <= 16380u);
    rsincos(idx >> 2, &s, &c);
    /* TODO: Investigate geometry entered before its current matrix is published */
    XASSERT(geometry_workspace.matrix);
    matrix = geometry_workspace.matrix;
    rotation->m[0][0] = (sint16)c;
    rotation->m[0][1] = (sint16)-s;
    rotation->m[0][2] = 0;
    rotation->m[1][0] = (sint16)s;
    /* Preserve the zero-extended cosine word, including its upper padding */
    rotation->rotation_words[2] = (uint16)c;
    rotation->rotation_words[3] = 0;
    rotation->rotation_words[4] = ONE;
    rotate_columns(matrix);
}

/* Actual 10F94 uses the low32 product before SRA14. */
GDB_CALL void transform_scale(sint32 x, sint32 y, sint32 z)
{
    FUNCTION_MARKER(0x80010f94, "GAME.EXE");
    /* TODO: Investigate geometry entered before its current matrix is published */
    XASSERT(geometry_workspace.matrix);
    POSE_MATRIX *matrix = geometry_workspace.matrix;
    uint32 row;
    for (row = 0; row < 3; row++)
    {
        sint16 col0 = matrix->m[row][0], col1 = matrix->m[row][1], col2 = matrix->m[row][2];
        matrix->m[row][0] = (sint16)((sint32)(x * (uint32)(sint32)col0) >> 14);
        matrix->m[row][1] = (sint16)((sint32)(y * (uint32)(sint32)col1) >> 14);
        matrix->m[row][2] = (sint16)((sint32)(z * (uint32)(sint32)col2) >> 14);
    }
}

/* 80011C60: Reset matrix stack to SCRATCHPAD, then pitch followed by yaw
 * The original uses unsigned halfword angle loads. Return is not consumed. */
GDB_CALL void transform_reset(void)
{
    FUNCTION_MARKER(0x80011c60, "GAME.EXE");
    POSE_MATRIX *matrix = &geometry_workspace.scratch.matrix[GEOMETRY_MATRIX_COUNT - 2];
    int i;
    geometry_workspace.matrix = matrix;
    geometry_workspace.matrix_storage = GEOMETRY_MATRIX_SCRATCH;
    for (i = 0; i < 5; i++)
        matrix->rotation_words[i] = (i == 0 || i == 2 || i == 4) ? ONE : 0;
    for (i = 0; i < 3; i++)
        matrix->t[i] = 0;
    transform_pitch((uint16)camera.angles.pitch);
    transform_yaw((uint16)camera.angles.yaw);
}

/* 80011D20: eight word stores include matrix padding and translations. */
GDB_CALL sint32 transform_identity(void)
{
    FUNCTION_MARKER(0x80011d20, "GAME.EXE");
    /* TODO: Investigate geometry entered before its current matrix is published */
    XASSERT(geometry_workspace.matrix);
    POSE_MATRIX *matrix = geometry_workspace.matrix;
    int i;
    for (i = 0; i < 5; i++)
        matrix->rotation_words[i] = (i == 0 || i == 2 || i == 4) ? ONE : 0;
    for (i = 0; i < 3; i++)
        matrix->t[i] = 0;
    return ONE;
}

/* Test the transformed origin against the original near and horizontal gates */
GDB_CALL sint32 transform_origin_visible(void)
{
    FUNCTION_MARKER(0x80011e5c, "GAME.EXE");
    /* TODO: Investigate geometry entered before its current matrix is published */
    XASSERT(geometry_workspace.matrix);
    const POSE_MATRIX *matrix = geometry_workspace.matrix;
    uint32 words[8];
    sint32 x, z;
    int i;
    for (i = 0; i < 5; i++)
        words[i] = matrix->rotation_words[i];
    for (i = 5; i < 8; i++)
        words[i] = matrix->t[i - 5];
    load_gte(words);
    xport_gte_write_data(0, 0);
    xport_gte_write_data(1, 0);
    xport_gte_execute(0x480012);
    x = xport_gte_read_data(25);
    xport_gte_read_data(26);
    z = xport_gte_read_data(27);
    /* Zero-vector MACs equal signed 16-bit translations, so ADD/SUB cannot overflow */
    if (z - 80 < 0)
        return 0;
    if (x <= 0)
        x = -x;
    x += (uint32)z < 800 ? z >> 2 : z >> 3;
    return z - x >= 0;
}

/* 80011F74: load all eight words before stores, then reload COP2 controls. */
GDB_CALL void transform_load(const POSE_MATRIX *src)
{
    FUNCTION_MARKER(0x80011f74, "GAME.EXE");
    /* TODO: Investigate geometry entered before its current matrix is published */
    XASSERT(geometry_workspace.matrix);
    POSE_MATRIX *dst = geometry_workspace.matrix;
    uint32 words[8];
    int i;
    for (i = 0; i < 8; i++)
        memcpy((uint8 *)words + 4 * i, (const uint8 *)src + 4 * i, sizeof(uint32));
    for (i = 0; i < 5; i++)
        dst->rotation_words[i] = words[i];
    for (i = 5; i < 8; i++)
        dst->t[i - 5] = words[i];
    load_gte(words);
}

GDB_CALL sint16 transform_origin_depth(void)
{
    FUNCTION_MARKER(0x80012190u, "GAME.EXE");
    /* TODO: Investigate geometry entered before its current matrix is published */
    XASSERT(geometry_workspace.matrix);
    const POSE_MATRIX *matrix = geometry_workspace.matrix;
    return matrix->t[2] >> 16;
}

/* Reload the current transform, then convert Q16 coordinates through RotTrans */
GDB_CALL uint32 transform_point(sint32 x, sint32 y, sint32 z, uint32 output[3])
{
    FUNCTION_MARKER(0x800614f4u, "GAME.EXE");
    SVECTOR v;
    VECTOR q;
    sint32 flags;
    transform_reload();
    v.vx = (sint16)(x >> 16);
    v.vy = (sint16)(y >> 16);
    v.vz = (sint16)(z >> 16);
    v.pad = 0;
    RotTrans(&v, &q, &flags);
    output[0] = (uint32)q.vx << 16;
    output[1] = (uint32)q.vy << 16;
    output[2] = (uint32)q.vz << 16;
    return output[2];
}
