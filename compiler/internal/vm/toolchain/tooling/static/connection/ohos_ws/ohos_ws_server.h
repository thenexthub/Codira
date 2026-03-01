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

#ifndef PANDA_TOOLING_INSPECTOR_CONNECTION_OHOS_OHOS_WS_SERVER_H
#define PANDA_TOOLING_INSPECTOR_CONNECTION_OHOS_OHOS_WS_SERVER_H

#include "connection/ohos_ws/ohos_ws_server_endpoint.h"

namespace ark::tooling::inspector {
// JSON-RPC server processing only one connection at time.
// Based on OHOS websocket protocol implementation.
// NOLINTNEXTLINE(fuchsia-multiple-inheritance)
class OhosWsServer final : public OhosWsServerEndpoint {
public:
    bool ParseMessage(const std::string& msg) override;

    // Creates socket and starts listening
    bool Start(uint32_t port = 0);
    bool Stop();
    bool StartForSocketpair(int socketfd);
    void InitEndPoint(std::shared_ptr<void> endPoint);

private:
    bool socketpairMode_ {false};

    bool AcceptNewWsConnection();
};
}  // namespace ark::tooling::inspector

#endif  // PANDA_TOOLING_INSPECTOR_CONNECTION_OHOS_OHOS_WS_SERVER_H
