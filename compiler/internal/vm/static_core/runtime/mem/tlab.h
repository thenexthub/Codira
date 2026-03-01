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
#ifndef PANDA_RUNTIME_MEM_TLAB_H
#define PANDA_RUNTIME_MEM_TLAB_H

#include "libarkbase/utils/logger.h"
#include "libarkbase/mem/mem.h"
#include "libarkbase/mem/pool_map.h"
#include "libarkbase/mem/mem_range.h"

namespace ark::mem {

// NOLINTNEXTLINE(cppcoreguidelines-macro-usage)
#define LOG_TLAB_ALLOCATOR(level) LOG(level, ALLOC) << "TLAB: "

#ifdef NDEBUG
static constexpr bool PANDA_TRACK_TLAB_ALLOCATIONS = false;
#else
static constexpr bool PANDA_TRACK_TLAB_ALLOCATIONS = true;
#endif
// Current TLAB structure looks like that:
//
// |--------------------------|
// |........TLAB class........|
// |--------------------------|
// |.........end addr.........|------------|
// |.......free pointer.......|--------|   |
// |........start addr........|----|   |   |
// |--------------------------|    |   |   |
//                                 |   |   |
//                                 |   |   |
// |--------------------------|    |   |   |
// |..Memory for allocations..|    |   |   |
// |--------------------------|    |   |   |
// |xxxxxxxxxxxxxxxxxxxxxxxxxx|<---|   |   |
// |xxxxxxxxxxxxxxxxxxxxxxxxxx|        |   |
// |xxxxxxxxxxxxxxxxxxxxxxxxxx|        |   |
// |xxxxxxxxxxxxxxxxxxxxxxxxxx|        |   |
// |xxxxxxxxxxxxxxxxxxxxxxxxxx|        |   |
// |xxxxxxxxxxxxxxxxxxxxxxxxxx|<-------|   |
// |..........................|            |
// |..........................|            |
// |..........................|            |
// |..........................|            |
// |..........................|<-----------|
// |--------------------------|
//
// Each TLAB is connected with certain thread:
// (NOTE: In current implementation, we can reach max one TLAB from a thread metadata)
// NOTE(aemelenko): If we don't use links on next, prev TLABS,
// it is better to remove these fields.
// |------------------------|              |---------------|
// | Thread Metainformation | ---------->  | Current  TLAB |----
// |------------------------|              |---------------|    |
//                                                              |
//                                                              |
//                                         |---------------|    |
//                                         | Previous TLAB |<---|
//                                         |---------------|

// How to use TLAB from the compiler if we want to allocate an object for class 'cls' with size 'allocation_size':
// NOTE: If we have PANDA_TRACK_TLAB_ALLOCATIONS option on, JIT should always call Runtime at AllocateObject calls.
// Pseudocode:
// IF  allocation_size > TLAB::GetMaxSize() || IsFinalizable(cls)
//     call HeapManager::AllocateObject(obj_class, allocation_size)
// ELSE
//     // We should use TLS for this purpose.
//     // Read current TLAB pointer from TLS:
//     load TLS.TLAB -> cur_TLAB
//     // Read uintptr_t value from TLAB structure:
//     load (AddressOf(cur_TLAB) + TLAB::TLABFreePointerOffset) -> free_pointer
//     // Read uintptr_t value from TLAB structure:
//     load (AddressOf(cur_TLAB) + TLAB::TLABEndAddrOffset) -> end_pointer
//     // Align the size of an object to DEFAULT_ALIGNMENT_IN_BYTES
//     // One can use GetAlignedObjectSize() method for that.
//     align (allocation_size, DEFAULT_ALIGNMENT_IN_BYTES) -> allocation_size
//     IF  free_pointer + allocation_size > end_pointer
//         // Goto slow path
//         call HeapManager::AllocateObject(obj_class, allocation_size)
//     // Calculate new_free_pointer:
//     new_free_pointer = AddressOf(free_pointer) + allocation_size
//     // Store new_free_pointer to (cur_TLAB + TLAB::TLABFreePointerOffset):
//     store (AddressOf(cur_TLAB) + TLAB::TLABFreePointerOffset) <- new_free_pointer
//     return free_pointer
//
// After that the Compiler should initialize class word inside new object and
// set correct GC bits in the mark word:
//     ObjectHeader obj_header
//     obj_header.SetClass(cls)
//     GetGC()->InitGCBitsForAllocationInTLAB(&obj_header)
//     free_pointer <- obj_header
//
// Runtime should provide these parameters:
// HeapManager::GetTLABMaxAllocSize() - max size that can be allocated via TLAB. (depends on the allocator used by GC)
// HeapManager::UseTLABForAllocations() - do we need to use TLABs for allocations. (it is a runtime option)
// GC::InitGCBitsForAllocationInTLAB() - method for initialize GC bits inside the object header
//                                       during allocations through TLAB
// TLAB::TLABFreePointerOffset() - an offset of a free pointer field inside TLAB.
// TLAB::TLABEndAddrOffset() - an offset of a end buffer pointer field inside TLAB.
//

class TLAB {
public:
    /**
     * @brief Construct TLAB with the buffer at @param address with @param size
     * @param address - a pointer into the memory where TLAB memory will be created
     * @param size - a size of the allocated memory for the TLAB
     */
    explicit TLAB(void *address = nullptr, size_t size = 0);
    ~TLAB();

    void Destroy();

    /**
     * @brief Fill a TLAB with the buffer at @param address with @param size
     * @param address - a pointer into the memory where TLAB memory will be created
     * @param size - a size of the allocated memory for the TLAB
     */
    void Fill(void *address, size_t size);

    /// @brief Set TLAB to be empty
    void Reset()
    {
        Fill(nullptr, 0U);
    }

    bool IsUninitialized()
    {
        return (memoryStartAddr_ == nullptr) || (curFreePosition_ == nullptr) || (memoryEndAddr_ == nullptr);
    }

    NO_MOVE_SEMANTIC(TLAB);
    NO_COPY_SEMANTIC(TLAB);

    /**
     * @brief Allocates memory with size @param size and aligned with DEFAULT_ALIGNMENT alignment
     * @param size - size of the allocated memory
     * @return pointer to the allocated memory on success, or nullptr on fail
     */
    void *Alloc(size_t size);

    /**
     * @brief Iterates over all objects in this TLAB
     * @param object_visitor
     */
    void IterateOverObjects(const std::function<void(ObjectHeader *objectHeader)> &objectVisitor);

    /**
     * @brief Iterates over objects in the range inclusively.
     * @param mem_visitor - function pointer or functor
     * @param mem_range - memory range
     */
    void IterateOverObjectsInRange(const std::function<void(ObjectHeader *objectHeader)> &memVisitor,
                                   const MemRange &memRange);

    /**
     * Collects dead objects and move alive with provided visitor
     * @param death_checker - functor for check if object alive
     * @param object_move_visitor - object visitor
     */
    template <typename ObjectMoveVisitorT>
    void CollectAndMove(const GCObjectVisitor &deathChecker, const ObjectMoveVisitorT &objectMoveVisitor)
    {
        LOG_TLAB_ALLOCATOR(DEBUG) << "CollectAndMove started";
        IterateOverObjects([&](ObjectHeader *objectHeader) {
            // We are interested only in moving alive objects, after that we cleanup this buffer
            if (deathChecker(objectHeader) == ObjectStatus::ALIVE_OBJECT) {
                LOG_TLAB_ALLOCATOR(DEBUG) << "CollectAndMove found alive object with addr " << objectHeader;
                objectMoveVisitor(objectHeader);
            }
        });
        LOG_TLAB_ALLOCATOR(DEBUG) << "CollectAndMove finished";
    }

    bool ContainObject(const ObjectHeader *obj);

    bool IsLive(const ObjectHeader *obj);

    TLAB *GetNextTLAB()
    {
        return nextTlab_;
    }

    TLAB *GetPrevTLAB()
    {
        return prevTlab_;
    }

    void SetNextTLAB(TLAB *tlabPointer)
    {
        nextTlab_ = tlabPointer;
    }

    void SetPrevTLAB(TLAB *tlabPointer)
    {
        prevTlab_ = tlabPointer;
    }

    void *GetStartAddr() const
    {
        return memoryStartAddr_;
    }

    void *GetEndAddr() const
    {
        return memoryEndAddr_;
    }

    void *GetCurPos() const
    {
        return curFreePosition_;
    }

    size_t GetOccupiedSize() const
    {
        ASSERT(ToUintPtr(curFreePosition_) >= ToUintPtr(memoryStartAddr_));
        return ToUintPtr(curFreePosition_) - ToUintPtr(memoryStartAddr_);
    }

    size_t GetFreeSize() const
    {
        ASSERT(ToUintPtr(curFreePosition_) >= ToUintPtr(memoryStartAddr_));
        ASSERT(ToUintPtr(curFreePosition_) <= ToUintPtr(memoryEndAddr_));
        return ToUintPtr(memoryEndAddr_) - ToUintPtr(curFreePosition_);
    }

    MemRange GetMemRangeForOccupiedMemory() const
    {
        return MemRange(ToUintPtr(memoryStartAddr_), ToUintPtr(curFreePosition_) - 1);
    }

    static constexpr size_t TLABStartAddrOffset()
    {
        return MEMBER_OFFSET(TLAB, memoryStartAddr_);
    }

    static constexpr size_t TLABFreePointerOffset()
    {
        return MEMBER_OFFSET(TLAB, curFreePosition_);
    }

    static constexpr size_t TLABEndAddrOffset()
    {
        return MEMBER_OFFSET(TLAB, memoryEndAddr_);
    }

    static constexpr AllocatorType GetAllocatorType()
    {
        return AllocatorType::TLAB_ALLOCATOR;
    }

    static constexpr float MIN_DESIRED_FILL_FRACTION = 0.5;

    size_t GetSize()
    {
        ASSERT(ToUintPtr(memoryEndAddr_) >= ToUintPtr(memoryStartAddr_));
        return ToUintPtr(memoryEndAddr_) - ToUintPtr(memoryStartAddr_);
    }

    float GetFillFraction()
    {
        size_t size = GetSize();
        if (size == 0) {
            // ZERO tlab case
            // consider it is always full
            return 1.0;
        }
        float fillFraction = static_cast<float>(GetOccupiedSize()) / static_cast<float>(size);
        ASSERT(fillFraction <= 1.0);
        return fillFraction;
    }

    void SetZeroedFlag(bool val)
    {
        zeroed_ = val;
    }

    bool IsZeroed() const
    {
        return zeroed_;
    }

private:
    TLAB *nextTlab_;
    TLAB *prevTlab_;
    // NOTE(aemelenko): Maybe use OBJECT_POINTER_SIZE here for heap allocation.
    void *memoryStartAddr_ {nullptr};
    void *memoryEndAddr_ {nullptr};
    void *curFreePosition_ {nullptr};
    bool zeroed_ = false;
};

#undef LOG_TLAB_ALLOCATOR

}  // namespace ark::mem

#endif  // PANDA_RUNTIME_MEM_TLAB_H
