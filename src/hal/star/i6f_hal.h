#include "i6f_common.h"
#include "i6f_rgn.h"
#include "i6f_sys.h"

#include <sys/select.h>
#include <unistd.h>

extern char keepRunning;

void i6f_hal_deinit(void);
int i6f_hal_init(void);

int i6f_region_create(char handle, hal_rect rect, short opacity);
void i6f_region_deinit(void);
void i6f_region_destroy(char handle);
void i6f_region_init(void);
int i6f_region_setbitmap(int handle, hal_bitmap *bitmap);