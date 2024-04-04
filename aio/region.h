#ifndef OSD_H_
#define OSD_H_

#ifdef __cplusplus
#if __cplusplus
extern "C"
{
#endif
#endif

#include "common.h"
#include "bitmap.h"

    int create_region(int channel, int handle, int x, int y, int width, int height);

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif
#endif