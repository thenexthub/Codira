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
#include "common_components/base/sys_call.h"

#include <cstdlib>
#include <unistd.h>
#if defined(_WIN64)
#include <processthreadsapi.h>
#elif defined(__APPLE__)
#include "sys/syscall.h"
#else
#include "linux/futex.h"
#include "sys/syscall.h"
#endif

namespace common {
#ifndef SYS_futex
#define SYS_futex __NR_futex
#endif

#if defined(__linux__) || defined(PANDA_TARGET_OHOS)
// only support FUTEX_WAIT/FUTEX_WAKE
int Futex(const volatile int* uaddr, int op, int val)
{
    return syscall(SYS_futex, uaddr, op, val, nullptr, nullptr, 0);
}
#endif

pid_t GetTid()
{
#if defined(_WIN64)
    return GetCurrentThreadId();
#elif defined(__APPLE__)
    uint64_t tid;
    pthread_threadid_np(nullptr, &tid);
    return static_cast<pid_t>(tid);
#else
    return syscall(SYS_gettid);
#endif
}

int GetPid()
{
#if defined(__APPLE__)
    return getpid();
#elif defined(_WIN64)
    return GetCurrentProcessId();
#else
    return syscall(SYS_getpid);
#endif
}
} // namespace common
