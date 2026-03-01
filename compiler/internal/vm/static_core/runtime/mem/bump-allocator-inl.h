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
#ifndef RUNTIME_MEM_PANDA_BUMP_ALLOCATOR_INL_H
#define RUNTIME_MEM_PANDA_BUMP_ALLOCATOR_INL_H

#include "libarkbase/utils/logger.h"
#include "runtime/include/mem/allocator.h"
#include "runtime/mem/bump-allocator.h"
#include "runtime/mem/object_helpers.h"
#include "runtime/mem/alloc_config.h"

namespace ark::mem {

// NOLINTNEXTLINE(cppcoreguidelines-macro-usage)
#define LOG_BUMP_ALLOCATOR(level) LOG(level, ALLOC) << "BumpPointerAllocator: "

template <typename AllocConfigT, typename LockConfigT, bool USE_TLABS>
BumpPointerAllocator<AllocConfigT, LockConfigT, USE_TLABS>::BumpPointerAllocator(Pool pool, SpaceType typeAllocation,
                                                                                 MemStatsType *memStats,
                                                                                 size_t tlabsMaxCount)
    : arena_(pool.GetSize(), pool.GetMem()),
      tlabManager_(tlabsMaxCount),
      typeAllocation_(typeAllocation),
      memStats_(memStats)
{
    LOG_BUMP_ALLOCATOR(DEBUG) << "Initializing of BumpPointerAllocator";
    AllocConfigT::InitializeCrossingMapForMemory(pool.GetMem(), arena_.GetSize());
    LOG_BUMP_ALLOCATOR(DEBUG) << "Initializing of BumpPointerAllocator finished";
    ASSERT(USE_TLABS ? tlabsMaxCount > 0 : tlabsMaxCount == 0);
}

template <typename AllocConfigT, typename LockConfigT, bool USE_TLABS>
BumpPointerAllocator<AllocConfigT, LockConfigT, USE_TLABS>::~BumpPointerAllocator()
{
    LOG_BUMP_ALLOCATOR(DEBUG) << "Destroying of BumpPointerAllocator";
    LOG_BUMP_ALLOCATOR(DEBUG) << "Destroying of BumpPointerAllocator finished";
}

template <typename AllocConfigT, typename LockConfigT, bool USE_TLABS>
void BumpPointerAllocator<AllocConfigT, LockConfigT, USE_TLABS>::Reset()
{
    // Remove CrossingMap and create for avoid check in Alloc method
    if (LIKELY(arena_.GetOccupiedSize() > 0)) {
        AllocConfigT::RemoveCrossingMapForMemory(arena_.GetMem(), arena_.GetSize());
        AllocConfigT::InitializeCrossingMapForMemory(arena_.GetMem(), arena_.GetSize());
    }
    arena_.Reset();
    if constexpr (USE_TLABS) {
        tlabManager_.Reset();
    }
}

template <typename AllocConfigT, typename LockConfigT, bool USE_TLABS>
void BumpPointerAllocator<AllocConfigT, LockConfigT, USE_TLABS>::ExpandMemory(void *mem, size_t size)
{
    LOG_BUMP_ALLOCATOR(DEBUG) << "Expand memory: Add " << std::dec << size << " bytes of memory at addr " << std::hex
                              << mem;
    ASSERT(ToUintPtr(arena_.GetArenaEnd()) == ToUintPtr(mem));
    if constexpr (USE_TLABS) {
        UNREACHABLE();
    }
    arena_.ExpandArena(mem, size);
    AllocConfigT::InitializeCrossingMapForMemory(mem, size);
}

template <typename AllocConfigT, typename LockConfigT, bool USE_TLABS>
void *BumpPointerAllocator<AllocConfigT, LockConfigT, USE_TLABS>::Alloc(size_t size, Alignment alignment)
{
    os::memory::LockHolder lock(allocatorLock_);
    LOG_BUMP_ALLOCATOR(DEBUG) << "Try to allocate " << std::dec << size << " bytes of memory";
    ASSERT(alignment == DEFAULT_ALIGNMENT);
    // We need to align up it here to write correct used memory size inside MemStats.
    // (each element allocated via BumpPointer allocator has DEFAULT_ALIGNMENT alignment).
    size = AlignUp(size, DEFAULT_ALIGNMENT_IN_BYTES);
    void *mem = nullptr;
    // NOLINTNEXTLINE(readability-braces-around-statements)
    if constexpr (!USE_TLABS) {
        // Use common scenario
        mem = arena_.Alloc(size, alignment);
        // NOLINTNEXTLINE(readability-misleading-indentation)
    } else {
        // We must take TLABs occupied memory into account.
        ASSERT(arena_.GetFreeSize() >= tlabManager_.GetTLABsOccupiedSize());
        if (arena_.GetFreeSize() - tlabManager_.GetTLABsOccupiedSize() >= size) {
            mem = arena_.Alloc(size, alignment);
        }
    }
    if (mem == nullptr) {
        LOG_BUMP_ALLOCATOR(DEBUG) << "Couldn't allocate memory";
        return nullptr;
    }
    AllocConfigT::OnAlloc(size, typeAllocation_, memStats_);
    AllocConfigT::AddToCrossingMap(mem, size);
    AllocConfigT::MemoryInit(mem);
    return mem;
}

template <typename AllocConfigT, typename LockConfigT, bool USE_TLABS>
TLAB *BumpPointerAllocator<AllocConfigT, LockConfigT, USE_TLABS>::CreateNewTLAB(size_t size)
{
    os::memory::LockHolder lock(allocatorLock_);
    // NOLINTNEXTLINE(readability-braces-around-statements, bugprone-suspicious-semicolon)
    if constexpr (!USE_TLABS) {
        UNREACHABLE();
    }
    LOG_BUMP_ALLOCATOR(DEBUG) << "Try to create a TLAB with size " << std::dec << size;
    ASSERT(size == AlignUp(size, DEFAULT_ALIGNMENT_IN_BYTES));
    TLAB *tlab = nullptr;
    ASSERT(arena_.GetFreeSize() >= tlabManager_.GetTLABsOccupiedSize());
    if (arena_.GetFreeSize() - tlabManager_.GetTLABsOccupiedSize() >= size) {
        tlab = tlabManager_.GetUnusedTLABInstance();
        if (tlab != nullptr) {
            tlabManager_.IncreaseTLABsOccupiedSize(size);
            uintptr_t endOfArena = ToUintPtr(arena_.GetArenaEnd());
            ASSERT(endOfArena >= tlabManager_.GetTLABsOccupiedSize());
            void *tlabBufferStart = ToVoidPtr(endOfArena - tlabManager_.GetTLABsOccupiedSize());
            tlab->Fill(tlabBufferStart, size);
            LOG_BUMP_ALLOCATOR(DEBUG) << "Created new TLAB with size " << std::dec << size << " at addr " << std::hex
                                      << tlabBufferStart;
        } else {
            LOG_BUMP_ALLOCATOR(DEBUG) << "Reached the limit of TLABs inside the allocator";
        }
    } else {
        LOG_BUMP_ALLOCATOR(DEBUG) << "Don't have enough memory for new TLAB with size " << std::dec << size;
    }
    return tlab;
}

template <typename AllocConfigT, typename LockConfigT, bool USE_TLABS>
void BumpPointerAllocator<AllocConfigT, LockConfigT, USE_TLABS>::VisitAndRemoveAllPools(const MemVisitor &memVisitor)
{
    os::memory::LockHolder lock(allocatorLock_);
    AllocConfigT::RemoveCrossingMapForMemory(arena_.GetMem(), arena_.GetSize());
    memVisitor(arena_.GetMem(), arena_.GetSize());
}

template <typename AllocConfigT, typename LockConfigT, bool USE_TLABS>
void BumpPointerAllocator<AllocConfigT, LockConfigT, USE_TLABS>::VisitAndRemoveFreePools(const MemVisitor &memVisitor)
{
    (void)memVisitor;
    os::memory::LockHolder lock(allocatorLock_);
    // We should do nothing here
}

template <typename AllocConfigT, typename LockConfigT, bool USE_TLABS>
void BumpPointerAllocator<AllocConfigT, LockConfigT, USE_TLABS>::IterateOverObjects(
    const std::function<void(ObjectHeader *objectHeader)> &objectVisitor)
{
    os::memory::LockHolder lock(allocatorLock_);
    LOG_BUMP_ALLOCATOR(DEBUG) << "Iteration over objects started";
    void *curPtr = arena_.GetAllocatedStart();
    void *endPtr = arena_.GetAllocatedEnd();
    while (curPtr < endPtr) {
        auto objectHeader = static_cast<ObjectHeader *>(curPtr);
        size_t objectSize = GetObjectSize(curPtr);
        objectVisitor(objectHeader);
        curPtr = ToVoidPtr(AlignUp(ToUintPtr(curPtr) + objectSize, DEFAULT_ALIGNMENT_IN_BYTES));
    }
    // NOLINTNEXTLINE(readability-braces-around-statements, bugprone-suspicious-semicolon)
    if constexpr (USE_TLABS) {
        LOG_BUMP_ALLOCATOR(DEBUG) << "Iterate over TLABs";
        // Iterate over objects in TLABs:
        tlabManager_.IterateOverTLABs([&objectVisitor](TLAB *tlab) {
            tlab->IterateOverObjects(objectVisitor);
            return true;
        });
        LOG_BUMP_ALLOCATOR(DEBUG) << "Iterate over TLABs finished";
    }
    LOG_BUMP_ALLOCATOR(DEBUG) << "Iteration over objects finished";
}

template <typename AllocConfigT, typename LockConfigT, bool USE_TLABS>
template <typename MemVisitor>
void BumpPointerAllocator<AllocConfigT, LockConfigT, USE_TLABS>::IterateOverObjectsInRange(const MemVisitor &memVisitor,
                                                                                           void *leftBorder,
                                                                                           void *rightBorder)
{
    ASSERT(ToUintPtr(rightBorder) >= ToUintPtr(leftBorder));
    // NOTE(ipetrov): These are temporary asserts because we can't do anything
    // if the range crosses different allocators memory pools
    ASSERT(ToUintPtr(rightBorder) - ToUintPtr(leftBorder) == (CrossingMapSingleton::GetCrossingMapGranularity() - 1U));
    ASSERT((ToUintPtr(rightBorder) & (~(CrossingMapSingleton::GetCrossingMapGranularity() - 1U))) ==
           (ToUintPtr(leftBorder) & (~(CrossingMapSingleton::GetCrossingMapGranularity() - 1U))));

    os::memory::LockHolder lock(allocatorLock_);
    LOG_BUMP_ALLOCATOR(DEBUG) << "IterateOverObjectsInRange for range [" << std::hex << leftBorder << ", "
                              << rightBorder << "]";
    MemRange inputMemRange(ToUintPtr(leftBorder), ToUintPtr(rightBorder));
    MemRange arenaOccupiedMemRange(0U, 0U);
    if (arena_.GetOccupiedSize() > 0) {
        arenaOccupiedMemRange =
            MemRange(ToUintPtr(arena_.GetAllocatedStart()), ToUintPtr(arena_.GetAllocatedEnd()) - 1);
    }
    // In this case, we iterate over objects in intersection of memory range of occupied memory via arena_.Alloc()
    // and memory range of input range
    if (arenaOccupiedMemRange.IsIntersect(inputMemRange)) {
        void *startPtr = ToVoidPtr(std::max(inputMemRange.GetStartAddress(), arenaOccupiedMemRange.GetStartAddress()));
        void *endPtr = ToVoidPtr(std::min(inputMemRange.GetEndAddress(), arenaOccupiedMemRange.GetEndAddress()));

        void *objAddr = AllocConfigT::FindFirstObjInCrossingMap(startPtr, endPtr);
        if (objAddr != nullptr) {
            ASSERT(arenaOccupiedMemRange.GetStartAddress() <= ToUintPtr(objAddr) &&
                   ToUintPtr(objAddr) <= arenaOccupiedMemRange.GetEndAddress());
            void *currentPtr = objAddr;
            while (currentPtr < endPtr) {
                auto *objectHeader = static_cast<ObjectHeader *>(currentPtr);
                size_t objectSize = GetObjectSize(currentPtr);
                memVisitor(objectHeader);
                currentPtr = ToVoidPtr(AlignUp(ToUintPtr(currentPtr) + objectSize, DEFAULT_ALIGNMENT_IN_BYTES));
            }
        }
    }
    // NOLINTNEXTLINE(readability-braces-around-statements, bugprone-suspicious-semicolon)
    if constexpr (USE_TLABS) {
        // If we didn't allocate any TLAB then we don't need iterate by TLABs
        if (tlabManager_.GetTLABsOccupiedSize() == 0) {
            return;
        }
        uintptr_t endOfArena = ToUintPtr(arena_.GetArenaEnd());
        uintptr_t startTlab = endOfArena - tlabManager_.GetTLABsOccupiedSize();
        MemRange tlabsMemRange(startTlab, endOfArena - 1);
        // In this case, we iterate over objects in intersection of memory range of TLABs
        // and memory range of input range
        if (tlabsMemRange.IsIntersect(inputMemRange)) {
            void *startPtr = ToVoidPtr(std::max(inputMemRange.GetStartAddress(), tlabsMemRange.GetStartAddress()));
            void *endPtr = ToVoidPtr(std::min(inputMemRange.GetEndAddress(), tlabsMemRange.GetEndAddress()));
            tlabManager_.IterateOverTLABs(
                [&memVisitor, memRange = MemRange(ToUintPtr(startPtr), ToUintPtr(endPtr))](TLAB *tlab) -> bool {
                    tlab->IterateOverObjectsInRange(memVisitor, memRange);
                    return true;
                });
        }
    }
}

template <typename AllocConfigT, typename LockConfigT, bool USE_TLABS>
MemRange BumpPointerAllocator<AllocConfigT, LockConfigT, USE_TLABS>::GetMemRange()
{
    return MemRange(ToUintPtr(arena_.GetAllocatedStart()), ToUintPtr(arena_.GetArenaEnd()) - 1);
}

template <typename AllocConfigT, typename LockConfigT, bool USE_TLABS>
template <typename ObjectMoveVisitorT>
void BumpPointerAllocator<AllocConfigT, LockConfigT, USE_TLABS>::CollectAndMove(
    const GCObjectVisitor &deathChecker, const ObjectMoveVisitorT &objectMoveVisitor)
{
    IterateOverObjects([&](ObjectHeader *objectHeader) {
        // We are interested only in moving alive objects, after that we cleanup arena
        if (deathChecker(objectHeader) == ObjectStatus::ALIVE_OBJECT) {
            objectMoveVisitor(objectHeader);
        }
    });
}

template <typename AllocConfigT, typename LockConfigT, bool USE_TLABS>
bool BumpPointerAllocator<AllocConfigT, LockConfigT, USE_TLABS>::ContainObject(const ObjectHeader *obj)
{
    bool result = false;
    result = arena_.InArena(const_cast<ObjectHeader *>(obj));
    if ((USE_TLABS) && (!result)) {
        // Check TLABs
        tlabManager_.IterateOverTLABs([&](TLAB *tlab) {
            result = tlab->ContainObject(obj);
            return !result;
        });
    }
    return result;
}

template <typename AllocConfigT, typename LockConfigT, bool USE_TLABS>
bool BumpPointerAllocator<AllocConfigT, LockConfigT, USE_TLABS>::IsLive(const ObjectHeader *obj)
{
    ASSERT(ContainObject(obj));
    void *objMem = static_cast<void *>(const_cast<ObjectHeader *>(obj));
    if (arena_.InArena(objMem)) {
        void *currentObj = AllocConfigT::FindFirstObjInCrossingMap(objMem, objMem);
        if (UNLIKELY(currentObj == nullptr)) {
            return false;
        }
        while (currentObj < objMem) {
            size_t objectSize = GetObjectSize(currentObj);
            currentObj = ToVoidPtr(AlignUp(ToUintPtr(currentObj) + objectSize, DEFAULT_ALIGNMENT_IN_BYTES));
        }
        return currentObj == objMem;
    }
    // NOLINTNEXTLINE(readability-braces-around-statements, bugprone-suspicious-semicolon)
    if constexpr (USE_TLABS) {
        bool result = false;
        auto objectVisitor = [&result, obj](ObjectHeader *objectHeader) {
            if (objectHeader == obj) {
                result = true;
            }
        };
        auto tlabVisitor = [&objectVisitor, obj](TLAB *tlab) {
            if (tlab->ContainObject(obj)) {
                tlab->IterateOverObjects(objectVisitor);
                return false;
            }
            return true;
        };
        tlabManager_.IterateOverTLABs(tlabVisitor);
        return result;
    }
    return false;
}

#undef LOG_BUMP_ALLOCATOR

}  // namespace ark::mem

#endif  // RUNTIME_MEM_PANDA_BUMP_ALLOCATOR_INL_H
