/**
 * Copyright (c) 2023-2025 Huawei Device Co., Ltd.
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
#ifndef PANDA_RUNTIME_COROUTINES_STACKFUL_COROUTINE_H
#define PANDA_RUNTIME_COROUTINES_STACKFUL_COROUTINE_H

#include "runtime/fibers/fiber_context.h"
#include "runtime/coroutines/coroutine_context.h"
#include "runtime/include/panda_vm.h"
#include "runtime/coroutines/affinity_mask.h"
#include "runtime/coroutines/stackful_coroutine_worker.h"

namespace ark {

/**
 * @brief Native context of a coroutine. "Fiber"-based implementation.
 *
 * This implementation uses panda fibers library to implement native coroutine context.
 */
class StackfulCoroutineContext : public CoroutineContext {
public:
    NO_COPY_SEMANTIC(StackfulCoroutineContext);
    NO_MOVE_SEMANTIC(StackfulCoroutineContext);

    /**
     * @param stack specifies the lowest address of the stack region to use;
     * it should have at least @param stack_size_bytes bytes accessible. If the stack grows down on the
     * target architecture, then the initial stack pointer of the coroutine will be set to
     * (stack + stack_size_bytes)
     */
    explicit StackfulCoroutineContext(uint8_t *stack, size_t stackSizeBytes);
    ~StackfulCoroutineContext() override;

    /**
     * Prepares the context for execution, links it to the managed context part (Coroutine instance) and registers the
     * created coroutine in the CoroutineManager (in the RUNNABLE status)
     */
    void AttachToCoroutine(Coroutine *co) override;
    /**
     * Manually destroys the context. Should be called by the Coroutine instance as a part of main coroutine
     * destruction. All other coroutines and their contexts are destroyed by the CoroutineManager once the coroutine
     * entrypoint execution finishes
     */
    void Destroy() override;

    void CleanUp() override;

    bool RetrieveStackInfo(void *&stackAddr, size_t &stackSize, size_t &guardSize) override;

    /**
     * Suspends the execution context, sets its status to either Status::RUNNABLE or Status::BLOCKED, depending on the
     * suspend reason.
     */
    void RequestSuspend(bool getsBlocked) override;
    /// Resumes the suspended context, sets status to RUNNING.
    void RequestResume() override;
    /// Unblock the coroutine and set its status to Status::RUNNABLE
    void RequestUnblock() override;

    // should be called then the main thread is done executing the program entrypoint
    void MainThreadFinished();
    /// Moves the main coroutine to Status::AWAIT_LOOP
    void EnterAwaitLoop();

    /// Coroutine status is a part of native context, because it might require some synchronization on access
    Coroutine::Status GetStatus() const override;

    /**
     * Transfer control to the target context
     * NB: this method will return only after the control is transferred back to the caller context
     */
    bool SwitchTo(StackfulCoroutineContext *target);

    /// @return the lowest address of the coroutine native stack (provided in the ctx contructor)
    uint8_t *GetStackLoAddrPtr() const
    {
        return stack_;
    }

    /// Executes a foreign lambda function within this context (does not corrupt the saved context)
    template <class L>
    bool ExecuteOnThisContext(L *lambda, StackfulCoroutineContext *requester)
    {
        ASSERT_PRINT(false, "This method should not be called");
        ASSERT(requester != nullptr);
        return rpcCallContext_.Execute(lambda, &requester->context_, &context_);
    }

    /// get the currently assigned worker thread
    StackfulCoroutineWorker *GetWorker() const
    {
        auto *coro = GetCoroutine();
        ASSERT(coro != nullptr);
        return reinterpret_cast<StackfulCoroutineWorker *>(coro->GetWorker());
    }

    /// @return current coroutine's affinity bits
    AffinityMask GetAffinityMask() const
    {
        return affinityMask_;
    }

    void SetAffinityMask(const AffinityMask &mask)
    {
        affinityMask_ = mask;
    }

    /// @return true if migration from worker is allowed
    bool IsMigrationAllowed() const
    {
        return affinityMask_.NumAllowedWorkers() > 1;
    }

protected:
    void SetStatus(Coroutine::Status newStatus) override;
    StackfulCoroutineManager *GetManager() const;

private:
    void ThreadProcImpl();
    static void CoroThreadProc(void *ctx);

    /// @brief The remote lambda call functionality implementation.
    // NOLINTNEXTLINE(cppcoreguidelines-pro-type-member-init)
    class RemoteCall {
    public:
        NO_COPY_SEMANTIC(RemoteCall);
        NO_MOVE_SEMANTIC(RemoteCall);

        template <class L>
        bool Execute(L *lambda, fibers::FiberContext *requesterContextPtr, fibers::FiberContext *hostContextPtr)
        {
            ASSERT(Coroutine::GetCurrent()->GetVM()->GetThreadManager()->GetMainThread() !=
                   ManagedThread::GetCurrent());

            callInProgress_ = true;
            requesterContextPtr_ = requesterContextPtr;
            lambda_ = lambda;

            fibers::CopyContext(&guestContext_, hostContextPtr);
            fibers::UpdateContextKeepStack(&guestContext_, RemoteCall::Proxy<L>, this);
            fibers::SwitchContext(requesterContextPtr_, &guestContext_);

            return true;
        }

        // NOLINTNEXTLINE(cppcoreguidelines-pro-type-member-init)
        RemoteCall() = default;
        ~RemoteCall() = default;

    private:
        template <class L>
        static void Proxy(void *ctx)
        {
            auto *thisInstance = static_cast<RemoteCall *>(ctx);
            ASSERT(thisInstance->callInProgress_);

            (*static_cast<L *>(thisInstance->lambda_))();

            thisInstance->callInProgress_ = false;
            fibers::SwitchContext(&thisInstance->guestContext_, thisInstance->requesterContextPtr_);
        }

        bool callInProgress_ = false;
        fibers::FiberContext *requesterContextPtr_ = nullptr;
        fibers::FiberContext guestContext_;
        void *lambda_ = nullptr;
    } rpcCallContext_;

    uint8_t *stack_ = nullptr;
    size_t stackSizeBytes_ = 0;
    fibers::FiberContext context_;
    Coroutine::Status status_ {Coroutine::Status::CREATED};
    AffinityMask affinityMask_ = AffinityMask::Empty();
#if defined(PANDA_TSAN_ON)
    void *tsanFiberCtx_ {nullptr};
#endif /* PANDA_TSAN_ON */
};

}  // namespace ark

#endif  // PANDA_RUNTIME_COROUTINES_STACKFUL_COROUTINE_H
