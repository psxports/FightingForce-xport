#include <stdio.h>

#include "game.h"

/* Signed projected Y retains full word bits for wrapped background arithmetic */
uint32 proj_horizon;

/* RTPT retains V1/V2 and exposes V0 through SXY0/SZ1 */
GDB_CALL sint32 proj_local(sint32 x, sint32 y, sint32 z, PROJECTION_POINT *dest)
{
    FUNCTION_MARKER(0x800120bc, "GAME.EXE");
    /* TODO: Investigate geometry entered before its current matrix is published */
    if (!geometry_workspace.matrix)
    {
        DIAGNOSTIC(fprintf(stderr, "Missing current geometry matrix in %s stage %u tick %u\n", __FUNCTION__, game_session.stage, game_session.tick));
        diagnostic_stop(__FUNCTION__, __FILE__, __LINE__);
    }
    const POSE_MATRIX *matrix = geometry_workspace.matrix;
    uint32 words[8], i, packed, depth;
    sint32 sx, sy, result;
    for (i = 0; i < 8; ++i)
        words[i] = i < 5 ? matrix->rotation_words[i] : (uint32)matrix->t[i - 5];
    for (i = 0; i < 5; ++i)
        xport_gte_write_control(i, words[i]);
    for (i = 5; i < 8; ++i)
        xport_gte_write_control(i, (sint32)words[i] >> 16);
    xport_gte_write_data(0, ((uint32)y << 16) | ((uint32)x & 65535));
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
GDB_CALL void proj_update_horizon(void)
{
    PROJECTION_POINT pos;
    transform_identity();
    transform_pitch(camera.angles.pitch);
    proj_local(0, 0, 32767, &pos);
    proj_horizon = (uint32)pos.y;
}
