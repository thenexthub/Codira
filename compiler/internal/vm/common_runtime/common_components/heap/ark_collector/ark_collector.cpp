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
#include "common_components/heap/ark_collector/ark_collector.h"

#include "common_components/common_runtime/hooks.h"
#include "common_components/log/log.h"
#include "common_components/mutator/mutator_manager-inl.h"
#include "common_components/heap/verification.h"
#include "common_interfaces/heap/heap_visitor.h"
#include "common_interfaces/objects/ref_field.h"
#include "common_interfaces/profiler/heap_profiler_listener.h"
#include "common_components/heap/allocator/fix_heap.h"
#include "common_components/heap/allocator/regional_heap.h"

#ifdef ENABLE_QOS
#include "qos.h"
#endif

namespace common {
bool ArkCollector::IsUnmovableFromObject(BaseObject* obj) const
{
    // filter const string object.
    if (!Heap::IsHeapAddress(obj)) {
        return false;
    }

    RegionDesc* regionInfo = nullptr;
    regionInfo = RegionDesc::GetAliveRegionDescAt(reinterpret_cast<uintptr_t>(obj));
    return regionInfo->IsUnmovableFromRegion();
}

bool ArkCollector::MarkObject(BaseObject* obj) const
{
    bool marked = RegionalHeap::MarkObject(obj);
    if (!marked) {
        RegionDesc* region = RegionDesc::GetAliveRegionDescAt(reinterpret_cast<HeapAddress>(obj));
        DCHECK_CC(!region->IsGarbageRegion());
        DLOG(TRACE, "mark obj %p<%p> in region %p(%u)@%#zx, live %u", obj, obj->GetTypeInfo(),
             region, region->GetRegionType(), region->GetRegionStart(), region->GetLiveByteCount());
    }
    return marked;
}

// this api updates current pointer as well as old pointer, caller should take care of this.
template<bool copy>
bool ArkCollector::TryUpdateRefFieldImpl(BaseObject* obj, RefField<>& field, BaseObject*& fromObj,
                                         BaseObject*& toObj) const
{
    RefField<> oldRef(field);
    fromObj = oldRef.GetTargetObject();
    if (IsFromObject(fromObj)) { //LCOV_EXCL_BR_LINE
        if (copy) { //LCOV_EXCL_BR_LINE
            toObj = const_cast<ArkCollector*>(this)->TryForwardObject(fromObj);
            if (toObj != nullptr) { //LCOV_EXCL_BR_LINE
                HeapProfilerListener::GetInstance().OnMoveEvent(reinterpret_cast<uintptr_t>(fromObj),
                                                                reinterpret_cast<uintptr_t>(toObj),
                                                                toObj->GetSize());
            }
        } else { //LCOV_EXCL_BR_LINE
            toObj = FindToVersion(fromObj);
        }
        if (toObj == nullptr) { //LCOV_EXCL_BR_LINE
            return false;
        }
        RefField<> tmpField(toObj, oldRef.IsWeak());
        if (field.CompareExchange(oldRef.GetFieldValue(), tmpField.GetFieldValue())) { //LCOV_EXCL_BR_LINE
            if (obj != nullptr) { //LCOV_EXCL_BR_LINE
                DLOG(TRACE, "update obj %p<%p>(%zu)+%zu ref-field@%p: %#zx -> %#zx", obj, obj->GetTypeInfo(),
                    obj->GetSize(), BaseObject::FieldOffset(obj, &field), &field, oldRef.GetFieldValue(),
                    tmpField.GetFieldValue());
            } else { //LCOV_EXCL_BR_LINE
                DLOG(TRACE, "update ref@%p: 0x%zx -> %p", &field, oldRef.GetFieldValue(), toObj);
            }
            return true;
        } else { //LCOV_EXCL_BR_LINE
            if (obj != nullptr) { //LCOV_EXCL_BR_LINE
                DLOG(TRACE,
                    "update obj %p<%p>(%zu)+%zu but cas failed ref-field@%p: %#zx(%#zx) -> %#zx but cas failed ",
                     obj, obj->GetTypeInfo(), obj->GetSize(), BaseObject::FieldOffset(obj, &field), &field,
                     oldRef.GetFieldValue(), field.GetFieldValue(), tmpField.GetFieldValue());
            } else { //LCOV_EXCL_BR_LINE
                DLOG(TRACE, "update but cas failed ref@%p: 0x%zx(%zx) -> %p", &field, oldRef.GetFieldValue(),
                     field.GetFieldValue(), toObj);
            }
            return true;
        }
    }

    return false;
}

bool ArkCollector::TryUpdateRefField(BaseObject* obj, RefField<>& field, BaseObject*& newRef) const
{
    BaseObject* oldRef = nullptr;
    return TryUpdateRefFieldImpl<false>(obj, field, oldRef, newRef);
}

bool ArkCollector::TryForwardRefField(BaseObject* obj, RefField<>& field, BaseObject*& newRef) const
{
    BaseObject* oldRef = nullptr;
    return TryUpdateRefFieldImpl<true>(obj, field, oldRef, newRef);
}

// this api untags current pointer as well as old pointer, caller should take care of this.
bool ArkCollector::TryUntagRefField(BaseObject* obj, RefField<>& field, BaseObject*& target) const
{
    for (;;) { //LCOV_EXCL_BR_LINE
        RefField<> oldRef(field);
        if (oldRef.IsTagged()) { //LCOV_EXCL_BR_LINE
            target = oldRef.GetTargetObject();
            RefField<> newRef(target);
            if (field.CompareExchange(oldRef.GetFieldValue(), newRef.GetFieldValue())) { //LCOV_EXCL_BR_LINE
                if (obj != nullptr) { //LCOV_EXCL_BR_LINE
                    DLOG(FIX, "untag obj %p<%p>(%zu) ref-field@%p: %#zx -> %#zx", obj, obj->GetTypeInfo(),
                         obj->GetSize(), &field, oldRef.GetFieldValue(), newRef.GetFieldValue());
                } else { //LCOV_EXCL_BR_LINE
                    DLOG(FIX, "untag ref@%p: %#zx -> %#zx", &field, oldRef.GetFieldValue(), newRef.GetFieldValue());
                }

                return true;
            }
        } else { //LCOV_EXCL_BR_LINE
            return false;
        }
    }

    return false;
}

static void MarkingRefField(BaseObject *obj, BaseObject *targetObj, RefField<> &field,
                            ParallelLocalMarkStack &markStack, RegionDesc *targetRegion);
// note each ref-field will not be marked twice, so each old pointer the markingr meets must come from previous gc.
static void MarkingRefField(BaseObject *obj, RefField<> &field, ParallelLocalMarkStack &markStack,
                            WeakStack &weakStack, const GCReason gcReason)
{
    RefField<> oldField(field);
    BaseObject* targetObj = oldField.GetTargetObject();

    if (!Heap::IsTaggedObject(oldField.GetFieldValue())) {
        return;
    }
    // field is tagged object, should be in heap
    DCHECK_CC(Heap::IsHeapAddress(targetObj));

    auto targetRegion = RegionDesc::GetAliveRegionDescAt(reinterpret_cast<MAddress>((void*)targetObj));
    if (gcReason != GC_REASON_YOUNG && oldField.IsWeak()) {
        DLOG(TRACE, "marking: skip weak obj when full gc, object: %p@%p, targetObj: %p", obj, &field, targetObj);
        // weak ref is cleared after roots pre-forward, so there might be a to-version weak ref which also need to be
        // cleared, offset recorded here will help us find it
        weakStack.emplace_back(&field, reinterpret_cast<uintptr_t>(&field) - reinterpret_cast<uintptr_t>(obj));
        return;
    }

    // cannot skip objects in EXEMPTED_FROM_REGION, because its rset is incomplete
    if (gcReason == GC_REASON_YOUNG && !targetRegion->IsInYoungSpace()) {
        DLOG(TRACE, "marking: skip non-young object %p@%p, target object: %p<%p>(%zu)",
            obj, &field, targetObj, targetObj->GetTypeInfo(), targetObj->GetSize());
        return;
    }
    common::MarkingRefField(obj, targetObj, field, markStack, targetRegion);
}

// note each ref-field will not be marked twice, so each old pointer the markingr meets must come from previous gc.
static void MarkingRefField(BaseObject *obj, BaseObject *targetObj, RefField<> &field,
                            ParallelLocalMarkStack &markStack, RegionDesc *targetRegion)
{
    if (targetRegion->IsNewObjectSinceMarking(targetObj)) {
        DLOG(TRACE, "marking: skip new obj %p<%p>(%zu)", targetObj, targetObj->GetTypeInfo(), targetObj->GetSize());
        return;
    }

    if (targetRegion->MarkObject(targetObj)) {
        DLOG(TRACE, "marking: obj has been marked %p", targetObj);
        return;
    }

    DLOG(TRACE, "marking obj %p ref@%p: %p<%p>(%zu)",
        obj, &field, targetObj, targetObj->GetTypeInfo(), targetObj->GetSize());
    markStack.Push(targetObj);
}

MarkingCollector::MarkingRefFieldVisitor ArkCollector::CreateMarkingObjectRefFieldsVisitor(
    ParallelLocalMarkStack &markStack, WeakStack &weakStack)
{
    MarkingRefFieldVisitor visitor;

    if (gcReason_ == GCReason::GC_REASON_YOUNG) {
        visitor.SetVisitor([obj = visitor.GetClosure(), &markStack, &weakStack](RefField<> &field) {
            const GCReason gcReason = GCReason::GC_REASON_YOUNG;
            MarkingRefField(*obj, field, markStack, weakStack, gcReason);
        });
    } else {
        visitor.SetVisitor([obj = visitor.GetClosure(), &markStack, &weakStack](RefField<> &field) {
            const GCReason gcReason = GCReason::GC_REASON_HEU;
            MarkingRefField(*obj, field, markStack, weakStack, gcReason);
        });
    }
    return visitor;
}

#ifdef PANDA_JS_ETS_HYBRID_MODE
// note each ref-field will not be traced twice, so each old pointer the tracer meets must come from previous gc.
void ArkCollector::MarkingXRef(RefField<> &field, ParallelLocalMarkStack &workStack) const
{
    BaseObject* targetObj = field.GetTargetObject();
    auto region = RegionDesc::GetRegionDescAt(reinterpret_cast<MAddress>(targetObj));
    // field is tagged object, should be in heap
    DCHECK_CC(Heap::IsHeapAddress(targetObj));

    DLOG(TRACE, "trace obj %p <%p>(%zu)", targetObj, targetObj->GetTypeInfo(), targetObj->GetSize());
    if (region->IsNewObjectSinceForward(targetObj)) {
        DLOG(TRACE, "trace: skip new obj %p<%p>(%zu)", targetObj, targetObj->GetTypeInfo(), targetObj->GetSize());
        return;
    }
    DCHECK_CC(!field.IsWeak());
    if (!region->MarkObject(targetObj)) {
        workStack.Push(targetObj);
    }
}

void ArkCollector::MarkingObjectXRef(BaseObject *obj, ParallelLocalMarkStack &workStack)
{
    auto refFunc = [this, &workStack] (RefField<>& field) {
        MarkingXRef(field, workStack);
    };

    obj->IterateXRef(refFunc);
}
#endif

void ArkCollector::FixRefField(BaseObject* obj, RefField<>& field) const
{
    RefField<> oldField(field);
    BaseObject* targetObj = oldField.GetTargetObject();
    if (!Heap::IsTaggedObject(oldField.GetFieldValue())) {
        return;
    }
    // target object could be null or non-heap for some static variable.
    if (!Heap::IsHeapAddress(targetObj)) {
        return;
    }

    RegionDesc::InlinedRegionMetaData *refRegion =  RegionDesc::InlinedRegionMetaData::GetInlinedRegionMetaData(
        reinterpret_cast<uintptr_t>(targetObj));
    bool isFrom = refRegion->IsFromRegion();
    bool isInRcent = refRegion->IsInRecentSpace();
    if (isInRcent) {
        RegionDesc::InlinedRegionMetaData *objRegion =  RegionDesc::InlinedRegionMetaData::GetInlinedRegionMetaData(
            reinterpret_cast<uintptr_t>(obj));
        if (!objRegion->IsInRecentSpace() &&
            objRegion->MarkRSetCardTable(obj)) {
            DLOG(TRACE,
                 "fix phase update point-out remember set of region %p, obj "
                 "%p, ref: <%p>",
                 objRegion, obj, targetObj->GetTypeInfo());
        }
        return;
    } else if (!isFrom) {
        return;
    }
    BaseObject* latest = FindToVersion(targetObj);

    if (latest == nullptr) { return; }

    CHECK_CC(latest->IsValidObject());
    RefField<> newField(latest, oldField.IsWeak());
    if (field.CompareExchange(oldField.GetFieldValue(), newField.GetFieldValue())) {
        DLOG(FIX, "fix obj %p+%zu ref@%p: %#zx => %p<%p>(%zu)", obj, obj->GetSize(), &field,
             oldField.GetFieldValue(), latest, latest->GetTypeInfo(), latest->GetSize());
    }
}

void ArkCollector::FixObjectRefFields(BaseObject* obj) const
{
    DLOG(FIX, "fix obj %p<%p>(%zu)", obj, obj->GetTypeInfo(), obj->GetSize());
    auto refFunc = [this, obj](RefField<>& field) { FixRefField(obj, field); };
    obj->ForEachRefField(refFunc);
}

BaseObject* ArkCollector::ForwardUpdateRawRef(ObjectRef& root)
{
    auto& refField = reinterpret_cast<RefField<>&>(root);
    RefField<> oldField(refField);
    BaseObject* oldObj = oldField.GetTargetObject();
    DLOG(FIX, "try fix raw-ref @%p: %p", &root, oldObj);
    if (IsFromObject(oldObj)) {
        BaseObject* toVersion = TryForwardObject(oldObj);
        CHECK_CC(toVersion != nullptr);
        HeapProfilerListener::GetInstance().OnMoveEvent(reinterpret_cast<uintptr_t>(oldObj),
                                                        reinterpret_cast<uintptr_t>(toVersion),
                                                        toVersion->GetSize());
        RefField<> newField(toVersion);
        // CAS failure means some mutator or gc thread writes a new ref (must be a to-object), no need to retry.
        if (refField.CompareExchange(oldField.GetFieldValue(), newField.GetFieldValue())) {
            DLOG(FIX, "fix raw-ref @%p: %p -> %p", &root, oldObj, toVersion);
            return toVersion;
        }
    }

    return oldObj;
}

class RemarkAndPreforwardVisitor {
public:
    RemarkAndPreforwardVisitor(LocalCollectStack &collectStack, ArkCollector *collector)
        : collectStack_(collectStack), collector_(collector) {}

    void operator()(RefField<> &refField)
    {
        RefField<> oldField(refField);
        BaseObject* oldObj = oldField.GetTargetObject();
        DLOG(FIX, "visit raw-ref @%p: %p", &refField, oldObj);

        auto regionType =
            RegionDesc::InlinedRegionMetaData::GetInlinedRegionMetaData(reinterpret_cast<uintptr_t>(oldObj))
                ->GetRegionType();
        if (regionType == RegionDesc::RegionType::FROM_REGION) {
            BaseObject* toVersion = collector_->TryForwardObject(oldObj);
            if (toVersion == nullptr) { //LCOV_EXCL_BR_LINE
                Heap::throwOOM();
                return;
            }
            HeapProfilerListener::GetInstance().OnMoveEvent(reinterpret_cast<uintptr_t>(oldObj),
                                                            reinterpret_cast<uintptr_t>(toVersion),
                                                            toVersion->GetSize());
            RefField<> newField(toVersion);
            // CAS failure means some mutator or gc thread writes a new ref (must be a to-object), no need to retry.
            if (refField.CompareExchange(oldField.GetFieldValue(), newField.GetFieldValue())) {
                DLOG(FIX, "fix raw-ref @%p: %p -> %p", &refField, oldObj, toVersion);
            }
            MarkToObject(oldObj, toVersion);
        } else {
            if (Heap::GetHeap().GetGCReason() != GC_REASON_YOUNG) {
                MarkObject(oldObj);
            } else if (RegionalHeap::IsYoungSpaceObject(oldObj) && !RegionalHeap::IsNewObjectSinceMarking(oldObj) &&
                       !RegionalHeap::IsMarkedObject(oldObj)) {
                // RSet don't protect exempted objects, we need to mark it
                MarkObject(oldObj);
            }
        }
    }

private:
    void MarkObject(BaseObject *object)
    {
        if (!RegionalHeap::IsNewObjectSinceMarking(object) && !collector_->MarkObject(object)) {
            collectStack_.Push(object);
        }
    }

    void MarkToObject(BaseObject *oldVersion, BaseObject *toVersion)
    {
        // We've checked oldVersion is in fromSpace, no need to check markingLine
        if (!collector_->MarkObject(oldVersion)) {
            // No need to count oldVersion object size, as it has been copied.
            collector_->MarkObject(toVersion);
            // oldVersion don't have valid type info, cannot push it
            collectStack_.Push(toVersion);
        }
    }

private:
    LocalCollectStack &collectStack_;
    ArkCollector *collector_;
};

class RemarkingAndPreforwardTask : public common::Task {
public:
    RemarkingAndPreforwardTask(ArkCollector *collector, GlobalMarkStack &globalMarkStack, TaskPackMonitor &monitor,
                               std::function<Mutator*()>& next)
        : Task(0), collector_(collector), globalMarkStack_(globalMarkStack), monitor_(monitor), getNextMutator_(next)
    {}

    bool Run([[maybe_unused]] uint32_t threadIndex) override
    {
        ThreadLocal::SetThreadType(ThreadType::GC_THREAD);
        LocalCollectStack collectStack(&globalMarkStack_);
        RemarkAndPreforwardVisitor visitor(collectStack, collector_);
        Mutator *mutator = getNextMutator_();
        while (mutator != nullptr) {
            VisitMutatorRoot(visitor, *mutator);
            mutator = getNextMutator_();
        }
        collectStack.Publish();
        ThreadLocal::SetThreadType(ThreadType::ARK_PROCESSOR);
        ThreadLocal::ClearAllocBufferRegion();
        monitor_.NotifyFinishOne();
        return true;
    }

private:
    ArkCollector *collector_ {nullptr};
    GlobalMarkStack &globalMarkStack_;
    TaskPackMonitor &monitor_;
    std::function<Mutator*()> &getNextMutator_;
};

void ArkCollector::ParallelRemarkAndPreforward(GlobalMarkStack &globalMarkStack)
{
    std::vector<Mutator*> taskList;
    MutatorManager &mutatorManager = MutatorManager::Instance();
    mutatorManager.VisitAllMutators([&taskList](Mutator &mutator) {
        taskList.push_back(&mutator);
    });
    std::atomic<int> taskIter = 0;
    std::function<Mutator*()> getNextMutator = [&taskIter, &taskList]() -> Mutator* {
        uint32_t idx = static_cast<uint32_t>(taskIter.fetch_add(1U, std::memory_order_relaxed));
        if (idx < taskList.size()) {
            return taskList[idx];
        }
        return nullptr;
    };

    const uint32_t runningWorkers = std::min<uint32_t>(GetGCThreadCount(true), taskList.size());
    uint32_t parallelCount = runningWorkers + 1; // 1 ：DaemonThread
    TaskPackMonitor monitor(runningWorkers, runningWorkers);
    for (uint32_t i = 1; i < parallelCount; ++i) {
        GetThreadPool()->PostTask(std::make_unique<RemarkingAndPreforwardTask>(this, globalMarkStack, monitor,
                                                                               getNextMutator));
    }
    // Run in daemon thread.
    LocalCollectStack collectStack(&globalMarkStack);
    RemarkAndPreforwardVisitor visitor(collectStack, this);
    VisitGlobalRoots(visitor);
    Mutator *mutator = getNextMutator();
    while (mutator != nullptr) {
        VisitMutatorRoot(visitor, *mutator);
        mutator = getNextMutator();
    }
    collectStack.Publish();
    monitor.WaitAllFinished();
}

void ArkCollector::RemarkAndPreforwardStaticRoots(GlobalMarkStack &globalMarkStack)
{
    OHOS_HITRACE(HITRACE_LEVEL_COMMERCIAL, "CMCGC::RemarkAndPreforwardStaticRoots", "");
    const uint32_t maxWorkers = GetGCThreadCount(true) - 1;
    if (maxWorkers > 0) {
        ParallelRemarkAndPreforward(globalMarkStack);
    } else {
        LocalCollectStack collectStack(&globalMarkStack);
        RemarkAndPreforwardVisitor visitor(collectStack, this);
        VisitSTWRoots(visitor);
        collectStack.Publish();
    }
}

void ArkCollector::PreforwardConcurrentRoots()
{
    RefFieldVisitor visitor = [this](RefField<> &refField) {
        RefField<> oldField(refField);
        BaseObject *oldObj = oldField.GetTargetObject();
        DLOG(FIX, "visit raw-ref @%p: %p", &refField, oldObj);
        if (IsFromObject(oldObj)) {
            BaseObject *toVersion = TryForwardObject(oldObj);
            ASSERT_LOGF(toVersion != nullptr, "TryForwardObject failed");
            HeapProfilerListener::GetInstance().OnMoveEvent(reinterpret_cast<uintptr_t>(oldObj),
                                                            reinterpret_cast<uintptr_t>(toVersion),
                                                            toVersion->GetSize());
            RefField<> newField(toVersion);
            // CAS failure means some mutator or gc thread writes a new ref (must be a to-object), no need to retry.
            if (refField.CompareExchange(oldField.GetFieldValue(), newField.GetFieldValue())) {
                DLOG(FIX, "fix raw-ref @%p: %p -> %p", &refField, oldObj, toVersion);
            }
        }
    };
    VisitConcurrentRoots(visitor);
}

void ArkCollector::PreforwardStaticWeakRoots()
{
    OHOS_HITRACE(HITRACE_LEVEL_COMMERCIAL, "CMCGC::PreforwardStaticRoots", "");

    WeakRefFieldVisitor weakVisitor = GetWeakRefFieldVisitor();
    VisitWeakRoots(weakVisitor);
    InvokeSharedNativePointerCallbacks();
    MutatorManager::Instance().VisitAllMutators([](Mutator& mutator) {
        // Request finalize callback in each vm-thread when gc finished.
        mutator.SetFinalizeRequest();
    });

    AllocationBuffer* allocBuffer = AllocationBuffer::GetAllocBuffer();
    if (LIKELY_CC(allocBuffer != nullptr)) {
        allocBuffer->ClearRegions();
    }
}

void ArkCollector::PreforwardConcurrencyModelRoots()
{
    LOG_COMMON(FATAL) << "Unresolved fatal";
    UNREACHABLE_CC();
}

class EnumRootsBuffer {
public:
    EnumRootsBuffer();
    void UpdateBufferSize();
    CArrayList<BaseObject *> *GetBuffer() { return &buffer_; }

private:
    static size_t bufferSize_;
    CArrayList<BaseObject *> buffer_;
};

size_t EnumRootsBuffer::bufferSize_ = 16;
EnumRootsBuffer::EnumRootsBuffer() : buffer_(bufferSize_)
{
    buffer_.clear();  // memset to zero and allocated real memory
}

void EnumRootsBuffer::UpdateBufferSize()
{
    if (buffer_.empty()) {
        return;
    }
    const size_t decreaseBufferThreshold = bufferSize_ >> 2;
    if (buffer_.size() < decreaseBufferThreshold) {
        bufferSize_ = bufferSize_ >> 1;
    } else {
        bufferSize_ = std::max(buffer_.capacity(), bufferSize_);
    }
    if (buffer_.capacity() > UINT16_MAX) {
        LOG_COMMON(INFO) << "too many roots, allocated buffer too large: " << buffer_.size() << ", allocate "
                         << (static_cast<double>(buffer_.capacity()) / MB);
    }
}

template <ArkCollector::EnumRootsPolicy policy>
CArrayList<BaseObject *> ArkCollector::EnumRoots()
{
    STWParam stwParam{"wgc-enumroot"};
    EnumRootsBuffer buffer;
    CArrayList<common::BaseObject *> *results = buffer.GetBuffer();
    common::RefFieldVisitor visitor = [&results](RefField<>& field) { results->push_back(field.GetTargetObject()); };

    if constexpr (policy == EnumRootsPolicy::NO_STW_AND_NO_FLIP_MUTATOR) {
        EnumRootsImpl<VisitRoots>(visitor);
    } else if constexpr (policy == EnumRootsPolicy::STW_AND_NO_FLIP_MUTATOR) {
        ScopedStopTheWorld stw(stwParam);
        OHOS_HITRACE(HITRACE_LEVEL_COMMERCIAL,
                     ("CMCGC::EnumRoots-STW-bufferSize(" + std::to_string(results->capacity()) + ")").c_str(), "");
        EnumRootsImpl<VisitRoots>(visitor);
    } else if constexpr (policy == EnumRootsPolicy::STW_AND_FLIP_MUTATOR) {
        auto rootSet = EnumRootsFlip(stwParam, visitor);
        for (const auto &roots : rootSet) {
            std::copy(roots.begin(), roots.end(), std::back_inserter(*results));
        }
        VisitConcurrentRoots(visitor);
    }
    buffer.UpdateBufferSize();
    GetGCStats().recordSTWTime(stwParam.GetElapsedNs());
    return std::move(*results);
}

void ArkCollector::MarkingHeap(const CArrayList<BaseObject *> &collectedRoots)
{
    COMMON_PHASE_TIMER("marking live objects");
    markedObjectCount_.store(0, std::memory_order_relaxed);
    TransitionToGCPhase(GCPhase::GC_PHASE_MARK, true);

    MarkingRoots(collectedRoots);
    ProcessFinalizers();
    ExemptFromSpace();
}

void ArkCollector::PostMarking()
{
    OHOS_HITRACE(HITRACE_LEVEL_COMMERCIAL, "CMCGC::PostMarking", "");
    COMMON_PHASE_TIMER("PostMarking");
    TransitionToGCPhase(GC_PHASE_POST_MARK, true);

    // clear satb buffer when gc finish tracing.
    SatbBuffer::Instance().ClearBuffer();

    WVerify::VerifyAfterMark(*this);
}

WeakRefFieldVisitor ArkCollector::GetWeakRefFieldVisitor()
{
    return [this](RefField<> &refField) -> bool {
        RefField<> oldField(refField);
        BaseObject *oldObj = oldField.GetTargetObject();
        if (gcReason_ == GC_REASON_YOUNG) {
            if (RegionalHeap::IsYoungSpaceObject(oldObj) && !IsMarkedObject(oldObj) &&
                !RegionalHeap::IsNewObjectSinceMarking(oldObj)) {
                return false;
            }
        } else {
            if (!IsMarkedObject(oldObj) && !RegionalHeap::IsNewObjectSinceMarking(oldObj)) {
                return false;
            }
        }

        DLOG(FIX, "visit weak raw-ref @%p: %p", &refField, oldObj);
        if (IsFromObject(oldObj)) {
            BaseObject *toVersion = TryForwardObject(oldObj);
            CHECK_CC(toVersion != nullptr);
            HeapProfilerListener::GetInstance().OnMoveEvent(reinterpret_cast<uintptr_t>(oldObj),
                                                            reinterpret_cast<uintptr_t>(toVersion),
                                                            toVersion->GetSize());
            RefField<> newField(toVersion);
            // CAS failure means some mutator or gc thread writes a new ref (must be
            // a to-object), no need to retry.
            if (refField.CompareExchange(oldField.GetFieldValue(), newField.GetFieldValue())) {
                DLOG(FIX, "fix weak raw-ref @%p: %p -> %p", &refField, oldObj, toVersion);
            }
        }
        return true;
    };
}

RefFieldVisitor ArkCollector::GetPrefowardRefFieldVisitor()
{
    return [this](RefField<> &refField) -> void {
        RefField<> oldField(refField);
        BaseObject *oldObj = oldField.GetTargetObject();
        if (IsFromObject(oldObj)) {
            BaseObject *toVersion = TryForwardObject(oldObj);
            CHECK_CC(toVersion != nullptr);
            HeapProfilerListener::GetInstance().OnMoveEvent(reinterpret_cast<uintptr_t>(oldObj),
                                                            reinterpret_cast<uintptr_t>(toVersion),
                                                            toVersion->GetSize());
            RefField<> newField(toVersion);
            // CAS failure means some mutator or gc thread writes a new ref (must be
            // a to-object), no need to retry.
            if (refField.CompareExchange(oldField.GetFieldValue(), newField.GetFieldValue())) {
                DLOG(FIX, "fix raw-ref @%p: %p -> %p", &refField, oldObj, toVersion);
            }
        }
    };
}

void ArkCollector::PreforwardFlip()
{
    auto remarkAndForwardGlobalRoot = [this]() {
        OHOS_HITRACE(HITRACE_LEVEL_COMMERCIAL, "CMCGC::PreforwardFlip[STW]", "");
        SetGCThreadQosPriority(common::PriorityMode::STW);
        ASSERT_LOGF(GetThreadPool() != nullptr, "thread pool is null");
        TransitionToGCPhase(GCPhase::GC_PHASE_FINAL_MARK, true);
        Remark();
        PostMarking();
        reinterpret_cast<RegionalHeap&>(theAllocator_).PrepareForward();

        TransitionToGCPhase(GCPhase::GC_PHASE_PRECOPY, true);
        WeakRefFieldVisitor weakVisitor = GetWeakRefFieldVisitor();
        SetGCThreadQosPriority(common::PriorityMode::FOREGROUND);

        if (Heap::GetHeap().GetGCReason() == GC_REASON_YOUNG) {
            // only visit weak roots that may reference young objects
            VisitDynamicWeakGlobalRoots(weakVisitor);
        } else {
            VisitDynamicWeakGlobalRoots(weakVisitor);
            VisitDynamicWeakGlobalRootsOld(weakVisitor);
        }
    };
    FlipFunction forwardMutatorRoot = [this](Mutator &mutator) {
        WeakRefFieldVisitor weakVisitor = GetWeakRefFieldVisitor();
        VisitWeakMutatorRoot(weakVisitor, mutator);
        RefFieldVisitor visitor = GetPrefowardRefFieldVisitor();
        VisitMutatorPreforwardRoot(visitor, mutator);
        // Request finalize callback in each vm-thread when gc finished.
        mutator.SetFinalizeRequest();
    };
    STWParam stwParam{"final-mark"};
    MutatorManager::Instance().FlipMutators(stwParam, remarkAndForwardGlobalRoot, &forwardMutatorRoot);
    InvokeSharedNativePointerCallbacks();
    GetGCStats().recordSTWTime(stwParam.GetElapsedNs());
    AllocationBuffer* allocBuffer = AllocationBuffer::GetAllocBuffer();
    if (LIKELY_CC(allocBuffer != nullptr)) {
        allocBuffer->ClearRegions();
    }
    if (gcReason_ == GC_REASON_YOUNG || globalWeakStack_.empty()) {
        return;
    }
    globalWeakStack_.clear();
}

void ArkCollector::Preforward()
{
    COMMON_PHASE_TIMER("Preforward");
    OHOS_HITRACE(HITRACE_LEVEL_COMMERCIAL, "CMCGC::Preforward[STW]", "");
    TransitionToGCPhase(GCPhase::GC_PHASE_PRECOPY, true);

    [[maybe_unused]] Taskpool *threadPool = GetThreadPool();
    ASSERT_LOGF(threadPool != nullptr, "thread pool is null");

    // copy and fix finalizer roots.
    // Only one root task, no need to post task.
    PreforwardStaticWeakRoots();
    RefFieldVisitor visitor = GetPrefowardRefFieldVisitor();
    VisitPreforwardRoots(visitor);
}

void ArkCollector::ConcurrentPreforward()
{
    OHOS_HITRACE(HITRACE_LEVEL_COMMERCIAL, "CMCGC::ConcurrentPreforward", "");
    PreforwardConcurrentRoots();
    ProcessStringTable();
}

void ArkCollector::PrepareFix()
{
    if (Heap::GetHeap().GetGCReason() == GCReason::GC_REASON_YOUNG) {
        // string table objects are always not in young space, skip it
        return;
    }

    COMMON_PHASE_TIMER("PrepareFix");

    // we cannot re-enter STW, check it first
    if (!MutatorManager::Instance().WorldStopped()) {
        STWParam prepareFixStwParam{"wgc-preparefix"};
        ScopedStopTheWorld stw(prepareFixStwParam);
        OHOS_HITRACE(HITRACE_LEVEL_COMMERCIAL, "CMCGC::PrepareFix[STW]", "");

#ifndef GC_STW_STRINGTABLE
        BaseRuntime::GetInstance()->StringTableCleanUp();
#endif

        GetGCStats().recordSTWTime(prepareFixStwParam.GetElapsedNs());
    } else {
#ifndef GC_STW_STRINGTABLE
        BaseRuntime::GetInstance()->StringTableCleanUp();
#endif
    }
}

void ArkCollector::ParallelFixHeap()
{
    auto& regionalHeap = reinterpret_cast<RegionalHeap&>(theAllocator_);
    auto taskList = regionalHeap.CollectFixTasks();
    std::atomic<int> taskIter = 0;
    std::function<FixHeapTask *()> getNextTask = [&taskIter, &taskList]() -> FixHeapTask* {
        uint32_t idx = static_cast<uint32_t>(taskIter.fetch_add(1U, std::memory_order_relaxed));
        if (idx < taskList.size()) {
            return &taskList[idx];
        }
        return nullptr;
    };

    const uint32_t runningWorkers = GetGCThreadCount(true) - 1;
    uint32_t parallelCount = runningWorkers + 1; // 1 ：DaemonThread
    FixHeapWorker::Result results[parallelCount];
    {
        OHOS_HITRACE(HITRACE_LEVEL_COMMERCIAL, "CMCGC::FixHeap [Parallel]", "");
        // Fix heap
        TaskPackMonitor monitor(runningWorkers, runningWorkers);
        for (uint32_t i = 1; i < parallelCount; ++i) {
            GetThreadPool()->PostTask(std::make_unique<FixHeapWorker>(this, monitor, results[i], getNextTask));
        }

        FixHeapWorker gcWorker(this, monitor, results[0], getNextTask);
        auto task = getNextTask();
        while (task != nullptr) {
            gcWorker.DispatchRegionFixTask(task);
            task = getNextTask();
        }
        monitor.WaitAllFinished();
    }

    {
        OHOS_HITRACE(HITRACE_LEVEL_COMMERCIAL, "CMCGC::Post FixHeap Clear [Parallel]", "");
        // Post clear task
        TaskPackMonitor monitor(runningWorkers, runningWorkers);
        for (uint32_t i = 1; i < parallelCount; ++i) {
            GetThreadPool()->PostTask(std::make_unique<PostFixHeapWorker>(results[i], monitor));
        }

        PostFixHeapWorker gcWorker(results[0], monitor);
        gcWorker.PostClearTask();
        PostFixHeapWorker::CollectEmptyRegions();
        monitor.WaitAllFinished();
    }
}

void ArkCollector::FixHeap()
{
    TransitionToGCPhase(GCPhase::GC_PHASE_FIX, true);
    COMMON_PHASE_TIMER("FixHeap");
    OHOS_HITRACE(HITRACE_LEVEL_COMMERCIAL, "CMCGC::FixHeap", "");
    ParallelFixHeap();

    WVerify::VerifyAfterFix(*this);
}

void ArkCollector::CollectGarbageWithXRef()
{
    const bool isNotYoungGC = gcReason_ != GCReason::GC_REASON_YOUNG;
#ifdef ENABLE_CMC_RB_DFX
    WVerify::DisableReadBarrierDFX(*this);
#endif
    STWParam stwParam{"stw-gc"};
    ScopedStopTheWorld stw(stwParam);
    RemoveXRefFromRoots();

    auto collectedRoots = EnumRoots<EnumRootsPolicy::NO_STW_AND_NO_FLIP_MUTATOR>();
    MarkingHeap(collectedRoots);
    TransitionToGCPhase(GCPhase::GC_PHASE_FINAL_MARK, true);
    Remark();
    SweepUnmarkedXRefs();
    PostMarking();

    AddXRefToRoots();
    Preforward();
    ConcurrentPreforward();
    // reclaim large objects should after preforward(may process weak ref) and
    // before fix heap(may clear live bit)
    if (isNotYoungGC) {
        CollectLargeGarbage();
    }
    SweepThreadLocalJitFort();

    CopyFromSpace();
    WVerify::VerifyAfterForward(*this);

    PrepareFix();
    FixHeap();
    if (isNotYoungGC) {
        CollectNonMovableGarbage();
    }

    TransitionToGCPhase(GCPhase::GC_PHASE_IDLE, true);

    ClearAllGCInfo();
    CollectSmallSpace();
    UnmarkAllXRefs();

#if defined(ENABLE_CMC_RB_DFX)
    WVerify::EnableReadBarrierDFX(*this);
#endif
    GetGCStats().recordSTWTime(stwParam.GetElapsedNs());
}

void ArkCollector::DoGarbageCollection()
{
    const bool isNotYoungGC = gcReason_ != GCReason::GC_REASON_YOUNG;
    OHOS_HITRACE(HITRACE_LEVEL_COMMERCIAL, "CMCGC::DoGarbageCollection", "");
    if (gcReason_ == GCReason::GC_REASON_XREF) {
        CollectGarbageWithXRef();
        return;
    }
    if (gcMode_ == GCMode::STW) { // 2: stw-gc
#ifdef ENABLE_CMC_RB_DFX
        WVerify::DisableReadBarrierDFX(*this);
#endif
        STWParam stwParam{"stw-gc"};
    {
        ScopedStopTheWorld stw(stwParam);
        auto collectedRoots = EnumRoots<EnumRootsPolicy::NO_STW_AND_NO_FLIP_MUTATOR>();
        MarkingHeap(collectedRoots);
        TransitionToGCPhase(GCPhase::GC_PHASE_FINAL_MARK, true);
        Remark();
        PostMarking();

        Preforward();
        ConcurrentPreforward();
        // reclaim large objects should after preforward(may process weak ref) and
        // before fix heap(may clear live bit)
        if (isNotYoungGC) {
            CollectLargeGarbage();
        }
        SweepThreadLocalJitFort();

        CopyFromSpace();
        WVerify::VerifyAfterForward(*this);

        PrepareFix();
        FixHeap();
        if (isNotYoungGC) {
            CollectNonMovableGarbage();
        }

        TransitionToGCPhase(GCPhase::GC_PHASE_IDLE, true);

        ClearAllGCInfo();
        CollectSmallSpace();

#if defined(ENABLE_CMC_RB_DFX)
        WVerify::EnableReadBarrierDFX(*this);
#endif
    }
        GetGCStats().recordSTWTime(stwParam.GetElapsedNs());
        return;
    } else if (gcMode_ == GCMode::CONCURRENT_MARK) { // 1: concurrent-mark
        auto collectedRoots = EnumRoots<EnumRootsPolicy::STW_AND_NO_FLIP_MUTATOR>();
        MarkingHeap(collectedRoots);
        STWParam finalMarkStwParam{"final-mark"};
    {
        ScopedStopTheWorld stw(finalMarkStwParam, true, GCPhase::GC_PHASE_FINAL_MARK);
        Remark();
        PostMarking();
        reinterpret_cast<RegionalHeap&>(theAllocator_).PrepareForward();
        Preforward();
    }
        GetGCStats().recordSTWTime(finalMarkStwParam.GetElapsedNs());
        ConcurrentPreforward();
        // reclaim large objects should after preforward(may process weak ref) and
        // before fix heap(may clear live bit)
        if (isNotYoungGC) {
            CollectLargeGarbage();
        }
        SweepThreadLocalJitFort();

        CopyFromSpace();
        WVerify::VerifyAfterForward(*this);

        PrepareFix();
        FixHeap();
        if (isNotYoungGC) {
            CollectNonMovableGarbage();
        }

        TransitionToGCPhase(GCPhase::GC_PHASE_IDLE, true);
        ClearAllGCInfo();
        CollectSmallSpace();
        return;
    }

    auto collectedRoots = EnumRoots<EnumRootsPolicy::STW_AND_FLIP_MUTATOR>();
    MarkingHeap(collectedRoots);
    PreforwardFlip();
    ConcurrentPreforward();
    // reclaim large objects should after preforward(may process weak ref)
    // and before fix heap(may clear live bit)
    if (isNotYoungGC) {
        CollectLargeGarbage();
    }
    SweepThreadLocalJitFort();

    CopyFromSpace();
    WVerify::VerifyAfterForward(*this);

    PrepareFix();
    FixHeap();
    if (isNotYoungGC) {
        CollectNonMovableGarbage();
    }

    TransitionToGCPhase(GCPhase::GC_PHASE_IDLE, true);
    ClearAllGCInfo();
    RegionalHeap &space = reinterpret_cast<RegionalHeap &>(theAllocator_);
    space.DumpAllRegionSummary("Peak GC log");
    space.DumpAllRegionStats("region statistics when gc ends");
    CollectSmallSpace();
}

CArrayList<CArrayList<BaseObject *>> ArkCollector::EnumRootsFlip(STWParam& param,
                                                                 const common::RefFieldVisitor &visitor)
{
    const auto enumGlobalRoots = [this, &visitor]() {
        SetGCThreadQosPriority(common::PriorityMode::STW);
        EnumRootsImpl<VisitGlobalRoots>(visitor);
        SetGCThreadQosPriority(common::PriorityMode::FOREGROUND);
    };

    std::mutex stackMutex;
    CArrayList<CArrayList<BaseObject *>> rootSet;  // allcate for each mutator
    FlipFunction enumMutatorRoot = [&rootSet, &stackMutex](Mutator &mutator) {
        CArrayList<BaseObject *> roots;
        RefFieldVisitor localVisitor = [&roots](RefField<> &root) { roots.emplace_back(root.GetTargetObject()); };
        VisitMutatorRoot(localVisitor, mutator);
        std::lock_guard<std::mutex> lockGuard(stackMutex);
        rootSet.emplace_back(std::move(roots));
    };
    MutatorManager::Instance().FlipMutators(param, enumGlobalRoots, &enumMutatorRoot);
    return rootSet;
}

void ArkCollector::ProcessStringTable()
{
#ifdef GC_STW_STRINGTABLE
    return;
#endif
    if (Heap::GetHeap().GetGCReason() == GC_REASON_YOUNG) {
        // no need to fix weak ref in young gc
        return;
    }

    WeakRefFieldVisitor weakVisitor = [this](RefField<> &refField) -> bool {
        auto isSurvivor = [this](BaseObject* oldObj) {
            RegionDesc* region = RegionDesc::GetAliveRegionDescAt(reinterpret_cast<uintptr_t>(oldObj));
            return (gcReason_ == GC_REASON_YOUNG && !region->IsInYoungSpace())
                || region->IsMarkedObject(oldObj)
                || region->IsNewObjectSinceMarking(oldObj)
                || region->IsToRegion();
        };

        RefField<> oldField(refField);
        BaseObject *oldObj = oldField.GetTargetObject();
        if (oldObj == nullptr) {
            return false;
        }
        if (!isSurvivor(oldObj)) {
            // CAS failure means some mutator or gc thread writes a new ref (must be
            // a to-object), no need to retry.
            RefField<> newField(nullptr);
            if (refField.CompareExchange(oldField.GetFieldValue(), newField.GetFieldValue())) {
                DLOG(FIX, "fix weak raw-ref @%p: %p -> %p", &refField, oldObj, nullptr);
            }
            return false;
        }
        DLOG(FIX, "visit weak raw-ref @%p: %p", &refField, oldObj);
        if (IsFromObject(oldObj)) {
            BaseObject *toVersion = TryForwardObject(oldObj);
            CHECK_CC(toVersion != nullptr);
            RefField<> newField(toVersion);
            // CAS failure means some mutator or gc thread writes a new ref (must be
            // a to-object), no need to retry.
            if (refField.CompareExchange(oldField.GetFieldValue(), newField.GetFieldValue())) {
                DLOG(FIX, "fix weak raw-ref @%p: %p -> %p", &refField, oldObj, toVersion);
            }
        }
        return true;
    };
    BaseRuntime::GetInstance()->ProcessStringTable(weakVisitor);
}


void ArkCollector::ProcessFinalizers()
{
    std::function<bool(BaseObject*)> finalizable = [this](BaseObject* obj) { return !IsMarkedObject(obj); };
    FinalizerProcessor& fp = collectorResources_.GetFinalizerProcessor();
    fp.EnqueueFinalizables(finalizable, snapshotFinalizerNum_);
    fp.Notify();
}

BaseObject* ArkCollector::ForwardObject(BaseObject* obj)
{
    BaseObject* to = TryForwardObject(obj);
    if (to != nullptr) {
        HeapProfilerListener::GetInstance().OnMoveEvent(reinterpret_cast<uintptr_t>(obj),
                                                        reinterpret_cast<uintptr_t>(to),
                                                        to->GetSize());
    }
    return (to != nullptr) ? to : obj;
}

BaseObject* ArkCollector::TryForwardObject(BaseObject* obj)
{
    return CopyObjectImpl(obj);
}

// ConcurrentGC
BaseObject* ArkCollector::CopyObjectImpl(BaseObject* obj)
{
    // reconsider phase difference between mutator and GC thread during transition.
    if (IsGcThread()) {
        CHECK_CC(GetGCPhase() == GCPhase::GC_PHASE_PRECOPY || GetGCPhase() == GCPhase::GC_PHASE_COPY ||
                 GetGCPhase() == GCPhase::GC_PHASE_FIX || GetGCPhase() == GCPhase::GC_PHASE_FINAL_MARK);
    } else {
        auto phase = Mutator::GetMutator()->GetMutatorPhase();
        CHECK_CC(phase == GCPhase::GC_PHASE_PRECOPY || phase == GCPhase::GC_PHASE_COPY ||
                 phase == GCPhase::GC_PHASE_FIX);
    }

    do {
        BaseStateWord oldWord = obj->GetBaseStateWord();

        // 1. object has already been forwarded
        if (obj->IsForwarded()) {
            auto toObj = GetForwardingPointer(obj);
            DLOG(COPY, "skip forwarded obj %p -> %p<%p>(%zu)", obj, toObj, toObj->GetTypeInfo(), toObj->GetSize());
            return toObj;
        }

        // ConcurrentGC
        // 2. object is being forwarded, spin until it is forwarded (or gets its own forwarded address)
        if (oldWord.IsForwarding()) {
            sched_yield();
            continue;
        }

        // 3. hope we can copy this object
        if (obj->TryLockExclusive(oldWord)) {
            return CopyObjectAfterExclusive(obj);
        }
    } while (true);
    LOG_COMMON(FATAL) << "forwardObject exit in wrong path";
    UNREACHABLE_CC();
    return nullptr;
}

BaseObject* ArkCollector::CopyObjectAfterExclusive(BaseObject* obj)
{
    size_t size = RegionalHeap::GetAllocSize(*obj);
    // 8: size of free object, but free object can not be copied.
    if (size == 8) {
        LOG_COMMON(FATAL) << "forward free obj: " << obj <<
            "is survived: " << (IsSurvivedObject(obj) ? "true" : "false");
    }
    BaseObject* toObj = fwdTable_.RouteObject(obj, size);
    if (toObj == nullptr) {
        // ConcurrentGC
        obj->UnlockExclusive(BaseStateWord::ForwardState::NORMAL);
        return toObj;
    }
    DLOG(COPY, "copy obj %p<%p>(%zu) to %p", obj, obj->GetTypeInfo(), size, toObj);
    CopyObject(*obj, *toObj, size);

    ASSERT_LOGF(IsToObject(toObj), "Copy object to invalid region");
    toObj->SetForwardState(BaseStateWord::ForwardState::NORMAL);

    std::atomic_thread_fence(std::memory_order_release);
    obj->SetSizeForwarded(size);
    // Avoid seeing the fwd pointer before observing the size modification
    // when calling GetSize during the CopyPhase.
    std::atomic_thread_fence(std::memory_order_release);
    obj->SetForwardingPointerAfterExclusive(toObj);
    return toObj;
}

void ArkCollector::ClearAllGCInfo()
{
    COMMON_PHASE_TIMER("ClearAllGCInfo");
    RegionalHeap& space = reinterpret_cast<RegionalHeap&>(theAllocator_);
    space.ClearAllGCInfo();
    reinterpret_cast<RegionalHeap&>(theAllocator_).ClearJitFortAwaitingMark();
}

void ArkCollector::CollectSmallSpace()
{
    OHOS_HITRACE(HITRACE_LEVEL_COMMERCIAL, "CMCGC::CollectSmallSpace", "");
    GCStats& stats = GetGCStats();
    RegionalHeap& space = reinterpret_cast<RegionalHeap&>(theAllocator_);
    {
        COMMON_PHASE_TIMER("CollectFromSpaceGarbage");
        stats.collectedBytes += stats.smallGarbageSize;
        if (gcReason_ == GC_REASON_APPSPAWN) {
            VLOG(DEBUG, "APPSPAWN GC Collect");
            space.CollectAppSpawnSpaceGarbage();
        } else {
            space.CollectFromSpaceGarbage();
            space.HandlePromotion();
        }
    }

    size_t candidateBytes = stats.fromSpaceSize + stats.nonMovableSpaceSize + stats.largeSpaceSize;
    stats.garbageRatio = (candidateBytes > 0) ? static_cast<float>(stats.collectedBytes) / candidateBytes : 0;

    stats.liveBytesAfterGC = space.GetAllocatedBytes();

    VLOG(INFO,
         "collect %zu B: small %zu - %zu B, non-movable %zu - %zu B, large %zu - %zu B. garbage ratio %.2f%%",
         stats.collectedBytes, stats.fromSpaceSize, stats.smallGarbageSize, stats.nonMovableSpaceSize,
         stats.nonMovableGarbageSize, stats.largeSpaceSize, stats.largeGarbageSize,
         stats.garbageRatio * 100); // The base of the percentage is 100
    OHOS_HITRACE(HITRACE_LEVEL_COMMERCIAL, "CMCGC::CollectSmallSpace END", (
                    "collect:" + std::to_string(stats.collectedBytes) +
                    "B;small:" + std::to_string(stats.fromSpaceSize) +
                    "-" + std::to_string(stats.smallGarbageSize) +
                    "B;non-movable:" + std::to_string(stats.nonMovableSpaceSize) +
                    "-" + std::to_string(stats.nonMovableGarbageSize) +
                    "B;large:" + std::to_string(stats.largeSpaceSize) +
                    "-" + std::to_string(stats.largeGarbageSize) +
                    "B;garbage ratio:" + std::to_string(stats.garbageRatio)
                ).c_str());

    collectorResources_.GetFinalizerProcessor().NotifyToReclaimGarbage();
}

void ArkCollector::SetGCThreadQosPriority(common::PriorityMode mode)
{
#ifdef ENABLE_QOS
    LOG_COMMON(DEBUG) << "SetGCThreadQosPriority gettid " << gettid();
    OHOS_HITRACE(HITRACE_LEVEL_COMMERCIAL, "CMCGC::SetGCThreadQosPriority", "");
    switch (mode) {
        case PriorityMode::STW: {
            OHOS::QOS::SetQosForOtherThread(OHOS::QOS::QosLevel::QOS_USER_INTERACTIVE, gettid());
            break;
        }
        case PriorityMode::FOREGROUND: {
            OHOS::QOS::SetQosForOtherThread(OHOS::QOS::QosLevel::QOS_USER_INITIATED, gettid());
            break;
        }
        case PriorityMode::BACKGROUND: {
            OHOS::QOS::ResetQosForOtherThread(gettid());
            break;
        }
        default:
            UNREACHABLE_CC();
            break;
    }
    common::Taskpool::GetCurrentTaskpool()->SetThreadPriority(mode);
#endif
}

bool ArkCollector::ShouldIgnoreRequest(GCRequest& request) { return request.ShouldBeIgnored(); }
} // namespace common
