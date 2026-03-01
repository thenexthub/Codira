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
#ifndef PANDA_MEM_FREELIST_ALLOCATOR_H
#define PANDA_MEM_FREELIST_ALLOCATOR_H

#include <array>

#include "libarkbase/mem/mem.h"
#include "libarkbase/mem/space.h"
#include "libarkbase/os/mutex.h"
#include "runtime/mem/freelist.h"
#include "runtime/mem/runslots.h"
#include "runtime/mem/lock_config_helper.h"

namespace ark::mem {

// NOTE(aemelenko): Move this constants to compile options
// Minimal size of this allocator is a max size of RunSlots allocator.
static constexpr size_t PANDA_FREELIST_ALLOCATOR_MIN_SIZE = RunSlots<>::MaxSlotSize();
static constexpr size_t PANDA_FREELIST_ALLOCATOR_SEGREGATED_LIST_SIZE = 16;
static constexpr bool PANDA_FREELIST_ALLOCATOR_SEGREGATED_LIST_FAST_INSERT = false;
static constexpr bool PANDA_FREELIST_ALLOCATOR_SEGREGATED_LIST_FAST_EXTRACT = false;

static constexpr Alignment FREELIST_DEFAULT_ALIGNMENT = DEFAULT_ALIGNMENT;

static constexpr size_t FREELIST_ALLOCATOR_MIN_SIZE = PANDA_FREELIST_ALLOCATOR_MIN_SIZE;
static_assert(FREELIST_ALLOCATOR_MIN_SIZE >= (sizeof(freelist::FreeListHeader) - sizeof(freelist::MemoryBlockHeader)));

class FreeListAllocatorLockConfig {
public:
    using CommonLock = os::memory::RWLock;
    using DummyLock = os::memory::DummyLock;

    template <MTModeT MT_MODE>
    using ParameterizedLock = typename LockConfigHelper<FreeListAllocatorLockConfig, MT_MODE>::Value;
};

template <typename T, typename AllocConfigT, typename LockConfigT>
class FreeListAllocatorAdapter;
enum class InternalAllocatorConfig;
template <InternalAllocatorConfig CONFIG>
class InternalAllocator;

//                                             FreeList Allocator layout:
//
//     |..........|xxxxxxxxxx|..........|........|00000000|..........|xxxxxxxxxx|..........|........|0000000000000000|
//     |..........|xxxxxxxxxx|..........|..Links.|00000000|..........|xxxxxxxxxx|..........|..Links.|0000000000000000|
//     |..Memory..|xOCCUPIEDx|..Memory..|...on...|00FREE00|..Memory..|xOCCUPIEDx|..Memory..|...on...|000000FREE000000|
//     |..Header..|xxMEMORYxx|..Header..|..next/.|0MEMORY0|..Header..|xxMEMORYxx|..Header..|..next/.|00000MEMORY00000|
//     |..........|xxxxxxxxxx|..........|..prev..|00000000|..........|xxxxxxxxxx|..........|..prev..|0000000000000000|
//     |..........|xxxxxxxxxx|..........|..free..|00000000|..........|xxxxxxxxxx|..........|..free..|0000000000000000|
//
//                        Blocks with alignments:
// 1) Padding header stored just after the main block header:
//     |..........||..........||xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx|
//     |..........||..........||xxxxxxxxxxALIGNEDxxxxxxxxxxxxxx|
//     |..Memory..||.Padding..||xxxxxxxxxxOCCUPIEDxxxxxxxxxxxxx|
//     |..Header..||..Header..||xxxxxxxxxxxMEMORYxxxxxxxxxxxxxx|
//     |..........||..........||xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx|
//     |..........||..........||xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx|
//
// 2) We have padding size after the main block header:
//     |..........|........|--------|..........||xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx|
//     |..........|........|--------|..........||xxxxxxxxxxALIGNEDxxxxxxxxxxxxxx|
//     |..Memory..|.Padding|--------|.Padding..||xxxxxxxxxxOCCUPIEDxxxxxxxxxxxxx|
//     |..Header..|..Size..|--------|..Header..||xxxxxxxxxxxMEMORYxxxxxxxxxxxxxx|
//     |..........|........|--------|..........||xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx|
//     |..........|........|--------|..........||xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx|

template <typename AllocConfigT, typename LockConfigT = FreeListAllocatorLockConfig::CommonLock>
class FreeListAllocator {
public:
    explicit FreeListAllocator(MemStatsType *memStats, SpaceType typeAllocation = SpaceType::SPACE_TYPE_OBJECT);
    ~FreeListAllocator();
    NO_COPY_SEMANTIC(FreeListAllocator);
    NO_MOVE_SEMANTIC(FreeListAllocator);

    template <typename T, typename... Args>
    [[nodiscard]] T *New(Args &&...args)
    {
        auto p = reinterpret_cast<void *>(Alloc(sizeof(T)));
        new (p) T(std::forward<Args>(args)...);
        return reinterpret_cast<T *>(p);
    }

    template <typename T>
    [[nodiscard]] T *AllocArray(size_t arrLength);

    template <bool NEED_LOCK = true>
    [[nodiscard]] void *Alloc(size_t size, Alignment align = FREELIST_DEFAULT_ALIGNMENT);

    void Free(void *mem);

    void Collect(const GCObjectVisitor &deathCheckerFn);

    bool AddMemoryPool(void *mem, size_t size);

    /**
     * @brief Iterates over all objects allocated by this allocator.
     * @tparam MemVisitor
     * @param object_visitor - function pointer or functor
     */
    template <typename ObjectVisitor>
    void IterateOverObjects(const ObjectVisitor &objectVisitor);

    /**
     * @brief Iterates over all memory pools used by this allocator
     * and remove them from the allocator structure.
     * NOTE: This method can't be used to clear all internal allocator
     * information and reuse the allocator somewhere else.
     * @tparam MemVisitor
     * @param mem_visitor - function pointer or functor
     */
    template <typename MemVisitor>
    void VisitAndRemoveAllPools(const MemVisitor &memVisitor);

    /**
     * @brief Visit memory pools that can be returned to the system in this allocator
     * and remove them from the allocator structure.
     * @tparam MemVisitor
     * @param mem_visitor - function pointer or functor
     */
    template <typename MemVisitor>
    void VisitAndRemoveFreePools(const MemVisitor &memVisitor);

    /**
     * @brief Iterates over objects in the range inclusively.
     * @tparam MemVisitor
     * @param mem_visitor - function pointer or functor
     * @param left_border - a pointer to the first byte of the range
     * @param right_border - a pointer to the last byte of the range
     */
    template <typename MemVisitor>
    void IterateOverObjectsInRange(const MemVisitor &memVisitor, void *leftBorder, void *rightBorder);

    FreeListAllocatorAdapter<void, AllocConfigT, LockConfigT> Adapter();

    /**
     * @brief returns maximum size which can be allocated by this allocator
     * @return
     */
    static constexpr size_t GetMaxSize()
    {
        return FREELIST_MAX_ALLOC_SIZE;
    }

    /**
     * @brief returns minimum pool size which can be added to this allocator
     * @return
     */
    static constexpr size_t GetMinPoolSize()
    {
        return FREELIST_DEFAULT_MEMORY_POOL_SIZE;
    }

    static constexpr size_t PoolAlign()
    {
        return sizeof(MemoryBlockHeader);
    }

    bool ContainObject(const ObjectHeader *obj);

    bool IsLive(const ObjectHeader *obj);

    static constexpr AllocatorType GetAllocatorType()
    {
        return AllocatorType::FREELIST_ALLOCATOR;
    }

    double CalculateExternalFragmentation();

#ifdef PANDA_MEASURE_FRAGMENTATION
    size_t GetAllocatedBytes() const
    {
        // Atomic with relaxed order reason: order is not required
        return allocatedBytes_.load(std::memory_order_relaxed);
    }
#endif

private:
    using MemoryBlockHeader = ark::mem::freelist::MemoryBlockHeader;
    using FreeListHeader = ark::mem::freelist::FreeListHeader;

    class alignas(sizeof(MemoryBlockHeader)) MemoryPoolHeader {
    public:
        void Initialize(size_t size, MemoryPoolHeader *prev, MemoryPoolHeader *next);

        ATTRIBUTE_NO_SANITIZE_ADDRESS
        MemoryPoolHeader *GetPrev()
        {
            ASAN_UNPOISON_MEMORY_REGION(this, sizeof(MemoryPoolHeader));
            MemoryPoolHeader *prev = prev_;
            ASAN_POISON_MEMORY_REGION(this, sizeof(MemoryPoolHeader));
            return prev;
        }

        ATTRIBUTE_NO_SANITIZE_ADDRESS
        MemoryPoolHeader *GetNext()
        {
            ASAN_UNPOISON_MEMORY_REGION(this, sizeof(MemoryPoolHeader));
            MemoryPoolHeader *next = next_;
            ASAN_POISON_MEMORY_REGION(this, sizeof(MemoryPoolHeader));
            return next;
        }

        ATTRIBUTE_NO_SANITIZE_ADDRESS
        void SetPrev(MemoryPoolHeader *prev)
        {
            ASAN_UNPOISON_MEMORY_REGION(this, sizeof(MemoryPoolHeader));
            prev_ = prev;
            ASAN_POISON_MEMORY_REGION(this, sizeof(MemoryPoolHeader));
        }

        ATTRIBUTE_NO_SANITIZE_ADDRESS
        void SetNext(MemoryPoolHeader *next)
        {
            ASAN_UNPOISON_MEMORY_REGION(this, sizeof(MemoryPoolHeader));
            next_ = next;
            ASAN_POISON_MEMORY_REGION(this, sizeof(MemoryPoolHeader));
        }

        ATTRIBUTE_NO_SANITIZE_ADDRESS
        MemoryBlockHeader *GetFirstMemoryHeader()
        {
            return static_cast<MemoryBlockHeader *>(ToVoidPtr(ToUintPtr(this) + sizeof(MemoryPoolHeader)));
        }

        ATTRIBUTE_NO_SANITIZE_ADDRESS
        size_t GetSize()
        {
            ASAN_UNPOISON_MEMORY_REGION(this, sizeof(MemoryPoolHeader));
            size_t size = size_;
            ASAN_POISON_MEMORY_REGION(this, sizeof(MemoryPoolHeader));
            return size;
        }

    private:
        MemoryPoolHeader *prev_ {nullptr};
        MemoryPoolHeader *next_ {nullptr};
        size_t size_ {0};
    };

    static_assert((sizeof(MemoryPoolHeader) % sizeof(MemoryBlockHeader)) == 0);
    static constexpr size_t FREELIST_DEFAULT_MEMORY_POOL_SIZE = PANDA_DEFAULT_ALLOCATOR_POOL_SIZE;
    static constexpr size_t FREELIST_MAX_ALLOC_SIZE =
        ((FREELIST_DEFAULT_MEMORY_POOL_SIZE - sizeof(MemoryPoolHeader)) / 2) - sizeof(MemoryBlockHeader);

    class SegregatedList {
    public:
        void AddMemoryBlock(FreeListHeader *freelistHeader);
        FreeListHeader *FindMemoryBlock(size_t size);
        void ReleaseFreeMemoryBlocks();
        double CalculateExternalFragmentation();

    private:
        static constexpr size_t SEGREGATED_LIST_SIZE = PANDA_FREELIST_ALLOCATOR_SEGREGATED_LIST_SIZE;
        static constexpr size_t SEGREGATED_LIST_FREE_BLOCK_RANGE =
            (FREELIST_MAX_ALLOC_SIZE - FREELIST_ALLOCATOR_MIN_SIZE) / SEGREGATED_LIST_SIZE;
        // If it is off, we insert memory in the list in the descending order.
        static constexpr bool SEGREGATED_LIST_FAST_INSERT = PANDA_FREELIST_ALLOCATOR_SEGREGATED_LIST_FAST_INSERT;
        // If it is off, we try to find the most suitable block in the list.
        static constexpr bool SEGREGATED_LIST_FAST_EXTRACT = PANDA_FREELIST_ALLOCATOR_SEGREGATED_LIST_FAST_EXTRACT;
        static_assert(((FREELIST_MAX_ALLOC_SIZE - FREELIST_ALLOCATOR_MIN_SIZE) % SEGREGATED_LIST_SIZE) == 0);

        size_t GetIndex(size_t size);

        FreeListHeader *GetFirstBlock(size_t index)
        {
            ASSERT(index < SEGREGATED_LIST_SIZE);
            return freeMemoryBlocks_[index].GetNextFree();
        }

        void SetFirstBlock(size_t index, FreeListHeader *newHead)
        {
            ASSERT(index < SEGREGATED_LIST_SIZE);
            freeMemoryBlocks_[index].SetNextFree(newHead);
        }

        FreeListHeader *FindTheMostSuitableBlockInOrderedList(size_t index, size_t size);

        FreeListHeader *FindSuitableBlockInList(FreeListHeader *head, size_t size);

        inline FreeListHeader *TryGetSuitableBlockBySize(FreeListHeader *head, size_t size, size_t index);

        // Each element of this array consists of memory blocks with size
        // from (FREELIST_ALLOCATOR_MIN_SIZE + SEGREGATED_LIST_FREE_BLOCK_RANGE * (N))
        // to   (FREELIST_ALLOCATOR_MIN_SIZE + SEGREGATED_LIST_FREE_BLOCK_RANGE * (N + 1)) (not inclusive)
        // where N is the element number in this array.
        std::array<FreeListHeader, SEGREGATED_LIST_SIZE> freeMemoryBlocks_;
    };

    MemoryBlockHeader *GetFreeListMemoryHeader(void *mem);

    bool AllocatedByFreeListAllocator(void *mem);

    bool AllocatedByFreeListAllocatorUnsafe(void *mem);

    // Try to coalesce a memory block with the next and previous blocks.
    FreeListHeader *TryToCoalescing(MemoryBlockHeader *memoryHeader);

    // Coalesce two neighboring memory blocks into one.
    void CoalesceMemoryBlocks(MemoryBlockHeader *firstBlock, MemoryBlockHeader *secondBlock);

    /**
     * @brief Divide memory_block into two - the first with first_block_size.
     * @param memory_block - a pointer to the divided block, first_block_size - size of the first part
     * @return the second memory block header
     */
    MemoryBlockHeader *SplitMemoryBlocks(MemoryBlockHeader *memoryBlock, size_t firstBlockSize);

    void AddToSegregatedList(FreeListHeader *freeListElement);

    MemoryBlockHeader *GetFromSegregatedList(size_t size, Alignment align);

    bool CanCreateNewBlockFromRemainder(MemoryBlockHeader *memory, size_t allocSize)
    {
        // NOTE(aemelenko): Maybe add some more complex solution for this check
        return (memory->GetSize() - allocSize) >= (FREELIST_ALLOCATOR_MIN_SIZE + sizeof(FreeListHeader));
    }

    void FreeUnsafe(void *mem);

    SegregatedList segregatedList_;

    // Links to head and tail of the memory pool headers
    MemoryPoolHeader *mempoolHead_ {nullptr};
    MemoryPoolHeader *mempoolTail_ {nullptr};
    SpaceType typeAllocation_;

    // RW lock which allows only one thread to change smth inside allocator
    // NOTE: The MT support expects that we can't iterate
    // and free (i.e. collect for an object scenario) simultaneously
    LockConfigT allocFreeLock_;

    MemStatsType *memStats_;

#ifdef PANDA_MEASURE_FRAGMENTATION
    std::atomic_size_t allocatedBytes_ {0};
#endif

    friend class FreeListAllocatorTest;
    template <InternalAllocatorConfig CONFIG>
    friend class InternalAllocator;
};

}  // namespace ark::mem

#endif  // PANDA_MEM_FREELIST_ALLOCATOR_H
