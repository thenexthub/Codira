/**
 * Copyright (c) 2022-2025 Huawei Device Co., Ltd.
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
#ifndef PANDA_RUNTIME_COROUTINES_THREADED_COROUTINE_MANAGER_H
#define PANDA_RUNTIME_COROUTINES_THREADED_COROUTINE_MANAGER_H

#include "libarkbase/os/mutex.h"
#include "runtime/coroutines/coroutine_manager.h"
#include "runtime/coroutines/coroutine.h"
#include "runtime/coroutines/priority_queue.h"
#include "runtime/include/value.h"

namespace ark {

/**
 * @brief Thread-based coroutine manager implementation.
 *
 * In this implementation coroutines are OS threads. The invariants (only N coroutines are running simultaniously, etc.)
 * are kept with the help of mutexes and condition variables.
 *
 * For interface function descriptions see CoroutineManager class declaration.
 */
class ThreadedCoroutineManager : public CoroutineManager {
public:
    NO_COPY_SEMANTIC(ThreadedCoroutineManager);
    NO_MOVE_SEMANTIC(ThreadedCoroutineManager);

    explicit ThreadedCoroutineManager(const CoroutineManagerConfig &config, CoroutineFactory factory)
        : CoroutineManager(config, factory)
    {
    }
    ~ThreadedCoroutineManager() override = default;

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
    void PreZygoteFork() override {};
    void PostZygoteFork() override {};

    /* ThreadManager interfaces, see ThreadManager class for the details */
    void WaitForDeregistration() override;
    void SuspendAllThreads() override;
    void ResumeAllThreads() override;
    bool IsRunningThreadExist() override;

    /* debugging tools */
    /**
     * For ThreadedCoroutineManager implementation: no action performed if an attempt to switch coroutines on current
     * worker is detected when coroutine switch is disabled.
     */
    void DisableCoroutineSwitch() override {};
    void EnableCoroutineSwitch() override {};
    bool IsCoroutineSwitchDisabled() override
    {
        return false;
    };

protected:
    bool EnumerateThreadsImpl(const ThreadManager::Callback &cb, unsigned int incMask,
                              unsigned int xorMask) const override;
    bool EnumerateWorkersImpl(const EnumerateWorkerCallback &cb) const override;

    CoroutineContext *CreateCoroutineContext(bool coroHasEntrypoint) override;
    void DeleteCoroutineContext(CoroutineContext *ctx) override;

    size_t GetCoroutineCount() override;
    size_t GetCoroutineCountLimit() override;

    /// @return number of coroutines that can execute simultaniously
    uint32_t GetActiveWorkersCount() const;
    /**
     * Create the required number of worker instances. They are required only for the interface
     * level compatibility with other parts of the runtime.
     */
    void CreateWorkers(size_t howMany, Runtime *runtime, PandaVM *vm) REQUIRES(workersLock_) override;
    CoroutineWorker *ChooseWorkerForCoroutine([[maybe_unused]] Coroutine *co);

private:
    LaunchResult LaunchImpl(EntrypointInfo &&epInfo, PandaString &&coroName, CoroutinePriority priority,
                            bool startSuspended = true);
    void ScheduleImpl();
    void UnblockWaitersImpl(CoroutineEvent *blocker) REQUIRES(coroSwitchLock_);

    /* runnables queue management */
    void PushToRunnableQueue(Coroutine *co, CoroutinePriority priority);
    Coroutine *PopFromRunnableQueue();
#ifndef NDEBUG
    void PrintRunnableQueue(const PandaString &requester);
#endif
    bool RunnableCoroutinesExist();

    /* coroutine registry management */
    void AddToRegistry(Coroutine *co) REQUIRES(coroListLock_);
    void RemoveFromRegistry(Coroutine *co) REQUIRES(coroListLock_);

    void DeleteCoroutineInstance(Coroutine *co);
    bool RegisterWaiter(Coroutine *waiter, CoroutineEvent *awaitee) RELEASE(awaitee);
    void ScheduleNextCoroutine();
    void MainCoroutineCompleted();

    void ProcessTimerEvents();

    os::memory::Mutex coroSwitchLock_;
    os::memory::Mutex waitersLock_;
    mutable os::memory::Mutex coroListLock_;
    // all registered coros
    PandaSet<Coroutine *> coroutines_ GUARDED_BY(coroListLock_);
    // ready coros
    PriorityQueue runnablesQueue_;
    // blocked coros: Coroutine AWAITS CoroutineEvent
    PandaMap<CoroutineEvent *, Coroutine *> waiters_;

    os::memory::ConditionVariable cvAwaitAll_;
    os::memory::Mutex cvAwaitAllMutex_;

    // worker related members
    PandaVector<CoroutineWorker *> workers_ GUARDED_BY(workersLock_);
    size_t activeWorkersCount_ GUARDED_BY(workersLock_) = 0;
    mutable os::memory::RecursiveMutex workersLock_;
    mutable os::memory::ConditionVariable workersCv_;

    // main is running from the very beginning
    std::atomic_uint32_t runningCorosCount_ = 1;
    std::atomic_uint32_t coroutineCount_ = 0;
};

}  // namespace ark

#endif /* PANDA_RUNTIME_COROUTINES_THREADED_COROUTINE_MANAGER_H */
