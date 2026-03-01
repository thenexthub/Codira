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
#ifndef PANDA_RUNTIME_SINGLE_THREAD_MANAGER_H
#define PANDA_RUNTIME_SINGLE_THREAD_MANAGER_H

#include "runtime/thread_manager.h"

namespace ark {
class SingleThreadManager : public ThreadManager {
public:
    NO_COPY_SEMANTIC(SingleThreadManager);
    NO_MOVE_SEMANTIC(SingleThreadManager);

    SingleThreadManager() = default;

    ~SingleThreadManager() override = default;

    void WaitForDeregistration() override {}

    void SuspendAllThreads() override
    {
        ManagedThread *mainThread = GetMainThread();
        if (Thread::GetCurrent() != mainThread) {
            mainThread->SuspendImpl(true);
        }
    }

    void ResumeAllThreads() override
    {
        ManagedThread *mainThread = GetMainThread();
        if (Thread::GetCurrent() != mainThread) {
            mainThread->ResumeImpl(true);
        }
    }

    bool IsRunningThreadExist() override
    {
        ManagedThread *mainThread = GetMainThread();
        if (Thread::GetCurrent() != mainThread) {
            if (mainThread->GetStatus() == ThreadStatus::RUNNING) {
                return true;
            }
        }
        return false;
    }

protected:
    bool EnumerateThreadsImpl(const Callback &cb, [[maybe_unused]] unsigned int incMask,
                              [[maybe_unused]] unsigned int xorMask) const override
    {
        return cb(GetMainThread());
    }
};
}  // namespace ark

#endif  // PANDA_RUNTIME_SINGLE_THREAD_MANAGER_H
