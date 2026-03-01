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
#include <pwd.h>
#include <sys/types.h>
#include <unistd.h>
#if defined(__APPLE__)
#include <sys/sysctl.h>
#define SYSCTL_ARG_NUM_HW_CPU 2
#endif
extern char** environ;

extern int64_t CODE_OS_ProcessorCount(void)
{
#if defined(__linux__)
    return (int64_t)sysconf(_SC_NPROCESSORS_ONLN);
#else
    int numCPU = 0;
    size_t len = sizeof(numCPU);

    int mib[SYSCTL_ARG_NUM_HW_CPU];
    mib[0] = CTL_HW;
    mib[1] = HW_AVAILCPU; // alternatively, try HW_NCPU;

    sysctl(mib, SYSCTL_ARG_NUM_HW_CPU, &numCPU, &len, NULL, 0);
    if (numCPU < 1) {
        mib[1] = HW_NCPU;
        sysctl(mib, SYSCTL_ARG_NUM_HW_CPU, &numCPU, &len, NULL, 0);
        if (numCPU < 1) {
            numCPU = 1;
        }
    }
    return (int64_t)numCPU;
#endif
}

#else

#include <windows.h>

extern int64_t CODE_OS_ProcessorCount(void)
{
    SYSTEM_INFO sysInfo;
    GetSystemInfo(&sysInfo);
    return (int64_t)sysInfo.dwNumberOfProcessors;
}

#endif
