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

#include <algorithm>
#include <chrono>
#include <cstdint>
#include <cstdio>
#ifdef PANDA_USE_FFRT
#include "c/executor_task.h"
#include "ffrt.h"
#include "ffrt_inner.h"
#endif
#include "libarkbase/os/thread.h"
#include "libarkbase/utils/logger.h"
#ifdef PANDA_TARGET_OHOS
#include "libarkbase/os/mem.h"
#endif
#ifdef PANDA_TARGET_UNIX
#include <fcntl.h>
#include <pthread.h>
#include <sys/resource.h>
#include <sys/syscall.h>
#include <csignal>
#endif
#include <securec.h>
#include <unistd.h>

namespace ark::os::thread {
[[maybe_unused]] constexpr size_t NO_GUARD_SIZE = 0;

ThreadId GetCurrentThreadId()
{
#if defined(HAVE_GETTID)
    static_assert(sizeof(decltype(gettid())) == sizeof(ThreadId), "Incorrect alias for ThreadID");
    return static_cast<ThreadId>(gettid());
#elif defined(PANDA_TARGET_MACOS)
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

int GetPPid()
{
    return getppid();
}

int GetUid()
{
    return getuid();
}

int GetEuid()
{
    return geteuid();
}

uint32_t GetGid()
{
    return getgid();
}

uint32_t GetEgid()
{
    return getegid();
}

std::vector<uint32_t> GetGroups()
{
    int groupCount = getgroups(0, nullptr);
    if (groupCount == -1) {
        return {};
    }

    std::vector<gid_t> groups(groupCount);
    groupCount = getgroups(groupCount, groups.data());
    if (groupCount == -1) {
        return {};
    }

    groups.resize(groupCount);

    gid_t egid = getegid();
    if (std::find(groups.begin(), groups.end(), egid) == groups.end()) {
        groups.push_back(egid);
    }

    return groups;
}

int SetPriority(int threadId, int prio)
{
    // The priority can be set within range [-20, 19]
    ASSERT(prio <= 19);   // 19: the lowest priority
    ASSERT(prio >= -20);  // -20: the highest priority
    // The return value is 0 if the function succeeds, and -1 if it fails.
    return setpriority(PRIO_PROCESS, threadId, prio);
}

int GetPriority(int threadId)
{
    return getpriority(PRIO_PROCESS, threadId);
}

int SetThreadName(NativeHandleType pthreadHandle, const char *name)
{
    ASSERT(pthreadHandle != 0);
#if defined(PANDA_TARGET_MACOS)
    return pthread_setname_np(name);
#else
    return pthread_setname_np(pthreadHandle, name);
#endif
}

NativeHandleType GetNativeHandle()
{
    return pthread_self();
}

void Yield()
{
    std::this_thread::yield();
}

void NativeSleep(unsigned int ms)
{
    std::this_thread::sleep_for(std::chrono::milliseconds(ms));
}

void NativeSleepUS(std::chrono::microseconds us)
{
    std::this_thread::sleep_for(us);
}

void ThreadDetach(NativeHandleType pthreadHandle)
{
    pthread_detach(pthreadHandle);
}

void ThreadJoin(NativeHandleType pthreadHandle, void **ret)
{
    pthread_join(pthreadHandle, ret);
}

void ThreadSendSignal(NativeHandleType pthreadHandle, int sig)
{
    LOG_IF(pthread_kill(pthreadHandle, sig) != 0, FATAL, COMMON) << "pthread_kill failed";
}

int ThreadGetStackInfo(NativeHandleType thread, void **stackAddr, size_t *stackSize, size_t *guardSize)
{
#ifdef PANDA_USE_FFRT
    /**
     * ffrt_get_current_coroutine_stack is a function that returns the stack information of the ffrt coroutine.
     * If the stack information is not available, it returns 0.
     * If the stack information is available, it returns the stack addr.
     * Currently lots of sdk codes using the ffrt to submit their task which will casuse native stack overflow.
     */
    if (ffrt_get_current_coroutine_stack(stackAddr, stackSize)) {
        *guardSize = NO_GUARD_SIZE;
        return 0;
    }
#endif /*PANDA_USE_FFRT*/
    pthread_attr_t attr;
    int s = pthread_attr_init(&attr);
#ifndef PANDA_TARGET_MACOS
    s += pthread_getattr_np(thread, &attr);
    if (s == 0) {
        s += pthread_attr_getguardsize(&attr, guardSize);
        s += pthread_attr_getstack(&attr, stackAddr, stackSize);
#if defined(PANDA_TARGET_OHOS)
        if (getpid() == gettid()) {
            /**
             *  konstanting:
             *  main thread's stack can automatically grow up to the RLIMIT_STACK by means of the OS,
             *  but MUSL does not care about that and returns the current (already mmap-ped) stack size.
             *  This can lead to complicated errors, so let's adjust the stack size manually.
             */
            struct rlimit lim;
            s += getrlimit(RLIMIT_STACK, &lim);
            if (s == 0) {
                uintptr_t stackHiAddr = ToUintPtr(*stackAddr) + *stackSize;
                size_t stackSizeLimit = lim.rlim_cur;

                static constexpr size_t OHOS_DEFAULT_STACK_LIMIT_SIZE = 8_MB;
                // HWASAN sets stack limit size to unlimited, so we manually set it to default OHOS stack size limit
                stackSizeLimit = stackSizeLimit == std::numeric_limits<size_t>::max() ? OHOS_DEFAULT_STACK_LIMIT_SIZE
                                                                                      : stackSizeLimit;

                // for some reason pthread interfaces subtract 1 page from size regardless of guard size
                uintptr_t stackLoAddr = stackHiAddr - stackSizeLimit + ark::os::mem::GetPageSize();
                *stackSize = stackSizeLimit;
                *stackAddr = ToVoidPtr(stackLoAddr);
            }
        }
#endif /* defined(PANDA_TARGET_OHOS) */
    }
#else  /* PANDA_TARGET_MACOS */
    *stackAddr = pthread_get_stackaddr_np(thread);
    *stackSize = pthread_get_stacksize_np(thread);
    s += pthread_attr_getguardsize(&attr, guardSize);
#endif /* PANDA_TARGET_MACOS */
    s += pthread_attr_destroy(&attr);
    return s;
}

}  // namespace ark::os::thread
