#include <stdio.h>

#include "game.h"

TERRAIN_MAP terrain_map;
BACKGROUND_IMAGE background_img;

/* Terrain projections retain the shared matrix and depth storage */
enum
{
    GRID_SIDE = 17,
    GRID_POINT_COUNT = GRID_SIDE * GRID_SIDE + 1
};

const TERRAIN_VIEW_OFFSET terrain_view_offset[TERRAIN_VIEW_DIR_COUNT] = {
    {0, -24}, {12, -24}, {24, -24}, {24, -12}, {24, 0}, {24, 12}, {24, 24}, {12, 24}, {0, 24}, {-12, 24}, {-24, 24}, {-24, 12}, {-24, 0}, {-24, -12}, {-24, -24}, {-12, -24},
};

/* Terrain grid uses six RTPT triples per row and a 17-point row stride
 * Packed X increments carry into Y at zero as in the original */
GDB_CALL uint32 terrain_draw_grid(sint32 unused, sint32 tile_x, sint32 tile_z)
{
    FUNCTION_MARKER(0x8001100C, "GAME.EXE");
    uint32 row, col, n, packed, packet, ot, head, color, words[8], depths[3];
    uint32 sx = (uint32)tile_x << 1, sz = (uint32)tile_z << 7;
    uint32 *xy;
    sint16 *proj_depth;
    const POSE_MATRIX *matrix;
    int i;
    /* TODO: Investigate geometry entered before its current matrix is published */
    if (!geometry_workspace.matrix)
    {
        DIAGNOSTIC(fprintf(stderr, "Missing current geometry matrix in %s stage %u tick %u\n", __FUNCTION__, game_session.stage, game_session.tick));
        diagnostic_stop(__FUNCTION__, __FILE__, __LINE__);
    }
    matrix = geometry_workspace.matrix;
    for (i = 0; i < 5; i++)
        words[i] = matrix->rotation_words[i];
    for (i = 0; i < 5; i++)
        xport_gte_write_control(i, words[i]);
    for (i = 5; i < 8; i++)
        words[i] = matrix->t[i - 5];
    for (i = 5; i < 8; i++)
        xport_gte_write_control(i, (sint32)words[i] >> 16);
    /* The eighteenth point overwrites the next row start; the final extra point stays live */
    xy = geometry_workspace.model.proj.xy;
    proj_depth = geometry_workspace.scratch.depth;
    for (row = 0; row < 17; row++)
    {
        uint32 z = (uint32)-2048 + row * 256;
        xport_gte_write_data(1, z);
        xport_gte_write_data(3, z);
        xport_gte_write_data(5, z);
        packed = 0xf800;
        for (col = 0; col < 18; col += 3)
        {
            for (i = 0; i < 3; i++)
            {
                xport_gte_write_data(2 * i, packed);
                packed += 256;
            }
            xport_gte_execute(0x280030);
            n = row * 17 + col;
            for (i = 0; i < 3; i++)
                xy[n + i] = xport_gte_read_data(12 + i);
            for (i = 0; i < 3; i++)
                depths[i] = xport_gte_read_data(17 + i);
            for (i = 0; i < 3; i++)
                proj_depth[n + i] = depths[i];
        }
    }
    color = render_style.color.rgbc;
    ot = render_buffers.ot_addr;
    packet = render_buffers.packet_addr;
    ot += 4;
    uint32 *ot_entry = psx_addr(ot, sizeof(*ot_entry));
    head = *ot_entry;
    for (row = 0; row < 16; row++)
    {
        for (col = 0; col < 16; col++)
        {
            sint32 tile;
            uint32 v0, v1, v2, v3, uv, descriptor0, descriptor1;
            sx &= 0x7e;
            sz &= 0x1f80;
            tile = (sint16)terrain_map.tile[sz >> 7][sx >> 1];
            n = row * 17 + col;
            if (tile != -4 && proj_depth[n] > 0 && proj_depth[n + 18] > 0)
            {
                v0 = xy[n];
                v1 = xy[n + 1];
                v2 = xy[n + 17];
                v3 = xy[n + 18];
                POLY_FT4 *poly = psx_addr(packet, sizeof(*poly));
                poly->p[0].xy.xy = v0; /* Branch delay slot also executes for horizontal rejection */
                if ((uint16)v0 < 320 || (uint16)v1 < 320 || (uint16)v2 < 320 || (uint16)v3 < 320)
                {
                    poly->p[1].xy.xy = v1;
                    poly->p[2].xy.xy = v2;
                    poly->p[3].xy.xy = v3;
                    if (tile < 0 || tile > TEXTURE_DESCRIPTOR_CAPACITY * 3 - 3)
                        diagnostic_stop(__FUNCTION__, __FILE__, __LINE__);
                    const uint32 *t = tex_desc.words + tile;
                    descriptor0 = t[0];
                    descriptor1 = t[1];
                    uv = t[2];
                    poly->p[0].uv_attr = descriptor0;
                    poly->p[1].uv_attr = descriptor1;
                    poly->p[2].uv.uv = uv;
                    poly->p[3].uv.uv = uv >> 16;
                    poly->rgbc.rgbc = color;
                    poly->tag = head | 0x09000000;
                    head = packet;
                    packet += 40;
                }
            }
            sx -= 2;
        }
        sx += 32;
        sz -= 128;
    }
    *ot_entry = head;
    render_style.color.rgbc = 0x2d000000;
    render_buffers.packet_addr = packet;
    return 0x8008d4b0;
}

GDB_CALL uint32 terrain_draw_faded_grid(uint32 unused, uint32 column, uint32 row)
{
    FUNCTION_MARKER(0x800112a4u, "GAME.EXE");
    uint32 r, n, packed, packet, ot, head, point_idx = 0, cell, sx[4], uv[3], color;
    uint32 words[8], proj_depths[3];
    sint32 z = -2048, cutoff, depth, delta, fade;
    sint64 difference;
    uint32 *xy, *xy_out;
    sint16 *proj_depth, *depth_out;
    const POSE_MATRIX *matrix;
    int i;
    column <<= 1;
    row <<= 7;
    /* TODO: Investigate geometry entered before its current matrix is published */
    if (!geometry_workspace.matrix)
    {
        DIAGNOSTIC(fprintf(stderr, "Missing current geometry matrix in %s stage %u tick %u\n", __FUNCTION__, game_session.stage, game_session.tick));
        diagnostic_stop(__FUNCTION__, __FILE__, __LINE__);
    }
    matrix = geometry_workspace.matrix;
    for (i = 0; i < 5; i++)
        words[i] = matrix->rotation_words[i];
    for (i = 0; i < 5; i++)
        xport_gte_write_control(i, words[i]);
    for (i = 5; i < 8; i++)
        words[i] = matrix->t[i - 5];
    for (i = 5; i < 8; i++)
        xport_gte_write_control(i, (sint32)words[i] >> 16);
    xy = geometry_workspace.model.proj.xy;
    proj_depth = geometry_workspace.scratch.depth;
    xy_out = xy;
    depth_out = proj_depth;
    /* Six RTPT triples per row with a seventeen-point stride */
    for (r = 0; r < 17; ++r, z += 256)
    {
        xport_gte_write_data(1, z);
        xport_gte_write_data(3, z);
        xport_gte_write_data(5, z);
        packed = 0xf800;
        for (n = 0; n < 6; n++)
        {
            for (i = 0; i < 3; i++)
            {
                xport_gte_write_data(2 * i, packed);
                packed += 256;
            }
            xport_gte_execute(0x280030);
            for (i = 0; i < 3; i++)
                xy_out[i] = xport_gte_read_data(12 + i);
            for (i = 0; i < 3; i++)
                proj_depths[i] = xport_gte_read_data(17 + i);
            for (i = 0; i < 3; i++)
                depth_out[i] = proj_depths[i];
            xy_out += 3;
            depth_out += 3;
        }
        xy_out--;
        depth_out--;
    }
    color = render_style.color.rgbc;
    ot = render_buffers.ot_addr + 8;
    packet = render_buffers.packet_addr;
    uint32 *ot_entry = psx_addr(ot, sizeof(*ot_entry));
    head = *ot_entry;
    cutoff = RENDER_FADE_CUTOFF;
    for (r = 0; r < 16; ++r)
    {
        for (n = 0; n < 16; ++n)
        {
            column &= 0x7e;
            row &= 0x1f80;
            cell = terrain_map.tile[row >> 7][column >> 1];
            if (cell != 0xfffc && proj_depth[point_idx] > 0)
            {
                depth = proj_depth[point_idx + 18];
                if (depth > 0)
                {
                    sx[0] = xy[point_idx];
                    sx[1] = xy[point_idx + 1];
                    color = 0x2d000000;
                    difference = (sint64)depth - cutoff;
                    if (difference < -2147483647 - 1 || difference > 2147483647)
                        diagnostic_stop(__FUNCTION__, __FILE__, __LINE__);
                    delta = difference;
                    /* The ADDI in the rejection branch delay slot still executes */
                    difference += 2048;
                    if (difference > 2147483647)
                        diagnostic_stop(__FUNCTION__, __FILE__, __LINE__);
                    fade = difference;
                    if (delta > 0)
                        goto next_faded_tile;
                    if (fade >= 0)
                    {
                        uint32 shade = 128 - (uint32)(fade >> 4);
                        color = 0x2c000000 | shade | (shade << 8) | (shade << 16);
                    }
                    sx[2] = xy[point_idx + 17];
                    sx[3] = xy[point_idx + 18];
                    /* Horizontal rejection still executes this delay-slot store */
                    POLY_FT4 *poly = psx_addr(packet, sizeof(*poly));
                    poly->p[0].xy.xy = sx[0];
                    if ((sx[0] & 65535) < 320 || (sx[1] & 65535) < 320 || (sx[2] & 65535) < 320 || (sx[3] & 65535) < 320)
                    {
                        poly->p[1].xy.xy = sx[1];
                        poly->p[2].xy.xy = sx[2];
                        poly->p[3].xy.xy = sx[3];
                        if (cell > TEXTURE_DESCRIPTOR_CAPACITY * 3 - 3)
                            diagnostic_stop(__FUNCTION__, __FILE__, __LINE__);
                        const uint32 *tex = tex_desc.words + cell;
                        uv[0] = tex[0];
                        uv[1] = tex[1];
                        uv[2] = tex[2];
                        poly->p[0].uv_attr = uv[0];
                        poly->p[1].uv_attr = uv[1];
                        poly->p[2].uv.uv = uv[2];
                        poly->p[3].uv.uv = uv[2] >> 16;
                        poly->rgbc.rgbc = color;
                        poly->tag = head | 0x09000000;
                        head = packet;
                        packet += 40;
                    }
                }
            }
        next_faded_tile:
            column -= 2;
            point_idx++;
        }
        column += 32;
        row -= 128;
        point_idx++;
    }
    *ot_entry = head;
    render_style.color.rgbc = 0x2d000000;
    render_buffers.packet_addr = packet;
    return 0x8008d4b0;
}

/* 80064150: update ten alternating 320-byte scanlines. MULT uses low 32 bits
 * before SAR, and division by 160 applies after wrapped SLL 14. */
static sint32 background_product(sint32 a, sint32 b)
{
    return (sint32)((uint32)a * (uint32)b) >> 8;
}

GDB_CALL sint32 background_render_strip(void)
{
    FUNCTION_MARKER(0x80064150, "GAME.EXE");
    uint32 parity, out_offset = 0;
    sint32 base_x = ~(sint32)camera.pos.x.whole;
    sint32 height = camera.pos.y.whole;
    sint32 base_z = ~(sint32)camera.pos.z.whole;
    sint32 angle = (sint16)camera.angles.yaw_bits >> 5;
    sint32 sn = sin_table[(uint32)angle & 2047];
    sint32 cs = sin_table[(uint32)(angle + 512) & 2047];
    uint32 sn160 = (uint32)sn * 160, cs160 = (uint32)cs * 160, sn256 = (uint32)sn << 8, cs256 = (uint32)cs << 8;
    sint32 ax = (sint32)(0u - cs160 - sn256) >> 15, az = (sint32)(cs256 - sn160) >> 15;
    sint32 bx = (sint32)(cs160 - sn256) >> 15, bz = (sint32)(sn160 + cs256) >> 15;
    sint32 h = (sint32)((uint32)height << 8), row, col;
    parity = disp_buffers.frame & 1;
    out_offset += 320 * parity;
    for (row = 1; row < 20; row += 2)
    {
        sint32 q, x, z;
        parity = disp_buffers.frame & 1;
        q = h / (row + (sint32)parity);
        x = background_product(q, ax);
        z = background_product(q, az);
        uint32 dx = (uint32)((sint32)(((uint32)background_product(q, bx) - (uint32)x) << 14) / 160) << 1;
        uint32 dz = (uint32)((sint32)(((uint32)background_product(q, bz) - (uint32)z) << 14) / 160) << 1;
        uint32 u = ((uint32)x + (uint32)base_x) << 16, v = ((uint32)z + (uint32)base_z) << 16;
        for (col = 0; col < 320; col++)
        {
            uint32 idx = ((uint32)((sint32)u >> 23) & 127) + ((uint32)((sint32)v >> 16) & 0x3f80);
            background_img.pix[out_offset++] = terrain_map.color_flat[idx];
            u += dx;
            v += dz;
        }
        out_offset += 320;
    }
    return 0;
}

/* 80063EEC: four terrain grids and optional distant texture/quads. */
GDB_CALL sint32 terrain_draw(void)
{
    FUNCTION_MARKER(0x80063EEC, "GAME.EXE");
    PROJECTION_POINT pos;
    uint32 cx, cz, x, z, tx, tz, dir;
    int i;
    cx = camera.pos.x.bits;
    x = cx & 0xffffff;
    cz = camera.pos.z.bits;
    z = cz & 0xffffff;
    dir = (uint32)((camera.angles.yaw + 2048) >> 12) & 15;
    tx = (sint32)~cx >> 24;
    tz = (sint32)~cz >> 24;
    if (camera.angles.pitch < 8192)
    {
        uint32 dx, dz;
        dx = terrain_view_offset[dir].x;
        x += dx << 23;
        dz = terrain_view_offset[dir].z;
        z += dz << 23;
        tx += (uint32)((sint32)dx >> 1);
        tz += (uint32)((sint32)dz >> 1);
    }
    transform_reset();
    for (i = 0; i < 4; i++)
    {
        transform_push();
        transform_translate((i & 1 ? 0xf8010000u : 0x07ff0000u) - x, 0u - camera.pos.y.bits, (i & 2 ? 0xf8010000u : 0x07ff0000u) - z);
        terrain_draw_grid(0, tx + (i & 1 ? 16 : 0), tz + (i & 2 ? 16 : 0));
        transform_pop();
    }
    if (game_session.stage == 0xffffffff)
        return -1;
    background_render_strip();
    pos.x = 0;
    pos.y = proj_horizon;
    pos.depth = 32767;
    pos.size = 21;
    spr_draw_height(&pos, tex_grps[TEXTURE_GROUP_TERRAIN].first_tex + 256);
    pos.x += 256;
    return (sint32)spr_draw_height(&pos, tex_grps[TEXTURE_GROUP_TERRAIN].first_tex + 257);
}

/* 80063900..80063AD0: Load terrain images before map/object initialization */
RESOURCE_VIEW terrain_load_texs(const char *name)
{
    RESOURCE_VIEW pbl = resource_find(name);
    uint32 count, x, y, bytes, available, scratch_offset, scratch_bytes, advance_bytes;
    uint8 *dest;
    PSX_RECT *rect;
    const uint8 *pixels;
    if (pbl.byte_len < sizeof(tex_pals.color[1]))
        diagnostic_stop(__FUNCTION__, __FILE__, __LINE__);
    pixels = pbl.data + 512;
    resource_copy_block(pbl.data, (uint8 *)tex_pals.color[1]);
    tex_pals.color[1][0] = 0;
    rect = &tex_upload_rects.terrain[TEXTURE_TERRAIN_BASE];
    if (rect->w < 0 || rect->h < 0)
        diagnostic_stop(__FUNCTION__, __FILE__, __LINE__);
    available = pbl.storage_len - (uint32)(pixels - pbl.storage);
    bytes = (uint32)rect->w * (uint32)rect->h * sizeof(uint16);
    if (bytes > available || ((uintptr_t)pixels & 3))
        diagnostic_stop(__FUNCTION__, __FILE__, __LINE__);
    if (LoadImagePSX(rect, (uint32 *)pixels) != 0)
        diagnostic_stop(__FUNCTION__, __FILE__, __LINE__);
    count = resource_swap32(resource_archive.dir->count_be);
    advance_bytes = 20 * count + 4;
    /* TODO: Investigate allocation beyond the actual native resource owner */
    if (!resource_arenas.scratch.data || (advance_bytes) > resource_arenas.scratch.available)
    {
        DIAGNOSTIC(fprintf(stderr, "Resource cursor scratch span %u available %u context %08X stage %u tick %u\n", (uint32)(advance_bytes), resource_arenas.scratch.available, resource_arenas.scratch.context, game_session.stage, game_session.tick));
        diagnostic_stop(__FUNCTION__, __FILE__, __LINE__);
    }
    resource_arenas.scratch.data += advance_bytes;
    resource_arenas.scratch.available -= advance_bytes;
    resource_arenas.scratch.context = resource_arenas.scratch.context + advance_bytes;
    /* Decode the staging file offset without aliasing separate native allocations */
    scratch_offset = resource_arenas.scratch.context - resource_archive.relocation_base;
    if (scratch_offset > resource_archive.byte_len || 128 * 128 > resource_archive.byte_len - scratch_offset)
    {
        /* TODO: Investigate a staging destination outside its actual archive owner */
        DIAGNOSTIC(fprintf(stderr, "Terrain half staging outside archive: cursor %08X context %08X offset %u bytes %u stage %u\n", resource_arenas.scratch.context, resource_archive.relocation_base, scratch_offset, resource_archive.byte_len, game_session.stage));
        diagnostic_stop(__FUNCTION__, __FILE__, __LINE__);
    }
    scratch_bytes = resource_archive.byte_len - scratch_offset;
    dest = (uint8 *)resource_archive.dir + scratch_offset;
    /* Original sampling continues across entries within the same WAD */
    if (available < 127 * 512 + 127 * 2 + 1)
        diagnostic_stop(__FUNCTION__, __FILE__, __LINE__);
    for (y = 0; y < 128; y++)
        for (x = 0; x < 128; x++)
            dest[y * 128 + x] = pixels[y * 512 + x * 2];
    rect = &tex_upload_rects.terrain[TEXTURE_TERRAIN_HALF];
    if (rect->w < 0 || rect->h < 0)
        diagnostic_stop(__FUNCTION__, __FILE__, __LINE__);
    bytes = (uint32)rect->w * (uint32)rect->h * sizeof(uint16);
    if (bytes > scratch_bytes || ((uintptr_t)dest & 3))
        diagnostic_stop(__FUNCTION__, __FILE__, __LINE__);
    if (LoadImagePSX(rect, (uint32 *)dest) != 0)
        diagnostic_stop(__FUNCTION__, __FILE__, __LINE__);
    /* Reload the cursor after the upload, as in the original */
    scratch_offset = resource_arenas.scratch.context - resource_archive.relocation_base;
    if (scratch_offset > resource_archive.byte_len || 64 * 64 > resource_archive.byte_len - scratch_offset)
    {
        /* TODO: Investigate a staging destination outside its actual archive owner */
        DIAGNOSTIC(fprintf(stderr, "Terrain quarter staging outside archive: cursor %08X context %08X offset %u bytes %u stage %u\n", resource_arenas.scratch.context, resource_archive.relocation_base, scratch_offset, resource_archive.byte_len, game_session.stage));
        diagnostic_stop(__FUNCTION__, __FILE__, __LINE__);
    }
    scratch_bytes = resource_archive.byte_len - scratch_offset;
    dest = (uint8 *)resource_archive.dir + scratch_offset;
    for (y = 0; y < 64; y++)
        for (x = 0; x < 64; x++)
            dest[y * 64 + x] = pixels[y * 1024 + x * 4];
    rect = &tex_upload_rects.terrain[TEXTURE_TERRAIN_QUARTER];
    if (rect->w < 0 || rect->h < 0)
        diagnostic_stop(__FUNCTION__, __FILE__, __LINE__);
    bytes = (uint32)rect->w * (uint32)rect->h * sizeof(uint16);
    if (bytes > scratch_bytes || ((uintptr_t)dest & 3))
        diagnostic_stop(__FUNCTION__, __FILE__, __LINE__);
    if (LoadImagePSX(rect, (uint32 *)dest) != 0)
        diagnostic_stop(__FUNCTION__, __FILE__, __LINE__);
    pbl.data += 512;
    pbl.context_address += 512;
    pbl.byte_len -= 512;
    return pbl;
}

/* 80063AE8..80063C38, after object initialization has returned. */
uint32 terrain_build_map(RESOURCE_VIEW map, RESOURCE_VIEW pixels)
{
    uint32 i, x, y;
    uint32 available = pixels.storage_len - (uint32)(pixels.data - pixels.storage);
    if (map.byte_len < sizeof(terrain_map.tile_flat))
        diagnostic_stop(__FUNCTION__, __FILE__, __LINE__);
    /* Preserve each original group of four reads followed by four writes */
    for (i = 0; i < 8192; i += 512)
        resource_copy_block(map.data + i, (uint8 *)terrain_map.tile_flat + i);
    for (y = 0; y < 64; y++)
        for (x = 0; x < 64; x++)
        {
            sint32 tile = (sint16)terrain_map.tile[y][x] >> 2;
            uint8 *dest = terrain_map.color_flat + 2 * y * TERRAIN_COLOR_SIDE + 2 * x;
            if (tile < 0)
            {
                dest[129] = 252;
                dest[128] = 252;
                dest[1] = 252;
                dest[0] = 252;
            }
            else
            {
                uint32 offset = 32u * ((uint32)tile & 7) + 8192u * ((uint32)tile >> 3);
                const uint8 *src;
                if (offset > available || 5141 > available - offset)
                    diagnostic_stop(__FUNCTION__, __FILE__, __LINE__);
                src = pixels.data + offset;
                dest[0] = src[2570];
                dest[1] = src[2580];
                dest[128] = src[5130];
                dest[129] = src[5140];
            }
        }
    return 0;
}

/* 8006109C..800610E8: Relocate MPD entries once after descriptor setup */
uint32 terrain_relocate_tiles(void)
{
    uint32 i;
    for (i = 0; i < 4096; i++)
    {
        uint32 val = terrain_map.tile_flat[i];
        if (val != 65532u)
            val = 3u * (val + (uint32)tex_grps[TEXTURE_GROUP_TERRAIN].first_tex);
        terrain_map.tile_flat[i] = val;
    }
    return 0;
}

uint32 terrain_load(const char *tex_name, const char *map_name)
{
    FUNCTION_MARKER(0x80063900, "GAME.EXE");
    RESOURCE_VIEW pixels = terrain_load_texs(tex_name);
    RESOURCE_VIEW map = resource_find(map_name);
    world_objects_load_map(map);
    return terrain_build_map(map, pixels);
}

static void terrain_desc(TEXTURE_DESCRIPTOR *desc, uint32 orientation)
{
    uint32 val;
    desc->clut = 31808;
    desc->tpage = getTPage(1, 0, 768, 0);
    val = (uint8)tex_allocator.x + ((orientation & 1) ? 31 : 0);
    desc->uv2.u = val;
    desc->uv0.u = val;
    val = (uint8)tex_allocator.y + (orientation < 2 ? 31 : 0);
    desc->uv1.v = val;
    desc->uv0.v = val;
    val = (uint8)tex_allocator.x + ((orientation & 1) ? 0 : 31);
    desc->uv3.u = val;
    desc->uv1.u = val;
    val = (uint8)tex_allocator.y + (orientation < 2 ? 0 : 31);
    desc->uv3.v = val;
    desc->uv2.v = val;
}

static void terrain_background_desc(TEXTURE_DESCRIPTOR *desc, uint32 page_x, uint32 right)
{
    desc->clut = 31808;
    desc->tpage = getTPage(1, 0, page_x, 256);
    desc->uv2.u = 0;
    desc->uv0.u = 0;
    desc->uv1.v = 224;
    desc->uv0.v = 224;
    desc->uv3.u = right;
    desc->uv1.u = right;
    desc->uv3.v = 239;
    desc->uv2.v = 239;
}

/* 80060DAC..80061098: Build 256 tile descriptors and two uncounted backgrounds */
void terrain_build_descs(void)
{
    uint32 base = (uint16)tex_desc_count, i, orientation;
    TEXTURE_DESCRIPTOR *desc;
    tex_grps[TEXTURE_GROUP_TERRAIN].first_tex = base;
    if (tex_desc_count > TEXTURE_DESCRIPTOR_CAPACITY - 258)
        diagnostic_stop(__FUNCTION__, __FILE__, __LINE__);
    desc = tex_desc.item + tex_desc_count;
    for (i = 0; i < 64; i++)
    {
        uint8 u = (uint8)(32 * (i & 7)), v = (uint8)(32 * (i >> 3));
        tex_allocator.x = u;
        tex_allocator.y = v;
        for (orientation = 0; orientation < 4; ++orientation)
        {
            terrain_desc(desc, orientation);
            desc++;
        }
        tex_desc_count = tex_desc_count + 4;
    }
    terrain_background_desc(desc, 0, 255);
    terrain_background_desc(desc + 1, 128, 63);
}
