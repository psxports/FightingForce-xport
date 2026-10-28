#ifndef OBJECT_CALLBACKS_H
#define OBJECT_CALLBACKS_H

#include "resources.h"

struct PROJECTILE_SOURCE;
struct EFFECT;
struct MODEL_OBJECT;

typedef enum OBJECT_CALL_SIGNATURE
{
    OBJECT_CALL_U32_NONE = 3,
    OBJECT_CALL_S32_NONE = 4,
    OBJECT_CALL_VOID_NONE = 5,
    OBJECT_CALL_INTPTR_EFFECT = 10,
    OBJECT_CALL_U32_EFFECT = 11,
    OBJECT_CALL_S32_EFFECT = 12,
    OBJECT_CALL_VOID_EFFECT = 13,
    OBJECT_CALL_VOID_MODEL = 14,
    OBJECT_CALL_U32_MODEL_EFFECT = 15,
    OBJECT_CALL_VOID_MODEL_EFFECT = 16,
    OBJECT_CALL_S32_MODEL_EFFECT = 17,
    OBJECT_CALL_U32_MODEL = 18,
    OBJECT_CALL_U32_MUTABLE_MODEL = 19,
    OBJECT_CALL_VOID_MUTABLE_MODEL = 20,
    OBJECT_CALL_S32_MUTABLE_MODEL = 21,
    OBJECT_CALL_INTPTR_MUTABLE_MODEL = 22,
    OBJECT_CALL_EFFECT_MODEL = 23,
    OBJECT_CALL_U32_MODEL_PROJECTILE = 24,
    OBJECT_CALL_U32_MODEL_ARG = 25,
    OBJECT_CALL_U32_CONST_EFFECT = 26
} OBJECT_CALL_SIGNATURE;

typedef struct OBJECT_CALLBACK
{
    OBJECT_CALL_SIGNATURE signature;

    union
    {
        intptr (*intptr_effect)(struct EFFECT *);
        uint32 (*u32_effect)(struct EFFECT *);
        uint32 (*u32_const_effect)(const struct EFFECT *);
        sint32 (*s32_effect)(struct EFFECT *);
        void (*no_result_effect)(const struct EFFECT *);
        void (*no_result_model)(const struct MODEL_OBJECT *);
        uint32 (*u32_model)(const struct MODEL_OBJECT *);
        uint32 (*u32_mutable_model)(struct MODEL_OBJECT *);
        void (*no_result_mutable_model)(struct MODEL_OBJECT *);
        sint32 (*s32_mutable_model)(struct MODEL_OBJECT *);
        intptr (*intptr_mutable_model)(struct MODEL_OBJECT *);
        uint32 (*u32_model_effect)(struct MODEL_OBJECT *, struct EFFECT *);
        sint32 (*s32_model_effect)(struct MODEL_OBJECT *, struct EFFECT *);
        void (*no_result_model_effect)(struct MODEL_OBJECT *, struct EFFECT *);
        struct EFFECT *(*effect_model)(const struct MODEL_OBJECT *);
        uint32 (*u32_none)(void);
        sint32 (*s32_none)(void);
        void (*no_result_none)(void);
        uint32 (*u32_model_arg)(struct MODEL_OBJECT *, uint32);
        uint32 (*u32_model_projectile)(struct MODEL_OBJECT *, const struct PROJECTILE_SOURCE *);
    } fn;

    uint32 second_arg;
} OBJECT_CALLBACK;

typedef enum OBJECT_CALLBACK_TABLE
{
    OBJECT_CALLBACK_WORLD_DRAW,
    OBJECT_CALLBACK_EFFECT_DRAW,
    OBJECT_CALLBACK_UPDATE,
    OBJECT_CALLBACK_EFFECT_UPDATE,
    OBJECT_CALLBACK_DYNAMIC_DRAW,
    OBJECT_CALLBACK_TABLE_COUNT
} OBJECT_CALLBACK_TABLE;

enum
{
    OBJECT_CALLBACK_COUNT = 202,
    OBJECT_CALLBACK_SLOT_COUNT = MODEL_TYPE_COUNT + 1
};

extern const OBJECT_CALLBACK *object_callbacks[OBJECT_CALLBACK_TABLE_COUNT][OBJECT_CALLBACK_SLOT_COUNT];
extern const uint32 object_callback_counts[OBJECT_CALLBACK_TABLE_COUNT];

extern const OBJECT_CALLBACK callback_object_draw_phase_yaw_variant;
extern const OBJECT_CALLBACK callback_object_draw_flag_256;
extern const OBJECT_CALLBACK callback_object_draw_camera_yaw_inner;
extern const OBJECT_CALLBACK callback_object_draw_rotated;
extern const OBJECT_CALLBACK callback_helicopter_update_scene;
extern const OBJECT_CALLBACK callback_helicopter_draw_scene;
extern const OBJECT_CALLBACK callback_world_objects_draw_tank;
extern const OBJECT_CALLBACK callback_object_draw_camera_x_gated_fixed_model;
extern const OBJECT_CALLBACK callback_object_draw_fixed_pair;
extern const OBJECT_CALLBACK callback_object_draw_side_spin;
extern const OBJECT_CALLBACK callback_object_ignore_callback0;
extern const OBJECT_CALLBACK callback_object_draw_raised_spinning_pair;
extern const OBJECT_CALLBACK callback_object_draw_height_offset;
extern const OBJECT_CALLBACK callback_object_draw_wave;
extern const OBJECT_CALLBACK callback_effect_draw_glass;
extern const OBJECT_CALLBACK callback_object_update_rising_spr;
extern const OBJECT_CALLBACK callback_effect_update_rising_spr_short;
extern const OBJECT_CALLBACK callback_effect_update_rising_spr_long;
extern const OBJECT_CALLBACK callback_effect_update_dust_short;
extern const OBJECT_CALLBACK callback_effect_update_dust_long;
extern const OBJECT_CALLBACK callback_effect_update_blast_large;
extern const OBJECT_CALLBACK callback_effect_update_blast;
extern const OBJECT_CALLBACK callback_effect_update_ring_large;
extern const OBJECT_CALLBACK callback_effect_update_ring;
extern const OBJECT_CALLBACK callback_effect_update_shockwave;
extern const OBJECT_CALLBACK callback_effect_update_ring_short;
extern const OBJECT_CALLBACK callback_effect_update_rising_smoke;
extern const OBJECT_CALLBACK callback_object_update_oscillating_spr;
extern const OBJECT_CALLBACK callback_object_update_impact_spr;
extern const OBJECT_CALLBACK callback_object_update_expiring;
extern const OBJECT_CALLBACK callback_object_update_physics;
extern const OBJECT_CALLBACK callback_object_update_physics_alias;
extern const OBJECT_CALLBACK callback_object_update_explosive;
extern const OBJECT_CALLBACK callback_effect_update_particle;
extern const OBJECT_CALLBACK callback_effect_update_falling_glass;
extern const OBJECT_CALLBACK callback_effect_update_slowing_particle;
extern const OBJECT_CALLBACK callback_effect_update_short_particle;
extern const OBJECT_CALLBACK callback_effect_update_long_particle;
extern const OBJECT_CALLBACK callback_object_draw_scene_variant;
extern const OBJECT_CALLBACK callback_object_update_ballistic_spin;
extern const OBJECT_CALLBACK callback_object_update_drifting_spin;
extern const OBJECT_CALLBACK callback_object_draw_depth_bob;
extern const OBJECT_CALLBACK callback_helicopter_update_intro;
extern const OBJECT_CALLBACK callback_helicopter_draw_intro;
extern const OBJECT_CALLBACK callback_effect_update_delayed_fragment;
extern const OBJECT_CALLBACK callback_effect_update_gate_fragment;
extern const OBJECT_CALLBACK callback_gate_shake;
extern const OBJECT_CALLBACK callback_gate_panels_shake;
extern const OBJECT_CALLBACK callback_object_update_falling_panel;
extern const OBJECT_CALLBACK callback_panel_shake;
extern const OBJECT_CALLBACK callback_object_update_falling_panel_sideways;
extern const OBJECT_CALLBACK callback_object_update_wobble;
extern const OBJECT_CALLBACK callback_object_update_wobble_debris;
extern const OBJECT_CALLBACK callback_falling_gate;
extern const OBJECT_CALLBACK callback_gate_fragment;
extern const OBJECT_CALLBACK callback_lift_update_marker;
extern const OBJECT_CALLBACK callback_object_update_oscillation;
extern const OBJECT_CALLBACK callback_object_update_compression;
extern const OBJECT_CALLBACK callback_dynamic_objects_settle_debris;
extern const OBJECT_CALLBACK callback_ground_bounce;
extern const OBJECT_CALLBACK callback_spinning_landing;
extern const OBJECT_CALLBACK callback_sliding_bounce;
extern const OBJECT_CALLBACK callback_effect_update_bouncing_particle;
extern const OBJECT_CALLBACK callback_world_objects_update_falling_door;
extern const OBJECT_CALLBACK callback_world_objects_update_bus_wreck;
extern const OBJECT_CALLBACK callback_world_objects_update_scrolling_wall;
extern const OBJECT_CALLBACK callback_world_objects_bob;
extern const OBJECT_CALLBACK callback_world_objects_update_radar;
extern const OBJECT_CALLBACK callback_object_update_wobble_particles;
extern const OBJECT_CALLBACK callback_object_update_smoke_emitter;
extern const OBJECT_CALLBACK callback_object_update_yaw;
extern const OBJECT_CALLBACK callback_lab_tank_update;
extern const OBJECT_CALLBACK callback_lab_tank_update_effect;
extern const OBJECT_CALLBACK callback_object_update_island_fragment;
extern const OBJECT_CALLBACK callback_air_bike_update_approach;
extern const OBJECT_CALLBACK callback_air_bike_update_departure;
extern const OBJECT_CALLBACK callback_world_objects_update_traffic;
extern const OBJECT_CALLBACK callback_effect_update_debris_low_gravity;
extern const OBJECT_CALLBACK callback_effect_update_debris;
extern const OBJECT_CALLBACK callback_effect_update_debris_quarter_speed;
extern const OBJECT_CALLBACK callback_projectile_update_accelerating;
extern const OBJECT_CALLBACK callback_world_objects_update_exit_doors;
extern const OBJECT_CALLBACK callback_object_draw_yaw_flag_128;
extern const OBJECT_CALLBACK callback_object_draw_camera_yaw_outer;
extern const OBJECT_CALLBACK callback_object_draw_kind_bias;
extern const OBJECT_CALLBACK callback_object_draw_stage_bias;
extern const OBJECT_CALLBACK callback_object_draw_flag_512;
extern const OBJECT_CALLBACK callback_object_draw_timed_scale;
extern const OBJECT_CALLBACK callback_object_draw_phase_gated;
extern const OBJECT_CALLBACK callback_object_draw_player_facing;
extern const OBJECT_CALLBACK callback_object_draw_repeated_pair;
extern const OBJECT_CALLBACK callback_object_draw_faded_fixed_model;
extern const OBJECT_CALLBACK callback_object_draw_stage_gated;
extern const OBJECT_CALLBACK callback_vehicle_steering_draw;
extern const OBJECT_CALLBACK callback_object_draw_quarter_turn;
extern const OBJECT_CALLBACK callback_world_objects_update_army_truck;
extern const OBJECT_CALLBACK callback_world_objects_update_army_truck_wreck;
extern const OBJECT_CALLBACK callback_world_objects_update_aircraft;
extern const OBJECT_CALLBACK callback_vehicle_steering_update;
extern const OBJECT_CALLBACK callback_train_update;
extern const OBJECT_CALLBACK callback_helicopter_update_patrol;
extern const OBJECT_CALLBACK callback_effect_remove_line;
extern const OBJECT_CALLBACK callback_effect_remove_ribbon;
extern const OBJECT_CALLBACK callback_effect_update_moving_ribbon;
extern const OBJECT_CALLBACK callback_effect_update_landing_spr;
extern const OBJECT_CALLBACK callback_effect_update_falling_spr;
extern const OBJECT_CALLBACK callback_effect_update_drifting_spr;
extern const OBJECT_CALLBACK callback_effect_update_spr_short;
extern const OBJECT_CALLBACK callback_effect_update_spr_long;
extern const OBJECT_CALLBACK callback_world_objects_update_burning_bin;
extern const OBJECT_CALLBACK callback_object_ignore_callback1;
extern const OBJECT_CALLBACK callback_projectile_update_ballistic;
extern const OBJECT_CALLBACK callback_world_objects_update_hydrant_spray;
extern const OBJECT_CALLBACK callback_world_objects_update_water_cannon;
extern const OBJECT_CALLBACK callback_world_objects_update_truck_fire;
extern const OBJECT_CALLBACK callback_helicopter_update;
extern const OBJECT_CALLBACK callback_helicopter_draw;
extern const OBJECT_CALLBACK callback_object_draw_scene_active;
extern const OBJECT_CALLBACK callback_lab_tank_start;
extern const OBJECT_CALLBACK callback_world_objects_update_lifting_door;
extern const OBJECT_CALLBACK callback_scene_vehicle_start_departure;
extern const OBJECT_CALLBACK callback_scene_vehicle_update_departure;
extern const OBJECT_CALLBACK callback_scene_vehicle_update_accelerating_truck;
extern const OBJECT_CALLBACK callback_world_objects_update_moving_emitter;
extern const OBJECT_CALLBACK callback_object_update_orbit;
extern const OBJECT_CALLBACK callback_object_update_orbit_sound;
extern const OBJECT_CALLBACK callback_object_update_wall_fragment;
extern const OBJECT_CALLBACK callback_scene_vehicle_update_taxi;
extern const OBJECT_CALLBACK callback_scene_vehicle_update_truck;
extern const OBJECT_CALLBACK callback_scene_vehicle_update_traffic;
extern const OBJECT_CALLBACK callback_object_ignore_callback2;
extern const OBJECT_CALLBACK callback_scene_vehicle_update_car;
extern const OBJECT_CALLBACK callback_helicopter_draw_patrol;
extern const OBJECT_CALLBACK callback_suspended_update;
extern const OBJECT_CALLBACK callback_suspended_draw;
extern const OBJECT_CALLBACK callback_object_draw_pitched_spinning_pair;
extern const OBJECT_CALLBACK callback_object_draw_phased_effects;
extern const OBJECT_CALLBACK callback_world_objects_draw_moving_emitter;
extern const OBJECT_CALLBACK callback_object_ignore_callback3;
extern const OBJECT_CALLBACK callback_world_objects_roll;
extern const OBJECT_CALLBACK callback_object_update_bouncing_drop;
extern const OBJECT_CALLBACK callback_object_update_rotation_shake;
extern const OBJECT_CALLBACK callback_object_draw_scene_gated;
extern const OBJECT_CALLBACK callback_object_draw_visible_player_facing;
extern const OBJECT_CALLBACK callback_object_draw_offset_shadow;
extern const OBJECT_CALLBACK callback_object_draw_shadowed_model;
extern const OBJECT_CALLBACK callback_world_objects_draw_car;
extern const OBJECT_CALLBACK callback_world_objects_draw_compressed_car;
extern const OBJECT_CALLBACK callback_object_draw_base_raised;
extern const OBJECT_CALLBACK callback_object_draw_base_compressed;
extern const OBJECT_CALLBACK callback_object_draw_raised;
extern const OBJECT_CALLBACK callback_object_draw_compressed;
extern const OBJECT_CALLBACK callback_object_draw_rotated_packets;
extern const OBJECT_CALLBACK callback_object_draw_near_fade;
extern const OBJECT_CALLBACK callback_lab_tank_draw;
extern const OBJECT_CALLBACK callback_lab_tank_draw_effect;
extern const OBJECT_CALLBACK callback_effect_draw_double_point;
extern const OBJECT_CALLBACK callback_effect_draw_attachment_model;
extern const OBJECT_CALLBACK callback_lift_draw;
extern const OBJECT_CALLBACK callback_object_draw_dimmed_yaw;
extern const OBJECT_CALLBACK callback_object_draw_inverted_roll;
extern const OBJECT_CALLBACK callback_object_draw_offset_spinning_pair;
extern const OBJECT_CALLBACK callback_object_draw_depth_rotated_packets;
extern const OBJECT_CALLBACK callback_effect_draw_hit_particle;
extern const OBJECT_CALLBACK callback_effect_draw_spark;
extern const OBJECT_CALLBACK callback_effect_draw_point;
extern const OBJECT_CALLBACK callback_effect_draw_rotating_particle;
extern const OBJECT_CALLBACK callback_effect_draw_short_particle;
extern const OBJECT_CALLBACK callback_effect_draw_long_particle;
extern const OBJECT_CALLBACK callback_effect_draw_model;
extern const OBJECT_CALLBACK callback_effect_draw_spr;
extern const OBJECT_CALLBACK callback_projectile_draw_glow;
extern const OBJECT_CALLBACK callback_effect_draw_ribbon;
extern const OBJECT_CALLBACK callback_effect_draw_moving_ribbon;
extern const OBJECT_CALLBACK callback_effect_draw_line;
extern const OBJECT_CALLBACK callback_object_ignore_callback4;
extern const OBJECT_CALLBACK callback_object_ignore_callback5;
extern const OBJECT_CALLBACK callback_effect_draw_blast_large;
extern const OBJECT_CALLBACK callback_effect_draw_blast;
extern const OBJECT_CALLBACK callback_effect_draw_oriented_model;
extern const OBJECT_CALLBACK callback_effect_draw_ring_large;
extern const OBJECT_CALLBACK callback_effect_draw_ring;
extern const OBJECT_CALLBACK callback_effect_draw_ring_small;
extern const OBJECT_CALLBACK callback_effect_draw_rising_smoke;
extern const OBJECT_CALLBACK callback_lift_draw_marker;
extern const OBJECT_CALLBACK callback_object_draw_settled_debris;
extern const OBJECT_CALLBACK callback_object_draw_depth_pair;
extern const OBJECT_CALLBACK callback_default_model_draw;
extern const OBJECT_CALLBACK callback_model_draw;
extern const OBJECT_CALLBACK callback_object_ignore_callback6;
extern const OBJECT_CALLBACK callback_item_draw_explosive;

GDB_CALL intptr object_call(const OBJECT_CALLBACK *callback, struct MODEL_OBJECT *item, struct EFFECT *effect);

GDB_CALL void object_ignore_callback0(void);

GDB_CALL void object_ignore_callback1(void);

GDB_CALL void object_ignore_callback2(void);

GDB_CALL void object_ignore_callback3(void);

GDB_CALL void object_ignore_callback4(void);

GDB_CALL void object_ignore_callback5(void);

GDB_CALL void object_ignore_callback6(void);

GDB_CALL uint32 object_draw_faded_fixed_model(void);

GDB_CALL uint32 object_draw_camera_x_gated_fixed_model(void);

GDB_CALL uint32 object_draw_timed_scale(void);

GDB_CALL uint32 object_draw_fixed_pair(void);

GDB_CALL uint32 object_draw_phase_yaw_variant(void);

#endif
