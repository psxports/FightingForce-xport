#ifndef SOUND_DEVICE_H
#define SOUND_DEVICE_H
#include "psx_spu.h"

enum
{
    SOUND_HEAP_LIMIT = 2,
    SOUND_HEAP_RECORD_COUNT = SOUND_HEAP_LIMIT + 1
};

extern SpuMallocState sound_heap;
extern SpuMallocRecord sound_heap_recs[SOUND_HEAP_RECORD_COUNT];
void sound_core_init(void);
/* Cold SDK reset CPU/core path; platform IRQ/event setup is a host boundary. */
uint32 sound_device_reset(void);
/* Native resource span to the existing shared SDK host-source API */
uint32 sound_transfer_write(const void *src, uint32 bytes, uint32 src_bytes);
void sound_transfer_wait(void);
sint32 sound_heap_init(sint32 entries, SpuMallocRecord *recs);
sint32 sound_heap_allocate(uint32 bytes);
#endif
