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

#ifndef PANDA_TOOLING_INSPECTOR_CONNECTION_SERVER_H
#define PANDA_TOOLING_INSPECTOR_CONNECTION_SERVER_H

#include <atomic>
#include <functional>

#include "json_serialization/serializable.h"
#include "libarkbase/os/mutex.h"
#include "libarkbase/utils/expected.h"
#include "libarkbase/utils/json_builder.h"

#include "json_serialization/jrpc_error.h"

namespace ark {
class JsonObject;
class JsonObjectBuilder;
}  // namespace ark

namespace ark::tooling::inspector {
/// @brief Base class for server with a single listener thread
class Server {
public:
    using MethodResponse = Expected<std::unique_ptr<JsonSerializable>, JRPCError>;
    using Handler = std::function<MethodResponse(const std::string &, const JsonObject &)>;

public:
    Server() = default;
    NO_COPY_SEMANTIC(Server);
    NO_MOVE_SEMANTIC(Server);
    virtual ~Server() = default;

    /// @brief Notifies the running server to stop.
    bool Kill()
    {
        return running_.exchange(false);
    }

    /// @brief Runs server loop until paused. This function must be entrypoint of listener thread.
    void Run(const std::string& msg)
    {
        ASSERT_PRINT(!running_, "Event loop is already running");

        ParseMessage(msg);
    }

    /// @brief Pauses the server while waiting for the current task to finish.
    void Pause() ACQUIRE_SHARED(taskExecution_)
    {
        taskExecution_.ReadLock();
    }

    /// @brief Notifies the event loop to continue.
    void Continue() RELEASE_GENERIC(taskExecution_)
    {
        taskExecution_.Unlock();
    }

    void OnCall(const char *method, Handler &&handler)
    {
        OnCallImpl(method, [this, h = std::move(handler)](const std::string &sessionId, const JsonObject &params) {
            os::memory::WriteLockHolder lock(taskExecution_);
            return h(sessionId, params);
        });
    }

    void Call(const char *method, std::function<void(JsonObjectBuilder &)> &&params)
    {
        Call({}, method, std::move(params));
    }

    void Call(const std::string &sessionId, const char *method)
    {
        Call(sessionId, method, [](JsonObjectBuilder & /* builder */) {});
    }

    void Call(const char *method)
    {
        Call({}, method, [](JsonObjectBuilder & /* builder */) {});
    }

    virtual void Call(const std::string &sessionId, const char *method,
                      std::function<void(JsonObjectBuilder &)> &&params) = 0;

    virtual void OnValidate(std::function<void()> &&handler) = 0;
    virtual void OnOpen(std::function<void()> &&handler) = 0;
    virtual void OnFail(std::function<void()> &&handler) = 0;

    // Run at most one event loop handler, may block.
    virtual bool ParseMessage(const std::string& msg) = 0;

private:
    virtual void OnCallImpl(const char *method, Handler &&handler) = 0;

private:
    std::atomic<bool> running_ {false};
    os::memory::RWLock taskExecution_;
};
}  // namespace ark::tooling::inspector

#endif  // PANDA_TOOLING_INSPECTOR_CONNECTION_SERVER_H
