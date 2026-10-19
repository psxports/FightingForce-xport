#include "ff.h"

GDB_CALL sint32 FUN_80061800(uint32 text, uint32 buffer)
{
    FUNCTION_MARKER(0x80061800u, "GAME.EXE");
    uint32 p = buffer, word = buffer, line_width = 0, word_width = 0, i;
    uint8 ch;
    for (i = 0; i < 40; i++)
        w_u8(0x800940d8 + i, 0);
    /* Preserve the original strcpy null checks and forward copy including NUL */
    if (text && buffer)
    {
        i = 0;
        do
        {
            ch = r_u8(text + i);
            w_u8(buffer + i, ch);
            ++i;
        } while (ch);
    }
    w_u32(0x80094330, 0);
    w_u32(0x800940d8, buffer);
    for (;;)
    {
        sint32 letter = r_s8(p);
        if (letter == 32 || !letter)
        {
            if ((sint32)(line_width + word_width) >= 273)
            {
                line_width = word_width;
                word_width = 0;
                i = r_u32(0x80094330) + 1;
                w_u32(0x80094330, i);
                if (r_u32(0x800940d4 + 4 * i) != word)
                    w_u32(0x800940d8 + 4 * i, word);
                w_u8(word - 1, 0);
                if (!r_s8(p))
                    break;
                /* The overflowing separator goes through the glyph path as in 618E8. */
            }
            else
            {
                line_width += 10 + word_width;
                word_width = 0;
                if (!r_s8(p))
                    break;
                word = ++p;
                continue;
            }
        }
        letter = r_s8(p++);
        word_width += (uint32)(sint32)r_s8(0x800800d0 + (uint32)FUN_80018670(letter));
    }
    w_u32(0x80094330, 0);
    for (;;)
    {
        i = r_u32(0x80094330);
        if (!r_u32(0x800940d8 + 4 * i))
            return (sint32)(i + 1);
        w_u32(0x80094330, ++i);
        if ((sint32)i >= 10)
            return 0;
    }
}

GDB_CALL sint32 FUN_8006159C(uint32 text, sint32 x, sint32 y, sint32 phase)
{
    FUNCTION_MARKER(0x8006159cu, "GAME.EXE");
    sint32 position[4], letter, sprite, offset, step, width;
    uint8 character = r_u8(text);
    position[1] = y;
    position[2] = 128;
    if (!character)
        return 0;
    do
    {
        letter = (sint8)character;
        text++;
        if (letter == 32)
            x = (sint32)((uint32)x + 6u);
        else
        {
            letter -= 65;
            sprite = letter + r_s16(0x8009acc8 + 2u * (uint32)(sint32)r_s16(0x800af4a8));
            if (phase < 0)
                offset = 400;
            else
            {
                step = phase > 16 ? 16 : phase;
                offset = (32767 - r_s16(0x800802d0 + ((uint32)step << 6))) >> 5;
            }
            position[0] = (sint32)((uint32)x + (uint32)offset);
            position[3] = 3 * offset + 64;
            if (offset != 400)
                FUN_80012594(position, sprite);
            width = (sint8)r_u8(0x800800d0 + (uint32)letter);
            x = (sint32)((uint32)x + (uint32)(width + (width >> 3)));
        }
        character = r_u8(text);
        letter = r_s8(text);
        phase = (sint32)((uint32)phase - 3);
    } while (letter);
    return 0;
}

GDB_CALL sint32 FUN_800616DC(uint32 text, sint32 x, sint32 y, sint32 phase)
{
    FUNCTION_MARKER(0x800616dcu, "GAME.EXE");
    sint32 position[4], letter, sprite, offset, step;
    uint8 character = r_u8(text);
    position[1] = y;
    position[2] = 128;
    if (!character)
        return 0;
    do
    {
        letter = (sint8)character - 65;
        text++;
        sprite = letter + r_s16(0x8009ad40 + 2u * (uint32)(sint32)r_s16(0x800af4a8));
        if (phase < 0)
            offset = 400;
        else
        {
            step = phase > 16 ? 16 : phase;
            offset = (32767 - r_s16(0x800802d0 + ((uint32)step << 6))) >> 5;
        }
        position[0] = (sint32)((uint32)x + (uint32)offset);
        position[3] = 3 * offset + 64;
        if (offset != 400)
            FUN_80012594(position, sprite);
        x = (sint32)((uint32)x + (uint32)(sint32)r_s16(0x8008013c + 2u * (uint32)letter));
        character = r_u8(text);
        letter = r_s8(text);
        phase = (sint32)((uint32)phase - 4);
    } while (letter);
    return 0;
}

GDB_CALL sint32 FUN_800619E8(sint32 y)
{
    FUNCTION_MARKER(0x800619e8u, "GAME.EXE");
    sint32 count = (sint32)r_u32(0x80094330);
    uint32 i = 0;
    if (count <= 0)
        return count;
    if (y == -1)
    {
        do
        {
            FUN_80061ADC(r_u32(0x800940d8 + 4u * i), (sint32)(16u * i + 120u - 8u * (uint32)count));
            ++i;
            count = (sint32)r_u32(0x80094330);
        } while ((sint32)i < count);
        return (sint32)(4u * i);
    }
    do
    {
        FUN_80061ADC(r_u32(0x800940d8 + 4u * i), (sint32)((uint32)y + 10u * i));
        ++i;
    } while ((sint32)i < (sint32)r_u32(0x80094330));
    return 0;
}

GDB_CALL sint32 FUN_80061CB8(uint32 text, sint32 x, sint32 y)
{
    FUNCTION_MARKER(0x80061cb8u, "GAME.EXE");
    uint32 width = (uint32)FUN_8005EFB8(text);
    return FUN_80061B7C(text, (sint32)((uint32)x + 10u - width), y);
}

GDB_CALL uint32 FUN_8005EEDC(void)
{
    FUNCTION_MARKER(0x8005eedcu, "GAME.EXE");
    sint32 level;
    w_u16(0x80093e14, 0);
    w_u16(0x80093e12, 0);
    w_u16(0x80093e10, 0);
    for (level = 0; level <= 256; level += 32)
        FUN_8005EE58(level);
    return 0;
}

GDB_CALL uint32 FUN_8005EF30(void)
{
    FUNCTION_MARKER(0x8005ef30u, "GAME.EXE");
    sint32 level;
    uint32 result;
    for (level = 256; level > 0; level -= 32)
        result = FUN_8005EE58(level);
    w_u16(0x80093e14, 0);
    w_u16(0x80093e12, 0);
    w_u16(0x80093e10, 0);
    return result;
}

/* Original stack-local XYZ crosses the host ABI as a native pointer,
 * just like the existing 12594 sprite path; no emulated RAM scratch alias. */
GDB_CALL uint32 FUN_8005EF80(sint32 x, sint32 y, uint32 texture)
{
    FUNCTION_MARKER(0x8005ef80u, "GAME.EXE");
    sint32 position[3] = {x, y, 256};
    return FUN_800124A0(position, texture);
}

/* 6494C: sky gradient and three horizontally wrapping sprites. MIPS keeps
 * coordinates in 32 bits until SH; the stage-4 horizon offset is +132. */
GDB_CALL uint32 FUN_8006494C(void)
{
    uint32 horizon = r_u32(0x8008d4d0), bottom, a, color, x, v, i;
    uint8 *p;
    if (r_u32(0x80093d58) == 4)
        horizon += 132u;
    bottom = horizon - 128u;
    if ((sint32)bottom > 0)
    {
        color = r_u32(0x8008d4c8);
        a = r_u32(0x8008d4b0);
        w_u32(0x8008d4b0, a + 36);
        p = psx_addr(a, 36);
        setPolyG4(p);
        for (i = 0; i < 4; i++)
        {
            p[4 + 8 * i] = i < 2 ? 0 : (uint8)(color << 3);
            p[5 + 8 * i] = i < 2 ? 0 : (uint8)(((sint32)color >> 2) & ~7);
            p[6 + 8 * i] = i < 2 ? 0 : (uint8)(((sint32)color >> 7) & ~7);
            w_u16(a + 8 + 8 * i, (uint16)((i & 1) ? 320 : 0));
            w_u16(a + 10 + 8 * i, (uint16)(i < 2 ? horizon - 288u : bottom));
        }
        FUN_80011E40(r_u32(0x8008d4b4), a);
    }
    x = (uint32)((sint32)r_s16(0x8008d490) >> 5) + 160u;
    v = (x & 256u) >> 1;
    x &= 255u;
    for (i = 0; i < 3; i++)
    {
        a = r_u32(0x8008d4b0);
        w_u32(0x8008d4b0, a + 20);
        p = psx_addr(a, 20);
        setSprt(p);
        w_u16(a + 14, 0x7c00);
        setShadeTex(p, 1);
        w_u16(a + 8, (uint16)(x + 256u * i - 256u));
        w_u16(a + 10, (uint16)bottom);
        p[12] = 0;
        p[13] = (uint8)(i == 1 ? 128u - v : v);
        w_u16(a + 16, 256);
        w_u16(a + 18, 128);
        FUN_80011E40(r_u32(0x8008d4b4), a);
    }
    a = r_u32(0x8008d4b0);
    w_u32(0x8008d4b0, a + 12);
    p = psx_addr(a, 12);
    setDrawMode(p, 1, 0, 142, NULL);
    if ((uint32)(r_u8(0x800872ec) - 1) < 2)
        w_u32(a + 4, 0xe100108e);
    return FUN_80011E40(r_u32(0x8008d4b4), a);
}

/* 5E9E8: five signed decimal quotients, blank leading zeros, literal units0. */
sint32 FUN_8005E9E8(sint32 player)
{
    static const sint32 divisors[5] = {100000, 10000, 1000, 100, 10};
    uint32 i, address, started = 0;
    sint32 value, q;
    uint8 digit;
    if ((uint32)player >= 2)
        return 0;
    address = 0x80094738 + 8u * (uint32)player;
    value = (sint32)r_u32(0x80094030 + 4u * (uint32)player);
    for (i = 0; i < 5; i++)
    {
        q = value / divisors[i];
        digit = (uint8)((uint32)q + 48);
        if (!started && digit == 48)
            digit = 32;
        else
            started = 1;
        w_u8(address + i, digit);
        value = (sint32)((uint32)value - (uint32)q * (uint32)divisors[i]);
    }
    w_u8(address + 5, 48);
    w_u8(address + 6, 0);
    return 48;
}

/* 8006686C: menu backdrop packets. Original signed shifts and low-halfword
 * coordinate stores; AA PsyQ initializers reuse existing normalized twins. */
GDB_CALL uint32 FUN_8006686C(void)
{
    sint32 x = (sint32)((uint32)((sint32)r_u32(0x8008d4a4) >> 8) - (((uint32)(sint32)r_s16(0x8008d490) << 10) + 0x200000u)) >> 16;
    uint32 horizon = r_u32(0x8008d4d0) - 256u, a, color;
    uint8 *p;
    if ((sint32)horizon > 0)
    {
        color = r_u32(0x8008d4c8);
        a = r_u32(0x8008d4b0);
        w_u32(0x8008d4b0, a + 24);
        p = psx_addr(a, 24);
        setPolyF4(p);
        p[4] = (uint8)(color << 3);
        p[5] = (uint8)(((sint32)color >> 2) & ~7);
        p[6] = (uint8)(((sint32)color >> 7) & ~7);
        w_u16(a + 8, 0);
        w_u16(a + 10, 0);
        w_u16(a + 12, 320);
        w_u16(a + 14, 0);
        w_u16(a + 16, 0);
        w_u16(a + 18, (uint16)horizon);
        w_u16(a + 20, 320);
        w_u16(a + 22, (uint16)horizon);
        FUN_80011E40(r_u32(0x8008d4b4), a);
    }
    a = r_u32(0x8008d4b0);
    w_u32(0x8008d4b0, a + 20);
    p = psx_addr(a, 20);
    setSprt(p);
    w_u16(a + 14, 0x7c00);
    setShadeTex(p, 1);
    w_u16(a + 8, (uint16)(0u - (uint32)x));
    w_u16(a + 10, (uint16)horizon);
    p[12] = p[13] = 0;
    w_u16(a + 16, 256);
    w_u16(a + 18, 256);
    FUN_80011E40(r_u32(0x8008d4b4), a);
    a = r_u32(0x8008d4b0);
    w_u32(0x8008d4b0, a + 12);
    p = psx_addr(a, 12);
    setDrawMode(p, 1, 0, 142, NULL);
    /* Original SDK supports alternate GPU revisions with the DFE bit at 12. */
    if ((uint32)(r_u8(0x800872ec) - 1) < 2)
        w_u32(a + 4, 0xe100108e);
    return FUN_80011E40(r_u32(0x8008d4b4), a);
}

/* 80062AB4: all colors precede coordinates; allocation padding is preserved */
GDB_CALL uint32 FUN_80062AB4(sint32 x, sint32 y, sint32 value)
{
    sint32 width = value >> 2;
    uint32 a, ux = (uint32)x, uy = (uint32)y;
    uint8 *p;
    int i;
    if (width > 0)
    {
        a = r_u32(0x8008d4b0);
        w_u32(0x8008d4b0, a + 36);
        p = psx_addr(a, 36);
        setPolyG4(p);
        for (i = 0; i < 4; i++)
        {
            w_u8(a + 4 + 8 * i, i < 2 ? 0 : 255);
            w_u8(a + 5 + 8 * i, i < 2 ? 0 : 255);
            w_u8(a + 6 + 8 * i, i < 2 ? 255 : 0);
        }
        w_u16(a + 8, (uint16)(ux + 1));
        w_u16(a + 10, (uint16)(uy + 1));
        w_u16(a + 16, (uint16)(ux + 1));
        w_u16(a + 18, (uint16)(uy + 5));
        w_u16(a + 24, (uint16)(ux + (uint32)width + 1));
        w_u16(a + 26, (uint16)(uy + 1));
        w_u16(a + 32, (uint16)(ux + (uint32)width + 1));
        w_u16(a + 34, (uint16)(uy + 5));
        FUN_80011E40(r_u32(0x8008d4b4) + 8188, a);
    }
    a = r_u32(0x8008d4b0);
    w_u32(0x8008d4b0, a + 24);
    p = psx_addr(a, 24);
    setPolyF4(p);
    w_u8(a + 4, 0);
    w_u8(a + 5, 0);
    w_u8(a + 6, 0);
    for (i = 0; i < 4; i++)
    {
        w_u16(a + 8 + 4u * i, (uint16)(ux + (i < 2 ? 66 : 0)));
        w_u16(a + 10 + 4u * i, (uint16)(uy + (i & 1 ? 6 : 0)));
    }
    return FUN_80011E40(r_u32(0x8008d4b4) + 8188, a);
}

#include <stdlib.h>

/* Original GAME.EXE:80018670..8001886C. IDA draft audited against all MIPS branches.
 * The unsigned lowercase-range comparison precedes signed special-character cases. */
GDB_CALL sint32 FUN_80018670(sint32 value)
{
    if ((uint32)value - 97u < 26u)
        value = (sint32)((uint32)value - 32u);
    switch (value)
    {
        case 91:
            return 26;
        case 93:
            return 27;
        case 123:
            return 28;
        case 125:
            return 29;
        case 63:
            return 30;
        case 46:
            return 31;
        case 40:
            return 32;
        case 41:
            return 33;
        case -93:
            return 34;
        case 36:
            return 35;
        case 37:
            return 36;
        case 94:
            return 37;
        case -60:
            return 48;
        case -42:
            return 49;
        case -36:
            return 50;
        case 45:
            return 51;
        case 44:
            return 52;
        case 39:
            return 53;
        case 33:
            return 54;
        case 95:
            return 55;
        case 60:
            return 56;
        case 35:
            return 57;
        case 61:
            return 58;
        default:
            return (sint32)((uint32)value - (value >= 65 ? 65u : 10u));
    }
}

/* Original GAME.EXE:8005EFB8..8005F050. Widths are LB (signed), not LBU.
 * Text bytes are sign-extended before conversion. Return includes the +10 margin. */
GDB_CALL sint32 FUN_8005EFB8(uint32 text)
{
    uint32 width = 0;
    uint8 character = r_u8(text);
    if (character)
    {
        do
        {
            sint32 ch = (sint8)character;
            text++;
            if (ch == 32)
                width += 6;
            else
            {
                sint32 index = FUN_80018670(ch);
                width += (sint32)r_s8(0x800800d0u + (uint32)index);
            }
            character = r_u8(text);
        } while (r_s8(text));
    }
    return (sint32)(width + 10u);
}

/* Native text view for original stack-local strings, using the same guest font */
sint32 text_width_host(const char *text)
{
    uint32 width = 0;
    while (*text)
    {
        sint32 ch = (sint8)*text++;
        if (ch == 32)
            width += 6;
        else
            width += (sint32)r_s8(0x800800d0 + (uint32)FUN_80018670(ch));
    }
    return (sint32)(width + 10);
}

static uint32 sprite_checked_80012594(sint64 value)
{
    if (value < -2147483647LL - 1 || value > 2147483647LL)
        ff_wip_stop(__FUNCTION__, __FILE__, __LINE__);
    return (uint32)value;
}

/* Draw-mode packet precedes the textured sprite in the final LIFO chain */
GDB_CALL uint32 FUN_80012390(const sint32 *position, sint32 sprite)
{
    uint32 packet = r_u32(0x8008d4b0), descriptor;
    sint32 x = (sint16)position[0], y = (sint16)position[1];
    uint32 uv_clut, mode, end, semitrans, uv, delta, ot, table, bucket, next;
    sint32 depth;
    descriptor = sprite_checked_80012594((sint64)(sint32)0x800a8768 + (sint32)((uint32)sprite << 2));
    descriptor = sprite_checked_80012594((sint64)(sint32)descriptor + (sint32)((uint32)sprite << 3));
    uv_clut = r_u32(descriptor);
    mode = r_u32(descriptor + 4);
    end = r_u32(descriptor + 8);
    semitrans = uv_clut & 1;
    uv = uv_clut - semitrans;
    delta = sprite_checked_80012594((sint64)(end >> 16) - (sint32)uv) & 65535;
    w_u32(packet + 12, uv);
    w_u32(packet + 4, (semitrans << 25) | 0x65000000);
    w_u16(packet + 8, (uint16)x);
    w_u16(packet + 10, (uint16)y);
    w_u16(packet + 16, (uint16)((delta & 255) + 1));
    w_u16(packet + 18, (uint16)((delta >> 8) + 1));
    ot = r_u32(0x8008d4b4);
    depth = position[2];
    table = r_u32(0x8008d4d8);
    table = sprite_checked_80012594((sint64)(sint32)table + (sint32)((uint32)(depth >> 2) << 1));
    bucket = sprite_checked_80012594((sint64)(sint32)ot + r_s16(table));
    w_u32(packet, r_u32(bucket) | 0x04000000);
    next = sprite_checked_80012594((sint64)(sint32)packet + 20);
    w_u32(bucket, packet & 0xffffff);
    packet = next;
    w_u32(packet + 4, (mode >> 16) + 0xe1000000 + 1024);
    w_u32(packet + 8, 0);
    w_u32(packet, r_u32(bucket) | 0x02000000);
    next = sprite_checked_80012594((sint64)(sint32)packet + 12);
    w_u32(bucket, packet & 0xffffff);
    w_u32(0x8008d4b0, next);
    return 0x8008d4b0;
}

/* 12594: perspective-scaled FT4. MIPS DIV has no compiler break guards. */
static sint32 ff_sprite_div(sint32 n, sint32 d)
{
    if (!d)
        return n < 0 ? 1 : -1;
    if (n == (sint32)0x80000000u && d == -1)
        return n;
    return n / d;
}

GDB_CALL uint32 FUN_80012594(const sint32 *position, sint32 sprite)
{
    uint32 p = r_u32(0x8008d4b0), a, uv, mode, end, color, semi, delta, w, h, bucket, ot, table;
    sint32 x = (sint16)position[0], y = (sint16)position[1], dx, dy, depth;
    uint32 height, right, bottom, left, top;
    a = sprite_checked_80012594((sint64)(sint32)0x800a8768 + (sint32)((uint32)sprite << 2));
    a = sprite_checked_80012594((sint64)(sint32)a + (sint32)((uint32)sprite << 3));
    uv = r_u32(a);
    mode = r_u32(a + 4);
    end = r_u32(a + 8);
    color = r_u32(0x8008d4bc);
    semi = uv & 1;
    uv -= semi;
    delta = (sprite_checked_80012594((sint64)(end >> 16) - (sint32)uv) & 65535) + 257;
    w = delta & 255;
    h = delta >> 8;
    w_u32(p + 12, uv);
    w_u32(p + 20, mode);
    w_u16(p + 28, (uint16)end);
    w_u16(p + 36, (uint16)(end >> 16));
    w_u32(p + 4, (semi << 25) | color);
    depth = position[2];
    height = (uint32)position[3];
    dx = ff_sprite_div((sint32)(w * height), depth);
    dy = w == h ? dx : ff_sprite_div((sint32)(h * height), depth);
    right = sprite_checked_80012594((sint64)x + dx);
    bottom = sprite_checked_80012594((sint64)y + dy);
    left = sprite_checked_80012594((sint64)x - dx);
    top = sprite_checked_80012594((sint64)y - dy);
    right = sprite_checked_80012594((sint64)(sint32)right - 1);
    bottom = sprite_checked_80012594((sint64)(sint32)bottom - 1);
    w_u16(p + 8, (uint16)left);
    w_u16(p + 24, (uint16)left);
    w_u16(p + 10, (uint16)top);
    w_u16(p + 18, (uint16)top);
    w_u16(p + 16, (uint16)right);
    w_u16(p + 32, (uint16)right);
    w_u16(p + 26, (uint16)bottom);
    w_u16(p + 34, (uint16)bottom);
    ot = r_u32(0x8008d4b4);
    table = r_u32(0x8008d4d8);
    table = sprite_checked_80012594((sint64)(sint32)table + (sint32)((uint32)(depth >> 2) << 1));
    bucket = sprite_checked_80012594((sint64)(sint32)ot + r_s16(table));
    w_u32(p, r_u32(bucket) | 0x09000000u);
    a = sprite_checked_80012594((sint64)(sint32)p + 40);
    w_u32(bucket, p & 0xffffff);
    w_u32(0x8008d4bc, 0x2d000000);
    w_u32(0x8008d4b0, a);
    return 0x8008d4b0;
}

/* Original GAME.EXE:80061B7C..80061C68. Signed glyph widths and font table LH. */
static sint32 text_draw_80061B7C(uint32 text, const char *native_text, sint32 x, sint32 y)
{
    sint32 position[3] = {x, y, 0};
    uint8 character = native_text ? (uint8)*native_text : r_u8(text);
    if (!character)
        return (sint32)((uint32)position[0] + 10);
    do
    {
        sint32 ch = (sint8)character;
        if (native_text)
            native_text++;
        else
            text++;
        if (ch == 32)
            position[0] = (sint32)((uint32)position[0] + 6u);
        else
        {
            sint32 glyph = FUN_80018670(ch), font = r_s16(0x800af4a8);
            FUN_80012390(position, (sint32)((uint32)glyph + (sint32)r_s16(0x8009acc8u + (uint32)font * 2u)));
            position[0] = (sint32)((uint32)position[0] + (sint32)r_s8(0x800800d0u + (uint32)glyph));
        }
        character = native_text ? (uint8)*native_text : r_u8(text);
    } while (native_text ? (sint8)*native_text : r_s8(text));
    return (sint32)((uint32)position[0] + 10u);
}

GDB_CALL sint32 FUN_80061B7C(uint32 text, sint32 x, sint32 y)
{
    return text_draw_80061B7C(text, NULL, x, y);
}

/* Native text view for original stack-local strings; packets stay in guest RAM */
sint32 text_draw_host(const char *text, sint32 x, sint32 y)
{
    return text_draw_80061B7C(0, text, x, y);
}

sint32 text_center_host(const char *text, sint32 y)
{
    sint32 width = text_width_host(text);
    return text_draw_host(text, (sint32)(160 - (uint32)(width >> 1)), y);
}

/* Original GAME.EXE:80061ADC..80061B28. SRA width/2, not C signed division. */
GDB_CALL sint32 FUN_80061ADC(uint32 text, sint32 y)
{
    sint32 width = FUN_8005EFB8(text);
    return FUN_80061B7C(text, (sint32)(160u - (uint32)(width >> 1)), y);
}

/* GAME.EXE:80061C68. Logo font bank, fixed depth 16. */
GDB_CALL uint32 FUN_80061C68(sint32 x, sint32 y)
{
    sint32 position[3] = {x, y, 16};
    return FUN_80012390(position, r_s16(0x8009acc8u + (uint32)r_s16(0x800af4b4) * 2u));
}

/* GAME.EXE:80012A00. Descriptor end coordinates are inclusive sprite limits;
 * the polygon path deliberately does not add the sprite path's +1. */
GDB_CALL void FUN_80012A00(uint32 packet, sint32 sprite, sint32 x, sint32 y)
{
    uint32 descriptor = 0x800a8768u + (uint32)sprite * 12u;
    uint32 uv = r_u32(descriptor), mode = r_u32(descriptor + 4), end = r_u32(descriptor + 8);
    uint32 delta = ((end >> 16) - uv) & 65535u;
    uint16 right = (uint16)((uint32)x + (delta & 255)), bottom = (uint16)((uint32)y + (delta >> 8));
    w_u32(packet + 12, uv);
    w_u32(packet + 24, mode);
    w_u16(packet + 36, (uint16)end);
    w_u16(packet + 48, (uint16)(end >> 16));
    w_u16(packet + 8, (uint16)x);
    w_u16(packet + 10, (uint16)y);
    w_u16(packet + 20, right);
    w_u16(packet + 22, (uint16)y);
    w_u16(packet + 32, (uint16)x);
    w_u16(packet + 34, bottom);
    w_u16(packet + 44, right);
    w_u16(packet + 46, bottom);
}

/* GAME.EXE:80011E40. Preserve the original full pointer store to the OT;
 * GPU traversal masks to 24 bits. A host pointer must never enter this chain. */
GDB_CALL uint32 FUN_80011E40(uint32 bucket, uint32 packet)
{
    uint32 tag = (uint32)(sint32)r_s8(packet + 3) << 24;
    uint32 previous = r_u32(bucket);
    tag |= previous;
    w_u32(bucket, packet);
    w_u32(packet, tag);
    return previous;
}

/* GAME.EXE:80062C0C. SB then sign extension of color deltas is intentional.
 * SDK 8006E7D4 is the existing PsyQ setPolyGT4 macro (12, 0x3c);
 * its normalized twins differ in their packet length and command constants. */
static sint32 text_gradient_80062C0C(uint32 text, const char *native_text, sint32 x, sint32 y, uint32 colors, const uint8 *native_colors)
{
    sint32 width = native_text ? text_width_host(native_text) : FUN_8005EFB8(text);
    uint32 advance = 0;
    sint8 delta[6];
    uint8 previous[6], character;
    int i;
    if (!width)
        return 0;
    for (i = 0; i < 6; i++)
    {
        int k = i < 3 ? i : i + 3;
        uint8 right = native_colors ? native_colors[k + 3] : r_u8(colors + k + 3);
        previous[i] = native_colors ? native_colors[k] : r_u8(colors + k);
        delta[i] = (sint8)(right - previous[i]);
    }
    character = native_text ? (uint8)*native_text : r_u8(text);
    if (!character)
        return (sint32)((uint32)x + 170);
    do
    {
        sint32 ch = (sint8)character;
        if (native_text)
            native_text++;
        else
            text++;
        if (ch == 32)
            advance += 6;
        else
        {
            uint8 next[6];
            sint32 glyph = FUN_80018670(ch);
            uint32 packet = r_u32(0x8008d4b0);
            uint8 *p = (uint8 *)psx_addr(packet, 52);
            w_u32(0x8008d4b0, packet + 52);
            setPolyGT4(p);
            for (i = 0; i < 6; i++)
            {
                sint32 product = (sint32)((uint32)(sint32)delta[i] * advance);
                int k = i < 3 ? i : i + 3;
                /* Original BREAK 6 on signed division overflow */
                if (width == -1 && (uint32)product == 0x80000000u)
                    ff_wip_stop(__FUNCTION__, __FILE__, __LINE__);
                uint8 base = native_colors ? native_colors[k] : r_u8(colors + k);
                next[i] = (uint8)((uint32)base + (uint32)(product / width));
            }
            for (i = 0; i < 3; i++)
                w_u8(packet + 4 + i, (uint8)(previous[i] * 8u - 128u));
            for (i = 0; i < 3; i++)
                w_u8(packet + 16 + i, (uint8)(next[i] * 8u - 128u));
            for (i = 0; i < 3; i++)
                w_u8(packet + 28 + i, (uint8)(next[i + 3] * 8u - 128u));
            for (i = 0; i < 3; i++)
                w_u8(packet + 40 + i, (uint8)(previous[i + 3] * 8u - 128u));
            FUN_80012A00(packet, (sint32)((uint32)glyph + (sint32)r_s16(0x8009acc8u + (uint32)r_s16(0x800af4a8) * 2u)), (sint32)((uint32)x + advance), y);
            advance += (sint32)r_s8(0x800800d0u + (uint32)glyph);
            FUN_80011E40(r_u32(0x8008d4b4) + 8188, packet);
            for (i = 0; i < 6; i++)
                previous[i] = next[i];
        }
        character = native_text ? (uint8)*native_text : r_u8(text);
    } while (native_text ? (sint8)*native_text : r_s8(text));
    return (sint32)(advance + (uint32)x + 170u);
}

GDB_CALL sint32 FUN_80062C0C(uint32 text, sint32 x, sint32 y, uint32 colors)
{
    return text_gradient_80062C0C(text, NULL, x, y, colors, NULL);
}

sint32 text_draw_gradient_host(const char *text, sint32 x, sint32 y, uint32 colors)
{
    return text_gradient_80062C0C(0, text, x, y, colors, NULL);
}

sint32 ff_stack_text_gradient_80062C0C(uint32 text, const char *native_text, sint32 x, sint32 y, const uint8 colors[12])
{
    return text_gradient_80062C0C(text, native_text, x, y, 0, colors);
}

sint32 text_center_gradient_host(const char *text, sint32 y)
{
    sint32 width = text_width_host(text);
    return text_draw_gradient_host(text, (sint32)(160 - (uint32)(width >> 1)), y, 0x80093be4);
}

/* Numeric and caller-local variable-height FT4 share the original packet order */
static uint32 variable_height_sprite_800128F4(uint32 position, const uint32 native_position[4], sint32 sprite)
{
    uint32 p, d, uv, mode, end, color, bit, depth, height, bucket, ot, table;
    sint32 x, y;
    sint64 diff, bottom;
    uint32 right;
    p = r_u32(0x8008d4b0);
    x = native_position ? (sint16)native_position[0] : r_s16(position);
    y = native_position ? (sint16)native_position[1] : r_s16(position + 4);
    d = 0x800a8768 + (uint32)sprite * 12;
    uv = r_u32(d);
    mode = r_u32(d + 4);
    end = r_u32(d + 8);
    color = r_u32(0x8008d4bc);
    bit = uv & 1;
    uv -= bit;
    diff = (sint64)(end >> 16) - (sint32)uv;
    if (diff < -2147483647LL - 1 || diff > 2147483647LL)
        ff_wip_stop(__FUNCTION__, __FILE__, __LINE__);
    w_u32(p + 12, uv);
    w_u32(p + 20, mode);
    w_u16(p + 28, (uint16)end);
    w_u16(p + 36, (uint16)(end >> 16));
    w_u32(p + 4, color | (bit << 25));
    depth = native_position ? native_position[2] : r_u32(position + 8);
    height = native_position ? native_position[3] : r_u32(position + 12);
    w_u16(p + 8, (uint16)x);
    w_u16(p + 24, (uint16)x);
    w_u16(p + 10, (uint16)y);
    w_u16(p + 18, (uint16)y);
    right = (uint32)x + ((uint32)diff & 255) + 1;
    bottom = (sint64)(sint32)height + y;
    if (bottom < -2147483647LL - 1 || bottom > 2147483647LL)
        ff_wip_stop(__FUNCTION__, __FILE__, __LINE__);
    w_u16(p + 16, (uint16)right);
    w_u16(p + 32, (uint16)right);
    w_u16(p + 26, (uint16)bottom);
    w_u16(p + 34, (uint16)bottom);
    ot = r_u32(0x8008d4b4);
    table = r_u32(0x8008d4d8);
    bucket = ot + (sint32)r_s16(table + ((uint32)((sint32)depth >> 2) << 1));
    w_u32(p, r_u32(bucket) | 0x09000000);
    w_u32(bucket, p & 0xffffff);
    w_u32(0x8008d4bc, 0x2d000000);
    w_u32(0x8008d4b0, p + 40);
    return 0x8008d4b0;
}

GDB_CALL uint32 FUN_800128F4(uint32 position, sint32 sprite)
{
    return variable_height_sprite_800128F4(position, NULL, sprite);
}

GDB_CALL uint32 ff_stack_height_sprite_800128F4(const uint32 position[4], sint32 sprite)
{
    return variable_height_sprite_800128F4(0, position, sprite);
}

/* 18DCC scans CRLF text in place; NUL does not advance the global cursor. */
uint32 FUN_80018DCC(void)
{
    uint32 p = r_u32(0x80093cfc), next = p + 1;
    while (r_u8(p))
    {
        w_u32(0x80093cfc, next);
        if (r_u8(p) == 13)
        {
            w_u8(p, 0);
            p = r_u32(0x80093cfc);
            next = p + 1;
            w_u32(0x80093cfc, next);
            w_u8(p, 0);
            return next;
        }
        p = r_u32(0x80093cfc);
        next = p + 1;
    }
    return next;
}

uint32 FUN_80018E50(void)
{
    static const uint32 head[] = {0x8009243c, 0x80092440, 0x80092424, 0x80092428, 0x8009242c, 0x80092430, 0x80092434, 0x80092438, 0x80092444, 0x80092448, 0x8009244c, 0x80092464, 0x80092468, 0x8009246c, 0x80092470, 0x80092450, 0x80092454, 0x80092458, 0x8009245c, 0x80092460, 0x80092474, 0x80092478, 0x8009247c, 0x80092480, 0x80092484, 0x80092488, 0x8009248c, 0x80092490, 0x80092494, 0x80092498, 0x8009249c, 0x800924a0, 0x800924a4, 0x800924a8, 0x800924ac, 0x800924b0, 0x800924b4, 0x800924b8, 0x800924bc, 0x800924c0, 0x800924c4, 0x800924c8, 0x800924cc, 0x800924d0, 0x800924d4, 0x800924d8, 0x800924dc, 0x800924e0, 0x800924e4, 0x800924e8, 0x800924ec, 0x800924f0, 0x800924f4, 0x800924f8, 0x800924fc};
    static const uint32 tail[] = {0x80092500, 0, 0x80092504, 0, 0x80092508, 0, 0x8009250c, 0, 0x80092510, 0, 0x80092514, 0, 0x80094600, 0x80094610, 0, 0x80094614, 0, 0x80094618, 0, 0x80094604, 0x80094618, 0, 0x80094608, 0x80094620, 0x80092528, 0, 0x80092518, 0, 0x8009251c, 0, 0x80092520, 0, 0x80092524, 0, 0x8009252c, 0, 0x80092530, 0, 0x80092534, 0, 0x80092538, 0, 0x8009253c, 0, 0x80094458, 0, 0x8009445c, 0, 0x80094460, 0, 0x80094464, 0, 0x80094468, 0, 0x8009446c, 0};
    uint32 i;
    FUN_80061448(r_u32(0x80093d70) + 208, r_u32(0x80093cfc));
    for (i = 0; i < 15; i++)
    {
        w_u32(0x80094678 + 4 * i, r_u32(0x80093cfc));
        FUN_80018DCC();
    }
    for (i = 0; i < sizeof(head) / sizeof(head[0]); i++)
    {
        w_u32(head[i], r_u32(0x80093cfc));
        FUN_80018DCC();
    }
    for (i = 0; i < 3; i++)
    {
        w_u32(0x800947e0 + 4 * i, r_u32(0x80093cfc));
        FUN_80018DCC();
    }
    for (i = 0; i < sizeof(tail) / sizeof(tail[0]); i++)
        if (tail[i])
            w_u32(tail[i], r_u32(0x80093cfc));
        else
            FUN_80018DCC();
    w_u32(0x80093cfc, (r_u32(0x80093cfc) + 3) & 0xfffffffcu);
    return 0xfffffffcu;
}

/* 63100: left health bar. MIPS signed HP shifts; byte stores preserve
 * untouched packet padding. 6E7C0/6E798 reuse PsyQ G4/F4 constructors. */
GDB_CALL uint32 FUN_80063100(uint32 xy, uint32 actor)
{
    sint32 maximum = r_s16(actor + 72) >> 2, old = r_s16(actor + 70) >> 2;
    sint32 shown = r_s16(actor + 68) >> 2, hp = r_s16(actor + 66) >> 2;
    uint32 d = 0x800a8768u + 12u * (uint32)(sint32)r_s16(0x8009ad3eu + 2u * (uint32)(sint32)r_s16(0x800af4a8));
    uint32 x = xy & 65535, y = (uint32)((sint32)xy >> 16), p, bucket, previous, i;
    int wide = maximum > 128, special;
    uint8 *bytes;
    sint32 delta, fade;
    if (wide)
    {
        maximum >>= 1;
        old >>= 1;
        shown >>= 1;
        hp >>= 1;
    }
    p = r_u32(0x8008d4b0);
    w_u32(0x8008d4b0, p + 36);
    bytes = psx_addr(p, 36);
    setPolyG4(bytes);
    special = r_s16(actor + 52) == 53;
    for (i = 0; i < 4; i++)
    {
        bytes[4 + 8 * i] = i < 2 ? (wide ? 0 : 128) : 255;
        bytes[5 + 8 * i] = i < 2 ? (wide ? 128 : 0) : (special ? 128 : 255);
        bytes[6 + 8 * i] = i < 2 ? (special ? 128 : 0) : (special ? 255 : 128);
        w_u16(p + 8 + 8 * i, (uint16)(x + (i < 2 ? 0 : (uint32)shown)));
        w_u16(p + 10 + 8 * i, (uint16)(y + 2 + ((i & 1) ? 4 : 0)));
    }
    FUN_80011E40(r_u32(0x8008d4b4) + 8188, p);
    delta = old - hp;
    if (delta < 0)
        delta = -delta;
    if (delta > 0)
    {
        fade = 255 * (shown - hp);
        if (fade < 0)
            fade = -fade;
        fade /= delta;
        p = r_u32(0x8008d4b0);
        w_u32(0x8008d4b0, p + 24);
        bytes = psx_addr(p, 24);
        setPolyF4(bytes);
        bytes[4] = (uint8)fade;
        bytes[5] = bytes[6] = 0;
        for (i = 0; i < 4; i++)
        {
            w_u16(p + 8 + 4 * i, (uint16)(x + (uint32)(i < 2 ? shown : old)));
            w_u16(p + 10 + 4 * i, (uint16)(y + 2 + ((i & 1) ? 4 : 0)));
        }
        FUN_80011E40(r_u32(0x8008d4b4) + 8188, p);
    }
    p = r_u32(0x8008d4b0);
    w_u32(0x8008d4b0, p + 40);
    w_u32(p + 4, 0x2d808080);
    w_u32(p + 12, r_u32(d) & ~1u);
    w_u32(p + 20, r_u32(d + 4));
    w_u16(p + 28, r_s16(d + 8));
    w_u16(p + 36, r_s16(d + 10));
    for (i = 0; i < 4; i++)
    {
        w_u16(p + 8 + 8 * i, (uint16)(x + ((i & 1) ? (uint32)maximum : 0)));
        w_u16(p + 10 + 8 * i, (uint16)(y + (i < 2 ? 0 : 11)));
    }
    /* 11E28: full PSX pointer in bucket, OR preceding link with DMA length. */
    bucket = r_u32(0x8008d4b4) + 8188;
    previous = r_u32(bucket);
    w_u32(bucket, p);
    w_u32(p, previous | 0x09000000);
    return previous;
}

/* 61D0C: score digits use signed kerning indexed by the next low nibble.
 * Unlike 61B7C, the original returns zero, including an empty string. */
GDB_CALL uint32 FUN_80061D0C(uint32 text, sint32 x, sint32 y)
{
    sint32 position[3] = {x, y, 0};
    while (r_u8(text))
    {
        sint32 digit = r_s8(text++);
        if (digit == 32)
            position[0] = (sint32)((uint32)position[0] + 6u);
        else
        {
            digit -= 48;
            FUN_80012390(position, digit + r_s16(0x8009ad14u + 2u * (uint32)(sint32)r_s16(0x800af4a8)));
            position[0] = (sint32)((uint32)position[0] + (sint32)r_s8(0x80080140u + 10u * (uint32)digit + (r_u8(text) & 15)));
        }
    }
    return 0;
}

/* 61E0C is a two-byte decimal formatter, not general sprintf. Negative
 * arguments follow the one-character branch; preserve full 32-bit return. */
GDB_CALL uint32 FUN_80061E0C(sint32 value, uint32 output)
{
    uint32 result = (uint32)value + 48u;
    if (value < 10)
    {
        w_u8(output, (uint8)result);
        w_u8(output + 1, 0);
    }
    else
    {
        w_u8(output, (uint8)(value / 10 + 48));
        result = (uint32)(value % 10) + 48;
        w_u8(output + 1, (uint8)result);
        w_u8(output + 2, 0);
    }
    return result;
}

/* 6351C: right bar, same fill arithmetic as 63100, reversed X. Border
 * is a run of sprites, not the left bar FT4; preserve last-call return. */
GDB_CALL uint32 FUN_8006351C(uint32 xy, uint32 actor)
{
    sint32 maximum = r_s16(actor + 72) >> 2, old = r_s16(actor + 70) >> 2;
    sint32 shown = r_s16(actor + 68) >> 2, hp = r_s16(actor + 66) >> 2;
    uint32 x = xy & 65535, y = (uint32)((sint32)xy >> 16), p, result, i;
    sint32 position[3] = {(sint32)x, (sint32)y, 0};
    int wide = maximum > 128, special;
    uint8 *bytes;
    sint32 delta, fade;
    if (wide)
    {
        maximum >>= 1;
        old >>= 1;
        shown >>= 1;
        hp >>= 1;
    }
    p = r_u32(0x8008d4b0);
    w_u32(0x8008d4b0, p + 36);
    bytes = psx_addr(p, 36);
    setPolyG4(bytes);
    special = r_s16(actor + 52) == 53;
    for (i = 0; i < 4; i++)
    {
        bytes[4 + 8 * i] = i < 2 ? (wide ? 0 : 128) : 255;
        bytes[5 + 8 * i] = i < 2 ? (wide ? 128 : 0) : (special ? 128 : 255);
        bytes[6 + 8 * i] = i < 2 ? (special ? 128 : 0) : (special ? 255 : 128);
        w_u16(p + 8 + 8 * i, (uint16)(x - (i < 2 ? 0 : (uint32)shown)));
        w_u16(p + 10 + 8 * i, (uint16)(y + 2 + ((i & 1) ? 4 : 0)));
    }
    result = FUN_80011E40(r_u32(0x8008d4b4) + 8188, p);
    delta = old - hp;
    if (delta < 0)
        delta = -delta;
    if (delta > 0)
    {
        fade = 255 * (shown - hp);
        if (fade < 0)
            fade = -fade;
        fade /= delta;
        p = r_u32(0x8008d4b0);
        w_u32(0x8008d4b0, p + 24);
        bytes = psx_addr(p, 24);
        setPolyF4(bytes);
        bytes[4] = (uint8)fade;
        bytes[5] = bytes[6] = 0;
        for (i = 0; i < 4; i++)
        {
            w_u16(p + 8 + 4 * i, (uint16)(x - (uint32)(i < 2 ? shown : old)));
            w_u16(p + 10 + 4 * i, (uint16)(y + 2 + ((i & 1) ? 4 : 0)));
        }
        result = FUN_80011E40(r_u32(0x8008d4b4) + 8188, p);
    }
    for (i = (uint32)(maximum >> 3); (sint32)i > 0; i--)
    {
        position[0] = (sint32)((uint32)position[0] - 8u);
        result = FUN_80012390(position, r_s16(0x8009ad3eu + 2u * (uint32)(sint32)r_s16(0x800af4a8)));
    }
    return result;
}

/* 61B28: centered text, arithmetic width/2, fixed palette93BE4. */
GDB_CALL sint32 FUN_80061B28(uint32 text, sint32 y)
{
    FUNCTION_MARKER(0x80061b28u, "GAME.EXE");
    sint32 width = FUN_8005EFB8(text);
    return FUN_80062C0C(text, (sint32)(160u - (uint32)(width >> 1)), y, 0x80093be4);
}
