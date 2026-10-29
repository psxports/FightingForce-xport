#include "game.h"

sint16 depth_table[DEPTH_TABLE_COUNT];

/* Positive finite float-bit helpers retain the reviewed software rounding */
/* Positive finite software-float paths used only by6A50C. These preserve
 * the original SDK rounding; they are not general IEEE754 replacements. */
static uint32 divide_bits(uint32 dividend, uint32 divisor)
{
    sint32 exponent;
    uint32 numerator, denominator, bit, quotient = 0;
    XASSERT(!((dividend | divisor) & 0x80000000u) && divisor);
    if (!dividend)
        return 0;
    exponent = (sint32)((dividend >> 23) & 255) - (sint32)((divisor >> 23) & 255) + 126;
    numerator = (dividend & 0x7fffff) | 0x800000;
    denominator = (divisor & 0x7fffff) | 0x800000;
    if (numerator < denominator)
    {
        numerator <<= 1;
        --exponent;
    }
    for (bit = 0x1000000; bit; bit >>= 1)
    {
        if (numerator >= denominator)
        {
            quotient |= bit;
            numerator -= denominator;
        }
        numerator <<= 1;
    }
    XASSERT(exponent >= 0 && exponent < 254);
    return ((uint32)(exponent + 1) << 23) | (((quotient + 1) >> 1) & 0xff7fffff);
}

static uint32 add_bits(uint32 left, uint32 right)
{
    uint32 ea = (left >> 23) & 255, eb = (right >> 23) & 255, x, y, sum;
    XASSERT(!((left | right) & 0x80000000u));
    if (!left)
        return right;
    if (!right)
        return left;
    if (eb + 25 < ea)
        return left;
    if (ea + 25 < eb)
        return right;
    x = ((left & 0x7fffff) | 0x800000) << 6;
    y = ((right & 0x7fffff) | 0x800000) << 6;
    if (eb < ea)
        y >>= ea - eb;
    else
    {
        x >>= eb - ea;
        ea = eb;
    }
    sum = x + y;
    while (!(sum & 0xe0000000))
    {
        sum <<= 1;
        --ea;
    }
    if (sum & 0x40000000)
    {
        sum >>= 1;
        ++ea;
    }
    sum += (sum & 0x40) ? 32 : 31;
    if (sum & 0x40000000)
    {
        sum >>= 1;
        ++ea;
    }
    XASSERT(ea < 255);
    return (ea << 23) | ((sum >> 6) & 0xff7fffff);
}

static uint32 int_to_bits(uint32 val)
{
    uint32 exponent = 127, top = val;
    if (!val)
        return 0;
    XASSERT(val <= 8191);
    while (top >> 1)
    {
        top >>= 1;
        ++exponent;
    }
    return (exponent << 23) | ((val << (150 - exponent)) & 0x7fffff);
}

static uint32 bits_to_int(uint32 bits)
{
    uint32 exponent = (bits >> 23) & 255;
    if (!bits || exponent < 126)
        return 0;
    XASSERT(exponent <= 157 && !(bits & 0x80000000u));
    return (((bits & 0x7fffff) | 0x800000) << 7) >> (157 - exponent);
}

uint32 depth_table_build(void)
{
    FUNCTION_MARKER(0x8006a50c, "GAME.EXE");
    uint32 i, state = 0x3f800000;
    for (i = 0; i < DEPTH_TABLE_COUNT; i++)
    {
        depth_table[i] = (uint16)(bits_to_int(divide_bits(0x44ffe000, state)) << 2);
        state = add_bits(state, divide_bits(int_to_bits(i), 0x46000000));
    }
    depth_table[0] = 8188;
    return 8188;
}
