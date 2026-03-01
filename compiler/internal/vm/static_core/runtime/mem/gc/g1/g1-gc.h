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
#ifndef PANDA_RUNTIME_MEM_GC_G1_G1_GC_H
#define PANDA_RUNTIME_MEM_GC_G1_G1_GC_H

#include <functional>

#include "runtime/mem/gc/card_table.h"
#include "runtime/mem/gc/gc.h"
#include "runtime/mem/gc/gc_barrier_set.h"
#include "runtime/mem/gc/g1/g1-allocator.h"
#include "runtime/mem/gc/g1/g1-marker.h"
#include "runtime/mem/gc/g1/collection_set.h"
#include "runtime/mem/gc/generational-gc-base.h"
#include "runtime/mem/heap_verifier.h"
#include "runtime/mem/gc/g1/g1_pause_tracker.h"
#include "runtime/mem/gc/g1/g1_analytics.h"
#include "runtime/mem/gc/g1/update_remset_worker.h"
#include "runtime/mem/gc/g1/object_ref.h"
#include "runtime/mem/gc/g1/g1-evacuate-regions-task.h"
#include "runtime/mem/gc/g1/gc_evacuate_regions_task_stack.h"

namespace ark {
class ManagedThread;
}  // namespace ark
namespace ark::mem {

template <typename LanguageConfig>
class G1EvacuateRegionsWorkerState;

/// @brief Class for reference informantion collecting for rem-sets in G1 GC
class RefInfo {
public:
    RefInfo() = default;

    RefInfo(ObjectHeader *object, uint32_t refOffset) : object_(object), refOffset_(refOffset) {}

    ~RefInfo() = default;

    ObjectHeader *GetObject() const
    {
        return object_;
    }

    uint32_t GetReferenceOffset() const
    {
        return refOffset_;
    }

    DEFAULT_COPY_SEMANTIC(RefInfo);
    DEFAULT_MOVE_SEMANTIC(RefInfo);

private:
    ObjectHeader *object_;
    uint32_t refOffset_;
};

/// @brief G1 alike GC
template <class LanguageConfig>
class G1GC : public GenerationalGC<LanguageConfig> {
    using RefVector = PandaVector<RefInfo>;
    using ReferenceCheckPredicateT = typename GC::ReferenceCheckPredicateT;
    using MemRangeRefsChecker = std::function<bool(Region *, const MemRange &)>;
    template <bool VECTOR>
    using MovedObjectsContainer = std::conditional_t<VECTOR, PandaVector<PandaVector<ObjectHeader *> *>,
                                                     PandaVector<PandaDeque<ObjectHeader *> *>>;
    using GarbageRegions = typename ObjectAllocatorG1<LanguageConfig::MT_MODE>::GarbageRegions;

public:
    explicit G1GC(ObjectAllocatorBase *objectAllocator, const GCSettings &settings);

    ~G1GC() override;

    void StopGC() override
    {
        GC::StopGC();
        // GC is using update_remset_worker so we need to stop GC first before we destroy the worker
        updateRemsetWorker_->DestroyWorker();
    }

    NO_MOVE_SEMANTIC(G1GC);
    NO_COPY_SEMANTIC(G1GC);

    void InitGCBits(ark::ObjectHeader *objHeader) override;

    void InitGCBitsForAllocationInTLAB(ark::ObjectHeader *object) override;

    bool IsPinningSupported() const final
    {
        // G1 GC supports region pinning, so G1 can pin objects
        return true;
    }

    void WorkerTaskProcessing(GCWorkersTask *task, void *workerData) override;

    void MarkReferences(GCMarkingStackType *references, GCPhase gcPhase) override;

    void MarkObject(ObjectHeader *object) override;

    bool MarkObjectIfNotMarked(ObjectHeader *object) override;

    void MarkObjectRecursively(ObjectHeader *object);

    bool InGCSweepRange(const ObjectHeader *object) const override;

    void OnThreadTerminate(ManagedThread *thread, mem::BuffersKeepingFlag keepBuffers) override;
    void OnThreadCreate(ManagedThread *thread) override;

    void PreZygoteFork() override;
    void PostZygoteFork() override;

    void OnWaitForIdleFail() override;

    void StartGC() override
    {
        updateRemsetWorker_->CreateWorker();
        GC::StartGC();
    }

    bool HasRefFromRemset(ObjectHeader *obj)
    {
        for (auto &refVector : uniqueRefsFromRemsets_) {
            auto it = std::find_if(refVector->cbegin(), refVector->cend(),
                                   [obj](auto ref) { return ref.GetObject() == obj; });
            if (it != refVector->cend()) {
                return true;
            }
        }
        return false;
    }

    void PostponeGCStart() override;
    void PostponeGCEnd() override;
    bool IsPostponeGCSupported() const override;

    void StartConcurrentScopeRoutine() const override;
    void EndConcurrentScopeRoutine() const override;

    void ComputeNewSize() override;
    bool Trigger(PandaUniquePtr<GCTask> task) override;
    void EvacuateStartingWith(void *ref) override;
    void SetExtensionData(GCExtensionData *data) override;

    void PostForkCallback(size_t restoreLimit) override;

protected:
    ALWAYS_INLINE ObjectAllocatorG1<LanguageConfig::MT_MODE> *GetG1ObjectAllocator() const
    {
        return static_cast<ObjectAllocatorG1<LanguageConfig::MT_MODE> *>(this->GetObjectAllocator());
    }

    // NOLINTBEGIN(misc-non-private-member-variables-in-classes)
    /// Queue with updated refs info
    GCG1BarrierSet::ThreadLocalCardQueues *updatedRefsQueue_ {nullptr};
    GCG1BarrierSet::ThreadLocalCardQueues *updatedRefsQueueTemp_ {nullptr};
    os::memory::Mutex queueLock_;
    os::memory::Mutex gcWorkerQueueLock_;
    // NOLINTEND(misc-non-private-member-variables-in-classes)

private:
    using Ref = typename ObjectReference<LanguageConfig::LANG_TYPE>::Type;

    void CreateUpdateRemsetWorker();
    void ProcessDirtyCards();
    template <typename Handler>
    void ProcessDirtyCards(const Handler &handler);
    bool HaveGarbageRegions();
    size_t GetOldCollectionSetCandidatesNumber();

    template <RegionFlag REGION_TYPE, bool FULL_GC>
    void DoRegionCompacting(Region *region, bool useGcWorkers,
                            PandaVector<PandaVector<ObjectHeader *> *> *movedObjectsVector);

    template <bool ATOMIC, bool CONCURRENTLY>
    void CollectNonRegularObjects();

    template <bool ATOMIC, bool CONCURRENTLY>
    void CollectEmptyRegions(GCTask &task, PandaVector<Region *> &&emptyTenuredRegions);

    template <bool ATOMIC, bool CONCURRENTLY>
    void ClearEmptyTenuredMovableRegions(PandaVector<Region *> &&emptyTenuredRegions);

    bool NeedToPromote(const Region *region) const;

    template <bool ATOMIC, RegionFlag REGION_TYPE, bool FULL_GC>
    void RegionCompactingImpl(Region *region, const ObjectVisitor &movedObjectSaver);

    template <bool ATOMIC, bool FULL_GC>
    void RegionPromotionImpl(Region *region, const ObjectVisitor &movedObjectSaver);

    // Return whether all cross region references were processed in mem_range
    template <typename Handler>
    void IterateOverRefsInMemRange(const MemRange &memRange, Region *region, Handler &refsHandler);

    template <typename Visitor>
    void CacheRefsFromDirtyCards(GlobalRemSet &globalRemSet, Visitor visitor);

    void InitializeImpl() override;

    bool NeedFullGC(const ark::GCTask &task);

    bool NeedToRunGC(const ark::GCTask &task);

    void StartGCCollection(ark::GCTask &task);

    void RunPhasesImpl(GCTask &task) override;

    void RunFullGC(ark::GCTask &task);
    void TryRunMixedGC(ark::GCTask &task);
    void CollectAndMoveTenuredRegions(const CollectionSet &collectionSet);
    void CollectAndMoveYoungRegions(const CollectionSet &collectionSet);

    void RunMixedGC(ark::GCTask &task, const CollectionSet &collectionSet);

    /// Determine whether GC need to run concurrent mark or mixed GC
    bool ScheduleMixedGCAndConcurrentMark(ark::GCTask &task);

    /// Start concurrent GC
    template <typename OnPauseMarker, typename ConcurrentMarker>
    void RunConcurrentGC(ark::GCTask &task, OnPauseMarker &pmarker, ConcurrentMarker &cmarker);

    void RunPhasesForRegions([[maybe_unused]] ark::GCTask &task, const CollectionSet &collectibleRegions);

    void PreStartupImp() override;

    size_t AdujustStartupLimit(size_t startupLimit) override;

    void VisitCard(CardTable::CardPtr card, const ObjectVisitor &objectVisitor, const CardVisitor &cardVisitor);

    /// GC for young generation. Runs with STW.
    void RunGC(GCTask &task, const CollectionSet &collectibleRegions);

    /**
     * Return true if garbage can be collected in single pass (VM supports it, no pinned objects, GC is not postponed
     * etc) otherwise false
     */
    bool SinglePassCompactionAvailable();
    void CollectInSinglePass(const GCTask &task);
    void UpdateMetricsAfterSinglePass(size_t allocatedBytesYoung, size_t allocatedBytesOld,
                                      size_t allocatedObjectsYoung, size_t allocatedObjectsOld);
    void CollectMetrickBeforeSinglePass(size_t &allocatedBytesYoung, size_t &allocatedBytesOld,
                                        size_t &allocatedObjectsYoung, size_t &allocatedObjectsOld);
    void EvacuateCollectionSet(const RemSet<> &remset);
    void UpdateAndSweep();
    void MergeRemSet(RemSet<> *remset);
    void HandleReferences(const GCTask &task);
    void ResetRegionAfterMixedGC();

    /// GC for tenured generation.
    void RunTenuredGC(const GCTask &task);

    /**
     * Mark predicate with calculation of live bytes in region
     * @tparam ATOMICALLY atomically or non-atomically live bytes calculation
     * @param object marked object from marking-stack
     * @param baseKlass class of the passed object from marking-stack
     * @see MarkStack
     * @see ConcurrentMarkImpl
     */
    template <bool ATOMICALLY = true>
    static void CalcLiveBytesMarkPreprocess(const ObjectHeader *object, BaseClass *baseKlass);

    /// Caches refs from remset and marks objects in collection set (young-generation + maybe some tenured regions).
    void MixedMarkAndCacheRefs(const GCTask &task, const CollectionSet &collectibleRegions);

    void EnableFullPromotionIfNeeded();
    void FullPromotion(const CollectionSet &collectibleRegions);
    void FastYoungMark(const CollectionSet &collectibleRegions);
    void EnqueueAllYoungCards(const CollectionSet &collectibleRegions);

    template <typename Marker, typename Predicate>
    GCRootVisitor CreateGCRootVisitor(GCMarkingStackType &objectsStack, Marker &marker, const Predicate &refPred);

    /**
     * Initial marks roots and fill in 1st level from roots into stack.
     * STW
     * @param objects_stack
     */
    template <bool PROCESS_WEAK_REFS, typename Marker>
    void InitialMark(GCMarkingStackType &markingStack, Marker &marker);

    template <typename Marker>
    void UnmarkAll(Marker &marker);

    void MarkStackMixed(GCMarkingStackType *stack);

    void MarkStackFull(GCMarkingStackType *stack);

    bool IsInCollectionSet(ObjectHeader *object);

    template <bool FULL_GC>
    void UpdateRefsAndClear(const CollectionSet &collectionSet, MovedObjectsContainer<FULL_GC> *movedObjectsContainer,
                            PandaVector<PandaVector<ObjectHeader *> *> *movedObjectsVector,
                            HeapVerifierIntoGC<LanguageConfig> *collectVerifier);

    /**
     * Collect dead objects in young generation and move survivors
     * @return true if moving was success, false otherwise
     */
    template <bool FULL_GC>
    bool CollectAndMove(const CollectionSet &collectionSet);

    /**
     * Collect verification info for CollectAndMove phase
     * @param collection_set collection set for the current phase
     * @return instance of verifier to be used to verify for updated references
     */
    [[nodiscard]] HeapVerifierIntoGC<LanguageConfig> CollectVerificationInfo(const CollectionSet &collectionSet);

    /**
     * Verify updted references
     * @param collect_verifier instance of the verifier that was obtained before references were updated
     * @param collection_set collection set for the current phase
     *
     * @see CollectVerificationInfo
     * @see UpdateRefsToMovedObjects
     */
    void VerifyCollectAndMove(HeapVerifierIntoGC<LanguageConfig> &&collectVerifier, const CollectionSet &collectionSet);

    template <bool FULL_GC, bool NEED_LOCK>
    std::conditional_t<FULL_GC, UpdateRemsetRefUpdater<LanguageConfig, NEED_LOCK>,
                       EnqueueRemsetRefUpdater<LanguageConfig>>
    CreateRefUpdater(GCG1BarrierSet::ThreadLocalCardQueues *updatedRefQueue) const;

    template <class ObjectsContainer>
    void ProcessMovedObjects(ObjectsContainer *movedObjects);

    /// Update refs to objects which were moved while garbage collection
    template <bool FULL_GC, bool ENABLE_WORKERS, class Visitor>
    void UpdateMovedObjectsReferences(MovedObjectsContainer<FULL_GC> *movedObjectsContainer, const Visitor &refUpdater);

    /// Update all refs to moved objects
    template <bool FULL_GC, bool USE_WORKERS>
    void UpdateRefsToMovedObjects(MovedObjectsContainer<FULL_GC> *movedObjectsContainer);

    bool IsMarked(const ObjectHeader *object) const override;
    bool IsMarkedEx(const ObjectHeader *object) const override;

    /// Start process of on pause marking
    void FullMarking(ark::GCTask &task);

    /**
     * Marking all objects on pause
     * @param task gc task for current GC
     * @param objects_stack stack for marked objects
     * @param use_gc_workers whether do marking in parallel
     */
    void OnPauseMark(GCTask &task, GCMarkingStackType *objectsStack, bool useGcWorkers);

    /// Iterate over roots and mark them concurrently
    template <bool PROCESS_WEAK_REFS, bool ATOMICALLY, typename Marker>
    NO_THREAD_SAFETY_ANALYSIS void ConcurrentMarkImpl(GCMarkingStackType *objectsStack, Marker &marker);

    void PauseTimeGoalDelay();

    /*
     * Mark the heap in concurrent mode and calculate live bytes
     */
    template <bool PROCESS_WEAK_REFS, typename Marker>
    void ConcurrentMark(const GCTask &task, GCMarkingStackType *objectsStack, Marker &marker);

    /// ReMarks objects after Concurrent marking and actualize information about live bytes
    template <bool PROCESS_WEAK_REFS, typename Marker>
    void Remark(const GCTask &task, Marker &marker);

    /// Sweep VM refs for non-regular (humongous + nonmovable) objects
    void SweepNonRegularVmRefs();

    void VerifyHeapBeforeConcurrent();

    void ConcurrentSweep(ark::GCTask &task, PandaVector<Region *> &&emptyTenuredRegions);

    /// Return collectible regions
    CollectionSet GetCollectibleRegions(ark::GCTask const &task, bool isMixed);
    template <typename Predicate>
    void DrainOldRegions(CollectionSet &collectionSet, Predicate pred);
    void AddOldRegionsMaxAllowed(CollectionSet &collectionSet);
    void AddOldRegionsAccordingPauseTimeGoal(CollectionSet &collectionSet);
    uint64_t AddMoreOldRegionsAccordingPauseTimeGoal(CollectionSet &collectionSet, uint64_t gcPauseTimeBudget);
    void ReleasePagesInFreePools();

    CollectionSet GetFullCollectionSet(PandaVector<std::pair<uint32_t, Region *>> &&garbageRegions);

    void UpdateCollectionSet(const CollectionSet &collectibleRegions);

    /// Interrupts release pages if the process is running
    void InterruptReleasePagesIfNeeded();

    /**
     * Starts release pages if the current status
     *  of releasePagesInterruptFlag_ equals @param oldStatus
     * @param oldStatus estimated status of releasePagesInterruptFlag_
     */
    void StartReleasePagesIfNeeded(ReleasePagesStatus oldStatus);

    /// Estimate space in tenured to objects from collectible regions
    bool HaveEnoughSpaceToMove(const CollectionSet &collectibleRegions);

    /// Check if we have enough free regions in tenured space
    bool HaveEnoughRegionsToMove(size_t num);

    /**
     * Add data from SATB buffer to the object stack
     * @param object_stack - stack to add data to
     */
    template <typename Marker>
    void DrainSatb(GCAdaptiveMarkingStack *objectStack, Marker &marker);

    void HandlePendingDirtyCards();

    void ReenqueueDirtyCards();

    void ClearSatb();

    /**
     * Iterate over object references in rem sets.
     * The Visitor is a functor which accepts an object (referee), the reference value,
     * offset of the reference in the object and the flag whether the reference is volatile.
     * The visitor can be called for the references to the collection set in the object or
     * for all references in an object which has at least one reference to the collection set.
     * The decision is implementation dependent.
     */
    template <class Visitor>
    void VisitRemSets(const Visitor &visitor);

    template <class Visitor>
    void UpdateRefsFromRemSets(const Visitor &visitor);

    void CacheRefsFromRemsets(const MemRangeRefsChecker &refsChecker);
    bool IsCollectionSetFullyPromoted() const;

    void ClearRefsFromRemsetsCache();

    void ActualizeRemSets();

    bool ShouldRunTenuredGC(const GCTask &task) override;

    void RestoreYoungCards(const CollectionSet &collectionSet);

    void ClearYoungCards(const CollectionSet &collectionSet);

    void ClearTenuredCards(const CollectionSet &collectionSet);

    size_t GetMaxMixedRegionsCount();

    void PrepareYoungRegionsForFullGC(const CollectionSet &collectionSet);

    void RestoreYoungRegionsAfterFullGC(const CollectionSet &collectionSet);

    template <typename Container>
    void BuildCrossYoungRemSets(const Container &young);

    size_t CalculateDesiredEdenLengthByPauseDelay();
    size_t CalculateDesiredEdenLengthByPauseDuration();

    template <bool ENABLE_BARRIER>
    void UpdatePreWrbEntrypointInThreads();

    void EnablePreWrbInThreads()
    {
        UpdatePreWrbEntrypointInThreads<true>();
    }

    void DisablePreWrbInThreads()
    {
        UpdatePreWrbEntrypointInThreads<false>();
    }

    void EnsurePreWrbDisabledInThreads();

    size_t GetUniqueRemsetRefsCount() const;

    void ExecuteMarkingTask(GCMarkWorkersTask::StackType *objectsStack);
    template <typename Marker>
    void ExecuteRemarkTask(GCMarkWorkersTask::StackType *objectsStack, Marker &marker);
    /**
     * @brief Method gets stack with only one array object initially.
     * The stack also has an information about the interval in the array
     * that should be traversed in the current task
     * @param objectsStack stack, initially having one array object
     */
    template <typename Marker>
    void ExecuteHugeArrayMarkTask(GCMarkWorkersTask::StackType *objectsStack, Marker &marker);
    void ExecuteFullMarkingTask(GCMarkWorkersTask::StackType *objectsStack);
    void ExecuteCompactingTask(Region *region, const ObjectVisitor &movedObjectsSaver);
    void ExecuteEnqueueRemsetsTask(GCUpdateRefsWorkersTask<false>::MovedObjectsRange *movedObjectsRange);
    void ExecuteEvacuateTask(typename G1EvacuateRegionsTask<Ref>::StackType *stack);

    void PrintFragmentationMetrics(const char *title);

    G1GCMarker<LanguageConfig, true> marker_;
    G1GCMarker<LanguageConfig, false> concMarker_;
    G1GCMixedMarker<LanguageConfig> mixedMarker_;
    XGCMarker<LanguageConfig, true> onPauseXMarker_;
    // NOTE(audovichenko): Do not use atomics in concXMarker_
    XGCMarker<LanguageConfig, true> concXMarker_;
    /// Flag indicates if we currently in concurrent marking phase
    std::atomic<bool> concurrentMarkingFlag_ {false};
    /// Flag indicates if we need to interrupt concurrent marking
    std::atomic<bool> interruptConcurrentFlag_ {false};
    /// Function called in the post WRB
    std::function<void(const void *, const void *)> postQueueFunc_ {nullptr};
    /// Current pre WRB entrypoint: either nullptr or the real function
    ObjRefProcessFunc currentPreWrbEntrypoint_ {nullptr};
    /**
     * After first process it stores humongous objects only, after marking them it's still store them for updating
     * pointers from Humongous
     */
    PandaList<PandaVector<ObjectHeader *> *> satbBuffList_ GUARDED_BY(satbAndNewobjBufLock_) {};
    PandaVector<ObjectHeader *> newobjBuffer_ GUARDED_BY(satbAndNewobjBufLock_);
    // The lock guards both variables: satb_buff_list_ and newobj_buffer_
    os::memory::Mutex satbAndNewobjBufLock_;
    UpdateRemsetWorker<LanguageConfig> *updateRemsetWorker_ {nullptr};
    GCMarkingStackType concurrentMarkingStack_;
    GCMarkingStackType::MarkedObjects mixedMarkedObjects_;
    std::atomic<bool> isMixedGcRequired_ {false};
    /// Number of tenured regions added at the young GC
    size_t numberOfMixedTenuredRegions_ {2};
    double regionGarbageRateThreshold_ {0.0};
    double g1PromotionRegionAliveRate_ {0.0};
    bool g1TrackFreedObjects_ {false};
    bool isExplicitConcurrentGcEnabled_ {false};
    bool fullCollectionSetPromotion_ {false};
    // There are may be some regions with pinned objects that GC cannot collect
    PandaVector<std::pair<uint32_t, Region *>> topGarbageRegions_ {};
    CollectionSet collectionSet_;
    // Max size of unique_refs_from_remsets_ buffer. It should be enough to store
    // almost all references to the collection set.
    // But any way there may be humongous arrays which contains a lot of references to the collection set.
    // For such objects GC created a new RefVector, which will be cleared at the end of the collections.
    static constexpr size_t MAX_REFS = 1024;
    // Storages for references from remsets to the collection set.
    // List elements have RefVector inside, with double size compare to previous one (starts from MAX_REFS)
    // Each vector element contains an object from the remset and the offset of
    // the field which refers to the collection set.
    PandaList<RefVector *> uniqueRefsFromRemsets_;
    // Dirty cards which are not fully processed before collection.
    // These cards are processed later.
    PandaUnorderedSet<CardTable::CardPtr> dirtyCards_;
#ifndef NDEBUG
    bool uniqueCardsInitialized_ = false;
#endif  // NDEBUG
    size_t regionSizeBits_;
    G1PauseTracker g1PauseTracker_;
    os::memory::Mutex concurrentMarkMutex_;
    os::memory::Mutex mixedMarkedObjectsMutex_;
    os::memory::ConditionVariable concurrentMarkCondVar_;
    G1Analytics analytics_;

    /// Flag indicates if we need to interrupt release physical pages to OS
    std::atomic<ReleasePagesStatus> releasePagesInterruptFlag_ {ReleasePagesStatus::FINISHED};

    size_t copiedBytesYoung_ {0};
    size_t copiedBytesOld_ {0};

    size_t copiedObjectsYoung_ {0};
    size_t copiedObjectsOld_ {0};
    bool singlePassCompactionEnabled_ {false};

    template <class, bool>
    friend class RefCacheBuilder;
    friend class G1GCTest;
    friend class RemSetChecker;
    template <class>
    friend class G1EvacuateRegionsWorkerState;
};

template <MTModeT MT_MODE>
class AllocConfig<GCType::G1_GC, MT_MODE> {
public:
    using ObjectAllocatorType = ObjectAllocatorG1<MT_MODE>;
    using CodeAllocatorType = CodeAllocator;
};

}  // namespace ark::mem

#endif  // PANDA_RUNTIME_MEM_GC_G1_G1_GC_H
