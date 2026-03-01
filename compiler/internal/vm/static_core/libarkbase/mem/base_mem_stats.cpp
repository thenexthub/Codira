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

#include "libarkbase/os/mutex.h"
#include "base_mem_stats.h"
#include "libarkbase/utils/logger.h"
#include "libarkbase/utils/type_helpers.h"

#include <numeric>
namespace ark {

void BaseMemStats::RecordAllocateRaw(size_t size, SpaceType typeMem)
{
    ASSERT(!IsHeapSpace(typeMem));
    RecordAllocate(size, typeMem);
}

void BaseMemStats::RecordAllocate(size_t size, SpaceType typeMem)
{
    auto index = helpers::ToUnderlying(typeMem);
    // Atomic with acq_rel order reason: data race with allocated_ with dependecies on reads after the load and on
    // writes before the store
    allocated_[index].fetch_add(size, std::memory_order_acq_rel);
}

void BaseMemStats::RecordMoved(size_t size, SpaceType typeMem)
{
    auto index = helpers::ToUnderlying(typeMem);
    // Atomic with acq_rel order reason: data race with allocated_ with dependecies on reads after the load and on
    // writes before the store
    uint64_t oldValue = allocated_[index].fetch_sub(size, std::memory_order_acq_rel);
    (void)oldValue;
    ASSERT(oldValue >= size);
}

void BaseMemStats::RecordFreeRaw(size_t size, SpaceType typeMem)
{
    ASSERT(!IsHeapSpace(typeMem));
    RecordFree(size, typeMem);
}

void BaseMemStats::RecordFree(size_t size, SpaceType typeMem)
{
    auto index = helpers::ToUnderlying(typeMem);
    // Atomic with acq_rel order reason: data race with allocated_ with dependecies on reads after the load and on
    // writes before the store
    freed_[index].fetch_add(size, std::memory_order_acq_rel);
}

uint64_t BaseMemStats::GetAllocated(SpaceType typeMem) const
{
    // Atomic with acquire order reason: data race with allocated_ with dependecies on reads after the load which should
    // become visible
    return allocated_[helpers::ToUnderlying(typeMem)].load(std::memory_order_acquire);
}

uint64_t BaseMemStats::GetFreed(SpaceType typeMem) const
{
    // Atomic with acquire order reason: data race with allocated_ with dependecies on reads after the load which should
    // become visible
    return freed_[helpers::ToUnderlying(typeMem)].load(std::memory_order_acquire);
}

uint64_t BaseMemStats::GetAllocatedHeap() const
{
    uint64_t result = 0;
    for (size_t index = 0; index < SPACE_TYPE_SIZE; index++) {
        SpaceType type = ToSpaceType(index);
        if (IsHeapSpace(type)) {
            // Atomic with acquire order reason: data race with allocated_ with dependecies on reads after the load
            // which should become visible
            result += allocated_[index].load(std::memory_order_acquire);
        }
    }
    return result;
}

uint64_t BaseMemStats::GetFreedHeap() const
{
    uint64_t result = 0;
    for (size_t index = 0; index < SPACE_TYPE_SIZE; index++) {
        SpaceType type = ToSpaceType(index);
        if (IsHeapSpace(type)) {
            // Atomic with acquire order reason: data race with allocated_ with dependecies on reads after the load
            // which should become visible
            result += freed_[index].load(std::memory_order_acquire);
        }
    }
    return result;
}

uint64_t BaseMemStats::GetFootprintHeap() const
{
    return helpers::UnsignedDifferenceUint64(GetAllocatedHeap(), GetFreedHeap());
}

uint64_t BaseMemStats::GetFootprint(SpaceType typeMem) const
{
    auto index = helpers::ToUnderlying(typeMem);
    // Atomic with acquire order reason: data race with allocated_ with dependecies on reads after the load which should
    // become visible
    LOG_IF(allocated_[index].load(std::memory_order_acquire) < freed_[index].load(std::memory_order_acquire), FATAL, GC)
        << "Allocated < Freed (mem type = " << std::dec
        << static_cast<size_t>(index)
        // Atomic with acquire order reason: data race with allocated_ with dependecies on reads after the load which
        // should become visible
        << "): " << allocated_[index].load(std::memory_order_acquire)
        << " < "
        // Atomic with acquire order reason: data race with allocated_ with dependecies on reads after the load which
        // should become visible
        << freed_[index].load(std::memory_order_acquire);
    // Atomic with acquire order reason: data race with allocated_ with dependecies on reads after the load which should
    // become visible
    return allocated_[index].load(std::memory_order_acquire) - freed_[index].load(std::memory_order_acquire);
}

uint64_t BaseMemStats::GetTotalFootprint() const
{
    return std::accumulate(begin(allocated_), end(allocated_), 0UL) - std::accumulate(begin(freed_), end(freed_), 0UL);
}

}  // namespace ark
