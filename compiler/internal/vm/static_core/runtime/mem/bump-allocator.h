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
#ifndef RUNTIME_MEM_PANDA_BUMP_ALLOCATOR_H
#define RUNTIME_MEM_PANDA_BUMP_ALLOCATOR_H

#include <functional>
#include <memory>

#include "libarkbase/mem/mem_pool.h"
#include "libarkbase/macros.h"
#include "libarkbase/mem/arena-inl.h"
#include "libarkbase/mem/mem.h"
#include "libarkbase/mem/mem_range.h"
#include "runtime/mem/tlab.h"
#include "runtime/mem/lock_config_helper.h"

namespace ark {
class ObjectHeader;
}  // namespace ark

namespace ark::mem {

class BumpPointerAllocatorLockConfig {
public:
    using CommonLock = os::memory::Mutex;
    using DummyLock = os::memory::DummyLock;

    template <MTModeT MT_MODE>
    using ParameterizedLock = typename LockConfigHelper<BumpPointerAllocatorLockConfig, MT_MODE>::Value;
};

// This allocator can allocate memory as a BumpPointerAllocator
// and also can allocate big pieces of memory for the TLABs.
//
// Structure:
//
//  |------------------------------------------------------------------------------------------------------------|
//  |                                                 Memory Pool                                                |
//  |------------------------------------------------------------------------------------------------------------|
//  |     allocated objects     |         unused memory        |                 memory for TLABs                |
//  |---------------------------|------------------------------|-------------------------------------------------|
//  |xxxxxxxxxx|xxxxxx|xxxxxxxxx|                              |               ||               ||               |
//  |xxxxxxxxxx|xxxxxx|xxxxxxxxx|                              |               ||               ||               |
//  |xxxxxxxxxx|xxxxxx|xxxxxxxxx|           free memory        |     TLAB 3    ||     TLAB 2    ||     TLAB 1    |
//  |xxxxxxxxxx|xxxxxx|xxxxxxxxx|                              |               ||               ||               |
//  |xxxxxxxxxx|xxxxxx|xxxxxxxxx|                              |               ||               ||               |
//  |------------------------------------------------------------------------------------------------------------|
//

template <typename AllocConfigT, typename LockConfigT = BumpPointerAllocatorLockConfig::CommonLock,
          bool USE_TLABS = false>
class BumpPointerAllocator {
public:
    DEFAULT_NOEXCEPT_MOVE_SEMANTIC(BumpPointerAllocator);
    NO_COPY_SEMANTIC(BumpPointerAllocator);
    ~BumpPointerAllocator();

    BumpPointerAllocator() = delete;

    TLAB *CreateNewTLAB(size_t size);

    /**
     * Construct BumpPointer allocator with provided pool
     * @param pool - pool
     */
    explicit BumpPointerAllocator(Pool pool, SpaceType typeAllocation, MemStatsType *memStats,
                                  size_t tlabsMaxCount = 0);

    [[nodiscard]] void *Alloc(size_t size, Alignment alignment = ark::DEFAULT_ALIGNMENT);

    void VisitAndRemoveAllPools(const MemVisitor &memVisitor);

    void VisitAndRemoveFreePools(const MemVisitor &memVisitor);

    /**
     * @brief Iterates over all objects allocated by this allocator
     * @param object_visitor
     */
    void IterateOverObjects(const std::function<void(ObjectHeader *objectHeader)> &objectVisitor);

    /**
     * @brief Iterates over objects in the range inclusively.
     * @tparam MemVisitor
     * @param mem_visitor - function pointer or functor
     * @param left_border - a pointer to the first byte of the range
     * @param right_border - a pointer to the last byte of the range
     */
    template <typename MemVisitor>
    void IterateOverObjectsInRange(const MemVisitor &memVisitor, void *leftBorder, void *rightBorder);

    /// Resets to the "all clear" state
    void Reset();

    /**
     * @brief Add an extra memory pool to the allocator.
     * The memory pool must be located just after the current memory given to this allocator.
     * @param mem - pointer to the extra memory pool.
     * @param size - a size of the extra memory pool.
     */
    void ExpandMemory(void *mem, size_t size);

    /**
     * Get MemRange used by allocator
     * @return MemRange for allocator
     */
    MemRange GetMemRange();

    // BumpPointer allocator can't be used for simple collection.
    // Only for CollectAndMove.
    void Collect(GCObjectVisitor deathCheckerFn) = delete;

    /**
     * Collects dead objects and move alive with provided visitor
     * @param death_checker - functor for check if object alive
     * @param object_move_visitor - object visitor
     */
    template <typename ObjectMoveVisitorT>
    void CollectAndMove(const GCObjectVisitor &deathChecker, const ObjectMoveVisitorT &objectMoveVisitor);

    static constexpr AllocatorType GetAllocatorType()
    {
        return AllocatorType::BUMP_ALLOCATOR;
    }

    bool ContainObject(const ObjectHeader *obj);

    bool IsLive(const ObjectHeader *obj);

private:
    class TLABsManager {
    public:
        explicit TLABsManager(size_t tlabsMaxCount) : tlabsMaxCount_(tlabsMaxCount), tlabs_(tlabsMaxCount) {}

        void Reset()
        {
            for (size_t i = 0; i < curTlabNum_; i++) {
                tlabs_[i].Fill(nullptr, 0);
            }
            curTlabNum_ = 0;
            tlabsOccupiedSize_ = 0;
        }

        TLAB *GetUnusedTLABInstance()
        {
            if (curTlabNum_ < tlabsMaxCount_) {
                return &tlabs_[curTlabNum_++];
            }
            return nullptr;
        }

        template <class Visitor>
        void IterateOverTLABs(const Visitor &visitor)
        {
            for (size_t i = 0; i < curTlabNum_; i++) {
                if (!visitor(&tlabs_[i])) {
                    return;
                }
            }
        }

        size_t GetTLABsOccupiedSize()
        {
            return tlabsOccupiedSize_;
        }

        void IncreaseTLABsOccupiedSize(size_t size)
        {
            tlabsOccupiedSize_ += size;
        }

    private:
        size_t curTlabNum_ {0};
        size_t tlabsMaxCount_;
        std::vector<TLAB> tlabs_;
        size_t tlabsOccupiedSize_ {0};
    };

    // Mutex, which allows only one thread to Alloc/Free/Collect/Iterate inside this allocator
    LockConfigT allocatorLock_;
    Arena arena_;
    TLABsManager tlabManager_;
    SpaceType typeAllocation_;
    MemStatsType *memStats_;
};

}  // namespace ark::mem

#endif  // RUNTIME_MEM_PANDA_BUMP_ALLOCATOR_H
