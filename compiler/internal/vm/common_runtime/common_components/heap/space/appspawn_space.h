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
#ifndef COMMON_COMPONENTS_HEAP_SPACE_APPSPAWN_SPACE_H
#define COMMON_COMPONENTS_HEAP_SPACE_APPSPAWN_SPACE_H

#include "common_components/heap/allocator/region_manager.h"
#include "common_components/heap/space/regional_space.h"
#include "common_components/heap/allocator/region_desc.h"

namespace common {
class AppSpawnSpace : public RegionalSpace {
public:
    AppSpawnSpace(RegionManager& regionManager)
        : RegionalSpace(regionManager),
          appSpawnRegionList_("appSpawn regions") {}

    size_t GetUsedUnitCount() const
    {
        return appSpawnRegionList_.GetUnitCount();
    }

    size_t GetAllocatedSize() const
    {
        return appSpawnRegionList_.GetAllocatedSize();
    }

    void ReassembleAppspawnSpace(RegionList& regionList)
    {
        appSpawnRegionList_.MergeRegionList(regionList, RegionDesc::RegionType::APPSPAWN_REGION);
    }

    void ClearAllGCInfo()
    {
        ClearGCInfo(appSpawnRegionList_);
    }

    void MarkRememberSet(const std::function<void(BaseObject*)>& func)
    {
        auto visitFunc = [&func](RegionDesc* region) {
            region->VisitRememberSetBeforeMarking(func);
        };
        appSpawnRegionList_.VisitAllRegions(visitFunc);
    }

    void ClearRSet()
    {
        appSpawnRegionList_.VisitAllRegions([](RegionDesc* region) { region->ClearRSet(); });
    }

    void CollectFixTasks(FixHeapTaskList& taskList)
    {
        if (Heap::GetHeap().GetGCReason() == GC_REASON_YOUNG) {
            FixHeapWorker::CollectFixHeapTasks(taskList, appSpawnRegionList_, FIX_OLD_REGION);
        } else {
            FixHeapWorker::CollectFixHeapTasks(taskList, appSpawnRegionList_, FIX_REGION);
        }
    }

    void DumpRegionStats() const
    {
        appSpawnRegionList_.DumpRegionSummary();
    }

private:
    RegionList appSpawnRegionList_;
};
} // namespace common
#endif // COMMON_COMPONENTS_HEAP_SPACE_APPSPAWN_SPACE_H