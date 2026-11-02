#include <stdio.h>
#include <string.h>

#include "game.h"
#include "psx_gpu.h"

/* PDT advances over 32 header bytes but reads only this six-byte prefix */
typedef struct TEXTURE_FILE_HEADER
{
    sint16 w;
    sint16 h;
    uint8 flags;
    sint8 mode;
} TEXTURE_FILE_HEADER;

typedef char TEXTURE_FILE_HEADER_LAYOUT[(sizeof(TEXTURE_FILE_HEADER) == 6 && offsetof(TEXTURE_FILE_HEADER, w) == 0 && offsetof(TEXTURE_FILE_HEADER, h) == 2 && offsetof(TEXTURE_FILE_HEADER, flags) == 4 && offsetof(TEXTURE_FILE_HEADER, mode) == 5) ? 1 : -1];
typedef char TEXTURE_RECTANGLE_LAYOUT[(sizeof(PSX_RECT) == 8 && offsetof(PSX_RECT, y) == 2 && offsetof(PSX_RECT, w) == 4 && offsetof(PSX_RECT, h) == 6) ? 1 : -1];

TEXTURE_UPLOAD_RECTANGLES tex_upload_rects;
const TEXTURE_UPLOAD_RECTANGLES tex_upload_rect_defaults = {
    {896, 0, 128, 128},
    {0, 480, 160, 16},
    {{768, 0, 128, 256}, {640, 0, 64, 128}, {704, 0, 32, 64}},
};

TEXTURE_ALLOCATOR tex_allocator;
TEXTURE_PALETTES tex_pals;
RESOURCE_FILE tex_file;
TEXTURE_DESCRIPTORS tex_desc;
TEXTURE_IMAGE tex_images[TEXTURE_DESCRIPTOR_CAPACITY];
sint16 tex_base_v[6];
typedef char TEXTURE_DESCRIPTOR_LAYOUT[(sizeof(TEXTURE_DESCRIPTOR) == 12 && offsetof(TEXTURE_DESCRIPTOR, clut) == 2 && offsetof(TEXTURE_DESCRIPTOR, tpage) == 6 && offsetof(TEXTURE_DESCRIPTOR, uv2) == 8 && offsetof(TEXTURE_DESCRIPTOR, uv3) == 10) ? 1 : -1];
static const uint32 blend_mode[4] = {3, 1, 0, 2};
TEXTURE_EFFECT_PALETTES tex_effect_pals;
typedef char TEXTURE_PALETTE_LAYOUT[(sizeof(TEXTURE_PALETTES) == 8192 && sizeof(tex_pals.color[0]) == 512) ? 1 : -1];
sint16 tex_map[TEXTURE_MAP_COUNT];
uint32 tex_count;
uint32 tex_desc_count;

/* Reset the 64x256 allocation map and row occupancy */
uint32 tex_allocator_reset(void)
{
    FUNCTION_MARKER(0x80066EF4, "GAME.EXE");
    uint32 i;
    memset(tex_images, 0, sizeof(tex_images));
    memset(tex_allocator.used, 0, sizeof(tex_allocator.used));
    memset(tex_allocator.row_used, 0, sizeof(tex_allocator.row_used));
    for (i = 0; i < 32; i++)
    {
        memset(tex_allocator.used[i], 1, 160);
        memset(tex_allocator.used[i] + 224, 1, 32);
        memset(tex_allocator.used[i] + 192, 1, 32);
        tex_allocator.row_used[i] = 224;
    }
    for (i = 0; i < 16; i++)
    {
        uint32 n = i < 8 ? 24 : 16;
        memset(tex_allocator.used[i] + 160, 1, n);
        tex_allocator.row_used[i] += n;
    }
    for (i = 60; i < 62; i++)
    {
        memset(tex_allocator.used[i], 1, 40);
        tex_allocator.row_used[i] += 40;
    }
    for (i = 62; i < 64; i++)
    {
        memset(tex_allocator.used[i], 1, 64);
        tex_allocator.row_used[i] = 64;
    }
    return 0;
}

static uint32 region_free(sint32 x, sint32 y, sint32 width, sint32 height)
{
    FUNCTION_MARKER(0x80066E4C, "GAME.EXE");
    sint32 row, col, yend = y + (uint32)height, xend = x + (uint32)width;
    for (row = y; row < yend; row = row + 1u)
    {
        XASSERT(row >= 0 && row < TEXTURE_ALLOC_ROWS);
        if ((sint16)tex_allocator.row_used[row] == 256)
            return 0;
        for (col = x; col < xend; col = col + 1u)
        {
            XASSERT(col >= 0 && col < TEXTURE_ALLOC_COLS);
            if (tex_allocator.used[row][col])
                return 0;
        }
    }
    return 1;
}

uint32 tex_alloc_pal(void)
{
    FUNCTION_MARKER(0x80066D14, "GAME.EXE");
    uint32 y, x;
    for (y = 32; y < 64; y++)
    {
        if ((sint16)tex_allocator.row_used[y] == 256)
            continue;
        for (x = 0; x < 256; x += 4)
        {
            if (tex_allocator.used[y][x] || tex_allocator.used[y][x + 1] || tex_allocator.used[y][x + 2] || tex_allocator.used[y][x + 3])
                continue;
            tex_allocator.used[y][x] = 1;
            tex_allocator.used[y][x + 1] = 1;
            tex_allocator.used[y][x + 2] = 1;
            tex_allocator.used[y][x + 3] = 1;
            tex_allocator.row_used[y] += 4;
            tex_allocator.x = 4 * x;
            tex_allocator.y = 8 * y;
            return 8 * y;
        }
    }

    XASSERT(0);
    return 0;
}

TEXTURE_GROUP tex_grps[TEXTURE_GROUP_COUNT];

const char *const tex_names[TEXTURE_NAME_COUNT] = {
    "FXSPRITE.PDT", "COMMON.PDT",  "OBJECT.PDT",   "LANGUAGE.PDT", "HAWK.PDT",    "MACE.PDT",    "SMASHER.PDT", "ALANA.PDT",   "HAWK2.PDT",   "BADDIE1.PDT", "BADDIE5.PDT", "THUG1.PDT",   "VAMP1.PDT",   "FATGUY1.PDT", "ZENG.PDT",     "BADDIE2.PDT",  "WASP1.PDT",    "MONKEY1.PDT",  "SECURIT1.PDT", "DOG1.PDT",     "BADBABE1.PDT", "FEMSOL1.PDT",  "BIGBLAK1.PDT", "DRIVER1.PDT", "XOM1.PDT",     "SOLDIER1.PDT", "SAS1.PDT",    "WETSUIT1.PDT", "RADIO1.PDT",  "GLOVE1.PDT",  "BIKER1.PDT",  "AIRBIKR1.PDT", "JETPAC1.PDT", "EXO1.PDT",   "VULKAN1.PDT", "TRAMP1.PDT",   "CRYO1.PDT",    "STAGE11.PDT",  "STAGE12.PDT",  "STAGE13.PDT",  "STAGE14.PDT",  "STAGE15.PDT",  "STAGE21.PDT", "STAGE31.PDT",  "STAGE32.PDT", "STAGE33.PDT", "STAGE33A.PDT", "STAGE41.PDT", "STAGE51.PDT",  "STAGE61.PDT",
    "STAGE62.PDT",  "STAGE63.PDT", "STAGE64.PDT",  "STAGE71.PDT",  "STAGE72.PDT", "STAGE73.PDT", "STAGEC5.PDT", "STAGEC6.PDT", "STAGEC7.PDT", "STAGEC9.PDT", "BACK13.PDT",  "TITLE.PDT",   "TITLE1.PDT",  "FONTS.PDT",   "GAMEFONT.PDT", "CREDITS1.PDT", "CREDITS2.PDT", "CREDITS3.PDT", "CREDITS4.PDT", "CREDITS5.PDT", "CREDITS6.PDT", "FXSPRITE.PPL", "COMMON.PPL",   "OBJECT.PPL",  "LANGUAGE.PPL", "HAWK.PPL",     "MACE.PPL",    "SMASHER.PPL",  "ALANA.PPL",   "HAWK2.PPL",   "BADDIE1.PPL", "BADDIE5.PPL",  "THUG1.PPL",   "VAMP1.PPL",  "FATGUY1.PPL", "ZENG.PPL",     "BADDIE2.PPL",  "WASP1.PPL",    "MONKEY1.PPL",  "SECURIT1.PPL", "DOG1.PPL",     "BADBABE1.PPL", "FEMSOL1.PPL", "BIGBLAK1.PPL", "DRIVER1.PPL", "XOM1.PPL",    "SOLDIER1.PPL", "SAS1.PPL",    "WETSUIT1.PPL", "RADIO1.PPL",
    "GLOVE1.PPL",   "BIKER1.PPL",  "AIRBIKR1.PPL", "JETPAC1.PPL",  "VULKAN1.PPL", "TRAMP1.PPL",  "CRYO1.PPL",   "EXO1.PPL",    "STAGE11.PPL", "STAGE12.PPL", "STAGE13.PPL", "STAGE14.PPL", "STAGE15.PPL", "STAGE21.PPL", "STAGE31.PPL",  "STAGE32.PPL",  "STAGE33.PPL",  "STAGE33A.PPL", "STAGE41.PPL",  "STAGE51.PPL",  "STAGE61.PPL",  "STAGE62.PPL",  "STAGE63.PPL",  "STAGE64.PPL", "STAGE71.PPL",  "STAGE72.PPL",  "STAGE73.PPL", "STAGEC5.PPL",  "STAGEC6.PPL", "STAGEC7.PPL", "STAGEC9.PPL", "BACK13.PPL",   "TITLE.PPL",   "TITLE1.PPL", "FONTS.PPL",   "GAMEFONT.PPL", "CREDITS1.PPL", "CREDITS2.PPL", "CREDITS3.PPL", "CREDITS4.PPL", "CREDITS5.PPL", "CREDITS6.PPL", "STAGE22.PDT", "STAGE22.PPL",  "STAGEFE.PDT", "STAGEFE.PPL", "LOGO.PDT",     "LOGO.PPL",
};

uint32 tex_load_background(const char *tex_name, const char *pal_name, sint32 upload_pixels)
{
    FUNCTION_MARKER(0x80067108, "GAME.EXE");
    RESOURCE_VIEW tex = resource_find(tex_name), pal;
    const uint16 *colors;
    PSX_RECT *rect = &tex_upload_rects.background;
    uint32 bytes, available = tex.storage_len - (uint32)(tex.data - tex.storage);
    uint8 first, last;
    XASSERT(available >= 32800);
    first = tex.data[32];
    last = tex.data[32799];
    rect->y = 0;
    if (upload_pixels)
    {
        XASSERT(rect->w >= 0 && rect->h >= 0);
        bytes = rect->w * (uint32)rect->h * sizeof(uint16);
        XASSERT(bytes <= available - 32 && !((uintptr_t)(tex.data + 32) & 3));
        {
            sint32 checked_result = LoadImagePSX(rect, (uint32 *)(tex.data + 32));
            XASSERT(checked_result == 0);
        }
    }
    DrawSync(0);
    rect->y = 128;
    if (upload_pixels)
    {
        XASSERT(rect->w >= 0 && rect->h >= 0 && available >= 32832);
        bytes = rect->w * (uint32)rect->h * sizeof(uint16);
        XASSERT(bytes <= available - 32832 && !((uintptr_t)(tex.data + 32832) & 3));
        {
            sint32 checked_result = LoadImagePSX(rect, (uint32 *)(tex.data + 32832));
            XASSERT(checked_result == 0);
        }
    }
    DrawSync(0);
    pal = resource_find(pal_name);
    XASSERT(pal.byte_len >= sizeof(tex_pals.color[0]) && !((uintptr_t)pal.data & 1));
    colors = (const uint16 *)pal.data;
    background.first_color = colors[first] & 0x7fff;
    background.last_color = colors[last] & 0x7fff;
    return resource_copy_block(pal.data, (uint8 *)tex_pals.words);
}

/* 8006055C..80060710: Build effect palette tables from group zero */
void tex_build_effect_pals(uint16 *pal)
{
    uint32 level, i;
    for (level = 0; level < 9; level++)
        for (i = 0; i < 16; i++)
        {
            uint32 c = pal[80 + i];
            sint32 r = (c & 31) - 4 * level, g = ((c >> 5) & 31) - 4 * level, b = ((c >> 10) & 31) - 4 * level;
            if (r < 0)
                r = 0;
            if (g < 0)
                g = 0;
            if (b < 0)
                b = 0;
            tex_effect_pals.fade[EFFECT_PAL_COLORS * level + i] = 0x8000 + r + 32 * g + 1024 * b;
        }
    for (i = 0; i < 15; i++)
        tex_effect_pals.cycle[i] = pal[16 + i];
    if (game_session.stage == 0)
    {
        for (i = 0; i < 8; i++)
        {
            uint32 c = pal[48 + i] | 0x8000;
            uint32 r = c & 31, g = (c >> 5) & 31, b = (c >> 10) & 31;
            pal[48 + i] = c;
            tex_effect_pals.stage0[i] = c;
            if (r)
                r--;
            if (g)
                g--;
            if (b)
                b--;
            tex_effect_pals.stage0_dim[i] = 0x8000 + r + 32 * g + 1024 * b;
        }
    }
    /* Stages1,3,4 run a memory-free delay loop in the original. */
}

/* 800609B8..80060B8C: Build a descriptor before group-specific UV changes */
TEXTURE_DESCRIPTOR *tex_build_desc(uint32 width, uint32 height, uint32 mode, uint32 flags, uint32 allocated, uint32 pal_slot, sint32 pal_x, sint32 pal_y_next)
{
    uint32 idx = tex_count, count = (uint16)tex_desc_count, x, y, page, abr;
    TEXTURE_DESCRIPTOR *desc;
    XASSERT(idx < TEXTURE_MAP_COUNT);
    tex_map[idx] = count;
    count = tex_desc_count;
    uint32 desc_idx = count + 1 - allocated;
    XASSERT(desc_idx < TEXTURE_DESCRIPTOR_CAPACITY);
    desc = tex_desc.item + desc_idx;
    tex_desc_count = count + allocated;
    desc->clut = (pal_slot + 496) << 6;
    if (mode == 0)
        desc->clut = (uint32)(pal_x >> 4) + (((uint32)pal_y_next - 1) << 6);
    x = tex_allocator.x;
    y = tex_allocator.y;
    XASSERT(flags < 4);
    abr = blend_mode[flags];
    page = (uint16)getTPage(mode, abr, ((sint32)((x & 0xff00) << 16)) >> 17, y & 256);
    desc->tpage = page;
    x = (uint8)tex_allocator.x;
    tex_allocator.x = x;
    y = (uint8)tex_allocator.y;
    tex_allocator.y = y;
    if (mode == 0)
    {
        x *= 2;
        tex_allocator.x = x;
        if (x >= 256)
        {
            desc->tpage++;
            x &= 255;
            tex_allocator.x = x;
        }
    }
    if (mode == 2)
        tex_allocator.x = (uint32)((sint16)tex_allocator.x >> 1);
    x = (uint8)tex_allocator.x;
    desc->uv2.u = (uint8)x;
    desc->uv0.u = (uint8)x;
    y = (uint8)tex_allocator.y;
    desc->uv1.v = (uint8)y;
    desc->uv0.v = (uint8)y;
    tex_images[desc_idx] = (TEXTURE_IMAGE){(uint16)x, (uint16)y, (uint16)width, (uint16)height, (uint8)mode, (uint8)allocated};
    x = (uint8)tex_allocator.x + width - 1;
    desc->uv3.u = (uint8)x;
    desc->uv1.u = (uint8)x;
    y = (uint8)tex_allocator.y + height - 1;
    desc->uv3.v = (uint8)y;
    desc->uv2.v = (uint8)y;
    if (flags)
        desc->uv0.u++;
    return desc;
}

static void uv_span(TEXTURE_DESCRIPTOR *desc, sint16 *target)
{
    uint8 y = (uint8)tex_allocator.y;
    desc->uv0.v = y;
    *target = (sint8)y;
    y = (uint8)tex_allocator.y + 31;
    desc->uv3.v = y;
    desc->uv2.v = y;
}

/* 80060B90..80060D60: Apply group-specific animated UV limits */
void tex_fix_uv(uint32 grp_idx, uint32 tex_idx, TEXTURE_DESCRIPTOR *desc)
{
    uint32 stage;
    if (grp_idx == 70)
    {
        stage = game_session.stage;
        if (stage == 0)
        {
            if (tex_idx - 6 < 2)
                uv_span(desc, tex_base_v + (tex_idx - 6));
            stage = game_session.stage;
        }
        if (stage - 12 < 2 && tex_idx - 136 < 4)
            uv_span(desc, tex_base_v + (tex_idx - 136));
    }
    else if (grp_idx == 8 && tex_idx == 62)
        uv_span(desc, tex_base_v);
    else if (grp_idx == 9 && tex_idx - 40 < 4)
        uv_span(desc, tex_base_v + (tex_idx - 40));
    else if (grp_idx == 20 && tex_idx - 144 < 6)
        uv_span(desc, tex_base_v + (tex_idx - 144));
    else if (grp_idx == 23 && tex_idx - 92 < 2)
        uv_span(desc, tex_base_v + (tex_idx - 92));
}

uint32 tex_load_grps(uint32 all_characters)
{
    FUNCTION_MARKER(0x80060394, "GAME.EXE");
    uint32 i, slot = 2, left = 0;
    sint32 palx = 0, paly = 0;
    tex_allocator_reset();
    tex_count = 0;
    tex_desc_count = 0;
    tex_allocator.small_img_count = 0;
    for (i = 0; i < TEXTURE_GROUP_COUNT; i++)
    {
        const char *name;
        TEXTURE_GROUP *grp = &tex_grps[i];
        RESOURCE_VIEW image;
        uint32 offset, available, ordinal, skip = 0;
        if (i >= TEXTURE_GROUP_HAWK && i <= TEXTURE_GROUP_HAWK2 && actor_types[0] != (sint32)(i - TEXTURE_GROUP_HAWK))
            skip = actor_types[1] != (sint32)(i - TEXTURE_GROUP_HAWK);
        if (all_characters)
            skip = 0;
        if (skip)
            continue;
        name = grp->pal_name;
        if (name && resource_exists(name))
        {
            RESOURCE_VIEW pal = resource_find(grp->pal_name);
            XASSERT(pal.byte_len >= sizeof(tex_pals.color[0]));
            if (i == TEXTURE_GROUP_EFFECTS)
            {
                XASSERT(!((uintptr_t)pal.data & 1));
                tex_build_effect_pals((uint16 *)pal.data);
            }
            ++slot;
            XASSERT(slot < TEXTURE_PAL_COUNT);
            resource_copy_block(pal.data, (uint8 *)tex_pals.color[slot]);
        }
        name = grp->img_name;
        if (!name || !resource_exists(name))
            continue;
        image = resource_find(grp->img_name);
        offset = 0;
        available = image.storage_len - (uint32)(image.data - image.storage);
        ordinal = 0;
        grp->first_tex = (uint16)tex_count;
        while (offset < resource_io.result_size)
        {
            const TEXTURE_FILE_HEADER *file;
            sint32 w, h, mode;
            uint32 flags, size, allocated = 1, pixels;
            TEXTURE_DESCRIPTOR *d;
            XASSERT(offset <= available && sizeof(*file) <= available - offset && !((uintptr_t)(image.data + offset) & 1));
            file = (const TEXTURE_FILE_HEADER *)(image.data + offset);
            w = file->w;
            h = file->h;
            flags = file->flags & 3;
            pixels = w * h;
            mode = file->mode;
            offset += 32;
            XASSERT(w < 258 && h < 258);
            size = ((mode == 0 ? ((sint32)pixels >> 1) + 32u : mode == 1 ? pixels : pixels << 1) + 31) & ~31u;
            if (w == 8 && h == 8)
            {
                flags = 0;
                mode = 1;
                allocated = 0;
                tex_allocator.small_img_count++;
            }
            else
            {
                PSX_RECT rect;
                if (mode == 0)
                {
                    if (!left)
                    {
                        tex_alloc_pal();
                        palx = (sint16)tex_allocator.x;
                        paly = (sint16)tex_allocator.y;
                        left = 8;
                    }
                    XASSERT(offset <= available && 32 <= available - offset && !((uintptr_t)(image.data + offset) & 3));
                    LoadClut((uint32 *)(image.data + offset), (sint16)palx, (sint16)paly);
                    paly = paly + 1u;
                    left--;
                    offset += 32;
                    size -= 32;
                }
                tex_alloc_image(w, h, mode, &rect);
                XASSERT(rect.w >= 0 && rect.h >= 0);
                uint32 bytes = rect.w * (uint32)rect.h * sizeof(uint16);
                XASSERT(offset <= available && bytes <= available - offset && !((uintptr_t)(image.data + offset) & 3));
                {
                    sint32 checked_result = LoadImagePSX(&rect, (uint32 *)(image.data + offset));
                    XASSERT(checked_result == 0);
                }
                DrawSync(0);
                offset += size;
            }
            d = tex_build_desc(w, h, mode, flags, allocated, slot, palx, paly);
            tex_fix_uv(i, ordinal, d);
            tex_count = tex_count + 1;
            ordinal++;
        }
    }
    terrain_build_descs();
    return terrain_relocate_tiles();
}

uint32 tex_alloc_image(sint32 width, sint32 height, sint32 mode, PSX_RECT *rect)
{
    FUNCTION_MARKER(0x80066ACC, "GAME.EXE");
    sint32 w = width;
    sint32 h;
    sint32 y;
    sint32 band;
    sint32 page;
    sint32 x;
    sint32 row;
    sint32 col;
    uint32 step;
    sint32 ylimit;
    sint32 xlimit;
    sint32 row_end;
    step = 16u << (mode & 31u);
    if (mode == 0)
        w >>= 1;
    if (mode == 2)
        w = (uint32)w << 1;
    w = (sint32)(w + 7u) >> 3;
    h = (sint32)(height + 7u) >> 3;
    if (w > 32)
        w = 32;
    if (h > 32)
        h = 32;
    for (band = 0; band < 64; band += 32)
    {
        ylimit = band + 33u - h;
        for (y = band; y < ylimit; y = y + 1u)
        {
            XASSERT(y >= 0 && y < TEXTURE_ALLOC_ROWS);
            if ((sint16)tex_allocator.row_used[y] == 256)
                continue;
            for (page = 0; page < 256; page = page + step)
            {
                xlimit = page + step - (w - 1u);
                for (x = page; x < xlimit; x = x + 1u)
                {
                    if (!region_free(x, y, w, h))
                        continue;
                    row_end = y + (uint32)h;
                    for (row = y; row < row_end; row = row + 1u)
                    {
                        /* Original byte fill does nothing for a nonpositive count */
                        for (col = 0; col < w; ++col)
                            tex_allocator.used[row][x + col] = 1;
                        tex_allocator.row_used[row] += (uint32)w;
                    }
                    tex_allocator.x = (uint32)x << 3;
                    tex_allocator.y = (uint32)y << 3;
                    rect->x = (uint32)x << 2;
                    rect->y = (uint32)y << 3;
                    rect->w = (uint32)w << 2;
                    rect->h = (uint32)h << 3;
                    return (uint32)h << 3;
                }
            }
        }
    }

    XASSERT(0);
    return 0;
}

uint32 tex_upload_pals(void)
{
    FUNCTION_MARKER(0x800645E8, "GAME.EXE");
    PSX_RECT rect = {0, 496, TEXTURE_PAL_COLORS, TEXTURE_PAL_COUNT};
    {
        sint32 checked_result = LoadImagePSX(&rect, tex_pals.words);
        XASSERT(checked_result == 0);
    }
    return 0;
}
