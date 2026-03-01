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

#ifndef LIBPANDABASE_TASKMANAGER_UTILS_TASK_LIFETIME_STATISTICS_H
#define LIBPANDABASE_TASKMANAGER_UTILS_TASK_LIFETIME_STATISTICS_H

#include "libarkbase/macros.h"
#include "libarkbase/taskmanager/task.h"
#include "libarkbase/taskmanager/task_queue_interface.h"

#include <atomic>
#include <unordered_map>
#include <string>
#include <vector>
#include <array>

namespace ark::taskmanager {

enum class TaskTimeStatsType : uint8_t {
    /**
     * NO_STATISTICS type is used when we don't want to collect and output any data. It has lowest overhead and
     * should be used as default one.
     */
    NO_STATISTICS,
    /// LIGHT_STATISTICS type collects data only for mean and max times. It has low overhead.
    LIGHT_STATISTICS,
};

PANDA_PUBLIC_API TaskTimeStatsType StringToTaskTimeStats(std::string_view str);
PANDA_PUBLIC_API std::ostream &operator<<(std::ostream &os, TaskTimeStatsType type);

/// @brief TaskTimeStatsBase is interface of TaskTimeStats classes.
class TaskTimeStatsBase {
    NO_COPY_SEMANTIC(TaskTimeStatsBase);
    NO_MOVE_SEMANTIC(TaskTimeStatsBase);

public:
    TaskTimeStatsBase() = default;
    virtual ~TaskTimeStatsBase() = default;
    /**
     * @brief Method registers WorkerThread in TaskTimeStats structure. You should not register helper thread
     * that can execute tasks.
     */
    virtual void RegisterWorkerThread();
    /**
     * @brief Method saves information about life time and execution time of task with specified TaskProperties.
     * @param queue: parent queue of task, information about which should be saved
     * @param lifeTime: time in microseconds between adding task in queue and end of its execution
     * @param executionTime: time in microseconds it took to complete the task. It should be less or equal to lifeTime.
     */
    virtual void CollectLifeAndExecutionTimes(QueueId id, uint64_t lifeTime, uint64_t executionTime) = 0;
    /**
     * @brief Method returns vector of strings with statistics. Each string represents element of statistics. For
     * example one string can represent statistics for one unique TaskProperties.
     */
    virtual std::vector<std::string> GetTaskStatistics() = 0;

protected:
    using ContainerId = size_t;
    static constexpr size_t DEFAULT_CONTAINER_ID = 0U;
    static thread_local ContainerId containerId_;  // for no WorkerThread should be equal to DEFAULT_CONTAINER_ID

private:
    std::atomic_size_t countOfRegisteredWorkers_ {0U};
};

namespace internal {

/**
 * @brief LightTaskTimeTimeStats is TaskTimeStats class that collect info to get mean and max
 * times for each TaskProperties
 */
class LightTaskTimeTimeStats final : public TaskTimeStatsBase {
    NO_COPY_SEMANTIC(LightTaskTimeTimeStats);
    NO_MOVE_SEMANTIC(LightTaskTimeTimeStats);

    struct MeanTimeStats;

    static constexpr size_t STATISTICS_CONTAINER_SIZE = MAX_ID_COUNT;

    using StatisticsContainer = std::array<MeanTimeStats, STATISTICS_CONTAINER_SIZE>;
    using StatisticsContainerPerThread = std::vector<StatisticsContainer>;

    // NOLINTBEGIN(misc-non-private-member-variables-in-classes)
    struct MeanTimeStats {
        uint64_t sumLifeTime = 0;
        uint64_t sumExecutionTime = 0;
        uint64_t maxLifeTime = 0;
        uint64_t maxExecutionTime = 0;
        size_t countOfTasks = 0;
    };
    // NOLINTEND(misc-non-private-member-variables-in-classes)

public:
    PANDA_PUBLIC_API explicit LightTaskTimeTimeStats(size_t countOfWorkers);
    PANDA_PUBLIC_API ~LightTaskTimeTimeStats() override = default;

    void CollectLifeAndExecutionTimes(QueueId id, uint64_t lifeTime, uint64_t executionTime) override;

    std::vector<std::string> GetTaskStatistics() override;

private:
    std::string GetStatisticsForProperties(QueueId id);
    size_t GetCountOfTasksWithProperties(QueueId id);

    StatisticsContainerPerThread statisticsContainerPerThread_;
};

}  // namespace internal

}  // namespace ark::taskmanager

#endif  // LIBPANDABASE_TASKMANAGER_UTILS_TASK_LIFETIME_STATISTICS_H
