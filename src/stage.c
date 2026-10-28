#include <stdio.h>

#include "game.h"

GDB_CALL void stage_update(void)
{
    uint32 stage, phase, step, base, i, timer;
    COLLISION_REGION *region;
    sint32 actor;
    uint32 y;
    FUNCTION_MARKER(0x8001e168, "GAME.EXE");
    timer = level_script_scene.event_timer;
    if ((sint32)timer > 0)
        level_script_scene.event_timer = timer - 1;
    stage = game_session.stage;
    if (stage == 0)
    {
        stage_animation_scroll_stage0();
        return;
    }
    if (stage == 1)
    {
        stage_animation_scroll_stage1();
        return;
    }
    if (stage == 2)
    {
        uint32 flags = lift.cycle, state, voice;
        if (!(flags & 1))
        {
            phase = lift.pos;
            if (!phase)
            {
                state = lift.stop_phase;
                if (!state)
                {
                    timer = stage_animation.phase[0];
                    lift.speed = 0;
                    level_wave.paused = 1;
                    if (!timer)
                    {
                        sound_voice_play(124, 255, 5888);
                        sound_voice_play(234, 255, 6144);
                    }
                    if ((lift.cycle & 2) || (sint32)game_session.tick >= 61)
                    {
                        timer = stage_animation.phase[0] + 1;
                        stage_animation.phase[0] = timer;
                        if (timer == 32)
                        {
                            level_wave.paused = 0;
                            lift.stop_phase = 1;
                        }
                    }
                }
                else if (state == 1)
                {
                    uint32 limit = level_wave.remaining_enemies;
                    level_wave.paused = 0;
                    if ((sint32)limit > 0)
                        lift.stop_phase = 2;
                }
                else if (state == 2 && (sint32)game_session.tick >= 61)
                {
                    if (stage_animation.phase[0] == 30u)
                        sound_voice_play(124, 255, 5888);
                    for (actor = 0; actor < 8; actor++)
                        if (!actor_is_defeated(actor) && (sint32)actor_positions[(uint32)actor].z.bits > 0x02000000)
                            break;
                    if (stage_animation.lift_auto_cycle == 1)
                    {
                        if (actor == 8)
                        {
                            timer = stage_animation.phase[0];
                            if ((sint32)timer > 0)
                                stage_animation.phase[0] = timer - 1;
                        }
                        if (level_wave.paused == 1)
                        {
                            timer = stage_animation.phase[0];
                            if ((sint32)timer > 0)
                                stage_animation.phase[0] = timer - 1;
                            if (!stage_animation.phase[0])
                            {
                                phase = lift.pos;
                                lift.stop_phase = 0;
                                lift.pos = phase + 1;
                            }
                        }
                    }
                    else
                    {
                        region = col_regions.named[0];
                        level_transition.exit_requested = 1;
                        region->col_kind = 0;
                    }
                }
                phase = lift.pos;
            }
            if ((sint32)phase > 0)
            {
                step = lift.speed;
                if ((sint32)step < 64)
                {
                    lift.speed = step + 1;
                    step = lift.speed;
                }
            }
            else
                step = lift.speed;
        }
        else
        {
            phase = lift.pos;
            if ((sint32)phase > 2048)
            {
                step = lift.speed;
                if ((sint32)step >= 2)
                {
                    lift.speed = step - 1;
                    step = lift.speed;
                }
            }
            else
                step = lift.speed;
        }
        phase = lift.pos;
        voice = stage_audio_slots[0].voice;
        lift.pos = phase + step;
        sound_voice_pitch(voice, 4 * step + 5120);
        phase = lift.pos;
        if ((sint32)phase >= 4096)
        {
            flags = lift.cycle;
            lift.pos = phase - 4096;
            lift.cycle = flags + 1;
            phase = lift.pos;
        }
        y = ((uint32)((sint32)((phase * 3) << 9) / 4096) - 3600) << 15;
        base = world_objects.linked_first;
        for (i = 0; i < 4; i++, y += 0x03000000)
        {
            uint32 idx = base + i;
            /* TODO: Investigate a lift link outside the world object pool */
            if (idx >= WORLD_OBJECT_COUNT)
            {
                DIAGNOSTIC(fprintf(stderr, "Invalid lift object %u base %u pair %u stage %u tick %u\n", idx, base, i, game_session.stage, game_session.tick));
                diagnostic_stop(__FUNCTION__, __FILE__, __LINE__);
            }
            world_objects.object[idx].pos.y.bits = y;
            idx += 4;
            /* TODO: Investigate a paired lift link outside the world object pool */
            if (idx >= WORLD_OBJECT_COUNT)
            {
                DIAGNOSTIC(fprintf(stderr, "Invalid paired lift object %u base %u pair %u stage %u tick %u\n", idx, base, i, game_session.stage, game_session.tick));
                diagnostic_stop(__FUNCTION__, __FILE__, __LINE__);
            }
            world_objects.object[idx].pos.y.bits = y;
        }
        stage_animation_scroll_lift();
        return;
    }
    if (stage == 3)
    {
        uint32 side, kind, height;
        if (game_session.scene_phase == 1 && players_past_z(0x00200000))
        {
            sound_voice_play(19, 255, 6144);
            sound_voice_play(200, 255, 6144);
            game_session.scene_phase = 2;
            disp_buffers.pulse_active = 1;
            disp_buffers.pulse_phase = 0;
            for (side = 0; side < 2; side++)
            {
                uint32 x = side ? 0x03000000 : 0xfd000000;
                effect_spawn(893, x, 0xfe800000, 0x03c00000);
                effect_spawn(893, x, 0xff000000, 0x03c00000);
                effect_spawn(893, x, 0xff800000, 0x03c00000);
                effect_spawn(603, x, 0, 0x03c00000);
            }
            disp_buffers.shake_ticks = 16;
            disp_buffers.flash_ticks = 4;
            sound_voice_play(19, 192, 4864);
            for (side = 0; side < 2; side++)
                for (i = 0; i < 16; i++)
                {
                    kind = (random_next() & 3) + 902;
                    height = (0 - (random_next() & 255)) << 16;
                    effect_spawn_debris_aimed(kind, side ? 0x03000000 : 0xfd000000, height, 0x03c00000, side ? 512 : 1536);
                }
            sound_voice_play(19, 128, 3584);
            level_wave.paused = 0;
        }
        disp_update_pulse();
        return;
    }
    if (stage == 4)
    {
        stage_animation_update_stage4();
        return;
    }
    if (stage == 6)
    {
        uint32 result, tick, count, hits = 0;
        tick = game_session.tick;
        result = (sint32)tick < 61;
        if (result)
            return;
        phase = game_session.scene_phase;
        result = tick & 7;
        if (!phase || result)
            return;
        result = world_objects.count;
        if ((sint32)result <= 0)
            return;
        for (i = 0;; i++)
        {
            /* TODO: Investigate stage destruction scanning outside the world pool */
            if (i >= WORLD_OBJECT_COUNT)
            {
                DIAGNOSTIC(fprintf(stderr, "Invalid stage destruction object %u count %u stage %u tick %u\n", i, world_objects.count, game_session.stage, game_session.tick));
                diagnostic_stop(__FUNCTION__, __FILE__, __LINE__);
            }
            MODEL_OBJECT *item = world_objects.object + i;
            if (item->kind - 200 < 2)
            {
                sound_voice_play(19, 255, 6144);
                disp_buffers.shake_ticks = 16;
                disp_buffers.flash_ticks = 4;
                for (base = 0; base < 4; base++)
                {
                    uint32 kind = (random_next() & 3) + 902;
                    const FIXED_VEC3 *pos = &item->pos;
                    uint32 x = pos->x.bits;
                    uint32 z = pos->z.bits;
                    effect_spawn_debris(kind, x, 0, z);
                }
                item->kind = 55;
                if (++hits >= 5)
                    return;
            }
            count = world_objects.count;
            result = (sint32)(i + 1) < (sint32)count;
            if (!result)
                return;
        }
    }
    if (stage == 7)
    {
        uint32 result, timer, next;
        COLLISION_REGION *target;
        COLLISION_REGION **target_slot;
        if (level_transition.exit_requested == 1)
        {
            timer = stage_animation.phase[1];
            if (timer == 48)
                col_regions.named[0]->col_kind = 0;
            else
                stage_animation.phase[1] = timer + 2;
        }
        if (level_wave.paused != 1)
            return;
        phase = game_session.scene_phase;
        if (!phase)
        {
            timer = stage_animation.phase[0];
            if (timer)
            {
                timer++;
                stage_animation.phase[0] = timer;
                if (timer != 32)
                    return;
                region = col_regions.named[1];
                game_session.scene_phase = 1;
                region->col_kind = 0;
                return;
            }
            result = (uint32)players_past_z(-0xed80000);
            if (result)
            {
                target = col_regions.named[2];
                stage_animation.phase[0] = 1;
                target->col_kind = 3;
                result = (uint32)sound_voice_play(124, 192, 5632);
            }
            return;
        }
        if (phase == 2 || phase == 4)
        {
            next = phase + 1;
            timer = stage_animation.phase[0];
            if (timer)
            {
                timer++;
                stage_animation.phase[0] = timer;
                if (timer == 32)
                {
                    target = col_regions.named[phase == 2 ? 5 : 3];
                    game_session.scene_phase = next;
                    target->col_kind = 0;
                }
                return;
            }
            result = (uint32)players_before_x(phase == 2 ? 0x07800000 : (sint32)0xf3d00000);
            if (result)
            {
                col_regions.named[phase == 2 ? 6 : 4]->col_kind = 3;
                result = (uint32)sound_voice_play(124, 192, 5632);
                if (phase == 4)
                {
                    result = (uint32)sound_voice_play(231, 112, 6144) + 1;
                    stage_audio_slots[0].voice_plus_one = result;
                }
                stage_animation.phase[0] = 1;
            }
            return;
        }
        if (phase == 1)
        {
            result = (uint32)players_past_z(-0x9d80000);
            target_slot = &col_regions.named[1];
        }
        else if (phase == 3)
        {
            result = (uint32)players_before_x(0x04200000);
            target_slot = &col_regions.named[5];
        }
        else if (phase == 5)
        {
            result = (uint32)players_before_x(-0xe880000);
            target_slot = &col_regions.named[3];
        }
        else
            return;
        if (result)
        {
            region = *target_slot;
            game_session.scene_phase = phase + 1;
            stage_animation.phase[0] = 0;
            level_wave.paused = 0;
            region->col_kind = 3;
        }
        return;
    }
    if (stage == 10)
    {
        uint32 result = (uint32)(sint32)level_wave.paused;
        if (result)
        {
            result = game_session.scene_phase;
            if (result == 1)
            {
                result = (uint32)players_past_z(-0x19800000);
                if (result)
                {
                    level_wave.paused = 0;
                    game_session.scene_phase = 2;
                }
            }
        }
        return;
    }
    if (stage == 11)
    {
        uint32 result = (uint32)(sint32)level_wave.paused;
        if (!result)
            return;
        result = game_session.scene_phase;
        if (result != 1u)
            return;
        if (players_past_z(0x07800000))
        {
            region = col_regions.named[2];
            game_session.scene_phase = 2;
            level_wave.paused = 0;
            region->col_kind = 3;
        }
        return;
    }
    if (stage - 12u < 2u)
    {
        if (level_wave.paused && !game_session.scene_phase && players_past_z(0))
        {
            region = col_regions.named[2];
            phase = game_session.scene_phase;
            level_wave.paused = 0;
            region->col_kind = 3;
            game_session.scene_phase = phase + 1;
        }
        stage_animation_scroll_stage12();
        return;
    }
    stage = game_session.stage;
    if (stage == 14)
    {
        stage_animation_update_stage14();
        return;
    }
    if (stage == 17)
    {
        stage_animation_scroll_stage17();
        return;
    }
    if (stage == 20)
    {
        uint32 result = (uint32)(sint32)level_wave.paused;
        if (result)
        {
            if (players_before_x(0x11d00000))
            {
                region = col_regions.named[2];
                level_wave.paused = 0;
                region->col_kind = 3;
            }
        }
        return;
    }
}

GDB_CALL void stage_update_world(void)
{
    FUNCTION_MARKER(0x8001d640u, "GAME.EXE");
    uint32 i, object, result, tick = 0, stage, phase, prev_car = 0;
    const OBJECT_CALLBACK *target;
    sint32 count, timer;
    timer = (sint32)disp_buffers.flash_ticks;
    if (timer > 0)
        disp_buffers.flash_ticks = (uint32)timer - 1;
    timer = (sint32)disp_buffers.shake_ticks;
    if (timer > 0)
        disp_buffers.shake_ticks = (uint32)timer - 1;
    count = (sint32)game_session.player_count;
    if (count > 0)
        tick = game_session.tick;
    for (i = 0; (sint32)i < count; i++)
    {
        /* TODO: Investigate a player count beyond the native actor pool */
        if (i >= ACTOR_COUNT)
        {
            DIAGNOSTIC(fprintf(stderr, "Invalid sequence player %u count %d stage %u tick %u\n", i, count, game_session.stage, game_session.tick));
            diagnostic_stop(__FUNCTION__, __FILE__, __LINE__);
        }
        ACTOR_POSE *pose = &actor_poses[i];
        sint32 goal = 0, val;
        if (actor_animations[i].id == 69)
            goal = (sint32)((uint32)sin_table[((tick << 5) & 0xfe0) >> 1] << 16) >> 22;
        val = (sint32)(pose->sway_yaw << 20) >> 20;
        pose->sway_yaw = ((uint32)val + (uint32)((goal - val) >> 3)) & 0xffe;
    }
    for (i = 0; i < 8; i++)
    {
        ACTOR_ANIMATION *playback = actor_animations + i;
        ACTOR_HEALTH *health = actor_health + i;
        if (actor_types[i] == ACTOR_TYPE_NONE)
            continue;
        timer = health->stun;
        if (timer > 0)
            health->stun = (uint16)(timer - 1);
        if (playback->src.kind == ACTOR_ANIMATION_SOURCE_NONE)
            playback->frame = 0xffffffffu;
        if (playback->frame != 0xffffffffu && actor_types[i] != 17)
            animation_update_motion(i);
        col_regions.actor[i].flags = 0;
        regions_apply_actor(i);
    }
    stage = game_session.stage;
    if (!stage)
        game_session.stage0_ticks++;
    if (stage == 23)
    {
        uint32 speed;
        if ((sint32)game_session.tick >= 33)
        {
            speed = lift.speed;
            if ((sint32)speed < 16)
                lift.speed = speed + 1;
        }
        result = lift.travel;
        speed = lift.speed;
        result += speed;
        lift.travel = result;
        if ((sint32)result > 81408)
            lift.travel = 81408;
        result = lift.pos + speed;
        lift.pos = result;
        if ((sint32)result > 40704)
            lift.pos = 40704;
        if (!game_session.scene_phase)
        {
            result = lift.pos;
            if ((sint32)result > 35776)
                lift.pos = result - 6912;
        }
        if (!(uint8)game_session.tick)
        {
            sound_voice_play(242, 127, (sint32)(random_next() & 1023u) + 4864);
            lift.sound_mark = 30;
        }
    }
    count = (sint32)world_objects.count;
    for (i = 0; (sint32)i < count; i++)
    {
        object = 0x800b89f0 + 36u * i;
        /* TODO: Investigate world dispatch beyond the native object pool */
        if (i >= WORLD_OBJECT_COUNT)
        {
            DIAGNOSTIC(fprintf(stderr, "Invalid sequence object %u count %d stage %u tick %u\n", i, count, game_session.stage, game_session.tick));
            diagnostic_stop(__FUNCTION__, __FILE__, __LINE__);
        }
        MODEL_OBJECT *item = world_objects.object + i;
        uint32 callback_type = item->kind;
        /* TODO: Investigate a world kind outside the native callback table */
        if (callback_type >= MODEL_TYPE_COUNT)
        {
            DIAGNOSTIC(fprintf(stderr, "Invalid sequence callback kind %u object %u stage %u tick %u\n", callback_type, i, game_session.stage, game_session.tick));
            diagnostic_stop(__FUNCTION__, __FILE__, __LINE__);
        }
        target = object_callbacks[OBJECT_CALLBACK_UPDATE][callback_type];
        /* Original 1D730 initializes S3=-1 before this callback loop. */
        if (target == &callback_projectile_update_ballistic)
        {
            projectile_update_ballistic(item, 0xffffffffu);
            prev_car = 0;
        }
        else if (target == &callback_helicopter_update)
        {
            helicopter_update_retained(item, prev_car, 0x800bbf78);
            prev_car = 0;
        }
        else if (target)
        {
            object_call(target, item, NULL);
            prev_car = target == &callback_scene_vehicle_update_traffic ? object : 0;
        }
        count = (sint32)world_objects.count;
    }
    result = 1;
    if (level_wave.paused == 1 && game_session.stage == 1)
    {
        result = game_session.scene_phase;
        if (result != 1)
            return;
        result = (uint32)players_before_z(-0x1000000);
        if (!result)
            return;
        phase = game_session.scene_phase;
        count = world_objects.count - 1u;
        level_wave.paused = 0;
        game_session.scene_phase = phase + 1;
        world_objects.count = (uint32)count;
        for (i = 0; (sint32)i < count; i++)
        {
            /* TODO: Investigate stage exit beyond the native object pool */
            if (i >= WORLD_OBJECT_COUNT)
            {
                DIAGNOSTIC(fprintf(stderr, "Invalid exit object %u count %d stage %u tick %u\n", i, count, game_session.stage, game_session.tick));
                diagnostic_stop(__FUNCTION__, __FILE__, __LINE__);
            }
            MODEL_OBJECT *item = world_objects.object + i;
            if (item->kind == 124)
                item->kind = 55;
        }
        count = (sint32)col_regions.count;
        for (i = 0; (sint32)i < count; i++)
        {
            COLLISION_REGION *region;
            if (i >= COLLISION_REGION_COUNT)
                diagnostic_stop(__FUNCTION__, __FILE__, __LINE__);
            region = col_regions.data + i;
            result = (uint32)(sint32)region->col_kind;
            if (!result)
                region->col_kind = 3;
        }
    }
}

static const char demo_exit_french0[] = "APPUYEZ SUR UNE";

static const char demo_exit_french1[] = "TOUCHE POUR QUITTER";

static const char demo_recording_text[] = "RECORDING DEMO";

GDB_CALL sint32 game_update_frame_prefix(void)
{
    uint32 tick, i;
    sint32 second;
    pad_game_poll();
    if (game_services.audit_input_event)
        game_services.audit_input_event(4);
    camera.frame_angles.yaw_bits = (uint16)camera.angles.yaw;
    camera.frame_angles.pitch_bits = (uint16)camera.angles.pitch;
    for (i = 0; i < 3; i++)
        camera.frame_pos.bits[i] = camera.pos.bits[i];
    tick = game_session.tick + 1;
    game_session.tick = tick;
    if ((sint32)tick > 0x40000)
        game_session.tick = 0x10000;
    frame_prepare();
    world_objects_draw();
    characters_draw();
    dynamic_objects_draw();
    player_update();
    npc_update();
    stage_update();
    stage_update_world();
    transform_identity();
    transform_pitch(camera.angles.pitch);
    transform_yaw(camera.angles.yaw);
    effects_draw();
    effects_update();
    dynamic_objects_update();
    /* 14F14..14F30: nullsub46, stop voices, reset input, return4 */
    if ((sint32)level_transition.continue_phase >= 385)
    {
        level_transition_noop();
        sound_voices_stop_all();
        pad_stream_disable();
        return 4;
    }
    second = actor_is_defeated(1);
    if ((sint32)(2 - (uint32)second) != actor_is_defeated(0))
    {
        camera_update_focus();
        uint32 stage = game_session.stage;
        if (stage >= CAMERA_STAGE_COUNT)
            diagnostic_stop(__FUNCTION__, __FILE__, __LINE__);
        camera_update_stage(camera_stage_refs[stage]);
    }
    if (pad_stream.mode == PAD_STREAM_PLAY)
    {
        if (game_settings.language == GAME_LANGUAGE_FRENCH)
        {
            text_center(demo_exit_french0, 170);
            text_center(demo_exit_french1, 190);
        }
        else
            text_center(localization.text[LOCAL_TEXT_PRESS_ANY_EXIT], 180);
    }
    if (game_session.tick & 16)
    {
        if (pad_stream.mode == PAD_STREAM_PLAY)
            text_center(localization.text[LOCAL_TEXT_DEMO], 120);
        else if (pad_stream.mode == PAD_STREAM_RECORD)
            text_center(demo_recording_text, 190);
    }
    hud_update();
    return 0;
}
