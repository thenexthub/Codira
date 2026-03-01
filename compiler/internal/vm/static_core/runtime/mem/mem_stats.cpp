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

#include "runtime/mem/mem_stats.h"

#include "libarkbase/utils/utf.h"
#include "runtime/include/class.h"
#include "runtime/include/runtime.h"
#include "runtime/mem/mem_stats_additional_info.h"
#include "runtime/mem/mem_stats_default.h"
#include "runtime/mem/object_helpers.h"

namespace ark::mem {

template <typename T>
void MemStats<T>::RecordAllocateObject(size_t size, SpaceType typeMem)
{
    RecordAllocateObjects(1, size, typeMem);
}

template <typename T>
void MemStats<T>::RecordAllocateObjects(size_t totalObjectNum, size_t totalObjectSize, SpaceType typeMem)
{
    ASSERT(IsHeapSpace(typeMem));
    RecordAllocate(totalObjectSize, typeMem);
    if (typeMem == SpaceType::SPACE_TYPE_HUMONGOUS_OBJECT) {
        // Atomic with acq_rel order reason: data race with humongous_objects_allocated_ with dependecies on reads after
        // the load and on writes before the store
        humongousObjectsAllocated_.fetch_add(totalObjectNum, std::memory_order_acq_rel);
    } else {
        // Atomic with acq_rel order reason: data race with objects_allocated_ with dependecies on reads after the load
        // and on writes before the store
        objectsAllocated_.fetch_add(totalObjectNum, std::memory_order_acq_rel);
    }
}

template <typename T>
void MemStats<T>::RecordYoungMovedObjects(size_t youngObjectNum, size_t size, SpaceType typeMem)
{
    ASSERT(IsHeapSpace(typeMem));
    RecordMoved(size, typeMem);
    // We can't move SPACE_TYPE_HUMONGOUS_OBJECT
    ASSERT(typeMem != SpaceType::SPACE_TYPE_HUMONGOUS_OBJECT);
    // Atomic with acq_rel order reason: data race with last_young_objects_moved_bytes_ with dependecies on reads after
    // the load and on writes before the store
    lastYoungObjectsMovedBytes_.fetch_add(size, std::memory_order_acq_rel);
    // Atomic with acq_rel order reason: data race with objects_allocated_ with dependecies on reads after the load and
    // on writes before the store
    [[maybe_unused]] uint64_t oldVal = objectsAllocated_.fetch_sub(youngObjectNum, std::memory_order_acq_rel);
    ASSERT(oldVal >= youngObjectNum);
}

template <typename T>
void MemStats<T>::RecordTenuredMovedObjects(size_t tenuredObjectNum, size_t size, SpaceType typeMem)
{
    ASSERT(IsHeapSpace(typeMem));
    RecordMoved(size, typeMem);
    // We can't move SPACE_TYPE_HUMONGOUS_OBJECT
    ASSERT(typeMem != SpaceType::SPACE_TYPE_HUMONGOUS_OBJECT);
    // Atomic with acq_rel order reason: data race with objects_allocated_ with dependecies on reads after the load and
    // on writes before the store
    [[maybe_unused]] uint64_t oldVal = objectsAllocated_.fetch_sub(tenuredObjectNum, std::memory_order_acq_rel);
    ASSERT(oldVal >= tenuredObjectNum);
}

template <typename T>
void MemStats<T>::RecordFreeObject(size_t objectSize, SpaceType typeMem)
{
    RecordFreeObjects(1, objectSize, typeMem);
}

template <typename T>
void MemStats<T>::RecordFreeObjects(size_t totalObjectNum, size_t totalObjectSize, SpaceType typeMem)
{
    ASSERT(IsHeapSpace(typeMem));
    RecordFree(totalObjectSize, typeMem);
    if (typeMem == SpaceType::SPACE_TYPE_HUMONGOUS_OBJECT) {
        // Atomic with acq_rel order reason: data race with humongous_objects_freed_ with dependecies on reads after the
        // load and on writes before the store
        humongousObjectsFreed_.fetch_add(totalObjectNum, std::memory_order_acq_rel);
    } else {
        // Atomic with acq_rel order reason: data race with objects_freed_ with dependecies on reads after the load and
        // on writes before the store
        objectsFreed_.fetch_add(totalObjectNum, std::memory_order_acq_rel);
    }
}

template <typename T>
PandaString MemStats<T>::GetStatistics()
{
    PandaStringStream statistic;
    statistic << "memory statistics:" << std::endl;
    statistic << "heap: allocated - " << GetAllocatedHeap() << ", freed - " << GetFreedHeap() << std::endl;
    statistic << "raw memory: allocated - " << GetAllocated(SpaceType::SPACE_TYPE_INTERNAL) << ", freed - "
              << GetFreed(SpaceType::SPACE_TYPE_INTERNAL) << std::endl;
    statistic << "compiler: allocated - " << GetAllocated(SpaceType::SPACE_TYPE_CODE) << std::endl;
    statistic << "ArenaAllocator: allocated - " << GetAllocated(SpaceType::SPACE_TYPE_COMPILER) << std::endl;
    statistic << "total footprint now - " << GetTotalFootprint() << std::endl;
    statistic << "total allocated object - " << GetTotalObjectsAllocated() << std::endl;
    auto additionalStatistics = static_cast<T *>(this)->GetAdditionalStatistics();
    return statistic.str() + additionalStatistics;
}

template <typename T>
[[nodiscard]] uint64_t MemStats<T>::GetTotalObjectsAllocated() const
{
    // Atomic with acquire order reason: data race with objects_allocated_ with dependecies on reads after the load
    // which should become visible
    return objectsAllocated_.load(std::memory_order_acquire);
}

template <typename T>
[[nodiscard]] uint64_t MemStats<T>::GetTotalObjectsFreed() const
{
    // Atomic with acquire order reason: data race with objects_freed_ with dependecies on reads after the load which
    // should become visible
    return objectsFreed_.load(std::memory_order_acquire);
}

template <typename T>
[[nodiscard]] uint64_t MemStats<T>::GetTotalRegularObjectsAllocated() const
{
    return GetTotalObjectsAllocated() - GetTotalHumongousObjectsAllocated();
}

template <typename T>
[[nodiscard]] uint64_t MemStats<T>::GetTotalRegularObjectsFreed() const
{
    return GetTotalObjectsFreed() - GetTotalHumongousObjectsFreed();
}

template <typename T>
[[nodiscard]] uint64_t MemStats<T>::GetTotalHumongousObjectsAllocated() const
{
    // Atomic with acquire order reason: data race with humongous_objects_allocated_ with dependecies on reads after the
    // load which should become visible
    return humongousObjectsAllocated_.load(std::memory_order_acquire);
}

template <typename T>
[[nodiscard]] uint64_t MemStats<T>::GetTotalHumongousObjectsFreed() const
{
    // Atomic with acquire order reason: data race with humongous_objects_freed_ with dependecies on reads after the
    // load which should become visible
    return humongousObjectsFreed_.load(std::memory_order_acquire);
}

template <typename T>
[[nodiscard]] uint64_t MemStats<T>::GetObjectsCountAlive() const
{
    return GetTotalObjectsAllocated() - GetTotalObjectsFreed();
}

template <typename T>
[[nodiscard]] uint64_t MemStats<T>::GetRegularObjectsCountAlive() const
{
    return GetTotalRegularObjectsAllocated() - GetTotalRegularObjectsFreed();
}

template <typename T>
[[nodiscard]] uint64_t MemStats<T>::GetHumonguousObjectsCountAlive() const
{
    return GetTotalHumongousObjectsAllocated() - GetTotalHumongousObjectsFreed();
}

template <typename T>
[[nodiscard]] uint64_t MemStats<T>::GetLastYoungObjectsMovedBytes() const
{
    // Atomic with acquire order reason: data race with last_young_objects_moved_bytes_ with dependecies on reads after
    // the load which should become visible
    return lastYoungObjectsMovedBytes_.load(std::memory_order_acquire);
}

template class MemStats<MemStatsDefault>;
template class MemStats<MemStatsAdditionalInfo>;
}  // namespace ark::mem
