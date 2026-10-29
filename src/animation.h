#ifndef ANIMATION_H
#define ANIMATION_H

#include "xport.h"
#include "actors.h"

enum
{
    ANIMATION_COUNT = 289
};

enum
{
    ANIMATION_TYPE_COUNT = 54
};

typedef enum ANIMATION_BANK_ID
{
    ANIMATION_BANK_NONE,
    ANIMATION_BANK_HAWK,
    ANIMATION_BANK_MACE,
    ANIMATION_BANK_SMASHER,
    ANIMATION_BANK_ALANA,
    ANIMATION_BANK_HAWK2,
    ANIMATION_BANK_SHARED0,
    ANIMATION_BANK_SHARED1,
    ANIMATION_BANK_SHARED2,
    ANIMATION_BANK_SHARED3,
    ANIMATION_BANK_SHARED4,
    ANIMATION_BANK_SHARED5,
    ANIMATION_BANK_SHARED6,
    ANIMATION_BANK_SHARED7,
    ANIMATION_BANK_SHARED8,
    ANIMATION_BANK_COUNT
} ANIMATION_BANK_ID;

/* Frame-local motion uses the original vertical/lateral/forward component order */
typedef struct ACTOR_FRAME_MOTION
{
    union
    {
        struct
        {
            sint32 vertical, lateral, forward;
        };

        sint32 v[3];
        uint32 bits[3];
    };

    uint32 events;

    union
    {
        sint32 forward_offset;
        uint32 forward_offset_bits;
    };
} ACTOR_FRAME_MOTION;

/* Serialized little-endian ADT stream header and packed frame descriptors */
typedef struct ANIMATION_STREAM_HEADER
{
    uint32 frame_count;
    uint32 frame[];
} ANIMATION_STREAM_HEADER;

/* Distinct PS1 views can name the same bytes without comparing equal */
typedef enum ANIMATION_STREAM_DOMAIN
{
    ANIMATION_STREAM_NATIVE,
    ANIMATION_STREAM_PHYSICAL,
    ANIMATION_STREAM_CACHED,
    ANIMATION_STREAM_UNCACHED,
    ANIMATION_STREAM_DOMAIN_COUNT
} ANIMATION_STREAM_DOMAIN;

typedef struct ANIMATION_STREAM
{
    const ANIMATION_STREAM_HEADER *header;
    uint32 byte_len;
    ANIMATION_STREAM_DOMAIN domain;
} ANIMATION_STREAM;

typedef enum ACTOR_ANIMATION_SOURCE_KIND
{
    ACTOR_ANIMATION_SOURCE_NONE,
    ACTOR_ANIMATION_SOURCE_STREAM,
    ACTOR_ANIMATION_SOURCE_PROCEDURAL
} ACTOR_ANIMATION_SOURCE_KIND;

typedef struct ACTOR_ANIMATION_SOURCE
{
    ACTOR_ANIMATION_SOURCE_KIND kind;
    ANIMATION_STREAM stream;
} ACTOR_ANIMATION_SOURCE;

typedef struct ACTOR_ANIMATION
{
    ACTOR_ANIMATION_SOURCE src;
    /* Type17 reuses these scalars as the van phase and local scene tick */
    uint32 frame;
    uint32 id;
    /* Retained whole-word selector also supplies a model frame */
    uint32 selector;
    /* Low actor bits plus the original stage/region contact latch bits */
    uint16 hit_flags;
    uint16 event_step;
    ACTOR_FRAME_MOTION motion;
} ACTOR_ANIMATION;

extern ACTOR_ANIMATION actor_animations[ACTOR_COUNT];

typedef struct ANIMATION_BANK
{
    ACTOR_ANIMATION_SOURCE stream[ANIMATION_COUNT];
} ANIMATION_BANK;

extern ANIMATION_BANK animation_banks[ANIMATION_BANK_COUNT];
extern ANIMATION_BANK_ID animation_type_banks[ANIMATION_TYPE_COUNT];

extern uint16 animation_flags[ANIMATION_COUNT];
extern uint32 animation_fall_exit;

sint32 animation_start_toggle(sint32 idx, ANIMATION_BANK_ID table, sint32 animation);
sint32 animation_start(sint32 idx, ANIMATION_BANK_ID table, sint32 animation);
ACTOR_ANIMATION_SOURCE animation_stream(ANIMATION_BANK_ID table, sint32 animation);
uint32 animation_matches_stream(sint32 idx, uint32 action);
sint32 animation_movement_state(uint32 state);
sint32 animation_update_motion(sint32 idx);
sint32 animation_scale_vertical_motion(sint32 idx);

#endif
