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

#ifndef COMMON_COMPONENTS_HEAP_ALLOCATOR_ALLOCATOR_H
#define COMMON_COMPONENTS_HEAP_ALLOCATOR_ALLOCATOR_H

#include "common_components/heap/allocator/alloc_buffer_manager.h"
#include "common_components/heap/heap.h"

namespace common {
// Allocator abstract class
class Allocator {
public:
    static constexpr size_t ALLOC_ALIGN = 8;
    static constexpr size_t HEADER_SIZE = 0; // no header for arkcommon object

    static Allocator* CreateAllocator();

    virtual HeapAddress Allocate(size_t size, AllocType allocType) = 0;
    virtual HeapAddress AllocateNoGC(size_t size, AllocType allocType) = 0;
    virtual bool ForEachObject(const std::function<void(BaseObject*)>&, bool safe) const = 0;

    // release physical pages of garbage memory.
    virtual size_t ReclaimGarbageMemory(bool releaseAll) = 0;
    virtual void FeedHungryBuffers() = 0;

    // returns the total size of live large objects, excluding alignment/roundup/header, ...
    // LargeObjects() is missing.
    virtual size_t LargeObjectSize() const = 0;

    // returns the total bytes that has been allocated, including alignment/roundup/header, ...
    // allocated bytes for large objects are included.
    virtual size_t GetAllocatedBytes() const = 0;

    virtual size_t GetSurvivedSize() const = 0;

    inline void RegisterAllocBuffer(AllocationBuffer& buffer) const
    {
        allocBufferManager_->RegisterAllocBuffer(buffer);
    }

    inline void UnregisterAllocBuffer(AllocationBuffer& buffer) const
    {
        allocBufferManager_->UnregisterAllocBuffer(buffer);
    }

    virtual ~Allocator() {}
    Allocator();

    virtual void Init(const RuntimeParam& param) = 0;
    virtual size_t GetMaxCapacity() const = 0;
    virtual size_t GetCurrentCapacity() const = 0;
    virtual size_t GetUsedPageSize() const = 0;
    virtual HeapAddress GetSpaceStartAddress() const = 0;
    virtual HeapAddress GetSpaceEndAddress() const = 0;

    // IsHeapAddress is a range-based check, used to quickly identify heap address,
    // non-heap address never falls into this address range.
    // for more accurate check, use IsHeapObject().
    ALWAYS_INLINE_CC bool IsHeapAddress(HeapAddress addr) const { return Heap::IsHeapAddress(addr); }

#ifndef NDEBUG
    virtual bool IsHeapObject(HeapAddress) const = 0;
#endif

    template <typename AllocBufferVisitor>
    void VisitAllocBuffers(const AllocBufferVisitor& visitor) { allocBufferManager_->VisitAllocBuffers(visitor); }
    void AddHungryBuffer(AllocationBuffer& buffer) { allocBufferManager_->AddHungryBuffer(buffer); }
    void SwapHungryBuffers(AllocBufferManager::HungryBuffers &getBufferList)
    {
        allocBufferManager_->SwapHungryBuffers(getBufferList);
    }

protected:
    AllocBufferManager* allocBufferManager_;
    std::atomic<bool> isAsyncAllocationEnable_ = { true };

private:
    bool InitAyncAllocation();
    bool asyncAllocationInitSwitch_ = true;
};
} // namespace common

#endif
