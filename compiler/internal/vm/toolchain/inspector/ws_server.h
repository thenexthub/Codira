/*
 * Copyright (c) 2022-2025 Huawei Device Co., Ltd.
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#ifndef ARKCOMPILER_TOOLCHAIN_INSPECTOR_WS_SERVER_H
#define ARKCOMPILER_TOOLCHAIN_INSPECTOR_WS_SERVER_H

#include <atomic>
#include <functional>
#include <memory>
#include <mutex>
#ifdef WINDOWS_PLATFORM
#include <pthread.h>
#endif
#include <string>

namespace OHOS::ArkCompiler::Toolchain {
class WebSocketServer;

struct DebugInfo {
    int socketfd {-2};
    std::string componentName {};
    int32_t instanceId {0};
    int port {-1};
};

class WsServer {
public:
    WsServer(const DebugInfo& debugInfo, const std::function<void(std::string&&)>& onMessage);
    ~WsServer();
    void RunServer(bool isHybrid = false);
    void ContinueRunserver();
    void StopServer();
    void SendReply(const std::string& message) const;
    void NotifyDisconnectEvent() const;

    pthread_t tid_ {0};

private:

    std::atomic<bool> terminateExecution_ { false };
    std::mutex wsMutex_;
    DebugInfo debugInfo_ {};
    std::function<void(std::string&&)> wsOnMessage_ {};
    std::shared_ptr<WebSocketServer> webSocket_ { nullptr };
};
} // namespace OHOS::ArkCompiler::Toolchain

#endif // ARKCOMPILER_TOOLCHAIN_INSPECTOR_WS_SERVER_H
