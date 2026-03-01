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

#ifndef LIBPANDABASE_MEM_POOL_MAP_H
#define LIBPANDABASE_MEM_POOL_MAP_H

#include <cstddef>
#include <array>
#include "libarkbase/macros.h"
#include "mem.h"
#include "space.h"

WEAK_FOR_LTO_START

namespace ark {

enum class AllocatorType : uint16_t {
    UNDEFINED,
    RUNSLOTS_ALLOCATOR,
    FREELIST_ALLOCATOR,
    HUMONGOUS_ALLOCATOR,
    ARENA_ALLOCATOR,
    TLAB_ALLOCATOR,
    BUMP_ALLOCATOR,
    REGION_ALLOCATOR,
    FRAME_ALLOCATOR,
    STACK_LIKE_ALLOCATOR,
    BUMP_ALLOCATOR_WITH_TLABS,
    NATIVE_STACKS_ALLOCATOR,
};

class AllocatorInfo {
public:
    explicit constexpr AllocatorInfo(AllocatorType type, const void *addr) : type_(type), headerAddr_(addr)
    {
        // We can't create AllocatorInfo without correct pointer to the allocator header
        ASSERT(headerAddr_ != nullptr);
    }

    AllocatorType GetType() const
    {
        return type_;
    }

    const void *GetAllocatorHeaderAddr() const
    {
        return headerAddr_;
    }

    virtual ~AllocatorInfo() = default;

    DEFAULT_COPY_SEMANTIC(AllocatorInfo);
    DEFAULT_MOVE_SEMANTIC(AllocatorInfo);

private:
    AllocatorType type_;
    const void *headerAddr_;
};

// PoolMap is used to manage all pools which has been given to the allocators.
// It can be used to find which allocator has been used to allocate an object.
class PoolMap {
public:
    PANDA_PUBLIC_API void AddPoolToMap(const void *poolAddr, size_t poolSize, SpaceType spaceType,
                                       AllocatorType allocatorType, const void *allocatorAddr);
    PANDA_PUBLIC_API void RemovePoolFromMap(const void *poolAddr, size_t poolSize);
    // Get Allocator info for the object allocated at this address.
    PANDA_PUBLIC_API AllocatorInfo GetAllocatorInfo(const void *addr) const;

    PANDA_PUBLIC_API void *GetFirstByteOfPoolForAddr(const void *addr) const;

    PANDA_PUBLIC_API SpaceType GetSpaceType(const void *addr) const;

    PANDA_PUBLIC_API bool IsEmpty() const;

private:
    static constexpr uint64_t POOL_MAP_COVERAGE = PANDA_MAX_HEAP_SIZE;
    static constexpr size_t POOL_MAP_GRANULARITY_SHIFT = 18;
    static constexpr size_t POOL_MAP_GRANULARITY_MASK = (1U << POOL_MAP_GRANULARITY_SHIFT) - 1U;
    static_assert(PANDA_POOL_ALIGNMENT_IN_BYTES == 1U << POOL_MAP_GRANULARITY_SHIFT);
    static constexpr size_t POOL_MAP_SIZE = POOL_MAP_COVERAGE >> POOL_MAP_GRANULARITY_SHIFT;

    static constexpr bool FIRST_BYTE_IN_SEGMENT_VALUE = true;

    using MapNumType = size_t;

    class PoolInfo {
    public:
        void Initialize(MapNumType segmentFirstMapNum, SpaceType spaceType, AllocatorType allocatorType,
                        const void *allocatorAddr)
        {
            ASSERT(allocatorType_ == AllocatorType::UNDEFINED);
            // Added a TSAN ignore here because TSAN thinks
            // that we can have a data race here - concurrent
            // initialization and reading.
            // However, we can't get an access for this fields
            // without initialization in the correct flow.
            TSAN_ANNOTATE_IGNORE_WRITES_BEGIN();
            segmentFirstMapNum_ = segmentFirstMapNum;
            allocatorAddr_ = allocatorAddr;
            spaceType_ = spaceType;
            allocatorType_ = allocatorType;
            TSAN_ANNOTATE_IGNORE_WRITES_END();
        }

        inline bool IsEmpty() const
        {
            return spaceType_ == SpaceType::SPACE_TYPE_UNDEFINED;
        }

        void Destroy()
        {
            segmentFirstMapNum_ = 0;
            allocatorAddr_ = nullptr;
            allocatorType_ = AllocatorType::UNDEFINED;
            spaceType_ = SpaceType::SPACE_TYPE_UNDEFINED;
        }

        MapNumType GetSegmentFirstMapNum() const
        {
            ASSERT(spaceType_ != SpaceType::SPACE_TYPE_UNDEFINED);
            return segmentFirstMapNum_;
        }

        AllocatorType GetAllocatorType() const
        {
            return allocatorType_;
        }

        const void *GetAllocatorAddr() const
        {
            return allocatorAddr_;
        }

        SpaceType GetSpaceType() const
        {
            return spaceType_;
        }

    private:
        AllocatorType allocatorType_ {AllocatorType::UNDEFINED};
        SpaceType spaceType_ {SpaceType::SPACE_TYPE_UNDEFINED};
        MapNumType segmentFirstMapNum_ {0};
        const void *allocatorAddr_ = nullptr;
    };

    static MapNumType AddrToMapNum(const void *addr)
    {
        MapNumType mapNum = ToUintPtr(addr) >> POOL_MAP_GRANULARITY_SHIFT;
        ASSERT(mapNum < POOL_MAP_SIZE);
        return mapNum;
    }

    static void *MapNumToAddr(MapNumType mapNum)
    {
        // Checking overflow
        ASSERT(static_cast<uint64_t>(mapNum) << POOL_MAP_GRANULARITY_SHIFT == mapNum << POOL_MAP_GRANULARITY_SHIFT);
        return ToVoidPtr(mapNum << POOL_MAP_GRANULARITY_SHIFT);
    }

    std::array<PoolInfo, POOL_MAP_SIZE> poolMap_;

    friend class PoolMapTest;
};

}  // namespace ark

WEAK_FOR_LTO_END

#endif  // LIBPANDABASE_MEM_POOL_MAP_H
