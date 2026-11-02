#include <stdio.h>

#include "game.h"

COLLISION_DROPS col_drops;

enum
{
    DROP_ANGLE_COUNT = 4,
    DROP_TYPE_COUNT = 6
};

static const uint16 drop_angles[DROP_ANGLE_COUNT] = {0xc300, 0xc000, 0xd500, 0xc700};
static const uint32 drop_types[DROP_TYPE_COUNT] = {499, 587, 496, 588, 515, 506};

typedef struct COLLISION_MODEL_RULE
{
    sint16 model_type;
    sint8 hits;
    sint8 kind;
    sint8 padding;
    sint8 alternate_attack;
} COLLISION_MODEL_RULE;

enum
{
    COLLISION_MODEL_RULE_COUNT = 107
};

static const COLLISION_MODEL_RULE model_rules[COLLISION_MODEL_RULE_COUNT + 1] = {
    {581, 1, 1, 0, 0},   {608, 2, 6, 0, 1},  {609, 2, 6, 0, 1},   {610, 2, 6, 0, 1}, {611, 2, 6, 0, 1},  {612, 2, 6, 0, 1},  {613, 2, 6, 0, 1}, {94, 4, 10, 0, 0},  {95, 4, 10, 0, 0},  {108, 4, 10, 0, 0}, {90, 4, 12, 0, 0}, {70, 4, 14, 96, 0}, {62, 4, 12, 0, 0}, {66, 4, 12, 0, 0}, {84, 4, 13, 0, 0}, {78, 4, 13, 0, 0}, {79, 4, 13, 0, 0}, {80, 4, 13, 0, 0}, {81, 4, 13, 0, 0}, {82, 4, 13, 0, 0}, {83, 4, 13, 0, 0}, {642, 2, 17, 32, 0}, {649, 4, 17, 32, 1}, {672, 4, 17, 16, 1}, {655, 4, 18, 32, 1}, {647, 1, 19, 16, 1}, {124, 4, 15, 64, 0}, {127, 4, 16, 64, 0}, {166, 4, 13, 0, 0}, {168, 4, 13, 0, 0}, {175, 4, 13, 0, 0}, {160, 4, 13, 0, 0}, {695, 1, 23, 0, 1}, {708, 1, 23, 0, 1}, {709, 1, 23, 0, 1}, {710, 1, 23, 0, 1},
    {696, 1, 24, 0, 1},  {718, 4, 9, 0, 0},  {713, 4, 6, 0, 0},   {715, 4, 6, 0, 0}, {720, 4, 11, 0, 0}, {215, 1, 7, 0, 0},  {217, 1, 7, 0, 0}, {219, 1, 7, 0, 0},  {223, 1, 7, 0, 0},  {221, 1, 7, 0, 0},  {231, 1, 7, 0, 0}, {239, 1, 7, 0, 0},  {227, 1, 7, 0, 0}, {229, 1, 7, 0, 0}, {241, 1, 7, 0, 0}, {233, 1, 7, 0, 0}, {235, 1, 7, 0, 0}, {237, 1, 7, 0, 0}, {225, 1, 7, 0, 0}, {737, 4, 6, 0, 1}, {735, 2, 6, 0, 1}, {736, 2, 6, 0, 1},   {748, 2, 6, 0, 1},   {755, 4, 20, 64, 0}, {740, 4, 13, 0, 0},  {749, 4, 13, 0, 0},  {295, 4, 5, 0, 1},   {297, 4, 5, 0, 1},   {289, 99, 8, 0, 0}, {299, 8, 5, 0, 1},  {300, 8, 5, 0, 1},  {301, 99, 3, 0, 1}, {326, 2, 6, 0, 1},  {773, 4, 25, 0, 1}, {774, 4, 25, 0, 1}, {775, 4, 25, 0, 1},
    {776, 16, 21, 0, 1}, {781, 99, 9, 0, 1}, {341, 99, 13, 0, 0}, {343, 4, 3, 0, 0}, {784, 4, 9, 0, 0},  {392, 6, 21, 0, 0}, {367, 2, 6, 0, 1}, {383, 4, 13, 0, 1}, {387, 4, 13, 0, 1}, {371, 4, 9, 0, 0},  {373, 4, 9, 0, 0}, {375, 4, 9, 0, 0},  {377, 4, 9, 0, 0}, {379, 4, 9, 0, 0}, {381, 4, 9, 0, 0}, {804, 4, 5, 0, 0}, {806, 4, 5, 0, 0}, {808, 4, 5, 0, 0}, {810, 4, 5, 0, 0}, {789, 4, 9, 0, 1}, {795, 4, 9, 0, 1}, {786, 4, 9, 0, 1},   {828, 2, 9, 0, 0},   {830, 2, 9, 0, 0},   {408, 1, 23, 0, 1},  {409, 4, 5, 0, 1},   {411, 4, 9, 0, 1},   {413, 4, 9, 0, 1},   {415, 4, 9, 0, 1},  {417, 4, 9, 0, 1},  {419, 4, 9, 0, 1},  {421, 4, 9, 0, 1},  {423, 4, 9, 0, 1},  {425, 4, 9, 0, 1},  {427, 4, 9, 0, 1},  {-1, -1, -1, 0, 0},
};

/* Original five arguments; padding is caller SP+10, not argument nine from IDA */
uint32 col_region_build(COLLISION_REGION *region, sint32 model_type, sint32 hits, sint32 kind, sint32 padding)
{
    FUNCTION_MARKER(0x80016a28, "GAME.EXE");
    sint32 min_x = 32767, min_z = 32767, max_x = -32768, max_z = -32768, i, n, x, z;
    MODEL_REFERENCE model;
    uint32 result;
    const SVECTOR *vertices;
    int oriented;
    region->hits = hits - 1u;
    region->model_type = model_type;
    region->col_kind = kind;
    oriented = (kind - 10u) < 2 || (kind - 12u) < 2 || kind == 21 || model_type == 781;
    region->shape = oriented;
    XASSERT(model_type >= 0 && model_type < MODEL_TYPE_COUNT);
    model = model_refs[(uint32)model_type];
    XASSERT(model.header && model.byte_len >= sizeof(*model.header));
    n = model.header->vertex_count;
    /* TODO: Investigate an original collision mesh reading outside its resource */
    XASSERT(n <= 0 || (model.vertices && !((uintptr_t)model.vertices & 1) && (uint32)n <= model.vertex_byte_len / sizeof(*vertices)));
    vertices = n > 0 ? model.vertices : NULL;
    for (i = 0; i < n; i++)
    {
        x = vertices[i].vx;
        if (x > max_x)
        {
            max_x = x;
            x = vertices[i].vx;
        }
        if (x < min_x)
            min_x = x;
        z = vertices[i].vz;
        if (z > max_z)
        {
            max_z = z;
            z = vertices[i].vz;
        }
        if (z < min_z)
            min_z = z;
    }
    min_x = min_x - (uint32)padding;
    min_z = min_z - (uint32)padding;
    max_x = max_x + (uint32)padding;
    max_z = max_z + (uint32)padding;
    if (max_x < 64)
        max_x = 64;
    if (min_x >= -63)
        min_x = -64;
    if (max_z < 64)
        max_z = 64;
    if (min_z >= -63)
        min_z = -64;
    if (oriented)
    {
        region->geometry.box.half_x = (sint32)(max_x - (uint32)min_x) >> 1;
        region->geometry.box.half_z = (sint32)(max_z - (uint32)min_z) >> 1;
        XASSERT(region->geometry.box.half_x >= 0); /* original fatal loop61120 */
    }
    else
    {
        region->geometry.axis.min_x = ((uint16)region->geometry.axis.min_x) + (uint32)min_x;
        region->geometry.axis.max_x = ((uint16)region->geometry.axis.max_x) + (uint32)max_x;
        region->geometry.axis.min_z = ((uint16)region->geometry.axis.min_z) + (uint32)min_z;
        region->geometry.axis.max_z = ((uint16)region->geometry.axis.max_z) + (uint32)max_z;
    }
    result = col_regions.count + 1;
    col_regions.count = result;
    return result;
}

uint32 col_region_configure(COLLISION_REGION *region, sint32 model_type)
{
    FUNCTION_MARKER(0x8003b77c, "GAME.EXE");
    const COLLISION_MODEL_RULE *entry = model_rules;
    uint32 val = (uint16)entry->model_type;
    sint32 hits, kind, padding;
    sint8 alternate_attack;
    if ((sint16)val == -1)
        return 0xffffffffu;
    for (;;)
    {
        if ((sint16)val == model_type)
        {
            hits = entry->hits;
            kind = entry->kind;
            padding = entry->padding;
            col_region_build(region, model_type, hits, kind, padding);
            alternate_attack = entry->alternate_attack;
            region->alternate_attack = alternate_attack;
        }
        ++entry;
        val = (uint16)entry->model_type;
        if (entry->model_type == -1)
            return val << 16;
    }
}

COLLISION_REFERENCE col_callbacks[COLLISION_SLOT_COUNT];

sint32 col_call(COLLISION_REFERENCE target, COLLISION_REGION *obstacle, MODEL_OBJECT *model, COLLISION_POINT *contact, sint32 actor_idx)
{
    if (game_services.col_event)
        return game_services.col_event(target, obstacle, model, contact, actor_idx);
    if (target)
    {
        if (target->signature == COLLISION_NO_ARGS)
            return target->fn.no_args();
        if (target->signature == COLLISION_CONTACT)
            return target->fn.contact(obstacle, model, contact, actor_idx);
        if (target->signature == COLLISION_MUTABLE_CONTACT)
            return target->fn.mutable_contact(obstacle, model, contact, actor_idx);
    }

    XASSERT(0);
    return 0;
}

/* Exact original leaf, including its return delay slot; see leaf-batch.json. */
uint32 col_noop(void)
{
    FUNCTION_MARKER(0x8001f510u, "GAME.EXE");
    return 0;
}

/* 1F518: model-specific destruction and replacement of a collision region. */
static sint32 damage_replace(COLLISION_REGION *obstacle, MODEL_OBJECT *model, const COLLISION_POINT *contact, sint32 actor_idx)
{
    FUNCTION_MARKER(0x8001f518u, "GAME.EXE");
    /* TODO: Investigate invalid actor references at the callback boundary */
    XASSERT(actor_idx == -1 || actor_idx >= 0 && actor_idx < ACTOR_COUNT);
    uint32 kind, v, height, angle, x, y, z;
    int i;
    /* TODO: Investigate wobble access without the matching native owner */
    XASSERT(model && model->payload_kind == MODEL_PAYLOAD_WOBBLE);
    model->wobble.ticks = 16;
    x = contact->pos.x;
    y = contact->pos.y;
    z = contact->pos.z;
    /* TODO: Investigate damage common fields outside the native object pools */
    XASSERT(model);
    kind = model->kind;
    dynamic_objects_spawn_transient(865, x, y, z);
    sound_voice_play(133, 160, 6144);
    v = (uint16)obstacle->hits - 1;
    obstacle->hits = v;
    if ((sint32)(v << 16) >= 0)
        return 1;
    model->kind = model->kind + 1u;
    if (kind == 295 || kind == 297)
    {
        obstacle->col_kind = 3;
        /* An absent actor would make the original write low RAM at 0x8A */
        if (actor_idx != -1)
            ++actor_bonus_counts[actor_idx];
        sound_voice_play(19, 255, 6144);
        /* TODO: Investigate damage callbacks without a native model object */
        XASSERT(model);
        x = model->pos.x.bits;
        z = model->pos.z.bits;
        effect_spawn(893, x, 0xffc00000u, z);
        disp_buffers.shake_ticks = 16;
        disp_buffers.flash_ticks = 4;
        for (i = 0; i < 8; i++)
        {
            v = (random_next() & 3u) + 902u;
            /* TODO: Investigate damage callbacks without a native model object */
            XASSERT(model);
            x = model->pos.x.bits;
            z = model->pos.z.bits;
            effect_spawn_debris(v, x, 0xffc00000u, z);
        }
    }
    if (kind - 299u < 2u)
    {
        model->kind = 301;
        /* An absent actor would make the original write low RAM at 0x8A */
        if (actor_idx != -1)
            ++actor_bonus_counts[actor_idx];
        sound_voice_play(19, 192, 6144);
        /* TODO: Investigate damage callbacks without a native model object */
        XASSERT(model);
        x = model->pos.x.bits;
        z = model->pos.z.bits;
        effect_spawn(894, x, 0xff800000u, z);
        actor_positions[actor_idx].x.bits = actor_positions[actor_idx].x.bits + 0x200000u;
        v = (model->pos.x.whole) + 32u;
        obstacle->geometry.coord[2] = v;
        obstacle->geometry.coord[0] = v;
        v = model->pos.z.half[1];
        disp_buffers.shake_ticks = 8;
        disp_buffers.flash_ticks = 4;
        obstacle->geometry.coord[3] = v;
        obstacle->geometry.coord[1] = v;
        col_region_configure(obstacle, 301);
        col_regions.count = col_regions.count - 1u;
    }
    if (kind == 804 || kind == 806 || kind == 808 || kind == 810 || kind == 409)
    {
        obstacle->col_kind = 3;
        /* An absent actor would make the original write low RAM at 0x8A */
        if (actor_idx != -1)
            ++actor_bonus_counts[actor_idx];
        sound_voice_play(19, 192, 6144);
        height = (kind == 808 || kind == 810 || kind == 409) ? 0xfffffee0u : 0xffffff60u;
        /* TODO: Investigate damage callbacks without a native model object */
        XASSERT(model);
        x = model->pos.x.bits;
        z = model->pos.z.bits;
        effect_spawn(894, x, height << 16, z);
        angle = model->pos.x.val > 0 ? 512u : 1536u;
        for (i = 0; i < 8; i++)
        {
            v = (random_next() & 3u) + 902u;
            /* TODO: Investigate damage callbacks without a native model object */
            XASSERT(model);
            x = model->pos.x.bits;
            z = model->pos.z.bits;
            effect_spawn_debris_aimed(v, x, height << 15, z, angle);
        }
        disp_buffers.shake_ticks = 8;
        disp_buffers.flash_ticks = 4;
    }
    return 1;
}

/* 1F7EC: region type6 breakable, with eight impact and eight debris calls. */
static sint32 damage_drop(COLLISION_REGION *obstacle, MODEL_OBJECT *model, const COLLISION_POINT *contact, sint32 actor_idx)
{
    FUNCTION_MARKER(0x8001f7ecu, "GAME.EXE");
    /* TODO: Investigate invalid actor references at the callback boundary */
    XASSERT(actor_idx == -1 || actor_idx >= 0 && actor_idx < ACTOR_COUNT);
    uint32 angle = actor_idx != -1 ? (uint32)(sint32)actor_orientation[actor_idx].yaw : 0;
    uint32 kind;
    /* TODO: Investigate damage common fields outside the native object pools */
    XASSERT(model);
    kind = model->kind;
    /* TODO: Investigate damage callbacks without a native model object */
    XASSERT(model);
    uint32 x = model->pos.x.bits;
    uint32 y = model->pos.y.bits;
    uint32 z = model->pos.z.bits;
    sint16 health;
    sint32 i, drop = -1;
    dynamic_objects_spawn_transient(865, contact->pos.x, contact->pos.y, contact->pos.z);
    sound_voice_play(112, 255, 6144);
    for (i = 0; i < 8; i++)
        effect_spawn_spark_reverse(contact->pos.x, contact->pos.y, contact->pos.z, angle);
    health = (uint16)((uint16)obstacle->hits - 1);
    obstacle->hits = health;
    if (health >= 0)
    {
        /* TODO: Investigate damage access without the matching native owner */
        XASSERT(model && (model->payload_kind == MODEL_PAYLOAD_WOBBLE || model->payload_kind == MODEL_PAYLOAD_PHYSICS));
        if (model->payload_kind == MODEL_PAYLOAD_WOBBLE)
            model->wobble.ticks = 16;
        else
            model->physics.life = 16;
        return 1;
    }
    obstacle->col_kind = 0;
    model->kind = 55;
    /* An absent actor would make the original write low RAM at 0x8A */
    if (actor_idx != -1)
        ++actor_bonus_counts[actor_idx];
    sound_voice_play(19, 255, 6144);
    effect_spawn(893, x, 0xffe00000u, z);
    disp_buffers.shake_ticks = 16;
    disp_buffers.flash_ticks = 4;
    for (i = 0; i < 8; i++)
        effect_spawn_debris(902u + (random_next() & 3u), x, y, z);
    if (kind == 608)
        drop = (sint32)random_next() % 3 + 587;
    else if (kind == 609)
        drop = 499;
    else if (kind == 610)
        drop = 504;
    else if (kind == 611)
        drop = 496;
    else if (kind == 612)
        drop = 497;
    else if (kind == 613)
        drop = 505;
    if (drop != -1)
        dynamic_objects_spawn_item(drop, x, 0, z, 0);
    return 1;
}

/* 1FA78: staged model damage and kind-specific item drops. */
static sint32 damage_staged_drop(COLLISION_REGION *obstacle, MODEL_OBJECT *model, const COLLISION_POINT *contact, sint32 actor_idx)
{
    FUNCTION_MARKER(0x8001fa78u, "GAME.EXE");
    /* TODO: Investigate invalid actor references at the callback boundary */
    XASSERT(actor_idx == -1 || actor_idx >= 0 && actor_idx < ACTOR_COUNT);
    uint32 angle = actor_idx != -1 ? (uint32)(sint32)actor_orientation[actor_idx].yaw : 0, kind, v, x, y, z;
    sint16 health;
    sint32 slot;
    MODEL_OBJECT *drop;
    int i;
    dynamic_objects_spawn_transient(865, contact->pos.x, contact->pos.y, contact->pos.z);
    for (i = 0; i < 8; i++)
        effect_spawn_spark_reverse(contact->pos.x, contact->pos.y, contact->pos.z, angle);
    sound_voice_play(112, 255, 6144);
    /* TODO: Investigate damage access without the matching native owner */
    XASSERT(model && (model->payload_kind == MODEL_PAYLOAD_WOBBLE || model->payload_kind == MODEL_PAYLOAD_PHYSICS || model->payload_kind == MODEL_PAYLOAD_ARMY_TRUCK));
    if (model->payload_kind == MODEL_PAYLOAD_WOBBLE)
        model->wobble.ticks = 16;
    else if (model->payload_kind == MODEL_PAYLOAD_PHYSICS)
        model->physics.life = 16;
    else
        model->truck.flags = 16;
    health = (uint16)((uint16)obstacle->hits - 1u);
    obstacle->hits = health;
    if (health >= 0)
        return 1;
    /* TODO: Investigate damage common fields outside the native object pools */
    XASSERT(model);
    kind = model->kind;
    model->kind = kind + 1u;
    if (kind == 789 || kind == 792 || kind == 795 || kind == 786)
    {
        obstacle->hits = 4;
        sound_voice_play(114, 255, 6144);
        return 1;
    }
    obstacle->col_kind = 3;
    /* An absent actor would make the original write low RAM at 0x8A */
    if (actor_idx != -1)
        ++actor_bonus_counts[actor_idx];
    sound_voice_play(19, 255, 6144);
    /* TODO: Investigate damage callbacks without a native model object */
    XASSERT(model);
    x = model->pos.x.bits;
    z = model->pos.z.bits;
    effect_spawn(893, x, 0xffc00000u, z);
    disp_buffers.shake_ticks = 16;
    disp_buffers.flash_ticks = 4;
    for (i = 0; i < 8; i++)
    {
        v = (random_next() & 3u) + 902u;
        /* TODO: Investigate damage callbacks without a native model object */
        XASSERT(model);
        x = model->pos.x.bits;
        y = model->pos.y.bits;
        z = model->pos.z.bits;
        effect_spawn_debris(v, x, y, z);
    }
    if (model->payload_kind == MODEL_PAYLOAD_WOBBLE)
        model->wobble.emit_ticks = 240;
    else if (model->payload_kind == MODEL_PAYLOAD_PHYSICS)
        model->physics.spin = 240;
    else
        model->truck.active = 240;
    if (model->payload_kind == MODEL_PAYLOAD_WOBBLE)
        model->wobble.ticks = 0;
    else if (model->payload_kind == MODEL_PAYLOAD_PHYSICS)
        model->physics.life = 0;
    else
        model->truck.flags = 0;
    if (kind == 718)
        for (i = 0; i < 4; i++)
        {
            angle = random_next() << 1;
            /* TODO: Investigate damage callbacks without a native model object */
            XASSERT(model);
            x = model->pos.x.bits;
            z = model->pos.z.bits;
            slot = dynamic_objects_spawn_item(591, x, 0xff800000u, z, angle);
            v = (random_next() & 15) + 50;
            /* TODO: Investigate the original pre-pool write after drop allocation failure */
            XASSERT(slot >= 0 && slot < DYNAMIC_OBJECT_COUNT);
            drop = dynamic_objects + slot;
            /* TODO: Investigate a destruction drop with a foreign native owner */
            XASSERT(drop->payload_kind == MODEL_PAYLOAD_PHYSICS);
            drop->physics.speed = (uint16)v;
            v = (random_next() & 15) + 60;
            drop->physics.vertical_speed = (uint16)v;
            drop->physics.obstacle_flags = 1;
        }
    if (kind == 828 || kind == 830)
    {
        angle = model->pos.x.val > 0 ? 0xffffc000u : 0x4000u;
        /* TODO: Investigate damage callbacks without a native model object */
        XASSERT(model);
        x = model->pos.x.bits;
        z = model->pos.z.bits;
        slot = dynamic_objects_spawn_item(505, x, 0xff000000u, z, angle);
        /* TODO: Investigate the original pre-pool write after drop allocation failure */
        XASSERT(slot >= 0 && slot < DYNAMIC_OBJECT_COUNT);
        drop = dynamic_objects + slot;
        /* TODO: Investigate a destruction drop with a foreign native owner */
        XASSERT(drop->payload_kind == MODEL_PAYLOAD_PHYSICS);
        drop->physics.speed = 32;
        drop->physics.obstacle_flags = 1;
    }
    if (kind == 784)
    {
        slot = dynamic_objects_spawn_item(515, 0xf6400000u, 0xff100000u, 0xf0c00000u, 0x4000u);
        MODEL_OBJECT *next_model0 = NULL;
        uint32 next_idx0;
        for (next_idx0 = 0; next_idx0 + 1 < WORLD_OBJECT_COUNT; next_idx0++)
            if (model == world_objects.object + next_idx0)
            {
                next_model0 = world_objects.object + next_idx0 + 1;
                break;
            }
        if (!next_model0)
            for (next_idx0 = 0; next_idx0 + 1 < DYNAMIC_OBJECT_COUNT; next_idx0++)
                if (model == dynamic_objects + next_idx0)
                {
                    next_model0 = dynamic_objects + next_idx0 + 1;
                    break;
                }
        model = next_model0;
        /* TODO: Investigate the original pre-pool write after drop allocation failure */
        XASSERT(slot >= 0 && slot < DYNAMIC_OBJECT_COUNT);
        drop = dynamic_objects + slot;
        /* TODO: Investigate a destruction drop with a foreign native owner */
        XASSERT(drop->payload_kind == MODEL_PAYLOAD_PHYSICS);
        drop->physics.speed = 64;
        drop->physics.vertical_speed = 64;
        drop->physics.obstacle_flags = 1;
        /* TODO: Investigate damage common fields outside the native object pools */
        XASSERT(model);
        model->kind = 55;
    }
    if (kind == 781)
    {
        const MODEL_OBJECT *relative_object;
        /* Absence is valid for comparison, but not a corrupt native reference */
        /* TODO: Investigate malformed relative-scene identity during destruction */
        XASSERT(world_objects.relative_scene.present <= 1 && (!world_objects.relative_scene.present || world_objects.relative_scene.idx < WORLD_OBJECT_COUNT));
        relative_object = world_objects.relative_scene.present ? world_objects.object + world_objects.relative_scene.idx : NULL;
        if (relative_object == model && model->truck.active)
        {
            model->truck.active = 0;
            level_wave.paused = 0;
        }
    }
    return 1;
}

/* 1FDA0: destruction precedes impact particles; fourth position word selects sound. */
static sint32 damage_wave_obstacle(COLLISION_REGION *obstacle, MODEL_OBJECT *model, const COLLISION_POINT *contact, sint32 actor_idx)
{
    FUNCTION_MARKER(0x8001fda0u, "GAME.EXE");
    /* TODO: Investigate invalid actor references at the callback boundary */
    XASSERT(actor_idx == -1 || actor_idx >= 0 && actor_idx < ACTOR_COUNT);
    uint32 angle = actor_idx != -1 ? (uint32)(sint32)actor_orientation[actor_idx].yaw : 0, v, x, y, z;
    sint16 health;
    int i;
    dynamic_objects_spawn_transient(865, contact->pos.x, contact->pos.y, contact->pos.z);
    health = (uint16)((uint16)obstacle->hits - 1);
    obstacle->hits = health;
    if (health < 0)
    {
        /* An absent actor would make the original write low RAM at 0x8A */
        if (actor_idx != -1)
            ++actor_bonus_counts[actor_idx];
        obstacle->col_kind = 0;
        /* TODO: Investigate damage common fields outside the native object pools */
        XASSERT(model);
        model->kind = 55;
        sound_voice_play(19, 255, 6144);
        /* TODO: Investigate damage callbacks without a native model object */
        XASSERT(model);
        x = model->pos.x.bits;
        z = model->pos.z.bits;
        effect_spawn(893, x, 0xffe00000u, z);
        disp_buffers.shake_ticks = 16;
        disp_buffers.flash_ticks = 4;
        for (i = 0; i < 8; i++)
        {
            v = (random_next() & 3u) + 902u;
            /* TODO: Investigate damage callbacks without a native model object */
            XASSERT(model);
            x = model->pos.x.bits;
            y = model->pos.y.bits;
            z = model->pos.z.bits;
            effect_spawn_debris(v, x, y, z);
        }
        level_wave.paused = 0;
    }
    for (i = 0; i < 8; i++)
        effect_spawn_spark_reverse(contact->pos.x, contact->pos.y, contact->pos.z, angle);
    if (contact->impact_sound)
        sound_voice_play(23, 192, 6144);
    else
        sound_voice_play(33, 255, 6144);
    return 2;
}

/* 1FF08: negative entry health explodes immediately; a decrement below zero
 * instead advances the damage stage. The two paths must remain distinct. */
static sint32 damage_explosive_pair(COLLISION_REGION *obstacle, MODEL_OBJECT *model, COLLISION_POINT *contact, sint32 actor_idx)
{
    FUNCTION_MARKER(0x8001ff08u, "GAME.EXE");
    MODEL_OBJECT *drop;
    /* TODO: Investigate invalid actor references at the callback boundary */
    XASSERT(actor_idx == -1 || actor_idx >= 0 && actor_idx < ACTOR_COUNT);
    uint32 angle = actor_idx != -1 ? (uint32)(sint32)actor_orientation[actor_idx].yaw : 0, kind, x, y, z, v, a, b, d, stage, rx, rz;
    FIXED_VEC3 point;
    sint32 slot, voice;
    sint16 health;
    int i;
    if (obstacle->hits < 0)
    {
        /* TODO: Investigate explosive voice without the matching native owner */
        XASSERT(model && model->payload_kind == MODEL_PAYLOAD_OSCILLATION);
        voice = model->oscillation.voice_plus_one;
        if (voice)
        {
            sound_voice_stop(voice - 1);
            model->oscillation.voice_plus_one = (sint16)(0);
        }
        obstacle->col_kind = 3;
        /* TODO: Investigate damage common fields outside the native object pools */
        XASSERT(model);
        /* Explicit predecessor handoff at the original kind transition, not a lazy getter */
        if (model->payload_kind == MODEL_PAYLOAD_OSCILLATION)
        {
            /* Original stores below immediately replace count and vertical speed */
            model->bounce = (BOUNCING_PART){.bounces = model->oscillation.roll_speed, .speed = model->oscillation.speed, .physics_spin = model->oscillation.pitch_speed, .physics_actor_idx = model->oscillation.phase, .physics_bounce_count = model->oscillation.voice_plus_one, .vertical_speed = model->oscillation.sound_clock, .physics_obstacle_flags = model->oscillation.physics_obstacle_flags};
            model->payload_kind = MODEL_PAYLOAD_BOUNCING_PART;
        }
        /* TODO: Investigate an invalid native predecessor at the bounce transition */
        XASSERT(model->payload_kind == MODEL_PAYLOAD_BOUNCING_PART);
        model->kind = 100;
        model->bounce.vertical_speed = -128;
        model->bounce.bounces = 1;
        MODEL_OBJECT *next_model1 = NULL;
        uint32 next_idx1;
        for (next_idx1 = 0; next_idx1 + 1 < WORLD_OBJECT_COUNT; next_idx1++)
            if (model == world_objects.object + next_idx1)
            {
                next_model1 = world_objects.object + next_idx1 + 1;
                break;
            }
        if (!next_model1)
            for (next_idx1 = 0; next_idx1 + 1 < DYNAMIC_OBJECT_COUNT; next_idx1++)
                if (model == dynamic_objects + next_idx1)
                {
                    next_model1 = dynamic_objects + next_idx1 + 1;
                    break;
                }
        model = next_model1;
        /* TODO: Investigate pair damage without a native model object */
        XASSERT(model);
        x = model->pos.x.bits;
        z = model->pos.z.bits;
        model->kind = 861;
        /* An absent actor would make the original write low RAM at 0x8A */
        if (actor_idx != -1)
            ++actor_bonus_counts[actor_idx];
        sound_voice_play(19, 255, 6144);
        effect_spawn(892, x, contact->pos.y, z);
        effect_spawn(603, x, 0, z);
        for (i = 0; i < 16; i++)
        {
            v = random_next();
            /* TODO: Investigate pair damage without a native model object */
            XASSERT(model);
            x = model->pos.x.bits;
            y = model->pos.y.bits;
            z = model->pos.z.bits;
            effect_spawn_debris((v & 3u) + 902u, x, y, z);
        }
        disp_buffers.shake_ticks = 16;
        disp_buffers.flash_ticks = 8;
        return 2;
    }
    dynamic_objects_spawn_transient(865, contact->pos.x, contact->pos.y, contact->pos.z);
    for (i = 0; i < 8; i++)
        effect_spawn_spark_reverse(contact->pos.x, contact->pos.y, contact->pos.z, angle);
    /* TODO: Investigate damage common fields outside the native object pools */
    XASSERT(model);
    kind = model->kind;
    if (contact->impact_sound)
        sound_voice_play(23, 192, 6144);
    else
        sound_voice_play(33, 255, 6144);
    if (kind == 96 || kind == 110)
    {
        col_regions.side = 4;
        obstacle->hits = 1;
    }
    else
    {
        /* TODO: Investigate explosive clock without the matching native owner */
        XASSERT(model && model->payload_kind == MODEL_PAYLOAD_OSCILLATION);
        stage = game_session.stage;
        if (stage == 0 || stage == 7)
        {
            if (!model->oscillation.voice_plus_one)
                model->oscillation.voice_plus_one = (uint16)(sound_voice_play(107, 255, 6144) + 1u);
            model->oscillation.sound_clock = (sint16)(300);
        }
    }
    a = 384;
    b = 768;
    if (kind == 95 || kind == 109)
    {
        a = 768;
        b = 1536;
    }
    /* TODO: Investigate damage impulse without the matching native owner */
    XASSERT(model && model->payload_kind == MODEL_PAYLOAD_OSCILLATION);
    if (!col_regions.side)
        model->oscillation.pitch_speed = (uint16)a;
    d = col_regions.side;
    if (d == 1)
    {
        model->oscillation.roll_speed = (uint16)b;
        d = col_regions.side;
    }
    if (d == 2)
        model->oscillation.pitch_speed = (uint16)(-(sint64)(a));
    if (col_regions.side == 3)
        model->oscillation.roll_speed = (uint16)(-(sint64)(b));
    health = (uint16)((uint16)obstacle->hits - 1);
    obstacle->hits = health;
    if (health >= 0)
        return 2;
    model->kind = model->kind + 1u;
    obstacle->hits = (uint16)obstacle->hits + 4;
    kind = model->kind;
    angle = model->angles.yaw;
    if (kind == 95 || kind == 109)
    {
        sound_voice_play(114, 255, 6144);
        /* TODO: Investigate pair damage without a native model object */
        XASSERT(model);
        x = model->pos.x.bits;
        z = model->pos.z.bits;
        transform_push();
        transform_identity();
        transform_pitch(32768);
        transform_yaw(angle);
        for (i = 0; i < 24; i++)
        {
            v = random_next() & 255u;
            rx = (v + (v >> 3) - 144u) << 16;
            rz = ((random_next() & 127u) + 0xfffffec4u) << 16;
            transform_point(rx, 0, rz, contact->pos.bits);
            effect_spawn_timed(870, x + contact->pos.x, 0xff600000u, z + contact->pos.z, 0, 0, 0);
        }
        for (i = 0; i < 16; i++)
        {
            v = random_next() & 255u;
            rx = (v + (v >> 3) - 144u) << 16;
            rz = ((random_next() & 127u) + 216u) << 16;
            transform_point(rx, 0, rz, contact->pos.bits);
            effect_spawn_timed(870, x + contact->pos.x, 0xff600000u, z + contact->pos.z, 0, 0, 0);
        }
        transform_pop();
        kind = model->kind;
    }
    if (kind == 96 || kind == 110)
    {
        /* An absent actor would make the original write low RAM at 0x8A */
        if (actor_idx != -1)
            ++actor_bonus_counts[actor_idx];
        sound_voice_play(115, 255, 6144);
        /* TODO: Investigate pair damage without a native model object */
        XASSERT(model);
        x = model->pos.x.bits;
        z = model->pos.z.bits;
        transform_push();
        transform_identity();
        transform_pitch(32768);
        transform_yaw(angle);
        transform_point(0, 0, -17301504, point.bits);
        dynamic_objects_spawn_oriented(534, x + point.x.bits, 0, z + point.z.bits, angle);
        transform_point(9437184, 0, 14155776, point.bits);
        slot = dynamic_objects_spawn_oriented(105, x + point.x.bits, 0, z + point.z.bits, angle + 0xffffc000u);
        /* TODO: Investigate the original pre-pool write after piece allocation failure */
        XASSERT(slot >= 0 && slot < DYNAMIC_OBJECT_COUNT);
        dynamic_objects[slot].angles.pitch_bits = 0x4000;
        transform_point(9437184, 0, -16515072, point.bits);
        slot = dynamic_objects_spawn_oriented(105, x + point.x.bits, 0, z + point.z.bits, angle + 0xffffc000u);
        /* TODO: Investigate the original pre-pool write after piece allocation failure */
        XASSERT(slot >= 0 && slot < DYNAMIC_OBJECT_COUNT);
        dynamic_objects[slot].angles.pitch_bits = 0x4000;
        transform_point(-9437184, 0, 14155776, point.bits);
        slot = dynamic_objects_spawn_oriented(105, x + point.x.bits, 0, z + point.z.bits, angle + 0x4000u);
        /* TODO: Investigate the original pre-pool write after piece allocation failure */
        XASSERT(slot >= 0 && slot < DYNAMIC_OBJECT_COUNT);
        dynamic_objects[slot].angles.pitch_bits = 0x4000;
        transform_point(-9437184, 0, -16515072, point.bits);
        slot = dynamic_objects_spawn_oriented(105, x + point.x.bits, 0, z + point.z.bits, angle + 0x4000u);
        stage = game_session.stage;
        /* TODO: Investigate the original pre-pool write after piece allocation failure */
        XASSERT(slot >= 0 && slot < DYNAMIC_OBJECT_COUNT);
        dynamic_objects[slot].angles.pitch_bits = 0x4000;
        if (stage == 0 || stage == 10)
        {
            kind = 504;
            if (stage == 0)
            {
                if (model->kind == 110)
                    kind = 506;
            }
            else if ((sint32)model->pos.z.bits < 0)
                kind = 503;
            transform_point(0, 0, 0x1000000, point.bits);
            slot = dynamic_objects_spawn_item(kind, x + point.x.bits, 0xffb00000u, z + point.z.bits, angle);
            /* TODO: Investigate the original pre-pool write after piece allocation failure */
            XASSERT(slot >= 0 && slot < DYNAMIC_OBJECT_COUNT);
            drop = dynamic_objects + slot;
            /* TODO: Investigate a spawned item with a foreign native owner */
            XASSERT(drop->payload_kind == MODEL_PAYLOAD_PHYSICS);
            drop->physics.speed = 70;
            drop->physics.vertical_speed = 96;
            drop->physics.obstacle_flags = 1;
        }
        MODEL_OBJECT *next;
        MODEL_OBJECT *next_model2 = NULL;
        uint32 next_idx2;
        for (next_idx2 = 0; next_idx2 + 1 < WORLD_OBJECT_COUNT; next_idx2++)
            if (model == world_objects.object + next_idx2)
            {
                next_model2 = world_objects.object + next_idx2 + 1;
                break;
            }
        if (!next_model2)
            for (next_idx2 = 0; next_idx2 + 1 < DYNAMIC_OBJECT_COUNT; next_idx2++)
                if (model == dynamic_objects + next_idx2)
                {
                    next_model2 = dynamic_objects + next_idx2 + 1;
                    break;
                }
        next = next_model2;
        /* TODO: Investigate damage common fields outside the native object pools */
        XASSERT(next);
        next->kind = 55;
        transform_pop();
    }
    return 2;
}

/* 205C4: staged object rotation and destruction. Original721 writes the
 * caller's position vector through the matrix transform */
static sint32 damage_rotating_pair(COLLISION_REGION *obstacle, MODEL_OBJECT *model, COLLISION_POINT *contact, sint32 actor_idx)
{
    FUNCTION_MARKER(0x800205c4u, "GAME.EXE");
    /* TODO: Investigate invalid actor references at the callback boundary */
    XASSERT(actor_idx == -1 || actor_idx >= 0 && actor_idx < ACTOR_COUNT);
    uint32 angle = actor_idx != -1 ? (uint32)(sint32)actor_orientation[actor_idx].yaw : 0, kind, a, b, d, v, x, z, rx, rz;
    FIXED_VEC3 point;
    sint16 health;
    sint32 slot;
    int i;
    dynamic_objects_spawn_transient(865, contact->pos.x, contact->pos.y, contact->pos.z);
    for (i = 0; i < 8; i++)
        effect_spawn_spark_reverse(contact->pos.x, contact->pos.y, contact->pos.z, angle);
    /* TODO: Investigate damage common fields outside the native object pools */
    XASSERT(model);
    kind = model->kind;
    if (contact->impact_sound)
        sound_voice_play(23, 192, 6144);
    else
        sound_voice_play(33, 255, 6144);
    a = 384;
    b = 768;
    if (kind == 722)
    {
        col_regions.side = 4;
        obstacle->hits = 1;
    }
    if (kind == 721)
    {
        a = 768;
        b = 1536;
    }
    /* TODO: Investigate damage impulse without the matching native owner */
    XASSERT(model && model->payload_kind == MODEL_PAYLOAD_OSCILLATION);
    if (!col_regions.side)
        model->oscillation.pitch_speed = (uint16)a;
    d = col_regions.side;
    if (d == 1)
    {
        model->oscillation.roll_speed = (uint16)b;
        d = col_regions.side;
    }
    if (d == 2)
        model->oscillation.pitch_speed = (uint16)(-(sint64)(a));
    if (col_regions.side == 3)
        model->oscillation.roll_speed = (uint16)(-(sint64)(b));
    health = (uint16)((uint16)obstacle->hits - 1u);
    obstacle->hits = health;
    if (health >= 0)
        return 2;
    v = stage_audio_slots[0].voice_plus_one;
    if (v)
    {
        sound_voice_stop(v - 1u);
        stage_audio_slots[0].voice_plus_one = 0;
    }
    model->kind = model->kind + 1u;
    obstacle->hits = (uint16)obstacle->hits + 4u;
    kind = model->kind;
    angle = model->angles.yaw;
    if (kind == 721)
    {
        sound_voice_play(114, 255, 6144);
        /* TODO: Investigate pair damage without a native model object */
        XASSERT(model);
        x = model->pos.x.bits;
        z = model->pos.z.bits;
        transform_push();
        transform_identity();
        transform_pitch(32768);
        transform_yaw(angle);
        for (i = 0; i < 24; i++)
        {
            v = random_next() & 255u;
            rx = (v + (v >> 3) - 144u) << 16;
            rz = ((random_next() & 127u) + 0xfffffec4u) << 16;
            transform_point(rx, 0, rz, contact->pos.bits);
            effect_spawn_timed(870, x + contact->pos.x, 0xff600000u, z + contact->pos.z, 0, 0, 0);
        }
        for (i = 0; i < 16; i++)
        {
            v = random_next() & 255u;
            rx = (v + (v >> 3) - 144u) << 16;
            rz = ((random_next() & 127u) + 216u) << 16;
            transform_point(rx, 0, rz, contact->pos.bits);
            effect_spawn_timed(870, x + contact->pos.x, 0xff600000u, z + contact->pos.z, 0, 0, 0);
        }
        transform_pop();
    }
    if (model->kind == 722)
    {
        /* An absent actor would make the original write low RAM at 0x8A */
        if (actor_idx != -1)
            ++actor_bonus_counts[actor_idx];
        sound_voice_play(115, 255, 6144);
        MODEL_OBJECT *next_model3 = NULL;
        uint32 next_idx3;
        for (next_idx3 = 0; next_idx3 + 1 < WORLD_OBJECT_COUNT; next_idx3++)
            if (model == world_objects.object + next_idx3)
            {
                next_model3 = world_objects.object + next_idx3 + 1;
                break;
            }
        if (!next_model3)
            for (next_idx3 = 0; next_idx3 + 1 < DYNAMIC_OBJECT_COUNT; next_idx3++)
                if (model == dynamic_objects + next_idx3)
                {
                    next_model3 = dynamic_objects + next_idx3 + 1;
                    break;
                }
        model = next_model3;
        /* TODO: Investigate pair damage without a native model object */
        XASSERT(model);
        x = model->pos.x.bits;
        z = model->pos.z.bits;
        model->kind = 55;
        transform_push();
        transform_identity();
        transform_pitch(32768);
        transform_yaw(angle);
        transform_point(9437184, 0, 15204352, point.bits);
        slot = dynamic_objects_spawn_oriented(724, x + point.x.bits, 0, z + point.z.bits, angle + 0xffffc000u);
        /* TODO: Investigate the original pre-pool write after piece allocation failure */
        XASSERT(slot >= 0 && slot < DYNAMIC_OBJECT_COUNT);
        dynamic_objects[slot].angles.pitch_bits = 0x4000;
        transform_point(9437184, 0, -15466496, point.bits);
        slot = dynamic_objects_spawn_oriented(724, x + point.x.bits, 0, z + point.z.bits, angle + 0xffffc000u);
        /* TODO: Investigate the original pre-pool write after piece allocation failure */
        XASSERT(slot >= 0 && slot < DYNAMIC_OBJECT_COUNT);
        dynamic_objects[slot].angles.pitch_bits = 0x4000;
        transform_point(-9437184, 0, 15204352, point.bits);
        slot = dynamic_objects_spawn_oriented(724, x + point.x.bits, 0, z + point.z.bits, angle + 0x4000u);
        /* TODO: Investigate the original pre-pool write after piece allocation failure */
        XASSERT(slot >= 0 && slot < DYNAMIC_OBJECT_COUNT);
        dynamic_objects[slot].angles.pitch_bits = 0x4000;
        transform_point(-9437184, 0, -15466496, point.bits);
        slot = dynamic_objects_spawn_oriented(724, x + point.x.bits, 0, z + point.z.bits, angle + 0x4000u);
        /* TODO: Investigate the original pre-pool write after piece allocation failure */
        XASSERT(slot >= 0 && slot < DYNAMIC_OBJECT_COUNT);
        dynamic_objects[slot].angles.pitch_bits = 0x4000;
        transform_pop();
    }
    return 2;
}

/* 20A60: health is inspected, not decremented; aimed debris uses signed yaw>>5. */
static sint32 damage_aimed_debris(COLLISION_REGION *obstacle, MODEL_OBJECT *model, const COLLISION_POINT *contact, sint32 actor_idx)
{
    FUNCTION_MARKER(0x80020a60u, "GAME.EXE");
    /* TODO: Investigate invalid actor references at the callback boundary */
    XASSERT(actor_idx == -1 || actor_idx >= 0 && actor_idx < ACTOR_COUNT);
    uint32 angle = actor_idx != -1 ? (uint32)(sint32)actor_orientation[actor_idx].yaw : 0, kind, v, x, y, z;
    sint16 health = obstacle->hits;
    int i;
    /* TODO: Investigate damage common fields outside the native object pools */
    XASSERT(model);
    kind = model->kind;
    if (health >= 0)
    {
        dynamic_objects_spawn_transient(866, contact->pos.x, contact->pos.y, contact->pos.z);
        for (i = 0; i < 8; i++)
            effect_spawn_spark_reverse(contact->pos.x, contact->pos.y, contact->pos.z, angle);
        if (contact->impact_sound)
            sound_voice_play(23, 192, 6144);
        else
            sound_voice_play(33, 255, 6144);
        return 2;
    }
    obstacle->col_kind = 3;
    if (kind == 78)
        model->kind = 85;
    if (kind == 79)
        model->kind = 86;
    if (kind == 80)
        model->kind = 87;
    if (kind >= 81 && kind <= 84)
        model->kind = 88;
    if (kind == 776)
        model->kind = 86;
    if (kind == 341)
        return 2;
    if (model->kind - 85u < 4u)
    {
        /* Explicit predecessor handoff at the original kind transition, not a lazy getter */
        if (model->payload_kind == MODEL_PAYLOAD_TRAFFIC)
        {
            /* Count and vertical speed receive their original stores immediately below */
            model->bounce = (BOUNCING_PART){.bounces = model->traffic.flags, .speed = model->traffic.speed, .physics_spin = model->traffic.physics_spin, .physics_actor_idx = model->traffic.physics_actor_idx, .physics_bounce_count = model->traffic.physics_bounce_count, .vertical_speed = model->traffic.physics_vertical_speed, .physics_obstacle_flags = model->traffic.physics_obstacle_flags};
            model->payload_kind = MODEL_PAYLOAD_BOUNCING_PART;
        }
        else if (model->payload_kind == MODEL_PAYLOAD_TANK)
        {
            /* The original following stores replace count and vertical speed */
            model->bounce = (BOUNCING_PART){.bounces = model->tank.flags, .speed = model->tank.speed, .physics_spin = model->tank.turret_yaw, .physics_actor_idx = model->tank.physics_actor_idx, .physics_bounce_count = model->tank.physics_bounce_count, .vertical_speed = model->tank.physics_vertical_speed, .physics_obstacle_flags = model->tank.physics_obstacle_flags};
            model->payload_kind = MODEL_PAYLOAD_BOUNCING_PART;
        }
        /* TODO: Investigate a bouncing-part transition without its native predecessor */
        XASSERT(model->payload_kind == MODEL_PAYLOAD_BOUNCING_PART);
        model->bounce.vertical_speed = -128;
        model->bounce.bounces = 1;
    }
    else
    {
        /* TODO: Investigate an invalid owner at the original model field write */
        XASSERT(model);
        switch (model->payload_kind)
        {
            case MODEL_PAYLOAD_UNMIGRATED:
                model->pending_motion.sound_clock = (uint16)-128;
                break;
            case MODEL_PAYLOAD_PANEL:
                model->panel.vertical_speed = (uint16)-128;
                break;
            case MODEL_PAYLOAD_SIDE_PANEL:
                model->panel.vertical_speed = (uint16)-128;
                break;
            case MODEL_PAYLOAD_SPRITE:
                model->spr.vertical_speed = (uint16)-128;
                break;
            case MODEL_PAYLOAD_BALLISTIC:
                model->ballistic.vertical_speed = (uint16)-128;
                break;
            case MODEL_PAYLOAD_DRIFT:
                model->drift.physics_vertical_speed = (uint16)-128;
                break;
            case MODEL_PAYLOAD_SHAKE:
                model->shake.physics_vertical_speed = (uint16)-128;
                break;
            case MODEL_PAYLOAD_GATE_FALL:
                model->gate_fall.physics_vertical_speed = (uint16)-128;
                break;
            case MODEL_PAYLOAD_GATE_FRAGMENT:
                model->gate_fragment.physics_vertical_speed = (uint16)-128;
                break;
            case MODEL_PAYLOAD_ROTATION:
                model->rotation.physics_vertical_speed = (uint16)-128;
                break;
            case MODEL_PAYLOAD_BOUNCING_DROP:
                model->drop.vertical_speed = (uint16)-128;
                break;
            case MODEL_PAYLOAD_BOUNCING_PART:
                model->bounce.vertical_speed = (uint16)-128;
                break;
            case MODEL_PAYLOAD_TRAFFIC:
                model->traffic.physics_vertical_speed = (uint16)-128;
                break;
            case MODEL_PAYLOAD_TANK:
                model->tank.physics_vertical_speed = (uint16)-128;
                break;
            case MODEL_PAYLOAD_OSCILLATION:
                model->oscillation.sound_clock = (uint16)-128;
                break;
            case MODEL_PAYLOAD_COMPANION:
                model->companion.physics_vertical_speed = (uint16)-128;
                break;
            case MODEL_PAYLOAD_LAB_TANK:
                model->lab_tank.vertical_speed = (uint16)-128;
                break;
            case MODEL_PAYLOAD_WOBBLE:
                model->wobble.physics_vertical_speed = (uint16)-128;
                break;
            case MODEL_PAYLOAD_PHYSICS:
                model->physics.vertical_speed = (uint16)-128;
                break;
            case MODEL_PAYLOAD_ARMY_TRUCK:
                model->truck.vertical_speed = (uint16)-128;
                break;
            case MODEL_PAYLOAD_AIR_BIKE:
                model->air_bike.phase = (uint16)-128;
                break;
            case MODEL_PAYLOAD_SCENE_VEHICLE:
                model->scene_vehicle.vertical_speed = (uint16)-128;
                break;
            case MODEL_PAYLOAD_SCENE_HELICOPTER:
                model->scene_helicopter.vertical_speed = (uint16)-128;
                break;
            case MODEL_PAYLOAD_COMBAT_HELICOPTER:
                model->helicopter.hover_height = (uint16)-128;
                break;
            case MODEL_PAYLOAD_PATROL_HELICOPTER:
                model->patrol.vertical_speed = (uint16)-128;
                break;
            case MODEL_PAYLOAD_WAVE:
                model->wave.physics_vertical_speed = (uint16)-128;
                break;
            case MODEL_PAYLOAD_TRAIN:
                model->train.vertical_speed = (uint16)-128;
                break;
            case MODEL_PAYLOAD_SUSPENDED:
                model->suspended.vertical_speed = (uint16)-128;
                break;
            default:
                /* TODO: Investigate an unpublished collision field owner */

                XASSERT(0);
        }
        /* TODO: Investigate an invalid owner at the original model field write */
        XASSERT(model);
        switch (model->payload_kind)
        {
            case MODEL_PAYLOAD_UNMIGRATED:
                model->pending_motion.roll_speed = 1;
                break;
            case MODEL_PAYLOAD_PANEL:
                model->panel.retire_ticks = 1;
                break;
            case MODEL_PAYLOAD_SIDE_PANEL:
                model->panel.roll_step = 1;
                break;
            case MODEL_PAYLOAD_SPRITE:
                model->spr.frame = 1;
                break;
            case MODEL_PAYLOAD_BALLISTIC:
                model->ballistic.pitch_step = 1;
                break;
            case MODEL_PAYLOAD_DRIFT:
                model->drift.flags = 1;
                break;
            case MODEL_PAYLOAD_SHAKE:
                model->shake.ticks = 1;
                break;
            case MODEL_PAYLOAD_GATE_FALL:
                model->gate_fall.pitch_speed = 1;
                break;
            case MODEL_PAYLOAD_GATE_FRAGMENT:
                model->gate_fragment.pitch_speed = 1;
                break;
            case MODEL_PAYLOAD_ROTATION:
                model->rotation.phase = 1;
                break;
            case MODEL_PAYLOAD_BOUNCING_DROP:
                model->drop.flags = 1;
                break;
            case MODEL_PAYLOAD_BOUNCING_PART:
                model->bounce.bounces = 1;
                break;
            case MODEL_PAYLOAD_TRAFFIC:
                model->traffic.flags = 1;
                break;
            case MODEL_PAYLOAD_TANK:
                model->tank.flags = 1;
                break;
            case MODEL_PAYLOAD_OSCILLATION:
                model->oscillation.roll_speed = 1;
                break;
            case MODEL_PAYLOAD_COMPANION:
                model->companion.flags = 1;
                break;
            case MODEL_PAYLOAD_LAB_TANK:
                model->lab_tank.flags = 1;
                break;
            case MODEL_PAYLOAD_WOBBLE:
                model->wobble.ticks = 1;
                break;
            case MODEL_PAYLOAD_PHYSICS:
                model->physics.life = 1;
                break;
            case MODEL_PAYLOAD_ARMY_TRUCK:
                model->truck.flags = 1;
                break;
            case MODEL_PAYLOAD_AIR_BIKE:
                model->air_bike.target[0].x = 1;
                break;
            case MODEL_PAYLOAD_SCENE_VEHICLE:
                model->scene_vehicle.flags = 1;
                break;
            case MODEL_PAYLOAD_SCENE_HELICOPTER:
                model->scene_helicopter.flags = 1;
                break;
            case MODEL_PAYLOAD_COMBAT_HELICOPTER:
                model->helicopter.bob_phase = 1;
                break;
            case MODEL_PAYLOAD_PATROL_HELICOPTER:
                model->patrol.voice_plus_one = 1;
                break;
            case MODEL_PAYLOAD_WAVE:
                model->wave.flags = 1;
                break;
            case MODEL_PAYLOAD_TRAIN:
                model->train.wait_ticks = 1;
                break;
            case MODEL_PAYLOAD_SUSPENDED:
                model->suspended.flags = 1;
                break;
            default:
                /* TODO: Investigate an unpublished collision field owner */

                XASSERT(0);
        }
    }
    /* An absent actor would make the original write low RAM at 0x8A */
    if (actor_idx != -1)
        ++actor_bonus_counts[actor_idx];
    sound_voice_play(19, 255, 6144);
    /* TODO: Investigate damage without a native model object */
    XASSERT(model);
    x = model->pos.x.bits;
    z = model->pos.z.bits;
    effect_spawn(892, x, contact->pos.y, z);
    /* TODO: Investigate damage without a native model object */
    XASSERT(model);
    x = model->pos.x.bits;
    z = model->pos.z.bits;
    effect_spawn(603, x, 0, z);
    for (i = 0; i < 16; i++)
    {
        v = (random_next() & 3u) + 902u;
        angle = ((sint16)model->angles.yaw_bits) >> 5;
        /* TODO: Investigate damage without a native model object */
        XASSERT(model);
        x = model->pos.x.bits;
        y = model->pos.y.bits;
        z = model->pos.z.bits;
        effect_spawn_debris_aimed(v, x, y, z, angle);
    }
    disp_buffers.shake_ticks = 16;
    disp_buffers.flash_ticks = 8;
    return 2;
}

/* 20C84: compound object damage. The kind63 path overwrites the input XYZ. */
static sint32 damage_compound_pair(COLLISION_REGION *obstacle, MODEL_OBJECT *model, COLLISION_POINT *contact, sint32 actor_idx)
{
    FUNCTION_MARKER(0x80020c84u, "GAME.EXE");
    /* TODO: Investigate invalid actor references at the callback boundary */
    XASSERT(actor_idx == -1 || actor_idx >= 0 && actor_idx < ACTOR_COUNT);
    uint32 angle = actor_idx != -1 ? (uint32)(sint32)actor_orientation[actor_idx].yaw : 0, kind, v, a, b, d, x, z, y, age;
    FIXED_VEC3 point;
    sint32 spread, slot;
    sint16 health;
    int i;
    uint32 engine_voice = 0;
    if (obstacle->hits < 0)
    {
        if (model && model->payload_kind == MODEL_PAYLOAD_OSCILLATION)
        {
            // LEGACY FIX: the van engine belongs to the body, not the adjacent shell
            engine_voice = (uint16)model->oscillation.voice_plus_one;
            if (engine_voice)
                sound_voice_stop(engine_voice - 1);
            model->oscillation.voice_plus_one = 0;
        }
        /* An absent actor would make the original write low RAM at 0x8A */
        if (actor_idx != -1)
            ++actor_bonus_counts[actor_idx];
        obstacle->col_kind = 3;
        /* TODO: Investigate damage common fields outside the native object pools */
        XASSERT(model);
        /* Explicit predecessor handoff at the original kind transition, not a lazy getter */
        if (model->payload_kind == MODEL_PAYLOAD_OSCILLATION)
        {
            /* Original stores below immediately replace count and vertical speed */
            model->bounce = (BOUNCING_PART){.bounces = model->oscillation.roll_speed, .speed = model->oscillation.speed, .physics_spin = model->oscillation.pitch_speed, .physics_actor_idx = model->oscillation.phase, .physics_bounce_count = model->oscillation.voice_plus_one, .vertical_speed = model->oscillation.sound_clock, .physics_obstacle_flags = model->oscillation.physics_obstacle_flags};
            model->payload_kind = MODEL_PAYLOAD_BOUNCING_PART;
        }
        /* TODO: Investigate an invalid native predecessor at the bounce transition */
        XASSERT(model->payload_kind == MODEL_PAYLOAD_BOUNCING_PART);
        model->kind = 67;
        model->bounce.vertical_speed = -128;
        model->bounce.bounces = 1;
        MODEL_OBJECT *next_model4 = NULL;
        uint32 next_idx4;
        for (next_idx4 = 0; next_idx4 + 1 < WORLD_OBJECT_COUNT; next_idx4++)
            if (model == world_objects.object + next_idx4)
            {
                next_model4 = world_objects.object + next_idx4 + 1;
                break;
            }
        if (!next_model4)
            for (next_idx4 = 0; next_idx4 + 1 < DYNAMIC_OBJECT_COUNT; next_idx4++)
                if (model == dynamic_objects + next_idx4)
                {
                    next_model4 = dynamic_objects + next_idx4 + 1;
                    break;
                }
        model = next_model4;
        /* TODO: Investigate compound companion voice without the matching native owner */
        XASSERT(model && model->payload_kind == MODEL_PAYLOAD_COMPANION);
        v = model->companion.voice;
        /* TODO: Investigate damage common fields outside the native object pools */
        XASSERT(model);
        model->kind = 861;
        if (!engine_voice)
            sound_voice_stop(v);
        sound_voice_play(19, 255, 6144);
        /* TODO: Investigate damage without a native model object */
        XASSERT(model);
        x = model->pos.x.bits;
        z = model->pos.z.bits;
        effect_spawn(892, x, contact->pos.y, z);
        /* TODO: Investigate damage without a native model object */
        XASSERT(model);
        x = model->pos.x.bits;
        z = model->pos.z.bits;
        effect_spawn(603, x, 0, z);
        for (i = 0; i < 16; i++)
        {
            v = (random_next() & 3u) + 902u;
            /* TODO: Investigate damage without a native model object */
            XASSERT(model);
            x = model->pos.x.bits;
            y = model->pos.y.bits;
            z = model->pos.z.bits;
            effect_spawn_debris(v, x, y, z);
        }
        disp_buffers.shake_ticks = 16;
        disp_buffers.flash_ticks = 8;
        return 2;
    }
    dynamic_objects_spawn_transient(865, contact->pos.x, contact->pos.y, contact->pos.z);
    for (i = 0; i < 8; i++)
        effect_spawn_spark_reverse(contact->pos.x, contact->pos.y, contact->pos.z, angle);
    /* TODO: Investigate damage common fields outside the native object pools */
    XASSERT(model);
    kind = model->kind;
    if (contact->impact_sound)
        sound_voice_play(23, 192, 6144);
    else
        sound_voice_play(33, 255, 6144);
    if (kind == 64)
    {
        col_regions.side = 4;
        obstacle->hits = 1;
    }
    a = kind == 63 ? 768u : 384u;
    b = kind == 63 ? 1536u : 768u;
    /* TODO: Investigate damage impulse without the matching native owner */
    XASSERT(model && model->payload_kind == MODEL_PAYLOAD_OSCILLATION);
    if (!col_regions.side)
        model->oscillation.pitch_speed = (uint16)a;
    d = col_regions.side;
    if (d == 1)
    {
        model->oscillation.roll_speed = (uint16)b;
        d = col_regions.side;
    }
    if (d == 2)
        model->oscillation.pitch_speed = (uint16)(-(sint64)(a));
    if (col_regions.side == 3)
        model->oscillation.roll_speed = (uint16)(-(sint64)(b));
    health = (uint16)((uint16)obstacle->hits - 1);
    obstacle->hits = health;
    if (health >= 0)
        return 2;
    kind = model->kind;
    model->kind = kind == 62 || kind == 66 ? 63 : 64;
    obstacle->hits = (uint16)obstacle->hits + 4;
    kind = model->kind;
    angle = model->angles.yaw;
    if (kind == 63)
    {
        sound_voice_play(114, 255, 6144);
        /* TODO: Investigate damage without a native model object */
        XASSERT(model);
        x = model->pos.x.bits;
        z = model->pos.z.bits;
        transform_push();
        transform_identity();
        transform_pitch(32768);
        transform_yaw(angle);
        for (i = 0; i < 24; i++)
        {
            spread = (random_next() & 511u) - 256;
            spread += spread >> 2;
            v = (0xffffff60u - (random_next() & 31u)) << 16;
            transform_point(v, 0, (uint32)spread << 16, contact->pos.bits);
            y = (0xffffff70u - (random_next() & 127u)) << 16;
            age = random_next() & 3u;
            effect_spawn_timed(871, x + contact->pos.x, y, z + contact->pos.z, age, 0, 0);
            spread = (random_next() & 511u) - 256;
            v = ((random_next() & 31u) + 160u) << 16;
            transform_point(v, 0, (uint32)spread << 16, contact->pos.bits);
            y = (0xffffff70u - (random_next() & 127u)) << 16;
            age = random_next() & 3u;
            effect_spawn_timed(871, x + contact->pos.x, y, z + contact->pos.z, age, 0, 0);
        }
        transform_pop();
        kind = model->kind;
    }
    if (kind == 64)
    {
        if (model->oscillation.voice_plus_one)
        {
            sound_voice_stop(model->oscillation.voice_plus_one - 1);
            model->oscillation.voice_plus_one = 0;
        }
        MODEL_OBJECT *next_model5 = NULL;
        uint32 next_idx5;
        for (next_idx5 = 0; next_idx5 + 1 < WORLD_OBJECT_COUNT; next_idx5++)
            if (model == world_objects.object + next_idx5)
            {
                next_model5 = world_objects.object + next_idx5 + 1;
                break;
            }
        if (!next_model5)
            for (next_idx5 = 0; next_idx5 + 1 < DYNAMIC_OBJECT_COUNT; next_idx5++)
                if (model == dynamic_objects + next_idx5)
                {
                    next_model5 = dynamic_objects + next_idx5 + 1;
                    break;
                }
        model = next_model5;
        /* An absent actor would make the original write low RAM at 0x8A */
        if (actor_idx != -1)
            ++actor_bonus_counts[actor_idx];
        sound_voice_play(115, 255, 6144);
        /* TODO: Investigate damage without a native model object */
        XASSERT(model);
        x = model->pos.x.bits;
        z = model->pos.z.bits;
        model->kind = 55;
        transform_push();
        transform_identity();
        transform_pitch(32768);
        transform_yaw(angle);
        transform_point(11272192, 0, 18874368, point.bits);
        slot = dynamic_objects_spawn_oriented(69, x + point.x.bits, 0, z + point.z.bits, angle + 0xffffc000u);
        /* TODO: Investigate the original pre-pool write after allocation failure */
        XASSERT(slot >= 0 && slot < DYNAMIC_OBJECT_COUNT);
        dynamic_objects[slot].angles.pitch_bits = 0x4000;
        transform_point(11272192, 0, -17825792, point.bits);
        slot = dynamic_objects_spawn_oriented(69, x + point.x.bits, 0, z + point.z.bits, angle + 0xffffc000u);
        /* TODO: Investigate the original pre-pool write after allocation failure */
        XASSERT(slot >= 0 && slot < DYNAMIC_OBJECT_COUNT);
        dynamic_objects[slot].angles.pitch_bits = 0x4000;
        transform_point(-11272192, 0, 18874368, point.bits);
        slot = dynamic_objects_spawn_oriented(69, x + point.x.bits, 0, z + point.z.bits, angle + 0x4000u);
        /* TODO: Investigate the original pre-pool write after allocation failure */
        XASSERT(slot >= 0 && slot < DYNAMIC_OBJECT_COUNT);
        dynamic_objects[slot].angles.pitch_bits = 0x4000;
        transform_point(-11272192, 0, -17825792, point.bits);
        slot = dynamic_objects_spawn_oriented(69, x + point.x.bits, 0, z + point.z.bits, angle + 0x4000u);
        /* TODO: Investigate the original pre-pool write after allocation failure */
        XASSERT(slot >= 0 && slot < DYNAMIC_OBJECT_COUNT);
        dynamic_objects[slot].angles.pitch_bits = 0x4000;
        transform_pop();
    }
    return 2;
}

/* 21220: damage/destruction callback; original167C4 has five arguments. */
static sint32 damage_grid_drop(COLLISION_REGION *obstacle, MODEL_OBJECT *model, const COLLISION_POINT *contact, sint32 actor_idx)
{
    FUNCTION_MARKER(0x80021220u, "GAME.EXE");
    /* TODO: Investigate invalid actor references at the callback boundary */
    XASSERT(actor_idx == -1 || actor_idx >= 0 && actor_idx < ACTOR_COUNT);
    uint32 x, y, z;
    sint16 health;
    sint32 slot;
    dynamic_objects_spawn_transient(865, contact->pos.x, contact->pos.y, contact->pos.z);
    health = (uint16)((uint16)obstacle->hits - 1);
    obstacle->hits = health;
    if (health >= 0)
        return 1;
    /* An absent actor would make the original write low RAM at 0x8A */
    if (actor_idx != -1)
        ++actor_bonus_counts[actor_idx];
    sound_voice_play(113, 255, 6144);
    /* TODO: Investigate damage without a native model object */
    XASSERT(model);
    x = model->pos.x.bits;
    y = model->pos.y.bits;
    z = model->pos.z.bits;
    model->kind = model->kind + 1u;
    slot = dynamic_objects_spawn_item(492, x, y, z, 0);
    /* TODO: Investigate the original pre-pool write after allocation failure */
    XASSERT(slot >= 0 && slot < DYNAMIC_OBJECT_COUNT);
    MODEL_OBJECT *drop = dynamic_objects + slot;
    /* TODO: Investigate an item transfer with a foreign native owner */
    XASSERT(drop->payload_kind == MODEL_PAYLOAD_PHYSICS);
    drop->physics.speed = 64;
    drop->physics.vertical_speed = 0;
    effect_spawn_slow_fragment(x - 0x180000, y - 0x200000, z - 0x100000, 0);
    effect_spawn_slow_fragment(x, y - 0x200000, z - 0x100000, 0);
    effect_spawn_slow_fragment(x + 0x180000, y - 0x200000, z - 0x100000, 0);
    effect_spawn_slow_fragment(x - 0x180000, y, z - 0x100000, 0);
    effect_spawn_slow_fragment(x, y, z - 0x100000, 0);
    effect_spawn_slow_fragment(x + 0x180000, y, z - 0x100000, 0);
    effect_spawn_slow_fragment(x - 0x180000, y + 0x200000, z - 0x100000, 0);
    effect_spawn_slow_fragment(x, y + 0x200000, z - 0x100000, 0);
    effect_spawn_slow_fragment(x + 0x180000, y + 0x200000, z - 0x100000, 0);
    return 1;
}

/* 213F8: type14 multi-stage damage; signed health selects two destruction paths. */
static sint32 damage_wave_gate(COLLISION_REGION *obstacle, MODEL_OBJECT *model, const COLLISION_POINT *contact, sint32 actor_idx)
{
    FUNCTION_MARKER(0x800213f8u, "GAME.EXE");
    /* TODO: Investigate invalid actor references at the callback boundary */
    XASSERT(actor_idx == -1 || actor_idx >= 0 && actor_idx < ACTOR_COUNT);
    uint32 x, z, other_x, other_z, random;
    sint16 health;
    sint32 slot;
    uint16 counter;
    col_regions.named[1] = obstacle;
    /* TODO: Investigate a callback without its matching native payload */
    XASSERT(model && model->payload_kind == MODEL_PAYLOAD_SHAKE);
    OBJECT_SHAKE *shake = &model->shake;
    shake->ticks = 8;
    sound_voice_play(117, 255, 6144);
    if (game_session.scene_phase != 1)
    {
        obstacle->hits = 4;
        dynamic_objects_spawn_transient(866, contact->pos.x, contact->pos.y, contact->pos.z);
        return 1;
    }
    if (obstacle->hits < 0)
    {
        obstacle->col_kind = 0;
        x = model->pos.x.bits;
        z = model->pos.z.bits;
        model->kind = 71;
        sound_voice_play(19, 255, 6144);
        effect_spawn(892, x, contact->pos.y, z);
        other_x = model->pos.x.bits;
        other_z = model->pos.z.bits;
        effect_spawn(603, other_x, 0, other_z);
        disp_buffers.shake_ticks = 16;
        disp_buffers.flash_ticks = 8;
        slot = dynamic_objects_spawn(72, x, 0, z);
        random = random_next();
        /* TODO: Investigate the original pre-pool write after gate allocation failure */
        XASSERT(slot >= 0 && slot < DYNAMIC_OBJECT_COUNT);
        dynamic_objects[slot].gate_fragment.speed = (random & 63) + 448;
        dynamic_objects[slot].angles.yaw_bits = 3072;
        slot = dynamic_objects_spawn(73, x, 0, z);
        random = random_next();
        /* TODO: Investigate the original pre-pool write after gate allocation failure */
        XASSERT(slot >= 0 && slot < DYNAMIC_OBJECT_COUNT);
        dynamic_objects[slot].gate_fragment.speed = (random & 63) + 512;
        dynamic_objects[slot].angles.yaw_bits = (uint16)-3072;
        level_wave.paused = 0;
        return 1;
    }
    dynamic_objects_spawn_transient(865, contact->pos.x, contact->pos.y, contact->pos.z);
    health = obstacle->hits;
    model->kind = health >= 3 ? 70u : health >= 2 ? 74u : health > 0 ? 75u : 76u;
    health = (uint16)((uint16)obstacle->hits - 1u);
    obstacle->hits = health;
    if (health < 0)
    {
        obstacle->hits = 0;
        obstacle->col_kind = 0;
        /* An absent actor would make the original write low RAM at 0x8A */
        counter = actor_idx != -1 ? actor_bonus_counts[actor_idx] : 0;
        level_wave.paused = 0;
        if (actor_idx != -1)
            actor_bonus_counts[actor_idx] = (uint16)(counter + 1);
        /* Preserve the retained origin while the same words become falling state */
        FALLING_GATE fall = {.pitch_speed = model->shake.ticks, .speed = model->shake.speed, .bounces = model->shake.oz, .ox = model->shake.ox, .physics_bounce_count = model->shake.physics_bounce_count, .physics_vertical_speed = model->shake.physics_vertical_speed, .physics_obstacle_flags = model->shake.physics_obstacle_flags};
        model->kind = 77;
        model->gate_fall = fall;
        model->payload_kind = MODEL_PAYLOAD_GATE_FALL;
        model->gate_fall.pitch_speed = 0;
        model->gate_fall.bounces = 0;
    }
    return 1;
}

/* 21628: stage gate precedes damage; preserve captured coordinates across sound calls. */
static sint32 damage_gated_panels(COLLISION_REGION *obstacle, MODEL_OBJECT *model, const COLLISION_POINT *contact, sint32 actor_idx)
{
    FUNCTION_MARKER(0x80021628u, "GAME.EXE");
    /* TODO: Investigate invalid actor references at the callback boundary */
    XASSERT(actor_idx == -1 || actor_idx >= 0 && actor_idx < ACTOR_COUNT);
    uint32 phase = game_session.scene_phase, x, z, kind, px, py;
    uint16 counter;
    sint16 health;
    int i;
    /* TODO: Investigate a callback without its matching native payload */
    XASSERT(model && model->payload_kind == MODEL_PAYLOAD_SHAKE);
    OBJECT_SHAKE *shake = &model->shake;
    shake->ticks = 8;
    if (phase != 1)
    {
        obstacle->hits = 4;
        dynamic_objects_spawn_transient(866, contact->pos.x, contact->pos.y, contact->pos.z);
        return 1;
    }
    dynamic_objects_spawn_transient(865, contact->pos.x, contact->pos.y, contact->pos.z);
    sound_voice_play(117, 255, 6144);
    sound_voice_play(133, 160, 6144);
    health = (uint16)((uint16)obstacle->hits - 1);
    obstacle->hits = health;
    if (health >= 0)
        return 1;
    obstacle->hits = 0;
    obstacle->col_kind = 0;
    x = model->pos.x.bits;
    model->kind = 55;
    shake->ticks = 0;
    shake->oz = 0;
    /* An absent actor would make the original write low RAM at 0x8A */
    counter = actor_idx != -1 ? actor_bonus_counts[actor_idx] : 0;
    z = model->pos.z.bits;
    if (actor_idx != -1)
        actor_bonus_counts[actor_idx] = (uint16)(counter + 1u);
    sound_voice_play(116, 255, 6144);
    sound_voice_play(19, 96, 6144);
    disp_buffers.shake_ticks = 8;
    disp_buffers.flash_ticks = 4;
    for (i = 0; i < 8; i++)
    {
        kind = (random_next() & 3u) + 906u;
        px = x + (((random_next() & 511u) - 256u) << 16);
        py = ((uint32)(-(sint64)(random_next() & 127u))) << 16;
        effect_spawn_debris_aimed(kind, px, py, z, 0);
    }
    for (i = 0; i < 4; i++)
    {
        kind = (random_next() & 3u) + 906u;
        px = x + (((random_next() & 511u) - 256u) << 16);
        py = ((uint32)(-(sint64)(random_next() & 127u))) << 16;
        effect_spawn_debris_aimed(kind, px, py, z, 1024);
    }
    dynamic_objects_spawn_oriented(126, x - 0x12c0000, 0xffa40000, z, 0);
    dynamic_objects_spawn_oriented(126, x - 0x640000, 0xffa40000, z, 0);
    dynamic_objects_spawn_oriented(126, x + 0x640000, 0xffa40000, z, 0);
    dynamic_objects_spawn_oriented(126, x + 0x12c0000, 0xffa40000, z, 0);
    dynamic_objects_spawn_oriented(125, x - 0x12c0000, 0xfeec0000, z, 0);
    dynamic_objects_spawn_oriented(125, x - 0x640000, 0xfeec0000, z, 0);
    dynamic_objects_spawn_oriented(125, x + 0x640000, 0xfeec0000, z, 0);
    dynamic_objects_spawn_oriented(125, x + 0x12c0000, 0xfeec0000, z, 0);
    return 1;
}

/* 21910: debris direction depends on signed X; coordinates reload after every call. */
static sint32 damage_side_panels(COLLISION_REGION *obstacle, MODEL_OBJECT *model, const COLLISION_POINT *contact, sint32 actor_idx)
{
    FUNCTION_MARKER(0x80021910u, "GAME.EXE");
    /* TODO: Investigate invalid actor references at the callback boundary */
    XASSERT(actor_idx == -1 || actor_idx >= 0 && actor_idx < ACTOR_COUNT);
    uint32 angle, kind, x, y, z, v;
    sint16 health;
    int i;
    /* TODO: Investigate a callback without its matching native payload */
    XASSERT(model && model->payload_kind == MODEL_PAYLOAD_SHAKE);
    OBJECT_SHAKE *shake = &model->shake;
    shake->ticks = 8;
    dynamic_objects_spawn_transient(865, contact->pos.x, contact->pos.y, contact->pos.z);
    sound_voice_play(117, 255, 6144);
    sound_voice_play(133, 160, 6144);
    health = (uint16)((uint16)obstacle->hits - 1);
    obstacle->hits = health;
    if (health >= 0)
        return 2;
    obstacle->hits = 0;
    obstacle->col_kind = 0;
    model->kind = 128;
    shake->ticks = 0;
    shake->oz = 0;
    /* An absent actor would make the original write low RAM at 0x8A */
    if (actor_idx != -1)
        ++actor_bonus_counts[actor_idx];
    sound_voice_play(116, 255, 6144);
    sound_voice_play(19, 96, 6144);
    x = model->pos.x.bits;
    disp_buffers.shake_ticks = 8;
    disp_buffers.flash_ticks = 4;
    angle = (sint32)x < 0 ? 1536u : 512u;
    for (i = 0; i < 8; i++)
    {
        kind = (random_next() & 3u) + 906u;
        y = ((uint32)(-(sint64)(random_next() & 127u))) << 16;
        v = random_next();
        z = model->pos.z.bits;
        x = model->pos.x.bits;
        effect_spawn_debris_aimed(kind, x, y, z + (((v & 511u) - 256u) << 16), angle);
    }
    angle ^= 1024u;
    for (i = 0; i < 4; i++)
    {
        kind = (random_next() & 3u) + 906u;
        y = ((uint32)(-(sint64)(random_next() & 127u))) << 16;
        v = random_next();
        z = model->pos.z.bits;
        x = model->pos.x.bits;
        effect_spawn_debris_aimed(kind, x, y, z + (((v & 511u) - 256u) << 16), angle);
    }
    y = model->pos.y.bits;
    z = model->pos.z.bits;
    x = model->pos.x.bits;
    dynamic_objects_spawn_oriented(130, x, y + 0xffa40000, z - 0x1200000, 0);
    y = model->pos.y.bits;
    z = model->pos.z.bits;
    x = model->pos.x.bits;
    dynamic_objects_spawn_oriented(129, x, y + 0xffa40000, z - 0x600000, 0);
    y = model->pos.y.bits;
    z = model->pos.z.bits;
    x = model->pos.x.bits;
    dynamic_objects_spawn_oriented(129, x, y + 0xffa40000, z + 0x600000, 0);
    y = model->pos.y.bits;
    z = model->pos.z.bits;
    x = model->pos.x.bits;
    dynamic_objects_spawn_oriented(131, x, y + 0xffa40000, z + 0x1200000, 0);
    y = model->pos.y.bits;
    z = model->pos.z.bits;
    x = model->pos.x.bits;
    dynamic_objects_spawn_oriented(130, x, y + 0xfeec0000, z - 0x1200000, 0);
    y = model->pos.y.bits;
    z = model->pos.z.bits;
    x = model->pos.x.bits;
    dynamic_objects_spawn_oriented(129, x, y + 0xfeec0000, z - 0x600000, 0);
    y = model->pos.y.bits;
    z = model->pos.z.bits;
    x = model->pos.x.bits;
    dynamic_objects_spawn_oriented(129, x, y + 0xfeec0000, z + 0x600000, 0);
    y = model->pos.y.bits;
    z = model->pos.z.bits;
    x = model->pos.x.bits;
    dynamic_objects_spawn_oriented(131, x, y + 0xfeec0000, z + 0x1200000, 0);
    return 2;
}

/* 21BF8: staged breakable; old649 deliberately falls through into new646. */
static sint32 damage_repeated_drop(COLLISION_REGION *obstacle, MODEL_OBJECT *model, const COLLISION_POINT *contact, sint32 actor_idx)
{
    FUNCTION_MARKER(0x80021bf8u, "GAME.EXE");
    /* TODO: Investigate invalid actor references at the callback boundary */
    XASSERT(actor_idx == -1 || actor_idx >= 0 && actor_idx < ACTOR_COUNT);
    uint32 angle = actor_idx != -1 ? (uint32)(sint32)actor_orientation[actor_idx].yaw : 0;
    uint32 entry_kind;
    /* TODO: Investigate damage common fields outside the native object pools */
    XASSERT(model);
    entry_kind = model->kind;
    uint32 kind, x, z, dx, dz, y, v, h, slot, heading;
    sint16 health;
    sint32 item_slot;
    MODEL_OBJECT *drop;
    int i;
    if (entry_kind == 646 || entry_kind == 674)
        return 2;
    /* TODO: Investigate wobble access without the matching native owner */
    XASSERT(model && model->payload_kind == MODEL_PAYLOAD_WOBBLE);
    model->wobble.ticks = 16;
    dynamic_objects_spawn_transient(865, contact->pos.x, contact->pos.y, contact->pos.z);
    for (i = 0; i < 8; i++)
        effect_spawn_spark_reverse(contact->pos.x, contact->pos.y, contact->pos.z, angle);
    if (contact->impact_sound)
        sound_voice_play(23, 192, 6144);
    else
        sound_voice_play(33, 255, 6144);
    sound_voice_play(133, 96, 6144);
    health = (uint16)obstacle->hits - 1;
    obstacle->hits = health;
    if (health >= 0)
        return 2;
    v = random_next();
    sound_voice_play(114, 160, (v & 511u) | 0x1a00u);
    model->kind = model->kind + 1u;
    h = (uint16)obstacle->hits;
    obstacle->hits = h + 2u;
    if (entry_kind == 672)
        obstacle->hits = h + 4u;
    if (entry_kind == 649)
    {
        /* An absent actor would make the original write low RAM at 0x8A */
        if (actor_idx != -1)
            ++actor_bonus_counts[actor_idx];
        model->kind = 646;
        for (i = 0; i < 2; i++)
        {
            v = random_next();
            /* TODO: Investigate damage without a native model object */
            XASSERT(model);
            x = model->pos.x.bits;
            /* TODO: Investigate damage without a native model object */
            XASSERT(model);
            z = model->pos.z.bits;
            effect_spawn_debris_aimed((v & 3u) + 906u, x, 0xffc00000u, z, 384);
            v = random_next();
            /* TODO: Investigate damage without a native model object */
            XASSERT(model);
            x = model->pos.x.bits;
            /* TODO: Investigate damage without a native model object */
            XASSERT(model);
            z = model->pos.z.bits;
            effect_spawn_debris_aimed((v & 3u) + 906u, x, 0xffc00000u, z, 640);
        }
        v = (random_next() & 8191u) + 45056u;
        /* TODO: Investigate damage without a native model object */
        XASSERT(model);
        x = model->pos.x.bits;
        /* TODO: Investigate damage without a native model object */
        XASSERT(model);
        z = model->pos.z.bits;
        item_slot = dynamic_objects_spawn_item(527, x, 0xff900000u, z, v);
        v = random_next();
        /* TODO: Investigate the original pre-pool write after item allocation failure */
        XASSERT(item_slot >= 0 && item_slot < DYNAMIC_OBJECT_COUNT);
        drop = dynamic_objects + item_slot;
        /* TODO: Investigate a destruction drop with a foreign native owner */
        XASSERT(drop->payload_kind == MODEL_PAYLOAD_PHYSICS);
        drop->physics.speed = (uint16)((v & 15) + 64);
        v = random_next();
        drop->physics.vertical_speed = (uint16)((v & 15) + 96);
        model->wobble.emit_ticks = (uint16)((random_next() & 1023u) + 768u);
        drop->physics.obstacle_flags = 1;
    }
    kind = model->kind;
    /* TODO: Investigate damage without a native model object */
    XASSERT(model);
    x = model->pos.x.bits;
    /* TODO: Investigate damage without a native model object */
    XASSERT(model);
    z = model->pos.z.bits;
    if (kind == 646 || kind == 674)
    {
        /* An absent actor would make the original write low RAM at 0x8A */
        if (actor_idx != -1)
            ++actor_bonus_counts[actor_idx];
        model->wobble.ticks = 0;
        model->wobble.emit_ticks = 240;
        for (i = 0; i < 2; i++)
        {
            v = random_next();
            /* TODO: Investigate damage without a native model object */
            XASSERT(model);
            dx = model->pos.x.bits;
            /* TODO: Investigate damage without a native model object */
            XASSERT(model);
            dz = model->pos.z.bits;
            effect_spawn_debris_aimed((v & 3u) + 906u, dx, 0xffc00000u, dz, 384);
            v = random_next();
            /* TODO: Investigate damage without a native model object */
            XASSERT(model);
            dx = model->pos.x.bits;
            /* TODO: Investigate damage without a native model object */
            XASSERT(model);
            dz = model->pos.z.bits;
            effect_spawn_debris_aimed((v & 3u) + 906u, dx, 0xffc00000u, dz, 640);
        }
        for (i = 0; i < 32; i++)
        {
            dx = ((random_next() & 127u) - 64u) << 16;
            dz = ((random_next() & 127u) - 64u) << 16;
            y = (0xffffffa0u - (random_next() & 127u)) << 16;
            effect_spawn_timed(870, x + dx, y, z + dz, 0, 0, 0);
        }
        v = (random_next() & 8191u) + 45056u;
        /* TODO: Investigate damage without a native model object */
        XASSERT(model);
        dx = model->pos.x.bits;
        /* TODO: Investigate damage without a native model object */
        XASSERT(model);
        dz = model->pos.z.bits;
        item_slot = dynamic_objects_spawn_item(491, dx, 0xffb00000u, dz, v);
        v = random_next();
        /* TODO: Investigate the original pre-pool write after item allocation failure */
        XASSERT(item_slot >= 0 && item_slot < DYNAMIC_OBJECT_COUNT);
        drop = dynamic_objects + item_slot;
        /* TODO: Investigate a destruction drop with a foreign native owner */
        XASSERT(drop->payload_kind == MODEL_PAYLOAD_PHYSICS);
        drop->physics.speed = (uint16)((v & 15) + 70);
        v = random_next();
        drop->physics.vertical_speed = (uint16)((v & 15) + 96);
        drop->physics.obstacle_flags = 1;
        return 2;
    }
    if (kind == 645)
    {
        for (i = 0; i < 2; i++)
        {
            v = random_next();
            /* TODO: Investigate damage without a native model object */
            XASSERT(model);
            dx = model->pos.x.bits;
            /* TODO: Investigate damage without a native model object */
            XASSERT(model);
            dz = model->pos.z.bits;
            effect_spawn_debris_aimed((v & 3u) + 906u, dx, 0xff800000u, dz, 384);
            v = random_next();
            /* TODO: Investigate damage without a native model object */
            XASSERT(model);
            dx = model->pos.x.bits;
            /* TODO: Investigate damage without a native model object */
            XASSERT(model);
            dz = model->pos.z.bits;
            effect_spawn_debris_aimed((v & 3u) + 906u, dx, 0xff800000u, dz, 640);
        }
    }
    else if (kind - 643u < 2u)
    {
        do
        {
            item_slot = dynamic_objects_spawn(512, x, 0xffb00000u, z);
            v = random_next();
            /* TODO: Investigate the original pre-pool write after item allocation failure */
            XASSERT(item_slot >= 0 && item_slot < DYNAMIC_OBJECT_COUNT);
            drop = dynamic_objects + item_slot;
            /* TODO: Investigate a destruction drop with a foreign native owner */
            XASSERT(drop->payload_kind == MODEL_PAYLOAD_PHYSICS);
            drop->physics.speed = (uint16)((v & 15) + 48);
            v = random_next();
            slot = col_drops.angle_cursor;
            kind--;
            drop->physics.vertical_speed = (uint16)((v & 15u) + 64u);
            /* TODO: Investigate an invalid original drop-angle cursor */
            XASSERT(slot < DROP_ANGLE_COUNT);
            heading = drop_angles[slot];
            drop->physics.obstacle_flags = 1;
            col_drops.angle_cursor = (slot + 1u) & 3u;
            dynamic_objects[item_slot].angles.yaw_bits = (uint16)heading;
        } while ((sint32)kind >= 643);
    }
    for (i = 0; i < 16; i++)
    {
        dz = ((random_next() & 127u) - 64u) << 16;
        y = (0xffffff60u - ((random_next() & 127u) - 64u)) << 16;
        effect_spawn_timed(870, x - 0x600000u, y, z + dz, 0, 0, 0);
        dx = ((random_next() & 127u) - 64u) << 16;
        y = (0xffffff60u - ((random_next() & 127u) - 64u)) << 16;
        effect_spawn_timed(870, x + dx, y, z + 0x600000u, 0, 0, 0);
    }
    return 2;
}

static sint32 damage_side_drop(COLLISION_REGION *obstacle, MODEL_OBJECT *model, const COLLISION_POINT *contact, sint32 actor_idx)
{
    FUNCTION_MARKER(0x8002225cu, "GAME.EXE");
    /* TODO: Investigate invalid actor references at the callback boundary */
    XASSERT(actor_idx == -1 || actor_idx >= 0 && actor_idx < ACTOR_COUNT);
    uint32 angle = actor_idx != -1 ? (uint32)(sint32)actor_orientation[actor_idx].yaw : 0, kind, x, z, v;
    /* TODO: Investigate damage common fields outside the native object pools */
    XASSERT(model);
    kind = model->kind;
    uint32 dx, dy, dz;
    sint16 health;
    sint32 slot;
    MODEL_OBJECT *drop;
    int i;
    /* TODO: Investigate wobble access without the matching native owner */
    XASSERT(model && model->payload_kind == MODEL_PAYLOAD_WOBBLE);
    model->wobble.ticks = 16;
    if (kind == 656)
        return 2;
    dynamic_objects_spawn_transient(865, contact->pos.x, contact->pos.y, contact->pos.z);
    for (i = 0; i < 8; i++)
        effect_spawn_spark_reverse(contact->pos.x, contact->pos.y, contact->pos.z, angle);
    if (contact->impact_sound)
        sound_voice_play(23, 192, 6144);
    else
        sound_voice_play(33, 255, 6144);
    /* TODO: Investigate damage without a native model object */
    XASSERT(model);
    x = model->pos.x.bits;
    health = (uint16)obstacle->hits - 1;
    /* TODO: Investigate damage without a native model object */
    XASSERT(model);
    z = model->pos.z.bits;
    obstacle->hits = health;
    if (health >= 0)
        return 2;
    /* An absent actor would make the original write low RAM at 0x8A */
    if (actor_idx != -1)
        ++actor_bonus_counts[actor_idx];
    sound_voice_play(19, 255, 6144);
    effect_spawn(893, x, 0xffc00000u, z);
    for (i = 0; i < 16; i++)
    {
        v = (random_next() & 3u) + 902u;
        /* TODO: Investigate damage without a native model object */
        XASSERT(model);
        dx = model->pos.x.bits;
        /* TODO: Investigate damage without a native model object */
        XASSERT(model);
        dy = model->pos.y.bits;
        /* TODO: Investigate damage without a native model object */
        XASSERT(model);
        dz = model->pos.z.bits;
        effect_spawn_debris(v, dx, dy, dz);
    }
    kind = model->kind;
    /* TODO: Investigate damage without a native model object */
    XASSERT(model);
    v = model->pos.x.bits;
    disp_buffers.shake_ticks = 16;
    disp_buffers.flash_ticks = 4;
    model->kind = kind + 1u;
    if ((sint32)v < 0)
    {
        angle = (random_next() & 8191u) + 12288u;
        slot = dynamic_objects_spawn_item(499, x, 0xffc00000u, z, angle);
        v = random_next();
        /* TODO: Investigate the original pre-pool write after item allocation failure */
        XASSERT(slot >= 0 && slot < DYNAMIC_OBJECT_COUNT);
        drop = dynamic_objects + slot;
        /* TODO: Investigate a destruction drop with a foreign native owner */
        XASSERT(drop->payload_kind == MODEL_PAYLOAD_PHYSICS);
        drop->physics.speed = (uint16)((v & 15) + 50);
        v = random_next();
        drop->physics.vertical_speed = (uint16)((v & 15) + 60);
        drop->physics.obstacle_flags = 1;
        angle = (random_next() & 8191u) + 12288u;
        slot = dynamic_objects_spawn_item(496, x, 0xffb00000u, z, angle);
    }
    else
    {
        kind = (sint32)random_next() % 3 + 587;
        angle = (random_next() & 4095u) + 47104u;
        slot = dynamic_objects_spawn_item(kind, x, 0xffb00000u, z, angle);
    }
    v = random_next();
    /* TODO: Investigate the original pre-pool write after item allocation failure */
    XASSERT(slot >= 0 && slot < DYNAMIC_OBJECT_COUNT);
    drop = dynamic_objects + slot;
    /* TODO: Investigate a destruction drop with a foreign native owner */
    XASSERT(drop->payload_kind == MODEL_PAYLOAD_PHYSICS);
    drop->physics.speed = (uint16)((v & 15) + 50);
    v = random_next();
    drop->physics.vertical_speed = (uint16)((v & 15) + 60);
    drop->physics.obstacle_flags = 1;
    return 2;
}

static sint32 damage_particle_burst(COLLISION_REGION *obstacle, MODEL_OBJECT *model, const COLLISION_POINT *contact, sint32 actor_idx)
{
    FUNCTION_MARKER(0x80022548u, "GAME.EXE");
    /* TODO: Investigate invalid actor references at the callback boundary */
    XASSERT(actor_idx == -1 || actor_idx >= 0 && actor_idx < ACTOR_COUNT);
    uint32 angle = actor_idx != -1 ? (uint32)(sint32)actor_orientation[actor_idx].yaw : 0, x, z, y, v;
    uint16 counter;
    int i;
    /* TODO: Investigate damage common fields outside the native object pools */
    XASSERT(model);
    if (model->kind == 648)
        return 2;
    dynamic_objects_spawn_transient(865, contact->pos.x, contact->pos.y, contact->pos.z);
    for (i = 0; i < 8; i++)
        effect_spawn_spark_reverse(contact->pos.x, contact->pos.y, contact->pos.z, angle);
    if (contact->impact_sound)
        sound_voice_play(23, 192, 6144);
    else
        sound_voice_play(33, 255, 6144);
    /* TODO: Investigate damage without a native model object */
    XASSERT(model);
    x = model->pos.x.bits;
    /* An absent actor would make the original write low RAM at 0x8A */
    counter = actor_idx != -1 ? actor_bonus_counts[actor_idx] : 0;
    /* TODO: Investigate damage without a native model object */
    XASSERT(model);
    z = model->pos.z.bits;
    if (actor_idx != -1)
        actor_bonus_counts[actor_idx] = (uint16)(counter + 1);
    sound_voice_play(113, 255, 6144);
    effect_spawn(894, x, 0xff800000u, z);
    for (i = 0; i < 16; i++)
    {
        y = (0xffffffc0u - (random_next() & 127u)) << 16;
        v = random_next() & 2047u;
        effect_spawn_spark_reverse(x, y, z, v);
    }
    v = model->kind;
    disp_buffers.shake_ticks = 8;
    disp_buffers.flash_ticks = 4;
    model->kind = v + 1;
    return 2;
}

/* 226C4: twelve fragments in two angle groups, then twelve fixed panels.
 * IDA omits the fifth 31DA8 argument: eight use 0, four use 1024. */
static sint32 damage_panel_grid(COLLISION_REGION *obstacle, MODEL_OBJECT *model, const COLLISION_POINT *contact, sint32 actor_idx)
{
    FUNCTION_MARKER(0x800226c4u, "GAME.EXE");
    /* TODO: Investigate invalid actor references at the callback boundary */
    XASSERT(actor_idx == -1 || actor_idx >= 0 && actor_idx < ACTOR_COUNT);
    uint32 x, z, kind, px, py;
    sint16 health;
    int i;
    /* TODO: Investigate an invalid owner at the original model field write */
    XASSERT(model);
    switch (model->payload_kind)
    {
        case MODEL_PAYLOAD_UNMIGRATED:
            model->pending_motion.roll_speed = 8;
            break;
        case MODEL_PAYLOAD_PANEL:
            model->panel.retire_ticks = 8;
            break;
        case MODEL_PAYLOAD_SIDE_PANEL:
            model->panel.roll_step = 8;
            break;
        case MODEL_PAYLOAD_SPRITE:
            model->spr.frame = 8;
            break;
        case MODEL_PAYLOAD_BALLISTIC:
            model->ballistic.pitch_step = 8;
            break;
        case MODEL_PAYLOAD_DRIFT:
            model->drift.flags = 8;
            break;
        case MODEL_PAYLOAD_SHAKE:
            model->shake.ticks = 8;
            break;
        case MODEL_PAYLOAD_GATE_FALL:
            model->gate_fall.pitch_speed = 8;
            break;
        case MODEL_PAYLOAD_GATE_FRAGMENT:
            model->gate_fragment.pitch_speed = 8;
            break;
        case MODEL_PAYLOAD_ROTATION:
            model->rotation.phase = 8;
            break;
        case MODEL_PAYLOAD_BOUNCING_DROP:
            model->drop.flags = 8;
            break;
        case MODEL_PAYLOAD_BOUNCING_PART:
            model->bounce.bounces = 8;
            break;
        case MODEL_PAYLOAD_TRAFFIC:
            model->traffic.flags = 8;
            break;
        case MODEL_PAYLOAD_TANK:
            model->tank.flags = 8;
            break;
        case MODEL_PAYLOAD_OSCILLATION:
            model->oscillation.roll_speed = 8;
            break;
        case MODEL_PAYLOAD_COMPANION:
            model->companion.flags = 8;
            break;
        case MODEL_PAYLOAD_LAB_TANK:
            model->lab_tank.flags = 8;
            break;
        case MODEL_PAYLOAD_WOBBLE:
            model->wobble.ticks = 8;
            break;
        case MODEL_PAYLOAD_PHYSICS:
            model->physics.life = 8;
            break;
        case MODEL_PAYLOAD_ARMY_TRUCK:
            model->truck.flags = 8;
            break;
        case MODEL_PAYLOAD_AIR_BIKE:
            model->air_bike.target[0].x = 8;
            break;
        case MODEL_PAYLOAD_SCENE_VEHICLE:
            model->scene_vehicle.flags = 8;
            break;
        case MODEL_PAYLOAD_SCENE_HELICOPTER:
            model->scene_helicopter.flags = 8;
            break;
        case MODEL_PAYLOAD_COMBAT_HELICOPTER:
            model->helicopter.bob_phase = 8;
            break;
        case MODEL_PAYLOAD_PATROL_HELICOPTER:
            model->patrol.voice_plus_one = 8;
            break;
        case MODEL_PAYLOAD_WAVE:
            model->wave.flags = 8;
            break;
        case MODEL_PAYLOAD_TRAIN:
            model->train.wait_ticks = 8;
            break;
        case MODEL_PAYLOAD_SUSPENDED:
            model->suspended.flags = 8;
            break;
        default:
            /* TODO: Investigate an unpublished collision field owner */

            XASSERT(0);
    }
    dynamic_objects_spawn_transient(865, contact->pos.x, contact->pos.y, contact->pos.z);
    sound_voice_play(133, 160, 6144);
    health = (uint16)((uint16)obstacle->hits - 1);
    obstacle->hits = health;
    if (health >= 0)
        return 1;
    /* An absent actor would make the original write low RAM at 0x8A */
    if (actor_idx != -1)
        ++actor_bonus_counts[actor_idx];
    sound_voice_play(116, 255, 6144);
    sound_voice_play(19, 96, 6144);
    obstacle->hits = 0;
    obstacle->col_kind = 0;
    /* TODO: Investigate damage without a native model object */
    XASSERT(model);
    x = model->pos.x.bits;
    /* TODO: Investigate damage without a native model object */
    XASSERT(model);
    z = model->pos.z.bits;
    model->kind = 55;
    disp_buffers.shake_ticks = 8;
    /* TODO: Investigate an invalid owner at the original model field write */
    XASSERT(model);
    switch (model->payload_kind)
    {
        case MODEL_PAYLOAD_UNMIGRATED:
            model->pending_motion.roll_speed = 0;
            break;
        case MODEL_PAYLOAD_PANEL:
            model->panel.retire_ticks = 0;
            break;
        case MODEL_PAYLOAD_SIDE_PANEL:
            model->panel.roll_step = 0;
            break;
        case MODEL_PAYLOAD_SPRITE:
            model->spr.frame = 0;
            break;
        case MODEL_PAYLOAD_BALLISTIC:
            model->ballistic.pitch_step = 0;
            break;
        case MODEL_PAYLOAD_DRIFT:
            model->drift.flags = 0;
            break;
        case MODEL_PAYLOAD_SHAKE:
            model->shake.ticks = 0;
            break;
        case MODEL_PAYLOAD_GATE_FALL:
            model->gate_fall.pitch_speed = 0;
            break;
        case MODEL_PAYLOAD_GATE_FRAGMENT:
            model->gate_fragment.pitch_speed = 0;
            break;
        case MODEL_PAYLOAD_ROTATION:
            model->rotation.phase = 0;
            break;
        case MODEL_PAYLOAD_BOUNCING_DROP:
            model->drop.flags = 0;
            break;
        case MODEL_PAYLOAD_BOUNCING_PART:
            model->bounce.bounces = 0;
            break;
        case MODEL_PAYLOAD_TRAFFIC:
            model->traffic.flags = 0;
            break;
        case MODEL_PAYLOAD_TANK:
            model->tank.flags = 0;
            break;
        case MODEL_PAYLOAD_OSCILLATION:
            model->oscillation.roll_speed = 0;
            break;
        case MODEL_PAYLOAD_COMPANION:
            model->companion.flags = 0;
            break;
        case MODEL_PAYLOAD_LAB_TANK:
            model->lab_tank.flags = 0;
            break;
        case MODEL_PAYLOAD_WOBBLE:
            model->wobble.ticks = 0;
            break;
        case MODEL_PAYLOAD_PHYSICS:
            model->physics.life = 0;
            break;
        case MODEL_PAYLOAD_ARMY_TRUCK:
            model->truck.flags = 0;
            break;
        case MODEL_PAYLOAD_AIR_BIKE:
            model->air_bike.target[0].x = 0;
            break;
        case MODEL_PAYLOAD_SCENE_VEHICLE:
            model->scene_vehicle.flags = 0;
            break;
        case MODEL_PAYLOAD_SCENE_HELICOPTER:
            model->scene_helicopter.flags = 0;
            break;
        case MODEL_PAYLOAD_COMBAT_HELICOPTER:
            model->helicopter.bob_phase = 0;
            break;
        case MODEL_PAYLOAD_PATROL_HELICOPTER:
            model->patrol.voice_plus_one = 0;
            break;
        case MODEL_PAYLOAD_WAVE:
            model->wave.flags = 0;
            break;
        case MODEL_PAYLOAD_TRAIN:
            model->train.wait_ticks = 0;
            break;
        case MODEL_PAYLOAD_SUSPENDED:
            model->suspended.flags = 0;
            break;
        default:
            /* TODO: Investigate an unpublished collision field owner */

            XASSERT(0);
    }
    /* TODO: Investigate an invalid owner at the original model field write */
    XASSERT(model);
    switch (model->payload_kind)
    {
        case MODEL_PAYLOAD_UNMIGRATED:
            model->pending_motion.pitch_speed = 0;
            break;
        case MODEL_PAYLOAD_PANEL:
            model->panel.initialized = 0;
            break;
        case MODEL_PAYLOAD_SIDE_PANEL:
            model->panel.initialized = 0;
            break;
        case MODEL_PAYLOAD_SPRITE:
            model->spr.rise = 0;
            break;
        case MODEL_PAYLOAD_BALLISTIC:
            model->ballistic.roll_step = 0;
            break;
        case MODEL_PAYLOAD_DRIFT:
            model->drift.phase = 0;
            break;
        case MODEL_PAYLOAD_SHAKE:
            model->shake.oz = 0;
            break;
        case MODEL_PAYLOAD_GATE_FALL:
            model->gate_fall.bounces = 0;
            break;
        case MODEL_PAYLOAD_GATE_FRAGMENT:
            model->gate_fragment.physics_spin = 0;
            break;
        case MODEL_PAYLOAD_ROTATION:
            model->rotation.axis = 0;
            break;
        case MODEL_PAYLOAD_BOUNCING_DROP:
            model->drop.released = 0;
            break;
        case MODEL_PAYLOAD_BOUNCING_PART:
            model->bounce.physics_spin = 0;
            break;
        case MODEL_PAYLOAD_TRAFFIC:
            model->traffic.physics_spin = 0;
            break;
        case MODEL_PAYLOAD_TANK:
            model->tank.turret_yaw = 0;
            break;
        case MODEL_PAYLOAD_OSCILLATION:
            model->oscillation.pitch_speed = 0;
            break;
        case MODEL_PAYLOAD_COMPANION:
            model->companion.physics_spin = 0;
            break;
        case MODEL_PAYLOAD_LAB_TANK:
            model->lab_tank.pitch_speed = 0;
            break;
        case MODEL_PAYLOAD_WOBBLE:
            model->wobble.emit_ticks = 0;
            break;
        case MODEL_PAYLOAD_PHYSICS:
            model->physics.spin = 0;
            break;
        case MODEL_PAYLOAD_ARMY_TRUCK:
            model->truck.active = 0;
            break;
        case MODEL_PAYLOAD_AIR_BIKE:
            model->air_bike.target[1].x = 0;
            break;
        case MODEL_PAYLOAD_SCENE_VEHICLE:
            model->scene_vehicle.start_tick = 0;
            break;
        case MODEL_PAYLOAD_SCENE_HELICOPTER:
            model->scene_helicopter.spin = 0;
            break;
        case MODEL_PAYLOAD_COMBAT_HELICOPTER:
            model->helicopter.phase = 0;
            break;
        case MODEL_PAYLOAD_PATROL_HELICOPTER:
            model->patrol.phase = 0;
            break;
        case MODEL_PAYLOAD_WAVE:
            model->wave.phase = 0;
            break;
        case MODEL_PAYLOAD_TRAIN:
            model->train.spin = 0;
            break;
        case MODEL_PAYLOAD_SUSPENDED:
            model->suspended.phase = 0;
            break;
        default:
            /* TODO: Investigate an unpublished collision field owner */

            XASSERT(0);
    }
    disp_buffers.flash_ticks = 4;
    for (i = 0; i < 8; i++)
    {
        kind = (random_next() & 3u) + 906u;
        px = x + (((random_next() & 511u) - 256u) << 16);
        py = ((uint32)(-(sint64)(random_next() & 127u))) << 16;
        effect_spawn_debris_aimed(kind, px, py, z, 0);
    }
    for (i = 0; i < 4; i++)
    {
        kind = (random_next() & 3u) + 906u;
        px = x + (((random_next() & 511u) - 256u) << 16);
        py = ((uint32)(-(sint64)(random_next() & 127u))) << 16;
        effect_spawn_debris_aimed(kind, px, py, z, 1024);
    }
    dynamic_objects_spawn_oriented(758, x, 0xfe000000, z - 0x2000000, 0);
    dynamic_objects_spawn_oriented(759, x, 0xfe000000, z - 0xc00000, 0);
    dynamic_objects_spawn_oriented(760, x, 0xfe000000, z + 0xc00000, 0);
    dynamic_objects_spawn_oriented(761, x, 0xfe000000, z + 0x2000000, 0);
    dynamic_objects_spawn_oriented(762, x, 0xff000000, z - 0x2000000, 0);
    dynamic_objects_spawn_oriented(763, x, 0xff000000, z - 0xc00000, 0);
    dynamic_objects_spawn_oriented(764, x, 0xff000000, z + 0xc00000, 0);
    dynamic_objects_spawn_oriented(765, x, 0xff000000, z + 0x2000000, 0);
    dynamic_objects_spawn_oriented(766, x, 0, z - 0x2000000, 0);
    dynamic_objects_spawn_oriented(767, x, 0, z - 0xc00000, 0);
    dynamic_objects_spawn_oriented(768, x, 0, z + 0xc00000, 0);
    dynamic_objects_spawn_oriented(769, x, 0, z + 0x2000000, 0);
    return 1;
}

/* 229C0: four RNG draws per fragment; 31B20 receives seven actual arguments. */
static sint32 damage_axis_fragments(COLLISION_REGION *obstacle, MODEL_OBJECT *model, const COLLISION_POINT *contact, sint32 actor_idx)
{
    FUNCTION_MARKER(0x800229c0u, "GAME.EXE");
    /* TODO: Investigate invalid actor references at the callback boundary */
    XASSERT(actor_idx == -1 || actor_idx >= 0 && actor_idx < ACTOR_COUNT);
    uint32 width, x, z, y, v, age, world_x, world_z;
    sint32 maximum, minimum;
    int i;
    /* TODO: Investigate damage common fields outside the native object pools */
    XASSERT(model);
    model->kind = model->kind + 1u;
    obstacle->col_kind = 3;
    /* An absent actor would make the original write low RAM at 0x8A */
    if (actor_idx != -1)
        ++actor_bonus_counts[actor_idx];
    sound_voice_play(113, 255, 6144);
    disp_buffers.flash_ticks = 4;
    for (i = 0; i < 32; i++)
    {
        if ((sint32)model->kind < 225)
        {
            maximum = obstacle->geometry.axis.max_z;
            minimum = obstacle->geometry.axis.min_z;
            width = maximum - minimum;
            x = 0;
            z = width * (random_next() << 1) - (width << 15);
        }
        else
        {
            maximum = obstacle->geometry.axis.max_x;
            minimum = obstacle->geometry.axis.min_x;
            width = maximum - minimum;
            z = 0;
            x = width * (random_next() << 1) - (width << 15);
        }
        v = random_next();
        y = ((uint32)(-(sint64)((v & 255u) + (random_next() & 63u)))) << 16;
        age = random_next() & 3u;
        /* TODO: Investigate damage without a native model object */
        XASSERT(model);
        world_x = model->pos.x.bits;
        /* TODO: Investigate damage without a native model object */
        XASSERT(model);
        world_z = model->pos.z.bits;
        effect_spawn_timed(871, world_x + x, y, world_z + z, age, 0, 0);
    }
    return 1;
}

static sint32 drop_limited_item(COLLISION_REGION *obstacle, MODEL_OBJECT *model, const COLLISION_POINT *contact, sint32 actor_idx)
{
    FUNCTION_MARKER(0x80022b18u, "GAME.EXE");
    MODEL_OBJECT *drop;
    sint32 i, count = 0;
    dynamic_objects_spawn_transient(865, contact->pos.x, contact->pos.y, contact->pos.z);
    for (i = 0; i < 8; i++)
        effect_spawn_spark_reverse(contact->pos.x, contact->pos.y, contact->pos.z, 512);
    if (contact->impact_sound)
        sound_voice_play(23, 192, 6144);
    else
        sound_voice_play(33, 255, 6144);
    for (i = 0; i < DYNAMIC_OBJECT_COUNT; i++)
        if (dynamic_objects[i].kind == 514)
            count++;
    if (count < 4)
    {
        /* TODO: Investigate damage without a native model object */
        XASSERT(model);
        uint32 z = model->pos.z.bits;
        /* TODO: Investigate damage without a native model object */
        XASSERT(model);
        uint32 x = model->pos.x.bits;
        sint32 slot = dynamic_objects_spawn(514, x, 0xff800000u, z - 0x200000u);
        uint32 val;
        val = (random_next() & 15) + 48;
        /* TODO: Investigate the original pre-pool write after limited item allocation failure */
        XASSERT(slot >= 0 && slot < DYNAMIC_OBJECT_COUNT);
        drop = dynamic_objects + slot;
        /* TODO: Investigate a spawned item with a foreign native owner */
        XASSERT(drop->payload_kind == MODEL_PAYLOAD_PHYSICS);
        drop->physics.speed = (uint16)val;
        val = (random_next() & 15) + 64;
        drop->physics.vertical_speed = (uint16)val;
        val = (random_next() & 4095) - 18432;
        dynamic_objects[slot].angles.yaw_bits = (uint16)val;
        drop->physics.obstacle_flags = 1;
    }
    return 2;
}

static sint32 damage_linked_panels(COLLISION_REGION *obstacle, MODEL_OBJECT *model, const COLLISION_POINT *contact, sint32 actor_idx)
{
    FUNCTION_MARKER(0x80022c60u, "GAME.EXE");
    /* TODO: Investigate invalid actor references at the callback boundary */
    XASSERT(actor_idx == -1 || actor_idx >= 0 && actor_idx < ACTOR_COUNT);
    uint32 kind, x, y, z;
    uint16 counter;
    sint16 health;
    sint32 count;
    int i;
    /* TODO: Investigate panel damage without a native object before its first write */
    XASSERT(model);
    switch (model->payload_kind)
    {
        case MODEL_PAYLOAD_UNMIGRATED:
            model->pending_motion.roll_speed = 8;
            break;
        case MODEL_PAYLOAD_PANEL:
            model->panel.retire_ticks = 8;
            break;
        case MODEL_PAYLOAD_SIDE_PANEL:
            model->panel.roll_step = 8;
            break;
        case MODEL_PAYLOAD_SPRITE:
            model->spr.frame = 8;
            break;
        case MODEL_PAYLOAD_BALLISTIC:
            model->ballistic.pitch_step = 8;
            break;
        case MODEL_PAYLOAD_DRIFT:
            model->drift.flags = 8;
            break;
        case MODEL_PAYLOAD_SHAKE:
            model->shake.ticks = 8;
            break;
        case MODEL_PAYLOAD_GATE_FALL:
            model->gate_fall.pitch_speed = 8;
            break;
        case MODEL_PAYLOAD_GATE_FRAGMENT:
            model->gate_fragment.pitch_speed = 8;
            break;
        case MODEL_PAYLOAD_ROTATION:
            model->rotation.phase = 8;
            break;
        case MODEL_PAYLOAD_BOUNCING_DROP:
            model->drop.flags = 8;
            break;
        case MODEL_PAYLOAD_BOUNCING_PART:
            model->bounce.bounces = 8;
            break;
        case MODEL_PAYLOAD_TRAFFIC:
            model->traffic.flags = 8;
            break;
        case MODEL_PAYLOAD_TANK:
            model->tank.flags = 8;
            break;
        case MODEL_PAYLOAD_OSCILLATION:
            model->oscillation.roll_speed = 8;
            break;
        case MODEL_PAYLOAD_COMPANION:
            model->companion.flags = 8;
            break;
        case MODEL_PAYLOAD_LAB_TANK:
            model->lab_tank.flags = 8;
            break;
        case MODEL_PAYLOAD_WOBBLE:
            model->wobble.ticks = 8;
            break;
        case MODEL_PAYLOAD_PHYSICS:
            model->physics.life = 8;
            break;
        case MODEL_PAYLOAD_ARMY_TRUCK:
            model->truck.flags = 8;
            break;
        case MODEL_PAYLOAD_AIR_BIKE:
            model->air_bike.target[0].x = 8;
            break;
        case MODEL_PAYLOAD_SCENE_VEHICLE:
            model->scene_vehicle.flags = 8;
            break;
        case MODEL_PAYLOAD_SCENE_HELICOPTER:
            model->scene_helicopter.flags = 8;
            break;
        case MODEL_PAYLOAD_COMBAT_HELICOPTER:
            model->helicopter.bob_phase = 8;
            break;
        case MODEL_PAYLOAD_PATROL_HELICOPTER:
            model->patrol.voice_plus_one = 8;
            break;
        case MODEL_PAYLOAD_WAVE:
            model->wave.flags = 8;
            break;
        case MODEL_PAYLOAD_TRAIN:
            model->train.wait_ticks = 8;
            break;
        case MODEL_PAYLOAD_SUSPENDED:
            model->suspended.flags = 8;
            break;
        default:
            /* TODO: Investigate an unpublished collision field owner */

            XASSERT(0);
    }
    dynamic_objects_spawn_transient(865, contact->pos.x, contact->pos.y, contact->pos.z);
    sound_voice_play(117, 255, 6144);
    health = (uint16)((uint16)obstacle->hits - 1);
    obstacle->hits = health;
    if (health >= 0)
        return 2;
    /* An absent actor would make the original write low RAM at 0x8A */
    counter = actor_idx != -1 ? actor_bonus_counts[actor_idx] : 0;
    count = world_objects.count;
    if (actor_idx != -1)
        actor_bonus_counts[actor_idx] = (uint16)(counter + 1);
    obstacle->hits = 0;
    obstacle->col_kind = 0;
    /* TODO: Investigate damage common fields outside the native object pools */
    XASSERT(model);
    model->kind = 55;
    /* TODO: Investigate an invalid owner at the original model field write */
    XASSERT(model);
    switch (model->payload_kind)
    {
        case MODEL_PAYLOAD_UNMIGRATED:
            model->pending_motion.roll_speed = 0;
            break;
        case MODEL_PAYLOAD_PANEL:
            model->panel.retire_ticks = 0;
            break;
        case MODEL_PAYLOAD_SIDE_PANEL:
            model->panel.roll_step = 0;
            break;
        case MODEL_PAYLOAD_SPRITE:
            model->spr.frame = 0;
            break;
        case MODEL_PAYLOAD_BALLISTIC:
            model->ballistic.pitch_step = 0;
            break;
        case MODEL_PAYLOAD_DRIFT:
            model->drift.flags = 0;
            break;
        case MODEL_PAYLOAD_SHAKE:
            model->shake.ticks = 0;
            break;
        case MODEL_PAYLOAD_GATE_FALL:
            model->gate_fall.pitch_speed = 0;
            break;
        case MODEL_PAYLOAD_GATE_FRAGMENT:
            model->gate_fragment.pitch_speed = 0;
            break;
        case MODEL_PAYLOAD_ROTATION:
            model->rotation.phase = 0;
            break;
        case MODEL_PAYLOAD_BOUNCING_DROP:
            model->drop.flags = 0;
            break;
        case MODEL_PAYLOAD_BOUNCING_PART:
            model->bounce.bounces = 0;
            break;
        case MODEL_PAYLOAD_TRAFFIC:
            model->traffic.flags = 0;
            break;
        case MODEL_PAYLOAD_TANK:
            model->tank.flags = 0;
            break;
        case MODEL_PAYLOAD_OSCILLATION:
            model->oscillation.roll_speed = 0;
            break;
        case MODEL_PAYLOAD_COMPANION:
            model->companion.flags = 0;
            break;
        case MODEL_PAYLOAD_LAB_TANK:
            model->lab_tank.flags = 0;
            break;
        case MODEL_PAYLOAD_WOBBLE:
            model->wobble.ticks = 0;
            break;
        case MODEL_PAYLOAD_PHYSICS:
            model->physics.life = 0;
            break;
        case MODEL_PAYLOAD_ARMY_TRUCK:
            model->truck.flags = 0;
            break;
        case MODEL_PAYLOAD_AIR_BIKE:
            model->air_bike.target[0].x = 0;
            break;
        case MODEL_PAYLOAD_SCENE_VEHICLE:
            model->scene_vehicle.flags = 0;
            break;
        case MODEL_PAYLOAD_SCENE_HELICOPTER:
            model->scene_helicopter.flags = 0;
            break;
        case MODEL_PAYLOAD_COMBAT_HELICOPTER:
            model->helicopter.bob_phase = 0;
            break;
        case MODEL_PAYLOAD_PATROL_HELICOPTER:
            model->patrol.voice_plus_one = 0;
            break;
        case MODEL_PAYLOAD_WAVE:
            model->wave.flags = 0;
            break;
        case MODEL_PAYLOAD_TRAIN:
            model->train.wait_ticks = 0;
            break;
        case MODEL_PAYLOAD_SUSPENDED:
            model->suspended.flags = 0;
            break;
        default:
            /* TODO: Investigate an unpublished collision field owner */

            XASSERT(0);
    }
    /* TODO: Investigate an invalid owner at the original model field write */
    XASSERT(model);
    switch (model->payload_kind)
    {
        case MODEL_PAYLOAD_UNMIGRATED:
            model->pending_motion.pitch_speed = 0;
            break;
        case MODEL_PAYLOAD_PANEL:
            model->panel.initialized = 0;
            break;
        case MODEL_PAYLOAD_SIDE_PANEL:
            model->panel.initialized = 0;
            break;
        case MODEL_PAYLOAD_SPRITE:
            model->spr.rise = 0;
            break;
        case MODEL_PAYLOAD_BALLISTIC:
            model->ballistic.roll_step = 0;
            break;
        case MODEL_PAYLOAD_DRIFT:
            model->drift.phase = 0;
            break;
        case MODEL_PAYLOAD_SHAKE:
            model->shake.oz = 0;
            break;
        case MODEL_PAYLOAD_GATE_FALL:
            model->gate_fall.bounces = 0;
            break;
        case MODEL_PAYLOAD_GATE_FRAGMENT:
            model->gate_fragment.physics_spin = 0;
            break;
        case MODEL_PAYLOAD_ROTATION:
            model->rotation.axis = 0;
            break;
        case MODEL_PAYLOAD_BOUNCING_DROP:
            model->drop.released = 0;
            break;
        case MODEL_PAYLOAD_BOUNCING_PART:
            model->bounce.physics_spin = 0;
            break;
        case MODEL_PAYLOAD_TRAFFIC:
            model->traffic.physics_spin = 0;
            break;
        case MODEL_PAYLOAD_TANK:
            model->tank.turret_yaw = 0;
            break;
        case MODEL_PAYLOAD_OSCILLATION:
            model->oscillation.pitch_speed = 0;
            break;
        case MODEL_PAYLOAD_COMPANION:
            model->companion.physics_spin = 0;
            break;
        case MODEL_PAYLOAD_LAB_TANK:
            model->lab_tank.pitch_speed = 0;
            break;
        case MODEL_PAYLOAD_WOBBLE:
            model->wobble.emit_ticks = 0;
            break;
        case MODEL_PAYLOAD_PHYSICS:
            model->physics.spin = 0;
            break;
        case MODEL_PAYLOAD_ARMY_TRUCK:
            model->truck.active = 0;
            break;
        case MODEL_PAYLOAD_AIR_BIKE:
            model->air_bike.target[1].x = 0;
            break;
        case MODEL_PAYLOAD_SCENE_VEHICLE:
            model->scene_vehicle.start_tick = 0;
            break;
        case MODEL_PAYLOAD_SCENE_HELICOPTER:
            model->scene_helicopter.spin = 0;
            break;
        case MODEL_PAYLOAD_COMBAT_HELICOPTER:
            model->helicopter.phase = 0;
            break;
        case MODEL_PAYLOAD_PATROL_HELICOPTER:
            model->patrol.phase = 0;
            break;
        case MODEL_PAYLOAD_WAVE:
            model->wave.phase = 0;
            break;
        case MODEL_PAYLOAD_TRAIN:
            model->train.spin = 0;
            break;
        case MODEL_PAYLOAD_SUSPENDED:
            model->suspended.phase = 0;
            break;
        default:
            /* TODO: Investigate an unpublished collision field owner */

            XASSERT(0);
    }
    for (i = 0; i < count; i++)
    {
        /* TODO: Investigate a linked-panel scan beyond the world pool */
        XASSERT(i < WORLD_OBJECT_COUNT);
        if (world_objects.object[i].kind == 348)
        {
            /* TODO: Investigate an unowned linked target without importing arbitrary RAM */
            XASSERT(world_objects.object[i].payload_kind == MODEL_PAYLOAD_SHAKE);
            world_objects.object[i].kind = 349;
            break;
        }
    }
    sound_voice_play(19, 255, 6144);
    disp_buffers.shake_ticks = 8;
    disp_buffers.flash_ticks = 4;
    for (i = 0; i < 12; i++)
    {
        kind = (random_next() & 3u) + 906u;
        y = ((uint32)(-(sint64)(random_next() & 127u))) << 16;
        z = ((random_next() & 511u) - 256u) << 16;
        /* TODO: Investigate damage without a native model object */
        XASSERT(model);
        z += model->pos.z.bits;
        /* TODO: Investigate damage without a native model object */
        XASSERT(model);
        x = model->pos.x.bits;
        effect_spawn_debris_aimed(kind, x, y, z, 512);
    }
    y = 0xfe700000;
    /* TODO: Investigate damage without a native model object */
    XASSERT(model);
    z = model->pos.z.bits;
    /* TODO: Investigate damage without a native model object */
    XASSERT(model);
    x = model->pos.x.bits;
    dynamic_objects_spawn_oriented(350, x, y, z + 0x1000000, 0);
    y = 0xfe700000;
    /* TODO: Investigate damage without a native model object */
    XASSERT(model);
    x = model->pos.x.bits;
    /* TODO: Investigate damage without a native model object */
    XASSERT(model);
    z = model->pos.z.bits;
    dynamic_objects_spawn_oriented(351, x, y, z, 0);
    y = 0xfe700000;
    /* TODO: Investigate damage without a native model object */
    XASSERT(model);
    z = model->pos.z.bits;
    /* TODO: Investigate damage without a native model object */
    XASSERT(model);
    x = model->pos.x.bits;
    dynamic_objects_spawn_oriented(352, x, y, z - 0x1000000, 0);
    y = 0xff100000;
    /* TODO: Investigate damage without a native model object */
    XASSERT(model);
    z = model->pos.z.bits;
    /* TODO: Investigate damage without a native model object */
    XASSERT(model);
    x = model->pos.x.bits;
    dynamic_objects_spawn_oriented(353, x, y, z + 0x1000000, 0);
    y = 0xff100000;
    /* TODO: Investigate damage without a native model object */
    XASSERT(model);
    x = model->pos.x.bits;
    /* TODO: Investigate damage without a native model object */
    XASSERT(model);
    z = model->pos.z.bits;
    dynamic_objects_spawn_oriented(354, x, y, z, 0);
    y = 0xff100000;
    /* TODO: Investigate damage without a native model object */
    XASSERT(model);
    z = model->pos.z.bits;
    /* TODO: Investigate damage without a native model object */
    XASSERT(model);
    x = model->pos.x.bits;
    dynamic_objects_spawn_oriented(355, x, y, z - 0x1000000, 0);
    y = 0xffb00000;
    /* TODO: Investigate damage without a native model object */
    XASSERT(model);
    z = model->pos.z.bits;
    /* TODO: Investigate damage without a native model object */
    XASSERT(model);
    x = model->pos.x.bits;
    dynamic_objects_spawn_oriented(356, x, y, z + 0x1000000, 0);
    y = 0xffb00000;
    /* TODO: Investigate damage without a native model object */
    XASSERT(model);
    x = model->pos.x.bits;
    /* TODO: Investigate damage without a native model object */
    XASSERT(model);
    z = model->pos.z.bits;
    dynamic_objects_spawn_oriented(357, x, y, z, 0);
    y = 0xffb00000;
    /* TODO: Investigate damage without a native model object */
    XASSERT(model);
    z = model->pos.z.bits;
    /* TODO: Investigate damage without a native model object */
    XASSERT(model);
    x = model->pos.x.bits;
    dynamic_objects_spawn_oriented(358, x, y, z - 0x1000000, 0);
    return 2;
}

/* 22EDC: clamp AFTER the original signed16 wrap of four times angle delta. */
static sint32 launch_rolling_object(COLLISION_REGION *obstacle, MODEL_OBJECT *model, const COLLISION_POINT *contact, sint32 actor_idx)
{
    FUNCTION_MARKER(0x80022edcu, "GAME.EXE");
    uint32 angle, v;
    sint32 delta, object_x, object_z, actor_x, actor_z;
    if (actor_idx == -1)
        return 0;
    dynamic_objects_spawn_transient(865, contact->pos.x, contact->pos.y, contact->pos.z);
    if (contact->impact_sound)
        sound_voice_play(23, 192, 6144);
    else
        sound_voice_play(33, 255, 6144);
    sound_voice_play(232, 255, 6144);
    /* TODO: Investigate damage without a native model object */
    XASSERT(model);
    /* TODO: Investigate a rolling launch with a foreign native payload */
    XASSERT(model->payload_kind == MODEL_PAYLOAD_PHYSICS);
    object_x = model->pos.x.whole;
    /* TODO: Investigate damage without a native model object */
    XASSERT(model);
    object_z = model->pos.z.whole;
    model->physics.speed = 40;
    model->physics.actor_idx = (uint16)actor_idx;
    /* TODO: Investigate an invalid actor at the launch coordinate read */
    XASSERT(actor_idx >= 0 && actor_idx < ACTOR_COUNT);
    actor_x = actor_positions[actor_idx].x.whole;
    actor_z = actor_positions[actor_idx].z.whole;
    angle = math_dir11(actor_x, actor_z, object_x, object_z);
    model->physics.heading = (uint16)angle;
    v = (angle - ((uint16)actor_orientation[actor_idx].yaw)) << 2;
    delta = (sint16)v;
    model->physics.roll_step = (uint16)v;
    if (delta > 512)
        model->physics.roll_step = 512;
    else if (delta < -512)
        model->physics.roll_step = (uint16)-512;
    return 0;
}

/* 22FF0: quadrant-dependent impulse and release of the linked object slot. */
static sint32 launch_linked_object(COLLISION_REGION *obstacle, MODEL_OBJECT *model, const COLLISION_POINT *contact, sint32 actor_idx)
{
    FUNCTION_MARKER(0x80022ff0u, "GAME.EXE");
    uint32 angle;
    sint32 object_x, object_z, actor_x, actor_z, linked;
    if (actor_idx == -1)
        return 0;
    dynamic_objects_spawn_transient(865, contact->pos.x, contact->pos.y, contact->pos.z);
    sound_voice_play(133, 1286, 6400);
    if (contact->impact_sound)
        sound_voice_play(23, 192, 6144);
    else
        sound_voice_play(33, 255, 6144);
    /* TODO: Investigate damage without a native model object */
    XASSERT(model);
    object_x = model->pos.x.whole;
    /* TODO: Investigate damage without a native model object */
    XASSERT(model);
    object_z = model->pos.z.whole;
    /* TODO: Investigate an invalid actor at the launch coordinate read */
    XASSERT(actor_idx >= 0 && actor_idx < ACTOR_COUNT);
    actor_x = actor_positions[actor_idx].x.whole;
    actor_z = actor_positions[actor_idx].z.whole;
    angle = (math_dir11(object_x, object_z, actor_x, actor_z) + 256u) & 2047u;
    /* TODO: Investigate a linked-object callback without its native owner */
    XASSERT(model && model->payload_kind == MODEL_PAYLOAD_ROTATION);
    ROTATION_SHAKE *rotation = &model->rotation;
    rotation->amplitude = 96;
    if (angle < 512)
    {
        rotation->axis = 1;
        rotation->phase = 1023;
    }
    else if (angle < 1024)
    {
        rotation->phase = 1023;
        rotation->axis = 0;
    }
    else if (angle < 1536)
    {
        rotation->axis = 1;
        rotation->phase = 2047;
    }
    else
    {
        rotation->phase = 2047;
        rotation->axis = 0;
    }
    rotation->phase_step = 8192;
    /* TODO: Investigate malformed native references instead of treating them as absent */
    XASSERT(rotation->linked.present <= 1 && (!rotation->linked.present || rotation->linked.idx < WORLD_OBJECT_COUNT));
    linked = rotation->linked.present ? (sint32)rotation->linked.idx : -1;
    if (linked != -1)
    {
        /* TODO: Investigate an invalid linked world slot before the original store */
        XASSERT(linked >= 0 && linked < WORLD_OBJECT_COUNT);
        /* TODO: Investigate a foreign linked slot or a link back to the rotating parent */
        XASSERT(world_objects.object[linked].payload_kind == MODEL_PAYLOAD_BOUNCING_DROP);
        world_objects.object[linked].drop.released = 1;
        linked = rotation->linked.present ? (sint32)rotation->linked.idx : -1;
        /* TODO: Investigate an invalid linked world slot before the original store */
        XASSERT(linked >= 0 && linked < WORLD_OBJECT_COUNT);
        world_objects.object[linked].drop.speed = 12;
        linked = rotation->linked.present ? (sint32)rotation->linked.idx : -1;
        /* TODO: Investigate an invalid linked world slot before the original store */
        XASSERT(linked >= 0 && linked < WORLD_OBJECT_COUNT);
        world_objects.object[linked].drop.vertical_speed = -8;
        linked = rotation->linked.present ? (sint32)rotation->linked.idx : -1;
        /* TODO: Investigate an invalid linked world slot before the original store */
        XASSERT(linked >= 0 && linked < WORLD_OBJECT_COUNT);
        world_objects.object[linked].angles.yaw_bits = (uint16)((angle - 1280u) << 5);
        linked = rotation->linked.present ? (sint32)rotation->linked.idx : -1;
        /* TODO: Investigate an invalid linked world slot before the original store */
        XASSERT(linked >= 0 && linked < WORLD_OBJECT_COUNT);
        world_objects.object[linked].drop.actor_idx = actor_idx;
        linked = rotation->linked.present ? (sint32)rotation->linked.idx : -1;
        /* TODO: Investigate an invalid linked world slot before the original store */
        XASSERT(linked >= 0 && linked < WORLD_OBJECT_COUNT);
        world_objects.object[linked].drop.bounces = 0;
        linked = rotation->linked.present ? (sint32)rotation->linked.idx : -1;
        /* TODO: Investigate an invalid linked world slot before the original store */
        XASSERT(linked >= 0 && linked < WORLD_OBJECT_COUNT);
        world_objects.object[linked].drop.obstacle_flags = 1;
        rotation->linked = (WORLD_OBJECT_REFERENCE){0};
    }
    return 0;
}

/* 2322C: rotating drop table, reloaded after each RNG/callee as in MIPS. */
static sint32 damage_cyclic_drop(COLLISION_REGION *obstacle, MODEL_OBJECT *model, const COLLISION_POINT *contact, sint32 actor_idx)
{
    FUNCTION_MARKER(0x8002322cu, "GAME.EXE");
    MODEL_OBJECT *drop;
    uint32 x, z, v, kind, debris_x, debris_y, debris_z, drop_idx;
    sint16 health;
    int i;
    sint32 slot;
    dynamic_objects_spawn_transient(865, contact->pos.x, contact->pos.y, contact->pos.z);
    if (contact->impact_sound)
        sound_voice_play(23, 192, 6144);
    else
        sound_voice_play(33, 255, 6144);
    /* TODO: Investigate damage without a native model object */
    XASSERT(model);
    x = model->pos.x.bits;
    v = (uint16)obstacle->hits;
    /* TODO: Investigate damage without a native model object */
    XASSERT(model);
    z = model->pos.z.bits;
    health = (uint16)(v - 1);
    obstacle->hits = health;
    if (health >= 0)
        return 2;
    obstacle->hits = 0;
    obstacle->col_kind = 0;
    model->kind = 55;
    /* TODO: Investigate an invalid owner at the original model field write */
    XASSERT(model);
    switch (model->payload_kind)
    {
        case MODEL_PAYLOAD_UNMIGRATED:
            model->pending_motion.roll_speed = 0;
            break;
        case MODEL_PAYLOAD_PANEL:
            model->panel.retire_ticks = 0;
            break;
        case MODEL_PAYLOAD_SIDE_PANEL:
            model->panel.roll_step = 0;
            break;
        case MODEL_PAYLOAD_SPRITE:
            model->spr.frame = 0;
            break;
        case MODEL_PAYLOAD_BALLISTIC:
            model->ballistic.pitch_step = 0;
            break;
        case MODEL_PAYLOAD_DRIFT:
            model->drift.flags = 0;
            break;
        case MODEL_PAYLOAD_SHAKE:
            model->shake.ticks = 0;
            break;
        case MODEL_PAYLOAD_GATE_FALL:
            model->gate_fall.pitch_speed = 0;
            break;
        case MODEL_PAYLOAD_GATE_FRAGMENT:
            model->gate_fragment.pitch_speed = 0;
            break;
        case MODEL_PAYLOAD_ROTATION:
            model->rotation.phase = 0;
            break;
        case MODEL_PAYLOAD_BOUNCING_DROP:
            model->drop.flags = 0;
            break;
        case MODEL_PAYLOAD_BOUNCING_PART:
            model->bounce.bounces = 0;
            break;
        case MODEL_PAYLOAD_TRAFFIC:
            model->traffic.flags = 0;
            break;
        case MODEL_PAYLOAD_TANK:
            model->tank.flags = 0;
            break;
        case MODEL_PAYLOAD_OSCILLATION:
            model->oscillation.roll_speed = 0;
            break;
        case MODEL_PAYLOAD_COMPANION:
            model->companion.flags = 0;
            break;
        case MODEL_PAYLOAD_LAB_TANK:
            model->lab_tank.flags = 0;
            break;
        case MODEL_PAYLOAD_WOBBLE:
            model->wobble.ticks = 0;
            break;
        case MODEL_PAYLOAD_PHYSICS:
            model->physics.life = 0;
            break;
        case MODEL_PAYLOAD_ARMY_TRUCK:
            model->truck.flags = 0;
            break;
        case MODEL_PAYLOAD_AIR_BIKE:
            model->air_bike.target[0].x = 0;
            break;
        case MODEL_PAYLOAD_SCENE_VEHICLE:
            model->scene_vehicle.flags = 0;
            break;
        case MODEL_PAYLOAD_SCENE_HELICOPTER:
            model->scene_helicopter.flags = 0;
            break;
        case MODEL_PAYLOAD_COMBAT_HELICOPTER:
            model->helicopter.bob_phase = 0;
            break;
        case MODEL_PAYLOAD_PATROL_HELICOPTER:
            model->patrol.voice_plus_one = 0;
            break;
        case MODEL_PAYLOAD_WAVE:
            model->wave.flags = 0;
            break;
        case MODEL_PAYLOAD_TRAIN:
            model->train.wait_ticks = 0;
            break;
        case MODEL_PAYLOAD_SUSPENDED:
            model->suspended.flags = 0;
            break;
        default:
            /* TODO: Investigate an unpublished collision field owner */

            XASSERT(0);
    }
    /* TODO: Investigate an invalid owner at the original model field write */
    XASSERT(model);
    switch (model->payload_kind)
    {
        case MODEL_PAYLOAD_UNMIGRATED:
            model->pending_motion.pitch_speed = 0;
            break;
        case MODEL_PAYLOAD_PANEL:
            model->panel.initialized = 0;
            break;
        case MODEL_PAYLOAD_SIDE_PANEL:
            model->panel.initialized = 0;
            break;
        case MODEL_PAYLOAD_SPRITE:
            model->spr.rise = 0;
            break;
        case MODEL_PAYLOAD_BALLISTIC:
            model->ballistic.roll_step = 0;
            break;
        case MODEL_PAYLOAD_DRIFT:
            model->drift.phase = 0;
            break;
        case MODEL_PAYLOAD_SHAKE:
            model->shake.oz = 0;
            break;
        case MODEL_PAYLOAD_GATE_FALL:
            model->gate_fall.bounces = 0;
            break;
        case MODEL_PAYLOAD_GATE_FRAGMENT:
            model->gate_fragment.physics_spin = 0;
            break;
        case MODEL_PAYLOAD_ROTATION:
            model->rotation.axis = 0;
            break;
        case MODEL_PAYLOAD_BOUNCING_DROP:
            model->drop.released = 0;
            break;
        case MODEL_PAYLOAD_BOUNCING_PART:
            model->bounce.physics_spin = 0;
            break;
        case MODEL_PAYLOAD_TRAFFIC:
            model->traffic.physics_spin = 0;
            break;
        case MODEL_PAYLOAD_TANK:
            model->tank.turret_yaw = 0;
            break;
        case MODEL_PAYLOAD_OSCILLATION:
            model->oscillation.pitch_speed = 0;
            break;
        case MODEL_PAYLOAD_COMPANION:
            model->companion.physics_spin = 0;
            break;
        case MODEL_PAYLOAD_LAB_TANK:
            model->lab_tank.pitch_speed = 0;
            break;
        case MODEL_PAYLOAD_WOBBLE:
            model->wobble.emit_ticks = 0;
            break;
        case MODEL_PAYLOAD_PHYSICS:
            model->physics.spin = 0;
            break;
        case MODEL_PAYLOAD_ARMY_TRUCK:
            model->truck.active = 0;
            break;
        case MODEL_PAYLOAD_AIR_BIKE:
            model->air_bike.target[1].x = 0;
            break;
        case MODEL_PAYLOAD_SCENE_VEHICLE:
            model->scene_vehicle.start_tick = 0;
            break;
        case MODEL_PAYLOAD_SCENE_HELICOPTER:
            model->scene_helicopter.spin = 0;
            break;
        case MODEL_PAYLOAD_COMBAT_HELICOPTER:
            model->helicopter.phase = 0;
            break;
        case MODEL_PAYLOAD_PATROL_HELICOPTER:
            model->patrol.phase = 0;
            break;
        case MODEL_PAYLOAD_WAVE:
            model->wave.phase = 0;
            break;
        case MODEL_PAYLOAD_TRAIN:
            model->train.spin = 0;
            break;
        case MODEL_PAYLOAD_SUSPENDED:
            model->suspended.phase = 0;
            break;
        default:
            /* TODO: Investigate an unpublished collision field owner */

            XASSERT(0);
    }
    sound_voice_play(19, 255, 6144);
    effect_spawn(893, x, 0xffc00000u, z);
    for (i = 0; i < 16; i++)
    {
        v = (random_next() & 3u) + 902u;
        /* TODO: Investigate damage without a native model object */
        XASSERT(model);
        debris_x = model->pos.x.bits;
        /* TODO: Investigate damage without a native model object */
        XASSERT(model);
        debris_y = model->pos.y.bits;
        /* TODO: Investigate damage without a native model object */
        XASSERT(model);
        debris_z = model->pos.z.bits;
        effect_spawn_debris(v, debris_x, debris_y, debris_z);
    }
    drop_idx = col_drops.type_cursor;
    /* TODO: Investigate an invalid original cyclic drop cursor */
    XASSERT(drop_idx < DROP_TYPE_COUNT);
    kind = drop_types[drop_idx];
    disp_buffers.shake_ticks = 16;
    disp_buffers.flash_ticks = 4;
    if (kind == 496)
    {
        v = (random_next() & 8191u) + 45056u;
        drop_idx = col_drops.type_cursor;
        /* TODO: Investigate an invalid original cyclic drop cursor */
        XASSERT(drop_idx < DROP_TYPE_COUNT);
        kind = drop_types[drop_idx];
        slot = dynamic_objects_spawn_item(kind, x, 0xffc00000u, z, v);
        /* TODO: Investigate the original pre-pool flag write after cyclic allocation failure */
        XASSERT(slot >= 0 && slot < DYNAMIC_OBJECT_COUNT);
        drop = dynamic_objects + slot;
        /* TODO: Investigate a spawned item with a foreign native owner */
        XASSERT(drop->payload_kind == MODEL_PAYLOAD_PHYSICS);
        drop->physics.obstacle_flags = 1;
        v = (random_next() & 8191u) + 45056u;
        drop_idx = col_drops.type_cursor;
        /* TODO: Investigate an invalid original cyclic drop cursor */
        XASSERT(drop_idx < DROP_TYPE_COUNT);
        kind = drop_types[drop_idx];
        slot = dynamic_objects_spawn_item(kind, x + 0x140000u, 0xffc00000u, z, v);
        /* TODO: Investigate the original pre-pool flag write after cyclic allocation failure */
        XASSERT(slot >= 0 && slot < DYNAMIC_OBJECT_COUNT);
        drop = dynamic_objects + slot;
        /* TODO: Investigate a spawned item with a foreign native owner */
        XASSERT(drop->payload_kind == MODEL_PAYLOAD_PHYSICS);
        drop->physics.obstacle_flags = 1;
        v = (random_next() & 8191u) + 45056u;
        drop_idx = col_drops.type_cursor;
        /* TODO: Investigate an invalid original cyclic drop cursor */
        XASSERT(drop_idx < DROP_TYPE_COUNT);
        kind = drop_types[drop_idx];
        slot = dynamic_objects_spawn_item(kind, x, 0xffc00000u, z + 0x140000u, v);
        /* TODO: Investigate the original pre-pool flag write after cyclic allocation failure */
        XASSERT(slot >= 0 && slot < DYNAMIC_OBJECT_COUNT);
        drop = dynamic_objects + slot;
        /* TODO: Investigate a spawned item with a foreign native owner */
        XASSERT(drop->payload_kind == MODEL_PAYLOAD_PHYSICS);
        drop->physics.obstacle_flags = 1;
    }
    else
    {
        v = (random_next() & 8191u) + 45056u;
        drop_idx = col_drops.type_cursor;
        /* TODO: Investigate an invalid original cyclic drop cursor */
        XASSERT(drop_idx < DROP_TYPE_COUNT);
        kind = drop_types[drop_idx];
        slot = dynamic_objects_spawn_item(kind, x, 0xffc00000u, z, v);
        /* TODO: Investigate the original pre-pool flag write after cyclic allocation failure */
        XASSERT(slot >= 0 && slot < DYNAMIC_OBJECT_COUNT);
        drop = dynamic_objects + slot;
        /* TODO: Investigate a spawned item with a foreign native owner */
        XASSERT(drop->payload_kind == MODEL_PAYLOAD_PHYSICS);
        drop->physics.obstacle_flags = 1;
    }
    v = col_drops.type_cursor + 1u;
    col_drops.type_cursor = v;
    if (v == DROP_TYPE_COUNT)
        col_drops.type_cursor = 0;
    return 2;
}

const COLLISION_HANDLER col_handlers[COLLISION_HANDLER_COUNT] = {
    {COLLISION_NO_ARGS, {.no_args = col_noop}},
    {COLLISION_CONTACT, {.contact = damage_replace}},
    {COLLISION_CONTACT, {.contact = damage_drop}},
    {COLLISION_CONTACT, {.contact = damage_staged_drop}},
    {COLLISION_CONTACT, {.contact = damage_wave_obstacle}},
    {COLLISION_MUTABLE_CONTACT, {.mutable_contact = damage_explosive_pair}},
    {COLLISION_MUTABLE_CONTACT, {.mutable_contact = damage_rotating_pair}},
    {COLLISION_CONTACT, {.contact = damage_aimed_debris}},
    {COLLISION_MUTABLE_CONTACT, {.mutable_contact = damage_compound_pair}},
    {COLLISION_CONTACT, {.contact = damage_grid_drop}},
    {COLLISION_CONTACT, {.contact = damage_wave_gate}},
    {COLLISION_CONTACT, {.contact = damage_gated_panels}},
    {COLLISION_CONTACT, {.contact = damage_side_panels}},
    {COLLISION_CONTACT, {.contact = damage_repeated_drop}},
    {COLLISION_CONTACT, {.contact = damage_side_drop}},
    {COLLISION_CONTACT, {.contact = damage_particle_burst}},
    {COLLISION_CONTACT, {.contact = damage_panel_grid}},
    {COLLISION_CONTACT, {.contact = damage_axis_fragments}},
    {COLLISION_CONTACT, {.contact = drop_limited_item}},
    {COLLISION_CONTACT, {.contact = damage_linked_panels}},
    {COLLISION_CONTACT, {.contact = launch_rolling_object}},
    {COLLISION_CONTACT, {.contact = launch_linked_object}},
    {COLLISION_CONTACT, {.contact = damage_cyclic_drop}},
};
