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

#ifndef PANDA_LIBPANDABASE_TASKMANAGER_TASK_QUEUE_H
#define PANDA_LIBPANDABASE_TASKMANAGER_TASK_QUEUE_H

#include <atomic>
#include <utility>
#include "libarkbase/os/mutex.h"
#include "libarkbase/taskmanager/schedulable_task_queue_interface.h"
#include "libarkbase/taskmanager/utils/task_time_stats.h"
#include "libarkbase/taskmanager/task.h"
#include "libarkbase/taskmanager/utils/two_lock_queue.h"
#include "libarkbase/taskmanager/utils/wait_list.h"

namespace ark::taskmanager::internal {

/**
 * @brief TaskQueue is a thread-safe queue for tasks. Queues can be registered in TaskScheduler and used to execute
 * tasks on workers. Also, queues can notify other threads when a new task is pushed.
 * @tparam Allocator - allocator of Task that will be used in internal queues. By default is used
 * std::allocator<Task>
 */
template <class Allocator = std::allocator<Task>>
class TaskQueue : public SchedulableTaskQueueInterface {
    using TaskAllocatorType = typename Allocator::template rebind<Task>::other;
    using TaskQueueAllocatorType = typename Allocator::template rebind<TaskQueue<TaskAllocatorType>>::other;
    template <class OtherAllocator>
    friend class TaskQueue;

public:
    NO_COPY_SEMANTIC(TaskQueue);
    NO_MOVE_SEMANTIC(TaskQueue);

    static PANDA_PUBLIC_API SchedulableTaskQueueInterface *Create(QueuePriority priority, TaskWaitList *waitList,
                                                                  TaskTimeStatsBase *taskTimeStats);
    static PANDA_PUBLIC_API void Destroy(SchedulableTaskQueueInterface *queue);

    PANDA_PUBLIC_API size_t AddForegroundTask(RunnerCallback runner) override;
    PANDA_PUBLIC_API size_t AddBackgroundTask(RunnerCallback runner) override;

    PANDA_PUBLIC_API WaiterId AddForegroundTaskInWaitList(RunnerCallback runtime, uint64_t timeToWait) override;
    PANDA_PUBLIC_API WaiterId AddBackgroundTaskInWaitList(RunnerCallback runtime, uint64_t timeToWait) override;

    PANDA_PUBLIC_API WaiterId AddForegroundTaskInWaitList(RunnerCallback runtime) override;
    PANDA_PUBLIC_API WaiterId AddBackgroundTaskInWaitList(RunnerCallback runtime) override;

    PANDA_PUBLIC_API void SignalWaitList(WaiterId id) override;

    [[nodiscard]] PANDA_PUBLIC_API bool IsEmpty() const override;
    [[nodiscard]] PANDA_PUBLIC_API bool HasForegroundTasks() const override;
    [[nodiscard]] PANDA_PUBLIC_API bool HasBackgroundTasks() const override;

    [[nodiscard]] PANDA_PUBLIC_API size_t Size() const override;
    [[nodiscard]] PANDA_PUBLIC_API size_t CountOfForegroundTasks() const override;
    [[nodiscard]] PANDA_PUBLIC_API size_t CountOfBackgroundTasks() const override;

    PANDA_PUBLIC_API size_t ExecuteTask() override;
    PANDA_PUBLIC_API size_t ExecuteForegroundTask() override;
    PANDA_PUBLIC_API size_t ExecuteBackgroundTask() override;

    PANDA_PUBLIC_API void WaitTasks() override;
    PANDA_PUBLIC_API void WaitForegroundTasks() override;
    PANDA_PUBLIC_API void WaitBackgroundTasks() override;

    [[nodiscard]] TaskPtr PopTask() override;
    [[nodiscard]] TaskPtr PopForegroundTask() override;
    [[nodiscard]] TaskPtr PopBackgroundTask() override;

    size_t PopTasksToWorker(const AddTaskToWorkerFunc &addForegroundTaskFunc,
                            const AddTaskToWorkerFunc &addBackgroundTaskFunc, size_t size) override;
    size_t PopForegroundTasksToHelperThread(const AddTaskToHelperFunc &addTaskFunc, size_t size) override;
    size_t PopBackgroundTasksToHelperThread(const AddTaskToHelperFunc &addTaskFunc, size_t size) override;

    size_t GetCountOfLiveTasks() const override;
    size_t GetCountOfLiveForegroundTasks() const override;
    size_t GetCountOfLiveBackgroundTasks() const override;

    TaskTimeStatsBase *GetTaskTimeStats() const override;

    void SetCallbacks(SignalWorkersCallback signalWorkersCallback,
                      CheckIfTimerThreadIsEnabledCallback checkTimerThreadExistCallback) override;
    void UnsetCallbacks() override;

private:
    using InternalTaskQueue = TwoLockQueue<Task, TaskAllocatorType, QueueElemAllocationType::INPLACE>;

    PANDA_PUBLIC_API size_t AddForegroundTaskImpl(RunnerCallback &&runner);
    PANDA_PUBLIC_API size_t AddBackgroundTaskImpl(RunnerCallback &&runner);

    PANDA_PUBLIC_API size_t IncrementCountOfLiveForegroundTasks();
    PANDA_PUBLIC_API size_t IncrementCountOfLiveBackgroundTasks();

    static void OnForegroundTaskDestructionCallback(TaskQueueInterface *queue, void *node);
    static void OnBackgroundTaskDestructionCallback(TaskQueueInterface *queue, void *node);

    TaskQueue(QueuePriority priority, TaskWaitList *waitList, TaskTimeStatsBase *taskTimeStats)
        : SchedulableTaskQueueInterface(priority), taskTimeStats_(taskTimeStats), waitList_(waitList)
    {
    }
    PANDA_PUBLIC_API ~TaskQueue() override
    {
        ASSERT(foregroundTaskQueue_.IsEmpty() && backgroundTaskQueue_.IsEmpty());
    }

    /// subscriber_lock_ is used in case of calling new_tasks_callback_
    SignalWorkersCallback signalWorkersCallback_;
    CheckIfTimerThreadIsEnabledCallback checkTimerThreadExistCallback_;
    TaskTimeStatsBase *taskTimeStats_ {nullptr};

    os::memory::Mutex waitingMutex_;
    os::memory::ConditionVariable waitingCondVar_;

    /// foreground part of TaskQueue
    std::atomic_size_t foregroundLiveTasks_ {0};
    InternalTaskQueue foregroundTaskQueue_;
    /// background part of TaskQueue
    std::atomic_size_t backgroundLiveTasks_ {0};
    InternalTaskQueue backgroundTaskQueue_;

    TaskWaitList *waitList_ = nullptr;
};

template <class Allocator>
inline SchedulableTaskQueueInterface *TaskQueue<Allocator>::Create(QueuePriority priority, TaskWaitList *waitList,
                                                                   TaskTimeStatsBase *taskTimeStats)
{
    TaskQueueAllocatorType allocator;
    auto *mem = allocator.allocate(1U);
    return new (mem) TaskQueue<TaskAllocatorType>(priority, waitList, taskTimeStats);
}

template <class Allocator>
inline void TaskQueue<Allocator>::Destroy(SchedulableTaskQueueInterface *queue)
{
    TaskQueueAllocatorType allocator;
    std::allocator_traits<TaskQueueAllocatorType>::destroy(allocator, queue);
    allocator.deallocate(static_cast<TaskQueue<TaskAllocatorType> *>(queue), 1U);
}

template <class Allocator>
inline void TaskQueue<Allocator>::OnForegroundTaskDestructionCallback(TaskQueueInterface *queue, void *node)
{
    InternalTaskQueue::TryDeleteNode(node);
    auto iQueue = reinterpret_cast<TaskQueue *>(queue);
    // Atomic with relaxed order reason: all non-atomic and relaxed stores will be see after waitingMutex_ getting
    auto aliveTasks = iQueue->foregroundLiveTasks_.fetch_sub(1U, std::memory_order_relaxed);
    if (aliveTasks == 1U) {
        os::memory::LockHolder lh(iQueue->waitingMutex_);
        iQueue->waitingCondVar_.SignalAll();
    }
}

template <class Allocator>
inline void TaskQueue<Allocator>::OnBackgroundTaskDestructionCallback(TaskQueueInterface *queue, void *node)
{
    InternalTaskQueue::TryDeleteNode(node);
    auto iQueue = reinterpret_cast<TaskQueue *>(queue);
    // Atomic with relaxed order reason: all non-atomic and relaxed stores will be see after waitingMutex_ getting
    auto aliveTasks = iQueue->backgroundLiveTasks_.fetch_sub(1U, std::memory_order_relaxed);
    if (aliveTasks == 1U) {
        os::memory::LockHolder lh(iQueue->waitingMutex_);
        iQueue->waitingCondVar_.SignalAll();
    }
}

template <class Allocator>
inline size_t TaskQueue<Allocator>::AddForegroundTask(RunnerCallback runner)
{
    IncrementCountOfLiveForegroundTasks();
    return AddForegroundTaskImpl(std::move(runner));
}

template <class Allocator>
inline size_t TaskQueue<Allocator>::AddBackgroundTask(RunnerCallback runner)
{
    IncrementCountOfLiveBackgroundTasks();
    return AddBackgroundTaskImpl(std::move(runner));
}

template <class Allocator>
inline PANDA_PUBLIC_API WaiterId TaskQueue<Allocator>::AddForegroundTaskInWaitList(RunnerCallback runner,
                                                                                   uint64_t timeToWait)
{
    if (!checkTimerThreadExistCallback_()) {
        return INVALID_WAITER_ID;
    }
    IncrementCountOfLiveForegroundTasks();
    auto waitListCallback = [this](RunnerCallback &&irunner) { AddForegroundTaskImpl(std::move(irunner)); };
    return waitList_->AddValueToWait({std::move(runner), waitListCallback}, timeToWait);
}

template <class Allocator>
inline PANDA_PUBLIC_API WaiterId TaskQueue<Allocator>::AddBackgroundTaskInWaitList(RunnerCallback runner,
                                                                                   uint64_t timeToWait)
{
    if (!checkTimerThreadExistCallback_()) {
        return INVALID_WAITER_ID;
    }
    IncrementCountOfLiveBackgroundTasks();
    auto waitListCallback = [this](RunnerCallback &&irunner) { AddBackgroundTaskImpl(std::move(irunner)); };
    return waitList_->AddValueToWait({std::move(runner), waitListCallback}, timeToWait);
}

template <class Allocator>
inline PANDA_PUBLIC_API WaiterId TaskQueue<Allocator>::AddForegroundTaskInWaitList(RunnerCallback runner)
{
    if (!checkTimerThreadExistCallback_()) {
        return INVALID_WAITER_ID;
    }
    IncrementCountOfLiveForegroundTasks();
    auto waitListCallback = [this](RunnerCallback &&irunner) { AddForegroundTaskImpl(std::move(irunner)); };
    return waitList_->AddValueToWait({std::move(runner), waitListCallback});
}

template <class Allocator>
inline PANDA_PUBLIC_API WaiterId TaskQueue<Allocator>::AddBackgroundTaskInWaitList(RunnerCallback runner)
{
    if (!checkTimerThreadExistCallback_()) {
        return INVALID_WAITER_ID;
    }
    IncrementCountOfLiveBackgroundTasks();
    auto waitListCallback = [this](RunnerCallback &&irunner) { AddBackgroundTaskImpl(std::move(irunner)); };
    return waitList_->AddValueToWait({std::move(runner), waitListCallback});
}

template <class Allocator>
void TaskQueue<Allocator>::SignalWaitList(WaiterId id)
{
    if (!checkTimerThreadExistCallback_()) {
        return;
    }
    auto waitVal = waitList_->GetValueById(id);
    if (!waitVal.has_value()) {
        return;
    }
    auto [task, taskPoster] = std::move(waitVal.value());
    taskPoster(std::move(task));
}

template <class Allocator>
inline bool TaskQueue<Allocator>::IsEmpty() const
{
    return foregroundTaskQueue_.IsEmpty() && backgroundTaskQueue_.IsEmpty();
}

template <class Allocator>
inline bool TaskQueue<Allocator>::HasForegroundTasks() const
{
    return !foregroundTaskQueue_.IsEmpty();
}

template <class Allocator>
inline bool TaskQueue<Allocator>::HasBackgroundTasks() const
{
    return !backgroundTaskQueue_.IsEmpty();
}

template <class Allocator>
inline size_t TaskQueue<Allocator>::Size() const
{
    return foregroundTaskQueue_.Size() + backgroundTaskQueue_.Size();
}

template <class Allocator>
inline size_t TaskQueue<Allocator>::CountOfForegroundTasks() const
{
    return foregroundTaskQueue_.Size();
}

template <class Allocator>
inline size_t TaskQueue<Allocator>::CountOfBackgroundTasks() const
{
    return backgroundTaskQueue_.Size();
}

template <class Allocator>
inline size_t TaskQueue<Allocator>::ExecuteTask()
{
    TaskPtr task = PopTask();
    if (task == nullptr) {
        return 0U;
    }
    task->RunTask();
    Task::Delete(task);
    return 1U;
}

template <class Allocator>
inline size_t TaskQueue<Allocator>::ExecuteForegroundTask()
{
    TaskPtr task = PopForegroundTask();
    if (task == nullptr) {
        return 0U;
    }
    task->RunTask();
    Task::Delete(task);
    return 1U;
}

template <class Allocator>
inline size_t TaskQueue<Allocator>::ExecuteBackgroundTask()
{
    TaskPtr task = PopBackgroundTask();
    if (task == nullptr) {
        return 0U;
    }
    task->RunTask();
    Task::Delete(task);
    return 1U;
}

template <class Allocator>
inline void TaskQueue<Allocator>::WaitTasks()
{
    os::memory::LockHolder lh(waitingMutex_);
    while (GetCountOfLiveBackgroundTasks() != 0 || GetCountOfLiveForegroundTasks() != 0) {
        waitingCondVar_.Wait(&waitingMutex_);
    }
}

template <class Allocator>
inline void TaskQueue<Allocator>::WaitForegroundTasks()
{
    os::memory::LockHolder lh(waitingMutex_);
    while (GetCountOfLiveForegroundTasks() != 0) {
        waitingCondVar_.Wait(&waitingMutex_);
    }
}

template <class Allocator>
inline void TaskQueue<Allocator>::WaitBackgroundTasks()
{
    os::memory::LockHolder lh(waitingMutex_);
    while (GetCountOfLiveBackgroundTasks() != 0) {
        waitingCondVar_.Wait(&waitingMutex_);
    }
}

template <class Allocator>
inline TaskPtr TaskQueue<Allocator>::PopTask()
{
    TaskPtr task = nullptr;
    if (foregroundTaskQueue_.TryPop(&task)) {
        return task;
    }
    backgroundTaskQueue_.TryPop(&task);
    return task;
}

template <class Allocator>
inline TaskPtr TaskQueue<Allocator>::PopForegroundTask()
{
    TaskPtr task = nullptr;
    foregroundTaskQueue_.TryPop(&task);
    return task;
}

template <class Allocator>
inline TaskPtr TaskQueue<Allocator>::PopBackgroundTask()
{
    TaskPtr task = nullptr;
    backgroundTaskQueue_.TryPop(&task);
    return task;
}

template <class Allocator>
// CC-OFFNXT(G.FUD.06) Splitting this function will degrade readability. Keyword "inline" needs to satisfy ODR rule.
inline size_t TaskQueue<Allocator>::PopTasksToWorker(const AddTaskToWorkerFunc &addForegroundTaskFunc,
                                                     const AddTaskToWorkerFunc &addBackgroundTaskFunc, size_t size)
{
    for (size_t i = 0; i < size; i++) {
        TaskPtr task = nullptr;
        if (foregroundTaskQueue_.TryPop(&task)) {
            addForegroundTaskFunc(task);
            continue;
        }
        if (backgroundTaskQueue_.TryPop(&task)) {
            addBackgroundTaskFunc(task);
            continue;
        }
        return i;
    }
    return size;
}

template <class Allocator>
inline size_t TaskQueue<Allocator>::PopForegroundTasksToHelperThread(const AddTaskToHelperFunc &addTaskFunc,
                                                                     size_t size)
{
    for (size_t i = 0; i < size; i++) {
        TaskPtr task = nullptr;
        if (foregroundTaskQueue_.TryPop(&task)) {
            addTaskFunc(task);
        }
        return i;
    }
    return size;
}

template <class Allocator>
inline size_t TaskQueue<Allocator>::PopBackgroundTasksToHelperThread(const AddTaskToHelperFunc &addTaskFunc,
                                                                     size_t size)
{
    for (size_t i = 0; i < size; i++) {
        TaskPtr task;
        if (backgroundTaskQueue_.TryPop(&task)) {
            addTaskFunc(task);
        }
        return i;
    }
    return size;
}

template <class Allocator>
inline size_t TaskQueue<Allocator>::GetCountOfLiveTasks() const
{
    return GetCountOfLiveForegroundTasks() + GetCountOfLiveBackgroundTasks();
}

template <class Allocator>
inline size_t TaskQueue<Allocator>::GetCountOfLiveForegroundTasks() const
{
    // Atomic with relaxed order reason: no order dependency with another variables
    return foregroundLiveTasks_.load(std::memory_order_relaxed);
}

template <class Allocator>
inline size_t TaskQueue<Allocator>::GetCountOfLiveBackgroundTasks() const
{
    // Atomic with relaxed order reason: no order dependency with another variables
    return backgroundLiveTasks_.load(std::memory_order_relaxed);
}

template <class Allocator>
inline TaskTimeStatsBase *TaskQueue<Allocator>::GetTaskTimeStats() const
{
    return taskTimeStats_;
}

template <class Allocator>
inline void TaskQueue<Allocator>::SetCallbacks(SignalWorkersCallback signalWorkersCallback,
                                               CheckIfTimerThreadIsEnabledCallback checkTimerThreadExistCallback)
{
    signalWorkersCallback_ = std::move(signalWorkersCallback);
    checkTimerThreadExistCallback_ = std::move(checkTimerThreadExistCallback);
}

template <class Allocator>
inline void TaskQueue<Allocator>::UnsetCallbacks()
{
    signalWorkersCallback_ = nullptr;
}

template <class Allocator>
inline size_t TaskQueue<Allocator>::AddForegroundTaskImpl(RunnerCallback &&runner)
{
    foregroundTaskQueue_.Emplace(Task::Create, std::move(runner), this, OnForegroundTaskDestructionCallback);
    if (signalWorkersCallback_ != nullptr) {
        signalWorkersCallback_();
    }
    return foregroundTaskQueue_.Size();
}

template <class Allocator>
inline size_t TaskQueue<Allocator>::AddBackgroundTaskImpl(RunnerCallback &&runner)
{
    backgroundTaskQueue_.Emplace(Task::Create, std::move(runner), this, OnBackgroundTaskDestructionCallback);
    if (signalWorkersCallback_ != nullptr) {
        signalWorkersCallback_();
    }
    return backgroundTaskQueue_.Size();
}

template <class Allocator>
inline size_t TaskQueue<Allocator>::IncrementCountOfLiveForegroundTasks()
{
    // Atomic with relaxed order reason: no order dependency with another variables
    return foregroundLiveTasks_.fetch_add(1U, std::memory_order_relaxed);
}
template <class Allocator>
inline size_t TaskQueue<Allocator>::IncrementCountOfLiveBackgroundTasks()
{
    // Atomic with relaxed order reason: no order dependency with another variables
    return backgroundLiveTasks_.fetch_add(1U, std::memory_order_relaxed);
}

}  // namespace ark::taskmanager::internal

#endif  // PANDA_LIBPANDABASE_TASKMANAGER_TASK_QUEUE_H
