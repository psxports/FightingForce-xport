#include <stddef.h>

#include "game.h"

CAMERA camera;
CAMERA_STAGE_REFERENCE camera_stage_refs[CAMERA_STAGE_COUNT];
const CAMERA camera_defaults = {.motion = {.phase = 1024}};
static const FIXED_VEC3 orbit_center = {.bits = {0, 0, 0xf3000000}};
const FIXED_VEC3 camera_menu_dst = {.bits = {0, 0xff800000, 0xef800000}};
const FIXED_VEC3 camera_options_dst = {.bits = {0, 0xf6000000, 0xf0800000}};

/* Q16 and halfword views have an explicit little-endian binary boundary */
_Static_assert(sizeof(FIXED_COORD) == 4, "Q16 coordinate size");
_Static_assert(offsetof(FIXED_COORD, whole) == 2, "Q16 integer half");
_Static_assert(sizeof(FIXED_VEC3) == 12, "Q16 vector size");
_Static_assert(sizeof(CAMERA_ANGLES) == 4, "Camera angle size");
_Static_assert(sizeof(FIXED_XZ) == 8, "Ground-plane center size");
_Static_assert(offsetof(CAMERA_MOTION, phase) == 48, "Camera phase wire offset");

sint32 camera_update(void)
{
    FUNCTION_MARKER(0x800474b8, "GAME.EXE");
    uint32 target_x = camera.target.x.bits;
    uint32 x = camera.pos.x.bits;
    uint32 vx = camera.velocity.x.bits;
    uint32 target_y = camera.target.y.bits;
    uint32 y = camera.pos.y.bits;
    uint32 target_z = camera.target.z.bits;
    uint32 z = camera.pos.z.bits;
    uint32 vy = camera.velocity.y.bits;
    uint32 vz = camera.velocity.z.bits;
    uint32 target_yaw, yaw, target_pitch, pitch;
    sint32 result;
    vx += (uint32)((sint32)(target_x - x - vx) >> 1);
    camera.velocity.x.bits = vx;
    x += (uint32)((sint32)vx >> 2);
    vy += (uint32)((sint32)(target_y - y - vy) >> 4);
    camera.velocity.y.bits = vy;
    y += (uint32)((sint32)vy >> 5);
    target_yaw = camera.target_angles.yaw_bits;
    yaw = camera.angles.yaw_bits;
    vz += (uint32)((sint32)(target_z - z - vz) >> 1);
    camera.velocity.z.bits = vz;
    z += (uint32)((sint32)vz >> 2);
    target_pitch = camera.target_angles.pitch_bits;
    pitch = camera.angles.pitch_bits;
    camera.pos.x.bits = x;
    camera.scaled_pos.x = (sint32)((uint32)(-(sint64)(x))) >> 12;
    camera.pos.y.bits = y;
    camera.pos.z.bits = z;
    camera.scaled_pos.y = (sint32)y >> 12;
    camera.scaled_pos.z = (sint32)((uint32)(-(sint64)(z))) >> 12;
    yaw += (uint32)((sint32)((target_yaw - yaw) << 16) >> 19);
    result = (sint32)((target_pitch - pitch + 15) << 16) >> 21;
    camera.angles.yaw_bits = (uint16)yaw;
    camera.angles.pitch_bits = (uint16)(pitch + result);
    return result;
}

sint32 camera_update_target(void)
{
    FUNCTION_MARKER(0x800476e8, "GAME.EXE");
    uint32 radius = camera.tracking.radius + 1280;
    uint32 angle = camera.angles.yaw_bits >> 5;
    uint32 diff, bit, x, z, bearing;
    sint32 i;
    sint32 timer;
    sint32 override;
    uint32 s;
    uint32 c;
    sint32 player_x;
    sint32 player_z;
    sint32 actor_x;
    sint32 actor_z;
    int raised;
    if (game_session.stage != 14 && game_session.player_count == 1 && pad_action.latch[0][4])
        radius = 2048;
    s = sin_table[angle];
    c = sin_table[MATH_QUARTER_TURN + angle];
    z = camera.tracking.center.z.bits;
    x = camera.tracking.center.x.bits;
    camera.target.x.bits = x + s * radius;
    camera.target.z.bits = z - c * radius;
    for (i = 2; i < 8; i++)
    {
        bit = 1 << i;
        if (actor_is_defeated(i))
        {
            camera.tracking.visibility_mask = camera.tracking.visibility_mask & ~bit;
            continue;
        }
        player_x = actor_positions[0].x.whole;
        player_z = actor_positions[0].z.whole;
        actor_x = actor_positions[i].x.whole;
        actor_z = actor_positions[i].z.whole;
        bearing = math_dir16(player_x, player_z, actor_x, actor_z);
        diff = bearing - camera.angles.yaw_bits;
        if (dir_distance < 384 && ((uint16)(diff + 6143) < 12287 || (uint16)(diff + 26624) > 53248))
            camera.tracking.visibility_mask = camera.tracking.visibility_mask | bit;
        if (dir_distance >= 513 || (uint16)(diff - 7169) < 18431 || (uint16)(diff + 25599) < 18431)
            camera.tracking.visibility_mask = camera.tracking.visibility_mask & ~bit;
    }
    if (camera.tracking.visibility_mask)
    {
        camera.tracking.hold_bits = 60;
        raised = 1;
    }
    else
    {
        timer = camera.tracking.hold_bits;
        if (timer > 0)
        {
            camera.tracking.hold_bits = timer - 1u;
            raised = 1;
        }
        else
            raised = (sint32)camera.tracking.pitch_control_bits < 0;
    }
    if (raised)
    {
        if (game_session.stage == 14)
        {
            camera.target.y.bits = 0xfec00000;
            camera.target_angles.pitch_bits = 3584;
        }
        else
        {
            camera.target.y.bits = 0xfe400000;
            camera.target_angles.pitch_bits = 4608;
        }
    }
    else
    {
        camera.target.y.bits = 0xfec00000;
        camera.target_angles.pitch_bits = 3072;
    }
    override = camera.tracking.pitch_control_bits;
    if (override > 0)
    {
        camera.target.y.bits = 0xfec00000;
        camera.target_angles.pitch_bits = (uint16) override;
    }
    return 0xfec00000;
}

sint32 camera_capture(const uint32 *pos, sint32 yaw, sint32 pitch)
{
    FUNCTION_MARKER(0x8004f980, "GAME.EXE");
    uint16 oldyaw = camera.angles.yaw_bits, oldpitch = camera.angles.pitch_bits;
    uint32 a = camera.pos.x.bits, b = camera.pos.y.bits, c = camera.pos.z.bits, val;
    camera.motion.phase_bits = 128;
    camera.motion.origin_angles.yaw_bits = oldyaw;
    camera.motion.origin_angles.pitch_bits = oldpitch;
    camera.motion.origin.x.bits = a;
    camera.motion.origin.y.bits = b;
    camera.motion.origin.z.bits = c;
    camera.motion.delta_angles.yaw_bits = (uint16)((uint32)yaw - oldyaw);
    camera.motion.delta_angles.pitch_bits = (uint16)((uint32)pitch - oldpitch);
    val = pos[0];
    camera.motion.delta.x.bits = val - a;
    val = pos[1];
    camera.motion.delta.y.bits = val - (b - 6291456);
    val = pos[2] - c;
    camera.motion.delta.z.bits = val;
    return val;
}

sint32 camera_update_transition(void)
{
    FUNCTION_MARKER(0x8004f520, "GAME.EXE");
    sint32 phase = camera.motion.phase, result;
    sint32 target_yaw, yaw, target_pitch, pitch;
    uint32 camera_x, camera_y, camera_z, target_x, target_y, target_z;
    if (phase < 512)
    {
        sint32 s = sin_table[phase & 2047u];
        uint32 coefficient = s >> 3, delta_x, delta_y, delta_z;
        uint32 base_x;
        uint32 base_y;
        uint32 base_z;
        uint32 base_yaw;
        uint32 base_pitch;
        sint32 yaw_product;
        sint32 pitch_product;
        uint32 angle_delta;
        angle_delta = camera.motion.delta_angles.yaw;
        yaw_product = s * angle_delta;
        angle_delta = camera.motion.delta_angles.pitch;
        pitch_product = s * angle_delta;
        delta_x = camera.motion.delta.x.val >> 12;
        delta_y = camera.motion.delta.y.val >> 12;
        base_yaw = camera.motion.origin_angles.yaw_bits;
        base_pitch = camera.motion.origin_angles.pitch_bits;
        base_x = camera.motion.origin.x.bits;
        base_y = camera.motion.origin.y.bits;
        delta_z = camera.motion.delta.z.val >> 12;
        base_z = camera.motion.origin.z.bits;
        camera.motion.phase_bits = (uint16)(phase + 32);
        camera.motion.target.x.bits = base_x + coefficient * delta_x;
        camera.motion.target_angles.yaw_bits = (uint16)(base_yaw + (yaw_product >> 15));
        camera.motion.target_angles.pitch_bits = (uint16)(base_pitch + (pitch_product >> 15));
        camera.motion.target.y.bits = base_y + coefficient * delta_y;
        camera.motion.target.z.bits = base_z + coefficient * delta_z;
    }
    target_yaw = camera.motion.target_angles.yaw;
    yaw = camera.angles.yaw;
    target_pitch = camera.motion.target_angles.pitch;
    pitch = camera.angles.pitch;
    camera_x = camera.pos.x.bits;
    target_x = camera.motion.target.x.bits;
    camera.angles.yaw_bits = (uint16)(yaw + ((target_yaw - yaw) >> 4));
    target_y = camera.motion.target.y.bits;
    camera.angles.pitch_bits = (uint16)(pitch + ((target_pitch - pitch) >> 4));
    camera_y = camera.pos.y.bits;
    camera_z = camera.pos.z.bits;
    camera_x += (uint32)((sint32)(target_x - camera_x) >> 4);
    target_z = camera.motion.target.z.bits;
    camera_y += (uint32)((sint32)(target_y - camera_y) >> 4);
    camera.pos.x.bits = camera_x;
    camera.pos.y.bits = camera_y;
    result = (sint32)(target_z - camera_z) >> 4;
    camera.pos.z.bits = camera_z + result;
    return result;
}

sint32 camera_update_motion(uint32 x_speed)
{
    FUNCTION_MARKER(0x8004f6d0u, "GAME.EXE");
    uint32 speed, target, p, vals[5], result;
    int i;
    sint32 wanted, cur;
    speed = x_speed + 0x10000u;
    camera.motion.delta.x.bits = speed;
    if ((sint32)speed < -2097152)
        camera.motion.delta.x.bits = 0xffe00000;
    else if ((sint32)speed > 2097152)
        camera.motion.delta.x.bits = 0x200000;
    for (i = 0; i < 2; i++)
    {
        p = 4u * i;
        target = (sint32)(camera.motion.origin.bits[1 + i] - camera.pos.bits[1 + i]) >> 3;
        speed = camera.motion.delta.bits[1 + i];
        if ((sint32)speed < (sint32)(target - 0x10000u))
            speed += 0x10000u;
        else if ((sint32)(target + 0x10000u) < (sint32)speed)
            speed -= 0x10000u;
        else
            speed = target;
        camera.motion.delta.bits[1 + i] = speed;
        if ((sint32)speed < -2097152)
            camera.motion.delta.bits[1 + i] = 0xffe00000;
        else if ((sint32)speed > 2097152)
            camera.motion.delta.bits[1 + i] = 0x200000;
    }
    for (i = 0; i < 2; i++)
    {
        p = 2u * i;
        wanted = (camera.motion.origin_angles.v[i] - camera.angles.v[i]) >> 4;
        cur = camera.motion.delta_angles.v[i];
        if (cur < wanted - 128)
            camera.motion.delta_angles.bits[i] = (uint16)(cur + 128);
        cur = camera.motion.delta_angles.v[i];
        camera.motion.delta_angles.bits[i] = (uint16)(wanted + 128 < cur ? cur - 128 : wanted);
    }
    result = ((uint16)camera.angles.yaw) + (uint16)camera.motion.delta_angles.yaw;
    vals[0] = result;
    vals[1] = ((uint16)camera.angles.pitch) + (uint16)camera.motion.delta_angles.pitch;
    for (i = 0; i < 3; i++)
        vals[2 + i] = camera.pos.bits[i] + camera.motion.delta.bits[i];
    camera.angles.yaw_bits = (uint16)vals[0];
    camera.angles.pitch_bits = (uint16)vals[1];
    for (i = 0; i < 3; i++)
        camera.pos.bits[i] = vals[2 + i];
    return result;
}

sint32 camera_orbit(void)
{
    FUNCTION_MARKER(0x8004fa10, "GAME.EXE");
    sint32 i, n = character_menu.player_count;
    FIXED_VEC3 pos = {.bits = {0, 0, 0}};
    sint32 angle;
    uint32 centerx, centerz, idx;
    for (i = 0; i < n; i++)
    {
        uint32 actor = character_menu.sel[i];
        pos.x.bits += actor_positions[actor].x.bits;
        actor = character_menu.sel[i];
        pos.z.bits += actor_positions[actor].z.bits;
    }
    n = character_menu.player_count;
    pos.x.bits = math_divide(pos.x.bits, n);
    pos.z.bits = math_divide(pos.z.bits, n);
    centerx = orbit_center.x.bits;
    centerz = orbit_center.z.bits;
    angle = (math_dir16((sint32)pos.x.bits >> 16, (sint32)pos.z.bits >> 16, (sint32)centerx >> 16, (sint32)centerz >> 16) - 0x8000u) << 16;
    idx = angle >> 21;
    pos.x.bits = centerx + ((uint32)(sint32)sin_table[idx & 2047] << 11);
    pos.y.bits = 0xfef00000u;
    pos.z.bits = centerz - ((uint32)(sint32)sin_table[(idx + 512) & 2047] << 11);
    return camera_capture(pos.bits, angle >> 16, 0);
}

sint32 camera_update_focus(void)
{
    FUNCTION_MARKER(0x80047600, "GAME.EXE");
    uint32 mask = (actor_health[0].hp > 0 ? 1u : 0u) + (actor_health[1].hp > 0 ? 2u : 0u);
    sint32 result;
    camera.tracking.radius = 0;
    if (mask - 1u < 2u)
    {
        camera.tracking.center.x.bits = actor_positions[mask - 1u].x.bits;
        result = actor_positions[mask - 1u].z.bits;
        camera.tracking.center.z.bits = result;
        return result;
    }
    if (mask != 3)
        return 3;
    camera.tracking.center.x.bits = ((actor_positions[0].x.whole) + (uint32)(sint32)actor_positions[1].x.whole) << 15;
    camera.tracking.center.z.bits = ((actor_positions[0].z.whole) + (uint32)(sint32)actor_positions[1].z.whole) << 15;
    result = math_distance_approx(actor_positions[0].x.whole, actor_positions[0].z.whole, actor_positions[1].x.whole, actor_positions[1].z.whole);
    camera.tracking.radius = result;
    return result;
}

static sint32 update_min_z(uint32 ox, uint32 shift, uint32 min_z)
{
    camera.target_angles.yaw_bits = (uint16)((sint32)(ox - camera.tracking.center.x.bits) >> shift);
    camera_update_target();
    if ((sint32)camera.target.z.bits < (sint32)min_z)
        camera.target.z.bits = min_z;
    return camera_update();
}

static sint32 update_from(sint32 x, sint32 z)
{
    FUNCTION_MARKER(0x80027d68u, "GAME.EXE");
    sint32 camera_x = camera.tracking.center.x.whole, camera_z = camera.tracking.center.z.whole;
    camera.target_angles.yaw_bits = (uint16)math_dir16(x >> 16, z >> 16, camera_x, camera_z);
    camera_update_target();
    return camera_update();
}

static sint32 update_target_from(sint32 x, sint32 z)
{
    FUNCTION_MARKER(0x80027db4u, "GAME.EXE");
    sint32 camera_x = camera.tracking.center.x.whole, camera_z = camera.tracking.center.z.whole;
    camera.target_angles.yaw_bits = (uint16)math_dir16(x >> 16, z >> 16, camera_x, camera_z);
    return camera_update_target();
}

static sint32 follow_biker(void)
{
    FUNCTION_MARKER(0x80028c08u, "GAME.EXE");
    sint32 timer = camera.tracking.pitch_control_bits, x, z, camera_x, camera_z;
    uint32 i, type;
    if (timer > 0)
        camera.tracking.pitch_control_bits = timer - 1u;
    /* The last qualifying biker wins; the distance limit does not shrink */
    for (i = 0; i < DYNAMIC_OBJECT_COUNT; i++)
    {
        const MODEL_OBJECT *item = dynamic_objects + i;
        type = item->kind;
        if (type == 460 || type == 462)
        {
            x = item->pos.x.whole;
            z = item->pos.z.whole;
            camera_x = camera.tracking.center.x.whole;
            camera_z = camera.tracking.center.z.whole;
            if (math_distance_approx(x, z, camera_x, camera_z) < 32767)
            {
                camera.tracking.pitch_control_bits = 60;
                camera.tracking.biker_idx = i;
            }
        }
    }
    timer = camera.tracking.pitch_control_bits;
    if (timer <= 0)
        return timer;
    uint32 idx = camera.tracking.biker_idx;
    /* TODO: Investigate a retained biker index outside the native dynamic pool */
    XASSERT(idx < DYNAMIC_OBJECT_COUNT);
    const MODEL_OBJECT *item = dynamic_objects + idx;
    x = item->pos.x.val;
    z = item->pos.z.val;
    return update_target_from(x, z);
}

sint32 camera_update_stage0(void)
{
    FUNCTION_MARKER(0x80027DF8, "GAME.EXE");
    uint32 mode = game_session.scene_phase;
    sint32 x, z, camera_x, camera_z;
    if (!mode)
    {
        x = 2240;
        z = ((sint32)(((0xf9000000u - actor_positions[0].z.bits) << 1) + 0xf9000000u)) >> 16;
    }
    else if ((mode = game_session.scene_phase) == 1)
    {
        uint32 i;
        camera_x = camera.tracking.center.x.whole;
        camera_z = camera.tracking.center.z.whole;
        camera.target_angles.yaw_bits = (uint16)math_dir16(256, 3392, camera_x, camera_z);
        camera_update_target();
        if (players_past_z(-0x6000000) && level_wave.paused == 1)
        {
            game_session.scene_phase = game_session.scene_phase + 1u;
            level_wave.paused = 0;
            for (i = 0; i < DYNAMIC_OBJECT_COUNT; i++)
            {
                MODEL_OBJECT *item = dynamic_objects + i;
                if (item->kind != 0xffffffffu && item->pos.z.val < (sint32)0xf9000000u)
                    item->kind = 0xffffffffu;
            }
            col_regions.named[1]->col_kind = 3;
        }
        return camera_update();
    }
    else if (game_session.scene_phase == 2)
    {
        x = 256;
        z = -1792;
    }
    else if (level_transition.exit_requested)
    {
        x = 128;
        z = 3392;
    }
    else
    {
        x = 0;
        z = -384;
    }
    camera_x = camera.tracking.center.x.whole;
    camera_z = camera.tracking.center.z.whole;
    camera.target_angles.yaw_bits = (uint16)math_dir16(x, z, camera_x, camera_z);
    camera_update_target();
    return camera_update();
}

static sint32 update_stage1(void)
{
    FUNCTION_MARKER(0x80027fa4u, "GAME.EXE");
    sint32 z = game_session.scene_phase == 2 ? 256 : 768;
    sint32 camera_x = camera.tracking.center.x.whole, camera_z = camera.tracking.center.z.whole;
    uint32 angle = math_dir16(0, z, camera_x, camera_z) - 32768u;
    camera.target_angles.yaw_bits = (uint16)angle;
    camera_update_target();
    if ((sint32)camera.target.x.bits > 0x02000000)
        camera.target.x.bits = 0x02000000;
    if ((sint32)camera.target.x.bits < (sint32)0xfe000000u)
        camera.target.x.bits = 0xfe000000u;
    return camera_update();
}

static sint32 update_stage2(void)
{
    FUNCTION_MARKER(0x8002804cu, "GAME.EXE");
    return update_min_z(0, 12, 0xfc400000u);
}

static sint32 update_stage3(void)
{
    FUNCTION_MARKER(0x800280b0u, "GAME.EXE");
    return update_min_z(0, 12, 0xfa300000u);
}

static sint32 update_stage4(void)
{
    FUNCTION_MARKER(0x80028114u, "GAME.EXE");
    uint32 tick, players, z, limit, offset, product, x;
    sint32 result;
    camera.target_angles.yaw_bits = (uint16)((sint32)((uint32)(-(sint64)(camera.tracking.center.x.bits))) >> 12);
    camera_update_target();
    if (game_session.tick == 150)
        sound_voice_play(207, 255, (random_next() & 255u) + 5632u);
    tick = game_session.tick;
    if (tick - 61u < 179u)
    {
        camera.target.x.bits = 0;
        camera.target.y.bits = 0xff000000;
        camera.target.z.bits = 0x09000000;
        camera.target_angles.pitch_bits = 64512;
    }
    tick = game_session.tick;
    if (tick - 240u < 390u)
    {
        if ((sint32)tick < 361)
        {
            camera.target.x.bits = 0;
            camera.target.y.bits = 0xfe000000;
            camera.target.z.bits = 0x0a000000;
        }
        else
        {
            camera.target.x.bits = 0xff800000;
            camera.target.y.bits = 0xfec00000;
            camera.target.z.bits = 0x0c000000;
            camera.target_angles.pitch_bits = 1024;
        }
    }
    players = game_session.player_count;
    tick = game_session.tick;
    if ((sint32)tick < 660)
    {
        game_session.player_count = 0;
        camera.target_angles.yaw_bits = (sint32)tick < 240 ? 0 : 2048;
    }
    else
    {
        z = camera.target.z.bits;
        if ((sint32)z > 0x09c00000)
            limit = 0x09c00000;
        else
        {
            z = camera.target.z.bits;
            limit = (sint32)z <= 0x067fffff ? 0x06800000 : z;
        }
        if (limit != z)
        {
            offset = ((uint16)camera.target_angles.yaw >> 5) * 2u;
            product = (sin_table[offset >> 1]) * (uint32)((sint32)(z - limit) >> 15);
            x = camera.target.x.bits;
            camera.target.x.bits = x + product;
            camera.target.z.bits = limit;
        }
    }
    result = camera_update();
    game_session.player_count = players;
    return result;
}

static sint32 update_stage5(void)
{
    FUNCTION_MARKER(0x8002837cu, "GAME.EXE");
    const MODEL_OBJECT *item;
    sint32 angle = (sint32)(0xf9000000u - camera.tracking.center.x.bits) >> 13;
    sint32 x, z, camera_x, camera_z;
    camera.target_angles.yaw_bits = (uint16)angle;
    if ((sint16)angle > 14336)
        camera.target_angles.yaw_bits = 14336;
    if (camera.target_angles.yaw < -14336)
        camera.target_angles.yaw_bits = (uint16)-14336;
    /* TODO: Investigate an invalid scene object before access */
    if (level_script_scene.object.pool == SCENE_OBJECT_WORLD && level_script_scene.object.idx < WORLD_OBJECT_COUNT)
        item = world_objects.object + level_script_scene.object.idx;
    else if (level_script_scene.object.pool == SCENE_OBJECT_DYNAMIC && level_script_scene.object.idx < DYNAMIC_OBJECT_COUNT)
        item = dynamic_objects + level_script_scene.object.idx;
    else if (level_script_scene.object.pool == SCENE_OBJECT_NONE)
        item = NULL;
    else
    {
        XASSERT(0);
    }
    if (item)
    {
        x = item->pos.x.whole;
        z = item->pos.z.whole;
        camera_x = camera.tracking.center.x.whole;
        camera_z = camera.tracking.center.z.whole;
        camera.target_angles.yaw_bits = (uint16)math_dir16(x, z, camera_x, camera_z);
    }
    camera_update_target();
    if ((sint32)camera.target.x.bits < (sint32)0xee800000u)
        camera.target.x.bits = 0xee800000u;
    if (game_session.scene_phase == 3 && level_wave.paused == 1)
        camera.target_angles.pitch_bits = 1024;
    return camera_update();
}

static sint32 update_stage6(void)
{
    FUNCTION_MARKER(0x8002848cu, "GAME.EXE");
    camera.target_angles.yaw_bits = (uint16)((sint32)(0xfa000000u - camera.tracking.center.x.bits) >> 14);
    camera_update_target();
    return camera_update();
}

static sint32 update_stage7(void)
{
    FUNCTION_MARKER(0x800284d4u, "GAME.EXE");
    sint32 state = game_session.scene_phase, angle, x, limit;
    if (state >= 5)
        angle = 16384u - ((sint32)(camera.tracking.center.z.bits + 0x04800000u) >> 14);
    else
    {
        x = camera.tracking.center.x.bits;
        if (x <= 0x074fffff && (sint32)game_session.scene_phase >= 3)
            angle = 16384u - ((sint32)(camera.tracking.center.z.bits + 0x04800000u) >> 12);
        else
        {
            x = x - 0x0f000000u;
            if (x < (sint32)0xfab00000u)
                x = 0xfab00000u;
            if (x > 0x05500000)
                x = 0x05500000;
            angle = (sint32)((uint32)(-(sint64)((uint32)x))) >> 12;
            angle = angle - (uint32)((sint16)angle >> 2);
        }
    }
    camera.target_angles.yaw_bits = (uint16)angle;
    camera_update_target();
    state = game_session.scene_phase;
    if (state == 4 || state == 6)
    {
        limit = state == 4 ? 0x06430000 : (sint32)0xf2500000u;
        if ((sint32)camera.target.x.bits > limit)
            camera.target.x.bits = limit;
    }
    return camera_update();
}

static sint32 update_stage8(void)
{
    FUNCTION_MARKER(0x80028610u, "GAME.EXE");
    camera.target_angles.yaw_bits = (uint16)((sint32)(0xfa000000u - camera.tracking.center.x.bits) >> 13);
    camera_update_target();
    return camera_update();
}

static sint32 update_stage9(void)
{
    FUNCTION_MARKER(0x80028658u, "GAME.EXE");
    camera.target_angles.yaw_bits = (uint16)((sint32)(0xfe000000u - camera.tracking.center.x.bits) >> 13);
    camera_update_target();
    return camera_update();
}

static sint32 update_stage10(void)
{
    FUNCTION_MARKER(0x800286a0u, "GAME.EXE");
    return update_min_z(0, 13, 0xe1100000u);
}

static sint32 update_stage11(void)
{
    FUNCTION_MARKER(0x80028704u, "GAME.EXE");
    sint32 angle;
    uint32 excess;
    uint32 offset, x, z, product;
    if (!players_past_z(0x12000000))
        angle = (sint32)(0xfd000000u - camera.tracking.center.x.bits) >> 13;
    else if (players_before_x(-0x6000000))
        angle = 16384u - ((sint32)(camera.tracking.center.z.bits - 0x16800000u) >> 13);
    else if (players_past_x(0))
        angle = ((sint32)(camera.tracking.center.z.bits - 0x16800000u) >> 13) - 16384u;
    else
        angle = (sint32)(0xfd000000u - camera.tracking.center.x.bits) >> 13;
    camera.target_angles.yaw_bits = (uint16)angle;
    camera_update_target();
    x = camera.target.x.bits;
    if ((sint32)x > 0x03000000)
    {
        offset = 2u * (((camera.target_angles.yaw >> 5) + 512) & 2047u);
        product = sin_table[offset >> 1];
        excess = (sint32)(x - 0x03000000u) >> 15;
        product *= excess;
        z = camera.target.z.bits;
        camera.target.z.bits = z - product;
        camera.target.x.bits = 0x03000000;
    }
    camera.tracking.pitch_control_bits = camera.target.z.bits + 0x0c7fffffu <= 0x0aeffffeu ? 512 : 0;
    return camera_update();
}

static sint32 update_stage12(void)
{
    FUNCTION_MARKER(0x80028874u, "GAME.EXE");
    sint32 angle = (sint32)(0x13000000u - camera.tracking.center.x.bits) >> 13, x, camera_x, camera_z;
    camera.target_angles.yaw_bits = (uint16)(angle - ((sint16)angle >> 3));
    camera_update_target();
    if ((sint32)camera.target.x.bits < 0x10000000)
        camera.target.x.bits = 0x10000000;
    if ((sint32)camera.target.x.bits > 0x18000000)
        camera.target.x.bits = 0x18000000;
    if ((sint32)level_transition.route_exit_tick >= 2)
    {
        x = level_transition.route_choice ? 6144 : 4096;
        camera_x = camera.tracking.center.x.whole;
        camera_z = camera.tracking.center.z.whole;
        camera.target_angles.yaw_bits = (uint16)(math_dir16(x, 3584, camera_x, camera_z) - 32768u);
        camera.target.x.bits = (uint32)x << 16;
        camera.target.z.bits = 0x0e000000;
    }
    return camera_update();
}

static sint32 update_actor2(void)
{
    FUNCTION_MARKER(0x80028988u, "GAME.EXE");
    sint32 x = actor_positions[2].x.bits, z = actor_positions[2].z.bits;
    update_from(x, z);
    return camera_update();
}

static sint32 update_stage14(void)
{
    FUNCTION_MARKER(0x800289c0u, "GAME.EXE");
    sint32 angle;
    if (!(uint8)game_session.tick && !disp_buffers.shake_ticks)
        disp_buffers.shake_ticks = 8;
    angle = (sint32)((uint32)(-(sint64)(camera.tracking.center.x.bits))) >> 13;
    camera.target_angles.yaw_bits = (uint16)angle;
    angle += sin_table[((game_session.tick << 4) & 0xff0u) >> 1] >> 6;
    camera.target_angles.yaw_bits = (uint16)angle;
    camera_update_target();
    if ((sint32)camera.target.z.bits < (sint32)0xe0c00000u)
        camera.target.z.bits = 0xe0c00000u;
    camera.target.x.bits = 0;
    return camera_update();
}

static sint32 update_stage15(void)
{
    FUNCTION_MARKER(0x80028a94u, "GAME.EXE");
    sint32 angle;
    uint32 x;
    uint32 z;
    int target = level_script_scene.relative != 0;
    if (players_past_x(0x10000000) && (z = camera.tracking.center.z.bits) < (sint32)0xf6000000u)
    {
        angle = (sint32)(z + 0x0e000000u) >> 13;
        camera.target_angles.yaw_bits = (uint16)(angle - 16384u);
    }
    else if (target)
    {
        x = level_script_scene.ox;
        z = level_script_scene.oz;
        update_target_from(x << 16, z << 16);
        return camera_update();
    }
    else
    {
        angle = (sint32)(0x04000000u - camera.tracking.center.x.bits) >> 13;
        if (angle > 16384)
            angle = 16384;
        if (angle < -16384)
            angle = -16384;
        camera.target_angles.yaw_bits = (uint16)angle;
    }
    camera_update_target();
    return camera_update();
}

static sint32 update_stage16(void)
{
    FUNCTION_MARKER(0x80028ba0u, "GAME.EXE");
    return update_min_z(0xff000000u, 15, 0xea200000u);
}

static sint32 update_stage17(void)
{
    FUNCTION_MARKER(0x80028d08u, "GAME.EXE");
    sint32 volume = (sint32)(0xf0000000u - camera.tracking.center.z.bits) >> 18, result, pitch;
    uint32 i, wave, y;
    if (volume < 72)
        volume = 72;
    if (volume > 192)
        volume = 192;
    sound_voice_volume(stage_audio_slots[0].voice, volume);
    pitch = sin_table[((game_session.tick << 4) & 0xff0u) >> 1] >> 9;
    sound_voice_pitch(stage_audio_slots[0].voice, pitch + 6144);
    camera.target_angles.yaw_bits = (uint16)((sint32)((uint32)(-(sint64)(camera.tracking.center.x.bits))) >> 13);
    follow_biker();
    if ((sint32)camera.tracking.pitch_control_bits > 0)
    {
        if ((sint32)camera.target.z.bits < (sint32)0xe2500000u)
            camera.target.z.bits = 0xe2500000;
        return camera_update();
    }
    camera_update_target();
    wave = sin_table[((game_session.tick << 4) & 0xff0u) >> 1];
    y = camera.target.y.bits;
    camera.target.y.bits = y + (wave << 6);
    if ((sint32)camera.target.z.bits < (sint32)0xe2500000u)
        camera.target.z.bits = 0xe2500000;
    camera_update();
    if (game_session.scene_phase != 1)
        return 1;
    result = level_wave.paused;
    if (result != 1)
        return result;
    result = players_past_z(-0x11b80000);
    if (!result)
        return result;
    game_session.scene_phase = game_session.scene_phase + 1u;
    level_wave.paused = 0;
    for (i = 0; i < DYNAMIC_OBJECT_COUNT; i++)
    {
        MODEL_OBJECT *item = dynamic_objects + i;
        if (item->kind != 0xffffffffu && item->pos.z.val < (sint32)0xee300000u)
            item->kind = 0xffffffffu;
    }
    col_regions.named[1]->col_kind = 3;
    return sound_reverb_enable_channels();
}

static sint32 update_stage18(void)
{
    FUNCTION_MARKER(0x80028f30u, "GAME.EXE");
    int orbit = 1;
    sint32 x, z, camera_x, camera_z;
    uint32 wave, y;
    if (level_script_scene.relative)
    {
        x = level_script_scene.ox;
        z = level_script_scene.oz;
        camera_x = camera.tracking.center.x.whole;
        camera_z = camera.tracking.center.z.whole;
        camera.target_angles.yaw_bits = (uint16)math_dir16(x, z, camera_x, camera_z);
    }
    else
    {
        camera.target_angles.yaw_bits = (uint16)(0xffff8000u - ((sint32)camera.tracking.center.x.bits >> 13));
        follow_biker();
        if ((sint32)camera.tracking.pitch_control_bits > 0)
            orbit = 0;
    }
    if (orbit)
    {
        camera_update_target();
        wave = sin_table[((game_session.tick << 4) & 0xff0u) >> 1];
        y = camera.target.y.bits;
        camera.target.y.bits = y + (wave << 6);
    }
    if ((sint32)camera.target.z.bits > 0x0dc00000)
        camera.target.z.bits = 0x0dc00000;
    return camera_update();
}

static sint32 update_stage19(void)
{
    FUNCTION_MARKER(0x8002903cu, "GAME.EXE");
    camera.target.y.bits = 0xfec00000u;
    camera.target_angles.yaw_bits = (uint16)((sint32)(0xfe800000u - camera.tracking.center.x.bits) >> 13);
    camera_update_target();
    return camera_update();
}

static sint32 update_stage20(void)
{
    FUNCTION_MARKER(0x80029090u, "GAME.EXE");
    if (level_wave.paused == 1)
    {
        camera.target_angles.yaw_bits = (uint16)(0x4000u - ((sint32)(camera.tracking.center.z.bits + 0x05a00000u) >> 13));
        camera_update_target();
    }
    else
    {
        sint32 x = actor_positions[2].x.bits, z = actor_positions[2].z.bits;
        update_from(x, z);
    }
    return camera_update();
}

static sint32 update_stage21(void)
{
    FUNCTION_MARKER(0x80029110u, "GAME.EXE");
    sint32 x = actor_positions[2].x.bits, z = actor_positions[2].z.bits;
    update_from(x, z);
    x = camera.target.x.bits;
    if (x < (sint32)0xef900000u)
        camera.target.x.bits = 0xef900000u;
    else if ((sint32)camera.target.x.bits > 0x0ec00000)
        camera.target.x.bits = 0x0ec00000;
    return camera_update();
}

static sint32 update_stage22(void)
{
    FUNCTION_MARKER(0x80029180u, "GAME.EXE");
    return update_min_z(0xfe800000u, 13, 0xe6600000u);
}

static sint32 update_stage23(void)
{
    FUNCTION_MARKER(0x800291e8u, "GAME.EXE");
    camera.target_angles.yaw_bits = (uint16)((sint32)(0x04800000u - camera.tracking.center.x.bits) >> 13);
    follow_biker();
    if ((sint32)camera.tracking.pitch_control_bits <= 0)
        camera_update_target();
    if ((sint32)camera.target.z.bits < (sint32)0xf4c00000u)
        camera.target.z.bits = 0xf4c00000u;
    return camera_update();
}

static sint32 update_stage24(void)
{
    FUNCTION_MARKER(0x8002926cu, "GAME.EXE");
    sint32 x = actor_positions[2].x.bits, z = actor_positions[2].z.bits;
    update_from(x, z);
    return camera_update();
}

static sint32 update_frontend(void)
{
    FUNCTION_MARKER(0x80040a48u, "GAME.EXE");
    camera.target_angles.pitch_bits = 0;
    camera.target.y.bits = 0xfdc00000u;
    camera_update_target();
    camera.target_angles.yaw_bits = 0;
    camera.target.x.bits = 0xfc000000u;
    return camera_update();
}

/* IDs preserve distinct original callbacks under identical code folding */
const CAMERA_STAGE_HANDLER camera_stage_handlers[CAMERA_HANDLER_COUNT] = {
    {camera_update_stage0, CAMERA_HANDLER_STAGE0}, {update_stage1, CAMERA_HANDLER_STAGE1},   {update_stage2, CAMERA_HANDLER_STAGE2},   {update_stage3, CAMERA_HANDLER_STAGE3},   {update_stage4, CAMERA_HANDLER_STAGE4},   {update_stage5, CAMERA_HANDLER_STAGE5},   {update_stage6, CAMERA_HANDLER_STAGE6},   {update_stage7, CAMERA_HANDLER_STAGE7},   {update_stage8, CAMERA_HANDLER_STAGE8},   {update_stage9, CAMERA_HANDLER_STAGE9},   {update_stage10, CAMERA_HANDLER_STAGE10}, {update_stage11, CAMERA_HANDLER_STAGE11}, {update_stage12, CAMERA_HANDLER_STAGE12},
    {update_actor2, CAMERA_HANDLER_ACTOR2},        {update_stage14, CAMERA_HANDLER_STAGE14}, {update_stage15, CAMERA_HANDLER_STAGE15}, {update_stage16, CAMERA_HANDLER_STAGE16}, {update_stage17, CAMERA_HANDLER_STAGE17}, {update_stage18, CAMERA_HANDLER_STAGE18}, {update_stage19, CAMERA_HANDLER_STAGE19}, {update_stage20, CAMERA_HANDLER_STAGE20}, {update_stage21, CAMERA_HANDLER_STAGE21}, {update_stage22, CAMERA_HANDLER_STAGE22}, {update_stage23, CAMERA_HANDLER_STAGE23}, {update_stage24, CAMERA_HANDLER_STAGE24}, {update_frontend, CAMERA_HANDLER_FRONTEND},
};

sint32 camera_update_stage(CAMERA_STAGE_REFERENCE ref)
{
    XASSERT(ref);
    return ref->update();
}
