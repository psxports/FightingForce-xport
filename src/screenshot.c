#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "game.h"
#include "psx_gpu.h"

SCREENSHOT screenshot;
const SCREENSHOT screenshot_defaults = {
    .filename = "FIGHT001.TGA",
    .tga_header = {0, 0, 2, 0, 0, 0, 0, 0, 0, 0, 0, 0, 64, 1, 0, 1, 16, 0},
    .rect = {0, 0, 320, 16},
};

/* PCdrv handles remain separate from CRT descriptors and complete within capture */
static FILE *files[16];

static void file_cleanup(void)
{
    uint32 i;
    for (i = 0; i < 16; ++i)
        if (files[i])
        {
            fclose(files[i]);
            files[i] = NULL;
        }
}

static sint32 file_create(const char *name, uint32 available)
{
    char path[128];
    uint32 i, n;
    static int registered;
    /* TODO: Investigate missing or unterminated native screenshot names */
    XASSERT(name && available);
    for (n = 0; n < sizeof(path) - 1 && n < available; ++n)
    {
        path[n] = name[n];
        if (!path[n])
            break;
        if (path[n] == '/' || path[n] == '\\' || path[n] == ':')
            return -1;
    }
    XASSERT(n != available);
    if (n == sizeof(path) - 1 || !n || !strcmp(path, ".") || !strcmp(path, ".."))
        return -1;
    for (i = 0; i < 16; ++i)
        if (!files[i])
            break;
    if (i == 16)
        return -1;
    if (!registered)
    {
        if (atexit(file_cleanup))
            return -1;
        registered = 1;
    }
    files[i] = xport_fopen(path, "wb");
    return files[i] ? (sint32)i + 1 : -1;
}

static sint32 file_close(uint32 handle)
{
    FUNCTION_MARKER(0x80079fc4u, "GAME.EXE");
    FILE *file;
    int result;
    if (!handle || handle > 16 || !files[handle - 1])
        return -1;
    file = files[handle - 1];
    files[handle - 1] = NULL;
    result = fclose(file);
    return result ? -1 : 0;
}

static sint32 file_write_chunk(uint32 handle, uint32 bytes, const uint8 *src)
{
    FUNCTION_MARKER(0x8007a0b4u, "GAME.EXE");
    size_t written;
    FILE *file;
    if (!handle || handle > 16 || !files[handle - 1])
        return -1;
    file = files[handle - 1];
    /* TODO: Investigate an invalid native write span without guest-address fallback */
    XASSERT(!bytes || src);
    written = fwrite(src, 1, bytes, file);
#if defined(XPORT_XCRT)
    /* XCRT writes synchronously and returns the actual native byte count */
    return written == bytes ? (sint32)written : -1;
#else
    return ferror(file) ? -1 : (sint32)written;
#endif
}

static uint32 file_write(uint32 handle, const uint8 *src, uint32 bytes)
{
    FUNCTION_MARKER(0x80079ff4u, "GAME.EXE");
    uint32 total = 0, chunk;
    sint32 written;
    while (bytes)
    {
        chunk = bytes > 0x8000 ? 0x8000 : bytes;
        written = file_write_chunk(handle, chunk, src);
        total += (uint32)written;
        if (written == -1)
            return 0xffffffffu;
        src += (uint32)written;
        bytes -= (uint32)written;
        if (written < (sint32)chunk)
            break;
    }
    return total;
}

uint32 screenshot_capture(void)
{
    FUNCTION_MARKER(0x80067940u, "GAME.EXE");
    uint32 handle = file_create((const char *)screenshot.filename, sizeof(screenshot.filename)), block, row, offset, val;
    uint32 words[4], idx, capture_bytes;
    uint8 *pix, *src, *dst;
    uint16 pixel;
    if (!handle)
        return 0; /* Original tests zero, so the -1 error also enters. */
    /* TODO: Investigate the original -1 create result instead of using an invalid handle */
    XASSERT(handle != 0xffffffff);
    file_write(handle, screenshot.tga_header, sizeof(screenshot.tga_header));
    for (block = 0; block < 256; block += 16)
    {
        DrawSync(0);
        screenshot.rect.y = (sint16)(240 - block);
        /* TODO: Investigate invalid imported capture geometry before device access */
        XASSERT(screenshot.rect.w >= 0 && screenshot.rect.h >= 0);
        capture_bytes = screenshot.rect.w * (uint32)screenshot.rect.h * sizeof(uint16);
        /* TODO: Investigate capture buffers shorter than the original fixed pixel operation */
        XASSERT(resource_arenas.scratch.data && resource_arenas.scratch.available >= capture_bytes && resource_arenas.scratch.available >= 10240);
        StoreImage(&screenshot.rect, (uint32 *)resource_arenas.scratch.data);
        DrawSync(0);
        /* Reload after the device synchronization, as in the original */
        XASSERT(resource_arenas.scratch.data && resource_arenas.scratch.available >= 10240);
        pix = resource_arenas.scratch.data;
        for (offset = 0; offset < 5120; ++offset, pix += 2)
        {
            memcpy(&pixel, pix, sizeof(pixel));
            val = pixel;
            pixel = ((val & 31) << 10) + (val & 0x3e0) + ((val >> 10) & 31);
            memcpy(pix, &pixel, sizeof(pixel));
        }
        for (row = 0; row < 16; ++row)
        {
            XASSERT(resource_arenas.scratch.data && resource_arenas.scratch.available >= 10240 && resource_arenas.persistent.data && resource_arenas.persistent.available >= 10240);
            dst = resource_arenas.persistent.data + 640 * row;
            src = resource_arenas.scratch.data + 640 * (15 - row);
            for (offset = 0; offset < 640; offset += 16)
            {
                /* Four loads precede four stores, including overlapping or unaligned spans */
                for (idx = 0; idx < 4; ++idx)
                    memcpy(words + idx, src + offset + 4 * idx, sizeof(*words));
                for (idx = 0; idx < 4; ++idx)
                    memcpy(dst + offset + 4 * idx, words + idx, sizeof(*words));
            }
        }
        file_write(handle, resource_arenas.persistent.data, 10240);
    }
    file_close(handle);
    val = screenshot.filename[7] + 1;
    screenshot.filename[7] = (uint8)val;
    if ((sint8)val < 58)
        return 1;
    screenshot.filename[7] = 48;
    val = screenshot.filename[6] + 1;
    screenshot.filename[6] = (uint8)val;
    if ((sint8)val < 58)
        return 1;
    screenshot.filename[6] = 48;
    val = screenshot.filename[5] + 1;
    screenshot.filename[5] = (uint8)val;
    return val;
}
