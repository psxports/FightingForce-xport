#ifndef TEXTURE_H
#define TEXTURE_H

#include "xport.h"
#include "psx.h"
#include "resources.h"

enum
{
    TEXTURE_GROUP_COUNT = 79,
    TEXTURE_NAME_COUNT = 148
};

enum
{
    TEXTURE_GROUP_EFFECTS = 0,
    TEXTURE_GROUP_HAWK = 36,
    TEXTURE_GROUP_MACE = 37,
    TEXTURE_GROUP_SMASHER = 38,
    TEXTURE_GROUP_ALANA = 39,
    TEXTURE_GROUP_HAWK2 = 40,
    TEXTURE_GROUP_GAME_FONT = 72,
    TEXTURE_GROUP_LANGUAGE = 73,
    TEXTURE_GROUP_TITLE = 74,
    TEXTURE_GROUP_TITLE_ALT = 75,
    TEXTURE_GROUP_FONTS = 76,
    TEXTURE_GROUP_TERRAIN = 77,
    TEXTURE_GROUP_LOGO = 78
};

typedef struct TEXTURE_GROUP
{
    const char *pal_name;
    const char *img_name;
    sint16 first_tex;
} TEXTURE_GROUP;

extern TEXTURE_GROUP tex_grps[TEXTURE_GROUP_COUNT];
extern const char *const tex_names[TEXTURE_NAME_COUNT];

enum
{
    TEXTURE_PAL_COUNT = 16,
    TEXTURE_PAL_COLORS = 256,
    TEXTURE_PAL_WORDS = TEXTURE_PAL_COLORS / 2,
    TEXTURE_PAL_EFFECTS = 3
};

typedef union TEXTURE_PALETTES
{
    uint16 color[TEXTURE_PAL_COUNT][TEXTURE_PAL_COLORS];
    uint32 words[TEXTURE_PAL_COUNT * TEXTURE_PAL_WORDS];
} TEXTURE_PALETTES;

extern TEXTURE_PALETTES tex_pals;

/* CPU archive outlives its active directory borrow, independently of progress resets */
enum
{
    TEXTURE_ARCHIVE_CAPACITY = 0xea000
};

extern RESOURCE_FILE tex_file;

enum
{
    EFFECT_FADE_LEVELS = 9,
    EFFECT_PAL_COLORS = 16,
    EFFECT_CYCLE_COLORS = 15,
    EFFECT_STAGE0_COLORS = 8
};

typedef struct TEXTURE_EFFECT_PALETTES
{
    uint16 fade[EFFECT_FADE_LEVELS * EFFECT_PAL_COLORS];
    uint16 cycle[EFFECT_CYCLE_COLORS];
    /* TODO: Establish consumers of the two stage-zero tables before assigning an effect role */
    uint16 stage0[EFFECT_STAGE0_COLORS];
    uint16 stage0_dim[EFFECT_STAGE0_COLORS];
} TEXTURE_EFFECT_PALETTES;

extern TEXTURE_EFFECT_PALETTES tex_effect_pals;
GDB_CALL uint32 tex_upload_pals(void);

enum
{
    TEXTURE_MAP_COUNT = 4000,
    TEXTURE_DESCRIPTOR_CAPACITY = TEXTURE_MAP_COUNT + 258
};

/* Packed GPU source data, not a command packet or a host-pointer record */
typedef union TEXTURE_DESCRIPTOR
{
    struct
    {
        UV uv0;
        uint16 clut;
        UV uv1;
        uint16 tpage;
        UV uv2;
        UV uv3;
    };

    uint32 words[3];
} TEXTURE_DESCRIPTOR;

typedef union TEXTURE_DESCRIPTORS
{
    TEXTURE_DESCRIPTOR item[TEXTURE_DESCRIPTOR_CAPACITY];
    uint32 words[TEXTURE_DESCRIPTOR_CAPACITY * 3];
} TEXTURE_DESCRIPTORS;

extern TEXTURE_DESCRIPTORS tex_desc;
extern sint16 tex_base_v[6];

extern sint16 tex_map[TEXTURE_MAP_COUNT];
extern uint32 tex_count;
extern uint32 tex_desc_count;

enum
{
    TEXTURE_ALLOC_ROWS = 64,
    TEXTURE_ALLOC_COLS = 256
};

typedef struct TEXTURE_ALLOCATOR
{
    uint8 used[TEXTURE_ALLOC_ROWS][TEXTURE_ALLOC_COLS];
    uint16 row_used[TEXTURE_ALLOC_ROWS];
    uint16 x;
    uint16 y;
    uint32 small_img_count;
} TEXTURE_ALLOCATOR;

enum
{
    TEXTURE_TERRAIN_BASE,
    TEXTURE_TERRAIN_HALF,
    TEXTURE_TERRAIN_QUARTER,
    TEXTURE_TERRAIN_IMAGE_COUNT
};

typedef struct TEXTURE_UPLOAD_RECTANGLES
{
    PSX_RECT background;
    PSX_RECT terrain_strip;
    PSX_RECT terrain[TEXTURE_TERRAIN_IMAGE_COUNT];
} TEXTURE_UPLOAD_RECTANGLES;

extern TEXTURE_UPLOAD_RECTANGLES tex_upload_rects;
extern const TEXTURE_UPLOAD_RECTANGLES tex_upload_rect_defaults;

extern TEXTURE_ALLOCATOR tex_allocator;

GDB_CALL uint32 tex_allocator_reset(void);
GDB_CALL uint32 tex_alloc_pal(void);
GDB_CALL uint32 tex_alloc_image(sint32 width, sint32 height, sint32 mode, PSX_RECT *rect);

GDB_CALL uint32 tex_load_background(const char *tex_name, const char *pal_name, sint32 upload_pixels);
GDB_CALL void tex_build_effect_pals(uint16 *pal);
GDB_CALL TEXTURE_DESCRIPTOR *tex_build_desc(uint32 width, uint32 height, uint32 mode, uint32 flags, uint32 allocated, uint32 pal_slot, sint32 pal_x, sint32 pal_y_next);
GDB_CALL void tex_fix_uv(uint32 grp_idx, uint32 tex_idx, TEXTURE_DESCRIPTOR *desc);
GDB_CALL uint32 tex_load_grps(uint32 all_characters);

#endif
