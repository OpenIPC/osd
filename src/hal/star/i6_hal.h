#pragma once

#include "i6_common.h"
#include "i6_rgn.h"
#include "i6_sys.h"

#include "../support.h"

#include <sys/select.h>
#include <unistd.h>

extern char keepRunning;

void i6_hal_deinit(void);
int i6_hal_init(void);

int i6_region_create(char handle, hal_rect rect, short opacity);
void i6_region_deinit(void);
void i6_region_destroy(char handle);
void i6_region_init(void);
int i6_region_setbitmap(int handle, hal_bitmap *bitmap);