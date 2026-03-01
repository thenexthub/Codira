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

#ifndef COMMON_COMPONENTS_TASKPOOL_TASK_QUEUE_H
#define COMMON_COMPONENTS_TASKPOOL_TASK_QUEUE_H

#include <algorithm>
#include <atomic>
#include <chrono>
#include <deque>
#include <functional>
#include <map>
#include <memory>
#include <mutex>
#include <condition_variable>

#include "common_components/taskpool/task.h"
#include "common_interfaces/base/common.h"

namespace common {
using SteadyTimePoint = std::chrono::steady_clock::time_point;
class TaskQueue {
public:
    TaskQueue() = default;
    ~TaskQueue() = default;

    NO_COPY_SEMANTIC_CC(TaskQueue);
    NO_MOVE_SEMANTIC_CC(TaskQueue);

    void PostTask(std::unique_ptr<Task> task);
    void PostDelayedTask(std::unique_ptr<Task> task, uint64_t delayMilliseconds);
    std::unique_ptr<Task> PopTask();

    void Terminate();
    void TerminateTask(int32_t id, TaskType type);
    void ForEachTask(const std::function<void(Task*)> &f);

private:
    void MoveExpiredTask(std::unique_lock<std::mutex> &lock);
    void WaitForTask(std::unique_lock<std::mutex> &lock);

    std::deque<std::unique_ptr<Task>> tasks_;

    struct DelayedTaskCompare {
        bool operator()(const SteadyTimePoint& left, const SteadyTimePoint& right) const
        {
            return (std::chrono::duration_cast<std::chrono::duration<double>>(right - left)).count() > 0;
        }
    };

    std::multimap<SteadyTimePoint, std::unique_ptr<Task>, DelayedTaskCompare> delayedTasks_;

    std::atomic_bool terminate_ = false;
    std::mutex mtx_;
    std::condition_variable cv_;
};
}  // namespace common
#endif  // COMMON_COMPONENTS_TASKPOOL_TASK_QUEUE_H
