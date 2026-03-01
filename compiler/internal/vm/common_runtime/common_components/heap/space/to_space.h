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
#ifndef COMMON_COMPONENTS_HEAP_SPACE_TO_SPACE_H
#define COMMON_COMPONENTS_HEAP_SPACE_TO_SPACE_H

#include <assert.h>
#include <list>
#include <map>
#include <set>
#include <thread>
#include <vector>

#include "common_components/heap/allocator/alloc_util.h"
#include "common_components/heap/allocator/allocator.h"
#include "common_components/heap/allocator/region_list.h"
#include "common_components/heap/allocator/region_manager.h"
#include "common_components/heap/space/regional_space.h"
#include "common_components/heap/space/from_space.h"
#include "common_components/mutator/mutator.h"
#if defined(COMMON_SANITIZER_SUPPORT)
#include "common_components/base/asan_interface.h"
#endif

namespace common {
class OldSpace;
// regions for small-sized movable objects, which may be moved during gc.
class ToSpace : public RegionalSpace {
public:
    ToSpace(RegionManager& regionManager) : RegionalSpace(regionManager),
        tlToRegionList_("thread local to-regions"),
        fullToRegionList_("full to-regions") {}

    void DumpRegionStats() const;

    void CollectFixTasks(FixHeapTaskList &taskList)
    {
        FixHeapWorker::CollectFixHeapTasks(taskList, tlToRegionList_, FIX_TO_REGION);
        FixHeapWorker::CollectFixHeapTasks(taskList, fullToRegionList_, FIX_TO_REGION);
    }

    void AddFullRegion(RegionDesc* region)
    {
        DCHECK_CC(Heap::GetHeap().IsGcStarted());
        fullToRegionList_.PrependRegion(region, RegionDesc::RegionType::TO_REGION);
    }

    RegionDesc* AllocateThreadLocalRegion(bool expectPhysicalMem);

    void HandleFullThreadLocalRegion(RegionDesc* region)
    {
        DCHECK_CC(Heap::GetHeap().IsGcStarted());
        DCHECK_CC(region->IsToRegion());
        tlToRegionList_.DeleteRegion(region);
        fullToRegionList_.PrependRegion(region, RegionDesc::RegionType::TO_REGION);
    }

    size_t GetAllocatedSize() const
    {
        return tlToRegionList_.GetAllocatedSize(false) + fullToRegionList_.GetAllocatedSize();
    }

    size_t GetSurvivedSize() const
    {
        return tlToRegionList_.GetAllocatedSize(false) + fullToRegionList_.GetAllocatedSize();
    }

    size_t GetUsedUnitCount() const
    {
        return tlToRegionList_.GetUnitCount() + fullToRegionList_.GetUnitCount();
    }

    void GetPromotedTo(OldSpace& mspace);

    void ClearAllGCInfo()
    {
        ClearGCInfo(tlToRegionList_);
        ClearGCInfo(fullToRegionList_);
    }

    RegionList& GetTlToRegionList() noexcept { return tlToRegionList_; }

    RegionList& GetFullToRegionList() noexcept { return fullToRegionList_; }

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

    // toRegionList is a list of to-space regions produced by gc threads.
    // when a region is prepended to this list, the region is probably not full, so the statistics
    // of this region-list are not reliable and need to be updated.
    RegionList tlToRegionList_;
    RegionList fullToRegionList_;
};
} // namespace common
#endif // COMMON_COMPONENTS_HEAP_SPACE_TO_SPACE_H
