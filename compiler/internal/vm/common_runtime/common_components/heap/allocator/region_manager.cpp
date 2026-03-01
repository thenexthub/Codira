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

#include "common_components/heap/allocator/region_manager.h"

#include <algorithm>
#include <cmath>
#include <cstdint>
#include <unistd.h>

#include "common_components/common_runtime/hooks.h"
#include "common_components/heap/allocator/region_desc.h"
#include "common_components/heap/allocator/region_list.h"
#include "common_components/heap/allocator/regional_heap.h"
#include "common_components/base/c_string.h"
#include "common_components/heap/collector/collector.h"
#include "common_components/heap/collector/marking_collector.h"
#include "common_components/common/base_object.h"
#include "common_components/common/scoped_object_access.h"
#include "common_components/heap/heap.h"
#include "common_components/mutator/mutator.inline.h"
#include "common_components/mutator/mutator_manager.h"
#include "common_components/heap/allocator/fix_heap.h"

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
uintptr_t RegionDesc::UnitInfo::totalUnitCount = 0;
uintptr_t RegionDesc::UnitInfo::unitInfoStart = 0;
uintptr_t RegionDesc::UnitInfo::heapStartAddress = 0;

static size_t GetPageSize() noexcept
{
#if defined(_WIN64)
    SYSTEM_INFO systeminfo;
    GetSystemInfo(&systeminfo);
    if (systeminfo.dwPageSize != 0) {
        return systeminfo.dwPageSize;
    } else {
        // default page size is 4KB if get system page size failed.
        return 4 * KB;
    }
#elif defined(__APPLE__)
    // default page size is 4KB in MacOS
    return 4 * KB;
#else
    return getpagesize();
#endif
}

// System default page size
const size_t COMMON_PAGE_SIZE = GetPageSize();
const size_t AllocatorUtils::ALLOC_PAGE_SIZE = COMMON_PAGE_SIZE;
// size of huge page is 2048KB.
constexpr size_t HUGE_PAGE_UNIT_NUM = (2048 * KB) / RegionDesc::UNIT_SIZE;

#if defined(GCINFO_DEBUG) && GCINFO_DEBUG
void RegionDesc::DumpRegionDesc(LogType type) const
{
    DLOG(type, "Region index: %zu, type: %s, address: 0x%zx(start=0x%zx)-0x%zx, allocated(B) %zu, live(B) %zu",
         GetUnitIdx(), GetTypeName(), GetRegionBase(), GetRegionStart(), GetRegionEnd(), GetRegionAllocatedSize(),
         GetLiveByteCount());
}

const char* RegionDesc::GetTypeName() const
{
    static constexpr const char* regionNames[] = {
        "undefined region",
        "thread local region",
        "recent fullregion",
        "from region",
        "unmovable from region",
        "to region",
        "full non-movable region",
        "recent non-movable region",
        "raw pointer non-movable region",
        "tl raw pointer region",
        "large region",
        "recent large region",
        "garbage region",
    };
    return regionNames[static_cast<uint8_t>(GetRegionType())];
}
#endif

void RegionDesc::VisitAllObjects(const std::function<void(BaseObject*)>&& func)
{
    uintptr_t allocPtr = GetRegionAllocPtr();
    VisitAllObjectsBefore(std::move(func), GetRegionAllocPtr());
}

void RegionDesc::VisitAllObjectsBefore(const std::function<void(BaseObject *)> &&func, uintptr_t end)
{
    uintptr_t position = GetRegionStart();

    if (IsMonoSizeNonMovableRegion()) {
        size_t size = static_cast<size_t>(GetRegionCellCount() + 1) * sizeof(uint64_t);
        while (position < end) {
            BaseObject *obj = reinterpret_cast<BaseObject *>(position);
            position += size;
            if (position > end) {
                break;
            }
            if (IsFreeNonMovableObject(obj)) {
                continue;
            }
            func(obj);
        }
        return;
    } else if (IsLargeRegion() && (position < end)) {
        if (IsJitFortAwaitInstallFlag()) {
            return;
        }
        func(reinterpret_cast<BaseObject *>(GetRegionStart()));
    } else if (IsSmallRegion()) {
        while (position < end) {
            // GetAllocSize should before call func, because object maybe destroy in compact gc.
            func(reinterpret_cast<BaseObject *>(position));
            size_t size = RegionalHeap::GetAllocSize(*reinterpret_cast<BaseObject *>(position));
            position += size;
        }
    }
}

void RegionDesc::VisitAllObjectsBeforeCopy(const std::function<void(BaseObject*)>&& func)
{
    uintptr_t allocPtr = GetRegionAllocPtr();
    uintptr_t phaseLine = GetCopyLine();
    uintptr_t end = std::min(phaseLine, allocPtr);
    VisitAllObjectsBefore(std::move(func), end);
}

bool RegionDesc::VisitLiveObjectsUntilFalse(const std::function<bool(BaseObject*)>&& func)
{
    // no need to visit this region.
    if (GetLiveByteCount() == 0) {
        return true;
    }

    MarkingCollector& collector = reinterpret_cast<MarkingCollector&>(Heap::GetHeap().GetCollector());
    if (IsLargeRegion()) {
        if (IsJitFortAwaitInstallFlag()) {
            return true;
        }
        return func(reinterpret_cast<BaseObject *>(GetRegionStart()));
    }
    if (IsSmallRegion()) {
        uintptr_t position = GetRegionStart();
        uintptr_t allocPtr = GetRegionAllocPtr();
        while (position < allocPtr) {
            BaseObject* obj = reinterpret_cast<BaseObject*>(position);
            if (collector.IsSurvivedObject(obj) && !func(obj)) { return false; }
            position += RegionalHeap::GetAllocSize(*obj);
        }
    }
    return true;
}

void RegionDesc::VisitRememberSetBeforeMarking(const std::function<void(BaseObject*)>& func)
{
    if (IsLargeRegion() && IsJitFortAwaitInstallFlag()) {
        // machine code which is not installed should skip here.
        return;
    }
    uintptr_t end = std::min(GetMarkingLine(), GetRegionAllocPtr());
    GetRSet()->VisitAllMarkedCardBefore(func, GetRegionBaseFast(), end);
}

void RegionDesc::VisitRememberSetBeforeCopy(const std::function<void(BaseObject*)>& func)
{
    if (IsLargeRegion() && IsJitFortAwaitInstallFlag()) {
        // machine code which is not installed should skip here.
        return;
    }
    uintptr_t end = std::min(GetCopyLine(), GetRegionAllocPtr());
    GetRSet()->VisitAllMarkedCardBefore(func, GetRegionBaseFast(), end);
}

void RegionDesc::VisitRememberSet(const std::function<void(BaseObject*)>& func)
{
    if (IsLargeRegion() && IsJitFortAwaitInstallFlag()) {
        // machine code which is not installed should skip here.
        return;
    }
    GetRSet()->VisitAllMarkedCardBefore(func, GetRegionBaseFast(), GetRegionAllocPtr());
}

void RegionList::MergeRegionListWithoutHead(RegionList& srcList, RegionDesc::RegionType regionType)
{
    RegionDesc *head = srcList.TakeHeadRegion();
    MergeRegionList(srcList, regionType);
    if (head) {
        srcList.PrependRegion(head, head->GetRegionType());
    }
}

void RegionList::MergeRegionList(RegionList& srcList, RegionDesc::RegionType regionType)
{
    RegionList regionList("region list cache");
    srcList.MoveTo(regionList);
    RegionDesc* head = regionList.GetHeadRegion();
    RegionDesc* tail = regionList.GetTailRegion();
    if (head == nullptr) {
        return;
    }
    std::lock_guard<std::mutex> lock(listMutex_);
    regionList.SetElementType(regionType);
    IncCounts(regionList.GetRegionCount(), regionList.GetUnitCount());
    if (listHead_ == nullptr) {
        listHead_ = head;
        listTail_ = tail;
    } else {
        tail->SetNextRegion(listHead_);
        listHead_->SetPrevRegion(tail);
        listHead_ = head;
    }
}

static const char *RegionDescRegionTypeToString(RegionDesc::RegionType type)
{
    static constexpr const char *enumStr[] = {
        [static_cast<uint8_t>(RegionDesc::RegionType::FREE_REGION)] = "FREE_REGION",
        [static_cast<uint8_t>(RegionDesc::RegionType::GARBAGE_REGION)] = "GARBAGE_REGION",
        [static_cast<uint8_t>(RegionDesc::RegionType::THREAD_LOCAL_REGION)] = "THREAD_LOCAL_REGION",
        [static_cast<uint8_t>(RegionDesc::RegionType::THREAD_LOCAL_OLD_REGION)] = "THREAD_LOCAL_OLD_REGION",
        [static_cast<uint8_t>(RegionDesc::RegionType::RECENT_FULL_REGION)] = "RECENT_FULL_REGION",
        [static_cast<uint8_t>(RegionDesc::RegionType::FROM_REGION)] = "FROM_REGION",
        [static_cast<uint8_t>(RegionDesc::RegionType::LONE_FROM_REGION)] = "LONE_FROM_REGION",
        [static_cast<uint8_t>(RegionDesc::RegionType::EXEMPTED_FROM_REGION)] = "EXEMPTED_FROM_REGION",
        [static_cast<uint8_t>(RegionDesc::RegionType::TO_REGION)] = "TO_REGION",
        [static_cast<uint8_t>(RegionDesc::RegionType::OLD_REGION)] = "OLD_REGION",
        [static_cast<uint8_t>(RegionDesc::RegionType::FULL_POLYSIZE_NONMOVABLE_REGION)] =
                                                            "FULL_POLYSIZE_NONMOVABLE_REGION",
        [static_cast<uint8_t>(RegionDesc::RegionType::RECENT_POLYSIZE_NONMOVABLE_REGION)] =
                                                            "RECENT_POLYSIZE_NONMOVABLE_REGION",
        [static_cast<uint8_t>(RegionDesc::RegionType::MONOSIZE_NONMOVABLE_REGION)] = "MONOSIZE_NONMOVABLE_REGION",
        [static_cast<uint8_t>(RegionDesc::RegionType::FULL_MONOSIZE_NONMOVABLE_REGION)] =
                                                            "FULL_MONOSIZE_NONMOVABLE_REGION",
        [static_cast<uint8_t>(RegionDesc::RegionType::RAW_POINTER_REGION)] = "RAW_POINTER_REGION",
        [static_cast<uint8_t>(RegionDesc::RegionType::TL_RAW_POINTER_REGION)] = "TL_RAW_POINTER_REGION",
        [static_cast<uint8_t>(RegionDesc::RegionType::RECENT_LARGE_REGION)] = "RECENT_LARGE_REGION",
        [static_cast<uint8_t>(RegionDesc::RegionType::LARGE_REGION)] = "LARGE_REGION",
        [static_cast<uint8_t>(RegionDesc::RegionType::READ_ONLY_REGION)] = "READ_ONLY_REGION",
        [static_cast<uint8_t>(RegionDesc::RegionType::APPSPAWN_REGION)] = "APPSPAWN_REGION",
    };
    ASSERT_LOGF(type < RegionDesc::RegionType::END_OF_REGION_TYPE, "Invalid region type");
    return enumStr[static_cast<uint8_t>(type)];
}

void RegionList::PrependRegion(RegionDesc* region, RegionDesc::RegionType type)
{
    std::lock_guard<std::mutex> lock(listMutex_);
    PrependRegionLocked(region, type);
}

void RegionList::PrependRegionLocked(RegionDesc* region, RegionDesc::RegionType type)
{
    if (region == nullptr) {
        return;
    }

    DLOG(REGION, "%s (%zu, %zu)+(%zu, %zu) prepend region %p(base=%#zx)@%#zx+%zu type %u->%u", listName_,
        regionCount_, unitCount_, 1llu, region->GetUnitCount(), region, region->GetRegionBase(),
        region->GetRegionStart(), region->GetRegionAllocatedSize(), region->GetRegionType(), type);

    region->SetRegionType(type);

    size_t totalRegionSize = region->GetRegionEnd() - region->GetRegionBase();
    os::PrctlSetVMA(reinterpret_cast<void *>(region->GetRegionBase()), totalRegionSize,
                    (std::string("ArkTS Heap CMCGC Region ") + RegionDescRegionTypeToString(type)).c_str());

    region->SetPrevRegion(nullptr);
    IncCounts(1, region->GetUnitCount());
    region->SetNextRegion(listHead_);
    if (listHead_ == nullptr) {
        ASSERT_LOGF(listTail_ == nullptr, "PrependRegion listTail is not null");
        listTail_ = region;
    } else {
        listHead_->SetPrevRegion(region);
    }
    listHead_ = region;
}

void RegionList::DeleteRegionLocked(RegionDesc* del)
{
    ASSERT_LOGF(listHead_ != nullptr && listTail_ != nullptr, "illegal region list");

    RegionDesc* pre = del->GetPrevRegion();
    RegionDesc* next = del->GetNextRegion();

    del->SetNextRegion(nullptr);
    del->SetPrevRegion(nullptr);

    DLOG(REGION, "%s (%zu, %zu)-(%zu, %zu) delete region %p(start=%p),@%#zx+%zu type %u", listName_,
        regionCount_, unitCount_, 1llu, del->GetUnitCount(),
        del, del->GetRegionBase(), del->GetRegionStart(), del->GetRegionAllocatedSize(), del->GetRegionType());

    DecCounts(1, del->GetUnitCount());

    if (listHead_ == del) { // delete head
        ASSERT_LOGF(pre == nullptr, "Delete Region pre is not null");
        listHead_ = next;
        if (listHead_ == nullptr) { // now empty
            listTail_ = nullptr;
            return;
        }
    } else {
        pre->SetNextRegion(next);
    }

    if (listTail_ == del) { // delete tail
        ASSERT_LOGF(next == nullptr, "Delete Region next is not null");
        listTail_ = pre;
        if (listTail_ == nullptr) { // now empty
            listHead_ = nullptr;
            return;
        }
    } else {
        next->SetPrevRegion(pre);
    }
}

void RegionList::DumpRegionSummary() const
{
    VLOG(DEBUG, "\t%s %zu: %zu units (%zu B, alloc %zu)", listName_,
         regionCount_, unitCount_, GetAllocatedSize(true), GetAllocatedSize(false));
}

#ifndef NDEBUG
void RegionList::DumpRegionList(const char* msg)
{
    DLOG(REGION, "dump region list %s", msg);
    std::lock_guard<std::mutex> lock(listMutex_);
    for (RegionDesc *region = listHead_; region != nullptr; region = region->GetNextRegion()) {
        DLOG(REGION, "region %p @0x%zx(start=0x%zx)+%zu units [%zu+%zu, %zu) type %u prev %p next %p", region,
            region->GetRegionBase(), region->GetRegionStart(), region->GetRegionAllocatedSize(),
            region->GetUnitIdx(), region->GetUnitCount(), region->GetUnitIdx() + region->GetUnitCount(),
            region->GetRegionType(), region->GetPrevRegion(), region->GetNextRegion());
    }
}
#endif
inline void RegionManager::TagHugePage(RegionDesc* region, size_t num) const
{
#if defined (__linux__) || defined(PANDA_TARGET_OHOS)
    (void)madvise(reinterpret_cast<void*>(region->GetRegionBase()), num * RegionDesc::UNIT_SIZE, MADV_HUGEPAGE);
#else
    (void)region;
    (void)num;
#endif
}

inline void RegionManager::UntagHugePage(RegionDesc* region, size_t num) const
{
#if defined (__linux__) || defined(PANDA_TARGET_OHOS)
    (void)madvise(reinterpret_cast<void*>(region->GetRegionBase()), num * RegionDesc::UNIT_SIZE, MADV_NOHUGEPAGE);
#else
    (void)region;
    (void)num;
#endif
}

size_t FreeRegionManager::ReleaseGarbageRegions(size_t targetCachedSize)
{
    size_t dirtyBytes = dirtyUnitTree_.GetTotalCount() * RegionDesc::UNIT_SIZE;
    if (dirtyBytes <= targetCachedSize) {
        VLOG(DEBUG, "release heap garbage memory 0 bytes, cache %zu(%zu) bytes", dirtyBytes, targetCachedSize);
        return 0;
    }

    size_t releasedBytes = 0;
    while (dirtyBytes > targetCachedSize) {
        std::lock_guard<std::mutex> lock1(dirtyUnitTreeMutex_);
        auto node = dirtyUnitTree_.RootNode();
        if (node == nullptr) { break; }
        uint32_t idx = node->GetIndex();
        uint32_t num = node->GetCount();
        dirtyUnitTree_.ReleaseRootNode();

        std::lock_guard<std::mutex> lock2(releasedUnitTreeMutex_);
        LOGF_CHECK(releasedUnitTree_.MergeInsert(idx, num, true)) <<
            "tid " << GetTid() << ": failed to release garbage units[" <<
            idx << "+" << num << ", " << (idx + num) << ")";
        releasedBytes += (num * RegionDesc::UNIT_SIZE);
        dirtyBytes = dirtyUnitTree_.GetTotalCount() * RegionDesc::UNIT_SIZE;
    }
    VLOG(DEBUG, "release heap garbage memory %zu bytes, cache %zu(%zu) bytes",
         releasedBytes, dirtyBytes, targetCachedSize);
    return releasedBytes;
}

void RegionManager::Initialize(size_t nRegion, uintptr_t regionInfoAddr)
{
    size_t metadataSize = GetMetadataSize(nRegion);
    size_t alignedHeapStart = RoundUp<size_t>(regionInfoAddr + metadataSize, RegionDesc::UNIT_SIZE);
    // align the start of region to 256KB
    /***
     * |***********|<-metadataSize->|**********************|
     * |**padding**|***RegionUnit***|*******Region*********|
     *  ^           ^                ^
     *  |           |                |
     *  |    reginInfoStart    alignedHeapStart
     * regionInfoAddr
    */
    regionInfoStart_ = alignedHeapStart - metadataSize;
    regionHeapStart_ = alignedHeapStart;
#ifdef _WIN64
    MemoryMap::CommitMemory(reinterpret_cast<void*>(regionInfoStart_), metadataSize);
#endif
    regionHeapEnd_ = regionHeapStart_ + nRegion * RegionDesc::UNIT_SIZE;
    inactiveZone_ = regionHeapStart_;
    // propagate region heap layout
    RegionDesc::Initialize(nRegion, regionInfoStart_, regionHeapStart_);
    freeRegionManager_.Initialize(nRegion);

    DLOG(REPORT, "region info @0x%zx+%zu, heap [0x%zx, 0x%zx), unit count %zu", regionInfoAddr, metadataSize,
         regionHeapStart_, regionHeapEnd_, nRegion);
#ifdef USE_HWASAN
    ASAN_UNPOISON_MEMORY_REGION(reinterpret_cast<const volatile void *>(regionInfoAddr),
        metadataSize + nRegion * RegionDesc::UNIT_SIZE);
    const uintptr_t p_addr = regionInfoAddr;
    const uintptr_t p_size = metadataSize + nRegion * RegionDesc::UNIT_SIZE;
    LOG_COMMON(DEBUG) << std::hex << "set [" << p_addr <<
                         std::hex << ", " << p_addr + p_size << ") unpoisoned\n";
#endif
}

void RegionManager::ReclaimRegion(RegionDesc* region)
{
    size_t num = region->GetUnitCount();
    size_t unitIndex = region->GetUnitIdx();
    if (num >= HUGE_PAGE_UNIT_NUM) {
        UntagHugePage(region, num);
    }
    DLOG(REGION, "reclaim region %p(base=%#zx)@%#zx+%zu type %u", region,
         region->GetRegionBase(), region->GetRegionStart(), region->GetRegionAllocatedSize(),
         region->GetRegionType());

    region->InitFreeUnits();
    freeRegionManager_.AddGarbageUnits(unitIndex, num);
}

size_t RegionManager::ReleaseRegion(RegionDesc* region)
{
    size_t res = region->GetRegionSize();
    size_t num = region->GetUnitCount();
    size_t unitIndex = region->GetUnitIdx();
    if (num >= HUGE_PAGE_UNIT_NUM) {
        UntagHugePage(region, num);
    }
    DLOG(REGION, "release region %p @%#zx+%zu type %u", region, region->GetRegionStart(),
        region->GetRegionAllocatedSize(), region->GetRegionType());

    region->InitFreeUnits();
    RegionDesc::ReleaseUnits(unitIndex, num);
    freeRegionManager_.AddReleaseUnits(unitIndex, num);
    return res;
}

void RegionManager::CountLiveObject(const BaseObject* obj)
{
    RegionDesc* region = RegionDesc::GetRegionDescAt(reinterpret_cast<HeapAddress>(obj));
    region->AddLiveByteCount(obj->GetSize());
}

void RegionManager::ForEachObjectUnsafe(const std::function<void(BaseObject*)>& visitor) const
{
    for (uintptr_t regionAddr = regionHeapStart_; regionAddr < inactiveZone_;) {
        RegionDesc* region = RegionDesc::GetRegionDescAt(regionAddr);
        uintptr_t next = region->GetRegionEnd();
        regionAddr = next;
        if (!region->IsValidRegion() || region->IsFreeRegion() || region -> IsGarbageRegion()) {
            continue;
        }
        region->VisitAllObjects([&visitor](BaseObject* object) { visitor(object); });
    }
}

void RegionManager::ForEachObjectSafe(const std::function<void(BaseObject*)>& visitor) const
{
    ScopedEnterSaferegion enterSaferegion(false);
    STWParam stwParam{"visit-all-objects"};
    ScopedStopTheWorld stw(stwParam);
    ForEachObjectUnsafe(visitor);
}

RegionDesc *RegionManager::TakeRegion(size_t num, RegionDesc::UnitRole type, bool expectPhysicalMem, bool allowGC,
    bool isCopy)
{
    // a chance to invoke heuristic gc.
    if (allowGC && !Heap::GetHeap().IsGcStarted()) {
        Heap::GetHeap().TryHeuristicGC();
    }

    // check for allocation since we do not want gc threads and mutators do any harm to each other.
    size_t size = num * RegionDesc::UNIT_SIZE;
    RequestForRegion(size);

    RegionDesc* head = garbageRegionList_.TakeHeadRegion();
    if (head != nullptr) {
        DLOG(REGION, "take garbage region %p@%#zx+%zu", head, head->GetRegionStart(), head->GetRegionSize());
        if (head->GetUnitCount() == num) {
            auto idx = head->GetUnitIdx();
#ifdef USE_HWASAN
            const uintptr_t pAddr = RegionDesc::GetUnitAddress(idx);
            ASAN_UNPOISON_MEMORY_REGION(reinterpret_cast<const volatile void *>(pAddr), size);
            LOG_COMMON(DEBUG) << std::hex << "set [" << pAddr <<
                                std::hex << ", " << pAddr + size << ") unpoisoned\n";
#endif
            RegionDesc::ClearUnits(idx, num);
            DLOG(REGION, "reuse garbage region %p@%#zx+%zu", head, head->GetRegionStart(), head->GetRegionSize());
            return RegionDesc::ResetRegion(idx, num, type);
        } else {
            DLOG(REGION, "reclaim garbage region %p@%#zx+%zu", head, head->GetRegionStart(), head->GetRegionSize());
            ReclaimRegion(head);
        }
    }

    RegionDesc* region = freeRegionManager_.TakeRegion(num, type, expectPhysicalMem);
    if (region != nullptr) {
        if (num >= HUGE_PAGE_UNIT_NUM) {
            TagHugePage(region, num);
        }
        return region;
    }

    // when free regions are not enough for allocation
    if (num <= GetInactiveUnitCount()) {
        uintptr_t addr = inactiveZone_.fetch_add(size);

#ifndef PANDA_TARGET_32
        size_t totalHeapSize = regionHeapEnd_ - regionHeapStart_;
        // 2: half space reserved for forward copy. throw oom when gc finish.
        if (GetActiveSize() * 2 > totalHeapSize) {
            if (!isCopy) {
                (void)inactiveZone_.fetch_sub(size);
                return nullptr;
            } else {
                Heap::GetHeap().SetForceThrowOOM(true);
            }
        }
#endif

        if (addr < regionHeapEnd_ - size) {
            region = RegionDesc::InitRegionAt(addr, num, type);
            size_t idx = region->GetUnitIdx();
#ifdef _WIN64
            MemoryMap::CommitMemory(
                reinterpret_cast<void*>(RegionDesc::GetUnitAddress(idx)), num * RegionDesc::UNIT_SIZE);
#endif
            (void)idx; // eliminate compilation warning
            DLOG(REGION, "take inactive units [%zu+%zu, %zu) at [0x%zx, 0x%zx)", idx, num, idx + num,
                 RegionDesc::GetUnitAddress(idx), RegionDesc::GetUnitAddress(idx + num));
            if (num >= HUGE_PAGE_UNIT_NUM) {
                TagHugePage(region, num);
            }
            if (expectPhysicalMem) {
                RegionDesc::ClearUnits(idx, num);
            }
            return region;
        } else {
            (void)inactiveZone_.fetch_sub(size);
        }
    }

    return nullptr;
}

#if defined(GCINFO_DEBUG) && GCINFO_DEBUG
void RegionManager::DumpRegionDesc() const
{
    if (!ENABLE_LOG(ALLOC)) {
        return;
    }
    for (uintptr_t regionAddr = regionHeapStart; regionAddr < inactiveZone;) {
        RegionDesc* region = RegionDesc::GetRegionDescAt(regionAddr);
        regionAddr = region->GetRegionEnd();
        if (!region->IsFreeRegion()) {
            region->DumpRegionDesc(ALLOC);
        }
    }
}
#endif

void RegionManager::DumpRegionStats() const
{
    size_t totalSize = regionHeapEnd_ - regionHeapStart_;
    size_t totalUnits = totalSize / RegionDesc::UNIT_SIZE;
    size_t activeSize = inactiveZone_ - regionHeapStart_;
    size_t activeUnits = activeSize / RegionDesc::UNIT_SIZE;
    VLOG(DEBUG, "\ttotal units: %zu (%zu B)", totalUnits, totalSize);
    VLOG(DEBUG, "\tactive units: %zu (%zu B)", activeUnits, activeSize);

    garbageRegionList_.DumpRegionSummary();

    size_t releasedUnits = freeRegionManager_.GetReleasedUnitCount();
    size_t dirtyUnits = freeRegionManager_.GetDirtyUnitCount();
    VLOG(DEBUG, "\treleased units: %zu (%zu B)", releasedUnits, releasedUnits * RegionDesc::UNIT_SIZE);
    VLOG(DEBUG, "\tdirty units: %zu (%zu B)", dirtyUnits, dirtyUnits * RegionDesc::UNIT_SIZE);
}

void RegionManager::RequestForRegion(size_t size)
{
    if (IsGcThread()) {
        // gc thread is always permitted for allocation.
        return;
    }

    Heap& heap = Heap::GetHeap();
    GCStats& gcstats = heap.GetCollector().GetGCStats();
    size_t allocatedBytes = heap.GetAllocatedSize() - gcstats.liveBytesAfterGC;
    constexpr double pi = 3.14;
    size_t availableBytesAfterGC = heap.GetMaxCapacity() - gcstats.liveBytesAfterGC;
    double heuAllocRate = std::cos((pi / 2.0) * allocatedBytes / availableBytesAfterGC) * gcstats.collectionRate;
    // for maximum performance, choose the larger one.
    double allocRate = std::max(
        static_cast<double>(BaseRuntime::GetInstance()->GetHeapParam().allocationRate) * MB / SECOND_TO_NANO_SECOND,
        heuAllocRate);
    ASSERT_LOGF(allocRate > 0.00001, "allocRate is zero"); // If it is less than 0.00001, it is considered as 0
    size_t waitTime = static_cast<size_t>(size / allocRate);
    uint64_t now = TimeUtil::NanoSeconds();
    if (prevRegionAllocTime_ + waitTime <= now) {
        prevRegionAllocTime_ = TimeUtil::NanoSeconds();
        return;
    }

    uint64_t sleepTime = std::min<uint64_t>(BaseRuntime::GetInstance()->GetHeapParam().allocationWaitTime,
                                  prevRegionAllocTime_ + waitTime - now);
    DLOG(ALLOC, "wait %zu ns to alloc %zu(B)", sleepTime, size);
    std::this_thread::sleep_for(std::chrono::nanoseconds{ sleepTime });
    prevRegionAllocTime_ = TimeUtil::NanoSeconds();
}
}