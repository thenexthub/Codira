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

#include <array>
#include <cstddef>
#include <cstdint>

#include "macros.h"
#include "mem.h"
#include "space.h"

WEAK_FOR_LTO_START

namespace panda {

enum class AllocatorType {
    UNDEFINED,
    RUNSLOTS_ALLOCATOR,
    FREELIST_ALLOCATOR,
    HUMONGOUS_ALLOCATOR,
    ARENA_ALLOCATOR,
    TLAB_ALLOCATOR,
    BUMP_ALLOCATOR,
    REGION_ALLOCATOR,
    FRAME_ALLOCATOR,
    BUMP_ALLOCATOR_WITH_TLABS,
};

class AllocatorInfo {
public:
    explicit constexpr AllocatorInfo(AllocatorType type, const void *addr) : type_(type), header_addr_(addr)
    {
        // We can't create AllocatorInfo without correct pointer to the allocator header
        ASSERT(header_addr_ != nullptr);
    }

    AllocatorType GetType() const
    {
        return type_;
    }

    const void *GetAllocatorHeaderAddr() const
    {
        return header_addr_;
    }

    virtual ~AllocatorInfo() = default;

    DEFAULT_COPY_SEMANTIC(AllocatorInfo);
    DEFAULT_MOVE_SEMANTIC(AllocatorInfo);

private:
    AllocatorType type_;
    const void *header_addr_;
};

// PoolMap is used to manage all pools which has been given to the allocators.
// It can be used to find which allocator has been used to allocate an object.
class PoolMap {
public:
    void AddPoolToMap(const void *pool_addr, size_t pool_size, SpaceType space_type, AllocatorType allocator_type,
                      const void *allocator_addr);
    void RemovePoolFromMap(const void *pool_addr, size_t pool_size);
    // Get Allocator info for the object allocated at this address.
    AllocatorInfo GetAllocatorInfo(const void *addr) const;

    void *GetFirstByteOfPoolForAddr(const void *addr) const;

    SpaceType GetSpaceType(const void *addr) const;

    bool IsEmpty() const;

private:
    static constexpr uint64_t POOL_MAP_COVERAGE = PANDA_MAX_HEAP_SIZE;
    static constexpr size_t POOL_MAP_GRANULARITY = PANDA_POOL_ALIGNMENT_IN_BYTES;
    static constexpr size_t POOL_MAP_SIZE = POOL_MAP_COVERAGE / POOL_MAP_GRANULARITY;

    static constexpr bool FIRST_BYTE_IN_SEGMENT_VALUE = true;

    class PoolInfo {
    public:
        void Initialize(bool first_byte_in_segment, SpaceType space_type, AllocatorType allocator_type,
                        const void *allocator_addr)
        {
            ASSERT(first_byte_in_segment_ == FIRST_BYTE_IN_SEGMENT_VALUE);
            ASSERT(allocator_type_ == AllocatorType::UNDEFINED);
            // Added a TSAN ignore here because TSAN thinks
            // that we can have a data race here - concurrent
            // initialization and reading.
            // However, we can't get an access for this fields
            // without initialization in the correct flow.
            TSAN_ANNOTATE_IGNORE_WRITES_BEGIN();
            first_byte_in_segment_ = first_byte_in_segment;
            allocator_addr_ = allocator_addr;
            space_type_ = space_type;
            allocator_type_ = allocator_type;
            TSAN_ANNOTATE_IGNORE_WRITES_END();
        }

        inline bool IsEmpty() const
        {
            return space_type_ == SpaceType::SPACE_TYPE_UNDEFINED;
        }

        void Destroy()
        {
            first_byte_in_segment_ = FIRST_BYTE_IN_SEGMENT_VALUE;
            allocator_addr_ = nullptr;
            allocator_type_ = AllocatorType::UNDEFINED;
            space_type_ = SpaceType::SPACE_TYPE_UNDEFINED;
        }

        bool IsFirstByteInSegment() const
        {
            return first_byte_in_segment_ == FIRST_BYTE_IN_SEGMENT_VALUE;
        }

        AllocatorType GetAllocatorType() const
        {
            return allocator_type_;
        }

        const void *GetAllocatorAddr() const
        {
            return allocator_addr_;
        }

        SpaceType GetSpaceType() const
        {
            return space_type_;
        }

    private:
        bool first_byte_in_segment_ {FIRST_BYTE_IN_SEGMENT_VALUE};
        AllocatorType allocator_type_ {AllocatorType::UNDEFINED};
        SpaceType space_type_ {SpaceType::SPACE_TYPE_UNDEFINED};
        const void *allocator_addr_ = nullptr;
    };

    static size_t AddrToMapNum(const void *addr)
    {
        size_t map_num = ToUintPtr(addr) / POOL_MAP_GRANULARITY;
        ASSERT(map_num < POOL_MAP_SIZE);
        return map_num;
    }

    static void *MapNumToAddr(size_t map_num)
    {
        // Checking overflow
        ASSERT(static_cast<uint64_t>(map_num) * POOL_MAP_GRANULARITY == map_num * POOL_MAP_GRANULARITY);
        return ToVoidPtr(map_num * POOL_MAP_GRANULARITY);
    }

    void *GetFirstByteInSegment(const void *addr) const;

    std::array<PoolInfo, POOL_MAP_SIZE> pool_map_;

    friend class PoolMapTest;
};

}  // namespace panda

WEAK_FOR_LTO_END

#endif  // LIBPANDABASE_MEM_POOL_MAP_H
