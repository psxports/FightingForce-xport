#include "game.h"

VEHICLE_STEERING vehicle_steering;

sint32 vehicle_steering_draw(MODEL_OBJECT *item)
{
    FUNCTION_MARKER(0x80032e20u, "GAME.EXE");
    /* TODO: Investigate common draw fields outside both native object pools */
    XASSERT(item);
    sint32 result, angle = item->angles.yaw;
    if (angle)
        transform_yaw(angle);
    model_draw_faded(model_refs[item->kind], 0, 0);
    /* TODO: Investigate forklift payload access with a foreign native owner */
    XASSERT(item && item->payload_kind == MODEL_PAYLOAD_PHYSICS);
    transform_translate(0, (uint32)(sint32)item->physics.fork_height << 16, 0x00d00000);
    model_draw_faded(model_refs[393], 0, 0);
    result = sin_table[((game_session.tick << 5) & 0x7e0u) >> 1] >> 7;
    item->physics.fork_height = (uint16)result;
    return result;
}

static sint32 vehicle_steering_hit_obstacle(MODEL_OBJECT *model, uint32 x, uint32 y, uint32 z)
{
    FUNCTION_MARKER(0x8003f4c4u, "GAME.EXE");
    sint32 hit = regions_find_projectile_obstacle((sint32)x, (sint32)z);
    const MODEL_OBJECT *target;
    FIXED_VEC3 pos;
    if (hit < 0)
        return (uint32)hit << 1;
    XASSERT((uint32)hit < COLLISION_REGION_COUNT);
    WORLD_OBJECT_REFERENCE ref = col_regions.data[hit].object;
    /* TODO: Investigate a projectile contact with a corrupt world reference */
    XASSERT(ref.present <= 1 && (!ref.present || ref.idx < WORLD_OBJECT_COUNT));
    target = ref.present ? world_objects.object + ref.idx : NULL;
    if (!target || target->kind != 367)
        return 367;
    pos.x.bits = x;
    pos.y.bits = y;
    pos.z.bits = z;
    effect_spawn(894, x, y, z);
    /* 31C50 saves S3=x at entry-SP-12, consumed by 1F004 */
    projectile_contact_vals(hit, 0, pos.bits, x);
    /* TODO: Investigate forklift payload access with a foreign native owner */
    XASSERT(model && model->payload_kind == MODEL_PAYLOAD_PHYSICS);
    model->physics.reverse_ticks = 40;
    model->physics.reverse = 1;
    model->physics.speed = 15;
    return 15;
}

static sint32 vehicle_steering_hit_actor(sint32 idx, MODEL_OBJECT *model, sint32 x, sint32 z)
{
    FUNCTION_MARKER(0x8003f5a0u, "GAME.EXE");
    sint32 first, second;
    x >>= 16;
    z >>= 16;
    first = math_distance_approx(x, z, actor_positions[idx].x.whole, actor_positions[idx].z.whole);
    second = math_distance_approx(x, z, actor_positions[idx].x.whole, actor_positions[idx].z.whole);
    if (first >= 80 && second >= 80)
        return 0;
    if (actor_animations[idx].id == 147)
        return 147;
    animation_start_held(idx, animation_type_banks[(uint32)actor_types[idx]], 147);
    /* TODO: Investigate forklift payload access with a foreign native owner */
    XASSERT(model && model->payload_kind == MODEL_PAYLOAD_PHYSICS);
    model->physics.reverse_ticks = 60;
    model->physics.reverse = 1;
    model->physics.speed = 10;
    return 10;
}

static sint32 vehicle_steering_probe(MODEL_OBJECT *model, uint32 dx, uint32 dz)
{
    FUNCTION_MARKER(0x8003f670u, "GAME.EXE");
    /* TODO: Investigate forklift payload access with a foreign native owner */
    XASSERT(model && model->payload_kind == MODEL_PAYLOAD_PHYSICS);
    uint32 x = model->pos.x.bits + dx;
    uint32 z = model->pos.z.bits + dz;
    uint32 y;
    if (!regions_find_obstacle(x, z))
        return 0;
    x = model->pos.x.bits + dx;
    z = model->pos.z.bits + dz;
    y = model->pos.y.bits;
    vehicle_steering_hit_obstacle(model, x, y, z);
    return 1;
}

sint32 vehicle_steering_move(MODEL_OBJECT *model)
{
    FUNCTION_MARKER(0x8003f6ecu, "GAME.EXE");
    /* TODO: Investigate common motion fields outside the native object pools */
    XASSERT(model);
    /* TODO: Investigate forklift payload access with a foreign native owner */
    XASSERT(model && model->payload_kind == MODEL_PAYLOAD_PHYSICS);
    uint32 angle = (model->angles.yaw >> 5) & 2047u, dx, dz;
    sint32 speed = model->physics.speed, result;
    const uint32 speed_word = speed;
    dx = speed_word * (sin_table[angle]);
    dz = speed_word * (sin_table[(angle + 512u) & 2047u]);
    if (model->physics.reverse == 1)
    {
        /* TODO: Investigate vehicle or movement callbacks without a native position */
        XASSERT(model);
        model->pos.x.bits = model->pos.x.bits - dx;
        model->physics.reverse_ticks = (uint16)(model->physics.reverse_ticks - 1);
        result = (uint32)(uint16)model->physics.reverse_ticks << 16;
        model->pos.z.bits = model->pos.z.bits + dz;
        if (result < 0)
            model->physics.reverse = 0;
        return result;
    }
    /* TODO: Investigate vehicle or movement callbacks without a native position */
    XASSERT(model);
    model->pos.x.bits = model->pos.x.bits + dx;
    model->pos.z.bits = model->pos.z.bits - dz;
    result = speed + 2;
    if (speed < 30)
        model->physics.speed = (uint16)result;
    return result;
}

static sint32 apply_turn(MODEL_OBJECT *model)
{
    FUNCTION_MARKER(0x8003f81cu, "GAME.EXE");
    /* TODO: Investigate forklift payload access with a foreign native owner */
    XASSERT(model && model->payload_kind == MODEL_PAYLOAD_PHYSICS);
    uint32 input = vehicle_steering.hit_flags, probe_idx, delta;
    sint32 error, result;
    if (input)
    {
        if ((input & 1u) || (input & 6u) == 6u)
        {
            if (model->physics.reverse == 1 && ((uint16)model->physics.reverse_ticks & 1u))
                model->physics.speed = 0;
            else
            {
                model->physics.reverse = (uint16)((uint16)model->physics.reverse ^ 1u);
                if (model->physics.reverse == 1)
                    model->physics.reverse_ticks = 35;
            }
        }
        else if (input & 6u)
        {
            probe_idx = input & 2u ? VEHICLE_PROBE_FAR_RIGHT : VEHICLE_PROBE_FAR_LEFT;
            delta = model->pos.x.bits + vehicle_steering.probe[probe_idx].x.bits;
            vehicle_steering.target.x.bits = delta;
            vehicle_steering.target.z.bits = model->pos.z.bits + vehicle_steering.probe[probe_idx].z.bits;
        }
    }
    delta = (uint32)math_dir16(model->pos.x.whole, model->pos.z.whole, vehicle_steering.target.x.whole, vehicle_steering.target.z.whole) - (uint16)model->angles.yaw;
    vehicle_steering.turn_error = (sint16)delta;
    error = (sint16)delta;
    if (error < 0)
        error = -error;
    if (error > 1024 && dir_distance < 300 && !model->physics.reverse)
    {
        model->physics.reverse_ticks = 25;
        model->physics.reverse = 1;
        model->physics.speed = 30;
    }
    result = vehicle_steering.turn_error >> (vehicle_steering.hit_flags ? 4 : 6);
    model->angles.yaw_bits = (uint16)(((uint16)model->angles.yaw) + (uint32)result);
    return result;
}

uint32 vehicle_steering_update(MODEL_OBJECT *model)
{
    FUNCTION_MARKER(0x8003f9c0u, "GAME.EXE");
    uint32 x = actor_positions[0].x.bits, z = actor_positions[0].z.bits, offset, dx, dz;
    FIXED_XZ *probe;
    sint32 distance;
    vehicle_steering.hit_flags = 0;
    vehicle_steering.target.x.bits = x;
    vehicle_steering.target.z.bits = z;
    /* TODO: Investigate vehicle steering without a native position */
    XASSERT(model);
    distance = math_distance_approx((sint32)x >> 16, (sint32)z >> 16, model->pos.x.whole, model->pos.z.whole);
    vehicle_steering_move(model);
    if (distance >= 65)
    {
        probe = vehicle_steering.probe + VEHICLE_PROBE_NEAR_LEFT;
        offset = 2u * (((model->angles.yaw >> 5) - 64) & 2047);
        dx = (sin_table[offset >> 1]) * 512u;
        dz = (uint32)(-(sint64)((sin_table[MATH_QUARTER_TURN + (offset >> 1)]) * 512u));
        probe->x.bits = dx;
        probe->z.bits = dz;
        if (model->physics.reverse == 1)
        {
            probe->x.bits = (uint32)(-(sint64)(dx));
            probe->z.bits = (uint32)(-(sint64)(dz));
        }
        if (vehicle_steering_probe(model, probe->x.bits, probe->z.bits))
            vehicle_steering.hit_flags |= VEHICLE_HIT_NEAR;
        probe = vehicle_steering.probe + VEHICLE_PROBE_NEAR_RIGHT;
        offset = 2u * (((model->angles.yaw >> 5) + 64) & 2047);
        dx = (sin_table[offset >> 1]) * 512u;
        dz = (uint32)(-(sint64)((sin_table[MATH_QUARTER_TURN + (offset >> 1)]) * 512u));
        probe->x.bits = dx;
        probe->z.bits = dz;
        if (model->physics.reverse == 1)
        {
            probe->x.bits = (uint32)(-(sint64)(dx));
            probe->z.bits = (uint32)(-(sint64)(dz));
        }
        if (vehicle_steering_probe(model, probe->x.bits, probe->z.bits))
            vehicle_steering.hit_flags |= VEHICLE_HIT_NEAR;
        probe = vehicle_steering.probe + VEHICLE_PROBE_FAR_LEFT;
        offset = 2u * (((model->angles.yaw >> 5) - 112) & 2047);
        dx = (sin_table[offset >> 1]) * 900u;
        dz = (uint32)(-(sint64)((sin_table[MATH_QUARTER_TURN + (offset >> 1)]) * 900u));
        probe->x.bits = dx;
        probe->z.bits = dz;
        if (model->physics.reverse == 1)
        {
            probe->x.bits = (uint32)(-(sint64)(dx));
            probe->z.bits = (uint32)(-(sint64)(dz));
        }
        if (vehicle_steering_probe(model, probe->x.bits, probe->z.bits))
            vehicle_steering.hit_flags |= VEHICLE_HIT_FAR_LEFT;
        probe = vehicle_steering.probe + VEHICLE_PROBE_FAR_RIGHT;
        offset = 2u * (((model->angles.yaw >> 5) + 112) & 2047);
        dx = (sin_table[offset >> 1]) * 900u;
        dz = (uint32)(-(sint64)((sin_table[MATH_QUARTER_TURN + (offset >> 1)]) * 900u));
        probe->x.bits = dx;
        probe->z.bits = dz;
        if (model->physics.reverse == 1)
        {
            probe->x.bits = (uint32)(-(sint64)(dx));
            probe->z.bits = (uint32)(-(sint64)(dz));
        }
        if (vehicle_steering_probe(model, probe->x.bits, probe->z.bits))
            vehicle_steering.hit_flags |= VEHICLE_HIT_FAR_RIGHT;
    }
    apply_turn(model);
    if (!model->physics.reverse)
    {
        probe = vehicle_steering.probe + VEHICLE_PROBE_NEAR_LEFT;
        vehicle_steering_hit_actor(0, model, model->pos.x.bits + probe->x.bits, model->pos.z.bits + probe->z.bits);
        probe = vehicle_steering.probe + VEHICLE_PROBE_NEAR_RIGHT;
        vehicle_steering_hit_actor(0, model, model->pos.x.bits + probe->x.bits, model->pos.z.bits + probe->z.bits);
        probe = vehicle_steering.probe + VEHICLE_PROBE_FAR_LEFT;
        vehicle_steering_hit_actor(0, model, model->pos.x.bits + probe->x.bits, model->pos.z.bits + probe->z.bits);
        probe = vehicle_steering.probe + VEHICLE_PROBE_FAR_RIGHT;
        vehicle_steering_hit_actor(0, model, model->pos.x.bits + probe->x.bits, model->pos.z.bits + probe->z.bits);
    }
    /* The original no-op debug tail leaves the live object Z in V0 */
    return model->pos.z.bits;
}
