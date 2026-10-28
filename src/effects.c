#include <stdio.h>

#include "game.h"

enum
{
    PARTICLE_COLOR_COUNT = 16
};

static const sint16 hit_colors[PARTICLE_COLOR_COUNT] = {-32737, -32740, -32742, -32744, -32746, -32748, -32750, -32752, -32754, -32756, -32758, -32760, -32762, -32764, -32766, -32768};
static const sint16 alt_hit_colors[PARTICLE_COLOR_COUNT] = {-993, -4068, -6118, -8168, -10218, -12268, -14318, -16368, -18418, -20468, -22518, -24568, -26618, -28668, -30718, -32768};
static const sint16 german_hit_colors[PARTICLE_COLOR_COUNT] = {-1, -3172, -5286, -7400, -9514, -11628, -13742, -15856, -17970, -20084, -22198, -24312, -26426, -28540, -30654, -32768};
static const sint16 spark_colors[PARTICLE_COLOR_COUNT] = {-8457, -10571, -11628, -13742, -14799, -16913, -17970, -20084, -21141, -23255, -24312, -26426, -27483, -29597, -30654, -32768};
static const sint16 point_colors[PARTICLE_COLOR_COUNT] = {-16925, -17981, -19037, -20093, -21150, -22206, -23262, -24318, -25375, -26431, -27487, -28543, -29600, -30656, -31712, -32768};

uint32 effect_rotate_cursor;
EFFECT effects[EFFECT_COUNT];

/* Scan slots1..111, leaving slot0 and the rotating tail reserved */
GDB_CALL EFFECT *effect_alloc(void)
{
    FUNCTION_MARKER(0x80030f44, "GAME.EXE");
    uint32 i;
    for (i = EFFECT_NORMAL_FIRST; i < EFFECT_NORMAL_END; i++)
        if (effects[i].kind == 0xffffffffu)
        {
            EFFECT *effect = effects + i;
            *effect = (EFFECT){0};
            return effect;
        }
    return NULL;
}

/* Advance and clear one of the 16 rotating tail slots */
GDB_CALL EFFECT *effect_alloc_rotating(void)
{
    FUNCTION_MARKER(0x80030facu, "GAME.EXE");
    uint32 i = (effect_rotate_cursor + 1u) & (EFFECT_ROTATE_COUNT - 1u);
    EFFECT *effect;
    effect_rotate_cursor = i;
    effect = effects + EFFECT_COUNT - EFFECT_ROTATE_COUNT + i;
    *effect = (EFFECT){0};
    return effect;
}

/* Shared actual3100C/310B8 structure, constants retain each variant's RNG range */
static uint32 spawn_hit(uint32 x, uint32 y, uint32 z, uint32 angle, int reverse, int actor_kind, sint32 idx)
{
    EFFECT *effect = effect_alloc();
    uint32 v;
    EFFECT_PARTICLE *particle;
    if (!effect)
        return actor_kind ? (uint32)idx << 4 : 0;
    particle = &effect->particle;
    /* TODO: Investigate actor-dependent particles outside the native pool */
    if (actor_kind && (idx < 0 || idx >= ACTOR_COUNT))
    {
        DIAGNOSTIC(fprintf(stderr, "Invalid hit-particle actor %d\n", idx));
        diagnostic_stop(__FUNCTION__, __FILE__, __LINE__);
    }
    v = actor_kind ? actor_types[(uint32)idx] == 53 : 0;
    effect->kind = v;
    particle->pos.x.bits = x;
    particle->pos.y.bits = y;
    particle->pos.z.bits = z;
    v = angle + (reverse ? 1024u : 0u) + (random_next() & (reverse ? 511u : 255u)) - (reverse ? 256u : 128u);
    particle->yaw = (uint16)(v & 2047u);
    v = (random_next() & (reverse ? 15u : 31u)) + (reverse ? 12u : 16u);
    particle->vertical_speed = (uint16)v;
    v = (random_next() & (reverse ? 15u : 31u)) + (reverse ? 32u : 64u);
    particle->radial_speed = (uint16)v;
    particle->age = 0;
    effect->payload_kind = EFFECT_PAYLOAD_PARTICLE;
    return v;
}

GDB_CALL uint32 effect_spawn_hit(uint32 x, uint32 y, uint32 z, uint32 angle)
{
    FUNCTION_MARKER(0x8003100c, "GAME.EXE");
    return spawn_hit(x, y, z, angle, 0, 0, 0);
}

GDB_CALL uint32 effect_spawn_hit_reverse(uint32 x, uint32 y, uint32 z, uint32 angle)
{
    FUNCTION_MARKER(0x800310b8, "GAME.EXE");
    return spawn_hit(x, y, z, angle, 1, 0, 0);
}

/* 31164/3124C: actual fifth stack argument is a full word angle, not IDA a9.
 * Same particle fields/RNG as 3100C/310B8; actor type53 selects kind1.
 * Allocation failure returns the branch-delay result index<<4 without RNG */
GDB_CALL uint32 effect_spawn_actor_hit(sint32 idx, uint32 x, uint32 y, uint32 z, uint32 angle)
{
    FUNCTION_MARKER(0x80031164, "GAME.EXE");
    return spawn_hit(x, y, z, angle, 0, 1, idx);
}

GDB_CALL uint32 effect_spawn_actor_hit_reverse(sint32 idx, uint32 x, uint32 y, uint32 z, uint32 angle)
{
    FUNCTION_MARKER(0x8003124c, "GAME.EXE");
    return spawn_hit(x, y, z, angle, 1, 1, idx);
}

GDB_CALL uint32 effect_spawn_rotating_particle(uint32 x, uint32 y, uint32 z, uint32 angle)
{
    FUNCTION_MARKER(0x80031338u, "GAME.EXE");
    EFFECT *effect = effect_alloc();
    uint32 result;
    EFFECT_PARTICLE *particle;
    if (!effect)
        return 4;
    particle = &effect->particle;
    effect->kind = 4;
    particle->pos.x.bits = x;
    particle->pos.y.bits = y;
    particle->pos.z.bits = z;
    particle->yaw = (uint16)((angle + (random_next() & 255u) - 128u) & 2047u);
    particle->pitch = (uint16)(random_next() & 2047u);
    particle->roll = (uint16)(random_next() & 2047u);
    particle->vertical_speed = (uint16)(0u - (random_next() & 15u));
    result = (random_next() & 31u) + 32u;
    particle->radial_speed = (uint16)result;
    particle->age = 0;
    effect->payload_kind = EFFECT_PAYLOAD_PARTICLE;
    return result;
}

GDB_CALL sint32 effects_spawn_falling_glass(uint32 x, uint32 y, uint32 z)
{
    FUNCTION_MARKER(0x800313fcu, "GAME.EXE");
    uint32 i, angle, radius;
    EFFECT *effect;
    EFFECT_PARTICLE *particle;
    for (i = 0; i < 8; i++)
    {
        effect = effect_alloc();
        if (!effect)
            continue;
        particle = &effect->particle;
        effect->kind = 897;
        angle = (random_next() & 2047u) * 2;
        radius = random_next() & 255u;
        particle->pos.x.bits = x + (uint32)(sint32)sin_table[angle >> 1] * radius;
        particle->pos.z.bits = z - (uint32)(sint32)sin_table[MATH_QUARTER_TURN + (angle >> 1)] * radius;
        particle->pos.y.bits = y;
        particle->yaw = (uint16)random_next();
        particle->pitch = (uint16)random_next();
        particle->roll = (uint16)random_next();
        particle->vertical_speed = (uint16)(random_next() & 15u);
        effect->payload_kind = EFFECT_PAYLOAD_PARTICLE;
    }
    return 0;
}

GDB_CALL uint32 effect_spawn_spark(uint32 x, uint32 y, uint32 z)
{
    FUNCTION_MARKER(0x80031510u, "GAME.EXE");
    EFFECT *effect = effect_alloc();
    uint32 result = 6;
    EFFECT_PARTICLE *particle;
    if (!effect)
        return result;
    particle = &effect->particle;
    effect->kind = 6;
    particle->pos.x.bits = x;
    particle->pos.y.bits = y;
    particle->pos.z.bits = z;
    particle->yaw = (uint16)(random_next() & 2047u);
    particle->vertical_speed = (uint16)((random_next() & 15u) + 8u);
    result = (random_next() & 15u) + 16u;
    particle->radial_speed = (uint16)result;
    particle->age = 0;
    effect->payload_kind = EFFECT_PAYLOAD_PARTICLE;
    return result;
}

/* 315A4: fifth word argument is angle; return is the third RNG low six bits */
GDB_CALL uint32 effect_spawn_glass_fragment(uint32 kind, uint32 x, uint32 y, uint32 z, uint32 angle)
{
    FUNCTION_MARKER(0x800315a4u, "GAME.EXE");
    EFFECT *effect = effect_alloc();
    uint32 v;
    EFFECT_PARTICLE *particle;
    if (!effect)
        return 0;
    /* TODO: Investigate constructor kinds outside the native particle contract */
    if (!(kind - 897u < 13u || kind < 6u || kind - 6u < 2u || kind - 577u < 3u || kind == 583 || kind - 599u < 3u || kind - 72u < 4u))
    {
        DIAGNOSTIC(fprintf(stderr, "Invalid particle constructor kind %u\n", kind));
        diagnostic_stop(__FUNCTION__, __FILE__, __LINE__);
    }
    particle = &effect->particle;
    effect->kind = kind;
    particle->pos.x.bits = x;
    particle->pos.y.bits = y;
    particle->pos.z.bits = z;
    particle->yaw = (uint16)((angle + (random_next() & 255u) - 128u) & 2047u);
    particle->radial_speed = (uint16)((random_next() & 63u) + 128u);
    v = random_next() & 63u;
    particle->vertical_speed = (uint16)(64u - v);
    particle->age = 0;
    effect->payload_kind = EFFECT_PAYLOAD_PARTICLE;
    return v;
}

/* Radial RNG precedes vertical RNG; allocation failure returns 583 */
GDB_CALL uint32 effect_spawn_slow_fragment(uint32 x, uint32 y, uint32 z, uint32 angle)
{
    FUNCTION_MARKER(0x80031660u, "GAME.EXE");
    EFFECT *effect = effect_alloc();
    uint32 v;
    EFFECT_PARTICLE *particle = NULL;
    if (!effect)
        return 583;
    particle = &effect->particle;
    effect->kind = 583;
    particle->pos.x.bits = x;
    particle->pos.y.bits = y;
    particle->pos.z.bits = z;
    particle->yaw = (uint16)((angle + (random_next() & 511u) - 256u) & 2047u);
    particle->radial_speed = (uint16)((random_next() & 31u) + 32u);
    v = random_next() & 31u;
    particle->vertical_speed = (uint16)(32u - v);
    particle->age = 0;
    effect->payload_kind = EFFECT_PAYLOAD_PARTICLE;
    return v;
}

GDB_CALL uint32 effect_spawn_gate_fragment(uint32 x, uint32 y, uint32 z, uint32 angle)
{
    FUNCTION_MARKER(0x80031710u, "GAME.EXE");
    EFFECT *effect = effect_alloc();
    uint32 result;
    EFFECT_PARTICLE *particle = NULL;
    if (!effect)
        return 0;
    particle = &effect->particle;
    effect->kind = (random_next() & 3u) + 72u;
    particle->pos.x.bits = x;
    particle->pos.y.bits = y;
    particle->pos.z.bits = z;
    particle->yaw = (uint16)((angle + (random_next() & 511u) - 256u) & 2047u);
    particle->radial_speed = (uint16)((random_next() & 63u) + 64u);
    result = random_next() & 63u;
    particle->vertical_speed = (uint16)(64u - result);
    particle->age = 0;
    effect->payload_kind = EFFECT_PAYLOAD_PARTICLE;
    return result;
}

GDB_CALL uint32 effect_spawn_at_attachment(sint32 idx)
{
    FUNCTION_MARKER(0x800317d0u, "GAME.EXE");
    EFFECT *effect = effect_alloc();
    uint32 result = (uint32)idx << 4;
    EFFECT_PARTICLE *particle;
    if (!effect)
        return result;
    particle = &effect->particle;
    /* TODO: Investigate attachment particles with invalid actor indices */
    if (idx < 0 || idx >= ACTOR_COUNT)
    {
        DIAGNOSTIC(fprintf(stderr, "Invalid attachment-particle actor %d\n", idx));
        diagnostic_stop(__FUNCTION__, __FILE__, __LINE__);
    }
    effect->kind = actor_types[idx] == 53;
    effect->payload_kind = EFFECT_PAYLOAD_PARTICLE;
    actor_place_attachment(&particle->pos, idx, 1);
    /* TODO: Investigate cross-format reuse during attachment placement */
    if (effect->payload_kind != EFFECT_PAYLOAD_PARTICLE)
    {
        DIAGNOSTIC(fprintf(stderr, "Particle payload changed during attachment placement %p\n", (void *)effect));
        diagnostic_stop(__FUNCTION__, __FILE__, __LINE__);
    }
    particle->yaw = (uint16)actor_orientation[idx].yaw & 2047u;
    result = (random_next() & 15u) + 12u;
    particle->vertical_speed = (uint16)result;
    particle->radial_speed = 0;
    particle->age = 0;
    effect->payload_kind = EFFECT_PAYLOAD_PARTICLE;
    return result;
}

/* 31894: reverse-facing kind6, three RNG calls, then signed speed halving */
GDB_CALL uint32 effect_spawn_spark_reverse(uint32 x, uint32 y, uint32 z, uint32 angle)
{
    FUNCTION_MARKER(0x80031894u, "GAME.EXE");
    EFFECT *effect = effect_alloc();
    uint32 v;
    EFFECT_PARTICLE *particle;
    if (!effect)
        return 6;
    particle = &effect->particle;
    effect->kind = 6;
    particle->pos.x.bits = x;
    particle->pos.y.bits = y;
    particle->pos.z.bits = z;
    particle->yaw = (uint16)((angle + 1024u + (random_next() & 511u) - 256u) & 2047u);
    particle->vertical_speed = (uint16)((random_next() & 31u) + 16u);
    particle->radial_speed = (uint16)((random_next() & 31u) + 64u);
    particle->vertical_speed = (uint16)((sint16)particle->vertical_speed >> 1);
    v = (uint32)(sint32)(particle->radial_speed >> 1);
    particle->radial_speed = (uint16)v;
    particle->age = 0;
    effect->payload_kind = EFFECT_PAYLOAD_PARTICLE;
    return v;
}

GDB_CALL uint32 effect_spawn_point(uint32 x, uint32 y, uint32 z, uint32 angle)
{
    FUNCTION_MARKER(0x80031968u, "GAME.EXE");
    EFFECT *effect = effect_alloc();
    uint32 result;
    EFFECT_PARTICLE *particle;
    if (!effect)
        return 7;
    particle = &effect->particle;
    effect->kind = 7;
    particle->pos.x.bits = x;
    particle->pos.y.bits = y;
    particle->pos.z.bits = z;
    particle->yaw = (uint16)angle;
    particle->vertical_speed = (uint16)((random_next() & 31u) + 16u);
    particle->radial_speed = (uint16)((random_next() & 31u) + 64u);
    particle->vertical_speed = (uint16)((sint16)particle->vertical_speed >> 1);
    result = particle->radial_speed >> 1;
    particle->radial_speed = (uint16)result;
    particle->age = 0;
    effect->payload_kind = EFFECT_PAYLOAD_PARTICLE;
    return result;
}

/* 31A28: attachment-dependent particle in the rotating pool */
GDB_CALL uint32 effect_spawn_attachment_particle(sint32 idx, uint32 kind)
{
    FUNCTION_MARKER(0x80031a28u, "GAME.EXE");
    EFFECT *effect = effect_alloc_rotating();
    uint32 bias, v;
    EFFECT_PARTICLE *particle;
    /* TODO: Investigate unsupported attachment kinds */
    if (kind - 577u >= 2u)
    {
        DIAGNOSTIC(fprintf(stderr, "Invalid attachment particle %p kind %u\n", (void *)effect, kind));
        diagnostic_stop(__FUNCTION__, __FILE__, __LINE__);
    }
    particle = &effect->particle;
    effect->kind = kind;
    /* TODO: Investigate original actor reads outside the native pool */
    if (idx < 0 || idx >= ACTOR_COUNT)
    {
        DIAGNOSTIC(fprintf(stderr, "Invalid attachment-particle actor %d\n", idx));
        diagnostic_stop(__FUNCTION__, __FILE__, __LINE__);
    }
    effect->payload_kind = EFFECT_PAYLOAD_PARTICLE;
    if (actor_types[idx] == 2)
    {
        actor_place_attachment(&particle->pos, idx, 3);
        bias = 0xfffffdc0u;
    }
    else
    {
        actor_place_attachment(&particle->pos, idx, 2);
        bias = 576;
    }
    /* TODO: Investigate cross-format reuse during attachment-particle callbacks */
    if (effect->payload_kind != EFFECT_PAYLOAD_PARTICLE)
    {
        DIAGNOSTIC(fprintf(stderr, "Attachment particle payload changed %p\n", (void *)effect));
        diagnostic_stop(__FUNCTION__, __FILE__, __LINE__);
    }
    particle->pos.y.bits = particle->pos.y.bits - 0x100000u;
    v = random_next() & 127u;
    particle->yaw = (uint16)((((uint16)actor_orientation[idx].yaw - v + bias) & 2047u) << 5);
    particle->vertical_speed = (uint16)((random_next() & 7u) + 32u);
    v = (random_next() & 7u) + 64u;
    particle->radial_speed = (uint16)v;
    particle->age = 0;
    effect->payload_kind = EFFECT_PAYLOAD_PARTICLE;
    return v;
}

/* 31B20: seven actual MIPS arguments; Hex-Rays draft stack arguments are shifted */
GDB_CALL EFFECT *effect_spawn_timed(uint32 kind, uint32 x, uint32 y, uint32 z, sint32 age, sint32 a, sint32 b)
{
    FUNCTION_MARKER(0x80031b20u, "GAME.EXE");
    EFFECT *effect = effect_alloc();
    uint32 v;
    EFFECT_SPRITE *spr = NULL;
    EFFECT_PARTICLE *particle = NULL;
    EFFECT_LINE *line = NULL;
    EFFECT_RIBBON *ribbon = NULL;
    if (!effect)
        return 0;
    *effect = (EFFECT){0};
    if (kind - 870u < 5u || kind == 877 || kind == 879 || kind == 880 || kind - 891u < 5u || kind - 575u < 2u || kind - 602u < 6u)
        spr = &effect->spr;
    else if (kind - 897u < 13u || kind < 6u || kind - 6u < 2u || kind - 577u < 3u || kind == 583 || kind - 599u < 3u || kind - 72u < 4u)
        particle = &effect->particle;
    else if (kind == 878)
        line = &effect->line;
    else if (kind == 875)
        ribbon = &effect->ribbon;
    else if (kind == 876)
        ribbon = &effect->moving_ribbon.ribbon;
    else
    {
        /* TODO: Establish the native payload contract for any other constructor kind */
        DIAGNOSTIC(fprintf(stderr, "Unsupported native effect constructor kind %u at %p\n", kind, (void *)effect));
        diagnostic_stop(__FUNCTION__, __FILE__, __LINE__);
    }
    effect->kind = kind;
    if (spr)
        spr->pos.x.bits = x;
    else if (particle)
        particle->pos.x.bits = x;
    else if (line)
        line->pos.x.bits = x;
    else if (ribbon)
        ribbon->pos.x.bits = x;
    if (spr)
        spr->pos.y.bits = y;
    else if (particle)
        particle->pos.y.bits = y;
    else if (line)
        line->pos.y.bits = y;
    else if (ribbon)
        ribbon->pos.y.bits = y;
    if (spr)
        spr->pos.z.bits = z;
    else if (particle)
        particle->pos.z.bits = z;
    else if (line)
        line->pos.z.bits = z;
    else if (ribbon)
        ribbon->pos.z.bits = z;
    if (spr)
        spr->motion.x = (uint16)a;
    else if (particle)
        particle->yaw = (uint16)a;
    else if (line)
        line->delta.x = (uint16)a;
    else if (ribbon)
        ribbon->delta.x = (uint16)a;
    if (spr)
        spr->motion.z = (uint16)b;
    else if (particle)
        particle->roll = (uint16)b;
    else if (line)
        line->delta.z = (uint16)b;
    else if (ribbon)
        ribbon->delta.z = (uint16)b;
    if (kind - 871u < 2u)
        v = (random_next() & 7u) + (uint32)age;
    else if (kind == 870)
        v = (random_next() & 3u) + (uint32)age;
    else if (kind == 879)
        v = age;
    else if (kind == 880)
        v = 0;
    else
    {
        v = (random_next() & 7u) + (uint32)age;
        if (spr)
            spr->frame = (uint16)v;
        else if (particle)
            particle->life = (uint16)v;
        else if (line)
            line->tex = (uint16)v;
        else if (ribbon)
            ribbon->frame = (uint16)v;
        if (spr)
            spr->rise = 216;
        else if (particle)
            particle->age = 216;
        else if (line)
            line->size1 = 216;
        if (spr)
            effect->payload_kind = EFFECT_PAYLOAD_SPRITE;
        if (particle)
            effect->payload_kind = EFFECT_PAYLOAD_PARTICLE;
        if (line)
            effect->payload_kind = EFFECT_PAYLOAD_LINE;
        if (ribbon)
            effect->payload_kind = kind == 875 ? EFFECT_PAYLOAD_RIBBON : EFFECT_PAYLOAD_MOVING_RIBBON;
        return effect;
    }
    if (spr)
        spr->frame = (uint16)v;
    else if (particle)
        particle->life = (uint16)v;
    else if (line)
        line->tex = (uint16)v;
    else if (ribbon)
        ribbon->frame = (uint16)v;
    if (spr)
        effect->payload_kind = EFFECT_PAYLOAD_SPRITE;
    if (particle)
        effect->payload_kind = EFFECT_PAYLOAD_PARTICLE;
    if (line)
        effect->payload_kind = EFFECT_PAYLOAD_LINE;
    if (ribbon)
        effect->payload_kind = kind == 875 ? EFFECT_PAYLOAD_RIBBON : EFFECT_PAYLOAD_MOVING_RIBBON;
    return effect;
}

/* 31C50: allocate and initialize one generic effect, including zero-fill */
GDB_CALL EFFECT *effect_spawn(uint32 kind, uint32 x, uint32 y, uint32 z)
{
    FUNCTION_MARKER(0x80031c50u, "GAME.EXE");
    EFFECT *effect = effect_alloc();
    EFFECT_SPRITE *spr = NULL;
    EFFECT_PARTICLE *particle = NULL;
    EFFECT_LINE *line = NULL;
    EFFECT_RIBBON *ribbon = NULL;
    if (!effect)
        return 0;
    *effect = (EFFECT){0};
    if (kind - 870u < 5u || kind == 877 || kind == 879 || kind == 880 || kind - 891u < 5u || kind - 575u < 2u || kind - 602u < 6u)
        spr = &effect->spr;
    else if (kind - 897u < 13u || kind < 6u || kind - 6u < 2u || kind - 577u < 3u || kind == 583 || kind - 599u < 3u || kind - 72u < 4u)
        particle = &effect->particle;
    else if (kind == 878)
        line = &effect->line;
    else if (kind == 875)
        ribbon = &effect->ribbon;
    else if (kind == 876)
        ribbon = &effect->moving_ribbon.ribbon;
    else
    {
        /* TODO: Establish the native payload contract for any other constructor kind */
        DIAGNOSTIC(fprintf(stderr, "Unsupported native effect constructor kind %u at %p\n", kind, (void *)effect));
        diagnostic_stop(__FUNCTION__, __FILE__, __LINE__);
    }
    effect->kind = kind;
    if (spr)
        spr->pos.x.bits = x;
    else if (particle)
        particle->pos.x.bits = x;
    else if (line)
        line->pos.x.bits = x;
    else if (ribbon)
        ribbon->pos.x.bits = x;
    if (spr)
        spr->pos.y.bits = y;
    else if (particle)
        particle->pos.y.bits = y;
    else if (line)
        line->pos.y.bits = y;
    else if (ribbon)
        ribbon->pos.y.bits = y;
    if (spr)
        spr->pos.z.bits = z;
    else if (particle)
        particle->pos.z.bits = z;
    else if (line)
        line->pos.z.bits = z;
    else if (ribbon)
        ribbon->pos.z.bits = z;
    if (spr)
        effect->payload_kind = EFFECT_PAYLOAD_SPRITE;
    if (particle)
        effect->payload_kind = EFFECT_PAYLOAD_PARTICLE;
    if (line)
        effect->payload_kind = EFFECT_PAYLOAD_LINE;
    if (ribbon)
        effect->payload_kind = kind == 875 ? EFFECT_PAYLOAD_RIBBON : EFFECT_PAYLOAD_MOVING_RIBBON;
    return effect;
}

/* 31CD8: five RNG draws and two real2F828 updates, age reset after each */
GDB_CALL intptr effect_spawn_debris(uint32 kind, uint32 x, uint32 y, uint32 z)
{
    FUNCTION_MARKER(0x80031cd8u, "GAME.EXE");
    EFFECT *effect = effect_alloc();
    uint32 v;
    EFFECT_PARTICLE *particle;
    intptr result;
    if (!effect)
        return 0;
    /* TODO: Investigate constructor kinds outside the native particle contract */
    if (!(kind - 897u < 13u || kind < 6u || kind - 6u < 2u || kind - 577u < 3u || kind == 583 || kind - 599u < 3u || kind - 72u < 4u))
    {
        DIAGNOSTIC(fprintf(stderr, "Invalid particle constructor kind %u\n", kind));
        diagnostic_stop(__FUNCTION__, __FILE__, __LINE__);
    }
    particle = &effect->particle;
    *particle = (EFFECT_PARTICLE){0};
    v = random_next();
    effect->kind = kind;
    particle->pos.x.bits = x;
    particle->pos.y.bits = y;
    particle->pos.z.bits = z;
    particle->yaw = (uint16)(v & 2047u);
    particle->pitch = (uint16)(random_next() << 1);
    particle->roll = (uint16)(random_next() << 1);
    particle->radial_speed = (uint16)((random_next() & 63u) + 80u);
    particle->vertical_speed = (uint16)((random_next() & 63u) + 80u);
    effect->payload_kind = EFFECT_PAYLOAD_PARTICLE;
    effect_update_debris(effect);
    /* TODO: Investigate original constructor writes after debris slot reuse */
    if (effect->payload_kind != EFFECT_PAYLOAD_PARTICLE)
    {
        DIAGNOSTIC(fprintf(stderr, "Debris payload changed during initial update %p\n", (void *)effect));
        diagnostic_stop(__FUNCTION__, __FILE__, __LINE__);
    }
    particle->age = 0;
    result = effect_update_debris(effect);
    /* TODO: Investigate original constructor writes after debris slot reuse */
    if (effect->payload_kind != EFFECT_PAYLOAD_PARTICLE)
    {
        DIAGNOSTIC(fprintf(stderr, "Debris payload changed during initial update %p\n", (void *)effect));
        diagnostic_stop(__FUNCTION__, __FILE__, __LINE__);
    }
    particle->age = 0;
    return result;
}

/* 31DA8: aimed variant, wider radial speed for kinds902..905 */
GDB_CALL intptr effect_spawn_debris_aimed(uint32 kind, uint32 x, uint32 y, uint32 z, uint32 angle)
{
    FUNCTION_MARKER(0x80031da8u, "GAME.EXE");
    EFFECT *effect = effect_alloc();
    uint32 v;
    EFFECT_PARTICLE *particle;
    intptr result;
    if (!effect)
        return 0;
    /* TODO: Investigate constructor kinds outside the native particle contract */
    if (!(kind - 897u < 13u || kind < 6u || kind - 6u < 2u || kind - 577u < 3u || kind == 583 || kind - 599u < 3u || kind - 72u < 4u))
    {
        DIAGNOSTIC(fprintf(stderr, "Invalid particle constructor kind %u\n", kind));
        diagnostic_stop(__FUNCTION__, __FILE__, __LINE__);
    }
    particle = &effect->particle;
    *particle = (EFFECT_PARTICLE){0};
    v = (angle - 256u + (random_next() & 511u)) & 2047u;
    effect->kind = kind;
    particle->pos.x.bits = x;
    particle->pos.y.bits = y;
    particle->pos.z.bits = z;
    particle->yaw = (uint16)v;
    particle->pitch = (uint16)(random_next() << 1);
    particle->roll = (uint16)(random_next() << 1);
    particle->radial_speed = (uint16)((random_next() & 63u) + (kind - 902u < 4u ? 160u : 80u));
    particle->vertical_speed = (uint16)((random_next() & 63u) + 80u);
    effect->payload_kind = EFFECT_PAYLOAD_PARTICLE;
    effect_update_debris(effect);
    /* TODO: Investigate original constructor writes after debris slot reuse */
    if (effect->payload_kind != EFFECT_PAYLOAD_PARTICLE)
    {
        DIAGNOSTIC(fprintf(stderr, "Debris payload changed during initial update %p\n", (void *)effect));
        diagnostic_stop(__FUNCTION__, __FILE__, __LINE__);
    }
    particle->age = 0;
    result = effect_update_debris(effect);
    /* TODO: Investigate original constructor writes after debris slot reuse */
    if (effect->payload_kind != EFFECT_PAYLOAD_PARTICLE)
    {
        DIAGNOSTIC(fprintf(stderr, "Debris payload changed during initial update %p\n", (void *)effect));
        diagnostic_stop(__FUNCTION__, __FILE__, __LINE__);
    }
    particle->age = 0;
    return result;
}

GDB_CALL EFFECT *effect_spawn_motion(uint32 kind, uint32 x, uint32 y, uint32 z, uint32 dx, uint32 dy, uint32 dz)
{
    FUNCTION_MARKER(0x8004141cu, "GAME.EXE");
    EFFECT *effect = effect_alloc();
    uint32 frame;
    EFFECT_SPRITE *spr = NULL;
    EFFECT_PARTICLE *particle = NULL;
    EFFECT_LINE *line = NULL;
    EFFECT_RIBBON *ribbon = NULL;
    if (!effect)
        return 0;
    *effect = (EFFECT){0};
    if (kind - 870u < 5u || kind == 877 || kind == 879 || kind == 880 || kind - 891u < 5u || kind - 575u < 2u || kind - 602u < 6u)
        spr = &effect->spr;
    else if (kind - 897u < 13u || kind < 6u || kind - 6u < 2u || kind - 577u < 3u || kind == 583 || kind - 599u < 3u || kind - 72u < 4u)
        particle = &effect->particle;
    else if (kind == 878)
        line = &effect->line;
    else if (kind == 875)
        ribbon = &effect->ribbon;
    else if (kind == 876)
        ribbon = &effect->moving_ribbon.ribbon;
    else
    {
        /* TODO: Establish the native payload contract for any other constructor kind */
        DIAGNOSTIC(fprintf(stderr, "Unsupported native effect constructor kind %u at %p\n", kind, (void *)effect));
        diagnostic_stop(__FUNCTION__, __FILE__, __LINE__);
    }
    random_next();
    frame = random_next() & (kind == 877 ? 1u : 7u);
    if (ribbon)
        ribbon->frame = frame;
    else if (spr)
        spr->frame = frame;
    else if (particle)
        particle->life = (uint16)frame;
    else if (line)
        line->tex = (uint16)frame;
    effect->kind = kind;
    if (ribbon)
    {
        ribbon->pos.x.bits = x;
        ribbon->pos.y.bits = y;
        ribbon->pos.z.bits = z;
        ribbon->delta.x = dx >> 16;
        ribbon->delta.y = dy >> 16;
        ribbon->delta.z = dz >> 16;
        effect->payload_kind = kind == 875 ? EFFECT_PAYLOAD_RIBBON : EFFECT_PAYLOAD_MOVING_RIBBON;
    }
    else if (spr)
    {
        spr->pos.x.bits = x;
        spr->pos.y.bits = y;
        spr->pos.z.bits = z;
        spr->motion.x = dx >> 16;
        spr->motion.y = dy >> 16;
        spr->motion.z = dz >> 16;
        effect->payload_kind = EFFECT_PAYLOAD_SPRITE;
    }
    else if (particle)
    {
        particle->pos.x.bits = x;
        particle->pos.y.bits = y;
        particle->pos.z.bits = z;
        particle->yaw = dx >> 16;
        particle->pitch = dy >> 16;
        particle->roll = dz >> 16;
        effect->payload_kind = EFFECT_PAYLOAD_PARTICLE;
    }
    else if (line)
    {
        line->pos.x.bits = x;
        line->pos.y.bits = y;
        line->pos.z.bits = z;
        line->delta.x = dx >> 16;
        line->delta.y = dy >> 16;
        line->delta.z = dz >> 16;
    }
    if (line)
        effect->payload_kind = EFFECT_PAYLOAD_LINE;
    return effect;
}

/* 4150C: nine actual arguments, not the thirteen inferred by IDA */
GDB_CALL sint32 effect_spawn_line(uint32 style, uint32 x, uint32 y, uint32 z, uint32 end_x, uint32 end_y, uint32 end_z, uint32 size, uint32 color)
{
    FUNCTION_MARKER(0x8004150cu, "GAME.EXE");
    EFFECT *effect = effect_alloc();
    EFFECT_LINE *line;
    sint32 dz;
    if (!effect)
        return 0;
    line = &effect->line;
    *line = (EFFECT_LINE){0};
    effect->kind = 878;
    line->tex = style;
    line->size_bits = size;
    line->color.rgbc = color;
    line->pos.x.bits = x;
    line->pos.y.bits = y;
    line->pos.z.bits = z;
    line->delta.x = (sint32)(end_x - x) >> 16;
    line->delta.y = (sint32)(end_y - y) >> 16;
    dz = (sint32)(end_z - z) >> 16;
    line->delta.z = dz;
    effect->payload_kind = EFFECT_PAYLOAD_LINE;
    return dz;
}

/* Spawn with the original yaw jitter and Q15 velocity products */
GDB_CALL sint32 effect_spawn_velocity(uint32 kind, uint32 x, uint32 y, uint32 z, sint32 yaw, sint32 pitch, sint32 speed)
{
    FUNCTION_MARKER(0x800412a8u, "GAME.EXE");
    EFFECT *effect = effect_alloc();
    uint32 r, yaw_idx, pitch_idx;
    sint32 scale, result;
    EFFECT_SPRITE *spr = NULL;
    EFFECT_PARTICLE *particle = NULL;
    EFFECT_LINE *line = NULL;
    EFFECT_RIBBON *ribbon = NULL;
    if (!effect)
        return 0;
    *effect = (EFFECT){0};
    if (kind - 870u < 5u || kind == 877 || kind == 879 || kind == 880 || kind - 891u < 5u || kind - 575u < 2u || kind - 602u < 6u)
        spr = &effect->spr;
    else if (kind - 897u < 13u || kind < 6u || kind - 6u < 2u || kind - 577u < 3u || kind == 583 || kind - 599u < 3u || kind - 72u < 4u)
        particle = &effect->particle;
    else if (kind == 878)
        line = &effect->line;
    else if (kind == 875)
        ribbon = &effect->ribbon;
    else if (kind == 876)
        ribbon = &effect->moving_ribbon.ribbon;
    else
    {
        /* TODO: Establish the native payload contract for any other constructor kind */
        DIAGNOSTIC(fprintf(stderr, "Unsupported native effect constructor kind %u at %p\n", kind, (void *)effect));
        diagnostic_stop(__FUNCTION__, __FILE__, __LINE__);
    }
    effect->kind = kind;
    if (spr)
        spr->pos.x.bits = x;
    else if (particle)
        particle->pos.x.bits = x;
    else if (line)
        line->pos.x.bits = x;
    else if (ribbon)
        ribbon->pos.x.bits = x;
    if (spr)
        spr->pos.y.bits = y;
    else if (particle)
        particle->pos.y.bits = y;
    else if (line)
        line->pos.y.bits = y;
    else if (ribbon)
        ribbon->pos.y.bits = y;
    if (spr)
        spr->pos.z.bits = z;
    else if (particle)
        particle->pos.z.bits = z;
    else if (line)
        line->pos.z.bits = z;
    else if (ribbon)
        ribbon->pos.z.bits = z;
    r = random_next();
    scale = (uint32)speed - 1u + (r & 3u);
    yaw_idx = ((sint32)((uint32)yaw + (uint32)((sint32)r >> 6)) >> 5) & 2047u;
    if (spr)
        spr->motion.x = (uint16)((sint32)((uint32)scale * (uint32)(sint32)sin_table[yaw_idx]) >> 15);
    else if (particle)
        particle->yaw = (uint16)((sint32)((uint32)scale * (uint32)(sint32)sin_table[yaw_idx]) >> 15);
    else if (line)
        line->delta.x = (uint16)((sint32)((uint32)scale * (uint32)(sint32)sin_table[yaw_idx]) >> 15);
    else if (ribbon)
        ribbon->delta.x = (uint16)((sint32)((uint32)scale * (uint32)(sint32)sin_table[yaw_idx]) >> 15);
    if (spr)
        spr->motion.z = (uint16)((sint32)((uint32)scale * (uint32)-sin_table[MATH_QUARTER_TURN + yaw_idx]) >> 15);
    else if (particle)
        particle->roll = (uint16)((sint32)((uint32)scale * (uint32)-sin_table[MATH_QUARTER_TURN + yaw_idx]) >> 15);
    else if (line)
        line->delta.z = (uint16)((sint32)((uint32)scale * (uint32)-sin_table[MATH_QUARTER_TURN + yaw_idx]) >> 15);
    else if (ribbon)
        ribbon->delta.z = (uint16)((sint32)((uint32)scale * (uint32)-sin_table[MATH_QUARTER_TURN + yaw_idx]) >> 15);
    pitch_idx = (uint32)(pitch >> 5) & 2047u;
    if (spr)
        spr->motion.y = (uint16)((sint32)((uint32)scale * (uint32)-sin_table[pitch_idx]) >> 15);
    else if (particle)
        particle->pitch = (uint16)((sint32)((uint32)scale * (uint32)-sin_table[pitch_idx]) >> 15);
    else if (line)
        line->delta.y = (uint16)((sint32)((uint32)scale * (uint32)-sin_table[pitch_idx]) >> 15);
    else if (ribbon)
        ribbon->delta.y = (uint16)((sint32)((uint32)scale * (uint32)-sin_table[pitch_idx]) >> 15);
    if (spr)
        spr->motion.x = (uint16)((sint32)((uint32)(sint32)spr->motion.x * (uint32)(sint32)sin_table[pitch_idx]) >> 15);
    else if (particle)
        particle->yaw = (uint16)((sint32)((uint32)(sint32)particle->yaw * (uint32)(sint32)sin_table[pitch_idx]) >> 15);
    else if (line)
        line->delta.x = (uint16)((sint32)((uint32)(sint32)line->delta.x * (uint32)(sint32)sin_table[pitch_idx]) >> 15);
    else if (ribbon)
        ribbon->delta.x = (uint16)((sint32)((uint32)(sint32)ribbon->delta.x * (uint32)(sint32)sin_table[pitch_idx]) >> 15);
    result = (sint32)((uint32)(sint32)(spr ? spr->motion.z : particle ? particle->roll : line ? line->delta.z : ribbon->delta.z) * (uint32)(sint32)sin_table[pitch_idx]) >> 15;
    if (spr)
        spr->motion.z = (uint16)result;
    else if (particle)
        particle->roll = (uint16)result;
    else if (line)
        line->delta.z = (uint16)result;
    else if (ribbon)
        ribbon->delta.z = (uint16)result;
    if (spr)
        effect->payload_kind = EFFECT_PAYLOAD_SPRITE;
    if (particle)
        effect->payload_kind = EFFECT_PAYLOAD_PARTICLE;
    if (line)
        effect->payload_kind = EFFECT_PAYLOAD_LINE;
    if (ribbon)
        effect->payload_kind = kind == 875 ? EFFECT_PAYLOAD_RIBBON : EFFECT_PAYLOAD_MOVING_RIBBON;
    return result;
}

/* Visit all 128 effect slots and skip absent update callbacks */
GDB_CALL sint32 effects_update(void)
{
    FUNCTION_MARKER(0x8001edb8, "GAME.EXE");
    uint32 i, type;
    const OBJECT_CALLBACK *target;
    for (i = 0; i < EFFECT_COUNT; i++)
    {
        type = effects[i].kind;
        if (type == 0xffffffff)
            continue;
        if (type >= MODEL_TYPE_COUNT)
            diagnostic_stop(__FUNCTION__, __FILE__, __LINE__);
        target = object_callbacks[OBJECT_CALLBACK_EFFECT_UPDATE][type];
        if (target)
            object_call(target, NULL, effects + i);
    }
    return 0;
}

/* Apply model height, then yaw, pitch and roll */
GDB_CALL void effect_draw_model(const struct EFFECT *effect)
{
    FUNCTION_MARKER(0x8005c66c, "GAME.EXE");
    uint32 x, y, z, camera_x, camera_y, camera_z, type, height;
    /* TODO: Investigate an unpublished or incompatible debris draw source */
    if (!effect || effect->payload_kind != EFFECT_PAYLOAD_PARTICLE)
    {
        DIAGNOSTIC(fprintf(stderr, "Invalid debris draw source %p\n", (const void *)effect));
        diagnostic_stop(__FUNCTION__, __FILE__, __LINE__);
    }
    const EFFECT_PARTICLE *debris = &effect->particle;
    transform_push();
    x = debris->pos.x.bits;
    camera_x = camera.pos.x.bits;
    type = effect->kind;
    /* TODO: Investigate missing publication or incompatible model callbacks */
    if (!(type - 897u < 13u || type < 6u || type - 6u < 2u || type - 577u < 3u || type == 583 || type - 599u < 3u || type - 72u < 4u) && type != 0xffffffffu)
    {
        DIAGNOSTIC(fprintf(stderr, "Invalid debris model publication %p kind %u\n", (const void *)effect, type));
        diagnostic_stop(__FUNCTION__, __FILE__, __LINE__);
    }
    y = debris->pos.y.bits;
    /* TODO: Investigate original model-height reads outside the native table */
    if (type >= MODEL_TYPE_COUNT)
    {
        DIAGNOSTIC(fprintf(stderr, "Invalid debris model-height index %u\n", type));
        diagnostic_stop(__FUNCTION__, __FILE__, __LINE__);
    }
    height = model_height[type];
    y += height;
    camera_y = camera.pos.y.bits;
    z = debris->pos.z.bits;
    camera_z = camera.pos.z.bits;
    transform_translate(x - camera_x, y - camera_y, z - camera_z);
    transform_pitch(32768);
    transform_yaw(debris->yaw);
    transform_pitch(debris->pitch);
    transform_roll(debris->roll);
    type = effect->kind;
    /* TODO: Investigate original late model-reference reads outside the native table */
    if (type >= MODEL_TYPE_COUNT)
    {
        DIAGNOSTIC(fprintf(stderr, "Invalid debris model-reference index %u\n", type));
        diagnostic_stop(__FUNCTION__, __FILE__, __LINE__);
    }
    model_draw_static(model_refs[type], 0, 0);
    transform_pop();
}

/* Visit all 128 effect slots and draw the default model for absent callbacks */
GDB_CALL sint32 effects_draw(void)
{
    FUNCTION_MARKER(0x8001ef68, "GAME.EXE");
    uint32 i, type;
    const OBJECT_CALLBACK *target;
    for (i = 0; i < EFFECT_COUNT; i++)
    {
        type = effects[i].kind;
        if (type == 0xffffffff)
            continue;
        if (type >= MODEL_TYPE_COUNT)
            diagnostic_stop(__FUNCTION__, __FILE__, __LINE__);
        target = object_callbacks[OBJECT_CALLBACK_EFFECT_DRAW][type];
        if (target)
            object_call(target, NULL, effects + i);
        else
            effect_draw_model(effects + i);
    }
    return 0;
}

/* Existing pair shares all operations except gravity and vertical shift */
static intptr update_debris(EFFECT *effect, uint32 gravity, uint32 vertical_shift)
{
    /* TODO: Investigate unpublished or cross-format debris callback sources */
    if (!effect || effect->payload_kind != EFFECT_PAYLOAD_PARTICLE || (!(effect->kind - 897u < 13u || effect->kind < 6u || effect->kind - 6u < 2u || effect->kind - 577u < 3u || effect->kind == 583 || effect->kind - 599u < 3u || effect->kind - 72u < 4u) && effect->kind != 0xffffffffu))
    {
        DIAGNOSTIC(fprintf(stderr, "Invalid native debris source %p stage %u tick %u\n", (void *)effect, game_session.stage, game_session.tick));
        diagnostic_stop(__FUNCTION__, __FILE__, __LINE__);
    }
    EFFECT_PARTICLE *debris = &effect->particle;
    uint32 val, x, y, z;
    sint32 speed, trig, yaw;
    speed = debris->radial_speed;
    yaw = debris->yaw;
    /* TODO: Investigate original adjacent trigonometric-table reads */
    if (yaw < 0 || yaw >= MATH_SIN_COUNT)
    {
        DIAGNOSTIC(fprintf(stderr, "Invalid debris trigonometric index %d for %p\n", yaw, (void *)effect));
        diagnostic_stop(__FUNCTION__, __FILE__, __LINE__);
    }
    trig = sin_table[yaw];
    x = debris->pos.x.bits;
    debris->pos.x.bits = x - (uint32)((speed * trig) >> 1);
    speed = debris->radial_speed;
    yaw = debris->yaw;
    /* TODO: Investigate original adjacent trigonometric-table reads */
    if (MATH_QUARTER_TURN + yaw < 0 || MATH_QUARTER_TURN + yaw >= MATH_SIN_COUNT)
    {
        DIAGNOSTIC(fprintf(stderr, "Invalid debris trigonometric index %d for %p\n", MATH_QUARTER_TURN + yaw, (void *)effect));
        diagnostic_stop(__FUNCTION__, __FILE__, __LINE__);
    }
    trig = sin_table[MATH_QUARTER_TURN + yaw];
    z = debris->pos.z.bits;
    debris->pos.z.bits = z + (uint32)((speed * trig) >> 1);
    debris->pitch = (uint16)((uint16)debris->pitch + 0x1180);
    debris->roll = (uint16)((uint16)debris->roll + 0x1c80);
    val = (uint32)(uint16)debris->vertical_speed - gravity;
    debris->vertical_speed = (uint16)val;
    y = debris->pos.y.bits - (uint32)((sint32)(val << 16) >> vertical_shift);
    debris->pos.y.bits = y;
    if ((sint32)y > 0)
    {
        effect->kind = 0xffffffffu;
        x = debris->pos.x.bits;
        z = debris->pos.z.bits;
        return (intptr)effect_spawn(894, x, 0, z);
    }
    val = (uint32)(uint16)debris->age + 1;
    debris->age = (uint16)val;
    if (val & 1u)
        return 1;
    x = debris->pos.x.bits;
    y = debris->pos.y.bits;
    z = debris->pos.z.bits;
    return (intptr)effect_spawn(605, x, y, z);
}

GDB_CALL intptr effect_update_debris_low_gravity(EFFECT *effect)
{
    FUNCTION_MARKER(0x8002f700u, "GAME.EXE");
    return update_debris(effect, 3, 2);
}

GDB_CALL intptr effect_update_debris(EFFECT *effect)
{
    FUNCTION_MARKER(0x8002f828u, "GAME.EXE");
    return update_debris(effect, 6, 1);
}

GDB_CALL intptr effect_update_debris_quarter_speed(EFFECT *effect)
{
    FUNCTION_MARKER(0x8002f950u, "GAME.EXE");
    /* TODO: Investigate unpublished or cross-format debris callback sources */
    if (!effect || effect->payload_kind != EFFECT_PAYLOAD_PARTICLE || (!(effect->kind - 897u < 13u || effect->kind < 6u || effect->kind - 6u < 2u || effect->kind - 577u < 3u || effect->kind == 583 || effect->kind - 599u < 3u || effect->kind - 72u < 4u) && effect->kind != 0xffffffffu))
    {
        DIAGNOSTIC(fprintf(stderr, "Invalid native debris source %p stage %u tick %u\n", (void *)effect, game_session.stage, game_session.tick));
        diagnostic_stop(__FUNCTION__, __FILE__, __LINE__);
    }
    EFFECT_PARTICLE *debris = &effect->particle;
    uint32 val, x, y, z;
    sint32 speed, trig, yaw;
    speed = debris->radial_speed;
    yaw = debris->yaw;
    /* TODO: Investigate original adjacent trigonometric-table reads */
    if (yaw < 0 || yaw >= MATH_SIN_COUNT)
    {
        DIAGNOSTIC(fprintf(stderr, "Invalid debris trigonometric index %d for %p\n", yaw, (void *)effect));
        diagnostic_stop(__FUNCTION__, __FILE__, __LINE__);
    }
    trig = sin_table[yaw];
    x = debris->pos.x.bits;
    debris->pos.x.bits = x - (uint32)((speed * trig) >> 3);
    speed = debris->radial_speed;
    yaw = debris->yaw;
    /* TODO: Investigate original adjacent trigonometric-table reads */
    if (MATH_QUARTER_TURN + yaw < 0 || MATH_QUARTER_TURN + yaw >= MATH_SIN_COUNT)
    {
        DIAGNOSTIC(fprintf(stderr, "Invalid debris trigonometric index %d for %p\n", MATH_QUARTER_TURN + yaw, (void *)effect));
        diagnostic_stop(__FUNCTION__, __FILE__, __LINE__);
    }
    trig = sin_table[MATH_QUARTER_TURN + yaw];
    z = debris->pos.z.bits;
    debris->pos.z.bits = z + (uint32)((speed * trig) >> 3);
    debris->pitch = (uint16)((uint16)debris->pitch + 4480);
    debris->roll = (uint16)((uint16)debris->roll + 7296);
    val = (uint32)(uint16)debris->vertical_speed - 6;
    debris->vertical_speed = (uint16)val;
    y = debris->pos.y.bits - (uint32)((sint32)(val << 16) >> 3);
    debris->pos.y.bits = y;
    if ((sint32)y > 0)
    {
        effect->kind = 0xffffffffu;
        x = debris->pos.x.bits;
        z = debris->pos.z.bits;
        return (intptr)effect_spawn(895, x, 0, z);
    }
    val = (uint32)(uint16)debris->age + 1;
    debris->age = (uint16)val;
    if (val & 1)
        return 1;
    x = debris->pos.x.bits;
    y = debris->pos.y.bits;
    z = debris->pos.z.bits;
    return (intptr)effect_spawn_timed(870, x, y, z, 0, 0, 0);
}

GDB_CALL void effect_draw_line(const struct EFFECT *effect)
{
    FUNCTION_MARKER(0x8005d024u, "GAME.EXE");
    PROJECTION_POINT a, b;
    uint32 color;
    const EFFECT_LINE *line;
    /* TODO: Investigate callbacks outside the line-effect pool contract */
    if (!effect || effect->kind != 878 || effect->payload_kind != EFFECT_PAYLOAD_LINE)
    {
        DIAGNOSTIC(fprintf(stderr, "Invalid line effect callback %p\n", (const void *)effect));
        diagnostic_stop(__FUNCTION__, __FILE__, __LINE__);
    }
    line = &effect->line;
    transform_push();
    transform_translate(line->pos.x.bits - camera.pos.x.bits, line->pos.y.bits - camera.pos.y.bits, line->pos.z.bits - camera.pos.z.bits);
    proj_local(0, 0, 0, &a);
    proj_local(line->delta.x, line->delta.y, line->delta.z, &b);
    a.depth = (sint32)((uint32)a.depth - 32u);
    b.depth = (sint32)((uint32)b.depth - 32u);
    color = line->color.rgbc;
    render_style.color.rgbc = color;
    render_style.blend_mode = (uint32)((sint32)color >> 24);
    render_style.color.rgbc = (color & 0xffffffu) | 0x2c000000u;
    a.size = line->size0;
    b.size = line->size1;
    /* TODO: Investigate original adjacent texture-map reads */
    if (line->tex < 0 || line->tex >= TEXTURE_MAP_COUNT)
    {
        DIAGNOSTIC(fprintf(stderr, "Invalid line effect texture %d\n", line->tex));
        diagnostic_stop(__FUNCTION__, __FILE__, __LINE__);
    }
    spr_draw_ribbon(&a, &b, tex_map[line->tex]);
    transform_pop();
}

GDB_CALL void effect_draw_ribbon(const struct EFFECT *effect)
{
    FUNCTION_MARKER(0x8005cdbcu, "GAME.EXE");
    PROJECTION_POINT a, b;
    const EFFECT_RIBBON *ribbon;
    /* TODO: Investigate callbacks outside the one-frame ribbon contract */
    if (!effect || effect->kind != 875 || effect->payload_kind != EFFECT_PAYLOAD_RIBBON)
    {
        DIAGNOSTIC(fprintf(stderr, "Invalid ribbon effect callback %p\n", (const void *)effect));
        diagnostic_stop(__FUNCTION__, __FILE__, __LINE__);
    }
    ribbon = &effect->ribbon;
    transform_push();
    transform_translate(ribbon->pos.x.bits - camera.pos.x.bits, ribbon->pos.y.bits - camera.pos.y.bits, ribbon->pos.z.bits - camera.pos.z.bits);
    proj_local(0, 0, 0, &a);
    proj_local(ribbon->delta.x, ribbon->delta.y, ribbon->delta.z, &b);
    render_style.color.rgbc = 0x2c303040;
    a.size = 128;
    b.size = 256;
    /* TODO: Investigate original adjacent texture-map reads */
    if (ribbon->frame < -106 || ribbon->frame >= TEXTURE_MAP_COUNT - 106)
    {
        DIAGNOSTIC(fprintf(stderr, "Invalid ribbon texture frame %d\n", ribbon->frame));
        diagnostic_stop(__FUNCTION__, __FILE__, __LINE__);
    }
    spr_draw_ribbon(&b, &a, tex_map[106 + ribbon->frame]);
    render_style.color.rgbc = 0x2c181820;
    a.size = 176;
    b.size = 336;
    /* TODO: Investigate original adjacent texture-map reads */
    if (ribbon->frame < -106 || ribbon->frame >= TEXTURE_MAP_COUNT - 106)
    {
        DIAGNOSTIC(fprintf(stderr, "Invalid ribbon texture frame %d\n", ribbon->frame));
        diagnostic_stop(__FUNCTION__, __FILE__, __LINE__);
    }
    spr_draw_ribbon(&b, &a, tex_map[106 + ribbon->frame]);
    transform_pop();
}

GDB_CALL uint32 effect_remove_line(struct EFFECT *effect)
{
    FUNCTION_MARKER(0x800415f4u, "GAME.EXE");
    /* TODO: Investigate a missing native removal target */
    if (!effect)
    {
        DIAGNOSTIC(fprintf(stderr, "Missing line/ribbon removal target in %s stage %u tick %u\n", __FUNCTION__, game_session.stage, game_session.tick));
        diagnostic_stop(__FUNCTION__, __FILE__, __LINE__);
    }
    effect->kind = 0xffffffffu;
    return 0xffffffffu;
}

GDB_CALL uint32 effect_remove_ribbon(struct EFFECT *effect)
{
    FUNCTION_MARKER(0x80041600u, "GAME.EXE");
    /* TODO: Investigate a missing native removal target */
    if (!effect)
    {
        DIAGNOSTIC(fprintf(stderr, "Missing line/ribbon removal target in %s stage %u tick %u\n", __FUNCTION__, game_session.stage, game_session.tick));
        diagnostic_stop(__FUNCTION__, __FILE__, __LINE__);
    }
    effect->kind = 0xffffffffu;
    return 0xffffffffu;
}

GDB_CALL sint32 effect_update_moving_ribbon(struct EFFECT *effect)
{
    FUNCTION_MARKER(0x8004160cu, "GAME.EXE");
    EFFECT_MOVING_RIBBON *moving;
    /* TODO: Investigate callbacks outside the moving-ribbon contract */
    if (!effect || effect->kind != 876 || effect->payload_kind != EFFECT_PAYLOAD_MOVING_RIBBON)
    {
        DIAGNOSTIC(fprintf(stderr, "Invalid moving ribbon update %p\n", (const void *)effect));
        diagnostic_stop(__FUNCTION__, __FILE__, __LINE__);
    }
    moving = &effect->moving_ribbon;
    moving->age = (uint16)(moving->age + 1);
    if (moving->age >= 5)
    {
        effect->kind = 0xffffffffu;
    }
    moving->ribbon.pos.x.bits += (uint32)(sint32)moving->ribbon.delta.x << 14;
    moving->ribbon.pos.y.bits += (uint32)(sint32)moving->ribbon.delta.y << 14;
    moving->ribbon.pos.z.bits += (uint32)(sint32)moving->ribbon.delta.z << 14;
    if (moving->age >= 4)
        actor_projectile_impact(moving->ribbon.pos.x.bits, moving->ribbon.pos.z.bits, NULL, effect);
    /* TODO: Investigate cross-variant reuse after the original impact call */
    if (effect->payload_kind != EFFECT_PAYLOAD_MOVING_RIBBON || (effect->kind != 876 && effect->kind != 0xffffffffu))
    {
        DIAGNOSTIC(fprintf(stderr, "Moving ribbon payload changed during impact %p\n", (const void *)effect));
        diagnostic_stop(__FUNCTION__, __FILE__, __LINE__);
    }
    if ((sint32)moving->ribbon.pos.y.bits > 0)
    {
        effect->kind = 0xffffffffu;
    }
    return -1;
}

GDB_CALL void effect_draw_moving_ribbon(const struct EFFECT *effect)
{
    FUNCTION_MARKER(0x8005cee4u, "GAME.EXE");
    PROJECTION_POINT a, b;
    const EFFECT_RIBBON *ribbon;
    /* TODO: Investigate callbacks outside the moving-ribbon contract */
    if (!effect || effect->kind != 876 || effect->payload_kind != EFFECT_PAYLOAD_MOVING_RIBBON)
    {
        DIAGNOSTIC(fprintf(stderr, "Invalid moving ribbon draw %p\n", (const void *)effect));
        diagnostic_stop(__FUNCTION__, __FILE__, __LINE__);
    }
    ribbon = &effect->moving_ribbon.ribbon;
    transform_push();
    transform_translate(ribbon->pos.x.bits - camera.pos.x.bits, ribbon->pos.y.bits - camera.pos.y.bits, ribbon->pos.z.bits - camera.pos.z.bits);
    proj_local(0, 0, 0, &a);
    if (!a.status)
    {
        proj_local(ribbon->delta.x, ribbon->delta.y, ribbon->delta.z, &b);
        if (!b.status)
        {
            render_style.color.rgbc = 0x2c303040;
            a.size = 128;
            b.size = 256;
            /* TODO: Investigate original adjacent texture-map reads */
            if (ribbon->frame < -131 || ribbon->frame >= TEXTURE_MAP_COUNT - 131)
            {
                DIAGNOSTIC(fprintf(stderr, "Invalid moving ribbon texture frame %d\n", ribbon->frame));
                diagnostic_stop(__FUNCTION__, __FILE__, __LINE__);
            }
            spr_draw_ribbon(&b, &a, tex_map[131 + ribbon->frame]);
            render_style.color.rgbc = 0x2c181820;
            a.size = 176;
            b.size = 336;
            /* TODO: Investigate original adjacent texture-map reads */
            if (ribbon->frame < -131 || ribbon->frame >= TEXTURE_MAP_COUNT - 131)
            {
                DIAGNOSTIC(fprintf(stderr, "Invalid moving ribbon texture frame %d\n", ribbon->frame));
                diagnostic_stop(__FUNCTION__, __FILE__, __LINE__);
            }
            spr_draw_ribbon(&b, &a, tex_map[131 + ribbon->frame]);
        }
    }
    transform_pop();
}

/* Cached model-kind dispatch retains original no-draw cases */
GDB_CALL void effect_draw_spr(struct MODEL_OBJECT *item, struct EFFECT *effect)
{
    FUNCTION_MARKER(0x8005c808, "GAME.EXE");
    /* TODO: Investigate absent or conflicting native sprite owners */
    if ((!item && !effect) || (item && effect))
    {
        DIAGNOSTIC(fprintf(stderr, "Invalid shared sprite owners model %p effect %p in %s stage %u tick %u\n", (void *)item, (void *)effect, __FUNCTION__, game_session.stage, game_session.tick));
        diagnostic_stop(__FUNCTION__, __FILE__, __LINE__);
    }
    uint32 *kind_ref = effect ? &effect->kind : &item->kind;

    enum
    {
        MOTION_FRAME_COUNT = 8
    };

    static const uint32 motion_sizes[MOTION_FRAME_COUNT] = {288, 384, 512, 768, 1280, 1024, 768, 768};
    /* TODO: Investigate shared callbacks outside the native object pools */
    if (!kind_ref)
    {
        DIAGNOSTIC(fprintf(stderr, "Invalid shared effect/model receiver %p effect %p in %s stage %u tick %u\n", (void *)item, (void *)effect, __FUNCTION__, game_session.stage, game_session.tick));
        diagnostic_stop(__FUNCTION__, __FILE__, __LINE__);
    }
    uint32 type = *kind_ref, shade;
    const sint16 *table = NULL;
    PROJECTION_POINT pos;
    sint32 age, sample = 0;
    /* TODO: Investigate missing publication or cross-format slot reuse */
    if (effect)
    {
        uint32 spr_kind = type;
        if (effect->payload_kind != EFFECT_PAYLOAD_SPRITE || (!(spr_kind - 870u < 5u || spr_kind == 877 || spr_kind == 879 || spr_kind == 880 || spr_kind - 891u < 5u || spr_kind - 575u < 2u || spr_kind - 602u < 6u) && spr_kind != 0xffffffffu))
        {
            DIAGNOSTIC(fprintf(stderr, "Invalid sprite publication %p kind %u tag %u\n", (void *)effect, spr_kind, (uint32)effect->payload_kind));
            diagnostic_stop(__FUNCTION__, __FILE__, __LINE__);
        }
    }
    const EFFECT_SPRITE *spr = effect ? &effect->spr : NULL;
    transform_push();
    object_transform_camera(item, effect);
    proj_local(0, 0, 0, &pos);
    if (pos.status)
    {
        transform_pop();
        return;
    }
    if (type - 862u < 3)
    {
        pos.x = (sint32)((uint32)pos.x - 12u);
        pos.y = (sint32)((uint32)pos.y - 8u);
        spr_draw(&pos, tex_map[(-848 + (type))]);
        transform_pop();
        return;
    }
    if (type == 869)
    {
        transform_pop(); /* Original stores size but never draws. */
        return;
    }
    /* TODO: Investigate a model sprite callback with a foreign native payload */
    if (item && (type == 881 || type == 865 || type == 866 || type - 575u < 2u || type - 870u < 5u || type == 877 || type == 879 || type == 880) && item->payload_kind != MODEL_PAYLOAD_SPRITE)
    {
        DIAGNOSTIC(fprintf(stderr, "Invalid model sprite payload object %p effect %p callback %s stage %u tick %u\n", (void *)item, (void *)effect, __FUNCTION__, game_session.stage, game_session.tick));
        diagnostic_stop(__FUNCTION__, __FILE__, __LINE__);
    }
    MODEL_SPRITE *model_spr = item && item->payload_kind == MODEL_PAYLOAD_SPRITE ? &item->spr : NULL;
    if (type == 881)
    {
        age = (sint16)(uint16)((spr ? (uint16)spr->frame : (uint16)model_spr->frame) + 1);
        if (model_spr)
            model_spr->frame = (sint16)age;
        if (age == 7)
        {
            *kind_ref = 0xffffffffu;
        }
        transform_pop();
        return;
    }
    switch (type)
    {
        case 870:
            age = spr ? spr->frame : model_spr->frame;
            shade = 32u - ((uint32)age << 1);
            pos.size = 384;
            render_style.color.rgbc = 0x2c000000u + (shade << 16) + (shade << 8) + shade;
            table = tex_map + 33;
            sample = spr ? spr->frame : model_spr->frame;
            break;
        case 871:
        case 872:
            age = spr ? spr->frame : model_spr->frame;
            shade = 32u - (uint32)age;
            pos.size = 512;
            render_style.color.rgbc = type == 871 ? 0x2c000000u + (shade << 16) + (shade << 8) + shade : 0x2c000000u + (shade << 8);
            table = tex_map + 33;
            sample = (sint16)(spr ? (uint16)spr->frame : (uint16)model_spr->frame) >> 1;
            break;
        case 879:
            pos.size = 512;
            table = tex_map + 102;
            sample = spr ? spr->frame : model_spr->frame;
            break;
        case 873:
        case 874:
        case 880:
            pos.size = 512;
            render_style.color.rgbc = 0x2c303030;
            table = type == 880 ? (tex_map + 106) : (tex_map + 88);
            sample = (sint16)(spr ? (uint16)spr->frame : (uint16)model_spr->frame) >> (type == 874 ? 2 : 1);
            break;
        case 877:
            pos.depth = (sint32)((uint32)pos.depth - 16u);
            age = spr ? spr->frame : model_spr->frame;
            /* TODO: Investigate original reads beyond the eight motion frames */
            if (age < 0 || age >= MOTION_FRAME_COUNT)
            {
                DIAGNOSTIC(fprintf(stderr, "Invalid motion-sprite size frame %d for object %p effect %p\n", age, (void *)item, (void *)effect));
                diagnostic_stop(__FUNCTION__, __FILE__, __LINE__);
            }
            pos.size = (sint32)motion_sizes[age];
            render_style.color.rgbc = 0x2c608080;
            table = tex_map + 131;
            sample = spr ? spr->frame : model_spr->frame;
            break;
        case 865:
        case 866:
            pos.size = 256;
            table = type == 865 ? (tex_map + 7) : (tex_map + 66);
            sample = spr ? spr->frame : model_spr->frame;
            break;
        case 575:
            age = spr ? spr->frame : model_spr->frame;
            shade = 64u - ((uint32)age << 2);
            render_style.color.rgbc = 0x2c000000u + (shade << 14) + (shade << 7) + shade;
            pos.size = (sint32)(((uint32)(sint32)(spr ? spr->frame : model_spr->frame) << 5) + 256);
            table = tex_map + 73;
            break;
        case 576:
            age = (sint16)(spr ? (uint16)spr->frame : (uint16)model_spr->frame);
            shade = 32u - (uint32)(age >> 1);
            render_style.color.rgbc = 0x2c000000u + (shade << 16) + (shade << 8) + shade;
            pos.size = (sint32)(((uint32)(sint32)(spr ? spr->frame : model_spr->frame) << 5) + 256);
            table = tex_map + 87;
            break;
        default:
            transform_pop();
            return;
    }
    /* 5CC94 repeats type879, unreachable after its earlier5C9D8 match. */
    sint32 tex_idx = (sint32)(table - tex_map) + sample;
    if (tex_idx < 0 || tex_idx >= TEXTURE_MAP_COUNT)
        diagnostic_stop(__FUNCTION__, __FILE__, __LINE__);
    spr_draw_scaled(&pos, tex_map[tex_idx]);
    transform_pop();
}

GDB_CALL uint32 effect_update_dust_short(struct MODEL_OBJECT *item, struct EFFECT *effect)
{
    FUNCTION_MARKER(0x8002ae10u, "GAME.EXE");
    /* TODO: Investigate absent or conflicting native sprite owners */
    if ((!item && !effect) || (item && effect))
    {
        DIAGNOSTIC(fprintf(stderr, "Invalid shared sprite owners model %p effect %p in %s stage %u tick %u\n", (void *)item, (void *)effect, __FUNCTION__, game_session.stage, game_session.tick));
        diagnostic_stop(__FUNCTION__, __FILE__, __LINE__);
    }
    uint32 *kind_ref = effect ? &effect->kind : &item->kind;
    /* TODO: Investigate missing publication or cross-format slot reuse */
    if (effect)
    {
        uint32 spr_kind = *kind_ref;
        if (effect->payload_kind != EFFECT_PAYLOAD_SPRITE || (!(spr_kind - 870u < 5u || spr_kind == 877 || spr_kind == 879 || spr_kind == 880 || spr_kind - 891u < 5u || spr_kind - 575u < 2u || spr_kind - 602u < 6u) && spr_kind != 0xffffffffu))
        {
            DIAGNOSTIC(fprintf(stderr, "Invalid sprite publication %p kind %u tag %u\n", (void *)effect, spr_kind, (uint32)effect->payload_kind));
            diagnostic_stop(__FUNCTION__, __FILE__, __LINE__);
        }
    }
    EFFECT_SPRITE *spr = effect ? &effect->spr : NULL;
    /* TODO: Investigate a model sprite callback with a foreign native payload */
    if (item && item->payload_kind != MODEL_PAYLOAD_SPRITE)
    {
        DIAGNOSTIC(fprintf(stderr, "Invalid model dust payload %p stage %u tick %u\n", (void *)item, game_session.stage, game_session.tick));
        diagnostic_stop(__FUNCTION__, __FILE__, __LINE__);
    }
    MODEL_SPRITE *model_spr = item ? &item->spr : NULL;
    uint16 age = (uint16)((uint16)(spr ? spr->frame : model_spr->frame) + 1u), speed;
    uint32 delta;
    if (spr)
        spr->frame = age;
    else
        model_spr->frame = (sint16)age;
    if ((sint16)age >= 14)
    {
        *kind_ref = 0xffffffffu;
    }
    speed = (uint16)(spr ? spr->rise : model_spr->rise) + 1u;
    if (spr)
        spr->rise = speed;
    else
        model_spr->rise = (sint16)speed;
    delta = (sint32)((uint32)speed << 16) >> 1;
    if (spr)
        spr->pos.y.bits = spr->pos.y.bits - delta;
    else
        item->pos.y.bits = item->pos.y.bits - delta;
    return delta;
}

GDB_CALL uint32 effect_update_dust_long(struct EFFECT *effect)
{
    FUNCTION_MARKER(0x8002ae64u, "GAME.EXE");
    /* TODO: Investigate unpublished or cross-format sprite callback sources */
    if (!effect || effect->payload_kind != EFFECT_PAYLOAD_SPRITE || (!(effect->kind - 870u < 5u || effect->kind == 877 || effect->kind == 879 || effect->kind == 880 || effect->kind - 891u < 5u || effect->kind - 575u < 2u || effect->kind - 602u < 6u) && effect->kind != 0xffffffffu))
    {
        DIAGNOSTIC(fprintf(stderr, "Invalid native sprite source %p in %s stage %u tick %u\n", (void *)effect, __FUNCTION__, game_session.stage, game_session.tick));
        diagnostic_stop(__FUNCTION__, __FILE__, __LINE__);
    }
    EFFECT_SPRITE *spr = &effect->spr;
    uint16 age = (uint16)spr->frame + 1u;
    uint16 speed;
    uint32 delta;
    spr->frame = age;
    if ((sint16)age >= 28)
    {
        effect->kind = 0xffffffffu;
    }
    speed = (uint16)spr->rise + 1u;
    spr->rise = speed;
    delta = (sint32)((uint32)speed << 16) >> 1;
    spr->pos.y.bits = spr->pos.y.bits - delta;
    return delta;
}

GDB_CALL uint32 effect_update_falling_spr(struct EFFECT *effect)
{
    FUNCTION_MARKER(0x800417ecu, "GAME.EXE");
    /* TODO: Investigate unpublished or cross-format sprite callback sources */
    if (!effect || effect->payload_kind != EFFECT_PAYLOAD_SPRITE || (!(effect->kind - 870u < 5u || effect->kind == 877 || effect->kind == 879 || effect->kind == 880 || effect->kind - 891u < 5u || effect->kind - 575u < 2u || effect->kind - 602u < 6u) && effect->kind != 0xffffffffu))
    {
        DIAGNOSTIC(fprintf(stderr, "Invalid native sprite source %p in %s stage %u tick %u\n", (void *)effect, __FUNCTION__, game_session.stage, game_session.tick));
        diagnostic_stop(__FUNCTION__, __FILE__, __LINE__);
    }
    EFFECT_SPRITE *spr = &effect->spr;
    uint16 age = (uint16)spr->frame + 1u;
    uint32 result;
    spr->frame = age;
    if ((sint16)age >= 56)
    {
        spr->frame = 55;
    }
    spr->pos.x.bits = spr->pos.x.bits + ((uint32)(sint32)spr->motion.x << 16);
    spr->pos.y.bits = spr->pos.y.bits + ((uint32)(sint32)spr->motion.y << 16);
    spr->pos.z.bits = spr->pos.z.bits + ((uint32)(sint32)spr->motion.z << 16);
    if ((sint32)spr->pos.y.bits > 0)
    {
        spr->pos.y.bits = 0;
        effect->kind = 0xffffffffu;
    }
    result = (uint16)spr->motion.y + 1u;
    spr->motion.y = (uint16)result;
    return result;
}

GDB_CALL sint32 effect_update_drifting_spr(struct EFFECT *effect)
{
    FUNCTION_MARKER(0x80041894u, "GAME.EXE");
    /* TODO: Investigate unpublished or cross-format sprite callback sources */
    if (!effect || effect->payload_kind != EFFECT_PAYLOAD_SPRITE || (!(effect->kind - 870u < 5u || effect->kind == 877 || effect->kind == 879 || effect->kind == 880 || effect->kind - 891u < 5u || effect->kind - 575u < 2u || effect->kind - 602u < 6u) && effect->kind != 0xffffffffu))
    {
        DIAGNOSTIC(fprintf(stderr, "Invalid native sprite source %p in %s stage %u tick %u\n", (void *)effect, __FUNCTION__, game_session.stage, game_session.tick));
        diagnostic_stop(__FUNCTION__, __FILE__, __LINE__);
    }
    EFFECT_SPRITE *spr = &effect->spr;
    sint32 result;
    spr->frame = (uint16)(spr->frame + 1);
    if (spr->frame >= 28)
    {
        effect->kind = 0xffffffffu;
    }
    if (spr->frame == 11)
    {
        spr->motion.x = (uint16)((random_next() & 15u) - 8u);
        spr->motion.z = (uint16)((random_next() & 15u) - 8u);
    }
    spr->pos.x.bits = spr->pos.x.bits + ((uint32)(sint32)spr->motion.x << 16);
    spr->pos.z.bits = spr->pos.z.bits + ((uint32)(sint32)spr->motion.z << 16);
    spr->rise = (uint16)(spr->rise - 13);
    result = (sint32)((uint32)(sint32)spr->rise << 16) >> 2;
    spr->pos.y.bits = spr->pos.y.bits - (uint32)result;
    return result;
}

GDB_CALL intptr effect_update_landing_spr(struct EFFECT *effect)
{
    FUNCTION_MARKER(0x800416e0u, "GAME.EXE");
    /* TODO: Investigate unpublished or cross-format sprite callback sources */
    if (!effect || effect->payload_kind != EFFECT_PAYLOAD_SPRITE || (!(effect->kind - 870u < 5u || effect->kind == 877 || effect->kind == 879 || effect->kind == 880 || effect->kind - 891u < 5u || effect->kind - 575u < 2u || effect->kind - 602u < 6u) && effect->kind != 0xffffffffu))
    {
        DIAGNOSTIC(fprintf(stderr, "Invalid native sprite source %p in %s stage %u tick %u\n", (void *)effect, __FUNCTION__, game_session.stage, game_session.tick));
        diagnostic_stop(__FUNCTION__, __FILE__, __LINE__);
    }
    EFFECT_SPRITE *spr = &effect->spr;
    uint32 r;
    intptr result;
    spr->frame = (uint16)(spr->frame + 1);
    if (spr->frame >= 8)
    {
        effect->kind = 0xffffffffu;
    }
    spr->pos.x.bits = spr->pos.x.bits + ((uint32)(sint32)spr->motion.x << 15);
    spr->pos.y.bits = spr->pos.y.bits + ((uint32)(sint32)spr->motion.y << 15);
    spr->pos.z.bits = spr->pos.z.bits + ((uint32)(sint32)spr->motion.z << 15);
    result = spr->pos.y.bits;
    if ((sint32)result > 0)
    {
        r = random_next();
        result = (intptr)effect_spawn_timed(871, spr->pos.x.bits + (((r & 127u) - 64u) << 16), 0u - (((r >> 12) & 3u) << 16), spr->pos.z.bits + ((((r >> 6) & 127u) - 64u) << 16), 0, 0, 0);
        /* TODO: Investigate cross-format reuse after the original dust spawn */
        if (effect->payload_kind != EFFECT_PAYLOAD_SPRITE)
        {
            DIAGNOSTIC(fprintf(stderr, "Sprite format changed during landing %p\n", (void *)effect));
            diagnostic_stop(__FUNCTION__, __FILE__, __LINE__);
        }
        spr->pos.y.bits = 0;
    }
    return result;
}

GDB_CALL uint32 effect_update_spr_short(struct EFFECT *effect)
{
    FUNCTION_MARKER(0x80041974u, "GAME.EXE");
    /* TODO: Investigate unpublished or cross-format sprite callback sources */
    if (!effect || effect->payload_kind != EFFECT_PAYLOAD_SPRITE || (!(effect->kind - 870u < 5u || effect->kind == 877 || effect->kind == 879 || effect->kind == 880 || effect->kind - 891u < 5u || effect->kind - 575u < 2u || effect->kind - 602u < 6u) && effect->kind != 0xffffffffu))
    {
        DIAGNOSTIC(fprintf(stderr, "Invalid native sprite source %p in %s stage %u tick %u\n", (void *)effect, __FUNCTION__, game_session.stage, game_session.tick));
        diagnostic_stop(__FUNCTION__, __FILE__, __LINE__);
    }
    EFFECT_SPRITE *spr = &effect->spr;
    uint16 age = (uint16)spr->frame + 1u;
    spr->frame = age;
    if ((sint16)age >= 4)
    {
        effect->kind = 0xffffffffu;
    }
    return 0xffffffffu;
}

GDB_CALL uint32 effect_update_spr_long(struct EFFECT *effect)
{
    FUNCTION_MARKER(0x800419a4u, "GAME.EXE");
    /* TODO: Investigate unpublished or cross-format sprite callback sources */
    if (!effect || effect->payload_kind != EFFECT_PAYLOAD_SPRITE || (!(effect->kind - 870u < 5u || effect->kind == 877 || effect->kind == 879 || effect->kind == 880 || effect->kind - 891u < 5u || effect->kind - 575u < 2u || effect->kind - 602u < 6u) && effect->kind != 0xffffffffu))
    {
        DIAGNOSTIC(fprintf(stderr, "Invalid native sprite source %p in %s stage %u tick %u\n", (void *)effect, __FUNCTION__, game_session.stage, game_session.tick));
        diagnostic_stop(__FUNCTION__, __FILE__, __LINE__);
    }
    EFFECT_SPRITE *spr = &effect->spr;
    uint16 age = (uint16)spr->frame + 1u;
    spr->frame = age;
    if ((sint16)age >= 32)
    {
        effect->kind = 0xffffffffu;
    }
    return 0xffffffffu;
}

GDB_CALL void effect_draw_oriented_model(const struct EFFECT *effect)
{
    FUNCTION_MARKER(0x8005d2e4u, "GAME.EXE");
    uint32 kind;
    transform_push();
    object_transform_camera(NULL, effect);
    kind = effect->kind;
    /* TODO: Investigate original late model-reference reads outside the native table */
    if (kind >= MODEL_TYPE_COUNT)
    {
        DIAGNOSTIC(fprintf(stderr, "Invalid oriented effect model %u for %p\n", kind, (const void *)effect));
        diagnostic_stop(__FUNCTION__, __FILE__, __LINE__);
    }
    model_draw_static(model_refs[kind], 0, 0);
    transform_pop();
}

GDB_CALL uint32 effect_update_blast_large(struct EFFECT *effect)
{
    FUNCTION_MARKER(0x8002aeb8u, "GAME.EXE");
    /* TODO: Investigate unpublished or cross-format sprite callback sources */
    if (!effect || effect->payload_kind != EFFECT_PAYLOAD_SPRITE || (!(effect->kind - 870u < 5u || effect->kind == 877 || effect->kind == 879 || effect->kind == 880 || effect->kind - 891u < 5u || effect->kind - 575u < 2u || effect->kind - 602u < 6u) && effect->kind != 0xffffffffu))
    {
        DIAGNOSTIC(fprintf(stderr, "Invalid native sprite source %p in %s stage %u tick %u\n", (void *)effect, __FUNCTION__, game_session.stage, game_session.tick));
        diagnostic_stop(__FUNCTION__, __FILE__, __LINE__);
    }
    EFFECT_SPRITE *spr = &effect->spr;
    uint16 age = (uint16)spr->frame + 1u;
    spr->frame = age;
    if ((sint16)age >= 36)
    {
        effect->kind = 0xffffffffu;
    }
    return 0xffffffffu;
}

GDB_CALL uint32 effect_update_blast(struct EFFECT *effect)
{
    FUNCTION_MARKER(0x8002aee8u, "GAME.EXE");
    /* TODO: Investigate unpublished or cross-format sprite callback sources */
    if (!effect || effect->payload_kind != EFFECT_PAYLOAD_SPRITE || (!(effect->kind - 870u < 5u || effect->kind == 877 || effect->kind == 879 || effect->kind == 880 || effect->kind - 891u < 5u || effect->kind - 575u < 2u || effect->kind - 602u < 6u) && effect->kind != 0xffffffffu))
    {
        DIAGNOSTIC(fprintf(stderr, "Invalid native sprite source %p in %s stage %u tick %u\n", (void *)effect, __FUNCTION__, game_session.stage, game_session.tick));
        diagnostic_stop(__FUNCTION__, __FILE__, __LINE__);
    }
    EFFECT_SPRITE *spr = &effect->spr;
    uint16 age = (uint16)spr->frame + 1u;
    spr->frame = age;
    if ((sint16)age >= 18)
    {
        effect->kind = 0xffffffffu;
    }
    return 0xffffffffu;
}

GDB_CALL void effect_draw_blast_large(const struct EFFECT *effect)
{
    FUNCTION_MARKER(0x8005d160u, "GAME.EXE");
    PROJECTION_POINT pos;
    /* TODO: Investigate draw callbacks crossing native payload formats */
    if (!effect || effect->payload_kind != EFFECT_PAYLOAD_SPRITE)
    {
        DIAGNOSTIC(fprintf(stderr, "Invalid sprite draw source %p in %s stage %u tick %u\n", (const void *)effect, __FUNCTION__, game_session.stage, game_session.tick));
        diagnostic_stop(__FUNCTION__, __FILE__, __LINE__);
    }
    sint32 tex_idx;
    transform_push();
    object_transform_camera(NULL, effect);
    const EFFECT_SPRITE *spr = &effect->spr;
    proj_local(0, 0, 0, &pos);
    if (!pos.status)
    {
        pos.size = 4096;
        render_style.color.rgbc = 0x2c404040;
        tex_idx = 74 + (spr->frame >> 2);
        /* TODO: Investigate original reads beyond the native texture map */
        if (tex_idx < 0 || tex_idx >= TEXTURE_MAP_COUNT)
        {
            DIAGNOSTIC(fprintf(stderr, "Invalid blast texture index %d for %p\n", tex_idx, (const void *)effect));
            diagnostic_stop(__FUNCTION__, __FILE__, __LINE__);
        }
        spr_draw_scaled(&pos, tex_map[tex_idx]);
    }
    transform_pop();
}

GDB_CALL uint32 effect_update_rising_spr_short(struct MODEL_OBJECT *item, struct EFFECT *effect)
{
    FUNCTION_MARKER(0x8002ace8u, "GAME.EXE");
    /* TODO: Investigate absent or conflicting native sprite owners */
    if ((!item && !effect) || (item && effect))
    {
        DIAGNOSTIC(fprintf(stderr, "Invalid shared sprite owners model %p effect %p in %s stage %u tick %u\n", (void *)item, (void *)effect, __FUNCTION__, game_session.stage, game_session.tick));
        diagnostic_stop(__FUNCTION__, __FILE__, __LINE__);
    }
    uint32 *kind_ref = effect ? &effect->kind : &item->kind;
    /* TODO: Investigate missing publication or cross-format slot reuse */
    if (effect)
    {
        uint32 spr_kind = *kind_ref;
        if (effect->payload_kind != EFFECT_PAYLOAD_SPRITE || (!(spr_kind - 870u < 5u || spr_kind == 877 || spr_kind == 879 || spr_kind == 880 || spr_kind - 891u < 5u || spr_kind - 575u < 2u || spr_kind - 602u < 6u) && spr_kind != 0xffffffffu))
        {
            DIAGNOSTIC(fprintf(stderr, "Invalid sprite publication %p kind %u tag %u\n", (void *)effect, spr_kind, (uint32)effect->payload_kind));
            diagnostic_stop(__FUNCTION__, __FILE__, __LINE__);
        }
    }
    EFFECT_SPRITE *spr = effect ? &effect->spr : NULL;
    /* TODO: Investigate a model sprite callback with a foreign native payload */
    if (item && item->payload_kind != MODEL_PAYLOAD_SPRITE)
    {
        DIAGNOSTIC(fprintf(stderr, "Invalid model sprite payload object %p effect %p callback %s stage %u tick %u\n", (void *)item, (void *)effect, __FUNCTION__, game_session.stage, game_session.tick));
        diagnostic_stop(__FUNCTION__, __FILE__, __LINE__);
    }
    MODEL_SPRITE *model_spr = item ? &item->spr : NULL;
    uint16 age, velocity;
    uint32 delta;
    if (!(spr ? spr->frame : model_spr->frame))
    {
        if (spr)
            spr->rise = (uint16)(random_next() & 3u);
        else
            model_spr->rise = (sint16)(random_next() & 3u);
    }
    age = (uint16)(spr ? spr->frame : model_spr->frame) + 1u;
    if (spr)
        spr->frame = age;
    else
        model_spr->frame = (sint16)age;
    if ((sint16)age == 16)
    {
        /* TODO: Investigate shared callbacks outside the native object pools */
        if (!kind_ref)
        {
            DIAGNOSTIC(fprintf(stderr, "Invalid shared effect/model receiver %p effect %p in %s stage %u tick %u\n", (void *)item, (void *)effect, __FUNCTION__, game_session.stage, game_session.tick));
            diagnostic_stop(__FUNCTION__, __FILE__, __LINE__);
        }
        *kind_ref = 0xffffffffu;
    }
    velocity = (uint16)(spr ? spr->rise : model_spr->rise) + 1u;
    if (spr)
        spr->rise = velocity;
    else
        model_spr->rise = (sint16)velocity;
    delta = (sint32)((uint32)velocity << 16) >> 1;
    /* TODO: Investigate shared callbacks outside the native object pools */
    if (!kind_ref)
    {
        DIAGNOSTIC(fprintf(stderr, "Invalid shared effect/model receiver %p effect %p in %s stage %u tick %u\n", (void *)item, (void *)effect, __FUNCTION__, game_session.stage, game_session.tick));
        diagnostic_stop(__FUNCTION__, __FILE__, __LINE__);
    }
    if (spr)
        spr->pos.y.bits = spr->pos.y.bits - delta;
    else
        item->pos.y.bits = item->pos.y.bits - delta;
    return delta;
}

GDB_CALL uint32 effect_update_rising_spr_long(struct MODEL_OBJECT *item, struct EFFECT *effect)
{
    FUNCTION_MARKER(0x8002ad7cu, "GAME.EXE");
    /* TODO: Investigate absent or conflicting native sprite owners */
    if ((!item && !effect) || (item && effect))
    {
        DIAGNOSTIC(fprintf(stderr, "Invalid shared sprite owners model %p effect %p in %s stage %u tick %u\n", (void *)item, (void *)effect, __FUNCTION__, game_session.stage, game_session.tick));
        diagnostic_stop(__FUNCTION__, __FILE__, __LINE__);
    }
    uint32 *kind_ref = effect ? &effect->kind : &item->kind;
    /* TODO: Investigate missing publication or cross-format slot reuse */
    if (effect)
    {
        uint32 spr_kind = *kind_ref;
        if (effect->payload_kind != EFFECT_PAYLOAD_SPRITE || (!(spr_kind - 870u < 5u || spr_kind == 877 || spr_kind == 879 || spr_kind == 880 || spr_kind - 891u < 5u || spr_kind - 575u < 2u || spr_kind - 602u < 6u) && spr_kind != 0xffffffffu))
        {
            DIAGNOSTIC(fprintf(stderr, "Invalid sprite publication %p kind %u tag %u\n", (void *)effect, spr_kind, (uint32)effect->payload_kind));
            diagnostic_stop(__FUNCTION__, __FILE__, __LINE__);
        }
    }
    EFFECT_SPRITE *spr = effect ? &effect->spr : NULL;
    /* TODO: Investigate a model sprite callback with a foreign native payload */
    if (item && item->payload_kind != MODEL_PAYLOAD_SPRITE)
    {
        DIAGNOSTIC(fprintf(stderr, "Invalid model sprite payload object %p effect %p callback %s stage %u tick %u\n", (void *)item, (void *)effect, __FUNCTION__, game_session.stage, game_session.tick));
        diagnostic_stop(__FUNCTION__, __FILE__, __LINE__);
    }
    MODEL_SPRITE *model_spr = item ? &item->spr : NULL;
    uint16 age, velocity;
    uint32 delta;
    if (!(spr ? spr->frame : model_spr->frame))
    {
        if (spr)
            spr->rise = (uint16)(random_next() & 3u);
        else
            model_spr->rise = (sint16)(random_next() & 3u);
    }
    age = (uint16)(spr ? spr->frame : model_spr->frame) + 1u;
    if (spr)
        spr->frame = age;
    else
        model_spr->frame = (sint16)age;
    if ((sint16)age == 64)
    {
        /* TODO: Investigate shared callbacks outside the native object pools */
        if (!kind_ref)
        {
            DIAGNOSTIC(fprintf(stderr, "Invalid shared effect/model receiver %p effect %p in %s stage %u tick %u\n", (void *)item, (void *)effect, __FUNCTION__, game_session.stage, game_session.tick));
            diagnostic_stop(__FUNCTION__, __FILE__, __LINE__);
        }
        *kind_ref = 0xffffffffu;
    }
    velocity = (uint16)(spr ? spr->rise : model_spr->rise) + 1u;
    if (spr)
        spr->rise = velocity;
    else
        model_spr->rise = (sint16)velocity;
    delta = (sint32)((uint32)velocity << 16) >> 1;
    /* TODO: Investigate shared callbacks outside the native object pools */
    if (!kind_ref)
    {
        DIAGNOSTIC(fprintf(stderr, "Invalid shared effect/model receiver %p effect %p in %s stage %u tick %u\n", (void *)item, (void *)effect, __FUNCTION__, game_session.stage, game_session.tick));
        diagnostic_stop(__FUNCTION__, __FILE__, __LINE__);
    }
    if (spr)
        spr->pos.y.bits = spr->pos.y.bits - delta;
    else
        item->pos.y.bits = item->pos.y.bits - delta;
    return delta;
}

GDB_CALL uint32 effect_update_rising_smoke(struct MODEL_OBJECT *item, struct EFFECT *effect)
{
    FUNCTION_MARKER(0x8002b150u, "GAME.EXE");
    /* TODO: Investigate absent or conflicting native sprite owners */
    if ((!item && !effect) || (item && effect))
    {
        DIAGNOSTIC(fprintf(stderr, "Invalid shared sprite owners model %p effect %p in %s stage %u tick %u\n", (void *)item, (void *)effect, __FUNCTION__, game_session.stage, game_session.tick));
        diagnostic_stop(__FUNCTION__, __FILE__, __LINE__);
    }
    uint32 *kind_ref = effect ? &effect->kind : &item->kind;
    /* TODO: Investigate missing publication or cross-format slot reuse */
    if (effect)
    {
        uint32 spr_kind = *kind_ref;
        if (effect->payload_kind != EFFECT_PAYLOAD_SPRITE || (!(spr_kind - 870u < 5u || spr_kind == 877 || spr_kind == 879 || spr_kind == 880 || spr_kind - 891u < 5u || spr_kind - 575u < 2u || spr_kind - 602u < 6u) && spr_kind != 0xffffffffu))
        {
            DIAGNOSTIC(fprintf(stderr, "Invalid sprite publication %p kind %u tag %u\n", (void *)effect, spr_kind, (uint32)effect->payload_kind));
            diagnostic_stop(__FUNCTION__, __FILE__, __LINE__);
        }
    }
    EFFECT_SPRITE *spr = effect ? &effect->spr : NULL;
    /* TODO: Investigate a model sprite callback with a foreign native payload */
    if (item && item->payload_kind != MODEL_PAYLOAD_SPRITE)
    {
        DIAGNOSTIC(fprintf(stderr, "Invalid model sprite payload object %p effect %p callback %s stage %u tick %u\n", (void *)item, (void *)effect, __FUNCTION__, game_session.stage, game_session.tick));
        diagnostic_stop(__FUNCTION__, __FILE__, __LINE__);
    }
    MODEL_SPRITE *model_spr = item ? &item->spr : NULL;
    uint16 age, velocity;
    uint32 delta;
    if (!(spr ? spr->frame : model_spr->frame))
    {
        if (spr)
            spr->rise = (uint16)(random_next() & 3u);
        else
            model_spr->rise = (sint16)(random_next() & 3u);
    }
    age = (uint16)(spr ? spr->frame : model_spr->frame) + 1u;
    if (spr)
        spr->frame = age;
    else
        model_spr->frame = (sint16)age;
    if ((sint16)age == 16)
    {
        /* TODO: Investigate shared callbacks outside the native object pools */
        if (!kind_ref)
        {
            DIAGNOSTIC(fprintf(stderr, "Invalid shared effect/model receiver %p effect %p in %s stage %u tick %u\n", (void *)item, (void *)effect, __FUNCTION__, game_session.stage, game_session.tick));
            diagnostic_stop(__FUNCTION__, __FILE__, __LINE__);
        }
        *kind_ref = 0xffffffffu;
    }
    velocity = (uint16)(spr ? spr->rise : model_spr->rise) + 1u;
    if (spr)
        spr->rise = velocity;
    else
        model_spr->rise = (sint16)velocity;
    delta = (sint32)((uint32)velocity << 16) >> 1;
    /* TODO: Investigate shared callbacks outside the native object pools */
    if (!kind_ref)
    {
        DIAGNOSTIC(fprintf(stderr, "Invalid shared effect/model receiver %p effect %p in %s stage %u tick %u\n", (void *)item, (void *)effect, __FUNCTION__, game_session.stage, game_session.tick));
        diagnostic_stop(__FUNCTION__, __FILE__, __LINE__);
    }
    if (spr)
        spr->pos.y.bits = spr->pos.y.bits - delta;
    else
        item->pos.y.bits = item->pos.y.bits - delta;
    return delta;
}

GDB_CALL uint32 effect_update_ring_large(struct EFFECT *effect)
{
    FUNCTION_MARKER(0x8002af18u, "GAME.EXE");
    /* TODO: Investigate unpublished or cross-format sprite callback sources */
    if (!effect || effect->payload_kind != EFFECT_PAYLOAD_SPRITE || (!(effect->kind - 870u < 5u || effect->kind == 877 || effect->kind == 879 || effect->kind == 880 || effect->kind - 891u < 5u || effect->kind - 575u < 2u || effect->kind - 602u < 6u) && effect->kind != 0xffffffffu))
    {
        DIAGNOSTIC(fprintf(stderr, "Invalid native sprite source %p in %s stage %u tick %u\n", (void *)effect, __FUNCTION__, game_session.stage, game_session.tick));
        diagnostic_stop(__FUNCTION__, __FILE__, __LINE__);
    }
    EFFECT_SPRITE *spr = &effect->spr;
    uint16 age = (uint16)spr->frame + 1u;
    spr->frame = age;
    if ((sint16)age >= 32)
    {
        effect->kind = 0xffffffffu;
    }
    return 0xffffffffu;
}

GDB_CALL uint32 effect_update_ring(struct EFFECT *effect)
{
    FUNCTION_MARKER(0x8002af48u, "GAME.EXE");
    /* TODO: Investigate unpublished or cross-format sprite callback sources */
    if (!effect || effect->payload_kind != EFFECT_PAYLOAD_SPRITE || (!(effect->kind - 870u < 5u || effect->kind == 877 || effect->kind == 879 || effect->kind == 880 || effect->kind - 891u < 5u || effect->kind - 575u < 2u || effect->kind - 602u < 6u) && effect->kind != 0xffffffffu))
    {
        DIAGNOSTIC(fprintf(stderr, "Invalid native sprite source %p in %s stage %u tick %u\n", (void *)effect, __FUNCTION__, game_session.stage, game_session.tick));
        diagnostic_stop(__FUNCTION__, __FILE__, __LINE__);
    }
    EFFECT_SPRITE *spr = &effect->spr;
    uint16 age = (uint16)spr->frame + 1u;
    spr->frame = age;
    if ((sint16)age >= 16)
    {
        effect->kind = 0xffffffffu;
    }
    return 0xffffffffu;
}

GDB_CALL uint32 effect_update_ring_short(struct EFFECT *effect)
{
    FUNCTION_MARKER(0x8002b120u, "GAME.EXE");
    /* TODO: Investigate unpublished or cross-format sprite callback sources */
    if (!effect || effect->payload_kind != EFFECT_PAYLOAD_SPRITE || (!(effect->kind - 870u < 5u || effect->kind == 877 || effect->kind == 879 || effect->kind == 880 || effect->kind - 891u < 5u || effect->kind - 575u < 2u || effect->kind - 602u < 6u) && effect->kind != 0xffffffffu))
    {
        DIAGNOSTIC(fprintf(stderr, "Invalid native sprite source %p in %s stage %u tick %u\n", (void *)effect, __FUNCTION__, game_session.stage, game_session.tick));
        diagnostic_stop(__FUNCTION__, __FILE__, __LINE__);
    }
    EFFECT_SPRITE *spr = &effect->spr;
    sint16 age = (uint16)spr->frame + 1u;
    spr->frame = (uint16)age;
    if (age >= 8)
    {
        effect->kind = 0xffffffffu;
    }
    return 0xffffffffu;
}

GDB_CALL void effect_draw_ring_large(const struct EFFECT *effect)
{
    FUNCTION_MARKER(0x8005d340u, "GAME.EXE");
    uint32 scale, color;
    /* TODO: Investigate draw callbacks crossing native payload formats */
    if (!effect || effect->payload_kind != EFFECT_PAYLOAD_SPRITE)
    {
        DIAGNOSTIC(fprintf(stderr, "Invalid sprite draw source %p in %s stage %u tick %u\n", (const void *)effect, __FUNCTION__, game_session.stage, game_session.tick));
        diagnostic_stop(__FUNCTION__, __FILE__, __LINE__);
    }
    transform_push();
    object_transform_camera(NULL, effect);
    const EFFECT_SPRITE *spr = &effect->spr;
    scale = (uint32)(sint32)spr->frame << 12;
    transform_scale(scale, scale, scale);
    color = 64u - 2u * (uint32)(sint32)spr->frame;
    render_style.color.rgbc = 0x2c000000u + (color << 16) + (color << 8) + color;
    model_draw_static(model_refs[602], 0, 0);
    transform_pop();
}

GDB_CALL void effect_draw_ring(const struct EFFECT *effect)
{
    FUNCTION_MARKER(0x8005d3d8u, "GAME.EXE");
    uint32 shade, kind;
    sint32 scale;
    /* TODO: Investigate draw callbacks crossing native payload formats */
    if (!effect || effect->payload_kind != EFFECT_PAYLOAD_SPRITE)
    {
        DIAGNOSTIC(fprintf(stderr, "Invalid sprite draw source %p in %s stage %u tick %u\n", (const void *)effect, __FUNCTION__, game_session.stage, game_session.tick));
        diagnostic_stop(__FUNCTION__, __FILE__, __LINE__);
    }
    transform_push();
    object_transform_camera(NULL, effect);
    const EFFECT_SPRITE *spr = &effect->spr;
    scale = (uint32)(sint32)spr->frame << 12;
    transform_scale(scale, scale, scale);
    shade = 64u - ((uint32)(sint32)spr->frame << 2);
    render_style.color.rgbc = 0x2c000000u + (shade << 16) + (shade << 8) + shade;
    kind = effect->kind;
    /* TODO: Investigate original late model-reference reads beyond the native table */
    if (kind >= MODEL_TYPE_COUNT)
    {
        DIAGNOSTIC(fprintf(stderr, "Invalid ring model %u for %p\n", kind, (const void *)effect));
        diagnostic_stop(__FUNCTION__, __FILE__, __LINE__);
    }
    model_draw_static(model_refs[kind], 0, kind == 604 ? 0 : 8192);
    transform_pop();
}

GDB_CALL void effect_draw_ring_small(const struct EFFECT *effect)
{
    FUNCTION_MARKER(0x8005d488u, "GAME.EXE");
    uint32 shade, kind;
    sint32 scale;
    /* TODO: Investigate draw callbacks crossing native payload formats */
    if (!effect || effect->payload_kind != EFFECT_PAYLOAD_SPRITE)
    {
        DIAGNOSTIC(fprintf(stderr, "Invalid sprite draw source %p in %s stage %u tick %u\n", (const void *)effect, __FUNCTION__, game_session.stage, game_session.tick));
        diagnostic_stop(__FUNCTION__, __FILE__, __LINE__);
    }
    transform_push();
    object_transform_camera(NULL, effect);
    const EFFECT_SPRITE *spr = &effect->spr;
    scale = (uint32)(sint32)spr->frame << 13;
    if (effect->kind == 607)
        scale >>= 1;
    transform_scale(scale, scale, scale);
    shade = 64u - ((uint32)(sint32)spr->frame << 3);
    render_style.color.rgbc = 0x2c000000u + (shade << 16) + (shade << 8) + shade;
    kind = effect->kind;
    /* TODO: Investigate original late model-reference reads beyond the native table */
    if (kind >= MODEL_TYPE_COUNT)
    {
        DIAGNOSTIC(fprintf(stderr, "Invalid small ring model %u for %p\n", kind, (const void *)effect));
        diagnostic_stop(__FUNCTION__, __FILE__, __LINE__);
    }
    model_draw_static(model_refs[kind], 0, 0);
    transform_pop();
}

GDB_CALL void effect_draw_rising_smoke(const struct EFFECT *effect)
{
    FUNCTION_MARKER(0x8005d53cu, "GAME.EXE");
    PROJECTION_POINT pos;
    uint32 color;
    /* TODO: Investigate draw callbacks crossing native payload formats */
    if (!effect || effect->payload_kind != EFFECT_PAYLOAD_SPRITE)
    {
        DIAGNOSTIC(fprintf(stderr, "Invalid sprite draw source %p in %s stage %u tick %u\n", (const void *)effect, __FUNCTION__, game_session.stage, game_session.tick));
        diagnostic_stop(__FUNCTION__, __FILE__, __LINE__);
    }
    transform_push();
    object_transform_camera(NULL, effect);
    const EFFECT_SPRITE *spr = &effect->spr;
    proj_local(0, 0, 0, &pos);
    if (!pos.status)
    {
        pos.size = 256;
        render_style.color.rgbc = 0x2c404040u;
        color = 64u - ((uint32)(sint32)spr->frame << 2);
        render_style.color.rgbc = 0x2c000000u + (color << 16) + (color << 8) + color;
        pos.size = (sint32)(((uint32)(sint32)spr->frame << 4) + 256u);
        spr_draw_scaled(&pos, tex_map[73]);
    }
    transform_pop();
}

GDB_CALL void effect_draw_blast(const struct EFFECT *effect)
{
    FUNCTION_MARKER(0x8005d1fcu, "GAME.EXE");
    PROJECTION_POINT pos;
    uint32 kind;
    /* TODO: Investigate draw callbacks crossing native payload formats */
    if (!effect || effect->payload_kind != EFFECT_PAYLOAD_SPRITE)
    {
        DIAGNOSTIC(fprintf(stderr, "Invalid sprite draw source %p in %s stage %u tick %u\n", (const void *)effect, __FUNCTION__, game_session.stage, game_session.tick));
        diagnostic_stop(__FUNCTION__, __FILE__, __LINE__);
    }
    sint32 tex_idx;
    transform_push();
    object_transform_camera(NULL, effect);
    const EFFECT_SPRITE *spr = &effect->spr;
    proj_local(0, 0, 0, &pos);
    if (!pos.status)
    {
        kind = effect->kind;
        if (kind == 892)
            pos.size = 2048;
        else if (kind == 893)
            pos.size = 1024;
        else if (kind == 894)
            pos.size = 256;
        else if (kind == 895)
            pos.size = 128;
        render_style.color.rgbc = 0x2c404040u;
        tex_idx = 74 + (spr->frame >> 1);
        /* TODO: Investigate original reads beyond the native texture map */
        if (tex_idx < 0 || tex_idx >= TEXTURE_MAP_COUNT)
        {
            DIAGNOSTIC(fprintf(stderr, "Invalid blast texture index %d for %p\n", tex_idx, (const void *)effect));
            diagnostic_stop(__FUNCTION__, __FILE__, __LINE__);
        }
        spr_draw_scaled(&pos, tex_map[tex_idx]);
    }
    transform_pop();
}

GDB_CALL sint32 effect_update_shockwave(struct EFFECT *effect)
{
    FUNCTION_MARKER(0x8002af78u, "GAME.EXE");
    /* TODO: Investigate unpublished or cross-format sprite callback sources */
    if (!effect || effect->payload_kind != EFFECT_PAYLOAD_SPRITE || (!(effect->kind - 870u < 5u || effect->kind == 877 || effect->kind == 879 || effect->kind == 880 || effect->kind - 891u < 5u || effect->kind - 575u < 2u || effect->kind - 602u < 6u) && effect->kind != 0xffffffffu))
    {
        DIAGNOSTIC(fprintf(stderr, "Invalid native sprite source %p in %s stage %u tick %u\n", (void *)effect, __FUNCTION__, game_session.stage, game_session.tick));
        diagnostic_stop(__FUNCTION__, __FILE__, __LINE__);
    }
    EFFECT_SPRITE *spr = &effect->spr;
    sint32 actor_idx = spr->owner;
    uint32 angle, dx, dz;
    COLLISION_POINT point;
    sint16 age = (uint16)spr->frame + 1u;
    sint32 result, owner;
    int i;
    spr->frame = age;
    if (age >= 16)
    {
        effect->kind = 0xffffffffu;
    }
    angle = ((uint32)(uint16)spr->frame << 8) & 0xf00u;
    dx = ((uint32)(sint32)sin_table[angle >> 1] * (uint32)(sint32)spr->frame) << 7;
    dz = ((uint32)(sint32)sin_table[MATH_QUARTER_TURN + (angle >> 1)] * (uint32)(sint32)spr->frame) << 7;
    point.pos.bits[1] = 0;
    point.impact_sound = 0;
    /* TODO: Investigate original probes through an invalid source actor */
    if (actor_idx < 0 || actor_idx >= ACTOR_COUNT)
    {
        DIAGNOSTIC(fprintf(stderr, "Invalid shockwave source actor %d for %p\n", actor_idx, (void *)effect));
        diagnostic_stop(__FUNCTION__, __FILE__, __LINE__);
    }
    for (i = 0; i < 4; i++)
    {
        point.pos.bits[0] = (i & 1) ? actor_positions[actor_idx].x.bits - dx : actor_positions[actor_idx].x.bits + dx;
        point.pos.bits[2] = (i == 0 || i == 3) ? actor_positions[actor_idx].z.bits - dz : actor_positions[actor_idx].z.bits + dz;
        /* TODO: Investigate cross-format reuse during the original probe chain */
        if (effect->payload_kind != EFFECT_PAYLOAD_SPRITE)
        {
            DIAGNOSTIC(fprintf(stderr, "Shockwave payload changed during probes %p\n", (void *)effect));
            diagnostic_stop(__FUNCTION__, __FILE__, __LINE__);
        }
        owner = spr->owner;
        /* TODO: Investigate invalid live owners instead of indexing adjacent actors */
        if (owner < 0 || owner >= ACTOR_COUNT)
        {
            DIAGNOSTIC(fprintf(stderr, "Invalid live shockwave owner %d for %p\n", owner, (void *)effect));
            diagnostic_stop(__FUNCTION__, __FILE__, __LINE__);
        }
        result = actor_process_stage_contact(&point, owner);
        if (result)
            return result;
    }
    return result;
}

GDB_CALL sint32 effect_update_particle(struct EFFECT *effect)
{
    FUNCTION_MARKER(0x8002bec0u, "GAME.EXE");
    /* TODO: Investigate unpublished or cross-format particle callback sources */
    if (!effect || effect->payload_kind != EFFECT_PAYLOAD_PARTICLE || (!(effect->kind - 897u < 13u || effect->kind < 6u || effect->kind - 6u < 2u || effect->kind - 577u < 3u || effect->kind == 583 || effect->kind - 599u < 3u || effect->kind - 72u < 4u) && effect->kind != 0xffffffffu))
    {
        DIAGNOSTIC(fprintf(stderr, "Invalid native particle source %p in %s stage %u tick %u\n", (void *)effect, __FUNCTION__, game_session.stage, game_session.tick));
        diagnostic_stop(__FUNCTION__, __FILE__, __LINE__);
    }
    EFFECT_PARTICLE *particle = &effect->particle;
    sint32 speed = particle->radial_speed, angle = particle->yaw, vertical, age;
    uint32 y;
    sint32 trig;
    /* TODO: Investigate original adjacent trigonometric-table reads */
    if (angle < 0 || angle >= MATH_SIN_COUNT)
    {
        DIAGNOSTIC(fprintf(stderr, "Invalid particle trigonometric index %d for %p\n", angle, (void *)effect));
        diagnostic_stop(__FUNCTION__, __FILE__, __LINE__);
    }
    trig = sin_table[angle];
    particle->pos.x.bits = particle->pos.x.bits + (uint32)((speed * trig) >> 1);
    speed = particle->radial_speed;
    angle = particle->yaw;
    /* TODO: Investigate original adjacent trigonometric-table reads */
    if (MATH_QUARTER_TURN + angle < 0 || MATH_QUARTER_TURN + angle >= MATH_SIN_COUNT)
    {
        DIAGNOSTIC(fprintf(stderr, "Invalid particle trigonometric index %d for %p\n", MATH_QUARTER_TURN + angle, (void *)effect));
        diagnostic_stop(__FUNCTION__, __FILE__, __LINE__);
    }
    trig = sin_table[MATH_QUARTER_TURN + angle];
    particle->pos.z.bits = particle->pos.z.bits - (uint32)((speed * trig) >> 1);
    vertical = (sint16)particle->vertical_speed;
    if (vertical >= -255)
    {
        particle->vertical_speed = (uint16)(vertical - 2);
    }
    y = particle->pos.y.bits - ((uint32)(sint32)(sint16)particle->vertical_speed << 15);
    particle->pos.y.bits = y;
    if ((sint32)y > 0)
    {
        effect->kind = 0xffffffff;
    }
    age = (sint16)particle->age;
    if (age < 63)
    {
        particle->age = (uint16)(age + 1);
    }
    return age + 1;
}

GDB_CALL void effect_draw_hit_particle(const struct EFFECT *effect)
{
    FUNCTION_MARKER(0x8005c2f0u, "GAME.EXE");
    /* TODO: Investigate unpublished or cross-format particle callback sources */
    if (!effect || effect->payload_kind != EFFECT_PAYLOAD_PARTICLE || (!(effect->kind - 897u < 13u || effect->kind < 6u || effect->kind - 6u < 2u || effect->kind - 577u < 3u || effect->kind == 583 || effect->kind - 599u < 3u || effect->kind - 72u < 4u) && effect->kind != 0xffffffffu))
    {
        DIAGNOSTIC(fprintf(stderr, "Invalid native particle source %p in %s stage %u tick %u\n", (const void *)effect, __FUNCTION__, game_session.stage, game_session.tick));
        diagnostic_stop(__FUNCTION__, __FILE__, __LINE__);
    }
    const EFFECT_PARTICLE *particle = &effect->particle;
    uint32 color, x, y, z, camera_x, camera_y, camera_z;
    PROJECTION_POINT pos;
    sint32 age, color_idx;
    const sint16 *table;
    transform_push();
    x = particle->pos.x.bits;
    camera_x = camera.pos.x.bits;
    y = particle->pos.y.bits;
    camera_y = camera.pos.y.bits;
    z = particle->pos.z.bits;
    camera_z = camera.pos.z.bits;
    transform_translate(x - camera_x, y - camera_y, z - camera_z);
    proj_local(0, 0, 0, &pos);
    if (!pos.status)
    {
        table = effect->kind == 1 ? alt_hit_colors : game_settings.language == GAME_LANGUAGE_GERMAN ? german_hit_colors : hit_colors;
        age = (sint16)(uint16)particle->age;
        color_idx = age >> 2;
        /* TODO: Investigate original color reads beyond the sixteen age samples */
        if (color_idx < 0 || color_idx >= PARTICLE_COLOR_COUNT)
        {
            DIAGNOSTIC(fprintf(stderr, "Invalid particle color index %d for %p\n", color_idx, (const void *)effect));
            diagnostic_stop(__FUNCTION__, __FILE__, __LINE__);
        }
        color = (uint32)(sint32)table[color_idx];
        age = (sint16)particle->age;
        if (age >= 17)
            particle_draw_point(pos.x, pos.y, pos.depth, color);
        else if ((sint16)particle->age >= 9)
            particle_draw_line(pos.x, pos.y, pos.depth, color);
        else
            particle_draw_quad(pos.x, pos.y, pos.depth, color);
    }
    transform_pop();
}

GDB_CALL void effect_draw_spark(const struct EFFECT *effect)
{
    FUNCTION_MARKER(0x8005c480u, "GAME.EXE");
    /* TODO: Investigate unpublished or cross-format particle callback sources */
    if (!effect || effect->payload_kind != EFFECT_PAYLOAD_PARTICLE || (!(effect->kind - 897u < 13u || effect->kind < 6u || effect->kind - 6u < 2u || effect->kind - 577u < 3u || effect->kind == 583 || effect->kind - 599u < 3u || effect->kind - 72u < 4u) && effect->kind != 0xffffffffu))
    {
        DIAGNOSTIC(fprintf(stderr, "Invalid native particle source %p in %s stage %u tick %u\n", (const void *)effect, __FUNCTION__, game_session.stage, game_session.tick));
        diagnostic_stop(__FUNCTION__, __FILE__, __LINE__);
    }
    const EFFECT_PARTICLE *particle = &effect->particle;
    uint32 color, x, y, z, camera_x, camera_y, camera_z;
    PROJECTION_POINT pos;
    sint32 age, color_idx;
    transform_push();
    x = particle->pos.x.bits;
    camera_x = camera.pos.x.bits;
    y = particle->pos.y.bits;
    camera_y = camera.pos.y.bits;
    z = particle->pos.z.bits;
    camera_z = camera.pos.z.bits;
    transform_translate(x - camera_x, y - camera_y, z - camera_z);
    proj_local(0, 0, 0, &pos);
    if (!pos.status)
    {
        age = (sint16)(uint16)particle->age;
        color_idx = age >> 2;
        /* TODO: Investigate original color reads beyond the sixteen age samples */
        if (color_idx < 0 || color_idx >= PARTICLE_COLOR_COUNT)
        {
            DIAGNOSTIC(fprintf(stderr, "Invalid particle color index %d for %p\n", color_idx, (const void *)effect));
            diagnostic_stop(__FUNCTION__, __FILE__, __LINE__);
        }
        color = (uint32)(sint32)spark_colors[color_idx];
        if (age >= 17)
            particle_draw_point(pos.x, pos.y, pos.depth, color);
        else if ((sint16)particle->age >= 9)
            particle_draw_line(pos.x, pos.y, pos.depth, color);
        else
            particle_draw_quad(pos.x, pos.y, pos.depth, color);
    }
    transform_pop();
}

GDB_CALL void effect_draw_point(const struct EFFECT *effect)
{
    FUNCTION_MARKER(0x8005c598u, "GAME.EXE");
    /* TODO: Investigate unpublished or cross-format particle callback sources */
    if (!effect || effect->payload_kind != EFFECT_PAYLOAD_PARTICLE || (!(effect->kind - 897u < 13u || effect->kind < 6u || effect->kind - 6u < 2u || effect->kind - 577u < 3u || effect->kind == 583 || effect->kind - 599u < 3u || effect->kind - 72u < 4u) && effect->kind != 0xffffffffu))
    {
        DIAGNOSTIC(fprintf(stderr, "Invalid native particle source %p in %s stage %u tick %u\n", (const void *)effect, __FUNCTION__, game_session.stage, game_session.tick));
        diagnostic_stop(__FUNCTION__, __FILE__, __LINE__);
    }
    const EFFECT_PARTICLE *particle = &effect->particle;
    PROJECTION_POINT pos;
    sint32 color_idx;
    transform_push();
    transform_translate(particle->pos.x.bits - camera.pos.x.bits, particle->pos.y.bits - camera.pos.y.bits, particle->pos.z.bits - camera.pos.z.bits);
    proj_local(0, 0, 0, &pos);
    if (!pos.status)
    {
        color_idx = (sint16)particle->age >> 2;
        /* TODO: Investigate original color reads beyond the sixteen age samples */
        if (color_idx < 0 || color_idx >= PARTICLE_COLOR_COUNT)
        {
            DIAGNOSTIC(fprintf(stderr, "Invalid particle color index %d for %p\n", color_idx, (const void *)effect));
            diagnostic_stop(__FUNCTION__, __FILE__, __LINE__);
        }
        particle_draw_point(pos.x, pos.y, pos.depth, (sint32)point_colors[color_idx]);
    }
    transform_pop();
}

static uint32 update_slowing_particle(struct EFFECT *effect)
{
    /* TODO: Investigate unpublished or cross-format particle callback sources */
    if (!effect || effect->payload_kind != EFFECT_PAYLOAD_PARTICLE || (!(effect->kind - 897u < 13u || effect->kind < 6u || effect->kind - 6u < 2u || effect->kind - 577u < 3u || effect->kind == 583 || effect->kind - 599u < 3u || effect->kind - 72u < 4u) && effect->kind != 0xffffffffu))
    {
        DIAGNOSTIC(fprintf(stderr, "Invalid native particle source %p in %s stage %u tick %u\n", (void *)effect, __FUNCTION__, game_session.stage, game_session.tick));
        diagnostic_stop(__FUNCTION__, __FILE__, __LINE__);
    }
    EFFECT_PARTICLE *particle = &effect->particle;
    uint32 speed, angle, delta;
    sint32 vertical;
    uint16 age;
    speed = (uint32)(sint32)particle->radial_speed;
    angle = (uint32)(sint32)particle->yaw;
    /* TODO: Investigate original adjacent trigonometric-table reads */
    if ((sint32)angle < 0 || (sint32)angle >= MATH_SIN_COUNT)
    {
        DIAGNOSTIC(fprintf(stderr, "Invalid particle-variant trig index %d for %p\n", (sint32)angle, (void *)effect));
        diagnostic_stop(__FUNCTION__, __FILE__, __LINE__);
    }
    delta = speed * (uint32)(sint32)sin_table[(sint32)angle];
    particle->pos.x.bits = particle->pos.x.bits + delta;
    speed = (uint32)(sint32)particle->radial_speed;
    angle = (uint32)(sint32)particle->yaw;
    /* TODO: Investigate original adjacent trigonometric-table reads */
    if (MATH_QUARTER_TURN + (sint32)angle < 0 || MATH_QUARTER_TURN + (sint32)angle >= MATH_SIN_COUNT)
    {
        DIAGNOSTIC(fprintf(stderr, "Invalid particle-variant trig index %d for %p\n", MATH_QUARTER_TURN + (sint32)angle, (void *)effect));
        diagnostic_stop(__FUNCTION__, __FILE__, __LINE__);
    }
    delta = speed * (uint32)(sint32)sin_table[MATH_QUARTER_TURN + (sint32)angle];
    particle->pos.z.bits = particle->pos.z.bits - delta;
    vertical = (sint16)particle->vertical_speed;
    if (vertical >= -255)
    {
        particle->vertical_speed = (uint16)(vertical - 1);
    }
    particle->pos.y.bits = particle->pos.y.bits - ((uint32)(sint32)(sint16)particle->vertical_speed << 16);
    age = (uint16)(sint16)particle->age + 1u;
    particle->age = age;
    if ((sint16)age >= 193)
    {
        effect->kind = 0xffffffffu;
    }
    return 0xffffffffu;
}

static uint32 update_aged_particle(struct EFFECT *effect, uint32 inc)
{
    /* TODO: Investigate unpublished or cross-format particle callback sources */
    if (!effect || effect->payload_kind != EFFECT_PAYLOAD_PARTICLE || (!(effect->kind - 897u < 13u || effect->kind < 6u || effect->kind - 6u < 2u || effect->kind - 577u < 3u || effect->kind == 583 || effect->kind - 599u < 3u || effect->kind - 72u < 4u) && effect->kind != 0xffffffffu))
    {
        DIAGNOSTIC(fprintf(stderr, "Invalid native particle source %p in %s stage %u tick %u\n", (void *)effect, __FUNCTION__, game_session.stage, game_session.tick));
        diagnostic_stop(__FUNCTION__, __FILE__, __LINE__);
    }
    EFFECT_PARTICLE *particle = &effect->particle;
    uint32 speed, angle, product;
    uint16 age;
    speed = (uint32)(sint32)particle->radial_speed;
    angle = (uint32)(sint32)particle->yaw;
    /* TODO: Investigate original adjacent trigonometric-table reads */
    if ((sint32)angle < 0 || (sint32)angle >= MATH_SIN_COUNT)
    {
        DIAGNOSTIC(fprintf(stderr, "Invalid particle-variant trig index %d for %p\n", (sint32)angle, (void *)effect));
        diagnostic_stop(__FUNCTION__, __FILE__, __LINE__);
    }
    product = speed * (uint32)(sint32)sin_table[(sint32)angle];
    particle->pos.x.bits = particle->pos.x.bits + product;
    speed = (uint32)(sint32)particle->radial_speed;
    angle = (uint32)(sint32)particle->yaw;
    /* TODO: Investigate original adjacent trigonometric-table reads */
    if (MATH_QUARTER_TURN + (sint32)angle < 0 || MATH_QUARTER_TURN + (sint32)angle >= MATH_SIN_COUNT)
    {
        DIAGNOSTIC(fprintf(stderr, "Invalid particle-variant trig index %d for %p\n", MATH_QUARTER_TURN + (sint32)angle, (void *)effect));
        diagnostic_stop(__FUNCTION__, __FILE__, __LINE__);
    }
    product = speed * (uint32)(sint32)sin_table[MATH_QUARTER_TURN + (sint32)angle];
    particle->pos.z.bits = particle->pos.z.bits - product;
    age = (uint16)particle->age + inc;
    particle->age = age;
    if ((sint16)age == 64)
    {
        effect->kind = 0xffffffffu;
        return 0xffffffffu;
    }
    return (uint32)(sint32)(sint16)age;
}

GDB_CALL uint32 effect_update_falling_glass(struct EFFECT *effect)
{
    FUNCTION_MARKER(0x8002bfa8u, "GAME.EXE");
    /* TODO: Investigate unpublished or cross-format particle callback sources */
    if (!effect || effect->payload_kind != EFFECT_PAYLOAD_PARTICLE || (!(effect->kind - 897u < 13u || effect->kind < 6u || effect->kind - 6u < 2u || effect->kind - 577u < 3u || effect->kind == 583 || effect->kind - 599u < 3u || effect->kind - 72u < 4u) && effect->kind != 0xffffffffu))
    {
        DIAGNOSTIC(fprintf(stderr, "Invalid native particle source %p in %s stage %u tick %u\n", (void *)effect, __FUNCTION__, game_session.stage, game_session.tick));
        diagnostic_stop(__FUNCTION__, __FILE__, __LINE__);
    }
    EFFECT_PARTICLE *particle = &effect->particle;
    uint32 result, i;
    particle->vertical_speed = (uint16)((uint32)(uint16)(sint16)particle->vertical_speed + 5u);
    particle->yaw = (uint16)((uint32)(uint16)particle->yaw + 480u);
    particle->pitch = (uint16)((uint32)(uint16)particle->pitch + 1120u);
    particle->roll = (uint16)((uint32)(uint16)particle->roll + 1824u);
    result = particle->pos.y.bits + ((uint32)(sint32)(sint16)particle->vertical_speed << 16);
    particle->pos.y.bits = result;
    if ((sint32)result > 0)
    {
        effect->kind = 0xffffffffu;
        for (i = 0; i < 4; i++)
        {
            /* TODO: Investigate cross-format reuse after the original glass free */
            if (effect->payload_kind != EFFECT_PAYLOAD_PARTICLE)
            {
                DIAGNOSTIC(fprintf(stderr, "Glass payload changed during spark creation %p\n", (void *)effect));
                diagnostic_stop(__FUNCTION__, __FILE__, __LINE__);
            }
            effect_spawn_spark(particle->pos.x.bits, 0, particle->pos.z.bits);
        }
        return 0;
    }
    return result;
}

GDB_CALL void effect_draw_glass(const struct EFFECT *effect)
{
    FUNCTION_MARKER(0x8002abe0u, "GAME.EXE");
    /* TODO: Investigate unpublished or cross-format particle callback sources */
    if (!effect || effect->payload_kind != EFFECT_PAYLOAD_PARTICLE || (!(effect->kind - 897u < 13u || effect->kind < 6u || effect->kind - 6u < 2u || effect->kind - 577u < 3u || effect->kind == 583 || effect->kind - 599u < 3u || effect->kind - 72u < 4u) && effect->kind != 0xffffffffu))
    {
        DIAGNOSTIC(fprintf(stderr, "Invalid native particle source %p in %s stage %u tick %u\n", (const void *)effect, __FUNCTION__, game_session.stage, game_session.tick));
        diagnostic_stop(__FUNCTION__, __FILE__, __LINE__);
    }
    const EFFECT_PARTICLE *particle = &effect->particle;
    uint32 kind;
    transform_push();
    transform_translate(particle->pos.x.bits - camera.pos.x.bits, particle->pos.y.bits - camera.pos.y.bits, particle->pos.z.bits - camera.pos.z.bits);
    transform_roll(particle->roll);
    transform_yaw(particle->pitch);
    transform_pitch(particle->yaw);
    kind = effect->kind;
    /* TODO: Investigate original late glass model-reference reads beyond the table */
    if (kind >= MODEL_TYPE_COUNT)
    {
        DIAGNOSTIC(fprintf(stderr, "Invalid glass model %u for %p\n", kind, (const void *)effect));
        diagnostic_stop(__FUNCTION__, __FILE__, __LINE__);
    }
    model_draw_static(model_refs[kind], 0, 0);
    transform_pop();
}

GDB_CALL uint32 effect_update_slowing_particle(struct EFFECT *effect)
{
    FUNCTION_MARKER(0x8002c060u, "GAME.EXE");
    /* TODO: Investigate unpublished or cross-format particle callback sources */
    if (!effect || effect->payload_kind != EFFECT_PAYLOAD_PARTICLE || (!(effect->kind - 897u < 13u || effect->kind < 6u || effect->kind - 6u < 2u || effect->kind - 577u < 3u || effect->kind == 583 || effect->kind - 599u < 3u || effect->kind - 72u < 4u) && effect->kind != 0xffffffffu))
    {
        DIAGNOSTIC(fprintf(stderr, "Invalid native particle source %p in %s stage %u tick %u\n", (void *)effect, __FUNCTION__, game_session.stage, game_session.tick));
        diagnostic_stop(__FUNCTION__, __FILE__, __LINE__);
    }
    EFFECT_PARTICLE *particle = &effect->particle;
    uint16 speed = (uint16)particle->radial_speed - 1u;
    particle->radial_speed = speed;
    if ((sint16)speed < 0)
    {
        particle->radial_speed = 0;
    }
    return update_slowing_particle(effect);
}

GDB_CALL uint32 effect_update_stationary_particle(struct EFFECT *effect)
{
    FUNCTION_MARKER(0x8002c078u, "GAME.EXE");
    /* TODO: Investigate unpublished or cross-format particle callback sources */
    if (!effect || effect->payload_kind != EFFECT_PAYLOAD_PARTICLE || (!(effect->kind - 897u < 13u || effect->kind < 6u || effect->kind - 6u < 2u || effect->kind - 577u < 3u || effect->kind == 583 || effect->kind - 599u < 3u || effect->kind - 72u < 4u) && effect->kind != 0xffffffffu))
    {
        DIAGNOSTIC(fprintf(stderr, "Invalid native particle source %p in %s stage %u tick %u\n", (void *)effect, __FUNCTION__, game_session.stage, game_session.tick));
        diagnostic_stop(__FUNCTION__, __FILE__, __LINE__);
    }
    EFFECT_PARTICLE *particle = &effect->particle;
    particle->radial_speed = 0;
    return update_slowing_particle(effect);
}

GDB_CALL uint32 effect_update_short_particle(struct EFFECT *effect)
{
    FUNCTION_MARKER(0x8002c154u, "GAME.EXE");
    return update_aged_particle(effect, 4);
}

GDB_CALL uint32 effect_update_long_particle(struct EFFECT *effect)
{
    FUNCTION_MARKER(0x8002c1f4u, "GAME.EXE");
    return update_aged_particle(effect, 2);
}

GDB_CALL uint32 effect_remove_short_particle(struct EFFECT *effect)
{
    FUNCTION_MARKER(0x8002c1dcu, "GAME.EXE");
    /* TODO: Investigate an absent native particle removal source */
    if (!effect)
        diagnostic_stop(__FUNCTION__, __FILE__, __LINE__);
    effect->kind = 0xffffffffu;
    return 0xffffffffu;
}

GDB_CALL uint32 effect_remove_long_particle(struct EFFECT *effect)
{
    FUNCTION_MARKER(0x8002c27cu, "GAME.EXE");
    /* TODO: Investigate an absent native particle removal source */
    if (!effect)
        diagnostic_stop(__FUNCTION__, __FILE__, __LINE__);
    effect->kind = 0xffffffffu;
    return 0xffffffffu;
}

GDB_CALL void effect_draw_rotating_particle(void)
{
    FUNCTION_MARKER(0x8005c654u, "GAME.EXE");
}

GDB_CALL void effect_draw_short_particle(void)
{
    FUNCTION_MARKER(0x8005c65cu, "GAME.EXE");
}

GDB_CALL void effect_draw_long_particle(void)
{
    FUNCTION_MARKER(0x8005c664u, "GAME.EXE");
}

GDB_CALL sint32 effect_update_bouncing_particle(struct MODEL_OBJECT *item, struct EFFECT *effect)
{
    FUNCTION_MARKER(0x8002e11cu, "GAME.EXE");
    /* TODO: Investigate absent or conflicting native bouncing owners */
    if ((!item && !effect) || (item && effect))
    {
        DIAGNOSTIC(fprintf(stderr, "Invalid bouncing owners model %p effect %p stage %u tick %u\n", (void *)item, (void *)effect, game_session.stage, game_session.tick));
        diagnostic_stop(__FUNCTION__, __FILE__, __LINE__);
    }
    PHYSICS_OBJECT *physics = NULL;
    if (item)
    {
        /* TODO: Investigate a bouncing callback with a foreign native owner */
        if (item->payload_kind != MODEL_PAYLOAD_PHYSICS)
        {
            DIAGNOSTIC(fprintf(stderr, "Invalid bouncing model %p kind %u payload %u stage %u tick %u\n", (void *)item, item->kind, (uint32)item->payload_kind, game_session.stage, game_session.tick));
            diagnostic_stop(__FUNCTION__, __FILE__, __LINE__);
        }
        physics = &item->physics;
    }
    if (effect)
    {
        uint32 kind = effect->kind;
        /* TODO: Investigate missing publication or incompatible particle callbacks */
        if (effect->payload_kind != EFFECT_PAYLOAD_PARTICLE || (!(kind - 897u < 13u || kind < 6u || kind - 6u < 2u || kind - 577u < 3u || kind == 583 || kind - 599u < 3u || kind - 72u < 4u) && kind != 0xffffffffu))
        {
            DIAGNOSTIC(fprintf(stderr, "Invalid bouncing particle %p kind %u payload %u stage %u tick %u\n", (void *)effect, kind, (uint32)effect->payload_kind, game_session.stage, game_session.tick));
            diagnostic_stop(__FUNCTION__, __FILE__, __LINE__);
        }
    }
    EFFECT_PARTICLE *particle = effect ? &effect->particle : NULL;
    uint32 *kind_ref = effect ? &effect->kind : &item->kind;
    /* TODO: Investigate shared callbacks outside the native object pools */
    if (!kind_ref)
    {
        DIAGNOSTIC(fprintf(stderr, "Invalid shared effect/model receiver %p effect %p in %s stage %u tick %u\n", (void *)item, (void *)effect, __FUNCTION__, game_session.stage, game_session.tick));
        diagnostic_stop(__FUNCTION__, __FILE__, __LINE__);
    }
    uint32 offset = ((uint16)(particle ? particle->yaw : item->angles.yaw) >> 5) * 2u, y, kind;
    sint32 bounce, volume, result;
    if (particle)
        particle->pos.x.bits = particle->pos.x.bits + (uint32)((particle->radial_speed * sin_table[offset >> 1]) >> 1);
    else
        item->pos.x.bits = item->pos.x.bits + (uint32)((physics->speed * sin_table[offset >> 1]) >> 1);
    if (particle)
        particle->pos.z.bits = particle->pos.z.bits - (uint32)((particle->radial_speed * sin_table[MATH_QUARTER_TURN + (offset >> 1)]) >> 1);
    else
        item->pos.z.bits = item->pos.z.bits - (uint32)((physics->speed * sin_table[MATH_QUARTER_TURN + (offset >> 1)]) >> 1);
    if (particle)
        particle->radial_speed = (uint16)((uint16)particle->radial_speed - 1u);
    else
        physics->speed = (sint16)((uint16)physics->speed - 1u);
    if ((particle ? particle->radial_speed : physics->speed) < 0)
    {
        if (particle)
            particle->radial_speed = 0;
        else
            physics->speed = 0;
    }
    if (particle)
        particle->vertical_speed = (uint16)((uint16)(sint16)particle->vertical_speed - (*kind_ref - 512u < 2u ? 16u : 5u));
    else
        physics->vertical_speed = (sint16)((uint16)physics->vertical_speed - (*kind_ref - 512u < 2u ? 16u : 5u));
    y = (particle ? particle->pos.y.bits : item->pos.y.bits) - ((uint32)(sint32)(particle ? (sint16)particle->vertical_speed : physics->vertical_speed) << 15);
    if (particle)
        particle->pos.y.bits = y;
    else
        item->pos.y.bits = y;
    if ((sint32)y > 0)
    {
        if (particle)
            particle->pos.y.bits = 0;
        else
            item->pos.y.bits = 0;
        bounce = (-(particle ? (sint16)particle->vertical_speed : physics->vertical_speed)) >> 1;
        if (particle)
            particle->vertical_speed = (uint16)bounce;
        else
            physics->vertical_speed = (sint16)bounce;
        if (bounce >= 17)
        {
            if (particle)
                particle->bounce_count = (uint16)((uint16)particle->bounce_count + 1u);
            else
                physics->bounce_count = (sint16)((uint16)physics->bounce_count + 1u);
            volume = 255 - 64 * (particle ? particle->bounce_count : physics->bounce_count);
            if (volume < 0)
                volume = 0;
            kind = *kind_ref;
            if (kind == 577)
                sound_voice_play(29, volume, 7168);
            if (kind == 514)
                sound_voice_play(29, volume, 6144);
            if (kind == 578)
                sound_voice_play(29, volume, 5632);
            if (kind - 512u < 2u)
            {
                sound_voice_play((sint32)random_next() % 3 + 119, volume, 6144);
                /* TODO: Investigate model payload reuse across bouncing callback calls */
                if (item && item->payload_kind != MODEL_PAYLOAD_PHYSICS)
                {
                    DIAGNOSTIC(fprintf(stderr, "Bouncing model payload changed object %p effect %p kind %u payload %u stage %u tick %u\n", (void *)item, (void *)effect, item->kind, item->payload_kind, game_session.stage, game_session.tick));
                    diagnostic_stop(__FUNCTION__, __FILE__, __LINE__);
                }
                /* TODO: Investigate cross-format reuse during attachment-particle callbacks */
                if (effect && effect->payload_kind != EFFECT_PAYLOAD_PARTICLE)
                {
                    DIAGNOSTIC(fprintf(stderr, "Attachment particle payload changed %p effect %p\n", (void *)item, (void *)effect));
                    diagnostic_stop(__FUNCTION__, __FILE__, __LINE__);
                }
                if (kind == 513 && !(particle ? particle->life : physics->life))
                {
                    if (particle)
                        particle->life = 65535;
                    else
                        physics->life = -1;
                }
            }
        }
    }
    /* TODO: Investigate model payload reuse across bouncing callback calls */
    if (item && item->payload_kind != MODEL_PAYLOAD_PHYSICS)
    {
        DIAGNOSTIC(fprintf(stderr, "Bouncing model payload changed object %p effect %p kind %u payload %u stage %u tick %u\n", (void *)item, (void *)effect, item->kind, item->payload_kind, game_session.stage, game_session.tick));
        diagnostic_stop(__FUNCTION__, __FILE__, __LINE__);
    }
    /* TODO: Investigate cross-format reuse during attachment-particle callbacks */
    if (effect && effect->payload_kind != EFFECT_PAYLOAD_PARTICLE)
    {
        DIAGNOSTIC(fprintf(stderr, "Attachment particle payload changed %p effect %p\n", (void *)item, (void *)effect));
        diagnostic_stop(__FUNCTION__, __FILE__, __LINE__);
    }
    result = particle ? particle->life : physics->life;
    if (result < 0)
    {
        if (particle)
            particle->life = (uint16)(result - 1);
        else
            physics->life = (sint16)(result - 1);
        if ((particle ? particle->life : physics->life) < -60)
        {
            *kind_ref = 0xffffffffu;
        }
        return -1;
    }
    return result;
}

GDB_CALL void effect_draw_double_point(const struct EFFECT *effect)
{
    FUNCTION_MARKER(0x8005bbacu, "GAME.EXE");
    /* TODO: Investigate unpublished or cross-format particle callback sources */
    if (!effect || effect->payload_kind != EFFECT_PAYLOAD_PARTICLE || (!(effect->kind - 897u < 13u || effect->kind < 6u || effect->kind - 6u < 2u || effect->kind - 577u < 3u || effect->kind == 583 || effect->kind - 599u < 3u || effect->kind - 72u < 4u) && effect->kind != 0xffffffffu))
    {
        DIAGNOSTIC(fprintf(stderr, "Invalid native particle source %p in %s stage %u tick %u\n", (const void *)effect, __FUNCTION__, game_session.stage, game_session.tick));
        diagnostic_stop(__FUNCTION__, __FILE__, __LINE__);
    }
    const EFFECT_PARTICLE *particle = &effect->particle;
    PROJECTION_POINT pos;
    transform_push();
    transform_translate(particle->pos.x.bits - camera.pos.x.bits, particle->pos.y.bits - camera.pos.y.bits, particle->pos.z.bits - camera.pos.z.bits);
    proj_local(0, 0, 0, &pos);
    if (!pos.status)
    {
        particle_draw_point(pos.x, pos.y, pos.depth, 0x4210);
        particle_draw_point((uint32)pos.x + 1u, pos.y, pos.depth, 0x4210);
    }
    transform_pop();
}

GDB_CALL void effect_draw_attachment_model(const struct EFFECT *effect)
{
    FUNCTION_MARKER(0x8005bc64u, "GAME.EXE");
    /* TODO: Investigate unpublished or cross-format particle callback sources */
    if (!effect || effect->payload_kind != EFFECT_PAYLOAD_PARTICLE || (!(effect->kind - 897u < 13u || effect->kind < 6u || effect->kind - 6u < 2u || effect->kind - 577u < 3u || effect->kind == 583 || effect->kind - 599u < 3u || effect->kind - 72u < 4u) && effect->kind != 0xffffffffu))
    {
        DIAGNOSTIC(fprintf(stderr, "Invalid native particle source %p in %s stage %u tick %u\n", (const void *)effect, __FUNCTION__, game_session.stage, game_session.tick));
        diagnostic_stop(__FUNCTION__, __FILE__, __LINE__);
    }
    const EFFECT_PARTICLE *particle = &effect->particle;
    uint32 kind;
    transform_push();
    transform_translate(particle->pos.x.bits - camera.pos.x.bits, particle->pos.y.bits - camera.pos.y.bits, particle->pos.z.bits - camera.pos.z.bits);
    transform_yaw(16384u - ((uint32)(sint32)particle->yaw << 5));
    kind = effect->kind;
    /* TODO: Investigate original model references beyond the native table */
    if (kind >= MODEL_TYPE_COUNT)
    {
        DIAGNOSTIC(fprintf(stderr, "Invalid attachment model %u for %p\n", kind, (const void *)effect));
        diagnostic_stop(__FUNCTION__, __FILE__, __LINE__);
    }
    model_draw_static(model_refs[kind], 0, 0);
    transform_pop();
}

static uint32 update_falling_fragment(struct EFFECT *effect)
{
    /* TODO: Investigate unpublished or cross-format particle callback sources */
    if (!effect || effect->payload_kind != EFFECT_PAYLOAD_PARTICLE || (!(effect->kind - 897u < 13u || effect->kind < 6u || effect->kind - 6u < 2u || effect->kind - 577u < 3u || effect->kind == 583 || effect->kind - 599u < 3u || effect->kind - 72u < 4u) && effect->kind != 0xffffffffu))
    {
        DIAGNOSTIC(fprintf(stderr, "Invalid native particle source %p in %s stage %u tick %u\n", (void *)effect, __FUNCTION__, game_session.stage, game_session.tick));
        diagnostic_stop(__FUNCTION__, __FILE__, __LINE__);
    }
    EFFECT_PARTICLE *particle = &effect->particle;
    uint32 speed, angle, product, y, result;
    sint32 vertical;
    speed = (uint32)(sint32)particle->radial_speed;
    angle = (uint32)(sint32)particle->yaw;
    /* TODO: Investigate original fragment reads beyond the trigonometric table */
    if ((sint32)angle < 0 || (sint32)angle >= MATH_SIN_COUNT)
    {
        DIAGNOSTIC(fprintf(stderr, "Invalid fragment trig index %d for %p\n", (sint32)angle, (void *)effect));
        diagnostic_stop(__FUNCTION__, __FILE__, __LINE__);
    }
    product = speed * (uint32)(sint32)sin_table[(sint32)angle];
    particle->pos.x.bits = particle->pos.x.bits + (uint32)((sint32)product >> 1);
    speed = (uint32)(sint32)particle->radial_speed;
    angle = (uint32)(sint32)particle->yaw;
    /* TODO: Investigate original fragment reads beyond the trigonometric table */
    if (MATH_QUARTER_TURN + (sint32)angle < 0 || MATH_QUARTER_TURN + (sint32)angle >= MATH_SIN_COUNT)
    {
        DIAGNOSTIC(fprintf(stderr, "Invalid fragment trig index %d for %p\n", MATH_QUARTER_TURN + (sint32)angle, (void *)effect));
        diagnostic_stop(__FUNCTION__, __FILE__, __LINE__);
    }
    product = speed * (uint32)(sint32)sin_table[MATH_QUARTER_TURN + (sint32)angle];
    particle->pos.z.bits = particle->pos.z.bits - (uint32)((sint32)product >> 1);
    particle->pitch = (uint16)((uint16)particle->pitch + 1120);
    particle->roll = (uint16)((uint16)particle->roll + 1824);
    particle->radial_speed = (uint16)((uint16)particle->radial_speed - 4);
    vertical = (sint16)particle->vertical_speed;
    if (vertical >= -255)
    {
        particle->vertical_speed = (uint16)(vertical - 6);
    }
    vertical = (sint16)particle->vertical_speed;
    y = particle->pos.y.bits - ((uint32)vertical << 15);
    particle->pos.y.bits = y;
    if ((sint32)y > 0)
    {
        effect->kind = 0xffffffffu;
    }
    result = (uint16)particle->age + 1;
    particle->age = (uint16)result;
    return result;
}

static uint32 update_fragment_bounce(struct EFFECT *effect)
{
    /* TODO: Investigate unpublished or cross-format particle callback sources */
    if (!effect || effect->payload_kind != EFFECT_PAYLOAD_PARTICLE || (!(effect->kind - 897u < 13u || effect->kind < 6u || effect->kind - 6u < 2u || effect->kind - 577u < 3u || effect->kind == 583 || effect->kind - 599u < 3u || effect->kind - 72u < 4u) && effect->kind != 0xffffffffu))
    {
        DIAGNOSTIC(fprintf(stderr, "Invalid native particle source %p in %s stage %u tick %u\n", (void *)effect, __FUNCTION__, game_session.stage, game_session.tick));
        diagnostic_stop(__FUNCTION__, __FILE__, __LINE__);
    }
    EFFECT_PARTICLE *particle = &effect->particle;
    uint32 y = particle->pos.y.bits - ((uint32)(sint32)(sint16)particle->vertical_speed << 15);
    particle->pos.y.bits = y;
    if ((sint32)y > 0)
    {
        particle->pos.y.bits = 0;
        particle->vertical_speed = (uint16)(-((sint16)particle->vertical_speed >> 1));
    }
    if (particle->radial_speed < 0)
    {
        effect->kind = 0xffffffffu;
    }
    return 0xffffffffu;
}

GDB_CALL uint32 effect_update_delayed_fragment(struct EFFECT *effect)
{
    FUNCTION_MARKER(0x8002cba0u, "GAME.EXE");
    /* TODO: Investigate unpublished or cross-format particle callback sources */
    if (!effect || effect->payload_kind != EFFECT_PAYLOAD_PARTICLE || (!(effect->kind - 897u < 13u || effect->kind < 6u || effect->kind - 6u < 2u || effect->kind - 577u < 3u || effect->kind == 583 || effect->kind - 599u < 3u || effect->kind - 72u < 4u) && effect->kind != 0xffffffffu))
    {
        DIAGNOSTIC(fprintf(stderr, "Invalid native particle source %p in %s stage %u tick %u\n", (void *)effect, __FUNCTION__, game_session.stage, game_session.tick));
        diagnostic_stop(__FUNCTION__, __FILE__, __LINE__);
    }
    EFFECT_PARTICLE *particle = &effect->particle;
    uint32 result;
    if ((sint16)particle->age < 3)
    {
        result = (uint16)particle->age + 1;
        particle->age = (uint16)result;
        return result;
    }
    return update_falling_fragment(effect);
}

GDB_CALL uint32 effect_update_fragment(struct EFFECT *effect)
{
    FUNCTION_MARKER(0x8002cbb0u, "GAME.EXE");
    return update_falling_fragment(effect);
}

GDB_CALL uint32 effect_update_gate_fragment(struct EFFECT *effect)
{
    FUNCTION_MARKER(0x8002ccbcu, "GAME.EXE");
    /* TODO: Investigate unpublished or cross-format particle callback sources */
    if (!effect || effect->payload_kind != EFFECT_PAYLOAD_PARTICLE || (!(effect->kind - 897u < 13u || effect->kind < 6u || effect->kind - 6u < 2u || effect->kind - 577u < 3u || effect->kind == 583 || effect->kind - 599u < 3u || effect->kind - 72u < 4u) && effect->kind != 0xffffffffu))
    {
        DIAGNOSTIC(fprintf(stderr, "Invalid native particle source %p in %s stage %u tick %u\n", (void *)effect, __FUNCTION__, game_session.stage, game_session.tick));
        diagnostic_stop(__FUNCTION__, __FILE__, __LINE__);
    }
    EFFECT_PARTICLE *particle = &effect->particle;
    uint32 speed = (uint32)(sint32)particle->radial_speed;
    uint32 angle = (uint32)(sint32)particle->yaw, product;
    sint32 velocity;
    /* TODO: Investigate original gate-fragment reads beyond the trigonometric table */
    if ((sint32)angle < 0 || (sint32)angle >= MATH_SIN_COUNT)
    {
        DIAGNOSTIC(fprintf(stderr, "Invalid gate-fragment trig index %d for %p\n", (sint32)angle, (void *)effect));
        diagnostic_stop(__FUNCTION__, __FILE__, __LINE__);
    }
    product = speed * (uint32)(sint32)sin_table[(sint32)angle];
    particle->pos.x.bits = particle->pos.x.bits + (uint32)((sint32)product >> 1);
    speed = (uint32)(sint32)particle->radial_speed;
    angle = (uint32)(sint32)particle->yaw;
    /* TODO: Investigate original gate-fragment reads beyond the trigonometric table */
    if (MATH_QUARTER_TURN + (sint32)angle < 0 || MATH_QUARTER_TURN + (sint32)angle >= MATH_SIN_COUNT)
    {
        DIAGNOSTIC(fprintf(stderr, "Invalid gate-fragment trig index %d for %p\n", MATH_QUARTER_TURN + (sint32)angle, (void *)effect));
        diagnostic_stop(__FUNCTION__, __FILE__, __LINE__);
    }
    product = speed * (uint32)(sint32)sin_table[MATH_QUARTER_TURN + (sint32)angle];
    particle->pos.z.bits = particle->pos.z.bits - (uint32)((sint32)product >> 1);
    particle->pitch = (uint16)((uint16)particle->pitch + 280u);
    particle->roll = (uint16)((uint16)particle->roll + 456u);
    particle->radial_speed = (uint16)((uint16)particle->radial_speed - 4u);
    velocity = (sint16)particle->vertical_speed;
    if (velocity >= -255)
    {
        particle->vertical_speed = (uint16)(velocity - 6);
    }
    return update_fragment_bounce(effect);
}

/* Nonstandard direct suffix entry consumes v1 from its caller. */
GDB_CALL uint32 effect_update_gate_fragment_velocity(struct EFFECT *effect, uint32 incoming_v1)
{
    FUNCTION_MARKER(0x8002cd78u, "GAME.EXE");
    /* TODO: Investigate unpublished or cross-format particle callback sources */
    if (!effect || effect->payload_kind != EFFECT_PAYLOAD_PARTICLE || (!(effect->kind - 897u < 13u || effect->kind < 6u || effect->kind - 6u < 2u || effect->kind - 577u < 3u || effect->kind == 583 || effect->kind - 599u < 3u || effect->kind - 72u < 4u) && effect->kind != 0xffffffffu))
    {
        DIAGNOSTIC(fprintf(stderr, "Invalid native particle source %p in %s stage %u tick %u\n", (void *)effect, __FUNCTION__, game_session.stage, game_session.tick));
        diagnostic_stop(__FUNCTION__, __FILE__, __LINE__);
    }
    EFFECT_PARTICLE *particle = &effect->particle;
    particle->vertical_speed = (uint16)(incoming_v1 - 6u);
    return update_fragment_bounce(effect);
}
