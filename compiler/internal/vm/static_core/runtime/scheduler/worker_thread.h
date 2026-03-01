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

#ifndef PANDA_RUNTIME_SCHEDULER_WORKER_THREAD_H_
#define PANDA_RUNTIME_SCHEDULER_WORKER_THREAD_H_

#include "runtime/include/thread.h"

namespace ark::scheduler {

class Task;

// Worker thread is a physical OS thread.
class WorkerThread : public ark::Thread {
public:
    explicit WorkerThread(PandaVM *vm);
    // We need to override this to call appropriate destructor in thread manager
    ~WorkerThread() override = default;

    static WorkerThread *GetCurrent()
    {
        auto thread = Thread::GetCurrent();
        ASSERT(thread != nullptr);
        ASSERT(thread->GetThreadType() == Thread::ThreadType::THREAD_TYPE_WORKER_THREAD);
        return static_cast<WorkerThread *>(thread);
    }

    static WorkerThread *Create(PandaVM *vm);
    void ProcessCreatedThread();
    static WorkerThread *AttachThread();
    static void DetachThread()
    {
        trace::ScopedTrace scopedTrace(__FUNCTION__);
        auto thread = GetCurrent();
        thread->Detach();
    }

    void Destroy();
    void SetTask(Task *tk)
    {
        task_ = tk;
    }

private:
    Task *task_ = nullptr;
    void Detach()
    {
        LOG(DEBUG, RUNTIME) << "Detaching worker thread";
        Destroy();
    }

    NO_COPY_SEMANTIC(WorkerThread);
    NO_MOVE_SEMANTIC(WorkerThread);
};

}  // namespace ark::scheduler

#endif  // PANDA_RUNTIME_SCHEDULER_WORKER_THREAD_H_
