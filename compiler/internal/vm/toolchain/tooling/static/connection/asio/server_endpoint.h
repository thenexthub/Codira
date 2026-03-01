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

#ifndef PANDA_TOOLING_INSPECTOR_CONNECTION_ASIO_SERVER_ENDPOINT_H
#define PANDA_TOOLING_INSPECTOR_CONNECTION_ASIO_SERVER_ENDPOINT_H

#include "websocketpp/common/connection_hdl.hpp"
#include "websocketpp/frame.hpp"
#include "websocketpp/server.hpp"

#include "connection/server_endpoint_base.h"

namespace ark::tooling::inspector {
// Server endpoint based on websocketpp implementation.
// NOLINTNEXTLINE(fuchsia-multiple-inheritance)
template <typename Config>
class ServerEndpoint : public ServerEndpointBase {
protected:
    using Endpoint = websocketpp::server<Config>;
    using ConnectionPtr = typename Endpoint::connection_ptr;

public:
    ServerEndpoint() noexcept;

protected:
    ConnectionPtr GetPinnedConnection()
    {
        return connection_;
    }

    bool Pin(const websocketpp::connection_hdl &hdl)
    {
        ConnectionPtr expected;
        // set new connection only if there is no already existing one
        return std::atomic_compare_exchange_strong(&connection_, &expected, endpoint_.get_con_from_hdl(hdl));
    }

    void Unpin(const websocketpp::connection_hdl &hdl)
    {
        auto expected = endpoint_.get_con_from_hdl(hdl);
        // expect that the given connection is the one already stored
        std::atomic_compare_exchange_strong(&connection_, &expected, {});
    }

    bool Close();

    Endpoint endpoint_;  // NOLINT(misc-non-private-member-variables-in-classes)

private:
    void SendMessage(const std::string &message) override
    {
        if (auto connection = GetPinnedConnection()) {
            connection->send(message, websocketpp::frame::opcode::text);
        } else {
            LOG(INFO, DEBUGGER) << "Did not send message: " << message;
        }
    }

    ConnectionPtr connection_;
};
}  // namespace ark::tooling::inspector

#endif  // PANDA_TOOLING_INSPECTOR_CONNECTION_ASIO_SERVER_ENDPOINT_H
