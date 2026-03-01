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
#ifndef COMMON_COMPONENTS_HEAP_SPACE_LARGE_SPACE_H
#define COMMON_COMPONENTS_HEAP_SPACE_LARGE_SPACE_H

#include "common_interfaces/base/common.h"
#include "common_components/heap/allocator/region_manager.h"
#include "common_components/heap/space/regional_space.h"

namespace common {

// regions for large-sized objects.
class LargeSpace : public RegionalSpace {
public:
    LargeSpace(RegionManager& regionManager)
        : RegionalSpace(regionManager),
          largeRegionList_("large regions"),
          recentLargeRegionList_("recent large regions") {}

    size_t GetRecentAllocatedSize() const
    {
        return recentLargeRegionList_.GetAllocatedSize();
    }

    size_t GetSurvivedSize() const
    {
        return largeRegionList_.GetAllocatedSize();
    }

    void CollectFixTasks(FixHeapTaskList& taskList);

    uintptr_t Alloc(size_t size, bool allowGC = true);

    void AssembleGarbageCandidates();
    size_t CollectLargeGarbage();

    size_t GetUsedUnitCount() const
    {
        return largeRegionList_.GetUnitCount() + recentLargeRegionList_.GetUnitCount();
    }

    size_t GetAllocatedSize() const
    {
        return largeRegionList_.GetAllocatedSize() + recentLargeRegionList_.GetAllocatedSize();
    }

    void ClearAllGCInfo()
    {
        ClearGCInfo(largeRegionList_);
        ClearGCInfo(recentLargeRegionList_);
    }

    void MarkRememberSet(const std::function<void(BaseObject*)>& func)
    {
        auto visitFunc = [&func](RegionDesc* region) {
            region->VisitRememberSetBeforeMarking(func);
        };
        recentLargeRegionList_.VisitAllRegions(visitFunc);
        largeRegionList_.VisitAllRegions(visitFunc);
    }

    void ClearRSet()
    {
        auto clearFunc = [](RegionDesc* region) {
            region->ClearRSet();
        };
        recentLargeRegionList_.VisitAllRegions(clearFunc);
        largeRegionList_.VisitAllRegions(clearFunc);
    }

    void DumpRegionStats() const
    {
        largeRegionList_.DumpRegionSummary();
        recentLargeRegionList_.DumpRegionSummary();
    }

private:
    // large region which allocated before last GC
    RegionList largeRegionList_;

    // large regions which allocated since last GC beginning.
    // record large regions in here first and move those to largeRegionList_ when gc starts.
    RegionList recentLargeRegionList_;
};
} // namespace common
#endif // COMMON_COMPONENTS_HEAP_SPACE_LARGE_SPACE_H