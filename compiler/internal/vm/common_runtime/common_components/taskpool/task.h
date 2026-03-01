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

#ifndef COMMON_COMPONENTS_TASKPOOL_TASK_H
#define COMMON_COMPONENTS_TASKPOOL_TASK_H

#include <atomic>
#include <condition_variable>
#include <mutex>

#include "common_interfaces/base/common.h"

namespace common {
enum class TaskType : uint8_t {
    PGO_RESET_OUT_PATH_TASK,
    PGO_DUMP_TASK,
    ALL,
};

static constexpr int32_t ALL_TASK_ID = -1;
// Tasks not managed by VM
static constexpr int32_t GLOBAL_TASK_ID = 0;

class Task {
public:
    explicit Task(int32_t id) : id_(id) {};
    virtual ~Task() = default;
    virtual bool Run(uint32_t threadIndex) = 0;

    NO_COPY_SEMANTIC_CC(Task);
    NO_MOVE_SEMANTIC_CC(Task);

    virtual TaskType GetTaskType() const
    {
        return TaskType::ALL;
    }

    int32_t GetId() const
    {
        return id_;
    }

    void Terminated()
    {
        terminate_ = true;
    }

    bool IsTerminate() const
    {
        return terminate_;
    }

private:
    int32_t id_ {0};
    volatile bool terminate_ {false};
};

class TaskPackMonitor {
public:
    explicit TaskPackMonitor(int posted, int capacity) : posted_(posted), capacity_(capacity)
    {
        DCHECK_CC(posted_ >= 0);
        DCHECK_CC(posted_ <= capacity_);
    }
    virtual ~TaskPackMonitor() = default;

    void WaitAllFinished()
    {
        std::unique_lock<std::mutex> lock(mutex_);
        while (posted_ > 0) {
            cv_.wait(lock);
        }
    }

    bool TryAddNewOne()
    {
        std::lock_guard<std::mutex> guard(mutex_);
        DCHECK_CC(running_ >= 0);
        if (posted_ < capacity_) {
            ++posted_;
            return true;
        }
        return false;
    }

    void NotifyFinishOne()
    {
        std::lock_guard<std::mutex> guard(mutex_);
        DCHECK_CC(posted_ >= 0);
        if (--posted_ == 0) {
            cv_.notify_all();
        }
    }

    bool WaitNextStepOrFinished()
    {
        std::unique_lock<std::mutex> lock(mutex_);
        if (terminated_) {
            return false;
        }
        cv_.wait(lock);
        if (terminated_) {
            return false;
        }
        return true;
    }

    bool TryStartStep()
    {
        std::lock_guard<std::mutex> guard(mutex_);
        if (terminated_) {
            return false;
        }
        ++running_;
        DCHECK_CC(running_ <= capacity_ + 1);
        return true;
    }

    void FinishStep()
    {
        std::lock_guard<std::mutex> guard(mutex_);
        DCHECK_CC(!terminated_);
        DCHECK_CC(running_ > 0);
        if (--running_ == 0) {
            terminated_ = true;
            cv_.notify_all();
        }
    }

    void WakeUpRunnerApproximately()
    {
        // This check may fail because is not inside lock, but for an approximate waking up it is ok
        size_t current = reinterpret_cast<std::atomic<size_t> *>(&running_)->load(std::memory_order_relaxed);
        if (UNLIKELY_CC(current < posted_)) {
            cv_.notify_one();
        }
    }

    NO_COPY_SEMANTIC_CC(TaskPackMonitor);
    NO_MOVE_SEMANTIC_CC(TaskPackMonitor);
private:
    size_t running_ {0};
    size_t posted_ {0};
    size_t capacity_ {0};
    bool terminated_ {false};
    std::condition_variable cv_;
    std::mutex mutex_;
};
}  // namespace common
#endif  // COMMON_COMPONENTS_TASKPOOL_TASK_H
