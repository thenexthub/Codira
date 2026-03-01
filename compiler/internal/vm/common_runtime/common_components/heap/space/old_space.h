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
#ifndef COMMON_COMPONENTS_HEAP_SPACE_OLD_SPACE_H
#define COMMON_COMPONENTS_HEAP_SPACE_OLD_SPACE_H

#include "common_components/heap/allocator/region_manager.h"
#include "common_components/heap/space/regional_space.h"
#include "common_components/heap/space/from_space.h"
#include "common_components/heap/space/to_space.h"
#include "common_components/mutator/mutator.h"
#include "common_components/heap/allocator/fix_heap.h"
#if defined(COMMON_SANITIZER_SUPPORT)
#include "common_components/base/asan_interface.h"
#endif

namespace common {
// regions for small-sized movable objects, which may be moved during gc.
class OldSpace : public RegionalSpace {
public:
    OldSpace(RegionManager& regionManager)
        : RegionalSpace(regionManager),
          tlOldRegionList_("thread-local old regions"),
          recentFullOldRegionList_("recent full old regions"),
          oldRegionList_("old regions") {}

    void DumpRegionStats() const;

    void CollectFixTasks(FixHeapTaskList &taskList)
    {
        if (Heap::GetHeap().GetGCReason() == GC_REASON_YOUNG) {
            FixHeapWorker::CollectFixHeapTasks(taskList, oldRegionList_, FIX_OLD_REGION);
            std::lock_guard<std::mutex> lock(lock_);
            FixHeapWorker::CollectFixHeapTasks(taskList, tlOldRegionList_, FIX_RECENT_OLD_REGION);
            FixHeapWorker::CollectFixHeapTasks(taskList, recentFullOldRegionList_, FIX_RECENT_OLD_REGION);
        } else {
            FixHeapWorker::CollectFixHeapTasks(taskList, oldRegionList_, FIX_REGION);
            std::lock_guard<std::mutex> lock(lock_);
            FixHeapWorker::CollectFixHeapTasks(taskList, tlOldRegionList_, FIX_RECENT_REGION);
            FixHeapWorker::CollectFixHeapTasks(taskList, recentFullOldRegionList_, FIX_RECENT_REGION);
        }
    }

    void AssembleGarbageCandidates(FromSpace& fromSpace)
    {
        fromSpace.AssembleGarbageCandidates(oldRegionList_);
    }

    size_t GetAllocatedSize() const
    {
        return tlOldRegionList_.GetAllocatedSize(false) + recentFullOldRegionList_.GetAllocatedSize() +
               oldRegionList_.GetAllocatedSize();
    }

    size_t GetUsedUnitCount() const
    {
        return tlOldRegionList_.GetRegionCount() + recentFullOldRegionList_.GetRegionCount() +
               oldRegionList_.GetUnitCount();
    }

    void PromoteRegionList(RegionList& list)
    {
        oldRegionList_.MergeRegionList(list, RegionDesc::RegionType::OLD_REGION);
    }

    void AssembleRecentFull()
    {
        oldRegionList_.MergeRegionList(recentFullOldRegionList_, RegionDesc::RegionType::OLD_REGION);
    }

    void ClearRSet()
    {
        ClearRSet(tlOldRegionList_.GetHeadRegion());
        ClearRSet(recentFullOldRegionList_.GetHeadRegion());
        ClearRSet(oldRegionList_.GetHeadRegion());
    }

    void ClearAllGCInfo()
    {
        ClearGCInfo(oldRegionList_);
        std::lock_guard<std::mutex> lock(lock_);
        ClearGCInfo(tlOldRegionList_);
        ClearGCInfo(recentFullOldRegionList_);
    }

    void MarkRememberSet(const std::function<void(BaseObject*)>& func)
    {
        auto visitFunc = [&func](RegionDesc* region) {
            region->VisitRememberSet(func);
        };

        auto visitRecentFunc = [&func](RegionDesc* region) {
            region->VisitRememberSetBeforeMarking(func);
        };

        // Need to visit objects allocated before current GC iteration, markingline is used to distinguish them.
        tlOldRegionList_.VisitAllRegions(visitRecentFunc);
        recentFullOldRegionList_.VisitAllRegions(visitRecentFunc);
        oldRegionList_.VisitAllRegions(visitFunc);
    }

    uintptr_t AllocFullRegion()
    {
        RegionDesc* region = regionManager_.TakeRegion(false, false);
        DCHECK_CC(region != nullptr);

        InitRegionPhaseLine(region);

        DLOG(REGION, "alloc small object region %p @0x%zx+%zu units[%zu+%zu, %zu) type %u",
            region, region->GetRegionStart(), region->GetRegionSize(),
            region->GetUnitIdx(), region->GetUnitCount(), region->GetUnitIdx() + region->GetUnitCount(),
            region->GetRegionType());
        AddFullRegion(region);

        uintptr_t start = region->GetRegionStart();
        uintptr_t addr = region->Alloc(region->GetRegionEnd() - region->GetRegionAllocPtr());
        (void)addr;
        DCHECK_CC(addr != 0);

        return start;
    }

    void AddFullRegion(RegionDesc *region)
    {
        recentFullOldRegionList_.PrependRegion(region, RegionDesc::RegionType::OLD_REGION);
    }

    void HandleFullThreadLocalRegion(RegionDesc* region)
    {
        std::lock_guard<std::mutex> lock(lock_);
        ASSERT_LOGF(region->GetRegionType() == RegionDesc::RegionType::THREAD_LOCAL_OLD_REGION,
                    "not thread local old region");
        tlOldRegionList_.DeleteRegion(region);
        recentFullOldRegionList_.PrependRegion(region, RegionDesc::RegionType::OLD_REGION);
    }

    RegionDesc* AllocateThreadLocalRegion(bool expectPhysicalMem);

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

    void ClearRSet(RegionDesc* region)
    {
        while (region != nullptr) {
            region->ClearRSet();
            region = region->GetNextRegion();
        }
    }
    // Used to exclude the promotion of TLS regions during the ClearGCInfo phase
    // This is necessary when the mutator can promote TL to recentFull while the GC is performing ClearGC
    std::mutex lock_;

    // regions for thread-local allocation.
    // regions in this list are already used for allocation but not full yet.
    RegionList tlOldRegionList_;

    // recentFullRegionList is a list of regions which become full .
    RegionList recentFullOldRegionList_;

    RegionList oldRegionList_;
};
} // namespace common
#endif // COMMON_COMPONENTS_HEAP_SPACE_OLD_SPACE_H
