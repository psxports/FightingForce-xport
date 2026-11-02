#include <stdio.h>

#include "game.h"
#include "hd.h"

/* Signed projected Y retains full word bits for wrapped background arithmetic */
uint32 proj_horizon;

/* RTPT retains V1/V2 and exposes V0 through SXY0/SZ1 */
sint32 proj_local(sint32 x, sint32 y, sint32 z, PROJECTION_POINT *dest)
{
    FUNCTION_MARKER(0x800120bc, "GAME.EXE");
    /* TODO: Investigate geometry entered before its current matrix is published */
    XASSERT(geometry_workspace.matrix);
    const POSE_MATRIX *matrix = geometry_workspace.matrix;
    uint32 words[8], i, packed, depth;
    sint32 sx, sy, result;
    for (i = 0; i < 8; ++i)
        words[i] = i < 5 ? matrix->rotation_words[i] : matrix->t[i - 5];
    for (i = 0; i < 5; ++i)
        xport_gte_write_control(i, words[i]);
    for (i = 5; i < 8; ++i)
        xport_gte_write_control(i, (sint32)words[i] >> 16);
    xport_gte_write_data(0, ((uint32)y << 16) | (x & 65535u));
    xport_gte_write_data(1, z);
    xport_gte_execute(0x280030);
    depth = xport_gte_read_data(17) & 65535;
    packed = xport_gte_read_data(12);
    sx = (sint16)packed;
    sy = (sint16)(packed >> 16);
    result = depth < 40 ? 4 : ((uint16)sx < 320 && (uint16)sy < 256 ? 0 : 1);
    dest->x = sx;
    dest->y = sy;
    dest->depth = (sint16)depth;
    dest->status = result;
    return result;
}

/* Existing 58844..5887C phase of original58634, not a new original function */
void proj_update_horizon(void)
{
    PROJECTION_POINT pos;
    transform_identity();
    transform_pitch(camera.angles.pitch);
    hd_collect_horizon(geometry_workspace.matrix);
    proj_local(0, 0, 32767, &pos);
    proj_horizon = (uint32)pos.y;
}
