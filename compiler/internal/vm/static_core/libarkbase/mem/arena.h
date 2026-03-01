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

#ifndef LIBPANDABASE_MEM_ARENA_H
#define LIBPANDABASE_MEM_ARENA_H

#include "mem.h"

WEAK_FOR_LTO_START

namespace ark {

constexpr size_t ARENA_DEFAULT_SIZE = 1_MB;
constexpr Alignment ARENA_DEFAULT_ALIGNMENT = DEFAULT_ALIGNMENT;

class Arena {
public:
    Arena(size_t buffSize, void *buff);
    virtual ~Arena();
    DEFAULT_MOVE_SEMANTIC(Arena);
    DEFAULT_COPY_SEMANTIC(Arena);

    /**
     * @brief Allocates memory with size @param size and aligned with @param alignment
     * @param size - size of the allocated memory
     * @param alignment - alignment of the allocated memory
     * @return pointer to the allocated memory on success, or nullptr on fail
     */
    void *Alloc(size_t size, Alignment alignment = ARENA_DEFAULT_ALIGNMENT);

    /**
     * @brief Links this Arena to the @param arena
     * @param arena - Arena which will be linked as next to the current
     */
    void LinkTo(Arena *arena);

    /// @brief Clear link to the next arena
    void ClearNextLink();

    /**
     * @brief Returns next linked Arena
     * @return next linked Arena or nullptr
     */
    Arena *GetNextArena() const;

    /// @return Size of free area in the arena
    size_t GetFreeSize() const;

    /// @return Size of an occupied area in the arena
    size_t GetOccupiedSize() const;

    /// @return A pointer to the first byte not in the arena
    void *GetArenaEnd() const;

    /// @return A pointer to the first not allocated byte
    void *GetAllocatedEnd() const;

    /// @return A pointer to the first allocated byte
    void *GetAllocatedStart() const;

    /// @return A pointer to the raw memory inside arena
    void *GetMem() const
    {
        return buff_;
    }

    void *GetTop() const
    {
        return curPos_;
    }

    size_t GetSize() const
    {
        return size_;
    }

    /**
     * @brief Check that @param mem is stored inside this Arena
     * @return true on success, or false on fail
     */
    bool InArena(const void *mem) const;

    /// @brief Mark all memory after @param mem as free. Check that @param mem is stored inside this arena.
    void Free(void *mem);

    /// @brief Set occupied memory size to @param new_size.
    void Resize(size_t newSize);

    /// @brief empties arena
    void Reset();

    /**
     * @brief Expand arena. The new memory must be located just after the current buffer.
     * @param extra_buff - pointer to the extra buffer located just after the current.
     * @param size - the size of the extra buffer
     */
    void ExpandArena(const void *extraBuff, size_t size);

protected:
    Arena(size_t buffSize, void *buff, Alignment startAlignment);
    /**
     * @brief Fast allocates memory with size @param size
     * @param size - size of the allocated memory, must be @param alignment aligned
     * @param alignment - alignment of the allocated memory, used only for debug
     * @return pointer to the allocated memory on success, or nullptr on fail
     */
    void *AlignedAlloc(size_t size, Alignment alignment);

    void *GetStartPos() const
    {
        return startPos_;
    }

private:
    Arena *next_ = nullptr;
    void *buff_ = nullptr;
    void *startPos_ = nullptr;
    void *curPos_ = nullptr;
    size_t size_ = 0;
};

template <Alignment ALIGNMENT_T>
class AlignedArena : public Arena {
public:
    AlignedArena(size_t buffSize, void *buff) : Arena(buffSize, buff, ALIGNMENT_T) {}

    ~AlignedArena() override = default;

    void *Alloc(size_t size)
    {
        return Arena::AlignedAlloc(size, ALIGNMENT_T);
    }

private:
    DEFAULT_MOVE_SEMANTIC(AlignedArena);
    DEFAULT_COPY_SEMANTIC(AlignedArena);
};

template <Alignment ALIGNMENT_T>
class DoubleLinkedAlignedArena : public AlignedArena<ALIGNMENT_T> {
public:
    DoubleLinkedAlignedArena(size_t buffSize, void *buff) : AlignedArena<ALIGNMENT_T>(buffSize, buff) {}

    /**
     * @brief Links this Arena to the next @param arena
     * @param arena - Arena which will be linked as next to the current
     */
    void LinkNext(DoubleLinkedAlignedArena *arena)
    {
        Arena::LinkTo(static_cast<Arena *>(arena));
    }

    /**
     * @brief Links this Arena to the prev @param arena
     * @param arena - Arena which will be linked as prev to the current
     */
    void LinkPrev(DoubleLinkedAlignedArena *arena)
    {
        ASSERT(prev_ == nullptr);
        prev_ = arena;
    }

    /**
     * @brief Returns next linked Arena
     * @return next linked Arena or nullptr
     */
    DoubleLinkedAlignedArena *GetNextArena() const
    {
        return static_cast<DoubleLinkedAlignedArena *>(Arena::GetNextArena());
    }

    /**
     * @brief Returns prev linked Arena
     * @return prev linked Arena or nullptr
     */
    DoubleLinkedAlignedArena *GetPrevArena() const
    {
        return prev_;
    }

    /// @brief Clear link to the prev arena
    void ClearPrevLink()
    {
        prev_ = nullptr;
    }

    ~DoubleLinkedAlignedArena() override = default;

    DEFAULT_MOVE_SEMANTIC(DoubleLinkedAlignedArena);
    DEFAULT_COPY_SEMANTIC(DoubleLinkedAlignedArena);

private:
    DoubleLinkedAlignedArena *prev_ = nullptr;
};

}  // namespace ark

WEAK_FOR_LTO_END

#endif  // LIBPANDABASE_MEM_ARENA_H
