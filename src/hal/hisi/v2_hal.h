#pragma once

#include "v2_common.h"
#include "v2_rgn.h"
#include "v2_sys.h"

#include "../support.h"

#include <fcntl.h>
#include <sys/ioctl.h>
#include <sys/select.h>
#include <unistd.h>

extern char keepRunning;

void v2_hal_deinit(void);
int v2_hal_init(void);

int v2_region_create(char handle, hal_rect rect, short opacity);
void v2_region_destroy(char handle);
int v2_region_setbitmap(int handle, hal_bitmap *bitmap);

float v2_system_readtemp(void);