#include "region.h"

const double inv16 = 1.0 / 16.0;

int create_region(int channel, int handle, int x, int y, int width, int height)
{
    int s32Ret;
#ifdef __SIGMASTAR__
    MI_RGN_ChnPort_t stChn;

    MI_RGN_Attr_t stRegionCurrent;
    MI_RGN_Attr_t stRegion;

    MI_RGN_ChnPortParam_t stChnAttr;
    MI_RGN_ChnPortParam_t stChnAttrCurrent;
#else
    MPP_CHN_S stChn;

    RGN_ATTR_S stRegionCurrent;
    RGN_ATTR_S stRegion;

    RGN_CHN_ATTR_S stChnAttr;
    RGN_CHN_ATTR_S stChnAttrCurrent;
#endif

    int enPixelFmt;
    unsigned int au32BgColor[3] = {0xffffffff, 0x0000000, 1};

#ifdef __SIGMASTAR__
    stChn.eModId = E_MI_RGN_MODID_VPE;
    stChn.s32OutputPortId = 0;
#else
    stChn.enModId = HI_ID_VENC;
#endif
    stChn.s32DevId = 0;
    stChn.s32ChnId = channel;

#ifdef __SIGMASTAR__
    stRegion.eType = E_MI_RGN_TYPE_OSD;
    stRegion.stOsdInitParam.stSize.u32Height = height;
    stRegion.stOsdInitParam.stSize.u32Width = width;
    stRegion.stOsdInitParam.ePixelFmt = PIXEL_FORMAT_1555;
#else
    stRegion.enType = OVERLAY_RGN;
    stRegion.unAttr.stOverlay.stSize.u32Height = height;
    stRegion.unAttr.stOverlay.stSize.u32Width = width;
    stRegion.unAttr.stOverlay.u32CanvasNum = 2;
    stRegion.unAttr.stOverlay.enPixelFmt = PIXEL_FORMAT_1555;
    stRegion.unAttr.stOverlay.u32BgColor = au32BgColor[1];
#endif

#ifdef __SIGMASTAR__
    // MI_RGN_DetachFromChn(handle, &stChn);
    // MI_RGN_Destroy(handle);
    // memset(&stRegionCurrent, 0, sizeof(MI_RGN_Attr_t));
    s32Ret = MI_RGN_GetAttr(handle, &stRegionCurrent);
#else
    // HI_MPI_RGN_DetachFromChn(HandleNum,&stChn);
    // HI_MPI_RGN_Destroy(HandleNum);
    s32Ret = HI_MPI_RGN_GetAttr(handle, &stRegionCurrent);
#endif
    if (s32Ret)
    {
        fprintf(stderr, "[%s:%d]RGN_GetAttr failed with %#x , creating region %d...\n", __func__, __LINE__, s32Ret, handle);
#ifdef __SIGMASTAR__
        s32Ret = MI_RGN_Create(handle, &stRegion);
#else
        s32Ret = HI_MPI_RGN_Create(handle, &stRegion);
#endif
        if (s32Ret)
        {
            fprintf(stderr, "[%s:%d]RGN_Create failed with %#x!\n", __func__, __LINE__, s32Ret);
            return -1;
        }
    }
    else
    {
#ifdef __SIGMASTAR__
        if (stRegionCurrent.stOsdInitParam.stSize.u32Height != stRegion.stOsdInitParam.stSize.u32Height || stRegionCurrent.stOsdInitParam.stSize.u32Width != stRegion.stOsdInitParam.stSize.u32Width)
#else
        if (stRegionCurrent.unAttr.stOverlay.stSize.u32Height != stRegion.unAttr.stOverlay.stSize.u32Height || stRegionCurrent.unAttr.stOverlay.stSize.u32Width != stRegion.unAttr.stOverlay.stSize.u32Width)
#endif
        {
            fprintf(stderr, "[%s:%d] Region parameters are different, recreating ... \n", __func__, __LINE__);
#ifdef __SIGMASTAR__
            MI_RGN_DetachFromChn(handle, &stChn);
            MI_RGN_Destroy(handle);
            s32Ret = MI_RGN_Create(handle, &stRegion);
#else
            HI_MPI_RGN_DetachFromChn(handle, &stChn);
            HI_MPI_RGN_Destroy(handle);
            s32Ret = HI_MPI_RGN_Create(handle, &stRegion);
#endif
            if (s32Ret)
            {
                fprintf(stderr, "[%s:%d]RGN_Create failed with %#x!\n", __func__, __LINE__, s32Ret);
                return -1;
            }
        }
    }

#ifdef __SIGMASTAR__
    s32Ret = MI_RGN_GetDisplayAttr(handle, &stChn, &stChnAttrCurrent);
#else
    s32Ret = HI_MPI_RGN_GetDisplayAttr(handle, &stChn, &stChnAttrCurrent);
#endif
    if (s32Ret)
        fprintf(stderr, "[%s:%d]RGN_GetDisplayAttr failed with %#x %d, attaching...\n", __func__, __LINE__, s32Ret, handle);
#ifdef __SIGMASTAR__
    else if (stChnAttrCurrent.stPoint.u32X != x || stChnAttrCurrent.stPoint.u32Y != y)
#else
    else if (stChnAttrCurrent.unChnAttr.stOverlayChn.stPoint.s32X != x || stChnAttrCurrent.unChnAttr.stOverlayChn.stPoint.s32Y != y)
#endif
    {
        fprintf(stderr, "[%s:%d] Position has changed, detaching handle %d from channel %d...\n", __func__, __LINE__, s32Ret, handle, &stChn.s32ChnId);
#ifdef __SIGMASTAR__
        MI_RGN_DetachFromChn(handle, &stChn);
#else
        HI_MPI_RGN_DetachFromChn(handle, &stChn);
#endif
    }

#ifdef __SIGMASTAR__
    memset(&stChnAttr, 0, sizeof(MI_RGN_ChnPortParam_t));
    stChnAttr.bShow = 1;
    stChnAttr.stPoint.u32X = x;
    stChnAttr.stPoint.u32Y = y;
    stChnAttr.unPara.stOsdChnPort.u32Layer = 0;
    stChnAttr.unPara.stOsdChnPort.stOsdAlphaAttr.eAlphaMode = E_MI_RGN_PIXEL_ALPHA;
    stChnAttr.unPara.stOsdChnPort.stOsdAlphaAttr.stAlphaPara.stArgb1555Alpha.u8BgAlpha = 0;
    stChnAttr.unPara.stOsdChnPort.stOsdAlphaAttr.stAlphaPara.stArgb1555Alpha.u8FgAlpha = 255;
    stChnAttr.unPara.stOsdChnPort.stColorInvertAttr.bEnableColorInv = 0;
    // stChnAttr.unPara.stOsdChnPort.stColorInvertAttr.eInvertColorMode = E_MI_RGN_BELOW_LUMA_THRESHOLD;
    // stChnAttr.unPara.stOsdChnPort.stColorInvertAttr.u16LumaThreshold = 128;
    // stChnAttr.unPara.stOsdChnPort.stColorInvertAttr.u16WDivNum = stRegion.stOsdInitParam.stSize.u32Width * inv16;
    // stChnAttr.unPara.stOsdChnPort.stColorInvertAttr.u16HDivNum = stRegion.stOsdInitParam.stSize.u32Height * inv16;
#else
    stChnAttr.bShow = 1;
    stChnAttr.enType = OVERLAY_RGN;
    stChnAttr.unChnAttr.stOverlayChn.u32BgAlpha = 0;
    stChnAttr.unChnAttr.stOverlayChn.u32FgAlpha = 128;
    stChnAttr.unChnAttr.stOverlayChn.stQpInfo.bQpDisable = 0;
    stChnAttr.unChnAttr.stOverlayChn.stQpInfo.bAbsQp = 0;
    stChnAttr.unChnAttr.stOverlayChn.stQpInfo.s32Qp = 0;
    stChnAttr.unChnAttr.stOverlayChn.stInvertColor.stInvColArea.u32Height = width;
    stChnAttr.unChnAttr.stOverlayChn.stInvertColor.stInvColArea.u32Width = height;
    stChnAttr.unChnAttr.stOverlayChn.stInvertColor.u32LumThresh = 128;
    stChnAttr.unChnAttr.stOverlayChn.stInvertColor.enChgMod = LESSTHAN_LUM_THRESH;
    stChnAttr.unChnAttr.stOverlayChn.stInvertColor.bInvColEn = 0;
#ifndef __16CV300__
    stChnAttr.unChnAttr.stOverlayChn.u16ColorLUT[0] = 0x3e0;
    stChnAttr.unChnAttr.stOverlayChn.u16ColorLUT[1] = 0x7FFF;
    stChnAttr.unChnAttr.stOverlayChn.enAttachDest = ATTACH_JPEG_MAIN;
#endif
    stChnAttr.unChnAttr.stOverlayChn.stPoint.s32X = x;
    stChnAttr.unChnAttr.stOverlayChn.stPoint.s32Y = y;
    stChnAttr.unChnAttr.stOverlayChn.u32Layer = 7;
#endif

#ifdef __SIGMASTAR__
    MI_RGN_AttachToChn(handle, &stChn, &stChnAttr);
#else
    HI_MPI_RGN_AttachToChn(handle, &stChn, &stChnAttr);
#endif

    return s32Ret;
}

int REGION_MST_LoadBmp(const char *filename, BITMAP *bitmap, int bFil, unsigned int u16FilColor, int enPixelFmt)
{
    OSD_SURFACE_S Surface;
    OSD_BITMAPFILEHEADER bmpFileHeader;
    OSD_BITMAPINFO bmpInfo;
    int s32BytesPerPix = 2;
    unsigned char *pu8Data;
    int R_Value;
    int G_Value;
    int B_Value;
    int Gr_Value;
    unsigned char Value_tmp;
    unsigned char Value;
    int s32Width;

    if (GetBmpInfo(filename, &bmpFileHeader, &bmpInfo) < 0)
    {
        fprintf(stderr, "GetBmpInfo err!\n");
        return -1;
    }

    switch (enPixelFmt)
    {
    case PIXEL_FORMAT_4444:
        Surface.enColorFmt = OSD_COLOR_FMT_RGB4444;
        break;
    case PIXEL_FORMAT_1555:
    case PIXEL_FORMAT_2BPP:
        Surface.enColorFmt = OSD_COLOR_FMT_RGB1555;
        break;
    case PIXEL_FORMAT_8888:
        Surface.enColorFmt = OSD_COLOR_FMT_RGB8888;
        s32BytesPerPix = 4;
        break;
    default:
        fprintf(stderr, "enPixelFormat err %d \n", enPixelFmt);
        return -1;
    }

    bitmap->pData = malloc(s32BytesPerPix * (bmpInfo.bmiHeader.biWidth) * (bmpInfo.bmiHeader.biHeight));
    if (bitmap->pData == NULL)
    {
        fprintf(stderr, "malloc osd memory err!\n");
        return -1;
    }

    CreateSurfaceByBitMap(filename, &Surface, (unsigned char *)(bitmap->pData));

    bitmap->u32Width = Surface.u16Width;
    bitmap->u32Height = Surface.u16Height;
    bitmap->enPixelFormat = enPixelFmt;

    int i, j, k;
    unsigned char *pu8Temp;

#ifndef __16CV300__
    if (enPixelFmt == PIXEL_FORMAT_2BPP)
    {
        s32Width = DIV_UP(bmpInfo.bmiHeader.biWidth, 4);
        pu8Data = malloc((s32Width) * (bmpInfo.bmiHeader.biHeight));
        if (NULL == pu8Data)
        {
            fprintf(stderr, "malloc osd memory err!\n");
            return -1;
        }
    }

#endif
    if (enPixelFmt != PIXEL_FORMAT_2BPP)
    {
        unsigned short *pu16Temp;
        pu16Temp = (unsigned short *)bitmap->pData;
        if (bFil)
        {
            for (i = 0; i < bitmap->u32Height; i++)
            {
                for (j = 0; j < bitmap->u32Width; j++)
                {
                    if (u16FilColor == *pu16Temp)
                    {
                        *pu16Temp &= 0x7FFF;
                    }

                    pu16Temp++;
                }
            }
        }
    }
    else
    {
        unsigned short *pu16Temp;

        pu16Temp = (unsigned short *)bitmap->pData;
        pu8Temp = (unsigned char *)pu8Data;

        for (i = 0; i < bitmap->u32Height; i++)
        {
            for (j = 0; j < bitmap->u32Width / 4; j++)
            {
                Value = 0;

                for (k = j; k < j + 4; k++)
                {
                    B_Value = *pu16Temp & 0x001F;
                    G_Value = *pu16Temp >> 5 & 0x001F;
                    R_Value = *pu16Temp >> 10 & 0x001F;
                    pu16Temp++;

                    Gr_Value = (R_Value * 299 + G_Value * 587 + B_Value * 144 + 500) / 1000;
                    if (Gr_Value > 16)
                        Value_tmp = 0x01;
                    else
                        Value_tmp = 0x00;
                    Value = (Value << 2) + Value_tmp;
                }
                *pu8Temp = Value;
                pu8Temp++;
            }
        }
        free(bitmap->pData);
        bitmap->pData = pu8Data;
    }

    return 0;
}

int set_bitmap(unsigned int handle, BITMAP *bitmap)
{
#ifdef __SIGMASTAR__
    int s32Ret = MI_RGN_SetBitMap(handle, (MI_RGN_Bitmap_t *)(bitmap));
#else
    int s32Ret = HI_MPI_RGN_SetBitMap(handle, (BITMAP_S *)(bitmap));
#endif
    if (s32Ret)
    {
        fprintf(stderr, "RGN_SetBitMap failed with %#x!\n", s32Ret);
        return -1;
    }
    return s32Ret;
}

int load_bitmap(unsigned int handle, int enPixelFmt)
{
    BITMAP* bitmap;
    char path[32];
    sprintf(path, "/tmp/osd%d.bmp", handle);
    REGION_MST_LoadBmp(path, bitmap, 0, 0, enPixelFmt);
    
    int s32Ret = set_bitmap(handle, bitmap);
    if (s32Ret)
        fprintf(stderr, "load_bitmap failed!Handle:%d\n", handle);
    free(bitmap->pData);
    return s32Ret;
}