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

#ifndef LIBPANDABASE_TASKMANAGER_TASK_MANAGER_H
#define LIBPANDABASE_TASKMANAGER_TASK_MANAGER_H

#include "libarkbase/taskmanager/task_queue_set.h"
#include "libarkbase/taskmanager/task_scheduler.h"
#include "libarkbase/taskmanager/timer_thread.h"
#include "libarkbase/macros.h"

namespace ark::taskmanager {

using TaskPtr = internal::TaskPtr;

/**
 * @brief TaskManager it's singletone that provide interfaces for usage of multithreading subsystem. You can create
 * TaskQueue by CreateTaskQueue method and use it to push-in your functors.
 *
 * Base usage scenario:
 * 1. You start scope of TaskManager usage this TaskManager::Start method;
 * 2. Next you create queue for pushing tasks. Use TaskManager::CreateTaskQueue method;
 * 3. Now you are free to use mutlithreading subsutem. Try to reduce usage of sync tools like mutexes in your tasks;
 * 4. If you want to finish usage of TaskManager, first you need to be sure that all queues are empty.
 *    You can do this only by wait method in TaskQueue;
 * 5. When you are sure that queues are empty use TaskManager:Finish to finilize usage of threads.
 */
class TaskManager {
public:
    /**
     * @brief Method creates task manager with specified count of workers. Also you can specify usage of execution time
     * stats collection. Method creates count of workers that can execute tasks from TaskQueue. This method can be
     * called next time only after execution of the TaskManager::Finish() method.
     * This method does not enable usage of wait list methods in TaskQueue. Please see TaskManager::EnableTimerThread()
     * for details.
     * @see TaskManager::Finish();
     * @see TaskManager::EnableTimerThread();
     */
    static PANDA_PUBLIC_API void Start(size_t workerCount,
                                       TaskTimeStatsType statsType = TaskTimeStatsType::NO_STATISTICS);
    /**
     * @brief Method finishes usage of TaskManager. All TaskQueues should be deleted before calling. This method can be
     * called after TaskManager::Start(...) method.
     * If you have used TaskManager::EnableTimerThread() you should execute TaskManager::DisableTimerThread() before
     * this method.
     * @see TaskManager::Start(...);
     * @see TaskManager::DisableTimerThread();
     */
    static PANDA_PUBLIC_API void Finish();
    /// @returns true if task manager exists, otherwise false
    static PANDA_PUBLIC_API bool IsUsed();
    /**
     * @brief Method allocates TaskQueue. It's importent that lifetime of Allocator should be more then TaskMananger
     * usage scope!
     * @return TaskQueueInterface pointer that can be used to interact with TaskQueue. This pointer should be freed by
     * TaskManager::DestroyTaskQueue method.
     * @see class TaskQueueInterface, @see TaskManager::DestroyTaskQueue
     */
    template <class Allocator = std::allocator<TaskPtr>>
    static PANDA_PUBLIC_API TaskQueueInterface *CreateTaskQueue(QueuePriority priority = DEFAULT_QUEUE_PRIORITY);
    /// @brief Method returns pointer to TaskQueue by queue id.
    static PANDA_PUBLIC_API TaskQueueInterface *GetTaskQueue(QueueId id);
    /**
     * @brief Method creates callback that will delete queue. It's importent that lifetime of Allocator should be more
     * then TaskMananger usage scope!
     */
    template <class Allocator = std::allocator<TaskPtr>>
    static PANDA_PUBLIC_API void DestroyTaskQueue(TaskQueueInterface *queue);
    /**
     * @brief Method chages count of currect workers.
     *  - if new count is more then MAX_WORKER_COUNT, method uses count equel to MAX_WORKER_COUNT
     *  - if new count is more then current one, method will create new workers;
     *  - if new count is less then current one, method will wait for finish excess workers;
     *  - if new count is equel to current one, method to nothing;
     *  - if you set 0, all workers will be disabled, and existed tasks will wait in queues.
     */
    static PANDA_PUBLIC_API void SetWorkersCount(size_t count);
    /// @returns count of workers
    static PANDA_PUBLIC_API size_t GetWorkersCount();
    /**
     * @brief Method starts TimerThread utility in TaskManager. If you want use methods like
     * TaskQueue::AddForegroundTaskInWaitList, you need to execute this method before. Otherwise they will return
     * invalid index. Also you should disable TimerThread with TaskManager::DisableTimerThread() before
     * TaskQueue::Finish().
     * @see TaskQueue::AddForegroundTaskInWaitList();
     * @see TaskManager::DisableTimerThread();
     */
    static PANDA_PUBLIC_API void EnableTimerThread();
    /**
     * @brief Method finishes usage of TimerThread in TaskManager. After its exectuion methods like
     * TaskQueue::AddForegroundTaskInWaitList will return invalid index.
     * @see TaskQueue::AddForegroundTaskInWaitList();
     */
    static PANDA_PUBLIC_API void DisableTimerThread();

    static PANDA_PUBLIC_API bool IsTimerThreadEnabled();

    NO_COPY_SEMANTIC(TaskManager);
    NO_MOVE_SEMANTIC(TaskManager);

private:
    TaskManager(size_t workerCount, TaskTimeStatsType statsType);
    ~TaskManager() = default;

    TaskWaitList waitList_;
    internal::TimerThread timerThread_;
    internal::TaskQueueSet queueSet_;
    internal::TaskScheduler scheduler_;

    PANDA_PUBLIC_API static TaskManager *inst_;
};

template <class Allocator>
inline PANDA_PUBLIC_API TaskQueueInterface *TaskManager::CreateTaskQueue(QueuePriority priority)
{
    ASSERT(inst_ != nullptr);
    return inst_->queueSet_.CreateQueue<Allocator>(priority);
}

template <class Allocator>
inline PANDA_PUBLIC_API void TaskManager::DestroyTaskQueue(TaskQueueInterface *queue)
{
    ASSERT(inst_ != nullptr);
    inst_->queueSet_.DeleteQueue<Allocator>(queue);
}

}  // namespace ark::taskmanager

#endif  // LIBPANDABASE_TASKMANAGER_TASK_MANAGER_H
