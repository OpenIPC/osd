#include "m6_common.h"
#include "m6_rgn.h"
#include "m6_sys.h"

#include <sys/select.h>
#include <unistd.h>

extern char keepRunning;

void m6_hal_deinit(void);
int m6_hal_init(void);

int m6_region_create(char handle, hal_rect rect, short opacity);
void m6_region_deinit(void);
void m6_region_destroy(char handle);
void m6_region_init(void);
int m6_region_setbitmap(int handle, hal_bitmap *bitmap);