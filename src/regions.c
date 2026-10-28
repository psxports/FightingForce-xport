#include "game.h"

COLLISION_REGIONS col_regions;
REGION_TRIGGER_REFERENCE region_triggers[REGION_KIND_COUNT];

static const sint16 pulse_scale = 64;

GDB_CALL uint32 regions_update_pickup(sint32 idx, uint32 retained_xyz[3])
{
    FUNCTION_MARKER(0x80033f7cu, "GAME.EXE");
    uint32 kind, type, x, z;
    COLLISION_REGION *region;
    FIXED_VEC3 point;
    sint32 turn;
    if ((uint32)idx >= ACTOR_COUNT || !col_regions.actor[idx].region)
        diagnostic_stop(__FUNCTION__, __FILE__, __LINE__);
    region = col_regions.actor[idx].region;
    WORLD_OBJECT_REFERENCE object_ref = region->object;
    region->trigger_kind = 17;
    turn = col_regions.actor[idx].heading;
    if (actor_animations[idx].motion.events & 0x800)
    {
        /* TODO: Investigate a pickup region without a valid world owner */
        if (object_ref.present != 1 || object_ref.idx >= WORLD_OBJECT_COUNT)
        {
            DIAGNOSTIC(fprintf(stderr, "Invalid pickup region world %u present %u actor %d stage %u tick %u\n", object_ref.idx, (uint32)object_ref.present, idx, game_session.stage, game_session.tick));
            diagnostic_stop(__FUNCTION__, __FILE__, __LINE__);
        }
        MODEL_OBJECT *model = world_objects.object + object_ref.idx;
        kind = model->kind;
        if (kind == 627)
            type = 493;
        else if (kind == 663)
            type = 494;
        else if (model->kind == 813)
            type = 511;
        else
            return 813;
        ++actor_bonus_counts[idx];
        model->kind = kind == 663 ? 664 : model->kind + 1;
        /* TODO: Recover the original unwritten pickup coordinates for this caller */
        if (!retained_xyz)
        {
            DIAGNOSTIC(fprintf(stderr, "Missing pickup coordinates actor %d kind %u stage %u tick %u\n", idx, kind, game_session.stage, game_session.tick));
            diagnostic_stop(__FUNCTION__, __FILE__, __LINE__);
        }
        /* Preserve caller coordinates when their original carrier is known */
        actor_items[idx].slot[ACTOR_ITEM_RIGHT] = (sint8)dynamic_objects_spawn_item(type, retained_xyz[0], retained_xyz[1], retained_xyz[2], 0);
        return sound_voice_play(32, 255, 6144);
    }
    if (((sint16)actor_animations[idx].event_step))
        return sound_play_effort_voice(idx);
    if (!retained_xyz)
        retained_xyz = point.bits;
    actor_orientation[idx].yaw = (uint16)turn;
    actor_attachment_pos(idx, 3, retained_xyz);
    /* TODO: Investigate a pickup region without a valid world owner */
    if (object_ref.present != 1 || object_ref.idx >= WORLD_OBJECT_COUNT)
    {
        DIAGNOSTIC(fprintf(stderr, "Invalid pickup region world %u present %u actor %d stage %u tick %u\n", object_ref.idx, (uint32)object_ref.present, idx, game_session.stage, game_session.tick));
        diagnostic_stop(__FUNCTION__, __FILE__, __LINE__);
    }
    const MODEL_OBJECT *model = world_objects.object + object_ref.idx;
    x = model->pos.x.bits - (retained_xyz[0] - actor_positions[idx].x.bits);
    z = model->pos.z.bits - (retained_xyz[2] - actor_positions[idx].z.bits);
    if ((uint32)(uint16)actor_types[idx] - 1u < 2u)
    {
        /* TODO: Investigate a pickup heading outside the original sine table */
        if (turn < 0 || turn >= MATH_SIN_COUNT)
        {
            DIAGNOSTIC(fprintf(stderr, "Invalid pickup sine heading %d actor %d stage %u tick %u\n", turn, idx, game_session.stage, game_session.tick));
            diagnostic_stop(__FUNCTION__, __FILE__, __LINE__);
        }
        x -= (uint32)(sint32)sin_table[turn] << 4;
        /* TODO: Investigate a pickup heading outside the repeated cosine quarter */
        if (turn >= MATH_ANGLE_COUNT)
        {
            DIAGNOSTIC(fprintf(stderr, "Invalid pickup cosine heading %d actor %d stage %u tick %u\n", turn, idx, game_session.stage, game_session.tick));
            diagnostic_stop(__FUNCTION__, __FILE__, __LINE__);
        }
        z += (uint32)(sint32)sin_table[MATH_QUARTER_TURN + turn] << 4;
    }
    if (turn == 512 || turn == 1536)
        actor_positions[idx].x.bits = x;
    if (!turn || turn == 1024)
        actor_positions[idx].z.bits = z;
    actor_animations[idx].motion.bits[1] = 0;
    actor_animations[idx].motion.bits[2] = 0;
    return 1024;
}

static sint32 light_val(COLLISION_REGION *region, sint32 idx, uint32 scale)
{
    sint32 radius = region->geometry.circle.radius, val, x, z, center_x, center_z, distance;
    x = actor_positions[idx].x.whole;
    z = actor_positions[idx].z.whole;
    center_x = region->geometry.circle.center_x;
    center_z = region->geometry.circle.center_z;
    distance = math_distance_approx(x, z, center_x, center_z);
    val = ((uint32)radius - (uint32)distance) * scale;
    if (!radius || (radius == -1 && (uint32)val == 0x80000000u))
        diagnostic_stop(__FUNCTION__, __FILE__, __LINE__);
    return val / radius;
}

static uint32 light_flag(ACTOR_LIGHTING *light)
{
    uint32 result = light->flags | 1u;
    light->flags = result;
    return result;
}

static uint32 face_angle(COLLISION_REGION *region, sint32 idx, uint32 base, uint16 flag, uint16 angle)
{
    if ((uint32)((uint16)actor_orientation[idx].yaw) - base >= 255u)
        return 0;
    ACTOR_REGION_CONTACT *contact = col_regions.actor + idx;
    contact->flags |= flag;
    if ((uint32)idx >= ACTOR_COUNT)
        diagnostic_stop(__FUNCTION__, __FILE__, __LINE__);
    contact->region = region;
    contact->heading = angle;
    return angle;
}

static uint32 light_warm(COLLISION_REGION *region, sint32 idx)
{
    FUNCTION_MARKER(0x8003060c, "GAME.EXE");
    ACTOR_LIGHTING *light;
    uint32 result;
    sint32 radius = region->geometry.circle.radius, val, x, z, center_x, center_z, distance;
    x = actor_positions[idx].x.whole;
    z = actor_positions[idx].z.whole;
    center_x = region->geometry.circle.center_x;
    center_z = region->geometry.circle.center_z;
    distance = math_distance_approx(x, z, center_x, center_z);
    val = ((uint32)radius - (uint32)distance) << 6;
    if (!radius || (radius == -1 && (uint32)val == 0x80000000))
        diagnostic_stop(__FUNCTION__, __FILE__, __LINE__);
    val /= radius;
    if ((uint32)idx >= ACTOR_COUNT)
        diagnostic_stop(__FUNCTION__, __FILE__, __LINE__);
    light = actor_lighting + idx;
    light->b = (val >> 2) - 48;
    light->g = (uint32)val - 48u;
    light->r = (uint32)val - 48u;
    result = light->flags | 1u;
    light->flags = result;
    return result;
}

static uint32 light_shade(COLLISION_REGION *region, sint32 idx)
{
    FUNCTION_MARKER(0x800306d4u, "GAME.EXE");
    ACTOR_LIGHTING *light;
    sint32 val = light_val(region, idx, 80);
    if (val >= 49)
        val = 48;
    if ((uint32)idx >= ACTOR_COUNT)
        diagnostic_stop(__FUNCTION__, __FILE__, __LINE__);
    light = actor_lighting + idx;
    light->r = 0xffffffd0u - (uint32)val;
    light->g = 0xffffffd0u - (uint32)val;
    light->b = 0xffffffd0u - (uint32)val;
    return light_flag(light);
}

static uint32 light_radial(COLLISION_REGION *region, sint32 idx)
{
    FUNCTION_MARKER(0x800307acu, "GAME.EXE");
    ACTOR_LIGHTING *light;
    sint32 val = light_val(region, idx, 80);
    if ((uint32)idx >= ACTOR_COUNT)
        diagnostic_stop(__FUNCTION__, __FILE__, __LINE__);
    light = actor_lighting + idx;
    light->r = (uint32)val - 48u;
    light->g = (uint32)val - 48u;
    light->b = (uint32)val - 48u;
    return light_flag(light);
}

static uint32 light_g(COLLISION_REGION *region, sint32 idx)
{
    FUNCTION_MARKER(0x80030874u, "GAME.EXE");
    ACTOR_LIGHTING *light;
    sint32 val = light_val(region, idx, 80);
    if ((uint32)idx >= ACTOR_COUNT)
        diagnostic_stop(__FUNCTION__, __FILE__, __LINE__);
    light = actor_lighting + idx;
    light->r = (uint32)(val >> 1) - 48u;
    light->g = (uint32)val - 48u;
    light->b = (uint32)(val >> 1) - 48u;
    return light_flag(light);
}

static uint32 light_r(COLLISION_REGION *region, sint32 idx)
{
    FUNCTION_MARKER(0x80030944u, "GAME.EXE");
    ACTOR_LIGHTING *light;
    sint32 val = light_val(region, idx, 80);
    if ((uint32)idx >= ACTOR_COUNT)
        diagnostic_stop(__FUNCTION__, __FILE__, __LINE__);
    light = actor_lighting + idx;
    light->r = (uint32)val - 48u;
    light->g = (uint32)(val >> 2) - 48u;
    light->b = (uint32)(val >> 2) - 48u;
    return light_flag(light);
}

static uint32 light_strip(COLLISION_REGION *region, sint32 idx)
{
    FUNCTION_MARKER(0x80030a14u, "GAME.EXE");
    ACTOR_LIGHTING *light;
    uint32 result;
    sint32 start = region->geometry.coord[1];
    sint32 end = region->geometry.coord[3], mid = (start + end) >> 1;
    sint32 delta = actor_positions[idx].z.whole - mid, radius = end - mid, val;
    if (delta < 0)
        delta = -delta;
    val = 80u * ((uint32)radius - (uint32)delta);
    if (!radius || (radius == -1 && (uint32)val == 0x80000000u))
        diagnostic_stop(__FUNCTION__, __FILE__, __LINE__);
    val /= radius;
    if (val > 32)
        val = 32;
    result = 0xffffffd0u - (uint32)val;
    if ((uint32)idx >= ACTOR_COUNT)
        diagnostic_stop(__FUNCTION__, __FILE__, __LINE__);
    light = actor_lighting + idx;
    light->b = result;
    light->g = result;
    light->r = result;
    result = light->flags | 1u;
    light->flags = result;
    return result;
}

static uint32 light_dark(COLLISION_REGION *region, sint32 idx)
{
    FUNCTION_MARKER(0x80030ad4u, "GAME.EXE");
    ACTOR_LIGHTING *light;
    sint32 val = light_val(region, idx, 128);
    if ((uint32)idx >= ACTOR_COUNT)
        diagnostic_stop(__FUNCTION__, __FILE__, __LINE__);
    light = actor_lighting + idx;
    light->b = 0u - (uint32)val;
    light->g = 0u - (uint32)val;
    light->r = 0u - (uint32)val;
    return light_flag(light);
}

static uint32 light_add(COLLISION_REGION *region, sint32 idx)
{
    FUNCTION_MARKER(0x80030b94u, "GAME.EXE");
    ACTOR_LIGHTING *light;
    sint32 val = light_val(region, idx, 32);
    if ((uint32)idx >= ACTOR_COUNT)
        diagnostic_stop(__FUNCTION__, __FILE__, __LINE__);
    light = actor_lighting + idx;
    light->b = val;
    light->g = val;
    light->r = val;
    return light_flag(light);
}

static uint32 face_angle0(COLLISION_REGION *region, sint32 idx)
{
    FUNCTION_MARKER(0x80030c50u, "GAME.EXE");
    uint32 result = (uint32)((uint16)actor_orientation[idx].yaw) - 128u < 1793u;
    if (!result)
    {
        ACTOR_REGION_CONTACT *contact = col_regions.actor + idx;
        result = contact->flags | 1u;
        contact->flags = (uint16)result;
        if ((uint32)idx >= ACTOR_COUNT)
            diagnostic_stop(__FUNCTION__, __FILE__, __LINE__);
        contact->region = region;
        contact->heading = 0;
    }
    return result;
}

GDB_CALL sint32 regions_find_obstacle(uint32 x, uint32 z)
{
    FUNCTION_MARKER(0x80019718, "GAME.EXE");
    sint32 count = (sint32)col_regions.count, px = (sint32)x >> 16, pz = (sint32)z >> 16;
    COLLISION_REGION *region = col_regions.data;
    uint32 angle, dx, dz, cs, sn;
    sint32 kind, lx, lz, hx, hz, a, b, side;
    for (; count > 0; --count, ++region)
    {
        if (region == col_regions.data + COLLISION_REGION_COUNT)
            diagnostic_stop(__FUNCTION__, __FILE__, __LINE__);
        if (region->col_kind < 2)
            continue;
        kind = region->shape;
        if (!kind)
        {
            if (!(region->geometry.axis.min_x < px && px < region->geometry.axis.max_x && region->geometry.axis.min_z < pz && pz < region->geometry.axis.max_z))
                continue;
        }
        else if (kind == 1)
        {
            /* TODO: Investigate a rotated region reference outside the world pool */
            if (region->object.present != 1 || region->object.idx >= WORLD_OBJECT_COUNT)
            {
                DIAGNOSTIC(fprintf(stderr, "Invalid rotated region object %u present %u stage %u tick %u\n", region->object.idx, (uint32)region->object.present, game_session.stage, game_session.tick));
                diagnostic_stop(__FUNCTION__, __FILE__, __LINE__);
            }
            const MODEL_OBJECT *model = world_objects.object + region->object.idx;
            angle = (uint32)model->angles.yaw_bits >> 5;
            dx = (uint32)px - (uint32)(sint32)model->pos.x.whole;
            cs = (uint32)(sint32)sin_table[MATH_QUARTER_TURN + angle];
            dz = (uint32)pz - (uint32)(sint32)model->pos.z.whole;
            sn = (uint32)(sint32)sin_table[angle];
            lx = (sint32)(dx * cs + dz * sn) >> 15;
            lz = (sint32)(dz * cs - dx * sn) >> 15;
            hx = region->geometry.box.half_x;
            if (!(-hx < lx && lx < hx))
                continue;
            hz = region->geometry.box.half_z;
            if (!(-hz < lz && lz < hz))
                continue;
            a = (sint16)(uint16)math_dir11(lx, lz, 0, 0);
            hz = region->geometry.box.half_z;
            hx = region->geometry.box.half_x;
            b = (sint16)(uint16)math_dir11(0, 0, hx, -hz);
            if (a < b || a > 2048 - b)
                side = 0;
            else if (a < 1024 - b)
                side = 1;
            else
                side = a < b + 1024 ? 2 : 3;
            col_regions.side = (uint32)side;
        }
        else
        {
            sint32 center_x = region->geometry.circle.center_x;
            sint32 center_z = region->geometry.circle.center_z;
            sint32 distance = math_distance_approx(px, pz, center_x, center_z);
            if (distance >= region->geometry.circle.radius)
                continue;
        }
        col_regions.sel = region;
        return 1;
    }
    return 0;
}

static uint32 face_angle1024(COLLISION_REGION *region, sint32 idx)
{
    FUNCTION_MARKER(0x80030ca8u, "GAME.EXE");
    uint32 result = (uint32)((uint16)actor_orientation[idx].yaw) - 897u < 255u;
    if (result)
    {
        ACTOR_REGION_CONTACT *contact = col_regions.actor + idx;
        contact->flags |= 2u;
        if ((uint32)idx >= ACTOR_COUNT)
            diagnostic_stop(__FUNCTION__, __FILE__, __LINE__);
        contact->region = region;
        contact->heading = 1024;
        result = 1024;
    }
    return result;
}

static uint32 face_angle512(COLLISION_REGION *region, sint32 idx)
{
    FUNCTION_MARKER(0x80030d04, "GAME.EXE");
    return face_angle(region, idx, 385, 4, 512);
}

static uint32 face_angle1536(COLLISION_REGION *region, sint32 idx)
{
    FUNCTION_MARKER(0x80030d60, "GAME.EXE");
    return face_angle(region, idx, 1409, 8, 1536);
}

static uint32 use_item(COLLISION_REGION *region, sint32 idx)
{
    FUNCTION_MARKER(0x80030dbcu, "GAME.EXE");
    uint32 object, count, i, val;
    COLLISION_REGION *cursor;
    sint32 slot = actor_items[idx].slot[ACTOR_ITEM_RIGHT];
    if (slot == -1)
        return 244u * (uint32)idx;
    /* TODO: Investigate item use with an invalid held slot */
    if ((uint32)slot >= DYNAMIC_OBJECT_COUNT)
    {
        DIAGNOSTIC(fprintf(stderr, "Invalid region item actor %d slot %d stage %u tick %u\n", idx, slot, game_session.stage, game_session.tick));
        diagnostic_stop(__FUNCTION__, __FILE__, __LINE__);
    }
    MODEL_OBJECT *item = dynamic_objects + slot;
    if (item->kind != 514)
        return 299;
    /* TODO: Investigate a item use region reference outside the world pool */
    if (region->object.present != 1 || region->object.idx >= WORLD_OBJECT_COUNT)
    {
        DIAGNOSTIC(fprintf(stderr, "Invalid item use region object %u present %u stage %u tick %u\n", region->object.idx, (uint32)region->object.present, game_session.stage, game_session.tick));
        diagnostic_stop(__FUNCTION__, __FILE__, __LINE__);
    }
    /* Temporary peer comparison operand until region references replace raw identity */
    object = region->object.present ? 0x800b89f0 + 36u * region->object.idx : 0;
    MODEL_OBJECT *model = world_objects.object + region->object.idx;
    if (model->kind != 299)
        return 299;
    sound_voice_play(151, 192, 6144);
    disp_buffers.flash_ticks = 4;
    model->kind = 300;
    actor_items[idx].slot[ACTOR_ITEM_RIGHT] = -1;
    item->kind = 0xffffffffu;
    region->col_kind = 0;
    region->trigger_kind = 17;
    count = col_regions.count;
    if ((sint32)count <= 0)
        return count;
    cursor = col_regions.data;
    i = 0;
    do
    {
        if (cursor == col_regions.data + COLLISION_REGION_COUNT)
            diagnostic_stop(__FUNCTION__, __FILE__, __LINE__);
        if ((cursor->object.present ? 0x800b89f0 + 36u * cursor->object.idx : 0) == object && cursor != region)
        {
            val = (uint32)(sint32)model->pos.x.whole + 32u;
            cursor->geometry.coord[2] = val;
            cursor->geometry.coord[0] = val;
            val = (uint16)model->pos.z.whole;
            cursor->geometry.coord[3] = val;
            cursor->geometry.coord[1] = val;
            col_region_configure(cursor, 300);
            val = col_regions.count - 1u;
            col_regions.count = val;
            return val;
        }
        i++;
        count = col_regions.count;
        ++cursor;
    } while ((sint32)i < (sint32)count);
    return 0;
}

static uint32 light_pulse(COLLISION_REGION *region, sint32 idx)
{
    FUNCTION_MARKER(0x80047174u, "GAME.EXE");
    ACTOR_LIGHTING *light;
    sint32 radius = region->geometry.circle.radius, val, x, z, center_x, center_z, scale;
    x = actor_positions[idx].x.whole;
    z = actor_positions[idx].z.whole;
    center_x = region->geometry.circle.center_x;
    center_z = region->geometry.circle.center_z;
    val = math_distance_approx(x, z, center_x, center_z);
    scale = pulse_scale;
    val = ((uint32)radius - (uint32)val) * (uint32)scale;
    if (!radius || (radius == -1 && (uint32)val == 0x80000000u))
        diagnostic_stop(__FUNCTION__, __FILE__, __LINE__);
    val /= radius;
    if ((uint32)idx >= ACTOR_COUNT)
        diagnostic_stop(__FUNCTION__, __FILE__, __LINE__);
    light = actor_lighting + idx;
    light->r = -64;
    light->g = (uint32)((sint32)((uint32)val * 192u) >> 7) - 64u;
    light->b = -64;
    return light_flag(light);
}

const REGION_TRIGGER region_trigger_catalog[REGION_TRIGGER_COUNT] = {
    {light_warm}, {light_shade}, {light_radial}, {light_g}, {light_r}, {light_strip}, {light_dark}, {light_add}, {face_angle0}, {face_angle1024}, {face_angle512}, {face_angle1536}, {use_item}, {light_pulse},
};

static uint32 call_trigger(REGION_TRIGGER_REFERENCE target, COLLISION_REGION *region, sint32 actor)
{
    if (target)
        return target->call(region, actor);
    diagnostic_stop(__FUNCTION__, __FILE__, __LINE__);
    return 0;
}

GDB_CALL uint32 regions_apply_actor(sint32 idx)
{
    FUNCTION_MARKER(0x80019ca4, "GAME.EXE");
    /* Preserve original V0 on an empty region set, not a native pointer */
    uint32 result = 0x800b4318 + 244u * (uint32)idx;
    COLLISION_REGION *region = col_regions.data;
    sint32 left = (sint32)col_regions.count;
    /* TODO: Investigate a region update outside the native actor pool */
    if ((uint32)idx >= ACTOR_COUNT)
    {
        DIAGNOSTIC(fprintf(stderr, "Invalid region actor %d region count %d stage %u tick %u\n", idx, left, game_session.stage, game_session.tick));
        diagnostic_stop(__FUNCTION__, __FILE__, __LINE__);
    }
    sint32 x = actor_positions[idx].x.whole;
    sint32 z = actor_positions[idx].z.whole;
    for (; left > 0; left--, ++region)
    {
        if (region == col_regions.data + COLLISION_REGION_COUNT)
            diagnostic_stop(__FUNCTION__, __FILE__, __LINE__);
        REGION_TRIGGER_REFERENCE *slot, target;
        sint32 shape, kind;
        kind = region->trigger_kind;
        result = 4u * (uint32)kind;
        if (kind < 0 || kind >= REGION_KIND_COUNT)
            diagnostic_stop(__FUNCTION__, __FILE__, __LINE__);
        slot = region_triggers + kind;
        target = *slot;
        if (!target)
            continue;
        shape = region->shape;
        if (!shape)
        {
            result = region->geometry.axis.min_x < x;
            if (!result)
                continue;
            result = x < region->geometry.axis.max_x;
            if (!result)
                continue;
            result = region->geometry.axis.min_z < z;
            if (!result)
                continue;
            result = z < region->geometry.axis.max_z;
            if (!result)
                continue;
            result = call_trigger(target, region, idx);
            continue;
        }
        result = 2;
        if (shape == 1)
        {
            /* TODO: Investigate a rotated region reference outside the world pool */
            if (region->object.present != 1 || region->object.idx >= WORLD_OBJECT_COUNT)
            {
                DIAGNOSTIC(fprintf(stderr, "Invalid rotated region object %u present %u stage %u tick %u\n", region->object.idx, (uint32)region->object.present, game_session.stage, game_session.tick));
                diagnostic_stop(__FUNCTION__, __FILE__, __LINE__);
            }
            const MODEL_OBJECT *model = world_objects.object + region->object.idx;
            uint32 angle = model->angles.yaw_bits >> 5;
            uint32 dx = (uint32)(x - model->pos.x.whole);
            sint32 c = sin_table[MATH_QUARTER_TURN + angle];
            uint32 dz = (uint32)(z - model->pos.z.whole);
            sint32 s = sin_table[angle];
            sint32 width = region->geometry.box.half_x, depth;
            sint32 rx = (sint32)(dx * (uint32)c + dz * (uint32)s) >> 15;
            sint32 rz = (sint32)(dz * (uint32)c - dx * (uint32)s) >> 15;
            col_regions.local_x = (uint32)rx;
            col_regions.local_z = (uint32)rz;
            /* Both upper comparisons execute in lower-bound branch delay slots. */
            result = rx < width;
            if (rx <= -width || !result)
                continue;
            depth = region->geometry.box.half_z;
            result = rz < depth;
            if (rz <= -depth || !result)
                continue;
        }
        else if (shape == 2)
        {
            sint32 center_x = region->geometry.circle.center_x;
            sint32 center_z = region->geometry.circle.center_z;
            sint32 distance = math_distance_approx(x, z, center_x, center_z);
            sint32 radius = region->geometry.circle.radius;
            result = distance < radius;
            if (!result)
                continue;
        }
        else
            continue;
        result = call_trigger(*slot, region, idx);
    }
    return result;
}

GDB_CALL sint32 regions_apply_radial_hit(const FIXED_VEC3 *pos, sint32 radius, uint32 retained_word)
{
    FUNCTION_MARKER(0x8002ba88u, "GAME.EXE");
    COLLISION_REGION *region = col_regions.data;
    COLLISION_POINT point;
    sint32 i, type, x, z, px, pz, count;
    /* TODO: Investigate an absent radial impact position */
    if (!pos)
    {
        DIAGNOSTIC(fprintf(stderr, "Missing radial region position radius %d stage %u tick %u\n", radius, game_session.stage, game_session.tick));
        diagnostic_stop(__FUNCTION__, __FILE__, __LINE__);
    }
    point.pos.bits[0] = pos->x.bits;
    point.pos.bits[1] = pos->y.bits;
    point.pos.bits[2] = pos->z.bits;
    point.impact_sound = retained_word;
    count = (sint32)col_regions.count;
    if (count <= 0)
        return count;
    px = point.pos.x >> 16;
    pz = point.pos.z >> 16;
    for (i = 0; i < (sint32)col_regions.count; i++, ++region)
    {
        if (region == col_regions.data + COLLISION_REGION_COUNT)
            diagnostic_stop(__FUNCTION__, __FILE__, __LINE__);
        type = region->col_kind;
        if ((type < 3 && type != 1) || (uint32)(uint16)type - 23u < 2u || region->hits < 0)
            continue;
        if (region->shape)
        {
            /* Non-axis shapes share the retained center coordinates */
            x = region->geometry.circle.center_x;
            z = region->geometry.circle.center_z;
        }
        else
        {
            x = (region->geometry.axis.min_x + region->geometry.axis.max_x) >> 1;
            z = (region->geometry.axis.min_z + region->geometry.axis.max_z) >> 1;
        }
        if (math_distance_approx(px, pz, x, z) >= radius)
            continue;
        region->hits = -1;
        sint32 handler_idx = region->col_kind;
        if (handler_idx < 0 || handler_idx >= COLLISION_SLOT_COUNT)
            diagnostic_stop(__FUNCTION__, __FILE__, __LINE__);
        col_call(col_callbacks[handler_idx], region, (region->object.present == 1 && region->object.idx < WORLD_OBJECT_COUNT ? world_objects.object + region->object.idx : NULL), &point, -1);
    }
    return 0;
}

GDB_CALL void regions_disable_object(const MODEL_OBJECT *object)
{
    FUNCTION_MARKER(0x800292a4u, "GAME.EXE");
    COLLISION_REGION *entry = col_regions.data;
    sint32 count = (sint32)col_regions.count, i = 0;
    while (i < count)
    {
        /* TODO: Investigate a region scan beyond the native physical pool */
        if (i >= COLLISION_REGION_COUNT)
        {
            DIAGNOSTIC(fprintf(stderr, "Invalid disable-region scan slot %d count %d kind %u stage %u tick %u\n", i, count, object ? object->kind : 0xffffffffu, game_session.stage, game_session.tick));
            diagnostic_stop(__FUNCTION__, __FILE__, __LINE__);
        }
        WORLD_OBJECT_REFERENCE ref = entry->object;
        /* TODO: Investigate a region with an invalid native world reference */
        if (ref.present > 1 || (ref.present && ref.idx >= WORLD_OBJECT_COUNT))
        {
            DIAGNOSTIC(fprintf(stderr, "Invalid disable-region owner slot %d world %u present %u count %d stage %u tick %u\n", i, ref.idx, (uint32)ref.present, count, game_session.stage, game_session.tick));
            diagnostic_stop(__FUNCTION__, __FILE__, __LINE__);
        }
        if ((ref.present ? world_objects.object + ref.idx : NULL) == object)
            break;
        ++i;
        ++entry;
    }
    /* TODO: Investigate the original next-entry clear after an owner search miss */
    if (i >= count || i >= COLLISION_REGION_COUNT)
    {
        DIAGNOSTIC(fprintf(stderr, "Missing disable-region owner kind %u count %d slot %d stage %u tick %u\n", object ? object->kind : 0xffffffffu, count, i, game_session.stage, game_session.tick));
        diagnostic_stop(__FUNCTION__, __FILE__, __LINE__);
    }
    entry->col_kind = 0;
}

GDB_CALL sint32 regions_find_projectile_obstacle(uint32 world_x, uint32 world_z)
{
    FUNCTION_MARKER(0x800199acu, "GAME.EXE");
    sint32 x = (sint32)world_x >> 16, z = (sint32)world_z >> 16, i, shape, lx, lz;
    COLLISION_REGION *p = col_regions.data;
    uint32 angle, dx, dz, s, c;
    for (i = 0; i < (sint32)col_regions.count; i++, ++p)
    {
        if (p == col_regions.data + COLLISION_REGION_COUNT)
            diagnostic_stop(__FUNCTION__, __FILE__, __LINE__);
        if (p->col_kind < 2)
            continue;
        shape = p->shape;
        if (!shape)
        {
            if (p->geometry.axis.min_x < x && x < p->geometry.axis.max_x && p->geometry.axis.min_z < z && z < p->geometry.axis.max_z)
                return i;
        }
        else if (shape == 1)
        {
            /* TODO: Investigate an oriented projectile region without a valid owner */
            if (p->object.present != 1 || p->object.idx >= WORLD_OBJECT_COUNT)
            {
                DIAGNOSTIC(fprintf(stderr, "Invalid projectile query region %d world %u present %u stage %u tick %u\n", i, p->object.idx, (uint32)p->object.present, game_session.stage, game_session.tick));
                diagnostic_stop(__FUNCTION__, __FILE__, __LINE__);
            }
            const MODEL_OBJECT *model = world_objects.object + p->object.idx;
            angle = 2u * ((uint16)model->angles.yaw >> 5);
            dx = (uint32)x - (uint32)(sint32)model->pos.x.whole;
            dz = (uint32)z - (uint32)(sint32)model->pos.z.whole;
            s = (uint32)(sint32)sin_table[angle >> 1];
            c = (uint32)(sint32)sin_table[MATH_QUARTER_TURN + (angle >> 1)];
            lx = (sint32)(dx * c + dz * s) >> 15;
            lz = (sint32)(dz * c - dx * s) >> 15;
            col_regions.local_x = (uint32)lx;
            col_regions.local_z = (uint32)lz;
            if (-p->geometry.box.half_x < lx && lx < p->geometry.box.half_x && -p->geometry.box.half_z < lz && lz < p->geometry.box.half_z)
                return i;
        }
        else if ((sint32)math_distance_approx(x, z, p->geometry.circle.center_x, p->geometry.circle.center_z) < p->geometry.circle.radius)
            return i;
    }
    return -1;
}

GDB_CALL sint32 regions_query_alternate_attack(uint32 x, uint32 z)
{
    FUNCTION_MARKER(0x80023fd0u, "GAME.EXE");
    if (!regions_find_obstacle(x, z))
        return 0;
    if (!col_regions.sel)
        diagnostic_stop(__FUNCTION__, __FILE__, __LINE__);
    return col_regions.sel->alternate_attack;
}
