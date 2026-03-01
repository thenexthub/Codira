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

#include "common_components/heap/space/to_space.h"
#include "common_components/heap/space/old_space.h"
#if defined(COMMON_SANITIZER_SUPPORT)
#include "common_components/base/asan_interface.h"
#endif

namespace common {
void ToSpace::DumpRegionStats() const
{
    size_t tlToRegions = tlToRegionList_.GetRegionCount();
    size_t tlToUnits = tlToRegionList_.GetUnitCount();
    size_t tlToSize = tlToUnits * RegionDesc::UNIT_SIZE;
    size_t allocTLToSize = tlToRegionList_.GetAllocatedSize(false);

    size_t fullToRegions = fullToRegionList_.GetRegionCount();
    size_t fullToUnits = fullToRegionList_.GetUnitCount();
    size_t fullToSize = fullToUnits * RegionDesc::UNIT_SIZE;
    size_t allocfullToSize = fullToRegionList_.GetAllocatedSize();

    size_t units = tlToUnits + fullToUnits;
    VLOG(DEBUG, "\tto space units: %zu (%zu B)", units, units * RegionDesc::UNIT_SIZE);
    VLOG(DEBUG, "\t  thread-local to-regions %zu: %zu units (%zu B, alloc %zu)",
        tlToRegions,  tlToUnits, tlToSize, allocTLToSize);
    VLOG(DEBUG, "\t  full to-regions %zu: %zu units (%zu B, alloc %zu)",
        fullToRegions,  fullToUnits, fullToSize, allocfullToSize);
}

void ToSpace::GetPromotedTo(OldSpace& mspace)
{
    // release thread-local to-space regions as they will promote to old-space
    AllocBufferVisitor visitor = [](AllocationBuffer& tlab) {
        tlab.ClearRegion<AllocBufferType::TO>();
    };
    Heap::GetHeap().GetAllocator().VisitAllocBuffers(visitor);

    mspace.PromoteRegionList(fullToRegionList_);
    mspace.PromoteRegionList(tlToRegionList_);
}

RegionDesc* ToSpace::AllocateThreadLocalRegion(bool expectPhysicalMem)
{
    RegionDesc* region = regionManager_.TakeRegion(expectPhysicalMem, false, true);
    if (region != nullptr) {
        DLOG(REGION, "alloc thread local to region @0x%zx+%zu type %u", region->GetRegionStart(),
             region->GetRegionAllocatedSize(),
             region->GetRegionType());
        tlToRegionList_.PrependRegion(region, RegionDesc::RegionType::TO_REGION);
    }
    return region;
}

} // namespace common
