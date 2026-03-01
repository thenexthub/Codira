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
#include "common_components/taskpool/taskpool.h"
#include "common_components/taskpool/task.h"

#include <atomic>
#include <chrono>
#include <thread>

namespace common {

class MockTask : public Task {
public:
    explicit MockTask(int32_t id)
        : Task(id), executed_(false), terminated_(false) {}

    bool Run(uint32_t threadId) override
    {
        executed_ = true;
        return true;
    }

    bool IsExecuted() const { return executed_; }

    bool IsTerminate() const
    {
        return terminated_;
    }

    void Terminate()
    {
        terminated_ = true;
    }

private:
    std::atomic<bool> executed_;
    std::atomic<bool> terminated_;
};

class TaskpoolTest : public common::test::BaseTestWithScope {
protected:
    void SetUp() override {}
    void TearDown() override {}

    class ScopedTaskpool {
    public:
        explicit ScopedTaskpool(int threadNum = DEFAULT_TASKPOOL_THREAD_NUM)
            : isInitialized_(false)
        {
            isInitialized_ = true;
            pool_.Initialize(threadNum);
        }

        Taskpool* Get()
        {
            return &pool_;
        }

    private:
        Taskpool pool_;
        bool isInitialized_;
    };
};

HWTEST_F_L0(TaskpoolTest, InitializeAndDestroy) {
    TaskpoolTest::ScopedTaskpool pool(2);
    EXPECT_NE(pool.Get(), nullptr);
    EXPECT_GT(pool.Get()->GetTotalThreadNum(), 0U);
}

HWTEST_F_L0(TaskpoolTest, TerminateTask) {
    TaskpoolTest::ScopedTaskpool pool(2);
    Taskpool* taskpool = pool.Get();
    ASSERT_NE(taskpool, nullptr);

    auto task1 = std::make_unique<MockTask>(1);
    auto task2 = std::make_unique<MockTask>(2);
    taskpool->PostTask(std::move(task1));
    taskpool->PostTask(std::move(task2));

    std::this_thread::sleep_for(std::chrono::milliseconds(100));

    taskpool->TerminateTask(1, TaskType::ALL);

    bool isTerminated = false;
    taskpool->ForEachTask([&isTerminated](Task* task) {
        if (task->GetId() == 1 && task->IsTerminate()) {
            isTerminated = true;
        }
    });

    EXPECT_FALSE(isTerminated);
}
}