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

#include <memory>

#include "libarkbase/os/cpu_affinity.h"
#include "libarkbase/os/mem.h"
#include "libarkbase/os/thread.h"
#include "libarkbase/utils/time.h"
#include "libarkbase/taskmanager/task_manager.h"
#include "runtime/assert_gc_scope.h"
#include "runtime/include/class.h"
#include "runtime/include/coretypes/dyn_objects.h"
#include "runtime/include/locks.h"
#include "runtime/include/runtime.h"
#include "runtime/include/runtime_notification.h"
#include "runtime/include/stack_walker-inl.h"
#include "runtime/include/safepoint_timer.h"
#include "runtime/mem/gc/epsilon/epsilon.h"
#include "runtime/mem/gc/epsilon-g1/epsilon-g1.h"
#include "runtime/mem/gc/gc.h"
#include "runtime/mem/gc/gc_root-inl.h"
#include "runtime/mem/gc/g1/g1-gc.h"
#include "runtime/mem/gc/stw-gc/stw-gc.h"
#include "runtime/mem/gc/cmc-gc-adapter/cmc-gc-adapter.h"
#include "runtime/mem/gc/workers/gc_workers_task_queue.h"
#include "runtime/mem/gc/workers/gc_workers_thread_pool.h"
#include "runtime/mem/pygote_space_allocator-inl.h"
#include "runtime/mem/heap_manager.h"
#include "runtime/mem/gc/reference-processor/reference_processor.h"
#include "runtime/mem/gc/gc-hung/gc_hung.h"
#include "runtime/include/panda_vm.h"
#include "runtime/include/object_accessor-inl.h"
#include "runtime/include/coretypes/class.h"
#include "runtime/thread_manager.h"
#include "runtime/mem/gc/gc_adaptive_stack_inl.h"
#include "libarkbase/utils/utils.h"
#include "runtime/trace.h"

namespace ark::mem {
using TaggedValue = coretypes::TaggedValue;
using TaggedType = coretypes::TaggedType;
using DynClass = coretypes::DynClass;

GC::GC(ObjectAllocatorBase *objectAllocator, const GCSettings &settings)
    : gcSettings_(settings),
      objectAllocator_(objectAllocator),
      internalAllocator_(InternalAllocator<>::GetInternalAllocatorFromRuntime())
{
    if (gcSettings_.UseTaskManagerForGC()) {
        // Create gc task queue for task manager
        gcWorkersTaskQueue_ = taskmanager::TaskManager::CreateTaskQueue<decltype(internalAllocator_->Adapter())>(
            taskmanager::MAX_QUEUE_PRIORITY);
        ASSERT(gcWorkersTaskQueue_ != nullptr);
    }
}

GC::~GC()
{
    InternalAllocatorPtr allocator = GetInternalAllocator();
    if (gcWorker_ != nullptr) {
        allocator->Delete(gcWorker_);
    }
    if (gcListenerManager_ != nullptr) {
        allocator->Delete(gcListenerManager_);
    }
    if (gcBarrierSet_ != nullptr) {
        allocator->Delete(gcBarrierSet_);
    }
    if (clearedReferences_ != nullptr) {
        allocator->Delete(clearedReferences_);
    }
    if (clearedReferencesLock_ != nullptr) {
        allocator->Delete(clearedReferencesLock_);
    }
    if (workersTaskPool_ != nullptr) {
        allocator->Delete(workersTaskPool_);
    }
    if (gcWorkersTaskQueue_ != nullptr) {
        taskmanager::TaskManager::DestroyTaskQueue<decltype(allocator->Adapter())>(gcWorkersTaskQueue_);
    }
}

Logger::Buffer GC::GetLogPrefix() const
{
    const char *phase = GCScopedPhase::GetPhaseAbbr(GetGCPhase());
    // Atomic with acquire order reason: data race with gc_counter_
    size_t counter = gcCounter_.load(std::memory_order_acquire);

    Logger::Buffer buffer;
    // NOLINTNEXTLINE(cppcoreguidelines-pro-type-vararg)
    buffer.Printf("[%zu, %s]: ", counter, phase);

    return buffer;
}

GCType GC::GetType()
{
    return gcType_;
}

void GC::SetPandaVM(PandaVM *vm)
{
    vm_ = vm;
    referenceProcessor_ = vm->GetReferenceProcessor();
}

NativeGcTriggerType GC::GetNativeGcTriggerType()
{
    return gcSettings_.GetNativeGcTriggerType();
}

size_t GC::SimpleNativeAllocationGcWatermark()
{
    return GetPandaVm()->GetOptions().GetMaxFree();
}

NO_THREAD_SAFETY_ANALYSIS void GC::WaitForIdleGC()
{
    while (!CASGCPhase(GCPhase::GC_PHASE_IDLE, GCPhase::GC_PHASE_RUNNING)) {
        GetPandaVm()->GetRendezvous()->SafepointEnd();
        // Interrupt the running GC if possible
        OnWaitForIdleFail();
        // NOTE(dtrubenkov): resolve it more properly
        constexpr uint64_t WAIT_FINISHED = 100;
        // Use NativeSleep for all threads, as this thread shouldn't hold Mutator lock here
        os::thread::NativeSleepUS(std::chrono::microseconds(WAIT_FINISHED));
        GetPandaVm()->GetRendezvous()->SafepointBegin();
    }
}

inline void GC::TriggerGCForNative()
{
    auto nativeGcTriggerType = GetNativeGcTriggerType();
    ASSERT_PRINT((nativeGcTriggerType == NativeGcTriggerType::NO_NATIVE_GC_TRIGGER) ||
                     (nativeGcTriggerType == NativeGcTriggerType::SIMPLE_STRATEGY),
                 "Unknown Native GC Trigger type");
    switch (nativeGcTriggerType) {
        case NativeGcTriggerType::NO_NATIVE_GC_TRIGGER:
            break;
        case NativeGcTriggerType::SIMPLE_STRATEGY:
            // Atomic with relaxed order reason: data race with native_bytes_registered_ with no synchronization or
            // ordering constraints imposed on other reads or writes
            if (nativeBytesRegistered_.load(std::memory_order_relaxed) > SimpleNativeAllocationGcWatermark()) {
                auto task = MakePandaUnique<GCTask>(GCTaskCause::NATIVE_ALLOC_CAUSE, time::GetCurrentTimeInNanos());
                AddGCTask(false, std::move(task));
                ManagedThread::GetCurrent()->SafepointPoll();
            }
            break;
        default:
            LOG(FATAL, GC) << "Unknown Native GC Trigger type";
            break;
    }
}

void GC::Initialize(PandaVM *vm)
{
    trace::ScopedTrace scopedTrace(__PRETTY_FUNCTION__);
    // GC saved the PandaVM instance, so we get allocator from the PandaVM.
    auto allocator = GetInternalAllocator();
    gcListenerManager_ = allocator->template New<GCListenerManager>();
    clearedReferencesLock_ = allocator->New<os::memory::Mutex>();
    ASSERT(clearedReferencesLock_ != nullptr);
    os::memory::LockHolder holder(*clearedReferencesLock_);
    clearedReferences_ = allocator->New<PandaVector<ark::mem::Reference *>>(allocator->Adapter());
    this->SetPandaVM(vm);
    InitializeImpl();
    gcWorker_ = allocator->New<GCWorker>(this);
}

void GC::CreateWorkersTaskPool()
{
    ASSERT(workersTaskPool_ == nullptr);
    if (this->IsWorkerThreadsExist()) {
        auto allocator = GetInternalAllocator();
        GCWorkersTaskPool *gcTaskPool = nullptr;
        if (this->GetSettings()->UseThreadPoolForGC()) {
            // Use internal gc thread pool
            gcTaskPool = allocator->New<GCWorkersThreadPool>(this, this->GetSettings()->GCWorkersCount());
        } else {
            // Use common TaskManager
            ASSERT(this->GetSettings()->UseTaskManagerForGC());
            gcTaskPool = allocator->New<GCWorkersTaskQueue>(this);
        }
        ASSERT(gcTaskPool != nullptr);
        workersTaskPool_ = gcTaskPool;
    }
}

void GC::DestroyWorkersTaskPool()
{
    if (workersTaskPool_ == nullptr) {
        return;
    }
    workersTaskPool_->WaitUntilTasksEnd();
    auto allocator = this->GetInternalAllocator();
    allocator->Delete(workersTaskPool_);
    workersTaskPool_ = nullptr;
}

void GC::StartGC()
{
    CreateWorker();
}

void GC::StopGC()
{
    DestroyWorker();
    DestroyWorkersTaskPool();
}

void GC::SetupCpuAffinity()
{
    if (!gcSettings_.ManageGcThreadsAffinity()) {
        return;
    }
    // Try to get CPU affinity fo GC Thread
    if (UNLIKELY(!os::CpuAffinityManager::GetCurrentThreadAffinity(affinityBeforeGc_))) {
        affinityBeforeGc_.Clear();
        return;
    }
    // Try to use best + middle for preventing issues when best core is used in another thread,
    // and GC waits for it to finish.
    if (!os::CpuAffinityManager::SetAffinityForCurrentThread(os::CpuPower::BEST | os::CpuPower::MIDDLE)) {
        affinityBeforeGc_.Clear();
    }
    // Some GCs don't use GC Workers
    if (workersTaskPool_ != nullptr && this->GetSettings()->UseThreadPoolForGC()) {
        static_cast<GCWorkersThreadPool *>(workersTaskPool_)->SetAffinityForGCWorkers();
    }
}

void GC::SetupCpuAffinityAfterConcurrent()
{
    if (!gcSettings_.ManageGcThreadsAffinity()) {
        return;
    }
    os::CpuAffinityManager::SetAffinityForCurrentThread(os::CpuPower::BEST | os::CpuPower::MIDDLE);
    // Some GCs don't use GC Workers
    if (workersTaskPool_ != nullptr && this->GetSettings()->UseThreadPoolForGC()) {
        static_cast<GCWorkersThreadPool *>(workersTaskPool_)->SetAffinityForGCWorkers();
    }
}

void GC::ResetCpuAffinity(bool beforeConcurrent)
{
    if (!gcSettings_.ManageGcThreadsAffinity()) {
        return;
    }
    if (!affinityBeforeGc_.IsEmpty()) {
        // Set GC Threads on weak CPUs before concurrent if needed
        if (beforeConcurrent && gcSettings_.UseWeakCpuForGcConcurrent()) {
            os::CpuAffinityManager::SetAffinityForCurrentThread(os::CpuPower::WEAK);
        } else {  // else set on saved affinity
            os::CpuAffinityManager::SetAffinityForCurrentThread(affinityBeforeGc_);
        }
    }
    // Some GCs don't use GC Workers
    if (workersTaskPool_ != nullptr && this->GetSettings()->UseThreadPoolForGC()) {
        static_cast<GCWorkersThreadPool *>(workersTaskPool_)->UnsetAffinityForGCWorkers();
    }
}

void GC::SetupCpuAffinityBeforeConcurrent()
{
    ResetCpuAffinity(true);
}

void GC::RestoreCpuAffinity()
{
    ResetCpuAffinity(false);
}

bool GC::GetFastGCFlag() const
{
    // Atomic with relaxed order reason: data race with no synchronization or ordering constraints imposed
    // on other reads or writes
    return fastGC_.load(std::memory_order_relaxed);
}

void GC::SetFastGCFlag(bool fastGC)
{
    // Atomic with relaxed order reason: data race with no synchronization or ordering constraints imposed
    // on other reads or writes
    fastGC_.store(fastGC, std::memory_order_relaxed);
}

bool GC::NeedRunGCAfterWaiting(size_t counterBeforeWaiting, const GCTask &task) const
{
    // Atomic with acquire order reason: data race with gc_counter_ with dependecies on reads after the load which
    // should become visible
    auto newCounter = gcCounter_.load(std::memory_order_acquire);
    ASSERT(newCounter >= counterBeforeWaiting);
    bool shouldRunGCAccordingToFlag = !(GetFastGCFlag() && task.reason == GCTaskCause::HEAP_USAGE_THRESHOLD_CAUSE);
    // Atomic with acquire order reason: data race with last_cause_ with dependecies on reads after the load which
    // should become visible
    return (newCounter == counterBeforeWaiting || lastCause_.load(std::memory_order_acquire) < task.reason) &&
           shouldRunGCAccordingToFlag;
}

bool GC::GCPhasesPreparation(const GCTask &task)
{
    // Atomic with acquire order reason: data race with gc_counter_ with dependecies on reads after the load which
    // should become visible
    auto oldCounter = gcCounter_.load(std::memory_order_acquire);
    WaitForIdleGC();
    if (!this->NeedRunGCAfterWaiting(oldCounter, task)) {
        SetGCPhase(GCPhase::GC_PHASE_IDLE);
        return false;
    }
    this->SetupCpuAffinity();
    this->GetTiming()->Reset();  // Clear records.
    // Atomic with release order reason: data race with last_cause_ with dependecies on writes before the store which
    // should become visible acquire
    lastCause_.store(task.reason, std::memory_order_release);
    if (gcSettings_.PreGCHeapVerification()) {
        trace::ScopedTrace preHeapVerifierTrace("PreGCHeapVeriFier");
        size_t failCount = VerifyHeap();
        if (gcSettings_.FailOnHeapVerification() && failCount > 0) {
            LOG(FATAL, GC) << "Heap corrupted before GC, HeapVerifier found " << failCount << " corruptions";
        }
    }
    // Atomic with acq_rel order reason: data race with gc_counter_ with dependecies on reads after the load and on
    // writes before the store
    gcCounter_.fetch_add(1, std::memory_order_acq_rel);
    if (gcSettings_.IsDumpHeap()) {
        PandaOStringStream os;
        os << "Heap dump before GC" << std::endl;
        GetPandaVm()->DumpHeap(&os);
        std::cerr << os.str() << std::endl;
    }
    return true;
}

void GC::GCPhasesFinish(const GCTask &task)
{
    ASSERT(task.collectionType != GCCollectionType::NONE);
    LOG(INFO, GC) << "[" << gcCounter_ << "] [" << task.collectionType << " (" << task.reason << ")] "
                  << GetPandaVm()->GetGCStats()->GetStatistics();

    if (gcSettings_.IsDumpHeap()) {
        PandaOStringStream os;
        os << "Heap dump after GC" << std::endl;
        GetPandaVm()->DumpHeap(&os);
        std::cerr << os.str() << std::endl;
    }

    if (gcSettings_.PostGCHeapVerification()) {
        trace::ScopedTrace postHeapVerifierTrace("PostGCHeapVeriFier");
        size_t failCount = VerifyHeap();
        if (gcSettings_.FailOnHeapVerification() && failCount > 0) {
            LOG(FATAL, GC) << "Heap corrupted after GC, HeapVerifier found " << failCount << " corruptions";
        }
    }
    this->RestoreCpuAffinity();

    SetGCPhase(GCPhase::GC_PHASE_IDLE);
}

// NOLINTNEXTLINE(performance-unnecessary-value-param)
void GC::RunPhases(GCTask &task)
{
    DCHECK_ALLOW_GARBAGE_COLLECTION;
    trace::ScopedTrace scopedTrace(__FUNCTION__);
    bool needRunGCAfterWaiting = GCPhasesPreparation(task);
    if (!needRunGCAfterWaiting) {
        return;
    }
    size_t bytesInHeapBeforeGc = GetPandaVm()->GetMemStats()->GetFootprintHeap();
    LOG_DEBUG_GC << "Bytes in heap before GC " << std::dec << bytesInHeapBeforeGc;
    {
        GCScopedStats scopedStats(GetPandaVm()->GetGCStats(), gcType_ == GCType::STW_GC ? GetStats() : nullptr);
        ScopedGcHung scopedHung(&task);
        GetPandaVm()->GetGCStats()->ResetLastPause();

        FireGCStarted(task, bytesInHeapBeforeGc);
        PreRunPhasesImpl();
        clearSoftReferencesEnabled_ = task.reason == GCTaskCause::OOM_CAUSE || IsExplicitFull(task);
        // NOLINTNEXTLINE(performance-unnecessary-value-param)
        RunPhasesImpl(task);
        // Clear Internal allocator unused pools (must do it on pause to avoid race conditions):
        // - Clear global part:
        InternalAllocator<>::GetInternalAllocatorFromRuntime()->VisitAndRemoveFreePools(
            [](void *mem, [[maybe_unused]] size_t size) { PoolManager::GetMmapMemPool()->FreePool(mem, size); });
        // - Clear local part:
        ClearLocalInternalAllocatorPools();

        size_t bytesInHeapAfterGc = GetPandaVm()->GetMemStats()->GetFootprintHeap();
        // There is case than bytes_in_heap_after_gc > 0 and bytes_in_heap_before_gc == 0.
        // Because TLABs are registered during GC
        if (bytesInHeapAfterGc > 0 && bytesInHeapBeforeGc > 0) {
            GetStats()->AddReclaimRatioValue(1 - static_cast<double>(bytesInHeapAfterGc) / bytesInHeapBeforeGc);
        }
        LOG_DEBUG_GC << "Bytes in heap after GC " << std::dec << bytesInHeapAfterGc;
        FireGCFinished(task, bytesInHeapBeforeGc, bytesInHeapAfterGc);
    }
    GCPhasesFinish(task);
}

template <class LanguageConfig>
GC *CreateGC(GCType gcType, ObjectAllocatorBase *objectAllocator, const GCSettings &settings)
{
    GC *ret = nullptr;
    InternalAllocatorPtr allocator {InternalAllocator<>::GetInternalAllocatorFromRuntime()};

    switch (gcType) {
        case GCType::EPSILON_GC:
            ret = allocator->New<EpsilonGC<LanguageConfig>>(objectAllocator, settings);
            break;
        case GCType::EPSILON_G1_GC:
            ret = allocator->New<EpsilonG1GC<LanguageConfig>>(objectAllocator, settings);
            break;
        case GCType::STW_GC:
            ret = allocator->New<StwGC<LanguageConfig>>(objectAllocator, settings);
            break;
        case GCType::G1_GC:
            ret = allocator->New<G1GC<LanguageConfig>>(objectAllocator, settings);
            break;
        case GCType::CMC_GC:
            ret = allocator->New<CMCGCAdapter<LanguageConfig>>(objectAllocator, settings);
            break;
        default:
            LOG(FATAL, GC) << "Unknown GC type";
            break;
    }
    return ret;
}

bool GC::CheckGCCause(GCTaskCause cause) const
{
    return cause != GCTaskCause::INVALID_CAUSE;
}

bool GC::IsMarkedEx(const ObjectHeader *object) const
{
    return IsMarked(object);
}

bool GC::MarkObjectIfNotMarked(ObjectHeader *objectHeader)
{
    ASSERT(objectHeader != nullptr);
    if (IsMarked(objectHeader)) {
        return false;
    }
    MarkObject(objectHeader);
    return true;
}

void GC::ProcessReference(GCMarkingStackType *objectsStack, const BaseClass *cls, const ObjectHeader *ref,
                          const ReferenceProcessPredicateT &pred)
{
    ASSERT(referenceProcessor_ != nullptr);
    referenceProcessor_->HandleReference(this, objectsStack, cls, ref, pred);
}

void GC::ProcessReferenceForSinglePassCompaction(const BaseClass *cls, const ObjectHeader *ref,
                                                 const ReferenceProcessorT &processor)
{
    ASSERT(referenceProcessor_ != nullptr);
    referenceProcessor_->HandleReference(this, cls, ref, processor);
}

void GC::AddReference(ObjectHeader *fromObj, ObjectHeader *object)
{
    ASSERT(IsMarked(object));
    GCMarkingStackType references(this);
    // NOTE(alovkov): support stack with workers here & put all refs in stack and only then process altogether for once
    ASSERT(!references.IsWorkersTaskSupported());
    references.PushToStack(fromObj, object);
    MarkReferences(&references, phase_);
    if (gcType_ != GCType::EPSILON_GC) {
        ASSERT(references.Empty());
    }
}

// NOLINTNEXTLINE(performance-unnecessary-value-param)
void GC::ProcessReferences(GCPhase gcPhase, const GCTask &task, const ReferenceClearPredicateT &pred)
{
    trace::ScopedTrace scopedTrace(__FUNCTION__);
    LOG(DEBUG, REF_PROC) << "Start processing cleared references";
    ASSERT(referenceProcessor_ != nullptr);
    bool clearSoftReferences = task.reason == GCTaskCause::OOM_CAUSE || IsExplicitFull(task);
    referenceProcessor_->ProcessReferences(false, clearSoftReferences, gcPhase, pred);
    Reference *processedRef = referenceProcessor_->CollectClearedReferences();
    if (processedRef != nullptr) {
        os::memory::LockHolder holder(*clearedReferencesLock_);
        // NOTE(alovkov): ged rid of cleared_references_ and just enqueue refs here?
        clearedReferences_->push_back(processedRef);
    }
}

void GC::ProcessReferences(const mem::GC::ReferenceClearPredicateT &pred)
{
    ASSERT(!this->IsFullGC());
    trace::ScopedTrace scopedTrace(__FUNCTION__);
    LOG(DEBUG, REF_PROC) << "Start processing cleared references";
    ASSERT(referenceProcessor_ != nullptr);
    referenceProcessor_->ProcessReferencesAfterCompaction(pred);
    Reference *processedRef = referenceProcessor_->CollectClearedReferences();
    if (processedRef != nullptr) {
        os::memory::LockHolder holder(*clearedReferencesLock_);
        clearedReferences_->push_back(processedRef);
    }
}

void GC::EvacuateStartingWith([[maybe_unused]] void *ref)
{
    ASSERT_PRINT(false, "Should be implemented by subclasses");
}

bool GC::IsClearSoftReferencesEnabled() const
{
    return clearSoftReferencesEnabled_;
}

void GC::SetGCPhase(GCPhase gcPhase)
{
    phase_ = gcPhase;
}

size_t GC::GetCounter() const
{
    return gcCounter_;
}

void GC::PostponeGCStart()
{
    ASSERT(IsPostponeGCSupported());
    isPostponeEnabled_ = true;
}

void GC::PostponeGCEnd()
{
    ASSERT(IsPostponeGCSupported());
    // Don't check IsPostponeEnabled because runtime can be created
    // during app launch. In this case PostponeGCStart is not called.
    isPostponeEnabled_ = false;
    gcWorker_->OnPostponeGCEnd();
}

bool GC::IsPostponeEnabled() const
{
    return isPostponeEnabled_;
}

void GC::DestroyWorker()
{
    // Atomic with seq_cst order reason: data race with gc_running_ with requirement for sequentially consistent order
    // where threads observe all modifications in the same order
    gcRunning_.store(false, std::memory_order_seq_cst);
    gcWorker_->FinalizeAndDestroyWorker();
}

void GC::CreateWorker()
{
    // Atomic with seq_cst order reason: data race with gc_running_ with requirement for sequentially consistent order
    // where threads observe all modifications in the same order
    gcRunning_.store(true, std::memory_order_seq_cst);
    ASSERT(gcWorker_ != nullptr);
    gcWorker_->CreateAndStartWorker();
}

void GC::DisableWorkerThreads()
{
    gcSettings_.SetGCWorkersCount(0);
    gcSettings_.SetParallelMarkingEnabled(false);
    gcSettings_.SetParallelCompactingEnabled(false);
    gcSettings_.SetParallelRefUpdatingEnabled(false);
}

void GC::EnableWorkerThreads()
{
    const RuntimeOptions &options = Runtime::GetOptions();
    gcSettings_.SetGCWorkersCount(options.GetGcWorkersCount());
    gcSettings_.SetParallelMarkingEnabled(options.IsGcParallelMarkingEnabled() && (options.GetGcWorkersCount() != 0));
    gcSettings_.SetParallelCompactingEnabled(options.IsGcParallelCompactingEnabled() &&
                                             (options.GetGcWorkersCount() != 0));
    gcSettings_.SetParallelRefUpdatingEnabled(options.IsGcParallelRefUpdatingEnabled() &&
                                              (options.GetGcWorkersCount() != 0));
}

void GC::PreZygoteFork()
{
    DestroyWorker();
    if (gcSettings_.UseTaskManagerForGC()) {
        ASSERT(gcWorkersTaskQueue_ != nullptr);
        ASSERT(gcWorkersTaskQueue_->IsEmpty());
    }
}

void GC::PostZygoteFork()
{
    CreateWorker();
}

class GC::PostForkGCTask : public GCTask {
public:
    PostForkGCTask(GCTaskCause gcReason, uint64_t gcTargetTime, size_t restoreLimit)
        : GCTask(gcReason, gcTargetTime), restoreLimit_(restoreLimit)
    {
    }

    void Run(mem::GC &gc) override
    {
        LOG(DEBUG, GC) << "Runing PostForkGCTask";
        gc.GetPandaVm()->GetGCTrigger()->RestoreMinTargetFootprint();
        gc.PostForkCallback(restoreLimit_);
        GCTask::Run(gc);
    }

    ~PostForkGCTask() override = default;

    NO_COPY_SEMANTIC(PostForkGCTask);
    NO_MOVE_SEMANTIC(PostForkGCTask);

private:
    // For g1gc, it saves young space original size.
    size_t restoreLimit_ {0};
};

void GC::PreStartup()
{
    // Add a delay GCTask.
    if ((!Runtime::GetCurrent()->IsZygote()) && (!gcSettings_.RunGCInPlace())) {
        // divide 4 to temporarily set target footprint to a high value to disable GC during App startup.
        size_t startupLimit = Runtime::GetOptions().GetHeapSizeLimit() / 4;
        GetPandaVm()->GetGCTrigger()->SetMinTargetFootprint(startupLimit);
        PreStartupImp();
        size_t originSize = AdujustStartupLimit(startupLimit);
        constexpr uint64_t DISABLE_GC_DURATION_NS = 3000ULL;
        auto task = MakePandaUnique<PostForkGCTask>(GCTaskCause::STARTUP_COMPLETE_CAUSE,
                                                    time::GetCurrentTimeInNanos() + DISABLE_GC_DURATION_NS, originSize);
        AddGCTask(true, std::move(task));
        LOG(DEBUG, GC) << "Add PostForkGCTask";
    }
}

#ifdef SAFEPOINT_TIME_CHECKER_ENABLED
namespace {
class NonRecordingScopedSafepointTimer final {
public:
    explicit NonRecordingScopedSafepointTimer(bool isManaged) : isManaged_ {isManaged}
    {
        if (isManaged_) {
            SafepointTimerTable::ResetTimers(ManagedThread::GetCurrent()->GetInternalId(), true);
        }
    }

    ~NonRecordingScopedSafepointTimer()
    {
        if (isManaged_) {
            SafepointTimerTable::ResetTimers(ManagedThread::GetCurrent()->GetInternalId(), false);
        }
    }

private:
    bool isManaged_;
};
}  // unnamed namespace
#endif  // SAFEPOINT_TIME_CHECKER_ENABLED

// NOLINTNEXTLINE(performance-unnecessary-value-param)
bool GC::AddGCTask(bool isManaged, PandaUniquePtr<GCTask> task)
{
#ifdef SAFEPOINT_TIME_CHECKER_ENABLED
    NonRecordingScopedSafepointTimer scopedTimer {isManaged};
#endif
    bool triggeredByThreshold = (task->reason == GCTaskCause::HEAP_USAGE_THRESHOLD_CAUSE);
    if (gcSettings_.RunGCInPlace()) {
        auto *gcTask = task.get();
        if (IsGCRunning()) {
            if (isManaged) {
                return WaitForGCInManaged(*gcTask);
            }
            return WaitForGC(*gcTask);
        }
    } else {
        if (triggeredByThreshold) {
            bool expect = true;
            if (canAddGcTask_.compare_exchange_strong(expect, false, std::memory_order_seq_cst)) {
                return gcWorker_->AddTask(std::move(task));
            }
        } else {
            return gcWorker_->AddTask(std::move(task));
        }
    }
    return false;
}

bool GC::IsReference(const BaseClass *cls, const ObjectHeader *ref, const ReferenceCheckPredicateT &pred)
{
    ASSERT(referenceProcessor_ != nullptr);
    return referenceProcessor_->IsReference(cls, ref, pred);
}

void GC::EnqueueReferences()
{
    while (true) {
        ark::mem::Reference *ref = nullptr;
        {
            os::memory::LockHolder holder(*clearedReferencesLock_);
            if (clearedReferences_->empty()) {
                break;
            }
            ref = clearedReferences_->back();
            clearedReferences_->pop_back();
        }
        ASSERT(ref != nullptr);
        ASSERT(referenceProcessor_ != nullptr);
        referenceProcessor_->ScheduleForEnqueue(ref);
    }
}

bool GC::IsFullGC() const
{
    // Atomic with relaxed order reason: data race with is_full_gc_ with no synchronization or ordering
    // constraints imposed on other reads or writes
    return isFullGc_.load(std::memory_order_relaxed);
}

void GC::SetFullGC(bool value)
{
    // Atomic with relaxed order reason: data race with is_full_gc_ with no synchronization or ordering
    // constraints imposed on other reads or writes
    isFullGc_.store(value, std::memory_order_relaxed);
}

void GC::NotifyNativeAllocations()
{
    // Atomic with relaxed order reason: data race with native_objects_notified_ with no synchronization or ordering
    // constraints imposed on other reads or writes
    nativeObjectsNotified_.fetch_add(NOTIFY_NATIVE_INTERVAL, std::memory_order_relaxed);
    TriggerGCForNative();
}

void GC::RegisterNativeAllocation(size_t bytes)
{
    ASSERT_NATIVE_CODE();
    size_t allocated;
    do {
        // Atomic with relaxed order reason: data race with native_bytes_registered_ with no synchronization or ordering
        // constraints imposed on other reads or writes
        allocated = nativeBytesRegistered_.load(std::memory_order_relaxed);
    } while (!nativeBytesRegistered_.compare_exchange_weak(allocated, allocated + bytes));
    if (allocated > std::numeric_limits<size_t>::max() - bytes) {
        // Atomic with relaxed order reason: data race with native_bytes_registered_ with no synchronization or ordering
        // constraints imposed on other reads or writes
        nativeBytesRegistered_.store(std::numeric_limits<size_t>::max(), std::memory_order_relaxed);
    }
    TriggerGCForNative();
}

void GC::RegisterNativeFree(size_t bytes)
{
    size_t allocated;
    size_t newFreedBytes;
    do {
        // Atomic with relaxed order reason: data race with native_bytes_registered_ with no synchronization or ordering
        // constraints imposed on other reads or writes
        allocated = nativeBytesRegistered_.load(std::memory_order_relaxed);
        newFreedBytes = std::min(allocated, bytes);
    } while (!nativeBytesRegistered_.compare_exchange_weak(allocated, allocated - newFreedBytes));
}

size_t GC::GetNativeBytesFromMallinfoAndRegister() const
{
    size_t mallinfoBytes = ark::os::mem::GetNativeBytesFromMallinfo();
    // Atomic with relaxed order reason: data race with native_bytes_registered_ with no synchronization or ordering
    // constraints imposed on other reads or writes
    size_t allBytes = mallinfoBytes + nativeBytesRegistered_.load(std::memory_order_relaxed);
    return allBytes;
}

bool GC::WaitForGC(GCTask task)
{
    // NOTE(maksenov): Notify only about pauses (#4681)
    Runtime::GetCurrent()->GetNotificationManager()->GarbageCollectorStartEvent();
    // Atomic with acquire order reason: data race with gc_counter_ with dependecies on reads after the load which
    // should become visible
    auto oldCounter = this->gcCounter_.load(std::memory_order_acquire);
    Timing suspendThreadsTiming;
    {
        ScopedTiming t("SuspendThreads", suspendThreadsTiming);
        Tracer::Start("GC::WaitForGC");
        this->GetPandaVm()->GetRendezvous()->SafepointBegin();
    }
    if (!this->NeedRunGCAfterWaiting(oldCounter, task)) {
        this->GetPandaVm()->GetRendezvous()->SafepointEnd();
        Tracer::Finish();
        return false;
    }

    // Create a copy of the constant GCTask to be able to change its value
    Tracer::Start("RunPhases");
    this->RunPhases(task);
    Tracer::Finish();

    if (UNLIKELY(this->IsLogDetailedGcInfoEnabled())) {
        PrintDetailedLog();
    }

    this->GetPandaVm()->GetRendezvous()->SafepointEnd();
    Tracer::Finish();
    Runtime::GetCurrent()->GetNotificationManager()->GarbageCollectorFinishEvent();
    this->GetPandaVm()->HandleGCFinished();
    this->GetPandaVm()->HandleEnqueueReferences();
    this->GetPandaVm()->ProcessReferenceFinalizers();
    return true;
}

bool GC::WaitForGCInManaged(const GCTask &task)
{
    Thread *baseThread = Thread::GetCurrent();
    if (ManagedThread::ThreadIsManagedThread(baseThread)) {
        ManagedThread *thread = ManagedThread::CastFromThread(baseThread);
        ASSERT(thread->GetMutatorLock()->HasLock());
        [[maybe_unused]] bool isDaemon = MTManagedThread::ThreadIsMTManagedThread(baseThread) &&
                                         MTManagedThread::CastFromThread(baseThread)->IsDaemon();
        ASSERT(!isDaemon || thread->GetStatus() == ThreadStatus::RUNNING);
        SAFEPOINT_TIME_CHECKER(SafepointTimerTable::ResetTimers(thread->GetInternalId(), true));
        vm_->GetMutatorLock()->Unlock();
        thread->PrintSuspensionStackIfNeeded();
        WaitForGC(task);
        vm_->GetMutatorLock()->ReadLock();
        SAFEPOINT_TIME_CHECKER(SafepointTimerTable::ResetTimers(thread->GetInternalId(), false));
        ASSERT(vm_->GetMutatorLock()->HasLock());
        this->GetPandaVm()->HandleGCRoutineInMutator();
        return true;
    }
    return false;
}

void GC::StartConcurrentScopeRoutine() const {}

void GC::EndConcurrentScopeRoutine() const {}

void GC::PrintDetailedLog()
{
    for (auto &footprint : this->footprintList_) {
        LOG(INFO, GC) << footprint.first << " : " << footprint.second;
    }
    LOG(INFO, GC) << this->GetTiming()->Dump();
}

ConcurrentScope::ConcurrentScope(GC *gc, bool autoStart)
{
    LOG(DEBUG, GC) << "Start ConcurrentScope";
    Tracer::Start("GC ConcurrentScope");
    gc_ = gc;
    if (autoStart) {
        Start();
    }
}

ConcurrentScope::~ConcurrentScope()
{
    LOG(DEBUG, GC) << "Stop ConcurrentScope";
    if (started_ && gc_->IsConcurrencyAllowed()) {
        gc_->GetPandaVm()->GetRendezvous()->SafepointBegin();
        gc_->SetupCpuAffinityAfterConcurrent();
        gc_->EndConcurrentScopeRoutine();
    }
    Tracer::Finish();
}

NO_THREAD_SAFETY_ANALYSIS void ConcurrentScope::Start()
{
    if (!started_ && gc_->IsConcurrencyAllowed()) {
        gc_->StartConcurrentScopeRoutine();
        gc_->SetupCpuAffinityBeforeConcurrent();
        gc_->GetPandaVm()->GetRendezvous()->SafepointEnd();
        started_ = true;
    }
}

void GC::WaitForGCOnPygoteFork(const GCTask &task)
{
    // do nothing if no pygote space
    auto pygoteSpaceAllocator = objectAllocator_->GetPygoteSpaceAllocator();
    if (pygoteSpaceAllocator == nullptr) {
        return;
    }

    // do nothing if not at first pygote fork
    if (pygoteSpaceAllocator->GetState() != PygoteSpaceState::STATE_PYGOTE_INIT) {
        return;
    }

    LOG(DEBUG, GC) << "== GC WaitForGCOnPygoteFork Start ==";

    // do we need a lock?
    // looks all other threads have been stopped before pygote fork

    // 0. indicate that we're rebuilding pygote space
    pygoteSpaceAllocator->SetState(PygoteSpaceState::STATE_PYGOTE_FORKING);

    // 1. trigger gc
    WaitForGC(task);

    // 2. move other space to pygote space
    MoveObjectsToPygoteSpace();

    // 3. indicate that we have done
    pygoteSpaceAllocator->SetState(PygoteSpaceState::STATE_PYGOTE_FORKED);

    // 4. disable pygote for allocation
    objectAllocator_->DisablePygoteAlloc();

    LOG(DEBUG, GC) << "== GC WaitForGCOnPygoteFork End ==";
}

bool GC::IsOnPygoteFork() const
{
    auto pygoteSpaceAllocator = objectAllocator_->GetPygoteSpaceAllocator();
    return pygoteSpaceAllocator != nullptr &&
           pygoteSpaceAllocator->GetState() == PygoteSpaceState::STATE_PYGOTE_FORKING;
}

void GC::MoveObjectsToPygoteSpace()
{
    trace::ScopedTrace scopedTrace(__FUNCTION__);
    LOG(DEBUG, GC) << "MoveObjectsToPygoteSpace: start";

    size_t allSizeMove = 0;
    size_t movedObjectsNum = 0;
    size_t bytesInHeapBeforeMove = GetPandaVm()->GetMemStats()->GetFootprintHeap();
    auto pygoteSpaceAllocator = objectAllocator_->GetPygoteSpaceAllocator();
    ObjectVisitor moveVisitor([this, &pygoteSpaceAllocator, &movedObjectsNum, &allSizeMove](ObjectHeader *src) -> void {
        size_t size = GetObjectSize(src);
        auto dst = reinterpret_cast<ObjectHeader *>(pygoteSpaceAllocator->Alloc(size));
        ASSERT(dst != nullptr);
        MemcpyUnsafe(dst, src, size);
        allSizeMove += size;
        movedObjectsNum++;
        SetForwardAddress(src, dst);
        LOG_DEBUG_GC << "object MOVED from " << std::hex << src << " to " << dst << ", size = " << std::dec << size;
    });

    // move all small movable objects to pygote space
    objectAllocator_->IterateRegularSizeObjects(moveVisitor);

    LOG(DEBUG, GC) << "MoveObjectsToPygoteSpace: move_num = " << movedObjectsNum << ", move_size = " << allSizeMove;

    if (allSizeMove > 0) {
        GetStats()->AddMemoryValue(allSizeMove, MemoryTypeStats::MOVED_BYTES);
        GetStats()->AddObjectsValue(movedObjectsNum, ObjectTypeStats::MOVED_OBJECTS);
    }
    if (bytesInHeapBeforeMove > 0) {
        GetStats()->AddCopiedRatioValue(static_cast<double>(allSizeMove) / bytesInHeapBeforeMove);
    }

    // Update because we moved objects from object_allocator -> pygote space
    UpdateRefsToMovedObjectsInPygoteSpace();
    CommonUpdateAndSweepVmRefs();

    // Clear the moved objects in old space
    objectAllocator_->FreeObjectsMovedToPygoteSpace();

    LOG(DEBUG, GC) << "MoveObjectsToPygoteSpace: finish";
}

void GC::SetForwardAddress(ObjectHeader *src, ObjectHeader *dst)
{
    auto baseCls = src->ClassAddr<BaseClass>();
    if (baseCls->IsDynamicClass()) {
        auto cls = static_cast<HClass *>(baseCls);
        // Note: During moving phase, 'src => dst'. Consider the src is a DynClass,
        //       since 'dst' is not in GC-status the 'manage-object' inside 'dst' won't be updated to
        //       'dst'. To fix it, we update 'manage-object' here rather than upating phase.
        if (cls->IsHClass()) {
            size_t offset = ObjectHeader::ObjectHeaderSize() + HClass::GetManagedObjectOffset();
            dst->SetFieldObject<false, false, true>(GetPandaVm()->GetAssociatedThread(), offset, dst);
        }
    }

    // Set fwd address in src
    bool updateRes = false;
    do {
        MarkWord markWord = src->AtomicGetMark();
        MarkWord fwdMarkWord =
            markWord.DecodeFromForwardingAddress(static_cast<MarkWord::MarkWordSize>(ToUintPtr(dst)));
        updateRes = src->AtomicSetMark<false>(markWord, fwdMarkWord);
    } while (!updateRes);
}

const ObjectHeader *GC::PopObjectFromStack(GCMarkingStackType *objectsStack)
{
    auto *object = objectsStack->PopFromStack();
    ASSERT(object != nullptr);
    return object;
}

bool GC::IsGenerational() const
{
    return IsGenerationalGCType(gcType_);
}

void GC::GCListenerManager::AddListener(GCListener *listener)
{
    os::memory::LockHolder lh(listenerLock_);
    newListeners_.push_back(listener);
}

void GC::GCListenerManager::RemoveListener(GCListener *listener)
{
    os::memory::LockHolder lh(listenerLock_);
    listenersForRemove_.push_back(listener);
}

void GC::GCListenerManager::NormalizeListenersOnStartGC()
{
    os::memory::LockHolder lh(listenerLock_);
    for (auto *listenerForRemove : listenersForRemove_) {
        auto it = std::find(newListeners_.begin(), newListeners_.end(), listenerForRemove);
        if (it != newListeners_.end()) {
            newListeners_.erase(it);
        }
        it = std::find(currentListeners_.begin(), currentListeners_.end(), listenerForRemove);
        if (it != currentListeners_.end()) {
            LOG(DEBUG, GC) << "Remove listener for GC: " << listenerForRemove;
            currentListeners_.erase(it);
        }
    }
    listenersForRemove_.clear();
    for (auto *newListener : newListeners_) {
        LOG(DEBUG, GC) << "Add new listener for GC: " << newListener;
        currentListeners_.push_back(newListener);
    }
    newListeners_.clear();
}

void GC::FireGCStarted(const GCTask &task, size_t bytesInHeapBeforeGc)
{
    gcListenerManager_->NormalizeListenersOnStartGC();
    gcListenerManager_->IterateOverListeners(
        [&](GCListener *listener) { listener->GCStarted(task, bytesInHeapBeforeGc); });
}

void GC::FireGCFinished(const GCTask &task, size_t bytesInHeapBeforeGc, size_t bytesInHeapAfterGc)
{
    gcListenerManager_->IterateOverListeners(
        [&](GCListener *listener) { listener->GCFinished(task, bytesInHeapBeforeGc, bytesInHeapAfterGc); });
}

void GC::FireGCPhaseStarted(GCPhase phase)
{
    gcListenerManager_->IterateOverListeners([phase](GCListener *listener) { listener->GCPhaseStarted(phase); });
}

void GC::FireGCPhaseFinished(GCPhase phase)
{
    gcListenerManager_->IterateOverListeners([phase](GCListener *listener) { listener->GCPhaseFinished(phase); });
}

void GC::OnWaitForIdleFail() {}

TEMPLATE_GC_CREATE_GC();

}  // namespace ark::mem
