/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2025. All rights reserved.
 * This source file is part of the Codira project, licensed under Apache-2.0
 * with Runtime Library Exception.
 *
 * See https://cangjie-lang.cn/pages/LICENSE for license information.
 */

#include <signal.h>
#include <stdint.h>
#include <sys/stat.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>

#include "securec.h"

extern int32_t CODE_OS_Close(int32_t fd)
{
    return close(fd);
}

extern int64_t CODE_OS_Lseek(int32_t fd, int64_t offset, int32_t whence)
{
    return lseek(fd, offset, whence);
}

extern int32_t CODE_OS_Dup(int32_t oldfd)
{
    return dup(oldfd);
}

extern int32_t CODE_OS_Dup2(int32_t oldfd, int32_t newfd)
{
    return dup2(oldfd, newfd);
}

extern ssize_t CODE_OS_Read(int32_t fd, uint8_t* buf, size_t count)
{
    return read(fd, buf, count);
}

extern ssize_t CODE_OS_Write(int32_t fd, uint8_t* buf, size_t count)
{
    return write(fd, buf, count);
}

extern uint32_t CODE_OS_Umask(uint32_t mask)
{
    return umask(mask);
}

extern int32_t CODE_OS_Getpid(void)
{
    return getpid();
}

extern int32_t CODE_OS_Isatty(int32_t fd)
{
    return isatty(fd);
}

extern char* CODE_OS_Getlogin(void)
{
    return getlogin();
}

#if defined(__linux__) || defined(__APPLE__) || defined(__ohos__)
extern ssize_t CODE_OS_Pread(int32_t fd, uint8_t* buf, size_t count, int32_t offset)
{
    return pread(fd, buf, count, offset);
}

extern ssize_t CODE_OS_Pwrite(int32_t fd, uint8_t* buf, size_t count, int32_t offset)
{
    return pwrite(fd, buf, count, offset);
}
#endif

#if defined(__linux__) || defined(__APPLE__)
extern char* CODE_OS_GetOs(void)
{
    FILE* fd = fopen("/proc/version", "r");
    if (!fd) {
        return NULL;
    }

    char* buffer = NULL;
    size_t len = 0;
    ssize_t ret = getline(&buffer, &len, fd);
    (void)fclose(fd);

    if (ret == -1) {
        free(buffer);
        return NULL;
    }
    return buffer;
}

extern int32_t CODE_OS_Fchown(int32_t fd, uint32_t owner, uint32_t group)
{
    return fchown(fd, owner, group);
}

extern int32_t CODE_OS_Fchmod(int32_t fd, uint32_t mode)
{
    return fchmod(fd, mode);
}

extern uint32_t CODE_OS_Getgid(void)
{
    return getgid();
}

extern uint32_t CODE_OS_Getuid(void)
{
    return getuid();
}

extern int32_t CODE_OS_Setgid(uint32_t gid)
{
    return setgid(gid);
}

extern int32_t CODE_OS_Setuid(uint32_t uid)
{
    return setuid(uid);
}

extern int32_t CODE_OS_Getpgid(int32_t pid)
{
    return getpgid(pid);
}

extern int32_t CODE_OS_Getgroups(int32_t size, uint32_t* gidArray)
{
    return getgroups(size, gidArray);
}

extern int32_t CODE_OS_Getppid(void)
{
    return getppid();
}

extern int32_t CODE_OS_Setpgid(int32_t pid, int32_t pgid)
{
    return setpgid(pid, pgid);
}

extern int32_t CODE_OS_Getpgrp(void)
{
    return getpgrp();
}

extern int32_t CODE_OS_Setpgrp(void)
{
    return setpgrp();
}

extern int32_t CODE_OS_Nice(int32_t inc)
{
    return nice(inc);
}

extern int32_t CODE_OS_Kill(int32_t pid, int32_t sig)
{
    return kill(pid, sig);
}

extern int32_t CODE_OS_Killpg(int32_t pgrp, int32_t sig)
{
    return killpg(pgrp, sig);
}

extern int32_t CODE_OS_Fchdir(int32_t fd)
{
    return fchdir(fd);
}
#endif
