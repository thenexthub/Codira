/**
 * Copyright (c) 2023-2026 Huawei Device Co., Ltd.
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

#include "libarkbase/os/time.h"
#include "runtime/include/thread_scopes.h"
#include "runtime/coroutines/stackful_coroutine_manager.h"
#include "runtime/coroutines/stackful_coroutine.h"
#include "runtime/coroutines/stackful_coroutine_worker.h"
#include <algorithm>

namespace ark {

StackfulCoroutineWorker::StackfulCoroutineWorker(Runtime *runtime, PandaVM *vm, StackfulCoroutineManager *coroManager,
                                                 ScheduleLoopType type, PandaString name, Id id, bool isMainWorker)
    : CoroutineWorker(runtime, vm, name, id, isMainWorker),
      coroManager_(coroManager),
      threadId_(os::thread::GetCurrentThreadId()),
      hasRunnableCoroEvent_(coroManager),
      stats_(std::move(name))
{
    LOG(DEBUG, COROUTINES) << "Created a coroutine worker instance: id=" << GetId() << " name=" << GetName();
    if (type == ScheduleLoopType::THREAD) {
        std::thread t(&StackfulCoroutineWorker::ThreadProc, this);
        os::thread::SetThreadName(t.native_handle(), GetName().c_str());
        t.detach();
        // will create the schedule loop coroutine in the thread proc in order to set the stack protector correctly
    } else {
        scheduleLoopCtx_ = coroManager->CreateNativeCoroutine(GetRuntime(), GetPandaVM(), ScheduleLoopProxy, this,
                                                              "[fiber_sch] " + GetName(), Coroutine::Type::SCHEDULER,
                                                              CoroutinePriority::MEDIUM_PRIORITY);
        ASSERT(scheduleLoopCtx_ != nullptr);
        scheduleLoopCtx_->LinkToExternalHolder(true);
        AddRunnableCoroutine(scheduleLoopCtx_);
    }
}

void StackfulCoroutineWorker::AddRunnableCoroutine(Coroutine *newCoro)
{
    ASSERT(newCoro != nullptr);
    os::memory::LockHolder lock(runnablesLock_);
    PushToRunnableQueue(newCoro, newCoro->GetPriority());
    RegisterIncomingActiveCoroutine(newCoro);
}

void StackfulCoroutineWorker::AddRunningCoroutine(Coroutine *newCoro)
{
    ASSERT(newCoro != nullptr);
    RegisterIncomingActiveCoroutine(newCoro);
}

void StackfulCoroutineWorker::AddCreatedCoroutineAndSwitchToIt(Coroutine *newCoro)
{
    // precondition: called within the current worker, no cross-worker calls allowed
    ASSERT(GetCurrentContext()->GetWorker() == this);
    RegisterIncomingActiveCoroutine(newCoro);

    // suspend current coro...
    auto *coro = Coroutine::GetCurrent();
    ScopedNativeCodeThread n(coro);
    coro->RequestSuspend(false);
    // ..and resume the new one
    auto *currentCtx = GetCurrentContext();
    auto *nextCtx = newCoro->GetContext<StackfulCoroutineContext>();
    nextCtx->RequestResume();
    Coroutine::SetCurrent(newCoro);

    SwitchCoroutineContext(currentCtx, nextCtx);

    // process finalization queue once this coro gets scheduled again
    FinalizeTerminatedCoros();
}

void StackfulCoroutineWorker::WaitForEvent(CoroutineEvent *awaitee)
{
    // precondition: this method is not called by the schedule loop coroutine

    Coroutine *waiter = Coroutine::GetCurrent();
    ASSERT(GetCurrentContext()->GetWorker() == this);
    ASSERT(awaitee != nullptr);
    ASSERT(waiter->IsInNativeCode());

    ProcessTimerEvents();

    if (awaitee->Happened()) {
        awaitee->Unlock();
        LOG(DEBUG, COROUTINES) << "StackfulCoroutineWorker::WaitForEvent finished (no await happened)";
        return;
    }

    ASSERT(!awaitee->Happened());

    waitersLock_.Lock();
    awaitee->Unlock();
    LOG(DEBUG, COROUTINES) << "StackfulCoroutineWorker::AddWaitingCoroutine: " << waiter->GetName() << " AWAITS";
    [[maybe_unused]] auto [_, inserted] = waiters_.insert({awaitee, waiter});
    ASSERT(inserted);

    runnablesLock_.Lock();
    if (!RunnableCoroutinesExist()) {
        runnablesLock_.Unlock();
        auto coroManager = static_cast<StackfulCoroutineManager *>(waiter->GetManager());
        LOG(FATAL, COROUTINES) << coroManager->GetAllWorkerFullStatus()->OutputInfo();
        UNREACHABLE();
    }
    // will unlock waiters_lock_ and switch ctx.
    // NB! If migration on await is enabled, current coro can migrate to another worker, so
    // IsCrossWorkerCall() will become true after resume!
    BlockCurrentCoroAndScheduleNext();
}

void StackfulCoroutineWorker::UnblockWaiters(CoroutineEvent *blocker)
{
    bool canMigrateAwakened = coroManager_->GetConfig().migrateAwakenedCoros && !IsMainWorker() && !InExclusiveMode();
    Coroutine *unblockedCoro = nullptr;
    {
        os::memory::LockHolder lockW(waitersLock_);
        auto w = waiters_.find(blocker);
        if (w != waiters_.end()) {
            unblockedCoro = w->second;
            waiters_.erase(w);
            if (!canMigrateAwakened) {
                os::memory::LockHolder lockR(runnablesLock_);
                unblockedCoro->RequestUnblock();
                PushToRunnableQueue(unblockedCoro, unblockedCoro->GetPriority());
            } else {
                // (wangyuzhong,#24880): in case of migrateAwakenedCoros == true we need to correctly issue the
                // external scheduler request from the correct worker. Here the coroutine becomes Active on one
                // worker(which causes the request to be sent) and then gets potentially transferred to another worker.
                unblockedCoro->RequestUnblock();
            }
        }
    }
    if (unblockedCoro == nullptr) {
        LOG(DEBUG, COROUTINES) << "The coroutine is nullptr.";
        return;
    }
    if (canMigrateAwakened) {
        coroManager_->MigrateAwakenedCoro(unblockedCoro);
        return;
    }
    if (blocker != &hasRunnableCoroEvent_) {
        hasRunnableCoroEvent_.Happen();
    }
}

void StackfulCoroutineWorker::RequestFinalization(Coroutine *finalizee)
{
    // precondition: current coro and finalizee belong to the current worker
    ASSERT(finalizee->GetWorker() == this);
    ASSERT(GetCurrentContext()->GetWorker() == this);

    ProcessTimerEvents();

#ifdef ARK_HYBRID
    finalizee->UnbindMutator();
#endif
    finalizationQueue_.push(finalizee);
    // finalizee will never be scheduled again
    ScheduleNextCoroUnlockNone();
}

void StackfulCoroutineWorker::RequestSchedule()
{
    auto *current = Coroutine::GetCurrent();
    if (!IsIdle()) {
        TriggerSchedulerExternally(current);
    }
    RequestScheduleImpl();
}

void StackfulCoroutineWorker::FinalizeFiberScheduleLoop()
{
    ASSERT(GetCurrentContext()->GetWorker() == this);

    // part of MAIN finalization sequence
    if (RunnableCoroutinesExist()) {
        // the schedule loop is still runnable
        ASSERT(scheduleLoopCtx_->HasNativeEntrypoint());
        runnablesLock_.Lock();
        // sch loop only
        ASSERT(runnables_.Size() == 1);
        SuspendCurrentCoroAndScheduleNext();
        ASSERT(!IsCrossWorkerCall());
    }
}

void StackfulCoroutineWorker::CompleteAllAffinedCoroutines()
{
    ASSERT(IsDisabledForCrossWorkersLaunch());

    while (ProcessAsyncWork()) {
    }
}

bool StackfulCoroutineWorker::ProcessAsyncWork()
{
    ASSERT_NATIVE_CODE();
    ASSERT(GetCurrentContext()->GetWorker() == this);

    bool asyncWorkExists = false;
    // CC-OFFNXT(G.FMT.04-CPP): project code style
    auto lock = [](auto &&...locks) { ([&]() NO_THREAD_SAFETY_ANALYSIS { locks.Lock(); }(), ...); };
    // CC-OFFNXT(G.FMT.04-CPP): project code style
    auto unlock = [](auto &&...locks) { ([&]() NO_THREAD_SAFETY_ANALYSIS { locks.Unlock(); }(), ...); };
    {
        lock(waitersLock_, runnablesLock_);
        if (runnables_.Size() > 1) {
            unlock(waitersLock_, runnablesLock_);
            coroManager_->Schedule();
            asyncWorkExists = true;
        } else if (!waiters_.empty()) {
            hasRunnableCoroEvent_.Lock();
            hasRunnableCoroEvent_.SetNotHappened();
            unlock(waitersLock_, runnablesLock_);
            coroManager_->Await(&hasRunnableCoroEvent_);
            asyncWorkExists = true;
        } else {
            unlock(waitersLock_, runnablesLock_);
        }
    }

    asyncWorkExists |= GetPandaVM()->RunEventLoop(ark::EventLoopRunMode::RUN_NOWAIT);

    return asyncWorkExists;
}

void StackfulCoroutineWorker::OnNewCoroutineStartup(Coroutine *co)
{
    CoroutineWorker::OnNewCoroutineStartup(co);
    FinalizeTerminatedCoros();
}

void StackfulCoroutineWorker::DisableCoroutineSwitch()
{
    ++disableCoroSwitchCounter_;
    LOG(DEBUG, COROUTINES) << "Coroutine switch on " << GetName()
                           << " has been disabled! Recursive ctr = " << disableCoroSwitchCounter_;
}

void StackfulCoroutineWorker::EnableCoroutineSwitch()
{
    ASSERT(IsCoroutineSwitchDisabled());
    --disableCoroSwitchCounter_;
    LOG(DEBUG, COROUTINES) << "Coroutine switch on " << GetName()
                           << " has been enabled! Recursive ctr = " << disableCoroSwitchCounter_;
}

bool StackfulCoroutineWorker::IsCoroutineSwitchDisabled()
{
    return disableCoroSwitchCounter_ > 0;
}

#ifndef NDEBUG
void StackfulCoroutineWorker::PrintRunnables(const PandaString &requester)
{
    os::memory::LockHolder lock(runnablesLock_);
    LOG(DEBUG, COROUTINES) << "[" << requester << "] ";
    runnables_.IterateOverCoroutines([](Coroutine *co) { LOG(DEBUG, COROUTINES) << co->GetName() << " <"; });
    LOG(DEBUG, COROUTINES) << "X";
}
#endif

size_t StackfulCoroutineWorker::GetRunnablesCount(Coroutine::Type type)
{
    os::memory::LockHolder lock(runnablesLock_);
    size_t runnablesCount = 0;
    runnables_.IterateOverCoroutines([type, &runnablesCount](Coroutine *coro) {
        if (coro->GetType() == type) {
            runnablesCount++;
        }
    });
    return runnablesCount;
}

void StackfulCoroutineWorker::ThreadProc()
{
    threadId_ = os::thread::GetCurrentThreadId();
    scheduleLoopCtx_ =
        coroManager_->CreateEntrypointlessCoroutine(GetRuntime(), GetPandaVM(), false, "[thr_sch] " + GetName(),
                                                    Coroutine::Type::SCHEDULER, CoroutinePriority::MEDIUM_PRIORITY);
    ASSERT(scheduleLoopCtx_ != nullptr);
    scheduleLoopCtx_->LinkToExternalHolder(false);
    Coroutine::SetCurrent(scheduleLoopCtx_);
    scheduleLoopCtx_->RequestResume();
    AddRunningCoroutine(scheduleLoopCtx_);
    scheduleLoopCtx_->NativeCodeBegin();
    coroManager_->OnWorkerStartup(this);

    // profiling: start interval here, end in ctxswitch
    stats_.StartInterval(CoroutineTimeStats::SCH_ALL);
    ScheduleLoopBody();

    coroManager_->DestroyEntrypointlessCoroutine(scheduleLoopCtx_);
    ASSERT(threadId_ == os::thread::GetCurrentThreadId());
    coroManager_->OnWorkerShutdown(this);
}

void StackfulCoroutineWorker::ScheduleLoop()
{
    LOG(DEBUG, COROUTINES) << "[" << GetName() << "] Schedule loop called!";
    ScheduleLoopBody();
}

void StackfulCoroutineWorker::ScheduleLoopBody()
{
    // run the loop
    while (IsActive()) {
        RequestScheduleImpl();
        os::memory::LockHolder lkRunnables(runnablesLock_);
        UpdateLoadFactor();
    }
}

void StackfulCoroutineWorker::ScheduleLoopProxy(void *worker)
{
    static_cast<StackfulCoroutineWorker *>(worker)->ScheduleLoop();
}

void StackfulCoroutineWorker::PushToRunnableQueue(Coroutine *co, CoroutinePriority priority)
{
    runnables_.Push(co, priority);
    UpdateLoadFactor();
    runnablesCv_.Signal();
}

Coroutine *StackfulCoroutineWorker::PopFromRunnableQueue()
{
    os::memory::LockHolder lock(runnablesLock_);
    ASSERT(!runnables_.Empty());
    auto [co, _] = runnables_.Pop();
    UpdateLoadFactor();
    return co;
}

bool StackfulCoroutineWorker::RunnableCoroutinesExist() const
{
    os::memory::LockHolder lock(runnablesLock_);
    return !runnables_.Empty();
}

void StackfulCoroutineWorker::WaitForRunnables(uint64_t waitingTimeMs)
{
    // NOTE(konstanting): in case of work stealing, use timed wait and try periodically to steal some runnables
    while (!RunnableCoroutinesExist() && IsActive()) {
        // profiling: no need to profile the SLEEPING state, closing the interval
        stats_.FinishInterval(CoroutineTimeStats::SCH_ALL);
        if (waitingTimeMs != WAITING_TIME_UNLIMITED) {
            runnablesCv_.TimedWait(&runnablesLock_, waitingTimeMs);
        } else {
            runnablesCv_.Wait(&runnablesLock_);
        }

        // profiling: reopening the interval after the sleep
        stats_.StartInterval(CoroutineTimeStats::SCH_ALL);
        if (!RunnableCoroutinesExist() && IsActive()) {
            LOG(DEBUG, COROUTINES) << "StackfulCoroutineWorker::WaitForRunnables: spurious wakeup!";
        } else {
            LOG(DEBUG, COROUTINES) << "StackfulCoroutineWorker::WaitForRunnables: wakeup!";
        }

        if (waitingTimeMs != WAITING_TIME_UNLIMITED) {
            break;
        }
    }
}

void StackfulCoroutineWorker::RegisterIncomingActiveCoroutine(Coroutine *newCoro)
{
    ASSERT(newCoro != nullptr);
    newCoro->SetWorker(this);
    auto canMigrate = newCoro->GetContext<StackfulCoroutineContext>()->IsMigrationAllowed();
    // NOLINTNEXTLINE(clang-analyzer-optin.cplusplus.VirtualCall)
    newCoro->LinkToExternalHolder((IsMainWorker() || InExclusiveMode()) && !canMigrate);
}

void StackfulCoroutineWorker::RequestScheduleImpl()
{
    ProcessTimerEvents();

    // precondition: called within the current worker, no cross-worker calls allowed
    ASSERT(GetCurrentContext()->GetWorker() == this);
    ASSERT_NATIVE_CODE();

    // NOTE(konstanting): implement coro migration, work stealing, etc.
    if (RunnableCoroutinesExist()) {
        runnablesLock_.Lock();
        SuspendCurrentCoroAndScheduleNext();
        ASSERT(!IsCrossWorkerCall() || (Coroutine::GetCurrent()->GetType() == Coroutine::Type::MUTATOR));
    } else {
        auto [needToWait, waitingTimeMs] = CalculateShortestTimerDelay();
        if (!needToWait) {
            return;
        }

        bool migrationHappened = coroManager_->MigrateCoroutinesInward(this);
        if (migrationHappened) {
            return;
        }

        os::memory::LockHolder lh(runnablesLock_);
        LOG(DEBUG, COROUTINES) << "StackfulCoroutineWorker::RequestSchedule: No runnables, starting to wait...";
        WaitForRunnables(waitingTimeMs);
    }
}

void StackfulCoroutineWorker::BlockCurrentCoroAndScheduleNext()
{
    // precondition: current coro is already added to the waiters_
    BlockCurrentCoro();
    // will transfer control to another coro... Can change current coroutine's host worker!
    ScheduleNextCoroUnlockRunnablesWaiters();
    // ...this coro has been scheduled again: process finalization queue
    if (!IsCrossWorkerCall()) {
        FinalizeTerminatedCoros();
    } else {
        // migration happened!
    }
}

void StackfulCoroutineWorker::SuspendCurrentCoroAndScheduleNext()
{
    // will transfer control to another coro... Can change current coroutine's host worker!
    SuspendCurrentCoro();
    // will transfer control to another coro...
    ScheduleNextCoroUnlockRunnables();
    // ...this coro has been scheduled again: process finalization queue
    if (!IsCrossWorkerCall()) {
        FinalizeTerminatedCoros();
    } else {
        // migration happened!
    }
}

template <bool SUSPEND_AS_BLOCKED>
void StackfulCoroutineWorker::SuspendCurrentCoroGeneric()
{
    auto *currentCoro = Coroutine::GetCurrent();
    ASSERT(currentCoro != nullptr);
    if (currentCoro->IsContextSwitchRisky()) {
        LOG(ERROR, COROUTINES) << "RISKY CONTEXT SWITCH!!! Call stack: ";
        currentCoro->PrintCallStack();
    }
    currentCoro->RequestSuspend(SUSPEND_AS_BLOCKED);
    if constexpr (!SUSPEND_AS_BLOCKED) {
        os::memory::LockHolder lock(runnablesLock_);
        PushToRunnableQueue(currentCoro, currentCoro->GetPriority());
    }
}

void StackfulCoroutineWorker::BlockCurrentCoro()
{
    SuspendCurrentCoroGeneric<true>();
}

void StackfulCoroutineWorker::SuspendCurrentCoro()
{
    SuspendCurrentCoroGeneric<false>();
}

void StackfulCoroutineWorker::ScheduleNextCoroUnlockRunnablesWaiters()
{
    // precondition: runnable coros are present
    auto *currentCtx = GetCurrentContext();
    auto *nextCtx = PrepareNextRunnableContextForSwitch();

    runnablesLock_.Unlock();
    waitersLock_.Unlock();

    SwitchCoroutineContext(currentCtx, nextCtx);
}

void StackfulCoroutineWorker::ScheduleNextCoroUnlockRunnables()
{
    // precondition: runnable coros are present
    auto *currentCtx = GetCurrentContext();
    auto *nextCtx = PrepareNextRunnableContextForSwitch();

    runnablesLock_.Unlock();

    SwitchCoroutineContext(currentCtx, nextCtx);
}

void StackfulCoroutineWorker::ScheduleNextCoroUnlockNone()
{
    // precondition: runnable coros are present
    auto *currentCtx = GetCurrentContext();
    auto *nextCtx = PrepareNextRunnableContextForSwitch();
    SwitchCoroutineContext(currentCtx, nextCtx);
}

StackfulCoroutineContext *StackfulCoroutineWorker::GetCurrentContext() const
{
    auto *co = Coroutine::GetCurrent();
    ASSERT(co != nullptr);
    return co->GetContext<StackfulCoroutineContext>();
}

StackfulCoroutineContext *StackfulCoroutineWorker::PrepareNextRunnableContextForSwitch()
{
    // precondition: runnable coros are present
    ASSERT(Coroutine::GetCurrent() != nullptr);
    auto *il = Coroutine::GetCurrent()->ReleaseImmediateLauncher();
    auto *nextCtx = il != nullptr ? il->GetContext<StackfulCoroutineContext>()
                                  : PopFromRunnableQueue()->GetContext<StackfulCoroutineContext>();
    nextCtx->RequestResume();
    Coroutine::SetCurrent(nextCtx->GetCoroutine());
    return nextCtx;
}

void StackfulCoroutineWorker::SwitchCoroutineContext(StackfulCoroutineContext *from, StackfulCoroutineContext *to)
{
    ASSERT(from != nullptr);
    ASSERT(to != nullptr);
    EnsureCoroutineSwitchEnabled(from->GetCoroutine());
    LOG(DEBUG, COROUTINES) << "Ctx switch: " << from->GetCoroutine()->GetName() << " --> "
                           << to->GetCoroutine()->GetName();
    stats_.FinishInterval(CoroutineTimeStats::SCH_ALL);
    OnBeforeContextSwitch(from, to);
    stats_.StartInterval(CoroutineTimeStats::CTX_SWITCH);
    // performs the fiber switch!
    from->SwitchTo(to);
    if (IsCrossWorkerCall()) {
        ASSERT(Coroutine::GetCurrent()->GetType() == Coroutine::Type::MUTATOR);
        // Here this != current coroutine's worker. The rest of this function will be executed CONCURRENTLY!
        // NOTE(konstanting): need to correctly handle stats_ update here
        return;
    }
    stats_.FinishInterval(CoroutineTimeStats::CTX_SWITCH);
    stats_.StartInterval(CoroutineTimeStats::SCH_ALL);
    OnAfterContextSwitch(from);
}

void StackfulCoroutineWorker::FinalizeTerminatedCoros()
{
    while (!finalizationQueue_.empty()) {
        auto *f = finalizationQueue_.front();
        finalizationQueue_.pop();
        coroManager_->DestroyEntrypointfulCoroutine(f);
    }
}

void StackfulCoroutineWorker::UpdateLoadFactor()
{
    loadFactor_ = (loadFactor_ + runnables_.Size()) / 2U;
}

void StackfulCoroutineWorker::EnsureCoroutineSwitchEnabled(Coroutine *coro)
{
    if (IsCoroutineSwitchDisabled()) {
        coro->PrintCallStack();
        LOG(FATAL, COROUTINES) << "ERROR ERROR ERROR >>> Trying to switch coroutines on " << GetName()
                               << " when coroutine switch is DISABLED!!! <<< ERROR ERROR ERROR";
        UNREACHABLE();
    }
}

void StackfulCoroutineWorker::MigrateTo(StackfulCoroutineWorker *to)
{
    os::memory::LockHolder fromLock(runnablesLock_);
    size_t migrateCount = runnables_.Size() / 2;  // migrate up to half of runnable coroutines
    if (migrateCount == 0) {
        LOG(DEBUG, COROUTINES) << "The blocked worker does not have runnable coroutines.";
        return;
    }

    os::memory::LockHolder toLock(to->runnablesLock_);
    MigrateCoroutinesImpl(to, migrateCount);
}

bool StackfulCoroutineWorker::MigrateFrom(StackfulCoroutineWorker *from)
{
    os::memory::LockHolder toLock(runnablesLock_);
    if (!IsIdle()) {
        LOG(DEBUG, COROUTINES) << "The worker is not idle.";
        return false;
    }

    os::memory::LockHolder fromLock(from->runnablesLock_);
    size_t migrateCount = from->runnables_.Size() / 2;  // migrate up to half of runnable coroutines
    if (migrateCount == 0) {
        LOG(DEBUG, COROUTINES) << "The target worker does not have runnable coroutines.";
        return true;
    }

    from->MigrateCoroutinesImpl(this, migrateCount);
    return true;
}

void StackfulCoroutineWorker::MigrateCoroutinesImpl(StackfulCoroutineWorker *to, size_t migrateCount)
{
    using CIterator = PriorityQueue::CIterator;
    PandaVector<CIterator> migratedCoros;
    runnables_.VisitCoroutines([&migratedCoros, &migrateCount, this, to](auto begin, auto end) {
        for (; migrateCount > 0 && begin != end; ++begin) {
            // not migrate SCHEDULER coroutine and FINALIZER coroutine
            if ((*begin)->GetType() != Coroutine::Type::MUTATOR) {
                continue;
            }
            auto mask = (*begin)->template GetContext<StackfulCoroutineContext>()->GetAffinityMask();
            if (mask.IsWorkerAllowed(to->GetId())) {
                LOG(DEBUG, COROUTINES) << "migrate coro " << (*begin)->GetCoroutineId() << " from " << GetId() << " to "
                                       << to->GetId();
                to->AddRunnableCoroutine(*(*begin));
                migratedCoros.push_back(begin);
                --migrateCount;
            }
        }
    });
    runnables_.RemoveCoroutines(migratedCoros);
}

bool StackfulCoroutineWorker::IsIdle()
{
    return GetRunnablesCount(Coroutine::Type::MUTATOR) == 0;
}

void StackfulCoroutineWorker::OnBeforeContextSwitch([[maybe_unused]] StackfulCoroutineContext *from,
                                                    [[maybe_unused]] StackfulCoroutineContext *to)
{
    /**
     * "from" is aready suspended or blocked, "to" is already resumed.
     * The context is NOT switched yet, so we are on the ctx of the "from" coroutine,
     * so it is STRICTLY NOT RECOMMENDED to run managed code from this event handler and
     * its callees.
     */
}

void StackfulCoroutineWorker::OnAfterContextSwitch(StackfulCoroutineContext *to)
{
    /**
     * "to" is already resumed. There is no reliable way to determine the ctx switch source yet.
     * The context is JUST SWITCHED, so we are on the ctx of the "to" coroutine.
     */
    Coroutine *coroTo = to->GetCoroutine();
    coroTo->OnContextSwitchedTo();
}

void StackfulCoroutineWorker::CacheLocalObjectsInCoroutines()
{
    os::memory::LockHolder lock(runnablesLock_);
    runnables_.IterateOverCoroutines([](Coroutine *co) { co->UpdateCachedObjects(); });
    {
        os::memory::LockHolder lh(waitersLock_);
        for (auto &[_, co] : waiters_) {
            co->UpdateCachedObjects();
        }
    }
}

void StackfulCoroutineWorker::GetFullWorkerStateInfo(StackfulCoroutineWorkerStateInfo *info) const
{
    {
        os::memory::LockHolder lh(waitersLock_);
        std::for_each(waiters_.begin(), waiters_.end(), [&info](const std::pair<CoroutineEvent *, Coroutine *> &pair) {
            info->AddCoroutine(pair.second);
        });
    }
    {
        os::memory::LockHolder lock(runnablesLock_);
        runnables_.IterateOverCoroutines([&info](Coroutine *co) { info->AddCoroutine(co); });
    }
}

void StackfulCoroutineWorker::ProcessTimerEvents()
{
    PandaVector<TimerEvent *> expiredTimers;
    {
        os::memory::LockHolder lh(waitersLock_);
        auto curTime = coroManager_->GetCurrentTime();
        for (auto &[evt, _] : waiters_) {
            if (evt->GetType() == CoroutineEvent::Type::TIMER) {
                auto *timerEvent = static_cast<TimerEvent *>(evt);
                timerEvent->SetCurrentTime(curTime);
                if (timerEvent->IsExpired()) {
                    expiredTimers.push_back(timerEvent);
                }
            }
        }
    }
    std::sort(expiredTimers.begin(), expiredTimers.end(),
              [](const TimerEvent *evt1, const TimerEvent *evt2) { return evt1->GetId() < evt2->GetId(); });
    for (auto *evt : expiredTimers) {
        evt->Happen();
    }
}

std::pair<bool, uint64_t> StackfulCoroutineWorker::CalculateShortestTimerDelay()
{
    uint64_t minTimerDelay = std::numeric_limits<uint64_t>::max();
    os::memory::LockHolder lh(waitersLock_);
    for (auto &[evt, _] : waiters_) {
        if (evt->GetType() == CoroutineEvent::Type::TIMER) {
            auto *timerEvent = static_cast<TimerEvent *>(evt);
            auto delay = timerEvent->GetDelay();
            if (timerEvent->IsExpired()) {
                return {false, 0};
            }
            minTimerDelay = std::min(delay, minTimerDelay);
        }
    }
    if (minTimerDelay == std::numeric_limits<uint64_t>::max()) {
        // Should retun WAITING_TIME_UNLIMITED based on method declaration
        return {true, WAITING_TIME_UNLIMITED};
    }
    // Returns time in ms that most close to correct one but is not less then it
    return {true, (minTimerDelay - 1) / 1000U + 1};  // NOLINT(readability-magic-numbers)
}

}  // namespace ark
