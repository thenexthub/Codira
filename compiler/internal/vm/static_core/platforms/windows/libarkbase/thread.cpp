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

#include "libarkbase/os/thread.h"
#include "libarkbase/utils/logger.h"

#include <errhandlingapi.h>
#include <handleapi.h>
#include <process.h>
#include <processthreadsapi.h>
#include <thread>

namespace ark::os::thread {
ThreadId GetCurrentThreadId()
{
    // The function is provided by mingw
    return ::GetCurrentThreadId();
}

int GetPid()
{
    return _getpid();
}

int GetPPid()
{
    // Unsupported on windows platform
    UNREACHABLE();
}

int GetUid()
{
    // Unsupported on windows platform
    UNREACHABLE();
}

int GetEuid()
{
    // Unsupported on windows platform
    UNREACHABLE();
}

uint32_t GetGid()
{
    // Unsupported on windows platform
    UNREACHABLE();
}

uint32_t GetEgid()
{
    // Unsupported on windows platform
    UNREACHABLE();
}

std::vector<uint32_t> GetGroups()
{
    // Unsupported on windows platform
    UNREACHABLE();
}

int SetPriority(DWORD threadId, int prio)
{
    // The priority can be set within range [-2, 2]
    ASSERT(prio >= -2);  // -2: the lowest priority
    ASSERT(prio <= 2);   // 2: the highest priority
    HANDLE thread = OpenThread(THREAD_SET_INFORMATION, false, threadId);
    if (thread == NULL) {
        LOG(FATAL, COMMON) << "OpenThread failed, error code " << GetLastError();
    }
    auto ret = SetThreadPriority(thread, prio);
    CloseHandle(thread);
    // Thre return value is nonzero if the function succeeds, and zero if it fails.
    return ret;
}

int GetPriority(DWORD threadId)
{
    HANDLE thread = OpenThread(THREAD_QUERY_INFORMATION, false, threadId);
    if (thread == NULL) {
        LOG(FATAL, COMMON) << "OpenThread failed, error code " << GetLastError();
    }
    auto ret = GetThreadPriority(thread);
    CloseHandle(thread);
    return ret;
}

int SetThreadName(NativeHandleType pthreadHandle, const char *name)
{
    ASSERT(pthreadHandle != 0);
    pthread_t thread = reinterpret_cast<pthread_t>(pthreadHandle);
    return pthread_setname_np(thread, name);
}

NativeHandleType GetNativeHandle()
{
    return reinterpret_cast<NativeHandleType>(pthread_self());
}

void Yield()
{
    std::this_thread::yield();
}

void NativeSleep(unsigned int ms)
{
    std::this_thread::sleep_for(std::chrono::milliseconds(ms));
}

void ThreadDetach(NativeHandleType pthreadHandle)
{
    pthread_detach(reinterpret_cast<pthread_t>(pthreadHandle));
}

void ThreadJoin(NativeHandleType pthreadHandle, void **ret)
{
    pthread_join(reinterpret_cast<pthread_t>(pthreadHandle), ret);
}
}  // namespace ark::os::thread
