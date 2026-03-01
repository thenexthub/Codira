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

#ifndef COMMON_COMPONENTS_HEAP_ALLOCATOR_FREE_REGION_MANAGER_H
#define COMMON_COMPONENTS_HEAP_ALLOCATOR_FREE_REGION_MANAGER_H

#include "common_components/heap/allocator/treap.h"
#include "common_components/heap/allocator/region_desc.h"
#include "common_components/common/scoped_object_access.h"

namespace common {
class RegionManager;

// This class is and should be accessed only for region allocation. we do not rely on it to check region status.
class FreeRegionManager {
public:
    explicit FreeRegionManager(RegionManager& manager) : regionManager_(manager) {}

    virtual ~FreeRegionManager()
    {
        dirtyUnitTree_.Fini();
        releasedUnitTree_.Fini();
    }

    void Initialize(uint32_t regionCnt)
    {
        releasedUnitTree_.Init(regionCnt);
        dirtyUnitTree_.Init(regionCnt);
    }

    RegionDesc* TakeRegion(size_t num, RegionDesc::UnitRole uclass, bool expectPhysicalMem)
    {
        uint32_t idx = 0;
        bool tryDirtyTree = true;
        bool tryReleasedTree = true;

        // try as hard as we can to take free regions for allocation.
        while (tryDirtyTree || tryReleasedTree) {
            // first try to get a dirty region.
            if (tryDirtyTree && dirtyUnitTreeMutex_.try_lock()) {
                if (dirtyUnitTree_.TakeUnits(num, idx)) {
                    DLOG(REGION, "c-tree %p alloc dirty units[%u+%u, %u) @[0x%zx, 0x%zx), %u dirty-units left",
                        &dirtyUnitTree_, idx, num, idx + num, RegionDesc::GetUnitAddress(idx),
                        RegionDesc::GetUnitAddress(idx + num), dirtyUnitTree_.GetTotalCount());

                    // it makes sense to slow down allocation by clearing region memory.
                    RegionDesc::ClearUnits(idx, num);
                    RegionDesc* region = RegionDesc::InitRegion(idx, num, uclass);
                    dirtyUnitTreeMutex_.unlock();
                    return region;
                }
                tryDirtyTree = false; // once we fail to take units, stop trying.
                dirtyUnitTreeMutex_.unlock();
            }

            // then try to get a released region.
            if (tryReleasedTree && releasedUnitTreeMutex_.try_lock()) {
                if (releasedUnitTree_.TakeUnits(num, idx)) {
#ifdef _WIN64
                    MemoryMap::CommitMemory(
                        reinterpret_cast<void*>(RegionDesc::GetUnitAddress(idx)), num * RegionDesc::UNIT_SIZE);
#endif
                    DLOG(REGION, "c-tree %p alloc released units%u+%u @0x%zx+%zu, %u released-units left",
                        &releasedUnitTree_, idx, num, RegionDesc::GetUnitAddress(idx), num * RegionDesc::UNIT_SIZE,
                        releasedUnitTree_.GetTotalCount());
                    RegionDesc* region = RegionDesc::InitRegion(idx, num, uclass);
                    releasedUnitTreeMutex_.unlock();
                    PrehandleReleasedUnit(expectPhysicalMem, idx, num);
                    return region;
                }
                tryReleasedTree = false; // once we fail to take units, stop trying.
                releasedUnitTreeMutex_.unlock();
            }
        }

        return nullptr;
    }

    // add units [idx, idx + num)
    void AddGarbageUnits(uint32_t idx, uint32_t num)
    {
        std::lock_guard<std::mutex> lg(dirtyUnitTreeMutex_);
        if (UNLIKELY_CC(!dirtyUnitTree_.MergeInsert(idx, num, true))) {
            LOG_COMMON(FATAL) << "tid " << GetTid() << ": failed to add dirty units [" <<
                idx << "+" << num << ", " << (idx + num) << ")";
        }
    }

    void AddReleaseUnits(uint32_t idx, uint32_t num)
    {
        std::lock_guard<std::mutex> lg(releasedUnitTreeMutex_);
        if (UNLIKELY_CC(!releasedUnitTree_.MergeInsert(idx, num, true))) {
            LOG_COMMON(FATAL) << "tid %d: failed to add release units [" <<
                idx << "+" << num << ", " << (idx + num) << ")";
        }
    }

    uint32_t GetDirtyUnitCount() const { return dirtyUnitTree_.GetTotalCount(); }
    uint32_t GetReleasedUnitCount() const { return releasedUnitTree_.GetTotalCount(); }

#ifndef NDEBUG
    void DumpReleasedUnitTree() const { releasedUnitTree_.DumpTree("released-unit tree"); }
    void DumpDirtyUnitTree() const { dirtyUnitTree_.DumpTree("dirty-unit tree"); }
#endif

    size_t CalculateBytesToRelease() const;
    size_t ReleaseGarbageRegions(size_t targetCachedSize);

private:
    inline void PrehandleReleasedUnit(bool expectPhysicalMem, size_t idx, size_t num) const
    {
        if (expectPhysicalMem) {
            RegionDesc::ClearUnits(idx, num);
        }
    }
    RegionManager& regionManager_;

    // physical pages of released units are probably released and they are prepared for allocation.
    std::mutex releasedUnitTreeMutex_;
    Treap releasedUnitTree_;

    // dirty units are neither cleared nor released, thus must be zeroed explicitly for allocation.
    std::mutex dirtyUnitTreeMutex_;
    Treap dirtyUnitTree_;
};
} // namespace common
#endif // COMMON_COMPONENTS_HEAP_ALLOCATOR_FREE_REGION_MANAGER_H
