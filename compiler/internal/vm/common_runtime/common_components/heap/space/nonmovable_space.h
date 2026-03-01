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
#ifndef COMMON_COMPONENTS_HEAP_SPACE_NONMOVABLE_SPACE_H
#define COMMON_COMPONENTS_HEAP_SPACE_NONMOVABLE_SPACE_H

#include "common_components/heap/allocator/region_manager.h"
#include "common_components/heap/space/regional_space.h"
#include "common_components/mutator/mutator.h"
#include "common_components/heap/allocator/fix_heap.h"
#if defined(COMMON_SANITIZER_SUPPORT)
#include "common_components/base/asan_interface.h"
#endif

namespace common {
// regions for non movable objects
class NonMovableSpace : public RegionalSpace {
public:
    NonMovableSpace(RegionManager& regionManager)
        : RegionalSpace(regionManager),
          polySizeRegionList_("mixed size non-movable regions"),
          recentPolySizeRegionList_("recent mixed size non-movable regions") {
        for (size_t i = 0; i < NONMOVABLE_OBJECT_SIZE_COUNT; i++) {
            recentMonoSizeRegionList_[i] = new RegionList("recent one size non-movable regions");
            monoSizeRegionList_[i] = new RegionList("one size non-movable regions");
        }
    }

    ~NonMovableSpace()
    {
        for (size_t i = 0; i < NONMOVABLE_OBJECT_SIZE_COUNT; i++) {
            if (recentMonoSizeRegionList_[i] != nullptr) {
                delete recentMonoSizeRegionList_[i];
                recentMonoSizeRegionList_[i] = nullptr;
            }
            if (monoSizeRegionList_[i] != nullptr) {
                delete monoSizeRegionList_[i];
                monoSizeRegionList_[i] = nullptr;
            }
        }
    }

    void CollectFixTasks(FixHeapTaskList& taskList);

    uintptr_t AllocInMonoSizeList(size_t size);
    uintptr_t Alloc(size_t size, bool allowGC = true);
    uintptr_t AllocInPolySizeList(size_t size, bool allowGC = true);

    uintptr_t AllocFullRegion();

    void AssembleGarbageCandidates();
    void MarkRememberSet(const std::function<void(BaseObject*)>& func);
    void DumpRegionStats() const;

    size_t GetRecentAllocatedSize() const
    {
        return recentPolySizeRegionList_.GetAllocatedSize();
    }

    size_t GetSurvivedSize() const
    {
        return polySizeRegionList_.GetAllocatedSize();
    }

    size_t GetUsedUnitCount() const
    {
        size_t nonMovableUnitCount =
            polySizeRegionList_.GetUnitCount() + recentPolySizeRegionList_.GetUnitCount();
        for (size_t i = 0; i < NONMOVABLE_OBJECT_SIZE_COUNT; i++) {
            nonMovableUnitCount += recentMonoSizeRegionList_[i]->GetUnitCount();
            nonMovableUnitCount += monoSizeRegionList_[i]->GetUnitCount();
        }
        return nonMovableUnitCount;
    }

    size_t GetAllocatedSize() const
    {
        size_t nonMovableSpaceSize =
            polySizeRegionList_.GetAllocatedSize() + recentPolySizeRegionList_.GetAllocatedSize();
        for (size_t i = 0; i < NONMOVABLE_OBJECT_SIZE_COUNT; i++) {
            nonMovableSpaceSize += recentMonoSizeRegionList_[i]->GetAllocatedSize();
            nonMovableSpaceSize += monoSizeRegionList_[i]->GetAllocatedSize();
        }
        return nonMovableSpaceSize;
    }

    void PrepareMarking()
    {
        RegionDesc* region = recentPolySizeRegionList_.GetHeadRegion();
        if (region != nullptr && region != RegionDesc::NullRegion()) {
            region->SetMarkingLine();
        }

        for (size_t i = 0; i < NONMOVABLE_OBJECT_SIZE_COUNT; i++) {
            RegionDesc* region = recentMonoSizeRegionList_[i]->GetHeadRegion();
            if (region != nullptr && region != RegionDesc::NullRegion()) {
                region->SetMarkingLine();
            }
        }
    }

    void PrepareForward()
    {
        RegionDesc* region = recentPolySizeRegionList_.GetHeadRegion();
        if (region != nullptr && region != RegionDesc::NullRegion()) {
            region->SetCopyLine();
        }

        for (size_t i = 0; i < NONMOVABLE_OBJECT_SIZE_COUNT; i++) {
            RegionDesc* region = recentMonoSizeRegionList_[i]->GetHeadRegion();
            if (region != nullptr && region != RegionDesc::NullRegion()) {
                region->SetCopyLine();
            }
        }
    }

    void ClearAllGCInfo()
    {
        ClearGCInfo(recentPolySizeRegionList_);
        ClearGCInfo(polySizeRegionList_);
        for (size_t i = 0; i < NONMOVABLE_OBJECT_SIZE_COUNT; i++) {
            ClearGCInfo(*recentMonoSizeRegionList_[i]);
            ClearGCInfo(*monoSizeRegionList_[i]);
        }
    }

    void ClearRSet()
    {
        auto clearFunc = [](RegionDesc* region) {
            region->ClearRSet();
        };
        recentPolySizeRegionList_.VisitAllRegions(clearFunc);
        polySizeRegionList_.VisitAllRegions(clearFunc);
        for (size_t i = 0; i < NONMOVABLE_OBJECT_SIZE_COUNT; i++) {
            recentMonoSizeRegionList_[i]->VisitAllRegions(clearFunc);
            monoSizeRegionList_[i]->VisitAllRegions(clearFunc);
        }
    }

private:
    constexpr static size_t NONMOVABLE_OBJECT_SIZE_COUNT = 128;
    constexpr static size_t NONMOVABLE_OBJECT_SIZE_THRESHOLD = sizeof(uint64_t) * NONMOVABLE_OBJECT_SIZE_COUNT;

    // objects in region have multi size.
    RegionList polySizeRegionList_;
    RegionList recentPolySizeRegionList_;

    // objects in region have only one size
    RegionList* monoSizeRegionList_[NONMOVABLE_OBJECT_SIZE_COUNT];
    RegionList* recentMonoSizeRegionList_[NONMOVABLE_OBJECT_SIZE_COUNT];
};
}

#endif // COMMON_COMPONENTS_HEAP_SPACE_NONMOVABLE_SPACE_H