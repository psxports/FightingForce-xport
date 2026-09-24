#include "ff.h"
#include <stdio.h>
#include <string.h>

sint32 ff_load_ram(const char *path)
{
    FILE *f = fopen(path, "rb");
    size_t n;
    int extra;
    if (!f)
        return 0;
    n = fread(DRAM, 1, sizeof(DRAM), f);
    extra = fgetc(f);
    fclose(f);
    return n == sizeof(DRAM) && extra == EOF;
}

/* Restore only reviewed initialized data, never original MIPS code */
sint32 ff_load_game_image(void)
{
    const uint8 *data = ff_initial_data;
    size_t data_size = ff_initial_data_size;
    size_t cursor;
    uint32 address;
    uint32 chunk_count;
    uint32 chunk_size;
    uint32 i;
    uint32 magic;
    if (data_size < 8)
        return 0;
    memcpy(&magic, data, 4);
    memcpy(&chunk_count, data + 4, 4);
    if (magic != 0x31444646 || chunk_count == 0 || chunk_count > 32 || data_size < 8u + 8u * chunk_count)
        return 0;
    cursor = 8u + 8u * chunk_count;
    for (i = 0; i < chunk_count; ++i)
    {
        memcpy(&address, data + 8u + 8u * i, 4);
        memcpy(&chunk_size, data + 12u + 8u * i, 4);
        if (address < 0x80000000u || chunk_size > sizeof(DRAM) || address - 0x80000000u > sizeof(DRAM) - chunk_size || cursor > data_size || chunk_size > data_size - cursor)
            return 0;
        cursor += chunk_size;
    }
    if (cursor != data_size)
        return 0;
    memset(DRAM, 0, sizeof(DRAM));
    memset(SCRATCHPAD, 0, sizeof(SCRATCHPAD));
    cursor = 8u + 8u * chunk_count;
    for (i = 0; i < chunk_count; ++i)
    {
        memcpy(&address, data + 8u + 8u * i, 4);
        memcpy(&chunk_size, data + 12u + 8u * i, 4);
        memcpy(DRAM + address - 0x80000000u, data + cursor, chunk_size);
        cursor += chunk_size;
    }
    return 1;
}
