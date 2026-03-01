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
#ifndef PANDA_RUNTIME_MEM_RUNSLOTS_ALLOCATOR_H
#define PANDA_RUNTIME_MEM_RUNSLOTS_ALLOCATOR_H

#include <algorithm>
#include <array>
#include <cstddef>

#include "libarkbase/macros.h"
#include "libarkbase/mem/mem.h"
#include "libarkbase/mem/space.h"
#include "libarkbase/utils/logger.h"
#include "runtime/mem/runslots.h"
#include "runtime/mem/gc/bitmap.h"
#include "runtime/mem/lock_config_helper.h"

namespace ark::mem {

// NOLINTNEXTLINE(cppcoreguidelines-macro-usage)
#define LOG_RUNSLOTS_ALLOCATOR(level) LOG(level, ALLOC) << "RunSlotsAllocator: "

class RunSlotsAllocatorLockConfig {
public:
    class CommonLock {
    public:
        using PoolLock = os::memory::RWLock;
        using ListLock = os::memory::Mutex;
        using RunSlotsLock = RunSlotsLockConfig::CommonLock;
    };

    class DummyLock {
    public:
        using PoolLock = os::memory::DummyLock;
        using ListLock = os::memory::DummyLock;
        using RunSlotsLock = RunSlotsLockConfig::DummyLock;
    };

    template <MTModeT MT_MODE>
    using ParameterizedLock = typename LockConfigHelper<RunSlotsAllocatorLockConfig, MT_MODE>::Value;
};

template <typename T, typename AllocConfigT, typename LockConfigT>
class RunSlotsAllocatorAdapter;
enum class InternalAllocatorConfig;
template <InternalAllocatorConfig CONFIG>
class InternalAllocator;

/**
 * RunSlotsAllocator is an allocator based on RunSlots instance.
 * It gets a big pool of memory from OS and uses it for creating RunSlots with different slot sizes.
 */
template <typename AllocConfigT, typename LockConfigT = RunSlotsAllocatorLockConfig::CommonLock>
class RunSlotsAllocator {
public:
    explicit RunSlotsAllocator(MemStatsType *memStats, SpaceType typeAllocation = SpaceType::SPACE_TYPE_OBJECT);
    ~RunSlotsAllocator();
    NO_COPY_SEMANTIC(RunSlotsAllocator);
    NO_MOVE_SEMANTIC(RunSlotsAllocator);

    template <typename T, typename... Args>
    [[nodiscard]] T *New(Args &&...args)
    {
        auto p = reinterpret_cast<void *>(Alloc(sizeof(T)));
        new (p) T(std::forward<Args>(args)...);
        return reinterpret_cast<T *>(p);
    }

    template <typename T>
    [[nodiscard]] T *AllocArray(size_t arrLength);

    template <bool NEED_LOCK = true, bool DISABLE_USE_FREE_RUNSLOTS = false>
    [[nodiscard]] void *Alloc(size_t size, Alignment align = DEFAULT_ALIGNMENT);

    void Free(void *mem);

    void Collect(const GCObjectVisitor &deathCheckerFn);

    bool AddMemoryPool(void *mem, size_t size);

    /**
     * @brief Iterates over all objects allocated by this allocator.
     * @tparam MemVisitor
     * @param mem_visitor - function pointer or functor
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

    RunSlotsAllocatorAdapter<void, AllocConfigT, LockConfigT> Adapter();

    /**
     * @brief returns maximum size which can be allocated by this allocator
     * @return
     */
    static constexpr size_t GetMaxSize()
    {
        return RunSlotsType::MaxSlotSize();
    }

    /**
     * @brief returns minimum pool size which can be added to this allocator
     * @return
     */
    static constexpr size_t GetMinPoolSize()
    {
        return MIN_POOL_SIZE;
    }

    static constexpr size_t PoolAlign()
    {
        return DEFAULT_ALIGNMENT_IN_BYTES;
    }

    size_t VerifyAllocator();

    bool ContainObject(const ObjectHeader *obj);

    bool IsLive(const ObjectHeader *obj);

    static constexpr AllocatorType GetAllocatorType()
    {
        return AllocatorType::RUNSLOTS_ALLOCATOR;
    }

#ifdef PANDA_MEASURE_FRAGMENTATION
    size_t GetAllocatedBytes() const
    {
        // Atomic with relaxed order reason: order is not required
        return allocatedBytes_.load(std::memory_order_relaxed);
    }
#endif

private:
    using RunSlotsType = RunSlots<typename LockConfigT::RunSlotsLock>;

    static constexpr size_t MIN_POOL_SIZE = PANDA_DEFAULT_ALLOCATOR_POOL_SIZE;

    class RunSlotsList {
    public:
        RunSlotsList()
        {
            head_ = nullptr;
            tail_ = nullptr;
        }

        typename LockConfigT::ListLock *GetLock()
        {
            return &lock_;
        }

        RunSlotsType *GetHead()
        {
            return head_;
        }

        RunSlotsType *GetTail()
        {
            return tail_;
        }

        void PushToTail(RunSlotsType *runslots);

        RunSlotsType *PopFromHead();

        RunSlotsType *PopFromTail();

        void PopFromList(RunSlotsType *runslots);

        bool IsInThisList(RunSlotsType *runslots)
        {
            RunSlotsType *current = head_;
            while (current != nullptr) {
                if (current == runslots) {
                    return true;
                }
                current = current->GetNextRunSlots();
            }
            return false;
        }

        ~RunSlotsList() = default;

        NO_COPY_SEMANTIC(RunSlotsList);
        NO_MOVE_SEMANTIC(RunSlotsList);

    private:
        RunSlotsType *head_;
        RunSlotsType *tail_;
        typename LockConfigT::ListLock lock_;
    };

    /**
     * MemPoolManager class is used for manage memory which we get from OS.
     * Current implementation limits the amount of memory pools which can be managed by this class.
     */

    // MemPoolManager structure:
    //
    //           part_occupied_head_ - is a pointer to the first partially occupied pool in occupied list
    //            |
    //            |                occupied_tail_ - is a pointer to the last occupied pool
    //            |                 |
    //            |  part occupied  |
    //            v     pools       v
    // |x|x|x|x|x|x|x|x|x|x|x|x|x|x|x|
    //  ^                           ^
    //  |      occupied  pools      |
    //
    //                   free_tail_ - is a pointer to the last totally free pool.
    //                    |
    //                    |
    //                    v
    // |0|0|0|0|0|0|0|0|0|0|
    //  ^                 ^
    //  |   free  pools   |
    //

    class MemPoolManager {
    public:
        explicit MemPoolManager();

        template <bool NEED_LOCK = true>
        RunSlotsType *GetNewRunSlots(size_t slotsSize);

        bool AddNewMemoryPool(void *mem, size_t size);

        template <typename ObjectVisitor>
        void IterateOverObjects(const ObjectVisitor &objectVisitor);

        template <typename MemVisitor>
        void VisitAndRemoveAllPools(const MemVisitor &memVisitor);

        template <typename MemVisitor>
        void VisitAllPoolsWithOccupiedSize(const MemVisitor &memVisitor);

        template <typename MemVisitor>
        void VisitAndRemoveFreePools(const MemVisitor &memVisitor);

        void ReturnAndReleaseRunSlotsMemory(RunSlotsType *runslots);

        bool IsInMemPools(void *object);

        ~MemPoolManager() = default;

        NO_COPY_SEMANTIC(MemPoolManager);
        NO_MOVE_SEMANTIC(MemPoolManager);

    private:
        class PoolListElement {
        public:
            PoolListElement();

            void Initialize(void *poolMem, uintptr_t unoccupiedMem, size_t size, PoolListElement *prev);

            static PoolListElement *Create(void *mem, size_t size, PoolListElement *prev)
            {
                LOG_RUNSLOTS_ALLOCATOR(DEBUG)
                    << "PoolMemory: Create new instance with size " << size << " bytes at addr " << std::hex << mem;
                ASSERT(mem != nullptr);
                ASSERT(sizeof(PoolListElement) <= RUNSLOTS_SIZE);
                ASAN_UNPOISON_MEMORY_REGION(mem, sizeof(PoolListElement));
                auto newElement = new (mem) PoolListElement();
                uintptr_t unoccupiedMem = AlignUp(ToUintPtr(mem) + sizeof(PoolListElement), RUNSLOTS_SIZE);
                ASSERT(unoccupiedMem < ToUintPtr(mem) + size);
                newElement->Initialize(mem, unoccupiedMem, size, prev);
                return newElement;
            }

            bool HasMemoryForRunSlots();

            bool IsInitialized()
            {
                return startMem_ != 0;
            }

            RunSlotsType *GetMemoryForRunSlots(size_t slotsSize);

            template <typename RunSlotsVisitor>
            void IterateOverRunSlots(const RunSlotsVisitor &runslotsVisitor);

            bool HasUsedMemory();

            size_t GetOccupiedSize();

            bool IsInUsedMemory(void *object);

            void *GetPoolMemory()
            {
                return ToVoidPtr(poolMem_);
            }

            size_t GetSize()
            {
                return size_;
            }

            PoolListElement *GetNext() const
            {
                return nextPool_;
            }

            PoolListElement *GetPrev() const
            {
                return prevPool_;
            }

            void SetPrev(PoolListElement *prev)
            {
                prevPool_ = prev;
            }

            void SetNext(PoolListElement *next)
            {
                nextPool_ = next;
            }

            void PopFromList();

            void AddFreedRunSlots(RunSlotsType *slots)
            {
                [[maybe_unused]] bool oldVal = freedRunslotsBitmap_.AtomicTestAndSet(slots);
                ASSERT(!oldVal);
                freededRunslotsCount_++;
                ASAN_POISON_MEMORY_REGION(slots, RUNSLOTS_SIZE);
            }

            bool IsInFreedRunSlots(void *addr)
            {
                void *alignAddr = ToVoidPtr((ToUintPtr(addr) >> RUNSLOTS_ALIGNMENT) << RUNSLOTS_ALIGNMENT);
                return freedRunslotsBitmap_.TestIfAddrValid(alignAddr);
            }

            size_t GetFreedRunSlotsCount()
            {
                return freededRunslotsCount_;
            }

            ~PoolListElement() = default;

            NO_COPY_SEMANTIC(PoolListElement);
            NO_MOVE_SEMANTIC(PoolListElement);

        private:
            using MemBitmapClass = MemBitmap<RUNSLOTS_SIZE, uintptr_t>;
            using BitMapStorageType = std::array<uint8_t, MemBitmapClass::GetBitMapSizeInByte(MIN_POOL_SIZE)>;

            uintptr_t GetFirstRunSlotsBlock(uintptr_t mem);

            RunSlotsType *GetFreedRunSlots(size_t slotsSize);

            uintptr_t poolMem_;
            uintptr_t startMem_;
            std::atomic<uintptr_t> freePtr_;
            size_t size_;
            PoolListElement *nextPool_;
            PoolListElement *prevPool_;
            size_t freededRunslotsCount_;
            BitMapStorageType storageForBitmap_;
            MemBitmapClass freedRunslotsBitmap_ {nullptr, MIN_POOL_SIZE, storageForBitmap_.data()};
        };

        PoolListElement *freeTail_;
        PoolListElement *partiallyOccupiedHead_;
        PoolListElement *occupiedTail_;
        typename LockConfigT::PoolLock lock_;
    };

    void ReleaseEmptyRunSlotsPagesUnsafe();

    template <bool LOCK_RUN_SLOTS>
    void FreeUnsafe(void *mem);

    bool FreeUnsafeInternal(RunSlotsType *runslots, void *mem);

    void TrimUnsafe();

    // Return true if this object could be allocated by the RunSlots allocator.
    // Does not check any live objects bitmap inside.
    bool AllocatedByRunSlotsAllocator(void *object);

    bool AllocatedByRunSlotsAllocatorUnsafe(void *object);

    template <bool NEED_LOCK = true>
    RunSlotsType *CreateNewRunSlotsFromMemory(size_t slotsSize);

    // Add one to the array size to just use the size (power of two) for RunSlots list without any modifications
    static constexpr size_t SLOTS_SIZES_VARIANTS = RunSlotsType::SlotSizesVariants() + 1;

    std::array<RunSlotsList, SLOTS_SIZES_VARIANTS> runslots_;

    // Add totally free RunSlots in this list for possibility to reuse them with different element sizes.
    RunSlotsList freeRunslots_;

    MemPoolManager memoryPool_;
    SpaceType typeAllocation_;

    MemStatsType *memStats_;

#ifdef PANDA_MEASURE_FRAGMENTATION
    std::atomic_size_t allocatedBytes_ {0};
#endif

    template <typename T>
    friend class PygoteSpaceAllocator;
    friend class RunSlotsAllocatorTest;
    template <InternalAllocatorConfig CONFIG>
    friend class InternalAllocator;
};

template <typename AllocConfigT, typename LockConfigT>
template <typename T>
T *RunSlotsAllocator<AllocConfigT, LockConfigT>::AllocArray(size_t arrLength)
{
    // NOTE(aemelenko): Very dirty hack. If you want to fix it, you must change RunSlotsAllocatorAdapter::max_size() too
    return static_cast<T *>(Alloc(sizeof(T) * arrLength));
}

#undef LOG_RUNSLOTS_ALLOCATOR

}  // namespace ark::mem

#endif  // PANDA_RUNTIME_MEM_RUNSLOTS_ALLOCATOR_H
