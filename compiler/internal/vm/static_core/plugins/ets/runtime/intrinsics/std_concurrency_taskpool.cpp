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

#include <atomic>
#include <thread>
#include "libarkbase/os/time.h"
#include "plugins/ets/runtime/types/ets_primitives.h"
#include "plugins/ets/runtime/types/ets_taskpool.h"
#include "runtime/include/runtime.h"
#include "plugins/ets/runtime/ets_coroutine.h"

namespace ark::ets::intrinsics::taskpool {

static std::atomic<EtsInt> g_taskId = 1;
static std::atomic<EtsInt> g_taskGroupId = 1;
static std::atomic<EtsInt> g_seqRunnerId = 1;
static std::atomic<EtsInt> g_asyncRunnerId = 1;

extern "C" EtsInt GenerateTaskId()
{
    return g_taskId++;
}

extern "C" EtsInt GetTaskId()
{
    EtsCoroutine *coro = EtsCoroutine::GetCurrent();
    ASSERT(coro != nullptr);
    auto id = coro->GetTaskpoolTaskId();
    return id;
}

extern "C" void SetTaskId(EtsInt taskId)
{
    EtsCoroutine *coro = EtsCoroutine::GetCurrent();
    ASSERT(coro != nullptr);
    coro->SetTaskpoolTaskId(taskId);
}

extern "C" EtsInt GenerateGroupId()
{
    return g_taskGroupId++;
}

extern "C" EtsInt GenerateSeqRunnerId()
{
    return g_seqRunnerId++;
}

extern "C" EtsInt GenerateAsyncRunnerId()
{
    return g_asyncRunnerId++;
}

extern "C" EtsBoolean IsUsingLaunchMode()
{
    const auto &taskpoolMode =
        Runtime::GetOptions().GetTaskpoolMode(plugins::LangToRuntimeType(panda_file::SourceLang::ETS));

    bool res = (taskpoolMode == TASKPOOL_LAUNCH_MODE);
    return ark::ets::ToEtsBoolean(res);
}

extern "C" EtsBoolean IsSupportingInterop()
{
    bool res = false;
#ifdef PANDA_ETS_INTEROP_JS
    res = Runtime::GetOptions().IsTaskpoolSupportInterop(plugins::LangToRuntimeType(panda_file::SourceLang::ETS));
#endif /* PANDA_ETS_INTEROP_JS */
    return ark::ets::ToEtsBoolean(res);
}

extern "C" EtsInt GetTaskPoolWorkersLimit()
{
    int32_t cpuCount = std::thread::hardware_concurrency();
    return cpuCount > 1 ? cpuCount - 1 : 1;  // 1: default number
}

}  // namespace ark::ets::intrinsics::taskpool
