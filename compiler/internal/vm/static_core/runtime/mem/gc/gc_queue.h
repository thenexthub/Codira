/**
 * Copyright (c) 2021-2024 Huawei Device Co., Ltd.
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 * http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#ifndef PANDA_RUNTIME_GC_QUEUE_H
#define PANDA_RUNTIME_GC_QUEUE_H

#include <memory>

#include "runtime/include/gc_task.h"
#include "runtime/include/mem/panda_containers.h"
#include "runtime/include/mem/panda_smart_pointers.h"

namespace ark::mem {

constexpr uint64_t GC_WAIT_TIMEOUT = 500U;

class GCQueueInterface {
public:
    GCQueueInterface() = default;
    virtual ~GCQueueInterface() = default;

    virtual PandaUniquePtr<GCTask> GetTask(bool needWaitTask) = 0;

    virtual bool AddTask(PandaUniquePtr<GCTask> task) = 0;

    virtual bool IsEmpty() const = 0;

    virtual void Signal() = 0;

    virtual bool WaitForGCTask() = 0;

    NO_COPY_SEMANTIC(GCQueueInterface);
    NO_MOVE_SEMANTIC(GCQueueInterface);
};

/// @class GCQueueWithTime is an ascending priority queue ordered by target time.
class GCQueueWithTime : public GCQueueInterface {
public:
    explicit GCQueueWithTime(GC *gc) : gc_(gc) {}

    PandaUniquePtr<GCTask> GetTask(bool needWaitTask) override;

    bool AddTask(PandaUniquePtr<GCTask> task) override;

    bool IsEmpty() const override
    {
        os::memory::LockHolder lock(lock_);
        return queue_.empty();
    }

    void Signal() override
    {
        os::memory::LockHolder lock(lock_);
        condVar_.Signal();
    }

    bool WaitForGCTask() override
    {
        os::memory::LockHolder lock(lock_);
        return condVar_.TimedWait(&lock_, GC_WAIT_TIMEOUT);
    }

private:
    class CompareByTime {
    public:
        bool operator()(const PandaUniquePtr<GCTask> &left, const PandaUniquePtr<GCTask> &right) const
        {
            return left->GetTargetTime() > right->GetTargetTime();
        }
    };

    GC *gc_;
    mutable os::memory::Mutex lock_;
    PandaPriorityQueue<PandaUniquePtr<GCTask>, PandaVector<PandaUniquePtr<GCTask>>, CompareByTime> queue_
        GUARDED_BY(lock_);
    os::memory::ConditionVariable condVar_;
    const char *queueName_ = "GC queue ordered by time";
};

}  // namespace ark::mem

#endif  // PANDA_RUNTIME_GC_QUEUE_H
