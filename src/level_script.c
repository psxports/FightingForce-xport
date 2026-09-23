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
        type = ff_s16(actor + 52);
        if (type != 51 && type != -1 && ff_s16(actor + 66) > 0)
            ++count;
    }
    return count;
}

/* 4B228 command1, delay and ordinary NPC creation, audited against MIPS.
 * Vehicle companions, special entrances and other commands remain explicit WIP. */
static sint32 script_word(void)
{
    uint32 p = ff_u32(0x800946b8);
    ff_w32(0x800946b8, p + 2);
    return ff_s16(p);
}

static sint32 script_weapon(sint32 type, uint32 capability, uint32 x, uint32 z)
{
    uint32 bits;
    if (type == -1)
        return -1;
    bits = ff_u32(0x800846b8 + 4u * (uint32)type);
    if ((bits & ff_u32(capability + 12)) != (bits & 0xffffu))
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
    if (*(sint8 *)ff_ptr(0x80093e00, 1))
    {
        x += ff_u32(0x80093df4) << 16;
        z += ff_u32(0x80093df8) << 16;
    }
    table = ff_u32(0x800b3be8 + 4u * (uint32)type);
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
        ff_w16(actor + 88, (uint16)companion);
        ff_w16(0x800b4318 + 244u * (uint32)companion + 88, (uint16)index);
    }
    ff_w32(actor + 160, type == 28 ? 0x800b3e90 : type == 32 ? 0x8009cd48 : type == 48 ? 0x80099f40 : type == 50 ? 0x8009e3d0 : 0x800aee90);
    role = script_word();
    target = script_word();
    ff_w32(actor + 168, (uint32)target);
    if ((uint32)role - 1u < 2u && target >= (sint32)ff_u32(0x800940b0))
    {
        ff_w32(actor + 168, ff_u32(0x800940b0) - 1u);
        role = 2;
    }
    ff_w32(actor + 164, (uint32)role);
    stats = ff_u32(actor + 180);
    behavior = script_word();
    ff_w32(actor + 184, (uint32)behavior);
    ff_w32(actor + 188, 0);
    capability = 0x80084f84 + 16u * (uint32) * (uint8 *)ff_ptr(stats + 8u * (uint32)behavior, 1);
    item = script_word();
    *(uint8 *)ff_ptr(actor + 126, 1) = (uint8)script_weapon(item, capability, x, z);
    item = script_word();
    *(uint8 *)ff_ptr(actor + 125, 1) = (uint8)script_weapon(item, capability, x, z);
    animation = script_word();
    if (animation == -1)
    {
        ff_w32(actor, 0);
        ff_w32(actor + 12, 0xffffffffu);
        ff_w32(actor + 4, 0xffffffffu);
    }
    else
    {
        if (animation == 203)
        {
            if (ff_u32(0x80093d58) == 8)
                FUN_80056670(228, 160, 6144);
            if (ff_u32(0x80093d58) == 2)
            {
                FUN_800313FC(ff_u32(actor + 40), 0xfdc00000u, ff_u32(actor + 48));
                FUN_80056670(20, 255, 6144);
                FUN_80056670(127, 128, 6912);
            }
            ff_w32(actor + 44, 0xfd800000u);
        }
        else if (animation == 205)
            ff_w32(actor + 44, 0xfd800000u);
        else if (animation == 207)
        {
            ff_w32(0x80091bf8, 2);
            ff_w32(actor + 44, 0xed400000u);
        }
        else if (animation == 287)
        {
            uint32 phase = ff_u32(0x80093da8), period = 9344u;
            if ((sint32)phase >= 9344)
            {
                phase = (phase - 9344u) % 3456u;
                period = 3456u;
            }
            ff_w32(actor + 44, 0u - ((period - phase) << 16) + 1968u);
            ff_w16(actor + 80, 10);
        }
        FUN_8004C4D8(actor, table, animation);
        FUN_80011A50(ff_u32(0x80081720 + 4u * (uint32)(sint32)ff_s16(actor + 52)), actor, 0x800ba210 + 504u * (uint32)index);
        FUN_8001AE7C(actor);
    }
    if (*(sint8 *)ff_ptr(0x80093e00, 1) && ff_u32(ff_u32(0x800940ac)) == 782 && ff_u32(0x80093d60) == 1)
    {
        ff_w16(actor + 52, 0xffff);
        ff_w32(0x80093dbc, ff_u32(0x80093dbc) - 1u);
    }
    ff_w32(0x80093dc0, ff_u32(0x80093dc0) - 1u);
}

GDB_CALL sint32 FUN_8004B228(void)
{
    uint32 p, remaining;
    sint32 timer, command, a, b, c, count, kind, slot;
    timer = (sint32)ff_u32(0x80093d40);
    if (timer > 0)
    {
        timer = (sint32)((uint32)timer + 1u);
        ff_w32(0x80093d40, (uint32)timer);
        if (timer >= 31)
        {
            ff_w32(0x80093d60, 1);
            ff_w32(0x80093dd0, 0);
            FUN_80014808();
            ff_w32(0x800b431c, 0xffffffffu);
            ff_w32(0x800b4410, 0xffffffffu);
            ff_w32(0x80093d40, 0);
        }
    }
    if (*(sint8 *)ff_ptr(0x80093dd9, 1))
        return 0;
    if ((uint32)FUN_8004BE28() == ff_u32(0x80093e1c))
        return 0;
    if (!ff_u32(0x80093dbc))
    {
        p = ff_u32(0x800946b8);
        command = ff_s16(p);
        if (command == -1)
            return -1;
        ff_w32(0x800946b8, p + 2);
        command = ff_s16(p);
        switch (command)
        {
            case 1:
                break;
            case 2:
                ff_w8(0x80093dd9, 1);
                ff_w8(0x800941a4, 1);
                ff_w32(0x800941b8, 1);
                if (ff_u32(0x80093d58) != 3)
                    ff_w16(ff_u32(0x800941c8) + 12, 0);
                break;
            case 3:
                ff_w8(0x80093dd9, 1);
                if (ff_u32(0x80093d58) == 6)
                    ff_w32(0x80093d40, 1);
                else
                    ff_w32(0x80093d60, 1);
                break;
            case 4:
                ff_w8(0x80093dd9, 1);
                ff_w32(0x80093d60, 1);
                break;
            case 6:
            case 8:
                ff_w8(0x80093dd9, 1);
                break;
            case 7:
                ff_w8(0x80094670, 0);
                break;
            case 9:
                ff_w16(ff_u32(0x800941cc) + 12u, 0);
                ff_w8(0x80093dd9, 1);
                ff_w16(ff_u32(ff_u32(0x800941cc) + 20u) + 26u, 1);
                break;
            case 10:
                ff_w8(0x80093dd9, 1);
                ff_w8(0x800941a4, 1);
                break;
            case 11:
            case 16:
                ff_w32(0x80094198, 1024);
                ff_w8(0x80093e00, 1);
                ff_w32(0x80093dfc, 240);
                ff_w8(0x80093dd9, 1);
                FUN_8002A154();
                ff_w16(0x80093dec, (uint16)script_word());
                ff_w16(0x80093dee, (uint16)script_word());
                if (command == 16)
                    ff_w32(0x80093df0, (uint32)script_word());
                else
                    ff_w32(0x80093d60, 2);
                ff_w32(ff_u32(0x800947c8) + 8u, 0xfd000000u);
                break;
            case 12:
                FUN_8002A154();
                ff_w16(0x80093dec, (uint16)script_word());
                ff_w16(0x80093dee, (uint16)script_word());
                ff_w8(0x80093e00, 0);
                ff_w32(0x80093df8, 0);
                ff_w32(0x80093df4, 0);
                break;
            case 14:
                ff_w8(0x80093e00, 1);
                ff_w32(0x80093dfc, 240);
                ff_w8(0x80093dd9, 1);
                FUN_8003EFEC();
                break;
            case 15:
                ff_w8(0x80093dd9, 1);
                ff_w32(0x80093d60, 1);
                ff_w16(ff_u32(0x800941cc) + 12u, 0);
                break;
            case 19:
                ff_w32(0x80094198, 0);
                break;
            case 20:
                ff_w8(0x80093dd9, 1);
                ff_w32(0x80093d60, ff_u32(0x80093d60) + 1u);
                break;
            case 17:
            case 18:
                ff_w8(0x80093dd9, 1);
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
                    ff_w16(object + 22, (uint16)script_word());
                    ff_w16(object + 24, (uint16)script_word());
                    ff_w16(object + 26, (uint16)script_word());
                    ff_w16(object + 28, (uint16)script_word());
                    --count;
                    ff_w16(object + 32, (uint16)ff_rand_8006D348());
                    ff_w32(0x80094198, 1);
                } while (count > 0);
                return 0;
            default:
                script_wip_fatal(0x8004b760);
        }
        p = ff_u32(0x800946b8);
        ff_w32(0x800946b8, p + 2);
        a = ff_s16(p);
        ff_w32(0x800946b8, p + 4);
        ff_w32(0x80093dbc, (uint32)a);
        b = ff_s16(p + 2);
        ff_w32(0x800946b8, p + 6);
        ff_w32(0x80093e1c, (uint32)b);
        c = ff_s16(p + 4);
        ff_w32(0x80094798, 0xffffffffu);
        ff_w32(0x800947d8, 0);
        ff_w32(0x80093dc0, (uint32)a);
        ff_w32(0x80093e18, (uint32)c);
        ff_w32(0x800944dc, (uint32)c);
        return 0;
    }
    if ((sint32)ff_u32(0x80093dc0) <= 0)
        return 0;
    remaining = ff_u32(0x80093dc4);
    if (!remaining)
    {
        p = ff_u32(0x800946b8);
        ff_w32(0x800946b8, p + 2);
        ff_w32(0x80093dc4, (uint32)(sint32)ff_s16(p));
        return 0;
    }
    --remaining;
    ff_w32(0x80093dc4, remaining);
    if (remaining)
        return 0;
    script_create();
    return 0;
}
