#pragma once

#include <stdio.h>
#include <string.h>

#define HAL_DANGER(mod, x, ...) \
    do { \
        fprintf(stderr, "[%s] \033[31m", (mod)); \
        fprintf(stderr, (x), ##__VA_ARGS__); \
        fprintf(stderr, "\033[0m"); \
    } while (0)

#define HAL_ERROR(mod, x, ...) \
    do { \
        fprintf(stderr, "[%s] \033[31m", (mod)); \
        fprintf(stderr, (x), ##__VA_ARGS__); \
        fprintf(stderr, "\033[0m"); \
        return EXIT_FAILURE; \
    } while (0)

#define HAL_INFO(mod, x, ...) \
    do { \
        fprintf(stderr, "[%s] ", (mod)); \
        fprintf(stderr, (x), ##__VA_ARGS__); \
    } while (0)

#define HAL_WARNING(mod, x, ...) \
    do { \
        fprintf(stderr, "[%s] \033[33m", (mod)); \
        fprintf(stderr, (x), ##__VA_ARGS__); \
        fprintf(stderr, "\033[0m"); \
    } while (0)

#ifndef CEILING
#define CEILING_POS(X) ((X-(int)(X)) > 0 ? (int)(X+1) : (int)(X))
#define CEILING_NEG(X) (int)(X)
#define CEILING(X) ( ((X) > 0) ? CEILING_POS(X) : CEILING_NEG(X) )
#endif

#define EMPTY(x) (x[0] == '\0')
#define ENDS_WITH(a, b) \
    ((strlen(a) > strlen(b)) && !strcmp(a + strlen(a) - strlen(b), b))
#define EQUALS(a, b) !strcmp(a, b)
#define EQUALS_CASE(a, b) !strcasecmp(a, b)
#define STARTS_WITH(a, b) !strncmp(a, b, strlen(b))