#include "game.h"

static sint32 query_actor(uint32 world_x, uint32 world_z, const MODEL_OBJECT *model, const PROJECTILE_SOURCE *projectile)
{
    FUNCTION_MARKER(0x80019ba8u, "GAME.EXE");
    uint32 height;
    sint32 owner, x, z;
    int i;
    for (i = 0; i < ACTOR_COUNT; i++)
    {
        if (actor_animation_blocks_hits(actor_animations[i].id))
            continue;
        /* TODO: Investigate an absent or foreign native projectile owner */
        if (!projectile && (!model || !(model->payload_kind == MODEL_PAYLOAD_PHYSICS || model->payload_kind == MODEL_PAYLOAD_AIR_BIKE || model->payload_kind == MODEL_PAYLOAD_SCENE_VEHICLE || model->payload_kind == MODEL_PAYLOAD_SCENE_HELICOPTER || model->payload_kind == MODEL_PAYLOAD_COMBAT_HELICOPTER || model->payload_kind == MODEL_PAYLOAD_PATROL_HELICOPTER)))
        {
            DIAGNOSTIC(fprintf(stderr, "Invalid projectile query model %p stage %u tick %u\n", (const void *)model, game_session.stage, game_session.tick));
            diagnostic_stop(__FUNCTION__, __FILE__, __LINE__);
        }
        owner = projectile ? projectile->owner : model && model->payload_kind == MODEL_PAYLOAD_PHYSICS ? model->physics.actor_idx : (model && model->payload_kind == MODEL_PAYLOAD_AIR_BIKE ? model->air_bike.target[1].z : (model && model->payload_kind == MODEL_PAYLOAD_SCENE_VEHICLE ? model->scene_vehicle.voice_plus_one[0] : (model && model->payload_kind == MODEL_PAYLOAD_SCENE_HELICOPTER ? model->scene_helicopter.active : (model && model->payload_kind == MODEL_PAYLOAD_COMBAT_HELICOPTER ? model->helicopter.voice_plus_one[0] : model->patrol.rotor_angle))));
        if (owner == i)
            continue;
        height = actor_positions[i].y.bits;
        height += actor_animations[i].motion.bits[0];
        if ((sint32)height < -16777216)
            continue;
        if (actor_contacts[i].captor != -1 || actor_contacts[i].captive != -1 || actor_health[i].hp <= 0)
            continue;
        x = actor_positions[i].x.whole;
        z = actor_positions[i].z.whole;
        if ((sint32)math_distance_approx((sint32)world_x >> 16, (sint32)world_z >> 16, x, z) < 64)
            return i;
    }
    return -1;
}

GDB_CALL sint32 projectile_query_model(uint32 world_x, uint32 world_z, const MODEL_OBJECT *model)
{
    return query_actor(world_x, world_z, model, NULL);
}

GDB_CALL sint32 projectile_query_src(uint32 world_x, uint32 world_z, const PROJECTILE_SOURCE *projectile)
{
    return query_actor(world_x, world_z, NULL, projectile);
}

/* 1F004: retained_word models entry-SP-12, not an original register argument.
 * Original initializes only XYZ at SP+16..24; callbacks can read SP+28. */
static sint32 apply_contact(sint32 idx, const MODEL_OBJECT *model, uintptr_t retained_word, const uint32 native_pos[3], uint32 native_kind)
{
    FUNCTION_MARKER(0x8001f004u, "GAME.EXE");
    COLLISION_REGION *obstacle;
    if ((uint32)idx >= COLLISION_REGION_COUNT)
        diagnostic_stop(__FUNCTION__, __FILE__, __LINE__);
    obstacle = col_regions.data + idx;
    /* TODO: Investigate projectile common fields outside both native object pools */
    if (!native_pos && !model)
    {
        DIAGNOSTIC(fprintf(stderr, "Invalid projectile model object %p in %s stage %u tick %u\n", (const void *)model, __FUNCTION__, game_session.stage, game_session.tick));
        diagnostic_stop(__FUNCTION__, __FILE__, __LINE__);
    }
    uint32 kind = native_pos ? native_kind : model->kind;
    MODEL_OBJECT *target_model = (obstacle->object.present == 1 && obstacle->object.idx < WORLD_OBJECT_COUNT ? world_objects.object + obstacle->object.idx : NULL);
    COLLISION_POINT point;
    if (kind == 17)
        return 51;
    if (kind == 51)
        return kind - 499u;
    if (kind - 499u >= 5u && kind != 504)
        obstacle->hits = 0xffff;
    const uint32 *pos_bits = native_pos ? native_pos : model ? model->pos.bits : NULL;
    /* TODO: Investigate a projectile source without a native position */
    if (!pos_bits)
    {
        DIAGNOSTIC(fprintf(stderr, "Invalid projectile position %p stage %u tick %u\n", (const void *)model, game_session.stage, game_session.tick));
        diagnostic_stop(__FUNCTION__, __FILE__, __LINE__);
    }
    point.pos.bits[0] = pos_bits[0];
    point.pos.bits[1] = pos_bits[1];
    point.pos.bits[2] = pos_bits[2];
    point.impact_sound = retained_word;
    sint32 handler_idx = obstacle->col_kind;
    if (handler_idx < 0 || handler_idx >= COLLISION_SLOT_COUNT)
        diagnostic_stop(__FUNCTION__, __FILE__, __LINE__);
    return col_call(col_callbacks[handler_idx], obstacle, target_model, &point, -1);
}

GDB_CALL sint32 projectile_contact_model(sint32 idx, const MODEL_OBJECT *model, uint32 retained_word)
{
    return apply_contact(idx, model, retained_word, NULL, 0);
}

/* Retained word is the original caller's entry-SP-12 carrier */
GDB_CALL sint32 projectile_contact_vals(sint32 idx, uint32 kind, const uint32 pos[3], uint32 retained_word)
{
    return apply_contact(idx, NULL, retained_word, pos, kind);
}

/* 2FD90: 64 ray steps; entry-SP-12 is saved S5 from199AC or S3 from31C50. */
static sint32 trace(MODEL_OBJECT *model, sint32 idx, uint32 native_pos[3], uint16 native_angle, uint32 native_kind)
{
    FUNCTION_MARKER(0x8002fd90u, "GAME.EXE");
    /* TODO: Investigate projectile common fields outside both native object pools */
    if (!native_pos && !model)
    {
        DIAGNOSTIC(fprintf(stderr, "Invalid projectile model object %p in %s stage %u tick %u\n", (void *)model, __FUNCTION__, game_session.stage, game_session.tick));
        diagnostic_stop(__FUNCTION__, __FILE__, __LINE__);
    }
    uint32 angle = (native_pos ? native_angle : model->angles.yaw_bits) >> 5;
    uint32 dx = (uint32)(sint32)sin_table[angle] << 7;
    uint32 dz = (uint32)(sint32)sin_table[MATH_QUARTER_TURN + angle] << 7;
    uint32 *pos_bits = native_pos ? native_pos : model ? model->pos.bits : NULL;
    /* TODO: Investigate a projectile source without a native position */
    if (!pos_bits)
    {
        DIAGNOSTIC(fprintf(stderr, "Invalid projectile position %p stage %u tick %u\n", (void *)model, game_session.stage, game_session.tick));
        diagnostic_stop(__FUNCTION__, __FILE__, __LINE__);
    }
    uint32 x = pos_bits[0];
    uint32 z = pos_bits[2];
    uint32 kind, effect_x, effect_y, effect_z;
    uintptr_t retained;
    PROJECTILE_SOURCE native_projectile;
    int i;
    sint32 hit;
    if (native_pos)
    {
        native_projectile.kind = native_kind;
        native_projectile.height = native_pos[1];
        native_projectile.angle = native_angle;
        native_projectile.owner = (sint16)idx;
    }
    for (i = 0; i < 64; i++)
    {
        hit = regions_find_projectile_obstacle((sint32)x, (sint32)z);
        if (hit >= 0)
        {
            kind = native_pos ? native_projectile.kind : model->kind;
            retained = dz;
            if (kind - 499u >= 5u && kind - 504u >= 2u)
            {
                effect_x = pos_bits[0];
                effect_y = pos_bits[1];
                effect_z = pos_bits[2];
                effect_spawn(894, effect_x, effect_y, effect_z);
                /* Preserve the native pointer width of this nonzero impact carrier */
                retained = native_pos ? (uintptr_t)&native_projectile : (uintptr_t)model;
            }
            apply_contact(hit, model, retained, native_pos, native_kind);
            return 1;
        }
        hit = native_pos ? projectile_query_src(x, z, &native_projectile) : projectile_query_model(x, z, model);
        if (hit >= 0 && hit != idx)
        {
            kind = native_pos ? native_projectile.kind : model->kind;
            if (kind - 499u >= 5u && kind - 504u >= 2u)
            {
                effect_y = pos_bits[1];
                effect_spawn(894, x, effect_y, z);
            }
            if (native_pos)
                actor_projectile_impact_src(x, z, &native_projectile);
            else
                actor_projectile_impact(x, z, model, NULL);
            return 1;
        }
        x += dx;
        z -= dz;
        pos_bits[0] = x;
        pos_bits[2] = z;
    }
    return 0;
}

GDB_CALL sint32 projectile_trace_model(MODEL_OBJECT *model, sint32 idx)
{
    return trace(model, idx, NULL, 0, 0);
}

GDB_CALL sint32 projectile_trace_npc(uint32 pos[3], uint16 angle, sint32 idx)
{
    return trace(NULL, idx, pos, angle, 499);
}

GDB_CALL sint32 projectile_trace_weapon(uint32 pos[3], uint32 kind, uint16 angle, sint32 idx)
{
    return trace(NULL, idx, pos, angle, kind);
}

GDB_CALL intptr projectile_update_accelerating(MODEL_OBJECT *model)
{
    FUNCTION_MARKER(0x8002fa90u, "GAME.EXE");
    uint32 offset, x, y, z, dx, dy, dz, result;
    sint32 slot, speed, hit, src_x, src_z, target_x, target_z;
    int i;
    /* TODO: Investigate motion callbacks without a native world or dynamic position */
    if (!model)
    {
        DIAGNOSTIC(fprintf(stderr, "Invalid object motion position %p stage %u tick %u\n", (void *)model, game_session.stage, game_session.tick));
        diagnostic_stop(__FUNCTION__, __FILE__, __LINE__);
    }
    x = model->pos.x.bits;
    y = model->pos.y.bits;
    z = model->pos.z.bits;
    dynamic_objects_spawn(575, x, y, z);
    /* TODO: Investigate projectile callbacks with a foreign payload owner */
    if (model->payload_kind != MODEL_PAYLOAD_PHYSICS)
    {
        DIAGNOSTIC(fprintf(stderr, "Invalid projectile payload %p kind %u owner %u stage %u tick %u\n", (void *)model, model->kind, (uint32)model->payload_kind, game_session.stage, game_session.tick));
        diagnostic_stop(__FUNCTION__, __FILE__, __LINE__);
    }
    slot = model->physics.target_idx;
    if (slot != -1)
    {
        src_x = model->pos.x.whole;
        src_z = model->pos.z.whole;
        /* TODO: Investigate a projectile target outside the actor pool */
        if (slot < 0 || slot >= ACTOR_COUNT)
        {
            DIAGNOSTIC(fprintf(stderr, "Invalid projectile target %d object %p stage %u tick %u\n", slot, (void *)model, game_session.stage, game_session.tick));
            diagnostic_stop(__FUNCTION__, __FILE__, __LINE__);
        }
        target_x = actor_positions[slot].x.whole;
        target_z = actor_positions[slot].z.whole;
        model->angles.yaw_bits = (uint16)math_dir16(src_x, src_z, target_x, target_z);
    }
    offset = (model->angles.yaw_bits >> 5) * 2;
    speed = model->physics.speed;
    dx = (speed * sin_table[offset >> 1]) >> 2;
    dy = (speed * sin_table[model->angles.pitch_bits >> 5]) >> 2;
    dz = (speed * sin_table[MATH_QUARTER_TURN + (offset >> 1)]) >> 2;
    x = model->pos.x.bits;
    y = model->pos.y.bits;
    z = model->pos.z.bits;
    for (i = 0; i < 4; i++)
    {
        if (model->physics.speed > 100)
        {
            hit = regions_find_projectile_obstacle(x, z);
            if (hit >= 0)
            {
                sound_voice_play(19, 255, 6144);
                effect_spawn(893, x, model->pos.y.bits, z);
                disp_buffers.shake_ticks = 16;
                disp_buffers.flash_ticks = 8;
                /* 31C50 saves caller S3=x at entry SP-12, read by 1F004. */
                projectile_contact_model(hit, model, x);
                actors_apply_radial_damage(x, z, NULL, model);
                model->kind = 0xffffffffu;
                return -1;
            }
            if (projectile_query_model(x, z, model) >= 0 || (sint32)model->pos.y.bits > 0)
            {
                sound_voice_play(19, 255, 6144);
                effect_spawn(893, x, model->pos.y.bits, z);
                disp_buffers.shake_ticks = 16;
                disp_buffers.flash_ticks = 8;
                actors_apply_radial_damage(x, z, NULL, model);
                model->kind = 0xffffffffu;
                result = model->pos.y.bits;
                if ((sint32)result > 0)
                {
                    x = model->pos.x.bits;
                    z = model->pos.z.bits;
                    return (intptr)effect_spawn(603, x, 0, z);
                }
                return (sint32)result;
            }
        }
        x += dx;
        y -= dy;
        z -= dz;
        model->pos.x.bits = x;
        model->pos.y.bits = y;
        model->pos.z.bits = z;
        if (i & 1)
        {
            uint32 trail_x = model->pos.x.bits;
            uint32 trail_y = model->pos.y.bits;
            dynamic_objects_spawn(575, trail_x, trail_y, z);
        }
    }
    result = (uint16)model->physics.vertical_speed - 4;
    model->physics.vertical_speed = (uint16)result;
    result = (sint32)(result << 16) >> 1;
    y = model->pos.y.bits;
    model->pos.y.bits = y - result;
    result = (uint16)model->physics.speed + 64;
    model->physics.speed = (uint16)result;
    return (sint32)result;
}

GDB_CALL uint32 projectile_update_ballistic(MODEL_OBJECT *model, uint32 retained_s3)
{
    FUNCTION_MARKER(0x80041b54u, "GAME.EXE");
    uint32 angle, speed, x, y, z, dx, dz, result;
    sint32 hit, contact, target, src_x, src_z, target_x, target_z;
    /* TODO: Investigate emitter operands outside both native object pools */
    if (!model)
    {
        DIAGNOSTIC(fprintf(stderr, "Invalid emitter position %p in %s stage %u tick %u\n", (void *)model, __FUNCTION__, game_session.stage, game_session.tick));
        diagnostic_stop(__FUNCTION__, __FILE__, __LINE__);
    }
    x = model->pos.x.bits;
    y = model->pos.y.bits;
    z = model->pos.z.bits;
    dynamic_objects_spawn(575, x, y, z);
    /* TODO: Investigate projectile callbacks with a foreign payload owner */
    if (model->payload_kind != MODEL_PAYLOAD_PHYSICS)
    {
        DIAGNOSTIC(fprintf(stderr, "Invalid projectile payload %p kind %u owner %u stage %u tick %u\n", (void *)model, model->kind, (uint32)model->payload_kind, game_session.stage, game_session.tick));
        diagnostic_stop(__FUNCTION__, __FILE__, __LINE__);
    }
    target = model->physics.target_idx;
    if (target != -1)
    {
        src_x = model->pos.x.whole;
        src_z = model->pos.z.whole;
        /* TODO: Investigate projectile target indices outside the native actor pool */
        if (target < 0 || target >= ACTOR_COUNT)
        {
            DIAGNOSTIC(fprintf(stderr, "Invalid projectile target %d object %p stage %u tick %u\n", target, (void *)model, game_session.stage, game_session.tick));
            diagnostic_stop(__FUNCTION__, __FILE__, __LINE__);
        }
        const FIXED_VEC3 *target_pos = actor_positions + target;
        target_x = target_pos->x.whole;
        target_z = target_pos->z.whole;
        model->angles.yaw_bits = (uint16)math_dir16(src_x, src_z, target_x, target_z);
    }
    angle = (model->angles.yaw_bits >> 5) * 2;
    speed = (uint32)(sint32)model->physics.speed;
    dx = speed * (uint32)(sint32)sin_table[angle >> 1];
    dz = speed * (uint32)(sint32)sin_table[MATH_QUARTER_TURN + (angle >> 1)];
    x = model->pos.x.bits + dx;
    model->pos.x.bits = x;
    z = model->pos.z.bits - dz;
    model->pos.z.bits = z;
    x = model->pos.x.bits;
    hit = regions_find_projectile_obstacle(x, z);
    contact = hit;
    if (hit < 0)
    {
        x = model->pos.x.bits;
        z = model->pos.z.bits;
        contact = projectile_query_model(x, z, model);
    }
    if (contact >= 0)
    {
        sound_voice_play(19, 255, 6144);
        x = model->pos.x.bits;
        y = model->pos.y.bits;
        z = model->pos.z.bits;
        effect_spawn(893, x, y, z);
        disp_buffers.shake_ticks = 16;
        disp_buffers.flash_ticks = 8;
        /* 31C50 saves inherited S3 at SP-12; 1F004 reads that retained word. */
        if (hit >= 0)
            projectile_contact_model(hit, model, retained_s3);
        x = model->pos.x.bits;
        z = model->pos.z.bits;
        actors_apply_radial_damage(x, z, NULL, model);
        model->kind = 0xffffffffu;
        return 0xffffffffu;
    }
    result = (uint16)model->physics.vertical_speed - 1;
    model->physics.vertical_speed = (uint16)result;
    result = (sint32)(result << 16) >> 1;
    y = model->pos.y.bits - result;
    model->pos.y.bits = y;
    if ((sint32)y <= 0)
        return result;
    model->kind = 0xffffffffu;
    return 0xffffffffu;
}

GDB_CALL void projectile_draw_glow(const struct MODEL_OBJECT *item)
{
    FUNCTION_MARKER(0x8005cd1cu, "GAME.EXE");
    PROJECTION_POINT pos;
    transform_push();
    object_transform_camera(item, NULL);
    proj_local(0, 0, 0, &pos);
    if (!pos.status)
    {
        pos.size = game_session.tick & 1u ? 704 : 512;
        render_style.color.rgbc = 0x2cffffffu;
        spr_draw_scaled(&pos, tex_map[66]);
    }
    transform_pop();
}
