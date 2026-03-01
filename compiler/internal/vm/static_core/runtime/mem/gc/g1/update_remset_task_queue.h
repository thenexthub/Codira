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

#ifndef PANDA_RUNTIME_MEM_GC_G1_UPDATE_REMSET_TASK_QUEUE_H
#define PANDA_RUNTIME_MEM_GC_G1_UPDATE_REMSET_TASK_QUEUE_H

#include "runtime/mem/gc/g1/update_remset_worker.h"

#include "libarkbase/taskmanager/task.h"
#include "libarkbase/taskmanager/utils/wait_list.h"

namespace ark::mem {

template <class LanguageConfig>
class UpdateRemsetTaskQueue final : public UpdateRemsetWorker<LanguageConfig> {
public:
    UpdateRemsetTaskQueue(G1GC<LanguageConfig> *gc, GCG1BarrierSet::ThreadLocalCardQueues *queue,
                          os::memory::Mutex *queueLock, size_t regionSize, bool updateConcurrent,
                          size_t minConcurrentCardsToProcess, size_t hotCardsProcessingFrequency);
    NO_COPY_SEMANTIC(UpdateRemsetTaskQueue);
    NO_MOVE_SEMANTIC(UpdateRemsetTaskQueue);
    ~UpdateRemsetTaskQueue() final;

private:
    void CreateWorkerImpl() final;

    void DestroyWorkerImpl() final;

    void ContinueProcessCards() REQUIRES(this->updateRemsetLock_) final;

    /**
     * @brief Add a new process cards task in task manager if such task does not exist. Else if taskRunnerWaiterId_ is
     * valid, signal wait list to execute task.
     */
    void StartProcessCards() REQUIRES(this->updateRemsetLock_);

    /// @brief Add a new process cards task in task manager wait list with timeout.
    void AddToWaitListWithTimeout() REQUIRES(this->updateRemsetLock_);

    /// @brief Add a new process cards task in task manager wait list. @returns id of waiter in wait list
    void AddToWaitList() REQUIRES(this->updateRemsetLock_);

    bool hasTaskInTaskmanager_ GUARDED_BY(this->updateRemsetLock_) {false};

    std::function<void()> taskRunner_ {nullptr};

    taskmanager::WaiterId taskRunnerWaiterId_ GUARDED_BY(this->updateRemsetLock_) {taskmanager::INVALID_WAITER_ID};
};

}  // namespace ark::mem

#endif  // PANDA_RUNTIME_MEM_GC_G1_UPDATE_REMSET_TASK_QUEUE_H
