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
#ifndef PANDA_MEM_FREELIST_ALLOCATOR_INL_H
#define PANDA_MEM_FREELIST_ALLOCATOR_INL_H

#include "libarkbase/utils/logger.h"
#include "runtime/mem/alloc_config.h"
#include "runtime/mem/freelist_allocator.h"
#include "runtime/mem/object_helpers.h"

namespace ark::mem {

// NOLINTNEXTLINE(cppcoreguidelines-macro-usage)
#define LOG_FREELIST_ALLOCATOR(level) LOG(level, ALLOC) << "FreeListAllocator: "

template <typename AllocConfigT, typename LockConfigT>
FreeListAllocator<AllocConfigT, LockConfigT>::FreeListAllocator(MemStatsType *memStats, SpaceType typeAllocation)
    : typeAllocation_(typeAllocation), memStats_(memStats)
{
    LOG_FREELIST_ALLOCATOR(DEBUG) << "Initializing FreeListAllocator";
    ASAN_POISON_MEMORY_REGION(&segregatedList_, sizeof(segregatedList_));
    LOG_FREELIST_ALLOCATOR(DEBUG) << "Initializing FreeListAllocator finished";
}

template <typename AllocConfigT, typename LockConfigT>
FreeListAllocator<AllocConfigT, LockConfigT>::~FreeListAllocator()
{
    LOG_FREELIST_ALLOCATOR(DEBUG) << "Destroying FreeListAllocator";
    ASAN_UNPOISON_MEMORY_REGION(&segregatedList_, sizeof(segregatedList_));
    LOG_FREELIST_ALLOCATOR(DEBUG) << "Destroying FreeListAllocator finished";
}

template <typename AllocConfigT, typename LockConfigT>
template <bool NEED_LOCK>
// CC-OFFNXT(G.FUN.01-CPP, G.FUD.06) Allocations perf critical, the change will create additional conditions and method
// calls that will degrade performance
inline void *FreeListAllocator<AllocConfigT, LockConfigT>::Alloc(size_t size, Alignment align)
{
    os::memory::WriteLockHolder<LockConfigT, NEED_LOCK> wlock(allocFreeLock_);
    LOG_FREELIST_ALLOCATOR(DEBUG) << "Try to allocate object with size " << std::dec << size;
    size_t allocSize = size;
    if (allocSize < FREELIST_ALLOCATOR_MIN_SIZE) {
        LOG_FREELIST_ALLOCATOR(DEBUG) << "Try to allocate an object with size less than min for this allocator";
        allocSize = FREELIST_ALLOCATOR_MIN_SIZE;
    }
    size_t defAlignedSize = AlignUp(allocSize, GetAlignmentInBytes(FREELIST_DEFAULT_ALIGNMENT));
    if (defAlignedSize > allocSize) {
        allocSize = defAlignedSize;
        LOG_FREELIST_ALLOCATOR(DEBUG) << "Align size to default alignment. New size = " << allocSize;
    }
    if (allocSize > FREELIST_MAX_ALLOC_SIZE) {
        LOG_FREELIST_ALLOCATOR(DEBUG) << "Try allocate too big memory for free list allocator. Return nullptr";
        return nullptr;
    }
    // Get best-fit memory piece from segregated list.
    MemoryBlockHeader *memoryBlock = GetFromSegregatedList(allocSize, align);
    if (memoryBlock == nullptr) {
        LOG_FREELIST_ALLOCATOR(DEBUG) << "Couldn't allocate memory";
        return nullptr;
    }
    LOG_FREELIST_ALLOCATOR(DEBUG) << "Found memory block at addr = " << std::hex << memoryBlock << " with size "
                                  << std::dec << memoryBlock->GetSize();
    ASSERT(memoryBlock->GetSize() >= allocSize);
    uintptr_t memoryPointer = ToUintPtr(memoryBlock->GetMemory());
    bool requiredPadding = false;
    if ((memoryPointer & (GetAlignmentInBytes(align) - 1)) != 0U) {
        LOG_FREELIST_ALLOCATOR(DEBUG) << "Raw memory is not aligned as we need. Create special header for padding";
        // Raw memory pointer is not aligned as we expected
        // We need to create extra header inside
        uintptr_t alignedMemoryPointer = AlignUp(memoryPointer + sizeof(MemoryBlockHeader), GetAlignmentInBytes(align));
        size_t sizeWithPadding = allocSize + (alignedMemoryPointer - memoryPointer);
        ASSERT(memoryBlock->GetSize() >= sizeWithPadding);
        auto paddingHeader =
            static_cast<MemoryBlockHeader *>(ToVoidPtr(alignedMemoryPointer - sizeof(MemoryBlockHeader)));
        LOG_FREELIST_ALLOCATOR(DEBUG) << "Created padding header at addr " << std::hex << paddingHeader;
        paddingHeader->Initialize(allocSize, memoryBlock);
        paddingHeader->SetAsPaddingHeader();
        // Update values
        memoryPointer = alignedMemoryPointer;
        allocSize = sizeWithPadding;
        requiredPadding = true;
    }
    if (CanCreateNewBlockFromRemainder(memoryBlock, allocSize)) {
        LOG_FREELIST_ALLOCATOR(DEBUG) << "Created new memory block from the remainder part:";
        MemoryBlockHeader *newFreeBlock = SplitMemoryBlocks(memoryBlock, allocSize);
        LOG_FREELIST_ALLOCATOR(DEBUG) << "New block started at addr " << std::hex << newFreeBlock << " with size "
                                      << std::dec << newFreeBlock->GetSize();
        memoryBlock->SetUsed();
        FreeListHeader *newFreeListElement = TryToCoalescing(newFreeBlock);
        ASSERT(!newFreeListElement->IsUsed());
        AddToSegregatedList(newFreeListElement);
    } else {
        LOG_FREELIST_ALLOCATOR(DEBUG) << "Can't create new block from the remainder. Use full block.";
        memoryBlock->SetUsed();
    }
    if (requiredPadding) {
        // We must update some values in current memory_block
        uintptr_t paddingSize = memoryPointer - ToUintPtr(memoryBlock->GetMemory());
        if (paddingSize == sizeof(MemoryBlockHeader)) {
            LOG_FREELIST_ALLOCATOR(DEBUG) << "SetHasPaddingHeaderAfter";
            memoryBlock->SetPaddingHeaderStoredAfterHeader();
        } else {
            LOG_FREELIST_ALLOCATOR(DEBUG) << "SetHasPaddingSizeAfter, size = " << paddingSize;
            memoryBlock->SetPaddingSizeStoredAfterHeader();
            memoryBlock->SetPaddingSize(paddingSize);
        }
    }
    LOG_FREELIST_ALLOCATOR(DEBUG) << "Allocated memory at addr " << std::hex << ToVoidPtr(memoryPointer);
    {
        AllocConfigT::OnAlloc(memoryBlock->GetSize(), typeAllocation_, memStats_);
        // It is not the object size itself, because we can't compute it from MemoryBlockHeader structure at Free call.
        // It is an approximation.
        size_t currentSize =
            ToUintPtr(memoryBlock) + memoryBlock->GetSize() + sizeof(MemoryBlockHeader) - memoryPointer;
        AllocConfigT::AddToCrossingMap(ToVoidPtr(memoryPointer), currentSize);
    }
    ASAN_UNPOISON_MEMORY_REGION(ToVoidPtr(memoryPointer), size);
    AllocConfigT::MemoryInit(ToVoidPtr(memoryPointer));
#ifdef PANDA_MEASURE_FRAGMENTATION
    // Atomic with relaxed order reason: order is not required
    allocatedBytes_.fetch_add(memoryBlock->GetSize(), std::memory_order_relaxed);
#endif
    return ToVoidPtr(memoryPointer);
}

template <typename AllocConfigT, typename LockConfigT>
void FreeListAllocator<AllocConfigT, LockConfigT>::Free(void *mem)
{
    os::memory::WriteLockHolder wlock(allocFreeLock_);
    FreeUnsafe(mem);
}

template <typename AllocConfigT, typename LockConfigT>
void FreeListAllocator<AllocConfigT, LockConfigT>::FreeUnsafe(void *mem)
{
    if (UNLIKELY(mem == nullptr)) {
        LOG_FREELIST_ALLOCATOR(DEBUG) << "Try to free memory at invalid addr 0";
        return;
    }
    LOG_FREELIST_ALLOCATOR(DEBUG) << "Try to free memory at addr " << std::hex << mem;
#ifndef NDEBUG
    if (!AllocatedByFreeListAllocatorUnsafe(mem)) {
        LOG_FREELIST_ALLOCATOR(DEBUG) << "Try to free memory no from this allocator";
        return;
    }
#endif  // !NDEBUG

    MemoryBlockHeader *memoryHeader = GetFreeListMemoryHeader(mem);
    LOG_FREELIST_ALLOCATOR(DEBUG) << "It is a memory with header " << std::hex << memoryHeader << " with size "
                                  << std::dec << memoryHeader->GetSize() << " (probably with padding)";
    {
        AllocConfigT::OnFree(memoryHeader->GetSize(), typeAllocation_, memStats_);
        // It is not the object size itself, because we can't compute it from MemoryBlockHeader structure.
        // It is an approximation.
        size_t currentSize =
            ToUintPtr(memoryHeader) + memoryHeader->GetSize() + sizeof(MemoryBlockHeader) - ToUintPtr(mem);
        MemoryBlockHeader *prevUsedHeader = memoryHeader->GetPrevUsedHeader();
        void *prevObject = nullptr;
        size_t prevSize = 0;
        if (prevUsedHeader != nullptr) {
            prevObject = prevUsedHeader->GetMemory();
            prevSize = ToUintPtr(prevUsedHeader) + prevUsedHeader->GetSize() + sizeof(MemoryBlockHeader) -
                       ToUintPtr(prevUsedHeader->GetMemory());
        }
        MemoryBlockHeader *nextUsedHeader = memoryHeader->GetNextUsedHeader();
        void *nextObject = nullptr;
        if (nextUsedHeader != nullptr) {
            nextObject = nextUsedHeader->GetMemory();
        }
        AllocConfigT::RemoveFromCrossingMap(mem, currentSize, nextObject, prevObject, prevSize);
    }
    memoryHeader->SetUnused();
    memoryHeader->SetCommonHeader();
    FreeListHeader *newFreeListElement = TryToCoalescing(memoryHeader);
    ASAN_POISON_MEMORY_REGION(newFreeListElement, newFreeListElement->GetSize() + sizeof(MemoryBlockHeader));
    AddToSegregatedList(newFreeListElement);
    LOG_FREELIST_ALLOCATOR(DEBUG) << "Freed memory at addr " << std::hex << mem;
#ifdef PANDA_MEASURE_FRAGMENTATION
    // Atomic with relaxed order reason: order is not required
    allocatedBytes_.fetch_sub(memoryHeader->GetSize(), std::memory_order_relaxed);
#endif
}

// NOTE(aemelenko): We can create a mutex for each pool to increase performance,
// but it requires pool alignment restriction
// (we must compute memory pool header addr from a memory block addr stored inside it)

// During Collect method call we iterate over memory blocks in each pool.
// This iteration can cause race conditions in multithreaded mode.
// E.g. in this scenario:
// |-------|---------|-------------------|------------------------------------------------------------------|
// | time: | Thread: |    Description:   |                         Memory footprint:                        |
// |-------|---------|-------------------|------------------------------------------------------------------|
// |       |         | Thread0 starts    |  |..............Free  Block.............|...Allocated block...|  |
// |       |         | iterating         |  |                                                               |
// |   0   |    0    | over mem blocks   |  current block pointer                                           |
// |       |         | and current block |                                                                  |
// |       |         | is free block     |                                                                  |
// |-------|---------|-------------------|------------------------------------------------------------------|
// |       |         | Thread1           |  |...Allocated block...|................|...Allocated block...|  |
// |   1   |    1    | allocates memory  |                        |                                         |
// |       |         | at this block     |               Unused memory peace                                |
// |       |         |                   |                                                                  |
// |-------|---------|-------------------|------------------------------------------------------------------|
// |       |         | Thread1           |  |...Allocated block...|................|...Allocated block...|  |
// |   2   |    1    | set up values in  |  |                                                               |
// |       |         | this block header |  change size of this block                                       |
// |       |         | (set up size)     |                                                                  |
// |-------|---------|-------------------|------------------------------------------------------------------|
// |       |         | Thread0 reads     |  |...Allocated block...|................|...Allocated block...|  |
// |       |         | some garbage or   |                                                                  |
// |   3   |    0    | wrong value to    |  current block pointer - points to wrong memory                  |
// |       |         | calculate next    |                                                                  |
// |       |         | block pointer     |                                                                  |
// |-------|---------|-------------------|------------------------------------------------------------------|
//
// Therefore, we must unlock allocator's alloc/free methods only
// when we have a pointer to allocated block (i.e. IsUsed())

template <typename AllocConfigT, typename LockConfigT>
void FreeListAllocator<AllocConfigT, LockConfigT>::Collect(const GCObjectVisitor &deathCheckerFn)
{
    LOG_FREELIST_ALLOCATOR(DEBUG) << "Collecting started";
    // NOTE(aemelenko): Looks like we can unlock alloc_free_lock_ for not dead objects during collection
    IterateOverObjects([this, &deathCheckerFn](ObjectHeader *mem) {
        if (deathCheckerFn(mem) == ObjectStatus::DEAD_OBJECT) {
            LOG(DEBUG, GC) << "DELETE OBJECT " << GetDebugInfoAboutObject(mem);
            FreeUnsafe(mem);
        }
    });
    LOG_FREELIST_ALLOCATOR(DEBUG) << "Collecting finished";
}

template <typename AllocConfigT, typename LockConfigT>
template <typename ObjectVisitor>
void FreeListAllocator<AllocConfigT, LockConfigT>::IterateOverObjects(const ObjectVisitor &objectVisitor)
{
    LOG_FREELIST_ALLOCATOR(DEBUG) << "Iterating over objects started";
    MemoryPoolHeader *currentPool = nullptr;
    {
        // Do this under lock because the pointer for mempool_tail can be changed by other threads
        // in AddMemoryPool method call.
        // NOTE: We add each new pool at the mempool_tail_. Therefore, we can read it once and iterate to head.
        os::memory::ReadLockHolder rlock(allocFreeLock_);
        currentPool = mempoolTail_;
    }
    while (currentPool != nullptr) {
        LOG_FREELIST_ALLOCATOR(DEBUG) << "  iterate over " << std::hex << currentPool;
        MemoryBlockHeader *currentMemHeader = currentPool->GetFirstMemoryHeader();
        while (currentMemHeader != nullptr) {
            // Lock any possible memory headers changes in the allocator.
            os::memory::WriteLockHolder wlock(allocFreeLock_);
            if (currentMemHeader->IsUsed()) {
                // We can call Free inside mem_visitor, that's why we should lock everything
                objectVisitor(static_cast<ObjectHeader *>(currentMemHeader->GetMemory()));
                // At this point, current_mem_header can point to a nonexistent memory block
                // (e.g., if we coalesce it with the previous free block).
                // However, we still have valid MemoryBlockHeader class field here.
                // NOTE: Firstly, we coalesce current_mem_header block with next (if it is free)
                // and update current block size. Therefore, we have a valid pointer to next memory block.

                // After calling mem_visitor() the current_mem_header may points to free block of memory,
                // which can be modified at the alloc call in other thread.
                // Therefore, do not unlock until we have a pointer to the Used block.
                currentMemHeader = currentMemHeader->GetNextUsedHeader();
            } else {
                // Current header marked as Unused so it can be modify by an allocation in some thread.
                // So, read next header with in locked state.
                currentMemHeader = currentMemHeader->GetNextUsedHeader();
            }
            // We have a pointer to Used memory block, or to nullptr.
            // Therefore, we can unlock.
        }
        currentPool = currentPool->GetPrev();
    }
    LOG_FREELIST_ALLOCATOR(DEBUG) << "Iterating over objects finished";
}

template <typename AllocConfigT, typename LockConfigT>
bool FreeListAllocator<AllocConfigT, LockConfigT>::AddMemoryPool(void *mem, size_t size)
{
    // Lock alloc/free because we add new block to segregated list here.
    os::memory::WriteLockHolder wlock(allocFreeLock_);
    ASSERT(mem != nullptr);
    LOG_FREELIST_ALLOCATOR(DEBUG) << "Add memory pool to FreeListAllocator from  " << std::hex << mem << " with size "
                                  << std::dec << size;
    ASSERT((ToUintPtr(mem) & (sizeof(MemoryBlockHeader) - 1)) == 0U);
    auto mempoolHeader = static_cast<MemoryPoolHeader *>(mem);
    if (mempoolHead_ == nullptr) {
        LOG_FREELIST_ALLOCATOR(DEBUG) << "Initialize mempool_head_";
        mempoolHeader->Initialize(size, nullptr, nullptr);
        mempoolHead_ = mempoolHeader;
        mempoolTail_ = mempoolHeader;
    } else {
        LOG_FREELIST_ALLOCATOR(DEBUG) << "Add this memory pool at the tail after block " << std::hex << mempoolTail_;
        mempoolHeader->Initialize(size, mempoolTail_, nullptr);
        mempoolTail_->SetNext(mempoolHeader);
        mempoolTail_ = mempoolHeader;
    }
    MemoryBlockHeader *firstMemHeader = mempoolHeader->GetFirstMemoryHeader();
    firstMemHeader->Initialize(size - sizeof(MemoryBlockHeader) - sizeof(MemoryPoolHeader), nullptr);
    firstMemHeader->SetLastBlockInPool();
    AddToSegregatedList(static_cast<FreeListHeader *>(firstMemHeader));
    ASAN_POISON_MEMORY_REGION(mem, size);
    AllocConfigT::InitializeCrossingMapForMemory(mem, size);
    return true;
}

template <typename AllocConfigT, typename LockConfigT>
template <typename MemVisitor>
void FreeListAllocator<AllocConfigT, LockConfigT>::VisitAndRemoveAllPools(const MemVisitor &memVisitor)
{
    // We call this method and return pools to the system.
    // Therefore, delete all objects to clear all external dependences
    LOG_FREELIST_ALLOCATOR(DEBUG) << "Clear all objects inside the allocator";
    // Lock everything to avoid race condition.
    os::memory::WriteLockHolder wlock(allocFreeLock_);
    MemoryPoolHeader *currentPool = mempoolHead_;
    while (currentPool != nullptr) {
        // Use tmp in case if visitor with side effects
        MemoryPoolHeader *tmp = currentPool->GetNext();
        AllocConfigT::RemoveCrossingMapForMemory(currentPool, currentPool->GetSize());
        memVisitor(currentPool, currentPool->GetSize());
        currentPool = tmp;
    }
}

template <typename AllocConfigT, typename LockConfigT>
template <typename MemVisitor>
void FreeListAllocator<AllocConfigT, LockConfigT>::VisitAndRemoveFreePools(const MemVisitor &memVisitor)
{
    // Lock everything to avoid race condition.
    os::memory::WriteLockHolder wlock(allocFreeLock_);
    MemoryPoolHeader *currentPool = mempoolHead_;
    while (currentPool != nullptr) {
        // Use tmp in case if visitor with side effects
        MemoryPoolHeader *tmp = currentPool->GetNext();
        MemoryBlockHeader *firstBlock = currentPool->GetFirstMemoryHeader();
        if (firstBlock->IsLastBlockInPool() && !firstBlock->IsUsed()) {
            // We have only one big memory block in this pool,
            // and this block is not used
            LOG_FREELIST_ALLOCATOR(DEBUG)
                << "VisitAndRemoveFreePools: Remove free memory pool from allocator with start addr" << std::hex
                << currentPool << " and size " << std::dec << currentPool->GetSize()
                << " bytes with the first block at addr " << std::hex << firstBlock << " and size " << std::dec
                << firstBlock->GetSize();
            auto freeHeader = static_cast<FreeListHeader *>(firstBlock);
            freeHeader->PopFromFreeList();
            MemoryPoolHeader *next = currentPool->GetNext();
            MemoryPoolHeader *prev = currentPool->GetPrev();
            if (next != nullptr) {
                ASSERT(next->GetPrev() == currentPool);
                next->SetPrev(prev);
            } else {
                // This means that current pool is the last
                ASSERT(mempoolTail_ == currentPool);
                LOG_FREELIST_ALLOCATOR(DEBUG) << "VisitAndRemoveFreePools: Change pools tail pointer";
                mempoolTail_ = prev;
            }
            if (prev != nullptr) {
                ASSERT(prev->GetNext() == currentPool);
                prev->SetNext(next);
            } else {
                // This means that current pool is the first
                ASSERT(mempoolHead_ == currentPool);
                LOG_FREELIST_ALLOCATOR(DEBUG) << "VisitAndRemoveFreePools: Change pools head pointer";
                mempoolHead_ = next;
            }
            AllocConfigT::RemoveCrossingMapForMemory(currentPool, currentPool->GetSize());
            memVisitor(currentPool, currentPool->GetSize());
        }
        currentPool = tmp;
    }
    segregatedList_.ReleaseFreeMemoryBlocks();
}

template <typename AllocConfigT, typename LockConfigT>
template <typename MemVisitor>
void FreeListAllocator<AllocConfigT, LockConfigT>::IterateOverObjectsInRange(const MemVisitor &memVisitor,
                                                                             void *leftBorder, void *rightBorder)
{
    // NOTE: Current implementation doesn't look at PANDA_CROSSING_MAP_MANAGE_CROSSED_BORDER flag
    LOG_FREELIST_ALLOCATOR(DEBUG) << "FreeListAllocator::IterateOverObjectsInRange for range [" << std::hex
                                  << leftBorder << ", " << rightBorder << "]";
    ASSERT(ToUintPtr(rightBorder) >= ToUintPtr(leftBorder));
    // NOTE(aemelenko): These are temporary asserts because we can't do anything
    // if the range crosses different allocators memory pools
    ASSERT(ToUintPtr(rightBorder) - ToUintPtr(leftBorder) == (CrossingMapSingleton::GetCrossingMapGranularity() - 1U));
    ASSERT((ToUintPtr(rightBorder) & (~(CrossingMapSingleton::GetCrossingMapGranularity() - 1U))) ==
           (ToUintPtr(leftBorder) & (~(CrossingMapSingleton::GetCrossingMapGranularity() - 1U))));
    MemoryBlockHeader *firstMemoryHeader = nullptr;
    {
        // Do this under lock because the pointer to the first object in CrossingMap can be changed during
        // CrossingMap call.
        os::memory::ReadLockHolder rlock(allocFreeLock_);
        if (!AllocatedByFreeListAllocatorUnsafe(leftBorder) && !AllocatedByFreeListAllocatorUnsafe(rightBorder)) {
            LOG_FREELIST_ALLOCATOR(DEBUG) << "This memory range is not covered by this allocator";
            return;
        }
        void *objAddr = AllocConfigT::FindFirstObjInCrossingMap(leftBorder, rightBorder);
        if (objAddr == nullptr) {
            return;
        }
        ASSERT(AllocatedByFreeListAllocatorUnsafe(objAddr));
        MemoryBlockHeader *memoryHeader = GetFreeListMemoryHeader(objAddr);
        // Memory header is a pointer to an object which starts in this range or in the previous.
        // In the second case, this object may not crosses the border of the current range.
        // (but there is an object stored after it, which crosses the current range).
        ASSERT(ToUintPtr(memoryHeader->GetMemory()) <= ToUintPtr(rightBorder));
        firstMemoryHeader = memoryHeader;
    }
    ASSERT(firstMemoryHeader != nullptr);
    // Let's start iteration:
    MemoryBlockHeader *currentMemHeader = firstMemoryHeader;
    LOG_FREELIST_ALLOCATOR(DEBUG) << "FreeListAllocator::IterateOverObjectsInRange start iterating from obj with addr "
                                  << std::hex << firstMemoryHeader->GetMemory();
    while (currentMemHeader != nullptr) {
        // We don't lock allocator because we point to the used block which can't be changed
        // during the iteration in range.
        void *objAddr = currentMemHeader->GetMemory();
        if (ToUintPtr(objAddr) > ToUintPtr(rightBorder)) {
            // Iteration over
            break;
        }
        LOG_FREELIST_ALLOCATOR(DEBUG)
            << "FreeListAllocator::IterateOverObjectsInRange found obj in this range with addr " << std::hex << objAddr;
        memVisitor(static_cast<ObjectHeader *>(objAddr));
        {
            os::memory::ReadLockHolder rlock(allocFreeLock_);
            currentMemHeader = currentMemHeader->GetNextUsedHeader();
        }
    }
    LOG_FREELIST_ALLOCATOR(DEBUG) << "FreeListAllocator::IterateOverObjectsInRange finished";
}

template <typename AllocConfigT, typename LockConfigT>
void FreeListAllocator<AllocConfigT, LockConfigT>::CoalesceMemoryBlocks(MemoryBlockHeader *firstBlock,
                                                                        MemoryBlockHeader *secondBlock)
{
    LOG_FREELIST_ALLOCATOR(DEBUG) << "CoalesceMemoryBlock: "
                                  << "first block = " << std::hex << firstBlock << " with size " << std::dec
                                  << firstBlock->GetSize() << " ; second block = " << std::hex << secondBlock
                                  << " with size " << std::dec << secondBlock->GetSize();
    ASSERT(firstBlock != nullptr);
    ASSERT(firstBlock->GetNextHeader() == secondBlock);
    ASSERT(secondBlock != nullptr);
    ASSERT(firstBlock->CanBeCoalescedWithNext() || secondBlock->CanBeCoalescedWithPrev());
    firstBlock->Initialize(firstBlock->GetSize() + secondBlock->GetSize() + sizeof(MemoryBlockHeader),
                           firstBlock->GetPrevHeader());
    if (secondBlock->IsLastBlockInPool()) {
        LOG_FREELIST_ALLOCATOR(DEBUG) << "CoalesceMemoryBlock: second_block was the last in a pool";
        firstBlock->SetLastBlockInPool();
    } else {
        ASSERT(firstBlock->GetNextHeader() != nullptr);
        firstBlock->GetNextHeader()->SetPrevHeader(firstBlock);
    }
}

template <typename AllocConfigT, typename LockConfigT>
freelist::MemoryBlockHeader *FreeListAllocator<AllocConfigT, LockConfigT>::SplitMemoryBlocks(
    MemoryBlockHeader *memoryBlock, size_t firstBlockSize)
{
    ASSERT(memoryBlock->GetSize() > (firstBlockSize + sizeof(MemoryBlockHeader)));
    ASSERT(!memoryBlock->IsUsed());
    auto secondBlock =
        static_cast<MemoryBlockHeader *>(ToVoidPtr(ToUintPtr(memoryBlock->GetMemory()) + firstBlockSize));
    size_t secondBlockSize = memoryBlock->GetSize() - firstBlockSize - sizeof(MemoryBlockHeader);
    secondBlock->Initialize(secondBlockSize, memoryBlock);
    if (memoryBlock->IsLastBlockInPool()) {
        secondBlock->SetLastBlockInPool();
    } else {
        ASSERT(secondBlock->GetNextHeader() != nullptr);
        secondBlock->GetNextHeader()->SetPrevHeader(secondBlock);
    }
    memoryBlock->Initialize(firstBlockSize, memoryBlock->GetPrevHeader());
    return secondBlock;
}

template <typename AllocConfigT, typename LockConfigT>
freelist::MemoryBlockHeader *FreeListAllocator<AllocConfigT, LockConfigT>::GetFreeListMemoryHeader(void *mem)
{
    ASSERT(mem != nullptr);
    auto memoryHeader = static_cast<MemoryBlockHeader *>(ToVoidPtr(ToUintPtr(mem) - sizeof(MemoryBlockHeader)));
    if (!memoryHeader->IsPaddingHeader()) {
        // We got correct header of this memory, just return it
        return memoryHeader;
    }
    // This is aligned memory with some free space before the memory pointer
    // The previous header must be a pointer to correct header of this memory block
    LOG_FREELIST_ALLOCATOR(DEBUG) << "It is a memory with padding at head";
    return memoryHeader->GetPrevHeader();
}

template <typename AllocConfigT, typename LockConfigT>
bool FreeListAllocator<AllocConfigT, LockConfigT>::AllocatedByFreeListAllocator(void *mem)
{
    os::memory::ReadLockHolder rlock(allocFreeLock_);
    return AllocatedByFreeListAllocatorUnsafe(mem);
}

template <typename AllocConfigT, typename LockConfigT>
bool FreeListAllocator<AllocConfigT, LockConfigT>::AllocatedByFreeListAllocatorUnsafe(void *mem)
{
    // NOTE(aemelenko): Create more complex solution
    MemoryPoolHeader *currentPool = mempoolHead_;
    while (currentPool != nullptr) {
        // This assert means that we asked about memory inside MemoryPoolHeader
        ASSERT(ToUintPtr(currentPool->GetFirstMemoryHeader()) > ToUintPtr(mem) ||
               ToUintPtr(currentPool) < ToUintPtr(mem));
        if ((ToUintPtr(currentPool->GetFirstMemoryHeader()) < ToUintPtr(mem)) &&
            ((ToUintPtr(currentPool) + currentPool->GetSize()) > ToUintPtr(mem))) {
            return true;
        }
        currentPool = currentPool->GetNext();
    }
    return false;
}

template <typename AllocConfigT, typename LockConfigT>
freelist::FreeListHeader *FreeListAllocator<AllocConfigT, LockConfigT>::TryToCoalescing(MemoryBlockHeader *memoryHeader)
{
    ASSERT(memoryHeader != nullptr);
    LOG_FREELIST_ALLOCATOR(DEBUG) << "TryToCoalescing memory block";
    if (memoryHeader->CanBeCoalescedWithNext()) {
        ASSERT(!memoryHeader->GetNextHeader()->IsUsed());
        LOG_FREELIST_ALLOCATOR(DEBUG) << "Coalesce with next block";
        auto nextFreeList = static_cast<FreeListHeader *>(memoryHeader->GetNextHeader());
        ASSERT(nextFreeList != nullptr);
        // Pop this free list element from the list
        nextFreeList->PopFromFreeList();
        // Combine these two blocks together
        CoalesceMemoryBlocks(memoryHeader, static_cast<MemoryBlockHeader *>(nextFreeList));
    }
    if (memoryHeader->CanBeCoalescedWithPrev()) {
        ASSERT(!memoryHeader->GetPrevHeader()->IsUsed());
        LOG_FREELIST_ALLOCATOR(DEBUG) << "Coalesce with prev block";
        auto prevFreeList = static_cast<FreeListHeader *>(memoryHeader->GetPrevHeader());
        // Pop this free list element from the list
        prevFreeList->PopFromFreeList();
        // Combine these two blocks together
        CoalesceMemoryBlocks(static_cast<MemoryBlockHeader *>(prevFreeList), memoryHeader);
        memoryHeader = static_cast<MemoryBlockHeader *>(prevFreeList);
    }
    return static_cast<FreeListHeader *>(memoryHeader);
}

template <typename AllocConfigT, typename LockConfigT>
void FreeListAllocator<AllocConfigT, LockConfigT>::AddToSegregatedList(FreeListHeader *freeListElement)
{
    LOG_FREELIST_ALLOCATOR(DEBUG) << "AddToSegregatedList: Add new block into segregated-list with size "
                                  << freeListElement->GetSize();
    segregatedList_.AddMemoryBlock(freeListElement);
}

template <typename AllocConfigT, typename LockConfigT>
freelist::MemoryBlockHeader *FreeListAllocator<AllocConfigT, LockConfigT>::GetFromSegregatedList(size_t size,
                                                                                                 Alignment align)
{
    LOG_FREELIST_ALLOCATOR(DEBUG) << "GetFromSegregatedList: Try to find memory for size " << size << " with alignment "
                                  << align;
    size_t alignedSize = size;
    if (align != FREELIST_DEFAULT_ALIGNMENT) {
        // NOTE(aemelenko): This is raw but fast solution with bigger fragmentation.
        // It's better to add here this value, but I'm not 100% sure about all corner cases.
        // (GetAlignmentInBytes(align) + sizeof(MemoryBlockHeader) - GetAlignmentInBytes(FREELIST_DEFAULT_ALIGNMENT))
        alignedSize += (GetAlignmentInBytes(align) + sizeof(MemoryBlockHeader));
    }
    FreeListHeader *memBlock = segregatedList_.FindMemoryBlock(alignedSize);
    if (memBlock != nullptr) {
        memBlock->PopFromFreeList();
        ASSERT((AlignUp(ToUintPtr(memBlock->GetMemory()), GetAlignmentInBytes(align)) -
                ToUintPtr(memBlock->GetMemory()) + size) <= memBlock->GetSize());
    }
    return static_cast<MemoryBlockHeader *>(memBlock);
}

template <typename AllocConfigT, typename LockConfigT>
ATTRIBUTE_NO_SANITIZE_ADDRESS void FreeListAllocator<AllocConfigT, LockConfigT>::MemoryPoolHeader::Initialize(
    size_t size, MemoryPoolHeader *prev, MemoryPoolHeader *next)
{
    LOG_FREELIST_ALLOCATOR(DEBUG) << "Init a new memory pool with size " << size << " with prev link = " << std::hex
                                  << prev << " and next link = " << next;
    ASAN_UNPOISON_MEMORY_REGION(this, sizeof(MemoryPoolHeader));
    size_ = size;
    prev_ = prev;
    next_ = next;
    ASAN_POISON_MEMORY_REGION(this, sizeof(MemoryPoolHeader));
}

template <typename AllocConfigT, typename LockConfigT>
void FreeListAllocator<AllocConfigT, LockConfigT>::SegregatedList::AddMemoryBlock(FreeListHeader *freelistHeader)
{
    size_t size = freelistHeader->GetSize();
    size_t index = GetIndex(size);
    if (SEGREGATED_LIST_FAST_INSERT) {
        freeMemoryBlocks_[index].InsertNext(freelistHeader);
    } else {
        FreeListHeader *mostSuitable = FindTheMostSuitableBlockInOrderedList(index, size);
        // The most suitable block with such must be equal to this size,
        // or the last with the bigger size in the ordered list,
        // or nullptr
        if (mostSuitable == nullptr) {
            freeMemoryBlocks_[index].InsertNext(freelistHeader);
        } else {
            mostSuitable->InsertNext(freelistHeader);
        }
    }
}

template <typename AllocConfigT, typename LockConfigT>
freelist::FreeListHeader *FreeListAllocator<AllocConfigT, LockConfigT>::SegregatedList::FindMemoryBlock(size_t size)
{
    size_t index = GetIndex(size);
    FreeListHeader *head = GetFirstBlock(index);
    FreeListHeader *suitableBlock = nullptr;
    if (head != nullptr) {
        // We have some memory in this range. Try to find suitable block.
        if (SEGREGATED_LIST_FAST_INSERT) {
            // We don't have any order in inserting blocks,
            // and we need to iterate over the whole list
            suitableBlock = FindSuitableBlockInList(head, size);
        } else {
            // All blocks in this list are in descending order.
            // We can check the first one to determine if we have a block
            // with this size or not.
            suitableBlock = TryGetSuitableBlockBySize(head, size, index);
        }
    }

    if (suitableBlock == nullptr) {
        // We didn't find the block in the head list. Try to find block in other lists.
        index++;
        for (; index < SEGREGATED_LIST_SIZE; index++) {
            if (GetFirstBlock(index) == nullptr) {
                continue;
            }
            if constexpr (SEGREGATED_LIST_FAST_INSERT || SEGREGATED_LIST_FAST_EXTRACT) {
                // Just get the first one
                suitableBlock = GetFirstBlock(index);
            } else {
                suitableBlock = FindTheMostSuitableBlockInOrderedList(index, size);
            }
            break;
        }
    }

    if (suitableBlock != nullptr) {
        ASSERT(suitableBlock->GetSize() >= size);
    }

    return suitableBlock;
}

template <typename AllocConfigT, typename LockConfigT>
freelist::FreeListHeader *FreeListAllocator<AllocConfigT, LockConfigT>::SegregatedList::FindSuitableBlockInList(
    freelist::FreeListHeader *head, size_t size)
{
    FreeListHeader *suitableBlock = nullptr;
    for (FreeListHeader *current = head; current != nullptr; current = current->GetNextFree()) {
        if (current->GetSize() < size) {
            continue;
        }
        if constexpr (SEGREGATED_LIST_FAST_EXTRACT) {
            suitableBlock = current;
            break;
        }
        if (suitableBlock != nullptr) {
            suitableBlock = current;
        } else {
            if (suitableBlock->GetSize() > current->GetSize()) {
                suitableBlock = current;
            }
        }
        if (suitableBlock->GetSize() == size) {
            break;
        }
    }
    return suitableBlock;
}

template <typename AllocConfigT, typename LockConfigT>
freelist::FreeListHeader *FreeListAllocator<AllocConfigT, LockConfigT>::SegregatedList::TryGetSuitableBlockBySize(
    freelist::FreeListHeader *head, size_t size, size_t index)
{
    if (head->GetSize() >= size) {
        if constexpr (SEGREGATED_LIST_FAST_EXTRACT) {
            // Just get the first element
            return head;
        }
        // Try to find the mist suitable memory for this size.
        return FindTheMostSuitableBlockInOrderedList(index, size);
    }
    return nullptr;
}

template <typename AllocConfigT, typename LockConfigT>
void FreeListAllocator<AllocConfigT, LockConfigT>::SegregatedList::ReleaseFreeMemoryBlocks()
{
    for (size_t index = 0; index < SEGREGATED_LIST_SIZE; index++) {
        FreeListHeader *current = GetFirstBlock(index);
        while (current != nullptr) {
            size_t blockSize = current->GetSize();
            // Start address from which we can release pages
            uintptr_t startAddr = AlignUp(ToUintPtr(current) + sizeof(FreeListHeader), os::mem::GetPageSize());
            // End address before which we can release pages
            uintptr_t endAddr =
                os::mem::AlignDownToPageSize(ToUintPtr(current) + sizeof(MemoryBlockHeader) + blockSize);
            if (startAddr < endAddr) {
                os::mem::ReleasePages(startAddr, endAddr);
            }
            current = current->GetNextFree();
        }
    }
}

template <typename AllocConfigT, typename LockConfigT>
// CC-OFFNXT(G.FMT.07) project code style
// CC-OFFNXT(G.FUD.06) perf critical
inline freelist::FreeListHeader *
FreeListAllocator<AllocConfigT, LockConfigT>::SegregatedList::FindTheMostSuitableBlockInOrderedList(size_t index,
                                                                                                    size_t size)
{
    static_assert(!SEGREGATED_LIST_FAST_INSERT);
    FreeListHeader *current = GetFirstBlock(index);
    if (current == nullptr) {
        return nullptr;
    }
    size_t currentSize = current->GetSize();
    if (currentSize < size) {
        return nullptr;
    }
    while (currentSize != size) {
        FreeListHeader *next = current->GetNextFree();
        if (next == nullptr) {
            // the current free list header is the last in the list.
            break;
        }
        size_t nextSize = next->GetSize();
        if (nextSize < size) {
            // the next free list header is less than size,
            // so we don't need to iterate anymore.
            break;
        }
        current = next;
        currentSize = nextSize;
    }
    return current;
}

template <typename AllocConfigT, typename LockConfigT>
size_t FreeListAllocator<AllocConfigT, LockConfigT>::SegregatedList::GetIndex(size_t size)
{
    ASSERT(size >= FREELIST_ALLOCATOR_MIN_SIZE);
    size_t index = (size - FREELIST_ALLOCATOR_MIN_SIZE) / SEGREGATED_LIST_FREE_BLOCK_RANGE;
    return index < SEGREGATED_LIST_SIZE ? index : SEGREGATED_LIST_SIZE - 1;
}

template <typename AllocConfigT, typename LockConfigT>
bool FreeListAllocator<AllocConfigT, LockConfigT>::ContainObject(const ObjectHeader *obj)
{
    return AllocatedByFreeListAllocatorUnsafe(const_cast<ObjectHeader *>(obj));
}

template <typename AllocConfigT, typename LockConfigT>
bool FreeListAllocator<AllocConfigT, LockConfigT>::IsLive(const ObjectHeader *obj)
{
    ASSERT(ContainObject(obj));
    void *objMem = static_cast<void *>(const_cast<ObjectHeader *>(obj));
    // Get start address of pool via PoolManager for input object
    // for avoid iteration over all pools in allocator
    auto memPoolHeader =
        static_cast<MemoryPoolHeader *>(PoolManager::GetMmapMemPool()->GetStartAddrPoolForAddr(objMem));
    [[maybe_unused]] auto allocInfo =
        PoolManager::GetMmapMemPool()->GetAllocatorInfoForAddr(static_cast<void *>(memPoolHeader));
    ASSERT(allocInfo.GetAllocatorHeaderAddr() == static_cast<const void *>(this));
    MemoryBlockHeader *currentMemHeader = memPoolHeader->GetFirstMemoryHeader();
    while (currentMemHeader != nullptr) {
        if (currentMemHeader->IsUsed()) {
            if (currentMemHeader->GetMemory() == objMem) {
                return true;
            }
        }
        currentMemHeader = currentMemHeader->GetNextUsedHeader();
    }
    return false;
}

template <typename AllocConfigT, typename LockConfigT>
double FreeListAllocator<AllocConfigT, LockConfigT>::CalculateExternalFragmentation()
{
    return segregatedList_.CalculateExternalFragmentation();
}

template <typename AllocConfigT, typename LockConfigT>
double FreeListAllocator<AllocConfigT, LockConfigT>::SegregatedList::CalculateExternalFragmentation()
{
    size_t totalFreeSize = 0;
    size_t maxFreeBlockSize = 0;
    for (size_t index = 0; index < SEGREGATED_LIST_SIZE; index++) {
        auto *current = GetFirstBlock(index);
        while (current != nullptr) {
            auto blockSize = current->GetSize();
            totalFreeSize += blockSize;
            if (blockSize > maxFreeBlockSize) {
                maxFreeBlockSize = blockSize;
            }
            current = current->GetNextFree();
        }
    }

    if (totalFreeSize == 0) {
        return 0;
    }

    return 1.0 - static_cast<double>(maxFreeBlockSize) / totalFreeSize;
}

#undef LOG_FREELIST_ALLOCATOR

}  // namespace ark::mem

#endif  // PANDA_MEM_FREELIST_ALLOCATOR_INL_H
