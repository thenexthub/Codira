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

#include "common_interfaces/base/os/thread.h"

#include <pthread.h>
#include <unistd.h>

#include "common_interfaces/base/common.h"

namespace common::os::thread {

ThreadId GetCurrentThreadId()
{
#if defined(HAVE_GETTID)
    static_assert(sizeof(decltype(gettid())) == sizeof(ThreadId), "Incorrect alias for ThreadID");
    return static_cast<ThreadId>(gettid());
#else
    uint64_t tid64;
    pthread_threadid_np(NULL, &tid64);
    return static_cast<ThreadId>(tid64);
#endif
}

NativeHandleType GetNativeHandle()
{
    return pthread_self();
}

int SetThreadName([[maybe_unused]] NativeHandleType threadNativeHandle, const char *name)
{
    DCHECK_CC(threadNativeHandle != 0);
    return pthread_setname_np(name);
}

}  // namespace common::os::thread

