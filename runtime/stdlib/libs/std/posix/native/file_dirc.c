/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2025. All rights reserved.
 * This source file is part of the Codira project, licensed under Apache-2.0
 * with Runtime Library Exception.
 *
 * See https://cangjie-lang.cn/pages/LICENSE for license information.
 */

#include <dirent.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <string.h>
#include <stdbool.h>
#include <errno.h>
#include "securec.h"
#include "file_dirc.h"

#if defined(_WIN32) && defined(__MINGW64__)
#define WIN_PIPE_SIZE 1024
#define lstat stat
#include <windows.h>
#include <psapi.h>
#else
#include <arpa/inet.h>
#include <pwd.h>
#include <sys/wait.h>
#endif

static bool IsType(uint32_t mode, uint32_t mask)
{
    return ((mode) & S_IFMT) == mask;
}

#if defined(__linux__) || defined(__APPLE__)
extern bool CODE_OS_IsType(const char* path, uint32_t mode)
{
    struct stat buf;
    int result = lstat(path, &buf);
    if (result < 0) {
        return 0;
    }
    return IsType((uint32_t)buf.st_mode, mode);
}
#endif

enum TypeFunc {
    REG_TYPE = 1,
    DIR_TYPE,
    CHR_TYPE,
    BLK_TYPE,
    FIFO_TYPE,
#if defined(__linux__) || defined(__ohos__) || defined(__APPLE__)
    LNK_TYPE,
    SOCK_TYPE,
#endif
};

extern bool CODE_OS_IsTypeFunc(const char* fileName, enum TypeFunc typeNum)
{
    struct stat buf;
    int result = lstat(fileName, &buf);
    if (result < 0) {
        return 0;
    }
    switch (typeNum) {
        case REG_TYPE:
            return S_ISREG(buf.st_mode);
        case DIR_TYPE:
            return S_ISDIR(buf.st_mode);
        case CHR_TYPE:
            return S_ISCHR(buf.st_mode);
        case BLK_TYPE:
            return S_ISBLK(buf.st_mode);
        case FIFO_TYPE:
            return S_ISFIFO(buf.st_mode);
#if defined(__linux__) || defined(__ohos__) || defined(__APPLE__)
        case LNK_TYPE:
            return S_ISLNK(buf.st_mode);
        case SOCK_TYPE:
            return S_ISSOCK(buf.st_mode);
#endif
        default:
            return 0;
    }
}

extern char* CODE_OS_GetCwd(void)
{
    return getcwd(NULL, 0);
}
