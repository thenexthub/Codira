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

#include "runtime/coroutines/coroutine_manager.h"
#include "runtime/coroutines/coroutine_events.h"
#include "plugins/ets/runtime/types/ets_promise.h"
#include "plugins/ets/runtime/ets_coroutine.h"
#include "plugins/ets/runtime/ets_platform_types.h"
#include "plugins/ets/runtime/ets_vm.h"
#include "plugins/ets/runtime/types/ets_method.h"

namespace ark::ets {

/*static*/
EtsPromise *EtsPromise::Create(EtsCoroutine *coro)
{
    [[maybe_unused]] EtsHandleScope scope(coro);
    auto *klass = PlatformTypes(coro)->corePromise;
    auto hPromise = EtsHandle<EtsPromise>(coro, EtsPromise::FromEtsObject(EtsObject::Create(coro, klass)));
    ASSERT(hPromise.GetPtr() != nullptr);
    auto *mutex = EtsMutex::Create(coro);
    hPromise->SetMutex(coro, mutex);
    auto *event = EtsEvent::Create(coro);
    hPromise->SetEvent(coro, event);
    return hPromise.GetPtr();
}

void EtsPromise::OnPromiseCompletion(EtsCoroutine *coro)
{
    auto *cbQueue = GetCallbackQueue(coro);
    auto *workerDomainQueue = GetWorkerDomainQueue(coro);
    auto queueSize = GetQueueSize();
    ASSERT(queueSize == 0 || cbQueue != nullptr);
    ASSERT(queueSize == 0 || workerDomainQueue != nullptr);

    if (Runtime::GetOptions().IsListUnhandledOnExitPromises(plugins::LangToRuntimeType(panda_file::SourceLang::ETS)) &&
        state_ == STATE_REJECTED && queueSize == 0) {
        coro->GetPandaVM()->GetUnhandledObjectManager()->AddRejectedPromise(this, coro);
    }

    // Unblock awaitee coros
    GetEvent(coro)->Fire();

    for (int idx = 0; idx < queueSize; ++idx) {
        auto *thenCallback = cbQueue->Get(idx);
        auto workerDomain = static_cast<CoroutineWorkerDomain>(workerDomainQueue->Get(idx));
        ASSERT(workerDomain == CoroutineWorkerDomain::MAIN || workerDomain == CoroutineWorkerDomain::GENERAL);
        auto groupId = workerDomain == CoroutineWorkerDomain::MAIN
                           ? CoroutineWorkerGroup::FromDomain(coro->GetCoroutineManager(), CoroutineWorkerDomain::MAIN)
                           : CoroutineWorkerGroup::AnyId();
        EtsPromise::LaunchCallback(coro, thenCallback, groupId);
    }
    ClearQueues(coro);
}

/* static */
void EtsPromise::LaunchCallback(EtsCoroutine *coro, EtsObject *callback, const CoroutineWorkerGroup::Id &groupId)
{
    // Launch callback in its own coroutine
    auto *coroManager = coro->GetCoroutineManager();
    auto *event = Runtime::GetCurrent()->GetInternalAllocator()->New<CompletionEvent>(nullptr, coroManager);

    EtsMethod *etsmethod = callback->GetClass()->GetInstanceMethod(INVOKE_METHOD_NAME, nullptr);
    auto *method = EtsMethod::ToRuntimeMethod(etsmethod);
    ASSERT(method != nullptr);
    auto args = PandaVector<Value> {Value(callback->GetCoreType())};
    auto launchResult =
        coroManager->Launch(event, method, std::move(args), groupId, EtsCoroutine::PROMISE_CALLBACK, false);
    if (UNLIKELY(launchResult == LaunchResult::COROUTINES_LIMIT_EXCEED)) {
        Runtime::GetCurrent()->GetInternalAllocator()->Delete(event);
    }
}

}  // namespace ark::ets
