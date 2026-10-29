#include <stdlib.h>
#include <string.h>

#include "game.h"
#include "psx.h"

PAD_GAME_STATE pad_game;
PAD_STREAM_STATE pad_stream;

DEMO_STATE demo;
RESOURCE_FILE demo_file;
const DEMO_STATE demo_defaults = {0, 0xffffffffu, DEMO_COUNT};

const DEMO_CONFIG demo_config[DEMO_COUNT] = {
    {0, 0, {0, 0xffffffffu}, 110},
    {0, 2, {1, 0xffffffffu}, 111},
    {0, 9, {3, 0xffffffffu}, 112},
    {0, 10, {2, 0xffffffffu}, 113},
};

typedef char PAD_INPUT_LAYOUT_CHECK[sizeof(PAD_INPUT_STREAM) == 8 && offsetof(PAD_INPUT_STREAM, frame) == 8 && sizeof(PAD_INPUT_FRAME) == 8 ? 1 : -1];

GDB_CALL uint32 pad_stream_play(PAD_INPUT_STREAM *data, uint32 byte_len)
{
    FUNCTION_MARKER(0x8005f058, "GAME.EXE");
    XASSERT(data && !((uintptr_t)data & 3u) && byte_len >= sizeof(*data));
    pad_stream.mode = PAD_STREAM_PLAY;
    pad_stream.data = data;
    pad_stream.capacity = (byte_len - sizeof(*data)) / sizeof(PAD_INPUT_FRAME);
    random_seed = data->seed;
    XASSERT(pad_stream.data->count <= pad_stream.capacity + 1);
    pad_stream.count = pad_stream.data->count;
    pad_stream.cursor = 1;
    return pad_stream.count;
}

GDB_CALL void pad_stream_disable(void)
{
    FUNCTION_MARKER(0x8005f0c8, "GAME.EXE");
    pad_stream.mode = PAD_STREAM_LIVE;
}

GDB_CALL uint32 pad_stream_rec(PAD_INPUT_STREAM *data, uint32 byte_len)
{
    FUNCTION_MARKER(0x8005f0d8, "GAME.EXE");
    uint32 random;
    XASSERT(data && !((uintptr_t)data & 3u) && byte_len >= sizeof(*data));
    pad_stream.mode = PAD_STREAM_RECORD;
    pad_stream.data = data;
    pad_stream.capacity = (byte_len - sizeof(*data)) / sizeof(PAD_INPUT_FRAME);
    data->count = 0;
    do
    {
        random = random_next() & 65535u;
        pad_stream.data->seed = random;
    } while (!pad_stream.data->seed);
    random_seed = pad_stream.data->seed;
    pad_stream.count = 1;
    pad_stream.cursor = 1;
    return 1;
}

GDB_CALL void pad_stream_reset(void)
{
    FUNCTION_MARKER(0x8005f214, "GAME.EXE");
    pad_stream.mode = PAD_STREAM_LIVE;
    pad_stream.cursor = 0;
    pad_stream.count = 0;
}

/* 5F558: raw gameplay buttons; the first mapping wins if both IDs match. */
GDB_CALL uint32 pad_game_buttons(uint32 pad)
{
    FUNCTION_MARKER(0x8005f558, "GAME.EXE");
    uint32 val;
    if (pad == pad_game.port[0])
        val = pad_game.sample[0];
    else if (pad == pad_game.port[1])
        val = pad_game.sample[1];
    else
        return 0;
    pad_decode.type = (uint8)(val >> 16);
    return val & 65535;
}

/* Original normal, recording and playback input paths */
GDB_CALL uint32 pad_game_poll(void)
{
    FUNCTION_MARKER(0x8005f23c, "GAME.EXE");
    if (game_services.audit_input_event)
        game_services.audit_input_event(1);
    uint32 val, mode, idx, count;
    mode = pad_stream.mode;
    if (mode == PAD_STREAM_RECORD)
    {
        val = (uint16)pad_decode_buttons(pad_game.port[0]);
        pad_game.sample[0] = val;
        pad_game.sample[0] = val | ((uint32)(sint32)(sint8)pad_decode.type << 16);
        val = (uint16)pad_decode_buttons(pad_game.port[1]);
        pad_game.sample[1] = val;
        pad_game.sample[1] = val | ((uint32)(sint32)(sint8)pad_decode.type << 16);
        idx = pad_stream.count;
        XASSERT(pad_stream.data && idx && idx <= pad_stream.capacity);
        val = pad_game.sample[0];
        pad_stream.data->frame[idx - 1].sample[0] = val;
        idx = pad_stream.count;
        XASSERT(pad_stream.data && idx && idx <= pad_stream.capacity);
        val = pad_game.sample[1];
        pad_stream.data->frame[idx - 1].sample[1] = val;
        val = pad_stream.count + 1;
        pad_stream.count = val;
        return val;
    }
    if (mode == PAD_STREAM_PLAY)
    {
        count = pad_stream.count;
        idx = pad_stream.cursor;
        if ((sint32)idx >= (sint32)(count - 1))
        {
            pad_game.sample[0] = 0;
            pad_game.sample[1] = 0;
            goto playback_exit;
        }
        XASSERT(pad_stream.data && idx && idx <= pad_stream.capacity);
        val = pad_stream.data->frame[idx - 1].sample[0];
        pad_game.sample[0] = val;
        idx = pad_stream.cursor;
        XASSERT(pad_stream.data && idx && idx <= pad_stream.capacity);
        val = pad_stream.data->frame[idx - 1].sample[1];
        pad_game.sample[1] = val;
        pad_stream.cursor = pad_stream.cursor + 1;
        if ((uint16)pad_decode_buttons(pad_game.port[0]))
            goto playback_exit;
        val = (uint16)pad_decode_buttons(pad_game.port[1]);
        if (val)
            goto playback_exit;
        return val;
    }
    pad_decode.reset_blocked = (uint8)(actor_types[0] == ACTOR_TYPE_NONE || actor_health[0].hp <= 0);
    val = (uint16)pad_decode_buttons(pad_game.port[0]);
    pad_game.sample[0] = val;
    pad_game.sample[0] = val | ((uint32)(sint32)(sint8)pad_decode.type << 16);
    pad_decode.reset_blocked = (uint8)((sint32)game_session.player_count < 2 || actor_types[1] == ACTOR_TYPE_NONE || actor_health[1].hp <= 0);
    val = (uint16)pad_decode_buttons(pad_game.port[1]);
    pad_game.sample[1] = val;
    pad_game.sample[1] = val | ((uint32)(sint32)(sint8)pad_decode.type << 16);
    pad_decode.reset_blocked = 0;
    return val;
playback_exit:
    sound_voices_stop_all();
    text_hint = TEXT_HINT_NONE;
    pad_decode.reset_blocked = 1;
    XASSERT(game_services.return_to_frontend);
    game_services.return_to_frontend();
    XASSERT(0);
    return 0;
}

PAD_ACTION_STATE pad_action;

const uint8 pad_bindings[PAD_SCHEME_COUNT][PAD_BIND_COUNT] = {{0, 1, 2, 3, 4, 5}, {2, 0, 3, 1, 4, 5}, {0, 2, 3, 1, 4, 5}, {2, 0, 1, 3, 4, 5}, {0, 1, 2, 3, 5, 4}, {2, 0, 3, 1, 5, 4}, {0, 2, 3, 1, 5, 4}, {2, 0, 1, 3, 5, 4}};

static uint8 *pad_action_slot(uint32 player, PAD_BIND binding)
{
    sint32 cfg = (sint8)pad_action.cfg[player];
    uint32 action = pad_bindings[cfg][binding];
    return &pad_action.latch[player][action];
}

/* Decay action latches, then retain new face-button edges for twelve ticks */
GDB_CALL uint8 *pad_action_press(uint32 player, uint32 buttons)
{
    FUNCTION_MARKER(0x8005f8f0, "GAME.EXE");
    uint32 i;
    uint8 *latch = pad_action.latch[player], *slot;
    uint8 val;
    for (i = 0; i < PAD_ACTION_COUNT; i++)
    {
        val = latch[i];
        if (val)
            latch[i] = val - 1;
    }
    if (buttons & 0x8000)
    {
        slot = pad_action_slot(player, PAD_BIND_SQUARE);
        *slot = 12;
    }
    if (buttons & 0x4000)
    {
        slot = pad_action_slot(player, PAD_BIND_CROSS);
        *slot = 12;
    }
    if (buttons & 0x2000)
    {
        slot = pad_action_slot(player, PAD_BIND_CIRCLE);
        *slot = 12;
    }
    if (buttons & 0x1000)
    {
        slot = pad_action_slot(player, PAD_BIND_TRIANGLE);
        *slot = 12;
        return slot;
    }
    return 0;
}

GDB_CALL uint8 *pad_action_hold(uint32 player, uint32 buttons)
{
    FUNCTION_MARKER(0x8005fa58, "GAME.EXE");
    uint8 *slot;
    if (buttons & 0x400)
    {
        slot = pad_action_slot(player, PAD_BIND_L1);
        *slot = 1;
    }
    if (buttons & 0x800)
    {
        slot = pad_action_slot(player, PAD_BIND_R1);
        *slot = 1;
        return slot;
    }
    return 0;
}

PAD_HOST_STATE pad_host = {1};
PAD_REPORT pad_reports[PAD_PLAYER_COUNT];
PAD_DECODE_STATE pad_decode;

typedef char PAD_REPORT_LAYOUT_CHECK[sizeof(PAD_REPORT) == 34 && offsetof(PAD_REPORT, buttons) == 2 && offsetof(PAD_REPORT, left_x) == 6 && offsetof(PAD_REPORT, left_y) == 7 ? 1 : -1];
PAD_MENU_STATE pad_menu;

/* InitPAD host boundary: register neutral disconnected buffers; host frame
 * polling supplies actual digital input before any game input decoder runs. */
void pad_host_init(void)
{
    PadInit(0);
    PadInitDirect((uint8 *)&pad_reports[0], (uint8 *)&pad_reports[1]);
    pad_host.enabled = 1;
}

void pad_host_publish(uint32 connected, uint32 buttons0, uint32 buttons1)
{
    uint32 i, buttons[PAD_PLAYER_COUNT] = {buttons0, buttons1};
    if (!pad_host.enabled)
        return;
    for (i = 0; i < 2; ++i)
    {
        pad_publish(i, connected > i, ~buttons[i]);
    }
}

GDB_CALL uint8 *pad_action_clear(sint32 player)
{
    FUNCTION_MARKER(0x8005fb00, "GAME.EXE");
    return memset(pad_action.latch[player], 0, sizeof(pad_action.latch[player]));
}

/* Poll mapped menu edges without changing the last raw menu sample */
GDB_CALL uint32 pad_menu_poll(sint32 player)
{
    FUNCTION_MARKER(0x80068114, "GAME.EXE");
    if (game_services.audit_input_event)
        game_services.audit_input_event(2);
    uint32 raw = pad_decode_buttons(pad_game.port[player]);
    pad_menu.pressed[player] = (raw ^ pad_menu.prev[player]) & raw;
    pad_menu.prev[player] = raw;
    return pad_menu.pressed[player];
}

GAME_SERVICES game_services;

/* GAME.EXE:800573A8. The second pad is counted only if the first is valid. */
GDB_CALL sint32 pad_count_devices(void)
{
    FUNCTION_MARKER(0x800573a8, "GAME.EXE");
    const PAD_REPORT *pad = &pad_reports[0];
    if (((pad->id >> 4) != 4 && (pad->id >> 4) != 7 && (pad->id >> 4) != 5) || pad_reports[0].status == 255)
        return 0;
    pad = &pad_reports[1];
    return (((pad->id >> 4) == 4 || (pad->id >> 4) == 7 || (pad->id >> 4) == 5) && pad_reports[1].status != 255) ? 2 : 1;
}

/* GAME.EXE:8005EC90. Return is always one, not the controller count. */
GDB_CALL sint32 pad_refresh_ports(void)
{
    FUNCTION_MARKER(0x8005ec90, "GAME.EXE");
    pad_decode.count = pad_count_devices();
    pad_game.port[0] = 0;
    pad_game.port[1] = 1;
    return 1;
}

/* GAME.EXE:8005F5AC. Complete input decoder including grace counters and reset.
 * The reset path does not return: IDA's post-longjmp store is unreachable. */
GDB_CALL sint16 pad_decode_buttons(sint32 port)
{
    FUNCTION_MARKER(0x8005f5ac, "GAME.EXE");
    /* Apply recorded buttons at the original decode-time load */
    if (game_services.audit_input_event)
        game_services.audit_input_event(0);
    uint32 idx = port;
    const PAD_REPORT *pad = &pad_reports[port != 0];
    sint16 buttons;
    sint32 decoded_type;
    uint8 type = pad->id >> 4;
    pad_decode.type = type;
    buttons = (sint16)~pad->buttons;
    if (buttons == -1)
        buttons = 0;
    if (pad->status == 255)
    {
        type = 255;
        if ((sint8)pad_decode.disconnect[idx] < 0)
        {
            ++pad_decode.disconnect[idx];
            type = 4;
        }
        pad_decode.type = type;
        buttons = 0;
    }
    else
        pad_decode.disconnect[idx] = 0;
    decoded_type = (sint8)pad_decode.type;
    if (decoded_type == 4)
        pad_decode.unsupported[idx] = 0;
    else if (decoded_type == 7 || decoded_type == 5)
    {
        sint32 raw_x, x, y;
        pad_decode.unsupported[idx] = 0;
        pad_decode.type = 22;
        raw_x = pad->left_x;
        x = raw_x - 128;
        pad_decode.x = x;
        y = pad->left_y - 128;
        pad_decode.y = y;
        if (x > 0)
        {
            x = raw_x - 160;
            pad_decode.x = x;
            if (x < 0)
                pad_decode.x = 0;
        }
        else
        {
            x = pad_decode.x + 32u;
            pad_decode.x = x;
            if (x > 0)
                pad_decode.x = 0;
        }
        y = pad_decode.y;
        if (y > 0)
        {
            y = y - 32u;
            pad_decode.y = y;
            if (y < 0)
                pad_decode.y = 0;
        }
        else
        {
            y = pad_decode.y + 32u;
            pad_decode.y = y;
            if (y > 0)
                pad_decode.y = 0;
        }
        x = (sint32)(pad_decode.x * 341u) >> 8;
        pad_decode.x = x;
        y = (sint32)(pad_decode.y * 341u) >> 8;
        pad_decode.y = y;
    }
    else
    {
        type = 255;
        if ((sint8)pad_decode.unsupported[idx] < 15)
        {
            ++pad_decode.unsupported[idx];
            type = 4;
        }
        pad_decode.type = type;
        buttons = 0;
    }
    if ((buttons & 9) == 9 && !pad_decode.reset_blocked)
    {
        sint16 count = (sint16)(pad_decode.reset_count[idx] + 1);
        pad_decode.reset_count[idx] = count;
        if (count >= 31)
        {
            pad_decode.reset_count[1] = 0;
            pad_decode.reset_count[0] = 0;
            sound_voices_stop_all();
            text_hint = TEXT_HINT_NONE;
            pad_decode.reset_blocked = 1;
            XASSERT(game_services.return_to_frontend);
            game_services.return_to_frontend();
            XASSERT(0);
        }
    }
    else
        pad_decode.reset_count[idx] = 0;
    return buttons;
}

/* Existing selection phase of original15C6C, not a new original function */
GDB_CALL void demo_select_next(void)
{
    uint32 idx = demo.next_idx;
    sint32 divisor = demo.count;
    sint32 numerator = idx + 1;
    uint32 type1, type0, level, stage;
    if (divisor == 0 || (divisor == -1 && numerator == (sint32)0x80000000))
    {
        /* Original division raises BREAK7 or BREAK6 before any table access */
        XASSERT(0);
        return;
    }
    type1 = demo_config[idx].player_type[1];
    demo.cur_idx = idx;
    demo.next_idx = (uint32)(numerator % divisor);
    game_session.player_count = type1 == 0xffffffff ? 1 : 2;
    const DEMO_CONFIG *entry = demo_config + demo.cur_idx;
    type0 = entry->player_type[0];
    level = entry->mode;
    stage = entry->stage;
    actor_types[0] = (uint16)type0;
    type1 = entry->player_type[1];
    game_session.mode = level;
    game_session.stage = stage;
    actor_types[1] = (uint16)type1;
}
