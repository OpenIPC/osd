#include <stdio.h>

void __ctype_b(void) {}

int __fgetc_unlocked(FILE *stream) {
  return fgetc(stream);
}

size_t _stdlib_mb_cur_max(void) {
  return 0;
}

#if defined(__SIGMASTAR__) && defined(__INFINITY6B0__)
void __stdin(void) {}
#endif