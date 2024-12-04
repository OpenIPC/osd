#pragma once

#include "v1_common.h"
#include "v1_rgn.h"
#include "v1_sys.h"

#include <fcntl.h>
#include <sys/ioctl.h>
#include <sys/select.h>
#include <unistd.h>

extern char keepRunning;

void v1_hal_deinit(void);
int v1_hal_init(void);

int v1_region_create(char handle, hal_rect rect, short opacity);
void v1_region_destroy(char handle);
int v1_region_setbitmap(int handle, hal_bitmap *bitmap);