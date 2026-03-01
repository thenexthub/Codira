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

#include "runtime/include/gc_task.h"

#include "runtime/mem/gc/gc.h"

namespace ark {

std::atomic<uint32_t> GCTask::nextId_ = 1;

void GCTask::Run(mem::GC &gc)
{
    gc.WaitForGC(*this);
    gc.SetCanAddGCTask(true);
}

void GCTask::Release(mem::InternalAllocatorPtr allocator)
{
    allocator->Delete(this);
}

void GCTask::UpdateGCCollectionType(GCCollectionType gcCollectionType)
{
    ASSERT(gcCollectionType != GCCollectionType::NONE);
    if (gcCollectionType <= collectionType) {
        return;
    }
    collectionType = gcCollectionType;
}

std::ostream &operator<<(std::ostream &os, const GCTaskCause &cause)
{
    switch (cause) {
        case GCTaskCause::INVALID_CAUSE:
            os << "Invalid";
            break;
        case GCTaskCause::PYGOTE_FORK_CAUSE:
            os << "PygoteFork";
            break;
        case GCTaskCause::STARTUP_COMPLETE_CAUSE:
            os << "StartupComplete";
            break;
        case GCTaskCause::NATIVE_ALLOC_CAUSE:
            os << "NativeAlloc";
            break;
        case GCTaskCause::EXPLICIT_CAUSE:
            os << "Explicit";
            break;
        case GCTaskCause::HEAP_USAGE_THRESHOLD_CAUSE:
            os << "Threshold";
            break;
        case GCTaskCause::MIXED:
            os << "Mixed";
            break;
        case GCTaskCause::YOUNG_GC_CAUSE:
            os << "Young";
            break;
        case GCTaskCause::OOM_CAUSE:
            os << "OOM";
            break;
        case GCTaskCause::CROSSREF_CAUSE:
            os << "Crossref";
            break;
        default:
            LOG(FATAL, GC) << "Unknown gc cause";
            break;
    }
    return os;
}

std::ostream &operator<<(std::ostream &os, const GCCollectionType &collectionType)
{
    switch (collectionType) {
        case GCCollectionType::NONE:
            os << "NONE";
            break;
        case GCCollectionType::YOUNG:
            os << "YOUNG";
            break;
        case GCCollectionType::TENURED:
            os << "TENURED";
            break;
        case GCCollectionType::MIXED:
            os << "MIXED";
            break;
        case GCCollectionType::FULL:
            os << "FULL";
            break;
        default:
            LOG(FATAL, GC) << "Unknown collection type";
            break;
    }
    return os;
}

}  // namespace ark
