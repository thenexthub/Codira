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

#include "plugins/ets/runtime/main_worker_external_scheduler.h"
#include "plugins/ets/runtime/ets_coroutine.h"

namespace ark::ets {

static void ScheduleWrapper(void *param);

MainWorkerExternalScheduler::MainWorkerExternalScheduler(TaskPosterFunc poster)
    : needTriggerScheduler_(true), poster_(poster)
{
}

void MainWorkerExternalScheduler::PostImpl(int64_t delayMs)
{
    if (delayMs <= 0) {
        if (needTriggerScheduler_.exchange(false)) {
            poster_(ScheduleWrapper, &needTriggerScheduler_, "ScheduleCoroutine", 1);
        }
    } else {
        auto schedFunc = []([[maybe_unused]] void *param) {
            EtsCoroutine::GetCurrent()->GetCoroutineManager()->Schedule();
        };
        poster_(schedFunc, nullptr, "ScheduleCoroutine", delayMs);
    }
}

static void ScheduleWrapper(void *param)
{
    auto *needTriggerScheduler = static_cast<std::atomic<bool> *>(param);
    *needTriggerScheduler = true;
    auto *coroMan = EtsCoroutine::GetCurrent()->GetCoroutineManager();
    ASSERT(EtsCoroutine::GetCurrent() == coroMan->GetMainThread());
    coroMan->Schedule();
}

}  // namespace ark::ets