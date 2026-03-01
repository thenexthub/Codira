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

#ifndef PANDA_TOOLING_INSPECTOR_CONNECTION_ASIO_ASIO_CONFIG_H
#define PANDA_TOOLING_INSPECTOR_CONNECTION_ASIO_ASIO_CONFIG_H

#include "websocketpp/config/asio_no_tls.hpp"
#include "websocketpp/logger/levels.hpp"
#include "websocketpp/transport/asio/endpoint.hpp"

#include "connection/asio/ws_logger.h"

namespace ark::tooling::inspector {
// NOLINTBEGIN(readability-identifier-naming)
struct AsioConfig : websocketpp::config::asio {
    static const websocketpp::log::level alog_level = websocketpp::log::alevel::access_core;

    using alog_type = WsLogger;
    using elog_type = WsLogger;

    struct transport_config : websocketpp::config::asio::transport_config {
        using alog_type = AsioConfig::alog_type;
        using elog_type = AsioConfig::elog_type;
    };

    using transport_type = websocketpp::transport::asio::endpoint<transport_config>;
};
// NOLINTEND(readability-identifier-naming)
}  // namespace ark::tooling::inspector

#endif  // PANDA_TOOLING_INSPECTOR_CONNECTION_ASIO_ASIO_CONFIG_H
