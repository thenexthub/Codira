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

#include "libarkbase/taskmanager/task_manager.h"
#include "libarkbase/utils/logger.h"

namespace ark::taskmanager::internal {

TaskQueueSet::TaskQueueSet(TaskWaitList *waitList, TaskTimeStatsType taskTimeStatsType) : waitList_(waitList)
{
    switch (taskTimeStatsType) {
        case TaskTimeStatsType::LIGHT_STATISTICS:
            taskTimeStats_ = new LightTaskTimeTimeStats(MAX_WORKER_COUNT);
            break;
        case TaskTimeStatsType::NO_STATISTICS:
            break;
        default:
            UNREACHABLE();
    }
    for (size_t i = 0; i < MAX_COUNT_OF_QUEUE; i++) {
        // Atomic with relaxed order reason: no order dependency with another variables
        queues_[i].store(nullptr, std::memory_order_relaxed);
    }
    checkIfTimerThreadExists_ = [] { return TaskManager::IsTimerThreadEnabled(); };
}

TaskQueueSet::~TaskQueueSet()
{
    for (size_t i = 0; i < MAX_COUNT_OF_QUEUE; i++) {
        // Atomic with relaxed order reason: no order dependency with another variables
        ASSERT_PRINT(queues_[i].load(std::memory_order_relaxed) == nullptr, "Queue was not deleted" << i);
    }
    if (taskTimeStats_ != nullptr) {
        auto stats = taskTimeStats_->GetTaskStatistics();
        for (const auto &s : stats) {
            LOG(INFO, TASK_MANAGER) << s;
        }
    }
    while (!deleterQueue_.empty()) {
        auto deleter = deleterQueue_.front();
        deleter();
        deleterQueue_.pop();
    }
    delete taskTimeStats_;
}

TaskQueueInterface *TaskQueueSet::GetQueue(QueueId id)
{
    TASK_MANAGER_CHECK_ID_VALUE(id);
    // Atomic with relaxed order reason: no order dependency with another variables
    return queues_[id].load(std::memory_order_relaxed);
}

TaskQueueInterface *TaskQueueSet::SelectQueue()
{
    // Collect all non-deleted queues
    std::array<TaskQueueInterface *, MAX_COUNT_OF_QUEUE> queues {};
    size_t countOfQueues = 0;
    size_t prioritySum = 0;
    for (size_t i = 0; i < MAX_COUNT_OF_QUEUE; i++) {
        // Atomic with acquire order reason: need to guaranty visibility of queue init
        auto *queue = queues_[i].load(std::memory_order_acquire);
        if (queue == nullptr || queue->IsEmpty()) {
            continue;
        }
        queues[countOfQueues] = queue;
        prioritySum += queue->GetPriority();
        countOfQueues++;
    }
    if (prioritySum == 0) {
        return nullptr;
    }
    // Select queue here
    // Atomic with relaxed order reason: no order dependency with another variables
    auto selectionIndex = selectionIndex_.fetch_add(1U, std::memory_order_relaxed) % prioritySum;
    for (size_t i = 0; i < countOfQueues; i++) {
        auto *queue = queues[i];
        auto priority = queue->GetPriority();
        if (selectionIndex < priority) {
            return queue;
        }
        selectionIndex -= priority;
    }

    return nullptr;
}

bool TaskQueueSet::AreQueuesEmpty() const
{
    for (size_t i = 0; i < MAX_COUNT_OF_QUEUE; i++) {
        // Atomic with acquire order reason: need to guaranty visibility of queue init
        auto *queue = queues_[i].load(std::memory_order_acquire);
        if (queue == nullptr) {
            continue;
        }
        if (!queue->IsEmpty()) {
            return false;
        }
    }
    return true;
}

size_t TaskQueueSet::GetCountOfLiveTasks() const
{
    size_t count = 0;
    for (size_t i = 0; i < MAX_COUNT_OF_QUEUE; i++) {
        // Atomic with acquire order reason: need to guaranty visibility of queue init
        auto *queue = queues_[i].load(std::memory_order_acquire);
        if (queue == nullptr) {
            continue;
        }
        auto *iQueue = static_cast<SchedulableTaskQueueInterface *>(queue);
        count += iQueue->GetCountOfLiveTasks();
    }
    return count;
}

TaskTimeStatsBase *TaskQueueSet::GetTaskTimeStats() const
{
    return taskTimeStats_;
}

}  // namespace ark::taskmanager::internal
