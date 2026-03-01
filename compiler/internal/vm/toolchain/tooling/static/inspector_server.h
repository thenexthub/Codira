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

#ifndef PANDA_TOOLING_INSPECTOR_INSPECTOR_SERVER_H
#define PANDA_TOOLING_INSPECTOR_INSPECTOR_SERVER_H

#include <cstddef>
#include <cstdint>
#include <functional>
#include <optional>
#include <set>
#include <string_view>
#include <vector>

#include "include/console_call_type.h"
#include "include/tooling/pt_thread.h"

#include "common.h"
#include "session_manager.h"
#include "source_manager.h"
#include "debugger/thread_state.h"
#include "types/evaluation_result.h"
#include "types/numeric_id.h"
#include "types/pause_on_exceptions_state.h"
#include "types/profile_result.h"
#include "types/property_descriptor.h"
#include "types/remote_object.h"
#include "types/scope.h"

namespace ark::tooling::inspector {
class UrlBreakpointResponse;
class Server;  // NOLINT(fuchsia-virtual-inheritance)
class UrlBreakpointRequest;

class InspectorServer final {
public:
    using SetBreakpointHandler = std::optional<BreakpointId>(PtThread, SourceFileFilter &&, int32_t,
                                                             std::set<std::string_view> &, const std::string *);
    using FrameInfoHandler = std::function<void(FrameId, std::string_view, std::string_view, size_t,
                                                const std::vector<Scope> &, const std::optional<RemoteObject> &)>;

public:
    explicit InspectorServer(Server &server);
    ~InspectorServer() = default;

    NO_COPY_SEMANTIC(InspectorServer);
    NO_MOVE_SEMANTIC(InspectorServer);

    void Kill();
    void Run(const std::string& msg);

    void OnValidate(std::function<void()> &&handler);
    void OnOpen(std::function<void()> &&handler);
    void OnFail(std::function<void()> &&handler);

    void CallDebuggerPaused(PtThread thread, const std::vector<BreakpointId> &hitBreakpoints,
                            const std::optional<RemoteObject> &exception, PauseReason pauseReason,
                            const std::function<void(const FrameInfoHandler &)> &enumerateFrames);
    void CallDebuggerResumed(PtThread thread);
    void CallDebuggerScriptParsed(ScriptId scriptId, std::string_view sourceFile);
    void CallRuntimeConsoleApiCalled(PtThread thread, ConsoleCallType type, uint64_t timestamp,
                                     const std::vector<RemoteObject> &arguments);
    void CallRuntimeExecutionContextCreated(PtThread thread);
    void CallRuntimeExecutionContextsCleared();
    bool CallTargetAttachedToTarget(PtThread thread);
    void CallTargetDetachedFromTarget(PtThread thread);

    void OnCallDebuggerContinueToLocation(std::function<void(PtThread, std::string_view, int32_t)> &&handler);
    void OnCallDebuggerEnable(std::function<void()> &&handler);
    void OnCallDebuggerGetPossibleBreakpoints(
        std::function<std::set<int32_t>(std::string_view, int32_t, int32_t, bool)> &&handler);
    void OnCallDebuggerGetScriptSource(std::function<std::string(std::string_view)> &&handler);
    void OnCallDebuggerPause(std::function<void(PtThread)> &&handler);
    void OnCallDebuggerRemoveBreakpoint(std::function<void(PtThread, BreakpointId)> &&handler);
    void OnCallDebuggerRemoveBreakpointsByUrl(std::function<void(PtThread, const char*, SourceFileFilter)> &&handler);
    void OnCallDebuggerRestartFrame(std::function<void(PtThread, FrameId)> &&handler);
    void OnCallDebuggerResume(std::function<void(PtThread)> &&handler);
    void OnCallDebuggerSetAsyncCallStackDepth(std::function<void(PtThread)> &&handler);
    void OnCallDebuggerSetBlackboxPatterns(std::function<void(PtThread)> &&handler);
    void OnCallDebuggerSmartStepInto(std::function<void(PtThread)> &&handler);
    void OnCallDebuggerSetBreakpoint(std::function<SetBreakpointHandler> &&handler);
    void OnCallDebuggerSetBreakpointByUrl(std::function<SetBreakpointHandler> &&handler);
    void OnCallDebuggerGetPossibleAndSetBreakpointByUrl(std::function<SetBreakpointHandler> &&handler);
    void OnCallDebuggerSetBreakpointsActive(std::function<void(PtThread, bool)> &&handler);
    void OnCallDebuggerSetSkipAllPauses(std::function<void(PtThread, bool)> &&handler);
    void OnCallDebuggerSetPauseOnExceptions(std::function<void(PtThread, PauseOnExceptionsState)> &&handler);
    void OnCallDebuggerStepInto(std::function<void(PtThread)> &&handler);
    void OnCallDebuggerStepOut(std::function<void(PtThread)> &&handler);
    void OnCallDebuggerStepOver(std::function<void(PtThread)> &&handler);
    void OnCallDebuggerEvaluateOnCallFrame(
        std::function<Expected<EvaluationResult, std::string>(PtThread, const std::string &, size_t)> &&handler);
    void OnCallDebuggerClientDisconnect(std::function<void(PtThread)> &&handler);
    void OnCallDebuggerDisable(std::function<void(PtThread)> &&handler);
    void OnCallDebuggerDropFrame(std::function<void(PtThread)> &&handler);
    void OnCallDebuggerSetNativeRange(std::function<void(PtThread)> &&handler);
    void OnCallDebuggerReplyNativeCalling(std::function<void(PtThread)> &&handler);
    void OnCallDebuggerCallFunctionOn(
        std::function<Expected<EvaluationResult, std::string>(PtThread, const std::string &, size_t)> &&handler);
    void OnCallDebuggerSetMixedDebugEnabled(std::function<void(PtThread, bool)> &&handler);
    void OnCallRuntimeEnable(std::function<void(PtThread)> &&handler);
    void OnCallRuntimeGetProperties(
        std::function<std::vector<PropertyDescriptor>(PtThread, RemoteObjectId, bool)> &&handler);
    void OnCallRuntimeRunIfWaitingForDebugger(std::function<void(PtThread)> &&handler);
    void OnCallRuntimeEvaluate(
        std::function<Expected<EvaluationResult, std::string>(PtThread, const std::string &)> &&handler);
    void OnCallTargetAttachToTarget();
    void OnCallProfilerEnable();
    void OnCallProfilerDisable();
    void OnCallProfilerSetSamplingInterval(std::function<void(int32_t)> &&handler);
    void OnCallProfilerStart(std::function<Expected<bool, std::string>()> &&handler);
    void OnCallProfilerStop(std::function<Expected<Profile, std::string>()> &&handler);

    SourceManager &GetSourceManager()
    {
        return sourceManager_;
    }

private:
    struct CallFrameInfo {
        FrameId frameId;
        std::string_view sourceFile;
        std::string_view methodName;
        int32_t lineNumber;
    };

private:
    void SendTargetAttachedToTarget(const std::string &sessionId);
    void EnumerateCallFrames(JsonArrayBuilder &callFrames, PtThread thread,
                             const std::function<void(const FrameInfoHandler &)> &enumerateFrames);
    void AddCallFrameInfo(JsonArrayBuilder &callFrames, const CallFrameInfo &callFrameInfo,
                          const std::vector<Scope> &scopeChain, PtThread thread,
                          const std::optional<RemoteObject> &objThis);
    Expected<std::unique_ptr<UrlBreakpointResponse>, std::string> SetBreakpointByUrl(
        const std::string &sessionId, const UrlBreakpointRequest &breakpointRequest,
        const std::function<SetBreakpointHandler> &handler);
    void AddLocations(UrlBreakpointResponse &response, const std::set<std::string_view> &sourceFiles,
                      int32_t lineNumber, PtThread thread);
    static void AddHitBreakpoints(JsonArrayBuilder &hitBreakpointsBuilder,
                                  const std::vector<BreakpointId> &hitBreakpoints);
    static std::string GetExecutionContextUniqueId(const PtThread &thread);

    Server &server_;
    SessionManager sessionManager_;
    SourceManager sourceManager_;
};
}  // namespace ark::tooling::inspector

#endif  // PANDA_TOOLING_INSPECTOR_INSPECTOR_SERVER_H
