#include "game.h"

/* Original initialized mode is two; segment storage belongs to BSS */
SUSPENDED suspended = {2};

GDB_CALL uint32 suspended_update(MODEL_OBJECT *model)
{
    FUNCTION_MARKER(0x80044d90u, "GAME.EXE");
    uint32 x, y, total = 0, offset, phase;
    sint32 floor, angle, drag, wave, amplitude, actor_idx;
    int i, above = 0;
    if (suspended.mode == 2)
    {
        for (i = 0; i < SUSPENDED_SEGMENT_COUNT; ++i)
        {
            SUSPENDED_SEGMENT *seg = suspended.segs + i;
            seg->angle = (sint16)((random_next() & 4095) + 12288);
            seg->angular_velocity = 0;
            seg->x = 0;
            seg->y = 0;
        }
        /* TODO: Investigate a suspended object without a native position */
        XASSERT(model);
        model->pos.y.bits = 0xf4000000;
        /* TODO: Investigate a missing or foreign native suspended owner */
        XASSERT(model && model->payload_kind == MODEL_PAYLOAD_SUSPENDED);
        model->suspended.phase = 0;
        suspended.mode = 3;
    }
    if (!suspended.mode)
        return 0;
    /* TODO: Investigate a suspended object without a native position */
    XASSERT(model);
    x = (uint16)model->pos.x.whole;
    y = (uint16)model->pos.y.whole;
    /* TODO: Investigate a missing or foreign native suspended owner */
    XASSERT(model && model->payload_kind == MODEL_PAYLOAD_SUSPENDED);
    actor_idx = model->suspended.actor_idx;
    /* TODO: Investigate an invalid actor retained by the suspended object */
    XASSERT(actor_idx >= 0 && actor_idx < ACTOR_COUNT);
    floor = actor_positions[actor_idx].y.whole;
    for (i = 0; i < SUSPENDED_SEGMENT_COUNT; ++i)
    {
        SUSPENDED_SEGMENT *seg = suspended.segs + i;
        if (!model->suspended.phase)
        {
            seg->angle = (sint16)((uint16)seg->angle + (uint16)seg->angular_velocity);
            if ((sint16)y < floor)
            {
                seg->angular_velocity = 0;
                seg->angle = 0;
            }
            phase = (uint16)seg->angle;
            angle = (sint16)phase;
            if (angle < 0)
                angle = -angle;
            drag = (seg->angular_velocity * (32768 - angle)) / 262144;
            wave = (96 * sin_table[phase >> 5]) / 8192;
            seg->angular_velocity = (sint16)((uint16)seg->angular_velocity - (uint32)(drag + wave));
            amplitude = (sint32)total < 0 ? total + 196608u : 196608u - total;
            if (amplitude <= 0)
                seg->angular_velocity = 0;
            else
                seg->angular_velocity = (sint16)((sint32)((seg->angular_velocity) * (uint32)amplitude) / 196608);
            total += (uint32)(sint32)seg->angle;
            offset = ((sint32)total >> 4) & 0xffeu;
            x += (uint32)((3 * sin_table[offset >> 1]) >> 10);
            y += (uint32)((3 * sin_table[MATH_QUARTER_TURN + (offset >> 1)]) >> 10);
            seg->x = (sint16)x;
            seg->y = (sint16)y;
        }
        else if (model->suspended.phase == 1)
            seg->angle = 0;
        else
        {
            seg->y = (sint16)((uint16)seg->y - (uint16)seg->fall_speed);
            seg->fall_speed = (sint16)((uint16)seg->fall_speed + 4);
            if (seg->fall_speed >= 65)
                seg->fall_speed = 64;
            angle = model->pos.y.whole;
            if (seg->y < angle)
                seg->y = (sint16)angle;
            else
                ++above;
        }
    }
    phase = model->suspended.phase;
    if (phase == 2 && !above)
    {
        model->kind = 0xffffffff;
        phase = model->suspended.phase;
    }
    if (phase == 1)
        model->suspended.phase = 2;
    return 2;
}

GDB_CALL void suspended_draw(const MODEL_OBJECT *model)
{
    FUNCTION_MARKER(0x800450f8u, "GAME.EXE");
    PROJECTION_POINT pos;
    sint32 x, y, z;
    uint32 i, camera_x, camera_y, camera_z;
    transform_push();
    /* TODO: Investigate a suspended object without a native position */
    XASSERT(model);
    z = model->pos.z.whole;
    camera_x = camera.pos.x.bits;
    camera_y = camera.pos.y.bits;
    camera_z = camera.pos.z.bits;
    transform_translate((uint32)(-(sint64)(camera_x)), (uint32)(-(sint64)(camera_y)), (uint32)(-(sint64)(camera_z)));
    x = model->pos.x.whole;
    y = model->pos.y.whole;
    proj_local(x, y, z, &pos);
    for (i = 0; i < SUSPENDED_SEGMENT_COUNT; i++)
        suspended_noop();
    transform_pop();
}

GDB_CALL void suspended_noop(void)
{
    FUNCTION_MARKER(0x80044d88u, "GAME.EXE");
}
