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

#include "common_components/heap/allocator/regional_heap.h"

#include "common_components/heap/collector/collector.h"
#include "common_components/heap/collector/collector_resources.h"
#include "common_components/platform/os.h"
#if defined(COMMON_SANITIZER_SUPPORT)
#include "common_components/sanitizer/sanitizer_interface.h"
#endif
#include "common_components/common/scoped_object_access.h"
#include "common_components/heap/heap.h"

namespace common {
template <AllocBufferType type>
RegionDesc* RegionalHeap::AllocateThreadLocalRegion(bool expectPhysicalMem)
{
    if constexpr (type == AllocBufferType::TO) {
        return toSpace_.AllocateThreadLocalRegion(expectPhysicalMem);
    } else if constexpr (type == AllocBufferType::YOUNG) {
        return youngSpace_.AllocateThreadLocalRegion(expectPhysicalMem);
    } else if constexpr (type == AllocBufferType::OLD) {
        return oldSpace_.AllocateThreadLocalRegion(expectPhysicalMem);
    }
}

// used to dump a brief summary of all regions.
void RegionalHeap::DumpAllRegionSummary(const char* msg) const
{
    auto from = fromSpace_.GetAllocatedSize();
    auto exempt = fromSpace_.GetSurvivedSize();
    auto to = toSpace_.GetAllocatedSize();
    auto young = youngSpace_.GetAllocatedSize();
    auto old = oldSpace_.GetAllocatedSize();
    auto other = nonMovableSpace_.GetAllocatedSize() + largeSpace_.GetAllocatedSize() +
                                appSpawnSpace_.GetAllocatedSize() + readonlySpace_.GetAllocatedSize() +
                                rawpointerSpace_.GetAllocatedSize();

    std::ostringstream oss;
    oss << msg << "Current allocated: " << Pretty(from + to + young + old + other) << ". (from: " << Pretty(from)
        << "(exempt: " << Pretty(exempt) << "), to: " << Pretty(to) << ", young: " << Pretty(young)
        << ", old: " << Pretty(old) << ", other: " << Pretty(other) << ")";
    VLOG(DEBUG, oss.str().c_str());
}

// used to dump a detailed information of all regions.
void RegionalHeap::DumpAllRegionStats(const char* msg) const
{
    VLOG(DEBUG, msg);
    youngSpace_.DumpRegionStats();
    oldSpace_.DumpRegionStats();
    fromSpace_.DumpRegionStats();
    toSpace_.DumpRegionStats();
    nonMovableSpace_.DumpRegionStats();
    largeSpace_.DumpRegionStats();
    appSpawnSpace_.DumpRegionStats();
    rawpointerSpace_.DumpRegionStats();
    readonlySpace_.DumpRegionStats();

    regionManager_.DumpRegionStats();

    size_t usedUnits = GetUsedUnitCount();
    VLOG(DEBUG, "\tused units: %zu (%zu B)", usedUnits, usedUnits * RegionDesc::UNIT_SIZE);
}

HeapAddress RegionalHeap::TryAllocateOnce(size_t allocSize, AllocType allocType)
{
    if (UNLIKELY_CC(allocType == AllocType::READ_ONLY_OBJECT)) {
        return readonlySpace_.Alloc(allocSize);
    }
    if (UNLIKELY_CC(allocSize >= RegionDesc::LARGE_OBJECT_DEFAULT_THRESHOLD)) {
        return largeSpace_.Alloc(allocSize);
    }
    if (UNLIKELY_CC(allocType == AllocType::NONMOVABLE_OBJECT)) {
        return nonMovableSpace_.Alloc(allocSize);
    }
    AllocationBuffer* allocBuffer = AllocationBuffer::GetOrCreateAllocBuffer();
    return allocBuffer->Allocate(allocSize, allocType);
}

bool RegionalHeap::ShouldRetryAllocation(size_t& tryTimes) const
{
    {
        // check safepoint
        ScopedEnterSaferegion enterSaferegion(true);
    }

    if (!IsRuntimeThread() && tryTimes <= static_cast<size_t>(TryAllocationThreshold::RESCHEDULE)) {
        return true;
    } else if (tryTimes < static_cast<size_t>(TryAllocationThreshold::TRIGGER_OOM)) {
        if (Heap::GetHeap().IsGcStarted()) {
            ScopedEnterSaferegion enterSaferegion(false);
            Heap::GetHeap().GetCollectorResources().WaitForGCFinish();
        } else {
            Heap::GetHeap().GetCollector().RequestGC(GC_REASON_HEU, false, GC_TYPE_FULL);
        }
        return true;
    } else if (tryTimes == static_cast<size_t>(TryAllocationThreshold::TRIGGER_OOM)) {
        if (!Heap::GetHeap().IsGcStarted()) {
            VLOG(INFO, "gc is triggered for OOM");
            Heap::GetHeap().GetCollector().RequestGC(GC_REASON_OOM, false, GC_TYPE_FULL);
        } else {
            ScopedEnterSaferegion enterSaferegion(false);
            Heap::GetHeap().GetCollectorResources().WaitForGCFinish();
            tryTimes--;
        }
        return true;
    } else { //LCOV_EXCL_BR_LINE
        Heap::throwOOM();
        return false;
    }
}

uintptr_t RegionalHeap::AllocOldRegion()
{
    return oldSpace_.AllocFullRegion();
}

uintptr_t RegionalHeap::AllocateNonMovableRegion()
{
    return nonMovableSpace_.AllocFullRegion();
}

uintptr_t RegionalHeap::AllocLargeRegion(size_t size)
{
    return largeSpace_.Alloc(size, false);
}

uintptr_t RegionalHeap::AllocJitFortRegion(size_t size)
{
    uintptr_t addr = largeSpace_.Alloc(size, false);
    os::PrctlSetVMA(reinterpret_cast<void *>(addr), size, "ArkTS Code");
    MarkJitFortMemAwaitingInstall(reinterpret_cast<BaseObject*>(addr));
    return addr;
}

HeapAddress RegionalHeap::Allocate(size_t size, AllocType allocType)
{
    size_t tryTimes = 0;
    uintptr_t internalAddr = 0;
    size_t allocSize = ToAllocatedSize(size);
    do {
        tryTimes++;
        internalAddr = TryAllocateOnce(allocSize, allocType);
        if (LIKELY_CC(internalAddr != 0)) {
            break;
        }
        if (IsGcThread()) {
            return 0; // it means gc doesn't have enough space to move this object.
        }
        if (!ShouldRetryAllocation(tryTimes)) {
            break;
        }
        (void)sched_yield();
    } while (true);
    if (internalAddr == 0) {
        return 0;
    }
#if defined(COMMON_TSAN_SUPPORT)
    Sanitizer::TsanAllocObject(reinterpret_cast<void *>(internalAddr), allocSize);
#endif
    return internalAddr + HEADER_SIZE;
}

// Only used for serialization in which allocType and target memory should keep consistency.
HeapAddress RegionalHeap::AllocateNoGC(size_t size, AllocType allocType)
{
    bool allowGC = false;
    uintptr_t internalAddr = 0;
    size_t allocSize = ToAllocatedSize(size);
    if (UNLIKELY_CC(allocType == AllocType::NONMOVABLE_OBJECT)) {
        internalAddr = nonMovableSpace_.Alloc(allocSize, allowGC);
    } else if (LIKELY_CC(allocType == AllocType::MOVEABLE_OBJECT || allocType == AllocType::MOVEABLE_OLD_OBJECT)) {
        AllocationBuffer* allocBuffer = AllocationBuffer::GetOrCreateAllocBuffer();
        internalAddr = allocBuffer->Allocate(allocSize, allocType);
    } else { //LCOV_EXCL_BR_LINE
        // Unreachable for serialization
        UNREACHABLE_CC();
    }
    if (internalAddr == 0) {
        return 0;
    }
#if defined(COMMON_TSAN_SUPPORT)
    Sanitizer::TsanAllocObject(reinterpret_cast<void *>(internalAddr), allocSize);
#endif
    return internalAddr + HEADER_SIZE;
}

void RegionalHeap::CopyRegion(RegionDesc* region)
{
    LOGF_CHECK(region->IsFromRegion()) << "region type " << static_cast<uint8_t>(region->GetRegionType());
    DLOG(COPY, "try forward region %p @0x%zx+%zu type %u, live bytes %u",
        region, region->GetRegionStart(), region->GetRegionAllocatedSize(),
        region->GetRegionType(), region->GetLiveByteCount());

    if (region->GetLiveByteCount() == 0) {
        return;
    }

    int32_t rawPointerCount = region->GetRawPointerObjectCount();
    CHECK_CC(rawPointerCount == 0);
    Collector& collector = Heap::GetHeap().GetCollector();
    bool forwarded = region->VisitLiveObjectsUntilFalse(
        [&collector](BaseObject* obj) { return collector.ForwardObject(obj); });
    if (!forwarded) {
        DLOG(COPY, "failure to forward region %p @0x%zx+%zu units[%zu+%zu, %zu) type %u, %u live bytes",
            region, region->GetRegionStart(), region->GetRegionAllocatedSize(),
            region->GetUnitIdx(), region->GetUnitCount(), region->GetUnitIdx() + region->GetUnitCount(),
            region->GetRegionType(), region->GetLiveByteCount());

        fromSpace_.DeleteFromRegion(region);
        // since this region is possibly partially-forwarded, treat it as to-region.
        toSpace_.AddFullRegion(region);
    }
}

void RegionalHeap::Init(const RuntimeParam& param)
{
    MemoryMap::Option opt = MemoryMap::DEFAULT_OPTIONS;
    opt.tag = "region_heap";
    size_t heapSize = param.heapParam.heapSize * KB;

#ifndef PANDA_TARGET_32
    static constexpr uint64_t MAX_SUPPORT_CAPACITY = 4ULL * GB;
    // 2: double heap size
    LOGF_CHECK((heapSize / 2)<= MAX_SUPPORT_CAPACITY) << "Max support capacity 4G";
#endif

    size_t totalSize = RegionManager::GetHeapMemorySize(heapSize);
    size_t regionNum = RegionManager::GetHeapUnitCount(heapSize);
#if defined(COMMON_ASAN_SUPPORT)
    // asan's memory alias technique needs a shareable page
    opt.flags &= ~MAP_PRIVATE;
    opt.flags |= MAP_SHARED;
    DLOG(SANITIZER, "mmap flags set to 0x%x", opt.flags);
#endif
    // this must succeed otherwise it won't return
    map_ = MemoryMap::MapMemoryAlignInner4G(totalSize, totalSize, opt);

    size_t metadataSize = RegionManager::GetMetadataSize(regionNum);
    uintptr_t baseAddr = reinterpret_cast<uintptr_t>(map_->GetBaseAddr());
    os::PrctlSetVMA(reinterpret_cast<void*>(baseAddr), metadataSize, "ArkTS Heap CMCGC Metadata");
    os::PrctlSetVMA(reinterpret_cast<void*>(baseAddr + metadataSize), totalSize - metadataSize,
                    "ArkTS Heap CMCGC RegionHeap");

#if defined(COMMON_SANITIZER_SUPPORT)
    Sanitizer::OnHeapAllocated(map->GetBaseAddr(), map->GetMappedSize());
#endif

    HeapAddress metadata = reinterpret_cast<HeapAddress>(map_->GetBaseAddr());
    fromSpace_.SetExemptedRegionThreshold(param.heapParam.exemptionThreshold);
    regionManager_.Initialize(regionNum, metadata);
    reservedStart_ = regionManager_.GetRegionHeapStart();
    reservedEnd_ = reinterpret_cast<HeapAddress>(map_->GetMappedEndAddr());
#if defined(COMMON_DUMP_ADDRESS)
    VLOG(DEBUG, "region metadata@%zx, heap @[0x%zx+%zu, 0x%zx)", metadata, reservedStart, reservedEnd - reservedStart,
         reservedEnd);
#endif
    Heap::OnHeapCreated(reservedStart_);
    Heap::OnHeapExtended(reservedEnd_);
}

AllocationBuffer* AllocationBuffer::GetOrCreateAllocBuffer()
{
    auto* buffer = AllocationBuffer::GetAllocBuffer();
    if (buffer == nullptr) {
        buffer = new (std::nothrow) AllocationBuffer();
        LOGF_CHECK(buffer != nullptr) << "new region alloc buffer fail";
        buffer->Init();
        ThreadLocal::SetAllocBuffer(buffer);
    }
    return buffer;
}
void AllocationBuffer::ClearThreadLocalRegion()
{
    if (LIKELY_CC(tlRegion_ != RegionDesc::NullRegion())) {
        RegionalHeap& heap = reinterpret_cast<RegionalHeap&>(Heap::GetHeap().GetAllocator());
        heap.HandleFullThreadLocalRegion<AllocBufferType::YOUNG>(tlRegion_);
        tlRegion_ = RegionDesc::NullRegion();
    }
    if (LIKELY_CC(tlOldRegion_ != RegionDesc::NullRegion())) {
        RegionalHeap& heap = reinterpret_cast<RegionalHeap&>(Heap::GetHeap().GetAllocator());
        heap.HandleFullThreadLocalRegion<AllocBufferType::OLD>(tlOldRegion_);
        tlOldRegion_ = RegionDesc::NullRegion();
    }
    if (LIKELY_CC(tlToRegion_ != RegionDesc::NullRegion())) {
        RegionalHeap& heap = reinterpret_cast<RegionalHeap&>(Heap::GetHeap().GetAllocator());
        heap.HandleFullThreadLocalRegion<AllocBufferType::TO>(tlToRegion_);
        tlToRegion_ = RegionDesc::NullRegion();
    }
}

void AllocationBuffer::Unregister()
{
    Heap::GetHeap().UnregisterAllocBuffer(*this);
}

AllocationBuffer* AllocationBuffer::GetAllocBuffer() { return ThreadLocal::GetAllocBuffer(); }

AllocationBuffer::~AllocationBuffer()
{
    ClearThreadLocalRegion();
}

void AllocationBuffer::Init()
{
    static_assert(MEMBER_OFFSET_CC(AllocationBuffer, tlRegion_) == 0,
                  "need to modify the offset of this value in llvm-project at the same time");
    tlRegion_ = RegionDesc::NullRegion();
    tlOldRegion_ = RegionDesc::NullRegion();
    Heap::GetHeap().RegisterAllocBuffer(*this);
}

HeapAddress AllocationBuffer::ToSpaceAllocate(size_t totalSize)
{
    HeapAddress addr = 0;
    if (LIKELY_CC(tlToRegion_ != RegionDesc::NullRegion())) {
        addr = tlToRegion_->Alloc(totalSize);
    }

    if (UNLIKELY_CC(addr == 0)) {
        RegionalHeap& heapSpace = reinterpret_cast<RegionalHeap&>(Heap::GetHeap().GetAllocator());

        heapSpace.HandleFullThreadLocalRegion<AllocBufferType::TO>(tlToRegion_);
        tlToRegion_ = RegionDesc::NullRegion();

        RegionDesc* r = heapSpace.AllocateThreadLocalRegion<AllocBufferType::TO>(false);
        if (UNLIKELY_CC(r == nullptr)) {
            return 0;
        }

        tlToRegion_ = r;
        addr = tlToRegion_->Alloc(totalSize);
    }

    DLOG(ALLOC, "alloc to 0x%zx(%zu)", addr, totalSize);
    return addr;
}

HeapAddress AllocationBuffer::Allocate(size_t totalSize, AllocType allocType)
{
    // a hoisted specific fast path which can be inlined
    HeapAddress addr = 0;
    if (UNLIKELY_CC(allocType == AllocType::RAW_POINTER_OBJECT)) {
        return AllocateRawPointerObject(totalSize);
    }

    ASSERT_LOGF(allocType == AllocType::MOVEABLE_OBJECT || allocType == AllocType::MOVEABLE_OLD_OBJECT,
                "unexpected alloc type");

    if (allocType == AllocType::MOVEABLE_OBJECT) {
        if (LIKELY_CC(tlRegion_ != RegionDesc::NullRegion())) {
            addr = tlRegion_->Alloc(totalSize);
        }
    } else if (allocType == AllocType::MOVEABLE_OLD_OBJECT) {
        if (LIKELY_CC(tlOldRegion_ != RegionDesc::NullRegion())) {
            addr = tlOldRegion_->Alloc(totalSize);
        }
    }

    if (UNLIKELY_CC(addr == 0)) {
        addr = AllocateImpl(totalSize, allocType);
    }

    DLOG(ALLOC, "alloc 0x%zx(%zu)", addr, totalSize);
    return addr;
}

// try an allocation but do not handle failure
HeapAddress AllocationBuffer::AllocateImpl(size_t totalSize, AllocType allocType)
{
    RegionalHeap& heapSpace = reinterpret_cast<RegionalHeap&>(Heap::GetHeap().GetAllocator());

    // allocate new thread local region and try alloc
    if (allocType == AllocType::MOVEABLE_OBJECT) {
        heapSpace.HandleFullThreadLocalRegion<AllocBufferType::YOUNG>(tlRegion_);
        tlRegion_ = RegionDesc::NullRegion();

        RegionDesc* r = heapSpace.AllocateThreadLocalRegion<AllocBufferType::YOUNG>(false);
        if (UNLIKELY_CC(r == nullptr)) {
            return 0;
        }

        tlRegion_ = r;
        return tlRegion_->Alloc(totalSize);
    } else if (allocType == AllocType::MOVEABLE_OLD_OBJECT) {
        heapSpace.HandleFullThreadLocalRegion<AllocBufferType::OLD>(tlOldRegion_);
        tlOldRegion_ = RegionDesc::NullRegion();

        RegionDesc* r = heapSpace.AllocateThreadLocalRegion<AllocBufferType::OLD>(false);
        if (UNLIKELY_CC(r == nullptr)) {
            return 0;
        }

        tlOldRegion_ = r;
        return tlOldRegion_->Alloc(totalSize);
    }
    UNREACHABLE_CC();
}

HeapAddress AllocationBuffer::AllocateRawPointerObject(size_t totalSize)
{
    RegionDesc* region = tlRawPointerRegions_.GetHeadRegion();
    if (region != nullptr) {
        HeapAddress allocAddr = region->Alloc(totalSize);
        if (allocAddr != 0) {
            return allocAddr;
        }
    }
    RegionManager& manager = reinterpret_cast<RegionalHeap&>(Heap::GetHeap().GetAllocator()).GetRegionManager();
    size_t needRegionNum = totalSize / RegionDesc::UNIT_SIZE + 1;
    // region should have at least 2 unit
    needRegionNum = (needRegionNum == 1) ? 2 : needRegionNum;
    region = manager.TakeRegion(needRegionNum, RegionDesc::UnitRole::SMALL_SIZED_UNITS);
    if (region == nullptr) {
        return 0;
    }
    // region is enough for totalSize.
    HeapAddress allocAddr = region->Alloc(totalSize);
    ASSERT_LOGF(allocAddr != 0, "allocation failure");
    tlRawPointerRegions_.PrependRegion(region, RegionDesc::RegionType::TL_RAW_POINTER_REGION);
    return allocAddr;
}

#ifndef NDEBUG
bool RegionalHeap::IsHeapObject(HeapAddress addr) const
{
    if (!IsHeapAddress(addr)) {
        return false;
    }
    return true;
}
#endif
void RegionalHeap::FeedHungryBuffers()
{
    ScopedObjectAccess soa;
    AllocBufferManager::HungryBuffers hungryBuffers;
    allocBufferManager_->SwapHungryBuffers(hungryBuffers);
    RegionDesc* region = nullptr;
    for (auto* buffer : hungryBuffers) {
        if (buffer->GetPreparedRegion() != nullptr) { continue; }
        if (region == nullptr) {
            region = AllocateThreadLocalRegion<AllocBufferType::YOUNG>(true);
            if (region == nullptr) { return; }
        }
        if (buffer->SetPreparedRegion(region)) {
            region = nullptr;
        }
    }
    if (region != nullptr) {
        regionManager_.CollectRegion(region);
    }
}

void RegionalHeap::MarkRememberSet(const std::function<void(BaseObject*)>& func)
{
    oldSpace_.MarkRememberSet(func);
    nonMovableSpace_.MarkRememberSet(func);
    largeSpace_.MarkRememberSet(func);
    appSpawnSpace_.MarkRememberSet(func);
    rawpointerSpace_.MarkRememberSet(func);
}

void RegionalHeap::ForEachAwaitingJitFortUnsafe(const std::function<void(BaseObject*)>& visitor) const
{
    DCHECK_CC(BaseRuntime::GetInstance()->GetMutatorManager().WorldStopped());
    for (const auto jitFort : awaitingJitFort_) {
        visitor(jitFort);
    }
}

void RegionalHeap::MarkJitFortMemInstalled(void *thread, BaseObject *obj)
{
    std::lock_guard guard(awaitingJitFortMutex_);
    // GC is running, we should mark JitFort installled after GC finish
    if (Heap::GetHeap().GetGCPhase() != GCPhase::GC_PHASE_IDLE) {
        jitFortPostGCInstallTask_.emplace(nullptr, obj);
    } else {
        // a threadlocal JitFort mem
        if (thread) {
            MarkThreadLocalJitFortInstalled(thread, obj);
        } else {
            RegionDesc::GetAliveRegionDescAt(reinterpret_cast<uintptr_t>(obj))->SetJitFortAwaitInstallFlag(false);
        }
        awaitingJitFort_.erase(obj);
    }
}

void RegionalHeap::MarkJitFortMemAwaitingInstall(BaseObject *obj)
{
    std::lock_guard guard(awaitingJitFortMutex_);
    RegionDesc::GetAliveRegionDescAt(reinterpret_cast<uintptr_t>(obj))->SetJitFortAwaitInstallFlag(true);
    awaitingJitFort_.insert(obj);
}

void RegionalHeap::HandlePostGCODEitFortInstallTask()
{
    DCHECK_CC(Heap::GetHeap().GetGCPhase() == GCPhase::GC_PHASE_IDLE);
    while (!jitFortPostGCInstallTask_.empty()) {
        auto [thread, machineCode] = jitFortPostGCInstallTask_.top();
        MarkJitFortMemInstalled(thread, machineCode);
        jitFortPostGCInstallTask_.pop();
    }
}

} // namespace common
