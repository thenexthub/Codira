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

#include "pool_map.h"

namespace ark {

void PoolMap::AddPoolToMap(const void *poolAddr, size_t poolSize, SpaceType spaceType, AllocatorType allocatorType,
                           const void *allocatorAddr)
{
    ASSERT((ToUintPtr(poolAddr) & POOL_MAP_GRANULARITY_MASK) == 0);
    ASSERT((poolSize & POOL_MAP_GRANULARITY_MASK) == 0);
    ASSERT(allocatorAddr != nullptr);
    MapNumType firstMapNum = AddrToMapNum(poolAddr);
    MapNumType lastMapNum = AddrToMapNum(ToVoidPtr(ToUintPtr(poolAddr) + poolSize - 1U));
    poolMap_[firstMapNum].Initialize(firstMapNum, spaceType, allocatorType, allocatorAddr);
    for (MapNumType i = firstMapNum + 1U; i <= lastMapNum; i++) {
        poolMap_[i].Initialize(firstMapNum, spaceType, allocatorType, allocatorAddr);
    }
}

void PoolMap::RemovePoolFromMap(const void *poolAddr, size_t poolSize)
{
    ASSERT((ToUintPtr(poolAddr) & POOL_MAP_GRANULARITY_MASK) == 0);
    ASSERT((poolSize & POOL_MAP_GRANULARITY_MASK) == 0);
    MapNumType firstMapNum = AddrToMapNum(poolAddr);
    MapNumType lastMapNum = AddrToMapNum(ToVoidPtr(ToUintPtr(poolAddr) + poolSize - 1U));
    for (MapNumType i = firstMapNum; i <= lastMapNum; i++) {
        poolMap_[i].Destroy();
    }
}

AllocatorInfo PoolMap::GetAllocatorInfo(const void *addr) const
{
    MapNumType mapNum = AddrToMapNum(addr);
    AllocatorType allocatorType = poolMap_[mapNum].GetAllocatorType();
    const void *allocatorAddr = poolMap_[mapNum].GetAllocatorAddr();
    // We can't get allocator info for not properly initialized pools
    ASSERT(allocatorType != AllocatorType::UNDEFINED);
    ASSERT(allocatorAddr != nullptr);
    return AllocatorInfo(allocatorType, allocatorAddr);
}

SpaceType PoolMap::GetSpaceType(const void *addr) const
{
    if (ToUintPtr(addr) > (POOL_MAP_COVERAGE - 1U)) {
        return SpaceType::SPACE_TYPE_UNDEFINED;
    }
    MapNumType mapNum = AddrToMapNum(addr);
    SpaceType spaceType = poolMap_[mapNum].GetSpaceType();
    // We can't get space type for not properly initialized pools
    ASSERT(spaceType != SpaceType::SPACE_TYPE_UNDEFINED);
    return spaceType;
}

void *PoolMap::GetFirstByteOfPoolForAddr(const void *addr) const
{
    MapNumType segmentFirstMapNum = poolMap_[AddrToMapNum(addr)].GetSegmentFirstMapNum();
    return MapNumToAddr(segmentFirstMapNum);
}

bool PoolMap::IsEmpty() const
{
    for (auto i : poolMap_) {
        if (!i.IsEmpty()) {
            return false;
        }
    }
    return true;
}

}  // namespace ark
