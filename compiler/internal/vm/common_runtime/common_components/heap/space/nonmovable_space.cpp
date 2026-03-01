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

#include "common_components/common_runtime/hooks.h"
#include "common_components/heap/allocator/region_desc.h"
#include "common_components/heap/allocator/region_list.h"
#include "common_components/heap/allocator/regional_heap.h"
#include "common_components/heap/collector/collector.h"
#include "common_components/heap/collector/marking_collector.h"
#include "common_components/common/base_object.h"
#include "common_components/heap/allocator/fix_heap.h"
#include "common_components/heap/allocator/region_manager.h"
#include "common_components/heap/space/nonmovable_space.h"

#if defined(COMMON_TSAN_SUPPORT)
#include "common_components/sanitizer/sanitizer_interface.h"
#endif
#include "common_components/log/log.h"
#include "common_components/taskpool/taskpool.h"
#include "common_interfaces/base_runtime.h"

#if defined(_WIN64)
#include <sysinfoapi.h>
#endif

namespace common {

void NonMovableSpace::AssembleGarbageCandidates()
{
    polySizeRegionList_.MergeRegionListWithoutHead(recentPolySizeRegionList_,
                                                   RegionDesc::RegionType::FULL_POLYSIZE_NONMOVABLE_REGION);
    RegionDesc* region = polySizeRegionList_.GetHeadRegion();
    for (size_t i = 0; i < NONMOVABLE_OBJECT_SIZE_COUNT; i++) {
        monoSizeRegionList_[i]->MergeRegionListWithoutHead(*recentMonoSizeRegionList_[i],
            RegionDesc::RegionType::FULL_MONOSIZE_NONMOVABLE_REGION);
    }
}

void CollectFixHeapTaskForFullRegion(MarkingCollector &collector, RegionList &list,
                                     FixHeapTaskList &taskList)
{
    RegionDesc *region = list.GetHeadRegion();
    while (region != nullptr) {
        auto liveBytes = region->GetLiveByteCount();
        if (liveBytes == 0) {
            PostFixHeapWorker::AddEmptyRegionToCollectDuringPostFix(&list, region);
            region = region->GetNextRegion();
            continue;
        }
        taskList.push_back({region, FIX_REGION});
        region = region->GetNextRegion();
    }
}

void NonMovableSpace::CollectFixTasks(FixHeapTaskList &taskList)
{
    // fix all objects.
    if (Heap::GetHeap().GetGCReason() == GC_REASON_YOUNG) {
        FixHeapWorker::CollectFixHeapTasks(taskList, recentPolySizeRegionList_, FIX_RECENT_OLD_REGION);
        FixHeapWorker::CollectFixHeapTasks(taskList, polySizeRegionList_, FIX_OLD_REGION);

        for (size_t i = 0; i < NONMOVABLE_OBJECT_SIZE_COUNT; i++) {
            FixHeapWorker::CollectFixHeapTasks(taskList, *recentMonoSizeRegionList_[i], FIX_RECENT_OLD_REGION);
            FixHeapWorker::CollectFixHeapTasks(taskList, *monoSizeRegionList_[i], FIX_OLD_REGION);
        }
    } else {
        FixHeapWorker::CollectFixHeapTasks(taskList, recentPolySizeRegionList_, FIX_RECENT_REGION);
        MarkingCollector &collector = reinterpret_cast<MarkingCollector &>(Heap::GetHeap().GetCollector());
        CollectFixHeapTaskForFullRegion(collector, polySizeRegionList_, taskList);
        for (size_t i = 0; i < NONMOVABLE_OBJECT_SIZE_COUNT; i++) {
            FixHeapWorker::CollectFixHeapTasks(taskList, *recentMonoSizeRegionList_[i], FIX_RECENT_REGION);
            CollectFixHeapTaskForFullRegion(collector, *monoSizeRegionList_[i], taskList);
        }
    }
}

void NonMovableSpace::DumpRegionStats() const
{
    polySizeRegionList_.DumpRegionSummary();
    recentPolySizeRegionList_.DumpRegionSummary();
}

uintptr_t NonMovableSpace::AllocInMonoSizeList(size_t cellCount)
{
    GCPhase mutatorPhase = Mutator::GetMutator()->GetMutatorPhase();
    // workaround: make sure collector doesn't fix newly allocated incomplete objects
    if (mutatorPhase == GC_PHASE_MARK || mutatorPhase == GC_PHASE_FIX) {
        return 0;
    }

    RegionList* list = monoSizeRegionList_[cellCount];
    std::lock_guard<std::mutex> lock(list->GetListMutex());
    uintptr_t allocPtr = list->AllocFromFreeListInLock();
    // For making bitmap comform with live object count, do not mark object repeated.
    if (allocPtr == 0 || mutatorPhase == GCPhase::GC_PHASE_IDLE) {
        return allocPtr;
    }

    // Mark new allocated non-movable object.
    RegionDesc* regionDesc = RegionDesc::GetRegionDescAt(allocPtr);
    BaseObject* object = reinterpret_cast<BaseObject*>(allocPtr);
    regionDesc->MarkObject(object);
    size_t size = (cellCount + 1) * sizeof(uint64_t);
    regionDesc->AddLiveByteCount(size);
    return allocPtr;
}

uintptr_t NonMovableSpace::Alloc(size_t size, bool allowGC)
{
    uintptr_t addr = 0;
    if (!allowGC || size > NONMOVABLE_OBJECT_SIZE_THRESHOLD) {
        DLOG(ALLOC, "alloc non-movable obj 0x%zx(%zu)", addr, size);
        return AllocInPolySizeList(size);
    }
    CHECK_CC(size % sizeof(uint64_t) == 0);
    size_t cellCount = size / sizeof(uint64_t) - 1;
    RegionList* list = recentMonoSizeRegionList_[cellCount];
    std::mutex& listMutex = list->GetListMutex();
    std::lock_guard<std::mutex> lock(listMutex);
    RegionDesc* headRegion = list->GetHeadRegion();
    if (headRegion != nullptr) {
        addr = headRegion->Alloc(size);
    }
    if (addr == 0) {
        addr = AllocInMonoSizeList(cellCount);
    }
    if (addr == 0) {
        RegionDesc* region =
            regionManager_.TakeRegion(1, RegionDesc::UnitRole::SMALL_SIZED_UNITS, false, allowGC);
        if (region == nullptr) {
            return 0;
        }
        DLOG(REGION, "alloc non-movable region @0x%zx+%zu type %u", region->GetRegionStart(),
             region->GetRegionAllocatedSize(),
             region->GetRegionType());
        DCHECK_CC(cellCount == static_cast<size_t>(static_cast<uint8_t>(cellCount)));
        region->SetRegionCellCount(static_cast<uint8_t>(cellCount));
        InitRegionPhaseLine(region);
        // To make sure the allocedSize are consistent, it must prepend region first then alloc object.
        list->PrependRegionLocked(region, RegionDesc::RegionType::MONOSIZE_NONMOVABLE_REGION);
        addr = region->Alloc(size);
    }
    DLOG(ALLOC, "alloc non-movable obj 0x%zx(%zu)", addr, size);
    return addr;
}

uintptr_t NonMovableSpace::AllocInPolySizeList(size_t size, bool allowGC)
{
    uintptr_t addr = 0;
    std::mutex& regionListMutex = recentPolySizeRegionList_.GetListMutex();

    std::lock_guard<std::mutex> lock(regionListMutex);
    RegionDesc* headRegion = recentPolySizeRegionList_.GetHeadRegion();
    if (headRegion != nullptr) {
        addr = headRegion->Alloc(size);
    }
    if (addr == 0) {
        RegionDesc* region =
            regionManager_.TakeRegion(1, RegionDesc::UnitRole::SMALL_SIZED_UNITS, false, allowGC);
        if (region == nullptr) {
            return 0;
        }
        DLOG(REGION, "alloc non-movable region @0x%zx+%zu type %u", region->GetRegionStart(),
             region->GetRegionAllocatedSize(),
             region->GetRegionType());

        InitRegionPhaseLine(region);
        // To make sure the allocedSize are consistent, it must prepend region first then alloc object.
        recentPolySizeRegionList_.PrependRegionLocked(region,
            RegionDesc::RegionType::RECENT_POLYSIZE_NONMOVABLE_REGION);
        addr = region->Alloc(size);
    }

    DLOG(ALLOC, "alloc non-movable obj 0x%zx(%zu)", addr, size);
    return addr;
}

uintptr_t NonMovableSpace::AllocFullRegion()
{
    RegionDesc* region = regionManager_.TakeRegion(false, false);
    DCHECK_CC(region != nullptr);

    InitRegionPhaseLine(region);

    DLOG(REGION, "alloc non-movable region @0x%zx+%zu type %u", region->GetRegionStart(),
         region->GetRegionAllocatedSize(),
         region->GetRegionType());
 
    recentPolySizeRegionList_.PrependRegion(region, RegionDesc::RegionType::RECENT_POLYSIZE_NONMOVABLE_REGION);

    uintptr_t start = region->GetRegionStart();
    uintptr_t addr = region->Alloc(region->GetRegionEnd() - region->GetRegionAllocPtr());
    DCHECK_CC(addr != 0);

    return start;
}

void NonMovableSpace::MarkRememberSet(const std::function<void(BaseObject*)>& func)
{
    auto visitFunc = [&func](RegionDesc* region) {
        region->VisitRememberSetBeforeMarking(func);
    };
    recentPolySizeRegionList_.VisitAllRegions(visitFunc);
    polySizeRegionList_.VisitAllRegions(visitFunc);

    for (size_t i = 0; i < NONMOVABLE_OBJECT_SIZE_COUNT; i++) {
        recentMonoSizeRegionList_[i]->VisitAllRegions(visitFunc);
        monoSizeRegionList_[i]->VisitAllRegions(visitFunc);
    }
}

}