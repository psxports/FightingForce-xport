#ifndef NPC_ACTIONS_H
#define NPC_ACTIONS_H

#include "xport.h"
#include "npc_behavior.h"

enum
{
    NPC_ACTION_SLOT_COUNT = 289,
    NPC_ACTION_COUNT = 75
};

typedef enum NPC_ACTION_BANK
{
    NPC_ACTION_DEFAULT,
    NPC_ACTION_TYPE28,
    NPC_ACTION_TYPE32,
    NPC_ACTION_JETPACK,
    NPC_ACTION_VULKAN,
    NPC_ACTION_BANK_COUNT
} NPC_ACTION_BANK;

typedef enum NPC_ACTION_SIGNATURE
{
    NPC_ACTION_SINT32,
    NPC_ACTION_UINT32,
    NPC_ACTION_NONE,
    NPC_ACTION_INTPTR
} NPC_ACTION_SIGNATURE;

typedef struct NPC_ACTION
{
    NPC_ACTION_SIGNATURE signature;

    union
    {
        sint32 (*s32_idx)(sint32);
        uint32 (*u32_idx)(sint32);
        void (*none)(void);
        intptr (*intptr_idx)(sint32);
    } fn;

    sint32 retained_result;
} NPC_ACTION;

typedef const NPC_ACTION *NPC_ACTION_REFERENCE;

typedef struct NPC_ACTION_BANK_REFERENCE
{
    NPC_ACTION_REFERENCE *actions;
    uint32 first;
} NPC_ACTION_BANK_REFERENCE;

extern NPC_ACTION_REFERENCE npc_actions[NPC_ACTION_BANK_COUNT][NPC_ACTION_SLOT_COUNT];
extern NPC_ACTION_REFERENCE npc_updates[NPC_BEHAVIOR_TYPE_COUNT];
extern NPC_ACTION_BANK_REFERENCE npc_action_actors[NPC_BEHAVIOR_ACTOR_COUNT];
extern const NPC_ACTION npc_action_update_stage4_idle;
extern const NPC_ACTION npc_action_update_stage4_departure;
extern const NPC_ACTION npc_action_van;
extern const NPC_ACTION npc_action_update_action;
extern const NPC_ACTION npc_action_finish_to_idle;
extern const NPC_ACTION npc_action_approach_target;
extern const NPC_ACTION npc_action_resume_approach0;
extern const NPC_ACTION npc_action_resume_approach1;
extern const NPC_ACTION npc_action_finish_grapple;
extern const NPC_ACTION npc_action_begin_captive_carry;
extern const NPC_ACTION npc_action_carry_captive_to_point;
extern const NPC_ACTION npc_action_begin_attack_wait;
extern const NPC_ACTION npc_action_resume_combat0;
extern const NPC_ACTION npc_action_update_attack_motion;
extern const NPC_ACTION npc_action_emit_attack_effects;
extern const NPC_ACTION npc_action_update_jetpack_sound;
extern const NPC_ACTION npc_action_update_vulkan;
extern const NPC_ACTION npc_action_resume_alternate_combat5;
extern const NPC_ACTION npc_action_finish_vulkan_combo;
extern const NPC_ACTION npc_action_finish_aimed_charge;
extern const NPC_ACTION npc_action_attack_beams;
extern const NPC_ACTION npc_action_resume_alternate_combat0;
extern const NPC_ACTION npc_action_resume_alternate_combat1;
extern const NPC_ACTION npc_action_begin_charge_loop;
extern const NPC_ACTION npc_action_finish_charge_start;
extern const NPC_ACTION npc_action_advance_charge;
extern const NPC_ACTION npc_action_resume_alternate_combat2;
extern const NPC_ACTION npc_action_retained;
extern const NPC_ACTION npc_action_resume_alternate_combat3;
extern const NPC_ACTION npc_action_animation_end;
extern const NPC_ACTION npc_action_continue_alternate_combat;
extern const NPC_ACTION npc_action_update_alternate_combat;
extern const NPC_ACTION npc_action_update_actor;
extern const NPC_ACTION npc_action_wait_captor_release;
extern const NPC_ACTION npc_action_resume_combat6;
extern const NPC_ACTION npc_action_continue_combat;
extern const NPC_ACTION npc_action_attack_continue;
extern const NPC_ACTION npc_action_update_combat;
extern const NPC_ACTION npc_action_fire_spread_and_wait;
extern const NPC_ACTION npc_action_claim_right_item0;
extern const NPC_ACTION npc_action_choose_held_item_action;
extern const NPC_ACTION npc_action_claim_right_item1;
extern const NPC_ACTION npc_action_claim_left_item;
extern const NPC_ACTION npc_action_resume_combat1;
extern const NPC_ACTION npc_action_claim_ranged_item;
extern const NPC_ACTION npc_action_resume_combat2;
extern const NPC_ACTION npc_action_detached;
extern const NPC_ACTION npc_action_update_knockdown_pose;
extern const NPC_ACTION npc_action_update_knockdown;
extern const NPC_ACTION npc_action_wait_recovery_deadline;
extern const NPC_ACTION npc_action_finish_recovery_turn;
extern const NPC_ACTION npc_action_finish_facing_target;
extern const NPC_ACTION npc_action_release_captive;
extern const NPC_ACTION npc_action_finish_captive_hold0;
extern const NPC_ACTION npc_action_finish_captive_strike;
extern const NPC_ACTION npc_action_finish_captive_hold1;
extern const NPC_ACTION npc_action_finish_turn;
extern const NPC_ACTION npc_action_finish_attack_lighting;
extern const NPC_ACTION npc_action_finish_combat_combo;
extern const NPC_ACTION npc_action_wait_target_animation;
extern const NPC_ACTION npc_action_choose_animation_variant;
extern const NPC_ACTION npc_action_resume_combat3;
extern const NPC_ACTION npc_action_update_fall;
extern const NPC_ACTION npc_action_resume_combat4;
extern const NPC_ACTION npc_action_set_stun_recovery_deadline;
extern const NPC_ACTION npc_action_throw_items;
extern const NPC_ACTION npc_action_update_recovery;
extern const NPC_ACTION npc_action_resume_combat5;
extern const NPC_ACTION npc_action_bike;
extern const NPC_ACTION npc_action_companion_wait;
extern const NPC_ACTION npc_action_lab_tank_wait;

GDB_CALL uint32 npc_update_van(sint32 idx);
GDB_CALL intptr npc_update_bike(sint32 idx);
GDB_CALL intptr npc_update_companion_wait(sint32 idx);

GDB_CALL sint32 npc_start_jump_off(sint32 idx, sint32 mode);

GDB_CALL uint32 npc_finish_animation(sint32 idx);

GDB_CALL sint32 npc_continue_attack_behavior(sint32 idx);

GDB_CALL intptr npc_update_vulkan(sint32 idx);

GDB_CALL intptr npc_update_stage4_departure(sint32 idx);

GDB_CALL sint32 npc_release_captive(sint32 idx);

GDB_CALL sint32 npc_carry_captive_to_point(sint32 idx);

GDB_CALL sint32 npc_advance_charge(sint32 idx);

GDB_CALL intptr npc_update_jetpack_sound(sint32 idx);

GDB_CALL sint32 npc_finish_aimed_charge(sint32 idx);

GDB_CALL sint32 npc_wait_target_animation(sint32 idx);

GDB_CALL sint32 npc_claim_ranged_item(sint32 idx);

GDB_CALL sint32 npc_begin_captive_carry(sint32 idx);

GDB_CALL sint32 npc_finish_vulkan_combo(sint32 idx);

GDB_CALL intptr npc_update_stage4_idle(sint32 idx);

GDB_CALL sint32 npc_finish_turn(sint32 idx);

GDB_CALL sint32 npc_finish_captive_strike(sint32 idx);

GDB_CALL sint32 npc_choose_held_item_action(sint32 idx);

GDB_CALL sint32 npc_claim_right_item0(sint32 idx);

GDB_CALL sint32 npc_claim_right_item1(sint32 idx);

GDB_CALL sint32 npc_claim_left_item(sint32 idx);

GDB_CALL sint32 npc_wait_captor_release(sint32 idx);

GDB_CALL sint32 npc_finish_to_idle(sint32 idx);

GDB_CALL sint32 npc_finish_charge_start(sint32 idx);

GDB_CALL sint32 npc_begin_charge_loop(sint32 idx);

GDB_CALL sint32 npc_update_knockdown_pose(sint32 idx);

GDB_CALL sint32 npc_finish_captive_hold0(sint32 idx);

GDB_CALL sint32 npc_finish_captive_hold1(sint32 idx);

GDB_CALL sint32 npc_choose_animation_variant(sint32 idx);

GDB_CALL sint32 npc_set_stun_recovery_deadline(sint32 idx);

GDB_CALL sint32 npc_resume_approach0(sint32 idx);

GDB_CALL sint32 npc_resume_approach1(sint32 idx);

GDB_CALL sint32 npc_resume_combat0(sint32 idx);

GDB_CALL sint32 npc_resume_alternate_combat5(sint32 idx);

GDB_CALL sint32 npc_resume_alternate_combat0(sint32 idx);

GDB_CALL sint32 npc_resume_alternate_combat1(sint32 idx);

GDB_CALL sint32 npc_resume_alternate_combat2(sint32 idx);

GDB_CALL sint32 npc_resume_alternate_combat3(sint32 idx);

GDB_CALL sint32 npc_resume_alternate_combat4(sint32 idx);

GDB_CALL sint32 npc_resume_combat1(sint32 idx);

GDB_CALL sint32 npc_resume_combat2(sint32 idx);

GDB_CALL sint32 npc_resume_combat3(sint32 idx);

GDB_CALL sint32 npc_resume_combat4(sint32 idx);

GDB_CALL sint32 npc_resume_combat5(sint32 idx);

GDB_CALL sint32 npc_update_alternate_combat(sint32 idx);

GDB_CALL sint32 npc_continue_alternate_combat(sint32 idx);

GDB_CALL sint32 npc_finish_recovery_turn(sint32 idx);

GDB_CALL sint32 npc_wait_recovery_deadline(sint32 idx);

GDB_CALL sint32 npc_fire_spread_and_wait(sint32 idx);

GDB_CALL sint32 npc_finish_facing_target(sint32 idx);

GDB_CALL sint32 npc_update_detached(sint32 idx);

GDB_CALL sint32 npc_update_knockdown(sint32 idx);

GDB_CALL sint32 npc_update_recovery(sint32 idx);

GDB_CALL sint32 npc_finish_attack_lighting(sint32 idx);

GDB_CALL sint32 npc_update_combat(sint32 idx);

GDB_CALL sint32 npc_continue_combat(sint32 idx);

GDB_CALL sint32 npc_finish_combat_combo(sint32 idx);

GDB_CALL sint32 npc_animation_allows_target_reaction(uint32 state);

GDB_CALL sint32 npc_target_reaction_class(uint32 state);

GDB_CALL sint32 npc_resume_combat6(sint32 idx);

GDB_CALL void npc_ignore_action(void);

#endif
