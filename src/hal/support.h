#include "types.h"
#include "hisi/v1_hal.h"
#include "hisi/v2_hal.h"
#include "hisi/v3_hal.h"
#include "hisi/v4_hal.h"
#include "star/i6_hal.h"
#include "star/i6c_hal.h"
#include "star/i6f_hal.h"

#include <errno.h>
#include <fcntl.h>
#include <linux/version.h>
#include <stdbool.h>
#include <stdio.h>
#include <sys/mman.h>
#include <unistd.h>

// Newer versions of musl have UAPI headers 
// that redefine struct sysinfo
#if defined(__GLIBC__) || defined(__UCLIBC__) \
    || LINUX_VERSION_CODE < KERNEL_VERSION(3, 4, 0)
#include <sys/sysinfo.h>
#else
#include <linux/sysinfo.h>
#endif

#ifdef __UCLIBC__
extern int asprintf(char **restrict strp, const char *restrict fmt, ...);
#endif

extern int sysinfo (struct sysinfo *__info);

void *mmap64(void *start, size_t len, int prot, int flags, int fd, off_t off);

extern char chnCount;

extern char chip[16];
extern char family[32];
extern hal_platform plat;
extern char sensor[16];
extern int series;

bool hal_registry(unsigned int addr, unsigned int *data, hal_register_op op);
void hal_identify(void);