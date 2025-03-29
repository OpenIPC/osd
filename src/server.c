#include "server.h"

#define MAX_CLIENTS 50
#define REQSIZE 512 * 1024

IMPORT_STR(.rodata, "../res/index.html", indexhtml);
extern const char indexhtml[];

struct Request {
    int clntFd;
    char *input, *method, *payload, *prot, *query, *uri;
    int paysize, total;
};

struct Client {
    int socket_fd;
} client_fds[MAX_CLIENTS];

struct Header {
    char *name, *value;
} reqhdr[17] = {{"\0", "\0"}};

int server_fd = -1;
pthread_t server_thread_id;
pthread_mutex_t client_fds_mutex;

const char error400[] = "HTTP/1.1 400 Bad Request\r\n" \
                        "Content-Type: text/plain\r\n" \
                        "Connection: close\r\n" \
                        "\r\n" \
                        "The server has no handler to the request.\r\n";
const char error404[] = "HTTP/1.1 404 Not Found\r\n" \
                        "Content-Type: text/plain\r\n" \
                        "Connection: close\r\n" \
                        "\r\n" \
                        "The requested resource was not found.\r\n";
const char error405[] = "HTTP/1.1 405 Method Not Allowed\r\n" \
                        "Content-Type: text/plain\r\n" \
                        "Connection: close\r\n" \
                        "\r\n" \
                        "This method is not handled by this server.\r\n";
const char error500[] = "HTTP/1.1 500 Internal Server Error\r\n" \
                        "Content-Type: text/plain\r\n" \
                        "Connection: close\r\n" \
                        "\r\n" \
                        "An invalid operation was caught on this request.\r\n";

void close_socket_fd(int socket_fd) {
    shutdown(socket_fd, SHUT_RDWR);
    close(socket_fd);
}

void free_client(int i) {
    if (client_fds[i].socket_fd < 0)
        return;
    close_socket_fd(client_fds[i].socket_fd);
    client_fds[i].socket_fd = -1;
}

int send_to_fd(int client_fd, char *buf, ssize_t size) {
    ssize_t sent = 0, len = 0;
    if (client_fd < 0)
        return -1;
    while (sent < size) {
        len = send(client_fd, buf + sent, size - sent, MSG_NOSIGNAL);
        if (len < 0)
            return -1;
        sent += len;
    }
    return 0;
}

int send_to_fd_nonblock(int client_fd, char *buf, ssize_t size) {
    if (client_fd < 0)
        return -1;
    send(client_fd, buf, size, MSG_DONTWAIT | MSG_NOSIGNAL);
    return 0;
}

int send_to_client(int i, char *buf, ssize_t size) {
    if (send_to_fd(client_fds[i].socket_fd, buf, size) < 0) {
        free_client(i);
        return -1;
    }
    return 0;
}

static void send_and_close(int client_fd, char *buf, ssize_t size) {
    send_to_fd(client_fd, buf, size);
    close_socket_fd(client_fd);
}

int send_file(const int client_fd, const char *path) {
    if (!access(path, F_OK)) {
        const char *mime = (path);
        FILE *file = fopen(path, "r");
        if (file == NULL) {
            close_socket_fd(client_fd);
            return 0;
        }
        char header[1024];
        int header_len = sprintf(
            header, "HTTP/1.1 200 OK\r\n"
            "Content-Type: %s\r\n"
            "Transfer-Encoding: chunked\r\n"
            "Connection: keep-alive\r\n\r\n",
            mime);
        send_to_fd(client_fd, header, header_len); // zero ending string!
        const int buf_size = 1024;
        char buf[buf_size + 2];
        char len_buf[50];
        ssize_t len_size;
        while (1) {
            ssize_t size = fread(buf, sizeof(char), buf_size, file);
            if (size <= 0)
                break;
            len_size = sprintf(len_buf, "%zX\r\n", size);
            buf[size++] = '\r';
            buf[size++] = '\n';
            send_to_fd(client_fd, len_buf, len_size); // send <SIZE>\r\n
            send_to_fd(client_fd, buf, size);         // send <DATA>\r\n
        }
        char end[] = "0\r\n\r\n";
        send_to_fd(client_fd, end, sizeof(end));
        fclose(file);
        close_socket_fd(client_fd);
        return 1;
    }
    send_and_close(client_fd, (char*)error404, strlen(error404));
    return 0;
}

void send_binary(const int client_fd, const char *data, const long size) {
    char *buf;
    int buf_len = asprintf(
        &buf,
        "HTTP/1.1 200 OK\r\n" \
        "Content-Type: application/octet-stream\r\n" \
        "Content-Length: %zu\r\n" \
        "Connection: close\r\n" \
        "\r\n",
        size);
    send_to_fd(client_fd, buf, buf_len);
    send_to_fd(client_fd, (char*)data, size);
    send_to_fd(client_fd, "\r\n", 2);
    close_socket_fd(client_fd);
    free(buf);
}

void send_html(const int client_fd, const char *data) {
    char *buf;
    int buf_len = asprintf(
        &buf,
        "HTTP/1.1 200 OK\r\n" \
        "Content-Type: text/html\r\n" \
        "Content-Length: %zu\r\n" \
        "Connection: close\r\n" \
        "\r\n%s",
        strlen(data), data);
    buf[buf_len++] = 0;
    send_and_close(client_fd, buf, buf_len);
    free(buf);
}

char *request_header(const char *name)
{
    struct Header *h = reqhdr;
    for (; h->name; h++)
        if (!strcasecmp(h->name, name))
            return h->value;
    return NULL;
}

struct Header *request_headers(void) { return reqhdr; }

void parse_request(struct Request *req) {
    struct sockaddr_in client_sock;
    socklen_t client_sock_len = sizeof(client_sock);
    memset(&client_sock, 0, client_sock_len);

    req->total = 0;
    int received = recv(req->clntFd, req->input, REQSIZE, 0);
    if (received < 0)
        HAL_WARNING("server", "recv() error\n");
    else if (!received)
        HAL_WARNING("server", "Client disconnected unexpectedly\n");
    req->total += received;

    if (req->total <= 0) return;

    getpeername(req->clntFd,
        (struct sockaddr *)&client_sock, &client_sock_len);

    char *state = NULL;
    req->method = strtok_r(req->input, " \t\r\n", &state);
    req->uri = strtok_r(NULL, " \t", &state);
    req->prot = strtok_r(NULL, " \t\r\n", &state);

    HAL_INFO("server", "\x1b[32mNew request: (%s) %s\n"
        "         Received from: %s\x1b[0m\n",
        req->method, req->uri, inet_ntoa(client_sock.sin_addr));

    if (req->query = strchr(req->uri, '?'))
        *req->query++ = '\0';
    else
        req->query = req->uri - 1;

    struct Header *h = reqhdr;
    char *l;
    while (h < reqhdr + 16)
    {
        char *k, *v, *e;
        if (!(k = strtok_r(NULL, "\r\n: \t", &state)))
            break;
        v = strtok_r(NULL, "\r\n", &state);
        while (*v && *v == ' ' && v++);
        h->name = k;
        h++->value = v;
#ifdef DEBUG_HTTP
        fprintf(stderr, "         (H) %s: %s\n", k, v);
#endif
        e = v + 1 + strlen(v);
        if (e[1] == '\r' && e[2] == '\n')
            break;
    }

    l = request_header("Content-Length");
    req->paysize = l ? atol(l) : 0;

    while (l && req->total < req->paysize) {
        received = recv(req->clntFd, req->input + req->total, REQSIZE - req->total, 0);
        if (received < 0) {
            HAL_WARNING("server", "recv() error\n");
            break;
        } else if (!received) {
            HAL_WARNING("server", "Client disconnected unexpectedly\n");
            break;
        }
        req->total += received;
    }

    req->payload = strtok_r(NULL, "\r\n", &state);
}

void respond_request(struct Request *req) {
    char response[256];

    if (!EQUALS(req->method, "GET") && !EQUALS(req->method, "POST")) {
        send_and_close(req->clntFd, (char*)error405, strlen(error405));
        return;
    }

    if (app_config.web_enable_auth) {
        char *auth = request_header("Authorization");
        char cred[66], valid[256];

        strcpy(cred, app_config.web_auth_user);
        strcpy(cred + strlen(app_config.web_auth_user), ":");
        strcpy(cred + strlen(app_config.web_auth_user) + 1, app_config.web_auth_pass);
        strcpy(valid, "Basic ");
        base64_encode(valid + 6, cred, strlen(cred));

        if (!auth || !EQUALS(auth, valid)) {
            int respLen = sprintf(response,
                "HTTP/1.1 401 Unauthorized\r\n"
                "Content-Type: text/plain\r\n"
                "WWW-Authenticate: Basic realm=\"Access the camera services\"\r\n"
                "Connection: close\r\n\r\n"
            );
            send_and_close(req->clntFd, response, respLen);
            return;
        }
    }

    if (EQUALS(req->uri, "/exit")) {
        int respLen = sprintf(
            response, "HTTP/1.1 200 OK\r\n"
                    "Connection: close\r\n\r\n"
                    "Closing...");
        send_and_close(req->clntFd, response, respLen);
        keepRunning = 0;
        graceful = 1;
        return;
    }

    if (EQUALS(req->uri, "/") || EQUALS(req->uri, "/index.htm") || EQUALS(req->uri, "/index.html")) {
        send_html(req->clntFd, indexhtml);
        return;
    }

    if (STARTS_WITH(req->uri, "/api/osd/")) {
        char *remain;
        int respLen;
        short id = strtol(req->uri + 9, &remain, 10);
        char saved = 0;
        if (remain == req->uri + 9 || id < 0 || id >= MAX_OSD) {
            respLen = sprintf(response,
                "HTTP/1.1 404 Not Found\r\n"
                "Content-Type: text/plain\r\n"
                "Connection: close\r\n"
                "\r\n"
                "The requested OSD ID was not found.\r\n"
            );
            send_and_close(req->clntFd, response, respLen);
            return;
        }
        if (EQUALS(req->method, "POST")) {
            char *type = request_header("Content-Type");
            if (STARTS_WITH(type, "multipart/form-data")) {
                char *bound = strstr(type, "boundary=") + strlen("boundary=");

                char *payloadb = strstr(req->payload, bound);
                payloadb = memstr(payloadb, "\r\n\r\n", req->total - (payloadb - req->input), 4);
                if (payloadb) payloadb += 4;

                char *payloade = memstr(payloadb, bound,
                    req->total - (payloadb - req->input), strlen(bound));
                if (payloade) payloade -= 4;

                char path[32];
                sprintf(path, "/tmp/osd%d.bmp", id);

                if (!memcmp(payloadb, "\x89\x50\x4E\x47\xD\xA\x1A\xA", 8)) {
                    spng_ctx *ctx = spng_ctx_new(0);
                    if (!ctx) {
                        HAL_DANGER("server", "Constructing the PNG decoder context failed!\n");
                        goto png_error;
                    }

                    spng_set_crc_action(ctx, SPNG_CRC_USE, SPNG_CRC_USE);
                    spng_set_chunk_limits(ctx,
                        app_config.web_server_thread_stack_size,
                        app_config.web_server_thread_stack_size);
                    spng_set_png_buffer(ctx, payloadb, payloade - payloadb);

                    struct spng_ihdr ihdr;
                    int err = spng_get_ihdr(ctx, &ihdr);
                    if (err) {
                        HAL_DANGER("server", "Parsing the PNG IHDR chunk failed!\nError: %s\n", spng_strerror(err));
                        goto png_error;
                    }
#ifdef DEBUG_IMAGE
    printf("[image] (PNG) width: %u, height: %u, depth: %u, color type: %u -> %s\n",
        ihdr.width, ihdr.height, ihdr.bit_depth, ihdr.color_type, color_type_str(ihdr.color_type));
    printf("        compress: %u, filter: %u, interl: %u\n",
        ihdr.compression_method, ihdr.filter_method, ihdr.interlace_method);
#endif

                    struct spng_plte plte = {0};
                    err = spng_get_plte(ctx, &plte);
                    if (err && err != SPNG_ECHUNKAVAIL) {
                        HAL_DANGER("server", "Parsing the PNG PLTE chunk failed!\nError: %s\n", spng_strerror(err));
                        goto png_error;
                    }
#ifdef DEBUG_IMAGE
    printf("        palette: %u\n", plte.n_entries);
#endif

                    size_t bitmapsize;
                    err = spng_decoded_image_size(ctx, SPNG_FMT_RGBA8, &bitmapsize);
                    if (err) {
                        HAL_DANGER("server", "Recovering the resulting image size failed!\nError: %s\n", spng_strerror(err));
                        goto png_error;
                    }

                    unsigned char *bitmap = malloc(bitmapsize);
                    if (!bitmap) {
                        HAL_DANGER("server", "Allocating the PNG bitmap output buffer for size %u failed!\n", bitmapsize);
                        goto png_error;
                    }

                    err = spng_decode_image(ctx, bitmap, bitmapsize, SPNG_FMT_RGBA8, 0);
                    if (!bitmap) {
                        HAL_DANGER("server", "Decoding the PNG image failed!\nError: %s\n", spng_strerror(err));
                        goto png_error;
                    }

                    int headersize = 2 + sizeof(bitmapfile) + sizeof(bitmapinfo) + sizeof(bitmapfields);
                    bitmapfile headerfile = {.size = bitmapsize + headersize, .offBits = headersize};
                    bitmapinfo headerinfo = {.size = sizeof(bitmapinfo), 
                                             .width = ihdr.width, .height = -ihdr.height, .planes = 1, .bitCount = 32,
                                             .compression = 3, .sizeImage = bitmapsize, .xPerMeter = 2835, .yPerMeter = 2835};
                    bitmapfields headerfields = {.redMask = 0xFF << 0, .greenMask = 0xFF << 8, .blueMask = 0xFF << 16, .alphaMask = 0xFF << 24,
                                                 .clrSpace = {'W', 'i', 'n', ' '}};

                    FILE *img = fopen(path, "wb");
                    fwrite("BM", sizeof(char), 2, img);
                    fwrite(&headerfile, sizeof(bitmapfile), 1, img);
                    fwrite(&headerinfo, sizeof(bitmapinfo), 1, img);
                    fwrite(&headerfields, sizeof(bitmapfields), 1, img);
                    fwrite(bitmap, sizeof(char), bitmapsize, img);
                    fclose(img);

png_error:
                    spng_ctx_free(ctx);
                    free(bitmap);
                    send_and_close(req->clntFd, (char*)error500, strlen(error500));
                } else {
                    FILE *img = fopen(path, "wb");
                    fwrite(payloadb, sizeof(char), payloade - payloadb, img);
                    fclose(img);
                }

                strcpy(osds[id].text, "");
                osds[id].updt = 1;
            } else {
                respLen = sprintf(response,
                    "HTTP/1.1 415 Unsupported Media Type\r\n"
                    "Content-Type: text/plain\r\n"
                    "Connection: close\r\n"
                    "\r\n"
                    "The payload must be presented as multipart/form-data.\r\n"
                );
                send_and_close(req->clntFd, response, respLen);
                return;
            }
        }
        if (!EMPTY(req->query))
        {
            char *remain;
            while (req->query) {
                char *value = split(&req->query, "&");
                if (!value || !*value) continue;
                unescape_uri(value);
                char *key = split(&value, "=");
                if (!key || !*key || !value || !*value) continue;
                if (EQUALS(key, "font"))
                    strcpy(osds[id].font, !EMPTY(value) ? value : DEF_FONT);
                else if (EQUALS(key, "text"))
                    strcpy(osds[id].text, value);
                else if (EQUALS(key, "size")) {
                    double result = strtod(value, &remain);
                    if (remain == value) continue;
                    osds[id].size = (result != 0 ? result : DEF_SIZE);
                }
                else if (EQUALS(key, "color")) {
                    int result = color_parse(value);
                    osds[id].color = result;
                }
                else if (EQUALS(key, "opal")) {
                    short result = strtol(value, &remain, 10);
                    if (remain != value)
                        osds[id].opal = result & 0xFF;
                }
                else if (EQUALS(key, "posx")) {
                    short result = strtol(value, &remain, 10);
                    if (remain != value)
                        osds[id].posx = result;
                }
                else if (EQUALS(key, "posy")) {
                    short result = strtol(value, &remain, 10);
                    if (remain != value)
                        osds[id].posy = result;
                }
                else if (EQUALS(key, "pos")) {
                    int x, y;
                    if (sscanf(value, "%d,%d", &x, &y) == 2) {
                        osds[id].posx = x;
                        osds[id].posy = y;
                    }
                }
                else if (EQUALS(key, "save") && 
                    (EQUALS_CASE(value, "true") || EQUALS(value, "1"))) {
                    saved = save_app_config();
                    if (!saved)
                        HAL_INFO("server", "Configuration saved!\n");
                    else
                        HAL_WARNING("server", "Failed to save configuration!\n");
                    break;
                }
            }
            osds[id].updt = 1;
        }
        int color = (((osds[id].color >> 10) & 0x1F) * 255 / 31) << 16 |
                    (((osds[id].color >> 5) & 0x1F) * 255 / 31) << 8 |
                    ((osds[id].color & 0x1F) * 255 / 31);
        respLen = sprintf(response,
            "HTTP/1.1 200 OK\r\n"
            "Content-Type: application/json;charset=UTF-8\r\n"
            "Connection: close\r\n"
            "\r\n"
            "{\"id\":%d,\"color\":\"%#x\",\"opal\":%d,\"pos\":[%d,%d],"
            "\"font\":\"%s\",\"size\":%.1f,\"text\":\"%s\",\"saved\":%s}",
            id, color, osds[id].opal, osds[id].posx, osds[id].posy,
            osds[id].font, osds[id].size, osds[id].text, saved == 0 ? "true" : "false");
        send_and_close(req->clntFd, response, respLen);
        return;
    }

    if (EQUALS(req->uri, "/api/status")) {
        struct sysinfo si;
        sysinfo(&si);
        char memory[16], uptime[48];
        short free = (si.freeram + si.bufferram) / 1024 / 1024;
        short total = si.totalram / 1024 / 1024;
        sprintf(memory, "%d/%dMB", total - free, total);
        if (si.uptime > 86400)
            sprintf(uptime, "%ld days, %ld:%02ld:%02ld", si.uptime / 86400, (si.uptime % 86400) / 3600, (si.uptime % 3600) / 60, si.uptime % 60);
        else if (si.uptime > 3600)
            sprintf(uptime, "%ld:%02ld:%02ld", si.uptime / 3600, (si.uptime % 3600) / 60, si.uptime % 60);
        else
            sprintf(uptime, "%ld:%02ld", si.uptime / 60, si.uptime % 60);
        int respLen = sprintf(response,
            "HTTP/1.1 200 OK\r\n"
            "Content-Type: application/json;charset=UTF-8\r\n"
            "Connection: close\r\n"
            "\r\n"
            "{\"chip\":\"%s\",\"loadavg\":[%.2f,%.2f,%.2f],\"memory\":\"%s\",\"uptime\":\"%s\"}",
            chip, si.loads[0] / 65536.0, si.loads[1] / 65536.0, si.loads[2] / 65536.0, memory, uptime);
        send_and_close(req->clntFd, response, respLen);
        return;
    }

    if (EQUALS(req->uri, "/api/time")) {
        struct timespec t;
        if (!EMPTY(req->query)) {
            char *remain;
            while (req->query) {
                char *value = split(&req->query, "&");
                if (!value || !*value) continue;
                unescape_uri(value);
                char *key = split(&value, "=");
                if (!key || !*key || !value || !*value) continue;
                if (EQUALS(key, "fmt")) {
                    strncpy(timefmt, value, 32);
                } else if (EQUALS(key, "ts")) {
                    short result = strtol(value, &remain, 10);
                    if (remain == value) continue;
                    t.tv_sec = result;
                    clock_settime(0, &t);
                }
            }
        }
        clock_gettime(0, &t);
        int respLen = sprintf(response,
            "HTTP/1.1 200 OK\r\n"
            "Content-Type: application/json;charset=UTF-8\r\n"
            "Connection: close\r\n"
            "\r\n"
            "{\"fmt\":\"%s\",\"ts\":%zu}", timefmt, t.tv_sec);
        send_and_close(req->clntFd, response, respLen);
        return;
    }

    if (app_config.web_enable_static && send_file(req->clntFd, req->uri))
        return;

    send_and_close(req->clntFd, (char*)error400, strlen(error400));
}

void *server_thread(void *vargp) {
    struct Request req = {0};
    int ret, server_fd = *((int *)vargp);
    int enable = 1;
    if (setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR, &enable, sizeof(int)) < 0) {
        HAL_WARNING("server", "setsockopt(SO_REUSEADDR) failed");
        fflush(stdout);
    }
    struct sockaddr_in client, server = {
        .sin_family = AF_INET,
        .sin_port = htons(app_config.web_port),
        .sin_addr.s_addr = htonl(INADDR_ANY)
    };
    if (ret = bind(server_fd, (struct sockaddr *)&server, sizeof(server))) {
        HAL_DANGER("server", "%s (%d)\n", strerror(errno), errno);
        keepRunning = 0;
        close_socket_fd(server_fd);
        return NULL;
    }
    listen(server_fd, 128);

    req.input = malloc(REQSIZE);

    while (keepRunning) {
        // Waiting for a new connection
        if ((req.clntFd = accept(server_fd, NULL, NULL)) == -1)
            break;

        parse_request(&req);

        respond_request(&req);
    }

    if (req.input)
        free(req.input);

    close_socket_fd(server_fd);
    HAL_INFO("server", "Thread has exited\n");
    return NULL;
}

int start_server() {
    for (uint32_t i = 0; i < MAX_CLIENTS; ++i) {
        client_fds[i].socket_fd = -1;
    }
    pthread_mutex_init(&client_fds_mutex, NULL);

    // Start the server and HTTP video streams thread
    server_fd = socket(AF_INET, SOCK_STREAM, 0);

    {
        pthread_attr_t thread_attr;
        pthread_attr_init(&thread_attr);
        size_t stacksize;
        pthread_attr_getstacksize(&thread_attr, &stacksize);
        size_t new_stacksize = app_config.web_server_thread_stack_size + REQSIZE;
        if (pthread_attr_setstacksize(&thread_attr, new_stacksize))
            HAL_WARNING("server", "Can't set stack size %zu\n", new_stacksize);
        if (pthread_create(
            &server_thread_id, &thread_attr, server_thread, (void *)&server_fd))
            HAL_ERROR("server", "Starting the server thread failed!\n");
        if (pthread_attr_setstacksize(&thread_attr, stacksize))
            HAL_DANGER("server", "Can't set stack size %zu\n", stacksize);
        pthread_attr_destroy(&thread_attr);
    }

    return EXIT_SUCCESS;
}

int stop_server() {
    keepRunning = 0;

    // Stop server_thread when server_fd is closed
    close_socket_fd(server_fd);
    pthread_join(server_thread_id, NULL);

    pthread_mutex_destroy(&client_fds_mutex);
    HAL_INFO("server", "Shutting down server...\n");

    return EXIT_SUCCESS;
}