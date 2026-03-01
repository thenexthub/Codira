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

#ifndef PANDA_TOOLING_INSPECTOR_CONNECTION_SERVER_ENDPOINT_BASE_H
#define PANDA_TOOLING_INSPECTOR_CONNECTION_SERVER_ENDPOINT_BASE_H

#include <functional>

#include "connection/endpoint_base.h"
#include "connection/server.h"

namespace ark::tooling::inspector {
// Base class for server endpoints implementations.
// Provides callbacks to be executed during client connections handling.
// NOLINTNEXTLINE(fuchsia-multiple-inheritance)
class ServerEndpointBase : public EndpointBase, public Server {
public:
    using MethodResponse = Expected<std::unique_ptr<JsonSerializable>, JRPCError>;
    using Handler = std::function<MethodResponse(const std::string &, const JsonObject &params)>;

public:
    void OnValidate(std::function<void()> &&handler) override
    {
        onValidate_ = std::move(handler);
    }

    void OnOpen(std::function<void()> &&handler) override
    {
        onOpen_ = std::move(handler);
    }

    void OnFail(std::function<void()> &&handler) override
    {
        onFail_ = std::move(handler);
    }

    using Server::Call;
    void Call(const std::string &sessionId, const char *method,
              std::function<void(JsonObjectBuilder &)> &&params) override;

    void OnCallImpl(const char *method, Handler &&handler) override;

protected:
    std::function<void()> onValidate_ = []() {};  // NOLINT(misc-non-private-member-variables-in-classes)
    std::function<void()> onOpen_ = []() {};      // NOLINT(misc-non-private-member-variables-in-classes)
    std::function<void()> onFail_ = []() {};      // NOLINT(misc-non-private-member-variables-in-classes)
};
}  // namespace ark::tooling::inspector

#endif  // PANDA_TOOLING_INSPECTOR_CONNECTION_SERVER_ENDPOINT_BASE_H
