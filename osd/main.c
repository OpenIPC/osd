#include "common.h"

#include "bitmap.h"
#include "net.h"
#include "region.h"
#include "text.h"

void *io_map;
struct osd *osds;

static void fill(char* str)
{
    unsigned int rxb_l, txb_l, cpu_l[6];
    char out[80] = "";
    char param = 0;
    int i = 0, j = 0;

    while(str[i] != 0)
    {
        if (str[i] != '$')
        {
            strncat(out, str + i, 1);
            j++;
        }
        else if (str[i + 1] == 'B')
        {
            i++;
            struct ifaddrs *ifaddr, *ifa;
            if (getifaddrs(&ifaddr) == -1) continue;

            for (ifa = ifaddr; ifa != NULL; ifa = ifa->ifa_next)
            { 
                if (!strcmp(ifa->ifa_name, "lo")) continue;
                if (!ifa->ifa_addr || ifa->ifa_addr->sa_family != AF_PACKET) continue;
                if (!ifa->ifa_data) continue;

                struct rtnl_link_stats *stats = ifa->ifa_data;
                char b[32];
                sprintf(b, "R:%dKbps S:%dKbps", 
                    (stats->rx_bytes - rxb_l) / 1024, (stats->tx_bytes - txb_l) / 1024);
                strcat(out, b);
                j += strlen(b);
                rxb_l = stats->rx_bytes;
                txb_l = stats->tx_bytes;
                break;
            }
            
            freeifaddrs(ifaddr);
        }
        else if (str[i + 1] == 'C')
        {
            i++;
            char tmp[6];
            unsigned int cpu[6];
            FILE *stat = fopen("/proc/stat", "r");
            fscanf(stat, "%s %u %u %u %u %u %u",
                tmp, &cpu[0], &cpu[1], &cpu[2], &cpu[3], &cpu[4], &cpu[5]);
            fclose(stat);

            char c[5];
            char avg = 100 - (cpu[3] - cpu_l[3]) / sysconf(_SC_NPROCESSORS_ONLN);
            sprintf(c, "%d%%", avg);
            strcat(out, c);
            j += strlen(c);
            for (int i = 0; i < sizeof(cpu) / sizeof(cpu[0]); i++)
                cpu_l[i] = cpu[i];
        }
        else if (str[i + 1] == 'M')
        {
            i++;
            struct sysinfo si;
            sysinfo(&si);

            char m[16];
            short used = (si.freeram + si.bufferram) / 1024 / 1024;
            short total = si.totalram / 1024 / 1024;
            sprintf(m, "%d/%dMB", used, total);
            strcat(out, m);
            j += strlen(m);
        }
        else if (str[i + 1] == 'T')
        {
            i++;
#ifdef __SIGMASTAR__
            unsigned short temp = 1370 * (400 - ((unsigned short*)io_map)[0x2918 >> 1]) / 1000.0f + 27;
#elif defined(__16CV300__)
            unsigned short temp = (((unsigned short*)io_map)[0x300A4 >> 1] - 125) / 806.0f * 165.0f - 40;
#else
            unsigned short temp = (((unsigned short*)io_map)[0x280BC >> 1] - 117) / 798.0f * 165.0f - 40;
#endif
            char t[8];
            sprintf(t, "%d", temp);
            strcat(out, t);
            j += strlen(t);
        }
        else if (str[i + 1] == '$') {
            i++;
            strcat(out, "$");
            j++;
        }
        i++; 
    }
    strncpy(str, out, 80);
}

void overlays()
{
    while(1)
    {
        for (int id = 0; id < MAX_OSD; id++)
        {
            if ((!empty(osds[id].text) && !osds[id].updt) || (empty(osds[id].text) && osds[id].updt)) {
                char out[80];
                strcpy(out, osds[id].text);
                if (strstr(out, "$"))
                {
                    fill(out);
                    osds[id].updt = 1;
                }

                RECT rect = measure_text(osds[id].font, osds[id].size, out);
                create_region(0, id, osds[id].posx, osds[id].posy, rect.width, rect.height);

                BITMAP bitmap = raster_text(osds[id].font, osds[id].size, out);
#ifdef __SIGMASTAR__
                int s32Ret = MI_RGN_SetBitMap(id, (MI_RGN_Bitmap_t *)(&bitmap));
#else
                int s32Ret = HI_MPI_RGN_SetBitMap(id, (BITMAP_S *)(&bitmap));
#endif
                if (s32Ret)
                    fprintf(stderr, "[%s:%d]RGN_SetBitMap failed with %#x %d!\n", __func__, __LINE__, s32Ret, id);
                free(bitmap.pData);
            }

            osds[id].updt = 0;
        }
        sleep(1);
    }
}

void route()
{
    if (starts_with(uri, "/api/osd/") &&
        uri[9] >= '0' && uri[9] <= (MAX_OSD - 1 + '0'))
    {
        char id = uri[9] - '0';
        if (!empty(query))
        {
            char *remain;
            for (char *item; (item = extract_pair(&query));) {
                char *value = item;
                char *key = extract_key(&value);
                if (!strcmp(key, "font"))
                    strcpy(osds[id].font, !empty(value) ? value : DEF_FONT);
                else if (!strcmp(key, "text"))
                    strcpy(osds[id].text, value);
                else if (!strcmp(key, "size")) {
                    double result = strtod(value, &remain);
                    if (remain == value) continue;
                    osds[id].size = (result != 0 ? result : DEF_SIZE);
                }
                else if (!strcmp(key, "posx")) {
                    short result = strtol(value, &remain, 10);
                    if (remain != value)
                        osds[id].posx = result;
                }
                else if (!strcmp(key, "posy")) {
                    short result = strtol(value, &remain, 10);
                    if (remain != value)
                        osds[id].posy = result;
                }
            }
            osds[id].updt = 1;
        }
        printf(\
            "HTTP/1.1 200 OK\r\n" \
            "Content-Type: application/json;charset=UTF-8\r\n" \
            "Connection: close\r\n" \
            "\r\n" \
        );
        printf("{\"id\":%d,\"pos\":[%d,%d],\"font\":\"%s\",\"size\":%.1f,\"text\":\"%s\"}", 
            id, osds[id].posx, osds[id].posy, osds[id].font, osds[id].size, osds[id].text);
        return;
    }
    else if (!strcmp(uri, "/api/time"))
    {
        struct timespec t;
        if (!empty(query))
        {
            char *remain;
            char *input = strdup(query);
            for (char *item; (item = extract_pair(&input));) {
                char *value = item;
                char *key = extract_key(&value);
                if (!strcmp(key, "ts")) {
                    long result = strtol(value, &remain, 10);
                    if (remain == value) continue;
                    t.tv_sec = result;
                    clock_settime(CLOCK_REALTIME, &t);
                }
            }
            free(input);
        }
        clock_gettime(CLOCK_REALTIME, &t);
        printf(\
            "HTTP/1.1 200 OK\r\n" \
            "Content-Type: application/json;charset=UTF-8\r\n" \
            "Connection: close\r\n" \
            "\r\n" \
        );
        printf("{\"ts\":%d}", t.tv_sec);
        return;
    }

    printf(\
        "HTTP/1.1 500 Not Handled\r\n" \
        "Content-Type: text/plain\r\n" \
        "Connection: close\r\n" \
        "\r\n" \
        "The server has no handler to the request.\r\n" \
    );
}

int main(int argc, char *argv[])
{
    int fd_mem = open("/dev/mem", O_RDWR);
    io_map = mmap(NULL, IO_SIZE, PROT_READ | PROT_WRITE, MAP_SHARED, fd_mem, IO_BASE);

#ifdef __SIGMASTAR__
    static MI_RGN_PaletteTable_t g_stPaletteTable = {{{0, 0, 0, 0}}};
    int s32Ret = MI_RGN_Init(&g_stPaletteTable);
    if (s32Ret)
        fprintf(stderr, "[%s:%d]RGN_Init failed with %#x!\n", __func__, __LINE__, s32Ret);
#endif

    osds = mmap(NULL, sizeof(*osds) * MAX_OSD,
        PROT_READ | PROT_WRITE, MAP_ANONYMOUS | MAP_SHARED, -1, 0);
    for (int id = 0; id < MAX_OSD; id++)
    {
        osds[id].size = DEF_SIZE;
        osds[id].posx = DEF_POSX;
        osds[id].posy = DEF_POSY;
        osds[id].updt = 0;
        strcpy(osds[id].font, DEF_FONT);
        osds[id].text[0] = '\0';
    }

    if (!fork())
        overlays();

    serve_forever();

    munmap(osds, sizeof(*osds) * MAX_OSD);

#ifdef __SIGMASTAR__
  s32Ret = MI_RGN_DeInit();
  if (s32Ret)
    printf("[%s:%d]RGN_DeInit failed with %#x!\n", __func__, __LINE__, s32Ret);
#endif

    munmap(io_map, IO_SIZE);
    close(fd_mem);

    return 0;
}