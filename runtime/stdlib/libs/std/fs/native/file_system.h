/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2025. All rights reserved.
 * This source file is part of the Codira project, licensed under Apache-2.0
 * with Runtime Library Exception.
 *
 * See https://cangjie-lang.cn/pages/LICENSE for license information.
 */

#ifndef CODIRA_FS_FILE_SYSTEM_H
#define CODIRA_FS_FILE_SYSTEM_H

#ifndef _MSC_BUILD
#include "securec.h"
#else
#define EOK 0
#endif

#ifndef _LARGEFILE_SOURCE
#define _LARGEFILE_SOURCE
#endif

#ifndef _LARGEFILE64_SOURCE
#define _LARGEFILE64_SOURCE
#endif

#ifndef _FILE_OFFSET_BITS
#define _FILE_OFFSET_BITS 64
#endif

#define __USE_XOPEN_EXTENDED 500

#define ERRNO_EINVAL (-2)
#define ERRNO_EPERM (-2)
#define ERRNO_ESTAT (-3)
#define DEFFILEMODE (S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP | S_IROTH | S_IWOTH)                                // 0666
#define DEF_DIR_MODE (S_IRUSR | S_IWUSR | S_IXUSR | S_IRGRP | S_IWGRP | S_IXGRP | S_IROTH | S_IWOTH | S_IXOTH) // 0777
#define BUF_SIZE (1024)

#define CODE_RDONLY 0
#define CODE_WRONLY 1
#define CODE_APPEND 2
#define CODE_RDWT 3

#include <assert.h>
#include <dirent.h>
#include <errno.h>
#include <fcntl.h>
#include <ftw.h>
#include <libgen.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <unistd.h>

#if defined(_WIN32) && defined(__MINGW64__)
#include <fileapi.h>
#include <io.h>
#include <tchar.h>
#include <windows.h>
#define SLASH (char)'\\'
#define MAX_PATH_LEN 260
#else // UNIX
#if defined(__APPLE__)
#define MAX_PATH_LEN 1024
#else
#define MAX_PATH_LEN 4096
#endif
#define SLASH (char)'/'
#endif // defined(_WIN32) && defined(__MINGW64__)

typedef struct {
    int64_t rtnCode;
    char* msg;
} FsError;

typedef struct {
#if defined(_WIN32) && defined(__MINGW64__)
    HANDLE fd;
#else
    intptr_t fd;
#endif
    char* msg;
} FsInfo;

static int8_t Chmod(const char* cPath, mode_t mask)
{
#if defined(_WIN32) && defined(__MINGW64__)
    int res = _chmod(cPath, mask);
#else
    int res = chmod(cPath, mask);
#endif
    if (res == -1 && errno == EPERM) {
        return ERRNO_EPERM;
    }
    if (res == 0) {
        return 1;
    }
    return 0;
}

extern FsError* GetDefaultResult(void)
{
    FsError* result = (FsError*)malloc(sizeof(FsError));
    if (result == NULL) {
        return NULL;
    }
    result->rtnCode = 0;
    result->msg = NULL;
    return result;
}

extern FsInfo* GetDefaultFsResult(void)
{
    FsInfo* result = (FsInfo*)malloc(sizeof(FsInfo));
    if (result == NULL) {
        return NULL;
    }
#if defined(_WIN32) && defined(__MINGW64__)
    result->fd = NULL;
#else
    result->fd = -1;
#endif
    result->msg = NULL;
    return result;
}

extern int CODE_FS_ErrnoGet(void)
{
    return errno;
}

#endif // CODIRA_FS_FILE_SYSTEM_H
