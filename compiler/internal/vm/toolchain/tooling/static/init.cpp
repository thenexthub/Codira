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

#include <cstdint>
#include <dlfcn.h>
#include <optional>

#ifdef PANDA_TOOLING_ASIO
#include "connection/asio/asio_server.h"
#else
#include "connection/ohos_ws/ohos_ws_server.h"
#endif  // PANDA_TOOLING_ASIO

#include "init.h"
#include "inspector.h"

#ifdef PANDA_TOOLING_ASIO
using InspectorWebSocketServer = ark::tooling::inspector::AsioServer;
#else
using InspectorWebSocketServer = ark::tooling::inspector::OhosWsServer;
#endif  // PANDA_TOOLING_ASIO


// NOLINTNEXTLINE(fuchsia-statically-constructed-objects)
std::optional<ark::tooling::inspector::Inspector> g_inspector;

// NOLINTNEXTLINE(fuchsia-statically-constructed-objects)
static ark::Runtime::DebugSessionHandle g_debugSession;

// NOLINTNEXTLINE(fuchsia-statically-constructed-objects)
static InspectorWebSocketServer g_server;

int StartDebugger(uint32_t port, bool breakOnStart)
{
    if (g_inspector) {
        LOG(ERROR, DEBUGGER) << "Debugger has already been started";
        return 1;
    }
    if (!g_server.Start(port)) {
        LOG(ERROR, DEBUGGER) << "call g_server.Start failed";
        return 1;
    }
    g_debugSession = ark::Runtime::GetCurrent()->StartDebugSession();
    g_inspector.emplace(g_server, g_debugSession->GetDebugger(), breakOnStart);
    return 0;
}

void InitializeInspector(std::shared_ptr<void> endPoint, bool breakOnStart)
{
    if (g_inspector) {
        g_server.Stop();
    }
    g_server.InitEndPoint(endPoint);
    g_debugSession = ark::Runtime::GetCurrent()->StartDebugSession();
    if (!g_inspector) {
        LOG(INFO, DEBUGGER) << "InitializeInspector started";
        g_debugSession = ark::Runtime::GetCurrent()->StartDebugSession();
        g_inspector.emplace(g_server, g_debugSession->GetDebugger(), breakOnStart);
    }
}

void StopInspector()
{
    if (!g_inspector) {
        LOG(ERROR, DEBUGGER) << "Debugger has not been started";
        return;
    }

    if (!g_server.Stop()) {
        return;
    }
    g_inspector.reset();
    g_debugSession.reset();
}

int StopDebugger()
{
    if (!g_inspector) {
        LOG(ERROR, DEBUGGER) << "Debugger has not been started";
        return 1;
    }

    if (!g_server.Stop()) {
        return 1;
    }
    g_inspector.reset();
    g_debugSession.reset();
    return 0;
}

void WaitForDebugger()
{
    if (!g_inspector) {
        LOG(ERROR, DEBUGGER) << "Debugger has not been started";
        return;
    }

    g_inspector->WaitForDebugger();
}

void HandleMessage(std::string &&msg)
{
    LOG(ERROR, DEBUGGER) << "Static OnMessage start";
    g_inspector->Run(msg);
}