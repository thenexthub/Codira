/**
 * Copyright (c) 2022-2025 Huawei Device Co., Ltd.
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

#include <cstdint>
#include <optional>

#ifdef PANDA_TOOLING_ASIO
#include "connection/asio/asio_server.h"
#else
#include "connection/ohos_ws/ohos_ws_server.h"
#endif  // PANDA_TOOLING_ASIO

#include "inspector.h"

namespace ark::tooling {
class DebugInterface;
}  // namespace ark::tooling

#ifdef PANDA_TOOLING_ASIO
using InspectorWebSocketServer = ark::tooling::inspector::AsioServer;
#else
using InspectorWebSocketServer = ark::tooling::inspector::OhosWsServer;
#endif  // PANDA_TOOLING_ASIO

// NOLINTNEXTLINE(fuchsia-statically-constructed-objects)
static ark::Runtime::DebugSessionHandle g_debugSession;

// NOLINTNEXTLINE(fuchsia-statically-constructed-objects)
static InspectorWebSocketServer g_server;

// NOLINTNEXTLINE(fuchsia-statically-constructed-objects)
static std::optional<ark::tooling::inspector::Inspector> g_inspector;

extern "C" int StartDebugger(uint32_t port, bool breakOnStart)
{
    if (g_inspector) {
        LOG(ERROR, DEBUGGER) << "Debugger has already been started";
        return 1;
    }

    if (!g_server.Start(port)) {
        return 1;
    }

    g_debugSession = ark::Runtime::GetCurrent()->StartDebugSession();
    g_inspector.emplace(g_server, g_debugSession->GetDebugger(), breakOnStart);
    g_inspector->Run();
    return 0;
}

extern "C" int StopDebugger()
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

extern "C" bool StartDebuggerForSocketpair([[maybe_unused]] int socketfd, bool breakOnStart)
{
    if (g_inspector) {
        g_server.Stop();
        g_inspector->Stop();
    }

#ifndef PANDA_TOOLING_ASIO
    if (!g_server.StartForSocketpair(socketfd)) {
        return false;
    }
#endif  // PANDA_TOOLING_ASIO

    if (!g_inspector) {
        g_debugSession = ark::Runtime::GetCurrent()->StartDebugSession();
        g_inspector.emplace(g_server, g_debugSession->GetDebugger(), breakOnStart);
    }

    g_inspector->Run();
    return true;
}

extern "C" void WaitForDebugger()
{
    if (!g_inspector) {
        LOG(ERROR, DEBUGGER) << "Debugger has not been started";
        return;
    }

    g_inspector->WaitForDebugger();
}
