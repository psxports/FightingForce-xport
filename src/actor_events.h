#ifndef ACTOR_EVENTS_H
#define ACTOR_EVENTS_H

#include "xport.h"

struct EFFECT;

enum
{
    ACTOR_EVENT_SLOT_COUNT = 289,
    ACTOR_EVENT_COUNT = 23
};

typedef enum ACTOR_EVENT_SIGNATURE
{
    ACTOR_EVENT_NONE,
    ACTOR_EVENT_SINT32,
    ACTOR_EVENT_UINT32,
    ACTOR_EVENT_PICKUP,
    ACTOR_EVENT_INTPTR,
    ACTOR_EVENT_EFFECT
} ACTOR_EVENT_SIGNATURE;

typedef struct ACTOR_EVENT
{
    ACTOR_EVENT_SIGNATURE signature;

    union
    {
        void (*none)(void);
        sint32 (*s32_idx)(sint32);
        uint32 (*u32_idx)(sint32);
        uint32 (*pickup)(sint32, uint32[3]);
        intptr (*intptr_idx)(sint32);
        struct EFFECT *(*effect_idx)(sint32);
    } fn;

} ACTOR_EVENT;

extern const ACTOR_EVENT *actor_events[ACTOR_EVENT_SLOT_COUNT];
extern const ACTOR_EVENT actor_event_noop;
extern const ACTOR_EVENT actor_event_attack_voice;
extern const ACTOR_EVENT actor_event_ground_effect;
extern const ACTOR_EVENT actor_event_ground_effect_voice;
extern const ACTOR_EVENT actor_event_attack_fixed_voice;
extern const ACTOR_EVENT actor_event_attack_effects;
extern const ACTOR_EVENT actor_event_hit_repeat;
extern const ACTOR_EVENT actor_event_running_attack;
extern const ACTOR_EVENT actor_event_hit_begin;
extern const ACTOR_EVENT actor_event_heavy_hit;
extern const ACTOR_EVENT actor_event_hit_end;
extern const ACTOR_EVENT actor_event_hit_attachment;
extern const ACTOR_EVENT actor_event_hit_impact;
extern const ACTOR_EVENT actor_event_hit_flash;
extern const ACTOR_EVENT actor_event_attack_attachment;
extern const ACTOR_EVENT actor_event_attachment_impact;
extern const ACTOR_EVENT actor_event_impact_voice;
extern const ACTOR_EVENT actor_event_attachment_particle0;
extern const ACTOR_EVENT actor_event_attachment_particle1;
extern const ACTOR_EVENT actor_event_burst_attachment;
extern const ACTOR_EVENT actor_event_pickup;
extern const ACTOR_EVENT actor_event_release_item;
extern const ACTOR_EVENT actor_event_shock_attack;

void actor_event_call(const ACTOR_EVENT *event, sint32 idx);

void actor_ignore_event(void);
sint32 actor_attack_voice_event(sint32 idx);
sint32 actor_attack_fixed_voice_event(sint32 idx);
sint32 actor_hit_repeat_event(sint32 idx);
sint32 actor_hit_begin_event(sint32 idx);
sint32 actor_heavy_hit_event(sint32 idx);
sint32 actor_hit_end_event(sint32 idx);
sint32 actor_hit_attachment_event(sint32 idx);
sint32 actor_hit_impact_event(sint32 idx);
sint32 actor_hit_flash_event(sint32 idx);
sint32 actor_attack_attachment_event(sint32 idx);
uint32 actor_attachment_impact_event(sint32 idx);
sint32 actor_impact_voice_event(sint32 idx);
uint32 actor_attachment_particle_event0(sint32 idx);
uint32 actor_attachment_particle_event1(sint32 idx);
intptr actor_burst_attachment_event(sint32 idx);
sint32 actor_release_item_event(sint32 idx);

#endif
