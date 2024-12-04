#include "support.h"

char chnCount = 0;

char chip[16] = "unknown";
char family[32] = {0};
hal_platform plat = HAL_PLATFORM_UNK;
int series = 0;

bool hal_registry(unsigned int addr, unsigned int *data, hal_register_op op) {
    static int mem_fd;
    static char *loaded_area;
    static unsigned int loaded_offset;
    static unsigned int loaded_size;

    unsigned int offset = addr & 0xffff0000;
    unsigned int size = 0xffff;
    if (!addr || (loaded_area && offset != loaded_offset))
        if (munmap(loaded_area, loaded_size))
            fprintf(stderr, "hal_registry munmap error: %s (%d)\n",
                strerror(errno), errno);

    if (!addr) {
        close(mem_fd);
        return true;
    }

    if (!mem_fd && (mem_fd = open("/dev/mem", O_RDWR | O_SYNC)) < 0) {
        fprintf(stderr, "can't open /dev/mem\n");
        return false;
    }

    volatile char *mapped_area;
    if (offset != loaded_offset) {
        mapped_area = mmap64(NULL, size, PROT_READ | PROT_WRITE, MAP_SHARED, mem_fd, offset);
        if (mapped_area == MAP_FAILED) {
            fprintf(stderr, "hal_registry mmap error: %s (%d)\n",
                    strerror(errno), errno);
            return false;
        }
        loaded_area = (char *)mapped_area;
        loaded_size = size;
        loaded_offset = offset;
    } else
        mapped_area = loaded_area;

    if (op & OP_READ)
        *data = *(volatile unsigned int *)(mapped_area + (addr - offset));
    if (op & OP_WRITE)
        *(volatile unsigned int *)(mapped_area + (addr - offset)) = *data;

    return true;
}

void hal_identify(void) {
    unsigned int val = 0;
    FILE *file;
    char *endMark;
    char line[200] = {0};

    if (!access("/proc/mi_modules", F_OK) && 
        hal_registry(0x1F003C00, &series, OP_READ)) {
        char package[4] = {0};
        short memory = 0;

        plat = HAL_PLATFORM_I6;
        chnCount = I6_VENC_CHN_NUM;

        if (file = fopen("/proc/cmdline", "r")) {
            fgets(line, 200, file);
            char *remain, *capacity = strstr(line, "LX_MEM=");
            memory = (short)(strtol(capacity + 7, &remain, 16) >> 20);
            fclose(file);
        }

        if (file = fopen("/sys/devices/soc0/machine", "r")) {
            fgets(line, 200, file);
            char *board = strstr(line, "SSC");
            strncpy(package, board + 4, 3);
            fclose(file);
        }

        switch (series) {
            case 0xEF:
                if (memory > 128)
                    strcpy(chip, "SSC327Q");
                else if (memory > 64) 
                    strcpy(chip, "SSC32[5/7]D");
                else 
                    strcpy(chip, "SSC32[3/5/7]");
                if (package[2] == 'B' && chip[strlen(chip) - 1] != 'Q')
                    strcat(chip, "E");
                strcpy(family, "infinity6");
                break;
            case 0xF1:
                if (package[2] == 'A')
                    strcpy(chip, "SSC33[8/9]G");
                else {
                    if (sysconf(_SC_NPROCESSORS_CONF) == 1)
                        strcpy(chip, "SSC30K");
                    else
                        strcpy(chip, "SSC33[6/8]");
                    if (memory > 128)
                        strcat(chip, "Q");
                    else if (memory > 64)
                        strcat(chip, "D");
                }
                strcpy(family, "infinity6e");
                break;
            case 0xF2: 
                strcpy(chip, "SSC33[3/5/7]");
                if (memory > 64)
                    strcat(chip, "D");
                if (package[2] == 'B')
                    strcat(chip, "E");
                strcpy(family, "infinity6b0");
                break;
            case 0xF9:
                plat = HAL_PLATFORM_I6C;
                strcpy(chip, "SSC37[7/8]");
                if (memory > 128)
                    strcat(chip, "Q");
                else if (memory > 64)
                    strcat(chip, "D");
                if (package[2] == 'D')
                    strcat(chip, "E");
                strcpy(family, "infinity6c");
                chnCount = I6C_VENC_CHN_NUM;
                break;
            case 0xFB:
                plat = HAL_PLATFORM_I6F;
                strcpy(chip, "SSC379G");
                strcpy(family, "infinity6f");
                chnCount = I6F_VENC_CHN_NUM;
                break;
            default:
                plat = HAL_PLATFORM_UNK;
                break;
        }
    }

    if (file = fopen("/proc/iomem", "r")) {
        while (fgets(line, 200, file))
            if (strstr(line, "uart")) {
                val = strtol(line, &endMark, 16);
                break;
            }
        fclose(file);
    }

    switch (val) {
        case 0x12040000:
        case 0x120a0000: val = 0x12020000; break;
        case 0x12100000:
            val = 0x12020000;
            series = 3;
            break;
        case 0x20080000:
            val = 0x20050000;
            series = 2;
            break;

        default: return;
    }

    unsigned SCSYSID[4] = {0};
    unsigned int out = 0;
    for (int i = 0; i < 4; i++) {
        if (!hal_registry(val + 0xEE0 + (i * 4), (unsigned*)&SCSYSID[i], OP_READ)) break;
        if (!i && (SCSYSID[i] >> 16 & 0xFF)) { out = SCSYSID[i]; break; }
        out |= (SCSYSID[i] & 0xFF) << i * 8;
    }

    if (out == 0x35180100) {
        series = 1;
        if (hal_registry(0x2005008C, &val, OP_READ))
            switch (val >> 8 & 0x7f) {
                case 0x10: out = 0x3518C100; break;
                case 0x57: out = 0x3518E100; break;
                default:
                    val = 3;
                    if (hal_registry(0x20050088, &val, OP_WRITE) &&
                        hal_registry(0x20050088, &val, OP_READ))
                        switch (val) {
                            case 1: out = 0x3516C100; break;
                            case 2: out = 0x3518E100; break;
                            case 3: out = 0x3518A100; break;
                        }
            }
    }

    sprintf(chip, "%s%X", 
        ((out >> 28) == 0x7) ? "GK" : "Hi", out);
    if (chip[6] == '0') {
        chip[6] = 'V';
    } else {
        chip[11] = '\0';
        chip[10] = chip[9];
        chip[9] = chip[8];
        chip[8] = chip[7];
        chip[7] = 'V';
    }

    if (series == 1) {
        plat = HAL_PLATFORM_V1;
        strcpy(family, "hisi-gen1");
        chnCount = V1_VENC_CHN_NUM;
        return;    
    } else if (series == 2) {
        plat = HAL_PLATFORM_V2;
        strcpy(family, "hisi-gen2");
        chnCount = V2_VENC_CHN_NUM;
        return;    
    } else if (series == 3) {
        plat = HAL_PLATFORM_V3;
        strcpy(family, "hisi-gen3");
        chnCount = V3_VENC_CHN_NUM;
        return;
    }

    series = 4;
    plat = HAL_PLATFORM_V4;
    strcpy(family, "hisi-gen4");
    chnCount = V4_VENC_CHN_NUM;
}