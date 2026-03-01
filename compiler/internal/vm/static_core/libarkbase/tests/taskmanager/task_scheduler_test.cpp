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
#include <tuple>
#include <gtest/gtest.h>

namespace ark::taskmanager {

constexpr size_t DEFAULT_SEED = 123456U;
constexpr size_t TIMEOUT = 1U;

class TaskSchedulerTest : public testing::TestWithParam<TaskTimeStatsType> {
public:
    TaskSchedulerTest()
    {
#ifdef PANDA_NIGHTLY_TEST_ON
        seed_ = std::time(NULL);
#else
        seed_ = DEFAULT_SEED;
#endif
    };
    ~TaskSchedulerTest() override = default;

    NO_COPY_SEMANTIC(TaskSchedulerTest);
    NO_MOVE_SEMANTIC(TaskSchedulerTest);

    static constexpr size_t THREADED_TASKS_COUNT = 100'000U;

    std::thread *CreateTaskProducerThread(TaskQueueInterface *queue)
    {
        return new std::thread(
            [queue](TaskSchedulerTest *test) {
                for (size_t i = 0; i < THREADED_TASKS_COUNT; i++) {
                    queue->AddBackgroundTask([test]() { test->IncrementGlobalCounter(); });
                }
                test->AddedSetOfTasks();
            },
            this);
    }

    void IncrementGlobalCounter()
    {
        globalCounter_++;
    }

    size_t GetGlobalCounter() const
    {
        return globalCounter_;
    }

    void SetTasksSetCount(size_t setCount)
    {
        tasksCount_ = setCount;
    }

    /// Wait for all tasks would be added in queues
    void WaitAllTask()
    {
        os::memory::LockHolder lockHolder(tasksMutex_);
        while (tasksSetAdded_ != tasksCount_) {
            tasksCondVar_.TimedWait(&tasksMutex_, TIMEOUT);
        }
    }

    void AddedSetOfTasks()
    {
        os::memory::LockHolder lockHolder(tasksMutex_);
        tasksSetAdded_++;
        tasksCondVar_.SignalAll();
    }

    size_t GetSeed() const
    {
        return seed_;
    }

private:
    os::memory::Mutex lock_;
    os::memory::ConditionVariable condVar_;
    std::atomic_size_t globalCounter_ = 0U;

    size_t tasksCount_ = 0U;
    std::atomic_size_t tasksSetAdded_ = 0U;
    os::memory::Mutex tasksMutex_;
    os::memory::ConditionVariable tasksCondVar_;

    size_t seed_ = 0;
};

TEST_F(TaskSchedulerTest, TaskQueueRegistration)
{
    // CC-OFFNXT(G.NAM.03-CPP): static_core files have specifice codestyle
    constexpr size_t THREADS_COUNT = 4U;
    TaskManager::Start(THREADS_COUNT);
    // CC-OFFNXT(G.NAM.03-CPP): static_core files have specifice codestyle
    constexpr uint8_t QUEUE_PRIORITY = MAX_QUEUE_PRIORITY;
    TaskQueueInterface *queue = TaskManager::CreateTaskQueue(QUEUE_PRIORITY);
    EXPECT_NE(queue, nullptr);
    TaskManager::DestroyTaskQueue(queue);
    TaskManager::Finish();
}

TEST_F(TaskSchedulerTest, TaskQueuesFillingFromOwner)
{
    srand(GetSeed());
    // Create TaskScheduler
    constexpr size_t THREADS_COUNT = 4U;
    TaskManager::Start(THREADS_COUNT);
    // Create and register 2 queues
    constexpr uint8_t QUEUE_PRIORITY = DEFAULT_QUEUE_PRIORITY;
    TaskQueueInterface *gcQueue = TaskManager::CreateTaskQueue(QUEUE_PRIORITY);
    TaskQueueInterface *jitQueue = TaskManager::CreateTaskQueue(QUEUE_PRIORITY);
    // Fill queues with tasks that increment counter with its type.
    constexpr size_t COUNT_OF_TASK = 10U;
    std::array<std::atomic_size_t, 2U> counters = {0U, 0U};
    for (size_t i = 0U; i < COUNT_OF_TASK; i++) {
        gcQueue->AddBackgroundTask([&counters]() {
            constexpr size_t GC_COUNTER = 0U;
            // Atomic with relaxed order reason: data race with counters[GC_COUNTER] with no synchronization or ordering
            // constraints
            counters[GC_COUNTER].fetch_add(1U, std::memory_order_relaxed);
        });
        jitQueue->AddBackgroundTask([&counters]() {
            constexpr size_t JIT_COUNTER = 1U;
            // Atomic with relaxed order reason: data race with counters[JIT_COUNTER] with no synchronization or
            // ordering constraints
            counters[JIT_COUNTER].fetch_add(1U, std::memory_order_relaxed);
        });
    }
    gcQueue->WaitTasks();
    jitQueue->WaitTasks();
    for (auto &counter : counters) {
        ASSERT_EQ(counter, COUNT_OF_TASK) << "seed:" << GetSeed();
    }
    TaskManager::DestroyTaskQueue(gcQueue);
    TaskManager::DestroyTaskQueue(jitQueue);
    TaskManager::Finish();
}

TEST_F(TaskSchedulerTest, TaskCreateTask)
{
    srand(GetSeed());
    // Create TaskScheduler
    constexpr size_t THREADS_COUNT = 4U;
    TaskManager::Start(THREADS_COUNT);
    // Create and register 2 queues
    constexpr uint8_t QUEUE_PRIORITY = DEFAULT_QUEUE_PRIORITY;
    TaskQueueInterface *gcQueue = TaskManager::CreateTaskQueue(QUEUE_PRIORITY);
    TaskQueueInterface *jitQueue = TaskManager::CreateTaskQueue(QUEUE_PRIORITY);

    // Fill queues with tasks that increment counter with its type. GC task will add JIT task in MT.
    std::array<std::atomic_size_t, 2U> counters = {0U, 0U};
    constexpr size_t COUNT_OF_TASK = 10U;
    for (size_t i = 0; i < COUNT_OF_TASK; i++) {
        gcQueue->AddBackgroundTask([&counters, &jitQueue]() {
            constexpr size_t GC_COUNTER = 0U;
            // Atomic with relaxed order reason: data race with counters[GC_COUNTER] with no synchronization or ordering
            // constraints
            counters[GC_COUNTER].fetch_add(1U, std::memory_order_relaxed);
            jitQueue->AddBackgroundTask([&counters]() {
                constexpr size_t JIT_COUNTER = 1U;
                // Atomic with relaxed order reason: data race with counters[JIT_COUNTER] with no synchronization or
                // ordering constraints
                counters[JIT_COUNTER].fetch_add(1U, std::memory_order_relaxed);
            });
        });
    }
    gcQueue->WaitTasks();
    jitQueue->WaitTasks();
    for (auto &counter : counters) {
        ASSERT_EQ(counter, COUNT_OF_TASK) << "seed:" << GetSeed();
    }
    TaskManager::DestroyTaskQueue(gcQueue);
    TaskManager::DestroyTaskQueue(jitQueue);
    TaskManager::Finish();
}

TEST_F(TaskSchedulerTest, MultithreadingUsage)
{
    srand(GetSeed());
    // Create TaskScheduler
    constexpr size_t THREADS_COUNT = 4U;
    TaskManager::Start(THREADS_COUNT);
    constexpr size_t PRODUCER_THREADS_COUNT = 3U;
    SetTasksSetCount(PRODUCER_THREADS_COUNT);
    constexpr uint8_t QUEUE_PRIORITY = DEFAULT_QUEUE_PRIORITY;
    auto *jitStaticQueue = TaskManager::CreateTaskQueue(QUEUE_PRIORITY);
    auto *gcStaticQueue = TaskManager::CreateTaskQueue(QUEUE_PRIORITY);
    auto *gcDynamicQueue = TaskManager::CreateTaskQueue(QUEUE_PRIORITY);

    auto jitStaticThread = CreateTaskProducerThread(jitStaticQueue);
    auto gcStaticThread = CreateTaskProducerThread(gcStaticQueue);
    auto gcDynamicThread = CreateTaskProducerThread(gcDynamicQueue);

    WaitAllTask();

    jitStaticQueue->WaitTasks();
    gcStaticQueue->WaitTasks();
    gcDynamicQueue->WaitTasks();

    ASSERT_EQ(GetGlobalCounter(), THREADED_TASKS_COUNT * PRODUCER_THREADS_COUNT) << "seed:" << GetSeed();

    jitStaticThread->join();
    gcStaticThread->join();
    gcDynamicThread->join();

    delete jitStaticThread;
    delete gcStaticThread;
    delete gcDynamicThread;

    TaskManager::DestroyTaskQueue(jitStaticQueue);
    TaskManager::DestroyTaskQueue(gcStaticQueue);
    TaskManager::DestroyTaskQueue(gcDynamicQueue);
    TaskManager::Finish();
}

TEST_F(TaskSchedulerTest, TaskSchedulerGetTask)
{
    srand(GetSeed());
    // Create TaskScheduler
    // CC-OFFNXT(G.NAM.03-CPP): static_core files have specifice codestyle
    constexpr size_t THREADS_COUNT = 0U;  // Worker will not be used in this test
    TaskManager::Start(THREADS_COUNT);
    constexpr uint8_t QUEUE_PRIORITY = DEFAULT_QUEUE_PRIORITY;
    auto queue = TaskManager::CreateTaskQueue(QUEUE_PRIORITY);

    std::queue<size_t> globalQueue;
    constexpr size_t COUNT_OF_TASKS = 100U;
    for (size_t i = 0U; i < COUNT_OF_TASKS; i++) {
        queue->AddBackgroundTask([&globalQueue]() { globalQueue.push(0U); });
    }
    for (size_t i = 0U; i < COUNT_OF_TASKS;) {
        i += queue->ExecuteTask();
    }
    ASSERT_EQ(queue->ExecuteTask(), 0U) << "seed:" << GetSeed();
    ASSERT_EQ(globalQueue.size(), COUNT_OF_TASKS) << "seed:" << GetSeed();
    TaskManager::DestroyTaskQueue(queue);
    TaskManager::Finish();
}

TEST_F(TaskSchedulerTest, TasksWithMutex)
{
    srand(GetSeed());
    // Create TaskScheduler
    constexpr size_t THREADS_COUNT = 4U;
    TaskManager::Start(THREADS_COUNT);
    // Create and register 2 queues
    constexpr uint8_t QUEUE_PRIORITY = DEFAULT_QUEUE_PRIORITY;
    TaskQueueInterface *gcQueue = TaskManager::CreateTaskQueue(QUEUE_PRIORITY);
    TaskQueueInterface *jitQueue = TaskManager::CreateTaskQueue(QUEUE_PRIORITY);
    // Fill queues with tasks that increment counter with its type.
    constexpr size_t COUNT_OF_TASK = 1000U;
    std::array<size_t, 2U> counters = {0U, 0U};
    os::memory::Mutex mainMutex;
    for (size_t i = 0U; i < COUNT_OF_TASK; i++) {
        gcQueue->AddBackgroundTask([&mainMutex, &counters]() {
            constexpr size_t GC_COUNTER = 0U;
            os::memory::LockHolder lockHolder(mainMutex);
            counters[GC_COUNTER]++;
        });
        jitQueue->AddBackgroundTask([&mainMutex, &counters]() {
            constexpr size_t JIT_COUNTER = 1U;
            os::memory::LockHolder lockHolder(mainMutex);
            counters[JIT_COUNTER]++;
        });
    }
    gcQueue->WaitTasks();
    jitQueue->WaitTasks();
    for (auto &counter : counters) {
        os::memory::LockHolder lockHolder(mainMutex);
        ASSERT_EQ(counter, COUNT_OF_TASK) << "seed:" << GetSeed();
    }
    TaskManager::DestroyTaskQueue(gcQueue);
    TaskManager::DestroyTaskQueue(jitQueue);
    TaskManager::Finish();
}

TEST_F(TaskSchedulerTest, TaskCreateTaskRecursively)
{
    srand(GetSeed());
    // Create TaskScheduler
    constexpr size_t THREADS_COUNT = 4U;
    TaskManager::Start(THREADS_COUNT);
    // Create and register 2 queues
    constexpr uint8_t QUEUE_PRIORITY = DEFAULT_QUEUE_PRIORITY;
    TaskQueueInterface *gcQueue = TaskManager::CreateTaskQueue(QUEUE_PRIORITY);

    std::atomic_size_t counter = 0U;
    constexpr size_t COUNT_OF_TASK = 10U;
    constexpr size_t COUNT_OF_REPLICAS = 6U;
    constexpr size_t MAX_RECURSION_DEPTH = 5U;
    std::function<void(size_t)> runner;
    runner = [&counter, &runner, &gcQueue](size_t recursionDepth) {
        if (recursionDepth < MAX_RECURSION_DEPTH) {
            for (size_t j = 0; j < COUNT_OF_REPLICAS; j++) {
                gcQueue->AddBackgroundTask([runner, recursionDepth]() { runner(recursionDepth + 1U); });
            }
            // Atomic with relaxed order reason: data race with counter with no synchronization or ordering
            // constraints
            counter.fetch_add(1U, std::memory_order_relaxed);
        }
    };
    for (size_t i = 0U; i < COUNT_OF_TASK; i++) {
        gcQueue->AddBackgroundTask([runner]() { runner(0U); });
    }
    gcQueue->WaitTasks();
    ASSERT_TRUE(gcQueue->IsEmpty());
    TaskManager::DestroyTaskQueue(gcQueue);
    TaskManager::Finish();
}

TEST_F(TaskSchedulerTest, TaskSchedulerTaskGetTask)
{
    srand(GetSeed());
    // Create TaskScheduler
    constexpr size_t THREADS_COUNT = 4U;
    TaskManager::Start(THREADS_COUNT);
    // Create and register 2 queues
    constexpr uint8_t QUEUE_PRIORITY = DEFAULT_QUEUE_PRIORITY;
    TaskQueueInterface *gcQueue = TaskManager::CreateTaskQueue(QUEUE_PRIORITY);

    std::atomic_size_t counter = 0U;
    constexpr size_t COUNT_OF_TASK = 100'000U;
    gcQueue->AddBackgroundTask([gcQueue]() {
        size_t executedTasksCount = 0U;
        while (true) {  // wait for valid task;
            executedTasksCount = gcQueue->ExecuteTask();
            if (executedTasksCount > 0U) {
                break;
            }
        }
        for (size_t i = 0U; i < COUNT_OF_TASK; i++) {
            executedTasksCount += gcQueue->ExecuteTask();
        }
    });
    for (size_t i = 0U; i < COUNT_OF_TASK; i++) {
        gcQueue->AddBackgroundTask([&counter]() {
            // Atomic with relaxed order reason: data race with counter with no synchronization or ordering
            // constraints
            counter.fetch_add(1U, std::memory_order_relaxed);
        });
    }
    gcQueue->WaitTasks();
    ASSERT_TRUE(gcQueue->IsEmpty());
    TaskManager::DestroyTaskQueue(gcQueue);
    TaskManager::Finish();
}

TEST_F(TaskSchedulerTest, TaskSchedulerWaitForFinishAllTaskFromQueue)
{
    srand(GetSeed());
    // Create TaskScheduler
    constexpr size_t THREADS_COUNT = 4U;
    TaskManager::Start(THREADS_COUNT);
    // Create and register 2 queues
    constexpr uint8_t QUEUE_PRIORITY = DEFAULT_QUEUE_PRIORITY;
    TaskQueueInterface *gcQueue = TaskManager::CreateTaskQueue(QUEUE_PRIORITY);
    TaskQueueInterface *jitQueue = TaskManager::CreateTaskQueue(QUEUE_PRIORITY);
    // Fill queues with tasks that increment counter with its type.
    constexpr size_t COUNT_OF_TASK = 10'000U;
    std::array<std::atomic_size_t, 3U> counters = {0U, 0U, 0U};
    for (size_t i = 0U; i < COUNT_OF_TASK; i++) {
        gcQueue->AddBackgroundTask([&counters]() {
            constexpr size_t GC_BACKGROUND_COUNTER = 0U;
            // Atomic with relaxed order reason: data race with counters[GC_BACKGROUND_COUNTER] with no synchronization
            // or ordering constraints
            counters[GC_BACKGROUND_COUNTER].fetch_add(1U, std::memory_order_relaxed);
        });
        gcQueue->AddForegroundTask([&counters]() {
            constexpr size_t GC_FOREGROUND_COUNTER = 1U;
            // Atomic with relaxed order reason: data race with counters[GC_FOREGROUND_COUNTER] with no synchronization
            // or ordering constraints
            counters[GC_FOREGROUND_COUNTER].fetch_add(1U, std::memory_order_relaxed);
        });
        jitQueue->AddBackgroundTask([&counters]() {
            constexpr size_t JIT_COUNTER = 2U;
            // Atomic with relaxed order reason: data race with counters[JIT_COUNTER] with no synchronization or
            // ordering constraints
            counters[JIT_COUNTER].fetch_add(1U, std::memory_order_relaxed);
        });
    }
    gcQueue->WaitForegroundTasks();
    ASSERT_FALSE(gcQueue->HasForegroundTasks());
    gcQueue->WaitBackgroundTasks();
    ASSERT_TRUE(gcQueue->IsEmpty());
    jitQueue->WaitTasks();
    ASSERT_TRUE(jitQueue->IsEmpty());
    for (auto &counter : counters) {
        ASSERT_EQ(counter, COUNT_OF_TASK) << "seed:" << GetSeed();
    }
    TaskManager::DestroyTaskQueue(gcQueue);
    TaskManager::DestroyTaskQueue(jitQueue);
    TaskManager::Finish();
}

TEST_F(TaskSchedulerTest, TaskSchedulerAddTaskToWaitListWithTimeTest)
{
    srand(GetSeed());
    // Create TaskScheduler
    constexpr size_t THREADS_COUNT = 1U;
    TaskManager::Start(THREADS_COUNT);
    TaskManager::EnableTimerThread();
    // Create and register 2 queues
    // CC-OFFNXT(G.NAM.03-CPP): static_core files have specifice codestyle
    constexpr uint8_t QUEUE_PRIORITY = DEFAULT_QUEUE_PRIORITY;
    TaskQueueInterface *gcQueue = TaskManager::CreateTaskQueue(QUEUE_PRIORITY);

    constexpr size_t WAIT_LIST_USAGE_COUNT = 5U;
    std::atomic_size_t sleepCount = 0U;
    std::function<void()> taskRunner;
    taskRunner = [gcQueue, &sleepCount, &taskRunner]() {
        if (sleepCount < WAIT_LIST_USAGE_COUNT) {
            sleepCount++;
            [[maybe_unused]] auto id = gcQueue->AddForegroundTaskInWaitList(taskRunner, 1U);
            ASSERT(id != INVALID_WAITER_ID);
        }
    };
    gcQueue->AddForegroundTask(taskRunner);
    gcQueue->WaitTasks();
    ASSERT_EQ(sleepCount, WAIT_LIST_USAGE_COUNT);
    // Fill queues with tasks that increment counter with its type.
    TaskManager::DestroyTaskQueue(gcQueue);
    TaskManager::DisableTimerThread();
    TaskManager::Finish();
}

TEST_F(TaskSchedulerTest, TaskSchedulerWaitListWithoutTimerThread)
{
    srand(GetSeed());
    // Create TaskScheduler
    constexpr size_t THREADS_COUNT = 1U;
    TaskManager::Start(THREADS_COUNT);
    // Create and register 2 queues
    // CC-OFFNXT(G.NAM.03-CPP): static_core files have specifice codestyle
    constexpr uint8_t QUEUE_PRIORITY = DEFAULT_QUEUE_PRIORITY;
    TaskQueueInterface *gcQueue = TaskManager::CreateTaskQueue(QUEUE_PRIORITY);

    ASSERT_EQ(gcQueue->AddForegroundTaskInWaitList([]() {}, 100U), INVALID_WAITER_ID);
    ASSERT_EQ(gcQueue->AddForegroundTaskInWaitList([]() {}), INVALID_WAITER_ID);

    TaskManager::DestroyTaskQueue(gcQueue);
    TaskManager::Finish();
}

TEST_F(TaskSchedulerTest, ChangeCountOfWorkers)
{
    srand(GetSeed());
    // CC-OFFNXT(G.NAM.03-CPP): static_core files have specifice codestyle
    constexpr size_t THREADS_COUNT = 4U;
    TaskManager::Start(THREADS_COUNT);
    ASSERT_EQ(TaskManager::GetWorkersCount(), THREADS_COUNT);

    TaskManager::SetWorkersCount(0U);
    ASSERT_EQ(TaskManager::GetWorkersCount(), 0U);

    TaskManager::SetWorkersCount(THREADS_COUNT);
    ASSERT_EQ(TaskManager::GetWorkersCount(), THREADS_COUNT);

    // CC-OFFNXT(G.NAM.03-CPP): static_core files have specifice codestyle
    constexpr size_t NEW_THREADS_COUNT = 6U;
    TaskManager::SetWorkersCount(NEW_THREADS_COUNT);
    ASSERT_EQ(TaskManager::GetWorkersCount(), NEW_THREADS_COUNT);

    TaskManager::SetWorkersCount(THREADS_COUNT);
    ASSERT_EQ(TaskManager::GetWorkersCount(), THREADS_COUNT);

    TaskManager::SetWorkersCount(MAX_WORKER_COUNT + 1U);
    ASSERT_EQ(TaskManager::GetWorkersCount(), MAX_WORKER_COUNT);

    TaskManager::Finish();
}

}  // namespace ark::taskmanager
