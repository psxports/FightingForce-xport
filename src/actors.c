#include <stddef.h>
#include <stdio.h>
#include <string.h>

#include "game.h"

sint16 actor_types[ACTOR_COUNT];
FIXED_VEC3 actor_positions[ACTOR_COUNT];
ACTOR_ORIENTATION actor_orientation[ACTOR_COUNT];
ACTOR_HEALTH actor_health[ACTOR_COUNT];
ACTOR_MOVEMENT actor_movement[ACTOR_COUNT];
ACTOR_CONTACT actor_contacts[ACTOR_COUNT];
ACTOR_LIFECYCLE actor_lifecycle[ACTOR_COUNT];
uint16 actor_bonus_counts[ACTOR_COUNT];
uint16 actor_bonus_totals[ACTOR_COUNT];
sint16 actor_companions[ACTOR_COUNT];
ACTOR_PAIR_MOTION actor_pair_motion[ACTOR_COUNT];
ACTOR_TRAIL actor_trails[ACTOR_COUNT];
ACTOR_COMBAT actor_combat[ACTOR_COUNT];
ACTOR_STATUS actor_status[ACTOR_COUNT];
sint16 actor_grapple_distance[ACTOR_TYPE_COUNT];
uint32 actor_score_vals[ACTOR_TYPE_COUNT];
ACTOR_LIGHTING actor_lighting[ACTOR_COUNT];
uint32 actor_ambient_shade;
ACTOR_TARGETING actor_targeting;

_Static_assert(sizeof(ACTOR_LIGHTING) == 4, "Actor flags and RGB word");
_Static_assert(offsetof(ACTOR_LIGHTING, flags) == 0, "Actor flag byte");
_Static_assert(offsetof(ACTOR_LIGHTING, r) == 1, "Actor red byte");
_Static_assert(offsetof(ACTOR_LIGHTING, g) == 2, "Actor green byte");
_Static_assert(offsetof(ACTOR_LIGHTING, b) == 3, "Actor blue byte");

typedef struct ACTOR_ARC_RULE
{
    sint32 attachment0, attachment1;
    uint32 type;
    uint32 flash_mask;
} ACTOR_ARC_RULE;

/* Both original readonly arc tables contain these same sixteen rules */
static const ACTOR_ARC_RULE arc_rules[16] = {
    {3, 7, 49, 4}, {2, 6, 50, 0}, {5, 9, 51, 0}, {8, 9, 49, 0}, {3, 9, 50, 4}, {2, 8, 51, 4}, {7, 9, 49, 0}, {6, 8, 50, 4}, {6, 9, 51, 0}, {7, 8, 49, 0}, {7, 1, 50, 4}, {3, 2, 51, 4}, {6, 1, 49, 0}, {9, 8, 50, 4}, {7, 6, 51, 0}, {5, 4, 49, 4},
};

uint32 actors_reset(void)
{
    FUNCTION_MARKER(0x80013848, "GAME.EXE");
    static const uint8 damage_out[5] = {96, 88, 112, 80, 96};
    static const uint8 damage_in[5] = {128, 136, 112, 144, 128};
    uint32 i, result = 2;
    for (i = 0; i < ACTOR_COUNT; i++)
    {
        actor_status[i].flags = 0;
        actor_animations[i].src = (ACTOR_ANIMATION_SOURCE){ACTOR_ANIMATION_SOURCE_NONE, {0}};
        actor_animations[i].frame = 0xffffffff;
        actor_animations[i].selector = 0;
        actor_animations[i].id = 69;
        actor_health[i].stun = 0;
        actor_contacts[i].target_ticks = 0;
        actor_contacts[i].last_target = ACTOR_SLOT_NONE;
        actor_contacts[i].captor = ACTOR_SLOT_NONE;
        actor_items[i].slot[ACTOR_ITEM_LEFT] = -1;
        actor_items[i].slot[ACTOR_ITEM_RIGHT] = -1;
        actor_orientation[i] = (ACTOR_ORIENTATION){0};
        if (i >= 2)
            actor_types[i] = ACTOR_TYPE_NONE;
        actor_lifecycle[i].death_ticks = actor_health[i].hp < 1 ? 62 : 0;
    }
    memset(actor_poses, 0, ACTOR_POSE_ACTOR_COUNT * sizeof(*actor_poses));
    for (i = 0; i < 2; i++)
    {
        uint32 first = (NPC_BEHAVIOR_NPC_PROFILE_COUNT + i) * NPC_BEHAVIOR_ROWS_PER_PROFILE;
        sint32 type;
        uint32 val;
        if (i && game_session.player_count != 2)
            break;
        type = actor_types[i];
        if (type == -1)
            continue;
        XASSERT(type >= 0 && type <= 4);
        npc_behavior_actors[i].profile = (NPC_BEHAVIOR_REFERENCE){npc_behavior_rows, NPC_BEHAVIOR_ROW_COUNT, first};
        npc_behavior_actors[i].behavior = 0;
        npc_behavior_rows[first].damage_out = damage_out[type];
        npc_behavior_rows[first].damage_in = damage_in[type];
        character_actor_names[i] = character_name_types[type];
        val = character_actor_names[i];
        if (i)
            result = val;
    }
    return result;
}

/* Only type -1 is inactive; retain flags and inactive actor colors */
uint32 actors_reset_health(void)
{
    FUNCTION_MARKER(0x80014498, "GAME.EXE");
    uint32 i;
    player_scores[1].val = 0;
    player_scores[0].val = 0;
    player_scores[1].continue_threshold = 50000;
    player_scores[0].continue_threshold = 50000;
    level_transition.bonus_item_pending = 0;
    for (i = 0; i < ACTOR_COUNT; i++)
        if (actor_types[i] != ACTOR_TYPE_NONE)
        {
            actor_health[i].shown = 380;
            actor_health[i].trail = 380;
            actor_health[i].hp = 380;
            actor_health[i].max = 384;
            actor_lighting[i].b = 0;
            actor_lighting[i].g = 0;
            actor_lighting[i].r = 0;
        }
    level_wave.population_gate = 4;
    character_name_cursor[0] = 0;
    character_name_cursor[1] = 0;
    return 4;
}

/* Damage preserves low-word products, signed shifts and halfword wrapping */
GDB_CALL uint32 actor_apply_damage(sint32 attacker, sint32 victim, sint32 damage)
{
    FUNCTION_MARKER(0x8001f230, "GAME.EXE");
    uint32 amount, entry, stun, mode;
    int has_attacker = 0;
    NPC_BEHAVIOR_REFERENCE base;
    uint16 hp;
    sint32 coefficient;
    uint32 difficulty;
    sint32 partner;
    if ((uint32)victim >= ACTOR_COUNT)
        return 0;
    ACTOR_HEALTH *health = actor_health + victim;
    ACTOR_CONTACT *contact = actor_contacts + victim;
    if (damage > 0)
    {
        has_attacker = 1;
        XASSERT((uint32)attacker < NPC_BEHAVIOR_ACTOR_COUNT);
        if (attacker < 2 && victim >= 2)
        {
            entry = npc_behavior_actors[attacker].behavior;
            base = npc_behavior_actors[attacker].profile;
            difficulty = game_settings.difficulty;
            entry = (base.first + entry) & NPC_BEHAVIOR_ROW_MASK;
            XASSERT(base.rows && entry < base.row_count);
            coefficient = base.rows[entry].damage_out;
            coefficient = (sint32)(coefficient * (256 + ((1 - difficulty) << 7))) >> 8;
        }
        else if (attacker >= 2 && victim < 2)
        {
            difficulty = game_settings.difficulty;
            entry = npc_behavior_actors[attacker].behavior;
            base = npc_behavior_actors[attacker].profile;
            entry = (base.first + entry) & NPC_BEHAVIOR_ROW_MASK;
            XASSERT(base.rows && entry < base.row_count);
            coefficient = base.rows[entry].damage_out;
            coefficient = (sint32)(coefficient * (256 - ((1 - difficulty) << 7))) >> 8;
        }
        else
        {
            entry = npc_behavior_actors[attacker].behavior;
            base = npc_behavior_actors[attacker].profile;
            entry = (base.first + entry) & NPC_BEHAVIOR_ROW_MASK;
            XASSERT(base.rows && entry < base.row_count);
            coefficient = base.rows[entry].damage_out;
        }
        if (coefficient <= 0)
            coefficient = 1;
        amount = (sint32)(damage * (uint32)coefficient) >> 8;
    }
    else
        amount = (uint32)(-(sint64)(damage));
    entry = npc_behavior_actors[victim].behavior;
    base = npc_behavior_actors[victim].profile;
    entry = (base.first + entry) & NPC_BEHAVIOR_ROW_MASK;
    XASSERT(base.rows && entry < base.row_count);
    amount = (sint32)(amount * base.rows[entry].damage_in) >> 8;
    if ((sint32)amount <= 0)
        amount = 1;
    if (has_attacker)
        actor_contacts[attacker].last_target = (uint8)victim;
    stun = (uint16)health->stun;
    contact->last_attacker = (uint8)attacker;
    mode = game_session.mode;
    health->stun = (uint16)(stun + amount);
    if (mode == 2 || victim >= 2 || !(game_settings.flags & GAME_FLAG_INVULNERABILITY))
    {
        hp = ((uint16)health->hp) - amount;
        health->hp = hp;
        if ((sint16)hp < 0)
            health->hp = 0;
    }
    partner = contact->captive;
    if (partner == -1)
        return 0xfffffff0;
    actor_contacts[(uint32)partner].captor = ACTOR_SLOT_NONE;
    contact->captive = ACTOR_SLOT_NONE;
    return 244u * partner;
}

/* Record the last target and preserve attacker death bookkeeping */
GDB_CALL sint32 actors_rec_hit(sint32 idx, sint32 victim)
{
    FUNCTION_MARKER(0x8001b850, "GAME.EXE");
    if (victim < 2 && game_session.mode != 2)
        return 2;
    if (actor_health[idx].hp <= 0)
        actors_rec_death(idx);
    actor_contacts[idx].last_target = (uint8)victim;
    actor_contacts[idx].target_ticks = 60;
    return 60;
}

/* Capture a local offset in the shared paired-action coordinates */
GDB_CALL sint32 actors_capture_pair_offset(sint32 idx, sint32 partner)
{
    FUNCTION_MARKER(0x800235c4u, "GAME.EXE");
    ACTOR_PAIR_MOTION *pair = actor_pair_motion + idx;
    uint32 dx = actor_positions[partner].x.whole - actor_positions[idx].x.whole, dz = actor_positions[partner].z.whole - actor_positions[idx].z.whole;
    uint32 angle = 2u * (actor_orientation[idx].yaw), s = sin_table[angle >> 1], c = sin_table[MATH_QUARTER_TURN + (angle >> 1)];
    sint32 x = (sint32)(dx * c + dz * s) >> 15, z = (sint32)(dz * c - dx * s) >> 15;
    pair->x = (uint16)x;
    pair->z = (uint16)z;
    return x;
}

GDB_CALL uint32 actors_apply_pair_offset(sint32 idx, sint32 partner)
{
    FUNCTION_MARKER(0x80023664u, "GAME.EXE");
    const ACTOR_PAIR_MOTION *pair = actor_pair_motion + idx;
    uint32 x, z, angle, s, c, dx, dz, v;
    x = pair->x;
    z = pair->z;
    angle = 2 * (((uint32)(-(sint64)(actor_orientation[idx].yaw))) & 2047);
    s = sin_table[angle >> 1];
    c = sin_table[MATH_QUARTER_TURN + (angle >> 1)];
    dx = (x * c + z * s) << 1;
    dz = (z * c - x * s) << 1;
    actor_positions[partner].x.bits = dx + actor_positions[idx].x.bits;
    actor_positions[partner].z.bits = dz + actor_positions[idx].z.bits;
    v = ((uint16)actor_orientation[idx].yaw) ^ 1024;
    actor_orientation[partner].yaw = (uint16)v;
    return v;
}

GDB_CALL sint32 actors_update_pair_motion(sint32 idx)
{
    FUNCTION_MARKER(0x80023c80, "GAME.EXE");
    const ACTOR_PAIR_MOTION *pair = actor_pair_motion + idx;
    uint32 angle, center;
    sint32 frame = actor_animations[idx].frame;
    sint32 len;
    sint32 phase;
    uint32 distance;
    uint32 amplitude;
    uint32 trig;
    if (frame < 0)
        return 0;
    len = pair->duration;
    if (frame >= len)
        return 0;
    phase = 1040 * frame;
    len = pair->duration;
    XASSERT(len && (len != -1 || (uint32)phase != 0x80000000));
    phase /= len;
    if (phase > 1024)
        phase = 1024;
    amplitude = pair->span;
    trig = sin_table[MATH_QUARTER_TURN + (uint32)phase];
    distance = ((sint32)(amplitude * trig)) >> 15;
    center = (uint32)(sint32)pair->x << 16;
    angle = 2u * (actor_orientation[idx].yaw);
    trig = sin_table[angle >> 1];
    actor_positions[idx].x.bits = center - (trig * distance);
    center = (uint32)(sint32)pair->z << 16;
    angle = 2u * (actor_orientation[idx].yaw);
    trig = sin_table[MATH_QUARTER_TURN + (angle >> 1)];
    actor_positions[idx].z.bits = center + (trig * distance);
    return 1;
}

/* Hold the captive opposite the captor and release after collision correction */
GDB_CALL sint32 actors_update_grapple(sint32 idx, sint32 victim)
{
    FUNCTION_MARKER(0x8004ada8u, "GAME.EXE");
    uint32 state, distance, s, c, x, z, original_x, original_z;
    sint32 yaw;
    if (actor_contacts[victim].grapple_mode == 1)
        return 2;
    if (actor_contacts[idx].grapple_mode == 2)
        return 172;
    state = actor_animations[victim].id;
    if (state == 172 || state == 77)
        return 77;
    /* TODO: Investigate grapple types outside the original distance table */
    XASSERT(actor_types[idx] >= 0 && actor_types[idx] < ACTOR_TYPE_COUNT);
    distance = actor_grapple_distance[actor_types[idx]];
    actor_orientation[victim].yaw = (uint16)((uint16)actor_orientation[idx].yaw ^ 1024u);
    yaw = actor_orientation[idx].yaw;
    /* TODO: Investigate grapple headings outside the original sine table */
    XASSERT(yaw >= 0 && yaw < MATH_SIN_COUNT);
    s = sin_table[yaw];
    yaw = actor_orientation[idx].yaw;
    /* TODO: Investigate grapple headings outside the original cosine view */
    XASSERT(yaw >= -MATH_QUARTER_TURN && yaw < MATH_ANGLE_COUNT);
    c = sin_table[MATH_QUARTER_TURN + yaw];
    original_x = x = actor_positions[idx].x.bits + distance * s;
    original_z = z = actor_positions[idx].z.bits - distance * c;
    actor_correct_obstacles(victim, &x, &z);
    if (x != original_x || z != original_z)
    {
        actor_contacts[victim].captor = ACTOR_SLOT_NONE;
        actor_contacts[idx].captive = ACTOR_SLOT_NONE;
        return -1;
    }
    actor_positions[victim].x.bits = x;
    actor_positions[victim].z.bits = z;
    return z;
}

/* Acquire a mutually facing eligible actor for the paired grapple animations */
GDB_CALL sint32 actors_try_grapple(sint32 idx, sint32 victim)
{
    FUNCTION_MARKER(0x80047ac0u, "GAME.EXE");
    uint32 table, other_table, angle, state;
    if (victim == -1)
        return 0;
    table = animation_type_banks[(uint32)actor_types[idx]];
    angle = math_dir16(actor_positions[idx].x.whole, actor_positions[idx].z.whole, actor_positions[victim].x.whole, actor_positions[victim].z.whole);
    if ((uint16)(angle - ((uint32)(uint16)actor_orientation[idx].yaw << 5) + 4096u) >= 8193u)
        return 0;
    angle = math_dir16(actor_positions[victim].x.whole, actor_positions[victim].z.whole, actor_positions[idx].x.whole, actor_positions[idx].z.whole);
    if ((uint16)(angle - ((uint32)(uint16)actor_orientation[victim].yaw << 5) + 4096u) >= 8193u)
        return 0;
    if (dir_distance >= 257 || !actor_health[victim].hp || actor_contacts[victim].captor != -1)
        return 0;
    state = npc_base_animation(victim);
    if (state != 69 && state != 82 && state != 103 && state != 113 && state != 108)
        return 0;
    actor_contacts[victim].captor = (uint8)idx;
    actor_contacts[victim].grapple_mode = 0;
    actor_contacts[idx].captive = (uint8)victim;
    actor_contacts[victim].escape_ticks = 127;
    sound_play_effort_voice(idx);
    other_table = animation_type_banks[(uint32)actor_types[victim]];
    if (idx < 2)
        animation_start_held(idx, table, 166);
    else
        npc_start_animation(idx, table, 166);
    if (victim < 2)
        animation_start_held(victim, other_table, 171);
    else
        npc_start_animation(victim, other_table, 171);
    return 1;
}

GDB_CALL uint32 actors_emit_arc(sint32 idx)
{
    FUNCTION_MARKER(0x80040cf4u, "GAME.EXE");
    uint32 v = actor_status[idx].flags & ACTOR_CULLED, rule_idx, mask;
    FIXED_VEC3 start, end;
    ACTOR_LIGHTING *light;
    const ACTOR_ARC_RULE *rule;
    if (v)
        return v;
    rule_idx = random_next() & 15u;
    rule = arc_rules + rule_idx;
    actor_attachment_pos(idx, rule->attachment0, start.bits);
    actor_attachment_pos(idx, rule->attachment1, end.bits);
    effect_spawn_line(rule->type, start.x.bits, start.y.bits, start.z.bits, end.x.bits, end.y.bits, end.z.bits, 0x02000200u, 0x00c0c0c0u);
    XASSERT((uint32)idx < ACTOR_COUNT);
    light = actor_lighting + idx;
    v = light->bits;
    mask = rule->flash_mask;
    v |= mask;
    light->bits = v;
    return v;
}

GDB_CALL intptr actors_emit_link(sint32 idx, sint32 other_idx)
{
    FUNCTION_MARKER(0x80040e14u, "GAME.EXE");
    uint32 rule_idx, flags, mask;
    FIXED_VEC3 start, end;
    ACTOR_LIGHTING *light, *other_light;
    const ACTOR_ARC_RULE *rule;
    if (actor_status[idx].flags & ACTOR_CULLED)
        return 2;
    if (actor_status[other_idx].flags & ACTOR_CULLED)
        return 2;
    rule_idx = random_next() & 15u;
    rule = arc_rules + rule_idx;
    actor_attachment_pos(idx, rule->attachment0, start.bits);
    actor_attachment_pos(other_idx, rule->attachment1, end.bits);
    effect_spawn_line(rule->type, start.x.bits, start.y.bits, start.z.bits, end.x.bits, end.y.bits, end.z.bits, 0x02000200u, 0x028090f0u);
    XASSERT((uint32)idx < ACTOR_COUNT);
    light = actor_lighting + idx;
    flags = light->bits;
    mask = rule->flash_mask;
    light->bits = flags | mask;
    XASSERT((uint32)other_idx < ACTOR_COUNT);
    other_light = actor_lighting + other_idx;
    flags = other_light->bits;
    mask = rule->flash_mask;
    other_light->bits = flags | mask;
    return (intptr)effect_spawn_timed(871, end.x.bits, end.y.bits, end.z.bits, 0, 0, 0);
}

/* 49864: nearest eligible front/rear target. Preserve the original right-hand
 * test: only505/506 extend reach; the signed comparison against -503 is dead */
GDB_CALL uint32 actors_find_facing_targets(sint32 idx)
{
    FUNCTION_MARKER(0x80049864, "GAME.EXE");
    /* TODO: Investigate a facing search outside the native actor pool */
    XASSERT(idx >= 0 && idx < ACTOR_COUNT);
    const FIXED_VEC3 *pos = actor_positions + idx;
    uint32 i, type, angle;
    sint32 reach = 0x2400000, cone = 16384, front = 0x7fffffff, rear = 0x7fffffff, slot, dx, dz, distance, mode;
    if (actor_items[idx].slot[ACTOR_ITEM_LEFT] != -1)
    {
        reach = 0x3fff0000;
        cone = 8192;
    }
    slot = actor_items[idx].slot[ACTOR_ITEM_RIGHT];
    if (slot != -1)
    {
        /* TODO: Investigate a facing-search item outside the native pool */
        XASSERT(slot >= 0 && slot < DYNAMIC_OBJECT_COUNT);
        const MODEL_OBJECT *item = dynamic_objects + slot;
        type = item->kind;
        if (type - 505u < 2u)
        {
            reach = 0x3fff0000;
            cone = 8192;
        }
    }
    actor_targeting.front.available = 0;
    actor_targeting.rear.available = 0;
    for (i = 0; i < ACTOR_COUNT; i++)
    {
        const FIXED_VEC3 *other_pos = actor_positions + i;
        if (i == (uint32)idx)
            continue;
        mode = game_session.mode;
        if (mode < 2 && i < 2)
            continue;
        if (mode == 2 && i >= 2)
            return 0;
        slot = actor_types[i];
        if (slot == -1 || slot == 17 || actor_health[i].hp <= 0 || other_pos->y.bits)
            continue;
        dx = pos->x.bits - other_pos->x.bits;
        if (dx < 0)
            dx = (uint32)(-(sint64)(dx));
        dz = pos->z.bits - other_pos->z.bits;
        if (dz < 0)
            dz = (uint32)(-(sint64)(dz));
        distance = dz < dx ? (sint32)(dx + (uint32)(dz >> 1)) : (sint32)(dz + (uint32)(dx >> 1));
        if (distance >= reach)
            continue;
        angle = math_dir16(pos->x.whole, pos->z.whole, other_pos->x.whole, other_pos->z.whole) - 32u * ((uint16)actor_orientation[idx].yaw);
        if (distance < front && (sint16)angle < cone && -cone < (sint16)angle)
        {
            front = distance;
            actor_targeting.front.available = 1;
            actor_targeting.actor_idx = i;
            actor_targeting.front.angle = (uint16)angle;
            actor_targeting.near_distance = distance >> 16;
            actor_targeting.rear.available = 0;
        }
        if (!actor_targeting.front.available && distance < rear && (uint16)(angle + 16383u) >= 32767u)
        {
            rear = distance;
            actor_targeting.rear.available = 1;
            actor_targeting.actor_idx = i;
            actor_targeting.rear.angle = (uint16)(angle - 32768u);
        }
    }
    return 0;
}

GDB_CALL sint32 actors_detach_companion(sint32 idx)
{
    FUNCTION_MARKER(0x8001ce14, "GAME.EXE");
    sint32 type = actor_types[idx], partner;
    if (((uint16)type) - 44u >= 2u && type != 46)
        return 0;
    partner = actor_companions[idx];
    if (partner == -1)
        return 0;
    actor_companions[idx] = ACTOR_SLOT_NONE;
    /* TODO: Investigate invalid original companion writes */
    XASSERT(partner >= 0 && partner < ACTOR_COUNT);
    actor_companions[partner] = ACTOR_SLOT_NONE;
    animation_start(idx, animation_type_banks[(uint32)type], 281);
    return 1;
}

/* Running attack voice and ground effect with two offset RNG draws */
GDB_CALL struct EFFECT *actors_emit_running_attack(sint32 idx)
{
    FUNCTION_MARKER(0x80033374, "GAME.EXE");
    ACTOR_COMBAT *combat = actor_combat + idx;
    uint32 x, z, random;
    sint32 kind = actor_types[idx];
    if (!actor_animations[idx].event_step)
    {
        if (kind == 2)
        {
            combat->run_voice = (uint16)(sound_voice_play(91, 80, 6144) + 1u);
            sound_voice_play(92, 255, 6144);
        }
        else
            sound_voice_play(56, 255, 6144);
    }
    if (actor_animations[idx].motion.events & 2048u)
    {
        sound_voice_stop(combat->run_voice - 1);
        combat->run_voice = 0;
        sound_voice_play(92, 255, 6144);
    }
    random = random_next();
    x = (((random & 31u) - 16u) << 16) + actor_positions[idx].x.bits;
    random = random_next();
    z = (((random & 31u) - 16u) << 16) + actor_positions[idx].z.bits;
    return effect_spawn_timed(871, x, 0, z, 0, 0, 0);
}

GDB_CALL uint32 actors_init_score_vals(void)
{
    FUNCTION_MARKER(0x8003ed10, "GAME.EXE");
    actor_score_vals[0] = 100;     /* 8003ED1C */
    actor_score_vals[1] = 100;     /* 8003ED20 */
    actor_score_vals[2] = 100;     /* 8003ED24 */
    actor_score_vals[3] = 100;     /* 8003ED28 */
    actor_score_vals[4] = 100;     /* 8003ED2C */
    actor_score_vals[5] = 100;     /* 8003ED30 */
    actor_score_vals[6] = 200;     /* 8003ED38 */
    actor_score_vals[7] = 150;     /* 8003ED40 */
    actor_score_vals[8] = 150;     /* 8003ED44 */
    actor_score_vals[9] = 250;     /* 8003ED4C */
    actor_score_vals[10] = 200;    /* 8003ED50 */
    actor_score_vals[11] = 100;    /* 8003ED54 */
    actor_score_vals[12] = 750;    /* 8003ED5C */
    actor_score_vals[14] = 150000; /* 8003ED68 */
    actor_score_vals[15] = 100;    /* 8003ED6C */
    actor_score_vals[18] = 350;    /* 8003ED74 */
    actor_score_vals[19] = 450;    /* 8003ED7C */
    actor_score_vals[20] = 550;    /* 8003ED84 */
    actor_score_vals[21] = 650;    /* 8003ED8C */
    actor_score_vals[22] = 800;    /* 8003ED94 */
    actor_score_vals[23] = 500;    /* 8003ED9C */
    actor_score_vals[24] = 650;    /* 8003EDA0 */
    actor_score_vals[25] = 750;    /* 8003EDA4 */
    actor_score_vals[26] = 1250;   /* 8003EDAC */
    actor_score_vals[27] = 300;    /* 8003EDB4 */
    actor_score_vals[28] = 100;    /* 8003EDB8 */
    actor_score_vals[29] = 1000;   /* 8003EDC0 */
    actor_score_vals[30] = 800;    /* 8003EDC4 */
    actor_score_vals[31] = 500;    /* 8003EDC8 */
    actor_score_vals[32] = 10000;  /* 8003EDD0 */
    actor_score_vals[33] = 1250;   /* 8003EDD4 */
    actor_score_vals[34] = 1500;   /* 8003EDDC */
    actor_score_vals[35] = 600;    /* 8003EDE4 */
    actor_score_vals[36] = 800;    /* 8003EDE8 */
    actor_score_vals[37] = 1000;   /* 8003EDEC */
    actor_score_vals[38] = 1000;   /* 8003EDF0 */
    actor_score_vals[39] = 1200;   /* 8003EDF8 */
    actor_score_vals[40] = 1000;   /* 8003EDFC */
    actor_score_vals[41] = 1250;   /* 8003EE00 */
    actor_score_vals[42] = 1500;   /* 8003EE04 */
    actor_score_vals[43] = 1700;   /* 8003EE0C */
    actor_score_vals[44] = 500;    /* 8003EE10 */
    actor_score_vals[45] = 600;    /* 8003EE14 */
    actor_score_vals[46] = 700;    /* 8003EE1C */
    actor_score_vals[47] = 900;    /* 8003EE24 */
    actor_score_vals[48] = 75000;  /* 8003EE30 */
    actor_score_vals[49] = 25000;  /* 8003EE38 */
    actor_score_vals[50] = 75000;  /* 8003EE3C */
    actor_score_vals[52] = 500;    /* 8003EE40 */
    actor_score_vals[53] = 800;    /* 8003EE48 */
    return 0x800a8600;
}

/* Clear HP before the death latch; retain attribution and release order */
GDB_CALL sint32 actors_rec_death(sint32 idx)
{
    FUNCTION_MARKER(0x8001b8dc, "GAME.EXE");
    uint32 wave, val, points, age;
    PLAYER_SCORE *score;
    sint32 result = (sint8)actor_lifecycle[idx].death_ticks, credit, partner, type;
    actor_health[idx].hp = 0;
    if (result)
        return result;
    sound_play_death_voice(idx);
    actor_items_drop(idx);
    actor_items[idx].slot[ACTOR_ITEM_RIGHT] = -1;
    actor_items[idx].slot[ACTOR_ITEM_LEFT] = -1;
    if (idx < 2)
        return 1;
    wave = level_wave.remaining_enemies;
    credit = actor_contacts[idx].last_attacker;
    level_wave.remaining_enemies = wave - 1;
    if ((uint32)credit < 2u)
    {
        type = actor_types[idx];
        score = player_scores + credit;
        val = score->val;
        points = actor_score_vals[type];
        score->val = val + points;
        hud_format_score(credit);
    }
    age = actor_lifecycle[idx].death_ticks;
    partner = actor_contacts[idx].captor;
    actor_contacts[idx].captor = ACTOR_SLOT_NONE;
    actor_lifecycle[idx].death_ticks = (uint8)(age + 1);
    if (partner == -1)
        return -16;
    actor_contacts[(uint32)partner].captive = ACTOR_SLOT_NONE;
    return 244u * partner;
}

/* 49B78 inline distance metric: preserve wrapped ABS including INT_MIN */
static sint32 target_distance(uint32 x, uint32 z, uint32 ox, uint32 oz)
{
    uint32 dx = x - ox, dz = z - oz;
    if ((sint32)dx < 0)
        dx = (uint32)(-(sint64)(dx));
    if ((sint32)dz < 0)
        dz = (uint32)(-(sint64)(dz));
    return (sint32)dz < (sint32)dx ? dx + (uint32)((sint32)dz >> 1) : dz + (uint32)((sint32)dx >> 1);
}

GDB_CALL sint32 actors_find_nearest_targets(sint32 idx)
{
    FUNCTION_MARKER(0x80049b78u, "GAME.EXE");
    const FIXED_VEC3 *pos = actor_positions + idx;
    uint32 x, z, yaw, ox, oz, s, c;
    sint32 i, distance, count = game_session.player_count;
    actor_targeting.player_distance = 0x7fffffff;
    actor_targeting.player_idx = 0xffffffff;
    i = 0;
    while (i < count)
    {
        /* TODO: Investigate a targeting player loop outside the actor pool */
        XASSERT(i < ACTOR_COUNT);
        const FIXED_VEC3 *other_pos = actor_positions + i;
        if (actor_health[i].hp > 0)
        {
            x = pos->x.bits;
            ox = other_pos->x.bits;
            z = pos->z.bits;
            oz = other_pos->z.bits;
            distance = target_distance(x, z, ox, oz);
            if (distance < actor_targeting.player_distance)
            {
                actor_targeting.player_distance = distance;
                actor_targeting.player_idx = i;
            }
        }
        count = game_session.player_count;
        ++i;
    }
    yaw = actor_orientation[idx].yaw;
    x = pos->x.bits;
    actor_targeting.near_distance = 0x7fffffff;
    /* TODO: Investigate targeting headings outside the original sine table */
    XASSERT(yaw < MATH_SIN_COUNT);
    s = sin_table[yaw];
    z = pos->z.bits;
    /* TODO: Investigate targeting headings outside the original cosine view */
    XASSERT(yaw < MATH_ANGLE_COUNT);
    c = sin_table[MATH_QUARTER_TURN + yaw];
    x += s << 8;
    z -= c << 8;
    for (i = 2; i < ACTOR_COUNT; i++)
    {
        const FIXED_VEC3 *other_pos = actor_positions + i;
        if (actor_types[i] == ACTOR_TYPE_NONE || actor_health[i].hp <= 0 || i == idx)
            continue;
        ox = other_pos->x.bits;
        oz = other_pos->z.bits;
        distance = target_distance(x, z, ox, oz);
        if (distance < actor_targeting.near_distance)
            actor_targeting.near_distance = distance;
    }
    return 0;
}

GDB_CALL intptr actor_process_attack_effects(sint32 idx)
{
    FUNCTION_MARKER(0x8003314c, "GAME.EXE");
    uint32 state, x, z;
    if (actor_animations[idx].motion.events & 0x8000u)
    {
        if (idx >= 2)
            return 0;
        actor_trails[idx].phase = 1;
        character_trail_clear(idx + 867);
        state = actor_animations[idx].id;
        if (state == 33)
        {
            if (actor_types[idx] != 2)
            {
                actor_trails[idx].mode = 1;
                return 39;
            }
            state = actor_animations[idx].id;
        }
        if (state == 39 && actor_types[idx] == 3)
        {
            actor_trails[idx].mode = 3;
            return 3;
        }
        actor_trails[idx].mode = 2;
        return 2;
    }
    if (actor_animations[idx].motion.events & 0x800u)
        return (intptr)actor_emit_ground_effect(idx);
    if (actor_animations[idx].id != 33)
        return sound_play_action_voice(idx);
    if (actor_types[idx] == 2)
    {
        EFFECT *shockwave;
        x = actor_positions[idx].x.bits;
        z = actor_positions[idx].z.bits;
        shockwave = effect_spawn(604, x, 0, z);
        /* TODO: Investigate the original low-RAM write after allocation failure */
        XASSERT(shockwave && shockwave->payload_kind == EFFECT_PAYLOAD_SPRITE && shockwave->kind == 604);
        shockwave->spr.owner = idx;
        sound_voice_play(19, 128, 6144);
        disp_buffers.shake_ticks = 16;
        x = actor_positions[idx].x.bits;
        z = actor_positions[idx].z.bits;
        return actors_apply_radial_damage(x, z, shockwave, NULL);
    }
    return 2;
}

GDB_CALL intptr actor_process_shock_attack(sint32 idx)
{
    const POSE_MATRIX *matrix;
    FUNCTION_MARKER(0x80040facu, "GAME.EXE");
    static const FIXED_VEC3 shock_points[4] = {{.val = {4194304, 0, 0}}, {.val = {262144, 4194304, 0}}, {.val = {-8388608, 0, 0}}, {.val = {-262144, 4194304, 0}}};
    FIXED_VEC3 points[4];
    uint32 x, z;
    EFFECT *shockwave;
    int i;
    intptr result;
    if (actor_animations[idx].motion.events & 0x800u)
    {
        x = actor_positions[idx].x.bits;
        z = actor_positions[idx].z.bits;
        shockwave = effect_spawn(604, x, 0, z);
        /* TODO: Investigate the original low-RAM write after allocation failure */
        XASSERT(shockwave && shockwave->payload_kind == EFFECT_PAYLOAD_SPRITE && shockwave->kind == 604);
        shockwave->spr.owner = idx;
        sound_voice_play(19, 128, 6144);
        disp_buffers.shake_ticks = 16;
        x = actor_positions[idx].x.bits;
        z = actor_positions[idx].z.bits;
        actors_apply_radial_damage(x, z, shockwave, NULL);
    }
    if (actor_animations[idx].motion.events & 0x2000u)
        actor_status[idx].flags = actor_status[idx].flags | ACTOR_ELECTRIC;
    if (!(actor_animations[idx].motion.events & 0x4000u))
        return 0;
    memcpy(points, shock_points, sizeof(points));
    if (actor_status[idx].flags & ACTOR_CULLED)
        return 2;
    for (i = 0; i < 3; i++)
    {
        points[1].bits[i] += points[0].bits[i];
        points[3].bits[i] += points[2].bits[i];
    }
    matrix = &actor_poses[idx].matrix[3];
    actor_transform_point(matrix, points[0].bits);
    actor_transform_point(matrix, points[1].bits);
    actor_transform_point(matrix, points[2].bits);
    actor_transform_point(matrix, points[3].bits);
    effect_spawn_motion(877, points[0].x.bits, points[0].y.bits, points[0].z.bits, points[1].x.bits - points[0].x.bits, points[1].y.bits - points[0].y.bits, points[1].z.bits - points[0].z.bits);
    result = (intptr)effect_spawn_motion(877, points[2].x.bits, points[2].y.bits, points[2].z.bits, points[3].x.bits - points[2].x.bits, points[3].y.bits - points[2].y.bits, points[3].z.bits - points[2].z.bits);
    return result;
}

GDB_CALL uint32 actor_transform_point(const POSE_MATRIX *matrix, uint32 point[3])
{
    FUNCTION_MARKER(0x800234e8u, "GAME.EXE");
    uint32 x, y, z;
    transform_push();
    transform_load(matrix);
    x = point[0];
    y = point[1];
    z = point[2];
    transform_point(x, y, z, point);
    transform_identity();
    transform_yaw(-camera.angles.yaw);
    transform_pitch(-camera.angles.pitch);
    x = point[0];
    y = point[1];
    z = point[2];
    transform_point(x, y, z, point);
    transform_pop();
    x = point[0];
    x += camera.pos.x.bits;
    point[0] = x;
    y = point[1];
    y += camera.pos.y.bits;
    point[1] = y;
    z = point[2];
    z += camera.pos.z.bits;
    point[2] = z;
    return z;
}

GDB_CALL EFFECT *actor_emit_ground_effect(sint32 idx)
{
    FUNCTION_MARKER(0x80032ee0u, "GAME.EXE");
    uint32 x, z;
    sound_play_ground_voice(idx);
    x = actor_positions[idx].x.bits;
    z = actor_positions[idx].z.bits;
    return effect_spawn(607, x, 0, z);
}

GDB_CALL intptr actor_ground_effect_event(sint32 idx)
{
    FUNCTION_MARKER(0x80033008u, "GAME.EXE");
    if (actor_animations[idx].motion.events & 0x800u)
        return (intptr)actor_emit_ground_effect(idx);
    return sound_play_action_voice(idx);
}

GDB_CALL intptr actor_ground_effect_voice_event(sint32 idx)
{
    FUNCTION_MARKER(0x80033068u, "GAME.EXE");
    if (actor_animations[idx].motion.events & 0x800u)
        return (intptr)actor_emit_ground_effect(idx);
    return sound_voice_play(187, 255, 6144);
}

GDB_CALL uint32 players_past_x(sint32 x)
{
    FUNCTION_MARKER(0x800472a8u, "GAME.EXE");
    uint32 result = 1;
    if (game_session.player_count == 1)
        return x < (sint32)actor_positions[0].x.bits;
    if (actor_health[0].hp > 0)
        result = (sint32)actor_positions[0].x.bits >= x;
    if (actor_health[1].hp > 0 && (sint32)actor_positions[1].x.bits < x)
        return 0;
    return result;
}

GDB_CALL uint32 players_before_x(sint32 x)
{
    FUNCTION_MARKER(0x8004732cu, "GAME.EXE");
    uint32 result = 1;
    if (game_session.player_count == 1)
        return (sint32)actor_positions[0].x.bits < x;
    if (actor_health[0].hp > 0)
        result = x >= (sint32)actor_positions[0].x.bits;
    if (actor_health[1].hp > 0 && x < (sint32)actor_positions[1].x.bits)
        return 0;
    return result;
}

GDB_CALL uint32 players_before_z(sint32 z)
{
    FUNCTION_MARKER(0x80047434u, "GAME.EXE");
    uint32 result = 1;
    if (game_session.player_count == 1)
        return (sint32)actor_positions[0].z.bits < z;
    if (actor_health[0].hp > 0)
        result = z >= (sint32)actor_positions[0].z.bits;
    if (actor_health[1].hp > 0 && z < (sint32)actor_positions[1].z.bits)
        return 0;
    return result;
}

GDB_CALL sint32 actor_is_defeated(sint32 idx)
{
    FUNCTION_MARKER(0x800196a4u, "GAME.EXE");
    if (idx < 2)
    {
        if (actor_types[idx] == ACTOR_TYPE_NONE)
            return 1;
        if (actor_health[idx].hp != 0)
            return 0;
    }
    if (actor_types[idx] == ACTOR_TYPE_NONE)
        return 1;
    return actor_health[idx].hp <= 0;
}

GDB_CALL sint32 players_past_z(sint32 z)
{
    FUNCTION_MARKER(0x800473b0u, "GAME.EXE");
    sint32 result = 1;
    if (game_session.player_count == 1)
        return z < (sint32)actor_positions[0].z.bits;
    if (actor_health[0].hp > 0)
        result = (sint32)actor_positions[0].z.bits >= z;
    if (actor_health[1].hp > 0 && (sint32)actor_positions[1].z.bits < z)
        return 0;
    return result;
}

GDB_CALL uint32 actor_apply_animation_damage(uint32 attacker, uint32 victim, uint32 state)
{
    FUNCTION_MARKER(0x8001f41cu, "GAME.EXE");
    uint32 damage;
    XASSERT(state < ANIMATION_COUNT);
    damage = animation_flags[state] >> 8;
    if (attacker < 9u && victim < 9u)
        return actor_apply_damage(attacker, victim, damage);
    return victim < 9u;
}
