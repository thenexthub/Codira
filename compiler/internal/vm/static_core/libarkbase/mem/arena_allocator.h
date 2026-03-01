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

#ifndef LIBPANDABASE_MEM_ARENA_ALLOCATOR_H
#define LIBPANDABASE_MEM_ARENA_ALLOCATOR_H

#include <array>
#include <cstdint>
#include <cstdlib>
#include <limits>
#include <utility>
#include <memory>

#include "libarkbase/concepts.h"
#include "libarkbase/mem/base_mem_stats.h"
#include "malloc_mem_pool-inl.h"
#include "mmap_mem_pool-inl.h"
#include "mem.h"
#include "mem_pool.h"
#include "arena-inl.h"

#define USE_MMAP_POOL_FOR_ARENAS

WEAK_FOR_LTO_START

namespace ark {

constexpr size_t DEFAULT_ARENA_SIZE = PANDA_DEFAULT_ARENA_SIZE;
constexpr Alignment DEFAULT_ARENA_ALIGNMENT = LOG_ALIGN_3;
// Buffer for on stack allocation
constexpr size_t ON_STACK_BUFFER_SIZE = 128 * SIZE_1K;
#ifdef FORCE_ARENA_ALLOCATOR_ON_STACK_CACHE
constexpr bool ON_STACK_ALLOCATION_ENABLED = true;
#else
constexpr bool ON_STACK_ALLOCATION_ENABLED = false;
#endif

constexpr size_t DEFAULT_ON_STACK_ARENA_ALLOCATOR_BUFF_SIZE = 128 * SIZE_1K;

template <typename T, bool USE_OOM_HANDLER>
class ArenaAllocatorAdapter;

template <bool USE_OOM_HANDLER = false>
class ArenaAllocatorT {
public:
    using OOMHandler = std::add_pointer_t<void()>;
    template <typename T>
    using AdapterType = ArenaAllocatorAdapter<T, USE_OOM_HANDLER>;

    PANDA_PUBLIC_API explicit ArenaAllocatorT(SpaceType spaceType, BaseMemStats *memStats = nullptr,
                                              bool limitAllocSizeByPool = false);
    ArenaAllocatorT(OOMHandler oomHandler, SpaceType spaceType, BaseMemStats *memStats = nullptr,
                    bool limitAllocSizeByPool = false);

    PANDA_PUBLIC_API ~ArenaAllocatorT();
    NO_COPY_SEMANTIC(ArenaAllocatorT);
    NO_MOVE_SEMANTIC(ArenaAllocatorT);

    [[nodiscard]] PANDA_PUBLIC_API void *Alloc(size_t size, Alignment align = DEFAULT_ARENA_ALIGNMENT);

    template <typename T, typename... Args>
    [[nodiscard]] std::enable_if_t<!std::is_array_v<T>, T *> New(Args &&...args)
    {
        auto p = reinterpret_cast<void *>(Alloc(sizeof(T)));
        if (UNLIKELY(p == nullptr)) {
            return nullptr;
        }
        new (p) T(std::forward<Args>(args)...);
        return reinterpret_cast<T *>(p);
    }

    template <typename T>
    [[nodiscard]] std::enable_if_t<is_unbounded_array_v<T>, std::remove_extent_t<T> *> New(size_t size)
    {
        static constexpr size_t SIZE_BEFORE_DATA_OFFSET =
            AlignUp(sizeof(size_t), GetAlignmentInBytes(DEFAULT_ARENA_ALIGNMENT));
        using ElementType = std::remove_extent_t<T>;
        void *p = Alloc(SIZE_BEFORE_DATA_OFFSET + sizeof(ElementType) * size);
        if (UNLIKELY(p == nullptr)) {
            return nullptr;
        }
        *static_cast<size_t *>(p) = size;
        auto *data = ToNativePtr<ElementType>(ToUintPtr(p) + SIZE_BEFORE_DATA_OFFSET);
        ElementType *currentElement = data;
        // NOLINTNEXTLINE(cppcoreguidelines-pro-bounds-pointer-arithmetic)
        for (size_t i = 0; i < size; ++i, ++currentElement) {
            new (currentElement) ElementType();
        }
        return data;
    }

    template <typename T>
    [[nodiscard]] T *AllocArray(size_t arrLength);

    ArenaAllocatorAdapter<void, USE_OOM_HANDLER> Adapter();

    PANDA_PUBLIC_API size_t GetAllocatedSize() const;

    /**
     * @brief Set the size of allocated memory to @param new_size.
     *  Free all memory that exceeds @param new_size bytes in the allocator.
     */
    PANDA_PUBLIC_API void Resize(size_t newSize);

    static constexpr AllocatorType GetAllocatorType()
    {
        return AllocatorType::ARENA_ALLOCATOR;
    }

protected:
    Arena *arenas_ = nullptr;  // NOLINT(misc-non-private-member-variables-in-classes)

private:
    template <bool USE_ON_STACK_BUFF, typename DummyArg = void>
    class OnStackBuffT {
    public:
        void *Alloc(size_t size, Alignment align = DEFAULT_ARENA_ALIGNMENT)
        {
            size_t freeSize = GetFreeSize();
            void *newPos = curPos_;
            void *ret = std::align(GetAlignmentInBytes(align), size, newPos, freeSize);
            if (ret != nullptr) {
                curPos_ = static_cast<char *>(ToVoidPtr(ToUintPtr(ret) + size));
            }
            return ret;
        }

        size_t GetFreeSize() const
        {
            return DEFAULT_ON_STACK_ARENA_ALLOCATOR_BUFF_SIZE - (curPos_ - &buff_[0]);
        }

        size_t GetOccupiedSize() const
        {
            return curPos_ - &buff_[0];
        }

        void Resize(size_t newSize)
        {
            ASSERT(newSize <= GetOccupiedSize());
            curPos_ = static_cast<char *>(ToVoidPtr(ToUintPtr(&buff_[0]) + newSize));
        }

    private:
        std::array<char, ON_STACK_BUFFER_SIZE> buff_ {0};
        char *curPos_ = &buff_[0];
    };

    template <typename DummyArg>
    class OnStackBuffT<false, DummyArg> {
    public:
        void *Alloc([[maybe_unused]] size_t size, [[maybe_unused]] Alignment align = DEFAULT_ARENA_ALIGNMENT)
        {
            return nullptr;
        }

        size_t GetOccupiedSize() const
        {
            return 0;
        }

        void Resize(size_t newSize)
        {
            (void)newSize;
        }
    };

    /**
     * @brief Adds Arena from MallocMemPool and links it to active
     * @param pool_size size of new pool
     */
    bool AddArenaFromPool(size_t poolSize);

    /**
     * @brief Allocate new element.
     * Try to allocate new element at current arena or try to add new pool to this allocator and allocate element at new
     * pool
     * @param size new element size
     * @param alignment alignment of new element address
     */
    [[nodiscard]] void *AllocateAndAddNewPool(size_t size, Alignment alignment);

    inline void AllocArenaMemStats(size_t size)
    {
        if (memStats_ != nullptr) {
            memStats_->RecordAllocateRaw(size, spaceType_);
        }
    }

    using OnStackBuff = OnStackBuffT<ON_STACK_ALLOCATION_ENABLED>;
    OnStackBuff buff_;
    BaseMemStats *memStats_;
    SpaceType spaceType_;
    OOMHandler oomHandler_ {nullptr};
    bool limitAllocSizeByPool_ {false};
};

using ArenaAllocator = ArenaAllocatorT<false>;
using ArenaAllocatorWithOOMHandler = ArenaAllocatorT<true>;

template <bool USE_OOM_HANDLER>
class ArenaResizeWrapper {
public:
    explicit ArenaResizeWrapper(ArenaAllocatorT<USE_OOM_HANDLER> *arenaAllocator)
        : oldSize_(arenaAllocator->GetAllocatedSize()), allocator_(arenaAllocator)
    {
    }

    ~ArenaResizeWrapper()
    {
        allocator_->Resize(oldSize_);
    }

private:
    size_t oldSize_;
    ArenaAllocatorT<USE_OOM_HANDLER> *allocator_;

    NO_COPY_SEMANTIC(ArenaResizeWrapper);
    NO_MOVE_SEMANTIC(ArenaResizeWrapper);
};

template <bool USE_OOM_HANDLER>
template <typename T>
T *ArenaAllocatorT<USE_OOM_HANDLER>::AllocArray(size_t arrLength)
{
    // NOTE(Dmitrii Trubenkov): change to the proper implementation
    return static_cast<T *>(Alloc(sizeof(T) * arrLength));
}

}  // namespace ark

WEAK_FOR_LTO_END

#endif  // LIBPANDABASE_MEM_ARENA_ALLOCATOR_H
