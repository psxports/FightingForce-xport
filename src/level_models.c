#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "game.h"
#include "platform.h"

/* Actual14DF8..15038, including playback and recording overlays. Stops before level-script/tail;
 * This prefix is not a completed gameplay iteration */

/* Original 14B70..14DF8, stopping immediately before the first frame */

/* 14A34..14B70 stage0 initialization, before music and frame-loop setup. */

GDB_CALL sint32 level_run(void)
{
    sint32 result;
    FUNCTION_MARKER(0x80014a34, "GAME.EXE");
    level_initialize();
    level_start_playback();
    for (;;)
    {
        result = game_update_frame_prefix();
        if (result)
            return result;
        result = level_transition_update();
        if (result)
            return result;
    }
}

/* 14808: initial player positions/orientation and camera origin. */

GDB_CALL uint32 level_load_common_models(void)
{
    FUNCTION_MARKER(0x80016c18u, "GAME.EXE");
    MODEL_REFERENCE model0, model1, model2, model3, model4;
    uint32 result;
    uint32 stage, v3;
    stage = game_session.stage;
    if (stage >= RESOURCE_STAGE_COUNT)
        diagnostic_stop(__FUNCTION__, __FILE__, __LINE__);
    resource_loader_call(RESOURCE_LOADER_STAGE, stage_resource_loaders[STAGE_RESOURCE_MODELS][stage]);
    result = resource_load_model("BAT.MOD", 0x000001ddu, 0x00000047u);      /* 80016C58 */
    result = resource_load_model("IRONBAR1.MOD", 0x000001ebu, 0x00000047u); /* 80016C6C */
    result = resource_load_model("KNIFE.MOD", 0x000001e6u, 0x00000047u);    /* 80016C80 */
    result = resource_load_model("KNIFE2.MOD", 0x000001e7u, 0x00000047u);   /* 80016C94 */
    result = resource_load_model("KNIFE3.MOD", 0x000001e8u, 0x00000047u);   /* 80016CA8 */
    result = resource_load_model("KNIFE4.MOD", 0x000001e9u, 0x00000047u);   /* 80016CBC */
    result = resource_load_model("BASEBAT1.MOD", 0x000001e4u, 0x00000047u); /* 80016CD0 */
    result = resource_load_model("BATPIPE1.MOD", 0x000001e5u, 0x00000047u); /* 80016CE4 */
    result = resource_load_model("WOODBAT1.MOD", 0x000001efu, 0x00000047u); /* 80016CF8 */
    result = resource_load_model("ASHTRY.MOD", 0x0000020cu, 0x00000047u);   /* 80016D0C */
    result = resource_load_model("ASHTRY2.MOD", 0x0000020du, 0x00000047u);  /* 80016D20 */
    result = resource_load_model("FIREXTIN.MOD", 0x0000020au, 0x00000047u); /* 80016D34 */
    result = resource_load_model("FIREXTN2.MOD", 0x0000020bu, 0x00000047u); /* 80016D48 */
    result = resource_load_model("PISTOL2.MOD", 0x000001f3u, 0x00000047u);  /* 80016D5C */
    result = resource_load_model("PISTOL3.MOD", 0x000001f4u, 0x00000047u);  /* 80016D70 */
    result = resource_load_model("PISTOL4.MOD", 0x000001f5u, 0x00000047u);  /* 80016D84 */
    result = resource_load_model("PISTOL5.MOD", 0x000001f6u, 0x00000047u);  /* 80016D98 */
    result = resource_load_model("PISTOL6.MOD", 0x000001f7u, 0x00000047u);  /* 80016DAC */
    result = resource_load_model("GUNSHOT1.MOD", 0x000001f8u, 0x00000047u); /* 80016DC0 */
    result = resource_load_model("BIGGUN.MOD", 0x00000203u, 0x00000047u);   /* 80016DD4 */
    result = resource_load_model("SHELCASE.MOD", 0x00000242u, 0x00000047u); /* 80016DE8 */
    result = resource_load_model("BAZOOK.MOD", 0x000001fau, 0x00000047u);   /* 80016DFC */
    result = resource_load_model("MISSILE1.MOD", 0x0000023cu, 0x00000047u); /* 80016E10 */
    model0 = model_refs[572];                                               /* 80016E20 */
    model_refs[573] = model0;                                               /* 80016E28 */
    v3 = model_height[572];                                                 /* 80016E34 */
    model_height[573] = v3;                                                 /* 80016E3C */
    result = resource_load_model("GRENADE.MOD", 0x000001f0u, 0x00000047u);  /* 80016E4C */
    result = resource_load_model("DYNAMITE.MOD", 0x000001f1u, 0x00000047u); /* 80016E60 */
    result = resource_load_model("LIMPMINE.MOD", 0x000001f2u, 0x00000047u); /* 80016E74 */
    result = resource_load_model("BOTTLE1.MOD", 0x000001deu, 0x00000047u);  /* 80016E88 */
    result = resource_load_model("BOTTLE1B.MOD", 0x000001dfu, 0x00000047u); /* 80016E9C */
    result = resource_load_model("GL_FRAG1.MOD", 0x00000257u, 0x00000047u); /* 80016EB0 */
    result = resource_load_model("BOTTLE2.MOD", 0x000001e0u, 0x00000047u);  /* 80016EC4 */
    result = resource_load_model("BOTTLE2B.MOD", 0x000001e1u, 0x00000047u); /* 80016ED8 */
    result = resource_load_model("GL_FRAG2.MOD", 0x00000258u, 0x00000047u); /* 80016EEC */
    result = resource_load_model("BOTTLE3.MOD", 0x000001e2u, 0x00000047u);  /* 80016F00 */
    result = resource_load_model("BOTTLE3B.MOD", 0x000001e3u, 0x00000047u); /* 80016F14 */
    result = resource_load_model("GL_FRAG3.MOD", 0x00000259u, 0x00000047u); /* 80016F28 */
    model1 = model_refs[478];                                               /* 80016F30 */
    model_refs[473] = model1;                                               /* 80016F38 */
    model2 = model_refs[480];                                               /* 80016F3C */
    model_refs[474] = model2;                                               /* 80016F44 */
    model3 = model_refs[482];                                               /* 80016F48 */
    model_refs[475] = model3;                                               /* 80016F50 */
    result = resource_load_model("FALLGLAS.MOD", 0x00000381u, 0x00000047u); /* 80016F60 */
    result = resource_load_model("WRECK1.MOD", 0x00000386u, 0x00000047u);   /* 80016F74 */
    result = resource_load_model("WRECK2.MOD", 0x00000387u, 0x00000047u);   /* 80016F88 */
    result = resource_load_model("WRECK3.MOD", 0x00000388u, 0x00000047u);   /* 80016F9C */
    result = resource_load_model("WRECK4.MOD", 0x00000389u, 0x00000047u);   /* 80016FB0 */
    result = resource_load_model("SMFRAG1.MOD", 0x0000038au, 0x00000047u);  /* 80016FC4 */
    result = resource_load_model("SMFRAG2.MOD", 0x0000038bu, 0x00000047u);  /* 80016FD8 */
    result = resource_load_model("SMFRAG3.MOD", 0x0000038cu, 0x00000047u);  /* 80016FEC */
    result = resource_load_model("SMFRAG4.MOD", 0x0000038du, 0x00000047u);  /* 80017000 */
    result = resource_load_model("SPECIAL0.MOD", 0x00000363u, 0x00000000u); /* 80017014 */
    result = resource_load_model("SPECIAL1.MOD", 0x00000364u, 0x00000000u); /* 80017028 */
    result = resource_load_model("HALO1.MOD", 0x00000372u, 0x00000000u);    /* 8001703C */
    result = resource_load_model("HALO2.MOD", 0x00000373u, 0x00000000u);    /* 80017050 */
    result = resource_load_model("HALO3.MOD", 0x00000374u, 0x00000000u);    /* 80017064 */
    result = resource_load_model("HALO4.MOD", 0x00000375u, 0x00000000u);    /* 80017078 */
    result = resource_load_model("TARGET.MOD", 0x00000377u, 0x00000000u);   /* 8001708C */
    result = resource_load_model("TARGETG.MOD", 0x00000378u, 0x00000000u);  /* 800170A0 */
    result = resource_load_model("TARGETW.MOD", 0x00000379u, 0x00000000u);  /* 800170B4 */
    result = resource_load_model("SHADOW.MOD", 0x0000035cu, 0x00000000u);   /* 800170C8 */
    result = resource_load_model("SCORCH.MOD", 0x0000035du, 0x00000047u);   /* 800170DC */
    result = resource_load_model("PUPHLTH.MOD", 0x0000024bu, 0x00000047u);  /* 800170F0 */
    result = resource_load_model("PUPHLTH2.MOD", 0x0000024cu, 0x00000047u); /* 80017104 */
    result = resource_load_model("PUPHLTH3.MOD", 0x0000024du, 0x00000047u); /* 80017118 */
    result = resource_load_model("PUPBURGR.MOD", 0x0000024eu, 0x00000047u); /* 8001712C */
    result = resource_load_model("PUPHDOG.MOD", 0x0000024fu, 0x00000047u);  /* 80017140 */
    result = resource_load_model("PUPCOIN.MOD", 0x00000251u, 0x00000047u);  /* 80017154 */
    result = resource_load_model("PUPCASH.MOD", 0x00000252u, 0x00000047u);  /* 80017168 */
    result = resource_load_model("PUPGOLD.MOD", 0x00000253u, 0x00000047u);  /* 8001717C */
    result = resource_load_model("PUPJEWEL.MOD", 0x00000254u, 0x00000047u); /* 80017190 */
    result = resource_load_model("PUPPMAG.MOD", 0x00000256u, 0x00000047u);  /* 800171A4 */
    result = resource_load_model("BLAST1.MOD", 0x0000025bu, 0x00000000u);   /* 800171B8 */
    result = resource_load_model("BLAST2.MOD", 0x0000025cu, 0x00000000u);   /* 800171CC */
    result = resource_load_model("BLAST3.MOD", 0x0000025eu, 0x00000000u);   /* 800171E0 */
    model4 = model_refs[606];                                               /* 800171E8 */
    model_refs[607] = model4;                                               /* 800171F0 */
    result = resource_load_model("CAR1SHAD.MOD", 0x0000006au, 0x00000000u); /* 80017200 */
    return result;
}
