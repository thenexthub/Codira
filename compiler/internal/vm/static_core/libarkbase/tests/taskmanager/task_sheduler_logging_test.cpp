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

#include "libarkbase/utils/logger.h"
#include "libarkbase/taskmanager/task_manager.h"

#include <cstdio>

#include <regex>
#include <sstream>
#include <gtest/gtest.h>

namespace ark::taskmanager {

TEST(TaskSchedulerMetrics, InfoLoggingTest)
{
    constexpr size_t THREADS_COUNT = 4;
    Logger::InitializeStdLogging(Logger::Level::INFO, ark::LOGGER_COMPONENT_MASK_ALL);
    EXPECT_TRUE(Logger::IsLoggingOn(Logger::Level::FATAL, Logger::Component::ALLOC));
    testing::internal::CaptureStderr();
    TaskManager::Start(THREADS_COUNT, TaskTimeStatsType::LIGHT_STATISTICS);
    auto *queue = TaskManager::CreateTaskQueue();

    queue->AddForegroundTask([]() {});
    queue->AddForegroundTask([]() {});

    TaskManager::DestroyTaskQueue(queue);
    TaskManager::Finish();

    std::string err = testing::internal::GetCapturedStderr();
    auto correctMessage = std::string(
        "\\[TID [0-9a-f]{6}\\] I/task_manager: TaskQueueId: [0-9]+; "
        "mean life time: [\\.0-9]+us; max life time: [\\.0-9]+us; mean execution time: "
        "[\\.0-9]+us; max execution time: [\\.0-9]+us; count of tasks: [0-9]+;\n");
    std::regex e(correctMessage);
    EXPECT_TRUE(std::regex_match(err, e)) << "real message: " << err << "expected to find: " << correctMessage;
}

TEST(TaskSchedulerMetrics, NoInfoLoggingTest)
{
    Logger::InitializeStdLogging(Logger::Level::INFO, ark::LOGGER_COMPONENT_MASK_ALL);
    EXPECT_TRUE(Logger::IsLoggingOn(Logger::Level::FATAL, Logger::Component::ALLOC));
    testing::internal::CaptureStderr();

    constexpr size_t THREADS_COUNT = 4;
    TaskManager::Start(THREADS_COUNT, TaskTimeStatsType::NO_STATISTICS);
    auto *queue = TaskManager::CreateTaskQueue();

    queue->AddBackgroundTask([]() {});
    queue->WaitTasks();

    std::string err = testing::internal::GetCapturedStderr();
    EXPECT_TRUE(err.empty()) << "it should be no message in output, but it's equal to " << err;

    TaskManager::DestroyTaskQueue(queue);
    TaskManager::Finish();
}

TEST(TaskTimeStatsTest, OperatorLessThanLessTest)
{
    {
        std::stringstream stream;
        stream << TaskTimeStatsType::NO_STATISTICS;
        EXPECT_EQ(stream.str(), "TaskTimeStatsType::NO_STATISTICS");
    }

    {
        std::stringstream stream;
        stream << TaskTimeStatsType::LIGHT_STATISTICS;
        EXPECT_EQ(stream.str(), "TaskTimeStatsType::LIGHT_STATISTICS");
    }
}

TEST(TaskTimeStatsTest, StringToTaskTimeStatsTest)
{
    EXPECT_EQ(StringToTaskTimeStats("no-task-stats"), TaskTimeStatsType::NO_STATISTICS);
    EXPECT_EQ(StringToTaskTimeStats("light-task-stats"), TaskTimeStatsType::LIGHT_STATISTICS);
}

}  // namespace ark::taskmanager
