#include <stddef.h>

#include "game.h"
#include "sound_device.h"

SOUND_SAMPLES sound_samples;

sint32 sound_samples_load(sint32 archive)
{
    FUNCTION_MARKER(0x80056ccc, "GAME.EXE");

    typedef struct SOUND_FILE
    {
        const char *name;
        uint32 id;
    } SOUND_FILE;

    static const SOUND_FILE files[] = {{"L_HELI1.VAG", 0},    {"L_HELI2.VAG", 1},    {"L_747PLN.VAG", 6},   {"L_COPCAR.VAG", 2},   {"SELECT.VAG", 4},    {"CHOOSE.VAG", 3},     {"BAD.VAG", 5},       {"BS_PP1.VAG", 7},     {"BS_KP1.VAG", 8},     {"BS_HIT1.VAG", 9},    {"BS_HIT2.VAG", 10},   {"BS_HIT3.VAG", 11},   {"BS_DIE1.VAG", 12},  {"BS_DIE2.VAG", 13}, {"VAMDIE.VAG", 14},  {"HAWK_FOF.VAG", 16}, {"STICK_SW.VAG", 15},  {"EXPLO1.VAG", 19},   {"SMEXPLO.VAG", 20},  {"MLAUNCH.VAG", 21},  {"GEN_SWSH.VAG", 22},  {"MP_FH.VAG", 23},     {"PISTOL2.VAG", 24},  {"HHCANON.VAG", 25},  {"TRIGGER.VAG", 26},  {"RINGPL.VAG", 122},  {"DRINK.VAG", 123},   {"SHOTGUN.VAG", 27},   {"SHTGNCK.VAG", 28},  {"CHING.VAG", 29},     {"CRATEHIT.VAG", 30}, {"CRATESLD.VAG", 31}, {"CAR_DAM1.VAG", 113}, {"CAR_DAM2.VAG", 114}, {"CAR_DAM3.VAG", 115}, {"BSCAR1DO.VAG", 108}, {"BSCAR1DC.VAG", 109}, {"BSCAR1.VAG", 110},   {"L_CARAL.VAG", 107},  {"CAR_LAND.VAG", 112}, {"MET_SHUT.VAG", 116},
                                       {"HIT_MSHT.VAG", 117}, {"L_OUTSD.VAG", 118},  {"COKECN1.VAG", 119},  {"COKECN2.VAG", 120},  {"COKECN3.VAG", 121}, {"PAPER1.VAG", 128},   {"PAPER2.VAG", 129},  {"L_WIND.VAG", 130},   {"WINDOW1.VAG", 127},  {"L_SUBWAY.VAG", 145}, {"CS_SKID1.VAG", 146}, {"L_SIREN.VAG", 147},  {"HAWK_P1.VAG", 33},  {"HAWK_P2.VAG", 34}, {"HAWK_P3.VAG", 35}, {"HAWK_K1.VAG", 36},  {"HAWK_K2.VAG", 37},   {"HAWK_JMP.VAG", 38}, {"HAWK_LND.VAG", 39}, {"HAWK_HIT.VAG", 40}, {"HAWK_DIE.VAG", 42},  {"HAWK_HED.VAG", 43},  {"HAWK_BP1.VAG", 44}, {"HAWK_K2B.VAG", 45}, {"HAWK_360.VAG", 46}, {"HAWK_GRB.VAG", 47}, {"HAWK_SW1.VAG", 48}, {"HAWK_SW2.VAG", 49},  {"HAWK_FOF.VAG", 50}, {"HAWK_TRW.VAG", 51},  {"HAWK_F1.VAG", 52},  {"HAWK_F2.VAG", 53},  {"HAWK_F3.VAG", 54},   {"HAWK_F4.VAG", 55},   {"HAWK_SKD.VAG", 56},  {"MACE_P1.VAG", 57},   {"MACE_P2.VAG", 58},   {"MACE_P3.VAG", 59},   {"MACE_K1.VAG", 60},   {"MACE_K2.VAG", 61},   {"MACE_JMP.VAG", 62},
                                       {"MACE_LND.VAG", 63},  {"MACE_H1.VAG", 64},   {"MACE_H2.VAG", 65},   {"MACE_DIE.VAG", 69},  {"MACE_GRB.VAG", 67}, {"MACE_AS.VAG", 70},   {"MACE_LS.VAG", 71},  {"MACE_FOF.VAG", 68},  {"MACE_TRW.VAG", 66},  {"MACE_F1.VAG", 72},   {"MACE_F2.VAG", 73},   {"MACE_F3.VAG", 74},   {"MACE_F4.VAG", 75},  {"SMAS_P1.VAG", 76}, {"SMAS_P2.VAG", 77}, {"SMAS_P3.VAG", 78},  {"SMAS_JMP.VAG", 79},  {"SMAS_LND.VAG", 80}, {"SMAS_HIT.VAG", 84}, {"SMAS_DIE.VAG", 82}, {"SMAS_GRB.VAG", 83},  {"SMAS_FOF.VAG", 86},  {"SMAS_TRW.VAG", 81}, {"SMAS_F1.VAG", 87},  {"SMAS_F2.VAG", 88},  {"SMAS_F3.VAG", 89},  {"SMAS_F4.VAG", 90},  {"SMAS_SKD.VAG", 92},  {"L_SMARL.VAG", 91},  {"ALAN_P1.VAG", 93},   {"ALAN_P2.VAG", 94},  {"ALAN_P3.VAG", 95},  {"ALAN_K1.VAG", 96},   {"ALAN_K2.VAG", 97},   {"ALAN_JMP.VAG", 98},  {"ALAN_H1.VAG", 99},   {"ALAN_H2.VAG", 100},  {"ALAN_DIE.VAG", 104}, {"ALAN_GRB.VAG", 103}, {"ALAN_FOF.VAG", 101}, {"ALAN_TRW.VAG", 102},
                                       {"EXWALK.VAG", 17},    {"LAZPISTL.VAG", 18},  {"L_CBRKSL.VAG", 131}, {"L_BSCAR.VAG", 132},  {"CR_SUSP.VAG", 133}, {"BUSSTOP.VAG", 134},  {"HITFRH.VAG", 135},  {"RICH_1.VAG", 136},   {"RICH_2.VAG", 137},   {"RICH_3.VAG", 138},   {"RICH_4.VAG", 139},   {"L_SHALAM.VAG", 140}, {"L_CIT2.VAG", 141},  {"TMETAL.VAG", 32},  {"EDMOVE.VAG", 124}, {"EDCLSE.VAG", 125},  {"L_BLMOTR.VAG", 126}, {"BIKEREV.VAG", 142}, {"L_BIKE.VAG", 143},  {"L_BIKE2.VAG", 144}, {"L_LOWEN.VAG", 152},  {"BLOOD.VAG", 153},    {"L_HOSE.VAG", 154},  {"L_FOUT.VAG", 155},  {"L_FIRE.VAG", 156},  {"L_FIRE2.VAG", 157}, {"L_FENGN.VAG", 158}, {"TD_THIT.VAG", 159},  {"TD_DIE.VAG", 160},  {"TD_HDLK.VAG", 161},  {"TD_HLP.VAG", 162},  {"TD_LAFF.VAG", 163}, {"TD_THRW.VAG", 164},  {"TRKDLND.VAG", 165},  {"L_EXENG.VAG", 166},  {"EXHITFLR.VAG", 167}, {"EXHITMTL.VAG", 168}, {"EXSWSH.VAG", 169},   {"L_EXSPN.VAG", 170},  {"EXCLP.VAG", 171},    {"EXUPCT.VAG", 172},
                                       {"EXLND.VAG", 173},    {"EXDIEE.VAG", 174},   {"EXLAUGH.VAG", 175},  {"EXGRB.VAG", 176},    {"EXPNCH.VAG", 177},  {"L_EXELEC.VAG", 184}, {"L_EXO1.VAG", 178},  {"L_CHAIN1.VAG", 180}, {"L_CHAIN2.VAG", 181}, {"EXLCHN.VAG", 179},   {"EXOHIT.VAG", 182},   {"EXOSWSH.VAG", 183},  {"EXZZAPP.VAG", 185}, {"L_JSPN.VAG", 186}, {"JETBST.VAG", 187}, {"L_JETLP.VAG", 188}, {"JETJMP.VAG", 189},   {"JETHIT1.VAG", 190}, {"JETHIT2.VAG", 191}, {"JETDIE.VAG", 192},  {"JETDIEE.VAG", 193},  {"JETLAF.VAG", 194},   {"JETGRB.VAG", 195},  {"JETSKD.VAG", 196},  {"JETLND.VAG", 197},  {"JETKICK.VAG", 198}, {"JETPASS.VAG", 199}, {"L_ALARM1.VAG", 200}, {"LAZMAC.VAG", 201},  {"LZMACOFF.VAG", 202}, {"LSWDON.VAG", 203},  {"LSWDOFF.VAG", 204}, {"LSWDSWS.VAG", 205},  {"LSWDHIT.VAG", 206},  {"Z_LAF.VAG", 207},    {"Z_BHIT.VAG", 208},   {"Z_LND.VAG", 209},    {"Z_PFLR.VAG", 210},   {"FORKLST.VAG", 211},  {"L_FORK.VAG", 212},   {"FORKLSTP.VAG", 213},
                                       {"L_AIRSHP.VAG", 214}, {"L_SUBWAY.VAG", 215}, {"L_HOVER.VAG", 235},  {"L_HOVENG.VAG", 236}, {"L_HHCAN.VAG", 216}, {"HHACEND.VAG", 217},  {"TRWWWEP.VAG", 218}, {"TRWMWEP.VAG", 219},  {"TRWGWEP.VAG", 220},  {"OILDRUM.VAG", 221},  {"BOTTLE.VAG", 222},   {"VAMDIE.VAG", 223},   {"VAMH1.VAG", 224},   {"VAMH2.VAG", 225},  {"VAMSL.VAG", 226},  {"VAMSWSH.VAG", 227}, {"PNUHISS.VAG", 150},  {"UGHISS.VAG", 241},  {"UGSQK.VAG", 242},   {"CHAIR.VAG", 232},   {"LIFTBONG.VAG", 234}, {"BIKESKID.VAG", 230}, {"FOGHORN.VAG", 233}, {"L_HELIC.VAG", 243}, {"L_MERC.VAG", 231},  {"MONKEY1.VAG", 228}, {"MONKEY2.VAG", 229}, {"TURNS.VAG", 151},    {"BIGD1.VAG", 244},   {"BIGD2.VAG", 245},    {"L_HOVER.VAG", 235}, {"L_COMPY.VAG", 239}, {"L_COMPY2.VAG", 240}, {"L_CRYO1.VAG", 237},  {"L_CRYO2.VAG", 238},  {NULL, 0xffffffff}};
    const SOUND_FILE *table;
    const char *name;
    uint32 id, size, total = 0, offset, base, available, i;
    RESOURCE_VIEW src;
    sint32 first_byte;
    for (i = 0; i < SOUND_SAMPLE_COUNT; i++)
        sound_samples.sample[SOUND_SAMPLE_COUNT - 1 - i].offset = 0xffffffff;
    sound_output_init();
    /* Physical CD reset remains an agreed native no-op boundary */
    resource_load(archive, 0x116000, NULL, SOUND_ARCHIVE_CAPACITY, &sound_samples.file, NULL);
    for (table = files; table->id != 0xffffffff; table++)
    {
        name = table->name;
        if (!resource_exists(name))
            continue;
        name = table->name;
        resource_find(name);
        size = (resource_io.result_size + 15) & ~15;
        total = total - 48 + size;
        resource_io.result_size = size;
    }
    sound_samples.spu_base = sound_heap_allocate(total + 1024);
    sound_transfer_wait();
    offset = 0;
    table = files;
    id = table->id;
    for (; id != 0xffffffff; table++, id = table->id)
    {
        name = table->name;
        if (!resource_exists(name))
            continue;
        name = table->name;
        src = resource_find(name);
        size = resource_io.result_size;
        base = sound_samples.spu_base;
        sound_samples.sample[id].offset = offset;
        size = (size + 15) & ~15;
        resource_io.result_size = size;
        /* TODO: Investigate a sample destination rejected by the SDK */
        {
            sint32 checked_result = SpuSetTransferStartAddr(base + offset);
#if defined(_DEBUG)
            sint32 transfer_failed = checked_result ? spu_transfer_failed() : 0;
            XASSERT(checked_result && !transfer_failed);
#endif
        }
        size = resource_io.result_size;
        available = src.storage_len - (uint32)(src.data - src.storage);
        /* TODO: Investigate an original VAG upload starting beyond its archive */
        XASSERT(available >= 48);
        sound_transfer_write(src.data + 48, size - 48, available - 48);
        sound_transfer_wait();
        name = table->name;
        size = resource_io.result_size;
        first_byte = name[0];
        offset = offset - 48 + size;
        sound_samples.sample[id].mode = (first_byte == 'L' && name[1] == '_') ? 2 : 1;
    }
    return -1;
}
