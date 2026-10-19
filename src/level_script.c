#include "ff.h"
#include <stdio.h>

GDB_CALL sint32 FUN_8004BE28(void)
{
    sint32 count = 0, type;
    uint32 i, actor;
    for (i = 2; i < 8; i++)
    {
        actor = 0x800b4318 + 244u * i;
        type = r_s16(actor + 52);
        if (type != 51 && type != -1 && r_s16(actor + 66) > 0)
            ++count;
    }
    return count;
}

/* Original signed script words advance the shared cursor before loading */
static sint32 script_word(void)
{
    uint32 p = r_u32(0x800946b8);
    w_u32(0x800946b8, p + 2);
    return r_s16(p);
}

static sint32 script_weapon(sint32 type, uint32 capability, uint32 x, uint32 z)
{
    uint32 bits;
    if (type == -1)
        return -1;
    bits = r_u32(0x800846b8 + 4u * (uint32)type);
    if ((bits & r_u32(capability + 12)) != (bits & 0xffffu))
        return -1;
    return FUN_800167C4(type, x, 1, z, 0);
}

static void script_create(void)
{
    sint32 type, xword, zword, yaw, flag, timer, index, companion, role, target, behavior, item, animation;
    uint32 x, z, table, actor, capability, stats, p, limit, stage, value;
    sint32 relative;
    p = r_u32(0x800946b8);
    w_u32(0x800946b8, p + 2);
    type = r_s16(p);
    w_u32(0x800946b8, p + 4);
    xword = r_s16(p + 2);
    w_u32(0x800946b8, p + 6);
    zword = r_s16(p + 4);
    w_u32(0x800946b8, p + 8);
    yaw = r_s16(p + 6);
    w_u32(0x800946b8, p + 10);
    flag = r_s16(p + 8);
    w_u32(0x800946b8, p + 12);
    relative = r_s8(0x80093e00);
    timer = r_s16(p + 10);
    x = (uint32)xword << 16;
    z = (uint32)zword << 16;
    if (relative)
    {
        x += r_u32(0x80093df4) << 16;
        z += r_u32(0x80093df8) << 16;
    }
    table = r_u32(0x800b3be8 + 4u * (uint32)type);
    index = FUN_8004AF18(type, x, z, yaw, flag, timer, 0);
    // TODO Increase the NPC slot count
    if (index < 0)
    {
        fprintf(stderr, "NPC pool exhausted in script_create: record=%08X type=%d allocation=primary slots=2..7\n", p, type);
        ff_wip_stop(__FUNCTION__, __FILE__, __LINE__);
    }
    actor = 0x800b4318 + 244u * (uint32)index;
    if (type == 44 || type == 45 || type == 46)
    {
        companion = FUN_8004AF18(51, x, z, yaw, 0, 0, 0);
        if (companion < 0)
        {
            fprintf(stderr, "NPC pool exhausted in script_create: record=%08X type=%d allocation=companion primary=%d slots=2..7\n", p, type, index);
            ff_wip_stop(__FUNCTION__, __FILE__, __LINE__);
        }
        w_u16(actor + 88, (uint16)companion);
        w_u16(0x800b4318 + 244u * (uint32)companion + 88, (uint16)index);
    }
    w_u32(actor + 160, type == 28 ? 0x800b3e90 : type == 32 ? 0x8009cd48 : type == 48 ? 0x80099f40 : type == 50 ? 0x8009e3d0 : 0x800aee90);
    p = r_u32(0x800946b8);
    w_u32(0x800946b8, p + 2);
    role = r_s16(p);
    w_u32(0x800946b8, p + 4);
    target = r_s16(p + 2);
    w_u32(actor + 168, (uint32)target);
    if ((uint32)role - 1u < 2u)
    {
        limit = r_u32(0x800940b0);
        if (target >= (sint32)limit)
        {
            w_u32(actor + 168, limit - 1);
            role = 2;
        }
    }
    p = r_u32(0x800946b8);
    stats = r_u32(actor + 180);
    w_u32(actor + 164, (uint32)role);
    w_u32(0x800946b8, p + 2);
    behavior = r_s16(p);
    w_u32(actor + 184, (uint32)behavior);
    behavior = (sint32)r_u32(actor + 184);
    w_u32(actor + 188, 0);
    capability = 0x80084f84 + 16u * (uint32)r_u8(stats + 8u * (uint32)behavior);
    w_u32(0x800946b8, p + 4);
    item = r_s16(p + 2);
    w_u8(actor + 126, (uint8)script_weapon(item, capability, x, z));
    item = script_word();
    w_u8(actor + 125, (uint8)script_weapon(item, capability, x, z));
    animation = script_word();
    if (animation == -1)
    {
        w_u32(actor, 0);
        w_u32(actor + 12, 0xffffffffu);
        w_u32(actor + 4, 0xffffffffu);
    }
    else
    {
        if (animation == 203)
        {
            stage = r_u32(0x80093d58);
            if (stage == 8)
            {
                FUN_80056670(228, 160, 6144);
                stage = r_u32(0x80093d58);
            }
            if (stage == 2)
            {
                value = r_u32(actor + 40);
                p = r_u32(actor + 48);
                FUN_800313FC(value, 0xfdc00000u, p);
                FUN_80056670(20, 255, 6144);
                FUN_80056670(127, 128, 6912);
            }
            w_u32(actor + 44, 0xfd800000u);
        }
        else if (animation == 205)
            w_u32(actor + 44, 0xfd800000u);
        else if (animation == 207)
        {
            w_u32(0x80091bf8, 2);
            w_u32(actor + 44, 0xed400000u);
        }
        else if (animation == 287)
        {
            uint32 phase = r_u32(0x80093da8), period = 9344u;
            if ((sint32)phase >= 9344)
            {
                phase = (phase - 9344u) % 3456u;
                period = 3456u;
            }
            w_u32(actor + 44, period - phase);
            value = r_u32(actor + 44);
            w_u16(actor + 80, 10);
            w_u32(actor + 44, 0u - (value << 16) + 1968u);
        }
        FUN_8004C4D8(actor, table, animation);
        FUN_80011A50(r_u32(0x80081720 + 4u * (uint32)(sint32)r_s16(actor + 52)), actor, 0x800ba210 + 504u * (uint32)index);
        FUN_8001AE7C(actor);
    }
    if (r_s8(0x80093e00) && r_u32(r_u32(0x800940ac)) == 782 && r_u32(0x80093d60) == 1)
    {
        value = r_u32(0x80093dbc);
        w_u16(actor + 52, 0xffff);
        w_u32(0x80093dbc, value - 1);
    }
    w_u32(0x80093dc0, r_u32(0x80093dc0) - 1u);
}

GDB_CALL sint32 FUN_8004B228(void)
{
    uint32 p, remaining, descriptor;
    sint32 timer, command, a, b, c, count, kind, slot;
    timer = (sint32)r_u32(0x80093d40);
    if (timer > 0)
    {
        timer = (sint32)((uint32)timer + 1u);
        w_u32(0x80093d40, (uint32)timer);
        if (timer >= 31)
        {
            w_u32(0x80093d60, 1);
            w_u32(0x80093dd0, 0);
            FUN_80014808();
            w_u32(0x800b431c, 0xffffffffu);
            w_u32(0x800b4410, 0xffffffffu);
            w_u32(0x80093d40, 0);
        }
    }
    if (r_s8(0x80093dd9))
        return 0;
    count = FUN_8004BE28();
    if ((uint32)count == r_u32(0x80093e1c))
        return 0;
    if (!r_u32(0x80093dbc))
    {
        /* Pending van passengers still belong to the current wave */
        for (slot = 2; slot < 8; slot++)
        {
            p = 0x800b4318 + 244 * (uint32)slot;
            if (r_s16(p + 52) == 17 && (sint32)r_u32(p + 4) < 31)
                return 0;
        }
        p = r_u32(0x800946b8);
        command = r_s16(p);
        if (command == -1)
            return -1;
        w_u32(0x800946b8, p + 2);
        command = r_s16(p);
        switch (command)
        {
            case 1:
                break;
            case 2:
                a = (sint32)r_u32(0x80093d58);
                w_u8(0x80093dd9, 1);
                w_u8(0x800941a4, 1);
                w_u32(0x800941b8, 1);
                if (a != 3)
                    w_u16(r_u32(0x800941c8) + 12, 0);
                break;
            case 3:
                a = (sint32)r_u32(0x80093d58);
                w_u8(0x80093dd9, 1);
                if (a == 6)
                    w_u32(0x80093d40, 1);
                else
                    w_u32(0x80093d60, 1);
                break;
            case 4:
                w_u8(0x80093dd9, 1);
                w_u32(0x80093d60, 1);
                break;
            case 6:
            case 8:
                w_u8(0x80093dd9, 1);
                break;
            case 7:
                w_u8(0x80094670, 0);
                break;
            case 9:
                w_u16(r_u32(0x800941cc) + 12u, 0);
                descriptor = r_u32(r_u32(0x800941cc) + 20);
                w_u8(0x80093dd9, 1);
                w_u16(descriptor + 26, 1);
                break;
            case 10:
                w_u8(0x80093dd9, 1);
                w_u8(0x800941a4, 1);
                break;
            case 11:
            case 16:
                w_u32(0x80094198, 1024);
                w_u8(0x80093e00, 1);
                w_u32(0x80093dfc, 240);
                w_u8(0x80093dd9, 1);
                FUN_8002A154();
                p = r_u32(0x800946b8);
                w_u32(0x800946b8, p + 2);
                a = r_u16(p);
                w_u32(0x800946b8, p + 4);
                w_u16(0x80093dec, (uint16)a);
                w_u16(0x80093dee, r_u16(p + 2));
                if (command == 16)
                {
                    w_u32(0x800946b8, p + 6);
                    a = r_s16(p + 4);
                    descriptor = r_u32(0x800947c8);
                    w_u32(0x80093df0, (uint32)a);
                }
                else
                {
                    descriptor = r_u32(0x800947c8);
                    w_u32(0x80093d60, 2);
                }
                w_u32(descriptor + 8, 0xfd000000);
                break;
            case 12:
                FUN_8002A154();
                p = r_u32(0x800946b8);
                w_u32(0x800946b8, p + 2);
                a = r_u16(p);
                w_u32(0x800946b8, p + 4);
                w_u16(0x80093dec, (uint16)a);
                b = r_u16(p + 2);
                w_u8(0x80093e00, 0);
                w_u32(0x80093df8, 0);
                w_u32(0x80093df4, 0);
                w_u16(0x80093dee, (uint16)b);
                break;
            case 14:
                w_u8(0x80093e00, 1);
                w_u32(0x80093dfc, 240);
                w_u8(0x80093dd9, 1);
                FUN_8003EFEC();
                break;
            case 15:
                descriptor = r_u32(0x800941cc);
                w_u8(0x80093dd9, 1);
                w_u32(0x80093d60, 1);
                w_u16(descriptor + 12, 0);
                break;
            case 19:
                w_u32(0x80094198, 0);
                break;
            case 20:
                remaining = r_u32(0x80093d60);
                w_u8(0x80093dd9, 1);
                w_u32(0x80093d60, remaining + 1);
                break;
            case 17:
            case 18:
                p = r_u32(0x800946b8);
                w_u8(0x80093dd9, 1);
                w_u32(0x800946b8, p + 2);
                count = r_s16(p);
                if (count <= 0)
                    return 0;
                do
                {
                    uint32 object;
                    p = r_u32(0x800946b8);
                    w_u32(0x800946b8, p + 2);
                    a = r_s16(p);
                    w_u32(0x800946b8, p + 4);
                    b = r_s16(p + 2);
                    kind = 460;
                    if ((count & 1) && command == 18)
                        kind = 462;
                    slot = FUN_800168BC((uint32)kind, (uint32)a << 16, 0, (uint32)b << 16, 0x8000);
                    object = 0x8009a3c8 + 36u * (uint32)slot;
                    p = r_u32(0x800946b8);
                    w_u32(0x800946b8, p + 2);
                    a = r_u16(p);
                    w_u32(0x800946b8, p + 4);
                    if (slot >= 0)
                        w_u16(object + 22, (uint16)a);
                    a = r_u16(p + 2);
                    w_u32(0x800946b8, p + 6);
                    if (slot >= 0)
                        w_u16(object + 24, (uint16)a);
                    a = r_u16(p + 4);
                    w_u32(0x800946b8, p + 8);
                    if (slot >= 0)
                        w_u16(object + 26, (uint16)a);
                    a = r_u16(p + 6);
                    if (slot >= 0)
                        w_u16(object + 28, (uint16)a);
                    --count;
                    a = (sint32)ff_rand_8006D348();
                    if (slot >= 0)
                        w_u16(object + 32, (uint16)a);
                    w_u32(0x80094198, 1);
                } while (count > 0);
                return 0;
            default:
                break;
        }
        p = r_u32(0x800946b8);
        w_u32(0x800946b8, p + 2);
        a = r_s16(p);
        w_u32(0x800946b8, p + 4);
        w_u32(0x80093dbc, (uint32)a);
        b = r_s16(p + 2);
        w_u32(0x800946b8, p + 6);
        w_u32(0x80093e1c, (uint32)b);
        c = r_s16(p + 4);
        w_u32(0x80094798, 0xffffffffu);
        w_u32(0x800947d8, 0);
        w_u32(0x80093dc0, (uint32)a);
        w_u32(0x80093e18, (uint32)c);
        w_u32(0x800944dc, (uint32)c);
        return 0;
    }
    if ((sint32)r_u32(0x80093dc0) <= 0)
        return 0;
    remaining = r_u32(0x80093dc4);
    if (!remaining)
    {
        p = r_u32(0x800946b8);
        w_u32(0x800946b8, p + 2);
        w_u32(0x80093dc4, (uint32)(sint32)r_s16(p));
        return 0;
    }
    --remaining;
    w_u32(0x80093dc4, remaining);
    if (remaining)
        return 0;
    script_create();
    return 0;
}
