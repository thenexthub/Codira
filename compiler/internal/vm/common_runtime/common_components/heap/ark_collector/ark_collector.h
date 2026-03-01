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

#ifndef COMMON_COMPONENTS_HEAP_ARK_COLLECTOR_ARKCOLLECTOR_H
#define COMMON_COMPONENTS_HEAP_ARK_COLLECTOR_ARKCOLLECTOR_H

#include <unordered_map>

#include "common_components/heap/allocator/regional_heap.h"
#include "common_components/heap/collector/copy_data_manager.h"
#include "common_components/heap/collector/marking_collector.h"
#include "common_interfaces/base_runtime.h"

namespace common {

class CopyTable {
public:
    explicit CopyTable(RegionalHeap& space) : theSpace(space) {}

    // if object is not relocated (forwarded or compacted), return nullptr.
    BaseObject* RouteObject(BaseObject* old, size_t size)
    {
        BaseObject* toAddress = theSpace.RouteObject(old, size);
        return toAddress;
    }

    BaseObject* GetForwardingPointer(BaseObject* old)
    {
        return old->GetForwardingPointer();
    }

    RegionalHeap& theSpace;
};

enum class GCMode: uint8_t {
    CMC = 0,
    CONCURRENT_MARK = 1,
    STW = 2
};

class ArkCollector : public MarkingCollector {
public:
    explicit ArkCollector(Allocator& allocator, CollectorResources& resources)
        : MarkingCollector(allocator, resources), fwdTable_(reinterpret_cast<RegionalHeap&>(allocator))
    {
        collectorType_ = CollectorType::SMOOTH_COLLECTOR;
    }

    ~ArkCollector() override = default;

    void Init(const RuntimeParam& param) override
    {
        HeapBitmapManager::GetHeapBitmapManager().InitializeHeapBitmap();
        DCHECK_CC(GetGCPhase() == GC_PHASE_IDLE);
        Heap::GetHeap().InstallBarrier(GC_PHASE_IDLE);
#ifdef PANDA_TARGET_32
        // cmc is not adapted for 32-bit systems
        gcMode_ = GCMode::STW;
#else
        if (param.gcParam.enableStwGC) {
            gcMode_ = GCMode::STW;
        } else {
            gcMode_ = GCMode::CMC;
        }
#endif

        // force STW for RB DFX
#ifdef ENABLE_CMC_RB_DFX
        gcMode_ = GCMode::STW;
#endif
    }

    void Fini() override
    {
        HeapBitmapManager::GetHeapBitmapManager().DestroyHeapBitmap();
    }

    bool ShouldIgnoreRequest(GCRequest& request) override;
    bool MarkObject(BaseObject* obj) const override;

    MarkingRefFieldVisitor CreateMarkingObjectRefFieldsVisitor(ParallelLocalMarkStack &workStack,
                                                               WeakStack &weakStack) override;
#ifdef PANDA_JS_ETS_HYBRID_MODE
    void MarkingXRef(RefField<> &ref, ParallelLocalMarkStack &workStack) const;
    void MarkingObjectXRef(BaseObject *obj, ParallelLocalMarkStack &workStack) override;
#endif

    void FixObjectRefFields(BaseObject* obj) const override;
    void FixRefField(BaseObject* obj, RefField<>& field) const;

    BaseObject* ForwardObject(BaseObject* fromVersion) override;

    bool IsOldPointer(RefField<>& ref) const override { return false; }

    bool IsCurrentPointer(RefField<>& ref) const override { return false; }

    void AddRawPointerObject(BaseObject* obj) override
    {
        RegionalHeap& space = reinterpret_cast<RegionalHeap&>(theAllocator_);
        space.AddRawPointerObject(obj);
    }

    void RemoveRawPointerObject(BaseObject* obj) override
    {
        RegionalHeap& space = reinterpret_cast<RegionalHeap&>(theAllocator_);
        space.RemoveRawPointerObject(obj);
    }

    BaseObject* ForwardUpdateRawRef(ObjectRef& ref);

    bool IsFromObject(BaseObject* obj) const override
    {
        // filter const string object.
        if (Heap::IsHeapAddress(obj)) {
            RegionDesc::InlinedRegionMetaData *objMetaRegion =
                RegionDesc::InlinedRegionMetaData::GetInlinedRegionMetaData(reinterpret_cast<uintptr_t>(obj));
            return objMetaRegion->IsFromRegion();
        }

        return false;
    }

    bool IsUnmovableFromObject(BaseObject* obj) const override;

    // this is called when caller assures from-object/from-region still exists.
    BaseObject* GetForwardingPointer(BaseObject* fromObj) { return fwdTable_.GetForwardingPointer(fromObj); }

    BaseObject* FindToVersion(BaseObject* obj) const override
    {
        return const_cast<ArkCollector*>(this)->fwdTable_.GetForwardingPointer(obj);
    }

    void SetGCThreadQosPriority(common::PriorityMode mode);

    BaseObject* CopyObjectImpl(BaseObject* obj);
    BaseObject* CopyObjectAfterExclusive(BaseObject* obj) override;

    BaseObject* TryForwardObject(BaseObject* fromVersion);

    bool TryUntagRefField(BaseObject* obj, RefField<>& field, BaseObject*& target) const override;
    bool TryUpdateRefField(BaseObject* obj, RefField<>& field, BaseObject*& newRef) const override;
    bool TryForwardRefField(BaseObject* obj, RefField<>& field, BaseObject*& newRef) const override;

    RefField<> GetAndTryTagRefField(BaseObject* target) const override
    {
        if (IsFromObject(target)) {
            return RefField<>(target);
        } else {
            return RefField<>(target);
        }
    }

protected:
    void CollectLargeGarbage()
    {
        COMMON_PHASE_TIMER("Collect large garbage");
        RegionalHeap& space = reinterpret_cast<RegionalHeap&>(theAllocator_);
        GCStats& stats = GetGCStats();
        stats.largeSpaceSize = space.LargeObjectSize();
        stats.largeGarbageSize = space.CollectLargeGarbage();
        stats.collectedBytes += stats.largeGarbageSize;
    }

    void CollectNonMovableGarbage()
    {
        RegionalHeap& space = reinterpret_cast<RegionalHeap&>(theAllocator_);
        GCStats& stats = GetGCStats();
        stats.nonMovableSpaceSize = space.NonMovableSpaceSize();
        stats.collectedBytes += stats.nonMovableGarbageSize;
    }

    void CollectSmallSpace();
    void ClearAllGCInfo();

    void DoGarbageCollection() override;
    void ProcessStringTable() override;

    void ProcessFinalizers() override;

private:
    friend class RemarkAndPreforwardVisitor;
    template<bool copy>
    bool TryUpdateRefFieldImpl(BaseObject* obj, RefField<>& ref, BaseObject*& oldRef, BaseObject*& newRef) const;

    enum class EnumRootsPolicy {
        NO_STW_AND_NO_FLIP_MUTATOR,
        STW_AND_NO_FLIP_MUTATOR,
        STW_AND_FLIP_MUTATOR,
    };

    template <EnumRootsPolicy policy>
    CArrayList<BaseObject *> EnumRoots();

    template <void (&rootsVisitFunc)(const common::RefFieldVisitor &)>
    void EnumRootsImpl(const common::RefFieldVisitor &visitor)
    {
        // assemble garbage candidates.
        reinterpret_cast<RegionalHeap &>(theAllocator_).AssembleGarbageCandidates();
        reinterpret_cast<RegionalHeap &>(theAllocator_).PrepareMarking();

        COMMON_PHASE_TIMER("enum roots & update old pointers within");
        TransitionToGCPhase(GCPhase::GC_PHASE_ENUM, true);

        rootsVisitFunc(visitor);
    }
    CArrayList<CArrayList<BaseObject *>> EnumRootsFlip(STWParam& param, const common::RefFieldVisitor &visitor);

    void MarkingHeap(const CArrayList<BaseObject *> &collectedRoots);
    void PostMarking();
    void RemarkAndPreforwardStaticRoots(GlobalMarkStack &globalMarkStack) override;
    void ParallelRemarkAndPreforward(GlobalMarkStack &globalMarkStack);
    void Preforward();
    void ConcurrentPreforward();

    void PreforwardConcurrentRoots();
    void PreforwardStaticWeakRoots();
    void PreforwardConcurrencyModelRoots();

    void PrepareFix();
    void ParallelFixHeap();
    void FixHeap(); // roots and ref-fields
    WeakRefFieldVisitor GetWeakRefFieldVisitor();
    RefFieldVisitor GetPrefowardRefFieldVisitor();
    void PreforwardFlip();

    void CollectGarbageWithXRef();

    CopyTable fwdTable_;

    GCMode gcMode_ = GCMode::CMC;
};
} // namespace common

#endif // COMMON_COMPONENTS_HEAP_ARK_COLLECTOR_ARKCOLLECTOR_H
