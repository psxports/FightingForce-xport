#include <string.h>
#include <stdio.h>

#include "game.h"

WORLD_OBJECTS world_objects;

static const sint16 traffic_wrap_kinds[16] = {78, 79, 80, 81, 82, 84, 83, 82, 79, 84, 81, 83, 78, 80, 84, 82};

typedef struct WORLD_OBJECT_TYPE_GROUP
{
    const sint16 *model;
    uint32 count;
} WORLD_OBJECT_TYPE_GROUP;

static const sint16 types_stage0[56] = {56, 61, 614, 615, 618, 619, 620, 621, 623, 622, 0, 0, 56, 625, 624, 464, 464, 464, 70, 626, 627, 526, 630, 631, 632, 55, 633, 634, 635, 636, 571, 637, 638, 0, 639, 640, 633, 0, 60, 641, 94, 920, 499, 506, 477, 478, 108, 108, 113, 114, 115, 116, 117, 598, 608, -1};
static const sint16 types_stage1[20] = {119, 465, 465, 666, 668, 524, 122, 466, 642, 647, 649, 522, 584, 655, 121, 123, 127, 124, -1, 0};
static const sint16 types_stage2[10] = {133, 663, 585, 522, 581, 55, 135, 132, -1, 0};
static const sint16 types_stage3[12] = {136, 465, 465, 55, 139, 671, 672, 585, 137, 138, -1, 0};
static const sint16 types_stage4[8] = {141, 143, 145, 55, 678, 146, 671, -1};
static const sint16 types_stage5[32] = {156, 157, 154, 153, 155, 149, 150, 158, 159, 160, 175, 161, 162, 164, 165, 166, 167, 535, 168, 169, 170, 171, 172, 536, 537, 173, 687, 55, 536, 536, -1, 0};
static const sint16 types_stage6[40] = {183, 184, 185, 186, 187, 188, 189, 190, 191, 192, 193, 194, 195, 196, 197, 198, 199, 200, 201, 554, 555, 556, 202, 203, 204, 205, 688, 689, 690, 691, 692, 206, 693, 528, 516, 517, 518, 519, -1, 0};
static const sint16 types_stage7[62] = {207, 208, 209, 210, 211, 215, 217, 223, 221, 231, 239, 227, 229, 241, 233, 235, 237, 694, 524, 695, 696, 697, 718, 698, 699, 700, 701, 703, 704, 705, 706, 707, 708, 709, 710, 711, 697, 712, 713, 720, 714, 715, 225, 245, 246, 219, 467, 468, 94, 247, 248, 469, 470, 480, 243, 244, 213, 716, 214, 482, 702, -1};
static const sint16 types_stage8[30] = {249, 250, 251, 252, 253, 254, 255, 256, 250, 257, 258, 259, 260, 261, 262, 249, 250, 263, 725, 726, 538, 539, 510, 727, 728, 729, 730, 731, 608, -1};
static const sint16 types_stage9[30] = {249, 250, 251, 252, 253, 254, 255, 256, 250, 257, 258, 259, 260, 261, 262, 249, 250, 263, 725, 726, 538, 539, 510, 727, 728, 729, 730, 731, 608, -1};
static const sint16 types_stage10[24] = {264, 265, 266, 732, 733, 734, 95, 55, 543, 540, 557, 559, 735, 736, 269, 267, 737, 560, 268, 482, 495, 528, 608, -1};
static const sint16 types_stage11[42] = {271, 272, 273, 274, 275, 739, 748, 541, 542, 740, 557, 558, 559, 560, 94, 741, 55, 55, 744, 543, 745, 746, 747, 276, 277, 749, 750, 751, 752, 753, 754, 755, 756, 757, 55, 55, 55, 55, 608, 609, -1, 0};
static const sint16 types_stage12[36] = {278, 279, 280, 302, 281, 282, 283, 284, 305, 285, 642, 295, 286, 545, 546, 549, 550, 287, 288, 289, 297, 299, 561, 55, 290, 291, 292, 293, 294, 547, 551, 552, 548, 608, -1, 0};
static const sint16 types_stage14[16] = {307, 309, 311, 313, 545, 546, 549, 550, 326, 608, 612, 610, 315, 316, -1, 0};
static const sint16 types_stage15[40] = {327, 328, 329, 330, 70, 55, 333, 334, 335, 336, 339, 331, 337, 338, 340, 780, 544, 770, 771, 772, 773, 774, 775, 784, 776, 341, 349, 778, 342, 343, 344, 345, 346, 347, 359, 781, 360, 332, 608, -1};
static const sint16 types_stage16[30] = {361, 362, 363, 364, 367, 544, 553, 371, 368, 369, 370, 373, 375, 377, 379, 383, 384, 385, 55, 381, 388, 0, 389, 390, 391, 627, 366, 608, -1, 0};
static const sint16 types_stage17[24] = {395, 396, 397, 798, 789, 792, 786, 804, 800, 801, 911, 802, 803, 795, 912, 806, 913, 914, 808, 810, 812, 471, 608, -1};
static const sint16 types_stage18[22] = {398, 55, 815, 610, 544, 505, 817, 818, 828, 55, 55, 608, 822, 823, 824, 825, 826, 55, 830, 341, 400, -1};
static const sint16 types_stage19[12] = {365, 367, 55, 553, 543, 383, 381, 55, 381, 608, -1, 0};
static const sint16 types_stage22[26] = {401, 402, 403, 562, 407, 408, 411, 413, 415, 417, 419, 409, 425, 427, 429, 431, 421, 423, 404, 405, 608, 612, 611, 406, -1, 0};
static const sint16 types_stage23[8] = {436, 437, 438, 439, 440, 441, 442, -1};
static const sint16 types_stage24[8] = {443, 608, 610, 610, 609, 611, -1, 0};
static const sint16 types_sequence1[16] = {156, 157, 154, 153, 149, 150, 841, 842, 844, 843, 845, 152, 151, -1, 445, -1};
static const sint16 types_sequence4[2] = {445, -1};
static const sint16 types_sequence6[4] = {444, 447, -1, 0};
/* Final four sequence7 backing values also supply linked-object yaw */
static const sint16 types_sequence7[6] = {451, -1, -16384, 16384, -16384, 16384};
static const sint16 types_frontend[12] = {454, 455, 456, 55, 55, 55, 55, 833, 835, 836, -1, 0};

static const WORLD_OBJECT_TYPE_GROUP world_object_type_defs[WORLD_OBJECT_TYPES_COUNT] = {
    {NULL, 0},
    {types_stage0, sizeof(types_stage0) / sizeof(*types_stage0)},
    {types_stage1, sizeof(types_stage1) / sizeof(*types_stage1)},
    {types_stage2, sizeof(types_stage2) / sizeof(*types_stage2)},
    {types_stage3, sizeof(types_stage3) / sizeof(*types_stage3)},
    {types_stage4, sizeof(types_stage4) / sizeof(*types_stage4)},
    {types_stage5, sizeof(types_stage5) / sizeof(*types_stage5)},
    {types_stage6, sizeof(types_stage6) / sizeof(*types_stage6)},
    {types_stage7, sizeof(types_stage7) / sizeof(*types_stage7)},
    {types_stage8, sizeof(types_stage8) / sizeof(*types_stage8)},
    {types_stage9, sizeof(types_stage9) / sizeof(*types_stage9)},
    {types_stage10, sizeof(types_stage10) / sizeof(*types_stage10)},
    {types_stage11, sizeof(types_stage11) / sizeof(*types_stage11)},
    {types_stage12, sizeof(types_stage12) / sizeof(*types_stage12)},
    {types_stage14, sizeof(types_stage14) / sizeof(*types_stage14)},
    {types_stage15, sizeof(types_stage15) / sizeof(*types_stage15)},
    {types_stage16, sizeof(types_stage16) / sizeof(*types_stage16)},
    {types_stage17, sizeof(types_stage17) / sizeof(*types_stage17)},
    {types_stage18, sizeof(types_stage18) / sizeof(*types_stage18)},
    {types_stage19, sizeof(types_stage19) / sizeof(*types_stage19)},
    {types_stage22, sizeof(types_stage22) / sizeof(*types_stage22)},
    {types_stage23, sizeof(types_stage23) / sizeof(*types_stage23)},
    {types_stage24, sizeof(types_stage24) / sizeof(*types_stage24)},
    {types_sequence1, sizeof(types_sequence1) / sizeof(*types_sequence1)},
    {types_sequence4, sizeof(types_sequence4) / sizeof(*types_sequence4)},
    {types_sequence6, sizeof(types_sequence6) / sizeof(*types_sequence6)},
    {types_sequence7, sizeof(types_sequence7) / sizeof(*types_sequence7)},
    {types_frontend, sizeof(types_frontend) / sizeof(*types_frontend)},
};

WORLD_OBJECT_TYPE_GROUP_ID world_object_type_grps[WORLD_OBJECT_STAGE_COUNT];
WORLD_OBJECT_TYPE_GROUP_ID world_object_active_types;
sint16 world_object_height[MODEL_TYPE_COUNT];
WORLD_REGION_KIND world_region_kinds[WORLD_REGION_KIND_COUNT];

/* Big-endian signed coordinates in the variable-length MPD byte stream */
static sint32 read_coord(const uint8 *data)
{
    sint32 high = (sint8)data[0];
    uint32 low = data[1];
    return high * 256 + (sint32)low;
}

GDB_CALL uint32 world_objects_expand(uint32 next_idx, uint32 type, uint32 x, uint32 z)
{
    FUNCTION_MARKER(0x80034244, "GAME.EXE");
    uint32 idx = next_idx, i, newtype = 0, angle = 0;
    MODEL_OBJECT *item;
    if (type == 920)
    {
        static const uint32 zs[3] = {0xe9a00000, 0xe7200000, 0xe4900000};
        idx--;
        for (i = 0; i < 3; i++, idx++)
        {
            /* TODO: Investigate construction beyond the native world pool */
            if (idx >= WORLD_OBJECT_COUNT)
            {
                DIAGNOSTIC(fprintf(stderr, "Invalid world construction slot %u kind %u in %s stage %u tick %u\n", idx, type, __FUNCTION__, game_session.stage, game_session.tick));
                diagnostic_stop(__FUNCTION__, __FILE__, __LINE__);
            }
            item = world_objects.object + idx;
            item->angles.yaw_bits = 0xc000;
            item->pos.x.bits = 0x50000000;
            item->pos.z.bits = zs[i];
            item->kind = 79;
            item->payload_kind = MODEL_PAYLOAD_TRAFFIC;
            item->traffic = (TRAFFIC_VEHICLE){0};
        }
        return 2;
    }
    if (type - 94u < 2)
    {
        newtype = 93;
        angle = 0x4900;
    }
    else if (type - 108u < 2)
    {
        newtype = 93;
        angle = 0xa000;
    }
    else if (type == 720)
    {
        newtype = 723;
        angle = 0xe000;
    }
    else if (type == 62)
    {
        newtype = 65;
        angle = 0xbc00;
    }
    if (newtype)
    {
        /* TODO: Investigate construction beyond the native world pool */
        if (idx >= WORLD_OBJECT_COUNT)
        {
            DIAGNOSTIC(fprintf(stderr, "Invalid world construction slot %u kind %u in %s stage %u tick %u\n", idx, type, __FUNCTION__, game_session.stage, game_session.tick));
            diagnostic_stop(__FUNCTION__, __FILE__, __LINE__);
        }
        item = world_objects.object + idx;
        item->pos.x.bits = x;
        item->pos.z.bits = z;
        item->kind = newtype;
        if (newtype == 65)
        {
            item->payload_kind = MODEL_PAYLOAD_COMPANION;
            item->companion = (COMPANION_MODEL){0};
        }
        item->angles.yaw_bits = (uint16)angle;
        uint32 prev_idx = idx - 1;
        /* TODO: Investigate construction beyond the native world pool */
        if (prev_idx >= WORLD_OBJECT_COUNT)
        {
            DIAGNOSTIC(fprintf(stderr, "Invalid world construction slot %u kind %u in %s stage %u tick %u\n", prev_idx, type, __FUNCTION__, game_session.stage, game_session.tick));
            diagnostic_stop(__FUNCTION__, __FILE__, __LINE__);
        }
        MODEL_OBJECT *prev = world_objects.object + prev_idx;
        prev->angles.yaw_bits = (uint16)angle;
        return 1;
    }
    if (type == 133)
    {
        world_objects.linked_first = world_objects.count - 1;
        for (i = 0; i < 7; i++, idx++)
        {
            /* TODO: Investigate construction beyond the native world pool */
            if (idx >= WORLD_OBJECT_COUNT)
            {
                DIAGNOSTIC(fprintf(stderr, "Invalid world construction slot %u kind %u in %s stage %u tick %u\n", idx, type, __FUNCTION__, game_session.stage, game_session.tick));
                diagnostic_stop(__FUNCTION__, __FILE__, __LINE__);
            }
            item = world_objects.object + idx;
            item->pos.x.bits = x;
            item->pos.z.bits = z;
            item->kind = i < 3 ? 133 : 134;
        }
        return 7;
    }
    if (type == 663)
    {
        /* TODO: Investigate construction beyond the native world pool */
        if (idx >= WORLD_OBJECT_COUNT)
        {
            DIAGNOSTIC(fprintf(stderr, "Invalid world construction slot %u kind %u in %s stage %u tick %u\n", idx, type, __FUNCTION__, game_session.stage, game_session.tick));
            diagnostic_stop(__FUNCTION__, __FILE__, __LINE__);
        }
        item = world_objects.object + idx;
        item->pos.x.bits = 0u - x;
        item->pos.z.bits = z;
        item->angles.yaw_bits = 0x8000;
        item->kind = 663;
        return 1;
    }
    if (type == 211)
        newtype = 212;
    if (type == 776)
    {
        idx--;
        /* TODO: Investigate construction beyond the native world pool */
        if (idx >= WORLD_OBJECT_COUNT)
        {
            DIAGNOSTIC(fprintf(stderr, "Invalid world construction slot %u kind %u in %s stage %u tick %u\n", idx, type, __FUNCTION__, game_session.stage, game_session.tick));
            diagnostic_stop(__FUNCTION__, __FILE__, __LINE__);
        }
        item = world_objects.object + idx;
        item->angles.yaw_bits = 0xc000;
    }
    if (type == 784)
        newtype = 783;
    if (type - 911u < 4)
    {
        idx--;
        /* TODO: Investigate construction beyond the native world pool */
        if (idx >= WORLD_OBJECT_COUNT)
        {
            DIAGNOSTIC(fprintf(stderr, "Invalid world construction slot %u kind %u in %s stage %u tick %u\n", idx, type, __FUNCTION__, game_session.stage, game_session.tick));
            diagnostic_stop(__FUNCTION__, __FILE__, __LINE__);
        }
        item = world_objects.object + idx;
        item->kind = 813;
        item->angles.yaw_bits = types_sequence7[2 + type - 911];
    }
    if (type == 398)
        newtype = 399;
    if (newtype)
    {
        /* TODO: Investigate construction beyond the native world pool */
        if (idx >= WORLD_OBJECT_COUNT)
        {
            DIAGNOSTIC(fprintf(stderr, "Invalid world construction slot %u kind %u in %s stage %u tick %u\n", idx, type, __FUNCTION__, game_session.stage, game_session.tick));
            diagnostic_stop(__FUNCTION__, __FILE__, __LINE__);
        }
        item = world_objects.object + idx;
        item->pos.x.bits = x;
        item->pos.z.bits = z;
        item->kind = newtype;
        return 1;
    }
    if (type == 429 || type == 431)
    {
        /* TODO: Investigate construction beyond the native world pool */
        if (idx >= WORLD_OBJECT_COUNT)
        {
            DIAGNOSTIC(fprintf(stderr, "Invalid world construction slot %u kind %u in %s stage %u tick %u\n", idx, type, __FUNCTION__, game_session.stage, game_session.tick));
            diagnostic_stop(__FUNCTION__, __FILE__, __LINE__);
        }
        item = world_objects.object + idx;
        item->pos.x.bits = x;
        item->pos.z.bits = z;
        item->kind = 433;
        LAB_TANK_OBJECT tank;
        if (item->payload_kind == MODEL_PAYLOAD_UNMIGRATED)
            tank = (LAB_TANK_OBJECT){.flags = item->pending_motion.roll_speed, .speed = item->pending_motion.speed, .pitch_speed = item->pending_motion.pitch_speed, .voice_plus_one[1] = item->pending_motion.phase, .voice_plus_one[0] = item->pending_motion.voice_plus_one, .vertical_speed = item->pending_motion.sound_clock, .physics_obstacle_flags = item->pending_motion.physics_obstacle_flags};
        else if (item->payload_kind == MODEL_PAYLOAD_LAB_TANK)
            tank = item->lab_tank;
        else
        {
            /* TODO: Investigate reusing a concrete foreign payload as a lab tank */
            DIAGNOSTIC(fprintf(stderr, "Foreign lab tank construction slot %u payload %u stage %u tick %u\n", idx, item->payload_kind, game_session.stage, game_session.tick));
            diagnostic_stop(__FUNCTION__, __FILE__, __LINE__);
        }
        item->payload_kind = MODEL_PAYLOAD_LAB_TANK;
        item->lab_tank = tank;
        item->lab_tank.pitch_speed = 0;
        lab_tank_level = 624;
        return 1;
    }
    if (type == 137)
    {
        uint32 prev_idx = idx - 1;
        /* TODO: Investigate construction beyond the native world pool */
        if (prev_idx >= WORLD_OBJECT_COUNT)
        {
            DIAGNOSTIC(fprintf(stderr, "Invalid world construction slot %u kind %u in %s stage %u tick %u\n", prev_idx, type, __FUNCTION__, game_session.stage, game_session.tick));
            diagnostic_stop(__FUNCTION__, __FILE__, __LINE__);
        }
        world_objects.tracked[6] = (WORLD_OBJECT_REFERENCE){prev_idx, 1};
    }
    if (type == 138)
    {
        uint32 prev_idx = idx - 1;
        /* TODO: Investigate construction beyond the native world pool */
        if (prev_idx >= WORLD_OBJECT_COUNT)
        {
            DIAGNOSTIC(fprintf(stderr, "Invalid world construction slot %u kind %u in %s stage %u tick %u\n", prev_idx, type, __FUNCTION__, game_session.stage, game_session.tick));
            diagnostic_stop(__FUNCTION__, __FILE__, __LINE__);
        }
        world_objects.tracked[7] = (WORLD_OBJECT_REFERENCE){prev_idx, 1};
    }
    if (type == 341)
    {
        uint32 prev_idx = idx - 1;
        /* TODO: Investigate construction beyond the native world pool */
        if (prev_idx >= WORLD_OBJECT_COUNT)
        {
            DIAGNOSTIC(fprintf(stderr, "Invalid world construction slot %u kind %u in %s stage %u tick %u\n", prev_idx, type, __FUNCTION__, game_session.stage, game_session.tick));
            diagnostic_stop(__FUNCTION__, __FILE__, __LINE__);
        }
        level_script_scene.object = (SCENE_OBJECT_REFERENCE){prev_idx, SCENE_OBJECT_WORLD};
    }
    return 0;
}

GDB_CALL uint32 world_objects_adjust(uint32 next_idx, uint32 type, uint32 x, uint32 z)
{
    FUNCTION_MARKER(0x8003EECC, "GAME.EXE");
    uint32 idx = next_idx - 1, i;
    MODEL_OBJECT *item;
    if (type == 268)
    {
        for (i = 0; i < 3; i++, idx++)
        {
            /* TODO: Investigate construction beyond the native world pool */
            if (idx >= WORLD_OBJECT_COUNT)
            {
                DIAGNOSTIC(fprintf(stderr, "Invalid world construction slot %u kind %u in %s stage %u tick %u\n", idx, type, __FUNCTION__, game_session.stage, game_session.tick));
                diagnostic_stop(__FUNCTION__, __FILE__, __LINE__);
            }
            item = world_objects.object + idx;
            /* Expanded slots start in the original whole-map zeroed pool */
            if (item->payload_kind == MODEL_PAYLOAD_UNMIGRATED)
            {
                item->patrol = (PATROL_HELICOPTER){0};
                item->payload_kind = MODEL_PAYLOAD_PATROL_HELICOPTER;
            }
            /* TODO: Investigate expansion over a foreign native model owner */
            if (item->payload_kind != MODEL_PAYLOAD_PATROL_HELICOPTER)
            {
                DIAGNOSTIC(fprintf(stderr, "Invalid patrol expansion slot %u payload %u stage %u tick %u\n", idx, (uint32)item->payload_kind, game_session.stage, game_session.tick));
                diagnostic_stop(__FUNCTION__, __FILE__, __LINE__);
            }
            item->pos.x.bits = i == 2 ? 0xf3800000u : x;
            item->pos.z.bits = z;
            item->patrol.mode = (uint16)i;
            item->kind = 268;
        }
        return 2;
    }
    if (type == 359)
    {
        /* TODO: Investigate construction beyond the native world pool */
        if (idx >= WORLD_OBJECT_COUNT)
        {
            DIAGNOSTIC(fprintf(stderr, "Invalid world construction slot %u kind %u in %s stage %u tick %u\n", idx, type, __FUNCTION__, game_session.stage, game_session.tick));
            diagnostic_stop(__FUNCTION__, __FILE__, __LINE__);
        }
        item = world_objects.object + idx;
        item->pos.x.bits = 0x1f400000;
        item->pos.z.bits = 0x1c200000;
        return 0;
    }
    if (type == 781)
    {
        /* TODO: Investigate construction beyond the native world pool */
        if (idx >= WORLD_OBJECT_COUNT)
        {
            DIAGNOSTIC(fprintf(stderr, "Invalid world construction slot %u kind %u in %s stage %u tick %u\n", idx, type, __FUNCTION__, game_session.stage, game_session.tick));
            diagnostic_stop(__FUNCTION__, __FILE__, __LINE__);
        }
        item = world_objects.object + idx;
        /* TODO: Investigate army truck access without its native owner */
        if (!item || item->payload_kind != MODEL_PAYLOAD_ARMY_TRUCK)
        {
            DIAGNOSTIC(fprintf(stderr, "Invalid army truck payload object %08X callback %s stage %u tick %u\n", 0x800b89f0 + 36u * idx, __FUNCTION__, game_session.stage, game_session.tick));
            diagnostic_stop(__FUNCTION__, __FILE__, __LINE__);
        }
        item->angles.yaw_bits = 0xc000;
        world_objects.relative_scene = (WORLD_OBJECT_REFERENCE){idx, 1};
    }
    return 0;
}

GDB_CALL uint32 world_objects_init_special(uint32 next_idx, uint32 type)
{
    FUNCTION_MARKER(0x800406C4, "GAME.EXE");
    uint32 idx = next_idx - 1, angle_idx, coord;
    MODEL_OBJECT *item;
    sint32 sample;
    switch (type)
    {
        case 832:
        {
            /* TODO: Investigate construction beyond the native world pool */
            if (idx >= WORLD_OBJECT_COUNT)
            {
                DIAGNOSTIC(fprintf(stderr, "Invalid world construction slot %u kind %u in %s stage %u tick %u\n", idx, type, __FUNCTION__, game_session.stage, game_session.tick));
                diagnostic_stop(__FUNCTION__, __FILE__, __LINE__);
            }
            item = world_objects.object + idx;
            item->pos.x.bits = 0xf0000000;
            item->pos.y.bits = 0xdc000000;
            item->pos.z.bits = 0x10000000;
            item->angles.yaw_bits = 0x3000;
            item->angles.pitch_bits = 0;
            item->angles.roll_bits = 0;
            break;
        }
        case 833:
        {
            /* TODO: Investigate construction beyond the native world pool */
            if (idx >= WORLD_OBJECT_COUNT)
            {
                DIAGNOSTIC(fprintf(stderr, "Invalid world construction slot %u kind %u in %s stage %u tick %u\n", idx, type, __FUNCTION__, game_session.stage, game_session.tick));
                diagnostic_stop(__FUNCTION__, __FILE__, __LINE__);
            }
            item = world_objects.object + idx;
            item->pos.x.bits = 0x01000000;
            item->pos.y.bits = 0xef000000;
            item->pos.z.bits = 0;
            item->angles.yaw_bits = 0;
            item->angles.pitch_bits = 0;
            item->angles.roll_bits = 0;
            break;
        }
        case 835:
            /* TODO: Investigate construction beyond the native world pool */
            if (idx >= WORLD_OBJECT_COUNT)
            {
                DIAGNOSTIC(fprintf(stderr, "Invalid world construction slot %u kind %u in %s stage %u tick %u\n", idx, type, __FUNCTION__, game_session.stage, game_session.tick));
                diagnostic_stop(__FUNCTION__, __FILE__, __LINE__);
            }
            item = world_objects.object + idx;
            item->angles.yaw_bits = 0x2000;
            break;
        case 836:
            /* TODO: Investigate construction beyond the native world pool */
            if (idx >= WORLD_OBJECT_COUNT)
            {
                DIAGNOSTIC(fprintf(stderr, "Invalid world construction slot %u kind %u in %s stage %u tick %u\n", idx, type, __FUNCTION__, game_session.stage, game_session.tick));
                diagnostic_stop(__FUNCTION__, __FILE__, __LINE__);
            }
            item = world_objects.object + idx;
            item->angles.yaw_bits = 0xd000;
            break;
        case 739:
        {
            /* TODO: Investigate construction beyond the native world pool */
            if (idx >= WORLD_OBJECT_COUNT)
            {
                DIAGNOSTIC(fprintf(stderr, "Invalid world construction slot %u kind %u in %s stage %u tick %u\n", idx, type, __FUNCTION__, game_session.stage, game_session.tick));
                diagnostic_stop(__FUNCTION__, __FILE__, __LINE__);
            }
            item = world_objects.object + idx;
            item->angles.yaw_bits = 0xed00;
            sample = sin_table[360];
            coord = item->pos.x.bits;
            /* TODO: Investigate a path origin store with a foreign native owner */
            if (item->payload_kind != MODEL_PAYLOAD_OSCILLATION)
            {
                DIAGNOSTIC(fprintf(stderr, "Invalid moving emitter origin slot %u kind %u owner %u stage %u tick %u\n", idx, item->kind, (uint32)item->payload_kind, game_session.stage, game_session.tick));
                diagnostic_stop(__FUNCTION__, __FILE__, __LINE__);
            }
            item->oscillation.base_x = (uint16)((sint32)(coord - ((uint32)sample << 14)) >> 16);
            angle_idx = item->angles.yaw_bits >> 5;
            sample = sin_table[angle_idx];
            coord = item->pos.z.bits;
            item->oscillation.base_z = (uint16)((sint32)(coord - ((uint32)sample << 14)) >> 16);
            break;
        }
        case 405:
        case 754:
            /* TODO: Investigate construction beyond the native world pool */
            if (idx >= WORLD_OBJECT_COUNT)
            {
                DIAGNOSTIC(fprintf(stderr, "Invalid world construction slot %u kind %u in %s stage %u tick %u\n", idx, type, __FUNCTION__, game_session.stage, game_session.tick));
                diagnostic_stop(__FUNCTION__, __FILE__, __LINE__);
            }
            item = world_objects.object + idx;
            if (type == 405)
            {
                /* TODO: Investigate scripted motion with a foreign native owner */
                if (!item || item->payload_kind != MODEL_PAYLOAD_OSCILLATION)
                {
                    DIAGNOSTIC(fprintf(stderr, "Invalid scripted motion owner %p in %s stage %u tick %u\n", (void *)item, __FUNCTION__, game_session.stage, game_session.tick));
                    diagnostic_stop(__FUNCTION__, __FILE__, __LINE__);
                }
                item->oscillation.door_height = 0;
                item->oscillation.door_phase = 0;
            }
            else
            {
                /* TODO: Investigate scripted motion with a foreign native owner */
                if (!item || item->payload_kind != MODEL_PAYLOAD_SCENE_VEHICLE)
                {
                    DIAGNOSTIC(fprintf(stderr, "Invalid scripted motion owner %p in %s stage %u tick %u\n", (void *)item, __FUNCTION__, game_session.stage, game_session.tick));
                    diagnostic_stop(__FUNCTION__, __FILE__, __LINE__);
                }
                item->scene_vehicle.speed = 0;
                item->scene_vehicle.motion_active = 0;
            }
            break;
        case 168:
            /* TODO: Investigate construction beyond the native world pool */
            if (idx >= WORLD_OBJECT_COUNT)
            {
                DIAGNOSTIC(fprintf(stderr, "Invalid world construction slot %u kind %u in %s stage %u tick %u\n", idx, type, __FUNCTION__, game_session.stage, game_session.tick));
                diagnostic_stop(__FUNCTION__, __FILE__, __LINE__);
            }
            item = world_objects.object + idx;
            /* TODO: Investigate a scene vehicle with a foreign native owner */
            if (!item || item->payload_kind != MODEL_PAYLOAD_SCENE_VEHICLE)
            {
                DIAGNOSTIC(fprintf(stderr, "Invalid scene vehicle owner in %s stage %u tick %u\n", __FUNCTION__, game_session.stage, game_session.tick));
                diagnostic_stop(__FUNCTION__, __FILE__, __LINE__);
            }
            item->scene_vehicle.start_tick = -1;
            item->scene_vehicle.voice_plus_one[1] = -1;
            break;
        case 166:
            /* TODO: Investigate construction beyond the native world pool */
            if (idx >= WORLD_OBJECT_COUNT)
            {
                DIAGNOSTIC(fprintf(stderr, "Invalid world construction slot %u kind %u in %s stage %u tick %u\n", idx, type, __FUNCTION__, game_session.stage, game_session.tick));
                diagnostic_stop(__FUNCTION__, __FILE__, __LINE__);
            }
            item = world_objects.object + idx;
            item->angles.yaw_bits = 0x7c00;
            /* TODO: Investigate a scene vehicle with a foreign native owner */
            if (!item || item->payload_kind != MODEL_PAYLOAD_SCENE_VEHICLE)
            {
                DIAGNOSTIC(fprintf(stderr, "Invalid scene vehicle owner in %s stage %u tick %u\n", __FUNCTION__, game_session.stage, game_session.tick));
                diagnostic_stop(__FUNCTION__, __FILE__, __LINE__);
            }
            item->scene_vehicle.start_tick = -1;
            break;
        case 175:
            /* TODO: Investigate construction beyond the native world pool */
            if (idx >= WORLD_OBJECT_COUNT)
            {
                DIAGNOSTIC(fprintf(stderr, "Invalid world construction slot %u kind %u in %s stage %u tick %u\n", idx, type, __FUNCTION__, game_session.stage, game_session.tick));
                diagnostic_stop(__FUNCTION__, __FILE__, __LINE__);
            }
            item = world_objects.object + idx;
            item->angles.yaw_bits = 0xc000;
            break;
        case 160:
            /* TODO: Investigate construction beyond the native world pool */
            if (idx >= WORLD_OBJECT_COUNT)
            {
                DIAGNOSTIC(fprintf(stderr, "Invalid world construction slot %u kind %u in %s stage %u tick %u\n", idx, type, __FUNCTION__, game_session.stage, game_session.tick));
                diagnostic_stop(__FUNCTION__, __FILE__, __LINE__);
            }
            item = world_objects.object + idx;
            item->angles.yaw_bits = 0xc000;
            /* TODO: Investigate a scene vehicle with a foreign native owner */
            if (!item || item->payload_kind != MODEL_PAYLOAD_SCENE_VEHICLE)
            {
                DIAGNOSTIC(fprintf(stderr, "Invalid scene vehicle owner in %s stage %u tick %u\n", __FUNCTION__, game_session.stage, game_session.tick));
                diagnostic_stop(__FUNCTION__, __FILE__, __LINE__);
            }
            item->scene_vehicle.start_tick = -1;
            break;
        case 687:
        {
            /* TODO: Investigate construction beyond the native world pool */
            if (idx >= WORLD_OBJECT_COUNT)
            {
                DIAGNOSTIC(fprintf(stderr, "Invalid world construction slot %u kind %u in %s stage %u tick %u\n", idx, type, __FUNCTION__, game_session.stage, game_session.tick));
                diagnostic_stop(__FUNCTION__, __FILE__, __LINE__);
            }
            item = world_objects.object + idx;
            item->pos.z.bits = 0x40000000;
            /* TODO: Investigate combat helicopter initialization with a foreign owner */
            if (item->payload_kind != MODEL_PAYLOAD_COMBAT_HELICOPTER)
            {
                DIAGNOSTIC(fprintf(stderr, "Invalid combat helicopter init slot %u payload %u stage %u tick %u\n", idx, item->payload_kind, game_session.stage, game_session.tick));
                diagnostic_stop(__FUNCTION__, __FILE__, __LINE__);
            }
            item->helicopter.voice_plus_one[0] = 0;
            break;
        }
        case 199:
            /* TODO: Investigate construction beyond the native world pool */
            if (idx >= WORLD_OBJECT_COUNT)
            {
                DIAGNOSTIC(fprintf(stderr, "Invalid world construction slot %u kind %u in %s stage %u tick %u\n", idx, type, __FUNCTION__, game_session.stage, game_session.tick));
                diagnostic_stop(__FUNCTION__, __FILE__, __LINE__);
            }
            item = world_objects.object + idx;
            item->angles.yaw_bits = 0xf000;
            if (item->payload_kind != MODEL_PAYLOAD_UNMIGRATED)
            {
                /* TODO: Investigate latent speed initialization over a concrete owner */
                DIAGNOSTIC(fprintf(stderr, "Invalid pending world speed slot %u kind %u payload %u stage %u tick %u\n", idx, item->kind, item->payload_kind, game_session.stage, game_session.tick));
                diagnostic_stop(__FUNCTION__, __FILE__, __LINE__);
            }
            item->pending_motion.speed = 20;
            break;
        case 202:
            /* TODO: Investigate construction beyond the native world pool */
            if (idx >= WORLD_OBJECT_COUNT)
            {
                DIAGNOSTIC(fprintf(stderr, "Invalid world construction slot %u kind %u in %s stage %u tick %u\n", idx, type, __FUNCTION__, game_session.stage, game_session.tick));
                diagnostic_stop(__FUNCTION__, __FILE__, __LINE__);
            }
            item = world_objects.object + idx;
            item->angles.yaw_bits = 0xf000;
            break;
        case 205:
            /* TODO: Investigate construction beyond the native world pool */
            if (idx >= WORLD_OBJECT_COUNT)
            {
                DIAGNOSTIC(fprintf(stderr, "Invalid world construction slot %u kind %u in %s stage %u tick %u\n", idx, type, __FUNCTION__, game_session.stage, game_session.tick));
                diagnostic_stop(__FUNCTION__, __FILE__, __LINE__);
            }
            item = world_objects.object + idx;
            item->angles.yaw_bits = 0x1000;
            break;
        case 689:
        case 690:
            /* TODO: Investigate construction beyond the native world pool */
            if (idx >= WORLD_OBJECT_COUNT)
            {
                DIAGNOSTIC(fprintf(stderr, "Invalid world construction slot %u kind %u in %s stage %u tick %u\n", idx, type, __FUNCTION__, game_session.stage, game_session.tick));
                diagnostic_stop(__FUNCTION__, __FILE__, __LINE__);
            }
            item = world_objects.object + idx;
            item->angles.yaw_bits = 0x1800;
            break;
        case 516:
        case 517:
        case 518:
        {
            uint32 phase = random_next() & 31;
            /* TODO: Investigate construction beyond the native world pool */
            if (idx >= WORLD_OBJECT_COUNT)
            {
                DIAGNOSTIC(fprintf(stderr, "Invalid world construction slot %u kind %u in %s stage %u tick %u\n", idx, type, __FUNCTION__, game_session.stage, game_session.tick));
                diagnostic_stop(__FUNCTION__, __FILE__, __LINE__);
            }
            item = world_objects.object + idx;
            /* TODO: Investigate item initialization with a foreign native owner */
            if (item->payload_kind != MODEL_PAYLOAD_PHYSICS)
            {
                DIAGNOSTIC(fprintf(stderr, "Invalid item phase owner slot %u kind %u requested %u owner %u stage %u tick %u\n", idx, item->kind, type, (uint32)item->payload_kind, game_session.stage, game_session.tick));
                diagnostic_stop(__FUNCTION__, __FILE__, __LINE__);
            }
            item->physics.spin = (uint16)phase;
            break;
        }
        case 696:
        {
            idx = next_idx;
            angle_idx = random_next() & 2047;
            sample = sin_table[angle_idx];
            uint32 prev_idx = idx - 1;
            /* TODO: Investigate construction beyond the native world pool */
            if (prev_idx >= WORLD_OBJECT_COUNT)
            {
                DIAGNOSTIC(fprintf(stderr, "Invalid world construction slot %u kind %u in %s stage %u tick %u\n", prev_idx, type, __FUNCTION__, game_session.stage, game_session.tick));
                diagnostic_stop(__FUNCTION__, __FILE__, __LINE__);
            }
            MODEL_OBJECT *prev = world_objects.object + prev_idx;
            coord = prev->pos.x.bits;
            /* TODO: Investigate construction beyond the native world pool */
            if (idx >= WORLD_OBJECT_COUNT)
            {
                DIAGNOSTIC(fprintf(stderr, "Invalid world construction slot %u kind %u in %s stage %u tick %u\n", idx, type, __FUNCTION__, game_session.stage, game_session.tick));
                diagnostic_stop(__FUNCTION__, __FILE__, __LINE__);
            }
            item = world_objects.object + idx;
            item->pos.x.bits = ((uint32)sample << 6) + coord;
            sample = sin_table[MATH_QUARTER_TURN + angle_idx];
            coord = prev->pos.z.bits;
            item->pos.z.bits = ((uint32)sample << 6) + coord;
            item->kind = 473;
            item->payload_kind = MODEL_PAYLOAD_BOUNCING_DROP;
            item->drop = (BOUNCING_DROP){0};
            item->pos.y.bits = 0xff700000;
            item->angles.yaw_bits = prev->angles.yaw_bits;
            item->angles.pitch_bits = prev->angles.pitch_bits;
            item->angles.roll_bits = prev->angles.roll_bits;
            item->drop.released = 0;
            /* The original modulo32 multiply reduces to4 * idx, then signed>>2 */
            /* TODO: Investigate construction of a linked child for a foreign parent */
            if (prev->payload_kind != MODEL_PAYLOAD_ROTATION)
            {
                DIAGNOSTIC(fprintf(stderr, "Invalid rotation parent slot %u kind %u payload %u stage %u tick %u\n", prev_idx, prev->kind, prev->payload_kind, game_session.stage, game_session.tick));
                diagnostic_stop(__FUNCTION__, __FILE__, __LINE__);
            }
            prev->rotation.linked = (WORLD_OBJECT_REFERENCE){idx, 1};
            return 1;
        }
        case 702:
            /* TODO: Investigate construction beyond the native world pool */
            if (idx >= WORLD_OBJECT_COUNT)
            {
                DIAGNOSTIC(fprintf(stderr, "Invalid world construction slot %u kind %u in %s stage %u tick %u\n", idx, type, __FUNCTION__, game_session.stage, game_session.tick));
                diagnostic_stop(__FUNCTION__, __FILE__, __LINE__);
            }
            item = world_objects.object + idx;
            item->kind = 701;
            item->angles.yaw_bits = 0x8000;
            break;
    }
    return 0;
}

/* MPD objects, pooled items and named gameplay regions */
GDB_CALL uint32 world_objects_load_map(RESOURCE_VIEW map)
{
    FUNCTION_MARKER(0x800173B0, "GAME.EXE");
    uint32 cursor = 8192, object_idx = 0, i, stage;
    COLLISION_REGION *region;
    uint8 tag;
    sint32 object_count;
    int more;
    world_objects.count = 0;
    world_objects.tracked_count = 0;
    memset(world_objects.object, 0, sizeof(world_objects.object));
    if (!map.data || cursor >= map.byte_len)
        diagnostic_stop(__FUNCTION__, __FILE__, __LINE__);
    tag = map.data[cursor];
    more = (sint8)tag != -1;
    while (more)
    {
        sint32 code = (sint8)tag;
        ++cursor;
        if (map.byte_len - cursor < 4)
            diagnostic_stop(__FUNCTION__, __FILE__, __LINE__);
        const WORLD_OBJECT_TYPE_GROUP *types;
        uint32 type, x, z, n;
        if (world_object_active_types < WORLD_OBJECT_TYPES_NONE || world_object_active_types >= WORLD_OBJECT_TYPES_COUNT)
            diagnostic_stop(__FUNCTION__, __FILE__, __LINE__);
        types = world_object_type_defs + world_object_active_types;
        if (code < 1 || (uint32)code > types->count)
            diagnostic_stop(__FUNCTION__, __FILE__, __LINE__);
        type = types->model[code - 1];
        x = 0u - ((uint32)read_coord(map.data + cursor) << 19);
        z = 0u - ((uint32)read_coord(map.data + cursor + 2) << 19);
        cursor += 4;
        if (type - 477u < 86 || type - 587u < 10)
        {
            sint32 slot = type - 477u < 86 ? dynamic_objects_spawn_item(type, x, 0, z, 0) : dynamic_objects_spawn(type, x, 0, z);
            uint16 angle = 2u * random_next();
            /* TODO: Investigate the original pre-pool write when MPD item allocation fails */
            if (slot < 0 || slot >= DYNAMIC_OBJECT_COUNT)
            {
                DIAGNOSTIC(fprintf(stderr, "Invalid MPD item allocation %d for kind %u at record %u stage %u\n", slot, type, cursor - 5u, game_session.stage));
                diagnostic_stop(__FUNCTION__, __FILE__, __LINE__);
            }
            dynamic_objects[slot].angles.yaw_bits = angle;
            goto next_mpd_object;
        }
        if (object_idx > WORLD_OBJECT_COUNT - 8)
        {
            DIAGNOSTIC(fprintf(stderr, "MPD world reserve exceeded slot %u kind %u record %u stage %u\n", object_idx, type, cursor - 5, game_session.stage));
            diagnostic_stop(__FUNCTION__, __FILE__, __LINE__);
        }
        MODEL_OBJECT *item = world_objects.object + object_idx;
        item->pos.x.bits = x;
        item->pos.z.bits = z;
        item->kind = type;
        if (type == 302)
            item->payload_kind = MODEL_PAYLOAD_TRAIN;
        else if (type == 896)
            item->payload_kind = MODEL_PAYLOAD_SUSPENDED;
        else if (type - 114u < 4u)
            item->payload_kind = MODEL_PAYLOAD_WAVE;
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
        world_objects.count = world_objects.count + 1;
        if (type - 464u < 8)
        {
            n = world_objects.tracked_count;
            world_objects.tracked_count = n + 1;
            /* TODO: Investigate original tracked-list overflow before adjacent state is written */
            if (n >= WORLD_TRACKED_COUNT)
            {
                DIAGNOSTIC(fprintf(stderr, "Tracked world list overflow %u for kind %u at record %u\n", n, type, cursor - 5u));
                diagnostic_stop(__FUNCTION__, __FILE__, __LINE__);
            }
            world_objects.tracked[n] = (WORLD_OBJECT_REFERENCE){object_idx, 1};
        }
        if (type == 466)
        {
            world_objects.tracked_count = world_objects.tracked_count - 1;
            world_objects.tracked[7] = (WORLD_OBJECT_REFERENCE){object_idx, 1};
        }
        object_idx++;
        n = world_objects_expand(object_idx, type, x, z);
        object_idx += n;
        world_objects.count = world_objects.count + n;
        n = world_objects_adjust(object_idx, type, x, z);
        object_idx += n;
        world_objects.count = world_objects.count + n;
        n = world_objects_init_special(object_idx, type);
        object_idx += n;
        world_objects.count = world_objects.count + n;
    next_mpd_object:
        if (cursor >= map.byte_len)
            diagnostic_stop(__FUNCTION__, __FILE__, __LINE__);
        tag = map.data[cursor];
        more = (sint8)map.data[cursor] != -1;
    }
    stage = game_session.stage;
    if (stage == 1)
    {
        uint32 n;
        WORLD_OBJECT_REFERENCE linked;
        /* TODO: Investigate MPD world slots outside the native pool */
        if (object_idx >= WORLD_OBJECT_COUNT)
        {
            DIAGNOSTIC(fprintf(stderr, "Invalid MPD world slot %u cursor %u count %u stage %u tick %u\n", object_idx, cursor, world_objects.count, game_session.stage, game_session.tick));
            diagnostic_stop(__FUNCTION__, __FILE__, __LINE__);
        }
        MODEL_OBJECT *item = world_objects.object + object_idx;
        item->pos.z.bits = 0x00180000;
        item->kind = 123;
        world_objects.count = world_objects.count + 1;
        linked = world_objects.tracked[7];
        n = world_objects.tracked_count;
        world_objects.tracked_count = n + 1;
        /* TODO: Investigate original tracked-list overflow on the stage1 append */
        if (n >= WORLD_TRACKED_COUNT)
        {
            DIAGNOSTIC(fprintf(stderr, "Tracked world stage1 append overflow %u\n", n));
            diagnostic_stop(__FUNCTION__, __FILE__, __LINE__);
        }
        world_objects.tracked[n] = linked;
        stage = game_session.stage;
    }
    if (stage == 2)
    {
        EFFECT *marker = effect_alloc();
        /* TODO: Investigate the original unchecked marker allocation before a low-RAM write */
        if (!marker)
        {
            DIAGNOSTIC(fprintf(stderr, "Invalid lift marker allocation %p\n", (void *)marker));
            diagnostic_stop(__FUNCTION__, __FILE__, __LINE__);
        }
        marker->kind = 5;
        marker->payload_kind = EFFECT_PAYLOAD_PARTICLE;
    }
    object_count = (sint32)world_objects.count;
    if (object_count > 0)
        object_count = (sint32)world_objects.count;
    for (i = 0; (sint32)i < object_count; i++)
    {
        /* TODO: Investigate MPD world slots outside the native pool */
        if (i >= WORLD_OBJECT_COUNT)
        {
            DIAGNOSTIC(fprintf(stderr, "Invalid MPD world slot %u cursor %u count %u stage %u tick %u\n", i, cursor, world_objects.count, game_session.stage, game_session.tick));
            diagnostic_stop(__FUNCTION__, __FILE__, __LINE__);
        }
        MODEL_OBJECT *item = world_objects.object + i;
        uint32 type = item->kind;
        if (type - 473u >= 2 && type != 475)
        {
            if (type >= MODEL_TYPE_COUNT)
                diagnostic_stop(__FUNCTION__, __FILE__, __LINE__);
            item->pos.y.bits = (uint32)world_object_height[type] << 16;
        }
    }
    cursor++;
    region = col_regions.data;
    memset(col_regions.data, 0, sizeof(col_regions.data));
    col_regions.named[1] = 0;
    col_regions.named[0] = 0;
    col_regions.count = 0;
    if (cursor >= map.byte_len)
        diagnostic_stop(__FUNCTION__, __FILE__, __LINE__);
    tag = map.data[cursor];
    more = (sint8)tag != -1;
    while (more)
    {
        sint32 kind = (sint8)tag;
        sint32 mode, a, b, c, d;
        ++cursor;
        switch (kind)
        {
            case 14:
                col_regions.named[col_regions.named[0] ? 1 : 0] = region;
                kind = 0;
                break;
            case 15:
                col_regions.named[1] = region;
                kind = 17;
                break;
            case 16:
                col_regions.named[2] = region;
                kind = 0;
                break;
            case 18:
            case 31:
                col_regions.named[3] = region;
                kind = 17;
                break;
            case 19:
            case 23:
            case 26:
                col_regions.named[1] = region;
                kind = 0;
                break;
            case 20:
            case 29:
            case 30:
                col_regions.named[2] = region;
                kind = 17;
                break;
            case 21:
                col_regions.named[3] = region;
                kind = 0;
                break;
            case 22:
            case 32:
                col_regions.named[4] = region;
                kind = 17;
                break;
            case 27:
            case 33:
                col_regions.named[5] = region;
                kind = 0;
                break;
            case 28:
                col_regions.named[6] = region;
                kind = 17;
                break;
            case 34:
                col_regions.named[6] = region;
                kind = 0;
                break;
            case 35:
                col_regions.named[7] = region;
                kind = 0;
                break;
            case 37:
                if (col_regions.named[1])
                    col_regions.named[2] = region;
                else
                    col_regions.named[col_regions.named[0] ? 1 : 0] = region;
                kind = 0;
                break;
            case 40:
                col_regions.named[8] = region;
                kind = 0;
                break;
            case 41:
                col_regions.named[9] = region;
                kind = 0;
                break;
            case 42:
                col_regions.named[10] = region;
                kind = 0;
                break;
        }
        if (region >= col_regions.data + COLLISION_REGION_COUNT)
            diagnostic_stop(__FUNCTION__, __FILE__, __LINE__);
        region->trigger_kind = kind;
        if (kind < 0 || kind >= WORLD_REGION_KIND_COUNT)
            diagnostic_stop(__FUNCTION__, __FILE__, __LINE__);
        mode = world_region_kinds[kind].mode;
        region->shape = mode;
        region->col_kind = world_region_kinds[kind].flags;
        if (map.byte_len - cursor < 8)
            diagnostic_stop(__FUNCTION__, __FILE__, __LINE__);
        a = -8 * read_coord(map.data + cursor);
        b = -8 * read_coord(map.data + cursor + 2);
        c = -8 * read_coord(map.data + cursor + 4);
        d = -8 * read_coord(map.data + cursor + 6);
        cursor += 8;
        if (mode == 2)
        {
            sint32 cx = (c + a) >> 1, cz = (d + b) >> 1, dx = a - cx, dz = b - cz;
            c = dx < dz ? dz : dx;
            a = cx;
            b = cz;
        }
        else
        {
            object_count = (sint32)world_objects.count;
            if (object_count > 0)
                object_count = (sint32)world_objects.count;
            for (i = 0; (sint32)i < object_count; i++)
            {
                /* TODO: Investigate MPD world slots outside the native pool */
                if (i >= WORLD_OBJECT_COUNT)
                {
                    DIAGNOSTIC(fprintf(stderr, "Invalid MPD world slot %u cursor %u count %u stage %u tick %u\n", i, cursor, world_objects.count, game_session.stage, game_session.tick));
                    diagnostic_stop(__FUNCTION__, __FILE__, __LINE__);
                }
                const MODEL_OBJECT *item = world_objects.object + i;
                sint32 x = item->pos.x.whole, z = item->pos.z.whole;
                if (x >= c && z >= d && a >= x && b >= z)
                {
                    region->object = (WORLD_OBJECT_REFERENCE){i, 1};
                    break;
                }
            }
        }
        region->geometry.coord[0] = c;
        region->geometry.coord[1] = d;
        region->geometry.coord[2] = a;
        region->geometry.coord[3] = b;
        col_regions.count = col_regions.count + 1;
        if (cursor >= map.byte_len)
            diagnostic_stop(__FUNCTION__, __FILE__, __LINE__);
        tag = map.data[cursor];
        more = (sint8)map.data[cursor] != -1;
        ++region;
    }
    return 0xffffffffu;
}

GDB_CALL uint32 world_objects_bob(MODEL_OBJECT *item)
{
    FUNCTION_MARKER(0x8002e7f0u, "GAME.EXE");
    uint32 tick = game_session.tick, type, height, delta;
    /* TODO: Investigate common UPDATE fields outside both native object pools */
    if (!item)
    {
        DIAGNOSTIC(fprintf(stderr, "Invalid UPDATE model object %p in %s stage %u tick %u\n", (void *)item, __FUNCTION__, game_session.stage, game_session.tick));
        diagnostic_stop(__FUNCTION__, __FILE__, __LINE__);
    }
    item->angles.yaw_bits = (uint16)(2u * (uint16)sin_table[tick & 2047u]);
    type = item->kind;
    if (type >= MODEL_TYPE_COUNT)
        diagnostic_stop(__FUNCTION__, __FILE__, __LINE__);
    height = (uint32)world_object_height[type] << 16;
    delta = (uint32)(sint32)sin_table[((tick << 5) & 0xfe0u) >> 1] << 5;
    item->pos.y.bits = height + delta;
    return delta;
}

/* Preserve per-object transforms and fail on missing render callbacks */
GDB_CALL sint32 world_objects_draw(void)
{
    FUNCTION_MARKER(0x8005c17c, "GAME.EXE");
    sint32 i, count;
    uint32 type, x, y, z;
    const OBJECT_CALLBACK *target;
    transform_reset();
    count = (sint32)world_objects.count;
    if (count <= 0)
        return count;
    i = 0;
    do
    {
        /* TODO: Investigate world counts that exceed the object pool */
        if (i >= WORLD_OBJECT_COUNT)
        {
            DIAGNOSTIC(fprintf(stderr, "Invalid world draw slot %d count %d stage %u tick %u\n", i, count, game_session.stage, game_session.tick));
            diagnostic_stop(__FUNCTION__, __FILE__, __LINE__);
        }
        const MODEL_OBJECT *item = world_objects.object + i;
        type = item->kind;
        if (type != 55)
        {
            if (type >= MODEL_TYPE_COUNT)
                diagnostic_stop(__FUNCTION__, __FILE__, __LINE__);
            if (!model_refs[type].header)
                diagnostic_stop(__FUNCTION__, __FILE__, __LINE__); /* Original fatal loop */
            transform_push();
            x = item->pos.x.bits;
            x -= camera.pos.x.bits;
            y = item->pos.y.bits;
            y -= camera.pos.y.bits;
            z = item->pos.z.bits;
            z -= camera.pos.z.bits;
            transform_translate(x, y, z);
            transform_pitch(32768);
            target = object_callbacks[OBJECT_CALLBACK_WORLD_DRAW][type];
            object_call(target, world_objects.object + i, NULL);
            transform_pop();
        }
        i++;
        count = (sint32)world_objects.count;
    } while (i < count);
    return 0;
}

/* Roll a launched world object and rebuild its retained collision bounds */
GDB_CALL void world_objects_roll(MODEL_OBJECT *item)
{
    FUNCTION_MARKER(0x800469a4u, "GAME.EXE");
    COLLISION_REGION *region = col_regions.data;
    uint32 radius, x, z, result, coord, angle, step, kind;
    COLLISION_POINT point;
    uint32 count = col_regions.count, i = 0;
    sint32 dx, dz, object_x, object_z, region_x, region_z, speed, s, c, owner, angle_idx;
    result = count;
    for (; (sint32)i < (sint32)count; i++, ++region)
    {
        if (i >= COLLISION_REGION_COUNT)
            diagnostic_stop(__FUNCTION__, __FILE__, __LINE__);
        /* TODO: Investigate a corrupt native world reference before region matching */
        if (region->object.present > 1 || (region->object.present && region->object.idx >= WORLD_OBJECT_COUNT))
        {
            DIAGNOSTIC(fprintf(stderr, "Invalid vehicle region world slot %u present %u stage %u tick %u\n", region->object.idx, (uint32)region->object.present, game_session.stage, game_session.tick));
            diagnostic_stop(__FUNCTION__, __FILE__, __LINE__);
        }
        if (region->object.present && item == world_objects.object + region->object.idx)
            break;
    }
    if ((sint32)count > 0)
        result = col_regions.count;
    if (i == result)
        return;
    /* TODO: Investigate common UPDATE fields outside both native object pools */
    if (!item)
    {
        DIAGNOSTIC(fprintf(stderr, "Invalid UPDATE model object %p in %s stage %u tick %u\n", (void *)item, __FUNCTION__, game_session.stage, game_session.tick));
        diagnostic_stop(__FUNCTION__, __FILE__, __LINE__);
    }
    /* TODO: Investigate a rolling object with a foreign native payload */
    if (item->payload_kind != MODEL_PAYLOAD_PHYSICS)
    {
        DIAGNOSTIC(fprintf(stderr, "Invalid rolling payload object %p kind %u owner %u stage %u tick %u\n", (void *)item, item->kind, (uint32)item->payload_kind, game_session.stage, game_session.tick));
        diagnostic_stop(__FUNCTION__, __FILE__, __LINE__);
    }
    /* TODO: Investigate the original inactive-region read for a negative count */
    if ((sint32)count < 0 || i >= COLLISION_REGION_COUNT)
    {
        DIAGNOSTIC(fprintf(stderr, "Invalid rolling region slot %u count %d object %p stage %u tick %u\n", i, (sint32)count, (void *)item, game_session.stage, game_session.tick));
        diagnostic_stop(__FUNCTION__, __FILE__, __LINE__);
    }
    speed = item->physics.speed;
    if (speed <= 0)
        return;
    object_x = item->pos.x.whole;
    region_x = region->geometry.axis.min_x;
    dx = object_x - region_x;
    object_z = item->pos.z.whole;
    region_z = region->geometry.axis.min_z;
    dz = object_z - region_z;
    radius = (uint32)math_sqrt_odd((uint32)dx * (uint32)dx + (uint32)dz * (uint32)dz) << 1;
    angle_idx = item->physics.heading;
    /* TODO: Investigate rolling headings outside the original Q15 table */
    if (angle_idx < 0 || angle_idx >= MATH_SIN_COUNT)
    {
        DIAGNOSTIC(fprintf(stderr, "Invalid rolling sine index %d object %p stage %u tick %u\n", angle_idx, (void *)item, game_session.stage, game_session.tick));
        diagnostic_stop(__FUNCTION__, __FILE__, __LINE__);
    }
    s = sin_table[angle_idx];
    x = item->pos.x.bits;
    x += (uint32)s * radius;
    /* TODO: Investigate rolling headings outside the original Q15 table */
    if (angle_idx < -MATH_QUARTER_TURN || angle_idx >= MATH_ANGLE_COUNT)
    {
        DIAGNOSTIC(fprintf(stderr, "Invalid rolling cosine index %d object %p stage %u tick %u\n", angle_idx, (void *)item, game_session.stage, game_session.tick));
        diagnostic_stop(__FUNCTION__, __FILE__, __LINE__);
    }
    c = sin_table[MATH_QUARTER_TURN + angle_idx];
    z = item->pos.z.bits;
    z -= (uint32)c * radius;
    point.pos.bits[0] = x;
    point.pos.bits[1] = item->pos.y.bits;
    point.pos.bits[2] = z;
    point.impact_sound = 1;
    owner = item->physics.actor_idx;
    kind = item->kind;
    actor_projectile_contact(owner, &point, kind);
    if (regions_find_obstacle(x, z))
    {
        if (col_regions.sel != region)
        {
            item->physics.speed = 0;
            return;
        }
    }
    angle = item->angles.yaw_bits;
    step = (uint16)item->physics.roll_step;
    item->angles.yaw_bits = angle + step;
    angle_idx = item->physics.heading;
    /* TODO: Investigate rolling headings outside the original Q15 table */
    if (angle_idx < 0 || angle_idx >= MATH_SIN_COUNT)
    {
        DIAGNOSTIC(fprintf(stderr, "Invalid rolling sine index %d object %p stage %u tick %u\n", angle_idx, (void *)item, game_session.stage, game_session.tick));
        diagnostic_stop(__FUNCTION__, __FILE__, __LINE__);
    }
    s = sin_table[angle_idx];
    speed = item->physics.speed;
    x = item->pos.x.bits;
    item->pos.x.bits = x + (uint32)s * (uint32)speed;
    angle_idx = item->physics.heading;
    /* TODO: Investigate rolling headings outside the original Q15 table */
    if (angle_idx < -MATH_QUARTER_TURN || angle_idx >= MATH_ANGLE_COUNT)
    {
        DIAGNOSTIC(fprintf(stderr, "Invalid rolling cosine index %d object %p stage %u tick %u\n", angle_idx, (void *)item, game_session.stage, game_session.tick));
        diagnostic_stop(__FUNCTION__, __FILE__, __LINE__);
    }
    c = sin_table[MATH_QUARTER_TURN + angle_idx];
    speed = item->physics.speed;
    z = item->pos.z.bits;
    item->pos.z.bits = z - (uint32)c * (uint32)speed;
    step = (uint16)item->physics.speed;
    item->physics.speed = step - 1;
    coord = (uint16)item->pos.x.whole;
    region->geometry.axis.max_x = coord;
    region->geometry.axis.min_x = coord;
    coord = (uint16)item->pos.z.whole;
    region->geometry.axis.max_z = coord;
    region->geometry.axis.min_z = coord;
    kind = item->kind;
    col_region_configure(region, kind);
    result = col_regions.count - 1u;
    col_regions.count = result;
}

static sint32 update_exit_doors(MODEL_OBJECT *item)
{
    FUNCTION_MARKER(0x800300bcu, "GAME.EXE");
    /* TODO: Investigate an exit-door sound flag without its native owner */
    if (!item || item->payload_kind != MODEL_PAYLOAD_OSCILLATION)
    {
        DIAGNOSTIC(fprintf(stderr, "Invalid exit-door owner object %p stage %u tick %u\n", (void *)item, game_session.stage, game_session.tick));
        diagnostic_stop(__FUNCTION__, __FILE__, __LINE__);
    }
    sint32 angle, step;
    MODEL_OBJECT *tracked;
    if (!(uint8)level_transition.exit_requested)
    {
        item->oscillation.sound_played = 0;
        return 0;
    }
    /* TODO: Investigate absent or invalid tracked references before object access */
    if (world_objects.tracked[6].present != 1 || world_objects.tracked[6].idx >= WORLD_OBJECT_COUNT)
    {
        DIAGNOSTIC(fprintf(stderr, "Invalid tracked world reference %u slot %u present %u\n", (uint32)(6), world_objects.tracked[6].idx, (uint32)world_objects.tracked[6].present));
        diagnostic_stop(__FUNCTION__, __FILE__, __LINE__);
    }
    tracked = world_objects.object + world_objects.tracked[6].idx;
    angle = tracked->angles.yaw;
    if (angle < 16384)
    {
        step = (angle + 33) >> 4;
        if (step >= 49 && !item->oscillation.sound_played)
        {
            sound_voice_play(124, 255, 4608);
            item->oscillation.sound_played = 1;
        }
        if (step >= 257)
            step = 256;
        angle += step;
    }
    if (angle >= 16385)
    {
        sound_voice_play(20, 255, 5632);
        disp_buffers.shake_ticks = 8;
        angle = 16384;
        /* TODO: Investigate an exit-door release without its named collision region */
        if (!col_regions.named[0])
        {
            DIAGNOSTIC(fprintf(stderr, "Missing exit-door region object %p angle %d stage %u tick %u\n", (void *)item, angle, game_session.stage, game_session.tick));
            diagnostic_stop(__FUNCTION__, __FILE__, __LINE__);
        }
        col_regions.named[0]->col_kind = 0;
    }
    /* TODO: Investigate absent or invalid tracked references before object access */
    if (world_objects.tracked[6].present != 1 || world_objects.tracked[6].idx >= WORLD_OBJECT_COUNT)
    {
        DIAGNOSTIC(fprintf(stderr, "Invalid tracked world reference %u slot %u present %u\n", (uint32)(6), world_objects.tracked[6].idx, (uint32)world_objects.tracked[6].present));
        diagnostic_stop(__FUNCTION__, __FILE__, __LINE__);
    }
    tracked = world_objects.object + world_objects.tracked[6].idx;
    tracked->angles.yaw_bits = (uint16)angle;
    /* TODO: Investigate absent or invalid tracked references before object access */
    if (world_objects.tracked[7].present != 1 || world_objects.tracked[7].idx >= WORLD_OBJECT_COUNT)
    {
        DIAGNOSTIC(fprintf(stderr, "Invalid tracked world reference %u slot %u present %u\n", (uint32)(7), world_objects.tracked[7].idx, (uint32)world_objects.tracked[7].present));
        diagnostic_stop(__FUNCTION__, __FILE__, __LINE__);
    }
    tracked = world_objects.object + world_objects.tracked[7].idx;
    tracked->angles.yaw_bits = (uint16)-angle;
    return -angle;
}

const OBJECT_CALLBACK callback_world_objects_update_exit_doors = {OBJECT_CALL_S32_MUTABLE_MODEL, {.s32_mutable_model = update_exit_doors}, 0};

/* Build model collision regions in world-slot order */
GDB_CALL uint32 world_objects_build_regions(void)
{
    FUNCTION_MARKER(0x80017bb0, "GAME.EXE");
    COLLISION_REGION *region;
    uint32 i = 0;
    uint32 object = 0x800b89f0, coord, type, result = world_objects.count;
    if ((sint32)result <= 0)
        return result;
    do
    {
        if (col_regions.count >= COLLISION_REGION_COUNT)
            diagnostic_stop(__FUNCTION__, __FILE__, __LINE__);
        region = col_regions.data + col_regions.count;
        /* TODO: Investigate original region creation beyond the world pool */
        if (i >= WORLD_OBJECT_COUNT)
        {
            DIAGNOSTIC(fprintf(stderr, "Invalid region world slot %u stage %u\n", i, game_session.stage));
            diagnostic_stop(__FUNCTION__, __FILE__, __LINE__);
        }
        region->object = (WORLD_OBJECT_REFERENCE){i, 1};
        const MODEL_OBJECT *item = world_objects.object + i;
        coord = (uint16)item->pos.x.whole;
        region->geometry.coord[2] = coord;
        region->geometry.coord[0] = coord;
        coord = (uint16)item->pos.z.whole;
        region->geometry.coord[3] = coord;
        region->geometry.coord[1] = coord;
        type = item->kind;
        ++i;
        col_region_configure(region, type);
        result = (sint32)i < (sint32)world_objects.count;
        object += 36;
    } while (result);
    return result;
}

GDB_CALL uint32 world_objects_update_traffic(MODEL_OBJECT *model)
{
    FUNCTION_MARKER(0x8002f59cu, "GAME.EXE");
    COLLISION_REGION *region = col_regions.data;
    uint32 i, x, step;
    sint32 count = (sint32)col_regions.count;
    int wrap;
    for (i = 0; (sint32)i < count; i++, ++region)
    {
        if (region == col_regions.data + COLLISION_REGION_COUNT)
            diagnostic_stop(__FUNCTION__, __FILE__, __LINE__);
        /* TODO: Investigate a corrupt native world reference before region matching */
        if (region->object.present > 1 || (region->object.present && region->object.idx >= WORLD_OBJECT_COUNT))
        {
            DIAGNOSTIC(fprintf(stderr, "Invalid vehicle region world slot %u present %u stage %u tick %u\n", region->object.idx, (uint32)region->object.present, game_session.stage, game_session.tick));
            diagnostic_stop(__FUNCTION__, __FILE__, __LINE__);
        }
        if (region->object.present && model == world_objects.object + region->object.idx)
            break;
    }
    /* TODO: Investigate the original unmatched search selecting an inactive region */
    if (count <= 0 || (sint32)i >= count)
    {
        DIAGNOSTIC(fprintf(stderr, "Unmatched traffic collision region object %p index %u count %d stage %u tick %u\n", (void *)model, i, count, game_session.stage, game_session.tick));
        diagnostic_stop(__FUNCTION__, __FILE__, __LINE__);
    }
    /* TODO: Investigate traffic callbacks without the matching native owner */
    if (!model || model->payload_kind != MODEL_PAYLOAD_TRAFFIC)
    {
        DIAGNOSTIC(fprintf(stderr, "Invalid traffic payload object %p stage %u tick %u\n", (void *)model, game_session.stage, game_session.tick));
        diagnostic_stop(__FUNCTION__, __FILE__, __LINE__);
    }
    TRAFFIC_VEHICLE *traffic = &model->traffic;
    if (model->angles.yaw == 16384)
    {
        step = (uint32)(sint32)traffic->speed << 16;
        x = model->pos.x.bits - step;
        model->pos.x.bits = x;
        wrap = (sint32)x < -1073741824;
        if (wrap)
            model->pos.x.bits = ((random_next() & 8191) + 16384u) << 16;
    }
    else
    {
        step = (uint32)(sint32)traffic->speed << 16;
        x = model->pos.x.bits + step;
        model->pos.x.bits = x;
        wrap = (sint32)x > 1073741824;
        if (wrap)
            model->pos.x.bits = (0xffffc000u - (random_next() & 8191)) << 16;
    }
    if (wrap)
    {
        model->kind = (uint32)(sint32)traffic_wrap_kinds[random_next() & 15];
        traffic->speed = 128 + (random_next() & 63);
        /* TODO: Investigate wrapping a vehicle without a writable collision region */
        if (region == col_regions.data + COLLISION_REGION_COUNT)
        {
            DIAGNOSTIC(fprintf(stderr, "Missing wrap vehicle region object %p count %u stage %u tick %u\n", (void *)model, col_regions.count, game_session.stage, game_session.tick));
            diagnostic_stop(__FUNCTION__, __FILE__, __LINE__);
        }
        col_region_configure(region, model->kind);
        col_regions.count = col_regions.count - 1u;
    }
    return actors_collide_vehicle(region, model);
}

GDB_CALL uint32 world_objects_draw_tank(const MODEL_OBJECT *item)
{
    FUNCTION_MARKER(0x8002a7dcu, "GAME.EXE");
    /* TODO: Investigate a tank draw without the matching native owner */
    if (!item || item->payload_kind != MODEL_PAYLOAD_TANK)
    {
        DIAGNOSTIC(fprintf(stderr, "Invalid tank model %p in %s stage %u tick %u\n", (const void *)item, __FUNCTION__, game_session.stage, game_session.tick));
        diagnostic_stop(__FUNCTION__, __FILE__, __LINE__);
    }
    transform_yaw(item->angles.yaw);
    model_draw_static(model_refs[776], 0, 0);
    transform_translate(0, 0x016a0000, -10485760);
    transform_yaw(item->tank.turret_yaw);
    return model_draw_static(model_refs[777], 0, 0);
}

GDB_CALL uint32 object_draw_base_compressed(MODEL_OBJECT *item)
{
    FUNCTION_MARKER(0x8005b404u, "GAME.EXE");
    /* TODO: Investigate common draw fields outside both native object pools */
    if (!item)
    {
        DIAGNOSTIC(fprintf(stderr, "Invalid compound model %p in %s stage %u tick %u\n", (void *)item, __FUNCTION__, game_session.stage, game_session.tick));
        diagnostic_stop(__FUNCTION__, __FILE__, __LINE__);
    }
    /* TODO: Investigate compression draw without the matching native owner */
    if (!item || item->payload_kind != MODEL_PAYLOAD_OSCILLATION)
    {
        DIAGNOSTIC(fprintf(stderr, "Invalid compression draw object %p callback %s stage %u tick %u\n", (void *)item, __FUNCTION__, game_session.stage, game_session.tick));
        diagnostic_stop(__FUNCTION__, __FILE__, __LINE__);
    }
    sint32 phase = item->oscillation.phase, height, angle;
    if (phase >= 3)
        phase = 2;
    /* TODO: Investigate a negative compression phase before the sine table */
    if (phase < 0)
    {
        DIAGNOSTIC(fprintf(stderr, "Invalid compression phase %d object %p stage %u tick %u\n", phase, (void *)item, game_session.stage, game_session.tick));
        diagnostic_stop(__FUNCTION__, __FILE__, __LINE__);
    }
    height = sin_table[phase << 8] >> 9;
    item->oscillation.pitch_speed = 0;
    item->oscillation.roll_speed = 0;
    angle = item->angles.yaw;
    if (angle)
        transform_yaw(angle);
    model_draw_static(model_refs[68], 0, 8192);
    transform_translate(0, (64u - (uint32)height) << 16, 0);
    angle = item->angles.roll;
    if (angle)
        transform_roll(angle);
    angle = item->angles.pitch;
    if (angle)
        transform_pitch(angle);
    return model_draw_static(model_refs[item->kind], 0, 0);
}

GDB_CALL uint32 object_draw_compressed(MODEL_OBJECT *item)
{
    FUNCTION_MARKER(0x8005b598u, "GAME.EXE");
    /* TODO: Investigate common draw fields outside both native object pools */
    if (!item)
    {
        DIAGNOSTIC(fprintf(stderr, "Invalid compound model %p in %s stage %u tick %u\n", (void *)item, __FUNCTION__, game_session.stage, game_session.tick));
        diagnostic_stop(__FUNCTION__, __FILE__, __LINE__);
    }
    /* TODO: Investigate compression draw without the matching native owner */
    if (!item || item->payload_kind != MODEL_PAYLOAD_OSCILLATION)
    {
        DIAGNOSTIC(fprintf(stderr, "Invalid compression draw object %p callback %s stage %u tick %u\n", (void *)item, __FUNCTION__, game_session.stage, game_session.tick));
        diagnostic_stop(__FUNCTION__, __FILE__, __LINE__);
    }
    sint32 phase = item->oscillation.phase, height, angle;
    if (phase >= 3)
        phase = 2;
    /* TODO: Investigate a negative compression phase before the sine table */
    if (phase < 0)
    {
        DIAGNOSTIC(fprintf(stderr, "Invalid compression phase %d object %p stage %u tick %u\n", phase, (void *)item, game_session.stage, game_session.tick));
        diagnostic_stop(__FUNCTION__, __FILE__, __LINE__);
    }
    height = (5 * sin_table[phase << 8]) >> 12;
    item->oscillation.pitch_speed = 0;
    item->oscillation.roll_speed = 0;
    angle = item->angles.yaw;
    if (angle)
        transform_yaw(angle);
    transform_translate(0, (40u - (uint32)height) << 16, 0);
    angle = item->angles.roll;
    if (angle)
        transform_roll(angle);
    angle = item->angles.pitch;
    if (angle)
        transform_pitch(angle);
    return model_draw_static(model_refs[item->kind], 0, 0);
}

GDB_CALL uint32 world_objects_draw_compressed_car(MODEL_OBJECT *item)
{
    FUNCTION_MARKER(0x8005b178u, "GAME.EXE");
    /* TODO: Investigate a model callback outside its native object domain */
    if (!item)
    {
        DIAGNOSTIC(fprintf(stderr, "Invalid model receiver %p in %s stage %u tick %u\n", (void *)item, __FUNCTION__, game_session.stage, game_session.tick));
        diagnostic_stop(__FUNCTION__, __FILE__, __LINE__);
    }
    /* TODO: Investigate compression draw without the matching native owner */
    if (!item || item->payload_kind != MODEL_PAYLOAD_OSCILLATION)
    {
        DIAGNOSTIC(fprintf(stderr, "Invalid compression draw object %p callback %s stage %u tick %u\n", (void *)item, __FUNCTION__, game_session.stage, game_session.tick));
        diagnostic_stop(__FUNCTION__, __FILE__, __LINE__);
    }
    sint32 phase = item->oscillation.phase, height, angle;
    uint32 offset = (uint32)(phase < 3 ? phase : 2) << 9;
    /* TODO: Investigate a negative compression phase before the native sine table */
    if (phase < 0)
    {
        DIAGNOSTIC(fprintf(stderr, "Invalid compression phase %d object %p stage %u tick %u\n", phase, (void *)item, game_session.stage, game_session.tick));
        diagnostic_stop(__FUNCTION__, __FILE__, __LINE__);
    }
    height = (sin_table[offset >> 1] * 5) >> 12;
    item->oscillation.pitch_speed = 0;
    item->oscillation.roll_speed = 0;
    angle = item->angles.yaw;
    if (angle)
        transform_yaw(angle);
    model_draw_static(model_refs[106], 0, 8192);
    transform_translate(0, (40u - (uint32)height) << 16, 0);
    angle = item->angles.roll;
    if (angle)
        transform_roll(angle);
    angle = item->angles.pitch;
    if (angle)
        transform_pitch(angle);
    transform_push();
    model_draw_static(model_refs[item->kind], 0, 0);
    transform_pop();
    transform_push();
    transform_translate(0, 5963776, -9437184);
    phase = item->oscillation.phase;
    /* TODO: Investigate a panel phase outside the native sine table */
    if (phase < 0 || (uint32)phase > ((MATH_SIN_COUNT - 1u) >> 7))
    {
        DIAGNOSTIC(fprintf(stderr, "Invalid panel phase %d object %p stage %u tick %u\n", phase, (void *)item, game_session.stage, game_session.tick));
        diagnostic_stop(__FUNCTION__, __FILE__, __LINE__);
    }
    transform_pitch(sin_table[(uint32)phase << 7] >> 2);
    model_draw_static(model_refs[1 + (item->kind)], 0, 0);
    transform_pop();
    transform_translate(0, 5767168, 14286848);
    phase = item->oscillation.phase;
    /* TODO: Investigate a panel phase outside the native sine table */
    if (phase < 0 || (uint32)phase > ((MATH_SIN_COUNT - 1u) >> 7))
    {
        DIAGNOSTIC(fprintf(stderr, "Invalid panel phase %d object %p stage %u tick %u\n", phase, (void *)item, game_session.stage, game_session.tick));
        diagnostic_stop(__FUNCTION__, __FILE__, __LINE__);
    }
    transform_pitch((-sin_table[(uint32)phase << 7]) >> 2);
    return model_draw_static(model_refs[2 + (item->kind)], 0, 0);
}

GDB_CALL uint32 world_objects_draw_car(const MODEL_OBJECT *model)
{
    FUNCTION_MARKER(0x8005ae3cu, "GAME.EXE");
    /* TODO: Investigate drawing common fields outside both native object pools */
    if (!model)
    {
        DIAGNOSTIC(fprintf(stderr, "Invalid draw model %p in %s stage %u tick %u\n", (const void *)model, __FUNCTION__, game_session.stage, game_session.tick));
        diagnostic_stop(__FUNCTION__, __FILE__, __LINE__);
    }
    uint32 original_kind = model->kind, phase, shade, color, result;
    PROJECTION_POINT pos;
    sint32 angle;
    angle = model->angles.yaw;
    if (angle)
        transform_yaw(angle);
    model_draw_static(model_refs[106], 0, 8192);
    transform_translate(0, 2621440, 0);
    angle = model->angles.roll;
    if (angle)
        transform_roll(angle);
    angle = model->angles.pitch;
    if (angle)
        transform_pitch(angle);
    model_draw_static(model_refs[model->kind], 0, 0);
    /* TODO: Investigate car lights without the matching native owner */
    if (!model || model->payload_kind != MODEL_PAYLOAD_OSCILLATION)
    {
        DIAGNOSTIC(fprintf(stderr, "Invalid car lights object %p callback %s stage %u tick %u\n", (const void *)model, __FUNCTION__, game_session.stage, game_session.tick));
        diagnostic_stop(__FUNCTION__, __FILE__, __LINE__);
    }
    if (!model->oscillation.voice_plus_one)
        return 0;
    phase = game_session.tick & 15u;
    shade = phase < 4 ? 128u : phase < 12 ? 128u - 16u * (phase - 4u) : 0;
    color = 0x2c000000u + (shade << 16) + (shade << 8) + shade;
    result = 0;
    if (original_kind == 94)
    {
        result = (uint32)proj_local(80, 36, -376, &pos);
        if (!result)
        {
            pos.depth = (sint32)((uint32)pos.depth - 32u);
            pos.size = 640;
            render_style.color.rgbc = color;
            result = spr_draw_scaled(&pos, tex_map[66]);
        }
        result = (uint32)proj_local(-80, 36, -376, &pos);
        if (!result)
        {
            pos.depth = (sint32)((uint32)pos.depth - 32u);
            pos.size = 640;
            render_style.color.rgbc = color;
            result = spr_draw_scaled(&pos, tex_map[66]);
        }
        result = (uint32)proj_local(128, 36, 384, &pos);
        if (!result)
        {
            pos.depth = (sint32)((uint32)pos.depth - 32u);
            pos.size = 640;
            render_style.color.rgbc = color;
            result = spr_draw_scaled(&pos, tex_map[66]);
        }
        result = (uint32)proj_local(-128, 36, 384, &pos);
        if (!result)
        {
            pos.depth = (sint32)((uint32)pos.depth - 32u);
            pos.size = 640;
            render_style.color.rgbc = color;
            result = spr_draw_scaled(&pos, tex_map[66]);
        }
    }
    result = (uint32)proj_local(140, 68, -176, &pos);
    if (!result)
    {
        pos.depth = (sint32)((uint32)pos.depth - 48u);
        pos.size = 640;
        render_style.color.rgbc = color;
        result = spr_draw_scaled(&pos, tex_map[66]);
    }
    result = (uint32)proj_local(-140, 68, -176, &pos);
    if (!result)
    {
        pos.depth = (sint32)((uint32)pos.depth - 48u);
        pos.size = 640;
        render_style.color.rgbc = color;
        result = spr_draw_scaled(&pos, tex_map[66]);
    }
    return result;
}

GDB_CALL sint32 world_objects_start_army_trucks(void)
{
    FUNCTION_MARKER(0x8003efecu, "GAME.EXE");
    sint32 count = (sint32)world_objects.count;
    uint32 i = 0;
    if (count <= 0)
        return count;
    do
    {
        /* TODO: Investigate a scene object count beyond native capacity */
        if (i >= WORLD_OBJECT_COUNT)
        {
            DIAGNOSTIC(fprintf(stderr, "Invalid scene scan slot %u count %u stage %u tick %u\n", i, world_objects.count, game_session.stage, game_session.tick));
            diagnostic_stop(__FUNCTION__, __FILE__, __LINE__);
        }
        MODEL_OBJECT *item = world_objects.object + i;
        if (item->kind == 781)
        {
            /* TODO: Investigate army truck access without its native owner */
            if (!item || item->payload_kind != MODEL_PAYLOAD_ARMY_TRUCK)
            {
                DIAGNOSTIC(fprintf(stderr, "Invalid army truck payload object %08X callback %s stage %u tick %u\n", 0x800b89f0 + 36u * i, __FUNCTION__, game_session.stage, game_session.tick));
                diagnostic_stop(__FUNCTION__, __FILE__, __LINE__);
            }
            item->truck.active = 1;
            sound_voice_play(110, 255, 5936);
        }
        ++i;
    } while ((sint32)i < (sint32)world_objects.count);
    return 0;
}

GDB_CALL sint32 world_objects_update_army_truck(MODEL_OBJECT *model)
{
    FUNCTION_MARKER(0x8003f088u, "GAME.EXE");
    COLLISION_REGION *region;
    FIXED_VEC3 point;
    uint32 offset, x, z, y, variant, angle, unsigned_turn;
    sint32 turn, count, speed, s, c, coord_x, coord_z;
    int i;
    /* TODO: Investigate army truck access without its native owner */
    if (!model || model->payload_kind != MODEL_PAYLOAD_ARMY_TRUCK)
    {
        DIAGNOSTIC(fprintf(stderr, "Invalid army truck payload object %p callback %s stage %u tick %u\n", (void *)model, __FUNCTION__, game_session.stage, game_session.tick));
        diagnostic_stop(__FUNCTION__, __FILE__, __LINE__);
    }
    if (model->truck.active)
    {
        /* TODO: Investigate common motion fields outside the native object pools */
        if (!model)
        {
            DIAGNOSTIC(fprintf(stderr, "Invalid moving model object %p in %s stage %u tick %u\n", (void *)model, __FUNCTION__, game_session.stage, game_session.tick));
            diagnostic_stop(__FUNCTION__, __FILE__, __LINE__);
        }
        angle = model->angles.yaw_bits;
        unsigned_turn = (uint16)model->truck.turn_step;
        turn = model->truck.turn_step;
        model->angles.yaw_bits = (uint16)(angle + unsigned_turn);
        if (turn < 100)
            model->truck.turn_step = (sint16)(turn + 25);
        angle = model->angles.yaw_bits;
        speed = model->truck.speed;
        s = sin_table[angle >> 5];
        angle = model->angles.yaw_bits;
        /* TODO: Investigate vehicle or movement callbacks without a native position */
        if (!model)
        {
            DIAGNOSTIC(fprintf(stderr, "Invalid vehicle or movement position %p stage %u tick %u\n", (void *)model, game_session.stage, game_session.tick));
            diagnostic_stop(__FUNCTION__, __FILE__, __LINE__);
        }
        x = model->pos.x.bits;
        offset = ((uint32)(((sint16)angle >> 5) + 512) & 2047) * 2;
        model->pos.x.bits = x + (uint32)speed * (uint32)s;
        speed = model->truck.speed;
        c = sin_table[offset >> 1];
        z = model->pos.z.bits;
        z -= (uint32)speed * (uint32)c;
        model->pos.z.bits = z;
        if ((sint32)z > (sint32)0xede00000u)
        {
            speed = model->truck.speed;
            if (speed < 120)
                model->truck.speed = (sint16)(speed + 4);
        }
        else
        {
            speed = (sint16)(uint16)((uint32)(uint16)model->truck.speed - 5);
            model->truck.speed = (sint16)speed;
            if (speed < 0)
            {
                model->truck.speed = 0;
                model->truck.active = 0;
                level_wave.paused = 0;
                transform_push();
                transform_identity();
                transform_pitch(32768);
                transform_yaw(model->angles.yaw);
                transform_point(0, 0, 0xfebb0000, point.bits);
                transform_pop();
            }
            angle = model->angles.yaw_bits + 380;
            model->angles.yaw_bits = (uint16)angle;
            transform_push();
            for (i = 0; i < 8; i++)
            {
                transform_identity();
                transform_pitch(32768);
                transform_yaw(model->angles.yaw);
                transform_point(0x00c80000u, 0, 0xfebb0000, point.bits);
                y = (0u - (random_next() & 63u)) << 16;
                variant = random_next() & 3u;
                x = model->pos.x.bits;
                z = model->pos.z.bits;
                effect_spawn_timed(871, x + point.x.bits, y, z + point.z.bits, variant, 0, 0);
                transform_identity();
                transform_pitch(32768);
                transform_yaw(model->angles.yaw);
                transform_point(0xff380000u, 0, 0xfebb0000, point.bits);
                y = (0u - (random_next() & 63u)) << 16;
                variant = random_next() & 3u;
                x = model->pos.x.bits;
                z = model->pos.z.bits;
                effect_spawn_timed(871, x + point.x.bits, y, z + point.z.bits, variant, 0, 0);
            }
            transform_pop();
        }
        coord_x = model->pos.x.whole;
        coord_z = model->pos.z.whole;
        level_script_scene.ox = coord_x;
        level_script_scene.oz = coord_z;
    }
    if (!model->truck.speed)
        return 0;
    count = (sint32)col_regions.count;
    region = col_regions.data;
    for (i = 0; i < count; i++, ++region)
    {
        if (region == col_regions.data + COLLISION_REGION_COUNT)
            diagnostic_stop(__FUNCTION__, __FILE__, __LINE__);
        /* TODO: Investigate a corrupt native world reference before region matching */
        if (region->object.present > 1 || (region->object.present && region->object.idx >= WORLD_OBJECT_COUNT))
        {
            DIAGNOSTIC(fprintf(stderr, "Invalid vehicle region world slot %u present %u stage %u tick %u\n", region->object.idx, (uint32)region->object.present, game_session.stage, game_session.tick));
            diagnostic_stop(__FUNCTION__, __FILE__, __LINE__);
        }
        if (region->object.present && model == world_objects.object + region->object.idx)
            break;
    }
    /* TODO: Investigate the original collision call without a matching region */
    if (i >= count)
    {
        DIAGNOSTIC(fprintf(stderr, "Missing army truck region object %p count %d stage %u tick %u\n", (void *)model, count, game_session.stage, game_session.tick));
        diagnostic_stop(__FUNCTION__, __FILE__, __LINE__);
    }
    return actors_collide_vehicle(region, model);
}

GDB_CALL uint32 world_objects_update_army_truck_wreck(MODEL_OBJECT *model)
{
    FUNCTION_MARKER(0x8003f388u, "GAME.EXE");
    /* TODO: Investigate common motion fields outside the native object pools */
    if (!model)
    {
        DIAGNOSTIC(fprintf(stderr, "Invalid moving model object %p in %s stage %u tick %u\n", (void *)model, __FUNCTION__, game_session.stage, game_session.tick));
        diagnostic_stop(__FUNCTION__, __FILE__, __LINE__);
    }
    /* TODO: Investigate army truck access without its native owner */
    if (!model || model->payload_kind != MODEL_PAYLOAD_ARMY_TRUCK)
    {
        DIAGNOSTIC(fprintf(stderr, "Invalid army truck payload object %p callback %s stage %u tick %u\n", (void *)model, __FUNCTION__, game_session.stage, game_session.tick));
        diagnostic_stop(__FUNCTION__, __FILE__, __LINE__);
    }
    uint32 angle = ((uint16)model->angles.yaw >> 5) * 2u, speed = (uint32)(sint32)model->truck.speed, delta, result, z;
    delta = speed * (uint32)(sint32)sin_table[angle >> 1];
    /* TODO: Investigate vehicle or movement callbacks without a native position */
    if (!model)
    {
        DIAGNOSTIC(fprintf(stderr, "Invalid vehicle or movement position %p stage %u tick %u\n", (void *)model, game_session.stage, game_session.tick));
        diagnostic_stop(__FUNCTION__, __FILE__, __LINE__);
    }
    uint32 x = model->pos.x.bits;
    speed = (uint32)(sint32)model->truck.speed;
    model->pos.x.bits = x + delta;
    delta = speed * (uint32)(sint32)sin_table[MATH_QUARTER_TURN + (angle >> 1)];
    result = (uint16)model->truck.speed - 8u;
    z = model->pos.z.bits;
    model->truck.speed = (sint16)result;
    result <<= 16;
    model->pos.z.bits = z - delta;
    if ((sint32)result < 0)
        model->truck.speed = 0;
    return result;
}

GDB_CALL sint32 world_objects_break_near_attachment(sint32 idx, sint32 attachment)
{
    FUNCTION_MARKER(0x8001f0b4, "GAME.EXE");
    FIXED_VEC3 point;
    uint32 kind, spawn_x, spawn_y, spawn_z, rng;
    sint32 i = 0, count, x, z, slot, object_x, object_z;
    actor_attachment_pos(idx, attachment, point.bits);
    x = point.x.whole;
    z = point.z.whole;
    count = (sint32)world_objects.count;
    if (count <= 0)
        return count;
    do
    {
        /* TODO: Investigate world count growth beyond the native pool */
        if (i >= WORLD_OBJECT_COUNT)
        {
            DIAGNOSTIC(fprintf(stderr, "Invalid attachment destruction world slot %d count %u stage %u tick %u\n", i, world_objects.count, game_session.stage, game_session.tick));
            diagnostic_stop(__FUNCTION__, __FILE__, __LINE__);
        }
        MODEL_OBJECT *world = world_objects.object + i;
        kind = world->kind;
        if (kind - 584u < 2u)
        {
            object_x = world->pos.x.whole;
            object_z = world->pos.z.whole;
            if (math_distance_approx(x, z, object_x, object_z) < 192)
            {
                if (world->kind - 584u < 2u)
                {
                    sound_voice_play(133, 255, 6144);
                    spawn_x = world->pos.x.bits;
                    spawn_y = world->pos.y.bits;
                    spawn_z = world->pos.z.bits;
                    effect_spawn(894, spawn_x, spawn_y, spawn_z);
                    spawn_x = world->pos.x.bits;
                    spawn_y = world->pos.y.bits;
                    spawn_z = world->pos.z.bits;
                    disp_buffers.shake_ticks = 8;
                    slot = dynamic_objects_spawn_item(527, spawn_x, spawn_y, spawn_z, 0);
                    /* TODO: Investigate item pool exhaustion after attachment destruction */
                    if (slot < 0 || slot >= DYNAMIC_OBJECT_COUNT)
                    {
                        DIAGNOSTIC(fprintf(stderr, "Invalid attachment destruction item actor %d attachment %d world %d slot %d stage %u tick %u\n", idx, attachment, i, slot, game_session.stage, game_session.tick));
                        diagnostic_stop(__FUNCTION__, __FILE__, __LINE__);
                    }
                    MODEL_OBJECT *item = dynamic_objects + slot;
                    /* TODO: Investigate a spawned item without the matching native owner */
                    if (item->payload_kind != MODEL_PAYLOAD_PHYSICS)
                    {
                        DIAGNOSTIC(fprintf(stderr, "Invalid attachment item owner actor %d world %d item %d kind %u owner %u stage %u tick %u\n", idx, i, slot, item->kind, (uint32)item->payload_kind, game_session.stage, game_session.tick));
                        diagnostic_stop(__FUNCTION__, __FILE__, __LINE__);
                    }
                    item->physics.speed = 64;
                    item->physics.vertical_speed = 32;
                    world->angles.yaw_bits = world->kind == 584 ? 0xc000 : 0x8000;
                    rng = random_next();
                    item->angles.yaw_bits = (uint16)((uint16)item->angles.yaw - 4096u + (rng & 8191u));
                }
                world->kind = 55;
                return 55;
            }
        }
        ++i;
    } while (i < (sint32)world_objects.count);
    return 0;
}

GDB_CALL uint32 world_objects_update_moving_emitter(MODEL_OBJECT *model)
{
    FUNCTION_MARKER(0x8004322cu, "GAME.EXE");
    uint32 tick, factor, offset, base, x, y, z, volume, pan, result, angle;
    sint32 voice, slot;
    /* TODO: Investigate common fields outside both native object pools */
    if (!model)
    {
        DIAGNOSTIC(fprintf(stderr, "Invalid emitter model %p in %s stage %u tick %u\n", (void *)model, __FUNCTION__, game_session.stage, game_session.tick));
        diagnostic_stop(__FUNCTION__, __FILE__, __LINE__);
    }
    /* TODO: Investigate a moving emitter with a foreign native payload */
    if (model->payload_kind != MODEL_PAYLOAD_OSCILLATION)
    {
        DIAGNOSTIC(fprintf(stderr, "Invalid moving emitter payload %p kind %u owner %u stage %u tick %u\n", (void *)model, model->kind, (uint32)model->payload_kind, game_session.stage, game_session.tick));
        diagnostic_stop(__FUNCTION__, __FILE__, __LINE__);
    }
    if (!model->oscillation.voice_plus_one)
        model->oscillation.voice_plus_one = (uint16)(sound_voice_play(145, 0, 6144) + 1u);
    tick = game_session.tick;
    base = (uint32)(sint32)model->oscillation.base_x << 16;
    factor = tick & 256u ? 256u - (tick & 255u) : (uint8)game_session.tick;
    offset = ((uint32)((model->angles.yaw >> 5) + 512) & 2047u) * 2u;
    x = base + factor * ((uint32)(sint32)sin_table[offset >> 1] << 7);
    model->pos.x.bits = x;
    base = (uint32)(sint32)model->oscillation.base_z << 16;
    offset = ((uint16)model->angles.yaw >> 5) * 2u;
    z = base + factor * ((uint32)(sint32)sin_table[offset >> 1] << 7) + (tick & 256u ? 0x02000000u : 0u);
    model->pos.z.bits = z;
    sound_spatial(&model->pos, &volume, &pan);
    voice = model->oscillation.voice_plus_one;
    if (voice)
        sound_voice_pan(voice - 1, volume, pan);
    result = camera.tracking.pitch_control_bits;
    if (!result)
        return result;
    result = game_session.tick & 15u;
    if (result)
        return result;
    result = model->pos.x.bits + 0x072fffffu;
    if (result > 0x072ffffeu)
        return result;
    angle = (random_next() & 8191u) - 4096u;
    x = model->pos.x.bits;
    y = model->pos.y.bits;
    z = model->pos.z.bits;
    slot = dynamic_objects_spawn_randomized(497, x, y, z, angle);
    /* TODO: Investigate original writes after projectile allocation failure */
    if (slot < 0 || slot >= DYNAMIC_OBJECT_COUNT)
    {
        DIAGNOSTIC(fprintf(stderr, "Invalid moving-object projectile slot %d object %p stage %u tick %u\n", slot, (void *)model, game_session.stage, game_session.tick));
        diagnostic_stop(__FUNCTION__, __FILE__, __LINE__);
    }
    MODEL_OBJECT *projectile = dynamic_objects + slot;
    /* TODO: Investigate an item transfer with a foreign native owner */
    if (projectile->payload_kind != MODEL_PAYLOAD_PHYSICS)
    {
        DIAGNOSTIC(fprintf(stderr, "Invalid item transfer owner slot %d kind %u owner %u phase emitter497 stage %u tick %u\n", slot, projectile->kind, (uint32)projectile->payload_kind, game_session.stage, game_session.tick));
        diagnostic_stop(__FUNCTION__, __FILE__, __LINE__);
    }
    projectile->physics.life = 90;
    projectile->physics.speed = 190;
    return 190;
}

GDB_CALL uint32 world_objects_draw_moving_emitter(const MODEL_OBJECT *item)
{
    FUNCTION_MARKER(0x800454e0u, "GAME.EXE");
    sint32 angle;
    /* TODO: Investigate common draw fields outside both native object pools */
    if (!item)
    {
        DIAGNOSTIC(fprintf(stderr, "Invalid compound model %p in %s stage %u tick %u\n", (const void *)item, __FUNCTION__, game_session.stage, game_session.tick));
        diagnostic_stop(__FUNCTION__, __FILE__, __LINE__);
    }
    transform_yaw(item->angles.yaw);
    angle = item->angles.roll;
    if (angle)
        transform_roll(angle);
    angle = item->angles.pitch;
    if (angle)
        transform_pitch(angle);
    transform_translate(-0x5000000, 0, 0);
    model_draw_faded(model_refs[item->kind], 0, 0);
    transform_translate(0x05000000, 0, 0);
    model_draw_faded(model_refs[item->kind], 0, 0);
    transform_translate(0x05000000, 0, 0);
    return model_draw_faded(model_refs[item->kind], 0, 0);
}

/* Original Q16 emitter offsets and speed are private immutable data */
enum
{
    BURNING_BIN_FLAME_HEIGHT = 96 << 16,
    BURNING_BIN_FLAME_SPEED = 64 << 16,
    BURNING_BIN_SMOKE_OFFSET = 32 << 16
};

GDB_CALL intptr world_objects_update_burning_bin(MODEL_OBJECT *model)
{
    FUNCTION_MARKER(0x800419d4u, "GAME.EXE");
    uint32 volume, pan, random, offset, x, z, result;
    /* TODO: Investigate a burning bin with a foreign native owner */
    if (!model || model->payload_kind != MODEL_PAYLOAD_OSCILLATION)
    {
        DIAGNOSTIC(fprintf(stderr, "Invalid burning bin owner object %p stage %u tick %u\n", (void *)model, game_session.stage, game_session.tick));
        diagnostic_stop(__FUNCTION__, __FILE__, __LINE__);
    }
    if (model->oscillation.voice_plus_one)
    {
        /* TODO: Investigate emitter operands outside both native object pools */
        if (!model)
        {
            DIAGNOSTIC(fprintf(stderr, "Invalid emitter position %p in %s stage %u tick %u\n", (void *)model, __FUNCTION__, game_session.stage, game_session.tick));
            diagnostic_stop(__FUNCTION__, __FILE__, __LINE__);
        }
        sound_spatial(&model->pos, &volume, &pan);
        volume = sound_volume_adjust(volume, -48);
        sound_voice_pan(model->oscillation.voice_plus_one - 1, volume, pan);
    }
    else
        model->oscillation.voice_plus_one = (uint16)((uint32)sound_voice_play(156, 0, 6144) + 1u);
    random = random_next();
    offset = (random & 2047u) * 2u;
    /* TODO: Investigate emitter operands outside both native object pools */
    if (!model)
    {
        DIAGNOSTIC(fprintf(stderr, "Invalid emitter position %p in %s stage %u tick %u\n", (void *)model, __FUNCTION__, game_session.stage, game_session.tick));
        diagnostic_stop(__FUNCTION__, __FILE__, __LINE__);
    }
    x = model->pos.x.bits + (uint32)(sint32)sin_table[offset >> 1] * (random & 31u);
    z = model->pos.z.bits - (uint32)(sint32)sin_table[MATH_QUARTER_TURN + (offset >> 1)] * (random & 31u);
    effect_spawn_motion(876, x, model->pos.y.bits - BURNING_BIN_FLAME_HEIGHT, z, 0, 0u - BURNING_BIN_FLAME_SPEED, 0);
    result = game_session.tick & 3u;
    if (result)
        return result;
    return (intptr)effect_spawn_timed(871, x, model->pos.y.bits - BURNING_BIN_FLAME_HEIGHT - BURNING_BIN_SMOKE_OFFSET, z, 0, 0, 0);
}

/* Original Q16 offsets for the truck-fire particle positions */
enum
{
    TRUCK_FIRE_POINT_COUNT = 21
};

static const uint32 truck_fire_offsets[TRUCK_FIRE_POINT_COUNT][3] = {
    {0x01000000u, 0xfe500000u, 0xff800000u}, {0x01320000u, 0xfe540000u, 0xff850000u}, {0x01650000u, 0xfe580000u, 0xff8a0000u}, {0x01970000u, 0xfe5c0000u, 0xff8f0000u}, {0x01ca0000u, 0xfe600000u, 0xff950000u}, {0x01fc0000u, 0xfe640000u, 0xff9a0000u}, {0x02300000u, 0xfe680000u, 0xffa00000u}, {0x02300000u, 0xfeb80000u, 0xff200000u}, {0x01fc0000u, 0xfeb80000u, 0xff200000u}, {0x01ca0000u, 0xfeb80000u, 0xff200000u}, {0x01970000u, 0xfeb80000u, 0xff200000u}, {0x01650000u, 0xfeb80000u, 0xff200000u}, {0x01320000u, 0xfeb80000u, 0xff200000u}, {0x01000000u, 0xfeb80000u, 0xff200000u}, {0x01000000u, 0xff200000u, 0xff200000u}, {0x00c00000u, 0xff200000u, 0xff200000u}, {0x00800000u, 0xff200000u, 0xff200000u}, {0x00400000u, 0xff200000u, 0xff200000u}, {0x00000000u, 0xff200000u, 0xff200000u}, {0xffc00000u, 0xff200000u, 0xff200000u}, {0xff800000u, 0xff200000u, 0xff200000u},
};

GDB_CALL sint32 world_objects_update_water_cannon(MODEL_OBJECT *model)
{
    FUNCTION_MARKER(0x80041dd8u, "GAME.EXE");
    uint32 angle;
    sint32 result;
    uint32 x, y, z;
    sint32 yaw;
    /* TODO: Investigate emitter operands outside both native object pools */
    if (!model)
    {
        DIAGNOSTIC(fprintf(stderr, "Invalid emitter position %p in %s stage %u tick %u\n", (void *)model, __FUNCTION__, game_session.stage, game_session.tick));
        diagnostic_stop(__FUNCTION__, __FILE__, __LINE__);
    }
    y = model->pos.y.bits;
    yaw = model->angles.yaw;
    x = model->pos.x.bits;
    z = model->pos.z.bits;
    effect_spawn_velocity(874, x, y - 0x500000u, z, yaw, 8192, 32);
    /* TODO: Investigate a water-cannon phase read with a foreign native owner */
    if (model->payload_kind != MODEL_PAYLOAD_OSCILLATION)
    {
        DIAGNOSTIC(fprintf(stderr, "Invalid water cannon owner object %p kind %u payload %u stage %u tick %u\n", (void *)model, model->kind, model->payload_kind, game_session.stage, game_session.tick));
        diagnostic_stop(__FUNCTION__, __FILE__, __LINE__);
    }
    angle = ((uint32)(uint16)model->oscillation.spray_phase + 8u) & 2047u;
    model->oscillation.spray_phase = (uint16)angle;
    result = (sin_table[angle] >> 2) + 1024;
    model->angles.yaw_bits = (uint16)result;
    return result;
}

GDB_CALL intptr world_objects_update_truck_fire(MODEL_OBJECT *model)
{
    FUNCTION_MARKER(0x80041e6cu, "GAME.EXE");
    sint32 random, divisor;
    uint32 slot, offset, x, y, z;
    if (game_session.tick & 1u)
        return 1;
    random = (sint32)random_next();
    divisor = TRUCK_FIRE_POINT_COUNT;
    /* TODO: Investigate invalid original emitter divisors */
    if (!divisor || (divisor == -1 && (uint32)random == 0x80000000u))
    {
        DIAGNOSTIC(fprintf(stderr, "Invalid emitter divisor %d random %08X object %p stage %u tick %u\n", divisor, (uint32)random, (void *)model, game_session.stage, game_session.tick));
        diagnostic_stop(__FUNCTION__, __FILE__, __LINE__);
    }
    slot = 3u * (uint32)(random % divisor);
    /* TODO: Investigate emitter operands outside both native object pools */
    if (!model)
    {
        DIAGNOSTIC(fprintf(stderr, "Invalid emitter position %p in %s stage %u tick %u\n", (void *)model, __FUNCTION__, game_session.stage, game_session.tick));
        diagnostic_stop(__FUNCTION__, __FILE__, __LINE__);
    }
    /* TODO: Investigate a truck-fire selection store with a foreign native owner */
    if (model->payload_kind != MODEL_PAYLOAD_OSCILLATION)
    {
        DIAGNOSTIC(fprintf(stderr, "Invalid truck fire owner object %p kind %u payload %u stage %u tick %u\n", (void *)model, model->kind, model->payload_kind, game_session.stage, game_session.tick));
        diagnostic_stop(__FUNCTION__, __FILE__, __LINE__);
    }
    model->oscillation.point_word = (uint16)slot;
    offset = (sint32)(slot << 16) >> 14;
    x = model->pos.x.bits;
    /* TODO: Investigate a selected point outside the original immutable table */
    if (offset >= TRUCK_FIRE_POINT_COUNT * 12u || offset % 12u)
    {
        DIAGNOSTIC(fprintf(stderr, "Invalid truck fire offset %08X word %u random %08X object %p stage %u tick %u\n", offset, slot, (uint32)random, (void *)model, game_session.stage, game_session.tick));
        diagnostic_stop(__FUNCTION__, __FILE__, __LINE__);
    }
    const uint32 *point = truck_fire_offsets[offset / 12u];
    x += point[0];
    y = model->pos.y.bits;
    y += point[1];
    z = model->pos.z.bits;
    z += point[2];
    y += ((uint32)(random >> 8) & 63u) << 16;
    return (intptr)effect_spawn_timed(880, x, y, z, 0, 0, 0);
}

GDB_CALL sint32 world_objects_update_falling_door(MODEL_OBJECT *item)
{
    FUNCTION_MARKER(0x8002e37cu, "GAME.EXE");
    /* TODO: Investigate common fields outside the native model pools */
    if (!item)
    {
        DIAGNOSTIC(fprintf(stderr, "Invalid sequence model %p in %s stage %u tick %u\n", (void *)item, __FUNCTION__, game_session.stage, game_session.tick));
        diagnostic_stop(__FUNCTION__, __FILE__, __LINE__);
    }
    /* TODO: Investigate a falling door with a foreign native owner */
    if (item->payload_kind != MODEL_PAYLOAD_PHYSICS)
    {
        DIAGNOSTIC(fprintf(stderr, "Invalid falling door owner object %p kind %u payload %u stage %u tick %u\n", (void *)item, item->kind, item->payload_kind, game_session.stage, game_session.tick));
        diagnostic_stop(__FUNCTION__, __FILE__, __LINE__);
    }
    uint32 speed = (uint32)(uint16)item->physics.pitch_speed + 64u;
    sint32 result, count;
    item->physics.pitch_speed = (uint16)speed;
    item->angles.pitch_bits = (uint16)((uint32)(uint16)item->angles.pitch - speed);
    if (item->angles.pitch >= -16384)
        return 0;
    count = item->physics.impact_count;
    if (!count)
    {
        regions_disable_object(item);
        count = item->physics.impact_count;
    }
    if (count < 2)
    {
        item->physics.impact_count = (uint16)(count + 1);
        disp_buffers.shake_ticks = 6;
        sound_voice_play(117, 255, 6144);
    }
    item->angles.pitch_bits = 0xc000;
    result = -(item->physics.pitch_speed >> 2);
    item->physics.pitch_speed = (uint16)result;
    return result;
}

GDB_CALL uint32 world_objects_update_bus_wreck(MODEL_OBJECT *model)
{
    FUNCTION_MARKER(0x8002e444u, "GAME.EXE");
    /* TODO: Investigate a bus wreck with a foreign native owner */
    if (!model || model->payload_kind != MODEL_PAYLOAD_PHYSICS)
    {
        DIAGNOSTIC(fprintf(stderr, "Invalid bus wreck owner object %p stage %u tick %u\n", (void *)model, game_session.stage, game_session.tick));
        diagnostic_stop(__FUNCTION__, __FILE__, __LINE__);
    }
    sint32 state = model->physics.blast_tick;
    uint32 x = 0, volume = 0, result, kind;
    int i;
    if (state == 1)
    {
        if (players_past_z(-0x7000000))
            model->physics.blast_tick = 2;
        return 2;
    }
    if (!state)
        return 2;
    switch (state)
    {
        case 2:
            x = 0xfe000000;
            volume = 128;
            break;
        case 8:
            x = 0xff000000;
            volume = 148;
            break;
        case 16:
            x = 0;
            volume = 168;
            break;
        case 24:
            x = 0x01000000;
            volume = 148;
            break;
        case 32:
            x = 0x02000000;
            volume = 128;
            break;
    }
    if (volume)
    {
        sound_voice_play(19, volume, 6144);
        effect_spawn(893, x, 0xff000000, 0xfdd80000);
        for (i = 0; i < 2; ++i)
        {
            kind = (random_next() & 3) + 902;
            effect_spawn_debris(kind, x, 0xff800000, 0xff000000);
        }
        disp_buffers.shake_ticks = 8;
        disp_buffers.flash_ticks = 4;
    }
    else if (state == 40)
    {
        sound_voice_play(19, 255, 6144);
        /* TODO: Investigate a scripted object without a native position */
        if (!model)
        {
            DIAGNOSTIC(fprintf(stderr, "Invalid scripted object position %p stage %u tick %u\n", (void *)model, game_session.stage, game_session.tick));
            diagnostic_stop(__FUNCTION__, __FILE__, __LINE__);
        }
        effect_spawn(892, model->pos.x.bits, 0xff000000, model->pos.z.bits);
        effect_spawn(603, model->pos.x.bits, 0, model->pos.z.bits);
        disp_buffers.shake_ticks = 16;
        disp_buffers.flash_ticks = 8;
        for (i = 0; i < 48; ++i)
        {
            kind = (random_next() & 3) + 902;
            effect_spawn_debris(kind, model->pos.x.bits, model->pos.y.bits, model->pos.z.bits);
        }
        /* IDA omits the fifth argument; the last two objects face 0x8000. */
        dynamic_objects_spawn_spinning(528, 0xfe200000, 0, 0xfe380000, 0);
        dynamic_objects_spawn_spinning(528, 0x02200000, 0, 0xfe380000, 0);
        dynamic_objects_spawn_spinning(528, 0xfe200000, 0, 0xffa80000, 32768);
        dynamic_objects_spawn_spinning(528, 0x02200000, 0, 0xffa80000, 32768);
        level_wave.paused = 0;
        model->kind = 270;
    }
    result = (uint32)(uint16)model->physics.blast_tick + 1;
    model->physics.blast_tick = (uint16)result;
    return result;
}

GDB_CALL sint32 world_objects_update_lifting_door(MODEL_OBJECT *model)
{
    FUNCTION_MARKER(0x800429f0u, "GAME.EXE");
    sint32 state, result;
    uint32 y;
    if (!level_wave.paused && !game_session.scene_phase)
        return 0;
    if (!game_session.scene_phase && !players_past_z(-0x89b0000))
        return 0;
    /* TODO: Investigate scripted motion with a foreign native owner */
    if (!model || model->payload_kind != MODEL_PAYLOAD_OSCILLATION)
    {
        DIAGNOSTIC(fprintf(stderr, "Invalid scripted motion owner %p in %s stage %u tick %u\n", (void *)model, __FUNCTION__, game_session.stage, game_session.tick));
        diagnostic_stop(__FUNCTION__, __FILE__, __LINE__);
    }
    if (!model->oscillation.door_phase)
    {
        /* TODO: Investigate a missing original door collision region */
        if (!col_regions.named[1])
        {
            DIAGNOSTIC(fprintf(stderr, "Missing lifting-door region 1 object %p phase %d height %d stage %u tick %u\n", (void *)model, model->oscillation.door_phase, model->oscillation.door_height, game_session.stage, game_session.tick));
            diagnostic_stop(__FUNCTION__, __FILE__, __LINE__);
        }
        col_regions.named[1]->col_kind = 3;
        game_session.scene_phase = 1;
        model->oscillation.door_phase = 1;
        sound_voice_play(244, 255, 6144);
        result = sound_voice_play(245, 255, 6144);
        level_wave.paused = 0;
        return result;
    }
    state = model->oscillation.door_phase;
    if (state == 1 || state == 2)
    {
        model->oscillation.door_height = (uint16)((uint16)model->oscillation.door_height + 8u);
        if (state == 1 && model->oscillation.door_height > 320)
        {
            /* TODO: Investigate a missing original door collision region */
            if (!col_regions.named[2])
            {
                DIAGNOSTIC(fprintf(stderr, "Missing lifting-door region 2 object %p phase %d height %d stage %u tick %u\n", (void *)model, model->oscillation.door_phase, model->oscillation.door_height, game_session.stage, game_session.tick));
                diagnostic_stop(__FUNCTION__, __FILE__, __LINE__);
            }
            col_regions.named[2]->col_kind = 0;
            model->oscillation.door_phase = 2;
        }
        else if (state == 2 && model->oscillation.door_height > 768)
        {
            model->oscillation.door_height = 768;
            model->oscillation.door_phase = 3;
        }
        y = (0u - (uint32)(sint32)model->oscillation.door_height) << 16;
        /* TODO: Investigate emitter operands outside both native object pools */
        if (!model)
        {
            DIAGNOSTIC(fprintf(stderr, "Invalid emitter position %p in %s stage %u tick %u\n", (void *)model, __FUNCTION__, game_session.stage, game_session.tick));
            diagnostic_stop(__FUNCTION__, __FILE__, __LINE__);
        }
        model->pos.y.bits = y;
        return y;
    }
    if (model->oscillation.door_phase == 3 && players_past_z(-0x5d00000))
    {
        /* TODO: Investigate a missing original door collision region */
        if (!col_regions.named[2])
        {
            DIAGNOSTIC(fprintf(stderr, "Missing lifting-door region 2 object %p phase %d height %d stage %u tick %u\n", (void *)model, model->oscillation.door_phase, model->oscillation.door_height, game_session.stage, game_session.tick));
            diagnostic_stop(__FUNCTION__, __FILE__, __LINE__);
        }
        col_regions.named[2]->col_kind = 3;
        game_session.scene_phase = 2;
        /* TODO: Investigate common fields outside both native object pools */
        if (!model)
        {
            DIAGNOSTIC(fprintf(stderr, "Invalid emitter model %p in %s stage %u tick %u\n", (void *)model, __FUNCTION__, game_session.stage, game_session.tick));
            diagnostic_stop(__FUNCTION__, __FILE__, __LINE__);
        }
        model->kind = 55;
        return 55;
    }
    return 3;
}

GDB_CALL uint32 world_objects_update_aircraft(MODEL_OBJECT *model)
{
    FUNCTION_MARKER(0x8003f410u, "GAME.EXE");
    /* TODO: Investigate an aircraft timer/phase access with a foreign native owner */
    if (!model || model->payload_kind != MODEL_PAYLOAD_PHYSICS)
    {
        DIAGNOSTIC(fprintf(stderr, "Invalid aircraft owner object %p stage %u tick %u\n", (void *)model, game_session.stage, game_session.tick));
        diagnostic_stop(__FUNCTION__, __FILE__, __LINE__);
    }
    sint32 state = model->physics.flight_phase;
    uint32 result, y;
    uint16 count;
    if (!state)
    {
        /* TODO: Investigate vehicle or movement callbacks without a native position */
        if (!model)
        {
            DIAGNOSTIC(fprintf(stderr, "Invalid vehicle or movement position %p stage %u tick %u\n", (void *)model, game_session.stage, game_session.tick));
            diagnostic_stop(__FUNCTION__, __FILE__, __LINE__);
        }
        uint32 x = model->pos.x.bits;
        y = model->pos.y.bits;
        model->pos.x.bits = x - 0x780000u;
        if ((sint32)y < 0)
        {
            model->pos.y.bits = y + 0xf0000u;
            model->angles.pitch_bits = 0;
        }
        result = (sint32)model->pos.x.bits > 0x1c200000;
        if (result && (sint32)model->pos.y.bits >= 0)
        {
            model->physics.flight_phase = 1;
            result = 500;
            model->physics.wait_ticks = 500;
        }
        return result;
    }
    if (state != 1)
        return 1;
    count = (uint16)model->physics.wait_ticks - 1u;
    model->physics.wait_ticks = count;
    if (count)
        return 0x1c200000;
    /* TODO: Investigate vehicle or movement callbacks without a native position */
    if (!model)
    {
        DIAGNOSTIC(fprintf(stderr, "Invalid vehicle or movement position %p stage %u tick %u\n", (void *)model, game_session.stage, game_session.tick));
        diagnostic_stop(__FUNCTION__, __FILE__, __LINE__);
    }
    model->pos.x.bits = 0x1c200000;
    model->pos.z.bits = 0x1c200000;
    model->physics.flight_phase = 0;
    model->pos.y.bits = 0xfc6b0000u;
    model->angles.pitch_bits = 0xf000;
    return 0xfffff000u;
}

GDB_CALL uint32 world_objects_update_scrolling_wall(MODEL_OBJECT *model)
{
    FUNCTION_MARKER(0x8002e7c4u, "GAME.EXE");
    /* TODO: Investigate an object operation without a native position */
    if (!model)
    {
        DIAGNOSTIC(fprintf(stderr, "Invalid oscillation or emitter position %p stage %u tick %u\n", (void *)model, game_session.stage, game_session.tick));
        diagnostic_stop(__FUNCTION__, __FILE__, __LINE__);
    }
    uint32 result = model->pos.z.bits - 0x00600000u;
    model->pos.z.bits = result;
    if ((sint32)result <= -536870912)
    {
        result = 0x16000000;
        model->pos.z.bits = result;
    }
    return result;
}

GDB_CALL uint32 world_objects_update_radar(MODEL_OBJECT *item)
{
    FUNCTION_MARKER(0x8002e870u, "GAME.EXE");
    sint32 result = sin_table[((game_session.tick << 4) & 0xff0u) >> 1] >> 1;
    /* TODO: Investigate common fields outside the native model pools */
    if (!item)
    {
        DIAGNOSTIC(fprintf(stderr, "Invalid sequence model %p in %s stage %u tick %u\n", (void *)item, __FUNCTION__, game_session.stage, game_session.tick));
        diagnostic_stop(__FUNCTION__, __FILE__, __LINE__);
    }
    item->angles.yaw_bits = (uint16)result;
    return result;
}

GDB_CALL struct EFFECT *world_objects_update_hydrant_spray(const MODEL_OBJECT *model)
{
    FUNCTION_MARKER(0x80041d60u, "GAME.EXE");
    uint32 x = (random_next() & 31u) << 16, z;
    /* TODO: Investigate emitter operands outside both native object pools */
    if (!model)
    {
        DIAGNOSTIC(fprintf(stderr, "Invalid emitter position %p in %s stage %u tick %u\n", (const void *)model, __FUNCTION__, game_session.stage, game_session.tick));
        diagnostic_stop(__FUNCTION__, __FILE__, __LINE__);
    }
    x += model->pos.x.bits;
    z = (random_next() & 31u) << 16;
    z += model->pos.z.bits;
    return effect_spawn_timed(873, x, model->pos.y.bits - 0x100000u, z, 0, 0, 0);
}
