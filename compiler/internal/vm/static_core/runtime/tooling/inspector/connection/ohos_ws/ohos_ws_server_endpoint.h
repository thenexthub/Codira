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

#ifndef PANDA_TOOLING_INSPECTOR_CONNECTION_OHOS_WS_OHOS_WS_SERVER_ENDPOINT_H
#define PANDA_TOOLING_INSPECTOR_CONNECTION_OHOS_WS_OHOS_WS_SERVER_ENDPOINT_H

#include "server/websocket_server.h"

#include "connection/server_endpoint_base.h"

namespace ark::tooling::inspector {
// Server endpoint based on OHOS websocket implementation
// NOLINTNEXTLINE(fuchsia-multiple-inheritance)
class OhosWsServerEndpoint : public ServerEndpointBase {
protected:
    using Endpoint = OHOS::ArkCompiler::Toolchain::WebSocketServer;

public:
    OhosWsServerEndpoint() noexcept;

protected:
    Endpoint endpoint_;  // NOLINT(misc-non-private-member-variables-in-classes)

private:
    void SendMessage(const std::string &message) override
    {
        auto wasSent = endpoint_.SendReply(message);
        if (!wasSent) {
            LOG(INFO, DEBUGGER) << "Did not send message: " << message;
        }
    }
};
}  // namespace ark::tooling::inspector

#endif  // PANDA_TOOLING_INSPECTOR_CONNECTION_OHOS_WS_OHOS_WS_SERVER_ENDPOINT_H
