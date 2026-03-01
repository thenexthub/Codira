/**
 * Copyright (c) 2022-2026 Huawei Device Co., Ltd.
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

#include "inspector_server.h"

#include <functional>
#include <memory>
#include <regex>
#include <string>
#include <utility>

#include "include/console_call_type.h"
#include "include/tooling/pt_thread.h"
#include "json_serialization/serializable.h"
#include "libarkbase/macros.h"
#include "libarkbase/utils/expected.h"
#include "libarkbase/utils/json_builder.h"
#include "libarkbase/utils/json_parser.h"
#include "libarkbase/utils/logger.h"

#include "connection/server.h"
#include "json_serialization/jrpc_error.h"
#include "types/custom_url_breakpoint_response.h"
#include "types/debugger_evaluation_request.h"
#include "types/debugger_call_function_on_request.h"
#include "types/location.h"
#include "types/numeric_id.h"
#include "types/possible_breakpoints_response.h"
#include "types/script_source_response.h"
#include "types/url_breakpoint_request.h"
#include "types/url_breakpoint_response.h"

namespace ark::tooling::inspector {
InspectorServer::InspectorServer(Server &server) : server_(server)
{
    OnCallTargetAttachToTarget();
}

void InspectorServer::Kill()
{
    server_.Kill();
}

void InspectorServer::Run(const std::string& msg)
{
    server_.Run(msg);
}

void InspectorServer::OnValidate(std::function<void()> &&handler)
{
    server_.OnValidate([handler = std::move(handler)]() {
        // Pause debugger events processing
        handler();

        // At this point, the whole messaging is stopped due to:
        // - Debugger events are not processed by the inspector after the call above;
        // - Client messages are not processed as we are executing on the server thread.
    });
}

void InspectorServer::OnOpen(std::function<void()> &&handler)
{
    server_.OnOpen([this, handler = std::move(handler)]() {
        // A new connection is open, reinitialize the state
        sessionManager_.EnumerateSessions([this](auto &id, [[maybe_unused]] auto thread) {
            if (!id.empty()) {
                SendTargetAttachedToTarget(id);
            }
        });

        // Resume debugger events processing
        handler();
    });
}

void InspectorServer::OnFail(std::function<void()> &&handler)
{
    server_.OnFail([handler = std::move(handler)]() {
        // Resume debugger events processing
        handler();
    });
}

static std::string_view GetPauseReasonString(PauseReason reason)
{
    switch (reason) {
        case PauseReason::EXCEPTION:
            return "exception";
        case PauseReason::BREAK_ON_START:
            return "Break on start";
        default:
            return "other";
    }
    UNREACHABLE();
}

void InspectorServer::CallDebuggerPaused(PtThread thread, const std::vector<BreakpointId> &hitBreakpoints,
                                         const std::optional<RemoteObject> &exception, PauseReason pauseReason,
                                         const std::function<void(const FrameInfoHandler &)> &enumerateFrames)
{
    auto sessionId = sessionManager_.GetSessionIdByThread(thread);

    server_.Call(sessionId, "Debugger.paused", [&](auto &params) {
        params.AddProperty("callFrames", [this, thread, &enumerateFrames](JsonArrayBuilder &callFrames) {
            EnumerateCallFrames(callFrames, thread, enumerateFrames);
        });

        params.AddProperty("hitBreakpoints", [&hitBreakpoints](JsonArrayBuilder &hitBreakpointsBuilder) {
            AddHitBreakpoints(hitBreakpointsBuilder, hitBreakpoints);
        });

        if (exception) {
            params.AddProperty("data", *exception);
        }

        params.AddProperty("reason", GetPauseReasonString(pauseReason));
        params.AddProperty("sessionId", sessionId);
        params.AddProperty("allSessions", [&](JsonArrayBuilder &sessionsBuilder) {
            sessionManager_.GetAllSessions(sessionsBuilder);
        });
    });
}

void InspectorServer::CallDebuggerResumed(PtThread thread)
{
    server_.Call(sessionManager_.GetSessionIdByThread(thread), "Debugger.resumed");
}

void InspectorServer::CallDebuggerScriptParsed(ScriptId scriptId, std::string_view sourceFile)
{
    server_.Call("", "Debugger.scriptParsed", [&sourceFile, &scriptId](auto &params) {
        params.AddProperty("executionContextId", 0);
        params.AddProperty("scriptId", std::to_string(scriptId));
        params.AddProperty("url", sourceFile.data());
        params.AddProperty("startLine", 0);
        params.AddProperty("startColumn", 0);
        params.AddProperty("endLine", std::numeric_limits<int>::max());
        params.AddProperty("endColumn", std::numeric_limits<int>::max());
        params.AddProperty("hash", "");
    });
}

void InspectorServer::CallRuntimeConsoleApiCalled(PtThread thread, ConsoleCallType type, uint64_t timestamp,
                                                  const std::vector<RemoteObject> &arguments)
{
    auto sessionId = sessionManager_.GetSessionIdByThread(thread);

    server_.Call(sessionId, "Runtime.consoleAPICalled", [&](auto &params) {
        params.AddProperty("executionContextId", 0);
        params.AddProperty("timestamp", timestamp);

        switch (type) {
            case ConsoleCallType::LOG:
                params.AddProperty("type", "log");
                break;
            case ConsoleCallType::DEBUG:
                params.AddProperty("type", "debug");
                break;
            case ConsoleCallType::INFO:
                params.AddProperty("type", "info");
                break;
            case ConsoleCallType::ERROR:
                params.AddProperty("type", "error");
                break;
            case ConsoleCallType::WARNING:
                params.AddProperty("type", "warning");
                break;
            default:
                UNREACHABLE();
        }

        params.AddProperty("args", [&](JsonArrayBuilder &argsBuilder) {
            for (const auto &argument : arguments) {
                argsBuilder.Add(argument);
            }
        });
    });
}

void InspectorServer::CallRuntimeExecutionContextCreated(PtThread thread)
{
    auto sessionId = sessionManager_.GetSessionIdByThread(thread);

    std::string name;
    if (thread != PtThread::NONE) {
        name = "Thread #" + std::to_string(thread.GetId());
    }

    server_.Call(sessionId, "Runtime.executionContextCreated", [&](auto &params) {
        params.AddProperty("context", [&](JsonObjectBuilder &context) {
            context.AddProperty("id", thread.GetId());
            context.AddProperty("origin", "");
            context.AddProperty("name", name);
            context.AddProperty("uniqueId", GetExecutionContextUniqueId(thread));
        });
    });
}

void InspectorServer::CallRuntimeExecutionContextsCleared()
{
    server_.Call("Runtime.executionContextsCleared");
}

bool InspectorServer::CallTargetAttachedToTarget(PtThread thread)
{
    auto &sessionId = sessionManager_.AddSession(thread);
    if (!sessionId.empty()) {
        SendTargetAttachedToTarget(sessionId);
        return true;
    }
    return false;
}

void InspectorServer::CallTargetDetachedFromTarget(PtThread thread)
{
    auto sessionId = sessionManager_.GetSessionIdByThread(thread);

    // Pause the server thread to ensure that there will be no dangling PtThreads
    server_.Pause();

    sessionManager_.RemoveSession(sessionId);

    // Now no one will retrieve the detached thread from the sessions manager
    server_.Continue();

    if (!sessionId.empty()) {
        server_.Call("Target.detachedFromTarget",
                     [&sessionId](auto &params) { params.AddProperty("sessionId", sessionId); });
    }
}

void InspectorServer::OnCallDebuggerContinueToLocation(
    std::function<void(PtThread, std::string_view, int32_t)> &&handler)
{
    // clang-format off
    server_.OnCall("Debugger.continueToLocation",
        [this, handler = std::move(handler)](auto &sessionId, const auto &params) -> Server::MethodResponse {
            auto location = Location::FromJsonProperty(params, "location");
            if (!location) {
                LOG(INFO, DEBUGGER) << location.Error();
                return Unexpected(JRPCError(location.Error()));
            }

            auto thread = sessionManager_.GetThreadBySessionId(sessionId);
            handler(thread, sourceManager_.GetSourceFileName(location->GetScriptId()),
                    location->GetLineNumber());
            return std::unique_ptr<JsonSerializable>();
        });
    // clang-format on
}

void InspectorServer::OnCallDebuggerEnable(std::function<void()> &&handler)
{
    struct Response : public JsonSerializable {
        void Serialize(JsonObjectBuilder &builder) const override
        {
            builder.AddProperty("debuggerId", 0);
            builder.AddProperty("protocols", [](JsonArrayBuilder &) {});
        }
    };

    server_.OnCall("Debugger.enable", [handler = std::move(handler)](auto, auto &) {
        handler();
        return std::unique_ptr<JsonSerializable>(std::make_unique<Response>());
    });
}

void InspectorServer::OnCallDebuggerGetPossibleBreakpoints(
    std::function<std::set<int32_t>(std::string_view, int32_t, int32_t, bool)> &&handler)
{
    // clang-format off
    server_.OnCall("Debugger.getPossibleBreakpoints",
        [this, handler = std::move(handler)](auto &, const JsonObject &params) -> Server::MethodResponse {
            auto optStart = Location::FromJsonProperty(params, "start");
            if (!optStart) {
                LOG(INFO, DEBUGGER) << optStart.Error();
                return Unexpected(JRPCError(optStart.Error(), ErrorCode::PARSE_ERROR));
            }

            auto scriptId = optStart->GetScriptId();

            int32_t endLine = ~0U;
            if (auto end = Location::FromJsonProperty(params, "end")) {
                if (end->GetScriptId() != scriptId) {
                    std::string_view msg = "Script ids don't match";
                    LOG(INFO, DEBUGGER) << msg;
                    return Unexpected(JRPCError(msg, ErrorCode::INVALID_PARAMS));
                }

                endLine = end->GetLineNumber();
            }

            bool restrictToFunction = false;
            if (auto prop = params.GetValue<JsonObject::BoolT>("restrictToFunction")) {
                restrictToFunction = *prop;
            }

            auto lineNumbers = handler(sourceManager_.GetSourceFileName(scriptId), optStart->GetLineNumber(),
                                       endLine, restrictToFunction);
            auto response = std::make_unique<PossibleBreakpointsResponse>();
            for (const auto &line : lineNumbers) {
                response->Add(Location(scriptId, line));
            }
            return std::unique_ptr<JsonSerializable>(std::move(response));
        });
    // clang-format on
}

void InspectorServer::OnCallDebuggerGetScriptSource(std::function<std::string(std::string_view)> &&handler)
{
    // clang-format off
    server_.OnCall("Debugger.getScriptSource",
        [this, handler = std::move(handler)](auto &, auto &params) -> Server::MethodResponse {
            auto scriptId = ParseNumericId<ScriptId>(params, "scriptId");
            if (scriptId) {
                auto sourceFile = sourceManager_.GetSourceFileName(*scriptId);
                return std::unique_ptr<JsonSerializable>(
                    std::make_unique<ScriptSourceResponse>(handler(sourceFile)));
            }
            LOG(INFO, DEBUGGER) << scriptId.Error();
            return Unexpected(JRPCError(scriptId.Error(), ErrorCode::PARSE_ERROR));
        });
    // clang-format on
}

void InspectorServer::OnCallDebuggerPause(std::function<void(PtThread)> &&handler)
{
    server_.OnCall("Debugger.pause", [this, handler = std::move(handler)](auto &sessionId, auto &) {
        auto thread = sessionManager_.GetThreadBySessionId(sessionId);
        handler(thread);
        return std::unique_ptr<JsonSerializable>();
    });
}

void InspectorServer::OnCallDebuggerRemoveBreakpoint(std::function<void(PtThread, BreakpointId)> &&handler)
{
    // clang-format off
    server_.OnCall("Debugger.removeBreakpoint",
        [this, handler = std::move(handler)](auto &sessionId, auto &params) -> Server::MethodResponse {
            auto breakpointId = ParseNumericId<BreakpointId>(params, "breakpointId");
            if (breakpointId) {
                handler(sessionManager_.GetThreadBySessionId(sessionId), *breakpointId);
                return std::unique_ptr<JsonSerializable>();
            }
            LOG(INFO, DEBUGGER) << breakpointId.Error();
            return Unexpected(JRPCError(breakpointId.Error()));
        });
    // clang-format on
}

static bool IsPathEqual(const std::string_view &src, const std::string_view &dst)
{
    if (src.size() != dst.size()) {
        return false;
    }

    for (size_t i = 0; i < src.size(); ++i) {
        if ((src[i] == '\\' || src[i] == '/') && (dst[i] == '\\' || dst[i] == '/')) {
            continue;
        }
        if (src[i] != dst[i]) {
            return false;
        }
    }

    return true;
}

static auto GetUrlFileFilter(const std::string &url)
{
    static constexpr std::string_view FILE_PREFIX = "file://";
    return [sourceFile = url.find(FILE_PREFIX) == 0 ? url.substr(FILE_PREFIX.size()) : url](auto fileName) {
        return IsPathEqual(sourceFile, fileName);
    };
}

void InspectorServer::OnCallDebuggerRemoveBreakpointsByUrl(
    std::function<void(PtThread, const char*, SourceFileFilter)> &&handler)
{
    // clang-format off
    server_.OnCall("Debugger.removeBreakpointsByUrl",
        [this, handler = std::move(handler)](auto &sessionId, auto &params) -> Server::MethodResponse {
            const auto *url = params.template GetValue<JsonObject::StringT>("url");
            if (url == nullptr) {
                std::string_view msg = "No 'url' parameter was provided for Debugger.removeBreakpointsByUrl";
                LOG(INFO, DEBUGGER) << msg;
                return Unexpected(JRPCError(msg, ErrorCode::PARSE_ERROR));
            }
            handler(sessionManager_.GetThreadBySessionId(sessionId), (*url).c_str(), GetUrlFileFilter(*url));
            return std::unique_ptr<JsonSerializable>();
    });
    // clang-format on
}

void InspectorServer::OnCallDebuggerRestartFrame(std::function<void(PtThread, FrameId)> &&handler)
{
    struct Response : public JsonSerializable {
        void Serialize(JsonObjectBuilder &builder) const override
        {
            builder.AddProperty("callFrames", [](JsonArrayBuilder &) {});
        }
    };
    // clang-format off
    server_.OnCall("Debugger.restartFrame",
        [this, handler = std::move(handler)](auto &sessionId, auto &params) -> Server::MethodResponse {
            auto thread = sessionManager_.GetThreadBySessionId(sessionId);

            auto frameId = ParseNumericId<FrameId>(params, "callFrameId");
            if (!frameId) {
                LOG(INFO, DEBUGGER) << frameId.Error();
                return Unexpected(JRPCError(frameId.Error(), ErrorCode::PARSE_ERROR));
            }

            handler(thread, *frameId);
            return std::unique_ptr<JsonSerializable>(std::make_unique<Response>());
        });
    // clang-format on
}

void InspectorServer::OnCallDebuggerResume(std::function<void(PtThread)> &&handler)
{
    server_.OnCall("Debugger.resume", [this, handler = std::move(handler)](auto &sessionId, auto &) {
        auto thread = sessionManager_.GetThreadBySessionId(sessionId);
        handler(thread);
        return std::unique_ptr<JsonSerializable>();
    });
}

void InspectorServer::OnCallDebuggerSetBreakpoint(std::function<SetBreakpointHandler> &&handler)
{
    class Response : public JsonSerializable {
    public:
        explicit Response(std::string &&bpId, Location &&loc)
            : breakpointId_(std::move(bpId)), location_(std::move(loc))
        {
        }

        void Serialize(JsonObjectBuilder &builder) const override
        {
            builder.AddProperty("breakpointId", breakpointId_);
            builder.AddProperty("actualLocation", location_);
        }

    private:
        std::string breakpointId_;
        Location location_;
    };
    // clang-format off
    server_.OnCall("Debugger.setBreakpoint",
        [this, handler = std::move(handler)](auto &sessionId, auto &params) -> Server::MethodResponse {
            auto location = Location::FromJsonProperty(params, "location");
            if (!location) {
                LOG(INFO, DEBUGGER) << location.Error();
                return Unexpected(JRPCError(location.Error(), ErrorCode::PARSE_ERROR));
            }
            auto condition = params.template GetValue<JsonObject::StringT>("condition");

            auto thread = sessionManager_.GetThreadBySessionId(sessionId);

            auto sourceFile = sourceManager_.GetSourceFileName(location->GetScriptId());
            std::set<std::string_view> sourceFiles;

            auto id = handler(
                thread, [sourceFile](auto fileName) { return fileName == sourceFile; },
                location->GetLineNumber(), sourceFiles, condition);
            if (!id) {
                std::string_view msg = "Failed to set breakpoint";
                LOG(INFO, DEBUGGER) << msg;
                return Unexpected(JRPCError(msg, ErrorCode::INTERNAL_ERROR));
            }
            auto response = std::make_unique<Response>(std::move(std::to_string(*id)), std::move(*location));
            return std::unique_ptr<JsonSerializable>(std::move(response));
        });
    // clang-format on
}

Expected<std::unique_ptr<UrlBreakpointResponse>, std::string> InspectorServer::SetBreakpointByUrl(
    const std::string &sessionId, const UrlBreakpointRequest &breakpointRequest,
    const std::function<SetBreakpointHandler> &handler)
{
    std::function<bool(std::string_view)> sourceFileFilter;
    if (const auto &url = breakpointRequest.GetUrl()) {
        sourceFileFilter = GetUrlFileFilter(*url);
    } else if (const auto &urlRegex = breakpointRequest.GetUrlRegex()) {
        sourceFileFilter = [regex = std::regex(*urlRegex)](auto fileName) {
            return std::regex_match(fileName.data(), regex);
        };
    } else {
        // Either 'url' or 'urlRegex' must be specified - checked in parser
        UNREACHABLE();
    }

    const auto *condition = breakpointRequest.GetCondition().has_value() ? &*breakpointRequest.GetCondition() : nullptr;

    std::set<std::string_view> sourceFiles;
    auto thread = sessionManager_.GetThreadBySessionId(sessionId);

    auto id = handler(thread, std::move(sourceFileFilter), breakpointRequest.GetLineNumber(), sourceFiles, condition);
    if (!id) {
        std::string msg = "Failed to set breakpoint";
        LOG(INFO, DEBUGGER) << msg;
        return Unexpected(msg);
    }

    auto breakpointInfo = std::make_unique<UrlBreakpointResponse>(*id);
    AddLocations(*breakpointInfo, sourceFiles, breakpointRequest.GetLineNumber(), thread);
    return Expected<std::unique_ptr<UrlBreakpointResponse>, std::string>(std::move(breakpointInfo));
}

void InspectorServer::OnCallDebuggerSetBreakpointByUrl(std::function<SetBreakpointHandler> &&handler)
{
    // clang-format off
    server_.OnCall("Debugger.setBreakpointByUrl",
        [this, handler = std::move(handler)](auto &sessionId, const JsonObject &params) -> Server::MethodResponse {
            auto optRequest = UrlBreakpointRequest::FromJson(params);
            if (!optRequest) {
                std::stringstream ss;
                ss << "Parse error: " << optRequest.Error();
                auto msg = ss.str();
                LOG(INFO, DEBUGGER) << msg;
                return Unexpected(JRPCError(std::move(msg), ErrorCode::PARSE_ERROR));
            }
            auto optResponse = SetBreakpointByUrl(sessionId, *optRequest, handler);
            if (optResponse) {
                return std::unique_ptr<JsonSerializable>(std::move(optResponse.Value()));
            }
            return Unexpected(JRPCError(optResponse.Error()));
        });
    // clang-format on
}

static Expected<std::vector<UrlBreakpointRequest>, std::string> ParseUrlBreakpointRequests(
    const std::vector<JsonObject::Value> &rawRequests)
{
    std::vector<UrlBreakpointRequest> requestedBreakpoints;
    for (const auto &rawRequest : rawRequests) {
        auto *jsonObject = rawRequest.Get<JsonObject::JsonObjPointer>();
        if (jsonObject == nullptr) {
            std::string msg = "Invalid 'locations' array in getPossibleAndSetBreakpointByUrl";
            LOG(INFO, DEBUGGER) << msg;
            return Unexpected(std::move(msg));
        }
        auto optBreakpointRequest = UrlBreakpointRequest::FromJson(**jsonObject);
        if (!optBreakpointRequest) {
            std::stringstream ss;
            ss << "Invalid breakpoint request: " << optBreakpointRequest.Error();
            auto msg = ss.str();
            LOG(INFO, DEBUGGER) << msg;
            return Unexpected(std::move(msg));
        }
        requestedBreakpoints.push_back(std::move(*optBreakpointRequest));
    }
    return requestedBreakpoints;
}

void InspectorServer::OnCallDebuggerGetPossibleAndSetBreakpointByUrl(std::function<SetBreakpointHandler> &&handler)
{
    // clang-format off
    server_.OnCall("Debugger.getPossibleAndSetBreakpointByUrl",
        [this, handler = std::move(handler)](auto &sessionId, const JsonObject &params) -> Server::MethodResponse {
            auto rawRequests = params.GetValue<JsonObject::ArrayT>("locations");
            if (rawRequests == nullptr) {
                std::string_view msg = "No 'locations' array in getPossibleAndSetBreakpointByUrl";
                LOG(INFO, DEBUGGER) << msg;
                return Unexpected(JRPCError(msg, ErrorCode::PARSE_ERROR));
            }
            auto optRequests = ParseUrlBreakpointRequests(*rawRequests);
            if (!optRequests) {
                return Unexpected(JRPCError(std::move(optRequests.Error()), ErrorCode::PARSE_ERROR));
            }

            auto response = std::make_unique<CustomUrlBreakpointLocations>();
            for (const auto &req : *optRequests) {
                auto optResponse = SetBreakpointByUrl(sessionId, req, handler);
                if (!optResponse.HasValue()) {
                    response->Add(CustomUrlBreakpointResponse(req.GetLineNumber()));
                    continue;
                }

                if (optResponse.Value()->GetLocations().size() != 0) {
                    response->Add(optResponse.Value()->ToCustomUrlBreakpointResponse());
                } else {
                    auto resp = CustomUrlBreakpointResponse(req.GetLineNumber());
                    resp.SetBreakpointId(optResponse.Value()->GetBreakpointId());
                    response->Add(std::move(resp));
                }
            }
            return std::unique_ptr<JsonSerializable>(std::move(response));
        });
    // clang-format on
}

void InspectorServer::OnCallDebuggerSetBreakpointsActive(std::function<void(PtThread, bool)> &&handler)
{
    // clang-format off
    server_.OnCall("Debugger.setBreakpointsActive",
        [this, handler = std::move(handler)](auto &sessionId, const JsonObject &params) -> Server::MethodResponse {
            bool active;
            if (auto prop = params.GetValue<JsonObject::BoolT>("active")) {
                active = *prop;
            } else {
                std::string_view msg = "No 'active' property";
                LOG(INFO, DEBUGGER) << msg;
                return Unexpected(JRPCError(msg));
            }

            auto thread = sessionManager_.GetThreadBySessionId(sessionId);
            handler(thread, active);
            return std::unique_ptr<JsonSerializable>();
        });
    // clang-format on
}

void InspectorServer::OnCallDebuggerSetSkipAllPauses(std::function<void(PtThread, bool)> &&handler)
{
    // clang-format off
    server_.OnCall("Debugger.setSkipAllPauses",
        [this, handler = std::move(handler)](auto &sessionId, const JsonObject &params) -> Server::MethodResponse {
            bool skip;
            if (auto prop = params.GetValue<JsonObject::BoolT>("skip")) {
                skip = *prop;
            } else {
                std::string_view msg = "No 'active' property";
                LOG(INFO, DEBUGGER) << msg;
                return Unexpected(JRPCError(msg, ErrorCode::INVALID_PARAMS));
            }

            auto thread = sessionManager_.GetThreadBySessionId(sessionId);
            handler(thread, skip);
            return std::unique_ptr<JsonSerializable>();
        });
    // clang-format on
}

void InspectorServer::OnCallDebuggerSetPauseOnExceptions(
    std::function<void(PtThread, PauseOnExceptionsState)> &&handler)
{
    // clang-format off
    server_.OnCall("Debugger.setPauseOnExceptions",
        [this, handler = std::move(handler)](auto &sessionId, const JsonObject &params) -> Server::MethodResponse {
            auto thread = sessionManager_.GetThreadBySessionId(sessionId);

            PauseOnExceptionsState state;
            auto stateStr = params.GetValue<JsonObject::StringT>("state");
            if (stateStr == nullptr) {
                std::string_view msg = "No 'state' property";
                LOG(INFO, DEBUGGER) << msg;
                return Unexpected(JRPCError(msg, ErrorCode::INVALID_PARAMS));
            }

            if (*stateStr == "none") {
                state = PauseOnExceptionsState::NONE;
            } else if (*stateStr == "caught") {
                state = PauseOnExceptionsState::CAUGHT;
            } else if (*stateStr == "uncaught") {
                state = PauseOnExceptionsState::UNCAUGHT;
            } else if (*stateStr == "all") {
                state = PauseOnExceptionsState::ALL;
            } else {
                std::stringstream ss;
                ss << "Invalid 'state' value: " << *stateStr;
                auto msg = ss.str();
                LOG(INFO, DEBUGGER) << msg;
                return Unexpected(JRPCError(std::move(msg), ErrorCode::INVALID_PARAMS));
            }

            handler(thread, state);
            return std::unique_ptr<JsonSerializable>();
        });
    // clang-format on
}

void InspectorServer::OnCallDebuggerStepInto(std::function<void(PtThread)> &&handler)
{
    server_.OnCall("Debugger.stepInto", [this, handler = std::move(handler)](auto &sessionId, auto &) {
        auto thread = sessionManager_.GetThreadBySessionId(sessionId);
        handler(thread);
        return std::unique_ptr<JsonSerializable>();
    });
}

void InspectorServer::OnCallDebuggerStepOut(std::function<void(PtThread)> &&handler)
{
    server_.OnCall("Debugger.stepOut", [this, handler = std::move(handler)](auto &sessionId, auto &) {
        auto thread = sessionManager_.GetThreadBySessionId(sessionId);
        handler(thread);
        return std::unique_ptr<JsonSerializable>();
    });
}

void InspectorServer::OnCallDebuggerStepOver(std::function<void(PtThread)> &&handler)
{
    server_.OnCall("Debugger.stepOver", [this, handler = std::move(handler)](auto &sessionId, auto &) {
        auto thread = sessionManager_.GetThreadBySessionId(sessionId);
        handler(thread);
        return std::unique_ptr<JsonSerializable>();
    });
}

void InspectorServer::OnCallDebuggerEvaluateOnCallFrame(
    std::function<Expected<EvaluationResult, std::string>(PtThread, const std::string &, size_t)> &&handler)
{
    // clang-format off
    server_.OnCall("Debugger.evaluateOnCallFrame",
        [this, handler = std::move(handler)](auto &sessionId, const JsonObject &params) -> Server::MethodResponse {
            auto thread = sessionManager_.GetThreadBySessionId(sessionId);

            auto optRequest = DebuggerEvaluationRequest::FromJson(params);
            if (!optRequest) {
                LOG(INFO, DEBUGGER) << optRequest.Error();
                return Unexpected(JRPCError(std::move(optRequest.Error()), ErrorCode::PARSE_ERROR));
            }

            auto optResult = handler(thread, optRequest->GetExpression(), optRequest->GetCallFrameId());
            if (!optResult) {
                std::stringstream ss;
                ss << "Evaluation failed: " << optResult.Error();
                auto msg = ss.str();
                LOG(DEBUG, DEBUGGER) << msg;
                return Unexpected(JRPCError(std::move(msg), ErrorCode::INTERNAL_ERROR));
            }

            return std::unique_ptr<JsonSerializable>(std::make_unique<EvaluationResult>(std::move(*optResult)));
        });
    // clang-format on
}

void InspectorServer::OnCallDebuggerCallFunctionOn(
    std::function<Expected<EvaluationResult, std::string>(PtThread, const std::string &, size_t)> &&handler)
{
    // clang-format off
    server_.OnCall("Debugger.callFunctionOn",
        [this, handler = std::move(handler)](auto &sessionId, const JsonObject &params) -> Server::MethodResponse {
            auto thread = sessionManager_.GetThreadBySessionId(sessionId);

            auto optRequest = DebuggerCallFunctionOnRequest::FromJson(params);
            if (!optRequest) {
                LOG(INFO, DEBUGGER) << optRequest.Error();
                return Unexpected(JRPCError(std::move(optRequest.Error()), ErrorCode::PARSE_ERROR));
            }

            auto optResult = handler(thread, optRequest->GetFunctionDeclaration(), optRequest->GetCallFrameId());
            if (!optResult) {
                std::stringstream ss;
                ss << "Evaluation failed: " << optResult.Error();
                auto msg = ss.str();
                LOG(DEBUG, DEBUGGER) << msg;
                return Unexpected(JRPCError(std::move(msg), ErrorCode::INTERNAL_ERROR));
            }

            return std::unique_ptr<JsonSerializable>(std::make_unique<EvaluationResult>(std::move(*optResult)));
        });
    // clang-format on
}

void InspectorServer::OnCallDebuggerSetMixedDebugEnabled(std::function<void(PtThread, bool)> &&handler)
{
    // clang-format off
    server_.OnCall("Debugger.setMixedDebugEnabled",
        [this, handler = std::move(handler)](auto &sessionId, const JsonObject &params) -> Server::MethodResponse {
            bool mixedDebugEnabled;
            if (auto prop = params.GetValue<JsonObject::BoolT>("mixedDebugEnabled")) {
                mixedDebugEnabled = *prop;
            } else {
                std::string_view msg = "No 'active' property";
                LOG(INFO, DEBUGGER) << msg;
                return Unexpected(JRPCError(msg, ErrorCode::INVALID_PARAMS));
            }

            auto thread = sessionManager_.GetThreadBySessionId(sessionId);
            handler(thread, mixedDebugEnabled);
            return std::unique_ptr<JsonSerializable>();
        });
    // clang-format on
}

void InspectorServer::OnCallDebuggerDisable(std::function<void(PtThread)> &&handler)
{
    server_.OnCall("Debugger.disable", [this, handler = std::move(handler)](auto &sessionId, auto &) {
        auto thread = sessionManager_.GetThreadBySessionId(sessionId);
        handler(thread);
        return std::unique_ptr<JsonSerializable>();
    });
}

void InspectorServer::OnCallDebuggerClientDisconnect(std::function<void(PtThread)> &&handler)
{
    server_.OnCall("Debugger.clientDisconnect", [this, handler = std::move(handler)](auto &sessionId, auto &) {
        auto thread = sessionManager_.GetThreadBySessionId(sessionId);
        handler(thread);
        return std::unique_ptr<JsonSerializable>();
    });
}

void InspectorServer::OnCallDebuggerSetAsyncCallStackDepth(std::function<void(PtThread)> &&handler)
{
    server_.OnCall("Debugger.setAsyncCallStackDepth", [this, handler = std::move(handler)](auto &sessionId, auto &) {
        auto thread = sessionManager_.GetThreadBySessionId(sessionId);
        handler(thread);
        return std::unique_ptr<JsonSerializable>();
    });
}

void InspectorServer::OnCallDebuggerSetBlackboxPatterns(std::function<void(PtThread)> &&handler)
{
    server_.OnCall("Debugger.setBlackboxPatterns", [this, handler = std::move(handler)](auto &sessionId, auto &) {
        auto thread = sessionManager_.GetThreadBySessionId(sessionId);
        handler(thread);
        return std::unique_ptr<JsonSerializable>();
    });
}

void InspectorServer::OnCallDebuggerSmartStepInto(std::function<void(PtThread)> &&handler)
{
    server_.OnCall("Debugger.smartStepInto", [this, handler = std::move(handler)](auto &sessionId, auto &) {
        auto thread = sessionManager_.GetThreadBySessionId(sessionId);
        handler(thread);
        return std::unique_ptr<JsonSerializable>();
    });
}

void InspectorServer::OnCallDebuggerDropFrame(std::function<void(PtThread)> &&handler)
{
    server_.OnCall("Debugger.dropFrame", [this, handler = std::move(handler)](auto &sessionId, auto &) {
        auto thread = sessionManager_.GetThreadBySessionId(sessionId);
        handler(thread);
        return std::unique_ptr<JsonSerializable>();
    });
}

void InspectorServer::OnCallDebuggerSetNativeRange(std::function<void(PtThread)> &&handler)
{
    server_.OnCall("Debugger.setNativeRange", [this, handler = std::move(handler)](auto &sessionId, auto &) {
        auto thread = sessionManager_.GetThreadBySessionId(sessionId);
        handler(thread);
        return std::unique_ptr<JsonSerializable>();
    });
}

void InspectorServer::OnCallDebuggerReplyNativeCalling(std::function<void(PtThread)> &&handler)
{
    server_.OnCall("Debugger.replyNativeCalling", [this, handler = std::move(handler)](auto &sessionId, auto &) {
        auto thread = sessionManager_.GetThreadBySessionId(sessionId);
        handler(thread);
        return std::unique_ptr<JsonSerializable>();
    });
}

void InspectorServer::OnCallRuntimeEnable(std::function<void(PtThread)> &&handler)
{
    server_.OnCall("Runtime.enable", [this, handler = std::move(handler)](auto &sessionId, auto &) {
        auto thread = sessionManager_.GetThreadBySessionId(sessionId);
        handler(thread);
        return std::unique_ptr<JsonSerializable>();
    });
}

void InspectorServer::OnCallRuntimeGetProperties(
    std::function<std::vector<PropertyDescriptor>(PtThread, RemoteObjectId, bool)> &&handler)
{
    class Response : public JsonSerializable {
    public:
        explicit Response(std::vector<PropertyDescriptor> props) : properties_(std::move(props)) {}
        void Serialize(JsonObjectBuilder &builder) const override
        {
            builder.AddProperty("result", [&](JsonArrayBuilder &array) {
                for (auto &descriptor : properties_) {
                    array.Add(descriptor);
                }
            });
        }

    private:
        std::vector<PropertyDescriptor> properties_;
    };
    // clang-format off
    server_.OnCall("Runtime.getProperties",
        [this, handler = std::move(handler)](auto &sessionId, const JsonObject &params) -> Server::MethodResponse {
            auto thread = sessionManager_.GetThreadBySessionId(sessionId);

            auto objectId = ParseNumericId<RemoteObjectId>(params, "objectId");
            if (!objectId) {
                LOG(INFO, DEBUGGER) << objectId.Error();
                return Unexpected(JRPCError(objectId.Error(), ErrorCode::PARSE_ERROR));
            }

            auto generatePreview = false;
            if (auto prop = params.GetValue<JsonObject::BoolT>("generatePreview")) {
                generatePreview = *prop;
            }

            auto properties = handler(thread, *objectId, generatePreview);
            return std::unique_ptr<JsonSerializable>(std::make_unique<Response>(std::move(properties)));
        });
    // clang-format on
}

void InspectorServer::OnCallRuntimeRunIfWaitingForDebugger(std::function<void(PtThread)> &&handler)
{
    server_.OnCall("Runtime.runIfWaitingForDebugger", [this, handler = std::move(handler)](auto &sessionId, auto &) {
        auto thread = sessionManager_.GetThreadBySessionId(sessionId);
        handler(thread);
        return std::unique_ptr<JsonSerializable>();
    });
}

void InspectorServer::OnCallRuntimeEvaluate(
    std::function<Expected<EvaluationResult, std::string>(PtThread, const std::string &)> &&handler)
{
    // clang-format off
    server_.OnCall("Runtime.evaluate",
        [this, handler = std::move(handler)](auto &sessionId, const JsonObject &params) -> Server::MethodResponse {
            auto thread = sessionManager_.GetThreadBySessionId(sessionId);

            auto *expressionStr = params.GetValue<JsonObject::StringT>("expression");
            if (expressionStr == nullptr || expressionStr->empty()) {
                std::string_view msg = "'expression' property is absent or empty";
                LOG(INFO, DEBUGGER) << msg;
                return Unexpected(JRPCError(msg, ErrorCode::PARSE_ERROR));
            }

            auto optResult = handler(thread, *expressionStr);
            if (!optResult) {
                std::stringstream ss;
                ss << "Evaluation failed: " << optResult.Error();
                auto msg = ss.str();
                LOG(DEBUG, DEBUGGER) << msg;
                return Unexpected(JRPCError(std::move(msg), ErrorCode::INTERNAL_ERROR));
            }

            return std::unique_ptr<JsonSerializable>(std::make_unique<EvaluationResult>(std::move(*optResult)));
        });
    // clang-format on
}

void InspectorServer::OnCallProfilerEnable()
{
    server_.OnCall("Profiler.enable", [](auto &, auto &) { return std::unique_ptr<JsonSerializable>(); });
}

void InspectorServer::OnCallProfilerDisable()
{
    server_.OnCall("Profiler.disable", [](auto &, auto &) { return std::unique_ptr<JsonSerializable>(); });
}

void InspectorServer::OnCallProfilerSetSamplingInterval(std::function<void(int32_t)> &&handler)
{
    // clang-format off
    server_.OnCall("Profiler.setSamplingInterval",
        [handler = std::move(handler)](auto &, const JsonObject &params) -> Server::MethodResponse {
            int32_t interval = 0;
            if (auto prop = params.GetValue<JsonObject::NumT>("interval")) {
                interval = *prop;
                handler(interval);
            } else {
                std::string_view msg = "No 'interval' property";
                LOG(INFO, DEBUGGER) << msg;
                return Unexpected(JRPCError(msg, ErrorCode::INVALID_REQUEST));
            }
            return std::unique_ptr<JsonSerializable>();
        });
    // clang-format on
}

void InspectorServer::OnCallProfilerStart(std::function<Expected<bool, std::string>()> &&handler)
{
    server_.OnCall("Profiler.start", [handler = std::move(handler)](auto &, auto &) -> Server::MethodResponse {
        auto optResult = handler();
        if (!optResult) {
            std::string msg = "Profiler failed: ";
            msg = msg.append(optResult.Error());
            LOG(DEBUG, DEBUGGER) << msg;
            return Unexpected(JRPCError(std::move(msg), ErrorCode::INTERNAL_ERROR));
        }
        return std::unique_ptr<JsonSerializable>();
    });
}

void InspectorServer::OnCallProfilerStop(std::function<Expected<Profile, std::string>()> &&handler)
{
    server_.OnCall("Profiler.stop", [handler = std::move(handler)](auto &, auto &) -> Server::MethodResponse {
        auto optResult = handler();
        if (!optResult) {
            std::stringstream ss;
            ss << "Profiler failed: " << optResult.Error();
            auto msg = ss.str();
            LOG(DEBUG, DEBUGGER) << msg;
            return Unexpected(JRPCError(std::move(msg), ErrorCode::INTERNAL_ERROR));
        }
        return std::unique_ptr<JsonSerializable>(std::make_unique<Profile>(std::move(*optResult)));
    });
}

void InspectorServer::OnCallTargetAttachToTarget()
{
    class Response : public JsonSerializable {
    public:
        explicit Response(std::string sessionId) : sessionId_(std::move(sessionId)) {}
        void Serialize(JsonObjectBuilder &builder) const override
        {
            builder.AddProperty("sessionId", sessionId_);
        }

    private:
        std::string sessionId_;
    };
    // clang-format off
    server_.OnCall("Target.attachToTarget",
        [this](auto &, const JsonObject &params) -> Server::MethodResponse {
            const auto *targetId = params.template GetValue<JsonObject::StringT>("targetId");
            if (targetId == nullptr) {
                std::string_view msg = "No 'targetId' property";
                LOG(INFO, DEBUGGER) << msg;
                return Unexpected(JRPCError(msg, ErrorCode::INVALID_REQUEST));
            }
            auto thread = sessionManager_.GetThreadBySessionId(*targetId);
            if (thread == PtThread::NONE) {
                std::string msg = "'targetId' ";
                msg = msg.append(*targetId).append(" did not start");
                LOG(INFO, DEBUGGER) << msg;
                return Unexpected(JRPCError(msg, ErrorCode::INVALID_PARAMS));
            }
            // No-op if coroutine already exists
            return std::unique_ptr<JsonSerializable>(std::make_unique<Response>(*targetId));
        });
    // clang-format on
}

void InspectorServer::SendTargetAttachedToTarget(const std::string &sessionId)
{
    server_.Call("Target.attachedToTarget", [&sessionId](auto &params) {
        params.AddProperty("sessionId", sessionId);
        params.AddProperty("targetInfo", [&sessionId](JsonObjectBuilder &targetInfo) {
            targetInfo.AddProperty("targetId", sessionId);
            targetInfo.AddProperty("type", "worker");
            targetInfo.AddProperty("title", sessionId);
            targetInfo.AddProperty("url", "");
            targetInfo.AddProperty("attached", true);
            targetInfo.AddProperty("canAccessOpener", false);
        });
        params.AddProperty("waitingForDebugger", true);
    });
}

void InspectorServer::EnumerateCallFrames(JsonArrayBuilder &callFrames, PtThread thread,
                                          const std::function<void(const FrameInfoHandler &)> &enumerateFrames)
{
    enumerateFrames([this, thread, &callFrames](auto frameId, auto methodName, auto sourceFile, auto lineNumber,
                                                auto &scopeChain, auto &objThis) {
        CallFrameInfo callFrameInfo {frameId, sourceFile, methodName, lineNumber};
        AddCallFrameInfo(callFrames, callFrameInfo, scopeChain, thread, objThis);
    });
}

void InspectorServer::AddCallFrameInfo(JsonArrayBuilder &callFrames, const CallFrameInfo &callFrameInfo,
                                       const std::vector<Scope> &scopeChain, [[maybe_unused]] PtThread thread,
                                       const std::optional<RemoteObject> &objThis)
{
    callFrames.Add([&](JsonObjectBuilder &callFrame) {
        auto [scriptId, isNew] = sourceManager_.GetScriptId(callFrameInfo.sourceFile);

        if (isNew) {
            CallDebuggerScriptParsed(scriptId, callFrameInfo.sourceFile);
        }

        callFrame.AddProperty("callFrameId", std::to_string(callFrameInfo.frameId));
        callFrame.AddProperty("functionName", callFrameInfo.methodName.data());
        callFrame.AddProperty("location", Location(scriptId, callFrameInfo.lineNumber));
        callFrame.AddProperty("url", callFrameInfo.sourceFile.data());

        callFrame.AddProperty("scopeChain", [&](JsonArrayBuilder &scopeChainBuilder) {
            for (auto &scope : scopeChain) {
                scopeChainBuilder.Add(scope);
            }
        });

        callFrame.AddProperty("this", objThis.value_or(RemoteObject::Undefined()));
        callFrame.AddProperty("canBeRestarted", true);
    });
}

void InspectorServer::AddLocations(UrlBreakpointResponse &response, const std::set<std::string_view> &sourceFiles,
                                   int32_t lineNumber, [[maybe_unused]] PtThread thread)
{
    for (auto sourceFile : sourceFiles) {
        auto [scriptId, isNew] = sourceManager_.GetScriptId(sourceFile);

        if (isNew) {
            CallDebuggerScriptParsed(scriptId, sourceFile);
        }
        response.AddLocation(Location {scriptId, lineNumber});
    }
}

/* static */
void InspectorServer::AddHitBreakpoints(JsonArrayBuilder &hitBreakpointsBuilder,
                                        const std::vector<BreakpointId> &hitBreakpoints)
{
    for (auto id : hitBreakpoints) {
        hitBreakpointsBuilder.Add(std::to_string(id));
    }
}

/* static */
std::string InspectorServer::GetExecutionContextUniqueId(const PtThread &thread)
{
    static int pid = os::thread::GetPid();
    std::stringstream ss;
    ss << pid << ':' << thread.GetId();
    return ss.str();
}
}  // namespace ark::tooling::inspector
