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

#include "connect_server.h"
#include <mutex>
#include <unistd.h>
#include "common/log_wrapper.h"
#include "websocket/server/websocket_server.h"

namespace OHOS::ArkCompiler::Toolchain {
std::shared_mutex g_sendMutex;

// defined in .cpp file for WebSocketServer forward declaration
ConnectServer::ConnectServer(int socketfd, std::function<void(std::string&&)> onMessage)
    : socketfd_(socketfd), wsOnMessage_(std::move(onMessage))
{}

ConnectServer::ConnectServer(const std::string& bundleName, std::function<void(std::string&&)> onMessage)
    : bundleName_(bundleName), wsOnMessage_(std::move(onMessage))
{}

ConnectServer::~ConnectServer() = default;

void ConnectServer::RunServer()
{
    terminateExecution_ = false;
    webSocket_ = std::make_unique<WebSocketServer>();
    tid_ = pthread_self();
#if defined(OHOS_PLATFORM)
    int runSeverInOldProcess = -2; // run sever in old process.
    int appPid = getprocpid();
    std::string pidStr = std::to_string(appPid);
    std::string sockName = pidStr + bundleName_;
    if (socketfd_ == runSeverInOldProcess) {
        if (!webSocket_->InitUnixWebSocket(sockName)) {
            return;
        }
    } else {
        if (!webSocket_->InitUnixWebSocket(socketfd_)) {
            return;
        }
    }
#endif
    while (!terminateExecution_) {
#if defined(OHOS_PLATFORM)
        if (socketfd_ == runSeverInOldProcess) {
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
            if (!message.empty()) {
                wsOnMessage_(std::move(message));
            }
        }
    }
}

void ConnectServer::StopServer()
{
    LOGI("ConnectServer StopServer");
    terminateExecution_ = true;
    if (webSocket_ != nullptr) {
        webSocket_->Close();
        pthread_join(tid_, nullptr);
        webSocket_.reset();
    }
}

void ConnectServer::SendMessage(const std::string& message) const
{
    std::unique_lock<std::shared_mutex> lock(g_sendMutex);
    if (webSocket_ == nullptr) {
        LOGE("ConnectServer SendReply websocket has been closed unexpectedly");
        return;
    }
    LOGI("ConnectServer SendReply: %{public}s", message.c_str());
    webSocket_->SendReply(message);
}
} // namespace OHOS::ArkCompiler::Toolchain
