#include <stdio.h>
#include <string.h>

#include "game.h"
#include "platform.h"

CARD_STORAGE card_storage;
CARD_FILES card_files;

/* Original immutable save-file identity and encoded header title */
const char card_save_name[CARD_SAVE_NAME_SIZE] = "BASLUS-00433";
const char card_save_title[CARD_SAVE_TITLE_SIZE] = "FightingForce";

/* Assert file boundaries only; native gameplay layouts remain independent */
typedef char CARD_HEADER_LAYOUT[(sizeof(CARD_HEADER) == CARD_HEADER_SIZE && offsetof(CARD_HEADER, title) == 4 && offsetof(CARD_HEADER, pal) == 96 && offsetof(CARD_HEADER, icon) == 128) ? 1 : -1];
typedef char CARD_DATA_LAYOUT[(sizeof(CARD_SAVE_DATA) == CARD_DATA_SIZE && sizeof(CARD_SCORE) == 12 && offsetof(CARD_SAVE_DATA, health) == 4 && offsetof(CARD_SAVE_DATA, score) == 8 && offsetof(CARD_SAVE_DATA, continue_threshold) == 16 && offsetof(CARD_SAVE_DATA, continues) == 24 && offsetof(CARD_SAVE_DATA, route_choice) == 28 && offsetof(CARD_SAVE_DATA, sound) == 32 && offsetof(CARD_SAVE_DATA, difficulty) == 37 && offsetof(CARD_SAVE_DATA, scr_offset) == 40 && offsetof(CARD_SAVE_DATA, high_score) == 44) ? 1 : -1];
typedef char CARD_FILE_LAYOUT[(sizeof(CARD_FILE) == CARD_FILE_SIZE && offsetof(CARD_FILE, data) == CARD_HEADER_SIZE && offsetof(CARD_FILE, tail) == CARD_HEADER_SIZE + CARD_DATA_SIZE) ? 1 : -1];
typedef char CARD_DIRECTORY_PREFIX_LAYOUT[(offsetof(DIRENTRY, attr) == 20 && offsetof(DIRENTRY, size) == 24 && CARD_DIRECTORY_NAME_SPAN == 28) ? 1 : -1];
const CARD_STORAGE card_storage_defaults = {.preview = {-1, -1, {-1, -1}}};

typedef struct CARD_CHARACTER_RANGE
{
    uint16 code;
    uint16 first;
} CARD_CHARACTER_RANGE;

static const CARD_CHARACTER_RANGE character_range[3] = {
    {0x824f, 48},
    {0x8260, 65},
    {0x8281, 97},
};

static const uint16 punctuation_code[33] = {
    0x8140, 0x8149, 0x8168, 0x8194, 0x8190, 0x8193, 0x8195, 0x8166, 0x8169, 0x816a, 0x8196, 0x817b, 0x8143, 0x817c, 0x8144, 0x815e, 0x8146, 0x8147, 0x8171, 0x8181, 0x8172, 0x8148, 0x8197, 0x816d, 0x818f, 0x816e, 0x814f, 0x8151, 0x8165, 0x816f, 0x8162, 0x8170, 0x8150,
};

/* Original packed color pairs and 16x16 4-bit icon words */
static const uint32 icon_pal[8] = {
    0x000b0000, 0x000f0007, 0x000308bb, 0x0014155f, 0x00570000, 0x00000000, 0x00000000, 0x00000000,
};

static const char file_pattern[] = "*";

static const uint32 icon[32] = {
    0x33111225, 0x52211133, 0x66666442, 0x24466666, 0x33779941, 0x34221111, 0x66644943, 0x34221746, 0x77333441, 0x83422249, 0x73311141, 0x88395559, 0x73112212, 0x88519555, 0x57122258, 0x88521755, 0x55732588, 0x85552175, 0x55593588, 0x21222117, 0x95554388, 0x14321137, 0x94552438, 0x34433377, 0x64722241, 0x34944666, 0x11112243, 0x14997733, 0x66666442, 0x24466666, 0x33111225, 0x52211133,
};

GDB_CALL sint32 card_init_events(void)
{
    FUNCTION_MARKER(0x8006b050u, "GAME.EXE");
    static const uint32 specs[4] = {4, 0x8000, 0x100, 0x2000};
    uint32 i;
    sint32 result = 0;
    /* WIP native event namespace is recreated on card subsystem init. */
    card_backend_reset_events();
    for (i = 0; i < 8; ++i)
        card_storage.event[i / CARD_EVENT_COUNT][i % CARD_EVENT_COUNT] = card_backend_open_event(i < 4 ? 0xf4000001u : 0xf0000011u, specs[i & 3], 0x2000, 0);
    card_backend_init(0);
    card_backend_start();
    ++platform_boundary_calls[0]; /* WIP _bu_init. */
    ++platform_boundary_calls[0]; /* WIP ChangeClearPAD(0). */
    for (i = 0; i < 8; ++i)
        result = card_backend_enable_event(card_storage.event[i / CARD_EVENT_COUNT][i % CARD_EVENT_COUNT]);
    return result;
}

/* WIP BIOS file boundary: an absent memory card has no files. The path is
 * still constructed by the translated caller; no successful reads are invented. */
static sint32 open_file_dummy(const char *path, uint32 mode)
{
    ++platform_boundary_calls[0];
    return -1;
}

static sint32 close_file_dummy(sint32 fd)
{
    ++platform_boundary_calls[0];
    return 0;
}

static void build_path(uint32 slot, const char *name, uint32 capacity)
{
    sint32 result;
#if defined(_DEBUG)
    const void *name_end = name ? memchr(name, 0, capacity) : NULL;
    XASSERT(name_end);
#endif
    result = snprintf(card_files.path, sizeof(card_files.path), "bu%2.2x:%s", slot & 255, name);
    XASSERT(result >= 0 && (uint32)result < sizeof(card_files.path));
}

static sint32 write_file_dummy(sint32 fd, const uint8 *src, uint32 bytes)
{
    ++platform_boundary_calls[0];
    return -1; /* WIP absent card. */
}

GDB_CALL sint32 card_write_save(uint32 slot, const char *name, uint32 name_capacity, const char *title, uint32 title_capacity, uint32 notice_context, uint8 *data, uint32 capacity)
{
    FUNCTION_MARKER(0x8006b22cu, "GAME.EXE");
    uint32 words[4];
    int i, j, frame;
    sint32 fd, written;
    if (card_file_exists(slot, name, name_capacity))
    {
        build_path(slot, name, name_capacity);
        if (!card_backend_erase(card_files.path))
            return -2;
    }
    card_files.header.signature[0] = 'S';
    card_files.header.signature[1] = 'C';
    card_files.header.icon_flags = 17;
    card_files.header.blocks = 1;
    card_encode_title(card_files.header.code_units + 2, CARD_HEADER_SIZE / 2 - 2, title, title_capacity);
    /* Palette copied as3+3+2 words; each load group precedes its stores. */
    for (i = 0; i < 8;)
    {
        int n = i < 6 ? 3 : 2;
        for (j = 0; j < n; ++j)
        {
            words[j] = icon_pal[i + j];
        }
        for (j = 0; j < n; ++j)
        {
            card_files.header.pal[i + j] = words[j];
        }
        i += n;
    }
    for (frame = 0; frame < 3; ++frame)
        for (i = 0; i < 128; i += 16)
        {
            for (j = 0; j < 4; ++j)
                words[j] = icon[i / 4 + j];
            for (j = 0; j < 4; ++j)
                card_files.header.icon[frame][i / 4 + j] = words[j];
        }
    XASSERT(data && capacity >= CARD_HEADER_SIZE);
    for (i = 0; i < CARD_HEADER_SIZE; i += 16)
    {
        for (j = 0; j < 4; ++j)
            words[j] = card_files.header.words[i / 4 + j];
        for (j = 0; j < 4; ++j)
            memcpy(data + i + 4 * j, &words[j], sizeof(words[j]));
    }
    build_path(slot, name, name_capacity);
    fd = open_file_dummy(card_files.path, ((uint32)(sint32)card_files.header.blocks << 16) | 0x200);
    if (fd == -1)
        return -1;
    close_file_dummy(fd);
    fd = open_file_dummy(card_files.path, 2);
    if (fd == -1)
        return -2;
    if (localization.text[LOCAL_TEXT_CARD_SAVING])
        card_draw_pending_notice(notice_context);
    XASSERT(capacity >= CARD_FILE_SIZE);
    written = write_file_dummy(fd, data, CARD_FILE_SIZE);
    close_file_dummy(fd);
    return written == CARD_FILE_SIZE ? 1 : -3;
}

GDB_CALL sint32 card_file_exists(uint32 slot, const char *name, uint32 capacity)
{
    FUNCTION_MARKER(0x8006b1c8u, "GAME.EXE");
    sint32 fd;
    build_path(slot, name, capacity);
    fd = open_file_dummy(card_files.path, 1);
    if (fd == -1)
        return 0;
    close_file_dummy(fd);
    return 1;
}

GDB_CALL sint32 card_format(uint32 slot)
{
    FUNCTION_MARKER(0x8006b6ccu, "GAME.EXE");
    snprintf(card_files.path, sizeof(card_files.path), "bu%2.2x:", slot & 255);
    /* WIP B0:41 format: absent card, no storage mutation. */
    ++platform_boundary_calls[0];
    return 0;
}

GDB_CALL sint32 card_delete_file(uint32 slot, const char *name, uint32 capacity)
{
    FUNCTION_MARKER(0x8006b710u, "GAME.EXE");
    if (!card_file_exists(slot, name, capacity))
        return 1;
    build_path(slot, name, capacity);
    return card_backend_erase(card_files.path);
}

GDB_CALL sint32 card_read_save(uint32 slot, const char *name, uint32 name_capacity, uint8 *dst, uint32 capacity)
{
    FUNCTION_MARKER(0x8006b77cu, "GAME.EXE");
    sint32 fd, count;
    build_path(slot, name, name_capacity);
    fd = open_file_dummy(card_files.path, 1);
    if (fd == -1)
        return 0;
    XASSERT(dst && capacity >= CARD_FILE_SIZE);
    count = card_backend_read(fd, dst, CARD_FILE_SIZE);
    close_file_dummy(fd);
    return count == CARD_FILE_SIZE;
}

GDB_CALL sint32 card_read_save_info(void)
{
    FUNCTION_MARKER(0x8006b9d0u, "GAME.EXE");
    uint8 stage, player_count, type0, type1;
    card_storage.preview.stage = -1;
    if (!card_read_save(0, card_save_name, CARD_SAVE_NAME_SIZE, card_files.save.bytes, CARD_FILE_SIZE))
    {
        card_storage.preview.stage = -2;
        return -2;
    }
    stage = card_files.save.data.stage;
    player_count = card_files.save.data.player_count;
    type0 = card_files.save.data.character_type[0];
    type1 = card_files.save.data.character_type[1];
    card_storage.preview.stage = stage;
    card_storage.preview.player_count = player_count;
    card_storage.preview.character_type[0] = type0;
    card_storage.preview.character_type[1] = type1;
    return stage;
}

static DIRENTRY *next_file_dummy(DIRENTRY *entry)
{
    ++platform_boundary_calls[0];
    return NULL;
}

GDB_CALL sint32 card_scan_directory(uint32 slot, DIRENTRY *entries, uint32 capacity)
{
    FUNCTION_MARKER(0x8006bf74u, "GAME.EXE");
    uint8 saved[128];
    uint32 count = 0;
    DIRENTRY *entry;
    char *buffer = card_files.scan_path;
    memcpy(saved, buffer, 128);
    snprintf(buffer, sizeof(card_files.scan_path), "bu%2.2x:", slot);
    strcat(buffer, file_pattern);
    XASSERT(entries && capacity);
    entry = entries;
    if (card_backend_first_file(buffer, entry) == entry)
    {
        count = 1;
        do
        {
            XASSERT(count < capacity);
            entry = entries + count;
            if (next_file_dummy(entry) != entry)
                break;
            ++count;
        } while (1);
    }
    memcpy(buffer, saved, 128);
    return count;
}

GDB_CALL sint32 card_list_files(uint32 slot, CARD_DIRECTORY *directory)
{
    FUNCTION_MARKER(0x8006bffcu, "GAME.EXE");
    sint32 count, fd, bytes;
    uint32 i, blocks;
    uint8 saved[264];
    char *tmp = card_files.decoded_title;
    card_storage.free_blocks = CARD_BLOCK_COUNT;
    XASSERT(directory);
    count = card_scan_directory(slot, directory->entry, CARD_DIRECTORY_ENTRY_COUNT);
    if (count <= 0)
        return count;
    XASSERT(count <= CARD_DIRECTORY_FILE_COUNT);
    memcpy(saved, tmp, sizeof(saved));
    for (i = 0; (sint32)i < count; ++i)
    {
        DIRENTRY *entry = directory->entry + i;
        char *name = directory->titles + i * CARD_DIRECTORY_TITLE_STRIDE;
        /* Retain filename extension into the SDK attr/size prefix, not host links */
        build_path(slot, (const char *)entry, CARD_DIRECTORY_NAME_SPAN);
        blocks = entry->size + 1u;
        blocks = (sint32)blocks / 8192;
        card_storage.free_blocks = card_storage.free_blocks - blocks;
        fd = open_file_dummy(card_files.path, 1);
        if (fd == -1)
        {
            memcpy(tmp, saved, sizeof(saved));
            return 0;
        }
        bytes = card_backend_read(fd, card_files.header.bytes, CARD_HEADER_SIZE);
        /* The original failure edge does not close an already-open descriptor. */
        if (bytes != CARD_HEADER_SIZE)
        {
            memcpy(tmp, saved, sizeof(saved));
            return 0;
        }
        if (card_files.header.bytes[4] < 128)
        {
            const char *src = (const char *)card_files.header.bytes + 4;
            const char *end = memchr(src, 0, CARD_HEADER_SIZE - 4);
            XASSERT(end && end - src < sizeof(card_files.decoded_title));
            memcpy(tmp, src, end - src + 1);
        }
        else
            card_decode_title(tmp, sizeof(card_files.decoded_title), card_files.header.code_units + 2, CARD_HEADER_SIZE / 2 - 2);
        strcpy(name, tmp);
        close_file_dummy(fd);
    }
    memcpy(tmp, saved, sizeof(saved));
    return count;
}

GDB_CALL uint32 card_encode_character(uint32 character)
{
    FUNCTION_MARKER(0x8006bdf4u, "GAME.EXE");
    uint32 ch = character & 255, grp = 0, punctuation = 0, val;
    if (((ch - 32) & 255) < 16)
        punctuation = 1;
    else if (((ch - 48) & 255) < 10)
        grp = 0;
    else if (((ch - 58) & 255) < 7)
        punctuation = 11;
    else if (((ch - 65) & 255) < 26)
        grp = 1;
    else if (((ch - 91) & 255) < 6)
        punctuation = 37;
    else if (((ch - 97) & 255) < 26)
        grp = 2;
    else if (((ch - 123) & 255) < 4)
        punctuation = 63;
    else
        return 0;
    if (punctuation)
        val = punctuation_code[ch - punctuation - 31];
    else
        val = character_range[grp].code + ch - character_range[grp].first;
    return ((val & 255) << 8) | ((val >> 8) & 255);
}

GDB_CALL sint32 card_encode_title(uint16 *dst, uint32 capacity, const char *src, uint32 src_capacity)
{
    FUNCTION_MARKER(0x8006bf14u, "GAME.EXE");
    uint32 i = 0;
    for (;;)
    {
        uint32 ch;
        XASSERT(src && i < src_capacity);
        ch = (uint8)src[i];
        if (!ch)
            break;
        XASSERT(dst && i < capacity);
        dst[i++] = card_encode_character(ch);
    }
    XASSERT(dst && i < capacity);
    dst[i] = 0;
    return 0;
}

GDB_CALL sint32 card_decode_character(uint32 character)
{
    FUNCTION_MARKER(0x8006c178u, "GAME.EXE");
    uint32 ch = character & 65535, high = ch >> 8;
    /* Original decision tree omits 5282 ('3'); do not fill that hole. */
    if ((ch & 255) == 0x82)
    {
        if (high >= 0x4f && high <= 0x58 && high != 0x52)
            return high - 0x4f + '0';
        if (high >= 0x60 && high <= 0x79)
            return high - 0x60 + 'A';
        if (high >= 0x81 && high <= 0x9a)
            return high - 0x81 + 'a';
    }
    if (ch == 0x8144)
        return '.';
    if (ch == 0x817c)
        return '-';
    return ' ';
}

GDB_CALL void card_decode_title(char *dst, uint32 capacity, const uint16 *src, uint32 count)
{
    FUNCTION_MARKER(0x8006c6e8u, "GAME.EXE");
    uint32 i = 0, ch;
    for (;;)
    {
        XASSERT(src && i < count);
        ch = src[i];
        XASSERT(dst && i < capacity);
        if (!ch)
            break;
        dst[i++] = (uint8)card_decode_character(ch);
    }
    dst[i] = 0;
}

/* WIP TestEvent: consume synthetic no-media events from the dummy backend. */
static sint32 test_event_dummy(uint32 handle)
{
    return card_backend_test_event(handle);
}

static sint32 test_grp(const uint32 *events)
{
    uint32 i;
    sint32 result = 0;
    for (i = 0; i < 4; ++i)
        result = test_event_dummy(events[i]);
    return result;
}

GDB_CALL sint32 card_clear_io_events(void)
{
    FUNCTION_MARKER(0x8006b894u, "GAME.EXE");
    return test_grp(card_storage.event[0]);
}

GDB_CALL sint32 card_clear_software_events(void)
{
    FUNCTION_MARKER(0x8006b950u, "GAME.EXE");
    return test_grp(card_storage.event[1]);
}

static sint32 wait_grp(const uint32 *events)
{
    uint32 i;
    for (;;)
        for (i = 0; i < 4; ++i)
            if (test_event_dummy(events[i]) == 1)
                return i;
}

GDB_CALL sint32 card_wait_io_event(void)
{
    FUNCTION_MARKER(0x8006b820u, "GAME.EXE");
    return wait_grp(card_storage.event[0]);
}

GDB_CALL sint32 card_wait_software_event(void)
{
    FUNCTION_MARKER(0x8006b8dcu, "GAME.EXE");
    return wait_grp(card_storage.event[1]);
}

static void complete(uint32 result)
{
    card_storage.completion = result;
    card_storage.phase = 3;
}

GDB_CALL sint32 card_update(void)
{
    FUNCTION_MARKER(0x8006bb44u, "GAME.EXE");
    uint32 state = card_storage.phase;
    sint32 result;
    if (!state)
    {
        card_clear_io_events();
        card_backend_request_no_media(0);
        card_storage.phase = 1;
        return 1;
    }
    if (state == 1)
    {
        if (test_event_dummy(card_storage.event[0][0]) == 1)
        {
            card_clear_io_events();
            card_backend_request_no_media(0);
            card_storage.phase = 4;
            return 4;
        }
        if (test_event_dummy(card_storage.event[0][1]) == 1)
        {
            complete(3);
            return 3;
        }
        if (test_event_dummy(card_storage.event[0][2]) == 1)
        {
            complete(0);
            return 3;
        }
        result = test_event_dummy(card_storage.event[0][3]);
        if (result == 1)
        {
            card_clear_software_events();
            card_backend_request_no_media(1);
            card_wait_software_event();
            card_clear_io_events();
            card_backend_request_no_media(0);
            card_storage.phase = 7;
            return 7;
        }
        return result;
    }
    if (state == 3)
    {
        card_storage.phase = 0;
        return 4;
    }
    if (state != 4 && state != 7)
        return 7;
    if (test_event_dummy(card_storage.event[0][0]) == 1)
    {
        complete(1);
        if (state == 7)
        {
            card_storage.preview.stage = -1;
            return -1;
        }
        if (card_storage.preview.stage == -1)
            return card_read_save_info();
        return -1;
    }
    result = test_event_dummy(card_storage.event[0][1]);
    if (result == 1)
    {
        complete(3);
        return result;
    }
    result = test_event_dummy(card_storage.event[0][2]);
    if (result == 1)
    {
        complete(0);
        return result;
    }
    result = test_event_dummy(card_storage.event[0][3]);
    if (result == 1)
    {
        complete(2);
        if (state == 7)
            return card_read_save_info();
    }
    return 2;
}

GDB_CALL sint32 card_wait(void)
{
    FUNCTION_MARKER(0x8006b998u, "GAME.EXE");
    do
    {
        disp_wait_vblank();
        card_update();
    } while (card_storage.phase);
    return 0;
}

GDB_CALL uint32 card_wait_status(uint32 *output)
{
    FUNCTION_MARKER(0x8006bdbcu, "GAME.EXE");
    card_update();
    card_wait();
    return card_poll_status(output);
}

GDB_CALL uint32 card_poll_status(uint32 *output)
{
    FUNCTION_MARKER(0x8006bd9cu, "GAME.EXE");
    uint32 val = card_storage.completion;
    if (!val)
        return 0;
    XASSERT(output);
    *output = val;
    return 1;
}

/* Temporary readonly export for historical binary diagnostics */
