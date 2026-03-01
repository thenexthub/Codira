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

#include "common_components/tests/test_helper.h"
#include "common_components/taskpool/task_queue.h"
#include "common_components/taskpool/task.h"

#include <thread>
#include <chrono>

namespace common {

class TaskQueueTest : public common::test::BaseTestWithScope {
protected:
    void SetUp() override
    {
        queue_ = new TaskQueue();
    }

    void TearDown() override
    {
        delete queue_;
        queue_ = nullptr;
    }

    TaskQueue* queue_;
};

class MockTask : public Task {
public:
    explicit MockTask(int id) : Task(id), executed_(false) {}

    bool Run(uint32_t threadId) override
    {
        executed_ = true;
        return true;
    }

    bool IsExecuted() const { return executed_; }

private:
    mutable bool executed_;
};


HWTEST_F_L0(TaskQueueTest, PopTask_DelayedTaskExpired_CanBeExecuted)
{
    auto task = std::make_unique<MockTask>(2);
    queue_->PostDelayedTask(std::move(task), 500);

    usleep(600 * 1000);

    auto poppedTask = queue_->PopTask();
    ASSERT_NE(poppedTask, nullptr);

    MockTask* mockTask = static_cast<MockTask*>(poppedTask.get());
    EXPECT_FALSE(mockTask->IsExecuted());

    (void)poppedTask->Run(0);
    EXPECT_TRUE(mockTask->IsExecuted());
}

HWTEST_F_L0(TaskQueueTest, PopTask_MultipleDelayedTasks_ExecuteInOrder)
{
    auto task1 = std::make_unique<MockTask>(1);
    auto task2 = std::make_unique<MockTask>(2);
    auto task3 = std::make_unique<MockTask>(3);

    queue_->PostDelayedTask(std::move(task1), 800);
    queue_->PostDelayedTask(std::move(task2), 500);
    queue_->PostDelayedTask(std::move(task3), 1000);

    usleep(600 * 1000);

    auto poppedTask = queue_->PopTask();
    ASSERT_NE(poppedTask, nullptr);
    EXPECT_EQ(poppedTask->GetId(), 2);

    poppedTask->Run(0);
    MockTask* mockTask2 = static_cast<MockTask*>(poppedTask.get());
    EXPECT_TRUE(mockTask2->IsExecuted());

    usleep(300 * 1000);

    poppedTask = queue_->PopTask();
    ASSERT_NE(poppedTask, nullptr);
    EXPECT_EQ(poppedTask->GetId(), 1);

    poppedTask->Run(0);
    MockTask* mockTask1 = static_cast<MockTask*>(poppedTask.get());
    EXPECT_TRUE(mockTask1->IsExecuted());

    usleep(200 * 1000);

    poppedTask = queue_->PopTask();
    ASSERT_NE(poppedTask, nullptr);
    EXPECT_EQ(poppedTask->GetId(), 3);

    poppedTask->Run(0);
    MockTask* mockTask3 = static_cast<MockTask*>(poppedTask.get());
    EXPECT_TRUE(mockTask3->IsExecuted());
}
}