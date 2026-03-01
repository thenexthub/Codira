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

#ifndef LIBPANDABASE_MEM_MALLOC_MEM_POOL_INLINE_H
#define LIBPANDABASE_MEM_MALLOC_MEM_POOL_INLINE_H

#include "malloc_mem_pool.h"
#include "mem.h"
#include "libarkbase/os/mem.h"
#include "libarkbase/utils/logger.h"
#include <cstdlib>
#include <memory>
#include <securec.h>

namespace ark {

// NOLINTNEXTLINE(cppcoreguidelines-macro-usage)
#define LOG_MALLOC_MEM_POOL(level) LOG(level, MEMORYPOOL) << "MallocMemPool: "

inline MallocMemPool::MallocMemPool() : MemPool("MallocMemPool")
{
    LOG_MALLOC_MEM_POOL(DEBUG) << "Successfully initialized MallocMemPool";
}

template <class ArenaT, OSPagesAllocPolicy OS_ALLOC_POLICY>
// CC-OFFNXT(G.FUD.06) perf critical, ODR
inline ArenaT *MallocMemPool::AllocArenaImpl(size_t size, [[maybe_unused]] SpaceType spaceType,
                                             [[maybe_unused]] AllocatorType allocatorType,
                                             [[maybe_unused]] const void *allocatorAddr)
{
    LOG_MALLOC_MEM_POOL(DEBUG) << "Try to get new arena with size " << std::dec << size << " for "
                               << SpaceTypeToString(spaceType);
    size_t maxAlignmentDrift = 0;
    if (DEFAULT_ALIGNMENT_IN_BYTES > alignof(ArenaT)) {
        maxAlignmentDrift = DEFAULT_ALIGNMENT_IN_BYTES - alignof(ArenaT);
    }
    size_t maxSize = size + sizeof(ArenaT) + maxAlignmentDrift;
    auto ret = ark::os::mem::AlignedAlloc(std::max(DEFAULT_ALIGNMENT_IN_BYTES, alignof(ArenaT)), maxSize);
    void *buff = reinterpret_cast<char *>(reinterpret_cast<std::uintptr_t>(ret) + sizeof(ArenaT));
    size_t sizeForBuff = maxSize - sizeof(ArenaT);
    buff = std::align(DEFAULT_ALIGNMENT_IN_BYTES, size, buff, sizeForBuff);
    ASSERT(buff != nullptr);
    ASSERT(reinterpret_cast<std::uintptr_t>(buff) - reinterpret_cast<std::uintptr_t>(ret) >= sizeof(ArenaT));
    ASSERT(sizeForBuff >= size);
    ret = new (ret) ArenaT(sizeForBuff, buff);
    ASSERT(reinterpret_cast<std::uintptr_t>(ret) + maxSize >= reinterpret_cast<std::uintptr_t>(buff) + size);
    LOG_MALLOC_MEM_POOL(DEBUG) << "Allocated new arena with size " << std::dec << sizeForBuff
                               << " at addr = " << std::hex << buff << " for " << SpaceTypeToString(spaceType);
    if (OS_ALLOC_POLICY == OSPagesAllocPolicy::ZEROED_MEMORY && ret != nullptr) {
        memset_s(ret, sizeForBuff, 0, sizeForBuff);
    }
    return static_cast<ArenaT *>(ret);
}

template <class ArenaT, OSPagesPolicy OS_PAGES_POLICY>
inline void MallocMemPool::FreeArenaImpl(ArenaT *arena)
{
    LOG_MALLOC_MEM_POOL(DEBUG) << "Try to free arena with size " << std::dec << arena->GetSize()
                               << " at addr = " << std::hex << arena;
    arena->~Arena();
    os::mem::AlignedFree(arena);
    LOG_MALLOC_MEM_POOL(DEBUG) << "Free arena call finished";
}

/* static */
template <OSPagesAllocPolicy OS_ALLOC_POLICY>
inline Pool MallocMemPool::AllocPoolImpl(size_t size, [[maybe_unused]] SpaceType spaceType,
                                         [[maybe_unused]] AllocatorType allocatorType,
                                         [[maybe_unused]] const void *allocatorAddr)
{
    LOG_MALLOC_MEM_POOL(DEBUG) << "Try to get new pool with size " << std::dec << size << " for "
                               << SpaceTypeToString(spaceType);
    void *mem = std::malloc(size);  // NOLINT(cppcoreguidelines-no-malloc)
    LOG_MALLOC_MEM_POOL(DEBUG) << "Allocated new pool with size " << std::dec << size << " at addr = " << std::hex
                               << mem << " for " << SpaceTypeToString(spaceType);
    if (OS_ALLOC_POLICY == OSPagesAllocPolicy::ZEROED_MEMORY && mem != nullptr) {
        memset_s(mem, size, 0, size);
    }
    return Pool(size, mem);
}

/* static */
template <OSPagesPolicy OS_PAGES_POLICY>
inline void MallocMemPool::FreePoolImpl(void *mem, [[maybe_unused]] size_t size)
{
    LOG_MALLOC_MEM_POOL(DEBUG) << "Try to free pool with size " << std::dec << size << " at addr = " << std::hex << mem;
    std::free(mem);  // NOLINT(cppcoreguidelines-no-malloc)
    LOG_MALLOC_MEM_POOL(DEBUG) << "Free pool call finished";
}

/* static */
inline AllocatorInfo MallocMemPool::GetAllocatorInfoForAddrImpl([[maybe_unused]] const void *addr)
{
    LOG(FATAL, ALLOC) << "Not implemented method";
    return AllocatorInfo(AllocatorType::UNDEFINED, nullptr);
}

/* static */
inline SpaceType MallocMemPool::GetSpaceTypeForAddrImpl([[maybe_unused]] const void *addr)
{
    LOG(FATAL, ALLOC) << "Not implemented method";
    return SpaceType::SPACE_TYPE_UNDEFINED;
}

/* static */
inline void *MallocMemPool::GetStartAddrPoolForAddrImpl([[maybe_unused]] const void *addr)
{
    LOG(FATAL, ALLOC) << "Not implemented method";
    return nullptr;
}

}  // namespace ark

#endif  // LIBPANDABASE_MEM_MALLOC_MEM_POOL_INLINE_H
