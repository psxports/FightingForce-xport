#include "game.h"

STAGE_ANIMATION stage_animation;

void stage_animation_update_stage14(void)
{
    FUNCTION_MARKER(0x8001da84u, "GAME.EXE");
    static const uint32 limits[3] = {0xed180000, 0xfa610000, 0x07c10000};
    uint32 state = game_session.scene_phase, next, val;
    COLLISION_REGION *region, *region0, *region1, *region2;
    sint32 step0 = -2;
    sint32 step1 = -2;
    uint32 z;
    int i, sound = 0;
    if (state < 3)
    {
        if ((uint8)level_wave.paused)
            col_regions.named[5 + state]->col_kind = 0;
        if (players_past_z(limits[state]))
        {
            next = game_session.scene_phase + 1;
            region = col_regions.named[2 + state];
            level_wave.paused = 0;
            game_session.scene_phase = next;
            region->col_kind = 3;
        }
    }
    for (i = 0; i < 8; ++i)
        if (actor_health[i].hp > 0)
        {
            z = actor_positions[i].z.whole;
            if (z + 4862u < 894u || z - 1986u < 958u)
                step0 = 2;
            if (z + 1438u < 926u)
                step1 = 2;
        }
    val = stage_animation.phase[0];
    if ((!val && step0 > 0) || (val == 64 && step0 < 0))
        sound = 2;
    val = stage_animation.phase[0] + step0;
    stage_animation.phase[0] = val;
    if ((sint32)val < 0)
        stage_animation.phase[0] = 0;
    if ((sint32)stage_animation.phase[0] >= 65)
        stage_animation.phase[0] = 64;
    val = stage_animation.phase[1];
    if ((!val && step1 > 0) || (val == 64 && step1 < 0))
        sound = 1;
    val = stage_animation.phase[1] + step1;
    stage_animation.phase[1] = val;
    if ((sint32)val < 0)
        stage_animation.phase[1] = 0;
    if ((sint32)stage_animation.phase[1] >= 65)
        stage_animation.phase[1] = 64;
    if (sound)
        sound_voice_play(124, 255, 5888);
    stage_animation_scroll_stage14();
    val = stage_animation.phase[0];
    region0 = col_regions.named[8];
    region1 = col_regions.named[10];
    next = stage_animation.phase[1];
    region1->col_kind = val == 64 ? 0 : 3;
    region0->col_kind = val == 64 ? 0 : 3;
    region2 = col_regions.named[9];
    region2->col_kind = next == 64 ? 0 : 3;
}

GDB_CALL uint32 stage_animation_update_stage4(void)
{
    FUNCTION_MARKER(0x8001ddbcu, "GAME.EXE");
    uint32 kind, x, y, r0, r1, result;
    int row, i;
    if (game_session.tick == 210)
    {
        stage_animation.stage4_voice = (uint16)sound_voice_play(130, 255, 6144);
        disp_buffers.shake_ticks = 16;
        disp_buffers.flash_ticks = 4;
        sound_voice_play(19, 255, 6144);
        sound_voice_play(127, 255, 6400);
        world_objects.object[0].kind++;
        row = -168;
        do
        {
            y = (uint32)row << 16;
            kind = (random_next() & 3) + 679;
            dynamic_objects_spawn_oriented(kind, 0xfd800000, y, 0x0dc80000, 0);
            row -= 336;
            kind = (random_next() & 3) + 679;
            dynamic_objects_spawn_oriented(kind, 0xfe800000, y, 0x0dc80000, 0);
            kind = (random_next() & 3) + 679;
            dynamic_objects_spawn_oriented(kind, 0xff800000, y, 0x0dc80000, 0);
            kind = (random_next() & 3) + 679;
            dynamic_objects_spawn_oriented(kind, 0x00800000, y, 0x0dc80000, 0);
            kind = (random_next() & 3) + 679;
            dynamic_objects_spawn_oriented(kind, 0x01800000, y, 0x0dc80000, 0);
            kind = (random_next() & 3) + 679;
            dynamic_objects_spawn_oriented(kind, 0x02800000, y, 0x0dc80000, 0);
        } while (row >= -840);
        i = 0;
        do
        {
            ++i;
            r0 = random_next();
            r1 = random_next();
            x = (r0 & 127) + (r1 & 63) - 96;
            r0 = random_next();
            r1 = random_next();
            y = ((uint32)(-(sint64)(r0 & 511)) - (r1 & 255)) << 16;
            effect_spawn_point(x << 19, y, 0x0dc80000, x & 2047);
        } while (i < 96);
        i = 0;
        do
        {
            ++i;
            kind = (sint32)random_next() % 3 + 675;
            x = ((random_next() & 31) - 16) << 19;
            r0 = random_next();
            dynamic_objects_spawn(kind, x, 0xff800000, ((r0 & 15) + 384) << 19);
        } while (i < 16);
        i = 0;
        do
        {
            ++i;
            r0 = random_next();
            r1 = random_next();
            x = (r0 & 127) + (r1 & 63) - 96;
            kind = (sint32)random_next() % 3 + 675;
            y = ((uint32)(-(sint64)(random_next() & 511))) << 16;
            dynamic_objects_spawn(kind, x << 19, y, 0x08000000);
        } while (i < 16);
    }
    if ((sint32)game_session.tick < 211)
        return 1;
    result = random_next() & 31;
    if (result)
        return result;
    kind = (sint32)random_next() % 3 + 675;
    r0 = random_next();
    r1 = random_next();
    x = ((r0 & 127) + (r1 & 63) - 96) << 19;
    y = ((uint32)(-(sint64)(random_next() & 511))) << 16;
    dynamic_objects_spawn(kind, x, y, 0x08000000);
    kind = (random_next() & 1) + 128;
    result = (random_next() & 511) + 5888;
    return sound_voice_play(kind, 255, result);
}

static uint32 draw_slide_x(const MODEL_OBJECT *item)
{
    FUNCTION_MARKER(0x80032aa4u, "GAME.EXE");
    uint32 offset = stage_animation.phase[0] << 3;
    /* TODO: Investigate a model callback outside its native object domain */
    XASSERT(item);
    if (item->kind == 467)
        offset = (uint32)(-(sint64)(offset));
    transform_translate(offset << 16, 0, 0);
    return model_draw_static(model_refs[item->kind], 0, 256);
}

static uint32 draw_slide_z(const MODEL_OBJECT *item)
{
    FUNCTION_MARKER(0x80032b20u, "GAME.EXE");
    uint32 offset = stage_animation.phase[0] << 3;
    /* TODO: Investigate a model callback outside its native object domain */
    XASSERT(item);
    if (item->kind == 470)
        offset = (uint32)(-(sint64)(offset));
    transform_translate(0, 0, offset << 16);
    return model_draw_static(model_refs[item->kind], 0, 256);
}

static uint32 draw_double_slide_z(const MODEL_OBJECT *item)
{
    FUNCTION_MARKER(0x80032b98u, "GAME.EXE");
    uint32 offset = stage_animation.phase[1];
    transform_push();
    offset <<= 19;
    transform_translate(0, 0, offset);
    /* TODO: Investigate a model callback outside its native object domain */
    XASSERT(item);
    model_draw_static(model_refs[item->kind], 0, 256);
    transform_pop();
    transform_yaw(32768);
    transform_translate(0, 0, offset);
    return model_draw_static(model_refs[item->kind], 0, 256);
}

static uint32 scroll_four_uv(sint16 tex0, const sint16 *table)
{
    sint32 tex_idx = (sint32)(table - tex_map) + tex0;
    XASSERT(tex_idx >= 0 && tex_idx < TEXTURE_MAP_COUNT);
    uint32 idx = (uint32)tex_map[tex_idx];
    XASSERT(idx <= TEXTURE_DESCRIPTOR_CAPACITY - 4);
    TEXTURE_DESCRIPTOR *uv = tex_desc.item + idx;
    uint32 i, val, offset;
    for (i = 0; i < 4; i++, uv++)
    {
        offset = (uint32)(sint32)tex_base_v[i];
        val = offset + stage_animation.phase[0];
        uv->uv1.v = (uint8)val;
        uv->uv0.v = (uint8)val;
        val += 31;
        uv->uv3.v = (uint8)val;
        uv->uv2.v = (uint8)val;
    }
    return 0;
}

/* Texture V windows retain byte wrapping */
GDB_CALL uint32 stage_animation_scroll_stage0(void)
{
    FUNCTION_MARKER(0x800580ac, "GAME.EXE");
    sint32 map_idx = tex_grps[70].first_tex + 6;
    XASSERT(map_idx >= 0 && map_idx < TEXTURE_MAP_COUNT);
    sint32 desc_idx = tex_map[map_idx];
    XASSERT(desc_idx >= 0 && desc_idx <= TEXTURE_DESCRIPTOR_CAPACITY - 2);
    TEXTURE_DESCRIPTOR *uv = tex_desc.item + desc_idx;
    uint32 i, v;
    for (i = 0; i < 2; i++, uv++)
    {
        v = tex_base_v[i];
        v += stage_animation.phase[0];
        uv->uv1.v = (uint8)v;
        uv->uv0.v = (uint8)v;
        uv->uv3.v = (uint8)(v + 31);
        uv->uv2.v = (uint8)(v + 31);
    }
    return 0;
}

GDB_CALL uint32 stage_animation_scroll_stage1(void)
{
    FUNCTION_MARKER(0x8005813cu, "GAME.EXE");
    sint32 map_idx = tex_grps[8].first_tex + 62;
    XASSERT(map_idx >= 0 && map_idx < TEXTURE_MAP_COUNT);
    sint32 desc_idx = tex_map[map_idx];
    XASSERT(desc_idx >= 0 && desc_idx < TEXTURE_DESCRIPTOR_CAPACITY);
    TEXTURE_DESCRIPTOR *uv = tex_desc.item + desc_idx;
    uint32 val = (tex_base_v[0]) + stage_animation.phase[0];
    uv->uv1.v = (uint8)val;
    uv->uv0.v = (uint8)val;
    val += 31;
    uv->uv3.v = (uint8)val;
    uv->uv2.v = (uint8)val;
    return val;
}

GDB_CALL uint32 stage_animation_scroll_lift(void)
{
    FUNCTION_MARKER(0x800581a8u, "GAME.EXE");
    return scroll_four_uv(tex_grps[9].first_tex, tex_map + 40);
}

GDB_CALL uint32 stage_animation_scroll_stage12(void)
{
    FUNCTION_MARKER(0x800582e4u, "GAME.EXE");
    return scroll_four_uv(tex_grps[70].first_tex, tex_map + 136);
}

GDB_CALL sint32 stage_animation_scroll_stage14(void)
{
    FUNCTION_MARKER(0x80058374u, "GAME.EXE");
    sint32 tex0 = tex_grps[20].first_tex;
    sint32 src_map = tex0 + 144, dst_map = tex0 + 152;
    XASSERT(src_map >= 0 && src_map < TEXTURE_MAP_COUNT && dst_map >= 0 && dst_map < TEXTURE_MAP_COUNT);
    sint32 src_idx = tex_map[src_map], dst_idx = tex_map[dst_map];
    XASSERT(src_idx >= 0 && src_idx <= TEXTURE_DESCRIPTOR_CAPACITY - 6 && dst_idx >= 0 && dst_idx <= TEXTURE_DESCRIPTOR_CAPACITY - 6);
    TEXTURE_DESCRIPTOR *src = tex_desc.item + src_idx, *dst = tex_desc.item + dst_idx;
    uint32 v[4], phase;
    int i, j;
    /* Four load-before-store chunks preserve overlap behavior of both MIPS copy paths */
    for (i = 0; i < 4; i++)
    {
        for (j = 0; j < 4; j++)
            v[j] = tex_desc.words[3 * src_idx + 4 * i + j];
        for (j = 0; j < 4; j++)
            tex_desc.words[3 * dst_idx + 4 * i + j] = v[j];
    }
    /* Aligned LWL/LWR and SWL/SWR pairs each access the full word twice */
    v[0] = tex_desc.words[3 * src_idx + 16];
    v[0] = tex_desc.words[3 * src_idx + 16];
    v[1] = tex_desc.words[3 * src_idx + 17];
    v[1] = tex_desc.words[3 * src_idx + 17];
    tex_desc.words[3 * dst_idx + 16] = v[0];
    tex_desc.words[3 * dst_idx + 16] = v[0];
    tex_desc.words[3 * dst_idx + 17] = v[1];
    tex_desc.words[3 * dst_idx + 17] = v[1];
    for (i = 0; i < 6; i++, src++, dst++)
    {
        phase = (tex_base_v[i]) + stage_animation.phase[i < 3 ? 0 : 1];
        src->uv1.v = (uint8)phase;
        src->uv0.v = (uint8)phase;
        src->uv3.v = (uint8)(phase + 31u);
        src->uv2.v = (uint8)(phase + 31u);
        phase += 32u;
        dst->uv1.v = (uint8)phase;
        dst->uv0.v = (uint8)phase;
        dst->uv3.v = (uint8)(phase + 31u);
        dst->uv2.v = (uint8)(phase + 31u);
    }
    return 0;
}

GDB_CALL sint32 stage_animation_scroll_stage17(void)
{
    FUNCTION_MARKER(0x80058524u, "GAME.EXE");
    sint32 tex0 = tex_grps[23].first_tex;
    sint32 src_map = tex0 + 92, dst_map = tex0 + 100;
    XASSERT(src_map >= 0 && src_map < TEXTURE_MAP_COUNT && dst_map >= 0 && dst_map < TEXTURE_MAP_COUNT);
    sint32 src_idx = tex_map[src_map], dst_idx = tex_map[dst_map];
    XASSERT(src_idx >= 0 && src_idx <= TEXTURE_DESCRIPTOR_CAPACITY - 2 && dst_idx >= 0 && dst_idx <= TEXTURE_DESCRIPTOR_CAPACITY - 2);
    TEXTURE_DESCRIPTOR *src = tex_desc.item + src_idx, *dst = tex_desc.item + dst_idx;
    uint32 i, v;
    uint8 u;
    for (i = 0; i < 2; i++, src++, dst++)
    {
        v = tex_base_v[i];
        v += stage_animation.phase[0];
        src->uv1.v = (uint8)v;
        src->uv0.v = (uint8)v;
        src->uv3.v = (uint8)(v + 31);
        src->uv2.v = (uint8)(v + 31);
        u = src->uv0.u;
        dst->uv2.u = u;
        dst->uv0.u = u;
        u = src->uv1.u;
        dst->uv3.u = u;
        dst->uv1.u = u;
        dst->uv1.v = (uint8)(v + 32);
        dst->uv0.v = (uint8)(v + 32);
        dst->uv3.v = (uint8)(v + 63);
        dst->uv2.v = (uint8)(v + 63);
        dst->tpage = (sint16)src->tpage;
    }
    return 0;
}

const OBJECT_CALLBACK callback_stage_animation_slide_x = {OBJECT_CALL_U32_MODEL, {.u32_model = draw_slide_x}, 0};

const OBJECT_CALLBACK callback_stage_animation_slide_z = {OBJECT_CALL_U32_MODEL, {.u32_model = draw_slide_z}, 0};

const OBJECT_CALLBACK callback_stage_animation_double_slide_z = {OBJECT_CALL_U32_MODEL, {.u32_model = draw_double_slide_z}, 0};
