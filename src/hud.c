#include "ff.h"
#include <stdio.h>
#include <stdlib.h>

/* Original 61E6C keeps its formatted continue prompt on the native stack */
GDB_CALL uint32 ff_continue_hud_80061E6C(uint32 player)
{
    uint32 actor = 0x800b4318 + 244 * player, partner = 0x800b4318 + 244 * (1 - player), name;
    sint32 x;
    char text[40];
    FUNCTION_MARKER(0x80061e6c, "GAME.EXE");
    if (r_u32(0x800940b0) == 1 || !r_s8(actor + 119) || r_s16(partner + 66) <= 0 || (sint32)r_u32(0x80093d50) <= 0)
        return 0;
    name = r_u32(actor + 240);
    if (player)
    {
        x = (sint32)(314 - (uint32)FUN_8005EFB8(name));
        FUN_80061B7C(name, x, 16);
        x = (sint32)((uint32)x - 10 - (uint32)FUN_8005EFB8(0x80094740));
        FUN_80061D0C(0x80094740, x, 16);
    }
    else
    {
        x = FUN_80061B7C(name, 16, 16);
        FUN_80061D0C(0x80094738, x, 16);
    }
    sprintf(text, (const char *)psx_addr(0x80093a5c, 1), (const char *)psx_addr(r_u32(0x800924ec), 1));
    x = player ? (sint32)(314 - (uint32)text_width_host(text)) : 16;
    if (r_u32(0x80093dd0) & 8)
        text_draw_host(text, x, 26);
    if (!(FUN_8005F5AC(r_u32(0x80094288 + 4 * player)) & 0x4000))
        return 0;
    w_u32(0x80093d50, r_u32(0x80093d50) - 1);
    w_u16(actor + 66, 380);
    w_u8(actor + 119, 0);
    w_u32(0x80094030 + 4 * player, 0);
    w_u32(0x800941b0 + 4 * player, 50000);
    FUN_8005E9E8((sint32)player);
    FUN_800149EC();
    w_u32(actor + 40, r_u32(partner + 40));
    w_u32(actor + 48, r_u32(partner + 48));
    w_u32(0x80093d54, 0);
    FUN_8001B2C4((sint32)player);
    return 0;
}

static void hud_ammo(uint32 actor, uint32 player, uint32 slot)
{
    sint32 index = r_s8(actor + slot);
    uint32 item, type;
    int show;
    sint32 x;
    sint32 ammo;
    char text[3];
    if (index == -1)
        return;
    item = 0x8009a3c8 + 36u * (uint32)index;
    type = r_u32(item);
    show = slot == 125 ? (type == 515 || type == 504) : (type == 515 || type == 506 || (!player && type == 505) || type - 499u < 5u);
    if (!show)
        return;
    /* Original 61E0C decimal bytes in the caller's native local buffer */
    ammo = r_s16(item + 22);
    if (ammo < 10)
    {
        text[0] = (char)((uint32)ammo + 48);
        text[1] = 0;
    }
    else
    {
        text[0] = (char)(ammo / 10 + 48);
        text[1] = (char)(ammo % 10 + 48);
        text[2] = 0;
    }
    x = FUN_80061B7C(0x80093a60, player ? 224 : 16, 200);
    text_draw_host(text, x, 200);
}

/* Original 620BC player HUD, branch choices and completion display */
GDB_CALL uint32 ff_hud_800620BC_stage0(void)
{
    uint32 i, actor, name, target;
    sint32 hp, shown, mode, x, index;
    FUNCTION_MARKER(0x800620bc, "GAME.EXE");
    for (i = 0; i < 8; i++)
    {
        actor = 0x800b4318 + 244u * i;
        if (r_s16(actor + 52) == -1)
            continue;
        hp = r_s16(actor + 66);
        shown = r_s16(actor + 68);
        if (hp < shown)
        {
            shown = (sint16)(shown - 4);
            w_u16(actor + 68, (uint16)shown);
            if (hp >= shown)
            {
                w_u16(actor + 68, (uint16)hp);
                w_u16(actor + 70, (uint16)hp);
            }
        }
        else if (shown < hp)
        {
            shown = (sint16)(shown + 4);
            w_u16(actor + 68, (uint16)shown);
            w_u16(actor + 70, (uint16)hp);
            if (shown >= hp)
                w_u16(actor + 68, (uint16)hp);
        }
    }
    for (i = 0; i < 2; i++)
    {
        actor = 0x800b4318 + 244u * i;
        if (r_s16(actor + 52) == -1)
            continue;
        mode = (sint32)r_u32(0x80093d5c);
        if ((i && r_u32(0x800940b0) != 2) || !(mode == 2 || (mode < 2 && r_s16(actor + 66) > 0)))
        {
            ff_continue_hud_80061E6C(i);
            continue;
        }
        name = r_u32(actor + 240);
        x = i ? 314 - FUN_8005EFB8(name) : 16;
        if (i)
            FUN_80061B7C(name, x, 16);
        else
            x = FUN_80061B7C(name, x, 16);
        if (mode < 2)
        {
            if (i)
                x = (sint32)((uint32)x - 10 - (uint32)FUN_8005EFB8(0x80094740));
            FUN_80061D0C(0x80094738 + 8 * i, x, 16);
        }
        if (i)
            FUN_8006351C(0x001a0130, actor);
        else
            FUN_80063100(0x001a0010, actor);
        hud_ammo(actor, i, 126);
        hud_ammo(actor, i, 125);
        index = r_s8(actor + 116);
        if (r_s8(actor + 118) && index != -1)
        {
            target = 0x800b4318 + 244u * (uint32)index;
            if (r_s16(target + 66) <= 0 || r_s16(target + 52) == -1)
                w_u8(actor + 118, 0);
            else
            {
                name = r_u32(target + 240);
                FUN_80061B7C(name, i ? 314 - FUN_8005EFB8(name) : 16, 36);
                if (i)
                    FUN_8006351C(0x002e0130, target);
                else
                    FUN_80063100(0x002e0010, target);
            }
        }
    }
    if (r_u32(0x80093d58) == 6 && r_u32(0x80093d60) && !r_u32(0x800941b8))
    {
        uint32 object0 = r_u32(0x800941c8), object1 = r_u32(0x800941cc), object2 = r_u32(0x800941d0), choice;
        sint16 angle;
        sint32 width;
        w_u16(object2 + 12, 0);
        w_u16(object1 + 12, 0);
        w_u16(object0 + 12, 0);
        if (r_u32(0x80093dd0) & 8)
            FUN_80061ADC(r_u32(0x80092438), 40);
        FUN_80047600();
        FUN_80067D3C();
        angle = (sint16)FUN_8001614C(-1536, 512, r_s16(0x80094006), (sint32)(0 - r_u32(0x80094008)) >> 16);
        choice = angle < -8192 ? 0 : angle >= 8193 ? 1 : 2;
        width = FUN_8005EFB8(r_u32(0x8009242c));
        width = (sint32)(300 - (uint32)width - (uint32)FUN_8005EFB8(r_u32(0x80092430))) >> 1;
        FUN_80062C0C(r_u32(0x8009242c), width, 75, choice == 0 ? 0x80093be4 : 0x80093bf0);
        x = FUN_8005EFB8(r_u32(0x8009242c));
        FUN_80062C0C(r_u32(0x80092430), (sint32)((uint32)width + (uint32)x + 30), 75, choice == 1 ? 0x80093be4 : 0x80093bf0);
        x = (sint32)(320 - (uint32)FUN_8005EFB8(r_u32(0x80092434))) >> 1;
        FUN_80062C0C(r_u32(0x80092434), x, 55, choice == 2 ? 0x80093be4 : 0x80093bf0);
        w_u8(0x80093de0, (uint8)choice);
    }
    if (r_u32(0x80093d58) == 13 && r_s8(0x800941a4) && (sint32)r_u32(0x80094674) > 0 && !r_u32(0x800941b8))
    {
        uint32 choice;
        sint32 width;
        if (r_u32(0x80093dd0) & 8)
            FUN_80061ADC(r_u32(0x80092438), 40);
        FUN_80047600();
        FUN_80067D3C();
        choice = (sint32)r_u32(0x80094004) > 0x14000000;
        width = FUN_8005EFB8(r_u32(0x80092428));
        width = (sint32)(320 - (uint32)width - (uint32)FUN_8005EFB8(r_u32(0x80092424))) >> 1;
        FUN_80062C0C(r_u32(0x80092424), width, 54, !choice ? 0x80093be4 : 0x80093bf0);
        x = FUN_8005EFB8(r_u32(0x80092424));
        FUN_80062C0C(r_u32(0x80092428), (sint32)((uint32)width + (uint32)x + 10), 54, choice ? 0x80093be4 : 0x80093bf0);
    }
    {
        uint32 transition = r_u32(0x800941b8), result = transition & 8;
        char text[96];
        if ((sint32)transition >= 16)
        {
            if (result)
                FUN_80061ADC(r_u32(0x8009243c), 60);
            FUN_80061ADC(r_u32(0x80092440), 100);
            sprintf(text, (const char *)psx_addr(0x80093a4c, 1), (const char *)psx_addr(r_u32(0x800924ec), 1), (sint32)r_u32(0x80093d50));
            x = (sint32)(160 - (uint32)(text_width_host(text) >> 1));
            text_draw_host(text, x, 190);
            if (r_s16(0x800b435a) > 0)
            {
                sprintf(text, (const char *)psx_addr(0x80093a68, 1), r_s16(0x800b43a2), 250 * r_s16(0x800b43a4));
                text_draw_host(text, 16, 120);
            }
            result = 2;
            if (r_u32(0x800940b0) == 2)
            {
                result = (uint32)(sint32)r_s16(0x800b444e);
                if ((sint32)result > 0)
                {
                    sprintf(text, (const char *)psx_addr(0x80093a68, 1), r_s16(0x800b4496), 250 * r_s16(0x800b4498));
                    x = (sint32)(314 - (uint32)text_width_host(text));
                    result = (uint32)text_draw_host(text, x, 120);
                }
            }
        }
        return result;
    }
}

/* Original 58634 scene tail, shared by gameplay, menu and all sequences */
GDB_CALL uint32 frame_draw_scene(void)
{
    sint32 fade;
    sint32 red, green, blue;
    uint32 tick;
    if ((sint32)r_u32(0x80093d58) < 26)
        ff_hud_800620BC_stage0();
    if ((sint32)r_u32(0x80093dd4) >= 192)
        FUN_80061ADC(0x80093570, 110);
    red = r_s16(0x8001000c);
    green = r_s16(0x8001000e);
    blue = r_s16(0x80010010);
    if (red + green + blue)
        FUN_8006471C();
    if (r_u32(0x80094190))
        FUN_80064820();
    frame_project_horizon();
    frame_draw_background();
    tick = r_u32(0x80093dd0);
    if ((sint32)tick >= 33)
        return 0;
    fade = (sint32)((tick << 3) - 256u);
    return FUN_80011CCC(fade, fade, fade);
}
