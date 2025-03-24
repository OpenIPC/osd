#pragma once

#include <limits.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "hal/config.h"
#include "hal/support.h"
#include "region.h"

struct AppConfig {
    // [system]
    unsigned short web_port;
    bool web_enable_auth;
    char web_auth_user[32];
    char web_auth_pass[32];
    bool web_enable_static;
    unsigned int web_server_thread_stack_size;
};

extern struct AppConfig app_config;
enum ConfigError parse_app_config(void);
void restore_app_config(void);
int save_app_config(void);
