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

#include "runtime/mem/heap_space.h"
#include "libarkbase/mem/mem.h"
#include "libarkbase/mem/pool_manager.h"
#include "libarkbase/mem/mmap_mem_pool-inl.h"
#include "libarkbase/mem/mem_pool.h"

namespace ark::mem {

void HeapSpace::Initialize(size_t initialSize, size_t maxSize, uint32_t minFreePercentage, uint32_t maxFreePercentage)
{
    ASSERT(!isInitialized_);
    memSpace_.Initialize(initialSize, maxSize);
    InitializePercentages(minFreePercentage, maxFreePercentage);
    isInitialized_ = true;
}

void HeapSpace::InitializePercentages(uint32_t minFreePercentage, uint32_t maxFreePercentage)
{
    minFreePercentage_ = static_cast<double>(std::min(minFreePercentage, MAX_FREE_PERCENTAGE)) / PERCENT_100_U32;
    maxFreePercentage_ = static_cast<double>(std::min(maxFreePercentage, MAX_FREE_PERCENTAGE)) / PERCENT_100_U32;
}

void HeapSpace::ObjectMemorySpace::Initialize(size_t initialSize, size_t maxSize)
{
    minSize_ = initialSize;
    maxSize_ = maxSize;
    ASSERT(minSize_ <= maxSize_);
    // Set current space size as initial_size
    currentSize_ = minSize_;
}

void HeapSpace::ObjectMemorySpace::ClampNewMaxSize(size_t newMaxSize)
{
    ASSERT(newMaxSize >= currentSize_);
    maxSize_ = std::min(newMaxSize, maxSize_);
}

inline void HeapSpace::ObjectMemorySpace::IncreaseBy(uint64_t bytes)
{
    currentSize_ = std::min(AlignUp(currentSize_ + bytes, DEFAULT_ALIGNMENT_IN_BYTES), static_cast<uint64_t>(maxSize_));
}

inline void HeapSpace::ObjectMemorySpace::ReduceBy(size_t bytes)
{
    ASSERT(currentSize_ >= bytes);
    currentSize_ = AlignUp(currentSize_ - bytes, DEFAULT_ALIGNMENT_IN_BYTES);
    currentSize_ = std::max(currentSize_, minSize_);
}

void HeapSpace::ObjectMemorySpace::ComputeNewSize(size_t freeBytes, double minFreePercentage, double maxFreePercentage)
{
    ASSERT(freeBytes <= currentSize_);
    // How many bytes are used in space now
    size_t usedBytes = currentSize_ - freeBytes;

    uint64_t minNeededBytes = static_cast<double>(usedBytes) / (1.0 - minFreePercentage);
    if (currentSize_ < minNeededBytes) {
        IncreaseBy(minNeededBytes - currentSize_);
        return;
    }

    uint64_t maxNeededBytes = static_cast<double>(usedBytes) / (1.0 - maxFreePercentage);
    if (currentSize_ > maxNeededBytes) {
        ReduceBy(currentSize_ - maxNeededBytes);
    }
}

inline size_t HeapSpace::GetCurrentFreeBytes(size_t bytesNotInThisSpace) const
{
    ASSERT(isInitialized_);
    size_t usedBytes = PoolManager::GetMmapMemPool()->GetObjectUsedBytes();
    ASSERT(usedBytes >= bytesNotInThisSpace);
    size_t usedBytesInCurrentSpace = usedBytes - bytesNotInThisSpace;
    ASSERT(GetCurrentSize() >= usedBytesInCurrentSpace);
    return GetCurrentSize() - usedBytesInCurrentSpace;
}

void HeapSpace::ComputeNewSize()
{
    os::memory::WriteLockHolder lock(heapLock_);
    memSpace_.ComputeNewSize(GetCurrentFreeBytes(), minFreePercentage_, maxFreePercentage_);
    // Get current free bytes count after computing new size
    size_t currentFreeBytesInSpace = GetCurrentFreeBytes();
    // If saved pool size was very big and such pool can not be allocate after GC
    // then we increase space to allocate this pool
    if (memSpace_.savedPoolSize > currentFreeBytesInSpace) {
        memSpace_.IncreaseBy(memSpace_.savedPoolSize - currentFreeBytesInSpace);
        memSpace_.savedPoolSize = 0;
        // Free bytes after increase space for new pool will = 0, so yet increase space
        memSpace_.ComputeNewSize(0, minFreePercentage_, maxFreePercentage_);
    }
    // ComputeNewSize is called on GC end
    SetIsWorkGC(false);
}

size_t HeapSpace::GetHeapSize() const
{
    return PoolManager::GetMmapMemPool()->GetObjectUsedBytes();
}

inline std::optional<size_t> HeapSpace::WillAlloc(size_t poolSize, size_t currentFreeBytesInSpace,
                                                  const ObjectMemorySpace *memSpace) const
{
    ASSERT(isInitialized_);
    // If can allocate pool (from free pool map or non-used memory) then just do it
    if (LIKELY(poolSize <= currentFreeBytesInSpace)) {
        // We have enough memory for allocation, no need to increase heap
        return {0};
    }
    // If we allocate pool during GC work then we must allocate new pool anyway, so we will try to increase heap space
    if (IsWorkGC()) {
        // if requested pool size greater free bytes in current heap space and non occupied memory then we can not
        // allocate such pool, so we need to trigger GC
        if (currentFreeBytesInSpace + memSpace->GetCurrentNonOccupiedSize() < poolSize) {
            return std::nullopt;
        }
        // In this case we need increase space for allocate new pool
        return {poolSize - currentFreeBytesInSpace};
    }
    // Otherwise we need to trigger GC
    return std::nullopt;
}

size_t HeapSpace::GetCurrentSize() const
{
    return memSpace_.GetCurrentSize();
}

void HeapSpace::ClampCurrentMaxHeapSize()
{
    os::memory::WriteLockHolder lock(heapLock_);
    memSpace_.ClampNewMaxSize(
        AlignUp(memSpace_.GetCurrentSize() + PANDA_DEFAULT_POOL_SIZE, PANDA_POOL_ALIGNMENT_IN_BYTES));
    PoolManager::GetMmapMemPool()->ReleaseFreePagesToOS();
}

inline Pool HeapSpace::TryAllocPoolBase(size_t poolSize, SpaceType spaceType, AllocatorType allocatorType,
                                        void *allocatorPtr, size_t currentFreeBytesInSpace, ObjectMemorySpace *memSpace,
                                        OSPagesAllocPolicy allocPolicy)
{
    auto increaseBytesOrNotAlloc = WillAlloc(poolSize, currentFreeBytesInSpace, memSpace);
    // Increase heap space if needed and allocate pool
    if (increaseBytesOrNotAlloc) {
        memSpace->IncreaseBy(increaseBytesOrNotAlloc.value());
        if (allocPolicy == OSPagesAllocPolicy::NO_POLICY) {
            return PoolManager::GetMmapMemPool()->template AllocPool<OSPagesAllocPolicy::NO_POLICY>(
                poolSize, spaceType, allocatorType, allocatorPtr);
        }
        return PoolManager::GetMmapMemPool()->template AllocPool<OSPagesAllocPolicy::ZEROED_MEMORY>(
            poolSize, spaceType, allocatorType, allocatorPtr);
    }
    // Save pool size for computing new space size
    memSpace->savedPoolSize = poolSize;
    return NULLPOOL;
}

Pool HeapSpace::TryAllocPool(size_t poolSize, SpaceType spaceType, AllocatorType allocatorType, void *allocatorPtr)
{
    os::memory::WriteLockHolder lock(heapLock_);
    return TryAllocPoolBase(poolSize, spaceType, allocatorType, allocatorPtr, GetCurrentFreeBytes(), &memSpace_);
}

inline Arena *HeapSpace::TryAllocArenaBase(size_t arenaSize, SpaceType spaceType, AllocatorType allocatorType,
                                           void *allocatorPtr, size_t currentFreeBytesInSpace,
                                           ObjectMemorySpace *memSpace)
{
    auto increaseBytesOrNotAlloc = WillAlloc(arenaSize, currentFreeBytesInSpace, memSpace);
    // Increase heap space if needed and allocate arena
    if (increaseBytesOrNotAlloc.has_value()) {
        memSpace->IncreaseBy(increaseBytesOrNotAlloc.value());
        return PoolManager::AllocArena(arenaSize, spaceType, allocatorType, allocatorPtr);
    }
    // Save arena size for computing new space size
    memSpace->savedPoolSize = arenaSize;
    return nullptr;
}

Arena *HeapSpace::TryAllocArena(size_t arenaSize, SpaceType spaceType, AllocatorType allocatorType, void *allocatorPtr)
{
    os::memory::WriteLockHolder lock(heapLock_);
    return TryAllocArenaBase(arenaSize, spaceType, allocatorType, allocatorPtr, GetCurrentFreeBytes(), &memSpace_);
}

void HeapSpace::FreePool(void *poolMem, size_t poolSize, bool releasePages)
{
    os::memory::ReadLockHolder lock(heapLock_);
    ASSERT(isInitialized_);
    // Just free pool
    if (releasePages) {
        PoolManager::GetMmapMemPool()->FreePool<OSPagesPolicy::IMMEDIATE_RETURN>(poolMem, poolSize);
    } else {
        PoolManager::GetMmapMemPool()->FreePool<OSPagesPolicy::NO_RETURN>(poolMem, poolSize);
    }
}

void HeapSpace::FreeArena(Arena *arena)
{
    os::memory::ReadLockHolder lock(heapLock_);
    ASSERT(isInitialized_);
    // Just free arena
    PoolManager::FreeArena(arena);
}

void GenerationalSpaces::Initialize(size_t initialYoungSize, bool wasSetInitialYoungSize, size_t maxYoungSize,
                                    bool wasSetMaxYoungSize, size_t initialTotalSize, size_t maxTotalSize,
                                    uint32_t minFreePercentage, uint32_t maxFreePercentage)
{
    // Temporary save total heap size parameters and set percetages
    HeapSpace::Initialize(initialTotalSize, maxTotalSize, minFreePercentage, maxFreePercentage);

    if (!wasSetInitialYoungSize && wasSetMaxYoungSize) {
        initialYoungSize = maxYoungSize;
    } else if (initialYoungSize > maxYoungSize) {
        LOG_IF(wasSetInitialYoungSize && wasSetMaxYoungSize, WARNING, RUNTIME)
            << "Initial young size(init-young-space-size=" << initialYoungSize
            << ") is larger than maximum young size (young-space-size=" << maxYoungSize
            << "). Set maximum young size to " << initialYoungSize;
        maxYoungSize = initialYoungSize;
    }
    youngSpace_.Initialize(initialYoungSize, maxYoungSize);
    ASSERT(youngSpace_.GetCurrentSize() <= memSpace_.GetCurrentSize());
    ASSERT(youngSpace_.GetMaxSize() <= memSpace_.GetMaxSize());
    // Use mem_space_ as tenured space
    memSpace_.Initialize(memSpace_.GetCurrentSize() - youngSpace_.GetCurrentSize(),
                         memSpace_.GetMaxSize() - youngSpace_.GetMaxSize());
}

size_t GenerationalSpaces::GetCurrentFreeYoungSize() const
{
    os::memory::ReadLockHolder lock(heapLock_);
    return GetCurrentFreeYoungSizeUnsafe();
}

size_t GenerationalSpaces::GetCurrentFreeTenuredSize() const
{
    os::memory::ReadLockHolder lock(heapLock_);
    return GetCurrentFreeTenuredSizeUnsafe();
}

size_t GenerationalSpaces::GetCurrentFreeYoungSizeUnsafe() const
{
    size_t allOccupiedYoungSize = youngSizeInSeparatePools_ + youngSizeInSharedPools_;
    ASSERT(youngSpace_.GetCurrentSize() >= allOccupiedYoungSize);
    return youngSpace_.GetCurrentSize() - allOccupiedYoungSize;
}

size_t GenerationalSpaces::GetCurrentFreeTenuredSizeUnsafe() const
{
    ASSERT(sharedPoolsSize_ >= tenuredSizeInSharedPools_);
    // bytes_not_in_tenured_space = occupied pools size by young + non-tenured size in shared pool
    return GetCurrentFreeBytes(youngSizeInSeparatePools_ + (sharedPoolsSize_ - tenuredSizeInSharedPools_));
}

void GenerationalSpaces::ComputeNewSize()
{
    os::memory::WriteLockHolder lock(heapLock_);
    ComputeNewYoung();
    ComputeNewTenured();
    SetIsWorkGC(false);
}

void GenerationalSpaces::ComputeNewYoung()
{
    double minFreePercentage = GetMinFreePercentage();
    double maxFreePercentage = GetMaxFreePercentage();
    youngSpace_.ComputeNewSize(GetCurrentFreeYoungSizeUnsafe(), minFreePercentage, maxFreePercentage);
    // Get free bytes count after computing new young size
    size_t freeYoungBytesAfterComputing = GetCurrentFreeYoungSizeUnsafe();
    // If saved pool size was very big and such pool can not be allocate in young after GC
    // then we increase young space to allocate this pool
    if (youngSpace_.savedPoolSize > freeYoungBytesAfterComputing) {
        youngSpace_.IncreaseBy(youngSpace_.savedPoolSize - freeYoungBytesAfterComputing);
        youngSpace_.savedPoolSize = 0;
        // Free bytes after increase young space for new pool will = 0, so yet increase young space
        youngSpace_.ComputeNewSize(0, minFreePercentage, maxFreePercentage);
    }
}

void GenerationalSpaces::UpdateSize(size_t desiredYoungSize)
{
    os::memory::WriteLockHolder lock(heapLock_);
    UpdateYoungSize(desiredYoungSize);
    ComputeNewTenured();
    SetIsWorkGC(false);
}

size_t GenerationalSpaces::UpdateYoungSpaceMaxSize(size_t size)
{
    os::memory::WriteLockHolder lock(heapLock_);
    size_t oldSize = youngSpace_.GetMaxSize();
    youngSpace_.SetMaxSize(size);
    youngSpace_.UseFullSpace();
    return oldSize;
}

void GenerationalSpaces::UpdateYoungSize(size_t desiredYoungSize)
{
    if (desiredYoungSize < youngSpace_.GetCurrentSize()) {
        auto allOccupiedYoungSize = youngSizeInSharedPools_ + youngSizeInSeparatePools_;
        // we cannot reduce young size below already occupied
        auto desiredSize = std::max(desiredYoungSize, allOccupiedYoungSize);
        youngSpace_.ReduceBy(youngSpace_.GetCurrentSize() - desiredSize);
    } else if (desiredYoungSize > youngSpace_.GetCurrentSize()) {
        youngSpace_.IncreaseBy(desiredYoungSize - youngSpace_.GetCurrentSize());
    }
}

void GenerationalSpaces::ComputeNewTenured()
{
    double minFreePercentage = GetMinFreePercentage();
    double maxFreePercentage = GetMaxFreePercentage();
    memSpace_.ComputeNewSize(GetCurrentFreeTenuredSizeUnsafe(), minFreePercentage, maxFreePercentage);
    // Get free bytes count after computing new tenured size
    size_t freeTenuredBytesAfterComputing = GetCurrentFreeTenuredSizeUnsafe();
    // If saved pool size was very big and such pool can not be allocate in tenured after GC
    // then we increase tenured space to allocate this pool
    if (memSpace_.savedPoolSize > freeTenuredBytesAfterComputing) {
        memSpace_.IncreaseBy(memSpace_.savedPoolSize - freeTenuredBytesAfterComputing);
        memSpace_.savedPoolSize = 0;
        // Free bytes after increase tenured space for new pool will = 0, so yet increase tenured space
        memSpace_.ComputeNewSize(0, minFreePercentage, maxFreePercentage);
    }
}

size_t GenerationalSpaces::GetHeapSize() const
{
    os::memory::ReadLockHolder lock(heapLock_);
    size_t usedBytesInSeparatePools = PoolManager::GetMmapMemPool()->GetObjectUsedBytes() - sharedPoolsSize_;
    size_t usedBytesInSharedPool = youngSizeInSharedPools_ + tenuredSizeInSharedPools_;
    return usedBytesInSeparatePools + usedBytesInSharedPool;
}

bool GenerationalSpaces::CanAllocInSpace(bool isYoung, size_t chunkSize) const
{
    os::memory::ReadLockHolder lock(heapLock_);
    ASSERT(isInitialized_);
    return isYoung ? WillAlloc(chunkSize, GetCurrentFreeYoungSizeUnsafe(), &youngSpace_).has_value()
                   : WillAlloc(chunkSize, GetCurrentFreeTenuredSizeUnsafe(), &memSpace_).has_value();
}

size_t GenerationalSpaces::GetCurrentYoungSize() const
{
    os::memory::ReadLockHolder lock(heapLock_);
    ASSERT(isInitialized_);
    return youngSpace_.GetCurrentSize();
}

size_t GenerationalSpaces::GetMaxYoungSize() const
{
    ASSERT(isInitialized_);
    return youngSpace_.GetMaxSize();
}

void GenerationalSpaces::UseFullYoungSpace()
{
    os::memory::WriteLockHolder lock(heapLock_);
    ASSERT(isInitialized_);
    youngSpace_.UseFullSpace();
}

Pool GenerationalSpaces::AllocSharedPool(size_t poolSize, SpaceType spaceType, AllocatorType allocatorType,
                                         void *allocatorPtr)
{
    os::memory::WriteLockHolder lock(heapLock_);
    ASSERT(isInitialized_);
    auto sharedPool = PoolManager::GetMmapMemPool()->AllocPool(poolSize, spaceType, allocatorType, allocatorPtr);
    sharedPoolsSize_ += sharedPool.GetSize();
    return sharedPool;
}

Pool GenerationalSpaces::AllocAlonePoolForYoung(SpaceType spaceType, AllocatorType allocatorType, void *allocatorPtr)
{
    os::memory::WriteLockHolder lock(heapLock_);
    ASSERT(isInitialized_);
    auto youngPool =
        PoolManager::GetMmapMemPool()->AllocPool(youngSpace_.GetMaxSize(), spaceType, allocatorType, allocatorPtr);
    youngSizeInSeparatePools_ = youngPool.GetSize();
    return youngPool;
}

Pool GenerationalSpaces::TryAllocPoolForYoung(size_t poolSize, SpaceType spaceType, AllocatorType allocatorType,
                                              void *allocatorPtr)
{
    os::memory::WriteLockHolder lock(heapLock_);
    auto youngPool = TryAllocPoolBase(poolSize, spaceType, allocatorType, allocatorPtr, GetCurrentFreeYoungSizeUnsafe(),
                                      &youngSpace_);
    youngSizeInSeparatePools_ += youngPool.GetSize();
    return youngPool;
}

Pool GenerationalSpaces::TryAllocPoolForTenured(size_t poolSize, SpaceType spaceType, AllocatorType allocatorType,
                                                void *allocatorPtr, OSPagesAllocPolicy allocPolicy)
{
    os::memory::WriteLockHolder lock(heapLock_);
    return TryAllocPoolBase(poolSize, spaceType, allocatorType, allocatorPtr, GetCurrentFreeTenuredSizeUnsafe(),
                            &memSpace_, allocPolicy);
}

Pool GenerationalSpaces::TryAllocPool(size_t poolSize, SpaceType spaceType, AllocatorType allocatorType,
                                      void *allocatorPtr)
{
    return TryAllocPoolForTenured(poolSize, spaceType, allocatorType, allocatorPtr);
}

Arena *GenerationalSpaces::TryAllocArenaForTenured(size_t arenaSize, SpaceType spaceType, AllocatorType allocatorType,
                                                   void *allocatorPtr)
{
    os::memory::WriteLockHolder lock(heapLock_);
    return TryAllocArenaBase(arenaSize, spaceType, allocatorType, allocatorPtr, GetCurrentFreeTenuredSizeUnsafe(),
                             &memSpace_);
}

Arena *GenerationalSpaces::TryAllocArena(size_t arenaSize, SpaceType spaceType, AllocatorType allocatorType,
                                         void *allocatorPtr)
{
    return TryAllocArenaForTenured(arenaSize, spaceType, allocatorType, allocatorPtr);
}

void GenerationalSpaces::FreeSharedPool(void *poolMem, size_t poolSize)
{
    os::memory::WriteLockHolder lock(heapLock_);
    ASSERT(sharedPoolsSize_ >= poolSize);
    sharedPoolsSize_ -= poolSize;
    PoolManager::GetMmapMemPool()->FreePool(poolMem, poolSize);
}

void GenerationalSpaces::FreeYoungPool(void *poolMem, size_t poolSize, bool releasePages)
{
    os::memory::WriteLockHolder lock(heapLock_);
    ASSERT(youngSizeInSeparatePools_ >= poolSize);
    youngSizeInSeparatePools_ -= poolSize;
    if (releasePages) {
        PoolManager::GetMmapMemPool()->FreePool<OSPagesPolicy::IMMEDIATE_RETURN>(poolMem, poolSize);
    } else {
        PoolManager::GetMmapMemPool()->FreePool<OSPagesPolicy::NO_RETURN>(poolMem, poolSize);
    }
}

void GenerationalSpaces::PromoteYoungPool(size_t poolSize)
{
    os::memory::WriteLockHolder lock(heapLock_);
    ASSERT(youngSizeInSeparatePools_ >= poolSize);
    auto increaseBytesOrNotAlloc = WillAlloc(poolSize, GetCurrentFreeTenuredSizeUnsafe(), &memSpace_);
    youngSizeInSeparatePools_ -= poolSize;
    ASSERT(increaseBytesOrNotAlloc.has_value());
    memSpace_.IncreaseBy(increaseBytesOrNotAlloc.value());
}

void GenerationalSpaces::FreeTenuredPool(void *poolMem, size_t poolSize, bool releasePages)
{
    // For tenured we just free pool
    HeapSpace::FreePool(poolMem, poolSize, releasePages);
}

void GenerationalSpaces::IncreaseYoungOccupiedInSharedPool(size_t chunkSize)
{
    os::memory::WriteLockHolder lock(heapLock_);
    ASSERT(isInitialized_);
    size_t freeBytes = GetCurrentFreeYoungSizeUnsafe();
    // Here we sure that we must allocate new memory, but if free bytes count less requested size (for example, during
    // GC work) then we increase young space size
    if (freeBytes < chunkSize) {
        youngSpace_.IncreaseBy(chunkSize - freeBytes);
    }
    youngSizeInSharedPools_ += chunkSize;
    ASSERT(youngSizeInSharedPools_ + tenuredSizeInSharedPools_ <= sharedPoolsSize_);
}

void GenerationalSpaces::IncreaseTenuredOccupiedInSharedPool(size_t chunkSize)
{
    os::memory::WriteLockHolder lock(heapLock_);
    ASSERT(isInitialized_);
    size_t freeBytes = GetCurrentFreeTenuredSizeUnsafe();
    // Here we sure that we must allocate new memory, but if free bytes count less requested size (for example, during
    // GC work) then we increase tenured space size
    if (freeBytes < chunkSize) {
        memSpace_.IncreaseBy(chunkSize - freeBytes);
    }
    tenuredSizeInSharedPools_ += chunkSize;
    ASSERT(youngSizeInSharedPools_ + tenuredSizeInSharedPools_ <= sharedPoolsSize_);
}

void GenerationalSpaces::ReduceYoungOccupiedInSharedPool(size_t chunkSize)
{
    os::memory::WriteLockHolder lock(heapLock_);
    ASSERT(isInitialized_);
    ASSERT(youngSizeInSharedPools_ >= chunkSize);
    youngSizeInSharedPools_ -= chunkSize;
}

void GenerationalSpaces::ReduceTenuredOccupiedInSharedPool(size_t chunkSize)
{
    os::memory::WriteLockHolder lock(heapLock_);
    ASSERT(isInitialized_);
    ASSERT(tenuredSizeInSharedPools_ >= chunkSize);
    tenuredSizeInSharedPools_ -= chunkSize;
}

}  // namespace ark::mem
