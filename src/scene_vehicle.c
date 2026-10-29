#include "game.h"

GDB_CALL intptr scene_vehicle_update_taxi(MODEL_OBJECT *model)
{
    FUNCTION_MARKER(0x80043978u, "GAME.EXE");
    /* TODO: Investigate a scene vehicle with a foreign native owner */
    XASSERT(model && model->payload_kind == MODEL_PAYLOAD_SCENE_VEHICLE);
    uint32 start = model->scene_vehicle.start_tick;
    sint32 age = game_session.tick - start, pitch, count, voice, wave;
    COLLISION_REGION *region;
    uint32 volume, pan, angle, multiplier, target_idx, scan_count, random, x, y, z;
    intptr result = 0;
    FIXED_VEC3 point;
    int i;
    if (model->scene_vehicle.voice_plus_one[1] > 0)
    {
        /* TODO: Investigate a scene callback without a native position */
        XASSERT(model);
        sound_spatial(&model->pos, &volume, &pan);
        if (age > 0)
        {
            sound_voice_pitch(model->scene_vehicle.voice_plus_one[1] - 1, ((uint32)(age >> 3) << 3) + 6400);
            if (age < 65)
                volume = (sint32)(age * volume) >> 6;
        }
        volume = sound_volume_adjust(volume, -80);
        sound_voice_pan(model->scene_vehicle.voice_plus_one[1] - 1, volume, pan);
    }
    if (model->scene_vehicle.start_tick == -1)
    {
        if (!level_wave.paused || game_session.scene_phase != 1 || players_before_z(0xec000000u))
            goto region_update;
        model->scene_vehicle.start_tick = (uint16)game_session.tick;
        model->scene_vehicle.voice_plus_one[1] = (uint16)(sound_voice_play(132, 0, 6400) + 1);
        game_session.scene_phase = game_session.scene_phase + 1;
        /* Age was captured before the start timestamp changed. */
    }
    if (age < 10)
        return 1;
    if (age < 90)
    {
        /* TODO: Investigate transport common fields outside both native object pools */
        XASSERT(model);
        angle = model->angles.yaw_bits;
        if (age < 40)
        {
            angle = (angle >> 5) * 2;
            multiplier = 128;
        }
        else
        {
            angle += age < 70 ? 192u : (age < 80 ? 0xfffffa00u : 0xffffff80u);
            model->angles.yaw_bits = (uint16)angle;
            angle = (angle >> 4) & 4094;
            multiplier = age < 70 ? 160 : (age < 80 ? 192 : 256);
        }
        wave = sin_table[angle >> 1];
        /* TODO: Investigate a scene callback without a native position */
        XASSERT(model);
        x = model->pos.x.bits;
        model->pos.x.bits = x - wave * multiplier;
        wave = sin_table[MATH_QUARTER_TURN + (angle >> 1)];
        z = model->pos.z.bits;
        model->pos.z.bits = z + wave * multiplier;
        if (age == 69)
            model->scene_vehicle.voice_plus_one[0] = (uint16)(sound_voice_play(131, 255, 5632) + 1);
        if (age >= 70 && (age & 1))
        {
            transform_push();
            transform_identity();
            transform_pitch(32768);
            transform_yaw(model->angles.yaw);
            transform_point(128, 0, 256, point.bits);
            x = model->pos.x.bits;
            y = model->pos.y.bits;
            z = model->pos.z.bits;
            effect_spawn_timed(871, (point.x.bits << 16) + x, y + 196608, (point.z.bits << 16) + z, 0, 0, 0);
            transform_pop();
            transform_push();
            transform_identity();
            transform_pitch(32768);
            transform_yaw(model->angles.yaw);
            transform_point(-128, 0, 256, point.bits);
            x = model->pos.x.bits;
            y = model->pos.y.bits;
            z = model->pos.z.bits;
            effect_spawn_timed(871, (point.x.bits << 16) + x, y + 196608, (point.z.bits << 16) + z, 0, 0, 0);
            transform_pop();
        }
    }
    else if (age < 98)
    {
        if (age == 90)
        {
            voice = model->scene_vehicle.voice_plus_one[0];
            if (voice)
            {
                sound_voice_stop(voice - 1);
                model->scene_vehicle.voice_plus_one[0] = 0;
            }
            voice = model->scene_vehicle.voice_plus_one[1];
            if (voice)
            {
                sound_voice_stop(voice - 1);
                model->scene_vehicle.voice_plus_one[1] = 0;
            }
            target_idx = 0;
            scan_count = world_objects.count;
            while (target_idx < scan_count)
            {
                /* TODO: Investigate an impact search exceeding the world object pool */
                XASSERT(target_idx < WORLD_OBJECT_COUNT);
                if (world_objects.object[target_idx].kind == 162)
                    break;
                target_idx++;
            }
            if (target_idx < scan_count)
            {
                sound_voice_play(134, 255, 6144);
                disp_buffers.shake_ticks = 16;
                disp_buffers.flash_ticks = 4;
                MODEL_OBJECT *target_model = world_objects.object + target_idx;
                random = random_next() & 0x17f;
                y = target_model->pos.y.bits;
                x = target_model->pos.x.bits;
                z = target_model->pos.z.bits;
                effect_spawn_timed(871, x + 0xffc00000u, y - (random << 16), z + 0xfe810000, 0, 0, 0);
                random = random_next() & 0x17f;
                y = target_model->pos.y.bits;
                x = target_model->pos.x.bits;
                z = target_model->pos.z.bits;
                effect_spawn_timed(871, x + 0xffd00000u, y - (random << 16), z + 0xfe810000, 0, 0, 0);
                random = random_next() & 0x17f;
                y = target_model->pos.y.bits;
                x = target_model->pos.x.bits;
                z = target_model->pos.z.bits;
                effect_spawn_timed(871, x + 0xffe00000u, y - (random << 16), z + 0xfe810000, 0, 0, 0);
                random = random_next() & 0x17f;
                y = target_model->pos.y.bits;
                x = target_model->pos.x.bits;
                z = target_model->pos.z.bits;
                effect_spawn_timed(871, x + 0xfff00000u, y - (random << 16), z + 0xfe810000, 0, 0, 0);
                random = random_next() & 0x17f;
                y = target_model->pos.y.bits;
                z = target_model->pos.z.bits;
                /* The center particle reads X after Z */
                x = target_model->pos.x.bits;
                effect_spawn_timed(871, x, y - (random << 16), z + 0xfe810000, 0, 0, 0);
                random = random_next() & 0x17f;
                y = target_model->pos.y.bits;
                x = target_model->pos.x.bits;
                z = target_model->pos.z.bits;
                effect_spawn_timed(871, x + 0x00100000u, y - (random << 16), z + 0xfe810000, 0, 0, 0);
                random = random_next() & 0x17f;
                y = target_model->pos.y.bits;
                x = target_model->pos.x.bits;
                z = target_model->pos.z.bits;
                effect_spawn_timed(871, x + 0x00200000u, y - (random << 16), z + 0xfe810000, 0, 0, 0);
                random = random_next() & 0x17f;
                y = target_model->pos.y.bits;
                x = target_model->pos.x.bits;
                z = target_model->pos.z.bits;
                effect_spawn_timed(871, x + 0x00300000u, y - (random << 16), z + 0xfe810000, 0, 0, 0);
                random = random_next() & 0x17f;
                y = target_model->pos.y.bits;
                x = target_model->pos.x.bits;
                z = target_model->pos.z.bits;
                effect_spawn_timed(871, x + 0x00400000u, y - (random << 16), z + 0xfe810000, 0, 0, 0);
                target_model->kind = 163;
            }
        }
        pitch = -((sint32)((uint32)sin_table[((uint32)(age - 90) << 8) >> 1] << 16) >> 20);
        /* TODO: Investigate transport common fields outside both native object pools */
        XASSERT(model);
        model->angles.pitch_bits = (uint16)pitch;
        /* TODO: Investigate a scene callback without a native position */
        XASSERT(model);
        model->pos.y.bits = (uint32)(pitch >> 6) << 15;
    }
    else if (age < 107)
    {
        pitch = -((sint32)((uint32)sin_table[((uint32)(age - 98) << 8) >> 1] << 16) >> 21);
        /* TODO: Investigate transport common fields outside both native object pools */
        XASSERT(model);
        model->angles.pitch_bits = (uint16)pitch;
        /* TODO: Investigate a scene callback without a native position */
        XASSERT(model);
        model->pos.y.bits = (uint32)(pitch >> 6) << 14;
    }
    else
    {
        random = random_next() & 31;
        /* TODO: Investigate a scene callback without a native position */
        XASSERT(model);
        x = (random << 16) + model->pos.x.bits;
        random = random_next();
        z = model->pos.z.bits;
        y = model->pos.y.bits;
        result = (intptr)effect_spawn_timed(871, x, y - 6291456, ((random & 31) << 16) + z, 0, 0, 0);
        random = random_next() & 31;
        x = (random << 16) + model->pos.x.bits;
        random = random_next();
        z = model->pos.z.bits;
        y = model->pos.y.bits;
        result = (intptr)effect_spawn_timed(871, x, y - 6291456, ((random & 31) << 16) + z, 0, 0, 0);
        return result;
    }
region_update:
    region = col_regions.data;
    count = col_regions.count;
    if (count > 0)
        count = col_regions.count;
    for (i = 0; i < count; ++i, ++region)
    {
        XASSERT(region != col_regions.data + COLLISION_REGION_COUNT);
        /* TODO: Investigate a corrupt native world reference before region matching */
        XASSERT(region->object.present <= 1 && (!region->object.present || region->object.idx < WORLD_OBJECT_COUNT));
        if (region->object.present && model == world_objects.object + region->object.idx && region->shape == 1)
            break;
    }
    /* TODO: Investigate the original collision call without a matching region */
    XASSERT(i < count);
    return actors_collide_vehicle(region, model);
}

GDB_CALL uint32 scene_vehicle_update_truck(MODEL_OBJECT *model)
{
    FUNCTION_MARKER(0x80044334u, "GAME.EXE");
    /* TODO: Investigate a scene vehicle with a foreign native owner */
    XASSERT(model && model->payload_kind == MODEL_PAYLOAD_SCENE_VEHICLE);
    sint32 start = model->scene_vehicle.start_tick;
    sint32 elapsed = game_session.tick - start;
    sint32 count;
    sint32 i;
    sint32 voice;
    uint32 wave;
    COLLISION_REGION *region;
    uint32 angle, offset, speed, x, y, z;
    FIXED_VEC3 point;
    if (start == -1)
    {
        if (game_session.scene_phase != 2)
            goto region_update;
        model->scene_vehicle.start_tick = (uint16)game_session.tick;
    }
    /* MIPS keeps elapsed computed with the old -1 start on the arming frame. */
    if (elapsed < 15)
        return elapsed < 50;
    if (elapsed < 50)
    {
        {
            SCENE_OBJECT_REFERENCE ref = {0};
            uint32 slot;
            for (slot = 0; slot < WORLD_OBJECT_COUNT; slot++)
                if (model == world_objects.object + slot)
                {
                    ref = (SCENE_OBJECT_REFERENCE){slot, SCENE_OBJECT_WORLD};
                    break;
                }
            if (!ref.pool)
                for (slot = 0; slot < DYNAMIC_OBJECT_COUNT; slot++)
                    if (model == dynamic_objects + slot)
                    {
                        ref = (SCENE_OBJECT_REFERENCE){slot, SCENE_OBJECT_DYNAMIC};
                        break;
                    }
            /* TODO: Investigate scene publications outside both native object pools */
            XASSERT(ref.pool);
            level_script_scene.object = ref;
        }
        /* TODO: Investigate transport common fields outside both native object pools */
        XASSERT(model);
        offset = 2 * (model->angles.yaw_bits >> 5);
        speed = 88;
    }
    else if (elapsed < 60)
    {
        if (elapsed == 50)
            sound_voice_play(233, 120, 7168);
        /* TODO: Investigate transport common fields outside both native object pools */
        XASSERT(model);
        angle = model->angles.yaw_bits - 512;
        model->angles.yaw_bits = (uint16)angle;
        offset = (angle >> 4) & 0xffe;
        speed = 160;
    }
    else if (elapsed < 140)
    {
        if (elapsed == 65)
            sound_voice_play(233, 162, 7168);
        /* TODO: Investigate transport common fields outside both native object pools */
        XASSERT(model);
        angle = model->angles.yaw_bits - 256;
        model->angles.yaw_bits = (uint16)angle;
        offset = (angle >> 4) & 0xffe;
        speed = 160u - 2u * (elapsed - 60u);
    }
    else
    {
        if (elapsed != 140)
            return 140;
        voice = model->scene_vehicle.voice_plus_one[0];
        if (voice)
            sound_voice_stop(voice - 1);
        level_wave.paused = 0;
        goto region_update;
    }
    wave = sin_table[offset >> 1];
    /* TODO: Investigate a transport callback without a native position */
    XASSERT(model);
    x = model->pos.x.bits;
    model->pos.x.bits = x - speed * wave;
    wave = sin_table[MATH_QUARTER_TURN + (offset >> 1)];
    z = model->pos.z.bits;
    model->pos.z.bits = z + speed * wave;
    if (elapsed >= 60)
    {
        if (elapsed == 85)
            sound_voice_play(230, 95, 5632);
        if (elapsed >= 86 && (elapsed & 1))
        {
            transform_push();
            transform_identity();
            transform_pitch(32768);
            transform_yaw(model->angles.yaw);
            transform_point(128, 0, -256, point.bits);
            x = model->pos.x.bits;
            y = model->pos.y.bits;
            z = model->pos.z.bits;
            effect_spawn_timed(871, (point.x.bits << 16) + x, y + 196608, (point.z.bits << 16) + z, 0, 0, 0);
            transform_pop();
            transform_push();
            transform_identity();
            transform_pitch(32768);
            transform_yaw(model->angles.yaw);
            transform_point(-128, 0, -256, point.bits);
            x = model->pos.x.bits;
            y = model->pos.y.bits;
            z = model->pos.z.bits;
            effect_spawn_timed(871, (point.x.bits << 16) + x, y + 196608, (point.z.bits << 16) + z, 0, 0, 0);
            transform_pop();
        }
    }
region_update:
    region = col_regions.data;
    count = col_regions.count;
    if (count <= 0)
        return count;
    for (i = 0; i < count; ++i, ++region)
    {
        XASSERT(region != col_regions.data + COLLISION_REGION_COUNT);
        /* TODO: Investigate a corrupt native world reference before region matching */
        XASSERT(region->object.present <= 1 && (!region->object.present || region->object.idx < WORLD_OBJECT_COUNT));
        if (region->object.present && model == world_objects.object + region->object.idx)
            break;
    }
    if (i < (sint32)col_regions.count)
        return actors_collide_vehicle(region, model);
    return 0;
}

GDB_CALL sint32 scene_vehicle_update_accelerating_truck(MODEL_OBJECT *model)
{
    FUNCTION_MARKER(0x80043128u, "GAME.EXE");
    /* TODO: Investigate emitter operands outside both native object pools */
    XASSERT(model);
    /* TODO: Investigate scripted motion with a foreign native owner */
    XASSERT(model && model->payload_kind == MODEL_PAYLOAD_SCENE_VEHICLE);
    uint32 volume, pan;
    if (!model->scene_vehicle.motion_active)
    {
        if (players_before_z(0x06000000))
            return 1;
        model->scene_vehicle.motion_active = 1;
        model->scene_vehicle.speed = 0;
        stage_audio_slots[4].voice = sound_voice_play(146, 0, 4608);
    }
    model->scene_vehicle.speed = (uint16)(model->scene_vehicle.speed + 1);
    if (model->scene_vehicle.speed >= 49)
        model->scene_vehicle.speed = 48;
    model->pos.x.bits = model->pos.x.bits + ((uint32)(sint32)model->scene_vehicle.speed << 16);
    sound_spatial(&model->pos, &volume, &pan);
    volume = sound_volume_adjust(volume, 24);
    sound_voice_pan(stage_audio_slots[4].voice, volume, pan);
    if ((sint32)model->pos.x.bits > 0x15000000)
    {
        model->kind = 55;
        return sound_voice_stop(stage_audio_slots[4].voice);
    }
    return 55;
}

static const uint32 transport_wrap_kinds[8] = {175, 176, 177, 178, 179, 180, 181, 182};

GDB_CALL uint32 scene_vehicle_update_traffic(MODEL_OBJECT *model)
{
    FUNCTION_MARKER(0x8004471cu, "GAME.EXE");
    COLLISION_REGION *region = col_regions.data;
    uint32 angle, pos, i = 0;
    sint32 count;
    uint32 wave;
    /* TODO: Investigate a scene-traffic callback with a foreign native owner */
    XASSERT(model && model->payload_kind == MODEL_PAYLOAD_SCENE_VEHICLE);
    if (model->pos.x.bits + 0x18000000u > 0x30000000u)
    {
        model->angles.yaw_bits = model->angles.yaw_bits ^ 0x8000;
        model->kind = transport_wrap_kinds[random_next() & 7u];
    }
    angle = 2 * (model->angles.yaw_bits >> 5);
    wave = sin_table[angle >> 1];
    pos = model->pos.x.bits;
    model->pos.x.bits = pos - 192 * wave;
    wave = sin_table[MATH_QUARTER_TURN + (angle >> 1)];
    pos = model->pos.z.bits;
    model->pos.z.bits = pos + 192 * wave;
    count = col_regions.count;
    while ((sint32)i < count)
    {
        XASSERT(region != col_regions.data + COLLISION_REGION_COUNT);
        /* TODO: Investigate a corrupt native world reference before region matching */
        XASSERT(region->object.present <= 1 && (!region->object.present || region->object.idx < WORLD_OBJECT_COUNT));
        if (region->object.present && model == world_objects.object + region->object.idx)
            break;
        i++;
        ++region;
    }
    /* TODO: Investigate the original traffic collision call without a matching region */
    XASSERT((sint32)i < count && i < COLLISION_REGION_COUNT);
    return actors_collide_vehicle(region, model);
}

/* Original Q16 exhaust placement data belongs to this two-phase scene car */
enum
{
    SCENE_CAR_IDLE_EXHAUST_WIDTH = 96 << 16,
    SCENE_CAR_EXHAUST_Z = 352 << 16,
    SCENE_CAR_MOVING_EXHAUST_WIDTH = 80 << 16
};

GDB_CALL uint32 scene_vehicle_start_departure(MODEL_OBJECT *model)
{
    FUNCTION_MARKER(0x80042ba0u, "GAME.EXE");
    COLLISION_REGION *region = col_regions.data;
    FIXED_VEC3 pos;
    uint32 volume, pan, quiet, step, z;
    sint32 voice;
    sint32 tick, count = col_regions.count, i;
    for (i = 0; i < count; ++i, ++region)
    {
        XASSERT(region != col_regions.data + COLLISION_REGION_COUNT);
        /* TODO: Investigate a corrupt native world reference before region matching */
        XASSERT(region->object.present <= 1 && (!region->object.present || region->object.idx < WORLD_OBJECT_COUNT));
        if (region->object.present && model == world_objects.object + region->object.idx)
            break;
    }
    tick = game_session.tick;
    if (tick == 1)
    {
        stage_audio_slots[0].voice = sound_voice_play(147, 0, 6144);
        stage_audio_slots[2].voice = sound_voice_play(132, 0, 6400);
        /* TODO: Investigate emitter operands outside both native object pools */
        XASSERT(model);
        pos.x.bits = model->pos.x.bits;
        pos.y.bits = model->pos.y.bits;
        pos.z.bits = 0x01000000;
        sound_spatial(&pos, &volume, &pan);
        volume = sound_volume_adjust(volume, -80);
        quiet = sound_volume_adjust(volume, -128);
        voice = stage_audio_slots[0].voice;
        sound_voice_pan(voice, volume, pan);
        voice = stage_audio_slots[2].voice;
        sound_voice_pan(voice, quiet, pan);
    }
    else if (tick >= 100)
    {
        if (tick < 130)
        {
            if (tick & 1)
            {
                uint32 random, x, y, z;
                random = random_next();
                /* TODO: Investigate exhaust emission without a native scene-car position */
                XASSERT(model);
                x = model->pos.x.bits - SCENE_CAR_IDLE_EXHAUST_WIDTH - ((random & 63u) << 16);
                z = model->pos.z.bits + SCENE_CAR_EXHAUST_Z;
                y = model->pos.y.bits;
                effect_spawn_timed(871, x, y, z, 0, 0, 0);
                random = random_next();
                x = model->pos.x.bits + SCENE_CAR_IDLE_EXHAUST_WIDTH + ((random & 63u) << 16);
                z = model->pos.z.bits + SCENE_CAR_EXHAUST_Z;
                y = model->pos.y.bits;
                effect_spawn_timed(871, x, y, z, 0, 0, 0);
            }
            if (game_session.tick == 110)
                stage_audio_slots[1].voice = sound_voice_play(146, 0, 4608);
            if ((sint32)game_session.tick >= 111)
            {
                /* TODO: Investigate emitter operands outside both native object pools */
                XASSERT(model);
                sound_spatial(&model->pos, &volume, &pan);
                sound_voice_pan(stage_audio_slots[1].voice, volume, pan);
            }
        }
        else if ((sint32)game_session.tick < 200)
        {
            /* TODO: Investigate emitter operands outside both native object pools */
            XASSERT(model);
            sound_spatial(&model->pos, &volume, &pan);
            sound_voice_pan(stage_audio_slots[1].voice, volume, pan);
            if (game_session.tick & 2)
            {
                uint32 random, x, y, z;
                random = random_next();
                /* TODO: Investigate exhaust emission without a native scene-car position */
                XASSERT(model);
                x = model->pos.x.bits - SCENE_CAR_MOVING_EXHAUST_WIDTH - ((random & 63u) << 16);
                z = model->pos.z.bits + SCENE_CAR_EXHAUST_Z;
                y = model->pos.y.bits;
                effect_spawn_timed(871, x, y, z, 0, 0, 0);
                random = random_next();
                x = model->pos.x.bits + SCENE_CAR_MOVING_EXHAUST_WIDTH + ((random & 63u) << 16);
                z = model->pos.z.bits + SCENE_CAR_EXHAUST_Z;
                y = model->pos.y.bits;
                effect_spawn_timed(871, x, y, z, 0, 0, 0);
            }
            /* TODO: Investigate a departure step with a foreign native owner */
            XASSERT(model->payload_kind == MODEL_PAYLOAD_SCENE_VEHICLE);
            step = (model->scene_vehicle.departure_step) * 196608u;
            z = model->pos.z.bits;
            model->pos.z.bits = z - step;
            model->scene_vehicle.departure_step = (uint16)((uint16)model->scene_vehicle.departure_step + 1);
        }
        else
        {
            sound_voice_stop(stage_audio_slots[1].voice);
            /* TODO: Investigate common fields outside both native object pools */
            XASSERT(model);
            model->kind = 749;
            model->pos.z.bits = 0x01000000;
            /* TODO: Investigate the original scene-car collision access without a match */
            XASSERT((sint32)i < (sint32)count && (uint32)i < COLLISION_REGION_COUNT);
            col_region_configure(region, model->kind);
            col_regions.count = col_regions.count - 1;
        }
    }
    /* TODO: Investigate the original scene-car collision access without a match */
    XASSERT((sint32)i < (sint32)count && (uint32)i < COLLISION_REGION_COUNT);
    return actors_collide_vehicle(region, model);
}

GDB_CALL sint32 scene_vehicle_update_departure(MODEL_OBJECT *model)
{
    FUNCTION_MARKER(0x80042f80u, "GAME.EXE");
    COLLISION_REGION *region;
    uint32 count, i, z, volume, pan;
    region = col_regions.data;
    count = col_regions.count;
    for (i = 0; (sint32)i < (sint32)count; i++, ++region)
    {
        XASSERT(region != col_regions.data + COLLISION_REGION_COUNT);
        /* TODO: Investigate a corrupt native world reference before region matching */
        XASSERT(region->object.present <= 1 && (!region->object.present || region->object.idx < WORLD_OBJECT_COUNT));
        if (region->object.present && model == world_objects.object + region->object.idx)
            break;
    }
    /* TODO: Investigate the original scene-car collision access without a match */
    XASSERT((sint32)i < (sint32)count && (uint32)i < COLLISION_REGION_COUNT);
    actors_collide_vehicle(region, model);
    /* TODO: Investigate emitter operands outside both native object pools */
    XASSERT(model);
    z = model->pos.z.bits - 0x800000u;
    model->pos.z.bits = z;
    if (z == 0xc7000000u)
    {
        sound_voice_pitch(stage_audio_slots[0].voice, 6128);
        sound_voice_pitch(stage_audio_slots[2].voice, 6336);
    }
    else if (model->pos.z.bits == 0xdf000000u)
    {
        sound_voice_pitch(stage_audio_slots[0].voice, 6112);
        sound_voice_pitch(stage_audio_slots[2].voice, 6272);
    }
    sound_spatial(&model->pos, &volume, &pan);
    volume = sound_volume_adjust(volume, -80);
    sound_voice_pan(stage_audio_slots[0].voice, volume, pan);
    volume = sound_volume_adjust(volume, -48);
    sound_voice_pan(stage_audio_slots[2].voice, volume, pan);
    region = col_regions.data;
    count = col_regions.count;
    for (i = 0; (sint32)i < (sint32)count; i++, ++region)
    {
        XASSERT(region != col_regions.data + COLLISION_REGION_COUNT);
        /* TODO: Investigate a corrupt native world reference before region matching */
        XASSERT(region->object.present <= 1 && (!region->object.present || region->object.idx < WORLD_OBJECT_COUNT));
        if (region->object.present && model == world_objects.object + region->object.idx)
            break;
    }
    /* TODO: Investigate the original scene-car collision access without a match */
    XASSERT((sint32)i < (sint32)count && (uint32)i < COLLISION_REGION_COUNT);
    actors_collide_vehicle(region, model);
    if ((sint32)model->pos.z.bits < (sint32)0xbb000000u)
    {
        model->kind = 55;
        sound_voice_stop(stage_audio_slots[0].voice);
        return sound_voice_stop(stage_audio_slots[2].voice);
    }
    return 55;
}

GDB_CALL uint32 scene_vehicle_update_car(MODEL_OBJECT *model)
{
    FUNCTION_MARKER(0x80044844u, "GAME.EXE");
    /* TODO: Investigate a scene vehicle with a foreign native owner */
    XASSERT(model && model->payload_kind == MODEL_PAYLOAD_SCENE_VEHICLE);
    uint32 start = model->scene_vehicle.start_tick;
    sint32 elapsed = game_session.tick - start, count, i, bounce, voice, wave;
    COLLISION_REGION *region;
    uint32 volume, pan, angle, offset, speed, world_idx, scan_count, pos;
    if (model->scene_vehicle.voice_plus_one[1])
    {
        /* TODO: Investigate a vehicle exit scene without a native position */
        XASSERT(model);
        sound_spatial(&model->pos, &volume, &pan);
        volume = sound_volume_adjust(volume, elapsed - 130u);
        sound_voice_pan(model->scene_vehicle.voice_plus_one[1] - 1, volume, pan);
        if (elapsed >= 96)
            sound_voice_pitch(model->scene_vehicle.voice_plus_one[1] - 1, 6912u - 8u * (elapsed - 95u));
    }
    if (model->scene_vehicle.start_tick == -1)
    {
        if (!level_wave.paused || game_session.scene_phase != 4 || players_before_z(-0xc000000))
            goto region_update;
        model->scene_vehicle.voice_plus_one[1] = (uint16)(sound_voice_play(132, 0, 6912) + 1);
        model->scene_vehicle.start_tick = (uint16)game_session.tick;
        {
            SCENE_OBJECT_REFERENCE ref = {0};
            uint32 slot;
            for (slot = 0; slot < WORLD_OBJECT_COUNT; slot++)
                if (model == world_objects.object + slot)
                {
                    ref = (SCENE_OBJECT_REFERENCE){slot, SCENE_OBJECT_WORLD};
                    break;
                }
            if (!ref.pool)
                for (slot = 0; slot < DYNAMIC_OBJECT_COUNT; slot++)
                    if (model == dynamic_objects + slot)
                    {
                        ref = (SCENE_OBJECT_REFERENCE){slot, SCENE_OBJECT_DYNAMIC};
                        break;
                    }
            /* TODO: Investigate scene publications outside both native object pools */
            XASSERT(ref.pool);
            level_script_scene.object = ref;
        }
    }
    if (elapsed < 0)
        goto region_update;
    if (elapsed < 16)
    {
        /* TODO: Investigate transport common fields outside both native object pools */
        XASSERT(model);
        offset = 2 * (model->angles.yaw_bits >> 5);
        speed = 96;
    }
    else if (elapsed < 59)
    {
        /* TODO: Investigate transport common fields outside both native object pools */
        XASSERT(model);
        angle = model->angles.yaw_bits - 256;
        model->angles.yaw_bits = (uint16)angle;
        offset = (angle >> 4) & 0xffe;
        speed = 129;
    }
    else if (elapsed < 112)
    {
        wave = model->scene_vehicle.speed;
        if (wave < 192)
            model->scene_vehicle.speed = (uint16)(wave + 8);
        /* TODO: Investigate transport common fields outside both native object pools */
        XASSERT(model);
        offset = 2 * (model->angles.yaw_bits >> 5);
        speed = model->scene_vehicle.speed;
    }
    else
    {
        if (elapsed == 112)
        {
            scan_count = world_objects.count;
            for (world_idx = 0; world_idx < scan_count; world_idx++)
            {
                /* TODO: Investigate a scene scan exceeding the world object pool */
                XASSERT(world_idx < WORLD_OBJECT_COUNT);
                MODEL_OBJECT *world = world_objects.object + world_idx;
                if (world->kind == 173)
                    world->kind = 174;
            }
            voice = model->scene_vehicle.voice_plus_one[0];
            if (voice)
            {
                sound_voice_stop(voice - 1);
                model->scene_vehicle.voice_plus_one[0] = 0;
            }
            voice = model->scene_vehicle.voice_plus_one[1];
            if (voice)
            {
                sound_voice_stop(voice - 1);
                model->scene_vehicle.voice_plus_one[1] = 0;
            }
            sound_voice_play(135, 255, 6144);
            sound_voice_play(154, 160, 6144);
            disp_buffers.shake_ticks = 16;
            disp_buffers.flash_ticks = 4;
        }
        else if (elapsed < 120)
        {
            bounce = -((sint32)((uint32)sin_table[((elapsed - 112u) << 8) >> 1] << 16) >> 20);
            /* TODO: Investigate transport common fields outside both native object pools */
            XASSERT(model);
            model->angles.pitch_bits = (uint16)bounce;
            /* TODO: Investigate a vehicle exit scene without a native position */
            XASSERT(model);
            model->pos.y.bits = (uint32)(bounce >> 6) << 15;
        }
        else if (elapsed < 129)
        {
            bounce = -((sint32)((uint32)sin_table[((elapsed - 120u) << 8) >> 1] << 16) >> 21);
            /* TODO: Investigate transport common fields outside both native object pools */
            XASSERT(model);
            model->angles.pitch_bits = (uint16)bounce;
            /* TODO: Investigate a vehicle exit scene without a native position */
            XASSERT(model);
            model->pos.y.bits = (uint32)(bounce >> 6) << 14;
        }
        else
        {
            if (elapsed != 129)
                return 129;
            level_wave.paused = 0;
            level_script_scene.object = (SCENE_OBJECT_REFERENCE){0};
        }
        goto region_update;
    }
    wave = sin_table[offset >> 1];
    /* TODO: Investigate a vehicle exit scene without a native position */
    XASSERT(model);
    pos = model->pos.x.bits;
    model->pos.x.bits = pos - speed * wave;
    if (elapsed >= 59)
        speed = model->scene_vehicle.speed;
    wave = sin_table[MATH_QUARTER_TURN + (offset >> 1)];
    pos = model->pos.z.bits;
    model->pos.z.bits = pos + speed * wave;
    if (elapsed >= 16 && elapsed < 59)
        model->scene_vehicle.speed = 129;
    if (elapsed == 90)
        model->scene_vehicle.voice_plus_one[0] = (uint16)(sound_voice_play(131, 255, 5632) + 1);
region_update:
    region = col_regions.data;
    count = col_regions.count;
    if (count <= 0)
        return count;
    for (i = 0; i < count; ++i, ++region)
    {
        XASSERT(region != col_regions.data + COLLISION_REGION_COUNT);
        /* TODO: Investigate a corrupt native world reference before region matching */
        XASSERT(region->object.present <= 1 && (!region->object.present || region->object.idx < WORLD_OBJECT_COUNT));
        if (region->object.present && model == world_objects.object + region->object.idx)
            break;
    }
    if (i < (sint32)col_regions.count)
        return actors_collide_vehicle(region, model);
    return 0;
}
