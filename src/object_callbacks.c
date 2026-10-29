#include <stdio.h>

#include "game.h"

const OBJECT_CALLBACK *object_callbacks[OBJECT_CALLBACK_TABLE_COUNT][OBJECT_CALLBACK_SLOT_COUNT];

/* Two original initializers also write the word after the last model */
const uint32 object_callback_counts[OBJECT_CALLBACK_TABLE_COUNT] = {MODEL_TYPE_COUNT + 1, MODEL_TYPE_COUNT, MODEL_TYPE_COUNT + 1, MODEL_TYPE_COUNT, MODEL_TYPE_COUNT};

const OBJECT_CALLBACK callback_object_draw_phase_yaw_variant = {OBJECT_CALL_U32_NONE, {.u32_none = object_draw_phase_yaw_variant}, 0};

const OBJECT_CALLBACK callback_object_draw_flag_256 = {OBJECT_CALL_U32_MODEL, {.u32_model = object_draw_flag_256}, 0};

const OBJECT_CALLBACK callback_object_draw_camera_yaw_inner = {OBJECT_CALL_U32_MODEL, {.u32_model = object_draw_camera_yaw_inner}, 0};

const OBJECT_CALLBACK callback_object_draw_rotated = {OBJECT_CALL_U32_MODEL, {.u32_model = object_draw_rotated}, 0};

const OBJECT_CALLBACK callback_helicopter_update_scene = {OBJECT_CALL_U32_MUTABLE_MODEL, {.u32_mutable_model = helicopter_update_scene}, 0};

const OBJECT_CALLBACK callback_helicopter_draw_scene = {OBJECT_CALL_VOID_MODEL, {.no_result_model = helicopter_draw_scene}, 0};

const OBJECT_CALLBACK callback_world_objects_draw_tank = {OBJECT_CALL_U32_MODEL, {.u32_model = world_objects_draw_tank}, 0};

const OBJECT_CALLBACK callback_object_draw_camera_x_gated_fixed_model = {OBJECT_CALL_U32_NONE, {.u32_none = object_draw_camera_x_gated_fixed_model}, 0};

const OBJECT_CALLBACK callback_object_draw_fixed_pair = {OBJECT_CALL_U32_NONE, {.u32_none = object_draw_fixed_pair}, 0};

const OBJECT_CALLBACK callback_object_draw_side_spin = {OBJECT_CALL_U32_MODEL, {.u32_model = object_draw_side_spin}, 0};

const OBJECT_CALLBACK callback_object_ignore_callback0 = {OBJECT_CALL_VOID_NONE, {.no_result_none = object_ignore_callback0}, 0};

const OBJECT_CALLBACK callback_object_draw_raised_spinning_pair = {OBJECT_CALL_VOID_MODEL, {.no_result_model = object_draw_raised_spinning_pair}, 0};

const OBJECT_CALLBACK callback_object_draw_height_offset = {OBJECT_CALL_VOID_MODEL, {.no_result_model = object_draw_height_offset}, 0};

const OBJECT_CALLBACK callback_object_draw_wave = {OBJECT_CALL_U32_MUTABLE_MODEL, {.u32_mutable_model = object_draw_wave}, 0};

const OBJECT_CALLBACK callback_effect_draw_glass = {OBJECT_CALL_VOID_EFFECT, {.no_result_effect = effect_draw_glass}, 0};

const OBJECT_CALLBACK callback_object_update_rising_spr = {OBJECT_CALL_U32_MUTABLE_MODEL, {.u32_mutable_model = object_update_rising_spr}, 0};

const OBJECT_CALLBACK callback_effect_update_rising_spr_short = {OBJECT_CALL_U32_MODEL_EFFECT, {.u32_model_effect = effect_update_rising_spr_short}, 0};

const OBJECT_CALLBACK callback_effect_update_rising_spr_long = {OBJECT_CALL_U32_MODEL_EFFECT, {.u32_model_effect = effect_update_rising_spr_long}, 0};

const OBJECT_CALLBACK callback_effect_update_dust_short = {OBJECT_CALL_U32_MODEL_EFFECT, {.u32_model_effect = effect_update_dust_short}, 0};

const OBJECT_CALLBACK callback_effect_update_dust_long = {OBJECT_CALL_U32_EFFECT, {.u32_effect = effect_update_dust_long}, 0};

const OBJECT_CALLBACK callback_effect_update_blast_large = {OBJECT_CALL_U32_EFFECT, {.u32_effect = effect_update_blast_large}, 0};

const OBJECT_CALLBACK callback_effect_update_blast = {OBJECT_CALL_U32_EFFECT, {.u32_effect = effect_update_blast}, 0};

const OBJECT_CALLBACK callback_effect_update_ring_large = {OBJECT_CALL_U32_EFFECT, {.u32_effect = effect_update_ring_large}, 0};

const OBJECT_CALLBACK callback_effect_update_ring = {OBJECT_CALL_U32_EFFECT, {.u32_effect = effect_update_ring}, 0};

const OBJECT_CALLBACK callback_effect_update_shockwave = {OBJECT_CALL_S32_EFFECT, {.s32_effect = effect_update_shockwave}, 0};

const OBJECT_CALLBACK callback_effect_update_ring_short = {OBJECT_CALL_U32_EFFECT, {.u32_effect = effect_update_ring_short}, 0};

const OBJECT_CALLBACK callback_effect_update_rising_smoke = {OBJECT_CALL_U32_MODEL_EFFECT, {.u32_model_effect = effect_update_rising_smoke}, 0};

const OBJECT_CALLBACK callback_object_update_oscillating_spr = {OBJECT_CALL_U32_MUTABLE_MODEL, {.u32_mutable_model = object_update_oscillating_spr}, 0};

const OBJECT_CALLBACK callback_object_update_impact_spr = {OBJECT_CALL_U32_MUTABLE_MODEL, {.u32_mutable_model = object_update_impact_spr}, 0};

const OBJECT_CALLBACK callback_object_update_expiring = {OBJECT_CALL_U32_MUTABLE_MODEL, {.u32_mutable_model = object_update_expiring}, 0};

const OBJECT_CALLBACK callback_object_update_physics = {OBJECT_CALL_S32_MUTABLE_MODEL, {.s32_mutable_model = object_update_physics}, 0};

const OBJECT_CALLBACK callback_object_update_physics_alias = {OBJECT_CALL_S32_MUTABLE_MODEL, {.s32_mutable_model = object_update_physics}, 0};

const OBJECT_CALLBACK callback_object_update_explosive = {OBJECT_CALL_S32_MUTABLE_MODEL, {.s32_mutable_model = object_update_explosive}, 0};

const OBJECT_CALLBACK callback_effect_update_particle = {OBJECT_CALL_S32_EFFECT, {.s32_effect = effect_update_particle}, 0};

const OBJECT_CALLBACK callback_effect_update_falling_glass = {OBJECT_CALL_U32_EFFECT, {.u32_effect = effect_update_falling_glass}, 0};

const OBJECT_CALLBACK callback_effect_update_slowing_particle = {OBJECT_CALL_U32_EFFECT, {.u32_effect = effect_update_slowing_particle}, 0};

const OBJECT_CALLBACK callback_effect_update_short_particle = {OBJECT_CALL_U32_EFFECT, {.u32_effect = effect_update_short_particle}, 0};

static const OBJECT_CALLBACK callback_effect_remove_short_particle = {OBJECT_CALL_U32_EFFECT, {.u32_effect = effect_remove_short_particle}, 0};

const OBJECT_CALLBACK callback_effect_update_long_particle = {OBJECT_CALL_U32_EFFECT, {.u32_effect = effect_update_long_particle}, 0};

static const OBJECT_CALLBACK callback_effect_remove_long_particle = {OBJECT_CALL_U32_EFFECT, {.u32_effect = effect_remove_long_particle}, 0};

const OBJECT_CALLBACK callback_object_draw_scene_variant = {OBJECT_CALL_U32_MODEL, {.u32_model = object_draw_scene_variant}, 0};

const OBJECT_CALLBACK callback_object_update_ballistic_spin = {OBJECT_CALL_S32_MUTABLE_MODEL, {.s32_mutable_model = object_update_ballistic_spin}, 0};

const OBJECT_CALLBACK callback_object_update_drifting_spin = {OBJECT_CALL_U32_MUTABLE_MODEL, {.u32_mutable_model = object_update_drifting_spin}, 0};

const OBJECT_CALLBACK callback_object_draw_depth_bob = {OBJECT_CALL_VOID_MODEL, {.no_result_model = object_draw_depth_bob}, 0};

const OBJECT_CALLBACK callback_helicopter_update_intro = {OBJECT_CALL_U32_MUTABLE_MODEL, {.u32_mutable_model = helicopter_update_intro}, 0};

const OBJECT_CALLBACK callback_helicopter_draw_intro = {OBJECT_CALL_VOID_MODEL, {.no_result_model = helicopter_draw_intro}, 0};

const OBJECT_CALLBACK callback_effect_update_delayed_fragment = {OBJECT_CALL_U32_EFFECT, {.u32_effect = effect_update_delayed_fragment}, 0};

const OBJECT_CALLBACK callback_effect_update_gate_fragment = {OBJECT_CALL_U32_EFFECT, {.u32_effect = effect_update_gate_fragment}, 0};

const OBJECT_CALLBACK callback_gate_shake = {OBJECT_CALL_U32_MUTABLE_MODEL, {.u32_mutable_model = object_update_gate_shake}, 0};

const OBJECT_CALLBACK callback_gate_panels_shake = {OBJECT_CALL_U32_MUTABLE_MODEL, {.u32_mutable_model = object_update_gate_panels_shake}, 0};

const OBJECT_CALLBACK callback_object_update_falling_panel = {OBJECT_CALL_S32_MUTABLE_MODEL, {.s32_mutable_model = object_update_falling_panel}, 0};

const OBJECT_CALLBACK callback_panel_shake = {OBJECT_CALL_U32_MUTABLE_MODEL, {.u32_mutable_model = object_update_shake}, 0};

const OBJECT_CALLBACK callback_object_update_falling_panel_sideways = {OBJECT_CALL_U32_MUTABLE_MODEL, {.u32_mutable_model = object_update_falling_panel_sideways}, 0};

const OBJECT_CALLBACK callback_object_update_wobble = {OBJECT_CALL_S32_MUTABLE_MODEL, {.s32_mutable_model = object_update_wobble}, 0};

const OBJECT_CALLBACK callback_object_update_wobble_debris = {OBJECT_CALL_INTPTR_MUTABLE_MODEL, {.intptr_mutable_model = object_update_wobble_debris}, 0};

const OBJECT_CALLBACK callback_falling_gate = {OBJECT_CALL_U32_MUTABLE_MODEL, {.u32_mutable_model = object_update_falling_gate}, 0};

const OBJECT_CALLBACK callback_gate_fragment = {OBJECT_CALL_S32_MUTABLE_MODEL, {.s32_mutable_model = object_update_gate_fragment}, 0};

const OBJECT_CALLBACK callback_object_update_oscillation = {OBJECT_CALL_U32_MUTABLE_MODEL, {.u32_mutable_model = object_update_oscillation}, 0};

const OBJECT_CALLBACK callback_object_update_compression = {OBJECT_CALL_U32_MUTABLE_MODEL, {.u32_mutable_model = object_update_compression}, 0};

const OBJECT_CALLBACK callback_dynamic_objects_settle_debris = {OBJECT_CALL_VOID_MUTABLE_MODEL, {.no_result_mutable_model = dynamic_objects_settle_debris}, 0};

const OBJECT_CALLBACK callback_ground_bounce = {OBJECT_CALL_U32_MUTABLE_MODEL, {.u32_mutable_model = object_update_ground_bounce}, 0};

const OBJECT_CALLBACK callback_spinning_landing = {OBJECT_CALL_U32_MUTABLE_MODEL, {.u32_mutable_model = object_update_spinning_landing}, 0};

const OBJECT_CALLBACK callback_sliding_bounce = {OBJECT_CALL_U32_MUTABLE_MODEL, {.u32_mutable_model = object_update_sliding_bounce}, 0};

const OBJECT_CALLBACK callback_effect_update_bouncing_particle = {OBJECT_CALL_S32_MODEL_EFFECT, {.s32_model_effect = effect_update_bouncing_particle}, 0};

const OBJECT_CALLBACK callback_world_objects_update_falling_door = {OBJECT_CALL_S32_MUTABLE_MODEL, {.s32_mutable_model = world_objects_update_falling_door}, 0};

const OBJECT_CALLBACK callback_world_objects_update_bus_wreck = {OBJECT_CALL_U32_MUTABLE_MODEL, {.u32_mutable_model = world_objects_update_bus_wreck}, 0};

const OBJECT_CALLBACK callback_world_objects_update_scrolling_wall = {OBJECT_CALL_U32_MUTABLE_MODEL, {.u32_mutable_model = world_objects_update_scrolling_wall}, 0};

const OBJECT_CALLBACK callback_world_objects_bob = {OBJECT_CALL_U32_MUTABLE_MODEL, {.u32_mutable_model = world_objects_bob}, 0};

const OBJECT_CALLBACK callback_world_objects_update_radar = {OBJECT_CALL_U32_MUTABLE_MODEL, {.u32_mutable_model = world_objects_update_radar}, 0};

const OBJECT_CALLBACK callback_object_update_wobble_particles = {OBJECT_CALL_S32_MUTABLE_MODEL, {.s32_mutable_model = object_update_wobble_particles}, 0};

const OBJECT_CALLBACK callback_object_update_smoke_emitter = {OBJECT_CALL_INTPTR_MUTABLE_MODEL, {.intptr_mutable_model = object_update_smoke_emitter}, 0};

const OBJECT_CALLBACK callback_object_update_yaw = {OBJECT_CALL_U32_MUTABLE_MODEL, {.u32_mutable_model = object_update_yaw}, 0};

const OBJECT_CALLBACK callback_object_update_island_fragment = {OBJECT_CALL_S32_MUTABLE_MODEL, {.s32_mutable_model = object_update_island_fragment}, 0};

const OBJECT_CALLBACK callback_air_bike_update_approach = {OBJECT_CALL_S32_MUTABLE_MODEL, {.s32_mutable_model = air_bike_update_approach}, 0};

const OBJECT_CALLBACK callback_air_bike_update_departure = {OBJECT_CALL_S32_MUTABLE_MODEL, {.s32_mutable_model = air_bike_update_departure}, 0};

const OBJECT_CALLBACK callback_world_objects_update_traffic = {OBJECT_CALL_U32_MUTABLE_MODEL, {.u32_mutable_model = world_objects_update_traffic}, 0};

const OBJECT_CALLBACK callback_effect_update_debris_low_gravity = {OBJECT_CALL_INTPTR_EFFECT, {.intptr_effect = effect_update_debris_low_gravity}, 0};

const OBJECT_CALLBACK callback_effect_update_debris = {OBJECT_CALL_INTPTR_EFFECT, {.intptr_effect = effect_update_debris}, 0};

const OBJECT_CALLBACK callback_effect_update_debris_quarter_speed = {OBJECT_CALL_INTPTR_EFFECT, {.intptr_effect = effect_update_debris_quarter_speed}, 0};

const OBJECT_CALLBACK callback_projectile_update_accelerating = {OBJECT_CALL_INTPTR_MUTABLE_MODEL, {.intptr_mutable_model = projectile_update_accelerating}, 0};

static const OBJECT_CALLBACK callback_model_update_spinning = {OBJECT_CALL_S32_MUTABLE_MODEL, {.s32_mutable_model = model_update_spinning}, 0};

const OBJECT_CALLBACK callback_object_draw_yaw_flag_128 = {OBJECT_CALL_U32_MODEL, {.u32_model = object_draw_yaw_flag_128}, 0};

const OBJECT_CALLBACK callback_object_draw_camera_yaw_outer = {OBJECT_CALL_U32_MODEL, {.u32_model = object_draw_camera_yaw_outer}, 0};

const OBJECT_CALLBACK callback_object_draw_kind_bias = {OBJECT_CALL_U32_MODEL, {.u32_model = object_draw_kind_bias}, 0};

const OBJECT_CALLBACK callback_object_draw_stage_bias = {OBJECT_CALL_U32_MODEL, {.u32_model = object_draw_stage_bias}, 0};

const OBJECT_CALLBACK callback_object_draw_flag_512 = {OBJECT_CALL_U32_MODEL, {.u32_model = object_draw_flag_512}, 0};

const OBJECT_CALLBACK callback_object_draw_timed_scale = {OBJECT_CALL_U32_NONE, {.u32_none = object_draw_timed_scale}, 0};

const OBJECT_CALLBACK callback_object_draw_phase_gated = {OBJECT_CALL_U32_MODEL, {.u32_model = object_draw_phase_gated}, 0};

const OBJECT_CALLBACK callback_object_draw_player_facing = {OBJECT_CALL_U32_MUTABLE_MODEL, {.u32_mutable_model = object_draw_player_facing}, 0};

const OBJECT_CALLBACK callback_object_draw_repeated_pair = {OBJECT_CALL_VOID_MODEL, {.no_result_model = object_draw_repeated_pair}, 0};

const OBJECT_CALLBACK callback_object_draw_faded_fixed_model = {OBJECT_CALL_U32_NONE, {.u32_none = object_draw_faded_fixed_model}, 0};

const OBJECT_CALLBACK callback_object_draw_stage_gated = {OBJECT_CALL_U32_MODEL, {.u32_model = object_draw_stage_gated}, 0};

const OBJECT_CALLBACK callback_vehicle_steering_draw = {OBJECT_CALL_S32_MUTABLE_MODEL, {.s32_mutable_model = vehicle_steering_draw}, 0};

const OBJECT_CALLBACK callback_object_draw_quarter_turn = {OBJECT_CALL_U32_MODEL, {.u32_model = object_draw_quarter_turn}, 0};

const OBJECT_CALLBACK callback_world_objects_update_army_truck = {OBJECT_CALL_S32_MUTABLE_MODEL, {.s32_mutable_model = world_objects_update_army_truck}, 0};

const OBJECT_CALLBACK callback_world_objects_update_army_truck_wreck = {OBJECT_CALL_U32_MUTABLE_MODEL, {.u32_mutable_model = world_objects_update_army_truck_wreck}, 0};

const OBJECT_CALLBACK callback_world_objects_update_aircraft = {OBJECT_CALL_U32_MUTABLE_MODEL, {.u32_mutable_model = world_objects_update_aircraft}, 0};

static const OBJECT_CALLBACK callback_vehicle_steering_move = {OBJECT_CALL_S32_MUTABLE_MODEL, {.s32_mutable_model = vehicle_steering_move}, 0};

const OBJECT_CALLBACK callback_vehicle_steering_update = {OBJECT_CALL_U32_MUTABLE_MODEL, {.u32_mutable_model = vehicle_steering_update}, 0};

const OBJECT_CALLBACK callback_train_update = {OBJECT_CALL_U32_MUTABLE_MODEL, {.u32_mutable_model = train_update}, 0};

const OBJECT_CALLBACK callback_helicopter_update_patrol = {OBJECT_CALL_U32_MUTABLE_MODEL, {.u32_mutable_model = helicopter_update_patrol}, 0};

const OBJECT_CALLBACK callback_effect_remove_line = {OBJECT_CALL_U32_EFFECT, {.u32_effect = effect_remove_line}, 0};

const OBJECT_CALLBACK callback_effect_remove_ribbon = {OBJECT_CALL_U32_EFFECT, {.u32_effect = effect_remove_ribbon}, 0};

const OBJECT_CALLBACK callback_effect_update_moving_ribbon = {OBJECT_CALL_S32_EFFECT, {.s32_effect = effect_update_moving_ribbon}, 0};

const OBJECT_CALLBACK callback_effect_update_landing_spr = {OBJECT_CALL_INTPTR_EFFECT, {.intptr_effect = effect_update_landing_spr}, 0};

const OBJECT_CALLBACK callback_effect_update_falling_spr = {OBJECT_CALL_U32_EFFECT, {.u32_effect = effect_update_falling_spr}, 0};

const OBJECT_CALLBACK callback_effect_update_drifting_spr = {OBJECT_CALL_S32_EFFECT, {.s32_effect = effect_update_drifting_spr}, 0};

const OBJECT_CALLBACK callback_effect_update_spr_short = {OBJECT_CALL_U32_EFFECT, {.u32_effect = effect_update_spr_short}, 0};

const OBJECT_CALLBACK callback_effect_update_spr_long = {OBJECT_CALL_U32_EFFECT, {.u32_effect = effect_update_spr_long}, 0};

const OBJECT_CALLBACK callback_world_objects_update_burning_bin = {OBJECT_CALL_INTPTR_MUTABLE_MODEL, {.intptr_mutable_model = world_objects_update_burning_bin}, 0};

const OBJECT_CALLBACK callback_object_ignore_callback1 = {OBJECT_CALL_VOID_NONE, {.no_result_none = object_ignore_callback1}, 0};

const OBJECT_CALLBACK callback_projectile_update_ballistic = {OBJECT_CALL_U32_MODEL_ARG, {.u32_model_arg = projectile_update_ballistic}, 0xffffffffu};

const OBJECT_CALLBACK callback_world_objects_update_hydrant_spray = {OBJECT_CALL_EFFECT_MODEL, {.effect_model = world_objects_update_hydrant_spray}, 0};

const OBJECT_CALLBACK callback_world_objects_update_water_cannon = {OBJECT_CALL_S32_MUTABLE_MODEL, {.s32_mutable_model = world_objects_update_water_cannon}, 0};

const OBJECT_CALLBACK callback_world_objects_update_truck_fire = {OBJECT_CALL_INTPTR_MUTABLE_MODEL, {.intptr_mutable_model = world_objects_update_truck_fire}, 0};

const OBJECT_CALLBACK callback_helicopter_update = {OBJECT_CALL_U32_MODEL_PROJECTILE, {.u32_model_projectile = helicopter_update}, 0};

const OBJECT_CALLBACK callback_helicopter_draw = {OBJECT_CALL_VOID_MODEL, {.no_result_model = helicopter_draw}, 0};

static const OBJECT_CALLBACK callback_object_draw_scene_early = {OBJECT_CALL_U32_MODEL, {.u32_model = object_draw_scene_early}, 0};

const OBJECT_CALLBACK callback_object_draw_scene_active = {OBJECT_CALL_U32_MODEL, {.u32_model = object_draw_scene_active}, 0};

const OBJECT_CALLBACK callback_world_objects_update_lifting_door = {OBJECT_CALL_S32_MUTABLE_MODEL, {.s32_mutable_model = world_objects_update_lifting_door}, 0};

const OBJECT_CALLBACK callback_scene_vehicle_start_departure = {OBJECT_CALL_U32_MUTABLE_MODEL, {.u32_mutable_model = scene_vehicle_start_departure}, 0};

const OBJECT_CALLBACK callback_scene_vehicle_update_departure = {OBJECT_CALL_S32_MUTABLE_MODEL, {.s32_mutable_model = scene_vehicle_update_departure}, 0};

const OBJECT_CALLBACK callback_scene_vehicle_update_accelerating_truck = {OBJECT_CALL_S32_MUTABLE_MODEL, {.s32_mutable_model = scene_vehicle_update_accelerating_truck}, 0};

const OBJECT_CALLBACK callback_world_objects_update_moving_emitter = {OBJECT_CALL_U32_MUTABLE_MODEL, {.u32_mutable_model = world_objects_update_moving_emitter}, 0};

const OBJECT_CALLBACK callback_object_update_orbit = {OBJECT_CALL_U32_MUTABLE_MODEL, {.u32_mutable_model = object_update_orbit}, 0};

const OBJECT_CALLBACK callback_object_update_orbit_sound = {OBJECT_CALL_S32_MUTABLE_MODEL, {.s32_mutable_model = object_update_orbit_sound}, 0};

const OBJECT_CALLBACK callback_object_update_wall_fragment = {OBJECT_CALL_U32_MUTABLE_MODEL, {.u32_mutable_model = object_update_wall_fragment}, 0};

const OBJECT_CALLBACK callback_scene_vehicle_update_taxi = {OBJECT_CALL_INTPTR_MUTABLE_MODEL, {.intptr_mutable_model = scene_vehicle_update_taxi}, 0};

const OBJECT_CALLBACK callback_scene_vehicle_update_truck = {OBJECT_CALL_U32_MUTABLE_MODEL, {.u32_mutable_model = scene_vehicle_update_truck}, 0};

const OBJECT_CALLBACK callback_scene_vehicle_update_traffic = {OBJECT_CALL_U32_MUTABLE_MODEL, {.u32_mutable_model = scene_vehicle_update_traffic}, 0};

const OBJECT_CALLBACK callback_object_ignore_callback2 = {OBJECT_CALL_VOID_NONE, {.no_result_none = object_ignore_callback2}, 0};

const OBJECT_CALLBACK callback_scene_vehicle_update_car = {OBJECT_CALL_U32_MUTABLE_MODEL, {.u32_mutable_model = scene_vehicle_update_car}, 0};

const OBJECT_CALLBACK callback_helicopter_draw_patrol = {OBJECT_CALL_VOID_MODEL, {.no_result_model = helicopter_draw_patrol}, 0};

const OBJECT_CALLBACK callback_suspended_update = {OBJECT_CALL_U32_MUTABLE_MODEL, {.u32_mutable_model = suspended_update}, 0};

const OBJECT_CALLBACK callback_suspended_draw = {OBJECT_CALL_VOID_MODEL, {.no_result_model = suspended_draw}, 0};

const OBJECT_CALLBACK callback_object_draw_pitched_spinning_pair = {OBJECT_CALL_VOID_MODEL, {.no_result_model = object_draw_pitched_spinning_pair}, 0};

const OBJECT_CALLBACK callback_object_draw_phased_effects = {OBJECT_CALL_VOID_MODEL, {.no_result_model = object_draw_phased_effects}, 0};

const OBJECT_CALLBACK callback_world_objects_draw_moving_emitter = {OBJECT_CALL_U32_MODEL, {.u32_model = world_objects_draw_moving_emitter}, 0};

const OBJECT_CALLBACK callback_object_ignore_callback3 = {OBJECT_CALL_VOID_NONE, {.no_result_none = object_ignore_callback3}, 0};

const OBJECT_CALLBACK callback_world_objects_roll = {OBJECT_CALL_VOID_MUTABLE_MODEL, {.no_result_mutable_model = world_objects_roll}, 0};

const OBJECT_CALLBACK callback_object_update_bouncing_drop = {OBJECT_CALL_S32_MUTABLE_MODEL, {.s32_mutable_model = object_update_bouncing_drop}, 0};

const OBJECT_CALLBACK callback_object_update_rotation_shake = {OBJECT_CALL_VOID_MUTABLE_MODEL, {.no_result_mutable_model = object_update_rotation_shake}, 0};

const OBJECT_CALLBACK callback_object_draw_scene_gated = {OBJECT_CALL_U32_MODEL, {.u32_model = object_draw_scene_gated}, 0};

static const OBJECT_CALLBACK callback_object_apply_rotations = {OBJECT_CALL_VOID_MODEL, {.no_result_model = object_apply_rotations}, 0};

static const OBJECT_CALLBACK callback_object_draw_visible_yaw = {OBJECT_CALL_U32_MODEL, {.u32_model = object_draw_visible_yaw}, 0};

const OBJECT_CALLBACK callback_object_draw_visible_player_facing = {OBJECT_CALL_U32_MUTABLE_MODEL, {.u32_mutable_model = object_draw_visible_player_facing}, 0};

const OBJECT_CALLBACK callback_object_draw_offset_shadow = {OBJECT_CALL_U32_MODEL, {.u32_model = object_draw_offset_shadow}, 0};

const OBJECT_CALLBACK callback_object_draw_shadowed_model = {OBJECT_CALL_U32_MODEL, {.u32_model = object_draw_shadowed_model}, 0};

const OBJECT_CALLBACK callback_world_objects_draw_car = {OBJECT_CALL_U32_MODEL, {.u32_model = world_objects_draw_car}, 0};

const OBJECT_CALLBACK callback_world_objects_draw_compressed_car = {OBJECT_CALL_U32_MUTABLE_MODEL, {.u32_mutable_model = world_objects_draw_compressed_car}, 0};

const OBJECT_CALLBACK callback_object_draw_base_raised = {OBJECT_CALL_U32_MODEL, {.u32_model = object_draw_base_raised}, 0};

const OBJECT_CALLBACK callback_object_draw_base_compressed = {OBJECT_CALL_U32_MUTABLE_MODEL, {.u32_mutable_model = object_draw_base_compressed}, 0};

const OBJECT_CALLBACK callback_object_draw_raised = {OBJECT_CALL_U32_MODEL, {.u32_model = object_draw_raised}, 0};

const OBJECT_CALLBACK callback_object_draw_compressed = {OBJECT_CALL_U32_MUTABLE_MODEL, {.u32_mutable_model = object_draw_compressed}, 0};

static const OBJECT_CALLBACK callback_object_draw_depth_fade = {OBJECT_CALL_U32_MODEL, {.u32_model = object_draw_depth_fade}, 0};

const OBJECT_CALLBACK callback_object_draw_rotated_packets = {OBJECT_CALL_U32_MODEL, {.u32_model = object_draw_rotated_packets}, 0};

const OBJECT_CALLBACK callback_object_draw_near_fade = {OBJECT_CALL_U32_MODEL, {.u32_model = object_draw_near_fade}, 0};

const OBJECT_CALLBACK callback_effect_draw_double_point = {OBJECT_CALL_VOID_EFFECT, {.no_result_effect = effect_draw_double_point}, 0};

const OBJECT_CALLBACK callback_effect_draw_attachment_model = {OBJECT_CALL_VOID_EFFECT, {.no_result_effect = effect_draw_attachment_model}, 0};

const OBJECT_CALLBACK callback_object_draw_dimmed_yaw = {OBJECT_CALL_VOID_MODEL, {.no_result_model = object_draw_dimmed_yaw}, 0};

const OBJECT_CALLBACK callback_object_draw_inverted_roll = {OBJECT_CALL_VOID_MODEL, {.no_result_model = object_draw_inverted_roll}, 0};

const OBJECT_CALLBACK callback_object_draw_offset_spinning_pair = {OBJECT_CALL_VOID_MODEL, {.no_result_model = object_draw_offset_spinning_pair}, 0};

const OBJECT_CALLBACK callback_object_draw_depth_rotated_packets = {OBJECT_CALL_U32_MODEL, {.u32_model = object_draw_depth_rotated_packets}, 0};

const OBJECT_CALLBACK callback_effect_draw_hit_particle = {OBJECT_CALL_VOID_EFFECT, {.no_result_effect = effect_draw_hit_particle}, 0};

const OBJECT_CALLBACK callback_effect_draw_spark = {OBJECT_CALL_VOID_EFFECT, {.no_result_effect = effect_draw_spark}, 0};

const OBJECT_CALLBACK callback_effect_draw_point = {OBJECT_CALL_VOID_EFFECT, {.no_result_effect = effect_draw_point}, 0};

const OBJECT_CALLBACK callback_effect_draw_rotating_particle = {OBJECT_CALL_VOID_NONE, {.no_result_none = effect_draw_rotating_particle}, 0};

const OBJECT_CALLBACK callback_effect_draw_short_particle = {OBJECT_CALL_VOID_NONE, {.no_result_none = effect_draw_short_particle}, 0};

const OBJECT_CALLBACK callback_effect_draw_long_particle = {OBJECT_CALL_VOID_NONE, {.no_result_none = effect_draw_long_particle}, 0};

const OBJECT_CALLBACK callback_effect_draw_model = {OBJECT_CALL_VOID_EFFECT, {.no_result_effect = effect_draw_model}, 0};

const OBJECT_CALLBACK callback_effect_draw_spr = {OBJECT_CALL_VOID_MODEL_EFFECT, {.no_result_model_effect = effect_draw_spr}, 0};

const OBJECT_CALLBACK callback_projectile_draw_glow = {OBJECT_CALL_VOID_MODEL, {.no_result_model = projectile_draw_glow}, 0};

const OBJECT_CALLBACK callback_effect_draw_ribbon = {OBJECT_CALL_VOID_EFFECT, {.no_result_effect = effect_draw_ribbon}, 0};

const OBJECT_CALLBACK callback_effect_draw_moving_ribbon = {OBJECT_CALL_VOID_EFFECT, {.no_result_effect = effect_draw_moving_ribbon}, 0};

const OBJECT_CALLBACK callback_effect_draw_line = {OBJECT_CALL_VOID_EFFECT, {.no_result_effect = effect_draw_line}, 0};

const OBJECT_CALLBACK callback_object_ignore_callback4 = {OBJECT_CALL_VOID_NONE, {.no_result_none = object_ignore_callback4}, 0};

const OBJECT_CALLBACK callback_object_ignore_callback5 = {OBJECT_CALL_VOID_NONE, {.no_result_none = object_ignore_callback5}, 0};

const OBJECT_CALLBACK callback_effect_draw_blast_large = {OBJECT_CALL_VOID_EFFECT, {.no_result_effect = effect_draw_blast_large}, 0};

const OBJECT_CALLBACK callback_effect_draw_blast = {OBJECT_CALL_VOID_EFFECT, {.no_result_effect = effect_draw_blast}, 0};

const OBJECT_CALLBACK callback_effect_draw_oriented_model = {OBJECT_CALL_VOID_EFFECT, {.no_result_effect = effect_draw_oriented_model}, 0};

const OBJECT_CALLBACK callback_effect_draw_ring_large = {OBJECT_CALL_VOID_EFFECT, {.no_result_effect = effect_draw_ring_large}, 0};

const OBJECT_CALLBACK callback_effect_draw_ring = {OBJECT_CALL_VOID_EFFECT, {.no_result_effect = effect_draw_ring}, 0};

const OBJECT_CALLBACK callback_effect_draw_ring_small = {OBJECT_CALL_VOID_EFFECT, {.no_result_effect = effect_draw_ring_small}, 0};

const OBJECT_CALLBACK callback_effect_draw_rising_smoke = {OBJECT_CALL_VOID_EFFECT, {.no_result_effect = effect_draw_rising_smoke}, 0};

const OBJECT_CALLBACK callback_object_draw_settled_debris = {OBJECT_CALL_VOID_MODEL, {.no_result_model = object_draw_settled_debris}, 0};

const OBJECT_CALLBACK callback_object_draw_depth_pair = {OBJECT_CALL_U32_MODEL, {.u32_model = object_draw_depth_pair}, 0};

const OBJECT_CALLBACK callback_object_ignore_callback6 = {OBJECT_CALL_VOID_NONE, {.no_result_none = object_ignore_callback6}, 0};

const OBJECT_CALLBACK callback_item_draw_explosive = {OBJECT_CALL_VOID_MODEL, {.no_result_model = item_draw_explosive}, 0};

GDB_CALL intptr object_call(const OBJECT_CALLBACK *callback, struct MODEL_OBJECT *item, struct EFFECT *effect)
{
    if (callback)
    {
        switch (callback->signature)
        {
            case OBJECT_CALL_U32_NONE:
                return callback->fn.u32_none();
            case OBJECT_CALL_S32_NONE:
                return callback->fn.s32_none();
            case OBJECT_CALL_VOID_NONE:
                callback->fn.no_result_none();
                return 0;
            case OBJECT_CALL_U32_MODEL_ARG:
                if (item && !effect)
                    return callback->fn.u32_model_arg(item, callback->second_arg);
                break;
            case OBJECT_CALL_U32_MODEL_PROJECTILE:
                if (item && !effect)
                    return callback->fn.u32_model_projectile(item, NULL);
                break;
            case OBJECT_CALL_INTPTR_EFFECT:
                if (effect)
                    return callback->fn.intptr_effect(effect);
                break;
            case OBJECT_CALL_U32_CONST_EFFECT:
                if (effect)
                    return callback->fn.u32_const_effect(effect);
                break;
            case OBJECT_CALL_U32_EFFECT:
                if (effect)
                    return callback->fn.u32_effect(effect);
                break;
            case OBJECT_CALL_S32_EFFECT:
                if (effect)
                    return callback->fn.s32_effect(effect);
                break;
            case OBJECT_CALL_U32_MODEL_EFFECT:
                if ((item && !effect) || (!item && effect))
                    return callback->fn.u32_model_effect(item, effect);
                break;
            case OBJECT_CALL_S32_MODEL_EFFECT:
                if ((item && !effect) || (!item && effect))
                    return callback->fn.s32_model_effect(item, effect);
                break;
            case OBJECT_CALL_VOID_MODEL_EFFECT:
                if ((item && !effect) || (!item && effect))
                {
                    callback->fn.no_result_model_effect(item, effect);
                    return 0;
                }
                break;
            case OBJECT_CALL_EFFECT_MODEL:
                if (item && !effect)
                    return (intptr)callback->fn.effect_model(item);
                break;
            case OBJECT_CALL_U32_MODEL:
                if (item && !effect)
                    return callback->fn.u32_model(item);
                break;
            case OBJECT_CALL_U32_MUTABLE_MODEL:
                if (item && !effect)
                    return callback->fn.u32_mutable_model(item);
                break;
            case OBJECT_CALL_S32_MUTABLE_MODEL:
                if (item && !effect)
                    return callback->fn.s32_mutable_model(item);
                break;
            case OBJECT_CALL_INTPTR_MUTABLE_MODEL:
                if (item && !effect)
                    return callback->fn.intptr_mutable_model(item);
                break;
            case OBJECT_CALL_VOID_MUTABLE_MODEL:
                if (item && !effect)
                {
                    callback->fn.no_result_mutable_model(item);
                    return 0;
                }
                break;
            case OBJECT_CALL_VOID_MODEL:
                if (item && !effect)
                {
                    callback->fn.no_result_model(item);
                    return 0;
                }
                break;
            case OBJECT_CALL_VOID_EFFECT:
                if (effect)
                {
                    callback->fn.no_result_effect(effect);
                    return 0;
                }
                break;
        }
    }
    /* TODO: Investigate missing callbacks or unsupported signatures without a raw-code fallback */
    XASSERT(0);
    return 0;
}

GDB_CALL void object_ignore_callback0(void)
{
    FUNCTION_MARKER(0x8002a974u, "GAME.EXE");
}

GDB_CALL void object_ignore_callback1(void)
{
    FUNCTION_MARKER(0x80041b4cu, "GAME.EXE");
}

GDB_CALL void object_ignore_callback2(void)
{
    FUNCTION_MARKER(0x8004483cu, "GAME.EXE");
}

GDB_CALL void object_ignore_callback3(void)
{
    FUNCTION_MARKER(0x800455dcu, "GAME.EXE");
}

GDB_CALL void object_ignore_callback4(void)
{
    FUNCTION_MARKER(0x8005d150u, "GAME.EXE");
}

GDB_CALL void object_ignore_callback5(void)
{
    FUNCTION_MARKER(0x8005d158u, "GAME.EXE");
}

GDB_CALL void object_ignore_callback6(void)
{
    FUNCTION_MARKER(0x8005daccu, "GAME.EXE");
}
