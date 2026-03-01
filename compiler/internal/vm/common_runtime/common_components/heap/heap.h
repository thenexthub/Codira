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

#ifndef COMMON_COMPONENTS_HEAP_HEAP_H
#define COMMON_COMPONENTS_HEAP_HEAP_H

#include <cstdlib>
#include <functional>

#include "common_components/base/immortal_wrapper.h"
#include "common_components/common/base_object.h"
#include "common_components/common/type_def.h"
#include "common_components/heap/barrier/barrier.h"
#include "common_components/heap/collector/collector.h"
#include "common_components/heap/collector/heuristic_gc_policy.h"
#include "common_interfaces/base/runtime_param.h"
#include "common_interfaces/base_runtime.h"
#include "common_interfaces/profiler/heap_profiler_listener.h"

namespace common {
class Allocator;
class AllocationBuffer;
class FinalizerProcessor;
class CollectorResources;
class HeuristicGCPolicy;
using MemoryReduceDegree = common::MemoryReduceDegree;
using AppSensitiveStatus = common::AppSensitiveStatus;
using StartupStatus = common::StartupStatus;

class Heap {
public:
    // These need to keep same with that in `RegionDesc`
    static constexpr size_t NORMAL_UNIT_SIZE = 256 * 1024;
    static constexpr size_t NORMAL_UNIT_HEADER_SIZE = AlignUp<size_t>(2 * sizeof(void *) + sizeof(uint8_t), 8);
    static constexpr size_t NORMAL_UNIT_AVAILABLE_SIZE = NORMAL_UNIT_SIZE - NORMAL_UNIT_HEADER_SIZE;

    static constexpr size_t GetNormalRegionSize()
    {
        return NORMAL_UNIT_SIZE;
    }

    static constexpr size_t GetNormalRegionHeaderSize()
    {
        return NORMAL_UNIT_HEADER_SIZE;
    }

    static constexpr size_t GetNormalRegionAvailableSize()
    {
        return NORMAL_UNIT_AVAILABLE_SIZE;
    }

    static void throwOOM()
    {
        // Maybe we need to add heapdump logic here
        HeapProfilerListener::GetInstance().OnOutOfMemoryEventCb();
        LOG_COMMON(FATAL) << "Out of Memory, abort.";
        UNREACHABLE_CC();
    }
    static Heap& GetHeap();
    static Barrier& GetBarrier() { return *currentBarrierPtr_->load(std::memory_order_relaxed); }

    // concurrent gc uses barrier to access heap.
    static bool UseBarrier() { return currentBarrierPtr_->load(std::memory_order_relaxed) != stwBarrierPtr_; }

    // should be removed after HeapParam is supported
    virtual void Init(const RuntimeParam& param) = 0;
    virtual void Fini() = 0;

    virtual void StartRuntimeThreads() = 0;
    virtual void StopRuntimeThreads() = 0;

    virtual bool IsSurvivedObject(const BaseObject*) const = 0;
    bool IsGarbage(const BaseObject* obj) const { return !IsSurvivedObject(obj); }

    virtual bool IsGcStarted() const = 0;
    virtual void WaitForGCFinish() = 0;
    virtual void MarkGCStart() = 0;
    virtual void MarkGCFinish() = 0;

    virtual bool IsGCEnabled() const = 0;
    virtual void EnableGC(bool val) = 0;

    virtual HeapAddress Allocate(size_t size, AllocType allocType, bool allowGC) = 0;

    virtual Collector& GetCollector() = 0;
    virtual Allocator& GetAllocator() = 0;
    virtual HeuristicGCPolicy& GetHeuristicGCPolicy() = 0;
    virtual void TryHeuristicGC() = 0;
    virtual void TryIdleGC() = 0;
    virtual void NotifyNativeAllocation(size_t bytes) = 0;
    virtual void NotifyNativeFree(size_t bytes) = 0;
    virtual void NotifyNativeReset(size_t oldBytes, size_t newBytes) = 0;
    virtual size_t GetNotifiedNativeSize() const = 0;
    virtual void SetNativeHeapThreshold(size_t newThreshold) = 0;
    virtual size_t GetNativeHeapThreshold() const = 0;
    virtual void ChangeGCParams(bool isBackground) = 0;
    virtual void RecordAliveSizeAfterLastGC(size_t aliveBytes) = 0;
    virtual bool CheckAndTriggerHintGC(MemoryReduceDegree degree) = 0;
    virtual void NotifyHighSensitive(bool isStart) = 0;
    virtual void SetRecordHeapObjectSizeBeforeSensitive(size_t objSize) = 0;
    virtual AppSensitiveStatus GetSensitiveStatus() = 0;
    virtual StartupStatus GetStartupStatus() = 0;
    virtual bool OnStartupEvent() const = 0;
    /* to avoid misunderstanding, variant types of heap size are defined as followed:
     * |------------------------------ max capacity ---------------------------------|
     * |------------------------------ current capacity ------------------------|
     * |------------------------------ committed size -----------------------|
     * |------------------------------ used size -------------------------|
     * |------------------------------ allocated size -------------|
     * |------------------------------ net size ------------|
     * so that inequality size <= capacity <= max capacity always holds.
     */
    virtual size_t GetMaxCapacity() const = 0;

    // or current capacity: a continuous address space to help heap management such as GC.
    virtual size_t GetCurrentCapacity() const = 0;

    // already used by allocator, including memory block cached for speeding up allocation.
    // we measure it in OS page granularity because physical memory is occupied by page.
    virtual size_t GetUsedPageSize() const = 0;

    // total memory allocated for each allocation request, including memory fragment for alignment or padding.
    virtual size_t GetAllocatedSize() const = 0;

    virtual size_t GetSurvivedSize() const = 0;

    virtual size_t GetRemainHeapSize() const = 0;

    virtual size_t GetAccumulatedAllocateSize() const = 0;
    virtual size_t GetAccumulatedFreeSize() const = 0;

    virtual HeapAddress GetStartAddress() const = 0;
    virtual HeapAddress GetSpaceEndAddress() const = 0;

    // IsHeapAddress is a range-based check, used to quickly identify heap address,
    // assuming non-heap address never falls into this address range.
    static bool IsHeapAddress(HeapAddress addr) { return (addr >= heapStartAddr_) && (addr < heapCurrentEnd_); }

    static bool IsTaggedObject(HeapAddress addr)
    {
        // relies on the definition of ArkTs
        static constexpr uint64_t TAG_BITS_SHIFT = 48;
        static constexpr uint64_t TAG_MARK = 0xFFFFULL << TAG_BITS_SHIFT;
        static constexpr uint64_t TAG_SPECIAL = 0x02ULL;
        static constexpr uint64_t TAG_BOOLEAN = 0x04ULL;
        static constexpr uint64_t TAG_HEAP_OBJECT_MASK = TAG_MARK | TAG_SPECIAL | TAG_BOOLEAN;

        if ((addr & TAG_HEAP_OBJECT_MASK) == 0) {
            return true;
        }

        return false;
    }

    static void MarkJitFortMemInstalled(void *thread, void *obj);

    static bool IsHeapAddress(const void* addr) { return IsHeapAddress(reinterpret_cast<HeapAddress>(addr)); }

    virtual void InstallBarrier(const GCPhase) = 0;

    virtual GCPhase GetGCPhase() const = 0;

    virtual void SetGCPhase(const GCPhase phase) = 0;

    virtual bool ForEachObject(const std::function<void(BaseObject*)>&, bool safe) = 0;

    virtual void VisitStaticRoots(const RefFieldVisitor& visitor) = 0;

    virtual FinalizerProcessor& GetFinalizerProcessor() = 0;

    virtual CollectorResources& GetCollectorResources() = 0;

    virtual void RegisterAllocBuffer(AllocationBuffer& buffer) = 0;

    virtual void UnregisterAllocBuffer(AllocationBuffer& buffer) = 0;

    virtual void StopGCWork() = 0;

    virtual GCReason GetGCReason() = 0;

    virtual void SetGCReason(GCReason reason) = 0;

    virtual bool InRecentSpace(const void *addr) = 0;
    virtual bool GetForceThrowOOM() const = 0;
    virtual void SetForceThrowOOM(bool val) = 0;

    static void OnHeapCreated(HeapAddress startAddr)
    {
        heapStartAddr_ = startAddr;
        heapCurrentEnd_ = 0;
    }

    static void OnHeapExtended(HeapAddress newEnd) { heapCurrentEnd_ = newEnd; }

    virtual ~Heap() {}
    static std::atomic<Barrier*>* currentBarrierPtr_; // record ptr for fast access
    static Barrier* stwBarrierPtr_;      // record nonGC barrier
    static HeapAddress heapStartAddr_;
    static HeapAddress heapCurrentEnd_;
}; // class Heap
} // namespace common
#endif
