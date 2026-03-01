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

#include "libarkbase/taskmanager/task_scheduler.h"
#include <atomic>
#include "libarkbase/taskmanager/task_queue_set.h"
#include "libarkbase/utils/logger.h"
#include "libarkbase/os/mutex.h"
#include "libarkbase/taskmanager/task_manager_common.h"

namespace ark::taskmanager::internal {

TaskScheduler::TaskScheduler(size_t workersCount, TaskWaitList *waitList, TaskQueueSet *queueSet)
    : waitList_(waitList), queueSet_(queueSet)
{
    ASSERT(!start_);
    start_ = true;
    LOG(DEBUG, TASK_MANAGER) << "TaskScheduler: creates " << workersCount << " threads";
    // Starts all workers
    SetCountOfWorkers(workersCount);
}

size_t TaskScheduler::StealTaskFromOtherWorker(WorkerThread *taskReceiver)
{
    // WorkerThread tries to find Worker with the most tasks
    // CC-OFFNXT(G.CTL.03): false positive
    while (true) {
        // When we start stealing worker is empty, so if it will be chosen, we will leave a method with 0
        WorkerThread *chosenWorker = taskReceiver;
        size_t taskCount = 0;
        // First stage: find worker with most count of tasks
        // Atomic with relaxed order reason: no order required
        size_t workersCount = workersCount_.load(std::memory_order_relaxed);
        for (size_t i = 0; i < workersCount; i++) {
            auto *worker = workers_[i].load(std::memory_order_relaxed);
            if (worker != nullptr && taskCount < worker->Size()) {
                chosenWorker = worker;
                taskCount = worker->Size();
            }
        }
        // Check if chosen worker contains anough tasks
        if (taskCount <= 1U) {
            return 0U;
        }
        // If worker was successfully found, steals task from its local queue
        TaskPtr task = chosenWorker->PopForegroundTask();
        if (task != nullptr) {
            taskReceiver->AddForegroundTask(task);
            return 1U;
        }
        task = chosenWorker->PopBackgroundTask();
        if (task != nullptr) {
            taskReceiver->AddBackgroundTask(task);
            return 1U;
        }
        // If getting faild with Pop task we should retry
    }
}

bool TaskScheduler::FillWithTasks(WorkerThread *worker)
{
    // CC-OFFNXT(G.CTL.03): false positive
    while (true) {
        auto queue = queueSet_->SelectQueue();
        if (queue != nullptr) {
            PutTasksInWorker(worker, reinterpret_cast<internal::SchedulableTaskQueueInterface *>(queue));
            return false;
        }

        if (GetCountOfTasksInSystem() != 0U && StealTaskFromOtherWorker(worker) != 0) {
            return false;
        }

        if (WaitUntilNewTasks(worker)) {
            return true;
        }
    }
}

size_t TaskScheduler::ProcessWaitList()
{
    // Atomic with relaxed order reason: no sync depends
    if (waitListIsProcessing_.exchange(true, std::memory_order_relaxed)) {
        return 0;
    }
    if (!waitList_->HaveReadyValue()) {
        // Atomic with relaxed order reason: no sync depends
        waitListIsProcessing_.store(false, std::memory_order_relaxed);
        return 0;
    }
    auto count = waitList_->ProcessWaitList([](TaskWaitListElem &&waitVal) {
        auto &[task, taskPoster] = waitVal;
        taskPoster(std::move(task));
    });
    // Atomic with relaxed order reason: no sync depends
    waitListIsProcessing_.store(false, std::memory_order_relaxed);
    return count;
}

bool TaskScheduler::WaitUntilNewTasks(WorkerThread *worker)
{
    os::memory::LockHolder lh(taskSchedulerStateLock_);
    // Atomic with relaxed order reason: no sync depends
    waitWorkersCount_.fetch_add(1, std::memory_order_relaxed);
    // we don't use while loop here, because WorkerThread should be fast, if it was contified waiting work correct, if
    // it was not we will retuen to the worker, where we should retry all the checks, so behiaviour is correct.
    if (AreQueuesEmpty() && !worker->CheckFinish()) {
        queuesWaitCondVar_.Wait(&taskSchedulerStateLock_);
    }
    // Atomic with relaxed order reason: no sync depends
    waitWorkersCount_.fetch_sub(1, std::memory_order_relaxed);
    return worker->CheckFinish();
}

size_t TaskScheduler::PutTasksInWorker(WorkerThread *worker, internal::SchedulableTaskQueueInterface *queue)
{
    auto addForegroundTaskFunc = [worker](TaskPtr &&task) { worker->AddForegroundTask(task); };
    auto addBackgroundTaskFunc = [worker](TaskPtr &&task) { worker->AddBackgroundTask(task); };

    // Now we calc how many task we want to get from queue. If there are few tasks, then we want them to be evenly
    // distributed among the workers.
    size_t size = queue->Size();
    // Atomic with relaxed order reason: no order required
    size_t workersCount = workersCount_.load(std::memory_order_relaxed);
    // CC-OFFNXT(G.EXP.22-CPP): workersCount represents conts of execution units, it's 0 only if currect worker doesn't
    // exist
    size_t countToGet = size / workersCount + 1;
    countToGet = (countToGet >= WorkerThread::WORKER_QUEUE_SIZE) ? WorkerThread::WORKER_QUEUE_SIZE : countToGet;
    // Execute popping task form queue
    size_t queueTaskCount = queue->PopTasksToWorker(addForegroundTaskFunc, addBackgroundTaskFunc, countToGet);
    LOG(DEBUG, TASK_MANAGER) << worker->GetWorkerName() << ": get tasks " << queueTaskCount << "; ";
    return queueTaskCount;
}

bool TaskScheduler::AreQueuesEmpty() const
{
    return queueSet_->AreQueuesEmpty();
}

bool TaskScheduler::AreWorkersEmpty() const
{
    // Atomic with relaxed order reason: no order required
    size_t workersCount = workersCount_.load(std::memory_order_relaxed);
    for (size_t i = 0; i < workersCount; i++) {
        // Atomic with relaxed order reason: no order required
        auto *worker = workers_[i].load(std::memory_order_relaxed);
        if (worker != nullptr && !worker->IsEmpty()) {
            return false;
        }
    }
    return true;
}

bool TaskScheduler::AreNoMoreTasks() const
{
    return GetCountOfTasksInSystem() == 0;
}

void TaskScheduler::SignalWorkers()
{
    // Atomic with relaxed order reason: no order required
    if (waitWorkersCount_.load(std::memory_order_relaxed) > 0) {
        os::memory::LockHolder lh(taskSchedulerStateLock_);
        queuesWaitCondVar_.Signal();
    }
}

size_t TaskScheduler::GetCountOfTasksInSystem() const
{
    return queueSet_->GetCountOfLiveTasks();
}

size_t TaskScheduler::GetCountOfWorkers() const
{
    os::memory::LockHolder lh(taskSchedulerStateLock_);
    // Atomic with relaxed order reason: no order required
    return workersCount_.load(std::memory_order_relaxed);
}

void TaskScheduler::SetCountOfWorkers(size_t count)
{
    os::memory::LockHolder lh(taskSchedulerStateLock_);
    // Atomic with relaxed order reason: no order required
    size_t currentCount = workersCount_.load(std::memory_order_relaxed);
    if (count > MAX_WORKER_COUNT) {
        count = MAX_WORKER_COUNT;
    }
    if (count > currentCount) {
        // Atomic with relaxed order reason: no order required
        workersCount_.store(count, std::memory_order_relaxed);
        for (size_t i = currentCount; i < count; i++) {
            auto workerName = "TMWorker_" + std::to_string(i + 1UL);
            // Atomic with relaxed order reason: no order required
            workers_[i].store(new WorkerThread(this, queueSet_->GetTaskTimeStats(), workerName),
                              std::memory_order_relaxed);
            LOG(DEBUG, TASK_MANAGER) << "TaskScheduler: created thread with name " << workerName;
        }
        return;
    }
    if (count < currentCount) {
        std::vector<WorkerThread *> workersToWait;
        for (size_t i = count; i != currentCount; i++) {
            // Atomic with relaxed order reason: no order required
            auto *worker = workers_[i].load(std::memory_order_relaxed);
            worker->SetFinish();
            workersToWait.push_back(worker);
        }
        queuesWaitCondVar_.SignalAll();
        taskSchedulerStateLock_.Unlock();
        for (auto *worker : workersToWait) {
            worker->Join();
        }
        taskSchedulerStateLock_.Lock();
        for (auto *worker : workersToWait) {
            workersToDelete_.push_back(worker);
        }
        // Atomic with relaxed order reason: no order required
        workersCount_.store(count, std::memory_order_relaxed);
        return;
    }
}

TaskScheduler::~TaskScheduler()
{
    ASSERT(start_);
    ASSERT(AreNoMoreTasks());
    SetCountOfWorkers(0);
    for (auto *worker : workersToDelete_) {
        delete worker;
    }
    LOG(DEBUG, TASK_MANAGER) << "TaskScheduler: Finalized";
}

}  // namespace ark::taskmanager::internal
