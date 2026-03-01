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

#include "arena-inl.h"
#include "arena_allocator.h"
#include "libarkbase/utils/logger.h"
#include "pool_manager.h"
#include "libarkbase/trace/trace.h"
#include "libarkbase/mem/base_mem_stats.h"
#include <cstdlib>
#include <cstring>

namespace ark {

template <bool USE_OOM_HANDLER>
ArenaAllocatorT<USE_OOM_HANDLER>::ArenaAllocatorT(SpaceType spaceType, BaseMemStats *memStats,
                                                  bool limitAllocSizeByPool)
    : memStats_(memStats), spaceType_(spaceType), limitAllocSizeByPool_(limitAllocSizeByPool)
{
    ASSERT(!USE_OOM_HANDLER);
    if (!ON_STACK_ALLOCATION_ENABLED) {
        arenas_ = PoolManager::AllocArena(DEFAULT_ARENA_SIZE, spaceType_, AllocatorType::ARENA_ALLOCATOR, this);
        ASSERT(arenas_ != nullptr);
        AllocArenaMemStats(DEFAULT_ARENA_SIZE);
    }
}

template <bool USE_OOM_HANDLER>
ArenaAllocatorT<USE_OOM_HANDLER>::ArenaAllocatorT(OOMHandler oomHandler, SpaceType spaceType, BaseMemStats *memStats,
                                                  bool limitAllocSizeByPool)
    : memStats_(memStats), spaceType_(spaceType), oomHandler_(oomHandler), limitAllocSizeByPool_(limitAllocSizeByPool)
{
    ASSERT(USE_OOM_HANDLER);
    if (!ON_STACK_ALLOCATION_ENABLED) {
        arenas_ = PoolManager::AllocArena(DEFAULT_ARENA_SIZE, spaceType_, AllocatorType::ARENA_ALLOCATOR, this);
        ASSERT(arenas_ != nullptr);
        AllocArenaMemStats(DEFAULT_ARENA_SIZE);
    }
}

template <bool USE_OOM_HANDLER>
ArenaAllocatorT<USE_OOM_HANDLER>::~ArenaAllocatorT()
{
    Arena *cur = arenas_;
    while (cur != nullptr) {
        Arena *tmp = cur->GetNextArena();
        PoolManager::FreeArena(cur);
        cur = tmp;
    }
}

template <bool USE_OOM_HANDLER>
inline void *ArenaAllocatorT<USE_OOM_HANDLER>::AllocateAndAddNewPool(size_t size, Alignment alignment)
{
    ASSERT(arenas_ != nullptr);
    void *mem = arenas_->Alloc(size, alignment);
    if (mem == nullptr) {
        bool addNewPool = false;
        if (limitAllocSizeByPool_) {
            addNewPool = AddArenaFromPool(std::max(AlignUp(size, alignment) + sizeof(Arena), DEFAULT_ARENA_SIZE));
        } else {
            addNewPool = AddArenaFromPool(DEFAULT_ARENA_SIZE);
        }
        if (UNLIKELY(!addNewPool)) {
            LOG(DEBUG, ALLOC) << "Can not add new pool for " << SpaceTypeToString(spaceType_);
            return nullptr;
        }
        mem = arenas_->Alloc(size, alignment);
        ASSERT(!limitAllocSizeByPool_ || mem != nullptr);
    }
    return mem;
}

template <bool USE_OOM_HANDLER>
void *ArenaAllocatorT<USE_OOM_HANDLER>::Alloc(size_t size, Alignment align)
{
    trace::ScopedTrace scopedTrace("ArenaAllocator allocate");
    LOG(DEBUG, ALLOC) << "ArenaAllocator: try to alloc " << size << " with align " << align;
    void *ret = nullptr;
    if (ON_STACK_ALLOCATION_ENABLED && UNLIKELY(!arenas_)) {
        LOG(DEBUG, ALLOC) << "\tTry to allocate from stack";
        ret = buff_.Alloc(size, align);
        LOG_IF(ret, DEBUG, ALLOC) << "\tallocate from stack buffer";
        if (ret == nullptr) {
            ret = AllocateAndAddNewPool(size, align);
        }
    } else {
        ret = AllocateAndAddNewPool(size, align);
    }
    // NOLINTNEXTLINE(readability-braces-around-statements, bugprone-suspicious-semicolon)
    if constexpr (USE_OOM_HANDLER) {
        if (ret == nullptr) {
            oomHandler_();
        }
    }
    LOG(DEBUG, ALLOC) << "ArenaAllocator: allocated " << size << " bytes aligned by " << align;
    AllocArenaMemStats(size);
    return ret;
}

template <bool USE_OOM_HANDLER>
void ArenaAllocatorT<USE_OOM_HANDLER>::Resize(size_t newSize)
{
    LOG(DEBUG, ALLOC) << "ArenaAllocator: resize to new size " << newSize;
    // NOTE(aemelenko): we have O(2n) here in the worst case
    size_t curSize = GetAllocatedSize();
    if (curSize <= newSize) {
        LOG_IF(curSize < newSize, FATAL, ALLOC) << "ArenaAllocator: resize to bigger size than we have. Do nothing";
        return;
    }

    size_t bytesToDelete = curSize - newSize;

    // Try to delete unused arenas
    while ((arenas_ != nullptr) && (bytesToDelete != 0)) {
        Arena *next = arenas_->GetNextArena();
        size_t curArenaSize = arenas_->GetOccupiedSize();
        if (curArenaSize < bytesToDelete) {
            // We need to free the whole arena
            PoolManager::FreeArena(arenas_);
            arenas_ = next;
            bytesToDelete -= curArenaSize;
        } else {
            arenas_->Resize(curArenaSize - bytesToDelete);
            bytesToDelete = 0;
        }
    }
    if ((ON_STACK_ALLOCATION_ENABLED) && (bytesToDelete > 0)) {
        size_t stackSize = buff_.GetOccupiedSize();
        ASSERT(stackSize >= bytesToDelete);
        buff_.Resize(stackSize - bytesToDelete);
        bytesToDelete = 0;
    }
    ASSERT(bytesToDelete == 0);
}

template <bool USE_OOM_HANDLER>
bool ArenaAllocatorT<USE_OOM_HANDLER>::AddArenaFromPool(size_t poolSize)
{
    ASSERT(poolSize != 0);
    poolSize = AlignUp(poolSize, PANDA_POOL_ALIGNMENT_IN_BYTES);
    Arena *newArena = PoolManager::AllocArena(poolSize, spaceType_, GetAllocatorType(), this);
    if (UNLIKELY(newArena == nullptr)) {
        return false;
    }
    newArena->LinkTo(arenas_);
    arenas_ = newArena;
    return true;
}

template <bool USE_OOM_HANDLER>
size_t ArenaAllocatorT<USE_OOM_HANDLER>::GetAllocatedSize() const
{
    size_t size = 0;
    if (ON_STACK_ALLOCATION_ENABLED) {
        size += buff_.GetOccupiedSize();
    }
    for (Arena *cur = arenas_; cur != nullptr; cur = cur->GetNextArena()) {
        size += cur->GetOccupiedSize();
    }
    return size;
}

template class ArenaAllocatorT<true>;
template class ArenaAllocatorT<false>;

}  // namespace ark
