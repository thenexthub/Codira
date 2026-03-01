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

#ifndef PANDA_TOOLING_INSPECTOR_CONNECTION_ENDPOINT_BASE_H
#define PANDA_TOOLING_INSPECTOR_CONNECTION_ENDPOINT_BASE_H

#include <functional>
#include <optional>
#include <string>
#include <type_traits>
#include <unordered_map>
#include <utility>

#include "libarkbase/utils/json_builder.h"
#include "libarkbase/utils/logger.h"

#include "json_serialization/jrpc_error.h"

namespace ark {
class JsonObject;
}  // namespace ark

namespace ark::tooling::inspector {
enum class InspectorErrorCode {
    PARSE_ERROR = -32700,
    INTERNAL_ERROR = -32603,
    INVALID_PARAMS = -32602,
    METHOD_NOT_FOUND = -32601,
    INVALID_REQUEST = -32600,
    SESSION_NOT_FOUND = -32001,
    SERVER_ERROR = -32000,
};

/// @brief Base class implementation of JSON-RPC endpoint handling the Inspector protocol.
class EndpointBase {
public:
    using Id = double;

private:
    using MethodHandler = std::function<void(const std::string &, std::optional<Id>, const JsonObject &)>;
    using ResultHandler = std::function<void(const JsonObject &)>;

public:
    void OnCall(const char *method, MethodHandler &&handler);

protected:
    void HandleMessage(const std::string &message);

    void OnResult(Id id, ResultHandler &&handler)
    {
        resultHandlers_[id] = std::move(handler);
    }

    void Call(
        const std::string &sessionId, std::optional<Id> id, const char *method,
        std::function<void(JsonObjectBuilder &)> &&params = [](JsonObjectBuilder & /* builder */) {});

    template <typename Result>
    void Reply(const std::string &sessionId, Id id, Result &&result)
    {
        Send([&sessionId, id, &result](JsonObjectBuilder &reply) {
            reply.AddProperty("id", id);
            reply.AddProperty("result", std::forward<Result>(result));

            if (!sessionId.empty()) {
                reply.AddProperty("sessionId", sessionId);
            }
        });
    }

    void ReplyError(const std::string &sessionId, Id id, JRPCError &&error)
    {
        Send([&sessionId, id, &error](JsonObjectBuilder &reply) {
            reply.AddProperty("id", id);
            reply.AddProperty("error", std::move(error));

            if (!sessionId.empty()) {
                reply.AddProperty("sessionId", sessionId);
            }
        });
    }

private:
    /// @brief Sends JSON message.
    virtual void SendMessage(const std::string &message) = 0;

    /// @brief Sends JSON message created with the provided build function.
    template <typename BuildFunction>
    void Send(BuildFunction &&build)
    {
        JsonObjectBuilder builder;
        build(builder);
        auto message = std::move(builder).Build();
        LOG(INFO, DEBUGGER) << "Sending " << message;
        SendMessage(message);
    }

    void HandleUnsupportedMethod(std::optional<double> optId, const std::string &method);

    os::memory::Mutex methodHandlersMutex_;
    std::unordered_map<std::string, MethodHandler> methodHandlers_ GUARDED_BY(methodHandlersMutex_);
    std::unordered_map<Id, ResultHandler> resultHandlers_;
};
}  // namespace ark::tooling::inspector

#endif  // PANDA_TOOLING_INSPECTOR_CONNECTION_ENDPOINT_BASE_H
