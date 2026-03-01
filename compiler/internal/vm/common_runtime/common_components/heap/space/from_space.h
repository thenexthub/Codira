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
#ifndef COMMON_COMPONENTS_HEAP_SPACE_FROM_SPACE_H
#define COMMON_COMPONENTS_HEAP_SPACE_FROM_SPACE_H

#include <assert.h>
#include <list>
#include <map>
#include <set>
#include <thread>
#include <vector>

#include "common_components/heap/allocator/alloc_util.h"
#include "common_components/heap/allocator/allocator.h"
#include "common_components/heap/allocator/region_manager.h"
#include "common_components/mutator/mutator.h"
#include "common_components/heap/allocator/fix_heap.h"
#include "common_components/heap/space/regional_space.h"
#if defined(COMMON_SANITIZER_SUPPORT)
#include "common_components/base/asan_interface.h"
#endif

namespace common {
class RegionalHeap;
class OldSpace;
class Taskpool;

// regions for small-sized movable objects, which may be moved during gc.
class FromSpace : public RegionalSpace {
public:
    FromSpace(RegionManager& regionManager, RegionalHeap& heap) : RegionalSpace(regionManager),
        fromRegionList_("from-regions"),
        exemptedFromRegionList_("exempted from-regions"), heap_(heap), exemptedRegionThreshold_(0) {}

    void DumpRegionStats() const;

    void AssembleGarbageCandidates(RegionList& list)
    {
        fromRegionList_.MergeRegionList(list, RegionDesc::RegionType::FROM_REGION);
    }

    void ExemptFromRegions();

    void CollectFixTasks(FixHeapTaskList &taskList)
    {
        FixHeapWorker::CollectFixHeapTasks(taskList, exemptedFromRegionList_, FIX_REGION);
    }

    size_t GetUsedUnitCount() const
    {
        return fromRegionList_.GetUnitCount() + exemptedFromRegionList_.GetUnitCount();
    }

    size_t GetAllocatedSize() const
    {
        return fromRegionList_.GetAllocatedSize() + exemptedFromRegionList_.GetAllocatedSize();
    }

    size_t GetFromRegionAllocatedSize() const
    {
        return fromRegionList_.GetAllocatedSize();
    }

    RegionList& GetFromRegionList() { return fromRegionList_; }

    bool TryDeleteFromRegion(RegionDesc* del, RegionDesc::RegionType oldType, RegionDesc::RegionType newType)
    {
        return fromRegionList_.TryDeleteRegion(del, oldType, newType);
    }

    void DeleteFromRegion(RegionDesc* region)
    {
        fromRegionList_.DeleteRegion(region);
    }

    void ExemptFromRegion(RegionDesc* region)
    {
        exemptedFromRegionList_.PrependRegion(region, RegionDesc::RegionType::EXEMPTED_FROM_REGION);
    }

    size_t GetSurvivedSize() const
    {
        return exemptedFromRegionList_.GetAllocatedSize();
    }

    RegionalHeap& GetHeap() { return heap_; }

    void ParallelCopyFromRegions(RegionDesc* startRegion, size_t regionCnt);
    void CopyFromRegions(Taskpool* threadPool);
    void CopyFromRegions();

    void GetPromotedTo(OldSpace& mspace);

    void SetExemptedRegionThreshold(double threshold)
    {
        exemptedRegionThreshold_ = threshold;
    }

    void ClearAllGCInfo()
    {
        ClearGCInfo(exemptedFromRegionList_);
        RegionDesc* region = fromRegionList_.GetHeadRegion();
        while (region != nullptr) {
            region->ClearRSet();
            region = region->GetNextRegion();
        }
    }

    RegionList& GetExemptedRegionList() noexcept { return exemptedFromRegionList_; }

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
    // fromRegionList is a list of full regions waiting to be collected (i.e. for forwarding).
    // region type must be FROM_REGION.
    RegionList fromRegionList_;

    // regions exempted by ExemptFromRegions, which will not be moved during current GC.
    RegionList exemptedFromRegionList_;

    RegionalHeap& heap_;

    double exemptedRegionThreshold_;
};
} // namespace common
#endif // COMMON_COMPONENTS_HEAP_SPACE_FROM_SPACE_H
