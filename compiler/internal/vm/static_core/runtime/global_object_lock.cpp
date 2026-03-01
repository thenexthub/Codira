/**
 * Copyright (c) 2021-2024 Huawei Device Co., Ltd.
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 * http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include "global_object_lock.h"
#include "runtime/include/thread_scopes.h"

namespace ark {
// Use some global lock as fast solution.
static os::memory::RecursiveMutex g_mtx;    // NOLINT(fuchsia-statically-constructed-objects)
static os::memory::ConditionVariable g_cv;  // NOLINT(fuchsia-statically-constructed-objects)

GlobalObjectLock::GlobalObjectLock([[maybe_unused]] const ObjectHeader *obj)
{
    ScopedChangeThreadStatus s(ManagedThread::GetCurrent(), ThreadStatus::IS_BLOCKED);
    g_mtx.Lock();
}

bool GlobalObjectLock::Wait([[maybe_unused]] bool ignoreInterruption) const
{
    ScopedChangeThreadStatus s(ManagedThread::GetCurrent(), ThreadStatus::IS_WAITING);
    g_cv.Wait(&g_mtx);
    return true;
}

bool GlobalObjectLock::TimedWait(uint64_t timeout) const
{
    ScopedChangeThreadStatus s(ManagedThread::GetCurrent(), ThreadStatus::IS_TIMED_WAITING);
    g_cv.TimedWait(&g_mtx, timeout);
    return true;
}

void GlobalObjectLock::Notify()
{
    g_cv.Signal();
}

void GlobalObjectLock::NotifyAll()
{
    g_cv.SignalAll();
}

GlobalObjectLock::~GlobalObjectLock()
{
    g_mtx.Unlock();
}
}  // namespace ark
