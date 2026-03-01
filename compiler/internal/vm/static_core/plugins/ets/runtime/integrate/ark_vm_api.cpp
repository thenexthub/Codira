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

#include "plugins/ets/runtime/integrate/ark_vm_api.h"
#include "plugins/ets/runtime/ets_coroutine.h"
#include "plugins/ets/runtime/main_worker_external_scheduler.h"
#include "libarkbase/macros.h"

// CC-OFFNXT(G.FUN.02-CPP) project code style
extern "C" arkvm_status ARKVM_RegisterExternalScheduler(arkvm_external_scheduler_poster poster)
{
    if (poster == nullptr) {
        return ARKVM_INVALID_ARGS;
    }

    auto *thread = ark::Thread::GetCurrent();
    if (thread == nullptr) {
        return ARKVM_INVALID_CONTEXT;
    }
    auto *coroutine = ark::ets::EtsCoroutine::GetCurrent();
    if (coroutine == nullptr || coroutine != coroutine->GetCoroutineManager()->GetMainThread()) {
        return ARKVM_INVALID_CONTEXT;
    }

    auto mainPoster = ark::MakePandaUnique<ark::ets::MainWorkerExternalScheduler>(poster);
    coroutine->GetWorker()->SetCallbackPoster(std::move(mainPoster));
    return ARKVM_OK;
}

// CC-OFFNXT(G.FUN.02-CPP) project code style
extern "C" arkvm_status ARKVM_RunScheduler(arkvm_schedule_mode mode)
{
    if (mode != ARKVM_SCHEDULER_RUN_ONCE) {
        return ARKVM_INVALID_ARGS;
    }

    auto *thread = ark::Thread::GetCurrent();
    if (thread == nullptr) {
        return ARKVM_INVALID_CONTEXT;
    }
    auto *coroutine = ark::ets::EtsCoroutine::GetCurrent();
    if (coroutine == nullptr) {
        return ARKVM_INVALID_CONTEXT;
    }

    coroutine->GetCoroutineManager()->Schedule();
    return ARKVM_OK;
}
