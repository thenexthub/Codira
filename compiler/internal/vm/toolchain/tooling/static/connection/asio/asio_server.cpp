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

#include "connection/asio/asio_server.h"

#include <memory>
#include <sstream>
#include <system_error>

#include "websocketpp/close.hpp"
#include "websocketpp/http/constants.hpp"
#include "websocketpp/uri.hpp"

#include "libarkbase/utils/json_builder.h"
#include "libarkbase/utils/logger.h"

#include "connection/asio/asio_config.h"

#define CONFIG AsioConfig  // NOLINT(cppcoreguidelines-macro-usage)
#include "server_endpoint-inl.h"
#undef CONFIG

namespace ark::tooling::inspector {
template <typename ConnectionPtr>
static void HandleHttpRequest(ConnectionPtr conn)
{
    static constexpr std::string_view GET = "GET";
    static constexpr std::string_view JSON_URI = "/json";
    static constexpr std::string_view JSON_LIST_URI = "/json/list";
    static constexpr std::string_view JSON_VERSION_URI = "/json/version";

    const auto &req = conn->get_request();
    const auto &uri = req.get_uri();
    if (req.get_method() != GET) {
        CloseConnection(conn);
        return;
    }

    if (uri == JSON_URI || uri == JSON_LIST_URI) {
        HandleJsonList(conn);
    } else if (uri == JSON_VERSION_URI) {
        HandleJsonVersion(conn);
    } else {
        CloseConnection(conn);
        return;
    }
    conn->append_header("Content-Type", "application/json; charset=UTF-8");
    conn->append_header("Cache-Control", "no-cache");
    conn->set_status(websocketpp::http::status_code::value::ok);
}

template <typename ConnectionPtr>
static void HandleJsonVersion(ConnectionPtr conn)
{
    JsonObjectBuilder builder;
    builder.AddProperty("browser", "ArkTS");
    builder.AddProperty("protocol-version", "1.1");
    AddWebSocketDebuggerUrl(conn, builder);
    conn->set_body(std::move(builder).Build());
}

template <typename ConnectionPtr>
static void HandleJsonList(ConnectionPtr conn)
{
    auto buildJson = [conn](JsonObjectBuilder &builder) {
        builder.AddProperty("description", "ArkTS");
        // Empty "id" corresponds to constructed URLs
        builder.AddProperty("id", "");
        builder.AddProperty("type", "node");
        // Do not add "url" fields as it dummy value might confuse some clients
        AddWebSocketDebuggerUrl(conn, builder);
        std::stringstream ss;
        // CC-OFFNXT(WordsTool.74) fixed protocol url
        ss << "devtools://devtools/bundled/devtools_app.html?ws=" << conn->get_host() << ':'
           << static_cast<int>(conn->get_port());
        builder.AddProperty("devToolsFrontendUrl", ss.str());
    };
    JsonArrayBuilder arrayBuilder;
    arrayBuilder.Add(std::move(buildJson));
    conn->set_body(std::move(arrayBuilder).Build());
}

template <typename ConnectionPtr>
static void CloseConnection(ConnectionPtr conn)
{
    std::error_code ec;
    conn->close(websocketpp::close::status::protocol_error, "", ec);
    if (ec) {
        LOG(WARNING, DEBUGGER) << "Failed to close invalid HTTP connection";
    }
}

template <typename ConnectionPtr>
static void AddWebSocketDebuggerUrl(ConnectionPtr conn, JsonObjectBuilder &builder)
{
    std::stringstream ss;
    ss << "ws://" << conn->get_host() << ':' << conn->get_port();
    builder.AddProperty("webSocketDebuggerUrl", ss.str());
}

bool AsioServer::Initialize()
{
    if (initialized_) {
        return true;
    }

    // Do JSON handshake, as it is expected by some debugger clients
    endpoint_.set_http_handler([this](auto hdl) { HandleHttpRequest(endpoint_.get_con_from_hdl(hdl)); });

    std::error_code ec;
    endpoint_.init_asio(ec);
    if (ec) {
        LOG(ERROR, DEBUGGER) << "Failed to initialize endpoint";
        return false;
    }

    endpoint_.set_reuse_addr(true);
    initialized_ = true;
    return true;
}

websocketpp::uri_ptr AsioServer::Start(uint32_t port)
{
    if (!Initialize()) {
        return nullptr;
    }

    std::error_code ec;

    endpoint_.listen(port, ec);
    if (ec) {
        LOG(ERROR, DEBUGGER) << "Failed to bind Inspector server on port " << port;
        return nullptr;
    }

    endpoint_.start_accept(ec);

    if (!ec) {
        auto ep = endpoint_.get_local_endpoint(ec);

        if (!ec) {
            LOG(INFO, DEBUGGER) << "Inspector server listening on " << ep;
            return std::make_shared<websocketpp::uri>(false, ep.address().to_string(), ep.port(), "/");
        }

        LOG(ERROR, DEBUGGER) << "Failed to get the TCP endpoint";
    } else {
        LOG(ERROR, DEBUGGER) << "Failed to start Inspector connection acceptance loop";
    }

    endpoint_.stop_listening(ec);
    return nullptr;
}

bool AsioServer::Stop()
{
    if (!Initialize()) {
        return false;
    }

    // Stop accepting new connections.
    Kill();
    // Close the current connection.
    Close();

    std::error_code ec;
    endpoint_.stop_listening(ec);
    return !ec;
}
}  // namespace ark::tooling::inspector
