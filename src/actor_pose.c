#include <stddef.h>
#include <stdio.h>
#include <string.h>

#include "game.h"

ACTOR_POSE actor_poses[ACTOR_POSE_COUNT];

/* Explicit wire composition shares these reviewed scalar layout boundaries */
_Static_assert(sizeof(ANIMATION_STREAM_HEADER) == 4, "ADT stream header size");
_Static_assert(offsetof(ANIMATION_STREAM_HEADER, frame) == 4, "ADT frame descriptor offset");
_Static_assert(sizeof(POSE_MATRIX) == 32, "Pose matrix size");
_Static_assert(offsetof(POSE_MATRIX, rotation_tail) == 18, "Packed rotation tail");
_Static_assert(offsetof(POSE_MATRIX, t) == 20, "Q16 translation offset");
_Static_assert(sizeof(POSE_POINT) == 12, "Pose point size");
_Static_assert(sizeof(POSE_ANGLES) == 6, "Pose angle size");
_Static_assert(offsetof(ACTOR_POSE, matrix) == 4, "Pose matrix offset");
_Static_assert(offsetof(ACTOR_POSE, point) == 132, "Pose point offset");
_Static_assert(offsetof(ACTOR_POSE, angles) == 216, "Pose angle offset");
_Static_assert(sizeof(ACTOR_POSE) == 504, "Pose wire extent");

/* Preserve the original signed ADD/ADDI exception before subsequent effects */
static uint32 add_checked(uint32 left, uint32 right)
{
    uint32 result = left + right;
    if (~(left ^ right) & (left ^ result) & 0x80000000)
        diagnostic_stop(__FUNCTION__, __FILE__, __LINE__);
    return result;
}

static uint32 read_bit(const uint8 **flags, const uint8 *end, uint32 *bits, uint32 *remaining)
{
    uint32 val;
    if (!*remaining)
    {
        if (*flags >= end)
            diagnostic_stop(__FUNCTION__, __FILE__, __LINE__);
        *bits = *(*flags)++;
        *remaining = 8;
    }
    val = *bits & 1;
    *bits >>= 1;
    --*remaining;
    return val;
}

/* Initial motion retains its distinct descriptor base and untouched lateral component */
GDB_CALL void actor_pose_load_initial_motion(sint32 idx)
{
    FUNCTION_MARKER(0x80011a04, "GAME.EXE");
    ACTOR_ANIMATION *playback = actor_animations + idx;
    const ANIMATION_STREAM *stream = &playback->src.stream;
    const uint8 *data;
    uint32 header, offset, val;
    if (playback->src.kind == ACTOR_ANIMATION_SOURCE_NONE)
    {
        /* The original null-source branch returns without publishing motion */
        return;
    }
    if (playback->src.kind != ACTOR_ANIMATION_SOURCE_STREAM || !stream->header || stream->byte_len < 8)
        diagnostic_stop(__FUNCTION__, __FILE__, __LINE__);
    header = stream->header->frame[0];
    ACTOR_FRAME_MOTION *motion = &playback->motion;
    motion->events = header >> 16;
    offset = header & 65535;
    if (offset > stream->byte_len || stream->byte_len - offset < 12)
        diagnostic_stop(__FUNCTION__, __FILE__, __LINE__);
    /* Startup uses the stream origin, not the frame decoder's four-byte bias */
    data = (const uint8 *)stream->header + offset;
    memcpy(&val, data, sizeof(val));
    motion->bits[0] = val;
    memcpy(&val, data + 4, sizeof(val));
    memcpy(&val, data + 8, sizeof(val));
    motion->bits[2] = val;
}

/* Decode an absolute first frame, then sparse signed angle deltas */
GDB_CALL void actor_pose_decode(MODEL_REFERENCE model, sint32 idx, ACTOR_POSE *pose)
{
    FUNCTION_MARKER(0x80011a50, "GAME.EXE");
    ACTOR_ANIMATION *playback = actor_animations + idx;
    const ANIMATION_STREAM *stream = &playback->src.stream;
    uint32 frame = playback->frame, count, header, next, offset, bits = 0, remaining = 0, i, angle_idx = 0;
    const MODEL_RIG_HEADER *rig;
    const uint8 *data, *flags, *vals, *end;
    if (playback->src.kind == ACTOR_ANIMATION_SOURCE_NONE)
        return;
    if (playback->src.kind != ACTOR_ANIMATION_SOURCE_STREAM || !stream->header || stream->byte_len < 4)
        diagnostic_stop(__FUNCTION__, __FILE__, __LINE__);
    ACTOR_FRAME_MOTION *motion = &playback->motion;
    if (!model.header || model.byte_len < offsetof(MODEL_RIG_HEADER, bone_count) + sizeof(rig->bone_count))
        diagnostic_stop(__FUNCTION__, __FILE__, __LINE__);
    rig = (const MODEL_RIG_HEADER *)model.header;
    count = rig->bone_count;
    count = add_checked(count << 1, count);
    next = add_checked(frame, 1);
    header = stream->header->frame_count;
    if (header > (stream->byte_len - 4) / 4 || frame >= header)
        diagnostic_stop(__FUNCTION__, __FILE__, __LINE__);
    playback->frame = header == next ? 0xffffffff : next;
    header = stream->header->frame[frame];
    motion->events = header >> 16;
    offset = (header & 65535) + 4;
    if (offset > stream->byte_len || stream->byte_len - offset < 13)
        diagnostic_stop(__FUNCTION__, __FILE__, __LINE__);
    data = (const uint8 *)stream->header + offset;
    end = (const uint8 *)stream->header + stream->byte_len;
    for (i = 0; i < 3; i++)
    {
        uint32 val;
        memcpy(&val, data, sizeof(val));
        data += sizeof(val);
        motion->bits[i] = val;
    }
    offset = *data++;
    flags = data;
    if (offset > (uint32)(end - data))
        diagnostic_stop(__FUNCTION__, __FILE__, __LINE__);
    vals = data + offset;
    do
    {
        uint32 changed = read_bit(&flags, end, &bits, &remaining);
        if (angle_idx >= ACTOR_POSE_BONE_COUNT * 3)
            diagnostic_stop(__FUNCTION__, __FILE__, __LINE__);
        sint16 *angle = &pose->angles[angle_idx / 3].v[angle_idx % 3];
        if (!frame)
        {
            uint16 val = 0;
            if (changed)
            {
                if ((uint32)(end - vals) < sizeof(val))
                    diagnostic_stop(__FUNCTION__, __FILE__, __LINE__);
                memcpy(&val, vals, sizeof(val));
                vals += sizeof(val);
            }
            count = add_checked(count, 0xffffffff);
            *angle = (sint16)val;
        }
        else
        {
            if (changed)
            {
                sint32 prev = *angle;
                uint32 wide = read_bit(&flags, end, &bits, &remaining);
                if (vals >= end)
                    diagnostic_stop(__FUNCTION__, __FILE__, __LINE__);
                sint32 delta = (sint8)*vals++;
                if (wide)
                {
                    if (vals >= end)
                        diagnostic_stop(__FUNCTION__, __FILE__, __LINE__);
                    delta = (uint8)delta + 256 * (sint32)(sint8)*vals++;
                }
                *angle = ((uint32)prev + (uint32)(delta * 2)) & 0xffe;
            }
            count = add_checked(count, 0xffffffff);
        }
        angle_idx++;
    } while (count);
}

GDB_CALL void actor_attachment_pos(sint32 idx, sint32 attachment, uint32 output[3])
{
    FUNCTION_MARKER(0x8001b9f4, "GAME.EXE");
    ACTOR_POSE *pose;
    const sint32 *pos;
    uint32 x, y, z;
    FIXED_VEC3 point;
    int i;
    transform_push();
    transform_identity();
    transform_yaw(-camera.angles.yaw);
    transform_pitch(-camera.angles.pitch);
    if ((uint32)idx >= ACTOR_POSE_ACTOR_COUNT)
        diagnostic_stop(__FUNCTION__, __FILE__, __LINE__);
    pose = &actor_poses[idx];
    if (attachment == 2)
    {
        transform_translate(0, -786432, 0);
        pos = pose->matrix[0].t;
    }
    else if (attachment == 3 || attachment == 11)
    {
        transform_translate(attachment == 11 ? -2097152 : 0, -786432, 0);
        pos = pose->matrix[1].t;
    }
    else if (attachment >= 4 && attachment <= 9)
        pos = pose->point[attachment - 4].v;
    else if (attachment == 1)
        pos = pose->point[6].v;
    else
    {
        diagnostic_stop(__FUNCTION__, __FILE__, __LINE__);
        return;
    }
    x = pos[0];
    y = pos[1];
    z = pos[2];
    transform_point(x, y, z, point.bits);
    for (i = 0; i < 3; i++)
        output[i] = camera.pos.bits[i] + point.bits[i];
    transform_pop();
}

/* Place the computed attachment in its caller-owned Q16 position */
GDB_CALL uint32 actor_place_attachment(FIXED_VEC3 *pos, sint32 idx, sint32 attachment)
{
    FUNCTION_MARKER(0x80017f88, "GAME.EXE");
    FIXED_VEC3 point;
    actor_attachment_pos(idx, attachment, point.bits);
    /* TODO: Investigate missing attachment destinations at the first output store */
    if (!pos)
    {
        DIAGNOSTIC(fprintf(stderr, "Missing attachment destination actor %d attachment %d stage %u tick %u\n", idx, attachment, game_session.stage, game_session.tick));
        diagnostic_stop(__FUNCTION__, __FILE__, __LINE__);
    }
    pos->x.bits = point.x.bits;
    pos->y.bits = point.y.bits;
    pos->z.bits = point.z.bits;
    return point.z.bits;
}

/* Preserve four-word staging even when diagnostic native views overlap */
void actor_pose_copy(ACTOR_POSE *to, const ACTOR_POSE *from)
{
    uint32 i, a, b, c, d;
    for (i = 0; i < 496; i += 16)
    {
        memcpy(&a, (const uint8 *)from + i, sizeof(a));
        memcpy(&b, (const uint8 *)from + i + 4, sizeof(b));
        memcpy(&c, (const uint8 *)from + i + 8, sizeof(c));
        memcpy(&d, (const uint8 *)from + i + 12, sizeof(d));
        memcpy((uint8 *)to + i, &a, sizeof(a));
        memcpy((uint8 *)to + i + 4, &b, sizeof(b));
        memcpy((uint8 *)to + i + 8, &c, sizeof(c));
        memcpy((uint8 *)to + i + 12, &d, sizeof(d));
    }
    memcpy(&a, (const uint8 *)from + 496, sizeof(a));
    memcpy(&b, (const uint8 *)from + 500, sizeof(b));
    memcpy((uint8 *)to + 496, &a, sizeof(a));
    memcpy((uint8 *)to + 500, &b, sizeof(b));
}
