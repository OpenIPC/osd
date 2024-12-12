#include "m6_hal.h"

m6_rgn_impl  m6_rgn;
m6_sys_impl  m6_sys;

char _m6_venc_dev[2] = { 0, 8 };
char _m6_venc_port = 0;

void m6_hal_deinit(void)
{
    m6_rgn_unload(&m6_rgn);
    m6_sys_unload(&m6_sys);
}

int m6_hal_init(void)
{
    int ret;

    if (ret = m6_sys_load(&m6_sys))
        return ret;
    if (ret = m6_rgn_load(&m6_rgn))
        return ret;

    return EXIT_SUCCESS;
}

int m6_region_create(char handle, hal_rect rect, short opacity)
{
    int ret;

    m6_sys_bind dest = { .module = M6_SYS_MOD_VENC, .port = _m6_venc_port };
    m6_rgn_cnf region, regionCurr;
    m6_rgn_chn attrib, attribCurr;

    region.type = M6_RGN_TYPE_OSD;
    region.pixFmt = M6_RGN_PIXFMT_ARGB1555;
    region.size.width = rect.width;
    region.size.height = rect.height;

    if (m6_rgn.fnGetRegionConfig(0, handle, &regionCurr)) {
        HAL_INFO("m6_rgn", "Creating region %d...\n", handle);
        if (ret = m6_rgn.fnCreateRegion(0, handle, &region))
            return ret;
    } else if (regionCurr.type != region.type ||
        regionCurr.size.height != region.size.height || 
        regionCurr.size.width != region.size.width) {
        HAL_INFO("m6_rgn", "Parameters are different, recreating "
            "region %d...\n", handle);
        for (dest.channel = 0; dest.channel < 2; dest.channel++) {
            dest.device = _m6_venc_dev[0];
            m6_rgn.fnDetachChannel(0, handle, &dest);
            dest.device = _m6_venc_dev[1];
            m6_rgn.fnDetachChannel(0, handle, &dest);
        }
        m6_rgn.fnDestroyRegion(0, handle);
        if (ret = m6_rgn.fnCreateRegion(0, handle, &region))
            return ret;
    }

    if (m6_rgn.fnGetChannelConfig(0, handle, &dest, &attribCurr))
        HAL_INFO("m6_rgn", "Attaching region %d...\n", handle);
    else if (attribCurr.point.x != rect.x || attribCurr.point.x != rect.y ||
        attribCurr.osd.bgFgAlpha[1] != opacity) {
        HAL_INFO("m6_rgn", "Parameters are different, reattaching "
            "region %d...\n", handle);
        for (dest.channel = 0; dest.channel < 2; dest.channel++) {
            dest.device = _m6_venc_dev[0];
            m6_rgn.fnDetachChannel(0, handle, &dest);
            dest.device = _m6_venc_dev[1];
            m6_rgn.fnDetachChannel(0, handle, &dest);
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
        dest.device = _m6_venc_dev[0];
        m6_rgn.fnAttachChannel(0, handle, &dest, &attrib);
        dest.device = _m6_venc_dev[1];
        m6_rgn.fnAttachChannel(0, handle, &dest, &attrib);
    }

    return EXIT_SUCCESS;
}

void m6_region_deinit(void)
{
    m6_rgn.fnDeinit(0);
}

void m6_region_destroy(char handle)
{
    m6_sys_bind dest = { .module = M6_SYS_MOD_VENC, .port = _m6_venc_port };
    
    for (dest.channel = 0; dest.channel < 2; dest.channel++) {
        dest.device = _m6_venc_dev[0];
        m6_rgn.fnDetachChannel(0, handle, &dest);
        dest.device = _m6_venc_dev[1];
        m6_rgn.fnDetachChannel(0, handle, &dest);
    }
    m6_rgn.fnDestroyRegion(0, handle);
}

void m6_region_init(void)
{
    m6_rgn_pal palette = {{{0, 0, 0, 0}}};
    m6_rgn.fnInit(0, &palette);
}

int m6_region_setbitmap(int handle, hal_bitmap *bitmap)
{
    m6_rgn_bmp nativeBmp = { .data = bitmap->data, .pixFmt = M6_RGN_PIXFMT_ARGB1555,
        .size.height = bitmap->dim.height, .size.width = bitmap->dim.width };

    return m6_rgn.fnSetBitmap(0, handle, &nativeBmp);
}