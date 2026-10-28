#include <stddef.h>
#include <stdio.h>

#include "game.h"

/* Serialized CCT frame after the little-endian count halfword */
typedef struct SCENE_FRAME
{
    sint16 x;
    sint16 y;
    sint16 z;
    uint16 yaw;
    uint16 pitch;
    uint16 roll;
} SCENE_FRAME;

_Static_assert(sizeof(SCENE_FRAME) == 12 && offsetof(SCENE_FRAME, x) == 0 && offsetof(SCENE_FRAME, y) == 2 && offsetof(SCENE_FRAME, z) == 4 && offsetof(SCENE_FRAME, yaw) == 6 && offsetof(SCENE_FRAME, pitch) == 8 && offsetof(SCENE_FRAME, roll) == 10, "CCT serialized frame layout");

SCENE_PLAYBACK scene_playback;

enum
{
    SCENE_SEQUENCE_COUNT = 8
};

static const sint16 loading_steps[SCENE_SEQUENCE_COUNT] = {69, 38, 29, 32, 244, 26, 253, 0};

/* Original scene resource operations retain their numeric result ABI temporarily */
GDB_CALL uint32 scene_load_resources0(void)
{
    FUNCTION_MARKER(0x80039578, "GAME.EXE");
    uint32 arena, byte_len;
    resource_load(67, resource_arenas.persistent.context, NULL, 0, NULL, &resource_arenas.persistent);
    arena = resource_arenas.persistent.context;
    byte_len = resource_io.file_size;
    /* TODO: Investigate allocation beyond the actual native resource owner */
    if (!resource_arenas.persistent.data || (byte_len) > resource_arenas.persistent.available)
    {
        DIAGNOSTIC(fprintf(stderr, "Resource cursor persistent span %u available %u context %08X stage %u tick %u\n", (uint32)(byte_len), resource_arenas.persistent.available, resource_arenas.persistent.context, game_session.stage, game_session.tick));
        diagnostic_stop(__FUNCTION__, __FILE__, __LINE__);
    }
    resource_arenas.persistent.data += byte_len;
    resource_arenas.persistent.available -= byte_len;
    resource_arenas.persistent.context = arena + byte_len;
    resource_load_model("SHADOW.MOD", 860, 0);     /* SHADOW.MOD */
    resource_load_model("CAR1SHAD.MOD", 106, 0);   /* CAR1SHAD.MOD */
    resource_load_model("HQ1.MOD", 56, 7);         /* HQ1.MOD */
    resource_load_model("STG1BUSH.MOD", 59, 7);    /* STG1BUSH.MOD */
    resource_load_model("STG1GATE.MOD", 70, 7);    /* STG1GATE.MOD */
    resource_load_model("STG1WALL.MOD", 60, 7);    /* STG1WALL.MOD */
    resource_load_model("LAMP.MOD", 614, 7);       /* LAMP.MOD */
    resource_load_model("LAMP2.MOD", 615, 7);      /* LAMP2.MOD */
    resource_load_model("STG1BAR.MOD", 61, 7);     /* STG1BAR.MOD */
    resource_load_model("BUILD1.MOD", 618, 7);     /* BUILD1.MOD */
    resource_load_model("BUILD2.MOD", 619, 7);     /* BUILD2.MOD */
    resource_load_model("BUILD3.MOD", 620, 7);     /* BUILD3.MOD */
    resource_load_model("BUILD4.MOD", 621, 7);     /* BUILD4.MOD */
    resource_load_model("BUILD5.MOD", 622, 7);     /* BUILD5.MOD */
    resource_load_model("TREE.MOD", 623, 7);       /* TREE.MOD */
    resource_load_model("TREE2.MOD", 624, 7);      /* TREE2.MOD */
    resource_load_model("BUSH.MOD", 625, 7);       /* BUSH.MOD */
    resource_load_model("STG1DOOR.MOD", 464, 7);   /* STG1DOOR.MOD */
    resource_load_model("CARLIGHT.MOD", 919, 0);   /* CARLIGHT.MOD */
    resource_load_model("CAR1BOD1.MOD", 94, 70);   /* CAR1BOD1.MOD */
    resource_load_model("CAR1WHLS.MOD", 93, 70);   /* CAR1WHLS.MOD */
    resource_load_model("PCARBOD1.MOD", 108, 70);  /* PCARBOD1.MOD */
    resource_load_model("TAXI.MOD", 84, 70);       /* TAXI.MOD */
    resource_load_model("PICKUP2.MOD", 78, 70);    /* PICKUP2.MOD */
    resource_load_model("JEEP1.MOD", 79, 70);      /* JEEP1.MOD */
    resource_load_model("BUS.MOD", 80, 70);        /* BUS.MOD */
    resource_load_model("CARRED1.MOD", 81, 70);    /* CARRED1.MOD */
    resource_load_model("CARBLUE1.MOD", 82, 70);   /* CARBLUE1.MOD */
    resource_load_model("POLICAR.MOD", 83, 70);    /* POLICAR.MOD */
    resource_load_model("BARRIER3.MOD", 627, 70);  /* BARRIER3.MOD */
    resource_load_model("BOXMETL1.MOD", 526, 70);  /* BOXMETL1.MOD */
    resource_load_model("PARKLAMP.MOD", 630, 70);  /* PARKLAMP.MOD */
    resource_load_model("SECURCAM.MOD", 633, 70);  /* SECURCAM.MOD */
    resource_load_model("PRKFENCE.MOD", 641, 70);  /* PRKFENCE.MOD */
    resource_load_model("POLCEBAR.MOD", 113, 70);  /* POLCEBAR.MOD */
    resource_load_model("2DCOP1.MOD", 114, 70);    /* 2DCOP1.MOD */
    resource_load_model("2DCOP2.MOD", 115, 70);    /* 2DCOP2.MOD */
    resource_load_model("2DCOP3.MOD", 116, 70);    /* 2DCOP3.MOD */
    resource_load_model("2DCOP4.MOD", 117, 70);    /* 2DCOP4.MOD */
    resource_load_model("HELICOP.MOD", 118, 70);   /* HELICOP.MOD */
    resource_load_model("ROTOR.MOD", 564, 71);     /* ROTOR.MOD */
    resource_load_model("HELISHAD.MOD", 834, 8);   /* HELISHAD.MOD */
    resource_load_animations("HAWK.ADT", 0);       /* HAWK.ADT */
    resource_load_animations("MACE.ADT", 1);       /* MACE.ADT */
    resource_load_animations("SMASHER.ADT", 2);    /* SMASHER.ADT */
    resource_load_animations("ALANA.ADT", 3);      /* ALANA.ADT */
    resource_load_character("HAWK.ACH", 0, 36);    /* HAWK.ACH */
    resource_load_character("MACE.ACH", 1, 37);    /* MACE.ACH */
    resource_load_character("SMASHER.ACH", 2, 38); /* SMASHER.ACH */
    resource_load_character("ALANA.ACH", 3, 39);   /* ALANA.ACH */
    actor_positions[0].x.bits = 0xfa000000u;
    actor_positions[0].z.bits = 0xf2680000u;
    actor_positions[1].x.bits = 0xfa180000u;
    actor_positions[1].z.bits = 0xf1b00000u;
    scene_playback.camera_frames = 550;
    scene_playback.end_tick = 550;
    RESOURCE_VIEW clip0 = resource_find("C1_01.CCT");
    scene_playback.stream[0] = (SCENE_STREAM){clip0.data, clip0.byte_len};
    RESOURCE_VIEW clip1 = resource_find("C1_15.CCT");
    scene_playback.stream[1] = (SCENE_STREAM){clip1.data, clip1.byte_len};
    return clip1.context_address;
}

GDB_CALL uint32 scene_load_resources1(void)
{
    FUNCTION_MARKER(0x80039a10u, "GAME.EXE");
    uint32 arena, byte_len;
    arena = resource_arenas.persistent.context;                           /* 80039A20 */
    resource_load(68, arena, NULL, 0, NULL, &resource_arenas.persistent); /* 80039A24 */
    arena = resource_arenas.persistent.context;                           /* 80039A30 */
    byte_len = resource_io.file_size;                                     /* 80039A38 */
    /* TODO: Investigate allocation beyond the actual native resource owner */
    if (!resource_arenas.persistent.data || (byte_len) > resource_arenas.persistent.available)
    {
        DIAGNOSTIC(fprintf(stderr, "Resource cursor persistent span %u available %u context %08X stage %u tick %u\n", (uint32)(byte_len), resource_arenas.persistent.available, resource_arenas.persistent.context, game_session.stage, game_session.tick));
        diagnostic_stop(__FUNCTION__, __FILE__, __LINE__);
    }
    resource_arenas.persistent.data += byte_len;
    resource_arenas.persistent.available -= byte_len;
    resource_arenas.persistent.context = arena + byte_len;                 /* 80039A48 */
    resource_load_model("BLAST1.MOD", 603, 0);                             /* 80039A58 */
    resource_load_model("BRIGHT01.MOD", 156, 13);                          /* 80039A6C */
    resource_load_model("BRIGHT02.MOD", 157, 13);                          /* 80039A80 */
    resource_load_model("BLEFT02.MOD", 154, 13);                           /* 80039A94 */
    resource_load_model("BLEFT01.MOD", 153, 13);                           /* 80039AA8 */
    resource_load_model("BEND01.MOD", 149, 13);                            /* 80039ABC */
    resource_load_model("BEND02.MOD", 150, 13);                            /* 80039AD0 */
    resource_load_model("BEEND01.MOD", 152, 13);                           /* 80039AE4 */
    resource_load_model("BEND04.MOD", 151, 13);                            /* 80039AF8 */
    resource_load_model("TREE4.MOD", 841, 70);                             /* 80039B0C */
    resource_load_model("TREEFLAT.MOD", 842, 70);                          /* 80039B20 */
    resource_load_model("TRAF2.MOD", 844, 70);                             /* 80039B34 */
    resource_load_model("TRAF1.MOD", 843, 70);                             /* 80039B48 */
    resource_load_model("STREET3.MOD", 845, 70);                           /* 80039B5C */
    resource_load_model("BLAKCAR1.MOD", 846, 70);                          /* 80039B70 */
    resource_load_model("BLUECAR1.MOD", 847, 70);                          /* 80039B84 */
    resource_load_model("TAXI.MOD", 848, 70);                              /* 80039B98 */
    resource_load_model("TRKLOAD4.MOD", 849, 70);                          /* 80039BAC */
    resource_load_model("TRUCK2.MOD", 850, 70);                            /* 80039BC0 */
    scene_playback.end_tick = 500;                                         /* 80039BD0 */
    scene_playback.camera_frames = 400;                                    /* 80039BDC */
    RESOURCE_VIEW clip0 = resource_find("C3_01.CCT");                      /* 80039BE8 */
    scene_playback.stream[0] = (SCENE_STREAM){clip0.data, clip0.byte_len}; /* 80039C04 */
    RESOURCE_VIEW clip1 = resource_find("C3_13.CCT");                      /* 80039C00 */
    scene_playback.stream[1] = (SCENE_STREAM){clip1.data, clip1.byte_len}; /* 80039C14 */
    RESOURCE_VIEW clip2 = resource_find("C3_15.CCT");                      /* 80039C10 */
    scene_playback.stream[2] = (SCENE_STREAM){clip2.data, clip2.byte_len}; /* 80039C24 */
    RESOURCE_VIEW clip3 = resource_find("C3_16.CCT");                      /* 80039C20 */
    scene_playback.stream[3] = (SCENE_STREAM){clip3.data, clip3.byte_len}; /* 80039C34 */
    RESOURCE_VIEW clip4 = resource_find("C3_17.CCT");                      /* 80039C30 */
    scene_playback.stream[4] = (SCENE_STREAM){clip4.data, clip4.byte_len}; /* 80039C44 */
    RESOURCE_VIEW clip5 = resource_find("C3_18.CCT");                      /* 80039C40 */
    scene_playback.stream[5] = (SCENE_STREAM){clip5.data, clip5.byte_len}; /* 80039C54 */
    RESOURCE_VIEW clip6 = resource_find("C3_19.CCT");                      /* 80039C50 */
    scene_playback.stream[6] = (SCENE_STREAM){clip6.data, clip6.byte_len}; /* 80039C64 */
    RESOURCE_VIEW clip7 = resource_find("C3_20.CCT");                      /* 80039C60 */
    scene_playback.stream[7] = (SCENE_STREAM){clip7.data, clip7.byte_len}; /* 80039C68 */
    return clip7.context_address;
}

GDB_CALL uint32 scene_load_resources2(void)
{
    FUNCTION_MARKER(0x80039c88u, "GAME.EXE");
    uint32 arena, byte_len;
    arena = resource_arenas.persistent.context;                           /* 80039C98 */
    resource_load(69, arena, NULL, 0, NULL, &resource_arenas.persistent); /* 80039C9C */
    arena = resource_arenas.persistent.context;                           /* 80039CA8 */
    byte_len = resource_io.file_size;                                     /* 80039CB0 */
    /* TODO: Investigate allocation beyond the actual native resource owner */
    if (!resource_arenas.persistent.data || (byte_len) > resource_arenas.persistent.available)
    {
        DIAGNOSTIC(fprintf(stderr, "Resource cursor persistent span %u available %u context %08X stage %u tick %u\n", (uint32)(byte_len), resource_arenas.persistent.available, resource_arenas.persistent.context, game_session.stage, game_session.tick));
        diagnostic_stop(__FUNCTION__, __FILE__, __LINE__);
    }
    resource_arenas.persistent.data += byte_len;
    resource_arenas.persistent.available -= byte_len;
    resource_arenas.persistent.context = arena + byte_len;                 /* 80039CC0 */
    resource_load_model("CAVEROOF.MOD", 361, 22);                          /* 80039CD0 */
    resource_load_model("NAVBACK1.MOD", 362, 22);                          /* 80039CE4 */
    resource_load_model("NAVEL03.MOD", 363, 22);                           /* 80039CF8 */
    resource_load_model("NAVEL04.MOD", 364, 22);                           /* 80039D0C */
    resource_load_model("BARRIER1.MOD", 627, 70);                          /* 80039D20 */
    resource_load_model("MOTOBOAT.MOD", 853, 7);                           /* 80039D34 */
    resource_load_model("HOVER.MOD", 851, 8);                              /* 80039D48 */
    resource_load_model("SEAHAWK.MOD", 854, 70);                           /* 80039D5C */
    resource_load_model("HELISHAD.MOD", 834, 70);                          /* 80039D70 */
    resource_load_model("ROTOR.MOD", 564, 71);                             /* 80039D84 */
    scene_playback.camera_frames = 400;                                    /* 80039D94 */
    scene_playback.end_tick = 400;                                         /* 80039D9C */
    RESOURCE_VIEW clip0 = resource_find("C5_01.CCT");                      /* 80039DA8 */
    scene_playback.stream[0] = (SCENE_STREAM){clip0.data, clip0.byte_len}; /* 80039DC4 */
    RESOURCE_VIEW clip1 = resource_find("C5_13.CCT");                      /* 80039DC0 */
    scene_playback.stream[1] = (SCENE_STREAM){clip1.data, clip1.byte_len}; /* 80039DD4 */
    RESOURCE_VIEW clip2 = resource_find("C5_15.CCT");                      /* 80039DD0 */
    scene_playback.stream[2] = (SCENE_STREAM){clip2.data, clip2.byte_len}; /* 80039DE4 */
    RESOURCE_VIEW clip3 = resource_find("C5_16.CCT");                      /* 80039DE0 */
    scene_playback.stream[3] = (SCENE_STREAM){clip3.data, clip3.byte_len}; /* 80039DF4 */
    RESOURCE_VIEW clip4 = resource_find("C5_17.CCT");                      /* 80039DF0 */
    scene_playback.stream[4] = (SCENE_STREAM){clip4.data, clip4.byte_len}; /* 80039DF8 */
    return clip4.context_address;
}

GDB_CALL uint32 scene_load_resources3(void)
{
    FUNCTION_MARKER(0x80039e18u, "GAME.EXE");
    uint32 arena, byte_len;
    arena = resource_arenas.persistent.context;                           /* 80039E28 */
    resource_load(70, arena, NULL, 0, NULL, &resource_arenas.persistent); /* 80039E2C */
    arena = resource_arenas.persistent.context;                           /* 80039E38 */
    byte_len = resource_io.file_size;                                     /* 80039E40 */
    /* TODO: Investigate allocation beyond the actual native resource owner */
    if (!resource_arenas.persistent.data || (byte_len) > resource_arenas.persistent.available)
    {
        DIAGNOSTIC(fprintf(stderr, "Resource cursor persistent span %u available %u context %08X stage %u tick %u\n", (uint32)(byte_len), resource_arenas.persistent.available, resource_arenas.persistent.context, game_session.stage, game_session.tick));
        diagnostic_stop(__FUNCTION__, __FILE__, __LINE__);
    }
    resource_arenas.persistent.data += byte_len;
    resource_arenas.persistent.available -= byte_len;
    resource_arenas.persistent.context = arena + byte_len;                 /* 80039E50 */
    resource_load_model("FENCE01.MOD", 327, 21);                           /* 80039E60 */
    resource_load_model("FENCE02.MOD", 328, 21);                           /* 80039E74 */
    resource_load_model("FENCE03.MOD", 329, 21);                           /* 80039E88 */
    resource_load_model("FENCE04.MOD", 330, 21);                           /* 80039E9C */
    resource_load_model("FUEL01.MOD", 348, 21);                            /* 80039EB0 */
    resource_load_model("HANGER01.MOD", 333, 21);                          /* 80039EC4 */
    resource_load_model("HANGER02.MOD", 334, 21);                          /* 80039ED8 */
    resource_load_model("HANGER03.MOD", 335, 21);                          /* 80039EEC */
    resource_load_model("HANGER04.MOD", 336, 21);                          /* 80039F00 */
    resource_load_model("HANGER05.MOD", 337, 21);                          /* 80039F14 */
    resource_load_model("HANGER06.MOD", 338, 21);                          /* 80039F28 */
    resource_load_model("WALL01.MOD", 339, 21);                            /* 80039F3C */
    resource_load_model("FENCE05.MOD", 331, 21);                           /* 80039F50 */
    resource_load_model("CONTRL01.MOD", 340, 21);                          /* 80039F64 */
    resource_load_model("WING.MOD", 856, 70);                              /* 80039F78 */
    scene_playback.camera_frames = 500;                                    /* 80039F88 */
    scene_playback.end_tick = 500;                                         /* 80039F90 */
    RESOURCE_VIEW clip0 = resource_find("C6_01.CCT");                      /* 80039F9C */
    scene_playback.stream[0] = (SCENE_STREAM){clip0.data, clip0.byte_len}; /* 80039FB8 */
    RESOURCE_VIEW clip1 = resource_find("C6_13.CCT");                      /* 80039FB4 */
    scene_playback.stream[1] = (SCENE_STREAM){clip1.data, clip1.byte_len}; /* 80039FBC */
    return clip1.context_address;
}

GDB_CALL uint32 scene_load_resources4(void)
{
    FUNCTION_MARKER(0x80039fdcu, "GAME.EXE");
    uint32 arena, byte_len;
    arena = resource_arenas.persistent.context;                           /* 80039FEC */
    resource_load(71, arena, NULL, 0, NULL, &resource_arenas.persistent); /* 80039FF0 */
    arena = resource_arenas.persistent.context;                           /* 80039FFC */
    byte_len = resource_io.file_size;                                     /* 8003A004 */
    /* TODO: Investigate allocation beyond the actual native resource owner */
    if (!resource_arenas.persistent.data || (byte_len) > resource_arenas.persistent.available)
    {
        DIAGNOSTIC(fprintf(stderr, "Resource cursor persistent span %u available %u context %08X stage %u tick %u\n", (uint32)(byte_len), resource_arenas.persistent.available, resource_arenas.persistent.context, game_session.stage, game_session.tick));
        diagnostic_stop(__FUNCTION__, __FILE__, __LINE__);
    }
    resource_arenas.persistent.data += byte_len;
    resource_arenas.persistent.available -= byte_len;
    resource_arenas.persistent.context = arena + byte_len;                 /* 8003A014 */
    resource_load_model("ISLAND02.MOD", 445, 31);                          /* 8003A024 */
    resource_load_model("CUTHO01.MOD", 852, 31);                           /* 8003A038 */
    resource_load_model("MOTOBOAT.MOD", 853, 70);                          /* 8003A04C */
    resource_load_model("SEAHAWK.MOD", 854, 70);                           /* 8003A060 */
    resource_load_model("HELISHAD.MOD", 834, 70);                          /* 8003A074 */
    resource_load_model("ROTOR.MOD", 564, 71);                             /* 8003A088 */
    scene_playback.camera_frames = 500;                                    /* 8003A098 */
    scene_playback.end_tick = 500;                                         /* 8003A0A0 */
    RESOURCE_VIEW clip0 = resource_find("C7_O1.CCT");                      /* 8003A0AC */
    scene_playback.stream[0] = (SCENE_STREAM){clip0.data, clip0.byte_len}; /* 8003A0C8 */
    RESOURCE_VIEW clip1 = resource_find("C7_13.CCT");                      /* 8003A0C4 */
    scene_playback.stream[1] = (SCENE_STREAM){clip1.data, clip1.byte_len}; /* 8003A0D8 */
    RESOURCE_VIEW clip2 = resource_find("C7_15.CCT");                      /* 8003A0D4 */
    scene_playback.stream[2] = (SCENE_STREAM){clip2.data, clip2.byte_len}; /* 8003A0E8 */
    RESOURCE_VIEW clip3 = resource_find("C7_16.CCT");                      /* 8003A0E4 */
    scene_playback.stream[3] = (SCENE_STREAM){clip3.data, clip3.byte_len}; /* 8003A0F8 */
    RESOURCE_VIEW clip4 = resource_find("C7_17.CCT");                      /* 8003A0F4 */
    scene_playback.stream[4] = (SCENE_STREAM){clip4.data, clip4.byte_len}; /* 8003A108 */
    RESOURCE_VIEW clip5 = resource_find("C7_18.CCT");                      /* 8003A104 */
    scene_playback.stream[5] = (SCENE_STREAM){clip5.data, clip5.byte_len}; /* 8003A10C */
    return clip5.context_address;
}

GDB_CALL uint32 scene_load_resources5(void)
{
    MODEL_REFERENCE model0;
    FUNCTION_MARKER(0x8003a12cu, "GAME.EXE");
    uint32 arena, byte_len;
    arena = resource_arenas.persistent.context;                           /* 8003A13C */
    resource_load(72, arena, NULL, 0, NULL, &resource_arenas.persistent); /* 8003A140 */
    arena = resource_arenas.persistent.context;                           /* 8003A14C */
    byte_len = resource_io.file_size;                                     /* 8003A154 */
    /* TODO: Investigate allocation beyond the actual native resource owner */
    if (!resource_arenas.persistent.data || (byte_len) > resource_arenas.persistent.available)
    {
        DIAGNOSTIC(fprintf(stderr, "Resource cursor persistent span %u available %u context %08X stage %u tick %u\n", (uint32)(byte_len), resource_arenas.persistent.available, resource_arenas.persistent.context, game_session.stage, game_session.tick));
        diagnostic_stop(__FUNCTION__, __FILE__, __LINE__);
    }
    resource_arenas.persistent.data += byte_len;
    resource_arenas.persistent.available -= byte_len;
    resource_arenas.persistent.context = arena + byte_len;                 /* 8003A164 */
    resource_load_model("WRECK1.MOD", 898, 71);                            /* 8003A174 */
    resource_load_model("WRECK2.MOD", 899, 71);                            /* 8003A188 */
    resource_load_model("WRECK3.MOD", 900, 71);                            /* 8003A19C */
    resource_load_model("WRECK4.MOD", 901, 71);                            /* 8003A1B0 */
    resource_load_model("BLAST1.MOD", 603, 0);                             /* 8003A1C4 */
    model0 = model_refs[603];                                              /* 8003A1D4 */
    model_refs[602] = model0;                                              /* 8003A1DC */
    resource_load_model("ISLAND02.MOD", 445, 31);                          /* 8003A1EC */
    resource_load_model("CUTWI01.MOD", 856, 31);                           /* 8003A200 */
    resource_load_model("CUTWI01E.MOD", 857, 31);                          /* 8003A214 */
    resource_load_model("HELIBIKE.MOD", 858, 70);                          /* 8003A228 */
    resource_load_model("HELIBIK2.MOD", 859, 70);                          /* 8003A23C */
    scene_playback.camera_frames = 450;                                    /* 8003A24C */
    scene_playback.end_tick = 450;                                         /* 8003A254 */
    RESOURCE_VIEW clip0 = resource_find("C8_01.CCT");                      /* 8003A260 */
    scene_playback.stream[0] = (SCENE_STREAM){clip0.data, clip0.byte_len}; /* 8003A27C */
    RESOURCE_VIEW clip1 = resource_find("C8_13.CCT");                      /* 8003A278 */
    scene_playback.stream[1] = (SCENE_STREAM){clip1.data, clip1.byte_len}; /* 8003A28C */
    RESOURCE_VIEW clip2 = resource_find("C8_15.CCT");                      /* 8003A288 */
    scene_playback.stream[2] = (SCENE_STREAM){clip2.data, clip2.byte_len}; /* 8003A29C */
    RESOURCE_VIEW clip3 = resource_find("C8_16.CCT");                      /* 8003A298 */
    scene_playback.stream[3] = (SCENE_STREAM){clip3.data, clip3.byte_len}; /* 8003A2A0 */
    return clip3.context_address;
}

GDB_CALL uint32 scene_load_resources6(void)
{
    MODEL_REFERENCE model0;
    FUNCTION_MARKER(0x8003a2c0u, "GAME.EXE");
    uint32 arena, byte_len;
    arena = resource_arenas.persistent.context;                           /* 8003A2D0 */
    resource_load(73, arena, NULL, 0, NULL, &resource_arenas.persistent); /* 8003A2D4 */
    arena = resource_arenas.persistent.context;                           /* 8003A2E0 */
    byte_len = resource_io.file_size;                                     /* 8003A2E8 */
    /* TODO: Investigate allocation beyond the actual native resource owner */
    if (!resource_arenas.persistent.data || (byte_len) > resource_arenas.persistent.available)
    {
        DIAGNOSTIC(fprintf(stderr, "Resource cursor persistent span %u available %u context %08X stage %u tick %u\n", (uint32)(byte_len), resource_arenas.persistent.available, resource_arenas.persistent.context, game_session.stage, game_session.tick));
        diagnostic_stop(__FUNCTION__, __FILE__, __LINE__);
    }
    resource_arenas.persistent.data += byte_len;
    resource_arenas.persistent.available -= byte_len;
    resource_arenas.persistent.context = arena + byte_len;                 /* 8003A2F8 */
    resource_load_model("WRECK1.MOD", 898, 71);                            /* 8003A308 */
    resource_load_model("WRECK2.MOD", 899, 71);                            /* 8003A31C */
    resource_load_model("WRECK3.MOD", 900, 71);                            /* 8003A330 */
    resource_load_model("WRECK4.MOD", 901, 71);                            /* 8003A344 */
    resource_load_model("BLAST1.MOD", 603, 0);                             /* 8003A358 */
    model0 = model_refs[603];                                              /* 8003A368 */
    model_refs[602] = model0;                                              /* 8003A370 */
    resource_load_model("ISLAND01.MOD", 444, 31);                          /* 8003A380 */
    resource_load_model("ISLANDEX.MOD", 446, 31);                          /* 8003A394 */
    resource_load_model("ISLANDT.MOD", 447, 31);                           /* 8003A3A8 */
    resource_load_model("ISLFRAG1.MOD", 448, 31);                          /* 8003A3BC */
    resource_load_model("HELIBIKE.MOD", 858, 70);                          /* 8003A3D0 */
    resource_load_model("HELIBIK2.MOD", 859, 70);                          /* 8003A3E4 */
    resource_load_model("SEAHAWK.MOD", 854, 70);                           /* 8003A3F8 */
    resource_load_model("SEAHAWK2.MOD", 855, 70);                          /* 8003A40C */
    resource_load_model("HELISHAD.MOD", 834, 70);                          /* 8003A420 */
    resource_load_model("ROTOR.MOD", 564, 71);                             /* 8003A434 */
    scene_playback.end_tick = 800;                                         /* 8003A444 */
    scene_playback.camera_frames = 600;                                    /* 8003A450 */
    RESOURCE_VIEW clip0 = resource_find("C9_01.CCT");                      /* 8003A45C */
    scene_playback.stream[0] = (SCENE_STREAM){clip0.data, clip0.byte_len}; /* 8003A478 */
    RESOURCE_VIEW clip1 = resource_find("C9_13.CCT");                      /* 8003A474 */
    scene_playback.stream[1] = (SCENE_STREAM){clip1.data, clip1.byte_len}; /* 8003A488 */
    RESOURCE_VIEW clip2 = resource_find("C9_15.CCT");                      /* 8003A484 */
    scene_playback.stream[2] = (SCENE_STREAM){clip2.data, clip2.byte_len}; /* 8003A498 */
    RESOURCE_VIEW clip3 = resource_find("C9_16.CCT");                      /* 8003A494 */
    scene_playback.stream[3] = (SCENE_STREAM){clip3.data, clip3.byte_len}; /* 8003A4A8 */
    RESOURCE_VIEW clip4 = resource_find("C9_17.CCT");                      /* 8003A4A4 */
    scene_playback.stream[4] = (SCENE_STREAM){clip4.data, clip4.byte_len}; /* 8003A4B8 */
    RESOURCE_VIEW clip5 = resource_find("C9_18.CCT");                      /* 8003A4B4 */
    scene_playback.stream[5] = (SCENE_STREAM){clip5.data, clip5.byte_len}; /* 8003A4BC */
    return clip5.context_address;
}

GDB_CALL uint32 scene_load_resources7(void)
{
    FUNCTION_MARKER(0x8003a4d4u, "GAME.EXE");
    uint32 arena, byte_len;
    arena = resource_arenas.persistent.context;                           /* 8003A4E4 */
    resource_load(74, arena, NULL, 0, NULL, &resource_arenas.persistent); /* 8003A4E8 */
    arena = resource_arenas.persistent.context;                           /* 8003A4F4 */
    byte_len = resource_io.file_size;                                     /* 8003A4FC */
    /* TODO: Investigate allocation beyond the actual native resource owner */
    if (!resource_arenas.persistent.data || (byte_len) > resource_arenas.persistent.available)
    {
        DIAGNOSTIC(fprintf(stderr, "Resource cursor persistent span %u available %u context %08X stage %u tick %u\n", (uint32)(byte_len), resource_arenas.persistent.available, resource_arenas.persistent.context, game_session.stage, game_session.tick));
        diagnostic_stop(__FUNCTION__, __FILE__, __LINE__);
    }
    resource_arenas.persistent.data += byte_len;
    resource_arenas.persistent.available -= byte_len;
    resource_arenas.persistent.context = arena + byte_len;                 /* 8003A50C */
    resource_load_model("NEWSPAPR.MOD", 450, 70);                          /* 8003A51C */
    resource_load_model("CELL01.MOD", 449, 7);                             /* 8003A530 */
    resource_load_model("PRISON01.MOD", 451, 7);                           /* 8003A544 */
    resource_load_model("SERCHLIT.MOD", 452, 70);                          /* 8003A558 */
    resource_load_model("SEAHAWK.MOD", 854, 70);                           /* 8003A56C */
    resource_load_model("HELISHAD.MOD", 834, 70);                          /* 8003A580 */
    resource_load_model("ROTOR.MOD", 564, 71);                             /* 8003A594 */
    resource_load_model("ZENG.MOD", 453, 46);                              /* 8003A5A8 */
    scene_playback.camera_frames = 600;                                    /* 8003A5B8 */
    scene_playback.end_tick = 600;                                         /* 8003A5C0 */
    RESOURCE_VIEW clip0 = resource_find("C11_01.CCT");                     /* 8003A5CC */
    scene_playback.stream[0] = (SCENE_STREAM){clip0.data, clip0.byte_len}; /* 8003A5E8 */
    RESOURCE_VIEW clip1 = resource_find("C11_15.CCT");                     /* 8003A5E4 */
    scene_playback.stream[1] = (SCENE_STREAM){clip1.data, clip1.byte_len}; /* 8003A5F8 */
    RESOURCE_VIEW clip2 = resource_find("C11_16.CCT");                     /* 8003A5F4 */
    scene_playback.stream[2] = (SCENE_STREAM){clip2.data, clip2.byte_len}; /* 8003A608 */
    RESOURCE_VIEW clip3 = resource_find("C11_17.CCT");                     /* 8003A604 */
    scene_playback.stream[3] = (SCENE_STREAM){clip3.data, clip3.byte_len}; /* 8003A618 */
    RESOURCE_VIEW clip4 = resource_find("C11_18.CCT");                     /* 8003A614 */
    scene_playback.stream[4] = (SCENE_STREAM){clip4.data, clip4.byte_len}; /* 8003A628 */
    RESOURCE_VIEW clip5 = resource_find("C11_19.CCT");                     /* 8003A624 */
    scene_playback.stream[5] = (SCENE_STREAM){clip5.data, clip5.byte_len}; /* 8003A638 */
    RESOURCE_VIEW clip6 = resource_find("C11_20.CCT");                     /* 8003A634 */
    scene_playback.stream[6] = (SCENE_STREAM){clip6.data, clip6.byte_len}; /* 8003A648 */
    RESOURCE_VIEW clip7 = resource_find("C12_01.CCT");                     /* 8003A644 */
    scene_playback.stream[7] = (SCENE_STREAM){clip7.data, clip7.byte_len}; /* 8003A64C */
    return clip7.context_address;
}

GDB_CALL uint32 scene_load_resources8(void)
{
    FUNCTION_MARKER(0x8003a664u, "GAME.EXE");
    uint32 arena, byte_len;
    arena = resource_arenas.persistent.context;                           /* 8003A670 */
    resource_load(75, arena, NULL, 0, NULL, &resource_arenas.persistent); /* 8003A674 */
    arena = resource_arenas.persistent.context;                           /* 8003A680 */
    byte_len = resource_io.file_size;                                     /* 8003A688 */
    /* TODO: Investigate allocation beyond the actual native resource owner */
    if (!resource_arenas.persistent.data || (byte_len) > resource_arenas.persistent.available)
    {
        DIAGNOSTIC(fprintf(stderr, "Resource cursor persistent span %u available %u context %08X stage %u tick %u\n", (uint32)(byte_len), resource_arenas.persistent.available, resource_arenas.persistent.context, game_session.stage, game_session.tick));
        diagnostic_stop(__FUNCTION__, __FILE__, __LINE__);
    }
    resource_arenas.persistent.data += byte_len;
    resource_arenas.persistent.available -= byte_len;
    resource_arenas.persistent.context = arena + byte_len; /* 8003A698 */
    return arena + byte_len;
}

/* 4799C: signed upper clamp only; all fixed-point shifts use unsigned bits
 * to preserve MIPS wrapping without shifting negative signed C values */
GDB_CALL sint32 scene_apply_camera_frame(sint32 frame)
{
    FUNCTION_MARKER(0x8004799c, "GAME.EXE");
    uint32 yaw;
    sint32 limit = (sint32)scene_playback.camera_frames;
    if (frame >= limit)
        frame = (uint32)limit - 1;
    const SCENE_STREAM *clip = scene_playback.stream;
    /* TODO: Investigate original CCT reads outside the resource entry */
    if (!clip->data || ((uintptr_t)clip->data & 1) || clip->byte_len < 2u + 10u || frame < 0 || (uint32)frame > (clip->byte_len - 2u - 10u) / sizeof(SCENE_FRAME))
    {
        DIAGNOSTIC(fprintf(stderr, "Invalid CCT camera frame %d bytes %u stage %u tick %u\n", frame, clip->byte_len, game_session.stage, game_session.tick));
        diagnostic_stop(__FUNCTION__, __FILE__, __LINE__);
    }
    const SCENE_FRAME *src = (const SCENE_FRAME *)(clip->data + 2u + sizeof(SCENE_FRAME) * (uint32)frame);
    camera.pos.x.bits = (uint32)src->x << 16;
    camera.pos.y.bits = (0u - src->y) << 16;
    camera.pos.z.bits = (0u - src->z) << 16;
    camera.angles.yaw_bits = src->yaw;
    yaw = 0u - src->pitch;
    camera.angles.pitch_bits = yaw;
    return yaw;
}

/* 47A40 does not clamp the frame; its final halfword store is in jr's slot */
GDB_CALL uint32 scene_apply_object_frame(sint32 frame, uint32 object_idx, uint32 stream)
{
    FUNCTION_MARKER(0x80047a40, "GAME.EXE");
    uint32 result;
    /* TODO: Investigate original source-table indices outside its eight slots */
    if (stream >= SCENE_STREAM_COUNT)
    {
        DIAGNOSTIC(fprintf(stderr, "Invalid CCT stream %u frame %d stage %u tick %u\n", stream, frame, game_session.stage, game_session.tick));
        diagnostic_stop(__FUNCTION__, __FILE__, __LINE__);
    }
    const SCENE_STREAM *clip = scene_playback.stream + stream;
    /* TODO: Investigate original CCT reads outside the resource entry */
    if (!clip->data || ((uintptr_t)clip->data & 1) || clip->byte_len < 2u + 12u || frame < 0 || (uint32)frame > (clip->byte_len - 2u - 12u) / sizeof(SCENE_FRAME))
    {
        DIAGNOSTIC(fprintf(stderr, "Invalid CCT frame %d stream %u bytes %u stage %u tick %u\n", frame, stream, clip->byte_len, game_session.stage, game_session.tick));
        diagnostic_stop(__FUNCTION__, __FILE__, __LINE__);
    }
    const SCENE_FRAME *src = (const SCENE_FRAME *)(clip->data + 2u + sizeof(SCENE_FRAME) * (uint32)frame);
    /* TODO: Investigate CCT destinations outside the native world pool */
    if (object_idx >= WORLD_OBJECT_COUNT)
    {
        DIAGNOSTIC(fprintf(stderr, "Invalid CCT world slot %u frame %d stream %u stage %u tick %u\n", object_idx, frame, stream, game_session.stage, game_session.tick));
        diagnostic_stop(__FUNCTION__, __FILE__, __LINE__);
    }
    MODEL_OBJECT *item = world_objects.object + object_idx;
    item->pos.x.bits = (uint32)src->x << 16;
    item->pos.y.bits = (0u - src->y) << 16;
    item->pos.z.bits = (0u - src->z) << 16;
    item->angles.yaw_bits = src->yaw;
    item->angles.pitch_bits = src->pitch;
    result = src->roll;
    item->angles.roll_bits = result;
    return result;
}

GDB_CALL sint32 scene_update_prison_ending(uint32 frame)
{
    FUNCTION_MARKER(0x8003b5f0u, "GAME.EXE");
    uint32 tick = game_session.tick;
    if ((sint32)tick < 300)
    {
        world_objects.count = 7;
        world_objects.object[1].kind = 854;
        scene_apply_object_frame(frame, 1, 1);
        world_objects.object[2].kind = 854;
        scene_apply_object_frame(frame, 2, 2);
        world_objects.object[3].kind = 452;
        scene_apply_object_frame(frame, 3, 3);
        world_objects.object[4].kind = 452;
        scene_apply_object_frame(frame, 4, 4);
        world_objects.object[5].kind = 452;
        scene_apply_object_frame(frame, 5, 5);
        world_objects.object[6].kind = 452;
        return scene_apply_object_frame(frame, 6, 6);
    }
    scene_playback.stream[0] = scene_playback.stream[7];
    scene_apply_camera_frame(tick - 300u);
    world_objects.count = 2;
    /* Original memset clears both complete model records before their new roles */
    world_objects.object[0] = (MODEL_OBJECT){0};
    world_objects.object[1] = (MODEL_OBJECT){0};
    MODEL_OBJECT *item = world_objects.object;
    item->kind = 449;
    item->pos.x.bits = 0;
    item->pos.z.bits = 0;
    item = world_objects.object + 1;
    item->kind = 453;
    item->pos.x.bits = 0x00f00000;
    item->pos.y.bits = 0x00280000;
    item->pos.z.bits = 0;
    item->angles.yaw_bits = 8192;
    return 8192;
}

GDB_CALL uint32 scene_update_hovercraft_fleet(sint32 frame)
{
    FUNCTION_MARKER(0x8003a934u, "GAME.EXE");
    if ((sint32)camera.pos.y.bits > (sint32)0xff300000u)
        camera.pos.y.bits = 0xff300000u;
    world_objects.count = 4;
    world_objects.object[0].kind = 851;
    scene_apply_object_frame(frame, 0, 1);
    world_objects.object[1].kind = 854;
    scene_apply_object_frame(frame, 1, 2);
    world_objects.object[2].kind = 854;
    scene_apply_object_frame(frame, 2, 3);
    world_objects.object[3].kind = 853;
    return scene_apply_object_frame(frame, 3, 4);
}

GDB_CALL uint32 scene_update_wing(sint32 frame)
{
    FUNCTION_MARKER(0x8003aa00u, "GAME.EXE");
    camera.angles.pitch_bits = (uint16)((uint32)(uint16)camera.angles.pitch - 2048u);
    world_objects.count = 17;
    world_objects.object[16].kind = 856;
    return scene_apply_object_frame(frame, 16, 1);
}

GDB_CALL uint32 scene_update_island_fleet(sint32 frame)
{
    FUNCTION_MARKER(0x8003aa54u, "GAME.EXE");
    world_objects.count = 6;
    world_objects.object[1].kind = 852;
    scene_apply_object_frame(frame, 1, 1);
    world_objects.object[2].kind = 854;
    scene_apply_object_frame(frame, 2, 2);
    world_objects.object[3].kind = 854;
    scene_apply_object_frame(frame, 3, 3);
    world_objects.object[4].kind = 853;
    scene_apply_object_frame(frame, 4, 4);
    world_objects.object[5].kind = 854;
    return scene_apply_object_frame(frame, 5, 5);
}

GDB_CALL sint32 scene_update_traffic_explosions(uint32 frame)
{
    FUNCTION_MARKER(0x8003a700u, "GAME.EXE");
    camera.pos.y.bits = (uint32)((sint32)camera.pos.y.bits >> 1);
    if ((sint32)game_session.tick >= (sint32)scene_playback.camera_frames)
        return 0;
    world_objects.count = 33;
    MODEL_OBJECT *item = world_objects.object + 26;
    item->kind = 848;
    scene_apply_object_frame(frame, 26, 1);
    if (game_session.tick == 202)
    {
        effect_spawn(892, item->pos.x.bits, 0xffc00000, item->pos.z.bits);
        effect_spawn(603, item->pos.x.bits, 0, item->pos.z.bits);
        disp_buffers.shake_ticks = 8;
        disp_buffers.flash_ticks = 4;
    }
    world_objects.object[27].kind = 850;
    scene_apply_object_frame(frame, 27, 2);
    world_objects.object[28].kind = 849;
    scene_apply_object_frame(frame, 28, 3);
    item = world_objects.object + 29;
    item->kind = 848;
    scene_apply_object_frame(frame, 29, 4);
    if (game_session.tick == 298)
    {
        effect_spawn(892, item->pos.x.bits, 0xffc00000, item->pos.z.bits);
        effect_spawn(603, item->pos.x.bits, 0, item->pos.z.bits);
        disp_buffers.shake_ticks = 8;
        disp_buffers.flash_ticks = 4;
    }
    item = world_objects.object + 30;
    item->kind = 847;
    scene_apply_object_frame(frame, 30, 5);
    if (game_session.tick == 254)
    {
        effect_spawn(892, item->pos.x.bits, 0xffc00000, item->pos.z.bits);
        effect_spawn(603, item->pos.x.bits, 0, item->pos.z.bits);
        disp_buffers.shake_ticks = 8;
        disp_buffers.flash_ticks = 4;
    }
    world_objects.object[31].kind = 846;
    scene_apply_object_frame(frame, 31, 6);
    world_objects.object[32].kind = 846;
    return scene_apply_object_frame(frame, 32, 7);
}

GDB_CALL uint32 scene_update_wing_destruction(uint32 frame)
{
    FUNCTION_MARKER(0x8003ab18u, "GAME.EXE");
    FIXED_VEC3 point;
    uint32 x, y, z, kind, tick;
    int i;
    world_objects.count = 4;
    scene_apply_object_frame(frame, 1, 1);
    MODEL_OBJECT *item = world_objects.object + 1;
    transform_push();
    transform_identity();
    transform_roll(item->angles.roll);
    transform_pitch(item->angles.pitch);
    transform_yaw(-item->angles.yaw);
    if (frame == 136 || frame == 168)
    {
        transform_point(frame == 168 ? (sint32)0xff080000u : 0x00f80000, 0, -0x1580000, point.bits);
        x = item->pos.x.bits + point.x.bits;
        y = item->pos.y.bits + point.y.bits;
        z = item->pos.z.bits + point.z.bits;
        effect_spawn(893, x, y, z);
        for (frame = 0; frame < 8; ++frame)
        {
            kind = (random_next() & 3) + 898;
            effect_spawn_debris(kind, x, y, z);
        }
        disp_buffers.shake_ticks = 8;
        disp_buffers.flash_ticks = 8;
    }
    /* Preserve frame register reuse by debris loops for later tests and CCT calls */
    if (frame - 137u < 262u)
    {
        if (!(frame & 1))
            for (i = 0; i < 5; ++i)
            {
                x = ((random_next() & 63) - 32) << 19;
                transform_point(x, -1048576, -22544384, point.bits);
                dynamic_objects_spawn(575, item->pos.x.bits + point.x.bits, item->pos.y.bits + point.y.bits, item->pos.z.bits + point.z.bits);
            }
        if (!(frame & 3))
        {
            transform_point(16252928, -1048576, -22544384, point.bits);
            dynamic_objects_spawn(576, item->pos.x.bits + point.x.bits, item->pos.y.bits + point.y.bits, item->pos.z.bits + point.z.bits);
            if ((sint32)frame >= 185)
            {
                transform_point(-16252928, -1048576, -22544384, point.bits);
                dynamic_objects_spawn(576, item->pos.x.bits + point.x.bits, item->pos.y.bits + point.y.bits, item->pos.z.bits + point.z.bits);
            }
        }
    }
    if (frame == 320 || frame == 352)
    {
        sound_voice_play(19, 128, 6144);
        effect_spawn(892, item->pos.x.bits, item->pos.y.bits, item->pos.z.bits);
        for (frame = 0; frame < 8; ++frame)
        {
            kind = (random_next() & 3) + 898;
            effect_spawn_debris(kind, item->pos.x.bits, item->pos.y.bits, item->pos.z.bits);
        }
        disp_buffers.flash_ticks = 4;
    }
    if (frame == 399)
    {
        effect_spawn(891, item->pos.x.bits, 0, item->pos.z.bits);
        effect_spawn(602, item->pos.x.bits, 0, item->pos.z.bits);
        for (frame = 0; frame < 64; ++frame)
        {
            kind = (random_next() & 3) + 898;
            effect_spawn_debris(kind, item->pos.x.bits, item->pos.y.bits, item->pos.z.bits);
        }
        disp_buffers.shake_ticks = 8;
        disp_buffers.flash_ticks = 8;
    }
    transform_pop();
    tick = game_session.tick;
    kind = 856;
    if ((sint32)tick >= 144 && ((sint32)tick >= 192 || !(tick & 8)))
        kind = 857;
    item->kind = kind;
    world_objects.object[2].kind = 858;
    scene_apply_object_frame(frame, 2, 2);
    world_objects.object[3].kind = 859;
    return scene_apply_object_frame(frame, 3, 3);
}

GDB_CALL uint32 scene_update_tower_destruction(uint32 frame)
{
    FUNCTION_MARKER(0x8003aed8u, "GAME.EXE");
    uint32 tick, y, kind, val, phase;
    int i;
    world_objects.count = 6;
    tick = game_session.tick;
    world_objects.object[0].kind = (sint32)tick < 500 ? 444 : 446;
    MODEL_OBJECT *item = world_objects.object + 1;
    if (tick - 501u < 129u)
    {
        val = item->pos.y.bits + ((tick - 500u) << 13);
        item->pos.y.bits = val;
        if ((sint32)val > 0x02f00000)
            item->pos.y.bits = 0x02f00000;
        item->angles.roll_bits = (uint16)((uint16)item->angles.roll + ((sint32)(game_session.tick - 500u) >> 1));
    }
    y = item->pos.y.bits;
    if (!(game_session.tick & 1))
    {
        dynamic_objects_spawn(575, 0xffb00000, 0xff400000, 0xfa800000);
        dynamic_objects_spawn(575, 0x00b80000, 0xff400000, 0xfb580000);
        dynamic_objects_spawn(575, 0xffb00000, 0xff400000, 0xfc700000);
        dynamic_objects_spawn(575, 0x00380000, 0xff400000, 0xfda00000);
    }
    if (!(game_session.tick & 31))
    {
        dynamic_objects_spawn(576, 0xffb00000, 0xff400000, 0xfa800000);
        dynamic_objects_spawn(576, 0x00b80000, 0xff400000, 0xfb580000);
        dynamic_objects_spawn(576, 0xffb00000, 0xff400000, 0xfc700000);
        dynamic_objects_spawn(576, 0x00380000, 0xff400000, 0xfda00000);
    }
    if (game_session.tick - 201u < 294u)
    {
        phase = random_next() & 2047;
        dynamic_objects_spawn(575, item->pos.x.bits + ((uint32)(sint32)sin_table[phase] << 9), y + 0xfa800000u, item->pos.z.bits - ((uint32)(sint32)sin_table[MATH_QUARTER_TURN + phase] << 9));
    }
    if (game_session.tick - 496u < 134u)
    {
        for (i = 0; i < 2; i++)
        {
            phase = random_next() & 2047;
            dynamic_objects_spawn(575, item->pos.x.bits + ((uint32)(sint32)sin_table[phase] << 9), y + 0xfa800000u, item->pos.z.bits - ((uint32)(sint32)sin_table[MATH_QUARTER_TURN + phase] << 9));
        }
        phase = random_next() & 2047;
        dynamic_objects_spawn(576, item->pos.x.bits + ((uint32)(sint32)sin_table[phase] << 9), y + 0xfa800000u, item->pos.z.bits - ((uint32)(sint32)sin_table[MATH_QUARTER_TURN + phase] << 9));
    }
    item->kind = (sint32)game_session.tick < 632 ? 447 : 55;
    tick = game_session.tick;
    if (tick == 470)
    {
        sound_voice_play(19, 192, 6144);
        effect_spawn(893, 0x00600000, 0xfd200000, 0x02b00000);
        tick = game_session.tick;
    }
    if (tick == 475)
    {
        sound_voice_play(19, 192, 6144);
        effect_spawn(893, 0x00600000, 0xfcc00000, 0x02b00000);
    }
    tick = game_session.tick;
    if (tick == 480)
    {
        sound_voice_play(19, 192, 6144);
        effect_spawn(893, 0x00600000, 0xfc800000, 0x02b00000);
        tick = game_session.tick;
    }
    if (tick == 485)
    {
        sound_voice_play(19, 192, 6144);
        effect_spawn(893, 0x00600000, 0xfc400000, 0x02b00000);
    }
    tick = game_session.tick;
    if (tick == 490)
    {
        sound_voice_play(19, 192, 6144);
        effect_spawn(893, 0x00600000, 0xfc000000, 0x02b00000);
        tick = game_session.tick;
    }
    if (tick == 500)
    {
        sound_voice_play(19, 255, 6144);
        effect_spawn(893, 0x00600000, 0xfc800000, 0x02b00000);
        effect_spawn(603, 0x00600000, 0xfc800000, 0x02b00000);
        disp_buffers.shake_ticks = 16;
        disp_buffers.flash_ticks = 16;
        dynamic_objects_spawn_oriented(448, 0x00880000, 0xfc400000, 0x02880000, 0x2000);
        dynamic_objects_spawn_oriented(448, 0x00380000, 0xfc400000, 0x02d80000, 0xa000);
        dynamic_objects_spawn_oriented(448, 0x00900000, 0xfc400000, 0x02e00000, 0x6000);
        dynamic_objects_spawn_oriented(448, 0x00380000, 0xfc400000, 0x02800000, 0xe000);
        dynamic_objects_spawn_oriented(448, 0x00880000, 0xfd200000, 0x02880000, 0x2000);
        dynamic_objects_spawn_oriented(448, 0x00380000, 0xfd200000, 0x02d80000, 0xa000);
        dynamic_objects_spawn_oriented(448, 0x00900000, 0xfd200000, 0x02e00000, 0x6000);
        dynamic_objects_spawn_oriented(448, 0x00380000, 0xfd200000, 0x02800000, 0xe000);
    }
    tick = game_session.tick;
    if (tick == 600 || tick == 620)
    {
        sound_voice_play(19, 192, 6144);
        effect_spawn(892, 0x02000000, y + 0xfb000000u, item->pos.z.bits);
        effect_spawn(603, 0x02000000, y + 0xfb000000u, item->pos.z.bits);
        disp_buffers.shake_ticks = 8;
        disp_buffers.flash_ticks = 8;
        tick = game_session.tick;
    }
    if (tick == 630)
    {
        sound_voice_play(19, 255, 6144);
        effect_spawn(891, 0x02800000, y + 0xfb000000u, item->pos.z.bits);
        effect_spawn(602, 0x02800000, y + 0xfb000000u, item->pos.z.bits);
        /* Preserve the input frame register reuse as the debris counter */
        for (frame = 0; frame < 64; ++frame)
        {
            kind = (random_next() & 3) + 898;
            effect_spawn_debris(kind, 0x02800000, y + 0xfb000000u, item->pos.z.bits);
        }
        disp_buffers.shake_ticks = 16;
        disp_buffers.flash_ticks = 16;
    }
    if ((sint32)game_session.tick >= (sint32)scene_playback.camera_frames)
        return 0;
    world_objects.object[2].kind = 854;
    scene_apply_object_frame(frame, 2, 2);
    world_objects.object[3].kind = 858;
    scene_apply_object_frame(frame, 3, 3);
    world_objects.object[4].kind = 858;
    scene_apply_object_frame(frame, 4, 4);
    world_objects.object[5].kind = 855;
    return scene_apply_object_frame(frame, 5, 5);
}

GDB_CALL uint32 scene_update_intro_helicopter(sint32 frame)
{
    FUNCTION_MARKER(0x8003a6acu, "GAME.EXE");
    camera.angles.pitch_bits = (uint16)((uint32)(uint16)camera.angles.pitch - 1024u);
    world_objects.count = 65;
    world_objects.object[64].kind = 118;
    return scene_apply_object_frame(frame, 64, 1);
}

/* Characters and shadows become visible from sequence tick401 */
static sint32 draw_characters(void)
{
    FUNCTION_MARKER(0x8005A8F4, "GAME.EXE");
    uint32 i, type, table;
    sint32 result;
    transform_identity();
    transform_pitch(camera.angles.pitch);
    transform_yaw(camera.angles.yaw);
    result = (sint32)game_session.player_count;
    for (i = 0; (sint32)i < (sint32)game_session.player_count; i++)
    {
        /* TODO: Investigate a scene character outside the native actor pool */
        if (i >= ACTOR_COUNT)
        {
            DIAGNOSTIC(fprintf(stderr, "Invalid scene character %u players %u stage %u tick %u\n", i, game_session.player_count, game_session.stage, game_session.tick));
            diagnostic_stop(__FUNCTION__, __FILE__, __LINE__);
        }
        type = (uint32)actor_types[i];
        /* TODO: Investigate a scene character without a valid animation type */
        if (type >= ANIMATION_TYPE_COUNT)
        {
            DIAGNOSTIC(fprintf(stderr, "Invalid scene animation type %u actor %u stage %u tick %u\n", type, i, game_session.stage, game_session.tick));
            diagnostic_stop(__FUNCTION__, __FILE__, __LINE__);
        }
        table = animation_type_banks[type];
        actor_orientation[i].yaw = 512;
        if (actor_animations[i].frame == 0xffffffffu)
            animation_start(i, table, 69);
        if (actor_animations[i].frame != 0xffffffffu)
            animation_update_motion(i);
        transform_push();
        type = (uint32)actor_types[i];
        /* TODO: Investigate a changed scene character type before height access */
        if (type >= MODEL_TYPE_COUNT)
        {
            DIAGNOSTIC(fprintf(stderr, "Invalid scene height type %u actor %u stage %u tick %u\n", type, i, game_session.stage, game_session.tick));
            diagnostic_stop(__FUNCTION__, __FILE__, __LINE__);
        }
        transform_translate(actor_positions[i].x.bits - camera.pos.x.bits, actor_positions[i].y.bits + model_height[type] - camera.pos.y.bits, actor_positions[i].z.bits - camera.pos.z.bits);
        transform_yaw((sint32)(sint16)(0u - 32u * (uint32)(sint32)actor_orientation[i].yaw) + 32768);
        transform_translate(0, actor_animations[i].motion.bits[0], actor_animations[i].motion.forward_offset_bits);
        type = (uint32)actor_types[i];
        /* TODO: Investigate a changed scene character type before model access */
        if (type >= MODEL_TYPE_COUNT)
        {
            DIAGNOSTIC(fprintf(stderr, "Invalid scene model type %u actor %u stage %u tick %u\n", type, i, game_session.stage, game_session.tick));
            diagnostic_stop(__FUNCTION__, __FILE__, __LINE__);
        }
        model_draw_pose(model_refs[type], &actor_poses[i]);
        transform_pop();
        transform_push();
        transform_translate(actor_positions[i].x.bits - camera.pos.x.bits, 0u - camera.pos.y.bits, actor_positions[i].z.bits - camera.pos.z.bits);
        transform_pitch(32768);
        transform_yaw((sint16)(32u * (uint32)(sint32)actor_orientation[i].yaw));
        model_draw_menu(model_refs[860], 0, 0);
        transform_pop();
        result = (sint32)(i + 1) < (sint32)game_session.player_count;
    }
    return result;
}

GDB_CALL sint32 sequence_run(sint32 sequence, void (*prepare_frame)(void))
{
    const RESOURCE_LOADER *tex;
    uint32 old_stage, stage, i;
    uint32 yaw, pitch, camera_x, camera_y, camera_z, end_tick, tick;
    sint32 remaining;
    WORLD_OBJECT_TYPE_GROUP_ID types;
    sint32 steps;
    sint32 track = 8;
    FUNCTION_MARKER(0x80015720, "GAME.EXE");
    /* TODO: Investigate an original index reading beyond the eight scene steps */
    if (sequence < 0 || sequence >= SCENE_SEQUENCE_COUNT)
    {
        DIAGNOSTIC(fprintf(stderr, "Invalid scene sequence %d stage %u tick %u\n", sequence, game_session.stage, game_session.tick));
        diagnostic_stop(__FUNCTION__, __FILE__, __LINE__);
    }
    steps = loading_steps[sequence];
    old_stage = game_session.stage;
    loading_begin(steps);
    game_session.stage = (uint32)sequence + 27;
    resource_arenas.persistent = resource_arenas.text_end;
    level_reset();
    character_resources_begin();
    resource_load((uint32)sequence + 32, 0x116000, NULL, TEXTURE_ARCHIVE_CAPACITY, &tex_file, NULL);
    types = world_object_type_grps[27 + sequence];
    tex = stage_resource_loaders[STAGE_RESOURCE_TEX][27 + sequence];
    world_object_active_types = types;
    resource_loader_call(RESOURCE_LOADER_TEXTURE, tex);
    tex_load_grps(0);
    stage = game_session.stage;
    if (stage == 31 || stage == 33)
    {
        sound_samples_load(171);
        stage = game_session.stage;
    }
    if (stage == 27)
        scene_load_resources0();
    stage = game_session.stage;
    if (stage == 28)
    {
        scene_load_resources1();
        stage = game_session.stage;
    }
    if (stage == 29)
        scene_load_resources2();
    stage = game_session.stage;
    if (stage == 30)
    {
        scene_load_resources3();
        stage = game_session.stage;
    }
    if (stage == 31)
        scene_load_resources4();
    stage = game_session.stage;
    if (stage == 32)
    {
        scene_load_resources5();
        stage = game_session.stage;
    }
    if (stage == 33)
        scene_load_resources6();
    if (game_session.stage == 34)
        scene_load_resources7();
    if (game_session.stage == 35)
        scene_load_resources8();
    disp_prepare_scene();
    if (game_session.stage == 34 && scene_playback.start_skip)
    {
        credits_run();
        game_session.stage = old_stage;
        return 0;
    }
    stage = game_session.stage;
    scene_playback.start_skip = 0;
    if (stage == 34 && scene_run_ending_disp())
    {
        credits_run();
        game_session.stage = old_stage;
        return 0;
    }
    for (i = 8; i > 0; i--)
        actor_animations[i - 1].frame = 0xffffffff;
    stage = game_session.stage;
    if (stage == 27)
        track = 6;
    if (stage == 28)
        track = 7;
    if (stage == 29)
        track = 3;
    if (stage == 30)
        track = 8;
    if (stage == 31)
        track = 9;
    if (stage == 32)
        track = 10;
    if (stage == 33)
        track = 11;
    if (stage == 34 || stage == 35)
        track = 4;
    if (stage != 34)
        loading_clear();
    cd_music_start_scene(track);
    if (game_session.stage == 28)
        for (i = 0; i < 13; i++)
            VSync(2);
    loading_clear();
    disp_fade_from_origin(0);

    for (;;)
    {
        scene_step();
        yaw = camera.angles.yaw_bits;
        pitch = camera.angles.pitch_bits;
        camera_x = camera.pos.x.bits;
        camera_y = camera.pos.y.bits;
        camera_z = camera.pos.z.bits;
        camera.frame_angles.yaw_bits = (uint16)yaw;
        camera.frame_angles.pitch_bits = (uint16)pitch;
        camera.frame_pos.x.bits = camera_x;
        camera.frame_pos.y.bits = camera_y;
        camera.frame_pos.z.bits = camera_z;
        prepare_frame();
        world_objects_draw();
        effects_draw();
        dynamic_objects_draw();
        if (game_session.stage == 27 && (sint32)game_session.tick >= 401)
            draw_characters();
        stage_update();
        stage_update_world();
        effects_update();
        dynamic_objects_update();
        hud_update();
        end_tick = scene_playback.end_tick;
        tick = game_session.tick;
        remaining = end_tick - tick;
        if (remaining < 16)
            disp_fade_from_origin((uint32)remaining << 4);
        for (i = 0; (sint32)i < (sint32)game_session.player_count; i++)
        {
            uint32 pad = (uint16)pad_decode_buttons(pad_game.port[i]);
            if (pad & 0x4008)
            {
                scene_playback.start_skip = (pad & 8) != 0;
                loading_clear();
                sound_voices_stop_all();
                if (game_session.stage == 34)
                    credits_run();
                game_session.stage = old_stage;
                return 0;
            }
        }
        tick = game_session.tick;
        end_tick = scene_playback.end_tick;
        if (tick == end_tick)
            break;
    }
    /* Preserve the two 1453C(0) calls through the host frame adapter */
    for (i = 0; i < 2; i++)
    {
        prepare_frame();
        disp_draw_scene();
        stage_update_world();
        hud_update();
        disp_fade_from_origin(0);
    }
    loading_clear();
    for (i = 0; i < 30; i++)
        VSync(2);
    sound_voices_stop_all();
    if (game_session.stage == 34)
        credits_run();
    game_session.stage = old_stage;
    scene_playback.start_skip = 0;
    return 1;
}

GDB_CALL uint32 scene_step(void)
{
    uint32 frame, stage;
    FUNCTION_MARKER(0x8001559c, "GAME.EXE");
    scene_apply_camera_frame(game_session.tick);
    stage = game_session.stage;
    if (stage == 27)
    {
        scene_update_intro_helicopter(game_session.tick);
        stage = game_session.stage;
    }
    if (stage == 28)
        scene_update_traffic_explosions(game_session.tick);
    stage = game_session.stage;
    if (stage == 29)
    {
        scene_update_hovercraft_fleet(game_session.tick);
        stage = game_session.stage;
    }
    if (stage == 30)
        scene_update_wing(game_session.tick);
    stage = game_session.stage;
    if (stage == 31)
    {
        scene_update_island_fleet(game_session.tick);
        stage = game_session.stage;
    }
    if (stage == 32)
        scene_update_wing_destruction(game_session.tick);
    stage = game_session.stage;
    if (stage == 33)
    {
        scene_update_tower_destruction(game_session.tick);
        stage = game_session.stage;
    }
    if (stage == 34)
        scene_update_prison_ending(game_session.tick);
    frame = game_session.tick + 1;
    game_session.tick = frame;
    return frame;
}

GDB_CALL sint32 scene_run_ending_disp(void)
{
    FUNCTION_MARKER(0x8005a690u, "GAME.EXE");
    uint32 phase = 456960u, packet, pad;
    int frame, i;
    cd_music_start_scene(24);
    for (frame = 0; frame < 210; frame++)
    {
        if (frame < 120)
            phase -= (uint32)(119 - frame) << 6;
        if (frame < 17)
            disp_fade_from_origin(frame * 16);
        if (209 - frame < 17)
            disp_fade_from_origin((209 - frame) * 16);
        disp_begin_frame();
        if (disp_buffers.fade.r + disp_buffers.fade.g + disp_buffers.fade.b)
            disp_draw_fade();
        packet = render_buffers.packet_addr;
        render_buffers.packet_addr = packet + 24u;
        POLY_F4 *poly = psx_addr(packet, sizeof(*poly));
        setlen(poly, 5);
        poly->rgbc.rgbc = 0x28000000;
        poly->p[0].x = 0;
        poly->p[0].y = 0;
        poly->p[1].x = 320;
        poly->p[1].y = 0;
        poly->p[2].x = 0;
        poly->p[2].y = 256;
        poly->p[3].x = 320;
        poly->p[3].y = 256;
        render_ot_insert(render_buffers.ot_addr, packet);
        transform_identity();
        transform_translate(0, 0, ((uint32)((sint32)phase >> 9) + 136u) << 16);
        transform_pitch(32768);
        transform_roll(phase);
        model_draw_static(model_refs[450], 0, 0);
        menu_present();
        for (i = 0; i < (sint32)game_session.player_count; i++)
        {
            pad = pad_decode_buttons(pad_game.port[i]);
            if (pad & 8u)
            {
                loading_clear();
                cd_music_stop();
                disp_fade_from_origin(0);
                return 1;
            }
            if (pad & 0x4000u)
            {
                cd_music_stop();
                return 0;
            }
        }
    }
    cd_music_stop();
    return 0;
}
