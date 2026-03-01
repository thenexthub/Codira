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

#include "connection/endpoint_base.h"

#include <optional>

#include "libarkbase/utils/json_builder.h"
#include "libarkbase/utils/json_parser.h"
#include "libarkbase/utils/logger.h"

namespace ark::tooling::inspector {
void EndpointBase::OnCall(const char *method, EndpointBase::MethodHandler &&handler)
{
    os::memory::LockHolder lock(methodHandlersMutex_);
    methodHandlers_[method] = std::move(handler);
}

void EndpointBase::HandleMessage(const std::string &message)
{
    JsonObject request(message);
    if (!request.IsValid()) {
        LOG(INFO, DEBUGGER) << "Invalid request: " << message;
        return;
    }

    LOG(INFO, DEBUGGER) << "Received " << message;

    auto sessionId = request.GetValue<JsonObject::StringT>("sessionId");
    auto id = request.GetValue<JsonObject::NumT>("id");
    auto method = request.GetValue<JsonObject::StringT>("method");
    auto result = request.GetValue<JsonObject::JsonObjPointer>("result");

    if (method != nullptr && result == nullptr) {
        os::memory::LockHolder lock(methodHandlersMutex_);

        auto optId = id != nullptr ? std::make_optional(*id) : std::nullopt;
        if (auto handler = methodHandlers_.find(*method); handler != methodHandlers_.end()) {
            auto *params = request.GetValue<JsonObject::JsonObjPointer>("params");

            JsonObject empty;

            handler->second(sessionId != nullptr ? *sessionId : std::string(), optId,
                            params != nullptr ? **params : empty);
        } else {
            LOG(WARNING, DEBUGGER) << "Unsupported method: " << message;
            HandleUnsupportedMethod(optId, *method);
        }
    } else if (result != nullptr && method == nullptr) {
        if (id == nullptr) {
            LOG(INFO, DEBUGGER) << "Response object with no \"id\": " << message;
            return;
        }

        if (auto handler = resultHandlers_.extract(*id)) {
            handler.mapped()(**result);
        }
    } else {
        LOG(INFO, DEBUGGER) << "Expected either 'method' or 'result' parameter";
    }
}

void EndpointBase::Call(const std::string &sessionId, std::optional<Id> id, const char *method,
                        std::function<void(JsonObjectBuilder &)> &&params)
{
    Send([&sessionId, id, method, &params](JsonObjectBuilder &call) {
        if (id) {
            call.AddProperty("id", *id);
        }

        call.AddProperty("method", method);
        call.AddProperty("params", std::move(params));

        if (!sessionId.empty()) {
            call.AddProperty("sessionId", sessionId);
        }
    });
}

void EndpointBase::HandleUnsupportedMethod(std::optional<double> optId, const std::string &method)
{
    Send([&optId, &method](JsonObjectBuilder &builder) {
        if (optId) {
            builder.AddProperty("id", *optId);
        }
        builder.AddProperty("error", [&method](JsonObjectBuilder &errorBuilder) {
            errorBuilder.AddProperty("code", static_cast<int>(InspectorErrorCode::METHOD_NOT_FOUND));
            errorBuilder.AddProperty("message", "Unsupported method: " + method);
        });
    });
}
}  // namespace ark::tooling::inspector
