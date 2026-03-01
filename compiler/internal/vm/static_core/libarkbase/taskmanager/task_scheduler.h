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

#ifndef PANDA_LIBPANDABASE_TASKMANAGER_TASK_MANAGER_H
#define PANDA_LIBPANDABASE_TASKMANAGER_TASK_MANAGER_H

#include "libarkbase/taskmanager/task_queue.h"
#include "libarkbase/taskmanager/utils/wait_list.h"
#include "libarkbase/taskmanager/worker_thread.h"
#include "libarkbase/taskmanager/utils/task_time_stats.h"
#include <vector>
#include <map>
#include <queue>

namespace ark::taskmanager {
class TaskQueueInterface;
}  // namespace ark::taskmanager

namespace ark::taskmanager::internal {

class TaskScheduler {
public:
    TaskScheduler(size_t workersCount, TaskWaitList *waitList, TaskQueueSet *queueSet);
    PANDA_PUBLIC_API ~TaskScheduler();
    NO_COPY_SEMANTIC(TaskScheduler);
    NO_MOVE_SEMANTIC(TaskScheduler);

    static constexpr uint64_t TASK_WAIT_TIMEOUT = 1U;

    using LocalTaskQueue = std::queue<Task>;

    /// @brief Returns true if TaskScheduler outputs log info
    PANDA_PUBLIC_API bool IsTaskLifetimeStatisticsUsed() const;

    /**
     * @brief Fills @arg worker (local queues) with tasks. The number of tasks obtained depends on the max size of the
     * worker's local queue and the number of workers. The algorithm strives to give the same number of tasks to all
     * workers. If queues are empty and TaskScheduler is not destroying workers will wait.  If it's true, workers should
     * finish after the execution of tasks.
     * @param worker - pointer on worker that should be fill will tasks.
     * @returns true if Worker should finish loop execution, otherwise returns false
     */
    bool FillWithTasks(WorkerThread *worker);

    /**
     * @brief Method steal task from worker with the largest number of tasks and push it to gotten worker.
     * @param worker: pointer to WorkerThread that should be fill with stollen task
     */
    size_t StealTaskFromOtherWorker(WorkerThread *taskReceiver);

    /// @brief Checks if task queues are empty
    bool AreQueuesEmpty() const;

    /// @brief Checks if worker local queues are empty
    bool AreWorkersEmpty() const;

    /**
     * @brief Executes tasks with specific properties. It will get them from queue or steal from workers.
     * @param properties - TaskProperties of tasks needs to help
     * @returns real count of tasks that was executed
     */
    PANDA_PUBLIC_API size_t HelpWorkersWithTasks(TaskQueueInterface *queue);

    /**
     * @brief Adds the task to the wait list with timeout. After the timeout expires, the task will be added
     * to its corresponding TaskQueue.
     * @param task: instance of task
     * @param time: waiting time in milliseconds
     * @returns unique waiter id. It can be used to signal wait list to add task to TaskQueue
     */
    PANDA_PUBLIC_API WaiterId AddTaskToWaitListWithTimeout(Task &&task, uint64_t time);

    /**
     * @brief Adds the task to the wait list.
     * @param task: instance of task
     * @returns unique waiter id. It can be used to signal wait list to add task to TaskQueue
     * @see TaskScheduler::SignalWaitList
     */
    PANDA_PUBLIC_API WaiterId AddTaskToWaitList(Task &&task);

    /**
     * @brief Signals wait list to add task in TaskQueue.
     * @param waiterId: unique waiter id
     * @see TaskScheduler::AddTaskToWaitListWithTimeout, TaskScheduler::AddTaskToWaitList
     */
    PANDA_PUBLIC_API void SignalWaitList(WaiterId waiterId);

    PANDA_PUBLIC_API size_t GetCountOfTasksInSystem() const;
    /// @brief Method signals workers if it's needed
    void SignalWorkers();

    PANDA_PUBLIC_API size_t GetCountOfWorkers() const;
    PANDA_PUBLIC_API void SetCountOfWorkers(size_t count);

private:
    /**
     * @brief Method get and execute tasks with specified properties. If there are no tasks with that properties method
     * will return nullopt.
     * @param properties - TaskProperties of task we want to get.
     * @returns real count of gotten tasks
     */
    size_t GetAndExecuteSetOfTasksFromQueue(TaskQueueInterface *properties);

    /**
     * @brief Method steal and execute one task from one Worker. Method will find worker the largest number of tasks,
     * steal one from it and execute.
     * @param properties - TaskProperties of tasks needs to help
     * @returns 1 if stealing was done successfully
     */
    size_t StealAndExecuteOneTaskFromWorkers(TaskQueueInterface *properties);

    size_t PutTasksInWorker(WorkerThread *worker, internal::SchedulableTaskQueueInterface *queue);

    /// @brief Checks if there are no tasks in queues and workers
    bool AreNoMoreTasks() const;

    void PutWaitTaskInLocalQueue(LocalTaskQueue &queue) REQUIRES(taskSchedulerStateLock_);

    void PutTaskInTaskQueues(LocalTaskQueue &queue);

    /**
     * @brief Method waits until new tasks coming or finishing of Task Scheduler usage
     * @return true if TaskScheduler have tasks to manager
     */
    bool WaitUntilNewTasks(WorkerThread *worker);

    size_t ProcessWaitList();

    TaskWaitList *waitList_ = nullptr;
    TaskQueueSet *queueSet_ = nullptr;
    std::atomic_bool waitListIsProcessing_ {false};

    /// Pointers to Worker Threads.
    std::array<std::atomic<WorkerThread *>, MAX_WORKER_COUNT> workers_ {};
    std::atomic_size_t workersCount_ = {0UL};

    /// Iterator for workers_ to balance stealing
    size_t workersIterator_ = {0UL};

    /// Represents count of task that sleeps
    std::atomic_size_t waitWorkersCount_ {0UL};

    /// task_scheduler_state_lock_ is used to check state of task
    os::memory::RecursiveMutex mutable taskSchedulerStateLock_;

    /**
     * queues_wait_cond_var_ is used when all registered queues are empty to wait until one of them will have a
     * task
     */
    os::memory::ConditionVariable queuesWaitCondVar_ GUARDED_BY(taskSchedulerStateLock_);

    /// start_ is true if we used Initialize method
    std::atomic_bool start_ {false};

    /// finish_ is true when TaskScheduler finish Workers and TaskQueues
    bool finish_ GUARDED_BY(taskSchedulerStateLock_) {false};

    std::atomic_size_t waitToFinish_ {0UL};
    std::vector<WorkerThread *> workersToDelete_;
};

}  // namespace ark::taskmanager::internal

#endif  // PANDA_LIBPANDABASE_TASKMANAGER_TASK_MANAGER_H
