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
#ifndef PANDA_RUNTIME_MEM_FRAME_ALLOCATOR_INL_H
#define PANDA_RUNTIME_MEM_FRAME_ALLOCATOR_INL_H

#include "runtime/mem/frame_allocator.h"

#include <cstring>

#include "libarkbase/mem/pool_manager.h"
#include "libarkbase/utils/logger.h"
#include "libarkbase/mem/stack_like_allocator-inl.h"

namespace ark::mem {

using StackFrameAllocator = StackLikeAllocator<>;

// NOLINTNEXTLINE(cppcoreguidelines-macro-usage)
#define LOG_FRAME_ALLOCATOR(level) LOG(level, ALLOC) << "FrameAllocator: "

template <Alignment ALIGNMENT, bool USE_MEMSET>
inline FrameAllocator<ALIGNMENT, USE_MEMSET>::FrameAllocator(bool useMalloc, SpaceType spaceType)
    : useMalloc_(useMalloc), spaceType_(spaceType)
{
    LOG_FRAME_ALLOCATOR(DEBUG) << "Initializing of FrameAllocator";
    if (!useMalloc_) {
        memPoolAlloc_ = PoolManager::GetMmapMemPool();
    }
    curArena_ = AllocateArenaImpl(FIRST_ARENA_SIZE);
    lastAllocArena_ = curArena_;
    biggestArenaSize_ = FIRST_ARENA_SIZE;
    arenaSizeNeedToGrow_ = true;
    LOG_FRAME_ALLOCATOR(DEBUG) << "Initializing of FrameAllocator finished";
}

template <Alignment ALIGNMENT, bool USE_MEMSET>
inline FrameAllocator<ALIGNMENT, USE_MEMSET>::~FrameAllocator()
{
    LOG_FRAME_ALLOCATOR(DEBUG) << "Destroying of FrameAllocator";
    while (lastAllocArena_ != nullptr) {
        LOG_FRAME_ALLOCATOR(DEBUG) << "Free arena at addr " << std::hex << lastAllocArena_;
        FramesArena *prevArena = lastAllocArena_->GetPrevArena();
        FreeArenaImpl(lastAllocArena_);
        lastAllocArena_ = prevArena;
    }
    curArena_ = nullptr;
    LOG_FRAME_ALLOCATOR(DEBUG) << "Destroying of FrameAllocator finished";
}

template <Alignment ALIGNMENT, bool USE_MEMSET>
// CC-OFFNXT(G.FUD.06) perf critical
inline bool FrameAllocator<ALIGNMENT, USE_MEMSET>::TryAllocateNewArena(size_t size)
{
    size_t arenaSize = GetNextArenaSize(size);
    LOG_FRAME_ALLOCATOR(DEBUG) << "Try to allocate a new arena with size " << arenaSize;
    FramesArena *newArena = AllocateArenaImpl(arenaSize);
    if (newArena == nullptr) {
        LOG_FRAME_ALLOCATOR(DEBUG) << "Couldn't get memory for a new arena";
        arenaSizeNeedToGrow_ = false;
        return false;
    }
    lastAllocArena_->LinkNext(newArena);
    newArena->LinkPrev(lastAllocArena_);
    lastAllocArena_ = newArena;
    emptyArenasCount_++;
    LOG_FRAME_ALLOCATOR(DEBUG) << "Successfully allocate new arena with addr " << std::hex << newArena;
    return true;
}

template <Alignment ALIGNMENT, bool USE_MEMSET>
ALWAYS_INLINE inline void *FrameAllocator<ALIGNMENT, USE_MEMSET>::Alloc(size_t size)
{
    ASSERT(AlignUp(size, GetAlignmentInBytes(ALIGNMENT)) == size);
    // Try to get free memory from current arenas
    void *mem = TryToAllocate(size);

    if (UNLIKELY(mem == nullptr)) {
        LOG_FRAME_ALLOCATOR(DEBUG) << "Can't allocate " << size << " bytes for a new frame in current arenas";
        if (!TryAllocateNewArena(size)) {
            LOG_FRAME_ALLOCATOR(DEBUG) << "Can't allocate a new arena, return nullptr";
            return nullptr;
        }
        mem = TryToAllocate(size);
        if (mem == nullptr) {
            LOG_FRAME_ALLOCATOR(DEBUG) << "Can't allocate memory in a totally free arena, change default arenas sizes";
            return nullptr;
        }
    }

    ASSERT(AlignUp(ToUintPtr(mem), GetAlignmentInBytes(ALIGNMENT)) == ToUintPtr(mem));
    LOG_FRAME_ALLOCATOR(DEBUG) << "Allocated memory at addr " << std::hex << mem;
    // NOLINTNEXTLINE(readability-braces-around-statements, bugprone-suspicious-semicolon)
    if constexpr (USE_MEMSET) {
        memset_s(mem, size, 0x00, size);
    }
    allocatedSize_ += size;
    return mem;
}

template <Alignment ALIGNMENT, bool USE_MEMSET>
ALWAYS_INLINE inline void FrameAllocator<ALIGNMENT, USE_MEMSET>::Free(void *mem)
{
    ASSERT(curArena_ != nullptr);  // must has been initialized!
    ASSERT(ToUintPtr(mem) == AlignUp(ToUintPtr(mem), GetAlignmentInBytes(ALIGNMENT)));
    if (curArena_->InArena(mem)) {
        allocatedSize_ -= ToUintPtr(curArena_->GetTop()) - ToUintPtr(mem);
        curArena_->Free(mem);
    } else {
        ASSERT(curArena_->GetOccupiedSize() == 0);
        ASSERT(curArena_->GetPrevArena() != nullptr);

        curArena_ = curArena_->GetPrevArena();
        ASSERT(curArena_->InArena(mem));
        allocatedSize_ -= ToUintPtr(curArena_->GetTop()) - ToUintPtr(mem);
        curArena_->Free(mem);
        if (UNLIKELY((emptyArenasCount_ + 1) > FRAME_ALLOC_MAX_FREE_ARENAS_THRESHOLD)) {
            FreeLastArena();
        } else {
            emptyArenasCount_++;
        }
    }
    LOG_FRAME_ALLOCATOR(DEBUG) << "Free memory at addr " << std::hex << mem;
}

template <Alignment ALIGNMENT, bool USE_MEMSET>
// CC-OFFNXT(G.FUD.06) perf critical
inline void *FrameAllocator<ALIGNMENT, USE_MEMSET>::TryToAllocate(size_t size)
{
    // Try to allocate memory in the current arena:
    ASSERT(curArena_ != nullptr);
    void *mem = curArena_->Alloc(size);
    if (LIKELY(mem != nullptr)) {
        return mem;
    }
    // We don't have enough memory in current arena, try to allocate in the next one:
    FramesArena *nextArena = curArena_->GetNextArena();
    if (nextArena == nullptr) {
        LOG_FRAME_ALLOCATOR(DEBUG) << "TryToPush failed - we don't have a free arena";
        return nullptr;
    }
    mem = nextArena->Alloc(size);
    if (LIKELY(mem != nullptr)) {
        ASSERT(emptyArenasCount_ > 0);
        emptyArenasCount_--;
        curArena_ = nextArena;
        return mem;
    }
    LOG_FRAME_ALLOCATOR(DEBUG) << "Couldn't allocate " << size << " bytes of memory in the totally free arena."
                               << " Change initial sizes of arenas";
    return nullptr;
}

template <Alignment ALIGNMENT, bool USE_MEMSET>
inline size_t FrameAllocator<ALIGNMENT, USE_MEMSET>::GetNextArenaSize(size_t size)
{
    size_t requestedSize = size + sizeof(FramesArena) + GetAlignmentInBytes(ALIGNMENT);
    if ((arenaSizeNeedToGrow_) || (biggestArenaSize_ < requestedSize)) {
        biggestArenaSize_ += ARENA_SIZE_GREW_LEVEL;
        if (biggestArenaSize_ < requestedSize) {
            biggestArenaSize_ = RoundUp(requestedSize, ARENA_SIZE_GREW_LEVEL);
        }
    } else {
        arenaSizeNeedToGrow_ = true;
    }
    return biggestArenaSize_;
}

template <Alignment ALIGNMENT, bool USE_MEMSET>
// CC-OFFNXT(G.FUD.06) perf critical
inline void FrameAllocator<ALIGNMENT, USE_MEMSET>::FreeLastArena()
{
    ASSERT(lastAllocArena_ != nullptr);
    FramesArena *arenaToFree = lastAllocArena_;
    lastAllocArena_ = arenaToFree->GetPrevArena();
    if (arenaToFree == curArena_) {
        curArena_ = lastAllocArena_;
    }
    if (lastAllocArena_ == nullptr) {
        ASSERT(lastAllocArena_ == curArena_);
        // To fix clang tidy warning
        // (it suggests that cur_arena_ can be not nullptr when
        //  last_alloc_arena_ is equal to nullptr)
        curArena_ = lastAllocArena_;
        LOG_FRAME_ALLOCATOR(DEBUG) << "Clear the last arena in the list";
    } else {
        lastAllocArena_->ClearNextLink();
    }
    LOG_FRAME_ALLOCATOR(DEBUG) << "Free the arena at addr " << std::hex << arenaToFree;
    FreeArenaImpl(arenaToFree);
    arenaSizeNeedToGrow_ = false;
}

template <Alignment ALIGNMENT, bool USE_MEMSET>
// CC-OFFNXT(G.FMT.07) project code style
// CC-OFFNXT(G.FUD.06) perf critical
inline typename FrameAllocator<ALIGNMENT, USE_MEMSET>::FramesArena *
FrameAllocator<ALIGNMENT, USE_MEMSET>::AllocateArenaImpl(size_t size)
{
    FramesArena *newArena = nullptr;
    if (!useMalloc_) {
        ASSERT(memPoolAlloc_ != nullptr);
        newArena = memPoolAlloc_->AllocArena<FramesArena>(size, spaceType_, AllocatorType::FRAME_ALLOCATOR, this);
    } else {
        auto mem = ark::os::mem::AlignedAlloc(GetAlignmentInBytes(ARENA_DEFAULT_ALIGNMENT), size);
        if (mem != nullptr) {
            auto arenaBuffOffs = AlignUp(sizeof(FramesArena), GetAlignmentInBytes(ARENA_DEFAULT_ALIGNMENT));
            newArena = new (mem) FramesArena(size - arenaBuffOffs, ToVoidPtr(ToUintPtr(mem) + arenaBuffOffs));
        }
    }
    return newArena;
}

template <Alignment ALIGNMENT, bool USE_MEMSET>
inline void FrameAllocator<ALIGNMENT, USE_MEMSET>::FreeArenaImpl(FramesArena *arena)
{
    ASSERT(arena != nullptr);
    if (!useMalloc_) {
        ASSERT(memPoolAlloc_ != nullptr);
        memPoolAlloc_->FreeArena<FramesArena>(arena);
    } else {
        os::mem::AlignedFree(arena);
    }
}

template <Alignment ALIGNMENT, bool USE_MEMSET>
inline bool FrameAllocator<ALIGNMENT, USE_MEMSET>::Contains(void *mem)
{
    auto curArena = curArena_;

    while (curArena != nullptr) {
        LOG_FRAME_ALLOCATOR(DEBUG) << "check InAllocator arena at addr " << std::hex << curArena;
        if (curArena->InArena(mem)) {
            return true;
        }
        curArena = curArena->GetPrevArena();
    }
    return false;
}

#undef LOG_FRAME_ALLOCATOR

}  // namespace ark::mem
#endif  // PANDA_RUNTIME_MEM_FRAME_ALLOCATOR_INL_H
