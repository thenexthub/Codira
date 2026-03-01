/**
 * Copyright (c) 2022-2026 Huawei Device Co., Ltd.
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 * http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include <algorithm>
#include <limits>
#include "libarkbase/macros.h"
#include "libarkbase/os/time.h"
#include "runtime/coroutines/coroutine_worker_group.h"
#include "runtime/coroutines/stackful_common.h"
#include "runtime/coroutines/coroutine.h"
#include "runtime/coroutines/stackful_coroutine.h"
#include "runtime/coroutines/stackful_coroutine_manager.h"
#include "runtime/include/panda_vm.h"
#include "runtime/include/runtime.h"
#include "runtime/include/runtime_notification.h"
#include "runtime/include/thread_scopes.h"

#include "runtime/trace.h"

namespace ark {

uint8_t *StackfulCoroutineManager::AllocCoroutineStack()
{
    return nativeStackAllocator_.AcquireStack();
}

void StackfulCoroutineManager::FreeCoroutineStack(uint8_t *stack)
{
    if (stack != nullptr) {
        nativeStackAllocator_.ReleaseStack(stack);
    }
}

void StackfulCoroutineManager::CreateMainCoroAndWorkers(size_t howMany, Runtime *runtime, PandaVM *vm)
{
    auto *wMain = CreateWorker(runtime, vm, StackfulCoroutineWorker::ScheduleLoopType::FIBER, "[main] worker ", true);
    ASSERT(wMain != nullptr);
    ASSERT(wMain->GetId() == MAIN_WORKER_ID);

    auto *mainCo = CreateMainCoroutine(runtime, vm);
    ASSERT(mainCo != nullptr);
    wMain->AddRunningCoroutine(mainCo);
    OnWorkerStartupImpl(wMain);

    CreateWorkersImpl(howMany, runtime, vm);
}

void StackfulCoroutineManager::CreateWorkers(size_t howMany, Runtime *runtime, PandaVM *vm)
{
    os::memory::LockHolder lh(workersLock_);
    Tracer::Start(Tracer::WORKERS_EXPANSION);
    CreateWorkersImpl(howMany, runtime, vm);
    Tracer::Finish();
}

void StackfulCoroutineManager::FinalizeWorkers(size_t howMany, Runtime *runtime, PandaVM *vm)
{
    Tracer::Start(Tracer::WORKERS_CONTRACTION);
    struct EntrypointParam {
        explicit EntrypointParam(size_t wCount, CoroutineManager *coroMan)
            : wCount_(wCount), workerFinalizationEvent(coroMan)
        {
        }

        // NOLINTBEGIN(misc-non-private-member-variables-in-classes)
        const size_t wCount_;
        GenericEvent workerFinalizationEvent;
        std::atomic<uint32_t> finalizedWorkersCount = 0;
        // NOLINTEND(misc-non-private-member-variables-in-classes)
    };

    auto wCountBeforeFinalization = GetActiveWorkersCount();
    ASSERT(wCountBeforeFinalization > howMany);

    auto entrypointParam = EntrypointParam(howMany, this);
    auto coroEntryPoint = [](void *param) {
        auto *finalizee = Coroutine::GetCurrent()->GetContext<StackfulCoroutineContext>()->GetWorker();
        finalizee->MigrateCoroutines();
        finalizee->CompleteAllAffinedCoroutines();
        finalizee->SetActive(false);
        auto *entryParams = reinterpret_cast<EntrypointParam *>(param);
        // Atomic with relaxed order reason: synchronization is not required
        if (entryParams->finalizedWorkersCount.fetch_add(1, std::memory_order_relaxed) == entryParams->wCount_ - 1) {
            entryParams->workerFinalizationEvent.Happen();
        }
    };

    for (auto i = 0U; i < howMany; i++) {
        auto *finWorker = ChooseWorkerForFinalization();
        auto *co = CreateNativeCoroutine(runtime, vm, coroEntryPoint, &entrypointParam, "[finalize coro] ",
                                         Coroutine::Type::FINALIZER, CoroutinePriority::CRITICAL_PRIORITY);
        ASSERT(co != nullptr);
        finWorker->AddRunnableCoroutine(co);
    }
    entrypointParam.workerFinalizationEvent.Lock();

    Await(&entrypointParam.workerFinalizationEvent);

    os::memory::LockHolder lh(workersLock_);
    while (activeWorkersCount_ != wCountBeforeFinalization - howMany) {
        workersCv_.Wait(&workersLock_);
    }
    ASSERT(activeWorkersCount_ > 0);
    Tracer::Finish();
}

StackfulCoroutineWorker *StackfulCoroutineManager::ChooseWorkerForFinalization()
{
    os::memory::LockHolder lh(workersLock_);
    auto finWorkerIt = std::find_if(workers_.begin(), workers_.end(), [](auto &&worker) {
        return !worker->IsMainWorker() && !worker->IsDisabledForCrossWorkersLaunch() && !worker->InExclusiveMode();
    });
    ASSERT(finWorkerIt != workers_.end());
    (*finWorkerIt)->DisableForCrossWorkersLaunch();
    return *finWorkerIt;
}

void StackfulCoroutineManager::CreateWorkersImpl(size_t howMany, Runtime *runtime, PandaVM *vm)
{
    if (howMany == 0) {
        LOG(DEBUG, COROUTINES)
            << "StackfulCoroutineManager::CreateWorkersImpl():creation of zero workers requested,skipping...";
        return;
    }
    auto wCountBeforeCreation = activeWorkersCount_;
    for (uint32_t i = 0; i < howMany; ++i) {
        auto *worker = CreateWorker(runtime, vm, StackfulCoroutineWorker::ScheduleLoopType::THREAD, "worker ");
        generalWorkerGroup_ = CoroutineWorkerGroup::ExtendGroup(generalWorkerGroup_, worker->GetId());
    }
    LOG(DEBUG, COROUTINES) << "StackfulCoroutineManager::CreateWorkers(): waiting for workers startup";
    while (activeWorkersCount_ != howMany + wCountBeforeCreation) {
        // NOTE(konstanting, #IAD5MH): need timed wait?..
        workersCv_.Wait(&workersLock_);
    }
}

StackfulCoroutineWorker *StackfulCoroutineManager::CreateWorker(Runtime *runtime, PandaVM *vm,
                                                                StackfulCoroutineWorker::ScheduleLoopType wType,
                                                                PandaString workerName, bool isMainWorker)
{
    auto allocator = runtime->GetInternalAllocator();
    auto workerId = AllocateWorkerId();
    workerName += ToPandaString(workerId);
    auto *worker = allocator->New<StackfulCoroutineWorker>(runtime, vm, this, wType, std::move(workerName), workerId,
                                                           isMainWorker);
    ASSERT(worker != nullptr);
    if (stats_.IsEnabled()) {
        worker->GetPerfStats().Enable();
    }
    return worker;
}

void StackfulCoroutineManager::OnWorkerShutdown(StackfulCoroutineWorker *worker)
{
    os::memory::LockHolder lock(workersLock_);
    auto workerIt = std::find_if(workers_.begin(), workers_.end(), [worker](auto &&w) { return w == worker; });
    workers_.erase(workerIt);
    // We may have a problem related to the coroutine affinity mask aliasing (The ABA Problem) #23715:
    // 1. Finalizing worker was available for the coroutine (the coroutine had a bit set in the affinity mask)
    // 2. New specific worker (e.g. EAWorker) was created with the same Id
    // 3. This worker becomes available for the coroutine, but it was not initially expected
    ReleaseWorkerId(worker->GetId());
    --activeWorkersCount_;
    auto &workerStats = worker->GetPerfStats();
    workerStats.Disable();
    finalizedWorkerStats_.emplace_back(std::move(workerStats));
    Runtime::GetCurrent()->GetInternalAllocator()->Delete(worker);
    workersCv_.Signal();
    LOG(DEBUG, COROUTINES) << "StackfulCoroutineManager::OnWorkerShutdown(): COMPLETED, workers left = "
                           << activeWorkersCount_;
}

void StackfulCoroutineManager::OnWorkerStartup(StackfulCoroutineWorker *worker)
{
    os::memory::LockHolder lock(workersLock_);
    OnWorkerStartupImpl(worker);
}

void StackfulCoroutineManager::OnWorkerStartupImpl(StackfulCoroutineWorker *worker)
{
    workers_.push_back(worker);
    ++activeWorkersCount_;
    worker->OnWorkerStartup();
    workersCv_.Signal();
    LOG(DEBUG, COROUTINES) << "StackfulCoroutineManager::OnWorkerStartup(): COMPLETED, active workers = "
                           << activeWorkersCount_;
}

void StackfulCoroutineManager::InitializeWorkerIdAllocator()
{
    os::memory::LockHolder lh(workerIdLock_);
    for (auto id = MAIN_WORKER_ID; id <= MAX_WORKER_ID; ++id) {
        freeWorkerIds_.push_back(id);
    }
}

CoroutineWorker::Id StackfulCoroutineManager::AllocateWorkerId()
{
    os::memory::LockHolder lh(workerIdLock_);
    ASSERT(!freeWorkerIds_.empty());
    auto workerId = freeWorkerIds_.front();
    freeWorkerIds_.pop_front();
    return workerId;
}

void StackfulCoroutineManager::ReleaseWorkerId(CoroutineWorker::Id workerId)
{
    os::memory::LockHolder lh(workerIdLock_);
    if (workerId != MAIN_WORKER_ID) {
        ASSERT(CoroutineWorkerGroup::HasWorker(generalWorkerGroup_, workerId) ||
               CoroutineWorkerGroup::HasWorker(eaWorkerGroup_, workerId));
        if (CoroutineWorkerGroup::HasWorker(generalWorkerGroup_, workerId)) {
            generalWorkerGroup_ = CoroutineWorkerGroup::ShrinkGroup(generalWorkerGroup_, workerId);
        } else {
            eaWorkerGroup_ = CoroutineWorkerGroup::ShrinkGroup(eaWorkerGroup_, workerId);
        }
    }
    freeWorkerIds_.push_back(workerId);
    ASSERT(freeWorkerIds_.size() <= AffinityMask::MAX_WORKERS_COUNT);
}

void StackfulCoroutineManager::DisableCoroutineSwitch()
{
    GetCurrentWorker()->DisableCoroutineSwitch();
}

void StackfulCoroutineManager::EnableCoroutineSwitch()
{
    GetCurrentWorker()->EnableCoroutineSwitch();
}

bool StackfulCoroutineManager::IsCoroutineSwitchDisabled()
{
    return GetCurrentWorker()->IsCoroutineSwitchDisabled();
}

void StackfulCoroutineManager::InitializeScheduler(Runtime *runtime, PandaVM *vm)
{
    // enable stats collection if needed
    if (GetConfig().enablePerfStats) {
        stats_.Enable();
    }
    if (GetConfig().workersCount == 1U) {
        SetSchedulingPolicy(CoroutineSchedulingPolicy::ANY_WORKER);
    }
    ScopedCoroutineStats s(&stats_, CoroutineTimeStats::INIT);
    // set limits
    coroStackSizeBytes_ = Runtime::GetCurrent()->GetOptions().GetCoroutineStackSizePages() * os::mem::GetPageSize();
    if (coroStackSizeBytes_ != AlignUp(coroStackSizeBytes_, PANDA_POOL_ALIGNMENT_IN_BYTES)) {
        size_t alignmentPages = PANDA_POOL_ALIGNMENT_IN_BYTES / os::mem::GetPageSize();
        LOG(FATAL, COROUTINES) << "Coroutine stack size should be >= " << alignmentPages
                               << " pages and should be aligned to " << alignmentPages << "-page boundary!";
    }
    size_t coroStackAreaSizeBytes = Runtime::GetCurrent()->GetOptions().GetCoroutinesStackMemLimit();
    coroutineCountLimit_ = coroStackAreaSizeBytes / coroStackSizeBytes_;

    CalculateWorkerLimits(exclusiveWorkersLimit_, commonWorkersCount_);
    CalculateUserCoroutinesLimits(userCoroutineCountLimit_,
                                  Runtime::GetCurrent()->GetOptions().GetCoroutinesUserLimit());

    nativeStackAllocator_.Initialize(coroStackSizeBytes_);
    ASSERT(commonWorkersCount_ + exclusiveWorkersLimit_ <= AffinityMask::MAX_WORKERS_COUNT);
    InitializeWorkerIdAllocator();
    {
        os::memory::LockHolder lock(workersLock_);
        CreateMainCoroAndWorkers(commonWorkersCount_ - 1, runtime, vm);  // 1 is for MAIN here
        LOG(DEBUG, COROUTINES) << "StackfulCoroutineManager(): successfully created and activated " << workers_.size()
                               << " coroutine workers";
        programCompletionEvent_ = Runtime::GetCurrent()->GetInternalAllocator()->New<GenericEvent>(this);
    }
}

void StackfulCoroutineManager::Finalize()
{
    os::memory::LockHolder lock(coroPoolLock_);

    auto allocator = Runtime::GetCurrent()->GetInternalAllocator();
    allocator->Delete(programCompletionEvent_);
    for (auto *co : coroutinePool_) {
        co->DestroyInternalResources();
        CoroutineManager::DestroyEntrypointfulCoroutine(co);
    }
    coroutinePool_.clear();
    nativeStackAllocator_.Finalize();
}

void StackfulCoroutineManager::AddToRegistry(Coroutine *co)
{
    co->GetVM()->GetGC()->OnThreadCreate(co);
    coroutines_.insert(co);
    coroutineCount_++;

    if (co->GetType() != Coroutine::Type::MUTATOR) {
        utilityCoroutineCount_++;
    }

    if (!CoroutineManager::IsSystemCoroutine(co)) {
        userCoroutineCount_++;
    }
}

void StackfulCoroutineManager::RemoveFromRegistry(Coroutine *co)
{
    coroutines_.erase(co);
    coroutineCount_--;
    if (co->GetType() != Coroutine::Type::MUTATOR) {
        utilityCoroutineCount_--;
    }
    if (!CoroutineManager::IsSystemCoroutine(co)) {
        userCoroutineCount_--;
    }
}

void StackfulCoroutineManager::RegisterCoroutine(Coroutine *co)
{
    os::memory::LockHolder lock(coroListLock_);
    AddToRegistry(co);
    // Propagate SUSPEND_REQUEST flag to the new coroutine to avoid the following situation:
    // * Main coro holds read lock of the MutatorLock.
    // * GC thread calls SuspendAll nad set SUSPEND_REQUEST flag to the main coro and
    //   tries to acquire write lock of the MutatorLock.
    // * Main coro creates a new coro and adds it to the coroutines_ list.
    // * SUSPEND_REQUEST is not set in the new coroutine
    // * New coro starts execution, acquires read lock of the MutatorLock and enters a long loop
    // * Main coro checks SUSPEND_REQUEST flag and blocks
    // * GC will not start becuase the new coro has no SUSPEND_REQUEST flag and it will never release the MutatorLock
    //
    // We need to propagate SUSPEND_REQUEST under the coroListLock_.
    // It guarantees that the flag is already set for the current coro and we need to propagate it
    // or GC will see the new coro in EnumerateAllThreads.
#ifndef ARK_HYBRID
    if (Thread::GetCurrent() != nullptr && Coroutine::GetCurrent() != nullptr &&
        Coroutine::GetCurrent()->IsSuspended() && !co->IsSuspended()) {
        co->SuspendImpl(true);
    }
#endif
}

bool StackfulCoroutineManager::TerminateCoroutine(Coroutine *co)
{
    if (co->HasManagedEntrypoint()) {
        // profiling: start interval here, end in ctxswitch after finalization request is done
        GetCurrentWorker()->GetPerfStats().StartInterval(CoroutineTimeStats::SCH_ALL);
    } else {
        // profiling: no need. MAIN and NATIVE EP coros are deleted from the SCHEDULER itself
        LOG(DEBUG, COROUTINES) << "StackfulCoroutineManager::TerminateCoroutine(): terminating "
                               << ((GetExistingWorkersCount() == 0) ? "MAIN..." : "NATIVE EP coro...");
    }

    LOG(DEBUG, COROUTINES) << "StackfulCoroutineManager::TerminateCoroutine() started";
    co->NativeCodeEnd();
    co->UpdateStatus(ThreadStatus::TERMINATING);

    {
        os::memory::LockHolder lList(coroListLock_);
        RemoveFromRegistry(co);
#ifdef ARK_HYBRID
        co->GetThreadHolder()->UnregisterCoroutine(co);
#endif
        // We need collect TLAB metrics and clear TLAB before calling the manage thread destructor
        // because of the possibility heap use after free. This happening when GC starts execute ResetYoungAllocator
        // method which start iterate set of threads, collect TLAB metrics and clear TLAB. If thread was deleted from
        // set but we haven't destroyed the thread yet, GC won't collect metrics and can complete TLAB
        // deletion faster. And when we try to get the TLAB metrics in the destructor of managed thread, we will get
        // heap use after free
        co->CollectTLABMetrics();
        co->ClearTLAB();
        // DestroyInternalResources()/CleanupInternalResources() must be called in one critical section with
        // RemoveFromRegistry (under core_list_lock_). This functions transfer cards from coro's post_barrier buffer to
        // UpdateRemsetThread internally. Situation when cards still remain and UpdateRemsetThread cannot visit the
        // coro (because it is already removed) must be impossible.
        if (Runtime::GetOptions().IsUseCoroutinePool() && co->HasManagedEntrypoint()) {
            co->CleanupInternalResources();
        } else {
            co->DestroyInternalResources();
        }
        co->UpdateStatus(ThreadStatus::FINISHED);
    }
    Runtime::GetCurrent()->GetNotificationManager()->ThreadEndEvent(co);

    if (co->HasManagedEntrypoint()) {
        CheckProgramCompletion();
        GetCurrentWorker()->RequestFinalization(co);
    } else if (co->HasNativeEntrypoint()) {
        CheckProgramCompletion();  // Should be removed after #29944
        GetCurrentWorker()->RequestFinalization(co);
    } else {
        // entrypointless and NOT native: e.g. MAIN
        // (do nothing, as entrypointless coroutines should be destroyed manually)
    }

    return false;
}

size_t StackfulCoroutineManager::GetActiveWorkersCount() const
{
    os::memory::LockHolder lkWorkers(workersLock_);
    return activeWorkersCount_;
}

size_t StackfulCoroutineManager::GetExistingWorkersCount() const
{
    os::memory::LockHolder lkWorkers(workersLock_);
    return workers_.size();
}

void StackfulCoroutineManager::CheckProgramCompletion()
{
    os::memory::LockHolder lkCompletion(programCompletionLock_);

    size_t activeWorkerCoros = GetActiveWorkersCount();
    if (coroutineCount_ <= 1 + activeWorkerCoros) {  // 1 here is for MAIN
        LOG(DEBUG, COROUTINES)
            << "StackfulCoroutineManager::CheckProgramCompletion(): all coroutines finished execution!";
        // programCompletionEvent_ acts as a stackful-friendly cond var
        programCompletionEvent_->Happen();
    } else {
        LOG(DEBUG, COROUTINES) << "StackfulCoroutineManager::CheckProgramCompletion(): still "
                               << coroutineCount_ - 1 - activeWorkerCoros << " coroutines exist...";
    }
}

CoroutineContext *StackfulCoroutineManager::CreateCoroutineContext(bool coroHasEntrypoint)
{
    return CreateCoroutineContextImpl(coroHasEntrypoint);
}

void StackfulCoroutineManager::DeleteCoroutineContext(CoroutineContext *ctx)
{
    FreeCoroutineStack(static_cast<StackfulCoroutineContext *>(ctx)->GetStackLoAddrPtr());
    Runtime::GetCurrent()->GetInternalAllocator()->Delete(ctx);
}

size_t StackfulCoroutineManager::GetCoroutineCount()
{
    return coroutineCount_;
}

size_t StackfulCoroutineManager::GetCoroutineCountLimit()
{
    return coroutineCountLimit_;
}

LaunchResult StackfulCoroutineManager::Launch(CompletionEvent *completionEvent, Method *entrypoint,
                                              PandaVector<Value> &&arguments, const CoroutineWorkerGroup::Id &groupId,
                                              CoroutinePriority priority, bool abortFlag)
{
    auto epInfo = Coroutine::ManagedEntrypointInfo {completionEvent, entrypoint, std::move(arguments)};
    return LaunchWithGroupId(std::move(epInfo), entrypoint->GetFullName(), groupId, priority, false, abortFlag);
}

LaunchResult StackfulCoroutineManager::LaunchImmediately(CompletionEvent *completionEvent, Method *entrypoint,
                                                         PandaVector<Value> &&arguments,
                                                         const CoroutineWorkerGroup::Id &groupId,
                                                         CoroutinePriority priority, bool abortFlag)
{
    auto epInfo = Coroutine::ManagedEntrypointInfo {completionEvent, entrypoint, std::move(arguments)};
    return LaunchWithGroupId(std::move(epInfo), entrypoint->GetFullName(), groupId, priority, true, abortFlag);
}

LaunchResult StackfulCoroutineManager::LaunchNative(NativeEntrypointFunc epFunc, void *param, PandaString coroName,
                                                    const CoroutineWorkerGroup::Id &groupId, CoroutinePriority priority,
                                                    bool launchImmediately, bool abortFlag)
{
    auto epInfo = Coroutine::NativeEntrypointInfo {epFunc, param};
    return LaunchWithGroupId(epInfo, std::move(coroName), groupId, priority, launchImmediately, abortFlag);
}

void StackfulCoroutineManager::Await(CoroutineEvent *awaitee)
{
    ASSERT_NATIVE_CODE();
    // profiling
    ScopedCoroutineStats s(&GetCurrentWorker()->GetPerfStats(), CoroutineTimeStats::SCH_ALL);

    ASSERT(awaitee != nullptr);
    [[maybe_unused]] auto *waiter = Coroutine::GetCurrent();
    LOG(DEBUG, COROUTINES) << "StackfulCoroutineManager::Await started by " + waiter->GetName();

    GetCurrentWorker()->WaitForEvent(awaitee);
    // NB: at this point the awaitee is likely already deleted
    LOG(DEBUG, COROUTINES) << "StackfulCoroutineManager::Await finished by " + waiter->GetName();
}

void StackfulCoroutineManager::UnblockWaiters(CoroutineEvent *blocker)
{
    // profiling: this function can be called either independently or as a path of some other SCH sequence,
    // hence using the "weak" stats collector
    ScopedCoroutineStats s(&GetCurrentWorker()->GetPerfStats(), CoroutineTimeStats::SCH_ALL, true);

    ASSERT(blocker != nullptr);
#ifndef NDEBUG
    {
        os::memory::LockHolder lkBlocker(*blocker);
        ASSERT(blocker->Happened());
    }
#endif

    os::memory::LockHolder lkWorkers(workersLock_);
    for (auto *w : workers_) {
        w->UnblockWaiters(blocker);
    }
}

void StackfulCoroutineManager::Schedule()
{
    // profiling
    ScopedCoroutineStats s(&GetCurrentWorker()->GetPerfStats(), CoroutineTimeStats::SCH_ALL);

    LOG(DEBUG, COROUTINES) << "StackfulCoroutineManager::Schedule() request from "
                           << Coroutine::GetCurrent()->GetName();
    GetCurrentWorker()->RequestSchedule();
}

bool StackfulCoroutineManager::EnumerateThreadsImpl(const ThreadManager::Callback &cb, unsigned int incMask,
                                                    unsigned int xorMask) const
{
    os::memory::LockHolder lock(coroListLock_);
    for (auto *t : coroutines_) {
        if (!ApplyCallbackToThread(cb, t, incMask, xorMask)) {
            return false;
        }
    }
    return true;
}

bool StackfulCoroutineManager::EnumerateWorkersImpl(const EnumerateWorkerCallback &cb) const
{
    os::memory::LockHolder lock(workersLock_);
    for (auto *w : workers_) {
        if (!cb(w)) {
            return false;
        }
    }
    return true;
}

void StackfulCoroutineManager::SuspendAllThreads()
{
    os::memory::LockHolder lock(coroListLock_);
    LOG(DEBUG, COROUTINES) << "StackfulCoroutineManager::SuspendAllThreads started";
    for (auto *t : coroutines_) {
        t->SuspendImpl(true);
    }
    LOG(DEBUG, COROUTINES) << "StackfulCoroutineManager::SuspendAllThreads finished";
}

void StackfulCoroutineManager::ResumeAllThreads()
{
    os::memory::LockHolder lock(coroListLock_);
    for (auto *t : coroutines_) {
        t->ResumeImpl(true);
    }
}

bool StackfulCoroutineManager::IsRunningThreadExist()
{
    UNREACHABLE();
    // NOTE(konstanting): correct implementation. Which coroutine do we consider running?
    return false;
}

void StackfulCoroutineManager::WaitForDeregistration()
{
    // profiling: start interval here, end in ctxswitch (if needed)
    GetCurrentWorker()->GetPerfStats().StartInterval(CoroutineTimeStats::SCH_ALL);
    //
    MainCoroutineCompleted();
}

void StackfulCoroutineManager::ReuseCoroutineInstance(Coroutine *co, EntrypointInfo &&epInfo, PandaString name,
                                                      CoroutinePriority priority)
{
    auto *ctx = co->GetContext<CoroutineContext>();
    co->ReInitialize(std::move(name), ctx, std::move(epInfo), priority);
}

Coroutine *StackfulCoroutineManager::TryGetCoroutineFromPool()
{
    os::memory::LockHolder lkPool(coroPoolLock_);
    if (coroutinePool_.empty()) {
        return nullptr;
    }
    Coroutine *co = coroutinePool_.back();
    coroutinePool_.pop_back();
    return co;
}

StackfulCoroutineWorker *StackfulCoroutineManager::ChooseWorkerForCoroutine(Coroutine *co)
{
    ASSERT(co != nullptr);
    auto maskValue = co->GetContext<StackfulCoroutineContext>()->GetAffinityMask();
    return ChooseWorkerImpl(WorkerSelectionPolicy::LEAST_LOADED, maskValue);
}

AffinityMask StackfulCoroutineManager::CalcAffinityMask(const CoroutineWorkerGroup::Id &groupId)
{
    ASSERT(groupId != CoroutineWorkerGroup::InvalidId());
    AffinityMask mask = AffinityMask::FromGroupId(groupId);
    switch (GetSchedulingPolicy()) {
        case CoroutineSchedulingPolicy::NON_MAIN_WORKER: {
            if (!CoroutineWorkerGroup::HasOnlyWorker(groupId, MAIN_WORKER_ID)) {
                mask.SetWorkerNotAllowed(MAIN_WORKER_ID);
            }
            break;
        }
        default:
        case CoroutineSchedulingPolicy::ANY_WORKER:
            break;
    }
    return mask;
}

Coroutine *StackfulCoroutineManager::GetCoroutineInstanceForLaunch(EntrypointInfo &&epInfo, PandaString &&coroName,
                                                                   CoroutinePriority priority,
                                                                   AffinityMask affinityMask, bool abortFlag)
{
    Coroutine *co = nullptr;
    if (Runtime::GetOptions().IsUseCoroutinePool()) {
        co = TryGetCoroutineFromPool();
    }
    if (co != nullptr) {
        ReuseCoroutineInstance(co, std::move(epInfo), std::move(coroName), priority);
    } else {
        co = CreateCoroutineInstance(std::move(epInfo), std::move(coroName), Coroutine::Type::MUTATOR, priority);
    }
    if (co == nullptr) {
        LOG(DEBUG, COROUTINES)
            << "StackfulCoroutineManager::GetCoroutineInstanceForLaunch: failed to create a coroutine!";
        return co;
    }
    co->SetAbortFlag(abortFlag);
    Runtime::GetCurrent()->GetNotificationManager()->ThreadStartEvent(co);
    co->GetContext<StackfulCoroutineContext>()->SetAffinityMask(affinityMask);
    return co;
}

LaunchResult StackfulCoroutineManager::LaunchImpl(EntrypointInfo &&epInfo, PandaString &&coroName,
                                                  const CoroutineWorkerGroup::Id &groupId, CoroutinePriority priority,
                                                  bool abortFlag)
{
#ifndef NDEBUG
    GetCurrentWorker()->PrintRunnables("LaunchImpl begin");
#endif
    Coroutine *co = nullptr;
    auto affinityMask = CalcAffinityMask(groupId);
    co = GetCoroutineInstanceForLaunch(std::move(epInfo), std::move(coroName), priority, affinityMask, abortFlag);
    if (co == nullptr) {
        LOG(DEBUG, COROUTINES) << "StackfulCoroutineManager::LaunchImpl: failed to create a coroutine!";
        return LaunchResult::COROUTINES_LIMIT_EXCEED;
    }
    {
        os::memory::LockHolder lkWorkers(workersLock_);
        auto *w = ChooseWorkerForCoroutine(co);
        if UNLIKELY (w == nullptr) {
            // We use this workaround for correct coroutine destruction, should be fixed by #29944
            co->SetEntrypointData(Coroutine::NativeEntrypointInfo {[]([[maybe_unused]] void *data) {}, co});
            GetCurrentWorker()->AddRunnableCoroutine(co);
            return LaunchResult::NO_SUITABLE_WORKER;
        }
        Coroutine::GetCurrent()->OnChildCoroutineCreated(co);
        w->AddRunnableCoroutine(co);
    }
#ifndef NDEBUG
    GetCurrentWorker()->PrintRunnables("LaunchImpl end");
#endif
    return LaunchResult::OK;
}

LaunchResult StackfulCoroutineManager::LaunchImmediatelyImpl(EntrypointInfo &&epInfo, PandaString &&coroName,
                                                             const CoroutineWorkerGroup::Id &groupId,
                                                             CoroutinePriority priority, bool abortFlag)
{
    Coroutine *co = nullptr;
    ASSERT(CoroutineWorkerGroup::HasOnlyWorker(groupId, Coroutine::GetCurrent()->GetWorker()->GetId()));
    auto affinityMask = CalcAffinityMask(groupId);

    co = GetCoroutineInstanceForLaunch(std::move(epInfo), std::move(coroName), priority, affinityMask, abortFlag);
    if (co == nullptr) {
        LOG(DEBUG, COROUTINES) << "StackfulCoroutineManager::LaunchImmediatelyImpl: failed to create a coroutine!";
        return LaunchResult::COROUTINES_LIMIT_EXCEED;
    }
    StackfulCoroutineWorker *w = nullptr;
    {
        os::memory::LockHolder lkWorkers(workersLock_);
        w = ChooseWorkerForCoroutine(co);
    }
    ASSERT(w != nullptr);
    // since we are going to switch the context, we have to close the interval
    GetCurrentWorker()->GetPerfStats().FinishInterval(CoroutineTimeStats::LAUNCH);
    co->SetImmediateLauncher(Coroutine::GetCurrent());
    Coroutine::GetCurrent()->OnChildCoroutineCreated(co);
    w->AddCreatedCoroutineAndSwitchToIt(co);
    // resume the interval once we schedule the original coro again
    GetCurrentWorker()->GetPerfStats().StartInterval(CoroutineTimeStats::LAUNCH);

    return LaunchResult::OK;
}

LaunchResult StackfulCoroutineManager::LaunchWithGroupId(Coroutine::EntrypointInfo &&epInfo, PandaString &&coroName,
                                                         CoroutineWorkerGroup::Id groupId, CoroutinePriority priority,
                                                         bool launchImmediately, bool abortFlag)
{
    // profiling: scheduler and launch time
    ScopedCoroutineStats sSch(&GetCurrentWorker()->GetPerfStats(), CoroutineTimeStats::SCH_ALL);
    ScopedCoroutineStats sLaunch(&GetCurrentWorker()->GetPerfStats(), CoroutineTimeStats::LAUNCH);

    LOG(DEBUG, COROUTINES) << "StackfulCoroutineManager::LaunchWithGroupId started";

    auto *co = Coroutine::GetCurrent();
    ASSERT(co != nullptr);
    auto *w = co->GetContext<StackfulCoroutineContext>()->GetWorker();
    if (groupId == CoroutineWorkerGroup::AnyId() && w->InExclusiveMode()) {
        groupId = ark::CoroutineWorkerGroup::GenerateExactWorkerId(w->GetId());
    }
    LaunchResult result = LaunchResult::OK;
    if (launchImmediately) {
        result = LaunchImmediatelyImpl(std::move(epInfo), std::move(coroName), groupId, priority, abortFlag);
    } else {
        result = LaunchImpl(std::move(epInfo), std::move(coroName), groupId, priority, abortFlag);
    }
    switch (result) {
        case LaunchResult::COROUTINES_LIMIT_EXCEED:
            ThrowCoroutinesLimitExceedError(
                "Unable to create a new coroutine: reached the limit for the number of existing coroutines.");
            break;
        case LaunchResult::NO_SUITABLE_WORKER:
            ThrowRuntimeException("Unable to launch coroutine: no suitable worker was found");
            break;
        case LaunchResult::OK:
            break;
        default:
            UNREACHABLE();
    }

    Tracer::Count(Tracer::LAUNCH, 1U);

    LOG(DEBUG, COROUTINES) << "StackfulCoroutineManager::LaunchWithGroupId finished";
    return result;
}

void StackfulCoroutineManager::DumpCoroutineStats() const
{
    LOG(DEBUG, COROUTINES) << "StackfulCoroutineManager: dumping performance statistics...";
    std::cout << "=== Coroutine statistics begin ===" << std::endl;
    std::cout << stats_.GetFullStatistics(finalizedWorkerStats_);
    std::cout << "=== Coroutine statistics end ===" << std::endl;
    LOG(DEBUG, COROUTINES) << "StackfulCoroutineManager: performance statistics dumped successfully.";
}

void StackfulCoroutineManager::ListUnhandledEventsOnProgramExit()
{
    auto *coro = Coroutine::GetCurrent();
    ASSERT(coro != nullptr);
    coro->ListUnhandledEventsOnProgramExit();
}

void StackfulCoroutineManager::WaitForNonMainCoroutinesCompletion()
{
    os::memory::LockHolder lkCompletion(programCompletionLock_);
    // It's neccessary to read activeWorkersCount before coroutineCount to avoid deadlock
    do {
        while (GetActiveWorkersCount() + 1 < coroutineCount_) {  // 1 is for MAIN
            programCompletionEvent_->SetNotHappened();
            programCompletionEvent_->Lock();
            programCompletionLock_.Unlock();
            GetCurrentWorker()->WaitForEvent(programCompletionEvent_);
            LOG(DEBUG, COROUTINES) << "StackfulCoroutineManager::WaitForNonMainCoroutinesCompletion(): possibly "
                                      "spurious wakeup from wait...";
            // NOTE(konstanting, #IAD5MH): test for the spurious wakeup
            programCompletionLock_.Lock();
        }
        programCompletionLock_.Unlock();
        ListUnhandledEventsOnProgramExit();
        programCompletionLock_.Lock();
    } while (GetActiveWorkersCount() + 1 < coroutineCount_);  // 1 is for MAIN
    // coroutineCount_ < 1 + GetActiveWorkersCount() in case of concurrent EWorker destroy
    // in this case coroutineCount_ >= 1 + GetActiveWorkersCount() - ExclusiveWorkersCount()
    ASSERT(!(GetActiveWorkersCount() + 1 < coroutineCount_));
}

void StackfulCoroutineManager::MainCoroutineCompleted()
{
    // precondition: MAIN is already in the native mode
    LOG(DEBUG, COROUTINES) << "StackfulCoroutineManager::MainCoroutineCompleted(): STARTED";
    // block till only schedule loop coroutines are present
    LOG(DEBUG, COROUTINES)
        << "StackfulCoroutineManager::MainCoroutineCompleted(): waiting for other coroutines to complete";
    GetCurrentWorker()->DestroyCallbackPoster();
    WaitForNonMainCoroutinesCompletion();

    LOG(DEBUG, COROUTINES) << "StackfulCoroutineManager::MainCoroutineCompleted(): stopping workers";
    {
        os::memory::LockHolder lock(workersLock_);
        for (auto *worker : workers_) {
            worker->SetActive(false);
        }
        while (activeWorkersCount_ > 1) {  // 1 is for MAIN
            // profiling: the SCH interval is expected to be started after the ctxswitch
            GetCurrentWorker()->GetPerfStats().FinishInterval(CoroutineTimeStats::SCH_ALL);
            // NOTE(konstanting, #IAD5MH): need timed wait?..
            workersCv_.Wait(&workersLock_);
            // profiling: we don't want to profile the sleeping state
            GetCurrentWorker()->GetPerfStats().StartInterval(CoroutineTimeStats::SCH_ALL);
        }
    }
    // Only system coroutines and current coro (MAIN) are left (1 is for MAIN)
    ASSERT(activeCoroutines_ == utilityCoroutineCount_ + 1);

    LOG(DEBUG, COROUTINES)
        << "StackfulCoroutineManager::MainCoroutineCompleted(): stopping await loop on the main worker";
    while (coroutineCount_ > 1) {
        GetCurrentWorker()->FinalizeFiberScheduleLoop();
    }
    // profiling: the SCH interval is expected to be started after the ctxswitch
    GetCurrentWorker()->GetPerfStats().FinishInterval(CoroutineTimeStats::SCH_ALL);

    OnWorkerShutdown(GetCurrentWorker());

#ifndef NDEBUG
    {
        os::memory::LockHolder lkWorkers(workersLock_);
        ASSERT(workers_.empty());
        ASSERT(activeWorkersCount_ == 0);
    }
#endif

    if (stats_.IsEnabled()) {
        DumpCoroutineStats();
    }
    stats_.Disable();

    // We need to lock programCompletionLock_ here to call
    // programCompletionLock_.Unlock() in ExclusiveWorker before runtime destruction
    os::memory::LockHolder lkCompletion(programCompletionLock_);
    GetCurrentContext()->MainThreadFinished();
    // MAIN finished, all workers are deleted, no active coros remain
    ASSERT(activeCoroutines_ == 0);
    LOG(DEBUG, COROUTINES) << "StackfulCoroutineManager::MainCoroutineCompleted(): DONE";
}

StackfulCoroutineContext *StackfulCoroutineManager::GetCurrentContext()
{
    auto *co = Coroutine::GetCurrent();
    ASSERT(co != nullptr);
    return co->GetContext<StackfulCoroutineContext>();
}

StackfulCoroutineWorker *StackfulCoroutineManager::GetCurrentWorker()
{
    return GetCurrentContext()->GetWorker();
}

void StackfulCoroutineManager::DestroyEntrypointfulCoroutine(Coroutine *co)
{
    if (Runtime::GetOptions().IsUseCoroutinePool() && co->HasManagedEntrypoint()) {
        co->CleanUp();
        os::memory::LockHolder lock(coroPoolLock_);
        coroutinePool_.push_back(co);
    } else {
        CoroutineManager::DestroyEntrypointfulCoroutine(co);
    }
}

StackfulCoroutineContext *StackfulCoroutineManager::CreateCoroutineContextImpl(bool needStack)
{
    uint8_t *stack = nullptr;
    size_t stackSizeBytes = 0;
    if (needStack) {
        stack = AllocCoroutineStack();
        if (stack == nullptr) {
            return nullptr;
        }
        stackSizeBytes = coroStackSizeBytes_;
    }
    return Runtime::GetCurrent()->GetInternalAllocator()->New<StackfulCoroutineContext>(stack, stackSizeBytes);
}

Coroutine *StackfulCoroutineManager::CreateNativeCoroutine(Runtime *runtime, PandaVM *vm,
                                                           Coroutine::NativeEntrypointInfo::NativeEntrypointFunc entry,
                                                           void *param, PandaString name, Coroutine::Type type,
                                                           CoroutinePriority priority)
{
    if (GetCoroutineCount() >= GetCoroutineCountLimit()) {
        // resource limit reached
        return nullptr;
    }
    if (!IsSystemCoroutine(type, true) && IsUserCoroutineLimitReached()) {
        // user coro limit reached
        return nullptr;
    }
    StackfulCoroutineContext *ctx = CreateCoroutineContextImpl(true);
    if (ctx == nullptr) {
        // do not proceed if we cannot create a context for the new coroutine
        return nullptr;
    }
    auto *co = GetCoroutineFactory()(runtime, vm, std::move(name), ctx, Coroutine::NativeEntrypointInfo(entry, param),
                                     type, priority);
    ASSERT(co != nullptr);

    // Let's assume that even the "native" coroutine can eventually try to execute some managed code.
    // In that case pre/post barrier buffers are necessary.
    co->InitBuffers();
    return co;
}

void StackfulCoroutineManager::DestroyNativeCoroutine(Coroutine *co)
{
    DestroyEntrypointlessCoroutine(co);
}

void StackfulCoroutineManager::OnCoroBecameActive(Coroutine *co)
{
    ASSERT(co->IsActive());
    IncrementActiveCoroutines();
    co->GetWorker()->OnCoroBecameActive(co);
}

void StackfulCoroutineManager::OnCoroBecameNonActive([[maybe_unused]] Coroutine *co)
{
    ASSERT(!co->IsActive());
    DecrementActiveCoroutines();
}

void StackfulCoroutineManager::OnNativeCallExit(Coroutine *co)
{
    if (GetConfig().enableDrainQueueIface) {
        // A temporary hack for draining the coroutine queue on the current worker.
        // Will stay there until we have the proper design for the execution model and
        // the rules for interaction with the app framework.
        LOG(DEBUG, COROUTINES) << "StackfulCoroutineManager::OnNativeCallExit(): START DRAINING COROQUEUE";

        // precondition: the event is handled for the current coroutine
        ASSERT(co == Coroutine::GetCurrent());
        auto *worker = GetCurrentWorker();
        if (!worker->IsMainWorker() && !worker->InExclusiveMode()) {
            return;
        }
        ScopedNativeCodeThread nativeCode(co);
        while (worker->GetRunnablesCount(Coroutine::Type::MUTATOR) > 0) {
            GetCurrentWorker()->RequestSchedule();
        }

        LOG(DEBUG, COROUTINES) << "StackfulCoroutineManager::OnNativeCallExit(): STOP DRAINING COROQUEUE";
    }
}

void StackfulCoroutineManager::IncrementActiveCoroutines()
{
    activeCoroutines_++;
}

void StackfulCoroutineManager::DecrementActiveCoroutines()
{
    [[maybe_unused]] uint32_t old = activeCoroutines_--;
    ASSERT(old > 0);
}

bool StackfulCoroutineManager::IsNoActiveMutatorsExceptCurrent()
{
    // activeCoroutines_ == 1 means that only current or main mutator is left
    // activeCoroutines_ == 0 means that main mutator is terminating
    // or all coroutines are blocked (a deadlock in managed code happened)

    // NOTE(konstanting): need to reevaluate the necessity of locks here as
    // atomics difference is somewhat confusing. Also, we may have concurrent access to them.
    return (activeCoroutines_ - utilityCoroutineCount_) <= 1;
}

Coroutine *StackfulCoroutineManager::CreateExclusiveWorkerForThread(Runtime *runtime, PandaVM *vm)
{
    ASSERT(Thread::GetCurrent() == nullptr);

    // actually we need this lock due to worker limit
    os::memory::LockHolder eWorkerLock(eWorkerCreationLock_);

    if (IsExclusiveWorkersLimitReached()) {
        LOG(DEBUG, COROUTINES) << "The program reached the limit of exclusive workers";
        return nullptr;
    }

    auto *eWorker = CreateWorker(runtime, vm, StackfulCoroutineWorker::ScheduleLoopType::FIBER, "[e-worker] ");
    {
        os::memory::LockHolder lh(workerIdLock_);
        eaWorkerGroup_ = CoroutineWorkerGroup::ExtendGroup(eaWorkerGroup_, eWorker->GetId());
    }
    ASSERT(eWorker != nullptr);
    eWorker->SetExclusiveMode(true);
    auto *eCoro = CreateEntrypointlessCoroutine(runtime, vm, true, "[ea_coro] " + eWorker->GetName(),
                                                Coroutine::Type::MUTATOR, CoroutinePriority::MEDIUM_PRIORITY);
    ASSERT(eCoro != nullptr);
    eWorker->AddRunningCoroutine(eCoro);
    OnWorkerStartup(eWorker);

    ASSERT(Coroutine::GetCurrent() == eCoro);
    Runtime::GetCurrent()->GetNotificationManager()->ThreadStartEvent(eCoro);
    return eCoro;
}

bool StackfulCoroutineManager::DestroyExclusiveWorker()
{
    auto *eWorker = GetCurrentWorker();
    if (!eWorker->InExclusiveMode()) {
        LOG(DEBUG, COROUTINES) << "Trying to destroy not exclusive worker";
        return false;
    }

    {
        os::memory::LockHolder lock(workersLock_);
        eWorker->DisableForCrossWorkersLaunch();
    }

    eWorker->DestroyCallbackPoster();
    eWorker->CompleteAllAffinedCoroutines();

    eWorker->SetActive(false);
    eWorker->FinalizeFiberScheduleLoop();

    CheckProgramCompletion();

    auto *eaCoro = Coroutine::GetCurrent();
    programCompletionLock_.Lock();
    DestroyEntrypointlessCoroutine(eaCoro);
    Coroutine::SetCurrent(nullptr);

    OnWorkerShutdown(eWorker);
    programCompletionLock_.Unlock();
    return true;
}

bool StackfulCoroutineManager::IsExclusiveWorkersLimitReached() const
{
    bool limitIsReached = GetActiveWorkersCount() - commonWorkersCount_ >= exclusiveWorkersLimit_;
    LOG_IF(limitIsReached, DEBUG, COROUTINES) << "The programm reached the limit of exclusive workers";
    return limitIsReached;
}

bool StackfulCoroutineManager::IsUserCoroutineLimitReached() const
{
    bool limitIsReached = userCoroutineCount_ >= userCoroutineCountLimit_;
    LOG_IF(limitIsReached, DEBUG, COROUTINES)
        << "The programm reached the limit of user coroutines " << userCoroutineCountLimit_;
    return limitIsReached;
}

bool StackfulCoroutineManager::MigrateCoroutinesInward(StackfulCoroutineWorker *to)
{
    if (!GetConfig().enableMigration) {
        LOG(DEBUG, COROUTINES) << "Migration is not supported.";
        return false;
    }
    if (to->IsMainWorker() || to->InExclusiveMode()) {
        return false;
    }

    auto affinityMask = CalcAffinityMask(CoroutineWorkerGroup::AnyId());
    os::memory::LockHolder lkWorkers(workersLock_);
    StackfulCoroutineWorker *from = ChooseWorkerImpl(WorkerSelectionPolicy::MOST_LOADED, affinityMask);
    if (from == nullptr || from->IsIdle()) {
        LOG(DEBUG, COROUTINES) << "StackfulCoroutineManager::MigrateCoroutinesInward : no suitable worker.";
        return false;
    }

    return to->MigrateFrom(from);
}

StackfulCoroutineWorker *StackfulCoroutineManager::ChooseWorkerImpl(WorkerSelectionPolicy policy, AffinityMask mask)
{
    auto preferFirstOverSecond = [policy](const StackfulCoroutineWorker *first, const StackfulCoroutineWorker *second) {
        // choosing the least loaded worker from the allowed worker set
        if (policy == WorkerSelectionPolicy::LEAST_LOADED) {
            return first->GetLoadFactor() < second->GetLoadFactor();
        }
        // choosing the most loaded worker from the allowed worker set
        return first->GetLoadFactor() > second->GetLoadFactor();
    };

    if (workers_.empty()) {
        LOG(DEBUG, COROUTINES) << "workers is empty.";
        return nullptr;
    }
#ifndef NDEBUG
    LOG(DEBUG, COROUTINES) << "Evaluating load factors:";
    for (auto w : workers_) {
        LOG(DEBUG, COROUTINES) << w->GetName() << ": LF = " << w->GetLoadFactor();
    }
#endif
    std::vector<StackfulCoroutineWorker *> suitableWorkers;
    std::copy_if(workers_.begin(), workers_.end(), std::back_inserter(suitableWorkers), [this, mask](auto *w) {
        auto isMasked = mask.IsWorkerAllowed(w->GetId());
        auto isSameWorker = !GetConfig().enableMigration && (GetCurrentWorker() == w);
        return isMasked && (isSameWorker || !w->IsDisabledForCrossWorkersLaunch());
    });
    if (UNLIKELY(suitableWorkers.empty())) {
        return nullptr;
    }
    auto wIt = std::min_element(suitableWorkers.begin(), suitableWorkers.end(), preferFirstOverSecond);
    LOG(DEBUG, COROUTINES) << "Chose worker: " << (*wIt)->GetName();

    return *wIt;
}

void StackfulCoroutineManager::MigrateAwakenedCoro(Coroutine *co)
{
    os::memory::LockHolder lkWorkers(workersLock_);
    auto *w = ChooseWorkerForCoroutine(co);
    ASSERT(w != nullptr);
    w->AddRunnableCoroutine(co);
}

PandaUniquePtr<StackfulCoroutineStateInfoTable> StackfulCoroutineManager::GetAllWorkerFullStatus() const
{
    os::memory::LockHolder lkWorkers(workersLock_);
    auto infoTable = MakePandaUnique<StackfulCoroutineStateInfoTable>();
    for (auto *worker : workers_) {
        infoTable->AddWorker(worker);
    }
    return infoTable;
}

static CoroutineWorkerGroup::Id TryApplyHint(const CoroutineWorkerGroup::Id &group,
                                             const PandaVector<CoroutineWorker::Id> &hint)
{
    CoroutineWorkerGroup::Id hintGroup = CoroutineWorkerGroup::Empty();
    for (auto h : hint) {
        ASSERT(h != CoroutineWorker::INVALID_ID);
        hintGroup = CoroutineWorkerGroup::ExtendGroup(hintGroup, h, false);
    }
    return ((group & hintGroup) != CoroutineWorkerGroup::Empty()) ? (group & hintGroup) : group;
}

CoroutineWorkerGroup::Id StackfulCoroutineManager::GenerateWorkerGroupId(CoroutineWorkerDomain domain,
                                                                         const PandaVector<CoroutineWorker::Id> &hint)
{
    switch (domain) {
        case CoroutineWorkerDomain::GENERAL:
            return TryApplyHint(generalWorkerGroup_, hint);
        case CoroutineWorkerDomain::EXACT_ID:
            return TryApplyHint(CoroutineWorkerGroup::AnyId(), hint);
        case CoroutineWorkerDomain::MAIN:
            // Ignore hint
            return CoroutineWorkerGroup::GenerateExactWorkerId(MAIN_WORKER_ID);
        case CoroutineWorkerDomain::EA:
            return TryApplyHint(eaWorkerGroup_, hint);
    }
    UNREACHABLE();
    return CoroutineWorkerGroup::InvalidId();
}

void StackfulCoroutineManager::PreZygoteFork()
{
    WaitForNonMainCoroutinesCompletion();

    FinalizeWorkers(commonWorkersCount_ - 1, Runtime::GetCurrent(), Runtime::GetCurrent()->GetPandaVM());
}

void StackfulCoroutineManager::PostZygoteFork()
{
    Runtime *runtime = Runtime::GetCurrent();
    CreateWorkers(commonWorkersCount_ - 1, runtime, runtime->GetPandaVM());
}

void StackfulCoroutineManager::CalculateUserCoroutinesLimits(size_t &userCoroutineCountLimit, size_t limit)
{
    // for general workers: 2 = 1 EP-less for THREAD schedule + 1 for potential finalizer
    // for MAIN and EA: 2 = 1 for FIBER schedule loop + 1 for EP-less mutator coro
    constexpr size_t SYSTEM_COROS_PER_WORKER = 2;
    size_t estimatedSystemCoroCount = SYSTEM_COROS_PER_WORKER * (commonWorkersCount_ + exclusiveWorkersLimit_);

    ASSERT(coroutineCountLimit_ > estimatedSystemCoroCount);
    constexpr size_t USER_COROUTINE_LIMIT = 7000;
    size_t userCoroutineMaxLimit = std::min(USER_COROUTINE_LIMIT, coroutineCountLimit_ - estimatedSystemCoroCount);

    if (limit == 0) {  // Auto set
        LOG(DEBUG, COROUTINES)
            << "StackfulCoroutineManager(): AUTO mode selected, will set number of user coroutine limit to it maximum: "
            << userCoroutineMaxLimit;
        limit = userCoroutineMaxLimit;           // Do not limit dedicated
    } else if (limit > userCoroutineMaxLimit) {  // Exceed possible limit
        LOG(DEBUG, COROUTINES)
            << "StackfulCoroutineManager(): Setted user coroutine limit exceed maximum. Cutting to maximum: "
            << userCoroutineMaxLimit;
        limit = userCoroutineMaxLimit;
    }
    userCoroutineCountLimit = limit;
}

void StackfulCoroutineManager::CalculateWorkerLimits(size_t &exclusiveWorkersLimit, size_t &commonWorkersLimit)
{
    // 1 is for MAIN
    size_t eWorkersLimit =
        std::min(AffinityMask::MAX_WORKERS_COUNT - 1, static_cast<size_t>(GetConfig().exclusiveWorkersLimit));

    // add preallocated exclusive workers count
    eWorkersLimit += GetConfig().preallocatedExclusiveWorkersCount;

    // create and activate workers
    size_t numberOfAvailableCores = std::max(std::thread::hardware_concurrency() / 4ULL, 2ULL);

    // workaround for issue #21582
    const size_t maxCommonWorkers =
        std::max(AffinityMask::MAX_WORKERS_COUNT - eWorkersLimit, static_cast<size_t>(2ULL));

    commonWorkersLimit = (GetConfig().workersCount == CoroutineManagerConfig::WORKERS_COUNT_AUTO)
                             ? std::min(numberOfAvailableCores, maxCommonWorkers)
                             : std::min(static_cast<size_t>(GetConfig().workersCount), maxCommonWorkers);
    if (GetConfig().workersCount == CoroutineManagerConfig::WORKERS_COUNT_AUTO) {
        LOG(DEBUG, COROUTINES) << "StackfulCoroutineManager(): AUTO mode selected, will set number of coroutine "
                                  "common workers to number of CPUs / 4, but not less than 2 and no more than "
                               << maxCommonWorkers << " = " << commonWorkersLimit;
    }
    ASSERT(commonWorkersLimit > 0);

    exclusiveWorkersLimit = std::min(AffinityMask::MAX_WORKERS_COUNT - commonWorkersLimit, eWorkersLimit);

    LOG(DEBUG, COROUTINES) << "StackfulCoroutineManager(): EWorkers limit is set to " << exclusiveWorkersLimit
                           << ", when suggested " << eWorkersLimit;
}
}  // namespace ark
