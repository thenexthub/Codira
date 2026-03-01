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

#ifndef COMMON_COMPONENTS_HEAP_ALLOCATOR_REGION_SPACE_H
#define COMMON_COMPONENTS_HEAP_ALLOCATOR_REGION_SPACE_H

#include <list>
#include <map>
#include <set>
#include <thread>
#include <vector>

#include "common_components/heap/allocator/alloc_util.h"
#include "common_components/heap/allocator/allocator.h"
#include "common_components/heap/allocator/region_manager.h"
#include "common_components/heap/space/young_space.h"
#include "common_components/heap/space/old_space.h"
#include "common_components/heap/space/from_space.h"
#include "common_components/heap/space/to_space.h"
#include "common_components/heap/space/nonmovable_space.h"
#include "common_components/mutator/mutator.h"
#include "common_components/heap/space/appspawn_space.h"
#include "common_components/heap/space/large_space.h"
#include "common_components/heap/space/rawpointer_space.h"
#include "common_components/heap/space/readonly_space.h"
#include "common_interfaces/objects/base_object.h"
#if defined(COMMON_SANITIZER_SUPPORT)
#include "common_components/sanitizer/sanitizer_interface.h"
#endif
#include "common_interfaces/base_runtime.h"

namespace common {
class Taskpool;

// RegionalHeap aims to be the API for other components of runtime
// the complication of implementation is delegated to RegionManager
// allocator should not depend on any assumptions on the details of RegionManager

class RegionalHeap : public Allocator {
public:
    static size_t ToAllocatedSize(size_t objSize)
    {
        size_t size = objSize + HEADER_SIZE;
        return RoundUp<size_t>(size, ALLOC_ALIGN);
    }

    static size_t GetAllocSize(const BaseObject& obj)
    {
        size_t objSize = obj.GetSize();
        return ToAllocatedSize(objSize);
    }

    RegionalHeap() : youngSpace_(regionManager_), oldSpace_(regionManager_),
        fromSpace_(regionManager_, *this), toSpace_(regionManager_),
        nonMovableSpace_(regionManager_), largeSpace_(regionManager_),
        appSpawnSpace_(regionManager_), rawpointerSpace_(regionManager_),
        readonlySpace_(regionManager_) {}

    NO_INLINE_CC virtual ~RegionalHeap()
    {
        if (allocBufferManager_ != nullptr) {
            delete allocBufferManager_;
            allocBufferManager_ = nullptr;
        }
#if defined(COMMON_SANITIZER_SUPPORT)
        Sanitizer::OnHeapDeallocated(map->GetBaseAddr(), map->GetMappedSize());
#endif
        MemoryMap::DestroyMemoryMap(map_);
    }

    void Init(const RuntimeParam &param) override;

    template <AllocBufferType type>
    RegionDesc* AllocateThreadLocalRegion(bool expectPhysicalMem = false);

    template <AllocBufferType type = AllocBufferType::YOUNG>
    void HandleFullThreadLocalRegion(RegionDesc* region) noexcept
    {
        if (region == RegionDesc::NullRegion()) {
            return;
        }
        ASSERT_LOGF(region->IsThreadLocalRegion() || region->IsToRegion() || region->IsOldRegion(),
                    "unexpected region type");

        if constexpr (type == AllocBufferType::YOUNG) {
            ASSERT_LOGF(!IsGcThread(), "unexpected gc thread for old space");
            youngSpace_.HandleFullThreadLocalRegion(region);
        } else if constexpr (type == AllocBufferType::OLD) {
            ASSERT_LOGF(!IsGcThread(), "unexpected gc thread for old space");
            oldSpace_.HandleFullThreadLocalRegion(region);
        } else if constexpr (type == AllocBufferType::TO) {
            toSpace_.HandleFullThreadLocalRegion(region);
        }
    }

    // only used for deserialize allocation, allocate one region and regard it as full region
    uintptr_t AllocOldRegion();
    uintptr_t AllocateNonMovableRegion();
    uintptr_t AllocLargeRegion(size_t size);
    uintptr_t AllocJitFortRegion(size_t size);

    HeapAddress Allocate(size_t size, AllocType allocType) override;

    HeapAddress AllocateNoGC(size_t size, AllocType allocType) override;

    RegionManager& GetRegionManager() noexcept { return regionManager_; }

    FromSpace& GetFromSpace() noexcept { return fromSpace_; }

    ToSpace& GetToSpace() noexcept { return toSpace_; }

    OldSpace& GetOldSpace() noexcept { return oldSpace_; }

    YoungSpace& GetYoungSpace() noexcept { return youngSpace_; }

    HeapAddress GetSpaceStartAddress() const override { return reservedStart_; }

    HeapAddress GetSpaceEndAddress() const override { return reservedEnd_; }

    size_t GetCurrentCapacity() const override { return regionManager_.GetInactiveZone() - reservedStart_; }
    size_t GetMaxCapacity() const override { return reservedEnd_ - reservedStart_; }

    inline size_t GetRecentAllocatedSize() const
    {
        return youngSpace_.GetRecentAllocatedSize() + nonMovableSpace_.GetRecentAllocatedSize() +
               largeSpace_.GetRecentAllocatedSize();
    }

    // size of objects survived in previous gc.
    size_t GetSurvivedSize() const override
    {
        return fromSpace_.GetSurvivedSize() + toSpace_.GetAllocatedSize() + youngSpace_.GetAllocatedSize() +
            oldSpace_.GetAllocatedSize() + nonMovableSpace_.GetSurvivedSize() + largeSpace_.GetAllocatedSize();
    }

    inline size_t GetUsedUnitCount() const
    {
        return fromSpace_.GetUsedUnitCount() + toSpace_.GetUsedUnitCount() + youngSpace_.GetUsedUnitCount() +
            oldSpace_.GetUsedUnitCount() + nonMovableSpace_.GetUsedUnitCount() + largeSpace_.GetUsedUnitCount() +
            appSpawnSpace_.GetUsedUnitCount() + readonlySpace_.GetUsedUnitCount() + rawpointerSpace_.GetUsedUnitCount();
    }

    size_t GetUsedPageSize() const override
    {
        return GetUsedUnitCount() * RegionDesc::UNIT_SIZE;
    }

    inline size_t GetTargetSize() const
    {
        double heapUtilization = BaseRuntime::GetInstance()->GetHeapParam().heapUtilization;
        return static_cast<size_t>(GetUsedPageSize() / heapUtilization);
    }

    size_t GetAllocatedBytes() const override
    {
        return fromSpace_.GetAllocatedSize() + toSpace_.GetAllocatedSize() + youngSpace_.GetAllocatedSize() +
            oldSpace_.GetAllocatedSize() + nonMovableSpace_.GetAllocatedSize() + LargeObjectSize();
    }

    size_t LargeObjectSize() const override { return largeSpace_.GetAllocatedSize(); }

    size_t FromSpaceSize() const { return fromSpace_.GetAllocatedSize(); }
    // note: it doesn't contain exemptFromRegion
    size_t FromRegionSize() const { return fromSpace_.GetFromRegionAllocatedSize(); }
    size_t ToSpaceSize() const { return toSpace_.GetAllocatedSize(); }

    size_t NonMovableSpaceSize() const { return nonMovableSpace_.GetAllocatedSize(); }

#ifndef NDEBUG
    bool IsHeapObject(HeapAddress addr) const override;
#endif

    size_t ReclaimGarbageMemory(bool releaseAll) override
    {
        {
            COMMON_PHASE_TIMER("ReclaimGarbageRegions");
            regionManager_.ReclaimGarbageRegions();
        }

        COMMON_PHASE_TIMER("ReleaseGarbageMemory");
        if (releaseAll) {
            return regionManager_.ReleaseGarbageRegions(0);
        } else {
            size_t size = GetAllocatedBytes();
            double cachedRatio = 1 - BaseRuntime::GetInstance()->GetHeapParam().heapUtilization;
            size_t targetCachedSize = static_cast<size_t>(size * cachedRatio);
            return regionManager_.ReleaseGarbageRegions(targetCachedSize);
        }
    }

    bool ForEachObject(const std::function<void(BaseObject*)>& visitor, bool safe) const override
    {
        if (UNLIKELY_CC(safe)) {
            regionManager_.ForEachObjectSafe(visitor);
        } else {
            regionManager_.ForEachObjectUnsafe(visitor);
        }
        return true;
    }

    void ExemptFromSpace()
    {
        COMMON_PHASE_TIMER("ExemptFromRegions");
        fromSpace_.ExemptFromRegions();
    }

    BaseObject* RouteObject(BaseObject* fromObj, size_t size)
    {
        AllocationBuffer* buffer = AllocationBuffer::GetOrCreateAllocBuffer();
        uintptr_t toAddr = buffer->ToSpaceAllocate(size);
        return reinterpret_cast<BaseObject*>(toAddr);
    }

    void CopyFromSpace(Taskpool *threadPool)
    {
        COMMON_PHASE_TIMER("CopyFromRegions");
        fromSpace_.CopyFromRegions(threadPool);
    }

    FixHeapTaskList CollectFixTasks()
    {
        FixHeapTaskList taskList;
        youngSpace_.CollectFixTasks(taskList);
        oldSpace_.CollectFixTasks(taskList);
        fromSpace_.CollectFixTasks(taskList);
        toSpace_.CollectFixTasks(taskList);
        nonMovableSpace_.CollectFixTasks(taskList);
        largeSpace_.CollectFixTasks(taskList);
        rawpointerSpace_.CollectFixTasks(taskList);
        appSpawnSpace_.CollectFixTasks(taskList);

        return taskList;
    }

    void MarkAwaitingJitFort()
    {
        ForEachAwaitingJitFortUnsafe(MarkObject);
    }

    void ClearJitFortAwaitingMark()
    {
        HandlePostGCODEitFortInstallTask();
    }

    void MarkJitFortMemInstalled(void *thread, BaseObject *obj);

    void SetReadOnlyToROSpace()
    {
        readonlySpace_.SetReadOnlyToRORegionList();
    }

    void ClearReadOnlyFromROSpace()
    {
        readonlySpace_.ClearReadOnlyFromRORegionList();
    }

    size_t CollectLargeGarbage() { return largeSpace_.CollectLargeGarbage(); }

    void CollectFromSpaceGarbage()
    {
        regionManager_.CollectFromSpaceGarbage(fromSpace_.GetFromRegionList());
    }

    void HandlePromotion()
    {
        fromSpace_.GetPromotedTo(oldSpace_);
        toSpace_.GetPromotedTo(oldSpace_);
    }

    void AssembleSmallGarbageCandidates()
    {
        youngSpace_.AssembleGarbageCandidates(fromSpace_);
        oldSpace_.AssembleRecentFull();
        if (Heap::GetHeap().GetGCReason() != GC_REASON_YOUNG) {
            oldSpace_.ClearRSet();
            oldSpace_.AssembleGarbageCandidates(fromSpace_);
            nonMovableSpace_.ClearRSet();
            largeSpace_.ClearRSet();
            appSpawnSpace_.ClearRSet();
            rawpointerSpace_.ClearRSet();
        }
    }

    void CollectAppSpawnSpaceGarbage()
    {
        regionManager_.CollectFromSpaceGarbage(fromSpace_.GetFromRegionList());
        appSpawnSpace_.ReassembleAppspawnSpace(fromSpace_.GetExemptedRegionList());
        appSpawnSpace_.ReassembleAppspawnSpace(toSpace_.GetTlToRegionList());
        appSpawnSpace_.ReassembleAppspawnSpace(toSpace_.GetFullToRegionList());
    }

    void ClearAllGCInfo()
    {
        youngSpace_.ClearAllGCInfo();
        oldSpace_.ClearAllGCInfo();
        toSpace_.ClearAllGCInfo();
        fromSpace_.ClearAllGCInfo();
        nonMovableSpace_.ClearAllGCInfo();
        largeSpace_.ClearAllGCInfo();
        appSpawnSpace_.ClearAllGCInfo();
        rawpointerSpace_.ClearAllGCInfo();
        readonlySpace_.ClearAllGCInfo();
    }

    void AssembleGarbageCandidates()
    {
        AssembleSmallGarbageCandidates();
        nonMovableSpace_.AssembleGarbageCandidates();
        largeSpace_.AssembleGarbageCandidates();
    }

    void DumpAllRegionSummary(const char* msg) const;
    void DumpAllRegionStats(const char* msg) const;

    void CountLiveObject(const BaseObject* obj) { regionManager_.CountLiveObject(obj); }

    void PrepareMarking()
    {
        AllocBufferVisitor visitor = [](AllocationBuffer& regionBuffer) {
            RegionDesc* region = regionBuffer.GetRegion<AllocBufferType::YOUNG>();
            if (region != RegionDesc::NullRegion()) {
                region->SetMarkingLine();
            }
            region = regionBuffer.GetRegion<AllocBufferType::OLD>();
            if (region != RegionDesc::NullRegion()) {
                region->SetMarkingLine();
            }
        };
        VisitAllocBuffers(visitor);

        nonMovableSpace_.PrepareMarking();
        readonlySpace_.PrepareMarking();
    }

    void PrepareForward()
    {
        AllocBufferVisitor visitor = [](AllocationBuffer& regionBuffer) {
            RegionDesc* region = regionBuffer.GetRegion<AllocBufferType::YOUNG>();
            if (region != RegionDesc::NullRegion()) {
                region->SetCopyLine();
            }
            region = regionBuffer.GetRegion<AllocBufferType::OLD>();
            if (region != RegionDesc::NullRegion()) {
                region->SetCopyLine();
            }
        };
        VisitAllocBuffers(visitor);

        nonMovableSpace_.PrepareForward();
        readonlySpace_.PrepareForward();
    }
    void FeedHungryBuffers() override;

    // markObj
    static bool MarkObject(const BaseObject* obj)
    {
        RegionDesc* regionInfo = RegionDesc::GetAliveRegionDescAt(reinterpret_cast<HeapAddress>(obj));
        return regionInfo->MarkObject(obj);
    }
    static bool ResurrentObject(const BaseObject* obj)
    {
        RegionDesc* regionInfo = RegionDesc::GetAliveRegionDescAt(reinterpret_cast<HeapAddress>(obj));
        return regionInfo->ResurrentObject(obj);
    }

    static bool EnqueueObject(const BaseObject* obj)
    {
        RegionDesc* regionInfo = RegionDesc::GetAliveRegionDescAt(reinterpret_cast<HeapAddress>(obj));
        return regionInfo->EnqueueObject(obj);
    }

    static bool IsMarkedObject(const BaseObject* obj)
    {
        RegionDesc* regionInfo = RegionDesc::GetAliveRegionDescAt(reinterpret_cast<HeapAddress>(obj));
        return regionInfo->IsMarkedObject(obj);
    }

    static bool IsResurrectedObject(const BaseObject* obj)
    {
        RegionDesc* regionInfo = RegionDesc::GetAliveRegionDescAt(reinterpret_cast<HeapAddress>(obj));
        return regionInfo->IsResurrectedObject(obj);
    }

    static bool IsEnqueuedObject(const BaseObject* obj)
    {
        RegionDesc* regionInfo = RegionDesc::GetAliveRegionDescAt(reinterpret_cast<HeapAddress>(obj));
        return regionInfo->IsEnqueuedObject(obj);
    }

    static bool IsNewObjectSinceMarking(const BaseObject* object)
    {
        RegionDesc* region = RegionDesc::GetAliveRegionDescAt(reinterpret_cast<uintptr_t>(object));
        ASSERT_LOGF(region != nullptr, "region is nullptr");
        return region->IsNewObjectSinceMarking(object);
    }

    static bool IsReadOnlyObject(const BaseObject* object)
    {
        RegionDesc* region = RegionDesc::GetAliveRegionDescAt(reinterpret_cast<uintptr_t>(object));
        ASSERT_LOGF(region != nullptr, "region is nullptr");
        return region->IsReadOnlyRegion();
    }

    static bool IsYoungSpaceObject(const BaseObject* object)
    {
        RegionDesc* region = RegionDesc::GetAliveRegionDescAt(reinterpret_cast<uintptr_t>(object));
        ASSERT_LOGF(region != nullptr, "region is nullptr");
        return region->IsInYoungSpace();
    }

    void AddRawPointerObject(BaseObject* obj)
    {
        RegionDesc* region = RegionDesc::GetAliveRegionDescAt(reinterpret_cast<HeapAddress>(obj));
        region->IncRawPointerObjectCount();
        if (region->IsFromRegion() && fromSpace_.TryDeleteFromRegion(region, RegionDesc::RegionType::FROM_REGION,
                RegionDesc::RegionType::RAW_POINTER_REGION)) {
            GCPhase phase = Heap::GetHeap().GetGCPhase();
            CHECK_CC(phase != GCPhase::GC_PHASE_COPY && phase != GCPhase::GC_PHASE_PRECOPY);
            rawpointerSpace_.AddRawPointerRegion(region);
        } else {
            CHECK_CC(region->GetRegionType() != RegionDesc::RegionType::LONE_FROM_REGION);
        }
    }

    void RemoveRawPointerObject(BaseObject* obj)
    {
        RegionDesc* region = RegionDesc::GetAliveRegionDescAt(reinterpret_cast<HeapAddress>(obj));
        region->DecRawPointerObjectCount();
    }

    void AddRawPointerRegion(RegionDesc* region)
    {
        rawpointerSpace_.AddRawPointerRegion(region);
    }

    void CopyRegion(RegionDesc* region);
    void MarkRememberSet(const std::function<void(BaseObject *)> &func);

    friend class Allocator;
private:
    enum class TryAllocationThreshold {
        RESCHEDULE = 3,
        TRIGGER_OOM = 4,
    };
    HeapAddress TryAllocateOnce(size_t allocSize, AllocType allocType);
    bool ShouldRetryAllocation(size_t& tryTimes) const;

    void ForEachAwaitingJitFortUnsafe(const std::function<void(BaseObject*)>& visitor) const;
    void MarkJitFortMemAwaitingInstall(BaseObject *obj);
    void HandlePostGCODEitFortInstallTask();

    HeapAddress reservedStart_ = 0;
    HeapAddress reservedEnd_ = 0;
    RegionManager regionManager_;
    MemoryMap* map_{ nullptr };

    // Awaiting JitFort object has no references from other objects,
    // but we need to keep them as live untill jit compilation has finished installing.
    std::set<BaseObject*> awaitingJitFort_;
    std::stack<std::pair<void*, BaseObject*>> jitFortPostGCInstallTask_;
    std::mutex awaitingJitFortMutex_;

    YoungSpace youngSpace_;
    OldSpace oldSpace_;
    FromSpace fromSpace_;
    ToSpace toSpace_;
    NonMovableSpace nonMovableSpace_;
    LargeSpace largeSpace_;
    AppSpawnSpace appSpawnSpace_;
    RawPointerSpace rawpointerSpace_;
    ReadOnlySpace readonlySpace_;
};
} // namespace common

#endif // COMMON_COMPONENTS_HEAP_ALLOCATOR_REGION_SPACE_H
