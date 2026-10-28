#ifndef TERRAIN_H
#define TERRAIN_H

#include "xport.h"
#include "resources.h"

enum
{
    TERRAIN_SIDE = 64,
    TERRAIN_COLOR_SIDE = 128,
    TERRAIN_VIEW_DIR_COUNT = 16
};

typedef struct TERRAIN_VIEW_OFFSET
{
    sint32 x, z;
} TERRAIN_VIEW_OFFSET;

extern const TERRAIN_VIEW_OFFSET terrain_view_offset[TERRAIN_VIEW_DIR_COUNT];

enum
{
    BACKGROUND_STRIP_WIDTH = 320,
    BACKGROUND_STRIP_HEIGHT = 20
};

typedef struct TERRAIN_MAP
{
    union
    {
        uint16 tile[TERRAIN_SIDE][TERRAIN_SIDE];
        uint16 tile_flat[TERRAIN_SIDE * TERRAIN_SIDE];
    };

    union
    {
        uint8 color[TERRAIN_COLOR_SIDE][TERRAIN_COLOR_SIDE];
        uint8 color_flat[TERRAIN_COLOR_SIDE * TERRAIN_COLOR_SIDE];
    };
} TERRAIN_MAP;

typedef union BACKGROUND_IMAGE
{
    uint8 row[BACKGROUND_STRIP_HEIGHT][BACKGROUND_STRIP_WIDTH];
    uint8 pix[BACKGROUND_STRIP_HEIGHT * BACKGROUND_STRIP_WIDTH];
    uint32 words[BACKGROUND_STRIP_HEIGHT * BACKGROUND_STRIP_WIDTH / sizeof(uint32)];
} BACKGROUND_IMAGE;

extern TERRAIN_MAP terrain_map;
extern BACKGROUND_IMAGE background_img;

GDB_CALL uint32 terrain_draw_grid(sint32 unused, sint32 tile_x, sint32 tile_z);
GDB_CALL uint32 terrain_draw_faded_grid(uint32 unused, uint32 column, uint32 row);
GDB_CALL sint32 background_render_strip(void);
GDB_CALL sint32 terrain_draw(void);
GDB_CALL RESOURCE_VIEW terrain_load_texs(const char *name);
GDB_CALL uint32 terrain_build_map(RESOURCE_VIEW map, RESOURCE_VIEW pixels);
GDB_CALL uint32 terrain_relocate_tiles(void);
GDB_CALL uint32 terrain_load(const char *tex_name, const char *map_name);
GDB_CALL void terrain_build_descs(void);

#endif
