/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2025. All rights reserved.
 * This source file is part of the Codira project, licensed under Apache-2.0
 * with Runtime Library Exception.
 *
 * See https://cangjie-lang.cn/pages/LICENSE for license information.
 */

#include <stdint.h>
#include <stdlib.h>
#include <time.h>

typedef struct NativeClock {
    int64_t sec;
    int64_t nanosec;
} NativeClock;

extern NativeClock CODE_TIME_Now(void)
{
    struct timespec wallNow = {0, 0};
    (void)clock_gettime(CLOCK_REALTIME, &wallNow);
    int64_t sec = wallNow.tv_sec;
    int64_t nanoSec = wallNow.tv_nsec;

    NativeClock ret = {sec, nanoSec};
    return ret;
}

extern NativeClock CODE_TIME_MonotonicNow(void)
{
    struct timespec monoNow = {0, 0};
    (void)clock_gettime(CLOCK_MONOTONIC, &monoNow);
    int64_t sec = monoNow.tv_sec;
    int64_t nanoSec = monoNow.tv_nsec;

    NativeClock ret = {sec, nanoSec};
    return ret;
}

extern const char *CODE_TIME_GetEnvVariable(const char *name)
{
    return getenv(name);
}
