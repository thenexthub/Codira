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

#ifndef COMMON_COMPONENTS_HEAP_ALLOC_BUFFER_H
#define COMMON_COMPONENTS_HEAP_ALLOC_BUFFER_H

#include <functional>

#include "common_components/mutator/thread_local.h"
#include "common_components/heap/allocator/region_list.h"
#include "common_components/common/mark_work_stack.h"

namespace common {

enum class AllocBufferType: uint8_t {
    YOUNG = 0, // for young space
    OLD,       // for old space
    TO         // for to space, valid only dring GC copy/fix phase and will become old-space later
};

// thread-local data structure
class AllocationBuffer {
public:
    AllocationBuffer() : tlRawPointerRegions_("thread-local raw-pointer regions") {}
    ~AllocationBuffer();
    void Init();
    static AllocationBuffer* GetOrCreateAllocBuffer();
    static AllocationBuffer* GetAllocBuffer();
    HeapAddress ToSpaceAllocate(size_t size);
    HeapAddress Allocate(size_t size, AllocType allocType);

    template<AllocBufferType type = AllocBufferType::YOUNG>
    RegionDesc* GetRegion()
    {
        if constexpr (type == AllocBufferType::YOUNG) {
            return tlRegion_;
        } else if constexpr (type == AllocBufferType::OLD) {
            return tlOldRegion_;
        } else if constexpr (type == AllocBufferType::TO) {
            return tlToRegion_;
        }
    }

    template <AllocBufferType type = AllocBufferType::YOUNG>
    void SetRegion(RegionDesc* newRegion)
    {
        if constexpr (type == AllocBufferType::YOUNG) {
            tlRegion_ = newRegion;
        } else if constexpr (type == AllocBufferType::OLD) {
            tlOldRegion_ = newRegion;
        } else if constexpr (type == AllocBufferType::TO) {
            tlToRegion_ = newRegion;
        }
    }

    RegionDesc* GetPreparedRegion() { return preparedRegion_.load(std::memory_order_acquire); }

    template <AllocBufferType type>
    inline void ClearRegion()
    {
        if constexpr (type == AllocBufferType::YOUNG) {
            tlRegion_ = RegionDesc::NullRegion();
        } else if constexpr (type == AllocBufferType::OLD) {
            tlOldRegion_ = RegionDesc::NullRegion();
        } else if constexpr (type == AllocBufferType::TO) {
            tlToRegion_ = RegionDesc::NullRegion();
        }
    }

    inline void ClearRegions()
    {
        ClearRegion<AllocBufferType::YOUNG>();
        ClearRegion<AllocBufferType::OLD>();
        ClearRegion<AllocBufferType::TO>();
    }

    void ClearThreadLocalRegion();
    void Unregister();

    bool SetPreparedRegion(RegionDesc* newPreparedRegion)
    {
        RegionDesc* expect = nullptr;
        return preparedRegion_.compare_exchange_strong(expect, newPreparedRegion, std::memory_order_release,
            std::memory_order_relaxed);
    }

    // record stack roots in allocBuffer so that mutator can concurrently enumerate roots without lock.
    void PushRoot(BaseObject* root) { stackRoots_.emplace_back(root); }

    // record stack roots in allocBuffer so that mutator can concurrently enumerate roots without lock.
    void PushRoot(uint64_t* root) { taggedObjStackRoots_.emplace_back(root); }

    // move the stack roots to other container so that other threads can visit them.
    template <class Functor>
    inline void MarkStack(Functor consumer)
    {
        if (taggedObjStackRoots_.empty()) {
            return;
        }
        for (uint64_t* obj : taggedObjStackRoots_) {
            consumer(reinterpret_cast<BaseObject*>(obj));
        }
        stackRoots_.clear();
    }

    template<AllocBufferType allocType>
    HeapAddress FastAllocateInTlab(size_t size)
    {
        if constexpr (allocType == AllocBufferType::YOUNG) {
            if (LIKELY_CC(tlRegion_ != RegionDesc::NullRegion())) {
                return tlRegion_->Alloc(size);
            }
        } else if constexpr (allocType == AllocBufferType::OLD) {
            if (LIKELY_CC(tlOldRegion_ != RegionDesc::NullRegion())) {
                return tlOldRegion_->Alloc(size);
            }
        }
        return 0;
    }

    // Allocation buffer is thread local, but held in multiple mutators per thread.
    // RefCount records how many mutators holds this allocbuffer.
    void IncreaseRefCount()
    {
        refCount_++;
    }

    bool DecreaseRefCount()
    {
        return --refCount_ <= 0;
    }

    static constexpr size_t GetTLRegionOffset()
    {
        return MEMBER_OFFSET_CC(AllocationBuffer, tlRegion_);
    }

    static constexpr size_t GetTLOldRegionOffset()
    {
        return MEMBER_OFFSET_CC(AllocationBuffer, tlOldRegion_);
    }

private:
    // slow path
    HeapAddress TryAllocateOnce(size_t totalSize, AllocType allocType);
    HeapAddress AllocateImpl(size_t totalSize, AllocType allocType);
    HeapAddress AllocateRawPointerObject(size_t totalSize);

    // tlRegion in AllocBuffer is a shortcut for fast allocation.
    // we should handle failure in RegionManager
    RegionDesc* tlRegion_ = RegionDesc::NullRegion();     // managed by young-space
    RegionDesc* tlOldRegion_ = RegionDesc::NullRegion();  // managed by old-space
    // only used in ToSpaceAllocate for GC copy
    RegionDesc* tlToRegion_ = RegionDesc::NullRegion();   // managed by to-space

    std::atomic<RegionDesc*> preparedRegion_ = { nullptr };
    // allocate objects which are exposed to runtime thus can not be moved.
    // allocation context is responsible to notify collector when these objects are safe to be collected.
    RegionList tlRawPointerRegions_;
    int64_t refCount_ { 0 };
    // Record stack roots in concurrent enum phase, waiting for GC to merge these roots
    std::list<BaseObject*> stackRoots_;

    std::list<uint64_t*> taggedObjStackRoots_;
};

static_assert(AllocationBuffer::GetTLRegionOffset() == 0);
} // namespace common

#endif // COMMON_COMPONENTS_HEAP_ALLOC_BUFFER_H
