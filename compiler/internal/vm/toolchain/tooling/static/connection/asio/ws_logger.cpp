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

#include "connection/asio/ws_logger.h"

#include "websocketpp/logger/levels.hpp"

#include "libarkbase/macros.h"
#include "libarkbase/utils/logger.h"

namespace ark::tooling::inspector {
void WsLogger::set_channels(Level channels)
{
    if (channels != 0) {
        dynamicChannels_ |= channels & staticChannels_;
    } else {
        dynamicChannels_ = 0;
    }
}

void WsLogger::write(Level channel, const std::string &string) const
{
    if (!dynamic_test(channel)) {
        return;
    }

    auto level = channel_log_level(channel);
    auto component = Logger::Component::DEBUGGER;

    if (!Logger::IsLoggingOnOrAbort(level, component)) {
        return;
    }

#ifndef NDEBUG
    if (Logger::IsMessageSuppressed(level, component)) {
        return;
    }
#endif

    Logger::Message message(level, component, false);
    auto &ms = message.GetStream();

    if (channelType_ == websocketpp::log::channel_type_hint::access) {
        ms << websocketpp::log::alevel::channel_name(channel) << ": ";
    }

    ms << string;
}

Logger::Level WsLogger::channel_log_level(Level channel) const
{
    if (channelType_ != websocketpp::log::channel_type_hint::error) {
        return Logger::Level::INFO;
    }

    switch (channel) {
        case websocketpp::log::elevel::devel:
            return Logger::Level::DEBUG;

        case websocketpp::log::elevel::library:
        case websocketpp::log::elevel::info:
            return Logger::Level::INFO;

        case websocketpp::log::elevel::warn:
            return Logger::Level::WARNING;

        case websocketpp::log::elevel::rerror:
            return Logger::Level::ERROR;

        case websocketpp::log::elevel::fatal:
            return Logger::Level::FATAL;

        default:
            UNREACHABLE();
    }
}
}  // namespace ark::tooling::inspector
