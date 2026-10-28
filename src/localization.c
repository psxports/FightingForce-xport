#include "game.h"

LOCALIZATION localization;

/* Original immutable stage names, independent of the disc language stream */
const char *const level_names[LEVEL_NAME_COUNT] = {
    [LEVEL_NAME_CAR_PARK] = "CAR PARK LEVEL",     [LEVEL_NAME_RECEPTION] = "RECEPTION LEVEL",     [LEVEL_NAME_LIFT] = "LIFT LEVEL",   [LEVEL_NAME_CORRIDOR] = "CORRIDOR LEVEL",   [LEVEL_NAME_ZENGS_OFFICE] = "ZENGS OFFICE LEVEL", [LEVEL_NAME_HIGH_STREET] = "HIGH STREET LEVEL", [LEVEL_NAME_HIGH_STREET_BOSS] = "HIGH STREET BOSS LEVEL", [LEVEL_NAME_MALL] = "MALL LEVEL", [LEVEL_NAME_PARK1] = "PARK 1 LEVEL", [LEVEL_NAME_PARK2] = "PARK 2 LEVEL", [LEVEL_NAME_BRIDGE] = "BRIDGE LEVEL", [LEVEL_NAME_BRONX] = "BRONX LEVEL", [LEVEL_NAME_SUBWAY] = "SUBWAY LEVEL", [LEVEL_NAME_SUBWAY_BOSS] = "SUBWAY BOSS LEVEL", [LEVEL_NAME_TRAIN] = "TRAIN LEVEL", [LEVEL_NAME_AIR_BASE] = "AIR BASE LEVEL", [LEVEL_NAME_NAVAL_BASE2] = "NAVAL BASE 2 LEVEL", [LEVEL_NAME_FLYING_WING] = "FLYING WING LEVEL", [LEVEL_NAME_HOVERCRAFT] = "HOVERCRAFT LEVEL", [LEVEL_NAME_NAVAL_BASE1] = "NAVAL BASE 1 LEVEL", [LEVEL_NAME_AIR_BASE_BOSS] = "AIR BASE BOSS LEVEL", [LEVEL_NAME_NAVAL_BASE_BOSS] = "NAVAL BASE BOSS LEVEL",
    [LEVEL_NAME_ISLAND_LAB] = "ISLAND LAB LEVEL", [LEVEL_NAME_ISLAND_LIFT] = "ISLAND LIFT LEVEL", [LEVEL_NAME_FINAL] = "FINAL LEVEL", [LEVEL_NAME_END_SEQUENCE] = "END SEQUENCE",
};

static uint32 next_line(void)
{
    FUNCTION_MARKER(0x80018dcc, "GAME.EXE");
    uint32 p = localization.cursor, next = p + 1;
    uint32 context = resource_arenas.text_end.context + 1;
    for (;;)
    {
        if (p >= localization.file.byte_len)
            diagnostic_stop(__FUNCTION__, __FILE__, __LINE__);
        if (!localization.file.data[p])
            return context;
        localization.cursor = next;
        /* TODO: Investigate allocation beyond the actual native resource owner */
        if (!resource_arenas.text_end.data || (1) > resource_arenas.text_end.available)
        {
            DIAGNOSTIC(fprintf(stderr, "Resource cursor text_end span %u available %u context %08X stage %u tick %u\n", (uint32)(1), resource_arenas.text_end.available, resource_arenas.text_end.context, game_session.stage, game_session.tick));
            diagnostic_stop(__FUNCTION__, __FILE__, __LINE__);
        }
        resource_arenas.text_end.data += 1;
        resource_arenas.text_end.available -= 1;
        resource_arenas.text_end.context = context;
        if (localization.file.data[p] == 13)
        {
            localization.file.data[p] = 0;
            p = localization.cursor;
            next = p + 1;
            context = resource_arenas.text_end.context + 1;
            localization.cursor = next;
            /* TODO: Investigate allocation beyond the actual native resource owner */
            if (!resource_arenas.text_end.data || (1) > resource_arenas.text_end.available)
            {
                DIAGNOSTIC(fprintf(stderr, "Resource cursor text_end span %u available %u context %08X stage %u tick %u\n", (uint32)(1), resource_arenas.text_end.available, resource_arenas.text_end.context, game_session.stage, game_session.tick));
                diagnostic_stop(__FUNCTION__, __FILE__, __LINE__);
            }
            resource_arenas.text_end.data += 1;
            resource_arenas.text_end.available -= 1;
            resource_arenas.text_end.context = context;
            if (p >= localization.file.byte_len)
                diagnostic_stop(__FUNCTION__, __FILE__, __LINE__);
            localization.file.data[p] = 0;
            return context;
        }
        p = localization.cursor;
        next = p + 1;
        context = resource_arenas.text_end.context + 1;
    }
}

GDB_CALL uint32 localization_load(void)
{
    FUNCTION_MARKER(0x80018e50, "GAME.EXE");
    uint32 i;
    resource_load(game_settings.language + 208, resource_arenas.text_end.context, NULL, 0, &localization.file, NULL);
    localization.cursor = 0;
    for (i = 0; i < TEXT_HINT_COUNT; i++)
    {
        localization.hint[i] = resource_arenas.text_end.data ? (const char *)localization.file.data + localization.cursor : NULL;
        next_line();
    }
    localization.text[LOCAL_TEXT_STAGE_CLEARED] = resource_arenas.text_end.data ? (const char *)localization.file.data + localization.cursor : NULL;
    next_line();
    localization.text[LOCAL_TEXT_DESTRUCTION_BONUS] = resource_arenas.text_end.data ? (const char *)localization.file.data + localization.cursor : NULL;
    next_line();
    localization.text[LOCAL_TEXT_ROUTE_AIRBASE] = resource_arenas.text_end.data ? (const char *)localization.file.data + localization.cursor : NULL;
    next_line();
    localization.text[LOCAL_TEXT_ROUTE_NAVAL_BASE] = resource_arenas.text_end.data ? (const char *)localization.file.data + localization.cursor : NULL;
    next_line();
    localization.text[LOCAL_TEXT_ROUTE_BRONX] = resource_arenas.text_end.data ? (const char *)localization.file.data + localization.cursor : NULL;
    next_line();
    localization.text[LOCAL_TEXT_ROUTE_PARK] = resource_arenas.text_end.data ? (const char *)localization.file.data + localization.cursor : NULL;
    next_line();
    localization.text[LOCAL_TEXT_ROUTE_MALL] = resource_arenas.text_end.data ? (const char *)localization.file.data + localization.cursor : NULL;
    next_line();
    localization.text[LOCAL_TEXT_CHOOSE_ROUTE] = resource_arenas.text_end.data ? (const char *)localization.file.data + localization.cursor : NULL;
    next_line();
    localization.text[LOCAL_TEXT_PRESS_START] = resource_arenas.text_end.data ? (const char *)localization.file.data + localization.cursor : NULL;
    next_line();
    localization.text[LOCAL_TEXT_DEMO] = resource_arenas.text_end.data ? (const char *)localization.file.data + localization.cursor : NULL;
    next_line();
    localization.text[LOCAL_TEXT_PRESS_ANY_EXIT] = resource_arenas.text_end.data ? (const char *)localization.file.data + localization.cursor : NULL;
    next_line();
    localization.text[LOCAL_TEXT_CARD_SEARCH] = resource_arenas.text_end.data ? (const char *)localization.file.data + localization.cursor : NULL;
    next_line();
    localization.text[LOCAL_TEXT_CARD_MISSING] = resource_arenas.text_end.data ? (const char *)localization.file.data + localization.cursor : NULL;
    next_line();
    localization.text[LOCAL_TEXT_CARD_SAVE_INFO1] = resource_arenas.text_end.data ? (const char *)localization.file.data + localization.cursor : NULL;
    next_line();
    localization.text[LOCAL_TEXT_CARD_SAVE_INFO2] = resource_arenas.text_end.data ? (const char *)localization.file.data + localization.cursor : NULL;
    next_line();
    localization.text[LOCAL_TEXT_CARD_REMOVED] = resource_arenas.text_end.data ? (const char *)localization.file.data + localization.cursor : NULL;
    next_line();
    localization.text[LOCAL_TEXT_CARD_FORMAT_ASK] = resource_arenas.text_end.data ? (const char *)localization.file.data + localization.cursor : NULL;
    next_line();
    localization.text[LOCAL_TEXT_CARD_SAVE_TITLE] = resource_arenas.text_end.data ? (const char *)localization.file.data + localization.cursor : NULL;
    next_line();
    localization.text[LOCAL_TEXT_CARD_LOAD_TITLE] = resource_arenas.text_end.data ? (const char *)localization.file.data + localization.cursor : NULL;
    next_line();
    card_prompts[CARD_PROMPT_DELETE] = (CARD_PROMPT){resource_arenas.text_end.data ? (const char *)localization.file.data + localization.cursor : NULL, resource_arenas.text_end.context};
    next_line();
    localization.text[LOCAL_TEXT_CARD_ACCESS] = resource_arenas.text_end.data ? (const char *)localization.file.data + localization.cursor : NULL;
    next_line();
    localization.text[LOCAL_TEXT_CARD_FORMATTING] = resource_arenas.text_end.data ? (const char *)localization.file.data + localization.cursor : NULL;
    next_line();
    localization.text[LOCAL_TEXT_CARD_FORMAT_FAILED] = resource_arenas.text_end.data ? (const char *)localization.file.data + localization.cursor : NULL;
    next_line();
    localization.text[LOCAL_TEXT_CARD_FORMAT_COMPLETE] = resource_arenas.text_end.data ? (const char *)localization.file.data + localization.cursor : NULL;
    next_line();
    localization.text[LOCAL_TEXT_CARD_SAVING] = resource_arenas.text_end.data ? (const char *)localization.file.data + localization.cursor : NULL;
    next_line();
    card_prompts[CARD_PROMPT_FULL] = (CARD_PROMPT){resource_arenas.text_end.data ? (const char *)localization.file.data + localization.cursor : NULL, resource_arenas.text_end.context};
    next_line();
    localization.text[LOCAL_TEXT_CARD_DELETE_SELECT] = resource_arenas.text_end.data ? (const char *)localization.file.data + localization.cursor : NULL;
    next_line();
    card_prompts[CARD_PROMPT_OVERWRITE] = (CARD_PROMPT){resource_arenas.text_end.data ? (const char *)localization.file.data + localization.cursor : NULL, resource_arenas.text_end.context};
    next_line();
    card_prompts[CARD_PROMPT_SAVE] = (CARD_PROMPT){resource_arenas.text_end.data ? (const char *)localization.file.data + localization.cursor : NULL, resource_arenas.text_end.context};
    next_line();
    localization.text[LOCAL_TEXT_CARD_INSERT] = resource_arenas.text_end.data ? (const char *)localization.file.data + localization.cursor : NULL;
    next_line();
    localization.text[LOCAL_TEXT_CARD_UNFORMATTED] = resource_arenas.text_end.data ? (const char *)localization.file.data + localization.cursor : NULL;
    next_line();
    card_prompts[CARD_PROMPT_SAVE_RETRY] = (CARD_PROMPT){resource_arenas.text_end.data ? (const char *)localization.file.data + localization.cursor : NULL, resource_arenas.text_end.context};
    next_line();
    localization.text[LOCAL_TEXT_CARD_SAVE_COMPLETE] = resource_arenas.text_end.data ? (const char *)localization.file.data + localization.cursor : NULL;
    next_line();
    localization.text[LOCAL_TEXT_CARD_INSERT_SAVED] = resource_arenas.text_end.data ? (const char *)localization.file.data + localization.cursor : NULL;
    next_line();
    localization.text[LOCAL_TEXT_CARD_INSERT_FORMATTED] = resource_arenas.text_end.data ? (const char *)localization.file.data + localization.cursor : NULL;
    next_line();
    localization.text[LOCAL_TEXT_CARD_LOADING] = resource_arenas.text_end.data ? (const char *)localization.file.data + localization.cursor : NULL;
    next_line();
    card_prompts[CARD_PROMPT_LOAD_RETRY] = (CARD_PROMPT){resource_arenas.text_end.data ? (const char *)localization.file.data + localization.cursor : NULL, resource_arenas.text_end.context};
    next_line();
    localization.text[LOCAL_TEXT_CARD_LOAD_COMPLETE] = resource_arenas.text_end.data ? (const char *)localization.file.data + localization.cursor : NULL;
    next_line();
    localization.text[LOCAL_TEXT_CARD_INSERT_FORMATTED_OR_CONTINUE] = resource_arenas.text_end.data ? (const char *)localization.file.data + localization.cursor : NULL;
    next_line();
    localization.text[LOCAL_TEXT_CARD_FULL_MANAGER] = resource_arenas.text_end.data ? (const char *)localization.file.data + localization.cursor : NULL;
    next_line();
    localization.text[LOCAL_TEXT_CARD_SAVE_CAUTION] = resource_arenas.text_end.data ? (const char *)localization.file.data + localization.cursor : NULL;
    next_line();
    localization.text[LOCAL_TEXT_LOAD_SETTINGS] = resource_arenas.text_end.data ? (const char *)localization.file.data + localization.cursor : NULL;
    next_line();
    localization.text[LOCAL_TEXT_SAVE_SETTINGS] = resource_arenas.text_end.data ? (const char *)localization.file.data + localization.cursor : NULL;
    next_line();
    localization.text[LOCAL_TEXT_CONTROLLER1_REMOVED] = resource_arenas.text_end.data ? (const char *)localization.file.data + localization.cursor : NULL;
    next_line();
    localization.text[LOCAL_TEXT_CONTROLLER2_REMOVED] = resource_arenas.text_end.data ? (const char *)localization.file.data + localization.cursor : NULL;
    next_line();
    localization.text[LOCAL_TEXT_YES] = resource_arenas.text_end.data ? (const char *)localization.file.data + localization.cursor : NULL;
    next_line();
    localization.text[LOCAL_TEXT_NO] = resource_arenas.text_end.data ? (const char *)localization.file.data + localization.cursor : NULL;
    next_line();
    localization.text[LOCAL_TEXT_PAUSED] = resource_arenas.text_end.data ? (const char *)localization.file.data + localization.cursor : NULL;
    next_line();
    localization.text[LOCAL_TEXT_PAUSED1P] = resource_arenas.text_end.data ? (const char *)localization.file.data + localization.cursor : NULL;
    next_line();
    localization.text[LOCAL_TEXT_PAUSED2P] = resource_arenas.text_end.data ? (const char *)localization.file.data + localization.cursor : NULL;
    next_line();
    localization.text[LOCAL_TEXT_CONTINUE] = resource_arenas.text_end.data ? (const char *)localization.file.data + localization.cursor : NULL;
    next_line();
    localization.text[LOCAL_TEXT_QUIT] = resource_arenas.text_end.data ? (const char *)localization.file.data + localization.cursor : NULL;
    next_line();
    localization.text[LOCAL_TEXT_QUIT_ASK] = resource_arenas.text_end.data ? (const char *)localization.file.data + localization.cursor : NULL;
    next_line();
    localization.text[LOCAL_TEXT_OPTIONS] = resource_arenas.text_end.data ? (const char *)localization.file.data + localization.cursor : NULL;
    next_line();
    localization.text[LOCAL_TEXT_DIFFICULTY] = resource_arenas.text_end.data ? (const char *)localization.file.data + localization.cursor : NULL;
    next_line();
    for (i = 0; i < GAME_DIFFICULTY_COUNT; i++)
    {
        localization.difficulty[i] = resource_arenas.text_end.data ? (const char *)localization.file.data + localization.cursor : NULL;
        next_line();
    }
    localization.text[LOCAL_TEXT_CONTROLLER_SETTINGS] = resource_arenas.text_end.data ? (const char *)localization.file.data + localization.cursor : NULL;
    next_line();
    localization.text[LOCAL_TEXT_PLAYER] = resource_arenas.text_end.data ? (const char *)localization.file.data + localization.cursor : NULL;
    next_line();
    localization.text[LOCAL_TEXT_SETTING] = resource_arenas.text_end.data ? (const char *)localization.file.data + localization.cursor : NULL;
    next_line();
    localization.text[LOCAL_TEXT_SCREEN_ADJUST] = resource_arenas.text_end.data ? (const char *)localization.file.data + localization.cursor : NULL;
    next_line();
    localization.text[LOCAL_TEXT_SELECT_FIGHTER] = resource_arenas.text_end.data ? (const char *)localization.file.data + localization.cursor : NULL;
    next_line();
    localization.text[LOCAL_TEXT_SELECT_FIGHTERS] = resource_arenas.text_end.data ? (const char *)localization.file.data + localization.cursor : NULL;
    next_line();
    localization.main_menu[0][0] = resource_arenas.text_end.data ? (const char *)localization.file.data + localization.cursor : NULL;
    localization.main_menu[1][0] = resource_arenas.text_end.data ? (const char *)localization.file.data + localization.cursor : NULL;
    next_line();
    localization.main_menu[1][1] = resource_arenas.text_end.data ? (const char *)localization.file.data + localization.cursor : NULL;
    next_line();
    localization.main_menu[1][2] = resource_arenas.text_end.data ? (const char *)localization.file.data + localization.cursor : NULL;
    next_line();
    localization.main_menu[0][1] = resource_arenas.text_end.data ? (const char *)localization.file.data + localization.cursor : NULL;
    localization.main_menu[1][2] = resource_arenas.text_end.data ? (const char *)localization.file.data + localization.cursor : NULL;
    next_line();
    localization.main_menu[0][2] = resource_arenas.text_end.data ? (const char *)localization.file.data + localization.cursor : NULL;
    world_objects.linked_first = resource_arenas.text_end.context;
    localization.text[LOCAL_TEXT_HIGH_SCORES] = resource_arenas.text_end.data ? (const char *)localization.file.data + localization.cursor : NULL;
    next_line();
    localization.text[LOCAL_TEXT_MUSIC_VOLUME] = resource_arenas.text_end.data ? (const char *)localization.file.data + localization.cursor : NULL;
    next_line();
    localization.text[LOCAL_TEXT_EFFECTS_VOLUME] = resource_arenas.text_end.data ? (const char *)localization.file.data + localization.cursor : NULL;
    next_line();
    localization.text[LOCAL_TEXT_LOAD_CARD_DATA] = resource_arenas.text_end.data ? (const char *)localization.file.data + localization.cursor : NULL;
    next_line();
    localization.text[LOCAL_TEXT_EXIT] = resource_arenas.text_end.data ? (const char *)localization.file.data + localization.cursor : NULL;
    next_line();
    localization.text[LOCAL_TEXT_CONGRATULATIONS] = resource_arenas.text_end.data ? (const char *)localization.file.data + localization.cursor : NULL;
    next_line();
    localization.text[LOCAL_TEXT_CONGRATULATIONS1P] = resource_arenas.text_end.data ? (const char *)localization.file.data + localization.cursor : NULL;
    next_line();
    localization.text[LOCAL_TEXT_CONGRATULATIONS2P] = resource_arenas.text_end.data ? (const char *)localization.file.data + localization.cursor : NULL;
    next_line();
    localization.text[LOCAL_TEXT_TOP_TEN] = resource_arenas.text_end.data ? (const char *)localization.file.data + localization.cursor : NULL;
    next_line();
    localization.text[LOCAL_TEXT_ENTER_NAME] = resource_arenas.text_end.data ? (const char *)localization.file.data + localization.cursor : NULL;
    next_line();
    localization.action[LOCALIZED_ACTION_PUNCH] = resource_arenas.text_end.data ? (const char *)localization.file.data + localization.cursor : NULL;
    next_line();
    localization.action[LOCALIZED_ACTION_JUMP] = resource_arenas.text_end.data ? (const char *)localization.file.data + localization.cursor : NULL;
    next_line();
    localization.action[LOCALIZED_ACTION_KICK] = resource_arenas.text_end.data ? (const char *)localization.file.data + localization.cursor : NULL;
    next_line();
    localization.action[LOCALIZED_ACTION_BACK_FIST] = resource_arenas.text_end.data ? (const char *)localization.file.data + localization.cursor : NULL;
    next_line();
    localization.action[LOCALIZED_ACTION_CAMERA] = resource_arenas.text_end.data ? (const char *)localization.file.data + localization.cursor : NULL;
    next_line();
    localization.action[LOCALIZED_ACTION_RUN] = resource_arenas.text_end.data ? (const char *)localization.file.data + localization.cursor : NULL;
    next_line();
    {
        uint32 padding = (4 - (resource_arenas.text_end.context & 3)) & 3;
        if (padding)
        {
            /* TODO: Investigate text alignment outside the native allocation */
            if (!resource_arenas.text_end.data || padding > resource_arenas.text_end.available)
                diagnostic_stop(__FUNCTION__, __FILE__, __LINE__);
            resource_arenas.text_end.data += padding;
            resource_arenas.text_end.available -= padding;
        }
        resource_arenas.text_end.context = (resource_arenas.text_end.context + 3) & 0xfffffffc;
    }
    return 0xfffffffc;
}
