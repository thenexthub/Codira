/**
 * Copyright (c) 2021-2024 Huawei Device Co., Ltd.
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

#include "default_debugger_agent.h"

namespace ark {
DefaultDebuggerAgent::DefaultDebuggerAgent(os::memory::Mutex &mutex)
    : LibraryAgent(mutex, PandaString(Runtime::GetOptions().GetDebuggerLibraryPath()), "StartDebugger", "StopDebugger")
{
}

bool DefaultDebuggerAgent::Load()
{
    debugSession_ = Runtime::GetCurrent()->StartDebugSession();
    if (!debugSession_) {
        LOG(ERROR, RUNTIME) << "Could not start debug session";
        return false;
    }

    if (!LibraryAgent::Load()) {
        debugSession_.reset();
        return false;
    }

    return true;
}

bool DefaultDebuggerAgent::Unload()
{
    auto result = LibraryAgent::Unload();
    debugSession_.reset();
    return result;
}

bool DefaultDebuggerAgent::CallLoadCallback(void *resolvedFunction)
{
    ASSERT(resolvedFunction);
    ASSERT(debugSession_);

    using StartDebuggerT = int (*)(uint32_t, bool);
    uint32_t port = Runtime::GetOptions().GetDebuggerPort();
    bool breakOnStart = Runtime::GetOptions().IsDebuggerBreakOnStart();
    int res = reinterpret_cast<StartDebuggerT>(resolvedFunction)(port, breakOnStart);
    if (res != 0) {
        LOG(ERROR, RUNTIME) << "'StartDebugger' has failed with " << res;
        return false;
    }

    return true;
}

bool DefaultDebuggerAgent::CallUnloadCallback(void *resolvedFunction)
{
    ASSERT(resolvedFunction);

    using StopDebugger = int (*)();
    int res = reinterpret_cast<StopDebugger>(resolvedFunction)();
    if (res != 0) {
        LOG(ERROR, RUNTIME) << "'StopDebugger' has failed with " << res;
        return false;
    }

    return true;
}
}  // namespace ark
