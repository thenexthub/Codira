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

#ifndef PANDA_TOOLING_INSPECTOR_CONNECTION_ASIO_WS_LOGGER_H
#define PANDA_TOOLING_INSPECTOR_CONNECTION_ASIO_WS_LOGGER_H

#include <string>

#include "websocketpp/logger/levels.hpp"

#include "libarkbase/utils/logger.h"

namespace ark::tooling::inspector {
class WsLogger {
    using ChannelType = websocketpp::log::channel_type_hint::value;
    using Level = websocketpp::log::level;

public:
    WsLogger(Level staticChannels, ChannelType channelType) : channelType_(channelType), staticChannels_(staticChannels)
    {
    }

    // NOLINTNEXTLINE(readability-identifier-naming)
    void set_channels(Level channels);
    // NOLINTNEXTLINE(readability-identifier-naming)
    bool static_test(Level channel) const
    {
        return (channel & staticChannels_) == channel;
    }
    // NOLINTNEXTLINE(readability-identifier-naming)
    bool dynamic_test(Level channel) const
    {
        return (channel & dynamicChannels_) == channel;
    }
    // NOLINTNEXTLINE(readability-identifier-naming)
    void write(Level channel, const std::string &string) const;

private:
    // NOLINTNEXTLINE(readability-identifier-naming)
    Logger::Level channel_log_level(Level channel) const;

    const ChannelType channelType_;
    const Level staticChannels_;
    Level dynamicChannels_ {0};
};
}  // namespace ark::tooling::inspector

#endif  // PANDA_TOOLING_INSPECTOR_CONNECTION_ASIO_WS_LOGGER_H
