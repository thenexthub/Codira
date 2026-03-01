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
#ifndef PANDA_MEM_HUMONGOUS_OBJ_ALLOCATOR_H
#define PANDA_MEM_HUMONGOUS_OBJ_ALLOCATOR_H

#include <limits>

#include "libarkbase/macros.h"
#include "libarkbase/mem/mem.h"
#include "libarkbase/mem/pool_manager.h"
#include "libarkbase/mem/space.h"
#include "libarkbase/utils/logger.h"
#include "runtime/mem/runslots.h"
#include "runtime/mem/lock_config_helper.h"

namespace ark::mem {

// NOLINTNEXTLINE(cppcoreguidelines-macro-usage)
#define LOG_HUMONGOUS_OBJ_ALLOCATOR(level) LOG(level, ALLOC) << "HumongousObjAllocator: "

// NOTE(aemelenko): Move this constants to compile options
static constexpr size_t PANDA_HUMONGOUS_OBJ_ALLOCATOR_RESERVED_MEM_MAX_POOLS_AMOUNT = 0;
static constexpr size_t PANDA_HUMONGOUS_OBJ_ALLOCATOR_RESERVED_MEM_MAX_POOL_SIZE = 8_MB;

class HumongousObjAllocatorLockConfig {
public:
    using CommonLock = os::memory::RWLock;
    using DummyLock = os::memory::DummyLock;

    template <MTModeT MT_MODE>
    using ParameterizedLock = typename LockConfigHelper<HumongousObjAllocatorLockConfig, MT_MODE>::Value;
};

template <typename T, typename AllocConfigT, typename LockConfigT>
class HumongousObjAllocatorAdapter;
enum class InternalAllocatorConfig;
template <InternalAllocatorConfig CONFIG>
class InternalAllocator;

/**
 * HumongousObjAllocator is an allocator used for huge objects which require
 * using the whole memory pool for each.
 */
template <typename AllocConfigT, typename LockConfigT = HumongousObjAllocatorLockConfig::CommonLock>
class HumongousObjAllocator {
public:
    explicit HumongousObjAllocator(MemStatsType *memStats,
                                   SpaceType typeAllocation = SpaceType::SPACE_TYPE_HUMONGOUS_OBJECT);
    ~HumongousObjAllocator();
    NO_COPY_SEMANTIC(HumongousObjAllocator);
    NO_MOVE_SEMANTIC(HumongousObjAllocator);

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
    [[nodiscard]] void *Alloc(size_t size, Alignment align = DEFAULT_ALIGNMENT);

    void Free(void *mem);

    void Collect(const GCObjectVisitor &deathCheckerFn);

    // It is essential that mem is page aligned
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

    HumongousObjAllocatorAdapter<void, AllocConfigT, LockConfigT> Adapter();

    /**
     * @brief returns maximum size which can be allocated by this allocator
     * @return
     */
    static constexpr size_t GetMaxSize()
    {
        return HUMONGOUS_OBJ_ALLOCATOR_MAX_SIZE;
    }

    /**
     * @brief returns minimum pool size to allocate an object with @param obj_size bytes
     * @return
     */
    static constexpr size_t GetMinPoolSize(size_t objSize)
    {
        // To note: It is not the smallest size of the pool
        // because we don't make a real object alignment value into account
        return AlignUp(objSize + sizeof(MemoryPoolHeader) + GetAlignmentInBytes(LOG_ALIGN_MAX),
                       PANDA_POOL_ALIGNMENT_IN_BYTES);
    }

    bool ContainObject(const ObjectHeader *obj);

    bool IsLive(const ObjectHeader *obj);

    static constexpr AllocatorType GetAllocatorType()
    {
        return AllocatorType::HUMONGOUS_ALLOCATOR;
    }

private:
#ifndef NDEBUG
    // Only for debug purpose to find some anomalous allocations
    static constexpr size_t HUMONGOUS_OBJ_ALLOCATOR_MAX_SIZE = 2_GB;
#else
    static constexpr size_t HUMONGOUS_OBJ_ALLOCATOR_MAX_SIZE = std::numeric_limits<size_t>::max();
#endif

    class MemoryPoolHeader {
    public:
        void Initialize(size_t size, MemoryPoolHeader *prev, MemoryPoolHeader *next);

        void Alloc(size_t size, Alignment align);

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

        void PopHeader();

        ATTRIBUTE_NO_SANITIZE_ADDRESS
        size_t GetPoolSize()
        {
            ASAN_UNPOISON_MEMORY_REGION(this, sizeof(MemoryPoolHeader));
            size_t size = poolSize_;
            ASAN_POISON_MEMORY_REGION(this, sizeof(MemoryPoolHeader));
            return size;
        }

        ATTRIBUTE_NO_SANITIZE_ADDRESS
        void *GetMemory()
        {
            ASAN_UNPOISON_MEMORY_REGION(this, sizeof(MemoryPoolHeader));
            void *addr = memAddr_;
            ASAN_POISON_MEMORY_REGION(this, sizeof(MemoryPoolHeader));
            return addr;
        }

    private:
        MemoryPoolHeader *prev_ {nullptr};
        MemoryPoolHeader *next_ {nullptr};
        size_t poolSize_ {0};
        void *memAddr_ {nullptr};
    };

    static constexpr size_t PAGE_SIZE_MASK = ~(PANDA_PAGE_SIZE - 1);

    class MemoryPoolList {
    public:
        void Insert(MemoryPoolHeader *pool);

        void Pop(MemoryPoolHeader *pool);

        /**
         * @brief Try to find a pool suitable for object with @param size.
         * @return a pointer to pool header on success, nullptr otherwise.
         */
        MemoryPoolHeader *FindSuitablePool(size_t size);

        /// @brief Iterate over pools in this list and pop all the elements.
        template <typename MemVisitor>
        void IterateAndPopOverPools(const MemVisitor &memVisitor);

        MemoryPoolHeader *GetListHead()
        {
            return head_;
        }

    private:
        bool IsInThisList(MemoryPoolHeader *pool);

        MemoryPoolHeader *head_ {nullptr};
    };

    // ReservedMemoryPools class is used to prevent ping-pong effect.
    // Elements in ReservedMemoryPools are sorted ascending.
    // When we free a pool, we try to insert in into ReservedMemoryPools first:
    // - If the pool is too big for ReservedMemoryPools, we skip inserting.
    // - If the pool is bigger than the smaller pool in ReservedMemoryPools, we insert it.
    class ReservedMemoryPools : public MemoryPoolList {
    public:
        /**
         * @brief Try to insert @param pool inside ReservedMemoryPools.
         * @return @param pool if not success, nullptr or crowded out pool on success.
         */
        MemoryPoolHeader *TryToInsert(MemoryPoolHeader *pool);

        void Pop(MemoryPoolHeader *pool)
        {
            elementsCount_--;
            LOG_HUMONGOUS_OBJ_ALLOCATOR(DEBUG)
                << "Pop from Reserved list. Now, there are " << elementsCount_ << " elements in it.";
            MemoryPoolList::Pop(pool);
        }

    private:
        static constexpr size_t MAX_POOL_SIZE = PANDA_HUMONGOUS_OBJ_ALLOCATOR_RESERVED_MEM_MAX_POOL_SIZE;
        static constexpr size_t MAX_POOLS_AMOUNT = PANDA_HUMONGOUS_OBJ_ALLOCATOR_RESERVED_MEM_MAX_POOLS_AMOUNT;

        void SortedInsert(MemoryPoolHeader *pool);

        size_t elementsCount_ {0};
    };

    void ReleaseUnusedPagesOnAlloc(MemoryPoolHeader *memoryPool, size_t allocSize);

    void InsertPool(MemoryPoolHeader *header);

    void FreeUnsafe(void *mem);

    bool AllocatedByHumongousObjAllocator(void *mem);

    bool AllocatedByHumongousObjAllocatorUnsafe(void *mem);

    MemoryPoolList occupiedPoolsList_;
    ReservedMemoryPools reservedPoolsList_;
    MemoryPoolList freePoolsList_;
    SpaceType typeAllocation_;

    // RW lock which allows only one thread to change smth inside allocator
    // NOTE: The MT support expects that we can't iterate
    // and free (i.e. collect for an object scenario) simultaneously
    LockConfigT allocFreeLock_;

    MemStatsType *memStats_;

    friend class HumongousObjAllocatorTest;
    template <InternalAllocatorConfig CONFIG>
    friend class InternalAllocator;
};  // namespace ark::mem

#undef LOG_HUMONGOUS_OBJ_ALLOCATOR

}  // namespace ark::mem

#endif  // PANDA_MEM_HUMONGOUS_OBJ_ALLOCATOR_H
