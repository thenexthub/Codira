/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2025. All rights reserved.
 * This source file is part of the Codira project, licensed under Apache-2.0
 * with Runtime Library Exception.
 *
 * See https://cangjie-lang.cn/pages/LICENSE for license information.
 */

#include <stdint.h>
#include <stdlib.h>

#if defined(__linux__) || defined(__APPLE__)
#define MAX_READ_LENGTH 4096
#include <sys/types.h>
#include <unistd.h>
#include <pwd.h>
#if defined(__APPLE__)
#include <sys/sysctl.h>
#define SYSCTL_ARG_NUM_HW_CPU 2
#endif

extern char* CODE_OS_TtynameR(int fd)
{
    char* buff = (char*)malloc(MAX_READ_LENGTH * sizeof(char));
    if (buff == NULL) {
        return NULL;
    }
    if (ttyname_r(fd, buff, MAX_READ_LENGTH) == 0) {
        return buff;
    } else {
        free(buff);
        return NULL;
    }
}

#endif
