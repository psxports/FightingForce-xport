#include "game.h"

/* Begin character loading with the original scratch cursor and four cleared banks */
uint32 character_resources_begin(void)
{
    FUNCTION_MARKER(0x80013a30, "GAME.EXE");
    uint32 i;
    resource_arenas.scratch = (RESOURCE_CURSOR){resource_storage.data + RESOURCE_SCRATCH_OFFSET, RESOURCE_STORAGE_CAPACITY - RESOURCE_SCRATCH_OFFSET, 0x116000};
    for (i = 0; i < ANIMATION_COUNT; i++)
    {
        animation_banks[ANIMATION_BANK_HAWK].stream[i] = (ACTOR_ANIMATION_SOURCE){0};
        animation_banks[ANIMATION_BANK_MACE].stream[i] = (ACTOR_ANIMATION_SOURCE){0};
        animation_banks[ANIMATION_BANK_SMASHER].stream[i] = (ACTOR_ANIMATION_SOURCE){0};
        animation_banks[ANIMATION_BANK_ALANA].stream[i] = (ACTOR_ANIMATION_SOURCE){0};
    }
    return 0;
}

GDB_CALL uint32 character_load_hawk(void)
{
    FUNCTION_MARKER(0x80013a98u, "GAME.EXE");
    resource_load(76, resource_arenas.scratch.context, NULL, 0, NULL, &resource_arenas.scratch);
    RESOURCE_VIEW dummy = resource_find("DUMMY.DAT");
    {
        uintptr_t ptr = (uintptr_t)dummy.data, base = (uintptr_t)resource_storage.data;
        /* TODO: Investigate a DUMMY cursor outside its actual native resource allocation */
        if (ptr < base || ptr - base > RESOURCE_STORAGE_CAPACITY)
        {
            DIAGNOSTIC(fprintf(stderr, "DUMMY resource cursor outside native storage context %08X stage %u tick %u\n", (uint32)(dummy.context_address), game_session.stage, game_session.tick));
            diagnostic_stop(__FUNCTION__, __FILE__, __LINE__);
        }
        resource_arenas.scratch = (RESOURCE_CURSOR){dummy.data, RESOURCE_STORAGE_CAPACITY - (uint32)(ptr - base), dummy.context_address};
    }
    resource_load_character("HAWK.ACH", 0, 36);
    return resource_load_animations("HAWK.ADT", 0);
}

GDB_CALL uint32 character_load_hawk2(void)
{
    FUNCTION_MARKER(0x80013afcu, "GAME.EXE");
    resource_load(80, resource_arenas.scratch.context, NULL, 0, NULL, &resource_arenas.scratch);
    RESOURCE_VIEW dummy = resource_find("DUMMY.DAT");
    {
        uintptr_t ptr = (uintptr_t)dummy.data, base = (uintptr_t)resource_storage.data;
        /* TODO: Investigate a DUMMY cursor outside its actual native resource allocation */
        if (ptr < base || ptr - base > RESOURCE_STORAGE_CAPACITY)
        {
            DIAGNOSTIC(fprintf(stderr, "DUMMY resource cursor outside native storage context %08X stage %u tick %u\n", (uint32)(dummy.context_address), game_session.stage, game_session.tick));
            diagnostic_stop(__FUNCTION__, __FILE__, __LINE__);
        }
        resource_arenas.scratch = (RESOURCE_CURSOR){dummy.data, RESOURCE_STORAGE_CAPACITY - (uint32)(ptr - base), dummy.context_address};
    }
    resource_load_character("HAWK2.ACH", 4, 40);
    return resource_load_animations("HAWK2.ADT", 4);
}

GDB_CALL uint32 character_load_mace(void)
{
    FUNCTION_MARKER(0x80013b60u, "GAME.EXE");
    resource_load(77, resource_arenas.scratch.context, NULL, 0, NULL, &resource_arenas.scratch);
    RESOURCE_VIEW dummy = resource_find("DUMMY.DAT");
    {
        uintptr_t ptr = (uintptr_t)dummy.data, base = (uintptr_t)resource_storage.data;
        /* TODO: Investigate a DUMMY cursor outside its actual native resource allocation */
        if (ptr < base || ptr - base > RESOURCE_STORAGE_CAPACITY)
        {
            DIAGNOSTIC(fprintf(stderr, "DUMMY resource cursor outside native storage context %08X stage %u tick %u\n", (uint32)(dummy.context_address), game_session.stage, game_session.tick));
            diagnostic_stop(__FUNCTION__, __FILE__, __LINE__);
        }
        resource_arenas.scratch = (RESOURCE_CURSOR){dummy.data, RESOURCE_STORAGE_CAPACITY - (uint32)(ptr - base), dummy.context_address};
    }
    resource_load_character("MACE.ACH", 1, 37);
    return resource_load_animations("MACE.ADT", 1);
}

GDB_CALL uint32 character_load_smasher(void)
{
    FUNCTION_MARKER(0x80013bc4u, "GAME.EXE");
    resource_load(78, resource_arenas.scratch.context, NULL, 0, NULL, &resource_arenas.scratch);
    RESOURCE_VIEW dummy = resource_find("DUMMY.DAT");
    {
        uintptr_t ptr = (uintptr_t)dummy.data, base = (uintptr_t)resource_storage.data;
        /* TODO: Investigate a DUMMY cursor outside its actual native resource allocation */
        if (ptr < base || ptr - base > RESOURCE_STORAGE_CAPACITY)
        {
            DIAGNOSTIC(fprintf(stderr, "DUMMY resource cursor outside native storage context %08X stage %u tick %u\n", (uint32)(dummy.context_address), game_session.stage, game_session.tick));
            diagnostic_stop(__FUNCTION__, __FILE__, __LINE__);
        }
        resource_arenas.scratch = (RESOURCE_CURSOR){dummy.data, RESOURCE_STORAGE_CAPACITY - (uint32)(ptr - base), dummy.context_address};
    }
    resource_load_character("SMASHER.ACH", 2, 38);
    return resource_load_animations("SMASHER.ADT", 2);
}

GDB_CALL uint32 character_load_alana(void)
{
    FUNCTION_MARKER(0x80013c28u, "GAME.EXE");
    resource_load(79, resource_arenas.scratch.context, NULL, 0, NULL, &resource_arenas.scratch);
    RESOURCE_VIEW dummy = resource_find("DUMMY.DAT");
    {
        uintptr_t ptr = (uintptr_t)dummy.data, base = (uintptr_t)resource_storage.data;
        /* TODO: Investigate a DUMMY cursor outside its actual native resource allocation */
        if (ptr < base || ptr - base > RESOURCE_STORAGE_CAPACITY)
        {
            DIAGNOSTIC(fprintf(stderr, "DUMMY resource cursor outside native storage context %08X stage %u tick %u\n", (uint32)(dummy.context_address), game_session.stage, game_session.tick));
            diagnostic_stop(__FUNCTION__, __FILE__, __LINE__);
        }
        resource_arenas.scratch = (RESOURCE_CURSOR){dummy.data, RESOURCE_STORAGE_CAPACITY - (uint32)(ptr - base), dummy.context_address};
    }
    resource_load_character("ALANA.ACH", 3, 39);
    return resource_load_animations("ALANA.ADT", 3);
}

GDB_CALL uint32 character_load_suit_variants(void)
{
    FUNCTION_MARKER(0x80013c8cu, "GAME.EXE");
    MODEL_REFERENCE model0, model1, model2;
    uint32 val;
    resource_load_character("SUIT.ACH", 5, 41);
    resource_load_character("AFROSUIT.ACH", 6, 41);
    resource_load_character("BALDSUIT.ACH", 7, 41);
    val = model_height[5];
    model0 = model_refs[5];
    model1 = model_refs[6];
    model2 = model_refs[7];
    model_height[6] = val;
    model_height[7] = val;
    model_refs[8] = model0;
    model_refs[9] = model1;
    model_refs[10] = model2;
    model_height[8] = val;
    model_height[9] = val;
    model_height[10] = val;
    return val;
}

GDB_CALL uint32 character_load_orange(void)
{
    FUNCTION_MARKER(0x80013d40u, "GAME.EXE");
    return resource_load_character("ORANGE.ACH", 11, 42);
}

GDB_CALL uint32 character_load_thug1(void)
{
    FUNCTION_MARKER(0x80013d6cu, "GAME.EXE");
    return resource_load_character("THUG1.ACH", 18, 43);
}

GDB_CALL uint32 character_load_thug2(void)
{
    FUNCTION_MARKER(0x80013d98u, "GAME.EXE");
    return resource_load_character("THUG2.ACH", 19, 43);
}

GDB_CALL uint32 character_load_thug3(void)
{
    FUNCTION_MARKER(0x80013dc4u, "GAME.EXE");
    return resource_load_character("THUG3.ACH", 20, 43);
}

GDB_CALL uint32 character_load_thugaa(void)
{
    FUNCTION_MARKER(0x80013df0u, "GAME.EXE");
    return resource_load_character("THUGAA.ACH", 21, 43);
}

GDB_CALL uint32 character_load_thugbb(void)
{
    FUNCTION_MARKER(0x80013e1cu, "GAME.EXE");
    return resource_load_character("THUGBB.ACH", 22, 43);
}

GDB_CALL uint32 character_load_afrotg(void)
{
    FUNCTION_MARKER(0x80013e48u, "GAME.EXE");
    return resource_load_character("AFROTG.ACH", 23, 43);
}

GDB_CALL uint32 character_load_afrotg2(void)
{
    FUNCTION_MARKER(0x80013e74u, "GAME.EXE");
    return resource_load_character("AFROTG2.ACH", 24, 43);
}

GDB_CALL uint32 character_load_afrotg3(void)
{
    FUNCTION_MARKER(0x80013ea0u, "GAME.EXE");
    return resource_load_character("AFROTG3.ACH", 25, 43);
}

GDB_CALL uint32 character_load_xom_variants(void)
{
    FUNCTION_MARKER(0x80013eccu, "GAME.EXE");
    resource_load_character("XOM1.ACH", 0x21u, 0x38u);
    return resource_load_character("XOM2.ACH", 0x22u, 0x38u);
}

GDB_CALL uint32 character_load_zeng(void)
{
    FUNCTION_MARKER(0x80013f0cu, "GAME.EXE");
    resource_load_character("ZENG.ACH", 0xeu, 0x2eu);
    return resource_load_animations("ZENG.ADT", 0xeu);
}

GDB_CALL uint32 character_load_baddie2(void)
{
    FUNCTION_MARKER(0x80013f48u, "GAME.EXE");
    return resource_load_model("BADDIE2.MOD", 0xfu, 0x2fu);
}

GDB_CALL uint32 character_load_vamp(void)
{
    FUNCTION_MARKER(0x80013f74u, "GAME.EXE");
    return resource_load_character("VAMP.ACH", 12, 44);
}

GDB_CALL uint32 character_load_fatguy(void)
{
    FUNCTION_MARKER(0x80013fa0u, "GAME.EXE");
    resource_load_character("FATGUY.ACH", 0xdu, 0x2du);
    return resource_load_animations("FATGUY.ADT", 0xdu);
}

GDB_CALL uint32 character_load_monkey(void)
{
    FUNCTION_MARKER(0x80013fdcu, "GAME.EXE");
    return resource_load_character("MONKEY.ACH", 26, 49);
}

GDB_CALL uint32 character_load_security(void)
{
    FUNCTION_MARKER(0x80014008u, "GAME.EXE");
    return resource_load_character("SECURITY.ACH", 27, 50);
}

GDB_CALL uint32 character_load_dog(void)
{
    FUNCTION_MARKER(0x80014034u, "GAME.EXE");
    resource_load_character("DOG.ACH", 0x1cu, 0x33u);
    return resource_load_animations("DOG.ADT", 0x1cu);
}

GDB_CALL uint32 character_load_badbabe(void)
{
    FUNCTION_MARKER(0x80014070u, "GAME.EXE");
    return resource_load_character("BADBABE.ACH", 29, 52);
}

GDB_CALL uint32 character_load_femsol(void)
{
    FUNCTION_MARKER(0x8001409cu, "GAME.EXE");
    return resource_load_character("FEMSOL.ACH", 30, 53);
}

GDB_CALL uint32 character_load_bigblack(void)
{
    FUNCTION_MARKER(0x800140c8u, "GAME.EXE");
    return resource_load_character("BIGBLACK.ACH", 31, 41);
}

GDB_CALL uint32 character_load_driver(void)
{
    FUNCTION_MARKER(0x800140f4u, "GAME.EXE");
    resource_load_character("DRIVER.ACH", 0x20u, 0x37u);
    return resource_load_animations("DRIVER.ADT", 0x20u);
}

GDB_CALL uint32 character_load_soldier_variants(void)
{
    FUNCTION_MARKER(0x80014130u, "GAME.EXE");
    resource_load_character("SOLDIER.ACH", 0x23u, 0x39u);
    return resource_load_character("SOLDIER2.ACH", 0x24u, 0x39u);
}

GDB_CALL uint32 character_load_sas(void)
{
    FUNCTION_MARKER(0x80014170u, "GAME.EXE");
    return resource_load_character("SAS.ACH", 37, 58);
}

GDB_CALL uint32 character_load_wets(void)
{
    FUNCTION_MARKER(0x8001419cu, "GAME.EXE");
    return resource_load_character("WETS.ACH", 38, 59);
}

GDB_CALL uint32 character_load_radio(void)
{
    FUNCTION_MARKER(0x800141c8u, "GAME.EXE");
    return resource_load_character("RADIO.ACH", 39, 60);
}

GDB_CALL uint32 character_load_exo_variants(void)
{
    FUNCTION_MARKER(0x800141f4u, "GAME.EXE");
    resource_load_character("EXO1.ACH", 0x28u, 0x3du);
    resource_load_character("EXO2.ACH", 0x29u, 0x3du);
    resource_load_character("EXO3.ACH", 0x2au, 0x3du);
    return resource_load_character("EXO4.ACH", 0x2bu, 0x3du);
}

GDB_CALL uint32 character_load_biker_variants(void)
{
    FUNCTION_MARKER(0x8001425cu, "GAME.EXE");
    resource_load_character("CASBIKER.ACH", 0x2cu, 0x3eu);
    resource_load_character("CASBIKE2.ACH", 0x2du, 0x3eu);
    resource_load_character("CASBIKE3.ACH", 0x2eu, 0x3eu);
    return resource_load_character("BIKE.ACH", 0x33u, 0x46u);
}

GDB_CALL uint32 character_load_airbiker(void)
{
    FUNCTION_MARKER(0x800142c4u, "GAME.EXE");
    return resource_load_character("AIRBIKER.ACH", 47, 63);
}

GDB_CALL uint32 character_load_jet(void)
{
    FUNCTION_MARKER(0x800142f0u, "GAME.EXE");
    resource_load_animations("GENFMALE.ADT", 0x30u);
    resource_load_animations("JET.ADT", 0x30u);
    return resource_load_character("JET.ACH", 0x30u, 0x40u);
}

GDB_CALL uint32 character_load_exc(void)
{
    FUNCTION_MARKER(0x8001433cu, "GAME.EXE");
    resource_load_character("EXC.ACH", 0x31u, 0x41u);
    return resource_load_animations("EXC.ADT", 0x31u);
}

GDB_CALL uint32 character_load_vulkan(void)
{
    FUNCTION_MARKER(0x80014378u, "GAME.EXE");
    resource_load_character("VULKAN.ACH", 50, 66);
    return resource_load_animations("VULKAN.ADT", 50);
}

GDB_CALL uint32 character_load_tramp(void)
{
    FUNCTION_MARKER(0x800143b4u, "GAME.EXE");
    return resource_load_character("TRAMP.ACH", 52, 67);
}

GDB_CALL uint32 character_load_cryo(void)
{
    FUNCTION_MARKER(0x800143e0u, "GAME.EXE");
    return resource_load_character("CRYO.ACH", 53, 68);
}

GDB_CALL uint32 character_load_players(void)
{
    FUNCTION_MARKER(0x8001440cu, "GAME.EXE");
    sint32 type = actor_types[0];

    if (type != -1)
    {
        if (type < 0 || type >= PLAYER_RESOURCE_COUNT)
            diagnostic_stop(__FUNCTION__, __FILE__, __LINE__);
        resource_loader_call(RESOURCE_LOADER_PLAYER, player_resource_loaders[type]);
    }
    if (game_session.player_count != 2)
        return 2;
    type = actor_types[1];
    if (type == -1)
        return 0xfffffffcu; /* BEQ delay slot SLL */
    if (type < 0 || type >= PLAYER_RESOURCE_COUNT)
        diagnostic_stop(__FUNCTION__, __FILE__, __LINE__);
    return resource_loader_call(RESOURCE_LOADER_PLAYER, player_resource_loaders[type]);
}
