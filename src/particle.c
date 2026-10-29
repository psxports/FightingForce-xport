#include "game.h"

typedef enum PARTICLE_SHAPE
{
    PARTICLE_SHAPE_POINT,
    PARTICLE_SHAPE_LINE,
    PARTICLE_SHAPE_QUAD,
    PARTICLE_SHAPE_COUNT
} PARTICLE_SHAPE;

/* Deferred tags preserve original bucket and packet alias behavior */
static uint32 draw_packet(sint32 x, sint32 y, sint32 z, uint32 color, PARTICLE_SHAPE shape)
{
    uint32 packet = render_buffers.packet_addr, xy = ((uint32)x & 65535) | ((uint32)y << 16);
    uint32 rgbc = (shape == PARTICLE_SHAPE_QUAD ? 0x28000000u : 0x40000000u) | ((color & 31) << 3) | ((color & 0x3e0) << 6) | ((color & 0x7c00) << 9);
    POLY_F4 *poly = NULL;
    LINE_F2 *line = NULL;
    uint32 ot, bucket;
    const sint16 *depth_offsets;
    sint32 offset;
    if (shape == PARTICLE_SHAPE_QUAD)
    {
        poly = psx_addr(packet, sizeof(*poly));
        poly->rgbc.rgbc = rgbc;
        poly->p[0].xy = xy;
        /* Packed additions retain carries between X and Y */
        xy = math_add_checked(xy, 2);
        poly->p[1].xy = xy;
        xy = math_add_checked(xy, 0x20000);
        poly->p[3].xy = xy;
        xy = math_add_checked(xy, -2);
        poly->p[2].xy = xy;
    }
    else
    {
        line = psx_addr(packet, sizeof(*line));
        line->rgbc.rgbc = rgbc;
        line->p[0].xy = xy;
        if (shape == PARTICLE_SHAPE_LINE)
            xy = math_add_checked(xy, 0x10000);
        line->p[1].xy = xy;
    }
    ot = render_buffers.ot_addr;
    depth_offsets = depth_table;
    /* Clamp the lookup index only; original out-of-range reachability is unproven */
    sint32 depth_idx = z >> 2;
    if (depth_idx < 0)
        depth_idx = 0;
    else if (depth_idx >= DEPTH_TABLE_COUNT)
        depth_idx = DEPTH_TABLE_COUNT - 1;
    offset = depth_offsets[depth_idx];
    bucket = math_add_checked(ot, offset);
    uint32 *ot_entry = psx_addr(bucket, sizeof(*ot_entry));
    if (shape == PARTICLE_SHAPE_QUAD)
        poly->tag = *ot_entry | 0x05000000u;
    else
        line->tag = *ot_entry | 0x03000000u;
    *ot_entry = packet;
    render_buffers.packet_addr = math_add_checked(packet, shape == PARTICLE_SHAPE_QUAD ? 24 : 16);
    return 0x8008d4b0;
}

uint32 particle_draw_point(sint32 x, sint32 y, sint32 z, uint32 color)
{
    FUNCTION_MARKER(0x800121a8, "GAME.EXE");
    return draw_packet(x, y, z, color, PARTICLE_SHAPE_POINT);
}

uint32 particle_draw_line(sint32 x, sint32 y, sint32 z, uint32 color)
{
    FUNCTION_MARKER(0x80012240, "GAME.EXE");
    return draw_packet(x, y, z, color, PARTICLE_SHAPE_LINE);
}

uint32 particle_draw_quad(sint32 x, sint32 y, sint32 z, uint32 color)
{
    FUNCTION_MARKER(0x800122e0, "GAME.EXE");
    return draw_packet(x, y, z, color, PARTICLE_SHAPE_QUAD);
}
