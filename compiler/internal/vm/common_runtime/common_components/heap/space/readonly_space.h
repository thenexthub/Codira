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
#ifndef COMMON_COMPONENTS_HEAP_SPACE_READONLY_SPACE_H
#define COMMON_COMPONENTS_HEAP_SPACE_READONLY_SPACE_H

#include "common_components/heap/allocator/region_manager.h"
#include "common_components/heap/space/regional_space.h"

namespace common {
// regions for read only objects
class ReadOnlySpace : public RegionalSpace {
public:
    ReadOnlySpace(RegionManager& regionManager) : RegionalSpace(regionManager),
        readOnlyRegionList_("read only region") {}

    size_t GetUsedUnitCount() const
    {
        return readOnlyRegionList_.GetUnitCount();
    }

    size_t GetAllocatedSize() const
    {
        return readOnlyRegionList_.GetAllocatedSize();
    }

    void PrepareMarking()
    {
        RegionDesc* readOnlyRegion = readOnlyRegionList_.GetHeadRegion();
        if (readOnlyRegion != nullptr && readOnlyRegion != RegionDesc::NullRegion()) {
            readOnlyRegion->SetMarkingLine();
        }
    }

    void PrepareForward()
    {
        RegionDesc* readOnlyRegion = readOnlyRegionList_.GetHeadRegion();
        if (readOnlyRegion != nullptr && readOnlyRegion != RegionDesc::NullRegion()) {
            readOnlyRegion->SetCopyLine();
        }
    }

    void ClearAllGCInfo()
    {
        ClearGCInfo(readOnlyRegionList_);
    }

    void SetReadOnlyToRORegionList()
    {
        auto visitor = [](RegionDesc* region) {
            if (region != nullptr) {
                region->SetReadOnly();
            }
        };
        readOnlyRegionList_.VisitAllRegions(visitor);
    }

    void ClearReadOnlyFromRORegionList()
    {
        auto visitor = [](RegionDesc* region) {
            if (region != nullptr) {
                region->ClearReadOnly();
            }
        };
        readOnlyRegionList_.VisitAllRegions(visitor);
    }

    void DumpRegionStats() const
    {
        readOnlyRegionList_.DumpRegionSummary();
    }

    uintptr_t Alloc(size_t size, bool allowGC = true)
    {
        uintptr_t addr = 0;
        std::mutex& regionListMutex = readOnlyRegionList_.GetListMutex();

        std::lock_guard<std::mutex> lock(regionListMutex);
        RegionDesc* headRegion = readOnlyRegionList_.GetHeadRegion();
        if (headRegion != nullptr) {
            addr = headRegion->Alloc(size);
        }
        if (addr == 0) {
            RegionDesc* region =
                regionManager_.TakeRegion(1, RegionDesc::UnitRole::SMALL_SIZED_UNITS, false, allowGC);
            if (region == nullptr) {
                return 0;
            }
            DLOG(REGION, "alloc read only region @0x%zx+%zu type %u", region->GetRegionStart(),
                 region->GetRegionAllocatedSize(),
                 region->GetRegionType());

            InitRegionPhaseLine(region);

            // To make sure the allocedSize are consistent, it must prepend region first then alloc object.
            readOnlyRegionList_.PrependRegionLocked(region, RegionDesc::RegionType::READ_ONLY_REGION);
            addr = region->Alloc(size);
        }

        DLOG(ALLOC, "alloc read only obj 0x%zx(%zu)", addr, size);
        return addr;
    }

private:
    // regions for read only objects
    RegionList readOnlyRegionList_;
};
} // namespace common
#endif // COMMON_COMPONENTS_HEAP_SPACE_READONLY_SPACE_H