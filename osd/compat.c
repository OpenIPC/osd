#include <stdio.h>

#ifdef __INGENIC__
#include <sys/syscall.h>
#include <unistd.h>
#endif

void __ctype_b(void) {}

int __fgetc_unlocked(FILE *stream) {
  return fgetc(stream);
}

size_t _stdlib_mb_cur_max(void) {
  return 0;
}

#ifdef __INGENIC__
void __assert(void) {}
void __ctype_tolower(void) {}
void __pthread_register_cancel(void) {}
void __pthread_unregister_cancel(void) {}

void *mmap(void *start, size_t len, int prot, int flags, int fd, unsigned int off) {
	return (void *)syscall(SYS_mmap2, start, len, prot, flags, fd, off >> 12);
}
#elif defined(__SIGMASTAR__) && defined(__INFINITY6B0__)
void __stdin(void) {}
#endif