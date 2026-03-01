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

#include "runtime/scheduler/task.h"
#include "runtime/scheduler/worker_thread.h"
#include "runtime/include/runtime.h"
#include "runtime/include/panda_vm.h"

namespace ark::scheduler {

// NOTE(xuliang): task id
Task::Task(PandaVM *vm, ObjectHeader *obj)
    : ManagedThread(-1, Runtime::GetCurrent()->GetInternalAllocator(), vm, Thread::ThreadType::THREAD_TYPE_TASK)
{
    future_ = obj;
}

Task *Task::Create(PandaVM *vm, ObjectHeader *obj)
{
    auto allocator = Runtime::GetCurrent()->GetInternalAllocator();
    auto *task = allocator->New<Task>(vm, obj);
    task->Initialize();
    return task;
}

void Task::Initialize()
{
    trace::ScopedTrace scopedTrace2("ThreadManager::RegisterThread");
    // NOTE(xuliang): RegisterThread
}

void Task::Destroy()
{
    ASSERT(this == Task::GetCurrent());
    // NOTE(xuliang): should be done in UnregisterExitedThread.
    GetVM()->GetGC()->OnThreadTerminate(this, mem::BuffersKeepingFlag::DELETE);
    auto allocator = Runtime::GetCurrent()->GetInternalAllocator();
    auto wt = workerThread_;
    allocator->Delete(this);
    Thread::SetCurrent(wt);
}

void Task::SwitchFromWorkerThread()
{
    workerThread_ = WorkerThread::GetCurrent();
    workerThread_->SetTask(this);
    Thread::SetCurrent(this);
}

void Task::SuspendCurrent()
{
    auto task = Task::GetCurrent();
    auto wt = task->workerThread_;
    ASSERT(wt != nullptr);
    Thread::SetCurrent(wt);
    wt->SetTask(nullptr);
}

void Task::EndCurrent()
{
    auto task = Task::GetCurrent();
    auto wt = task->workerThread_;
    ASSERT(wt != nullptr);
    task->Destroy();
    Thread::SetCurrent(wt);
    wt->SetTask(nullptr);
}

}  // namespace ark::scheduler
