#include <stddef.h>
#include <string.h>

#include "game.h"

_Static_assert(sizeof(NPC_BEHAVIOR_ROW) == 8, "AII row size");
_Static_assert(offsetof(NPC_BEHAVIOR_ROW, capability) == 0, "AII capability offset");
_Static_assert(offsetof(NPC_BEHAVIOR_ROW, hit_delay) == 1, "AII hit_delay offset");
_Static_assert(offsetof(NPC_BEHAVIOR_ROW, recovery_delay) == 2, "AII recovery_delay offset");
_Static_assert(offsetof(NPC_BEHAVIOR_ROW, reaction_threshold) == 3, "AII reaction_threshold offset");
_Static_assert(offsetof(NPC_BEHAVIOR_ROW, pickup_bias) == 4, "AII pickup_bias offset");
_Static_assert(offsetof(NPC_BEHAVIOR_ROW, damage_out) == 5, "AII damage_out offset");
_Static_assert(offsetof(NPC_BEHAVIOR_ROW, health_units) == 6, "AII health_units offset");
_Static_assert(offsetof(NPC_BEHAVIOR_ROW, damage_in) == 7, "AII damage_in offset");

uint32 npc_behavior_file[NPC_AI_STORAGE_WORD_COUNT];
uint32 npc_ai_storage[NPC_AI_STORAGE_WORD_COUNT];
NPC_AI_CAPABILITY npc_ai_capabilities[NPC_AI_CAPABILITY_COUNT];

NPC_BEHAVIOR_ROW npc_behavior_rows[NPC_BEHAVIOR_ROW_COUNT];
NPC_BEHAVIOR_ACTOR npc_behavior_actors[NPC_BEHAVIOR_ACTOR_COUNT];
NPC_BEHAVIOR_CYCLE npc_behavior_cycle;

enum
{
    NPC_BEHAVIOR_ACTION_COUNT = 47
};

typedef struct NPC_BEHAVIOR_ACTION
{
    uint32 animation;
    uint32 weapon_flags;
} NPC_BEHAVIOR_ACTION;

static const NPC_BEHAVIOR_ACTION actions[NPC_BEHAVIOR_ACTION_COUNT] = {
    {69, 0}, {0, 0}, {1, 0}, {2, 0}, {3, 0}, {4, 0}, {5, 0}, {6, 0}, {7, 0}, {8, 0}, {26, 0}, {27, 0}, {28, 0}, {29, 0}, {30, 0}, {30, 0}, {35, 1}, {37, 1}, {38, 1}, {42, 2}, {43, 2}, {44, 2}, {45, 2}, {166, 0}, {192, 4}, {193, 32}, {54, 0}, {55, 0}, {56, 0}, {161, 16}, {46, 0}, {47, 0}, {48, 0}, {223, 0}, {224, 0}, {225, 0}, {226, 0}, {35, 1}, {37, 1}, {192, 64}, {31, 0}, {3, 0}, {9, 0}, {50, 0}, {51, 0}, {52, 0}, {53, 0},
};

/* Load stage AI command streams and behavior parameters */
GDB_CALL void npc_behavior_load(void)
{
    FUNCTION_MARKER(0x8004c1cc, "GAME.EXE");
    uint32 p = 0, end, table_idx = 0, block_end, count, initial_count;
    uint32 code_offset, j, q, flags, size, val;
    uint32 *choices;
    resource_load(game_session.stage + 120, 0x116000, npc_behavior_file, sizeof(npc_behavior_file), NULL, NULL);
    size = resource_io.file_size;
    if ((sint32)size > 0)
    {
        if (size > sizeof(npc_ai_storage))
            diagnostic_stop(__FUNCTION__, __FILE__, __LINE__);
        /* Keep bytes beyond the loaded MVS prefix */
        memcpy(npc_ai_storage, npc_behavior_file, size);
    }
    size = resource_io.file_size;
    if (size >= 0x1801)
        diagnostic_stop(__FUNCTION__, __FILE__, __LINE__); /* Original infinite loop at4C224 */
    size = resource_io.file_size;
    end = size >> 2;
    while (p < end)
    {
        if (table_idx >= NPC_AI_CAPABILITY_COUNT || p > NPC_AI_STORAGE_WORD_COUNT - 2)
            diagnostic_stop(__FUNCTION__, __FILE__, __LINE__);
        NPC_AI_CAPABILITY *table = npc_ai_capabilities + table_idx;
        block_end = (p + npc_ai_storage[p]) & NPC_AI_WORD_MASK;
        if (block_end <= p || block_end > NPC_AI_STORAGE_WORD_COUNT)
            diagnostic_stop(__FUNCTION__, __FILE__, __LINE__);
        ++p;
        count = npc_ai_storage[p];
        table->choice_count = count;
        count = table->choice_count;
        initial_count = table->choice_count;
        ++p;
        code_offset = (p + count) & NPC_AI_WORD_MASK;
        if (p > NPC_AI_STORAGE_WORD_COUNT || code_offset > NPC_AI_STORAGE_WORD_COUNT)
            diagnostic_stop(__FUNCTION__, __FILE__, __LINE__);
        table->choices = npc_ai_storage + p;
        table->choice_capacity = NPC_AI_STORAGE_WORD_COUNT - p;
        table->weapon_flags = 0;
        table->code = npc_ai_storage + code_offset;
        table->code_capacity = NPC_AI_STORAGE_WORD_COUNT - code_offset;
        j = 0;
        if ((sint32)initial_count > 0)
        {
            do
            {
                choices = table->choices;
                if (!choices || j >= table->choice_capacity)
                    diagnostic_stop(__FUNCTION__, __FILE__, __LINE__);
                val = choices[j];
                q = (code_offset + val) & NPC_AI_WORD_MASK;
                do
                {
                    if (q > NPC_AI_STORAGE_WORD_COUNT - 3)
                        diagnostic_stop(__FUNCTION__, __FILE__, __LINE__);
                    flags = npc_ai_storage[q];
                    if (flags >= NPC_BEHAVIOR_ACTION_COUNT)
                        diagnostic_stop(__FUNCTION__, __FILE__, __LINE__);
                    flags = actions[flags].weapon_flags;
                    if (flags)
                    {
                        choices = table->choices;
                        if (!choices || j >= table->choice_capacity)
                            diagnostic_stop(__FUNCTION__, __FILE__, __LINE__);
                        val = choices[j];
                        choices[j] = (flags << 16) | val | 0x80000000;
                    }
                    flags = npc_ai_storage[q + 2];
                    q = (q + 3) & NPC_AI_WORD_MASK;
                } while (flags);
                count = table->choice_count;
                ++j;
            } while ((sint32)j < (sint32)count);
        }
        for (q = code_offset + 1; q < block_end; q += 3)
        {
            flags = npc_ai_storage[q];
            val = table->weapon_flags;
            if (flags >= NPC_BEHAVIOR_ACTION_COUNT)
                diagnostic_stop(__FUNCTION__, __FILE__, __LINE__);
            flags = actions[flags].weapon_flags;
            table->weapon_flags = val | flags;
            flags = npc_ai_storage[q];
            if (flags >= NPC_BEHAVIOR_ACTION_COUNT)
                diagnostic_stop(__FUNCTION__, __FILE__, __LINE__);
            npc_ai_storage[q] = actions[flags].animation;
        }
        ++table_idx;
        p = block_end;
    }
    resource_load(game_session.stage + 145, 0x116000, npc_behavior_file, sizeof(npc_behavior_file), NULL, NULL);
    size = resource_io.file_size;
    if ((sint32)size > 0)
    {
        if (size > sizeof(npc_behavior_rows))
            diagnostic_stop(__FUNCTION__, __FILE__, __LINE__);
        /* Preserve the untouched tail of shorter AII files */
        memcpy(npc_behavior_rows, npc_behavior_file, size);
    }
}
