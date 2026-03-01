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

#include "common_components/heap/space/young_space.h"
#if defined(COMMON_SANITIZER_SUPPORT)
#include "common_components/base/asan_interface.h"
#endif

namespace common {
void YoungSpace::DumpRegionStats() const
{
    size_t tlRegions = tlRegionList_.GetRegionCount();
    size_t tlUnits = tlRegionList_.GetUnitCount();
    size_t tlSize = tlUnits * RegionDesc::UNIT_SIZE;
    size_t allocTLSize = tlRegionList_.GetAllocatedSize();

    size_t recentFullRegions = recentFullRegionList_.GetRegionCount();
    size_t recentFullUnits = recentFullRegionList_.GetUnitCount();
    size_t recentFullSize = recentFullUnits * RegionDesc::UNIT_SIZE;
    size_t allocRecentFullSize = recentFullRegionList_.GetAllocatedSize();

    size_t units = tlUnits + recentFullUnits;
    VLOG(DEBUG, "\tyoung space units: %zu (%zu B)", units, units * RegionDesc::UNIT_SIZE);
    VLOG(DEBUG, "\t  tl-regions %zu: %zu units (%zu B, alloc %zu)", tlRegions,  tlUnits, tlSize, allocTLSize);
    VLOG(DEBUG, "\t  recent-full regions %zu: %zu units (%zu B, alloc %zu)",
        recentFullRegions, recentFullUnits, recentFullSize, allocRecentFullSize);
}

RegionDesc* YoungSpace::AllocateThreadLocalRegion(bool expectPhysicalMem)
{
    RegionDesc* region = regionManager_.TakeRegion(expectPhysicalMem, true);
    ASSERT_LOGF(!IsGcThread(), "GC thread cannot take tlRegion");
    if (region != nullptr) {
        DLOG(REGION, "alloc thread local young region @0x%zx+%zu type %u", region->GetRegionStart(),
             region->GetRegionAllocatedSize(),
             region->GetRegionType());
        InitRegionPhaseLine(region);
        tlRegionList_.PrependRegion(region, RegionDesc::RegionType::THREAD_LOCAL_REGION);
    }
    return region;
}
} // namespace common
