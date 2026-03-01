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

#include "runtime/scheduler/worker_thread.h"
#include "runtime/include/runtime.h"

namespace ark::scheduler {

WorkerThread::WorkerThread(PandaVM *vm) : Thread(vm, Thread::ThreadType::THREAD_TYPE_WORKER_THREAD) {}

/* static - creation of the initial Worker thread */
WorkerThread *WorkerThread::Create(PandaVM *vm)
{
    ASSERT(Thread::GetCurrent() == nullptr);
    auto allocator = Runtime::GetCurrent()->GetInternalAllocator();
    auto *thread = allocator->New<WorkerThread>(vm);
    thread->ProcessCreatedThread();
    return thread;
}

/* Common actions for creation of the thread. */
void WorkerThread::ProcessCreatedThread()
{
    Thread::SetCurrent(this);
    // Runtime takes ownership of the thread
    trace::ScopedTrace scopedTrace2("ThreadManager::RegisterThread");
    // NOTE(xuliang): RegisterThread
}

WorkerThread *WorkerThread::AttachThread()
{
    trace::ScopedTrace scopedTrace(__FUNCTION__);

    return WorkerThread::Create(Runtime::GetCurrent()->GetPandaVM());
}

void WorkerThread::Destroy()
{
    ASSERT(this == WorkerThread::GetCurrent());
    // NOTE(xuliang): should be done in UnregisterExitedThread.
    auto allocator = Runtime::GetCurrent()->GetInternalAllocator();
    allocator->Delete(this);
    Thread::SetCurrent(nullptr);
}

}  // namespace ark::scheduler
