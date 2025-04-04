#include "app_config.h"

const char *appconf_paths[] = {"./osd.yaml", "/etc/osd.yaml"};

struct AppConfig app_config;

static inline void *open_app_config(FILE **file, const char *flags) {
    const char **path = appconf_paths;
    *file = NULL;

    while (*path) {
        if (access(*path++, F_OK)) continue;
        if (*flags == 'w') {
            char bkPath[32];
            sprintf(bkPath, "%s.bak", *(path - 1));
            remove(bkPath);
            rename(*(path - 1), bkPath);
        }
        *file = fopen(*(path - 1), flags);
        break;
    }
}

void restore_app_config(void) {
    const char **path = appconf_paths;

    while (*path) {
        char bkPath[32];
        sprintf(bkPath, "%s.bak", *path);
        if (!access(bkPath, F_OK)) {
            remove(*path);
            rename(bkPath, *path);
        }
        path++;
    }
}

int save_app_config(void) {
    FILE *file;

    open_app_config(&file, "w");
    if (!file)
        HAL_ERROR("app_config", "Can't open config file for writing\n");

    fprintf(file, "system:\n");
    fprintf(file, "  web_port: %d\n", app_config.web_port);
    if (app_config.web_whitelist) {
        fprintf(file, "  web_whitelist: ");
        for (int i = 0; app_config.web_whitelist[i] && *app_config.web_whitelist[i]; i++) {
            fprintf(file, "    - %s\n", app_config.web_whitelist[i]);
        }
    }
    fprintf(file, "  web_enable_auth: %s\n", app_config.web_enable_auth ? "true" : "false");
    fprintf(file, "  web_auth_user: %s\n", app_config.web_auth_user);
    fprintf(file, "  web_auth_pass: %s\n", app_config.web_auth_pass);
    fprintf(file, "  web_enable_static: %s\n", app_config.web_enable_static ? "true" : "false");
    fprintf(file, "  web_server_thread_stack_size: %d\n", app_config.web_server_thread_stack_size);

    for (char i = 0; i < MAX_OSD; i++) {
        char imgEmpty = EMPTY(osds[i].img);
        char textEmpty = EMPTY(osds[i].text);
        if (imgEmpty && textEmpty) continue;
    
        fprintf(file, "osd%d:\n", i);
        if (!imgEmpty)
            fprintf(file, "  img: %s\n", osds[i].img);
        if (textEmpty)
            continue;
        fprintf(file, "  text: %s\n", osds[i].text);
        fprintf(file, "  font: %s\n", osds[i].font);
        fprintf(file, "  opal: %d\n", osds[i].opal);
        fprintf(file, "  posx: %d\n", osds[i].posx);
        fprintf(file, "  posy: %d\n", osds[i].posy);
        fprintf(file, "  size: %.1f\n", osds[i].size);
        fprintf(file, "  color: %#04x\n", osds[i].color);
    }

    fclose(file);
    return EXIT_SUCCESS;
}

enum ConfigError parse_app_config(void) {
    memset(&app_config, 0, sizeof(struct AppConfig));

    app_config.web_port = 8080;
    *app_config.web_whitelist[0] = '\0';
    app_config.web_enable_auth = false;
    app_config.web_enable_static = false;
    app_config.web_server_thread_stack_size = 32 * 1024;

    struct IniConfig ini;
    memset(&ini, 0, sizeof(struct IniConfig));

    FILE *file;
    open_app_config(&file, "r");
    if (!open_config(&ini, &file))  {
        printf("Can't find config osd.yaml in:\n"
            "    ./osd.yaml\n    /etc/osd.yaml\n");
        return -1;
    }

    enum ConfigError err;
    find_sections(&ini);

    int port, count;
    err = parse_int(&ini, "system", "web_port", 1, INT_MAX, &port);
    if (err != CONFIG_OK)
        goto RET_ERR;
    app_config.web_port = (unsigned short)port;
    parse_list(&ini, "system", "web_whitelist",
        sizeof(app_config.web_whitelist) / sizeof(*app_config.web_whitelist),
        &count, app_config.web_whitelist);
    *app_config.web_whitelist[count] = '\0';
    parse_bool(&ini, "system", "web_enable_auth", &app_config.web_enable_auth);
    parse_param_value(
        &ini, "system", "web_auth_user", app_config.web_auth_user);
    parse_param_value(
        &ini, "system", "web_auth_pass", app_config.web_auth_pass);
    err = parse_bool(
        &ini, "system", "web_enable_static", &app_config.web_enable_static);
    if (err != CONFIG_OK)
        goto RET_ERR;
    err = parse_int(
        &ini, "system", "web_server_thread_stack_size", 16 * 1024, INT_MAX,
        &app_config.web_server_thread_stack_size);
    if (err != CONFIG_OK)
        goto RET_ERR;

    for (char i = 0; i < MAX_OSD; i++) {
        char param[8];
        int val;
        sprintf(param, "osd%d", i);
        parse_param_value(&ini, param, "img", osds[i].img);
        parse_param_value(&ini, param, "text", osds[i].text);
        parse_param_value(&ini, param, "font", osds[i].font);
        err = parse_int(&ini, param, "opal", 0, 255, &val);
        if (err == CONFIG_OK) osds[i].opal = (unsigned char)val;
        err = parse_int(&ini, param, "posx", 0, SHRT_MAX, &val);
        if (err == CONFIG_OK) osds[i].posx = (short)val;
        err = parse_int(&ini, param, "posy", 0, SHRT_MAX, &val);
        if (err == CONFIG_OK) osds[i].posy = (short)val;
        parse_double(&ini, param, "size", 0, INT_MAX, &osds[i].size);
        parse_int(&ini, param, "color", 0, 0xFFFF, &osds[i].color);
        osds[i].updt = 1;
    }

    free(ini.str);
    return CONFIG_OK;
RET_ERR:
    free(ini.str);
    return err;
}