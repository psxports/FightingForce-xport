#include <stddef.h>
#include <stdio.h>

#include "game.h"

_Static_assert(sizeof(ITEM_PROPERTIES) == 4, "Item property word size");
_Static_assert(offsetof(ITEM_PROPERTIES, capabilities) == 0, "Item capability offset");
_Static_assert(offsetof(ITEM_PROPERTIES, damage) == 2, "Item damage offset");

const ITEM_PROPERTIES item_properties[DYNAMIC_ITEM_TYPE_COUNT] = {
    {.packed = 0x00000000}, {.packed = 0x00600001}, {.packed = 0x005a0003}, {.packed = 0x00640002}, {.packed = 0x005a0003}, {.packed = 0x00640002}, {.packed = 0x005a0003}, {.packed = 0x00640002}, {.packed = 0x008c0001}, {.packed = 0x00780001}, {.packed = 0x00500002}, {.packed = 0x00640002}, {.packed = 0x00780002}, {.packed = 0x008c0002}, {.packed = 0x00aa0001}, {.packed = 0x00700001}, {.packed = 0x01100001}, {.packed = 0x00600001}, {.packed = 0x00640001}, {.packed = 0x00820001}, {.packed = 0x00640008}, {.packed = 0x00500008}, {.packed = 0x00500008}, {.packed = 0x00040004}, {.packed = 0x00040004}, {.packed = 0x00040004}, {.packed = 0x00040004}, {.packed = 0x00040004}, {.packed = 0x00040008},
    {.packed = 0x00040008}, {.packed = 0x00280040}, {.packed = 0x00280040}, {.packed = 0x00280040}, {.packed = 0x00280040}, {.packed = 0x00a00008}, {.packed = 0x00600008}, {.packed = 0x00000008}, {.packed = 0x00000008}, {.packed = 0x00000008}, {.packed = 0x00040020}, {.packed = 0x00280008}, {.packed = 0x003c0008}, {.packed = 0x00500008}, {.packed = 0x00780001}, {.packed = 0x00780001}, {.packed = 0x00000008}, {.packed = 0x01400008}, {.packed = 0x01400010}, {.packed = 0x00f00008}, {.packed = 0x00f00010}, {.packed = 0x01600010}, {.packed = 0x00c80010}, {.packed = 0x00c80010}, {.packed = 0x00f00010}, {.packed = 0x00dc0010}, {.packed = 0x00c80010}, {.packed = 0x01180010}, {.packed = 0x01680010},
    {.packed = 0x01680010}, {.packed = 0x00780010}, {.packed = 0x00f00010}, {.packed = 0x00f00010}, {.packed = 0x00a00010}, {.packed = 0x00f00010}, {.packed = 0x01400010}, {.packed = 0x01180010}, {.packed = 0x01680010}, {.packed = 0x01400010}, {.packed = 0x01400010}, {.packed = 0x00780010}, {.packed = 0x00780010}, {.packed = 0x00780010}, {.packed = 0x00780010}, {.packed = 0x00780010}, {.packed = 0x00780010}, {.packed = 0x00780010}, {.packed = 0x00780010}, {.packed = 0x01400010}, {.packed = 0x01400010}, {.packed = 0x01400010}, {.packed = 0x01400010}, {.packed = 0x00c80010}, {.packed = 0x00f00010}, {.packed = 0x01400010}, {.packed = 0x01900010}, {.packed = 0x00780010}, {.packed = 0x00780010},
};

sint8 dynamic_item_uses[DYNAMIC_ITEM_TYPE_COUNT];
MODEL_OBJECT dynamic_objects[DYNAMIC_OBJECT_COUNT];

/* Dynamic slots are distinct from the effect and MPD scene-object pools
 * Storage and callback pointer migration remain coupled to their consumers */

sint32 dynamic_objects_spawn(uint32 kind, uint32 x, uint32 y, uint32 z)
{
    FUNCTION_MARKER(0x80016654, "GAME.EXE");
    sint32 idx;
    for (idx = 0; idx < DYNAMIC_OBJECT_COUNT; idx++)
    {
        if (dynamic_objects[idx].kind != 0xffffffffu)
            continue;
        MODEL_OBJECT *item = dynamic_objects + idx;
        *item = (MODEL_OBJECT){0};
        item->kind = kind;
        if (kind == 302)
            item->payload_kind = MODEL_PAYLOAD_TRAIN;
        else if (kind == 896)
            item->payload_kind = MODEL_PAYLOAD_SUSPENDED;
        else if (kind - 114u < 4u)
            item->payload_kind = MODEL_PAYLOAD_WAVE;
        else if (kind == 125 || kind == 126)
            item->payload_kind = MODEL_PAYLOAD_PANEL;
        else if (kind - 129u < 3u || kind - 350u < 9u || kind - 758u < 12u)
            item->payload_kind = MODEL_PAYLOAD_SIDE_PANEL;
        else if (kind - 575u < 2u || kind - 862u < 5u || kind == 869 || kind - 881u < 5u)
            item->payload_kind = MODEL_PAYLOAD_SPRITE;
        else if (kind - 679u < 4u)
            item->payload_kind = MODEL_PAYLOAD_BALLISTIC;
        else if (kind - 675u < 3u)
            item->payload_kind = MODEL_PAYLOAD_DRIFT;
        else if (kind == 70 || kind == 71 || kind - 74u < 3u || kind == 124 || kind - 127u < 2u || kind - 348u < 2u)
            item->payload_kind = MODEL_PAYLOAD_SHAKE;
        else if (kind == 77)
            item->payload_kind = MODEL_PAYLOAD_GATE_FALL;
        else if (kind - 72u < 2u)
            item->payload_kind = MODEL_PAYLOAD_GATE_FRAGMENT;
        else if (kind == 696)
        {
            item->payload_kind = MODEL_PAYLOAD_ROTATION;
            item->rotation.linked = (WORLD_OBJECT_REFERENCE){0, 1};
        }
        else if (kind - 473u < 3u)
            item->payload_kind = MODEL_PAYLOAD_BOUNCING_DROP;
        else if (kind == 67 || kind - 85u < 4u || kind == 92 || kind - 99u < 2u)
            item->payload_kind = MODEL_PAYLOAD_BOUNCING_PART;
        else if (kind - 78u < 7u)
            item->payload_kind = MODEL_PAYLOAD_TRAFFIC;
        else if (kind == 776)
            item->payload_kind = MODEL_PAYLOAD_TANK;
        else if (kind == 17 || kind - 62u < 3u || kind == 66 || kind == 90 || kind - 94u < 3u || kind - 108u < 3u || kind - 720u < 3u || kind == 739 || kind == 731 || kind == 204 || kind == 693 || kind == 405 || kind == 137)
            item->payload_kind = MODEL_PAYLOAD_OSCILLATION;
        else if (kind == 65)
            item->payload_kind = MODEL_PAYLOAD_COMPANION;
        else if (kind == 433)
            item->payload_kind = MODEL_PAYLOAD_LAB_TANK;
        else if (kind - 295u < 7u || kind == 326 || kind == 367 || kind - 371u < 12u || kind - 409u < 24u || kind - 608u < 6u || kind - 642u < 5u || kind == 649 || kind - 655u < 2u || kind - 672u < 3u || kind - 718u < 2u || kind == 737 || kind == 748 || kind - 786u < 12u || kind - 804u < 8u)
            item->payload_kind = MODEL_PAYLOAD_WOBBLE;
        else if (kind - 587u < 10u || kind == 69 || kind == 105 || kind == 724 || kind == 448 || kind == 717 || kind - 269u < 2u || kind == 392 || kind == 359 || kind == 695 || kind - 708u < 3u || kind == 408 || kind - 572u < 3u || kind - DYNAMIC_ITEM_TYPE_FIRST < DYNAMIC_ITEM_TYPE_COUNT || kind == 713 || kind == 715 || kind - 735u < 2u || kind - 784u < 2u || kind - 828u < 4u)
            item->payload_kind = MODEL_PAYLOAD_PHYSICS;
        else if (kind - 781u < 2u)
            item->payload_kind = MODEL_PAYLOAD_ARMY_TRUCK;
        else if (kind - 460u < 4u)
            item->payload_kind = MODEL_PAYLOAD_AIR_BIKE;
        else if (kind == 160 || kind == 166 || kind == 168 || kind == 740 || kind == 749 || kind - 175u < 8u || kind == 754)
            item->payload_kind = MODEL_PAYLOAD_SCENE_VEHICLE;
        else if (kind == 341 || kind == 146)
            item->payload_kind = MODEL_PAYLOAD_SCENE_HELICOPTER;
        else if (kind == 268)
            item->payload_kind = MODEL_PAYLOAD_PATROL_HELICOPTER;
        else if (kind == 687)
            item->payload_kind = MODEL_PAYLOAD_COMBAT_HELICOPTER;
        item->pos.x.bits = x;
        item->pos.y.bits = y;
        item->pos.z.bits = z;
        item->angles.yaw_bits = camera.angles.yaw_bits;
        if (kind == 869)
            item->spr.frame = (sint16)(random_next() & 3u);
        if (kind - 587u < 10u)
            item->physics.life = 300;
        return idx;
    }
    return -1;
}

sint32 dynamic_objects_spawn_randomized(sint32 kind, uint32 x, uint32 y, uint32 z, uint32 angle)
{
    FUNCTION_MARKER(0x80016748u, "GAME.EXE");
    sint32 slot = dynamic_objects_spawn_item(kind, x, y, z, angle);
    uint32 val = random_next();
    /* TODO: Investigate the original pre-pool writes after item allocation failure */
    XASSERT(slot >= 0 && slot < DYNAMIC_OBJECT_COUNT);
    MODEL_OBJECT *item = dynamic_objects + slot;
    /* TODO: Investigate a randomized item with a foreign native owner */
    XASSERT(item->payload_kind == MODEL_PAYLOAD_PHYSICS);
    item->physics.speed = (uint16)((val & 15) + 50);
    val = random_next();
    item->physics.vertical_speed = (uint16)((val & 15) + 60);
    item->physics.obstacle_flags = 1;
    return slot;
}

sint32 dynamic_objects_spawn_item(sint32 type, uint32 x, uint32 y, uint32 z, uint32 angle)
{
    FUNCTION_MARKER(0x800167c4, "GAME.EXE");
    sint32 i, uses;
    for (i = 0; i < DYNAMIC_OBJECT_COUNT; i++)
    {
        if (dynamic_objects[i].kind != 0xffffffffu)
            continue;
        MODEL_OBJECT *item = dynamic_objects + i;
        *item = (MODEL_OBJECT){0};
        item->kind = type;
        if ((uint32)type - DYNAMIC_ITEM_TYPE_FIRST < DYNAMIC_ITEM_TYPE_COUNT || type - 587u < 10u)
            item->payload_kind = MODEL_PAYLOAD_PHYSICS;
        item->pos.x.bits = x;
        item->pos.y.bits = y;
        item->pos.z.bits = z;
        item->angles.yaw_bits = (uint16)angle;
        if (type - 587u < 10)
        {
            /* Original table read is overwritten before any call or return */
            uses = 300;
        }
        else if ((uint32)type - DYNAMIC_ITEM_TYPE_FIRST < DYNAMIC_ITEM_TYPE_COUNT)
            uses = dynamic_item_uses[type - DYNAMIC_ITEM_TYPE_FIRST];
        else
        {
            /* TODO: Investigate item kinds that read beyond the uses table */

            XASSERT(0);
        }
        item->physics.life = (uint16)uses;
        return i;
    }
    return -1;
}

sint32 dynamic_objects_spawn_oriented(uint32 kind, uint32 x, uint32 y, uint32 z, uint32 angle)
{
    FUNCTION_MARKER(0x800168bcu, "GAME.EXE");
    sint32 i;
    for (i = 0; i < DYNAMIC_OBJECT_COUNT; i++)
        if (dynamic_objects[i].kind == 0xffffffffu)
        {
            MODEL_OBJECT *item = dynamic_objects + i;
            *item = (MODEL_OBJECT){0};
            item->kind = kind;
            if (kind == 302)
                item->payload_kind = MODEL_PAYLOAD_TRAIN;
            else if (kind == 896)
                item->payload_kind = MODEL_PAYLOAD_SUSPENDED;
            else if (kind - 114u < 4u)
                item->payload_kind = MODEL_PAYLOAD_WAVE;
            else if (kind == 125 || kind == 126)
                item->payload_kind = MODEL_PAYLOAD_PANEL;
            else if (kind - 129u < 3u || kind - 350u < 9u || kind - 758u < 12u)
                item->payload_kind = MODEL_PAYLOAD_SIDE_PANEL;
            else if (kind - 575u < 2u || kind - 862u < 5u || kind == 869 || kind - 881u < 5u)
                item->payload_kind = MODEL_PAYLOAD_SPRITE;
            else if (kind - 679u < 4u)
                item->payload_kind = MODEL_PAYLOAD_BALLISTIC;
            else if (kind - 675u < 3u)
                item->payload_kind = MODEL_PAYLOAD_DRIFT;
            else if (kind == 70 || kind == 71 || kind - 74u < 3u || kind == 124 || kind - 127u < 2u || kind - 348u < 2u)
                item->payload_kind = MODEL_PAYLOAD_SHAKE;
            else if (kind == 77)
                item->payload_kind = MODEL_PAYLOAD_GATE_FALL;
            else if (kind - 72u < 2u)
                item->payload_kind = MODEL_PAYLOAD_GATE_FRAGMENT;
            else if (kind == 696)
            {
                item->payload_kind = MODEL_PAYLOAD_ROTATION;
                item->rotation.linked = (WORLD_OBJECT_REFERENCE){0, 1};
            }
            else if (kind - 473u < 3u)
                item->payload_kind = MODEL_PAYLOAD_BOUNCING_DROP;
            else if (kind == 67 || kind - 85u < 4u || kind == 92 || kind - 99u < 2u)
                item->payload_kind = MODEL_PAYLOAD_BOUNCING_PART;
            else if (kind - 78u < 7u)
                item->payload_kind = MODEL_PAYLOAD_TRAFFIC;
            else if (kind == 776)
                item->payload_kind = MODEL_PAYLOAD_TANK;
            else if (kind == 17 || kind - 62u < 3u || kind == 66 || kind == 90 || kind - 94u < 3u || kind - 108u < 3u || kind - 720u < 3u || kind == 739 || kind == 731 || kind == 204 || kind == 693 || kind == 405 || kind == 137)
                item->payload_kind = MODEL_PAYLOAD_OSCILLATION;
            else if (kind == 65)
                item->payload_kind = MODEL_PAYLOAD_COMPANION;
            else if (kind == 433)
                item->payload_kind = MODEL_PAYLOAD_LAB_TANK;
            else if (kind - 295u < 7u || kind == 326 || kind == 367 || kind - 371u < 12u || kind - 409u < 24u || kind - 608u < 6u || kind - 642u < 5u || kind == 649 || kind - 655u < 2u || kind - 672u < 3u || kind - 718u < 2u || kind == 737 || kind == 748 || kind - 786u < 12u || kind - 804u < 8u)
                item->payload_kind = MODEL_PAYLOAD_WOBBLE;
            else if (kind - 587u < 10u || kind == 69 || kind == 105 || kind == 724 || kind == 448 || kind == 717 || kind - 269u < 2u || kind == 392 || kind == 359 || kind == 695 || kind - 708u < 3u || kind == 408 || kind - 572u < 3u || kind - DYNAMIC_ITEM_TYPE_FIRST < DYNAMIC_ITEM_TYPE_COUNT || kind == 713 || kind == 715 || kind - 735u < 2u || kind - 784u < 2u || kind - 828u < 4u)
                item->payload_kind = MODEL_PAYLOAD_PHYSICS;
            else if (kind - 781u < 2u)
                item->payload_kind = MODEL_PAYLOAD_ARMY_TRUCK;
            else if (kind - 460u < 4u)
                item->payload_kind = MODEL_PAYLOAD_AIR_BIKE;
            else if (kind == 160 || kind == 166 || kind == 168 || kind == 740 || kind == 749 || kind - 175u < 8u || kind == 754)
                item->payload_kind = MODEL_PAYLOAD_SCENE_VEHICLE;
            else if (kind == 341 || kind == 146)
                item->payload_kind = MODEL_PAYLOAD_SCENE_HELICOPTER;
            else if (kind == 268)
                item->payload_kind = MODEL_PAYLOAD_PATROL_HELICOPTER;
            else if (kind == 687)
                item->payload_kind = MODEL_PAYLOAD_COMBAT_HELICOPTER;
            item->pos.x.bits = x;
            item->pos.y.bits = y;
            item->pos.z.bits = z;
            item->angles.yaw_bits = (uint16)angle;
            return i;
        }
    return -1;
}

uint32 dynamic_objects_spawn_transient(uint32 type, uint32 x, uint32 y, uint32 z)
{
    FUNCTION_MARKER(0x8001697C, "GAME.EXE");
    uint32 i, angle;
    for (i = 0; i < DYNAMIC_OBJECT_COUNT; i++)
        if (dynamic_objects[i].kind == 0xffffffffu)
        {
            MODEL_OBJECT *item = dynamic_objects + i;
            *item = (MODEL_OBJECT){0};
            angle = (uint16)camera.angles.yaw;
            item->kind = type;
            if (type == 302)
                item->payload_kind = MODEL_PAYLOAD_TRAIN;
            else if (type == 896)
                item->payload_kind = MODEL_PAYLOAD_SUSPENDED;
            else if (type - 114u < 4u)
                item->payload_kind = MODEL_PAYLOAD_WAVE;
            else if (type == 125 || type == 126)
                item->payload_kind = MODEL_PAYLOAD_PANEL;
            else if (type - 129u < 3u || type - 350u < 9u || type - 758u < 12u)
                item->payload_kind = MODEL_PAYLOAD_SIDE_PANEL;
            else if (type - 575u < 2u || type - 862u < 5u || type == 869 || type - 881u < 5u)
                item->payload_kind = MODEL_PAYLOAD_SPRITE;
            else if (type - 679u < 4u)
                item->payload_kind = MODEL_PAYLOAD_BALLISTIC;
            else if (type - 675u < 3u)
                item->payload_kind = MODEL_PAYLOAD_DRIFT;
            else if (type == 70 || type == 71 || type - 74u < 3u || type == 124 || type - 127u < 2u || type - 348u < 2u)
                item->payload_kind = MODEL_PAYLOAD_SHAKE;
            else if (type == 77)
                item->payload_kind = MODEL_PAYLOAD_GATE_FALL;
            else if (type - 72u < 2u)
                item->payload_kind = MODEL_PAYLOAD_GATE_FRAGMENT;
            else if (type == 696)
            {
                item->payload_kind = MODEL_PAYLOAD_ROTATION;
                item->rotation.linked = (WORLD_OBJECT_REFERENCE){0, 1};
            }
            else if (type - 473u < 3u)
                item->payload_kind = MODEL_PAYLOAD_BOUNCING_DROP;
            else if (type == 67 || type - 85u < 4u || type == 92 || type - 99u < 2u)
                item->payload_kind = MODEL_PAYLOAD_BOUNCING_PART;
            else if (type - 78u < 7u)
                item->payload_kind = MODEL_PAYLOAD_TRAFFIC;
            else if (type == 776)
                item->payload_kind = MODEL_PAYLOAD_TANK;
            else if (type == 17 || type - 62u < 3u || type == 66 || type == 90 || type - 94u < 3u || type - 108u < 3u || type - 720u < 3u || type == 739 || type == 731 || type == 204 || type == 693 || type == 405 || type == 137)
                item->payload_kind = MODEL_PAYLOAD_OSCILLATION;
            else if (type == 65)
                item->payload_kind = MODEL_PAYLOAD_COMPANION;
            else if (type == 433)
                item->payload_kind = MODEL_PAYLOAD_LAB_TANK;
            else if (type - 295u < 7u || type == 326 || type == 367 || type - 371u < 12u || type - 409u < 24u || type - 608u < 6u || type - 642u < 5u || type == 649 || type - 655u < 2u || type - 672u < 3u || type - 718u < 2u || type == 737 || type == 748 || type - 786u < 12u || type - 804u < 8u)
                item->payload_kind = MODEL_PAYLOAD_WOBBLE;
            else if (type - 587u < 10u || type == 69 || type == 105 || type == 724 || type == 448 || type == 717 || type - 269u < 2u || type == 392 || type == 359 || type == 695 || type - 708u < 3u || type == 408 || type - 572u < 3u || type - DYNAMIC_ITEM_TYPE_FIRST < DYNAMIC_ITEM_TYPE_COUNT || type == 713 || type == 715 || type - 735u < 2u || type - 784u < 2u || type - 828u < 4u)
                item->payload_kind = MODEL_PAYLOAD_PHYSICS;
            else if (type - 781u < 2u)
                item->payload_kind = MODEL_PAYLOAD_ARMY_TRUCK;
            else if (type - 460u < 4u)
                item->payload_kind = MODEL_PAYLOAD_AIR_BIKE;
            else if (type == 160 || type == 166 || type == 168 || type == 740 || type == 749 || type - 175u < 8u || type == 754)
                item->payload_kind = MODEL_PAYLOAD_SCENE_VEHICLE;
            else if (type == 341 || type == 146)
                item->payload_kind = MODEL_PAYLOAD_SCENE_HELICOPTER;
            else if (type == 268)
                item->payload_kind = MODEL_PAYLOAD_PATROL_HELICOPTER;
            else if (type == 687)
                item->payload_kind = MODEL_PAYLOAD_COMBAT_HELICOPTER;
            item->pos.x.bits = x;
            item->pos.y.bits = y;
            item->pos.z.bits = z;
            item->angles.yaw_bits = (uint16)angle;
            return angle;
        }
    return 0;
}

sint32 dynamic_objects_update(void)
{
    FUNCTION_MARKER(0x8001ED1C, "GAME.EXE");
    uint32 i, object, type, prev_car = 0, retained_angle = 0x800bbf78;
    const OBJECT_CALLBACK *target;
    for (i = 0, object = 0x8009a3c8; i < DYNAMIC_OBJECT_COUNT; i++, object += 36)
    {
        type = dynamic_objects[i].kind;
        if (type == 0xffffffff)
            continue;
        XASSERT(type < MODEL_TYPE_COUNT);
        target = object_callbacks[OBJECT_CALLBACK_UPDATE][type];
        if (target == &callback_helicopter_update)
        {
            helicopter_update_retained(dynamic_objects + i, prev_car, retained_angle);
            prev_car = 0;
        }
        else if (target == &callback_scene_vehicle_update_traffic)
        {
            object_call(target, dynamic_objects + i, NULL);
            prev_car = object;
            retained_angle = 0x800bbf78;
        }
        else if (!target || target == &callback_object_update_physics || target == &callback_object_update_physics_alias)
        {
            /* Physics preserves height and saves incoming S0 as angle */
            object_call(target ? target : &callback_object_update_physics, dynamic_objects + i, NULL);
            retained_angle = object;
        }
        else
        {
            object_call(target ? target : &callback_object_update_physics, dynamic_objects + i, NULL);
            prev_car = 0;
        }
    }
    return 0;
}

/* Select contact damage without narrowing the original model-kind guards */
uint32 object_apply_damage(sint32 attacker, sint32 victim, uint32 kind)
{
    FUNCTION_MARKER(0x8001f468, "GAME.EXE");
    sint32 damage;
    if (kind == 572 || kind == 574 || kind == 604)
        damage = 64;
    else if (kind == 695 || kind - 708u < 2u || kind == 710 || kind == 408)
        damage = 32;
    else
    {
        uint32 item_idx = kind - DYNAMIC_ITEM_TYPE_FIRST;
        if (item_idx >= DYNAMIC_ITEM_TYPE_COUNT + 1u || (uint32)attacker >= 9u || (uint32)victim >= 9u)
            return 0;
        if (item_idx < DYNAMIC_ITEM_TYPE_COUNT)
            damage = item_properties[item_idx].damage;
        else
            /* Type563 overlaps the first AI choice count */
            damage = (sint16)(npc_ai_capabilities[0].choice_count >> 16);
    }
    if (damage < 0)
        damage = 0;
    return actor_apply_damage(attacker, victim, damage);
}

uint32 object_update_expiring(MODEL_OBJECT *model)
{
    FUNCTION_MARKER(0x8002b2b4u, "GAME.EXE");
    /* TODO: Investigate an expiring model without its common native timer */
    XASSERT(model && model->payload_kind == MODEL_PAYLOAD_UNMIGRATED);
    uint16 age = (uint16)model->pending_motion.retire_ticks + 1u;
    model->pending_motion.retire_ticks = age;
    if ((sint16)age >= 17)
        model->kind = 0xffffffffu;
    return 0xffffffffu;
}

void dynamic_objects_settle_debris(MODEL_OBJECT *item)
{
    FUNCTION_MARKER(0x8002ddb8u, "GAME.EXE");
    uint32 speed;
    uint32 angle;
    sint32 result;
    uint32 kind;
    FIXED_VEC3 point;
    sint32 bounce, z;
    /* TODO: Investigate common object fields outside the world/dynamic pools */
    XASSERT(item);
    /* TODO: Investigate settling debris without its constructed native owner */
    XASSERT(item->payload_kind == MODEL_PAYLOAD_PHYSICS);
    speed = (uint16)item->physics.pitch_speed + 256u;
    item->physics.pitch_speed = (uint16)speed;
    angle = (uint16)item->angles.pitch - speed;
    item->angles.pitch_bits = (uint16)angle;
    result = angle << 16;
    if (result >= 0)
        return;
    item->angles.pitch_bits = 0;
    bounce = -(item->physics.pitch_speed >> 1);
    item->physics.pitch_speed = (uint16)bounce;
    if ((uint16)(bounce + 255) >= 511)
        return; /* Original delay-slot result 724 is unused by game callers */
    kind = item->kind;
    kind = kind == 724 ? 530 : kind == 69 ? 529 : 528;
    item->kind = kind;
    kind = item->kind;
    XASSERT(kind - DYNAMIC_ITEM_TYPE_FIRST < DYNAMIC_ITEM_TYPE_COUNT);
    item->physics.life = dynamic_item_uses[kind - DYNAMIC_ITEM_TYPE_FIRST];
    transform_push();
    transform_identity();
    transform_pitch(32768);
    transform_yaw(item->angles.yaw);
    kind = item->kind;
    z = kind == 529 ? -3932160 : kind == 530 ? -3670016 : -3145728;
    transform_point(0, 0, z, point.bits);
    item->pos.x.bits = item->pos.x.bits + point.x.bits;
    item->pos.z.bits = item->pos.z.bits + point.z.bits;
    transform_pop();
}

sint32 dynamic_objects_draw(void)
{
    FUNCTION_MARKER(0x8001EE44, "GAME.EXE");
    uint8 attached[DYNAMIC_OBJECT_COUNT] = {0};
    uint32 i, j, type;
    const OBJECT_CALLBACK *target;
    sint32 idx;
    for (i = 0; i < ACTOR_COUNT; i++)
        for (j = 0; j < ACTOR_ITEM_HAND_COUNT; j++)
        {
            idx = actor_items[i].slot[j];
            if (idx != -1)
            {
                XASSERT((uint32)idx < DYNAMIC_OBJECT_COUNT);
                attached[idx] = 1;
            }
        }
    for (i = 0; i < DYNAMIC_OBJECT_COUNT; i++)
    {
        type = dynamic_objects[i].kind;
        if (type == 0xffffffff || attached[i])
            continue;
        XASSERT(type < MODEL_TYPE_COUNT);
        target = object_callbacks[OBJECT_CALLBACK_DYNAMIC_DRAW][type];
        object_call(target ? target : &callback_default_model_draw, dynamic_objects + i, NULL);
    }
    return 0;
}

/* Borrow physics roles from the current payload without copying its state */
typedef struct OBJECT_MOTION_VIEW
{
    uint16 *life;
    uint16 *speed;
    uint16 *spin;
    uint16 *actor_idx;
    uint16 *bounce_count;
    uint16 *vertical_speed;
    uint16 *obstacle_flags;
} OBJECT_MOTION_VIEW;

static void object_motion_bind(OBJECT_MOTION_VIEW *motion, MODEL_OBJECT *item)
{
    switch (item->payload_kind)
    {
        case MODEL_PAYLOAD_UNMIGRATED:
            motion->life = (uint16 *)&item->pending_motion.roll_speed;
            motion->speed = (uint16 *)&item->pending_motion.speed;
            motion->spin = (uint16 *)&item->pending_motion.pitch_speed;
            motion->actor_idx = (uint16 *)&item->pending_motion.phase;
            motion->bounce_count = (uint16 *)&item->pending_motion.voice_plus_one;
            motion->vertical_speed = (uint16 *)&item->pending_motion.sound_clock;
            motion->obstacle_flags = (uint16 *)&item->pending_motion.physics_obstacle_flags;
            break;
        case MODEL_PAYLOAD_PANEL:
            motion->life = (uint16 *)&item->panel.retire_ticks;
            motion->speed = (uint16 *)&item->panel.speed;
            motion->spin = (uint16 *)&item->panel.initialized;
            motion->actor_idx = (uint16 *)&item->panel.pitch_step;
            motion->bounce_count = (uint16 *)&item->panel.roll_step;
            motion->vertical_speed = (uint16 *)&item->panel.vertical_speed;
            motion->obstacle_flags = (uint16 *)&item->panel.physics_obstacle_flags;
            break;
        case MODEL_PAYLOAD_SIDE_PANEL:
            motion->life = (uint16 *)&item->panel.roll_step;
            motion->speed = (uint16 *)&item->panel.speed;
            motion->spin = (uint16 *)&item->panel.initialized;
            motion->actor_idx = (uint16 *)&item->panel.pitch_step;
            motion->bounce_count = (uint16 *)&item->panel.physics_bounce_count;
            motion->vertical_speed = (uint16 *)&item->panel.vertical_speed;
            motion->obstacle_flags = (uint16 *)&item->panel.physics_obstacle_flags;
            break;
        case MODEL_PAYLOAD_SPRITE:
            motion->life = (uint16 *)&item->spr.frame;
            motion->speed = (uint16 *)&item->spr.speed;
            motion->spin = (uint16 *)&item->spr.rise;
            motion->actor_idx = (uint16 *)&item->spr.physics_actor_idx;
            motion->bounce_count = (uint16 *)&item->spr.physics_bounce_count;
            motion->vertical_speed = (uint16 *)&item->spr.vertical_speed;
            motion->obstacle_flags = (uint16 *)&item->spr.physics_obstacle_flags;
            break;
        case MODEL_PAYLOAD_BALLISTIC:
            motion->life = (uint16 *)&item->ballistic.pitch_step;
            motion->speed = (uint16 *)&item->ballistic.speed;
            motion->spin = (uint16 *)&item->ballistic.roll_step;
            motion->actor_idx = (uint16 *)&item->ballistic.physics_actor_idx;
            motion->bounce_count = (uint16 *)&item->ballistic.physics_bounce_count;
            motion->vertical_speed = (uint16 *)&item->ballistic.vertical_speed;
            motion->obstacle_flags = (uint16 *)&item->ballistic.physics_obstacle_flags;
            break;
        case MODEL_PAYLOAD_DRIFT:
            motion->life = (uint16 *)&item->drift.flags;
            motion->speed = (uint16 *)&item->drift.speed;
            motion->spin = (uint16 *)&item->drift.phase;
            motion->actor_idx = (uint16 *)&item->drift.physics_actor_idx;
            motion->bounce_count = (uint16 *)&item->drift.physics_bounce_count;
            motion->vertical_speed = (uint16 *)&item->drift.physics_vertical_speed;
            motion->obstacle_flags = (uint16 *)&item->drift.physics_obstacle_flags;
            break;
        case MODEL_PAYLOAD_SHAKE:
            motion->life = (uint16 *)&item->shake.ticks;
            motion->speed = (uint16 *)&item->shake.speed;
            motion->spin = (uint16 *)&item->shake.oz;
            motion->actor_idx = (uint16 *)&item->shake.ox;
            motion->bounce_count = (uint16 *)&item->shake.physics_bounce_count;
            motion->vertical_speed = (uint16 *)&item->shake.physics_vertical_speed;
            motion->obstacle_flags = (uint16 *)&item->shake.physics_obstacle_flags;
            break;
        case MODEL_PAYLOAD_GATE_FALL:
            motion->life = (uint16 *)&item->gate_fall.pitch_speed;
            motion->speed = (uint16 *)&item->gate_fall.speed;
            motion->spin = (uint16 *)&item->gate_fall.bounces;
            motion->actor_idx = (uint16 *)&item->gate_fall.ox;
            motion->bounce_count = (uint16 *)&item->gate_fall.physics_bounce_count;
            motion->vertical_speed = (uint16 *)&item->gate_fall.physics_vertical_speed;
            motion->obstacle_flags = (uint16 *)&item->gate_fall.physics_obstacle_flags;
            break;
        case MODEL_PAYLOAD_GATE_FRAGMENT:
            motion->life = (uint16 *)&item->gate_fragment.pitch_speed;
            motion->speed = (uint16 *)&item->gate_fragment.speed;
            motion->spin = (uint16 *)&item->gate_fragment.physics_spin;
            motion->actor_idx = (uint16 *)&item->gate_fragment.physics_actor_idx;
            motion->bounce_count = (uint16 *)&item->gate_fragment.physics_bounce_count;
            motion->vertical_speed = (uint16 *)&item->gate_fragment.physics_vertical_speed;
            motion->obstacle_flags = (uint16 *)&item->gate_fragment.physics_obstacle_flags;
            break;
        case MODEL_PAYLOAD_ROTATION:
            motion->life = (uint16 *)&item->rotation.phase;
            motion->speed = (uint16 *)&item->rotation.amplitude;
            motion->spin = (uint16 *)&item->rotation.axis;
            motion->actor_idx = (uint16 *)&item->rotation.phase_step;
            motion->bounce_count = NULL;
            motion->vertical_speed = (uint16 *)&item->rotation.physics_vertical_speed;
            motion->obstacle_flags = (uint16 *)&item->rotation.physics_obstacle_flags;
            break;
        case MODEL_PAYLOAD_BOUNCING_DROP:
            motion->life = (uint16 *)&item->drop.flags;
            motion->speed = (uint16 *)&item->drop.speed;
            motion->spin = (uint16 *)&item->drop.released;
            motion->actor_idx = (uint16 *)&item->drop.actor_idx;
            motion->bounce_count = (uint16 *)&item->drop.bounces;
            motion->vertical_speed = (uint16 *)&item->drop.vertical_speed;
            motion->obstacle_flags = (uint16 *)&item->drop.obstacle_flags;
            break;
        case MODEL_PAYLOAD_BOUNCING_PART:
            motion->life = (uint16 *)&item->bounce.bounces;
            motion->speed = (uint16 *)&item->bounce.speed;
            motion->spin = (uint16 *)&item->bounce.physics_spin;
            motion->actor_idx = (uint16 *)&item->bounce.physics_actor_idx;
            motion->bounce_count = (uint16 *)&item->bounce.physics_bounce_count;
            motion->vertical_speed = (uint16 *)&item->bounce.vertical_speed;
            motion->obstacle_flags = (uint16 *)&item->bounce.physics_obstacle_flags;
            break;
        case MODEL_PAYLOAD_TRAFFIC:
            motion->life = (uint16 *)&item->traffic.flags;
            motion->speed = (uint16 *)&item->traffic.speed;
            motion->spin = (uint16 *)&item->traffic.physics_spin;
            motion->actor_idx = (uint16 *)&item->traffic.physics_actor_idx;
            motion->bounce_count = (uint16 *)&item->traffic.physics_bounce_count;
            motion->vertical_speed = (uint16 *)&item->traffic.physics_vertical_speed;
            motion->obstacle_flags = (uint16 *)&item->traffic.physics_obstacle_flags;
            break;
        case MODEL_PAYLOAD_TANK:
            motion->life = (uint16 *)&item->tank.flags;
            motion->speed = (uint16 *)&item->tank.speed;
            motion->spin = (uint16 *)&item->tank.turret_yaw;
            motion->actor_idx = (uint16 *)&item->tank.physics_actor_idx;
            motion->bounce_count = (uint16 *)&item->tank.physics_bounce_count;
            motion->vertical_speed = (uint16 *)&item->tank.physics_vertical_speed;
            motion->obstacle_flags = (uint16 *)&item->tank.physics_obstacle_flags;
            break;
        case MODEL_PAYLOAD_OSCILLATION:
            motion->life = (uint16 *)&item->oscillation.roll_speed;
            motion->speed = (uint16 *)&item->oscillation.speed;
            motion->spin = (uint16 *)&item->oscillation.pitch_speed;
            motion->actor_idx = (uint16 *)&item->oscillation.phase;
            motion->bounce_count = (uint16 *)&item->oscillation.voice_plus_one;
            motion->vertical_speed = (uint16 *)&item->oscillation.sound_clock;
            motion->obstacle_flags = (uint16 *)&item->oscillation.physics_obstacle_flags;
            break;
        case MODEL_PAYLOAD_COMPANION:
            motion->life = (uint16 *)&item->companion.flags;
            motion->speed = (uint16 *)&item->companion.speed;
            motion->spin = (uint16 *)&item->companion.physics_spin;
            motion->actor_idx = (uint16 *)&item->companion.physics_actor_idx;
            motion->bounce_count = (uint16 *)&item->companion.voice;
            motion->vertical_speed = (uint16 *)&item->companion.physics_vertical_speed;
            motion->obstacle_flags = (uint16 *)&item->companion.physics_obstacle_flags;
            break;
        case MODEL_PAYLOAD_LAB_TANK:
            motion->life = (uint16 *)&item->lab_tank.flags;
            motion->speed = (uint16 *)&item->lab_tank.speed;
            motion->spin = (uint16 *)&item->lab_tank.pitch_speed;
            motion->actor_idx = (uint16 *)&item->lab_tank.voice_plus_one[1];
            motion->bounce_count = (uint16 *)&item->lab_tank.voice_plus_one[0];
            motion->vertical_speed = (uint16 *)&item->lab_tank.vertical_speed;
            motion->obstacle_flags = (uint16 *)&item->lab_tank.physics_obstacle_flags;
            break;
        case MODEL_PAYLOAD_WOBBLE:
            motion->life = (uint16 *)&item->wobble.ticks;
            motion->speed = (uint16 *)&item->wobble.speed;
            motion->spin = (uint16 *)&item->wobble.emit_ticks;
            motion->actor_idx = (uint16 *)&item->wobble.physics_actor_idx;
            motion->bounce_count = (uint16 *)&item->wobble.voice_plus_one;
            motion->vertical_speed = (uint16 *)&item->wobble.physics_vertical_speed;
            motion->obstacle_flags = (uint16 *)&item->wobble.physics_obstacle_flags;
            break;
        case MODEL_PAYLOAD_PHYSICS:
            motion->life = (uint16 *)&item->physics.life;
            motion->speed = (uint16 *)&item->physics.speed;
            motion->spin = (uint16 *)&item->physics.spin;
            motion->actor_idx = (uint16 *)&item->physics.actor_idx;
            motion->bounce_count = (uint16 *)&item->physics.bounce_count;
            motion->vertical_speed = (uint16 *)&item->physics.vertical_speed;
            motion->obstacle_flags = (uint16 *)&item->physics.obstacle_flags;
            break;
        case MODEL_PAYLOAD_ARMY_TRUCK:
            motion->life = (uint16 *)&item->truck.flags;
            motion->speed = (uint16 *)&item->truck.speed;
            motion->spin = (uint16 *)&item->truck.active;
            motion->actor_idx = (uint16 *)&item->truck.turn_step;
            motion->bounce_count = (uint16 *)&item->truck.bounce_count;
            motion->vertical_speed = (uint16 *)&item->truck.vertical_speed;
            motion->obstacle_flags = (uint16 *)&item->truck.obstacle_flags;
            break;
        case MODEL_PAYLOAD_AIR_BIKE:
            motion->life = (uint16 *)&item->air_bike.target[0].x;
            motion->speed = (uint16 *)&item->air_bike.target[0].z;
            motion->spin = (uint16 *)&item->air_bike.target[1].x;
            motion->actor_idx = (uint16 *)&item->air_bike.target[1].z;
            motion->bounce_count = (uint16 *)&item->air_bike.voice_plus_one;
            motion->vertical_speed = (uint16 *)&item->air_bike.phase;
            motion->obstacle_flags = (uint16 *)&item->air_bike.actor_idx;
            break;
        case MODEL_PAYLOAD_SCENE_VEHICLE:
            motion->life = (uint16 *)&item->scene_vehicle.flags;
            motion->speed = (uint16 *)&item->scene_vehicle.speed;
            motion->spin = (uint16 *)&item->scene_vehicle.start_tick;
            motion->actor_idx = (uint16 *)&item->scene_vehicle.voice_plus_one[0];
            motion->bounce_count = (uint16 *)&item->scene_vehicle.voice_plus_one[1];
            motion->vertical_speed = (uint16 *)&item->scene_vehicle.vertical_speed;
            motion->obstacle_flags = (uint16 *)&item->scene_vehicle.obstacle_flags;
            break;
        case MODEL_PAYLOAD_SCENE_HELICOPTER:
            motion->life = (uint16 *)&item->scene_helicopter.flags;
            motion->speed = (uint16 *)&item->scene_helicopter.speed;
            motion->spin = (uint16 *)&item->scene_helicopter.spin;
            motion->actor_idx = (uint16 *)&item->scene_helicopter.active;
            motion->bounce_count = (uint16 *)&item->scene_helicopter.voice_plus_one;
            motion->vertical_speed = (uint16 *)&item->scene_helicopter.vertical_speed;
            motion->obstacle_flags = (uint16 *)&item->scene_helicopter.obstacle_flags;
            break;
        case MODEL_PAYLOAD_COMBAT_HELICOPTER:
            motion->life = (uint16 *)&item->helicopter.bob_phase;
            motion->speed = (uint16 *)&item->helicopter.shot_distance;
            motion->spin = (uint16 *)&item->helicopter.phase;
            motion->actor_idx = (uint16 *)&item->helicopter.voice_plus_one[0];
            motion->bounce_count = (uint16 *)&item->helicopter.voice_plus_one[1];
            motion->vertical_speed = (uint16 *)&item->helicopter.hover_height;
            motion->obstacle_flags = (uint16 *)&item->helicopter.shot_passes;
            break;
        case MODEL_PAYLOAD_PATROL_HELICOPTER:
            motion->life = (uint16 *)&item->patrol.voice_plus_one;
            motion->speed = (uint16 *)&item->patrol.speed;
            motion->spin = (uint16 *)&item->patrol.phase;
            motion->actor_idx = (uint16 *)&item->patrol.rotor_angle;
            motion->bounce_count = (uint16 *)&item->patrol.mode;
            motion->vertical_speed = (uint16 *)&item->patrol.vertical_speed;
            motion->obstacle_flags = (uint16 *)&item->patrol.obstacle_flags;
            break;
        case MODEL_PAYLOAD_WAVE:
            motion->life = (uint16 *)&item->wave.flags;
            motion->speed = (uint16 *)&item->wave.speed;
            motion->spin = (uint16 *)&item->wave.phase;
            motion->actor_idx = (uint16 *)&item->wave.physics_actor_idx;
            motion->bounce_count = (uint16 *)&item->wave.physics_bounce_count;
            motion->vertical_speed = (uint16 *)&item->wave.physics_vertical_speed;
            motion->obstacle_flags = (uint16 *)&item->wave.physics_obstacle_flags;
            break;
        case MODEL_PAYLOAD_TRAIN:
            motion->life = (uint16 *)&item->train.wait_ticks;
            motion->speed = (uint16 *)&item->train.speed;
            motion->spin = (uint16 *)&item->train.spin;
            motion->actor_idx = (uint16 *)&item->train.state;
            motion->bounce_count = (uint16 *)&item->train.voice_plus_one;
            motion->vertical_speed = (uint16 *)&item->train.vertical_speed;
            motion->obstacle_flags = (uint16 *)&item->train.physics_obstacle_flags;
            break;
        case MODEL_PAYLOAD_SUSPENDED:
            motion->life = (uint16 *)&item->suspended.flags;
            motion->speed = (uint16 *)&item->suspended.actor_idx;
            motion->spin = (uint16 *)&item->suspended.phase;
            motion->actor_idx = (uint16 *)&item->suspended.scene_active;
            motion->bounce_count = (uint16 *)&item->suspended.physics_bounce_count;
            motion->vertical_speed = (uint16 *)&item->suspended.vertical_speed;
            motion->obstacle_flags = (uint16 *)&item->suspended.physics_obstacle_flags;
            break;
        default:
            /* TODO: Investigate an unpublished generic physics payload */

            XASSERT(0);
    }
}

/* Item motion, collision response, bounce sounds and lifetime */
sint32 object_update_physics(MODEL_OBJECT *item)
{
    FUNCTION_MARKER(0x8002b2e4, "GAME.EXE");
    /* TODO: Investigate common object fields outside the world/dynamic pools */
    XASSERT(item);
    /* TODO: Investigate physics calls without the constructed native owner */
    XASSERT(item->kind - 587u >= 10u && item->kind != 69 && item->kind != 105 && item->kind != 724 && item->kind != 448 && item->kind != 717 && item->kind - 269u >= 2u && item->kind != 392 && item->kind != 359 && item->kind != 695 && item->kind - 708u >= 3u && item->kind != 408 && item->kind - 572u >= 3u && item->kind - DYNAMIC_ITEM_TYPE_FIRST >= DYNAMIC_ITEM_TYPE_COUNT && item->kind != 713 && item->kind != 715 && item->kind - 735u >= 2u && item->kind - 784u >= 2u && item->kind - 828u >= 4u || item->payload_kind == MODEL_PAYLOAD_PHYSICS);
    /* TODO: Investigate a truck passed to physics without its native owner */
    XASSERT(item->kind - 781u >= 2u || item->payload_kind == MODEL_PAYLOAD_ARMY_TRUCK);
    OBJECT_MOTION_VIEW motion;
    object_motion_bind(&motion, item);
    uint32 type = item->kind;
    sint32 speed = (sint16)*motion.speed, life;
    if (speed > 0)
    {
        uint32 angle = item->angles.yaw_bits >> 5, x, z, height;
        COLLISION_POINT point;
        uint32 s = sin_table[angle];
        uint32 c = sin_table[MATH_QUARTER_TURN + angle];
        uint32 dx = (sint32)(speed * s) >> 1;
        uint32 dz = (sint32)(speed * c) >> 1;
        uint32 reach_x = s << 7, reach_z = c << 7;
        sint32 val, spin;
        int step;
        x = item->pos.x.bits;
        z = item->pos.z.bits;
        if ((*motion.obstacle_flags & 1) && !regions_find_obstacle(x, z))
        {
            *motion.obstacle_flags = (uint16)(*motion.obstacle_flags - 1);
        }
        for (step = 0; step < 2; step++)
        {
            if (!(*motion.obstacle_flags & 1))
            {
                if ((sint16)*motion.speed >= 13 && type - 586u >= 12u)
                {
                    point.pos.bits[0] = x + reach_x;
                    point.pos.bits[1] = item->pos.y.bits;
                    point.pos.bits[2] = z - reach_z;
                    point.impact_sound = 1;
                    {
                        sint32 owner = (sint16)*motion.actor_idx;
                        uint32 kind = item->kind;
                        sint32 hit = actor_projectile_contact(owner, &point, kind);
                        /* Collision may publish a different native payload */
                        object_motion_bind(&motion, item);
                        if (hit)
                        {
                            if (type == 526)
                                sound_voice_play(30, 255, 6144);
                            if (type == 478 || type == 480 || type == 482)
                            {
                                int particle;
                                item->kind = type + 1;
                                sound_voice_play(113, 255, 6400);
                                for (particle = 0; particle < 12; particle++)
                                {
                                    uint32 particle_angle = item->angles.yaw_bits >> 5;
                                    uint32 particle_x = item->pos.x.bits;
                                    uint32 particle_y = item->pos.y.bits;
                                    uint32 particle_z = item->pos.z.bits;
                                    effect_spawn_glass_fragment(599 + (type - 478) / 2, particle_x, particle_y, particle_z, particle_angle);
                                }
                            }
                            if (type - 499u >= 5u && type != 504 && type != 506 && type != 515 && type - 496u >= 2u && type != 505)
                            {
                                val = (sint16)(*motion.life - 1);
                                *motion.life = (uint16)val;
                                if (val <= 0)
                                {
                                    *motion.life = 65535;
                                }
                            }
                            if (hit == 2)
                            {
                                dx = dz = 0;
                                *motion.speed = 1;
                            }
                            else
                            {
                                *motion.speed = (uint16)((sint16)*motion.speed >> 1);
                            }
                        }
                    }
                }
                if (!(*motion.obstacle_flags & 1))
                {
                    if (regions_find_obstacle(x + reach_x, z - reach_z) & 1)
                    {
                        dx = dz = 0;
                        *motion.speed = 1;
                    }
                    if (!(*motion.obstacle_flags & 1) && (regions_find_obstacle(x + 2u * dx, z - 2u * dz) & 1))
                    {
                        dx = dz = 0;
                        *motion.speed = 1;
                    }
                }
            }
            x += dx;
            z -= dz;
        }
        item->pos.x.bits = x;
        item->pos.z.bits = z;
        val = *motion.vertical_speed - 16;
        *motion.vertical_speed = (uint16)val;
        height = item->pos.y.bits;
        item->pos.y.bits = height - ((sint32)((uint32)(uint16)val << 16) >> 1);
        spin = (sint16)*motion.spin >> 1;
        val = item->angles.pitch_bits;
        item->angles.pitch_bits = (uint16)(val + spin);
        height = item->pos.y.bits;
        if ((sint32)height > 393216)
        {
            /* 2B714..2B8CC: material sound and diminishing volume on each bounce. */
            sint32 bounce = motion.bounce_count ? (sint16)*motion.bounce_count : (sint16)(item->rotation.linked.present ? item->rotation.linked.idx : 0xffffu);
            sint32 volume = 255 >> (bounce & 31u), sample = 29, pitch = 6144;
            int stop_spin = 0;
            if (motion.bounce_count)
                *motion.bounce_count = (uint16)(bounce + 1);
            else
            {
                uint16 linked_word = (uint16)(bounce + 1);
                /* TODO: Investigate physics producing an invalid linked-object index */
                XASSERT(linked_word == 0xffffu || linked_word < WORLD_OBJECT_COUNT);
                item->rotation.linked = linked_word == 0xffffu ? (WORLD_OBJECT_REFERENCE){0} : (WORLD_OBJECT_REFERENCE){linked_word, 1};
            }
            if (type == 526)
            {
                sample = 31;
                stop_spin = 1;
            }
            else if (type == 525 || type == 506 || type == 515 || type == 540 || type == 532 || type == 524)
            {
                sample = 219;
                pitch = 4096;
            }
            else if (type - 543u < 2u || type - 553u < 2u || type - 555u < 2u || type == 535 || type - 522u < 2u)
                sample = 221;
            else if (type == 484 || type == 492 || type == 495)
                sample = 218;
            else if (type - 499u < 5u || type == 505)
            {
                sample = 220;
                pitch = 7168;
            }
            else if (type == 497)
                sample = 52;
            else if (type == 504)
                sample = 220;
            else if (type == 477 || type == 485 || type == 487 || type == 489 || type - 493u < 2u || type == 491 || type == 511 || type == 520)
                sample = 219;
            else if (type - 473u < 3u || type - 478u < 6u)
            {
                sample = 222;
                stop_spin = 1;
            }
            else if ((sint32)type >= 522)
            {
                sample = 31;
                stop_spin = 1;
            }
            if (!stop_spin || (sint16)*motion.spin)
                sound_voice_play(sample, volume, pitch);
            if (stop_spin)
            {
                *motion.spin = 0;
                *motion.vertical_speed = 0;
            }
        }
        if ((sint32)height > 0)
        {
            *motion.obstacle_flags = *motion.obstacle_flags & 65534;
            item->angles.pitch_bits = 0;
            item->pos.y.bits = 0;
            *motion.vertical_speed = (uint16)(-(sint16)*motion.vertical_speed >> 1);
        }
        val = (sint16)(*motion.speed - 3);
        *motion.speed = (uint16)val;
        if (val < 0)
        {
            *motion.speed = 0;
        }
        height = item->pos.y.bits;
        if ((height == 1 || (sint32)height < 0) && !(sint16)*motion.speed)
        {
            *motion.speed = 3;
        }
    }
    else
    {
        if (item->pos.y.bits != 1)
        {
            if (motion.bounce_count)
                *motion.bounce_count = 0;
            else
            {
                uint16 linked_word = 0;
                /* TODO: Investigate physics producing an invalid linked-object index */
                XASSERT(linked_word == 0xffffu || linked_word < WORLD_OBJECT_COUNT);
                item->rotation.linked = linked_word == 0xffffu ? (WORLD_OBJECT_REFERENCE){0} : (WORLD_OBJECT_REFERENCE){linked_word, 1};
            }
            item->angles.pitch_bits = 0;
            item->pos.y.bits = 0;
            if (!(sint16)*motion.life && (type == 506 || type == 515 || type - 499u < 5u || type - 504u < 2u))
            {
                *motion.life = 65535;
            }
        }
    }
    type = item->kind;
    if (type - 587 < 11)
    {
        life = (sint16)*motion.life;
        if (life >= 0)
        {
            *motion.life = (uint16)(life - 1);
            life = (sint16)*motion.life;
        }
    }
    else
        life = (sint16)*motion.life;
    if (life >= 0)
        return life;
    life = (sint16)(*motion.life - 1);
    *motion.life = (uint16)life;
    if (life < -60)
        item->kind = 0xffffffffu;
    return -1;
}

sint32 object_update_bouncing_drop(MODEL_OBJECT *model)
{
    FUNCTION_MARKER(0x80046c08u, "GAME.EXE");
    uint32 offset;
    uint32 x;
    uint32 z;
    sint32 y;
    uint32 kind;
    sint32 speed, result, bounces;
    /* TODO: Investigate a linked-object callback without its native owner */
    XASSERT(model && model->payload_kind == MODEL_PAYLOAD_BOUNCING_DROP);
    BOUNCING_DROP *drop = &model->drop;
    if (drop->speed <= 0)
    {
        if (!drop->released)
            return 473;
        kind = model->kind;
        kind = kind == 473 ? 479 : (kind == 474 || kind == 475 ? 481 : 55);
        if (kind != 55)
        {
            /* TODO: Investigate losing a released drop when the dynamic pool is full */
            sint32 slot = dynamic_objects_spawn_item(kind, model->pos.x.bits, 0, model->pos.z.bits, model->angles.yaw);
            XASSERT(slot >= 0 && slot < DYNAMIC_OBJECT_COUNT);
        }
        model->kind = 55;
        return 55;
    }
    offset = ((uint16)model->angles.yaw >> 5) * 2u;
    if (drop->obstacle_flags & 1u)
    {
        if (!regions_find_obstacle(model->pos.x.val, model->pos.z.val))
            drop->obstacle_flags = drop->obstacle_flags - 1u;
    }
    speed = drop->speed;
    x = model->pos.x.bits + ((uint32)(speed * sin_table[offset >> 1]) << 1);
    z = model->pos.z.bits - ((uint32)(speed * sin_table[MATH_QUARTER_TURN + (offset >> 1)]) << 1);
    if (drop->obstacle_flags || !regions_find_obstacle((sint32)x, (sint32)z))
    {
        model->pos.x.bits = x;
        model->pos.z.bits = z;
    }
    model->pos.y.bits += (uint32)(sint32)drop->vertical_speed << 16;
    drop->vertical_speed = (uint16)drop->vertical_speed + 1u;
    bounces = drop->bounces;
    y = model->pos.y.bits;
    if (y <= 0)
        return y;
    if (!bounces)
        sound_voice_play(222, 255 >> (bounces & 31), 6144);
    drop->bounces = (uint16)drop->bounces + 1u;
    model->pos.y.bits = 0;
    drop->vertical_speed = (uint32)(-(sint64)(drop->vertical_speed >> 1));
    result = drop->speed >> 2;
    drop->speed = result;
    return result;
}

void object_update_rotation_shake(MODEL_OBJECT *model)
{
    FUNCTION_MARKER(0x80046e1cu, "GAME.EXE");
    sint32 amplitude;
    sint32 val;
    sint32 phase;
    uint32 s;
    /* TODO: Investigate a linked-object callback without its native owner */
    XASSERT(model && model->payload_kind == MODEL_PAYLOAD_ROTATION);
    ROTATION_SHAKE *rotation = &model->rotation;
    amplitude = rotation->amplitude;
    if (amplitude <= 0)
        return;
    if (rotation->axis)
    {
        phase = rotation->phase;
        /* TODO: Investigate a rotation phase outside the original sine table */
        XASSERT(phase >= 0 && phase < MATH_SIN_COUNT);
        s = sin_table[phase];
        val = (sint32)(s * amplitude) >> 12;
        model->angles.pitch_bits = (uint16)val;
        model->angles.roll_bits = 0;
    }
    else
    {
        model->angles.pitch_bits = 0;
        phase = rotation->phase;
        /* TODO: Investigate a rotation phase outside the original sine table */
        XASSERT(phase >= 0 && phase < MATH_SIN_COUNT);
        s = sin_table[phase];
        val = (sint32)(s * (rotation->amplitude)) >> 12;
        model->angles.roll_bits = (uint16)val;
    }
    rotation->phase = (((uint16)rotation->phase) + (rotation->phase_step >> 7)) & 2047u;
    rotation->phase_step = rotation->phase_step + 511;
    rotation->amplitude = rotation->amplitude - 2;
}

void object_apply_rotations(const MODEL_OBJECT *model)
{
    FUNCTION_MARKER(0x80047108u, "GAME.EXE");
    /* TODO: Investigate rotation common fields outside both native object pools */
    XASSERT(model);
    sint32 angle = model->angles.yaw;
    if (angle)
        transform_yaw(angle);
    angle = model->angles.roll;
    if (angle)
        transform_roll(angle);
    angle = model->angles.pitch;
    if (angle)
        transform_pitch(angle);
}

/* Orbiting scene object and its two spatial sound channels */
sint32 object_update_orbit_sound(MODEL_OBJECT *model)
{
    FUNCTION_MARKER(0x80043520, "GAME.EXE");
    uint32 angle, inc, val, spatial_volume, pan;
    sint32 volume, voice;
    /* TODO: Investigate rotation common fields outside both native object pools */
    XASSERT(model);
    angle = model->angles.pitch_bits;
    inc = (uint32)(sint32)sin_table[angle >> 5] << 8;
    /* TODO: Investigate an orbiting sound object without a native position */
    XASSERT(model);
    val = model->pos.y.bits;
    model->pos.y.bits = val + inc;
    angle = model->angles.pitch_bits;
    inc = (uint32)(sint32)sin_table[(((sint16)angle >> 5) + 512) & 2047] << 8;
    val = model->pos.z.bits;
    model->pos.z.bits = val - inc;
    angle = model->angles.pitch_bits;
    inc = (uint32)(sint32)sin_table[angle >> 5] << 6;
    val = model->pos.x.bits;
    model->pos.x.bits = val + inc;
    angle = model->angles.pitch_bits;
    model->angles.pitch_bits = (uint16)(angle - 128);
    model->angles.roll_bits = 2048;
    sound_spatial(&model->pos, &spatial_volume, &pan);
    voice = menu.ambient_voice[0];
    sound_voice_pan(voice, spatial_volume, pan);
    sound_spatial(&model->pos, &spatial_volume, &pan);
    volume = (spatial_volume - 128) << 1;
    if (volume < 0)
        volume = 0;
    voice = menu.ambient_voice[1];
    return sound_voice_pan(voice, volume, pan);
}

sint32 object_update_ballistic_spin(MODEL_OBJECT *model)
{
    FUNCTION_MARKER(0x8002c2e4u, "GAME.EXE");
    sint32 velocity, speed, trig, angle;
    uint32 y, delta, coord, rotation, inc;
    /* TODO: Investigate a falling debris callback with a foreign native payload */
    XASSERT(model && model->payload_kind == MODEL_PAYLOAD_BALLISTIC);
    BALLISTIC_DEBRIS *debris = &model->ballistic;
    if (!debris->roll_step)
    {
        debris->pitch_step = (uint16)((random_next() & 2047u) + 512u);
        debris->roll_step = (uint16)((random_next() & 511u) + 128u);
        /* TODO: Investigate falling objects without a published native position */
        XASSERT(model);
        model->angles.yaw_bits = (uint16)(((sint32)((uint32)(-(sint64)(model->pos.x.bits))) >> 19) & 2047);
        debris->speed = (sint16)((uint16)-256);
        debris->vertical_speed = (uint16)(random_next() & 63u);
    }
    speed = debris->speed;
    /* TODO: Investigate physics common fields outside both native object pools */
    XASSERT(model);
    angle = model->angles.yaw;
    /* TODO: Investigate a ballistic spin heading outside the sine table */
    XASSERT(angle >= 0 && angle < MATH_SIN_COUNT);
    trig = sin_table[angle];
    delta = (speed * trig) >> 1;
    /* TODO: Investigate falling objects without a published native position */
    XASSERT(model);
    coord = model->pos.x.bits;
    model->pos.x.bits = coord + delta;
    speed = debris->speed;
    angle = model->angles.yaw;
    /* TODO: Investigate a ballistic spin heading outside the cosine table span */
    XASSERT(angle >= -MATH_QUARTER_TURN && angle < MATH_ANGLE_COUNT);
    trig = sin_table[MATH_QUARTER_TURN + angle];
    delta = (speed * trig) >> 1;
    coord = model->pos.z.bits;
    model->pos.z.bits = coord - delta;
    rotation = model->angles.pitch_bits;
    inc = (uint16)debris->pitch_step;
    model->angles.pitch_bits = (uint16)(rotation + inc);
    rotation = model->angles.roll_bits;
    inc = (uint16)debris->roll_step;
    model->angles.roll_bits = (uint16)(rotation + inc);
    velocity = debris->vertical_speed;
    if (velocity >= -255)
        debris->vertical_speed = (uint16)(velocity - 3);
    delta = (uint32)(sint32)debris->vertical_speed << 15;
    y = model->pos.y.bits - delta;
    model->pos.y.bits = y;
    if ((sint32)y > 0x02000000)
    {
        model->kind = 0xffffffffu;
        return -1;
    }
    return 0;
}

uint32 object_update_drifting_spin(MODEL_OBJECT *model)
{
    FUNCTION_MARKER(0x8002c454u, "GAME.EXE");
    uint32 result, delta, coord, offset, random;
    /* TODO: Investigate a falling debris callback with a foreign native payload */
    XASSERT(model && model->payload_kind == MODEL_PAYLOAD_DRIFT);
    DRIFTING_DEBRIS *debris = &model->drift;
    if (!debris->phase)
    {
        debris->phase = (uint16)random_next();
        random = random_next();
        /* TODO: Investigate physics common fields outside both native object pools */
        XASSERT(model);
        model->angles.pitch_bits = (uint16)random;
        model->angles.roll_bits = (uint16)random_next();
        debris->speed = (uint16)(32);
    }
    delta = (uint32)(sint32)debris->speed << 16;
    /* TODO: Investigate falling objects without a published native position */
    XASSERT(model);
    coord = model->pos.z.bits;
    model->pos.z.bits = coord + delta;
    model->angles.pitch_bits = (uint16)(model->angles.pitch_bits + 1120);
    model->angles.roll_bits = (uint16)(model->angles.roll_bits + 1824);
    offset = ((uint32)(sint32)debris->phase << 3) & 0xff8;
    delta = (uint32)(sint32)sin_table[offset >> 1] << 4;
    result = model->pos.y.bits - delta;
    model->pos.y.bits = result;
    if ((sint32)result > 0)
        model->pos.y.bits = 0;
    if ((sint32)model->pos.z.bits > 0x40000000)
        model->kind = 0xffffffffu;
    debris->phase = (uint16)((uint16)debris->phase + 1);
    result = (uint16)debris->speed + 1;
    debris->speed = (uint16)result;
    return result;
}

sint32 object_update_falling_panel(MODEL_OBJECT *model)
{
    FUNCTION_MARKER(0x8002cef8u, "GAME.EXE");
    FIXED_VEC3 point;
    sint32 height = 0, angle, timer;
    uint32 offset, random;
    /* TODO: Investigate panel callbacks without the matching native payload */
    XASSERT(model && model->payload_kind == MODEL_PAYLOAD_PANEL);
    FALLING_PANEL *panel = &model->panel;
    if (!panel->initialized)
    {
        panel->initialized = (uint16)(1);
        panel->roll_step = (uint16)((random_next() & 4095u) + 135);
        panel->pitch_step = (uint16)((random_next() & 4095u) + 135);
        /* TODO: Investigate physics common fields outside both native object pools */
        XASSERT(model);
        model->angles.yaw_bits = (uint16)((uint16)model->angles.yaw - 2048);
        random = random_next() & 4095;
        model->angles.yaw_bits = (uint16)((uint16)model->angles.yaw + random);
        panel->vertical_speed = (uint16)((random_next() & 15) + 32);
        panel->speed = (uint16)((random_next() & 63) + 64);
    }
    /* TODO: Investigate physics common fields outside both native object pools */
    XASSERT(model);
    offset = (uint16)model->angles.yaw >> 5;
    offset *= 2;
    /* TODO: Investigate falling objects without a published native position */
    XASSERT(model);
    model->pos.x.bits = model->pos.x.bits + ((panel->speed * sin_table[offset >> 1]) >> 1);
    model->pos.z.bits = model->pos.z.bits - ((panel->speed * sin_table[MATH_QUARTER_TURN + (offset >> 1)]) >> 1);
    panel->speed = (uint16)((uint16)panel->speed - 2);
    if (panel->speed < 0)
        panel->speed = (uint16)(0);
    model->pos.y.bits = model->pos.y.bits - ((uint32)(sint32)panel->vertical_speed << 15);
    panel->vertical_speed = (uint16)((uint16)panel->vertical_speed - 8);
    transform_push();
    transform_identity();
    transform_pitch(32768);
    transform_yaw(model->angles.yaw);
    transform_pitch(model->angles.pitch);
    transform_roll(model->angles.roll);
    transform_point(0x00600000, -0x5c0000, 0, point.bits);
    if (height < point.y.val)
        height = point.y.val;
    transform_point(0x00600000, 0x005c0000, 0, point.bits);
    if (height < point.y.val)
        height = point.y.val;
    transform_point(-0x600000, -0x5c0000, 0, point.bits);
    if (height < point.y.val)
        height = point.y.val;
    transform_point(-0x600000, 0x005c0000, 0, point.bits);
    if (height < point.y.val)
        height = point.y.val;
    transform_pop();
    if ((sint32)model->pos.y.bits >= (sint32)((uint32)(-(sint64)(height))))
    {
        model->pos.y.bits = (uint32)(-(sint64)(height));
        if (panel->pitch_step)
            panel->retire_ticks = (sint16)(uint16)(65535);
        panel->vertical_speed = (uint16)(0);
        panel->roll_step = (uint16)(0);
        panel->pitch_step = (uint16)(0);
        angle = (sint16)(16384u - (uint16)model->angles.pitch);
        model->angles.pitch_bits = (uint16)((uint16)model->angles.pitch + (angle >> 1));
        angle = model->angles.roll;
        model->angles.roll_bits = (uint16)(angle + ((-angle) >> 1));
        /* TODO: Review the original bounce calculation after clearing vertical speed */
        panel->vertical_speed = (uint16)((-panel->vertical_speed) >> 1);
    }
    model->angles.roll_bits = (uint16)((uint16)model->angles.roll + (uint16)panel->roll_step);
    model->angles.pitch_bits = (uint16)((uint16)model->angles.pitch + (uint16)panel->pitch_step);
    timer = panel->retire_ticks;
    if (timer < 0)
    {
        timer = (sint16)(timer - 1);
        panel->retire_ticks = (uint16)timer;
        if (timer < -60)
            model->kind = 0xffffffff;
        return -1;
    }
    return timer;
}

uint32 object_update_falling_panel_sideways(MODEL_OBJECT *model)
{
    FUNCTION_MARKER(0x8002d2a4u, "GAME.EXE");
    FIXED_VEC3 point;
    uint32 result;
    sint32 height = 0, angle, extent_z = 0x600000, extent_y = 0x5c0000;
    uint32 offset, random;
    /* TODO: Investigate panel callbacks without the matching native payload */
    XASSERT(model && model->payload_kind == MODEL_PAYLOAD_SIDE_PANEL);
    FALLING_PANEL *panel = &model->panel;
    if (!panel->initialized)
    {
        panel->initialized = (uint16)(1);
        panel->roll_step = (uint16)((random_next() & 4095u) + 135);
        panel->pitch_step = (uint16)((random_next() & 4095u) + 135);
        /* TODO: Investigate physics common fields outside both native object pools */
        XASSERT(model);
        model->angles.yaw_bits = (uint16)((uint16)model->angles.yaw - 2048);
        random = random_next() & 4095;
        model->angles.yaw_bits = (uint16)((uint16)model->angles.yaw + random);
        panel->vertical_speed = (uint16)((random_next() & 15) + 32);
        panel->speed = (uint16)((random_next() & 63) + 64);
    }
    /* TODO: Investigate physics common fields outside both native object pools */
    XASSERT(model);
    offset = ((model->angles.yaw >> 5) + 512) & 2047;
    /* TODO: Investigate falling objects without a published native position */
    XASSERT(model);
    if ((sint32)model->pos.x.bits < 0)
        offset ^= 1024;
    offset *= 2;
    /* TODO: Investigate falling objects without a published native position */
    XASSERT(model);
    model->pos.x.bits = model->pos.x.bits + ((panel->speed * sin_table[offset >> 1]) >> 1);
    model->pos.z.bits = model->pos.z.bits - ((panel->speed * sin_table[MATH_QUARTER_TURN + (offset >> 1)]) >> 1);
    panel->speed = (uint16)((uint16)panel->speed - 2);
    if (panel->speed < 0)
        panel->speed = (uint16)(0);
    model->pos.y.bits = model->pos.y.bits - ((uint32)(sint32)panel->vertical_speed << 15);
    panel->vertical_speed = (uint16)((uint16)panel->vertical_speed - 8);
    transform_push();
    transform_identity();
    transform_pitch(32768);
    transform_yaw(model->angles.yaw);
    transform_pitch(model->angles.pitch);
    transform_roll(model->angles.roll);
    if ((sint32)model->kind >= 350)
    {
        extent_z = 0x800000;
        extent_y = 0x500000;
    }
    transform_point(0, -extent_y, extent_z, point.bits);
    if (height < point.y.val)
        height = point.y.val;
    transform_point(0, extent_y, extent_z, point.bits);
    if (height < point.y.val)
        height = point.y.val;
    transform_point(0, -extent_y, -extent_z, point.bits);
    if (height < point.y.val)
        height = point.y.val;
    transform_point(0, extent_y, -extent_z, point.bits);
    if (height < point.y.val)
        height = point.y.val;
    transform_pop();
    if ((sint32)model->pos.y.bits >= (sint32)((uint32)(-(sint64)(height))))
    {
        model->pos.y.bits = (uint32)(-(sint64)(height));
        panel->vertical_speed = (uint16)(0);
        panel->roll_step = (uint16)(0);
        panel->pitch_step = (uint16)(0);
        angle = (sint16)(16384u - (uint16)model->angles.roll);
        model->angles.roll_bits = (uint16)((uint16)model->angles.roll + (angle >> 1));
        angle = model->angles.pitch;
        model->angles.pitch_bits = (uint16)(angle + ((-angle) >> 1));
        /* TODO: Review the original bounce calculation after clearing vertical speed */
        panel->vertical_speed = (uint16)((-panel->vertical_speed) >> 1);
    }
    model->angles.roll_bits = (uint16)((uint16)model->angles.roll + (uint16)panel->roll_step);
    result = ((uint16)model->angles.pitch) + (uint16)panel->pitch_step;
    model->angles.pitch_bits = (uint16)result;
    return result;
}

uint32 object_update_shake(MODEL_OBJECT *model)
{
    FUNCTION_MARKER(0x8002d218u, "GAME.EXE");
    sint32 timer;
    uint32 odd;
    /* TODO: Investigate shake callbacks without the matching native owner */
    XASSERT(model && model->payload_kind == MODEL_PAYLOAD_SHAKE);
    OBJECT_SHAKE *shake = &model->shake;
    timer = shake->ticks;
    if (!timer)
    {
        shake->ox = model->pos.x.whole;
        shake->oz = model->pos.z.whole;
        timer = shake->ticks;
    }
    if (timer > 0)
        shake->ticks = timer - 1;
    odd = (uint16)shake->ticks & 1u;
    model->pos.x.bits = ((shake->ox) + (odd << 3)) << 16;
    odd = (uint16)shake->ticks & 1u;
    model->pos.z.bits = ((shake->oz) + (odd << 4)) << 16;
    return odd << 4;
}

uint32 object_update_ground_bounce(MODEL_OBJECT *model)
{
    FUNCTION_MARKER(0x8002df00u, "GAME.EXE");
    uint16 velocity;
    uint32 result;
    sint32 count;
    /* TODO: Investigate bounce callbacks without their native part owner */
    XASSERT(model && model->payload_kind == MODEL_PAYLOAD_BOUNCING_PART);
    BOUNCING_PART *part = &model->bounce;
    model->pos.y.bits = model->pos.y.bits + ((uint32)(sint32)part->vertical_speed << 15);
    velocity = (uint16)part->vertical_speed + 8u;
    part->vertical_speed = velocity;
    if ((sint32)model->pos.y.bits > 0 && (sint16)velocity > 0)
    {
        model->pos.y.bits = 0;
        part->vertical_speed = (-part->vertical_speed) >> 2;
        count = part->bounces;
        if (count >= 0)
        {
            count = (sint16)(count - 1);
            part->bounces = count;
        }
        if (count < 0)
            part->vertical_speed = 0;
    }
    result = (sint32)model->pos.y.bits >> 13;
    model->angles.pitch_bits = (uint16)result;
    return result;
}

uint32 object_update_spinning_landing(MODEL_OBJECT *model)
{
    FUNCTION_MARKER(0x8002dfa4u, "GAME.EXE");
    sint32 result;
    /* TODO: Investigate bounce callbacks without their native part owner */
    XASSERT(model && model->payload_kind == MODEL_PAYLOAD_BOUNCING_PART);
    BOUNCING_PART *part = &model->bounce;
    model->pos.y.bits = model->pos.y.bits + ((uint32)(sint32)part->vertical_speed << 15);
    part->vertical_speed = (uint16)part->vertical_speed + 8u;
    if (part->bounces > 0)
    {
        model->angles.pitch_bits = (uint16)((uint16)model->angles.pitch + 64u);
        model->angles.roll_bits = (uint16)((uint16)model->angles.roll + 1024u);
    }
    result = (sint32)model->pos.y.bits > -16777216;
    if (result)
    {
        result = part->vertical_speed;
        if (result > 0)
        {
            model->pos.y.bits = 0xff000000u;
            part->vertical_speed = 0;
            part->bounces = 0;
        }
    }
    return result;
}

uint32 object_update_sliding_bounce(MODEL_OBJECT *model)
{
    FUNCTION_MARKER(0x8002e038u, "GAME.EXE");
    sint32 speed;
    uint32 y, result, delta;
    uint16 vertical;
    sint32 count;
    /* TODO: Investigate bounce callbacks without their native part owner */
    XASSERT(model && model->payload_kind == MODEL_PAYLOAD_BOUNCING_PART);
    BOUNCING_PART *part = &model->bounce;
    speed = part->speed - 4;
    speed = speed < 0 ? 0 : speed;
    part->speed = speed;
    if (model->angles.yaw == 16384)
        speed = -speed;
    model->pos.x.bits = model->pos.x.bits + ((uint32)speed << 16);
    delta = (uint32)(sint32)part->vertical_speed << 14;
    y = model->pos.y.bits + delta;
    model->pos.y.bits = y;
    vertical = (uint16)part->vertical_speed + 10;
    part->vertical_speed = vertical;
    y = model->pos.y.bits;
    if ((sint32)y > 0 && (sint16)vertical > 0)
    {
        model->pos.y.bits = 0;
        part->vertical_speed = (-part->vertical_speed) >> 2;
        count = part->bounces;
        if (count >= 0)
        {
            part->bounces = count - 1;
            count = (sint16)(count - 1);
        }
        if (count < 0)
            part->vertical_speed = 0;
    }
    result = (sint32)model->pos.y.bits >> 13;
    model->angles.pitch_bits = (uint16)result;
    return result;
}

uint32 object_update_stationary_bounce(MODEL_OBJECT *model)
{
    FUNCTION_MARKER(0x8002e048u, "GAME.EXE");
    sint32 speed = 0;
    uint32 y, result, delta;
    uint16 vertical;
    sint32 count;
    /* TODO: Investigate bounce callbacks without their native part owner */
    XASSERT(model && model->payload_kind == MODEL_PAYLOAD_BOUNCING_PART);
    BOUNCING_PART *part = &model->bounce;
    part->speed = speed;
    if (model->angles.yaw == 16384)
        speed = -speed;
    model->pos.x.bits = model->pos.x.bits + ((uint32)speed << 16);
    delta = (uint32)(sint32)part->vertical_speed << 14;
    y = model->pos.y.bits + delta;
    model->pos.y.bits = y;
    vertical = (uint16)part->vertical_speed + 10;
    part->vertical_speed = vertical;
    y = model->pos.y.bits;
    if ((sint32)y > 0 && (sint16)vertical > 0)
    {
        model->pos.y.bits = 0;
        part->vertical_speed = (-part->vertical_speed) >> 2;
        count = part->bounces;
        if (count >= 0)
        {
            part->bounces = count - 1;
            count = (sint16)(count - 1);
        }
        if (count < 0)
            part->vertical_speed = 0;
    }
    result = (sint32)model->pos.y.bits >> 13;
    model->angles.pitch_bits = (uint16)result;
    return result;
}

sint32 dynamic_objects_allocate(void)
{
    FUNCTION_MARKER(0x800165ecu, "GAME.EXE");
    uint32 i;
    for (i = 0; i < DYNAMIC_OBJECT_COUNT; i++)
        if (dynamic_objects[i].kind == 0xffffffffu)
        {
            dynamic_objects[i] = (MODEL_OBJECT){0};
            return i;
        }
    return -1;
}

uint32 object_update_gate_shake(MODEL_OBJECT *model)
{
    FUNCTION_MARKER(0x8002cde0u, "GAME.EXE");
    sint32 timer;
    uint32 coord;
    uint32 odd;
    /* TODO: Investigate shake callbacks without the matching native owner */
    XASSERT(model && model->payload_kind == MODEL_PAYLOAD_SHAKE);
    OBJECT_SHAKE *shake = &model->shake;
    timer = shake->ticks;
    if (!timer)
    {
        shake->ox = model->pos.x.whole;
        shake->oz = model->pos.z.whole;
        timer = shake->ticks;
    }
    if (timer > 0)
        shake->ticks = timer - 1;
    coord = shake->ox;
    odd = (uint16)shake->ticks & 1;
    model->pos.x.bits = (coord + (odd << 4)) << 16;
    coord = shake->oz;
    odd = (uint16)shake->ticks & 1;
    model->pos.z.bits = (coord + (odd << 3)) << 16;
    return odd << 3;
}

uint32 object_update_gate_panels_shake(MODEL_OBJECT *model)
{
    FUNCTION_MARKER(0x8002ce6cu, "GAME.EXE");
    sint32 timer;
    uint32 origin, odd;
    /* TODO: Investigate shake callbacks without the matching native owner */
    XASSERT(model && model->payload_kind == MODEL_PAYLOAD_SHAKE);
    OBJECT_SHAKE *shake = &model->shake;
    timer = shake->ticks;
    if (timer == 0)
    {
        shake->ox = model->pos.x.whole;
        shake->oz = model->pos.z.whole;
        timer = shake->ticks;
    }
    if (timer > 0)
        shake->ticks = timer - 1;
    origin = shake->ox;
    odd = (uint16)shake->ticks & 1;
    model->pos.x.bits = (origin + (odd << 3)) << 16;
    origin = shake->oz;
    odd = (uint16)shake->ticks & 1;
    model->pos.z.bits = (origin + (odd << 4)) << 16;
    return odd << 4;
}

uint32 object_start_gate_panels_shake(MODEL_OBJECT *model)
{
    FUNCTION_MARKER(0x8002ce78u, "GAME.EXE");
    sint32 timer;
    uint32 origin, odd;
    /* TODO: Investigate shake callbacks without the matching native owner */
    XASSERT(model && model->payload_kind == MODEL_PAYLOAD_SHAKE);
    OBJECT_SHAKE *shake = &model->shake;
    shake->ox = model->pos.x.whole;
    shake->oz = model->pos.z.whole;
    timer = shake->ticks;
    if (timer > 0)
        shake->ticks = timer - 1;
    origin = shake->ox;
    odd = (uint16)shake->ticks & 1;
    model->pos.x.bits = (origin + (odd << 3)) << 16;
    origin = shake->oz;
    odd = (uint16)shake->ticks & 1;
    model->pos.z.bits = (origin + (odd << 4)) << 16;
    return odd << 4;
}

uint32 object_update_falling_gate(MODEL_OBJECT *model)
{
    FUNCTION_MARKER(0x8002d7c8u, "GAME.EXE");
    uint16 speed, angle;
    sint32 count, offset;
    uint32 i, z, result;
    /* TODO: Investigate a callback without its matching native payload */
    XASSERT(model && model->payload_kind == MODEL_PAYLOAD_GATE_FALL);
    FALLING_GATE *gate_fall = &model->gate_fall;
    speed = (uint16)gate_fall->pitch_speed + 128u;
    gate_fall->pitch_speed = speed;
    angle = (uint16)model->angles.pitch - speed;
    model->angles.pitch_bits = angle;
    if ((sint16)angle >= -15360)
        return 0;
    count = gate_fall->bounces;
    if (count < 4)
    {
        gate_fall->bounces = count + 1;
        disp_buffers.shake_ticks = 6;
        if (gate_fall->bounces == 1)
            sound_voice_play(116, 255, 6144);
        for (i = 0; i < 32; i++)
        {
            offset = (random_next() & 1023) - 512;
            offset += offset >> 3;
            z = ((random_next() & 255) + 320u) << 16;
            uint32 spawn_z = z + model->pos.z.bits;
            uint32 spawn_x = ((uint32)offset << 16) + model->pos.x.bits;
            effect_spawn_timed(870, spawn_x, 0, spawn_z, 0, 0, 0);
        }
    }
    model->angles.pitch_bits = 0xc400;
    result = (uint32)(-(sint64)(gate_fall->pitch_speed >> 2));
    gate_fall->pitch_speed = result;
    return result;
}

sint32 object_update_gate_fragment(MODEL_OBJECT *model)
{
    FUNCTION_MARKER(0x8002d8fcu, "GAME.EXE");
    uint32 angle;
    sint32 result;
    /* TODO: Investigate a callback without its matching native payload */
    XASSERT(model && model->payload_kind == MODEL_PAYLOAD_GATE_FRAGMENT);
    GATE_FRAGMENT *gate_fragment = &model->gate_fragment;
    gate_fragment->pitch_speed = gate_fragment->pitch_speed + (model->kind == 72 ? 96 : 64);
    model->angles.pitch_bits = (uint16)(model->angles.pitch - gate_fragment->pitch_speed);
    angle = 2u * ((uint32)(uint16)model->angles.yaw >> 5);
    model->pos.x.bits = model->pos.x.bits - ((gate_fragment->speed * sin_table[angle >> 1]) >> 1);
    model->pos.z.bits = model->pos.z.bits + ((gate_fragment->speed * sin_table[MATH_QUARTER_TURN + (angle >> 1)]) >> 1);
    gate_fragment->speed = gate_fragment->speed - 64;
    if (gate_fragment->speed < 0)
        gate_fragment->speed = 0;
    if (model->angles.pitch >= -16384)
        return -16384;
    model->angles.pitch_bits = (uint16)-16384;
    result = -(gate_fragment->pitch_speed >> 1);
    gate_fragment->pitch_speed = result;
    return result;
}

uint32 object_update_oscillation(MODEL_OBJECT *model)
{
    FUNCTION_MARKER(0x8002da3cu, "GAME.EXE");
    sint32 angle, velocity, timer;
    uint32 result = 720;
    /* TODO: Investigate common fields outside both native object pools */
    XASSERT(model);
    /* TODO: Investigate oscillation update without the matching native owner */
    XASSERT(model && model->payload_kind == MODEL_PAYLOAD_OSCILLATION);
    angle = model->angles.pitch;
    velocity = model->oscillation.pitch_speed;
    velocity += (-angle - velocity) >> 3;
    model->oscillation.pitch_speed = velocity;
    angle = (sint16)(angle + ((sint16)velocity >> 2));
    model->angles.pitch_bits = (uint16)angle;
    if (angle > 1024)
        model->angles.pitch_bits = 1024;
    if (model->angles.pitch < -1024)
        model->angles.pitch_bits = (uint16)-1024;
    angle = model->angles.roll;
    velocity = model->oscillation.roll_speed;
    velocity += (-angle - velocity) >> 3;
    model->oscillation.roll_speed = velocity;
    angle = (sint16)(angle + ((sint16)velocity >> 2));
    model->angles.roll_bits = (uint16)angle;
    if (angle > 1536)
        model->angles.roll_bits = 1536;
    if (model->angles.roll < -1536)
        model->angles.roll_bits = (uint16)-1536;
    timer = model->oscillation.sound_clock;
    if (timer > 0)
    {
        model->oscillation.sound_clock = timer - 1;
        if (timer == 1)
        {
            /* The sound may already have stopped before this timer expires */
            if (model->oscillation.voice_plus_one)
                sound_voice_stop(model->oscillation.voice_plus_one - 1);
            model->oscillation.voice_plus_one = 0;
        }
    }
    if (model->kind == 720)
    {
        model->angles.yaw_bits = (uint16)((uint16)model->angles.yaw + 256u);
        MODEL_OBJECT *next = NULL;
        /* Preserve the next element of the same native pool */
        uint32 i;
        for (i = 0; i + 1u < WORLD_OBJECT_COUNT; i++)
            if (model == world_objects.object + i)
            {
                next = world_objects.object + i + 1u;
                break;
            }
        if (!next)
            for (i = 0; i + 1u < DYNAMIC_OBJECT_COUNT; i++)
                if (model == dynamic_objects + i)
                {
                    next = dynamic_objects + i + 1u;
                    break;
                }
        /* TODO: Investigate a missing adjacent object after the current yaw update */
        XASSERT(next);
        result = next->angles.yaw_bits + 256u;
        next->angles.yaw_bits = (uint16)result;
    }
    return result;
}

uint32 object_update_compression(MODEL_OBJECT *model)
{
    FUNCTION_MARKER(0x8002dba0u, "GAME.EXE");
    sint32 voice, pitch, offset, phase;
    uint32 i, x, z, y, age;
    FIXED_VEC3 point;
    object_update_oscillation(model);
    /* TODO: Investigate compression update without the matching native owner */
    XASSERT(model && model->payload_kind == MODEL_PAYLOAD_OSCILLATION);
    if (!model->oscillation.phase)
        model->oscillation.sound_clock = 6144;
    voice = model->oscillation.voice_plus_one;
    if (voice > 0)
    {
        sound_voice_pitch(voice - 1, model->oscillation.sound_clock);
        pitch = (sint16)((uint16)model->oscillation.sound_clock - 16u);
        model->oscillation.sound_clock = pitch;
        if (pitch < 4096)
        {
            sound_voice_stop(model->oscillation.voice_plus_one - 1);
            model->oscillation.voice_plus_one = 0;
        }
    }
    if (model->oscillation.phase == 2)
    {
        transform_push();
        transform_identity();
        transform_pitch(32768);
        /* TODO: Investigate common fields outside both native object pools */
        XASSERT(model);
        transform_yaw(model->angles.yaw);
        x = model->pos.x.bits;
        z = model->pos.z.bits;
        disp_buffers.shake_ticks = 8;
        for (i = 0; i < 24; i++)
        {
            offset = (random_next() & 511) - 256;
            offset += offset >> 2;
            y = (0xffffff70u - (random_next() & 31)) << 16;
            transform_point(y, 0, (uint32)offset << 16, point.bits);
            y = ((uint32)(-(sint64)(random_next() & 31))) << 16;
            age = random_next() & 3;
            effect_spawn_timed(871, x + point.x.bits, y, z + point.z.bits, age, 0, 0);
            offset = (random_next() & 511) - 256;
            y = ((random_next() & 31) + 144u) << 16;
            transform_point(y, 0, (uint32)offset << 16, point.bits);
            y = ((uint32)(-(sint64)(random_next() & 31))) << 16;
            age = random_next() & 3;
            effect_spawn_timed(871, x + point.x.bits, y, z + point.z.bits, age, 0, 0);
        }
        transform_pop();
    }
    phase = model->oscillation.phase;
    if (phase < 4)
        model->oscillation.phase = phase + 1;
    return phase + 1;
}

sint32 object_update_wobble(MODEL_OBJECT *model)
{
    FUNCTION_MARKER(0x8002d5d4, "GAME.EXE");
    /* TODO: Investigate common fields outside both native object pools */
    XASSERT(model);
    /* TODO: Investigate wobble access without the matching native owner */
    XASSERT(model && model->payload_kind == MODEL_PAYLOAD_WOBBLE);
    sint32 timer = model->wobble.ticks;
    uint32 s = sin_table[((timer & 7u) << 9) >> 1];
    sint32 angle = (sint32)(s * timer) >> 9;
    model->angles.bits[model->kind - 786u < 2u ? 2 : 1] = (uint16)angle;
    s = sin_table[(((uint32)(uint16)model->angles.pitch >> 4) & 0x7fe) >> 1];
    model->pos.y.bits = ((uint32)(-(sint64)(s))) << 8;
    timer = model->wobble.ticks;
    if (timer > 0)
    {
        --timer;
        model->wobble.ticks = timer;
    }
    return timer;
}

intptr object_update_wobble_debris(MODEL_OBJECT *model)
{
    FUNCTION_MARKER(0x8002d678u, "GAME.EXE");
    uint32 kind, x, z;
    /* TODO: Investigate common fields outside both native object pools */
    XASSERT(model);
    /* TODO: Investigate wobble access without the matching native owner */
    XASSERT(model && model->payload_kind == MODEL_PAYLOAD_WOBBLE);
    sint32 phase = model->wobble.ticks, angle = (sin_table[((phase & 7u) << 9) >> 1] * phase) >> 9, count;
    model->angles.bits[model->kind == 788 ? 2 : 1] = (uint16)angle;
    model->pos.y.bits = ((uint32)(-(sint64)(sin_table[(((uint32)(uint16)model->angles.pitch >> 4) & 0x7feu) >> 1]))) << 8;
    if (model->wobble.ticks > 0)
        model->wobble.ticks = model->wobble.ticks - 1;
    count = model->wobble.emit_ticks;
    if (count <= 0)
        return count;
    model->wobble.emit_ticks = count - 1;
    kind = model->kind == 428 ? 872 : 871;
    if ((uint16)model->wobble.emit_ticks & 1u)
        return 1;
    x = ((random_next() & 63u) - 32u) << 16;
    x += model->pos.x.bits;
    z = ((random_next() & 63u) - 32u) << 16;
    z += model->pos.z.bits;
    return (intptr)effect_spawn_timed(kind, x, 0xffc00000u, z, 0, 0, 0);
}

sint32 object_update_wobble_particles(MODEL_OBJECT *model)
{
    FUNCTION_MARKER(0x8002e8a4u, "GAME.EXE");
    uint32 kind, x, z;
    uint32 height;
    sint32 phase;
    sint32 angle;
    if (!(game_session.tick & 3u))
    {
        /* TODO: Investigate common fields outside both native object pools */
        XASSERT(model);
        kind = model->kind;
        height = kind == 809 || kind == 811 ? -224 : -112;
        x = model->pos.x.bits;
        if (kind == 805)
            x += 0x300000u;
        if (model->kind == 807)
            x -= 0x300000u;
        x += ((random_next() & 31u) - 16u) << 16;
        z = ((random_next() & 31u) - 16u) << 16;
        z += model->pos.z.bits;
        effect_spawn_timed(871, x, height << 16, z, 0, 0, 0);
    }
    /* TODO: Investigate wobble access without the matching native owner */
    XASSERT(model && model->payload_kind == MODEL_PAYLOAD_WOBBLE);
    phase = model->wobble.ticks;
    angle = (sin_table[((phase & 7u) << 9) >> 1] * phase) >> 9;
    /* TODO: Investigate common fields outside both native object pools */
    XASSERT(model);
    model->angles.pitch_bits = (uint16)angle;
    model->pos.y.bits = ((uint32)(-(sint64)(sin_table[(((uint32)angle >> 4) & 0x7feu) >> 1]))) << 8;
    phase = model->wobble.ticks;
    if (phase > 0)
    {
        phase--;
        model->wobble.ticks = phase;
    }
    return phase;
}

intptr object_update_smoke_emitter(MODEL_OBJECT *model)
{
    FUNCTION_MARKER(0x8002e9fcu, "GAME.EXE");
    uint32 volume, pan, x, z;
    EFFECT *effect;
    /* TODO: Investigate wobble access without the matching native owner */
    XASSERT(model && model->payload_kind == MODEL_PAYLOAD_WOBBLE);
    if (model->wobble.voice_plus_one)
    {
        /* TODO: Investigate an object operation without a native position */
        XASSERT(model);
        sound_spatial(&model->pos, &volume, &pan);
        volume = sound_volume_adjust(volume, -128);
        sound_voice_pan(model->wobble.voice_plus_one - 1, volume, pan);
    }
    else
    {
        uint32 random = random_next();
        model->wobble.voice_plus_one = (uint16)(sound_voice_play(154, 0, (sint32)(random & 63u) + 6144) + 1u);
    }
    if (game_session.tick & 1u)
        return 410;
    /* TODO: Investigate an object operation without a native position */
    XASSERT(model);
    x = model->pos.x.bits;
    if (model->kind == 410)
        x -= 0x600000u;
    x += ((random_next() & 31u) - 16u) << 16;
    z = ((random_next() & 31u) - 16u) << 16;
    z += model->pos.z.bits;
    effect = effect_spawn_timed(872, x, 0xff100000u, z, 0, 0, 0);
    /* TODO: Investigate the original unchecked allocation writing to address0x1A */
    XASSERT(effect && effect->kind == 872 && effect->payload_kind == EFFECT_PAYLOAD_SPRITE);
    effect->spr.rise = (uint16)((32767 - sin_table[((game_session.tick << 3) & 0x7f8u) >> 1]) >> 10);
    return (intptr)effect;
}

sint32 object_update_explosive(MODEL_OBJECT *item)
{
    FUNCTION_MARKER(0x8002bc1cu, "GAME.EXE");
    uint32 state, result, x, y, z;
    ANIMATION_BANK_ID bank;
    sint32 timer, i, type, distance, object_x, object_z, actor_x, actor_z;
    object_update_physics(item);
    /* TODO: Investigate foreign objects or unpublished native positions */
    XASSERT(item);
    /* TODO: Investigate explosive callbacks without a compatible native owner */
    XASSERT(item && item->payload_kind == MODEL_PAYLOAD_PHYSICS);
    timer = item->physics.life;
    if (timer <= 0)
        return timer;
    item->physics.life = timer - 1;
    result = (uint32)(timer - 1) << 16;
    if (result)
        return result;
    sound_voice_play(19, 255, 6144);
    x = item->pos.x.bits;
    y = item->pos.y.bits;
    z = item->pos.z.bits;
    effect_spawn(892, x, y, z);
    x = item->pos.x.bits;
    z = item->pos.z.bits;
    effect_spawn(603, x, 0, z);
    disp_buffers.shake_ticks = 16;
    disp_buffers.flash_ticks = 4;
    /* The retained radial point word is the reviewed saved return address */
    regions_apply_radial_hit(&item->pos, 1024, 0x80031c80);
    for (i = 0; i < 8; i++)
    {
        type = actor_types[i];
        if (type == -1 || actor_lifecycle[i].delay || type == 17)
            continue;
        state = actor_animations[i].id;
        if (state == 205 || state == 203 || actor_health[i].hp <= 0)
            continue;
        object_x = item->pos.x.whole;
        object_z = item->pos.z.whole;
        actor_x = actor_positions[i].x.whole;
        actor_z = actor_positions[i].z.whole;
        distance = math_distance_approx(object_x, object_z, actor_x, actor_z);
        if (distance >= 1024)
            continue;
        actor_health[i].hp = (uint16)(((uint16)actor_health[i].hp) - 128u + (distance >> 4));
        object_x = item->pos.x.whole;
        object_z = item->pos.z.whole;
        actor_x = actor_positions[i].x.whole;
        actor_z = actor_positions[i].z.whole;
        result = math_dir11(object_x, object_z, actor_x, actor_z);
        actor_orientation[i].yaw = (uint16)(result ^ 1024u);
        actor_items_drop(i);
        type = actor_types[i];
        /* TODO: Investigate an invalid reloaded actor type after dropping held items */
        XASSERT(type >= 0 && type < ANIMATION_TYPE_COUNT);
        bank = animation_type_banks[type];
        animation_start(i, bank, 147);
    }
    item->kind = 0xffffffffu;
    return -1;
}

sint32 object_update_island_fragment(MODEL_OBJECT *model)
{
    FUNCTION_MARKER(0x8002ee60u, "GAME.EXE");
    /* TODO: Investigate common motion fields outside the native object pools */
    XASSERT(model);
    /* TODO: Investigate an island fragment with a foreign native payload */
    XASSERT(model->payload_kind == MODEL_PAYLOAD_PHYSICS);
    uint32 offset = ((uint16)model->angles.yaw >> 5) * 2u, y, result;
    if (!((uint16)model->physics.fall_speed & 63u))
    {
        /* TODO: Investigate motion callbacks without a native world or dynamic position */
        XASSERT(model);
        dynamic_objects_spawn(575, model->pos.x.bits, model->pos.y.bits, model->pos.z.bits);
    }
    if (!model->physics.fall_speed)
        model->physics.slide_speed = (uint16)((random_next() & 31u) + 256u);
    /* TODO: Investigate motion callbacks without a native world or dynamic position */
    XASSERT(model);
    model->pos.x.bits = model->pos.x.bits + ((sin_table[offset >> 1] * model->physics.slide_speed) >> 3);
    model->pos.z.bits = model->pos.z.bits - ((sin_table[MATH_QUARTER_TURN + (offset >> 1)] * model->physics.slide_speed) >> 3);
    model->physics.fall_speed = (uint16)((uint16)model->physics.fall_speed + 32u);
    model->angles.pitch_bits = (uint16)(((uint16)model->angles.pitch) + (model->physics.fall_speed >> 1));
    if (model->angles.pitch > 20480)
        model->angles.pitch_bits = 20480;
    y = model->pos.y.bits + ((uint32)(sint32)model->physics.fall_speed << 9);
    model->pos.y.bits = y;
    if ((sint32)y > (sint32)0xfda00000u)
    {
        model->pos.y.bits = 0xfda00000;
        model->angles.pitch_bits = 20480;
    }
    result = ((uint16)model->physics.slide_speed) - 16u;
    model->physics.slide_speed = (uint16)result;
    result <<= 16;
    if ((sint32)result < 0)
        model->physics.slide_speed = 0;
    return result;
}

sint32 model_update_spinning(MODEL_OBJECT *model)
{
    FUNCTION_MARKER(0x8002ff30u, "GAME.EXE");
    /* TODO: Investigate common motion fields outside the native object pools */
    XASSERT(model);
    uint32 offset = ((uint16)model->angles.yaw >> 5) * 2u, x, z, dx, dz, val;
    /* TODO: Investigate motion callbacks without a native world or dynamic position */
    XASSERT(model);
    x = model->pos.x.bits;
    z = model->pos.z.bits;
    sint32 i, hit, result, speed;
    /* Each variant names the original signed speed word without assuming native layout */
    switch (model->payload_kind)
    {
        case MODEL_PAYLOAD_TRAIN:
            speed = model->train.speed;
            break;
        case MODEL_PAYLOAD_SUSPENDED:
            speed = model->suspended.actor_idx;
            break;
        case MODEL_PAYLOAD_PANEL:
            speed = model->panel.speed;
            break;
        case MODEL_PAYLOAD_SIDE_PANEL:
            speed = model->panel.speed;
            break;
        case MODEL_PAYLOAD_BALLISTIC:
            speed = model->ballistic.speed;
            break;
        case MODEL_PAYLOAD_DRIFT:
            speed = model->drift.speed;
            break;
        case MODEL_PAYLOAD_SHAKE:
            speed = model->shake.speed;
            break;
        case MODEL_PAYLOAD_GATE_FALL:
            speed = model->gate_fall.speed;
            break;
        case MODEL_PAYLOAD_GATE_FRAGMENT:
            speed = model->gate_fragment.speed;
            break;
        case MODEL_PAYLOAD_ROTATION:
            speed = model->rotation.amplitude;
            break;
        case MODEL_PAYLOAD_BOUNCING_DROP:
            speed = model->drop.speed;
            break;
        case MODEL_PAYLOAD_BOUNCING_PART:
            speed = model->bounce.speed;
            break;
        case MODEL_PAYLOAD_TRAFFIC:
            speed = model->traffic.speed;
            break;
        case MODEL_PAYLOAD_TANK:
            speed = model->tank.speed;
            break;
        case MODEL_PAYLOAD_OSCILLATION:
            speed = model->oscillation.speed;
            break;
        case MODEL_PAYLOAD_COMPANION:
            speed = model->companion.speed;
            break;
        case MODEL_PAYLOAD_LAB_TANK:
            speed = model->lab_tank.speed;
            break;
        case MODEL_PAYLOAD_WOBBLE:
            speed = model->wobble.speed;
            break;
        case MODEL_PAYLOAD_PHYSICS:
            speed = model->physics.speed;
            break;
        case MODEL_PAYLOAD_ARMY_TRUCK:
            speed = model->truck.speed;
            break;
        case MODEL_PAYLOAD_AIR_BIKE:
            speed = model->air_bike.target[0].z;
            break;
        case MODEL_PAYLOAD_SCENE_VEHICLE:
            speed = model->scene_vehicle.speed;
            break;
        case MODEL_PAYLOAD_SCENE_HELICOPTER:
            speed = model->scene_helicopter.speed;
            break;
        case MODEL_PAYLOAD_COMBAT_HELICOPTER:
            speed = model->helicopter.shot_distance;
            break;
        case MODEL_PAYLOAD_PATROL_HELICOPTER:
            speed = model->patrol.speed;
            break;
        case MODEL_PAYLOAD_UNMIGRATED:
            speed = model->pending_motion.speed;
            break;
        case MODEL_PAYLOAD_SPRITE:
            speed = model->spr.speed;
            break;
        case MODEL_PAYLOAD_WAVE:
            speed = model->wave.speed;
            break;
        default:
            /* TODO: Investigate an unpublished spinning-object speed owner */

            XASSERT(0);
    }
    dx = (speed * sin_table[offset >> 1]) >> 1;
    dz = (speed * sin_table[MATH_QUARTER_TURN + (offset >> 1)]) >> 1;
    for (i = 0; i < 4; i++)
    {
        hit = regions_find_projectile_obstacle((sint32)x, (sint32)z);
        if (hit >= 0)
        {
            sound_voice_play(19, 192, 6144);
            effect_spawn(894, model->pos.x.bits, model->pos.y.bits, model->pos.z.bits);
            /* 31C50 saves caller S3 (this loop index) at entry-SP-12. */
            result = projectile_contact_model(hit, model, i);
            model->kind = 0xffffffffu;
            return result;
        }
        x += dx;
        z -= dz;
        model->pos.x.bits = x;
        model->pos.z.bits = z;
    }
    sint16 *vertical_speed = NULL, *pitch_speed = NULL;
    switch (model->payload_kind)
    {
        case MODEL_PAYLOAD_TRAIN:
            vertical_speed = &model->train.vertical_speed;
            pitch_speed = &model->train.spin;
            break;
        case MODEL_PAYLOAD_SUSPENDED:
            vertical_speed = &model->suspended.vertical_speed;
            pitch_speed = &model->suspended.phase;
            break;
        case MODEL_PAYLOAD_PANEL:
            vertical_speed = &model->panel.vertical_speed;
            pitch_speed = &model->panel.initialized;
            break;
        case MODEL_PAYLOAD_SIDE_PANEL:
            vertical_speed = &model->panel.vertical_speed;
            pitch_speed = &model->panel.initialized;
            break;
        case MODEL_PAYLOAD_BALLISTIC:
            vertical_speed = &model->ballistic.vertical_speed;
            pitch_speed = &model->ballistic.roll_step;
            break;
        case MODEL_PAYLOAD_BOUNCING_DROP:
            vertical_speed = &model->drop.vertical_speed;
            pitch_speed = &model->drop.released;
            break;
        case MODEL_PAYLOAD_OSCILLATION:
            vertical_speed = &model->oscillation.sound_clock;
            pitch_speed = &model->oscillation.pitch_speed;
            break;
        case MODEL_PAYLOAD_LAB_TANK:
            vertical_speed = &model->lab_tank.vertical_speed;
            pitch_speed = &model->lab_tank.pitch_speed;
            break;
        case MODEL_PAYLOAD_PHYSICS:
            vertical_speed = &model->physics.vertical_speed;
            pitch_speed = &model->physics.spin;
            break;
        case MODEL_PAYLOAD_ARMY_TRUCK:
            vertical_speed = &model->truck.vertical_speed;
            pitch_speed = &model->truck.active;
            break;
        case MODEL_PAYLOAD_AIR_BIKE:
            vertical_speed = &model->air_bike.phase;
            pitch_speed = &model->air_bike.target[1].x;
            break;
        case MODEL_PAYLOAD_SCENE_VEHICLE:
            vertical_speed = &model->scene_vehicle.vertical_speed;
            pitch_speed = &model->scene_vehicle.start_tick;
            break;
        case MODEL_PAYLOAD_SCENE_HELICOPTER:
            vertical_speed = &model->scene_helicopter.vertical_speed;
            pitch_speed = &model->scene_helicopter.spin;
            break;
        case MODEL_PAYLOAD_COMBAT_HELICOPTER:
            vertical_speed = &model->helicopter.hover_height;
            pitch_speed = &model->helicopter.phase;
            break;
        case MODEL_PAYLOAD_PATROL_HELICOPTER:
            vertical_speed = &model->patrol.vertical_speed;
            pitch_speed = &model->patrol.phase;
            break;
        case MODEL_PAYLOAD_UNMIGRATED:
            vertical_speed = &model->pending_motion.sound_clock;
            pitch_speed = &model->pending_motion.pitch_speed;
            break;
        case MODEL_PAYLOAD_SPRITE:
            vertical_speed = &model->spr.vertical_speed;
            pitch_speed = &model->spr.rise;
            break;
        case MODEL_PAYLOAD_DRIFT:
            vertical_speed = &model->drift.physics_vertical_speed;
            pitch_speed = &model->drift.phase;
            break;
        case MODEL_PAYLOAD_SHAKE:
            vertical_speed = &model->shake.physics_vertical_speed;
            pitch_speed = &model->shake.oz;
            break;
        case MODEL_PAYLOAD_GATE_FALL:
            vertical_speed = &model->gate_fall.physics_vertical_speed;
            pitch_speed = &model->gate_fall.bounces;
            break;
        case MODEL_PAYLOAD_GATE_FRAGMENT:
            vertical_speed = &model->gate_fragment.physics_vertical_speed;
            pitch_speed = &model->gate_fragment.physics_spin;
            break;
        case MODEL_PAYLOAD_ROTATION:
            vertical_speed = &model->rotation.physics_vertical_speed;
            pitch_speed = &model->rotation.axis;
            break;
        case MODEL_PAYLOAD_BOUNCING_PART:
            vertical_speed = &model->bounce.vertical_speed;
            pitch_speed = &model->bounce.physics_spin;
            break;
        case MODEL_PAYLOAD_TRAFFIC:
            vertical_speed = &model->traffic.physics_vertical_speed;
            pitch_speed = &model->traffic.physics_spin;
            break;
        case MODEL_PAYLOAD_TANK:
            vertical_speed = &model->tank.physics_vertical_speed;
            pitch_speed = &model->tank.turret_yaw;
            break;
        case MODEL_PAYLOAD_COMPANION:
            vertical_speed = &model->companion.physics_vertical_speed;
            pitch_speed = &model->companion.physics_spin;
            break;
        case MODEL_PAYLOAD_WOBBLE:
            vertical_speed = &model->wobble.physics_vertical_speed;
            pitch_speed = &model->wobble.emit_ticks;
            break;
        case MODEL_PAYLOAD_WAVE:
            vertical_speed = &model->wave.physics_vertical_speed;
            pitch_speed = &model->wave.phase;
            break;
        default:
            /* TODO: Investigate an unpublished spinning-object motion owner */

            XASSERT(0);
    }
    val = ((uint16)*vertical_speed) - 2u;
    *vertical_speed = (sint16)val;
    model->pos.y.bits = model->pos.y.bits - ((sint32)(val << 16) >> 1);
    model->angles.pitch_bits = (uint16)(((uint16)model->angles.pitch) + (uint32)(*pitch_speed >> 1));
    if ((sint32)model->pos.y.bits > 0)
        model->kind = 0xffffffffu;
    return -1;
}

uint32 dynamic_objects_spawn_spinning(uint32 kind, uint32 x, uint32 y, uint32 z, uint32 angle)
{
    FUNCTION_MARKER(0x80031eb4u, "GAME.EXE");
    sint32 slot = dynamic_objects_allocate();
    if (slot < 0)
        return 0;
    angle = angle - 4096u + (random_next() & 8191u);
    MODEL_OBJECT *item = dynamic_objects + slot;
    item->kind = kind;
    if (kind == 302)
        item->payload_kind = MODEL_PAYLOAD_TRAIN;
    else if (kind == 896)
        item->payload_kind = MODEL_PAYLOAD_SUSPENDED;
    else if (kind - 114u < 4u)
        item->payload_kind = MODEL_PAYLOAD_WAVE;
    else if (kind == 125 || kind == 126)
        item->payload_kind = MODEL_PAYLOAD_PANEL;
    else if (kind - 129u < 3u || kind - 350u < 9u || kind - 758u < 12u)
        item->payload_kind = MODEL_PAYLOAD_SIDE_PANEL;
    else if (kind - 575u < 2u || kind - 862u < 5u || kind == 869 || kind - 881u < 5u)
        item->payload_kind = MODEL_PAYLOAD_SPRITE;
    else if (kind - 679u < 4u)
        item->payload_kind = MODEL_PAYLOAD_BALLISTIC;
    else if (kind - 675u < 3u)
        item->payload_kind = MODEL_PAYLOAD_DRIFT;
    else if (kind == 70 || kind == 71 || kind - 74u < 3u || kind == 124 || kind - 127u < 2u || kind - 348u < 2u)
        item->payload_kind = MODEL_PAYLOAD_SHAKE;
    else if (kind == 77)
        item->payload_kind = MODEL_PAYLOAD_GATE_FALL;
    else if (kind - 72u < 2u)
        item->payload_kind = MODEL_PAYLOAD_GATE_FRAGMENT;
    else if (kind == 696)
    {
        item->payload_kind = MODEL_PAYLOAD_ROTATION;
        item->rotation.linked = (WORLD_OBJECT_REFERENCE){0, 1};
    }
    else if (kind - 473u < 3u)
        item->payload_kind = MODEL_PAYLOAD_BOUNCING_DROP;
    else if (kind == 67 || kind - 85u < 4u || kind == 92 || kind - 99u < 2u)
        item->payload_kind = MODEL_PAYLOAD_BOUNCING_PART;
    else if (kind - 78u < 7u)
        item->payload_kind = MODEL_PAYLOAD_TRAFFIC;
    else if (kind == 776)
        item->payload_kind = MODEL_PAYLOAD_TANK;
    else if (kind == 17 || kind - 62u < 3u || kind == 66 || kind == 90 || kind - 94u < 3u || kind - 108u < 3u || kind - 720u < 3u || kind == 739 || kind == 731 || kind == 204 || kind == 693 || kind == 405 || kind == 137)
        item->payload_kind = MODEL_PAYLOAD_OSCILLATION;
    else if (kind == 65)
        item->payload_kind = MODEL_PAYLOAD_COMPANION;
    else if (kind == 433)
        item->payload_kind = MODEL_PAYLOAD_LAB_TANK;
    else if (kind - 295u < 7u || kind == 326 || kind == 367 || kind - 371u < 12u || kind - 409u < 24u || kind - 608u < 6u || kind - 642u < 5u || kind == 649 || kind - 655u < 2u || kind - 672u < 3u || kind - 718u < 2u || kind == 737 || kind == 748 || kind - 786u < 12u || kind - 804u < 8u)
        item->payload_kind = MODEL_PAYLOAD_WOBBLE;
    else if (kind - 587u < 10u || kind == 69 || kind == 105 || kind == 724 || kind == 448 || kind == 717 || kind - 269u < 2u || kind == 392 || kind == 359 || kind == 695 || kind - 708u < 3u || kind == 408 || kind - 572u < 3u || kind - DYNAMIC_ITEM_TYPE_FIRST < DYNAMIC_ITEM_TYPE_COUNT || kind == 713 || kind == 715 || kind - 735u < 2u || kind - 784u < 2u || kind - 828u < 4u)
        item->payload_kind = MODEL_PAYLOAD_PHYSICS;
    else if (kind - 781u < 2u)
        item->payload_kind = MODEL_PAYLOAD_ARMY_TRUCK;
    else if (kind - 460u < 4u)
        item->payload_kind = MODEL_PAYLOAD_AIR_BIKE;
    else if (kind == 160 || kind == 166 || kind == 168 || kind == 740 || kind == 749 || kind - 175u < 8u || kind == 754)
        item->payload_kind = MODEL_PAYLOAD_SCENE_VEHICLE;
    else if (kind == 341 || kind == 146)
        item->payload_kind = MODEL_PAYLOAD_SCENE_HELICOPTER;
    else if (kind == 268)
        item->payload_kind = MODEL_PAYLOAD_PATROL_HELICOPTER;
    else if (kind == 687)
        item->payload_kind = MODEL_PAYLOAD_COMBAT_HELICOPTER;
    item->pos.x.bits = x;
    item->pos.y.bits = y;
    item->pos.z.bits = z;
    item->angles.yaw_bits = (uint16)angle;
    item->angles.pitch_bits = 0;
    item->angles.roll_bits = 0;
    OBJECT_MOTION_VIEW motion;
    object_motion_bind(&motion, item);
    *motion.speed = 128;
    *motion.vertical_speed = 64;
    *motion.spin = 6528;
    return 6528;
}

uint32 object_update_rising_spr(MODEL_OBJECT *model)
{
    FUNCTION_MARKER(0x8002ac8cu, "GAME.EXE");
    /* TODO: Investigate scripted common fields outside both native object pools */
    XASSERT(model);
    model->angles.yaw_bits = camera.angles.yaw;
    /* TODO: Investigate a model sprite callback with a foreign native payload */
    XASSERT(!model || model->payload_kind == MODEL_PAYLOAD_SPRITE);
    MODEL_SPRITE *model_spr = model ? &model->spr : NULL;
    uint32 delta = (uint32)(sint32)model_spr->vertical_speed << 16;
    model->pos.y.bits -= delta;
    model_spr->vertical_speed = (uint16)((uint16)model_spr->vertical_speed + 1u);
    if (model->pos.y.val < -536870912)
        model->kind = 0xffffffffu;
    return 0xffffffffu;
}

uint32 object_update_oscillating_spr(MODEL_OBJECT *model)
{
    FUNCTION_MARKER(0x8002b1e4u, "GAME.EXE");
    uint32 phase, delta, x, result;
    /* TODO: Investigate a model sprite callback with a foreign native payload */
    XASSERT(!model || model->payload_kind == MODEL_PAYLOAD_SPRITE);
    MODEL_SPRITE *model_spr = model ? &model->spr : NULL;
    /* TODO: Investigate a sprite age read outside both native model pools */
    XASSERT(model_spr);
    uint16 age = (uint16)model_spr->frame + 1u;
    model_spr->frame = age;
    if ((sint16)age == 40)
    {
        /* TODO: Investigate common fields outside both native object pools */
        XASSERT(model);
        model->kind = 0xffffffffu;
    }
    phase = (uint32)(sint32)model_spr->frame << 3;
    /* TODO: Investigate an expired or corrupt age outside the original sine table */
    XASSERT((sint32)phase >= 0 && phase < MATH_SIN_COUNT);
    delta = (uint32)(sint32)sin_table[phase] << 4;
    /* TODO: Investigate an object operation without a native position */
    XASSERT(model);
    model->pos.y.bits = model->pos.y.bits - delta;
    phase <<= 3;
    if ((sint32)phase >= 513)
        phase = 512;
    delta = (uint32)(sint32)sin_table[MATH_QUARTER_TURN + phase] << 6;
    x = model->pos.x.bits;
    if ((sint32)x > 0)
    {
        delta = (uint32)(-(sint64)(delta));
        x = model->pos.x.bits;
    }
    result = x + delta;
    model->pos.x.bits = result;
    return result;
}

uint32 object_update_orbit(MODEL_OBJECT *model)
{
    FUNCTION_MARKER(0x8004348cu, "GAME.EXE");
    /* TODO: Investigate transport common fields outside both native object pools */
    XASSERT(model);
    uint32 angle = (uint16)model->angles.yaw, delta, result;
    delta = (uint32)(sint32)sin_table[angle >> 5] << 8;
    /* TODO: Investigate a transport callback without a native position */
    XASSERT(model);
    model->pos.x.bits += delta;
    angle = ((model->angles.yaw >> 5) + 512u) & 2047u;
    delta = (uint32)(sint32)sin_table[angle] << 8;
    model->pos.z.bits -= delta;
    result = (uint16)model->angles.yaw + 32u;
    model->angles.yaw_bits = (uint16)result;
    return result;
}

uint32 object_update_wall_fragment(MODEL_OBJECT *model)
{
    FUNCTION_MARKER(0x80043680u, "GAME.EXE");
    uint32 offset, result;
    FIXED_VEC3 point;
    sint32 height = 0, angle;
    /* TODO: Investigate a falling debris callback with a foreign native payload */
    XASSERT(model && model->payload_kind == MODEL_PAYLOAD_SIDE_PANEL);
    FALLING_PANEL *panel = &model->panel;
    if (!panel->initialized)
    {
        panel->initialized = (uint16)(1);
        panel->roll_step = (uint16)((random_next() & 4095u) + 135u);
        panel->pitch_step = (uint16)((random_next() & 4095u) + 135u);
        /* TODO: Investigate transport common fields outside both native object pools */
        XASSERT(model);
        model->angles.yaw_bits = (uint16)((uint16)model->angles.yaw - 2048u);
        result = random_next() & 4095u;
        model->angles.yaw_bits = (uint16)((uint16)model->angles.yaw + result);
        panel->vertical_speed = (uint16)((random_next() & 15u) + 32u);
        panel->speed = (uint16)((random_next() & 63u) + 64u);
    }
    /* TODO: Investigate transport common fields outside both native object pools */
    XASSERT(model);
    offset = ((model->angles.yaw >> 5) + 512) & 2047u;
    /* TODO: Investigate a transport callback without a native position */
    XASSERT(model);
    if (model->pos.x.val < 0)
        offset ^= 1024u;
    offset *= 2u;
    model->pos.x.bits += (uint32)((panel->speed * sin_table[offset >> 1]) >> 1);
    model->pos.z.bits -= (uint32)((panel->speed * sin_table[MATH_QUARTER_TURN + (offset >> 1)]) >> 1);
    panel->speed = (uint16)((uint16)panel->speed - 2u);
    if (panel->speed < 0)
        panel->speed = (uint16)(0);
    model->pos.y.bits -= (uint32)(sint32)panel->vertical_speed << 15;
    panel->vertical_speed = (uint16)((uint16)panel->vertical_speed - 8u);
    transform_push();
    transform_identity();
    transform_pitch(32768);
    transform_yaw(model->angles.yaw);
    transform_pitch(model->angles.pitch);
    transform_roll(model->angles.roll);
    transform_point(0, 0xffa40000u, 0x00600000u, point.bits);
    if (height < point.y.val)
        height = point.y.val;
    transform_point(0, 0x005c0000u, 0x00600000u, point.bits);
    if (height < point.y.val)
        height = point.y.val;
    transform_point(0, 0xffa40000u, 0xffa00000u, point.bits);
    if (height < point.y.val)
        height = point.y.val;
    transform_point(0, 0x005c0000u, 0xffa00000u, point.bits);
    if (height < point.y.val)
        height = point.y.val;
    transform_pop();
    if (model->pos.y.val >= (sint32)((uint32)(-(sint64)(height))))
    {
        model->pos.y.bits = (uint32)(-(sint64)(height));
        panel->vertical_speed = (uint16)(0);
        panel->roll_step = (uint16)(0);
        panel->pitch_step = (uint16)(0);
        angle = (sint16)(16384u - (uint16)model->angles.roll);
        model->angles.roll_bits = (uint16)((uint16)model->angles.roll + (angle >> 1));
        angle = model->angles.pitch;
        model->angles.pitch_bits = (uint16)(angle + ((-angle) >> 1));
        /* TODO: Review the original bounce calculation after clearing vertical speed */
        panel->vertical_speed = (uint16)((-panel->vertical_speed) >> 1);
    }
    model->angles.roll_bits = (uint16)((uint16)model->angles.roll + (uint16)panel->roll_step);
    result = ((uint16)model->angles.pitch) + (uint16)panel->pitch_step;
    model->angles.pitch_bits = (uint16)result;
    return result;
}
