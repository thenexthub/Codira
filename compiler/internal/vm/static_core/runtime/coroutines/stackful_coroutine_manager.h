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
#ifndef PANDA_RUNTIME_COROUTINES_STACKFUL_COROUTINE_MANAGER_H
#define PANDA_RUNTIME_COROUTINES_STACKFUL_COROUTINE_MANAGER_H

#include "runtime/coroutines/coroutine_manager.h"
#include "runtime/coroutines/affinity_mask.h"
#include "runtime/coroutines/stackful_coroutine.h"
#include "runtime/coroutines/stackful_coroutine_worker.h"
#include "runtime/coroutines/coroutine_stats.h"

#include "runtime/coroutines/stackful_coroutine_state_info.h"

#include "runtime/coroutines/native_stack_allocator/native_stack_allocator.h"

namespace ark {

/**
 * @brief Stackful ("fiber"-based) coroutine manager implementation.
 *
 * In this implementation coroutines are user-level threads ("fibers") with manually allocated stacks.
 *
 * For interface function descriptions see CoroutineManager class declaration.
 */
class StackfulCoroutineManager : public CoroutineManager {
public:
    NO_COPY_SEMANTIC(StackfulCoroutineManager);
    NO_MOVE_SEMANTIC(StackfulCoroutineManager);

    explicit StackfulCoroutineManager(const CoroutineManagerConfig &config, CoroutineFactory factory)
        : CoroutineManager(config, factory)
    {
    }
    ~StackfulCoroutineManager() override = default;

    /* CoroutineManager interfaces, see CoroutineManager class for the details */
    void InitializeScheduler(Runtime *runtime, PandaVM *vm) override;
    void Finalize() override;
    void RegisterCoroutine(Coroutine *co) override;
    bool TerminateCoroutine(Coroutine *co) override;
    LaunchResult Launch(CompletionEvent *completionEvent, Method *entrypoint, PandaVector<Value> &&arguments,
                        const CoroutineWorkerGroup::Id &groupId, CoroutinePriority priority, bool abortFlag) override;
    LaunchResult LaunchImmediately(CompletionEvent *completionEvent, Method *entrypoint, PandaVector<Value> &&arguments,
                                   const CoroutineWorkerGroup::Id &groupId, CoroutinePriority priority,
                                   bool abortFlag) override;
    LaunchResult LaunchNative(NativeEntrypointFunc epFunc, void *param, PandaString coroName,
                              const CoroutineWorkerGroup::Id &groupId, CoroutinePriority priority,
                              bool launchImmediately, bool abortFlag) override;
    void Schedule() override;
    void Await(CoroutineEvent *awaitee) RELEASE(awaitee) override;
    void UnblockWaiters(CoroutineEvent *blocker) override;

    Coroutine *CreateExclusiveWorkerForThread(Runtime *runtime, PandaVM *vm) override;
    bool DestroyExclusiveWorker() override;
    bool IsExclusiveWorkersLimitReached() const override;
    bool IsUserCoroutineLimitReached() const override;

    void CreateWorkers(size_t howMany, Runtime *runtime, PandaVM *vm) override;
    void FinalizeWorkers(size_t howMany, Runtime *runtime, PandaVM *vm) override;

    void PreZygoteFork() override;
    void PostZygoteFork() override;

    /* ThreadManager interfaces, see ThreadManager class for the details */
    void WaitForDeregistration() override;
    void SuspendAllThreads() override;
    void ResumeAllThreads() override;
    bool IsRunningThreadExist() override;

    /**
     * @brief Creates a coroutine instance with a native function as an entry point
     * @param entry native function to execute
     * @param param param to pass to the EP
     */
    Coroutine *CreateNativeCoroutine(Runtime *runtime, PandaVM *vm,
                                     Coroutine::NativeEntrypointInfo::NativeEntrypointFunc entry, void *param,
                                     PandaString name, Coroutine::Type type, CoroutinePriority priority);
    /// destroy the "native" coroutine created earlier
    void DestroyNativeCoroutine(Coroutine *co);
    void DestroyEntrypointfulCoroutine(Coroutine *co) override;

    /// get next free worker id
    size_t GetNextFreeWorkerId();

    /* events */
    /// called when a coroutine worker thread ends its execution
    void OnWorkerShutdown(StackfulCoroutineWorker *worker);
    /// called when a coroutine worker thread starts its execution
    void OnWorkerStartup(StackfulCoroutineWorker *worker);
    /// Should be called when a coro makes the non_active->active transition (see the state diagram in coroutine.h)
    void OnCoroBecameActive(Coroutine *co) override;
    /**
     * Should be called when a running coro is being blocked or terminated, i.e. makes
     * the active->non_active transition (see the state diagram in coroutine.h)
     */
    void OnCoroBecameNonActive(Coroutine *co) override;
    /// Should be called at the end of the VM native interface call
    void OnNativeCallExit(Coroutine *co) override;

    /* debugging tools */
    /**
     * For StackfulCoroutineManager implementation: a fatal error is issued if an attempt to switch coroutines on
     * current worker is detected when coroutine switch is disabled.
     */
    void DisableCoroutineSwitch() override;
    void EnableCoroutineSwitch() override;
    bool IsCoroutineSwitchDisabled() override;

    /* profiling tools */
    CoroutineStats &GetPerfStats()
    {
        return stats_;
    }

    /// migrate coroutines from other workers to the 'to' worker
    bool MigrateCoroutinesInward(StackfulCoroutineWorker *to);

    /// trigger the managerThread to migrate
    void TriggerMigration();
    /**
     * @brief migrate the awakened coroutine to the worker with the lowest load
     * @param co the awakened coroutine
     */
    void MigrateAwakenedCoro(Coroutine *co);

    PandaUniquePtr<StackfulCoroutineStateInfoTable> GetAllWorkerFullStatus() const;

    CoroutineWorkerGroup::Id GenerateWorkerGroupId(CoroutineWorkerDomain domain,
                                                   const PandaVector<CoroutineWorker::Id> &hint) override;

protected:
    static constexpr CoroutineWorker::Id MAIN_WORKER_ID = 0U;
    // maximum worker id is bound by the number of bits in the affinity mask
    static constexpr CoroutineWorker::Id MAX_WORKER_ID = AffinityMask::MAX_WORKERS_COUNT - 1U;
    static_assert(MAIN_WORKER_ID != CoroutineWorker::INVALID_ID);
    static_assert(MAX_WORKER_ID >= MAIN_WORKER_ID);

    bool EnumerateThreadsImpl(const ThreadManager::Callback &cb, unsigned int incMask,
                              unsigned int xorMask) const override;
    bool EnumerateWorkersImpl(const EnumerateWorkerCallback &cb) const override;

    CoroutineContext *CreateCoroutineContext(bool coroHasEntrypoint) override;
    void DeleteCoroutineContext(CoroutineContext *ctx) override;

    size_t GetCoroutineCount() override;
    size_t GetCoroutineCountLimit() override;

    StackfulCoroutineContext *GetCurrentContext();
    StackfulCoroutineWorker *GetCurrentWorker();

    /**
     * @brief reuse a cached coroutine instance in case when coroutine pool is enabled
     * see Coroutine::ReInitialize for details
     */
    void ReuseCoroutineInstance(Coroutine *co, EntrypointInfo &&epInfo, PandaString name, CoroutinePriority priority);

private:
    StackfulCoroutineContext *CreateCoroutineContextImpl(bool needStack);
    StackfulCoroutineWorker *ChooseWorkerForCoroutine(Coroutine *co) REQUIRES(workersLock_);
    AffinityMask CalcAffinityMask(const CoroutineWorkerGroup::Id &groupId = CoroutineWorkerGroup::AnyId());

    Coroutine *GetCoroutineInstanceForLaunch(EntrypointInfo &&epInfo, PandaString &&coroName,
                                             CoroutinePriority priority, AffinityMask affinityMask, bool abortFlag);
    LaunchResult LaunchImpl(EntrypointInfo &&epInfo, PandaString &&coroName, const CoroutineWorkerGroup::Id &groupId,
                            CoroutinePriority priority, bool abortFlag);
    LaunchResult LaunchImmediatelyImpl(EntrypointInfo &&epInfo, PandaString &&coroName,
                                       const CoroutineWorkerGroup::Id &groupId, CoroutinePriority priority,
                                       bool abortFlag);
    LaunchResult LaunchWithGroupId(EntrypointInfo &&epInfo, PandaString &&coroName, CoroutineWorkerGroup::Id groupId,
                                   CoroutinePriority priority, bool launchImmediately, bool abortFlag);
    /**
     * Tries to extract a coroutine instance from the pool for further reuse, returns nullptr in case when it is not
     * possible.
     */
    Coroutine *TryGetCoroutineFromPool();

    /* workers API */
    void CreateWorkersImpl(size_t howMany, Runtime *runtime, PandaVM *vm) REQUIRES(workersLock_);
    /**
     * This method creates main worker and coroutine + the number of common workers
     * @param howMany total number of common worker threads, NOT including MAIN
     */
    void CreateMainCoroAndWorkers(size_t howMany, Runtime *runtime, PandaVM *vm) REQUIRES(workersLock_);
    void OnWorkerStartupImpl(StackfulCoroutineWorker *worker) REQUIRES(workersLock_);
    StackfulCoroutineWorker *CreateWorker(Runtime *runtime, PandaVM *vm,
                                          StackfulCoroutineWorker::ScheduleLoopType wType, PandaString workerName,
                                          bool isMainWorker = false);

    /* coroutine registry management */
    void AddToRegistry(Coroutine *co) REQUIRES(coroListLock_);
    void RemoveFromRegistry(Coroutine *co) REQUIRES(coroListLock_);

    /// call to check if we are done executing managed code and set appropriate member flags
    void CheckProgramCompletion();
    /// call when main coroutine is done executing its managed EP
    void MainCoroutineCompleted();
    /// wait till all the non-main coroutines with managed EP finish execution
    void WaitForNonMainCoroutinesCompletion();
    /// @return number of running worker loop coroutines
    size_t GetActiveWorkersCount() const;
    /// @return number of existing worker instances
    size_t GetExistingWorkersCount() const;
    /// dump coroutine stats to stdout
    void DumpCoroutineStats() const;

    /* resource management */
    uint8_t *AllocCoroutineStack();
    void FreeCoroutineStack(uint8_t *stack);

    /// @return true if there is no active coroutines
    bool IsNoActiveMutatorsExceptCurrent();
    /// Increment/decrement active corotuines count
    void IncrementActiveCoroutines();
    void DecrementActiveCoroutines();

    /// list unhandled language specific events on program exit
    void ListUnhandledEventsOnProgramExit();

    StackfulCoroutineWorker *ChooseWorkerForFinalization();

    void InitializeWorkerIdAllocator();
    CoroutineWorker::Id AllocateWorkerId();
    void ReleaseWorkerId(CoroutineWorker::Id workerId);

    StackfulCoroutineWorker *ChooseWorkerImpl(WorkerSelectionPolicy policy, AffinityMask maskValue)
        REQUIRES(workersLock_);

    /**
     * @brief Calculate worker limits based on configuration
     * @param config The coroutine manager configuration
     * @param exclusiveWorkersLimit Output parameter for exclusive workers limit
     * @param commonWorkersLimit Output parameter for common workers limit
     */
    void CalculateWorkerLimits(size_t &exclusiveWorkersLimit, size_t &commonWorkersLimit);
    void CalculateUserCoroutinesLimits(size_t &userCoroutineCountLimit, size_t limit);

private:  // data members
    // for thread safety with GC
    mutable os::memory::Mutex coroListLock_;
    // all registered coros
    PandaSet<Coroutine *> coroutines_ GUARDED_BY(coroListLock_);

    // worker threads-related members
    PandaList<StackfulCoroutineWorker *> workers_ GUARDED_BY(workersLock_);
    // allocator of worker ids
    os::memory::Mutex workerIdLock_;
    PandaList<CoroutineWorker::Id> freeWorkerIds_ GUARDED_BY(workerIdLock_);
    size_t activeWorkersCount_ GUARDED_BY(workersLock_) = 0;
    mutable os::memory::RecursiveMutex workersLock_;
    mutable os::memory::ConditionVariable workersCv_;

    // events that control program completion
    os::memory::Mutex programCompletionLock_;
    CoroutineEvent *programCompletionEvent_ = nullptr;

    // various counters
    std::atomic_uint32_t coroutineCount_ = 0;
    size_t coroutineCountLimit_ = 0;
    size_t userCoroutineCountLimit_ = 0;
    // user coro (= non system) is EP-full mutator
    size_t exclusiveWorkersLimit_ = 0;
    size_t commonWorkersCount_ = 0;
    size_t coroStackSizeBytes_ = 0;

    std::atomic_uint32_t userCoroutineCount_ = 0;
    // active coroutines are runnable + running coroutines
    std::atomic_uint32_t activeCoroutines_ = 0;
    // NOTE(konstanting): make it a map once number of the coroutine types gets bigger
    // Utility coro is a system coro that is FINALIZER or SCHEDULER
    std::atomic_uint32_t utilityCoroutineCount_ = 0;

    /**
     * @brief holds pointers to the cached coroutine instances in order to speedup coroutine creation and destruction.
     * linked coroutinecontext instances are cached too (we just keep the cached coroutines linked to their contexts).
     * used only in case when --use-coroutine-pool=true
     */
    PandaVector<Coroutine *> coroutinePool_ GUARDED_BY(coroPoolLock_);
    mutable os::memory::Mutex coroPoolLock_;

    // stats
    CoroutineStats stats_;
    PandaVector<CoroutineWorkerStats> finalizedWorkerStats_;

    os::memory::Mutex eWorkerCreationLock_;

    // the time interval between detecting worker blocking
    static constexpr uint32_t DETECTION_INTERVAL_VALUE = 5000;

    CoroutineWorkerGroup::Id generalWorkerGroup_ = CoroutineWorkerGroup::Empty();
    CoroutineWorkerGroup::Id eaWorkerGroup_ = CoroutineWorkerGroup::Empty();

    NativeStackAllocator nativeStackAllocator_;
};

}  // namespace ark

#endif /* PANDA_RUNTIME_COROUTINES_STACKFUL_COROUTINE_MANAGER_H */
