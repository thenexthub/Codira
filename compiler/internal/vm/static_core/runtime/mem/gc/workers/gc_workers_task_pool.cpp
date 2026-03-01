/**
 * Copyright (c) 2023-2024 Huawei Device Co., Ltd.
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

#include "runtime/mem/gc/workers/gc_workers_task_pool.h"

namespace ark::mem {

void GCWorkersTaskPool::IncreaseSolvedTasks()
{
    // Atomic with seq_cst order reason: solved_tasks_ value synchronization
    auto solvedTasksOldValue = solvedTasks_.fetch_add(1, std::memory_order_seq_cst);
    if (solvedTasksOldValue + 1U == sendedTasks_) {
        os::memory::LockHolder lock(allSolvedTasksCondVarLock_);
        allSolvedTasksCondVar_.Signal();

        // Here we use double check to be sure that IncreaseSolvedTasks method will release
        // all_solved_tasks_cond_var_lock_ before GCWorkersTaskPool deleting in GC::DestroyWorkersTaskPool method
        if (solvedTasksOldValue >= solvedTasksSnapshot_ && solvedTasksOldValue + 1U == sendedTasks_) {
            solvedTasksSnapshot_ = solvedTasksOldValue + 1U;
        }
    }
}

bool GCWorkersTaskPool::AddTask(GCWorkersTask &&task)
{
    sendedTasks_++;
    [[maybe_unused]] GCWorkersTaskTypes type = task.GetType();
    if (this->TryAddTask(std::forward<GCWorkersTask &&>(task))) {
        LOG(DEBUG, GC) << "Added a new " << GCWorkersTaskTypesToString(type) << " to gc task pool";
        return true;
    }
    // Couldn't add gc workers task to pool
    sendedTasks_--;
    return false;
}

void GCWorkersTaskPool::RunGCWorkersTask(GCWorkersTask *task, void *workerData)
{
    gc_->WorkerTaskProcessing(task, workerData);
    IncreaseSolvedTasks();
}

void GCWorkersTaskPool::WaitUntilTasksEnd()
{
    for (;;) {
        // Current thread can to help workers with gc task processing.
        // Try to get gc workers task from pool if it's possible and run the task immediately
        this->RunInCurrentThread();
        os::memory::LockHolder lock(allSolvedTasksCondVarLock_);
        // If all sended task were solved then break from wait loop
        // Here we use double check to be sure that IncreaseSolvedTasks method will release
        // all_solved_tasks_cond_var_lock_ before GCWorkersTaskPool deleting in GC::DestroyWorkersTaskPool method
        if (solvedTasks_ == sendedTasks_ && solvedTasks_ == solvedTasksSnapshot_) {
            ResetTasks();
            break;
        }
        allSolvedTasksCondVar_.TimedWait(&allSolvedTasksCondVarLock_, ALL_GC_TASKS_FINISH_WAIT_TIMEOUT);
    }
}

}  // namespace ark::mem
