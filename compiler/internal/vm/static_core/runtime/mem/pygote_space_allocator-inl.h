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
#ifndef RUNTIME_MEM_PANDA_PYGOTE_SPACE_ALLOCATOR_INL_H
#define RUNTIME_MEM_PANDA_PYGOTE_SPACE_ALLOCATOR_INL_H

#include "libarkbase/utils/logger.h"
#include "runtime/include/mem/allocator.h"
#include "runtime/mem/object_helpers.h"
#include "runtime/mem/pygote_space_allocator.h"
#include "runtime/mem/runslots_allocator-inl.h"
#include "runtime/include/runtime.h"

namespace ark::mem {

// NOLINTNEXTLINE(cppcoreguidelines-macro-usage)
#define LOG_PYGOTE_SPACE_ALLOCATOR(level) LOG(level, ALLOC) << "PygoteSpaceAllocator: "

template <typename AllocConfigT>
PygoteSpaceAllocator<AllocConfigT>::PygoteSpaceAllocator(MemStatsType *memStats)
    : runslotsAlloc_(memStats), memStats_(memStats)
{
    LOG_PYGOTE_SPACE_ALLOCATOR(DEBUG) << "Initializing of PygoteSpaceAllocator";
}

template <typename AllocConfigT>
PygoteSpaceAllocator<AllocConfigT>::~PygoteSpaceAllocator()
{
    auto cur = arena_;
    while (cur != nullptr) {
        auto tmp = cur->GetNextArena();
        PoolManager::FreeArena(cur);
        cur = tmp;
    }
    auto allocator = Runtime::GetCurrent()->GetInternalAllocator();
    for (const auto &bitmap : liveBitmaps_) {
        allocator->Delete(bitmap->GetBitMap().data());
        allocator->Delete(bitmap);
    }
    LOG_PYGOTE_SPACE_ALLOCATOR(DEBUG) << "Destroying of PygoteSpaceAllocator";
}

template <typename AllocConfigT>
// CC-OFFNXT(G.FUD.06) perf critical
inline void PygoteSpaceAllocator<AllocConfigT>::SetState(PygoteSpaceState newState)
{
    // must move to next state
    ASSERT(newState > state_);
    state_ = newState;

    if (state_ != STATE_PYGOTE_FORKED) {
        return;
    }
    // build bitmaps for used pools
    runslotsAlloc_.memoryPool_.VisitAllPoolsWithOccupiedSize(
        [this](void *mem, size_t usedSize, size_t) { CreateLiveBitmap(mem, usedSize); });
    runslotsAlloc_.IterateOverObjects([this](ObjectHeader *object) {
        for (auto bitmap : liveBitmaps_) {
            if (bitmap->IsAddrInRange(object)) {
                bitmap->Set(object);
                return;
            }
        }
    });

    // trim unused pages in runslots allocator
    runslotsAlloc_.TrimUnsafe();

    // only trim the last arena
    if (arena_ != nullptr && arena_->GetFreeSize() >= ark::os::mem::GetPageSize()) {
        uintptr_t start = AlignUp(ToUintPtr(arena_->GetAllocatedEnd()), ark::os::mem::GetPageSize());
        uintptr_t end = ToUintPtr(arena_->GetArenaEnd());
        os::mem::ReleasePages(start, end);
    }
}

template <typename AllocConfigT>
// CC-OFFNXT(G.FUD.06) perf critical
inline void *PygoteSpaceAllocator<AllocConfigT>::Alloc(size_t size, Alignment align)
{
    ASSERT(state_ == STATE_PYGOTE_INIT || state_ == STATE_PYGOTE_FORKING);

    // alloc from runslots firstly, if failed, try to alloc from new arena
    // NOTE(yxr) : will optimzie this later, currently we use runslots as much as possible before we have crossing map
    // or mark card table with object header, also it will reduce the bitmap count which will reduce the gc mark time.
    void *obj = runslotsAlloc_.template Alloc<true, false>(size, align);
    if (obj != nullptr) {
        return obj;
    }
    if (state_ == STATE_PYGOTE_INIT) {
        // try again in lock
        static os::memory::Mutex poolLock;
        os::memory::LockHolder lock(poolLock);
        obj = runslotsAlloc_.Alloc(size, align);
        if (obj != nullptr) {
            return obj;
        }

        auto pool = heapSpace_->TryAllocPool(RunSlotsAllocator<AllocConfigT>::GetMinPoolSize(), spaceType_,
                                             AllocatorType::RUNSLOTS_ALLOCATOR, this);
        if (UNLIKELY(pool.GetMem() == nullptr)) {
            return nullptr;
        }
        if (!runslotsAlloc_.AddMemoryPool(pool.GetMem(), pool.GetSize())) {
            LOG(FATAL, ALLOC) << "PygoteSpaceAllocator: couldn't add memory pool to object allocator";
        }
        // alloc object again
        obj = runslotsAlloc_.Alloc(size, align);
    } else {
        if (arena_ != nullptr) {
            obj = arena_->Alloc(size, align);
        }
        if (obj == nullptr) {
            auto newArena =
                heapSpace_->TryAllocArena(DEFAULT_ARENA_SIZE, spaceType_, AllocatorType::ARENA_ALLOCATOR, this);
            if (newArena == nullptr) {
                return nullptr;
            }
            CreateLiveBitmap(newArena, DEFAULT_ARENA_SIZE);
            newArena->LinkTo(arena_);
            arena_ = newArena;
            obj = arena_->Alloc(size, align);
        }
        liveBitmaps_.back()->Set(obj);  // mark live in bitmap
        AllocConfigT::OnAlloc(size, spaceType_, memStats_);
        AllocConfigT::MemoryInit(obj);
    }
    return obj;
}

template <typename AllocConfigT>
// CC-OFFNXT(G.FUD.06) perf critical
inline void PygoteSpaceAllocator<AllocConfigT>::Free(void *mem)
{
    for (auto bitmap : liveBitmaps_) {
        if (bitmap->IsAddrInRange(mem)) {
            bitmap->Clear(mem);
            return;
        }
    }

    if (state_ == STATE_PYGOTE_FORKED) {
        return;
    }

    if (runslotsAlloc_.ContainObject(reinterpret_cast<ObjectHeader *>(mem))) {
        runslotsAlloc_.Free(mem);
    }
}

template <typename AllocConfigT>
inline bool PygoteSpaceAllocator<AllocConfigT>::ContainObject(const ObjectHeader *object)
{
    // see if in runslots firstly
    if (runslotsAlloc_.ContainObject(object)) {
        return true;
    }

    // see if in arena list
    for (auto cur = arena_; cur != nullptr; cur = cur->GetNextArena()) {
        if (cur->InArena(const_cast<ObjectHeader *>(object))) {
            return true;
        }
    }
    return false;
}

template <typename AllocConfigT>
inline bool PygoteSpaceAllocator<AllocConfigT>::IsLive(const ObjectHeader *object)
{
    for (auto bitmap : liveBitmaps_) {
        if (bitmap->IsAddrInRange(object)) {
            return bitmap->Test(object);
        }
    }

    if (state_ == STATE_PYGOTE_FORKED) {
        return false;
    }

    return runslotsAlloc_.ContainObject(object) && runslotsAlloc_.IsLive(object);
}

template <typename AllocConfigT>
inline void PygoteSpaceAllocator<AllocConfigT>::CreateLiveBitmap(void *heapBegin, size_t heapSize)
{
    auto allocator = Runtime::GetCurrent()->GetInternalAllocator();
    auto bitmapData = allocator->Alloc(MarkBitmap::GetBitMapSizeInByte(heapSize));
    ASSERT(bitmapData != nullptr);
    auto bitmap = allocator->Alloc(sizeof(MarkBitmap));
    ASSERT(bitmap != nullptr);
    auto bitmapObj = new (bitmap) MarkBitmap(heapBegin, heapSize, bitmapData);
    bitmapObj->ClearAllBits();
    liveBitmaps_.emplace_back(bitmapObj);
}

template <typename AllocConfigT>
inline void PygoteSpaceAllocator<AllocConfigT>::ClearLiveBitmaps()
{
    for (auto bitmap : liveBitmaps_) {
        bitmap->ClearAllBits();
    }
}

template <typename AllocConfigT>
template <typename Visitor>
// CC-OFFNXT(G.FUD.06) perf critical
inline void PygoteSpaceAllocator<AllocConfigT>::IterateOverObjectsInRange(const Visitor &visitor, void *start,
                                                                          void *end)
{
    if (liveBitmaps_.empty()) {
        ASSERT(arena_ == nullptr);
        runslotsAlloc_.IterateOverObjectsInRange(visitor, start, end);
        return;
    }
    for (auto bitmap : liveBitmaps_) {
        auto [left, right] = bitmap->GetHeapRange();
        left = std::max(ToUintPtr(start), left);
        right = std::min(ToUintPtr(end), right);
        if (left < right) {
            bitmap->IterateOverMarkedChunkInRange(ToVoidPtr(left), ToVoidPtr(right), [&visitor](void *mem) {
                visitor(reinterpret_cast<ObjectHeader *>(mem));
            });
        }
    }
}

template <typename AllocConfigT>
// CC-OFFNXT(G.FUD.06) perf critical
inline void PygoteSpaceAllocator<AllocConfigT>::IterateOverObjects(const ObjectVisitor &objectVisitor)
{
    if (!liveBitmaps_.empty()) {
        for (auto bitmap : liveBitmaps_) {
            bitmap->IterateOverMarkedChunks(
                [&objectVisitor](void *mem) { objectVisitor(static_cast<ObjectHeader *>(static_cast<void *>(mem))); });
        }
        if (state_ != STATE_PYGOTE_FORKED) {
            runslotsAlloc_.IterateOverObjects(objectVisitor);
        }
    } else {
        ASSERT(arena_ == nullptr);
        runslotsAlloc_.IterateOverObjects(objectVisitor);
    }
}

template <typename AllocConfigT>
inline void PygoteSpaceAllocator<AllocConfigT>::VisitAndRemoveAllPools(const MemVisitor &memVisitor)
{
    // IterateOverPools only used when allocator should be destroyed
    auto cur = arena_;
    while (cur != nullptr) {
        auto tmp = cur->GetNextArena();
        heapSpace_->FreeArena(cur);
        cur = tmp;
    }
    arena_ = nullptr;  // avoid to duplicated free
    runslotsAlloc_.VisitAndRemoveAllPools(memVisitor);
}

template <typename AllocConfigT>
inline void PygoteSpaceAllocator<AllocConfigT>::VisitAndRemoveFreePools(const MemVisitor &memVisitor)
{
    // afte pygote fork, we don't change pygote space for free unused pools
    if (state_ == STATE_PYGOTE_FORKED) {
        return;
    }

    // before pygote fork, call underlying allocator to free unused pools
    runslotsAlloc_.VisitAndRemoveFreePools(memVisitor);
}

template <typename AllocConfigT>
inline void PygoteSpaceAllocator<AllocConfigT>::Collect(const GCObjectVisitor &gcVisitor)
{
    // the live bitmaps has been updated in gc process, need to do nothing here
    if (state_ == STATE_PYGOTE_FORKED) {
        return;
    }

    // before pygote fork, call underlying allocator to collect garbage
    runslotsAlloc_.Collect(gcVisitor);
}

#undef LOG_PYGOTE_SPACE_ALLOCATOR

}  // namespace ark::mem

#endif  // RUNTIME_MEM_PANDA_PYGOTE_SPACE_ALLOCATOR_INL_H
