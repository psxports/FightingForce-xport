#include "game.h"

const RESOURCE_LOADER *stage_resource_loaders[STAGE_RESOURCE_KIND_COUNT][RESOURCE_STAGE_COUNT];

static uint32 load_stage0_surface(void)
{
    FUNCTION_MARKER(0x8005e148, "GAME.EXE");
    tex_load_background("STG11.PDT", "STG11.PPL", 1);
    return terrain_load("STG11.PBL", "STG11.MPD");
}

static uint32 load_stage1_surface(void)
{
    FUNCTION_MARKER(0x8005e190, "GAME.EXE");
    tex_load_background("STG12.PDT", "STG12.PPL", 1);
    return terrain_load("STG12.PBL", "STG12.MPD");
}

static uint32 load_stage2_surface(void)
{
    FUNCTION_MARKER(0x8005e1d8, "GAME.EXE");
    /* BACK13 contains tiles; the original raw upload reads beyond its archive */
    tex_load_background("BACK13.PDT", "BACK13.PPL", 0);
    return terrain_load("STG13.PBL", "STG13.MPD");
}

static uint32 load_stage3_surface(void)
{
    FUNCTION_MARKER(0x8005e220, "GAME.EXE");
    return terrain_load("STG14.PBL", "STG14.MPD");
}

static uint32 load_stage4_surface(void)
{
    FUNCTION_MARKER(0x8005e250, "GAME.EXE");
    tex_load_background("STG14.PDT", "STG14.PPL", 1);
    return terrain_load("STG15.PBL", "STG15.MPD");
}

static uint32 load_stage5_surface(void)
{
    FUNCTION_MARKER(0x8005e298, "GAME.EXE");
    tex_load_background("STG21.PDT", "STG21.PPL", 1);
    return terrain_load("STG21.PBL", "STG21.MPD");
}

static uint32 load_stage6_surface(void)
{
    FUNCTION_MARKER(0x8005e2e0, "GAME.EXE");
    tex_load_background("STG21.PDT", "STG21.PPL", 1);
    return terrain_load("STG22.PBL", "STG22.MPD");
}

static uint32 load_stage7_surface(void)
{
    FUNCTION_MARKER(0x8005e328, "GAME.EXE");
    tex_load_background("STG31.PDT", "STG31.PPL", 1);
    return terrain_load("STG31.PBL", "STG31.MPD");
}

static uint32 load_stage8_surface(void)
{
    FUNCTION_MARKER(0x8005e370, "GAME.EXE");
    tex_load_background("STG32.PDT", "STG32.PPL", 1);
    return terrain_load("STG32A.PBL", "STG32A.MPD");
}

static uint32 load_stage9_surface(void)
{
    FUNCTION_MARKER(0x8005e3b8, "GAME.EXE");
    tex_load_background("STG32.PDT", "STG32.PPL", 1);
    return terrain_load("STG32B.PBL", "STG32B.MPD");
}

static uint32 load_stage10_surface(void)
{
    FUNCTION_MARKER(0x8005e400, "GAME.EXE");
    tex_load_background("STG33.PDT", "STG33.PPL", 1);
    return terrain_load("STG33.PBL", "STG33.MPD");
}

static uint32 load_stage11_surface(void)
{
    FUNCTION_MARKER(0x8005e448, "GAME.EXE");
    tex_load_background("STG33A.PDT", "STG33A.PPL", 1);
    return terrain_load("STG33A.PBL", "STG33A.MPD");
}

static uint32 load_stage12_surface(void)
{
    FUNCTION_MARKER(0x8005e490, "GAME.EXE");
    tex_load_background("STG33A.PDT", "STG33A.PPL", 1);
    return terrain_load("STG41.PBL", "STG41.MPD");
}

static uint32 load_stage14_surface(void)
{
    FUNCTION_MARKER(0x8005e4d8, "GAME.EXE");
    tex_load_background("STG11.PDT", "STG11.PPL", 1);
    return terrain_load("STG51.PBL", "STG51.MPD");
}

static uint32 load_stage15_surface(void)
{
    FUNCTION_MARKER(0x8005e520, "GAME.EXE");
    tex_load_background("STG61.PDT", "STG61.PPL", 1);
    return terrain_load("STG61.PBL", "STG61.MPD");
}

static uint32 load_stage16_surface(void)
{
    FUNCTION_MARKER(0x8005e568, "GAME.EXE");
    tex_load_background("STG62.PDT", "STG62.PPL", 1);
    return terrain_load("STG62.PBL", "STG62.MPD");
}

static uint32 load_stage17_surface(void)
{
    FUNCTION_MARKER(0x8005e5b0, "GAME.EXE");
    tex_load_background("STG63.PDT", "STG63.PPL", 1);
    return terrain_load("STG63.PBL", "STG63.MPD");
}

static uint32 load_stage18_surface(void)
{
    FUNCTION_MARKER(0x8005e5f8, "GAME.EXE");
    tex_load_background("STG63.PDT", "STG63.PPL", 1);
    return terrain_load("STG64.PBL", "STG64.MPD");
}

static uint32 load_stage19_surface(void)
{
    FUNCTION_MARKER(0x8005e640, "GAME.EXE");
    tex_load_background("STG62.PDT", "STG62.PPL", 1);
    return terrain_load("STG65.PBL", "STG65.MPD");
}

static uint32 load_stage20_surface(void)
{
    FUNCTION_MARKER(0x8005e688, "GAME.EXE");
    tex_load_background("STG61.PDT", "STG61.PPL", 1);
    return terrain_load("STG66.PBL", "STG66.MPD");
}

static uint32 load_stage21_surface(void)
{
    FUNCTION_MARKER(0x8005e6d0, "GAME.EXE");
    tex_load_background("STG62.PDT", "STG62.PPL", 1);
    return terrain_load("STG67.PBL", "STG67.MPD");
}

static uint32 load_stage22_surface(void)
{
    FUNCTION_MARKER(0x8005e718, "GAME.EXE");
    return terrain_load("STG71.PBL", "STG71.MPD");
}

static uint32 load_stage23_surface(void)
{
    FUNCTION_MARKER(0x8005e748, "GAME.EXE");
    tex_load_background("STG72.PDT", "STG72.PPL", 1);
    return terrain_load("STG72.PBL", "STG72.MPD");
}

static uint32 load_stage24_surface(void)
{
    FUNCTION_MARKER(0x8005e790, "GAME.EXE");
    return terrain_load("STG73.PBL", "STG73.MPD");
}

uint32 resource_load_frontend_surface(void)
{
    FUNCTION_MARKER(0x8005e7c0, "GAME.EXE");
    tex_load_background("STGFE.PDT", "STGFE.PPL", 1);
    return terrain_load("STGFE.PBL", "STGFE.MPD");
}

static uint32 load_stage28_surface(void)
{
    FUNCTION_MARKER(0x8005e808, "GAME.EXE");
    tex_load_background("STG21.PDT", "STG21.PPL", 1);
    return terrain_load("STGC3.PBL", "STGC3.MPD");
}

static uint32 load_stgc5_surface(void)
{
    FUNCTION_MARKER(0x8005e850, "GAME.EXE");
    tex_load_background("STGC5.PDT", "STGC5.PPL", 1);
    return terrain_load("STGC5.PBL", "STGC5.MPD");
}

static uint32 load_stgc6_surface(void)
{
    FUNCTION_MARKER(0x8005e898, "GAME.EXE");
    tex_load_background("STGC6.PDT", "STGC6.PPL", 1);
    return terrain_load("STGC6.PBL", "STGC6.MPD");
}

static uint32 load_stage31_surface(void)
{
    FUNCTION_MARKER(0x8005e8e0, "GAME.EXE");
    tex_load_background("STGC7.PDT", "STGC7.PPL", 1);
    return terrain_load("STGC7.PBL", "STGC7.MPD");
}

static uint32 load_stage33_surface(void)
{
    FUNCTION_MARKER(0x8005e928, "GAME.EXE");
    tex_load_background("STGC7.PDT", "STGC7.PPL", 1);
    return terrain_load("STGC9.PBL", "STGC9.MPD");
}

static uint32 load_stage34_surface(void)
{
    FUNCTION_MARKER(0x8005e970, "GAME.EXE");
    tex_load_background("STG11.PDT", "STG11.PPL", 1);
    return terrain_load("STGC11.PBL", "STGC11.MPD");
}

static uint32 load_stage0_actors(void)
{
    FUNCTION_MARKER(0x80034530, "GAME.EXE");
    resource_load(85, resource_arenas.scratch.context, NULL, 0, NULL, &resource_arenas.scratch);
    RESOURCE_VIEW dummy = resource_find("DUMMY.DAT");
    {
        uintptr_t ptr = (uintptr_t)dummy.data, base = (uintptr_t)resource_storage.data;
        /* TODO: Investigate a DUMMY cursor outside its actual native resource allocation */
        XASSERT(ptr >= base && ptr - base <= RESOURCE_STORAGE_CAPACITY);
        resource_arenas.scratch = (RESOURCE_CURSOR){dummy.data, RESOURCE_STORAGE_CAPACITY - (uint32)(ptr - base), dummy.context_address};
    }
    resource_load_animations("GENMALE.ADT", -1);
    resource_load_animations("GENBIGMA.ADT", -3);
    character_load_bigblack();
    return character_load_suit_variants();
}

static uint32 load_stage0_models(void)
{
    FUNCTION_MARKER(0x800345a0, "GAME.EXE");
    MODEL_REFERENCE model0, model1, model2, model3, model4, model5, model6, model7;
    uint32 result;
    uint32 load_context, alloc_context, byte_len;
    load_context = resource_arenas.persistent.context;
    result = resource_load(0x00000029u, load_context, NULL, 0, NULL, &resource_arenas.persistent);
    alloc_context = resource_arenas.persistent.context;
    byte_len = resource_io.file_size;
    /* TODO: Investigate allocation beyond the actual native resource owner */
    XASSERT(resource_arenas.persistent.data && (byte_len) <= resource_arenas.persistent.available);
    resource_arenas.persistent.data += byte_len;
    resource_arenas.persistent.available -= byte_len;
    resource_arenas.persistent.context = alloc_context + byte_len;
    result = resource_load_model("HQ1.MOD", 0x00000038u, 0x00000007u);
    result = resource_load_model("HQ2.MOD", 0x00000039u, 0x00000007u);
    result = resource_load_model("HQ3.MOD", 0x0000003au, 0x00000007u);
    result = resource_load_model("STG1BUSH.MOD", 0x0000003bu, 0x00000007u);
    result = resource_load_model("STG1GATE.MOD", 0x00000046u, 0x00000007u);
    result = resource_load_model("STG1GTE2.MOD", 0x00000047u, 0x00000007u);
    result = resource_load_model("STG1GTE3.MOD", 0x00000048u, 0x00000007u);
    result = resource_load_model("STG1GTE4.MOD", 0x00000049u, 0x00000007u);
    result = resource_load_model("STG1GTE5.MOD", 0x0000004au, 0x00000007u);
    result = resource_load_model("STG1GTE6.MOD", 0x0000004bu, 0x00000007u);
    result = resource_load_model("STG1GTE7.MOD", 0x0000004cu, 0x00000007u);
    model0 = model_refs[75];
    model_refs[77] = model0;
    result = resource_load_model("STG1WALL.MOD", 0x0000003cu, 0x00000007u);
    result = resource_load_model("LAMP.MOD", 0x00000266u, 0x00000007u);
    result = resource_load_model("LAMP2.MOD", 0x00000267u, 0x00000007u);
    result = resource_load_model("STG1BAR.MOD", 0x0000003du, 0x00000007u);
    result = resource_load_model("BUILD1.MOD", 0x0000026au, 0x00000007u);
    result = resource_load_model("BUILD2.MOD", 0x0000026bu, 0x00000007u);
    result = resource_load_model("BUILD3.MOD", 0x0000026cu, 0x00000007u);
    result = resource_load_model("BUILD4.MOD", 0x0000026du, 0x00000007u);
    result = resource_load_model("BUILD5.MOD", 0x0000026eu, 0x00000007u);
    result = resource_load_model("TREE.MOD", 0x0000026fu, 0x00000007u);
    result = resource_load_model("TREE2.MOD", 0x00000270u, 0x00000007u);
    result = resource_load_model("BUSH.MOD", 0x00000271u, 0x00000007u);
    result = resource_load_model("STG1DOOR.MOD", 0x000001d0u, 0x00000007u);
    result = resource_load_model("CARLIGHT.MOD", 0x00000397u, 0x00000000u);
    result = resource_load_model("BUSSHAD.MOD", 0x00000059u, 0x00000000u);
    result = resource_load_model("CAR1BOD1.MOD", 0x0000005eu, 0x00000046u);
    result = resource_load_model("CAR1BOD2.MOD", 0x0000005fu, 0x00000046u);
    result = resource_load_model("CAR1BOD3.MOD", 0x00000060u, 0x00000046u);
    result = resource_load_model("CAR1WHL.MOD", 0x00000210u, 0x00000046u);
    result = resource_load_model("CAR1WHLS.MOD", 0x0000005du, 0x00000046u);
    result = resource_load_model("CAR1BON.MOD", 0x00000061u, 0x00000046u);
    result = resource_load_model("CAR1TRN.MOD", 0x00000062u, 0x00000046u);
    result = resource_load_model("PCARBOD1.MOD", 0x0000006cu, 0x00000046u);
    result = resource_load_model("PCARBOD2.MOD", 0x0000006du, 0x00000046u);
    result = resource_load_model("PCARBOD3.MOD", 0x0000006eu, 0x00000046u);
    result = resource_load_model("PCARTRN.MOD", 0x00000070u, 0x00000046u);
    result = resource_load_model("PCARBON.MOD", 0x0000006fu, 0x00000046u);
    result = resource_load_model("CAR1EXP.MOD", 0x00000063u, 0x00000047u);
    result = resource_load_model("CAR1EDR.MOD", 0x00000065u, 0x00000047u);
    result = resource_load_model("CAR1EBN.MOD", 0x00000066u, 0x00000047u);
    result = resource_load_model("CAR1ENG.MOD", 0x00000215u, 0x00000047u);
    result = resource_load_model("CAR1ENG2.MOD", 0x00000216u, 0x00000047u);
    result = resource_load_model("CAR1EFB.MOD", 0x00000067u, 0x00000047u);
    result = resource_load_model("CAR1ETR.MOD", 0x00000068u, 0x00000047u);
    model1 = model_refs[99];
    model_refs[100] = model1;
    model2 = model_refs[528];
    model_refs[105] = model2;
    result = resource_load_model("TAXI.MOD", 0x00000054u, 0x00000046u);
    result = resource_load_model("PICKUP2.MOD", 0x0000004eu, 0x00000046u);
    result = resource_load_model("JEEP1.MOD", 0x0000004fu, 0x00000046u);
    result = resource_load_model("BUS.MOD", 0x00000050u, 0x00000046u);
    result = resource_load_model("CARRED1.MOD", 0x00000051u, 0x00000046u);
    result = resource_load_model("CARBLUE1.MOD", 0x00000052u, 0x00000046u);
    result = resource_load_model("POLICAR.MOD", 0x00000053u, 0x00000046u);
    result = resource_load_model("PICK2EXP.MOD", 0x00000055u, 0x00000047u);
    result = resource_load_model("JEEP1EXP.MOD", 0x00000056u, 0x00000047u);
    result = resource_load_model("BUSEXP.MOD", 0x00000057u, 0x00000047u);
    model3 = model_refs[99];
    model_refs[88] = model3;
    result = resource_load_model("VAN1BOD2.MOD", 0x0000003fu, 0x00000046u);
    model4 = model_refs[63];
    model_refs[64] = model4;
    result = resource_load_model("VAN1WHLS.MOD", 0x00000041u, 0x00000046u);
    result = resource_load_model("VAN2BOD.MOD", 0x00000011u, 0x00000046u);
    model5 = model_refs[17];
    model_refs[66] = model5;
    result = resource_load_model("VAN1WHL.MOD", 0x00000211u, 0x00000046u);
    model6 = model_refs[529];
    model_refs[69] = model6;
    result = resource_load_model("VAN2EXP.MOD", 0x00000043u, 0x00000047u);
    result = resource_load_model("VAN2SHAD.MOD", 0x00000044u, 0x00000000u);
    result = resource_load_model("BARRIER3.MOD", 0x00000273u, 0x00000046u);
    result = resource_load_model("BARR3LEG.MOD", 0x00000274u, 0x00000046u);
    result = resource_load_model("BARR3BAR.MOD", 0x000001edu, 0x00000046u);
    result = resource_load_model("BOXMETL1.MOD", 0x0000020eu, 0x00000046u);
    result = resource_load_model("BOXMETL2.MOD", 0x00000275u, 0x00000046u);
    model7 = model_refs[629];
    model_refs[608] = model7;
    result = resource_load_model("PARKLAMP.MOD", 0x00000276u, 0x00000046u);
    result = resource_load_model("SECURCAM.MOD", 0x00000279u, 0x00000046u);
    result = resource_load_model("PRKFENCE.MOD", 0x00000281u, 0x00000046u);
    result = resource_load_model("POLCEBAR.MOD", 0x00000071u, 0x00000046u);
    result = resource_load_model("2DCOP1.MOD", 0x00000072u, 0x00000046u);
    result = resource_load_model("2DCOP2.MOD", 0x00000073u, 0x00000046u);
    result = resource_load_model("2DCOP3.MOD", 0x00000074u, 0x00000046u);
    result = resource_load_model("2DCOP4.MOD", 0x00000075u, 0x00000046u);
    return result;
}

static uint32 load_stage1_actors(void)
{
    FUNCTION_MARKER(0x80034c20u, "GAME.EXE");
    uint32 scratch_context, dummy_context;
    scratch_context = resource_arenas.scratch.context;
    resource_load(0x00000056u, scratch_context, NULL, 0, NULL, &resource_arenas.scratch);
    RESOURCE_VIEW dummy = resource_find("DUMMY.DAT");
    dummy_context = dummy.context_address;
    {
        uintptr_t ptr = (uintptr_t)dummy.data, base = (uintptr_t)resource_storage.data;
        /* TODO: Investigate a DUMMY cursor outside its actual native resource allocation */
        XASSERT(ptr >= base && ptr - base <= RESOURCE_STORAGE_CAPACITY);
        resource_arenas.scratch = (RESOURCE_CURSOR){dummy.data, RESOURCE_STORAGE_CAPACITY - (uint32)(ptr - base), dummy_context};
    }
    resource_load_animations("GENMALE.ADT", 0xffffffffu);
    character_load_suit_variants();
    return character_load_orange();
}

static uint32 load_stage1_models(void)
{
    FUNCTION_MARKER(0x80034c80u, "GAME.EXE");
    uint32 load_context, alloc_context, byte_len;
    load_context = resource_arenas.persistent.context;
    resource_load(0x0000002au, load_context, NULL, 0, NULL, &resource_arenas.persistent);
    alloc_context = resource_arenas.persistent.context;
    byte_len = resource_io.file_size;
    /* TODO: Investigate allocation beyond the actual native resource owner */
    XASSERT(resource_arenas.persistent.data && (byte_len) <= resource_arenas.persistent.available);
    resource_arenas.persistent.data += byte_len;
    resource_arenas.persistent.available -= byte_len;
    resource_arenas.persistent.context = alloc_context + byte_len;
    resource_load_model("HALL12.MOD", 0x00000077u, 0x00000008u);
    resource_load_model("DOOR.MOD", 0x000001d1u, 0x00000008u);
    resource_load_model("MONITOR.MOD", 0x00000299u, 0x00000008u);
    resource_load_model("MONIT2.MOD", 0x0000029au, 0x00000008u);
    resource_load_model("MONIT3.MOD", 0x0000029bu, 0x00000008u);
    resource_load_model("CORRIDOR.MOD", 0x0000007au, 0x00000008u);
    resource_load_model("12DOOR1.MOD", 0x000001d2u, 0x00000008u);
    resource_load_model("TREE3.MOD", 0x0000029cu, 0x00000046u);
    resource_load_model("12GATE1.MOD", 0x0000007bu, 0x00000046u);
    resource_load_model("12GATE2.MOD", 0x0000007cu, 0x00000046u);
    resource_load_model("12GATE5.MOD", 0x0000007du, 0x00000046u);
    resource_load_model("12GATE6.MOD", 0x0000007eu, 0x00000046u);
    resource_load_model("METGATE1.MOD", 0x0000007fu, 0x00000046u);
    resource_load_model("METGATE5.MOD", 0x00000080u, 0x00000046u);
    resource_load_model("METGATE2.MOD", 0x00000081u, 0x00000046u);
    resource_load_model("METGATE3.MOD", 0x00000082u, 0x00000046u);
    resource_load_model("METGATE4.MOD", 0x00000083u, 0x00000046u);
    resource_load_model("DRNKMCH2.MOD", 0x00000282u, 0x00000046u);
    resource_load_model("DRKMCH2A.MOD", 0x00000283u, 0x00000046u);
    resource_load_model("DRKMCH2B.MOD", 0x00000284u, 0x00000046u);
    resource_load_model("DRKMCH2C.MOD", 0x00000285u, 0x00000046u);
    resource_load_model("DRKMCH2D.MOD", 0x00000286u, 0x00000047u);
    resource_load_model("SODACAN.MOD", 0x00000200u, 0x00000046u);
    resource_load_model("SODACAN2.MOD", 0x00000201u, 0x00000046u);
    resource_load_model("DESK2.MOD", 0x00000079u, 0x00000046u);
    resource_load_model("INFOCNTR.MOD", 0x00000289u, 0x00000046u);
    resource_load_model("INFOSCRN.MOD", 0x0000020fu, 0x00000047u);
    resource_load_model("PLANT1.MOD", 0x0000028au, 0x00000046u);
    resource_load_model("PLANT2.MOD", 0x0000028bu, 0x00000046u);
    resource_load_model("PLANT2A.MOD", 0x0000028cu, 0x00000046u);
    resource_load_model("PLANT2B.MOD", 0x0000028du, 0x00000046u);
    resource_load_model("PLANT2C.MOD", 0x0000028eu, 0x00000046u);
    resource_load_model("WALLTV1.MOD", 0x00000248u, 0x00000047u);
    resource_load_model("GLASCASE.MOD", 0x00000287u, 0x00000046u);
    resource_load_model("GLASCSEX.MOD", 0x00000288u, 0x00000046u);
    resource_load_model("BOXMETL.MOD", 0x0000028fu, 0x00000046u);
    return resource_load_model("BOXMETLX.MOD", 0x00000290u, 0x00000046u);
}

static uint32 load_stage2_actors(void)
{
    FUNCTION_MARKER(0x80034facu, "GAME.EXE");
    uint32 scratch_context, dummy_context;
    scratch_context = resource_arenas.scratch.context;
    resource_load(0x00000057u, scratch_context, NULL, 0, NULL, &resource_arenas.scratch);
    RESOURCE_VIEW dummy = resource_find("DUMMY.DAT");
    dummy_context = dummy.context_address;
    {
        uintptr_t ptr = (uintptr_t)dummy.data, base = (uintptr_t)resource_storage.data;
        /* TODO: Investigate a DUMMY cursor outside its actual native resource allocation */
        XASSERT(ptr >= base && ptr - base <= RESOURCE_STORAGE_CAPACITY);
        resource_arenas.scratch = (RESOURCE_CURSOR){dummy.data, RESOURCE_STORAGE_CAPACITY - (uint32)(ptr - base), dummy_context};
    }
    resource_load_animations("GENMALE.ADT", 0xffffffffu);
    resource_load_animations("GENBIGMA.ADT", 0xfffffffdu);
    character_load_suit_variants();
    character_load_bigblack();
    return character_load_security();
}

static uint32 load_stage2_models(void)
{
    FUNCTION_MARKER(0x80035024u, "GAME.EXE");
    uint32 load_context, alloc_context, byte_len;
    load_context = resource_arenas.persistent.context;
    resource_load(0x0000002bu, load_context, NULL, 0, NULL, &resource_arenas.persistent);
    alloc_context = resource_arenas.persistent.context;
    byte_len = resource_io.file_size;
    /* TODO: Investigate allocation beyond the actual native resource owner */
    XASSERT(resource_arenas.persistent.data && (byte_len) <= resource_arenas.persistent.available);
    resource_arenas.persistent.data += byte_len;
    resource_arenas.persistent.available -= byte_len;
    resource_arenas.persistent.context = alloc_context + byte_len;
    resource_load_model("BACKWALL.MOD", 0x00000085u, 0x0000000au);
    resource_load_model("BACKWAL2.MOD", 0x00000086u, 0x0000000au);
    resource_load_model("CORRIDOR.MOD", 0x00000087u, 0x00000009u);
    resource_load_model("ELVBAR1A.MOD", 0x00000297u, 0x00000046u);
    resource_load_model("ELVBAR1B.MOD", 0x000001eeu, 0x00000046u);
    resource_load_model("ELVBAR1C.MOD", 0x00000298u, 0x00000046u);
    resource_load_model("WALLTV2.MOD", 0x00000249u, 0x00000047u);
    resource_load_model("INFOSCRN.MOD", 0x0000020fu, 0x00000047u);
    resource_load_model("ASHTRY.MOD", 0x0000020cu, 0x00000047u);
    resource_load_model("FIREAXE.MOD", 0x000001ecu, 0x00000047u);
    resource_load_model("FIREBOX.MOD", 0x00000245u, 0x00000047u);
    resource_load_model("FIREBOX2.MOD", 0x00000246u, 0x00000047u);
    resource_load_model("FBFRAG.MOD", 0x00000247u, 0x00000009u);
    return resource_load_model("NEWZL2.MOD", 0x00000084u, 0x00000009u);
}

static uint32 load_stage3_actors(void)
{
    FUNCTION_MARKER(0x80035184u, "GAME.EXE");
    uint32 scratch_context, dummy_context;
    scratch_context = resource_arenas.scratch.context;
    resource_load(0x00000058u, scratch_context, NULL, 0, NULL, &resource_arenas.scratch);
    RESOURCE_VIEW dummy = resource_find("DUMMY.DAT");
    dummy_context = dummy.context_address;
    {
        uintptr_t ptr = (uintptr_t)dummy.data, base = (uintptr_t)resource_storage.data;
        /* TODO: Investigate a DUMMY cursor outside its actual native resource allocation */
        XASSERT(ptr >= base && ptr - base <= RESOURCE_STORAGE_CAPACITY);
        resource_arenas.scratch = (RESOURCE_CURSOR){dummy.data, RESOURCE_STORAGE_CAPACITY - (uint32)(ptr - base), dummy_context};
    }
    resource_load_animations("GENMALE.ADT", 0xffffffffu);
    resource_load_animations("GENBIGMA.ADT", 0xfffffffdu);
    character_load_bigblack();
    return character_load_security();
}

static uint32 load_stage3_models(void)
{
    FUNCTION_MARKER(0x800351f4u, "GAME.EXE");
    uint32 load_context, alloc_context, byte_len;
    load_context = resource_arenas.persistent.context;
    resource_load(0x0000002cu, load_context, NULL, 0, NULL, &resource_arenas.persistent);
    alloc_context = resource_arenas.persistent.context;
    byte_len = resource_io.file_size;
    /* TODO: Investigate allocation beyond the actual native resource owner */
    XASSERT(resource_arenas.persistent.data && (byte_len) <= resource_arenas.persistent.available);
    resource_arenas.persistent.data += byte_len;
    resource_arenas.persistent.available -= byte_len;
    resource_arenas.persistent.context = alloc_context + byte_len;
    resource_load_model("ROOM14.MOD", 0x00000088u, 0x0000000bu);
    resource_load_model("DOOR.MOD", 0x000001d1u, 0x0000000bu);
    resource_load_model("PLANT4.MOD", 0x0000029fu, 0x00000046u);
    resource_load_model("DRNKFNTN.MOD", 0x000002a0u, 0x00000046u);
    resource_load_model("DRKF2EX1.MOD", 0x000002a1u, 0x00000046u);
    resource_load_model("DRKF2EX2.MOD", 0x000002a2u, 0x00000046u);
    resource_load_model("ASHTRY.MOD", 0x0000020cu, 0x00000047u);
    resource_load_model("WALLTV2.MOD", 0x00000249u, 0x00000047u);
    resource_load_model("OFFDOOR1.MOD", 0x00000089u, 0x0000000bu);
    resource_load_model("OFFDOOR2.MOD", 0x0000008au, 0x0000000bu);
    resource_load_model("DOOR6.MOD", 0x0000008bu, 0x0000000bu);
    return resource_load_model("DOOR6X.MOD", 0x0000008cu, 0x0000000bu);
}

static uint32 load_stage4_actors(void)
{
    FUNCTION_MARKER(0x8003532cu, "GAME.EXE");
    uint32 scratch_context, dummy_context;
    scratch_context = resource_arenas.scratch.context;
    resource_load(0x00000059u, scratch_context, NULL, 0, NULL, &resource_arenas.scratch);
    RESOURCE_VIEW dummy = resource_find("DUMMY.DAT");
    dummy_context = dummy.context_address;
    {
        uintptr_t ptr = (uintptr_t)dummy.data, base = (uintptr_t)resource_storage.data;
        /* TODO: Investigate a DUMMY cursor outside its actual native resource allocation */
        XASSERT(ptr >= base && ptr - base <= RESOURCE_STORAGE_CAPACITY);
        resource_arenas.scratch = (RESOURCE_CURSOR){dummy.data, RESOURCE_STORAGE_CAPACITY - (uint32)(ptr - base), dummy_context};
    }
    resource_load_animations("GENFMALE.ADT", 0xfffffffeu);
    character_load_vamp();
    return character_load_zeng();
}

static uint32 load_stage4_models(void)
{
    FUNCTION_MARKER(0x8003538cu, "GAME.EXE");
    uint32 load_context, alloc_context, byte_len;
    load_context = resource_arenas.persistent.context;
    resource_load(0x0000002du, load_context, NULL, 0, NULL, &resource_arenas.persistent);
    alloc_context = resource_arenas.persistent.context;
    byte_len = resource_io.file_size;
    /* TODO: Investigate allocation beyond the actual native resource owner */
    XASSERT(resource_arenas.persistent.data && (byte_len) <= resource_arenas.persistent.available);
    resource_arenas.persistent.data += byte_len;
    resource_arenas.persistent.available -= byte_len;
    resource_arenas.persistent.context = alloc_context + byte_len;
    resource_load_model("ROOM15.MOD", 0x0000008du, 0x0000000cu);
    resource_load_model("ROOM153.MOD", 0x0000008eu, 0x0000000cu);
    resource_load_model("WINPANE1.MOD", 0x000002a7u, 0x0000000cu);
    resource_load_model("WINPANE2.MOD", 0x000002a8u, 0x0000000cu);
    resource_load_model("WINPANE3.MOD", 0x000002a9u, 0x0000000cu);
    resource_load_model("WINPANE4.MOD", 0x000002aau, 0x0000000cu);
    resource_load_model("CHAIRBOS.MOD", 0x00000091u, 0x0000000cu);
    resource_load_model("DESKBOSS.MOD", 0x0000008fu, 0x0000000cu);
    resource_load_model("ASHTRY.MOD", 0x0000020cu, 0x00000046u);
    resource_load_model("PAPER1.MOD", 0x000002a3u, 0x00000046u);
    resource_load_model("PAPER2.MOD", 0x000002a4u, 0x00000046u);
    resource_load_model("PAPER3.MOD", 0x000002a5u, 0x00000046u);
    resource_load_model("BRIDGE.MOD", 0x000002a6u, 0x00000046u);
    resource_load_model("HELICOP.MOD", 0x00000092u, 0x00000046u);
    resource_load_model("HELENGIN.MOD", 0x00000093u, 0x00000046u);
    resource_load_model("ZENG2.MOD", 0x00000094u, 0x0000002eu);
    return resource_load_model("PLANT4.MOD", 0x0000029fu, 0x00000046u);
}

static uint32 load_stage5_actors(void)
{
    FUNCTION_MARKER(0x80035528u, "GAME.EXE");
    uint32 scratch_context, dummy_context;
    scratch_context = resource_arenas.scratch.context;
    resource_load(0x0000005au, scratch_context, NULL, 0, NULL, &resource_arenas.scratch);
    RESOURCE_VIEW dummy = resource_find("DUMMY.DAT");
    dummy_context = dummy.context_address;
    {
        uintptr_t ptr = (uintptr_t)dummy.data, base = (uintptr_t)resource_storage.data;
        /* TODO: Investigate a DUMMY cursor outside its actual native resource allocation */
        XASSERT(ptr >= base && ptr - base <= RESOURCE_STORAGE_CAPACITY);
        resource_arenas.scratch = (RESOURCE_CURSOR){dummy.data, RESOURCE_STORAGE_CAPACITY - (uint32)(ptr - base), dummy_context};
    }
    resource_load_animations("GENMALE.ADT", 0xffffffffu);
    resource_load_animations("CASBIKER.ADT", 0xffffffffu);
    resource_load_animations("BIKE.ADT", 0xffffffffu);
    character_load_thug1();
    character_load_thug2();
    character_load_afrotg();
    character_load_security();
    return character_load_biker_variants();
}

static uint32 load_stage5_models(void)
{
    FUNCTION_MARKER(0x800355c0u, "GAME.EXE");
    uint32 load_context, alloc_context, byte_len;
    load_context = resource_arenas.persistent.context;
    resource_load(0x0000002eu, load_context, NULL, 0, NULL, &resource_arenas.persistent);
    alloc_context = resource_arenas.persistent.context;
    byte_len = resource_io.file_size;
    /* TODO: Investigate allocation beyond the actual native resource owner */
    XASSERT(resource_arenas.persistent.data && (byte_len) <= resource_arenas.persistent.available);
    resource_arenas.persistent.data += byte_len;
    resource_arenas.persistent.available -= byte_len;
    resource_arenas.persistent.context = alloc_context + byte_len;
    resource_load_model("BEND01.MOD", 0x00000095u, 0x0000000du);
    resource_load_model("BEND02.MOD", 0x00000096u, 0x0000000du);
    resource_load_model("BLEFT01.MOD", 0x00000099u, 0x0000000du);
    resource_load_model("BLEFT02.MOD", 0x0000009au, 0x0000000du);
    resource_load_model("BLEFT03.MOD", 0x0000009bu, 0x0000000du);
    resource_load_model("BRIGHT01.MOD", 0x0000009cu, 0x0000000du);
    resource_load_model("BRIGHT02.MOD", 0x0000009du, 0x0000000du);
    resource_load_model("NEWS4.MOD", 0x0000009eu, 0x00000046u);
    resource_load_model("TREE4.MOD", 0x0000009fu, 0x00000046u);
    resource_load_model("DAYCAR1.MOD", 0x000000a0u, 0x00000046u);
    resource_load_model("LAMP51.MOD", 0x000000a1u, 0x00000046u);
    resource_load_model("BUSHELTR.MOD", 0x000000a2u, 0x00000046u);
    resource_load_model("BUSHLEXP.MOD", 0x000000a3u, 0x00000046u);
    resource_load_model("TREEFLAT.MOD", 0x000000a4u, 0x00000046u);
    resource_load_model("LAMP52.MOD", 0x000000a5u, 0x00000046u);
    resource_load_model("UPSTRUCK.MOD", 0x000000a6u, 0x00000046u);
    resource_load_model("PHONE1.MOD", 0x000000a7u, 0x00000046u);
    resource_load_model("WASTEBIN.MOD", 0x00000217u, 0x00000046u);
    resource_load_model("TAXI.MOD", 0x000000a8u, 0x00000046u);
    resource_load_model("POLICAR.MOD", 0x000000a9u, 0x00000046u);
    resource_load_model("TRAF2.MOD", 0x000000aau, 0x00000046u);
    resource_load_model("TRAF1.MOD", 0x000000abu, 0x00000046u);
    resource_load_model("STREET3.MOD", 0x000000acu, 0x00000046u);
    resource_load_model("CRATE.MOD", 0x00000218u, 0x00000046u);
    resource_load_model("CRATE1.MOD", 0x00000219u, 0x00000046u);
    resource_load_model("FIREHYDT.MOD", 0x000000adu, 0x00000046u);
    resource_load_model("FIREHEXP.MOD", 0x000000aeu, 0x00000046u);
    resource_load_model("HELCOP2.MOD", 0x000002afu, 0x00000047u);
    resource_load_model("ROTOR.MOD", 0x00000234u, 0x00000047u);
    resource_load_model("DAYCAR3.MOD", 0x000000afu, 0x00000046u);
    resource_load_model("GRNCAR1.MOD", 0x000000b0u, 0x00000046u);
    resource_load_model("REDVAN1.MOD", 0x000000b1u, 0x00000046u);
    resource_load_model("YELCAR1.MOD", 0x000000b2u, 0x00000046u);
    resource_load_model("GRNVAN1.MOD", 0x000000b3u, 0x00000046u);
    resource_load_model("REDCAR1.MOD", 0x000000b4u, 0x00000046u);
    resource_load_model("CARPOL1.MOD", 0x000000b5u, 0x00000046u);
    resource_load_model("BLKCAR1.MOD", 0x000000b6u, 0x00000046u);
    resource_load_model("MOTOWRK1.MOD", 0x00000213u, 0x00000047u);
    resource_load_model("MOTOWRK2.MOD", 0x00000208u, 0x00000047u);
    resource_load_model("MOTOWRK3.MOD", 0x00000214u, 0x00000047u);
    return resource_load_model("MOTOEXP.MOD", 0x00000244u, 0x00000047u);
}

static uint32 load_stage6_actors(void)
{
    FUNCTION_MARKER(0x8003593cu, "GAME.EXE");
    uint32 scratch_context, dummy_context;
    scratch_context = resource_arenas.scratch.context;
    resource_load(0x0000005bu, scratch_context, NULL, 0, NULL, &resource_arenas.scratch);
    RESOURCE_VIEW dummy = resource_find("DUMMY.DAT");
    dummy_context = dummy.context_address;
    {
        uintptr_t ptr = (uintptr_t)dummy.data, base = (uintptr_t)resource_storage.data;
        /* TODO: Investigate a DUMMY cursor outside its actual native resource allocation */
        XASSERT(ptr >= base && ptr - base <= RESOURCE_STORAGE_CAPACITY);
        resource_arenas.scratch = (RESOURCE_CURSOR){dummy.data, RESOURCE_STORAGE_CAPACITY - (uint32)(ptr - base), dummy_context};
    }
    return character_load_driver();
}

static uint32 load_stage6_models(void)
{
    FUNCTION_MARKER(0x80035984u, "GAME.EXE");
    uint32 load_context, alloc_context, byte_len;
    load_context = resource_arenas.persistent.context;
    resource_load(0x0000002fu, load_context, NULL, 0, NULL, &resource_arenas.persistent);
    alloc_context = resource_arenas.persistent.context;
    byte_len = resource_io.file_size;
    /* TODO: Investigate allocation beyond the actual native resource owner */
    XASSERT(resource_arenas.persistent.data && (byte_len) <= resource_arenas.persistent.available);
    resource_arenas.persistent.data += byte_len;
    resource_arenas.persistent.available -= byte_len;
    resource_arenas.persistent.context = alloc_context + byte_len;
    resource_load_model("BRIGHT01.MOD", 0x000000b7u, 0x0000000eu);
    resource_load_model("BRIGHT02.MOD", 0x000000b8u, 0x0000000eu);
    resource_load_model("BLEFT02.MOD", 0x000000b9u, 0x0000000eu);
    resource_load_model("BLEFT01.MOD", 0x000000bau, 0x0000000eu);
    resource_load_model("BLEFT03.MOD", 0x000000bbu, 0x0000000eu);
    resource_load_model("BEND01.MOD", 0x000000bcu, 0x0000000eu);
    resource_load_model("BEND02.MOD", 0x000000bdu, 0x0000000eu);
    resource_load_model("TREE4.MOD", 0x000000beu, 0x00000046u);
    resource_load_model("TREEFLAT.MOD", 0x000000bfu, 0x00000046u);
    resource_load_model("TRAF2.MOD", 0x000000c0u, 0x00000046u);
    resource_load_model("TRAF1.MOD", 0x000000c1u, 0x00000046u);
    resource_load_model("STREET3.MOD", 0x000000c2u, 0x00000046u);
    resource_load_model("BEEND01.MOD", 0x000000c3u, 0x0000000eu);
    resource_load_model("BEEND02.MOD", 0x000000c4u, 0x0000000eu);
    resource_load_model("BEND03.MOD", 0x000000c5u, 0x0000000eu);
    resource_load_model("BEND04.MOD", 0x000000c6u, 0x0000000eu);
    resource_load_model("FIRETRCK.MOD", 0x000000c7u, 0x00000046u);
    resource_load_model("POLCBAR2.MOD", 0x000000c8u, 0x00000046u);
    resource_load_model("POLCEBAR.MOD", 0x000000c9u, 0x00000046u);
    resource_load_model("FUELDRUM.MOD", 0x0000022au, 0x00000046u);
    resource_load_model("FUELDRM2.MOD", 0x0000022bu, 0x00000046u);
    resource_load_model("FUELDRM3.MOD", 0x0000022cu, 0x00000046u);
    resource_load_model("POLICAR.MOD", 0x000000cau, 0x00000046u);
    resource_load_model("CAR1EXP.MOD", 0x000000cbu, 0x00000047u);
    resource_load_model("TRCKEXP.MOD", 0x000000ccu, 0x00000047u);
    resource_load_model("AMBULNCE.MOD", 0x000000cdu, 0x00000046u);
    resource_load_model("2DFIRE1.MOD", 0x000002b0u, 0x00000046u);
    resource_load_model("2DCOP1.MOD", 0x000002b1u, 0x00000046u);
    resource_load_model("2DCOP2.MOD", 0x000002b2u, 0x00000046u);
    resource_load_model("2DFIRE2.MOD", 0x000002b3u, 0x00000046u);
    resource_load_model("2DMEDIC1.MOD", 0x000002b4u, 0x00000046u);
    resource_load_model("TRKLOAD4.MOD", 0x000000ceu, 0x00000046u);
    resource_load_model("WATRCAN1.MOD", 0x000002b5u, 0x00000046u);
    resource_load_model("FLATTYRE.MOD", 0x00000210u, 0x00000047u);
    resource_load_model("DEBRIS1.MOD", 0x00000204u, 0x00000047u);
    resource_load_model("DEBRIS2.MOD", 0x00000205u, 0x00000047u);
    resource_load_model("DEBRIS3.MOD", 0x00000206u, 0x00000047u);
    resource_load_model("DEBRIS4.MOD", 0x00000207u, 0x00000047u);
    return resource_load_model("FIREAXE.MOD", 0x000001ecu, 0x00000047u);
}

static uint32 load_stage7_actors(void)
{
    FUNCTION_MARKER(0x80035cd8u, "GAME.EXE");
    uint32 scratch_context, dummy_context;
    scratch_context = resource_arenas.scratch.context;
    resource_load(0x0000005cu, scratch_context, NULL, 0, NULL, &resource_arenas.scratch);
    RESOURCE_VIEW dummy = resource_find("DUMMY.DAT");
    dummy_context = dummy.context_address;
    {
        uintptr_t ptr = (uintptr_t)dummy.data, base = (uintptr_t)resource_storage.data;
        /* TODO: Investigate a DUMMY cursor outside its actual native resource allocation */
        XASSERT(ptr >= base && ptr - base <= RESOURCE_STORAGE_CAPACITY);
        resource_arenas.scratch = (RESOURCE_CURSOR){dummy.data, RESOURCE_STORAGE_CAPACITY - (uint32)(ptr - base), dummy_context};
    }
    resource_load_animations("GENMALE.ADT", 0xffffffffu);
    character_load_suit_variants();
    character_load_thug3();
    character_load_afrotg3();
    character_load_security();
    return character_load_orange();
}

static void load_stage7_models(void)
{
    MODEL_REFERENCE model0, model1, model2, model3, model4, model5, model6, model7;
    FUNCTION_MARKER(0x80035d50u, "GAME.EXE");
    uint32 load_context, alloc_context, byte_len;
    load_context = resource_arenas.persistent.context;
    resource_load(0x00000030u, load_context, NULL, 0, NULL, &resource_arenas.persistent);
    alloc_context = resource_arenas.persistent.context;
    byte_len = resource_io.file_size;
    /* TODO: Investigate allocation beyond the actual native resource owner */
    XASSERT(resource_arenas.persistent.data && (byte_len) <= resource_arenas.persistent.available);
    resource_arenas.persistent.data += byte_len;
    resource_arenas.persistent.available -= byte_len;
    resource_arenas.persistent.context = alloc_context + byte_len;
    resource_load_model("MALL01.MOD", 0x000000cfu, 0x0000000fu);
    resource_load_model("MALL02M.MOD", 0x000000d0u, 0x0000000fu);
    resource_load_model("MALL03.MOD", 0x000000d1u, 0x0000000fu);
    resource_load_model("MALL02A.MOD", 0x000000d2u, 0x0000000fu);
    resource_load_model("MALL04L.MOD", 0x000000d3u, 0x0000000fu);
    resource_load_model("MDOOR.MOD", 0x000000d4u, 0x0000000fu);
    resource_load_model("BACKWALL.MOD", 0x000000d5u, 0x00000046u);
    resource_load_model("MALL02X.MOD", 0x000000d6u, 0x0000000fu);
    resource_load_model("PDOOR1.MOD", 0x000002ccu, 0x0000000fu);
    resource_load_model("PDOOR1X.MOD", 0x000002cdu, 0x0000000fu);
    resource_load_model("1CLOSED1.MOD", 0x000000f1u, 0x0000000fu);
    resource_load_model("1CLOSED2.MOD", 0x000000ddu, 0x0000000fu);
    resource_load_model("1CLOSED3.MOD", 0x000000e1u, 0x0000000fu);
    resource_load_model("1VENNET1.MOD", 0x000000d7u, 0x0000000fu);
    resource_load_model("1GEKO1.MOD", 0x000000d9u, 0x0000000fu);
    resource_load_model("1GEKO2.MOD", 0x000000dbu, 0x0000000fu);
    resource_load_model("1REVARD.MOD", 0x000000dfu, 0x0000000fu);
    resource_load_model("1TVZONE.MOD", 0x000000e7u, 0x0000000fu);
    resource_load_model("1PIZZA.MOD", 0x000000efu, 0x0000000fu);
    resource_load_model("1REVARD1.MOD", 0x000000e3u, 0x0000000fu);
    resource_load_model("1MUSICX1.MOD", 0x000000e5u, 0x0000000fu);
    resource_load_model("1PIZZA1.MOD", 0x000000e9u, 0x0000000fu);
    resource_load_model("1TVZONE1.MOD", 0x000000ebu, 0x0000000fu);
    resource_load_model("1NUKUT.MOD", 0x000000edu, 0x0000000fu);
    resource_load_model("1CLOSD1X.MOD", 0x000000f2u, 0x0000000fu);
    resource_load_model("1CLOSD2X.MOD", 0x000000deu, 0x0000000fu);
    resource_load_model("1CLOSD3X.MOD", 0x000000e2u, 0x0000000fu);
    resource_load_model("1VENET1X.MOD", 0x000000d8u, 0x0000000fu);
    resource_load_model("1GEKO1X.MOD", 0x000000dau, 0x0000000fu);
    resource_load_model("1GEKO2X.MOD", 0x000000dcu, 0x0000000fu);
    resource_load_model("1REVARDX.MOD", 0x000000e0u, 0x0000000fu);
    resource_load_model("1TVZONEX.MOD", 0x000000e8u, 0x0000000fu);
    resource_load_model("1PIZZAX.MOD", 0x000000f0u, 0x0000000fu);
    resource_load_model("1REVRD1X.MOD", 0x000000e4u, 0x0000000fu);
    resource_load_model("1MUSCX1X.MOD", 0x000000e6u, 0x0000000fu);
    resource_load_model("1PIZZA1X.MOD", 0x000000eau, 0x0000000fu);
    resource_load_model("1TVZOE1X.MOD", 0x000000ecu, 0x0000000fu);
    resource_load_model("1NUKUTX.MOD", 0x000000eeu, 0x0000000fu);
    resource_load_model("CANOPY.MOD", 0x000000f3u, 0x0000000fu);
    resource_load_model("CANOPY1.MOD", 0x000000f4u, 0x0000000fu);
    resource_load_model("HDOGSTND.MOD", 0x000002ceu, 0x00000046u);
    resource_load_model("HDOGSTDX.MOD", 0x000002cfu, 0x00000047u);
    resource_load_model("AIRCOND.MOD", 0x000002b6u, 0x00000046u);
    resource_load_model("CAFESEAT.MOD", 0x000002b7u, 0x00000046u);
    resource_load_model("CAFETABL.MOD", 0x000002b8u, 0x00000046u);
    resource_load_model("CAMERA.MOD", 0x000002b9u, 0x00000046u);
    resource_load_model("LIFTCON1.MOD", 0x000002bau, 0x00000046u);
    resource_load_model("PHONE1.MOD", 0x000002bbu, 0x00000046u);
    resource_load_model("PLANT1.MOD", 0x000002bcu, 0x00000046u);
    resource_load_model("PLANT2.MOD", 0x000002bdu, 0x00000046u);
    resource_load_model("SEATING1.MOD", 0x000002bfu, 0x00000046u);
    resource_load_model("WALLAMP.MOD", 0x000002c0u, 0x00000046u);
    resource_load_model("WALLAMP1.MOD", 0x000002c1u, 0x00000046u);
    resource_load_model("WALLAMP2.MOD", 0x000002c2u, 0x00000046u);
    resource_load_model("WALLAMP3.MOD", 0x000002c3u, 0x00000046u);
    resource_load_model("CAFSEAT1.MOD", 0x000002c4u, 0x00000046u);
    resource_load_model("CAFSEAT2.MOD", 0x000002c5u, 0x00000046u);
    resource_load_model("CAFSEAT3.MOD", 0x000002c6u, 0x00000046u);
    resource_load_model("LIFT1.MOD", 0x000002c7u, 0x00000046u);
    resource_load_model("PHONE2.MOD", 0x000002c8u, 0x00000046u);
    resource_load_model("CART1.MOD", 0x000002c9u, 0x00000046u);
    resource_load_model("CARSTAND.MOD", 0x000002cau, 0x00000046u);
    resource_load_model("CARSIGN1.MOD", 0x000002cbu, 0x00000046u);
    resource_load_model("MERCBOD1.MOD", 0x000002d0u, 0x00000046u);
    resource_load_model("MERCBOD2.MOD", 0x000002d1u, 0x00000046u);
    model0 = model_refs[721];
    model_refs[722] = model0;
    resource_load_model("MERCWHLS.MOD", 0x000002d3u, 0x00000046u);
    resource_load_model("MERCWHL.MOD", 0x00000212u, 0x00000046u);
    model1 = model_refs[530];
    model_refs[724] = model1;
    resource_load_model("CAR1BOD1.MOD", 0x0000005eu, 0x00000046u);
    resource_load_model("CAR1BOD2.MOD", 0x0000005fu, 0x00000046u);
    resource_load_model("CAR1BOD3.MOD", 0x00000060u, 0x00000046u);
    resource_load_model("CAR1WHL.MOD", 0x00000210u, 0x00000046u);
    resource_load_model("CAR1WHLS.MOD", 0x0000005du, 0x00000046u);
    resource_load_model("CAR1BON.MOD", 0x00000061u, 0x00000046u);
    resource_load_model("CAR1TRN.MOD", 0x00000062u, 0x00000046u);
    resource_load_model("CAR1EXP.MOD", 0x00000063u, 0x00000047u);
    resource_load_model("CAR1EDR.MOD", 0x00000065u, 0x00000047u);
    resource_load_model("CAR1EBN.MOD", 0x00000066u, 0x00000047u);
    resource_load_model("CAR1ENG.MOD", 0x00000215u, 0x00000047u);
    resource_load_model("CAR1ENG2.MOD", 0x00000216u, 0x00000047u);
    resource_load_model("CAR1EFB.MOD", 0x00000067u, 0x00000047u);
    resource_load_model("CAR1ETR.MOD", 0x00000068u, 0x00000047u);
    model2 = model_refs[99];
    model_refs[100] = model2;
    model3 = model_refs[528];
    model_refs[105] = model3;
    resource_load_model("SLIDOR1.MOD", 0x000001d3u, 0x00000046u);
    resource_load_model("SLIDOR2.MOD", 0x000001d4u, 0x00000046u);
    model4 = model_refs[467];
    model_refs[245] = model4;
    model5 = model_refs[468];
    model_refs[246] = model5;
    resource_load_model("SLIDOR3.MOD", 0x000001d5u, 0x00000046u);
    resource_load_model("SLIDOR4.MOD", 0x000001d6u, 0x00000046u);
    model6 = model_refs[469];
    model_refs[247] = model6;
    model7 = model_refs[470];
    model_refs[248] = model7;
    return;
}

static uint32 load_stage8_actors(void)
{
    FUNCTION_MARKER(0x800364acu, "GAME.EXE");
    uint32 scratch_context, dummy_context;
    scratch_context = resource_arenas.scratch.context;
    resource_load(0x0000005du, scratch_context, NULL, 0, NULL, &resource_arenas.scratch);
    RESOURCE_VIEW dummy = resource_find("DUMMY.DAT");
    dummy_context = dummy.context_address;
    {
        uintptr_t ptr = (uintptr_t)dummy.data, base = (uintptr_t)resource_storage.data;
        /* TODO: Investigate a DUMMY cursor outside its actual native resource allocation */
        XASSERT(ptr >= base && ptr - base <= RESOURCE_STORAGE_CAPACITY);
        resource_arenas.scratch = (RESOURCE_CURSOR){dummy.data, RESOURCE_STORAGE_CAPACITY - (uint32)(ptr - base), dummy_context};
    }
    resource_load_animations("GENMALE.ADT", 0xffffffffu);
    resource_load_animations("MONKEY.ADT", 0xffffffffu);
    resource_load_animations("CASBIKER.ADT", 0xffffffffu);
    resource_load_animations("BIKE.ADT", 0xffffffffu);
    resource_load_animations("TRAMP.ADT", 0xffffffffu);
    character_load_biker_variants();
    character_load_monkey();
    character_load_thugaa();
    character_load_thugbb();
    return character_load_tramp();
}

static uint32 load_stage8_models(void)
{
    MODEL_REFERENCE model0, model1;
    FUNCTION_MARKER(0x80036564u, "GAME.EXE");
    uint32 load_context, alloc_context, byte_len;
    load_context = resource_arenas.persistent.context;
    resource_load(0x00000031u, load_context, NULL, 0, NULL, &resource_arenas.persistent);
    alloc_context = resource_arenas.persistent.context;
    byte_len = resource_io.file_size;
    /* TODO: Investigate allocation beyond the actual native resource owner */
    XASSERT(resource_arenas.persistent.data && (byte_len) <= resource_arenas.persistent.available);
    resource_arenas.persistent.data += byte_len;
    resource_arenas.persistent.available -= byte_len;
    resource_arenas.persistent.context = alloc_context + byte_len;
    resource_load_model("NTREE01A.MOD", 0x000000f9u, 0x00000010u);
    resource_load_model("NTREE02A.MOD", 0x000000fau, 0x00000010u);
    resource_load_model("WATERW01.MOD", 0x000000fbu, 0x00000010u);
    resource_load_model("WATRW02A.MOD", 0x000000fcu, 0x00000010u);
    resource_load_model("WATRW03A.MOD", 0x000000fdu, 0x00000010u);
    resource_load_model("RAIL01.MOD", 0x000000feu, 0x00000010u);
    resource_load_model("RAIL02.MOD", 0x000000ffu, 0x00000010u);
    resource_load_model("RAIL03.MOD", 0x00000100u, 0x00000010u);
    resource_load_model("BUSH01.MOD", 0x00000101u, 0x00000010u);
    resource_load_model("BUSH02.MOD", 0x00000102u, 0x00000010u);
    resource_load_model("BUSH02A.MOD", 0x00000103u, 0x00000010u);
    resource_load_model("PARKLAMP.MOD", 0x00000104u, 0x00000010u);
    resource_load_model("MONU02.MOD", 0x00000105u, 0x00000010u);
    resource_load_model("RAIL01A.MOD", 0x00000106u, 0x00000010u);
    resource_load_model("PARKGATA.MOD", 0x00000107u, 0x00000010u);
    resource_load_model("DRNKFNTN.MOD", 0x000002d6u, 0x00000046u);
    model0 = model_refs[726];
    model_refs[608] = model0;
    resource_load_model("PRKSEATA.MOD", 0x000002d5u, 0x00000046u);
    resource_load_model("ROCK1.MOD", 0x0000021au, 0x00000046u);
    resource_load_model("ROCK2.MOD", 0x0000021bu, 0x00000046u);
    resource_load_model("ROCK3.MOD", 0x000001feu, 0x00000046u);
    resource_load_model("SIGNPOST.MOD", 0x000002d7u, 0x00000046u);
    resource_load_model("WASTEBIN.MOD", 0x000002d8u, 0x00000046u);
    model1 = model_refs[728];
    model_refs[731] = model1;
    resource_load_model("PKSEAT2A.MOD", 0x000002d9u, 0x00000046u);
    resource_load_model("BARRIER1.MOD", 0x000002dau, 0x00000046u);
    resource_load_model("MOTOWRK1.MOD", 0x00000213u, 0x00000047u);
    resource_load_model("MOTOWRK2.MOD", 0x00000208u, 0x00000047u);
    resource_load_model("MOTOWRK3.MOD", 0x00000214u, 0x00000047u);
    return resource_load_model("MOTOEXP.MOD", 0x00000244u, 0x00000047u);
}

static uint32 load_stage9_actors(void)
{
    FUNCTION_MARKER(0x80036804u, "GAME.EXE");
    uint32 scratch_context, dummy_context;
    scratch_context = resource_arenas.scratch.context;
    resource_load(0x0000005eu, scratch_context, NULL, 0, NULL, &resource_arenas.scratch);
    RESOURCE_VIEW dummy = resource_find("DUMMY.DAT");
    dummy_context = dummy.context_address;
    {
        uintptr_t ptr = (uintptr_t)dummy.data, base = (uintptr_t)resource_storage.data;
        /* TODO: Investigate a DUMMY cursor outside its actual native resource allocation */
        XASSERT(ptr >= base && ptr - base <= RESOURCE_STORAGE_CAPACITY);
        resource_arenas.scratch = (RESOURCE_CURSOR){dummy.data, RESOURCE_STORAGE_CAPACITY - (uint32)(ptr - base), dummy_context};
    }
    resource_load_animations("GENMALE.ADT", 0xffffffffu);
    resource_load_animations("MONKEY.ADT", 0xffffffffu);
    resource_load_animations("CASBIKER.ADT", 0xffffffffu);
    resource_load_animations("BIKE.ADT", 0xffffffffu);
    resource_load_animations("TRAMP.ADT", 0xffffffffu);
    character_load_monkey();
    character_load_thugaa();
    character_load_thugbb();
    character_load_biker_variants();
    return character_load_tramp();
}

static uint32 load_stage9_models(void)
{
    MODEL_REFERENCE model0, model1;
    FUNCTION_MARKER(0x800368bcu, "GAME.EXE");
    uint32 load_context, alloc_context, byte_len;
    load_context = resource_arenas.persistent.context;
    resource_load(0x00000031u, load_context, NULL, 0, NULL, &resource_arenas.persistent);
    alloc_context = resource_arenas.persistent.context;
    byte_len = resource_io.file_size;
    /* TODO: Investigate allocation beyond the actual native resource owner */
    XASSERT(resource_arenas.persistent.data && (byte_len) <= resource_arenas.persistent.available);
    resource_arenas.persistent.data += byte_len;
    resource_arenas.persistent.available -= byte_len;
    resource_arenas.persistent.context = alloc_context + byte_len;
    resource_load_model("NTREE01A.MOD", 0x000000f9u, 0x00000010u);
    resource_load_model("NTREE02A.MOD", 0x000000fau, 0x00000010u);
    resource_load_model("WATERW01.MOD", 0x000000fbu, 0x00000010u);
    resource_load_model("WATRW02A.MOD", 0x000000fcu, 0x00000010u);
    resource_load_model("WATRW03A.MOD", 0x000000fdu, 0x00000010u);
    resource_load_model("RAIL01.MOD", 0x000000feu, 0x00000010u);
    resource_load_model("RAIL02.MOD", 0x000000ffu, 0x00000010u);
    resource_load_model("RAIL03.MOD", 0x00000100u, 0x00000010u);
    resource_load_model("BUSH01.MOD", 0x00000101u, 0x00000010u);
    resource_load_model("BUSH02.MOD", 0x00000102u, 0x00000010u);
    resource_load_model("BUSH02A.MOD", 0x00000103u, 0x00000010u);
    resource_load_model("PARKLAMP.MOD", 0x00000104u, 0x00000010u);
    resource_load_model("MONU02.MOD", 0x00000105u, 0x00000010u);
    resource_load_model("RAIL01A.MOD", 0x00000106u, 0x00000010u);
    resource_load_model("PARKGATA.MOD", 0x00000107u, 0x00000010u);
    resource_load_model("DRNKFNTN.MOD", 0x000002d6u, 0x00000046u);
    model0 = model_refs[726];
    model_refs[608] = model0;
    resource_load_model("PRKSEATA.MOD", 0x000002d5u, 0x00000046u);
    resource_load_model("ROCK1.MOD", 0x0000021au, 0x00000046u);
    resource_load_model("ROCK2.MOD", 0x0000021bu, 0x00000046u);
    resource_load_model("ROCK3.MOD", 0x000001feu, 0x00000046u);
    resource_load_model("SIGNPOST.MOD", 0x000002d7u, 0x00000046u);
    resource_load_model("WASTEBIN.MOD", 0x000002d8u, 0x00000046u);
    model1 = model_refs[728];
    model_refs[731] = model1;
    resource_load_model("PKSEAT2A.MOD", 0x000002d9u, 0x00000046u);
    resource_load_model("BARRIER1.MOD", 0x000002dau, 0x00000046u);
    resource_load_model("MOTOWRK1.MOD", 0x00000213u, 0x00000047u);
    resource_load_model("MOTOWRK2.MOD", 0x00000208u, 0x00000047u);
    resource_load_model("MOTOWRK3.MOD", 0x00000214u, 0x00000047u);
    return resource_load_model("MOTOEXP.MOD", 0x00000244u, 0x00000047u);
}

static uint32 load_stage10_actors(void)
{
    FUNCTION_MARKER(0x80036b5cu, "GAME.EXE");
    uint32 scratch_context, dummy_context;
    scratch_context = resource_arenas.scratch.context;
    resource_load(0x0000005fu, scratch_context, NULL, 0, NULL, &resource_arenas.scratch);
    RESOURCE_VIEW dummy = resource_find("DUMMY.DAT");
    dummy_context = dummy.context_address;
    {
        uintptr_t ptr = (uintptr_t)dummy.data, base = (uintptr_t)resource_storage.data;
        /* TODO: Investigate a DUMMY cursor outside its actual native resource allocation */
        XASSERT(ptr >= base && ptr - base <= RESOURCE_STORAGE_CAPACITY);
        resource_arenas.scratch = (RESOURCE_CURSOR){dummy.data, RESOURCE_STORAGE_CAPACITY - (uint32)(ptr - base), dummy_context};
    }
    resource_load_animations("GENMALE.ADT", 0xffffffffu);
    resource_load_animations("GENFMALE.ADT", 0xfffffffeu);
    character_load_badbabe();
    return character_load_xom_variants();
}

static void load_stage10_models(void)
{
    MODEL_REFERENCE model0, model1, model2;
    FUNCTION_MARKER(0x80036bccu, "GAME.EXE");
    uint32 load_context, alloc_context, byte_len;
    load_context = resource_arenas.persistent.context;
    resource_load(0x00000032u, load_context, NULL, 0, NULL, &resource_arenas.persistent);
    alloc_context = resource_arenas.persistent.context;
    byte_len = resource_io.file_size;
    /* TODO: Investigate allocation beyond the actual native resource owner */
    XASSERT(resource_arenas.persistent.data && (byte_len) <= resource_arenas.persistent.available);
    resource_arenas.persistent.data += byte_len;
    resource_arenas.persistent.available -= byte_len;
    resource_arenas.persistent.context = alloc_context + byte_len;
    resource_load_model("B01A.MOD", 0x00000108u, 0x00000011u);
    resource_load_model("B02X.MOD", 0x00000109u, 0x00000011u);
    resource_load_model("B02Y.MOD", 0x0000010au, 0x00000011u);
    resource_load_model("B03B.MOD", 0x0000010bu, 0x00000011u);
    resource_load_model("LIGHTLEF.MOD", 0x000002dcu, 0x00000011u);
    resource_load_model("LIGHTRIG.MOD", 0x000002ddu, 0x00000011u);
    resource_load_model("SIGN01.MOD", 0x000002deu, 0x00000011u);
    resource_load_model("FUELDRUM.MOD", 0x0000021fu, 0x00000046u);
    resource_load_model("GIRDER2.MOD", 0x0000021cu, 0x00000046u);
    resource_load_model("SLAB1.MOD", 0x0000022du, 0x00000046u);
    resource_load_model("SLAB3.MOD", 0x0000022fu, 0x00000046u);
    resource_load_model("SLAB4.MOD", 0x00000230u, 0x00000046u);
    resource_load_model("CRATE1.MOD", 0x000002dfu, 0x00000046u);
    resource_load_model("CRATE2.MOD", 0x000002e0u, 0x00000046u);
    model0 = model_refs[735];
    model_refs[608] = model0;
    resource_load_model("BUSBROKE.MOD", 0x0000010du, 0x00000046u);
    resource_load_model("BUSBRKE2.MOD", 0x0000010eu, 0x00000047u);
    resource_load_model("BARRIER1.MOD", 0x000002e1u, 0x00000046u);
    resource_load_model("HELCOP.MOD", 0x0000010cu, 0x00000046u);
    resource_load_model("ROTOR.MOD", 0x00000234u, 0x00000047u);
    resource_load_model("WRECK2.MOD", 0x000002e2u, 0x00000046u);
    resource_load_model("CAR1BOD2.MOD", 0x0000005fu, 0x00000046u);
    resource_load_model("CAR1BOD3.MOD", 0x00000060u, 0x00000046u);
    resource_load_model("CAR1WHL.MOD", 0x00000210u, 0x00000046u);
    resource_load_model("CAR1WHLS.MOD", 0x0000005du, 0x00000046u);
    resource_load_model("CAR1BON.MOD", 0x00000061u, 0x00000046u);
    resource_load_model("CAR1TRN.MOD", 0x00000062u, 0x00000046u);
    resource_load_model("CAR1EXP.MOD", 0x00000063u, 0x00000047u);
    resource_load_model("CAR1EDR.MOD", 0x00000065u, 0x00000047u);
    resource_load_model("CAR1EBN.MOD", 0x00000066u, 0x00000047u);
    resource_load_model("CAR1ENG.MOD", 0x00000215u, 0x00000047u);
    resource_load_model("CAR1ENG2.MOD", 0x00000216u, 0x00000047u);
    resource_load_model("CAR1EFB.MOD", 0x00000067u, 0x00000047u);
    resource_load_model("CAR1ETR.MOD", 0x00000068u, 0x00000047u);
    model1 = model_refs[99];
    model_refs[100] = model1;
    model2 = model_refs[528];
    model_refs[105] = model2;
    return;
}

static uint32 load_stage11_actors(void)
{
    FUNCTION_MARKER(0x80036edcu, "GAME.EXE");
    uint32 scratch_context, dummy_context;
    scratch_context = resource_arenas.scratch.context;
    resource_load(0x00000060u, scratch_context, NULL, 0, NULL, &resource_arenas.scratch);
    RESOURCE_VIEW dummy = resource_find("DUMMY.DAT");
    dummy_context = dummy.context_address;
    {
        uintptr_t ptr = (uintptr_t)dummy.data, base = (uintptr_t)resource_storage.data;
        /* TODO: Investigate a DUMMY cursor outside its actual native resource allocation */
        XASSERT(ptr >= base && ptr - base <= RESOURCE_STORAGE_CAPACITY);
        resource_arenas.scratch = (RESOURCE_CURSOR){dummy.data, RESOURCE_STORAGE_CAPACITY - (uint32)(ptr - base), dummy_context};
    }
    resource_load_animations("GENMALE.ADT", 0xffffffffu);
    character_load_xom_variants();
    character_load_thug1();
    return character_load_afrotg();
}

static void load_stage11_models(void)
{
    MODEL_REFERENCE model0, model1, model2;
    FUNCTION_MARKER(0x80036f44u, "GAME.EXE");
    uint32 load_context, alloc_context, byte_len;
    load_context = resource_arenas.persistent.context;
    resource_load(0x00000033u, load_context, NULL, 0, NULL, &resource_arenas.persistent);
    alloc_context = resource_arenas.persistent.context;
    byte_len = resource_io.file_size;
    /* TODO: Investigate allocation beyond the actual native resource owner */
    XASSERT(resource_arenas.persistent.data && (byte_len) <= resource_arenas.persistent.available);
    resource_arenas.persistent.data += byte_len;
    resource_arenas.persistent.available -= byte_len;
    resource_arenas.persistent.context = alloc_context + byte_len;
    resource_load_model("BR01.MOD", 0x0000010fu, 0x00000012u);
    resource_load_model("BR02A.MOD", 0x00000110u, 0x00000012u);
    resource_load_model("BR03A.MOD", 0x00000111u, 0x00000012u);
    resource_load_model("BR04A.MOD", 0x00000112u, 0x00000012u);
    resource_load_model("BR05.MOD", 0x00000113u, 0x00000012u);
    resource_load_model("TRAINNGU.MOD", 0x000002e3u, 0x00000046u);
    resource_load_model("CRATE1.MOD", 0x000002ecu, 0x00000046u);
    resource_load_model("CRATE2.MOD", 0x0000021du, 0x00000046u);
    resource_load_model("CRATE3.MOD", 0x0000021eu, 0x00000046u);
    model0 = model_refs[748];
    model_refs[608] = model0;
    model_refs[609] = model0;
    resource_load_model("PIMPCAR1.MOD", 0x000002e4u, 0x00000046u);
    resource_load_model("SLAB1.MOD", 0x0000022du, 0x00000046u);
    resource_load_model("SLAB2.MOD", 0x0000022eu, 0x00000046u);
    resource_load_model("SLAB3.MOD", 0x0000022fu, 0x00000046u);
    resource_load_model("SLAB4.MOD", 0x00000230u, 0x00000046u);
    resource_load_model("PHONE1.MOD", 0x000002e5u, 0x00000046u);
    resource_load_model("AIRCOND.MOD", 0x000002e6u, 0x00000046u);
    resource_load_model("AIRPIPE1.MOD", 0x000002e7u, 0x00000046u);
    resource_load_model("WALLPIPE.MOD", 0x000002e8u, 0x00000046u);
    resource_load_model("FUELDRUM.MOD", 0x0000021fu, 0x00000046u);
    resource_load_model("FUELDRM2.MOD", 0x000002e9u, 0x00000046u);
    resource_load_model("FUELDRM3.MOD", 0x000002eau, 0x00000046u);
    resource_load_model("WALPIPE2.MOD", 0x000002ebu, 0x00000046u);
    resource_load_model("BR04B.MOD", 0x00000114u, 0x00000012u);
    resource_load_model("BR04AA.MOD", 0x00000115u, 0x00000012u);
    resource_load_model("POLICAR.MOD", 0x000002edu, 0x00000046u);
    resource_load_model("STRTLAMP.MOD", 0x000002eeu, 0x00000046u);
    resource_load_model("STRTLMP2.MOD", 0x000002efu, 0x00000046u);
    resource_load_model("AIRCOND2.MOD", 0x000002f0u, 0x00000046u);
    resource_load_model("AIRPIPE2.MOD", 0x000002f1u, 0x00000046u);
    resource_load_model("TRUCK1.MOD", 0x000002f2u, 0x00000046u);
    resource_load_model("ST33GATE.MOD", 0x000002f3u, 0x00000012u);
    resource_load_model("ST33LAMP.MOD", 0x000002f4u, 0x00000046u);
    resource_load_model("ST33LMP2.MOD", 0x000002f5u, 0x00000046u);
    resource_load_model("CAR1BOD1.MOD", 0x0000005eu, 0x00000046u);
    resource_load_model("CAR1BOD2.MOD", 0x0000005fu, 0x00000046u);
    resource_load_model("CAR1BOD3.MOD", 0x00000060u, 0x00000046u);
    resource_load_model("CAR1WHL.MOD", 0x00000210u, 0x00000046u);
    resource_load_model("CAR1WHLS.MOD", 0x0000005du, 0x00000046u);
    resource_load_model("CAR1BON.MOD", 0x00000061u, 0x00000046u);
    resource_load_model("CAR1TRN.MOD", 0x00000062u, 0x00000046u);
    resource_load_model("CAR1EXP.MOD", 0x00000063u, 0x00000047u);
    resource_load_model("CAR1EDR.MOD", 0x00000065u, 0x00000047u);
    resource_load_model("CAR1EBN.MOD", 0x00000066u, 0x00000047u);
    resource_load_model("CAR1ENG.MOD", 0x00000215u, 0x00000047u);
    resource_load_model("CAR1ENG2.MOD", 0x00000216u, 0x00000047u);
    resource_load_model("CAR1EFB.MOD", 0x00000067u, 0x00000047u);
    resource_load_model("CAR1ETR.MOD", 0x00000068u, 0x00000047u);
    resource_load_model("WALFRG1.MOD", 0x000002f6u, 0x00000012u);
    resource_load_model("WALFRG2.MOD", 0x000002f7u, 0x00000012u);
    resource_load_model("WALFRG3.MOD", 0x000002f8u, 0x00000012u);
    resource_load_model("WALFRG4.MOD", 0x000002f9u, 0x00000012u);
    resource_load_model("WALFRG5.MOD", 0x000002fau, 0x00000012u);
    resource_load_model("WALFRG6.MOD", 0x000002fbu, 0x00000012u);
    resource_load_model("WALFRG7.MOD", 0x000002fcu, 0x00000012u);
    resource_load_model("WALFRG8.MOD", 0x000002fdu, 0x00000012u);
    resource_load_model("WALFRG9.MOD", 0x000002feu, 0x00000012u);
    resource_load_model("WALFRG10.MOD", 0x000002ffu, 0x00000012u);
    resource_load_model("WALFRG11.MOD", 0x00000300u, 0x00000012u);
    resource_load_model("WALFRG12.MOD", 0x00000301u, 0x00000012u);
    model1 = model_refs[99];
    model_refs[100] = model1;
    model2 = model_refs[528];
    model_refs[105] = model2;
    return;
}

static uint32 load_stage12_actors(void)
{
    FUNCTION_MARKER(0x80037460u, "GAME.EXE");
    uint32 scratch_context, dummy_context;
    scratch_context = resource_arenas.scratch.context;
    resource_load(0x00000061u, scratch_context, NULL, 0, NULL, &resource_arenas.scratch);
    RESOURCE_VIEW dummy = resource_find("DUMMY.DAT");
    dummy_context = dummy.context_address;
    {
        uintptr_t ptr = (uintptr_t)dummy.data, base = (uintptr_t)resource_storage.data;
        /* TODO: Investigate a DUMMY cursor outside its actual native resource allocation */
        XASSERT(ptr >= base && ptr - base <= RESOURCE_STORAGE_CAPACITY);
        resource_arenas.scratch = (RESOURCE_CURSOR){dummy.data, RESOURCE_STORAGE_CAPACITY - (uint32)(ptr - base), dummy_context};
    }
    resource_load_animations("GENMALE.ADT", 0xffffffffu);
    character_load_suit_variants();
    character_load_thug3();
    character_load_afrotg2();
    return character_load_afrotg3();
}

static uint32 load_stage12_models(void)
{
    FUNCTION_MARKER(0x800374d0u, "GAME.EXE");
    uint32 load_context, alloc_context, byte_len;
    load_context = resource_arenas.persistent.context;
    resource_load(0x00000034u, load_context, NULL, 0, NULL, &resource_arenas.persistent);
    alloc_context = resource_arenas.persistent.context;
    byte_len = resource_io.file_size;
    /* TODO: Investigate allocation beyond the actual native resource owner */
    XASSERT(resource_arenas.persistent.data && (byte_len) <= resource_arenas.persistent.available);
    resource_arenas.persistent.data += byte_len;
    resource_arenas.persistent.available -= byte_len;
    resource_arenas.persistent.context = alloc_context + byte_len;
    resource_load_model("SUBMAINL.MOD", 0x00000116u, 0x00000013u);
    resource_load_model("SUBMAINR.MOD", 0x00000117u, 0x00000013u);
    resource_load_model("SUBESC.MOD", 0x00000118u, 0x00000013u);
    resource_load_model("TRAIN1.MOD", 0x0000012eu, 0x00000046u);
    resource_load_model("TRAIN2.MOD", 0x0000012fu, 0x00000046u);
    resource_load_model("TRAIN3.MOD", 0x00000130u, 0x00000046u);
    resource_load_model("SUBRAIL1.MOD", 0x00000119u, 0x00000013u);
    resource_load_model("TICKET.MOD", 0x0000011au, 0x00000013u);
    resource_load_model("TICKET1.MOD", 0x0000011bu, 0x00000013u);
    resource_load_model("PILLAR.MOD", 0x0000011cu, 0x00000013u);
    resource_load_model("CAMERA.MOD", 0x0000011du, 0x00000046u);
    resource_load_model("INFOTV1.MOD", 0x0000011eu, 0x00000046u);
    resource_load_model("LUGAGE1.MOD", 0x00000221u, 0x00000046u);
    resource_load_model("LUGAGE2.MOD", 0x00000222u, 0x00000046u);
    resource_load_model("LUGAGE3.MOD", 0x00000225u, 0x00000046u);
    resource_load_model("LUGAGE4.MOD", 0x00000226u, 0x00000046u);
    resource_load_model("LUGAGE5.MOD", 0x00000223u, 0x00000046u);
    resource_load_model("LUGAGE6.MOD", 0x00000227u, 0x00000046u);
    resource_load_model("LUGAGE7.MOD", 0x00000228u, 0x00000046u);
    resource_load_model("LUGAGE8.MOD", 0x00000224u, 0x00000046u);
    resource_load_model("PHONE1.MOD", 0x0000011fu, 0x00000046u);
    resource_load_model("TANNOY.MOD", 0x00000120u, 0x00000046u);
    resource_load_model("TICKMACH.MOD", 0x00000121u, 0x00000046u);
    resource_load_model("WARNCONE.MOD", 0x00000231u, 0x00000046u);
    resource_load_model("CAMERA1.MOD", 0x00000122u, 0x00000046u);
    resource_load_model("PHONE2.MOD", 0x00000123u, 0x00000046u);
    resource_load_model("INFOTV2.MOD", 0x00000124u, 0x00000046u);
    resource_load_model("SUBENT1.MOD", 0x00000125u, 0x00000013u);
    resource_load_model("SUBESC2.MOD", 0x00000126u, 0x00000013u);
    resource_load_model("ELECKART.MOD", 0x00000127u, 0x00000046u);
    resource_load_model("ELKRTEXP.MOD", 0x00000128u, 0x00000046u);
    resource_load_model("TRAILER1.MOD", 0x00000129u, 0x00000046u);
    resource_load_model("TRAILERX.MOD", 0x0000012au, 0x00000046u);
    resource_load_model("TURNSTLE.MOD", 0x0000012bu, 0x00000046u);
    resource_load_model("TURNSTL2.MOD", 0x0000012cu, 0x00000046u);
    resource_load_model("TURNSTLX.MOD", 0x0000012du, 0x00000046u);
    resource_load_model("TOKEN.MOD", 0x00000202u, 0x00000046u);
    resource_load_model("DRNKMCH2.MOD", 0x00000282u, 0x00000046u);
    resource_load_model("DRKMCH2A.MOD", 0x00000283u, 0x00000046u);
    resource_load_model("DRKMCH2B.MOD", 0x00000284u, 0x00000046u);
    resource_load_model("DRKMCH2C.MOD", 0x00000285u, 0x00000046u);
    resource_load_model("DRKMCH2D.MOD", 0x00000286u, 0x00000047u);
    resource_load_model("SODACAN.MOD", 0x00000200u, 0x00000046u);
    resource_load_model("SODACAN2.MOD", 0x00000201u, 0x00000046u);
    resource_load_model("BENCH1.MOD", 0x00000131u, 0x00000046u);
    resource_load_model("BENCH1X.MOD", 0x00000132u, 0x00000046u);
    return resource_load_model("CRATE1.MOD", 0x00000260u, 0x00000046u);
}

static uint32 load_stage13_actors(void)
{
    FUNCTION_MARKER(0x800378c4u, "GAME.EXE");
    uint32 scratch_context, dummy_context;
    scratch_context = resource_arenas.scratch.context;
    resource_load(0x00000062u, scratch_context, NULL, 0, NULL, &resource_arenas.scratch);
    RESOURCE_VIEW dummy = resource_find("DUMMY.DAT");
    dummy_context = dummy.context_address;
    {
        uintptr_t ptr = (uintptr_t)dummy.data, base = (uintptr_t)resource_storage.data;
        /* TODO: Investigate a DUMMY cursor outside its actual native resource allocation */
        XASSERT(ptr >= base && ptr - base <= RESOURCE_STORAGE_CAPACITY);
        resource_arenas.scratch = (RESOURCE_CURSOR){dummy.data, RESOURCE_STORAGE_CAPACITY - (uint32)(ptr - base), dummy_context};
    }
    return character_load_exc();
}

static uint32 load_stage14_actors(void)
{
    FUNCTION_MARKER(0x8003790cu, "GAME.EXE");
    uint32 scratch_context, dummy_context;
    scratch_context = resource_arenas.scratch.context;
    resource_load(0x00000063u, scratch_context, NULL, 0, NULL, &resource_arenas.scratch);
    RESOURCE_VIEW dummy = resource_find("DUMMY.DAT");
    dummy_context = dummy.context_address;
    {
        uintptr_t ptr = (uintptr_t)dummy.data, base = (uintptr_t)resource_storage.data;
        /* TODO: Investigate a DUMMY cursor outside its actual native resource allocation */
        XASSERT(ptr >= base && ptr - base <= RESOURCE_STORAGE_CAPACITY);
        resource_arenas.scratch = (RESOURCE_CURSOR){dummy.data, RESOURCE_STORAGE_CAPACITY - (uint32)(ptr - base), dummy_context};
    }
    resource_load_animations("GENMALE.ADT", 0xffffffffu);
    resource_load_animations("GENFMALE.ADT", 0xfffffffeu);
    resource_load_animations("TRAMP.ADT", 0xffffffffu);
    character_load_tramp();
    character_load_badbabe();
    character_load_thugaa();
    return character_load_thugbb();
}

static void load_stage14_models(void)
{
    MODEL_REFERENCE model0;
    FUNCTION_MARKER(0x8003799cu, "GAME.EXE");
    uint32 load_context, alloc_context, byte_len;
    load_context = resource_arenas.persistent.context;
    resource_load(0x00000036u, load_context, NULL, 0, NULL, &resource_arenas.persistent);
    alloc_context = resource_arenas.persistent.context;
    byte_len = resource_io.file_size;
    /* TODO: Investigate allocation beyond the actual native resource owner */
    XASSERT(resource_arenas.persistent.data && (byte_len) <= resource_arenas.persistent.available);
    resource_arenas.persistent.data += byte_len;
    resource_arenas.persistent.available -= byte_len;
    resource_arenas.persistent.context = alloc_context + byte_len;
    resource_load_model("TRAIN01T.MOD", 0x00000133u, 0x00000014u);
    resource_load_model("TRAIN02T.MOD", 0x00000135u, 0x00000014u);
    resource_load_model("TRAIN03T.MOD", 0x00000137u, 0x00000014u);
    resource_load_model("TRAIN04T.MOD", 0x00000139u, 0x00000014u);
    resource_load_model("TRAIN01R.MOD", 0x00000134u, 0x00000014u);
    resource_load_model("TRAIN02R.MOD", 0x00000136u, 0x00000014u);
    resource_load_model("TRAIN03R.MOD", 0x00000138u, 0x00000014u);
    resource_load_model("TRAIN04R.MOD", 0x0000013au, 0x00000014u);
    resource_load_model("SWALL01.MOD", 0x0000013bu, 0x00000014u);
    resource_load_model("SWALL02.MOD", 0x0000013cu, 0x00000014u);
    resource_load_model("LUGAGE1.MOD", 0x00000221u, 0x00000046u);
    resource_load_model("LUGAGE2.MOD", 0x00000222u, 0x00000046u);
    resource_load_model("LUGAGE3.MOD", 0x00000225u, 0x00000046u);
    resource_load_model("LUGAGE4.MOD", 0x00000226u, 0x00000046u);
    resource_load_model("CRATE1.MOD", 0x00000146u, 0x00000046u);
    model0 = model_refs[326];
    model_refs[608] = model0;
    model_refs[612] = model0;
    model_refs[610] = model0;
    return;
}

static uint32 load_stage15_actors(void)
{
    FUNCTION_MARKER(0x80037b28u, "GAME.EXE");
    uint32 scratch_context, dummy_context;
    scratch_context = resource_arenas.scratch.context;
    resource_load(0x00000064u, scratch_context, NULL, 0, NULL, &resource_arenas.scratch);
    RESOURCE_VIEW dummy = resource_find("DUMMY.DAT");
    dummy_context = dummy.context_address;
    {
        uintptr_t ptr = (uintptr_t)dummy.data, base = (uintptr_t)resource_storage.data;
        /* TODO: Investigate a DUMMY cursor outside its actual native resource allocation */
        XASSERT(ptr >= base && ptr - base <= RESOURCE_STORAGE_CAPACITY);
        resource_arenas.scratch = (RESOURCE_CURSOR){dummy.data, RESOURCE_STORAGE_CAPACITY - (uint32)(ptr - base), dummy_context};
    }
    resource_load_animations("GENMALE.ADT", 0xffffffffu);
    resource_load_animations("GENFMALE.ADT", 0xfffffffeu);
    character_load_femsol();
    character_load_soldier_variants();
    return character_load_sas();
}

static uint32 load_airbase_common_models(void)
{
    MODEL_REFERENCE model0, model1;
    FUNCTION_MARKER(0x80037ba0u, "GAME.EXE");
    resource_load_model("FENCE01.MOD", 0x00000147u, 0x00000015u);
    resource_load_model("FENCE02.MOD", 0x00000148u, 0x00000015u);
    resource_load_model("FENCE03.MOD", 0x00000149u, 0x00000015u);
    resource_load_model("FENCE04.MOD", 0x0000014au, 0x00000015u);
    resource_load_model("FENCE05.MOD", 0x0000014bu, 0x00000015u);
    resource_load_model("FENCE09.MOD", 0x0000014cu, 0x00000015u);
    resource_load_model("HANGER01.MOD", 0x0000014du, 0x00000015u);
    resource_load_model("HANGER02.MOD", 0x0000014eu, 0x00000015u);
    resource_load_model("HANGER03.MOD", 0x0000014fu, 0x00000015u);
    resource_load_model("HANGER04.MOD", 0x00000150u, 0x00000015u);
    resource_load_model("HANGER05.MOD", 0x00000151u, 0x00000015u);
    resource_load_model("HANGER06.MOD", 0x00000152u, 0x00000015u);
    resource_load_model("WALL01.MOD", 0x00000153u, 0x00000015u);
    resource_load_model("CONTRL01.MOD", 0x00000154u, 0x00000015u);
    resource_load_model("FUELDRUM.MOD", 0x00000220u, 0x00000046u);
    resource_load_model("FUELDRM2.MOD", 0x00000302u, 0x00000046u);
    resource_load_model("HUMVEE.MOD", 0x00000303u, 0x00000046u);
    resource_load_model("FORKLIFT.MOD", 0x00000304u, 0x00000046u);
    resource_load_model("CRATE1.MOD", 0x00000305u, 0x00000046u);
    resource_load_model("CRATE2.MOD", 0x00000306u, 0x00000046u);
    resource_load_model("CRATE3.MOD", 0x00000307u, 0x00000046u);
    resource_load_model("TANK3.MOD", 0x00000308u, 0x00000046u);
    resource_load_model("MISTURET.MOD", 0x00000309u, 0x00000046u);
    resource_load_model("JEEP.MOD", 0x0000030au, 0x00000046u);
    resource_load_model("JEEPEXP.MOD", 0x0000030bu, 0x00000047u);
    model0 = model_refs[774];
    model_refs[608] = model0;
    resource_load_model("ARMYTRK2.MOD", 0x0000030du, 0x00000046u);
    resource_load_model("ARMTRK2X.MOD", 0x0000030eu, 0x00000047u);
    resource_load_model("A10A.MOD", 0x00000310u, 0x00000046u);
    resource_load_model("A10B.MOD", 0x0000030fu, 0x00000046u);
    resource_load_model("A10EXP.MOD", 0x00000311u, 0x00000046u);
    resource_load_model("SECDOOR1.MOD", 0x0000015du, 0x00000015u);
    resource_load_model("SDFRAG1.MOD", 0x0000015eu, 0x00000015u);
    resource_load_model("SDFRAG2.MOD", 0x0000015fu, 0x00000015u);
    resource_load_model("SDFRAG3.MOD", 0x00000160u, 0x00000015u);
    resource_load_model("SDFRAG4.MOD", 0x00000161u, 0x00000015u);
    resource_load_model("SDFRAG5.MOD", 0x00000162u, 0x00000015u);
    resource_load_model("SDFRAG6.MOD", 0x00000163u, 0x00000015u);
    resource_load_model("SDFRAG7.MOD", 0x00000164u, 0x00000015u);
    resource_load_model("SDFRAG8.MOD", 0x00000165u, 0x00000015u);
    resource_load_model("SDFRAG9.MOD", 0x00000166u, 0x00000015u);
    resource_load_model("BLAKHAWK.MOD", 0x00000155u, 0x00000046u);
    resource_load_model("ROTOR.MOD", 0x00000234u, 0x00000047u);
    resource_load_model("HELISHAD.MOD", 0x00000342u, 0x00000046u);
    resource_load_model("GATE01.MOD", 0x00000046u, 0x00000015u);
    resource_load_model("GATE02.MOD", 0x00000047u, 0x00000015u);
    resource_load_model("GATE03.MOD", 0x00000048u, 0x00000015u);
    resource_load_model("GATE04.MOD", 0x00000049u, 0x00000015u);
    resource_load_model("GATE05.MOD", 0x0000004au, 0x00000015u);
    resource_load_model("GATE06.MOD", 0x0000004bu, 0x00000015u);
    resource_load_model("GATE07.MOD", 0x0000004cu, 0x00000015u);
    model1 = model_refs[75];
    model_refs[77] = model1;
    resource_load_model("GARDTOWR.MOD", 0x00000156u, 0x00000015u);
    resource_load_model("FENCE06.MOD", 0x00000157u, 0x00000015u);
    resource_load_model("GARDTWR2.MOD", 0x00000158u, 0x00000015u);
    resource_load_model("WALL02.MOD", 0x00000159u, 0x00000015u);
    resource_load_model("GARDBAR1.MOD", 0x0000015au, 0x00000046u);
    resource_load_model("GARDBAR2.MOD", 0x0000015bu, 0x00000046u);
    resource_load_model("AWAC1.MOD", 0x00000167u, 0x00000046u);
    return resource_load_model("WALL03.MOD", 0x00000168u, 0x00000015u);
}

static uint32 load_stage15_models(void)
{
    FUNCTION_MARKER(0x80038068u, "GAME.EXE");
    uint32 load_context, alloc_context, byte_len;
    load_context = resource_arenas.persistent.context;
    resource_load(0x00000037u, load_context, NULL, 0, NULL, &resource_arenas.persistent);
    alloc_context = resource_arenas.persistent.context;
    byte_len = resource_io.file_size;
    /* TODO: Investigate allocation beyond the actual native resource owner */
    XASSERT(resource_arenas.persistent.data && (byte_len) <= resource_arenas.persistent.available);
    resource_arenas.persistent.data += byte_len;
    resource_arenas.persistent.available -= byte_len;
    resource_arenas.persistent.context = alloc_context + byte_len;
    return load_airbase_common_models();
}

static uint32 load_stage16_actors(void)
{
    FUNCTION_MARKER(0x800380b8u, "GAME.EXE");
    uint32 scratch_context, dummy_context;
    scratch_context = resource_arenas.scratch.context;
    resource_load(0x00000065u, scratch_context, NULL, 0, NULL, &resource_arenas.scratch);
    RESOURCE_VIEW dummy = resource_find("DUMMY.DAT");
    dummy_context = dummy.context_address;
    {
        uintptr_t ptr = (uintptr_t)dummy.data, base = (uintptr_t)resource_storage.data;
        /* TODO: Investigate a DUMMY cursor outside its actual native resource allocation */
        XASSERT(ptr >= base && ptr - base <= RESOURCE_STORAGE_CAPACITY);
        resource_arenas.scratch = (RESOURCE_CURSOR){dummy.data, RESOURCE_STORAGE_CAPACITY - (uint32)(ptr - base), dummy_context};
    }
    resource_load_animations("GENMALE.ADT", 0xffffffffu);
    character_load_soldier_variants();
    character_load_security();
    return character_load_orange();
}

static uint32 load_stage16_models(void)
{
    MODEL_REFERENCE model0;
    FUNCTION_MARKER(0x80038120u, "GAME.EXE");
    uint32 load_context, alloc_context, byte_len;
    load_context = resource_arenas.persistent.context;
    resource_load(0x00000038u, load_context, NULL, 0, NULL, &resource_arenas.persistent);
    alloc_context = resource_arenas.persistent.context;
    byte_len = resource_io.file_size;
    /* TODO: Investigate allocation beyond the actual native resource owner */
    XASSERT(resource_arenas.persistent.data && (byte_len) <= resource_arenas.persistent.available);
    resource_arenas.persistent.data += byte_len;
    resource_arenas.persistent.available -= byte_len;
    resource_arenas.persistent.context = alloc_context + byte_len;
    resource_load_model("CAVEROOF.MOD", 0x00000169u, 0x00000016u);
    resource_load_model("NAVBACK1.MOD", 0x0000016au, 0x00000016u);
    resource_load_model("NAVEL03.MOD", 0x0000016bu, 0x00000016u);
    resource_load_model("NAVEL04.MOD", 0x0000016cu, 0x00000016u);
    resource_load_model("CRATE1.MOD", 0x0000016fu, 0x00000046u);
    resource_load_model("FUELDRUM.MOD", 0x00000220u, 0x00000046u);
    resource_load_model("FUELDRM2.MOD", 0x00000229u, 0x00000046u);
    resource_load_model("JEEP2.MOD", 0x00000170u, 0x00000046u);
    resource_load_model("LOAD1.MOD", 0x00000171u, 0x00000046u);
    resource_load_model("LOAD2.MOD", 0x00000172u, 0x00000046u);
    model0 = model_refs[367];
    model_refs[608] = model0;
    resource_load_model("FUELTANK.MOD", 0x00000173u, 0x00000046u);
    resource_load_model("FUELTNKX.MOD", 0x00000174u, 0x00000047u);
    resource_load_model("MACHNRY1.MOD", 0x00000175u, 0x00000046u);
    resource_load_model("MCHNRY1X.MOD", 0x00000176u, 0x00000047u);
    resource_load_model("MACHNRY2.MOD", 0x00000177u, 0x00000046u);
    resource_load_model("MCHNRY2X.MOD", 0x00000178u, 0x00000047u);
    resource_load_model("MACHNRY3.MOD", 0x00000179u, 0x00000046u);
    resource_load_model("MCHNRY3X.MOD", 0x0000017au, 0x00000047u);
    resource_load_model("MACHNRY4.MOD", 0x0000017bu, 0x00000046u);
    resource_load_model("MCHNRY4X.MOD", 0x0000017cu, 0x00000047u);
    resource_load_model("WORKTABL.MOD", 0x0000017du, 0x00000046u);
    resource_load_model("WRKTABLX.MOD", 0x0000017eu, 0x00000047u);
    resource_load_model("PALETTE.MOD", 0x0000017fu, 0x00000046u);
    resource_load_model("PIPE1.MOD", 0x00000180u, 0x00000046u);
    resource_load_model("PIPE2.MOD", 0x00000181u, 0x00000046u);
    resource_load_model("TVMON1.MOD", 0x00000184u, 0x00000046u);
    resource_load_model("HOVRCRFT.MOD", 0x00000185u, 0x00000046u);
    resource_load_model("MOTOBOAT.MOD", 0x00000186u, 0x00000046u);
    resource_load_model("LANDCRFT.MOD", 0x00000187u, 0x00000046u);
    resource_load_model("BARRIER1.MOD", 0x00000273u, 0x00000046u);
    resource_load_model("BARR1LEG.MOD", 0x00000274u, 0x00000046u);
    return resource_load_model("BARR1BAR.MOD", 0x000001edu, 0x00000046u);
}

static uint32 load_stage17_actors(void)
{
    FUNCTION_MARKER(0x800383fcu, "GAME.EXE");
    uint32 scratch_context, dummy_context;
    scratch_context = resource_arenas.scratch.context;
    resource_load(0x00000066u, scratch_context, NULL, 0, NULL, &resource_arenas.scratch);
    RESOURCE_VIEW dummy = resource_find("DUMMY.DAT");
    dummy_context = dummy.context_address;
    {
        uintptr_t ptr = (uintptr_t)dummy.data, base = (uintptr_t)resource_storage.data;
        /* TODO: Investigate a DUMMY cursor outside its actual native resource allocation */
        XASSERT(ptr >= base && ptr - base <= RESOURCE_STORAGE_CAPACITY);
        resource_arenas.scratch = (RESOURCE_CURSOR){dummy.data, RESOURCE_STORAGE_CAPACITY - (uint32)(ptr - base), dummy_context};
    }
    resource_load_animations("GENMALE.ADT", 0xffffffffu);
    resource_load_animations("GENFMALE.ADT", 0xfffffffeu);
    character_load_soldier_variants();
    character_load_femsol();
    character_load_sas();
    return character_load_airbiker();
}

static void load_stage17_models(void)
{
    MODEL_REFERENCE model0;
    FUNCTION_MARKER(0x8003847cu, "GAME.EXE");
    uint32 load_context, alloc_context, byte_len;
    load_context = resource_arenas.persistent.context;
    resource_load(0x00000039u, load_context, NULL, 0, NULL, &resource_arenas.persistent);
    alloc_context = resource_arenas.persistent.context;
    byte_len = resource_io.file_size;
    /* TODO: Investigate allocation beyond the actual native resource owner */
    XASSERT(resource_arenas.persistent.data && (byte_len) <= resource_arenas.persistent.available);
    resource_arenas.persistent.data += byte_len;
    resource_arenas.persistent.available -= byte_len;
    resource_arenas.persistent.context = alloc_context + byte_len;
    resource_load_model("NWING01.MOD", 0x0000018bu, 0x00000017u);
    resource_load_model("WING03.MOD", 0x0000018cu, 0x00000017u);
    resource_load_model("WINGJET.MOD", 0x0000018du, 0x00000017u);
    resource_load_model("HOLOGLBE.MOD", 0x0000031eu, 0x00000046u);
    resource_load_model("PILMON1.MOD", 0x0000031fu, 0x00000046u);
    resource_load_model("RADAR1.MOD", 0x00000320u, 0x00000046u);
    resource_load_model("RADAR2.MOD", 0x00000321u, 0x00000046u);
    resource_load_model("S63PIPE1.MOD", 0x00000322u, 0x00000046u);
    resource_load_model("S63PIPE2.MOD", 0x00000323u, 0x00000046u);
    resource_load_model("PILOTCHR.MOD", 0x00000312u, 0x00000046u);
    resource_load_model("PILCHRX1.MOD", 0x00000313u, 0x00000046u);
    resource_load_model("PILCHRX2.MOD", 0x00000314u, 0x00000047u);
    resource_load_model("PILDESK1.MOD", 0x00000315u, 0x00000046u);
    resource_load_model("PDESK1X1.MOD", 0x00000316u, 0x00000046u);
    resource_load_model("PDESK1X2.MOD", 0x00000317u, 0x00000047u);
    resource_load_model("PILDESK2.MOD", 0x00000318u, 0x00000046u);
    resource_load_model("PDESK2X1.MOD", 0x00000319u, 0x00000046u);
    resource_load_model("PDESK2X2.MOD", 0x0000031au, 0x00000047u);
    resource_load_model("PILDESK3.MOD", 0x0000031bu, 0x00000046u);
    resource_load_model("PDESK3X1.MOD", 0x0000031cu, 0x00000046u);
    resource_load_model("PDESK3X2.MOD", 0x0000031du, 0x00000047u);
    resource_load_model("PIPE1.MOD", 0x00000324u, 0x00000046u);
    resource_load_model("PIPE1X.MOD", 0x00000325u, 0x00000046u);
    resource_load_model("PIPE2.MOD", 0x00000326u, 0x00000046u);
    resource_load_model("PIPE2X.MOD", 0x00000327u, 0x00000046u);
    resource_load_model("PIPE3.MOD", 0x00000328u, 0x00000046u);
    resource_load_model("PIPE3X.MOD", 0x00000329u, 0x00000046u);
    resource_load_model("PIPE4.MOD", 0x0000032au, 0x00000046u);
    resource_load_model("PIPE4X.MOD", 0x0000032bu, 0x00000046u);
    resource_load_model("S63MON1.MOD", 0x0000032cu, 0x00000046u);
    resource_load_model("S63BAR1A.MOD", 0x0000032du, 0x00000046u);
    resource_load_model("S63BAR1B.MOD", 0x0000032eu, 0x00000046u);
    resource_load_model("S63BAR1C.MOD", 0x000001ffu, 0x00000046u);
    resource_load_model("HBIKMEN1.MOD", 0x000001ccu, 0x00000046u);
    resource_load_model("HBIKMEN2.MOD", 0x000001cdu, 0x00000046u);
    resource_load_model("HBIKMEN3.MOD", 0x000001ceu, 0x00000046u);
    resource_load_model("HBIKMEN4.MOD", 0x000001cfu, 0x00000046u);
    resource_load_model("CRATE1.MOD", 0x00000260u, 0x00000046u);
    model0 = model_refs[806];
    model_refs[471] = model0;
    return;
}

static uint32 load_stage18_actors(void)
{
    FUNCTION_MARKER(0x800387c8u, "GAME.EXE");
    uint32 scratch_context, dummy_context;
    scratch_context = resource_arenas.scratch.context;
    resource_load(0x00000067u, scratch_context, NULL, 0, NULL, &resource_arenas.scratch);
    RESOURCE_VIEW dummy = resource_find("DUMMY.DAT");
    dummy_context = dummy.context_address;
    {
        uintptr_t ptr = (uintptr_t)dummy.data, base = (uintptr_t)resource_storage.data;
        /* TODO: Investigate a DUMMY cursor outside its actual native resource allocation */
        XASSERT(ptr >= base && ptr - base <= RESOURCE_STORAGE_CAPACITY);
        resource_arenas.scratch = (RESOURCE_CURSOR){dummy.data, RESOURCE_STORAGE_CAPACITY - (uint32)(ptr - base), dummy_context};
    }
    resource_load_animations("GENMALE.ADT", 0xffffffffu);
    resource_load_animations("GENFMALE.ADT", 0xfffffffeu);
    character_load_wets();
    character_load_soldier_variants();
    character_load_femsol();
    character_load_sas();
    return character_load_airbiker();
}

static uint32 load_stage18_models(void)
{
    MODEL_REFERENCE model0, model1;
    FUNCTION_MARKER(0x80038850u, "GAME.EXE");
    uint32 load_context, alloc_context, byte_len;
    load_context = resource_arenas.persistent.context;
    resource_load(0x0000003au, load_context, NULL, 0, NULL, &resource_arenas.persistent);
    alloc_context = resource_arenas.persistent.context;
    byte_len = resource_io.file_size;
    /* TODO: Investigate allocation beyond the actual native resource owner */
    XASSERT(resource_arenas.persistent.data && (byte_len) <= resource_arenas.persistent.available);
    resource_arenas.persistent.data += byte_len;
    resource_arenas.persistent.available -= byte_len;
    resource_arenas.persistent.context = alloc_context + byte_len;
    resource_load_model("HOVER02A.MOD", 0x0000018eu, 0x00000018u);
    resource_load_model("HOVER02B.MOD", 0x0000018fu, 0x00000018u);
    resource_load_model("FAN.MOD", 0x00000190u, 0x00000047u);
    resource_load_model("AMMOBOX1.MOD", 0x0000032fu, 0x00000046u);
    resource_load_model("CRATE1.MOD", 0x00000330u, 0x00000046u);
    resource_load_model("FUELDRUM.MOD", 0x00000220u, 0x00000046u);
    resource_load_model("FLAREGUN.MOD", 0x000001f9u, 0x00000046u);
    resource_load_model("FLARSHEL.MOD", 0x00000331u, 0x00000046u);
    resource_load_model("FUELTANK.MOD", 0x00000332u, 0x00000046u);
    resource_load_model("HOVRMIS1.MOD", 0x00000333u, 0x00000046u);
    resource_load_model("HOVRMIS2.MOD", 0x00000334u, 0x00000046u);
    resource_load_model("S64BOX1.MOD", 0x00000335u, 0x00000046u);
    model0 = model_refs[821];
    model_refs[608] = model0;
    model1 = model_refs[816];
    model_refs[610] = model1;
    resource_load_model("S64PIPE1.MOD", 0x00000336u, 0x00000046u);
    resource_load_model("S64PIPE2.MOD", 0x00000337u, 0x00000046u);
    resource_load_model("S64RADAR.MOD", 0x00000338u, 0x00000046u);
    resource_load_model("S64RADR1.MOD", 0x00000339u, 0x00000046u);
    resource_load_model("S64RADR2.MOD", 0x0000033au, 0x00000046u);
    resource_load_model("GUNSTND1.MOD", 0x0000033cu, 0x00000046u);
    resource_load_model("GUNSTD1X.MOD", 0x0000033du, 0x00000047u);
    resource_load_model("GUNSTND2.MOD", 0x0000033eu, 0x00000046u);
    resource_load_model("GUNSTD2X.MOD", 0x0000033fu, 0x00000047u);
    resource_load_model("HBIKMEN1.MOD", 0x000001ccu, 0x00000046u);
    resource_load_model("HBIKMEN2.MOD", 0x000001cdu, 0x00000046u);
    resource_load_model("SEAHAWK.MOD", 0x00000155u, 0x00000046u);
    return resource_load_model("ROTOR.MOD", 0x00000234u, 0x00000047u);
}

static uint32 load_stage19_actors(void)
{
    FUNCTION_MARKER(0x80038aacu, "GAME.EXE");
    uint32 scratch_context, dummy_context;
    scratch_context = resource_arenas.scratch.context;
    resource_load(0x00000068u, scratch_context, NULL, 0, NULL, &resource_arenas.scratch);
    RESOURCE_VIEW dummy = resource_find("DUMMY.DAT");
    dummy_context = dummy.context_address;
    {
        uintptr_t ptr = (uintptr_t)dummy.data, base = (uintptr_t)resource_storage.data;
        /* TODO: Investigate a DUMMY cursor outside its actual native resource allocation */
        XASSERT(ptr >= base && ptr - base <= RESOURCE_STORAGE_CAPACITY);
        resource_arenas.scratch = (RESOURCE_CURSOR){dummy.data, RESOURCE_STORAGE_CAPACITY - (uint32)(ptr - base), dummy_context};
    }
    resource_load_animations("GENMALE.ADT", 0xffffffffu);
    character_load_security();
    return character_load_orange();
}

static void load_stage19_models(void)
{
    MODEL_REFERENCE model0;
    FUNCTION_MARKER(0x80038b0cu, "GAME.EXE");
    uint32 load_context, alloc_context, byte_len;
    load_context = resource_arenas.persistent.context;
    resource_load(0x0000003bu, load_context, NULL, 0, NULL, &resource_arenas.persistent);
    alloc_context = resource_arenas.persistent.context;
    byte_len = resource_io.file_size;
    /* TODO: Investigate allocation beyond the actual native resource owner */
    XASSERT(resource_arenas.persistent.data && (byte_len) <= resource_arenas.persistent.available);
    resource_arenas.persistent.data += byte_len;
    resource_arenas.persistent.available -= byte_len;
    resource_arenas.persistent.context = alloc_context + byte_len;
    resource_load_model("NAVEL05.MOD", 0x0000016du, 0x00000016u);
    resource_load_model("CRATE1.MOD", 0x0000016fu, 0x00000046u);
    resource_load_model("FUELDRM2.MOD", 0x00000229u, 0x00000046u);
    resource_load_model("FUELDRUM.MOD", 0x0000021fu, 0x00000046u);
    resource_load_model("PALETTE.MOD", 0x0000017fu, 0x00000046u);
    resource_load_model("FLIFTBOD.MOD", 0x00000188u, 0x00000046u);
    resource_load_model("FLIFTFRK.MOD", 0x00000189u, 0x00000046u);
    resource_load_model("FORKLEXP.MOD", 0x0000018au, 0x00000046u);
    resource_load_model("WORKTABL.MOD", 0x0000017du, 0x00000046u);
    resource_load_model("WRKTABLX.MOD", 0x0000017eu, 0x00000047u);
    model0 = model_refs[367];
    model_refs[608] = model0;
    return;
}

static uint32 load_stage20_actors(void)
{
    FUNCTION_MARKER(0x80038c28u, "GAME.EXE");
    uint32 scratch_context, dummy_context;
    scratch_context = resource_arenas.scratch.context;
    resource_load(0x00000069u, scratch_context, NULL, 0, NULL, &resource_arenas.scratch);
    RESOURCE_VIEW dummy = resource_find("DUMMY.DAT");
    dummy_context = dummy.context_address;
    {
        uintptr_t ptr = (uintptr_t)dummy.data, base = (uintptr_t)resource_storage.data;
        /* TODO: Investigate a DUMMY cursor outside its actual native resource allocation */
        XASSERT(ptr >= base && ptr - base <= RESOURCE_STORAGE_CAPACITY);
        resource_arenas.scratch = (RESOURCE_CURSOR){dummy.data, RESOURCE_STORAGE_CAPACITY - (uint32)(ptr - base), dummy_context};
    }
    return character_load_jet();
}

static uint32 load_stage20_models(void)
{
    FUNCTION_MARKER(0x80038c70u, "GAME.EXE");
    uint32 load_context, alloc_context, byte_len;
    load_context = resource_arenas.persistent.context;
    resource_load(0x0000003cu, load_context, NULL, 0, NULL, &resource_arenas.persistent);
    alloc_context = resource_arenas.persistent.context;
    byte_len = resource_io.file_size;
    /* TODO: Investigate allocation beyond the actual native resource owner */
    XASSERT(resource_arenas.persistent.data && (byte_len) <= resource_arenas.persistent.available);
    resource_arenas.persistent.data += byte_len;
    resource_arenas.persistent.available -= byte_len;
    resource_arenas.persistent.context = alloc_context + byte_len;
    return load_airbase_common_models();
}

static uint32 load_stage21_actors(void)
{
    FUNCTION_MARKER(0x80038cc0u, "GAME.EXE");
    uint32 scratch_context, dummy_context;
    scratch_context = resource_arenas.scratch.context;
    resource_load(0x0000006au, scratch_context, NULL, 0, NULL, &resource_arenas.scratch);
    RESOURCE_VIEW dummy = resource_find("DUMMY.DAT");
    dummy_context = dummy.context_address;
    {
        uintptr_t ptr = (uintptr_t)dummy.data, base = (uintptr_t)resource_storage.data;
        /* TODO: Investigate a DUMMY cursor outside its actual native resource allocation */
        XASSERT(ptr >= base && ptr - base <= RESOURCE_STORAGE_CAPACITY);
        resource_arenas.scratch = (RESOURCE_CURSOR){dummy.data, RESOURCE_STORAGE_CAPACITY - (uint32)(ptr - base), dummy_context};
    }
    return character_load_vulkan();
}

static uint32 load_stage21_models(void)
{
    FUNCTION_MARKER(0x80038d08u, "GAME.EXE");
    uint32 load_context, alloc_context, byte_len;
    load_context = resource_arenas.persistent.context;
    resource_load(0x0000003du, load_context, NULL, 0, NULL, &resource_arenas.persistent);
    alloc_context = resource_arenas.persistent.context;
    byte_len = resource_io.file_size;
    /* TODO: Investigate allocation beyond the actual native resource owner */
    XASSERT(resource_arenas.persistent.data && (byte_len) <= resource_arenas.persistent.available);
    resource_arenas.persistent.data += byte_len;
    resource_arenas.persistent.available -= byte_len;
    resource_arenas.persistent.context = alloc_context + byte_len;
    resource_load_model("CAVEROOF.MOD", 0x00000169u, 0x00000016u);
    resource_load_model("NAVBACK1.MOD", 0x0000016au, 0x00000016u);
    resource_load_model("NAVEL03.MOD", 0x0000016bu, 0x00000016u);
    resource_load_model("NAVEL04.MOD", 0x0000016cu, 0x00000016u);
    resource_load_model("NBACKWL.MOD", 0x0000016eu, 0x00000016u);
    resource_load_model("CRATE1.MOD", 0x0000016fu, 0x00000046u);
    resource_load_model("FUELDRUM.MOD", 0x00000220u, 0x00000046u);
    resource_load_model("FUELDRM2.MOD", 0x00000229u, 0x00000046u);
    resource_load_model("FUELTANK.MOD", 0x00000173u, 0x00000046u);
    resource_load_model("JEEP2.MOD", 0x00000170u, 0x00000046u);
    resource_load_model("LOAD1.MOD", 0x00000171u, 0x00000046u);
    resource_load_model("LOAD2.MOD", 0x00000172u, 0x00000046u);
    resource_load_model("MACHNRY1.MOD", 0x00000175u, 0x00000046u);
    resource_load_model("MACHNRY2.MOD", 0x00000177u, 0x00000046u);
    resource_load_model("MACHNRY3.MOD", 0x00000179u, 0x00000046u);
    resource_load_model("MACHNRY4.MOD", 0x0000017bu, 0x00000046u);
    resource_load_model("PALETTE.MOD", 0x0000017fu, 0x00000046u);
    resource_load_model("PIPE1.MOD", 0x00000180u, 0x00000046u);
    resource_load_model("PIPE2.MOD", 0x00000181u, 0x00000046u);
    resource_load_model("WORKTABL.MOD", 0x0000017du, 0x00000046u);
    resource_load_model("TVMON1.MOD", 0x00000184u, 0x00000046u);
    resource_load_model("HOVRCRFT.MOD", 0x00000185u, 0x00000046u);
    resource_load_model("MOTOBOAT.MOD", 0x00000186u, 0x00000046u);
    resource_load_model("LANDCRFT.MOD", 0x00000187u, 0x00000046u);
    resource_load_model("BARRIER1.MOD", 0x00000273u, 0x00000046u);
    resource_load_model("BARR1LEG.MOD", 0x00000274u, 0x00000046u);
    return resource_load_model("BARR1BAR.MOD", 0x000001edu, 0x00000046u);
}

static uint32 load_stage22_actors(void)
{
    FUNCTION_MARKER(0x80038f6cu, "GAME.EXE");
    uint32 scratch_context, dummy_context;
    scratch_context = resource_arenas.scratch.context;
    resource_load(0x0000006bu, scratch_context, NULL, 0, NULL, &resource_arenas.scratch);
    RESOURCE_VIEW dummy = resource_find("DUMMY.DAT");
    dummy_context = dummy.context_address;
    {
        uintptr_t ptr = (uintptr_t)dummy.data, base = (uintptr_t)resource_storage.data;
        /* TODO: Investigate a DUMMY cursor outside its actual native resource allocation */
        XASSERT(ptr >= base && ptr - base <= RESOURCE_STORAGE_CAPACITY);
        resource_arenas.scratch = (RESOURCE_CURSOR){dummy.data, RESOURCE_STORAGE_CAPACITY - (uint32)(ptr - base), dummy_context};
    }
    resource_load_animations("GENMALE.ADT", 0xffffffffu);
    resource_load_animations("CRYO.ADT", 0xffffffffu);
    character_load_radio();
    character_load_exo_variants();
    return character_load_cryo();
}

static void load_stage22_models(void)
{
    MODEL_REFERENCE model0;
    FUNCTION_MARKER(0x80038fe4u, "GAME.EXE");
    uint32 load_context, alloc_context, byte_len;
    load_context = resource_arenas.persistent.context;
    resource_load(0x0000003eu, load_context, NULL, 0, NULL, &resource_arenas.persistent);
    alloc_context = resource_arenas.persistent.context;
    byte_len = resource_io.file_size;
    /* TODO: Investigate allocation beyond the actual native resource owner */
    XASSERT(resource_arenas.persistent.data && (byte_len) <= resource_arenas.persistent.available);
    resource_arenas.persistent.data += byte_len;
    resource_arenas.persistent.available -= byte_len;
    resource_arenas.persistent.context = alloc_context + byte_len;
    resource_load_model("LAB01S.MOD", 0x00000191u, 0x0000001au);
    resource_load_model("LAB03M.MOD", 0x00000192u, 0x0000001au);
    resource_load_model("LAB03N.MOD", 0x00000193u, 0x0000001au);
    resource_load_model("LAB01Z.MOD", 0x00000194u, 0x0000001au);
    resource_load_model("DOOR01.MOD", 0x00000195u, 0x0000001au);
    resource_load_model("LABEND.MOD", 0x00000196u, 0x0000001au);
    resource_load_model("CRATE2.MOD", 0x00000232u, 0x00000046u);
    resource_load_model("CRATE3.MOD", 0x00000197u, 0x00000046u);
    resource_load_model("DSKCHAIR.MOD", 0x00000198u, 0x00000046u);
    resource_load_model("LABPIPE1.MOD", 0x00000199u, 0x00000046u);
    resource_load_model("LABPIP1X.MOD", 0x0000019au, 0x00000046u);
    resource_load_model("LABCOMP1.MOD", 0x0000019bu, 0x00000046u);
    resource_load_model("LABCMP1X.MOD", 0x0000019cu, 0x00000047u);
    resource_load_model("LABCOMP3.MOD", 0x0000019du, 0x00000046u);
    resource_load_model("LABCMP3X.MOD", 0x0000019eu, 0x00000047u);
    resource_load_model("LABCOMP4.MOD", 0x0000019fu, 0x00000046u);
    resource_load_model("LABCMP4X.MOD", 0x000001a0u, 0x00000047u);
    resource_load_model("LABCOMP5.MOD", 0x000001a1u, 0x00000046u);
    resource_load_model("LABCMP5X.MOD", 0x000001a2u, 0x00000047u);
    resource_load_model("LABCOMP7.MOD", 0x000001a3u, 0x00000046u);
    resource_load_model("LABCMP7X.MOD", 0x000001a4u, 0x00000047u);
    resource_load_model("LABCOMP8.MOD", 0x000001a5u, 0x00000046u);
    resource_load_model("LABCMP8X.MOD", 0x000001a6u, 0x00000047u);
    resource_load_model("LABCOMP9.MOD", 0x000001a7u, 0x00000046u);
    resource_load_model("LABCMP9X.MOD", 0x000001a8u, 0x00000047u);
    resource_load_model("LABTABL1.MOD", 0x000001a9u, 0x00000046u);
    resource_load_model("LABTAB1X.MOD", 0x000001aau, 0x00000047u);
    resource_load_model("LABTANK1.MOD", 0x000001abu, 0x00000046u);
    resource_load_model("LABTNK1X.MOD", 0x000001acu, 0x00000047u);
    resource_load_model("LABTNK2.MOD", 0x000001adu, 0x00000046u);
    resource_load_model("LABTNK2X.MOD", 0x000001aeu, 0x00000047u);
    resource_load_model("LABTNK3.MOD", 0x000001afu, 0x00000046u);
    resource_load_model("LABTNK3X.MOD", 0x000001b0u, 0x00000047u);
    resource_load_model("LABTNKGL.MOD", 0x000001b1u, 0x00000046u);
    resource_load_model("LABTNKGT.MOD", 0x000001b2u, 0x00000046u);
    resource_load_model("CRYO2.MOD", 0x000001b3u, 0x00000044u);
    model0 = model_refs[407];
    model_refs[608] = model0;
    model_refs[612] = model0;
    model_refs[611] = model0;
    return;
}

static uint32 load_stage23_actors(void)
{
    FUNCTION_MARKER(0x80039314u, "GAME.EXE");
    uint32 scratch_context, dummy_context;
    scratch_context = resource_arenas.scratch.context;
    resource_load(0x0000006cu, scratch_context, NULL, 0, NULL, &resource_arenas.scratch);
    RESOURCE_VIEW dummy = resource_find("DUMMY.DAT");
    dummy_context = dummy.context_address;
    {
        uintptr_t ptr = (uintptr_t)dummy.data, base = (uintptr_t)resource_storage.data;
        /* TODO: Investigate a DUMMY cursor outside its actual native resource allocation */
        XASSERT(ptr >= base && ptr - base <= RESOURCE_STORAGE_CAPACITY);
        resource_arenas.scratch = (RESOURCE_CURSOR){dummy.data, RESOURCE_STORAGE_CAPACITY - (uint32)(ptr - base), dummy_context};
    }
    resource_load_animations("GENMALE.ADT", 0xffffffffu);
    character_load_soldier_variants();
    character_load_sas();
    return character_load_airbiker();
}

static uint32 load_stage23_models(void)
{
    FUNCTION_MARKER(0x8003937cu, "GAME.EXE");
    uint32 load_context, alloc_context, byte_len;
    load_context = resource_arenas.persistent.context;
    resource_load(0x0000003fu, load_context, NULL, 0, NULL, &resource_arenas.persistent);
    alloc_context = resource_arenas.persistent.context;
    byte_len = resource_io.file_size;
    /* TODO: Investigate allocation beyond the actual native resource owner */
    XASSERT(resource_arenas.persistent.data && (byte_len) <= resource_arenas.persistent.available);
    resource_arenas.persistent.data += byte_len;
    resource_arenas.persistent.available -= byte_len;
    resource_arenas.persistent.context = alloc_context + byte_len;
    resource_load_model("LIFTFR01.MOD", 0x000001b4u, 0x00000007u);
    resource_load_model("LIFT01.MOD", 0x000001b5u, 0x0000001bu);
    resource_load_model("LIFT02.MOD", 0x000001b6u, 0x0000001bu);
    resource_load_model("LIFTBOT.MOD", 0x000001b7u, 0x0000001bu);
    resource_load_model("LIFTBOT2.MOD", 0x000001b8u, 0x0000001bu);
    resource_load_model("LIFTTOP.MOD", 0x000001b9u, 0x0000001bu);
    resource_load_model("LIFTTOP2.MOD", 0x000001bau, 0x0000001bu);
    resource_load_model("HBIKMEN1.MOD", 0x000001ccu, 0x00000046u);
    resource_load_model("HBIKMEN2.MOD", 0x000001cdu, 0x00000046u);
    resource_load_model("HBIKMEN3.MOD", 0x000001ceu, 0x00000046u);
    return resource_load_model("HBIKMEN4.MOD", 0x000001cfu, 0x00000046u);
}

static uint32 load_stage24_actors(void)
{
    FUNCTION_MARKER(0x800394a0u, "GAME.EXE");
    uint32 scratch_context, dummy_context;
    scratch_context = resource_arenas.scratch.context;
    resource_load(0x0000006du, scratch_context, NULL, 0, NULL, &resource_arenas.scratch);
    RESOURCE_VIEW dummy = resource_find("DUMMY.DAT");
    dummy_context = dummy.context_address;
    {
        uintptr_t ptr = (uintptr_t)dummy.data, base = (uintptr_t)resource_storage.data;
        /* TODO: Investigate a DUMMY cursor outside its actual native resource allocation */
        XASSERT(ptr >= base && ptr - base <= RESOURCE_STORAGE_CAPACITY);
        resource_arenas.scratch = (RESOURCE_CURSOR){dummy.data, RESOURCE_STORAGE_CAPACITY - (uint32)(ptr - base), dummy_context};
    }
    return character_load_zeng();
}

static void load_stage24_models(void)
{
    MODEL_REFERENCE model0;
    FUNCTION_MARKER(0x800394e8u, "GAME.EXE");
    uint32 load_context, alloc_context, byte_len;
    load_context = resource_arenas.persistent.context;
    resource_load(0x00000040u, load_context, NULL, 0, NULL, &resource_arenas.persistent);
    alloc_context = resource_arenas.persistent.context;
    byte_len = resource_io.file_size;
    /* TODO: Investigate allocation beyond the actual native resource owner */
    XASSERT(resource_arenas.persistent.data && (byte_len) <= resource_arenas.persistent.available);
    resource_arenas.persistent.data += byte_len;
    resource_arenas.persistent.available -= byte_len;
    resource_arenas.persistent.context = alloc_context + byte_len;
    resource_load_model("END01.MOD", 0x000001bbu, 0x0000001cu);
    resource_load_model("CRATE1.MOD", 0x00000197u, 0x0000001cu);
    model0 = model_refs[407];
    model_refs[608] = model0;
    model_refs[610] = model0;
    model_refs[613] = model0;
    model_refs[609] = model0;
    model_refs[611] = model0;
    return;
}

/* IDs keep original callback identities distinct even under identical code folding */
const RESOURCE_LOADER resource_loaders[RESOURCE_LOADER_COUNT] = {
    {character_load_hawk, RESOURCE_LOADER_HAWK, RESOURCE_LOADER_PLAYER},
    {character_load_hawk2, RESOURCE_LOADER_HAWK2, RESOURCE_LOADER_PLAYER},
    {character_load_mace, RESOURCE_LOADER_MACE, RESOURCE_LOADER_PLAYER},
    {character_load_smasher, RESOURCE_LOADER_SMASHER, RESOURCE_LOADER_PLAYER},
    {character_load_alana, RESOURCE_LOADER_ALANA, RESOURCE_LOADER_PLAYER},
    {load_stage0_actors, RESOURCE_LOADER_STAGE0_ACTORS, RESOURCE_LOADER_STAGE},
    {load_stage0_models, RESOURCE_LOADER_STAGE0_MODELS, RESOURCE_LOADER_STAGE},
    {load_stage1_actors, RESOURCE_LOADER_STAGE1_ACTORS, RESOURCE_LOADER_STAGE},
    {load_stage1_models, RESOURCE_LOADER_STAGE1_MODELS, RESOURCE_LOADER_STAGE},
    {load_stage2_actors, RESOURCE_LOADER_STAGE2_ACTORS, RESOURCE_LOADER_STAGE},
    {load_stage2_models, RESOURCE_LOADER_STAGE2_MODELS, RESOURCE_LOADER_STAGE},
    {load_stage3_actors, RESOURCE_LOADER_STAGE3_ACTORS, RESOURCE_LOADER_STAGE},
    {load_stage3_models, RESOURCE_LOADER_STAGE3_MODELS, RESOURCE_LOADER_STAGE},
    {load_stage4_actors, RESOURCE_LOADER_STAGE4_ACTORS, RESOURCE_LOADER_STAGE},
    {load_stage4_models, RESOURCE_LOADER_STAGE4_MODELS, RESOURCE_LOADER_STAGE},
    {load_stage5_actors, RESOURCE_LOADER_STAGE5_ACTORS, RESOURCE_LOADER_STAGE},
    {load_stage5_models, RESOURCE_LOADER_STAGE5_MODELS, RESOURCE_LOADER_STAGE},
    {load_stage6_actors, RESOURCE_LOADER_STAGE6_ACTORS, RESOURCE_LOADER_STAGE},
    {load_stage6_models, RESOURCE_LOADER_STAGE6_MODELS, RESOURCE_LOADER_STAGE},
    {load_stage7_actors, RESOURCE_LOADER_STAGE7_ACTORS, RESOURCE_LOADER_STAGE},
    {NULL, RESOURCE_LOADER_STAGE7_MODELS, RESOURCE_LOADER_STAGE, load_stage7_models},
    {load_stage8_actors, RESOURCE_LOADER_STAGE8_ACTORS, RESOURCE_LOADER_STAGE},
    {load_stage8_models, RESOURCE_LOADER_STAGE8_MODELS, RESOURCE_LOADER_STAGE},
    {load_stage9_actors, RESOURCE_LOADER_STAGE9_ACTORS, RESOURCE_LOADER_STAGE},
    {load_stage9_models, RESOURCE_LOADER_STAGE9_MODELS, RESOURCE_LOADER_STAGE},
    {load_stage10_actors, RESOURCE_LOADER_STAGE10_ACTORS, RESOURCE_LOADER_STAGE},
    {NULL, RESOURCE_LOADER_STAGE10_MODELS, RESOURCE_LOADER_STAGE, load_stage10_models},
    {load_stage11_actors, RESOURCE_LOADER_STAGE11_ACTORS, RESOURCE_LOADER_STAGE},
    {NULL, RESOURCE_LOADER_STAGE11_MODELS, RESOURCE_LOADER_STAGE, load_stage11_models},
    {load_stage12_actors, RESOURCE_LOADER_STAGE12_ACTORS, RESOURCE_LOADER_STAGE},
    {load_stage12_models, RESOURCE_LOADER_STAGE12_MODELS, RESOURCE_LOADER_STAGE},
    {load_stage13_actors, RESOURCE_LOADER_STAGE13_ACTORS, RESOURCE_LOADER_STAGE},
    {load_stage14_actors, RESOURCE_LOADER_STAGE14_ACTORS, RESOURCE_LOADER_STAGE},
    {NULL, RESOURCE_LOADER_STAGE14_MODELS, RESOURCE_LOADER_STAGE, load_stage14_models},
    {load_stage15_actors, RESOURCE_LOADER_STAGE15_ACTORS, RESOURCE_LOADER_STAGE},
    {load_airbase_common_models, RESOURCE_LOADER_AIRBASE_COMMON_MODELS, RESOURCE_LOADER_STAGE},
    {load_stage15_models, RESOURCE_LOADER_STAGE15_MODELS, RESOURCE_LOADER_STAGE},
    {load_stage16_actors, RESOURCE_LOADER_STAGE16_ACTORS, RESOURCE_LOADER_STAGE},
    {load_stage16_models, RESOURCE_LOADER_STAGE16_MODELS, RESOURCE_LOADER_STAGE},
    {load_stage17_actors, RESOURCE_LOADER_STAGE17_ACTORS, RESOURCE_LOADER_STAGE},
    {NULL, RESOURCE_LOADER_STAGE17_MODELS, RESOURCE_LOADER_STAGE, load_stage17_models},
    {load_stage18_actors, RESOURCE_LOADER_STAGE18_ACTORS, RESOURCE_LOADER_STAGE},
    {load_stage18_models, RESOURCE_LOADER_STAGE18_MODELS, RESOURCE_LOADER_STAGE},
    {load_stage19_actors, RESOURCE_LOADER_STAGE19_ACTORS, RESOURCE_LOADER_STAGE},
    {NULL, RESOURCE_LOADER_STAGE19_MODELS, RESOURCE_LOADER_STAGE, load_stage19_models},
    {load_stage20_actors, RESOURCE_LOADER_STAGE20_ACTORS, RESOURCE_LOADER_STAGE},
    {load_stage20_models, RESOURCE_LOADER_STAGE20_MODELS, RESOURCE_LOADER_STAGE},
    {load_stage21_actors, RESOURCE_LOADER_STAGE21_ACTORS, RESOURCE_LOADER_STAGE},
    {load_stage21_models, RESOURCE_LOADER_STAGE21_MODELS, RESOURCE_LOADER_STAGE},
    {load_stage22_actors, RESOURCE_LOADER_STAGE22_ACTORS, RESOURCE_LOADER_STAGE},
    {NULL, RESOURCE_LOADER_STAGE22_MODELS, RESOURCE_LOADER_STAGE, load_stage22_models},
    {load_stage23_actors, RESOURCE_LOADER_STAGE23_ACTORS, RESOURCE_LOADER_STAGE},
    {load_stage23_models, RESOURCE_LOADER_STAGE23_MODELS, RESOURCE_LOADER_STAGE},
    {load_stage24_actors, RESOURCE_LOADER_STAGE24_ACTORS, RESOURCE_LOADER_STAGE},
    {NULL, RESOURCE_LOADER_STAGE24_MODELS, RESOURCE_LOADER_STAGE, load_stage24_models},
    {scene_load_resources1, RESOURCE_LOADER_SCENE1, RESOURCE_LOADER_STAGE},
    {scene_load_resources2, RESOURCE_LOADER_SCENE2, RESOURCE_LOADER_STAGE},
    {scene_load_resources3, RESOURCE_LOADER_SCENE3, RESOURCE_LOADER_STAGE},
    {scene_load_resources4, RESOURCE_LOADER_SCENE4, RESOURCE_LOADER_STAGE},
    {scene_load_resources5, RESOURCE_LOADER_SCENE5, RESOURCE_LOADER_STAGE},
    {scene_load_resources6, RESOURCE_LOADER_SCENE6, RESOURCE_LOADER_STAGE},
    {scene_load_resources7, RESOURCE_LOADER_SCENE7, RESOURCE_LOADER_STAGE},
    {scene_load_resources8, RESOURCE_LOADER_SCENE8, RESOURCE_LOADER_STAGE},
    {resource_load_frontend_models, RESOURCE_LOADER_FRONTEND, RESOURCE_LOADER_STAGE},
    {NULL, RESOURCE_LOADER_STAGE14_SCROLL, RESOURCE_LOADER_TEXTURE, NULL, stage_animation_scroll_stage14},
    {NULL, RESOURCE_LOADER_STAGE17_SCROLL, RESOURCE_LOADER_TEXTURE, NULL, stage_animation_scroll_stage17},
    {load_stage0_surface, RESOURCE_LOADER_STAGE0_TEX, RESOURCE_LOADER_TEXTURE},
    {load_stage1_surface, RESOURCE_LOADER_STAGE1_TEX, RESOURCE_LOADER_TEXTURE},
    {load_stage2_surface, RESOURCE_LOADER_STAGE2_TEX, RESOURCE_LOADER_TEXTURE},
    {load_stage3_surface, RESOURCE_LOADER_STAGE3_TEX, RESOURCE_LOADER_TEXTURE},
    {load_stage4_surface, RESOURCE_LOADER_STAGE4_TEX, RESOURCE_LOADER_TEXTURE},
    {load_stage5_surface, RESOURCE_LOADER_STAGE5_TEX, RESOURCE_LOADER_TEXTURE},
    {load_stage6_surface, RESOURCE_LOADER_STAGE6_TEX, RESOURCE_LOADER_TEXTURE},
    {load_stage7_surface, RESOURCE_LOADER_STAGE7_TEX, RESOURCE_LOADER_TEXTURE},
    {load_stage8_surface, RESOURCE_LOADER_STAGE8_TEX, RESOURCE_LOADER_TEXTURE},
    {load_stage9_surface, RESOURCE_LOADER_STAGE9_TEX, RESOURCE_LOADER_TEXTURE},
    {load_stage10_surface, RESOURCE_LOADER_STAGE10_TEX, RESOURCE_LOADER_TEXTURE},
    {load_stage11_surface, RESOURCE_LOADER_STAGE11_TEX, RESOURCE_LOADER_TEXTURE},
    {load_stage12_surface, RESOURCE_LOADER_STAGE12_TEX, RESOURCE_LOADER_TEXTURE},
    {load_stage14_surface, RESOURCE_LOADER_STAGE14_TEX, RESOURCE_LOADER_TEXTURE},
    {load_stage15_surface, RESOURCE_LOADER_STAGE15_TEX, RESOURCE_LOADER_TEXTURE},
    {load_stage16_surface, RESOURCE_LOADER_STAGE16_TEX, RESOURCE_LOADER_TEXTURE},
    {load_stage17_surface, RESOURCE_LOADER_STAGE17_TEX, RESOURCE_LOADER_TEXTURE},
    {load_stage18_surface, RESOURCE_LOADER_STAGE18_TEX, RESOURCE_LOADER_TEXTURE},
    {load_stage19_surface, RESOURCE_LOADER_STAGE19_TEX, RESOURCE_LOADER_TEXTURE},
    {load_stage20_surface, RESOURCE_LOADER_STAGE20_TEX, RESOURCE_LOADER_TEXTURE},
    {load_stage21_surface, RESOURCE_LOADER_STAGE21_TEX, RESOURCE_LOADER_TEXTURE},
    {load_stage22_surface, RESOURCE_LOADER_STAGE22_TEX, RESOURCE_LOADER_TEXTURE},
    {load_stage23_surface, RESOURCE_LOADER_STAGE23_TEX, RESOURCE_LOADER_TEXTURE},
    {load_stage24_surface, RESOURCE_LOADER_STAGE24_TEX, RESOURCE_LOADER_TEXTURE},
    {resource_load_frontend_surface, RESOURCE_LOADER_FRONTEND_TEX, RESOURCE_LOADER_TEXTURE},
    {load_stage28_surface, RESOURCE_LOADER_STAGE28_TEX, RESOURCE_LOADER_TEXTURE},
    {load_stgc5_surface, RESOURCE_LOADER_STGC5_TEX, RESOURCE_LOADER_TEXTURE},
    {load_stgc6_surface, RESOURCE_LOADER_STGC6_TEX, RESOURCE_LOADER_TEXTURE},
    {load_stage31_surface, RESOURCE_LOADER_STAGE31_TEX, RESOURCE_LOADER_TEXTURE},
    {load_stage33_surface, RESOURCE_LOADER_STAGE33_TEX, RESOURCE_LOADER_TEXTURE},
    {load_stage34_surface, RESOURCE_LOADER_STAGE34_TEX, RESOURCE_LOADER_TEXTURE},
};

const RESOURCE_LOADER *const player_resource_loaders[PLAYER_RESOURCE_COUNT] = {
    resource_loaders + RESOURCE_LOADER_HAWK, resource_loaders + RESOURCE_LOADER_MACE, resource_loaders + RESOURCE_LOADER_SMASHER, resource_loaders + RESOURCE_LOADER_ALANA, resource_loaders + RESOURCE_LOADER_HAWK2,
};

/* Consolidated model, player and texture dispatch; numeric targets stay in diagnostics */
uint32 resource_loader_call(RESOURCE_LOADER_DOMAIN domain, const RESOURCE_LOADER *loader)
{
    /* TODO: Investigate a missing or wrong-domain loader without a raw-code fallback */
    XASSERT(loader && loader->domain == domain);
    if (loader->load_void)
    {
        loader->load_void();
        return 0;
    }
    if (loader->load_signed)
        return loader->load_signed();
    return loader->load();
}
