#pragma once

#include "v4_common.h"
#include "v4_rgn.h"
#include "v4_sys.h"

#include "../support.h"

#include <fcntl.h>
#include <sys/ioctl.h>
#include <sys/select.h>
#include <unistd.h>

extern char keepRunning;

void v4_hal_deinit(void);
int v4_hal_init(void);

int v4_region_create(char handle, hal_rect rect, short opacity);
void v4_region_destroy(char handle);
int v4_region_setbitmap(int handle, hal_bitmap *bitmap);

int v4_system_calculate_block(short width, short height, v4_common_pixfmt pixFmt,
    unsigned int alignWidth);
void v4_system_deinit(void);
int v4_system_init(char *snrConfig);