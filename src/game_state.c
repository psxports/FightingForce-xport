#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "game.h"
#include "sound_device.h"
#include "psx_spu.h"

/* Reset native game owners and device workspaces for a cold start */
sint32 game_cold_reset(void)
{
    /* Immutable linked-DMA terminal, not initialized game state */
    const LINE_F2 terminal = {.tag = 0x03ffffffu, .rgbc = {.rgbc = 0x40000000u}};
    _Static_assert(sizeof(terminal) == 16, "OT terminal device packet size");
    uint32 i;
    memset(DRAM, 0, sizeof(DRAM));
    sound_heap = (SpuMallocState){0};
    memset(sound_heap_recs, 0, sizeof(sound_heap_recs));
    memset(SCRATCHPAD, 0, sizeof(SCRATCHPAD));
    memset(&geometry_workspace, 0, sizeof(geometry_workspace));
    /* No current matrix exists before transform_reset publishes the native stack */
    geometry_workspace.matrix = NULL;
    geometry_workspace.matrix_storage = GEOMETRY_MATRIX_NULL;
    random_seed = 0;
    free(tex_file.data);
    tex_file = (RESOURCE_FILE){0};
    loading = loading_defaults;
    effect_rotate_cursor = 0;
    memset(effects, 0, sizeof(effects));
    memset(&game_session, 0, sizeof(game_session));
    screenshot = screenshot_defaults;
    demo = demo_defaults;
    scene_playback = (SCENE_PLAYBACK){0};
    background = (BACKGROUND){0};
    free(demo_file.data);
    demo_file = (RESOURCE_FILE){0};
    memset(&disp_buffers, 0, sizeof(disp_buffers));
    memset(&render_buffers, 0, sizeof(render_buffers));
    memset(depth_table, 0, sizeof(depth_table));
    render_style.color.rgbc = 0x2d000000;
    render_style.blend_mode = 0;
    proj_horizon = 0;
    lab_tank_level = 0;
    lift = (LIFT){0};
    suspended = (SUSPENDED){2};
    vehicle_steering = (VEHICLE_STEERING){0};
    train_accel[TRAIN_SIDE_LOW_X] = 50;
    train_accel[TRAIN_SIDE_HIGH_X] = 50;
    stage_animation = (STAGE_ANIMATION){0};
    item_pickup = (ITEM_PICKUP){0};
    memset(actor_items, 0, sizeof(actor_items));
    memset(&action_history, 0, sizeof(action_history));
    memset(animation_banks, 0, sizeof(animation_banks));
    memset(animation_type_banks, 0, sizeof(animation_type_banks));
    text_hint = TEXT_HINT_NONE;
    memset(animation_flags, 0, sizeof(animation_flags));
    animation_fall_exit = 0;
    memset(&resource_io, 0, sizeof(resource_io));
    resource_arenas = resource_arena_defaults;
    for (i = 0; i < sizeof(stage_resource_loaders) / sizeof(stage_resource_loaders[0]); i++)
        for (uint32 j = 0; j < sizeof(stage_resource_loaders[0]) / sizeof(stage_resource_loaders[0][0]); j++)
            stage_resource_loaders[i][j] = NULL;
    memset(level_starts, 0, sizeof(level_starts));
    memset(&world_objects, 0, sizeof(world_objects));
    memset(dynamic_objects, 0, sizeof(dynamic_objects));
    memset(world_object_type_grps, 0, sizeof(world_object_type_grps));
    world_object_active_types = WORLD_OBJECT_TYPES_NONE;
    for (i = 0; i < LEVEL_SCRIPT_STAGE_COUNT; i++)
        level_script_starts[i] = LEVEL_SCRIPT_NONE;
    level_script_cursor = LEVEL_SCRIPT_NONE;
    level_wave = level_wave_defaults;
    memset(&level_script_scene, 0, sizeof(level_script_scene));
    memset(&level_transition, 0, sizeof(level_transition));
    game_pause_flags = 0;
    memset(&cd_music, 0, sizeof(cd_music));
    cd_music.track = 0xffffffffu;
    memset(&sound_voices, 0, sizeof(sound_voices));
    for (i = 0; i < STAGE_AUDIO_SLOT_COUNT; i++)
        stage_audio_slots[i].voice = 0;
    memset(character_name_types, 0, sizeof(character_name_types));
    memset(character_actor_names, 0, sizeof(character_actor_names));
    for (i = 0; i < sizeof(object_callbacks) / sizeof(object_callbacks[0]); i++)
        for (uint32 j = 0; j < sizeof(object_callbacks[0]) / sizeof(object_callbacks[0][0]); j++)
            object_callbacks[i][j] = NULL;
    for (i = 0; i < sizeof(col_callbacks) / sizeof(col_callbacks[0]); i++)
        col_callbacks[i] = NULL;
    memset(&col_drops, 0, sizeof(col_drops));
    for (i = 0; i < sizeof(region_triggers) / sizeof(region_triggers[0]); i++)
        region_triggers[i] = NULL;
    memset(&col_regions, 0, sizeof(col_regions));
    memset(actor_lighting, 0, sizeof(actor_lighting));
    memset(actor_animations, 0, sizeof(actor_animations));
    memset(actor_positions, 0, sizeof(actor_positions));
    memset(actor_orientation, 0, sizeof(actor_orientation));
    memset(actor_health, 0, sizeof(actor_health));
    memset(actor_types, 0, sizeof(actor_types));
    memset(actor_movement, 0, sizeof(actor_movement));
    memset(actor_contacts, 0, sizeof(actor_contacts));
    memset(actor_lifecycle, 0, sizeof(actor_lifecycle));
    memset(actor_bonus_counts, 0, sizeof(actor_bonus_counts));
    memset(actor_bonus_totals, 0, sizeof(actor_bonus_totals));
    memset(actor_companions, 0, sizeof(actor_companions));
    memset(actor_pair_motion, 0, sizeof(actor_pair_motion));
    memset(actor_trails, 0, sizeof(actor_trails));
    memset(actor_combat, 0, sizeof(actor_combat));
    memset(actor_status, 0, sizeof(actor_status));
    memset(actor_grapple_distance, 0, sizeof(actor_grapple_distance));
    memset(actor_score_vals, 0, sizeof(actor_score_vals));
    actor_ambient_shade = 0;
    for (i = 0; i < sizeof(actor_events) / sizeof(actor_events[0]); i++)
        actor_events[i] = NULL;
    memset(actor_poses, 0, sizeof(actor_poses));
    camera = camera_defaults;
    for (i = 0; i < sizeof(camera_stage_refs) / sizeof(camera_stage_refs[0]); i++)
        camera_stage_refs[i] = NULL;
    free(menu_text.data);
    menu_text = (MENU_TEXT){0};
    menu = menu_defaults;
    options_menu = options_menu_defaults;
    game_settings = game_settings_defaults;
    memcpy(high_scores, high_scores_defaults, sizeof(high_scores));
    memset(&hud, 0, sizeof(hud));
    memset(player_scores, 0, sizeof(player_scores));
    memset(player_repeat_counts, 0, sizeof(player_repeat_counts));
    memset(&text_layout, 0, sizeof(text_layout));
    memset(&high_score_workspace, 0, sizeof(high_score_workspace));
    memset(&card_text, 0, sizeof(card_text));
    memset(&card_dialog, 0, sizeof(card_dialog));
    memset(card_prompts, 0, sizeof(card_prompts));
    free(localization.file.data);
    memset(&localization, 0, sizeof(localization));
    card_storage = card_storage_defaults;
    memset(&card_files, 0, sizeof(card_files));
    text_active_gradient = text_gradient_defaults;
    text_gradient_phase = 0;
    character_menu = character_menu_defaults;
    dir_distance = 0;
    for (i = 0; i < sizeof(npc_actions) / sizeof(npc_actions[0]); i++)
        for (uint32 j = 0; j < sizeof(npc_actions[0]) / sizeof(npc_actions[0][0]); j++)
            npc_actions[i][j] = NULL;
    for (i = 0; i < sizeof(npc_updates) / sizeof(npc_updates[0]); i++)
        npc_updates[i] = NULL;
    memset(npc_action_actors, 0, sizeof(npc_action_actors));
    memset(npc_target_priorities, 0, sizeof(npc_target_priorities));
    memset(npc_targets, 0, sizeof(npc_targets));
    memset(npc_target_distances, 0, sizeof(npc_target_distances));
    memset(&actor_targeting, 0, sizeof(actor_targeting));
    memset(&actor_col, 0, sizeof(actor_col));
    actor_col.player_hits_enabled = 1;
    memset(npc_behavior_file, 0, sizeof(npc_behavior_file));
    memset(npc_ai_storage, 0, sizeof(npc_ai_storage));
    memset(npc_ai_capabilities, 0, sizeof(npc_ai_capabilities));
    memset(npc_behavior_rows, 0, sizeof(npc_behavior_rows));
    memset(npc_behavior_actors, 0, sizeof(npc_behavior_actors));
    npc_behavior_cycle = (NPC_BEHAVIOR_CYCLE){0};
    npc_attack_geometry = npc_attack_geometry_defaults;
    npc_fall_speed_limit = npc_fall_speed_limit_default;
    npc_drop_phase = 0;
    npc_update_idx = 0;
    memset(character_name_cursor, 0, sizeof(character_name_cursor));
    sound_settings = sound_settings_defaults;
    sound_reverb_configured = 0;
    free(sound_samples.file.data);
    memset(&sound_samples, 0, sizeof(sound_samples));
    sound_attributes.cur = 0;
    for (i = 0; i < SOUND_VOICE_ATTRIBUTE_COUNT; i++)
        memset(sound_attributes.packet[i].wire, 0, SOUND_VOICE_ATTRIBUTE_BYTES);
    memset(dynamic_item_uses, 0, sizeof(dynamic_item_uses));
    memset(item_corrections, 0, sizeof(item_corrections));
    memset(world_object_height, 0, sizeof(world_object_height));
    memset(world_region_kinds, 0, sizeof(world_region_kinds));
    memset(&terrain_map, 0, sizeof(terrain_map));
    memset(background_img.pix, 0, sizeof(background_img.pix));
    memset(model_refs, 0, sizeof(model_refs));
    memset(model_height, 0, sizeof(model_height));
    memset(&resource_archive, 0, sizeof(resource_archive));
    for (i = 0; i < sizeof(resource_file_names) / sizeof(resource_file_names[0]); i++)
        resource_file_names[i] = NULL;
    memset(tex_grps, 0, sizeof(tex_grps));
    memset(&tex_allocator, 0, sizeof(tex_allocator));
    tex_upload_rects = tex_upload_rect_defaults;
    memset(tex_map, 0, sizeof(tex_map));
    memset(tex_desc.words, 0, sizeof(tex_desc.words));
    memset(tex_base_v, 0, sizeof(tex_base_v));
    memset(&tex_pals, 0, sizeof(tex_pals));
    memset(&tex_effect_pals, 0, sizeof(tex_effect_pals));
    tex_count = 0;
    tex_desc_count = 0;
    memset(&pad_menu, 0, sizeof(pad_menu));
    memset(&pad_action, 0, sizeof(pad_action));
    memset(&pad_game, 0, sizeof(pad_game));
    memset(&pad_decode, 0, sizeof(pad_decode));
    memset(pad_reports, 0, sizeof(pad_reports));
    memset(&pad_stream, 0, sizeof(pad_stream));
    memcpy(psx_addr(RENDER_OT_END, sizeof(terminal)), &terminal, sizeof(terminal));
    /* Reset resource backing after publishing the device terminal */
    memset(resource_storage.data, 0, sizeof(resource_storage.data));
    spu_native_transfer_state->requested_mode = 0;
    spu_native_transfer_state->normalized_mode = 0;
    spu_native_transfer_state->completion = 1;
    spu_native_transfer_state->source_address = 0;
    spu_native_transfer_state->dma_blocks = 0;
    spu_native_transfer_state->address_units = 0;
    /* TODO: Investigate a native completion callback surviving cold reset */
    XASSERT(!spu_native_transfer_state->completion_callback);
    spu_native_transfer_state->completion_callback = NULL;
    /* Heap metadata and its zero caller records were reset above */
    *spu_key_control = (SpuKeyControl){0};
    for (i = 0; i < SPU_VOICE_COUNT; i++)
        spu_sample_notes[i] = SPU_INITIAL_SAMPLE_NOTE;
    /* Address configuration already uses the reviewed fixed SDK constants */
    return 1;
}
