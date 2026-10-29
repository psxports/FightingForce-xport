#include <stddef.h>

#include "game.h"

RENDER_BUFFERS render_buffers;

/* Forward links preserve the supplied physical or KSEG alias bits */
sint32 render_ot_init(uint32 table_addr)
{
    FUNCTION_MARKER(0x80011d50, "GAME.EXE");
    uint32 *table = psx_addr(table_addr, RENDER_OT_COUNT * sizeof(*table));
    uint32 i;
    for (i = 0; i < RENDER_OT_COUNT - 1; i++)
        table[i] = table_addr + 4u * (i + 1);
    table[RENDER_OT_COUNT - 1] = RENDER_OT_END;
    return RENDER_OT_END;
}

/* The final empty run links its last node, not the terminal address */
sint32 render_ot_compact(void)
{
    FUNCTION_MARKER(0x80011d9c, "GAME.EXE");
    uint32 cur = render_buffers.ot_addr, tag, next, node;
    for (;;)
    {
        uint32 *prev_tag = psx_addr(cur, sizeof(*prev_tag));
        tag = *prev_tag;
        next = tag & 0xffffff;
        if (next == RENDER_OT_END)
            return RENDER_OT_END;
        cur = next;
        if (tag & 0xff000000)
            continue;
        for (;;)
        {
            node = cur;
            const uint32 *cur_tag = psx_addr(cur, sizeof(*cur_tag));
            tag = *cur_tag;
            next = tag & 0xffffff;
            if (next == RENDER_OT_END)
            {
                *prev_tag = cur;
                return RENDER_OT_END;
            }
            cur = next;
            if (tag & 0xff000000)
                break;
        }
        *prev_tag = (*prev_tag & 0xff000000) | node;
    }
}

uint32 render_ot_insert_flags(uint32 bucket_addr, uint32 packet_addr, uint32 flags)
{
    FUNCTION_MARKER(0x80011e28, "GAME.EXE");
    uint32 *ot_entry = psx_addr(bucket_addr, sizeof(*ot_entry));
    uint32 prev = *ot_entry;
    *ot_entry = packet_addr;
    uint32 *packet_tag = psx_addr(packet_addr, sizeof(*packet_tag));
    *packet_tag = prev | flags;
    return prev;
}

/* Read the signed tag byte before the bucket, including when both alias */
uint32 render_ot_insert(uint32 bucket_addr, uint32 packet_addr)
{
    FUNCTION_MARKER(0x80011e40, "GAME.EXE");
    uint32 *packet_tag = psx_addr(packet_addr, sizeof(*packet_tag));
    uint32 tag = (uint32)(sint32)((const sint8 *)packet_tag)[3] << 24;
    uint32 *ot_entry = psx_addr(bucket_addr, sizeof(*ot_entry));
    uint32 prev = *ot_entry;
    tag |= prev;
    *ot_entry = packet_addr;
    *packet_tag = tag;
    return prev;
}

uint32 render_buffers_select(void)
{
    FUNCTION_MARKER(0x80064684, "GAME.EXE");
    uint32 parity = disp_buffers.frame & 1;
    render_buffers.prev_ot_addr = parity ? 0xe5140 : 0xe7140;
    parity = disp_buffers.frame & 1;
    render_buffers.ot_addr = parity ? 0xe7140 : 0xe5140;
    parity = disp_buffers.frame & 1;
    render_buffers.packet_addr = parity ? 0xff8a0 : 0xe9140;
    return parity;
}

uint32 render_insert_tex_page(uint32 page, uint32 bucket)
{
    FUNCTION_MARKER(0x80066a44u, "GAME.EXE");
    uint32 p = render_buffers.packet_addr;
    render_buffers.packet_addr = p + 12;
    DR_MODE *draw_mode = psx_addr(p, sizeof(*draw_mode));
    SetDrawMode(draw_mode, 1, 0, page, NULL);
    return render_ot_insert(render_buffers.ot_addr + 4 * bucket, p);
}
