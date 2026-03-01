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

#include "libarkbase/taskmanager/task_scheduler.h"
#include "runtime/include/thread.h"
#include "runtime/mem/gc/gc.h"
#include "runtime/mem/gc/workers/gc_worker.h"

namespace ark::mem {
GCWorker::GCWorker(GC *gc) : gc_(gc)
{
    auto internalAllocator = gc_->GetInternalAllocator();
    gcTaskQueue_ = internalAllocator->New<GCQueueWithTime>(gc_);
    ASSERT(gcTaskQueue_ != nullptr);
    auto *vm = gc_->GetPandaVm();
    ASSERT(vm != nullptr);
    gcThread_ = internalAllocator->New<Thread>(vm, Thread::ThreadType::THREAD_TYPE_GC);
    ASSERT(gcThread_ != nullptr);
    if (gc_->GetSettings()->UseTaskManagerForGC()) {
        gcRunner_ = [this]() { this->GCTaskRunner(); };
    }
}

GCWorker::~GCWorker()
{
    auto internalAllocator = gc_->GetInternalAllocator();
    internalAllocator->Delete(gcThread_);
    internalAllocator->Delete(gcTaskQueue_);
}

/* static */
void GCWorker::GCThreadLoop(GCWorker *gcWorker)
{
    // We need to set VM to current_thread, since GC can call ObjectAccessor::GetBarrierSet() methods
    ScopedCurrentThread gcCurrentThreadScope(gcWorker->gcThread_);

    while (true) {
        // Get gc task from local gc tasks queue
        auto task = gcWorker->GetTask();
        if (!gcWorker->gc_->IsGCRunning()) {
            LOG(DEBUG, GC) << "Stopping GC thread";
            break;
        }
        gcWorker->RunGC(std::move(task));
    }
}

void GCWorker::CreateAndStartWorker()
{
    // If GC runs in place or Task manager is used for GC, so no need create separate internal GC worker
    if (gc_->GetSettings()->RunGCInPlace()) {
        return;
    }
    if (gc_->GetSettings()->UseTaskManagerForGC()) {
        needToFinish_ = false;
        return;
    }
    ASSERT(gc_->GetSettings()->UseThreadPoolForGC());
    ASSERT(gcInternalThread_ == nullptr);
    auto allocator = gc_->GetInternalAllocator();
    gcInternalThread_ = allocator->New<std::thread>(GCWorker::GCThreadLoop, this);
    ASSERT(gcInternalThread_ != nullptr);
    auto setGcThreadNameResult = os::thread::SetThreadName(gcInternalThread_->native_handle(), "GCThread");
    LOG_IF(setGcThreadNameResult != 0, ERROR, RUNTIME) << "Failed to set a name for the gc thread";
}

void GCWorker::FinalizeAndDestroyWorker()
{
    // Signal that no need to delay task running
    gcTaskQueue_->Signal();
    if (gc_->GetSettings()->UseTaskManagerForGC()) {
        needToFinish_ = true;
        gc_->GetWorkersTaskQueue()->WaitBackgroundTasks();
        return;
    }
    ASSERT(gc_->GetSettings()->UseThreadPoolForGC());
    // Internal GC thread was not created, so just return
    if (gcInternalThread_ == nullptr) {
        return;
    }
    gcInternalThread_->join();
    gc_->GetInternalAllocator()->Delete(gcInternalThread_);
    gcInternalThread_ = nullptr;
}

void GCWorker::CreateAndAddTaskToTaskManager()
{
    ASSERT_PRINT(gcRunner_ != nullptr, "Need to create task only for TaskManager case");
    gc_->GetWorkersTaskQueue()->AddBackgroundTask(gcRunner_);
}

void GCWorker::GCTaskRunner()
{
    // only one task can get gc task from queue and run it
    if (!gcTaskRunMutex_.TryLock()) {
        // If any task is executed in TaskManager then current task should do nothing and just return
        // According task for TaskManager will be created after RunGC if needed
        return;
    }
    // Task manager does not know anything about panda threads, so set gc thread as current thread during task running
    ScopedCurrentThread gcCurrentThreadScope(gcThread_);
    auto gcTask = GetTask();
    // If GC was not started then task should not be run, so delay the task execution
    if (!gc_->IsGCRunning()) {
        if (!needToFinish_) {
            // Added task can run on another worker and try to lock gc_task_run_mutex_, but in the current worker we
            // already held the mutex, so TryLock fails and task running cancels
            // So unlock the mutex before adding task
            gcTaskRunMutex_.Unlock();
            AddTask(std::move(gcTask));
        } else {
            gcTaskRunMutex_.Unlock();
        }
        return;
    }
    RunGC(std::move(gcTask));
    gcTaskRunMutex_.Unlock();
    // If gc tasks queue has a task, so need to create Task for TaskManager to process it
    if (!gcTaskQueue_->IsEmpty()) {
        CreateAndAddTaskToTaskManager();
    }
}

bool GCWorker::AddTask(PandaUniquePtr<GCTask> task)
{
    bool wasAdded = gcTaskQueue_->AddTask(std::move(task));
    // If Task Manager is used then create a new task for task manager and put it
    if (wasAdded && gc_->GetSettings()->UseTaskManagerForGC()) {
        CreateAndAddTaskToTaskManager();
    }
    return wasAdded;
}

PandaUniquePtr<GCTask> GCWorker::GetTask()
{
    auto fullGcBombingFreq = gc_->GetSettings()->FullGCBombingFrequency();
    // 0 means full gc bombing is not used, so just return task from local queue
    if (fullGcBombingFreq == 0U) {
        return gcTaskQueue_->GetTask(gc_->GetSettings()->UseThreadPoolForGC());
    }
    // Need to bombs full GC in according with full gc bombing frequency
    if (collectNumberMod_ == fullGcBombingFreq) {
        collectNumberMod_ = 1;
        return MakePandaUnique<GCTask>(GCTaskCause::OOM_CAUSE, time::GetCurrentTimeInNanos());
    }
    ++collectNumberMod_;
    return gcTaskQueue_->GetTask(gc_->GetSettings()->UseThreadPoolForGC());
}

void GCWorker::RunGC(PandaUniquePtr<GCTask> task)
{
    if (task == nullptr || task->reason == GCTaskCause::INVALID_CAUSE) {
        return;
    }
    if (gc_->IsPostponeEnabled() && (task->reason == GCTaskCause::HEAP_USAGE_THRESHOLD_CAUSE)) {
        os::memory::LockHolder lh(postponedTasksMutex_);
        if (postponedTasks_.empty() || postponedTasks_.back()->reason != task->reason) {
            postponedTasks_.push(std::move(task));
        }
        return;
    }
    LOG(DEBUG, GC) << "Running GC task, reason " << task->reason;
    task->Run(*gc_);
}

void GCWorker::OnPostponeGCEnd()
{
    os::memory::LockHolder lh(postponedTasksMutex_);
    while (!postponedTasks_.empty()) {
        AddTask(std::move(postponedTasks_.back()));
        postponedTasks_.pop();
    }
}

}  // namespace ark::mem
