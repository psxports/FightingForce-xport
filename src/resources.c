#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "game.h"
#include "platform.h"
#include "psx_gpu.h"

RESOURCE_IO resource_io;
RESOURCE_ARENAS resource_arenas;
RESOURCE_STORAGE resource_storage;
const RESOURCE_ARENAS resource_arena_defaults = {
    .text_end = {resource_storage.data, RESOURCE_STORAGE_CAPACITY, 0x800c0d68},
};
MODEL_REFERENCE model_refs[MODEL_TYPE_COUNT];
uint32 model_height[MODEL_TYPE_COUNT];
RESOURCE_ARCHIVE resource_archive;

_Static_assert(sizeof(RESOURCE_ENTRY) == 20 && offsetof(RESOURCE_ENTRY, size_be) == 12 && offsetof(RESOURCE_ENTRY, offset_be) == 16 && offsetof(RESOURCE_DIRECTORY, entry) == 4, "WAD directory wire layout");
const char *resource_file_names[RESOURCE_FILE_COUNT];

const char *const resource_paths[RESOURCE_PATH_COUNT] = {
    "\\FRONT\\LOADING.WAD;1",     "\\FRONT\\TITLE.WAD;1",       "\\FRONT\\FRONT.WAD;1",       "\\FRONT\\LANGUAGE.WAD;1",    "\\FRONT\\MEMCARD.WAD;1",     "\\FRONT\\CREDITS.WAD;1",     "\\FRONT\\ENGLISH.WAD;1",     "\\FRONT\\FRENCH.WAD;1",      "\\FRONT\\GERMAN.WAD;1",      "\\STGFILES\\STAGE11.WAD;1",  "\\STGFILES\\STAGE12.WAD;1",  "\\STGFILES\\STAGE13.WAD;1",  "\\STGFILES\\STAGE14.WAD;1",  "\\STGFILES\\STAGE15.WAD;1",  "\\STGFILES\\STAGE21.WAD;1",  "\\STGFILES\\STAGE31.WAD;1",  "\\STGFILES\\STAGE32A.WAD;1", "\\STGFILES\\STAGE32B.WAD;1", "\\STGFILES\\STAGE33.WAD;1",  "\\STGFILES\\STAGE33A.WAD;1", "\\STGFILES\\STAGE41.WAD;1",  "\\STGFILES\\STAGE42.WAD;1",  "\\STGFILES\\STAGE51.WAD;1",  "\\STGFILES\\STAGE61.WAD;1",  "\\STGFILES\\STAGE62.WAD;1",  "\\STGFILES\\STAGE63.WAD;1",  "\\STGFILES\\STAGE64.WAD;1",  "\\STGFILES\\STAGE65.WAD;1",  "\\STGFILES\\STAGE66.WAD;1",  "\\STGFILES\\STAGE67.WAD;1",  "\\STGFILES\\STAGE71.WAD;1",  "\\STGFILES\\STAGE72.WAD;1",
    "\\STGFILES\\STAGE73.WAD;1",  "\\CUTFILES\\STAGEC1.WAD;1",  "\\CUTFILES\\STAGEC3.WAD;1",  "\\CUTFILES\\STAGEC5.WAD;1",  "\\CUTFILES\\STAGEC6.WAD;1",  "\\CUTFILES\\STAGEC7.WAD;1",  "\\CUTFILES\\STAGEC8.WAD;1",  "\\CUTFILES\\STAGEC9.WAD;1",  "\\CUTFILES\\STAGEC11.WAD;1", "\\CUTFILES\\STAGEC12.WAD;1", "\\MODFILES\\MODELS11.WAD;1", "\\MODFILES\\MODELS12.WAD;1", "\\MODFILES\\MODELS13.WAD;1", "\\MODFILES\\MODELS14.WAD;1", "\\MODFILES\\MODELS15.WAD;1", "\\MODFILES\\MODELS21.WAD;1", "\\MODFILES\\MODELS31.WAD;1", "\\MODFILES\\MODELS32.WAD;1", "\\MODFILES\\MODELS33.WAD;1", "\\MODFILES\\MODEL33A.WAD;1", "\\MODFILES\\MODELS41.WAD;1", "\\MODFILES\\MODELS51.WAD;1", "\\MODFILES\\MODELS61.WAD;1", "\\MODFILES\\MODELS62.WAD;1", "\\MODFILES\\MODELS63.WAD;1", "\\MODFILES\\MODELS64.WAD;1", "\\MODFILES\\MODELS65.WAD;1", "\\MODFILES\\MODELS66.WAD;1", "\\MODFILES\\MODELS67.WAD;1", "\\MODFILES\\MODELS71.WAD;1", "\\MODFILES\\MODELS72.WAD;1", "\\MODFILES\\MODELS73.WAD;1",
    "\\CUTFILES\\MODELSC1.WAD;1", "\\CUTFILES\\MODELSC3.WAD;1", "\\CUTFILES\\MODELSC5.WAD;1", "\\CUTFILES\\MODELSC6.WAD;1", "\\CUTFILES\\MODELSC7.WAD;1", "\\CUTFILES\\MODELSC8.WAD;1", "\\CUTFILES\\MODELSC9.WAD;1", "\\CUTFILES\\MODELC11.WAD;1", "\\CUTFILES\\MODELC12.WAD;1", "\\CHAR\\HAWK.WAD;1",         "\\CHAR\\MACE.WAD;1",         "\\CHAR\\SMASHER.WAD;1",      "\\CHAR\\ALANA.WAD;1",        "\\CHAR\\HAWK2.WAD;1",        "\\CHAR\\JETPAC1.WAD;1",      "\\SOUND\\SAMPS11.WAD;1",     "\\SOUND\\SAMPS12.WAD;1",     "\\SOUND\\SAMPS13.WAD;1",     "\\SOUND\\SAMPS14.WAD;1",     "\\SOUND\\SAMPS21.WAD;1",     "\\SOUND\\SAMPS31.WAD;1",     "\\SOUND\\SAMPS32.WAD;1",     "\\SOUND\\SAMPS33.WAD;1",     "\\SOUND\\SAMPS33A.WAD;1",    "\\SOUND\\SAMPS41.WAD;1",     "\\SOUND\\SAMPS42.WAD;1",     "\\SOUND\\SAMPS51.WAD;1",     "\\SOUND\\SAMPS61.WAD;1",     "\\SOUND\\SAMPS62.WAD;1",     "\\SOUND\\SAMPS63.WAD;1",     "\\SOUND\\SAMPS64.WAD;1",     "\\SOUND\\SAMPS65.WAD;1",
    "\\SOUND\\SAMPS66.WAD;1",     "\\SOUND\\SAMPS67.WAD;1",     "\\SOUND\\SAMPS71.WAD;1",     "\\SOUND\\SAMPS72.WAD;1",     "\\SOUND\\SAMPS73.WAD;1",     "\\FRONT\\WARNING.WAD;1",     "\\STGFILES\\STAGE22.WAD;1",  "\\MODFILES\\MODELS22.WAD;1", "\\SOUND\\SAMPS22.WAD;1",     "\\STGFILES\\STAGEFE.WAD;1",  "\\MODFILES\\MODELSFE.WAD;1", "\\SOUND\\SAMPSFE.WAD;1",     "\\CHAR\\CHARS11.WAD;1",      "\\CHAR\\CHARS12.WAD;1",      "\\CHAR\\CHARS13.WAD;1",      "\\CHAR\\CHARS14.WAD;1",      "\\CHAR\\CHARS15.WAD;1",      "\\CHAR\\CHARS21.WAD;1",      "\\CHAR\\CHARS22.WAD;1",      "\\CHAR\\CHARS31.WAD;1",      "\\CHAR\\CHARS32A.WAD;1",     "\\CHAR\\CHARS32B.WAD;1",     "\\CHAR\\CHARS33.WAD;1",      "\\CHAR\\CHARS33A.WAD;1",     "\\CHAR\\CHARS41.WAD;1",      "\\CHAR\\CHARS42.WAD;1",      "\\CHAR\\CHARS51.WAD;1",      "\\CHAR\\CHARS61.WAD;1",      "\\CHAR\\CHARS62.WAD;1",      "\\CHAR\\CHARS63.WAD;1",      "\\CHAR\\CHARS64.WAD;1",      "\\CHAR\\CHARS65.WAD;1",
    "\\CHAR\\CHARS66.WAD;1",      "\\CHAR\\CHARS67.WAD;1",      "\\CHAR\\CHARS71.WAD;1",      "\\CHAR\\CHARS72.WAD;1",      "\\CHAR\\CHARS73.WAD;1",      "\\CHAR\\GENMALE.WAD;1",      "\\CHAR\\GENFMALE.WAD;1",     "\\CHAR\\GENBIGMA.WAD;1",     "\\DEMOS\\000.DEM;1",         "\\DEMOS\\001.DEM;1",         "\\DEMOS\\002.DEM;1",         "\\DEMOS\\003.DEM;1",         "\\DEMOS\\004.DEM;1",         "\\DEMOS\\005.DEM;1",         "\\DEMOS\\006.DEM;1",         "\\DEMOS\\007.DEM;1",         "\\DEMOS\\008.DEM;1",         "\\DEMOS\\009.DEM;1",         "\\AID\\LEV11.MVS;1",         "\\AID\\LEV12.MVS;1",         "\\AID\\LEV13.MVS;1",         "\\AID\\LEV14.MVS;1",         "\\AID\\LEV15.MVS;1",         "\\AID\\LEV21.MVS;1",         "\\AID\\LEV22.MVS;1",         "\\AID\\LEV31.MVS;1",         "\\AID\\LEV32A.MVS;1",        "\\AID\\LEV33.MVS;1",         "\\AID\\LEV33A.MVS;1",        "\\AID\\LEV41.MVS;1",         "\\AID\\LEV42.MVS;1",         "\\AID\\LEV51.MVS;1",
    "\\AID\\LEV61.MVS;1",         "\\AID\\LEV62.MVS;1",         "\\AID\\LEV63.MVS;1",         "\\AID\\LEV64.MVS;1",         "\\AID\\LEV65.MVS;1",         "\\AID\\LEV66.MVS;1",         "\\AID\\LEV71.MVS;1",         "\\AID\\LEV72.MVS;1",         "\\AID\\LEV73.MVS;1",         "\\AII\\LEV11.INT;1",         "\\AII\\LEV12.INT;1",         "\\AII\\LEV13.INT;1",         "\\AII\\LEV14.INT;1",         "\\AII\\LEV15.INT;1",         "\\AII\\LEV21.INT;1",         "\\AII\\LEV22.INT;1",         "\\AII\\LEV31.INT;1",         "\\AII\\LEV32A.INT;1",        "\\AII\\LEV33.INT;1",         "\\AII\\LEV33A.INT;1",        "\\AII\\LEV41.INT;1",         "\\AII\\LEV42.INT;1",         "\\AII\\LEV51.INT;1",         "\\AII\\LEV61.INT;1",         "\\AII\\LEV62.INT;1",         "\\AII\\LEV63.INT;1",         "\\AII\\LEV64.INT;1",         "\\AII\\LEV65.INT;1",         "\\AII\\LEV66.INT;1",         "\\AII\\LEV71.INT;1",         "\\AII\\LEV72.INT;1",         "\\AII\\LEV73.INT;1",
};

/* Read original disc files below DATA */
static sint32 read_disc_file(const char *disc_name, void *dst, uint32 capacity, RESOURCE_FILE *file)
{
    char extracted[260];
    char path[sizeof(extracted) + 6];
    uint32 size;
    FILE *f;
    size_t got, trailing;
    uint8 extra;
    ++platform_boundary_calls[1];
    {
        /* Read only an extracted ISO file below DATA */
        const char *name = disc_name;
        size_t n = 0, j;
        long len;
        if (*name == '\\' || *name == '/')
            ++name;
        while (*name && *name != ';' && n + 1 < sizeof(extracted))
        {
            unsigned char c = (unsigned char)*name++;
            if (c == ':' || c < 32)
                return 0;
            extracted[n++] = c == '\\' ? '/' : (char)c;
        }
        if (*name && strcmp(name, ";1"))
            return 0;
        extracted[n] = 0;
        if (!n || extracted[0] == '/')
            return 0;
        for (j = 0; j < n; j++)
            if ((j == 0 || extracted[j - 1] == '/') && extracted[j] == '.' && (extracted[j + 1] == '/' || !extracted[j + 1] || (extracted[j + 1] == '.' && (extracted[j + 2] == '/' || !extracted[j + 2]))))
                return 0;
        if (strncmp(extracted, "MODFILES/", 9) && strncmp(extracted, "STGFILES/", 9) && strncmp(extracted, "CUTFILES/", 9) && strncmp(extracted, "CHAR/", 5) && strncmp(extracted, "SOUND/", 6) && strncmp(extracted, "FRONT/", 6) && strncmp(extracted, "AID/", 4) && strncmp(extracted, "AII/", 4) && strncmp(extracted, "DEMOS/", 6))
            return 0;
        if (n + sizeof("DATA/") > sizeof(path))
            return 0;
        memcpy(path, "DATA/", sizeof("DATA/") - 1);
        memcpy(path + sizeof("DATA/") - 1, extracted, n + 1);
        f = xport_fopen(path, "rb");
        if (!f)
            return 0;
        if (fseek(f, 0, SEEK_END) || (len = ftell(f)) < 0 || (unsigned long)len > capacity)
        {
            fclose(f);
            return 0;
        }
        size = len;
        if (fseek(f, 0, SEEK_SET))
        {
            fclose(f);
            return 0;
        }
        if (file)
        {
            if (!size)
            {
                fclose(f);
                return 0;
            }
            if (size > file->capacity)
            {
                uint8 *data = realloc(file->data, size);
                XASSERT(data);
                file->data = data;
                file->capacity = size;
            }
            dst = file->data;
        }
        got = fread(dst, 1, size, f);
        trailing = fread(&extra, 1, 1, f);
        fclose(f);
        if (got != size || trailing)
            return 0;
        if (file)
            file->byte_len = size;
        return size;
    }
}

static sint32 resource_disc_read(const char *name, void *dst, uint32 capacity, RESOURCE_FILE *file)
{
    FUNCTION_MARKER(0x80067554u, "GAME.EXE");
    sint32 size;
    /* Native synchronous transport retains the agreed physical-CD WIP boundary */
    size = read_disc_file(name, dst, capacity, file);
    XASSERT(size > 0);
    if (size <= 0)
    {
        resource_io.result_size = 0;
        return -1;
    }
    resource_io.result_size = size;
    return 0;
}

uint32 resource_load(uint32 idx, uint32 relocation_base, void *native_dst, uint32 native_capacity, RESOURCE_FILE *file, const RESOURCE_CURSOR *arena)
{
    FUNCTION_MARKER(0x80061448, "GAME.EXE");
    uint32 attempt, capacity;
    void *dst;
    XASSERT(idx < RESOURCE_FILE_COUNT && resource_file_names[idx]);
    if (arena)
    {
        uintptr_t ptr = (uintptr_t)arena->data, base = (uintptr_t)resource_storage.data;
        /* TODO: Investigate a load without a valid native allocation span */
        XASSERT(!file && !native_dst && !native_capacity && arena->data && !(ptr & 3) && ptr >= base && ptr - base < RESOURCE_STORAGE_CAPACITY && arena->available && arena->available <= RESOURCE_STORAGE_CAPACITY - (ptr - base));
        dst = arena->data;
        capacity = arena->available;
    }
    else if (file)
    {
        XASSERT(!native_dst && native_capacity <= 0x7fffffff && file->byte_len <= file->capacity && (!file->capacity || file->data) && (file->capacity || !file->data));
        dst = NULL;
        capacity = native_capacity ? native_capacity : 0x7fffffff;
    }
    else if (native_dst)
    {
        dst = native_dst;
        capacity = native_capacity;
    }
    else
    {
        /* TODO: Investigate a caller without a native file, buffer or allocation owner */

        XASSERT(0);
        return 0;
    }
    /* Preserve original context bits without treating native storage as a numeric address */
    resource_archive.relocation_base = relocation_base;
    for (attempt = 0; attempt < 5; attempt++)
    {
        sint32 size;
        resource_disc_read(resource_file_names[idx], dst, capacity, file);
        size = resource_io.result_size;
        if (size > 0)
        {
            resource_io.file_size = size;
            resource_archive.dir = file ? (RESOURCE_DIRECTORY *)file->data : dst;
            resource_archive.byte_len = size;
            return size;
        }
    }
    XASSERT(0);
    return 0;
}

uint32 resource_swap32(uint32 val)
{
    FUNCTION_MARKER(0x80011F20, "GAME.EXE");
    return (val >> 24) | (val << 24) | ((val >> 8) & 0xff00u) | ((val & 0xff00u) << 8);
}

/* Shared lookup tail of 61140 and 611F8; borrow the file without copying it */
static const RESOURCE_ENTRY *find_entry(const char *name)
{
    uint8 key[RESOURCE_NAME_SIZE] = {0};
    uint32 n = 0, i;
    sint32 count;
    XASSERT(resource_archive.dir && resource_archive.byte_len >= sizeof(uint32) && !((uintptr_t)resource_archive.dir & 3));
    count = (sint32)resource_swap32(resource_archive.dir->count_be);
    XASSERT(count <= 0 || (uint32)count <= (resource_archive.byte_len - sizeof(uint32)) / sizeof(RESOURCE_ENTRY));
    XASSERT(name);
    for (;;)
    {
        uint8 c;
        XASSERT(n < sizeof(key));
        c = name[n];
        key[n++] = c;
        if (!c)
            break;
    }
    for (i = 0; (sint32)i < count; i++)
        if (!memcmp(key, resource_archive.dir->entry[i].name, sizeof(resource_archive.dir->entry[i].name)))
            return &resource_archive.dir->entry[i];
    return NULL;
}

/* Original 611F8 lookup tail, also used without its loading-screen prefix */
RESOURCE_VIEW resource_find_quiet(const char *name)
{
    const RESOURCE_ENTRY *entry = find_entry(name);
    uint32 offset;
    RESOURCE_VIEW view;
    XASSERT(entry);
    resource_io.result_size = resource_swap32(entry->size_be);
    offset = resource_swap32(entry->offset_be);
    XASSERT(offset <= resource_archive.byte_len && resource_io.result_size <= resource_archive.byte_len - offset);
    view.storage = (uint8 *)resource_archive.dir;
    view.storage_len = resource_archive.byte_len;
    view.data = view.storage + offset;
    view.byte_len = resource_io.result_size;
    view.context_address = offset + resource_archive.relocation_base;
    return view;
}

sint32 resource_exists(const char *name)
{
    FUNCTION_MARKER(0x80061140, "GAME.EXE");
    const RESOURCE_ENTRY *entry = find_entry(name);
    /* TODO: Investigate a found entry whose original encoded result becomes zero */
#if defined(_DEBUG)
    sint32 encoded_result_valid = !entry || resource_swap32(resource_archive.relocation_base + entry->offset_be);
    XASSERT(encoded_result_valid);
#endif
    return entry != NULL;
}

RESOURCE_VIEW resource_find(const char *name)
{
    FUNCTION_MARKER(0x800611F8, "GAME.EXE");
    sint32 total = loading.total;
    loading.count++;
    if (total > 0)
    {
        POLY_G4 p;
        uint32 i, parity;
        sint32 count = loading.count;
        uint32 width;
        if (count > total)
            count = total;
        /* Preserve MIPS wrapping shifts/adds BEFORE signed division. */
        width = (sint32)(count * 170u) / total;
        memset(&p, 0, sizeof(p));
        setPolyG4(&p);
        p.p[0].rgbc.r = p.p[1].rgbc.r = 255;
        p.p[2].rgbc.r = p.p[3].rgbc.r = 216;
        p.p[2].rgbc.b = p.p[3].rgbc.b = 178;
        p.p[0].xy.x = p.p[1].xy.x = 77;
        p.p[2].xy.x = p.p[3].xy.x = (sint16)(width + 77);
        p.p[0].xy.y = p.p[2].xy.y = 203;
        p.p[1].xy.y = p.p[3].xy.y = 214;
        for (i = 0; i < 2; i++)
        {
            parity = disp_buffers.frame & 1;
            gpu_put_draw_env(0x800b8938 + 92 * parity, 320 * parity, 0);
            gpu_packet(&p); /* DrawSync is synchronous on the host. */
            disp_buffers.frame = disp_buffers.frame + 1;
        }
        if (!loading.active)
            gpu_present();
    }
    return resource_find_quiet(name);
}

typedef struct MODEL_FILE_HEADER
{
    uint32 height;
    MODEL_BODY_HEADER mesh;
} MODEL_FILE_HEADER;

typedef struct CHARACTER_FILE_HEADER
{
    MODEL_FILE_HEADER model;
    /* Copy size on disc, relocated to the source-tail address before copying */
    uint32 copy_size_or_end;
    uint32 bone_count;
    uint32 commands_offset;
    uint32 part_count;
    uint32 part_vertex_count[];
} CHARACTER_FILE_HEADER;

_Static_assert(sizeof(MODEL_FILE_HEADER) == 20 && offsetof(MODEL_FILE_HEADER, mesh) == 4 && sizeof(MODEL_BODY_HEADER) == 16 && offsetof(MODEL_BODY_HEADER, faces_wire) == 8 && sizeof(CHARACTER_FILE_HEADER) == 36 && offsetof(CHARACTER_FILE_HEADER, copy_size_or_end) == 20 && offsetof(CHARACTER_FILE_HEADER, bone_count) == 24 && offsetof(CHARACTER_FILE_HEADER, commands_offset) == 28 && offsetof(CHARACTER_FILE_HEADER, part_count) == 32 && offsetof(CHARACTER_FILE_HEADER, part_vertex_count) == 36, "Model and character file wire layouts");
_Static_assert(sizeof(MODEL_FACE_RECORD) == 12 && offsetof(MODEL_FACE_RECORD, flags) == 8 && offsetof(MODEL_FACE_RECORD, tex_offset) == 10, "Model face wire layout");
_Static_assert(sizeof(MODEL_RIG_HEADER) == 32 && offsetof(MODEL_RIG_HEADER, copy_size_or_end) == 16 && offsetof(MODEL_RIG_HEADER, bone_count) == 20 && offsetof(MODEL_RIG_HEADER, commands_offset) == 24 && offsetof(MODEL_RIG_HEADER, part_count) == 28 && offsetof(MODEL_RIG_HEADER, part_vertex_count) == 32, "Model rig wire layout");

_Static_assert(sizeof(SVECTOR) == 8 && offsetof(SVECTOR, vx) == 0 && offsetof(SVECTOR, vy) == 2 && offsetof(SVECTOR, vz) == 4, "Model vertex SDK layout");

uint32 resource_load_model(const char *name, uint32 type, uint32 tex_grp)
{
    FUNCTION_MARKER(0x8005FF2C, "GAME.EXE");
    RESOURCE_VIEW view = resource_find(name);
    uint32 p = view.context_address, vertices, faces, i;
    sint32 nv, nf;
    uint32 byte_len = view.byte_len;
    MODEL_FILE_HEADER *file;
    SVECTOR *v;
    MODEL_FACE_RECORD *face_recs;
    XASSERT(!((uintptr_t)view.data & 3) && byte_len >= sizeof(MODEL_FILE_HEADER));
    file = (MODEL_FILE_HEADER *)view.data;
    XASSERT(type < MODEL_TYPE_COUNT);
    model_height[type] = file->height;
    nv = file->mesh.vertex_count;
    nf = file->mesh.face_count;
    XASSERT(nv >= 0 && nf >= 0 && (uint32)nv <= (byte_len - sizeof(MODEL_FILE_HEADER)) / sizeof(SVECTOR) && (uint32)nf <= (byte_len - sizeof(MODEL_FILE_HEADER) - sizeof(SVECTOR) * (uint32)nv) / sizeof(MODEL_FACE_RECORD));
    vertices = p + 20;
    faces = vertices + 8u * nv;
    file->mesh.vertices_wire = vertices;
    file->mesh.faces_wire = faces;
    v = nv > 0 ? (SVECTOR *)(view.data + sizeof(MODEL_FILE_HEADER)) : NULL;
    for (i = 0; (sint32)i < nv; i++)
    {
        v[i].vx >>= 1;
        v[i].vy >>= 1;
        v[i].vz >>= 1;
    }
    face_recs = nf > 0 ? (MODEL_FACE_RECORD *)(view.data + sizeof(MODEL_FILE_HEADER) + sizeof(SVECTOR) * (uint32)nv) : NULL;
    for (i = 0; (sint32)i < nf; i++)
    {
        MODEL_FACE_RECORD *face = face_recs + i;
        if ((sint16)face->vertex_offset[2] != -1 || (sint16)face->vertex_offset[3] != -1)
        {
            XASSERT(tex_grp < TEXTURE_GROUP_COUNT);
            sint32 grp = tex_grps[tex_grp].first_tex;
            sint32 offset = (sint16)face->tex_offset / 12;
            sint32 idx = grp + offset;
            XASSERT(idx >= 0 && idx < TEXTURE_MAP_COUNT);
            face->tex_offset = 12 * tex_map[idx];
        }
    }
    model_refs[type].byte_len = byte_len - sizeof(uint32);
    model_refs[type].header = &file->mesh;
    model_refs[type].vertices = (SVECTOR *)(view.data + sizeof(MODEL_FILE_HEADER));
    /* RTPT may read adjacent WAD bytes beyond the declared MOD member */
    model_refs[type].vertex_byte_len = view.storage_len - (uint32)(view.data - view.storage) - sizeof(MODEL_FILE_HEADER);
    model_refs[type].faces = (MODEL_FACE_RECORD *)(view.data + sizeof(MODEL_FILE_HEADER) + sizeof(SVECTOR) * (uint32)nv);
    model_refs[type].face_byte_len = byte_len - sizeof(MODEL_FILE_HEADER) - sizeof(SVECTOR) * (uint32)nv;
    if (type - 665 < 2 || type == 667 || type == 94)
    {
        model_height[type] = 0;
        return 4 * type;
    }
    return 94;
}

static uint32 relocate_offset(uint32 *offset, uint32 base)
{
    FUNCTION_MARKER(0x80011D74, "GAME.EXE");
    int64_t sum = (int64_t)(sint32)*offset + (sint32)base;
    XASSERT(sum <= 2147483647LL && sum >= (-2147483647LL - 1));
    *offset = (uint32)sum;
    return sum;
}

uint32 resource_load_character(const char *name, uint32 type, uint32 grp)
{
    FUNCTION_MARKER(0x80060110, "GAME.EXE");
    RESOURCE_VIEW view = resource_find(name);
    uint32 p = view.context_address, src = p + 4, size, dst, i, faces_wire, val = 0xff50;
    uint32 vertex_offset, face_offset;
    sint32 count;
    uint32 byte_len = view.byte_len;
    CHARACTER_FILE_HEADER *file;
    MODEL_BODY_HEADER *copy;
    MODEL_FACE_RECORD *face_recs;
    XASSERT(!((uintptr_t)view.data & 3) && byte_len >= sizeof(CHARACTER_FILE_HEADER));
    file = (CHARACTER_FILE_HEADER *)view.data;
    XASSERT(type < MODEL_TYPE_COUNT);
    model_height[type] = file->model.height;
    size = file->copy_size_or_end;
    XASSERT((sint32)size >= 0 && size >= sizeof(CHARACTER_FILE_HEADER) - sizeof(uint32) && size <= byte_len - sizeof(uint32));
    XASSERT(file->model.mesh.vertex_count >= 0 && file->model.mesh.face_count >= 0 && file->model.mesh.vertices_wire <= size && file->model.mesh.faces_wire <= size && (uint32)file->model.mesh.vertex_count <= (size - file->model.mesh.vertices_wire) / sizeof(SVECTOR) && (uint32)file->model.mesh.face_count <= (size - file->model.mesh.faces_wire) / sizeof(MODEL_FACE_RECORD));
    XASSERT(!(file->model.mesh.vertices_wire & 3) && !(file->model.mesh.faces_wire & 3) && !(file->commands_offset & 3) && file->part_count <= (size - (sizeof(CHARACTER_FILE_HEADER) - sizeof(uint32))) / sizeof(uint32) && file->commands_offset >= sizeof(CHARACTER_FILE_HEADER) - sizeof(uint32) + file->part_count * sizeof(uint32) && file->commands_offset <= size - sizeof(uint32));
    relocate_offset(&file->copy_size_or_end, src);
    dst = resource_arenas.persistent.context;
    uint8 *copy_data = resource_arenas.persistent.data;
    uintptr_t copy_addr = (uintptr_t)copy_data, pool = (uintptr_t)resource_storage.data;
    /* TODO: Investigate invalid or overlapping ACH copies without guessing copy semantics */
    XASSERT(copy_data && !(copy_addr & 3) && copy_addr >= pool && copy_addr - pool <= RESOURCE_STORAGE_CAPACITY && resource_arenas.persistent.available <= RESOURCE_STORAGE_CAPACITY - (copy_addr - pool) && size <= resource_arenas.persistent.available);
    copy = (MODEL_BODY_HEADER *)copy_data;
    uintptr_t copy_begin = (uintptr_t)copy, src_begin = (uintptr_t)(view.data + sizeof(uint32));
    XASSERT(copy_begin >= src_begin + size || src_begin >= copy_begin + size);
    memcpy(copy, view.data + sizeof(uint32), size);
    dst = resource_arenas.persistent.context;
    /* TODO: Investigate allocation beyond the actual native resource owner */
    XASSERT(resource_arenas.persistent.data && (size) <= resource_arenas.persistent.available);
    resource_arenas.persistent.data += size;
    resource_arenas.persistent.available -= size;
    resource_arenas.persistent.context = dst + size;
    /* The destination begins at the vertices field, after the file height */
    XASSERT(!(dst & 3));
    /* The original reload cannot change across the synchronous native copy */
    copy = (MODEL_BODY_HEADER *)copy_data;
    count = copy->face_count;
    /* Decode the validated relative links before mutating their serialized words */
    vertex_offset = copy->vertices_wire;
    face_offset = copy->faces_wire;
    relocate_offset(&copy->vertices_wire, dst);
    relocate_offset(&copy->faces_wire, dst);
    faces_wire = copy->faces_wire;
    XASSERT(count <= 0 || (!(faces_wire & 3) && face_offset <= size && (uint32)count <= (size - face_offset) / sizeof(*face_recs)));
    face_recs = count > 0 ? (MODEL_FACE_RECORD *)((uint8 *)copy + face_offset) : NULL;
    for (i = 0; (sint32)i < count; i++)
    {
        MODEL_FACE_RECORD *face = face_recs + i;
        XASSERT(grp < TEXTURE_GROUP_COUNT);
        sint32 tex_grp = tex_grps[grp].first_tex;
        sint32 offset = (sint16)face->tex_offset / 12;
        sint32 idx = tex_grp + offset;
        XASSERT(idx >= 0 && idx < TEXTURE_MAP_COUNT);
        face->tex_offset = 12 * tex_map[idx];
    }
    /* TODO: Investigate face data aliasing the relocated mesh prefix before supporting it */
    XASSERT(copy->vertices_wire == dst + vertex_offset && copy->faces_wire == faces_wire && copy->face_count == count);
    model_refs[type].byte_len = size;
    model_refs[type].header = copy;
    model_refs[type].vertices = (SVECTOR *)((uint8 *)copy + vertex_offset);
    model_refs[type].vertex_byte_len = size - vertex_offset;
    model_refs[type].faces = (MODEL_FACE_RECORD *)((uint8 *)copy + face_offset);
    model_refs[type].face_byte_len = size - face_offset;
    switch (type)
    {
        case 0:
        case 5:
        case 6:
        case 7:
        case 8:
        case 9:
        case 10:
            val = 0xff58;
            break;
        case 1:
            val = 0xff68;
            break;
        case 2:
        case 14:
        case 31:
        case 49:
            val = 0xff5c;
            break;
        case 3:
            val = 0xff74;
            break;
        case 11:
            val = 0xff54;
            break;
        case 12:
        case 13:
        case 29:
        case 38:
            val = 0xff60;
            break;
        case 48:
            val = 0xff6c;
            break;
        case 51:
            val = 0xffc0;
            break;
        case 28:
            val = 0xffb0;
            break;
    }
    val <<= 16;
    model_height[type] = val;
    return val;
}

/* Original command catalog; native names and flags retain original row order */
typedef struct ANIMATION_DEF
{
    const char *name;
    uint16 flags;
    uint16 idx;
} ANIMATION_DEF;

static const ANIMATION_DEF animation_defs[] = {
    {"PUNCH1", 0x2810, 0},   {"PUNCH2", 0x5010, 1},     {"PUNCH3", 0x7810, 2},     {"PUNCH4", 0xa010, 3},    {"PUNCH4A", 0x2810, 10},  {"PUNCH4B", 0x5010, 11},  {"PUNCH4C", 0x7810, 12},  {"PUNCH4D", 0xa010, 13},  {"PUNCH5", 0xc810, 4},     {"PUNCH6", 0xf010, 5},     {"PUNCH7", 0xf010, 6},     {"PUNCH8", 0xf010, 7},     {"PUNCH9", 0xf010, 8},     {"PUNCH10", 0xa010, 9},    {"APUNCH4A", 0x2810, 14}, {"APUNCH4B", 0x5010, 15}, {"APUNCH4C", 0x7800, 16}, {"APUNCH4D", 0xa000, 17}, {"APUNCH4E", 0x2800, 18}, {"SLAP1A", 0x2800, 19},   {"SLAP1B", 0x5000, 20},   {"SLAP1C", 0x5000, 21},   {"SLAP1D", 0x5000, 22},   {"SLAP1E", 0x5000, 23},   {"BFIST1", 0x2820, 24},    {"BFIST2", 0x2820, 25},    {"KICK1", 0x5008, 26},     {"KICK2", 0x7808, 27},     {"KICK3", 0x5008, 28},     {"KICK4", 0x5008, 39},    {"KICK3", 0x5008, 29},    {"KICK5", 0xa008, 30},    {"KICK10", 0x5008, 31},    {"PUSH1", 0xf010, 32},     {"360SPIN", 0xff00, 33},  {"HEAD1", 0xa000, 34},
    {"KNEE1", 0x8c01, 40},   {"KNEE2", 0xa000, 41},     {"BAT1", 0x5000, 35},      {"BAT2", 0x6400, 37},     {"BAT3", 0x7800, 38},     {"KNIFE1", 0x5000, 42},   {"KNIFE2", 0x7800, 43},   {"KNIFE3", 0x5000, 44},   {"KNIFE4", 0x7800, 45},    {"FREE1", 0x5000, 49},     {"FREE1", 0x5000, 36},     {"STD1", 0x0001, 68},      {"VAMPSPS1", 0x3c00, 46},  {"VAMPSPS2", 0x5000, 47},  {"VAMPSPS3", 0x6400, 48}, {"VPUNCH1", 0x8c10, 0},   {"VPUNCH2", 0xb410, 1},   {"WALK", 0x0002, 82},     {"WALK2", 0x0002, 83},    {"WALK3", 0x0002, 84},    {"WALK4", 0x0002, 85},    {"WALK5", 0x0002, 86},    {"STBY1", 0x0001, 69},    {"STBY3", 0x0001, 70},    {"STBY4", 0x0001, 71},     {"STBY5", 0x0001, 72},     {"STBY6", 0x0001, 73},     {"STBY7", 0x0001, 74},     {"STBY10", 0x0001, 76},    {"STBY11", 0x0001, 77},   {"STBY15", 0x0001, 78},   {"GUNSBY", 0x0001, 79},   {"BATSBY", 0x0001, 80},    {"SWDSBY", 0x0001, 81},    {"STBY91", 0x0001, 211},  {"STBY92", 0x0001, 212},
    {"STBY93", 0x0001, 213}, {"STBY94", 0x0001, 214},   {"STBY95", 0x0001, 215},   {"STBY96", 0x0001, 216},  {"STBY97", 0x0001, 217},  {"STBY98", 0x0001, 218},  {"STBY99", 0x0001, 219},  {"WALK90", 0x0000, 220},  {"WALK91", 0x0000, 221},   {"WALK92", 0x0000, 222},   {"PICK1", 0x0000, 118},    {"PICK2", 0x0000, 119},    {"PICK2A", 0x0000, 120},   {"PICK4A", 0x0000, 121},   {"PICK4B", 0x0000, 122},  {"PICK1B", 0x0000, 123},  {"PICK2B", 0x0000, 124},  {"PICK2C", 0x0000, 125},  {"PICK2A1", 0x0000, 126}, {"PICK2A2", 0x0000, 127}, {"PICK5A", 0x0000, 128},  {"PICK5B", 0x0000, 129},  {"DRINK", 0x0000, 132},   {"ROT1", 0x0000, 92},     {"ROT3", 0x0000, 93},      {"ROT5", 0x0000, 94},      {"ROT6", 0x0000, 95},      {"ROT10", 0x0000, 96},     {"ROT15", 0x0000, 97},     {"RUN", 0x0002, 87},      {"RUN2", 0x0002, 88},     {"RUN3", 0x0002, 89},     {"RUN5", 0x0002, 90},      {"STEPF", 0x0000, 98},     {"STEPF1", 0x0000, 99},   {"STEPF2", 0x0000, 100},
    {"STEPF3", 0x0000, 101}, {"STEPF4", 0x0000, 102},   {"STEPB", 0x0000, 103},    {"STEPB1", 0x0000, 104},  {"STEPB2", 0x0000, 105},  {"STEPB3", 0x0000, 106},  {"STEPB4", 0x0000, 107},  {"STEPR", 0x0000, 108},   {"STEPR1", 0x0000, 109},   {"STEPR2", 0x0000, 110},   {"STEPR3", 0x0000, 111},   {"STEPR4", 0x0000, 112},   {"STEPL", 0x0000, 113},    {"STEPL1", 0x0000, 114},   {"STEPL2", 0x0000, 115},  {"STEPL3", 0x0000, 116},  {"STEPL4", 0x0000, 117},  {"HIT1", 0x0000, 133},    {"HIT1", 0x0000, 134},    {"HIT1", 0x0000, 135},    {"HIT1", 0x0000, 136},    {"HIT1A", 0x0000, 133},   {"HIT1B", 0x0000, 134},   {"HIT1C", 0x0000, 135},   {"HIT1D", 0x0000, 136},    {"HIT2", 0x0000, 152},     {"HIT2", 0x0000, 153},     {"HIT2A", 0x0000, 152},    {"HIT2B", 0x0000, 153},    {"HIT3", 0x0000, 138},    {"HIT4", 0x0000, 139},    {"HITS1", 0x0000, 137},   {"HIT9", 0x5a00, 140},     {"HIT10", 0x5a00, 141},    {"HIT11", 0x5a00, 142},   {"HIT5", 0x0000, 155},
    {"HIT15", 0x5a00, 156},  {"HIT6", 0x0000, 157},     {"HIT14A", 0x5a00, 143},   {"HIT14B", 0x5a00, 144},  {"HIT16A", 0x5a01, 145},  {"HIT16B", 0x5a01, 146},  {"FALL1", 0x1400, 147},   {"FALL3", 0x1400, 148},   {"GETUP1", 0x0000, 149},   {"GETUP1", 0x0000, 151},   {"GETUP2", 0x0000, 150},   {"GETUP3", 0x0000, 151},   {"FALL2", 0x5000, 154},    {"JUMP1", 0x5000, 158},    {"JUMPS1", 0x6408, 65},   {"JUMPS2", 0x6408, 66},   {"SLIDE1", 0x1428, 67},   {"THROW2A", 0x0000, 159}, {"THROW2B", 0x0000, 160}, {"THROW3A", 0x0000, 161}, {"THROW3B", 0x0000, 162}, {"THROW5A", 0x0000, 163}, {"THROW5B", 0x0000, 164}, {"GRAB1", 0x0018, 166},   {"GRAB2", 0x0000, 167},    {"GRAB2A", 0x0001, 169},   {"GRAB3", 0x0000, 170},    {"AGRAB2", 0x0000, 168},   {"GRAB-1", 0x0000, 171},   {"GRAB-1", 0x0000, 175},  {"GRAB-2", 0x3200, 172},  {"GRAB-3", 0x4600, 173},  {"GRAB-4", 0x5000, 174},   {"GETOUT", 0x0000, 176},   {"THROW1", 0x7800, 177},  {"THROW2", 0x7800, 178},
    {"THROW4", 0x7800, 179}, {"THROWN1", 0x7800, 180},  {"THROWN2", 0x7800, 181},  {"THROWN3", 0x7800, 182}, {"THROWN4", 0x7800, 183}, {"THROWN5", 0x7800, 184}, {"THROWN6", 0x7800, 185}, {"THROWN7", 0x7800, 186}, {"THROWN9", 0x7800, 187},  {"THROWN10", 0x7800, 188}, {"THROWN12", 0x3c00, 189}, {"SHOCK", 0x0000, 190},    {"ELEVFALL", 0x0000, 191}, {"WRENCH1", 0x0000, 130},  {"WRENCH2", 0x0000, 131}, {"SHOOT1", 0x0000, 192},  {"SHOOT2", 0x0000, 193},  {"SHOOT2A", 0x0000, 193}, {"SHOOT2B", 0x0000, 194}, {"SHOOT3", 0x0000, 195},  {"SHOOT5", 0x0000, 196},  {"ROT1A", 0x0000, 91},    {"BLOCKIN1", 0x0000, 57}, {"BLOCKOT1", 0x0000, 58}, {"BLOCKHT1", 0x0000, 59},  {"BLOCKSB1", 0x0000, 60},  {"BSBRTH", 0x0001, 233},   {"DROP", 0x0000, 205},     {"DROPLAND", 0x0000, 206}, {"ROPE1", 0x0001, 207},   {"SMLDROP", 0x0000, 203}, {"SMLLAND", 0x0000, 204}, {"HIT2DIZF", 0x0000, 234}, {"HIT2DIZB", 0x0000, 235}, {"DIZZY", 0x0000, 236},   {"DIZFALL", 0x0000, 237},
    {"STBY9", 0x0000, 238},  {"GETUPDIZ", 0x0000, 239}, {"FACEKICK", 0x0000, 240}, {"BAT2HED", 0x0000, 241}, {"FNTSBY", 0x0000, 242},  {"PUNCHEX1", 0x7810, 54}, {"PUNCHEX2", 0xa010, 55}, {"PUNCHEX3", 0xc810, 56}, {"BLOCKIN1", 0x0000, 208}, {"BLOCKOT1", 0x0000, 209}, {"BLOCKSB1", 0x0000, 210}, {"SBLOCKIN", 0x0000, 208}, {"SBLOCKOT", 0x0000, 209}, {"SBLOCKSB", 0x0000, 210}, {"VBLKIN1", 0x0000, 61},  {"VBLKOT1", 0x0000, 62},  {"VBLKHT1", 0x0000, 63},  {"VBLKSB1", 0x0000, 64},  {"COCK1", 0x0000, 247},   {"COCK2", 0x0000, 248},   {"BLADE1", 0x0000, 223},  {"BLADE2", 0x0000, 224},  {"BLADE3", 0x0000, 225},  {"BLADE4", 0x0000, 226},  {"SHOT1", 0x0000, 249},    {"SHOT2", 0x0000, 250},    {"SHTDEAD", 0x0000, 251},  {"FSTBY", 0x0000, 252},    {"FDSBY", 0x0000, 253},    {"ATTACK1", 0x5000, 230}, {"ATTACK2", 0x5000, 231}, {"ATTACK3", 0x5000, 232}, {"FLYF", 0x0000, 254},     {"FLYFIN", 0x0000, 255},   {"FLYFOUT", 0x0000, 256}, {"FLYB", 0x0000, 257},
    {"FLYBIN", 0x0000, 258}, {"FLYBOUT", 0x0000, 259},  {"FLYL", 0x0000, 260},     {"FLYLIN", 0x0000, 261},  {"FLYLOUT", 0x0000, 262}, {"FLYR", 0x0000, 263},    {"FLYRIN", 0x0000, 264},  {"FLYROUT", 0x0000, 265}, {"FLYS", 0x0000, 266},     {"FLYSIN", 0x0000, 267},   {"FLYSOUT", 0x0000, 268},  {"JUMPOFF", 0x0000, 269},  {"GUNIN", 0x0001, 270},    {"GUNFIRE", 0x0001, 271},  {"GUNOUT", 0x0001, 272},  {"LAUNCH1", 0x0000, 273}, {"LAUNCH2", 0x0000, 274}, {"LAUNCH3", 0x0000, 275}, {"LAUNCH4", 0x0000, 276}, {"BSTBY", 0x0000, 277},   {"B180", 0x0000, 278},    {"RSTBY", 0x0000, 279},   {"R180", 0x0000, 280},    {"RHIT", 0x0000, 281},    {"SLEEPSBY", 0x0001, 282}, {"SLEEPUP1", 0x0000, 283}, {"SLEEPUP2", 0x0000, 284}, {"TRAMPSBY", 0x0001, 285}, {"FLOATTNK", 0x0001, 286}, {"STBY1", 0x0001, 287},   {"JUMP1", 0x0000, 288},   {"SPESH1", 0x6400, 50},   {"SPESH2", 0x7800, 51},    {"SPESH3", 0x6400, 52},    {"SPESH4", 0x8c00, 53},
};

/* Little-endian ADT record; size includes its four-byte size word */
typedef struct ANIMATION_FILE_ENTRY
{
    char name[8];
    uint32 size;
    uint8 data[];
} ANIMATION_FILE_ENTRY;

_Static_assert(sizeof(ANIMATION_FILE_ENTRY) == 12 && offsetof(ANIMATION_FILE_ENTRY, size) == 8 && offsetof(ANIMATION_FILE_ENTRY, data) == 12, "Animation file wire layout");

uint32 resource_load_animations(const char *name, sint32 type)
{
    FUNCTION_MARKER(0x8001721C, "GAME.EXE");
    uint32 p, remaining, i;
    RESOURCE_VIEW view;
    ANIMATION_BANK_ID table;
    const uint8 *cursor;
    if (type == -1)
        table = ANIMATION_BANK_SHARED6;
    else if (type == -2)
        table = ANIMATION_BANK_SHARED7;
    else if (type == -3)
        table = ANIMATION_BANK_SHARED8;
    else
    {
        XASSERT((uint32)type < ANIMATION_TYPE_COUNT);
        table = animation_type_banks[type];
    }
    view = resource_find(name);
    p = view.context_address;
    remaining = view.byte_len;
    cursor = view.data;
    for (;;)
    {
        const ANIMATION_FILE_ENTRY *rec;
        size_t len;
        uint32 step;
        XASSERT(remaining);
        if (!*cursor)
            break;
#if defined(_DEBUG)
        XASSERT(remaining >= sizeof(ANIMATION_FILE_ENTRY) && !((uintptr_t)cursor & 3));
        const void *name_end = memchr(cursor, 0, remaining);
        XASSERT(name_end);
#endif
        rec = (const ANIMATION_FILE_ENTRY *)cursor;
        XASSERT(rec->size >= sizeof(uint32) && rec->size <= remaining - 8);
        len = strlen((const char *)cursor);
        if (len > 8)
            len = 8;
        for (i = 0; i < sizeof(animation_defs) / sizeof(animation_defs[0]); i++)
        {
            const ANIMATION_DEF *def = &animation_defs[i];
            if (strlen(def->name) == len && !memcmp(rec->name, def->name, len))
            {
                if (table == ANIMATION_BANK_NONE)
                {
                    /* TODO: Resolve a missing animation bank after the full CDB playthrough */

                    XASSERT(0);
                }
                else
                {
                    XASSERT(table < ANIMATION_BANK_COUNT);
                    uintptr_t data = (uintptr_t)rec->data, base = (uintptr_t)resource_storage.data;
                    uint32 in_arena = data >= base && data - base < RESOURCE_STORAGE_CAPACITY;
                    uint32 capacity = in_arena ? RESOURCE_STORAGE_CAPACITY - (uint32)(data - base) : view.storage_len - (uint32)(rec->data - view.storage);
                    ANIMATION_STREAM_DOMAIN domain = ANIMATION_STREAM_NATIVE;
                    if (in_arena)
                    {
                        uint32 context = p + 12, offset = context & 0x1fffffff, seg = context & 0xe0000000;
                        /* TODO: Investigate a serialized identity that disagrees with its actual native stream */
                        XASSERT((!seg || seg == 0x80000000 || seg == 0xa0000000) && offset >= RESOURCE_STORAGE_ORIGIN && data - base == offset - RESOURCE_STORAGE_ORIGIN);
                        domain = seg == 0x80000000 ? ANIMATION_STREAM_CACHED : seg == 0xa0000000 ? ANIMATION_STREAM_UNCACHED : ANIMATION_STREAM_PHYSICAL;
                    }
                    ANIMATION_STREAM stream = {(const ANIMATION_STREAM_HEADER *)rec->data, capacity, domain};
                    animation_banks[table].stream[def->idx] = (ACTOR_ANIMATION_SOURCE){ACTOR_ANIMATION_SOURCE_STREAM, stream};
                }
                animation_flags[def->idx] = def->flags;
            }
        }
        /* Reload the record size after all binding writes, as in the original */
        XASSERT(rec->size >= sizeof(uint32) && rec->size <= remaining - 8);
        step = rec->size + 8;
        p += step;
        cursor += step;
        remaining -= step;
    }
    return 0;
}

uint32 resource_load_frontend_models(void)
{
    FUNCTION_MARKER(0x80040AA0, "GAME.EXE");

    typedef struct FRONTEND_MODEL
    {
        const char *name;
        uint32 type;
        uint32 tex_grp;
    } FRONTEND_MODEL;

    static const FRONTEND_MODEL models[] = {
        {"TARGET.MOD", 887, 0}, {"TARGETG.MOD", 888, 0}, {"SHADOW.MOD", 860, 0}, {"FRONTB01.MOD", 454, 34}, {"FRONTB02.MOD", 455, 34}, {"FRONTB04.MOD", 456, 34}, {"LAMPL01.MOD", 457, 34}, {"LAMPL02.MOD", 458, 34}, {"LAMPR01.MOD", 459, 34}, {"AIRLINER.MOD", 832, 70}, {"HELCOP.MOD", 833, 70}, {"ROTOR.MOD", 564, 70}, {"HELISHAD.MOD", 834, 70}, {"POLICAR1.MOD", 835, 70}, {"POLICAR2.MOD", 836, 70}, {"CARSHAD1.MOD", 837, 70}, {"LIGHT1.MOD", 838, 70}, {"LIGHT2.MOD", 839, 70}, {"LIGHT3.MOD", 840, 70},
    };
    static const char *const animation_names[4] = {"HAWK.ADT", "MACE.ADT", "SMASHER.ADT", "ALANA.ADT"};
    static const char *const character_names[4] = {"HAWK.ACH", "MACE.ACH", "SMASHER.ACH", "ALANA.ACH"};
    uint32 i, result = 0;
    resource_load(66, resource_arenas.persistent.context, NULL, 0, NULL, &resource_arenas.persistent);
    /* TODO: Investigate allocation beyond the actual native resource owner */
    XASSERT(resource_arenas.persistent.data && (resource_io.file_size) <= resource_arenas.persistent.available);
    resource_arenas.persistent.data += resource_io.file_size;
    resource_arenas.persistent.available -= resource_io.file_size;
    resource_arenas.persistent.context = resource_arenas.persistent.context + resource_io.file_size;
    for (i = 0; i < 19; i++)
        resource_load_model(models[i].name, models[i].type, models[i].tex_grp);
    for (i = 0; i < 4; i++)
        resource_load_animations(animation_names[i], i);
    for (i = 0; i < 4; i++)
        result = resource_load_character(character_names[i], i, 36 + i);
    return result;
}

/* Preserve four loads before four stores, including overlapping byte views */
uint32 resource_copy_block(const uint8 *src, uint8 *dst)
{
    uint32 offset, idx, words[4];
    /* TODO: Investigate invalid native copy pointers without substituting a buffer */
    XASSERT(src && dst);
    for (offset = 0; offset < 512; offset += 16)
    {
        for (idx = 0; idx < 4; ++idx)
            memcpy(&words[idx], src + offset + idx * 4, sizeof(words[idx]));
        for (idx = 0; idx < 4; ++idx)
            memcpy(dst + offset + idx * 4, &words[idx], sizeof(words[idx]));
    }
    return words[0];
}

/* Original count shifts by one, then copies uint32 words in forward order */
uint32 resource_copy_words(uint8 *dst, const uint8 *src, uint32 count)
{
    FUNCTION_MARKER(0x80011F50, "GAME.EXE");
    uint32 result;
    /* TODO: Investigate counts0/1 before the original do-while countdown underflows */
    XASSERT(src && dst && count >= 2);
    count >>= 1;
    do
    {
        memcpy(&result, src, sizeof(result));
        src += sizeof(result);
        memcpy(dst, &result, sizeof(result));
        --count;
        dst += sizeof(result);
    } while (count);
    return result;
}

sint32 model_vertex_dir(MODEL_REFERENCE mesh, uint32 v0, uint32 v1)
{
    FUNCTION_MARKER(0x800163acu, "GAME.EXE");
    /* TODO: Investigate original vertex indices outside the retained resource */
    XASSERT(mesh.vertices && v0 < mesh.vertex_byte_len / sizeof(*mesh.vertices) && v1 < mesh.vertex_byte_len / sizeof(*mesh.vertices));
    sint32 x0 = mesh.vertices[v0].vx;
    sint32 z0 = mesh.vertices[v0].vz;
    sint32 x1 = mesh.vertices[v1].vx;
    sint32 z1 = mesh.vertices[v1].vz;
    return (sint16)math_dir16(x0, z0, x1, z1);
}
