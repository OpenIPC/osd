#include "net.h"

static int listenfd;
static char *buf;
int *clients;

static void start_server();
static void respond(int);

typedef struct
{
    char *name, *value;
} header_t;
static header_t reqhdr[17] = {{"\0", "\0"}};

char *method,
    *uri,
    *query,
    *prot,
    *payload;

int payload_size;

void serve_forever()
{
    struct sockaddr_in clientaddr;
    socklen_t addrlen;

    int slot = 0;

    printf(
        "Server started %shttp://127.0.0.1:%s%s\n",
        "\033[92m", PORT, "\033[0m");

    clients = mmap(NULL, sizeof(*clients) * MAX_CONN,
                   PROT_READ | PROT_WRITE, MAP_ANONYMOUS | MAP_SHARED, -1, 0);
    for (int i = 0; i < MAX_CONN; i++)
        clients[i] = -1;

    start_server(PORT);

    signal(SIGCHLD, SIG_IGN);

    while (1)
    {
        addrlen = sizeof(clientaddr);
        if ((clients[slot] = accept(listenfd, (struct sockaddr *)&clientaddr, &addrlen)) < 0)
            fatal("accept() error");
        else if (!fork())
        {
            close(listenfd);
            respond(slot);
            close(clients[slot]);
            clients[slot] = -1;
            exit(0);
        }
        else
            close(clients[slot]);

        while (clients[slot] != -1)
            slot = (slot + 1) % MAX_CONN;
    }
}

char *request_header(const char *name)
{
    header_t *h = reqhdr;
    while (h->name)
    {
        if (!strcmp(h->name, name))
            return h->value;
        h++;
    }
    return NULL;
}

header_t *request_headers(void) { return reqhdr; }

static void unescape_uri(char *uri)
{
    char chr = 0;
    char *src = uri;
    char *dst = uri;

    while (*src && !isspace((int)(*src)) && (*src != '%'))
        src++;

    dst = src;
    while (*src && !isspace((int)(*src)))
    {
        if (*src == '+')
            chr = ' ';
        else if ((*src == '%') && src[1] && src[2])
        {
            src++;
            chr = ((*src & 0x0F) + 9 * (*src > '9')) * 16;
            src++;
            chr += ((*src & 0x0F) + 9 * (*src > '9'));
        }
        else
            chr = *src;
        *dst++ = chr;
        src++;
    }
    *dst = '\0';
}

char *extract_key(char **pair)
{
    char *key = strsep(pair, "=");
    unescape_uri(key);
    unescape_uri(*pair);
    return key;
}

char *extract_pair(char **input)
{
    char *curr;
    while (*input && (*input)[0] && !(curr = strsep(input, "&"))[0]);
    return curr;
}

void start_server()
{
    struct addrinfo hints, *res, *p;
    memset(&hints, 0, sizeof(hints));
    hints.ai_family = AF_INET;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_flags = AI_PASSIVE;

    if (getaddrinfo(NULL, PORT, &hints, &res))
        fatal("getaddrinfo() error");

    for (p = res; p != NULL; p = p->ai_next)
    {
        int option = 1;
        listenfd = socket(p->ai_family, p->ai_socktype, 0);
        setsockopt(listenfd, SOL_SOCKET, SO_REUSEADDR, &option, sizeof(option));
        if (listenfd == -1)
            continue;
        if (!bind(listenfd, p->ai_addr, p->ai_addrlen))
            break;
    }

    if (!p)
        fatal("socket() or bind() error");

    freeaddrinfo(res);

    if (listen(listenfd, QUEUE_SIZE))
        fatal("listen() error");
}

void respond(int slot)
{
    int rcvd;
    int total = 0;

    buf = malloc(BUF_SIZE);

    rcvd = recv(clients[slot], buf + total, BUF_SIZE - total, 0);

    if (rcvd < 0)
        goto finish;
    else if (rcvd == 0)
        fputs("Client disconnected unexpectedly.\n", stderr);

    total += rcvd;

    if (total > 0)
    {
        method = strtok(buf, " \t\r\n");
        uri = strtok(NULL, " \t");
        prot = strtok(NULL, " \t\r\n");

        fprintf(stderr, "\x1b[32m + [%s] %s\x1b[0m\n", method, uri);

        if (query = strchr(uri, '?'))
            *query++ = '\0';
        else
            query = uri - 1;

        header_t *h = reqhdr;
        char *t, *t2;
        while (h < reqhdr + 16)
        {
            char *k, *v, *e;
            k = strtok(NULL, "\r\n: \t");
            if (!k)
                break;
            v = strtok(NULL, "\r\n");
            while (*v && *v == ' ')
                v++;
            h->name = k;
            h->value = v;
            h++;
            fprintf(stderr, "[H] %s: %s\n", k, v);
            e = v + 1 + strlen(v);
            if (e[1] == '\r' && e[2] == '\n')
                break;
        }

        t = strtok(NULL, "\r\n");
        payload = t;
        t2 = request_header("Content-Length");
        payload_size = t2 ? atol(t2) : (total - (t - buf));

        while (t2 && total < payload_size)
        {
            rcvd = recv(clients[slot], buf + total, BUF_SIZE - total, 0);

            if (rcvd < 0)
            {
                fputs("recv() error\n", stderr);
                break;
            }
            else if (rcvd == 0)
            {
                fputs("Client disconnected unexpectedly.\n", stderr);
                break;
            }

            total += rcvd;
        }

        if (!t)
        {
            t = strtok(NULL, "\r\n");
            payload = t;
        }

finish:
        int clientfd = clients[slot];
        dup2(clientfd, STDOUT_FILENO);
        close(clientfd);

        route();

        fflush(stdout);
        shutdown(STDOUT_FILENO, SHUT_WR);
        close(STDOUT_FILENO);
    }

    free(buf);
}