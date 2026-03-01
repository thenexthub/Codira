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
#ifndef PANDA_LIBPANDABASE_MEM_STACK_LIKE_ALLOCATOR_H
#define PANDA_LIBPANDABASE_MEM_STACK_LIKE_ALLOCATOR_H

#include "libarkbase/mem/mem.h"
#include "libarkbase/mem/pool_map.h"

namespace ark::mem {
// Note: we only have 4GB of memory on arm32, so we need to limit the max size of the stack like allocator
// details can be found in #26461
#ifdef PANDA_TARGET_ARM32
static constexpr size_t STACK_LIKE_ALLOCATOR_DEFAUL_MAX_SIZE = 2_MB;
#else
static constexpr size_t STACK_LIKE_ALLOCATOR_DEFAUL_MAX_SIZE = 48_MB;
#endif

//                                          Allocation flow looks like that:
//
//  1. Allocate big memory piece via mmap.
//  2. Allocate/Free memory in this preallocated memory piece.
//  3. Return nullptr if we reached the limit of created memory piece.

template <Alignment ALIGNMENT = DEFAULT_FRAME_ALIGNMENT, size_t MAX_SIZE = STACK_LIKE_ALLOCATOR_DEFAUL_MAX_SIZE>
class StackLikeAllocator {
public:
    explicit StackLikeAllocator(bool usePoolManager = true, SpaceType spaceType = SpaceType::SPACE_TYPE_FRAMES);
    ~StackLikeAllocator();
    NO_MOVE_SEMANTIC(StackLikeAllocator);
    NO_COPY_SEMANTIC(StackLikeAllocator);

    template <bool USE_MEMSET = true>
    [[nodiscard]] void *Alloc(size_t size);

    void Free(void *mem);

    /// @brief Returns true if address inside current allocator.
    bool Contains(void *mem);

    static constexpr AllocatorType GetAllocatorType()
    {
        return AllocatorType::STACK_LIKE_ALLOCATOR;
    }

    static constexpr uint32_t GetFreePointerOffset()
    {
        return MEMBER_OFFSET(StackLikeAllocator, freePointer_);
    }

    static constexpr uint32_t GetEndAddrOffset()
    {
        return MEMBER_OFFSET(StackLikeAllocator, endAddr_);
    }

    size_t GetAllocatedSize() const
    {
        ASSERT(ToUintPtr(freePointer_) >= ToUintPtr(startAddr_));
        return ToUintPtr(freePointer_) - ToUintPtr(startAddr_);
    }

    void SetReservedMemorySize(size_t size)
    {
        ASSERT(GetFullMemorySize() >= size);
        reservedEndAddr_ = ToVoidPtr(ToUintPtr(startAddr_) + size);
    }

    void UseWholeMemory()
    {
        endAddr_ = allocatedEndAddr_;
    }

    void ReserveMemory()
    {
        ASSERT(reservedEndAddr_ != nullptr);
        endAddr_ = reservedEndAddr_;
    }

    size_t GetFullMemorySize() const
    {
        return ToUintPtr(allocatedEndAddr_) - ToUintPtr(startAddr_);
    }

    static constexpr size_t RELEASE_PAGES_SHIFT = 18;

private:
    static constexpr size_t RELEASE_PAGES_SIZE = 256_KB;
    static_assert(RELEASE_PAGES_SIZE == (1U << RELEASE_PAGES_SHIFT));
    static_assert(MAX_SIZE % GetAlignmentInBytes(ALIGNMENT) == 0);
    void *startAddr_ {nullptr};
    void *endAddr_ {nullptr};
    void *freePointer_ {nullptr};
    bool usePoolManager_ {false};
    void *reservedEndAddr_ {nullptr};
    void *allocatedEndAddr_ {nullptr};
};
}  // namespace ark::mem

#endif  // PANDA_LIBPANDABASE_MEM_STACK_LIKE_ALLOCATOR_H
