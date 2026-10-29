#include "game.h"

static uint32 checked_word(sint64 val)
{
    XASSERT(val >= -2147483647LL - 1 && val <= 2147483647LL);
    return val;
}

/* MIPS DIV retains hardware results without compiler BREAK guards */
static sint32 divide(sint32 dividend, sint32 divisor)
{
    if (!divisor)
        return dividend < 0 ? 1 : -1;
    if (dividend == (sint32)0x80000000u && divisor == -1)
        return dividend;
    return dividend / divisor;
}

/* Preserve deferred tag writes and packed command bits; early initialization changes aliases */
uint32 spr_draw(const PROJECTION_POINT *pos, sint32 spr_idx)
{
    FUNCTION_MARKER(0x80012390, "GAME.EXE");
    uint32 packet = render_buffers.packet_addr;
    const TEXTURE_DESCRIPTOR *desc;
    sint32 x = (sint16)pos->x, y = (sint16)pos->y;
    uint32 uv_clut;
    uint32 mode;
    uint32 end;
    uint32 semitrans;
    uint32 uv;
    uint32 delta;
    sint32 ot;
    uint32 bucket;
    uint32 next;
    const sint16 *depth_offsets;
    sint32 depth;
    XASSERT(spr_idx >= 0 && spr_idx < TEXTURE_DESCRIPTOR_CAPACITY);
    desc = tex_desc.item + spr_idx;
    uv_clut = desc->words[0];
    mode = desc->words[1];
    end = desc->words[2];
    semitrans = uv_clut & 1;
    uv = uv_clut - semitrans;
    delta = checked_word((sint64)(end >> 16) - (sint32)uv) & 65535;
    SPRT *spr = psx_addr(packet, sizeof(*spr));
    spr->p[0].uv_attr = uv;
    spr->rgbc.rgbc = (semitrans << 25) | 0x65000000;
    spr->p[0].xy.x = x;
    spr->p[0].xy.y = y;
    spr->w = (delta & 255) + 1;
    spr->h = (delta >> 8) + 1;
    ot = render_buffers.ot_addr;
    depth = pos->depth;
    depth_offsets = depth_table;
    /* Clamp the lookup index only; original out-of-range reachability is unproven */
    sint32 depth_idx = depth >> 2;
    if (depth_idx < 0)
        depth_idx = 0;
    else if (depth_idx >= DEPTH_TABLE_COUNT)
        depth_idx = DEPTH_TABLE_COUNT - 1;
    bucket = checked_word((sint64)ot + depth_offsets[depth_idx]);
    uint32 *ot_entry = psx_addr(bucket, sizeof(*ot_entry));
    spr->tag = *ot_entry | 0x04000000;
    next = checked_word((sint64)(sint32)packet + 20);
    *ot_entry = packet & 0xffffff;
    packet = next;
    DR_MODE *draw_mode = psx_addr(packet, sizeof(*draw_mode));
    draw_mode->code[0] = (mode >> 16) + 0xe1000000 + 1024;
    draw_mode->code[1] = 0;
    draw_mode->tag = *ot_entry | 0x02000000;
    next = checked_word((sint64)(sint32)packet + 12);
    *ot_entry = packet & 0xffffff;
    render_buffers.packet_addr = next;
    return 0x8008d4b0;
}

uint32 spr_draw_quad(const PROJECTION_POINT *pos, uint32 tex)
{
    FUNCTION_MARKER(0x800124a0u, "GAME.EXE");
    uint32 packet = render_buffers.packet_addr;
    sint32 x = (sint16)pos->x, y = (sint16)pos->y;
    uint32 uv_clut, uv_tpage, uv_end, semitrans, uv, extent, right, bottom, ot, link, next;
    const sint16 *depth_offsets;
    sint64 difference;
    XASSERT(tex < TEXTURE_DESCRIPTOR_CAPACITY);
    const TEXTURE_DESCRIPTOR *desc = tex_desc.item + tex;
    uv_clut = desc->words[0];
    uv_tpage = desc->words[1];
    uv_end = desc->words[2];
    semitrans = uv_clut & 1;
    uv = uv_clut - semitrans;
    difference = (sint64)(uv_end >> 16) - (sint32)uv;
    XASSERT(difference >= -2147483647LL - 1 && difference <= 2147483647LL);
    extent = (uint32)difference & 65535;
    right = (extent & 255) + x;
    bottom = (extent >> 8) + y;
    POLY_FT4 *poly = psx_addr(packet, sizeof(*poly));
    poly->p[0].uv_attr = uv;
    poly->p[1].uv_attr = uv_tpage;
    poly->p[2].uv.uv = uv_end;
    poly->p[3].uv.uv = uv_end >> 16;
    poly->rgbc.rgbc = 0x2d000000u | (semitrans << 25);
    poly->p[0].xy.x = x;
    poly->p[2].xy.x = x;
    poly->p[0].xy.y = y;
    poly->p[1].xy.y = y;
    poly->p[1].xy.x = right;
    poly->p[3].xy.x = right;
    poly->p[2].xy.y = bottom;
    poly->p[3].xy.y = bottom;
    ot = render_buffers.ot_addr;
    depth_offsets = depth_table;
    /* Clamp the lookup index only; original out-of-range reachability is unproven */
    sint32 depth_idx = pos->depth >> 2;
    if (depth_idx < 0)
        depth_idx = 0;
    else if (depth_idx >= DEPTH_TABLE_COUNT)
        depth_idx = DEPTH_TABLE_COUNT - 1;
    link = math_add_checked(ot, depth_offsets[depth_idx]);
    uint32 *ot_entry = psx_addr(link, sizeof(*ot_entry));
    poly->tag = *ot_entry | 0x09000000u;
    next = math_add_checked(packet, 40);
    *ot_entry = packet & 0xffffffu;
    render_buffers.packet_addr = next;
    return 0x8008d4b0;
}

uint32 spr_draw_scaled(const PROJECTION_POINT *pos, sint32 spr)
{
    FUNCTION_MARKER(0x80012594, "GAME.EXE");
    uint32 packet = render_buffers.packet_addr;
    uint32 next_packet;
    uint32 uv;
    uint32 mode;
    uint32 end;
    uint32 color;
    uint32 semitrans;
    uint32 delta;
    uint32 width;
    uint32 height;
    uint32 bucket;
    sint32 ot;
    const sint16 *depth_offsets;
    sint32 x = (sint16)pos->x, y = (sint16)pos->y, dx, dy, depth;
    uint32 size, right, bottom, left, top;
    XASSERT(spr >= 0 && spr < TEXTURE_DESCRIPTOR_CAPACITY);
    const TEXTURE_DESCRIPTOR *desc = tex_desc.item + spr;
    uv = desc->words[0];
    mode = desc->words[1];
    end = desc->words[2];
    color = render_style.color.rgbc;
    semitrans = uv & 1;
    uv -= semitrans;
    delta = (checked_word((sint64)(end >> 16) - (sint32)uv) & 65535) + 257;
    width = delta & 255;
    height = delta >> 8;
    POLY_FT4 *poly = psx_addr(packet, sizeof(*poly));
    poly->p[0].uv_attr = uv;
    poly->p[1].uv_attr = mode;
    poly->p[2].uv.uv = end;
    poly->p[3].uv.uv = end >> 16;
    poly->rgbc.rgbc = (semitrans << 25) | color;
    depth = pos->depth;
    size = pos->size;
    dx = divide(width * size, depth);
    dy = width == height ? dx : divide(height * size, depth);
    right = checked_word((sint64)x + dx);
    bottom = checked_word((sint64)y + dy);
    left = checked_word((sint64)x - dx);
    top = checked_word((sint64)y - dy);
    right = checked_word((sint64)(sint32)right - 1);
    bottom = checked_word((sint64)(sint32)bottom - 1);
    poly->p[0].xy.x = left;
    poly->p[2].xy.x = left;
    poly->p[0].xy.y = top;
    poly->p[1].xy.y = top;
    poly->p[1].xy.x = right;
    poly->p[3].xy.x = right;
    poly->p[2].xy.y = bottom;
    poly->p[3].xy.y = bottom;
    ot = render_buffers.ot_addr;
    depth_offsets = depth_table;
    /* Clamp the lookup index only; original out-of-range reachability is unproven */
    sint32 depth_idx = depth >> 2;
    if (depth_idx < 0)
        depth_idx = 0;
    else if (depth_idx >= DEPTH_TABLE_COUNT)
        depth_idx = DEPTH_TABLE_COUNT - 1;
    bucket = checked_word((sint64)ot + depth_offsets[depth_idx]);
    uint32 *ot_entry = psx_addr(bucket, sizeof(*ot_entry));
    poly->tag = *ot_entry | 0x09000000u;
    next_packet = checked_word((sint64)(sint32)packet + 40);
    *ot_entry = packet & 0xffffff;
    render_style.color.rgbc = 0x2d000000;
    render_buffers.packet_addr = next_packet;
    return 0x8008d4b0;
}

void spr_set_gradient_quad(uint32 packet, sint32 spr, sint32 x, sint32 y)
{
    FUNCTION_MARKER(0x80012a00u, "GAME.EXE");
    XASSERT(spr >= 0 && spr < TEXTURE_DESCRIPTOR_CAPACITY);
    const TEXTURE_DESCRIPTOR *desc = tex_desc.item + spr;
    uint32 uv = desc->words[0], mode = desc->words[1], end = desc->words[2];
    uint32 delta = ((end >> 16) - uv) & 65535u;
    uint16 right = (uint16)(x + (delta & 255)), bottom = y + (delta >> 8);
    POLY_GT4 *poly = psx_addr(packet, sizeof(*poly));
    poly->p[0].uv_attr = uv;
    poly->p[1].uv_attr = mode;
    poly->p[2].uv.uv = (uint16)end;
    poly->p[3].uv.uv = (uint16)(end >> 16);
    poly->p[0].xy.x = (uint16)x;
    poly->p[0].xy.y = (uint16)y;
    poly->p[1].xy.x = right;
    poly->p[1].xy.y = (uint16)y;
    poly->p[2].xy.x = (uint16)x;
    poly->p[2].xy.y = bottom;
    poly->p[3].xy.x = right;
    poly->p[3].xy.y = bottom;
}

/* Rejected ribbons still write color/UV and reset the shared color */
uint32 spr_draw_ribbon(const PROJECTION_POINT *a, const PROJECTION_POINT *b, sint32 spr)
{
    FUNCTION_MARKER(0x8006a1b4u, "GAME.EXE");
    uint32 packet = render_buffers.packet_addr, depth = (sint32)(a->depth + (uint32)b->depth) >> 3, color, tex_flags;
    SVECTOR dir;
    VECTOR delta;
    sint32 numerator;
    uint32 width;
    uint32 dx;
    uint32 dy;
    uint32 ot;
    sint32 uv_end;
    sint32 uv_start;
    uint32 pass;
    const PROJECTION_POINT *point;
    XASSERT(spr >= 0 && spr < TEXTURE_DESCRIPTOR_CAPACITY);
    const TEXTURE_DESCRIPTOR *tex = tex_desc.item + spr;
    tex_flags = tex->words[0];
    color = render_style.color.rgbc;
    if (tex_flags & 1u)
        color |= 0x02000000u;
    POLY_FT4 *poly = psx_addr(packet, sizeof(*poly));
    poly->rgbc.rgbc = color;
    poly->p[0].uv_attr = tex->words[0] & 0xfffffffeu;
    render_style.color.rgbc = 0x2d000000;
    if (depth >= 2048)
        return 0;
    delta.vx = b->x - (uint32)a->x;
    delta.vy = b->y - (uint32)a->y;
    delta.vz = 0;
    delta.pad = 0;
    /* Preserve the original image-dependent zero-vector coefficient */
    if (!VectorNormalS(&delta, &dir))
        xport_gte_write_data(8, 4112);
    render_buffers.packet_addr = render_buffers.packet_addr + 40;
    poly->p[1].uv_attr = tex->words[1];
    poly->p[2].uv.uv = tex->uv2.uv;
    poly->p[3].uv.uv = tex->uv3.uv;
    if (render_style.blend_mode)
    {
        uint32 packet_mode = poly->p[1].attr;
        uint32 blend_mode = (uint16)render_style.blend_mode;
        poly->p[1].attr = (packet_mode & 0xff9fu) | ((blend_mode - 1u) << 5);
        poly->rgbc.rgbc = poly->rgbc.rgbc | 0x02000000u;
    }
    for (pass = 0; pass < 2; pass++)
    {
        POINT_T *edge = poly->p + 2 * pass;
        point = pass ? b : a;
        uv_end = (sint8)(pass ? tex->uv3.u : tex->uv1.u);
        uv_start = (sint8)(pass ? tex->uv2.u : tex->uv0.u);
        width = (uv_end - uv_start) >> 1;
        numerator = width * point->size;
        XASSERT(point->depth && (point->depth != -1 || (uint32)numerator != 0x80000000u));
        width = numerator / point->depth;
        dx = (sint32)(width * (dir.vy)) >> 12;
        dy = (sint32)(width * (-dir.vx)) >> 12;
        edge[0].xy.x = point->x - dx;
        edge[0].xy.y = point->y - dy;
        edge[1].xy.x = point->x + dx;
        edge[1].xy.y = point->y + dy;
    }
    ot = depth_table[depth] >> 2;
    return render_ot_insert_flags(render_buffers.ot_addr + 4u * ot, packet, 0x09000000);
}

uint32 spr_draw_height(const PROJECTION_POINT *pos, sint32 spr)
{
    FUNCTION_MARKER(0x800128f4, "GAME.EXE");
    uint32 packet;
    uint32 uv;
    uint32 mode;
    uint32 end;
    uint32 color;
    uint32 semitrans;
    sint32 depth;
    sint32 height;
    uint32 bucket;
    uint32 ot;
    const sint16 *depth_offsets;
    uint32 next_packet;
    sint32 x, y;
    sint64 diff, bottom;
    uint32 right;
    packet = render_buffers.packet_addr;
    x = (sint16)pos->x;
    y = (sint16)pos->y;
    XASSERT(spr >= 0 && spr < TEXTURE_DESCRIPTOR_CAPACITY);
    const TEXTURE_DESCRIPTOR *desc = tex_desc.item + spr;
    uv = desc->words[0];
    mode = desc->words[1];
    end = desc->words[2];
    color = render_style.color.rgbc;
    semitrans = uv & 1;
    uv -= semitrans;
    diff = (sint64)(end >> 16) - (sint32)uv;
    XASSERT(diff >= -2147483647LL - 1 && diff <= 2147483647LL);
    POLY_FT4 *poly = psx_addr(packet, sizeof(*poly));
    poly->p[0].uv_attr = uv;
    poly->p[1].uv_attr = mode;
    poly->p[2].uv.uv = end;
    poly->p[3].uv.uv = end >> 16;
    poly->rgbc.rgbc = color | (semitrans << 25);
    depth = pos->depth;
    height = pos->size;
    poly->p[0].xy.x = x;
    poly->p[2].xy.x = x;
    poly->p[0].xy.y = y;
    poly->p[1].xy.y = y;
    right = x + ((uint32)diff & 255) + 1;
    bottom = (sint64)height + y;
    XASSERT(bottom >= -2147483647LL - 1 && bottom <= 2147483647LL);
    poly->p[1].xy.x = right;
    poly->p[3].xy.x = right;
    poly->p[2].xy.y = bottom;
    poly->p[3].xy.y = bottom;
    ot = render_buffers.ot_addr;
    depth_offsets = depth_table;
    /* Clamp the lookup index only; original out-of-range reachability is unproven */
    sint32 depth_idx = depth >> 2;
    if (depth_idx < 0)
        depth_idx = 0;
    else if (depth_idx >= DEPTH_TABLE_COUNT)
        depth_idx = DEPTH_TABLE_COUNT - 1;
    bucket = math_add_checked(ot, depth_offsets[depth_idx]);
    uint32 *ot_entry = psx_addr(bucket, sizeof(*ot_entry));
    poly->tag = *ot_entry | 0x09000000;
    next_packet = math_add_checked(packet, 40);
    *ot_entry = packet & 0xffffff;
    render_style.color.rgbc = 0x2d000000;
    render_buffers.packet_addr = next_packet;
    return 0x8008d4b0;
}
