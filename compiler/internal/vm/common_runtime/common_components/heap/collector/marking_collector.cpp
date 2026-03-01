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

#include "common_components/heap/collector/marking_collector.h"

#include <new>
#include <iomanip>
#include <string>

#include "common_components/heap/allocator/alloc_buffer.h"
#include "common_components/heap/collector/heuristic_gc_policy.h"
#include "common_interfaces/base/runtime_param.h"
#include "common_components/heap/collector/utils.h"

namespace common {
const size_t MarkingCollector::MAX_MARKING_WORK_SIZE = 16; // fork task if bigger
const size_t MarkingCollector::MIN_MARKING_WORK_SIZE = 8;  // forbid forking task if smaller

void StaticRootTable::VisitRoots(const RefFieldVisitor& visitor)
{
    std::lock_guard<std::mutex> lock(gcRootsLock_);
    uint32_t gcRootsSize = 0;
    std::unordered_set<RefField<>*> visitedSet;
    for (auto iter = gcRootsBuckets_.begin(); iter != gcRootsBuckets_.end(); iter++) {
        gcRootsSize = iter->second;
        StaticRootArray* array = iter->first;
        for (uint32_t i = 0; i < gcRootsSize; i++) {
            RefField<>* root = array->content[i];
            // make sure to visit each static root only once time.
            if (visitedSet.find(root) != visitedSet.end()) {
                continue;
            }
            visitedSet.insert(root);
            visitor(*root);
        }
    }
}

template <bool ProcessXRef>
class ConcurrentMarkingTask : public common::Task {
public:
    ConcurrentMarkingTask(uint32_t id, MarkingCollector &tc, ParallelMarkingMonitor &monitor,
                          GlobalMarkStack &globalMarkStack)
        : Task(id), collector_(tc), monitor_(monitor), globalMarkStack_(globalMarkStack)
    {}

    ~ConcurrentMarkingTask() override = default;

    // run concurrent marking task.
    bool Run([[maybe_unused]] uint32_t threadIndex) override
    {
        ParallelLocalMarkStack markStack(&globalMarkStack_, &monitor_);
        do {
            if (!monitor_.TryStartStep()) {
                break;
            }
            collector_.ProcessMarkStack<ProcessXRef>(threadIndex, markStack);
            monitor_.FinishStep();
        } while (monitor_.WaitNextStepOrFinished());
        monitor_.NotifyFinishOne();
        return true;
    }

private:
    MarkingCollector &collector_;
    ParallelMarkingMonitor &monitor_;
    GlobalMarkStack &globalMarkStack_;
};

static void ClearWeakRef(WeakStack::value_type *begin, WeakStack::value_type *end)
{
    for (auto iter = begin; iter != end; ++iter) {
        RefField<> *fieldPointer = iter->first;
        size_t offset = iter->second;
        ASSERT_LOGF(offset % sizeof(RefField<>) == 0, "offset is not aligned");

        RefField<> &field = reinterpret_cast<RefField<>&>(*fieldPointer);
        RefField<> oldField(field);

        if (!Heap::IsTaggedObject(oldField.GetFieldValue())) {
            continue;
        }
        BaseObject* targetObj = oldField.GetTargetObject();
        DCHECK_CC(Heap::IsHeapAddress(targetObj));

        auto targetRegion = RegionDesc::GetAliveRegionDescAt(reinterpret_cast<MAddress>(targetObj));
        if (targetRegion->IsMarkedObject(targetObj) || targetRegion->IsNewObjectSinceMarking(targetObj)) {
            continue;
        }

        BaseObject* obj = reinterpret_cast<BaseObject*>(reinterpret_cast<uintptr_t>(&field) - offset);
        if (RegionDesc::GetAliveRegionType(reinterpret_cast<MAddress>(obj)) == RegionDesc::RegionType::FROM_REGION) {
            BaseObject* toObj = obj->GetForwardingPointer();

            // Make sure even the object that contains the weak reference is trimed before forwarding, the weak ref
            // field is still within the object
            if (toObj != nullptr && offset < obj->GetSizeForwarded()) {
                RefField<>& toField = *reinterpret_cast<RefField<>*>(reinterpret_cast<uintptr_t>(toObj) + offset);
                toField.ClearRef(oldField.GetFieldValue());
            }
        }
        field.ClearRef(oldField.GetFieldValue());
    }
}

void MarkingCollector::ProcessWeakStack(WeakStack &weakStack)
{
    WeakStack::value_type *begin = &(*weakStack.begin());
    WeakStack::value_type *end = begin + weakStack.size();
    ClearWeakRef(begin, end);
}

template <bool ProcessXRef>
void MarkingCollector::ProcessMarkStack([[maybe_unused]] uint32_t threadIndex, ParallelLocalMarkStack &markStack)
{
    size_t nNewlyMarked = 0;
    WeakStack weakStack;
    auto visitor = CreateMarkingObjectRefFieldsVisitor(markStack, weakStack);
    std::vector<BaseObject *> remarkStack;
    auto fetchFromSatbBuffer = [this, &markStack, &remarkStack]() {
        SatbBuffer::Instance().TryFetchOneRetiredNode(remarkStack);
        bool needProcess = false;
        while (!remarkStack.empty()) {
            BaseObject *obj = remarkStack.back();
            remarkStack.pop_back();
            if (Heap::IsHeapAddress(obj) && (!MarkObject(obj))) {
                markStack.Push(obj);
                needProcess = true;
                DLOG(TRACE, "tracing take from satb buffer: obj %p", obj);
            }
        }
        return needProcess;
    };
    size_t iterationCnt = 0;
    constexpr size_t maxIterationLoopNum = 1000;
    // loop until work stack empty.
    while (true) {
        BaseObject *object;
        while (markStack.Pop(&object)) {
            ++nNewlyMarked;
            auto region = RegionDesc::GetAliveRegionDescAt(
                static_cast<MAddress>(reinterpret_cast<uintptr_t>(object)));
            visitor.SetMarkingRefFieldArgs(object);
            auto objSize = object->ForEachRefFieldAndGetSize(visitor.GetRefFieldVisitor());
            region->AddLiveByteCount(objSize);
#ifdef PANDA_JS_ETS_HYBRID_MODE
            if constexpr (ProcessXRef) {
                MarkingObjectXRef(object, markStack);
            }
#endif
        }
        // Try some task from satb buffer, bound the loop to make sure it converges in time
        if (++iterationCnt >= maxIterationLoopNum) {
            break;
        }
        if (!fetchFromSatbBuffer()) {
            break;
        }
    }
    DCHECK_CC(markStack.IsEmpty());
    // newly marked statistics.
    markedObjectCount_.fetch_add(nNewlyMarked, std::memory_order_relaxed);
    MergeWeakStack(weakStack);
}

void MarkingCollector::MergeWeakStack(WeakStack &weakStack)
{
    std::lock_guard<std::mutex> lock(weakStackLock_);

    // Preprocess the weak stack to minimize work during STW remark.
    for (const auto &pair : weakStack) {
        RefField<> oldField(*pair.first);

        if (!Heap::IsTaggedObject(oldField.GetFieldValue())) {
            continue;
        }
        auto obj = oldField.GetTargetObject();
        DCHECK_CC(Heap::IsHeapAddress(obj));

        auto region = RegionDesc::GetAliveRegionDescAt(reinterpret_cast<MAddress>(obj));
        if (region->IsNewObjectSinceMarking(obj) || region->IsMarkedObject(obj)) {
            continue;
        }

        globalWeakStack_.push_back(pair);
    }
}

void MarkingCollector::EnumConcurrencyModelRoots(RootSet& rootSet) const
{
    LOG_COMMON(FATAL) << "Unresolved fatal";
    UNREACHABLE_CC();
}

class MergeMutatorRootsScope {
public:
    MergeMutatorRootsScope() : manager_(&MutatorManager::Instance()), worldStopped_(manager_->WorldStopped())
    {
        if (!worldStopped_) {
            manager_->MutatorManagementWLock();
        }
    }

    ~MergeMutatorRootsScope()
    {
        if (!worldStopped_) {
            manager_->MutatorManagementWUnlock();
        }
    }

private:
    MutatorManager *manager_;
    bool worldStopped_;
};

void MarkingCollector::TracingImpl(GlobalMarkStack &globalMarkStack, bool parallel, bool Remark)
{
    OHOS_HITRACE(HITRACE_LEVEL_COMMERCIAL, ("CMCGC::TracingImpl_" + std::to_string(globalMarkStack.Count())).c_str(),
        "");

    // enable parallel marking if we have thread pool.
    Taskpool *threadPool = GetThreadPool();
    ASSERT_LOGF(threadPool != nullptr, "thread pool is null");
    if (parallel) { // parallel marking.
        uint32_t parallelCount = 0;
        // During the STW remark phase, Expect it to utilize all GC threads.
        if (Remark) {
            parallelCount = GetGCThreadCount(true);
        } else {
            parallelCount = GetGCThreadCount(true) - 1;
        }
        uint32_t threadCount = parallelCount + 1;
        ParallelMarkingMonitor monitor(parallelCount, parallelCount);
        for (uint32_t i = 0; i < parallelCount; ++i) {
#ifdef PANDA_JS_ETS_HYBRID_MODE
            if (gcReason_ == GCReason::GC_REASON_XREF) {
                threadPool->PostTask(
                    std::make_unique<ConcurrentMarkingTask<true>>(0, *this, monitor, globalMarkStack));
            } else {
                threadPool->PostTask(std::make_unique<ConcurrentMarkingTask<false>>(0, *this, monitor, globalMarkStack));
            }
#else
            threadPool->PostTask(std::make_unique<ConcurrentMarkingTask<false>>(0, *this, monitor, globalMarkStack));
#endif
        }
        ParallelLocalMarkStack markStack(&globalMarkStack, &monitor);
        do {
            if (!monitor.TryStartStep()) {
                break;
            }
#ifdef PANDA_JS_ETS_HYBRID_MODE
            if (gcReason_ == GCReason::GC_REASON_XREF) {
                ProcessMarkStack<true>(0, markStack);
            } else {
                ProcessMarkStack<false>(0, markStack);
            }
#else
            ProcessMarkStack<false>(0, markStack);
#endif
            monitor.FinishStep();
        } while (monitor.WaitNextStepOrFinished());
        monitor.WaitAllFinished();
    } else {
        // serial marking with a single mark task.
        // Fixme: this `ParallelLocalMarkStack` could be replaced with `SequentialLocalMarkStack`, and no need to
        // use monitor, but this need to add template param to `ProcessMarkStack`.
        // So for convenience just use a fake dummy parallel one.
        ParallelMarkingMonitor dummyMonitor(0, 0);
        ParallelLocalMarkStack markStack(&globalMarkStack, &dummyMonitor);
#ifdef PANDA_JS_ETS_HYBRID_MODE
            if (gcReason_ == GCReason::GC_REASON_XREF) {
                ProcessMarkStack<true>(0, markStack);
            } else {
                ProcessMarkStack<false>(0, markStack);
            }
#else
            ProcessMarkStack<false>(0, markStack);
#endif
    }
}

bool MarkingCollector::PushRootToWorkStack(LocalCollectStack &collectStack, BaseObject *obj)
{
    RegionDesc *regionInfo = RegionDesc::GetAliveRegionDescAt(reinterpret_cast<HeapAddress>(obj));
    if (gcReason_ == GCReason::GC_REASON_YOUNG && !regionInfo->IsInYoungSpace()) {
        DLOG(ENUM, "enum: skip old object %p<%p>(%zu)", obj, obj->GetTypeInfo(), obj->GetSize());
        return false;
    }

    // inline MarkObject
    bool marked = regionInfo->MarkObject(obj);
    if (!marked) {
        DCHECK_CC(!regionInfo->IsGarbageRegion());
        DLOG(TRACE, "mark obj %p<%p>(%zu) in region %p(%u)@%#zx, live %u", obj, obj->GetTypeInfo(), obj->GetSize(),
             regionInfo, regionInfo->GetRegionType(), regionInfo->GetRegionStart(), regionInfo->GetLiveByteCount());
        collectStack.Push(obj);
        return true;
    } else {
        return false;
    }
}

void MarkingCollector::PushRootsToWorkStack(LocalCollectStack &collectStack,
                                            const CArrayList<BaseObject *> &collectedRoots)
{
    OHOS_HITRACE(HITRACE_LEVEL_COMMERCIAL,
                 ("CMCGC::PushRootsToWorkStack_" + std::to_string(collectedRoots.size())).c_str(), "");
    for (BaseObject *obj : collectedRoots) {
        PushRootToWorkStack(collectStack, obj);
    }
}

void MarkingCollector::MarkingRoots(const CArrayList<BaseObject *> &collectedRoots)
{
    OHOS_HITRACE(HITRACE_LEVEL_COMMERCIAL, "CMCGC::MarkingRoots", "");

    GlobalMarkStack globalMarkStack;

    {
        LocalCollectStack collectStack(&globalMarkStack);

        PushRootsToWorkStack(collectStack, collectedRoots);

        if (Heap::GetHeap().GetGCReason() == GC_REASON_YOUNG) {
            OHOS_HITRACE(HITRACE_LEVEL_COMMERCIAL, "CMCGC::PushRootInRSet", "");
            auto func = [this, &collectStack](BaseObject *object) { MarkRememberSetImpl(object, collectStack); };
            RegionalHeap &space = reinterpret_cast<RegionalHeap &>(Heap::GetHeap().GetAllocator());
            space.MarkRememberSet(func);
        }

        collectStack.Publish();
    }

    COMMON_PHASE_TIMER("MarkingRoots");

    ASSERT_LOGF(GetThreadPool() != nullptr, "null thread pool");

    // use fewer threads and lower priority for concurrent mark.
    const uint32_t maxWorkers = GetGCThreadCount(true) - 1;

    {
        COMMON_PHASE_TIMER("Concurrent marking");
        TracingImpl(globalMarkStack, maxWorkers > 0, false);
    }
}

void MarkingCollector::Remark()
{
    GlobalMarkStack globalMarkStack;
    const uint32_t maxWorkers = GetGCThreadCount(true) - 1;
    OHOS_HITRACE(HITRACE_LEVEL_COMMERCIAL, "CMCGC::Remark[STW]", "");
    COMMON_PHASE_TIMER("STW re-marking");
    RemarkAndPreforwardStaticRoots(globalMarkStack);
    ConcurrentRemark(globalMarkStack, maxWorkers > 0); // Mark enqueue
    TracingImpl(globalMarkStack, maxWorkers > 0, true);
    MarkAwaitingJitFort(); // Mark awaiting
    ClearWeakStack(maxWorkers > 0);

    OHOS_HITRACE(HITRACE_LEVEL_COMMERCIAL, "CMCGC::MarkingRoots END",
        ("mark obejects:" + std::to_string(markedObjectCount_.load(std::memory_order_relaxed))).c_str());
    VLOG(DEBUG, "mark %zu objects", markedObjectCount_.load(std::memory_order_relaxed));
}

class ClearWeakRefTask : public ArrayTaskDispatcher::ArrayTask {
public:
    using T = WeakStack::value_type;
    explicit ClearWeakRefTask(CArrayList<T> *inputs) : data_(inputs) {}
    void Run(void *begin, void *end) override
    {
        T *first = reinterpret_cast<T *>(begin);
        T *last = reinterpret_cast<T *>(end);
        DCHECK_CC(((uintptr_t)last - (uintptr_t)first) % sizeof(T) == 0 &&
               ((uintptr_t)first - (uintptr_t)data_->data()) % sizeof(T) == 0);
        DCHECK_CC(first == last || (first >= data_->data() && last > data_->data()));
        DCHECK_CC(first == last ||
            (first < (data_->data() + data_->size()) && last <= (data_->data() + data_->size())));
        ClearWeakRef(first, last);
    }
    CArrayList<T> *data_;
};

void MarkingCollector::ClearWeakStack(bool parallel)
{
    OHOS_HITRACE(HITRACE_LEVEL_COMMERCIAL, "CMCGC::ProcessGlobalWeakStack", "");
    if (gcReason_ == GC_REASON_YOUNG || globalWeakStack_.empty()) {
        return;
    }
    // the globalWeakStack_ cannot be modified during task execution
    Taskpool *threadPool = GetThreadPool();
    ASSERT_LOGF(threadPool != nullptr, "thread pool is null");
    constexpr size_t BATCH_N = 200;
    if (parallel && globalWeakStack_.size() > BATCH_N) {
        auto inputs = &globalWeakStack_;
        ClearWeakRefTask callback(inputs);
        DCHECK_CC(inputs->size() < (SIZE_MAX / sizeof(WeakStack::value_type)));
        ArrayTaskDispatcher dispatcher(inputs->data(), inputs->size() * sizeof(WeakStack::value_type),
                                       BATCH_N * sizeof(WeakStack::value_type), &callback);
        dispatcher.Dispatch(threadPool, threadPool->GetTotalThreadNum());
        dispatcher.JoinAndWait();
    } else {
        ProcessWeakStack(globalWeakStack_);
    }
}

bool MarkingCollector::MarkSatbBuffer(GlobalMarkStack &globalMarkStack)
{
    OHOS_HITRACE(HITRACE_LEVEL_COMMERCIAL, "CMCGC::MarkSatbBuffer", "");
    COMMON_PHASE_TIMER("MarkSatbBuffer");
    auto visitSatbObj = [this, &globalMarkStack]() {
        std::vector<BaseObject *> remarkStack;
        auto func = [&remarkStack](Mutator& mutator) {
            const SatbBuffer::TreapNode* node = mutator.GetSatbBufferNode();
            if (node != nullptr) {
                const_cast<SatbBuffer::TreapNode*>(node)->GetObjects(remarkStack);
            }
        };
        MutatorManager::Instance().VisitAllMutators(func);
        SatbBuffer::Instance().GetRetiredObjects(remarkStack);

        LocalCollectStack collectStack(&globalMarkStack);
        while (!remarkStack.empty()) { // LCOV_EXCL_BR_LINE
            BaseObject* obj = remarkStack.back();
            remarkStack.pop_back();
            if (Heap::IsHeapAddress(obj) && !this->MarkObject(obj)) {
                collectStack.Push(obj);
                DLOG(TRACE, "satb buffer add obj %p", obj);
            }
        }
        collectStack.Publish();
    };

    visitSatbObj();
    return true;
}

void MarkingCollector::MarkRememberSetImpl(BaseObject* object, LocalCollectStack &collectStack)
{
    object->ForEachRefField([this, &collectStack, &object](RefField<>& field) {
        BaseObject* targetObj = field.GetTargetObject();
        if (Heap::IsHeapAddress(targetObj)) {
            RegionDesc* region = RegionDesc::GetAliveRegionDescAt(reinterpret_cast<HeapAddress>(targetObj));
            if (region->IsInYoungSpace() &&
                !region->IsNewObjectSinceMarking(targetObj) &&
                !this->MarkObject(targetObj)) {
                collectStack.Push(targetObj);
                DLOG(TRACE, "remember set marking obj: %p@%p, ref: %p", object, &field, targetObj);
            }
        }
    });
}

void MarkingCollector::ConcurrentRemark(GlobalMarkStack &globalMarkStack, bool parallel)
{
    LOGF_CHECK(MarkSatbBuffer(globalMarkStack)) << "not cleared\n";
}

void MarkingCollector::MarkAwaitingJitFort()
{
    reinterpret_cast<RegionalHeap&>(theAllocator_).MarkAwaitingJitFort();
}

void MarkingCollector::Init(const RuntimeParam& param) {}

void MarkingCollector::Fini() { Collector::Fini(); }

#if defined(GCINFO_DEBUG) && GCINFO_DEBUG
void MarkingCollector::DumpHeap(const CString& tag)
{
    ASSERT_LOGF(MutatorManager::Instance().WorldStopped(), "Not In STW");
    DLOG(FRAGMENT, "DumpHeap %s", tag.Str());
    // dump roots
    DumpRoots(FRAGMENT);
    // dump object contents
    auto dumpVisitor = [](BaseObject* obj) {
        // support Dump
        // obj->DumpObject(FRAGMENT)
    };
    bool ret = Heap::GetHeap().ForEachObject(dumpVisitor, false);
    LOGE_IF(UNLIKELY_CC(!ret)) << "theAllocator.ForEachObject() in DumpHeap() return false.";

    // dump object types
    DLOG(FRAGMENT, "Print Type information");
    std::set<TypeInfo*> classinfoSet;
    auto assembleClassInfoVisitor = [&classinfoSet](BaseObject* obj) {
        TypeInfo* classInfo = obj->GetTypeInfo();
        // No need to check the result of insertion, because there are multiple-insertions.
        (void)classinfoSet.insert(classInfo);
    };
    ret = Heap::GetHeap().ForEachObject(assembleClassInfoVisitor, false);
    LOGE_IF(UNLIKELY_CC(!ret)) << "theAllocator.ForEachObject()#2 in DumpHeap() return false.";

    for (auto it = classinfoSet.begin(); it != classinfoSet.end(); it++) {
        TypeInfo* classInfo = *it;
    }
    DLOG(FRAGMENT, "Dump Allocator");
}

void MarkingCollector::DumpRoots(LogType logType)
{
    LOG_COMMON(FATAL) << "Unresolved fatal";
    UNREACHABLE_CC();
}
#endif

void MarkingCollector::PreGarbageCollection(bool isConcurrent)
{
    // SatbBuffer should be initialized before concurrent enumeration.
    SatbBuffer::Instance().Init();
    // prepare thread pool.

    GCStats& gcStats = GetGCStats();
    gcStats.reason = gcReason_;
    gcStats.async = !g_gcRequests[gcReason_].IsSyncGC();
    gcStats.gcType = gcType_;
    gcStats.isConcurrentMark = isConcurrent;
    gcStats.collectedBytes = 0;
    gcStats.smallGarbageSize = 0;
    gcStats.nonMovableGarbageSize = 0;
    gcStats.largeGarbageSize = 0;
    gcStats.gcStartTime = TimeUtil::NanoSeconds();
    gcStats.totalSTWTime = 0;
    gcStats.maxSTWTime = 0;
#if defined(GCINFO_DEBUG) && GCINFO_DEBUG
    DumpBeforeGC();
#endif
}

void MarkingCollector::PostGarbageCollection(uint64_t gcIndex)
{
    SatbBuffer::Instance().ReclaimALLPages();
    // release pages in PagePool
    PagePool::Instance().Trim();
    collectorResources_.MarkGCFinish(gcIndex);

#if defined(GCINFO_DEBUG) && GCINFO_DEBUG
    DumpAfterGC();
#endif
}

void MarkingCollector::UpdateGCStats()
{
    RegionalHeap& space = reinterpret_cast<RegionalHeap&>(theAllocator_);
    GCStats& gcStats = GetGCStats();
    gcStats.Dump();

    size_t oldThreshold = gcStats.heapThreshold;
    size_t oldTargetFootprint = gcStats.targetFootprint;
    size_t recentBytes = space.GetRecentAllocatedSize();
    size_t survivedBytes = space.GetSurvivedSize();
    size_t bytesAllocated = space.GetAllocatedBytes();

    size_t targetSize;
    HeapParam& heapParam = BaseRuntime::GetInstance()->GetHeapParam();
    GCParam& gcParam = BaseRuntime::GetInstance()->GetGCParam();
    if (!gcStats.isYoungGC()) {
        gcStats.shouldRequestYoung = true;
        size_t delta = bytesAllocated * (1.0 / heapParam.heapUtilization - 1.0);
        size_t growBytes = std::min(delta, gcParam.maxGrowBytes);
        growBytes = std::max(growBytes, gcParam.minGrowBytes);
        targetSize = bytesAllocated + growBytes * gcParam.multiplier;
    } else {
        gcStats.shouldRequestYoung = gcStats.collectionRate * gcParam.ygcRateAdjustment >= g_fullGCMeanRate
                                && bytesAllocated <= oldThreshold;
        size_t adjustMaxGrowBytes = gcParam.maxGrowBytes * gcParam.multiplier;
        if (bytesAllocated + adjustMaxGrowBytes < oldTargetFootprint) {
            targetSize = bytesAllocated + adjustMaxGrowBytes;
        } else {
            targetSize = std::max(bytesAllocated, oldTargetFootprint);
        }
    }

    gcStats.targetFootprint = targetSize;
    size_t remainingBytes = recentBytes;
    remainingBytes = std::min(remainingBytes, gcParam.kMaxConcurrentRemainingBytes);
    remainingBytes = std::max(remainingBytes, gcParam.kMinConcurrentRemainingBytes);
    if (UNLIKELY_CC(remainingBytes > gcStats.targetFootprint)) {
        remainingBytes = std::min(gcParam.kMinConcurrentRemainingBytes, gcStats.targetFootprint);
    }
    gcStats.heapThreshold = std::max(gcStats.targetFootprint - remainingBytes, bytesAllocated);
    gcStats.heapThreshold = std::max(gcStats.heapThreshold, 20 * MB); // 20 MB:set 20 MB as min heapThreshold
    gcStats.heapThreshold = std::min(gcStats.heapThreshold, gcParam.gcThreshold);

    UpdateNativeThreshold(gcParam);
    Heap::GetHeap().RecordAliveSizeAfterLastGC(bytesAllocated);
    if (!gcStats.isYoungGC()) {
        Heap::GetHeap().SetRecordHeapObjectSizeBeforeSensitive(bytesAllocated);
    }

    if (!gcStats.isYoungGC()) {
        g_gcRequests[GC_REASON_HEU].SetMinInterval(gcParam.gcInterval);
    } else {
        g_gcRequests[GC_REASON_YOUNG].SetMinInterval(gcParam.gcInterval);
    }
    gcStats.IncreaseAccumulatedFreeSize(bytesAllocated);
    std::ostringstream oss;
    oss << "allocated bytes " << bytesAllocated << " (survive bytes " << survivedBytes
                     << ", recent-allocated " << recentBytes << "), update target footprint "
                     << oldTargetFootprint << " -> " << gcStats.targetFootprint
                     << ", update gc threshold " << oldThreshold << " -> " << gcStats.heapThreshold;
    VLOG(INFO, oss.str().c_str());
    OHOS_HITRACE(HITRACE_LEVEL_COMMERCIAL, "CMCGC::UpdateGCStats END", (
                    "allocated bytes:" + std::to_string(bytesAllocated) +
                    ";survive bytes:" + std::to_string(survivedBytes) +
                    ";recent allocated:" + std::to_string(recentBytes) +
                    ";update target footprint:" + std::to_string(oldTargetFootprint) +
                    ";new target footprint:" + std::to_string(gcStats.targetFootprint) +
                    ";old gc threshold:" + std::to_string(oldThreshold) +
                    ";new gc threshold:" + std::to_string(gcStats.heapThreshold) +
                    ";native size:" + std::to_string(Heap::GetHeap().GetNotifiedNativeSize()) +
                    ";new native threshold:" + std::to_string(Heap::GetHeap().GetNativeHeapThreshold())
                ).c_str());
}

void MarkingCollector::UpdateNativeThreshold(GCParam& gcParam)
{
    size_t nativeHeapSize = Heap::GetHeap().GetNotifiedNativeSize();
    size_t newNativeHeapThreshold = Heap::GetHeap().GetNotifiedNativeSize();
    if (nativeHeapSize < MAX_NATIVE_SIZE_INC) {
        newNativeHeapThreshold = std::max(nativeHeapSize + gcParam.minGrowBytes,
                                          nativeHeapSize * NATIVE_MULTIPLIER);
    } else {
        newNativeHeapThreshold += MAX_NATIVE_STEP;
    }
    newNativeHeapThreshold = std::min(newNativeHeapThreshold, MAX_GLOBAL_NATIVE_LIMIT);
    Heap::GetHeap().SetNativeHeapThreshold(newNativeHeapThreshold);
    collectorResources_.SetIsNativeGCInvoked(false);
}

void MarkingCollector::CopyObject(const BaseObject& fromObj, BaseObject& toObj, size_t size) const
{
    uintptr_t from = reinterpret_cast<uintptr_t>(&fromObj);
    uintptr_t to = reinterpret_cast<uintptr_t>(&toObj);
    LOGE_IF(memmove_s(reinterpret_cast<void*>(to), size, reinterpret_cast<void*>(from), size) != EOK) <<
        "memmove_s fail";
#if defined(COMMON_TSAN_SUPPORT)
    Sanitizer::TsanFixShadow(reinterpret_cast<void *>(from), reinterpret_cast<void *>(to), size);
#endif
}

void MarkingCollector::ReclaimGarbageMemory(GCReason reason)
{
    if (reason == GC_REASON_OOM) {
        Heap::GetHeap().GetAllocator().ReclaimGarbageMemory(true);
    } else {
        Heap::GetHeap().GetAllocator().ReclaimGarbageMemory(false);
    }
}

void MarkingCollector::RunGarbageCollection(uint64_t gcIndex, GCReason reason, GCType gcType)
{
    gcReason_ = reason;
    gcType_ = gcType;
    auto gcReasonName = std::string(g_gcRequests[gcReason_].name);
    auto currentAllocatedSize = Heap::GetHeap().GetAllocatedSize();
    auto currentThreshold = Heap::GetHeap().GetCollector().GetGCStats().GetThreshold();
    VLOG(INFO, "Begin GC log. GCReason: %s, GCType: %s, Current allocated %s, Current threshold %s, gcIndex=%llu",
        gcReasonName.c_str(), GCTypeToString(gcType), Pretty(currentAllocatedSize).c_str(),
        Pretty(currentThreshold).c_str(), gcIndex);
    OHOS_HITRACE(HITRACE_LEVEL_COMMERCIAL, "CMCGC::RunGarbageCollection", (
                    "GCReason:" + gcReasonName + ";GCType:" + GCTypeToString(gcType) +
                    ";Sensitive:" + std::to_string(static_cast<int>(Heap::GetHeap().GetSensitiveStatus())) +
                    ";Startup:" + std::to_string(static_cast<int>(Heap::GetHeap().GetStartupStatus())) +
                    ";Current Allocated:" + Pretty(currentAllocatedSize) +
                    ";Current Threshold:" + Pretty(currentThreshold) +
                    ";Current Native:" + Pretty(Heap::GetHeap().GetNotifiedNativeSize()) +
                    ";NativeThreshold:" + Pretty(Heap::GetHeap().GetNativeHeapThreshold())
                ).c_str());
    // prevent other threads stop-the-world during GC.
    // this may be removed in the future.
    ScopedSTWLock stwLock;
    PreGarbageCollection(true);
    Heap::GetHeap().SetGCReason(reason);
    GCStats& gcStats = GetGCStats();

    DoGarbageCollection();

    HeapBitmapManager::GetHeapBitmapManager().ClearHeapBitmap();

    ReclaimGarbageMemory(reason);

    PostGarbageCollection(gcIndex);
    MutatorManager::Instance().DestroyExpiredMutators();
    gcStats.gcEndTime = TimeUtil::NanoSeconds();

    uint64_t gcTimeNs = gcStats.gcEndTime - gcStats.gcStartTime;
    double rate = (static_cast<double>(gcStats.collectedBytes) / gcTimeNs) * (static_cast<double>(NS_PER_S) / MB);
    {
        std::ostringstream oss;
        const int prec = 3;
        oss << "total gc time: " << Pretty(gcTimeNs / NS_PER_US) << " us, collection rate ";
        oss << std::setprecision(prec) << rate << " MB/s";
        VLOG(INFO, oss.str().c_str());
    }

    g_gcCount++;
    g_gcTotalTimeUs += (gcTimeNs / NS_PER_US);
    g_gcCollectedTotalBytes += gcStats.collectedBytes;
    gcStats.collectionRate = rate;

    if (!gcStats.isYoungGC()) {
        if (g_fullGCCount == 0) {
            g_fullGCMeanRate = rate;
        } else {
            g_fullGCMeanRate = (g_fullGCMeanRate * g_fullGCCount + rate) / (g_fullGCCount + 1);
        }
        g_fullGCCount++;
    }

    UpdateGCStats();

    if (Heap::GetHeap().GetForceThrowOOM()) {
        Heap::throwOOM();
    }
}

void MarkingCollector::CopyFromSpace()
{
    OHOS_HITRACE(HITRACE_LEVEL_COMMERCIAL, "CMCGC::CopyFromSpace", "");
    TransitionToGCPhase(GCPhase::GC_PHASE_COPY, true);
    RegionalHeap& space = reinterpret_cast<RegionalHeap&>(theAllocator_);
    GCStats& stats = GetGCStats();
    stats.liveBytesBeforeGC = space.GetAllocatedBytes();
    stats.fromSpaceSize = space.FromSpaceSize();
    space.CopyFromSpace(GetThreadPool());

    stats.smallGarbageSize = space.FromRegionSize() - space.ToSpaceSize();
}

void MarkingCollector::ExemptFromSpace()
{
    RegionalHeap& space = reinterpret_cast<RegionalHeap&>(theAllocator_);
    space.ExemptFromSpace();
}

} // namespace common
