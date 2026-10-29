#include <string.h>
#include <stdlib.h>
#include <stdio.h>

#include "game.h"

ITEM_CORRECTION item_corrections[DYNAMIC_ITEM_TYPE_COUNT];

/* Original trail mesh operations retain the pad halfword of every vertex */
GDB_CALL sint32 character_trail_clear(sint32 resource)
{
    FUNCTION_MARKER(0x80057ca4, "GAME.EXE");
    MODEL_REFERENCE model;
    uint32 i;
    SVECTOR *vertices;
    XASSERT(resource >= 0 && resource < MODEL_TYPE_COUNT);
    model = model_refs[resource];
    XASSERT(model.header && model.byte_len >= sizeof(*model.header));
    /* TODO: Investigate an original trail mesh shorter than its fixed 18 vertices */
    XASSERT(model.vertices && !((uintptr_t)model.vertices & 1) && model.vertex_byte_len >= 18 * sizeof(*vertices));
    vertices = model.vertices;
    /* The original clears exactly 18 vertices, retaining each pad halfword */
    for (i = 0; i < 18; i++)
    {
        vertices[i].vx = 0;
        vertices[i].vy = 0;
        vertices[i].vz = 0;
    }
    return 0;
}

GDB_CALL uint32 character_trail_append(sint32 idx, sint32 attachment)
{
    FUNCTION_MARKER(0x80057ce4, "GAME.EXE");
    ACTOR_TRAIL *trail = actor_trails + idx;
    MODEL_REFERENCE model;
    SVECTOR *vertices;
    uint32 angle;
    FIXED_VEC3 point;
    sint32 mode = trail->mode, x, y, z, s, c, height, age;
    SVECTOR v[2];
    int i;
    if (mode == 1 || mode == 3)
        actor_attachment_pos(idx, attachment, point.bits);
    else
        actor_items_calc_weapon_point(idx, 0, point.bits);
    x = (sint32)(point.x.bits - actor_positions[idx].x.bits) >> 16;
    y = (sint32)(point.y.bits - actor_positions[idx].y.bits) >> 16;
    z = (sint32)(point.z.bits - actor_positions[idx].z.bits) >> 16;
    if (mode == 1)
    {
        angle = ((uint32)math_dir16(0, 0, x, z) >> 4) & 4094u;
        s = sin_table[angle >> 1];
        c = sin_table[MATH_QUARTER_TURN + (angle >> 1)];
        x += s >> 9;
        z -= c >> 9;
        v[0].vx = (uint16)x;
        v[0].vy = (uint16)y;
        v[0].vz = (uint16)z;
        v[1].vx = (uint16)(x - ((3 * s) >> 10));
        v[1].vy = (uint16)y;
        v[1].vz = (uint16)(z + ((3 * c) >> 10));
    }
    else
    {
        v[0].vx = (uint16)x;
        v[0].vy = (uint16)y;
        v[0].vz = (uint16)z;
        height = (sint16)(model_height[(uint32)actor_types[idx]] >> 16);
        v[1].vx = (uint16)((x * 5u) >> 3);
        v[1].vy = (uint16)(((sint32)((y - (uint32)height) * 5u) >> 3) + height);
        v[1].vz = (uint16)((z * 5u) >> 3);
    }
    model = model_refs[867 + (idx & 1u)];
    /* TODO: Investigate an original trail mesh shorter than its fixed 18 vertices */
    XASSERT(model.header && model.byte_len >= sizeof(*model.header) && model.vertices && !((uintptr_t)model.vertices & 1) && model.vertex_byte_len >= 18 * sizeof(*vertices));
    vertices = model.vertices;
    if ((sint16)trail->phase == 1)
    {
        for (i = 0; i < 9; i++)
        {
            vertices[2 * i].vx = v[0].vx;
            vertices[2 * i].vy = v[0].vy;
            vertices[2 * i].vz = v[0].vz;
            vertices[2 * i + 1].vx = v[1].vx;
            vertices[2 * i + 1].vy = v[1].vy;
            vertices[2 * i + 1].vz = v[1].vz;
        }
    }
    else
    {
        for (i = 0; i < 16; i++)
        {
            vertices[i].vx = vertices[i + 2].vx;
            vertices[i].vy = vertices[i + 2].vy;
            vertices[i].vz = vertices[i + 2].vz;
        }
        vertices[16].vx = v[0].vx;
        vertices[16].vy = v[0].vy;
        vertices[16].vz = v[0].vz;
        vertices[17].vx = v[1].vx;
        vertices[17].vy = v[1].vy;
        vertices[17].vz = v[1].vz;
    }
    age = ((sint16)trail->phase) >> 2;
    if (age >= 8)
    {
        trail->mode = 0;
        trail->phase = 0;
    }
    sint64 pal_offset = 0x6a0 + 16LL * idx;
    XASSERT(pal_offset >= 0 && pal_offset <= (sint64)sizeof(tex_pals) - 32);
    sint64 fade_offset = 32LL * age + 16 * ((uint16)actor_types[idx] & 1u);
    uint8 fade_bytes[32];
    size_t copied = 0, part;
    /* TODO: Investigate a trail palette source before the native tile span */
    XASSERT(fade_offset >= -(sint64)sizeof(terrain_map.tile_flat));
    if (copied < sizeof(fade_bytes) && fade_offset < 0)
    {
        part = (size_t)-fade_offset;
        if (part > sizeof(fade_bytes) - copied)
            part = sizeof(fade_bytes) - copied;
        memcpy(fade_bytes + copied, (const uint8 *)terrain_map.tile_flat + sizeof(terrain_map.tile_flat) + fade_offset, part);
        copied += part;
        fade_offset += part;
    }
    if (copied < sizeof(fade_bytes) && fade_offset < (sint64)sizeof(tex_effect_pals.fade))
    {
        part = sizeof(tex_effect_pals.fade) - (size_t)fade_offset;
        if (part > sizeof(fade_bytes) - copied)
            part = sizeof(fade_bytes) - copied;
        memcpy(fade_bytes + copied, (const uint8 *)&tex_effect_pals.fade + fade_offset, part);
        copied += part;
        fade_offset += part;
    }
    sint64 image_offset = fade_offset - sizeof(tex_effect_pals.fade);
    if (copied < sizeof(fade_bytes) && image_offset >= 0 && image_offset < sizeof(background_img.pix))
    {
        part = sizeof(background_img.pix) - (size_t)image_offset;
        if (part > sizeof(fade_bytes) - copied)
            part = sizeof(fade_bytes) - copied;
        memcpy(fade_bytes + copied, background_img.pix + image_offset, part);
        copied += part;
        fade_offset += part;
    }
    sint64 color_offset = fade_offset - sizeof(tex_effect_pals.fade) - sizeof(background_img.pix);
    if (copied < sizeof(fade_bytes) && color_offset >= 0 && color_offset < sizeof(terrain_map.color_flat))
    {
        part = sizeof(terrain_map.color_flat) - (size_t)color_offset;
        if (part > sizeof(fade_bytes) - copied)
            part = sizeof(fade_bytes) - copied;
        memcpy(fade_bytes + copied, terrain_map.color_flat + color_offset, part);
        copied += part;
        fade_offset += part;
    }
    /* TODO: Investigate a trail palette source after the native color span */
    XASSERT(copied >= sizeof(fade_bytes));
    return resource_copy_words((uint8 *)&tex_pals + pal_offset, fade_bytes, 16);
}

/* 5C740: object-local transform, with wrapped 32-bit camera subtraction. */

/* Retain the existing platform/debug symbol while routing the complete body. */

static uint32 actor_color(ACTOR_LIGHTING *light, int player)
{
    sint32 r, g, b;
    uint8 flags;
    uint32 color;
    r = 128 + light->r;
    g = 128 + light->g;
    b = 128 + light->b;
    render_style.color.rgbc = 0x2c000000;
    flags = light->flags;
    if (flags & 2)
    {
        render_style.color.rgbc = 0x2e000000;
        render_style.blend_mode = light->flags >> 4;
    }
    flags = light->flags;
    if (flags & 4)
    {
        r += player ? 96 : 128;
        g += player ? 96 : 128;
        b += player ? 96 : 128;
        if (r >= 256)
            r = 255;
        if (g >= 256)
            g = 255;
        if (b >= 256)
            b = 255;
        light->flags &= 251;
    }
    color = render_style.color.rgbc | (uint32)r | ((uint32)g << 8) | ((uint32)b << 16);
    render_style.color.rgbc = color;
    return color;
}

/* 58D9C: item correction rotations in Z/Y/X order, then signed fixed translation. */
GDB_CALL void item_apply_correction(const ITEM_CORRECTION *correction)
{
    FUNCTION_MARKER(0x80058d9c, "GAME.EXE");
    sint32 angle = correction->roll;
    uint32 x, y, z;
    if (angle)
        transform_roll(angle);
    angle = correction->yaw;
    if (angle)
        transform_yaw(angle);
    angle = correction->pitch;
    if (angle)
        transform_pitch(angle);
    x = (uint32)(sint32)correction->x << 16;
    y = (uint32)(sint32)correction->y << 16;
    z = (uint32)(sint32)correction->z << 16;
    transform_translate(x, y, z);
}

/* Player right-hand path59E78..5A05C retains character-specific corrections */
static void player_right_item(uint32 idx, uint32 color)
{
    sint32 slot, type;
    /* TODO: Investigate render actors outside the native equipment domain */
    XASSERT(idx < ACTOR_COUNT);
    slot = actor_items[idx].slot[ACTOR_ITEM_RIGHT];
    uint32 kind;
    if (slot == -1)
        return;
    /* TODO: Investigate held render references outside the dynamic pool */
    XASSERT(slot >= 0 && slot < DYNAMIC_OBJECT_COUNT);
    MODEL_OBJECT *item = dynamic_objects + slot;
    kind = item->kind;
    if (kind == 0xffffffffu)
    {
        actor_items[idx].slot[ACTOR_ITEM_RIGHT] = -1;
        return;
    }
    transform_load(&actor_poses[idx].matrix[1]);
    transform_translate(0, -786432, 0);
    transform_yaw(6144);
    const FIXED_VEC3 *actor_pos = actor_positions + idx;
    type = actor_types[idx];
    if (type == 1)
    {
        if (kind == 506)
        {
            transform_translate(0, 0, -786432);
            transform_pitch(-1536);
            transform_yaw(-2048);
        }
        else
        {
            transform_yaw(61440);
            transform_roll(4096);
        }
        type = actor_types[idx];
    }
    if (type == 2)
    {
        if (kind == 506)
            transform_pitch(-2560);
        else
        {
            if (kind == 515)
            {
                transform_translate(0, 0, 131072);
                transform_pitch(64512);
                transform_yaw(10240);
            }
            else
                transform_yaw(61440);
            transform_roll(64512);
        }
    }
    if (actor_types[idx] == 3)
    {
        if (kind == 506)
        {
            transform_translate(0, 0, -786432);
            transform_yaw(-3072);
            transform_pitch(-1024);
        }
        else
        {
            transform_yaw(61440);
            transform_roll(4096);
        }
    }
    item->pos.x.bits = actor_pos->x.bits;
    item->pos.z.bits = actor_pos->z.bits;
    /* TODO: Investigate a held kind outside its correction table at the original read */
    XASSERT(kind - DYNAMIC_ITEM_TYPE_FIRST < DYNAMIC_ITEM_TYPE_COUNT);
    item_apply_correction(item_corrections + (kind - DYNAMIC_ITEM_TYPE_FIRST));
    render_style.color.rgbc = color;
    model_draw_static(model_refs[kind], 0, 0);
    if (kind - 496u < 2u)
    {
        /* TODO: Investigate a held explosive with a foreign native owner */
        XASSERT(item->payload_kind == MODEL_PAYLOAD_PHYSICS);
        if (item->physics.life > 0)
            item_draw_explosive_countdown(item);
    }
}

/* NPC right hand and both left hands share the unadjusted bone transform. */
static void actor_other_item(uint32 idx, uint32 color, int left, int player)
{
    ACTOR_ITEM_HAND hand = left ? ACTOR_ITEM_LEFT : ACTOR_ITEM_RIGHT;
    uint32 kind;
    sint32 slot, type;
    /* TODO: Investigate render actors outside the native equipment domain */
    XASSERT(idx < ACTOR_COUNT);
    slot = actor_items[idx].slot[hand];
    if (slot == -1)
        return;
    /* TODO: Investigate held render references outside the dynamic pool */
    XASSERT(slot >= 0 && slot < DYNAMIC_OBJECT_COUNT);
    MODEL_OBJECT *item = dynamic_objects + slot;
    kind = item->kind;
    if (kind == 0xffffffffu)
    {
        actor_items[idx].slot[hand] = -1;
        return;
    }
    transform_load(&actor_poses[idx].matrix[left ? 0 : 1]);
    transform_translate(0, -786432, 0);
    transform_yaw(6144);
    const FIXED_VEC3 *actor_pos = actor_positions + idx;
    if (player && left)
    {
        type = actor_types[idx];
        if (type == 1 && kind == 515)
        {
            transform_translate(-524288, -524288, -1310720);
            transform_pitch(59392);
            transform_yaw(5632);
            transform_roll(7936);
            type = actor_types[idx];
        }
        if (type == 3 && kind == 515)
        {
            transform_translate(-524288, -524288, -1310720);
            transform_pitch(59392);
            transform_yaw(5632);
            transform_roll(7936);
        }
    }
    item->pos.x.bits = actor_pos->x.bits;
    item->pos.z.bits = actor_pos->z.bits;
    /* TODO: Investigate a held kind outside its correction table at the original read */
    XASSERT(kind - DYNAMIC_ITEM_TYPE_FIRST < DYNAMIC_ITEM_TYPE_COUNT);
    item_apply_correction(item_corrections + (kind - DYNAMIC_ITEM_TYPE_FIRST));
    render_style.color.rgbc = color;
    model_draw_static(model_refs[kind], 0, 0);
    if (!left && kind - 496u < 2)
    {
        /* TODO: Investigate a held explosive with a foreign native owner */
        XASSERT(item->payload_kind == MODEL_PAYLOAD_PHYSICS);
        if (item->physics.life > 0)
            item_draw_explosive_countdown(item);
    }
}

static void actor_trail(uint32 idx)
{
    ACTOR_LIGHTING *light = actor_lighting + idx;
    uint32 color = render_style.color.rgbc;
    render_style.color.rgbc = color | 0x02000000;
    render_style.blend_mode = 4;
    model_draw_pose(model_refs[(uint32)actor_types[idx]], &actor_poses[ACTOR_POSE_TRAIL1]);
    if (!(sint8)game_pause_flags && !(game_session.tick & 1))
        actor_pose_copy(&actor_poses[ACTOR_POSE_TRAIL1], &actor_poses[ACTOR_POSE_TRAIL0]);
    render_style.color.rgbc = color | 0x02000000;
    render_style.blend_mode = 1;
    model_draw_pose(model_refs[(uint32)actor_types[idx]], &actor_poses[ACTOR_POSE_TRAIL0]);
    if (!(sint8)game_pause_flags)
    {
        if (!(game_session.tick & 1))
            actor_pose_copy(&actor_poses[ACTOR_POSE_TRAIL0], &actor_poses[idx]);
        if (!(sint8)game_pause_flags)
            light->flags &= 247;
    }
    render_style.color.rgbc = color;
}

static void actor_van(uint32 idx)
{
    const FIXED_VEC3 *pos = actor_positions + idx;
    const ACTOR_ORIENTATION *orientation = actor_orientation + idx;
    uint32 x, y, z, camera_x, camera_y, camera_z, height;
    transform_push();
    x = pos->x.bits;
    camera_x = camera.pos.x.bits;
    y = pos->y.bits;
    height = model_height[17];
    camera_y = camera.pos.y.bits;
    z = pos->z.bits;
    camera_z = camera.pos.z.bits;
    transform_translate(x - camera_x, y + height - camera_y, z - camera_z);
    transform_pitch(32768);
    transform_yaw((sint16)((uint32)(sint32)orientation->yaw << 5));
    model_draw_static(model_refs[65], 0, 0);
    transform_push();
    transform_roll(orientation->roll);
    transform_translate(0, 4194304, 0);
    model_draw_static(model_refs[17], 0, 0);
    transform_pop();
    model_draw_static(model_refs[68], 0, 4096);
    transform_translate(0, 0, 30408704);
    transform_yaw(16384);
    model_draw_pulsing_marker();
    transform_pop();
}

static void actor_body(uint32 idx, sint32 type, int player)
{
    ACTOR_LIGHTING *light;
    ACTOR_STATUS *status = actor_status + idx;
    const FIXED_VEC3 *pos = actor_positions + idx;
    const ACTOR_ORIENTATION *orientation = actor_orientation + idx;
    ACTOR_FRAME_MOTION *motion;
    uint32 color, x, y, z, camera_x, camera_y, camera_z, height;
    sint32 yaw, scale;
    transform_push();
    x = pos->x.bits;
    camera_x = camera.pos.x.bits;
    y = pos->y.bits;
    height = model_height[(uint32)type];
    camera_y = camera.pos.y.bits;
    z = pos->z.bits;
    camera_z = camera.pos.z.bits;
    transform_translate(x - camera_x, y + height - camera_y, z - camera_z);
    if ((!player && type == 51) || transform_origin_visible())
    {
        status->flags = status->flags & ~ACTOR_CULLED;
        XASSERT(idx < ACTOR_COUNT);
        light = actor_lighting + idx;
        motion = &actor_animations[idx].motion;
        color = actor_color(light, player);
        if (!player && type == 51)
            transform_yaw(32768);
        yaw = (sint16)((uint32)(-(sint64)((uint32)(sint32)orientation->yaw << 5))) + 32768;
        transform_yaw(yaw);
        y = motion->bits[0];
        z = motion->forward_offset_bits;
        transform_translate(0, y, z);
        if (!player && type == 14 && (light->flags & 8))
            actor_trail(idx);
        model_draw_pose(model_refs[(uint32)actor_types[idx]], &actor_poses[idx]);
        if (player)
            player_right_item(idx, color);
        else
            actor_other_item(idx, color, 0, 0);
        actor_other_item(idx, color, 1, player);
        transform_pop();
        transform_push();
        x = pos->x.bits;
        camera_x = camera.pos.x.bits;
        camera_y = camera.pos.y.bits;
        z = pos->z.bits;
        camera_z = camera.pos.z.bits;
        transform_translate(x - camera_x, (uint32)(-(sint64)(camera_y)), z - camera_z);
        yaw = (sint16)((uint32)(-(sint64)((uint32)(sint32)orientation->yaw << 5))) + 32768;
        transform_yaw(yaw);
        if (player)
            model_draw_static(model_refs[860], 0, 0);
        else
        {
            if (type == 28 || type == 51)
                transform_yaw(16384);
            y = pos->y.bits;
            height = motion->bits[0];
            scale = (uint32)((sint32)(y + height) >> 12) + (type == 50 ? 24576 : 16384);
            if (scale > 0)
            {
                transform_scale(scale, scale, scale);
                model_draw_static(model_refs[860], 0, 0);
            }
        }
    }
    else
        status->flags = player ? (status->flags | ACTOR_CULLED) : 2u;
    transform_pop();
}

/* 593FC: character, held-item, trail, van and shadow rendering. */
GDB_CALL sint32 characters_draw(void)
{
    FUNCTION_MARKER(0x800593fc, "GAME.EXE");
    sint32 i, type, count;
    uint32 x, y, z, camera_x, camera_y, camera_z, frame;
    MODEL_REFERENCE model;
    transform_reset();
    level_transition_continue_prompt();
    for (i = 2; i < 8; i++)
    {
        type = actor_types[i];
        if (type == -1 || (((sint8)actor_lifecycle[i].death_ticks) & 2))
            continue;
        if (type == 17)
        {
            actor_van(i);
            continue;
        }
        actor_body(i, type, 0);
    }
    count = game_session.player_count;
    if (count <= 0)
        return count;
    i = 0;
    do
    {
        if (level_transition.route_exit_tick == 2)
            return 2;
        /* TODO: Investigate renderer actor counts beyond the native pool */
        XASSERT(i >= 0 && i < ACTOR_COUNT);
        type = actor_types[i];
        if (type != -1 && !(actor_lifecycle[i].death_ticks & 2))
        {
            if ((sint16)actor_trails[i].phase)
            {
                transform_push();
                x = actor_positions[i].x.bits;
                camera_x = camera.pos.x.bits;
                y = actor_positions[i].y.bits;
                camera_y = camera.pos.y.bits;
                z = actor_positions[i].z.bits;
                camera_z = camera.pos.z.bits;
                transform_translate(x - camera_x, y - camera_y, z - camera_z);
                model = model_refs[867 + (i & 1u)];
                frame = actor_animations[i].selector;
                model_draw_static(model, frame, 0);
                transform_pop();
            }
            actor_body(i, type, 1);
        }
        ++i;
        count = game_session.player_count;
    } while (i < count);
    return 0;
}

/* 5D67C: settled debris local transform and kind-dependent origin bias. */
GDB_CALL void object_draw_settled_debris(const MODEL_OBJECT *item)
{
    FUNCTION_MARKER(0x8005d67cu, "GAME.EXE");
    uint32 kind;
    transform_push();
    object_transform_camera(item, NULL);
    kind = item->kind;
    transform_translate(0, 0, kind == 529 ? -3932160 : kind == 530 ? -3670016 : -3145728);
    model_draw_static(model_refs[item->kind], 0, 0);
    transform_pop();
}

/* 5B178: base and two hinged panels, caller owns the outer transform. */
