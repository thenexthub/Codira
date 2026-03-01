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
#ifndef COMMON_COMPONENTS_HEAP_YOUNG_SPACE_H
#define COMMON_COMPONENTS_HEAP_YOUNG_SPACE_H

#include <assert.h>
#include <list>
#include <map>
#include <set>
#include <thread>
#include <vector>

#include "common_components/heap/allocator/alloc_util.h"
#include "common_components/heap/allocator/allocator.h"
#include "common_components/heap/allocator/region_manager.h"
#include "common_components/heap/space/regional_space.h"
#include "common_components/heap/space/from_space.h"
#include "common_components/mutator/mutator.h"
#include "common_components/heap/allocator/fix_heap.h"
#include "common_components/heap/allocator/region_desc.h"
#if defined(COMMON_SANITIZER_SUPPORT)
#include "common_components/base/asan_interface.h"
#endif

namespace common {
// regions for small-sized movable objects, which may be moved during gc.
class YoungSpace : public RegionalSpace {
public:
    YoungSpace(RegionManager& regionManager) : RegionalSpace(regionManager),
        tlRegionList_("thread local regions"),
        recentFullRegionList_("recent full regions") {}

    void DumpRegionStats() const;

    void CollectFixTasks(FixHeapTaskList &taskList)
    {
        std::lock_guard<std::mutex> lock(lock_);
        FixHeapWorker::CollectFixHeapTasks(taskList, tlRegionList_, FIX_RECENT_REGION);
        FixHeapWorker::CollectFixHeapTasks(taskList, recentFullRegionList_, FIX_RECENT_REGION);
    }

    RegionDesc* AllocateThreadLocalRegion(bool expectPhysicalMem);

    void AddFullRegion(RegionDesc* region)
    {
        recentFullRegionList_.PrependRegion(region, RegionDesc::RegionType::RECENT_FULL_REGION);
    }

    void HandleFullThreadLocalRegion(RegionDesc* region)
    {
        std::lock_guard<std::mutex> lock(lock_);
        DCHECK_CC(region->IsThreadLocalRegion());
        tlRegionList_.DeleteRegion(region);
        recentFullRegionList_.PrependRegion(region, RegionDesc::RegionType::RECENT_FULL_REGION);
    }

    void AssembleGarbageCandidates(FromSpace& fromSpace)
    {
        fromSpace.AssembleGarbageCandidates(recentFullRegionList_);
    }

    size_t GetUsedUnitCount() const
    {
        return tlRegionList_.GetUnitCount() + recentFullRegionList_.GetUnitCount();
    }

    size_t GetAllocatedSize() const
    {
        return tlRegionList_.GetAllocatedSize(false) + recentFullRegionList_.GetAllocatedSize();
    }

    size_t GetRecentAllocatedSize() const
    {
        return recentFullRegionList_.GetAllocatedSize();
    }

    void ClearAllGCInfo()
    {
        std::lock_guard<std::mutex> lock(lock_);
        ClearGCInfo(tlRegionList_);
        ClearGCInfo(recentFullRegionList_);
    }

    RegionList& GetTlRegionList() noexcept { return tlRegionList_; }

    RegionList& GetRecentFullRegionList() noexcept { return recentFullRegionList_; }

private:
    void ClearGCInfo(RegionList& list)
    {
        RegionList tmp("temp region list");
        list.CopyListTo(tmp);
        tmp.VisitAllRegions([](RegionDesc* region) {
            region->ClearMarkingCopyLine();
            region->ClearLiveInfo();
            region->ResetMarkBit();
        });
    }
    // Used to exclude the promotion of TLS regions during the ClearGCInfo phase
    // This is necessary when the mutator can promote TL to recentFull while the GC is performing ClearGC
    std::mutex lock_;

    // regions for thread-local allocation.
    // regions in this list are already used for allocation but not full yet.
    RegionList tlRegionList_;

    // recentFullRegionList is a list of regions which become full .
    RegionList recentFullRegionList_;
};
} // namespace common
#endif // COMMON_COMPONENTS_HEAP_YOUNG_SPACE_H
