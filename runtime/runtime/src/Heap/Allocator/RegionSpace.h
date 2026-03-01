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


#ifndef MRT_REGION_SPACE_H
#define MRT_REGION_SPACE_H

#include <cassert>
#include <list>
#include <map>
#include <set>
#include <thread>
#include <vector>

#include "AllocUtil.h"
#include "Allocator.h"
#include "ExceptionManager.h"
#include "Mutator/Mutator.h"
#include "RegionManager.h"
#if defined(CODIRA_SANITIZER_SUPPORT) || defined(CODIRA_GWPASAN_SUPPORT)
#include "Sanitizer/SanitizerInterface.h"
#endif

namespace MapleRuntime {
// RegionSpace aims to be the API for other components of runtime
// the complication of implementation is delegated to RegionManager
// allocator should not depend on any assumptions on the details of RegionManager
class RegionSpace : public Allocator {
public:
    static size_t ToAllocSize(size_t objSize)
    {
        size_t size = objSize + HEADER_SIZE;
        return RoundUp<size_t>(size, ALLOC_ALIGN);
    }

    static size_t GetAllocSize(const BaseObject& obj)
    {
        size_t objSize = obj.GetSize();
        return ToAllocSize(objSize);
    }

    RegionSpace() = default;
    ATTR_NO_INLINE ~RegionSpace() override
    {
        if (allocBufferManager != nullptr) {
            delete allocBufferManager;
            allocBufferManager = nullptr;
        }
#if defined(CODIRA_SANITIZER_SUPPORT) || defined(CODIRA_GWPASAN_SUPPORT)
        Sanitizer::OnHeapDeallocated(map->GetBaseAddr(), map->GetMappedSize());
#endif
        MemMap::DestroyMemMap(map);
    }

    void Init(const HeapParam&) override;

    MAddress Allocate(size_t size, AllocType allocType) override;

    RegionManager& GetRegionManager() noexcept { return regionManager; }

    MAddress GetSpaceStartAddress() const override { return reservedStart; }

    MAddress GetSpaceEndAddress() const override { return reservedEnd; }

    size_t GetCurrentCapacity() const override { return regionManager.GetInactiveZone() - reservedStart; }
    size_t GetMaxCapacity() const override { return reservedEnd - reservedStart; }

    inline size_t GetRecentAllocatedSize() const { return regionManager.GetRecentAllocatedSize(); }

    // size of objects survived in previous gc.
    inline size_t GetSurvivedSize() const { return regionManager.GetSurvivedSize(); }

    size_t GetUsedPageSize() const override { return regionManager.GetUsedRegionSize(); }

    inline size_t GetTargetSize() const
    {
        double heapUtilization = CodiraRuntime::GetHeapParam().heapUtilization;
        return static_cast<size_t>(GetUsedPageSize() / heapUtilization);
    }

    size_t AllocatedBytes() const override { return regionManager.GetAllocatedSize(); }

    size_t LargeObjectBytes() const override { return regionManager.GetLargeObjectSize(); }

    size_t FromSpaceSize() const { return regionManager.GetFromSpaceSize(); }

    size_t PinnedSpaceSize() const { return regionManager.GetPinnedSpaceSize(); }

#if defined(MRT_DEBUG) && (MRT_DEBUG == 1)
    bool IsHeapObject(MAddress addr) const override;
#endif

    // info dump
    void GetInstances(const TypeInfo*, bool, size_t, std::vector<MObject*>&) const {}
    void ClassInstanceNum(std::map<CString, long>&) const {}

    size_t ReclaimGarbageMemory(bool releaseAll) override
    {
        {
            MRT_PHASE_TIMER("ReclaimGarbageRegions");
            regionManager.ReclaimGarbageRegions();
        }

        MRT_PHASE_TIMER("ReleaseGarbageMemory");
        if (releaseAll) {
            return regionManager.ReleaseGarbageRegions(0);
        } else {
            size_t size = regionManager.GetAllocatedSize();
            double cachedRatio = 1 - CodiraRuntime::GetHeapParam().heapUtilization;
            size_t targetCachedSize = static_cast<size_t>(size * cachedRatio);
            return regionManager.ReleaseGarbageRegions(targetCachedSize);
        }
    }
#if defined(__EULER__)
    void TryReclaimGarbageMemory() override
    {
        double cachedRatio = regionManager.GetCacheRatio();
        if (cachedRatio == 1.0) { // 1.0 is the default value
            return;
        }
        {
            MRT_PHASE_TIMER("TryReclaimGarbageRegions");
            regionManager.ReclaimGarbageRegions();
        }
        MRT_PHASE_TIMER("TryReleaseGarbageMemory");
        size_t size = regionManager.GetAllocatedSize();
        size_t targetCachedSize = static_cast<size_t>(size * cachedRatio);
        regionManager.ReleaseGarbageRegions(targetCachedSize);
        return;
    }
#endif
    bool ForEachObj(const std::function<void(BaseObject*)>& visitor, bool safe) const override
    {
        if (UNLIKELY(safe)) {
            regionManager.ForEachObjSafe(visitor);
        } else {
            regionManager.ForEachObjUnsafe(visitor);
        }
        return true;
    }

    // Return the garbage size of from space.
    size_t RefineFromSpace()
    {
        MRT_PHASE_TIMER("ExemptFromRegions");
        return regionManager.ExemptFromRegions();
    }

    BaseObject* RouteObject(BaseObject* fromObj) { return regionManager.RouteObject(fromObj); }

    void PrepareFromSpace() { regionManager.PrepareFromRegionList(); }

    void ClearAllLiveInfo() { regionManager.ClearAllLiveInfo(); }

    void ForwardFromSpace(GCThreadPool* threadPool)
    {
        MRT_PHASE_TIMER("ForwardFromRegions");
        regionManager.ForwardFromRegions(threadPool);
    }

    size_t CollectLargeGarbage() { return regionManager.CollectLargeGarbage(); }

    size_t CollectPinnedGarbage() { return regionManager.CollectPinnedGarbage(); }

    void CollectFromSpaceGarbage()
    {
        regionManager.CollectFromSpaceGarbage();
        regionManager.ReassembleFromSpace();
    }

    void AssembleGarbageCandidates(bool collectAll = false)
    {
        regionManager.AssembleSmallGarbageCandidates();
        regionManager.AssemblePinnedGarbageCandidates(collectAll);
        regionManager.AssembleLargeGarbageCandidates();
    }

    void DumpRegionStats(const char* msg) const { regionManager.DumpRegionStats(msg); }

    void CountLiveObject(const BaseObject* obj) { regionManager.CountLiveObject(obj); }

    void PrepareTrace() { regionManager.PrepareTrace(); }
    void FeedHungryBuffers() override;

    static bool MarkObject(const BaseObject* obj)
    {
        RegionInfo* regionInfo = RegionInfo::GetRegionInfoAt(reinterpret_cast<MAddress>(obj));
        return regionInfo->MarkObject(obj);
    }

    static bool IsMarkedObject(const BaseObject* obj)
    {
        RegionInfo* regionInfo = RegionInfo::GetRegionInfoAt(reinterpret_cast<MAddress>(obj));
        return regionInfo->IsMarkedObject(obj);
    }

    static bool ShouldEnqueue(const BaseObject* obj)
    {
        RegionInfo* regionInfo = RegionInfo::GetRegionInfoAt(reinterpret_cast<MAddress>(obj));
        if (regionInfo->IsTraceRegion()) {
            return false;
        }
        size_t offset = regionInfo->GetAddressOffset(reinterpret_cast<MAddress>(obj));
        if (regionInfo->IsMarkedObject(offset)) {
            return false;
        }
        return !regionInfo->EnqueueObject(obj, offset);
    }

    static bool IsResurrectedObject(const BaseObject* obj)
    {
        RegionInfo* regionInfo = RegionInfo::GetRegionInfoAt(reinterpret_cast<MAddress>(obj));
        return regionInfo->IsResurrectedObject(obj);
    }

    void AddRawPointerObject(BaseObject* obj) { regionManager.AddRawPointerObject(obj); }

    void RemoveRawPointerObject(BaseObject* obj) { regionManager.RemoveRawPointerObject(obj); }

    friend class Allocator;

private:
    enum class TryAllocationThreshold {
        RESCHEDULE = 3,
        TRIGGER_OOM = 5,
    };
    MAddress TryAllocateOnce(size_t allocSize, AllocType allocType);
    bool ShouldRetryAllocation(size_t& tryTimes, size_t size) const;
    MAddress reservedStart = 0;
    MAddress reservedEnd = 0;
    RegionManager regionManager;
    MemMap* map{ nullptr };
};
} // namespace MapleRuntime
#endif // MRT_REGION_SPACE_H
