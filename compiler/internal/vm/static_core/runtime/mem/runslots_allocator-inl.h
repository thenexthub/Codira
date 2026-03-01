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
#ifndef PANDA_RUNTIME_MEM_RUNSLOTS_ALLOCATOR_INL_H
#define PANDA_RUNTIME_MEM_RUNSLOTS_ALLOCATOR_INL_H

#include <securec.h>
#include "libarkbase/utils/asan_interface.h"
#include "runtime/mem/alloc_config.h"
#include "runtime/mem/object_helpers.h"
#include "runtime/mem/runslots_allocator.h"

namespace ark::mem {

// NOLINTNEXTLINE(cppcoreguidelines-macro-usage)
#define LOG_RUNSLOTS_ALLOCATOR(level) LOG(level, ALLOC) << "RunSlotsAllocator: "

template <typename AllocConfigT, typename LockConfigT>
inline RunSlotsAllocator<AllocConfigT, LockConfigT>::RunSlotsAllocator(MemStatsType *memStats, SpaceType typeAllocation)
    : typeAllocation_(typeAllocation), memStats_(memStats)
{
    LOG_RUNSLOTS_ALLOCATOR(DEBUG) << "Initializing RunSlotsAllocator";
    LOG_RUNSLOTS_ALLOCATOR(DEBUG) << "Initializing RunSlotsAllocator finished";
}

template <typename AllocConfigT, typename LockConfigT>
inline RunSlotsAllocator<AllocConfigT, LockConfigT>::~RunSlotsAllocator()
{
    LOG_RUNSLOTS_ALLOCATOR(DEBUG) << "Destroying RunSlotsAllocator";
    LOG_RUNSLOTS_ALLOCATOR(DEBUG) << "Destroying RunSlotsAllocator finished";
}

template <typename AllocConfigT, typename LockConfigT>
template <bool NEED_LOCK, bool DISABLE_USE_FREE_RUNSLOTS>
// CC-OFFNXT(G.FUN.01-CPP) Allocations perf critical, the change will create additional conditions and method calls that
// will degrade performance
// CC-OFFNXT(G.FUD.06) Allocations perf critical, the change will create additional conditions and method calls that
// will degrade performance
inline void *RunSlotsAllocator<AllocConfigT, LockConfigT>::Alloc(size_t size, Alignment align)
{
    using ListLock = typename LockConfigT::ListLock;
    LOG_RUNSLOTS_ALLOCATOR(DEBUG) << "Try to allocate " << size << " bytes of memory with align " << align;
    if (size == 0) {
        LOG_RUNSLOTS_ALLOCATOR(DEBUG) << "Failed to allocate - size of object is null";
        return nullptr;
    }
    // NOTE(aemelenko): Do smth more memory flexible with alignment
    size_t alignmentSize = GetAlignmentInBytes(align);
    if (alignmentSize > size) {
        LOG_RUNSLOTS_ALLOCATOR(DEBUG) << "Change size of allocation to " << alignmentSize
                                      << " bytes because of alignment";
        size = alignmentSize;
    }
    if (size > RunSlotsType::MaxSlotSize()) {
        LOG_RUNSLOTS_ALLOCATOR(DEBUG) << "Failed to allocate - size of object is too big";
        return nullptr;
    }
    size_t slotSizePowerOfTwo = RunSlotsType::ConvertToPowerOfTwoUnsafe(size);
    size_t arrayIndex = slotSizePowerOfTwo;
    const size_t runSlotSize = 1UL << slotSizePowerOfTwo;
    RunSlotsType *runslots = nullptr;
    bool usedFromFreedRunslotsList = false;
    {
        os::memory::LockHolder<ListLock, NEED_LOCK> listLock(*runslots_[arrayIndex].GetLock());
        runslots = runslots_[arrayIndex].PopFromHead();
    }
    if (runslots == nullptr) {
        LOG_RUNSLOTS_ALLOCATOR(DEBUG) << "We don't have free RunSlots for size " << runSlotSize
                                      << ". Try to get new one.";
        if (DISABLE_USE_FREE_RUNSLOTS) {
            return nullptr;
        }
        {
            os::memory::LockHolder<ListLock, NEED_LOCK> listLock(*freeRunslots_.GetLock());
            runslots = freeRunslots_.PopFromHead();
        }
        if (runslots != nullptr) {
            usedFromFreedRunslotsList = true;
            LOG_RUNSLOTS_ALLOCATOR(DEBUG) << "Get RunSlots from free list";
        } else {
            LOG_RUNSLOTS_ALLOCATOR(DEBUG)
                << "Failed to get new RunSlots from free list, try to allocate one from memory";
            runslots = CreateNewRunSlotsFromMemory(runSlotSize);
            if (runslots == nullptr) {
                LOG_RUNSLOTS_ALLOCATOR(DEBUG) << "Failed to allocate an object, couldn't create RunSlots";
                return nullptr;
            }
        }
    }
    void *allocatedMem = nullptr;
    {
        os::memory::LockHolder<typename LockConfigT::RunSlotsLock, NEED_LOCK> runslotsLock(*runslots->GetLock());
        if (usedFromFreedRunslotsList) {
            // NOTE(aemelenko): if we allocate and free two different size objects,
            //                  we will have a perf issue here. Maybe it is better to delete free_runslots_?
            if (runslots->GetSlotsSize() != runSlotSize) {
                runslots->Initialize(runSlotSize, runslots->GetPoolPointer(), false);
            }
        }
        LOG_RUNSLOTS_ALLOCATOR(DEBUG) << "Used runslots with addr " << std::hex << runslots;
        allocatedMem = static_cast<void *>(runslots->PopFreeSlot());
        if (allocatedMem == nullptr) {
            UNREACHABLE();
        }
        LOG_RUNSLOTS_ALLOCATOR(DEBUG) << "Allocate a memory at address " << std::hex << allocatedMem;
        if (!runslots->IsFull()) {
            os::memory::LockHolder<ListLock, NEED_LOCK> listLock(*runslots_[arrayIndex].GetLock());
            // We didn't take the last free slot from this RunSlots
            runslots_[arrayIndex].PushToTail(runslots);
        }
        ASAN_UNPOISON_MEMORY_REGION(allocatedMem, size);
        AllocConfigT::OnAlloc(runSlotSize, typeAllocation_, memStats_);
        AllocConfigT::MemoryInit(allocatedMem);
#ifdef PANDA_MEASURE_FRAGMENTATION
        // Atomic with relaxed order reason: order is not required
        allocatedBytes_.fetch_add(runSlotSize, std::memory_order_relaxed);
#endif
    }
    return allocatedMem;
}

template <typename AllocConfigT, typename LockConfigT>
inline void RunSlotsAllocator<AllocConfigT, LockConfigT>::Free(void *mem)
{
    FreeUnsafe<true>(mem);
}

template <typename AllocConfigT, typename LockConfigT>
// CC-OFFNXT(G.FUD.06) perf critical
inline void RunSlotsAllocator<AllocConfigT, LockConfigT>::ReleaseEmptyRunSlotsPagesUnsafe()
{
    // Iterate over free_runslots list:
    RunSlotsType *curFreeRunslots = nullptr;
    {
        os::memory::LockHolder listLock(*freeRunslots_.GetLock());
        curFreeRunslots = freeRunslots_.PopFromHead();
    }
    while (curFreeRunslots != nullptr) {
        memoryPool_.ReturnAndReleaseRunSlotsMemory(curFreeRunslots);

        {
            os::memory::LockHolder listLock(*freeRunslots_.GetLock());
            curFreeRunslots = freeRunslots_.PopFromHead();
        }
    }
}

template <typename AllocConfigT, typename LockConfigT>
// CC-OFFNXT(G.FUD.06) perf critical
inline bool RunSlotsAllocator<AllocConfigT, LockConfigT>::FreeUnsafeInternal(RunSlotsType *runslots, void *mem)
{
    bool needToAddToFreeList = false;
    // NOTE(aemelenko): Here can be a performance issue when we allocate/deallocate one object.
    const size_t runSlotSize = runslots->GetSlotsSize();
    size_t arrayIndex = RunSlotsType::ConvertToPowerOfTwoUnsafe(runSlotSize);
    bool runslotsWasFull = runslots->IsFull();
    runslots->PushFreeSlot(static_cast<FreeSlot *>(mem));
    /**
     * RunSlotsAllocator doesn't know this real size which we use in slot, so we record upper bound - size of the
     * slot.
     */
    AllocConfigT::OnFree(runSlotSize, typeAllocation_, memStats_);
#ifdef PANDA_MEASURE_FRAGMENTATION
    // Atomic with relaxed order reason: order is not required
    allocatedBytes_.fetch_sub(runSlotSize, std::memory_order_relaxed);
#endif
    ASAN_POISON_MEMORY_REGION(mem, runSlotSize);
    ASSERT(!(runslotsWasFull && runslots->IsEmpty()));  // Runslots has more that one slot inside.
    if (runslotsWasFull) {
        LOG_RUNSLOTS_ALLOCATOR(DEBUG) << "This RunSlots was full and now we must add it to the RunSlots list";

        os::memory::LockHolder listLock(*runslots_[arrayIndex].GetLock());
#if PANDA_ENABLE_SLOW_DEBUG
        ASSERT(!runslots_[arrayIndex].IsInThisList(runslots));
#endif
        runslots_[arrayIndex].PushToTail(runslots);
    } else if (runslots->IsEmpty()) {
        os::memory::LockHolder listLock(*runslots_[arrayIndex].GetLock());
        // Check that we may took this runslots from list on alloc
        // and waiting for lock
        if ((runslots->GetNextRunSlots() != nullptr) || (runslots->GetPrevRunSlots() != nullptr) ||
            (runslots_[arrayIndex].GetHead() == runslots)) {
            LOG_RUNSLOTS_ALLOCATOR(DEBUG)
                << "This RunSlots is empty. Pop it from runslots list and push it to free_runslots_";
            runslots_[arrayIndex].PopFromList(runslots);
            needToAddToFreeList = true;
        }
    }

    return needToAddToFreeList;
}

template <typename AllocConfigT, typename LockConfigT>
template <bool LOCK_RUN_SLOTS>
// CC-OFFNXT(G.FUD.06) perf critical
inline void RunSlotsAllocator<AllocConfigT, LockConfigT>::FreeUnsafe(void *mem)
{
    if (UNLIKELY(mem == nullptr)) {
        LOG_RUNSLOTS_ALLOCATOR(DEBUG) << "Try to free memory at invalid addr 0";
        return;
    }
    LOG_RUNSLOTS_ALLOCATOR(DEBUG) << "Try to free object at address " << std::hex << mem;
#ifndef NDEBUG
    if (!AllocatedByRunSlotsAllocatorUnsafe(mem)) {
        LOG_RUNSLOTS_ALLOCATOR(DEBUG) << "This object was not allocated by this allocator";
        return;
    }
#endif  // !NDEBUG

    // Now we 100% sure that this object was allocated by RunSlots allocator.
    // We can just do alignment for this address and get a pointer to RunSlots header
    uintptr_t runslotsAddr = (ToUintPtr(mem) >> RUNSLOTS_ALIGNMENT) << RUNSLOTS_ALIGNMENT;
    auto runslots = static_cast<RunSlotsType *>(ToVoidPtr(runslotsAddr));
    LOG_RUNSLOTS_ALLOCATOR(DEBUG) << "It is RunSlots with addr " << std::hex << static_cast<void *>(runslots);

    // NOLINTNEXTLINE(readability-braces-around-statements, bugprone-suspicious-semicolon)
    if constexpr (LOCK_RUN_SLOTS) {
        runslots->GetLock()->Lock();
    }

    bool needToAddToFreeList = FreeUnsafeInternal(runslots, mem);

    // NOLINTNEXTLINE(readability-braces-around-statements, bugprone-suspicious-semicolon)
    if constexpr (LOCK_RUN_SLOTS) {
        runslots->GetLock()->Unlock();
    }

    if (needToAddToFreeList) {
        os::memory::LockHolder listLock(*freeRunslots_.GetLock());
        freeRunslots_.PushToTail(runslots);
    }
    LOG_RUNSLOTS_ALLOCATOR(DEBUG) << "Freed object at address " << std::hex << mem;
}

template <typename AllocConfigT, typename LockConfigT>
inline void RunSlotsAllocator<AllocConfigT, LockConfigT>::Collect(const GCObjectVisitor &deathCheckerFn)
{
    LOG_RUNSLOTS_ALLOCATOR(DEBUG) << "Collecting for RunSlots allocator started";
    IterateOverObjects([this, &deathCheckerFn](ObjectHeader *objectHeader) {
        LOG_RUNSLOTS_ALLOCATOR(DEBUG) << "  iterate over " << std::hex << objectHeader;
        if (deathCheckerFn(objectHeader) == ObjectStatus::DEAD_OBJECT) {
            LOG(DEBUG, GC) << "DELETE OBJECT " << GetDebugInfoAboutObject(objectHeader);
            FreeUnsafe<false>(objectHeader);
        }
    });
    LOG_RUNSLOTS_ALLOCATOR(DEBUG) << "Collecting for RunSlots allocator finished";
}

template <typename AllocConfigT, typename LockConfigT>
template <typename ObjectVisitor>
void RunSlotsAllocator<AllocConfigT, LockConfigT>::IterateOverObjects(const ObjectVisitor &objectVisitor)
{
    LOG_RUNSLOTS_ALLOCATOR(DEBUG) << "Iteration over objects started";
    memoryPool_.IterateOverObjects(objectVisitor);
    LOG_RUNSLOTS_ALLOCATOR(DEBUG) << "Iteration over objects finished";
}

template <typename AllocConfigT, typename LockConfigT>
bool RunSlotsAllocator<AllocConfigT, LockConfigT>::AllocatedByRunSlotsAllocator(void *object)
{
    return AllocatedByRunSlotsAllocatorUnsafe(object);
}

template <typename AllocConfigT, typename LockConfigT>
bool RunSlotsAllocator<AllocConfigT, LockConfigT>::AllocatedByRunSlotsAllocatorUnsafe(void *object)
{
    // NOTE(aemelenko): Add more complex and optimized solution for this method
    return memoryPool_.IsInMemPools(object);
}

template <typename AllocConfigT, typename LockConfigT>
template <bool NEED_LOCK>
// CC-OFFNXT(G.FMT.07) project code style
inline typename RunSlotsAllocator<AllocConfigT, LockConfigT>::RunSlotsType *
RunSlotsAllocator<AllocConfigT, LockConfigT>::CreateNewRunSlotsFromMemory(size_t slotsSize)
{
    RunSlotsType *runslots = memoryPool_.template GetNewRunSlots<NEED_LOCK>(slotsSize);
    if (runslots != nullptr) {
        LOG_RUNSLOTS_ALLOCATOR(DEBUG) << "Take " << RUNSLOTS_SIZE << " bytes of memory for new RunSlots instance from "
                                      << std::hex << runslots;
        return runslots;
    }
    LOG_RUNSLOTS_ALLOCATOR(DEBUG) << "There is no free memory for RunSlots";
    return runslots;
}

template <typename AllocConfigT, typename LockConfigT>
// CC-OFFNXT(G.FUD.06) perf critical
inline bool RunSlotsAllocator<AllocConfigT, LockConfigT>::AddMemoryPool(void *mem, size_t size)
{
    LOG_RUNSLOTS_ALLOCATOR(DEBUG) << "Get new memory pool with size " << size << " bytes, at addr " << std::hex << mem;
    // Try to add this memory to the memory_pool_
    if (mem == nullptr) {
        LOG_RUNSLOTS_ALLOCATOR(DEBUG) << "Failed to add memory, the memory is nullptr";
        return false;
    }
    if (size > MIN_POOL_SIZE) {
        // NOTE(aemelenko): The size of the pool is fixed by now,
        // because it is requested for correct freed_runslots_bitmap_
        // workflow. Fix it in #4018
        LOG_RUNSLOTS_ALLOCATOR(DEBUG)
            << "Can't add new memory pool to this allocator because the memory size is equal to " << MIN_POOL_SIZE;
        return false;
    }
    if (!memoryPool_.AddNewMemoryPool(mem, size)) {
        LOG_RUNSLOTS_ALLOCATOR(DEBUG)
            << "Can't add new memory pool to this allocator. Maybe we already added too much memory pools.";
        return false;
    }
    return true;
}

template <typename AllocConfigT, typename LockConfigT>
template <typename MemVisitor>
void RunSlotsAllocator<AllocConfigT, LockConfigT>::VisitAndRemoveAllPools(const MemVisitor &memVisitor)
{
    // We call this method and return pools to the system.
    // Therefore, delete all objects to clear all external dependences
    LOG_RUNSLOTS_ALLOCATOR(DEBUG) << "Clear all objects inside the allocator";
    memoryPool_.VisitAndRemoveAllPools(memVisitor);
}

template <typename AllocConfigT, typename LockConfigT>
template <typename MemVisitor>
void RunSlotsAllocator<AllocConfigT, LockConfigT>::VisitAndRemoveFreePools(const MemVisitor &memVisitor)
{
    ReleaseEmptyRunSlotsPagesUnsafe();
    // We need to remove RunSlots from RunSlotsList
    // All of them must be inside free_runslots_ list.
    memoryPool_.VisitAndRemoveFreePools(memVisitor);
}

template <typename AllocConfigT, typename LockConfigT>
template <typename MemVisitor>
void RunSlotsAllocator<AllocConfigT, LockConfigT>::IterateOverObjectsInRange(const MemVisitor &memVisitor,
                                                                             void *leftBorder, void *rightBorder)
{
    LOG_RUNSLOTS_ALLOCATOR(DEBUG) << "IterateOverObjectsInRange for range [" << std::hex << leftBorder << ", "
                                  << rightBorder << "]";
    ASSERT(ToUintPtr(rightBorder) >= ToUintPtr(leftBorder));
    if (!AllocatedByRunSlotsAllocatorUnsafe(leftBorder)) {
        LOG_RUNSLOTS_ALLOCATOR(DEBUG) << "This memory range is not covered by this allocator";
        return;
    }
    // NOTE(aemelenko): These are temporary asserts because we can't do anything
    // if the range crosses different allocators memory pools
    ASSERT(ToUintPtr(rightBorder) - ToUintPtr(leftBorder) == (CrossingMapSingleton::GetCrossingMapGranularity() - 1U));
    ASSERT((ToUintPtr(rightBorder) & (~(CrossingMapSingleton::GetCrossingMapGranularity() - 1U))) ==
           (ToUintPtr(leftBorder) & (~(CrossingMapSingleton::GetCrossingMapGranularity() - 1U))));
    // Now we 100% sure that this left_border was allocated by RunSlots allocator.
    // We can just do alignment for this address and get a pointer to RunSlots header
    uintptr_t runslotsAddr = (ToUintPtr(leftBorder) >> RUNSLOTS_ALIGNMENT) << RUNSLOTS_ALIGNMENT;
    while (runslotsAddr < ToUintPtr(rightBorder)) {
        auto runslots = static_cast<RunSlotsType *>(ToVoidPtr(runslotsAddr));
        os::memory::LockHolder runslotsLock(*runslots->GetLock());
        LOG_RUNSLOTS_ALLOCATOR(DEBUG) << "IterateOverObjectsInRange, It is RunSlots with addr " << std::hex
                                      << static_cast<void *>(runslots);
        runslots->IterateOverOccupiedSlots(memVisitor);
        runslotsAddr += RUNSLOTS_SIZE;
    }
    LOG_RUNSLOTS_ALLOCATOR(DEBUG) << "IterateOverObjectsInRange finished";
}

template <typename AllocConfigT, typename LockConfigT>
size_t RunSlotsAllocator<AllocConfigT, LockConfigT>::VerifyAllocator()
{
    size_t failCnt = 0;
    for (size_t i = 0; i < SLOTS_SIZES_VARIANTS; i++) {
        RunSlotsType *runslots = nullptr;
        {
            os::memory::LockHolder listLock(*runslots_[i].GetLock());
            runslots = runslots_[i].GetHead();
        }
        if (runslots != nullptr) {
            os::memory::LockHolder runslotsLock(*runslots->GetLock());
            failCnt += runslots->VerifyRun();
        }
    }
    return failCnt;
}

template <typename AllocConfigT, typename LockConfigT>
bool RunSlotsAllocator<AllocConfigT, LockConfigT>::ContainObject(const ObjectHeader *obj)
{
    return AllocatedByRunSlotsAllocatorUnsafe(const_cast<ObjectHeader *>(obj));
}

template <typename AllocConfigT, typename LockConfigT>
bool RunSlotsAllocator<AllocConfigT, LockConfigT>::IsLive(const ObjectHeader *obj)
{
    ASSERT(ContainObject(obj));
    uintptr_t runslotsAddr = ToUintPtr(obj) >> RUNSLOTS_ALIGNMENT << RUNSLOTS_ALIGNMENT;
    auto run = static_cast<RunSlotsType *>(ToVoidPtr(runslotsAddr));
    if (run->IsEmpty()) {
        return false;
    }
    return run->IsLive(obj);
}

template <typename AllocConfigT, typename LockConfigT>
void RunSlotsAllocator<AllocConfigT, LockConfigT>::TrimUnsafe()
{
    // release page in free runslots list
    auto head = freeRunslots_.GetHead();
    while (head != nullptr) {
        auto next = head->GetNextRunSlots();
        os::mem::ReleasePages(ToUintPtr(head), ToUintPtr(head) + RUNSLOTS_SIZE);
        head = next;
    }

    memoryPool_.VisitAllPoolsWithOccupiedSize([](void *mem, size_t usedSize, size_t size) {
        uintptr_t start = AlignUp(ToUintPtr(mem) + usedSize, ark::os::mem::GetPageSize());
        uintptr_t end = ToUintPtr(mem) + size;
        if (end >= start + ark::os::mem::GetPageSize()) {
            os::mem::ReleasePages(start, end);
        }
    });
}

template <typename AllocConfigT, typename LockConfigT>
// CC-OFFNXT(G.FUD.06) perf critical
inline void RunSlotsAllocator<AllocConfigT, LockConfigT>::RunSlotsList::PushToTail(RunSlotsType *runslots)
{
    LOG_RUNSLOTS_ALLOCATOR(DEBUG) << "Push to tail RunSlots at addr " << std::hex << static_cast<void *>(runslots);
    LOG_RUNSLOTS_ALLOCATOR(DEBUG) << "     tail_ " << std::hex << tail_;
    if (tail_ == nullptr) {
        LOG_RUNSLOTS_ALLOCATOR(DEBUG) << "     List was empty, setup head_ and tail_";
        // this means that head_ == nullptr too
        head_ = runslots;
        tail_ = runslots;
        return;
    }
    tail_->SetNextRunSlots(runslots);
    runslots->SetPrevRunSlots(tail_);
    tail_ = runslots;
    tail_->SetNextRunSlots(nullptr);
}

template <typename AllocConfigT, typename LockConfigT>
// CC-OFFNXT(G.FUD.06) perf critical
// CC-OFFNXT(G.FMT.07) project code style
inline typename RunSlotsAllocator<AllocConfigT, LockConfigT>::RunSlotsType *
RunSlotsAllocator<AllocConfigT, LockConfigT>::RunSlotsList::PopFromHead()
{
    LOG_RUNSLOTS_ALLOCATOR(DEBUG) << "PopFromHead";
    if (UNLIKELY(head_ == nullptr)) {
        LOG_RUNSLOTS_ALLOCATOR(DEBUG) << "      List is empty, nothing to pop";
        return nullptr;
    }
    RunSlotsType *headRunslots = head_;
    LOG_RUNSLOTS_ALLOCATOR(DEBUG) << "     popped from head RunSlots " << std::hex << headRunslots;
    head_ = headRunslots->GetNextRunSlots();
    if (head_ == nullptr) {
        LOG_RUNSLOTS_ALLOCATOR(DEBUG) << "     Now list is empty";
        // We pop the last element in the list
        tail_ = nullptr;
    } else {
        head_->SetPrevRunSlots(nullptr);
    }
    headRunslots->SetNextRunSlots(nullptr);
    return headRunslots;
}

template <typename AllocConfigT, typename LockConfigT>
// CC-OFFNXT(G.FUD.06) perf critical
// CC-OFFNXT(G.FMT.07) project code style
inline typename RunSlotsAllocator<AllocConfigT, LockConfigT>::RunSlotsType *
RunSlotsAllocator<AllocConfigT, LockConfigT>::RunSlotsList::PopFromTail()
{
    LOG_RUNSLOTS_ALLOCATOR(DEBUG) << "PopFromTail";
    if (UNLIKELY(tail_ == nullptr)) {
        LOG_RUNSLOTS_ALLOCATOR(DEBUG) << "      List is empty, nothing to pop";
        return nullptr;
    }
    RunSlotsType *tailRunslots = tail_;
    LOG_RUNSLOTS_ALLOCATOR(DEBUG) << "     popped from tail RunSlots " << std::hex << tailRunslots;
    tail_ = tailRunslots->GetPrevRunSlots();
    if (tail_ == nullptr) {
        LOG_RUNSLOTS_ALLOCATOR(DEBUG) << "     Now list is empty";
        // We pop the last element in the list
        head_ = nullptr;
    } else {
        tail_->SetNextRunSlots(nullptr);
    }
    tailRunslots->SetPrevRunSlots(nullptr);
    return tailRunslots;
}

template <typename AllocConfigT, typename LockConfigT>
// CC-OFFNXT(G.FUD.06) perf critical
inline void RunSlotsAllocator<AllocConfigT, LockConfigT>::RunSlotsList::PopFromList(RunSlotsType *runslots)
{
    LOG_RUNSLOTS_ALLOCATOR(DEBUG) << "PopFromList RunSlots with addr " << std::hex << runslots;
    LOG_RUNSLOTS_ALLOCATOR(DEBUG) << "head_ = " << std::hex << head_;
    LOG_RUNSLOTS_ALLOCATOR(DEBUG) << "tail_ = " << std::hex << tail_;

    if (runslots == head_) {
        LOG_RUNSLOTS_ALLOCATOR(DEBUG) << "It is RunSlots from the head.";
        PopFromHead();
        return;
    }
    if (runslots == tail_) {
        LOG_RUNSLOTS_ALLOCATOR(DEBUG) << "It is RunSlots from the tail.";
        PopFromTail();
        return;
    }
    LOG_RUNSLOTS_ALLOCATOR(DEBUG) << "Remove RunSlots from the list.";
    ASSERT(runslots != nullptr);
    RunSlotsType *nextRunslots = runslots->GetNextRunSlots();
    RunSlotsType *previousRunslots = runslots->GetPrevRunSlots();
    ASSERT(nextRunslots != nullptr);
    ASSERT(previousRunslots != nullptr);

    nextRunslots->SetPrevRunSlots(previousRunslots);
    previousRunslots->SetNextRunSlots(nextRunslots);
    runslots->SetNextRunSlots(nullptr);
    runslots->SetPrevRunSlots(nullptr);
}

template <typename AllocConfigT, typename LockConfigT>
inline RunSlotsAllocator<AllocConfigT, LockConfigT>::MemPoolManager::MemPoolManager()
{
    occupiedTail_ = nullptr;
    freeTail_ = nullptr;
    partiallyOccupiedHead_ = nullptr;
}

template <typename AllocConfigT, typename LockConfigT>
template <bool NEED_LOCK>
// CC-OFFNXT(G.FUD.06) perf critical
// CC-OFFNXT(G.FMT.07) project code style
inline typename RunSlotsAllocator<AllocConfigT, LockConfigT>::RunSlotsType *
RunSlotsAllocator<AllocConfigT, LockConfigT>::MemPoolManager::GetNewRunSlots(size_t slotsSize)
{
    os::memory::WriteLockHolder<typename LockConfigT::PoolLock, NEED_LOCK> wlock(lock_);
    RunSlotsType *newRunslots = nullptr;
    if (partiallyOccupiedHead_ != nullptr) {
        newRunslots = partiallyOccupiedHead_->GetMemoryForRunSlots(slotsSize);
        ASSERT(newRunslots != nullptr);
        if (UNLIKELY(!partiallyOccupiedHead_->HasMemoryForRunSlots())) {
            partiallyOccupiedHead_ = partiallyOccupiedHead_->GetNext();
            ASSERT((partiallyOccupiedHead_ == nullptr) || (partiallyOccupiedHead_->HasMemoryForRunSlots()));
        }
    } else if (freeTail_ != nullptr) {
        LOG_RUNSLOTS_ALLOCATOR(DEBUG)
            << "MemPoolManager: occupied_tail_ doesn't have memory for RunSlots, get new pool from free pools";
        PoolListElement *freeElement = freeTail_;
        freeTail_ = freeTail_->GetPrev();

        freeElement->PopFromList();
        freeElement->SetPrev(occupiedTail_);

        if (occupiedTail_ != nullptr) {
            ASSERT(occupiedTail_->GetNext() == nullptr);
            occupiedTail_->SetNext(freeElement);
        }
        occupiedTail_ = freeElement;

        if (partiallyOccupiedHead_ == nullptr) {
            partiallyOccupiedHead_ = occupiedTail_;
            ASSERT(partiallyOccupiedHead_->HasMemoryForRunSlots());
        }

        ASSERT(occupiedTail_->GetNext() == nullptr);
        newRunslots = occupiedTail_->GetMemoryForRunSlots(slotsSize);
        ASSERT(newRunslots != nullptr);
    }
    return newRunslots;
}

template <typename AllocConfigT, typename LockConfigT>
inline bool RunSlotsAllocator<AllocConfigT, LockConfigT>::MemPoolManager::AddNewMemoryPool(void *mem, size_t size)
{
    os::memory::WriteLockHolder wlock(lock_);
    PoolListElement *newPool = PoolListElement::Create(mem, size, freeTail_);
    if (freeTail_ != nullptr) {
        ASSERT(freeTail_->GetNext() == nullptr);
        freeTail_->SetNext(newPool);
    }
    freeTail_ = newPool;
    ASAN_POISON_MEMORY_REGION(mem, size);
    // To not unpoison it every time at access.
    ASAN_UNPOISON_MEMORY_REGION(mem, sizeof(PoolListElement));
    return true;
}

template <typename AllocConfigT, typename LockConfigT>
// CC-OFFNXT(G.FUD.06) perf critical
inline void RunSlotsAllocator<AllocConfigT, LockConfigT>::MemPoolManager::ReturnAndReleaseRunSlotsMemory(
    RunSlotsType *runslots)
{
    os::memory::WriteLockHolder wlock(lock_);
    auto pool = static_cast<PoolListElement *>(ToVoidPtr(runslots->GetPoolPointer()));
    if (!pool->HasMemoryForRunSlots()) {
        ASSERT(partiallyOccupiedHead_ != pool);
        // We should add move this pool to the end of a occupied list
        if (pool != occupiedTail_) {
            pool->PopFromList();
            pool->SetPrev(occupiedTail_);
            if (UNLIKELY(occupiedTail_ == nullptr)) {
                UNREACHABLE();
            }
            occupiedTail_->SetNext(pool);
            occupiedTail_ = pool;
        } else {
            ASSERT(partiallyOccupiedHead_ == nullptr);
        }
        if (partiallyOccupiedHead_ == nullptr) {
            partiallyOccupiedHead_ = occupiedTail_;
        }
    }

    pool->AddFreedRunSlots(runslots);
    ASSERT(partiallyOccupiedHead_->HasMemoryForRunSlots());

    // Start address from which we can release pages
    uintptr_t startAddr = AlignUp(ToUintPtr(runslots), os::mem::GetPageSize());
    // End address before which we can release pages
    uintptr_t endAddr = os::mem::AlignDownToPageSize(ToUintPtr(runslots) + RUNSLOTS_SIZE);
    if (startAddr < endAddr) {
        os::mem::ReleasePages(startAddr, endAddr);
    }
}

template <typename AllocConfigT, typename LockConfigT>
bool RunSlotsAllocator<AllocConfigT, LockConfigT>::MemPoolManager::IsInMemPools(void *object)
{
    os::memory::ReadLockHolder rlock(lock_);
    PoolListElement *current = occupiedTail_;
    while (current != nullptr) {
        if (current->IsInUsedMemory(object)) {
            return true;
        }
        current = current->GetPrev();
    }
    return false;
}

template <typename AllocConfigT, typename LockConfigT>
template <typename ObjectVisitor>
void RunSlotsAllocator<AllocConfigT, LockConfigT>::MemPoolManager::IterateOverObjects(
    const ObjectVisitor &objectVisitor)
{
    PoolListElement *currentPool = nullptr;
    {
        os::memory::ReadLockHolder rlock(lock_);
        currentPool = occupiedTail_;
    }
    while (currentPool != nullptr) {
        currentPool->IterateOverRunSlots([&currentPool, &objectVisitor](RunSlotsType *runslots) {
            os::memory::LockHolder runslotsLock(*runslots->GetLock());
            UNUSED_VAR(currentPool);  // For release build
            ASSERT(runslots->GetPoolPointer() == ToUintPtr(currentPool));
            runslots->IterateOverOccupiedSlots(objectVisitor);
            return true;
        });
        {
            os::memory::ReadLockHolder rlock(lock_);
            currentPool = currentPool->GetPrev();
        }
    }
}

template <typename AllocConfigT, typename LockConfigT>
template <typename MemVisitor>
void RunSlotsAllocator<AllocConfigT, LockConfigT>::MemPoolManager::VisitAndRemoveAllPools(const MemVisitor &memVisitor)
{
    os::memory::WriteLockHolder wlock(lock_);
    PoolListElement *currentPool = occupiedTail_;
    while (currentPool != nullptr) {
        // Use tmp in case if visitor with side effects
        PoolListElement *tmp = currentPool->GetPrev();
        memVisitor(currentPool->GetPoolMemory(), currentPool->GetSize());
        currentPool = tmp;
    }
    occupiedTail_ = nullptr;
    currentPool = freeTail_;
    while (currentPool != nullptr) {
        // Use tmp in case if visitor with side effects
        PoolListElement *tmp = currentPool->GetPrev();
        memVisitor(currentPool->GetPoolMemory(), currentPool->GetSize());
        currentPool = tmp;
    }
    freeTail_ = nullptr;
}

template <typename AllocConfigT, typename LockConfigT>
template <typename MemVisitor>
void RunSlotsAllocator<AllocConfigT, LockConfigT>::MemPoolManager::VisitAllPoolsWithOccupiedSize(
    const MemVisitor &memVisitor)
{
    os::memory::WriteLockHolder wlock(lock_);
    PoolListElement *currentPool = occupiedTail_;
    while (currentPool != nullptr) {
        // Use tmp in case if visitor with side effects
        PoolListElement *tmp = currentPool->GetPrev();
        memVisitor(currentPool->GetPoolMemory(), currentPool->GetOccupiedSize(), currentPool->GetSize());
        currentPool = tmp;
    }
}

template <typename AllocConfigT, typename LockConfigT>
template <typename MemVisitor>
void RunSlotsAllocator<AllocConfigT, LockConfigT>::MemPoolManager::VisitAndRemoveFreePools(const MemVisitor &memVisitor)
{
    os::memory::WriteLockHolder wlock(lock_);
    LOG_RUNSLOTS_ALLOCATOR(DEBUG) << "VisitAllFreePools inside RunSlotsAllocator";
    // First, iterate over totally free pools:
    PoolListElement *currentPool = freeTail_;
    while (currentPool != nullptr) {
        LOG_RUNSLOTS_ALLOCATOR(DEBUG) << "VisitAllFreePools: Visit free pool with addr " << std::hex
                                      << currentPool->GetPoolMemory() << " and size " << std::dec
                                      << currentPool->GetSize();
        // Use tmp in case if visitor with side effects
        PoolListElement *tmp = currentPool->GetPrev();
        memVisitor(currentPool->GetPoolMemory(), currentPool->GetSize());
        currentPool = tmp;
    }
    freeTail_ = nullptr;
    // Second, try to find free pool in occupied:
    currentPool = occupiedTail_;
    while (currentPool != nullptr) {
        // Use tmp in case if visitor with side effects
        PoolListElement *tmp = currentPool->GetPrev();
        if (!currentPool->HasUsedMemory()) {
            LOG_RUNSLOTS_ALLOCATOR(DEBUG)
                << "VisitAllFreePools: Visit occupied pool with addr " << std::hex << currentPool->GetPoolMemory()
                << " and size " << std::dec << currentPool->GetSize();
            // This Pool doesn't have any occupied memory in RunSlots
            // Therefore, we can free it
            if (occupiedTail_ == currentPool) {
                LOG_RUNSLOTS_ALLOCATOR(DEBUG) << "VisitAllFreePools: Update occupied_tail_";
                occupiedTail_ = currentPool->GetPrev();
            }
            if (currentPool == partiallyOccupiedHead_) {
                partiallyOccupiedHead_ = partiallyOccupiedHead_->GetNext();
                ASSERT((partiallyOccupiedHead_ == nullptr) || (partiallyOccupiedHead_->HasMemoryForRunSlots()));
            }
            currentPool->PopFromList();
            memVisitor(currentPool->GetPoolMemory(), currentPool->GetSize());
        }
        currentPool = tmp;
    }
}

// NOLINTNEXTLINE(cppcoreguidelines-pro-type-member-init)
template <typename AllocConfigT, typename LockConfigT>
inline RunSlotsAllocator<AllocConfigT, LockConfigT>::MemPoolManager::PoolListElement::PoolListElement()
{
    startMem_ = 0;
    poolMem_ = 0;
    size_ = 0;
    freePtr_ = 0;
    prevPool_ = nullptr;
    nextPool_ = nullptr;
    freededRunslotsCount_ = 0;
    memset_s(storageForBitmap_.data(), sizeof(BitMapStorageType), 0, sizeof(BitMapStorageType));
}

template <typename AllocConfigT, typename LockConfigT>
void RunSlotsAllocator<AllocConfigT, LockConfigT>::MemPoolManager::PoolListElement::PopFromList()
{
    if (nextPool_ != nullptr) {
        nextPool_->SetPrev(prevPool_);
    }
    if (prevPool_ != nullptr) {
        prevPool_->SetNext(nextPool_);
    }
    nextPool_ = nullptr;
    prevPool_ = nullptr;
}

template <typename AllocConfigT, typename LockConfigT>
uintptr_t RunSlotsAllocator<AllocConfigT, LockConfigT>::MemPoolManager::PoolListElement::GetFirstRunSlotsBlock(
    uintptr_t mem)
{
    return AlignUp(mem, 1UL << RUNSLOTS_ALIGNMENT);
}

template <typename AllocConfigT, typename LockConfigT>
// CC-OFFNXT(G.FUD.06) perf critical
inline void RunSlotsAllocator<AllocConfigT, LockConfigT>::MemPoolManager::PoolListElement::Initialize(
    void *poolMem, uintptr_t unoccupiedMem, size_t size, PoolListElement *prev)
{
    startMem_ = unoccupiedMem;
    poolMem_ = ToUintPtr(poolMem);
    size_ = size;
    // Atomic with release order reason: data race with free_ptr_ with dependecies on writes before the store which
    // should become visible acquire
    freePtr_.store(GetFirstRunSlotsBlock(startMem_), std::memory_order_release);
    prevPool_ = prev;
    nextPool_ = nullptr;
    freededRunslotsCount_ = 0;
    freedRunslotsBitmap_.ReInitializeMemoryRange(poolMem);
    ASSERT(freedRunslotsBitmap_.FindFirstMarkedChunks() == nullptr);
    // Atomic with acquire order reason: data race with free_ptr_ with dependecies on reads after the load which should
    // become visible
    LOG_RUNSLOTS_ALLOCATOR(DEBUG) << "PoolMemory: first free RunSlots block = " << std::hex
                                  << freePtr_.load(std::memory_order_acquire);
}

template <typename AllocConfigT, typename LockConfigT>
// CC-OFFNXT(G.FUD.06) perf critical
// CC-OFFNXT(G.FMT.07) project code style
inline typename RunSlotsAllocator<AllocConfigT, LockConfigT>::RunSlotsType *
RunSlotsAllocator<AllocConfigT, LockConfigT>::MemPoolManager::PoolListElement::GetMemoryForRunSlots(size_t slotsSize)
{
    if (!HasMemoryForRunSlots()) {
        LOG_RUNSLOTS_ALLOCATOR(DEBUG) << "PoolMemory: There is no free memory for RunSlots";
        return nullptr;
    }
    RunSlotsType *runslots = GetFreedRunSlots(slotsSize);
    if (runslots == nullptr) {
        // Atomic with acquire order reason: data race with free_ptr_ with dependecies on reads after the load which
        // should become visible
        uintptr_t oldMem = freePtr_.load(std::memory_order_acquire);
        ASSERT(poolMem_ + size_ >= oldMem + RUNSLOTS_SIZE);

        // Initialize it firstly before updating free ptr
        // because it will be visible outside after that.
        runslots = static_cast<RunSlotsType *>(ToVoidPtr(oldMem));
        runslots->Initialize(slotsSize, ToUintPtr(this), true);
        // Atomic with acq_rel order reason: data race with free_ptr_ with dependecies on reads after the load and on
        // writes before the store
        freePtr_.fetch_add(RUNSLOTS_SIZE, std::memory_order_acq_rel);
        // Atomic with acquire order reason: data race with free_ptr_ with dependecies on reads after the load which
        // should become visible
        ASSERT(freePtr_.load(std::memory_order_acquire) == (oldMem + RUNSLOTS_SIZE));
        LOG_RUNSLOTS_ALLOCATOR(DEBUG) << "PoolMemory: Took memory for RunSlots from addr " << std::hex
                                      << ToVoidPtr(oldMem)
                                      // Atomic with acquire order reason: data race with free_ptr_
                                      << ". New first free RunSlots block = "
                                      << ToVoidPtr(freePtr_.load(std::memory_order_acquire));
    }
    ASSERT(runslots != nullptr);
    return runslots;
}

template <typename AllocConfigT, typename LockConfigT>
template <typename RunSlotsVisitor>
void RunSlotsAllocator<AllocConfigT, LockConfigT>::MemPoolManager::PoolListElement::IterateOverRunSlots(
    const RunSlotsVisitor &runslotsVisitor)
{
    LOG_RUNSLOTS_ALLOCATOR(DEBUG) << "Iterating over runslots inside pool with address" << std::hex << poolMem_
                                  << " with size " << std::dec << size_ << " bytes";
    uintptr_t currentRunslot = GetFirstRunSlotsBlock(startMem_);
    // Atomic with acquire order reason: data race with free_ptr_ with dependecies on reads after the load which should
    // become visible
    uintptr_t lastRunslot = freePtr_.load(std::memory_order_acquire);
    while (currentRunslot < lastRunslot) {
        ASSERT(startMem_ <= currentRunslot);
        if (!freedRunslotsBitmap_.AtomicTest(ToVoidPtr(currentRunslot))) {
            auto curRs = static_cast<RunSlotsType *>(ToVoidPtr(currentRunslot));
            LOG_RUNSLOTS_ALLOCATOR(DEBUG) << "Iterating. Process RunSlots " << std::hex << curRs;
            if (!runslotsVisitor(curRs)) {
                return;
            }
        }
        currentRunslot += RUNSLOTS_SIZE;
    }
    LOG_RUNSLOTS_ALLOCATOR(DEBUG) << "Iterating runslots inside this pool finished";
}

template <typename AllocConfigT, typename LockConfigT>
bool RunSlotsAllocator<AllocConfigT, LockConfigT>::MemPoolManager::PoolListElement::HasUsedMemory()
{
    uintptr_t currentRunslot = GetFirstRunSlotsBlock(startMem_);
    // Atomic with acquire order reason: data race with free_ptr_ with dependecies on reads after the load which should
    // become visible
    uintptr_t lastRunslot = freePtr_.load(std::memory_order_acquire);
    while (currentRunslot < lastRunslot) {
        ASSERT(startMem_ <= currentRunslot);
        if (!freedRunslotsBitmap_.AtomicTest(ToVoidPtr(currentRunslot))) {
            // We have runslots instance which is in use somewhere.
            return true;
        }
        currentRunslot += RUNSLOTS_SIZE;
    }
    return false;
}

template <typename AllocConfigT, typename LockConfigT>
size_t RunSlotsAllocator<AllocConfigT, LockConfigT>::MemPoolManager::PoolListElement::GetOccupiedSize()
{
    if (!IsInitialized()) {
        return 0;
    }
    // Atomic with acquire order reason: data race with free_ptr_ with dependecies on reads after the load which should
    // become visible
    return freePtr_.load(std::memory_order_acquire) - poolMem_;
}

template <typename AllocConfigT, typename LockConfigT>
bool RunSlotsAllocator<AllocConfigT, LockConfigT>::MemPoolManager::PoolListElement::IsInUsedMemory(void *object)
{
    uintptr_t memPointer = startMem_;
    ASSERT(!((ToUintPtr(object) < GetFirstRunSlotsBlock(memPointer)) && (ToUintPtr(object) >= memPointer)));
    // Atomic with acquire order reason: data race with free_ptr_ with dependecies on reads after the load which should
    // become visible
    bool isInAllocatedMemory = (ToUintPtr(object) < freePtr_.load(std::memory_order_acquire)) &&
                               (ToUintPtr(object) >= GetFirstRunSlotsBlock(memPointer));
    return isInAllocatedMemory && !IsInFreedRunSlots(object);
}

template <typename AllocConfigT, typename LockConfigT>
// CC-OFFNXT(G.FMT.07) project code style
typename RunSlotsAllocator<AllocConfigT, LockConfigT>::RunSlotsType *
RunSlotsAllocator<AllocConfigT, LockConfigT>::MemPoolManager::PoolListElement::GetFreedRunSlots(size_t slotsSize)
{
    auto slots = static_cast<RunSlotsType *>(freedRunslotsBitmap_.FindFirstMarkedChunks());
    if (slots == nullptr) {
        ASSERT(freededRunslotsCount_ == 0);
        return nullptr;
    }

    // Initialize it firstly before updating bitmap
    // because it will be visible outside after that.
    slots->Initialize(slotsSize, ToUintPtr(this), true);

    ASSERT(freededRunslotsCount_ > 0);
    [[maybe_unused]] bool oldVal = freedRunslotsBitmap_.AtomicTestAndClear(slots);
    ASSERT(oldVal);
    freededRunslotsCount_--;

    return slots;
}

template <typename AllocConfigT, typename LockConfigT>
bool RunSlotsAllocator<AllocConfigT, LockConfigT>::MemPoolManager::PoolListElement::HasMemoryForRunSlots()
{
    if (!IsInitialized()) {
        return false;
    }
    // Atomic with acquire order reason: data race with free_ptr_ with dependecies on reads after the load which should
    // become visible
    bool hasFreeMemory = (freePtr_.load(std::memory_order_acquire) + RUNSLOTS_SIZE) <= (poolMem_ + size_);
    bool hasFreedRunslots = (freededRunslotsCount_ > 0);
    ASSERT(hasFreedRunslots == (freedRunslotsBitmap_.FindFirstMarkedChunks() != nullptr));
    return hasFreeMemory || hasFreedRunslots;
}

#undef LOG_RUNSLOTS_ALLOCATOR

}  // namespace ark::mem
#endif  // PANDA_RUNTIME_MEM_RUNSLOTS_ALLOCATOR_INL_H
