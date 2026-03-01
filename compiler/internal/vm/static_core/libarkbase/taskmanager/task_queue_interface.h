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

#ifndef PANDA_LIBPANDABASE_TASKMANAGER_TASK_QUEUE_INTERFACE_H
#define PANDA_LIBPANDABASE_TASKMANAGER_TASK_QUEUE_INTERFACE_H

#include <atomic>

#include "libarkbase/taskmanager/task_manager_common.h"
#include "libarkbase/taskmanager/utils/wait_list.h"

namespace ark::taskmanager {

/**
 * @brief TaskQueueInteface is an interface of push-thread-safe queue for tasks. Queues can be registered in
 * TaskScheduler and used to execute tasks on workers. Also, queues can notify other threads when a new task is pushed.
 */
class TaskQueueInterface {
public:
    NO_COPY_SEMANTIC(TaskQueueInterface);
    NO_MOVE_SEMANTIC(TaskQueueInterface);

    PANDA_PUBLIC_API explicit TaskQueueInterface(QueuePriority priority = DEFAULT_QUEUE_PRIORITY) : priority_(priority)
    {
        TASK_MANAGER_CHECK_PRIORITY_VALUE(priority);
    }
    PANDA_PUBLIC_API virtual ~TaskQueueInterface() = default;

    /**
     * Method pushes runner in TaskQueue as foreground task.
     * @return current count of foreground tasks in queue
     */
    PANDA_PUBLIC_API virtual size_t AddForegroundTask(RunnerCallback runner) = 0;
    /**
     * Method pushes runner in TaskQueue as background task.
     * @return current count of background tasks in queue
     */
    PANDA_PUBLIC_API virtual size_t AddBackgroundTask(RunnerCallback runner) = 0;

    /**
     * Method adds your foreground RunnerCallback in WaitList with delay you specify. If you did not enable usage of
     * TimerThread this method will return INVALID_WAITER_ID.
     * @param runner - functor use want ot adds;
     * @param timeToWait - the time for which the task will be postponed;
     * @return WaiterId associated with the pending runner. If TimerThread is disabled returns invalid WaiterId.
     * @see TaskManager::EnableTimerThread;
     */
    PANDA_PUBLIC_API virtual WaiterId AddForegroundTaskInWaitList(RunnerCallback runner, uint64_t timeToWait) = 0;
    /**
     * Method adds your background RunnerCallback in WaitList with delay you specify. If you did not enable usage of
     * TimerThread this method will return INVALID_WAITER_ID.
     * @param runner - functor use want ot adds;
     * @param timeToWait - the time for which the task will be postponed;
     * @return WaiterId associated with the pending runner. If TimerThread is disabled returns invalid WaiterId.
     * @see TaskManager::EnableTimerThread;
     */
    PANDA_PUBLIC_API virtual WaiterId AddBackgroundTaskInWaitList(RunnerCallback runner, uint64_t timeToWait) = 0;

    /**
     * Method adds your foreground RunnerCallback in WaitList. You should notify WaitList with
     * TaskQueue::SignalWaitList() to release runner. If you did not enable usage of TimerThread this method will return
     * INVALID_WAITER_ID.
     * @param runner - functor use want ot adds;
     * @param timeToWait - the time for which the task will be postponed;
     * @return WaiterId associated with the pending runner. If TimerThread is disabled returns invalid WaiterId.
     * @see TaskManager::EnableTimerThread;
     */
    PANDA_PUBLIC_API virtual WaiterId AddForegroundTaskInWaitList(RunnerCallback runner) = 0;
    /**
     * Method adds your background RunnerCallback in WaitList. You should notify WaitList with
     * TaskQueue::SignalWaitList() to release runner. If you did not enable usage of TimerThread this method will return
     * INVALID_WAITER_ID.
     * @param runner - functor use want ot adds;
     * @param timeToWait - the time for which the task will be postponed;
     * @return WaiterId associated with the pending runner. If TimerThread is disabled returns invalid WaiterId.
     * @see TaskManager::EnableTimerThread;
     */
    PANDA_PUBLIC_API virtual WaiterId AddBackgroundTaskInWaitList(RunnerCallback runner) = 0;

    /**
     * Method notifies WaitList to release runner associated with inputed WaiterId. If you did not enable usage of
     * TimerThread method will do nothing.
     * @param id - WaiterId value associated with runner you want to release
     * @see TaskManager::EnableTimerThread;
     */
    PANDA_PUBLIC_API virtual void SignalWaitList(WaiterId id) = 0;

    [[nodiscard]] PANDA_PUBLIC_API virtual bool IsEmpty() const = 0;
    [[nodiscard]] PANDA_PUBLIC_API virtual bool HasForegroundTasks() const = 0;
    [[nodiscard]] PANDA_PUBLIC_API virtual bool HasBackgroundTasks() const = 0;

    [[nodiscard]] PANDA_PUBLIC_API virtual size_t Size() const = 0;
    [[nodiscard]] PANDA_PUBLIC_API virtual size_t CountOfForegroundTasks() const = 0;
    [[nodiscard]] PANDA_PUBLIC_API virtual size_t CountOfBackgroundTasks() const = 0;

    PANDA_PUBLIC_API size_t virtual ExecuteTask() = 0;
    PANDA_PUBLIC_API size_t virtual ExecuteForegroundTask() = 0;
    PANDA_PUBLIC_API size_t virtual ExecuteBackgroundTask() = 0;

    PANDA_PUBLIC_API void virtual WaitTasks() = 0;
    PANDA_PUBLIC_API void virtual WaitForegroundTasks() = 0;
    PANDA_PUBLIC_API void virtual WaitBackgroundTasks() = 0;

    void SetPriority(QueuePriority priority);
    QueuePriority GetPriority() const;

    void Register(QueueId id);
    QueueId GetQueueId() const;

private:
    QueueId id_ = INVALID_ID;
    std::atomic<QueuePriority> priority_;
};

inline QueuePriority TaskQueueInterface::GetPriority() const
{
    // Atomic with relaxed order reason: no order dependency with another variables
    return priority_.load(std::memory_order_relaxed);
}

inline void TaskQueueInterface::SetPriority(QueuePriority priority)
{
    TASK_MANAGER_CHECK_PRIORITY_VALUE(priority);
    // Atomic with relaxed order reason: no order dependency with another variables
    priority_.store(priority, std::memory_order_relaxed);
}

inline QueueId TaskQueueInterface::GetQueueId() const
{
    return id_;
}

inline void TaskQueueInterface::Register(QueueId id)
{
    id_ = id;
}

}  // namespace ark::taskmanager

#endif  // PANDA_LIBPANDABASE_TASKMANAGER_TASK_QUEUE_INTERFACE_H
