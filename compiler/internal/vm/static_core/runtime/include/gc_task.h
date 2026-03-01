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

#ifndef PANDA_RUNTIME_GC_TASK_H_
#define PANDA_RUNTIME_GC_TASK_H_

#include <cstdint>

#include "runtime/include/mem/allocator.h"

namespace ark {

class ManagedThread;

namespace mem {
class GC;
}  // namespace mem

/// Causes are ordered by priority. Bigger index - means bigger priority in GC
enum class GCTaskCause : uint8_t {
    INVALID_CAUSE = 0,
    YOUNG_GC_CAUSE,  // if young space is full
    PYGOTE_FORK_CAUSE,
    STARTUP_COMPLETE_CAUSE,
    NATIVE_ALLOC_CAUSE,
    HEAP_USAGE_THRESHOLD_CAUSE,
    MIXED,           // startGC(mixed). In this case we ignore garbage percentage for tenured regions.
    EXPLICIT_CAUSE,  // System.gc
    OOM_CAUSE,       // if all heap is full
    CROSSREF_CAUSE,
};

/**
 * Collection types are ordered by gc collection scale
 *
 * @see GCTask::UpdateGCCollectionType
 */
enum class GCCollectionType : uint8_t { NONE = 0, YOUNG, MIXED, TENURED, FULL };

struct PANDA_PUBLIC_API GCTask {
    explicit GCTask(GCTaskCause gcReason) : GCTask(gcReason, 0U) {}

    explicit GCTask(GCTaskCause gcReason, uint64_t gcTargetTime)
    {
        this->reason = gcReason;
        this->targetTime_ = gcTargetTime;
        this->collectionType = GCCollectionType::NONE;
        this->id_ = nextId_++;
    }

    uint64_t GetId() const
    {
        return id_;
    }

    /**
     * Update collection type in the gc task if the new coolcetion type is bigger
     * @param gc_collection_type new gc collection type
     */
    void UpdateGCCollectionType(GCCollectionType gcCollectionType);

    uint64_t GetTargetTime() const
    {
        return targetTime_;
    }

    // NOLINTNEXTLINE(google-runtime-references)
    virtual void Run(mem::GC &gc);

    virtual void Release(mem::InternalAllocatorPtr allocator);

    virtual ~GCTask() = default;

    GCTask(const GCTask &other) = default;
    GCTask &operator=(const GCTask &other) = default;
    GCTask(GCTask &&other) = default;
    GCTask &operator=(GCTask &&other) = default;

    GCTaskCause reason;               // NOLINT(misc-non-private-member-variables-in-classes)
    GCCollectionType collectionType;  // NOLINT(misc-non-private-member-variables-in-classes)

private:
    uint32_t id_ = 0;
    uint64_t targetTime_;
    static std::atomic<uint32_t> nextId_;
};

std::ostream &operator<<(std::ostream &os, const GCTaskCause &cause);
std::ostream &operator<<(std::ostream &os, const GCCollectionType &collectionType);

}  // namespace ark

#endif  // PANDA_RUNTIME_GC_TASK_H_
