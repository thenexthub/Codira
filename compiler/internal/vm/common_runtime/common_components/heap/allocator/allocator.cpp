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
#include "common_components/heap/allocator/allocator.h"

#include <cinttypes>
#include <cstdint>

#include "common_components/base/immortal_wrapper.h"
#include "common_components/common/base_object.h"
#include "common_components/heap/allocator/regional_heap.h"
#include "common_components/mutator/thread_local.h"

namespace common {
using namespace std;
Allocator::Allocator()
{
    allocBufferManager_ = new (std::nothrow) AllocBufferManager();
    LOGF_CHECK(allocBufferManager_ != nullptr) << "new alloc buffer manager failed";
    asyncAllocationInitSwitch_ = InitAyncAllocation();
    isAsyncAllocationEnable_.store(asyncAllocationInitSwitch_);
}

bool Allocator::InitAyncAllocation()
{
    auto enableAsyncAllocation = std::getenv("arkEnableAsyncAllocation");
    if (enableAsyncAllocation == nullptr) {
#if defined(PANDA_TARGET_OHOS)
        return true;
#else
        return false;
#endif
    }
    if (strlen(enableAsyncAllocation) != 1) {
        LOG_COMMON(ERROR) << "Unsupported arkEnableAsyncAllocation, arkEnableAsyncAllocation should be 0 or 1.\n";
#if defined(PANDA_TARGET_OHOS)
        return true;
#else
        return false;
#endif
    }

    switch (enableAsyncAllocation[0]) {
        case '0':
            return false;
        case '1':
            return true;
        default:
            LOG_COMMON(ERROR) << "Unsupported arkEnableAsyncAllocation, arkEnableAsyncAllocation should be 0 or 1.\n";
    }
    return true;
}

// PageAlllocator
// the input parameter cat should be guaranteed in the range of value of enum type AllocationTag by
// external invoker, in order to avoid exceed the border of matrix
AggregateAllocator& AggregateAllocator::Instance(AllocationTag tag)
{
    static ImmortalWrapper<AggregateAllocator> instance[MAX_ALLOCATION_TAG];
    return *(instance[tag]);
}

// PagePool
PagePool& PagePool::Instance() noexcept
{
    static ImmortalWrapper<PagePool> instance("PagePool");
    return *instance;
}

Allocator* Allocator::CreateAllocator()
{
    RegionalHeap* heapSpace = new (std::nothrow) RegionalHeap();
    LOGF_CHECK(heapSpace != nullptr) << "New RegionalHeap failed";
    return heapSpace;
}
} // namespace common
