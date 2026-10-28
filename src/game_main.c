#include "game.h"
#include "diagnostics.h"
#include "platform.h"
#include "psx_gpu.h"
#include "sound_device.h"
#include "movies.h"
#include "psx_spu.h"
#include "xport.h"
#include "audit/audit_scene_models.h"
#include "audit/audit_scene_render_state.h"
#if defined(_WIN32)
    #include <windows.h>
#elif defined(__EMSCRIPTEN__)
    #include <emscripten.h>
    #define Sleep emscripten_sleep
#endif
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

GDB_CALL void diagnostic_end(void)
{
}

int audit_input_run(void);
int audit_sound_run(void);
int frontend_run(uint32 limit, int headless);
int frontend_run_script(const char *script, int headless);

#if !defined(NDEBUG) && !defined(__EMSCRIPTEN__)
/* Fixed diagnostic wire words preserve every matrix field, including its tail */
static void geometry_audit_load(POSE_MATRIX *matrix, const uint32 words[8])
{
    int i;
    for (i = 0; i < 5; i++)
        matrix->rotation_words[i] = words[i];
    for (i = 0; i < 3; i++)
        matrix->t[i] = (sint32)words[i + 5];
}

static void geometry_audit_store(const POSE_MATRIX *matrix, uint32 words[8])
{
    int i;
    for (i = 0; i < 5; i++)
        words[i] = matrix->rotation_words[i];
    for (i = 0; i < 3; i++)
        words[i + 5] = (uint32)matrix->t[i];
}

typedef struct ANIMATION_START_AUDIT_JOB
{
    uint32 words[19];
    uint8 stream[128];
    uint8 history[32];
} ANIMATION_START_AUDIT_JOB;

_Static_assert(sizeof(ANIMATION_START_AUDIT_JOB) == 236, "Animation startup fixture record size");

typedef struct ANIMATION_START_AUDIT_CONTEXT
{
    FILE *out;
    uint32 ordinal, actor;
    const uint8 *stream;
    uint8 stream_guard[128], old_stream[128], old_guard[128];
    ACTOR_ANIMATION playback[ACTOR_COUNT];
    FIXED_VEC3 positions[ACTOR_COUNT];
    sint16 types[ACTOR_COUNT];
    uint16 flags[ANIMATION_COUNT];
    ANIMATION_BANK bank;
    ANIMATION_STREAM *sources;
    uint32 source_count, source_bytes;
    const uint8 *source_blob, *source_guard;
    ACTOR_MOVEMENT movement[ACTOR_COUNT];
    ACTOR_ITEMS items[ACTOR_COUNT];
    MODEL_OBJECT objects[DYNAMIC_OBJECT_COUNT];
    GAME_SESSION session;
} ANIMATION_START_AUDIT_CONTEXT;

static ANIMATION_START_AUDIT_CONTEXT animation_start_audit;

static void animation_start_audit_guard(void)
{
    ACTOR_ANIMATION *actual = actor_animations + animation_start_audit.actor;
    ACTOR_ANIMATION *allowed = animation_start_audit.playback + animation_start_audit.actor;
    memcpy(&allowed->src, &actual->src, sizeof(allowed->src));
    allowed->frame = actual->frame;
    allowed->id = actual->id;
    allowed->hit_flags = actual->hit_flags;
    allowed->event_step = actual->event_step;
    memcpy(&allowed->motion, &actual->motion, sizeof(allowed->motion));
    if (animation_start_audit.sources)
    {
        animation_start_audit.movement[0].prev_state = actor_movement[0].prev_state;
        if (memcmp(animation_start_audit.movement, actor_movement, sizeof(animation_start_audit.movement)) || memcmp(animation_start_audit.items, actor_items, sizeof(animation_start_audit.items)) || memcmp(animation_start_audit.objects, dynamic_objects, sizeof(animation_start_audit.objects)) || memcmp(animation_start_audit.source_blob, animation_start_audit.source_guard, animation_start_audit.source_bytes))
            abort();
    }
    if (memcmp(animation_start_audit.playback, actor_animations, sizeof(animation_start_audit.playback)) || memcmp(animation_start_audit.positions, actor_positions, sizeof(animation_start_audit.positions)) || memcmp(animation_start_audit.types, actor_types, sizeof(animation_start_audit.types)) || memcmp(animation_start_audit.flags, animation_flags, sizeof(animation_start_audit.flags)) || memcmp(&animation_start_audit.bank, animation_banks + ANIMATION_BANK_HAWK, sizeof(animation_start_audit.bank)) || memcmp(&animation_start_audit.session, &game_session, sizeof(animation_start_audit.session)) || (!animation_start_audit.sources && (memcmp(animation_start_audit.stream, animation_start_audit.stream_guard, sizeof(animation_start_audit.stream_guard)) || memcmp(animation_start_audit.old_stream, animation_start_audit.old_guard, sizeof(animation_start_audit.old_guard)))))
        abort();
}

static int animation_start_audit_store(uint32 disposition, uint32 result)
{
    ACTOR_ANIMATION *playback = actor_animations + animation_start_audit.actor;
    uint32 source, fields[14];
    animation_start_audit_guard();
    if (playback->src.kind == ACTOR_ANIMATION_SOURCE_NONE)
        source = 0;
    else if (animation_start_audit.sources && playback->src.kind == ACTOR_ANIMATION_SOURCE_STREAM)
    {
        source = 0;
        for (uint32 i = 1; i <= animation_start_audit.source_count; i++)
        {
            ANIMATION_STREAM *stream = animation_start_audit.sources + i;
            if (stream->header == playback->src.stream.header && stream->domain == playback->src.stream.domain && stream->byte_len == playback->src.stream.byte_len)
            {
                if (source)
                    abort();
                source = i;
            }
        }
        if (!source)
            abort();
    }
    else if (playback->src.kind == ACTOR_ANIMATION_SOURCE_STREAM && playback->src.stream.byte_len == 128 && playback->src.stream.domain == ANIMATION_STREAM_NATIVE)
    {
        if ((const uint8 *)playback->src.stream.header == animation_start_audit.old_stream)
            source = 1;
        else if ((const uint8 *)playback->src.stream.header == animation_start_audit.stream)
            source = 2;
        else
            abort();
    }
    else
        abort();
    fields[0] = animation_start_audit.ordinal;
    fields[1] = disposition;
    fields[2] = result;
    fields[3] = source;
    fields[4] = playback->frame;
    fields[5] = playback->id;
    memcpy(fields + 6, playback->motion.bits, sizeof(playback->motion.bits));
    fields[9] = playback->motion.events;
    fields[10] = playback->motion.forward_offset_bits;
    fields[11] = playback->hit_flags;
    fields[12] = playback->event_step;
    fields[13] = animation_fall_exit;
    if (fwrite(fields, sizeof(fields), 1, animation_start_audit.out) != 1 || fwrite(action_history.action, sizeof(action_history.action), 1, animation_start_audit.out) != 1 || fwrite(action_history.predicted, sizeof(action_history.predicted), 1, animation_start_audit.out) != 1 || fwrite(action_history.matched, sizeof(action_history.matched), 1, animation_start_audit.out) != 1)
        return 0;
    if (animation_start_audit.sources)
    {
        uint32 held[5] = {playback->selector, actor_movement[0].prev_state, actor_items[0].bits, dynamic_objects[0].kind, dynamic_objects[1].kind};
        return fwrite(held, sizeof(held), 1, animation_start_audit.out) == 1;
    }
    return 1;
}

static int frontend_world_audit_store(FILE *out, uint32 result)
{
    uint32 i;
    uint32 metadata[6] = {result, world_objects.count, world_objects.tracked_count, col_regions.count, col_regions.named[0] != NULL, col_regions.named[1] != NULL};
    if (fwrite(metadata, sizeof(metadata), 1, out) != 1)
        return 0;
    for (i = 0; i < WORLD_OBJECT_COUNT; i++)
    {
        MODEL_OBJECT *item = world_objects.object + i;
        MODEL_OBJECT common;
        uint32 fields[7] = {item->kind, item->pos.x.bits, item->pos.y.bits, item->pos.z.bits, item->angles.yaw_bits, item->angles.pitch_bits, item->angles.roll_bits};
        memset(&common, 0, sizeof(common));
        common.kind = item->kind;
        common.pos = item->pos;
        common.angles = item->angles;
        if (memcmp(&common, item, sizeof(common)))
            abort();
        if (fwrite(fields, sizeof(fields), 1, out) != 1)
            return 0;
    }
    for (i = 0; i < COLLISION_REGION_COUNT; i++)
    {
        COLLISION_REGION *region = col_regions.data + i;
        uint16 fields[10] = {(uint16)region->geometry.coord[0], (uint16)region->geometry.coord[1], (uint16)region->geometry.coord[2], (uint16)region->geometry.coord[3], (uint16)region->trigger_kind, (uint16)region->shape, (uint16)region->col_kind, (uint16)region->model_type, (uint16)region->hits, (uint16)region->alternate_attack};
        uint32 object[2] = {region->object.idx, region->object.present};
        if (object[1] > 1 || (object[1] && object[0] >= WORLD_OBJECT_COUNT) || (!object[1] && object[0]))
            abort();
        if (fwrite(fields, sizeof(fields), 1, out) != 1 || fwrite(object, sizeof(object), 1, out) != 1)
            return 0;
    }
    return 1;
}

/* Use the same writable register subset as the original observer's restore */
static void geometry_audit_restore_gte(const uint32 words[64])
{
    int i;
    for (i = 0; i < 32; i++)
        xport_gte_write_control(i, words[i + 32]);
    for (i = 0; i < 15; i++)
        xport_gte_write_data(i, words[i]);
    for (i = 16; i < 28; i++)
        xport_gte_write_data(i, words[i]);
}

/* Native fixture transport only; no original function identity */
static int geometry_audit_run(uint32 mode)
{
    static const char *const names[6] = {"camera-matrix", "matrix-stack", "horizon", "projection", "rotation", "model-matrix"};
    static const uint32 input_words[6] = {16, 30, 1, 14, 14, 15};
    static const uint32 output_words[6] = {14, 34, 14, 5, 13, 21};
    char input_path[160], output_path[160];
    uint32 header[4], gte[64], job[30], observation[34];
    GEOMETRY_SCRATCH scratch_guard;
    GEOMETRY_MODEL_BUFFER model_guard;
    CAMERA camera_guard;
    FILE *in, *out;
    size_t got = 0;
    int result = 0, i;
    if (mode >= 6)
        return 4;
    snprintf(input_path, sizeof(input_path), "../status/menu/%s-native-jobs.bin", names[mode]);
    snprintf(output_path, sizeof(output_path), "../status/menu/%s-native.bin", names[mode]);
    in = xport_fopen(input_path, "rb");
    out = xport_fopen(output_path, "wb");
    if (!in || !out)
    {
        if (in)
            fclose(in);
        if (out)
            fclose(out);
        return 4;
    }
    if (fread(header, sizeof(header), 1, in) != 1 || header[0] != 0x324a4746 || header[1] != 1 || header[2] != mode || header[3] != input_words[mode] || fread(gte, sizeof(gte), 1, in) != 1)
        result = 4;
    if (!result)
    {
        geometry_audit_restore_gte(gte);
        header[0] = 0x324e4746;
        header[3] = output_words[mode];
        if (fwrite(header, sizeof(header), 1, out) != 1)
            result = 4;
    }
    while (!result && (got = fread(job, 1, input_words[mode] * 4, in)) == input_words[mode] * 4)
    {
        POSE_MATRIX *base;
        uint32 slot = GEOMETRY_MATRIX_COUNT - 2;
        memset(&geometry_workspace.scratch, 0xa5, sizeof(geometry_workspace.scratch));
        memset(&geometry_workspace.model, 0xa5, sizeof(geometry_workspace.model));
        geometry_workspace.matrix_storage = GEOMETRY_MATRIX_SCRATCH;
        base = geometry_workspace.scratch.matrix;
        if (mode == 1)
        {
            if (job[0] > 2 || job[1] > 1 || job[2] < 1 || job[2] >= (uint32)(job[1] ? GEOMETRY_MODEL_MATRIX_COUNT : GEOMETRY_MATRIX_COUNT) - 1u)
            {
                result = 4;
                break;
            }
            if (job[1])
            {
                base = geometry_workspace.model.matrix;
                geometry_workspace.matrix_storage = GEOMETRY_MATRIX_MODEL;
            }
            slot = job[2];
            for (i = 0; i < 3; i++)
                geometry_audit_load(&base[slot - 1 + i], job + 6 + 8 * i);
            geometry_audit_restore_gte(gte);
        }
        else if (mode == 3)
        {
            geometry_audit_load(&base[slot], job + 6);
            geometry_audit_restore_gte(gte);
            SetGeomOffset((sint32)job[3], (sint32)job[4]);
            SetGeomScreen((sint32)job[5]);
        }
        else if (mode != 2)
        {
            const uint32 *matrix = job + (mode == 0 ? 3 : mode == 5 ? 2 : 1);
            geometry_audit_load(&base[slot], matrix);
            for (i = 0; i < 5; i++)
                base[slot + 1].rotation_words[i] = matrix[8 + i];
        }
        geometry_workspace.matrix = &base[slot];
        if (mode == 0)
        {
            if (job[0] > 2)
            {
                result = 4;
                break;
            }
            camera.angles.yaw_bits = (uint16)job[1];
            camera.angles.pitch_bits = (uint16)job[2];
            if (job[0] == 2)
                geometry_workspace.matrix = &base[GEOMETRY_MATRIX_COUNT - 4];
        }
        else if (mode == 2)
            camera.angles.pitch_bits = (uint16)job[0];
        memcpy(&scratch_guard, &geometry_workspace.scratch, sizeof(scratch_guard));
        memcpy(&model_guard, &geometry_workspace.model, sizeof(model_guard));
        memcpy(&camera_guard, &camera, sizeof(camera_guard));
        if (mode == 0)
        {
            if (job[0] == 0)
                transform_pitch((sint32)job[1]);
            else if (job[0] == 1)
                transform_yaw((sint32)job[1]);
            else
                transform_reset();
        }
        else if (mode == 1)
        {
            if (job[0] == 0)
                transform_pop();
            else if (job[0] == 1)
                transform_push();
            else
                transform_translate((sint32)job[3], (sint32)job[4], (sint32)job[5]);
        }
        else if (mode == 2)
            proj_update_horizon();
        else if (mode == 3)
        {
            PROJECTION_POINT point;
            observation[4] = proj_local((sint32)job[0], (sint32)job[1], (sint32)job[2], &point);
            for (i = 0; i < 4; i++)
                observation[i] = (uint32)point.v[i];
        }
        else if (mode == 5)
        {
            if (job[0] == 0)
                transform_roll((sint32)job[1]);
            else if (job[0] == 1)
                transform_roll_idx(job[1]);
            else if (job[0] == 2)
                transform_reload();
            else if (job[0] == 3)
                transform_pitch_idx(job[1]);
            else
                transform_yaw_idx(job[1]);
        }
        else if (job[0] == 0x80000000)
            transform_identity();
        else
            transform_pitch((sint32)job[0]);
        if (geometry_workspace.matrix_storage != (mode == 1 && job[1] ? GEOMETRY_MATRIX_MODEL : GEOMETRY_MATRIX_SCRATCH) || geometry_workspace.matrix != &base[mode == 1 ? slot + (job[0] == 0 ? 1 : job[0] == 1 ? -1 : 0) : slot])
            abort();
        if (mode == 1)
        {
            POSE_MATRIX *guard = job[1] ? model_guard.matrix : scratch_guard.matrix;
            observation[0] = job[1];
            observation[1] = (uint32)(geometry_workspace.matrix - base);
            for (i = 0; i < 3; i++)
            {
                geometry_audit_store(&base[slot - 1 + i], observation + 2 + 8 * i);
                memcpy(&guard[slot - 1 + i], &base[slot - 1 + i], sizeof(*guard));
            }
            for (i = 0; i < 8; i++)
                observation[26 + i] = xport_gte_read_control(i);
        }
        else if (mode != 3)
        {
            geometry_audit_store(&base[slot], observation);
            memcpy(&scratch_guard.matrix[slot], &base[slot], sizeof(*base));
            for (i = 0; i < 5; i++)
            {
                observation[8 + i] = base[slot + 1].rotation_words[i];
                scratch_guard.matrix[slot + 1].rotation_words[i] = base[slot + 1].rotation_words[i];
            }
            if (mode == 0)
                observation[13] = (uint32)(geometry_workspace.matrix - base);
            else if (mode == 2)
                observation[13] = proj_horizon;
            else if (mode == 5)
            {
                /* The existing wire observes controls only for matrix reload */
                for (i = 0; i < 8; i++)
                    observation[13 + i] = job[0] == 2 ? xport_gte_read_control(i) : 0;
            }
        }
        if (memcmp(&scratch_guard, &geometry_workspace.scratch, sizeof(scratch_guard)) || memcmp(&model_guard, &geometry_workspace.model, sizeof(model_guard)) || memcmp(&camera_guard, &camera, sizeof(camera_guard)))
            abort();
        if (fwrite(observation, output_words[mode] * 4, 1, out) != 1)
            result = 4;
    }
    if (ferror(in) || (!result && got))
        result = 4;
    if (fclose(in))
        result = 4;
    if (fclose(out))
        result = 4;
    if (!result)
        diagnostic_end();
    return result;
}
#endif

int game_run_arguments(int argc, char **argv)
{
#if defined(__EMSCRIPTEN__) || defined(NDEBUG)
    /* Keep the native audit dispatcher in Debug builds only */
    if (!diagnostic_options(&argc, argv))
        return 2;
    return frontend_run(0, 0);
#else
    uint32 frame = 0, limit = 0;
    int headless = 0;
    if (!diagnostic_options(&argc, argv))
        return 2;
    if (argc == 1)
        return frontend_run(0, 0);
    if (argc == 2 && !strcmp(argv[1], "--movie-smoke"))
    {
        DIAGNOSTIC(fputs("Legacy movie smoke is retired; native FMV verification is pending\n", stderr));
        return 2;
    }
    if (argc > 1 && !strcmp(argv[1], "--audit-player-footstep"))
    {
        const uint32 output_header[4] = {0x324e4846, 1, 1, 76};
        FILE *in, *out;
        uint32 header[4], job[2], cases = 0;
        size_t got = 0;
        int result = 0;
        in = xport_fopen("../status/gameplay/player-footstep-jobs.bin", "rb");
        out = xport_fopen("../status/gameplay/player-footstep-native.bin", "wb");
        if (!in || !out)
        {
            if (in)
                fclose(in);
            if (out)
                fclose(out);
            return 3;
        }
        if (fread(header, sizeof(header), 1, in) != 1 || header[0] != 0x324a4846 || header[1] != 1 || header[2] != 2 || header[3] != 76 || fwrite(output_header, sizeof(output_header), 1, out) != 1)
            result = 3;
        while (!result && (got = fread(job, 1, sizeof(job), in)) == sizeof(job))
        {
            if (job[0] >= 4)
            {
                result = 3;
                break;
            }
            random_seed = job[1];
            player_play_footstep((sint32)job[0]);
            if (fwrite(&random_seed, sizeof(random_seed), 1, out) != 1 || ++cases > header[3])
                result = 3;
        }
        if (ferror(in) || (!result && got) || cases != 76 || ferror(out))
            result = 3;
        if (fclose(in))
            result = 3;
        if (fclose(out))
            result = 3;
        if (!result)
            diagnostic_end();
        return result;
    }
    if (argc > 1 && !strcmp(argv[1], "--audit-player-separation"))
    {
        const uint32 output_header[4] = {0x324e5a46, 1, 11, 240};
        FILE *in, *out;
        uint32 header[4], job[11], cases = 0;
        size_t got = 0;
        int result = 0;
        in = xport_fopen("../status/gameplay/player-separation-jobs.bin", "rb");
        out = xport_fopen("../status/gameplay/player-separation-native.bin", "wb");
        if (!in || !out)
        {
            if (in)
                fclose(in);
            if (out)
                fclose(out);
            return 3;
        }
        if (fread(header, sizeof(header), 1, in) != 1 || header[0] != 0x324a5a46 || header[1] != 1 || header[2] != 11 || header[3] != 240 || fwrite(output_header, sizeof(output_header), 1, out) != 1)
            result = 3;
        while (!result && (got = fread(job, 1, sizeof(job), in)) == sizeof(job))
        {
            ACTOR_ANIMATION animation_guard[ACTOR_COUNT];
            ACTOR_MOVEMENT movement_guard[ACTOR_COUNT];
            ACTOR_ORIENTATION orientation_guard[ACTOR_COUNT];
            ACTOR_HEALTH health_guard[ACTOR_COUNT];
            FIXED_VEC3 position_guard[ACTOR_COUNT];
            uint32 fields[11], count_guard;
            sint32 distance_guard;
            if (job[0] >= PLAYER_COUNT)
            {
                result = 3;
                break;
            }
            game_session.player_count = job[1];
            actor_animations[job[0]].id = job[2];
            actor_movement[job[0]].speed = job[3];
            actor_orientation[job[0]].yaw = (sint16)(uint16)job[4];
            actor_health[0].hp = (sint16)(uint16)job[5];
            actor_health[1].hp = (sint16)(uint16)job[6];
            actor_positions[0].x.bits = job[7];
            actor_positions[0].z.bits = job[8];
            actor_positions[1].x.bits = job[9];
            actor_positions[1].z.bits = job[10];
            memcpy(animation_guard, actor_animations, sizeof(animation_guard));
            memcpy(movement_guard, actor_movement, sizeof(movement_guard));
            memcpy(orientation_guard, actor_orientation, sizeof(orientation_guard));
            memcpy(health_guard, actor_health, sizeof(health_guard));
            memcpy(position_guard, actor_positions, sizeof(position_guard));
            count_guard = game_session.player_count;
            distance_guard = dir_distance;
            fields[0] = (uint32)player_step_exceeds_bounds((sint32)job[0]);
            fields[1] = game_session.player_count;
            fields[2] = actor_animations[job[0]].id;
            fields[3] = actor_movement[job[0]].speed;
            fields[4] = (uint16)actor_orientation[job[0]].yaw;
            fields[5] = (uint16)actor_health[0].hp;
            fields[6] = (uint16)actor_health[1].hp;
            fields[7] = actor_positions[0].x.bits;
            fields[8] = actor_positions[0].z.bits;
            fields[9] = actor_positions[1].x.bits;
            fields[10] = actor_positions[1].z.bits;
            if (memcmp(animation_guard, actor_animations, sizeof(animation_guard)) || memcmp(movement_guard, actor_movement, sizeof(movement_guard)) || memcmp(orientation_guard, actor_orientation, sizeof(orientation_guard)) || memcmp(health_guard, actor_health, sizeof(health_guard)) || memcmp(position_guard, actor_positions, sizeof(position_guard)) || count_guard != game_session.player_count || distance_guard != dir_distance)
                abort();
            if (fwrite(fields, sizeof(fields), 1, out) != 1 || ++cases > header[3])
                result = 3;
        }
        if (ferror(in) || (!result && got) || cases != 240 || ferror(out))
            result = 3;
        if (fclose(in))
            result = 3;
        if (fclose(out))
            result = 3;
        if (!result)
            diagnostic_end();
        return result;
    }
    if (argc > 1 && !strcmp(argv[1], "--audit-player-direction"))
    {
        const uint32 output_header[4] = {0x324e5146, 1, 6, 435};
        FILE *in, *out;
        uint32 header[4], job[8], cases = 0;
        size_t got = 0;
        int result = 0;
        in = xport_fopen("../status/gameplay/player-direction-jobs.bin", "rb");
        out = xport_fopen("../status/gameplay/player-direction-native.bin", "wb");
        if (!in || !out)
        {
            if (in)
                fclose(in);
            if (out)
                fclose(out);
            return 3;
        }
        if (fread(header, sizeof(header), 1, in) != 1 || header[0] != 0x324a5146 || header[1] != 1 || header[2] != 8 || header[3] != 435 || fwrite(output_header, sizeof(output_header), 1, out) != 1)
            result = 3;
        while (!result && (got = fread(job, 1, sizeof(job), in)) == sizeof(job))
        {
            ACTOR_ANIMATION animation_guard[ACTOR_COUNT];
            ACTOR_MOVEMENT movement_guard[ACTOR_COUNT];
            ACTOR_ORIENTATION orientation_guard[ACTOR_COUNT];
            CAMERA camera_guard;
            PAD_DECODE_STATE pad_guard;
            uint32 fields[6], buttons = job[0];
            actor_animations[0].frame = job[6];
            actor_animations[0].id = job[5];
            actor_orientation[0].yaw = (sint16)(uint16)job[1];
            actor_movement[0].desired_yaw = (uint16)job[2];
            camera.frame_angles.yaw_bits = (uint16)job[3];
            /* The original entry starts after analog admission */
            pad_decode = (PAD_DECODE_STATE){0};
            memcpy(animation_guard, actor_animations, sizeof(animation_guard));
            memcpy(movement_guard, actor_movement, sizeof(movement_guard));
            memcpy(orientation_guard, actor_orientation, sizeof(orientation_guard));
            memcpy(&camera_guard, &camera, sizeof(camera_guard));
            memcpy(&pad_guard, &pad_decode, sizeof(pad_guard));
            fields[0] = player_update_dir(0, (sint32)job[4], &buttons, job[7]);
            fields[1] = buttons;
            fields[2] = actor_animations[0].frame;
            fields[3] = actor_animations[0].id;
            fields[4] = (uint16)actor_orientation[0].yaw;
            fields[5] = actor_movement[0].desired_yaw;
            movement_guard[0].desired_yaw = actor_movement[0].desired_yaw;
            orientation_guard[0].yaw = actor_orientation[0].yaw;
            if (memcmp(animation_guard, actor_animations, sizeof(animation_guard)) || memcmp(movement_guard, actor_movement, sizeof(movement_guard)) || memcmp(orientation_guard, actor_orientation, sizeof(orientation_guard)) || memcmp(&camera_guard, &camera, sizeof(camera_guard)) || memcmp(&pad_guard, &pad_decode, sizeof(pad_guard)))
                abort();
            if (fwrite(fields, sizeof(fields), 1, out) != 1 || ++cases > header[3])
                result = 3;
        }
        if (ferror(in) || (!result && got) || cases != 435 || ferror(out))
            result = 3;
        if (fclose(in))
            result = 3;
        if (fclose(out))
            result = 3;
        if (!result)
            diagnostic_end();
        return result;
    }
    if (argc > 1 && !strcmp(argv[1], "--audit-score"))
    {
        static uint8 workspace_guard[RENDER_WORKSPACE_BYTES];
        static TEXTURE_DESCRIPTORS descriptor_guard;
        const uint32 output_header[4] = {0x324e4b46, 1, 524, 108};
        FILE *in, *out;
        uint32 header[6], cases = 0;
        char value[16];
        size_t got = 0;
        int result = 0;
        in = xport_fopen("../status/gameplay/score-jobs.bin", "rb");
        out = xport_fopen("../status/gameplay/score-native.bin", "wb");
        if (!in || !out)
        {
            if (in)
                fclose(in);
            if (out)
                fclose(out);
            return 3;
        }
        if (fread(header, sizeof(header), 1, in) != 1 || header[0] != 0x324a4b46 || header[1] != 1 || header[2] != 108 || header[3] > TEXTURE_MAP_COUNT - 39 || header[4] > TEXTURE_DESCRIPTOR_CAPACITY - 10 || header[5] != 10)
            result = 3;
        if (!result && (fread(tex_desc.item + header[4], 10 * sizeof(TEXTURE_DESCRIPTOR), 1, in) != 1 || fwrite(output_header, sizeof(output_header), 1, out) != 1))
            result = 3;
        if (!result)
        {
            tex_grps[TEXTURE_GROUP_GAME_FONT].first_tex = (sint16)header[3];
            tex_map[header[3] + 38] = (sint16)header[4];
        }
        while (!result && (got = fread(value, 1, sizeof(value), in)) == sizeof(value))
        {
            uint32 observed[3], used;
            uint32 *head = psx_addr(0x801ccffc, sizeof(uint32));
            uint8 *packet = psx_addr(0x801d0000, 512);
            uint8 *workspace = psx_addr(RENDER_WORKSPACE_BASE, RENDER_WORKSPACE_BYTES);
            char text_guard[16];
            sint16 depth_guard[DEPTH_TABLE_COUNT], map_guard[TEXTURE_MAP_COUNT];
            TEXTURE_GROUP group_guard[TEXTURE_GROUP_COUNT];
            RENDER_BUFFERS buffers_guard;
            if (!memchr(value, 0, sizeof(value)))
            {
                result = 3;
                break;
            }
            render_buffers.packet_addr = 0x801d0000;
            render_buffers.ot_addr = 0x801cb000;
            depth_table[0] = 8188;
            *head = 0xffffff;
            memset(packet, 0xa5, 512);
            memcpy(workspace_guard, workspace, sizeof(workspace_guard));
            memcpy(&descriptor_guard, &tex_desc, sizeof(descriptor_guard));
            memcpy(depth_guard, depth_table, sizeof(depth_guard));
            memcpy(map_guard, tex_map, sizeof(map_guard));
            memcpy(group_guard, tex_grps, sizeof(group_guard));
            memcpy(text_guard, value, sizeof(text_guard));
            memcpy(&buffers_guard, &render_buffers, sizeof(buffers_guard));
            observed[0] = text_draw_digits(value, sizeof(value), 64, 16);
            observed[1] = render_buffers.packet_addr;
            observed[2] = *head;
            used = render_buffers.packet_addr - 0x801d0000;
            if (used > 512)
                abort();
            memcpy(workspace_guard + 0x1ccffc - RENDER_WORKSPACE_BASE, head, sizeof(*head));
            memcpy(workspace_guard + 0x1d0000 - RENDER_WORKSPACE_BASE, packet, used);
            buffers_guard.packet_addr = render_buffers.packet_addr;
            if (memcmp(workspace_guard, workspace, sizeof(workspace_guard)) || memcmp(&descriptor_guard, &tex_desc, sizeof(descriptor_guard)) || memcmp(depth_guard, depth_table, sizeof(depth_guard)) || memcmp(map_guard, tex_map, sizeof(map_guard)) || memcmp(group_guard, tex_grps, sizeof(group_guard)) || memcmp(text_guard, value, sizeof(text_guard)) || memcmp(&buffers_guard, &render_buffers, sizeof(buffers_guard)))
                abort();
            if (fwrite(observed, sizeof(observed), 1, out) != 1 || fwrite(packet, 512, 1, out) != 1 || ++cases > header[2])
                result = 3;
        }
        if (ferror(in) || (!result && got) || cases != 108 || ferror(out))
            result = 3;
        if (fclose(in))
            result = 3;
        if (fclose(out))
            result = 3;
        if (!result)
            diagnostic_end();
        return result;
    }
    if (argc > 1 && !strcmp(argv[1], "--audit-wobble"))
    {
        const uint32 output_header[4] = {0x324e5746, 1, 9, 70};
        FILE *in, *out;
        uint32 header[4], job[3], cases = 0;
        size_t got = 0;
        int result = 0;
        in = xport_fopen("../status/gameplay/wobble-jobs.bin", "rb");
        out = xport_fopen("../status/gameplay/wobble-native.bin", "wb");
        if (!in || !out)
        {
            if (in)
                fclose(in);
            if (out)
                fclose(out);
            return 3;
        }
        if (fread(header, sizeof(header), 1, in) != 1 || header[0] != 0x324a5746 || header[1] != 1 || header[2] != 3 || header[3] != 70 || fwrite(output_header, sizeof(output_header), 1, out) != 1)
            result = 3;
        while (!result && (got = fread(job, 1, sizeof(job), in)) == sizeof(job))
        {
            MODEL_OBJECT model = {0}, unchanged;
            uint32 fields[9], seed_guard = random_seed;
            model.kind = job[0];
            model.pos.x.bits = 0xa5a5a5a5;
            model.pos.y.bits = 0xa5a5a5a5;
            model.pos.z.bits = 0xa5a5a5a5;
            model.angles.yaw_bits = 0xa5a5;
            model.angles.pitch_bits = (uint16)job[1];
            model.angles.roll_bits = 0xa5a5;
            model.payload_kind = MODEL_PAYLOAD_WOBBLE;
            model.wobble.ticks = (sint16)(uint16)job[2];
            memcpy(&unchanged, &model, sizeof(unchanged));
            fields[0] = (uint32)object_update_wobble(&model);
            fields[1] = model.kind;
            fields[2] = model.pos.x.bits;
            fields[3] = model.pos.y.bits;
            fields[4] = model.pos.z.bits;
            fields[5] = model.angles.yaw_bits;
            fields[6] = model.angles.pitch_bits;
            fields[7] = model.angles.roll_bits;
            fields[8] = (uint16)model.wobble.ticks;
            unchanged.pos.y.bits = model.pos.y.bits;
            unchanged.angles.bits[job[0] - 786u < 2u ? 2 : 1] = model.angles.bits[job[0] - 786u < 2u ? 2 : 1];
            unchanged.wobble.ticks = model.wobble.ticks;
            if (memcmp(&unchanged, &model, sizeof(unchanged)) || seed_guard != random_seed)
                abort();
            if (fwrite(fields, sizeof(fields), 1, out) != 1 || ++cases > header[3])
                result = 3;
        }
        if (ferror(in) || (!result && got) || cases != 70 || ferror(out))
            result = 3;
        if (fclose(in))
            result = 3;
        if (fclose(out))
            result = 3;
        if (!result)
            diagnostic_end();
        return result;
    }
    if (argc > 1 && !strcmp(argv[1], "--audit-damage"))
    {
        const uint32 output_header[4] = {0x324e4946, 1, 49, 36};
        FILE *in, *out;
        uint32 job[9], cases = 0, i;
        size_t got = 0;
        int result = 0;
        in = xport_fopen("../status/gameplay/damage-jobs.bin", "rb");
        out = xport_fopen("../status/gameplay/damage-native.bin", "wb");
        if (!in || !out)
        {
            if (in)
                fclose(in);
            if (out)
                fclose(out);
            return 3;
        }
        if (fwrite(output_header, sizeof(output_header), 1, out) != 1)
            result = 3;
        while (!result && (got = fread(job, 1, sizeof(job), in)) == sizeof(job))
        {
            NPC_BEHAVIOR_ROW row = {0}, row_guard;
            NPC_BEHAVIOR_ACTOR behavior_guard[ACTOR_COUNT];
            ACTOR_HEALTH health_guard[ACTOR_COUNT];
            ACTOR_CONTACT contact_guard[ACTOR_COUNT];
            GAME_SETTINGS settings_guard;
            GAME_SESSION session_guard;
            uint32 fields[49];
            row.damage_out = (uint8)job[3];
            row.damage_in = (uint8)job[4];
            for (i = 0; i < ACTOR_COUNT; i++)
            {
                npc_behavior_actors[i].profile = (NPC_BEHAVIOR_REFERENCE){&row, 1, 0};
                npc_behavior_actors[i].behavior = 0;
                actor_health[i] = (ACTOR_HEALTH){0};
                actor_health[i].hp = 400;
                actor_health[i].stun = 32766;
                actor_contacts[i] = (ACTOR_CONTACT){0};
                actor_contacts[i].last_target = (sint8)0xa5;
                actor_contacts[i].last_attacker = (sint8)0xa5;
                actor_contacts[i].captor = (sint8)0xa5;
                actor_contacts[i].captive = (sint8)(uint8)job[8];
            }
            game_settings.difficulty_bits = (uint8)job[5];
            game_settings.flags = job[6];
            game_session.mode = job[7];
            memcpy(&row_guard, &row, sizeof(row_guard));
            memcpy(behavior_guard, npc_behavior_actors, sizeof(behavior_guard));
            memcpy(health_guard, actor_health, sizeof(health_guard));
            memcpy(contact_guard, actor_contacts, sizeof(contact_guard));
            memcpy(&settings_guard, &game_settings, sizeof(settings_guard));
            memcpy(&session_guard, &game_session, sizeof(session_guard));
            fields[0] = actor_apply_damage((sint32)job[0], (sint32)job[1], (sint32)job[2]);
            for (i = 0; i < ACTOR_COUNT; i++)
            {
                fields[1 + 6 * i] = (uint16)actor_health[i].hp;
                fields[2 + 6 * i] = (uint16)actor_health[i].stun;
                fields[3 + 6 * i] = (uint8)actor_contacts[i].last_target;
                fields[4 + 6 * i] = (uint8)actor_contacts[i].last_attacker;
                fields[5 + 6 * i] = (uint8)actor_contacts[i].captor;
                fields[6 + 6 * i] = (uint8)actor_contacts[i].captive;
                health_guard[i].hp = actor_health[i].hp;
                health_guard[i].stun = actor_health[i].stun;
                contact_guard[i].last_target = actor_contacts[i].last_target;
                contact_guard[i].last_attacker = actor_contacts[i].last_attacker;
                contact_guard[i].captor = actor_contacts[i].captor;
                contact_guard[i].captive = actor_contacts[i].captive;
            }
            if (memcmp(&row_guard, &row, sizeof(row_guard)) || memcmp(behavior_guard, npc_behavior_actors, sizeof(behavior_guard)) || memcmp(health_guard, actor_health, sizeof(health_guard)) || memcmp(contact_guard, actor_contacts, sizeof(contact_guard)) || memcmp(&settings_guard, &game_settings, sizeof(settings_guard)) || memcmp(&session_guard, &game_session, sizeof(session_guard)))
                abort();
            if (fwrite(fields, sizeof(fields), 1, out) != 1 || ++cases > 36)
                result = 3;
        }
        for (i = 0; i < ACTOR_COUNT; i++)
            npc_behavior_actors[i].profile = (NPC_BEHAVIOR_REFERENCE){0};
        if (ferror(in) || (!result && got) || cases != 36 || ferror(out))
            result = 3;
        if (fclose(in))
            result = 3;
        if (fclose(out))
            result = 3;
        if (!result)
            diagnostic_end();
        return result;
    }
    if (argc > 1 && !strcmp(argv[1], "--audit-pain-sound"))
    {
        FILE *in, *out;
        uint32 job[2], cases = 0;
        sint32 parameters[3];
        size_t got = 0;
        int result = 0;
        in = xport_fopen("../status/gameplay/pain-sound-jobs.bin", "rb");
        out = xport_fopen("../status/gameplay/pain-sound-native.bin", "wb");
        if (!in || !out)
        {
            if (in)
                fclose(in);
            if (out)
                fclose(out);
            return 3;
        }
        while (!result && (got = fread(job, 1, sizeof(job), in)) == sizeof(job))
        {
            sint16 types_guard[ACTOR_COUNT];
            actor_types[0] = (sint16)(uint16)job[0];
            random_seed = job[1];
            memcpy(types_guard, actor_types, sizeof(types_guard));
            sound_pain_parameters(0, parameters);
            if (memcmp(types_guard, actor_types, sizeof(types_guard)))
                abort();
            if (fwrite(parameters, sizeof(parameters), 1, out) != 1 || fwrite(&random_seed, sizeof(random_seed), 1, out) != 1 || ++cases > 56)
                result = 3;
        }
        if (ferror(in) || (!result && got) || cases != 56 || ferror(out))
            result = 3;
        if (fclose(in))
            result = 3;
        if (fclose(out))
            result = 3;
        if (!result)
            diagnostic_end();
        return result;
    }
    if (argc > 1 && !strcmp(argv[1], "--audit-number"))
    {
        FILE *in, *out;
        uint32 value, observed, cases = 0, i;

        struct
        {
            uint8 before[16];
            char digits[4];
            uint8 after[16];
        } buffer;

        size_t got = 0;
        int result = 0;
        in = xport_fopen("../status/gameplay/number-jobs.bin", "rb");
        out = xport_fopen("../status/gameplay/number-native.bin", "wb");
        if (!in || !out)
        {
            if (in)
                fclose(in);
            if (out)
                fclose(out);
            return 3;
        }
        while (!result && (got = fread(&value, 1, sizeof(value), in)) == sizeof(value))
        {
            memset(&buffer, 0xa5, sizeof(buffer));
            observed = text_format_digits((sint32)value, buffer.digits);
            for (i = 0; i < sizeof(buffer.before); i++)
                if (buffer.before[i] != 0xa5 || buffer.after[i] != 0xa5)
                    abort();
            if (fwrite(&observed, sizeof(observed), 1, out) != 1 || fwrite(buffer.digits, sizeof(buffer.digits), 1, out) != 1 || ++cases > 150)
                result = 3;
        }
        if (ferror(in) || (!result && got) || cases != 150 || ferror(out))
            result = 3;
        if (fclose(in))
            result = 3;
        if (fclose(out))
            result = 3;
        if (!result)
            diagnostic_end();
        return result;
    }

    if (argc > 1 && (!strcmp(argv[1], "--audit-healthbar") || !strcmp(argv[1], "--audit-rightbar")))
    {
        static uint8 workspace_guard[RENDER_WORKSPACE_BYTES];
        static TEXTURE_DESCRIPTORS descriptor_guard;
        int right = !strcmp(argv[1], "--audit-rightbar");
        uint32 arena_bytes = right ? 32768 : 128;
        uint32 output_header[5] = {0x324e4246, 1, (uint32)right, arena_bytes + 12, 54};
        FILE *in, *out;
        uint32 header[6], job[6], cases = 0;
        size_t got = 0;
        int result = 0;
        in = xport_fopen(right ? "../status/gameplay/rightbar-jobs.bin" : "../status/gameplay/healthbar-jobs.bin", "rb");
        out = xport_fopen(right ? "../status/gameplay/rightbar-native.bin" : "../status/gameplay/healthbar-native.bin", "wb");
        if (!in || !out)
        {
            if (in)
                fclose(in);
            if (out)
                fclose(out);
            return 3;
        }
        if (fread(header, sizeof(header), 1, in) != 1 || header[0] != 0x324a4246 || header[1] != 1 || header[2] != (uint32)right || header[3] != 54 || header[4] > TEXTURE_MAP_COUNT - 60 || header[5] >= TEXTURE_DESCRIPTOR_CAPACITY)
            result = 3;
        if (!result && (fread(tex_desc.item + header[5], sizeof(TEXTURE_DESCRIPTOR), 1, in) != 1 || fwrite(output_header, sizeof(output_header), 1, out) != 1))
            result = 3;
        if (!result)
        {
            tex_grps[TEXTURE_GROUP_GAME_FONT].first_tex = (sint16)header[4];
            tex_map[header[4] + 59] = (sint16)header[5];
        }
        while (!result && (got = fread(job, 1, sizeof(job), in)) == sizeof(job))
        {
            uint32 observed[3], used;
            uint32 *head = psx_addr(0x801ccffc, sizeof(uint32));
            uint8 *packet = psx_addr(0x801d0000, arena_bytes);
            uint8 *workspace = psx_addr(RENDER_WORKSPACE_BASE, RENDER_WORKSPACE_BYTES);
            ACTOR_HEALTH health_guard[ACTOR_COUNT];
            sint16 types_guard[ACTOR_COUNT];
            sint16 depth_guard[DEPTH_TABLE_COUNT], map_guard[TEXTURE_MAP_COUNT];
            TEXTURE_GROUP group_guard[TEXTURE_GROUP_COUNT];
            RENDER_BUFFERS buffers_guard;
            actor_types[0] = (sint16)(uint16)job[1];
            actor_health[0].hp = (sint16)(uint16)job[2];
            actor_health[0].shown = (sint16)(uint16)job[3];
            actor_health[0].trail = (sint16)(uint16)job[4];
            actor_health[0].max = (sint16)(uint16)job[5];
            render_buffers.packet_addr = 0x801d0000;
            render_buffers.ot_addr = 0x801cb000;
            depth_table[0] = 8188;
            *head = 0xffffff;
            memset(packet, 0xa5, arena_bytes);
            memcpy(workspace_guard, workspace, sizeof(workspace_guard));
            memcpy(&descriptor_guard, &tex_desc, sizeof(descriptor_guard));
            memcpy(depth_guard, depth_table, sizeof(depth_guard));
            memcpy(map_guard, tex_map, sizeof(map_guard));
            memcpy(group_guard, tex_grps, sizeof(group_guard));
            memcpy(health_guard, actor_health, sizeof(health_guard));
            memcpy(types_guard, actor_types, sizeof(types_guard));
            memcpy(&buffers_guard, &render_buffers, sizeof(buffers_guard));
            observed[0] = right ? hud_draw_health_right(job[0], 0) : hud_draw_health_left(job[0], 0);
            observed[1] = render_buffers.packet_addr;
            observed[2] = *head;
            used = render_buffers.packet_addr - 0x801d0000;
            if (used > arena_bytes)
                abort();
            memcpy(workspace_guard + 0x1ccffc - RENDER_WORKSPACE_BASE, head, sizeof(*head));
            memcpy(workspace_guard + 0x1d0000 - RENDER_WORKSPACE_BASE, packet, used);
            buffers_guard.packet_addr = render_buffers.packet_addr;
            if (memcmp(workspace_guard, workspace, sizeof(workspace_guard)) || memcmp(&descriptor_guard, &tex_desc, sizeof(descriptor_guard)) || memcmp(depth_guard, depth_table, sizeof(depth_guard)) || memcmp(map_guard, tex_map, sizeof(map_guard)) || memcmp(group_guard, tex_grps, sizeof(group_guard)) || memcmp(health_guard, actor_health, sizeof(health_guard)) || memcmp(types_guard, actor_types, sizeof(types_guard)) || memcmp(&buffers_guard, &render_buffers, sizeof(buffers_guard)))
                abort();
            if (fwrite(observed, sizeof(observed), 1, out) != 1 || fwrite(packet, arena_bytes, 1, out) != 1 || ++cases > header[3])
                result = 3;
        }
        if (ferror(in) || (!result && got) || cases != 54 || ferror(out))
            result = 3;
        if (fclose(in))
            result = 3;
        if (fclose(out))
            result = 3;
        if (!result)
            diagnostic_end();
        return result;
    }
    if (argc > 1 && !strcmp(argv[1], "--audit-held-animation"))
    {
        FILE *in, *out;
        uint32 header[4], bank_header[6], ids[ANIMATION_COUNT], rows[ANIMATION_COUNT + 1][3];
        uint32 seed[15], jobs[109][5], i, j, count = 0, single = UINT32_MAX, result_word;
        uint32 output_header[4] = {0x334e4846, 1, 112, 0};
        uint8 history[36], *blob = NULL, *blob_guard = NULL;
        uint16 flags[ANIMATION_COUNT];
        ANIMATION_STREAM sources[ANIMATION_COUNT + 2];
        ANIMATION_BANK bank;
        int result = 0;
        if (argc > 2)
        {
            char *end;
            unsigned long ordinal;
            if (argc != 4 || strcmp(argv[2], "--stop-case") || argv[3][0] < '0' || argv[3][0] > '9')
                return 4;
            ordinal = strtoul(argv[3], &end, 10);
            if (*end || ordinal >= 109)
                return 4;
            single = (uint32)ordinal;
        }
        in = xport_fopen("../status/gameplay/held-animation-jobs.bin", "rb");
        out = xport_fopen("../status/gameplay/held-animation-native.bin", "wb");
        if (!in || !out)
        {
            if (in)
                fclose(in);
            if (out)
                fclose(out);
            return 3;
        }
        if (fread(header, sizeof(header), 1, in) != 1 || header[0] != 0x334a4846 || header[1] != 1 || header[2] != 109 || fread(bank_header, sizeof(bank_header), 1, in) != 1 || bank_header[0] != 0x334a4246 || bank_header[1] != 1 || bank_header[2] != ANIMATION_COUNT || bank_header[3] > ANIMATION_COUNT + 1 || bank_header[4] > (ANIMATION_COUNT + 1) * (65547 + 3) || bank_header[5] > bank_header[3] || header[3] != 24 + ANIMATION_COUNT * 6 + bank_header[3] * 12 + bank_header[4] || fread(ids, sizeof(ids), 1, in) != 1 || fread(flags, sizeof(flags), 1, in) != 1 || fread(rows, 12, bank_header[3], in) != bank_header[3])
            result = 3;
        if (!result)
        {
            blob = malloc(bank_header[4] ? bank_header[4] : 4);
            blob_guard = malloc(bank_header[4] ? bank_header[4] : 4);
            if (!blob || !blob_guard || fread(blob, 1, bank_header[4], in) != bank_header[4] || fread(seed, sizeof(seed), 1, in) != 1 || seed[9] != 0 || fread(history, sizeof(history), 1, in) != 1 || fread(jobs, sizeof(jobs), 1, in) != 1 || fgetc(in) != EOF || ferror(in))
                result = 3;
        }
        if (!result)
        {
            memset(sources, 0, sizeof(sources));
            memset(&bank, 0, sizeof(bank));
            memcpy(blob_guard, blob, bank_header[4]);
            for (i = 0; !result && i < bank_header[3]; i++)
            {
                uint32 domain = rows[i][0], offset = rows[i][1], bytes = rows[i][2];
                if (!domain || domain >= ANIMATION_STREAM_DOMAIN_COUNT || (offset & 3) || bytes < 8 || bytes > 65547 || offset > bank_header[4] || bytes > bank_header[4] - offset)
                {
                    result = 3;
                    break;
                }
                for (j = 0; j < i; j++)
                    if (rows[j][0] == domain && rows[j][1] == offset)
                        result = 3;
                sources[i + 1] = (ANIMATION_STREAM){(const ANIMATION_STREAM_HEADER *)(blob + offset), bytes, (ANIMATION_STREAM_DOMAIN)domain};
            }
            for (i = 0; !result && i < ANIMATION_COUNT; i++)
            {
                if (ids[i] > bank_header[3])
                    result = 3;
                else if (ids[i])
                    bank.stream[i] = (ACTOR_ANIMATION_SOURCE){ACTOR_ANIMATION_SOURCE_STREAM, sources[ids[i]]};
            }
            if (!ids[69] || !ids[71] || seed[10] > 65535 || seed[11] > 65535 || seed[13] > 65535)
                result = 3;
            for (i = 0; i < 109; i++)
            {
                if (jobs[i][2] >= ANIMATION_COUNT || jobs[i][3] > 1 || jobs[i][4] > 1)
                    result = 3;
                ++count;
            }
        }
        output_header[3] = single == UINT32_MAX ? count : 1;
        if (!result && fwrite(output_header, sizeof(output_header), 1, out) != 1)
            result = 3;
        for (i = 0; !result && i < 109; i++)
        {
            ACTOR_ANIMATION *playback = actor_animations;
            if (single != UINT32_MAX && i != single)
                continue;
            memset(playback, 0, sizeof(*playback));
            if (bank_header[5])
                playback->src = (ACTOR_ANIMATION_SOURCE){ACTOR_ANIMATION_SOURCE_STREAM, sources[bank_header[5]]};
            playback->frame = seed[1];
            playback->id = seed[2];
            memcpy(playback->motion.bits, seed + 3, sizeof(playback->motion.bits));
            playback->motion.events = seed[6];
            playback->motion.forward_offset_bits = seed[7];
            actor_positions[0].y.bits = seed[8];
            actor_types[0] = (sint16)seed[9];
            playback->hit_flags = (uint16)seed[10];
            playback->event_step = (uint16)seed[11];
            animation_fall_exit = seed[12];
            actor_movement[0].prev_state = (uint16)seed[13];
            playback->selector = seed[14];
            game_session.stage = seed[0];
            memcpy(action_history.action, history, sizeof(action_history.action));
            memcpy(action_history.predicted, history + 32, sizeof(action_history.predicted));
            memcpy(action_history.matched, history + 34, sizeof(action_history.matched));
            actor_items[0].slot[ACTOR_ITEM_LEFT] = jobs[i][0] == UINT32_MAX ? -1 : 0;
            actor_items[0].slot[ACTOR_ITEM_RIGHT] = jobs[i][1] == UINT32_MAX ? -1 : 1;
            dynamic_objects[0].kind = jobs[i][0];
            dynamic_objects[1].kind = jobs[i][1];
            memcpy(animation_flags, flags, sizeof(flags));
            memcpy(animation_banks + ANIMATION_BANK_HAWK, &bank, sizeof(bank));
            if (jobs[i][3])
                animation_banks[ANIMATION_BANK_HAWK].stream[71] = (ACTOR_ANIMATION_SOURCE){0};
            animation_start_audit.out = out;
            animation_start_audit.ordinal = i;
            animation_start_audit.actor = 0;
            animation_start_audit.sources = sources;
            animation_start_audit.source_count = bank_header[3];
            animation_start_audit.source_blob = blob;
            animation_start_audit.source_guard = blob_guard;
            animation_start_audit.source_bytes = bank_header[4];
            memcpy(animation_start_audit.playback, actor_animations, sizeof(animation_start_audit.playback));
            memcpy(animation_start_audit.positions, actor_positions, sizeof(animation_start_audit.positions));
            memcpy(animation_start_audit.types, actor_types, sizeof(animation_start_audit.types));
            memcpy(animation_start_audit.flags, animation_flags, sizeof(animation_start_audit.flags));
            memcpy(&animation_start_audit.bank, animation_banks + ANIMATION_BANK_HAWK, sizeof(animation_start_audit.bank));
            memcpy(&animation_start_audit.session, &game_session, sizeof(animation_start_audit.session));
            memcpy(animation_start_audit.movement, actor_movement, sizeof(animation_start_audit.movement));
            memcpy(animation_start_audit.items, actor_items, sizeof(animation_start_audit.items));
            memcpy(animation_start_audit.objects, dynamic_objects, sizeof(animation_start_audit.objects));
            result_word = (uint32)animation_start_held(0, ANIMATION_BANK_HAWK, (sint32)jobs[i][2]);
            if (!animation_start_audit_store(0, result_word))
                result = 3;
            actor_animations[0].src = (ACTOR_ANIMATION_SOURCE){0};
            memset(animation_banks + ANIMATION_BANK_HAWK, 0, sizeof(ANIMATION_BANK));
        }
        diagnostic_stop_observer = NULL;
        memset(&animation_start_audit, 0, sizeof(animation_start_audit));
        if (fclose(in))
            result = 3;
        if (fclose(out))
            result = 3;
        free(blob_guard);
        free(blob);
        if (!result)
            diagnostic_end();
        return result;
    }
    if (argc > 1 && !strcmp(argv[1], "--audit-action-input"))
    {
        FILE *in, *out;
        uint32 header[4], job[7], output_header[4] = {0x334e4946, 1, 67, 0}, value, cases = 0;
        uint8 bindings[sizeof(pad_bindings)], cfg[PAD_PLAYER_COUNT], latch[PAD_PLAYER_COUNT][PAD_ACTION_COUNT], type;
        PAD_ACTION_STATE action_guard;
        PAD_GAME_STATE game_guard;
        PAD_DECODE_STATE decode_guard;
        size_t got = 0;
        int actual = argc == 3 && !strcmp(argv[2], "--actual"), result = 0;
        if ((argc != 2 && !actual) || argc > 3)
            return 3;
        in = xport_fopen(actual ? "../status/gameplay/action-input-real-jobs.bin" : "../status/gameplay/action-input-jobs.bin", "rb");
        out = xport_fopen(actual ? "../status/gameplay/action-input-real-native.bin" : "../status/gameplay/action-input-native.bin", "wb");
        if (!in || !out)
        {
            if (in)
                fclose(in);
            if (out)
                fclose(out);
            return 3;
        }
        if (fread(header, sizeof(header), 1, in) != 1 || header[0] != 0x334a4946 || header[1] != 1 || header[2] != (actual ? 328u : 128u) || header[3] != sizeof(pad_bindings) || fread(bindings, sizeof(bindings), 1, in) != 1 || memcmp(bindings, pad_bindings, sizeof(bindings)))
            result = 3;
        for (uint32 i = 0; !result && i < sizeof(bindings); i++)
            if (bindings[i] >= PAD_ACTION_COUNT)
                result = 3;
        output_header[3] = actual ? 328 : 128;
        if (!result && fwrite(output_header, sizeof(output_header), 1, out) != 1)
            result = 3;
        while (!result && (got = fread(job, 1, sizeof(job), in)) == sizeof(job))
        {
            uint8 *slot;
            if (job[0] > 2 || (job[0] && job[1] >= PAD_PLAYER_COUNT) || fread(cfg, sizeof(cfg), 1, in) != 1 || cfg[0] >= PAD_SCHEME_COUNT || cfg[1] >= PAD_SCHEME_COUNT || fread(latch, sizeof(latch), 1, in) != 1 || fread(&type, sizeof(type), 1, in) != 1 || cases >= output_header[3])
            {
                result = 3;
                break;
            }
            memcpy(pad_action.cfg, cfg, sizeof(cfg));
            memcpy(pad_action.latch, latch, sizeof(latch));
            pad_game.port[0] = job[3];
            pad_game.port[1] = job[4];
            pad_game.sample[0] = job[5];
            pad_game.sample[1] = job[6];
            pad_decode.type = type;
            memcpy(&action_guard, &pad_action, sizeof(action_guard));
            memcpy(&game_guard, &pad_game, sizeof(game_guard));
            memcpy(&decode_guard, &pad_decode, sizeof(decode_guard));
            if (!job[0])
            {
                value = pad_game_buttons(job[1]);
                decode_guard.type = pad_decode.type;
            }
            else
            {
                uintptr_t begin = (uintptr_t)pad_action.latch[job[1]], selected;
                slot = job[0] == 1 ? pad_action_press(job[1], job[2]) : pad_action_hold(job[1], job[2]);
                value = 0;
                if (slot)
                {
                    selected = (uintptr_t)slot;
                    if (selected < begin || selected - begin >= PAD_ACTION_COUNT)
                        abort();
                    value = (uint32)(selected - begin) + 1;
                }
                memcpy(action_guard.latch[job[1]], pad_action.latch[job[1]], PAD_ACTION_COUNT);
            }
            if (memcmp(&action_guard, &pad_action, sizeof(action_guard)) || memcmp(&game_guard, &pad_game, sizeof(game_guard)) || memcmp(&decode_guard, &pad_decode, sizeof(decode_guard)) || memcmp(bindings, pad_bindings, sizeof(bindings)))
                abort();
            if (fwrite(&value, sizeof(value), 1, out) != 1 || fwrite(pad_action.latch, sizeof(pad_action.latch), 1, out) != 1 || fwrite(pad_action.cfg, sizeof(pad_action.cfg), 1, out) != 1 || fwrite(&pad_decode.type, sizeof(pad_decode.type), 1, out) != 1 || fwrite(pad_bindings, sizeof(pad_bindings), 1, out) != 1)
                result = 3;
            ++cases;
        }
        if (ferror(in) || (!result && got) || cases != output_header[3] || ferror(out))
            result = 3;
        if (fclose(in))
            result = 3;
        if (fclose(out))
            result = 3;
        if (!result)
            diagnostic_end();
        return result;
    }
    if (argc > 1 && !strcmp(argv[1], "--audit-target-selection"))
    {
        const uint32 input_header[4] = {0x334a5446, 1, 59, 45};
        const uint32 output_header[4] = {0x334e5446, 1, 64, 45};
        FILE *in, *out;
        uint32 header[4], seed[7], job[59], fields[64], cases = 0, i;
        FIXED_VEC3 position_guard[ACTOR_COUNT];
        sint16 type_guard[ACTOR_COUNT];
        ACTOR_HEALTH health_guard[ACTOR_COUNT];
        ACTOR_ORIENTATION orientation_guard[ACTOR_COUNT];
        ACTOR_ITEMS hands_guard[ACTOR_COUNT];
        MODEL_OBJECT pool_guard[DYNAMIC_OBJECT_COUNT];
        GAME_SESSION session_guard;
        ACTOR_TARGETING targeting_guard;
        size_t got = 0;
        int result = 0;
        in = xport_fopen("../status/gameplay/target-selection-jobs.bin", "rb");
        out = xport_fopen("../status/gameplay/target-selection-native.bin", "wb");
        if (!in || !out)
        {
            if (in)
                fclose(in);
            if (out)
                fclose(out);
            return 3;
        }
        if (fread(header, sizeof(header), 1, in) != 1 || memcmp(header, input_header, sizeof(header)) || fread(seed, sizeof(seed), 1, in) != 1 || seed[2] > 65535 || seed[4] > 65535 || fwrite(output_header, sizeof(output_header), 1, out) != 1)
            result = 3;
        while (!result && (got = fread(job, 1, sizeof(job), in)) == sizeof(job))
        {
            if (job[0] >= ACTOR_COUNT || cases >= 45)
            {
                result = 3;
                break;
            }
            for (i = 0; i < ACTOR_COUNT; i++)
            {
                uint32 *row = job + 3 + i * 7;
                if (row[3] > 65535 || row[4] > 65535 || row[5] > 65535 || row[6] > 65535)
                    result = 3;
                actor_positions[i].x.bits = row[0];
                actor_positions[i].y.bits = row[1];
                actor_positions[i].z.bits = row[2];
                actor_types[i] = (sint16)(uint16)row[3];
                actor_health[i].hp = (sint16)(uint16)row[4];
                actor_orientation[i].yaw = (sint16)(uint16)row[5];
                actor_items[i].bits = (uint16)row[6];
            }
            if (result)
                break;
            game_session.mode = job[1];
            dynamic_objects[0].kind = job[2];
            memset(&actor_targeting, 0xa5, sizeof(actor_targeting));
            actor_targeting.actor_idx = seed[0];
            actor_targeting.front.available = seed[1];
            actor_targeting.front.angle = (sint16)(uint16)seed[2];
            actor_targeting.rear.available = seed[3];
            actor_targeting.rear.angle = (sint16)(uint16)seed[4];
            actor_targeting.near_distance = (sint32)seed[5];
            dir_distance = (sint32)seed[6];
            memcpy(position_guard, actor_positions, sizeof(position_guard));
            memcpy(type_guard, actor_types, sizeof(type_guard));
            memcpy(health_guard, actor_health, sizeof(health_guard));
            memcpy(orientation_guard, actor_orientation, sizeof(orientation_guard));
            memcpy(hands_guard, actor_items, sizeof(hands_guard));
            memcpy(pool_guard, dynamic_objects, sizeof(pool_guard));
            memcpy(&session_guard, &game_session, sizeof(session_guard));
            memcpy(&targeting_guard, &actor_targeting, sizeof(targeting_guard));
            fields[0] = actors_find_facing_targets((sint32)job[0]);
            fields[1] = actor_targeting.actor_idx;
            fields[2] = actor_targeting.front.available;
            fields[3] = (uint16)actor_targeting.front.angle;
            fields[4] = actor_targeting.rear.available;
            fields[5] = (uint16)actor_targeting.rear.angle;
            fields[6] = (uint32)actor_targeting.near_distance;
            fields[7] = (uint32)dir_distance;
            targeting_guard.actor_idx = actor_targeting.actor_idx;
            memcpy(&targeting_guard.front, &actor_targeting.front, sizeof(targeting_guard.front));
            memcpy(&targeting_guard.rear, &actor_targeting.rear, sizeof(targeting_guard.rear));
            targeting_guard.near_distance = actor_targeting.near_distance;
            if (memcmp(position_guard, actor_positions, sizeof(position_guard)) || memcmp(type_guard, actor_types, sizeof(type_guard)) || memcmp(health_guard, actor_health, sizeof(health_guard)) || memcmp(orientation_guard, actor_orientation, sizeof(orientation_guard)) || memcmp(hands_guard, actor_items, sizeof(hands_guard)) || memcmp(pool_guard, dynamic_objects, sizeof(pool_guard)) || memcmp(&session_guard, &game_session, sizeof(session_guard)) || memcmp(&targeting_guard, &actor_targeting, sizeof(targeting_guard)))
                abort();
            for (i = 0; i < ACTOR_COUNT; i++)
            {
                uint32 *row = fields + 8 + i * 7;
                row[0] = actor_positions[i].x.bits;
                row[1] = actor_positions[i].y.bits;
                row[2] = actor_positions[i].z.bits;
                row[3] = (uint16)actor_types[i];
                row[4] = (uint16)actor_health[i].hp;
                row[5] = (uint16)actor_orientation[i].yaw;
                row[6] = actor_items[i].bits;
            }
            if (fwrite(fields, sizeof(fields), 1, out) != 1)
                result = 3;
            ++cases;
        }
        if (ferror(in) || (!result && got) || cases != 45 || ferror(out))
            result = 3;
        if (fclose(in))
            result = 3;
        if (fclose(out))
            result = 3;
        if (!result)
            diagnostic_end();
        return result;
    }
    if (argc > 1 && !strcmp(argv[1], "--audit-ai-roles"))
    {
        const uint32 output_header[4] = {0x324e4e46, 1, 49, 9};
        FILE *in, *out;
        uint32 header[4], job[48], cases = 0, i;
        size_t got = 0;
        int result = 0;
        in = xport_fopen("../status/gameplay/ai-roles-jobs.bin", "rb");
        out = xport_fopen("../status/gameplay/ai-roles-native.bin", "wb");
        if (!in || !out)
        {
            if (in)
                fclose(in);
            if (out)
                fclose(out);
            return 3;
        }
        if (fread(header, sizeof(header), 1, in) != 1 || header[0] != 0x324a4e46 || header[1] != 1 || header[2] != 48 || header[3] != 9 || fwrite(output_header, sizeof(output_header), 1, out) != 1)
            result = 3;
        while (!result && (got = fread(job, 1, sizeof(job), in)) == sizeof(job))
        {
            FIXED_VEC3 position_guard[ACTOR_COUNT];
            sint16 types_guard[ACTOR_COUNT];
            uint32 targets_guard[ACTOR_COUNT], fields[49];
            sint32 distance_guard = dir_distance;
            for (i = 0; i < ACTOR_COUNT; i++)
            {
                actor_types[i] = (sint16)(uint16)job[6 * i];
                actor_positions[i].x.bits = job[6 * i + 1];
                actor_positions[i].z.bits = job[6 * i + 2];
                npc_targets[i].role = job[6 * i + 3];
                npc_targets[i].idx = job[6 * i + 4];
                npc_target_distances[i] = (sint32)job[6 * i + 5];
                targets_guard[i] = npc_targets[i].idx;
                if (i >= 2 && actor_types[i] != ACTOR_TYPE_NONE && npc_targets[i].role - 1u < 2u && npc_targets[i].idx != 0xffffffff && npc_targets[i].idx >= ACTOR_COUNT)
                    result = 3;
            }
            if (result)
                break;
            memcpy(position_guard, actor_positions, sizeof(position_guard));
            memcpy(types_guard, actor_types, sizeof(types_guard));
            fields[0] = npc_balance_targets();
            for (i = 0; i < ACTOR_COUNT; i++)
            {
                fields[1 + 6 * i] = (uint16)actor_types[i];
                fields[2 + 6 * i] = actor_positions[i].x.bits;
                fields[3 + 6 * i] = actor_positions[i].z.bits;
                fields[4 + 6 * i] = npc_targets[i].role;
                fields[5 + 6 * i] = npc_targets[i].idx;
                fields[6 + 6 * i] = (uint32)npc_target_distances[i];
                if (targets_guard[i] != npc_targets[i].idx)
                    abort();
            }
            if (memcmp(position_guard, actor_positions, sizeof(position_guard)) || memcmp(types_guard, actor_types, sizeof(types_guard)) || distance_guard != dir_distance)
                abort();
            if (fwrite(fields, sizeof(fields), 1, out) != 1 || ++cases > header[3])
                result = 3;
        }
        if (ferror(in) || (!result && got) || cases != 9 || ferror(out))
            result = 3;
        if (fclose(in))
            result = 3;
        if (fclose(out))
            result = 3;
        if (!result)
            diagnostic_end();
        return result;
    }
    if (argc > 1 && !strcmp(argv[1], "--audit-item-selection"))
    {
        static MODEL_OBJECT objects_guard[DYNAMIC_OBJECT_COUNT];
        const uint32 output_header[4] = {0x324e5046, 1, 335, 61};
        FILE *in, *out;
        uint32 header[4], job[337], cases = 0, i;
        size_t got = 0;
        int result = 0;
        in = xport_fopen("../status/gameplay/item-selection-jobs.bin", "rb");
        out = xport_fopen("../status/gameplay/item-selection-native.bin", "wb");
        if (!in || !out)
        {
            if (in)
                fclose(in);
            if (out)
                fclose(out);
            return 3;
        }
        if (fread(header, sizeof(header), 1, in) != 1 || header[0] != 0x324a5046 || header[1] != 1 || header[2] != 337 || header[3] != 61 || fwrite(output_header, sizeof(output_header), 1, out) != 1)
            result = 3;
        while (!result && (got = fread(job, 1, sizeof(job), in)) == sizeof(job))
        {
            FIXED_VEC3 position_guard[ACTOR_COUNT];
            ACTOR_ORIENTATION orientation_guard[ACTOR_COUNT];
            ACTOR_ITEMS hands_guard[ACTOR_COUNT];
            sint16 types_guard[ACTOR_COUNT];
            ITEM_PICKUP pickup_guard;
            uint32 fields[335], count_guard;
            sint32 distance_guard = dir_distance;
            if (job[0] >= ITEM_PICKUP_PLAYER_COUNT)
            {
                result = 3;
                break;
            }
            game_session.player_count = job[2];
            item_pickup.nearby[0] = job[3];
            item_pickup.nearby[1] = job[4];
            for (i = 0; i < 2; i++)
            {
                uint32 at = 5 + 6 * i;
                actor_positions[i].x.bits = job[at];
                actor_positions[i].y.bits = job[at + 1];
                actor_positions[i].z.bits = job[at + 2];
                actor_types[i] = (sint16)(uint16)job[at + 3];
                actor_orientation[i].yaw = (sint16)(uint16)job[at + 4];
                actor_items[i].bits = (uint16)job[at + 5];
            }
            for (i = 0; i < DYNAMIC_OBJECT_COUNT; i++)
            {
                uint32 at = 17 + 5 * i;
                MODEL_OBJECT *model = dynamic_objects + i;
                memset(model, 0, sizeof(*model));
                model->kind = job[at];
                model->pos.x.bits = job[at + 1];
                model->pos.y.bits = job[at + 2];
                model->pos.z.bits = job[at + 3];
                model->payload_kind = MODEL_PAYLOAD_PHYSICS;
                model->physics.life = (sint16)(uint16)job[at + 4];
            }
            memcpy(objects_guard, dynamic_objects, sizeof(objects_guard));
            memcpy(position_guard, actor_positions, sizeof(position_guard));
            memcpy(orientation_guard, actor_orientation, sizeof(orientation_guard));
            memcpy(hands_guard, actor_items, sizeof(hands_guard));
            memcpy(types_guard, actor_types, sizeof(types_guard));
            memcpy(&pickup_guard, &item_pickup, sizeof(pickup_guard));
            count_guard = game_session.player_count;
            fields[0] = item_pickup_select((sint32)job[0], (sint32)job[1]);
            fields[1] = item_pickup.nearby[0];
            fields[2] = item_pickup.nearby[1];
            for (i = 0; i < 2; i++)
            {
                uint32 at = 3 + 6 * i;
                fields[at] = actor_positions[i].x.bits;
                fields[at + 1] = actor_positions[i].y.bits;
                fields[at + 2] = actor_positions[i].z.bits;
                fields[at + 3] = (uint16)actor_types[i];
                fields[at + 4] = (uint16)actor_orientation[i].yaw;
                fields[at + 5] = actor_items[i].bits;
            }
            for (i = 0; i < DYNAMIC_OBJECT_COUNT; i++)
            {
                uint32 at = 15 + 5 * i;
                const MODEL_OBJECT *model = dynamic_objects + i;
                fields[at] = model->kind;
                fields[at + 1] = model->pos.x.bits;
                fields[at + 2] = model->pos.y.bits;
                fields[at + 3] = model->pos.z.bits;
                fields[at + 4] = (uint16)model->physics.life;
            }
            pickup_guard.nearby[job[0]] = item_pickup.nearby[job[0]];
            if (memcmp(objects_guard, dynamic_objects, sizeof(objects_guard)) || memcmp(position_guard, actor_positions, sizeof(position_guard)) || memcmp(orientation_guard, actor_orientation, sizeof(orientation_guard)) || memcmp(hands_guard, actor_items, sizeof(hands_guard)) || memcmp(types_guard, actor_types, sizeof(types_guard)) || memcmp(&pickup_guard, &item_pickup, sizeof(pickup_guard)) || count_guard != game_session.player_count || distance_guard != dir_distance)
                abort();
            if (fwrite(fields, sizeof(fields), 1, out) != 1 || ++cases > header[3])
                result = 3;
        }
        if (ferror(in) || (!result && got) || cases != 61 || ferror(out))
            result = 3;
        if (fclose(in))
            result = 3;
        if (fclose(out))
            result = 3;
        if (!result)
            diagnostic_end();
        return result;
    }

    if (argc > 1 && !strcmp(argv[1], "--audit-sequence-render"))
    {
        static WORLD_OBJECTS world_guard;
        static MODEL_REFERENCE model_guard[MODEL_TYPE_COUNT];
        static TEXTURE_DESCRIPTORS descriptors_guard;
        static sint16 map_guard[TEXTURE_MAP_COUNT], depth_guard[DEPTH_TABLE_COUNT];
        FIXED_VEC3 positions_guard[ACTOR_COUNT];
        RESOURCE_FILE models = {0};
        CAMERA camera_guard;
        GAME_SESSION session_guard;
        RENDER_BUFFERS buffers_guard;
        uint32 header[4], seed[12], row[15], callbacks[WORLD_OBJECT_COUNT], gte[64], i, j, result = 0, draw_result = 0;
        uint32 count_guard = tex_count, descriptor_count_guard = tex_desc_count;
        uint8 *models_guard = NULL, *dram_guard = NULL;
        FILE *input = xport_fopen("../status/gameplay/sequence-render-models.bin", "rb");
        FILE *output = NULL;
        if (!input)
            return 4;
        if (!diagnostic_scene_models_read(input, &models))
            result = 4;
        fclose(input);
        input = xport_fopen("../status/gameplay/sequence-render-state.bin", "rb");
        if (!input)
            result = 4;
        if (input)
        {
            if (!result && !diagnostic_scene_render_state_read(input, gte))
                result = 4;
            fclose(input);
        }
        input = xport_fopen("../status/gameplay/sequence-render-world.bin", "rb");
        if (!input)
            result = 4;
        if (input)
        {
            if (!result && (fread(header, sizeof(header), 1, input) != 1 || header[0] != 0x35575246 || header[1] != 1 || header[2] != 65 || header[3] != 12 || fread(seed, sizeof(seed), 1, input) != 1 || seed[0] != 27 || seed[1] != 1 || seed[3] != header[2] || seed[7] > 65535 || seed[8] > 65535))
                result = 4;
            for (i = 0; !result && i < header[2]; i++)
            {
                MODEL_OBJECT *item = world_objects.object + i;
                if (fread(row, sizeof(row), 1, input) != 1 || row[0] >= MODEL_TYPE_COUNT || row[4] > 65535 || row[5] > 65535 || row[6] > 65535)
                {
                    result = 4;
                    break;
                }
                for (j = 8; j < 15; j++)
                    if (row[j] > 65535)
                        result = 4;
                const OBJECT_CALLBACK *target = NULL;
                if (row[7] == 0x80029f48)
                    target = &callback_object_draw_phase_yaw_variant;
                if (row[7] == 0x8003262c)
                    target = &callback_object_draw_yaw_flag_128;
                if (row[7] == 0x8002a048)
                    target = &callback_object_draw_camera_yaw_inner;
                if (row[7] == 0x8002ab40)
                    target = &callback_object_draw_wave;
                if (row[7] == 0x8005ab84)
                    target = &callback_object_draw_visible_player_facing;
                if (row[7] == 0x8002a97c)
                    target = &callback_object_draw_raised_spinning_pair;
                if (row[7] == 0x8005ae3c)
                    target = &callback_world_objects_draw_car;
                if (row[7] == 0x8002a0c4)
                    target = &callback_object_draw_rotated;
                if (row[0] == 55 ? row[7] != 0 : (!target || object_callbacks[OBJECT_CALLBACK_WORLD_DRAW][row[0]] != target || !model_refs[row[0]].header))
                    result = 4;
                if (result)
                    break;
                *item = (MODEL_OBJECT){0};
                item->kind = row[0];
                for (j = 0; j < 3; j++)
                {
                    item->pos.bits[j] = row[j + 1];
                    item->angles.bits[j] = (uint16)row[j + 4];
                }
                callbacks[i] = row[7];
                if (target == &callback_object_draw_wave)
                {
                    item->payload_kind = MODEL_PAYLOAD_WAVE;
                    item->wave.flags = (sint16)row[8];
                    item->wave.speed = (sint16)row[9];
                    item->wave.phase = (sint16)row[10];
                    item->wave.physics_actor_idx = (sint16)row[11];
                    item->wave.physics_bounce_count = (sint16)row[12];
                    item->wave.physics_vertical_speed = (sint16)row[13];
                    item->wave.physics_obstacle_flags = (uint16)row[14];
                }
                if (target == &callback_world_objects_draw_car)
                {
                    item->payload_kind = MODEL_PAYLOAD_OSCILLATION;
                    item->oscillation.roll_speed = (sint16)row[8];
                    item->oscillation.speed = (sint16)row[9];
                    item->oscillation.pitch_speed = (sint16)row[10];
                    item->oscillation.phase = (sint16)row[11];
                    item->oscillation.voice_plus_one = (sint16)row[12];
                    item->oscillation.sound_clock = (sint16)row[13];
                    item->oscillation.physics_obstacle_flags = (uint16)row[14];
                }
            }
            if (!result && fgetc(input) != EOF)
                result = 4;
            fclose(input);
        }
        if (!result)
        {
            models_guard = malloc(models.byte_len);
            dram_guard = malloc(sizeof(DRAM));
            if (!models_guard || !dram_guard)
                result = 4;
        }
        if (!result)
        {
            game_session.stage = seed[0];
            game_session.tick = seed[1];
            game_session.scene_phase = seed[2];
            world_objects.count = seed[3];
            for (j = 0; j < 3; j++)
            {
                camera.pos.bits[j] = seed[j + 4];
                actor_positions[0].bits[j] = seed[j + 9];
            }
            camera.angles.yaw_bits = (uint16)seed[7];
            camera.angles.pitch_bits = (uint16)seed[8];
            geometry_audit_restore_gte(gte);
            render_buffers.packet_addr = 0x801d0000;
            render_buffers.ot_addr = 0x801cb000;
            memset(psx_addr(0x801d0000, 65536), 0xa5, 65536);
            render_ot_init(0x801cb000);
            memcpy(models_guard, models.data, models.byte_len);
            memcpy(model_guard, model_refs, sizeof(model_guard));
            memcpy(&world_guard, &world_objects, sizeof(world_guard));
            memcpy(&camera_guard, &camera, sizeof(camera_guard));
            memcpy(&session_guard, &game_session, sizeof(session_guard));
            memcpy(&buffers_guard, &render_buffers, sizeof(buffers_guard));
            memcpy(positions_guard, actor_positions, sizeof(positions_guard));
            memcpy(&descriptors_guard, &tex_desc, sizeof(descriptors_guard));
            memcpy(map_guard, tex_map, sizeof(map_guard));
            memcpy(depth_guard, depth_table, sizeof(depth_guard));
            count_guard = tex_count;
            descriptor_count_guard = tex_desc_count;
            memcpy(dram_guard, DRAM, sizeof(DRAM));
            DIAGNOSTIC(printf("FF_RENDER_WORLD %p %u\n", (void *)world_objects.object, (uint32)sizeof(MODEL_OBJECT)));
            DIAGNOSTIC(printf("FF_RENDER_CALLBACK 80029f48 %p\n", (const void *)&callback_object_draw_phase_yaw_variant));
            DIAGNOSTIC(printf("FF_RENDER_CALLBACK 8003262c %p\n", (const void *)&callback_object_draw_yaw_flag_128));
            DIAGNOSTIC(printf("FF_RENDER_CALLBACK 8002a048 %p\n", (const void *)&callback_object_draw_camera_yaw_inner));
            DIAGNOSTIC(printf("FF_RENDER_CALLBACK 8002ab40 %p\n", (const void *)&callback_object_draw_wave));
            DIAGNOSTIC(printf("FF_RENDER_CALLBACK 8005ab84 %p\n", (const void *)&callback_object_draw_visible_player_facing));
            DIAGNOSTIC(printf("FF_RENDER_CALLBACK 8002a97c %p\n", (const void *)&callback_object_draw_raised_spinning_pair));
            DIAGNOSTIC(printf("FF_RENDER_CALLBACK 8005ae3c %p\n", (const void *)&callback_world_objects_draw_car));
            DIAGNOSTIC(printf("FF_RENDER_CALLBACK 8002a0c4 %p\n", (const void *)&callback_object_draw_rotated));
            draw_result = (uint32)world_objects_draw();
            for (i = 0; i < header[2]; i++)
            {
                if (callbacks[i] == 0x8005ab84)
                    world_guard.object[i].angles.yaw_bits = world_objects.object[i].angles.yaw_bits;
                if (callbacks[i] == 0x8002ab40)
                    world_guard.object[i].wave.phase = world_objects.object[i].wave.phase;
            }
            buffers_guard.packet_addr = render_buffers.packet_addr;
            memcpy(dram_guard + 0x1d0000, psx_addr(0x801d0000, 65536), 65536);
            memcpy(dram_guard + 0x1cb000, psx_addr(0x801cb000, 8192), 8192);
            if (render_buffers.packet_addr < 0x801d0000 || render_buffers.packet_addr > 0x801e0000 || memcmp(&world_guard, &world_objects, sizeof(world_guard)) || memcmp(model_guard, model_refs, sizeof(model_guard)) || memcmp(&camera_guard, &camera, sizeof(camera_guard)) || memcmp(&session_guard, &game_session, sizeof(session_guard)) || memcmp(&buffers_guard, &render_buffers, sizeof(buffers_guard)) || memcmp(positions_guard, actor_positions, sizeof(positions_guard)) || memcmp(&descriptors_guard, &tex_desc, sizeof(descriptors_guard)) || memcmp(map_guard, tex_map, sizeof(map_guard)) || memcmp(depth_guard, depth_table, sizeof(depth_guard)) || tex_count != count_guard || tex_desc_count != descriptor_count_guard || memcmp(models_guard, models.data, models.byte_len) || memcmp(dram_guard, DRAM, sizeof(DRAM)))
                result = 6;
        }
        if (!result)
        {
            output = xport_fopen("../status/gameplay/sequence-render-native.bin", "wb");
            if (!output)
                result = 5;
        }
        if (!result)
        {
            uint32 output_header[6] = {0x354e5246, 1, world_objects.count, 15, 65536, RENDER_OT_COUNT};
            uint32 fields[3] = {draw_result, render_buffers.packet_addr, render_buffers.ot_addr};
            if (fwrite(output_header, sizeof(output_header), 1, output) != 1 || fwrite(fields, sizeof(fields), 1, output) != 1)
                result = 5;
        }
        for (i = 0; !result && i < header[2]; i++)
        {
            const MODEL_OBJECT *item = world_objects.object + i;
            row[0] = item->kind;
            for (j = 0; j < 3; j++)
            {
                row[j + 1] = item->pos.bits[j];
                row[j + 4] = item->angles.bits[j];
            }
            row[7] = callbacks[i];
            memset(row + 8, 0, 7 * sizeof(uint32));
            if (callbacks[i] == 0x8002ab40)
            {
                row[8] = (uint16)item->wave.flags;
                row[9] = (uint16)item->wave.speed;
                row[10] = (uint16)item->wave.phase;
                row[11] = (uint16)item->wave.physics_actor_idx;
                row[12] = (uint16)item->wave.physics_bounce_count;
                row[13] = (uint16)item->wave.physics_vertical_speed;
                row[14] = item->wave.physics_obstacle_flags;
            }
            if (callbacks[i] == 0x8005ae3c)
            {
                row[8] = (uint16)item->oscillation.roll_speed;
                row[9] = (uint16)item->oscillation.speed;
                row[10] = (uint16)item->oscillation.pitch_speed;
                row[11] = (uint16)item->oscillation.phase;
                row[12] = (uint16)item->oscillation.voice_plus_one;
                row[13] = (uint16)item->oscillation.sound_clock;
                row[14] = item->oscillation.physics_obstacle_flags;
            }
            if (fwrite(row, sizeof(row), 1, output) != 1)
                result = 5;
        }
        if (!result && (!diagnostic_scene_render_state_write(output) || fwrite(psx_addr(0x801d0000, 65536), 65536, 1, output) != 1 || fwrite(psx_addr(0x801cb000, 8192), 8192, 1, output) != 1))
            result = 5;
        if (output && fclose(output) && !result)
            result = 5;
        if (models.data)
            memset(model_refs, 0, sizeof(model_refs));
        free(models.data);
        free(models_guard);
        free(dram_guard);
        if (!result)
            diagnostic_end();
        return (int)result;
    }
    if (argc > 1 && !strcmp(argv[1], "--audit-sequence-update"))
    {
        static COLLISION_REGIONS regions_guard;
        static WORLD_OBJECTS world_guard;
        static ACTOR_POSE pose_guard[ACTOR_COUNT];
        ACTOR_ANIMATION animation_guard[ACTOR_COUNT];
        ACTOR_HEALTH health_guard[ACTOR_COUNT];
        FIXED_VEC3 positions[ACTOR_COUNT];
        ACTOR_ORIENTATION orientation[ACTOR_COUNT];
        ACTOR_LIGHTING lighting[ACTOR_COUNT];
        sint16 types[ACTOR_COUNT];
        uint32 header[5], row[15], seed[8], trigger[REGION_KIND_COUNT], headers[ACTOR_COUNT], headers_guard[ACTOR_COUNT];
        uint32 callback[WORLD_OBJECT_COUNT], i, j, result = 0, active = 0, calls = 0;
        GAME_SESSION session_guard;
        DISPLAY_BUFFERS display_guard;
        const uint32 output_header[4] = {0x354e5355, 1, 12, 15};
        FILE *in = xport_fopen("../status/gameplay/sequence-update-jobs.bin", "rb");
        FILE *out = xport_fopen("../status/gameplay/sequence-update-native.bin", "wb");
        if (!in || !out)
        {
            if (in)
                fclose(in);
            if (out)
                fclose(out);
            return 4;
        }
        if (fread(header, sizeof(header), 1, in) != 1 || header[0] != 0x354a5355 || header[1] != 1 || header[2] != 21 || header[3] != 1 || header[4] != ACTOR_COUNT || fread(trigger, sizeof(trigger), 1, in) != 1)
            result = 4;
        for (i = 0; !result && i < REGION_KIND_COUNT; i++)
        {
            static const uint32 original_targets[REGION_TRIGGER_COUNT] = {0x8003060c, 0x800306d4, 0x800307ac, 0x80030874, 0x80030944, 0x80030a14, 0x80030ad4, 0x80030b94, 0x80030c50, 0x80030ca8, 0x80030d04, 0x80030d60, 0x80030dbc, 0x80047174};
            REGION_TRIGGER_REFERENCE target = NULL;
            for (j = 0; j < REGION_TRIGGER_COUNT; j++)
                if (trigger[i] == original_targets[j])
                    target = region_trigger_catalog + j;
            if ((trigger[i] && !target) || region_triggers[i] != target)
                result = 4;
        }
        if (!result)
            col_regions.count = header[2];
        for (i = 0; !result && i < header[2]; i++)
        {
            COLLISION_REGION *region = col_regions.data + i;
            if (fread(row, 12 * sizeof(uint32), 1, in) != 1)
            {
                result = 4;
                break;
            }
            for (j = 0; j < 10; j++)
                if (row[j] > 65535)
                    result = 4;
            if (row[10] > 1 || (row[10] && row[11] >= WORLD_OBJECT_COUNT) || (!row[10] && row[11]) || row[4] >= REGION_KIND_COUNT || (trigger[row[4]] && row[5]))
                result = 4;
            if (result)
                break;
            for (j = 0; j < 4; j++)
                region->geometry.coord[j] = (sint16)row[j];
            region->trigger_kind = (sint16)row[4];
            region->shape = (sint16)row[5];
            region->col_kind = (sint16)row[6];
            region->model_type = (sint16)row[7];
            region->hits = (sint16)row[8];
            region->alternate_attack = (sint16)row[9];
            region->object.present = (uint8)row[10];
            region->object.idx = row[11];
        }
        for (i = 0; !result && i < ACTOR_COUNT; i++)
        {
            if (fread(row, 7 * sizeof(uint32), 1, in) != 1 || row[3] > 65535 || (row[4] != 0xffffffff && row[4] >= COLLISION_REGION_COUNT) || row[5] > 65535 || row[6] > 65535)
            {
                result = 4;
                break;
            }
            for (j = 0; j < 3; j++)
                actor_positions[i].bits[j] = row[j];
            actor_orientation[i].yaw = (sint16)row[3];
            col_regions.actor[i].region = row[4] == 0xffffffff ? NULL : col_regions.data + row[4];
            col_regions.actor[i].flags = (uint16)row[5];
            col_regions.actor[i].heading = (sint16)row[6];
        }
        if (!result && fread(row, 3 * sizeof(uint32), 1, in) != 1)
            result = 4;
        if (!result)
        {
            col_regions.local_x = row[0];
            col_regions.local_z = row[1];
            col_regions.side = row[2];
            if (fread(seed, sizeof(seed), 1, in) != 1 || seed[0] != 27 || seed[1] != 1 || seed[2] > ACTOR_COUNT || seed[7] > WORLD_OBJECT_COUNT || fread(headers, sizeof(headers), 1, in) != 1)
                result = 4;
        }
        for (i = 0; !result && i < ACTOR_COUNT; i++)
        {
            if (fread(row, 8 * sizeof(uint32), 1, in) != 1 || row[0] > 65535 || row[1] > 65535 || row[3] != 0xffffffff || row[5] > ACTOR_ANIMATION_SOURCE_PROCEDURAL || (row[5] == ACTOR_ANIMATION_SOURCE_STREAM && (row[6] >= ACTOR_COUNT || row[7] < 1 || row[7] > 3)) || (row[5] != ACTOR_ANIMATION_SOURCE_STREAM && (row[6] != 0xffffffff || row[7])))
            {
                result = 4;
                break;
            }
            actor_types[i] = (sint16)row[0];
            active += actor_types[i] != ACTOR_TYPE_NONE;
            actor_health[i].stun = (sint16)row[1];
            actor_animations[i].id = row[2];
            actor_animations[i].frame = row[3];
            actor_poses[i].sway_yaw = row[4];
            actor_animations[i].src = (ACTOR_ANIMATION_SOURCE){0};
            actor_animations[i].src.kind = (ACTOR_ANIMATION_SOURCE_KIND)row[5];
            if (row[5] == ACTOR_ANIMATION_SOURCE_STREAM)
            {
                actor_animations[i].src.stream.header = (const ANIMATION_STREAM_HEADER *)(headers + row[6]);
                actor_animations[i].src.stream.byte_len = sizeof(uint32);
                actor_animations[i].src.stream.domain = (ANIMATION_STREAM_DOMAIN)row[7];
            }
        }
        for (i = 0; !result && i < seed[7]; i++)
        {
            MODEL_OBJECT *item = world_objects.object + i;
            if (fread(row, sizeof(row), 1, in) != 1 || row[0] >= MODEL_TYPE_COUNT || row[4] > 65535 || row[5] > 65535 || row[6] > 65535)
            {
                result = 4;
                break;
            }
            for (j = 8; j < 15; j++)
                if (row[j] > 65535)
                    result = 4;
            const OBJECT_CALLBACK *target = row[7] == 0 ? NULL : row[7] == 0x8002a974 ? &callback_object_ignore_callback0 : row[7] == 0x8002cde0 ? &callback_gate_shake : row[7] == 0x8002da3c ? &callback_object_update_oscillation : NULL;
            if ((row[7] && !target) || object_callbacks[OBJECT_CALLBACK_UPDATE][row[0]] != target || (target == &callback_object_update_oscillation && ((sint16)row[13] == 1 || (row[0] == 720 && i + 1 >= seed[7]))))
                result = 4;
            if (result)
                break;
            *item = (MODEL_OBJECT){0};
            item->kind = row[0];
            for (j = 0; j < 3; j++)
            {
                item->pos.bits[j] = row[1 + j];
                item->angles.bits[j] = (uint16)row[4 + j];
            }
            callback[i] = row[7];
            calls += row[7] != 0;
            if (target == &callback_gate_shake)
            {
                item->payload_kind = MODEL_PAYLOAD_SHAKE;
                item->shake.ticks = (sint16)row[8];
                item->shake.speed = (sint16)row[9];
                item->shake.oz = (sint16)row[10];
                item->shake.ox = (sint16)row[11];
                item->shake.physics_bounce_count = (sint16)row[12];
                item->shake.physics_vertical_speed = (sint16)row[13];
                item->shake.physics_obstacle_flags = (uint16)row[14];
            }
            if (target == &callback_object_update_oscillation)
            {
                item->payload_kind = MODEL_PAYLOAD_OSCILLATION;
                item->oscillation.roll_speed = (sint16)row[8];
                item->oscillation.speed = (sint16)row[9];
                item->oscillation.pitch_speed = (sint16)row[10];
                item->oscillation.phase = (sint16)row[11];
                item->oscillation.voice_plus_one = (sint16)row[12];
                item->oscillation.sound_clock = (sint16)row[13];
                item->oscillation.physics_obstacle_flags = (uint16)row[14];
            }
        }
        if (!result && (active != 4 || calls != 5 || fgetc(in) != EOF))
            result = 4;
        if (!result)
        {
            game_session.stage = seed[0];
            game_session.tick = seed[1];
            game_session.player_count = seed[2];
            game_session.stage0_ticks = seed[3];
            game_session.scene_phase = seed[4];
            disp_buffers.flash_ticks = seed[5];
            disp_buffers.shake_ticks = seed[6];
            world_objects.count = seed[7];
            memcpy(&regions_guard, &col_regions, sizeof(regions_guard));
            memcpy(&world_guard, &world_objects, sizeof(world_guard));
            memcpy(pose_guard, actor_poses, sizeof(pose_guard));
            memcpy(animation_guard, actor_animations, sizeof(animation_guard));
            memcpy(health_guard, actor_health, sizeof(health_guard));
            memcpy(positions, actor_positions, sizeof(positions));
            memcpy(orientation, actor_orientation, sizeof(orientation));
            memcpy(lighting, actor_lighting, sizeof(lighting));
            memcpy(types, actor_types, sizeof(types));
            memcpy(&session_guard, &game_session, sizeof(session_guard));
            memcpy(&display_guard, &disp_buffers, sizeof(display_guard));
            memcpy(headers_guard, headers, sizeof(headers));
            DIAGNOSTIC(printf("FF_UPDATE_WORLD %p %u\n", (void *)world_objects.object, (uint32)sizeof(MODEL_OBJECT)));
            DIAGNOSTIC(printf("FF_UPDATE_CALLBACK 8002a974 %p\n", (const void *)&callback_object_ignore_callback0));
            DIAGNOSTIC(printf("FF_UPDATE_CALLBACK 8002cde0 %p\n", (const void *)&callback_gate_shake));
            DIAGNOSTIC(printf("FF_UPDATE_CALLBACK 8002da3c %p\n", (const void *)&callback_object_update_oscillation));
            stage_update_world();
            display_guard.flash_ticks = disp_buffers.flash_ticks;
            display_guard.shake_ticks = disp_buffers.shake_ticks;
            for (i = 0; i < ACTOR_COUNT; i++)
            {
                if (i < seed[2])
                    pose_guard[i].sway_yaw = actor_poses[i].sway_yaw;
                if (actor_types[i] != ACTOR_TYPE_NONE)
                {
                    animation_guard[i].frame = actor_animations[i].frame;
                    health_guard[i].stun = actor_health[i].stun;
                    memcpy(&regions_guard.actor[i], &col_regions.actor[i], sizeof(ACTOR_REGION_CONTACT));
                }
            }
            for (i = 0; i < seed[7]; i++)
            {
                MODEL_OBJECT *expected = world_guard.object + i;
                const MODEL_OBJECT *item = world_objects.object + i;
                if (callback[i] == 0x8002cde0)
                {
                    expected->pos.x = item->pos.x;
                    expected->pos.z = item->pos.z;
                    expected->shake.ticks = item->shake.ticks;
                    expected->shake.ox = item->shake.ox;
                    expected->shake.oz = item->shake.oz;
                }
                if (callback[i] == 0x8002da3c)
                {
                    expected->angles.pitch = item->angles.pitch;
                    expected->angles.roll = item->angles.roll;
                    expected->oscillation.pitch_speed = item->oscillation.pitch_speed;
                    expected->oscillation.roll_speed = item->oscillation.roll_speed;
                    expected->oscillation.sound_clock = item->oscillation.sound_clock;
                    if (item->kind == 720)
                    {
                        expected->angles.yaw = item->angles.yaw;
                        world_guard.object[i + 1].angles.yaw = world_objects.object[i + 1].angles.yaw;
                    }
                }
            }
            if (memcmp(&regions_guard, &col_regions, sizeof(regions_guard)) || memcmp(&world_guard, &world_objects, sizeof(world_guard)) || memcmp(pose_guard, actor_poses, sizeof(pose_guard)) || memcmp(animation_guard, actor_animations, sizeof(animation_guard)) || memcmp(health_guard, actor_health, sizeof(health_guard)) || memcmp(positions, actor_positions, sizeof(positions)) || memcmp(orientation, actor_orientation, sizeof(orientation)) || memcmp(lighting, actor_lighting, sizeof(lighting)) || memcmp(types, actor_types, sizeof(types)) || memcmp(&session_guard, &game_session, sizeof(session_guard)) || memcmp(&display_guard, &disp_buffers, sizeof(display_guard)) || memcmp(headers_guard, headers, sizeof(headers)))
                result = 6;
        }
        if (!result)
        {
            uint32 scalars[11] = {game_session.stage, game_session.tick, game_session.player_count, game_session.stage0_ticks, game_session.scene_phase, disp_buffers.flash_ticks, disp_buffers.shake_ticks, world_objects.count, col_regions.local_x, col_regions.local_z, col_regions.side};
            if (fwrite(output_header, sizeof(output_header), 1, out) != 1 || fwrite(scalars, sizeof(scalars), 1, out) != 1)
                result = 5;
        }
        for (i = 0; !result && i < ACTOR_COUNT; i++)
        {
            row[0] = (uint16)actor_types[i];
            row[1] = (uint16)actor_health[i].stun;
            row[2] = actor_animations[i].id;
            row[3] = actor_animations[i].frame;
            row[4] = actor_poses[i].sway_yaw;
            for (j = 0; j < 3; j++)
                row[5 + j] = actor_positions[i].bits[j];
            row[8] = (uint16)actor_orientation[i].yaw;
            row[9] = 0xffffffff;
            if (col_regions.actor[i].region)
            {
                for (j = 0; j < COLLISION_REGION_COUNT; j++)
                    if (col_regions.actor[i].region == col_regions.data + j)
                        row[9] = j;
                if (row[9] == 0xffffffff)
                    result = 6;
            }
            row[10] = col_regions.actor[i].flags;
            row[11] = (uint16)col_regions.actor[i].heading;
            if (!result && fwrite(row, 12 * sizeof(uint32), 1, out) != 1)
                result = 5;
        }
        for (i = 0; !result && i < seed[7]; i++)
        {
            const MODEL_OBJECT *item = world_objects.object + i;
            row[0] = item->kind;
            for (j = 0; j < 3; j++)
            {
                row[1 + j] = item->pos.bits[j];
                row[4 + j] = item->angles.bits[j];
            }
            row[7] = callback[i];
            memset(row + 8, 0, 7 * sizeof(uint32));
            if (callback[i] == 0x8002cde0)
            {
                row[8] = (uint16)item->shake.ticks;
                row[9] = (uint16)item->shake.speed;
                row[10] = (uint16)item->shake.oz;
                row[11] = (uint16)item->shake.ox;
                row[12] = (uint16)item->shake.physics_bounce_count;
                row[13] = (uint16)item->shake.physics_vertical_speed;
                row[14] = item->shake.physics_obstacle_flags;
            }
            if (callback[i] == 0x8002da3c)
            {
                row[8] = (uint16)item->oscillation.roll_speed;
                row[9] = (uint16)item->oscillation.speed;
                row[10] = (uint16)item->oscillation.pitch_speed;
                row[11] = (uint16)item->oscillation.phase;
                row[12] = (uint16)item->oscillation.voice_plus_one;
                row[13] = (uint16)item->oscillation.sound_clock;
                row[14] = item->oscillation.physics_obstacle_flags;
            }
            if (fwrite(row, sizeof(row), 1, out) != 1)
                result = 5;
        }
        for (i = 0; i < ACTOR_COUNT; i++)
            actor_animations[i].src = (ACTOR_ANIMATION_SOURCE){0};
        fclose(in);
        if (fclose(out) && !result)
            result = 5;
        if (!result)
            diagnostic_end();
        return (int)result;
    }
    if (argc > 1 && !strcmp(argv[1], "--audit-regions"))
    {
        static COLLISION_REGIONS regions_guard;
        static WORLD_OBJECTS world_guard;
        FIXED_VEC3 positions[ACTOR_COUNT];
        ACTOR_ORIENTATION orientation[ACTOR_COUNT];
        ACTOR_LIGHTING lighting[ACTOR_COUNT];
        ACTOR_REGION_CONTACT contacts[ACTOR_COUNT];
        uint32 header[5], row[12], job[4], trigger[REGION_KIND_COUNT], i, j, result = 0;
        const uint32 output_header[4] = {0x344e5246, 1, 12, 240};
        FILE *in = xport_fopen("../status/gameplay/region-jobs.bin", "rb");
        FILE *out = xport_fopen("../status/gameplay/region-native.bin", "wb");
        if (!in || !out)
        {
            if (in)
                fclose(in);
            if (out)
                fclose(out);
            return 4;
        }
        /* Use the same trigger defaults as normal game startup */
        game_tables_init_models();
        if (fread(header, sizeof(header), 1, in) != 1 || header[0] != 0x344a5246 || header[1] != 1 || header[2] != 21 || header[3] != 240 || header[4] != ACTOR_COUNT || fread(trigger, sizeof(trigger), 1, in) != 1)
            result = 4;
        for (i = 0; !result && i < REGION_KIND_COUNT; i++)
        {
            static const uint32 original_targets[REGION_TRIGGER_COUNT] = {0x8003060c, 0x800306d4, 0x800307ac, 0x80030874, 0x80030944, 0x80030a14, 0x80030ad4, 0x80030b94, 0x80030c50, 0x80030ca8, 0x80030d04, 0x80030d60, 0x80030dbc, 0x80047174};
            REGION_TRIGGER_REFERENCE target = NULL;
            for (j = 0; j < REGION_TRIGGER_COUNT; j++)
                if (trigger[i] == original_targets[j])
                    target = region_trigger_catalog + j;
            if ((trigger[i] && !target) || region_triggers[i] != target)
                result = 4;
        }
        if (!result)
            col_regions.count = header[2];
        for (i = 0; !result && i < header[2]; i++)
        {
            COLLISION_REGION *region = col_regions.data + i;
            if (fread(row, sizeof(row), 1, in) != 1)
            {
                result = 4;
                break;
            }
            for (j = 0; j < 10; j++)
                if (row[j] > 65535)
                    result = 4;
            if (row[10] > 1 || (row[10] && row[11] >= WORLD_OBJECT_COUNT) || (!row[10] && row[11]) || row[4] >= REGION_KIND_COUNT || (trigger[row[4]] && row[5]))
                result = 4;
            if (result)
                break;
            for (j = 0; j < 4; j++)
                region->geometry.coord[j] = (sint16)row[j];
            region->trigger_kind = (sint16)row[4];
            region->shape = (sint16)row[5];
            region->col_kind = (sint16)row[6];
            region->model_type = (sint16)row[7];
            region->hits = (sint16)row[8];
            region->alternate_attack = (sint16)row[9];
            region->object.present = (uint8)row[10];
            region->object.idx = row[11];
        }
        for (i = 0; !result && i < ACTOR_COUNT; i++)
        {
            if (fread(row, 7 * sizeof(uint32), 1, in) != 1 || row[3] > 65535 || (row[4] != 0xffffffff && row[4] >= COLLISION_REGION_COUNT) || row[5] > 65535 || row[6] > 65535)
            {
                result = 4;
                break;
            }
            for (j = 0; j < 3; j++)
                actor_positions[i].bits[j] = row[j];
            actor_orientation[i].yaw = (sint16)row[3];
            col_regions.actor[i].region = row[4] == 0xffffffff ? NULL : col_regions.data + row[4];
            col_regions.actor[i].flags = (uint16)row[5];
            col_regions.actor[i].heading = (sint16)row[6];
        }
        if (!result && fread(row, 3 * sizeof(uint32), 1, in) != 1)
            result = 4;
        if (!result)
        {
            col_regions.local_x = row[0];
            col_regions.local_z = row[1];
            col_regions.side = row[2];
            memcpy(positions, actor_positions, sizeof(positions));
            memcpy(orientation, actor_orientation, sizeof(orientation));
            memcpy(lighting, actor_lighting, sizeof(lighting));
            memcpy(contacts, col_regions.actor, sizeof(contacts));
            memcpy(&regions_guard, &col_regions, sizeof(regions_guard));
            memcpy(&world_guard, &world_objects, sizeof(world_guard));
            if (fwrite(output_header, sizeof(output_header), 1, out) != 1)
                result = 5;
        }
        for (i = 0; !result && i < header[3]; i++)
        {
            if (fread(job, sizeof(job), 1, in) != 1 || job[3] >= 4 || job[2] > 65535)
            {
                result = 4;
                break;
            }
            memcpy(actor_positions, positions, sizeof(positions));
            memcpy(actor_orientation, orientation, sizeof(orientation));
            memcpy(col_regions.actor, contacts, sizeof(contacts));
            actor_positions[job[3]].x.whole = (sint16)job[0];
            actor_positions[job[3]].z.whole = (sint16)job[1];
            actor_orientation[job[3]].yaw = (sint16)job[2];
            memcpy(&regions_guard, &col_regions, sizeof(regions_guard));
            row[0] = regions_apply_actor((sint32)job[3]);
            for (j = 0; j < ACTOR_COUNT; j++)
            {
                FIXED_VEC3 expected_pos = positions[j];
                ACTOR_ORIENTATION expected_orientation = orientation[j];
                if (j == job[3])
                {
                    expected_pos.x.whole = (sint16)job[0];
                    expected_pos.z.whole = (sint16)job[1];
                    expected_orientation.yaw = (sint16)job[2];
                }
                if (memcmp(&actor_positions[j], &expected_pos, sizeof(expected_pos)) || memcmp(&actor_orientation[j], &expected_orientation, sizeof(expected_orientation)))
                    result = 6;
            }
            memcpy(&regions_guard.actor[job[3]], &col_regions.actor[job[3]], sizeof(ACTOR_REGION_CONTACT));
            if (memcmp(&regions_guard, &col_regions, sizeof(regions_guard)) || memcmp(&world_guard, &world_objects, sizeof(world_guard)) || memcmp(lighting, actor_lighting, sizeof(lighting)))
                result = 6;
            row[1] = job[3];
            for (j = 0; j < 3; j++)
                row[2 + j] = actor_positions[job[3]].bits[j];
            row[5] = (uint16)actor_orientation[job[3]].yaw;
            row[6] = 0xffffffff;
            if (col_regions.actor[job[3]].region)
            {
                for (j = 0; j < COLLISION_REGION_COUNT; j++)
                    if (col_regions.actor[job[3]].region == col_regions.data + j)
                        row[6] = j;
                if (row[6] == 0xffffffff)
                    result = 6;
            }
            row[7] = col_regions.actor[job[3]].flags;
            row[8] = (uint16)col_regions.actor[job[3]].heading;
            row[9] = col_regions.local_x;
            row[10] = col_regions.local_z;
            row[11] = col_regions.side;
            if (!result && fwrite(row, sizeof(row), 1, out) != 1)
                result = 5;
        }
        if (!result && fgetc(in) != EOF)
            result = 4;
        fclose(in);
        if (fclose(out) && !result)
            result = 5;
        if (!result)
            diagnostic_end();
        return (int)result;
    }
    if (argc > 1 && !strcmp(argv[1], "--audit-sequence-frames"))
    {
        static WORLD_OBJECTS world_guard;
        static const sint32 edge_frames[4] = {549, 550, 551, 0x7fffffff};
        const uint32 output_header[4] = {0x344e4646, 1, 16, 554};
        RESOURCE_FILE archive = {0};
        uint8 *readonly_guard = NULL;
        uint32 header[6], seed[19], values[16], i;
        CAMERA camera_guard;
        GAME_SESSION session_guard;
        SCENE_PLAYBACK scene_guard;
        FILE *in = xport_fopen("../status/gameplay/sequence-frames-jobs.bin", "rb");
        FILE *out = xport_fopen("../status/gameplay/sequence-frames-native.bin", "wb");
        int result = 0;
        if (!in || !out)
        {
            if (in)
                fclose(in);
            if (out)
                fclose(out);
            return 4;
        }
        if (fread(header, sizeof(header), 1, in) != 1 || header[0] != 0x344a4646 || header[1] != 1 || !header[2] || header[2] > RESOURCE_STORAGE_CAPACITY || header[3] != 550 || header[4] != 4 || header[5] != 19 || fread(seed, sizeof(seed), 1, in) != 1)
            result = 4;
        if (!result && (seed[0] > header[2] || seed[2] > header[2] || (seed[0] & 1) || (seed[2] & 1) || seed[1] < 6602 || seed[3] < 6602 || seed[1] > header[2] - seed[0] || seed[3] > header[2] - seed[2] || seed[4] != 550 || seed[9] > 65535 || seed[10] > 65535 || seed[16] > 65535 || seed[17] > 65535 || seed[18] > 65535))
            result = 4;
        if (!result)
        {
            archive.data = malloc(header[2]);
            readonly_guard = malloc(header[2]);
            archive.capacity = archive.byte_len = header[2];
            if (!archive.data || !readonly_guard || fread(archive.data, header[2], 1, in) != 1 || fgetc(in) != EOF)
                result = 4;
        }
        if (!result)
        {
            memcpy(readonly_guard, archive.data, archive.byte_len);
            scene_playback.stream[0] = (SCENE_STREAM){archive.data + seed[0], seed[1]};
            scene_playback.stream[1] = (SCENE_STREAM){archive.data + seed[2], seed[3]};
            scene_playback.camera_frames = seed[4];
            scene_playback.end_tick = seed[5];
            camera.pos.x.bits = seed[6];
            camera.pos.y.bits = seed[7];
            camera.pos.z.bits = seed[8];
            camera.angles.yaw_bits = (uint16)seed[9];
            camera.angles.pitch_bits = (uint16)seed[10];
            world_objects.count = seed[11];
            world_objects.object[64].kind = seed[12];
            world_objects.object[64].pos.x.bits = seed[13];
            world_objects.object[64].pos.y.bits = seed[14];
            world_objects.object[64].pos.z.bits = seed[15];
            world_objects.object[64].angles.yaw_bits = (uint16)seed[16];
            world_objects.object[64].angles.pitch_bits = (uint16)seed[17];
            world_objects.object[64].angles.roll_bits = (uint16)seed[18];
            game_session.tick = 0;
            game_session.stage = 27;
            if (fwrite(output_header, sizeof(output_header), 1, out) != 1)
                result = 4;
            for (i = 0; !result && i < 554; i++)
            {
                memcpy(&world_guard, &world_objects, sizeof(world_guard));
                memcpy(&camera_guard, &camera, sizeof(camera_guard));
                memcpy(&session_guard, &game_session, sizeof(session_guard));
                memcpy(&scene_guard, &scene_playback, sizeof(scene_guard));
                values[0] = i < 550 ? scene_step() : (uint32)scene_apply_camera_frame(edge_frames[i - 550]);
                camera_guard.pos = camera.pos;
                camera_guard.angles = camera.angles;
                if (i < 550)
                {
                    session_guard.tick = game_session.tick;
                    world_guard.count = world_objects.count;
                    world_guard.object[64].kind = world_objects.object[64].kind;
                    world_guard.object[64].pos = world_objects.object[64].pos;
                    world_guard.object[64].angles = world_objects.object[64].angles;
                }
                if (memcmp(&world_guard, &world_objects, sizeof(world_guard)) || memcmp(&camera_guard, &camera, sizeof(camera_guard)) || memcmp(&session_guard, &game_session, sizeof(session_guard)) || memcmp(&scene_guard, &scene_playback, sizeof(scene_guard)) || memcmp(readonly_guard, archive.data, archive.byte_len))
                    abort();
                values[1] = game_session.tick;
                values[2] = game_session.stage;
                values[3] = world_objects.count;
                values[4] = camera.pos.x.bits;
                values[5] = camera.pos.y.bits;
                values[6] = camera.pos.z.bits;
                values[7] = camera.angles.yaw_bits;
                values[8] = camera.angles.pitch_bits;
                values[9] = world_objects.object[64].kind;
                values[10] = world_objects.object[64].pos.x.bits;
                values[11] = world_objects.object[64].pos.y.bits;
                values[12] = world_objects.object[64].pos.z.bits;
                values[13] = world_objects.object[64].angles.yaw_bits;
                values[14] = world_objects.object[64].angles.pitch_bits;
                values[15] = world_objects.object[64].angles.roll_bits;
                if (fwrite(values, sizeof(values), 1, out) != 1)
                    result = 4;
            }
        }
        scene_playback.stream[0] = (SCENE_STREAM){0};
        scene_playback.stream[1] = (SCENE_STREAM){0};
        free(readonly_guard);
        free(archive.data);
        if (fclose(in))
            result = 4;
        if (fclose(out))
            result = 4;
        diagnostic_end();
        return result;
    }
    if (argc > 1 && (!strcmp(argv[1], "--audit-player-resources") || !strcmp(argv[1], "--audit-mace-player-resources") || !strcmp(argv[1], "--audit-smasher-player-resources")))
    {
        static uint8 storage_guard[RESOURCE_STORAGE_CAPACITY];
        static MODEL_REFERENCE refs_guard[MODEL_TYPE_COUNT];
        static ANIMATION_BANK banks_guard[ANIMATION_BANK_COUNT];
        const ANIMATION_BANK_ID banks[4] = {ANIMATION_BANK_HAWK, ANIMATION_BANK_MACE, ANIMATION_BANK_SMASHER, ANIMATION_BANK_ALANA};
        uint32 fixture = !strcmp(argv[1], "--audit-player-resources") ? 0 : (!strcmp(argv[1], "--audit-mace-player-resources") ? 1 : 2);
        const char *input_paths[3] = {"../status/gameplay/player-resources-jobs.bin", "../status/gameplay/mace-player-resources-jobs.bin", "../status/gameplay/smasher-player-resources-jobs.bin"};
        const char *output_paths[3] = {"../status/gameplay/player-resources-native.bin", "../status/gameplay/mace-player-resources-native.bin", "../status/gameplay/smasher-player-resources-native.bin"};
        FILE *in = xport_fopen(input_paths[fixture], "rb"), *out = xport_fopen(output_paths[fixture], "wb");
        uint32 header[6], seed[16], i, j, value, output_header[4], fields[20], model_fields[7];
        uint32 heights_guard[MODEL_TYPE_COUNT];
        sint16 starts[TEXTURE_GROUP_COUNT], map_guard[TEXTURE_MAP_COUNT], types_guard[ACTOR_COUNT];
        TEXTURE_GROUP groups_guard[TEXTURE_GROUP_COUNT];
        ANIMATION_BANK_ID type_guard[ANIMATION_TYPE_COUNT];
        const char *files_guard[RESOURCE_FILE_COUNT];
        RESOURCE_ARENAS arenas_guard;
        GAME_SESSION session_guard;
        LOADING loading_guard;
        DISPLAY_BUFFERS display_guard;
        uintptr_t scratch_offset, persistent_offset, archive_offset, copy_bytes;
        int result = 0;
        if (!in || !out)
        {
            if (in)
                fclose(in);
            if (out)
                fclose(out);
            return 4;
        }
        if (fread(header, sizeof(header), 1, in) != 1 || header[0] != 0x344a5046 || header[1] != 1 || header[2] != 16 || header[3] != 4 || header[4] != ANIMATION_COUNT || header[5] != sizeof(seed) + sizeof(starts) + sizeof(tex_map) + sizeof(model_height) + 4 * ANIMATION_COUNT * 16 + sizeof(animation_flags) || fread(seed, sizeof(seed), 1, in) != 1 || fread(starts, sizeof(starts), 1, in) != 1 || fread(tex_map, sizeof(tex_map), 1, in) != 1 || fread(model_height, sizeof(model_height), 1, in) != 1)
            result = 4;
        /* These three historical fixtures cover one selected player; preserve the original gate */
        if (!result && (seed[0] != fixture || seed[1] > 65535 || seed[2] == 2 || seed[15] != 1 || seed[9] > 65535 || seed[10] > 65535 || seed[3] > RESOURCE_STORAGE_CAPACITY - 4 || seed[6] > RESOURCE_STORAGE_CAPACITY - 4 || (seed[3] & 3) || (seed[6] & 3) || seed[5] != RESOURCE_STORAGE_CAPACITY - seed[3] || seed[8] != RESOURCE_STORAGE_CAPACITY - seed[6] || (seed[4] & 0x1fffffff) != RESOURCE_STORAGE_ORIGIN + seed[3] || (seed[7] & 0x1fffffff) != RESOURCE_STORAGE_ORIGIN + seed[6] || ((seed[4] & 0xe0000000) != 0 && (seed[4] & 0xe0000000) != 0x80000000 && (seed[4] & 0xe0000000) != 0xa0000000) || ((seed[7] & 0xe0000000) != 0 && (seed[7] & 0xe0000000) != 0x80000000 && (seed[7] & 0xe0000000) != 0xa0000000)))
            result = 4;
        if (!result)
        {
            /* Only serialized resource storage is initialized; no original game RAM is imported */
            memset(resource_storage.data, 0xa5, sizeof(resource_storage.data));
            memset(model_refs, 0, sizeof(model_refs));
        }
        for (i = 0; !result && i < 4; i++)
            for (j = 0; !result && j < ANIMATION_COUNT; j++)
            {
                uint32 words[4];
                ACTOR_ANIMATION_SOURCE *stream = animation_banks[banks[i]].stream + j;
                if (fread(words, sizeof(words), 1, in) != 1)
                    result = 4;
                else if (words[0] == ACTOR_ANIMATION_SOURCE_NONE)
                {
                    if (words[1] != 0xffffffff || words[2] || words[3])
                        result = 4;
                    else
                        *stream = (ACTOR_ANIMATION_SOURCE){0};
                }
                else if (words[0] == ACTOR_ANIMATION_SOURCE_STREAM && words[1] <= RESOURCE_STORAGE_CAPACITY - 4 && !(words[1] & 3) && words[2] == RESOURCE_STORAGE_CAPACITY - words[1] && words[3] >= ANIMATION_STREAM_PHYSICAL && words[3] < ANIMATION_STREAM_DOMAIN_COUNT)
                    *stream = (ACTOR_ANIMATION_SOURCE){ACTOR_ANIMATION_SOURCE_STREAM, {(const ANIMATION_STREAM_HEADER *)(resource_storage.data + words[1]), words[2], (ANIMATION_STREAM_DOMAIN)words[3]}};
                else
                    result = 4;
            }
        if (!result && (fread(animation_flags, sizeof(animation_flags), 1, in) != 1 || fgetc(in) != EOF))
            result = 4;
        if (!result)
        {
            actor_types[0] = (sint16)(uint16)seed[0];
            actor_types[1] = (sint16)(uint16)seed[1];
            game_session.player_count = seed[2];
            resource_arenas.scratch = (RESOURCE_CURSOR){resource_storage.data + seed[3], seed[5], seed[4]};
            resource_arenas.persistent = (RESOURCE_CURSOR){resource_storage.data + seed[6], seed[8], seed[7]};
            loading = (LOADING){(sint16)(uint16)seed[9], (sint16)(uint16)seed[10], seed[11]};
            disp_buffers.frame = seed[12];
            resource_io = (RESOURCE_IO){seed[13], seed[14]};
            for (i = 0; i < TEXTURE_GROUP_COUNT; i++)
                tex_grps[i].first_tex = starts[i];
            for (i = 0; i < 4; i++)
                animation_type_banks[i] = banks[i];
            resource_file_names[76 + fixture] = resource_paths[73 + fixture];
            memcpy(storage_guard, resource_storage.data, sizeof(storage_guard));
            memcpy(refs_guard, model_refs, sizeof(refs_guard));
            memcpy(banks_guard, animation_banks, sizeof(banks_guard));
            memcpy(heights_guard, model_height, sizeof(heights_guard));
            memcpy(types_guard, actor_types, sizeof(types_guard));
            memcpy(groups_guard, tex_grps, sizeof(groups_guard));
            memcpy(map_guard, tex_map, sizeof(map_guard));
            memcpy(type_guard, animation_type_banks, sizeof(type_guard));
            for (uint32 file_idx = 0; file_idx < RESOURCE_FILE_COUNT; file_idx++)
                files_guard[file_idx] = resource_file_names[file_idx];
            memcpy(&arenas_guard, &resource_arenas, sizeof(arenas_guard));
            memcpy(&session_guard, &game_session, sizeof(session_guard));
            memcpy(&loading_guard, &loading, sizeof(loading_guard));
            memcpy(&display_guard, &disp_buffers, sizeof(display_guard));
            value = character_load_players();
            if ((uintptr_t)resource_arenas.scratch.data < (uintptr_t)resource_storage.data || (uintptr_t)resource_arenas.persistent.data < (uintptr_t)resource_storage.data || (uintptr_t)resource_archive.dir < (uintptr_t)resource_storage.data)
                abort();
            scratch_offset = (uintptr_t)resource_arenas.scratch.data - (uintptr_t)resource_storage.data;
            persistent_offset = (uintptr_t)resource_arenas.persistent.data - (uintptr_t)resource_storage.data;
            archive_offset = (uintptr_t)resource_archive.dir - (uintptr_t)resource_storage.data;
            if (scratch_offset > RESOURCE_STORAGE_CAPACITY || persistent_offset > RESOURCE_STORAGE_CAPACITY || persistent_offset < seed[6] || archive_offset != seed[3] || resource_archive.byte_len > RESOURCE_STORAGE_CAPACITY - archive_offset || resource_archive.relocation_base != seed[4] || resource_archive.byte_len != resource_io.file_size || resource_arenas.scratch.available != RESOURCE_STORAGE_CAPACITY - scratch_offset || resource_arenas.persistent.available != RESOURCE_STORAGE_CAPACITY - persistent_offset || resource_arenas.scratch.context - seed[4] != scratch_offset - seed[3] || resource_arenas.persistent.context - seed[7] != persistent_offset - seed[6])
                abort();
            copy_bytes = persistent_offset - seed[6];
            memcpy(storage_guard + archive_offset, resource_archive.dir, resource_archive.byte_len);
            memcpy(storage_guard + seed[6], resource_storage.data + seed[6], copy_bytes);
            memcpy(refs_guard + fixture, model_refs + fixture, sizeof(model_refs[0]));
            memcpy(banks_guard + banks[fixture], animation_banks + banks[fixture], sizeof(animation_banks[0]));
            heights_guard[fixture] = model_height[fixture];
            arenas_guard.scratch = resource_arenas.scratch;
            arenas_guard.persistent = resource_arenas.persistent;
            loading_guard.count = loading.count;
            display_guard.frame = disp_buffers.frame;
            if (memcmp(storage_guard, resource_storage.data, sizeof(storage_guard)) || memcmp(refs_guard, model_refs, sizeof(refs_guard)) || memcmp(banks_guard, animation_banks, sizeof(banks_guard)) || memcmp(heights_guard, model_height, sizeof(heights_guard)) || memcmp(types_guard, actor_types, sizeof(types_guard)) || memcmp(groups_guard, tex_grps, sizeof(groups_guard)) || memcmp(map_guard, tex_map, sizeof(map_guard)) || memcmp(type_guard, animation_type_banks, sizeof(type_guard)) || memcmp(files_guard, resource_file_names, sizeof(files_guard)) || memcmp(&arenas_guard, &resource_arenas, sizeof(arenas_guard)) || memcmp(&session_guard, &game_session, sizeof(session_guard)) || memcmp(&loading_guard, &loading, sizeof(loading_guard)) || memcmp(&display_guard, &disp_buffers, sizeof(display_guard)))
                abort();
            {
                MODEL_REFERENCE *model = model_refs + fixture;
                uintptr_t head, vertices, faces;
                if (!model->header || !model->vertices || !model->faces || (uintptr_t)model->header < (uintptr_t)resource_storage.data || (uintptr_t)model->vertices < (uintptr_t)resource_storage.data || (uintptr_t)model->faces < (uintptr_t)resource_storage.data)
                    abort();
                head = (uintptr_t)model->header - (uintptr_t)resource_storage.data;
                vertices = (uintptr_t)model->vertices - (uintptr_t)resource_storage.data;
                faces = (uintptr_t)model->faces - (uintptr_t)resource_storage.data;
                if (head != seed[6] || model->byte_len != copy_bytes || vertices > RESOURCE_STORAGE_CAPACITY || faces > RESOURCE_STORAGE_CAPACITY || model->vertex_byte_len > RESOURCE_STORAGE_CAPACITY - vertices || model->face_byte_len > RESOURCE_STORAGE_CAPACITY - faces)
                    abort();
                model_fields[0] = fixture;
                model_fields[1] = (uint32)head;
                model_fields[2] = model->byte_len;
                model_fields[3] = (uint32)vertices;
                model_fields[4] = model->vertex_byte_len;
                model_fields[5] = (uint32)faces;
                model_fields[6] = model->face_byte_len;
            }
            fields[0] = value;
            fields[1] = (uint32)scratch_offset;
            fields[2] = resource_arenas.scratch.context;
            fields[3] = resource_arenas.scratch.available;
            fields[4] = (uint32)persistent_offset;
            fields[5] = resource_arenas.persistent.context;
            fields[6] = resource_arenas.persistent.available;
            fields[7] = (uint32)archive_offset;
            fields[8] = resource_archive.byte_len;
            fields[9] = resource_archive.relocation_base;
            fields[10] = resource_io.result_size;
            fields[11] = resource_io.file_size;
            fields[12] = (uint16)loading.bar_x;
            fields[13] = (uint16)loading.total;
            fields[14] = loading.count;
            fields[15] = disp_buffers.frame;
            fields[16] = fixture;
            fields[17] = seed[6];
            fields[18] = (uint32)copy_bytes;
            fields[19] = RESOURCE_STORAGE_CAPACITY;
            output_header[0] = 0x344e5046;
            output_header[1] = 1;
            output_header[2] = sizeof(fields) + sizeof(model_fields) + sizeof(model_height) + 4 * ANIMATION_COUNT * 16 + sizeof(animation_flags) + resource_archive.byte_len + (uint32)copy_bytes;
            output_header[3] = 1;
            if (fwrite(output_header, sizeof(output_header), 1, out) != 1 || fwrite(fields, sizeof(fields), 1, out) != 1 || fwrite(model_fields, sizeof(model_fields), 1, out) != 1 || fwrite(model_height, sizeof(model_height), 1, out) != 1)
                result = 4;
            for (i = 0; !result && i < 4; i++)
                for (j = 0; !result && j < ANIMATION_COUNT; j++)
                {
                    const ACTOR_ANIMATION_SOURCE *stream = animation_banks[banks[i]].stream + j;
                    uint32 words[4] = {stream->kind, 0xffffffff, stream->stream.byte_len, stream->stream.domain};
                    if (stream->kind == ACTOR_ANIMATION_SOURCE_NONE)
                    {
                        if (stream->stream.header || stream->stream.byte_len || stream->stream.domain)
                            abort();
                    }
                    else if (stream->kind == ACTOR_ANIMATION_SOURCE_STREAM)
                    {
                        uintptr_t offset;
                        if ((uintptr_t)stream->stream.header < (uintptr_t)resource_storage.data)
                            abort();
                        offset = (uintptr_t)stream->stream.header - (uintptr_t)resource_storage.data;
                        if (offset > RESOURCE_STORAGE_CAPACITY - 4 || stream->stream.byte_len != RESOURCE_STORAGE_CAPACITY - offset || stream->stream.domain < ANIMATION_STREAM_PHYSICAL || stream->stream.domain >= ANIMATION_STREAM_DOMAIN_COUNT)
                            abort();
                        words[1] = (uint32)offset;
                    }
                    else
                        abort();
                    if (fwrite(words, sizeof(words), 1, out) != 1)
                        result = 4;
                }
            if (!result && (fwrite(animation_flags, sizeof(animation_flags), 1, out) != 1 || fwrite(resource_archive.dir, resource_archive.byte_len, 1, out) != 1 || fwrite(resource_storage.data + seed[6], copy_bytes, 1, out) != 1))
                result = 4;
        }
        if (fclose(in))
            result = 4;
        if (fclose(out))
            result = 4;
        diagnostic_end();
        return result;
    }
    if (argc > 1 && !strcmp(argv[1], "--audit-enemy-resources"))
    {
        static uint8 storage_guard[RESOURCE_STORAGE_CAPACITY];
        static MODEL_REFERENCE refs_guard[MODEL_TYPE_COUNT];
        static ANIMATION_BANK banks_guard[ANIMATION_BANK_COUNT];
        const ANIMATION_BANK_ID banks[2] = {ANIMATION_BANK_SHARED6, ANIMATION_BANK_SHARED8};
        const uint32 types[7] = {5, 6, 7, 8, 9, 10, 31};
        FILE *in = xport_fopen("../status/gameplay/enemy-resources-jobs.bin", "rb");
        FILE *out = xport_fopen("../status/gameplay/enemy-resources-native.bin", "wb");
        uint32 header[6], seed[14], i, j, value, output_header[4], fields[20], model_fields[7];
        uint32 heights_guard[MODEL_TYPE_COUNT];
        sint16 starts[TEXTURE_GROUP_COUNT], map_guard[TEXTURE_MAP_COUNT], types_guard[ACTOR_COUNT];
        TEXTURE_GROUP groups_guard[TEXTURE_GROUP_COUNT];
        ANIMATION_BANK_ID type_guard[ANIMATION_TYPE_COUNT];
        const char *files_guard[RESOURCE_FILE_COUNT];
        RESOURCE_ARENAS arenas_guard;
        GAME_SESSION session_guard;
        LOADING loading_guard;
        DISPLAY_BUFFERS display_guard;
        uintptr_t scratch_offset, persistent_offset, archive_offset, copy_bytes;
        int result = 0;
        if (!in || !out)
        {
            if (in)
                fclose(in);
            if (out)
                fclose(out);
            return 4;
        }
        if (fread(header, sizeof(header), 1, in) != 1 || header[0] != 0x344a4546 || header[1] != 1 || header[2] != 14 || header[3] != 2 || header[4] != ANIMATION_COUNT || header[5] != sizeof(seed) + sizeof(starts) + sizeof(tex_map) + sizeof(model_height) + 2 * ANIMATION_COUNT * 16 + sizeof(animation_flags) || fread(seed, sizeof(seed), 1, in) != 1 || fread(starts, sizeof(starts), 1, in) != 1 || fread(tex_map, sizeof(tex_map), 1, in) != 1 || fread(model_height, sizeof(model_height), 1, in) != 1)
            result = 4;
        if (!result && (seed[0] > RESOURCE_STORAGE_CAPACITY - 4 || seed[3] > RESOURCE_STORAGE_CAPACITY - 4 || (seed[0] & 3) || (seed[3] & 3) || seed[2] != RESOURCE_STORAGE_CAPACITY - seed[0] || seed[5] != RESOURCE_STORAGE_CAPACITY - seed[3] || seed[6] > 65535 || seed[7] > 65535 || !seed[12] || seed[12] > RESOURCE_STORAGE_CAPACITY - seed[0] || (seed[1] & 0x1fffffff) != RESOURCE_STORAGE_ORIGIN + seed[0] || (seed[4] & 0x1fffffff) != RESOURCE_STORAGE_ORIGIN + seed[3] || ((seed[1] & 0xe0000000) != 0 && (seed[1] & 0xe0000000) != 0x80000000 && (seed[1] & 0xe0000000) != 0xa0000000) || ((seed[4] & 0xe0000000) != 0 && (seed[4] & 0xe0000000) != 0x80000000 && (seed[4] & 0xe0000000) != 0xa0000000)))
            result = 4;
        if (!result)
        {
            /* Only serialized resource storage is initialized; no original game RAM is imported */
            memset(resource_storage.data, 0xa5, sizeof(resource_storage.data));
            memset(model_refs, 0, sizeof(model_refs));
        }
        for (i = 0; !result && i < 2; i++)
            for (j = 0; !result && j < ANIMATION_COUNT; j++)
            {
                uint32 words[4];
                ACTOR_ANIMATION_SOURCE *stream = animation_banks[banks[i]].stream + j;
                if (fread(words, sizeof(words), 1, in) != 1)
                    result = 4;
                else if (words[0] == ACTOR_ANIMATION_SOURCE_NONE)
                {
                    if (words[1] != 0xffffffff || words[2] || words[3])
                        result = 4;
                    else
                        *stream = (ACTOR_ANIMATION_SOURCE){0};
                }
                else if (words[0] == ACTOR_ANIMATION_SOURCE_STREAM && words[1] <= RESOURCE_STORAGE_CAPACITY - 4 && !(words[1] & 3) && words[2] == RESOURCE_STORAGE_CAPACITY - words[1] && words[3] >= ANIMATION_STREAM_PHYSICAL && words[3] < ANIMATION_STREAM_DOMAIN_COUNT)
                    *stream = (ACTOR_ANIMATION_SOURCE){ACTOR_ANIMATION_SOURCE_STREAM, {(const ANIMATION_STREAM_HEADER *)(resource_storage.data + words[1]), words[2], (ANIMATION_STREAM_DOMAIN)words[3]}};
                else
                    result = 4;
            }
        if (!result && (fread(animation_flags, sizeof(animation_flags), 1, in) != 1 || fgetc(in) != EOF))
            result = 4;
        if (!result)
        {
            game_session.stage = seed[13];
            resource_arenas.scratch = (RESOURCE_CURSOR){resource_storage.data + seed[0], seed[2], seed[1]};
            resource_arenas.persistent = (RESOURCE_CURSOR){resource_storage.data + seed[3], seed[5], seed[4]};
            loading = (LOADING){(sint16)(uint16)seed[6], (sint16)(uint16)seed[7], seed[8]};
            disp_buffers.frame = seed[9];
            resource_io = (RESOURCE_IO){seed[10], seed[11]};
            for (i = 0; i < TEXTURE_GROUP_COUNT; i++)
                tex_grps[i].first_tex = starts[i];
            resource_file_names[85] = resource_paths[108];
            memcpy(storage_guard, resource_storage.data, sizeof(storage_guard));
            memcpy(refs_guard, model_refs, sizeof(refs_guard));
            memcpy(banks_guard, animation_banks, sizeof(banks_guard));
            memcpy(heights_guard, model_height, sizeof(heights_guard));
            memcpy(types_guard, actor_types, sizeof(types_guard));
            memcpy(groups_guard, tex_grps, sizeof(groups_guard));
            memcpy(map_guard, tex_map, sizeof(map_guard));
            memcpy(type_guard, animation_type_banks, sizeof(type_guard));
            for (uint32 file_idx = 0; file_idx < RESOURCE_FILE_COUNT; file_idx++)
                files_guard[file_idx] = resource_file_names[file_idx];
            memcpy(&arenas_guard, &resource_arenas, sizeof(arenas_guard));
            memcpy(&session_guard, &game_session, sizeof(session_guard));
            memcpy(&loading_guard, &loading, sizeof(loading_guard));
            memcpy(&display_guard, &disp_buffers, sizeof(display_guard));
            value = resource_loader_call(RESOURCE_LOADER_STAGE, resource_loaders + RESOURCE_LOADER_STAGE0_ACTORS);
            if ((uintptr_t)resource_arenas.scratch.data < (uintptr_t)resource_storage.data || (uintptr_t)resource_arenas.persistent.data < (uintptr_t)resource_storage.data || (uintptr_t)resource_archive.dir < (uintptr_t)resource_storage.data)
                abort();
            scratch_offset = (uintptr_t)resource_arenas.scratch.data - (uintptr_t)resource_storage.data;
            persistent_offset = (uintptr_t)resource_arenas.persistent.data - (uintptr_t)resource_storage.data;
            archive_offset = (uintptr_t)resource_archive.dir - (uintptr_t)resource_storage.data;
            if (scratch_offset > RESOURCE_STORAGE_CAPACITY || persistent_offset > RESOURCE_STORAGE_CAPACITY || persistent_offset < seed[3] || archive_offset != seed[0] || resource_archive.byte_len > RESOURCE_STORAGE_CAPACITY - archive_offset || resource_archive.relocation_base != seed[1] || resource_archive.byte_len != resource_io.file_size || resource_archive.byte_len != seed[12] || resource_arenas.scratch.available != RESOURCE_STORAGE_CAPACITY - scratch_offset || resource_arenas.persistent.available != RESOURCE_STORAGE_CAPACITY - persistent_offset || resource_arenas.scratch.context - seed[1] != scratch_offset - seed[0] || resource_arenas.persistent.context - seed[4] != persistent_offset - seed[3])
                abort();
            copy_bytes = persistent_offset - seed[3];
            memcpy(storage_guard + archive_offset, resource_archive.dir, resource_archive.byte_len);
            memcpy(storage_guard + seed[3], resource_storage.data + seed[3], copy_bytes);
            for (i = 0; i < 7; i++)
            {
                memcpy(refs_guard + types[i], model_refs + types[i], sizeof(model_refs[0]));
                heights_guard[types[i]] = model_height[types[i]];
            }
            for (i = 0; i < 2; i++)
                memcpy(banks_guard + banks[i], animation_banks + banks[i], sizeof(animation_banks[0]));
            arenas_guard.scratch = resource_arenas.scratch;
            arenas_guard.persistent = resource_arenas.persistent;
            loading_guard.count = loading.count;
            display_guard.frame = disp_buffers.frame;
            if (memcmp(storage_guard, resource_storage.data, sizeof(storage_guard)) || memcmp(refs_guard, model_refs, sizeof(refs_guard)) || memcmp(banks_guard, animation_banks, sizeof(banks_guard)) || memcmp(heights_guard, model_height, sizeof(heights_guard)) || memcmp(types_guard, actor_types, sizeof(types_guard)) || memcmp(groups_guard, tex_grps, sizeof(groups_guard)) || memcmp(map_guard, tex_map, sizeof(map_guard)) || memcmp(type_guard, animation_type_banks, sizeof(type_guard)) || memcmp(files_guard, resource_file_names, sizeof(files_guard)) || memcmp(&arenas_guard, &resource_arenas, sizeof(arenas_guard)) || memcmp(&session_guard, &game_session, sizeof(session_guard)) || memcmp(&loading_guard, &loading, sizeof(loading_guard)) || memcmp(&display_guard, &disp_buffers, sizeof(display_guard)))
                abort();
            fields[0] = value;
            fields[1] = (uint32)scratch_offset;
            fields[2] = resource_arenas.scratch.context;
            fields[3] = resource_arenas.scratch.available;
            fields[4] = (uint32)persistent_offset;
            fields[5] = resource_arenas.persistent.context;
            fields[6] = resource_arenas.persistent.available;
            fields[7] = (uint32)archive_offset;
            fields[8] = resource_archive.byte_len;
            fields[9] = resource_archive.relocation_base;
            fields[10] = resource_io.result_size;
            fields[11] = resource_io.file_size;
            fields[12] = (uint16)loading.bar_x;
            fields[13] = (uint16)loading.total;
            fields[14] = loading.count;
            fields[15] = disp_buffers.frame;
            fields[16] = 7;
            fields[17] = seed[3];
            fields[18] = (uint32)copy_bytes;
            fields[19] = RESOURCE_STORAGE_CAPACITY;
            output_header[0] = 0x344e4546;
            output_header[1] = 1;
            output_header[2] = sizeof(fields) + 7 * sizeof(model_fields) + sizeof(model_height) + 2 * ANIMATION_COUNT * 16 + sizeof(animation_flags) + resource_archive.byte_len + (uint32)copy_bytes;
            output_header[3] = 1;
            if (fwrite(output_header, sizeof(output_header), 1, out) != 1 || fwrite(fields, sizeof(fields), 1, out) != 1 || fwrite(model_height, sizeof(model_height), 1, out) != 1)
                result = 4;
            for (i = 0; !result && i < 7; i++)
            {
                MODEL_REFERENCE *model = model_refs + types[i];
                uintptr_t head, vertices, faces;
                if (!model->header || !model->vertices || !model->faces || (uintptr_t)model->header < (uintptr_t)resource_storage.data || (uintptr_t)model->vertices < (uintptr_t)resource_storage.data || (uintptr_t)model->faces < (uintptr_t)resource_storage.data)
                    abort();
                head = (uintptr_t)model->header - (uintptr_t)resource_storage.data;
                vertices = (uintptr_t)model->vertices - (uintptr_t)resource_storage.data;
                faces = (uintptr_t)model->faces - (uintptr_t)resource_storage.data;
                if (head < seed[3] || head > persistent_offset || model->byte_len > persistent_offset - head || vertices > RESOURCE_STORAGE_CAPACITY || faces > RESOURCE_STORAGE_CAPACITY || model->vertex_byte_len > RESOURCE_STORAGE_CAPACITY - vertices || model->face_byte_len > RESOURCE_STORAGE_CAPACITY - faces)
                    abort();
                model_fields[0] = types[i];
                model_fields[1] = (uint32)head;
                model_fields[2] = model->byte_len;
                model_fields[3] = (uint32)vertices;
                model_fields[4] = model->vertex_byte_len;
                model_fields[5] = (uint32)faces;
                model_fields[6] = model->face_byte_len;
                if (fwrite(model_fields, sizeof(model_fields), 1, out) != 1)
                    result = 4;
            }
            for (i = 0; !result && i < 2; i++)
                for (j = 0; !result && j < ANIMATION_COUNT; j++)
                {
                    const ACTOR_ANIMATION_SOURCE *stream = animation_banks[banks[i]].stream + j;
                    uint32 words[4] = {stream->kind, 0xffffffff, stream->stream.byte_len, stream->stream.domain};
                    if (stream->kind == ACTOR_ANIMATION_SOURCE_NONE)
                    {
                        if (stream->stream.header || stream->stream.byte_len || stream->stream.domain)
                            abort();
                    }
                    else if (stream->kind == ACTOR_ANIMATION_SOURCE_STREAM)
                    {
                        uintptr_t offset;
                        if ((uintptr_t)stream->stream.header < (uintptr_t)resource_storage.data)
                            abort();
                        offset = (uintptr_t)stream->stream.header - (uintptr_t)resource_storage.data;
                        if (offset > RESOURCE_STORAGE_CAPACITY - 4 || stream->stream.byte_len != RESOURCE_STORAGE_CAPACITY - offset || stream->stream.domain < ANIMATION_STREAM_PHYSICAL || stream->stream.domain >= ANIMATION_STREAM_DOMAIN_COUNT)
                            abort();
                        words[1] = (uint32)offset;
                    }
                    else
                        abort();
                    if (fwrite(words, sizeof(words), 1, out) != 1)
                        result = 4;
                }
            if (!result && (fwrite(animation_flags, sizeof(animation_flags), 1, out) != 1 || fwrite(resource_archive.dir, resource_archive.byte_len, 1, out) != 1 || fwrite(resource_storage.data + seed[3], copy_bytes, 1, out) != 1))
                result = 4;
        }
        if (fclose(in))
            result = 4;
        if (fclose(out))
            result = 4;
        diagnostic_end();
        return result;
    }
    if (argc > 1 && !strcmp(argv[1], "--audit-level-models"))
    {
        static uint8 storage_guard[RESOURCE_STORAGE_CAPACITY];
        static MODEL_REFERENCE refs_guard[MODEL_TYPE_COUNT];
        const uint32 types[] = {17, 56, 57, 58, 59, 60, 61, 63, 64, 65, 66, 67, 68, 69, 70, 71, 72, 73, 74, 75, 76, 77, 78, 79, 80, 81, 82, 83, 84, 85, 86, 87, 88, 89, 93, 94, 95, 96, 97, 98, 99, 100, 101, 102, 103, 104, 105, 106, 108, 109, 110, 111, 112, 113, 114, 115, 116, 117, 464, 473, 474, 475, 477, 478, 479, 480, 481, 482, 483, 484, 485, 486, 487, 488, 489, 491, 493, 495, 496, 497, 498, 499, 500, 501, 502, 503, 504, 506, 515, 522, 523, 524, 525, 526, 528, 529, 533, 534, 572, 573, 578, 587, 588, 589, 590, 591, 593, 594, 595, 596, 598, 599, 600, 601, 603, 604, 606, 607, 608, 614, 615, 618, 619, 620, 621, 622, 623, 624, 625, 627, 628, 629, 630, 633, 641, 860, 861, 867, 868, 882, 883, 884, 885, 887, 888, 889, 897, 902, 903, 904, 905, 906, 907, 908, 909, 919};
        uint32 header[6], seed[12], fields[14], output_header[4], i, value = 0;
        uint32 heights_guard[MODEL_TYPE_COUNT];
        sint16 starts[TEXTURE_GROUP_COUNT], map_guard[TEXTURE_MAP_COUNT];
        TEXTURE_GROUP groups_guard[TEXTURE_GROUP_COUNT];
        const char *files_guard[RESOURCE_FILE_COUNT];
        const RESOURCE_LOADER *loaders_guard[STAGE_RESOURCE_KIND_COUNT][RESOURCE_STAGE_COUNT];
        RESOURCE_ARENAS arenas_guard;
        GAME_SESSION session_guard;
        LOADING loading_guard;
        DISPLAY_BUFFERS display_guard;
        FILE *in = xport_fopen("../status/gameplay/level-models-jobs.bin", "rb");
        FILE *out = xport_fopen("../status/gameplay/level-models-native.bin", "wb");
        uint8 *archive = NULL;
        int result = 0;
        if (!in || !out)
        {
            if (in)
                fclose(in);
            if (out)
                fclose(out);
            return 4;
        }
        if (fread(header, sizeof(header), 1, in) != 1 || header[0] != 0x344a4c46 || header[1] != 1 || header[2] != 12 || header[3] != TEXTURE_GROUP_COUNT || header[4] != TEXTURE_MAP_COUNT || header[5] != MODEL_TYPE_COUNT || fread(seed, sizeof(seed), 1, in) != 1 || fread(starts, sizeof(starts), 1, in) != 1 || fread(tex_map, sizeof(tex_map), 1, in) != 1 || fread(model_height, sizeof(model_height), 1, in) != 1 || fgetc(in) != EOF)
            result = 4;
        if (!result && (seed[0] != 0 || seed[1] > RESOURCE_STORAGE_CAPACITY || (seed[1] & 3) || !seed[10] || seed[10] > RESOURCE_STORAGE_CAPACITY - seed[1] || seed[3] != RESOURCE_STORAGE_CAPACITY - seed[1] || seed[4] > 65535 || seed[5] > 65535 || seed[11] != sizeof(types) / sizeof(types[0]) || (seed[2] & 0x1fffffff) != RESOURCE_STORAGE_ORIGIN + seed[1] || ((seed[2] & 0xe0000000) != 0 && (seed[2] & 0xe0000000) != 0x80000000 && (seed[2] & 0xe0000000) != 0xa0000000)))
            result = 4;
        if (!result)
        {
            /* The complete original stage0 callback and common caller share one WAD owner */
            memset(resource_storage.data, 0xa5, sizeof(resource_storage.data));
            memset(model_refs, 0, sizeof(model_refs));
            archive = resource_storage.data + seed[1];
            resource_arenas.persistent = (RESOURCE_CURSOR){archive, seed[3], seed[2]};
            game_session.stage = seed[0];
            loading = (LOADING){(sint16)(uint16)seed[4], (sint16)(uint16)seed[5], seed[6]};
            disp_buffers.frame = seed[7];
            resource_io = (RESOURCE_IO){seed[8], seed[9]};
            for (i = 0; i < TEXTURE_GROUP_COUNT; i++)
                tex_grps[i].first_tex = starts[i];
            resource_file_names[41] = resource_paths[42];
            stage_resource_loaders[STAGE_RESOURCE_MODELS][0] = resource_loaders + RESOURCE_LOADER_STAGE0_MODELS;
            memcpy(storage_guard, resource_storage.data, sizeof(storage_guard));
            memcpy(refs_guard, model_refs, sizeof(refs_guard));
            memcpy(heights_guard, model_height, sizeof(heights_guard));
            memcpy(groups_guard, tex_grps, sizeof(groups_guard));
            memcpy(map_guard, tex_map, sizeof(map_guard));
            for (uint32 file_idx = 0; file_idx < RESOURCE_FILE_COUNT; file_idx++)
                files_guard[file_idx] = resource_file_names[file_idx];
            for (uint32 kind_idx = 0; kind_idx < STAGE_RESOURCE_KIND_COUNT; kind_idx++)
                for (uint32 stage_idx = 0; stage_idx < RESOURCE_STAGE_COUNT; stage_idx++)
                    loaders_guard[kind_idx][stage_idx] = stage_resource_loaders[kind_idx][stage_idx];
            memcpy(&arenas_guard, &resource_arenas, sizeof(arenas_guard));
            memcpy(&session_guard, &game_session, sizeof(session_guard));
            memcpy(&loading_guard, &loading, sizeof(loading_guard));
            memcpy(&display_guard, &disp_buffers, sizeof(display_guard));
            value = level_load_common_models();
            if (resource_archive.dir != (RESOURCE_DIRECTORY *)archive || resource_archive.byte_len != seed[10] || resource_archive.relocation_base != seed[2] || resource_io.file_size != seed[10] || resource_arenas.persistent.data != archive + seed[10] || resource_arenas.persistent.context != seed[2] + seed[10] || resource_arenas.persistent.available != seed[3] - seed[10])
                abort();
            memcpy(storage_guard + seed[1], archive, seed[10]);
            for (i = 0; i < sizeof(types) / sizeof(types[0]); i++)
            {
                memcpy(refs_guard + types[i], model_refs + types[i], sizeof(model_refs[0]));
                heights_guard[types[i]] = model_height[types[i]];
            }
            arenas_guard.persistent = resource_arenas.persistent;
            loading_guard.count = loading.count;
            display_guard.frame = disp_buffers.frame;
            if (memcmp(storage_guard, resource_storage.data, sizeof(storage_guard)) || memcmp(refs_guard, model_refs, sizeof(refs_guard)) || memcmp(heights_guard, model_height, sizeof(heights_guard)) || memcmp(groups_guard, tex_grps, sizeof(groups_guard)) || memcmp(map_guard, tex_map, sizeof(map_guard)) || memcmp(files_guard, resource_file_names, sizeof(files_guard)) || memcmp(loaders_guard, stage_resource_loaders, sizeof(loaders_guard)) || memcmp(&arenas_guard, &resource_arenas, sizeof(arenas_guard)) || memcmp(&session_guard, &game_session, sizeof(session_guard)) || memcmp(&loading_guard, &loading, sizeof(loading_guard)) || memcmp(&display_guard, &disp_buffers, sizeof(display_guard)))
                abort();
            fields[0] = value;
            fields[1] = game_session.stage;
            fields[2] = seed[1] + seed[10];
            fields[3] = resource_arenas.persistent.context;
            fields[4] = resource_arenas.persistent.available;
            fields[5] = seed[1];
            fields[6] = resource_archive.byte_len;
            fields[7] = resource_archive.relocation_base;
            fields[8] = resource_io.result_size;
            fields[9] = resource_io.file_size;
            fields[10] = (uint16)loading.bar_x;
            fields[11] = (uint16)loading.total;
            fields[12] = loading.count;
            fields[13] = disp_buffers.frame;
            output_header[0] = 0x344e4c46;
            output_header[1] = 1;
            output_header[2] = sizeof(fields) + sizeof(model_height) + (sizeof(types) / sizeof(types[0])) * 28 + seed[10];
            output_header[3] = 1;
            if (fwrite(output_header, sizeof(output_header), 1, out) != 1 || fwrite(fields, sizeof(fields), 1, out) != 1 || fwrite(model_height, sizeof(model_height), 1, out) != 1)
                result = 4;
            for (i = 0; !result && i < sizeof(types) / sizeof(types[0]); i++)
            {
                MODEL_REFERENCE *model = model_refs + types[i];
                uintptr_t head, vertex, face;
                uint32 row[7];
                if (!model->header || !model->vertices || !model->faces || (uintptr_t)model->header < (uintptr_t)archive || (uintptr_t)model->vertices < (uintptr_t)archive || (uintptr_t)model->faces < (uintptr_t)archive)
                    abort();
                head = (uintptr_t)model->header - (uintptr_t)archive;
                vertex = (uintptr_t)model->vertices - (uintptr_t)archive;
                face = (uintptr_t)model->faces - (uintptr_t)archive;
                if (head > seed[10] || vertex > seed[10] || face > seed[10] || model->byte_len > seed[10] - head || model->vertex_byte_len > seed[10] - vertex || model->face_byte_len > seed[10] - face)
                    abort();
                row[0] = types[i];
                row[1] = seed[1] + (uint32)head;
                row[2] = model->byte_len;
                row[3] = seed[1] + (uint32)vertex;
                row[4] = model->vertex_byte_len;
                row[5] = seed[1] + (uint32)face;
                row[6] = model->face_byte_len;
                if (fwrite(row, sizeof(row), 1, out) != 1)
                    result = 4;
            }
            if (!result && fwrite(archive, seed[10], 1, out) != 1)
                result = 4;
        }
        if (fclose(in))
            result = 4;
        if (fclose(out))
            result = 4;
        diagnostic_end();
        return result;
    }
    if (argc > 1 && !strcmp(argv[1], "--audit-sequence-init"))
    {
        static uint8 storage_guard[RESOURCE_STORAGE_CAPACITY];
        static MODEL_REFERENCE refs_guard[MODEL_TYPE_COUNT];
        static ANIMATION_BANK banks_guard[ANIMATION_BANK_COUNT];
        const ANIMATION_BANK_ID banks[4] = {ANIMATION_BANK_HAWK, ANIMATION_BANK_MACE, ANIMATION_BANK_SMASHER, ANIMATION_BANK_ALANA};
        const uint32 types[] = {0, 1, 2, 3, 56, 59, 60, 61, 70, 78, 79, 80, 81, 82, 83, 84, 93, 94, 106, 108, 113, 114, 115, 116, 117, 118, 464, 526, 564, 614, 615, 618, 619, 620, 621, 622, 623, 624, 625, 627, 630, 633, 641, 834, 860, 919};
        uint32 header[6], seed[12], fields[24], output_header[4], i, j, value = 0;
        uint32 owned_bytes = 0, copy_bytes = 0;
        uint32 heights_guard[MODEL_TYPE_COUNT];
        sint16 starts[TEXTURE_GROUP_COUNT], map_guard[TEXTURE_MAP_COUNT];
        TEXTURE_GROUP groups_guard[TEXTURE_GROUP_COUNT];
        ANIMATION_BANK_ID bank_types_guard[ANIMATION_TYPE_COUNT];
        FIXED_VEC3 positions_guard[ACTOR_COUNT];
        sint16 actor_types_guard[ACTOR_COUNT];
        SCENE_PLAYBACK scene_guard;
        const char *files_guard[RESOURCE_FILE_COUNT];
        const RESOURCE_LOADER *loaders_guard[STAGE_RESOURCE_KIND_COUNT][RESOURCE_STAGE_COUNT];
        RESOURCE_ARENAS arenas_guard;
        GAME_SESSION session_guard;
        LOADING loading_guard;
        DISPLAY_BUFFERS display_guard;
        FILE *in = xport_fopen("../status/gameplay/sequence-init-jobs.bin", "rb");
        FILE *out = xport_fopen("../status/gameplay/sequence-init-native.bin", "wb");
        uint8 *archive = NULL;
        int result = 0;
        if (!in || !out)
        {
            if (in)
                fclose(in);
            if (out)
                fclose(out);
            return 4;
        }
        if (fread(header, sizeof(header), 1, in) != 1 || header[0] != 0x344a5346 || header[1] != 1 || header[2] != 12 || header[3] != TEXTURE_GROUP_COUNT || header[4] != TEXTURE_MAP_COUNT || header[5] != MODEL_TYPE_COUNT || fread(seed, sizeof(seed), 1, in) != 1 || fread(starts, sizeof(starts), 1, in) != 1 || fread(tex_map, sizeof(tex_map), 1, in) != 1 || fread(model_height, sizeof(model_height), 1, in) != 1)
            result = 4;
        if (!result && (seed[1] > RESOURCE_STORAGE_CAPACITY || (seed[1] & 3) || !seed[10] || seed[10] > RESOURCE_STORAGE_CAPACITY - seed[1] || seed[3] != RESOURCE_STORAGE_CAPACITY - seed[1] || seed[4] > 65535 || seed[5] > 65535 || seed[11] != sizeof(types) / sizeof(types[0]) || (seed[2] & 0x1fffffff) != RESOURCE_STORAGE_ORIGIN + seed[1] || ((seed[2] & 0xe0000000) != 0 && (seed[2] & 0xe0000000) != 0x80000000 && (seed[2] & 0xe0000000) != 0xa0000000)))
            result = 4;
        for (i = 0; !result && i < 4; i++)
            for (j = 0; !result && j < ANIMATION_COUNT; j++)
            {
                uint32 words[4];
                ACTOR_ANIMATION_SOURCE *stream = animation_banks[banks[i]].stream + j;
                if (fread(words, sizeof(words), 1, in) != 1)
                    result = 4;
                else if (words[0] == ACTOR_ANIMATION_SOURCE_NONE)
                {
                    if (words[1] != 0xffffffff || words[2] || words[3])
                        result = 4;
                    else
                        *stream = (ACTOR_ANIMATION_SOURCE){0};
                }
                else if (words[0] == ACTOR_ANIMATION_SOURCE_STREAM && words[1] <= RESOURCE_STORAGE_CAPACITY - 4 && !(words[1] & 3) && words[2] == RESOURCE_STORAGE_CAPACITY - words[1] && words[3] >= ANIMATION_STREAM_PHYSICAL && words[3] < ANIMATION_STREAM_DOMAIN_COUNT)
                    *stream = (ACTOR_ANIMATION_SOURCE){ACTOR_ANIMATION_SOURCE_STREAM, {(const ANIMATION_STREAM_HEADER *)(resource_storage.data + words[1]), words[2], (ANIMATION_STREAM_DOMAIN)words[3]}};
                else
                    result = 4;
            }
        if (!result && (fread(animation_flags, sizeof(animation_flags), 1, in) != 1 || fread(actor_positions, sizeof(actor_positions), 1, in) != 1 || fgetc(in) != EOF))
            result = 4;
        if (!result)
        {
            /* Scene MOD,ADT,ACH and CCT borrows retain one resource owner */
            memset(resource_storage.data, 0xa5, sizeof(resource_storage.data));
            memset(model_refs, 0, sizeof(model_refs));
            archive = resource_storage.data + seed[1];
            resource_arenas.persistent = (RESOURCE_CURSOR){archive, seed[3], seed[2]};
            game_session.stage = seed[0];
            loading = (LOADING){(sint16)(uint16)seed[4], (sint16)(uint16)seed[5], seed[6]};
            disp_buffers.frame = seed[7];
            resource_io = (RESOURCE_IO){seed[8], seed[9]};
            for (i = 0; i < TEXTURE_GROUP_COUNT; i++)
                tex_grps[i].first_tex = starts[i];
            resource_file_names[67] = resource_paths[64];
            for (i = 0; i < 4; i++)
                animation_type_banks[i] = banks[i];
            memcpy(storage_guard, resource_storage.data, sizeof(storage_guard));
            memcpy(refs_guard, model_refs, sizeof(refs_guard));
            memcpy(banks_guard, animation_banks, sizeof(banks_guard));
            memcpy(bank_types_guard, animation_type_banks, sizeof(bank_types_guard));
            memcpy(positions_guard, actor_positions, sizeof(positions_guard));
            memcpy(actor_types_guard, actor_types, sizeof(actor_types_guard));
            memcpy(&scene_guard, &scene_playback, sizeof(scene_guard));
            memcpy(heights_guard, model_height, sizeof(heights_guard));
            memcpy(groups_guard, tex_grps, sizeof(groups_guard));
            memcpy(map_guard, tex_map, sizeof(map_guard));
            for (uint32 file_idx = 0; file_idx < RESOURCE_FILE_COUNT; file_idx++)
                files_guard[file_idx] = resource_file_names[file_idx];
            for (uint32 kind_idx = 0; kind_idx < STAGE_RESOURCE_KIND_COUNT; kind_idx++)
                for (uint32 stage_idx = 0; stage_idx < RESOURCE_STAGE_COUNT; stage_idx++)
                    loaders_guard[kind_idx][stage_idx] = stage_resource_loaders[kind_idx][stage_idx];
            memcpy(&arenas_guard, &resource_arenas, sizeof(arenas_guard));
            memcpy(&session_guard, &game_session, sizeof(session_guard));
            memcpy(&loading_guard, &loading, sizeof(loading_guard));
            memcpy(&display_guard, &disp_buffers, sizeof(display_guard));
            value = scene_load_resources0();
            if (resource_archive.dir != (RESOURCE_DIRECTORY *)archive || resource_archive.byte_len != seed[10] || resource_archive.relocation_base != seed[2] || resource_io.file_size != seed[10] || (uintptr_t)resource_arenas.persistent.data < (uintptr_t)archive + seed[10] || (uintptr_t)resource_arenas.persistent.data > (uintptr_t)resource_storage.data + RESOURCE_STORAGE_CAPACITY)
                abort();
            owned_bytes = (uint32)((uintptr_t)resource_arenas.persistent.data - (uintptr_t)archive);
            copy_bytes = owned_bytes - seed[10];
            if (resource_arenas.persistent.context != seed[2] + owned_bytes || resource_arenas.persistent.available != seed[3] - owned_bytes)
                abort();
            memcpy(storage_guard + seed[1], archive, owned_bytes);
            for (i = 0; i < sizeof(types) / sizeof(types[0]); i++)
            {
                memcpy(refs_guard + types[i], model_refs + types[i], sizeof(model_refs[0]));
                heights_guard[types[i]] = model_height[types[i]];
            }
            for (i = 0; i < 4; i++)
                memcpy(banks_guard + banks[i], animation_banks + banks[i], sizeof(animation_banks[0]));
            positions_guard[0].x = actor_positions[0].x;
            positions_guard[0].z = actor_positions[0].z;
            positions_guard[1].x = actor_positions[1].x;
            positions_guard[1].z = actor_positions[1].z;
            scene_guard.camera_frames = scene_playback.camera_frames;
            scene_guard.end_tick = scene_playback.end_tick;
            memcpy(scene_guard.stream, scene_playback.stream, 2 * sizeof(scene_playback.stream[0]));
            arenas_guard.persistent = resource_arenas.persistent;
            loading_guard.count = loading.count;
            display_guard.frame = disp_buffers.frame;
            if (memcmp(storage_guard, resource_storage.data, sizeof(storage_guard)) || memcmp(refs_guard, model_refs, sizeof(refs_guard)) || memcmp(heights_guard, model_height, sizeof(heights_guard)) || memcmp(groups_guard, tex_grps, sizeof(groups_guard)) || memcmp(map_guard, tex_map, sizeof(map_guard)) || memcmp(files_guard, resource_file_names, sizeof(files_guard)) || memcmp(loaders_guard, stage_resource_loaders, sizeof(loaders_guard)) || memcmp(banks_guard, animation_banks, sizeof(banks_guard)) || memcmp(bank_types_guard, animation_type_banks, sizeof(bank_types_guard)) || memcmp(positions_guard, actor_positions, sizeof(positions_guard)) || memcmp(actor_types_guard, actor_types, sizeof(actor_types_guard)) || memcmp(&scene_guard, &scene_playback, sizeof(scene_guard)) || memcmp(&arenas_guard, &resource_arenas, sizeof(arenas_guard)) || memcmp(&session_guard, &game_session, sizeof(session_guard)) || memcmp(&loading_guard, &loading, sizeof(loading_guard)) ||
                memcmp(&display_guard, &disp_buffers, sizeof(display_guard)))
                abort();
            fields[0] = value;
            fields[1] = game_session.stage;
            fields[2] = seed[1] + owned_bytes;
            fields[3] = resource_arenas.persistent.context;
            fields[4] = resource_arenas.persistent.available;
            fields[5] = seed[1];
            fields[6] = resource_archive.byte_len;
            fields[7] = resource_archive.relocation_base;
            fields[8] = resource_io.result_size;
            fields[9] = resource_io.file_size;
            fields[10] = (uint16)loading.bar_x;
            fields[11] = (uint16)loading.total;
            fields[12] = loading.count;
            fields[13] = disp_buffers.frame;
            fields[14] = scene_playback.camera_frames;
            fields[15] = scene_playback.end_tick;
            for (i = 0; i < 2; i++)
            {
                uintptr_t at;
                if (!scene_playback.stream[i].data || (uintptr_t)scene_playback.stream[i].data < (uintptr_t)archive)
                    abort();
                at = (uintptr_t)scene_playback.stream[i].data - (uintptr_t)archive;
                if (at > seed[10] || scene_playback.stream[i].byte_len > seed[10] - at)
                    abort();
                fields[16 + 2 * i] = seed[1] + (uint32)at;
                fields[17 + 2 * i] = scene_playback.stream[i].byte_len;
            }
            fields[20] = sizeof(types) / sizeof(types[0]);
            fields[21] = seed[1] + seed[10];
            fields[22] = copy_bytes;
            fields[23] = RESOURCE_STORAGE_CAPACITY;
            output_header[0] = 0x344e5346;
            output_header[1] = 1;
            output_header[2] = sizeof(fields) + sizeof(model_height) + (sizeof(types) / sizeof(types[0])) * 28 + 4 * ANIMATION_COUNT * 16 + sizeof(animation_flags) + sizeof(actor_positions) + owned_bytes;
            output_header[3] = 1;
            if (fwrite(output_header, sizeof(output_header), 1, out) != 1 || fwrite(fields, sizeof(fields), 1, out) != 1 || fwrite(model_height, sizeof(model_height), 1, out) != 1)
                result = 4;
            for (i = 0; !result && i < sizeof(types) / sizeof(types[0]); i++)
            {
                MODEL_REFERENCE *model = model_refs + types[i];
                uintptr_t head, vertex, face;
                uint32 row[7];
                if (!model->header || !model->vertices || !model->faces || (uintptr_t)model->header < (uintptr_t)archive || (uintptr_t)model->vertices < (uintptr_t)archive || (uintptr_t)model->faces < (uintptr_t)archive)
                    abort();
                head = (uintptr_t)model->header - (uintptr_t)archive;
                vertex = (uintptr_t)model->vertices - (uintptr_t)archive;
                face = (uintptr_t)model->faces - (uintptr_t)archive;
                if (head > owned_bytes || vertex > owned_bytes || face > owned_bytes || model->byte_len > owned_bytes - head || model->vertex_byte_len > owned_bytes - vertex || model->face_byte_len > owned_bytes - face)
                    abort();
                row[0] = types[i];
                row[1] = seed[1] + (uint32)head;
                row[2] = model->byte_len;
                row[3] = seed[1] + (uint32)vertex;
                row[4] = model->vertex_byte_len;
                row[5] = seed[1] + (uint32)face;
                row[6] = model->face_byte_len;
                if (fwrite(row, sizeof(row), 1, out) != 1)
                    result = 4;
            }
            for (i = 0; !result && i < 4; i++)
                for (j = 0; !result && j < ANIMATION_COUNT; j++)
                {
                    const ACTOR_ANIMATION_SOURCE *stream = animation_banks[banks[i]].stream + j;
                    uint32 words[4] = {stream->kind, 0xffffffff, stream->stream.byte_len, stream->stream.domain};
                    if (stream->kind == ACTOR_ANIMATION_SOURCE_NONE)
                    {
                        if (stream->stream.header || stream->stream.byte_len || stream->stream.domain)
                            abort();
                    }
                    else if (stream->kind == ACTOR_ANIMATION_SOURCE_STREAM)
                    {
                        uintptr_t offset;
                        if ((uintptr_t)stream->stream.header < (uintptr_t)resource_storage.data)
                            abort();
                        offset = (uintptr_t)stream->stream.header - (uintptr_t)resource_storage.data;
                        if (offset > RESOURCE_STORAGE_CAPACITY - 4 || stream->stream.byte_len != RESOURCE_STORAGE_CAPACITY - offset || stream->stream.domain < ANIMATION_STREAM_PHYSICAL || stream->stream.domain >= ANIMATION_STREAM_DOMAIN_COUNT)
                            abort();
                        words[1] = (uint32)offset;
                    }
                    else
                        abort();
                    if (fwrite(words, sizeof(words), 1, out) != 1)
                        result = 4;
                }
            if (!result && (fwrite(animation_flags, sizeof(animation_flags), 1, out) != 1 || fwrite(actor_positions, sizeof(actor_positions), 1, out) != 1 || fwrite(archive, owned_bytes, 1, out) != 1))
                result = 4;
        }
        if (fclose(in))
            result = 4;
        if (fclose(out))
            result = 4;
        diagnostic_end();
        return result;
    }
    if (argc > 2 && !strcmp(argv[1], "--menu-cold-script"))
        return frontend_run_script(argv[2], argc > 3 && !strcmp(argv[3], "--headless"));
    if (argc > 1 && !strcmp(argv[1], "--menu-cold"))
        return frontend_run(argc > 2 ? (uint32)strtoul(argv[2], 0, 0) : 0, argc > 3 && !strcmp(argv[3], "--headless"));
    if (argc > 1 && strcmp(argv[1], "--audit-model-matrix") == 0)
        return geometry_audit_run(5);
    if (argc > 1 && strcmp(argv[1], "--audit-collision-distance") == 0)
    {
        FILE *in, *out;
        uint32 a[4], observation[2];
        size_t got = 0;
        int result = 0;
        {
            uint32 initial_distance;
            FILE *seed = xport_fopen("../status/menu/collision-distance-seed.bin", "rb");
            int valid;
            if (!seed)
                return 4;
            valid = fread(&initial_distance, sizeof(initial_distance), 1, seed) == 1 && fgetc(seed) == EOF && !ferror(seed);
            if (fclose(seed))
                valid = 0;
            if (!valid)
                return 4;
            dir_distance = (sint32)initial_distance;
        }
        in = xport_fopen("../status/menu/collision-distance-jobs.bin", "rb");
        out = xport_fopen("../status/menu/collision-distance-native.bin", "wb");
        if (!in || !out)
        {
            if (in)
                fclose(in);
            if (out)
                fclose(out);
            return 4;
        }
        while (!result && (got = fread(a, 1, sizeof(a), in)) == sizeof(a))
        {
            sint32 unchanged = dir_distance;
            observation[0] = (uint32)math_distance_approx((sint32)a[0], (sint32)a[1], (sint32)a[2], (sint32)a[3]);
            observation[1] = (uint32)dir_distance;
            if (dir_distance != unchanged)
                abort();
            if (fwrite(observation, sizeof(observation), 1, out) != 1)
                result = 4;
        }
        if (ferror(in) || (!result && got))
            result = 4;
        if (fclose(in))
            result = 4;
        if (fclose(out))
            result = 4;
        if (!result)
            diagnostic_end();
        return result;
    }
    if (argc > 1 && strcmp(argv[1], "--audit-spatial-audio") == 0)
    {
        FILE *in, *out;
        uint32 a[4], observation[4];
        size_t got = 0;
        int result = 0;
        in = xport_fopen("../status/menu/spatial-audio-jobs.bin", "rb");
        out = xport_fopen("../status/menu/spatial-audio-native.bin", "wb");
        if (!in || !out)
        {
            if (in)
                fclose(in);
            if (out)
                fclose(out);
            return 4;
        }
        while (!result && (got = fread(a, 1, sizeof(a), in)) == sizeof(a))
        {
            FIXED_VEC3 position = {0}, unchanged;
            CAMERA camera_guard;
            uint32 volume = 0xa5a5a5a5, pan = 0xa5a5a5a5;
            camera.pos.x.whole = (sint16)(uint16)a[0];
            camera.pos.z.whole = (sint16)(uint16)a[1];
            camera.angles.yaw_bits = (uint16)(a[0] + a[3]);
            position.x.whole = (sint16)(uint16)a[2];
            position.z.whole = (sint16)(uint16)a[3];
            memcpy(&unchanged, &position, sizeof(unchanged));
            memcpy(&camera_guard, &camera, sizeof(camera_guard));
            observation[0] = (uint32)sound_spatial(&position, &volume, &pan);
            observation[1] = (uint32)dir_distance;
            observation[2] = volume;
            observation[3] = pan;
            if (memcmp(&unchanged, &position, sizeof(unchanged)) || memcmp(&camera_guard, &camera, sizeof(camera_guard)))
                abort();
            if (fwrite(observation, sizeof(observation), 1, out) != 1)
                result = 4;
        }
        if (ferror(in) || (!result && got))
            result = 4;
        if (fclose(in))
            result = 4;
        if (fclose(out))
            result = 4;
        if (!result)
            diagnostic_end();
        return result;
    }
    if (argc > 1 && !strcmp(argv[1], "--audit-texture-uv-fix"))
    {
        FILE *in, *out;
        size_t got = 0;
        int result = 0;
        uint32 a[4];
        const uint32 header[4] = {0x32565546, 1, 24, 0};
        in = xport_fopen("../status/menu/texture-uv-fix-jobs.bin", "rb");
        out = xport_fopen("../status/menu/texture-uv-fix-native.bin", "wb");
        if (!in || !out)
        {
            if (in)
                fclose(in);
            if (out)
                fclose(out);
            return 4;
        }
        if (fwrite(header, sizeof(header), 1, out) != 1)
            result = 4;
        while (!result && (got = fread(a, 1, sizeof(a), in)) == sizeof(a))
        {
            game_session.stage = a[0];
            tex_allocator.y = a[3];
            memset(tex_base_v, 0xa5, sizeof(tex_base_v));
            TEXTURE_DESCRIPTOR desc;
            TEXTURE_ALLOCATOR unchanged;
            memcpy(&unchanged, &tex_allocator, sizeof(unchanged));
            memset(&desc, 0xa5, sizeof(desc));
            tex_fix_uv(a[1], a[2], &desc);
            if (memcmp(&unchanged, &tex_allocator, sizeof(unchanged)))
                abort();
            if (fwrite(tex_base_v, sizeof(tex_base_v), 1, out) != 1)
                result = 4;
            if (fwrite(desc.words, sizeof(desc.words), 1, out) != 1)
                result = 4;
        }
        if (ferror(in) || (!result && got))
            result = 4;
        if (fclose(in))
            result = 4;
        if (fclose(out))
            result = 4;
        if (!result)
            diagnostic_end();
        return result;
    }
    if (argc > 1 && !strcmp(argv[1], "--audit-texture-descriptor"))
    {
        FILE *in, *out;
        size_t got = 0;
        int result = 0;
        uint32 a[10];
        in = xport_fopen("../status/menu/texture-descriptor-jobs.bin", "rb");
        out = xport_fopen("../status/menu/texture-descriptor-native.bin", "wb");
        if (!in || !out)
        {
            if (in)
                fclose(in);
            if (out)
                fclose(out);
            return 4;
        }
        while (!result && (got = fread(a, 1, sizeof(a), in)) == sizeof(a))
        {
            tex_desc_count = 64;
            tex_count = 5;
            tex_map[5] = (sint16)0xa5a5;
            memset(tex_desc.words + 63 * 3, 0xa5, 12 * sizeof(uint32));
            tex_allocator.x = a[8];
            tex_allocator.y = a[9];
            tex_build_desc(a[0], a[1], a[2], a[3], a[4], a[5], a[6], a[7]);
            if (fwrite(tex_desc.words + 63 * 3, 12 * sizeof(uint32), 1, out) != 1)
                result = 4;
            if (fwrite(&tex_map[5], sizeof(tex_map[5]), 1, out) != 1)
                result = 4;
            if (fwrite(&tex_desc_count, sizeof(tex_desc_count), 1, out) != 1)
                result = 4;
            if (fwrite(&tex_allocator.x, sizeof(tex_allocator.x), 1, out) != 1)
                result = 4;
            if (fwrite(&tex_allocator.y, sizeof(tex_allocator.y), 1, out) != 1)
                result = 4;
        }
        if (ferror(in) || (!result && got))
            result = 4;
        if (fclose(in))
            result = 4;
        if (fclose(out))
            result = 4;
        if (!result)
            diagnostic_end();
        return result;
    }
    if (argc > 1 && !strcmp(argv[1], "--audit-effect-palette"))
    {
        const uint32 header[4] = {0x32504546, 1, 862, 0};
        FILE *in, *out;
        uint32 stage;
        uint16 palette[TEXTURE_PAL_COLORS];
        size_t got = 0;
        int result = 0;
        in = xport_fopen("../status/menu/effect-palette-jobs.bin", "rb");
        out = xport_fopen("../status/menu/effect-palette-native.bin", "wb");
        if (!in || !out)
        {
            if (in)
                fclose(in);
            if (out)
                fclose(out);
            return 4;
        }
        if (fwrite(header, sizeof(header), 1, out) != 1)
            result = 4;
        while (!result && (got = fread(&stage, 1, sizeof(stage), in)) == sizeof(stage))
        {
            if (fread(palette, 1, sizeof(palette), in) != sizeof(palette))
            {
                result = 4;
                break;
            }
            game_session.stage = stage;
            memset(&tex_effect_pals, 0xa5, sizeof(tex_effect_pals));
            tex_build_effect_pals(palette);
            if (game_session.stage != stage)
                abort();
            if (fwrite(palette, sizeof(palette), 1, out) != 1 || fwrite(tex_effect_pals.fade, sizeof(tex_effect_pals.fade), 1, out) != 1 || fwrite(tex_effect_pals.cycle, sizeof(tex_effect_pals.cycle), 1, out) != 1 || fwrite(tex_effect_pals.stage0, sizeof(tex_effect_pals.stage0), 1, out) != 1 || fwrite(tex_effect_pals.stage0_dim, sizeof(tex_effect_pals.stage0_dim), 1, out) != 1)
                result = 4;
        }
        if (ferror(in) || (!result && got))
            result = 4;
        if (fclose(in))
            result = 4;
        if (fclose(out))
            result = 4;
        if (!result)
            diagnostic_end();
        return result;
    }
    if (argc > 1 && !strcmp(argv[1], "--audit-texture-pack"))
    {
        FILE *in, *out;
        uint32 a[4], value;
        size_t got = 0;
        int result = 0;
        in = xport_fopen("../status/menu/texture-pack-jobs.bin", "rb");
        out = xport_fopen("../status/menu/texture-pack-native.bin", "wb");
        if (!in || !out)
        {
            if (in)
                fclose(in);
            if (out)
                fclose(out);
            return 4;
        }
        tex_allocator.small_img_count = 0x12345678;
        tex_allocator_reset();
        while (!result && (got = fread(a, 1, sizeof(a), in)) == sizeof(a))
        {
            PSX_RECT rect;
            uint16 rectangle[4];
            memset(&rect, 0xa5, sizeof(rect));
            value = a[0] ? tex_alloc_pal() : tex_alloc_image((sint32)a[1], (sint32)a[2], (sint32)a[3], &rect);
            rectangle[0] = (uint16)rect.x;
            rectangle[1] = (uint16)rect.y;
            rectangle[2] = (uint16)rect.w;
            rectangle[3] = (uint16)rect.h;
            if (tex_allocator.small_img_count != 0x12345678)
                abort();
            if (fwrite(&value, sizeof(value), 1, out) != 1 || fwrite(&tex_allocator.x, sizeof(tex_allocator.x), 1, out) != 1 || fwrite(&tex_allocator.y, sizeof(tex_allocator.y), 1, out) != 1 || fwrite(rectangle, sizeof(rectangle), 1, out) != 1)
                result = 4;
        }
        if (ferror(in) || (!result && got))
            result = 4;
        if (!result && (fwrite(tex_allocator.used, sizeof(tex_allocator.used), 1, out) != 1 || fwrite(tex_allocator.row_used, sizeof(tex_allocator.row_used), 1, out) != 1))
            result = 4;
        if (fclose(in))
            result = 4;
        if (fclose(out))
            result = 4;
        if (!result)
            diagnostic_end();
        return result;
    }
    if (argc > 1 && !strcmp(argv[1], "--audit-texture-map-reset"))
    {
        const uint32 header[4] = {0x32525446, 1, 16516, 0};
        uint32 i, value;
        int result = 0;
        FILE *out = xport_fopen("../status/menu/texture-map-reset-native.bin", "wb");
        if (!out)
            return 4;
        if (fwrite(header, sizeof(header), 1, out) != 1)
            result = 4;
        for (i = 0; !result && i < 3; i++)
        {
            TEXTURE_ALLOCATOR unchanged;
            memset(tex_allocator.used, i ? 0xff : 0xa5, sizeof(tex_allocator.used));
            memset(tex_allocator.row_used, i ? 0xff : 0xa5, sizeof(tex_allocator.row_used));
            tex_allocator.x = 0xa5a5;
            tex_allocator.y = 0x5a5a;
            tex_allocator.small_img_count = 0x12345678;
            memcpy(&unchanged, &tex_allocator, sizeof(unchanged));
            value = tex_allocator_reset();
            if (i == 2)
                value = tex_allocator_reset();
            memcpy(unchanged.used, tex_allocator.used, sizeof(unchanged.used));
            memcpy(unchanged.row_used, tex_allocator.row_used, sizeof(unchanged.row_used));
            if (memcmp(&unchanged, &tex_allocator, sizeof(unchanged)))
                abort();
            if (fwrite(&value, sizeof(value), 1, out) != 1 || fwrite(tex_allocator.used, sizeof(tex_allocator.used), 1, out) != 1 || fwrite(tex_allocator.row_used, sizeof(tex_allocator.row_used), 1, out) != 1)
                result = 4;
        }
        if (fclose(out))
            result = 4;
        if (!result)
            diagnostic_end();
        return result;
    }
    if (argc > 1 && (!strcmp(argv[1], "--audit-resource-exists") || !strcmp(argv[1], "--audit-resource-progress")))
    {
        FILE *in, *out;
        uint8 *archive = NULL, *archive_guard = NULL;
        uint32 header[5], output_header[4], job[7], cases = 0;
        size_t got = 0, job_bytes;
        int progress = !strcmp(argv[1], "--audit-resource-progress"), result = 0;
        if (progress)
            xport_set_headless(1);
        job_bytes = progress ? 28 : 20;
        in = xport_fopen(progress ? "../status/menu/resource-progress-jobs.bin" : "../status/menu/resource-exists-jobs.bin", "rb");
        out = xport_fopen(progress ? "../status/menu/resource-progress-native.bin" : "../status/menu/resource-exists-native.bin", "wb");
        if (!in || !out)
        {
            if (in)
                fclose(in);
            if (out)
                fclose(out);
            return 4;
        }
        if (fread(header, sizeof(header), 1, in) != 1 || header[0] != 0x324a4546 || header[1] != 1 || header[2] < 4 || header[2] > RESOURCE_STORAGE_CAPACITY || header[3] != (progress ? 45u : 35u))
            result = 4;
        if (!result)
        {
            archive = malloc(header[2]);
            archive_guard = malloc(header[2]);
            if (!archive || !archive_guard || fread(archive, 1, header[2], in) != header[2])
                result = 4;
        }
        if (!result)
        {
            memcpy(archive_guard, archive, header[2]);
            resource_archive = (RESOURCE_ARCHIVE){(RESOURCE_DIRECTORY *)archive, header[2], header[4]};
            output_header[0] = 0x324e4546;
            output_header[1] = 1;
            output_header[2] = progress ? 16 : 8;
            output_header[3] = header[3];
            if (fwrite(output_header, sizeof(output_header), 1, out) != 1)
                result = 4;
        }
        while (!result && (got = fread(job, 1, job_bytes, in)) == job_bytes)
        {
            RESOURCE_ARCHIVE owner_guard;
            RESOURCE_IO io_guard;
            LOADING loading_guard;
            DISPLAY_BUFFERS display_guard;
            uint32 fields[4], job_guard[7];
            char *name = (char *)(progress ? job + 3 : job);
            if (cases >= header[3] || !memchr(name, 0, 16))
            {
                result = 4;
                break;
            }
            if (progress)
            {
                loading.total = (sint16)(uint16)job[0];
                loading.count = job[1];
                disp_buffers.frame = job[2];
            }
            else
            {
                resource_archive.dir->count_be = resource_swap32(job[4]);
                memcpy(archive_guard, archive, 4);
            }
            resource_io.result_size = 0xa5a5a5a5;
            memcpy(job_guard, job, job_bytes);
            memcpy(&owner_guard, &resource_archive, sizeof(owner_guard));
            memcpy(&io_guard, &resource_io, sizeof(io_guard));
            memcpy(&loading_guard, &loading, sizeof(loading_guard));
            memcpy(&display_guard, &disp_buffers, sizeof(display_guard));
            if (progress)
            {
                RESOURCE_VIEW view = resource_find(name);
                uintptr_t offset;
                if (view.storage != archive || view.storage_len != header[2] || (uintptr_t)view.data < (uintptr_t)archive || (uintptr_t)view.data - (uintptr_t)archive > header[2])
                    abort();
                offset = (uintptr_t)view.data - (uintptr_t)archive;
                if (view.byte_len > header[2] - offset || view.context_address - header[4] != offset || view.byte_len != resource_io.result_size)
                    abort();
                fields[0] = (uint32)offset;
                fields[1] = loading.count;
                fields[2] = disp_buffers.frame;
                fields[3] = resource_io.result_size;
                io_guard.result_size = resource_io.result_size;
                loading_guard.count = loading.count;
                display_guard.frame = disp_buffers.frame;
            }
            else
            {
                fields[0] = (uint32)resource_exists(name);
                fields[1] = resource_io.result_size;
            }
            if (memcmp(job_guard, job, job_bytes) || memcmp(archive_guard, archive, header[2]) || memcmp(&owner_guard, &resource_archive, sizeof(owner_guard)) || memcmp(&io_guard, &resource_io, sizeof(io_guard)) || memcmp(&loading_guard, &loading, sizeof(loading_guard)) || memcmp(&display_guard, &disp_buffers, sizeof(display_guard)))
                abort();
            if (fwrite(fields, progress ? 16 : 8, 1, out) != 1)
                result = 4;
            ++cases;
        }
        if (ferror(in) || (!result && (got || cases != header[3])))
            result = 4;
        if (fclose(in))
            result = 4;
        if (fclose(out))
            result = 4;
        resource_archive = (RESOURCE_ARCHIVE){0};
        free(archive_guard);
        free(archive);
        if (!result)
            diagnostic_end();
        return result;
    }
    if (argc > 1 && !strcmp(argv[1], "--audit-terrain-descriptors"))
    {
        FILE *in, *out;
        uint32 job[2], base, i, cases = 0, count_guard;
        TEXTURE_DESCRIPTOR *desc;
        TEXTURE_GROUP group_guard[TEXTURE_GROUP_COUNT];
        TEXTURE_ALLOCATOR allocator_guard;
        size_t got = 0;
        int result = 0;
        in = xport_fopen("../status/menu/terrain-descriptors-jobs.bin", "rb");
        out = xport_fopen("../status/menu/terrain-descriptors-native.bin", "wb");
        if (!in || !out)
        {
            if (in)
                fclose(in);
            if (out)
                fclose(out);
            return 4;
        }
        while (!result && (got = fread(job, 1, sizeof(job), in)) == sizeof(job))
        {
            base = job[0];
            if (base > TEXTURE_DESCRIPTOR_CAPACITY - 258 || job[1] > 65535)
            {
                result = 4;
                break;
            }
            desc = tex_desc.item + base;
            memset(&tex_desc, 0xa5, sizeof(tex_desc));
            tex_desc_count = base;
            tex_grps[TEXTURE_GROUP_FONTS].first_tex = (sint16)(uint16)job[1];
            memcpy(group_guard, tex_grps, sizeof(group_guard));
            memcpy(&allocator_guard, &tex_allocator, sizeof(allocator_guard));
            count_guard = tex_count;
            terrain_build_descs();
            group_guard[TEXTURE_GROUP_TERRAIN].first_tex = tex_grps[TEXTURE_GROUP_TERRAIN].first_tex;
            allocator_guard.x = tex_allocator.x;
            allocator_guard.y = tex_allocator.y;
            if (memcmp(group_guard, tex_grps, sizeof(group_guard)) || memcmp(&allocator_guard, &tex_allocator, sizeof(allocator_guard)) || count_guard != tex_count)
                abort();
            for (i = 0; i < sizeof(tex_desc); i++)
                if ((i < base * sizeof(*desc) || i >= (base + 258) * sizeof(*desc)) && ((uint8 *)&tex_desc)[i] != 0xa5)
                    abort();
            if (fwrite(&tex_grps[TEXTURE_GROUP_FONTS].first_tex, sizeof(sint16), 1, out) != 1 || fwrite(&tex_grps[TEXTURE_GROUP_TERRAIN].first_tex, sizeof(sint16), 1, out) != 1 || fwrite(&tex_allocator.x, sizeof(tex_allocator.x), 1, out) != 1 || fwrite(&tex_allocator.y, sizeof(tex_allocator.y), 1, out) != 1 || fwrite(&tex_desc_count, sizeof(tex_desc_count), 1, out) != 1 || fwrite(desc, 258 * sizeof(*desc), 1, out) != 1 || ++cases > 3)
                result = 4;
        }
        if (ferror(in) || (!result && got) || cases != 3 || ferror(out))
            result = 4;
        if (fclose(in))
            result = 4;
        if (fclose(out))
            result = 4;
        if (!result)
            diagnostic_end();
        return result;
    }
    if (argc > 1 && !strcmp(argv[1], "--audit-mpd-objects"))
    {
        const uint32 expected_header[4] = {0x324a4d46, 1, 3, 8303};
        const uint32 output_header[4] = {0x324e4d46, 1, 3, 7192};
        FILE *in, *out;
        uint32 header[4], cases = 0, i, result_word;
        uint8 data[8303], data_guard[8303], kinds[80];
        sint16 height_guard[MODEL_TYPE_COUNT];
        WORLD_REGION_KIND kind_guard[WORLD_REGION_KIND_COUNT];
        size_t got = 0;
        int result = 0;
        in = xport_fopen("../status/menu/mpd-objects-jobs.bin", "rb");
        out = xport_fopen("../status/menu/mpd-objects-native.bin", "wb");
        if (!in || !out)
        {
            if (in)
                fclose(in);
            if (out)
                fclose(out);
            return 4;
        }
        if (fread(header, sizeof(header), 1, in) != 1 || memcmp(header, expected_header, sizeof(header)) || fread(world_object_height, sizeof(world_object_height), 1, in) != 1 || fread(kinds, sizeof(kinds), 1, in) != 1 || fwrite(output_header, sizeof(output_header), 1, out) != 1)
            result = 4;
        if (!result)
        {
            for (i = 0; i < WORLD_REGION_KIND_COUNT; i++)
            {
                world_region_kinds[i].mode = (sint8)kinds[2 * i];
                world_region_kinds[i].flags = (sint8)kinds[2 * i + 1];
            }
            memcpy(height_guard, world_object_height, sizeof(height_guard));
            memcpy(kind_guard, world_region_kinds, sizeof(kind_guard));
        }
        while (!result && (got = fread(data, 1, sizeof(data), in)) == sizeof(data))
        {
            WORLD_OBJECTS world_guard;
            COLLISION_REGIONS regions_guard;
            GAME_SESSION session_guard;
            uint32 seed_guard;
            RESOURCE_VIEW map = {0};
            if (cases >= 3)
            {
                result = 4;
                break;
            }
            game_session.stage = 26;
            world_object_active_types = WORLD_OBJECT_TYPES_FRONTEND;
            for (i = 0; i < WORLD_OBJECT_COUNT; i++)
            {
                MODEL_OBJECT *item = world_objects.object + i;
                memset(item, 0, sizeof(*item));
                item->kind = 0xa5a5a5a5;
                item->pos.x.bits = item->pos.y.bits = item->pos.z.bits = 0xa5a5a5a5;
                item->angles.yaw_bits = item->angles.pitch_bits = item->angles.roll_bits = 0xa5a5;
                item->payload_kind = MODEL_PAYLOAD_SPRITE;
                item->spr.frame = 123;
            }
            for (i = 0; i < COLLISION_REGION_COUNT; i++)
            {
                COLLISION_REGION *region = col_regions.data + i;
                memset(region, 0, sizeof(*region));
                region->geometry.coord[0] = region->geometry.coord[1] = 123;
                region->geometry.coord[2] = region->geometry.coord[3] = 456;
                region->trigger_kind = 3;
                region->shape = 2;
                region->col_kind = 1;
                region->model_type = 454;
                region->hits = 12;
                region->alternate_attack = 1;
                region->object = (WORLD_OBJECT_REFERENCE){0, 1};
            }
            col_regions.named[0] = col_regions.data;
            col_regions.named[1] = col_regions.data + 1;
            memcpy(&world_guard, &world_objects, sizeof(world_guard));
            memcpy(&regions_guard, &col_regions, sizeof(regions_guard));
            memcpy(&session_guard, &game_session, sizeof(session_guard));
            memcpy(data_guard, data, sizeof(data_guard));
            seed_guard = random_seed;
            map.storage = data;
            map.storage_len = sizeof(data);
            map.data = data;
            map.byte_len = sizeof(data);
            result_word = world_objects_load_map(map);
            world_guard.count = world_objects.count;
            world_guard.tracked_count = world_objects.tracked_count;
            memcpy(world_guard.object, world_objects.object, sizeof(world_guard.object));
            regions_guard.count = col_regions.count;
            memcpy(regions_guard.data, col_regions.data, sizeof(regions_guard.data));
            regions_guard.named[0] = col_regions.named[0];
            regions_guard.named[1] = col_regions.named[1];
            if (memcmp(data_guard, data, sizeof(data)) || memcmp(&world_guard, &world_objects, sizeof(world_guard)) || memcmp(&regions_guard, &col_regions, sizeof(regions_guard)) || memcmp(&session_guard, &game_session, sizeof(session_guard)) || memcmp(height_guard, world_object_height, sizeof(height_guard)) || memcmp(kind_guard, world_region_kinds, sizeof(kind_guard)) || random_seed != seed_guard || world_object_active_types != WORLD_OBJECT_TYPES_FRONTEND)
                abort();
            if (!frontend_world_audit_store(out, result_word))
                result = 4;
            ++cases;
        }
        if (ferror(in) || (!result && (got || cases != 3)))
            result = 4;
        if (fclose(in))
            result = 4;
        if (fclose(out))
            result = 4;
        if (!result)
            diagnostic_end();
        return result;
    }
    if (argc > 1 && !strcmp(argv[1], "--audit-terrain-relocate"))
    {
        FILE *in, *out;
        uint32 offset, value, cases = 0;
        TEXTURE_GROUP group_guard[TEXTURE_GROUP_COUNT];
        uint8 color_guard[sizeof(terrain_map.color_flat)];
        size_t got = 0;
        int result = 0;
        in = xport_fopen("../status/menu/terrain-relocate-jobs.bin", "rb");
        out = xport_fopen("../status/menu/terrain-relocate-native.bin", "wb");
        if (!in || !out)
        {
            if (in)
                fclose(in);
            if (out)
                fclose(out);
            return 4;
        }
        while (!result && (got = fread(&offset, 1, sizeof(offset), in)) == sizeof(offset))
        {
            if (offset > 65535 || fread(terrain_map.tile_flat, sizeof(terrain_map.tile_flat), 1, in) != 1)
            {
                result = 4;
                break;
            }
            tex_grps[TEXTURE_GROUP_TERRAIN].first_tex = (sint16)(uint16)offset;
            memcpy(group_guard, tex_grps, sizeof(group_guard));
            memcpy(color_guard, terrain_map.color_flat, sizeof(color_guard));
            value = terrain_relocate_tiles();
            if (memcmp(group_guard, tex_grps, sizeof(group_guard)) || memcmp(color_guard, terrain_map.color_flat, sizeof(color_guard)))
                abort();
            if (fwrite(&value, sizeof(value), 1, out) != 1 || fwrite(terrain_map.tile_flat, sizeof(terrain_map.tile_flat), 1, out) != 1 || ++cases > 5)
                result = 4;
        }
        if (ferror(in) || (!result && got) || cases != 5 || ferror(out))
            result = 4;
        if (fclose(in))
            result = 4;
        if (fclose(out))
            result = 4;
        if (!result)
            diagnostic_end();
        return result;
    }
    if (argc > 1 && !strcmp(argv[1], "--audit-terrain-map"))
    {
        FILE *in, *out, *asset;
        uint32 value, cases = 0;
        static uint8 pixels[65536], pixels_guard[65536];

        struct
        {
            uint8 before[16];
            uint8 bytes[8192];
            uint8 after[16];
        } map, map_guard;

        RESOURCE_VIEW pixel_view = {.data = pixels, .byte_len = sizeof(pixels), .storage = pixels, .storage_len = sizeof(pixels)};
        RESOURCE_VIEW map_view = {.data = map.bytes, .byte_len = sizeof(map.bytes), .storage = map.bytes, .storage_len = sizeof(map.bytes)};
        size_t got = 0;
        int result = 0;
        asset = xport_fopen("../status/menu/terrain-map-pixels.bin", "rb");
        if (!asset)
            return 4;
        if (fread(pixels, sizeof(pixels), 1, asset) != 1 || fgetc(asset) != EOF || ferror(asset))
            result = 4;
        if (fclose(asset))
            result = 4;
        if (result)
            return result;
        memcpy(pixels_guard, pixels, sizeof(pixels_guard));
        in = xport_fopen("../status/menu/terrain-map-jobs.bin", "rb");
        out = xport_fopen("../status/menu/terrain-map-native.bin", "wb");
        if (!in || !out)
        {
            if (in)
                fclose(in);
            if (out)
                fclose(out);
            return 4;
        }
        memset(&map, 0xa5, sizeof(map));
        while (!result && (got = fread(map.bytes, 1, sizeof(map.bytes), in)) == sizeof(map.bytes))
        {
            memcpy(&map_guard, &map, sizeof(map_guard));
            memset(&terrain_map, 0xa5, sizeof(terrain_map));
            value = terrain_build_map(map_view, pixel_view);
            if (memcmp(&map, &map_guard, sizeof(map)) || memcmp(pixels_guard, pixels, sizeof(pixels)))
                abort();
            if (fwrite(&value, sizeof(value), 1, out) != 1 || fwrite(terrain_map.tile_flat, sizeof(terrain_map.tile_flat), 1, out) != 1 || fwrite(terrain_map.color_flat, sizeof(terrain_map.color_flat), 1, out) != 1 || ++cases > 4)
                result = 4;
        }
        if (ferror(in) || (!result && got) || cases != 4 || ferror(out))
            result = 4;
        if (fclose(in))
            result = 4;
        if (fclose(out))
            result = 4;
        if (!result)
            diagnostic_end();
        return result;
    }
    if (argc > 1 && !strcmp(argv[1], "--audit-score-format"))
    {
        const uint32 header[4] = {0x32534846, 1, 20, 0};
        FILE *in, *out;
        uint32 job[3], value;
        size_t got = 0;
        int result = 0;
        in = xport_fopen("../status/menu/score-format-jobs.bin", "rb");
        out = xport_fopen("../status/menu/score-format-native.bin", "wb");
        if (!in || !out)
        {
            if (in)
                fclose(in);
            if (out)
                fclose(out);
            return 4;
        }
        if (fwrite(header, sizeof(header), 1, out) != 1)
            result = 4;
        while (!result && (got = fread(job, 1, sizeof(job), in)) == sizeof(job))
        {
            HUD hud_guard;
            PLAYER_SCORE scores_guard[PLAYER_COUNT];
            player_scores[0].val = job[1];
            player_scores[1].val = job[2];
            memset(hud.score_text, 0xa5, sizeof(hud.score_text));
            memcpy(&hud_guard, &hud, sizeof(hud_guard));
            memcpy(scores_guard, player_scores, sizeof(scores_guard));
            value = (uint32)hud_format_score((sint32)job[0]);
            memcpy(hud_guard.score_text, hud.score_text, sizeof(hud_guard.score_text));
            if (memcmp(&hud_guard, &hud, sizeof(hud_guard)) || memcmp(scores_guard, player_scores, sizeof(scores_guard)))
                abort();
            if (fwrite(&value, sizeof(value), 1, out) != 1 || fwrite(hud.score_text, sizeof(hud.score_text), 1, out) != 1)
                result = 4;
        }
        if (ferror(in) || (!result && got))
            result = 4;
        if (fclose(in))
            result = 4;
        if (fclose(out))
            result = 4;
        if (!result)
            diagnostic_end();
        return result;
    }
    if (argc > 1 && !strcmp(argv[1], "--audit-startup-table"))
    {
        const uint32 header[4] = {0x324e4446, 1, 16388, 1};
        FILE *out;
        uint32 value;
        int result = 0;
        out = xport_fopen("../status/menu/startup-table-native.bin", "wb");
        if (!out)
            return 4;
        memset(depth_table, 0xa5, sizeof(depth_table));
        value = depth_table_build();
        if (fwrite(header, sizeof(header), 1, out) != 1 || fwrite(&value, sizeof(value), 1, out) != 1 || fwrite(depth_table, sizeof(depth_table), 1, out) != 1 || ferror(out))
            result = 4;
        if (fclose(out))
            result = 4;
        if (!result)
            diagnostic_end();
        return result;
    }

    if (argc > 1 && !strcmp(argv[1], "--audit-menu-models"))
    {
        static uint8 storage_guard[RESOURCE_STORAGE_CAPACITY];
        static MODEL_REFERENCE refs_guard[MODEL_TYPE_COUNT];
        static ANIMATION_BANK banks_guard[ANIMATION_BANK_COUNT];
        const ANIMATION_BANK_ID banks[4] = {ANIMATION_BANK_HAWK, ANIMATION_BANK_MACE, ANIMATION_BANK_SMASHER, ANIMATION_BANK_ALANA};
        const uint32 types[23] = {887, 888, 860, 454, 455, 456, 457, 458, 459, 832, 833, 564, 834, 835, 836, 837, 838, 839, 840, 0, 1, 2, 3};
        const uint32 output_header[4] = {0x324e4a46, 1, 123418, 1};
        FILE *in, *out;
        uint32 header[5], i, j, value = 0;
        sint16 starts[TEXTURE_GROUP_COUNT], map_guard[TEXTURE_MAP_COUNT];
        TEXTURE_GROUP groups_guard[TEXTURE_GROUP_COUNT];
        ANIMATION_BANK_ID type_guard[ANIMATION_TYPE_COUNT];
        const char *files_guard[RESOURCE_FILE_COUNT];
        RESOURCE_ARENAS arenas_guard;
        LOADING loading_guard;
        DISPLAY_BUFFERS display_guard;
        uint8 *arena = NULL;
        uintptr_t advance = 0;
        int result = 0;
        in = xport_fopen("../status/menu/menu-models-jobs.bin", "rb");
        out = xport_fopen("../status/menu/menu-models-native.bin", "wb");
        if (!in || !out)
        {
            if (in)
                fclose(in);
            if (out)
                fclose(out);
            return 4;
        }
        if (fread(header, sizeof(header), 1, in) != 1 || header[0] != 0x324a4a46 || header[1] != 1 || header[2] > RESOURCE_STORAGE_CAPACITY - 100000 || (header[2] & 3) || fread(starts, sizeof(starts), 1, in) != 1 || fread(tex_map, sizeof(tex_map), 1, in) != 1 || fread(model_height, sizeof(model_height), 1, in) != 1)
            result = 4;
        for (i = 0; !result && i < 4; i++)
        {
            for (j = 0; !result && j < ANIMATION_COUNT; j++)
            {
                uint32 words[4];
                ACTOR_ANIMATION_SOURCE *source = animation_banks[banks[i]].stream + j;
                if (fread(words, sizeof(words), 1, in) != 1)
                    result = 4;
                else if (words[0] == ACTOR_ANIMATION_SOURCE_NONE)
                {
                    if (words[1] != 0xffffffff || words[2] || words[3])
                        result = 4;
                    else
                        *source = (ACTOR_ANIMATION_SOURCE){0};
                }
                else if (words[0] == ACTOR_ANIMATION_SOURCE_STREAM && words[1] <= RESOURCE_STORAGE_CAPACITY - 4 && words[2] == RESOURCE_STORAGE_CAPACITY - words[1] && words[3] >= ANIMATION_STREAM_PHYSICAL && words[3] < ANIMATION_STREAM_DOMAIN_COUNT)
                    *source = (ACTOR_ANIMATION_SOURCE){ACTOR_ANIMATION_SOURCE_STREAM, {(const ANIMATION_STREAM_HEADER *)(resource_storage.data + words[1]), words[2], (ANIMATION_STREAM_DOMAIN)words[3]}};
                else
                    result = 4;
            }
        }
        if (!result && (fread(animation_flags, sizeof(animation_flags), 1, in) != 1 || fgetc(in) != EOF))
            result = 4;
        if (!result)
        {
            arena = resource_storage.data + header[2];
            memset(arena, 0xa5, 100000);
            memset(model_refs, 0, sizeof(model_refs));
            for (i = 0; i < TEXTURE_GROUP_COUNT; i++)
                tex_grps[i].first_tex = starts[i];
            for (i = 0; i < 4; i++)
                animation_type_banks[i] = banks[i];
            resource_file_names[66] = resource_paths[106];
            resource_arenas.persistent = (RESOURCE_CURSOR){arena, RESOURCE_STORAGE_CAPACITY - header[2], header[3]};
            loading.total = 0;
            loading.count = header[4];
            memcpy(storage_guard, resource_storage.data, sizeof(storage_guard));
            memcpy(refs_guard, model_refs, sizeof(refs_guard));
            memcpy(banks_guard, animation_banks, sizeof(banks_guard));
            memcpy(groups_guard, tex_grps, sizeof(groups_guard));
            memcpy(map_guard, tex_map, sizeof(map_guard));
            memcpy(type_guard, animation_type_banks, sizeof(type_guard));
            for (uint32 file_idx = 0; file_idx < RESOURCE_FILE_COUNT; file_idx++)
                files_guard[file_idx] = resource_file_names[file_idx];
            memcpy(&arenas_guard, &resource_arenas, sizeof(arenas_guard));
            memcpy(&loading_guard, &loading, sizeof(loading_guard));
            memcpy(&display_guard, &disp_buffers, sizeof(display_guard));
            value = resource_load_frontend_models();
            if (resource_archive.dir != (RESOURCE_DIRECTORY *)arena || resource_archive.byte_len != resource_io.file_size || resource_io.file_size != 59180 || resource_archive.relocation_base != header[3] || (uintptr_t)resource_arenas.persistent.data < (uintptr_t)arena || (uintptr_t)resource_arenas.persistent.data - (uintptr_t)arena > 100000)
                abort();
            advance = (uintptr_t)resource_arenas.persistent.data - (uintptr_t)arena;
            if (resource_arenas.persistent.context - header[3] != advance || resource_arenas.persistent.available != RESOURCE_STORAGE_CAPACITY - header[2] - advance)
                abort();
            for (i = 0; i < 23; i++)
                memcpy(refs_guard + types[i], model_refs + types[i], sizeof(model_refs[0]));
            for (i = 0; i < 4; i++)
                memcpy(banks_guard + banks[i], animation_banks + banks[i], sizeof(animation_banks[0]));
            memcpy(storage_guard + header[2], arena, 100000);
            memcpy(&arenas_guard.persistent, &resource_arenas.persistent, sizeof(arenas_guard.persistent));
            loading_guard.count = loading.count;
            if (memcmp(storage_guard, resource_storage.data, sizeof(storage_guard)) || memcmp(refs_guard, model_refs, sizeof(refs_guard)) || memcmp(banks_guard, animation_banks, sizeof(banks_guard)) || memcmp(groups_guard, tex_grps, sizeof(groups_guard)) || memcmp(map_guard, tex_map, sizeof(map_guard)) || memcmp(type_guard, animation_type_banks, sizeof(type_guard)) || memcmp(files_guard, resource_file_names, sizeof(files_guard)) || memcmp(&arenas_guard, &resource_arenas, sizeof(arenas_guard)) || memcmp(&loading_guard, &loading, sizeof(loading_guard)) || memcmp(&display_guard, &disp_buffers, sizeof(display_guard)))
                abort();
            if (fwrite(output_header, sizeof(output_header), 1, out) != 1)
                result = 4;
            {
                uint32 values[5] = {value, (uint32)advance, resource_io.file_size, resource_io.result_size, loading.count};
                if (fwrite(values, sizeof(values), 1, out) != 1 || fwrite(arena, 100000, 1, out) != 1)
                    result = 4;
            }
            for (i = 0; i < 23; i++)
            {
                MODEL_REFERENCE *model = model_refs + types[i];
                uintptr_t head, vertices, faces;
                uint32 fields[7];
                if (!model->header || !model->vertices || !model->faces || (uintptr_t)model->header < (uintptr_t)resource_storage.data || (uintptr_t)model->header - (uintptr_t)resource_storage.data > RESOURCE_STORAGE_CAPACITY || (uintptr_t)model->vertices < (uintptr_t)resource_storage.data || (uintptr_t)model->vertices - (uintptr_t)resource_storage.data > RESOURCE_STORAGE_CAPACITY || (uintptr_t)model->faces < (uintptr_t)resource_storage.data || (uintptr_t)model->faces - (uintptr_t)resource_storage.data > RESOURCE_STORAGE_CAPACITY)
                    abort();
                head = (uintptr_t)model->header - (uintptr_t)resource_storage.data;
                vertices = (uintptr_t)model->vertices - (uintptr_t)resource_storage.data;
                faces = (uintptr_t)model->faces - (uintptr_t)resource_storage.data;
                if (model->byte_len > RESOURCE_STORAGE_CAPACITY - head || model->vertex_byte_len > RESOURCE_STORAGE_CAPACITY - vertices || model->face_byte_len > RESOURCE_STORAGE_CAPACITY - faces)
                    abort();
                fields[0] = types[i];
                fields[1] = (uint32)head;
                fields[2] = model->byte_len;
                fields[3] = (uint32)vertices;
                fields[4] = model->vertex_byte_len;
                fields[5] = (uint32)faces;
                fields[6] = model->face_byte_len;
                if (fwrite(fields, sizeof(fields), 1, out) != 1)
                    result = 4;
            }
            if (fwrite(model_height, sizeof(model_height), 1, out) != 1)
                result = 4;
            for (i = 0; i < 4; i++)
                for (j = 0; j < ANIMATION_COUNT; j++)
                {
                    ACTOR_ANIMATION_SOURCE *source = animation_banks[banks[i]].stream + j;
                    uint32 words[4] = {source->kind, 0xffffffff, source->stream.byte_len, source->stream.domain};
                    if (source->kind == ACTOR_ANIMATION_SOURCE_NONE)
                    {
                        if (source->stream.header || source->stream.byte_len || source->stream.domain)
                            abort();
                    }
                    else if (source->kind == ACTOR_ANIMATION_SOURCE_STREAM)
                    {
                        uintptr_t offset;
                        if ((uintptr_t)source->stream.header < (uintptr_t)resource_storage.data || (uintptr_t)source->stream.header - (uintptr_t)resource_storage.data > RESOURCE_STORAGE_CAPACITY - 4)
                            abort();
                        offset = (uintptr_t)source->stream.header - (uintptr_t)resource_storage.data;
                        if (source->stream.byte_len != RESOURCE_STORAGE_CAPACITY - offset || source->stream.domain < ANIMATION_STREAM_PHYSICAL || source->stream.domain >= ANIMATION_STREAM_DOMAIN_COUNT)
                            abort();
                        words[1] = (uint32)offset;
                    }
                    else
                        abort();
                    if (fwrite(words, sizeof(words), 1, out) != 1)
                        result = 4;
                }
            if (fwrite(animation_flags, sizeof(animation_flags), 1, out) != 1)
                result = 4;
        }
        if (ferror(in) || ferror(out))
            result = 4;
        if (fclose(in))
            result = 4;
        if (fclose(out))
            result = 4;
        resource_archive = (RESOURCE_ARCHIVE){0};
        resource_arenas.persistent = (RESOURCE_CURSOR){0};
        memset(model_refs, 0, sizeof(model_refs));
        memset(animation_banks, 0, sizeof(animation_banks));
        if (!result)
            diagnostic_end();
        return result;
    }
    if (argc > 1 && !strcmp(argv[1], "--audit-actor-assets"))
    {
        static uint8 storage_guard[RESOURCE_STORAGE_CAPACITY];
        static ANIMATION_BANK banks_guard[ANIMATION_BANK_COUNT];
        static MODEL_REFERENCE models_guard[MODEL_TYPE_COUNT];
        const ANIMATION_BANK_ID first_banks[4] = {ANIMATION_BANK_HAWK, ANIMATION_BANK_MACE, ANIMATION_BANK_SMASHER, ANIMATION_BANK_ALANA};
        const uint32 output_header[4] = {0x324e4146, 1, 29, 0};
        FILE *in, *out;
        uint8 *initial_archive = NULL, *archive = NULL, *destination = NULL;
        uint32 header[6], initial[3], job[8], cases = 0, i;
        sint16 starts[TEXTURE_GROUP_COUNT], map_guard[TEXTURE_MAP_COUNT];
        TEXTURE_GROUP groups_guard[TEXTURE_GROUP_COUNT];
        ANIMATION_BANK_ID type_banks_guard[ANIMATION_TYPE_COUNT];
        size_t got = 0;
        int result = 0;
        in = xport_fopen("../status/menu/actor-assets-jobs.bin", "rb");
        out = xport_fopen("../status/menu/actor-assets-native.bin", "wb");
        if (!in || !out)
        {
            if (in)
                fclose(in);
            if (out)
                fclose(out);
            return 4;
        }
        if (fread(header, sizeof(header), 1, in) != 1 || header[0] != 0x324a4146 || header[1] != 1 || header[2] != 59180 || header[3] != 29 || header[4] > RESOURCE_STORAGE_CAPACITY - 59180 || header[5] > RESOURCE_STORAGE_CAPACITY - 16384 || ((header[4] | header[5]) & 3) || !(header[4] + 59180 <= header[5] || header[5] + 16384 <= header[4]) || fread(initial, sizeof(initial), 1, in) != 1 || fread(starts, sizeof(starts), 1, in) != 1 || fread(tex_map, sizeof(tex_map), 1, in) != 1 || fwrite(output_header, sizeof(output_header), 1, out) != 1)
            result = 4;
        if (!result)
        {
            initial_archive = malloc(header[2]);
            if (!initial_archive || fread(initial_archive, 1, header[2], in) != header[2])
                result = 4;
            archive = resource_storage.data + header[4];
            destination = resource_storage.data + header[5];
            for (i = 0; i < TEXTURE_GROUP_COUNT; i++)
                tex_grps[i].first_tex = starts[i];
            for (i = 0; i < 4; i++)
                animation_type_banks[i] = first_banks[i];
            memcpy(groups_guard, tex_grps, sizeof(groups_guard));
            memcpy(map_guard, tex_map, sizeof(map_guard));
            memcpy(type_banks_guard, animation_type_banks, sizeof(type_banks_guard));
        }
        while (!result && (got = fread(job, 1, sizeof(job), in)) == sizeof(job))
        {
            uint32 value, record[3], cursor[3], model_fields[7], heights_guard[MODEL_TYPE_COUNT];
            uint32 job_guard[8];
            ANIMATION_BANK_ID table;
            RESOURCE_ARCHIVE owner_guard;
            RESOURCE_ARENAS arenas_guard;
            RESOURCE_IO io_guard;
            LOADING loading_guard;
            uintptr_t advance;
            if (cases >= header[3] || !memchr(job, 0, 16) || job[4] > 1 || job[7] != initial[2] || (job[4] && (job[5] >= MODEL_TYPE_COUNT || job[6] >= TEXTURE_GROUP_COUNT)) || (!job[4] && job[5] > 3 && job[5] < 0xfffffffd))
            {
                result = 4;
                break;
            }
            table = job[4] ? ANIMATION_BANK_ALANA : job[5] == 0xffffffff ? ANIMATION_BANK_SHARED6 : job[5] == 0xfffffffe ? ANIMATION_BANK_SHARED7 : job[5] == 0xfffffffd ? ANIMATION_BANK_SHARED8 : animation_type_banks[job[5]];
            if (table <= ANIMATION_BANK_NONE || table >= ANIMATION_BANK_COUNT)
                abort();
            memcpy(archive, initial_archive, header[2]);
            memset(destination, 0xa5, 16384);
            memset(animation_banks + table, 0, sizeof(animation_banks[table]));
            memset(animation_flags, 0xa5, sizeof(animation_flags));
            memset(model_refs, 0, sizeof(model_refs));
            memset(model_height, 0xa5, sizeof(model_height));
            resource_archive = (RESOURCE_ARCHIVE){(RESOURCE_DIRECTORY *)archive, header[2], initial[0]};
            resource_arenas.persistent = (RESOURCE_CURSOR){destination, RESOURCE_STORAGE_CAPACITY - header[5], initial[2]};
            loading.total = 0;
            loading.count = initial[1];
            resource_io.result_size = 0xa5a5a5a5;
            memcpy(job_guard, job, sizeof(job_guard));
            memcpy(storage_guard, resource_storage.data, sizeof(storage_guard));
            memcpy(banks_guard, animation_banks, sizeof(banks_guard));
            memcpy(models_guard, model_refs, sizeof(models_guard));
            memcpy(heights_guard, model_height, sizeof(heights_guard));
            memcpy(&owner_guard, &resource_archive, sizeof(owner_guard));
            memcpy(&arenas_guard, &resource_arenas, sizeof(arenas_guard));
            memcpy(&io_guard, &resource_io, sizeof(io_guard));
            memcpy(&loading_guard, &loading, sizeof(loading_guard));
            value = job[4] ? resource_load_character((const char *)job, job[5], job[6]) : resource_load_animations((const char *)job, (sint32)job[5]);
            if ((uintptr_t)resource_arenas.persistent.data < (uintptr_t)destination || (uintptr_t)resource_arenas.persistent.data - (uintptr_t)destination > 16384)
                abort();
            advance = (uintptr_t)resource_arenas.persistent.data - (uintptr_t)destination;
            if (resource_arenas.persistent.context - initial[2] != advance || resource_arenas.persistent.available != RESOURCE_STORAGE_CAPACITY - header[5] - advance)
                abort();
            cursor[0] = (uint32)advance;
            cursor[1] = resource_io.result_size;
            cursor[2] = loading.count;
            if (job[4])
            {
                MODEL_REFERENCE *model = model_refs + job[5];
                uintptr_t head, vertices, faces;
                if (!model->header || !model->vertices || !model->faces || (uintptr_t)model->header < (uintptr_t)destination || (uintptr_t)model->header - (uintptr_t)destination > 16384 || (uintptr_t)model->vertices < (uintptr_t)destination || (uintptr_t)model->vertices - (uintptr_t)destination > 16384 || (uintptr_t)model->faces < (uintptr_t)destination || (uintptr_t)model->faces - (uintptr_t)destination > 16384)
                    abort();
                head = (uintptr_t)model->header - (uintptr_t)destination;
                vertices = (uintptr_t)model->vertices - (uintptr_t)destination;
                faces = (uintptr_t)model->faces - (uintptr_t)destination;
                if (model->byte_len > 16384 - head || model->vertex_byte_len > 16384 - vertices || model->face_byte_len > 16384 - faces)
                    abort();
                model_fields[0] = (uint32)head;
                model_fields[1] = model->byte_len;
                model_fields[2] = (uint32)vertices;
                model_fields[3] = model->vertex_byte_len;
                model_fields[4] = (uint32)faces;
                model_fields[5] = model->face_byte_len;
                model_fields[6] = model_height[job[5]];
                memcpy(models_guard + job[5], model, sizeof(*model));
                heights_guard[job[5]] = model_height[job[5]];
            }
            memcpy(banks_guard + table, animation_banks + table, sizeof(animation_banks[table]));
            memcpy(storage_guard + header[4], archive, header[2]);
            memcpy(storage_guard + header[5], destination, 16384);
            memcpy(&arenas_guard.persistent, &resource_arenas.persistent, sizeof(arenas_guard.persistent));
            io_guard.result_size = resource_io.result_size;
            loading_guard.count = loading.count;
            if (memcmp(job_guard, job, sizeof(job_guard)) || memcmp(storage_guard, resource_storage.data, sizeof(storage_guard)) || memcmp(banks_guard, animation_banks, sizeof(banks_guard)) || memcmp(models_guard, model_refs, sizeof(models_guard)) || memcmp(heights_guard, model_height, sizeof(heights_guard)) || memcmp(type_banks_guard, animation_type_banks, sizeof(type_banks_guard)) || memcmp(groups_guard, tex_grps, sizeof(groups_guard)) || memcmp(map_guard, tex_map, sizeof(map_guard)) || memcmp(&owner_guard, &resource_archive, sizeof(owner_guard)) || memcmp(&arenas_guard, &resource_arenas, sizeof(arenas_guard)) || memcmp(&io_guard, &resource_io, sizeof(io_guard)) || memcmp(&loading_guard, &loading, sizeof(loading_guard)))
                abort();
            record[0] = job[4];
            record[1] = job[5];
            record[2] = job[4] ? 80822 : 80794;
            if (fwrite(record, sizeof(record), 1, out) != 1 || fwrite(&value, sizeof(value), 1, out) != 1 || fwrite(archive, 1, header[2], out) != header[2] || fwrite(destination, 16384, 1, out) != 1)
                result = 4;
            for (i = 0; i < ANIMATION_COUNT; i++)
            {
                ACTOR_ANIMATION_SOURCE *source = animation_banks[table].stream + i;
                uint32 words[4] = {source->kind, 0xffffffff, source->stream.byte_len, source->stream.domain};
                if (source->kind == ACTOR_ANIMATION_SOURCE_NONE)
                {
                    if (source->stream.header || source->stream.byte_len || source->stream.domain != ANIMATION_STREAM_NATIVE)
                        abort();
                }
                else if (source->kind == ACTOR_ANIMATION_SOURCE_STREAM)
                {
                    uintptr_t offset;
                    if ((uintptr_t)source->stream.header < (uintptr_t)archive || (uintptr_t)source->stream.header - (uintptr_t)archive > header[2] - 4)
                        abort();
                    offset = (uintptr_t)source->stream.header - (uintptr_t)archive;
                    if (source->stream.byte_len != RESOURCE_STORAGE_CAPACITY - header[4] - offset || source->stream.domain != ANIMATION_STREAM_CACHED)
                        abort();
                    words[1] = (uint32)offset;
                }
                else
                    abort();
                if (fwrite(words, sizeof(words), 1, out) != 1)
                    result = 4;
            }
            if (fwrite(animation_flags, sizeof(animation_flags), 1, out) != 1 || (job[4] && fwrite(model_fields, sizeof(model_fields), 1, out) != 1) || fwrite(cursor, sizeof(cursor), 1, out) != 1)
                result = 4;
            ++cases;
        }
        if (ferror(in) || (!result && (got || cases != header[3])))
            result = 4;
        if (fclose(in))
            result = 4;
        if (fclose(out))
            result = 4;
        resource_archive = (RESOURCE_ARCHIVE){0};
        resource_arenas.persistent = (RESOURCE_CURSOR){0};
        memset(model_refs, 0, sizeof(model_refs));
        memset(animation_banks, 0, sizeof(animation_banks));
        free(initial_archive);
        if (!result)
            diagnostic_end();
        return result;
    }
    if (argc > 1 && !strcmp(argv[1], "--audit-model-load"))
    {
        const uint32 expected_header[4] = {0x324a4c46, 1, 59180, 23};
        const uint32 output_header[4] = {0x324e4c46, 1, 59220, 23};
        static MODEL_REFERENCE refs_guard[MODEL_TYPE_COUNT];
        FILE *in, *out;
        uint8 *archive = NULL, *initial_archive = NULL;
        uint32 header[4], initial[2], job[7], cases = 0, i;
        sint16 starts[TEXTURE_GROUP_COUNT], map_guard[TEXTURE_MAP_COUNT];
        TEXTURE_GROUP groups_guard[TEXTURE_GROUP_COUNT];
        size_t got = 0;
        int result = 0;
        in = xport_fopen("../status/menu/model-load-jobs.bin", "rb");
        out = xport_fopen("../status/menu/model-load-native.bin", "wb");
        if (!in || !out)
        {
            if (in)
                fclose(in);
            if (out)
                fclose(out);
            return 4;
        }
        if (fread(header, sizeof(header), 1, in) != 1 || memcmp(header, expected_header, sizeof(header)) || fread(initial, sizeof(initial), 1, in) != 1 || fread(starts, sizeof(starts), 1, in) != 1 || fread(tex_map, sizeof(tex_map), 1, in) != 1 || fwrite(output_header, sizeof(output_header), 1, out) != 1)
            result = 4;
        if (!result)
        {
            archive = malloc(header[2]);
            initial_archive = malloc(header[2]);
            if (!archive || !initial_archive || fread(initial_archive, 1, header[2], in) != header[2])
                result = 4;
            for (i = 0; i < TEXTURE_GROUP_COUNT; i++)
                tex_grps[i].first_tex = starts[i];
            memcpy(groups_guard, tex_grps, sizeof(groups_guard));
            memcpy(map_guard, tex_map, sizeof(map_guard));
        }
        while (!result && (got = fread(job, 1, sizeof(job), in)) == sizeof(job))
        {
            uint32 value, fields[9], heights_guard[MODEL_TYPE_COUNT];
            uint32 job_guard[7];
            RESOURCE_ARCHIVE owner_guard;
            RESOURCE_IO io_guard;
            LOADING loading_guard;
            MODEL_REFERENCE *model;
            uintptr_t head, vertices, faces;
            if (cases >= header[3] || !memchr(job, 0, 16) || job[4] >= MODEL_TYPE_COUNT || job[5] >= TEXTURE_GROUP_COUNT || job[6] > 1)
            {
                result = 4;
                break;
            }
            memcpy(archive, initial_archive, header[2]);
            resource_archive = (RESOURCE_ARCHIVE){(RESOURCE_DIRECTORY *)archive, header[2], initial[0]};
            memset(model_refs, 0, sizeof(model_refs));
            memset(model_height, 0xa5, sizeof(model_height));
            loading.total = 0;
            loading.count = initial[1];
            resource_io.result_size = 0xa5a5a5a5;
            memcpy(job_guard, job, sizeof(job_guard));
            memcpy(refs_guard, model_refs, sizeof(refs_guard));
            memcpy(heights_guard, model_height, sizeof(heights_guard));
            memcpy(&owner_guard, &resource_archive, sizeof(owner_guard));
            memcpy(&io_guard, &resource_io, sizeof(io_guard));
            memcpy(&loading_guard, &loading, sizeof(loading_guard));
            value = resource_load_model((const char *)job, job[4], job[5]);
            model = model_refs + job[4];
            if (!model->header || !model->vertices || !model->faces || (uintptr_t)model->header < (uintptr_t)archive || (uintptr_t)model->header - (uintptr_t)archive > header[2] || (uintptr_t)model->vertices < (uintptr_t)archive || (uintptr_t)model->vertices - (uintptr_t)archive > header[2] || (uintptr_t)model->faces < (uintptr_t)archive || (uintptr_t)model->faces - (uintptr_t)archive > header[2])
                abort();
            head = (uintptr_t)model->header - (uintptr_t)archive;
            vertices = (uintptr_t)model->vertices - (uintptr_t)archive;
            faces = (uintptr_t)model->faces - (uintptr_t)archive;
            if (model->byte_len > header[2] - head || model->vertex_byte_len > header[2] - vertices || model->face_byte_len > header[2] - faces)
                abort();
            fields[0] = (uint32)head;
            fields[1] = model->byte_len;
            fields[2] = (uint32)vertices;
            fields[3] = model->vertex_byte_len;
            fields[4] = (uint32)faces;
            fields[5] = model->face_byte_len;
            fields[6] = model_height[job[4]];
            fields[7] = resource_io.result_size;
            fields[8] = loading.count;
            memcpy(refs_guard + job[4], model, sizeof(*model));
            heights_guard[job[4]] = model_height[job[4]];
            io_guard.result_size = resource_io.result_size;
            loading_guard.count = loading.count;
            if (memcmp(job_guard, job, sizeof(job_guard)) || memcmp(refs_guard, model_refs, sizeof(refs_guard)) || memcmp(heights_guard, model_height, sizeof(heights_guard)) || memcmp(groups_guard, tex_grps, sizeof(groups_guard)) || memcmp(map_guard, tex_map, sizeof(map_guard)) || memcmp(&owner_guard, &resource_archive, sizeof(owner_guard)) || memcmp(&io_guard, &resource_io, sizeof(io_guard)) || memcmp(&loading_guard, &loading, sizeof(loading_guard)))
                abort();
            if (fwrite(&value, sizeof(value), 1, out) != 1 || fwrite(archive, 1, header[2], out) != header[2] || fwrite(fields, sizeof(fields), 1, out) != 1)
                result = 4;
            ++cases;
        }
        if (ferror(in) || (!result && (got || cases != header[3])))
            result = 4;
        if (fclose(in))
            result = 4;
        if (fclose(out))
            result = 4;
        resource_archive = (RESOURCE_ARCHIVE){0};
        memset(model_refs, 0, sizeof(model_refs));
        free(initial_archive);
        free(archive);
        if (!result)
            diagnostic_end();
        return result;
    }
    if (argc > 1 && !strcmp(argv[1], "--audit-stage-reset"))
    {
        const uint32 input_header[4] = {0x324a5346, 1, 44, 27};
        const uint32 output_header[4] = {0x324e5346, 1, 1208, 27};
        uint32 header[4], job[44], i, value, case_count = 0;
        FILE *in, *out;
        size_t got = 0;
        int result = 0;
        in = xport_fopen("../status/menu/stage-reset-jobs.bin", "rb");
        out = xport_fopen("../status/menu/stage-reset-native.bin", "wb");
        if (!in || !out)
        {
            if (in)
                fclose(in);
            if (out)
                fclose(out);
            return 4;
        }
        if (fread(header, sizeof(header), 1, in) != 1 || memcmp(header, input_header, sizeof(header)) || fwrite(output_header, sizeof(output_header), 1, out) != 1)
            result = 4;
        while (!result && (got = fread(job, 1, sizeof(job), in)) == sizeof(job))
        {
            LEVEL_TRANSITION level_transition_guard;
            LEVEL_WAVE level_wave_guard;
            ACTOR_COLLISION actor_col_guard;
            CAMERA camera_guard;
            DISPLAY_BUFFERS disp_buffers_guard;
            GAME_SESSION game_session_guard;
            LEVEL_SCRIPT_SCENE level_script_scene_guard;
            STAGE_ANIMATION stage_animation_guard;
            LIFT lift_guard;
            NPC_BEHAVIOR_CYCLE npc_behavior_cycle_guard;
            ITEM_PICKUP item_pickup_guard;
            COLLISION_REGIONS col_regions_guard;
            ACTOR_HEALTH health_guard[ACTOR_COUNT];
            ACTOR_LIFECYCLE lifecycle_guard[ACTOR_COUNT];
            EFFECT effects_guard[EFFECT_COUNT];
            MODEL_OBJECT dynamic_guard[DYNAMIC_OBJECT_COUNT];
            uint32 starts_guard[LEVEL_SCRIPT_STAGE_COUNT];
            sint16 types_guard[ACTOR_COUNT];
            uint32 fields[43];
            if (case_count >= 27 || job[0] != case_count || (job[1] != LEVEL_SCRIPT_NONE && job[1] >= LEVEL_SCRIPT_WORD_COUNT) || job[2] > 65535 || job[3] > 65535)
            {
                result = 4;
                break;
            }
            level_transition.restart_timer = (uint32)0xa5a5a5a5;
            level_transition.continue_choice = (sint8)0xa5;
            level_wave.population_gate = (uint32)0xa5a5a5a5;
            level_wave.spawn_delay = (uint32)0xa5a5a5a5;
            level_wave.pending_spawns = (uint32)0xa5a5a5a5;
            level_wave.remaining_enemies = (uint32)0xa5a5a5a5;
            actor_col.reaction_deadline = (uint32)0xa5a5a5a5;
            camera.tracking.visibility_mask = (uint32)0xa5a5a5a5;
            camera.tracking.hold_bits = (uint32)0xa5a5a5a5;
            level_transition.continue_phase = (uint32)0xa5a5a5a5;
            game_session.tick = (uint32)0xa5a5a5a5;
            camera.tracking.pitch_control_bits = (uint32)0xa5a5a5a5;
            camera.angles.pitch_bits = (uint16)0xa5a5;
            disp_buffers.pulse_active = (sint8)0xa5;
            game_pause_flags = (sint8)0xa5;
            level_transition.route_exit_tick = (uint32)0xa5a5a5a5;
            disp_buffers.pulse_phase = (uint32)0xa5a5a5a5;
            level_script_scene.relative = (sint8)0xa5;
            level_script_scene.remaining_landings = (uint32)0xa5a5a5a5;
            level_script_scene.oz = (sint32)0xa5a5a5a5;
            level_script_scene.ox = (sint32)0xa5a5a5a5;
            stage_animation.phase[0] = (uint32)0xa5a5a5a5;
            stage_animation.hold_ticks = (uint32)0xa5a5a5a5;
            lift.travel = (uint32)0xa5a5a5a5;
            lift.pos = (uint32)0xa5a5a5a5;
            lift.speed = (uint32)0xa5a5a5a5;
            lift.cycle = (uint32)0xa5a5a5a5;
            lift.stop_phase = (uint32)0xa5a5a5a5;
            game_session.scene_phase = (uint32)0xa5a5a5a5;
            level_transition.exit_requested = (sint8)0xa5;
            level_transition.finish_tick = (uint32)0xa5a5a5a5;
            level_wave.paused = (sint8)0xa5;
            npc_behavior_cycle.progress = (uint32)0xa5a5a5a5;
            npc_behavior_cycle.advance = (uint32)0xa5a5a5a5;
            item_pickup.nearby[0] = (uint32)0xa5a5a5a5;
            item_pickup.nearby[1] = (uint32)0xa5a5a5a5;
            stage_animation.lift_auto_cycle = (uint8)0xa5;
            actor_ambient_shade = (uint32)0xa5a5a5a5;
            game_session.stage = job[0];
            level_script_starts[job[0]] = job[1];
            level_script_cursor = LEVEL_SCRIPT_NONE;
            camera.angles.yaw_bits = (uint16)job[2];
            camera.angles.pitch_bits = (uint16)job[3];
            col_regions.sel = col_regions.data;
            level_script_scene.object = (SCENE_OBJECT_REFERENCE){0, SCENE_OBJECT_DYNAMIC};
            memset(&action_history, 0xa5, sizeof(action_history));
            memset(stage_audio_slots, 0xa5, sizeof(stage_audio_slots));
            memset(effects, 0, sizeof(effects));
            memset(dynamic_objects, 0, sizeof(dynamic_objects));
            for (i = 0; i < EFFECT_COUNT; i++)
                effects[i].kind = 0xa5a5a5a5;
            for (i = 0; i < DYNAMIC_OBJECT_COUNT; i++)
                dynamic_objects[i].kind = 0xa5a5a5a5;
            for (i = 0; i < ACTOR_COUNT; i++)
            {
                actor_health[i].hp = (sint16)((sint32)i - 4);
                actor_types[i] = (sint16)(uint16)job[4 + 5 * i];
                actor_lighting[i].bits = job[5 + 5 * i];
                actor_lifecycle[i].death_ticks = (uint8)job[6 + 5 * i];
                actor_companions[i] = (sint16)(uint16)job[7 + 5 * i];
                actor_bonus_counts[i] = (uint16)job[8 + 5 * i];
            }
            memcpy(&level_transition_guard, &level_transition, sizeof(level_transition_guard));
            memcpy(&level_wave_guard, &level_wave, sizeof(level_wave_guard));
            memcpy(&actor_col_guard, &actor_col, sizeof(actor_col_guard));
            memcpy(&camera_guard, &camera, sizeof(camera_guard));
            memcpy(&disp_buffers_guard, &disp_buffers, sizeof(disp_buffers_guard));
            memcpy(&game_session_guard, &game_session, sizeof(game_session_guard));
            memcpy(&level_script_scene_guard, &level_script_scene, sizeof(level_script_scene_guard));
            memcpy(&stage_animation_guard, &stage_animation, sizeof(stage_animation_guard));
            memcpy(&lift_guard, &lift, sizeof(lift_guard));
            memcpy(&npc_behavior_cycle_guard, &npc_behavior_cycle, sizeof(npc_behavior_cycle_guard));
            memcpy(&item_pickup_guard, &item_pickup, sizeof(item_pickup_guard));
            memcpy(&col_regions_guard, &col_regions, sizeof(col_regions_guard));
            memcpy(health_guard, actor_health, sizeof(health_guard));
            memcpy(lifecycle_guard, actor_lifecycle, sizeof(lifecycle_guard));
            memcpy(types_guard, actor_types, sizeof(types_guard));
            memcpy(effects_guard, effects, sizeof(effects_guard));
            memcpy(dynamic_guard, dynamic_objects, sizeof(dynamic_guard));
            memcpy(starts_guard, level_script_starts, sizeof(starts_guard));
            value = action_history_reset() == action_history.predicted;
            level_reset();
            level_transition_guard.restart_timer = level_transition.restart_timer;
            level_transition_guard.continue_choice = level_transition.continue_choice;
            level_wave_guard.population_gate = level_wave.population_gate;
            level_wave_guard.spawn_delay = level_wave.spawn_delay;
            level_wave_guard.pending_spawns = level_wave.pending_spawns;
            level_wave_guard.remaining_enemies = level_wave.remaining_enemies;
            actor_col_guard.reaction_deadline = actor_col.reaction_deadline;
            camera_guard.tracking.visibility_mask = camera.tracking.visibility_mask;
            camera_guard.tracking.hold_bits = camera.tracking.hold_bits;
            level_transition_guard.continue_phase = level_transition.continue_phase;
            game_session_guard.tick = game_session.tick;
            camera_guard.tracking.pitch_control_bits = camera.tracking.pitch_control_bits;
            camera_guard.angles.pitch_bits = camera.angles.pitch_bits;
            disp_buffers_guard.pulse_active = disp_buffers.pulse_active;
            level_transition_guard.route_exit_tick = level_transition.route_exit_tick;
            disp_buffers_guard.pulse_phase = disp_buffers.pulse_phase;
            level_script_scene_guard.relative = level_script_scene.relative;
            level_script_scene_guard.remaining_landings = level_script_scene.remaining_landings;
            level_script_scene_guard.oz = level_script_scene.oz;
            level_script_scene_guard.ox = level_script_scene.ox;
            stage_animation_guard.phase[0] = stage_animation.phase[0];
            stage_animation_guard.hold_ticks = stage_animation.hold_ticks;
            lift_guard.travel = lift.travel;
            lift_guard.pos = lift.pos;
            lift_guard.speed = lift.speed;
            lift_guard.cycle = lift.cycle;
            lift_guard.stop_phase = lift.stop_phase;
            game_session_guard.scene_phase = game_session.scene_phase;
            level_transition_guard.exit_requested = level_transition.exit_requested;
            level_transition_guard.finish_tick = level_transition.finish_tick;
            level_wave_guard.paused = level_wave.paused;
            npc_behavior_cycle_guard.progress = npc_behavior_cycle.progress;
            npc_behavior_cycle_guard.advance = npc_behavior_cycle.advance;
            item_pickup_guard.nearby[0] = item_pickup.nearby[0];
            item_pickup_guard.nearby[1] = item_pickup.nearby[1];
            stage_animation_guard.lift_auto_cycle = stage_animation.lift_auto_cycle;
            col_regions_guard.sel = col_regions.sel;
            memcpy(&level_script_scene_guard.object, &level_script_scene.object, sizeof(level_script_scene_guard.object));
            for (i = 0; i < ACTOR_COUNT; i++)
                lifecycle_guard[i].death_ticks = actor_lifecycle[i].death_ticks;
            for (i = 0; i < EFFECT_COUNT; i++)
                effects_guard[i].kind = effects[i].kind;
            for (i = 0; i < DYNAMIC_OBJECT_COUNT; i++)
                dynamic_guard[i].kind = dynamic_objects[i].kind;
            if (memcmp(&level_transition_guard, &level_transition, sizeof(level_transition_guard)) || memcmp(&level_wave_guard, &level_wave, sizeof(level_wave_guard)) || memcmp(&actor_col_guard, &actor_col, sizeof(actor_col_guard)) || memcmp(&camera_guard, &camera, sizeof(camera_guard)) || memcmp(&disp_buffers_guard, &disp_buffers, sizeof(disp_buffers_guard)) || memcmp(&game_session_guard, &game_session, sizeof(game_session_guard)) || memcmp(&level_script_scene_guard, &level_script_scene, sizeof(level_script_scene_guard)) || memcmp(&stage_animation_guard, &stage_animation, sizeof(stage_animation_guard)) || memcmp(&lift_guard, &lift, sizeof(lift_guard)) || memcmp(&npc_behavior_cycle_guard, &npc_behavior_cycle, sizeof(npc_behavior_cycle_guard)) || memcmp(&item_pickup_guard, &item_pickup, sizeof(item_pickup_guard)) || memcmp(&col_regions_guard, &col_regions, sizeof(col_regions_guard)) || memcmp(health_guard, actor_health, sizeof(health_guard)) ||
                memcmp(lifecycle_guard, actor_lifecycle, sizeof(lifecycle_guard)) || memcmp(types_guard, actor_types, sizeof(types_guard)) || memcmp(effects_guard, effects, sizeof(effects_guard)) || memcmp(dynamic_guard, dynamic_objects, sizeof(dynamic_guard)) || memcmp(starts_guard, level_script_starts, sizeof(starts_guard)))
                abort();
            fields[0] = value;
            fields[1] = (uint32)level_transition.restart_timer;
            fields[2] = (uint8)level_transition.continue_choice;
            fields[3] = (uint32)level_wave.population_gate;
            fields[4] = (uint32)level_wave.spawn_delay;
            fields[5] = (uint32)level_wave.pending_spawns;
            fields[6] = (uint32)level_wave.remaining_enemies;
            fields[7] = (uint32)actor_col.reaction_deadline;
            fields[8] = (uint32)camera.tracking.visibility_mask;
            fields[9] = (uint32)camera.tracking.hold_bits;
            fields[10] = (uint32)level_transition.continue_phase;
            fields[11] = (uint32)game_session.tick;
            fields[12] = (uint32)camera.tracking.pitch_control_bits;
            fields[13] = (uint16)camera.angles.pitch_bits;
            fields[14] = (uint8)disp_buffers.pulse_active;
            fields[15] = (uint8)game_pause_flags;
            fields[16] = (uint32)level_transition.route_exit_tick;
            fields[17] = (uint32)disp_buffers.pulse_phase;
            fields[18] = (uint8)level_script_scene.relative;
            fields[19] = (uint32)level_script_scene.remaining_landings;
            fields[20] = (uint32)level_script_scene.oz;
            fields[21] = (uint32)level_script_scene.ox;
            fields[22] = (uint32)stage_animation.phase[0];
            fields[23] = (uint32)stage_animation.hold_ticks;
            fields[24] = (uint32)lift.travel;
            fields[25] = (uint32)lift.pos;
            fields[26] = (uint32)lift.speed;
            fields[27] = (uint32)lift.cycle;
            fields[28] = (uint32)lift.stop_phase;
            fields[29] = (uint32)game_session.scene_phase;
            fields[30] = (uint8)level_transition.exit_requested;
            fields[31] = (uint32)level_transition.finish_tick;
            fields[32] = (uint8)level_wave.paused;
            fields[33] = (uint32)npc_behavior_cycle.progress;
            fields[34] = (uint32)npc_behavior_cycle.advance;
            fields[35] = (uint32)item_pickup.nearby[0];
            fields[36] = (uint32)item_pickup.nearby[1];
            fields[37] = (uint8)stage_animation.lift_auto_cycle;
            fields[38] = (uint32)actor_ambient_shade;
            fields[39] = col_regions.sel != NULL;
            fields[40] = level_script_cursor;
            fields[41] = level_script_scene.object.pool;
            fields[42] = level_script_scene.object.idx;
            if (fwrite(fields, sizeof(fields), 1, out) != 1 || fwrite(action_history.action, sizeof(action_history.action), 1, out) != 1 || fwrite(action_history.predicted, sizeof(action_history.predicted), 1, out) != 1 || fwrite(action_history.matched, sizeof(action_history.matched), 1, out) != 1)
                result = 4;
            for (i = 0; i < ACTOR_COUNT; i++)
            {
                uint32 actor_fields[6] = {(uint16)actor_types[i], (uint16)actor_health[i].hp, actor_lighting[i].bits, actor_lifecycle[i].death_ticks, (uint16)actor_companions[i], actor_bonus_counts[i]};
                if (fwrite(actor_fields, sizeof(actor_fields), 1, out) != 1)
                    result = 4;
            }
            for (i = 0; i < EFFECT_COUNT; i++)
                if (fwrite(&effects[i].kind, sizeof(effects[i].kind), 1, out) != 1)
                    result = 4;
            for (i = 0; i < DYNAMIC_OBJECT_COUNT; i++)
                if (fwrite(&dynamic_objects[i].kind, sizeof(dynamic_objects[i].kind), 1, out) != 1)
                    result = 4;
            for (i = 0; i < STAGE_AUDIO_SLOT_COUNT; i++)
                if (fwrite(&stage_audio_slots[i].voice, sizeof(stage_audio_slots[i].voice), 1, out) != 1)
                    result = 4;
            value = camera.angles.yaw_bits;
            if (fwrite(&value, sizeof(value), 1, out) != 1 || fwrite(&game_session.stage, sizeof(game_session.stage), 1, out) != 1)
                result = 4;
            ++case_count;
        }
        if (ferror(in) || (!result && got) || case_count != 27)
            result = 4;
        if (fclose(in))
            result = 4;
        if (fclose(out))
            result = 4;
        if (!result)
            diagnostic_end();
        return result;
    }
    if (argc > 1 && !strcmp(argv[1], "--audit-startup-reset"))
    {
        const uint32 input_header[4] = {0x324a5546, 1, 72, 4};
        const uint32 output_header[4] = {0x324e5546, 1, 23364, 72};
        const ANIMATION_BANK_ID banks[4] = {ANIMATION_BANK_HAWK, ANIMATION_BANK_MACE, ANIMATION_BANK_SMASHER, ANIMATION_BANK_ALANA};
        static ANIMATION_BANK banks_guard[ANIMATION_BANK_COUNT];
        uint32 header[4], initial[21], a[4], i, j, value, cases = 0;
        uint8 rows[80];
        FILE *in, *out;
        size_t got = 0;
        int result = 0;
        in = xport_fopen("../status/menu/startup-reset-jobs.bin", "rb");
        out = xport_fopen("../status/menu/startup-reset-native.bin", "wb");
        if (!in || !out)
        {
            if (in)
                fclose(in);
            if (out)
                fclose(out);
            return 4;
        }
        if (fread(header, sizeof(header), 1, in) != 1 || memcmp(header, input_header, sizeof(header)) || fread(initial, sizeof(initial), 1, in) != 1 || fread(rows, sizeof(rows), 1, in) != 1 || fwrite(output_header, sizeof(output_header), 1, out) != 1)
            result = 4;
        for (i = 0; !result && i < 13; i++)
            if (initial[i] >= CHARACTER_NAME_COUNT && initial[i] != 0xffffffff)
                result = 4;
        while (!result && (got = fread(a, 1, sizeof(a), in)) == sizeof(a))
        {
            ACTOR_STATUS actor_status_guard[ACTOR_COUNT];
            ACTOR_ANIMATION actor_animations_guard[ACTOR_COUNT];
            ACTOR_HEALTH actor_health_guard[ACTOR_COUNT];
            ACTOR_CONTACT actor_contacts_guard[ACTOR_COUNT];
            ACTOR_LIFECYCLE actor_lifecycle_guard[ACTOR_COUNT];
            NPC_BEHAVIOR_ACTOR npc_behavior_actors_guard[ACTOR_COUNT];
            ACTOR_POSE trail_guard[2];
            NPC_BEHAVIOR_ROW rows_guard[NPC_BEHAVIOR_ROW_COUNT];
            RESOURCE_ARENAS arenas_guard;
            GAME_SESSION session_guard;
            CHARACTER_NAME_ID names_guard[CHARACTER_NAME_ACTOR_COUNT];
            CHARACTER_NAME_ID type_names_guard[CHARACTER_NAME_TYPE_COUNT];
            if (cases >= 72 || a[2] < 1 || a[2] > 2 || ((uint16)a[0] != 65535 && (uint16)a[0] > 4) || ((uint16)a[1] != 65535 && (uint16)a[1] > 4))
            {
                result = 4;
                break;
            }
            for (i = 0; i < 5; i++)
                character_name_types[i] = (CHARACTER_NAME_ID)initial[i];
            for (i = 0; i < 10; i++)
            {
                NPC_BEHAVIOR_ROW *row = npc_behavior_rows + NPC_BEHAVIOR_NPC_PROFILE_COUNT * NPC_BEHAVIOR_ROWS_PER_PROFILE + i;
                row->capability = rows[8 * i];
                row->hit_delay = rows[8 * i + 1];
                row->recovery_delay = rows[8 * i + 2];
                row->reaction_threshold = rows[8 * i + 3];
                row->pickup_bias = rows[8 * i + 4];
                row->damage_out = rows[8 * i + 5];
                row->health_units = rows[8 * i + 6];
                row->damage_in = rows[8 * i + 7];
            }
            memset(actor_status, 0xa5, sizeof(actor_status));
            memset(actor_health, 0xa5, sizeof(actor_health));
            memset(actor_contacts, 0xa5, sizeof(actor_contacts));
            memset(actor_lifecycle, 0xa5, sizeof(actor_lifecycle));
            memset(actor_items, 0xa5, sizeof(actor_items));
            memset(actor_orientation, 0xa5, sizeof(actor_orientation));
            memset(actor_poses, 0xa5, sizeof(actor_poses));
            for (i = 0; i < ACTOR_COUNT; i++)
            {
                actor_types[i] = i < 2 ? (sint16)(uint16)a[i] : 0;
                actor_health[i].hp = (sint16)(uint16)(a[3] + i);
                actor_animations[i].src = (ACTOR_ANIMATION_SOURCE){ACTOR_ANIMATION_SOURCE_PROCEDURAL, {0}};
                actor_animations[i].frame = 0xa5a5a5a5;
                actor_animations[i].selector = 0xa5a5a5a5;
                actor_animations[i].id = 0xa5a5a5a5;
                npc_behavior_actors[i].profile = (NPC_BEHAVIOR_REFERENCE){npc_behavior_rows, NPC_BEHAVIOR_ROW_COUNT, 0};
                npc_behavior_actors[i].behavior = initial[13 + i];
                character_actor_names[i] = (CHARACTER_NAME_ID)initial[5 + i];
            }
            for (i = 0; i < 4; i++)
                for (j = 0; j < ANIMATION_COUNT; j++)
                    animation_banks[banks[i]].stream[j] = (ACTOR_ANIMATION_SOURCE){ACTOR_ANIMATION_SOURCE_PROCEDURAL, {0}};
            game_session.player_count = a[2];
            resource_arenas.scratch = (RESOURCE_CURSOR){resource_storage.data, RESOURCE_STORAGE_CAPACITY, RESOURCE_STORAGE_ORIGIN};
            memcpy(actor_status_guard, actor_status, sizeof(actor_status_guard));
            memcpy(actor_animations_guard, actor_animations, sizeof(actor_animations_guard));
            memcpy(actor_health_guard, actor_health, sizeof(actor_health_guard));
            memcpy(actor_contacts_guard, actor_contacts, sizeof(actor_contacts_guard));
            memcpy(actor_lifecycle_guard, actor_lifecycle, sizeof(actor_lifecycle_guard));
            memcpy(npc_behavior_actors_guard, npc_behavior_actors, sizeof(npc_behavior_actors_guard));
            memcpy(trail_guard, actor_poses + ACTOR_POSE_TRAIL0, sizeof(trail_guard));
            memcpy(rows_guard, npc_behavior_rows, sizeof(rows_guard));
            memcpy(banks_guard, animation_banks, sizeof(banks_guard));
            memcpy(names_guard, character_actor_names, sizeof(names_guard));
            memcpy(type_names_guard, character_name_types, sizeof(type_names_guard));
            memcpy(&arenas_guard, &resource_arenas, sizeof(arenas_guard));
            memcpy(&session_guard, &game_session, sizeof(session_guard));
            value = character_resources_begin();
            if (fwrite(&value, sizeof(value), 1, out) != 1)
                result = 4;
            value = actors_reset();
            if (fwrite(&value, sizeof(value), 1, out) != 1)
                result = 4;
            for (i = 0; i < ACTOR_COUNT; i++)
            {
                actor_status_guard[i].flags = actor_status[i].flags;
                actor_animations_guard[i].frame = actor_animations[i].frame;
                actor_animations_guard[i].selector = actor_animations[i].selector;
                actor_animations_guard[i].id = actor_animations[i].id;
                actor_health_guard[i].stun = actor_health[i].stun;
                actor_contacts_guard[i].target_ticks = actor_contacts[i].target_ticks;
                actor_contacts_guard[i].last_target = actor_contacts[i].last_target;
                actor_contacts_guard[i].captor = actor_contacts[i].captor;
                actor_lifecycle_guard[i].death_ticks = actor_lifecycle[i].death_ticks;
                memcpy(&actor_animations_guard[i].src, &actor_animations[i].src, sizeof(actor_animations_guard[i].src));
                if (i < a[2] && actor_types[i] != ACTOR_TYPE_NONE)
                {
                    uint32 first = (NPC_BEHAVIOR_NPC_PROFILE_COUNT + i) * NPC_BEHAVIOR_ROWS_PER_PROFILE;
                    if (npc_behavior_actors[i].profile.rows != npc_behavior_rows || npc_behavior_actors[i].profile.row_count != NPC_BEHAVIOR_ROW_COUNT || npc_behavior_actors[i].profile.first != first)
                        abort();
                    memcpy(&npc_behavior_actors_guard[i].profile, &npc_behavior_actors[i].profile, sizeof(npc_behavior_actors_guard[i].profile));
                    npc_behavior_actors_guard[i].behavior = npc_behavior_actors[i].behavior;
                    rows_guard[first].damage_out = npc_behavior_rows[first].damage_out;
                    rows_guard[first].damage_in = npc_behavior_rows[first].damage_in;
                    names_guard[i] = character_actor_names[i];
                }
            }
            for (i = 0; i < 4; i++)
                for (j = 0; j < ANIMATION_COUNT; j++)
                {
                    ACTOR_ANIMATION_SOURCE *source = animation_banks[banks[i]].stream + j;
                    uint32 words[4] = {source->kind, source->stream.header != NULL, source->stream.byte_len, source->stream.domain};
                    memcpy(banks_guard[banks[i]].stream + j, source, sizeof(*source));
                    if (fwrite(words, sizeof(words), 1, out) != 1)
                        result = 4;
                }
            memcpy(&arenas_guard.scratch, &resource_arenas.scratch, sizeof(arenas_guard.scratch));
            if (memcmp(actor_status_guard, actor_status, sizeof(actor_status_guard)) || memcmp(actor_animations_guard, actor_animations, sizeof(actor_animations_guard)) || memcmp(actor_health_guard, actor_health, sizeof(actor_health_guard)) || memcmp(actor_contacts_guard, actor_contacts, sizeof(actor_contacts_guard)) || memcmp(actor_lifecycle_guard, actor_lifecycle, sizeof(actor_lifecycle_guard)) || memcmp(npc_behavior_actors_guard, npc_behavior_actors, sizeof(npc_behavior_actors_guard)) || memcmp(trail_guard, actor_poses + ACTOR_POSE_TRAIL0, sizeof(trail_guard)) || memcmp(rows_guard, npc_behavior_rows, sizeof(rows_guard)) || memcmp(banks_guard, animation_banks, sizeof(banks_guard)) || memcmp(names_guard, character_actor_names, sizeof(names_guard)) || memcmp(type_names_guard, character_name_types, sizeof(type_names_guard)) || memcmp(&arenas_guard, &resource_arenas, sizeof(arenas_guard)) || memcmp(&session_guard, &game_session, sizeof(session_guard)))
                abort();
            for (i = 0; i < ACTOR_POSE_ACTOR_COUNT; i++)
            {
                ACTOR_POSE *pose = actor_poses + i;
                if (fwrite(&pose->sway_yaw, sizeof(pose->sway_yaw), 1, out) != 1)
                    result = 4;
                for (j = 0; j < ACTOR_POSE_MATRIX_COUNT; j++)
                {
                    uint32 words[8];
                    geometry_audit_store(pose->matrix + j, words);
                    if (fwrite(words, sizeof(words), 1, out) != 1)
                        result = 4;
                }
                for (j = 0; j < ACTOR_POSE_POINT_COUNT; j++)
                    if (fwrite(pose->point[j].bits, sizeof(pose->point[j].bits), 1, out) != 1)
                        result = 4;
                for (j = 0; j < ACTOR_POSE_BONE_COUNT; j++)
                    if (fwrite(pose->angles[j].v, sizeof(pose->angles[j].v), 1, out) != 1)
                        result = 4;
            }
            for (i = 0; i < ACTOR_COUNT; i++)
            {
                ACTOR_ANIMATION_SOURCE *source = &actor_animations[i].src;
                uint32 actor_fields[23];
                actor_fields[0] = source->kind;
                actor_fields[1] = source->stream.header != NULL;
                actor_fields[2] = source->stream.byte_len;
                actor_fields[3] = source->stream.domain;
                actor_fields[4] = actor_status[i].flags;
                actor_fields[5] = actor_animations[i].frame;
                actor_fields[6] = actor_animations[i].selector;
                actor_fields[7] = actor_animations[i].id;
                actor_fields[8] = (uint16)actor_health[i].stun;
                actor_fields[9] = (uint8)actor_contacts[i].target_ticks;
                actor_fields[10] = (uint8)actor_contacts[i].last_target;
                actor_fields[11] = (uint8)actor_contacts[i].captor;
                actor_fields[12] = (uint8)actor_items[i].slot[ACTOR_ITEM_LEFT];
                actor_fields[13] = (uint8)actor_items[i].slot[ACTOR_ITEM_RIGHT];
                actor_fields[14] = (uint16)actor_orientation[i].yaw;
                actor_fields[15] = (uint16)actor_orientation[i].roll;
                actor_fields[16] = (uint16)actor_orientation[i].yaw_rate;
                actor_fields[17] = (uint16)actor_orientation[i].roll_rate;
                actor_fields[18] = (uint16)actor_types[i];
                actor_fields[19] = (uint16)actor_health[i].hp;
                actor_fields[20] = (uint8)actor_lifecycle[i].death_ticks;
                actor_fields[21] = npc_behavior_actors[i].behavior;
                actor_fields[22] = character_actor_names[i];
                if (fwrite(actor_fields, sizeof(actor_fields), 1, out) != 1)
                    result = 4;
            }
            for (i = NPC_BEHAVIOR_NPC_PROFILE_COUNT * NPC_BEHAVIOR_ROWS_PER_PROFILE; i < NPC_BEHAVIOR_ROW_COUNT; i++)
            {
                NPC_BEHAVIOR_ROW *row = npc_behavior_rows + i;
                uint8 bytes[8] = {row->capability, row->hit_delay, row->recovery_delay, row->reaction_threshold, row->pickup_bias, row->damage_out, row->health_units, row->damage_in};
                if (fwrite(bytes, sizeof(bytes), 1, out) != 1)
                    result = 4;
            }
            if (resource_arenas.scratch.data != resource_storage.data + RESOURCE_SCRATCH_OFFSET)
                abort();
            {
                uint32 words[3] = {RESOURCE_SCRATCH_OFFSET, resource_arenas.scratch.available, resource_arenas.scratch.context};
                if (fwrite(words, sizeof(words), 1, out) != 1)
                    result = 4;
            }
            ++cases;
        }
        if (ferror(in) || (!result && got) || cases != 72)
            result = 4;
        if (fclose(in))
            result = 4;
        if (fclose(out))
            result = 4;
        if (!result)
            diagnostic_end();
        return result;
    }
    if (argc > 1 && (!strcmp(argv[1], "--audit-texture-loader") || !strcmp(argv[1], "--audit-scene-resources")))
    {
        FILE *in, *out, *image;
        uint32 header[9], groups[TEXTURE_GROUP_COUNT][3], i, fields[6];
        uint32 output_header[4] = {0x324e4c46, 1, 1098, 2329};
        uint8 *archive = NULL, *archive_guard = NULL;
        RESOURCE_ARCHIVE owner_guard;
        RESOURCE_ARENAS arenas_guard;
        RESOURCE_IO io_guard;
        LOADING loading_guard;
        GAME_SESSION session_guard;
        TERRAIN_MAP terrain_guard;
        TEXTURE_GROUP group_guard[TEXTURE_GROUP_COUNT];
        sint16 rect_words[16], height_guard[MODEL_TYPE_COUNT];
        uint8 kinds[80];
        WORLD_REGION_KIND kind_guard[WORLD_REGION_KIND_COUNT];
        TEXTURE_UPLOAD_RECTANGLES rect_guard;
        BACKGROUND background_guard;
        uint32 scene_fields[4], seed_guard = random_seed;
        int scene = !strcmp(argv[1], "--audit-scene-resources"), result = 0;
        in = xport_fopen(scene ? "../status/menu/scene-resources-jobs.bin" : "../status/menu/texture-loader-jobs.bin", "rb");
        out = xport_fopen(scene ? "../status/menu/scene-resources-native.bin" : "../status/menu/texture-loader-native.bin", "wb");
        image = xport_fopen(scene ? "../status/menu/scene-resources-native.vram" : "../status/menu/texture-loader-native.vram", "wb");
        if (!in || !out || !image)
        {
            if (in)
                fclose(in);
            if (out)
                fclose(out);
            if (image)
                fclose(image);
            return 4;
        }
        if (fread(header, sizeof(header), 1, in) != 1 || header[0] != 0x324a4c46 || header[1] != 1 || header[2] != 511180 || header[3] != 26 || header[5] == 1 || header[5] == 2 || header[5] > 255 || header[6] != 1098 || header[7] != 2329 || header[8] != 1 || fread(groups, sizeof(groups), 1, in) != 1)
            result = 4;
        if (!result)
        {
            for (i = 0; i < TEXTURE_GROUP_COUNT; i++)
            {
                if ((groups[i][0] != UINT32_MAX && groups[i][0] >= TEXTURE_NAME_COUNT) || (groups[i][1] != UINT32_MAX && groups[i][1] >= TEXTURE_NAME_COUNT) || groups[i][2] > 65535)
                {
                    result = 4;
                    break;
                }
                tex_grps[i].img_name = groups[i][0] == UINT32_MAX ? NULL : tex_names[groups[i][0]];
                tex_grps[i].pal_name = groups[i][1] == UINT32_MAX ? NULL : tex_names[groups[i][1]];
                tex_grps[i].first_tex = (sint16)(uint16)groups[i][2];
            }
        }
        if (!result)
        {
            memset(&tex_desc, 0xa5, sizeof(tex_desc));
            memset(tex_map, 0xa5, sizeof(tex_map));
            if (fread(&tex_allocator.x, sizeof(tex_allocator.x), 1, in) != 1 || fread(&tex_allocator.y, sizeof(tex_allocator.y), 1, in) != 1 || fread(tex_base_v, sizeof(tex_base_v), 1, in) != 1 || fread(tex_desc.item, 1098 * sizeof(TEXTURE_DESCRIPTOR), 1, in) != 1 || fread(tex_map, 2329 * sizeof(sint16), 1, in) != 1 || fread(&tex_pals, sizeof(tex_pals), 1, in) != 1 || fread(tex_effect_pals.fade, sizeof(tex_effect_pals.fade), 1, in) != 1 || fread(tex_effect_pals.cycle, sizeof(tex_effect_pals.cycle), 1, in) != 1 || fread(tex_effect_pals.stage0, sizeof(tex_effect_pals.stage0), 1, in) != 1 || fread(tex_effect_pals.stage0_dim, sizeof(tex_effect_pals.stage0_dim), 1, in) != 1 || fread(terrain_map.tile_flat, sizeof(terrain_map.tile_flat), 1, in) != 1)
                result = 4;
        }
        if (!result)
        {
            archive = malloc(header[2]);
            archive_guard = malloc(header[2]);
            if (!archive || !archive_guard || fread(archive, 1, header[2], in) != header[2])
                result = 4;
        }
        if (!result && scene)
        {
            if (fread(rect_words, sizeof(rect_words), 1, in) != 1 || fread(world_object_height, sizeof(world_object_height), 1, in) != 1 || fread(kinds, sizeof(kinds), 1, in) != 1)
                result = 4;
            else
            {
                for (i = 0; i < 4; i++)
                {
                    PSX_RECT *rect = i ? tex_upload_rects.terrain + i - 1 : &tex_upload_rects.background;
                    sint16 *words = rect_words + i * 4;
                    if (words[0] < 0 || words[1] < 0 || words[2] < 0 || words[3] < 0 || words[0] + words[2] > 1024 || words[1] + words[3] > 512)
                        result = 4;
                    rect->x = words[0];
                    rect->y = words[1];
                    rect->w = words[2];
                    rect->h = words[3];
                }
                for (i = 0; i < WORLD_REGION_KIND_COUNT; i++)
                {
                    world_region_kinds[i].mode = (sint8)kinds[2 * i];
                    world_region_kinds[i].flags = (sint8)kinds[2 * i + 1];
                }
                memcpy(height_guard, world_object_height, sizeof(height_guard));
                memcpy(kind_guard, world_region_kinds, sizeof(kind_guard));
                world_object_active_types = WORLD_OBJECT_TYPES_FRONTEND;
                output_header[1] = 2;
            }
        }
        if (!result && (fgetc(in) != EOF || ferror(in)))
            result = 4;
        if (!result)
        {
            memcpy(archive_guard, archive, header[2]);
            resource_archive = (RESOURCE_ARCHIVE){(RESOURCE_DIRECTORY *)archive, header[2], 0};
            if (scene)
                resource_arenas.scratch = (RESOURCE_CURSOR){archive, header[2], 0};
            memcpy(&rect_guard, &tex_upload_rects, sizeof(rect_guard));
            memcpy(&background_guard, &background, sizeof(background_guard));
            loading.total = 0;
            loading.count = header[4];
            game_session.stage = header[3];
            memcpy(&owner_guard, &resource_archive, sizeof(owner_guard));
            memcpy(&arenas_guard, &resource_arenas, sizeof(arenas_guard));
            memcpy(&io_guard, &resource_io, sizeof(io_guard));
            memcpy(&loading_guard, &loading, sizeof(loading_guard));
            memcpy(&session_guard, &game_session, sizeof(session_guard));
            memcpy(&terrain_guard, &terrain_map, sizeof(terrain_guard));
            memcpy(group_guard, tex_grps, sizeof(group_guard));
            xport_set_headless(1);
            gpu_init_empty();
            memset(VRAM, scene ? 0 : 0xa5, sizeof(VRAM));
            if (scene)
            {
                scene_fields[0] = resource_load_frontend_surface();
                if (resource_arenas.scratch.data != archive + 444 || resource_arenas.scratch.context != 444 || resource_arenas.scratch.available != header[2] - 444)
                    abort();
                scene_fields[1] = background.first_color;
                scene_fields[2] = background.last_color;
                scene_fields[3] = 444;
                memcpy(archive_guard + 444, archive + 444, 16384);
                memcpy(&arenas_guard.scratch, &resource_arenas.scratch, sizeof(arenas_guard.scratch));
                memcpy(terrain_guard.color_flat, terrain_map.color_flat, sizeof(terrain_guard.color_flat));
                rect_guard.background.y = tex_upload_rects.background.y;
                background_guard.first_color = background.first_color;
                background_guard.last_color = background.last_color;
            }
            fields[0] = tex_load_grps(1);
            fields[1] = tex_count;
            fields[2] = tex_desc_count;
            fields[3] = tex_allocator.small_img_count;
            fields[4] = loading.count;
            fields[5] = resource_io.result_size;
            if (tex_count != 2329 || tex_desc_count != 1096 || tex_count - tex_allocator.small_img_count != 840)
                abort();
            for (i = 1098 * sizeof(TEXTURE_DESCRIPTOR); i < sizeof(tex_desc); i++)
                if (((uint8 *)&tex_desc)[i] != 0xa5)
                    abort();
            for (i = 2329 * sizeof(sint16); i < sizeof(tex_map); i++)
                if (((uint8 *)tex_map)[i] != 0xa5)
                    abort();
            for (i = 0; i < TEXTURE_GROUP_COUNT; i++)
                group_guard[i].first_tex = tex_grps[i].first_tex;
            io_guard.result_size = resource_io.result_size;
            loading_guard.count = loading.count;
            memcpy(terrain_guard.tile_flat, terrain_map.tile_flat, sizeof(terrain_guard.tile_flat));
            if ((scene && (memcmp(height_guard, world_object_height, sizeof(height_guard)) || memcmp(kind_guard, world_region_kinds, sizeof(kind_guard)) || random_seed != seed_guard || world_object_active_types != WORLD_OBJECT_TYPES_FRONTEND)) || memcmp(&rect_guard, &tex_upload_rects, sizeof(rect_guard)) || memcmp(&background_guard, &background, sizeof(background_guard)) || memcmp(archive_guard, archive, header[2]) || memcmp(&owner_guard, &resource_archive, sizeof(owner_guard)) || memcmp(&arenas_guard, &resource_arenas, sizeof(arenas_guard)) || memcmp(&io_guard, &resource_io, sizeof(io_guard)) || memcmp(&loading_guard, &loading, sizeof(loading_guard)) || memcmp(&session_guard, &game_session, sizeof(session_guard)) || memcmp(&terrain_guard, &terrain_map, sizeof(terrain_guard)) || memcmp(group_guard, tex_grps, sizeof(group_guard)))
                abort();
            if (fwrite(output_header, sizeof(output_header), 1, out) != 1 || fwrite(fields, sizeof(fields), 1, out) != 1 || fwrite(&tex_allocator.x, sizeof(tex_allocator.x), 1, out) != 1 || fwrite(&tex_allocator.y, sizeof(tex_allocator.y), 1, out) != 1 || fwrite(tex_allocator.row_used, sizeof(tex_allocator.row_used), 1, out) != 1 || fwrite(tex_base_v, sizeof(tex_base_v), 1, out) != 1)
                result = 4;
            for (i = 0; !result && i < TEXTURE_GROUP_COUNT; i++)
                if (fwrite(&tex_grps[i].first_tex, sizeof(sint16), 1, out) != 1)
                    result = 4;
            if (fwrite(tex_desc.item, 1098 * sizeof(TEXTURE_DESCRIPTOR), 1, out) != 1 || fwrite(tex_map, 2329 * sizeof(sint16), 1, out) != 1 || fwrite(&tex_pals, sizeof(tex_pals), 1, out) != 1 || fwrite(terrain_map.tile_flat, sizeof(terrain_map.tile_flat), 1, out) != 1 || fwrite(tex_allocator.used, sizeof(tex_allocator.used), 1, out) != 1 || fwrite(tex_effect_pals.fade, sizeof(tex_effect_pals.fade), 1, out) != 1 || fwrite(tex_effect_pals.cycle, sizeof(tex_effect_pals.cycle), 1, out) != 1 || fwrite(tex_effect_pals.stage0, sizeof(tex_effect_pals.stage0), 1, out) != 1 || fwrite(tex_effect_pals.stage0_dim, sizeof(tex_effect_pals.stage0_dim), 1, out) != 1 || fwrite(VRAM, sizeof(VRAM), 1, image) != 1)
                result = 4;
            if (scene)
            {
                if (fwrite(scene_fields, sizeof(scene_fields), 1, out) != 1 || fwrite(&tex_upload_rects.background, sizeof(tex_upload_rects.background), 1, out) != 1 || fwrite(tex_upload_rects.terrain, sizeof(tex_upload_rects.terrain), 1, out) != 1 || fwrite(terrain_map.color_flat, sizeof(terrain_map.color_flat), 1, out) != 1 || fwrite(archive + 444, 16384, 1, out) != 1 || !frontend_world_audit_store(out, scene_fields[0]))
                    result = 4;
                resource_arenas.scratch = (RESOURCE_CURSOR){0};
            }
            resource_archive = (RESOURCE_ARCHIVE){0};
        }
        if (fclose(in))
            result = 4;
        if (fclose(out))
            result = 4;
        if (fclose(image))
            result = 4;
        free(archive_guard);
        free(archive);
        if (!result)
            diagnostic_end();
        return result;
    }

    if (argc > 1 && (!strcmp(argv[1], "--audit-terrain-textures") || !strcmp(argv[1], "--audit-background") || !strcmp(argv[1], "--audit-terrain-loader")))
    {
        FILE *in, *out, *image;
        uint32 header[4], output_header[4], count, i, value = 0;
        sint16 rect_words[16];
        uint8 *archive = NULL, *archive_guard = NULL;
        int full = !strcmp(argv[1], "--audit-terrain-loader");
        int terrain = full || !strcmp(argv[1], "--audit-terrain-textures"), result = 0;
        uint8 kinds[80];
        sint16 height_guard[MODEL_TYPE_COUNT];
        WORLD_REGION_KIND kind_guard[WORLD_REGION_KIND_COUNT];
        in = xport_fopen(full ? "../status/menu/terrain-loader-jobs.bin" : terrain ? "../status/menu/terrain-textures-jobs.bin" : "../status/menu/background-jobs.bin", "rb");
        out = xport_fopen(full ? "../status/menu/terrain-loader-native.bin" : terrain ? "../status/menu/terrain-textures-native.bin" : "../status/menu/background-native.bin", "wb");
        image = xport_fopen(full ? "../status/menu/terrain-loader-native.vram" : terrain ? "../status/menu/terrain-textures-native.vram" : "../status/menu/background-native.vram", "wb");
        if (!in || !out || !image)
        {
            if (in)
                fclose(in);
            if (out)
                fclose(out);
            if (image)
                fclose(image);
            return 4;
        }
        if (fread(header, sizeof(header), 1, in) != 1 || header[0] != 0x324a5446 || header[1] != 1 || header[2] != (full ? 2u : (uint32)terrain) || header[3] != 511180 || fread(&count, sizeof(count), 1, in) != 1 || fread(rect_words, sizeof(rect_words), 1, in) != 1)
            result = 4;
        if (!result)
        {
            archive = malloc(header[3]);
            archive_guard = malloc(header[3]);
            if (!archive || !archive_guard || fread(archive, 1, header[3], in) != header[3])
                result = 4;
        }
        if (!result && full)
        {
            if (fread(world_object_height, sizeof(world_object_height), 1, in) != 1 || fread(kinds, sizeof(kinds), 1, in) != 1)
                result = 4;
            else
            {
                for (i = 0; i < WORLD_REGION_KIND_COUNT; i++)
                {
                    world_region_kinds[i].mode = (sint8)kinds[2 * i];
                    world_region_kinds[i].flags = (sint8)kinds[2 * i + 1];
                }
                memcpy(height_guard, world_object_height, sizeof(height_guard));
                memcpy(kind_guard, world_region_kinds, sizeof(kind_guard));
            }
        }
        if (!result && fgetc(in) != EOF)
            result = 4;
        if (!result)
        {
            RESOURCE_ARCHIVE owner_guard;
            RESOURCE_ARENAS arenas_guard;
            RESOURCE_IO io_guard;
            LOADING loading_guard;
            DISPLAY_BUFFERS display_guard;
            TEXTURE_PALETTES pals_guard;
            TEXTURE_UPLOAD_RECTANGLES rect_guard;
            BACKGROUND background_guard;
            WORLD_OBJECTS world_guard;
            COLLISION_REGIONS regions_guard;
            GAME_SESSION session_guard;
            TERRAIN_MAP map_guard;
            WORLD_OBJECT_TYPE_GROUP_ID active_types_guard;
            uint32 seed_guard;
            uint32 fields[5];
            uintptr_t staging_offset = 0;
            memcpy(archive_guard, archive, header[3]);
            resource_archive = (RESOURCE_ARCHIVE){(RESOURCE_DIRECTORY *)archive, header[3], 0};
            resource_arenas.scratch = (RESOURCE_CURSOR){archive, header[3], 0};
            loading.total = 0;
            loading.count = count;
            for (i = 0; i < 4; i++)
            {
                PSX_RECT *rect = i ? tex_upload_rects.terrain + i - 1 : &tex_upload_rects.background;
                rect->x = rect_words[4 * i];
                rect->y = rect_words[4 * i + 1];
                rect->w = rect_words[4 * i + 2];
                rect->h = rect_words[4 * i + 3];
            }
            if (full)
            {
                game_session.stage = 26;
                world_object_active_types = WORLD_OBJECT_TYPES_FRONTEND;
                for (i = 0; i < WORLD_OBJECT_COUNT; i++)
                {
                    MODEL_OBJECT *item = world_objects.object + i;
                    memset(item, 0, sizeof(*item));
                    item->kind = 0xa5a5a5a5;
                    item->pos.x.bits = item->pos.y.bits = item->pos.z.bits = 0xa5a5a5a5;
                    item->angles.yaw_bits = item->angles.pitch_bits = item->angles.roll_bits = 0xa5a5;
                    item->payload_kind = MODEL_PAYLOAD_SPRITE;
                    item->spr.frame = 123;
                }
                for (i = 0; i < COLLISION_REGION_COUNT; i++)
                {
                    COLLISION_REGION *region = col_regions.data + i;
                    memset(region, 0, sizeof(*region));
                    region->geometry.coord[0] = region->geometry.coord[1] = 123;
                    region->geometry.coord[2] = region->geometry.coord[3] = 456;
                    region->trigger_kind = 3;
                    region->shape = 2;
                    region->col_kind = 1;
                    region->model_type = 454;
                    region->hits = 12;
                    region->alternate_attack = 1;
                    region->object = (WORLD_OBJECT_REFERENCE){0, 1};
                }
                col_regions.named[0] = col_regions.data;
                col_regions.named[1] = col_regions.data + 1;
                memset(&terrain_map, 0xa5, sizeof(terrain_map));
            }
            memcpy(&world_guard, &world_objects, sizeof(world_guard));
            memcpy(&regions_guard, &col_regions, sizeof(regions_guard));
            memcpy(&session_guard, &game_session, sizeof(session_guard));
            memcpy(&map_guard, &terrain_map, sizeof(map_guard));
            active_types_guard = world_object_active_types;
            seed_guard = random_seed;
            memcpy(&owner_guard, &resource_archive, sizeof(owner_guard));
            memcpy(&arenas_guard, &resource_arenas, sizeof(arenas_guard));
            memcpy(&io_guard, &resource_io, sizeof(io_guard));
            memcpy(&loading_guard, &loading, sizeof(loading_guard));
            memcpy(&display_guard, &disp_buffers, sizeof(display_guard));
            memcpy(&pals_guard, &tex_pals, sizeof(pals_guard));
            memcpy(&rect_guard, &tex_upload_rects, sizeof(rect_guard));
            memcpy(&background_guard, &background, sizeof(background_guard));
            xport_set_headless(1);
            gpu_init_empty();
            memset(VRAM, 0xa5, sizeof(VRAM));
            if (full)
            {
                value = terrain_load("STGFE.PBL", "STGFE.MPD");
                if ((uintptr_t)resource_arenas.scratch.data < (uintptr_t)archive || (uintptr_t)resource_arenas.scratch.data - (uintptr_t)archive > header[3])
                    abort();
                staging_offset = (uintptr_t)resource_arenas.scratch.data - (uintptr_t)archive;
                if (16384 > header[3] - staging_offset || resource_arenas.scratch.context != staging_offset || resource_arenas.scratch.available != header[3] - staging_offset)
                    abort();
                fields[0] = value;
                fields[1] = (uint32)staging_offset;
                fields[2] = loading.count;
                fields[3] = resource_io.result_size;
                memcpy(archive_guard + staging_offset, archive + staging_offset, 16384);
                memcpy(&arenas_guard.scratch, &resource_arenas.scratch, sizeof(arenas_guard.scratch));
                world_guard.count = world_objects.count;
                world_guard.tracked_count = world_objects.tracked_count;
                memcpy(world_guard.object, world_objects.object, sizeof(world_guard.object));
                regions_guard.count = col_regions.count;
                memcpy(regions_guard.data, col_regions.data, sizeof(regions_guard.data));
                regions_guard.named[0] = col_regions.named[0];
                regions_guard.named[1] = col_regions.named[1];
                memcpy(&map_guard, &terrain_map, sizeof(map_guard));
            }
            else if (terrain)
            {
                RESOURCE_VIEW view = terrain_load_texs("STGFE.PBL");
                uintptr_t offset;
                if (view.storage != archive || view.storage_len != header[3] || (uintptr_t)view.data < (uintptr_t)archive || (uintptr_t)view.data - (uintptr_t)archive > header[3] || (uintptr_t)resource_arenas.scratch.data < (uintptr_t)archive || (uintptr_t)resource_arenas.scratch.data - (uintptr_t)archive > header[3])
                    abort();
                offset = (uintptr_t)view.data - (uintptr_t)archive;
                staging_offset = (uintptr_t)resource_arenas.scratch.data - (uintptr_t)archive;
                if (view.byte_len > header[3] - offset || view.context_address != offset || 16384 > header[3] - staging_offset || resource_arenas.scratch.context != staging_offset || resource_arenas.scratch.available != header[3] - staging_offset)
                    abort();
                fields[0] = (uint32)offset;
                fields[1] = view.byte_len;
                fields[2] = (uint32)staging_offset;
                fields[3] = loading.count;
                fields[4] = resource_io.result_size;
                memcpy(archive_guard + staging_offset, archive + staging_offset, 16384);
                memcpy(&arenas_guard.scratch, &resource_arenas.scratch, sizeof(arenas_guard.scratch));
            }
            else
            {
                value = tex_load_background("STGFE.PDT", "STGFE.PPL", 1);
                background_guard.first_color = background.first_color;
                background_guard.last_color = background.last_color;
                rect_guard.background.y = tex_upload_rects.background.y;
            }
            memcpy(pals_guard.color[terrain ? 1 : 0], tex_pals.color[terrain ? 1 : 0], sizeof(tex_pals.color[0]));
            io_guard.result_size = resource_io.result_size;
            loading_guard.count = loading.count;
            if ((full && (memcmp(height_guard, world_object_height, sizeof(height_guard)) || memcmp(kind_guard, world_region_kinds, sizeof(kind_guard)))) || memcmp(&world_guard, &world_objects, sizeof(world_guard)) || memcmp(&regions_guard, &col_regions, sizeof(regions_guard)) || memcmp(&session_guard, &game_session, sizeof(session_guard)) || memcmp(&map_guard, &terrain_map, sizeof(map_guard)) || active_types_guard != world_object_active_types || seed_guard != random_seed || memcmp(archive_guard, archive, header[3]) || memcmp(&owner_guard, &resource_archive, sizeof(owner_guard)) || memcmp(&arenas_guard, &resource_arenas, sizeof(arenas_guard)) || memcmp(&io_guard, &resource_io, sizeof(io_guard)) || memcmp(&loading_guard, &loading, sizeof(loading_guard)) || memcmp(&display_guard, &disp_buffers, sizeof(display_guard)) || memcmp(&pals_guard, &tex_pals, sizeof(pals_guard)) || memcmp(&rect_guard, &tex_upload_rects, sizeof(rect_guard)) ||
                memcmp(&background_guard, &background, sizeof(background_guard)))
                abort();
            output_header[0] = 0x324e5446;
            output_header[1] = 1;
            output_header[2] = full ? 2 : terrain;
            output_header[3] = full ? 48680 : terrain ? 16916 : 540;
            if (fwrite(output_header, sizeof(output_header), 1, out) != 1)
                result = 4;
            if (full)
            {
                if (fwrite(fields, 16, 1, out) != 1 || fwrite(tex_pals.color[1], sizeof(tex_pals.color[1]), 1, out) != 1 || fwrite(resource_arenas.scratch.data, 16384, 1, out) != 1 || fwrite(terrain_map.tile_flat, sizeof(terrain_map.tile_flat), 1, out) != 1 || fwrite(terrain_map.color_flat, sizeof(terrain_map.color_flat), 1, out) != 1 || !frontend_world_audit_store(out, value))
                    result = 4;
            }
            else if (terrain)
            {
                if (fwrite(fields, sizeof(fields), 1, out) != 1 || fwrite(tex_pals.color[1], sizeof(tex_pals.color[1]), 1, out) != 1 || fwrite(resource_arenas.scratch.data, 16384, 1, out) != 1)
                    result = 4;
            }
            else
            {
                uint32 colors[2] = {background.first_color, background.last_color};
                sint16 rect[4] = {tex_upload_rects.background.x, tex_upload_rects.background.y, tex_upload_rects.background.w, tex_upload_rects.background.h};
                uint32 counters[2] = {loading.count, resource_io.result_size};
                if (fwrite(&value, sizeof(value), 1, out) != 1 || fwrite(tex_pals.color[0], sizeof(tex_pals.color[0]), 1, out) != 1 || fwrite(colors, sizeof(colors), 1, out) != 1 || fwrite(rect, sizeof(rect), 1, out) != 1 || fwrite(counters, sizeof(counters), 1, out) != 1)
                    result = 4;
            }
            if (fwrite(VRAM, sizeof(VRAM), 1, image) != 1)
                result = 4;
        }
        if (ferror(in) || ferror(out) || ferror(image))
            result = 4;
        if (fclose(in))
            result = 4;
        if (fclose(out))
            result = 4;
        if (fclose(image))
            result = 4;
        resource_archive = (RESOURCE_ARCHIVE){0};
        resource_arenas.scratch = (RESOURCE_CURSOR){0};
        free(archive_guard);
        free(archive);
        if (!result)
            diagnostic_end();
        return result;
    }

    if (argc > 1 && (!strcmp(argv[1], "--audit-resource-find") || !strcmp(argv[1], "--audit-stage-resource-find")))
    {
        FILE *in, *out;
        uint8 name[16], name_guard[16], *archive = NULL, *archive_guard = NULL;
        uint32 header[4], output_header[4], cases = 0;
        size_t got = 0;
        int stage = !strcmp(argv[1], "--audit-stage-resource-find"), result = 0;
        in = xport_fopen(stage ? "../status/menu/stage-resource-find-jobs.bin" : "../status/menu/resource-find-jobs.bin", "rb");
        out = xport_fopen(stage ? "../status/menu/stage-resource-find-native.bin" : "../status/menu/resource-find-native.bin", "wb");
        if (!in || !out)
        {
            if (in)
                fclose(in);
            if (out)
                fclose(out);
            return 4;
        }
        if (fread(header, sizeof(header), 1, in) != 1 || header[0] != 0x324a5246 || header[1] != 1 || header[2] < 4 || header[2] > RESOURCE_STORAGE_CAPACITY || header[3] != (stage ? 22u : 28u))
            result = 4;
        if (!result)
        {
            archive = malloc(header[2]);
            archive_guard = malloc(header[2]);
            if (!archive || !archive_guard || fread(archive, 1, header[2], in) != header[2])
                result = 4;
        }
        if (!result)
        {
            memcpy(archive_guard, archive, header[2]);
            resource_archive = (RESOURCE_ARCHIVE){(RESOURCE_DIRECTORY *)archive, header[2], 0};
            output_header[0] = 0x324e5246;
            output_header[1] = 1;
            output_header[2] = 16;
            output_header[3] = header[3];
            if (fwrite(output_header, sizeof(output_header), 1, out) != 1)
                result = 4;
        }
        while (!result && (got = fread(name, 1, sizeof(name), in)) == sizeof(name))
        {
            RESOURCE_VIEW view;
            RESOURCE_ARCHIVE archive_owner_guard;
            uint32 fields[4], file_size_guard;
            uintptr_t offset;
            if (cases >= header[3] || !memchr(name, 0, sizeof(name)))
            {
                result = 4;
                break;
            }
            memcpy(name_guard, name, sizeof(name));
            memcpy(&archive_owner_guard, &resource_archive, sizeof(archive_owner_guard));
            resource_io.result_size = 0xa5a5a5a5;
            file_size_guard = resource_io.file_size;
            view = resource_find_quiet((const char *)name);
            if (view.storage != archive || view.storage_len != header[2] || (uintptr_t)view.data < (uintptr_t)archive || (uintptr_t)view.data - (uintptr_t)archive > header[2])
                abort();
            offset = (uintptr_t)view.data - (uintptr_t)archive;
            if (view.byte_len > header[2] - offset || resource_io.file_size != file_size_guard || memcmp(name_guard, name, sizeof(name)) || memcmp(archive_guard, archive, header[2]) || memcmp(&archive_owner_guard, &resource_archive, sizeof(archive_owner_guard)))
                abort();
            fields[0] = (uint32)offset;
            fields[1] = view.byte_len;
            fields[2] = resource_io.result_size;
            fields[3] = view.context_address;
            if (fwrite(fields, sizeof(fields), 1, out) != 1)
                result = 4;
            ++cases;
        }
        if (ferror(in) || (!result && (got || cases != header[3])))
            result = 4;
        if (fclose(in))
            result = 4;
        if (fclose(out))
            result = 4;
        resource_archive = (RESOURCE_ARCHIVE){0};
        free(archive_guard);
        free(archive);
        if (!result)
            diagnostic_end();
        return result;
    }
    if (argc > 1 && strcmp(argv[1], "--audit-actor-init") == 0)
    {
        const uint32 header[4] = {0x32414846, 1, 164, 0};
        FILE *in, *out;
        uint32 a[ACTOR_COUNT], value, i;
        size_t got = 0;
        int result = 0;
        in = xport_fopen("../status/menu/actor-init-jobs.bin", "rb");
        out = xport_fopen("../status/menu/actor-init-native.bin", "wb");
        if (!in || !out)
        {
            if (in)
                fclose(in);
            if (out)
                fclose(out);
            return 4;
        }
        if (fwrite(header, sizeof(header), 1, out) != 1)
            result = 4;
        while (!result && (got = fread(a, 1, sizeof(a), in)) == sizeof(a))
        {
            sint16 types_guard[ACTOR_COUNT];
            LEVEL_WAVE wave_guard;
            LEVEL_TRANSITION transition_guard;
            memset(actor_health, 0xa5, sizeof(actor_health));
            memset(actor_lighting, 0xa5, sizeof(actor_lighting));
            memset(player_scores, 0xa5, sizeof(player_scores));
            memset(character_name_cursor, 0xa5, sizeof(character_name_cursor));
            level_transition.bonus_item_pending = 0xa5a5a5a5;
            level_wave.population_gate = 0xa5a5a5a5;
            for (i = 0; i < ACTOR_COUNT; i++)
                actor_types[i] = (sint16)(uint16)a[i];
            memcpy(types_guard, actor_types, sizeof(types_guard));
            memcpy(&wave_guard, &level_wave, sizeof(wave_guard));
            memcpy(&transition_guard, &level_transition, sizeof(transition_guard));
            value = actors_reset_health();
            wave_guard.population_gate = level_wave.population_gate;
            transition_guard.bonus_item_pending = level_transition.bonus_item_pending;
            if (memcmp(types_guard, actor_types, sizeof(types_guard)) || memcmp(&wave_guard, &level_wave, sizeof(wave_guard)) || memcmp(&transition_guard, &level_transition, sizeof(transition_guard)))
                abort();
            if (fwrite(&value, sizeof(value), 1, out) != 1)
                result = 4;
            for (i = 0; i < ACTOR_COUNT; i++)
            {
                uint16 fields[6] = {(uint16)actor_types[i], (uint16)actor_health[i].hp, (uint16)actor_health[i].shown, (uint16)actor_health[i].trail, (uint16)actor_health[i].max, (uint16)actor_health[i].stun};
                if (fwrite(fields, sizeof(fields), 1, out) != 1 || fwrite(&actor_lighting[i].bits, sizeof(actor_lighting[i].bits), 1, out) != 1)
                    result = 4;
            }
            {
                uint32 globals[8] = {player_scores[0].val, player_scores[1].val, player_scores[0].continue_threshold, player_scores[1].continue_threshold, level_transition.bonus_item_pending, level_wave.population_gate, character_name_cursor[0], character_name_cursor[1]};
                if (fwrite(globals, sizeof(globals), 1, out) != 1)
                    result = 4;
            }
        }
        if (ferror(in) || (!result && got))
            result = 4;
        if (fclose(in))
            result = 4;
        if (fclose(out))
            result = 4;
        if (!result)
            diagnostic_end();
        return result;
    }
    if (argc > 1 && strcmp(argv[1], "--audit-overlay") == 0)
    {
        FILE *in, *out;
        uint32 a[4];
        size_t got = 0;
        int result = 0;
        in = xport_fopen("../status/menu/overlay-jobs.bin", "rb");
        out = xport_fopen("../status/menu/overlay-native.bin", "wb");
        if (!in || !out)
        {
            if (in)
                fclose(in);
            if (out)
                fclose(out);
            return 4;
        }
        while (!result && (got = fread(a, 1, sizeof(a), in)) == sizeof(a))
        {
            uint32 observation[3];
            DISPLAY_BUFFERS unchanged;
            uint32 previous_ot = render_buffers.prev_ot_addr;
            render_buffers.packet_addr = 0x801d0000;
            render_buffers.ot_addr = 0x801c0000;
            /* Actual GPU packet and bucket bytes retain their device addresses */
            memset(psx_addr(0x801d0000, 64), 0xa5, 64);
            w_u32(0x801c1ffc, 0x12abcdef);
            disp_buffers.fade.r = (sint16)(uint16)a[1];
            disp_buffers.fade.g = (sint16)(uint16)a[2];
            disp_buffers.fade.b = (sint16)(uint16)a[3];
            disp_buffers.flash_ticks = a[1];
            memcpy(&unchanged, &disp_buffers, sizeof(unchanged));
            observation[0] = a[0] ? disp_draw_flash() : disp_draw_fade();
            observation[1] = render_buffers.packet_addr;
            observation[2] = r_u32(0x801c1ffc);
            if (memcmp(&unchanged, &disp_buffers, sizeof(unchanged)) || render_buffers.ot_addr != 0x801c0000 || render_buffers.prev_ot_addr != previous_ot)
                abort();
            if (fwrite(observation, sizeof(observation), 1, out) != 1 || fwrite(psx_addr(0x801d0000, 64), 64, 1, out) != 1)
                result = 4;
        }
        if (ferror(in) || (!result && got))
            result = 4;
        if (fclose(in))
            result = 4;
        if (fclose(out))
            result = 4;
        if (!result)
            diagnostic_end();
        return result;
    }
    if (argc > 1 && strcmp(argv[1], "--audit-display-offsets") == 0)
    {
        FILE *in, *out;
        uint32 a[4];
        size_t got = 0;
        int result = 0;
        in = xport_fopen("../status/menu/display-offsets-jobs.bin", "rb");
        out = xport_fopen("../status/menu/display-offsets-native.bin", "wb");
        if (!in || !out)
        {
            if (in)
                fclose(in);
            if (out)
                fclose(out);
            return 4;
        }
        while (!result && (got = fread(a, 1, sizeof(a), in)) == sizeof(a))
        {
            int index;
            DISPLAY_BUFFERS unchanged;
            disp_buffers.shake_ticks = a[0];
            disp_buffers.frame = a[1];
            game_settings.scr_offset.x = (uint16)a[2];
            game_settings.scr_offset.y = (uint16)a[3];
            memset(disp_buffers.env, 0xa5, sizeof(disp_buffers.env));
            memcpy(&unchanged, &disp_buffers, sizeof(unchanged));
            disp_update_offsets();
            unchanged.env[a[1] & 1].screen.x = disp_buffers.env[a[1] & 1].screen.x;
            unchanged.env[a[1] & 1].screen.y = disp_buffers.env[a[1] & 1].screen.y;
            if (memcmp(&unchanged, &disp_buffers, sizeof(unchanged)))
                abort();
            for (index = 0; index < DISPLAY_ENV_COUNT; index++)
            {
                const DISPENV *env = &disp_buffers.env[index];
                uint16 rect[8] = {(uint16)env->disp.x, (uint16)env->disp.y, (uint16)env->disp.w, (uint16)env->disp.h, (uint16)env->screen.x, (uint16)env->screen.y, (uint16)env->screen.w, (uint16)env->screen.h};
                uint8 flags[4] = {env->isinter, env->isrgb24, env->pad0, env->pad1};
                if (fwrite(rect, sizeof(rect), 1, out) != 1 || fwrite(flags, sizeof(flags), 1, out) != 1)
                    result = 4;
            }
        }
        if (ferror(in) || (!result && got))
            result = 4;
        if (fclose(in))
            result = 4;
        if (fclose(out))
            result = 4;
        if (!result)
            diagnostic_end();
        return result;
    }
    if (argc > 1 && strcmp(argv[1], "--audit-frame-buffers") == 0)
    {
        FILE *in, *out;
        uint32 a[4];
        size_t got = 0;
        int result = 0;
        in = xport_fopen("../status/menu/frame-buffers-jobs.bin", "rb");
        out = xport_fopen("../status/menu/frame-buffers-native.bin", "wb");
        if (!in || !out)
        {
            if (in)
                fclose(in);
            if (out)
                fclose(out);
            return 4;
        }
        while (!result && (got = fread(a, 1, sizeof(a), in)) == sizeof(a))
        {
            uint32 observation[4];
            DISPLAY_BUFFERS unchanged;
            disp_buffers.frame = a[0];
            memcpy(&unchanged, &disp_buffers, sizeof(unchanged));
            observation[0] = render_buffers_select();
            observation[1] = render_buffers.packet_addr;
            observation[2] = render_buffers.ot_addr;
            observation[3] = render_buffers.prev_ot_addr;
            if (memcmp(&unchanged, &disp_buffers, sizeof(unchanged)))
                abort();
            if (fwrite(observation, sizeof(observation), 1, out) != 1)
                result = 4;
        }
        if (ferror(in) || (!result && got))
            result = 4;
        if (fclose(in))
            result = 4;
        if (fclose(out))
            result = 4;
        if (!result)
            diagnostic_end();
        return result;
    }
    if (argc > 1 && strcmp(argv[1], "--audit-actor-direction") == 0)
    {
        FILE *in, *out;
        uint32 a[4], observation[2];
        size_t got = 0;
        int result = 0;
        in = xport_fopen("../status/menu/actor-direction-jobs.bin", "rb");
        out = xport_fopen("../status/menu/actor-direction-native.bin", "wb");
        if (!in || !out)
        {
            if (in)
                fclose(in);
            if (out)
                fclose(out);
            return 4;
        }
        while (!result && (got = fread(a, 1, sizeof(a), in)) == sizeof(a))
        {
            dir_distance = (sint32)0xa5a5a5a5;
            observation[0] = (uint32)math_dir11((sint32)a[0], (sint32)a[1], (sint32)a[2], (sint32)a[3]);
            observation[1] = (uint32)dir_distance;
            if (fwrite(observation, sizeof(observation), 1, out) != 1)
                result = 4;
        }
        if (ferror(in) || (!result && got))
            result = 4;
        if (fclose(in))
            result = 4;
        if (fclose(out))
            result = 4;
        if (!result)
            diagnostic_end();
        return result;
    }
    if (argc > 1 && strcmp(argv[1], "--audit-actor-math") == 0)
    {
        FILE *in, *out;
        uint32 a[5], observation[2];
        size_t got = 0;
        int result = 0;
        in = xport_fopen("../status/menu/actor-math-jobs.bin", "rb");
        out = xport_fopen("../status/menu/actor-math-native.bin", "wb");
        if (!in || !out)
        {
            if (in)
                fclose(in);
            if (out)
                fclose(out);
            return 4;
        }
        while (!result && (got = fread(a, 1, sizeof(a), in)) == sizeof(a))
        {
            ACTOR_ANIMATION unchanged;
            /* The original helper loads only the actor's animation ID */
            actor_animations[0].id = a[1];
            memcpy(&unchanged, &actor_animations[0], sizeof(unchanged));
            observation[0] = (uint32)(a[0] == 0 ? animation_movement_state(actor_animations[0].id) : a[0] == 1 ? math_sqrt_newton((sint32)a[1]) : math_distance_newton((sint32)a[1], (sint32)a[2], (sint32)a[3], (sint32)a[4]));
            observation[1] = actor_animations[0].id;
            if (memcmp(&unchanged, &actor_animations[0], sizeof(unchanged)))
                abort();
            if (fwrite(observation, sizeof(observation), 1, out) != 1)
                result = 4;
        }
        if (ferror(in) || (!result && got))
            result = 4;
        if (fclose(in))
            result = 4;
        if (fclose(out))
            result = 4;
        if (!result)
            diagnostic_end();
        return result;
    }
    if (argc > 1 && strcmp(argv[1], "--audit-animation-decode") == 0)
    {
        FILE *in, *out;
        const uint32 input_header[4] = {0x334a5046, 1, 7, 103};
        const uint32 output_header[4] = {0x334e5046, 1, 524, 103};
        uint32 header[4], job[7], fields[5], cases = 0, i;

        union
        {
            uint32 alignment;
            uint8 bytes[1024];
        } stream;

        uint8 stream_guard[1024];

        struct
        {
            uint8 before[16];
            ACTOR_POSE pose;
            uint8 after[16];
        } output, pose_guard;

        MODEL_RIG_HEADER rig, rig_guard;
        MODEL_REFERENCE model = {.header = &rig.mesh, .byte_len = sizeof(rig)};
        ACTOR_ANIMATION playback_guard[ACTOR_COUNT];
        size_t got = 0;
        int result = 0;
        in = xport_fopen("../status/menu/animation-decode-jobs.bin", "rb");
        out = xport_fopen("../status/menu/animation-decode-native.bin", "wb");
        if (!in || !out)
        {
            if (in)
                fclose(in);
            if (out)
                fclose(out);
            return 4;
        }
        if (fread(header, sizeof(header), 1, in) != 1 || memcmp(header, input_header, sizeof(header)) || fwrite(output_header, sizeof(output_header), 1, out) != 1)
            result = 4;
        while (!result && (got = fread(job, 1, sizeof(job), in)) == sizeof(job))
        {
            ACTOR_ANIMATION *playback = actor_animations;
            memset(&output, 0xa5, sizeof(output));
            if (!job[0] || job[0] > ACTOR_POSE_BONE_COUNT || job[1] > 1 || cases >= 103 || fread(stream.bytes, sizeof(stream.bytes), 1, in) != 1 || fread(&output.pose, sizeof(output.pose), 1, in) != 1)
            {
                result = 4;
                break;
            }
            memset(&rig, 0, sizeof(rig));
            rig.bone_count = job[0];
            memcpy(&rig_guard, &rig, sizeof(rig_guard));
            memset(playback, 0xa5, sizeof(*playback));
            memset(&playback->src, 0, sizeof(playback->src));
            if (job[1])
            {
                playback->src.kind = ACTOR_ANIMATION_SOURCE_STREAM;
                playback->src.stream = (ANIMATION_STREAM){(const ANIMATION_STREAM_HEADER *)stream.bytes, sizeof(stream.bytes), ANIMATION_STREAM_NATIVE};
            }
            playback->frame = job[2];
            memcpy(playback->motion.bits, job + 3, sizeof(playback->motion.bits));
            playback->motion.events = job[6];
            memcpy(playback_guard, actor_animations, sizeof(playback_guard));
            memcpy(&pose_guard, &output, sizeof(pose_guard));
            memcpy(stream_guard, stream.bytes, sizeof(stream_guard));
            actor_pose_decode(model, 0, &output.pose);
            playback_guard[0].frame = playback->frame;
            memcpy(playback_guard[0].motion.bits, playback->motion.bits, sizeof(playback->motion.bits));
            playback_guard[0].motion.events = playback->motion.events;
            if (job[1])
                for (i = 0; i < job[0]; i++)
                    memcpy(pose_guard.pose.angles + i, output.pose.angles + i, sizeof(POSE_ANGLES));
            if (memcmp(playback_guard, actor_animations, sizeof(playback_guard)) || memcmp(&pose_guard, &output, sizeof(output)) || memcmp(stream_guard, stream.bytes, sizeof(stream_guard)) || memcmp(&rig_guard, &rig, sizeof(rig)))
                abort();
            fields[0] = playback->frame;
            memcpy(fields + 1, playback->motion.bits, sizeof(playback->motion.bits));
            fields[4] = playback->motion.events;
            if (fwrite(fields, sizeof(fields), 1, out) != 1 || fwrite(&output.pose, sizeof(output.pose), 1, out) != 1)
                result = 4;
            ++cases;
        }
        if (ferror(in) || (!result && got) || cases != 103 || ferror(out))
            result = 4;
        actor_animations[0].src = (ACTOR_ANIMATION_SOURCE){0};
        if (fclose(in))
            result = 4;
        if (fclose(out))
            result = 4;
        if (!result)
            diagnostic_end();
        return result;
    }
    if (argc > 1 && strcmp(argv[1], "--audit-animation-start") == 0)
    {
        const uint32 input_header[4] = {0x334a5346, 1, 19, 216};
        uint32 header[4], output_header[4] = {0x334e5346, 1, 92, 0};
        uint32 i, count = 0, single = UINT32_MAX, result_word;
        FILE *in, *out;
        ANIMATION_START_AUDIT_JOB *jobs = NULL;
        int result = 0;
        if (argc > 2)
        {
            char *end;
            unsigned long ordinal;
            if (argc != 4 || strcmp(argv[2], "--stop-case") || argv[3][0] < '0' || argv[3][0] > '9')
                return 4;
            ordinal = strtoul(argv[3], &end, 10);
            if (*end || ordinal >= 216)
                return 4;
            single = (uint32)ordinal;
        }
        in = xport_fopen("../status/menu/animation-start-jobs.bin", "rb");
        out = xport_fopen("../status/menu/animation-start-native.bin", "wb");
        if (!in || !out)
        {
            if (in)
                fclose(in);
            if (out)
                fclose(out);
            return 4;
        }
        jobs = malloc(216 * sizeof(*jobs));
        if (!jobs || fread(header, sizeof(header), 1, in) != 1 || memcmp(header, input_header, sizeof(header)) || fread(jobs, sizeof(*jobs), 216, in) != 216 || fgetc(in) != EOF || ferror(in))
            result = 4;
        for (i = 0; !result && i < 216; i++)
        {
            uint32 *words = jobs[i].words;
            if (words[0] > 1 || words[1] >= 4 || (!words[0] && words[1] >= 2) || (words[0] && words[2] >= ANIMATION_COUNT) || words[3] > 65535 || words[4] > 1 || words[6] > 1 || words[15] > 65535 || words[16] > 65535 || words[17] > 65535 || words[18] > 1)
                result = 4;
            ++count;
        }
        output_header[3] = single == UINT32_MAX ? count : 1;
        if (!result && fwrite(output_header, sizeof(output_header), 1, out) != 1)
            result = 4;
        for (i = 0; !result && i < 216; i++)
        {
            ANIMATION_START_AUDIT_JOB *job = jobs + i;
            uint32 *words = job->words, actor = words[1];
            ACTOR_ANIMATION *playback = actor_animations + actor;
            if (single != UINT32_MAX && single != i)
                continue;
            memset(playback, 0xa5, sizeof(*playback));
            memset(&playback->src, 0, sizeof(playback->src));
            memset(animation_start_audit.old_stream, 0xa5, sizeof(animation_start_audit.old_stream));
            if (words[6])
                playback->src = (ACTOR_ANIMATION_SOURCE){ACTOR_ANIMATION_SOURCE_STREAM, {(const ANIMATION_STREAM_HEADER *)animation_start_audit.old_stream, 128, ANIMATION_STREAM_NATIVE}};
            playback->frame = words[7];
            playback->id = words[8];
            memcpy(playback->motion.bits, words + 9, sizeof(playback->motion.bits));
            playback->motion.events = words[12];
            playback->motion.forward_offset_bits = words[13];
            actor_positions[actor].y.bits = words[14];
            actor_types[actor] = (sint16)(uint16)words[15];
            playback->hit_flags = (uint16)words[16];
            playback->event_step = (uint16)words[17];
            memcpy(action_history.action, job->history, sizeof(action_history.action));
            memset(action_history.predicted, 7, sizeof(action_history.predicted));
            memset(action_history.matched, 0xa5, sizeof(action_history.matched));
            animation_fall_exit = 0xa5a5a5a5;
            game_session.stage = words[5];
            if (words[0])
            {
                animation_flags[words[2]] = (uint16)words[3];
                animation_banks[ANIMATION_BANK_HAWK].stream[words[2]] = (ACTOR_ANIMATION_SOURCE){0};
                if (words[4])
                    animation_banks[ANIMATION_BANK_HAWK].stream[words[2]] = (ACTOR_ANIMATION_SOURCE){ACTOR_ANIMATION_SOURCE_STREAM, {(const ANIMATION_STREAM_HEADER *)job->stream, 128, ANIMATION_STREAM_NATIVE}};
            }
            animation_start_audit.out = out;
            animation_start_audit.ordinal = i;
            animation_start_audit.actor = actor;
            animation_start_audit.stream = job->stream;
            memcpy(animation_start_audit.stream_guard, job->stream, sizeof(job->stream));
            memcpy(animation_start_audit.old_guard, animation_start_audit.old_stream, sizeof(animation_start_audit.old_guard));
            memcpy(animation_start_audit.playback, actor_animations, sizeof(animation_start_audit.playback));
            memcpy(animation_start_audit.positions, actor_positions, sizeof(animation_start_audit.positions));
            memcpy(animation_start_audit.types, actor_types, sizeof(animation_start_audit.types));
            memcpy(animation_start_audit.flags, animation_flags, sizeof(animation_start_audit.flags));
            memcpy(&animation_start_audit.bank, animation_banks + ANIMATION_BANK_HAWK, sizeof(animation_start_audit.bank));
            memcpy(&animation_start_audit.session, &game_session, sizeof(animation_start_audit.session));
            result_word = words[0] ? (uint32)animation_start((sint32)actor, ANIMATION_BANK_HAWK, (sint32)words[2]) : action_history_rec((sint32)actor, (sint32)words[2]);
            if (!animation_start_audit_store(0, result_word))
                result = 4;
            actor_animations[actor].src = (ACTOR_ANIMATION_SOURCE){0};
            if (words[0])
                animation_banks[ANIMATION_BANK_HAWK].stream[words[2]] = (ACTOR_ANIMATION_SOURCE){0};
        }
        diagnostic_stop_observer = NULL;
        memset(&animation_start_audit, 0, sizeof(animation_start_audit));
        if (fclose(in))
            result = 4;
        if (fclose(out))
            result = 4;
        free(jobs);
        if (!result)
            diagnostic_end();
        return result;
    }
    if (argc > 1 && strcmp(argv[1], "--audit-animation-base") == 0)
    {
        FILE *in, *out;
        const uint32 input_header[4] = {0x334a4146, 1, 8, 158};
        const uint32 output_header[4] = {0x334e4146, 1, 7, 158};
        uint32 header[4], job[8], fields[7], cases = 0, nulls = 0;

        union
        {
            uint32 alignment;
            uint8 bytes[128];
        } stream;

        uint8 stream_guard[128];
        ACTOR_ANIMATION playback_guard[ACTOR_COUNT];
        GAME_SESSION session_guard;
        size_t got = 0;
        int result = 0;
        if (argc != 2)
            return 4;
        in = xport_fopen("../status/menu/animation-base-jobs.bin", "rb");
        out = xport_fopen("../status/menu/animation-base-native.bin", "wb");
        if (!in || !out)
        {
            if (in)
                fclose(in);
            if (out)
                fclose(out);
            return 4;
        }
        if (fread(header, sizeof(header), 1, in) != 1 || memcmp(header, input_header, sizeof(header)) || fwrite(output_header, sizeof(output_header), 1, out) != 1)
            result = 4;
        while (!result && (got = fread(job, 1, sizeof(job), in)) == sizeof(job))
        {
            ACTOR_ANIMATION *playback = actor_animations;
            if (job[0] > 1 || job[1] > 1 || fread(stream.bytes, sizeof(stream.bytes), 1, in) != 1 || cases >= 158)
            {
                result = 4;
                break;
            }
            memset(playback, 0xa5, sizeof(*playback));
            memset(&playback->src, 0, sizeof(playback->src));
            if (job[1])
            {
                playback->src.kind = ACTOR_ANIMATION_SOURCE_STREAM;
                playback->src.stream = (ANIMATION_STREAM){(const ANIMATION_STREAM_HEADER *)stream.bytes, sizeof(stream.bytes), ANIMATION_STREAM_NATIVE};
            }
            playback->id = job[3];
            memcpy(playback->motion.bits, job + 4, sizeof(playback->motion.bits));
            playback->motion.events = job[7];
            game_session.stage = job[2];
            memcpy(playback_guard, actor_animations, sizeof(playback_guard));
            memcpy(&session_guard, &game_session, sizeof(session_guard));
            memcpy(stream_guard, stream.bytes, sizeof(stream_guard));
            fields[0] = 0;
            fields[1] = 0;
            if (!job[0] && !job[1])
            {
                ++nulls;
                actor_pose_load_initial_motion(0);
            }
            else if (job[0])
                fields[1] = (uint32)animation_scale_vertical_motion(0);
            else
                actor_pose_load_initial_motion(0);
            playback_guard[0].motion.bits[0] = playback->motion.bits[0];
            playback_guard[0].motion.bits[2] = playback->motion.bits[2];
            playback_guard[0].motion.events = playback->motion.events;
            if (memcmp(playback_guard, actor_animations, sizeof(playback_guard)) || memcmp(&session_guard, &game_session, sizeof(session_guard)) || memcmp(stream_guard, stream.bytes, sizeof(stream_guard)))
                abort();
            fields[2] = playback->id;
            memcpy(fields + 3, playback->motion.bits, sizeof(playback->motion.bits));
            fields[6] = playback->motion.events;
            if (fwrite(fields, sizeof(fields), 1, out) != 1)
                result = 4;
            ++cases;
        }
        if (ferror(in) || (!result && got) || cases != 158 || nulls != 6 || ferror(out))
            result = 4;
        actor_animations[0].src = (ACTOR_ANIMATION_SOURCE){0};
        if (fclose(in))
            result = 4;
        if (fclose(out))
            result = 4;
        if (!result)
            diagnostic_end();
        return result;
    }
    if (argc > 1 && strcmp(argv[1], "--audit-camera-matrix") == 0)
        return geometry_audit_run(0);
    if (argc > 1 && strcmp(argv[1], "--audit-matrix-stack") == 0)
        return geometry_audit_run(1);
    if (argc > 1 && strcmp(argv[1], "--audit-ordering") == 0)
    {
        FILE *in, *out;
        uint32 args[2], value;
        uint8 state[8192];
        RENDER_BUFFERS unchanged;
        size_t got = 0;
        int result = 0;
        in = xport_fopen("../status/menu/ordering-jobs.bin", "rb");
        out = xport_fopen("../status/menu/ordering-native.bin", "wb");
        if (!in || !out)
        {
            if (in)
                fclose(in);
            if (out)
                fclose(out);
            return 4;
        }
        while (!result && (got = fread(args, 1, sizeof(args), in)) == sizeof(args))
        {
            if (fread(state, 1, sizeof(state), in) != sizeof(state))
            {
                result = 4;
                break;
            }
            /* This fixed wire region is the GPU ordering table device storage */
            memcpy(psx_addr(0x1cb000, sizeof(state)), state, sizeof(state));
            render_buffers.ot_addr = args[1];
            unchanged = render_buffers;
            value = args[0] ? render_ot_compact() : render_ot_init(args[1]);
            if (memcmp(&unchanged, &render_buffers, sizeof(unchanged)))
                abort();
            if (fwrite(&value, sizeof(value), 1, out) != 1 || fwrite(psx_addr(0x1cb000, sizeof(state)), sizeof(state), 1, out) != 1)
                result = 4;
        }
        if (ferror(in) || (!result && got))
            result = 4;
        if (fclose(in))
            result = 4;
        if (fclose(out))
            result = 4;
        if (!result)
            diagnostic_end();
        return result;
    }
    if (argc > 1 && strcmp(argv[1], "--audit-horizon") == 0)
        return geometry_audit_run(2);
    if (argc > 1 && strcmp(argv[1], "--audit-projection") == 0)
        return geometry_audit_run(3);
    if (argc > 1 && strcmp(argv[1], "--audit-rotation") == 0)
        return geometry_audit_run(4);
    if (argc > 1 && strcmp(argv[1], "--audit-camera") == 0)
    {
        FILE *in, *out;
        uint8 state[82];
        uint32 observation[22];
        const uint32 header[4] = {0x32434e46, 1, 22, 0};
        CAMERA unchanged;
        size_t got = 0;
        int result = 0;
        in = xport_fopen("../status/menu/camera-jobs.bin", "rb");
        out = xport_fopen("../status/menu/camera-native.bin", "wb");
        if (!in || !out)
        {
            if (in)
                fclose(in);
            if (out)
                fclose(out);
            return 4;
        }
        if (fwrite(header, sizeof(header), 1, out) != 1)
            result = 4;
        while (!result && (got = fread(state, 1, sizeof(state), in)) == sizeof(state))
        {
            /* Decode only this diagnostic job's explicit fixed-width fields */
            memset(&camera, 0xa5, sizeof(camera));
            memcpy(&camera.angles.yaw_bits, state + 0, 2);
            memcpy(&camera.angles.pitch_bits, state + 2, 2);
            memcpy(&camera.pos.x.bits, state + 20, 4);
            memcpy(&camera.pos.y.bits, state + 24, 4);
            memcpy(&camera.pos.z.bits, state + 28, 4);
            memcpy(&camera.motion.origin.x.bits, state + 32, 4);
            memcpy(&camera.motion.origin.y.bits, state + 36, 4);
            memcpy(&camera.motion.origin.z.bits, state + 40, 4);
            memcpy(&camera.motion.delta.x.bits, state + 44, 4);
            memcpy(&camera.motion.delta.y.bits, state + 48, 4);
            memcpy(&camera.motion.delta.z.bits, state + 52, 4);
            memcpy(&camera.motion.target.x.bits, state + 56, 4);
            memcpy(&camera.motion.target.y.bits, state + 60, 4);
            memcpy(&camera.motion.target.z.bits, state + 64, 4);
            memcpy(&camera.motion.origin_angles.yaw_bits, state + 68, 2);
            memcpy(&camera.motion.origin_angles.pitch_bits, state + 70, 2);
            memcpy(&camera.motion.delta_angles.yaw_bits, state + 72, 2);
            memcpy(&camera.motion.delta_angles.pitch_bits, state + 74, 2);
            memcpy(&camera.motion.target_angles.yaw_bits, state + 76, 2);
            memcpy(&camera.motion.target_angles.pitch_bits, state + 78, 2);
            memcpy(&camera.motion.phase_bits, state + 80, 2);
            memcpy(&unchanged, &camera, sizeof(camera));
            observation[0] = camera_update_transition();
            observation[1] = camera.angles.yaw_bits;
            observation[2] = camera.angles.pitch_bits;
            observation[3] = camera.pos.x.bits;
            observation[4] = camera.pos.y.bits;
            observation[5] = camera.pos.z.bits;
            observation[6] = camera.motion.origin.x.bits;
            observation[7] = camera.motion.origin.y.bits;
            observation[8] = camera.motion.origin.z.bits;
            observation[9] = camera.motion.delta.x.bits;
            observation[10] = camera.motion.delta.y.bits;
            observation[11] = camera.motion.delta.z.bits;
            observation[12] = camera.motion.target.x.bits;
            observation[13] = camera.motion.target.y.bits;
            observation[14] = camera.motion.target.z.bits;
            observation[15] = camera.motion.origin_angles.yaw_bits;
            observation[16] = camera.motion.origin_angles.pitch_bits;
            observation[17] = camera.motion.delta_angles.yaw_bits;
            observation[18] = camera.motion.delta_angles.pitch_bits;
            observation[19] = camera.motion.target_angles.yaw_bits;
            observation[20] = camera.motion.target_angles.pitch_bits;
            observation[21] = camera.motion.phase_bits;
            /* Observe all owned fields and reject writes outside this operation */
            unchanged.angles.yaw_bits = camera.angles.yaw_bits;
            unchanged.angles.pitch_bits = camera.angles.pitch_bits;
            unchanged.pos.x.bits = camera.pos.x.bits;
            unchanged.pos.y.bits = camera.pos.y.bits;
            unchanged.pos.z.bits = camera.pos.z.bits;
            unchanged.motion.target.x.bits = camera.motion.target.x.bits;
            unchanged.motion.target.y.bits = camera.motion.target.y.bits;
            unchanged.motion.target.z.bits = camera.motion.target.z.bits;
            unchanged.motion.target_angles.yaw_bits = camera.motion.target_angles.yaw_bits;
            unchanged.motion.target_angles.pitch_bits = camera.motion.target_angles.pitch_bits;
            unchanged.motion.phase_bits = camera.motion.phase_bits;
            if (memcmp(&unchanged, &camera, sizeof(camera)))
                abort();
            if (fwrite(observation, sizeof(observation), 1, out) != 1)
                result = 4;
        }
        if (ferror(in) || (!result && got))
            result = 4;
        if (fclose(in))
            result = 4;
        if (fclose(out))
            result = 4;
        if (!result)
            diagnostic_end();
        return result;
    }
    if (argc > 1 && strcmp(argv[1], "--audit-input") == 0)
        return audit_input_run();
    if (argc > 1 && strcmp(argv[1], "--audit-sound") == 0)
        return audit_sound_run();
    if (argc > 1 && strcmp(argv[1], "--audit-pitch") == 0)
    {
        FILE *in, *out;
        uint32 args[4];
        uint16 value;
        size_t got = 0;
        int result = 0;
        /* The shared SDK owns the immutable table and needs no game state */
        in = xport_fopen("../status/menu/pitch-jobs.bin", "rb");
        out = xport_fopen("../status/menu/pitch-native.bin", "wb");
        if (!in || !out)
        {
            if (in)
                fclose(in);
            if (out)
                fclose(out);
            return 4;
        }
        while (!result && (got = fread(args, 1, sizeof(args), in)) == sizeof(args))
        {
            value = _spu_note2pitch((uint16)args[0], (uint16)args[1], (uint16)args[2], (uint16)args[3]);
            if (fwrite(&value, sizeof(value), 1, out) != 1)
                result = 4;
        }
        if (ferror(in) || (!result && got))
            result = 4;
        if (fclose(in))
            result = 4;
        if (fclose(out))
            result = 4;
        if (!result)
            diagnostic_end();
        return result;
    }
    if (argc > 1 && strcmp(argv[1], "--platform-smoke") == 0)
    {
        limit = argc > 2 ? (uint32)strtoul(argv[2], 0, 0) : 120;
        headless = argc > 3 && strcmp(argv[3], "--headless") == 0;
    }
    else
    {
        DIAGNOSTIC(fprintf(stderr, "Unknown or retired diagnostic command\n"));
        return 3;
    }
    xport_set_headless(headless);
    ResetGraph(0);
    while (!xport_isquit() && frame < limit)
    {
        POLY_G3 p;
        gpu_begin();
        memset(&p, 0, sizeof(p));
        setPolyG3(&p);
        p.p[0].rgbc.r = 255;
        p.p[1].rgbc.g = 255;
        p.p[2].rgbc.b = 255;
        p.p[0].xy.x = 160;
        p.p[0].xy.y = 20;
        p.p[1].xy.x = 30;
        p.p[1].xy.y = 210;
        p.p[2].xy.x = 290;
        p.p[2].xy.y = 210;
        gpu_packet(&p);
        gpu_present();
        VSync(0);
        ++frame;
    }
    DIAGNOSTIC(printf("platform_frames %u pad %08X\n", frame, PadRead(0)));
    if (headless && !gpu_save_frame("../status/menu/platform-frame.bgrx"))
        return 8;
    return 0;
#endif
}
