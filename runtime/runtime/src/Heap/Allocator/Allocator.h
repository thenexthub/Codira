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


#ifndef MRT_ALLOCATOR_H
#define MRT_ALLOCATOR_H

#include "AllocBufferManager.h"
#include "Heap/GcThreadPool.h"
#include "Heap/Heap.h"

namespace MapleRuntime {
// Allocator abstract class
class Allocator {
public:
    static constexpr size_t ALLOC_ALIGN = 8;
    static constexpr size_t HEADER_SIZE = 0; // no header for cangjie object

    static Allocator* NewAllocator();

    virtual MAddress Allocate(size_t size, AllocType allocType) = 0;
    virtual bool ForEachObj(const std::function<void(BaseObject*)>&, bool safe) const = 0;

    // release physical pages of garbage memory.
    virtual size_t ReclaimGarbageMemory(bool releaseAll) = 0;
#if defined(__EULER__)
    virtual void TryReclaimGarbageMemory() = 0;
#endif
    virtual void FeedHungryBuffers() = 0;

    // returns the total size of live large objects, excluding alignment/roundup/header, ...
    // LargeObjects() is missing.
    virtual size_t LargeObjectBytes() const = 0;

    // returns the total bytes that has been allocated, including alignment/roundup/header, ...
    // allocated bytes for large objects are included.
    virtual size_t AllocatedBytes() const = 0;

    inline void RegisterAllocBuffer(AllocBuffer& buffer) const { allocBufferManager->RegisterAllocBuffer(buffer); }

    inline void RemoveAllocBuffer(AllocBuffer& buffer) const { allocBufferManager->RemoveAllocBuffer(buffer); }

    virtual ~Allocator() {}
    Allocator();

    virtual void Init(const HeapParam&) = 0;
    virtual size_t GetMaxCapacity() const = 0;
    virtual size_t GetCurrentCapacity() const = 0;
    virtual size_t GetUsedPageSize() const = 0;
    virtual MAddress GetSpaceStartAddress() const = 0;
    virtual MAddress GetSpaceEndAddress() const = 0;
    void EnableAsyncAllocation(bool enableAyncAlloc)
    {
        isAsyncAllocationEnable.store(enableAyncAlloc & asyncAllocationInitSwitch, std::memory_order_release);
    }

    bool IsAsyncAllocationEnable()
    {
        return isAsyncAllocationEnable.load(std::memory_order_acquire);
    }
    // IsHeapAddress is a range-based check, used to quickly identify heap address,
    // non-heap address never falls into this address range.
    // for more accurate check, use IsHeapObject().
    ALWAYS_INLINE bool IsHeapAddress(MAddress addr) const { return Heap::IsHeapAddress(addr); }

#if defined(MRT_DEBUG) && (MRT_DEBUG == 1)
    virtual bool IsHeapObject(MAddress) const = 0;
#endif

    void VisitAllocBuffers(const AllocBufferVisitor& visitor) { allocBufferManager->VisitAllocBuffers(visitor); }
    size_t GetAllocBufersCount() { return allocBufferManager->GetAllocBufersCount(); }
    void AddHungryBuffer(AllocBuffer& buffer) { allocBufferManager->AddHungryBuffer(buffer); }
    void SwapHungryBuffers(AllocBufferManager::HungryBuffers &getBufferList)
    {
        allocBufferManager->SwapHungryBuffers(getBufferList);
    }

protected:
    AllocBufferManager* allocBufferManager;
    std::atomic<bool> isAsyncAllocationEnable = { true };
private:
    bool InitAyncAllocation();
    bool asyncAllocationInitSwitch = true;
};
} // namespace MapleRuntime
#endif // MRT_ALLOCATOR_H
