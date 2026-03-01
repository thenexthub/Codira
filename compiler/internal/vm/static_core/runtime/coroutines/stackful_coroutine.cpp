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

#include "runtime/coroutines/coroutine.h"
#include "runtime/include/panda_vm.h"
#include "runtime/include/thread_scopes.h"
#include "runtime/coroutines/stackful_coroutine.h"
#include "runtime/coroutines/stackful_coroutine_manager.h"

#if defined(PANDA_TSAN_ON)
#include <sanitizer/tsan_interface.h>
#endif /* PANDA_TSAN_ON */

namespace ark {

// clang-tidy cannot detect that we are going to initialize context_ via getcontext()
// NOLINTNEXTLINE(cppcoreguidelines-pro-type-member-init)
StackfulCoroutineContext::StackfulCoroutineContext(uint8_t *stack, size_t stackSizeBytes)
    : stack_(stack), stackSizeBytes_(stackSizeBytes)
{
    fibers::GetCurrentContext(&context_);
#if defined(PANDA_TSAN_ON)
    if (stack == nullptr) {
        // entrypointless coroutine uses the current thread context
        tsanFiberCtx_ = __tsan_get_current_fiber();
    } else {
        tsanFiberCtx_ = __tsan_create_fiber(0);
    }
#endif /* PANDA_TSAN_ON */
}

StackfulCoroutineContext::~StackfulCoroutineContext()
{
#if defined(PANDA_TSAN_ON)
    if (stack_ != nullptr) {
        __tsan_destroy_fiber(tsanFiberCtx_);
    }
#else
    // make clang-tidy happy! this is not a trivial dtor!
    ;
#endif /* PANDA_TSAN_ON */
}

void StackfulCoroutineContext::AttachToCoroutine(Coroutine *co)
{
    CoroutineContext::AttachToCoroutine(co);
    if (co->HasManagedEntrypoint() || co->HasNativeEntrypoint()) {
        fibers::UpdateContext(&context_, CoroThreadProc, this, stack_, stackSizeBytes_);
    }
    co->GetManager()->RegisterCoroutine(co);
    SetStatus(Coroutine::Status::RUNNABLE);
}

bool StackfulCoroutineContext::RetrieveStackInfo(void *&stackAddr, size_t &stackSize, size_t &guardSize)
{
    stackAddr = stack_;
    stackSize = stackSizeBytes_;
    guardSize = 0;
    return true;
}

Coroutine::Status StackfulCoroutineContext::GetStatus() const
{
    return status_;
}

StackfulCoroutineManager *StackfulCoroutineContext::GetManager() const
{
    ASSERT(GetCoroutine() != nullptr);
    return static_cast<StackfulCoroutineManager *>(GetCoroutine()->GetManager());
}

void StackfulCoroutineContext::SetStatus(Coroutine::Status newStatus)
{
    ASSERT(GetCoroutine() != nullptr);
#ifndef NDEBUG
    PandaString setter = (Thread::GetCurrent() == nullptr) ? "null" : Coroutine::GetCurrent()->GetName();
    LOG(DEBUG, COROUTINES) << GetCoroutine()->GetName() << ": " << status_ << " -> " << newStatus << " by " << setter;
#endif
    Coroutine::Status oldStatus = status_;
    status_ = newStatus;
    GetCoroutine()->OnStatusChanged(oldStatus, newStatus);
}

void StackfulCoroutineContext::Destroy()
{
    auto *co = GetCoroutine();
    if (co->HasManagedEntrypoint()) {
        // coroutines with an entry point should not be destroyed manually!
        UNREACHABLE();
    }
    ASSERT(co == Coroutine::GetCurrent());
    ASSERT(co->GetStatus() != ThreadStatus::FINISHED);

    co->UpdateStatus(ThreadStatus::TERMINATING);
    SetStatus(Coroutine::Status::TERMINATING);

    if (co->GetManager()->TerminateCoroutine(co)) {
        // detach
        Coroutine::SetCurrent(nullptr);
    }
}

void StackfulCoroutineContext::CleanUp()
{
#ifdef PANDA_ASAN_ON
    void *contextStackP;
    size_t contextStackSize;
    size_t contextGuardSize;
    RetrieveStackInfo(contextStackP, contextStackSize, contextGuardSize);
    ASAN_UNPOISON_MEMORY_REGION(contextStackP, contextStackSize);
#endif  // PANDA_ASAN_ON
    affinityMask_ = AffinityMask::Empty();
}

/*static*/
void StackfulCoroutineContext::CoroThreadProc(void *ctx)
{
    static_cast<StackfulCoroutineContext *>(ctx)->ThreadProcImpl();
}

void StackfulCoroutineContext::ThreadProcImpl()
{
    auto *co = GetCoroutine();
    // profiling: the interval was started in the ctxswitch
    GetWorker()->GetPerfStats().FinishInterval(CoroutineTimeStats::CTX_SWITCH);
    // consider changing this to INIT later on...
    GetWorker()->GetPerfStats().StartInterval(CoroutineTimeStats::SCH_ALL);
    // new coroutine startup event
    GetWorker()->OnNewCoroutineStartup(co);

    co->NativeCodeBegin();
    SetStatus(Coroutine::Status::RUNNING);
    if (co->HasManagedEntrypoint()) {
        ScopedManagedCodeThread s(co);
        PandaVector<Value> args = std::move(co->GetManagedEntrypointArguments());
        // profiling
        GetWorker()->GetPerfStats().FinishInterval(CoroutineTimeStats::SCH_ALL);
        Value result = co->GetManagedEntrypoint()->Invoke(co, args.data());
        co->RequestCompletion(result);
    } else if (co->HasNativeEntrypoint()) {
        // profiling: jump to the NATIVE EP, will end the SCH_ALL there
        co->GetNativeEntrypoint()(co->GetNativeEntrypointParam());
    }
    if (co->HasPendingException() && co->HasAbortFlag()) {
        co->HandleUncaughtException();
    }
    SetStatus(Coroutine::Status::TERMINATING);
    co->GetManager()->TerminateCoroutine(co);
}

bool StackfulCoroutineContext::SwitchTo(StackfulCoroutineContext *target)
{
    ASSERT(target != nullptr);
#if defined(PANDA_TSAN_ON)
    __tsan_switch_to_fiber(target->tsanFiberCtx_, 0);
#endif /* PANDA_TSAN_ON */
    fibers::SwitchContext(&context_, &target->context_);
    // maybe eventually we will check the return value of SwitchContext() and return false in case of error...
    return true;
}

void StackfulCoroutineContext::RequestSuspend(bool getsBlocked)
{
#ifdef ARK_HYBRID
    GetCoroutine()->UnbindMutator();
#endif
    SetStatus(getsBlocked ? Coroutine::Status::BLOCKED : Coroutine::Status::RUNNABLE);
}

void StackfulCoroutineContext::RequestResume()
{
#ifdef ARK_HYBRID
    GetCoroutine()->BindMutator();
#endif
    UpdateId(os::thread::GetCurrentThreadId(), GetCoroutine());
    SetStatus(Coroutine::Status::RUNNING);
}

void StackfulCoroutineContext::RequestUnblock()
{
    SetStatus(Coroutine::Status::RUNNABLE);
}

void StackfulCoroutineContext::MainThreadFinished()
{
    SetStatus(Coroutine::Status::TERMINATING);
}

void StackfulCoroutineContext::EnterAwaitLoop()
{
    SetStatus(Coroutine::Status::AWAIT_LOOP);
}

}  // namespace ark
