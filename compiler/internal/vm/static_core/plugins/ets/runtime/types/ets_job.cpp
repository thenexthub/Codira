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

#include "plugins/ets/runtime/types/ets_job.h"
#include "plugins/ets/runtime/ets_coroutine.h"
#include "plugins/ets/runtime/ets_platform_types.h"
#include "plugins/ets/runtime/ets_vm.h"

namespace ark::ets {

/*static*/
EtsJob *EtsJob::Create(EtsCoroutine *coro)
{
    [[maybe_unused]] EtsHandleScope scope(coro);
    auto *klass = PlatformTypes(coro)->coreJob;
    auto hJobObject = EtsObject::Create(coro, klass);
    ASSERT(hJobObject != nullptr);
    auto hJob = EtsHandle<EtsJob>(coro, EtsJob::FromEtsObject(hJobObject));
    auto *mutex = EtsMutex::Create(coro);
    hJob->SetMutex(coro, mutex);
    auto *event = EtsEvent::Create(coro);
    hJob->SetEvent(coro, event);
    hJob->state_ = STATE_RUNNING;
    return hJob.GetPtr();
}

/*static*/
void EtsJob::EtsJobFinish(EtsJob *job, EtsObject *value)
{
    EtsCoroutine *coro = EtsCoroutine::GetCurrent();
    ASSERT(coro != nullptr);
    if (job == nullptr) {
        LanguageContext ctx = Runtime::GetCurrent()->GetLanguageContext(panda_file::SourceLang::ETS);
        ThrowNullPointerException(ctx, coro);
        return;
    }
    [[maybe_unused]] EtsHandleScope scope(coro);
    EtsHandle<EtsJob> hJob(coro, job);
    ASSERT(hJob.GetPtr() != nullptr);
    EtsHandle<EtsObject> hvalue(coro, value);
    EtsMutex::LockHolder lh(hJob);
    if (!hJob->IsRunning()) {
        return;
    }
    hJob->Finish(coro, hvalue.GetPtr());
}

/*static*/
void EtsJob::EtsJobFail(EtsJob *job, EtsObject *error)
{
    EtsCoroutine *coro = EtsCoroutine::GetCurrent();
    ASSERT(coro != nullptr);
    if (job == nullptr) {
        LanguageContext ctx = Runtime::GetCurrent()->GetLanguageContext(panda_file::SourceLang::ETS);
        ThrowNullPointerException(ctx, coro);
        return;
    }
    [[maybe_unused]] EtsHandleScope scope(coro);
    EtsHandle<EtsJob> hJob(coro, job);
    ASSERT(hJob.GetPtr() != nullptr);
    EtsHandle<EtsObject> herror(coro, error);
    EtsMutex::LockHolder lh(hJob);
    if (!hJob->IsRunning()) {
        return;
    }
    hJob->Fail(coro, herror.GetPtr());
}

}  // namespace ark::ets
