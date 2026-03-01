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
#ifndef PANDA_MEM_HUMONGOUS_OBJ_ALLOCATOR_INL_H
#define PANDA_MEM_HUMONGOUS_OBJ_ALLOCATOR_INL_H

#include "runtime/mem/alloc_config.h"
#include "runtime/mem/humongous_obj_allocator.h"
#include "runtime/mem/object_helpers.h"

namespace ark::mem {

// NOLINTNEXTLINE(cppcoreguidelines-macro-usage)
#define LOG_HUMONGOUS_OBJ_ALLOCATOR(level) LOG(level, ALLOC) << "HumongousObjAllocator: "

template <typename AllocConfigT, typename LockConfigT>
HumongousObjAllocator<AllocConfigT, LockConfigT>::HumongousObjAllocator(MemStatsType *memStats,
                                                                        SpaceType typeAllocation)
    : typeAllocation_(typeAllocation), memStats_(memStats)
{
    LOG_HUMONGOUS_OBJ_ALLOCATOR(DEBUG) << "Initializing HumongousObjAllocator";
    LOG_HUMONGOUS_OBJ_ALLOCATOR(DEBUG) << "Initializing HumongousObjAllocator finished";
}

template <typename AllocConfigT, typename LockConfigT>
HumongousObjAllocator<AllocConfigT, LockConfigT>::~HumongousObjAllocator()
{
    LOG_HUMONGOUS_OBJ_ALLOCATOR(DEBUG) << "Destroying HumongousObjAllocator";
    LOG_HUMONGOUS_OBJ_ALLOCATOR(DEBUG) << "Destroying HumongousObjAllocator finished";
}

template <typename AllocConfigT, typename LockConfigT>
template <bool NEED_LOCK>
void *HumongousObjAllocator<AllocConfigT, LockConfigT>::Alloc(const size_t size, const Alignment align)
{
    os::memory::WriteLockHolder<LockConfigT, NEED_LOCK> wlock(allocFreeLock_);
    LOG_HUMONGOUS_OBJ_ALLOCATOR(DEBUG) << "Try to allocate memory with size " << size;

    // Check that we can get a memory header for the memory pointer by using PAGE_SIZE_MASK mask
    if (UNLIKELY(PANDA_PAGE_SIZE <= sizeof(MemoryPoolHeader) + GetAlignmentInBytes(align))) {
        ASSERT(PANDA_PAGE_SIZE > sizeof(MemoryPoolHeader) + GetAlignmentInBytes(align));
        LOG_HUMONGOUS_OBJ_ALLOCATOR(DEBUG) << "The align is too big for this allocator. Return nullptr.";
        return nullptr;
    }

    // NOTE(aemelenko): this is quite raw approximation.
    // We can save about sizeof(MemoryPoolHeader) / 2 bytes here
    // (BTW, it is not so much for MB allocations)
    size_t alignedSize = size + sizeof(MemoryPoolHeader) + GetAlignmentInBytes(align);

    void *mem = nullptr;

    if (UNLIKELY(alignedSize > HUMONGOUS_OBJ_ALLOCATOR_MAX_SIZE)) {
        // the size is too big
        LOG_HUMONGOUS_OBJ_ALLOCATOR(DEBUG) << "The size is too big for this allocator. Return nullptr.";
        return nullptr;
    }

    // First try to find suitable block in Reserved pools
    MemoryPoolHeader *memHeader = reservedPoolsList_.FindSuitablePool(alignedSize);
    if (memHeader != nullptr) {
        LOG_HUMONGOUS_OBJ_ALLOCATOR(DEBUG) << "Find reserved memory block with size " << memHeader->GetPoolSize();
        reservedPoolsList_.Pop(memHeader);
        memHeader->Alloc(size, align);
        mem = memHeader->GetMemory();
    } else {
        memHeader = freePoolsList_.FindSuitablePool(alignedSize);
        if (memHeader != nullptr) {
            LOG_HUMONGOUS_OBJ_ALLOCATOR(DEBUG) << "Find free memory block with size " << memHeader->GetPoolSize();
            freePoolsList_.Pop(memHeader);
            memHeader->Alloc(size, align);
            mem = memHeader->GetMemory();
        } else {
            LOG_HUMONGOUS_OBJ_ALLOCATOR(DEBUG) << "Can't find memory for this size";
            return nullptr;
        }
    }
    occupiedPoolsList_.Insert(memHeader);
    LOG_HUMONGOUS_OBJ_ALLOCATOR(DEBUG) << "Allocated memory at addr " << std::hex << mem;
    AllocConfigT::OnAlloc(memHeader->GetPoolSize(), typeAllocation_, memStats_);
    ASAN_UNPOISON_MEMORY_REGION(mem, size);
    AllocConfigT::MemoryInit(mem);
    ReleaseUnusedPagesOnAlloc(memHeader, size);
    return mem;
}

template <typename AllocConfigT, typename LockConfigT>
void HumongousObjAllocator<AllocConfigT, LockConfigT>::Free(void *mem)
{
    os::memory::WriteLockHolder wlock(allocFreeLock_);
    FreeUnsafe(mem);
}

template <typename AllocConfigT, typename LockConfigT>
void HumongousObjAllocator<AllocConfigT, LockConfigT>::FreeUnsafe(void *mem)
{
    if (UNLIKELY(mem == nullptr)) {
        LOG_HUMONGOUS_OBJ_ALLOCATOR(DEBUG) << "Try to free memory at invalid addr 0";
        return;
    }
    LOG_HUMONGOUS_OBJ_ALLOCATOR(DEBUG) << "Try to free memory at addr " << std::hex << mem;
#ifndef NDEBUG
    if (!AllocatedByHumongousObjAllocatorUnsafe(mem)) {
        LOG_HUMONGOUS_OBJ_ALLOCATOR(DEBUG) << "Try to free memory not from this allocator";
        return;
    }
#endif  // !NDEBUG

    // Each memory pool is PAGE_SIZE aligned, so to get a header we need just to align a pointer
    auto memHeader = static_cast<MemoryPoolHeader *>(ToVoidPtr(ToUintPtr(mem) & PAGE_SIZE_MASK));
    LOG_HUMONGOUS_OBJ_ALLOCATOR(DEBUG) << "It is a MemoryPoolHeader with addr " << std::hex << memHeader << " and size "
                                       << std::dec << memHeader->GetPoolSize();
    occupiedPoolsList_.Pop(memHeader);
    AllocConfigT::OnFree(memHeader->GetPoolSize(), typeAllocation_, memStats_);
    ASAN_POISON_MEMORY_REGION(memHeader, memHeader->GetPoolSize());
    InsertPool(memHeader);
    LOG_HUMONGOUS_OBJ_ALLOCATOR(DEBUG) << "Freed memory at addr " << std::hex << mem;
}

template <typename AllocConfigT, typename LockConfigT>
void HumongousObjAllocator<AllocConfigT, LockConfigT>::Collect(const GCObjectVisitor &deathCheckerFn)
{
    LOG_HUMONGOUS_OBJ_ALLOCATOR(DEBUG) << "Collecting started";
    IterateOverObjects([this, &deathCheckerFn](ObjectHeader *objectHeader) {
        if (deathCheckerFn(objectHeader) == ObjectStatus::DEAD_OBJECT) {
            LOG(DEBUG, GC) << "DELETE OBJECT " << GetDebugInfoAboutObject(objectHeader);
            FreeUnsafe(objectHeader);
        }
    });
    LOG_HUMONGOUS_OBJ_ALLOCATOR(DEBUG) << "Collecting finished";
}

template <typename AllocConfigT, typename LockConfigT>
template <typename ObjectVisitor>
void HumongousObjAllocator<AllocConfigT, LockConfigT>::IterateOverObjects(const ObjectVisitor &objectVisitor)
{
    LOG_HUMONGOUS_OBJ_ALLOCATOR(DEBUG) << "Iterating over objects started";
    MemoryPoolHeader *currentPool = nullptr;
    {
        os::memory::ReadLockHolder rlock(allocFreeLock_);
        currentPool = occupiedPoolsList_.GetListHead();
    }
    while (currentPool != nullptr) {
        os::memory::WriteLockHolder wlock(allocFreeLock_);
        LOG_HUMONGOUS_OBJ_ALLOCATOR(DEBUG) << "  check pool at addr " << std::hex << currentPool;
        MemoryPoolHeader *next = currentPool->GetNext();
        objectVisitor(static_cast<ObjectHeader *>(currentPool->GetMemory()));
        currentPool = next;
    }
    LOG_HUMONGOUS_OBJ_ALLOCATOR(DEBUG) << "Iterating over objects finished";
}

template <typename AllocConfigT, typename LockConfigT>
bool HumongousObjAllocator<AllocConfigT, LockConfigT>::AddMemoryPool(void *mem, size_t size)
{
    os::memory::WriteLockHolder wlock(allocFreeLock_);
    ASSERT(mem != nullptr);
    LOG_HUMONGOUS_OBJ_ALLOCATOR(DEBUG) << "Add memory pool to HumongousObjAllocator from  " << std::hex << mem
                                       << " with size " << std::dec << size;
    if (AlignUp(ToUintPtr(mem), PANDA_PAGE_SIZE) != ToUintPtr(mem)) {
        return false;
    }
    auto mempoolHeader = static_cast<MemoryPoolHeader *>(mem);
    mempoolHeader->Initialize(size, nullptr, nullptr);
    InsertPool(mempoolHeader);
    ASAN_POISON_MEMORY_REGION(mem, size);
    return true;
}

template <typename AllocConfigT, typename LockConfigT>
void HumongousObjAllocator<AllocConfigT, LockConfigT>::ReleaseUnusedPagesOnAlloc(MemoryPoolHeader *memoryPool,
                                                                                 size_t allocSize)
{
    ASSERT(memoryPool != nullptr);
    uintptr_t allocAddr = ToUintPtr(memoryPool->GetMemory());
    uintptr_t poolAddr = ToUintPtr(memoryPool);
    size_t poolSize = memoryPool->GetPoolSize();
    uintptr_t firstFreePage = AlignUp(allocAddr + allocSize, os::mem::GetPageSize());
    uintptr_t endOfLastFreePage = os::mem::AlignDownToPageSize(poolAddr + poolSize);
    if (firstFreePage < endOfLastFreePage) {
        os::mem::ReleasePages(firstFreePage, endOfLastFreePage);
    }
}

template <typename AllocConfigT, typename LockConfigT>
void HumongousObjAllocator<AllocConfigT, LockConfigT>::InsertPool(MemoryPoolHeader *header)
{
    LOG_HUMONGOUS_OBJ_ALLOCATOR(DEBUG) << "Try to insert pool with size " << header->GetPoolSize()
                                       << " in Reserved memory";
    // Try to insert it into ReservedMemoryPools
    MemoryPoolHeader *memHeader = reservedPoolsList_.TryToInsert(header);
    if (memHeader == nullptr) {
        LOG_HUMONGOUS_OBJ_ALLOCATOR(DEBUG) << "Successfully inserted in Reserved memory";
        // We successfully insert header into ReservedMemoryPools
        return;
    }
    // We have a crowded out pool or the "header" argument in mem_header
    // Insert it into free_pools
    LOG_HUMONGOUS_OBJ_ALLOCATOR(DEBUG) << "Couldn't insert into Reserved memory. Insert in free pools";
    freePoolsList_.Insert(memHeader);
}

template <typename AllocConfigT, typename LockConfigT>
template <typename MemVisitor>
void HumongousObjAllocator<AllocConfigT, LockConfigT>::VisitAndRemoveAllPools(const MemVisitor &memVisitor)
{
    // We call this method and return pools to the system.
    // Therefore, delete all objects to clear all external dependences
    LOG_HUMONGOUS_OBJ_ALLOCATOR(DEBUG) << "Clear all objects inside the allocator";
    os::memory::WriteLockHolder wlock(allocFreeLock_);
    occupiedPoolsList_.IterateAndPopOverPools(memVisitor);
    reservedPoolsList_.IterateAndPopOverPools(memVisitor);
    freePoolsList_.IterateAndPopOverPools(memVisitor);
}

template <typename AllocConfigT, typename LockConfigT>
template <typename MemVisitor>
void HumongousObjAllocator<AllocConfigT, LockConfigT>::VisitAndRemoveFreePools(const MemVisitor &memVisitor)
{
    os::memory::WriteLockHolder wlock(allocFreeLock_);
    freePoolsList_.IterateAndPopOverPools(memVisitor);
}

template <typename AllocConfigT, typename LockConfigT>
template <typename MemVisitor>
void HumongousObjAllocator<AllocConfigT, LockConfigT>::IterateOverObjectsInRange(const MemVisitor &memVisitor,
                                                                                 void *leftBorder, void *rightBorder)
{
    // NOTE: Current implementation doesn't look at PANDA_CROSSING_MAP_MANAGE_CROSSED_BORDER flag
    LOG_HUMONGOUS_OBJ_ALLOCATOR(DEBUG) << "HumongousObjAllocator::IterateOverObjectsInRange for range [" << std::hex
                                       << leftBorder << ", " << rightBorder << "]";
    ASSERT(ToUintPtr(rightBorder) >= ToUintPtr(leftBorder));
    // NOTE(aemelenko): These are temporary asserts because we can't do anything
    // if the range crosses different allocators memory pools
    ASSERT(ToUintPtr(rightBorder) - ToUintPtr(leftBorder) == (CrossingMapSingleton::GetCrossingMapGranularity() - 1U));
    ASSERT((ToUintPtr(rightBorder) & (~(CrossingMapSingleton::GetCrossingMapGranularity() - 1U))) ==
           (ToUintPtr(leftBorder) & (~(CrossingMapSingleton::GetCrossingMapGranularity() - 1U))));

    // Try to find a pool with this range
    MemoryPoolHeader *discoveredPool = nullptr;
    MemoryPoolHeader *currentPool = nullptr;
    {
        os::memory::ReadLockHolder rlock(allocFreeLock_);
        currentPool = occupiedPoolsList_.GetListHead();
    }
    while (currentPool != nullptr) {
        // Use current pool here because it is page aligned
        uintptr_t currentPoolStart = ToUintPtr(currentPool);
        uintptr_t currentPoolEnd = ToUintPtr(currentPool->GetMemory()) + currentPool->GetPoolSize();
        if (currentPoolStart <= ToUintPtr(leftBorder)) {
            // Check that this range is located in the same pool
            if (currentPoolEnd >= ToUintPtr(rightBorder)) {
                discoveredPool = currentPool;
                break;
            }
        }
        {
            os::memory::ReadLockHolder rlock(allocFreeLock_);
            currentPool = currentPool->GetNext();
        }
    }

    if (discoveredPool != nullptr) {
        LOG_HUMONGOUS_OBJ_ALLOCATOR(DEBUG)
            << "HumongousObjAllocator: It is a MemoryPoolHeader with addr " << std::hex << discoveredPool
            << " and size " << std::dec << discoveredPool->GetPoolSize();
        memVisitor(static_cast<ObjectHeader *>(discoveredPool->GetMemory()));
    } else {
        LOG_HUMONGOUS_OBJ_ALLOCATOR(DEBUG)
            << "HumongousObjAllocator This memory range is not covered by this allocator";
    }
    LOG_HUMONGOUS_OBJ_ALLOCATOR(DEBUG) << "HumongousObjAllocator::IterateOverObjectsInRange finished";
}

template <typename AllocConfigT, typename LockConfigT>
bool HumongousObjAllocator<AllocConfigT, LockConfigT>::AllocatedByHumongousObjAllocator(void *mem)
{
    os::memory::ReadLockHolder rlock(allocFreeLock_);
    return AllocatedByHumongousObjAllocatorUnsafe(mem);
}

template <typename AllocConfigT, typename LockConfigT>
bool HumongousObjAllocator<AllocConfigT, LockConfigT>::AllocatedByHumongousObjAllocatorUnsafe(void *mem)
{
    MemoryPoolHeader *currentPool = occupiedPoolsList_.GetListHead();
    while (currentPool != nullptr) {
        if (currentPool->GetMemory() == mem) {
            return true;
        }
        currentPool = currentPool->GetNext();
    }
    return false;
}

template <typename AllocConfigT, typename LockConfigT>
ATTRIBUTE_NO_SANITIZE_ADDRESS void HumongousObjAllocator<AllocConfigT, LockConfigT>::MemoryPoolHeader::Initialize(
    size_t size, MemoryPoolHeader *prev, MemoryPoolHeader *next)
{
    ASAN_UNPOISON_MEMORY_REGION(this, sizeof(MemoryPoolHeader));
    poolSize_ = size;
    prev_ = prev;
    next_ = next;
    memAddr_ = nullptr;
    ASAN_POISON_MEMORY_REGION(this, sizeof(MemoryPoolHeader));
}

template <typename AllocConfigT, typename LockConfigT>
ATTRIBUTE_NO_SANITIZE_ADDRESS void HumongousObjAllocator<AllocConfigT, LockConfigT>::MemoryPoolHeader::Alloc(
    size_t size, Alignment align)
{
    (void)size;
    ASAN_UNPOISON_MEMORY_REGION(this, sizeof(MemoryPoolHeader));
    memAddr_ = ToVoidPtr(AlignUp(ToUintPtr(this) + sizeof(MemoryPoolHeader), GetAlignmentInBytes(align)));
    ASSERT(ToUintPtr(memAddr_) + size <= ToUintPtr(this) + poolSize_);
    ASAN_POISON_MEMORY_REGION(this, sizeof(MemoryPoolHeader));
}

template <typename AllocConfigT, typename LockConfigT>
ATTRIBUTE_NO_SANITIZE_ADDRESS void HumongousObjAllocator<AllocConfigT, LockConfigT>::MemoryPoolHeader::PopHeader()
{
    ASAN_UNPOISON_MEMORY_REGION(this, sizeof(MemoryPoolHeader));
    if (prev_ != nullptr) {
        ASAN_UNPOISON_MEMORY_REGION(prev_, sizeof(MemoryPoolHeader));
        prev_->SetNext(next_);
        ASAN_POISON_MEMORY_REGION(prev_, sizeof(MemoryPoolHeader));
    }
    if (next_ != nullptr) {
        ASAN_UNPOISON_MEMORY_REGION(next_, sizeof(MemoryPoolHeader));
        next_->SetPrev(prev_);
        ASAN_POISON_MEMORY_REGION(next_, sizeof(MemoryPoolHeader));
    }
    next_ = nullptr;
    prev_ = nullptr;
    ASAN_POISON_MEMORY_REGION(this, sizeof(MemoryPoolHeader));
}

template <typename AllocConfigT, typename LockConfigT>
void HumongousObjAllocator<AllocConfigT, LockConfigT>::MemoryPoolList::Pop(MemoryPoolHeader *pool)
{
    LOG_HUMONGOUS_OBJ_ALLOCATOR(DEBUG) << "Pop a pool with addr " << std::hex << pool << " from the pool list";
    ASSERT(IsInThisList(pool));
    if (head_ == pool) {
        head_ = pool->GetNext();
        LOG_HUMONGOUS_OBJ_ALLOCATOR(DEBUG) << "It was a pointer to list head. Change head to " << std::hex << head_;
    }
    pool->PopHeader();
}

template <typename AllocConfigT, typename LockConfigT>
void HumongousObjAllocator<AllocConfigT, LockConfigT>::MemoryPoolList::Insert(MemoryPoolHeader *pool)
{
    LOG_HUMONGOUS_OBJ_ALLOCATOR(DEBUG) << "Insert a pool with addr " << std::hex << pool << " into the pool list";
    if (head_ != nullptr) {
        head_->SetPrev(pool);
    } else {
        LOG_HUMONGOUS_OBJ_ALLOCATOR(DEBUG) << "The head was not initialized. Set it up.";
    }
    pool->SetNext(head_);
    pool->SetPrev(nullptr);
    head_ = pool;
}

template <typename AllocConfigT, typename LockConfigT>
// CC-OFFNXT(G.FMT.07) project code style
typename HumongousObjAllocator<AllocConfigT, LockConfigT>::MemoryPoolHeader *
HumongousObjAllocator<AllocConfigT, LockConfigT>::MemoryPoolList::FindSuitablePool(size_t size)
{
    LOG_HUMONGOUS_OBJ_ALLOCATOR(DEBUG) << "Try to find suitable pool for memory with size " << size;
    MemoryPoolHeader *curPool = head_;
    while (curPool != nullptr) {
        if (curPool->GetPoolSize() >= size) {
            break;
        }
        curPool = curPool->GetNext();
    }
    LOG_HUMONGOUS_OBJ_ALLOCATOR(DEBUG) << "Found a pool with addr " << std::hex << curPool;
    return curPool;
}

template <typename AllocConfigT, typename LockConfigT>
bool HumongousObjAllocator<AllocConfigT, LockConfigT>::MemoryPoolList::IsInThisList(MemoryPoolHeader *pool)
{
    // NOTE(aemelenko): Do it only in debug build
    MemoryPoolHeader *curPool = head_;
    while (curPool != nullptr) {
        if (curPool == pool) {
            break;
        }
        curPool = curPool->GetNext();
    }
    return curPool != nullptr;
}

template <typename AllocConfigT, typename LockConfigT>
template <typename MemVisitor>
void HumongousObjAllocator<AllocConfigT, LockConfigT>::MemoryPoolList::IterateAndPopOverPools(
    const MemVisitor &memVisitor)
{
    MemoryPoolHeader *currentPool = head_;
    while (currentPool != nullptr) {
        MemoryPoolHeader *tmp = currentPool->GetNext();
        this->Pop(currentPool);
        memVisitor(currentPool, currentPool->GetPoolSize());
        currentPool = tmp;
    }
}

template <typename AllocConfigT, typename LockConfigT>
// CC-OFFNXT(G.FMT.07) project code style
typename HumongousObjAllocator<AllocConfigT, LockConfigT>::MemoryPoolHeader *
HumongousObjAllocator<AllocConfigT, LockConfigT>::ReservedMemoryPools::TryToInsert(MemoryPoolHeader *pool)
{
    LOG_HUMONGOUS_OBJ_ALLOCATOR(DEBUG) << "Try to insert a pool in Reserved memory with addr " << std::hex << pool;
    if (pool->GetPoolSize() > MAX_POOL_SIZE) {
        // This pool is too big for inserting in Reserved
        LOG_HUMONGOUS_OBJ_ALLOCATOR(DEBUG) << "It is too big for Reserved memory";
        return pool;
    }
    if (elementsCount_ < MAX_POOLS_AMOUNT) {
        // We can insert the memory pool to Reserved
        SortedInsert(pool);
        elementsCount_++;
        LOG_HUMONGOUS_OBJ_ALLOCATOR(DEBUG) << "We don't have max amount of elements in Reserved list. Just insert.";
        return nullptr;
    }
    // We have the max amount of elements in the Reserved pools list
    // Try to swap the smallest pool (which is the first because it is ordered list)
    LOG_HUMONGOUS_OBJ_ALLOCATOR(DEBUG) << "We have max amount of elements in Reserved list.";
    MemoryPoolHeader *smallestPool = this->GetListHead();
    if (smallestPool == nullptr) {
        // It is the only variant when smallest_pool can be equal to nullptr.
        ASSERT(MAX_POOLS_AMOUNT == 0);
        LOG_HUMONGOUS_OBJ_ALLOCATOR(DEBUG) << "MAX_POOLS_AMOUNT for Reserved list is equal to zero. Do nothing";
        return pool;
    }
    ASSERT(smallestPool != nullptr);
    if (smallestPool->GetPoolSize() >= pool->GetPoolSize()) {
        LOG_HUMONGOUS_OBJ_ALLOCATOR(DEBUG) << "The pool is too small. Do not insert it";
        return pool;
    }
    // Just pop this element from the list. Do not update elements_count_ value
    MemoryPoolList::Pop(smallestPool);
    SortedInsert(pool);
    LOG_HUMONGOUS_OBJ_ALLOCATOR(DEBUG) << "Swap the smallest element in Reserved list with addr " << std::hex
                                       << smallestPool;
    return smallestPool;
}

template <typename AllocConfigT, typename LockConfigT>
void HumongousObjAllocator<AllocConfigT, LockConfigT>::ReservedMemoryPools::SortedInsert(MemoryPoolHeader *pool)
{
    size_t poolSize = pool->GetPoolSize();
    MemoryPoolHeader *listHead = this->GetListHead();
    if (listHead == nullptr) {
        this->Insert(pool);
        return;
    }
    if (listHead->GetPoolSize() >= poolSize) {
        // Do this comparison to not update head_ in this method
        this->Insert(pool);
        return;
    }
    MemoryPoolHeader *cur = listHead;
    while (cur != nullptr) {
        if (cur->GetPoolSize() >= poolSize) {
            pool->SetNext(cur);
            pool->SetPrev(cur->GetPrev());
            cur->GetPrev()->SetNext(pool);
            cur->SetPrev(pool);
            return;
        }
        MemoryPoolHeader *next = cur->GetNext();
        if (next == nullptr) {
            cur->SetNext(pool);
            pool->SetNext(nullptr);
            pool->SetPrev(cur);
            return;
        }
        cur = next;
    }
}

template <typename AllocConfigT, typename LockConfigT>
bool HumongousObjAllocator<AllocConfigT, LockConfigT>::ContainObject(const ObjectHeader *obj)
{
    return AllocatedByHumongousObjAllocatorUnsafe(const_cast<ObjectHeader *>(obj));
}

template <typename AllocConfigT, typename LockConfigT>
bool HumongousObjAllocator<AllocConfigT, LockConfigT>::IsLive(const ObjectHeader *obj)
{
    ASSERT(ContainObject(obj));
    auto *memHeader = static_cast<MemoryPoolHeader *>(ToVoidPtr(ToUintPtr(obj) & PAGE_SIZE_MASK));
    ASSERT(PoolManager::GetMmapMemPool()->GetStartAddrPoolForAddr(
               // CC-OFFNXT(G.FMT.06,G.FMT.06-CPP) project code style
               static_cast<void *>(const_cast<ObjectHeader *>(obj))) == static_cast<void *>(memHeader));
    return memHeader->GetMemory() == static_cast<void *>(const_cast<ObjectHeader *>(obj));
}

#undef LOG_HUMONGOUS_OBJ_ALLOCATOR

}  // namespace ark::mem

#endif  // PANDA_MEM_HUMONGOUS_OBJ_ALLOCATOR_INL_H
