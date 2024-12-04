#pragma once

#include "v3_common.h"
#include "v3_rgn.h"
#include "v3_sys.h"

#include <fcntl.h>
#include <sys/ioctl.h>
#include <sys/select.h>
#include <unistd.h>

extern char keepRunning;

void v3_hal_deinit(void);
int v3_hal_init(void);

int v3_region_create(char handle, hal_rect rect, short opacity);
void v3_region_destroy(char handle);
int v3_region_setbitmap(int handle, hal_bitmap *bitmap);