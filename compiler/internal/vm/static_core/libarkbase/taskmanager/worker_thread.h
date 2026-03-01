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

#ifndef PANDA_LIBPANDABASE_TASKMANAGER_WORKER_THREAD_H
#define PANDA_LIBPANDABASE_TASKMANAGER_WORKER_THREAD_H

#include "libarkbase/taskmanager/schedulable_task_queue_interface.h"
#include "libarkbase/taskmanager/utils/two_lock_queue.h"
#include "libarkbase/os/mutex.h"
#include <thread>

namespace ark::taskmanager::internal {

class TaskScheduler;
class TaskQueueSet;

class WorkerThread {
public:
    NO_COPY_SEMANTIC(WorkerThread);
    NO_MOVE_SEMANTIC(WorkerThread);

    static constexpr size_t WORKER_QUEUE_SIZE = 4UL;

    WorkerThread(TaskScheduler *scheduler, TaskTimeStatsBase *taskTimeStats, const std::string &name);
    ~WorkerThread();

    void AddForegroundTask(TaskPtr task);
    void AddBackgroundTask(TaskPtr task);
    /// @brief Returns true if all internal queues are empty
    bool IsEmpty() const;
    /// @brief Returns count of tasks in local queue
    size_t Size() const;

    TaskPtr PopTask();
    TaskPtr PopForegroundTask();
    TaskPtr PopBackgroundTask();
    /// @brief Waits for worker finish
    void Join();

    std::string GetWorkerName() const;
    /// @brief method starts WorkerLoop. All workers should be registered before Start executing
    void Start();

    void SetFinish()
    {
        // Atomic with relaxed order reason: no order requirement
        finish_.store(true, std::memory_order_relaxed);
    }

    bool CheckFinish() const
    {
        // Atomic with relaxed order reason: no order requirement
        return finish_.load(std::memory_order_relaxed);
    }

private:
    /// @brief Main workers algorithm
    void WorkerLoop();
    /**
     * @brief pops and executes all tasks from internal queue.
     * Also counts executed tasks in finishedTasksCounterMap_.
     */
    size_t ExecuteTasksFromLocalQueue();
    /**
     * @brief method wait for starting
     * @see Start
     */
    void WaitForStart();

    using SelfTaskQueue = TwoLockQueue<Task, std::allocator<Task>, QueueElemAllocationType::OUTSIDE>;
    SelfTaskQueue foregroundTaskQueue_;
    SelfTaskQueue backgroundTaskQueue_;
    std::thread *thread_ = nullptr;
    TaskScheduler *scheduler_ = nullptr;
    TaskTimeStatsBase *taskTimeStats_ = nullptr;
    std::string name_;
    os::memory::Mutex startWaitLock_;
    os::memory::ConditionVariable startWaitCondVar_ GUARDED_BY(startWaitLock_);
    bool start_ {false};
    std::atomic_bool finish_ {false};
};

}  // namespace ark::taskmanager::internal

#endif  // PANDA_LIBPANDABASE_TASKMANAGER_WORKER_THREAD_H
