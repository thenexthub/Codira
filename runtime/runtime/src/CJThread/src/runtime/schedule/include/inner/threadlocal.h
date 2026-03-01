/*
 * Copyright (c) NeXTHub Corporation. All Rights Reserved.
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Author: Tunjay Akbarli
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at:
 *
 * http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 *
 * Please contact NeXTHub Corporation, 651 N Broad St, Suite 201,
 * Middletown, DE 19709, New Castle County, USA.
 */

#ifndef MRT_CODETHREAD_THREADLOCAL_H
#define MRT_CODETHREAD_THREADLOCAL_H

#include "macro_def.h"

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif

#ifdef TLS_COMMON_DYNAMIC

#define CODETHREAD_OFFSET 2
#define SCHEDULE_OFFSET 3
#define PREEMPT_FLAG_OFFSET 4
#define PROTECT_ADDR_OFFSET 5
#define PREEMPT_REQUEST_OFFSET 6

extern GetTlsHookFunc g_getTlsFunc;

MRT_INLINE static struct CODEThread *CODEThreadGet(void)
{
    uintptr_t *addr = g_getTlsFunc();
    return (struct CODEThread*)(*(addr + CODETHREAD_OFFSET));
}

MRT_INLINE static void CODEThreadSet(struct CODEThread *codethread)
{
    uintptr_t *addr = g_getTlsFunc();
    *(addr + CODETHREAD_OFFSET) = (uintptr_t)codethread;
}

MRT_INLINE static struct Schedule *ScheduleGet(void)
{
    uintptr_t *addr = g_getTlsFunc();
    return (struct Schedule*)(*(addr + SCHEDULE_OFFSET));
}

MRT_INLINE static void ScheduleSet(struct Schedule *schedule)
{
    uintptr_t *addr = g_getTlsFunc();
    *(addr + SCHEDULE_OFFSET) = (uintptr_t)schedule;
}

MRT_INLINE static void **CODEThreadAddr(void)
{
    uintptr_t *addr = g_getTlsFunc();
    return (void**)(addr + CODETHREAD_OFFSET);
}

MRT_INLINE static uintptr_t PreemptFlagGet(void)
{
    uintptr_t *addr = g_getTlsFunc();
    return *(addr + PREEMPT_FLAG_OFFSET);
}

MRT_INLINE static uintptr_t *PreemptFlagAddr(void)
{
    uintptr_t *addr = g_getTlsFunc();
    return addr + PREEMPT_FLAG_OFFSET;
}

MRT_INLINE static void PreemptFlagSet(uintptr_t value)
{
    uintptr_t *addr = g_getTlsFunc();
    *(addr + PREEMPT_FLAG_OFFSET) = value;
}

MRT_INLINE static uintptr_t *PreemptRequestAddr(void)
{
    uintptr_t *addr = g_getTlsFunc();
    return addr + PREEMPT_REQUEST_OFFSET;
}

MRT_INLINE static uintptr_t ProtectAddrGet(void)
{
    uintptr_t *addr = g_getTlsFunc();
    return *(addr + PROTECT_ADDR_OFFSET);
}

MRT_INLINE static void ProtectAddrSet(uintptr_t value)
{
    uintptr_t *addr = g_getTlsFunc();
    *(addr + PROTECT_ADDR_OFFSET) = value;
}
#else

extern __thread struct CODEThread *g_codethread;
extern __thread struct Schedule *g_schedule;
extern __thread uintptr_t g_preemptFlag;
extern __thread uintptr_t g_protectAddr;

/**
 * @brief Obtains the current scheduling framework.
 * @retval schedule pointer
 */
MRT_INLINE static struct Schedule *ScheduleGet(void)
{
    return g_schedule;
}

/**
 * @brief Set the current scheduling framework.
 */
MRT_INLINE static void ScheduleSet(struct Schedule *schedule)
{
    g_schedule = schedule;
}

/**
 * @brief Obtains the current codethread.
 * @retval codethread pointer
 */
MRT_INLINE static struct CODEThread *CODEThreadGet(void)
{
    return g_codethread;
}

/**
 * @brief Set the specified codethread as the current codethread.
 */
MRT_INLINE static void CODEThreadSet(struct CODEThread *codethread)
{
    g_codethread = codethread;
}

MRT_INLINE static void **CODEThreadAddr(void)
{
    return (void **)&g_codethread;
}

MRT_INLINE static uintptr_t PreemptFlagGet(void)
{
    return g_preemptFlag;
}

MRT_INLINE static uintptr_t *PreemptFlagAddr(void)
{
    return &g_preemptFlag;
}

MRT_INLINE static uintptr_t *PreemptRequestAddr(void)
{
    return NULL;
}

MRT_INLINE static void PreemptFlagSet(uintptr_t value)
{
    g_preemptFlag = value;
}

MRT_INLINE static uintptr_t ProtectAddrGet(void)
{
    return g_protectAddr;
}

MRT_INLINE static void ProtectAddrSet(uintptr_t value)
{
    g_protectAddr = value;
}

#endif

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif

#endif // MRT_CODETHREAD_THREADLOCAL_H
