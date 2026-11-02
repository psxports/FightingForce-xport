#include "hd.h"
#include "psx_gpu.h"
#include "vid_psx_sampling.h"
#include "transform.h"
#include "depth_table.h"
#include "render_style.h"
#include "render_buffers.h"
#include "game_session.h"
#include "animation.h"
#include "actor_items.h"
#include "dynamic_objects.h"
#include "world_objects.h"
#include "game_pause.h"
#include "camera.h"
#include <string.h>

#define HD_VERTEX_BONE_MASK 65535u
#define HD_VERTEX_CORNER_SHIFT 16
#define HD_VERTEX_TWO_SIDED 262144u
#define HD_VERTEX_TERRAIN 524288u
#define HD_VERTEX_LINE 1048576u
#define HD_VERTEX_FLIPPED 2097152u
#define HD_PASS_FADED 1u
#define HD_PASS_ORDERED 2u
#define HD_STRING_VALUE_(value) #value
#define HD_STRING_VALUE(value) HD_STRING_VALUE_(value)

static const char model_vertex[] = "#define HD_VERTEX_BONE_MASK " HD_STRING_VALUE(
    HD_VERTEX_BONE_MASK) "\n"
                         "#define HD_VERTEX_CORNER_SHIFT " HD_STRING_VALUE(HD_VERTEX_CORNER_SHIFT) "\n"
                                                                                                   "#define HD_VERTEX_TWO_SIDED " HD_STRING_VALUE(HD_VERTEX_TWO_SIDED) "\n"
                                                                                                                                                                       "#define HD_VERTEX_TERRAIN " HD_STRING_VALUE(
                                                                                                                                                                           HD_VERTEX_TERRAIN) "\n"
                                                                                                                                                                                              "#define HD_VERTEX_LINE " HD_STRING_VALUE(
                                                                                                                                                                                                  HD_VERTEX_LINE) "\n"
                                                                                                                                                                                                                  "#define HD_VERTEX_FLIPPED " HD_STRING_VALUE(
                                                                                                                                                                                                                      HD_VERTEX_FLIPPED) "\n"
                                                                                                                                                                                                                                         "#define HD_PASS_FADED " HD_STRING_VALUE(
                                                                                                                                                                                                                                             HD_PASS_FADED) "\n"
                                                                                                                                                                                                                                                            "#define HD_PASS_ORDERED " HD_STRING_VALUE(
                                                                                                                                                                                                                                                                HD_PASS_ORDERED) "\n"
                                                                                                                                                                                                                                                                                 "layout(location=0) in vec4 aPos;\n"
                                                                                                                                                                                                                                                                                 "layout(location=1) in vec2 aUV;\n"
                                                                                                                                                                                                                                                                                 "layout(location=2) in vec4 aColor;\n"
                                                                                                                                                                                                                                                                                 "layout(location=3) in vec4 aNormal;\n"
                                                                                                                                                                                                                                                                                 "layout(location=4) in vec4 aExtra;\n"
                                                                                                                                                                                                                                                                                 "uniform vec4 uDraw[8];\n"
                                                                                                                                                                                                                                                                                 "uniform highp usampler2D uAux0;\n"
                                                                                                                                                                                                                                                                                 "uniform highp usampler2D uAux1;\n"
                                                                                                                                                                                                                                                                                 "out vec2 vUV;out vec4 vColor;out float vDepth;out vec2 vFog;\n"
                                                                                                                                                                                                                                                                                 "flat out uvec4 vTex;flat out uvec2 vRect;\n"
                                                                                                                                                                                                                                                                                 "uvec4 tableWord(highp usampler2D image,uint index){uint w=uint(textureSize(image,0).x);return texelFetch(image,ivec2(int(index%w),int(index/w)),0);}\n"
                                                                                                                                                                                                                                                                                 "vec4 poseRow(uint index){uvec4 a=tableWord(uAux1,index),b=tableWord(uAux1,index+1u);return vec4(uintBitsToFloat(a.x|(a.y<<16)),uintBitsToFloat(a.z|(a.w<<16)),uintBitsToFloat(b.x|(b.y<<16)),uintBitsToFloat(b.z|(b.w<<16)));}\n"
                                                                                                                                                                                                                                                                                 "vec3 viewPoint(vec4 source){vec4 p=vec4(source.xyz,1.0);if(source.w!=0.0){uint part=uint(uDraw[5].w)+(uint(source.w)-1u)*6u;p=vec4(dot(poseRow(part),p),dot(poseRow(part+2u),p),dot(poseRow(part+4u),p),1.0);}return vec3(dot(uDraw[0],p),dot(uDraw[1],p),dot(uDraw[2],p));}\n"
                                                                                                                                                                                                                                                                                 "vec4 projectPoint(vec3 p){return vec4(p.xy*uDraw[3].xy+p.z*uDraw[3].zw,p.z*uDraw[5].x+uDraw[5].y,p.z);}\n"
                                                                                                                                                                                                                                                                                 "void main(){uint recordFlags=uint(aPos.w),corner=(recordFlags>>HD_VERTEX_CORNER_SHIFT)&3u;bool line=(recordFlags&HD_VERTEX_LINE)!=0u;bool terrain=(recordFlags&HD_VERTEX_TERRAIN)!=0u;uint descriptor=uint(aUV.x);vec3 local=aPos.xyz;vec3 terrainEnd=vec3(0.0);bool outside=false;if(terrain){uint col=uint(aPos.x),row=uint(aPos.z);uint tileIndex=(((uint(uDraw[7].y)-row)&63u)*64u+((uint(uDraw[7].x)-col)&63u));uvec4 entries=tableWord(uAux1,tileIndex/4u);uint tile=entries[tileIndex&3u];outside=tile==65532u;descriptor=tile/3u;uint x=col+(corner&1u),z=row+(corner>>1);local=vec3(float(x)*256.0-2048.0,x>=8u?1.0:0.0,float(z)*256.0-2048.0);terrainEnd=vec3(float(col+1u)*256.0-2048.0,col+1u>=8u?1.0:0.0,float(row+1u)*256.0-2048.0);}\n"
                                                                                                                                                                                                                                                                                 "vDepth=0.0;vFog=vec2(0.0);vUV=vec2(0.0);vRect=uvec2(0u);vTex=uvec4(0u);vColor=vec4(uDraw[4].rgb,1.0);bool clipped=outside;if(outside){gl_Position=vec4(2.0,2.0,2.0,1.0);return;}\n"
                                                                                                                                                                                                                                                                                 "uvec4 d=uvec4(0u),e=uvec4(0u),f=uvec4(0u);uint mode=uint(uDraw[6].z);bool semi=false;if(!line){uint index=descriptor*3u+uint(uDraw[6].w);d=tableWord(uAux0,index);e=tableWord(uAux0,index+1u);f=tableWord(uAux0,index+2u);semi=(!terrain&&(d.x&1u)!=0u)||(uint(uDraw[6].x)&1u)!=0u;if((mode&HD_PASS_ORDERED)==0u&&(semi||uDraw[4].w!=0.0)){gl_Position=vec4(2.0,2.0,2.0,1.0);return;}}\n"
                                                                                                                                                                                                                                                                                 "vec3 q=viewPoint(vec4(local,float(recordFlags&HD_VERTEX_BONE_MASK)));gl_Position=projectPoint(q);vDepth=q.z;\n"
                                                                                                                                                                                                                                                                                 "if(line){vec3 r=viewPoint(aColor);float depth=(q.z+r.z)*0.5;clipped=depth<=0.0||depth>32752.0||(q.z<1.0&&r.z<1.0);vTex.z=8192u;vColor=vec4(aNormal.xyz/255.0,1.0);\n"
                                                                                                                                                                                                                                                                                 "if(!clipped){if(q.z<1.0)q=mix(q,r,(1.0-q.z)/(r.z-q.z));if(r.z<1.0)r=mix(r,q,(1.0-r.z)/(q.z-r.z));vec2 a=q.xy*uDraw[3].xy/q.z+uDraw[3].zw,b=r.xy*uDraw[3].xy/r.z+uDraw[3].zw;\n"
                                                                                                                                                                                                                                                                                 "vec2 delta=(b-a)*uDraw[7].xy;float size=length(delta);vec2 dir=size>0.000001?delta/size:vec2(aExtra.x,0.0);vec2 offset=(-dir+vec2(-dir.y,dir.x)*aNormal.w)*uDraw[7].z/uDraw[7].xy;offset+=vec2(uDraw[7].w/uDraw[7].x,-uDraw[7].z/uDraw[7].y);gl_Position=vec4((a+offset)*q.z,q.z*uDraw[5].x+uDraw[5].y,q.z);\n"
                                                                                                                                                                                                                                                                                 "if((uint(uDraw[6].x)&2u)!=0u){vec3 normal=vec3(uDraw[0].y,uDraw[1].y,uDraw[2].y);vec3 ray=vec3((a+offset-uDraw[3].zw)/uDraw[3].xy,1.0);float divisor=dot(normal,ray);if(abs(divisor)>0.000001){float depth=dot(normal,q)/divisor;if(depth>=1.0)gl_Position.z=(depth*uDraw[5].x+uDraw[5].y)*(q.z/depth);}}}\n"
                                                                                                                                                                                                                                                                                 "}else{bool flipped=(recordFlags&HD_VERTEX_FLIPPED)!=0u;uint anchorA=flipped?1u:0u,anchorB=flipped?0u:1u,anchorC=flipped?2u:3u;\n"
                                                                                                                                                                                                                                                                                 "vec3 a=terrain?(corner==3u?q:viewPoint(vec4(terrainEnd,0.0))):(corner==anchorA?q:viewPoint(aColor));vec3 c=terrain?a:(corner==anchorC?q:viewPoint(aExtra));float distance=(a.z+c.z+uDraw[4].w)*0.5;\n"
                                                                                                                                                                                                                                                                                 "if((recordFlags&HD_VERTEX_TWO_SIDED)==0u){vec3 b=corner==anchorB?q:viewPoint(aNormal);if(dot(a,cross(b-a,c-a))<0.0)clipped=true;}bool faded=(mode&HD_PASS_FADED)!=0u;clipped=clipped||(!terrain&&distance>32768.0)||(faded&&distance>5120.0);\n"
                                                                                                                                                                                                                                                                                 "uint uv=corner==0u?(terrain?d.x:(d.x&65534u)):corner==1u?d.z:corner==2u?e.x:e.y;\n"
                                                                                                                                                                                                                                                                                 "vRect=e.zw;vUV=vec2(uv&255u,uv>>8)-vec2(vRect)+0.5;uint flags=1u|32u|256u|(terrain?512u:0u);bool raw=uDraw[5].z!=0.0;\n"
                                                                                                                                                                                                                                                                                 "if(faded){float fade=distance-3072.0;float shade=fade<=0.0?128.0:128.0-fade/16.0;vColor.rgb=vec3(floor(shade*256.0+0.5)/(255.0*256.0));raw=fade<0.0;}\n"
                                                                                                                                                                                                                                                                                 "if(raw)flags|=2u;if(f.y!=0u)flags|=4u;if(semi)flags|=128u;\n"
                                                                                                                                                                                                                                                                                 "uint page=d.w;if(uDraw[6].y!=0.0)page=(page&~96u)|((uint(uDraw[6].y)-1u)<<5);vTex=uvec4(page,d.y,flags,f.x);}\n"
                                                                                                                                                                                                                                                                                 "if((uint(uDraw[6].x)&2u)!=0u){float bias=(uint(uDraw[6].x)&4u)!=0u?4.0:1.0;gl_Position.z-=bias/max(gl_Position.w,1.0)+gl_Position.w*0.000000476837158203125;}\n"
                                                                                                                                                                                                                                                                                 "if(clipped)gl_Position=vec4(2.0,2.0,2.0,1.0);}\n";

static const char ui_vertex[] = "layout(location=0) in vec4 aPos;\n"
                                "layout(location=1) in vec2 aUV;\n"
                                "layout(location=2) in vec4 aColor;\n"
                                "layout(location=4) in float aAlpha;\n"
                                "layout(location=5) in uvec4 aTex;\n"
                                "layout(location=6) in uvec2 aRect;\n"
                                "uniform vec4 uDraw[8];\n"
                                "out vec2 vUV;out vec4 vColor;out float vDepth;out vec2 vFog;\n"
                                "flat out uvec4 vTex;flat out uvec2 vRect;\n"
                                "void main(){gl_Position=aPos;vUV=aUV;vColor=vec4(aColor.rgb/255.0,aAlpha)*uDraw[4];vTex=aTex;vRect=aRect;"
                                "if((aTex.z&4096u)!=0u){gl_Position=vec4(aPos.xy*uDraw[0].xy+aPos.z*uDraw[0].zw,aPos.z*uDraw[1].x+uDraw[1].y,aPos.z);"
                                "gl_Position.y-=aAlpha*uDraw[1].z*aPos.z;vColor=vec4(0.0,0.0,0.0,1.0);}"
                                "if((aTex.z&16384u)!=0u){float depth=max(aPos.z+aPos.w,1.0);"
                                "vec2 corner=vec2(float(aRect.x&1u),float((aRect.x>>1)&1u));"
                                "vec2 extent=vec2(float(aRect.y),float(aTex.w))*aAlpha/depth;"
                                "vec2 span=abs(2.0*trunc(extent)-1.0);"
                                "bool oversized=(aRect.x&4u)==0u&&(span.x>1023.0||span.y>511.0);"
                                "vec2 offset=(corner*2.0-1.0)*extent-corner;"
                                "if((aRect.x&4u)!=0u){extent=vec2(float(aRect.y),float(aTex.w));offset=corner*extent+uDraw[2].xy;vUV=aUV+corner*extent;}"
                                "gl_Position=vec4(aPos.xy*uDraw[0].xy+aPos.z*uDraw[0].zw,aPos.z*uDraw[1].x+uDraw[1].y,aPos.z);"
                                "gl_Position.z=(depth*uDraw[1].x+uDraw[1].y)*(aPos.z/depth);"
                                "gl_Position.xy+=offset*vec2(uDraw[1].w,-uDraw[1].z)*aPos.z;"
                                "vColor=vec4(aColor.rgb/255.0,1.0);vRect=uvec2(uDraw[2].zw);vTex.w=uint(uDraw[3].x);"
                                "vUV-=vec2(vRect.x&255u,vRect.y&255u);"
                                "if(oversized||aPos.z<1.0||aPos.z+aPos.w<=0.0)gl_Position=vec4(2.0,2.0,2.0,1.0);}"
                                "if((aTex.z&32768u)!=0u){bool end=aRect.x>=2u;vec4 q=vec4(aPos.xyz,aAlpha),r=end?uDraw[2]:uDraw[3];"
                                "float width=float(int(aRect.y)-((aRect.y&32768u)!=0u?65536:0));float radius=width*q.w;"
                                "float otherWidth=end?uDraw[7].x:uDraw[7].y;vec4 uvOther=end?uDraw[5]:uDraw[6];"
                                "vec2 otherUV=(aRect.x&1u)!=0u?uvOther.zw:uvOther.xy;"
                                "bool clipped=q.z<1.0&&r.z<1.0;"
                                "if(q.z<1.0&&!clipped){float t=(1.0-q.z)/(r.z-q.z);radius=mix(radius,otherWidth*r.w,t);vUV=mix(vUV,otherUV,t);q=mix(q,r,t);}"
                                "if(r.z<1.0&&!clipped)r=mix(r,q,(1.0-r.z)/(q.z-r.z));"
                                "vec2 delta=r.xy/max(r.z,1.0)-q.xy/max(q.z,1.0);if(end)delta=-delta;"
                                "float lengthSquared=dot(delta,delta);vec2 normal=lengthSquared>0.000000000001?vec2(delta.y,-delta.x)*inversesqrt(lengthSquared):vec2(0.0);"
                                "vec2 offset=normal*radius/max(q.z+aPos.w,1.0)*((aRect.x&1u)!=0u?1.0:-1.0);"
                                "gl_Position=vec4(q.xy*uDraw[0].xy+q.z*uDraw[0].zw,q.z*uDraw[1].x+uDraw[1].y,q.z);"
                                "gl_Position.xy+=offset*vec2(uDraw[1].w,-uDraw[1].z)*q.z;"
                                "vColor=vec4(aColor.rgb/255.0,1.0);vRect=uvec2(0u);vTex.w=65535u;"
                                "if(clipped)gl_Position=vec4(2.0,2.0,2.0,1.0);}"
                                "vDepth=0.0;vFog=vec2(0.0);}\n";

static const char fragment[] = "flat in uvec4 vTex;flat in uvec2 vRect;\n"
                               "uniform vec4 uDraw[8];uniform highp usampler2D uAux1;\n"
                               "#define VID_PSX_ADDRESS sourceAddress\n"
                               "ivec2 sourceAddress(ivec2 pixel,ivec4 region,uint repeatAxes);\n"
                               "uint vidPsxWord(ivec2 address){"
                               "uvec4 c=uvec4(round(texelFetch(uTex,address,0)*vec4(31.0,31.0,31.0,1.0)));"
                               "return c.r|(c.g<<5)|(c.b<<10)|(c.a<<15);}\n" VID_PSX_SAMPLING_GLSL "ivec2 sourceAddress(ivec2 pixel,ivec4 region,uint repeatAxes){"
                               "if((vTex.z&1024u)==0u)return vidPsxAddress(pixel,region,repeatAxes);"
                               "uint bank=((uint(pixel.x)>>8)&1u)==0u?(vTex.w&255u):(vTex.w>>8);"
                               "return ivec2(int(uint(pixel.x)&255u),clamp(pixel.y,0,int(vRect.y))+int(bank));}\n"
                               "vec4 groundTap(ivec2 position){uvec2 cell=uvec2(position)&uvec2(127u);"
                               "uint index=cell.y*128u+cell.x,word=(vTex.x|(vTex.y<<16))+(index>>3);"
                               "uint width=uint(textureSize(uAux1,0).x);uvec4 entries=texelFetch(uAux1,ivec2(int(word%width),int(word/width)),0);"
                               "uint entry=(entries[(index>>1)&3u]>>((index&1u)*8u))&255u;"
                               "uint palette=vidPsxWord(ivec2(int((vTex.w&63u)*16u+entry),int(vTex.w>>6))&ivec2(1023,511));"
                               "return vidPsxColor(palette,0u,false);}\n"
                               "void main(){vec4 color=vColor;uint flags=vTex.z;"
                               "if((flags&2048u)!=0u){float q=uDraw[0].y/(vUV.y+1.0);"
                               "vec2 local=vec2((vUV.x-160.0)*uDraw[1].x-256.0*uDraw[0].w,(vUV.x-160.0)*uDraw[0].w+256.0*uDraw[1].x);"
                               "vec2 uv=(q*local-vec2(1.0)-uDraw[0].xz)/128.0;"
                               "color=groundTap(ivec2(floor(uv)));if(color.a==0.0)discard;"
                               "if((flags&4u)!=0u){vec2 p=uv-0.5,f=fract(p);ivec2 base=ivec2(floor(p));"
                               "vec4 a=groundTap(base),b=groundTap(base+ivec2(1,0)),c=groundTap(base+ivec2(0,1)),d=groundTap(base+ivec2(1,1));"
                               "vec4 sum=mix(mix(a,b,f.x),mix(c,d,f.x),f.y);if(sum.a==0.0)discard;color.rgb=sum.rgb/sum.a;}"
                               "outColor=color;return;}"
                               "if((flags&1u)!=0u){ivec2 origin=ivec2(vRect&uvec2(255u,255u));"
                               "ivec4 region=ivec4(origin,ivec2((vTex.w&255u)+1u,(vTex.w>>8)+1u));"
                               "uint repeatAxes=(flags>>3)&3u;uint stpClass=uint(uParams[0].x);"
                               "if((flags&256u)!=0u){bool semi=(flags&128u)!=0u;if(stpClass==0u)stpClass=semi?1u:0u;"
                               "else if(stpClass==2u&&!semi)discard;}"
                               "ivec2 address=ivec2(floor(vUV))+origin;"
                               "if((vRect.x&256u)!=0u)address=clamp(address,region.xy,region.xy+region.zw-1);"
                               "if((flags&1024u)!=0u){region=ivec4(0,0,256,256);"
                               "address=sourceAddress(ivec2(floor(vUV)),region,repeatAxes);}"
                               "uint nearestWord=vidPsxDecode(vTex.x,vTex.y,address);"
                               "vec4 texel=vidPsxColor(nearestWord,stpClass,false);"
                               "if(texel.a!=0.0&&(flags&4u)!=0u)texel=vidPsxFilter(vUV,vTex.x,vTex.y,region,repeatAxes,stpClass,false);"
                               "if(texel.a==0.0)discard;"
                               "color.rgb=texel.rgb*((flags&2u)!=0u?vec3(1.0):vColor.rgb*(255.0/128.0));}"
                               "color.rgb*=uParams[0].y;color.a*=uParams[0].z;outColor=vidFog(color);}\n";

typedef struct HD_MODEL_DRAW
{
    uint32 type, color, blend, first_part, part_count;
    uint32 descriptor_bank, mutable_model, ordinal, actor;
    uint32 pass_first, pass_count;
    HD_ACTOR_PART actor_part;
    uint32 object_owner, object_next, previous_record;
    HD_OBJECT_PART object_part;
    sint32 bias;
    int faded;
    MATRIXF matrix;
    MATRIXF camera;
    HD_BOUNDS bounds;
} HD_MODEL_DRAW;

typedef struct HD_DESCRIPTOR_BANK
{
    TEXTURE_DESCRIPTORS descriptors;
    TEXTURE_IMAGE images[TEXTURE_DESCRIPTOR_CAPACITY];
} HD_DESCRIPTOR_BANK;

typedef struct HD_TERRAIN_DRAW
{
    MATRIXF matrix;
    MATRIXF camera;
    uint32 color, descriptor_bank;
    int faded;
    uint32 column, row, tile_map;
    VID_GEOMETRY *mesh;
} HD_TERRAIN_DRAW;

typedef struct HD_TERRAIN_MESH
{
    uint16 tiles[64][64];
    VID_TEXTURE *image;
    VID_MATERIAL *material[4];
} HD_TERRAIN_MESH;

typedef struct HD_UI_DRAW
{
    union
    {
        HD_UI_VERTEX vertices[4];
        HD_MASK_VERTEX mask[4];

        struct
        {
            VECTORF point[2];
            float size[2], depth_bias;
            uint32 color;
            uint16 uv[4];
            sint16 extent[2];
            float offset[2];
            int fixed;
        } effect;
    };

    uint16 page, clut, flags, bucket;
    uint16 sky_v;
    uint16 ground_map;
    uint16 region[2], extent;
    HD_UI_LAYOUT layout;
    HD_UI_LAYER layer;
    uint32 next, ordinal, object_owner, object_next, previous_record;
    uint32 hand_actor;
    MATRIXF camera;
    HD_OBJECT_PART object_part;
    HD_BACKGROUND_PART background_part;
    uint32 horizon_edges;
    sint16 parallax_yaw;
} HD_UI_DRAW;

typedef struct HD_GROUND_MAP
{
    uint8 colors[16384];
    float camera[5];
    MATRIXF pose;
} HD_GROUND_MAP;

typedef struct HD_MUTABLE_MODEL
{
    HD_MODEL model;
    uint32 type;
    uint32 appends;
    SVECTOR points[18];
} HD_MUTABLE_MODEL;

typedef struct HD_TRAIL_BUFFER
{
    HD_MODEL model;
    MODEL_BODY_HEADER header;
    MODEL_FACE_RECORD faces[9];
    SVECTOR points[20];
    uint32 type;
} HD_TRAIL_BUFFER;

typedef struct HD_ACTOR_HAND
{
    MATRIXF correction;
    uint32 slot, part;
    int valid;
} HD_ACTOR_HAND;

enum
{
    HD_OBJECT_COUNT = WORLD_OBJECT_COUNT + DYNAMIC_OBJECT_COUNT + EFFECT_COUNT
};

typedef struct HD_OBJECT_SAMPLE
{
    uint32 kind, model_head, model_count, ui_head, ui_count;
} HD_OBJECT_SAMPLE;

typedef struct HD_PASS_FACE
{
    uint32 face;
    HD_BLEND blend;
    int semi;
} HD_PASS_FACE;

typedef struct HD_FRAME
{
    HD_MODEL_DRAW *models;
    HD_PASS_FACE *pass_faces;
    uint32 pass_count, pass_capacity;
    HD_MUTABLE_MODEL *mutable_models;
    uint32 mutable_count, mutable_capacity;
    HD_RIG_PART *parts;
    uint32 model_count, model_capacity, part_count, part_capacity;
    uint32 ordinal, sequence, previous;
    uint32 stage, tick;
    int paused;
    uint32 trail_model[2];
    HD_OBJECT_SAMPLE objects[HD_OBJECT_COUNT];
    uint32 actor_model[ACTOR_COUNT][HD_ACTOR_PART_COUNT];
    uint32 actor_attachment[ACTOR_COUNT][ACTOR_POSE_MATRIX_COUNT];
    HD_ACTOR_HAND actor_hand[ACTOR_COUNT][2];
    float focal, center_x, center_y;
    float horizon;
    MATRIXF camera;
    MATRIXF horizon_view;
    int horizon_valid;
    uint32 width, height;
    sint32 offset_x, offset_y;
    uint32 missing;
    int valid, broken, partial;
    HD_DESCRIPTOR_BANK *banks;
    uint32 bank_count, bank_capacity;
    HD_TERRAIN_DRAW *terrain;
    uint32 terrain_count, terrain_capacity;
    HD_UI_DRAW *ui;
    uint32 ui_count, ui_capacity;
    uint32 ui_head[3][RENDER_OT_COUNT];
    HD_GROUND_MAP *ground_maps;
    uint32 ground_count, ground_capacity;
} HD_FRAME;

typedef struct HD_UI_BUFFER
{
    VID_GEOMETRY *mesh;
    uint32 capacity;
} HD_UI_BUFFER;

typedef struct HD_ORDERED_FACE
{
    uint32 model, face, bucket, next;
    HD_BLEND blend;
    int semi;
} HD_ORDERED_FACE;

typedef struct HD_RENDER_MODEL
{
    const HD_MODEL *model;
    float uniforms[HD_DRAW_VECTORS * 4];
    VID_INDEX *opaque;
    uint32 opaque_count;
} HD_RENDER_MODEL;

typedef struct HD_TRANSFORM
{
    MATRIXF matrix;
    int valid;
} HD_TRANSFORM;

enum
{
    HD_FRAME_COUNT = 3
};

typedef struct HD_MATRIX_SAMPLE
{
    float basis[3][3], scale[3];
    QUATF rotation;
    int status;
} HD_MATRIX_SAMPLE;

typedef struct HD_POSE_SPAN
{
    uint32 first, count, hash;
} HD_POSE_SPAN;

typedef struct HD_RESOURCES
{
    VID_TEXTURE *vram;
    uint32 vram_version;
    int vram_valid;
    VID_SHADER *shader[2];
    VID_MATERIAL *material[4][HD_BLEND_COUNT][3];
    VID_TEXTURE *descriptors, *poses, *ground_colors;
    uint32 pose_count;
    HD_POSE_SPAN *pose_spans;
    uint32 pose_span_count, pose_span_capacity;
    int descriptors_valid, poses_dirty;
    uint32 pose_dirty_first, pose_dirty_last;
    uint32 prepared_frame, prepared_sequence;
    uint32 ground_frame, ground_sequence;
    HD_MODEL model[MODEL_TYPE_COUNT];
    VID_GEOMETRY *terrain_grid;
    HD_TERRAIN_MESH *terrain_meshes;
    uint32 terrain_mesh_count, terrain_mesh_capacity;
    HD_FRAME frame[HD_FRAME_COUNT];
    uint32 trail_appends[2];
    HD_TRAIL_BUFFER **trail_buffers;
    uint32 trail_buffer_count, trail_buffer_capacity, trail_buffer_cursor;
    uint32 current_frame, last_frame, sequence;
    float render_fract;
    uint32 actor_owner, object_owner;
    HD_OBJECT_PART object_part;
    HD_ACTOR_PART actor_part;
    HD_RIG_PART *blend_parts;
    uint32 blend_capacity;
    int collecting;
    int frame_open, frame_hd, frame_raster, presenting, vram_locked;
    int wait_valid, wait_presented, wait_sealed, history_pending;
    uint32 vblank_count, wait_counter, clock_counter;
    uint64 clock_field;
    float clock_fract;
    sint32 display_offset[2];
    uint32 page_frame[2];
    uint32 page_vram_version[2];
    int page_ready[2], software_ready[2];
    HD_MATRIX_SAMPLE matrix_samples[256];

    struct
    {
        MATRIXF previous, current, view;
        float phase;
        int status;
    } camera_blend;

    HD_TRANSFORM transform[GEOMETRY_MATRIX_COUNT + GEOMETRY_MODEL_MATRIX_COUNT];
    HD_POSE_SAMPLE actor_pose[ACTOR_POSE_COUNT];
    HD_ORDERED_FACE *ordered;
    uint32 ordered_count, ordered_capacity;
    HD_ORDERED_FACE *traversal;
    uint32 traversal_capacity;
    uint32 order_head[RENDER_OT_COUNT];
    HD_RENDER_MODEL *render_model;
    uint32 render_capacity;
    HD_UI_LAYOUT ui_layout;
    VID_INDEX *quad_indices;
    uint32 quad_capacity;
    HD_UI_BUFFER *ui_buffers;
    uint32 ui_buffer_count, ui_buffer_capacity, ui_buffer_cursor;
    uint32 *ui_order;
    uint32 ui_order_capacity;
} HD_RESOURCES;

static HD_RESOURCES hd;

static int reserve_frame(void **storage, uint32 *capacity, uint32 needed, size_t stride);

enum
{
    HD_MISSING_LINES = 1,
    /* Region origins are bytes; bit 8 requests bounded UI nearest sampling */
    HD_RECT_CLAMP = 256,
    HD_MISSING_TERRAIN_BLEND = 8,
    HD_DESCRIPTOR_BANK_COUNT = 16,
    HD_GROUND_MAP_COUNT = 16
};

enum
{
    HD_TABLE_WIDTH = 256,
    HD_DESCRIPTOR_HEIGHT = (TEXTURE_DESCRIPTOR_CAPACITY * 3 + HD_TABLE_WIDTH - 1) / HD_TABLE_WIDTH,
    HD_POSE_CAPACITY = 4096,
    HD_GROUND_MAP_WORDS = 2048,
    HD_GROUND_MAP_BASE = 0,
    HD_POSE_HEIGHT = (HD_POSE_CAPACITY * 6 + HD_TABLE_WIDTH - 1) / HD_TABLE_WIDTH,
    HD_GROUND_HEIGHT = (HD_FRAME_COUNT * HD_GROUND_MAP_COUNT * HD_GROUND_MAP_WORDS + HD_TABLE_WIDTH - 1) / HD_TABLE_WIDTH
};

static int tables_init(void)
{
    if (!hd.descriptors)
    {
        hd.descriptors = vid_tex_create_fmt(HD_TABLE_WIDTH, HD_DESCRIPTOR_HEIGHT * HD_DESCRIPTOR_BANK_COUNT, VID_RGBA16UI);
        if (!hd.descriptors)
            return 0;
        memset(vid_tex_data(hd.descriptors), 0, HD_TABLE_WIDTH * HD_DESCRIPTOR_HEIGHT * HD_DESCRIPTOR_BANK_COUNT * 8);
        vid_tex_commit(hd.descriptors);
    }
    if (!hd.poses)
    {
        hd.poses = vid_tex_create_fmt(HD_TABLE_WIDTH, HD_POSE_HEIGHT, VID_RGBA16UI);
        if (!hd.poses)
            return 0;
        memset(vid_tex_data(hd.poses), 0, HD_TABLE_WIDTH * HD_POSE_HEIGHT * 8);
        vid_tex_commit(hd.poses);
    }
    if (!hd.ground_colors)
    {
        hd.ground_colors = vid_tex_create_fmt(HD_TABLE_WIDTH, HD_GROUND_HEIGHT, VID_RGBA16UI);
        if (!hd.ground_colors)
            return 0;
        memset(vid_tex_data(hd.ground_colors), 0, HD_TABLE_WIDTH * HD_GROUND_HEIGHT * 8);
        vid_tex_commit(hd.ground_colors);
    }
    return 1;
}

void hd_matrix_rows(const POSE_MATRIX *matrix, float rows[12])
{
    int row, col;
    for (row = 0; row < 3; row++)
    {
        for (col = 0; col < 3; col++)
            rows[row * 4 + col] = matrix->m[row][col] / 4096.0f;
        rows[row * 4 + 3] = matrix->t[row] / 65536.0f;
    }
}

int hd_init(void)
{
    if (!hd.vram)
    {
        hd.vram = vid_tex_create_fmt(GPU_VRAM_WIDTH, GPU_VRAM_HEIGHT, VID_RGBA5551);
        hd.vram_valid = 0;
    }
    return hd.vram != NULL;
}

static void model_release(HD_MODEL *model)
{
    vid_mesh_free(model->mesh);
    vid_index_free(model->indices);
    free((void *)model->vertices);
    free(model->faces);
    free(model->bounds);
    *model = (HD_MODEL){0};
}

static void model_invalidate(uint32 type)
{
    HD_MODEL *model;
    uint32 frame, draw;
    XASSERT(type < MODEL_TYPE_COUNT);
    for (frame = 0; frame < HD_FRAME_COUNT; frame++)
        for (draw = 0; draw < hd.frame[frame].model_count; draw++)
            if (hd.frame[frame].models[draw].type == type)
            {
                hd.frame[frame].valid = 0;
                hd.frame[frame].broken = 1;
                break;
            }
    for (frame = 0; frame < HD_FRAME_COUNT; frame++)
        for (draw = 0; draw < hd.frame[frame].mutable_capacity; draw++)
            if (hd.frame[frame].mutable_models[draw].type == type)
                model_release(&hd.frame[frame].mutable_models[draw].model);
    model = hd.model + type;
    model_release(model);
    for (uint32 item = 0; item < hd.trail_buffer_count; item++)
        if (hd.trail_buffers[item]->type == type)
            model_release(&hd.trail_buffers[item]->model);
}

void hd_model_invalidate(uint32 type)
{
    XASSERT(type < MODEL_TYPE_COUNT);
    MODEL_REFERENCE source = model_refs[type];
    /* resource reloads also replace meshes borrowed through another model slot */
    for (uint32 alias = 0; alias < MODEL_TYPE_COUNT; alias++)
    {
        const MODEL_REFERENCE *cached = &hd.model[alias].source;
        if (alias == type || (source.header && cached->header == source.header && cached->vertices == source.vertices && cached->faces == source.faces))
            model_invalidate(alias);
    }
}

void hd_reset(void)
{
    uint32 type;
    if (hd.frame_open)
        gpu_frame_end();
    hd.frame_open = hd.frame_hd = 0;
    hd.wait_valid = hd.wait_presented = hd.wait_sealed = 0;
    hd.history_pending = 1;
    hd.frame_raster = hd.presenting = hd.vram_locked = 0;
    hd.page_frame[0] = hd.page_frame[1] = UINT32_MAX;
    memset(hd.page_ready, 0, sizeof(hd.page_ready));
    memset(hd.software_ready, 0, sizeof(hd.software_ready));
    gpu_set_raster(1);
    hd.collecting = 0;
    memset(hd.matrix_samples, 0, sizeof(hd.matrix_samples));
    hd.camera_blend.status = 0;
    hd.actor_owner = hd.object_owner = UINT32_MAX;
    for (uint32 map = 0; map < hd.terrain_mesh_count; map++)
    {
        for (uint32 layer = 0; layer < 4; layer++)
            vid_mat_free(hd.terrain_meshes[map].material[layer]);
        vid_tex_free(hd.terrain_meshes[map].image);
    }
    vid_mesh_free(hd.terrain_grid);
    hd.terrain_grid = NULL;
    hd.terrain_mesh_count = 0;
    for (type = 0; type < HD_FRAME_COUNT; type++)
    {
        hd.frame[type].model_count = hd.frame[type].part_count = 0;
        hd.frame[type].valid = 0;
        hd.frame[type].partial = 0;
        memset(hd.frame[type].objects, 0, sizeof(hd.frame[type].objects));
        memset(hd.frame[type].actor_model, 255, sizeof(hd.frame[type].actor_model));
        for (uint32 grid = 0; grid < hd.frame[type].terrain_capacity; grid++)
        {
            hd.frame[type].terrain[grid] = (HD_TERRAIN_DRAW){0};
        }
        for (uint32 item = 0; item < hd.frame[type].mutable_capacity; item++)
            model_release(&hd.frame[type].mutable_models[item].model);
        hd.frame[type].mutable_count = 0;
        hd.frame[type].terrain_count = 0;
        hd.frame[type].ui_count = 0;
        hd.frame[type].ground_count = 0;
    }
    memset(hd.transform, 0, sizeof(hd.transform));
    memset(hd.actor_pose, 0, sizeof(hd.actor_pose));
    for (type = 0; type < MODEL_TYPE_COUNT; type++)
        model_invalidate(type);
    hd.vram_valid = 0;
    hd.descriptors_valid = 0;
    hd.prepared_frame = hd.ground_frame = UINT32_MAX;
    hd.pose_count = hd.pose_span_count = 0;
    hd.poses_dirty = 0;
    hd.ordered_count = 0;
    hd.ui_buffer_cursor = 0;
    hd.ui_layout = HD_UI_CENTER;
}

void hd_free(void)
{
    int overlay, blend, stp;
    hd_reset();
    free(hd.terrain_meshes);
    hd.terrain_meshes = NULL;
    hd.terrain_mesh_capacity = 0;
    for (uint32 item = 0; item < hd.trail_buffer_count; item++)
        free(hd.trail_buffers[item]);
    free(hd.trail_buffers);
    hd.trail_buffers = NULL;
    hd.trail_buffer_count = hd.trail_buffer_capacity = hd.trail_buffer_cursor = 0;
    for (overlay = 0; overlay < HD_FRAME_COUNT; overlay++)
    {
        free(hd.frame[overlay].models);
        free(hd.frame[overlay].pass_faces);
        free(hd.frame[overlay].mutable_models);
        free(hd.frame[overlay].parts);
        free(hd.frame[overlay].banks);
        free(hd.frame[overlay].terrain);
        free(hd.frame[overlay].ui);
        free(hd.frame[overlay].ground_maps);
        memset(hd.frame + overlay, 0, sizeof(HD_FRAME));
    }
    for (overlay = 0; overlay < 4; overlay++)
    {
        for (blend = 0; blend < HD_BLEND_COUNT; blend++)
            for (stp = 0; stp < 3; stp++)
            {
                vid_mat_free(hd.material[overlay][blend][stp]);
                hd.material[overlay][blend][stp] = NULL;
            }
    }
    for (overlay = 0; overlay < 2; overlay++)
    {
        vid_shader_free(hd.shader[overlay]);
        hd.shader[overlay] = NULL;
    }
    free(hd.ordered);
    free(hd.traversal);
    free(hd.render_model);
    free(hd.blend_parts);
    free(hd.pose_spans);
    hd.pose_spans = NULL;
    hd.pose_span_count = hd.pose_span_capacity = 0;
    hd.blend_parts = NULL;
    hd.blend_capacity = 0;
    for (uint32 index = 0; index < hd.ui_buffer_count; index++)
        vid_mesh_free(hd.ui_buffers[index].mesh);
    vid_index_free(hd.quad_indices);
    hd.quad_indices = NULL;
    hd.quad_capacity = 0;
    free(hd.ui_buffers);
    free(hd.ui_order);
    hd.ui_buffers = NULL;
    hd.ui_order = NULL;
    hd.ui_buffer_count = hd.ui_buffer_capacity = hd.ui_order_capacity = 0;
    hd.ordered = NULL;
    hd.traversal = NULL;
    hd.render_model = NULL;
    hd.ordered_capacity = hd.traversal_capacity = hd.render_capacity = 0;
    vid_tex_free(hd.vram);
    vid_tex_free(hd.descriptors);
    vid_tex_free(hd.poses);
    vid_tex_free(hd.ground_colors);
    hd.descriptors = hd.poses = hd.ground_colors = NULL;
    hd.vram = NULL;
    hd.vram_valid = 0;
    hd.vram_version = 0;
}

VID_TEXTURE *hd_vram_sync(void)
{
    if (hd.vram_locked)
        return hd.vram_valid ? hd.vram : NULL;
    uint32 version = gpu_vram_version();
    uint32 i;
    uint16 *pixels;
    if (!hd_init())
        return NULL;
    if (hd.vram_valid && hd.vram_version == version)
        return hd.vram;
    pixels = (uint16 *)vid_tex_data(hd.vram);
    if (!pixels)
        return NULL;
    for (i = 0; i < GPU_VRAM_WIDTH * GPU_VRAM_HEIGHT; i++)
    {
        uint32 word = VRAM[i];
        /* GL RGB5_A1 stores R high and the original STP bit low */
        pixels[i] = (uint16)(((word & 31u) << 11) | ((word & 992u) << 1) | ((word >> 9) & 62u) | (word >> 15));
    }
    vid_tex_commit(hd.vram);
    hd.vram_version = version;
    hd.vram_valid = 1;
    return hd.vram;
}

static VID_MATERIAL *create_material(HD_BLEND blend, int overlay, uint32 stp_class, uint32 shader_index, VID_TEXTURE *auxiliary)
{
    VID_MATERIAL *material;
    VID_MATERIAL_DATA *data;
    if (!hd_init() || !tables_init())
        return NULL;
    if (!hd.shader[shader_index])
    {
        hd.shader[shader_index] = vid_shader_create(shader_index == 1 ? ui_vertex : model_vertex, fragment);
        if (!hd.shader[shader_index])
            return NULL;
    }
    material = vid_mat_create();
    if (!material)
        return NULL;
    data = vid_mat_data(material);
    data->shader = hd.shader[shader_index];
    data->texture = hd.vram;
    data->aux[0] = hd.descriptors;
    data->aux[1] = auxiliary ? auxiliary : shader_index == 1 ? hd.ground_colors : hd.poses;
    data->blend = blend == HD_BLEND_HALF ? VID_BLEND_ALPHA : blend == HD_BLEND_SUBTRACT ? VID_BLEND_SUBTRACT : blend == HD_BLEND_OPAQUE ? VID_BLEND_NONE : VID_BLEND_ADD;
    data->depth_test = overlay != 1;
    data->depth_write = overlay == 0 && blend == HD_BLEND_OPAQUE;
    data->params[0] = (float)stp_class;
    data->params[1] = blend == HD_BLEND_QUARTER ? 0.25f : 1.0f;
    data->params[2] = blend == HD_BLEND_HALF ? 0.5f : 1.0f;
    vid_mat_commit(material);
    return material;
}

static VID_MATERIAL *material_for(HD_BLEND blend, int overlay, uint32 stp_class)
{
    XASSERT(blend >= 0 && blend < HD_BLEND_COUNT && overlay >= 0 && overlay < 4 && stp_class < 3);
    VID_MATERIAL **slot = &hd.material[overlay][blend][stp_class];
    if (!*slot)
        *slot = create_material(blend, overlay, stp_class, overlay == 1 || overlay == 3, NULL);
    return *slot;
}

static void rig_rotate(MATRIXF *matrix, uint32 axis, sint32 angle)
{
    MATRIXF rotation;
    float radians = (angle & 4095) * RAD12;
    IdentityMatrixf(&rotation);
    if (axis == 0)
        RotMatrixXf(-radians, &rotation);
    else if (axis == 1)
        RotMatrixYf(-radians, &rotation);
    else
        RotMatrixZf(radians, &rotation);
    MulMatrixf(matrix, &rotation);
}

static void rig_pose_angles(MATRIXF *matrix, sint32 roll, sint32 yaw, sint32 pitch)
{
    rig_rotate(matrix, 2, roll);
    rig_rotate(matrix, 1, (sint32)(0u - (uint32)yaw));
    rig_rotate(matrix, 0, (sint32)(0u - (uint32)pitch));
}

uint32 hd_pose_parts(MODEL_REFERENCE model, const ACTOR_POSE *pose, const MATRIXF *base, HD_RIG_PART *parts, uint32 capacity, HD_POSE_SAMPLE *sample)
{
    MATRIXF stack[32];
    const MODEL_RIG_HEADER *rig = (const MODEL_RIG_HEADER *)model.header;
    const uint32 *stream, *end;
    uint32 depth = 0, part = 0, first = 0, command, i, row, col;
    XASSERT(rig && model.byte_len >= sizeof(*rig) && pose && base && parts && sample);
    XASSERT(!(rig->commands_offset & 3) && rig->commands_offset <= model.byte_len - sizeof(uint32));
    XASSERT(rig->part_count <= capacity && rig->part_count <= (model.byte_len - sizeof(*rig)) / sizeof(uint32));
    end = (const uint32 *)((const uint8 *)rig + (model.byte_len & ~3u));
    stream = (const uint32 *)((const uint8 *)rig + rig->commands_offset);
    for (i = 0; i < ACTOR_POSE_MATRIX_COUNT; i++)
    {
        float rows[12];
        hd_matrix_rows(pose->matrix + i, rows);
        for (row = 0; row < 3; row++)
        {
            for (col = 0; col < 3; col++)
                sample->attachment[i].m[row][col] = rows[row * 4 + col];
            sample->attachment[i].t[row] = rows[row * 4 + 3];
        }
    }
    for (i = 0; i < ACTOR_POSE_POINT_COUNT; i++)
        sample->point[i] = (VECTORF){pose->point[i].x / 65536.0f, pose->point[i].y / 65536.0f, pose->point[i].z / 65536.0f};
    sample->attachment_mask = sample->point_mask = 0;
    memset(sample->attachment_part, 255, sizeof(sample->attachment_part));
    stack[0] = *base;
    rig_rotate(stack, 0, pose->angles[0].pitch);
    rig_rotate(stack, 1, pose->angles[0].yaw);
    rig_rotate(stack, 2, pose->angles[0].roll);
    XASSERT(stream < end);
    command = *stream++;
    for (;;)
    {
        MATRIXF *matrix = stack + depth;
        uint32 flag, bone, count;
        VECTORF translation;
        if (command == 1)
        {
            XASSERT(depth < 31);
            stack[depth + 1] = *matrix;
            depth++;
        }
        else if (command == 2)
        {
            XASSERT(depth);
            depth--;
        }
        else
        {
            XASSERT(end - stream >= 11 && part < rig->part_count);
            flag = stream[0];
            translation = (VECTORF){(sint32)stream[1] / 65536.0f, (sint32)stream[2] / 65536.0f, (sint32)stream[3] / 65536.0f};
            RotTransf(matrix, &translation, &translation);
            TransMatrixf(matrix, &translation);
            rig_pose_angles(matrix, (sint32)stream[4], (sint32)stream[5], (sint32)stream[6]);
            bone = stream[7];
            XASSERT(bone < ACTOR_POSE_BONE_COUNT);
            rig_pose_angles(matrix, pose->angles[bone].roll, pose->angles[bone].yaw, pose->angles[bone].pitch);
            if (flag == 1)
            {
                rig_rotate(matrix, 1, (sint32)pose->sway_yaw);
                sample->point[6] = translation;
                sample->point_mask |= 1u << 6;
            }
            else if (flag == 2 || flag == 3 || flag == 10 || flag == 11)
            {
                uint32 attachment = flag == 2 ? 0 : flag == 3 ? 1 : flag == 10 ? 2 : 3;
                sample->attachment[attachment] = *matrix;
                sample->attachment_part[attachment] = part;
                sample->attachment_mask |= 1u << attachment;
            }
            else if (flag >= 4 && flag <= 9)
            {
                sample->point[flag - 4] = translation;
                sample->point_mask |= 1u << (flag - 4);
            }
            count = rig->part_vertex_count[part];
            XASSERT(model.header->vertex_count >= 0 && first <= (uint32)model.header->vertex_count && count <= (uint32)model.header->vertex_count - first);
            parts[part++] = (HD_RIG_PART){first, count, *matrix};
            first += count;
            rig_pose_angles(matrix, (sint32)stream[8], (sint32)stream[9], (sint32)stream[10]);
            stream += 11;
        }
        XASSERT(stream < end);
        command = *stream++;
        if (!command)
            break;
    }
    /* ZENG resources retain an untransformed vertex tail */
    XASSERT(part == rig->part_count);
    return part;
}

static int descriptors_sync_bank(const TEXTURE_DESCRIPTORS *descriptors, const TEXTURE_IMAGE *images, uint32 bank)
{
    uint32 i;
    int changed = !hd.descriptors_valid;
    uint16 *pixels;
    if (!tables_init())
        return 0;
    XASSERT(bank < HD_DESCRIPTOR_BANK_COUNT);
    pixels = (uint16 *)vid_tex_data(hd.descriptors) + bank * TEXTURE_DESCRIPTOR_CAPACITY * 12;
    for (i = 0; i < TEXTURE_DESCRIPTOR_CAPACITY; i++)
    {
        const TEXTURE_DESCRIPTOR *source = descriptors->item + i;
        const TEXTURE_IMAGE *image = images + i;
        uint16 encoded[12] = {source->uv0.uv, source->clut, source->uv1.uv, source->tpage, source->uv2.uv, source->uv3.uv, 0, 0, 65535, 0, 0, 0};
        if (image->uploaded && image->width && image->height && image->x + image->width <= 256 && image->y + image->height <= 256 && image->mode == ((source->tpage >> 7) & 3))
        {
            uint32 corner;
            uint16 uv[4] = {source->uv0.uv & 65534u, source->uv1.uv, source->uv2.uv, source->uv3.uv};
            encoded[6] = image->x;
            encoded[7] = image->y;
            encoded[8] = (uint16)((image->width - 1) | ((image->height - 1) << 8));
            for (corner = 0; corner < 4; corner++)
            {
                uint32 u = uv[corner] & 255u, v = uv[corner] >> 8;
                if (u < image->x || u >= image->x + (uint32)image->width || v < image->y || v >= image->y + (uint32)image->height)
                    break;
            }
            encoded[9] = corner == 4;
        }
        if (memcmp(pixels + i * 12, encoded, sizeof(encoded)))
        {
            memcpy(pixels + i * 12, encoded, sizeof(encoded));
            changed = 1;
        }
    }
    if (changed)
        vid_tex_commit(hd.descriptors);
    hd.descriptors_valid = 1;
    return 1;
}

static int prepare_descriptors(const HD_FRAME *frame)
{
    if (hd.descriptors_valid && hd.prepared_frame == hd.last_frame && hd.prepared_sequence == frame->sequence)
        return 1;
    for (uint32 bank = 0; bank < frame->bank_count; bank++)
        if (!descriptors_sync_bank(&frame->banks[bank].descriptors, frame->banks[bank].images, bank))
            return 0;
    hd.prepared_frame = hd.last_frame;
    hd.prepared_sequence = frame->sequence;
    return 1;
}

int hd_descriptors_sync(const TEXTURE_DESCRIPTORS *descriptors, const TEXTURE_IMAGE *images)
{
    hd.prepared_frame = UINT32_MAX;
    return descriptors_sync_bank(descriptors, images, 0);
}

void hd_pose_begin(void)
{
    hd.pose_count = hd.pose_span_count = 0;
    hd.poses_dirty = 0;
}

static uint32 pose_hash(const HD_RIG_PART *parts, uint32 count)
{
    uint32 hash = 2166136261u;
    for (uint32 part = 0; part < count; part++)
        for (uint32 row = 0; row < 3; row++)
            for (uint32 col = 0; col < 4; col++)
            {
                float value = col < 3 ? parts[part].matrix.m[row][col] : parts[part].matrix.t[row];
                uint32 bits;
                memcpy(&bits, &value, sizeof(bits));
                hash = (hash ^ bits) * 16777619u;
            }
    return hash;
}

static int pose_matches(const HD_RIG_PART *parts, uint32 count, const uint16 *pixels)
{
    for (uint32 part = 0; part < count; part++)
        for (uint32 row = 0; row < 3; row++)
            for (uint32 col = 0; col < 4; col++)
            {
                float value = col < 3 ? parts[part].matrix.m[row][col] : parts[part].matrix.t[row];
                uint32 bits;
                memcpy(&bits, &value, sizeof(bits));
                if (bits != ((uint32)pixels[0] | ((uint32)pixels[1] << 16)))
                    return 0;
                pixels += 2;
            }
    return 1;
}

uint32 hd_pose_upload(const HD_RIG_PART *parts, uint32 count)
{
    uint32 first = hd.pose_count * 6, index, row, col;
    uint16 *pixels;
    _Static_assert(sizeof(float) == sizeof(uint32), "GPU float word size");
    XASSERT(count <= HD_POSE_CAPACITY);
    if (!tables_init())
        return UINT32_MAX;
    uint16 *table = (uint16 *)vid_tex_data(hd.poses);
    uint32 hash = pose_hash(parts, count);
    for (uint32 item = 0; item < hd.pose_span_count; item++)
    {
        const HD_POSE_SPAN *span = hd.pose_spans + item;
        if (span->count == count && span->hash == hash && pose_matches(parts, count, table + span->first * 4))
            return span->first;
    }
    XASSERT(hd.pose_count <= HD_POSE_CAPACITY && count <= HD_POSE_CAPACITY - hd.pose_count);
    pixels = table + first * 4;
    for (index = 0; index < count; index++)
        for (row = 0; row < 3; row++)
            for (col = 0; col < 4; col++)
            {
                float value = col < 3 ? parts[index].matrix.m[row][col] : parts[index].matrix.t[row];
                uint32 bits;
                memcpy(&bits, &value, sizeof(bits));
                uint16 low = (uint16)bits, high = (uint16)(bits >> 16);
                if (pixels[0] != low || pixels[1] != high)
                {
                    uint32 word = (uint32)(pixels - table) / 4;
                    if (!hd.poses_dirty)
                        hd.pose_dirty_first = word;
                    hd.pose_dirty_last = word;
                    hd.poses_dirty = 1;
                    pixels[0] = low;
                    pixels[1] = high;
                }
                pixels += 2;
            }
    hd.pose_count += count;
    if (count && reserve_frame((void **)&hd.pose_spans, &hd.pose_span_capacity, hd.pose_span_count + 1, sizeof(*hd.pose_spans)))
        hd.pose_spans[hd.pose_span_count++] = (HD_POSE_SPAN){first, count, hash};
    return first;
}

void hd_pose_commit(void)
{
    if (hd.poses_dirty)
    {
        uint32 first = hd.pose_dirty_first / HD_TABLE_WIDTH;
        vid_tex_commit_rows(hd.poses, first, hd.pose_dirty_last / HD_TABLE_WIDTH - first + 1);
    }
    hd.poses_dirty = 0;
}

static int model_bounds(HD_MODEL *cache, MODEL_REFERENCE model)
{
    const MODEL_RIG_HEADER *rig = cache->rig ? (const MODEL_RIG_HEADER *)model.header : NULL;
    uint32 first = 0, parts = rig ? rig->part_count : 1;
    XASSERT(parts && model.header->vertex_count >= 0 && (uint32)model.header->vertex_count <= model.vertex_byte_len / sizeof(SVECTOR));
    if (!cache->bounds)
        cache->bounds = calloc(parts, sizeof(*cache->bounds));
    if (!cache->bounds)
        return 0;
    cache->bounds_count = parts;
    for (uint32 part = 0; part < parts; part++)
    {
        uint32 count = rig ? rig->part_vertex_count[part] : (uint32)model.header->vertex_count;
        XASSERT(first <= (uint32)model.header->vertex_count && count <= (uint32)model.header->vertex_count - first);
        HD_BOUNDS *bounds = cache->bounds + part;
        *bounds = (HD_BOUNDS){0};
        for (uint32 vertex = 0; vertex < count; vertex++)
        {
            const SVECTOR *source = model.vertices + first + vertex;
            const float point[3] = {source->vx, source->vy, source->vz};
            for (uint32 axis = 0; axis < 3; axis++)
            {
                if (!vertex || point[axis] < bounds->min[axis])
                    bounds->min[axis] = point[axis];
                if (!vertex || point[axis] > bounds->max[axis])
                    bounds->max[axis] = point[axis];
            }
        }
        first += count;
    }
    return 1;
}

static void model_gpu_point(const VID_VERTEX16 *source, int rig, float value[4])
{
    for (uint32 axis = 0; axis < 3; axis++)
        value[axis] = source->pos[axis] + source->extra[axis] * source->extra[3];
    value[3] = rig ? (float)((uint16)source->n[3] + 1u) : 0;
}

static int model_gpu_update(HD_MODEL *model)
{
    VID_VERTEX3D *vertices = vid_mesh_data_3d(model->mesh);
    if (!vertices)
        return 0;
    for (uint32 face = 0; face < model->face_count; face++)
    {
        const HD_FACE_RANGE *range = model->faces + face;
        const VID_VERTEX16 *source = model->vertices + face * 4;
        int line = range->descriptor == UINT16_MAX;
        uint32 a = range->flags & 2 ? 1 : 0, b = range->flags & 2 ? 0 : 1, c = range->flags & 2 ? 2 : 3;
        for (uint32 corner = 0; corner < 4; corner++)
        {
            VID_VERTEX3D *vertex = vertices + face * 4 + corner;
            model_gpu_point(source + corner, model->rig, vertex->pos);
            uint32 flags = (uint32)vertex->pos[3] | (corner << HD_VERTEX_CORNER_SHIFT) | ((range->flags & 1) ? HD_VERTEX_TWO_SIDED : 0) | ((range->flags & 2) ? HD_VERTEX_FLIPPED : 0) | (line ? HD_VERTEX_LINE : 0);
            vertex->pos[3] = (float)flags;
            vertex->uv[0] = range->descriptor;
            vertex->uv[1] = (float)face;
            if (line)
            {
                for (uint32 axis = 0; axis < 3; axis++)
                {
                    vertex->color[axis] = source[corner].n[axis];
                    vertex->n[axis] = source[corner].color[axis];
                }
                vertex->color[3] = model->rig ? source[corner].extra[0] + 1 : 0;
                vertex->n[3] = source[corner].extra[1];
                vertex->extra[0] = source[corner].extra[2];
                vertex->extra[1] = vertex->extra[2] = vertex->extra[3] = 0;
            }
            else
            {
                model_gpu_point(source + a, model->rig, vertex->color);
                model_gpu_point(source + b, model->rig, vertex->n);
                model_gpu_point(source + c, model->rig, vertex->extra);
            }
        }
    }
    vid_mesh_commit(model->mesh);
    return 1;
}

static HD_MODEL *model_build(HD_MODEL *cache, MODEL_REFERENCE model, int rigged)
{
    static const uint32 normal_order[4] = {0, 1, 3, 2}, flipped_order[4] = {1, 0, 2, 3};
    static const uint32 triangles[6] = {0, 1, 2, 1, 3, 2};
    VID_VERTEX16 *vertices;
    uint32 *indices;
    uint16 *vertex_parts = NULL;
    uint32 face_count, face_index;
    XASSERT(model.header && model.header->vertex_count >= 0 && model.header->face_count > 0);
    face_count = (uint32)model.header->face_count;
    XASSERT(model.faces && face_count <= model.face_byte_len / sizeof(*model.faces) && face_count <= UINT32_MAX / 6);
    model_release(cache);
    cache->source = model;
    cache->rig = rigged;
    cache->face_count = face_count;
    cache->mesh = vid_mesh_create_3d(face_count * 4);
    cache->indices = vid_index_create(face_count * 6);
    cache->faces = calloc(face_count, sizeof(*cache->faces));
    if (!cache->mesh || !cache->indices || !cache->faces)
        goto failed;
    if (rigged)
    {
        const MODEL_RIG_HEADER *rig = (const MODEL_RIG_HEADER *)model.header;
        uint32 part, first = 0;
        XASSERT(model.byte_len >= sizeof(*rig) && rig->part_count <= (model.byte_len - sizeof(*rig)) / sizeof(uint32));
        vertex_parts = malloc((size_t)model.header->vertex_count * sizeof(*vertex_parts));
        if (!vertex_parts)
            goto failed;
        memset(vertex_parts, 255, (size_t)model.header->vertex_count * sizeof(*vertex_parts));
        for (part = 0; part < rig->part_count; part++)
        {
            uint32 count = rig->part_vertex_count[part], vertex;
            XASSERT(part < 32768 && first <= (uint32)model.header->vertex_count && count <= (uint32)model.header->vertex_count - first);
            for (vertex = 0; vertex < count; vertex++)
                vertex_parts[first + vertex] = (uint16)part;
            first += count;
        }
    }
    if (!model_bounds(cache, model))
        goto failed;
    vertices = calloc((size_t)face_count * 4, sizeof(*vertices));
    if (!vertices)
        goto failed;
    cache->vertices = vertices;
    indices = vid_index_data(cache->indices);
    memset(vertices, 0, (size_t)face_count * 4 * sizeof(*vertices));
    for (face_index = 0; face_index < face_count; face_index++)
    {
        const MODEL_FACE_RECORD *face = model.faces + face_index;
        const uint32 *order = face->flags & 2 ? flipped_order : normal_order;
        uint32 corner;
        cache->faces[face_index].first = cache->index_count;
        cache->faces[face_index].flags = face->flags;
        cache->faces[face_index].descriptor = (uint16)(face->tex_offset / sizeof(TEXTURE_DESCRIPTOR));
        if ((sint32)face->words[1] < 0)
        {
            XASSERT(model.vertices);
            cache->line_count++;
            cache->unsupported_lines += (face->words[2] >> 24) != 0x40;
            cache->faces[face_index].descriptor = UINT16_MAX;
            for (corner = 0; corner < 4; corner++)
            {
                uint32 endpoint = corner & 1;
                uint32 current = face->vertex_offset[endpoint] / 4, other = face->vertex_offset[1 - endpoint] / 4;
                VID_VERTEX16 *vertex = vertices + face_index * 4 + corner;
                XASSERT(!(face->vertex_offset[endpoint] & 3) && current < (uint32)model.header->vertex_count && current < model.vertex_byte_len / sizeof(SVECTOR));
                XASSERT(!(face->vertex_offset[1 - endpoint] & 3) && other < (uint32)model.header->vertex_count && other < model.vertex_byte_len / sizeof(SVECTOR));
                const SVECTOR *a = model.vertices + current, *b = model.vertices + other;
                vertex->pos[0] = a->vx;
                vertex->pos[1] = a->vy;
                vertex->pos[2] = a->vz;
                vertex->n[0] = b->vx;
                vertex->n[1] = b->vy;
                vertex->n[2] = b->vz;
                vertex->color[0] = (uint8)face->words[2];
                vertex->color[1] = (uint8)(face->words[2] >> 8);
                vertex->color[2] = (uint8)(face->words[2] >> 16);
                vertex->color[3] = 255;
                vertex->tex[2] = HD_TEX_LINE;
                vertex->extra[1] = ((corner & 2) ? 1.0f : -1.0f) * (endpoint ? -1.0f : 1.0f);
                vertex->extra[2] = endpoint ? -1.0f : 1.0f;
                if (rigged)
                {
                    XASSERT(vertex_parts[current] != UINT16_MAX && vertex_parts[other] != UINT16_MAX);
                    vertex->n[3] = (sint16)vertex_parts[current];
                    vertex->extra[0] = (float)vertex_parts[other];
                    vertex->tex[2] |= HD_TEX_RIG;
                }
            }
            for (corner = 0; corner < 6; corner++)
                indices[cache->index_count++] = face_index * 4 + triangles[corner];
            cache->faces[face_index].count = 6;
            continue;
        }
        XASSERT(face->tex_offset % sizeof(TEXTURE_DESCRIPTOR) == 0 && face->tex_offset / sizeof(TEXTURE_DESCRIPTOR) < TEXTURE_DESCRIPTOR_CAPACITY);
        for (corner = 0; corner < 4; corner++)
        {
            uint32 offset = face->vertex_offset[order[corner]], vertex_index = offset / 4;
            const SVECTOR *source;
            VID_VERTEX16 *vertex = vertices + face_index * 4 + corner;
            XASSERT(model.vertices && !(offset & 3) && vertex_index < (uint32)model.header->vertex_count && vertex_index < model.vertex_byte_len / sizeof(*source));
            source = model.vertices + vertex_index;
            vertex->pos[0] = source->vx;
            vertex->pos[1] = source->vy;
            vertex->pos[2] = source->vz;
            memset(vertex->color, 255, sizeof(vertex->color));
            vertex->tex[0] = face->tex_offset / sizeof(TEXTURE_DESCRIPTOR);
            vertex->tex[1] = (uint16)corner;
            vertex->tex[2] = HD_TEX_TEXTURED | HD_TEX_DESCRIPTOR | HD_TEX_FILTERED | HD_TEX_CLASSIFIED;
            vertex->rect[0] = (uint16)face_index;
            vertex->rect[1] = (uint16)(face_index >> 16);
            if (rigged)
            {
                XASSERT(vertex_parts[vertex_index] != 65535);
                vertex->n[3] = (sint16)vertex_parts[vertex_index];
                vertex->tex[2] |= HD_TEX_RIG;
            }
        }
        for (corner = 0; corner < 6; corner++)
            indices[cache->index_count++] = face_index * 4 + triangles[corner];
        cache->faces[face_index].count = 6;
    }
    free(vertex_parts);
    if (!model_gpu_update(cache))
        goto failed;
    for (uint32 face = 0; face < cache->face_count / 2; face++)
        for (uint32 corner = 0; corner < 6; corner++)
        {
            uint32 left = face * 6 + corner, right = cache->index_count - (face + 1) * 6 + corner;
            uint32 saved = indices[left];
            indices[left] = indices[right];
            indices[right] = saved;
        }
    for (uint32 face = 0; face < cache->face_count; face++)
        cache->faces[face].first = cache->index_count - cache->faces[face].first - cache->faces[face].count;
    vid_index_commit(cache->indices, cache->index_count);
    vid_index_static(cache->indices);
    return cache;
failed:
    free(vertex_parts);
    model_release(cache);
    return NULL;
}

const HD_MODEL *hd_model_mesh(uint32 type, MODEL_REFERENCE model, int rigged)
{
    XASSERT(type < MODEL_TYPE_COUNT && type != 867 && type != 868);
    XASSERT(model.header);
    HD_MODEL *cache = hd.model + type;
    if (cache->mesh && cache->source.header == model.header && cache->source.vertices == model.vertices && cache->source.faces == model.faces && cache->source.byte_len == model.byte_len && cache->face_count == (uint32)model.header->face_count && cache->rig == rigged)
        return cache;
    model_invalidate(type);
    return model_build(cache, model, rigged);
}

static void matrix_from_pose(const POSE_MATRIX *source, MATRIXF *destination)
{
    uint32 row, col;
    for (row = 0; row < 3; row++)
    {
        for (col = 0; col < 3; col++)
            destination->m[row][col] = source->m[row][col] / 4096.0f;
        destination->t[row] = source->t[row] / 65536.0f;
    }
}

static HD_TRANSFORM *transform_slot(const POSE_MATRIX *slot)
{
    uintptr_t address = (uintptr_t)slot, scratch = (uintptr_t)&geometry_workspace.scratch, model = (uintptr_t)&geometry_workspace.model;
    uint32 index;
    if (address >= scratch && address - scratch < sizeof(geometry_workspace.scratch.matrix))
    {
        XASSERT((address - scratch) % sizeof(POSE_MATRIX) == 0);
        index = (uint32)((address - scratch) / sizeof(POSE_MATRIX));
    }
    else
    {
        XASSERT(address >= model && address - model < sizeof(geometry_workspace.model.matrix) && (address - model) % sizeof(POSE_MATRIX) == 0);
        index = GEOMETRY_MATRIX_COUNT + (uint32)((address - model) / sizeof(POSE_MATRIX));
    }
    if (!hd.transform[index].valid)
    {
        matrix_from_pose(slot, &hd.transform[index].matrix);
        hd.transform[index].valid = 1;
    }
    return hd.transform + index;
}

const MATRIXF *hd_transform_current(const POSE_MATRIX *slot)
{
    return hd.collecting ? &transform_slot(slot)->matrix : NULL;
}

void hd_transform_identity(const POSE_MATRIX *slot)
{
    if (hd.collecting)
        IdentityMatrixf(&transform_slot(slot)->matrix);
}

void hd_transform_copy(const POSE_MATRIX *destination, const POSE_MATRIX *source)
{
    if (hd.collecting)
    {
        MATRIXF matrix = transform_slot(source)->matrix;
        transform_slot(destination)->matrix = matrix;
    }
}

void hd_transform_load(const POSE_MATRIX *destination, const POSE_MATRIX *source)
{
    MATRIXF matrix;
    uint32 actor, attachment;
    if (!hd.collecting)
        return;
    for (actor = 0; actor < ACTOR_POSE_COUNT; actor++)
        for (attachment = 0; attachment < ACTOR_POSE_MATRIX_COUNT; attachment++)
            if (source == &actor_poses[actor].matrix[attachment] && (hd.actor_pose[actor].attachment_mask & (1u << attachment)))
            {
                transform_slot(destination)->matrix = hd.actor_pose[actor].attachment[attachment];
                return;
            }
    uintptr_t address = (uintptr_t)source, scratch = (uintptr_t)&geometry_workspace.scratch, model = (uintptr_t)&geometry_workspace.model;
    if ((address >= scratch && address - scratch < sizeof(geometry_workspace.scratch.matrix)) || (address >= model && address - model < sizeof(geometry_workspace.model.matrix)))
        matrix = transform_slot(source)->matrix;
    else
        matrix_from_pose(source, &matrix);
    transform_slot(destination)->matrix = matrix;
}

void hd_transform_translate(const POSE_MATRIX *slot, sint32 x, sint32 y, sint32 z)
{
    MATRIXF *matrix;
    VECTORF point = {x / 65536.0f, y / 65536.0f, z / 65536.0f};
    if (!hd.collecting)
        return;
    matrix = &transform_slot(slot)->matrix;
    RotTransf(matrix, &point, &point);
    TransMatrixf(matrix, &point);
}

void hd_transform_rotate(const POSE_MATRIX *slot, uint32 axis, uint32 angle)
{
    if (hd.collecting)
        rig_rotate(&transform_slot(slot)->matrix, axis, (sint32)angle);
}

void hd_transform_scale(const POSE_MATRIX *slot, sint32 x, sint32 y, sint32 z)
{
    VECTORF scale = {x / 16384.0f, y / 16384.0f, z / 16384.0f};
    if (hd.collecting)
        ScaleMatrixf(&transform_slot(slot)->matrix, &scale);
}

static int reserve_frame(void **storage, uint32 *capacity, uint32 needed, size_t stride)
{
    uint32 size = *capacity ? *capacity : 32;
    void *grown;
    if (needed <= *capacity)
        return 1;
    while (size < needed)
    {
        XASSERT(size <= UINT32_MAX / 2);
        size *= 2;
    }
    XASSERT(size <= SIZE_MAX / stride);
    grown = realloc(*storage, size * stride);
    if (!grown)
        return 0;
    *storage = grown;
    *capacity = size;
    return 1;
}

static void camera_world(MATRIXF *world)
{
    MATRIXF view;
    IdentityMatrixf(&view);
    rig_rotate(&view, 0, ((0u - (uint32)(sint32)camera.angles.pitch) >> 4) & 4095u);
    rig_rotate(&view, 1, ((0u - (uint32)(sint32)camera.angles.yaw) >> 4) & 4095u);
    TransposeMatrixf(&view, world);
    world->t[0] = (sint32)camera.pos.x.bits / 65536.0f;
    world->t[1] = (sint32)camera.pos.y.bits / 65536.0f;
    world->t[2] = (sint32)camera.pos.z.bits / 65536.0f;
}

void hd_collect_begin(float focal, float center_x, float center_y, uint32 width, uint32 height)
{
    HD_FRAME *frame;
    hd.current_frame = (hd.last_frame + 1) % HD_FRAME_COUNT;
    while (hd.current_frame == hd.page_frame[0] || hd.current_frame == hd.page_frame[1])
        hd.current_frame = (hd.current_frame + 1) % HD_FRAME_COUNT;
    frame = hd.frame + hd.current_frame;
    frame->model_count = frame->part_count = frame->missing = frame->pass_count = 0;
    memset(frame->actor_model, 255, sizeof(frame->actor_model));
    memset(frame->actor_attachment, 255, sizeof(frame->actor_attachment));
    memset(frame->actor_hand, 0, sizeof(frame->actor_hand));
    memset(frame->objects, 0, sizeof(frame->objects));
    memset(frame->trail_model, 255, sizeof(frame->trail_model));
    hd.actor_owner = hd.object_owner = UINT32_MAX;
    frame->mutable_count = 0;
    frame->ordinal = 0;
    frame->valid = frame->broken = frame->partial = 0;
    frame->stage = game_session.stage;
    frame->tick = game_session.tick;
    frame->paused = game_pause_flags != 0;
    frame->bank_count = 0;
    frame->terrain_count = 0;
    frame->ui_count = 0;
    frame->ground_count = 0;
    memset(frame->ui_head, 255, sizeof(frame->ui_head));
    hd.ui_layout = HD_UI_CENTER;
    frame->focal = focal;
    frame->center_x = center_x;
    frame->center_y = center_y;
    frame->horizon = center_y;
    camera_world(&frame->camera);
    frame->horizon_valid = 0;
    XASSERT(width && height);
    frame->width = width;
    frame->height = height;
    memset(hd.transform, 0, sizeof(hd.transform));
    memset(hd.actor_pose, 0, sizeof(hd.actor_pose));
    hd.collecting = 1;
}

static void match_previous_records(HD_FRAME *frame);
static void match_background_records(HD_FRAME *frame);
static void prepare_model_passes(HD_FRAME *frame);

void hd_collect_seal(sint32 offset_x, sint32 offset_y)
{
    if (hd.collecting)
    {
        hd.frame[hd.current_frame].offset_x = offset_x;
        hd.frame[hd.current_frame].offset_y = offset_y;
        hd.frame[hd.current_frame].valid = !hd.frame[hd.current_frame].broken;
        const HD_FRAME *previous = hd.frame + hd.last_frame;
        HD_FRAME *current = hd.frame + hd.current_frame;
        int cut = hd.history_pending || current->stage != previous->stage || current->tick < previous->tick || current->paused != previous->paused || current->width != previous->width || current->height != previous->height || current->focal != previous->focal;
        current->previous = cut ? UINT32_MAX : hd.last_frame;
        hd.history_pending = 0;
        hd.frame[hd.current_frame].sequence = ++hd.sequence;
        match_previous_records(current);
        match_background_records(current);
        prepare_model_passes(current);
        hd.last_frame = hd.current_frame;
        hd.collecting = 0;
    }
    else if (hd.wait_sealed)
    {
        hd.frame[hd.last_frame].offset_x = offset_x;
        hd.frame[hd.last_frame].offset_y = offset_y;
        hd.wait_sealed = 0;
    }
}

void hd_history_reset(void)
{
    hd.history_pending = 1;
    hd.wait_valid = 0;
}

/* allocation cuts history without changing any retained source pose */
void hd_actor_reset(uint32 actor)
{
    XASSERT(actor < ACTOR_COUNT);
    for (uint32 frame = 0; frame < HD_FRAME_COUNT; frame++)
        memset(hd.frame[frame].actor_model[actor], 255, sizeof(hd.frame[frame].actor_model[actor]));
}

static uint32 object_slot(HD_OBJECT_POOL pool, uint32 slot)
{
    XASSERT(pool >= HD_OBJECT_WORLD && pool <= HD_OBJECT_EFFECT && slot < (uint32)(pool == HD_OBJECT_WORLD ? WORLD_OBJECT_COUNT : pool == HD_OBJECT_DYNAMIC ? DYNAMIC_OBJECT_COUNT : EFFECT_COUNT));
    return slot + (pool == HD_OBJECT_WORLD ? 0 : pool == HD_OBJECT_DYNAMIC ? WORLD_OBJECT_COUNT : WORLD_OBJECT_COUNT + DYNAMIC_OBJECT_COUNT);
}

void hd_object_begin(HD_OBJECT_POOL pool, uint32 slot, uint32 kind)
{
    hd.object_owner = object_slot(pool, slot);
    hd.object_part = HD_OBJECT_PRIMARY;
    if (hd.collecting)
        hd.frame[hd.current_frame].objects[hd.object_owner].kind = kind;
}

void hd_object_part(HD_OBJECT_PART part)
{
    hd.object_part = part;
}

void hd_object_end(void)
{
    hd.object_owner = UINT32_MAX;
    hd.object_part = HD_OBJECT_PRIMARY;
}

void hd_object_reset(HD_OBJECT_POOL pool, uint32 slot)
{
    uint32 owner = object_slot(pool, slot);
    for (uint32 frame = 0; frame < HD_FRAME_COUNT; frame++)
    {
        hd.frame[frame].objects[owner] = (HD_OBJECT_SAMPLE){0};
        for (uint32 item = 0; item < hd.frame[frame].model_count; item++)
            if (hd.frame[frame].models[item].object_owner == owner)
                hd.frame[frame].models[item].previous_record = UINT32_MAX;
        for (uint32 item = 0; item < hd.frame[frame].ui_count; item++)
            if (hd.frame[frame].ui[item].object_owner == owner)
                hd.frame[frame].ui[item].previous_record = UINT32_MAX;
        if (pool == HD_OBJECT_DYNAMIC)
            for (uint32 actor = 0; actor < ACTOR_COUNT; actor++)
                for (uint32 hand = 0; hand < 2; hand++)
                    if (hd.frame[frame].actor_hand[actor][hand].valid && hd.frame[frame].actor_hand[actor][hand].slot == slot)
                    {
                        hd.frame[frame].actor_model[actor][hand ? HD_ACTOR_RIGHT : HD_ACTOR_LEFT] = UINT32_MAX;
                    }
    }
}

void hd_trail_reset(uint32 resource)
{
    XASSERT(resource == 867 || resource == 868);
    for (uint32 frame = 0; frame < HD_FRAME_COUNT; frame++)
        hd.frame[frame].trail_model[resource - 867] = UINT32_MAX;
}

void hd_trail_append(uint32 resource, int reset)
{
    XASSERT(resource == 867 || resource == 868);
    if (reset)
        hd_trail_reset(resource);
    hd.trail_appends[resource - 867]++;
}

void hd_actor_part(uint32 actor, HD_ACTOR_PART part)
{
    XASSERT(actor < ACTOR_COUNT && part < HD_ACTOR_PART_COUNT);
    hd.actor_owner = actor;
    hd.actor_part = part;
}

void hd_actor_part_end(void)
{
    hd.actor_owner = UINT32_MAX;
}

void hd_collect_partial(void)
{
    if (hd.collecting)
        hd.frame[hd.current_frame].partial = 1;
}

/* The source clear writes both software pages regardless of raster policy */
void hd_pages_clear(void)
{
    if (hd.frame_open)
        gpu_frame_end();
    hd.frame_open = hd.frame_hd = hd.collecting = 0;
    hd.wait_valid = hd.wait_presented = hd.wait_sealed = 0;
    hd.history_pending = 1;
    gpu_set_raster(1);
    hd.vram_locked = hd.presenting = 0;
    for (uint32 page = 0; page < 2; page++)
    {
        hd.page_ready[page] = 0;
        hd.software_ready[page] = 1;
        hd.page_frame[page] = UINT32_MAX;
    }
    for (uint32 sample = 0; sample < HD_FRAME_COUNT; sample++)
    {
        hd.frame[sample].valid = 0;
        hd.frame[sample].partial = 0;
    }
}

void hd_frame_begin(int delayed)
{
    if (hd.frame_open)
        gpu_frame_end();
    const HD_FRAME *frame = hd.frame + hd.current_frame;
    int selected = xport_hd && !xport_is_headless() && (!delayed || (hd.collecting && !frame->broken && !frame->missing));
    if (selected != hd.frame_hd)
    {
        hd.history_pending = 1;
        hd.wait_valid = 0;
    }
    hd.frame_hd = selected;
    hd.wait_presented = hd.wait_sealed = 0;
    /* Rebuild software pages during mode handoff before exposing their contents */
    hd.frame_raster = !hd.frame_hd || !hd.page_ready[0] || !hd.page_ready[1];
    hd.frame_open = 1;
    gpu_frame_begin();
    gpu_set_raster(hd.frame_raster);
}

/* The selected source page uses the preceding upload phase, without an image copy */
static int snapshot_present(uint32 selected, const HD_FRAME *display, VID_RECT viewport, float phase)
{
    uint32 latest = hd.last_frame;
    if (selected >= HD_FRAME_COUNT || !hd.frame[selected].valid || !hd.vram_valid || !vid_begin(0, 0, 0))
        return 0;
    hd.last_frame = selected;
    hd.display_offset[0] = display->offset_x;
    hd.display_offset[1] = display->offset_y;
    hd.presenting = hd.vram_locked = 1;
    hd_render_begin();
    hd_render_phase(phase);
    int rendered = hd_ui_draw(viewport, HD_UI_BACKGROUND) && hd_terrain_draw(viewport) && hd_world_draw(viewport) && hd_ui_draw(viewport, HD_UI_OVERLAY);
    if (rendered)
        rendered = vid_end();
    else
        vid_cancel();
    hd.presenting = hd.vram_locked = 0;
    hd.last_frame = latest;
    return rendered;
}

static int page_present(uint32 page, const HD_FRAME *display, VID_RECT viewport)
{
    XASSERT(hd.page_vram_version[page] == hd.vram_version);
    return snapshot_present(hd.page_frame[page], display, viewport, 1);
}

void hd_vblank(void)
{
    hd.vblank_count++;
}

void hd_vblank_reset(void)
{
    hd.vblank_count = 0;
    hd.wait_valid = 0;
    hd.history_pending = 1;
}

static void invalidate_presentations(void)
{
    memset(hd.page_ready, 0, sizeof(hd.page_ready));
    hd.page_frame[0] = hd.page_frame[1] = UINT32_MAX;
    hd.wait_valid = hd.wait_presented = 0;
    hd.history_pending = 1;
}

sint32 hd_vsync(void)
{
    HD_FRAME *frame = hd.frame + hd.current_frame;
    int ready = hd.wait_valid && hd.frame_hd && hd.frame_open && !hd.frame_raster && hd.collecting && !frame->broken && !frame->missing && !frame->partial && !game_pause_flags;
    if (ready)
    {
        /* seal completed producers without publishing new native display offsets */
        hd_collect_seal(hd.display_offset[0], hd.display_offset[1]);
        hd.wait_sealed = 1;
        ready = frame->previous < HD_FRAME_COUNT && hd.frame[frame->previous].valid && hd.frame[frame->previous].sequence + 1 == frame->sequence;
    }
    if (ready)
    {
        for (;;)
        {
            if (VSync(-1) < 0 || hd.vblank_count - hd.wait_counter >= 2)
                break;
            uint64 field;
            float fract = xport_frame_fract(&field);
            uint64 origin = hd.clock_field + (uint32)(hd.wait_counter - hd.clock_counter);
            float elapsed = field >= origin ? (float)(field - origin) : -(float)(origin - field);
            float phase = (elapsed + fract - hd.clock_fract) * 0.5f;
            phase = phase < 0 ? 0 : phase > 1 ? 1 : phase;
            sint32 width, height;
            vid_resize(&width, &height);
            if (width <= 0 || height <= 0)
            {
                invalidate_presentations();
                break;
            }
            if (!hd_vram_sync())
                abort();
            if (!snapshot_present(hd.last_frame, frame, (VID_RECT){0, 0, width, height}, phase))
            {
                vid_resize(&width, &height);
                if (width > 0 && height > 0)
                    abort();
                invalidate_presentations();
                break;
            }
            hd.wait_presented = 1;
        }
    }
    /* retain the SDK's original callback service and blocking-sync bookkeeping */
    sint32 result = VSync(2);
    if (!hd.wait_valid || hd.vblank_count - hd.wait_counter > 4)
    {
        hd.clock_fract = xport_frame_fract(&hd.clock_field);
        hd.clock_counter = hd.vblank_count;
        if (hd.wait_valid)
            hd.history_pending = 1;
    }
    hd.wait_counter = hd.vblank_count;
    hd.wait_valid = result >= 0 && hd.frame_hd && !game_pause_flags;
    return result;
}

void hd_present(float phase)
{
    const HD_FRAME *frame = hd.frame + hd.last_frame;
    if (xport_is_headless() || !hd.frame_hd)
    {
        gpu_present();
        return;
    }
    sint32 width, height;
    vid_resize(&width, &height);
    if (width <= 0 || height <= 0)
    {
        invalidate_presentations();
        return;
    }
    if (!frame->valid || frame->missing || frame->partial)
    {
        gpu_present();
        return;
    }
    if (!snapshot_present(hd.last_frame, frame, (VID_RECT){0, 0, width, height}, phase))
        abort();
}

void hd_frame_end(int present, uint32 draw_page)
{
    if (!hd.frame_open)
        return;
    hd.frame_open = 0;
    gpu_frame_end();
    XASSERT(draw_page < 2);
    uint32 front = draw_page ^ 1u;
    const HD_FRAME *frame = hd.frame + hd.last_frame;
    hd.software_ready[draw_page] = hd.frame_raster && (!frame->partial || hd.software_ready[draw_page]);
    hd.page_ready[draw_page] = 0;
    hd.page_frame[draw_page] = UINT32_MAX;
    if (!present)
        return;
    if (xport_is_headless())
    {
        if (present != HD_PRESENT_DEFERRED)
            gpu_present();
        return;
    }
    sint32 width, height;
    vid_resize(&width, &height);
    if (width <= 0 || height <= 0)
    {
        invalidate_presentations();
        return;
    }
    VID_RECT viewport = {0, 0, width, height};
    int rendered = 1;
    int native_front = hd.page_ready[front] && (hd.frame_hd || !hd.software_ready[front]);
    if (hd.frame_hd)
        rendered = frame->valid && !frame->missing && !frame->partial;
    /* Display the opposite source snapshot with the palette phase it originally used */
    if (rendered && present == HD_PRESENT_DEFERRED)
        rendered = 1;
    else if (rendered && hd.frame_hd && hd.wait_presented)
        rendered = 1;
    else if (rendered && native_front)
        rendered = page_present(front, frame, viewport);
    else if (rendered && hd.software_ready[front])
        rendered = gpu_present();
    else if (rendered)
        rendered = vid_begin(0, 0, 0) && vid_end();
    /* Preserve one VRAM transport; advance it only after all old-page draws finish */
    if (rendered && (hd.frame_hd || native_front))
        rendered = hd_vram_sync() != NULL;
    if (hd.frame_hd && rendered)
    {
        hd.page_frame[draw_page] = hd.last_frame;
        hd.page_vram_version[draw_page] = hd.vram_version;
        hd.page_ready[draw_page] = 1;
    }
    else
        hd.page_frame[draw_page] = UINT32_MAX;
    hd.display_offset[0] = frame->offset_x;
    hd.display_offset[1] = frame->offset_y;
    XASSERT(rendered);
    if (!rendered)
        abort();
}

static uint32 snapshot_descriptors(HD_FRAME *frame)
{
    uint32 bank;
    if (frame->bank_count == HD_DESCRIPTOR_BANK_COUNT)
        return UINT32_MAX;
    if (frame->bank_count == frame->bank_capacity)
    {
        uint32 capacity = frame->bank_capacity ? frame->bank_capacity * 2 : 1;
        HD_DESCRIPTOR_BANK *grown = realloc(frame->banks, capacity * sizeof(*frame->banks));
        if (!grown)
            return UINT32_MAX;
        frame->banks = grown;
        frame->bank_capacity = capacity;
    }
    bank = frame->bank_count++;
    frame->banks[bank].descriptors = tex_desc;
    memcpy(frame->banks[bank].images, tex_images, sizeof(tex_images));
    return bank;
}

static int mutable_model_update(HD_MODEL *cache, MODEL_REFERENCE model)
{
    static const uint32 order[2][4] = {{0, 1, 3, 2}, {1, 0, 2, 3}};
    int changed = 0;
    for (uint32 face = 0; face < cache->face_count; face++)
        for (uint32 corner = 0; corner < 4; corner++)
        {
            const MODEL_FACE_RECORD *record = model.faces + face;
            int line = (sint32)record->words[1] < 0;
            uint32 index = record->vertex_offset[line ? corner & 1 : order[(record->flags >> 1) & 1][corner]] / 4;
            XASSERT(index < model.vertex_byte_len / sizeof(SVECTOR));
            const SVECTOR *point = model.vertices + index;
            const VID_VERTEX16 *vertex = cache->vertices + face * 4 + corner;
            changed |= point->vx != vertex->pos[0] || point->vy != vertex->pos[1] || point->vz != vertex->pos[2];
        }
    if (!changed)
        return 1;
    VID_VERTEX16 *vertices = (VID_VERTEX16 *)cache->vertices;
    if (!vertices)
        return 0;
    for (uint32 face = 0; face < cache->face_count; face++)
        for (uint32 corner = 0; corner < 4; corner++)
        {
            const MODEL_FACE_RECORD *record = model.faces + face;
            int line = (sint32)record->words[1] < 0;
            uint32 index = record->vertex_offset[line ? corner & 1 : order[(record->flags >> 1) & 1][corner]] / 4;
            const SVECTOR *point = model.vertices + index;
            VID_VERTEX16 *vertex = vertices + face * 4 + corner;
            vertex->pos[0] = point->vx;
            vertex->pos[1] = point->vy;
            vertex->pos[2] = point->vz;
            if (line)
            {
                uint32 other = record->vertex_offset[1 - (corner & 1)] / 4;
                XASSERT(other < model.vertex_byte_len / sizeof(SVECTOR));
                vertex->n[0] = model.vertices[other].vx;
                vertex->n[1] = model.vertices[other].vy;
                vertex->n[2] = model.vertices[other].vz;
            }
        }
    if (!model_gpu_update(cache))
        return 0;
    return model_bounds(cache, model);
}

static const HD_MODEL *mutable_model_capture(HD_FRAME *frame, uint32 type, MODEL_REFERENCE model)
{
    uint32 capacity = frame->mutable_capacity;
    if (!reserve_frame((void **)&frame->mutable_models, &frame->mutable_capacity, frame->mutable_count + 1, sizeof(*frame->mutable_models)))
        return NULL;
    if (frame->mutable_capacity != capacity)
        memset(frame->mutable_models + capacity, 0, (frame->mutable_capacity - capacity) * sizeof(*frame->mutable_models));
    HD_MUTABLE_MODEL *slot = frame->mutable_models + frame->mutable_count;
    HD_MODEL *cache = &slot->model;
    if (slot->type != type || !cache->mesh || cache->source.header != model.header || cache->source.vertices != model.vertices || cache->source.faces != model.faces || cache->face_count != (uint32)model.header->face_count || !mutable_model_update(cache, model))
        cache = model_build(cache, model, 0);
    slot->type = type;
    XASSERT(model.vertex_byte_len >= sizeof(slot->points));
    memcpy(slot->points, model.vertices, sizeof(slot->points));
    slot->appends = hd.trail_appends[type - 867];
    return cache;
}

static const HD_MODEL *draw_model(const HD_FRAME *frame, const HD_MODEL_DRAW *draw)
{
    return draw->mutable_model == UINT32_MAX ? hd.model + draw->type : &frame->mutable_models[draw->mutable_model].model;
}

static int model_is_decal(uint32 type)
{
    switch (type)
    {
        case 68:
        case 89:
        case 106:
        case 834:
        case 860:
        case 867:
        case 868:
        case 887:
        case 888:
        case 919:
            return 1;
        default:
            return 0;
    }
}

static int ordered_face(const HD_MODEL_DRAW *draw, const HD_MODEL *model, const HD_DESCRIPTOR_BANK *bank, uint32 face, HD_PASS_FACE *pass)
{
    const HD_FACE_RANGE *range = model->faces + face;
    if (!range->count)
        return 0;
    int line = range->descriptor == UINT16_MAX;
    const TEXTURE_DESCRIPTOR *texture = line ? NULL : bank->descriptors.item + range->descriptor;
    int semi = !line && ((texture->uv0.u & 1) || (!draw->faded && (draw->color & 0x02000000u)));
    if (!semi && !model_is_decal(draw->type) && (line || !draw->bias))
        return 0;
    uint32 page = line ? 0 : texture->tpage;
    if (draw->blend && !draw->faded)
        page = (page & ~96u) | (((draw->blend - 1u) & 2047u) << 5);
    static const HD_BLEND modes[4] = {HD_BLEND_HALF, HD_BLEND_ADD, HD_BLEND_SUBTRACT, HD_BLEND_QUARTER};
    *pass = (HD_PASS_FACE){face, modes[(page >> 5) & 3], semi};
    return 1;
}

static void prepare_model_passes(HD_FRAME *frame)
{
    frame->pass_count = 0;
    if (!frame->valid)
        return;
    for (uint32 item = 0; item < frame->model_count; item++)
    {
        HD_MODEL_DRAW *draw = frame->models + item;
        draw->pass_first = frame->pass_count;
        draw->pass_count = 0;
        /* interpolated trails may add a face between snapshots */
        if (draw->mutable_model != UINT32_MAX)
        {
            draw->pass_count = UINT32_MAX;
            continue;
        }
        const HD_MODEL *model = draw_model(frame, draw);
        const HD_DESCRIPTOR_BANK *bank = frame->banks + draw->descriptor_bank;
        for (uint32 face = 0; face < model->face_count; face++)
        {
            HD_PASS_FACE pass;
            if (!ordered_face(draw, model, bank, face, &pass))
                continue;
            if (!reserve_frame((void **)&frame->pass_faces, &frame->pass_capacity, frame->pass_count + 1, sizeof(*frame->pass_faces)))
            {
                frame->valid = 0;
                frame->broken = 1;
                return;
            }
            frame->pass_faces[frame->pass_count++] = pass;
            draw->pass_count++;
        }
    }
}

static void draw_bounds(const HD_RIG_PART *parts, HD_MODEL_DRAW *draw, const HD_MODEL *model)
{
    XASSERT(model->bounds_count == (draw->part_count ? draw->part_count : 1));
    for (uint32 part = 0; part < model->bounds_count; part++)
    {
        MATRIXF matrix = draw->matrix;
        if (draw->part_count)
            CompMatrixf(&draw->matrix, &parts[draw->first_part + part].matrix, &matrix);
        for (uint32 corner = 0; corner < 8; corner++)
        {
            float local[3];
            for (uint32 axis = 0; axis < 3; axis++)
                local[axis] = corner & (1u << axis) ? model->bounds[part].max[axis] : model->bounds[part].min[axis];
            VECTORF point = {local[0], local[1], local[2]};
            RotTransf(&matrix, &point, &point);
            const float camera[3] = {point.vx, point.vy, point.vz};
            for (uint32 axis = 0; axis < 3; axis++)
            {
                if ((!part && !corner) || camera[axis] < draw->bounds.min[axis])
                    draw->bounds.min[axis] = camera[axis];
                if ((!part && !corner) || camera[axis] > draw->bounds.max[axis])
                    draw->bounds.max[axis] = camera[axis];
            }
        }
    }
    /* Keep CPU/GPU rounding inside conservative culling bounds */
    for (uint32 axis = 0; axis < 3; axis++)
    {
        draw->bounds.min[axis] -= 1;
        draw->bounds.max[axis] += 1;
    }
}

/* Factor the original correction from an orthogonal hand anchor */
static void actor_hand_capture(HD_FRAME *frame, const HD_MODEL_DRAW *draw)
{
    uint32 hand = draw->actor_part == HD_ACTOR_LEFT ? 0 : 1;
    uint32 body_index = frame->actor_model[draw->actor][HD_ACTOR_BODY];
    uint32 part = frame->actor_attachment[draw->actor][hand];
    if (body_index >= frame->model_count)
        return;
    const HD_MODEL_DRAW *body = frame->models + body_index;
    if (part >= body->part_count)
        return;
    MATRIXF anchor, inverse;
    CompMatrixf(&body->matrix, &frame->parts[body->first_part + part].matrix, &anchor);
    IdentityMatrixf(&inverse);
    float length[3] = {0};
    for (uint32 col = 0; col < 3; col++)
        for (uint32 row = 0; row < 3; row++)
            length[col] += anchor.m[row][col] * anchor.m[row][col];
    for (uint32 row = 0; row < 3; row++)
    {
        if (length[row] <= 0)
            return;
        for (uint32 col = 0; col < 3; col++)
        {
            float dot = 0;
            for (uint32 axis = 0; axis < 3; axis++)
                dot += anchor.m[axis][row] * anchor.m[axis][col];
            if (row != col && dot * dot > 0.000001f * length[row] * length[col])
                return;
            inverse.m[row][col] = anchor.m[col][row] / length[row];
        }
    }
    VECTORF offset = {-anchor.t[0], -anchor.t[1], -anchor.t[2]};
    ApplyMatrixf(&inverse, &offset, &offset);
    TransMatrixf(&inverse, &offset);
    HD_ACTOR_HAND *record = &frame->actor_hand[draw->actor][hand];
    CompMatrixf(&inverse, &draw->matrix, &record->correction);
    record->part = part;
    record->slot = (uint32)actor_items[draw->actor].slot[hand ? ACTOR_ITEM_RIGHT : ACTOR_ITEM_LEFT];
    record->valid = 1;
}

static void collect_model(MODEL_REFERENCE model, const ACTOR_POSE *pose, const MATRIXF *matrix, uint32 color, uint32 blend, sint32 bias, int faded)
{
    HD_FRAME *frame;
    HD_MODEL_DRAW *draw;
    const HD_MODEL *cache;
    uint32 type, face, actor, bank;
    int new_bank;
    if (!hd.collecting)
        return;
    frame = hd.frame + hd.current_frame;
    for (type = 0; type < MODEL_TYPE_COUNT; type++)
        if (model_refs[type].header == model.header && model_refs[type].vertices == model.vertices && model_refs[type].faces == model.faces)
            break;
    XASSERT(type < MODEL_TYPE_COUNT);
    int mutable = type == 867 || type == 868;
    XASSERT(!mutable || !pose);
    cache = mutable ? mutable_model_capture(frame, type, model) : hd_model_mesh(type, model, pose != NULL);
    if (!cache || !reserve_frame((void **)&frame->models, &frame->model_capacity, frame->model_count + 1, sizeof(*frame->models)))
    {
        frame->broken = 1;
        return;
    }
    new_bank = frame->bank_count == 0;
    bank = frame->bank_count ? frame->bank_count - 1 : 0;
    for (face = 0; !new_bank && face < cache->face_count; face++)
    {
        uint32 descriptor;
        if (!cache->faces[face].count || cache->faces[face].descriptor == UINT16_MAX)
            continue;
        descriptor = cache->faces[face].descriptor;
        if (memcmp(frame->banks[bank].descriptors.item + descriptor, tex_desc.item + descriptor, sizeof(TEXTURE_DESCRIPTOR)) || memcmp(frame->banks[bank].images + descriptor, tex_images + descriptor, sizeof(TEXTURE_IMAGE)))
            new_bank = 1;
    }
    if (new_bank)
    {
        bank = snapshot_descriptors(frame);
        if (bank == UINT32_MAX)
        {
            frame->broken = 1;
            return;
        }
    }
    if (cache->unsupported_lines)
        frame->missing |= HD_MISSING_LINES;
    draw = frame->models + frame->model_count;
    *draw = (HD_MODEL_DRAW){0};
    draw->actor = hd.actor_owner;
    draw->actor_part = hd.actor_part;
    draw->type = type;
    draw->ordinal = frame->ordinal++;
    draw->mutable_model = mutable ? frame->mutable_count++ : UINT32_MAX;
    draw->color = color;
    draw->blend = blend;
    draw->bias = bias;
    draw->faded = faded;
    draw->descriptor_bank = bank;
    draw->matrix = *matrix;
    camera_world(&draw->camera);
    if (mutable)
        frame->trail_model[type - 867] = frame->model_count;
    if (pose)
    {
        const MODEL_RIG_HEADER *rig = (const MODEL_RIG_HEADER *)model.header;
        MATRIXF identity;
        HD_POSE_SAMPLE sample;
        uint32 attachment;
        if (!reserve_frame((void **)&frame->parts, &frame->part_capacity, frame->part_count + rig->part_count, sizeof(*frame->parts)))
        {
            frame->broken = 1;
            return;
        }
        IdentityMatrixf(&identity);
        draw->first_part = frame->part_count;
        draw->part_count = hd_pose_parts(model, pose, &identity, frame->parts + frame->part_count, rig->part_count, &sample);
        frame->part_count += draw->part_count;
        for (actor = 0; actor < ACTOR_POSE_COUNT; actor++)
            if (pose == actor_poses + actor)
            {
                if (actor < ACTOR_COUNT)
                {
                    if (draw->actor == UINT32_MAX && hd.object_owner == UINT32_MAX)
                    {
                        draw->actor = actor;
                        draw->actor_part = HD_ACTOR_BODY;
                    }
                    if (draw->actor < ACTOR_COUNT && draw->actor_part == HD_ACTOR_BODY)
                        memcpy(frame->actor_attachment[draw->actor], sample.attachment_part, sizeof(sample.attachment_part));
                }
                for (attachment = 0; attachment < ACTOR_POSE_MATRIX_COUNT; attachment++)
                    if (sample.attachment_mask & (1u << attachment))
                        CompMatrixf(&draw->matrix, sample.attachment + attachment, hd.actor_pose[actor].attachment + attachment);
                hd.actor_pose[actor].attachment_mask = sample.attachment_mask;
                break;
            }
    }
    if (draw->actor < ACTOR_COUNT)
    {
        frame->actor_model[draw->actor][draw->actor_part] = frame->model_count;
        if (draw->actor_part == HD_ACTOR_RIGHT || draw->actor_part == HD_ACTOR_LEFT)
            actor_hand_capture(frame, draw);
    }
    draw->object_owner = draw->actor < ACTOR_COUNT ? UINT32_MAX : hd.object_owner;
    draw->object_part = hd.object_part;
    draw->object_next = UINT32_MAX;
    if (draw->object_owner < HD_OBJECT_COUNT)
    {
        HD_OBJECT_SAMPLE *owner = frame->objects + draw->object_owner;
        if (owner->model_count)
            draw->object_next = owner->model_head;
        owner->model_head = frame->model_count;
        owner->model_count++;
    }
    draw_bounds(frame->parts, draw, cache);
    frame->model_count++;
}

void hd_collect_model(MODEL_REFERENCE model, const ACTOR_POSE *pose, const POSE_MATRIX *matrix, uint32 color, uint32 blend, sint32 bias, int faded)
{
    if (hd.collecting)
        collect_model(model, pose, &transform_slot(matrix)->matrix, color, blend, bias, faded);
}

static void matrix_translate(MATRIXF *matrix, float x, float y, float z)
{
    VECTORF point = {x, y, z};
    RotTransf(matrix, &point, &point);
    TransMatrixf(matrix, &point);
}

static void matrix_rotate(MATRIXF *matrix, uint32 axis, sint32 angle)
{
    rig_rotate(matrix, axis, ((0u - (uint32)angle) >> 4) & 4095u);
}

/* Preserve the original facing approximation without publishing dir_distance */
static sint32 object_direction(const MODEL_OBJECT *object, const FIXED_VEC3 *target)
{
    static const uint16 bias[4] = {16384, 0, 32768, 49152};
    sint32 x = target->x.whole - object->pos.x.whole, z = target->z.whole - object->pos.z.whole;
    uint32 quadrant = (z < 0) | ((x < 0) << 1), value;
    if (x < 0)
        x = -x;
    if (z < 0)
        z = -z;
    value = z < x ? (((uint32)z << 13) / (uint32)x) ^ 16383u : ((uint32)x << 13) / (uint32)(z ? z : 1);
    return (sint16)((value + bias[quadrant]) ^ ((quadrant == 0 || quadrant == 3) ? 16383u : 0u));
}

void hd_collect_culled_object(const MODEL_OBJECT *object, const POSE_MATRIX *source, HD_OBJECT_DRAW draw)
{
    if (!hd.collecting)
        return;
    XASSERT(object && object->kind < MODEL_TYPE_COUNT && draw >= HD_OBJECT_YAW && draw <= HD_OBJECT_ROADSIDE);
    MATRIXF matrix = transform_slot(source)->matrix;
    sint32 angle = draw == HD_OBJECT_FACING ? object_direction(object, actor_positions) : object->angles.yaw;
    matrix_rotate(&matrix, 1, angle);
    uint32 color = render_style.color.rgbc;
    if (draw == HD_OBJECT_ROADSIDE && matrix.t[2] > 8192)
    {
        float fade = (matrix.t[2] - 8192) / 128;
        uint32 shade = 128 - (fade > 96 ? 96 : (uint32)fade);
        color = 0x2c000000u | shade | (shade << 8) | (shade << 16);
    }
    collect_model(model_refs[object->kind], NULL, &matrix, color, render_style.blend_mode, 0, 0);
    if (draw != HD_OBJECT_ROADSIDE)
        return;
    collect_model(model_refs[object->kind == 80 ? 89 : 106], NULL, &matrix, 0x2d000000u, 0, 8192, 0);
    matrix_rotate(&matrix, 1, -16384);
    matrix_translate(&matrix, -384 - (object->kind == 78 ? 128 : object->kind == 80 ? 256 : 0), 0, 0);
    uint32 shade = 64 - 16 * (game_session.tick & 1u);
    color = 0x2c000000u | shade | (shade << 8) | (shade << 16);
    collect_model(model_refs[919], NULL, &matrix, color, 0, 4096, 0);
}

static uint32 terrain_map(const uint16 tiles[64][64])
{
    for (uint32 map = 0; map < hd.terrain_mesh_count; map++)
        if (!memcmp(hd.terrain_meshes[map].tiles, tiles, sizeof(hd.terrain_meshes[map].tiles)))
            return map;
    if (!reserve_frame((void **)&hd.terrain_meshes, &hd.terrain_mesh_capacity, hd.terrain_mesh_count + 1, sizeof(*hd.terrain_meshes)))
        return UINT32_MAX;
    if (!hd.terrain_grid)
    {
        hd.terrain_grid = vid_mesh_create_3d(256 * 4);
        if (!hd.terrain_grid)
            return UINT32_MAX;
        VID_VERTEX3D *vertices = vid_mesh_data_3d(hd.terrain_grid);
        memset(vertices, 0, 256 * 4 * sizeof(*vertices));
        for (uint32 row = 0; row < 16; row++)
            for (uint32 column = 0; column < 16; column++)
                for (uint32 corner = 0; corner < 4; corner++)
                {
                    VID_VERTEX3D *vertex = vertices + (255 - row * 16 - column) * 4 + corner;
                    vertex->pos[0] = (float)column;
                    vertex->pos[2] = (float)row;
                    vertex->pos[3] = (float)((corner << HD_VERTEX_CORNER_SHIFT) | HD_VERTEX_TWO_SIDED | HD_VERTEX_TERRAIN);
                }
        vid_mesh_commit(hd.terrain_grid);
    }
    VID_TEXTURE *image = vid_tex_create_fmt(32, 32, VID_RGBA16UI);
    if (!image)
        return UINT32_MAX;
    for (uint32 row = 0; row < 64; row++)
        for (uint32 col = 0; col < 64; col++)
        {
            XASSERT(tiles[row][col] == 0xfffc || (tiles[row][col] % 3 == 0 && tiles[row][col] / 3 < TEXTURE_DESCRIPTOR_CAPACITY));
        }
    memcpy(vid_tex_data(image), tiles, sizeof(hd.terrain_meshes[0].tiles));
    vid_tex_commit(image);
    HD_TERRAIN_MESH *entry = hd.terrain_meshes + hd.terrain_mesh_count;
    *entry = (HD_TERRAIN_MESH){0};
    memcpy(entry->tiles, tiles, sizeof(entry->tiles));
    entry->image = image;
    return hd.terrain_mesh_count++;
}

void hd_collect_terrain(const uint16 tiles[64][64], uint32 column, uint32 row, const POSE_MATRIX *matrix, uint32 color, int faded)
{
    HD_FRAME *frame;
    HD_TERRAIN_DRAW *draw;
    uint32 bank, old_capacity;
    if (!hd.collecting)
        return;
    frame = hd.frame + hd.current_frame;
    old_capacity = frame->terrain_capacity;
    if (!reserve_frame((void **)&frame->terrain, &frame->terrain_capacity, frame->terrain_count + 1, sizeof(*frame->terrain)))
        goto failed;
    if (frame->terrain_capacity > old_capacity)
        memset(frame->terrain + old_capacity, 0, (frame->terrain_capacity - old_capacity) * sizeof(*frame->terrain));
    draw = frame->terrain + frame->terrain_count;
    bank = frame->bank_count ? frame->bank_count - 1 : UINT32_MAX;
    if (bank == UINT32_MAX || memcmp(&frame->banks[bank].descriptors, &tex_desc, sizeof(tex_desc)) || memcmp(frame->banks[bank].images, tex_images, sizeof(tex_images)))
        bank = snapshot_descriptors(frame);
    if (bank == UINT32_MAX)
        goto failed;
    draw->tile_map = terrain_map(tiles);
    if (draw->tile_map == UINT32_MAX)
        goto failed;
    draw->mesh = hd.terrain_grid;
    draw->column = column & 63u;
    draw->row = row & 63u;
    draw->matrix = transform_slot(matrix)->matrix;
    camera_world(&draw->camera);
    draw->color = color;
    draw->faded = faded;
    draw->descriptor_bank = bank;
    if (!faded && (color & 0x02000000u))
        frame->missing |= HD_MISSING_TERRAIN_BLEND;
    frame->terrain_count++;
    return;
failed:
    frame->broken = 1;
}

HD_UI_LAYOUT hd_ui_layout(HD_UI_LAYOUT layout)
{
    HD_UI_LAYOUT previous = hd.ui_layout;
    XASSERT(layout >= HD_UI_CENTER && layout <= HD_UI_STRETCH);
    hd.ui_layout = layout;
    return previous;
}

static HD_UI_DRAW *collect_ui_record(uint32 bucket, HD_UI_LAYER layer)
{
    HD_FRAME *frame;
    HD_UI_DRAW *draw;
    if (!hd.collecting)
        return NULL;
    XASSERT(bucket < RENDER_OT_COUNT && layer >= HD_UI_BACKGROUND && layer <= HD_UI_EFFECT);
    frame = hd.frame + hd.current_frame;
    if (!reserve_frame((void **)&frame->ui, &frame->ui_capacity, frame->ui_count + 1, sizeof(*frame->ui)))
    {
        frame->broken = 1;
        return NULL;
    }
    draw = frame->ui + frame->ui_count;
    draw->next = frame->ui_head[layer][bucket];
    frame->ui_head[layer][bucket] = frame->ui_count++;
    draw->bucket = (uint16)bucket;
    draw->ordinal = frame->ordinal++;
    draw->layout = hd.ui_layout;
    draw->layer = layer;
    draw->region[0] = draw->region[1] = 0;
    draw->extent = 65535;
    draw->background_part = HD_BACKGROUND_NONE;
    draw->horizon_edges = 0;
    draw->hand_actor = UINT32_MAX;
    if (layer == HD_UI_EFFECT)
        camera_world(&draw->camera);
    draw->object_owner = layer == HD_UI_EFFECT ? hd.object_owner : UINT32_MAX;
    draw->object_part = hd.object_part;
    draw->object_next = UINT32_MAX;
    if (draw->object_owner < HD_OBJECT_COUNT)
    {
        HD_OBJECT_SAMPLE *owner = frame->objects + draw->object_owner;
        if (owner->ui_count)
            draw->object_next = owner->ui_head;
        owner->ui_head = frame->ui_count - 1;
        owner->ui_count++;
    }
    return draw;
}

static HD_UI_DRAW *collect_ui(const HD_UI_VERTEX vertices[4], uint16 page, uint16 clut, uint16 flags, uint32 bucket, HD_UI_LAYER layer)
{
    HD_UI_DRAW *draw = collect_ui_record(bucket, layer);
    if (!draw)
        return NULL;
    XASSERT(!(flags & ~(HD_TEX_TEXTURED | HD_TEX_RAW | HD_TEX_SEMI | HD_TEX_SKY | HD_TEX_DISTANT_GROUND | HD_TEX_FILTERED)));
    memcpy(draw->vertices, vertices, sizeof(draw->vertices));
    draw->page = page;
    draw->clut = clut;
    draw->flags = flags;
    return draw;
}

void hd_collect_ui(const HD_UI_VERTEX vertices[4], uint16 page, uint16 clut, uint16 flags, uint32 bucket, HD_UI_LAYER layer)
{
    XASSERT(!(flags & HD_TEX_FILTERED) || (flags & (HD_TEX_DISTANT_GROUND | HD_TEX_SKY)));
    collect_ui(vertices, page, clut, flags, bucket, layer);
}

void hd_collect_ui_image(const HD_UI_VERTEX vertices[4], uint16 page, uint16 clut, uint16 flags, uint32 bucket, HD_UI_LAYER layer, const TEXTURE_IMAGE *image)
{
    int bounded = image && image->uploaded && image->mode == ((page >> 7) & 3) && image->width && image->height && image->x + image->width <= 256 && image->y + image->height <= 256;
    XASSERT((flags & HD_TEX_TEXTURED) && !(flags & (HD_TEX_SKY | HD_TEX_DISTANT_GROUND)));
    for (uint32 corner = 0; bounded && corner < 4; corner++)
        bounded = vertices[corner].u >= image->x && vertices[corner].u <= image->x + image->width && vertices[corner].v >= image->y && vertices[corner].v <= image->y + image->height;
    HD_UI_DRAW *draw = collect_ui(vertices, page, clut, bounded ? flags : flags & ~HD_TEX_FILTERED, bucket, layer);
    if (draw && bounded)
    {
        draw->region[0] = image->x | HD_RECT_CLAMP;
        draw->region[1] = image->y;
        draw->extent = (image->width - 1) | ((image->height - 1) << 8);
    }
}

void hd_collect_background_gradient(float top, float bottom, uint32 top_color, uint32 bottom_color, HD_BACKGROUND_PART part, uint32 horizon_edges)
{
    if (!hd.collecting)
        return;
    HD_FRAME *frame = hd.frame + hd.current_frame;
    HD_UI_VERTEX vertices[4] = {{0, top, 0, 0, top_color}, {frame->width, top, 0, 0, top_color}, {0, bottom, 0, 0, bottom_color}, {frame->width, bottom, 0, 0, bottom_color}};
    uint32 count = frame->ui_count;
    HD_UI_LAYOUT previous = hd_ui_layout(HD_UI_STRETCH);
    hd_collect_ui(vertices, 0, 0, 0, 0, HD_UI_BACKGROUND);
    hd_ui_layout(previous);
    if (count != frame->ui_count)
    {
        frame->ui[count].background_part = part;
        frame->ui[count].horizon_edges = horizon_edges;
    }
}

void hd_collect_mask(const HD_MASK_VERTEX vertices[4], uint32 bucket, HD_BACKGROUND_PART part)
{
    HD_UI_DRAW *draw = collect_ui_record(bucket, HD_UI_BACKGROUND);
    if (!draw)
        return;
    memcpy(draw->mask, vertices, sizeof(draw->mask));
    draw->background_part = part;
    camera_world(&draw->camera);
    draw->page = draw->clut = 0;
    draw->flags = HD_TEX_BACKGROUND_MASK;
}

static void effect_texture(HD_UI_DRAW *draw, uint32 texture, uint32 color, uint32 blend)
{
    const TEXTURE_DESCRIPTOR *source = tex_desc.item + texture;
    draw->effect.fixed = 0;
    draw->effect.offset[0] = draw->effect.offset[1] = 0;
    draw->effect.color = color;
    draw->effect.uv[0] = source->uv0.uv & 65534u;
    draw->effect.uv[1] = source->uv1.uv;
    draw->effect.uv[2] = source->uv2.uv;
    draw->effect.uv[3] = source->uv3.uv;
    draw->page = source->tpage;
    draw->clut = source->clut;
    draw->flags = HD_TEX_TEXTURED;
    if (color & 0x01000000u)
        draw->flags |= HD_TEX_RAW;
    if ((source->uv0.u & 1) || (color & 0x02000000u) || blend)
        draw->flags |= HD_TEX_SEMI;
    if (blend)
        draw->page = (draw->page & ~96u) | (((uint16)blend - 1u) << 5);
}

static uint32 effect_bucket(float depth, uint32 count)
{
    uint32 index = depth <= 0 ? 0 : depth >= count * 4 ? count - 1 : (uint32)(depth * 0.25f);
    sint32 offset = depth_table[index];
    XASSERT(offset >= 0 && offset < RENDER_OT_COUNT * 4 && !(offset & 3));
    return (uint32)offset >> 2;
}

static void filter_billboard(HD_UI_DRAW *draw, uint32 texture)
{
    const TEXTURE_IMAGE *image = tex_images + texture;
    if (!image->uploaded || image->mode != ((draw->page >> 7) & 3) || !image->width || !image->height || image->x + image->width > 256 || image->y + image->height > 256)
        return;
    uint32 min_u = 255, min_v = 255, max_u = 0, max_v = 0;
    for (uint32 corner = 0; corner < 4; corner++)
    {
        uint32 uv = draw->effect.uv[corner], u = uv & 255u, v = uv >> 8;
        if (u < image->x || u >= (uint32)image->x + image->width || v < image->y || v >= (uint32)image->y + image->height)
            return;
        if (u < min_u)
            min_u = u;
        if (v < min_v)
            min_v = v;
        if (u > max_u)
            max_u = u;
        if (v > max_v)
            max_v = v;
    }
    draw->flags |= HD_TEX_FILTERED;
    draw->region[0] = min_u | HD_RECT_CLAMP;
    draw->region[1] = min_v;
    draw->extent = (max_u - min_u) | ((max_v - min_v) << 8);
}

static HD_UI_DRAW *collect_effect_center(VECTORF center, float depth_bias)
{
    /* retain clipped endpoints so interpolation can cross the near plane */
    HD_UI_DRAW *draw = collect_ui_record(effect_bucket(center.vz + depth_bias, DEPTH_TABLE_COUNT), HD_UI_EFFECT);
    if (draw)
    {
        memset(&draw->effect, 0, sizeof(draw->effect));
        draw->effect.point[0] = center;
        draw->effect.depth_bias = depth_bias;
        draw->effect.fixed = 0;
    }
    return draw;
}

static HD_UI_DRAW *collect_effect_point(const POSE_MATRIX *matrix, VECTORF local, float depth_bias)
{
    const MATRIXF *view = hd_transform_current(matrix);
    if (!view)
        return NULL;
    VECTORF center;
    RotTransf(view, &local, &center);
    return collect_effect_center(center, depth_bias);
}

void hd_collect_sprite(const POSE_MATRIX *matrix, VECTORF local, float size, float depth_bias, uint32 texture, uint32 color)
{
    HD_UI_DRAW *draw = collect_effect_point(matrix, local, depth_bias);
    if (!draw)
        return;
    XASSERT(texture < TEXTURE_DESCRIPTOR_CAPACITY);
    effect_texture(draw, texture, color, 0);
    uint32 extent = ((draw->effect.uv[3] - draw->effect.uv[0]) & 65535u) + 257;
    draw->effect.size[0] = size;
    draw->effect.extent[0] = extent & 255;
    draw->effect.extent[1] = extent >> 8;
    draw->flags |= HD_TEX_BILLBOARD;
    filter_billboard(draw, texture);
}

void hd_collect_culled_spr(const MODEL_OBJECT *object, const EFFECT *effect, const POSE_MATRIX *matrix, const uint32 motion_sizes[8])
{
    if (!hd.collecting)
        return;
    XASSERT((object || effect) && (!object || !effect));
    uint32 type = effect ? effect->kind : object->kind;
    switch (type)
    {
        case 575:
        case 576:
        case 865:
        case 866:
        case 870:
        case 871:
        case 872:
        case 873:
        case 874:
        case 877:
        case 879:
        case 880:
            break;
        default:
            return;
    }
    XASSERT(effect ? effect->payload_kind == EFFECT_PAYLOAD_SPRITE : object->payload_kind == MODEL_PAYLOAD_SPRITE);
    sint32 age = effect ? effect->spr.frame : object->spr.frame;
    sint32 sample = 0, base = 0, size = 512;
    uint32 color = render_style.color.rgbc, shade;
    switch (type)
    {
        case 870:
            shade = 32u - ((uint32)age << 1);
            color = 0x2c000000u + (shade << 16) + (shade << 8) + shade;
            size = 384;
            base = 33;
            sample = age;
            break;
        case 871:
        case 872:
            shade = 32u - age;
            color = type == 871 ? 0x2c000000u + (shade << 16) + (shade << 8) + shade : 0x2c000000u + (shade << 8);
            base = 33;
            sample = age >> 1;
            break;
        case 879:
            base = 102;
            sample = age;
            break;
        case 873:
        case 874:
        case 880:
            color = 0x2c303030u;
            base = type == 880 ? 106 : 88;
            sample = age >> (type == 874 ? 2 : 1);
            break;
        case 877:
            XASSERT(age >= 0 && age < 8);
            size = (sint32)motion_sizes[age];
            color = 0x2c608080u;
            base = 131;
            sample = age;
            break;
        case 865:
        case 866:
            size = 256;
            base = type == 865 ? 7 : 66;
            sample = age;
            break;
        case 575:
            shade = 64u - ((uint32)age << 2);
            color = 0x2c000000u + (shade << 14) + (shade << 7) + shade;
            size = ((uint32)age << 5) + 256u;
            base = 73;
            break;
        case 576:
            shade = 32u - (age >> 1);
            color = 0x2c000000u + (shade << 16) + (shade << 8) + shade;
            size = ((uint32)age << 5) + 256u;
            base = 87;
            break;
    }
    XASSERT(base + sample >= 0 && base + sample < TEXTURE_MAP_COUNT);
    hd_collect_sprite(matrix, (VECTORF){0}, (float)size, type == 877 ? -16.0f : 0.0f, tex_map[base + sample], color);
}

static void collect_fixed_sprite(HD_UI_DRAW *draw, float offset_x, float offset_y, uint32 texture)
{
    if (!draw)
        return;
    XASSERT(texture < TEXTURE_DESCRIPTOR_CAPACITY);
    effect_texture(draw, texture, 0x2d000000, 0);
    uint32 extent = (draw->effect.uv[3] - draw->effect.uv[0]) & 65535u;
    draw->effect.extent[0] = (extent & 255) + 1;
    draw->effect.extent[1] = (extent >> 8) + 1;
    draw->effect.size[0] = 1;
    draw->effect.offset[0] = offset_x;
    draw->effect.offset[1] = offset_y;
    draw->effect.fixed = 1;
    draw->flags |= HD_TEX_BILLBOARD;
    filter_billboard(draw, texture);
    if (hd.actor_owner < ACTOR_COUNT && hd.actor_part == HD_ACTOR_RIGHT)
        draw->hand_actor = hd.actor_owner;
}

void hd_collect_sprite_fixed(const POSE_MATRIX *matrix, VECTORF local, float offset_x, float offset_y, float depth_bias, uint32 texture)
{
    collect_fixed_sprite(collect_effect_point(matrix, local, depth_bias), offset_x, offset_y, texture);
}

void hd_collect_particle(const POSE_MATRIX *matrix, VECTORF local, float offset_x, uint32 color, HD_PARTICLE_SHAPE shape)
{
    HD_UI_DRAW *draw = collect_effect_point(matrix, local, 0);
    if (!draw)
        return;
    XASSERT(shape >= HD_PARTICLE_POINT && shape <= HD_PARTICLE_QUAD);
    draw->effect.extent[0] = shape == HD_PARTICLE_QUAD ? 2 : 1;
    draw->effect.extent[1] = shape == HD_PARTICLE_POINT ? 1 : 2;
    draw->effect.size[0] = 1;
    draw->effect.offset[0] = offset_x;
    draw->effect.offset[1] = 0;
    draw->effect.fixed = 1;
    draw->effect.color = ((color & 31) << 3) | ((color & 0x3e0) << 6) | ((color & 0x7c00) << 9);
    memset(draw->effect.uv, 0, sizeof(draw->effect.uv));
    draw->page = draw->clut = 0;
    draw->flags = HD_TEX_BILLBOARD;
}

void hd_collect_ribbon(const POSE_MATRIX *matrix, VECTORF a, VECTORF b, float size_a, float size_b, float depth_bias, uint32 texture, uint32 color, uint32 blend)
{
    const MATRIXF *view = hd_transform_current(matrix);
    if (!view)
        return;
    XASSERT(texture < TEXTURE_DESCRIPTOR_CAPACITY);
    VECTORF points[2];
    RotTransf(view, &a, points);
    RotTransf(view, &b, points + 1);
    float depth = (points[0].vz + points[1].vz) * 0.5f + depth_bias;
    HD_UI_DRAW *draw = collect_ui_record(effect_bucket(depth, 2048), HD_UI_EFFECT);
    if (!draw)
        return;
    memset(&draw->effect, 0, sizeof(draw->effect));
    effect_texture(draw, texture, color, blend);
    draw->effect.point[0] = points[0];
    draw->effect.point[1] = points[1];
    draw->effect.size[0] = size_a;
    draw->effect.size[1] = size_b;
    draw->effect.depth_bias = depth_bias;
    const TEXTURE_DESCRIPTOR *source = tex_desc.item + texture;
    draw->effect.extent[0] = ((sint8)source->uv1.u - (sint8)source->uv0.u) >> 1;
    draw->effect.extent[1] = ((sint8)source->uv3.u - (sint8)source->uv2.u) >> 1;
    draw->flags |= HD_TEX_RIBBON;
}

void hd_collect_horizon(const POSE_MATRIX *matrix)
{
    const MATRIXF *view = hd_transform_current(matrix);
    if (view)
    {
        HD_FRAME *frame = hd.frame + hd.current_frame;
        camera_world(&frame->camera);
        frame->horizon_view = *view;
        frame->horizon_valid = 1;
        VECTORF point = {0, 0, 32767};
        RotTransf(view, &point, &point);
        float depth = point.vz > 1 ? point.vz : 1;
        frame->horizon = frame->center_y + frame->focal * point.vy / depth;
    }
}

float hd_horizon(void)
{
    return hd.collecting ? hd.frame[hd.current_frame].horizon : 0;
}

void hd_collect_parallax(float top)
{
    if (!hd.collecting)
        return;
    HD_FRAME *frame = hd.frame + hd.current_frame;
    float left = 32 + (sint16)camera.angles.yaw / 64.0f - (sint32)camera.pos.x.bits / 16777216.0f;
    const HD_UI_VERTEX vertices[4] = {{left, top, 0, 0, 0xffffff}, {left + 256, top, 256, 0, 0xffffff}, {left, top + 256, 0, 256, 0xffffff}, {left + 256, top + 256, 256, 256, 0xffffff}};
    const TEXTURE_IMAGE image = {0, 0, 256, 256, 1, 1};
    uint32 count = frame->ui_count;
    hd_collect_ui_image(vertices, 142, 0x7c00, HD_TEX_TEXTURED | HD_TEX_RAW | HD_TEX_FILTERED, 0, HD_UI_BACKGROUND, &image);
    if (count != frame->ui_count)
    {
        frame->ui[count].background_part = HD_BACKGROUND_PARALLAX;
        frame->ui[count].horizon_edges = HD_HORIZON_TOP | HD_HORIZON_BOTTOM;
        frame->ui[count].parallax_yaw = camera.angles.yaw;
    }
}

void hd_collect_sky(float scroll, float top, float height, uint8 center_v, uint8 side_v)
{
    HD_FRAME *frame;
    HD_UI_LAYOUT previous;
    uint32 count;
    if (!hd.collecting)
        return;
    XASSERT((height == 128 || height == 256) && center_v + height <= 256 && side_v + height <= 256);
    XASSERT(scroll >= -65536 && scroll <= 65536);
    scroll -= (sint32)(scroll / 512.0f) * 512.0f;
    frame = hd.frame + hd.current_frame;
    float width = (float)frame->width;
    const HD_UI_VERTEX vertices[4] = {{0, top, -scroll, 0, 0xffffff}, {width, top, width - scroll, 0, 0xffffff}, {0, top + height, -scroll, height, 0xffffff}, {width, top + height, width - scroll, height, 0xffffff}};
    count = frame->ui_count;
    previous = hd_ui_layout(HD_UI_STRETCH);
    hd_collect_ui(vertices, 142, 0x7c00, HD_TEX_TEXTURED | HD_TEX_RAW | HD_TEX_SKY | HD_TEX_FILTERED, 0, HD_UI_BACKGROUND);
    hd_ui_layout(previous);
    if (frame->ui_count != count)
    {
        frame->ui[count].sky_v = center_v | ((uint16)side_v << 8);
        /* Each filter tap wraps X into its own bank and clamps within that row */
        frame->ui[count].region[1] = (uint16)height - 1;
    }
}

void hd_collect_ground(const uint8 colors[16384], float x, float y, float z, uint16 yaw, float top, uint32 bucket, int filtered)
{
    HD_FRAME *frame;
    HD_GROUND_MAP *map;
    HD_UI_LAYOUT previous;
    MATRIXF basis;
    uint32 count, base;
    if (!hd.collecting)
        return;
    frame = hd.frame + hd.current_frame;
    if (frame->ground_count == HD_GROUND_MAP_COUNT)
        goto failed;
    if (frame->ground_count == frame->ground_capacity)
    {
        uint32 capacity = frame->ground_capacity ? frame->ground_capacity * 2 : 1;
        HD_GROUND_MAP *grown = realloc(frame->ground_maps, capacity * sizeof(*frame->ground_maps));
        if (!grown)
            goto failed;
        frame->ground_maps = grown;
        frame->ground_capacity = capacity;
    }
    map = frame->ground_maps + frame->ground_count;
    memcpy(map->colors, colors, sizeof(map->colors));
    IdentityMatrixf(&basis);
    RotMatrixYf(RAD12 * (sint16)yaw / 16.0f, &basis);
    basis.t[0] = x;
    basis.t[1] = y;
    basis.t[2] = z;
    map->pose = basis;
    map->camera[0] = x;
    map->camera[1] = y;
    map->camera[2] = z;
    map->camera[3] = basis.m[0][2];
    map->camera[4] = basis.m[0][0];
    float width = (float)frame->width;
    const HD_UI_VERTEX vertices[4] = {{0, top, 0, 0, 0xffffff}, {width, top, width, 0, 0xffffff}, {0, top + 21, 0, 15, 0xffffff}, {width, top + 21, width, 15, 0xffffff}};
    count = frame->ui_count;
    base = HD_GROUND_MAP_BASE + (hd.current_frame * HD_GROUND_MAP_COUNT + frame->ground_count) * HD_GROUND_MAP_WORDS;
    previous = hd_ui_layout(HD_UI_STRETCH);
    hd_collect_ui(vertices, (uint16)base, (uint16)(base >> 16), HD_TEX_DISTANT_GROUND | (filtered ? HD_TEX_FILTERED : 0), bucket, HD_UI_BACKGROUND);
    hd_ui_layout(previous);
    if (frame->ui_count != count)
        frame->ui[count].ground_map = (uint16)frame->ground_count++;
    return;
failed:
    frame->broken = 1;
}

void hd_collect_ui_sprite(sint32 x, sint32 y, uint32 texture, uint32 bucket)
{
    const TEXTURE_DESCRIPTOR *source;
    uint32 uv, extent, width, height, corner;
    HD_UI_VERTEX vertices[4];
    uint16 flags = HD_TEX_TEXTURED | HD_TEX_RAW;
    if (!hd.collecting)
        return;
    XASSERT(texture < TEXTURE_DESCRIPTOR_CAPACITY);
    source = tex_desc.item + texture;
    uv = source->uv0.uv & 65534u;
    extent = (source->uv3.uv - uv) & 65535u;
    width = (extent & 255) + 1;
    height = (extent >> 8) + 1;
    if (source->uv0.u & 1)
        flags |= HD_TEX_SEMI;
    for (corner = 0; corner < 4; corner++)
        vertices[corner] = (HD_UI_VERTEX){(sint16)x + (float)((corner & 1) ? width : 0), (sint16)y + (float)((corner & 2) ? height : 0), (float)((uv & 255) + ((corner & 1) ? width : 0)), (float)((uv >> 8) + ((corner & 2) ? height : 0)), 0xffffff};
    hd_collect_ui_image(vertices, source->tpage, source->clut, flags, bucket, HD_UI_OVERLAY, tex_images + texture);
}

void hd_collect_ui_scaled_sprite(sint32 x, sint32 y, sint32 depth, uint32 size, uint32 texture, uint32 color, uint32 bucket, int filtered)
{
    const TEXTURE_DESCRIPTOR *source;
    uint32 uv, extent, width, height, corner;
    float dx, dy;
    HD_UI_VERTEX vertices[4];
    uint16 flags = HD_TEX_TEXTURED;
    if (!hd.collecting)
        return;
    XASSERT(texture < TEXTURE_DESCRIPTOR_CAPACITY && depth);
    source = tex_desc.item + texture;
    uv = source->uv0.uv & 65534u;
    extent = ((source->uv3.uv - uv) & 65535u) + 257;
    width = extent & 255;
    height = extent >> 8;
    dx = (float)width * size / depth;
    dy = (float)height * size / depth;
    if (filtered)
        flags |= HD_TEX_FILTERED;
    if (color & 0x01000000u)
        flags |= HD_TEX_RAW;
    if ((source->uv0.u & 1) || (color & 0x02000000u))
        flags |= HD_TEX_SEMI;
    const uint16 coords[4] = {(uint16)uv, source->uv1.uv, source->uv2.uv, source->uv3.uv};
    for (corner = 0; corner < 4; corner++)
        vertices[corner] = (HD_UI_VERTEX){(sint16)x + ((corner & 1) ? dx - 1 : -dx), (sint16)y + ((corner & 2) ? dy - 1 : -dy), (float)(coords[corner] & 255), (float)(coords[corner] >> 8), color};
    hd_collect_ui_image(vertices, source->tpage, source->clut, flags, bucket, HD_UI_OVERLAY, tex_images + texture);
}

void hd_collect_ui_gradient(sint32 x, sint32 y, uint32 texture, const uint32 colors[4], uint32 bucket)
{
    const TEXTURE_DESCRIPTOR *source;
    uint32 extent, corner;
    HD_UI_VERTEX vertices[4];
    if (!hd.collecting)
        return;
    XASSERT(texture < TEXTURE_DESCRIPTOR_CAPACITY);
    source = tex_desc.item + texture;
    extent = (source->uv3.uv - source->uv0.uv) & 65535u;
    const uint16 coords[4] = {source->uv0.uv, source->uv1.uv, source->uv2.uv, source->uv3.uv};
    for (corner = 0; corner < 4; corner++)
        vertices[corner] = (HD_UI_VERTEX){(sint16)(x + ((corner & 1) ? extent & 255 : 0)), (sint16)(y + ((corner & 2) ? extent >> 8 : 0)), (float)(coords[corner] & 255), (float)(coords[corner] >> 8), colors[corner]};
    hd_collect_ui(vertices, source->tpage, source->clut, HD_TEX_TEXTURED, bucket, HD_UI_OVERLAY);
}

static void actor_item(uint32 actor, int left, int player, uint32 color)
{
    sint32 slot = actor_items[actor].slot[left ? ACTOR_ITEM_LEFT : ACTOR_ITEM_RIGHT];
    if (slot == -1)
        return;
    XASSERT(slot >= 0 && slot < DYNAMIC_OBJECT_COUNT);
    const MODEL_OBJECT *item = dynamic_objects + slot;
    uint32 kind = item->kind;
    if (kind == UINT32_MAX)
        return;
    XASSERT(kind - DYNAMIC_ITEM_TYPE_FIRST < DYNAMIC_ITEM_TYPE_COUNT);
    uint32 attachment = left ? 0 : 1;
    MATRIXF matrix;
    if (hd.actor_pose[actor].attachment_mask & (1u << attachment))
        matrix = hd.actor_pose[actor].attachment[attachment];
    else
        matrix_from_pose(&actor_poses[actor].matrix[attachment], &matrix);
    matrix_translate(&matrix, 0, -12, 0);
    matrix_rotate(&matrix, 1, 6144);
    sint32 type = actor_types[actor];
    if (player && !left)
    {
        if (type == 1 || type == 3)
        {
            if (kind == 506)
            {
                matrix_translate(&matrix, 0, 0, -12);
                if (type == 1)
                {
                    matrix_rotate(&matrix, 0, -1536);
                    matrix_rotate(&matrix, 1, -2048);
                }
                else
                {
                    matrix_rotate(&matrix, 1, -3072);
                    matrix_rotate(&matrix, 0, -1024);
                }
            }
            else
            {
                matrix_rotate(&matrix, 1, 61440);
                matrix_rotate(&matrix, 2, 4096);
            }
        }
        if (type == 2)
        {
            if (kind == 506)
                matrix_rotate(&matrix, 0, -2560);
            else
            {
                if (kind == 515)
                {
                    matrix_translate(&matrix, 0, 0, 2);
                    matrix_rotate(&matrix, 0, 64512);
                    matrix_rotate(&matrix, 1, 10240);
                }
                else
                    matrix_rotate(&matrix, 1, 61440);
                matrix_rotate(&matrix, 2, 64512);
            }
        }
    }
    if (player && left && (type == 1 || type == 3) && kind == 515)
    {
        matrix_translate(&matrix, -8, -8, -20);
        matrix_rotate(&matrix, 0, 59392);
        matrix_rotate(&matrix, 1, 5632);
        matrix_rotate(&matrix, 2, 7936);
    }
    const ITEM_CORRECTION *correction = item_corrections + kind - DYNAMIC_ITEM_TYPE_FIRST;
    matrix_rotate(&matrix, 2, correction->roll);
    matrix_rotate(&matrix, 1, correction->yaw);
    matrix_rotate(&matrix, 0, correction->pitch);
    matrix_translate(&matrix, correction->x, correction->y, correction->z);
    collect_model(model_refs[kind], NULL, &matrix, color, 0, 0, 0);
    if (left || kind - 496u >= 2u)
        return;
    XASSERT(item->payload_kind == MODEL_PAYLOAD_PHYSICS);
    sint32 timer = item->physics.life;
    if (timer <= 0)
        return;
    uint32 group = (timer + 29) / 30;
    if (group < 2 && (group != 1 || !(game_session.tick & 2)))
        return;
    XASSERT(121u + group < TEXTURE_MAP_COUNT);
    VECTORF center = {matrix.t[0], matrix.t[1], matrix.t[2]};
    collect_fixed_sprite(collect_effect_center(center, center.vz >= 65 ? -64.0f : 0.0f), 0, 0, tex_map[121u + group]);
}

/* Culled candidates never publish lighting, attachment, equipment or trail writes */
void hd_collect_culled_actor(uint32 actor, sint32 type, int player, const POSE_MATRIX *source)
{
    if (!hd.collecting)
        return;
    XASSERT(actor < ACTOR_COUNT && type >= 0 && type < ACTOR_TYPE_COUNT);
    MATRIXF root = transform_slot(source)->matrix, matrix = root;
    const ACTOR_LIGHTING *light = actor_lighting + actor;
    uint32 color = light->flags & 2 ? 0x2e000000u : 0x2c000000u;
    uint32 blend = light->flags & 2 ? light->flags >> 4 : render_style.blend_mode;
    const sint8 channels[3] = {light->r, light->g, light->b};
    for (uint32 channel = 0; channel < 3; channel++)
    {
        sint32 value = 128 + channels[channel];
        if (light->flags & 4)
            value += player ? 96 : 128;
        color |= (uint32)(value > 255 ? 255 : value) << (channel * 8);
    }
    sint32 yaw = (sint16)(0u - ((uint32)(sint32)actor_orientation[actor].yaw << 5)) + 32768;
    if (!player && type == 51)
        matrix_rotate(&matrix, 1, 32768);
    matrix_rotate(&matrix, 1, yaw);
    const ACTOR_FRAME_MOTION *motion = &actor_animations[actor].motion;
    matrix_translate(&matrix, 0, (sint32)motion->bits[0] / 65536.0f, (sint32)motion->forward_offset_bits / 65536.0f);
    if (!player && type == 14 && (light->flags & 8))
    {
        hd_actor_part(actor, HD_ACTOR_GHOST1);
        collect_model(model_refs[type], &actor_poses[ACTOR_POSE_TRAIL1], &matrix, color | 0x02000000u, 4, 0, 0);
        hd_actor_part(actor, HD_ACTOR_GHOST0);
        collect_model(model_refs[type], &actor_poses[ACTOR_POSE_TRAIL0], &matrix, color | 0x02000000u, 1, 0, 0);
        blend = 0;
    }
    hd_actor_part(actor, HD_ACTOR_BODY);
    collect_model(model_refs[type], actor_poses + actor, &matrix, color, blend, 0, 0);
    hd_actor_part(actor, HD_ACTOR_RIGHT);
    actor_item(actor, 0, player, color);
    hd_actor_part(actor, HD_ACTOR_LEFT);
    actor_item(actor, 1, player, color);
    matrix = root;
    matrix_translate(&matrix, 0, (sint32)(0u - actor_positions[actor].y.bits - model_height[type]) / 65536.0f, 0);
    matrix_rotate(&matrix, 1, yaw);
    if (!player)
    {
        if (type == 28 || type == 51)
            matrix_rotate(&matrix, 1, 16384);
        sint32 scale = ((sint32)(actor_positions[actor].y.bits + motion->bits[0]) >> 12) + (type == 50 ? 24576 : 16384);
        if (scale <= 0)
        {
            hd_actor_part_end();
            return;
        }
        VECTORF extent = {scale / 16384.0f, scale / 16384.0f, scale / 16384.0f};
        ScaleMatrixf(&matrix, &extent);
    }
    hd_actor_part(actor, HD_ACTOR_SHADOW);
    collect_model(model_refs[860], NULL, &matrix, 0x2d000000u, 0, 0, 0);
    hd_actor_part_end();
}

void hd_render_phase(float fract)
{
    XASSERT(fract >= 0 && fract <= 1);
    hd.render_fract = fract;
}

void hd_render_begin(void)
{
    hd.render_fract = 1;
    hd.ordered_count = 0;
    hd.ui_buffer_cursor = 0;
    hd.trail_buffer_cursor = 0;
    hd_pose_begin();
}

static void face_indices(uint32 *indices, uint32 face)
{
    static const uint32 triangle[6] = {0, 1, 2, 1, 3, 2};
    for (uint32 corner = 0; corner < 6; corner++)
        indices[corner] = face * 4 + triangle[corner];
}

static VID_INDEX *quad_indices(uint32 count)
{
    if (hd.quad_capacity >= count)
        return hd.quad_indices;
    uint32 capacity = hd.quad_capacity ? hd.quad_capacity : 32;
    while (capacity < count)
    {
        XASSERT(capacity <= UINT32_MAX / 12);
        capacity *= 2;
    }
    VID_INDEX *indices = vid_index_create(capacity * 6);
    if (!indices)
        return NULL;
    uint32 *values = vid_index_data(indices);
    for (uint32 quad = 0; quad < capacity; quad++)
        face_indices(values + quad * 6, quad);
    vid_index_commit(indices, capacity * 6);
    vid_index_static(indices);
    /* queued groups retain the previous buffer if this frame needs a larger one */
    vid_index_free(hd.quad_indices);
    hd.quad_indices = indices;
    hd.quad_capacity = capacity;
    return indices;
}

static void draw_uniforms(const HD_FRAME *frame, const MATRIXF *matrix, uint32 color, uint32 bank, VID_RECT viewport, float *uniforms)
{
    uint32 row, col;
    float aspect = (float)viewport.w / viewport.h;
    memset(uniforms, 0, HD_DRAW_VECTORS * 4 * sizeof(*uniforms));
    for (row = 0; row < 3; row++)
    {
        for (col = 0; col < 3; col++)
            uniforms[row * 4 + col] = matrix->m[row][col];
        uniforms[row * 4 + 3] = matrix->t[row];
    }
    /* Preserve OG framing at 4:3 and expand only the horizontal field */
    float horizontal = (4.0f / 3.0f) / aspect;
    uniforms[12] = 2.0f * frame->focal / frame->width * horizontal;
    uniforms[13] = -2.0f * frame->focal / frame->height;
    uniforms[14] = (2.0f * (frame->center_x + (hd.presenting ? hd.display_offset[0] : frame->offset_x)) / frame->width - 1.0f) * horizontal;
    uniforms[15] = 1.0f - 2.0f * (frame->center_y + (hd.presenting ? hd.display_offset[1] : frame->offset_y)) / frame->height;
    uniforms[16] = (color & 255) / 255.0f;
    uniforms[17] = ((color >> 8) & 255) / 255.0f;
    uniforms[18] = ((color >> 16) & 255) / 255.0f;
    uniforms[19] = 1;
    uniforms[20] = 65537.0f / 65535.0f;
    uniforms[21] = -131072.0f / 65535.0f;
    uniforms[22] = (color & 0x01000000u) != 0;
    uniforms[27] = (float)(bank * TEXTURE_DESCRIPTOR_CAPACITY * 3);
    uniforms[28] = (float)viewport.w;
    uniforms[29] = (float)viewport.h;
    uniforms[30] = (float)viewport.h / frame->height;
    uniforms[31] = (float)viewport.h * (4.0f / 3.0f) / frame->width;
}

static int visible_model(const HD_FRAME *frame, const HD_MODEL_DRAW *draw, const float *uniforms)
{
    uint32 outside = 63;
    /* Include the native line endpoint expansion in the viewport test */
    float margin_x = 4.0f / frame->width * (4.0f / 3.0f) * uniforms[29] / uniforms[28];
    float margin_y = 4.0f / frame->height;
    for (uint32 corner = 0; corner < 8; corner++)
    {
        float x = corner & 1 ? draw->bounds.max[0] : draw->bounds.min[0];
        float y = corner & 2 ? draw->bounds.max[1] : draw->bounds.min[1];
        float z = corner & 4 ? draw->bounds.max[2] : draw->bounds.min[2];
        float clip_x = x * uniforms[12] + z * uniforms[14];
        float clip_y = y * uniforms[13] + z * uniforms[15];
        float edge_x = z * (1 + margin_x), edge_y = z * (1 + margin_y);
        uint32 mask = (clip_x < -edge_x) | ((clip_x > edge_x) << 1) | ((clip_y < -edge_y) << 2) | ((clip_y > edge_y) << 3) | ((z < 1) << 4) | ((z > 65536) << 5);
        outside &= mask;
    }
    return !outside;
}

static uint32 ordered_bucket(const HD_RIG_PART *parts, const HD_MODEL_DRAW *draw, const HD_MODEL *model, uint32 face)
{
    const HD_FACE_RANGE *range = model->faces + face;
    const VID_VERTEX16 *source = model->vertices + face * 4;
    uint32 corners[2] = {range->flags & 2 ? 1u : 0u, range->flags & 2 ? 2u : 3u};
    int line = range->descriptor == UINT16_MAX;
    if (line)
    {
        corners[0] = 0;
        corners[1] = 1;
    }
    float depth = 0;
    /* only blend ordering needs a CPU depth key; visibility and shading belong to GPU */
    for (uint32 end = 0; end < 2; end++)
    {
        const VID_VERTEX16 *vertex = source + corners[end];
        VECTORF point = {vertex->pos[0] + vertex->extra[0] * vertex->extra[3], vertex->pos[1] + vertex->extra[1] * vertex->extra[3], vertex->pos[2] + vertex->extra[2] * vertex->extra[3]};
        if (draw->part_count)
        {
            uint32 part = (uint16)vertex->n[3];
            XASSERT(part < draw->part_count);
            RotTransf(&parts[draw->first_part + part].matrix, &point, &point);
        }
        float z = draw->matrix.m[2][0] * point.vx + draw->matrix.m[2][1] * point.vy + draw->matrix.m[2][2] * point.vz;
        z += draw->matrix.t[2];
        depth += z;
    }
    float index = (depth + (line ? 0 : draw->bias)) * 0.125f;
    uint32 clamped = index <= 0 ? 0 : index >= DEPTH_TABLE_COUNT ? DEPTH_TABLE_COUNT - 1 : (uint32)index;
    sint32 offset = depth_table[clamped];
    XASSERT(offset >= 0 && offset < 8192 && !(offset & 3));
    return (uint32)offset >> 2;
}

static int submit_face_range(const HD_MODEL *model, VID_INDEX *indices, uint32 first, uint32 count, const float *uniforms, HD_BLEND blend, int layer, uint32 stp)
{
    VID_MATERIAL *material = material_for(blend, layer, stp);
    if (!material)
        return 0;
    vid_uniforms(uniforms, HD_DRAW_VECTORS);
    vid_pass(material);
    return vid_submit(model->mesh, indices, first, count);
}

static int submit_faces(const HD_MODEL *model, VID_INDEX *indices, uint32 count, const float *uniforms, HD_BLEND blend, int layer, uint32 stp)
{
    return submit_face_range(model, indices, 0, count, uniforms, blend, layer, stp);
}

static int ui_draw_selected(VID_RECT viewport, HD_UI_LAYER layer, uint32 selected, uint32 selected_count);

static int matrix_sample(const MATRIXF *matrix, HD_MATRIX_SAMPLE *result)
{
    uint32 hash = 2166136261u;
    for (uint32 row = 0; row < 3; row++)
        for (uint32 col = 0; col < 3; col++)
        {
            uint32 bits;
            memcpy(&bits, &matrix->m[row][col], sizeof(bits));
            hash = (hash ^ bits) * 16777619u;
        }
    HD_MATRIX_SAMPLE *sample = hd.matrix_samples + (hash & 255u);
    if (!sample->status || memcmp(sample->basis, matrix->m, sizeof(sample->basis)))
    {
        memcpy(sample->basis, matrix->m, sizeof(sample->basis));
        sample->status = 2;
        MATRIXF rotation;
        IdentityMatrixf(&rotation);
        for (uint32 col = 0; col < 3; col++)
        {
            VECTORF axis = {matrix->m[0][col], matrix->m[1][col], matrix->m[2][col]};
            sample->scale[col] = NormalizeVectorf(&axis, &axis);
            if (sample->scale[col] <= 0)
                return 0;
            for (uint32 row = 0; row < 3; row++)
                rotation.m[row][col] = row == 0 ? axis.vx : row == 1 ? axis.vy : axis.vz;
        }
        /* sheared or mirrored transforms retain the original interpolation fallback */
        VECTORF a = {rotation.m[0][0], rotation.m[1][0], rotation.m[2][0]};
        VECTORF b = {rotation.m[0][1], rotation.m[1][1], rotation.m[2][1]}, cross;
        OuterProductf(&a, &b, &cross);
        float determinant = cross.vx * rotation.m[0][2] + cross.vy * rotation.m[1][2] + cross.vz * rotation.m[2][2];
        if (determinant < 0.999f || determinant > 1.001f)
            return 0;
        MatrixQuatf(&rotation, &sample->rotation);
        sample->status = 1;
    }
    if (sample->status != 1)
        return 0;
    /* copy the sample so a colliding lookup cannot overwrite the first endpoint */
    *result = *sample;
    return 1;
}

static int matrix_blend(const MATRIXF *previous, const MATRIXF *current, float fract, MATRIXF *result)
{
    if (fract == 0 || fract == 1)
    {
        *result = fract == 0 ? *previous : *current;
        return 1;
    }
    HD_MATRIX_SAMPLE a, b;
    if (!matrix_sample(previous, &a) || !matrix_sample(current, &b))
        return 0;
    QUATF mixed;
    SlerpQuatf(&a.rotation, &b.rotation, fract, &mixed);
    QuatMatrixf(&mixed, result);
    float size[3];
    for (uint32 axis = 0; axis < 3; axis++)
    {
        size[axis] = a.scale[axis] + (b.scale[axis] - a.scale[axis]) * fract;
        result->t[axis] = previous->t[axis] + (current->t[axis] - previous->t[axis]) * fract;
    }
    VECTORF extent = {size[0], size[1], size[2]};
    ScaleMatrixf(result, &extent);
    return 1;
}

static int camera_view_blend(const MATRIXF *previous, const MATRIXF *current, MATRIXF *view)
{
    if (hd.camera_blend.status && hd.camera_blend.phase == hd.render_fract && !memcmp(previous, &hd.camera_blend.previous, sizeof(*previous)) && !memcmp(current, &hd.camera_blend.current, sizeof(*current)))
    {
        if (hd.camera_blend.status != 1)
            return 0;
        *view = hd.camera_blend.view;
        return 1;
    }
    hd.camera_blend.previous = *previous;
    hd.camera_blend.current = *current;
    hd.camera_blend.phase = hd.render_fract;
    hd.camera_blend.status = 2;
    MATRIXF camera;
    if (!matrix_blend(previous, current, hd.render_fract, &camera))
        return 0;
    TransposeMatrixf(&camera, view);
    VECTORF position = {-camera.t[0], -camera.t[1], -camera.t[2]};
    ApplyMatrixf(view, &position, &position);
    TransMatrixf(view, &position);
    hd.camera_blend.view = *view;
    hd.camera_blend.status = 1;
    return 1;
}

static int model_matrix_blend(const HD_MODEL_DRAW *previous, const HD_MODEL_DRAW *current, MATRIXF *result)
{
    if (hd.render_fract == 0 || hd.render_fract == 1)
    {
        *result = hd.render_fract == 0 ? previous->matrix : current->matrix;
        return 1;
    }
    MATRIXF a, b, world, view;
    CompMatrixf(&previous->camera, &previous->matrix, &a);
    CompMatrixf(&current->camera, &current->matrix, &b);
    if (!matrix_blend(&a, &b, hd.render_fract, &world) || !camera_view_blend(&previous->camera, &current->camera, &view))
        return 0;
    CompMatrixf(&view, &world, result);
    return 1;
}

static int seed_camera_view(const HD_FRAME *frame, const MATRIXF *camera, MATRIXF *view)
{
    if (hd.render_fract == 1 || frame->previous >= HD_FRAME_COUNT)
        return 0;
    const HD_FRAME *previous = hd.frame + frame->previous;
    return previous->valid && previous->sequence + 1 == frame->sequence && camera_view_blend(&previous->camera, camera, view);
}

static int model_camera_seed(const HD_FRAME *frame, const HD_MODEL_DRAW *current, HD_MODEL_DRAW *draw)
{
    MATRIXF view, world;
    if (!seed_camera_view(frame, &current->camera, &view))
        return 0;
    CompMatrixf(&current->camera, &current->matrix, &world);
    CompMatrixf(&view, &world, &draw->matrix);
    return 1;
}

static int ui_camera_seed(const HD_FRAME *frame, const HD_UI_DRAW *current, HD_UI_DRAW *draw)
{
    MATRIXF view;
    if (!(current->flags & (HD_TEX_BILLBOARD | HD_TEX_RIBBON)) || !seed_camera_view(frame, &current->camera, &view))
        return 0;
    for (uint32 point = 0; point < ((current->flags & HD_TEX_RIBBON) ? 2u : 1u); point++)
    {
        VECTORF world;
        RotTransf(&current->camera, current->effect.point + point, &world);
        RotTransf(&view, &world, draw->effect.point + point);
    }
    return 1;
}

int hd_terrain_draw(VID_RECT viewport)
{
    HD_FRAME *frame = hd.frame + hd.last_frame;
    uint32 index;
    if (!frame->valid || (frame->missing & HD_MISSING_TERRAIN_BLEND) || viewport.w <= 0 || viewport.h <= 0 || !hd_vram_sync())
        return 0;
    if (!prepare_descriptors(frame))
        return 0;
    vid_viewport(viewport);
    for (index = frame->terrain_count; index > 0; index--)
    {
        const HD_TERRAIN_DRAW *draw = frame->terrain + index - 1;
        HD_TERRAIN_DRAW blended;
        if (hd.render_fract != 1 && frame->previous < HD_FRAME_COUNT)
        {
            const HD_FRAME *previous = hd.frame + frame->previous;
            MATRIXF view, world;
            if (previous->valid && previous->sequence + 1 == frame->sequence && previous->terrain_count && camera_view_blend(&previous->terrain[0].camera, &draw->camera, &view))
            {
                blended = *draw;
                CompMatrixf(&draw->camera, &draw->matrix, &world);
                CompMatrixf(&view, &world, &blended.matrix);
                draw = &blended;
            }
        }
        float uniforms[HD_DRAW_VECTORS * 4];
        VID_INDEX *indices = quad_indices(256);
        if (!indices)
            return 0;
        uint32 layer = frame->stage == 18 ? 2 : 0;
        HD_TERRAIN_MESH *map = hd.terrain_meshes + draw->tile_map;
        if (!map->material[layer])
            map->material[layer] = create_material(HD_BLEND_OPAQUE, layer, 0, 0, map->image);
        if (!map->material[layer])
            return 0;
        draw_uniforms(frame, &draw->matrix, draw->color, draw->descriptor_bank, viewport, uniforms);
        uniforms[19] = 0;
        uniforms[26] = draw->faded ? HD_PASS_FADED : 0;
        uniforms[28] = (float)draw->column;
        uniforms[29] = (float)draw->row;
        /* scrolling water stays behind the hovercraft's independent scene */
        vid_uniforms(uniforms, HD_DRAW_VECTORS);
        vid_pass(map->material[layer]);
        if (!vid_submit(draw->mesh, indices, 0, 256 * 6))
            return 0;
    }
    hd_pose_commit();
    return 1;
}

static int hand_blend(const HD_FRAME *previous, const HD_FRAME *frame, const HD_MODEL_DRAW *current, MATRIXF *matrix)
{
    uint32 actor = current->actor, hand = current->actor_part == HD_ACTOR_LEFT ? 0 : 1;
    const HD_ACTOR_HAND *a = &previous->actor_hand[actor][hand], *b = &frame->actor_hand[actor][hand];
    uint32 old_index = previous->actor_model[actor][HD_ACTOR_BODY], new_index = frame->actor_model[actor][HD_ACTOR_BODY];
    if (!b->valid || b->slot >= DYNAMIC_OBJECT_COUNT || old_index >= previous->model_count || new_index >= frame->model_count)
        return 0;
    const HD_MODEL_DRAW *old_body = previous->models + old_index, *new_body = frame->models + new_index;
    if (old_body->type != new_body->type || old_body->part_count != new_body->part_count || b->part >= old_body->part_count || previous->actor_attachment[actor][hand] != b->part)
        return 0;
    uint32 old_hand = previous->actor_model[actor][current->actor_part];
    int continuous = a->valid && a->slot == b->slot && a->part == b->part && old_hand < previous->model_count && previous->models[old_hand].type == current->type;
    MATRIXF root, bone, correction, anchor;
    if (!model_matrix_blend(old_body, new_body, &root) || !matrix_blend(&previous->parts[old_body->first_part + b->part].matrix, &frame->parts[new_body->first_part + b->part].matrix, hd.render_fract, &bone) || !matrix_blend(continuous ? &a->correction : &b->correction, &b->correction, hd.render_fract, &correction))
        return 0;
    CompMatrixf(&root, &bone, &anchor);
    CompMatrixf(&anchor, &correction, matrix);
    if (continuous && hd.render_fract == 0)
        *matrix = previous->models[old_hand].matrix;
    return 1;
}

static int parts_blend(const HD_FRAME *previous, const HD_MODEL_DRAW *source, const HD_FRAME *frame, const HD_MODEL_DRAW *current, HD_MODEL_DRAW *draw)
{
    if (!reserve_frame((void **)&hd.blend_parts, &hd.blend_capacity, current->part_count, sizeof(*hd.blend_parts)))
        return 0;
    for (uint32 part = 0; part < current->part_count; part++)
    {
        const HD_RIG_PART *a = previous->parts + source->first_part + part, *b = frame->parts + current->first_part + part;
        if (a->first_vertex != b->first_vertex || a->vertex_count != b->vertex_count)
            return 0;
        hd.blend_parts[part] = *b;
        if (!matrix_blend(&a->matrix, &b->matrix, hd.render_fract, &hd.blend_parts[part].matrix))
            return 0;
    }
    draw->first_part = 0;
    return 1;
}

static int actor_blend(const HD_FRAME *frame, const HD_MODEL_DRAW *current, HD_MODEL_DRAW *draw)
{
    if (hd.render_fract == 1 || current->actor >= ACTOR_COUNT || frame->previous >= HD_FRAME_COUNT)
        return 0;
    const HD_FRAME *previous = hd.frame + frame->previous;
    if (!previous->valid || previous->sequence + 1 != frame->sequence)
        return 0;
    if (current->actor_part == HD_ACTOR_RIGHT || current->actor_part == HD_ACTOR_LEFT)
    {
        if (current->part_count)
            return 0;
        draw->first_part = 0;
        return hand_blend(previous, frame, current, &draw->matrix);
    }
    uint32 index = previous->actor_model[current->actor][current->actor_part];
    if (index >= previous->model_count)
        return 0;
    const HD_MODEL_DRAW *source = previous->models + index;
    if (source->type != current->type || source->part_count != current->part_count || !model_matrix_blend(source, current, &draw->matrix))
        return 0;
    return parts_blend(previous, source, frame, current, draw);
}

static const HD_FRAME *object_previous(const HD_FRAME *frame, uint32 owner)
{
    if (owner >= HD_OBJECT_COUNT || frame->previous >= HD_FRAME_COUNT)
        return NULL;
    const HD_FRAME *previous = hd.frame + frame->previous;
    return previous->valid && previous->sequence + 1 == frame->sequence && previous->objects[owner].kind == frame->objects[owner].kind ? previous : NULL;
}

static void match_previous_records(HD_FRAME *frame)
{
    for (uint32 item = 0; item < frame->model_count; item++)
    {
        HD_MODEL_DRAW *draw = frame->models + item;
        draw->previous_record = UINT32_MAX;
        const HD_FRAME *previous = object_previous(frame, draw->object_owner);
        if (!previous || draw->mutable_model != UINT32_MAX)
            continue;
        const HD_OBJECT_SAMPLE *a = previous->objects + draw->object_owner, *b = frame->objects + draw->object_owner;
        uint32 matches = 0, current_matches = 0, index = a->model_head, source = UINT32_MAX;
        for (uint32 n = 0; n < a->model_count; n++)
        {
            XASSERT(index < previous->model_count);
            const HD_MODEL_DRAW *candidate = previous->models + index;
            if (candidate->type == draw->type && candidate->object_part == draw->object_part)
            {
                source = index;
                matches++;
            }
            index = candidate->object_next;
        }
        index = b->model_head;
        for (uint32 n = 0; n < b->model_count; n++)
        {
            XASSERT(index < frame->model_count);
            const HD_MODEL_DRAW *candidate = frame->models + index;
            current_matches += candidate->type == draw->type && candidate->object_part == draw->object_part;
            index = candidate->object_next;
        }
        /* repeated same-kind parts require an explicit semantic role */
        if (matches == 1 && current_matches == 1)
            draw->previous_record = source;
    }
    for (uint32 item = 0; item < frame->ui_count; item++)
    {
        HD_UI_DRAW *draw = frame->ui + item;
        draw->previous_record = UINT32_MAX;
        const HD_FRAME *previous = object_previous(frame, draw->object_owner);
        if (!previous || !(draw->flags & (HD_TEX_BILLBOARD | HD_TEX_RIBBON)))
            continue;
        const HD_OBJECT_SAMPLE *a = previous->objects + draw->object_owner, *b = frame->objects + draw->object_owner;
        uint32 matches = 0, current_matches = 0, index = a->ui_head, source = UINT32_MAX;
        for (uint32 n = 0; n < a->ui_count; n++)
        {
            XASSERT(index < previous->ui_count);
            const HD_UI_DRAW *candidate = previous->ui + index;
            if (candidate->object_part == draw->object_part)
            {
                source = index;
                matches++;
            }
            index = candidate->object_next;
        }
        index = b->ui_head;
        for (uint32 n = 0; n < b->ui_count; n++)
        {
            XASSERT(index < frame->ui_count);
            const HD_UI_DRAW *candidate = frame->ui + index;
            current_matches += candidate->object_part == draw->object_part;
            index = candidate->object_next;
        }
        if (matches == 1 && current_matches == 1)
            draw->previous_record = source;
    }
}

static void match_background_records(HD_FRAME *frame)
{
    if (frame->previous >= HD_FRAME_COUNT)
        return;
    const HD_FRAME *previous = hd.frame + frame->previous;
    if (!previous->valid || previous->sequence + 1 != frame->sequence)
        return;
    for (uint32 item = 0; item < frame->ui_count; item++)
    {
        HD_UI_DRAW *draw = frame->ui + item;
        if (draw->background_part == HD_BACKGROUND_NONE && !(draw->flags & (HD_TEX_SKY | HD_TEX_DISTANT_GROUND)))
            continue;
        uint32 matches = 0, source = UINT32_MAX;
        for (uint32 index = 0; index < previous->ui_count; index++)
        {
            const HD_UI_DRAW *candidate = previous->ui + index;
            int match;
            if (draw->flags & HD_TEX_SKY)
            {
                int single_bank = (uint8)draw->sky_v == (uint8)(draw->sky_v >> 8);
                match = (candidate->flags & HD_TEX_SKY) && candidate->region[1] == draw->region[1] && (single_bank ? candidate->sky_v == draw->sky_v : (uint8)candidate->sky_v != (uint8)(candidate->sky_v >> 8));
            }
            else if (draw->flags & HD_TEX_DISTANT_GROUND)
                match = (candidate->flags & HD_TEX_DISTANT_GROUND) != 0;
            else
                match = candidate->background_part == draw->background_part;
            if (match)
            {
                matches++;
                source = index;
            }
        }
        /* ground retains the original last matching strip; other roles must be unique */
        draw->previous_record = matches == 1 || (draw->flags & HD_TEX_DISTANT_GROUND) ? source : UINT32_MAX;
    }
}

static int object_model_blend(const HD_FRAME *frame, const HD_MODEL_DRAW *current, HD_MODEL_DRAW *draw)
{
    const HD_FRAME *previous = object_previous(frame, current->object_owner);
    if (!previous || current->mutable_model != UINT32_MAX)
        return 0;
    if (current->previous_record >= previous->model_count)
        return 0;
    const HD_MODEL_DRAW *source = previous->models + current->previous_record;
    if (source->part_count != current->part_count || !model_matrix_blend(source, current, &draw->matrix))
        return 0;
    return parts_blend(previous, source, frame, current, draw);
}

static int object_ui_blend(const HD_FRAME *frame, const HD_UI_DRAW *current, HD_UI_DRAW *draw)
{
    const HD_FRAME *previous = object_previous(frame, current->object_owner);
    if (!previous || !(current->flags & (HD_TEX_BILLBOARD | HD_TEX_RIBBON)))
        return 0;
    if (current->previous_record >= previous->ui_count)
        return 0;
    const HD_UI_DRAW *source = previous->ui + current->previous_record;
    if (source->flags != current->flags || source->effect.fixed != current->effect.fixed)
        return 0;
    MATRIXF view;
    if (!camera_view_blend(&source->camera, &current->camera, &view))
        return 0;
    for (uint32 point = 0; point < ((current->flags & HD_TEX_RIBBON) ? 2u : 1u); point++)
    {
        VECTORF a, b, world;
        RotTransf(&source->camera, source->effect.point + point, &a);
        RotTransf(&current->camera, current->effect.point + point, &b);
        world = (VECTORF){a.vx + (b.vx - a.vx) * hd.render_fract, a.vy + (b.vy - a.vy) * hd.render_fract, a.vz + (b.vz - a.vz) * hd.render_fract};
        if (hd.render_fract == 0)
            draw->effect.point[point] = source->effect.point[point];
        else
            RotTransf(&view, &world, draw->effect.point + point);
        draw->effect.size[point] = source->effect.size[point] + (current->effect.size[point] - source->effect.size[point]) * hd.render_fract;
    }
    for (uint32 axis = 0; axis < 2; axis++)
        draw->effect.offset[axis] = source->effect.offset[axis] + (current->effect.offset[axis] - source->effect.offset[axis]) * hd.render_fract;
    draw->effect.depth_bias = source->effect.depth_bias + (current->effect.depth_bias - source->effect.depth_bias) * hd.render_fract;
    return 1;
}

static int hand_ui_blend(const HD_FRAME *frame, const HD_UI_DRAW *current, HD_UI_DRAW *draw)
{
    uint32 index = frame->actor_model[current->hand_actor][HD_ACTOR_RIGHT];
    if (index >= frame->model_count)
        return 0;
    HD_MODEL_DRAW hand = frame->models[index];
    if (!actor_blend(frame, &frame->models[index], &hand))
        return 0;
    /* held countdowns follow the item's corrected bone anchor */
    draw->effect.point[0] = (VECTORF){hand.matrix.t[0], hand.matrix.t[1], hand.matrix.t[2]};
    draw->effect.depth_bias = hand.matrix.t[2] >= 65 ? -64.0f : 0.0f;
    return 1;
}

static const HD_MODEL *trail_blend(const HD_FRAME *frame, const HD_MODEL_DRAW *current, HD_MODEL_DRAW *draw)
{
    if (current->mutable_model == UINT32_MAX || frame->previous >= HD_FRAME_COUNT)
        return NULL;
    const HD_FRAME *previous = hd.frame + frame->previous;
    uint32 index = previous->trail_model[current->type - 867];
    if (!previous->valid || previous->sequence + 1 != frame->sequence || index >= previous->model_count)
        return NULL;
    const HD_MODEL_DRAW *old = previous->models + index;
    if (old->type != current->type || old->mutable_model == UINT32_MAX)
        return NULL;
    const HD_MUTABLE_MODEL *a = previous->mutable_models + old->mutable_model, *b = frame->mutable_models + current->mutable_model;
    uint32 appended = b->appends - a->appends;
    if (appended > 1 || !model_matrix_blend(old, current, &draw->matrix))
        return NULL;
    if (!reserve_frame((void **)&hd.trail_buffers, &hd.trail_buffer_capacity, hd.trail_buffer_cursor + 1, sizeof(*hd.trail_buffers)))
        return NULL;
    if (hd.trail_buffer_cursor == hd.trail_buffer_count)
    {
        HD_TRAIL_BUFFER *created = calloc(1, sizeof(*created));
        if (!created)
            return NULL;
        hd.trail_buffers[hd.trail_buffer_count++] = created;
    }
    HD_TRAIL_BUFFER *slot = hd.trail_buffers[hd.trail_buffer_cursor++];
    slot->type = current->type;
    uint32 sections = appended ? 10 : 9, face_count = sections - 1;
    slot->header = (MODEL_BODY_HEADER){0};
    slot->header.vertex_count = (sint16)(sections * 2);
    slot->header.face_count = (sint16)face_count;
    VECTORF points[20];
    HD_BOUNDS bounds;
    for (uint32 section = 0; section < sections; section++)
        for (uint32 side = 0; side < 2; side++)
        {
            /* the old tail contracts to its successor while the new head grows */
            uint32 from = section < 9 ? section : 8;
            uint32 to = appended && section ? section - 1 : section;
            const SVECTOR *p = a->points + from * 2 + side, *q = b->points + to * 2 + side;
            uint32 vertex = section * 2 + side;
            slot->points[vertex] = *p;
            points[vertex] = (VECTORF){p->vx + (q->vx - p->vx) * hd.render_fract, p->vy + (q->vy - p->vy) * hd.render_fract, p->vz + (q->vz - p->vz) * hd.render_fract};
            const float value[3] = {points[vertex].vx, points[vertex].vy, points[vertex].vz};
            for (uint32 axis = 0; axis < 3; axis++)
            {
                if (!vertex || value[axis] < bounds.min[axis])
                    bounds.min[axis] = value[axis];
                if (!vertex || value[axis] > bounds.max[axis])
                    bounds.max[axis] = value[axis];
            }
        }
    for (uint32 face = 0; face < face_count; face++)
    {
        uint32 source = appended && face ? face - 1 : face;
        slot->faces[face] = b->model.source.faces[source];
        const uint16 vertices[4] = {(uint16)((face + 1) * 8), (uint16)((face + 1) * 8 + 4), (uint16)(face * 8 + 4), (uint16)(face * 8)};
        memcpy(slot->faces[face].vertex_offset, vertices, sizeof(vertices));
    }
    MODEL_REFERENCE source = {&slot->header, sizeof(slot->header), slot->points, sections * 2 * sizeof(*slot->points), slot->faces, face_count * sizeof(*slot->faces)};
    HD_MODEL *model = &slot->model;
    if (!model->mesh || model->face_count != face_count)
    {
        if (!model_build(model, source, 0))
            return NULL;
    }
    /* retain reference vertices and write the final float displacement only once */
    VID_VERTEX16 *vertices = (VID_VERTEX16 *)model->vertices;
    static const uint32 order[2][4] = {{0, 1, 3, 2}, {1, 0, 2, 3}};
    for (uint32 face = 0; face < face_count; face++)
    {
        const MODEL_FACE_RECORD *record = slot->faces + face;
        model->faces[face].flags = record->flags;
        model->faces[face].descriptor = record->tex_offset / sizeof(TEXTURE_DESCRIPTOR);
        for (uint32 corner = 0; corner < 4; corner++)
        {
            VID_VERTEX16 *vertex = vertices + face * 4 + corner;
            const VECTORF *point = points + record->vertex_offset[order[(record->flags >> 1) & 1][corner]] / 4;
            vertex->extra[0] = point->vx - vertex->pos[0];
            vertex->extra[1] = point->vy - vertex->pos[1];
            vertex->extra[2] = point->vz - vertex->pos[2];
            vertex->extra[3] = 1;
            vertex->tex[0] = model->faces[face].descriptor;
        }
    }
    model->bounds[0] = bounds;
    if (!model_gpu_update(model))
        return NULL;
    draw_bounds(frame->parts, draw, model);
    return model;
}

static int models_draw(VID_RECT viewport, int effects)
{
    HD_FRAME *frame = hd.frame + hd.last_frame;
    uint32 index;
    if (!frame->valid || frame->missing || viewport.w <= 0 || viewport.h <= 0)
        return 0;
    if (!hd_vram_sync() || !reserve_frame((void **)&hd.render_model, &hd.render_capacity, frame->model_count, sizeof(*hd.render_model)))
        return 0;
    if (!prepare_descriptors(frame))
        return 0;
    hd.ordered_count = 0;
    memset(hd.order_head, 255, sizeof(hd.order_head));
    vid_viewport(viewport);
    for (index = 0; index < frame->model_count; index++)
    {
        const HD_MODEL_DRAW *draw = frame->models + index;
        HD_MODEL_DRAW blended;
        int interpolated = 0;
        const HD_RIG_PART *parts = frame->parts;
        const HD_MODEL *model = draw_model(frame, draw);
        if (hd.render_fract != 1 && draw->mutable_model != UINT32_MAX)
        {
            blended = *draw;
            const HD_MODEL *trail = trail_blend(frame, draw, &blended);
            if (trail)
            {
                model = trail;
                draw = &blended;
                interpolated = 1;
            }
        }
        if (!interpolated && hd.render_fract != 1 && (draw->actor < ACTOR_COUNT || draw->object_owner < HD_OBJECT_COUNT))
        {
            blended = *draw;
            if (draw->actor < ACTOR_COUNT ? actor_blend(frame, draw, &blended) : object_model_blend(frame, draw, &blended))
            {
                parts = hd.blend_parts;
                draw = &blended;
                interpolated = 1;
                draw_bounds(parts, &blended, model);
            }
        }
        if (!interpolated && hd.render_fract != 1)
        {
            blended = *draw;
            if (model_camera_seed(frame, draw, &blended))
            {
                draw = &blended;
                draw_bounds(parts, &blended, model);
            }
        }
        const HD_DESCRIPTOR_BANK *bank = frame->banks + draw->descriptor_bank;
        /* the projected beam is CARLIGHT; VAN2SHAD contains only the van shadow */
        int decal = model_is_decal(draw->type);
        /* fallen gate77 shares its mesh with upright gate75 */
        int gate_fragment = draw->type == 72 || draw->type == 73 || draw->type == 77 || (draw->object_owner < HD_OBJECT_COUNT && frame->objects[draw->object_owner].kind == 77);
        if (frame->stage == 0 && draw->object_owner < HD_OBJECT_COUNT && frame->objects[draw->object_owner].kind == 77 && !model->rig)
        {
            MATRIXF camera = draw->camera, view, world;
            if (seed_camera_view(frame, &draw->camera, &view))
            {
                TransposeMatrixf(&view, &camera);
                VECTORF position = {-view.t[0], -view.t[1], -view.t[2]};
                ApplyMatrixf(&camera, &position, &position);
                TransMatrixf(&camera, &position);
            }
            CompMatrixf(&camera, &draw->matrix, &world);
            float penetration = 0;
            for (sint32 vertex = 0; vertex < model->source.header->vertex_count; vertex++)
            {
                const SVECTOR *point = model->source.vertices + vertex;
                float y = world.m[1][0] * point->vx + world.m[1][1] * point->vy + world.m[1][2] * point->vz + world.t[1];
                if (y > penetration)
                    penetration = y;
            }
            if (penetration > 0)
            {
                /* the original fallen mesh extends below the stage0 ground plane */
                blended = *draw;
                for (uint32 axis = 0; axis < 3; axis++)
                    blended.matrix.t[axis] -= (penetration + 0.5f) * camera.m[1][axis];
                draw = &blended;
                draw_bounds(parts, &blended, model);
            }
        }
        float *uniforms = hd.render_model[index].uniforms;
        hd.render_model[index].model = model;
        uint32 face;
        hd.render_model[index].opaque_count = 0;
        draw_uniforms(frame, &draw->matrix, draw->color, draw->descriptor_bank, viewport, uniforms);
        if (!visible_model(frame, draw, uniforms))
            continue;
        uniforms[19] = (float)draw->bias;
        uniforms[26] = draw->faded ? HD_PASS_FADED : 0;
        uniforms[24] = !draw->faded && (draw->color & 0x02000000u);
        /* half a view unit plus four depth steps; gate fragments retain depth writes */
        if (decal || gate_fragment)
            uniforms[24] += 2;
        /* trails need two view units; other decals retain half a unit */
        if (draw->type == 867 || draw->type == 868)
            uniforms[24] += 4;
        uniforms[25] = draw->faded || !draw->blend ? 0 : (float)(((draw->blend - 1u) & 2047u) + 1u);
        if (draw->part_count)
        {
            uint32 base = hd_pose_upload(parts + draw->first_part, draw->part_count);
            if (base == UINT32_MAX)
                return 0;
            uniforms[23] = (float)base;
        }
        uint32 count = draw->pass_count == UINT32_MAX ? model->face_count : draw->pass_count;
        uint32 ordered_faces = 0;
        for (face = 0; face < count; face++)
        {
            HD_PASS_FACE pass;
            if (draw->pass_count == UINT32_MAX)
            {
                if (!ordered_face(draw, model, bank, face, &pass))
                    continue;
            }
            else
                pass = frame->pass_faces[draw->pass_first + face];
            if (!reserve_frame((void **)&hd.ordered, &hd.ordered_capacity, hd.ordered_count + 1, sizeof(*hd.ordered)))
                return 0;
            ordered_faces++;
            uint32 bucket = ordered_bucket(parts, draw, model, pass.face);
            hd.ordered[hd.ordered_count] = (HD_ORDERED_FACE){index, pass.face, bucket, hd.order_head[bucket], pass.blend, pass.semi};
            hd.order_head[bucket] = hd.ordered_count++;
        }
        hd.render_model[index].opaque = model->indices;
        hd.render_model[index].opaque_count = decal || ordered_faces == model->face_count ? 0 : model->index_count;
    }
    for (index = frame->model_count; index > 0; index--)
    {
        const HD_RENDER_MODEL *render = hd.render_model + index - 1;
        if (render->opaque_count && !submit_faces(render->model, render->opaque, render->opaque_count, render->uniforms, HD_BLEND_OPAQUE, 0, 0))
            return 0;
    }
    uint32 effect_count = 0;
    if (effects)
        for (uint32 item = 0; item < frame->ui_count; item++)
            effect_count += frame->ui[item].layer == HD_UI_EFFECT;
    if (!reserve_frame((void **)&hd.traversal, &hd.traversal_capacity, hd.ordered_count + effect_count, sizeof(*hd.traversal)))
        return 0;
    uint32 selected = 0;
    /* Merge source insertion ordinals within each OT bucket, without a geometric sort */
    for (uint32 bucket = 0; bucket < RENDER_OT_COUNT; bucket++)
    {
        uint32 item = hd.order_head[bucket];
        uint32 effect = effects ? frame->ui_head[HD_UI_EFFECT][bucket] : UINT32_MAX;
        while (item != UINT32_MAX || effect != UINT32_MAX)
        {
            if (effect != UINT32_MAX && (item == UINT32_MAX || frame->ui[effect].ordinal > frame->models[hd.ordered[item].model].ordinal))
            {
                hd.traversal[selected++] = (HD_ORDERED_FACE){UINT32_MAX, effect, bucket, UINT32_MAX, HD_BLEND_OPAQUE, 0};
                effect = frame->ui[effect].next;
            }
            else
            {
                hd.traversal[selected++] = hd.ordered[item];
                item = hd.ordered[item].next;
            }
        }
    }
    XASSERT(selected == hd.ordered_count + effect_count);
    uint32 ordered_count = selected;
    for (index = 0; index < ordered_count;)
    {
        const HD_ORDERED_FACE *first = hd.traversal + index;
        if (first->model == UINT32_MAX)
        {
            uint32 end = index + 1;
            while (end < ordered_count && hd.traversal[end].model == UINT32_MAX)
                end++;
            if (!ui_draw_selected(viewport, HD_UI_EFFECT, index, end - index))
                return 0;
            index = end;
            continue;
        }
        const HD_MODEL *model = hd.render_model[first->model].model;
        uint32 first_index = model->faces[first->face].first;
        uint32 end = index + 1;
        while (end < ordered_count && hd.traversal[end].model == first->model && hd.traversal[end].blend == first->blend && hd.traversal[end].semi == first->semi && model->faces[hd.traversal[end].face].first == first_index + (end - index) * 6)
            end++;
        uint32 count = (end - index) * 6;
        float uniforms[HD_DRAW_VECTORS * 4];
        memcpy(uniforms, hd.render_model[first->model].uniforms, sizeof(uniforms));
        uniforms[26] = (float)((uint32)uniforms[26] | HD_PASS_ORDERED);
        if (!submit_face_range(model, model->indices, first_index, count, uniforms, HD_BLEND_OPAQUE, 2, first->semi ? 1 : 0))
            return 0;
        if (first->semi && !submit_face_range(model, model->indices, first_index, count, uniforms, first->blend, 2, 2))
            return 0;
        index = end;
    }
    hd_pose_commit();
    return 1;
}

int hd_models_draw(VID_RECT viewport)
{
    return models_draw(viewport, 0);
}

int hd_world_draw(VID_RECT viewport)
{
    return models_draw(viewport, 1);
}

static VID_GEOMETRY *render_ui_mesh(uint32 quads)
{
    HD_UI_BUFFER *slot;
    if (!reserve_frame((void **)&hd.ui_buffers, &hd.ui_buffer_capacity, hd.ui_buffer_cursor + 1, sizeof(*hd.ui_buffers)))
        return NULL;
    if (hd.ui_buffer_cursor == hd.ui_buffer_count)
        hd.ui_buffers[hd.ui_buffer_count++] = (HD_UI_BUFFER){0};
    slot = hd.ui_buffers + hd.ui_buffer_cursor++;
    if (slot->capacity < quads)
    {
        uint32 capacity = 32;
        while (capacity < quads)
        {
            XASSERT(capacity <= UINT32_MAX / 8);
            capacity *= 2;
        }
        vid_mesh_free(slot->mesh);
        slot->mesh = vid_mesh_create_2d(capacity * 4);
        slot->capacity = slot->mesh ? capacity : 0;
    }
    return slot->mesh;
}

static float horizon_blend(const HD_FRAME *previous, const HD_FRAME *frame)
{
    float horizon = previous->horizon + (frame->horizon - previous->horizon) * hd.render_fract;
    MATRIXF view;
    if (previous->horizon_valid && frame->horizon_valid && matrix_blend(&previous->horizon_view, &frame->horizon_view, hd.render_fract, &view))
    {
        VECTORF point = {0, 0, 32767};
        RotTransf(&view, &point, &point);
        horizon = frame->center_y + frame->focal * point.vy / (point.vz > 1 ? point.vz : 1);
    }
    return horizon;
}

static int sky_blend(const HD_FRAME *frame, const HD_UI_DRAW *current, HD_UI_DRAW *draw)
{
    if (frame->previous >= HD_FRAME_COUNT)
        return 0;
    const HD_FRAME *previous = hd.frame + frame->previous;
    if (!previous->valid || previous->sequence + 1 != frame->sequence)
        return 0;
    if (current->previous_record >= previous->ui_count)
        return 0;
    const HD_UI_DRAW *source = previous->ui + current->previous_record;
    float delta = source->vertices[0].u - current->vertices[0].u;
    if (source->sky_v != current->sky_v)
    {
        if ((uint16)((source->sky_v << 8) | (source->sky_v >> 8)) != current->sky_v)
            return 0;
        delta += 256;
    }
    float period = (uint8)current->sky_v == (uint8)(current->sky_v >> 8) ? 256.0f : 512.0f;
    while (delta > period * 0.5f)
        delta -= period;
    while (delta < -period * 0.5f)
        delta += period;
    float horizon = horizon_blend(previous, frame);
    for (uint32 corner = 0; corner < 4; corner++)
    {
        draw->vertices[corner].u = current->vertices[corner].u + delta * (1 - hd.render_fract);
        float a = source->vertices[corner].y - previous->horizon, b = current->vertices[corner].y - frame->horizon;
        draw->vertices[corner].y = horizon + a + (b - a) * hd.render_fract;
    }
    return 1;
}

static int background_blend(const HD_FRAME *frame, const HD_UI_DRAW *current, HD_UI_DRAW *draw)
{
    const HD_FRAME *previous = NULL;
    const HD_UI_DRAW *source = NULL;
    if (frame->previous < HD_FRAME_COUNT)
    {
        previous = hd.frame + frame->previous;
        if (previous->valid && previous->sequence + 1 == frame->sequence)
        {
            if (current->previous_record < previous->ui_count)
                source = previous->ui + current->previous_record;
            if (source && (source->flags != current->flags || source->horizon_edges != current->horizon_edges))
                source = NULL;
        }
    }
    if (source && current->background_part == HD_BACKGROUND_PARALLAX)
    {
        sint32 delta = (sint32)current->parallax_yaw - source->parallax_yaw;
        /* the original finite image jumps at the signed yaw boundary */
        if (delta > 32768 || delta < -32768)
            source = NULL;
        else
            for (uint32 corner = 0; corner < 4; corner++)
                draw->vertices[corner].x = source->vertices[corner].x + (current->vertices[corner].x - source->vertices[corner].x) * hd.render_fract;
    }
    if (current->flags & HD_TEX_BACKGROUND_MASK)
    {
        if (!source || hd.render_fract == 1)
            return 0;
        MATRIXF view;
        if (!camera_view_blend(&source->camera, &current->camera, &view))
            return 0;
        for (uint32 corner = 0; corner < 4; corner++)
        {
            if (hd.render_fract == 0)
            {
                draw->mask[corner] = source->mask[corner];
                continue;
            }
            VECTORF a = {source->mask[corner].x, source->mask[corner].y, source->mask[corner].z};
            VECTORF b = {current->mask[corner].x, current->mask[corner].y, current->mask[corner].z};
            RotTransf(&source->camera, &a, &a);
            RotTransf(&current->camera, &b, &b);
            a.vx += (b.vx - a.vx) * hd.render_fract;
            a.vy += (b.vy - a.vy) * hd.render_fract;
            a.vz += (b.vz - a.vz) * hd.render_fract;
            RotTransf(&view, &a, &a);
            draw->mask[corner].x = a.vx;
            draw->mask[corner].y = a.vy;
            draw->mask[corner].z = a.vz;
        }
        return 1;
    }
    float horizon = source ? horizon_blend(previous, frame) : frame->horizon;
    for (uint32 corner = 0; corner < 4; corner++)
    {
        uint32 edge = corner < 2 ? HD_HORIZON_TOP : HD_HORIZON_BOTTOM;
        float b = current->vertices[corner].y;
        if (source)
        {
            float a = source->vertices[corner].y;
            if (current->horizon_edges & edge)
            {
                a -= previous->horizon;
                b -= frame->horizon;
            }
            b = a + (b - a) * hd.render_fract;
            if (current->horizon_edges & edge)
                b += horizon;
        }
        if (corner < 2 && (current->horizon_edges & HD_HORIZON_CLAMP_TOP) && b > 0)
            b = 0;
        draw->vertices[corner].y = b;
    }
    return 1;
}

static int ground_blend(const HD_FRAME *frame, const HD_UI_DRAW *current, HD_UI_DRAW *draw, float camera[5])
{
    if (frame->previous >= HD_FRAME_COUNT || frame->ground_count != 1)
        return 0;
    const HD_FRAME *previous = hd.frame + frame->previous;
    if (!previous->valid || previous->sequence + 1 != frame->sequence || previous->ground_count != 1)
        return 0;
    const HD_UI_DRAW *source = current->previous_record < previous->ui_count ? previous->ui + current->previous_record : NULL;
    MATRIXF pose;
    if (!source || !matrix_blend(&previous->ground_maps[0].pose, &frame->ground_maps[0].pose, hd.render_fract, &pose))
        return 0;
    memcpy(camera, pose.t, 3 * sizeof(float));
    camera[3] = pose.m[0][2];
    camera[4] = pose.m[0][0];
    float horizon = horizon_blend(previous, frame);
    for (uint32 corner = 0; corner < 4; corner++)
    {
        float a = source->vertices[corner].y - previous->horizon, b = current->vertices[corner].y - frame->horizon;
        draw->vertices[corner].y = horizon + a + (b - a) * hd.render_fract;
    }
    return 1;
}

static int ui_draw_selected(VID_RECT viewport, HD_UI_LAYER layer, uint32 selected, uint32 selected_count)
{
    const HD_FRAME *frame = hd.frame + hd.last_frame;
    uint32 count = 0, bucket, first;
    float uniforms[HD_DRAW_VECTORS * 4] = {0};
    float horizontal, ui_horizontal, ui_vertical;
    XASSERT(layer >= HD_UI_BACKGROUND && layer <= HD_UI_EFFECT);
    if (!frame->valid || viewport.w <= 0 || viewport.h <= 0)
        return 0;
    if (!frame->ui_count)
        return 1;
    if (!hd_vram_sync() || !reserve_frame((void **)&hd.ui_order, &hd.ui_order_capacity, frame->ui_count, sizeof(*hd.ui_order)))
        return 0;
    if (frame->ground_count && (hd.ground_frame != hd.last_frame || hd.ground_sequence != frame->sequence))
    {
        if (!tables_init())
            return 0;
        uint16 *pixels = (uint16 *)vid_tex_data(hd.ground_colors);
        uint32 first_dirty = UINT32_MAX, last_dirty = 0;
        for (uint32 map = 0; map < frame->ground_count; map++)
        {
            uint32 base = (HD_GROUND_MAP_BASE + (hd.last_frame * HD_GROUND_MAP_COUNT + map) * HD_GROUND_MAP_WORDS) * 4;
            for (uint32 item = 0; item < 8192; item++)
            {
                uint16 word = frame->ground_maps[map].colors[item * 2] | ((uint16)frame->ground_maps[map].colors[item * 2 + 1] << 8);
                if (pixels[base + item] != word)
                {
                    pixels[base + item] = word;
                    if (first_dirty == UINT32_MAX)
                        first_dirty = (base + item) / 4;
                    last_dirty = (base + item) / 4;
                }
            }
        }
        if (first_dirty != UINT32_MAX)
        {
            uint32 first = first_dirty / HD_TABLE_WIDTH;
            vid_tex_commit_rows(hd.ground_colors, first, last_dirty / HD_TABLE_WIDTH - first + 1);
        }
    }
    hd.ground_frame = hd.last_frame;
    hd.ground_sequence = frame->sequence;
    if (selected != UINT32_MAX)
    {
        for (uint32 item = 0; item < selected_count; item++)
        {
            uint32 index = hd.traversal[selected + item].face;
            XASSERT(index < frame->ui_count && frame->ui[index].layer == layer);
            hd.ui_order[count++] = index;
        }
    }
    else
        for (bucket = 0; bucket < RENDER_OT_COUNT; bucket++)
            for (uint32 item = frame->ui_head[layer][bucket]; item != UINT32_MAX; item = frame->ui[item].next)
                hd.ui_order[count++] = item;
    uniforms[16] = uniforms[17] = uniforms[18] = uniforms[19] = 1;
    horizontal = (4.0f / 3.0f) * viewport.h / viewport.w;
    /* fit the complete UI in portrait views while keeping wide-screen anchors */
    ui_horizontal = horizontal > 1 ? 1 : horizontal;
    ui_vertical = horizontal > 1 ? 1 / horizontal : 1;
    vid_viewport(viewport);
    for (first = 0; first < count;)
    {
        const HD_UI_DRAW *draw = frame->ui + hd.ui_order[first];
        HD_UI_DRAW blended;
        if (draw->background_part != HD_BACKGROUND_NONE)
        {
            blended = *draw;
            if (background_blend(frame, draw, &blended))
                draw = &blended;
        }
        if (hd.render_fract != 1 && (draw->flags & HD_TEX_SKY))
        {
            blended = *draw;
            if (sky_blend(frame, draw, &blended))
                draw = &blended;
        }
        if (hd.render_fract != 1 && (draw->flags & (HD_TEX_BILLBOARD | HD_TEX_RIBBON)))
        {
            blended = *draw;
            if ((draw->hand_actor < ACTOR_COUNT ? hand_ui_blend(frame, draw, &blended) : object_ui_blend(frame, draw, &blended)) || ui_camera_seed(frame, draw, &blended))
                draw = &blended;
        }
        if (draw->flags & HD_TEX_RIBBON)
        {
            float depth = (draw->effect.point[0].vz + draw->effect.point[1].vz) * 0.5f + draw->effect.depth_bias;
            if (depth < 0 || depth >= 8192)
            {
                first++;
                continue;
            }
        }
        uint32 end = first + 1, vertex_count, quad_count;
        int semi = (draw->flags & HD_TEX_SEMI) != 0;
        VID_GEOMETRY *mesh;
        VID_INDEX *indices;
        VID_VERTEX2D *vertices;
        if (!semi && draw->background_part == HD_BACKGROUND_NONE && !(draw->flags & (HD_TEX_SKY | HD_TEX_DISTANT_GROUND | HD_TEX_BACKGROUND_MASK | HD_TEX_BILLBOARD | HD_TEX_RIBBON)))
            while (end < count && frame->ui[hd.ui_order[end]].background_part == HD_BACKGROUND_NONE && !(frame->ui[hd.ui_order[end]].flags & (HD_TEX_SEMI | HD_TEX_SKY | HD_TEX_DISTANT_GROUND | HD_TEX_BACKGROUND_MASK | HD_TEX_BILLBOARD | HD_TEX_RIBBON)))
                end++;
        if ((draw->flags & HD_TEX_BILLBOARD) && !draw->effect.fixed)
            while (end < count)
            {
                const HD_UI_DRAW *next = frame->ui + hd.ui_order[end];
                if (!(next->flags & HD_TEX_BILLBOARD) || ((next->flags ^ draw->flags) & (HD_TEX_SEMI | HD_TEX_TEXTURED)) || (semi && ((next->page ^ draw->page) & 96u)) || next->effect.fixed || next->region[0] != draw->region[0] || next->region[1] != draw->region[1] || next->extent != draw->extent)
                    break;
                end++;
            }
        if (draw->flags & HD_TEX_DISTANT_GROUND)
        {
            memcpy(uniforms, frame->ground_maps[draw->ground_map].camera, 5 * sizeof(float));
            if (hd.render_fract != 1)
            {
                blended = *draw;
                if (ground_blend(frame, draw, &blended, uniforms))
                    draw = &blended;
            }
        }
        if (draw->flags & (HD_TEX_BACKGROUND_MASK | HD_TEX_BILLBOARD | HD_TEX_RIBBON))
        {
            float projection[HD_DRAW_VECTORS * 4];
            MATRIXF identity;
            IdentityMatrixf(&identity);
            draw_uniforms(frame, &identity, 0, 0, viewport, projection);
            memcpy(uniforms, projection + 12, 4 * sizeof(float));
            uniforms[4] = projection[20];
            uniforms[5] = projection[21];
            uniforms[6] = 2.0f / frame->height;
            uniforms[7] = 2.0f / frame->width * horizontal;
        }
        if (draw->flags & HD_TEX_BILLBOARD)
        {
            uniforms[8] = draw->effect.offset[0];
            uniforms[9] = draw->effect.offset[1];
            uniforms[10] = (float)draw->region[0];
            uniforms[11] = (float)draw->region[1];
            uniforms[12] = (float)draw->extent;
        }
        if (draw->flags & HD_TEX_RIBBON)
        {
            for (uint32 point = 0; point < 2; point++)
            {
                memcpy(uniforms + 8 + point * 4, &draw->effect.point[point], 3 * sizeof(float));
                uniforms[11 + point * 4] = draw->effect.size[point];
                uniforms[28 + point] = (float)draw->effect.extent[point];
                for (uint32 side = 0; side < 2; side++)
                {
                    uint16 uv = draw->effect.uv[point * 2 + side];
                    uniforms[20 + point * 4 + side * 2] = (float)(uv & 255);
                    uniforms[21 + point * 4 + side * 2] = (float)(uv >> 8);
                }
            }
        }
        quad_count = end - first;
        XASSERT(quad_count <= UINT32_MAX / 6);
        mesh = render_ui_mesh(quad_count);
        indices = quad_indices(quad_count);
        if (!mesh || !indices)
            return 0;
        vertices = vid_mesh_data_2d(mesh);
        vertex_count = quad_count * 4;
        memset(vertices, 0, vertex_count * sizeof(*vertices));
        for (uint32 item = first; item < end; item++)
        {
            const HD_UI_DRAW *source = frame->ui + hd.ui_order[item];
            HD_UI_DRAW interpolated;
            if (item == first)
                source = draw;
            else if (hd.render_fract != 1 && (source->flags & HD_TEX_BILLBOARD))
            {
                interpolated = *source;
                if ((source->hand_actor < ACTOR_COUNT ? hand_ui_blend(frame, source, &interpolated) : object_ui_blend(frame, source, &interpolated)) || ui_camera_seed(frame, source, &interpolated))
                    source = &interpolated;
            }
            for (uint32 corner = 0; corner < 4; corner++)
            {
                VID_VERTEX2D *vertex = vertices + (item - first) * 4 + corner;
                if (source->flags & (HD_TEX_BILLBOARD | HD_TEX_RIBBON))
                {
                    uint32 end = (source->flags & HD_TEX_RIBBON) ? corner >> 1 : 0;
                    vertex->pos[0] = source->effect.point[end].vx;
                    vertex->pos[1] = source->effect.point[end].vy;
                    vertex->pos[2] = source->effect.point[end].vz;
                    vertex->pos[3] = source->effect.depth_bias;
                    vertex->alpha = source->effect.size[end];
                    uint16 uv = source->effect.uv[source->effect.fixed ? 0 : corner];
                    vertex->uv[0] = (float)(uv & 255);
                    vertex->uv[1] = (float)(uv >> 8);
                    vertex->color[0] = (uint8)source->effect.color;
                    vertex->color[1] = (uint8)(source->effect.color >> 8);
                    vertex->color[2] = (uint8)(source->effect.color >> 16);
                    vertex->color[3] = 255;
                    vertex->tex[0] = source->page;
                    vertex->tex[1] = source->clut;
                    vertex->tex[2] = source->flags;
                    vertex->tex[3] = source->effect.extent[1];
                    vertex->rect[0] = (uint16)(corner | (source->effect.fixed ? 4 : 0));
                    vertex->rect[1] = source->effect.extent[(source->flags & HD_TEX_RIBBON) ? end : 0];
                    continue;
                }
                if (source->flags & HD_TEX_BACKGROUND_MASK)
                {
                    const HD_MASK_VERTEX *point = source->mask + corner;
                    vertex->pos[0] = point->x;
                    vertex->pos[1] = point->y;
                    vertex->pos[2] = point->z;
                    vertex->pos[3] = 1;
                    vertex->alpha = point->offset_y;
                    vertex->tex[2] = HD_TEX_BACKGROUND_MASK;
                    continue;
                }
                const HD_UI_VERTEX *point = source->vertices + corner;
                float x = 2.0f * (point->x + (hd.presenting ? hd.display_offset[0] : frame->offset_x)) / frame->width;
                vertex->pos[0] = source->layout == HD_UI_STRETCH ? x - 1 : source->layout == HD_UI_LEFT ? -1 + x * ui_horizontal : source->layout == HD_UI_RIGHT ? 1 + (x - 2) * ui_horizontal : (x - 1) * ui_horizontal;
                vertex->pos[1] = 1.0f - 2.0f * (point->y + (hd.presenting ? hd.display_offset[1] : frame->offset_y)) / frame->height;
                if (source->layout != HD_UI_STRETCH)
                    vertex->pos[1] *= ui_vertical;
                vertex->pos[3] = 1;
                vertex->uv[0] = point->u - (source->region[0] & 255u);
                if (source->flags & (HD_TEX_SKY | HD_TEX_DISTANT_GROUND))
                    vertex->uv[0] += (point->x - frame->width * 0.5f) * (1.0f / horizontal - 1.0f);
                vertex->uv[1] = point->v - ((source->flags & HD_TEX_SKY) ? 0 : source->region[1]);
                vertex->rect[0] = source->region[0];
                vertex->rect[1] = source->region[1];
                vertex->color[0] = (uint8)point->color;
                vertex->color[1] = (uint8)(point->color >> 8);
                vertex->color[2] = (uint8)(point->color >> 16);
                vertex->color[3] = 255;
                vertex->alpha = 1;
                vertex->tex[0] = source->page;
                vertex->tex[1] = source->clut;
                vertex->tex[2] = source->flags;
                vertex->tex[3] = source->flags & HD_TEX_SKY ? source->sky_v : source->flags & HD_TEX_DISTANT_GROUND ? 31808 : source->extent;
            }
        }
        vid_mesh_commit(mesh);
        HD_MODEL model = {0};
        model.mesh = mesh;
        if (!semi)
        {
            if (!submit_faces(&model, indices, quad_count * 6, uniforms, HD_BLEND_OPAQUE, layer == HD_UI_EFFECT ? 3 : 1, 0))
                return 0;
        }
        else
        {
            static const HD_BLEND modes[4] = {HD_BLEND_HALF, HD_BLEND_ADD, HD_BLEND_SUBTRACT, HD_BLEND_QUARTER};
            /* preserve each quad's opaque/STP pair before the next overlapping quad */
            for (uint32 quad = 0; quad < quad_count; quad++)
            {
                if ((draw->flags & HD_TEX_TEXTURED) && !submit_face_range(&model, indices, quad * 6, 6, uniforms, HD_BLEND_OPAQUE, layer == HD_UI_EFFECT ? 3 : 1, 1))
                    return 0;
                if (!submit_face_range(&model, indices, quad * 6, 6, uniforms, modes[(draw->page >> 5) & 3], layer == HD_UI_EFFECT ? 3 : 1, 2))
                    return 0;
            }
        }
        first = end;
    }
    hd_pose_commit();
    return 1;
}

int hd_ui_draw(VID_RECT viewport, HD_UI_LAYER layer)
{
    return ui_draw_selected(viewport, layer, UINT32_MAX, 0);
}
