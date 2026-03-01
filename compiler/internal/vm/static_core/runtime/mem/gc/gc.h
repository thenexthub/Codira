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
#ifndef PANDA_RUNTIME_MEM_GC_GC_H
#define PANDA_RUNTIME_MEM_GC_GC_H

#include <atomic>
#include <map>
#include <string_view>
#include <vector>

#include "libarkbase/os/cpu_affinity.h"
#include "libarkbase/os/mutex.h"
#include "libarkbase/os/thread.h"
#include "libarkbase/taskmanager/task_queue.h"
#include "libarkbase/trace/trace.h"
#include "libarkbase/utils/expected.h"
#include "runtime/include/gc_task.h"
#include "runtime/include/object_header.h"
#include "runtime/include/language_config.h"
#include "runtime/include/locks.h"
#include "runtime/include/mem/panda_containers.h"
#include "runtime/include/mem/panda_smart_pointers.h"
#include "runtime/include/mem/panda_string.h"
#include "runtime/mem/allocator_adapter.h"
#include "runtime/mem/gc/gc_settings.h"
#include "runtime/mem/gc/gc_barrier_set.h"
#include "runtime/mem/gc/gc_phase.h"
#include "runtime/mem/gc/gc_root.h"
#include "runtime/mem/gc/gc_adaptive_marking_stack.h"
#include "runtime/mem/gc/gc_scope.h"
#include "runtime/mem/gc/gc_scoped_phase.h"
#include "runtime/mem/gc/gc_stats.h"
#include "runtime/mem/gc/gc_types.h"
#include "runtime/mem/refstorage/reference.h"
#include "runtime/mem/gc/bitmap.h"
#include "runtime/mem/gc/workers/gc_worker.h"
#include "runtime/mem/object_helpers.h"
#include "runtime/timing.h"
#include "runtime/mem/region_allocator.h"

namespace ark {
class BaseClass;
class HClass;
class PandaVM;
class Timing;
namespace mem {
class G1GCTest;
class GlobalObjectStorage;
class ReferenceProcessor;
template <MTModeT MT_MODE>
class ObjectAllocatorG1;
namespace test {
class MemStatsGenGCTest;
class ReferenceStorageTest;
class RemSetTest;
}  // namespace test
namespace ecmascript {
class EcmaReferenceProcessor;
}  // namespace ecmascript
}  // namespace mem
}  // namespace ark

namespace ark::coretypes {
class Array;
class DynClass;
}  // namespace ark::coretypes

namespace ark::mem {

// NOLINTNEXTLINE(cppcoreguidelines-macro-usage)
#define LOG_DEBUG_GC LOG(DEBUG, GC) << this->GetLogPrefix()
// NOLINTNEXTLINE(cppcoreguidelines-macro-usage)
#define LOG_INFO_GC LOG(INFO, GC) << this->GetLogPrefix()
// NOLINTNEXTLINE(cppcoreguidelines-macro-usage)
#define LOG_DEBUG_OBJECT_EVENTS LOG(DEBUG, MM_OBJECT_EVENTS)

// forward declarations:
class GCListener;
class GCScopePhase;
class GCScopedPhase;
class GCQueueInterface;
class GCDynamicObjectHelpers;
class GCWorkersTaskPool;
class GCWorkersTask;

enum class GCError { GC_ERROR_NO_ROOTS, GC_ERROR_NO_FRAMES, GC_ERROR_LAST = GC_ERROR_NO_FRAMES };

enum ClassRootsVisitFlag : bool {
    ENABLED = true,
    DISABLED = false,
};

enum CardTableVisitFlag : bool {
    VISIT_ENABLED = true,
    VISIT_DISABLED = false,
};

enum BuffersKeepingFlag : bool {
    KEEP = true,
    DELETE = false,
};

class GCListener {
public:
    GCListener() = default;
    NO_COPY_SEMANTIC(GCListener);
    DEFAULT_MOVE_SEMANTIC(GCListener);
    virtual ~GCListener() = default;
    virtual void GCStarted([[maybe_unused]] const GCTask &task, [[maybe_unused]] size_t heapSize) {}
    virtual void GCFinished([[maybe_unused]] const GCTask &task, [[maybe_unused]] size_t heapSizeBeforeGc,
                            [[maybe_unused]] size_t heapSize)
    {
    }
    virtual void GCPhaseStarted([[maybe_unused]] GCPhase phase) {}
    virtual void GCPhaseFinished([[maybe_unused]] GCPhase phase) {}
};

class GCExtensionData;

using UpdateRefInObject = std::function<void(ObjectHeader *)>;

// base class for all GCs
class GC {
public:
    using MarkPreprocess = std::function<void(const ObjectHeader *, BaseClass *)>;
    using ReferenceCheckPredicateT = std::function<bool(const ObjectHeader *)>;
    using ReferenceClearPredicateT = std::function<bool(const ObjectHeader *)>;
    using ReferenceProcessPredicateT = std::function<bool(const ObjectHeader *)>;
    using ReferenceProcessorT = std::function<void(void *)>;

    static constexpr bool EmptyReferenceProcessPredicate([[maybe_unused]] const ObjectHeader *ref)
    {
        return true;
    }

    static constexpr void EmptyMarkPreprocess([[maybe_unused]] const ObjectHeader *ref,
                                              [[maybe_unused]] BaseClass *baseKlass)
    {
    }

    explicit GC(ObjectAllocatorBase *objectAllocator, const GCSettings &settings);
    NO_COPY_SEMANTIC(GC);
    NO_MOVE_SEMANTIC(GC);
    virtual ~GC() = 0;

    GCType GetType();

    /// @brief Initialize GC
    void Initialize(PandaVM *vm);

    /**
     * @brief Starts GC after initialization
     * Creates worker thread, sets gc_running_ to true
     */
    virtual void StartGC();

    /**
     * @brief Stops GC for runtime destruction
     * Joins GC thread, clears queue
     */
    virtual void StopGC();

    /**
     * Should be used to wait while GC should work exclusively
     * Note: for non-mt STW GC can be used to run GC.
     * @return false if the task is discarded. Otherwise true.
     * The task may be discarded if the GC already executing a task with
     * the same reason.
     */
    virtual bool WaitForGC(GCTask task);

    /**
     * Should be used to wait while GC should be executed in managed scope
     * @return false if the task is discarded. Otherwise true.
     * The task may be discarded if the GC already executing a task with
     * the same reason.
     */
    bool WaitForGCInManaged(const GCTask &task) NO_THREAD_SAFETY_ANALYSIS;

    /// Only be used to at first pygote fork
    void WaitForGCOnPygoteFork(const GCTask &task);

    bool IsOnPygoteFork() const;

    /**
     * Initialize GC bits on object creation.
     * Required only for GCs with switched bits
     */
    virtual void InitGCBits(ark::ObjectHeader *objHeader) = 0;

    /// Initialize GC bits on object creation for the TLAB allocation.
    virtual void InitGCBitsForAllocationInTLAB(ark::ObjectHeader *objHeader) = 0;

    bool IsTLABsSupported() const
    {
        return tlabsSupported_;
    }

    /// @return true if GC supports object pinning (will not move pinned object), false otherwise
    virtual bool IsPinningSupported() const = 0;

    /// @return true if cause is suitable for the GC, false otherwise
    virtual bool CheckGCCause(GCTaskCause cause) const;

    /**
     * Trigger GC.
     * @return false if the task is discarded. Otherwise true.
     * The task may be discarded if the GC already executing a task with
     * the same reason. The task may be discarded by other reasons.
     */
    virtual bool Trigger(PandaUniquePtr<GCTask> task) = 0;

    virtual bool IsFullGC() const;

    /// Return true if gc has generations, false otherwise
    bool IsGenerational() const;

    PandaString DumpStatistics()
    {
        return instanceStats_.GetDump(gcType_);
    }

    void AddListener(GCListener *listener)
    {
        ASSERT(gcListenerManager_ != nullptr);
        gcListenerManager_->AddListener(listener);
    }

    void RemoveListener(GCListener *listener)
    {
        ASSERT(gcListenerManager_ != nullptr);
        gcListenerManager_->RemoveListener(listener);
    }

    GCBarrierSet *GetBarrierSet()
    {
        ASSERT(gcBarrierSet_ != nullptr);
        return gcBarrierSet_;
    }

    GCWorkersTaskPool *GetWorkersTaskPool() const
    {
        ASSERT(workersTaskPool_ != nullptr);
        return workersTaskPool_;
    }

    // Additional NativeGC
    void NotifyNativeAllocations();

    void RegisterNativeAllocation(size_t bytes);

    void RegisterNativeFree(size_t bytes);

    int32_t GetNotifyNativeInterval()
    {
        return NOTIFY_NATIVE_INTERVAL;
    }

    // Calling CheckGCForNative immediately for every NOTIFY_NATIVE_INTERVAL allocations
    static constexpr int32_t NOTIFY_NATIVE_INTERVAL = 32;

    // Calling CheckGCForNative immediately if size exceeds the following
    static constexpr size_t CHECK_IMMEDIATELY_THRESHOLD = 300000;

    inline bool IsLogDetailedGcInfoEnabled() const
    {
        return gcSettings_.LogDetailedGCInfoEnabled();
    }

    inline bool IsLogDetailedGcCompactionInfoEnabled() const
    {
        return gcSettings_.LogDetailedGCCompactionInfoEnabled();
    }

    inline GCPhase GetGCPhase() const
    {
        return phase_;
    }

    inline GCTaskCause GetLastGCCause() const
    {
        // Atomic with acquire order reason: data race with another threads which can update the variable
        return lastCause_.load(std::memory_order_acquire);
    }

    inline bool IsGCRunning()
    {
        // Atomic with seq_cst order reason: data race with gc_running_ with requirement for sequentially consistent
        // order where threads observe all modifications in the same order
        return gcRunning_.load(std::memory_order_seq_cst);
    }

    void PreStartup();

    InternalAllocatorPtr GetInternalAllocator() const
    {
        return internalAllocator_;
    }

    /**
     * Enqueue all references in ReferenceQueue. Should be done after GC to avoid deadlock (lock in
     * ReferenceQueue.class)
     */
    void EnqueueReferences();

    /// Process all references which GC found in marking phase.
    void ProcessReferences(GCPhase gcPhase, const GCTask &task, const ReferenceClearPredicateT &pred);

    /// Process all references which were found during evacuation
    void ProcessReferences(const mem::GC::ReferenceClearPredicateT &pred);

    virtual void EvacuateStartingWith(void *ref);

    size_t GetNativeBytesRegistered()
    {
        // Atomic with relaxed order reason: data race with native_bytes_registered_ with no synchronization or ordering
        // constraints imposed on other reads or writes
        return nativeBytesRegistered_.load(std::memory_order_relaxed);
    }

    virtual void SetPandaVM(PandaVM *vm);

    PandaVM *GetPandaVm() const
    {
        return vm_;
    }

    taskmanager::TaskQueueInterface *GetWorkersTaskQueue() const
    {
        return gcWorkersTaskQueue_;
    }

    virtual void PreZygoteFork();

    virtual void PostZygoteFork();

    /**
     * Processes thread's remaining pre and post barrier buffer entries on its termination.
     *
     * @param keep_buffers specifies whether to clear (=BuffersKeepingFlag::KEEP) or deallocate
     * (=BuffersKeepingFlag::DELETE) pre and post barrier buffers upon OnThreadTerminate() completion
     */
    virtual void OnThreadTerminate([[maybe_unused]] ManagedThread *thread,
                                   [[maybe_unused]] mem::BuffersKeepingFlag keepBuffers)
    {
    }

    /// Performs the actions that are required upon thread creation (if any)
    virtual void OnThreadCreate([[maybe_unused]] ManagedThread *thread) {}

    void SetCanAddGCTask(bool canAddTask)
    {
        // Atomic with relaxed order reason: data race with can_add_gc_task_ with no synchronization or ordering
        // constraints imposed on other reads or writes
        canAddGcTask_.store(canAddTask, std::memory_order_relaxed);
    }

    bool CanAddGCTask() const
    {
        // Atomic with relaxed order reason: data race with can_add_gc_task_ with no synchronization or ordering
        // constraints imposed on other reads or writes
        return canAddGcTask_.load(std::memory_order_relaxed);
    }

    GCExtensionData *GetExtensionData() const
    {
        return extensionData_;
    }

    virtual void SetExtensionData(GCExtensionData *data)
    {
        extensionData_ = data;
    }

    virtual void PostForkCallback([[maybe_unused]] size_t restoreLimit) {}

    /// Check if the object addr is in the GC sweep range
    virtual bool InGCSweepRange([[maybe_unused]] const ObjectHeader *obj) const
    {
        return true;
    }

    virtual CardTable *GetCardTable() const
    {
        return nullptr;
    }

    /// Called from GCWorker thread to assign thread specific data
    virtual bool InitWorker(void **workerData)
    {
        *workerData = nullptr;
        return true;
    }

    /// Called from GCWorker thread to destroy thread specific data
    virtual void DestroyWorker([[maybe_unused]] void *workerData) {}

    /// Process a task sent to GC workers thread.
    virtual void WorkerTaskProcessing([[maybe_unused]] GCWorkersTask *task, [[maybe_unused]] void *workerData)
    {
        LOG(FATAL, GC) << "Unimplemented method";
    }

    virtual bool IsMutatorAllowed()
    {
        return false;
    }

    /// Return true of ref is an instance of reference or it's ancestor, false otherwise
    bool IsReference(const BaseClass *cls, const ObjectHeader *ref, const ReferenceCheckPredicateT &pred);

    void ProcessReference(GCMarkingStackType *objectsStack, const BaseClass *cls, const ObjectHeader *ref,
                          const ReferenceProcessPredicateT &pred);
    void ProcessReferenceForSinglePassCompaction(const BaseClass *cls, const ObjectHeader *ref,
                                                 const ReferenceProcessorT &processor);

    ALWAYS_INLINE ObjectAllocatorBase *GetObjectAllocator() const
    {
        return objectAllocator_;
    }

    // called if we fail change state from idle to running
    virtual void OnWaitForIdleFail();

    virtual void PendingGC() {}

    /**
     * Check if the object is marked for GC(alive)
     * @param object
     * @return true if object marked for GC
     */
    virtual bool IsMarked(const ObjectHeader *object) const = 0;

    /**
     * Check if the object is marked for GC(alive)
     * It is similar to IsMarked method but can contain additional GC specific logic.
     * If caller is not aware about GC mode it should use this method instead of IsMarked.
     * @param object
     * @return true if object marked for GC
     */
    virtual bool IsMarkedEx(const ObjectHeader *object) const;

    /**
     * Mark object.
     * Note: for some GCs it is not necessary set GC bit to 1.
     * @param object_header
     * @return true if object old state is not marked
     */
    virtual bool MarkObjectIfNotMarked(ObjectHeader *objectHeader);

    /**
     * Mark object.
     * Note: for some GCs it is not necessary set GC bit to 1.
     * @param object_header
     */
    virtual void MarkObject(ObjectHeader *objectHeader) = 0;

    /**
     * Add reference for later processing in marking phase
     * @param object - object from which we start to mark
     */
    void AddReference(ObjectHeader *fromObject, ObjectHeader *object);

    void SetGCPhase(GCPhase gcPhase);

    size_t GetCounter() const;

    virtual void PostponeGCStart();

    virtual void PostponeGCEnd();

    virtual bool IsPostponeGCSupported() const = 0;

    bool IsPostponeEnabled() const;

    virtual void ComputeNewSize()
    {
        GetObjectAllocator()->GetHeapSpace()->ComputeNewSize();
    }

    /// @return GC specific settings based on runtime options and GC type
    const GCSettings *GetSettings() const
    {
        return &gcSettings_;
    }

    bool IsClearSoftReferencesEnabled() const;

    bool GetFastGCFlag() const;
    void SetFastGCFlag(bool fastGC);

protected:
    /// @brief Runs all phases
    void RunPhases(GCTask &task);

    /**
     * Add task to GC Queue to be run by a GC worker (or run in place)
     * @return false if the task is discarded. Otherwise true.
     * The task may be discarded if the GC already executing a task with
     * the same reason. The task may be discarded by other reasons (for example, task is invalid).
     */
    bool AddGCTask(bool isManaged, PandaUniquePtr<GCTask> task);

    virtual void InitializeImpl() = 0;
    virtual void PreRunPhasesImpl() = 0;
    virtual void RunPhasesImpl(GCTask &task) = 0;
    virtual void PreStartupImp() {}
    virtual size_t AdujustStartupLimit(size_t startupLimit)
    {
        return startupLimit;
    }

    inline bool IsTracingEnabled() const
    {
        return gcSettings_.IsGcEnableTracing();
    }

    inline void BeginTracePoint(const PandaString &tracePointName) const
    {
        if (IsTracingEnabled()) {
            trace::BeginTracePoint(tracePointName.c_str());
        }
    }

    inline void EndTracePoint() const
    {
        if (IsTracingEnabled()) {
            trace::EndTracePoint();
        }
    }

    virtual void VisitRoots(const GCRootVisitor &gcRootVisitor, VisitGCRootFlags flags) = 0;
    virtual void VisitClassRoots(const GCRootVisitor &gcRootVisitor) = 0;
    virtual void VisitCardTableRoots(CardTable *cardTable, const GCRootVisitor &gcRootVisitor,
                                     const MemRangeChecker &rangeChecker, const ObjectChecker &rangeObjectChecker,
                                     const ObjectChecker &fromObjectChecker, uint32_t processedFlag) = 0;

    inline bool CASGCPhase(GCPhase expected, GCPhase set)
    {
        return phase_.compare_exchange_strong(expected, set);
    }

    GCInstanceStats *GetStats()
    {
        return &instanceStats_;
    }

    inline void SetType(GCType gcType)
    {
        gcType_ = gcType;
    }

    inline void SetTLABsSupported()
    {
        tlabsSupported_ = true;
    }

    void SetGCBarrierSet(GCBarrierSet *barrierSet)
    {
        ASSERT(gcBarrierSet_ == nullptr);
        gcBarrierSet_ = barrierSet;
    }

    /**
     * @brief Create GC workers task pool which runs some gc phases in parallel
     * This pool can be based on internal thread pool or TaskManager workers
     */
    void CreateWorkersTaskPool();

    /// @brief Destroy GC workers task pool if it was created
    void DestroyWorkersTaskPool();

    /// Mark all references which we added by AddReference method
    virtual void MarkReferences(GCMarkingStackType *references, GCPhase gcPhase) = 0;

    virtual void UpdateRefsToMovedObjectsInPygoteSpace() = 0;
    /// Update all refs to moved objects
    virtual void CommonUpdateAndSweepVmRefs() = 0;

    const ObjectHeader *PopObjectFromStack(GCMarkingStackType *objectsStack);

    Timing *GetTiming()
    {
        return &timing_;
    }

    template <GCScopeType GC_SCOPE_TYPE>
    friend class GCScope;

    void SetForwardAddress(ObjectHeader *src, ObjectHeader *dst);

    // vector here because we can add some references on young-gc and get new refs on old-gc
    // it's possible if we make 2 GCs for one safepoint
    // max length of this vector - is 2
    // NOLINTNEXTLINE(misc-non-private-member-variables-in-classes)
    PandaVector<ark::mem::Reference *> *clearedReferences_ GUARDED_BY(clearedReferencesLock_) {nullptr};

    os::memory::Mutex *clearedReferencesLock_ {nullptr};  // NOLINT(misc-non-private-member-variables-in-classes)

    std::atomic<size_t> gcCounter_ {0};  // NOLINT(misc-non-private-member-variables-in-classes)
    // NOLINTNEXTLINE(misc-non-private-member-variables-in-classes)
    std::atomic<GCTaskCause> lastCause_ {GCTaskCause::INVALID_CAUSE};

    bool IsExplicitFull(const ark::GCTask &task) const
    {
        return (task.reason == GCTaskCause::EXPLICIT_CAUSE) && !gcSettings_.IsExplicitConcurrentGcEnabled();
    }

    const ReferenceProcessor *GetReferenceProcessor() const
    {
        return referenceProcessor_;
    }

    bool IsWorkerThreadsExist() const
    {
        return gcSettings_.GCWorkersCount() != 0;
    }

    void EnableWorkerThreads();
    void DisableWorkerThreads();

    /// @return true if GC can work in concurrent mode
    bool IsConcurrencyAllowed() const
    {
        return gcSettings_.IsConcurrencyEnabled();
    }

    Logger::Buffer GetLogPrefix() const;

    void FireGCStarted(const GCTask &task, size_t bytesInHeapBeforeGc);
    void FireGCFinished(const GCTask &task, size_t bytesInHeapBeforeGc, size_t bytesInHeapAfterGc);
    void FireGCPhaseStarted(GCPhase phase);
    void FireGCPhaseFinished(GCPhase phase);

    void SetFullGC(bool value);

    /// Set GC Threads on best and middle cores before GC
    void SetupCpuAffinity();

    /// Set GC Threads on best and middle cores after concurrent phase
    void SetupCpuAffinityAfterConcurrent();

    /// Set GC Threads on saved or weak cores before concurrent phase
    void SetupCpuAffinityBeforeConcurrent();

    /// Restore GC Threads after GC on saved cores
    void RestoreCpuAffinity();

    virtual void StartConcurrentScopeRoutine() const;
    virtual void EndConcurrentScopeRoutine() const;

    virtual void PrintDetailedLog();

    Timing timing_;  // NOLINT(misc-non-private-member-variables-in-classes)

    PandaVector<std::pair<PandaString, uint64_t>>
        footprintList_;  // NOLINT(misc-non-private-member-variables-in-classes)

    virtual size_t VerifyHeap() = 0;

private:
    /// Reset GC Threads on saved or weak cores
    void ResetCpuAffinity(bool beforeConcurrent);

    /**
     * Check whether run GC after waiting for mutator threads. Tasks for GC can pass from several mutator threads, so
     * sometime no need to run GC many times. Also some GCs run in place, but in this time GC can run in GC-thread, and
     * "in-place" GC wait for idle state for running, so need to check whether run such GC after waiting for threads
     * @see WaitForIdleGC
     *
     * @param counter_before_waiting value of gc counter before waiting for mutator threads
     * @param task current GC task
     *
     * @return true if need to run GC with current task after waiting for mutator threads or false otherwise
     */
    bool NeedRunGCAfterWaiting(size_t counterBeforeWaiting, const GCTask &task) const;

    /**
     * @brief Making several setups before phases launch
     * @return true if GC run is still needed
     */
    bool GCPhasesPreparation(const GCTask &task);

    /// @brief Getting logs, heap dumps and launching PostGCHeapVerification after GC phases
    void GCPhasesFinish(const GCTask &task);

    /**
     * @brief Create GC worker if needed and set gc status to running (gc_running_ variable)
     * @see IsGCRunning
     */
    void CreateWorker();

    /**
     * @brief Join and destroy GC worker if needed and set gc status to non-running (gc_running_ variable)
     * @see IsGCRunning
     */
    void DestroyWorker();

    /// Move small objects to pygote space at first pygote fork
    void MoveObjectsToPygoteSpace();

    size_t GetNativeBytesFromMallinfoAndRegister() const;
    virtual void ClearLocalInternalAllocatorPools() = 0;
    NativeGcTriggerType GetNativeGcTriggerType();

    class GCListenerManager {
    public:
        GCListenerManager() = default;
        NO_COPY_SEMANTIC(GCListenerManager);
        NO_MOVE_SEMANTIC(GCListenerManager);
        ~GCListenerManager() = default;

        PANDA_PUBLIC_API void AddListener(GCListener *newListener);
        PANDA_PUBLIC_API void RemoveListener(GCListener *newListener);

        void NormalizeListenersOnStartGC();

        template <class Visitor>
        void IterateOverListeners(const Visitor &visitor)
        {
            os::memory::LockHolder lh(listenerLock_);
            for (auto *gcListener : currentListeners_) {
                if (gcListener != nullptr) {
                    visitor(gcListener);
                }
            }
        }

    private:
        os::memory::Mutex listenerLock_;
        PandaList<GCListener *> currentListeners_ GUARDED_BY(listenerLock_);
        PandaList<GCListener *> newListeners_ GUARDED_BY(listenerLock_);
        PandaList<GCListener *> listenersForRemove_ GUARDED_BY(listenerLock_);
    };

    volatile std::atomic<GCPhase> phase_ {GCPhase::GC_PHASE_IDLE};
    GCType gcType_ {GCType::INVALID_GC};
    GCSettings gcSettings_;
    GCListenerManager *gcListenerManager_ {nullptr};
    GCBarrierSet *gcBarrierSet_ {nullptr};
    ObjectAllocatorBase *objectAllocator_ {nullptr};
    InternalAllocatorPtr internalAllocator_ {nullptr};
    GCInstanceStats instanceStats_;
    os::CpuSet affinityBeforeGc_ {};

    // Additional NativeGC
    std::atomic<size_t> nativeBytesRegistered_ = 0;
    std::atomic<size_t> nativeObjectsNotified_ = 0;

    ReferenceProcessor *referenceProcessor_ {nullptr};

    // NOTE(ipetrov): choose suitable priority
    static constexpr size_t GC_TASK_QUEUE_PRIORITY = taskmanager::MAX_QUEUE_PRIORITY;
    taskmanager::TaskQueueInterface *gcWorkersTaskQueue_ = nullptr;

    /* GC worker specific variables */
    GCWorker *gcWorker_ = nullptr;
    std::atomic_bool gcRunning_ = false;
    std::atomic<bool> canAddGcTask_ = true;

    bool tlabsSupported_ = false;

    // Additional data for extensions
    GCExtensionData *extensionData_ {nullptr};

    GCWorkersTaskPool *workersTaskPool_ {nullptr};
    class PostForkGCTask;

    friend class ecmascript::EcmaReferenceProcessor;
    friend class ark::mem::test::MemStatsGenGCTest;
    friend class ark::mem::test::ReferenceStorageTest;
    friend class ark::mem::test::RemSetTest;
    friend class GCScopedPhase;
    friend class GlobalObjectStorage;
    // NOTE(maksenov): Avoid using specific ObjectHelpers class here
    friend class GCDynamicObjectHelpers;
    friend class GCStaticObjectHelpers;
    friend class G1GCTest;
    friend class GCTestLog;

    void TriggerGCForNative();
    size_t SimpleNativeAllocationGcWatermark();
    /// Waits while current GC task(if any) will be processed
    void WaitForIdleGC() NO_THREAD_SAFETY_ANALYSIS;

    friend class GCScopedPhase;
    friend class ConcurrentScope;

    PandaVM *vm_ {nullptr};
    std::atomic<bool> isFullGc_ {false};
    std::atomic<bool> isPostponeEnabled_ {false};
    std::atomic<bool> fastGC_ {false};
    bool clearSoftReferencesEnabled_ {false};
};

/**
 * @brief Create GC with @param gc_type
 * @param gc_type - type of create GC
 * @return pointer to created GC on success, nullptr on failure
 */
template <class LanguageConfig>
GC *CreateGC(GCType gcType, ObjectAllocatorBase *objectAllocator, const GCSettings &settings);

/// Enable concurrent mode. Should be used only from STW code.
class ConcurrentScope final {
public:
    explicit ConcurrentScope(GC *gc, bool autoStart = true);
    NO_COPY_SEMANTIC(ConcurrentScope);
    NO_MOVE_SEMANTIC(ConcurrentScope);
    ~ConcurrentScope();
    void Start();

private:
    GC *gc_;
    bool started_ = false;
};
}  // namespace ark::mem

#endif  // PANDA_RUNTIME_MEM_GC_GC_HMA
