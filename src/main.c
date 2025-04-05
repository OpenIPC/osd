#include "app_config.h"
#include "hal/macros.h"
#include "server.h"

#include <getopt.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

char graceful = 0, keepRunning = 1;

void handle_error(int signo) {
    write(STDERR_FILENO, "Error occurred! Quitting...\n", 28);
    keepRunning = 0;
    exit(EXIT_FAILURE);
}

void handle_exit(int signo) {
    write(STDERR_FILENO, "Graceful shutdown...\n", 21);
    keepRunning = 0;
    graceful = 1;
}

int main(int argc, char *argv[]) {
    {
        char signal_error[] = {SIGABRT, SIGBUS, SIGFPE, SIGILL, SIGSEGV};
        char signal_exit[] = {SIGINT, SIGQUIT, SIGTERM};
        char signal_null[] = {EPIPE, SIGPIPE};

        for (char *s = signal_error; s < (&signal_error)[1]; s++)
            signal(*s, handle_error);
        for (char *s = signal_exit; s < (&signal_exit)[1]; s++)
            signal(*s, handle_exit);
        for (char *s = signal_null; s < (&signal_null)[1]; s++)
            signal(*s, NULL);
    }

    hal_identify();

    if (!*family)
        HAL_ERROR("hal", "Unsupported chip family! Quitting...\n");

    fprintf(stderr, "\033[7m OSD for %s \033[0m\n", family);
    fprintf(stderr, "Chip ID: %s\n", chip);

    if (parse_app_config() != CONFIG_OK)
        HAL_ERROR("hal", "Can't load app config 'osd.yaml'\n");

    switch (plat) {
#if defined(__ARM_PCS_VFP)
        case HAL_PLATFORM_I6:  i6_hal_init(); break;
        case HAL_PLATFORM_I6C: i6c_hal_init(); break;
        case HAL_PLATFORM_M6:  m6_hal_init(); break;
#elif defined(__ARM_PCS)
        case HAL_PLATFORM_V1:  v1_hal_init(); break;
        case HAL_PLATFORM_V2:  v2_hal_init(); break;
        case HAL_PLATFORM_V3:  v3_hal_init(); break;
        case HAL_PLATFORM_V4:  v4_hal_init(); break;
#endif
    }

    start_server();

    start_region_handler();

    while (keepRunning) {
        sleep(1);
    }

    stop_region_handler();

    stop_server();

    switch (plat) {
#if defined(__ARM_PCS_VFP)
        case HAL_PLATFORM_I6:  i6_hal_deinit(); break;
        case HAL_PLATFORM_I6C: i6c_hal_deinit(); break;
        case HAL_PLATFORM_M6:  m6_hal_deinit(); break;
#elif defined(__ARM_PCS)
        case HAL_PLATFORM_V1:  v1_hal_deinit(); break;
        case HAL_PLATFORM_V2:  v2_hal_deinit(); break;
        case HAL_PLATFORM_V3:  v3_hal_deinit(); break;
        case HAL_PLATFORM_V4:  v4_hal_deinit(); break;
#endif
    }

    if (!graceful)
        restore_app_config();

    fprintf(stderr, "Main thread is shutting down...\n");
    return EXIT_SUCCESS;
}