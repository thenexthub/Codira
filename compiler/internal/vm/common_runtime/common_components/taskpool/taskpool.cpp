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

#include "common_components/taskpool/taskpool.h"

#include "common_components/platform/cpu.h"

namespace common {
Taskpool *Taskpool::GetCurrentTaskpool()
{
    static Taskpool *taskpool = new Taskpool();
    return taskpool;
}

void Taskpool::Initialize(int threadNum, std::function<void(os::thread::NativeHandleType)> prologueHook,
                          const std::function<void(os::thread::NativeHandleType)> epilogueHook)
{
    std::lock_guard<std::mutex> guard(mutex_);
    if (isInitialized_++ <= 0) {
        runner_ = std::make_unique<Runner>(TheMostSuitableThreadNum(threadNum), prologueHook, epilogueHook);
    }
}

void Taskpool::Destroy(int32_t id)
{
    std::lock_guard<std::mutex> guard(mutex_);
    if (isInitialized_ <= 0) {
        return;
    }
    isInitialized_--;
    if (isInitialized_ == 0) {
        runner_->TerminateThread();
    } else {
        runner_->TerminateTask(id, TaskType::ALL);
    }
}

void Taskpool::TerminateTask(int32_t id, TaskType type)
{
    if (isInitialized_ <= 0) {
        return;
    }
    runner_->TerminateTask(id, type);
}

uint32_t Taskpool::TheMostSuitableThreadNum(uint32_t threadNum) const
{
    if (threadNum > 0) {
        return std::min<uint32_t>(threadNum, MAX_TASKPOOL_THREAD_NUM);
    }
    uint32_t numOfThreads = std::min<uint32_t>(NumberOfCpuCore() / 2, MAX_TASKPOOL_THREAD_NUM);
    if (numOfThreads > MIN_TASKPOOL_THREAD_NUM) {
        return numOfThreads - 1;  // 1 for daemon thread.
    }
    return MIN_TASKPOOL_THREAD_NUM;  // At least MIN_TASKPOOL_THREAD_NUM GC threads, and 1 extra daemon thread.
}

void Taskpool::ForEachTask(const std::function<void(Task *)> &f)
{
    if (isInitialized_ <= 0) {
        return;
    }
    runner_->ForEachTask(f);
}
}  // namespace common

