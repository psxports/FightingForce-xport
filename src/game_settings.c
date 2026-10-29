#include "game.h"

_Static_assert(sizeof(high_scores) / sizeof(high_scores[0]) == sizeof(card_files.save.data.high_score) / sizeof(card_files.save.data.high_score[0]), "Native and saved score counts match");

GAME_SETTINGS game_settings;
const GAME_SETTINGS game_settings_defaults = {.difficulty = GAME_DIFFICULTY_NORMAL};

static uint32 copy_scores(int load)
{
    uint32 *fields[HIGH_SCORE_COUNT * 3], *saved[HIGH_SCORE_COUNT * 3];
    uint32 i, j, words[4];
    for (i = 0; i < HIGH_SCORE_COUNT; i++)
    {
        HIGH_SCORE *score = high_scores + i;
        CARD_SCORE *file_score = card_files.save.data.high_score + i;
        fields[3 * i] = &score->score;
        fields[3 * i + 1] = &score->name.bits;
        fields[3 * i + 2] = &score->flags;
        saved[3 * i] = &file_score->score;
        saved[3 * i + 1] = &file_score->name_bits;
        saved[3 * i + 2] = &file_score->flags;
    }
    for (i = 0; i < 28; i += 4)
    {
        for (j = 0; j < 4; j++)
            words[j] = load ? *saved[i + j] : *fields[i + j];
        for (j = 0; j < 4; j++)
            if (load)
                *fields[i + j] = words[j];
            else
                *saved[i + j] = words[j];
    }
    words[0] = load ? *saved[28] : *fields[28];
    words[1] = load ? *saved[29] : *fields[29];
    if (load)
    {
        *fields[28] = words[0];
        *fields[29] = words[1];
    }
    else
    {
        *saved[28] = words[0];
        *saved[29] = words[1];
    }
    return words[0];
}

uint32 game_settings_restore_card(void)
{
    FUNCTION_MARKER(0x80067bacu, "GAME.EXE");
    uint32 settings = card_files.save.data.sound;
    sound_settings.flags = settings;
    sound_settings.effects_volume = settings >> 16;
    sound_settings.music_volume = settings >> 24;
    game_settings.difficulty_bits = card_files.save.data.difficulty;
    pad_action.cfg[0] = card_files.save.data.pad_cfg[0];
    pad_action.cfg[1] = card_files.save.data.pad_cfg[1];
    game_settings.scr_offset.x = (sint8)card_files.save.data.scr_offset[0];
    game_settings.scr_offset.y = (sint8)card_files.save.data.scr_offset[1];
    return copy_scores(1);
}

uint32 game_settings_capture_card(void)
{
    FUNCTION_MARKER(0x80067c84u, "GAME.EXE");
    uint32 settings = sound_settings.flags | ((uint32)sound_settings.effects_volume << 16) | ((uint32)sound_settings.music_volume << 24);
    card_files.save.data.sound = settings;
    card_files.save.data.difficulty = game_settings.difficulty_bits;
    card_files.save.data.pad_cfg[0] = pad_action.cfg[0];
    card_files.save.data.pad_cfg[1] = pad_action.cfg[1];
    card_files.save.data.scr_offset[0] = (uint8)game_settings.scr_offset.x;
    card_files.save.data.scr_offset[1] = (uint8)game_settings.scr_offset.y;
    return copy_scores(0);
}
