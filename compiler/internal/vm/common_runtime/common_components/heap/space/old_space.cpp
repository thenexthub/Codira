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

#include "common_components/heap/space/old_space.h"
#if defined(COMMON_SANITIZER_SUPPORT)
#include "common_components/base/asan_interface.h"
#endif

namespace common {
void OldSpace::DumpRegionStats() const
{
    size_t oldRegions =
        tlOldRegionList_.GetRegionCount() + recentFullOldRegionList_.GetRegionCount() + oldRegionList_.GetRegionCount();
    size_t oldUnits =
        tlOldRegionList_.GetUnitCount() + recentFullOldRegionList_.GetUnitCount() + oldRegionList_.GetUnitCount();
    size_t oldSize = oldUnits * RegionDesc::UNIT_SIZE;
    size_t allocFromSize = GetAllocatedSize();

    VLOG(DEBUG, "\told-regions %zu: %zu units (%zu B, alloc %zu)",
        oldRegions,  oldUnits, oldSize, allocFromSize);
}

RegionDesc* OldSpace::AllocateThreadLocalRegion(bool expectPhysicalMem)
{
    RegionDesc* region = regionManager_.TakeRegion(expectPhysicalMem, true);
    ASSERT_LOGF(!IsGcThread(), "GC thread cannot take tlOldRegion");
    if (region != nullptr) {
        DLOG(REGION, "alloc thread local old region @0x%zx+%zu type %u", region->GetRegionStart(),
             region->GetRegionAllocatedSize(),
             region->GetRegionType());
        InitRegionPhaseLine(region);
        tlOldRegionList_.PrependRegion(region, RegionDesc::RegionType::THREAD_LOCAL_OLD_REGION);
    }
    return region;
}
} // namespace common
