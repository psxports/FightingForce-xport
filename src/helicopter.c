#include <stdio.h>

#include "game.h"

static sint32 move_toward(MODEL_OBJECT *item, sint32 target_x, sint32 target_z)
{
    FUNCTION_MARKER(0x80041f5cu, "GAME.EXE");
    /* TODO: Investigate a missing native helicopter object */
    XASSERT(item);
    sint32 yaw = (sint16)item->angles.yaw_bits >> 5, pitch = item->angles.pitch, roll = item->angles.roll, error, distance, speed;
    sint32 x, z, trig;
    uint32 delta, result, offset, pos;
    x = item->pos.x.whole;
    z = item->pos.z.whole;
    delta = math_dir16(x, z, target_x, target_z) - ((uint32)yaw << 5);
    if ((uint16)(delta + 8192) >= 16385)
    {
        result = item->angles.yaw_bits + ((sint16)delta >> 4);
        item->angles.yaw_bits = (uint16)result;
        return result;
    }
    error = (sint16)delta;
    if (error > 4096)
        error = 4096;
    if (error < -4096)
        error = -4096;
    roll -= (roll + error) >> 4;
    yaw = (yaw - (roll >> 8)) & 2047;
    distance = dir_distance - 3072u;
    if (distance > 3072)
        distance = 3072;
    pitch = pitch + (uint32)((sint32)(((uint32)distance << 2) - pitch) >> 4);
    speed = sin_table[((pitch >> 4) & 0xffeu) >> 1] >> 7;
    offset = yaw * 2u;
    trig = sin_table[offset >> 1];
    pos = item->pos.x.bits;
    item->pos.x.bits = pos + (speed * trig);
    trig = sin_table[MATH_QUARTER_TURN + (offset >> 1)];
    pos = item->pos.z.bits;
    item->pos.z.bits = pos - (speed * trig);
    result = (uint32)yaw << 5;
    item->angles.yaw_bits = (uint16)result;
    item->angles.pitch_bits = (uint16)pitch;
    item->angles.roll_bits = (uint16)roll;
    return result;
}

uint32 helicopter_update(MODEL_OBJECT *item, const PROJECTILE_SOURCE *retained_projectile)
{
    FUNCTION_MARKER(0x800420f8u, "GAME.EXE");
    sint32 player_x = actor_positions[0].x.whole, player_z = actor_positions[0].z.whole, state, delta, distance, pitch, volume, voice, x, z;
    uint32 angle, phase, bob_phase, height, dx, dz, speed, sample, result, volume_word, pan, world_x, world_z;
    uint32 s;
    if ((sint32)game_session.scene_phase < 3)
        return 1;
    /* TODO: Investigate a combat helicopter with a foreign native owner */
    XASSERT(item && item->payload_kind == MODEL_PAYLOAD_COMBAT_HELICOPTER);
    if (item->helicopter.voice_plus_one[0])
    {
        /* TODO: Investigate spatial sound from a helicopter outside both native pools */
        XASSERT(item);
        sound_spatial(&item->pos, &volume_word, &pan);
        volume = sound_volume_adjust(volume_word, 128);
        sound_voice_pan(item->helicopter.voice_plus_one[0] - 1, volume, pan);
    }
    else
    {
        item->helicopter.voice_plus_one[0] = (uint16)(sound_voice_play(243, 255, 6144) + 1);
    }

    {
        /* TODO: Investigate scene publication from an invalid native object operand */
        XASSERT(item);
        SCENE_OBJECT_REFERENCE ref = {0};
        uint32 slot;
        for (slot = 0; slot < WORLD_OBJECT_COUNT; slot++)
            if (item == world_objects.object + slot)
            {
                ref = (SCENE_OBJECT_REFERENCE){slot, SCENE_OBJECT_WORLD};
                break;
            }
        if (!ref.pool)
            for (slot = 0; slot < DYNAMIC_OBJECT_COUNT; slot++)
                if (item == dynamic_objects + slot)
                {
                    ref = (SCENE_OBJECT_REFERENCE){slot, SCENE_OBJECT_DYNAMIC};
                    break;
                }
        /* TODO: Investigate scene publications outside both native object pools */
        XASSERT(ref.pool);
        level_script_scene.object = ref;
    }
    state = item->helicopter.phase;
    if (!state)
    {
        move_toward(item, player_x, player_z);
        if (dir_distance < 3072)
        {
            item->helicopter.phase = 1;
            item->helicopter.bob_phase = 0;
            item->helicopter.hover_height = (uint16)item->pos.y.whole;
        }
    }
    else if ((state = item->helicopter.phase) == 1)
    {
        x = item->pos.x.whole;
        z = item->pos.z.whole;
        angle = math_dir16(x, z, player_x, player_z) + 8192;
        delta = (sint16)(angle - item->angles.yaw_bits);
        if (delta > 4096)
            delta = 4096;
        if (delta < -4096)
            delta = -4096;
        item->angles.yaw_bits = (uint16)(item->angles.yaw_bits + (delta >> 4));
        pitch = item->angles.pitch_bits;
        item->angles.pitch_bits = (uint16)(pitch - ((sint16)pitch >> 3));
        bob_phase = ((uint16)item->helicopter.bob_phase + 32) & 2047;
        item->helicopter.bob_phase = (uint16)bob_phase;
        height = (uint32)(sint32)item->helicopter.hover_height << 16;
        s = sin_table[bob_phase];
        item->pos.y.bits = height + (s << 4);
        if ((uint16)(delta + 1023) < 2047)
        {
            item->helicopter.phase = 2;
            item->helicopter.shot_distance = (uint16)(dir_distance >> 1);
            item->helicopter.voice_plus_one[1] = (uint16)(sound_voice_play(180, 255, 6144) + 1);
        }
    }
    else if (state == 2)
    {
        x = item->pos.x.whole;
        z = item->pos.z.whole;
        angle = math_dir16(x, z, player_x, player_z) + 8192;
        phase = ((angle + (random_next() & 512) - 8448) << 16) >> 21;
        delta = (sint16)(angle - item->angles.yaw_bits);
        if (delta > 4096)
            delta = 4096;
        if (delta < -4096)
            delta = -4096;
        item->angles.yaw_bits = (uint16)(item->angles.yaw_bits + (delta >> 3));
        pitch = item->angles.pitch_bits;
        item->angles.pitch_bits = (uint16)(pitch - ((sint16)pitch >> 3));
        bob_phase = ((uint16)item->helicopter.bob_phase + 32) & 2047;
        item->helicopter.bob_phase = (uint16)bob_phase;
        height = (uint32)(sint32)item->helicopter.hover_height << 16;
        s = sin_table[bob_phase];
        item->pos.y.bits = height + (s << 4);
        if (!(game_session.tick & 1))
        {
            PROJECTILE_SOURCE projectile;
            speed = (uint16)item->helicopter.shot_distance;
            dx = (((sint16)speed) * (uint32)(sint32)sin_table[phase]) << 1;
            dz = (((sint16)speed) * (uint32)(sint32)sin_table[MATH_QUARTER_TURN + phase]) << 1;
            item->helicopter.shot_distance = (uint16)(speed + 128);
            distance = dir_distance;
            if ((sint32)((distance >> 1) + (uint32)distance) < (sint16)(speed + 128))
            {
                item->helicopter.shot_distance = (uint16)(distance >> 1);
                item->helicopter.shot_passes = (uint16)((uint16)item->helicopter.shot_passes + 1);
            }
            world_x = item->pos.x.bits;
            world_z = item->pos.z.bits;
            effect_spawn_timed(879, dx + world_x, 0, world_z - dz, 0, 0, 0);
            projectile.kind = 499;
            projectile.owner = -1;
            /* Preserve reviewed stack carriers or the agreed ground-shot correction */
            projectile.height = retained_projectile ? retained_projectile->height : 0;
            projectile.angle = retained_projectile ? retained_projectile->angle : (uint16)(phase << 5);
            world_x = item->pos.x.bits;
            world_z = item->pos.z.bits;
            if (projectile_query_src(dx + world_x, world_z - dz, &projectile) >= 0)
            {
                world_x = item->pos.x.bits;
                world_z = item->pos.z.bits;
                actor_projectile_impact_src(dx + world_x, world_z - dz, &projectile);
            }
            if (!(random_next() & 3))
            {
                sample = (random_next() & 3) + 136;
                volume = (random_next() & 63) + 16;
                sound_voice_play(sample, volume, 6272);
            }
        }
        if (dir_distance >= 4097)
            item->helicopter.phase = 3;
        if (item->helicopter.shot_passes >= 5)
        {
            item->helicopter.phase = 4;
            level_wave.paused = 0;
        }
        voice = item->helicopter.voice_plus_one[1];
        if (voice && item->helicopter.phase != 2)
            sound_voice_stop(voice - 1);
    }
    else if ((state = item->helicopter.phase) == 3)
    {
        x = item->pos.x.whole;
        z = item->pos.z.whole;
        angle = math_dir16(x, z, player_x, player_z);
        delta = (sint16)(angle - item->angles.yaw_bits);
        if (delta > 4096)
            delta = 4096;
        if (delta < -4096)
            delta = -4096;
        item->angles.yaw_bits = (uint16)(item->angles.yaw_bits + (delta >> 3));
        distance = dir_distance - 3072u;
        if (distance > 3072)
            distance = 3072;
        pitch = item->angles.pitch;
        item->angles.pitch_bits = (uint16)(pitch + (uint32)((sint32)(((uint32)distance << 2) - pitch) >> 4));
        bob_phase = ((uint16)item->helicopter.bob_phase + 32) & 2047;
        item->helicopter.bob_phase = (uint16)bob_phase;
        height = (uint32)(sint32)item->helicopter.hover_height << 16;
        s = sin_table[bob_phase];
        item->pos.y.bits = height + (s << 4);
        if ((uint16)(delta + 1023) < 2047)
        {
            item->helicopter.phase = 0;
            item->pos.y.bits = (uint32)(sint32)item->helicopter.hover_height << 16;
        }
    }
    else if (state == 4)
    {
        level_script_scene.object = (SCENE_OBJECT_REFERENCE){0};
        move_toward(item, 0, 28672);
        if (dir_distance < 3072)
        {
            item->kind = 55;
            voice = item->helicopter.voice_plus_one[1];
            if (voice)
                sound_voice_stop(voice - 1);
            voice = item->helicopter.voice_plus_one[0];
            if (voice)
                sound_voice_stop(voice - 1);
        }
    }
    result = item->pos.x.bits;
    if ((sint32)result < (sint32)0xeff00000)
    {
        item->pos.x.bits = 0xeff00000;
        return 1;
    }
    result = item->pos.x.bits;
    if ((sint32)result > 0)
        item->pos.x.bits = 0;
    return result;
}

/* Retained numeric spill bits are projectile values, never host pointers */
uint32 helicopter_update_retained(MODEL_OBJECT *item, uint32 prev_car, uint32 retained_angle)
{
    PROJECTILE_SOURCE projectile;
    if (!prev_car)
        return helicopter_update(item, NULL);
    /* Nested 4471C to 29AFC saves S0 and S2 into these two retained fields */
    projectile.kind = 499;
    projectile.height = prev_car;
    projectile.angle = (uint16)retained_angle;
    projectile.owner = -1;
    return helicopter_update(item, &projectile);
}

uint32 helicopter_update_scene(MODEL_OBJECT *model)
{
    FUNCTION_MARKER(0x8002a168u, "GAME.EXE");
    COLLISION_REGION *region = col_regions.data;
    uint32 i = 0, yaw, pitch, roll, angle, target, val, offset, result, volume, pan, height, velocity, wave, pos;
    sint32 count = col_regions.count;
    sint32 vertical;
    sint32 difference;
    uint32 speed;
    sint32 turn;
    sint32 x;
    sint32 z;
    sint32 target_x;
    sint32 target_z;
    sint32 voice;
    while ((sint32)i < count)
    {
        XASSERT(region != col_regions.data + COLLISION_REGION_COUNT);
        /* TODO: Investigate a corrupt native world reference before region matching */
        XASSERT(region->object.present <= 1 && (!region->object.present || region->object.idx < WORLD_OBJECT_COUNT));
        if (region->object.present && model == world_objects.object + region->object.idx)
            break;
        ++i;
        ++region;
    }
    /* TODO: Investigate a scene helicopter outside its native owner */
    XASSERT(model && model->payload_kind == MODEL_PAYLOAD_SCENE_HELICOPTER);
    yaw = model->angles.yaw_bits;
    pitch = model->angles.pitch_bits;
    roll = model->angles.roll_bits;
    if (game_session.stage == 15 && game_session.scene_phase == 2 && level_script_scene.event_timer == 1)
    {
        /* TODO: Investigate an absent or invalid relative-scene object before access */
        XASSERT(world_objects.relative_scene.present == 1 && world_objects.relative_scene.idx < WORLD_OBJECT_COUNT);
        regions_disable_object(world_objects.object + world_objects.relative_scene.idx);
        /* TODO: Investigate an absent or invalid relative-scene object before access */
        XASSERT(world_objects.relative_scene.present == 1 && world_objects.relative_scene.idx < WORLD_OBJECT_COUNT);
        world_objects.object[world_objects.relative_scene.idx].kind = 55;
    }
    if (model->scene_helicopter.voice_plus_one)
    {
        sound_spatial(&model->pos, &volume, &pan);
        volume = sound_volume_adjust(volume, 64);
        voice = model->scene_helicopter.voice_plus_one - 1;
        sound_voice_pan(voice, volume, pan);
    }
    else
        model->scene_helicopter.voice_plus_one = (uint16)(sound_voice_play(243, 0, 6144) + 1);
    if (!model->scene_helicopter.active && !level_script_scene.remaining_landings)
        return pitch << 16;
    vertical = (sint16)pitch >> 8;
    if (vertical == -1)
        vertical = 0;
    /* TODO: Investigate the original collision write without a matching region */
    XASSERT((sint32)i < count && i < COLLISION_REGION_COUNT);
    if ((sint32)model->pos.y.bits > (sint32)0xff400000u)
    {
        region->col_kind = 3;
        actors_collide_vehicle(region, model);
    }
    else
        region->col_kind = 0;
    x = model->pos.x.whole;
    z = model->pos.z.whole;
    target_x = level_script_scene.target_x;
    target_z = level_script_scene.target_z;
    angle = math_dir16(x, z, target_x, target_z);
    target = 0xfd000000;
    if (dir_distance < 2304 && !vertical)
        target = (uint16)(yaw - 0x3801) < 0xfff ? 0 : 0xfe800000;
    height = model->pos.y.bits;
    velocity = level_script_scene.height_velocity;
    val = velocity + ((sint32)(target - height - velocity) >> 4);
    level_script_scene.height_velocity = val;
    val = ((sint32)(val + 16) >> 5) + model->pos.y.bits;
    model->pos.y.bits = val;
    if ((sint32)val <= (sint32)0xfd800000u)
    {
        difference = (sint16)(angle - yaw);
        if (difference > 8192)
            difference = 8192;
        if (difference < -8192)
            difference = -8192;
        roll -= (uint32)(((sint16)roll + difference) >> 4);
        yaw -= (uint32)((sint16)roll >> 4);
        difference = dir_distance - 2048u;
        if (difference > 2048)
            difference = 2048;
        pitch += (uint32)((sint32)(difference - (uint32)(sint32)(sint16)pitch) >> 5);
        speed = sin_table[((pitch >> 1) & 0xffe) >> 1] >> 8;
        offset = (yaw >> 4) & 0xffe;
        wave = sin_table[offset >> 1];
        pos = model->pos.x.bits;
        model->pos.x.bits = pos + speed * wave;
        wave = sin_table[MATH_QUARTER_TURN + (offset >> 1)];
        pos = model->pos.z.bits;
        model->pos.z.bits = pos - speed * wave;
    }
    else
    {
        if (dir_distance < 2304 && (uint16)(yaw - 0x3801) < 0xfff)
        {
            if (model->pos.y.whole == -1)
            {
                camera.tracking.pitch_control_bits = 0;
                model->scene_helicopter.active = 0;
                if (game_session.stage != 18 || (sint32)model->pos.x.bits > (sint32)0xe7000000u || game_session.stage != 18)
                    level_wave.paused = 0;
                val = level_script_scene.remaining_landings;
                if ((sint32)val > 0)
                {
                    val--;
                    level_script_scene.remaining_landings = val;
                    if (!val && game_session.stage == 18)
                    {
                        level_script_scene.target_x = -7992;
                        level_script_scene.target_z = 1600;
                        level_script_scene.relative = 0;
                        model->scene_helicopter.active = 1;
                    }
                }
            }
        }
        else
        {
            roll -= (uint32)((sint16)roll >> 4);
            pitch -= (uint32)((sint16)pitch >> 4);
        }
        turn = level_script_scene.yaw_step;
        turn += (0x4000 - (sint16)yaw - turn) >> 4;
        level_script_scene.yaw_step = (uint16)turn;
        yaw += (uint32)(((sint16)turn + 16) >> 5);
    }
    level_script_scene.ox = model->pos.x.whole;
    result = model->pos.z.whole;
    level_script_scene.oz = result;
    model->angles.yaw_bits = (uint16)yaw;
    model->angles.pitch_bits = (uint16)pitch;
    model->angles.roll_bits = (uint16)roll;
    return result;
}

void helicopter_draw_scene(const MODEL_OBJECT *model)
{
    FUNCTION_MARKER(0x8002a640u, "GAME.EXE");
    sint32 amplitude, scale;
    uint32 offset;
    if (game_session.stage == 18)
    {
        /* TODO: Investigate a compound draw without a native position */
        XASSERT(model);
        if (model->pos.x.val < (sint32)0xe5300000u)
            return;
    }
    transform_push();
    transform_translate(0, 0x02000000, 0);
    /* TODO: Investigate compound draw fields outside both native object pools */
    XASSERT(model);
    transform_yaw(model->angles.yaw);
    transform_roll(-model->angles.roll);
    transform_pitch(model->angles.pitch);
    /* TODO: Investigate a compound draw without a native position */
    XASSERT(model);
    amplitude = (sint32)((uint32)(-(sint64)(model->pos.y.bits))) >> 18;
    if (amplitude > 64)
        amplitude = 64;
    offset = (game_session.tick << 4) & 0xff0u;
    transform_translate(0, (sin_table[offset >> 1]) * (uint32)amplitude, 0);
    model_draw_static(model_refs[341], 0, 0);
    transform_translate(0, 0x00100000, 0);
    transform_yaw((uint32)(uint16)game_session.tick << 12);
    model_draw_static(model_refs[564], 0, 0);
    transform_pop();
    if (game_session.stage == 18)
        return;
    transform_push();
    transform_yaw(model->angles.yaw);
    transform_translate(0, model->pos.y.val, -0x1400000);
    scale = (model->pos.y.val >> 14) + 16384;
    transform_scale(scale, scale, scale);
    model_draw_static(model_refs[834], 0, 0);
    transform_pop();
}

void helicopter_draw(const MODEL_OBJECT *model)
{
    FUNCTION_MARKER(0x8004277cu, "GAME.EXE");
    /* TODO: Investigate object drawing without a native position */
    XASSERT(model);
    if (model->pos.y.val <= (sint32)0xe0000000u)
        return;
    transform_push();
    transform_yaw(model->angles.yaw);
    transform_roll(model->angles.roll);
    transform_pitch(model->angles.pitch);
    model_draw_static(model_refs[687], 0, 0);
    transform_yaw((uint32)(uint16)game_session.tick << 12);
    model_draw_static(model_refs[564], 0, 0);
    transform_pop();
}

uint32 helicopter_update_intro(MODEL_OBJECT *model)
{
    FUNCTION_MARKER(0x8002c69cu, "GAME.EXE");
    uint32 volume, pan, tick, yaw, pitch, roll, delta, offset, val;
    sint32 angle, target, speed;
    /* TODO: Investigate an intro helicopter with a foreign native owner */
    XASSERT(model && model->payload_kind == MODEL_PAYLOAD_SCENE_HELICOPTER);
    if (model->scene_helicopter.voice_plus_one)
    {
        /* TODO: Investigate scripted motion without a native position */
        XASSERT(model);
        sound_spatial(&model->pos, &volume, &pan);
        volume = sound_volume_adjust(volume, 160);
        sound_voice_pan(model->scene_helicopter.voice_plus_one - 1, volume, pan);
        sound_voice_pitch(model->scene_helicopter.voice_plus_one - 1, 2 * model->scene_helicopter.speed + 6144);
    }
    else
        model->scene_helicopter.voice_plus_one = (uint16)(sound_voice_play(214, 0, 6144) + 1u);
    tick = game_session.tick;
    if (tick - 420u < 210u)
    {
        /* TODO: Investigate scripted common fields outside both native object pools */
        XASSERT(model);
        yaw = ((uint16)model->angles.yaw) << 5;
        angle = ((sint16)(16384u - yaw)) >> 5;
        target = model->scene_helicopter.yaw_step;
        target += (angle - target) >> 4;
        model->scene_helicopter.yaw_step = (uint16)target;
        yaw = ((yaw + target) << 16) >> 21;
        model->angles.yaw_bits = (uint16)yaw;
        if (yaw - 481u < 63u)
        {
            /* TODO: Investigate scripted motion without a native position */
            XASSERT(model);
            val = model->pos.x.bits;
            model->pos.x.bits = val + ((sint32)(0xfe800000u - val) >> 5);
            target = -((sint32)(0x13780000u - model->pos.z.bits) >> 13);
            angle = model->angles.roll;
            angle += (target - angle) >> 4;
            model->angles.roll_bits = (uint16)angle;
            model->pos.z.bits -= (uint32)((sint32)((uint32)angle << 16) >> 8);
            speed = model->angles.roll;
            if (speed < 0)
                speed = -speed;
            model->scene_helicopter.speed = (uint16)(speed >> 6);
        }
    }
    else
    {
        if ((sint32)tick < 2)
        {
            /* TODO: Investigate scripted motion without a native position */
            XASSERT(model);
            model->pos.x.bits = 0x30000000;
            model->pos.z.bits = 0x60000000;
            level_script_scene.target_x = -4096;
            level_script_scene.target_z = 12288;
            tick = game_session.tick;
        }
        if (tick == 120)
        {
            level_script_scene.target_x = -3072;
            level_script_scene.target_z = 12288;
            tick = game_session.tick;
        }
        if (tick == 240)
        {
            level_script_scene.target_x = -256;
            level_script_scene.target_z = 5120;
        }
        if (game_session.tick == 630)
        {
            level_script_scene.target_x = -24576;
            level_script_scene.target_z = 24576;
        }
        /* TODO: Investigate scripted common fields outside both native object pools */
        XASSERT(model);
        yaw = (uint16)model->angles.yaw;
        pitch = (uint16)model->angles.pitch;
        roll = (uint16)model->angles.roll;
        /* TODO: Investigate scripted motion without a native position */
        XASSERT(model);
        delta = math_dir16(model->pos.x.whole, model->pos.z.whole, level_script_scene.target_x, level_script_scene.target_z) - (yaw << 5);
        angle = (sint16)delta;
        if (angle > 8192)
            angle = 8192;
        if (angle < -8192)
            angle = -8192;
        if (game_session.tick - 361u < 239u)
            angle = 0;
        roll -= (uint32)(((sint16)roll + angle) >> 4);
        yaw = (yaw - ((sint16)roll >> 10)) & 2047;
        target = dir_distance - 1024u;
        if (target > 1024)
            target = 1024;
        if (game_session.tick - 270u >= 360u)
            target = (uint32)target << 1;
        pitch += (uint32)((sint32)(target - (uint32)(sint32)(sint16)pitch) >> 5);
        speed = sin_table[((pitch >> 4) & 0xffe) >> 1] >> 5;
        model->scene_helicopter.speed = (uint16)speed;
        offset = yaw * 2;
        model->pos.x.bits += (uint32)(speed * sin_table[offset >> 1]);
        model->pos.z.bits -= (uint32)(speed * sin_table[MATH_QUARTER_TURN + (offset >> 1)]);
        model->angles.yaw_bits = (uint16)yaw;
        model->angles.pitch_bits = (uint16)pitch;
        model->angles.roll_bits = (uint16)roll;
    }
    tick = game_session.tick;
    /* TODO: Investigate scripted motion without a native position */
    XASSERT(model);
    model->pos.y.bits = ((uint32)(sint32)sin_table[((tick << 4) & 0xff0) >> 1] << 4) + 0xfe900000u;
    if (tick == 1050)
    {
        model->kind = 55;
        return sound_voice_stop(model->scene_helicopter.voice_plus_one - 1);
    }
    return 55;
}

void helicopter_draw_intro(const MODEL_OBJECT *item)
{
    FUNCTION_MARKER(0x8002cac4u, "GAME.EXE");
    transform_push();
    /* TODO: Investigate common draw fields outside both native object pools */
    XASSERT(item);
    transform_yaw((uint32)(sint32)item->angles.yaw << 5);
    transform_roll(-item->angles.roll);
    transform_pitch(item->angles.pitch);
    model_draw_static(model_refs[146], 0, 0);
    transform_push();
    transform_pitch((uint32)(sint32)item->angles.pitch << 3);
    model_draw_static(model_refs[147], 0, 0);
    transform_pop();
    if ((sint32)game_session.tick >= 631)
    {
        transform_translate(0, -0x1660000, 0x1000000);
        transform_yaw(16384);
        model_draw_static(model_refs[148], 0, 0);
    }
    transform_pop();
}

uint32 helicopter_update_patrol(MODEL_OBJECT *model)
{
    FUNCTION_MARKER(0x800403dcu, "GAME.EXE");
    uint32 volume, pan, offset, result;
    sint32 mode, phase, angle, speed;
    /* TODO: Investigate a patrol helicopter with a foreign native owner */
    XASSERT(model && model->payload_kind == MODEL_PAYLOAD_PATROL_HELICOPTER);
    if (model->patrol.voice_plus_one)
    {
        /* TODO: Investigate an object update without a native position */
        XASSERT(model);
        sound_spatial(&model->pos, &volume, &pan);
        sound_voice_pan(model->patrol.voice_plus_one - 1, volume, pan);
    }
    else
        model->patrol.voice_plus_one = (uint16)(sound_voice_play(243, 255, 6144) + 1u);
    mode = model->patrol.mode;
    if (mode == 1)
    {
        /* TODO: Investigate physics common fields outside both native object pools */
        XASSERT(model);
        angle = (model->angles.yaw + 24576) >> 5;
        /* TODO: Investigate an object update without a native position */
        XASSERT(model);
        model->pos.y.bits = 0xfa240000;
        speed = sin_table[angle & 2047u];
        model->pos.x.bits = actor_positions[0].x.bits + (25000u * sin_table[(angle + 512) & 2047u]);
        model->angles.roll_bits = 63488;
        model->angles.yaw_bits = (uint16)((uint16)model->angles.yaw + 64u);
        model->pos.z.bits = actor_positions[0].z.bits + (25000 * speed);
    }
    else if (mode == 2)
    {
        /* TODO: Investigate an object update without a native position */
        XASSERT(model);
        uint32 x = model->pos.x.bits;
        model->pos.y.bits = 0xfa240000;
        model->pos.x.bits = x + 0x00780000u;
        model->angles.pitch_bits = 1024;
        model->angles.yaw_bits = 16384;
        model->pos.z.bits = actor_positions[0].z.bits + 0x0fa00000u;
    }
    else
    {
        phase = model->patrol.phase;
        /* TODO: Investigate an object update without a native position */
        XASSERT(model);
        model->pos.y.bits = 0xf4480000;
        model->patrol.speed = 200;
        if (phase < 2)
        {
            if (model->angles.roll < 0)
                model->angles.roll_bits = (uint16)(model->angles.roll + 1024);
            if (!model->patrol.phase)
            {
                if (model->pos.z.val < (sint32)0xf6a00000u)
                    model->patrol.phase = 2;
            }
            else if (model->patrol.phase == 1 && model->pos.z.val > 0x3e800000)
                model->patrol.phase = 2;
        }
        else if (phase == 2)
        {
            angle = model->angles.roll;
            model->angles.yaw_bits = (uint16)((uint16)model->angles.yaw + 256u);
            if (angle >= -4095)
                model->angles.roll_bits = (uint16)(angle - 320);
            offset = (uint16)model->angles.yaw >> 5;
            if (!offset)
                model->patrol.phase = 0;
            if (offset == 1024)
                model->patrol.phase = 1;
        }
        angle = model->angles.yaw >> 5;
        speed = model->patrol.speed;
        uint32 x = model->pos.x.bits;
        uint32 z = model->pos.z.bits;
        model->pos.x.bits = x + (speed * sin_table[angle & 2047u]);
        model->pos.z.bits = z - (speed * sin_table[(angle + 512) & 2047u]);
    }
    result = ((uint16)model->patrol.rotor_angle) + 256u;
    model->patrol.rotor_angle = (uint16)result;
    return result;
}

void helicopter_draw_patrol(const MODEL_OBJECT *item)
{
    FUNCTION_MARKER(0x80044cd4u, "GAME.EXE");
    sint32 angle;
    transform_push();
    /* TODO: Investigate common draw fields outside both native object pools */
    XASSERT(item);
    transform_yaw(item->angles.yaw);
    angle = item->angles.roll;
    if (angle)
        transform_roll(angle);
    angle = item->angles.pitch;
    if (angle)
        transform_pitch(angle);
    model_draw_static(model_refs[item->kind], 0, 0);
    transform_yaw((uint32)(uint16)game_session.tick << 12);
    model_draw_static(model_refs[564], 0, 0);
    transform_pop();
}
