#ifndef RENDER_BUFFERS_H
#define RENDER_BUFFERS_H

#include "xport.h"
#include "psx.h"

enum
{
    RENDER_OT_COUNT = 2048,
    RENDER_OT_END = 0x10018
};

/* Device packet workspace extends to the shared mapped RAM limit */
enum
{
    RENDER_WORKSPACE_BASE = 0xe5140,
    RENDER_WORKSPACE_BYTES = PSX_DRAM_SIZE - RENDER_WORKSPACE_BASE,
    RENDER_DRAW_ENV_BASE = 0xb8938,
    RENDER_DRAW_ENV_COUNT = 2
};

_Static_assert(sizeof(DRAWENV) == 92, "DRAWENV device workspace stride");

typedef struct RENDER_BUFFERS
{
    /* PSX GPU DMA addresses, never host pointers or native object identities */
    uint32 packet_addr;
    uint32 ot_addr;
    uint32 prev_ot_addr;
} RENDER_BUFFERS;

extern RENDER_BUFFERS render_buffers;

sint32 render_ot_init(uint32 table_addr);
sint32 render_ot_compact(void);
uint32 render_ot_insert_flags(uint32 bucket_addr, uint32 packet_addr, uint32 flags);
uint32 render_ot_insert(uint32 bucket_addr, uint32 packet_addr);
uint32 render_buffers_select(void);

uint32 render_insert_tex_page(uint32 page, uint32 bucket);

#endif
