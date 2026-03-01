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
#include <chrono>
#include <gtest/gtest.h>
#include <cmath>

namespace ark::taskmanager {

class TaskManagerGCCornerCaseTest : public ::testing::Test {
public:
    static constexpr size_t COUNT_OF_WORKERS = 4;

    static constexpr size_t REPETITION_COUNT = 1000;

    static constexpr size_t FIRST_GC_STAGE_TASK_COUNT = 500;
    static constexpr size_t SECOND_GC_STAGE_TASK_COUNT = 50;
    static constexpr size_t THIRD_GC_STAGE_TASK_COUNT = 30;
    static constexpr size_t ADDITIONAL_GC_STAGE_TASK_COUNT = 50;

    static constexpr std::chrono::nanoseconds FIRST_GC_STAGE_TASK_DURATION {1500};
    static constexpr std::chrono::microseconds SECOND_GC_STAGE_TASK_DURATION {1};
    static constexpr std::chrono::microseconds THIRD_GC_STAGE_TASK_DURATION {1};
    static constexpr std::chrono::nanoseconds ADDITIONAL_GC_STAGE_TASK_DURATION {1500};

    static constexpr size_t CALCULATION_BASE = 0x777777;

    TaskManagerGCCornerCaseTest() = default;
    ~TaskManagerGCCornerCaseTest() override = default;

    NO_COPY_SEMANTIC(TaskManagerGCCornerCaseTest);
    NO_MOVE_SEMANTIC(TaskManagerGCCornerCaseTest);

    void SetQueue(TaskQueueInterface *queue)
    {
        queue_ = queue;
    }

    TaskQueueInterface *GetQueue()
    {
        return queue_;
    }

    void GetAndRunForegroundTasks()
    {
        while (true) {
            size_t count = queue_->ExecuteForegroundTask();
            if (count == 0) {
                break;
            }
        }
    }

    void BigGcTask(bool needAdditionalTasks)
    {
        LoadForegroundTasks(FIRST_GC_STAGE_TASK_COUNT, []() { LoadRunner(FIRST_GC_STAGE_TASK_DURATION); });
        GetAndRunForegroundTasks();
        queue_->WaitForegroundTasks();

        LoadForegroundTasks(SECOND_GC_STAGE_TASK_COUNT, []() { LoadRunner(SECOND_GC_STAGE_TASK_DURATION); });
        GetAndRunForegroundTasks();
        queue_->WaitForegroundTasks();

        LoadForegroundTasks(THIRD_GC_STAGE_TASK_COUNT, []() { LoadRunner(THIRD_GC_STAGE_TASK_DURATION); });
        GetAndRunForegroundTasks();
        queue_->WaitForegroundTasks();

        if (needAdditionalTasks) {
            LoadForegroundTasks(ADDITIONAL_GC_STAGE_TASK_COUNT,
                                []() { LoadRunner(ADDITIONAL_GC_STAGE_TASK_DURATION); });
            GetAndRunForegroundTasks();
            queue_->WaitForegroundTasks();
        }
    }

    void LoadForegroundTasks(size_t count, const std::function<void()> &runner)
    {
        for (size_t i = 0; i < count; i++) {
            queue_->AddForegroundTask(runner);
        }
    }

    static void LoadRunner(std::chrono::nanoseconds time)
    {
        const auto startTime = std::chrono::system_clock::now();
        volatile size_t counter = 0;
        do {
            // just calculations for load
            counter += std::log(CALCULATION_BASE * time.count()) + 1;
        } while ((std::chrono::system_clock::now() - startTime) < time && counter > 1);
    }

    void SetUp() override
    {
        TaskManager::Start(COUNT_OF_WORKERS, TaskTimeStatsType::NO_STATISTICS);
    }

    void TearDown() override
    {
        TaskManager::Finish();
    }

private:
    TaskQueueInterface *queue_ {nullptr};
};

TEST_F(TaskManagerGCCornerCaseTest, TriggerGcInMainThread)
{
    auto *queue = TaskManager::CreateTaskQueue();
    SetQueue(queue);
    for (size_t i = 0; i < REPETITION_COUNT; i++) {
        BigGcTask(false);
    }
    TaskManager::DestroyTaskQueue(queue);
}

TEST_F(TaskManagerGCCornerCaseTest, TriggerGcInTaskManager)
{
    auto *queue = TaskManager::CreateTaskQueue();
    SetQueue(queue);
    for (size_t i = 0; i < REPETITION_COUNT; i++) {
        queue->AddBackgroundTask([this]() { BigGcTask(true); });
        queue->WaitTasks();
    }
    TaskManager::DestroyTaskQueue(queue);
}

}  // namespace ark::taskmanager
