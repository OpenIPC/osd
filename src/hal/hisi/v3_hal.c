#if defined(__arm__) && !defined(__ARM_PCS_VFP)

#include "v3_hal.h"

v3_rgn_impl     v3_rgn;
v3_sys_impl     v3_sys;

char _v3_venc_dev = 0;

void v3_hal_deinit(void)
{
    v3_rgn_unload(&v3_rgn);
    v3_sys_unload(&v3_sys);
}

int v3_hal_init(void)
{
    int ret;

    if (ret = v3_sys_load(&v3_sys))
        return ret;
    if (ret = v3_rgn_load(&v3_rgn))
        return ret;

    return EXIT_SUCCESS;
}

int v3_region_create(char handle, hal_rect rect, short opacity)
{
    int ret;

    v3_sys_bind dest = { .module = V3_SYS_MOD_VENC, .device = _v3_venc_dev };
    v3_rgn_cnf region, regionCurr;
    v3_rgn_chn attrib, attribCurr;

    rect.height += rect.height & 1;
    rect.width += rect.width & 1;

    memset(&region, 0, sizeof(region));
    region.type = V3_RGN_TYPE_OVERLAY;
    region.overlay.pixFmt = V3_PIXFMT_ARGB1555;
    region.overlay.size.width = rect.width;
    region.overlay.size.height = rect.height;
    region.overlay.canvas = handle + 1;

    if (v3_rgn.fnGetRegionConfig(handle, &regionCurr)) {
        HAL_INFO("v3_rgn", "Creating region %d...\n", handle);
        if (ret = v3_rgn.fnCreateRegion(handle, &region))
            return ret;
    } else if (regionCurr.type != region.type ||
        regionCurr.overlay.size.height != region.overlay.size.height || 
        regionCurr.overlay.size.width != region.overlay.size.width) {
        HAL_INFO("v3_rgn", "Parameters are different, recreating "
            "region %d...\n", handle);
        for (dest.channel = 0; dest.channel < 2; dest.channel++)
            v3_rgn.fnDetachChannel(handle, &dest);
        v3_rgn.fnDestroyRegion(handle);
        if (ret = v3_rgn.fnCreateRegion(handle, &region))
            return ret;
    }

    if (v3_rgn.fnGetChannelConfig(handle, &dest, &attribCurr))
        HAL_INFO("v3_rgn", "Attaching region %d...\n", handle);
    else if (attribCurr.overlay.point.x != rect.x ||
        attribCurr.overlay.point.x != rect.y) {
        HAL_INFO("v3_rgn", "Position has changed, reattaching "
            "region %d...\n", handle);
        for (dest.channel = 0; dest.channel < 2; dest.channel++)
            v3_rgn.fnDetachChannel(handle, &dest);
    }

    memset(&attrib, 0, sizeof(attrib));
    attrib.show = 1;
    attrib.type = V3_RGN_TYPE_OVERLAY;
    attrib.overlay.bgAlpha = 0;
    attrib.overlay.fgAlpha = opacity >> 1;
    attrib.overlay.point.x = rect.x;
    attrib.overlay.point.y = rect.y;
    attrib.overlay.layer = 7;

    for (dest.channel = 0; dest.channel < 2; dest.channel++)
        v3_rgn.fnAttachChannel(handle, &dest, &attrib);

    return EXIT_SUCCESS;
}

void v3_region_destroy(char handle)
{
    v3_sys_bind dest = { .module = V3_SYS_MOD_VENC, .device = _v3_venc_dev };

    for (dest.channel = 0; dest.channel < 2; dest.channel++)
        v3_rgn.fnDetachChannel(handle, &dest);
    v3_rgn.fnDestroyRegion(handle);
}

int v3_region_setbitmap(int handle, hal_bitmap *bitmap)
{
    v3_rgn_bmp nativeBmp = { .data = bitmap->data, .pixFmt = V3_PIXFMT_ARGB1555,
        .size.height = bitmap->dim.height, .size.width = bitmap->dim.width };

    return v3_rgn.fnSetBitmap(handle, &nativeBmp);
}

float v3_system_readtemp(void)
{
    char v3a_device = 0;
    int val, prep = 0x60fa0000;
    float result = 0.0 / 0.0;

    if (EQUALS(chip, "Hi3516AV200") ||
        EQUALS(chip, "Hi3519V101") ||
        EQUALS(chip, "Hi3556V100") ||
        EQUALS(chip, "Hi3559V100"))
        v3a_device = 1;

    if (hal_registry(v3a_device ? 0x120a0110 : 0x1203009c, &val, OP_READ) && prep != val)
        hal_registry(v3a_device ? 0x120a0110 : 0x1203009c, &prep, OP_WRITE);

    if (!hal_registry(v3a_device ? 0x120a0118 : 0x120300a4, &val, OP_READ))
        return result;

    result = val & ((1 << 10) - 1);
    return ((result - 125) / 806) * 165 - 40;
}

#endif