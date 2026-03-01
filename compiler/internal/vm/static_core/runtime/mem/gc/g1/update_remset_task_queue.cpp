/**
 * Copyright (c) 2023-2025 Huawei Device Co., Ltd.
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

#include "runtime/mem/gc/g1/update_remset_task_queue.h"

#include "libarkbase/taskmanager/task_scheduler.h"
#include "runtime/include/language_config.h"
#include "runtime/mem/gc/g1/g1-gc.h"

namespace ark::mem {

template <class LanguageConfig>
UpdateRemsetTaskQueue<LanguageConfig>::UpdateRemsetTaskQueue(G1GC<LanguageConfig> *gc,
                                                             GCG1BarrierSet::ThreadLocalCardQueues *queue,
                                                             os::memory::Mutex *queueLock, size_t regionSize,
                                                             bool updateConcurrent, size_t minConcurrentCardsToProcess,
                                                             size_t hotCardsProcessingFrequency)
    : UpdateRemsetWorker<LanguageConfig>(gc, queue, queueLock, regionSize, updateConcurrent,
                                         minConcurrentCardsToProcess, hotCardsProcessingFrequency)
{
    taskRunner_ = [this]() {
        os::memory::LockHolder lh(this->updateRemsetLock_);
        taskRunnerWaiterId_ = taskmanager::INVALID_WAITER_ID;
        ASSERT(this->hasTaskInTaskmanager_);

        auto iterationFlag = this->GetFlag();
        if (iterationFlag == UpdateRemsetWorker<LanguageConfig>::UpdateRemsetWorkerFlags::IS_STOP_WORKER) {
            this->hasTaskInTaskmanager_ = false;
            return;
        }

        if (iterationFlag == UpdateRemsetWorker<LanguageConfig>::UpdateRemsetWorkerFlags::IS_PAUSED_BY_GC_THREAD ||
            iterationFlag == UpdateRemsetWorker<LanguageConfig>::UpdateRemsetWorkerFlags::IS_INVALIDATE_REGIONS) {
            this->AddToWaitList();
            return;
        }

        ASSERT(!this->pausedByGcThread_);
        auto processedCards = this->ProcessAllCards();
        if (processedCards < this->GetMinConcurrentCardsToProcess()) {
            // need to add to wait list with timeout
            this->AddToWaitListWithTimeout();
            return;
        }

        // After cards processing add new cards processing task to task manager
        this->hasTaskInTaskmanager_ = false;
        this->StartProcessCards();
    };
}

template <class LanguageConfig>
UpdateRemsetTaskQueue<LanguageConfig>::~UpdateRemsetTaskQueue() = default;

template <class LanguageConfig>
void UpdateRemsetTaskQueue<LanguageConfig>::StartProcessCards()
{
    // If we already have process cards task in task manager then no need to add a new task
    if (hasTaskInTaskmanager_) {
        return;
    }
    ASSERT(taskRunner_);
    hasTaskInTaskmanager_ = true;
    this->GetGC()->GetWorkersTaskQueue()->AddForegroundTask(taskRunner_);
}

template <class LanguageConfig>
void UpdateRemsetTaskQueue<LanguageConfig>::ContinueProcessCards()
{
    if (taskRunnerWaiterId_ != taskmanager::INVALID_WAITER_ID) {
        this->GetGC()->GetWorkersTaskQueue()->SignalWaitList(taskRunnerWaiterId_);
    } else {
        StartProcessCards();
    }
}

template <class LanguageConfig>
void UpdateRemsetTaskQueue<LanguageConfig>::CreateWorkerImpl()
{
    os::memory::LockHolder lh(this->updateRemsetLock_);
    ASSERT(!hasTaskInTaskmanager_);
    StartProcessCards();
}

template <class LanguageConfig>
void UpdateRemsetTaskQueue<LanguageConfig>::DestroyWorkerImpl()
{
    this->GetGC()->GetWorkersTaskQueue()->WaitForegroundTasks();
    [[maybe_unused]] os::memory::LockHolder lh(this->updateRemsetLock_);
    ASSERT(!hasTaskInTaskmanager_);
}

template <class LanguageConfig>
void UpdateRemsetTaskQueue<LanguageConfig>::AddToWaitList()
{
    ASSERT(taskRunner_);
    taskRunnerWaiterId_ = this->GetGC()->GetWorkersTaskQueue()->AddForegroundTaskInWaitList(taskRunner_);
}

template <class LanguageConfig>
void UpdateRemsetTaskQueue<LanguageConfig>::AddToWaitListWithTimeout()
{
    ASSERT(taskRunner_);
    static constexpr uint64_t TIMEOUT_MS = 1U;
    taskRunnerWaiterId_ = this->GetGC()->GetWorkersTaskQueue()->AddForegroundTaskInWaitList(taskRunner_, TIMEOUT_MS);
}

TEMPLATE_CLASS_LANGUAGE_CONFIG(UpdateRemsetTaskQueue);
}  // namespace ark::mem
