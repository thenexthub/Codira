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


#include <functional>
#include <string>

#include "library_loader.h"
#include "common/log_wrapper.h"
#include "init_static.h"

#if defined(UNIX_PLATFORM)
#include <dlfcn.h>
#elif defined(WINDOWS_PLATFORM)
#include <windows.h>
#ifdef ERROR
#undef ERROR
#endif
#else
#error "Unsupported platform"
#endif

namespace OHOS::ArkCompiler::Toolchain {


using InitializeInspectorFunc = void(*)(std::shared_ptr<void>, bool);
using HandleMessageFunc = void(*)(std::string&&);
using StopInspectorFunc = void(*)();
using WaitForDebuggerFunc = void(*)();
using StartDebuggerFunc = void(*)(uint32_t, bool);
using StopDebuggerFunc = void(*)();

static void* g_debuggerHandle = nullptr;

static InitializeInspectorFunc g_initializeInspectorForStatic = nullptr;
static HandleMessageFunc g_handleMessageForStatic = nullptr;
static StopInspectorFunc g_stopInspectorForStatic = nullptr;
static WaitForDebuggerFunc g_waitForDebuggerForStatic = nullptr;
static StartDebuggerFunc g_startDebuggerForStatic = nullptr;
static StopDebuggerFunc g_stopDebuggerForStatic = nullptr;

bool InitializeArkFunctionsForStatic()
{
    if (g_debuggerHandle) {
        return true;
    }
    g_debuggerHandle = Load("libarkinspector.so");
    if (g_debuggerHandle == nullptr) {
        return false;
    }
    g_initializeInspectorForStatic = reinterpret_cast<InitializeInspectorFunc>(
        ResolveSymbol(g_debuggerHandle, "InitializeInspector"));
    if (g_initializeInspectorForStatic == nullptr) {
        return false;
    }
    g_startDebuggerForStatic = reinterpret_cast<StartDebuggerFunc>(
        ResolveSymbol(g_debuggerHandle, "StartDebugger"));
    if (g_startDebuggerForStatic == nullptr) {
        return false;
    }
    g_stopDebuggerForStatic = reinterpret_cast<StopDebuggerFunc>(
        ResolveSymbol(g_debuggerHandle, "StopDebugger"));
    if (g_stopDebuggerForStatic == nullptr) {
        return false;
    }
    g_handleMessageForStatic = reinterpret_cast<HandleMessageFunc>(
        ResolveSymbol(g_debuggerHandle, "HandleMessage"));
    if (g_handleMessageForStatic == nullptr) {
        return false;
    }
    g_stopInspectorForStatic = reinterpret_cast<StopInspectorFunc>(
        ResolveSymbol(g_debuggerHandle, "StopInspector"));
    if (g_stopInspectorForStatic == nullptr) {
        return false;
    }
    g_waitForDebuggerForStatic = reinterpret_cast<WaitForDebuggerFunc>(
        ResolveSymbol(g_debuggerHandle, "WaitForDebugger"));
    if (g_waitForDebuggerForStatic == nullptr) {
        return false;
    }
    return true;
}

void OnMessageStatic(std::string&& message)
{
    g_handleMessageForStatic(std::move(message));
}

int StopDebuggerForStatic()
{
    g_stopInspectorForStatic();
    return 0;
}

bool StartDebuggerForStatic(std::shared_ptr<void> endpoint, bool breakOnStart)
{
    if (endpoint == nullptr) {
        LOGE("StartDebuggerForStatic Endpoint == nullptr");
    }
    g_initializeInspectorForStatic(endpoint, breakOnStart);
    return true;
}
void WaitForDebuggerForStatic()
{
    g_waitForDebuggerForStatic();
}

int StartDebuggerInitForStatic(uint32_t port, bool breakOnStart)
{
    if (!InitializeArkFunctionsForStatic()) {
        LOGE("StartDebuggerInitForStatic Error");
    }
    g_startDebuggerForStatic(port, breakOnStart);
    return 1;
}

int StopDebuggerInitForStatic()
{
    if (!InitializeArkFunctionsForStatic()) {
        LOGE("StopDebuggerInitForStatic Error");
    }
    g_stopDebuggerForStatic();
    return 1;
}
}
