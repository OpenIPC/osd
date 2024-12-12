#include "error.h"

#include <stdio.h>

char *errstr(int error) {
    int level;
    int module = (error >> 16) & 0xFF;

    switch (plat) {
        case HAL_PLATFORM_I6:
            level = (error >> 12) & 0xF;
            error = error & 0xFF000FFF | (level > 0 ? (4 << 13) : 0);
            switch (module) {
                case I6_SYS_MOD_SYS:
                    error |= (V4_SYS_MOD_SYS << 16); break;
                case I6_SYS_MOD_RGN:
                    error |= (V4_SYS_MOD_RGN << 16); break;
                default:
                    error |= (module << 16); break;
            }
            break;
        case HAL_PLATFORM_I6C:
            level = (error >> 12) & 0xF;
            error = error & 0xFF000FFF | (level > 0 ? (4 << 13) : 0);
            switch (module) {
                case I6C_SYS_MOD_SYS:
                    error |= (V4_SYS_MOD_SYS << 16); break;
                case I6C_SYS_MOD_RGN:
                    error |= (V4_SYS_MOD_RGN << 16); break;
                default:
                    error |= (module << 16); break;
            }
            break;
        case HAL_PLATFORM_M6:
            level = (error >> 12) & 0xF;
            error = error & 0xFF000FFF | (level > 0 ? (4 << 13) : 0);
            switch (module) {
                case M6_SYS_MOD_SYS:
                    error |= (V4_SYS_MOD_SYS << 16); break;
                case M6_SYS_MOD_RGN:
                    error |= (V4_SYS_MOD_RGN << 16); break;
                default:
                    error |= (module << 16); break;
            }
            break;
    }

    switch (error) {
        case 0xA0028003:
            return "ERR_SYS_ILLEGAL_PARAM: The parameter "
                "configuration is invalid.";
        case 0xA0028006:
            return "ERR_SYS_NULL_PTR: The pointer is null.";
        case 0xA0028009:
            return "ERR_SYS_NOT_PERM: The operation is forbidden.";
        case 0xA0028010:
            return "ERR_SYS_NOTREADY: The system control attributes "
                "are not configured.";
        case 0xA0028012:
            return "ERR_SYS_BUSY: The system is busy.";
        case 0xA002800C:
            return "ERR_SYS_NOMEM: The memory fails to be allocated "
                "due to some causes such as insufficient system memory.";

        case 0xA0038001:
            return "ERR_RGN_INVALID_DEVID: The device ID exceeds the "
                "valid range.";
        case 0xA0038002:
            return "ERR_RGN_INVALID_CHNID: The channel ID is "
                "incorrect or the region handle is invalid.";
        case 0xA0038003:
            return "ERR_RGN_ILLEGAL_PARAM: The parameter value "
                "exceeds its valid range.";
        case 0xA0038004:
            return "ERR_RGN_EXIST: The device, channel, or resource "
                "to be created already exists.";
        case 0xA0038005:
            return "ERR_RGN_UNEXIST: The device, channel, or resource "
                "to be used or destroyed does not exist.";
        case 0xA0038006:
            return "ERR_RGN_NULL_PTR: The pointer is null.";
        case 0xA0038007:
            return "ERR_RGN_NOT_CONFIG: The module is not configured.";
        case 0xA0038008:
            return "ERR_RGN_NOT_SUPPORT: The parameter or function is "
                "not supported.";
        case 0xA0038009:
            return "ERR_RGN_NOT_PERM: The operation, for example, "
                "attempting to modify the value of a static parameter, is "
                "forbidden.";
        case 0xA003800C:
            return "ERR_RGN_NOMEM: The memory fails to be allocated "
                "due to some causes such as insufficient system memory.";
        case 0xA003800D:
            return "ERR_RGN_NOBUF: The buffer fails to be allocated "
                "due to some causes such as oversize of the data buffer applied "
                "for.";
        case 0xA003800E:
            return "ERR_RGN_BUF_EMPTY: The buffer is empty.";
        case 0xA003800F:
            return "ERR_RGN_BUF_FULL: The buffer is full.";
        case 0xA0038010:
            return "ERR_RGN_NOTREADY: The system is not initialized "
                "or the corresponding module is not loaded.";
        case 0xA0038011:
            return "ERR_RGN_BADADDR: The address is invalid.";
        case 0xA0038012:
            return "ERR_RGN_BUSY: The system is busy.";

        default:
            return "Unknown error code.";
    }
}
