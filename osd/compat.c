#include <stdio.h>

int __ctype_b;

int __fgetc_unlocked(FILE *stream) {
  return fgetc(stream);
}

size_t _stdlib_mb_cur_max(void) {
  return 0;
}