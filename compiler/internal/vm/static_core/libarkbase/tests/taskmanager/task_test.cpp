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

#include "libarkbase/taskmanager/task_queue.h"
#include <gtest/gtest.h>
#include <thread>
#include <queue>

namespace ark::taskmanager::internal {

class TaskTest : public testing::Test {
public:
    TaskTest() = default;
    ~TaskTest() override = default;

    NO_COPY_SEMANTIC(TaskTest);
    NO_MOVE_SEMANTIC(TaskTest);

private:
};

TEST_F(TaskTest, TaskQueueSimpleTest)
{
    size_t counter = 0;
    // Creation of TaskQueue
    // CC-OFFNXT(G.NAM.03-CPP): static_core files have specifice codestyle
    constexpr uint8_t QUEUE_PRIORITY = MAX_QUEUE_PRIORITY;
    SchedulableTaskQueueInterface *queue = TaskQueue<>::Create(QUEUE_PRIORITY, nullptr, nullptr);
    EXPECT_TRUE(queue->IsEmpty());
    EXPECT_EQ(queue->Size(), 0);
    EXPECT_EQ(queue->GetPriority(), QUEUE_PRIORITY);
    // Add COUNT_OF_TASKS tasks in queue-> Each task increment counter.
    // CC-OFFNXT(G.NAM.03-CPP): static_core files have specifice codestyle
    constexpr size_t COUNT_OF_TASKS = 10;
    for (size_t i = 0; i < COUNT_OF_TASKS; i++) {
        queue->AddBackgroundTask([&counter]() { counter++; });
    }
    EXPECT_FALSE(queue->IsEmpty());
    EXPECT_EQ(queue->Size(), COUNT_OF_TASKS);
    // Pop count_of_done_task tasks from queue and execute them.
    // CC-OFFNXT(G.NAM.03-CPP): static_core files have specifice codestyle
    constexpr size_t COUNT_OF_DONE_TASKS = 6;
    ASSERT(COUNT_OF_DONE_TASKS < COUNT_OF_TASKS);
    for (size_t i = 0; i < COUNT_OF_DONE_TASKS; i++) {
        auto popTask = static_cast<SchedulableTaskQueueInterface *>(queue)->PopTask();
        ASSERT_NE(popTask, nullptr);
        popTask->RunTask();
        EXPECT_EQ(counter, i + 1);
        Task::Delete(popTask);
    }
    // Now in queue counter_of_tasks - COUNT_OF_DONE_TASKS objects.
    EXPECT_EQ(queue->IsEmpty(), COUNT_OF_TASKS == COUNT_OF_DONE_TASKS);
    EXPECT_EQ(queue->Size(), COUNT_OF_TASKS - COUNT_OF_DONE_TASKS);
    // Change priority of this queue
    // CC-OFFNXT(G.NAM.03-CPP): static_core files have specifice codestyle
    constexpr size_t NEW_QUEUE_PRIORITY = MIN_QUEUE_PRIORITY;
    queue->SetPriority(NEW_QUEUE_PRIORITY);
    EXPECT_EQ(queue->IsEmpty(), COUNT_OF_TASKS == COUNT_OF_DONE_TASKS);
    EXPECT_EQ(queue->Size(), COUNT_OF_TASKS - COUNT_OF_DONE_TASKS);
    EXPECT_EQ(queue->GetPriority(), NEW_QUEUE_PRIORITY);
    // Add in queue counter_of_tasks new tasks-> Each add 2 to counter
    for (size_t i = 0; i < COUNT_OF_TASKS; i++) {
        queue->AddBackgroundTask([&counter]() { counter += 2U; });
    }
    // After we have 2 * counter_of_tasks - counter_of_done_tasks objects in queue
    EXPECT_FALSE(queue->IsEmpty());
    EXPECT_EQ(queue->Size(), 2U * COUNT_OF_TASKS - COUNT_OF_DONE_TASKS);
    // Pop and execute all tasks in queue->
    while (!queue->IsEmpty()) {
        auto nextTask = static_cast<SchedulableTaskQueueInterface *>(queue)->PopTask();
        nextTask->RunTask();
        Task::Delete(nextTask);
    }
    // After all task is done, counter = 3 * COUNT_OF_TASKS
    EXPECT_EQ(counter, 3 * COUNT_OF_TASKS);
    EXPECT_EQ(queue->Size(), 0);
    TaskQueue<>::Destroy(queue);
}

TEST_F(TaskTest, TaskQueueMultithreadingOnePushOnePop)
{
    // CC-OFFNXT(G.NAM.03-CPP): static_core files have specifice codestyle
    constexpr uint8_t QUEUE_PRIORITY = MAX_QUEUE_PRIORITY;
    SchedulableTaskQueueInterface *queue = TaskQueue<>::Create(QUEUE_PRIORITY, nullptr, nullptr);
    std::atomic_size_t counter = 0;
    // CC-OFFNXT(G.NAM.03-CPP): static_core files have specifice codestyle
    constexpr size_t RESULT_COUNT = 10;
    auto pusher = [&queue, &counter]() {
        for (size_t i = 0; i < RESULT_COUNT; i++) {
            queue->AddBackgroundTask([&counter]() { counter++; });
        }
    };
    auto popper = [&queue]() {
        for (size_t i = 0; i < RESULT_COUNT;) {
            auto task = queue->PopTask();
            if (task == nullptr) {
                continue;
            }
            task->RunTask();
            Task::Delete(task);
            i++;
        }
    };
    auto *worker1 = new std::thread(pusher);
    auto *worker2 = new std::thread(popper);
    worker1->join();
    worker2->join();
    delete worker1;
    delete worker2;
    EXPECT_EQ(counter, RESULT_COUNT);
    TaskQueue<>::Destroy(queue);
}

TEST_F(TaskTest, TaskQueueMultithreadingNPushNPop)
{
    // CC-OFFNXT(G.NAM.03-CPP): static_core files have specifice codestyle
    constexpr uint8_t QUEUE_PRIORITY = MAX_QUEUE_PRIORITY;
    SchedulableTaskQueueInterface *queue = TaskQueue<>::Create(QUEUE_PRIORITY, nullptr, nullptr);
    std::atomic_size_t counter = 0;
    // CC-OFFNXT(G.NAM.03-CPP): static_core files have specifice codestyle
    constexpr size_t RESULT_COUNT = 10'000;
    auto pusher = [&queue, &counter]() {
        for (size_t i = 0; i < RESULT_COUNT; i++) {
            queue->AddBackgroundTask([&counter]() { counter++; });
        }
    };
    auto popper = [&queue]() {
        TaskPtr task;
        for (size_t i = 0; i < RESULT_COUNT;) {
            task = queue->PopTask();
            if (task == nullptr) {
                continue;
            }
            task->RunTask();
            Task::Delete(task);
            i++;
        }
    };
    std::vector<std::thread *> pushers;
    std::vector<std::thread *> poppers;
    // CC-OFFNXT(G.NAM.03-CPP): static_core files have specifice codestyle
    constexpr size_t COUNT_OF_WORKERS = 10;
    for (size_t i = 0; i < COUNT_OF_WORKERS; i++) {
        pushers.push_back(new std::thread(pusher));
        poppers.push_back(new std::thread(popper));
    }
    for (size_t i = 0; i < COUNT_OF_WORKERS; i++) {
        pushers[i]->join();
        poppers[i]->join();
        delete pushers[i];
        delete poppers[i];
    }
    EXPECT_EQ(counter, RESULT_COUNT * COUNT_OF_WORKERS);
    TaskQueue<>::Destroy(queue);
}

TEST_F(TaskTest, TaskQueueWaitForQueueEmptyAndFinish)
{
    // CC-OFFNXT(G.NAM.03-CPP): static_core files have specifice codestyle
    constexpr uint8_t QUEUE_PRIORITY = MAX_QUEUE_PRIORITY;
    SchedulableTaskQueueInterface *queue = TaskQueue<>::Create(QUEUE_PRIORITY, nullptr, nullptr);
    std::atomic_size_t counter = 0;
    // CC-OFFNXT(G.NAM.03-CPP): static_core files have specifice codestyle
    constexpr size_t TASK_COUNT = 100'000;
    for (size_t i = 0; i < TASK_COUNT; i++) {
        queue->AddBackgroundTask([&counter]() { counter++; });
    }

    // CC-OFFNXT(G.NAM.03-CPP): static_core files have specifice codestyle
    constexpr size_t THREAD_COUNTER = 10;
    os::memory::Mutex popLock;
    std::vector<std::thread> poppers;
    for (size_t i = 0; i < THREAD_COUNTER; i++) {
        poppers.emplace_back([&queue]() {
            while (true) {
                auto task = queue->PopTask();
                if (task == nullptr) {
                    break;
                }
                task->RunTask();
                Task::Delete(task);
            }
        });
    }

    for (auto &popper : poppers) {
        popper.join();
    }
    EXPECT_EQ(counter, TASK_COUNT);
    TaskQueue<>::Destroy(queue);
}

TEST_F(TaskTest, TaskQueueForegroundAndBackgroundTasks)
{
    // CC-OFFNXT(G.NAM.03-CPP): static_core files have specifice codestyle
    constexpr uint8_t QUEUE_PRIORITY = MAX_QUEUE_PRIORITY;
    SchedulableTaskQueueInterface *queue = TaskQueue<>::Create(QUEUE_PRIORITY, nullptr, nullptr);
    std::queue<std::string> modeQueue;
    // CC-OFFNXT(G.NAM.03-CPP): static_core files have specifice codestyle
    constexpr size_t TASKS_COUNT = 100;

    for (size_t i = 0; i < TASKS_COUNT; i++) {
        queue->AddBackgroundTask([&modeQueue]() { modeQueue.push("background"); });
    }
    for (size_t i = 0; i < TASKS_COUNT; i++) {
        queue->AddForegroundTask([&modeQueue]() { modeQueue.push("foreground"); });
    }

    for (size_t i = 0; i < 2U * TASKS_COUNT; i++) {
        auto task = queue->PopTask();
        ASSERT_TRUE(task != nullptr);
        task->RunTask();
        Task::Delete(task);
    }

    for (size_t i = 0; i < TASKS_COUNT; i++) {
        auto mode = modeQueue.front();
        modeQueue.pop();
        EXPECT_EQ(mode, "foreground");
    }
    for (size_t i = 0; i < TASKS_COUNT; i++) {
        auto mode = modeQueue.front();
        modeQueue.pop();
        EXPECT_EQ(mode, "background");
    }
    EXPECT_TRUE(modeQueue.empty());
    TaskQueue<>::Destroy(queue);
}

TEST_F(TaskTest, PopTaskWithExecutionMode)
{
    // CC-OFFNXT(G.NAM.03-CPP): static_core files have specifice codestyle
    constexpr uint8_t QUEUE_PRIORITY = MAX_QUEUE_PRIORITY;
    SchedulableTaskQueueInterface *queue = TaskQueue<>::Create(QUEUE_PRIORITY, nullptr, nullptr);
    std::queue<std::string> modeQueue;
    // CC-OFFNXT(G.NAM.03-CPP): static_core files have specifice codestyle
    constexpr size_t TASKS_COUNT = 100;

    for (size_t i = 0; i < TASKS_COUNT; i++) {
        queue->AddBackgroundTask([&modeQueue]() { modeQueue.push("background"); });
    }
    for (size_t i = 0; i < TASKS_COUNT; i++) {
        queue->AddForegroundTask([&modeQueue]() { modeQueue.push("foreground"); });
    }

    for (size_t i = 0; i < TASKS_COUNT; i++) {
        auto task = queue->PopForegroundTask();
        ASSERT_TRUE(task != nullptr);
        task->RunTask();
        Task::Delete(task);
        task = queue->PopBackgroundTask();
        ASSERT_TRUE(task != nullptr);
        task->RunTask();
        Task::Delete(task);
    }

    for (size_t i = 0; i < TASKS_COUNT; i++) {
        auto mode = modeQueue.front();
        modeQueue.pop();
        EXPECT_EQ(mode, "foreground");
        mode = modeQueue.front();
        modeQueue.pop();
        EXPECT_EQ(mode, "background");
    }
    EXPECT_TRUE(modeQueue.empty());
    TaskQueue<>::Destroy(queue);
}

}  // namespace ark::taskmanager::internal
