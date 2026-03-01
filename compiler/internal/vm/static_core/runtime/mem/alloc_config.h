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
#ifndef PANDA_RUNTIME_MEM_ALLOC_CONFIG_H
#define PANDA_RUNTIME_MEM_ALLOC_CONFIG_H

#include "runtime/include/object_header.h"
#include "runtime/arch/memory_helpers.h"
#include "runtime/mem/gc/heap-space-misc/crossing_map_singleton.h"
#include "libarkbase/mem/mem.h"
#include "libarkbase/mem/mem_range.h"
#include "runtime/mem/mem_stats_additional_info.h"
#include "runtime/mem/mem_stats_default.h"
#include "libarkbase/utils/tsan_interface.h"

namespace ark::mem {

/**
 * We want to record stats about allocations and free events. Allocators don't care about the type of allocated memory.
 * It could be raw memory for any reason or memory for object in the programming language. If it's a memory for object -
 * we can cast void* to object and get the specific size of this object, otherwise we should believe to allocator and
 * can record only approximate size. Because of this we force allocators to use specific config for their needs.
 */
/// Config for objects allocators with Crossing Map support.
class ObjectAllocConfigWithCrossingMap {
public:
    static void OnAlloc(size_t size, SpaceType typeMem, MemStatsType *memStats)
    {
        memStats->RecordAllocateObject(size, typeMem);
    }

    static void OnFree(size_t size, SpaceType typeMem, MemStatsType *memStats)
    {
        memStats->RecordFreeObject(size, typeMem);
    }

    /// @brief Initialize an object memory allocated by an allocator.
    static void MemoryInit(void *mem)
    {
        // zeroing according to newobj description in ISA
        TSAN_ANNOTATE_IGNORE_WRITES_BEGIN();
        memset_s(mem, ObjectHeader::ObjectHeaderSize(), 0, ObjectHeader::ObjectHeaderSize());
        TSAN_ANNOTATE_IGNORE_WRITES_END();
        // zero init should be visible from other threads even if pointer to object was fetched
        // without 'volatile' specifier so full memory barrier is required
        // required by some language-specs
        arch::FullMemoryBarrier();
    }

    /// @brief Record new allocation of an object and add it to Crossing Map.
    static void AddToCrossingMap(void *objAddr, size_t objSize)
    {
        CrossingMapSingleton::AddObject(objAddr, objSize);
    }

    /**
     * @brief Record free call of an object and remove it from Crossing Map.
     * @param obj_addr - pointer to the removing object (object header).
     * @param obj_size - size of the removing object.
     * @param next_obj_addr - pointer to the next object (object header). It can be nullptr.
     * @param prev_obj_addr - pointer to the previous object (object header). It can be nullptr.
     * @param prev_obj_size - size of the previous object.
     *        It is used check if previous object crosses the borders of the current map.
     */
    static void RemoveFromCrossingMap(void *objAddr, size_t objSize, void *nextObjAddr, void *prevObjAddr = nullptr,
                                      size_t prevObjSize = 0)
    {
        CrossingMapSingleton::RemoveObject(objAddr, objSize, nextObjAddr, prevObjAddr, prevObjSize);
    }

    /**
     * @brief Find and return the first object, which starts in an interval inclusively
     * or an object, which crosses the interval border.
     * It is essential to check the previous object of the returned object to make sure that
     * we find the first object, which crosses the border of this interval.
     * @param start_addr - pointer to the first byte of the interval.
     * @param end_addr - pointer to the last byte of the interval.
     * @return Returns the first object which starts inside an interval,
     *  or an object which crosses a border of this interval
     *  or nullptr
     */
    static void *FindFirstObjInCrossingMap(void *startAddr, void *endAddr)
    {
        return CrossingMapSingleton::FindFirstObject(startAddr, endAddr);
    }

    /**
     * @brief Initialize a Crossing map for the corresponding memory ranges.
     * @param start_addr - pointer to the first byte of the interval.
     * @param size - size of the interval.
     */
    static void InitializeCrossingMapForMemory(void *startAddr, size_t size)
    {
        return CrossingMapSingleton::InitializeCrossingMapForMemory(startAddr, size);
    }

    /**
     * @brief Remove a Crossing map for the corresponding memory ranges.
     * @param start_addr - pointer to the first byte of the interval.
     * @param size - size of the interval.
     */
    static void RemoveCrossingMapForMemory(void *startAddr, size_t size)
    {
        return CrossingMapSingleton::RemoveCrossingMapForMemory(startAddr, size);
    }

    /**
     * @brief Record new allocation of the young range.
     * @param mem_range - range of a new young memory.
     */
    static void OnInitYoungRegion(const MemRange &memRange)
    {
        CrossingMapSingleton::MarkCardsAsYoung(memRange);
    }
};

/// Config for objects allocators.
class ObjectAllocConfig {
public:
    static void OnAlloc(size_t size, SpaceType typeMem, MemStatsType *memStats)
    {
        memStats->RecordAllocateObject(size, typeMem);
    }

    static void OnFree(size_t size, SpaceType typeMem, MemStatsType *memStats)
    {
        memStats->RecordFreeObject(size, typeMem);
    }

    /// @brief Initialize an object memory allocated by an allocator.
    static void MemoryInit(void *mem)
    {
        // zeroing only ObjectHeader size according to newobj description in ISA
        TSAN_ANNOTATE_IGNORE_WRITES_BEGIN();
        memset_s(mem, ObjectHeader::ObjectHeaderSize(), 0, ObjectHeader::ObjectHeaderSize());
        TSAN_ANNOTATE_IGNORE_WRITES_END();
        // zero init should be visible from other threads even if pointer to object was fetched
        // without 'volatile' specifier so full memory barrier is required
        // required by some language-specs
        arch::FullMemoryBarrier();
    }

    // We don't use crossing map in this config.
    static void AddToCrossingMap([[maybe_unused]] void *objAddr, [[maybe_unused]] size_t objSize) {}

    // We don't use crossing map in this config.
    static void RemoveFromCrossingMap([[maybe_unused]] void *objAddr, [[maybe_unused]] size_t objSize,
                                      [[maybe_unused]] void *nextObjAddr = nullptr,
                                      [[maybe_unused]] void *prevObjAddr = nullptr,
                                      [[maybe_unused]] size_t prevObjSize = 0)
    {
    }

    // We don't use crossing map in this config.
    static void *FindFirstObjInCrossingMap([[maybe_unused]] void *startAddr, [[maybe_unused]] void *endAddr)
    {
        // We can't call CrossingMap when we don't use it
        ASSERT(startAddr == nullptr);
        return nullptr;
    }

    // We don't use crossing map in this config.
    static void InitializeCrossingMapForMemory([[maybe_unused]] void *startAddr, [[maybe_unused]] size_t size) {}

    // We don't use crossing map in this config.
    static void RemoveCrossingMapForMemory([[maybe_unused]] void *startAddr, [[maybe_unused]] size_t size) {}

    /**
     * @brief Record new allocation of the young range.
     * @param mem_range - range of a new young memory.
     */
    static void OnInitYoungRegion(const MemRange &memRange)
    {
        CrossingMapSingleton::MarkCardsAsYoung(memRange);
    }
};

/// Config for raw memory allocators.
class RawMemoryConfig {
public:
    static void OnAlloc(size_t size, SpaceType typeMem, MemStatsType *memStats)
    {
        ASSERT(typeMem == SpaceType::SPACE_TYPE_INTERNAL);
        memStats->RecordAllocateRaw(size, typeMem);
    }

    static void OnFree(size_t size, SpaceType typeMem, MemStatsType *memStats)
    {
        ASSERT(typeMem == SpaceType::SPACE_TYPE_INTERNAL);
        memStats->RecordFreeRaw(size, typeMem);
    }

    /// @brief We don't need it for raw memory.
    static void MemoryInit([[maybe_unused]] void *mem) {}

    // We don't use crossing map for raw memory allocations.
    static void AddToCrossingMap([[maybe_unused]] void *objAddr, [[maybe_unused]] size_t objSize) {}

    // We don't use crossing map for raw memory allocations.
    static void RemoveFromCrossingMap([[maybe_unused]] void *objAddr, [[maybe_unused]] size_t objSize,
                                      [[maybe_unused]] void *nextObjAddr = nullptr,
                                      [[maybe_unused]] void *prevObjAddr = nullptr,
                                      [[maybe_unused]] size_t prevObjSize = 0)
    {
    }

    // We don't use crossing map for raw memory allocations.
    static void *FindFirstObjInCrossingMap([[maybe_unused]] void *startAddr, [[maybe_unused]] void *endAddr)
    {
        // We can't call CrossingMap when we don't use it
        ASSERT(startAddr == nullptr);
        return nullptr;
    }

    // We don't use crossing map for raw memory allocations.
    static void InitializeCrossingMapForMemory([[maybe_unused]] void *startAddr, [[maybe_unused]] size_t size) {}

    // We don't use crossing map for raw memory allocations.
    static void RemoveCrossingMapForMemory([[maybe_unused]] void *startAddr, [[maybe_unused]] size_t size) {}

    // We don't need to track young memory for raw allocations.
    static void OnInitYoungRegion([[maybe_unused]] const MemRange &memRange) {}
};

/// Debug config with empty MemStats calls and with Crossing Map support.
class EmptyAllocConfigWithCrossingMap {
public:
    static void OnAlloc([[maybe_unused]] size_t size, [[maybe_unused]] SpaceType typeMem,
                        [[maybe_unused]] MemStatsType *memStats)
    {
    }

    static void OnFree([[maybe_unused]] size_t size, [[maybe_unused]] SpaceType typeMem,
                       [[maybe_unused]] MemStatsType *memStats)
    {
    }

    /// @brief Initialize memory for correct test execution.
    static void MemoryInit([[maybe_unused]] void *mem) {}

    /// @brief Record new allocation of an object and add it to Crossing Map.
    static void AddToCrossingMap(void *objAddr, size_t objSize)
    {
        CrossingMapSingleton::AddObject(objAddr, objSize);
    }

    /**
     * @brief Record free call of an object and remove it from Crossing Map.
     * @param obj_addr - pointer to the removing object (object header).
     * @param obj_size - size of the removing object.
     * @param next_obj_addr - pointer to the next object (object header). It can be nullptr.
     * @param prev_obj_addr - pointer to the previous object (object header). It can be nullptr.
     * @param prev_obj_size - size of the previous object.
     *        It is used check if previous object crosses the borders of the current map.
     */
    static void RemoveFromCrossingMap(void *objAddr, size_t objSize, void *nextObjAddr, void *prevObjAddr = nullptr,
                                      size_t prevObjSize = 0)
    {
        CrossingMapSingleton::RemoveObject(objAddr, objSize, nextObjAddr, prevObjAddr, prevObjSize);
    }

    /**
     * @brief Find and return the first object, which starts in an interval inclusively
     * or an object, which crosses the interval border.
     * It is essential to check the previous object of the returned object to make sure that
     * we find the first object, which crosses the border of this interval.
     * @param start_addr - pointer to the first byte of the interval.
     * @param end_addr - pointer to the last byte of the interval.
     * @return Returns the first object which starts inside an interval,
     *  or an object which crosses a border of this interval
     *  or nullptr
     */
    static void *FindFirstObjInCrossingMap(void *startAddr, void *endAddr)
    {
        return CrossingMapSingleton::FindFirstObject(startAddr, endAddr);
    }

    /**
     * @brief Initialize a Crossing map for the corresponding memory ranges.
     * @param start_addr - pointer to the first byte of the interval.
     * @param size - size of the interval.
     */
    static void InitializeCrossingMapForMemory(void *startAddr, size_t size)
    {
        return CrossingMapSingleton::InitializeCrossingMapForMemory(startAddr, size);
    }

    /**
     * @brief Remove a Crossing map for the corresponding memory ranges.
     * @param start_addr - pointer to the first byte of the interval.
     * @param size - size of the interval.
     */
    static void RemoveCrossingMapForMemory(void *startAddr, size_t size)
    {
        return CrossingMapSingleton::RemoveCrossingMapForMemory(startAddr, size);
    }

    // We don't need to track young memory for these allocations.
    static void OnInitYoungRegion([[maybe_unused]] const MemRange &memRange) {}
};

/*
 * Config for disuse of stats for memory allocators
 */
class EmptyMemoryConfig {
public:
    ALWAYS_INLINE static void OnAlloc([[maybe_unused]] size_t size, [[maybe_unused]] SpaceType typeMem,
                                      [[maybe_unused]] MemStatsType *memStats)
    {
    }
    ALWAYS_INLINE static void OnFree([[maybe_unused]] size_t size, [[maybe_unused]] SpaceType typeMem,
                                     [[maybe_unused]] MemStatsType *memStats)
    {
    }
    ALWAYS_INLINE static void MemoryInit([[maybe_unused]] void *mem) {}
    ALWAYS_INLINE static void AddToCrossingMap([[maybe_unused]] void *objAddr, [[maybe_unused]] size_t objSize) {}
    ALWAYS_INLINE static void RemoveFromCrossingMap([[maybe_unused]] void *objAddr, [[maybe_unused]] size_t objSize,
                                                    [[maybe_unused]] void *nextObjAddr = nullptr,
                                                    [[maybe_unused]] void *prevObjAddr = nullptr,
                                                    [[maybe_unused]] size_t prevObjSize = 0)
    {
    }

    ALWAYS_INLINE static void *FindFirstObjInCrossingMap([[maybe_unused]] void *startAddr,
                                                         [[maybe_unused]] void *endAddr)
    {
        // We can't call CrossingMap when we don't use it
        ASSERT(startAddr == nullptr);
        return nullptr;
    }

    static void InitializeCrossingMapForMemory([[maybe_unused]] void *startAddr, [[maybe_unused]] size_t size) {}

    static void RemoveCrossingMapForMemory([[maybe_unused]] void *startAddr, [[maybe_unused]] size_t size) {}

    static void OnInitYoungRegion([[maybe_unused]] const MemRange &memRange) {}
};

}  // namespace ark::mem
#endif  // PANDA_RUNTIME_MEM_ALLOC_CONFIG_H
