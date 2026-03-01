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

#ifndef LIBPANDABASE_MEM_MEM_POOL_H
#define LIBPANDABASE_MEM_MEM_POOL_H

#include <cstddef>
#include "macros.h"
#include "mem.h"
#include "pool_map.h"

namespace panda {
class Arena;

class Pool {
public:
    explicit constexpr Pool(size_t size, void *mem) : size_(size), mem_(mem) {}
    explicit Pool(std::pair<size_t, void *> pool) : size_(pool.first), mem_(pool.second) {}

    size_t GetSize() const
    {
        return size_;
    }

    void *GetMem()
    {
        return mem_;
    }

    bool operator==(const Pool &other) const
    {
        return (this->size_ == other.size_) && (this->mem_ == other.mem_);
    }

    bool operator!=(const Pool &other) const
    {
        return !(*this == other);
    }

    ~Pool() = default;

    DEFAULT_COPY_SEMANTIC(Pool);
    DEFAULT_MOVE_SEMANTIC(Pool);

private:
    size_t size_;
    void *mem_;
};

constexpr Pool NULLPOOL {0, nullptr};

template <class MemPoolImplT>
class MemPool {
public:
    virtual ~MemPool() = default;
    explicit MemPool(std::string pool_name) : name_(std::move(pool_name)) {}
    DEFAULT_NOEXCEPT_MOVE_SEMANTIC(MemPool);
    DEFAULT_COPY_SEMANTIC(MemPool);

    /**
     * Allocates arena with size bytes
     * @tparam ArenaT - type of Arena
     * @param size - size of buffer in arena in bytes
     * @param space_type - type of the space which arena allocated for
     * @param allocator_type - type of the allocator which arena allocated for
     * @param allocator_addr - address of the allocator header.
     * @return pointer to allocated arena
     */
    // TODO(aemelenko): We must always define allocator_addr for AllocArena
    // because we set up arena at the first bytes of the pool
    template <class ArenaT = Arena>
    inline ArenaT *AllocArena(size_t size, SpaceType space_type, AllocatorType allocator_type,
                              const void *allocator_addr = nullptr)
    {
        return static_cast<MemPoolImplT *>(this)->template AllocArenaImpl<ArenaT>(size, space_type, allocator_type,
                                                                                  allocator_addr);
    }

    /**
     * Frees allocated arena
     * @tparam ArenaT - arena type
     * @param arena - pointer to the arena
     */
    template <class ArenaT = Arena>
    inline void FreeArena(ArenaT *arena)
    {
        static_cast<MemPoolImplT *>(this)->template FreeArenaImpl<ArenaT>(arena);
    }

    /**
     * Allocates pool with at least size bytes
     * @param size - minimal size of a pool in bytes
     * @param space_type - type of the space which pool allocated for
     * @param allocator_type - type of the allocator which arena allocated for
     * @param allocator_addr - address of the allocator header.
     *  If it is not defined, it means that allocator header will be located at the first bytes of the returned pool.
     * @return pool info with the size and a pointer
     */
    Pool AllocPool(size_t size, SpaceType space_type, AllocatorType allocator_type,
                   const void *allocator_addr = nullptr)
    {
        return static_cast<MemPoolImplT *>(this)->AllocPoolImpl(size, space_type, allocator_type, allocator_addr);
    }

    /**
     * Frees allocated pool
     * @param mem - pointer to an allocated pool
     * @param size - size of the allocated pool in bytes
     */
    void FreePool(void *mem, size_t size)
    {
        static_cast<MemPoolImplT *>(this)->FreePoolImpl(mem, size);
    }

    /**
     * Get info about the allocator in which this address is used
     * @param addr
     * @return Allocator info with a type and pointer to the allocator header
     */
    AllocatorInfo GetAllocatorInfoForAddr(const void *addr) const
    {
        return static_cast<const MemPoolImplT *>(this)->GetAllocatorInfoForAddrImpl(addr);
    }

    /**
     * Get space type which this address used for
     * @param addr
     * @return space type
     */
    SpaceType GetSpaceTypeForAddr(const void *addr) const
    {
        return static_cast<const MemPoolImplT *>(this)->GetSpaceTypeForAddrImpl(addr);
    }

    /**
     * Get address of pool start for input address
     * @param addr address in pool
     * @return address of pool start
     */
    void *GetStartAddrPoolForAddr(const void *addr) const
    {
        return static_cast<const MemPoolImplT *>(this)->GetStartAddrPoolForAddrImpl(addr);
    }

private:
    std::string name_;
};

}  // namespace panda

#endif  // LIBPANDABASE_MEM_MEM_POOL_H
