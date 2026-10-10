#include "ff.h"
#include "wip.h"
#include <stdio.h>
#include <stdlib.h>
#define script_wip_zero(pc) \
    do \
    { \
        if (ff_wip_visit((pc), "level_script", __FUNCTION__, __FILE__, __LINE__, 1, "return_zero_from_function")) \
            return 0; \
        script_wip_abort(pc); \
    } while (0)
#define script_wip_void(pc) \
    do \
    { \
        if (ff_wip_visit((pc), "level_script", __FUNCTION__, __FILE__, __LINE__, 1, "return_from_function")) \
            return; \
        script_wip_abort(pc); \
    } while (0)
#define script_wip_fatal(pc) \
    do \
    { \
        ff_wip_visit((pc), "level_script", __FUNCTION__, __FILE__, __LINE__, 0, "abort"); \
        script_wip_abort(pc); \
    } while (0)

static void script_wip_abort(uint32 pc)
{
    fprintf(stderr, "WIP level script4B228 branch %08X\n", pc);
    abort();
}

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

/* 4B228 command1, delay and ordinary NPC creation, audited against MIPS.
 * Vehicle companions, special entrances and other commands remain explicit WIP. */
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
    uint32 x, z, table, actor, capability, stats;
    type = script_word();
    xword = script_word();
    zword = script_word();
    yaw = script_word();
    flag = script_word();
    timer = script_word();
    x = (uint32)xword << 16;
    z = (uint32)zword << 16;
    if (r_s8(0x80093e00))
    {
        x += r_u32(0x80093df4) << 16;
        z += r_u32(0x80093df8) << 16;
    }
    table = r_u32(0x800b3be8 + 4u * (uint32)type);
    index = FUN_8004AF18(type, x, z, yaw, flag, timer, 0);
    /* Original indexes -1 unchecked if full: fail explicitly outside supported state. */
    if (index < 2 || index >= 8)
        script_wip_fatal(0x8004b938);
    actor = 0x800b4318 + 244u * (uint32)index;
    if (type == 44 || type == 45 || type == 46)
    {
        companion = FUN_8004AF18(51, x, z, yaw, 0, 0, 0);
        if (companion < 2 || companion >= 8)
            script_wip_fatal(0x8004b998);
        w_u16(actor + 88, (uint16)companion);
        w_u16(0x800b4318 + 244u * (uint32)companion + 88, (uint16)index);
    }
    w_u32(actor + 160, type == 28 ? 0x800b3e90 : type == 32 ? 0x8009cd48 : type == 48 ? 0x80099f40 : type == 50 ? 0x8009e3d0 : 0x800aee90);
    role = script_word();
    target = script_word();
    w_u32(actor + 168, (uint32)target);
    if ((uint32)role - 1u < 2u && target >= (sint32)r_u32(0x800940b0))
    {
        w_u32(actor + 168, r_u32(0x800940b0) - 1u);
        role = 2;
    }
    w_u32(actor + 164, (uint32)role);
    stats = r_u32(actor + 180);
    behavior = script_word();
    w_u32(actor + 184, (uint32)behavior);
    w_u32(actor + 188, 0);
    capability = 0x80084f84 + 16u * (uint32) r_u8(stats + 8u * (uint32)behavior);
    item = script_word();
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
            if (r_u32(0x80093d58) == 8)
                FUN_80056670(228, 160, 6144);
            if (r_u32(0x80093d58) == 2)
            {
                FUN_800313FC(r_u32(actor + 40), 0xfdc00000u, r_u32(actor + 48));
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
            w_u32(actor + 44, 0u - ((period - phase) << 16) + 1968u);
            w_u16(actor + 80, 10);
        }
        FUN_8004C4D8(actor, table, animation);
        FUN_80011A50(r_u32(0x80081720 + 4u * (uint32)(sint32)r_s16(actor + 52)), actor, 0x800ba210 + 504u * (uint32)index);
        FUN_8001AE7C(actor);
    }
    if (r_s8(0x80093e00) && r_u32(r_u32(0x800940ac)) == 782 && r_u32(0x80093d60) == 1)
    {
        w_u16(actor + 52, 0xffff);
        w_u32(0x80093dbc, r_u32(0x80093dbc) - 1u);
    }
    w_u32(0x80093dc0, r_u32(0x80093dc0) - 1u);
}

GDB_CALL sint32 FUN_8004B228(void)
{
    uint32 p, remaining;
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
    if ((uint32)FUN_8004BE28() == r_u32(0x80093e1c))
        return 0;
    if (!r_u32(0x80093dbc))
    {
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
                w_u8(0x80093dd9, 1);
                w_u8(0x800941a4, 1);
                w_u32(0x800941b8, 1);
                if (r_u32(0x80093d58) != 3)
                    w_u16(r_u32(0x800941c8) + 12, 0);
                break;
            case 3:
                w_u8(0x80093dd9, 1);
                if (r_u32(0x80093d58) == 6)
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
                w_u8(0x80093dd9, 1);
                w_u16(r_u32(r_u32(0x800941cc) + 20u) + 26u, 1);
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
                w_u16(0x80093dec, (uint16)script_word());
                w_u16(0x80093dee, (uint16)script_word());
                if (command == 16)
                    w_u32(0x80093df0, (uint32)script_word());
                else
                    w_u32(0x80093d60, 2);
                w_u32(r_u32(0x800947c8) + 8u, 0xfd000000u);
                break;
            case 12:
                FUN_8002A154();
                w_u16(0x80093dec, (uint16)script_word());
                w_u16(0x80093dee, (uint16)script_word());
                w_u8(0x80093e00, 0);
                w_u32(0x80093df8, 0);
                w_u32(0x80093df4, 0);
                break;
            case 14:
                w_u8(0x80093e00, 1);
                w_u32(0x80093dfc, 240);
                w_u8(0x80093dd9, 1);
                FUN_8003EFEC();
                break;
            case 15:
                w_u8(0x80093dd9, 1);
                w_u32(0x80093d60, 1);
                w_u16(r_u32(0x800941cc) + 12u, 0);
                break;
            case 19:
                w_u32(0x80094198, 0);
                break;
            case 20:
                w_u8(0x80093dd9, 1);
                w_u32(0x80093d60, r_u32(0x80093d60) + 1u);
                break;
            case 17:
            case 18:
                w_u8(0x80093dd9, 1);
                count = script_word();
                if (count <= 0)
                    return 0;
                do
                {
                    uint32 object;
                    a = script_word();
                    b = script_word();
                    kind = 460;
                    if ((count & 1) && command == 18)
                        kind = 462;
                    slot = FUN_800168BC((uint32)kind, (uint32)a << 16, 0, (uint32)b << 16, 0x8000);
                    if (slot < 0 || slot >= 64)
                        script_wip_fatal(0x8004b6c0);
                    object = 0x8009a3c8 + 36u * (uint32)slot;
                    w_u16(object + 22, (uint16)script_word());
                    w_u16(object + 24, (uint16)script_word());
                    w_u16(object + 26, (uint16)script_word());
                    w_u16(object + 28, (uint16)script_word());
                    --count;
                    w_u16(object + 32, (uint16)ff_rand_8006D348());
                    w_u32(0x80094198, 1);
                } while (count > 0);
                return 0;
            default:
                script_wip_fatal(0x8004b760);
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
