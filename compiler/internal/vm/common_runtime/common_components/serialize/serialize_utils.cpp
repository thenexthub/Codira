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

#include "common_components/serialize/serialize_utils.h"

#include "common_components/heap/allocator/region_desc.h"
#include "common_interfaces/base_runtime.h"

namespace common {
SerializedBaseObjectSpace SerializeUtils::GetSerializeObjectSpace(uintptr_t obj)
{
    RegionDesc::RegionType type = RegionDesc::GetAliveRegionType(obj);
    switch (type) {
        case RegionDesc::RegionType::THREAD_LOCAL_REGION:
        case RegionDesc::RegionType::THREAD_LOCAL_OLD_REGION:
        case RegionDesc::RegionType::RECENT_FULL_REGION:
        case RegionDesc::RegionType::FROM_REGION:
        case RegionDesc::RegionType::LONE_FROM_REGION:
        case RegionDesc::RegionType::EXEMPTED_FROM_REGION:
        case RegionDesc::RegionType::TO_REGION:
        case RegionDesc::RegionType::OLD_REGION:
            return SerializedBaseObjectSpace::REGULAR_SPACE;
        case RegionDesc::RegionType::FULL_POLYSIZE_NONMOVABLE_REGION:
        case RegionDesc::RegionType::RECENT_POLYSIZE_NONMOVABLE_REGION:
        case RegionDesc::RegionType::MONOSIZE_NONMOVABLE_REGION:
        case RegionDesc::RegionType::FULL_MONOSIZE_NONMOVABLE_REGION:
        case RegionDesc::RegionType::READ_ONLY_REGION:
        case RegionDesc::RegionType::APPSPAWN_REGION:
            return SerializedBaseObjectSpace::PIN_SPACE;
        case RegionDesc::RegionType::RECENT_LARGE_REGION:
        case RegionDesc::RegionType::LARGE_REGION:
            return SerializedBaseObjectSpace::LARGE_SPACE;
        default:
            return SerializedBaseObjectSpace::OTHER;
    }
}
}  // namespace common
