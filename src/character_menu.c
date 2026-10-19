#include "ff.h"
#include <stdio.h>
#include <string.h>

/* GAME.EXE:8004F4B8. Return one only if the candidate is absent. */
GDB_CALL sint32 FUN_8004F4B8(sint32 candidate, uint32 selected, sint32 count)
{
    sint32 i;
    for (i = 0; i < count; i++)
        if (r_u32(selected + 4u * i) == (uint32)candidate)
            return 0;
    return 1;
}

static uint32 actor_name(uint32 selection)
{
    return r_u32(0x800b6e90u + 4u * (uint32)(sint32)r_s16(0x800b434c + 244u * selection));
}

/* Original stack-local name string stays local through packet emission */
static void draw_pair_names(uint32 format, uint32 first, uint32 second)
{
    char buffer[72];
    snprintf(buffer, 72, (const char *)psx_addr(format, 1), (const char *)psx_addr(first, 1), (const char *)psx_addr(second, 1));
    text_center_host(buffer, 50);
}

/* GAME.EXE:80050798. Character selection callback. Cancel resets ready bits but
 * intentionally does not return early: further same-frame input still executes. */
GDB_CALL sint32 FUN_80050798(void)
{
    sint32 i;
    uint32 second, first, available, players, ready;
    second = r_u32(0x80092780);
    first = r_u32(0x8009277c);
    if (second == first)
        w_u32(0x80092780, (second + 1) & 3);
    FUN_80061ADC(r_u32(0x8009469c), 214);
    FUN_8005EC90();
    available = r_u32(0x8009400c);
    players = r_u32(0x80092784);
    w_u32(0x800940b0, available);
    if ((sint32)available < (sint32)players)
    {
        w_u32(0x800927ec, 0x80050270);
        FUN_8004F980((uint32 *)psx_addr(0x80092720, 12), 0, -12032);
        return FUN_8004F490();
    }
    if (players == 1)
    {
        FUN_80061ADC(r_u32(0x80092510), 28);
        FUN_80061ADC(actor_name(r_u32(0x8009277c)), 50);
    }
    else
    {
        uint32 language, format, first_type, second_type, first_name, second_name;
        FUN_80061ADC(r_u32(0x80092514), 28);
        language = r_u32(0x80093d70);
        format = language == 1 ? 0x80092828 : language == 2 ? 0x80092834 : 0x80092840;
        first = r_u32(0x8009277c);
        second = r_u32(0x80092780);
        first_type = (uint32)(sint32)r_s16(0x800b434c + 244 * first);
        second_type = (uint32)(sint32)r_s16(0x800b434c + 244 * second);
        first_name = r_u32(0x800b6e90 + 4 * first_type);
        second_name = r_u32(0x800b6e90 + 4 * second_type);
        draw_pair_names(format, first_name, second_name);
    }
    for (i = 0; i < (sint32)r_u32(0x80092784); i++)
    {
        uint32 raw = (uint32)FUN_8005F5AC((sint32)r_u32(0x80094288 + 4u * i)), edge = 0x80092798 + 4u * i, selected = 0x8009277c + 4u * i, cancel;
        w_u32(edge, (raw ^ r_u32(0x80092790 + 4u * i)) & raw);
        w_u32(0x80092790 + 4u * i, raw);
        cancel = r_u32(edge);
        w_u32(0x800927a0, raw);
        if (cancel & 0x1000)
        {
            w_u32(0x800927ec, 0x80050270);
            w_u32(0x8009272c, 0);
            FUN_8004F980((uint32 *)psx_addr(0x80092720, 12), 0, -12032);
            FUN_8004F490();
        }
        if (!((r_u32(0x8009272c) >> ((uint32)i & 31)) & 1))
        {
            if (r_u32(edge) & 0x80)
            {
                sint32 selected_value = r_s32(selected);
                sint32 candidate = (sint32)((uint32)selected_value - 1);
                if (selected_value <= 0)
                    goto select_next;
                for (; candidate >= 0; candidate--)
                    if (FUN_8004F4B8(candidate, 0x8009277c, (sint32)r_u32(0x80092784)))
                    {
                        w_u32(selected, (uint32)candidate);
                        FUN_8004FA10();
                        FUN_8004F468();
                        break;
                    }
                if (candidate == -1)
                    ff_menu_effect(5, 223);
            }
        select_next:
            if (r_u32(edge) & 0x20)
            {
                sint32 selected_value = r_s32(selected);
                sint32 candidate = (sint32)((uint32)selected_value + 1);
                if (selected_value >= 3)
                    goto select_confirm;
                for (; candidate < 4; candidate++)
                    if (FUN_8004F4B8(candidate, 0x8009277c, (sint32)r_u32(0x80092784)))
                    {
                        w_u32(selected, (uint32)candidate);
                        FUN_8004FA10();
                        FUN_8004F468();
                        break;
                    }
                if (candidate == 4)
                    ff_menu_effect(5, 223);
            }
        select_confirm:
            if (r_u32(edge) & 0x4000)
            {
                FUN_8004F490();
                players = r_u32(0x80092784);
                ready = r_u32(0x8009272c);
                w_u32(0x800940b0, players);
                w_u32(0x8009272c, ready | (1u << ((uint32)i & 31)));
            }
        }
    }
    players = r_u32(0x80092784);
    if (players == 1)
    {
        ready = r_u32(0x8009272c);
        if (ready == players)
        {
            w_u32(0x80092788, ready);
            players = r_u32(0x80092784);
        }
    }
    if (players == 2)
    {
        if (r_u32(0x8009272c) == 3)
            w_u32(0x80092788, 1);
        return 1;
    }
    return 3;
}
