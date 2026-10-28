#include "game.h"
#include "psx_gpu.h"

const LOADING loading_defaults = {640, -1, 0, 0};
LOADING loading;

uint32 loading_clear(void)
{
    FUNCTION_MARKER(0x8005ed78, "GAME.EXE");
    loading.bar_x = 640;
    loading.total = -1;
    loading.active = 0;
    /* Host uploads are synchronous; DrawSync/VSync hardware waits are omitted */
    gpu_clear_menu_surfaces();
    disp_buffers.flash_ticks = 0;
    disp_buffers.shake_ticks = 0;
    disp_buffers.fade_origin.b = (sint16)(0);
    disp_buffers.fade_origin.g = (sint16)(0);
    disp_buffers.fade_origin.r = (sint16)(0);
    disp_set_fade(-256, -256, -256);
    render_ot_init(render_buffers.ot_addr);
    render_buffers_select();
    return render_ot_init(render_buffers.ot_addr);
}

uint32 loading_begin(uint32 steps)
{
    FUNCTION_MARKER(0x8005eccc, "GAME.EXE");
    if (game_services.audit_input_event)
        game_services.audit_input_event(3);
    loading_clear();
    loading.active = 1;
    resource_arenas.persistent = resource_arenas.text_end;
    loading.bar_x = 640;
    loading.total = -1;
    cd_music_stop();
    resource_load(0, 0x116000, NULL, TEXTURE_ARCHIVE_CAPACITY, &tex_file, NULL);
    tex_load_grps(0);
    tex_upload_pals();
    loading_draw();
    loading_draw();
    loading_draw();
    disp_set_fade(-256, -256, -256);
    loading.bar_x = 77;
    loading.total = (sint16)(uint16)(steps - 1);
    loading.count = 0;
    return 77;
}

uint32 loading_draw(void)
{
    FUNCTION_MARKER(0x80063c54, "GAME.EXE");
    const TEXTURE_DESCRIPTOR *desc;
    uint32 i, p;
    SPRT *spr;
    disp_begin_frame();
    uint32 desc_idx = (uint32)tex_map[(uint32)tex_grps[TEXTURE_GROUP_TITLE_ALT].first_tex];
    if (desc_idx > TEXTURE_DESCRIPTOR_CAPACITY - 3)
        diagnostic_stop(__FUNCTION__, __FILE__, __LINE__);
    desc = tex_desc.item + desc_idx;
    for (i = 0; i < 3; i++, desc++)
    {
        p = render_buffers.packet_addr;
        render_buffers.packet_addr = p + 20;
        spr = (SPRT *)psx_addr(p, sizeof(*spr));
        setSprt(spr);
        setUV0(spr, desc->uv0.u, desc->uv0.v);
        setWH(spr, i == 2 ? 64 : 128, 255);
        setXY0(spr, 128 * i, 0);
        setShadeTex(spr, 1);
        render_ot_insert(render_buffers.ot_addr + 4, p);
        render_insert_tex_page((sint16)desc->tpage, 1);
    }
    return menu_present();
}
