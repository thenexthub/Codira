/**
 * Copyright (c) 2022-2024 Huawei Device Co., Ltd.
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

#ifndef PANDA_TOOLING_INSPECTOR_CONNECTION_ASIO_SERVER_ENDPOINT_INL_H
#define PANDA_TOOLING_INSPECTOR_CONNECTION_ASIO_SERVER_ENDPOINT_INL_H

#ifndef CONFIG
#error Define CONFIG before including this header
#endif

#include "connection/asio/server_endpoint.h"

namespace ark::tooling::inspector {
template <>  // NOLINTNEXTLINE(misc-definitions-in-headers)
ServerEndpoint<CONFIG>::ServerEndpoint() noexcept
{
    endpoint_.set_message_handler(
        [this](auto /* hdl */, auto message) { this->HandleMessage(message->get_payload()); });

    endpoint_.set_validate_handler([this](auto hdl) {
        onValidate_();

        if (Pin(hdl)) {
            return true;
        }

        endpoint_.get_con_from_hdl(hdl)->set_body("Another debug session is in progress");
        return false;
    });

    endpoint_.set_open_handler([this](auto) { onOpen_(); });
    endpoint_.set_fail_handler([this](auto hdl) {
        onFail_();
        Unpin(hdl);
    });

    endpoint_.set_close_handler([this](auto hdl) { Unpin(hdl); });
}

template <>  // NOLINTNEXTLINE(misc-definitions-in-headers)
bool ServerEndpoint<CONFIG>::Close()
{
    if (auto connection = GetPinnedConnection()) {
        std::error_code ec;
        connection->close(websocketpp::close::status::going_away, "", ec);
        if (ec) {
            LOG(ERROR, DEBUGGER) << "Error closing websocket connection: " << ec.message();
            return false;
        }
    }
    return true;
}
}  // namespace ark::tooling::inspector

#endif  // PANDA_TOOLING_INSPECTOR_CONNECTION_ASIO_SERVER_ENDPOINT_INL_H
