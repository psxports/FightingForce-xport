#ifndef RESOURCES_H
#define RESOURCES_H

#include "xport.h"
#include "psx.h"

enum
{
    RESOURCE_FILE_COUNT = 211,
    RESOURCE_PATH_COUNT = 192
};

enum
{
    RESOURCE_NAME_SIZE = 16
};

/* Serialized model prefix; wire words retain original relocation effects, not native links */
typedef struct MODEL_BODY_HEADER
{
    uint32 vertices_wire;
    sint32 vertex_count;
    uint32 faces_wire;
    sint32 face_count;
} MODEL_BODY_HEADER;

typedef struct MODEL_RIG_HEADER
{
    MODEL_BODY_HEADER mesh;
    uint32 copy_size_or_end;
    uint32 bone_count;
    uint32 commands_offset;
    uint32 part_count;
    uint32 part_vertex_count[];
} MODEL_RIG_HEADER;

enum
{
    MODEL_TYPE_COUNT = 920
};

/* Serialized 12-byte face; some primitive kinds interpret the last word as color */
typedef union MODEL_FACE_RECORD
{
    struct
    {
        uint16 vertex_offset[4];
        uint16 flags;
        uint16 tex_offset;
    };

    uint32 words[3];
} MODEL_FACE_RECORD;

typedef struct MODEL_REFERENCE
{
    MODEL_BODY_HEADER *header;
    uint32 byte_len;
    /* Borrowed links share the resource arena lifetime, including aliased trail writes */
    SVECTOR *vertices;
    uint32 vertex_byte_len;
    MODEL_FACE_RECORD *faces;
    uint32 face_byte_len;
} MODEL_REFERENCE;

GDB_CALL sint32 model_vertex_dir(MODEL_REFERENCE mesh, uint32 v0, uint32 v1);

extern MODEL_REFERENCE model_refs[MODEL_TYPE_COUNT];
extern uint32 model_height[MODEL_TYPE_COUNT];

typedef struct RESOURCE_FILE
{
    uint8 *data;
    uint32 capacity;
    uint32 byte_len;
} RESOURCE_FILE;

/* Stable serialized asset storage shared by overlapping WAD and ACH loads */
enum
{
    RESOURCE_STORAGE_ORIGIN = 0xc0d68,
    RESOURCE_STORAGE_CAPACITY = 0x200000 - RESOURCE_STORAGE_ORIGIN
};

typedef union RESOURCE_STORAGE
{
    uint32 alignment;
    uint8 data[RESOURCE_STORAGE_CAPACITY];
} RESOURCE_STORAGE;

extern RESOURCE_STORAGE resource_storage;

/* Native allocation position plus its original serialized relocation label */
typedef struct RESOURCE_CURSOR
{
    uint8 *data;
    uint32 available;
    uint32 context;
} RESOURCE_CURSOR;

enum
{
    RESOURCE_SCRATCH_OFFSET = 0x116000 - RESOURCE_STORAGE_ORIGIN
};

typedef struct RESOURCE_ARENAS
{
    RESOURCE_CURSOR scratch;
    RESOURCE_CURSOR persistent;
    RESOURCE_CURSOR text_end;
} RESOURCE_ARENAS;

extern RESOURCE_ARENAS resource_arenas;
extern const RESOURCE_ARENAS resource_arena_defaults;

typedef struct RESOURCE_IO
{
    uint32 result_size;
    uint32 file_size;
} RESOURCE_IO;

/* Serialized WAD directory; numeric fields are big-endian */
typedef struct RESOURCE_ENTRY
{
    uint8 name[12];
    uint32 size_be;
    uint32 offset_be;
} RESOURCE_ENTRY;

typedef struct RESOURCE_DIRECTORY
{
    uint32 count_be;
    RESOURCE_ENTRY entry[];
} RESOURCE_DIRECTORY;

typedef struct RESOURCE_VIEW
{
    uint8 *data;
    uint32 byte_len;
    uint8 *storage;
    uint32 storage_len;
    /* Serialized load label plus entry offset, never a payload pointer */
    uint32 context_address;
} RESOURCE_VIEW;

typedef struct RESOURCE_ARCHIVE
{
    RESOURCE_DIRECTORY *dir;
    uint32 byte_len;
    /* Serialized model relocation base, never an archive payload pointer */
    uint32 relocation_base;
} RESOURCE_ARCHIVE;

extern RESOURCE_IO resource_io;
extern RESOURCE_ARCHIVE resource_archive;
extern const char *resource_file_names[RESOURCE_FILE_COUNT];
extern const char *const resource_paths[RESOURCE_PATH_COUNT];

GDB_CALL uint32 resource_copy_block(const uint8 *src, uint8 *dst);
GDB_CALL uint32 resource_copy_words(uint8 *dst, const uint8 *src, uint32 count);
GDB_CALL uint32 resource_swap32(uint32 val);
/* File owners grow within nonzero native_capacity; zero retains the existing file limit */
GDB_CALL uint32 resource_load(uint32 idx, uint32 relocation_base, void *native_dst, uint32 native_capacity, RESOURCE_FILE *file, const RESOURCE_CURSOR *arena);
GDB_CALL RESOURCE_VIEW resource_find(const char *name);
GDB_CALL sint32 resource_exists(const char *name);
GDB_CALL RESOURCE_VIEW resource_find_quiet(const char *name);
GDB_CALL uint32 resource_load_model(const char *name, uint32 type, uint32 tex_grp);
GDB_CALL uint32 resource_load_character(const char *name, uint32 type, uint32 grp);
GDB_CALL uint32 resource_load_animations(const char *name, sint32 type);
GDB_CALL uint32 resource_load_frontend_models(void);

#endif
