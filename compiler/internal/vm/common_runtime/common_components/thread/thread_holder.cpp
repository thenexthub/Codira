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

#include "common_interfaces/thread/thread_holder-inl.h"

#include "common_components/common_runtime/hooks.h"
#include "common_components/mutator/mutator.h"
#include "common_components/mutator/thread_local.h"
#include "common_interfaces/base_runtime.h"
#include "common_interfaces/thread/base_thread.h"
#include "common_interfaces/thread/thread_holder_manager.h"
#include "common_interfaces/thread/thread_state_transition.h"

#ifdef PANDA_JS_ETS_HYBRID_MODE
namespace common {
InterOpCoroutineToNativeHookFunc interOpCoroutineToNativeHook = nullptr;
InterOpCoroutineToRunningHookFunc interOpCoroutineToRunningHook = nullptr;

bool InterOpCoroutineToNative(ThreadHolder *current)
{
    if (interOpCoroutineToNativeHook == nullptr) {
        return false;
    }
    return interOpCoroutineToNativeHook(current);
}

bool InterOpCoroutineToRunning(ThreadHolder *current)
{
    if (interOpCoroutineToRunningHook == nullptr) {
        return false;
    }
    return interOpCoroutineToRunningHook(current);
}

void RegisterInterOpCoroutineToNativeHook(InterOpCoroutineToNativeHookFunc func)
{
    interOpCoroutineToNativeHook = func;
}
void RegisterInterOpCoroutineToRunningHook(InterOpCoroutineToRunningHookFunc func)
{
    interOpCoroutineToRunningHook = func;
}
}
#endif

namespace common {
thread_local ThreadHolder *currentThreadHolder = nullptr;

ThreadHolder *ThreadHolder::CreateAndRegisterNewThreadHolder(void *vm)
{
    if (ThreadLocal::IsArkProcessor()) {
        LOG_COMMON(FATAL) << "CreateAndRegisterNewThreadHolder fail";
        return nullptr;
    }
    Mutator* mutator = Mutator::NewMutator();
    CHECK_CC(mutator != nullptr);
    mutator->SetEcmaVMPtr(vm);
    ThreadHolder *holder = mutator->GetThreadHolder();
    BaseRuntime::GetInstance()->GetThreadHolderManager().RegisterThreadHolder(holder);
    return holder;
}

void ThreadHolder::ReleaseAllocBuffer()
{
    ThreadManagedScope scope(this);
    if (allocBuffer_) {
        auto buf = reinterpret_cast<AllocationBuffer*>(allocBuffer_);
        if (buf->DecreaseRefCount()) {
            buf->Unregister();
            delete buf;
        }
        allocBuffer_ = nullptr;
    }
}

void ThreadHolder::DestroyThreadHolder(ThreadHolder *holder)
{
    holder->ReleaseAllocBuffer();
    holder->TransferToNative();
    BaseRuntime::GetInstance()->GetThreadHolderManager().UnregisterThreadHolder(holder);
}

ThreadHolder *ThreadHolder::GetCurrent()
{
    return currentThreadHolder;
}

void ThreadHolder::SetCurrent(ThreadHolder *holder)
{
    currentThreadHolder = holder;
}

void ThreadHolder::RegisterJSThread(JSThread *jsThread)
{
    DCHECK_CC(!IsInRunningState());
    TransferToRunning();
    DCHECK_CC(jsThread_ == nullptr);
    jsThread_ = jsThread;
    mutatorBase_->RegisterJSThread(jsThread);
    SynchronizeGCPhaseToJSThread(jsThread, mutatorBase_->GetMutatorPhase());
    TransferToNative();
}

void ThreadHolder::UnregisterJSThread(JSThread *jsThread)
{
    DCHECK_CC(!IsInRunningState());
    TransferToRunning();
    DCHECK_CC(jsThread_ == jsThread);
    jsThread_ = nullptr;
    mutatorBase_->UnregisterJSThread();
    if (coroutines_.empty()) {
        ThreadHolder::DestroyThreadHolder(this);
    }
}

void ThreadHolder::RegisterCoroutine(Coroutine *coroutine)
{
    // Expect in Native when calling this func
    ThreadManagedScope scope(this);
    DCHECK_CC(coroutines_.find(coroutine) == coroutines_.end());
    coroutines_.insert(coroutine);
}

void ThreadHolder::UnregisterCoroutine(Coroutine *coroutine)
{
    DCHECK_CC(!IsInRunningState());
    TransferToRunning();
    DCHECK_CC(coroutines_.find(coroutine) != coroutines_.end());
    coroutines_.erase(coroutine);
    if (coroutines_.empty() && jsThread_ == nullptr) {
        ThreadHolder::DestroyThreadHolder(this);
    }
}

bool ThreadHolder::TryBindMutator()
{
    if (ThreadLocal::IsArkProcessor()) {
        return false;
    }

    BaseRuntime::GetInstance()->GetThreadHolderManager().BindMutator(this);
    allocBuffer_ = ThreadLocal::GetAllocBuffer();
    reinterpret_cast<AllocationBuffer*>(allocBuffer_)->IncreaseRefCount();
    DCHECK_CC(allocBuffer_ != nullptr);
    return true;
}

void ThreadHolder::BindMutator()
{
    if (!TryBindMutator()) {
        LOG_COMMON(FATAL) << "BindMutator fail";
        return;
    }
}

void ThreadHolder::UnbindMutator()
{
    allocBuffer_ = nullptr;
    BaseRuntime::GetInstance()->GetThreadHolderManager().UnbindMutator(this);
}

void ThreadHolder::WaitSuspension()
{
    mutatorBase_->HandleSuspensionRequest();
}

void ThreadHolder::VisitAllThreads(CommonRootVisitor visitor)
{
    if (jsThread_ != nullptr) {
        VisitJSThread(jsThread_, visitor);
    }
    for ([[maybe_unused]] auto *coroutine : coroutines_) {
        // Depending on the integrated so
    }
}

ThreadHolder::TryBindMutatorScope::TryBindMutatorScope(ThreadHolder *holder) : holder_(nullptr)
{
    if (holder->TryBindMutator()) {
        holder_ = holder;
    }
}

ThreadHolder::TryBindMutatorScope::~TryBindMutatorScope()
{
    if (holder_ != nullptr) {
        holder_->ReleaseAllocBuffer();
        holder_->UnbindMutator();
        holder_ = nullptr;
    }
}
} // namespace common
