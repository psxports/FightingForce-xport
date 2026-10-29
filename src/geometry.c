#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "game.h"

RENDER_STYLE render_style;

GDB_CALL sint32 packet_color_half(const uint16 packet[9])
{
    FUNCTION_MARKER(0x80064638u, "GAME.EXE");
    /* Original color-split callee64620 leaves the return carrier unchanged */
    return (sint16)packet[8] >> 1;
}

static sint32 lightning_jitter(sint32 divisor)
{
    FUNCTION_MARKER(0x80057700u, "GAME.EXE");
    uint32 random;
    sint32 val;
    if (divisor < 0)
        divisor = (uint32)(-(sint64)(divisor));
    random = random_next();
    val = (sint32)random >> 1;
    val = val + (uint32)(val >> 1);
    XASSERT(divisor && (divisor != -1 || (uint32)val != 0x80000000u));
    val /= divisor;
    return random & 1u ? (sint32)((uint32)(-(sint64)((uint32)val))) : val;
}

GDB_CALL uint32 lightning_draw_strip(const FIXED_VEC3 *src, const FIXED_VEC3 *dst)
{
    FUNCTION_MARKER(0x80057788u, "GAME.EXE");
    FIXED_VEC3 start, end, step, cur, point;
    uint32 camera_x, camera_y, camera_z;
    PROJECTION_POINT prev, proj;
    uint16 packet[9];
    sint32 count;
    sint32 idx;
    sint32 depth;
    sint32 old_depth;
    uint32 width;
    uint32 new_width;
    uint32 delta;
    start.x = src->x;
    start.y = src->y;
    start.z = src->z;
    end.x = dst->x;
    end.y = dst->y;
    end.z = dst->z;
    cur = start;
    count = math_distance_approx(start.x.val >> 16, start.z.val >> 16, end.x.val >> 16, end.z.val >> 16) >> 8;
    if (count < 3)
        count = 3;
    step.x.bits = (uint32)((sint32)(end.x.bits - start.x.bits) / count) << 1;
    step.y.bits = (uint32)((sint32)(end.y.bits - start.y.bits) / count) << 1;
    step.z.bits = (uint32)((sint32)(end.z.bits - start.z.bits) / count) << 1;
    transform_push();
    camera_x = camera.pos.x.bits;
    camera_y = camera.pos.y.bits;
    camera_z = camera.pos.z.bits;
    transform_translate(start.x.bits - camera_x, start.y.bits - camera_y, start.z.bits - camera_z);
    proj_local(0, 0, 0, &prev);
    transform_pop();
    for (idx = 1; idx <= count; ++idx)
    {
        cur.x.bits += step.x.bits;
        cur.y.bits += step.y.bits;
        cur.z.bits += step.z.bits;
        if (idx == count)
            point = end;
        else
        {
            point.x.bits = cur.x.bits + (((random_next() & 127) - 64u) << 17);
            point.y.bits = cur.y.bits + (((random_next() & 127) - 64u) << 17);
            point.z.bits = cur.z.bits + (((random_next() & 127) - 64u) << 17);
        }
        transform_push();
        camera_x = camera.pos.x.bits;
        camera_y = camera.pos.y.bits;
        camera_z = camera.pos.z.bits;
        transform_translate(point.x.bits - camera_x, point.y.bits - camera_y, point.z.bits - camera_z);
        proj_local(0, 0, 0, &proj);
        transform_pop();
        if (prev.status < 4 && proj.status < 4)
        {
            old_depth = prev.depth;
            depth = proj.depth;
            XASSERT(old_depth && depth); /* Original57A2C/57A60 BREAK */
            width = 65536 / old_depth;
            new_width = 65536 / depth;
            packet[2] = (uint16)prev.x;
            packet[3] = (uint16)((uint16)prev.y - width);
            packet[4] = (uint16)proj.x;
            packet[5] = (uint16)((uint16)proj.y - new_width);
            packet[6] = (uint16)proj.x;
            packet[7] = (uint16)((uint16)proj.y + new_width);
            packet[0] = (uint16)prev.x;
            packet[1] = (uint16)((uint16)prev.y + width);
            if (idx != 1 && idx != count)
                random_next(); /* Unused result is still an RNG advance */
            packet[8] = 831;
            depth = proj.depth >> 2;
            packet[0] = (uint16)proj.x;
            packet[1] = (uint16)proj.y;
            delta = lightning_jitter(depth);
            packet[2] = (uint16)(packet[0] + delta);
            delta = lightning_jitter(depth);
            packet[3] = (uint16)(packet[1] + delta);
            packet[8] = 1373;
            packet_color_half(packet);
            delta = lightning_jitter(depth);
            packet[0] = (uint16)(packet[2] + delta);
            delta = lightning_jitter(depth);
            packet[1] = (uint16)(packet[3] + delta);
            packet[8] = 827;
            packet_color_half(packet);
            delta = lightning_jitter(depth);
            packet[2] = (uint16)(packet[0] + delta);
            delta = lightning_jitter(depth);
            packet[3] = (uint16)(packet[1] + delta);
            packet[8] = 1647;
            packet_color_half(packet);
            delta = lightning_jitter(depth);
            packet[0] = (uint16)(packet[2] + delta);
            delta = lightning_jitter(depth);
            packet[1] = (uint16)(packet[3] + delta);
            packet[8] = 553;
            packet_color_half(packet);
        }
        prev = proj;
    }
    return 1;
}

GDB_CALL uint32 object_draw_scene_gated(const MODEL_OBJECT *model)
{
    FUNCTION_MARKER(0x80046ef0u, "GAME.EXE");
    /* TODO: Investigate rotation common fields outside both native object pools */
    XASSERT(model);
    uint32 kind = model->kind, bias, state;
    sint32 angle = model->angles.yaw, x, z;
    if (angle)
        transform_yaw(angle);
    angle = model->angles.roll;
    if (angle)
        transform_roll(angle);
    angle = model->angles.pitch;
    if (angle)
        transform_pitch(angle);
    bias = kind - 207u < 4u || kind == 214 || kind == 211 ? 512 : 0;
    state = game_session.scene_phase;
    if (!state)
    {
        /* TODO: Investigate object drawing without a native position */
        XASSERT(model);
        z = model->pos.z.val;
        if (z <= (sint32)0xf2900000)
            goto draw_model;
        state = game_session.scene_phase;
        if (!state)
        {
            if (stage_animation.phase[0])
                goto test_front;
            state = game_session.scene_phase;
        }
    }
    if (state - 1 < 2)
    {
        /* TODO: Investigate object drawing without a native position */
        XASSERT(model);
        z = model->pos.z.val;
        goto test_front;
    }
    goto select_middle;

test_front:
    if (z > (sint32)0xf2900000)
    {
        /* TODO: Investigate object drawing without a native position */
        XASSERT(model);
        x = model->pos.x.val;
        if (x > 0x06000000)
            goto draw_model;
    }
select_middle:
    state = game_session.scene_phase;
    if ((state == 2 && stage_animation.phase[0]) || state - 3 < 2 || state == 5)
    {
        /* TODO: Investigate object drawing without a native position */
        XASSERT(model);
        x = model->pos.x.val;
        if (x + 0x0dffffffu <= 0x13fffffe)
            goto draw_model;
    }
    state = game_session.scene_phase;
    if (state == 4 && stage_animation.phase[0])
        goto test_left;
    state = game_session.scene_phase;
    if (state - 5 >= 2)
        return 0;
test_left:
    /* TODO: Investigate object drawing without a native position */
    XASSERT(model);
    x = model->pos.x.val;
    if (x >= (sint32)0xf2000000)
        return 0;

draw_model:
    return model_draw_static(model_refs[model->kind], 0, bias);
}

GDB_CALL void object_draw_depth_bob(const MODEL_OBJECT *model)
{
    FUNCTION_MARKER(0x8002c55cu, "GAME.EXE");
    uint32 x, y, z, color, offset, camera_x, camera_y, camera_z;
    sint32 shade;
    uint32 wave;
    sint32 angle;
    transform_push();
    /* TODO: Investigate a compound draw without a native position */
    XASSERT(model);
    y = model->pos.y.bits;
    /* TODO: Investigate a falling debris callback with a foreign native payload */
    XASSERT(model && model->payload_kind == MODEL_PAYLOAD_DRIFT);
    const DRIFTING_DEBRIS *debris = &model->drift;
    offset = ((uint32)(sint32)debris->phase << 3) & 0xff8;
    wave = sin_table[MATH_QUARTER_TURN + (offset >> 1)];
    z = model->pos.z.bits - (wave << 9);
    x = model->pos.x.bits;
    camera_x = camera.pos.x.bits;
    camera_y = camera.pos.y.bits;
    camera_z = camera.pos.z.bits;
    transform_translate(x - camera_x, y - camera_y, z - camera_z);
    transform_pitch(32768);
    angle = model->angles.pitch;
    if (angle)
        transform_pitch(angle);
    angle = model->angles.roll;
    if (angle)
        transform_roll(angle);
    shade = ((sint32)z >> 19) - 450;
    if (shade < 0)
        shade = 0;
    else
        shade >>= 2;
    if (shade > 128)
        shade = 128;
    color = 128u - shade;
    render_style.color.rgbc = 0x2c000000u + color * 0x10101u;
    model_draw_static(model_refs[model->kind], 0, 0);
    transform_pop();
}

GDB_CALL void object_draw_repeated_pair(const MODEL_OBJECT *model)
{
    FUNCTION_MARKER(0x80032c4cu, "GAME.EXE");
    transform_push();
    /* TODO: Investigate phase drawing without a native position */
    XASSERT(model);
    if (model->pos.x.val > 0x13000000)
        transform_yaw(32768);
    model_draw_faded(model_refs[model->kind], 0, 0);
    transform_translate(0, 0, -0xe600000);
    model_draw_faded(model_refs[303], 0, 0);
    transform_translate(0, 0, 0x07300000);
    model_draw_faded(model_refs[304], 0, 0);
    transform_translate(0, 0, 0x0e600000);
    model_draw_faded(model_refs[303], 0, 0);
    transform_translate(0, 0, 0x07300000);
    model_draw_faded(model_refs[304], 0, 0);
    transform_pop();
}

GDB_CALL uint32 object_draw_shadowed_model(const MODEL_OBJECT *model)
{
    FUNCTION_MARKER(0x8005ad54u, "GAME.EXE");
    /* TODO: Investigate drawing common fields outside both native object pools */
    XASSERT(model);
    uint32 kind = model->kind;
    sint32 angle = model->angles.yaw;
    if (angle)
        transform_yaw(angle);
    transform_push();
    /* TODO: Investigate a shadow draw without a native position */
    XASSERT(model);
    transform_translate(0, model->pos.y.val, 0);
    model_draw_static(model_refs[(kind == 87 ? 89u : 106u)], 0, 8192);
    transform_pop();
    angle = model->angles.roll;
    if (angle)
        transform_roll(angle);
    angle = model->angles.pitch;
    if (angle)
        transform_pitch(angle);
    return model_draw_static(model_refs[model->kind], 0, 0);
}

GDB_CALL uint32 object_draw_phase_gated(const MODEL_OBJECT *model)
{
    FUNCTION_MARKER(0x80032818u, "GAME.EXE");
    /* TODO: Investigate drawing common fields outside both native object pools */
    XASSERT(model);
    uint32 kind = model->kind, state, mode;
    sint32 x, z, bias;
    object_apply_rotations(model);
    bias = (kind - 207u < 4u || kind == 214 || kind == 211) ? 512 : 0;
    state = game_session.scene_phase;
    if (!state)
    {
        /* TODO: Investigate phase drawing without a native position */
        XASSERT(model);
        z = model->pos.z.val;
        if (z <= (sint32)0xf2900000)
            goto draw_model;
        state = game_session.scene_phase;
        if (!state)
        {
            mode = stage_animation.phase[0];
            if (mode)
                goto test_front;
        }
    }
    state = game_session.scene_phase;
    if (state - 1 < 2 || state == 3)
    {
        /* TODO: Investigate phase drawing without a native position */
        XASSERT(model);
        z = model->pos.z.val;
        goto test_front;
    }
    goto select_middle;

test_front:
    if (z > (sint32)0xf2900000)
    {
        /* TODO: Investigate phase drawing without a native position */
        XASSERT(model);
        x = model->pos.x.val;
        if (x > 0x06000000)
            goto draw_model;
    }
    state = game_session.scene_phase;
select_middle:
    if (state == 2)
    {
        mode = stage_animation.phase[0];
        if (mode)
            goto test_middle;
    }
    state = game_session.scene_phase;
    if (state - 3 < 2 || state == 5)
        goto test_middle;
    goto select_left;

test_middle:
    /* TODO: Investigate phase drawing without a native position */
    XASSERT(model);
    x = model->pos.x.val;
    if (x + 0x0dffffffu <= 0x13fffffe)
        goto draw_model;
    state = game_session.scene_phase;
select_left:
    if (state == 4)
    {
        mode = stage_animation.phase[0];
        if (mode)
            goto test_left;
    }
    state = game_session.scene_phase;
    if (state - 5 >= 2)
        return 0;
test_left:
    /* TODO: Investigate phase drawing without a native position */
    XASSERT(model);
    x = model->pos.x.val;
    if (x >= (sint32)0xf2000000)
        return 0;

draw_model:
    return model_draw_static(model_refs[model->kind], 0, bias);
}

GDB_CALL uint32 object_draw_player_facing(MODEL_OBJECT *model)
{
    FUNCTION_MARKER(0x80032a48u, "GAME.EXE");
    /* TODO: Investigate phase drawing without a native position */
    XASSERT(model);
    sint32 x = model->pos.x.whole;
    sint32 z = model->pos.z.whole;
    sint32 player_x = actor_positions[0].x.whole;
    sint32 player_z = actor_positions[0].z.whole;
    sint16 angle = (sint16)math_dir16(x, z, player_x, player_z);
    model->angles.yaw_bits = angle;
    transform_yaw(angle);
    return object_draw_phase_gated(model);
}

GDB_CALL uint32 object_draw_stage_gated(const MODEL_OBJECT *item)
{
    FUNCTION_MARKER(0x80032d70u, "GAME.EXE");
    if ((sint32)stage_animation.phase[0] <= 0 && !players_past_z(-0x12000000))
        return 0;
    object_apply_rotations(item);
    /* TODO: Investigate common draw fields outside both native object pools */
    XASSERT(item);
    return model_draw_static(model_refs[item->kind], 0, 0);
}

GDB_CALL uint32 object_draw_rotated_packets(const MODEL_OBJECT *item)
{
    FUNCTION_MARKER(0x8005b71cu, "GAME.EXE");
    object_apply_rotations(item);
    /* TODO: Investigate common draw fields outside both native object pools */
    XASSERT(item);
    return model_draw_faded(model_refs[item->kind], 0, 0);
}

GDB_CALL uint32 object_draw_depth_rotated_packets(const MODEL_OBJECT *item)
{
    FUNCTION_MARKER(0x8005c0e8u, "GAME.EXE");
    transform_origin_depth();
    object_apply_rotations(item);
    /* TODO: Investigate common draw fields outside both native object pools */
    XASSERT(item);
    return model_draw_faded(model_refs[item->kind], 0, 0);
}

GDB_CALL uint32 object_draw_scene_active(const MODEL_OBJECT *item)
{
    FUNCTION_MARKER(0x800428ecu, "GAME.EXE");
    uint32 state = game_session.scene_phase;
    if ((sint32)state <= 0)
        return state;
    object_apply_rotations(item);
    /* TODO: Investigate common draw fields outside both native object pools */
    XASSERT(item);
    return model_draw_faded(model_refs[item->kind], 0, 0);
}

GDB_CALL uint32 object_draw_scene_early(const MODEL_OBJECT *item)
{
    FUNCTION_MARKER(0x80042848u, "GAME.EXE");
    if (game_session.scene_phase >= 2u)
        return 0;
    object_apply_rotations(item);
    /* TODO: Investigate common draw fields outside both native object pools */
    XASSERT(item);
    return model_draw_faded(model_refs[item->kind], 0, 0);
}

GDB_CALL uint32 object_draw_faded_fixed_model(void)
{
    FUNCTION_MARKER(0x80032d44u, "GAME.EXE");
    return model_draw_faded(model_refs[281], 0, 1024);
}

GDB_CALL uint32 object_draw_raised(const MODEL_OBJECT *item)
{
    FUNCTION_MARKER(0x8005b4f8u, "GAME.EXE");
    /* TODO: Investigate common draw fields outside both native object pools */
    XASSERT(item);
    sint32 angle = item->angles.yaw;
    if (angle)
        transform_yaw(angle);
    transform_translate(0, 0x280000, 0);
    angle = item->angles.roll;
    if (angle)
        transform_roll(angle);
    angle = item->angles.pitch;
    if (angle)
        transform_pitch(angle);
    return model_draw_static(model_refs[item->kind], 0, 0);
}

GDB_CALL uint32 object_draw_base_raised(const MODEL_OBJECT *item)
{
    FUNCTION_MARKER(0x8005b350u, "GAME.EXE");
    /* TODO: Investigate common draw fields outside both native object pools */
    XASSERT(item);
    sint32 angle = item->angles.yaw;
    if (angle)
        transform_yaw(angle);
    model_draw_static(model_refs[68], 0, 8192);
    transform_translate(0, 0x400000, 0);
    angle = item->angles.roll;
    if (angle)
        transform_roll(angle);
    angle = item->angles.pitch;
    if (angle)
        transform_pitch(angle);
    return model_draw_static(model_refs[item->kind], 0, 0);
}

/* Project model vertices and submit distance-faded quads and lines */

GDB_CALL uint32 model_draw_faded(MODEL_REFERENCE model, uint32 unused, sint32 bias)
{
    FUNCTION_MARKER(0x80011594u, "GAME.EXE");
    /* TODO: Investigate geometry entered before its current matrix is published */
    XASSERT(geometry_workspace.matrix);
    POSE_MATRIX *matrix = geometry_workspace.matrix;
    uint32 words[8];
    uint32 *xy, *xy_output;
    sint16 *depths, *depth_output;
    uint32 left, packet, ot, saved_depths[3];
    const sint16 *depth_offsets;
    const MODEL_FACE_RECORD *face;
    const MODEL_BODY_HEADER *header;
    const uint32 *vertices;
    uint32 proj_count;
    sint32 count, below127, below193, below385 = 0, cutoff, depth_carrier;
    int i;
    for (i = 0; i < 5; i++)
        words[i] = matrix->rotation_words[i];
    for (i = 0; i < 5; i++)
        xport_gte_write_control(i, words[i]);
    for (i = 5; i < 8; i++)
        words[i] = matrix->t[i - 5];
    for (i = 5; i < 8; i++)
        xport_gte_write_control(i, (sint32)words[i] >> 16);
    depth_carrier = (sint32)words[7] >> 16;
    XASSERT(model.header && model.byte_len >= sizeof(*header));
    header = model.header;
    count = header->vertex_count;
    proj_count = count > 6 ? (count + 2u) / 3 * 3 : 6;
    /* TODO: Investigate an original RTPT read extending beyond its model resource */
    XASSERT(model.vertices && !((uintptr_t)model.vertices & 3) && proj_count <= model.vertex_byte_len / sizeof(SVECTOR));
    /* RTPT consumes complete packed triples, including real adjacent file bytes */
    vertices = (const uint32 *)model.vertices;
    below127 = math_add_checked(count, -127);
    below193 = math_add_checked(count, -193);
    if (below127 >= 0)
        below385 = math_add_checked(count, -385);
    xy = below127 < 0 || below193 < 0 ? geometry_workspace.scratch.xy : geometry_workspace.model.proj.xy;
    depths = below127 < 0 ? geometry_workspace.scratch.depth + 256 : below193 < 0 ? geometry_workspace.model.depth : below385 < 0 ? geometry_workspace.scratch.depth : geometry_workspace.model.depth;
    xy_output = xy + 3;
    depth_output = depths + 3;
    for (i = 0; i < 6; i++)
        xport_gte_write_data(i, vertices[i]);
    xport_gte_execute(0x280030);
    vertices += 6;
    count = math_add_checked(count, -3);
    for (i = 0; i < 3; i++)
        xy_output[i - 3] = xport_gte_read_data(12 + i);
    for (i = 0; i < 3; i++)
        saved_depths[i] = xport_gte_read_data(17 + i);
    /* The original always projects a second triple before storing the first depths */
    do
    {
        for (i = 0; i < 6; i++)
            xport_gte_write_data(i, vertices[i]);
        xport_gte_execute(0x280030);
        vertices += 6;
        for (i = 0; i < 3; i++)
            depth_output[i - 3] = (sint16)saved_depths[i];
        xy_output += 3;
        depth_output += 3;
        count = math_add_checked(count, -3);
        for (i = 0; i < 3; i++)
            xy_output[i - 3] = xport_gte_read_data(12 + i);
        for (i = 0; i < 3; i++)
            saved_depths[i] = xport_gte_read_data(17 + i);
    } while (count > 0);
    for (i = 0; i < 3; i++)
        depth_output[i - 3] = (sint16)saved_depths[i];
    left = header->face_count;
    /* TODO: Investigate an original empty or out-of-resource face stream */
    XASSERT(left && model.faces && !((uintptr_t)model.faces & 3) && left <= model.face_byte_len / sizeof(*face));
    face = model.faces;
    cutoff = RENDER_FADE_CUTOFF;
    depth_offsets = depth_table;
    ot = render_buffers.ot_addr;
    packet = render_buffers.packet_addr;
    do
    {
        uint32 word1 = face->words[1], word0 = face->words[0], i0 = word0 & 65535, i1 = word0 >> 16;
        sint16 d0;
        sint32 depth;
        memcpy(&d0, (const uint8 *)depths + (i0 >> 1), sizeof(d0));
        if ((sint32)word1 < 0)
        {
            sint16 d1;
            sint32 sum;
            memcpy(&d1, (const uint8 *)depths + (i1 >> 1), sizeof(d1));
            sum = d0 + d1;
            uint32 xy0_offset, xy1_offset;
            left = math_add_checked(left, -1);
            /* The dead ADD checks the previous T7, not the current depth sum */
            math_add_checked(depth_carrier, bias);
            depth_carrier = depth = sum >> 3;
            xy0_offset = i0;
            if (depth > 8188)
                goto next_face;
            xy1_offset = i1;
            if (depth > 0 && depth <= 8188)
            {
                uint32 command = face->words[2], s0, s1;
                memcpy(&s0, (const uint8 *)xy + xy0_offset, sizeof(s0));
                memcpy(&s1, (const uint8 *)xy + xy1_offset, sizeof(s1));
                sint32 offset = depth_offsets[depth];
                depth_carrier = offset;
                uint32 bucket, next;
                LINE_F2 *line = psx_addr(packet, sizeof(*line));
                line->rgbc.rgbc = command;
                line->p[0].xy = s0;
                line->p[1].xy = s1;
                bucket = math_add_checked(ot, offset);
                uint32 *ot_entry = psx_addr(bucket, sizeof(*ot_entry));
                uint32 link = *ot_entry;
                face++;
                line->tag = link | 0x03000000u;
                next = math_add_checked(packet, 16);
                *ot_entry = packet;
                packet = next;
                continue;
            }
        }
        else
        {
            left = math_add_checked(left, -1);
            if (d0 <= 0)
                goto next_face;
            uint32 i2 = word1 & 65535u, i3 = word1 >> 16, color = 0x2d000000u;
            sint16 d1;
            memcpy(&d1, (const uint8 *)depths + (i2 >> 1), sizeof(d1));
            if (d1 > 0)
            {
                sint32 delta, fade;
                sint64 difference;
                depth = (sint32)math_add_checked(d0 + d1, bias) >> 1;
                difference = (sint64)depth - cutoff;
                XASSERT(difference >= -2147483647LL - 1 && difference <= 2147483647LL);
                delta = (sint32)difference;
                fade = math_add_checked(delta, 2048);
                depth_carrier = fade;
                if (delta <= 0)
                {
                    if (fade >= 0)
                    {
                        uint32 shade = 128u - (fade >> 4);
                        color = 0x2c000000u | shade | (shade << 8) | (shade << 16);
                    }
                    depth_carrier = depth >>= 2;
                    uint32 xy0_offset = i0;
                    if (depth <= 8192)
                    {
                        uint32 s0, s1, s2, s3;
                        memcpy(&s0, (const uint8 *)xy + xy0_offset, sizeof(s0));
                        memcpy(&s1, (const uint8 *)xy + i1, sizeof(s1));
                        memcpy(&s2, (const uint8 *)xy + i2, sizeof(s2));
                        memcpy(&s3, (const uint8 *)xy + i3, sizeof(s3));
                        if ((s0 & 65535u) < 320 || (s1 & 65535u) < 320 || (s2 & 65535u) < 320 || (s3 & 65535u) < 320)
                        {
                            uint32 flags;
                            sint32 offset, area;
                            xport_gte_write_data(12, s0);
                            xport_gte_write_data(14, s2);
                            if ((sint32)s0 <= 0 && (sint32)s1 <= 0 && (sint32)s2 <= 0 && (sint32)s3 < 0)
                                goto next_face;
                            xport_gte_write_data(13, s1);
                            /* Clamp the lookup index only; original out-of-range reachability is unproven */
                            sint32 depth_idx = depth;
                            if (depth_idx < 0)
                                depth_idx = 0;
                            else if (depth_idx >= DEPTH_TABLE_COUNT)
                                depth_idx = DEPTH_TABLE_COUNT - 1;
                            offset = depth_offsets[depth_idx];
                            depth_carrier = offset;
                            xport_gte_execute(0x140006);
                            flags = face->words[2];
                            area = xport_gte_read_data(24);
                            if ((flags & 1u) || area >= 0)
                            {
                                uint32 tex_offset = flags >> 16, uv, mode, end, semi, bucket, next;
                                XASSERT(!(tex_offset & 3) && tex_offset <= sizeof(tex_desc.words) - 12);
                                const uint32 *tex = tex_desc.words + tex_offset / 4;
                                POLY_FT4 *poly = psx_addr(packet, sizeof(*poly));
                                if (flags & 2u)
                                {
                                    poly->p[0].xy.xy = s1;
                                    poly->p[1].xy.xy = s0;
                                    poly->p[2].xy.xy = s2;
                                    poly->p[3].xy.xy = s3;
                                }
                                else
                                {
                                    poly->p[0].xy.xy = s0;
                                    poly->p[1].xy.xy = s1;
                                    poly->p[2].xy.xy = s3;
                                    poly->p[3].xy.xy = s2;
                                }
                                uv = tex[0];
                                mode = tex[1];
                                end = tex[2];
                                semi = uv & 1u;
                                poly->p[0].uv_attr = uv - semi;
                                poly->p[1].uv_attr = mode;
                                poly->p[2].uv.uv = end;
                                poly->p[3].uv.uv = end >> 16;
                                poly->rgbc.rgbc = (semi << 25) | color;
                                bucket = math_add_checked(ot, offset);
                                uint32 *ot_entry = psx_addr(bucket, sizeof(*ot_entry));
                                poly->tag = *ot_entry | 0x09000000u;
                                next = math_add_checked(packet, 40);
                                *ot_entry = packet & 0xffffffu;
                                packet = next;
                            }
                        }
                    }
                }
            }
        }
    next_face:
        face++;
    } while (left);
    render_style.color.rgbc = 0x2d000000u;
    render_buffers.packet_addr = packet;
    return 0x8008d4b0;
}

GDB_CALL uint32 object_draw_depth_fade(const MODEL_OBJECT *item)
{
    FUNCTION_MARKER(0x8005b684u, "GAME.EXE");
    PROJECTION_POINT pos;
    uint32 model, color;
    proj_local(0, 0, 0, &pos);
    /* TODO: Investigate drawing common fields outside both native object pools */
    XASSERT(item);
    model = item->kind;
    if (pos.depth - 1u >= 0x2fffu)
        return 0;
    color = 128u - (pos.depth >> 7);
    render_style.color.rgbc = 0x2c000000u + (color << 16) + (color << 8) + color;
    return model_draw_static(model_refs[model], 0, 0);
}

GDB_CALL uint32 object_draw_near_fade(const MODEL_OBJECT *item)
{
    FUNCTION_MARKER(0x8005b7acu, "GAME.EXE");
    PROJECTION_POINT pos;
    sint32 shade;
    uint32 color;
    proj_local(0, 0, 0, &pos);
    shade = (pos.depth >> 2) - 32;
    if (shade <= 0)
        return 1;
    if (shade < 128)
    {
        color = shade;
        render_style.color.rgbc = 0x2c000000u + (color << 16) + (color << 8) + color;
    }
    /* TODO: Investigate drawing common fields outside both native object pools */
    XASSERT(item);
    return model_draw_static(model_refs[item->kind], 0, 0);
}

GDB_CALL uint32 object_draw_depth_pair(const MODEL_OBJECT *item)
{
    FUNCTION_MARKER(0x8005d7a4u, "GAME.EXE");
    if (transform_origin_depth() - 257u >= 0xeffu)
        return 0;
    /* TODO: Investigate drawing common fields outside both native object pools */
    XASSERT(item);
    model_draw_static(model_refs[item->kind], 0, 0);
    return model_draw_static(model_refs[1 + (item->kind)], 0, 0);
}

GDB_CALL uint32 object_draw_visible_yaw(const MODEL_OBJECT *item)
{
    FUNCTION_MARKER(0x8005ab18u, "GAME.EXE");
    sint32 angle;
    if (!transform_origin_visible())
        return 0;
    /* TODO: Investigate drawing common fields outside both native object pools */
    XASSERT(item);
    angle = item->angles.yaw;
    if (angle)
        transform_yaw(angle);
    return model_draw_static(model_refs[item->kind], 0, 0);
}

GDB_CALL uint32 object_draw_camera_x_gated_fixed_model(void)
{
    FUNCTION_MARKER(0x8002a858u, "GAME.EXE");
    if ((sint32)camera.tracking.center.x.bits > 0x0fffffff)
        return 1;
    return model_draw_static(model_refs[360], 0, 0);
}

GDB_CALL uint32 object_draw_side_spin(const MODEL_OBJECT *model)
{
    FUNCTION_MARKER(0x8002a8fcu, "GAME.EXE");
    uint32 angle;
    sint32 x;
    transform_translate(0, 0x02800000, 0);
    /* TODO: Investigate a compound draw without a native position */
    XASSERT(model);
    x = model->pos.x.val;
    angle = game_session.tick;
    if (x > 0)
        angle = (uint32)(-(sint64)(angle));
    transform_roll(angle << 12);
    return model_draw_static(model_refs[400], 0, 0);
}

GDB_CALL uint32 object_draw_camera_yaw_outer(const MODEL_OBJECT *item)
{
    FUNCTION_MARKER(0x80032680u, "GAME.EXE");
    if ((uint16)(camera.angles.yaw_bits + 16384) <= 32768)
        return 0;
    /* TODO: Investigate drawing common fields outside both native object pools */
    XASSERT(item);
    return model_draw_static(model_refs[item->kind], 0, 0);
}

GDB_CALL uint32 object_draw_kind_bias(const MODEL_OBJECT *item)
{
    FUNCTION_MARKER(0x800326dcu, "GAME.EXE");
    /* TODO: Investigate drawing common fields outside both native object pools */
    XASSERT(item);
    uint32 kind = item->kind;
    return model_draw_static(model_refs[kind], 0, kind - 133u < 2u ? 1024 : 512);
}

GDB_CALL uint32 object_draw_stage_bias(const MODEL_OBJECT *item)
{
    FUNCTION_MARKER(0x80032728u, "GAME.EXE");
    if (!stage_animation.phase[0])
        return 0;
    /* TODO: Investigate drawing common fields outside both native object pools */
    XASSERT(item);
    return model_draw_static(model_refs[item->kind], 0, 512);
}

GDB_CALL uint32 object_draw_timed_scale(void)
{
    FUNCTION_MARKER(0x800327b0u, "GAME.EXE");
    sint32 phase = game_session.tick - 270u;
    if (phase < 0)
        phase = 0;
    else if (phase > 128)
        phase = 128;
    transform_scale(16384, 16384, phase << 7);
    return model_draw_static(model_refs[678], 0, 0);
}

GDB_CALL uint32 object_draw_flag_256(const MODEL_OBJECT *item)
{
    FUNCTION_MARKER(0x8002a00cu, "GAME.EXE");
    /* TODO: Investigate drawing common fields outside both native object pools */
    XASSERT(item);
    return model_draw_static(model_refs[item->kind], 0, 256);
}

GDB_CALL uint32 object_draw_flag_512(const MODEL_OBJECT *item)
{
    FUNCTION_MARKER(0x80032774u, "GAME.EXE");
    /* TODO: Investigate drawing common fields outside both native object pools */
    XASSERT(item);
    return model_draw_static(model_refs[item->kind], 0, 512);
}

GDB_CALL uint32 object_draw_fixed_pair(void)
{
    FUNCTION_MARKER(0x8002a89cu, "GAME.EXE");
    model_draw_static(model_refs[784], 0, 0);
    transform_translate(0x04e00000, 0x00f00000, 0);
    transform_yaw(16384);
    return model_draw_static(model_refs[515], 0, 0);
}

GDB_CALL void object_draw_height_offset(const MODEL_OBJECT *model)
{
    FUNCTION_MARKER(0x8002aa6cu, "GAME.EXE");
    uint32 x, y, z, camera_x, camera_y, camera_z, kind, height;
    transform_push();
    /* TODO: Investigate a compound draw without a native position */
    XASSERT(model);
    x = model->pos.x.bits;
    camera_x = camera.pos.x.bits;
    kind = model->kind;
    y = model->pos.y.bits;
    /* TODO: Investigate a model kind outside the native height table */
    XASSERT(kind < MODEL_TYPE_COUNT);
    height = model_height[kind];
    y += height;
    camera_y = camera.pos.y.bits;
    z = model->pos.z.bits;
    camera_z = camera.pos.z.bits;
    transform_translate(x - camera_x, y - camera_y, z - camera_z);
    transform_pitch(32768);
    transform_yaw(model->angles.yaw);
    transform_roll(-model->angles.roll);
    transform_pitch(model->angles.pitch);
    model_draw_static(model_refs[model->kind], 0, 0);
    transform_pop();
}

GDB_CALL uint32 object_draw_scene_variant(const MODEL_OBJECT *item)
{
    FUNCTION_MARKER(0x8002c294u, "GAME.EXE");
    /* TODO: Investigate drawing common fields outside both native object pools */
    XASSERT(item);
    uint32 kind = item->kind;
    if (game_session.scene_phase == 2)
        ++kind;
    return model_draw_static(model_refs[kind], 0, 0);
}

GDB_CALL uint32 object_draw_quarter_turn(const MODEL_OBJECT *item)
{
    FUNCTION_MARKER(0x8003ee7cu, "GAME.EXE");
    transform_yaw(49152);
    /* TODO: Investigate drawing common fields outside both native object pools */
    XASSERT(item);
    return model_draw_static(model_refs[item->kind], 0, 0);
}

GDB_CALL void object_draw_dimmed_yaw(const MODEL_OBJECT *item)
{
    FUNCTION_MARKER(0x8005bf0cu, "GAME.EXE");
    transform_push();
    /* TODO: Investigate common fields outside the native model pools */
    XASSERT(item);
    transform_yaw(item->angles.yaw);
    render_style.color.rgbc = 0x2c404040;
    model_draw_static(model_refs[item->kind], 0, 0);
    transform_pop();
}

GDB_CALL void object_draw_inverted_roll(const MODEL_OBJECT *item)
{
    FUNCTION_MARKER(0x8005bf7cu, "GAME.EXE");
    transform_push();
    /* TODO: Investigate common fields outside the native model pools */
    XASSERT(item);
    transform_yaw(item->angles.yaw);
    transform_pitch(item->angles.pitch);
    transform_roll(-item->angles.roll);
    model_draw_static(model_refs[item->kind], 0, 0);
    transform_pop();
}

GDB_CALL void object_draw_offset_spinning_pair(const MODEL_OBJECT *model)
{
    FUNCTION_MARKER(0x8005bff4u, "GAME.EXE");
    transform_push();
    /* TODO: Investigate drawing common fields outside both native object pools */
    XASSERT(model);
    transform_yaw(model->angles.yaw);
    transform_pitch(model->angles.pitch);
    transform_roll(-model->angles.roll);
    model_draw_static(model_refs[model->kind], 0, -256);
    transform_translate(-3145728, 2621440, 0);
    transform_yaw((uint32)(uint16)game_session.tick << 12);
    model_draw_static(model_refs[564], 0, -256);
    transform_pop();
    transform_push();
    /* TODO: Investigate a shadow draw without a native position */
    XASSERT(model);
    transform_translate(0, model->pos.y.val, 0);
    transform_yaw(model->angles.yaw);
    model_draw_static(model_refs[834], 0, -2048);
    transform_pop();
}

/* Exact original leaf, including its return delay slot; see leaf-batch.json. */

/* Exact original leaf, including its return delay slot; see leaf-batch.json. */
GDB_CALL uint32 object_update_yaw(MODEL_OBJECT *item)
{
    FUNCTION_MARKER(0x8002eb58u, "GAME.EXE");
    /* TODO: Investigate common fields outside the native model pools */
    XASSERT(item);
    uint32 val = (uint16)item->angles.yaw + 1004u;
    item->angles.yaw_bits = (uint16)val;
    return val;
}

/* Exact original leaf, including its return delay slot; see leaf-batch.json. */

/* Exact original leaf, including its return delay slot; see leaf-batch.json. */

/* Exact original leaf, including its return delay slot; see leaf-batch.json. */

/* Exact original leaf, including its return delay slot; see leaf-batch.json. */

/* Exact original leaf, including its return delay slot; see leaf-batch.json. */

/* Exact original leaf, including its return delay slot; see leaf-batch.json. */

/* Exact original leaf, including its return delay slot; see leaf-batch.json. */

/* Exact original leaf, including its return delay slot; see leaf-batch.json. */

/* Exact original leaf, including its return delay slot; see leaf-batch.json. */

/* Exact original leaf, including its return delay slot; see leaf-batch.json. */

/* Exact original leaf, including its return delay slot; see leaf-batch.json. */

/* Exact original leaf, including its return delay slot; see leaf-batch.json. */

/* Exact original leaf, including its return delay slot; see leaf-batch.json. */

static void model_apply_pose_rotations(const void *src, int halfwords)
{
    uint32 roll, yaw, pitch;
    roll = halfwords ? (uint32)(sint32)((const POSE_ANGLES *)src)->roll : ((const uint32 *)src)[0];
    if (roll)
        transform_roll_idx(roll << 2);
    yaw = halfwords ? (uint32)(sint32)((const POSE_ANGLES *)src)->yaw : ((const uint32 *)src)[1];
    if (yaw)
    {
        XASSERT(yaw != 0x80000000u);
        transform_yaw_idx((((uint32)(-(sint64)(yaw))) & 0xfff) << 2);
    }
    pitch = halfwords ? (uint32)(sint32)((const POSE_ANGLES *)src)->pitch : ((const uint32 *)src)[2];
    if (pitch)
    {
        XASSERT(pitch != 0x80000000u);
        transform_pitch_idx((((uint32)(-(sint64)(pitch))) & 0xfff) << 2);
    }
}

/* Model stream transform phase through 80010AB8
 * FUN_8001060C performs the final matrix pop and shared packet phase */
GDB_CALL void model_transform_pose(MODEL_REFERENCE model, ACTOR_POSE *pose)
{
    uint32 idx = 0, command;
    const uint32 *stream, *counts, *end;
    const MODEL_RIG_HEADER *rig;
    sint32 angle;
    XASSERT(model.header && model.byte_len >= sizeof(*rig));
    rig = (const MODEL_RIG_HEADER *)model.header;
    counts = rig->part_vertex_count;
    end = (const uint32 *)model.header + model.byte_len / sizeof(uint32);
    XASSERT(!(rig->commands_offset & 3) && rig->commands_offset <= model.byte_len - sizeof(uint32));
    stream = (const uint32 *)((const uint8 *)rig + rig->commands_offset);
    transform_push();
    angle = pose->angles[0].pitch;
    if (angle)
        transform_pitch_idx(((uint32)angle & 0xfff) << 2);
    angle = pose->angles[0].yaw;
    if (angle)
        transform_yaw_idx(((uint32)angle & 0xfff) << 2);
    angle = pose->angles[0].roll;
    if (angle)
        transform_roll_idx(((uint32)angle & 0xfff) << 2);
    XASSERT(stream != end);
    command = *stream;
    stream++;
    for (;;)
    {
        POSE_MATRIX *matrix;
        uint32 flag, bone, count, point_idx = 0xffffffff, words[8], xyz[3];
        int i;
        if (command == 1)
        {
            transform_push();
            goto next_command;
        }
        if (command == 2)
        {
            transform_pop();
            goto next_command;
        }
        XASSERT(end - stream >= 4);
        flag = *stream;
        /* TODO: Investigate geometry entered before its current matrix is published */
        XASSERT(geometry_workspace.matrix);
        matrix = geometry_workspace.matrix;
        for (i = 0; i < 5; i++)
            words[i] = matrix->rotation_words[i];
        for (i = 0; i < 5; i++)
            xport_gte_write_control(i, words[i]);
        for (i = 5; i < 8; i++)
            words[i] = (uint32)matrix->t[i - 5];
        for (i = 5; i < 8; i++)
            xport_gte_write_control(i, (sint32)words[i] >> 16);
        for (i = 0; i < 3; i++)
            xyz[i] = stream[1 + i];
        stream += 4;
        xport_gte_write_data(0, (xyz[0] >> 16) | (xyz[1] & 0xffff0000));
        xport_gte_write_data(1, (sint32)xyz[2] >> 16);
        xport_gte_execute(0x480012);
        /* TODO: Investigate geometry entered before its current matrix is published */
        XASSERT(geometry_workspace.matrix);
        matrix = geometry_workspace.matrix;
        for (i = 0; i < 3; i++)
            xyz[i] = xport_gte_read_data(25 + i) << 16;
        for (i = 0; i < 3; i++)
            matrix->t[i] = (sint32)xyz[i];
        XASSERT(end - stream >= 3);
        model_apply_pose_rotations(stream, 0);
        stream += 3;
        XASSERT(stream != end);
        bone = *stream;
        stream++;
        XASSERT(bone < ACTOR_POSE_BONE_COUNT);
        model_apply_pose_rotations(&pose->angles[bone], 1);
        if (flag == 1)
        {
            transform_yaw_idx(pose->sway_yaw << 2);
            point_idx = 6;
        }
        else if (flag == 2 || flag == 3 || flag == 10 || flag == 11)
        {
            uint32 matrix_idx = flag == 2 ? 0 : flag == 3 ? 1 : flag == 10 ? 2 : 3;
            /* TODO: Investigate geometry entered before its current matrix is published */
            XASSERT(geometry_workspace.matrix);
            matrix = geometry_workspace.matrix;
            for (i = 0; i < 8; i++)
                words[i] = i < 5 ? matrix->rotation_words[i] : (uint32)matrix->t[i - 5];
            memcpy(&pose->matrix[matrix_idx], words, sizeof(POSE_MATRIX));
        }
        else if (flag >= 4 && flag <= 9)
            point_idx = flag - 4;
        if (point_idx != 0xffffffff)
        {
            /* TODO: Investigate geometry entered before its current matrix is published */
            XASSERT(geometry_workspace.matrix);
            matrix = geometry_workspace.matrix;
            for (i = 0; i < 3; i++)
                xyz[i] = (uint32)matrix->t[i];
            for (i = 0; i < 3; i++)
                pose->point[point_idx].v[i] = xyz[i];
        }
        XASSERT(counts != end);
        count = *counts;
        counts++;
        if (count)
        {
            uint32 proj_count, remaining = count, depths[3];
            uint32 *xy;
            sint16 *depth_out;
            const uint32 *vertex;
            /* TODO: Investigate geometry entered before its current matrix is published */
            XASSERT(geometry_workspace.matrix);
            matrix = geometry_workspace.matrix;
            for (i = 0; i < 8; i++)
                words[i] = i < 5 ? matrix->rotation_words[i] : (uint32)matrix->t[i - 5];
            for (i = 0; i < 5; i++)
                xport_gte_write_control(i, words[i]);
            for (i = 5; i < 8; i++)
                xport_gte_write_control(i, (sint32)words[i] >> 16);
            proj_count = (sint32)count > 3 ? (count + 2) / 3 * 3 : 3;
            /* TODO: Investigate original rig parts reading past their model resource */
            XASSERT(model.vertices && !((uintptr_t)model.vertices & 3) && idx <= model.vertex_byte_len / sizeof(SVECTOR) && proj_count <= model.vertex_byte_len / sizeof(SVECTOR) - idx);
            vertex = (const uint32 *)(model.vertices + idx);
            xy = geometry_workspace.model.proj.xy + idx;
            depth_out = geometry_workspace.model.depth + idx;
            idx += count;
            do
            {
                for (i = 0; i < 6; i++)
                    xport_gte_write_data(i, vertex[i]);
                xport_gte_execute(0x280030);
                vertex += 6;
                XASSERT((sint32)remaining >= -2147483645);
                remaining -= 3;
                for (i = 0; i < 3; i++)
                    xy[i] = xport_gte_read_data(12 + i);
                for (i = 0; i < 3; i++)
                    depths[i] = xport_gte_read_data(17 + i);
                for (i = 0; i < 3; i++)
                    depth_out[i] = (sint16)depths[i];
                xy += 3;
                depth_out += 3;
            } while ((sint32)remaining > 0);
        }
        XASSERT(end - stream >= 3);
        model_apply_pose_rotations(stream, 0);
        stream += 3;
    next_command:
        XASSERT(stream != end);
        command = *stream;
        stream++;
        if (!command)
            break;
    }
}

/* Shared polygon phase. Signed packed XY tests and line bucket alias behavior
 * deliberately follow MIPS rather than a generic polygon clipping policy. */
GDB_CALL uint32 model_draw_packets(uint32 count, const MODEL_FACE_RECORD *recs, const uint32 *xy, const sint16 *depths, sint32 bias, sint32 depth_carrier)
{
    uint32 packet, ot, color, blend, blend_override;
    const sint16 *depth_offsets;
    depth_offsets = depth_table;
    color = render_style.color.rgbc;
    blend = render_style.blend_mode;
    blend_override = math_add_checked(blend, -1) << 21;
    ot = render_buffers.ot_addr;
    packet = render_buffers.packet_addr;
    XASSERT(count);
    do
    {
        uint32 word0, word1, i0, i1, i2, i3;
        sint32 depth;
        uint32 a, b, c, d, bucket, uv0, uv1, uv2, flags;
        const uint32 *tex;
        word1 = recs->words[1];
        word0 = recs->words[0];
        i0 = word0 & 65535;
        i1 = word0 >> 16;
        i2 = word1 & 65535;
        i3 = word1 >> 16;
        if ((sint32)word1 < 0)
        {
            sint16 z0, z1;
            memcpy(&z0, (const uint8 *)depths + (i0 >> 1), sizeof(z0));
            memcpy(&z1, (const uint8 *)depths + (i1 >> 1), sizeof(z1));
            uint32 xy0_offset, xy1_offset, link, next;
            count = math_add_checked(count, -1);
            /* The overwritten ADD still checks the previous T7 for overflow */
            math_add_checked(depth_carrier, bias);
            depth_carrier = depth = (z0 + z1) >> 3;
            xy0_offset = i0;
            if (depth > 8188)
                goto next_rec;
            xy1_offset = i1;
            if (depth <= 0)
                goto next_rec;
            flags = recs->words[2];
            memcpy(&a, (const uint8 *)xy + xy0_offset, sizeof(a));
            memcpy(&b, (const uint8 *)xy + xy1_offset, sizeof(b));
            depth_carrier = depth_offsets[depth];
            /* Original model line records carry GP0(40), not a synthesized color command */
            LINE_F2 *line = psx_addr(packet, sizeof(*line));
            line->rgbc.rgbc = flags;
            line->p[0].xy = a;
            line->p[1].xy = b;
            bucket = math_add_checked(ot, depth_carrier);
            uint32 *ot_entry = psx_addr(bucket, sizeof(*ot_entry));
            link = *ot_entry;
            recs++;
            line->tag = link | 0x03000000;
            next = math_add_checked(packet, 16);
            *ot_entry = packet;
            packet = next;
            continue;
        }
        else
        {
            sint16 z0, z2;
            sint32 clip;
            uint32 xy0_offset, next;
            memcpy(&z0, (const uint8 *)depths + (i0 >> 1), sizeof(z0));
            count = math_add_checked(count, -1);
            if (z0 <= 0)
                goto next_rec;
            memcpy(&z2, (const uint8 *)depths + (i2 >> 1), sizeof(z2));
            if (z2 <= 0)
                goto next_rec;
            depth_carrier = depth = (sint32)math_add_checked(z0 + z2, bias) >> 3;
            xy0_offset = i0;
            if (depth > 8192)
                goto next_rec;
            memcpy(&a, (const uint8 *)xy + xy0_offset, sizeof(a));
            memcpy(&b, (const uint8 *)xy + i1, sizeof(b));
            memcpy(&c, (const uint8 *)xy + i2, sizeof(c));
            memcpy(&d, (const uint8 *)xy + i3, sizeof(d));
            if ((a & 65535) >= 320 && (b & 65535) >= 320 && (c & 65535) >= 320 && (d & 65535) >= 320)
                goto next_rec;
            xport_gte_write_data(12, a);
            xport_gte_write_data(14, c);
            if ((sint32)a <= 0 && (sint32)b <= 0 && (sint32)c <= 0 && (sint32)d < 0)
                goto next_rec;
            xport_gte_write_data(13, b);
            /* Clamp the lookup index only; original out-of-range reachability is unproven */
            sint32 depth_idx = depth;
            if (depth_idx < 0)
                depth_idx = 0;
            else if (depth_idx >= DEPTH_TABLE_COUNT)
                depth_idx = DEPTH_TABLE_COUNT - 1;
            depth_carrier = depth_offsets[depth_idx];
            xport_gte_execute(0x140006);
            flags = recs->words[2];
            clip = (sint32)xport_gte_read_data(24);
            if (!(flags & 1) && clip < 0)
                goto next_rec;
            uint32 tex_offset = flags >> 16;
            XASSERT(!(tex_offset & 3) && tex_offset <= sizeof(tex_desc.words) - 12);
            tex = tex_desc.words + tex_offset / 4;
            POLY_FT4 *poly = psx_addr(packet, sizeof(*poly));
            if (flags & 2)
            {
                poly->p[0].xy.xy = b;
                poly->p[1].xy.xy = a;
                poly->p[2].xy.xy = c;
                poly->p[3].xy.xy = d;
            }
            else
            {
                poly->p[0].xy.xy = a;
                poly->p[1].xy.xy = b;
                poly->p[2].xy.xy = d;
                poly->p[3].xy.xy = c;
            }
            uv0 = tex[0];
            uv1 = tex[1];
            uv2 = tex[2];
            if (blend)
                uv1 = (uv1 & 0xff9fffff) | blend_override;
            poly->p[0].uv_attr = uv0 - (uv0 & 1);
            poly->p[1].uv_attr = uv1;
            poly->p[2].uv.uv = uv2;
            poly->p[3].uv.uv = uv2 >> 16;
            poly->rgbc.rgbc = color | ((uv0 & 1) << 25);
            bucket = math_add_checked(ot, depth_carrier);
            uint32 *ot_entry = psx_addr(bucket, sizeof(*ot_entry));
            poly->tag = *ot_entry | 0x09000000;
            next = math_add_checked(packet, 40);
            *ot_entry = packet & 0xffffff;
            packet = next;
        }
    next_rec:
        recs++;
    } while (count);
    render_style.color.rgbc = 0x2d000000;
    render_style.blend_mode = 0;
    render_buffers.packet_addr = packet;
    return 0x8008d4b0;
}

GDB_CALL uint32 model_draw_pose(MODEL_REFERENCE model, ACTOR_POSE *pose)
{
    FUNCTION_MARKER(0x8001060cu, "GAME.EXE");
    uint32 count;
    const MODEL_FACE_RECORD *recs;
    const MODEL_BODY_HEADER *header;
    model_transform_pose(model, pose);
    transform_pop();
    header = model.header;
    count = header->face_count;
    /* TODO: Investigate an original empty or out-of-resource face stream */
    XASSERT(count && model.faces && !((uintptr_t)model.faces & 3) && count <= model.face_byte_len / sizeof(*recs));
    recs = model.faces;
    /* With zero bias the overwritten ADD cannot observe the incoming carrier */
    return model_draw_packets(count, recs, geometry_workspace.model.proj.xy, geometry_workspace.model.depth, 0, 0);
}

/* Static entry pipelines depth stores behind the next RTPT triple
 * The original always projects at least two triples */
GDB_CALL uint32 model_draw_static(MODEL_REFERENCE model, uint32 unused, sint32 bias)
{
    sint32 count, below127, below193, below385 = 0;
    const POSE_MATRIX *matrix;
    uint32 words[8];
    uint32 *xy, *xy_out;
    sint16 *depths, *depth_out;
    uint32 prev_depths[3], remaining, packets;
    const MODEL_FACE_RECORD *recs;
    const MODEL_BODY_HEADER *header;
    const uint32 *vertices;
    uint32 proj_count;
    int i;
    /* TODO: Investigate geometry entered before its current matrix is published */
    XASSERT(geometry_workspace.matrix);
    matrix = geometry_workspace.matrix;
    for (i = 0; i < 5; i++)
        words[i] = matrix->rotation_words[i];
    for (i = 0; i < 5; i++)
        xport_gte_write_control(i, words[i]);
    for (i = 5; i < 8; i++)
        words[i] = (uint32)matrix->t[i - 5];
    for (i = 5; i < 8; i++)
        xport_gte_write_control(i, (sint32)words[i] >> 16);
    XASSERT(model.header && model.byte_len >= sizeof(*header));
    header = model.header;
    count = header->vertex_count;
    proj_count = count > 6 ? ((uint32)count + 2) / 3 * 3 : 6;
    /* TODO: Investigate an original RTPT read extending beyond its model resource */
    XASSERT(model.vertices && !((uintptr_t)model.vertices & 3) && proj_count <= model.vertex_byte_len / sizeof(SVECTOR));
    /* RTPT consumes complete packed triples, including real adjacent file bytes */
    vertices = (const uint32 *)model.vertices;
    below127 = (sint32)math_add_checked(count, -127);
    below193 = (sint32)math_add_checked(count, -193);
    if (below127 >= 0)
        below385 = (sint32)math_add_checked(count, -385);
    xy = below127 < 0 || below193 < 0 ? geometry_workspace.scratch.xy : geometry_workspace.model.proj.xy;
    depths = below127 < 0 ? geometry_workspace.scratch.depth + 256 : below193 < 0 ? geometry_workspace.model.depth : below385 < 0 ? geometry_workspace.scratch.depth : geometry_workspace.model.depth;
    xy_out = xy + 3;
    depth_out = depths + 3;
    for (i = 0; i < 6; i++)
        xport_gte_write_data(i, vertices[i]);
    xport_gte_execute(0x280030);
    vertices += 6;
    remaining = math_add_checked(count, -3);
    for (i = 0; i < 3; i++)
        xy_out[i - 3] = xport_gte_read_data(12 + i);
    for (i = 0; i < 3; i++)
        prev_depths[i] = xport_gte_read_data(17 + i);
    do
    {
        for (i = 0; i < 6; i++)
            xport_gte_write_data(i, vertices[i]);
        xport_gte_execute(0x280030);
        vertices += 6;
        for (i = 0; i < 3; i++)
            depth_out[i - 3] = (sint16)prev_depths[i];
        xy_out += 3;
        depth_out += 3;
        remaining = math_add_checked(remaining, -3);
        for (i = 0; i < 3; i++)
            xy_out[i - 3] = xport_gte_read_data(12 + i);
        for (i = 0; i < 3; i++)
            prev_depths[i] = xport_gte_read_data(17 + i);
    } while ((sint32)remaining > 0);
    for (i = 0; i < 3; i++)
        depth_out[i - 3] = (sint16)prev_depths[i];
    packets = header->face_count;
    /* TODO: Investigate an original empty or out-of-resource face stream */
    XASSERT(packets && model.faces && !((uintptr_t)model.faces & 3) && packets <= model.face_byte_len / sizeof(*recs));
    recs = model.faces;
    return model_draw_packets(packets, recs, xy, depths, bias, (sint32)words[7] >> 16);
}

GDB_CALL uint32 model_draw_menu(MODEL_REFERENCE model, uint32 unused, sint32 bias)
{
    FUNCTION_MARKER(0x80067b8cu, "GAME.EXE");
    return model_draw_static(model, unused, bias);
}

GDB_CALL uint32 object_draw_rotated(const MODEL_OBJECT *item)
{
    FUNCTION_MARKER(0x8002a0c4, "GAME.EXE");
    object_apply_rotations(item);
    /* TODO: Investigate drawing common fields outside both native object pools */
    XASSERT(item);
    return model_draw_static(model_refs[item->kind], 0, 0);
}

GDB_CALL uint32 object_draw_phase_yaw_variant(void)
{
    FUNCTION_MARKER(0x80029f48, "GAME.EXE");
    uint32 model;
    if ((sint32)game_session.scene_phase < 3 || ((camera.angles.yaw_bits + 12288) & 65535) >= 24577)
        model_draw_static(model_refs[59], 0, 0);
    if ((sint32)game_session.scene_phase < 3)
        model = 56;
    else
        model = ((camera.angles.yaw_bits + 16384) & 65535) > 32768 ? 58 : 57;
    return model_draw_static(model_refs[model], 0, 0);
}

GDB_CALL uint32 object_draw_yaw_flag_128(const MODEL_OBJECT *item)
{
    FUNCTION_MARKER(0x8003262c, "GAME.EXE");
    /* TODO: Investigate drawing common fields outside both native object pools */
    XASSERT(item);
    transform_yaw(item->angles.yaw);
    return model_draw_static(model_refs[item->kind], 0, 128);
}

GDB_CALL uint32 object_draw_camera_yaw_inner(const MODEL_OBJECT *item)
{
    FUNCTION_MARKER(0x8002a048, "GAME.EXE");
    sint32 angle;
    if (((camera.angles.yaw_bits + 16383) & 65535) >= 32767)
        return 0;
    /* TODO: Investigate drawing common fields outside both native object pools */
    XASSERT(item);
    angle = item->angles.yaw;
    if (angle)
        transform_yaw(angle);
    return model_draw_static(model_refs[item->kind], 0, 0);
}

GDB_CALL uint32 object_draw_wave(MODEL_OBJECT *item)
{
    FUNCTION_MARKER(0x8002ab40, "GAME.EXE");
    uint32 phase, tick;
    /* TODO: Investigate a wave draw with an absent or foreign native phase owner */
    XASSERT(item && item->payload_kind == MODEL_PAYLOAD_WAVE);
    if ((sint32)game_session.tick < 2)
        item->wave.phase = (uint16)random_next();
    phase = item->wave.phase;
    tick = game_session.tick;
    phase = ((phase + tick) << 5) & 0xfe0;
    transform_translate(0, 0, 8 * sin_table[phase >> 1]);
    /* TODO: Investigate common fields outside the native model pools */
    XASSERT(item);
    return model_draw_static(model_refs[item->kind], 0, 0);
}

GDB_CALL uint32 object_draw_visible_player_facing(MODEL_OBJECT *model)
{
    FUNCTION_MARKER(0x8005ab84, "GAME.EXE");
    sint16 angle;
    sint32 object_x, object_z, player_x, player_z;
    if (!transform_origin_visible())
        return 0;
    /* TODO: Investigate visible drawing without a native position */
    XASSERT(model);
    object_x = model->pos.x.whole;
    object_z = model->pos.z.whole;
    player_x = actor_positions[0].x.whole;
    player_z = actor_positions[0].z.whole;
    angle = (sint16)math_dir16(object_x, object_z, player_x, player_z);
    model->angles.yaw_bits = angle;
    transform_yaw(angle);
    return model_draw_static(model_refs[model->kind], 0, 0);
}

GDB_CALL void object_draw_raised_spinning_pair(const MODEL_OBJECT *model)
{
    FUNCTION_MARKER(0x8002a97c, "GAME.EXE");
    transform_push();
    transform_translate(0, 0x800000, 0);
    /* TODO: Investigate compound draw fields outside both native object pools */
    XASSERT(model);
    transform_yaw(model->angles.yaw);
    transform_roll(-model->angles.roll);
    transform_pitch(model->angles.pitch);
    model_draw_static(model_refs[118], 0, 0);
    transform_translate(0, 0x400000, 0);
    transform_yaw((uint32)(uint16)game_session.tick << 12);
    model_draw_static(model_refs[564], 0, 0);
    transform_pop();
    transform_push();
    /* TODO: Investigate a compound draw without a native position */
    XASSERT(model);
    transform_translate(0, model->pos.y.val, 0);
    transform_yaw(model->angles.yaw);
    model_draw_static(model_refs[834], 0, 0);
    transform_pop();
}

/* 5AE3C: model transforms and six/two flashing sprites, actual MIPS order. */

/* 800451A0: compound scene model, spinning component and ground shadow. */
GDB_CALL void object_draw_pitched_spinning_pair(const MODEL_OBJECT *model)
{
    FUNCTION_MARKER(0x800451a0, "GAME.EXE");
    sint32 angle;
    /* TODO: Investigate a compound draw without a native position */
    XASSERT(model);
    if (model->pos.y.val <= -536870912)
        return;
    transform_push();
    transform_yaw(model->angles.yaw);
    angle = model->angles.roll;
    if (angle)
        transform_roll(angle);
    transform_pitch(model->angles.pitch + 4096);
    model_draw_static(model_refs[833], 0, 0);
    transform_yaw((uint32)(uint16)game_session.tick << 12);
    model_draw_static(model_refs[564], 0, 0);
    transform_pop();
    transform_push();
    transform_translate(0, model->pos.y.val, 0);
    transform_yaw(model->angles.yaw);
    model_draw_menu(model_refs[834], 0, 0);
    transform_pop();
}

/* 800452AC: static object with three phase-offset effect attachments. */
GDB_CALL void object_draw_phased_effects(const MODEL_OBJECT *model)
{
    FUNCTION_MARKER(0x800452ac, "GAME.EXE");
    uint32 phase[3];
    sint32 angle;
    transform_push();
    /* TODO: Investigate compound draw fields outside both native object pools */
    XASSERT(model);
    transform_yaw(model->angles.yaw);
    angle = model->angles.roll;
    if (angle)
        transform_roll(angle);
    if (model->angles.pitch)
        transform_pitch(4096);
    model_draw_static(model_refs[model->kind], 0, 0);
    phase[0] = game_session.tick & 31;
    if (model->kind == 836)
        phase[0] = (phase[0] + 19) & 31;
    phase[1] = (phase[0] + 8) & 31;
    phase[2] = (phase[0] + 16) & 31;
    transform_translate(-4194304, 16252928, 0);
    if (phase[0] < 6)
        model_draw_static(model_refs[phase[0] < 2 ? 840 : phase[0] < 4 ? 839 : 838], 0, -256);
    transform_translate(4194304, 0, 0);
    if (phase[1] < 6)
        model_draw_static(model_refs[phase[1] < 2 ? 840 : phase[1] < 4 ? 839 : 838], 0, -256);
    transform_translate(4194304, 0, 0);
    if (phase[2] < 6)
        model_draw_static(model_refs[phase[2] < 2 ? 840 : phase[2] < 4 ? 839 : 838], 0, -256);
    transform_pop();
    transform_push();
    /* TODO: Investigate a compound draw without a native position */
    XASSERT(model);
    transform_translate(0, model->pos.y.val, 0);
    transform_yaw(model->angles.yaw);
    model_draw_menu(model_refs[837], 0, 0);
    transform_pop();
}

GDB_CALL uint32 model_draw_pulsing_marker(void)
{
    FUNCTION_MARKER(0x80059394u, "GAME.EXE");
    uint32 shade = 64u - 16u * (game_session.tick & 1u);
    render_style.color.rgbc = 0x2c000000u + (shade << 16) + (shade << 8) + shade;
    return model_draw_static(model_refs[919], 0, 4096);
}

/* 5AC08 compound roadside model, depth shade and alternating shadow. */
GDB_CALL uint32 object_draw_offset_shadow(const MODEL_OBJECT *item)
{
    FUNCTION_MARKER(0x8005ac08, "GAME.EXE");
    /* TODO: Investigate common draw fields outside both native object pools */
    XASSERT(item);
    uint32 type = item->kind;
    FIXED_VEC3 pos;
    sint32 depth, shade;
    if (!transform_origin_visible())
        return 0;
    transform_point(0, 0, 0, pos.bits);
    transform_yaw(item->angles.yaw);
    depth = (sint16)(pos.z.bits >> 16);
    if (depth > 8192)
    {
        shade = (depth - 8192) >> 7;
        if (shade > 96)
            shade = 96;
        shade = 128 - shade;
        render_style.color.rgbc = 0x2c000000u + ((uint32)shade << 16) + ((uint32)shade << 8) + shade;
    }
    model_draw_static(model_refs[type], 0, 0);
    model_draw_static(model_refs[type == 80 ? 89 : 106], 0, 8192);
    transform_yaw(-16384);
    transform_translate(-25165824, 0, 0);
    if (type == 78)
        transform_translate(-8388608, 0, 0);
    if (type == 80)
        transform_translate(-16777216, 0, 0);
    return model_draw_pulsing_marker();
}

/* 2B28C: remove the impact sprite at age6, then increment its unsigned age. */
GDB_CALL uint32 object_update_impact_spr(MODEL_OBJECT *item)
{
    FUNCTION_MARKER(0x8002b28cu, "GAME.EXE");
    uint32 result;
    /* TODO: Investigate a model sprite callback with a foreign native payload */
    XASSERT(!item || item->payload_kind == MODEL_PAYLOAD_SPRITE);
    MODEL_SPRITE *model_spr = item ? &item->spr : NULL;
    /* TODO: Investigate an impact sprite outside the native dynamic pool */
    XASSERT(model_spr);
    if (model_spr->frame == 6)
    {
        /* TODO: Investigate a dynamic callback outside its native pool */
        XASSERT(item);
        item->kind = 0xffffffff;
    }
    result = ((uint16)model_spr->frame) + 1;
    model_spr->frame = (uint16)result;
    return result;
}

/* 11FEC: original JR RA/NOP; no result or side effects. */

GDB_CALL void object_transform_camera(const MODEL_OBJECT *item, const EFFECT *effect)
{
    FUNCTION_MARKER(0x8005c740, "GAME.EXE");
    uint32 x, y, z, camera_x, camera_y, camera_z, type, height;
    sint32 angle;
    const EFFECT_SPRITE *spr = NULL;
    const EFFECT_PARTICLE *debris = NULL;
    const FIXED_VEC3 *pos = NULL;
    /* TODO: Investigate foreign objects or unpublished effect positions */
    XASSERT((item || effect) && (!item || !effect));
    if (effect)
    {
        if (effect->payload_kind == EFFECT_PAYLOAD_SPRITE)
        {
            spr = &effect->spr;
            pos = &spr->pos;
        }
        else if (effect->payload_kind == EFFECT_PAYLOAD_PARTICLE)
        {
            debris = &effect->particle;
            pos = &debris->pos;
        }
        else
        {
            /* TODO: Investigate callbacks crossing native payload formats */

            XASSERT(0);
        }
    }
    else
        pos = &item->pos;
    x = pos->x.bits;
    camera_x = camera.pos.x.bits;
    type = effect ? effect->kind : item->kind;
    if (effect)
    {
        /* TODO: Investigate missing publication or cross-format slot reuse */
        XASSERT((!spr || type - 870u < 5u || type == 877 || type == 879 || type == 880 || type - 891u < 5u || type - 575u < 2u || type - 602u < 6u || type == 0xffffffffu) && (!debris || type - 897u < 13u || type < 6u || type - 6u < 2u || type - 577u < 3u || type == 583 || type - 599u < 3u || type - 72u < 4u || type == 0xffffffffu));
    }
    y = pos->y.bits;
    /* TODO: Investigate original model-height reads outside the native table */
    XASSERT(type < MODEL_TYPE_COUNT);
    height = model_height[type];
    y += height;
    camera_y = camera.pos.y.bits;
    z = pos->z.bits;
    camera_z = camera.pos.z.bits;
    transform_translate(x - camera_x, y - camera_y, z - camera_z);
    transform_pitch(32768);
    angle = spr ? spr->motion.x : debris ? debris->yaw : item->angles.yaw;
    if (angle)
        transform_yaw(angle);
    angle = spr ? spr->motion.y : debris ? debris->pitch : item->angles.pitch;
    if (angle)
        transform_pitch(angle);
    angle = spr ? spr->motion.z : debris ? debris->roll : item->angles.roll;
    if (angle)
        transform_roll(angle);
}
