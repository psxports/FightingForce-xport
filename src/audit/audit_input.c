/* Controlled GDB/CDB comparison entry; never used by the game frame loop. */
#include "ff.h"
#include <stdio.h>
#include <string.h>
#include <setjmp.h>
static jmp_buf audit_jump;
static uint32 trace[5];

static sint32 record_key(sint32 enabled, uint32 channels)
{
    trace[1] = (uint32)enabled;
    trace[2] = channels;
    return 0;
}

static void record_jump(uint32 buffer, sint32 value)
{
    trace[3] = buffer;
    trace[4] = (uint32)value;
    longjmp(audit_jump, 1);
}

extern void ff_audit_end(void);

int ff_audit_input(void)
{
    FILE *jobs, *out;
    uint32 j[10];
    const uint32 start = 0x80093a18, size = 0x1380;
    if (!ff_load_ram("FF-menu.ram"))
        return 2;
    jobs = xport_fopen("../status/menu/input-jobs.bin", "rb");
    out = xport_fopen("../status/menu/input-native.bin", "wb");
    if (!jobs || !out)
        return 4;
    ff_services.spu_key = record_key;
    ff_services.game_longjmp = record_jump;
    while (fread(j, sizeof(j), 1, jobs) == 1)
    {
        memset(psx_addr(start, size), 0, size);
        memcpy(psx_addr(0x80094748, 8), j + 2, 8);
        memcpy(psx_addr(0x80094770, 8), j + 4, 8);
        w_u8(0x80093a1c + j[1], (uint8)j[6]);
        w_u8(0x80093a20 + j[1], (uint8)j[7]);
        w_u16(0x80093a18 + 2 * j[1], (uint16)j[8]);
        w_u8(0x80093cf8, (uint8)j[9]);
        w_u32(0x8009355c, 0x80094748);
        w_u32(0x80093560, 0x80094770);
        w_u32(0x800945d4, 0x12345678);
        w_u32(0x800945f8, 0x87654321);
        memset(psx_addr(0x80094708, 24), 0xaa, 24);
        memset(trace, 0, sizeof(trace));
        if (setjmp(audit_jump))
            trace[0] = 0x7ffffffe;
        else if (j[0] == 0x8005f5ac)
            trace[0] = (uint32)FUN_8005F5AC((sint32)j[1]);
        else if (j[0] == 0x800573a8)
            trace[0] = (uint32)FUN_800573A8();
        else if (j[0] == 0x8005ec90)
            trace[0] = (uint32)FUN_8005EC90();
        else
            return 5;
        fwrite(trace, sizeof(trace), 1, out);
        fwrite(psx_addr(start, size), 1, size, out);
    }
    fclose(jobs);
    fclose(out);
    ff_audit_end();
    return 0;
}
