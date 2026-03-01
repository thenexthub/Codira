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


#include "Base/ImmortalWrapper.h"
#include "Heap/Allocator/RegionInfo.h"
#include "Heap/Allocator/RegionSpace.h"
#include "ForwardDataManager.h"
#include "LiveInfo.h"

namespace MapleRuntime {
uintptr_t RouteInfo::GetRoute(uint64_t preLiveBytes)
{
    if (preLiveBytes < toRegion1UsedBytes) {
        return toRegion1StartAddress + preLiveBytes;
    } else { // object is routed to to-region2
        CHECK(toRegion2Idx != INVALID_VALUE);
        RegionInfo* toRegion2 = reinterpret_cast<RegionInfo*>(RegionInfo::GetRegionInfo(toRegion2Idx));
        return toRegion2->GetRegionStart() + (preLiveBytes - toRegion1UsedBytes);
    }
}
} // namespace MapleRuntime
