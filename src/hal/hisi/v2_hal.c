#include "v2_hal.h"

v2_rgn_impl     v2_rgn;
v2_sys_impl     v2_sys;

char _v2_venc_dev = 0;

void v2_hal_deinit(void)
{
    v2_rgn_unload(&v2_rgn);
    v2_sys_unload(&v2_sys);
}

int v2_hal_init(void)
{
    int ret;

    if (ret = v2_sys_load(&v2_sys))
        return ret;
    if (ret = v2_rgn_load(&v2_rgn))
        return ret;

    return EXIT_SUCCESS;
}

int v2_region_create(char handle, hal_rect rect, short opacity)
{
    int ret;

    v2_sys_bind dest = { .module = V2_SYS_MOD_VENC, .device = _v2_venc_dev };
    v2_rgn_cnf region, regionCurr;
    v2_rgn_chn attrib, attribCurr;

    rect.height += rect.height & 1;
    rect.width += rect.width & 1;

    memset(&region, 0, sizeof(region));
    region.type = V2_RGN_TYPE_OVERLAY;
    region.overlay.pixFmt = V2_PIXFMT_ARGB1555;
    region.overlay.size.width = rect.width;
    region.overlay.size.height = rect.height;
    region.overlay.canvas = handle + 1;

    if (v2_rgn.fnGetRegionConfig(handle, &regionCurr)) {
        HAL_INFO("v2_rgn", "Creating region %d...\n", handle);
        if (ret = v2_rgn.fnCreateRegion(handle, &region))
            return ret;
    } else if (regionCurr.type != region.type ||
        regionCurr.overlay.size.height != region.overlay.size.height || 
        regionCurr.overlay.size.width != region.overlay.size.width) {
        HAL_INFO("v2_rgn", "Parameters are different, recreating "
            "region %d...\n", handle);
        for (dest.channel = 0; dest.channel < 2; dest.channel++)
            v2_rgn.fnDetachChannel(handle, &dest);
        v2_rgn.fnDestroyRegion(handle);
        if (ret = v2_rgn.fnCreateRegion(handle, &region))
            return ret;
    }

    if (v2_rgn.fnGetChannelConfig(handle, &dest, &attribCurr))
        HAL_INFO("v2_rgn", "Attaching region %d...\n", handle);
    else if (attribCurr.overlay.point.x != rect.x || attribCurr.overlay.point.x != rect.y) {
        HAL_INFO("v2_rgn", "Position has changed, reattaching "
            "region %d...\n", handle);
        for (dest.channel = 0; dest.channel < 2; dest.channel++)
            v2_rgn.fnDetachChannel(handle, &dest);
    }

    memset(&attrib, 0, sizeof(attrib));
    attrib.show = 1;
    attrib.type = V2_RGN_TYPE_OVERLAY;
    attrib.overlay.bgAlpha = 0;
    attrib.overlay.fgAlpha = opacity >> 1;
    attrib.overlay.point.x = rect.x;
    attrib.overlay.point.y = rect.y;
    attrib.overlay.layer = 7;

    for (dest.channel = 0; dest.channel < 2; dest.channel++)
        v2_rgn.fnAttachChannel(handle, &dest, &attrib);

    return EXIT_SUCCESS;
}

void v2_region_destroy(char handle)
{
    v2_sys_bind dest = { .module = V2_SYS_MOD_VENC, .device = _v2_venc_dev };
    
    for (dest.channel = 0; dest.channel < 2; dest.channel++)
        v2_rgn.fnDetachChannel(handle, &dest);
    v2_rgn.fnDestroyRegion(handle);
}

int v2_region_setbitmap(int handle, hal_bitmap *bitmap)
{
    v2_rgn_bmp nativeBmp = { .data = bitmap->data, .pixFmt = V2_PIXFMT_ARGB1555,
        .size.height = bitmap->dim.height, .size.width = bitmap->dim.width };

    return v2_rgn.fnSetBitmap(handle, &nativeBmp);
}

float v2_system_readtemp(void)
{
    int val, prep = 0x60fa0000;
    float result = 0.0 / 0.0;
    if (hal_registry(0x20270110, &val, OP_READ) && prep != val)
        hal_registry(0x20270110, &prep, OP_WRITE);
    
    if (!hal_registry(0x20270114, &val, OP_READ))
        return result;
    result = val & ((1 << 8) - 1);
    return ((result * 180) / 256) - 40;
}