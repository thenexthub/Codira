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

#include "ws_server.h"
#include "init_static.h"

#include <unistd.h>

#include "common/log_wrapper.h"
#include "websocket/server/websocket_server.h"

namespace OHOS::ArkCompiler::Toolchain {
std::shared_mutex g_mutex;

// defined in .cpp file for WebSocketServer forward declaration
WsServer::WsServer(const DebugInfo& debugInfo, const std::function<void(std::string&&)>& onMessage)
    : debugInfo_(debugInfo), wsOnMessage_(onMessage)
{}

WsServer::~WsServer() = default;

void WsServer::RunServer(bool isHybrid)
{
    {
        std::lock_guard<std::mutex> lock(wsMutex_);
        if (terminateExecution_) {
            LOGE("WsServer has been terminated unexpectedly");
            return;
        }
        webSocket_ = std::make_unique<WebSocketServer>();
#if !defined(OHOS_PLATFORM)
        LOGI("WsSever Runsever: Init tcp websocket %{public}d", debugInfo_.port);
        if (!webSocket_->InitTcpWebSocket(debugInfo_.port)) {
            return;
        }
#else
        int runSeverInOldProcess = -2;
        if (debugInfo_.socketfd == runSeverInOldProcess) {
            int appPid = getprocpid();
            std::string pidStr = std::to_string(appPid);
            std::string instanceIdStr("");

            if (debugInfo_.instanceId != 0) {
                instanceIdStr = std::to_string(debugInfo_.instanceId);
            }
            std::string sockName = pidStr + instanceIdStr + debugInfo_.componentName;
            LOGI("WsServer RunServer fport localabstract: %{public}d%{public}s%{public}s",
                appPid, instanceIdStr.c_str(), debugInfo_.componentName.c_str());
            if (!webSocket_->InitUnixWebSocket(sockName)) {
                return;
            }
        } else {
            LOGI("WsServer RunServer fport ark: %{public}d", debugInfo_.socketfd);
            if (!webSocket_->InitUnixWebSocket(debugInfo_.socketfd)) {
                return;
            }
        }
        if (isHybrid) {
            StartDebuggerForStatic(webSocket_, true);
        }
#endif
    }
    ContinueRunserver();
}
void WsServer::ContinueRunserver()
{
    while (!terminateExecution_) {
#if !defined(OHOS_PLATFORM)
        if (!webSocket_->AcceptNewConnection()) {
            return;
        }
#else
        int runSeverInOldProcess = -2;
        if (debugInfo_.socketfd == runSeverInOldProcess) {
            if (!webSocket_->AcceptNewConnection()) {
                return;
            }
        } else {
            if (!webSocket_->ConnectUnixWebSocketBySocketpair()) {
                return;
            }
        }
#endif
        while (webSocket_->IsConnected()) {
            std::string message = webSocket_->Decode();
            if (!message.empty() && webSocket_->IsDecodeDisconnectMsg(message)) {
                LOGI("WsServer receiving disconnect msg: %{public}s", message.c_str());
                NotifyDisconnectEvent();
            } else if (!message.empty()) {
                LOGI("WsServer OnMessage: %{public}s", message.c_str());
                wsOnMessage_(std::move(message));
            }
        }
    }
}

void WsServer::StopServer()
{
    LOGI("WsServer StopServer");
    {
        std::lock_guard<std::mutex> lock(wsMutex_);
        terminateExecution_ = true;
        if (webSocket_ != nullptr) {
            webSocket_->Close();
        }
    }
    pthread_join(tid_, nullptr);
    if (webSocket_ != nullptr) {
        webSocket_.reset();
    }
}

void WsServer::SendReply(const std::string& message) const
{
    std::unique_lock<std::shared_mutex> lock(g_mutex);
    if (webSocket_ == nullptr) {
        LOGE("WsServer SendReply websocket has been closed unexpectedly");
        return;
    }
    LOGI("WsServer SendReply: %{public}s", message.c_str());
    if (!webSocket_->SendReply(message)) {
        LOGE("WsServer SendReply send fail");
        NotifyDisconnectEvent();
    }
}

void WsServer::NotifyDisconnectEvent() const
{
    std::string message = "{\"id\":0, \"method\":\"Debugger.clientDisconnect\", \"params\":{}}";
    wsOnMessage_(std::move(message));
}
} // namespace OHOS::ArkCompiler::Toolchain
