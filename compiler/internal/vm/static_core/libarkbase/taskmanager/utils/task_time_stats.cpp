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

#include "libarkbase/taskmanager/utils/task_time_stats.h"
#include "libarkbase/utils/time.h"

#include <sstream>
#include <numeric>
#include <cmath>
#include <ostream>

namespace ark::taskmanager {

TaskTimeStatsType StringToTaskTimeStats(std::string_view str)
{
    if (str == "no-task-stats") {
        return TaskTimeStatsType::NO_STATISTICS;
    }
    if (str == "light-task-stats") {
        return TaskTimeStatsType::LIGHT_STATISTICS;
    }
    UNREACHABLE();
}

std::ostream &operator<<(std::ostream &os, TaskTimeStatsType type)
{
    switch (type) {
        case TaskTimeStatsType::NO_STATISTICS:
            os << "TaskTimeStatsType::NO_STATISTICS";
            break;
        case TaskTimeStatsType::LIGHT_STATISTICS:
            os << "TaskTimeStatsType::LIGHT_STATISTICS";
            break;
        default:
            UNREACHABLE();
            break;
    }
    return os;
}

thread_local TaskTimeStatsBase::ContainerId TaskTimeStatsBase::containerId_ = TaskTimeStatsBase::DEFAULT_CONTAINER_ID;

void TaskTimeStatsBase::RegisterWorkerThread()
{
    // Atomic with acq_rel order reason: fetch_add operation should be sync with calls from other threads
    size_t registerNumber = countOfRegisteredWorkers_.fetch_add(1, std::memory_order_acq_rel);
    containerId_ = registerNumber + 1U;
}

namespace internal {

LightTaskTimeTimeStats::LightTaskTimeTimeStats(size_t countOfWorkers)
    : statisticsContainerPerThread_(countOfWorkers + 1U)
{
    for (auto &statisticsContainer : statisticsContainerPerThread_) {
        for (size_t i = 0; i < STATISTICS_CONTAINER_SIZE; i++) {
            statisticsContainer[i] = MeanTimeStats();
        }
    }
}

void LightTaskTimeTimeStats::CollectLifeAndExecutionTimes(QueueId id, uint64_t lifeTime, uint64_t executionTime)
{
    auto &meanTimeStats = statisticsContainerPerThread_[containerId_][id];

    meanTimeStats.sumExecutionTime += executionTime;
    meanTimeStats.sumLifeTime += lifeTime;

    meanTimeStats.maxExecutionTime = std::max(executionTime, meanTimeStats.maxExecutionTime);
    meanTimeStats.maxLifeTime = std::max(lifeTime, meanTimeStats.maxLifeTime);

    ASSERT(meanTimeStats.sumLifeTime >= meanTimeStats.sumExecutionTime);

    meanTimeStats.countOfTasks++;
}

std::vector<std::string> LightTaskTimeTimeStats::GetTaskStatistics()
{
    std::vector<std::string> resultData;
    for (QueueId id = 0; id < MAX_ID_COUNT; id++) {
        if (GetCountOfTasksWithProperties(id) == 0) {
            continue;
        }
        resultData.push_back(GetStatisticsForProperties(id));
    }
    return resultData;
}

std::string LightTaskTimeTimeStats::GetStatisticsForProperties(QueueId id)
{
    std::stringstream stream;
    uint64_t sumLifeTime = 0;
    uint64_t sumExecutionTime = 0;
    uint64_t maxLifeTime = 0;
    uint64_t maxExecutionTime = 0;
    size_t sumTaskCount = 0;
    for (const auto &statisticsContainer : statisticsContainerPerThread_) {
        const auto &meanStatistics = statisticsContainer[id];
        sumLifeTime += meanStatistics.sumLifeTime;
        sumExecutionTime += meanStatistics.sumExecutionTime;

        maxLifeTime = std::max(meanStatistics.maxLifeTime, maxLifeTime);
        maxExecutionTime = std::max(meanStatistics.maxExecutionTime, maxExecutionTime);

        sumTaskCount += meanStatistics.countOfTasks;
    }
    ASSERT(sumTaskCount != 0);
    auto meanLifeTime = static_cast<double>(sumLifeTime) / sumTaskCount;
    auto meanExecutionTime = static_cast<double>(sumExecutionTime) / sumTaskCount;
    stream << "TaskQueueId: " << id << "; mean life time: " << meanLifeTime << "us; max life time: " << maxLifeTime
           << "us; mean execution time: " << meanExecutionTime << "us; "
           << "max execution time: " << maxExecutionTime << "us; count of tasks: " << sumTaskCount << ";";
    return stream.str();
}

size_t LightTaskTimeTimeStats::GetCountOfTasksWithProperties(QueueId id)
{
    size_t sumTaskCount = 0;
    for (const auto &statisticsContainer : statisticsContainerPerThread_) {
        const auto &meanStatistics = statisticsContainer[id];
        sumTaskCount += meanStatistics.countOfTasks;
    }
    return sumTaskCount;
}

}  // namespace internal

}  // namespace ark::taskmanager
