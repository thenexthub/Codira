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
#include "plugins/ets/runtime/ets_coroutine.h"
#include "plugins/ets/runtime/ets_exceptions.h"
#include "plugins/ets/runtime/types/ets_promise.h"
#include "plugins/ets/runtime/types/ets_job.h"
#include "plugins/ets/runtime/ets_class_linker.h"
#include "plugins/ets/runtime/ets_handle_scope.h"
#include "plugins/ets/runtime/ets_handle.h"
#include "plugins/ets/runtime/ets_panda_file_items.h"
#include "runtime/include/object_header.h"
#include "types/ets_object.h"

namespace ark::ets::intrinsics {
extern "C" {
EtsObject *EtsAwaitJob(EtsJob *job)
{
    EtsCoroutine *currentCoro = EtsCoroutine::GetCurrent();
    if (job == nullptr) {
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
    EtsHandle<EtsJob> jobHandle(currentCoro, job);

    LOG(DEBUG, COROUTINES) << "Job::await: starting await() for a job...";
    jobHandle->Wait();
    ASSERT(!jobHandle->IsRunning());
    LOG(DEBUG, COROUTINES) << "Job::await: await() finished.";

    if (jobHandle->IsFinished()) {
        LOG(DEBUG, COROUTINES) << "Job::await: job is already successfully finished!";
        return jobHandle->GetValue(currentCoro);
    }
    LOG(DEBUG, COROUTINES) << "Job::await: job is failed!";

    if (Runtime::GetOptions().IsListUnhandledOnExitJobs(plugins::LangToRuntimeType(panda_file::SourceLang::ETS))) {
        currentCoro->GetPandaVM()->GetUnhandledObjectManager()->RemoveFailedJob(jobHandle.GetPtr());
    }

    auto *exc = jobHandle->GetValue(currentCoro);
    currentCoro->SetException(exc->GetCoreType());
    return nullptr;
}

void EtsFinishJob(EtsJob *job, EtsObject *value)
{
    EtsCoroutine *coro = EtsCoroutine::GetCurrent();
    if (job == nullptr) {
        LanguageContext ctx = Runtime::GetCurrent()->GetLanguageContext(panda_file::SourceLang::ETS);
        ThrowNullPointerException(ctx, coro);
        return;
    }
    [[maybe_unused]] EtsHandleScope scope(coro);
    EtsHandle<EtsJob> hjob(coro, job);
    EtsHandle<EtsObject> hvalue(coro, value);
    EtsMutex::LockHolder lh(hjob);
    if (!hjob->IsRunning()) {
        return;
    }
    hjob->Finish(coro, hvalue.GetPtr());
}

void EtsFailJob(EtsJob *job, EtsObject *error)
{
    EtsCoroutine *coro = EtsCoroutine::GetCurrent();
    if (job == nullptr) {
        LanguageContext ctx = Runtime::GetCurrent()->GetLanguageContext(panda_file::SourceLang::ETS);
        ThrowNullPointerException(ctx, coro);
        return;
    }
    [[maybe_unused]] EtsHandleScope scope(coro);
    EtsHandle<EtsJob> hjob(coro, job);
    ASSERT(error != nullptr);
    EtsHandle<EtsObject> herror(coro, error);
    EtsMutex::LockHolder lh(hjob);

    EtsClassLinker *classLinker = coro->GetPandaVM()->GetClassLinker();
    auto *errorClass = classLinker->GetClass(panda_file_items::class_descriptors::ERROR.data());
    if (!herror->IsInstanceOf(errorClass)) {
        ThrowRuntimeException("fail() argument is not an Error object");
        return;
    }

    if (!hjob->IsRunning()) {
        return;
    }
    hjob->Fail(coro, herror.GetPtr());
}
}
}  // namespace ark::ets::intrinsics
