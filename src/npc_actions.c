#include "game.h"

NPC_ACTION_REFERENCE npc_actions[NPC_ACTION_BANK_COUNT][NPC_ACTION_SLOT_COUNT];
NPC_ACTION_REFERENCE npc_updates[NPC_BEHAVIOR_TYPE_COUNT];
NPC_ACTION_BANK_REFERENCE npc_action_actors[NPC_BEHAVIOR_ACTOR_COUNT];

const NPC_ACTION npc_action_update_stage4_idle = {NPC_ACTION_INTPTR, {.intptr_idx = npc_update_stage4_idle}, 0};

const NPC_ACTION npc_action_update_stage4_departure = {NPC_ACTION_INTPTR, {.intptr_idx = npc_update_stage4_departure}, 0};

const NPC_ACTION npc_action_van = {NPC_ACTION_UINT32, {.u32_idx = npc_update_van}, 0};

const NPC_ACTION npc_action_update_action = {NPC_ACTION_INTPTR, {.intptr_idx = npc_update_action}, 0};

const NPC_ACTION npc_action_finish_to_idle = {NPC_ACTION_SINT32, {.s32_idx = npc_finish_to_idle}, 0};

const NPC_ACTION npc_action_approach_target = {NPC_ACTION_SINT32, {.s32_idx = npc_approach_target}, 0};

const NPC_ACTION npc_action_resume_approach0 = {NPC_ACTION_SINT32, {.s32_idx = npc_resume_approach0}, 0};

const NPC_ACTION npc_action_resume_approach1 = {NPC_ACTION_SINT32, {.s32_idx = npc_resume_approach1}, 0};

const NPC_ACTION npc_action_finish_grapple = {NPC_ACTION_SINT32, {.s32_idx = npc_finish_grapple}, 0};

const NPC_ACTION npc_action_begin_captive_carry = {NPC_ACTION_SINT32, {.s32_idx = npc_begin_captive_carry}, 0};

const NPC_ACTION npc_action_carry_captive_to_point = {NPC_ACTION_SINT32, {.s32_idx = npc_carry_captive_to_point}, 0};

static const NPC_ACTION npc_action_move_oscillating = {NPC_ACTION_SINT32, {.s32_idx = npc_move_oscillating}, 0};

const NPC_ACTION npc_action_begin_attack_wait = {NPC_ACTION_SINT32, {.s32_idx = npc_begin_attack_wait}, 0};

const NPC_ACTION npc_action_resume_combat0 = {NPC_ACTION_SINT32, {.s32_idx = npc_resume_combat0}, 0};

const NPC_ACTION npc_action_update_attack_motion = {NPC_ACTION_SINT32, {.s32_idx = npc_update_attack_motion}, 0};

const NPC_ACTION npc_action_emit_attack_effects = {NPC_ACTION_SINT32, {.s32_idx = npc_emit_attack_effects}, 0};

static const NPC_ACTION npc_action_attack_electric = {NPC_ACTION_SINT32, {.s32_idx = npc_attack_electric}, 0};

const NPC_ACTION npc_action_update_jetpack_sound = {NPC_ACTION_INTPTR, {.intptr_idx = npc_update_jetpack_sound}, 0};

const NPC_ACTION npc_action_update_vulkan = {NPC_ACTION_INTPTR, {.intptr_idx = npc_update_vulkan}, 0};

const NPC_ACTION npc_action_resume_alternate_combat5 = {NPC_ACTION_SINT32, {.s32_idx = npc_resume_alternate_combat5}, 0};

static const NPC_ACTION npc_action_update_delay = {NPC_ACTION_SINT32, {.s32_idx = npc_update_delay}, 0};

const NPC_ACTION npc_action_finish_vulkan_combo = {NPC_ACTION_SINT32, {.s32_idx = npc_finish_vulkan_combo}, 0};

const NPC_ACTION npc_action_finish_aimed_charge = {NPC_ACTION_SINT32, {.s32_idx = npc_finish_aimed_charge}, 0};

const NPC_ACTION npc_action_attack_beams = {NPC_ACTION_SINT32, {.s32_idx = npc_attack_beams}, 0};

const NPC_ACTION npc_action_resume_alternate_combat0 = {NPC_ACTION_SINT32, {.s32_idx = npc_resume_alternate_combat0}, 0};

const NPC_ACTION npc_action_resume_alternate_combat1 = {NPC_ACTION_SINT32, {.s32_idx = npc_resume_alternate_combat1}, 0};

const NPC_ACTION npc_action_begin_charge_loop = {NPC_ACTION_SINT32, {.s32_idx = npc_begin_charge_loop}, 0};

const NPC_ACTION npc_action_finish_charge_start = {NPC_ACTION_SINT32, {.s32_idx = npc_finish_charge_start}, 0};

const NPC_ACTION npc_action_advance_charge = {NPC_ACTION_SINT32, {.s32_idx = npc_advance_charge}, 0};

const NPC_ACTION npc_action_resume_alternate_combat2 = {NPC_ACTION_SINT32, {.s32_idx = npc_resume_alternate_combat2}, 0};

const NPC_ACTION npc_action_retained = {NPC_ACTION_NONE, {.none = npc_ignore_action}, (sint32)0x8004ef64};

const NPC_ACTION npc_action_resume_alternate_combat3 = {NPC_ACTION_SINT32, {.s32_idx = npc_resume_alternate_combat3}, 0};

const NPC_ACTION npc_action_animation_end = {NPC_ACTION_UINT32, {.u32_idx = npc_finish_animation}, 0};

const NPC_ACTION npc_action_continue_alternate_combat = {NPC_ACTION_SINT32, {.s32_idx = npc_continue_alternate_combat}, 0};

static const NPC_ACTION npc_action_resume_alternate_combat4 = {NPC_ACTION_SINT32, {.s32_idx = npc_resume_alternate_combat4}, 0};

const NPC_ACTION npc_action_update_alternate_combat = {NPC_ACTION_SINT32, {.s32_idx = npc_update_alternate_combat}, 0};

const NPC_ACTION npc_action_update_actor = {NPC_ACTION_INTPTR, {.intptr_idx = npc_update_actor}, 0};

const NPC_ACTION npc_action_wait_captor_release = {NPC_ACTION_SINT32, {.s32_idx = npc_wait_captor_release}, 0};

const NPC_ACTION npc_action_resume_combat6 = {NPC_ACTION_SINT32, {.s32_idx = npc_resume_combat6}, 0};

const NPC_ACTION npc_action_continue_combat = {NPC_ACTION_SINT32, {.s32_idx = npc_continue_combat}, 0};

const NPC_ACTION npc_action_attack_continue = {NPC_ACTION_SINT32, {.s32_idx = npc_continue_attack_behavior}, 0};

const NPC_ACTION npc_action_update_combat = {NPC_ACTION_SINT32, {.s32_idx = npc_update_combat}, 0};

const NPC_ACTION npc_action_fire_spread_and_wait = {NPC_ACTION_SINT32, {.s32_idx = npc_fire_spread_and_wait}, 0};

const NPC_ACTION npc_action_claim_right_item0 = {NPC_ACTION_SINT32, {.s32_idx = npc_claim_right_item0}, 0};

const NPC_ACTION npc_action_choose_held_item_action = {NPC_ACTION_SINT32, {.s32_idx = npc_choose_held_item_action}, 0};

const NPC_ACTION npc_action_claim_right_item1 = {NPC_ACTION_SINT32, {.s32_idx = npc_claim_right_item1}, 0};

const NPC_ACTION npc_action_claim_left_item = {NPC_ACTION_SINT32, {.s32_idx = npc_claim_left_item}, 0};

const NPC_ACTION npc_action_resume_combat1 = {NPC_ACTION_SINT32, {.s32_idx = npc_resume_combat1}, 0};

const NPC_ACTION npc_action_claim_ranged_item = {NPC_ACTION_SINT32, {.s32_idx = npc_claim_ranged_item}, 0};

const NPC_ACTION npc_action_resume_combat2 = {NPC_ACTION_SINT32, {.s32_idx = npc_resume_combat2}, 0};

const NPC_ACTION npc_action_detached = {NPC_ACTION_SINT32, {.s32_idx = npc_update_detached}, 0};

const NPC_ACTION npc_action_update_knockdown_pose = {NPC_ACTION_SINT32, {.s32_idx = npc_update_knockdown_pose}, 0};

const NPC_ACTION npc_action_update_knockdown = {NPC_ACTION_SINT32, {.s32_idx = npc_update_knockdown}, 0};

const NPC_ACTION npc_action_wait_recovery_deadline = {NPC_ACTION_SINT32, {.s32_idx = npc_wait_recovery_deadline}, 0};

const NPC_ACTION npc_action_finish_recovery_turn = {NPC_ACTION_SINT32, {.s32_idx = npc_finish_recovery_turn}, 0};

const NPC_ACTION npc_action_finish_facing_target = {NPC_ACTION_SINT32, {.s32_idx = npc_finish_facing_target}, 0};

const NPC_ACTION npc_action_release_captive = {NPC_ACTION_SINT32, {.s32_idx = npc_release_captive}, 0};

const NPC_ACTION npc_action_finish_captive_hold0 = {NPC_ACTION_SINT32, {.s32_idx = npc_finish_captive_hold0}, 0};

const NPC_ACTION npc_action_finish_captive_strike = {NPC_ACTION_SINT32, {.s32_idx = npc_finish_captive_strike}, 0};

const NPC_ACTION npc_action_finish_captive_hold1 = {NPC_ACTION_SINT32, {.s32_idx = npc_finish_captive_hold1}, 0};

const NPC_ACTION npc_action_finish_turn = {NPC_ACTION_SINT32, {.s32_idx = npc_finish_turn}, 0};

const NPC_ACTION npc_action_finish_attack_lighting = {NPC_ACTION_SINT32, {.s32_idx = npc_finish_attack_lighting}, 0};

const NPC_ACTION npc_action_finish_combat_combo = {NPC_ACTION_SINT32, {.s32_idx = npc_finish_combat_combo}, 0};

const NPC_ACTION npc_action_wait_target_animation = {NPC_ACTION_SINT32, {.s32_idx = npc_wait_target_animation}, 0};

const NPC_ACTION npc_action_choose_animation_variant = {NPC_ACTION_SINT32, {.s32_idx = npc_choose_animation_variant}, 0};

const NPC_ACTION npc_action_resume_combat3 = {NPC_ACTION_SINT32, {.s32_idx = npc_resume_combat3}, 0};

const NPC_ACTION npc_action_update_fall = {NPC_ACTION_SINT32, {.s32_idx = npc_update_fall}, 0};

const NPC_ACTION npc_action_resume_combat4 = {NPC_ACTION_SINT32, {.s32_idx = npc_resume_combat4}, 0};

const NPC_ACTION npc_action_set_stun_recovery_deadline = {NPC_ACTION_SINT32, {.s32_idx = npc_set_stun_recovery_deadline}, 0};

const NPC_ACTION npc_action_throw_items = {NPC_ACTION_SINT32, {.s32_idx = npc_throw_items}, 0};

const NPC_ACTION npc_action_update_recovery = {NPC_ACTION_SINT32, {.s32_idx = npc_update_recovery}, 0};

const NPC_ACTION npc_action_resume_combat5 = {NPC_ACTION_SINT32, {.s32_idx = npc_resume_combat5}, 0};

const NPC_ACTION npc_action_bike = {NPC_ACTION_INTPTR, {.intptr_idx = npc_update_bike}, 0};

const NPC_ACTION npc_action_companion_wait = {NPC_ACTION_INTPTR, {.intptr_idx = npc_update_companion_wait}, 0};


GDB_CALL void npc_ignore_action(void)
{
    FUNCTION_MARKER(0x8004ef64u, "GAME.EXE");
}
