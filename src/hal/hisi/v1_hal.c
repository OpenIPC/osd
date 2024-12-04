#include "v1_hal.h"

v1_rgn_impl     v1_rgn;
v1_sys_impl     v1_sys;

char _v1_venc_dev = 0;
char _v1_vpss_grp = 0;

void v1_hal_deinit(void)
{
    v1_rgn_unload(&v1_rgn);
    v1_sys_unload(&v1_sys);
}

int v1_hal_init(void)
{
    int ret;

    if (ret = v1_sys_load(&v1_sys))
        return ret;
    if (ret = v1_rgn_load(&v1_rgn))
        return ret;

    return EXIT_SUCCESS;
}

int v1_region_create(char handle, hal_rect rect, short opacity)
{
    int ret;

    v1_sys_bind channel = { .module = V1_SYS_MOD_VPSS,
        .device = _v1_vpss_grp, .channel = 0 };
    v1_rgn_cnf region, regionCurr;
    v1_rgn_chn attrib, attribCurr;

    memset(&region, 0, sizeof(region));
    region.type = V1_RGN_TYPE_OVERLAY;
    region.overlay.pixFmt = V1_PIXFMT_ARGB1555;
    region.overlay.size.width = rect.width;
    region.overlay.size.height = rect.height;
    region.overlay.canvas = handle + 1;

    if (v1_rgn.fnGetRegionConfig(handle, &regionCurr)) {
        HAL_INFO("v1_rgn", "Creating region %d...\n", handle);
        if (ret = v1_rgn.fnCreateRegion(handle, &region))
            return ret;
    } else if (regionCurr.overlay.size.height != region.overlay.size.height || 
        regionCurr.overlay.size.width != region.overlay.size.width) {
        HAL_INFO("v1_rgn", "Parameters are different, recreating "
            "region %d...\n", handle);
        v1_rgn.fnDetachChannel(handle, &channel);
        v1_rgn.fnDestroyRegion(handle);
        if (ret = v1_rgn.fnCreateRegion(handle, &region))
            return ret;
    }

    if (v1_rgn.fnGetChannelConfig(handle, &channel, &attribCurr))
        HAL_INFO("v1_rgn", "Attaching region %d...\n", handle);
    else if (attribCurr.overlay.point.x != rect.x || attribCurr.overlay.point.x != rect.y) {
        HAL_INFO("v1_rgn", "Position has changed, reattaching "
            "region %d...\n", handle);
        v1_rgn.fnDetachChannel(handle, &channel);
    }

    memset(&attrib, 0, sizeof(attrib));
    attrib.show = 1;
    attrib.type = V1_RGN_TYPE_OVERLAY;
    attrib.overlay.bgAlpha = 0;
    attrib.overlay.fgAlpha = opacity >> 1;
    attrib.overlay.point.x = rect.x;
    attrib.overlay.point.y = rect.y;
    attrib.overlay.layer = 7;

    v1_rgn.fnAttachChannel(handle, &channel, &attrib);

    return EXIT_SUCCESS;
}

void v1_region_destroy(char handle)
{
    v1_sys_bind channel = { .module = V1_SYS_MOD_VPSS,
        .device = _v1_vpss_grp, .channel = 0 };
    
    v1_rgn.fnDetachChannel(handle, &channel);
    v1_rgn.fnDestroyRegion(handle);
}

int v1_region_setbitmap(int handle, hal_bitmap *bitmap)
{
    v1_rgn_bmp nativeBmp = { .data = bitmap->data, .pixFmt = V1_PIXFMT_ARGB1555,
        .size.height = bitmap->dim.height, .size.width = bitmap->dim.width };

    return v1_rgn.fnSetBitmap(handle, &nativeBmp);
}