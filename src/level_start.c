#include "game.h"

LEVEL_START level_starts[LEVEL_START_COUNT];

/* Place players and camera using stage coordinates in Q16 */
GDB_CALL uint32 level_start_players(void)
{
    FUNCTION_MARKER(0x80014808, "GAME.EXE");
    uint32 stage = game_session.stage, x, z, dx, dz, i = 0, count;
    sint32 angle;
    if (stage >= LEVEL_START_COUNT)
        diagnostic_stop(__FUNCTION__, __FILE__, __LINE__);
    const LEVEL_START *start = level_starts + stage;
    x = (uint32)(sint32)start->x << 16;
    z = (uint32)(sint32)start->z << 16;
    angle = start->angle;
    count = game_session.player_count;
    if ((sint32)count > 0)
        do
        {
            pad_action_clear(i);
            ACTOR_ORIENTATION *orientation = actor_orientation + i;
            ACTOR_MOVEMENT *movement = actor_movement + i;
            ACTOR_CONTACT *contact = actor_contacts + i;
            contact->captive = ACTOR_SLOT_NONE;
            count = game_session.player_count;
            ++i;
            orientation->yaw = (sint16)angle;
            movement->desired_yaw = (uint16)((uint32)angle << 5);
            orientation->roll_rate = 0;
            orientation->roll = 0;
        } while ((sint32)i < (sint32)count);
    if (game_session.player_count == 1)
    {
        actor_positions[0].x.bits = x;
        actor_positions[0].y.bits = 0;
        actor_positions[0].z.bits = z;
    }
    else
    {
        dx = 192 * (sint32)sin_table[MATH_QUARTER_TURN + angle];
        dz = 192 * (sint32)sin_table[angle];
        actor_positions[0].x.bits = x + dx;
        actor_positions[0].z.bits = z + dz;
        actor_positions[1].x.bits = x - dx;
        actor_positions[1].z.bits = z - dz;
    }
    x = actor_positions[0].x.bits;
    z = actor_positions[0].z.bits;
    camera.angles.yaw_bits = (uint16)(((uint32)angle << 5) - 32768u);
    camera.pos.y.bits = 0xfe800000u;
    camera.velocity.z.bits = 0;
    camera.velocity.y.bits = 0;
    camera.velocity.x.bits = 0;
    camera.pos.x.bits = x;
    camera.pos.z.bits = z;
    return 0xfe800000u;
}

GDB_CALL void level_reset(void)
{
    FUNCTION_MARKER(0x80018318, "GAME.EXE");
    uint32 i, stage, shade;
    col_regions.sel = 0;
    level_transition.restart_timer = 0;
    level_transition.continue_choice = 0;
    for (i = 0; i < EFFECT_COUNT; i++)
        effects[i].kind = 0xffffffff;
    for (i = 0; i < DYNAMIC_OBJECT_COUNT; i++)
        dynamic_objects[i].kind = 0xffffffff;
    stage = game_session.stage;
    if (stage >= LEVEL_SCRIPT_STAGE_COUNT)
        diagnostic_stop(__FUNCTION__, __FILE__, __LINE__);
    level_script_cursor = level_script_starts[stage];
    level_wave.population_gate = 1;
    level_wave.spawn_delay = 0;
    level_wave.pending_spawns = 0;
    level_wave.remaining_enemies = 0;
    actor_col.reaction_deadline = 0;
    camera.tracking.visibility_mask = 0;
    camera.tracking.hold_bits = 0;
    level_transition.continue_phase = 0;
    game_session.tick = 0;
    camera.tracking.pitch_control_bits = 0;
    camera.angles.pitch_bits = 0;
    disp_buffers.pulse_active = 0;
    game_pause_flags = 0;
    level_transition.route_exit_tick = 0;
    disp_buffers.pulse_phase = 0;
    level_script_scene.relative = 0;
    level_script_scene.remaining_landings = 0;
    level_script_scene.oz = 0;
    level_script_scene.ox = 0;
    stage_animation.phase[0] = 0;
    stage_animation.hold_ticks = 0;
    if (stage == 2)
        stage_animation.phase[0] = 32;
    lift.travel = 0;
    lift.pos = 0;
    lift.speed = 0;
    lift.cycle = 0;
    lift.stop_phase = 2;
    game_session.scene_phase = 0;
    level_transition.exit_requested = 0;
    level_transition.finish_tick = 0;
    level_wave.paused = 0;
    level_wave.population_gate = 0xffffffff;
    npc_behavior_cycle.progress = 0;
    npc_behavior_cycle.advance = 0;
    action_history_reset();
    item_pickup.nearby[1] = 0xffffffff;
    item_pickup.nearby[0] = 0xffffffff;
    stage = game_session.stage;
    stage_animation.lift_auto_cycle = stage == 2;
    if (stage == 2)
        level_wave.paused = 1;
    stage = game_session.stage;
    shade = (stage == 0 || stage - 8 < 2 || stage - 10 < 2 || stage - 12 < 2 || stage == 16 || stage == 19 || stage - 21 < 2 || stage == 24) ? 0xffffffd0 : 0;
    actor_ambient_shade = shade;
    for (i = 0; i < 8; i++)
    {
        shade = (uint8)actor_ambient_shade;
        actor_lighting[i].b = shade;
        actor_lighting[i].g = shade;
        actor_lighting[i].r = shade;
        actor_companions[i] = ACTOR_SLOT_NONE;
        actor_bonus_counts[i] = 0;
        if (actor_health[i].hp <= 0)
            actor_lifecycle[i].death_ticks = 62;
    }
    level_script_scene.object = (SCENE_OBJECT_REFERENCE){0};
    for (i = 0; i < STAGE_AUDIO_SLOT_COUNT; i++)
        stage_audio_slots[i].voice = -1;
}

enum
{
    LEVEL_LOADING_STAGE_COUNT = 26,
    LEVEL_BONUS_ITEM_CHOICE_COUNT = 4
};

static const sint16 loading_steps[LEVEL_LOADING_STAGE_COUNT] = {389, 357, 332, 321, 327, 385, 350, 403, 365, 365, 350, 370, 369, 373, 321, 378, 347, 348, 341, 324, 382, 351, 360, 323, 286, 0};

static const sint16 bonus_item_types[LEVEL_BONUS_ITEM_CHOICE_COUNT] = {499, 504, 500, 501};

GDB_CALL void level_start_playback(void)
{
    sint32 i, type, slot, track;
    uint32 stage = game_session.stage;
    switch (stage)
    {
        case 0:
            track = 22;
            break;
        case 1:
        case 4:
            track = 13;
            break;
        case 2:
        case 3:
        case 23:
            track = 15;
            break;
        case 5:
            track = 16;
            break;
        case 6:
        case 13:
            track = 25;
            break;
        case 7:
        case 8:
        case 9:
            track = 18;
            break;
        case 10:
        case 11:
            track = 17;
            break;
        case 12:
        case 14:
            track = 23;
            break;
        case 15:
        case 16:
        case 19:
            track = 19;
            break;
        case 17:
        case 18:
            track = 21;
            break;
        case 22:
            track = 14;
            break;
        case 24:
            track = 26;
            break;
        default:
            track = 20;
            break;
    }
    cd_music_start_track(track);
    disp_prepare_scene();
    stage_audio_start();
    if (demo.cur_idx != 0xffffffff)
    {
        uint32 byte_len = resource_load(demo_config[demo.cur_idx].file_idx, resource_arenas.scratch.context + 256, NULL, 0, &demo_file, NULL);
        pad_stream_play((PAD_INPUT_STREAM *)demo_file.data, byte_len);
    }
    if (level_transition.bonus_item_pending == 1)
    {
        level_transition.bonus_item_pending = 0;
        for (i = 0; i < (sint32)game_session.player_count; i++)
        {
            type = bonus_item_types[random_next() & (LEVEL_BONUS_ITEM_CHOICE_COUNT - 1)];
            if (i >= ACTOR_COUNT)
            {
                /* TODO: Investigate a bonus grant outside the native actor pool */
                DIAGNOSTIC(fprintf(stderr, "Invalid bonus actor slot %d player count %u stage %u tick %u\n", i, game_session.player_count, game_session.stage, game_session.tick));
                diagnostic_stop(__FUNCTION__, __FILE__, __LINE__);
            }
            if (actor_health[i].hp > 0)
            {
                if (type != 504)
                    actor_items[i].slot[ACTOR_ITEM_LEFT] = -1;
                slot = dynamic_objects_spawn_item(type, actor_positions[i].x.bits, 1, actor_positions[i].z.bits, 0);
                actor_items[i].slot[type == 504 ? ACTOR_ITEM_LEFT : ACTOR_ITEM_RIGHT] = (sint8)slot;
                if (type == 504)
                    actor_items[i].slot[ACTOR_ITEM_RIGHT] = -1;
            }
        }
    }
}

GDB_CALL uint32 level_initialize(void)
{
    const RESOURCE_LOADER *tex;
    uint32 stage = game_session.stage;
    WORLD_OBJECT_TYPE_GROUP_ID types;
    if (stage >= LEVEL_LOADING_STAGE_COUNT)
    {
        /* TODO: Investigate a level initialization outside the original step table */
        DIAGNOSTIC(fprintf(stderr, "Invalid level loading stage %u tick %u\n", stage, game_session.tick));
        diagnostic_stop(__FUNCTION__, __FILE__, __LINE__);
    }
    loading_begin((sint32)loading_steps[stage]);
    npc_behavior_load();
    level_reset();
    character_resources_begin();
    stage = game_session.stage;
    resource_load(stage + 5, 0x116000, NULL, TEXTURE_ARCHIVE_CAPACITY, &tex_file, NULL);
    stage = game_session.stage;
    if (stage >= WORLD_OBJECT_STAGE_COUNT)
        diagnostic_stop(__FUNCTION__, __FILE__, __LINE__);
    types = world_object_type_grps[stage];
    tex = stage_resource_loaders[STAGE_RESOURCE_TEX][stage];
    world_object_active_types = types;
    resource_loader_call(RESOURCE_LOADER_TEXTURE, tex);
    tex_load_grps(0);
    sound_samples_load(game_session.stage + 171);
    level_load_common_models();
    stage = game_session.stage;
    if (stage >= RESOURCE_STAGE_COUNT)
        diagnostic_stop(__FUNCTION__, __FILE__, __LINE__);
    resource_loader_call(RESOURCE_LOADER_STAGE, stage_resource_loaders[STAGE_RESOURCE_ACTORS][stage]);
    character_load_players();
    world_objects_build_regions();
    actors_reset();
    if (game_session.player_count == 1)
        actor_health[1].hp = 0;
    pad_refresh_ports();
    return level_start_players();
}
