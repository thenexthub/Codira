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

#ifndef LIBPANDABASE_TASKMANAGER_TASK_QUEUE_SET_H
#define LIBPANDABASE_TASKMANAGER_TASK_QUEUE_SET_H

#include <queue>
#include "libarkbase/taskmanager/task_queue.h"

namespace ark::taskmanager::internal {

class TaskScheduler;

class TaskQueueSet {
public:
    explicit TaskQueueSet(TaskWaitList *waitList,
                          TaskTimeStatsType taskTimeStatsType = TaskTimeStatsType::NO_STATISTICS);
    ~TaskQueueSet();
    NO_COPY_SEMANTIC(TaskQueueSet);
    NO_MOVE_SEMANTIC(TaskQueueSet);

    template <class Allocator>
    TaskQueueInterface *CreateQueue(QueuePriority priority);
    template <class Allocator>
    void DeleteQueue(TaskQueueInterface *queue);
    TaskQueueInterface *GetQueue(QueueId id);
    TaskQueueInterface *SelectQueue();
    bool AreQueuesEmpty() const;
    size_t GetCountOfLiveTasks() const;

    void SetCallbacks(std::function<void()> signalWorkersCallback)
    {
        signalWorkersCallback_ = std::move(signalWorkersCallback);
    }
    TaskTimeStatsBase *GetTaskTimeStats() const;

private:
    TaskWaitList *waitList_ = nullptr;
    std::function<void()> signalWorkersCallback_;
    std::function<void()> signalWaitersCallback_;

    std::function<bool()> checkIfTimerThreadExists_;

    TaskTimeStatsBase *taskTimeStats_ = nullptr;
    std::atomic_size_t selectionIndex_ {0};
    std::array<std::atomic<TaskQueueInterface *>, MAX_COUNT_OF_QUEUE> queues_ {};
    std::queue<std::function<void()>> deleterQueue_;
};

template <class Allocator>
// CC-OFFNXT(G.FUD.06) Splitting this function will degrade readability. Keyword "inline" needs to satisfy ODR rule.
inline TaskQueueInterface *TaskQueueSet::CreateQueue(QueuePriority priority)
{
    size_t i = 0;
    auto *queue = internal::TaskQueue<Allocator>::Create(priority, waitList_, taskTimeStats_);
    ASSERT(signalWorkersCallback_ != nullptr);
    queue->SetCallbacks(signalWorkersCallback_, checkIfTimerThreadExists_);
    while (i != MAX_COUNT_OF_QUEUE) {
        for (i = 0; i < MAX_COUNT_OF_QUEUE; i++) {
            // Atomic with relaxed order reason: no order dependency with another variables
            if (queues_[i].load(std::memory_order_relaxed) != nullptr) {
                continue;
            }
            queue->Register(i);
            TaskQueueInterface *nullp = nullptr;
            // Atomic with release order reason: need guaranty that init of queue will be visible. For more info
            // https://www.open-std.org/jtc1/sc22/wg21/docs/cwg_defects.html#710
            if (!queues_[i].compare_exchange_weak(nullp, queue, std::memory_order_release)) {
                break;
            }
            return queue;
        }
    }
    internal::TaskQueue<Allocator>::Destroy(queue);
    return nullptr;
}

template <class Allocator>
inline void TaskQueueSet::DeleteQueue(TaskQueueInterface *queue)
{
    while (!queue->IsEmpty()) {
        queue->ExecuteTask();
    }
    queue->WaitTasks();
    auto id = queue->GetQueueId();
    // Atomic with relaxed order reason: no order dependency with another variables
    ASSERT(queues_[id].load(std::memory_order_relaxed) == queue);
    // Atomic with relaxed order reason: no order dependency with another variables
    queues_[id].store(nullptr, std::memory_order_relaxed);
    deleterQueue_.push([queue] {
        internal::TaskQueue<Allocator>::Destroy(static_cast<internal::SchedulableTaskQueueInterface *>(queue));
    });
}

}  // namespace ark::taskmanager::internal

#endif  // LIBPANDABASE_TASKMANAGER_TASK_QUEUE_SET_H
