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
#ifndef COMMON_COMPONENTS_HEAP_SPACE_RAWPOINTER_SPACE_H
#define COMMON_COMPONENTS_HEAP_SPACE_RAWPOINTER_SPACE_H

#include "common_components/heap/allocator/region_manager.h"
#include "common_components/heap/space/regional_space.h"

namespace common {
class RawPointerSpace : public RegionalSpace {
public:
    RawPointerSpace(RegionManager& regionManager)
        : RegionalSpace(regionManager),
          rawPointerRegionList_("raw pointer regions") {}

    void AddRawPointerRegion(RegionDesc* region)
    {
        rawPointerRegionList_.PrependRegion(region, RegionDesc::RegionType::RAW_POINTER_REGION);
    }

    size_t GetUsedUnitCount() const
    {
        return rawPointerRegionList_.GetUnitCount();
    }

    size_t GetAllocatedSize() const
    {
        return rawPointerRegionList_.GetAllocatedSize();
    }

    void ClearAllGCInfo()
    {
        ClearGCInfo(rawPointerRegionList_);
    }

    void MarkRememberSet(const std::function<void(BaseObject*)>& func)
    {
        auto visitFunc = [&func](RegionDesc* region) {
            region->VisitRememberSetBeforeMarking(func);
        };
        rawPointerRegionList_.VisitAllRegions(visitFunc);
    }

    void ClearRSet()
    {
        rawPointerRegionList_.VisitAllRegions([](RegionDesc* region) { region->ClearRSet(); });
    }

    void CollectFixTasks(FixHeapTaskList& taskList)
    {
        if (Heap::GetHeap().GetGCReason() == GC_REASON_YOUNG) {
            FixHeapWorker::CollectFixHeapTasks(taskList, rawPointerRegionList_, FIX_OLD_REGION);
        } else {
            FixHeapWorker::CollectFixHeapTasks(taskList, rawPointerRegionList_, FIX_REGION);
        }
    }

    void DumpRegionStats() const
    {
        rawPointerRegionList_.DumpRegionSummary();
    }

private:

    // region lists for small-sized raw-pointer objects (i.e. future, monitor)
    // which can not be moved ever (even during compaction).
    RegionList rawPointerRegionList_; // delete rawPointerRegion, use PinnedRegion
};
} // namespace common
#endif // COMMON_COMPONENTS_HEAP_SPACE_RAWPOINTER_SPACE_H