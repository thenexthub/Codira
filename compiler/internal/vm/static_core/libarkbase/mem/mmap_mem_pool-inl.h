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

#ifndef LIBPANDABASE_MEM_MMAP_MEM_POOL_INLINE_H
#define LIBPANDABASE_MEM_MMAP_MEM_POOL_INLINE_H

#include <utility>
#ifdef PANDA_QEMU_BUILD
// Unfortunately, madvise on QEMU works differently, and we should zeroed pages by hand.
#include <securec.h>
#endif
#include "mmap_mem_pool.h"
#include "mem.h"
#include "libarkbase/os/mem.h"
#include "libarkbase/utils/logger.h"
#include "libarkbase/mem/arena-inl.h"
#include "libarkbase/mem/mem_config.h"
#include "libarkbase/utils/asan_interface.h"

namespace ark {

// NOLINTNEXTLINE(cppcoreguidelines-macro-usage)
#define LOG_MMAP_MEM_POOL(level) LOG(level, MEMORYPOOL) << "MmapMemPool: "

template <OSPagesAllocPolicy OS_ALLOC_POLICY>
// CC-OFFNXT(G.FUD.06) perf critical, ODR
inline Pool MmapPoolMap::PopFreePool(size_t size)
{
    auto element = freePools_.lower_bound(size);
    if (element == freePools_.end()) {
        return NULLPOOL;
    }
    auto mmapPool = element->second;
    ASSERT(!mmapPool->IsUsed(freePools_.end()));
    auto elementSize = element->first;
    ASSERT(elementSize == mmapPool->GetSize());
    auto elementMem = mmapPool->GetMem();

    if (unreturnedPool_.GetMmapPool() == mmapPool) {
        unreturnedPool_ = UnreturnedToOSPool();
    }

    mmapPool->SetFreePoolsIter(freePools_.end());
    Pool pool(size, elementMem, mmapPool->IsReturnedToOS());
    freePools_.erase(element);
    if (size < elementSize) {
        Pool newPool(elementSize - size, ToVoidPtr(ToUintPtr(elementMem) + size));
        mmapPool->SetSize(size);
        auto newMmapPool = new MmapPool(newPool, freePools_.end());
        poolMap_.insert(std::pair<void *, MmapPool *>(newPool.GetMem(), newMmapPool));
        auto newFreePoolsIter = freePools_.insert(std::pair<size_t, MmapPool *>(newPool.GetSize(), newMmapPool));
        newMmapPool->SetFreePoolsIter(newFreePoolsIter);
        newMmapPool->SetReturnedToOS(mmapPool->IsReturnedToOS());
    }
    if (OS_ALLOC_POLICY == OSPagesAllocPolicy::ZEROED_MEMORY && !mmapPool->IsReturnedToOS()) {
        uintptr_t poolStart = ToUintPtr(pool.GetMem());
        size_t poolSize = pool.GetSize();
        LOG_MMAP_MEM_POOL(DEBUG) << "Return pages to OS from Free Pool to get zeroed memory: start = " << pool.GetMem()
                                 << " with size " << poolSize;
        os::mem::ReleasePages(poolStart, poolStart + poolSize);
        pool.SetZeroFlag();
    }
    return pool;
}

template <OSPagesPolicy OS_PAGES_POLICY>
// CC-OFFNXT(G.FUD.06) perf critical, ODR
inline std::pair<size_t, OSPagesPolicy> MmapPoolMap::PushFreePool(Pool pool)
{
    bool returnedToOs = OS_PAGES_POLICY == OSPagesPolicy::IMMEDIATE_RETURN;
    auto mmapPoolElement = poolMap_.find(pool.GetMem());
    if (UNLIKELY(mmapPoolElement == poolMap_.end())) {
        LOG_MMAP_MEM_POOL(FATAL) << "can't find mmap pool in the pool map when PushFreePool";
    }

    auto mmapPool = mmapPoolElement->second;
    ASSERT(mmapPool->IsUsed(freePools_.end()));

    auto prevPool = (mmapPoolElement != poolMap_.begin()) ? (prev(mmapPoolElement, 1)->second) : nullptr;
    if (prevPool != nullptr && !prevPool->IsUsed(freePools_.end())) {
        unreturnedPool_ = unreturnedPool_.GetMmapPool() == prevPool ? UnreturnedToOSPool() : unreturnedPool_;
        ASSERT(ToUintPtr(prevPool->GetMem()) + prevPool->GetSize() == ToUintPtr(mmapPool->GetMem()));
        returnedToOs = returnedToOs && prevPool->IsReturnedToOS();
        freePools_.erase(prevPool->GetFreePoolsIter());
        prevPool->SetSize(prevPool->GetSize() + mmapPool->GetSize());
        delete mmapPool;
        poolMap_.erase(mmapPoolElement--);
        mmapPool = prevPool;
    }

    auto nextPool = (mmapPoolElement != prev(poolMap_.end(), 1)) ? (next(mmapPoolElement, 1)->second) : nullptr;
    if (nextPool != nullptr && !nextPool->IsUsed(freePools_.end())) {
        unreturnedPool_ = unreturnedPool_.GetMmapPool() == nextPool ? UnreturnedToOSPool() : unreturnedPool_;
        ASSERT(ToUintPtr(mmapPool->GetMem()) + mmapPool->GetSize() == ToUintPtr(nextPool->GetMem()));
        returnedToOs = returnedToOs && nextPool->IsReturnedToOS();
        freePools_.erase(nextPool->GetFreePoolsIter());
        mmapPool->SetSize(nextPool->GetSize() + mmapPool->GetSize());
        delete nextPool;
        poolMap_.erase(++mmapPoolElement);
    } else if (nextPool == nullptr) {
        // It is the last pool. Transform it to free space.
        poolMap_.erase(mmapPoolElement);
        size_t size = mmapPool->GetSize();
        delete mmapPool;
        if (returnedToOs) {
            return {size, OSPagesPolicy::IMMEDIATE_RETURN};
        }
        return {size, OSPagesPolicy::NO_RETURN};
    }

    auto res = freePools_.insert(std::pair<size_t, MmapPool *>(mmapPool->GetSize(), mmapPool));
    mmapPool->SetFreePoolsIter(res);
    mmapPool->SetReturnedToOS(returnedToOs);
    return {0, OS_PAGES_POLICY};
}

inline void MmapPoolMap::IterateOverFreePools(const std::function<void(size_t, MmapPool *)> &visitor)
{
    for (auto &it : freePools_) {
        visitor(it.first, it.second);
    }
}

inline void MmapPoolMap::AddNewPool(Pool pool)
{
    auto newMmapPool = new MmapPool(pool, freePools_.end());
    poolMap_.insert(std::pair<void *, MmapPool *>(pool.GetMem(), newMmapPool));
}

inline size_t MmapPoolMap::GetAllSize() const
{
    size_t bytes = 0;
    for (const auto &pool : freePools_) {
        bytes += pool.first;
    }
    return bytes;
}

// CC-OFFNXT(G.FUD.06) solid logic, ODR
inline bool MmapPoolMap::HaveEnoughFreePools(size_t poolsNum, size_t poolSize) const
{
    ASSERT(poolSize != 0);
    size_t pools = 0;
    for (auto pool = freePools_.rbegin(); pool != freePools_.rend(); pool++) {
        if (pool->first < poolSize) {
            return false;
        }
        if ((pools += pool->first / poolSize) >= poolsNum) {
            return true;
        }
    }
    return false;
}

// CC-OFFNXT(G.FUD.06) solid logic, ODR
inline MmapMemPool::MmapMemPool() : MemPool("MmapMemPool"), nonObjectSpacesCurrentSize_ {0}, nonObjectSpacesMaxSize_ {0}
{
    ASSERT(static_cast<uint64_t>(mem::MemConfig::GetHeapSizeLimit()) <= PANDA_MAX_HEAP_SIZE);
    uint64_t objectSpaceSize = mem::MemConfig::GetHeapSizeLimit();
    if (objectSpaceSize > PANDA_MAX_HEAP_SIZE) {
        LOG_MMAP_MEM_POOL(FATAL) << "The memory limits is too high. We can't allocate so much memory from the system";
    }
    ASSERT(objectSpaceSize <= PANDA_MAX_HEAP_SIZE);
#if defined(PANDA_32_BIT_MANAGED_POINTER) && defined(PANDA_TARGET_64) && !defined(PANDA_TARGET_WINDOWS) && \
    !defined(PANDA_TARGET_MACOS)
    void *mem = ark::os::mem::MapRWAnonymousInFirst4GB(ToVoidPtr(PANDA_32BITS_HEAP_START_ADDRESS), objectSpaceSize,
                                                       // Object space must be aligned to PANDA_POOL_ALIGNMENT_IN_BYTES
                                                       PANDA_POOL_ALIGNMENT_IN_BYTES);
    ASSERT((ToUintPtr(mem) < PANDA_32BITS_HEAP_END_OBJECTS_ADDRESS) || (objectSpaceSize == 0));
    ASSERT(ToUintPtr(mem) + objectSpaceSize <= PANDA_32BITS_HEAP_END_OBJECTS_ADDRESS);
#else
    // We should get aligned to PANDA_POOL_ALIGNMENT_IN_BYTES size
    void *mem = ark::os::mem::MapRWAnonymousWithAlignmentRaw(objectSpaceSize, PANDA_POOL_ALIGNMENT_IN_BYTES);
#endif
    LOG_IF(((mem == nullptr) && (objectSpaceSize != 0)), FATAL, MEMORYPOOL)
        << "MmapMemPool: couldn't mmap " << objectSpaceSize << " bytes of memory for the system";
    ASSERT(AlignUp(ToUintPtr(mem), PANDA_POOL_ALIGNMENT_IN_BYTES) == ToUintPtr(mem));
    minObjectMemoryAddr_ = ToUintPtr(mem);
    mmapedObjectMemorySize_ = objectSpaceSize;
    commonSpace_.Initialize(minObjectMemoryAddr_, objectSpaceSize);
    nonObjectSpacesMaxSize_[SpaceTypeToIndex(SpaceType::SPACE_TYPE_CODE)] = mem::MemConfig::GetCodeCacheSizeLimit();
    nonObjectSpacesMaxSize_[SpaceTypeToIndex(SpaceType::SPACE_TYPE_COMPILER)] =
        mem::MemConfig::GetCompilerMemorySizeLimit();
    nonObjectSpacesMaxSize_[SpaceTypeToIndex(SpaceType::SPACE_TYPE_INTERNAL)] =
        mem::MemConfig::GetInternalMemorySizeLimit();
    // Should be fixed in 9888
    nonObjectSpacesMaxSize_[SpaceTypeToIndex(SpaceType::SPACE_TYPE_FRAMES)] = std::numeric_limits<size_t>::max();
    nonObjectSpacesMaxSize_[SpaceTypeToIndex(SpaceType::SPACE_TYPE_NATIVE_STACKS)] =
        mem::MemConfig::GetNativeStacksMemorySizeLimit();
    LOG_MMAP_MEM_POOL(DEBUG) << "Successfully initialized MMapMemPool. Object memory start from addr "
                             << ToVoidPtr(minObjectMemoryAddr_) << " Preallocated size is equal to " << objectSpaceSize;
}

inline bool MmapPoolMap::FindAndSetUnreturnedFreePool()
{
    ASSERT(unreturnedPool_.IsEmpty());
    for (auto &&[_, pool] : freePools_) {
        if (!pool->IsReturnedToOS()) {
            unreturnedPool_ = UnreturnedToOSPool(pool);
            return true;
        }
    }
    return false;
}

inline void MmapPoolMap::ReleasePagesInUnreturnedPool(size_t poolSize)
{
    ASSERT(poolSize != 0 && !unreturnedPool_.IsEmpty());

    auto pool = unreturnedPool_.GetAndClearUnreturnedPool(poolSize);
    auto unreturnedMem = ToUintPtr(pool.GetMem());
    os::mem::ReleasePages(unreturnedMem, unreturnedMem + pool.GetSize());
    if (unreturnedPool_.GetUnreturnedSize() == 0) {
        unreturnedPool_.SetReturnedToOS();
        unreturnedPool_ = UnreturnedToOSPool();
    }
    LOG_MMAP_MEM_POOL(DEBUG) << "Return pages to OS from Free Pool: start = " << pool.GetMem() << " with size "
                             << pool.GetSize();
}

inline void MmapPoolMap::ReleasePagesInFreePools()
{
    IterateOverFreePools([](size_t poolSize, MmapPool *pool) {
        // Iterate over non returned to OS pools:
        if (!pool->IsReturnedToOS()) {
            pool->SetReturnedToOS(true);
            auto poolStart = ToUintPtr(pool->GetMem());
            LOG_MMAP_MEM_POOL(DEBUG) << "Return pages to OS from Free Pool: start = " << pool->GetMem() << " with size "
                                     << poolSize;
            os::mem::ReleasePages(poolStart, poolStart + poolSize);
        }
    });
}

inline void MmapMemPool::ClearNonObjectMmapedPools()
{
    for (auto i : nonObjectMmapedPools_) {
        Pool pool = std::get<0>(i.second);
        [[maybe_unused]] AllocatorInfo info = std::get<1>(i.second);
        [[maybe_unused]] SpaceType type = std::get<2>(i.second);

        ASSERT(info.GetType() != AllocatorType::UNDEFINED);
        ASSERT(type != SpaceType::SPACE_TYPE_UNDEFINED);
        // does not clears non_object_mmaped_pools_ record because can fail (int munmap(void*, size_t) returned -1)
        FreeRawMemImpl(pool.GetMem(), pool.GetSize());
    }
    nonObjectMmapedPools_.clear();
}

inline MmapMemPool::~MmapMemPool()
{
    ClearNonObjectMmapedPools();
    void *mmapedMemAddr = ToVoidPtr(minObjectMemoryAddr_);
    if (mmapedMemAddr == nullptr) {
        ASSERT(mmapedObjectMemorySize_ == 0);
        return;
    }

    ASSERT(poolMap_.IsEmpty());

    // NOTE(dtrubenkov): consider madvise(mem, total_size_, MADV_DONTNEED); when possible
    if (auto unmapRes = ark::os::mem::UnmapRaw(mmapedMemAddr, mmapedObjectMemorySize_)) {
        LOG_MMAP_MEM_POOL(FATAL) << "Destructor unnmap  error: " << unmapRes->ToString();
    }
}

template <class ArenaT, OSPagesAllocPolicy OS_ALLOC_POLICY>
// CC-OFFNXT(G.FUD.06) perf critical
inline ArenaT *MmapMemPool::AllocArenaImpl(size_t size, SpaceType spaceType, AllocatorType allocatorType,
                                           const void *allocatorAddr)
{
    os::memory::LockHolder lk(lock_);
    LOG_MMAP_MEM_POOL(DEBUG) << "Try to get new arena with size " << std::dec << size << " for "
                             << SpaceTypeToString(spaceType);
    Pool poolForArena = AllocPoolUnsafe<OS_ALLOC_POLICY>(size, spaceType, allocatorType, allocatorAddr);
    void *mem = poolForArena.GetMem();
    if (UNLIKELY(mem == nullptr)) {
        LOG_MMAP_MEM_POOL(ERROR) << "Failed to allocate new arena"
                                 << " for " << SpaceTypeToString(spaceType);
        return nullptr;
    }
    ASSERT(poolForArena.GetSize() == size);
    auto arenaBuffOffs =
        AlignUp(ToUintPtr(mem) + sizeof(ArenaT), GetAlignmentInBytes(ARENA_DEFAULT_ALIGNMENT)) - ToUintPtr(mem);
    mem = new (mem) ArenaT(size - arenaBuffOffs, ToVoidPtr(ToUintPtr(mem) + arenaBuffOffs));
    LOG_MMAP_MEM_POOL(DEBUG) << "Allocated new arena with size " << std::dec << poolForArena.GetSize()
                             << " at addr = " << std::hex << poolForArena.GetMem() << " for "
                             << SpaceTypeToString(spaceType);
    return static_cast<ArenaT *>(mem);
}

template <class ArenaT, OSPagesPolicy OS_PAGES_POLICY>
// CC-OFFNXT(G.FUD.06) perf critical, ODR
inline void MmapMemPool::FreeArenaImpl(ArenaT *arena)
{
    os::memory::LockHolder lk(lock_);
    size_t size = arena->GetSize() + (ToUintPtr(arena->GetMem()) - ToUintPtr(arena));
    ASSERT(size == AlignUp(size, ark::os::mem::GetPageSize()));
    LOG_MMAP_MEM_POOL(DEBUG) << "Try to free arena with size " << std::dec << size << " at addr = " << std::hex
                             << arena;
    FreePoolUnsafe<OS_PAGES_POLICY>(arena, size);
    LOG_MMAP_MEM_POOL(DEBUG) << "Free arena call finished";
}

// CC-OFFNXT(G.FUD.06) solid logic, ODR
inline void *MmapMemPool::AllocRawMemNonObjectImpl(size_t size, SpaceType spaceType)
{
    ASSERT(!IsHeapSpace(spaceType));
    void *mem = nullptr;
    if (LIKELY(nonObjectSpacesMaxSize_[SpaceTypeToIndex(spaceType)] >=
               nonObjectSpacesCurrentSize_[SpaceTypeToIndex(spaceType)] + size)) {
        mem = ark::os::mem::MapRWAnonymousWithAlignmentRaw(size, PANDA_POOL_ALIGNMENT_IN_BYTES);
        if (mem != nullptr) {
            nonObjectSpacesCurrentSize_[SpaceTypeToIndex(spaceType)] += size;
        }
    }
    LOG_MMAP_MEM_POOL(DEBUG) << "Occupied memory for " << SpaceTypeToString(spaceType) << " - " << std::dec
                             << nonObjectSpacesCurrentSize_[SpaceTypeToIndex(spaceType)];
    return mem;
}

template <OSPagesAllocPolicy OS_ALLOC_POLICY>
inline std::pair<void *, bool> MmapMemPool::AllocRawMemObjectImpl(size_t size, SpaceType type)
{
    ASSERT(IsHeapSpace(type));
    auto p = commonSpace_.template AllocRawMem<OS_ALLOC_POLICY>(size, &commonSpacePools_);
    LOG_MMAP_MEM_POOL(DEBUG) << "Occupied memory for " << SpaceTypeToString(type) << " - " << std::dec
                             << commonSpace_.GetOccupiedMemorySize();
    return p;
}

template <OSPagesAllocPolicy OS_ALLOC_POLICY>
// CC-OFFNXT(G.FUD.06) perf critical, solid logic, ODR
inline std::pair<void *, bool> MmapMemPool::AllocRawMemImpl(size_t size, SpaceType type)
{
    os::memory::LockHolder lk(lock_);
    ASSERT(size % ark::os::mem::GetPageSize() == 0);
    // NOTE: We need this check because we use this memory for Pools too
    // which require PANDA_POOL_ALIGNMENT_IN_BYTES alignment
    ASSERT(size == AlignUp(size, PANDA_POOL_ALIGNMENT_IN_BYTES));
    void *mem = nullptr;
    bool isZero = false;
    switch (type) {
        // Internal spaces
        case SpaceType::SPACE_TYPE_COMPILER:
        case SpaceType::SPACE_TYPE_INTERNAL:
        case SpaceType::SPACE_TYPE_CODE:
        case SpaceType::SPACE_TYPE_FRAMES:
        case SpaceType::SPACE_TYPE_NATIVE_STACKS:
            ASSERT(!IsHeapSpace(type));
            mem = AllocRawMemNonObjectImpl(size, type);
            break;
        // Heap spaces:
        case SpaceType::SPACE_TYPE_HUMONGOUS_OBJECT:
        case SpaceType::SPACE_TYPE_NON_MOVABLE_OBJECT:
        case SpaceType::SPACE_TYPE_OBJECT: {
            std::pair<void *, bool> result = AllocRawMemObjectImpl<OS_ALLOC_POLICY>(size, type);
            mem = result.first;
            isZero = result.second;
            break;
        }
        default:
            LOG_MMAP_MEM_POOL(FATAL) << "Try to use incorrect " << SpaceTypeToString(type) << " for AllocRawMem.";
    }
    if (UNLIKELY(mem == nullptr)) {
        LOG_MMAP_MEM_POOL(DEBUG) << "OOM when trying to allocate " << size << " bytes for " << SpaceTypeToString(type);
        // We have OOM and must return nullptr
        mem = nullptr;
    } else {
        LOG_MMAP_MEM_POOL(DEBUG) << "Allocate raw memory with size " << size << " at addr = " << mem << " for "
                                 << SpaceTypeToString(type);
    }
    return {mem, isZero};
}

/* static */
inline void MmapMemPool::FreeRawMemImpl(void *mem, size_t size)
{
    if (auto unmapRes = ark::os::mem::UnmapRaw(mem, size)) {
        LOG_MMAP_MEM_POOL(FATAL) << "Destructor unnmap  error: " << unmapRes->ToString();
    }
    LOG_MMAP_MEM_POOL(DEBUG) << "Deallocated raw memory with size " << size << " at addr = " << mem;
}

template <OSPagesAllocPolicy OS_ALLOC_POLICY>
// CC-OFFNXT(G.FUD.06, G.FUN.01-CPP) perf critical, solid logic
inline Pool MmapMemPool::AllocPoolUnsafe(size_t size, SpaceType spaceType, AllocatorType allocatorType,
                                         const void *allocatorAddr)
{
    ASSERT(size == AlignUp(size, ark::os::mem::GetPageSize()));
    ASSERT(size == AlignUp(size, PANDA_POOL_ALIGNMENT_IN_BYTES));
    Pool pool = NULLPOOL;
    bool addToPoolMap = false;
    // Try to find free pool from the allocated earlier
    switch (spaceType) {
        case SpaceType::SPACE_TYPE_CODE:
        case SpaceType::SPACE_TYPE_COMPILER:
        case SpaceType::SPACE_TYPE_INTERNAL:
        case SpaceType::SPACE_TYPE_FRAMES:
        case SpaceType::SPACE_TYPE_NATIVE_STACKS:
            // We always use mmap for these space types
            break;
        case SpaceType::SPACE_TYPE_HUMONGOUS_OBJECT:
        case SpaceType::SPACE_TYPE_NON_MOVABLE_OBJECT:
        case SpaceType::SPACE_TYPE_OBJECT:
            addToPoolMap = true;
            pool = commonSpacePools_.template PopFreePool<OS_ALLOC_POLICY>(size);
            break;
        default:
            LOG_MMAP_MEM_POOL(FATAL) << "Try to use incorrect " << SpaceTypeToString(spaceType)
                                     << " for AllocPoolUnsafe.";
    }
    if (pool.GetMem() != nullptr) {
        LOG_MMAP_MEM_POOL(DEBUG) << "Reuse pool with size " << pool.GetSize() << " at addr = " << pool.GetMem()
                                 << " for " << SpaceTypeToString(spaceType);
    }
    if (pool.GetMem() == nullptr) {
        std::pair<void *, bool> result = AllocRawMemImpl<OS_ALLOC_POLICY>(size, spaceType);
        pool = Pool(size, result.first, result.second);
    }
    if (pool.GetMem() == nullptr) {
        return pool;
    }
    ASAN_UNPOISON_MEMORY_REGION(pool.GetMem(), pool.GetSize());
    if (UNLIKELY(allocatorAddr == nullptr)) {
        // Save a pointer to the first byte of a Pool
        allocatorAddr = pool.GetMem();
    }
    if (addToPoolMap) {
        poolMap_.AddPoolToMap(ToVoidPtr(ToUintPtr(pool.GetMem()) - GetMinObjectAddress()), pool.GetSize(), spaceType,
                              allocatorType, allocatorAddr);
#ifdef PANDA_QEMU_BUILD
        // Unfortunately, madvise on QEMU works differently, and we should zeroed pages by hand.
        if (OS_ALLOC_POLICY == OSPagesAllocPolicy::ZEROED_MEMORY || pool.IsZeroed()) {
            memset_s(pool.GetMem(), pool.GetSize(), 0, pool.GetSize());
        }
#endif
    } else {
        AddToNonObjectPoolsMap(std::make_tuple(pool, AllocatorInfo(allocatorType, allocatorAddr), spaceType));
    }
    os::mem::TagAnonymousMemory(pool.GetMem(), pool.GetSize(), SpaceTypeToString(spaceType));
    ASSERT(AlignUp(ToUintPtr(pool.GetMem()), PANDA_POOL_ALIGNMENT_IN_BYTES) == ToUintPtr(pool.GetMem()));
    return pool;
}

template <OSPagesPolicy OS_PAGES_POLICY>
// CC-OFFNXT(G.FUD.06) perf critical, solid logic, ODR
inline void MmapMemPool::FreePoolUnsafe(void *mem, size_t size)
{
    ASSERT(size == AlignUp(size, ark::os::mem::GetPageSize()));
    ASAN_POISON_MEMORY_REGION(mem, size);
    SpaceType poolSpaceType = GetSpaceTypeForAddrImpl(mem);
    switch (poolSpaceType) {
        case SpaceType::SPACE_TYPE_HUMONGOUS_OBJECT:
        case SpaceType::SPACE_TYPE_NON_MOVABLE_OBJECT:
        case SpaceType::SPACE_TYPE_OBJECT: {
            auto freeSize = commonSpacePools_.PushFreePool<OS_PAGES_POLICY>(Pool(size, mem));
            commonSpace_.FreeMem(freeSize.first, freeSize.second);
            break;
        }
        case SpaceType::SPACE_TYPE_COMPILER:
        case SpaceType::SPACE_TYPE_INTERNAL:
        case SpaceType::SPACE_TYPE_CODE:
        case SpaceType::SPACE_TYPE_FRAMES:
        case SpaceType::SPACE_TYPE_NATIVE_STACKS:
            ASSERT(!IsHeapSpace(poolSpaceType));
            nonObjectSpacesCurrentSize_[SpaceTypeToIndex(poolSpaceType)] -= size;
            FreeRawMemImpl(mem, size);
            break;
        default:
            LOG_MMAP_MEM_POOL(FATAL) << "Try to use incorrect " << SpaceTypeToString(poolSpaceType)
                                     << " for FreePoolUnsafe.";
    }
    os::mem::TagAnonymousMemory(mem, size, nullptr);
    if (IsHeapSpace(poolSpaceType)) {
        poolMap_.RemovePoolFromMap(ToVoidPtr(ToUintPtr(mem) - GetMinObjectAddress()), size);
        if constexpr (OS_PAGES_POLICY == OSPagesPolicy::IMMEDIATE_RETURN) {
            LOG_MMAP_MEM_POOL(DEBUG) << "IMMEDIATE_RETURN and release pages for this pool";
            os::mem::ReleasePages(ToUintPtr(mem), ToUintPtr(mem) + size);
        }
    } else {
        RemoveFromNonObjectPoolsMap(mem);
    }
    LOG_MMAP_MEM_POOL(DEBUG) << "Freed " << std::dec << size << " memory for " << SpaceTypeToString(poolSpaceType);
}

template <OSPagesAllocPolicy OS_ALLOC_POLICY>
inline Pool MmapMemPool::AllocPoolImpl(size_t size, SpaceType spaceType, AllocatorType allocatorType,
                                       const void *allocatorAddr)
{
    os::memory::LockHolder lk(lock_);
    LOG_MMAP_MEM_POOL(DEBUG) << "Try to get new pool with size " << std::dec << size << " for "
                             << SpaceTypeToString(spaceType);
    Pool pool = AllocPoolUnsafe<OS_ALLOC_POLICY>(size, spaceType, allocatorType, allocatorAddr);
    LOG_MMAP_MEM_POOL(DEBUG) << "Allocated new pool with size " << std::dec << pool.GetSize()
                             << " at addr = " << std::hex << pool.GetMem() << " for " << SpaceTypeToString(spaceType);
    return pool;
}

template <OSPagesPolicy OS_PAGES_POLICY>
inline void MmapMemPool::FreePoolImpl(void *mem, size_t size)
{
    os::memory::LockHolder lk(lock_);
    LOG_MMAP_MEM_POOL(DEBUG) << "Try to free pool with size " << std::dec << size << " at addr = " << std::hex << mem;
    FreePoolUnsafe<OS_PAGES_POLICY>(mem, size);
    LOG_MMAP_MEM_POOL(DEBUG) << "Free pool call finished";
}

inline void MmapMemPool::AddToNonObjectPoolsMap(std::tuple<Pool, AllocatorInfo, SpaceType> poolInfo)
{
    void *poolAddr = std::get<0>(poolInfo).GetMem();
    ASSERT(nonObjectMmapedPools_.find(poolAddr) == nonObjectMmapedPools_.end());
    nonObjectMmapedPools_.insert({poolAddr, poolInfo});
}

inline void MmapMemPool::RemoveFromNonObjectPoolsMap(void *poolAddr)
{
    auto element = nonObjectMmapedPools_.find(poolAddr);
    ASSERT(element != nonObjectMmapedPools_.end());
    nonObjectMmapedPools_.erase(element);
}

// CC-OFFNXT(G.FUD.06) Splitting this function will degrade readability, solid logic, ODR
inline std::tuple<Pool, AllocatorInfo, SpaceType> MmapMemPool::FindAddrInNonObjectPoolsMap(const void *addr) const
{
    auto element = nonObjectMmapedPools_.lower_bound(addr);
    uintptr_t poolStart =
        (element != nonObjectMmapedPools_.end()) ? ToUintPtr(element->first) : (std::numeric_limits<uintptr_t>::max());
    if (ToUintPtr(addr) < poolStart) {
        ASSERT(element != nonObjectMmapedPools_.begin());
        element = std::prev(element);
        poolStart = ToUintPtr(element->first);
    }
    ASSERT(element != nonObjectMmapedPools_.end());
    [[maybe_unused]] uintptr_t poolEnd = poolStart + std::get<0>(element->second).GetSize();
    ASSERT((poolStart <= ToUintPtr(addr)) && (ToUintPtr(addr) < poolEnd));
    return element->second;
}

inline AllocatorInfo MmapMemPool::GetAllocatorInfoForAddrImpl(const void *addr) const
{
    if ((ToUintPtr(addr) < GetMinObjectAddress()) || (ToUintPtr(addr) >= GetMaxObjectAddress())) {
        os::memory::LockHolder lk(lock_);
        return std::get<1>(FindAddrInNonObjectPoolsMap(addr));
    }
    AllocatorInfo info = poolMap_.GetAllocatorInfo(ToVoidPtr(ToUintPtr(addr) - GetMinObjectAddress()));
    ASSERT(info.GetType() != AllocatorType::UNDEFINED);
    ASSERT(info.GetAllocatorHeaderAddr() != nullptr);
    return info;
}

inline SpaceType MmapMemPool::GetSpaceTypeForAddrImpl(const void *addr) const
{
    if ((ToUintPtr(addr) < GetMinObjectAddress()) || (ToUintPtr(addr) >= GetMaxObjectAddress())) {
        os::memory::LockHolder lk(lock_);
        // <2> is a pointer to SpaceType
        return std::get<2>(FindAddrInNonObjectPoolsMap(addr));
    }
    // Since this method is designed to work without locks for fast space reading, we add an
    // annotation here to prevent a false alarm with PoolMap::PoolInfo::Destroy.
    // This data race is not real because this method can be called only for a valid memory
    // i.e., memory that was initialized and not freed after that.
    TSAN_ANNOTATE_IGNORE_WRITES_BEGIN();
    SpaceType spaceType = poolMap_.GetSpaceType(ToVoidPtr(ToUintPtr(addr) - GetMinObjectAddress()));
    TSAN_ANNOTATE_IGNORE_WRITES_END();
    ASSERT(spaceType != SpaceType::SPACE_TYPE_UNDEFINED);
    return spaceType;
}

inline void *MmapMemPool::GetStartAddrPoolForAddrImpl(const void *addr) const
{
    // We optimized this call and expect that it will be used only for object space
    ASSERT(!(ToUintPtr(addr) < GetMinObjectAddress()) || (ToUintPtr(addr) >= GetMaxObjectAddress()));
    void *poolStartAddr = poolMap_.GetFirstByteOfPoolForAddr(ToVoidPtr(ToUintPtr(addr) - GetMinObjectAddress()));
    return ToVoidPtr(ToUintPtr(poolStartAddr) + GetMinObjectAddress());
}

inline size_t MmapMemPool::GetObjectSpaceFreeBytes() const
{
    os::memory::LockHolder lk(lock_);

    size_t unusedBytes = commonSpace_.GetFreeSpace();
    size_t freedBytes = commonSpacePools_.GetAllSize();
    ASSERT(unusedBytes + freedBytes <= commonSpace_.GetMaxSize());
    return unusedBytes + freedBytes;
}

inline bool MmapMemPool::HaveEnoughPoolsInObjectSpace(size_t poolsNum, size_t poolSize) const
{
    os::memory::LockHolder lk(lock_);

    size_t unusedBytes = commonSpace_.GetFreeSpace();
    ASSERT(poolSize != 0);
    size_t pools = unusedBytes / poolSize;
    if (pools >= poolsNum) {
        return true;
    }
    return commonSpacePools_.HaveEnoughFreePools(poolsNum - pools, poolSize);
}

inline size_t MmapMemPool::GetObjectUsedBytes() const
{
    os::memory::LockHolder lk(lock_);
    ASSERT(commonSpace_.GetOccupiedMemorySize() >= commonSpacePools_.GetAllSize());
    return commonSpace_.GetOccupiedMemorySize() - commonSpacePools_.GetAllSize();
}

inline void MmapMemPool::ReleaseFreePagesToOS()
{
    os::memory::LockHolder lk(lock_);
    auto unreturnedSize = commonSpacePools_.GetUnreturnedToOsSize();
    if (unreturnedSize != 0) {
        commonSpacePools_.ReleasePagesInUnreturnedPool(unreturnedSize);
    }
    commonSpacePools_.ReleasePagesInFreePools();
    unreturnedSize = commonSpace_.GetUnreturnedToOsSize();
    if (unreturnedSize != 0) {
        commonSpace_.ReleasePagesInMainPool(unreturnedSize);
    }
}

inline bool MmapMemPool::ReleaseFreePagesToOSWithInterruption(const InterruptFlag &interruptFlag)
{
    bool wasInterrupted = ReleasePagesInUnreturnedPoolWithInterruption(interruptFlag);
    if (wasInterrupted) {
        return true;
    }
    wasInterrupted = ReleasePagesInFreePoolsWithInterruption(interruptFlag);
    if (wasInterrupted) {
        return true;
    }
    wasInterrupted = ReleasePagesInMainPoolWithInterruption(interruptFlag);
    return wasInterrupted;
}

// CC-OFFNXT(G.FUD.06) perf critical, ODR
inline bool MmapMemPool::ReleasePagesInUnreturnedPoolWithInterruption(const InterruptFlag &interruptFlag)
{
    while (true) {
        {
            os::memory::LockHolder lk(lock_);
            auto unreturnedSize = commonSpacePools_.GetUnreturnedToOsSize();
            if (unreturnedSize == 0) {
                return false;
            }
            if (interruptFlag == ReleasePagesStatus::NEED_INTERRUPT) {
                return true;
            }
            auto poolSize = std::min(RELEASE_MEM_SIZE, unreturnedSize);
            commonSpacePools_.ReleasePagesInUnreturnedPool(poolSize);
        }
        /* @sync 1
         * @description Wait for interruption from G1GC Mixed collection
         */
    }
}

// CC-OFFNXT(G.FUD.06) perf critical, ODR
inline bool MmapMemPool::ReleasePagesInFreePoolsWithInterruption(const InterruptFlag &interruptFlag)
{
    while (true) {
        {
            os::memory::LockHolder lk(lock_);
            auto poolFound = commonSpacePools_.FindAndSetUnreturnedFreePool();
            if (!poolFound) {
                return false;
            }
        }
        auto wasInterrupted = ReleasePagesInUnreturnedPoolWithInterruption(interruptFlag);
        if (wasInterrupted) {
            return true;
        }
    }
}

// CC-OFFNXT(G.FUD.06) solid logic, ODR
inline bool MmapMemPool::ReleasePagesInMainPoolWithInterruption(const InterruptFlag &interruptFlag)
{
    while (true) {
        os::memory::LockHolder lk(lock_);
        auto unreturnedSize = commonSpace_.GetUnreturnedToOsSize();
        if (unreturnedSize == 0) {
            return false;
        }
        if (interruptFlag == ReleasePagesStatus::NEED_INTERRUPT) {
            return true;
        }
        auto poolSize = std::min(RELEASE_MEM_SIZE, unreturnedSize);
        commonSpace_.ReleasePagesInMainPool(poolSize);
    }
}

inline void MmapMemPool::SpaceMemory::ReleasePagesInMainPool(size_t poolSize)
{
    ASSERT(poolSize != 0);
    Pool mainPool = GetAndClearUnreturnedToOSMemory(poolSize);
    auto poolStart = ToUintPtr(mainPool.GetMem());
    os::mem::ReleasePages(poolStart, poolStart + mainPool.GetSize());
    LOG_MMAP_MEM_POOL(DEBUG) << "Return pages to OS from common_space: start = " << mainPool.GetMem() << " with size "
                             << mainPool.GetSize();
}

#undef LOG_MMAP_MEM_POOL

}  // namespace ark

#endif  // LIBPANDABASE_MEM_MMAP_MEM_POOL_INLINE_H
