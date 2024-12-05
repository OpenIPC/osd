#include "i6_hal.h"

i6_rgn_impl  i6_rgn;
i6_sys_impl  i6_sys;

char _i6_vpe_chn = 0;
char _i6_vpe_dev = 0;

void i6_hal_deinit(void)
{
    i6_rgn_unload(&i6_rgn);
    i6_sys_unload(&i6_sys);
}

int i6_hal_init(void)
{
    int ret;

    if (ret = i6_sys_load(&i6_sys))
        return ret;
    if (ret = i6_rgn_load(&i6_rgn))
        return ret;

    return EXIT_SUCCESS;
}

int i6_region_create(char handle, hal_rect rect, short opacity)
{
    int ret;

    i6_sys_bind dest = { .module = 0, .device = _i6_vpe_dev,
        .channel = _i6_vpe_chn };
    i6_rgn_cnf region, regionCurr;
    i6_rgn_chn attrib, attribCurr;

    region.type = I6_RGN_TYPE_OSD;
    region.pixFmt = I6_RGN_PIXFMT_ARGB1555;
    region.size.width = rect.width;
    region.size.height = rect.height;

    if (i6_rgn.fnGetRegionConfig(handle, &regionCurr)) {
        HAL_INFO("i6_rgn", "Creating region %d...\n", handle);
        if (ret = i6_rgn.fnCreateRegion(handle, &region))
            return ret;
    } else if (regionCurr.type != region.type ||
        regionCurr.size.height != region.size.height || 
        regionCurr.size.width != region.size.width) {
        HAL_INFO("i6_rgn", "Parameters are different, recreating "
            "region %d...\n", handle);
        for (dest.port = 0; dest.port < 2; dest.port++)
            i6_rgn.fnDetachChannel(handle, &dest);
        i6_rgn.fnDestroyRegion(handle);
        if (ret = i6_rgn.fnCreateRegion(handle, &region))
            return ret;
    }

    if (i6_rgn.fnGetChannelConfig(handle, &dest, &attribCurr))
        HAL_INFO("i6_rgn", "Attaching region %d...\n", handle);
    else if (attribCurr.point.x != rect.x || attribCurr.point.x != rect.y ||
        attribCurr.osd.bgFgAlpha[1] != opacity) {
        HAL_INFO("i6_rgn", "Parameters are different, reattaching "
            "region %d...\n", handle);
        for (dest.port = 0; dest.port < 2; dest.port++)
            i6_rgn.fnDetachChannel(handle, &dest);
    }

    memset(&attrib, 0, sizeof(attrib));
    attrib.show = 1;
    attrib.point.x = rect.x;
    attrib.point.y = rect.y;
    attrib.osd.layer = 0;
    attrib.osd.constAlphaOn = 0;
    attrib.osd.bgFgAlpha[0] = 0;
    attrib.osd.bgFgAlpha[1] = opacity;

    for (dest.port = 0; dest.port < 2; dest.port++)
        i6_rgn.fnAttachChannel(handle, &dest, &attrib);

    return EXIT_SUCCESS;
}

void i6_region_deinit(void)
{
    i6_rgn.fnDeinit();
}

void i6_region_destroy(char handle)
{
    i6_sys_bind dest = { .module = 0, .device = _i6_vpe_dev,
        .channel = _i6_vpe_chn };
    
    for (dest.port = 0; dest.port < 2; dest.port++)
        i6_rgn.fnDetachChannel(handle, &dest);
    i6_rgn.fnDestroyRegion(handle);
}

void i6_region_init(void)
{
    i6_rgn_pal palette = {{{0, 0, 0, 0}}};
    i6_rgn.fnInit(&palette);
}

int i6_region_setbitmap(int handle, hal_bitmap *bitmap)
{
    i6_rgn_bmp nativeBmp = { .data = bitmap->data, .pixFmt = I6_RGN_PIXFMT_ARGB1555,
        .size.height = bitmap->dim.height, .size.width = bitmap->dim.width };

    return i6_rgn.fnSetBitmap(handle, &nativeBmp);
}