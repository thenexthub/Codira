/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2025. All rights reserved.
 * This source file is part of the Codira project, licensed under Apache-2.0
 * with Runtime Library Exception.
 *
 * See https://cangjie-lang.cn/pages/LICENSE for license information.
 */

#include "cpu_counters.h"

#ifdef __linux__
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <unistd.h>
#include <dirent.h>
#include <errno.h>
#include <sys/user.h>
#include <sys/mman.h>
#include <sys/ioctl.h>
#include <sys/syscall.h>
#include <sys/ptrace.h>
#include <sys/wait.h>
#include <linux/perf_event.h>

const uint64_t COUNTER_MAP[] = {PERF_COUNT_HW_CPU_CYCLES, PERF_COUNT_HW_INSTRUCTIONS, PERF_COUNT_HW_CACHE_REFERENCES,
    PERF_COUNT_HW_CACHE_MISSES, PERF_COUNT_HW_BRANCH_INSTRUCTIONS, PERF_COUNT_HW_BRANCH_MISSES,
    PERF_COUNT_HW_BUS_CYCLES, PERF_COUNT_HW_STALLED_CYCLES_FRONTEND, PERF_COUNT_HW_STALLED_CYCLES_BACKEND,
    PERF_COUNT_HW_REF_CPU_CYCLES, PERF_COUNT_SW_CPU_CLOCK, PERF_COUNT_SW_TASK_CLOCK, PERF_COUNT_SW_PAGE_FAULTS,
    PERF_COUNT_SW_CONTEXT_SWITCHES, PERF_COUNT_SW_CPU_MIGRATIONS, PERF_COUNT_SW_PAGE_FAULTS_MIN,
    PERF_COUNT_SW_PAGE_FAULTS_MAJ, PERF_COUNT_SW_EMULATION_FAULTS};

#define LAST_HW_COUNTER_INDEX 9

static uint64_t MapConfig(int counter)
{
    return COUNTER_MAP[counter];
}

static uint64_t MapType(int counter)
{
    if (counter <= LAST_HW_COUNTER_INDEX) {
        return PERF_TYPE_HARDWARE;
    } else {
        return PERF_TYPE_SOFTWARE;
    }
}

int g_fd = 0;

int InitPerf(int counter)
{
    const int supportedCounters = sizeof(COUNTER_MAP) / sizeof(uint64_t);
    static int counterId = -1;

    if (counter >= supportedCounters || counter < 0) {
        return -1;
    }
    if (counterId == counter) {
        return 0;
    }

    struct perf_event_attr pe = {0};
    if (g_fd != 0) {
        ioctl(g_fd, PERF_EVENT_IOC_DISABLE, 0);
        close(g_fd);
        g_fd = 0;
        counterId = -1;
    }
    // Configure the event to count
    pe.type = (unsigned int)MapType(counter);
    pe.size = sizeof(struct perf_event_attr);
    pe.config = MapConfig(counter);
    pe.disabled = 1;
    pe.exclude_kernel = 1; // Do not measure instructions executed in the kernel
    pe.exclude_hv = 1;

    g_fd = (int)syscall(SYS_perf_event_open, &pe, getpid(), -1, -1, 0);
    if (g_fd < 0) {
        fprintf(stderr, "error: Open perf event failed %d.\n", errno);
        return errno;
    }

    ioctl(g_fd, PERF_EVENT_IOC_RESET, 0);
    ioctl(g_fd, PERF_EVENT_IOC_ENABLE, 0);
    counterId = counter;

    return 0;
}

uint64_t ReadPerf(void)
{
    uint64_t val = 0;

    ssize_t len = read(g_fd, &val, sizeof(val));
    if (len < 0) {
        return (uint64_t)-1;
    }

    return val;
}

#endif // ifdef __linux__

#if defined(__x86_64__) || defined(_M_X64) || defined(i386) \
    || defined(__i386__) || defined(__i386) || defined(_M_IX86)

#ifdef _WIN32
#include <intrin.h>
#else
#include <x86intrin.h>
#endif

uint64_t GetRdtscp(void)
{
    unsigned int ui;
    return __rdtscp(&ui);
}

uint64_t GetRdtsc(void)
{
    return __rdtsc();
}

#endif
