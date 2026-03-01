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

#ifndef PANDA_LIBPANDABASE_TASKMANAGER_TASK_H
#define PANDA_LIBPANDABASE_TASKMANAGER_TASK_H

#include "libarkbase/macros.h"
#include "libarkbase/taskmanager/task_manager_common.h"

#include <memory>

namespace ark::taskmanager {
class TaskQueueInterface;
}  // namespace ark::taskmanager

namespace ark::taskmanager::internal {

/**
 * @brief TaskLifeTimeAggregator structure used to save and log life time of task and execution time.
 * It's methods doesn't works on release build.
 */
class TaskLifeTimeAggregator {
public:
    NO_COPY_SEMANTIC(TaskLifeTimeAggregator);
    DEFAULT_MOVE_SEMANTIC(TaskLifeTimeAggregator);
    PANDA_PUBLIC_API TaskLifeTimeAggregator() = default;
    PANDA_PUBLIC_API ~TaskLifeTimeAggregator() = default;
    /// @brief Saves time of task adding in queue.
    PANDA_PUBLIC_API void GetAndStoreTimeOfTaskAddingToQueue();
    /// @brief Saves start time of execution. Use this method before task execution start
    PANDA_PUBLIC_API void GetAndStoreTimeOfTaskExecutionStart();
    /// @brief Logs life time and execution time of task. Use this method after task execution end
    PANDA_PUBLIC_API void GetTimeOfTaskExecutionFinishAndStoreTimeStats(TaskQueueInterface *queue);

private:
    // Time points for logs
    uint64_t addingToQueueTime_ {0};
    uint64_t startExecutionTime_ {0};
};

class Task {
public:
    using OnDestructionCallback = void (*)(TaskQueueInterface *, void *);
    using Ptr = Task *;

    PANDA_PUBLIC_API Task() = default;
    PANDA_PUBLIC_API ~Task() = default;
    NO_COPY_SEMANTIC(Task);
    DEFAULT_MOVE_SEMANTIC(Task);

    /**
     * @brief Tasks are created through this method with the specified arguments.
     * @param properties - properties of task, it contains TaskType, VMType and ExecutionMote.
     * @param runner - body of task, that will be executed.
     */
    [[nodiscard]] PANDA_PUBLIC_API static Ptr Create(void *buf, void *node, RunnerCallback runner,
                                                     TaskQueueInterface *queue, OnDestructionCallback callback);

    PANDA_PUBLIC_API static void Delete(Ptr task);

    /// @brief Executes body of task
    PANDA_PUBLIC_API void RunTask();
    /// @brief Makes task invalid, it should not be executed anymore.
    PANDA_PUBLIC_API void MakeInvalid();
    /// @brief Checks if task is invalid
    PANDA_PUBLIC_API bool IsInvalid() const;

private:
    Task(RunnerCallback runner, TaskQueueInterface *queue, OnDestructionCallback callback, void *node)
        : runner_(std::move(runner)), parentQueue_(queue), onDestructionCallback_(callback), nodePtr_(node)
    {
        ASSERT(queue != nullptr);
        ASSERT(callback != nullptr);
        EventOnTaskAdding();
    }

    PANDA_PUBLIC_API void EventOnTaskAdding();
    PANDA_PUBLIC_API void EventOnStartExecution();
    PANDA_PUBLIC_API void EventOnEndExecution();

    RunnerCallback runner_ {nullptr};
    TaskQueueInterface *parentQueue_ {nullptr};
    TaskLifeTimeAggregator lifeTimeStorage_;
    OnDestructionCallback onDestructionCallback_ {nullptr};
    void *nodePtr_ = nullptr;
};

using TaskPtr = Task::Ptr;

}  // namespace ark::taskmanager::internal

#endif  // PANDA_LIBPANDABASE_TASKMANAGER_TASK_H
