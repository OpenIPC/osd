#include "i6f_hal.h"

i6f_rgn_impl  i6f_rgn;
i6f_sys_impl  i6f_sys;

char _i6f_venc_dev[2] = { 0, 8 };
char _i6f_venc_port = 0;

void i6f_hal_deinit(void)
{
    i6f_rgn_unload(&i6f_rgn);
    i6f_sys_unload(&i6f_sys);
}

int i6f_hal_init(void)
{
    int ret;

    if (ret = i6f_sys_load(&i6f_sys))
        return ret;
    if (ret = i6f_rgn_load(&i6f_rgn))
        return ret;

    return EXIT_SUCCESS;
}

int i6f_region_create(char handle, hal_rect rect, short opacity)
{
    int ret;

    i6f_sys_bind dest = { .module = I6F_SYS_MOD_VENC, .port = _i6f_venc_port };
    i6f_rgn_cnf region, regionCurr;
    i6f_rgn_chn attrib, attribCurr;

    region.type = I6F_RGN_TYPE_OSD;
    region.pixFmt = I6F_RGN_PIXFMT_ARGB1555;
    region.size.width = rect.width;
    region.size.height = rect.height;

    if (i6f_rgn.fnGetRegionConfig(0, handle, &regionCurr)) {
        HAL_INFO("i6f_rgn", "Creating region %d...\n", handle);
        if (ret = i6f_rgn.fnCreateRegion(0, handle, &region))
            return ret;
    } else if (regionCurr.type != region.type ||
        regionCurr.size.height != region.size.height || 
        regionCurr.size.width != region.size.width) {
        HAL_INFO("i6f_rgn", "Parameters are different, recreating "
            "region %d...\n", handle);
        for (dest.channel = 0; dest.channel < 2; dest.channel++) {
            dest.device = _i6f_venc_dev[0];
            i6f_rgn.fnDetachChannel(0, handle, &dest);
            dest.device = _i6f_venc_dev[1];
            i6f_rgn.fnDetachChannel(0, handle, &dest);
        }
        i6f_rgn.fnDestroyRegion(0, handle);
        if (ret = i6f_rgn.fnCreateRegion(0, handle, &region))
            return ret;
    }

    if (i6f_rgn.fnGetChannelConfig(0, handle, &dest, &attribCurr))
        HAL_INFO("i6f_rgn", "Attaching region %d...\n", handle);
    else if (attribCurr.point.x != rect.x || attribCurr.point.x != rect.y ||
        attribCurr.osd.bgFgAlpha[1] != opacity) {
        HAL_INFO("i6f_rgn", "Parameters are different, reattaching "
            "region %d...\n", handle);
        for (dest.channel = 0; dest.channel < 2; dest.channel++) {
            dest.device = _i6f_venc_dev[0];
            i6f_rgn.fnDetachChannel(0, handle, &dest);
            dest.device = _i6f_venc_dev[1];
            i6f_rgn.fnDetachChannel(0, handle, &dest);
        }
    }

    memset(&attrib, 0, sizeof(attrib));
    attrib.show = 1;
    attrib.point.x = rect.x;
    attrib.point.y = rect.y;
    attrib.osd.layer = 0;
    attrib.osd.constAlphaOn = 0;
    attrib.osd.bgFgAlpha[0] = 0;
    attrib.osd.bgFgAlpha[1] = opacity;

    for (dest.channel = 0; dest.channel < 2; dest.channel++) {
        dest.device = _i6f_venc_dev[0];
        i6f_rgn.fnAttachChannel(0, handle, &dest, &attrib);
        dest.device = _i6f_venc_dev[1];
        i6f_rgn.fnAttachChannel(0, handle, &dest, &attrib);
    }

    return EXIT_SUCCESS;
}

void i6f_region_deinit(void)
{
    i6f_rgn.fnDeinit(0);
}

void i6f_region_destroy(char handle)
{
    i6f_sys_bind dest = { .module = I6F_SYS_MOD_VENC, .port = _i6f_venc_port };
    
    for (dest.channel = 0; dest.channel < 2; dest.channel++) {
        dest.device = _i6f_venc_dev[0];
        i6f_rgn.fnDetachChannel(0, handle, &dest);
        dest.device = _i6f_venc_dev[1];
        i6f_rgn.fnDetachChannel(0, handle, &dest);
    }
    i6f_rgn.fnDestroyRegion(0, handle);
}

void i6f_region_init(void)
{
    i6f_rgn_pal palette = {{{0, 0, 0, 0}}};
    i6f_rgn.fnInit(0, &palette);
}

int i6f_region_setbitmap(int handle, hal_bitmap *bitmap)
{
    i6f_rgn_bmp nativeBmp = { .data = bitmap->data, .pixFmt = I6F_RGN_PIXFMT_ARGB1555,
        .size.height = bitmap->dim.height, .size.width = bitmap->dim.width };

    return i6f_rgn.fnSetBitmap(0, handle, &nativeBmp);
}