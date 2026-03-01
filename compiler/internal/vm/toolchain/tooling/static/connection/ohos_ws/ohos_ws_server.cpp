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

#include "connection/ohos_ws/ohos_ws_server.h"

#include <string_view>

#include "libarkbase/os/thread.h"
#include "libarkbase/utils/logger.h"

#include "inspector.h"
namespace ark::tooling::inspector {
[[maybe_unused]] static constexpr std::string_view G_ARK_TS_INSPECTOR_NAME = "ArkEtsDebugger";

bool OhosWsServer::ParseMessage(const std::string& msg)
{
    if (endpoint_ == nullptr) {
        return false;
    }
    if (!endpoint_->IsConnected() && !AcceptNewWsConnection()) {
        LOG(WARNING, DEBUGGER) << "Inspector server is unable to establish a new connection, exiting";
        return false;
    }

    HandleMessage(msg);
    return true;
}

bool OhosWsServer::Start([[maybe_unused]] uint32_t port)
{
    return true;
}

void OhosWsServer::InitEndPoint(std::shared_ptr<void> endPoint)
{
    if (endPoint == nullptr) {
        LOG(ERROR, DEBUGGER) << "OhosWsServer::InitEndPoint endPoint is nullptr";
        return;
    }

    endpoint_ = std::static_pointer_cast<OHOS::ArkCompiler::Toolchain::WebSocketServer>(endPoint);
}

bool OhosWsServer::Stop()
{
    // Stop event loop before closing endpoint server.
    Kill();
    socketpairMode_ = false;
    return true;
}

bool OhosWsServer::StartForSocketpair(int socketfd)
{
    bool succeeded = endpoint_->InitUnixWebSocket(socketfd);
    if (succeeded) {
        LOG(INFO, DEBUGGER) << "Inspector server listening on " << socketfd;
        socketpairMode_ = true;
        return true;
    }

    LOG(ERROR, DEBUGGER) << "Failed to bind Inspector server on " << socketfd;
    return false;
}

bool OhosWsServer::AcceptNewWsConnection()
{
    if (socketpairMode_) {
        return endpoint_->ConnectUnixWebSocketBySocketpair();
    }
    return endpoint_->AcceptNewConnection();
}

}  // namespace ark::tooling::inspector
