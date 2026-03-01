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

#include "intrinsics.h"
#include "plugins/ets/runtime/ets_utils.h"
#include "plugins/ets/runtime/ets_coroutine.h"
#include "plugins/ets/runtime/ets_exceptions.h"
#include "plugins/ets/runtime/ets_platform_types.h"
#include "plugins/ets/runtime/ets_vm.h"
#include "plugins/ets/runtime/types/ets_method.h"
#include "runtime/coroutines/coroutine_manager.h"
#include "plugins/ets/runtime/types/ets_promise.h"
#include "plugins/ets/runtime/ets_handle_scope.h"
#include "plugins/ets/runtime/ets_handle.h"
#include "plugins/ets/runtime/job_queue.h"
#include "runtime/coroutines/stackful_coroutine.h"
#include "runtime/coroutines/coroutine_events.h"
#include "runtime/include/mem/panda_containers.h"

namespace ark::ets::intrinsics {

void SubscribePromiseOnResultObject(EtsPromise *outsidePromise, EtsPromise *internalPromise)
{
    PandaVector<Value> args {Value(outsidePromise), Value(internalPromise)};

    PlatformTypes()->corePromiseSubscribeOnAnotherPromise->GetPandaMethod()->Invoke(EtsCoroutine::GetCurrent(),
                                                                                    args.data());
}

static void EnsureCapacity(EtsCoroutine *coro, EtsHandle<EtsPromise> &hpromise)
{
    ASSERT(hpromise.GetPtr() != nullptr);
    ASSERT(hpromise->IsLocked());
    int queueLength = hpromise->GetCallbackQueue(coro) == nullptr ? 0 : hpromise->GetCallbackQueue(coro)->GetLength();
    if (hpromise->GetQueueSize() != queueLength) {
        return;
    }
    auto newQueueLength = queueLength * 2U + 1U;
    auto *objectClass = coro->GetPandaVM()->GetClassLinker()->GetClassRoot(EtsClassRoot::OBJECT);
    auto *newCallbackQueue = EtsObjectArray::Create(objectClass, newQueueLength);
    if (hpromise->GetQueueSize() != 0) {
        hpromise->GetCallbackQueue(coro)->CopyDataTo(newCallbackQueue);
    }
    hpromise->SetCallbackQueue(coro, newCallbackQueue);
    auto *newWorkerDomainQueue = EtsIntArray::Create(newQueueLength);
    if (hpromise->GetQueueSize() != 0) {
        auto *workerDomainQueueData = hpromise->GetWorkerDomainQueue(coro)->GetData<EtsCoroutine *>();
        [[maybe_unused]] auto err =
            memcpy_s(newWorkerDomainQueue->GetData<CoroutineWorkerDomain>(), newQueueLength * sizeof(EtsInt),
                     workerDomainQueueData, queueLength * sizeof(CoroutineWorkerDomain));
        ASSERT(err == EOK);
    }
    hpromise->SetWorkerDomainQueue(coro, newWorkerDomainQueue);
}

void EtsPromiseResolve(EtsPromise *promise, EtsObject *value, EtsBoolean wasLinked)
{
    EtsCoroutine *coro = EtsCoroutine::GetCurrent();
    if (promise == nullptr) {
        LanguageContext ctx = Runtime::GetCurrent()->GetLanguageContext(panda_file::SourceLang::ETS);
        ThrowNullPointerException(ctx, coro);
        return;
    }
    [[maybe_unused]] EtsHandleScope scope(coro);
    EtsHandle<EtsPromise> hpromise(coro, promise);
    EtsHandle<EtsObject> hvalue(coro, value);

    if (wasLinked == 0) {
        /* When the value is still a Promise, the lock must be unlocked first. */
        hpromise->Lock();
        if (!hpromise->IsPending()) {
            hpromise->Unlock();
            return;
        }
        if (hvalue.GetPtr() != nullptr && hvalue->IsInstanceOf(PlatformTypes(coro)->corePromise)) {
            auto internalPromise = EtsPromise::FromEtsObject(hvalue.GetPtr());
            EtsHandle<EtsPromise> hInternalPromise(coro, internalPromise);
            hpromise->Unlock();
            SubscribePromiseOnResultObject(hpromise.GetPtr(), hInternalPromise.GetPtr());
            return;
        }
        hpromise->Resolve(coro, hvalue.GetPtr());
        hpromise->Unlock();
    } else {
        /* When the value is still a Promise, the lock must be unlocked first. */
        hpromise->Lock();
        if (!hpromise->IsLinked()) {
            hpromise->Unlock();
            return;
        }
        if (hvalue.GetPtr() != nullptr && hvalue->IsInstanceOf(PlatformTypes(coro)->corePromise)) {
            auto internalPromise = EtsPromise::FromEtsObject(hvalue.GetPtr());
            EtsHandle<EtsPromise> hInternalPromise(coro, internalPromise);
            hpromise->ChangeStateToPendingFromLinked();
            hpromise->Unlock();
            SubscribePromiseOnResultObject(hpromise.GetPtr(), hInternalPromise.GetPtr());
            return;
        }
        hpromise->Resolve(coro, hvalue.GetPtr());
        hpromise->Unlock();
    }
}

void EtsPromiseReject(EtsPromise *promise, EtsObject *error, EtsBoolean wasLinked)
{
    EtsCoroutine *coro = EtsCoroutine::GetCurrent();
    if (promise == nullptr) {
        LanguageContext ctx = Runtime::GetCurrent()->GetLanguageContext(panda_file::SourceLang::ETS);
        ThrowNullPointerException(ctx, coro);
        return;
    }
    [[maybe_unused]] EtsHandleScope scope(coro);
    EtsHandle<EtsPromise> hpromise(coro, promise);
    EtsHandle<EtsObject> herror(coro, error);
    EtsMutex::LockHolder lh(hpromise);
    if ((!hpromise->IsPending() && wasLinked == 0) || (!hpromise->IsLinked() && wasLinked != 0)) {
        return;
    }
    hpromise->Reject(coro, herror.GetPtr());
}

void EtsPromiseSubmitCallback(EtsPromise *promise, EtsObject *callback)
{
    auto *coro = EtsCoroutine::GetCurrent();
    ASSERT(coro != nullptr);
    auto workerDomain =
        coro->GetWorker()->IsMainWorker() ? CoroutineWorkerDomain::MAIN : CoroutineWorkerDomain::GENERAL;
    [[maybe_unused]] EtsHandleScope scope(coro);
    EtsHandle<EtsPromise> hpromise(coro, promise);
    EtsHandle<EtsObject> hcallback(coro, callback);
    EtsMutex::LockHolder lh(hpromise);
    if (hpromise->IsPending() || hpromise->IsLinked()) {
        EnsureCapacity(coro, hpromise);
        hpromise->SubmitCallback(coro, hcallback.GetPtr(), workerDomain);
        return;
    }
    if (Runtime::GetOptions().IsListUnhandledOnExitPromises(plugins::LangToRuntimeType(panda_file::SourceLang::ETS))) {
        coro->GetPandaVM()->GetUnhandledObjectManager()->RemoveRejectedPromise(hpromise.GetPtr(), coro);
    }
    ASSERT(hpromise->GetQueueSize() == 0);
    ASSERT(hpromise->GetCallbackQueue(coro) == nullptr);
    ASSERT(hpromise->GetWorkerDomainQueue(coro) == nullptr);
    auto groupId = workerDomain == CoroutineWorkerDomain::MAIN
                       ? CoroutineWorkerGroup::FromDomain(coro->GetCoroutineManager(), CoroutineWorkerDomain::MAIN)
                       : CoroutineWorkerGroup::AnyId();
    EtsPromise::LaunchCallback(coro, hcallback.GetPtr(), groupId);
}

static EtsObject *AwaitProxyPromise(EtsCoroutine *currentCoro, EtsHandle<EtsPromise> &promiseHandle)
{
    /**
     * This is a backed by JS equivalent promise.
     * ETS mode: error, no one can create such a promise!
     * JS mode:
     *      - add a callback to JQ, that will:
     *          - resolve the promise with some value OR reject it
     *          - unblock the coro via event
     *          - schedule();
     *      - create a blocker event and link it to the promise
     *      - block current coroutine on the event
     *      - Schedule();
     *          (the last two steps are actually the cm->await()'s job)
     *      - return promise.value() if resolved or throw() it if rejected
     */
    ASSERT(promiseHandle.GetPtr() != nullptr);
    promiseHandle->Wait();
    ASSERT(!promiseHandle->IsPending() && !promiseHandle->IsLinked());

    // will get here after the JS callback is called
    if (promiseHandle->IsResolved()) {
        LOG(DEBUG, COROUTINES) << "Promise::await: await() finished, promise has been resolved.";
        return promiseHandle->GetValue(currentCoro);
    }
    // rejected
    if (Runtime::GetOptions().IsListUnhandledOnExitPromises(plugins::LangToRuntimeType(panda_file::SourceLang::ETS))) {
        currentCoro->GetPandaVM()->GetUnhandledObjectManager()->RemoveRejectedPromise(promiseHandle.GetPtr(),
                                                                                      currentCoro);
    }
    LOG(DEBUG, COROUTINES) << "Promise::await: await() finished, promise has been rejected.";
    auto *exc = promiseHandle->GetValue(currentCoro);
    ASSERT(exc != nullptr);
    currentCoro->SetException(exc->GetCoreType());
    return nullptr;
}

EtsObject *EtsAwaitPromise(EtsPromise *promise)
{
    EtsCoroutine *currentCoro = EtsCoroutine::GetCurrent();
    if (promise == nullptr) {
        LanguageContext ctx = Runtime::GetCurrent()->GetLanguageContext(panda_file::SourceLang::ETS);
        ThrowNullPointerException(ctx, currentCoro);
        return nullptr;
    }
    if (currentCoro->GetCoroutineManager()->IsCoroutineSwitchDisabled()) {
        ThrowEtsException(currentCoro, panda_file_items::class_descriptors::INVALID_COROUTINE_OPERATION_ERROR,
                          "Cannot await in the current context!");
        return nullptr;
    }
    [[maybe_unused]] EtsHandleScope scope(currentCoro);
    EtsHandle<EtsPromise> promiseHandle(currentCoro, promise);

    {
        ScopedNativeCodeThread n(currentCoro);
        currentCoro->GetManager()->Schedule();
    }

    /* CASE 1. This is a converted JS promise */
    if (promiseHandle->IsProxy()) {
        return AwaitProxyPromise(currentCoro, promiseHandle);
    }

    /* CASE 2. This is a native ETS promise */
    LOG(DEBUG, COROUTINES) << "Promise::await: starting await() for a promise...";
    promiseHandle->Wait();
    ASSERT(!promiseHandle->IsPending() && !promiseHandle->IsLinked());
    LOG(DEBUG, COROUTINES) << "Promise::await: await() finished.";

    /**
     * The promise is already resolved or rejected. Further actions:
     *      ETS mode:
     *          if resolved: return Promise.value
     *          if rejected: throw Promise.value
     *      JS mode: NOTE!
     *          - suspend coro, create resolved JS promise and put it to the Q, on callback resume the coro
     *            and possibly throw
     *          - JQ::put(current_coro, promise)
     *
     */
    if (promiseHandle->IsResolved()) {
        LOG(DEBUG, COROUTINES) << "Promise::await: promise is already resolved!";
        return promiseHandle->GetValue(currentCoro);
    }
    if (Runtime::GetOptions().IsListUnhandledOnExitPromises(plugins::LangToRuntimeType(panda_file::SourceLang::ETS))) {
        currentCoro->GetPandaVM()->GetUnhandledObjectManager()->RemoveRejectedPromise(promiseHandle.GetPtr(),
                                                                                      currentCoro);
    }
    LOG(DEBUG, COROUTINES) << "Promise::await: promise is already rejected!";
    auto *exc = promiseHandle->GetValue(currentCoro);
    currentCoro->SetException(exc->GetCoreType());
    return nullptr;
}
}  // namespace ark::ets::intrinsics
