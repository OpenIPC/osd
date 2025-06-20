#if defined(__arm__) && !defined(__ARM_PCS_VFP)

#include "v4_hal.h"

v4_rgn_impl     v4_rgn;
v4_sys_impl     v4_sys;

char _v4_venc_dev = 0;

void v4_hal_deinit(void)
{
    v4_rgn_unload(&v4_rgn);
    v4_sys_unload(&v4_sys);
}

int v4_hal_init(void)
{
    int ret;

    if (ret = v4_sys_load(&v4_sys))
        return ret;
    if (ret = v4_rgn_load(&v4_rgn))
        return ret;

    return EXIT_SUCCESS;
}

int v4_region_create(char handle, hal_rect rect, short opacity)
{
    int ret;

    v4_sys_bind dest = { .module = V4_SYS_MOD_VENC, .device = _v4_venc_dev };
    v4_rgn_cnf region, regionCurr;
    v4_rgn_chn attrib, attribCurr;

    rect.height += rect.height & 1;
    rect.width += rect.width & 1;

    memset(&region, 0, sizeof(region));
    region.type = V4_RGN_TYPE_OVERLAY;
    region.overlay.pixFmt = V4_PIXFMT_ARGB1555;
    region.overlay.size.width = rect.width;
    region.overlay.size.height = rect.height;
    region.overlay.canvas = handle + 1;

    if (v4_rgn.fnGetRegionConfig(handle, &regionCurr)) {
        HAL_INFO("v4_rgn", "Creating region %d...\n", handle);
        if (ret = v4_rgn.fnCreateRegion(handle, &region))
            return ret;
    } else if (regionCurr.type != region.type ||
        regionCurr.overlay.size.height != region.overlay.size.height || 
        regionCurr.overlay.size.width != region.overlay.size.width) {
        HAL_INFO("v4_rgn", "Parameters are different, recreating "
            "region %d...\n", handle);
        for (dest.channel = 0; dest.channel < 2; dest.channel++)
            v4_rgn.fnDetachChannel(handle, &dest);
        v4_rgn.fnDestroyRegion(handle);
        if (ret = v4_rgn.fnCreateRegion(handle, &region))
            return ret;
    }

    if (v4_rgn.fnGetChannelConfig(handle, &dest, &attribCurr))
        HAL_INFO("v4_rgn", "Attaching region %d...\n", handle);
    else if (attribCurr.overlay.point.x != rect.x ||
        attribCurr.overlay.point.x != rect.y) {
        HAL_INFO("v4_rgn", "Position has changed, reattaching "
            "region %d...\n", handle);
        for (dest.channel = 0; dest.channel < 2; dest.channel++)
            v4_rgn.fnDetachChannel(handle, &dest);
    }

    memset(&attrib, 0, sizeof(attrib));
    attrib.show = 1;
    attrib.type = V4_RGN_TYPE_OVERLAY;
    attrib.overlay.bgAlpha = 0;
    attrib.overlay.fgAlpha = opacity >> 1;
    attrib.overlay.point.x = rect.x;
    attrib.overlay.point.y = rect.y;
    attrib.overlay.layer = 7;
    attrib.overlay.attachDest = V4_RGN_DEST_MAIN;

    for (dest.channel = 0; dest.channel < 2; dest.channel++)
        if (v4_rgn.fnAttachChannel(handle, &dest, &attrib)) ;

    return EXIT_SUCCESS;
}

void v4_region_destroy(char handle)
{
    v4_sys_bind dest = { .module = V4_SYS_MOD_VENC, .device = _v4_venc_dev };

    for (dest.channel = 0; dest.channel < 2; dest.channel++)
        v4_rgn.fnDetachChannel(handle, &dest);    
    v4_rgn.fnDestroyRegion(handle);
}

int v4_region_setbitmap(int handle, hal_bitmap *bitmap)
{
    v4_rgn_bmp nativeBmp = { .data = bitmap->data, .pixFmt = V4_PIXFMT_ARGB1555,
        .size.height = bitmap->dim.height, .size.width = bitmap->dim.width };

    return v4_rgn.fnSetBitmap(handle, &nativeBmp);
}

float v4_system_readtemp(void)
{
    char v4a_device = 0;
    int val, prep;
    float result = 0.0 / 0.0;

    if (EQUALS(chip, "Hi3516AV300") ||
        EQUALS(chip, "Hi3516DV300") ||
        EQUALS(chip, "Hi3516CV500"))
        v4a_device = 1;

    prep = v4a_device ? 0x60fa0000 : 0xc3200000;
    if (hal_registry(v4a_device ? 0x120300b4 : 0x120280b4, &val, OP_READ) && prep != val)
        hal_registry(v4a_device ? 0x120300b4 : 0x120280b4, &prep, OP_WRITE);

    if (!hal_registry(v4a_device ? 0x120300bc : 0x120280bc, &val, OP_READ))
        return result;

    result = val & ((1 << 10) - 1);
    return ((result - (v4a_device ? 136 : 117)) / (v4a_device ? 793 : 798)) * 165 - 40;
}

#endif