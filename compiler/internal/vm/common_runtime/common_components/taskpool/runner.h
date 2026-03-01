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

#ifndef COMMON_COMPONENTS_TASKPOOL_RUNNER_H
#define COMMON_COMPONENTS_TASKPOOL_RUNNER_H

#include <array>
#include <memory>
#include <mutex>
#include <thread>
#include <vector>
#include <functional>

#include "common_components/taskpool/task_queue.h"
#include "common_interfaces/base/common.h"
#include "common_interfaces/base/os/thread.h"

namespace common {
static constexpr uint32_t MIN_TASKPOOL_THREAD_NUM = 3;
static constexpr uint32_t MAX_TASKPOOL_THREAD_NUM = 5;
static constexpr uint32_t DEFAULT_TASKPOOL_THREAD_NUM = 0;

enum class PriorityMode { STW, FOREGROUND, BACKGROUND };

class Runner {
public:
    explicit Runner(uint32_t threadNum, const std::function<void(os::thread::NativeHandleType)> prologueHook,
                    const std::function<void(os::thread::NativeHandleType)> epilogueHook);
    ~Runner() = default;

    NO_COPY_SEMANTIC_CC(Runner);
    NO_MOVE_SEMANTIC_CC(Runner);

    void PostTask(std::unique_ptr<Task> task)
    {
        taskQueue_.PostTask(std::move(task));
    }

    void PostDelayedTask(std::unique_ptr<Task> task, uint64_t delayMilliseconds)
    {
        taskQueue_.PostDelayedTask(std::move(task), delayMilliseconds);
    }

    void PUBLIC_API TerminateThread();
    void TerminateTask(int32_t id, TaskType type);
    void SetQosPriority(PriorityMode mode);
    void RecordThreadId();

    uint32_t GetTotalThreadNum() const
    {
        return totalThreadNum_;
    }

    bool IsInThreadPool(std::thread::id id)
    {
        std::lock_guard<std::mutex> guard(mtxPool_);
        for (auto &thread : threadPool_) {
            if (thread->get_id() == id) {
                return true;
            }
        }
        return false;
    }

    void PrologueHook(os::thread::NativeHandleType thread)
    {
        if (prologueHook_ != nullptr) {
            prologueHook_(thread);
        }
    }
    void EpilogueHook(os::thread::NativeHandleType thread)
    {
        if (epilogueHook_ != nullptr) {
            epilogueHook_(thread);
        }
    }
    void ForEachTask(const std::function<void(Task *)> &f);

private:
    void Run(uint32_t threadId);
    void SetRunTask(uint32_t threadId, Task *task);

    std::vector<std::unique_ptr<std::thread>> threadPool_ {};
    TaskQueue taskQueue_ {};
    std::array<Task *, MAX_TASKPOOL_THREAD_NUM + 1> runningTask_;
    uint32_t totalThreadNum_ {0};
    std::vector<uint32_t> gcThreadId_ {};
    std::mutex mtx_;
    std::mutex mtxPool_;

    std::function<void(os::thread::NativeHandleType)> prologueHook_;
    std::function<void(os::thread::NativeHandleType)> epilogueHook_;
};
}  // namespace common
#endif  // COMMON_COMPONENTS_TASKPOOL_RUNNER_H

