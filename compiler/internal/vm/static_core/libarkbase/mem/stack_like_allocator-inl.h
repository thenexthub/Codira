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
#ifndef PANDA_LIBPANDABASE_MEM_STACK_LIKE_ALLOCATOR_INL_H
#define PANDA_LIBPANDABASE_MEM_STACK_LIKE_ALLOCATOR_INL_H

#include <securec.h>
#include "libarkbase/mem/stack_like_allocator.h"
#include "libarkbase/utils/logger.h"
#include "libarkbase/utils/asan_interface.h"

namespace ark::mem {
// NOLINTNEXTLINE(cppcoreguidelines-macro-usage)
#define LOG_STACK_LIKE_ALLOCATOR(level) LOG(level, ALLOC) << "StackLikeAllocator: "

template <Alignment ALIGNMENT, size_t MAX_SIZE>
// CC-OFFNXT(G.FUD.06) Splitting this function will degrade readability. Keyword "inline" needs to satisfy ODR rule.
inline StackLikeAllocator<ALIGNMENT, MAX_SIZE>::StackLikeAllocator(bool usePoolManager, SpaceType spaceType)
    : usePoolManager_(usePoolManager)
{
    LOG_STACK_LIKE_ALLOCATOR(DEBUG) << "Initializing of StackLikeAllocator";
    ASSERT(RELEASE_PAGES_SIZE == AlignUp(RELEASE_PAGES_SIZE, os::mem::GetPageSize()));
    // MMAP!
    if (usePoolManager_) {
        // clang-format off
        startAddr_ = PoolManager::GetMmapMemPool()
            ->AllocPool(MAX_SIZE, spaceType, AllocatorType::STACK_LIKE_ALLOCATOR, this).GetMem();
        // clang-format on
    } else {
        startAddr_ = ark::os::mem::MapRWAnonymousWithAlignmentRaw(
            MAX_SIZE, std::max(GetAlignmentInBytes(ALIGNMENT), static_cast<size_t>(ark::os::mem::GetPageSize())));
    }
    if (startAddr_ == nullptr) {
        LOG_STACK_LIKE_ALLOCATOR(FATAL) << "Can't get initial memory";
    }
    freePointer_ = startAddr_;
    endAddr_ = ToVoidPtr(ToUintPtr(startAddr_) + MAX_SIZE);
    allocatedEndAddr_ = endAddr_;
    ASSERT(AlignUp(ToUintPtr(freePointer_), GetAlignmentInBytes(ALIGNMENT)) == ToUintPtr(freePointer_));
    LOG_STACK_LIKE_ALLOCATOR(DEBUG) << "Initializing of StackLikeAllocator finished";
}

template <Alignment ALIGNMENT, size_t MAX_SIZE>
inline StackLikeAllocator<ALIGNMENT, MAX_SIZE>::~StackLikeAllocator()
{
    LOG_STACK_LIKE_ALLOCATOR(DEBUG) << "Destroying of StackLikeAllocator";
    if (usePoolManager_) {
        PoolManager::GetMmapMemPool()->FreePool(startAddr_, MAX_SIZE);
    } else {
        ark::os::mem::UnmapRaw(startAddr_, MAX_SIZE);
    }
    LOG_STACK_LIKE_ALLOCATOR(DEBUG) << "Destroying of StackLikeAllocator finished";
}

template <Alignment ALIGNMENT, size_t MAX_SIZE>
template <bool USE_MEMSET>
// CC-OFFNXT(G.FUD.06) perf critical, ODR
inline void *StackLikeAllocator<ALIGNMENT, MAX_SIZE>::Alloc(size_t size)
{
    ASSERT(AlignUp(size, GetAlignmentInBytes(ALIGNMENT)) == size);

    void *ret = nullptr;
    uintptr_t newCurPos = ToUintPtr(freePointer_) + size;
    if (LIKELY(newCurPos <= ToUintPtr(endAddr_))) {
        ret = freePointer_;
        freePointer_ = ToVoidPtr(newCurPos);
        ASAN_UNPOISON_MEMORY_REGION(ret, size);
    } else {
        return nullptr;
    }

    ASSERT(AlignUp(ToUintPtr(ret), GetAlignmentInBytes(ALIGNMENT)) == ToUintPtr(ret));
    // NOLINTNEXTLINE(readability-braces-around-statements, bugprone-suspicious-semicolon)
    if constexpr (USE_MEMSET) {
        memset_s(ret, size, 0x00, size);
    }
    LOG_STACK_LIKE_ALLOCATOR(DEBUG) << "Allocated memory at addr " << std::hex << ret;
    return ret;
}

template <Alignment ALIGNMENT, size_t MAX_SIZE>
// CC-OFFNXT(G.FUD.06) perf critical, ODR
inline void StackLikeAllocator<ALIGNMENT, MAX_SIZE>::Free(void *mem)
{
    ASSERT(ToUintPtr(mem) == AlignUp(ToUintPtr(mem), GetAlignmentInBytes(ALIGNMENT)));
    ASSERT(Contains(mem));
    if ((ToUintPtr(mem) >> RELEASE_PAGES_SHIFT) != (ToUintPtr(freePointer_) >> RELEASE_PAGES_SHIFT)) {
        // Start address from which we can release pages
        uintptr_t startAddr = AlignUp(ToUintPtr(mem), RELEASE_PAGES_SIZE);
        // We do page release calls each RELEASE_PAGES_SIZE interval,
        // Therefore, we should clear the last RELEASE_PAGES_SIZE interval
        uintptr_t endAddr = AlignUp(ToUintPtr(freePointer_), RELEASE_PAGES_SIZE);
        os::mem::ReleasePages(startAddr, endAddr);
        LOG_STACK_LIKE_ALLOCATOR(DEBUG) << "Release " << std::dec << endAddr - startAddr
                                        << " memory bytes in interval [" << std::hex << startAddr << "; " << endAddr
                                        << "]";
    }
    ASAN_POISON_MEMORY_REGION(mem, ToUintPtr(freePointer_) - ToUintPtr(mem));
    freePointer_ = mem;
    LOG_STACK_LIKE_ALLOCATOR(DEBUG) << "Free memory at addr " << std::hex << mem;
}

template <Alignment ALIGNMENT, size_t MAX_SIZE>
inline bool StackLikeAllocator<ALIGNMENT, MAX_SIZE>::Contains(void *mem)
{
    return (ToUintPtr(mem) >= ToUintPtr(startAddr_)) && (ToUintPtr(mem) < ToUintPtr(freePointer_));
}

#undef LOG_STACK_LIKE_ALLOCATOR
}  // namespace ark::mem

#endif  // PANDA_LIBPANDABASE_MEM_STACK_LIKE_ALLOCATOR_INL_H
