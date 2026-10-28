#include <stdio.h>

#include "game.h"

LIFT lift;

static void draw_section(uint32 model, uint32 height)
{
    FUNCTION_MARKER(0x8005bd04u, "GAME.EXE");
    if (height + 0x17ffu >= 0x1fffu)
        return;
    transform_push();
    transform_translate(0, height << 16, 0);
    model_draw_static(model_refs[model], 0, 2048);
    transform_pop();
}

static void draw(const MODEL_OBJECT *item)
{
    FUNCTION_MARKER(0x8005bd7cu, "GAME.EXE");
    sint32 phase = (sint32)lift.pos >> 4;
    uint32 color, height;
    if (phase < 433)
    {
        phase -= 416;
        if (phase < 0)
            phase = 0;
        color = 4u * (uint32)phase + 64u;
        render_style.color.rgbc = 0x2c000000u + (color << 16) + (color << 8) + color;
    }
    /* TODO: Investigate a model callback outside its native object domain */
    if (!item)
    {
        DIAGNOSTIC(fprintf(stderr, "Invalid model receiver %p in %s stage %u tick %u\n", (const void *)item, __FUNCTION__, game_session.stage, game_session.tick));
        diagnostic_stop(__FUNCTION__, __FILE__, __LINE__);
    }
    model_draw_static(model_refs[item->kind], 0, 2048);
    height = 0u - lift.pos;
    draw_section(440, height);
    height += 1472u;
    draw_section(440, height);
    height += 1472u;
    draw_section(440, height);
    height += 1472u;
    draw_section(440, height);
    height += 1472u;
    draw_section(439, height);
    height += 3456u;
    draw_section(437, height);
    height += 3456u;
    draw_section(438, height);
    height += 3456u;
    draw_section(437, height);
    height += 3456u;
    draw_section(438, height);
    height += 3456u;
    draw_section(437, height);
    height += 3456u;
    draw_section(438, height);
    height += 3456u;
    draw_section(437, height);
    height += 3456u;
    draw_section(438, height);
    height += 3456u;
    draw_section(441, height);
    draw_section(442, height + 3456u);
}

static uint32 update_marker(struct EFFECT *effect)
{
    FUNCTION_MARKER(0x8002da0cu, "GAME.EXE");
    /* TODO: Investigate a lift marker with an unpublished or foreign payload */
    if (!effect || effect->payload_kind != EFFECT_PAYLOAD_PARTICLE || (!(effect->kind - 897u < 13u || effect->kind < 6u || effect->kind - 6u < 2u || effect->kind - 577u < 3u || effect->kind == 583 || effect->kind - 599u < 3u || effect->kind - 72u < 4u) && effect->kind != 0xffffffffu))
    {
        DIAGNOSTIC(fprintf(stderr, "Invalid lift marker payload %p kind %u owner %u stage %u tick %u\n", (void *)effect, effect ? effect->kind : 0xffffffffu, effect ? (uint32)effect->payload_kind : 0xffffffffu, game_session.stage, game_session.tick));
        diagnostic_stop(__FUNCTION__, __FILE__, __LINE__);
    }
    EFFECT_PARTICLE *particle = &effect->particle;
    particle->pos.x.bits = ((lift.cycle << 5) - 240u) << 16;
    particle->pos.y.bits = 0xfe300000u;
    particle->pos.z.bits = 0x01a00000u;
    return 0x01a00000u;
}

const OBJECT_CALLBACK callback_lift_draw = {OBJECT_CALL_VOID_MODEL, {.no_result_model = draw}, 0};

const OBJECT_CALLBACK callback_lift_update_marker = {OBJECT_CALL_U32_EFFECT, {.u32_effect = update_marker}, 0};

static void draw_marker(const struct EFFECT *effect)
{
    FUNCTION_MARKER(0x8005d608u, "GAME.EXE");
    PROJECTION_POINT pos;
    transform_push();
    object_transform_camera(NULL, effect);
    proj_local(0, 0, 0, &pos);
    if (!pos.status)
    {
        pos.size = 512;
        spr_draw_scaled(&pos, tex_map[66]);
    }
    transform_pop();
}

const OBJECT_CALLBACK callback_lift_draw_marker = {OBJECT_CALL_VOID_EFFECT, {.no_result_effect = draw_marker}, 0};
