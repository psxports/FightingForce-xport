#include <string.h>
#include <stdlib.h>

#include "game.h"
#include "platform.h"
#include "psx.h"
/* WIP BIOS card events: opaque handles, no asynchronous card completion. */
uint32 card_backend_events[8][5];
uint8 card_backend_pending[8];

/* WIP card requests deliver the no-media event (spec0100), never success. */
void card_backend_request_no_media(uint32 grp)
{
    uint32 i;
    ++platform_boundary_calls[0];
    for (i = 0; i < 8; i++)
        if (card_backend_events[i][0] == (grp ? 0xf0000011u : 0xf4000001u) && card_backend_events[i][1] == 0x100)
            card_backend_pending[i] = 1;
}

sint32 card_backend_test_event(uint32 handle)
{
    uint32 i = handle - 0xf1000000u;
    ++platform_boundary_calls[0];
    if (i >= 8 || !card_backend_events[i][4] || !card_backend_pending[i])
        return 0;
    card_backend_pending[i] = 0;
    return 1;
}

static uint32 card_event_count;

void card_backend_reset_events(void)
{
    card_event_count = 0;
    memset(card_backend_events, 0, sizeof(card_backend_events));
    memset(card_backend_pending, 0, sizeof(card_backend_pending));
}

uint32 card_backend_open_event(uint32 desc, uint32 spec, uint32 mode, uint32 callback)
{
    uint32 *e;
    if (card_event_count >= 8)
        abort();
    e = card_backend_events[card_event_count];
    e[0] = desc;
    e[1] = spec;
    e[2] = mode;
    e[3] = callback;
    ++platform_boundary_calls[0];
    return 0xf1000000u + card_event_count++;
}

sint32 card_backend_enable_event(uint32 handle)
{
    uint32 i = handle - 0xf1000000u;
    if (i >= card_event_count)
        return 0;
    card_backend_events[i][4] = 1;
    ++platform_boundary_calls[0];
    return 1;
}

/* Explicit user-authorized dummy services. Callers must handle unavailable data;
 * do not report successful reads or consume uninitialized destination buffers. */
uint32 platform_boundary_calls[3];

sint32 card_backend_read(sint32 slot, void *data, uint32 bytes)
{
    ++platform_boundary_calls[0];
    return -1;
}

GDB_CALL void platform_interrupts_disable(void)
{
    FUNCTION_MARKER(0x8006ce80u, "GAME.EXE");
    platform_interrupts_enabled = 0;
}

GDB_CALL void platform_interrupts_enable(void)
{
    FUNCTION_MARKER(0x8006ce90u, "GAME.EXE");
    platform_interrupts_enabled = 1;
}

GDB_CALL void card_backend_init(uint32 mode)
{
    FUNCTION_MARKER(0x8006de48u, "GAME.EXE");
    ++platform_boundary_calls[0]; /* WIP ChangeClearPAD(0). */
    platform_interrupts_disable();
    card_pad_init(mode);
    /* WIP SKIP DF8C/E044 patch PSX BIOS card interrupt handlers. Native
  * no-media events replace those handlers, without executable RAM patches. */
    platform_interrupts_disable();
    ++platform_boundary_calls[0];
    platform_interrupts_disable();
    ++platform_boundary_calls[0];
    platform_interrupts_enable();
}

GDB_CALL void card_backend_start(void)
{
    FUNCTION_MARKER(0x8006de9cu, "GAME.EXE");
    platform_interrupts_disable();
    card_pad_start();
    ++platform_boundary_calls[0];
    platform_interrupts_enable();
}

GDB_CALL void card_pad_init(uint32 mode)
{
    FUNCTION_MARKER(0x8006df04u, "GAME.EXE");
    /* WIP BIOS B0:4A InitCARD: no physical card controller on the host. */
    ++platform_boundary_calls[0];
}

GDB_CALL void card_pad_start(void)
{
    FUNCTION_MARKER(0x8006df14u, "GAME.EXE");
    /* WIP BIOS B0:4B StartCARD; requests use the no-media event backend. */
    ++platform_boundary_calls[0];
}

GDB_CALL DIRENTRY *card_backend_first_file(const char *path, DIRENTRY *entry)
{
    FUNCTION_MARKER(0x8006cef0u, "GAME.EXE");
    /* BIOS B0:42 firstfile, WIP empty card directory. */
    ++platform_boundary_calls[0];
    return NULL;
}

GDB_CALL sint32 card_backend_erase(const char *path)
{
    FUNCTION_MARKER(0x8006cf10u, "GAME.EXE");
    /* BIOS B0:45 erase, WIP absent card. */
    ++platform_boundary_calls[0];
    return 0;
}

/* Host card boundaries retain the original interrupt enable observation */
uint32 platform_interrupts_enabled = 1;
