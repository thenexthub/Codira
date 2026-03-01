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

#ifndef JS_NATIVE_API_V8_INSPECTOR_H
#define JS_NATIVE_API_V8_INSPECTOR_H

#include <cstddef>
#include <memory>

#include "js_native_api_v8.h"
#include "jsvm_host_port.h"
#include "jsvm_inspector_agent.h"
#include "v8.h"

#ifndef ENABLE_INSPECTOR
#error("This header can only be used when inspector is enabled")
#endif

namespace v8_inspector {
class StringView;
} // namespace v8_inspector

namespace jsvm {
namespace inspector {
class InspectorSessionDelegate;
class InspectorSession;

class InspectorSession {
public:
    virtual ~InspectorSession() = default;
    virtual void Dispatch(const v8_inspector::StringView& message) = 0;
};

class InspectorSessionDelegate {
public:
    virtual ~InspectorSessionDelegate() = default;
    virtual void SendMessageToFrontend(const v8_inspector::StringView& message) = 0;
};
} // namespace inspector
} // namespace jsvm

namespace v8impl {

using jsvm::ExclusiveAccess;
using jsvm::HostPort;
using jsvm::inspector::InspectorSession;
using jsvm::inspector::InspectorSessionDelegate;

class IsolateData;
using Environment = JSVM_Env__;

class InspectorClient;
class InspectorIo;

struct ContextInfo {
    explicit ContextInfo(const std::string& name) : name(name) {}
    const std::string name;
    std::string origin;
    bool isDefault = false;
};

class Agent : public jsvm::InspectorAgent {
public:
    explicit Agent(Environment* env);
    ~Agent();

public:
    bool Start(const std::string& pathParam, const std::string& hostName, int port, int pid = -1) override
    {
        auto hostPort = std::make_shared<jsvm::ExclusiveAccess<jsvm::HostPort>>(hostName, port, pid);
        return Start(pathParam, hostPort, true, false);
    }

    bool Start(const std::string& pathParam, int pid) override;

    // Stop and destroy io_
    void Stop() override;

    // Returns true if the inspector is actually in use.
    bool IsActive() override;

    // Blocks till frontend connects and sends "runIfWaitingForDebugger"
    void WaitForConnect() override;

    // Blocks till all the sessions with "WaitForDisconnectOnShutdown" disconnect
    void WaitForDisconnect() override;

    void PauseOnNextJavascriptStatement(const std::string& reason) override;

public:
    // Called to create inspector sessions that can be used from the same thread.
    // The inspector responds by using the delegate to send messages back.
    std::unique_ptr<InspectorSession> Connect(std::unique_ptr<InspectorSessionDelegate> delegate, bool preventShutdown);

    // Can only be called from the main thread.
    bool StartIoThread();

    std::shared_ptr<ExclusiveAccess<HostPort>> GetHostPort()
    {
        return hostPort;
    }

    inline Environment* env() const
    {
        return parentEnv;
    }

private:
    // Create client_, may create io if option enabled
    bool Start(const std::string& pathParam,
               std::shared_ptr<ExclusiveAccess<HostPort>> hostPortParam,
               bool isMain,
               bool waitForConnect);

    Environment* parentEnv;
    // Encapsulates majority of the Inspector functionality
    std::shared_ptr<InspectorClient> client;
    // Interface for transports, e.g. WebSocket server
    std::unique_ptr<InspectorIo> io;
    std::string path;

    std::shared_ptr<ExclusiveAccess<HostPort>> hostPort;
};

} // namespace v8impl

#endif