/**
 * Copyright (c) 2022-2024 Huawei Device Co., Ltd.
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
#ifndef PANDA_RUNTIME_COROUTINES_THREADED_COROUTINE_H
#define PANDA_RUNTIME_COROUTINES_THREADED_COROUTINE_H

#include "runtime/coroutines/coroutine_context.h"

namespace ark {

/**
 * @brief Native context of a coroutine. std::thread-based implementation.
 *
 * This implementation utilizes std::thread with condition variables and mutexes
 * to emulate context switch on suspension.
 *
 */
class ThreadedCoroutineContext : public CoroutineContext {
public:
    NO_COPY_SEMANTIC(ThreadedCoroutineContext);
    NO_MOVE_SEMANTIC(ThreadedCoroutineContext);

    explicit ThreadedCoroutineContext() = default;
    ~ThreadedCoroutineContext() override = default;

    /**
     * Prepares the context (including the std::thread object creation) for execution, links it to the managed context
     * part (Coroutine instance) and registers the created coroutine in the CoroutineManager (in the RUNNABLE status)
     */
    void AttachToCoroutine(Coroutine *co) override;
    /**
     * Manually destroys the context. Should be called by the Coroutine instance as a part of main coroutine
     * destruction. All other coroutines and their contexts are destroyed by the CoroutineManager once the coroutine
     * entrypoint execution finishes
     */
    void Destroy() override;

    void CleanUp() override {}

    bool RetrieveStackInfo(void *&stackAddr, size_t &stackSize, size_t &guardSize) override;

    /**
     * Intended to be called from the context of a running thread that is going to be suspended.
     * Changes status to Status::RUNNABLE or Status::BLOCKED, depending on the suspend
     * reason. The next step for the caller thread is to call WaitUntilResumed() and block on it.
     */
    void RequestSuspend(bool getsBlocked) override;
    /**
     * Resume the suspended coroutine by setting status to Status::RUNNING.
     * Wakes up the thread that is blocked on WaitUntilResumed()
     */
    void RequestResume() override;
    /// Unblock the coroutine and set its status to Status::RUNNABLE
    void RequestUnblock() override;

    /**
     * Intended to be called from the context of a running coroutine that is going to
     * launch a new one. Blocks until the created coroutine passes the init sequence.
     */
    void WaitUntilInitialized();
    /// Blocks until the coroutine becomes running
    void WaitUntilResumed();
    /// Moves the main coroutine to Status::TERMINATING
    void MainThreadFinished();
    /// Moves the main coroutine to Status::AWAIT_LOOP
    void EnterAwaitLoop();

    Coroutine::Status GetStatus() const override;

protected:
    void SetStatus(Coroutine::Status newStatus) override;
    os::thread::NativeHandleType GetCoroutineNativeHandle();
    void SetCoroutineNativeHandle(os::thread::NativeHandleType h);

private:
    /// std::thread's body
    static void ThreadProc(ThreadedCoroutineContext *ctx);
    /**
     * Notify the waiters that the coroutine has finished its initialization sequence.
     * Unblocks the thread waiting on the WaitUntilInitialized()
     */
    void InitializationDone();

    os::memory::ConditionVariable cv_;
    os::memory::Mutex cvMutex_;
    os::thread::NativeHandleType nativeHandle_ {};

    std::atomic<Coroutine::Status> status_ {Coroutine::Status::CREATED};
};

}  // namespace ark

#endif  // PANDA_RUNTIME_COROUTINES_THREADED_COROUTINE_H
