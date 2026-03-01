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

#ifndef PANDA_LIBPANDABASE_TASKMANAGER_SCHEDULABLE_TASK_QUEUE_INTERFACE_H
#define PANDA_LIBPANDABASE_TASKMANAGER_SCHEDULABLE_TASK_QUEUE_INTERFACE_H

#include "libarkbase/taskmanager/task.h"
#include "libarkbase/taskmanager/task_queue_interface.h"
#include "libarkbase/taskmanager/utils/task_time_stats.h"

namespace ark::taskmanager::internal {

class SchedulableTaskQueueInterface : public TaskQueueInterface {
public:
    NO_COPY_SEMANTIC(SchedulableTaskQueueInterface);
    NO_MOVE_SEMANTIC(SchedulableTaskQueueInterface);

    /**
     * NewTasksCallback instance should be called after tasks adding. It should have next arguments:
     * 1. property of tasks you added
     * 2. count of tasks you added
     */
    using SignalWorkersCallback = std::function<void()>;
    using SignalWaitersCallback = std::function<void()>;

    using CheckIfTimerThreadIsEnabledCallback = std::function<bool()>;

    using AddTaskToWorkerFunc = std::function<void(TaskPtr)>;
    using AddTaskToHelperFunc = std::function<void(TaskPtr)>;

    using GetSplittingFactorFunc = std::function<size_t()>;

    explicit SchedulableTaskQueueInterface(QueuePriority priority) : TaskQueueInterface(priority) {}
    ~SchedulableTaskQueueInterface() override = default;

    /// @brief Pops task from queue, return nullptr if it's empty.
    [[nodiscard]] virtual TaskPtr PopTask() = 0;
    /// @brief Pops foreground task from queue, return nullptr if no foreground tasks in queue.
    [[nodiscard]] virtual TaskPtr PopForegroundTask() = 0;
    /// @brief Pops background task from queue, return nullptr if no background tasks in queue.
    [[nodiscard]] virtual TaskPtr PopBackgroundTask() = 0;

    virtual size_t PopTasksToWorker(const AddTaskToWorkerFunc &addFrontendTaskFunc,
                                    const AddTaskToWorkerFunc &addBackgroundTaskFunc, size_t size) = 0;
    virtual size_t PopForegroundTasksToHelperThread(const AddTaskToHelperFunc &addTaskFunc, size_t size) = 0;
    virtual size_t PopBackgroundTasksToHelperThread(const AddTaskToHelperFunc &addTaskFunc, size_t size) = 0;

    virtual size_t GetCountOfLiveTasks() const = 0;
    virtual size_t GetCountOfLiveForegroundTasks() const = 0;
    virtual size_t GetCountOfLiveBackgroundTasks() const = 0;

    virtual TaskTimeStatsBase *GetTaskTimeStats() const = 0;

    void virtual SetCallbacks(SignalWorkersCallback signalWorkersCallback,
                              CheckIfTimerThreadIsEnabledCallback checkTimerThreadExistCallback) = 0;

    /// @brief Removes callback function.
    void virtual UnsetCallbacks() = 0;
};

}  // namespace ark::taskmanager::internal

#endif  // PANDA_LIBPANDABASE_TASKMANAGER_SCHEDULABLE_TASK_QUEUE_INTERFACE_H
