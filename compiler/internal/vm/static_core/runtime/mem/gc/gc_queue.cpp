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

#include "runtime/mem/gc/gc_queue.h"

#include "libarkbase/utils/time.h"
#include "runtime/mem/gc/gc.h"

namespace ark::mem {

constexpr int64_t NANOSECONDS_PER_MILLISEC = 1000000;

PandaUniquePtr<GCTask> GCQueueWithTime::GetTask(bool needWaitTask)
{
    os::memory::LockHolder lock(lock_);
    while (queue_.empty()) {
        if (!gc_->IsGCRunning()) {
            LOG(DEBUG, GC) << "GetTask() Return INVALID_CAUSE";
            return nullptr;
        }
        if (needWaitTask) {
            LOG(DEBUG, GC) << "Empty " << queueName_ << ", waiting...";
            condVar_.Wait(&lock_);
        } else {
            LOG(DEBUG, GC) << "Empty " << queueName_ << ", return nullptr";
            return nullptr;
        }
    }
    GCTask *task = queue_.top().get();
    auto currentTime = time::GetCurrentTimeInNanos();
    while (gc_->IsGCRunning() && (task->GetTargetTime() >= currentTime)) {
        auto delta = task->GetTargetTime() - currentTime;
        uint64_t ms = delta / NANOSECONDS_PER_MILLISEC;
        uint64_t ns = delta % NANOSECONDS_PER_MILLISEC;
        LOG(DEBUG, GC) << "GetTask TimedWait";
        condVar_.TimedWait(&lock_, ms, ns);
        task = queue_.top().get();
        currentTime = time::GetCurrentTimeInNanos();
    }
    PandaUniquePtr<GCTask> returnedTask = std::move(*const_cast<PandaUniquePtr<GCTask> *>(&queue_.top()));
    queue_.pop();
    LOG(DEBUG, GC) << "Extract a task from a " << queueName_;
    return returnedTask;
}

bool GCQueueWithTime::AddTask(PandaUniquePtr<GCTask> task)
{
    if (task == nullptr) {
        return false;
    }
    os::memory::LockHolder lock(lock_);
    if (!queue_.empty()) {
        auto *lastElem = queue_.top().get();
        if (lastElem->reason == task->reason) {
            // do not duplicate GC task with the same reason.
            return false;
        }
    }
    LOG(DEBUG, GC) << "Add task to a " << queueName_;
    queue_.push(std::move(task));
    condVar_.Signal();
    return true;
}

}  // namespace ark::mem
