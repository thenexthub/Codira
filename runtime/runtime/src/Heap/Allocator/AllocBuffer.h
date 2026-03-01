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


#ifndef MRT_ALLOC_BUFFER_H
#define MRT_ALLOC_BUFFER_H

#include <functional>

#include "Common/MarkWorkStack.h"
#include "RegionList.h"

namespace MapleRuntime {
// thread-local data structure
class AllocBuffer {
public:
    AllocBuffer() : tlRawPointerRegions("thread-local raw-pointer regions"),
                    tlLargeRawPointerRegions("thread-local large raw-pointer regions") {}
    ~AllocBuffer();
    void Init();
    void Fini();
    static AllocBuffer* GetOrCreateAllocBuffer();
    static AllocBuffer* GetAllocBuffer();

    MAddress Allocate(size_t size, AllocType allocType);
    RegionInfo* GetRegion() { return tlRegion; }
    RegionInfo* GetPreparedRegion() { return preparedRegion.load(std::memory_order_relaxed); }
    void SetRegion(RegionInfo* newRegion) { tlRegion = newRegion; }
    inline void ClearRegion()
    {
        if (tlRegion == RegionInfo::NullRegion()) {
            return;
        }
        DLOG(REGION, "alloc buffer clear tlRegion %p@[0x%zx, 0x%zx)", tlRegion, tlRegion->GetRegionStart(),
             tlRegion->GetRegionEnd());
        tlRegion = RegionInfo::NullRegion();
    }

    bool SetPreparedRegion(RegionInfo* newPreparedRegion)
    {
        RegionInfo* expect = nullptr;
        return preparedRegion.compare_exchange_strong(expect, newPreparedRegion, std::memory_order_release);
    }
    void CommitRawPointerRegions();

    // record stack roots in allocBuffer so that mutator can concurrently enumerate roots without lock.
    void PushRoot(BaseObject* root) { stackRoots.emplace_back(root); }

    // move the stack roots to other container so that other threads can visit them.
    template<class WorkStack>
    inline void MergeRoots(WorkStack& workStack)
    {
        if (stackRoots.empty()) {
            return;
        }
        for (BaseObject* obj : stackRoots) {
            workStack.push_back(obj);
        }
        stackRoots.clear();
    }

private:
    // slow path
    MAddress TryAllocateOnce(size_t totalSize, AllocType allocType);
    MAddress AllocateImpl(size_t totalSize, AllocType allocType);
    MAddress AllocateRawPointerObject(size_t totalSize);

    // tlRegion in AllocBuffer is a shortcut for fast allocation.
    // we should handle failure in RegionManager
    RegionInfo* tlRegion = RegionInfo::NullRegion();

    std::atomic<RegionInfo*> preparedRegion = { nullptr };
    // allocate objects which are exposed to runtime thus can not be moved.
    // allocation context is responsible to notify collector when these objects are safe to be collected.
    RegionList tlRawPointerRegions;
    RegionList tlLargeRawPointerRegions;
    // Record stack roots in concurrent enum phase, waiting for GC to merge these roots
    std::list<BaseObject*> stackRoots;
};
} // namespace MapleRuntime
#endif // MRT_ALLOC_BUFFER_H
