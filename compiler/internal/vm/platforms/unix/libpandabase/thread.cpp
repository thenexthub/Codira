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

#include "utils/logger.h"

#ifdef PANDA_TARGET_UNIX
#include <unistd.h>
#include <sys/resource.h>
#include <csignal>
#endif

namespace panda::os::thread {

ThreadId GetCurrentThreadId()
{
#if defined(HAVE_GETTID)
    static_assert(sizeof(decltype(gettid())) == sizeof(ThreadId), "Incorrect alias for ThreadID");
    return static_cast<ThreadId>(gettid());
#elif defined(PANDA_TARGET_MACOS) || defined(PANDA_TARGET_IOS)
    uint64_t tid64;
    pthread_threadid_np(NULL, &tid64);
    return static_cast<ThreadId>(tid64);
#else
    // NOLINTNEXTLINE(cppcoreguidelines-pro-type-vararg)
    return static_cast<ThreadId>(syscall(SYS_gettid));
#endif
}

int GetPid()
{
    return getpid();
}

int SetPriority(int thread_id, int prio)
{
    // The priority can be set within range [-20, 19]
    ASSERT(prio <= 19);  // 19: the lowest priority
    ASSERT(prio >= -20);  // -20: the highest priority
    // The return value is 0 if the function succeeds, and -1 if it fails.
    return setpriority(PRIO_PROCESS, thread_id, prio);
}

int GetPriority(int thread_id)
{
    return getpriority(PRIO_PROCESS, thread_id);
}

int SetThreadName(native_handle_type pthread_handle, const char *name)
{
    ASSERT(pthread_handle != 0);
#if defined(PANDA_TARGET_MACOS) || defined(PANDA_TARGET_IOS)
    return pthread_setname_np(name);
#else
    return pthread_setname_np(pthread_handle, name);
#endif
}

native_handle_type GetNativeHandle()
{
    return pthread_self();
}

void ThreadYield()
{
    std::this_thread::yield();
}

void NativeSleep(unsigned int ms)
{
    std::this_thread::sleep_for(std::chrono::milliseconds(ms));
}

void ThreadDetach(native_handle_type pthread_handle)
{
    pthread_detach(pthread_handle);
}

void ThreadExit(void *ret)
{
    pthread_exit(ret);
}

void ThreadJoin(native_handle_type pthread_handle, void **ret)
{
    pthread_join(pthread_handle, ret);
}

void ThreadSendSignal(native_handle_type pthread_handle, int sig)
{
    LOG_IF(pthread_kill(pthread_handle, sig) != 0, FATAL, COMMON) << "pthread_kill failed";
}

int ThreadGetStackInfo(native_handle_type thread, void **stack_addr, size_t *stack_size, size_t *guard_size)
{
    pthread_attr_t attr;
    int s = pthread_attr_init(&attr);
#if !defined(PANDA_TARGET_MACOS) && !defined(PANDA_TARGET_IOS)
    s += pthread_getattr_np(thread, &attr);
    if (s == 0) {
        s += pthread_attr_getguardsize(&attr, guard_size);
        s += pthread_attr_getstack(&attr, stack_addr, stack_size);
    }
#else
    s += pthread_attr_getguardsize(&attr, guard_size);
    s += pthread_attr_getstack(&attr, stack_addr, stack_size);
#endif
    s += pthread_attr_destroy(&attr);
    return s;
}

}  // namespace panda::os::thread
