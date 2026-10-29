#ifndef STAGE_ANIMATION_H
#define STAGE_ANIMATION_H

#include "xport.h"
#include "object_callbacks.h"

enum
{
    STAGE_ANIMATION_PHASE_COUNT = 2
};

typedef struct STAGE_ANIMATION
{
    uint32 phase[STAGE_ANIMATION_PHASE_COUNT];
    uint32 hold_ticks;
    /* TODO: Identify any consumer of the original stored sample130 voice */
    uint16 stage4_voice;
    uint8 lift_auto_cycle;
} STAGE_ANIMATION;

extern STAGE_ANIMATION stage_animation;

void stage_animation_update_stage14(void);
uint32 stage_animation_update_stage4(void);
uint32 stage_animation_scroll_stage0(void);
uint32 stage_animation_scroll_stage1(void);
uint32 stage_animation_scroll_lift(void);
uint32 stage_animation_scroll_stage12(void);
sint32 stage_animation_scroll_stage14(void);
sint32 stage_animation_scroll_stage17(void);

extern const OBJECT_CALLBACK callback_stage_animation_slide_x;
extern const OBJECT_CALLBACK callback_stage_animation_slide_z;
extern const OBJECT_CALLBACK callback_stage_animation_double_slide_z;

#endif
