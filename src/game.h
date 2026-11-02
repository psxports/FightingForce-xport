#ifndef GAME_H
#define GAME_H
#include "xport.h"
#include "psx.h"
#include "resources.h"
#include "animation.h"
#include "loading.h"
#include "resource_loaders.h"
#include "character_resources.h"
#include "texture.h"
#include "terrain.h"
#include "world_objects.h"
#include "effects.h"
#include "dynamic_objects.h"
#include "projectile.h"
#include "air_bike.h"
#include "scene_vehicle.h"
#include "level_script.h"
#include "level_transition.h"
#include "level_start.h"
#include "level_models.h"
#include "game_pause.h"
#include "cd_music.h"
#include "sound_voice.h"
#include "sound.h"
#include "stage_audio.h"
#include "sound_samples.h"
#include "sound_output.h"
#include "character_names.h"
#include "npc_behavior.h"
#include "npc_actions.h"
#include "npc_targeting.h"
#include "object_callbacks.h"
#include "collision.h"
#include "regions.h"
#include "actor_events.h"
#include "actor_pose.h"
#include "model_render.h"
#include "transform.h"
#include "actors.h"
#include "actor_collision.h"
#include "helicopter.h"
#include "actor_items.h"
#include "high_scores.h"
#include "hud.h"
#include "player.h"
#include "localization.h"
#include "card_dialog.h"
#include "card_storage.h"
#include "game_settings.h"
#include "game_session.h"
#include "game_tables.h"
#include "screenshot.h"
#include "demo.h"
#include "scene_playback.h"
#include "display.h"
#include "background.h"
#include "render_style.h"
#include "depth_table.h"
#include "projection.h"
#include "sprite.h"
#include "particle.h"
#include "render_buffers.h"
#include "options_menu.h"
#include "menu.h"
#include "character_menu.h"
#include "character_render.h"
#include "camera.h"
#include "lab_tank.h"
#include "lift.h"
#include "suspended.h"
#include "vehicle_steering.h"
#include "train.h"
#include "stage_animation.h"
#include "stage.h"
#include "item_pickup.h"
#include "geometry_math.h"
#include "text.h"
#include "action_history.h"
#include "diagnostics.h"
/* Diagnostic expressions and environment controls are Debug-only */
#if defined(NDEBUG)
    #define DIAGNOSTIC(...) \
        do \
        { \
        } while (0)
    #define DIAGNOSTIC_ENV(name) NULL
#else
    #define DIAGNOSTIC(...) \
        do \
        { \
            __VA_ARGS__; \
        } while (0)
    #define DIAGNOSTIC_ENV(name) getenv(name)
#endif
int game_run_arguments(int argc, char **argv);

/* Native binding for SDK/BIOS services in the separate SLUS image. */

uint32 sound_device_init(void);
uint32 lightning_draw_strip(const FIXED_VEC3 *src, const FIXED_VEC3 *dst);
uint32 credits_run(void);

typedef struct PAD_HOST_STATE
{
    uint32 enabled;
} PAD_HOST_STATE;

extern PAD_HOST_STATE pad_host;

enum
{
    PAD_PLAYER_COUNT = 2
};

/* Direct PAD transport report, including unused response bytes */
typedef struct PAD_REPORT
{
    uint8 status;
    uint8 id;
    uint16 buttons;
    uint8 right_x, right_y;
    uint8 left_x, left_y;
    uint8 transport_tail[26];
} PAD_REPORT;

typedef struct PAD_DECODE_STATE
{
    uint8 type;
    uint8 reset_blocked;
    sint32 x, y;
    uint16 reset_count[PAD_PLAYER_COUNT];
    uint8 disconnect[PAD_PLAYER_COUNT];
    uint8 unsupported[PAD_PLAYER_COUNT];
    uint32 count;
} PAD_DECODE_STATE;

extern PAD_REPORT pad_reports[PAD_PLAYER_COUNT];
extern PAD_DECODE_STATE pad_decode;

typedef struct PAD_GAME_STATE
{
    uint32 port[PAD_PLAYER_COUNT];
    uint32 sample[PAD_PLAYER_COUNT];
    uint32 prev[PAD_PLAYER_COUNT];
    sint32 analog_x[PAD_PLAYER_COUNT], analog_y[PAD_PLAYER_COUNT];
} PAD_GAME_STATE;

extern PAD_GAME_STATE pad_game;

typedef struct PAD_INPUT_FRAME
{
    uint32 sample[PAD_PLAYER_COUNT];
} PAD_INPUT_FRAME;

/* Binary input stream: header followed by eight-byte frame records */
typedef struct PAD_INPUT_STREAM
{
    uint32 count;
    uint32 seed;
    PAD_INPUT_FRAME frame[];
} PAD_INPUT_STREAM;

enum
{
    PAD_STREAM_LIVE,
    PAD_STREAM_PLAY,
    PAD_STREAM_RECORD
};

typedef struct PAD_STREAM_STATE
{
    uint32 mode;
    uint32 count;
    uint32 cursor;
    uint32 capacity;
    PAD_INPUT_STREAM *data;
} PAD_STREAM_STATE;

extern PAD_STREAM_STATE pad_stream;

typedef struct PAD_MENU_STATE
{
    uint32 prev[PAD_PLAYER_COUNT];
    uint32 pressed[PAD_PLAYER_COUNT];
    uint32 last_raw;
} PAD_MENU_STATE;

extern PAD_MENU_STATE pad_menu;

typedef enum PAD_BIND
{
    PAD_BIND_CROSS,
    PAD_BIND_CIRCLE,
    PAD_BIND_SQUARE,
    PAD_BIND_TRIANGLE,
    PAD_BIND_L1,
    PAD_BIND_R1,
    PAD_BIND_COUNT
} PAD_BIND;

enum
{
    PAD_SCHEME_COUNT = 8,
    PAD_ACTION_COUNT = 6
};

typedef struct PAD_ACTION_STATE
{
    uint8 cfg[PAD_PLAYER_COUNT];
    uint8 latch[PAD_PLAYER_COUNT][PAD_ACTION_COUNT];
} PAD_ACTION_STATE;

extern PAD_ACTION_STATE pad_action;
extern const uint8 pad_bindings[PAD_SCHEME_COUNT][PAD_BIND_COUNT];
void pad_host_publish(uint32 connected, uint32 buttons0, uint32 buttons1);
sint32 npc_update_attack_motion(sint32 idx);
sint32 npc_attack_beams(sint32 idx);
extern uint32 platform_interrupts_enabled;
void platform_interrupts_disable(void);
void platform_interrupts_enable(void);
void card_backend_init(uint32 mode);
void card_backend_start(void);
void card_pad_init(uint32 mode);
void card_pad_start(void);

DIRENTRY *card_backend_first_file(const char *path, DIRENTRY *entry);
sint32 card_backend_erase(const char *path);
uint32 npc_build_attack_vectors(sint32 idx, uint32 scale, uint32 output[16]);
sint32 npc_emit_attack_effects(sint32 idx);
sint32 regions_apply_radial_hit(const FIXED_VEC3 *pos, sint32 radius, uint32 retained_word);
sint32 packet_color_half(const uint16 packet[9]);
sint32 npc_attack_electric(sint32 idx);
sint32 npc_move_oscillating(sint32 idx);
uint32 object_draw_scene_gated(const MODEL_OBJECT *model);
sint32 npc_finish_grapple(sint32 idx);
sint32 npc_fire_fan(sint32 idx);
sint32 model_update_spinning(MODEL_OBJECT *model);
sint32 npc_throw_items(sint32 idx);
void object_draw_depth_bob(const MODEL_OBJECT *model);
sint32 npc_update_fall(sint32 idx);
void object_draw_repeated_pair(const MODEL_OBJECT *model);
sint32 actor_hit_effects(sint32 idx);
uint32 object_draw_shadowed_model(const MODEL_OBJECT *model);
uint32 actor_transform_point(const POSE_MATRIX *matrix, uint32 point[3]);
uint32 object_draw_phase_gated(const MODEL_OBJECT *model);
uint32 object_draw_player_facing(MODEL_OBJECT *model);
uint32 object_draw_stage_gated(const MODEL_OBJECT *item);
uint32 object_draw_raised(const MODEL_OBJECT *item);
uint32 object_draw_base_raised(const MODEL_OBJECT *item);
uint32 pad_stream_play(PAD_INPUT_STREAM *data, uint32 byte_len);
uint32 pad_stream_rec(PAD_INPUT_STREAM *data, uint32 byte_len);
uint32 object_draw_rotated_packets(const MODEL_OBJECT *item);
uint32 object_draw_depth_rotated_packets(const MODEL_OBJECT *item);
uint32 object_draw_scene_active(const MODEL_OBJECT *item);
uint32 object_draw_scene_early(const MODEL_OBJECT *item);
uint32 object_draw_depth_fade(const MODEL_OBJECT *item);
uint32 object_draw_near_fade(const MODEL_OBJECT *item);
uint32 object_draw_depth_pair(const MODEL_OBJECT *item);
uint32 object_draw_visible_yaw(const MODEL_OBJECT *item);
sint32 npc_begin_attack_wait(sint32 idx);
sint32 npc_update_delay(sint32 idx);
intptr npc_update_action(sint32 idx);
uint32 object_draw_side_spin(const MODEL_OBJECT *model);
uint32 object_draw_camera_yaw_outer(const MODEL_OBJECT *item);
uint32 object_draw_kind_bias(const MODEL_OBJECT *item);
uint32 object_draw_stage_bias(const MODEL_OBJECT *item);
uint32 object_draw_flag_256(const MODEL_OBJECT *item);
void object_draw_height_offset(const MODEL_OBJECT *model);
uint32 object_draw_scene_variant(const MODEL_OBJECT *item);
uint32 object_draw_flag_512(const MODEL_OBJECT *item);
uint32 object_draw_quarter_turn(const MODEL_OBJECT *item);
void object_draw_dimmed_yaw(const MODEL_OBJECT *item);
void object_draw_inverted_roll(const MODEL_OBJECT *item);
void object_draw_offset_spinning_pair(const MODEL_OBJECT *model);
sint32 math_distance_xz(uint32 x0, uint32 z0, uint32 x1, uint32 z1);
sint32 npc_approach_target(sint32 idx);
sint32 npc_update_ranged_attack(sint32 idx);
uint32 object_update_yaw(MODEL_OBJECT *item);

void object_draw_settled_debris(const MODEL_OBJECT *item);
uint32 pad_game_poll(void);
void pad_stream_disable(void);
sint32 level_run(void);
uint32 object_draw_yaw_flag_128(const MODEL_OBJECT *item);
uint32 object_draw_camera_yaw_inner(const MODEL_OBJECT *item);
uint32 object_draw_wave(MODEL_OBJECT *item);
uint32 object_draw_visible_player_facing(MODEL_OBJECT *model);
uint32 object_draw_offset_shadow(const MODEL_OBJECT *item);
void object_draw_raised_spinning_pair(const MODEL_OBJECT *model);

sint32 game_init_devices(void);
void pad_host_init(void);
uint8 *pad_action_clear(sint32 player);

sint32 game_cold_reset(void);
uint32 frame_draw_scene(void);
uint32 frame_prepare(void);
sint32 frame_update(void);
void frame_clock_start(void);
void frame_clock_stop(void);

sint32 pad_count_devices(void);
sint32 pad_refresh_ports(void);
sint16 pad_decode_buttons(sint32 port);
void pad_stream_reset(void);
uint32 pad_menu_poll(sint32 player);
sint32 menu_update_frame(void);
sint32 menu_update_selection_frame(uint32 *retained_s3);
void menu_prepare_frame(void);
void object_transform_camera(const MODEL_OBJECT *item, const EFFECT *effect);
sint32 player_update(void);
uint32 pad_game_buttons(uint32 pad);
uint8 *pad_action_press(uint32 player, uint32 buttons);
uint8 *pad_action_hold(uint32 player, uint32 buttons);
extern uint32 random_seed;
uint32 random_next(void);
uint32 object_update_impact_spr(MODEL_OBJECT *item);

/* Required runtime services. Unbound services fail explicitly, never silently
 * stand in for an implementation. Audit harnesses may bind recording adapters. */
typedef struct GAME_SERVICES
{
    void (*audit_input_event)(uint32 event); /* Optional diagnostic pad source; NULL in normal runs. */
    void (*frontend_frame)(void);
    uint32 (*spu_key)(sint32 enabled, uint32 channels);
    void (*return_to_frontend)(void);
    sint32 (*play_effect)(sint32 effect, sint32 volume, sint32 pitch);
    sint32 (*spu_voice_attributes)(SpuVoiceAttr *attributes);
    sint32 (*spu_key_on_attributes)(SpuVoiceAttr *attributes);
    sint32 (*col_event)(COLLISION_REFERENCE target, COLLISION_REGION *obstacle, struct MODEL_OBJECT *model, const COLLISION_POINT *contact, sint32 actor_idx);
} GAME_SERVICES;

extern GAME_SERVICES game_services;

uint32 object_draw_rotated(const MODEL_OBJECT *item);

void object_draw_pitched_spinning_pair(const MODEL_OBJECT *model);

void object_draw_phased_effects(const MODEL_OBJECT *model);

sint32 object_update_orbit_sound(MODEL_OBJECT *model);

uint32 menu_present(void);
uint32 menu_initialize(void);
uint32 menu_run(void);

sint32 npc_create(sint32 type, uint32 x, uint32 z, sint32 yaw, sint32 flag, sint32 timer, uint32 extra);

sint32 npc_update_entry_delay(sint32 idx);

sint32 npc_turn_to_point(sint32 idx, uint32 x, uint32 z);

sint32 npc_steer_to_point(sint32 idx, uint32 x, uint32 z, uint32 flags);

intptr npc_update_actor(sint32 idx);

sint32 npc_react_to_target(sint32 idx);

sint32 npc_update(void);

void pause_refresh_pad(void);

sint32 npc_attack_players(sint32 idx, sint32 attachment);

#endif
