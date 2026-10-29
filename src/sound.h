#ifndef SOUND_H
#define SOUND_H

#include "xport.h"
#include "geometry_math.h"

sint32 sound_play_action_voice(sint32 idx);

sint32 sound_play_ground_voice(sint32 idx);

sint32 sound_spatial(const FIXED_VEC3 *pos, uint32 *volume, uint32 *pan);

sint32 sound_play_attack_voice(sint32 idx);

sint32 sound_play_heavy_hit_voice(sint32 idx);

sint32 sound_play_chord(void);

uint32 sound_volume_adjust(uint32 val, sint32 delta);

sint32 sound_play_pain(sint32 idx);

void sound_pain_parameters(sint32 idx, sint32 parameters[3]);

sint32 sound_play_effort_voice(sint32 idx);

sint32 sound_play_strike_voice(sint32 idx);

uint32 sound_play_death_voice(sint32 idx);

uint32 sound_play_hit_voice(sint32 idx);

#endif
