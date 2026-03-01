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

#ifndef LIBPANDABASE_MEM_MMAP_MEM_POOL_H
#define LIBPANDABASE_MEM_MMAP_MEM_POOL_H

#include "mem/mem_pool.h"
#include "mem/mem.h"
#include "os/mem.h"
#include "os/mutex.h"
#include "mem/space.h"

#include <map>
#include <tuple>

namespace panda {

class MMapMemPoolTest;
namespace mem::test {
class InternalAllocatorTest;
}  // namespace mem::test

class MmapPool {
public:
    using FreePoolsIter = std::multimap<size_t, MmapPool *>::iterator;
    explicit MmapPool(Pool pool, FreePoolsIter free_pools_iter) : pool_(pool), free_pools_iter_(free_pools_iter) {}

    ~MmapPool() = default;

    DEFAULT_COPY_SEMANTIC(MmapPool);
    DEFAULT_MOVE_SEMANTIC(MmapPool);

    size_t GetSize()
    {
        return pool_.GetSize();
    }

    void SetSize(size_t size)
    {
        pool_ = Pool(size, GetMem());
    }

    void *GetMem()
    {
        return pool_.GetMem();
    }

    // A free pool will be store in the free_pools_, and it's iterator will be recorded in the free_pools_iter_.
    // If the free_pools_iter_ is equal to the end of free_pools_, the pool is used.
    bool IsUsed(FreePoolsIter end_iter)
    {
        return free_pools_iter_ == end_iter;
    }

    FreePoolsIter GetFreePoolsIter()
    {
        return free_pools_iter_;
    }

    void SetFreePoolsIter(FreePoolsIter free_pools_iter)
    {
        free_pools_iter_ = free_pools_iter;
    }

private:
    Pool pool_;
    // record the iterator of the pool in the multimap
    FreePoolsIter free_pools_iter_;
};

class MmapPoolMap {
public:
    MmapPoolMap() = default;

    ~MmapPoolMap()
    {
        for (auto &pool : pool_map_) {
            delete pool.second;
        }
    }

    DEFAULT_COPY_SEMANTIC(MmapPoolMap);
    DEFAULT_MOVE_SEMANTIC(MmapPoolMap);

    // Find a free pool with enough size in the map. Split the pool, if the pool size is larger than required size.
    Pool PopFreePool(size_t size);

    // push the unused pool to the map.
    void PushFreePool(Pool pool);

    // Add a new pool to the map. This pool will be marked as used.
    void AddNewPool(Pool pool);

    // Get the sum of all free pools size.
    size_t GetAllSize() const;

    /**
     * To check if we can alloc enough pools from free pools
     * @param pools_num the number of pools we need
     * @param pool_size the size of the pool we need
     * @return true if we can make sure that we have enough space in free pools to alloc pools we need
     */
    bool HaveEnoughFreePools(size_t pools_num, size_t pool_size) const;

private:
    std::map<void *, MmapPool *> pool_map_;
    std::multimap<size_t, MmapPool *> free_pools_;
};

class MmapMemPool : public MemPool<MmapMemPool> {
public:
    NO_COPY_SEMANTIC(MmapMemPool);
    NO_MOVE_SEMANTIC(MmapMemPool);
    ~MmapMemPool() override;

    /**
     * Get min address in pool
     * @return min address in pool
     */
    uintptr_t GetMinObjectAddress() const
    {
        return min_object_memory_addr_;
    }

    void *GetAddressOfMinObjectAddress()
    {
        return static_cast<void *>(&min_object_memory_addr_);
    }

    /**
     * Get max address in pool
     * @return max address in pool
     */
    uintptr_t GetMaxObjectAddress() const
    {
        return min_object_memory_addr_ + mmaped_object_memory_size_;
    }

    size_t GetTotalObjectSize() const
    {
        return mmaped_object_memory_size_;
    }

    /**
     * Get start address of pool for input address in this pool
     * @param addr address in pool
     * @return start address of pool
     */
    void *GetStartAddrPoolForAddr(const void *addr) const
    {
        return GetStartAddrPoolForAddrImpl(addr);
    }

    size_t GetObjectSpaceFreeBytes() const;

    // To check if we can alloc enough pools in object space
    bool HaveEnoughPoolsInObjectSpace(size_t pools_num, size_t pool_size) const;

    /**
     * @return used bytes count in object space (so exclude bytes in free pools)
     */
    size_t GetObjectUsedBytes() const;

    void SetUseCompilerSpaceSizeLimit(bool use_limit)
    {
        use_compiler_space_size_limit_ = use_limit;
    }

private:
    template <class ArenaT = Arena>
    ArenaT *AllocArenaImpl(size_t size, SpaceType space_type, AllocatorType allocator_type, const void *allocator_addr);
    template <class ArenaT = Arena>
    void FreeArenaImpl(ArenaT *arena);

    void *AllocRawMemImpl(size_t size, SpaceType type);
    void *AllocRawMemCompilerImpl(size_t size);
    void *AllocRawMemInternalImpl(size_t size);
    void *AllocRawMemCodeImpl(size_t size);
    void *AllocRawMemObjectImpl(size_t size, SpaceType type);
    void FreeRawMemImpl(void *mem, size_t size);

    Pool AllocPoolImpl(size_t size, SpaceType space_type, AllocatorType allocator_type, const void *allocator_addr);
    void FreePoolImpl(void *mem, size_t size);

    AllocatorInfo GetAllocatorInfoForAddrImpl(const void *addr) const;
    SpaceType GetSpaceTypeForAddrImpl(const void *addr) const;
    void *GetStartAddrPoolForAddrImpl(const void *addr) const;

    Pool AllocPoolUnsafe(size_t size, SpaceType space_type, AllocatorType allocator_type, const void *allocator_addr);
    void FreePoolUnsafe(void *mem, size_t size);

    void AddToNonObjectPoolsMap(std::tuple<Pool, AllocatorInfo, SpaceType> pool_info);
    void RemoveFromNonObjectPoolsMap(void *pool_addr);
    std::tuple<Pool, AllocatorInfo, SpaceType> FindAddrInNonObjectPoolsMap(const void *addr) const;

    MmapMemPool();

    // A super class for raw memory allocation for spaces.
    class SpaceMemory {
    public:
        void Initialize(uintptr_t min_addr, size_t max_size)
        {
            min_address_ = min_addr;
            max_size_ = max_size;
            cur_alloc_offset_ = 0U;
        }

        uintptr_t GetMinAddress() const
        {
            return min_address_;
        }

        size_t GetMaxSize() const
        {
            return max_size_;
        }

        size_t GetOccupiedMemorySize() const
        {
            return cur_alloc_offset_;
        }

        inline size_t GetFreeSpace() const
        {
            ASSERT(max_size_ >= cur_alloc_offset_);
            return max_size_ - cur_alloc_offset_;
        }

        void *AllocRawMem(size_t size, MmapPoolMap *pool_map)
        {
            if (UNLIKELY(GetFreeSpace() < size)) {
                return nullptr;
            }
            void *mem = ToVoidPtr(min_address_ + cur_alloc_offset_);
            cur_alloc_offset_ += size;
            pool_map->AddNewPool(Pool(size, mem));
            return mem;
        }

    private:
        uintptr_t min_address_ {0U};    /// < Min address for the space
        size_t max_size_ {0U};          /// < Max size in bytes for the space
        size_t cur_alloc_offset_ {0U};  /// < A value of occupied memory from the min_address_
    };

    uintptr_t min_object_memory_addr_ {0U};  // < Minimal address of the mmaped object memory
    size_t mmaped_object_memory_size_ {0U};  // < Size of whole the mmaped object memory

    SpaceMemory common_space_;

    PoolMap pool_map_;  /// < Pool map for object pools with all required information for quick search

    MmapPoolMap common_space_pools_;

    size_t code_space_current_size_ {0};
    size_t compiler_space_current_size_ {0};
    size_t internal_space_current_size_ {0};

    size_t code_space_max_size_ {0};
    size_t compiler_space_max_size_ {0};
    size_t internal_space_max_size_ {0};

    bool use_compiler_space_size_limit_ {false};

    // Map for non object pools allocated via mmap
    std::map<const void *, std::tuple<Pool, AllocatorInfo, SpaceType>> non_object_mmaped_pools_;
    // AllocRawMem is called both from alloc and externally
    mutable os::memory::RecursiveMutex lock_;

    friend class PoolManager;
    friend class MemPool<MmapMemPool>;
    friend class MMapMemPoolTest;
    friend class mem::test::InternalAllocatorTest;
};

}  // namespace panda

#endif  // LIBPANDABASE_MEM_MMAP_MEM_POOL_H
