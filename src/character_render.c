#include "ff.h"
#include <stdlib.h>
#include <stdio.h>

/* 5C740: object-local transform, with wrapped 32-bit camera subtraction. */
GDB_CALL void FUN_8005C740(uint32 object)
{
    uint32 x, y, z, camera_x, camera_y, camera_z, type, height;
    sint32 angle;
    x = r_u32(object + 4);
    camera_x = r_u32(0x8008d4a4);
    type = r_u32(object);
    y = r_u32(object + 8);
    height = r_u32(0x800990d8 + 4u * type);
    y += height;
    camera_y = r_u32(0x8008d4a8);
    z = r_u32(object + 12);
    camera_z = r_u32(0x8008d4ac);
    FUN_800100C4((sint32)(x - camera_x), (sint32)(y - camera_y), (sint32)(z - camera_z));
    FUN_800101CC(32768);
    angle = r_s16(object + 16);
    if (angle)
        FUN_8001033C(angle);
    angle = r_s16(object + 18);
    if (angle)
        FUN_800101CC(angle);
    angle = r_s16(object + 20);
    if (angle)
        FUN_800104A0(angle);
}

/* 5C808: complete type dispatch; unknown kinds are original no-draw cases. */
GDB_CALL uint32 FUN_8005C808(uint32 object)
{
    uint32 type = r_u32(object), shade, table = 0;
    sint32 position[4], age, sample = 0;
    FUN_80010040();
    FUN_8005C740(object);
    ff_project_local(0, 0, 0, position);
    if (position[3])
        return FUN_80010028();
    if (type - 862u < 3)
    {
        position[0] = (sint32)((uint32)position[0] - 12u);
        position[1] = (sint32)((uint32)position[1] - 8u);
        FUN_80012390(position, r_s16(0x8009a628 + 2u * type));
        return FUN_80010028();
    }
    if (type == 869)
        return FUN_80010028(); /* Original stores size but never draws. */
    if (type == 881)
    {
        age = (sint16)(uint16)(r_u16(object + 22) + 1);
        w_u16(object + 22, (uint16)age);
        if (age == 7)
            w_u32(object, 0xffffffffu);
        return FUN_80010028();
    }
    switch (type)
    {
        case 870:
            age = r_s16(object + 22);
            shade = 32u - ((uint32)age << 1);
            position[3] = 384;
            w_u32(0x8008d4bc, 0x2c000000u + (shade << 16) + (shade << 8) + shade);
            table = 0x8009ad0a;
            sample = r_s16(object + 22);
            break;
        case 871:
        case 872:
            age = r_s16(object + 22);
            shade = 32u - (uint32)age;
            position[3] = 512;
            w_u32(0x8008d4bc, type == 871 ? 0x2c000000u + (shade << 16) + (shade << 8) + shade : 0x2c000000u + (shade << 8));
            table = 0x8009ad0a;
            sample = (sint16)r_u16(object + 22) >> 1;
            break;
        case 879:
            position[3] = 512;
            table = 0x8009ad94;
            sample = r_s16(object + 22);
            break;
        case 873:
        case 874:
        case 880:
            position[3] = 512;
            w_u32(0x8008d4bc, 0x2c303030);
            table = type == 880 ? 0x8009ad9c : 0x8009ad78;
            sample = (sint16)r_u16(object + 22) >> (type == 874 ? 2 : 1);
            break;
        case 877:
            position[2] = (sint32)((uint32)position[2] - 16u);
            age = r_s16(object + 22);
            position[3] = (sint32)r_u32(0x8008010c + 4u * (uint32)age);
            w_u32(0x8008d4bc, 0x2c608080);
            table = 0x8009adce;
            sample = r_s16(object + 22);
            break;
        case 865:
        case 866:
            position[3] = 256;
            table = type == 865 ? 0x8009acd6 : 0x8009ad4c;
            sample = r_s16(object + 22);
            break;
        case 575:
            age = r_s16(object + 22);
            shade = 64u - ((uint32)age << 2);
            w_u32(0x8008d4bc, 0x2c000000u + (shade << 14) + (shade << 7) + shade);
            position[3] = (sint32)(((uint32)(sint32)r_s16(object + 22) << 5) + 256);
            table = 0x8009ad5a;
            break;
        case 576:
            age = (sint16)r_u16(object + 22);
            shade = 32u - (uint32)(age >> 1);
            w_u32(0x8008d4bc, 0x2c000000u + (shade << 16) + (shade << 8) + shade);
            position[3] = (sint32)(((uint32)(sint32)r_s16(object + 22) << 5) + 256);
            table = 0x8009ad76;
            break;
        default:
            return FUN_80010028();
    }
    /* 5CC94 repeats type879, unreachable after its earlier5C9D8 match. */
    FUN_80012594(position, r_s16(table + 2u * (uint32)sample));
    return FUN_80010028();
}

GDB_CALL uint32 FUN_8005D820(uint32 object)
{
    uint32 type = r_u32(object), a, selected;
    sint32 flags, timer;
    FUN_80010040();
    FUN_8005C740(object);
    flags = r_s16(object + 22);
    if (flags < 0 && (flags & 2))
    {
        return FUN_80010028();
    }
    if (type - 496u < 2u && r_s16(object + 22) > 0)
        FUN_8002BDD0(object);
    if (!r_u32(object + 8) && !r_s16(object + 24))
    {
        if (type == 477)
            FUN_800100C4(0, 0, -3670016);
        if (type == 490 || type == 683)
            FUN_800100C4(0, 0, -2621440);
        if (type == 508)
            FUN_800100C4(0, 2621440, 0);
        if (type == 515)
            FUN_800100C4(0, 2621440, -2097152);
        a = r_u32(0x80094550);
        selected = a != 0xffffffffu && 0x8009a3c8 + 36u * a == object;
        if (!selected)
        {
            a = r_u32(0x80094554);
            selected = a != 0xffffffffu && 0x8009a3c8 + 36u * a == object;
        }
        if (selected && !r_s8(0x80093dd8) && !(r_u32(0x8008d4d4) & 31))
            w_u32(0x8008d4bc, 0x2cffffff);
        ff_static_model_80010B18(r_u32(0x80081720 + 4u * type), 0, 0);
    }
    else
    {
        ff_static_model_80010B18(r_u32(0x80081720 + 4u * type), 0, 0);
        if (type == 683 && (timer = r_s16(object + 28)) > 0)
        {
            w_u16(object + 28, (uint16)(timer - 1));
            FUN_800104A0(8192);
            FUN_800100C4(0, 0, 9437184);
            ff_static_model_80010B18(r_u32(0x800821d4 + 4u * (uint32)(sint32)r_s16(object + 28)), 0, 0);
        }
    }
    return FUN_80010028();
}

/* Exact twins 5DA8C/5DAAC share their sole callee, not separate translations. */
GDB_CALL uint32 FUN_8005DA8C(uint32 object)
{
    return FUN_8005D820(object);
}

GDB_CALL uint32 FUN_8005DAAC(uint32 object)
{
    return FUN_8005D820(object);
}

/* Actual 10F94 uses the low32 product before SRA14. */
GDB_CALL void FUN_80010F94(sint32 x, sint32 y, sint32 z)
{
    uint32 matrix = r_u32(0x8008d494), i;
    uint32 scale[3] = {(uint32)x, (uint32)y, (uint32)z};
    for (i = 0; i < 3; i++, matrix += 6)
    {
        sint16 a = r_s16(matrix), b = r_s16(matrix + 2), c = r_s16(matrix + 4);
        w_u16(matrix, (uint16)((sint32)(scale[0] * (uint32)(sint32)a) >> 14));
        w_u16(matrix + 2, (uint16)((sint32)(scale[1] * (uint32)(sint32)b) >> 14));
        w_u16(matrix + 4, (uint16)((sint32)(scale[2] * (uint32)(sint32)c) >> 14));
    }
}

/* 5FB44: continue prompt, fade, countdown and one/two-controller selection. */
GDB_CALL sint32 FUN_8005FB44(void)
{
    uint32 value = r_u32(0x80093dd4), step, buttons, palette;
    char text[128];
    sint32 red = 0, green, timer, width, x;
    if ((sint32)value <= 0)
        return (sint32)value;
    step = value - 128u;
    if ((sint32)step < 0)
        step = 0;
    green = (sint32)(0u - (step << 1));
    if (green < -256)
        green = -256;
    if ((sint32)step >= 65)
    {
        red = (sint32)(0u - ((step - 64u) << 1));
        if (red < -256)
            red = -256;
    }
    FUN_80011CCC(red, green, green);
    FUN_80011FEC();
    if ((sint32)r_u32(0x80093d50) < (sint32)r_u32(0x800940b0))
    {
        w_u32(0x80093d4c, 329);
        goto advance;
    }
    if (r_u32(0x80093dd4) != 191)
        goto advance;
    FUN_80067D3C();
    sprintf(text, "%s ?", (const char *)psx_addr(r_u32(0x800924ec), 1));
    text_center_host(text, 100);
    timer = (sint32)r_u32(0x80093d4c);
    sprintf(text, "%d", (sint32)(10u - (uint32)(timer / 30)));
    timer = (sint32)r_u32(0x80093d4c);
    if (timer % 30 < 4)
        text_center_gradient_host(text, 130);
    else
        text_center_host(text, 130);
    if (r_u32(0x80093d70) == 2)
        sprintf(text, "ANZAHL DER CONTINUES = %d", (sint32)r_u32(0x80093d50));
    else
    {
        uint32 label = r_u32(0x800924ec);
        uint32 continues = r_u32(0x80093d50);
        sprintf(text, "%sS = %d", (const char *)psx_addr(label, 1), (sint32)continues);
    }
    text_center_host(text, 190);
    width = FUN_8005EFB8(r_u32(0x800924dc));
    x = FUN_8005EFB8(r_u32(0x800924d8));
    x = (sint32)(320u - (uint32)width - (uint32)x) >> 1;
    palette = r_s8(0x80093d48) ? 0x80093bf0 : 0x80093be4;
    FUN_80062C0C(r_u32(0x800924d8), x, 160, palette);
    palette = r_s8(0x80093d48) == 1 ? 0x80093be4 : 0x80093bf0;
    width = FUN_8005EFB8(r_u32(0x800924d8));
    FUN_80062C0C(r_u32(0x800924dc), (sint32)((uint32)x + (uint32)width + 10u), 160, palette);
    buttons = (uint32)FUN_8005F5AC((sint32)r_u32(0x80094288));
    if (r_u32(0x800940b0) == 2)
        buttons |= (uint32)FUN_8005F5AC((sint32)r_u32(0x8009428c));
    if (buttons & 128)
        w_u8(0x80093d48, 0);
    if (buttons & 32)
        w_u8(0x80093d48, 1);
    value = r_u32(0x80093d4c) + 1u;
    w_u32(0x80093d4c, value);
    if (value == 329)
        w_u32(0x80093dd4, r_u32(0x80093dd4) + 2u);
    if (!(buttons & 16384))
        return 1;
    if (r_s8(0x80093d48) != 1)
    {
        w_u32(0x80093dd4, 384);
        return 384;
    }
    w_u32(0x80093d4c, 329);
    w_u32(0x80093d50, 0);
advance:
    value = r_u32(0x80093dd4) + 2u;
    w_u32(0x80093dd4, value);
    return (sint32)value;
}

/* Retain the existing platform/debug symbol while routing the complete body. */
GDB_CALL sint32 ff_game_over_8005FB44_live(void)
{
    return FUN_8005FB44();
}

static uint32 actor_color(uint32 actor, int player)
{
    sint32 r, g, b;
    uint8 flags;
    uint32 color;
    r = 128 + r_s8(actor + 157);
    g = 128 + r_s8(actor + 158);
    b = 128 + r_s8(actor + 159);
    w_u32(0x8008d4bc, 0x2c000000);
    flags = r_u8(actor + 156);
    if (flags & 2)
    {
        w_u32(0x8008d4bc, 0x2e000000);
        w_u32(0x8008d4c0, r_u8(actor + 156) >> 4);
    }
    flags = r_u8(actor + 156);
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
        w_u8(actor + 156, r_u8(actor + 156) & 251);
    }
    color = r_u32(0x8008d4bc) | (uint32)r | ((uint32)g << 8) | ((uint32)b << 16);
    w_u32(0x8008d4bc, color);
    return color;
}

/* 58D9C: item correction rotations in Z/Y/X order, then signed fixed translation. */
GDB_CALL uint32 FUN_80058D9C(uint32 correction)
{
    FUNCTION_MARKER(0x80058d9c, "GAME.EXE");
    sint32 angle = r_s16(correction + 2);
    uint32 x, y, z;
    if (angle)
        FUN_800104A0(angle);
    angle = r_s16(correction + 4);
    if (angle)
        FUN_8001033C(angle);
    angle = r_s16(correction);
    if (angle)
        FUN_800101CC(angle);
    x = (uint32)(sint32)r_s16(correction + 6) << 16;
    y = (uint32)(sint32)r_s16(correction + 8) << 16;
    z = (uint32)(sint32)r_s16(correction + 10) << 16;
    return FUN_800100C4((sint32)x, (sint32)y, (sint32)z);
}

/* Player right-hand path59E78..5A05C retains character-specific corrections */
static void player_right_item(uint32 actor, uint32 index, uint32 color)
{
    sint32 slot = r_s8(actor + 126), type;
    uint32 item, kind;
    if (slot == -1)
        return;
    item = 0x8009a3c8 + 36u * (uint32)slot;
    kind = r_u32(item);
    if (kind == 0xffffffffu)
    {
        w_u8(actor + 126, 255);
        return;
    }
    FUN_80011F74(0x800ba234 + 504u * index);
    FUN_800100C4(0, -786432, 0);
    FUN_8001033C(6144);
    type = r_s16(actor + 52);
    if (type == 1)
    {
        if (kind == 506)
        {
            FUN_800100C4(0, 0, -786432);
            FUN_800101CC(-1536);
            FUN_8001033C(-2048);
        }
        else
        {
            FUN_8001033C(61440);
            FUN_800104A0(4096);
        }
        type = r_s16(actor + 52);
    }
    if (type == 2)
    {
        if (kind == 506)
            FUN_800101CC(-2560);
        else
        {
            if (kind == 515)
            {
                FUN_800100C4(0, 0, 131072);
                FUN_800101CC(64512);
                FUN_8001033C(10240);
            }
            else
                FUN_8001033C(61440);
            FUN_800104A0(64512);
        }
    }
    if (r_s16(actor + 52) == 3)
    {
        if (kind == 506)
        {
            FUN_800100C4(0, 0, -786432);
            FUN_8001033C(-3072);
            FUN_800101CC(-1024);
        }
        else
        {
            FUN_8001033C(61440);
            FUN_800104A0(4096);
        }
    }
    w_u32(item + 4, r_u32(actor + 40));
    w_u32(item + 12, r_u32(actor + 48));
    FUN_80058D9C(0x8009d208 + 12u * kind);
    w_u32(0x8008d4bc, color);
    ff_static_model_80010B18(r_u32(0x80081720 + 4u * kind), 0, 0);
    if (kind - 496u < 2u && r_s16(item + 22) > 0)
        FUN_8002BDD0(item);
}

/* NPC right hand and both left hands share the unadjusted bone transform. */
static void actor_other_item(uint32 actor, uint32 index, uint32 color, int left, int player)
{
    uint32 field = left ? 125 : 126, item, kind;
    sint32 slot = r_s8(actor + field), type;
    if (slot == -1)
        return;
    item = 0x8009a3c8 + 36u * (uint32)slot;
    kind = r_u32(item);
    if (kind == 0xffffffffu)
    {
        w_u8(actor + field, 255);
        return;
    }
    FUN_80011F74((left ? 0x800ba214u : 0x800ba234u) + 504u * index);
    FUN_800100C4(0, -786432, 0);
    FUN_8001033C(6144);
    if (player && left)
    {
        type = r_s16(actor + 52);
        if (type == 1 && kind == 515)
        {
            FUN_800100C4(-524288, -524288, -1310720);
            FUN_800101CC(59392);
            FUN_8001033C(5632);
            FUN_800104A0(7936);
            type = r_s16(actor + 52);
        }
        if (type == 3 && kind == 515)
        {
            FUN_800100C4(-524288, -524288, -1310720);
            FUN_800101CC(59392);
            FUN_8001033C(5632);
            FUN_800104A0(7936);
        }
    }
    w_u32(item + 4, r_u32(actor + 40));
    w_u32(item + 12, r_u32(actor + 48));
    FUN_80058D9C(0x8009d208 + 12u * kind);
    w_u32(0x8008d4bc, color);
    ff_static_model_80010B18(r_u32(0x80081720 + 4u * kind), 0, 0);
    if (!left && kind - 496u < 2 && r_s16(item + 22) > 0)
        FUN_8002BDD0(item);
}

/* Preserve original four-load/four-store ordering, including possible overlap. */
static void actor_copy_pose(uint32 to, uint32 from)
{
    uint32 i, a, b, c, d;
    for (i = 0; i < 496; i += 16)
    {
        a = r_u32(from + i);
        b = r_u32(from + i + 4);
        c = r_u32(from + i + 8);
        d = r_u32(from + i + 12);
        w_u32(to + i, a);
        w_u32(to + i + 4, b);
        w_u32(to + i + 8, c);
        w_u32(to + i + 12, d);
    }
    a = r_u32(from + 496);
    b = r_u32(from + 500);
    w_u32(to + 496, a);
    w_u32(to + 500, b);
}

static void actor_trail(uint32 actor, uint32 index)
{
    uint32 color = r_u32(0x8008d4bc);
    w_u32(0x8008d4bc, color | 0x02000000);
    w_u32(0x8008d4c0, 4);
    FUN_8001060C(r_u32(0x80081720 + 4u * (uint32)(sint32)r_s16(actor + 52)), 0x800bb3c8);
    if (!r_s8(0x80093dd8) && !(r_u32(0x80093dd0) & 1))
        actor_copy_pose(0x800bb3c8, 0x800bb1d0);
    w_u32(0x8008d4bc, color | 0x02000000);
    w_u32(0x8008d4c0, 1);
    FUN_8001060C(r_u32(0x80081720 + 4u * (uint32)(sint32)r_s16(actor + 52)), 0x800bb1d0);
    if (!r_s8(0x80093dd8))
    {
        if (!(r_u32(0x80093dd0) & 1))
            actor_copy_pose(0x800bb1d0, 0x800ba210 + 504u * index);
        if (!r_s8(0x80093dd8))
            w_u8(actor + 156, r_u8(actor + 156) & 247);
    }
    w_u32(0x8008d4bc, color);
}

static void actor_van(uint32 actor)
{
    uint32 x, y, z, camera_x, camera_y, camera_z, height;
    FUN_80010040();
    x = r_u32(actor + 40);
    camera_x = r_u32(0x8008d4a4);
    y = r_u32(actor + 44);
    height = r_u32(0x8009911c);
    camera_y = r_u32(0x8008d4a8);
    z = r_u32(actor + 48);
    camera_z = r_u32(0x8008d4ac);
    FUN_800100C4((sint32)(x - camera_x), (sint32)(y + height - camera_y), (sint32)(z - camera_z));
    FUN_800101CC(32768);
    FUN_8001033C((sint16)((uint32)(sint32)r_s16(actor + 56) << 5));
    ff_static_model_80010B18(r_u32(0x80081824), 0, 0);
    FUN_80010040();
    FUN_800104A0(r_s16(actor + 60));
    FUN_800100C4(0, 4194304, 0);
    ff_static_model_80010B18(r_u32(0x80081764), 0, 0);
    FUN_80010028();
    ff_static_model_80010B18(r_u32(0x80081830), 0, 4096);
    FUN_800100C4(0, 0, 30408704);
    FUN_8001033C(16384);
    FUN_80059394();
    FUN_80010028();
}

static void actor_body(uint32 actor, uint32 index, sint32 type, int player)
{
    uint32 color, x, y, z, camera_x, camera_y, camera_z, height;
    sint32 yaw, scale;
    FUN_80010040();
    x = r_u32(actor + 40);
    camera_x = r_u32(0x8008d4a4);
    y = r_u32(actor + 44);
    height = r_u32(0x800990d8 + 4 * (uint32)type);
    camera_y = r_u32(0x8008d4a8);
    z = r_u32(actor + 48);
    camera_z = r_u32(0x8008d4ac);
    FUN_800100C4((sint32)(x - camera_x), (sint32)(y + height - camera_y), (sint32)(z - camera_z));
    if ((!player && type == 51) || FUN_80011E5C())
    {
        w_u32(actor + 232, r_u32(actor + 232) & ~2u);
        color = actor_color(actor, player);
        if (!player && type == 51)
            FUN_8001033C(32768);
        yaw = (sint16)(0u - ((uint32)(sint32)r_s16(actor + 56) << 5)) + 32768;
        FUN_8001033C(yaw);
        y = r_u32(actor + 16);
        z = r_u32(actor + 32);
        FUN_800100C4(0, (sint32)y, (sint32)z);
        if (!player && type == 14 && (r_u8(actor + 156) & 8))
            actor_trail(actor, index);
        FUN_8001060C(r_u32(0x80081720 + 4u * (uint32)r_s16(actor + 52)), 0x800ba210 + 504u * index);
        if (player)
            player_right_item(actor, index, color);
        else
            actor_other_item(actor, index, color, 0, 0);
        actor_other_item(actor, index, color, 1, player);
        FUN_80010028();
        FUN_80010040();
        x = r_u32(actor + 40);
        camera_x = r_u32(0x8008d4a4);
        camera_y = r_u32(0x8008d4a8);
        z = r_u32(actor + 48);
        camera_z = r_u32(0x8008d4ac);
        FUN_800100C4((sint32)(x - camera_x), (sint32)(0 - camera_y), (sint32)(z - camera_z));
        yaw = (sint16)(0u - ((uint32)(sint32)r_s16(actor + 56) << 5)) + 32768;
        FUN_8001033C(yaw);
        if (player)
            ff_static_model_80010B18(r_u32(0x80082490), 0, 0);
        else
        {
            if (type == 28 || type == 51)
                FUN_8001033C(16384);
            y = r_u32(actor + 44);
            height = r_u32(actor + 16);
            scale = (sint32)((uint32)((sint32)(y + height) >> 12) + (type == 50 ? 24576 : 16384));
            if (scale > 0)
            {
                FUN_80010F94(scale, scale, scale);
                ff_static_model_80010B18(r_u32(0x80082490), 0, 0);
            }
        }
    }
    else
        w_u32(actor + 232, player ? (r_u32(actor + 232) | 2u) : 2u);
    FUN_80010028();
}

/* 593FC: character, held-item, trail, van and shadow rendering. */
GDB_CALL sint32 ff_characters_800593FC(void)
{
    FUNCTION_MARKER(0x800593fc, "GAME.EXE");
    sint32 i, type, count;
    uint32 actor, x, y, z, camera_x, camera_y, camera_z, model, frame;
    FUN_80011C60();
    ff_game_over_8005FB44_live();
    for (i = 2; i < 8; i++)
    {
        actor = 0x800b4318 + 244u * (uint32)i;
        type = r_s16(actor + 52);
        if (type == -1 || (r_s8(actor + 119) & 2))
            continue;
        if (type == 17)
        {
            actor_van(actor);
            continue;
        }
        actor_body(actor, (uint32)i, type, 0);
    }
    count = (sint32)r_u32(0x800940b0);
    if (count <= 0)
        return count;
    i = 0;
    do
    {
        if (r_u32(0x80093e04) == 2)
            return 2;
        actor = 0x800b4318 + 244u * (uint32)i;
        type = r_s16(actor + 52);
        if (type != -1 && !(r_u8(actor + 119) & 2))
        {
            if (r_s16(actor + 98))
            {
                FUN_80010040();
                x = r_u32(actor + 40);
                camera_x = r_u32(0x8008d4a4);
                y = r_u32(actor + 44);
                camera_y = r_u32(0x8008d4a8);
                z = r_u32(actor + 48);
                camera_z = r_u32(0x8008d4ac);
                FUN_800100C4((sint32)(x - camera_x), (sint32)(y - camera_y), (sint32)(z - camera_z));
                model = r_u32(0x800824ac + 4 * ((uint32)i & 1));
                frame = r_u32(actor + 8);
                ff_static_model_80010B18(model, frame, 0);
                FUN_80010028();
            }
            actor_body(actor, (uint32)i, type, 1);
        }
        ++i;
        count = (sint32)r_u32(0x800940b0);
    } while (i < count);
    return 0;
}

/* 2BDD0: weapon effect intensity groups, projected depth bias and sprite. */
GDB_CALL uint32 FUN_8002BDD0(uint32 object)
{
    FUNCTION_MARKER(0x8002bdd0u, "GAME.EXE");
    sint32 group = (r_s16(object + 22) + 29) / 30, position[4];
    /* Group2/above takes the unconditional path at2BE0C. */
    if (group < 2 && (group != 1 || !(r_u32(0x80093dd0) & 2)))
        return 2;
    ff_project_local(0, 0, 0, position);
    if (position[2] >= 65)
        position[2] = (sint32)((uint32)position[2] - 64);
    if (position[3])
        return (uint32)group << 1;
    return FUN_80012390(position, r_s16(0x8009adba + 2u * (uint32)group));
}

/* 5D67C: settled debris local transform and kind-dependent origin bias. */
GDB_CALL uint32 FUN_8005D67C(uint32 object)
{
    FUNCTION_MARKER(0x8005d67cu, "GAME.EXE");
    uint32 kind;
    FUN_80010040();
    FUN_8005C740(object);
    kind = r_u32(object);
    FUN_800100C4(0, 0, kind == 529 ? -3932160 : kind == 530 ? -3670016 : -3145728);
    ff_static_model_80010B18(r_u32(0x80081720 + 4u * r_u32(object)), 0, 0);
    return FUN_80010028();
}

/* 5B178: base and two hinged panels, caller owns the outer transform. */
GDB_CALL uint32 FUN_8005B178(uint32 object)
{
    FUNCTION_MARKER(0x8005b178u, "GAME.EXE");
    sint32 phase = r_s16(object + 28), height, angle;
    uint32 offset = (uint32)(phase < 3 ? phase : 2) << 9;
    height = (r_s16(0x800802d0 + offset) * 5) >> 12;
    w_u16(object + 26, 0);
    w_u16(object + 22, 0);
    angle = r_s16(object + 16);
    if (angle)
        FUN_8001033C(angle);
    ff_static_model_80010B18(r_u32(0x800818c8), 0, 8192);
    FUN_800100C4(0, (sint32)((40u - (uint32)height) << 16), 0);
    angle = r_s16(object + 20);
    if (angle)
        FUN_800104A0(angle);
    angle = r_s16(object + 18);
    if (angle)
        FUN_800101CC(angle);
    FUN_80010040();
    ff_static_model_80010B18(r_u32(0x80081720 + 4u * r_u32(object)), 0, 0);
    FUN_80010028();
    FUN_80010040();
    FUN_800100C4(0, 5963776, -9437184);
    FUN_800101CC((sint16)r_u16(0x800802d0 + ((uint32)(sint32)r_s16(object + 28) << 8)) >> 2);
    ff_static_model_80010B18(r_u32(0x80081724 + 4u * r_u32(object)), 0, 0);
    FUN_80010028();
    FUN_800100C4(0, 5767168, 14286848);
    FUN_800101CC((-r_s16(0x800802d0 + ((uint32)(sint32)r_s16(object + 28) << 8))) >> 2);
    return ff_static_model_80010B18(r_u32(0x80081728 + 4u * r_u32(object)), 0, 0);
}
