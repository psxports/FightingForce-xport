#ifndef HD_H
#define HD_H

#include "vid.h"
#include "actor_pose.h"
#include "texture.h"
#include "model_object.h"
#include "effects.h"

/* Rows contain Q12 rotation converted to float and Q16 translation in world units */
void hd_matrix_rows(const POSE_MATRIX *matrix, float rows[12]);

/* Resources belong to this producer; the host owns the graphics context */
int hd_init(void);
void hd_reset(void);
void hd_free(void);
VID_TEXTURE *hd_vram_sync(void);

enum
{
    HD_TEX_TEXTURED = 1,
    HD_TEX_RAW = 2,
    HD_TEX_FILTERED = 4,
    HD_TEX_REPEAT_X = 8,
    HD_TEX_REPEAT_Y = 16,
    HD_TEX_DESCRIPTOR = 32,
    HD_TEX_RIG = 64,
    HD_TEX_SEMI = 128,
    HD_TEX_CLASSIFIED = 256,
    HD_TEX_TERRAIN = 512,
    HD_TEX_SKY = 1024,
    HD_TEX_DISTANT_GROUND = 2048,
    HD_TEX_BACKGROUND_MASK = 4096,
    HD_TEX_LINE = 8192,
    HD_TEX_BILLBOARD = 16384,
    HD_TEX_RIBBON = 32768,
    HD_DRAW_VECTORS = 8
};

typedef enum HD_BLEND
{
    HD_BLEND_OPAQUE,
    HD_BLEND_HALF,
    HD_BLEND_ADD,
    HD_BLEND_SUBTRACT,
    HD_BLEND_QUARTER,
    HD_BLEND_COUNT
} HD_BLEND;

/* Texture attributes hold tpage, CLUT, flags and packed (width-1,height-1) */
/* Rectangle attributes hold the logical image origin in page texels */
/* World descriptor UVs are decoded on GPU; UI UVs use local image texels */
/* Draw vectors hold matrix rows, projection, tint, pose and source material offsets */

typedef struct HD_RIG_PART
{
    uint32 first_vertex, vertex_count;
    MATRIXF matrix;
} HD_RIG_PART;

typedef struct HD_POSE_SAMPLE
{
    MATRIXF attachment[ACTOR_POSE_MATRIX_COUNT];
    VECTORF point[ACTOR_POSE_POINT_COUNT];
    uint32 attachment_mask, point_mask;
    uint32 attachment_part[ACTOR_POSE_MATRIX_COUNT];
} HD_POSE_SAMPLE;

/* Evaluate the native rig without GTE commands or writes to the actor pose */
uint32 hd_pose_parts(MODEL_REFERENCE model, const ACTOR_POSE *pose, const MATRIXF *base, HD_RIG_PART *parts, uint32 capacity, HD_POSE_SAMPLE *sample);

/* Tables are finalized before vid_end; all queued objects sample that image */
int hd_descriptors_sync(const TEXTURE_DESCRIPTORS *descriptors, const TEXTURE_IMAGE *images);
void hd_pose_begin(void);
uint32 hd_pose_upload(const HD_RIG_PART *parts, uint32 count);
void hd_pose_commit(void);

typedef struct HD_FACE_RANGE
{
    uint32 first, count;
    uint16 flags, descriptor;
} HD_FACE_RANGE;

typedef struct HD_BOUNDS
{
    float min[3], max[3];
} HD_BOUNDS;

typedef struct HD_MODEL
{
    VID_GEOMETRY *mesh;
    const VID_VERTEX16 *vertices;
    VID_INDEX *indices;
    HD_FACE_RANGE *faces;
    HD_BOUNDS *bounds;
    uint32 bounds_count;
    uint32 face_count, index_count, line_count, unsupported_lines;
    MODEL_REFERENCE source;
    int rig;
} HD_MODEL;

/* Borrowed cache entry; invalidation/reset releases its resource references */
const HD_MODEL *hd_model_mesh(uint32 type, MODEL_REFERENCE model, int rig);
void hd_model_invalidate(uint32 type);

typedef enum HD_ACTOR_PART
{
    HD_ACTOR_BODY,
    HD_ACTOR_SHADOW,
    HD_ACTOR_RIGHT,
    HD_ACTOR_LEFT,
    HD_ACTOR_GHOST0,
    HD_ACTOR_GHOST1,
    HD_ACTOR_VAN_FRONT,
    HD_ACTOR_VAN_BODY,
    HD_ACTOR_VAN_BACK,
    HD_ACTOR_VAN_MARKER,
    HD_ACTOR_PART_COUNT
} HD_ACTOR_PART;

void hd_actor_part(uint32 actor, HD_ACTOR_PART part);
void hd_actor_part_end(void);

typedef enum HD_OBJECT_POOL
{
    HD_OBJECT_WORLD,
    HD_OBJECT_DYNAMIC,
    HD_OBJECT_EFFECT
} HD_OBJECT_POOL;

typedef enum HD_OBJECT_PART
{
    HD_OBJECT_PRIMARY,
    HD_OBJECT_MIRROR,
    HD_OBJECT_ATTACHMENT0,
    HD_OBJECT_ATTACHMENT1,
    HD_OBJECT_ATTACHMENT2,
    HD_OBJECT_MARKER,
    HD_OBJECT_LIGHT0,
    HD_OBJECT_LIGHT1,
    HD_OBJECT_LIGHT2,
    HD_OBJECT_LIGHT3,
    HD_OBJECT_LIGHT4,
    HD_OBJECT_LIGHT5,
    HD_OBJECT_OUTER,
    HD_OBJECT_SECOND_POINT,
    HD_OBJECT_COUNTDOWN
} HD_OBJECT_PART;

void hd_object_begin(HD_OBJECT_POOL pool, uint32 slot, uint32 kind);
void hd_object_part(HD_OBJECT_PART part);
void hd_object_end(void);
void hd_object_reset(HD_OBJECT_POOL pool, uint32 slot);
void hd_trail_reset(uint32 resource);
void hd_trail_append(uint32 resource, int reset);
void hd_vblank(void);
void hd_vblank_reset(void);
sint32 hd_vsync(void);

enum
{
    HD_PRESENT_DEFERRED = 2
};

void hd_present(float phase);

/* Collection follows original frame boundaries; model submission is render-only */
void hd_collect_begin(float focal, float center_x, float center_y, uint32 width, uint32 height);
void hd_collect_seal(sint32 offset_x, sint32 offset_y);
void hd_actor_reset(uint32 actor);
void hd_history_reset(void);
void hd_collect_partial(void);
void hd_pages_clear(void);
void hd_frame_begin(int delayed);
void hd_frame_end(int present, uint32 draw_page);
void hd_collect_model(MODEL_REFERENCE model, const ACTOR_POSE *pose, const POSE_MATRIX *matrix, uint32 color, uint32 blend, sint32 bias, int faded);

typedef enum HD_OBJECT_DRAW
{
    HD_OBJECT_YAW,
    HD_OBJECT_FACING,
    HD_OBJECT_ROADSIDE
} HD_OBJECT_DRAW;

void hd_collect_culled_object(const MODEL_OBJECT *object, const POSE_MATRIX *matrix, HD_OBJECT_DRAW draw);
void hd_collect_culled_actor(uint32 actor, sint32 type, int player, const POSE_MATRIX *matrix);
void hd_collect_culled_spr(const MODEL_OBJECT *object, const EFFECT *effect, const POSE_MATRIX *matrix, const uint32 motion_sizes[8]);
void hd_collect_terrain(const uint16 tiles[64][64], uint32 column, uint32 row, const POSE_MATRIX *matrix, uint32 color, int faded);
int hd_models_draw(VID_RECT viewport);
int hd_world_draw(VID_RECT viewport);
int hd_terrain_draw(VID_RECT viewport);
void hd_render_begin(void);
/* Select the phase between consecutive completed snapshots for this render */
void hd_render_phase(float fract);

typedef enum HD_UI_LAYOUT
{
    HD_UI_CENTER,
    HD_UI_LEFT,
    HD_UI_RIGHT,
    HD_UI_STRETCH
} HD_UI_LAYOUT;

typedef enum HD_UI_LAYER
{
    HD_UI_BACKGROUND,
    HD_UI_OVERLAY,
    HD_UI_EFFECT
} HD_UI_LAYER;

typedef struct HD_UI_VERTEX
{
    float x, y, u, v;
    uint32 color;
} HD_UI_VERTEX;

typedef struct HD_MASK_VERTEX
{
    float x, y, z, offset_y;
} HD_MASK_VERTEX;

typedef enum HD_BACKGROUND_PART
{
    HD_BACKGROUND_NONE,
    HD_BACKGROUND_SKY_GRADIENT,
    HD_BACKGROUND_HORIZON_COVER,
    HD_BACKGROUND_GROUND_COVER,
    HD_BACKGROUND_LOWER_FILL,
    HD_BACKGROUND_MASK_LEFT,
    HD_BACKGROUND_MASK_RIGHT,
    HD_BACKGROUND_MASK_WAVE,
    HD_BACKGROUND_PARALLAX
} HD_BACKGROUND_PART;

enum
{
    HD_HORIZON_TOP = 1,
    HD_HORIZON_BOTTOM = 2,
    HD_HORIZON_CLAMP_TOP = 4
};

void hd_collect_background_gradient(float top, float bottom, uint32 top_color, uint32 bottom_color, HD_BACKGROUND_PART part, uint32 horizon_edges);
HD_UI_LAYOUT hd_ui_layout(HD_UI_LAYOUT layout);
void hd_collect_ui(const HD_UI_VERTEX vertices[4], uint16 page, uint16 clut, uint16 flags, uint32 bucket, HD_UI_LAYER layer);
void hd_collect_ui_image(const HD_UI_VERTEX vertices[4], uint16 page, uint16 clut, uint16 flags, uint32 bucket, HD_UI_LAYER layer, const TEXTURE_IMAGE *image);
void hd_collect_ui_sprite(sint32 x, sint32 y, uint32 texture, uint32 bucket);
void hd_collect_ui_scaled_sprite(sint32 x, sint32 y, sint32 depth, uint32 size, uint32 texture, uint32 color, uint32 bucket, int filtered);
void hd_collect_ui_gradient(sint32 x, sint32 y, uint32 texture, const uint32 colors[4], uint32 bucket);
int hd_ui_draw(VID_RECT viewport, HD_UI_LAYER layer);
void hd_collect_parallax(float top);
void hd_collect_sky(float scroll, float top, float height, uint8 center_v, uint8 side_v);
void hd_collect_ground(const uint8 colors[16384], float x, float y, float z, uint16 yaw, float top, uint32 bucket, int filtered);
void hd_collect_mask(const HD_MASK_VERTEX vertices[4], uint32 bucket, HD_BACKGROUND_PART part);
void hd_collect_sprite(const POSE_MATRIX *matrix, VECTORF local, float size, float depth_bias, uint32 texture, uint32 color);
void hd_collect_ribbon(const POSE_MATRIX *matrix, VECTORF a, VECTORF b, float size_a, float size_b, float depth_bias, uint32 texture, uint32 color, uint32 blend);
void hd_collect_sprite_fixed(const POSE_MATRIX *matrix, VECTORF local, float offset_x, float offset_y, float depth_bias, uint32 texture);

typedef enum HD_PARTICLE_SHAPE
{
    HD_PARTICLE_POINT,
    HD_PARTICLE_LINE,
    HD_PARTICLE_QUAD
} HD_PARTICLE_SHAPE;

void hd_collect_particle(const POSE_MATRIX *matrix, VECTORF local, float offset_x, uint32 color, HD_PARTICLE_SHAPE shape);
void hd_collect_horizon(const POSE_MATRIX *matrix);
float hd_horizon(void);

const MATRIXF *hd_transform_current(const POSE_MATRIX *slot);
void hd_transform_identity(const POSE_MATRIX *slot);
void hd_transform_copy(const POSE_MATRIX *destination, const POSE_MATRIX *source);
void hd_transform_load(const POSE_MATRIX *destination, const POSE_MATRIX *source);
void hd_transform_translate(const POSE_MATRIX *slot, sint32 x, sint32 y, sint32 z);
void hd_transform_rotate(const POSE_MATRIX *slot, uint32 axis, uint32 angle);
void hd_transform_scale(const POSE_MATRIX *slot, sint32 x, sint32 y, sint32 z);

#endif
