#include "game.h"

/* Both initialized tracks begin with acceleration fifty */
sint16 train_accel[TRAIN_SIDE_COUNT] = {50, 50};

GDB_CALL uint32 train_update(MODEL_OBJECT *model)
{
    FUNCTION_MARKER(0x8003fdccu, "GAME.EXE");
    sint32 side, max_speed, state, distance, val, volume, denominator, numerator;
    uint32 z, result;
    sint16 *accel;
    /* TODO: Investigate a missing or foreign native train owner */
    if (!model || model->payload_kind != MODEL_PAYLOAD_TRAIN)
    {
        DIAGNOSTIC(fprintf(stderr, "Invalid train model %p stage %u tick %u\n", (void *)model, game_session.stage, game_session.tick));
        diagnostic_stop(__FUNCTION__, __FILE__, __LINE__);
    }
    FIXED_VEC3 *pos = &model->pos;
    if (!model->train.voice_plus_one)
        model->train.voice_plus_one = (uint16)(sound_voice_play(145, 0, 6656) + 1);
    /* TODO: Investigate a train update without a native position */
    if (!pos)
    {
        DIAGNOSTIC(fprintf(stderr, "Invalid train position %p stage %u tick %u\n", (void *)model, game_session.stage, game_session.tick));
        diagnostic_stop(__FUNCTION__, __FILE__, __LINE__);
    }
    side = pos->x.val > 0x0f9fffff;
    max_speed = side ? 16640 : 19200;
    val = model->train.speed;
    z = pos->z.bits;
    pos->z.bits = side ? z - ((uint32)val << 9) : z + ((uint32)val << 9);
    accel = train_accel + side;
    val = (uint16)model->train.speed;
    val = (sint16)(val + (uint16)*accel);
    model->train.speed = (uint16)val;
    if (val > max_speed)
        model->train.speed = (uint16)max_speed;
    if (model->train.speed < 0)
    {
        model->train.speed = 0;
        *accel = 0;
        model->train.state = 2;
        sound_voice_play(124, 127, 5632);
        sound_voice_play(150, 255, 6144);
        sound_voice_play(241, 255, 6144);
    }
    state = model->train.state;
    if (state == 0 || state == 1)
    {
        z = pos->z.bits;
        distance = side ? z - 0x0d800000u : 0x0d800000u - z;
        val = distance < 0 ? (uint32)distance + 0x3e800000u : 0x3e800000u - (uint32)distance;
        val = (val >> 8) / 16000;
        volume = (sint32)((uint32)val * (uint32)(sint32)model->train.speed) / (max_speed >> 1);
        if (volume < 0)
            volume = 0;
        if (volume > 255)
            volume = 255;
        sound_voice_volume(model->train.voice_plus_one - 1, volume);
        if (state == 1)
            goto finish;
        val = (sint16)(uint16)model->train.speed;
        if (val >= max_speed && (uint32)distance - 1u <= 0x1f3ffffe)
        {
            val >>= 7;
            numerator = 0u - ((uint32)val * (uint32)val << 7);
            denominator = (uint32)(distance >> 16) << 1;
            if (!denominator || (denominator == -1 && numerator == (sint32)0x80000000u))
                diagnostic_stop(__FUNCTION__, __FILE__, __LINE__); /* Original400CC/400E4 BREAK */
            *accel = (sint16)(numerator / denominator);
            model->train.state = 1;
            sound_voice_play(242, 255, 5632);
        }
        if (!*accel)
        {
            if (!(uint8)level_transition.exit_requested)
                model->train.speed = -1;
            if (!*accel && (uint8)level_transition.exit_requested)
            {
                model->train.state = 2;
                sound_voice_play(124, 127, 5632);
                sound_voice_play(150, 255, 6144);
            }
        }
    }
    else if (state == 2)
    {
        if (side)
        {
            val = (sint32)stage_animation.phase[0];
            if (val < 32)
                stage_animation.phase[0] = (uint32)val + 1;
        }
        if ((sint32)stage_animation.phase[0] >= 9)
        {
            if ((uint8)level_transition.exit_requested && game_session.stage == 13)
            {
                /* TODO: Investigate an absent train exit collision region */
                if (!col_regions.named[0])
                {
                    DIAGNOSTIC(fprintf(stderr, "Missing train exit region 0 object %p side %d phase %u stage %u tick %u\n", (void *)model, side, stage_animation.phase[0], game_session.stage, game_session.tick));
                    diagnostic_stop(__FUNCTION__, __FILE__, __LINE__);
                }
                col_regions.named[0]->col_kind = 0;
                /* TODO: Investigate an absent train exit collision region */
                if (!col_regions.named[1])
                {
                    DIAGNOSTIC(fprintf(stderr, "Missing train exit region 1 object %p side %d phase %u stage %u tick %u\n", (void *)model, side, stage_animation.phase[0], game_session.stage, game_session.tick));
                    diagnostic_stop(__FUNCTION__, __FILE__, __LINE__);
                }
                col_regions.named[1]->col_kind = 0;
            }
            else if ((sint32)game_session.scene_phase >= 2)
                level_wave.paused = 0;
        }
        if ((sint32)stage_animation.phase[0] >= 31)
        {
            if (!(uint8)level_transition.exit_requested)
            {
                model->train.state = 3;
                model->train.wait_ticks = 100;
            }
            else
            {
                if ((sint32)level_transition.route_exit_tick > 0)
                {
                    model->train.state = 4;
                    sound_voice_play(124, 127, 5632);
                    sound_voice_play(150, 255, 6144);
                }
                stage_animation.phase[0] = 32;
            }
        }
    }
    else if (state == 3)
    {
        val = (sint16)((uint16)model->train.wait_ticks - 1);
        model->train.wait_ticks = (uint16)val;
        if (val < 0)
        {
            model->train.state = 4;
            sound_voice_play(124, 127, 5632);
            sound_voice_play(150, 255, 6144);
        }
    }
    else if (state == 4)
    {
        if (!stage_animation.phase[0])
        {
            val = (sint32)game_session.scene_phase;
            model->train.state = 0;
            *accel = 50;
            if (val >= 2)
                level_wave.paused = 1;
        }
        if (side)
        {
            val = (sint32)stage_animation.phase[0];
            if (val > 0)
                stage_animation.phase[0] = (uint32)val - 1;
        }
    }
finish:
    result = (uint32)(sint32)level_transition.route_choice;
    if ((uint32)side != result)
        return result;
    result = stage_animation.phase[0];
    if (result)
        return result;
    val = (sint32)level_transition.route_exit_tick;
    result = (uint32)val + 1;
    if (val > 0)
    {
        z = pos->z.bits;
        col_regions.count = 0;
        level_transition.route_exit_tick = result;
        actor_positions[1].z.bits = z;
        actor_positions[0].z.bits = z;
    }
    return result;
}
