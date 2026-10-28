#ifndef CARD_STORAGE_H
#define CARD_STORAGE_H

#include "xport.h"
#include "psx.h"

enum
{
    CARD_EVENT_GROUP_COUNT = 2,
    CARD_EVENT_COUNT = 4
};

typedef struct CARD_SAVE_INFO
{
    sint8 stage;
    sint8 player_count;
    sint8 character_type[2];
} CARD_SAVE_INFO;

enum
{
    CARD_HEADER_SIZE = 512,
    CARD_DATA_SIZE = 164,
    CARD_FILE_SIZE = 8192,
    CARD_HIGH_SCORE_COUNT = 10
};

typedef union CARD_HEADER
{
    struct
    {
        uint8 signature[2];
        uint8 icon_flags;
        sint8 blocks;
        uint16 title[32];
        uint8 reserved[28];
        uint32 pal[8];
        uint32 icon[3][32];
    };

    uint8 bytes[CARD_HEADER_SIZE];
    uint16 code_units[CARD_HEADER_SIZE / 2];
    uint32 words[CARD_HEADER_SIZE / 4];
} CARD_HEADER;

typedef struct CARD_SCORE
{
    uint32 score;
    uint32 name_bits;
    uint32 flags;
} CARD_SCORE;

typedef union CARD_SAVE_DATA
{
    struct
    {
        uint8 stage;
        uint8 player_count;
        uint8 character_type[2];
        uint16 health[2];
        uint32 score[2];
        uint32 continue_threshold[2];
        uint32 continues;
        uint8 route_choice;
        uint8 reserved0[3];
        uint32 sound;
        uint8 reserved1;
        uint8 difficulty;
        uint8 pad_cfg[2];
        uint8 scr_offset[2];
        uint8 reserved2[2];
        CARD_SCORE high_score[CARD_HIGH_SCORE_COUNT];
    };

    uint8 bytes[CARD_DATA_SIZE];
} CARD_SAVE_DATA;

/* Serialized file objects, not native gameplay structure dumps */
typedef union CARD_FILE
{
    struct
    {
        CARD_HEADER header;
        CARD_SAVE_DATA data;
        uint8 tail[CARD_FILE_SIZE - CARD_HEADER_SIZE - CARD_DATA_SIZE];
    };

    uint8 bytes[CARD_FILE_SIZE];
} CARD_FILE;

enum
{
    CARD_BLOCK_COUNT = 15,
    CARD_DIRECTORY_FILE_COUNT = CARD_BLOCK_COUNT,
    CARD_DIRECTORY_ENTRY_COUNT = CARD_DIRECTORY_FILE_COUNT + 1,
    CARD_DIRECTORY_TITLE_STRIDE = 64,
    CARD_DIRECTORY_TITLE_CAPACITY = 264,
    CARD_DIRECTORY_TITLE_POOL_SIZE = (CARD_DIRECTORY_FILE_COUNT - 1) * CARD_DIRECTORY_TITLE_STRIDE + CARD_DIRECTORY_TITLE_CAPACITY,
    CARD_DIRECTORY_NAME_SPAN = offsetof(DIRENTRY, size) + sizeof(sint32),
    CARD_SAVE_NAME_SIZE = 13,
    CARD_SAVE_TITLE_SIZE = 14
};

typedef struct CARD_DIRECTORY
{
    /* The extra SDK output receives the terminal nextfile probe */
    DIRENTRY entry[CARD_DIRECTORY_ENTRY_COUNT];
    /* Preserve original overlapping copies at a 64-byte title stride */
    char titles[CARD_DIRECTORY_TITLE_POOL_SIZE];
} CARD_DIRECTORY;

extern const char card_save_name[CARD_SAVE_NAME_SIZE];
extern const char card_save_title[CARD_SAVE_TITLE_SIZE];

typedef struct CARD_FILES
{
    CARD_HEADER header;
    CARD_FILE save;
    CARD_FILE verify;
    char decoded_title[264];
    char path[160];
    char scan_path[128];
    CARD_DIRECTORY directory;
} CARD_FILES;

extern CARD_FILES card_files;

typedef struct CARD_STORAGE
{
    uint32 event[CARD_EVENT_GROUP_COUNT][CARD_EVENT_COUNT];
    uint32 phase;
    uint32 completion;
    uint32 free_blocks;
    CARD_SAVE_INFO preview;
    uint32 status;
    uint32 prev_status;
} CARD_STORAGE;

extern CARD_STORAGE card_storage;
extern const CARD_STORAGE card_storage_defaults;

GDB_CALL sint32 card_init_events(void);
/* Notice context retains original saved S3, not a native filename pointer */
GDB_CALL sint32 card_write_save(uint32 slot, const char *name, uint32 name_capacity, const char *title, uint32 title_capacity, uint32 notice_context, uint8 *data, uint32 capacity);
GDB_CALL sint32 card_file_exists(uint32 slot, const char *name, uint32 capacity);
GDB_CALL sint32 card_format(uint32 slot);
GDB_CALL sint32 card_delete_file(uint32 slot, const char *name, uint32 capacity);
GDB_CALL sint32 card_read_save(uint32 slot, const char *name, uint32 name_capacity, uint8 *dst, uint32 capacity);
GDB_CALL sint32 card_read_save_info(void);
GDB_CALL sint32 card_scan_directory(uint32 slot, DIRENTRY *entries, uint32 capacity);
GDB_CALL sint32 card_list_files(uint32 slot, CARD_DIRECTORY *directory);
GDB_CALL uint32 card_encode_character(uint32 character);
GDB_CALL sint32 card_encode_title(uint16 *dst, uint32 capacity, const char *src, uint32 src_capacity);
GDB_CALL sint32 card_decode_character(uint32 character);
GDB_CALL void card_decode_title(char *dst, uint32 capacity, const uint16 *src, uint32 count);
GDB_CALL sint32 card_clear_io_events(void);
GDB_CALL sint32 card_clear_software_events(void);
GDB_CALL sint32 card_wait_io_event(void);
GDB_CALL sint32 card_wait_software_event(void);
GDB_CALL sint32 card_update(void);
GDB_CALL sint32 card_wait(void);
GDB_CALL uint32 card_wait_status(uint32 *output);
GDB_CALL uint32 card_poll_status(uint32 *output);

#endif
