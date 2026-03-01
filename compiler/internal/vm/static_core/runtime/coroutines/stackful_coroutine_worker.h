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

#ifndef PANDA_RUNTIME_COROUTINES_STACKFUL_COROUTINE_WORKER_H
#define PANDA_RUNTIME_COROUTINES_STACKFUL_COROUTINE_WORKER_H

#include "runtime/coroutines/coroutine.h"
#include "runtime/coroutines/coroutine_worker.h"
#include "runtime/coroutines/coroutine_events.h"
#include "runtime/coroutines/stackful_common.h"
#include "runtime/coroutines/coroutine_stats.h"
#include "runtime/coroutines/priority_queue.h"
#include "runtime/include/external_callback_poster.h"

#include "runtime/coroutines/stackful_coroutine_state_info.h"

namespace ark {

class StackfulCoroutineContext;
class StackfulCoroutineManager;

/**
 * Represents a worker thread for stackful coroutines.
 * Contains local part of the scheduling machinery (local coroutine queues, methods)
 */
class StackfulCoroutineWorker : public CoroutineWorker {
public:
    enum class ScheduleLoopType { THREAD, FIBER };

    NO_COPY_SEMANTIC(StackfulCoroutineWorker);
    NO_MOVE_SEMANTIC(StackfulCoroutineWorker);

    /**
     * @brief The worker constructor. Create the worker and its schedule loop.
     *
     * Notable parameters:
     * @param type defines the schedule loop type for this worker: a separate thread or a coroutine ("FIBER")
     */
    StackfulCoroutineWorker(Runtime *runtime, PandaVM *vm, StackfulCoroutineManager *coroManager, ScheduleLoopType type,
                            PandaString name, Id id, bool isMainWorker);
    ~StackfulCoroutineWorker() override = default;

    /// @return false if the worker is stopped and does not schedule anything, otherwise true
    bool IsActive() const
    {
        os::memory::LockHolder lock(runnablesLock_);
        return active_;
    }

    /// enable or disable the worker
    void SetActive(bool val)
    {
        os::memory::LockHolder lock(runnablesLock_);
        active_ = val;
        runnablesCv_.Signal();
    }

    /// @return the moving average number of runnable coroutines in the queue
    double GetLoadFactor() const
    {
        return loadFactor_;
    }

    void DisableForCrossWorkersLaunch()
    {
        isDisabledForCrossWorkersLaunch_ = true;
    }

    bool IsDisabledForCrossWorkersLaunch() const
    {
        return isDisabledForCrossWorkersLaunch_;
    }

    /**
     * @brief Adds a coroutine to the runnables queue. Any new incoming RUNNABLE coroutine should be added
     * via this interface! And vice versa: no intra-worker queue transitions should be done via this
     * interface
     * @param newCoro coroutine to add
     * @param prioritize if true, add to the beginning of the queue (otherwise to the end)
     */
    void AddRunnableCoroutine(Coroutine *newCoro);

    /**
     * @brief Registers the RUNNING coroutine in the worker's structures.
     * Any incoming running coroutine should be added via this interface! And vice versa: no
     * intra-worker queue transitions should be done via this interface.
     * @param newCoro coroutine to add
     */
    void AddRunningCoroutine(Coroutine *newCoro);

    /**
     * @brief Add a new created coroutine and switch to it immediately.
     * @param newCoro coroutine to add
     */
    void AddCreatedCoroutineAndSwitchToIt(Coroutine *newCoro);

    /**
     * @brief Block current coroutine till an event happens and switch context to the next ready one
     * @param awaitee the event to wait
     */
    void WaitForEvent(CoroutineEvent *awaitee) RELEASE(awaitee);

    /**
     * @brief Signal that an event has happened and unblock all the coroutines in the current worker that are waiting
     * for this event
     * @param blocker the event that has happened
     */
    void UnblockWaiters(CoroutineEvent *blocker);

    /**
     * @brief Add a coroutine to the finalization queue for future destruction and schedule the next ready one for
     * execution. Used by a coroutine being terminated for safe self-destruction.
     * @param finalizee coroutine to finalize (the caller)
     */
    void RequestFinalization(Coroutine *finalizee);

    /// @brief schedule the next ready coroutine from the runnables queue for execution
    void RequestSchedule();

    /// @brief call to delete the fake "schedule loop" coroutine
    void FinalizeFiberScheduleLoop();

    /// @brief schedule runnable coroutines and wait for blocked coroutines
    void CompleteAllAffinedCoroutines();

    /* debugging tools */
    // See CoroutineManager/StackfulCoroutineManager for details
    void DisableCoroutineSwitch();
    void EnableCoroutineSwitch();
    bool IsCoroutineSwitchDisabled();

    /// @brief Migrate all not affinned coroutines from worker
    void MigrateCoroutines() {}

    /**
     * @brief Method prints in info next information about coroutine worker:
     *  - list of coroutine stacks on this worker
     */
    void GetFullWorkerStateInfo(StackfulCoroutineWorkerStateInfo *info) const;

#ifndef NDEBUG
    void PrintRunnables(const PandaString &requester);
#endif
    /**
     * @brief Returns the number of coroutines in the runnables queue that belong to a certain type.
     * Since another worker can concurrently add more coroutines before or after the call, the returned value
     * should be used only for debugging purposes AND ONLY IF YOU KNOW WHAT EXACTLY ARE YOU DOING.
     * @param type the type of coroutines to count
     */
    size_t GetRunnablesCount(Coroutine::Type type);

    /* profiling tools */
    CoroutineWorkerStats &GetPerfStats()
    {
        return stats_;
    }

    /// @brief get exclusive status of worker
    bool InExclusiveMode() const override
    {
        // Atomic with relaxed order reason: sync is not needed here
        return inExclusiveMode_.load(std::memory_order_relaxed);
    }

    /// @brief set exclusive status of worker
    void SetExclusiveMode(bool inExclusiveMode)
    {
        // Atomic with relaxed order reason: sync is not needed here
        inExclusiveMode_.store(inExclusiveMode, std::memory_order_relaxed);
    }

    /// migrate coroutines from this to the 'to' worker
    void MigrateTo(StackfulCoroutineWorker *to);
    /// migrate coroutines from the 'from' worker to this
    bool MigrateFrom(StackfulCoroutineWorker *from);
    /// check whether the worker is idle
    bool IsIdle();

    bool ProcessAsyncWork() NO_THREAD_SAFETY_ANALYSIS override;
    void OnNewCoroutineStartup(Coroutine *co) override;

private:
    /* schedule loop management */
    /// the EP for threaded schedule loops
    void ThreadProc();
    /// the EP for fiber schedule loops
    void ScheduleLoop();
    /// the schedule loop itself
    void ScheduleLoopBody();
    /// the helper proxy function for the fiber schedule loop
    static void ScheduleLoopProxy(void *worker);

    /* runnables queue management */
    void PushToRunnableQueue(Coroutine *co, CoroutinePriority priority) REQUIRES(runnablesLock_);
    Coroutine *PopFromRunnableQueue();
    bool RunnableCoroutinesExist() const;
    /**
     * @brief Method waits for @param waitingTime ms, if waitingTimeMs == WAITING_TIME_UNLIMITED, will wait until signal
     * of runnablesCv_ condvar
     */
    void WaitForRunnables(uint64_t waitingTimeMs) REQUIRES(runnablesLock_);
    /**
     * @brief Register a new active (= runnable or running) coroutine on this worker.
     * @param newCoro the incoming coroutine to register
     */
    void RegisterIncomingActiveCoroutine(Coroutine *newCoro);

    /* scheduling machinery from high level functions to elementary helpers */
    /**
     * Schedule the next ready coroutine from the runnables queue for execution if present, otherwise wait till those
     * appear, then pick the best suitable and schedule it. After that eventually the current coroutine will be
     * scheduled for execution too. Upon that, the control flow will get back to this function and it will return.
     * This method may have no effect if another worker adds a coroutine to the current worker during this call.
     */
    void RequestScheduleImpl();
    void BlockCurrentCoroAndScheduleNext() RELEASE(runnablesLock_) RELEASE(waitersLock_);
    void SuspendCurrentCoroAndScheduleNext() RELEASE(runnablesLock_);
    template <bool SUSPEND_AS_BLOCKED>
    void SuspendCurrentCoroGeneric();
    void BlockCurrentCoro();
    void SuspendCurrentCoro();
    /* "the lesser evil": keep thread safety annotations but duplicate the function body */
    void ScheduleNextCoroUnlockRunnablesWaiters() RELEASE(runnablesLock_) RELEASE(waitersLock_);
    void ScheduleNextCoroUnlockRunnables() RELEASE(runnablesLock_);
    void ScheduleNextCoroUnlockNone();
    StackfulCoroutineContext *GetCurrentContext() const;
    StackfulCoroutineContext *PrepareNextRunnableContextForSwitch();
    void SwitchCoroutineContext(StackfulCoroutineContext *from, StackfulCoroutineContext *to);

    /* various helper functions */
    /// parse the finalization queue and destroy all coroutines from it
    void FinalizeTerminatedCoros();
    /// recalculate the load factor
    void UpdateLoadFactor() REQUIRES(runnablesLock_);
    /**
     * This checker is called on a coroutine switch attempt. Issues fatal error in case when coroutine switch is
     * disabled.
     */
    void EnsureCoroutineSwitchEnabled(Coroutine *coro);

    /// @return true if current method is called from another worker instance
    bool IsCrossWorkerCall()
    {
        ASSERT(Coroutine::GetCurrent() != nullptr);
        return (this != Coroutine::GetCurrent()->GetWorker());
    }

    void MigrateCoroutinesImpl(StackfulCoroutineWorker *to, size_t migrateCount) REQUIRES(runnablesLock_);

    /* events */
    /// called right before the coroutineContext is switched
    void OnBeforeContextSwitch(StackfulCoroutineContext *from, StackfulCoroutineContext *to);
    /// called right after the coroutineContext is switched (in case if no migration happened)
    void OnAfterContextSwitch(StackfulCoroutineContext *to);

    /// worker local storage
    void CacheLocalObjectsInCoroutines() override;

    void ProcessTimerEvents();

    /**
     * @brief Method checks if we need to wait timer and if it is, returns time to wait.
     * @returns false and 0 if no need to wait. Otherwise, @returns true and time to wait
     * (if time is WAITING_TIME_UNLIMITED constant, no timed waiting, only waiting for signal)
     */
    std::pair<bool, uint64_t> CalculateShortestTimerDelay();

private:  // data members
    StackfulCoroutineManager *coroManager_;
    Coroutine *scheduleLoopCtx_ = nullptr;
    bool active_ GUARDED_BY(runnablesLock_) = true;
    os::thread::ThreadId threadId_;

    // runnable coroutines-related members
    mutable os::memory::RecursiveMutex runnablesLock_;
    os::memory::ConditionVariable runnablesCv_;
    PriorityQueue runnables_ GUARDED_BY(runnablesLock_);
    // blocked coros-related members: Coroutine AWAITS CoroutineEvent
    mutable os::memory::Mutex waitersLock_;
    PandaMap<CoroutineEvent *, Coroutine *> waiters_ GUARDED_BY(waitersLock_);
    // terminated coros (waiting for deletion)
    PandaQueue<Coroutine *> finalizationQueue_;

    /// the moving average number of coroutines in the runnable queue
    std::atomic<double> loadFactor_ = 0;

    // the timestamp of the last coroutine context switch
    std::atomic<uint64_t> lastCtxSwitchTimeMillis_ = 0;

    /**
     * If worker is in exclusive mode, it means that:
     * 1. launch/transition of coroutine from other workers to e-worker is disabled
     * 2. child e-worker coroutines will be scheduled on the same worker
     */
    std::atomic<bool> inExclusiveMode_ = false;
    std::atomic<bool> isDisabledForCrossWorkersLaunch_ = false;

    /**
     * This counter is incremented on DisableCoroutineSwitch calls and decremented on EnableCoroutineSwitch calls.
     * The value 0 means that coroutine switch is ENABLED.
     */
    uint32_t disableCoroSwitchCounter_ = 0;

    GenericEvent hasRunnableCoroEvent_;

    // stats
    CoroutineWorkerStats stats_;

    // the maximum continuous execution time of a coroutine
    static constexpr uint32_t MAX_EXECUTION_DURATION_MS = 6000;

    static constexpr uint64_t WAITING_TIME_UNLIMITED = std::numeric_limits<uint64_t>::max();
};

}  // namespace ark

#endif /* PANDA_RUNTIME_COROUTINES_STACKFUL_COROUTINE_WORKER_H */
