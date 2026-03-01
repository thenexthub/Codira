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

#ifndef COMMON_COMPONENTS_HEAP_ALLOCATOR_REGION_MANAGER_H
#define COMMON_COMPONENTS_HEAP_ALLOCATOR_REGION_MANAGER_H

#include <list>
#include <map>
#include <set>
#include <thread>
#include <vector>

#include "common_components/common/run_type.h"
#include "common_components/heap/allocator/alloc_buffer.h"
#include "common_components/heap/allocator/allocator.h"
#include "common_components/heap/allocator/free_region_manager.h"
#include "common_components/heap/allocator/region_list.h"
#include "common_components/heap/allocator/fix_heap.h"
#include "common_components/heap/allocator/slot_list.h"
#include "common_components/common_runtime/hooks.h"

namespace common {
class MarkingCollector;
class CompactCollector;
class RegionManager;
class Taskpool;
// RegionManager needs to know header size and alignment in order to iterate objects linearly
// and thus its Alloc should be rewrite with AllocObj(objSize)
class RegionManager {
public:
    /* region memory layout:
        1. some paddings memory to aligned
        2. region info for each region, part of heap metadata
        3. region space for allocation, i.e., the heap  --- start address is aligend to `RegionDesc::UNIT_SIZE`
    */
    static size_t GetHeapMemorySize(size_t heapSize)
    {
        size_t regionNum = GetHeapUnitCount(heapSize);
        size_t metadataSize = GetMetadataSize(regionNum);
        // Add one more `RegionDesc::UNIT_SIZE` totalSize, because we need the region address is aligned to
        // `RegionDesc::UNIT_SIZE`, this need some paddings
        size_t totalSize = metadataSize + RoundUp<size_t>(heapSize, RegionDesc::UNIT_SIZE) + RegionDesc::UNIT_SIZE;
        return totalSize;
    }

    static size_t GetHeapUnitCount(size_t heapSize)
    {
        heapSize = RoundUp<size_t>(heapSize, RegionDesc::UNIT_SIZE);
        size_t regionNum = heapSize / RegionDesc::UNIT_SIZE;
        return regionNum;
    }

    static size_t GetMetadataSize(size_t regionNum)
    {
        size_t metadataSize = regionNum * sizeof(RegionDesc);
        return RoundUp<size_t>(metadataSize, COMMON_PAGE_SIZE);
    }

    void Initialize(size_t regionNum, uintptr_t regionInfoStart);

    RegionManager()
        : freeRegionManager_(*this), garbageRegionList_("garbage regions") {}

    RegionManager(const RegionManager&) = delete;

    RegionManager& operator=(const RegionManager&) = delete;

#if defined(GCINFO_DEBUG) && GCINFO_DEBUG
    void DumpRegionDesc() const;
#endif

    void DumpRegionStats() const;

    uintptr_t GetInactiveZone() const { return inactiveZone_; }

    uintptr_t GetRegionHeapStart() const { return regionHeapStart_; }

    RegionDesc* GetFirstRegion() const
    {
        if (regionHeapStart_ < inactiveZone_) {
            return RegionDesc::GetRegionDescAt(regionHeapStart_);
        }
        return nullptr;
    }

    ~RegionManager() {}

    // take a region with *num* units for allocation
    RegionDesc* TakeRegion(size_t num, RegionDesc::UnitRole, bool expectPhysicalMem = false, bool allowgc = true,
        bool isCopy = false);

    RegionDesc* TakeRegion(bool expectPhysicalMem, bool allowgc, bool isCopy = false)
    {
        return TakeRegion(1, RegionDesc::UnitRole::SMALL_SIZED_UNITS, expectPhysicalMem, allowgc, isCopy);
    }

    void CountLiveObject(const BaseObject* obj);

    void CollectFromSpaceGarbage(RegionList& fromList)
    {
        garbageRegionList_.MergeRegionList(fromList, RegionDesc::RegionType::GARBAGE_REGION);
    }

    size_t CollectRegion(RegionDesc* region)
    {
        DLOG(REGION, "collect region %p@%#zx+%zu type %u", region, region->GetRegionStart(),
             region->GetLiveByteCount(), region->GetRegionType());

#ifdef USE_HWASAN
        ASAN_POISON_MEMORY_REGION(reinterpret_cast<const volatile void *>(region->GetRegionBase()),
            region->GetRegionBaseSize());
        const uintptr_t p_addr = region->GetRegionBase();
        const uintptr_t p_size = region->GetRegionBaseSize();
        LOG_COMMON(DEBUG) << std::hex << "set [" << p_addr <<
                             std::hex << ", " << p_addr + p_size << ") poisoned\n";
#endif
        garbageRegionList_.PrependRegion(region, RegionDesc::RegionType::GARBAGE_REGION);
        if (region->IsLargeRegion()) {
            return region->GetRegionSize();
        } else {
            return region->GetRegionSize() - region->GetLiveByteCount();
        }
    }

    void ReclaimRegion(RegionDesc* region);
    size_t ReleaseRegion(RegionDesc* region);

    void ReclaimGarbageRegions()
    {
        RegionDesc* garbage = garbageRegionList_.TakeHeadRegion();
        while (garbage != nullptr) {
            ReclaimRegion(garbage);
            garbage = garbageRegionList_.TakeHeadRegion();
        }
    }

    // targetSize: size of memory which we do not release and keep it as cache for future allocation.
    size_t ReleaseGarbageRegions(size_t targetSize) { return freeRegionManager_.ReleaseGarbageRegions(targetSize); }

    void ForEachObjectUnsafe(const std::function<void(BaseObject*)>& visitor) const;
    void ForEachObjectSafe(const std::function<void(BaseObject*)>& visitor) const;

    size_t GetDirtyUnitCount() const { return freeRegionManager_.GetDirtyUnitCount(); }

    size_t GetInactiveUnitCount() const { return (regionHeapEnd_ - inactiveZone_) / RegionDesc::UNIT_SIZE; }
    size_t GetActiveSize() const { return inactiveZone_ - regionHeapStart_; }

    RegionDesc* GetNextNeighborRegion(RegionDesc* region) const
    {
        HeapAddress address = region->GetRegionEnd();
        if (address < inactiveZone_.load()) {
            return RegionDesc::GetRegionDescAt(address);
        }
        return nullptr;
    }

    // this method checks whether allocation is permitted for now, otherwise, it is suspened
    // until allocation does no harm to gc.
    void RequestForRegion(size_t size);

private:
    inline void TagHugePage(RegionDesc* region, size_t num) const;
    inline void UntagHugePage(RegionDesc* region, size_t num) const;

    void ClearGCInfo(RegionList& list)
    {
        RegionList tmp("temp region list");
        list.CopyListTo(tmp);
        tmp.VisitAllRegions([](RegionDesc* region) {
            region->ClearMarkingCopyLine();
            region->ClearLiveInfo();
            region->ResetMarkBit();
        });
    }

    FreeRegionManager freeRegionManager_;

    // cache for fromRegionList after forwarding.
    RegionList garbageRegionList_;

    uintptr_t regionInfoStart_ = 0; // the address of first RegionDesc

    uintptr_t regionHeapStart_ = 0; // the address of first region to allocate object
    uintptr_t regionHeapEnd_ = 0;

    // the time when previous region was allocated, which is assigned with returned value by timeutil::NanoSeconds().
    std::atomic<uint64_t> prevRegionAllocTime_ = { 0 };

    // heap space not allocated yet for even once. this value should not be decreased.
    std::atomic<uintptr_t> inactiveZone_ = { 0 };

    friend class VerifyIterator;
};
} // namespace common

#endif // COMMON_COMPONENTS_HEAP_ALLOCATOR_REGION_MANAGER_H
