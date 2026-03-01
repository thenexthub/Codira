/*
 * Copyright (c) 2023-2025 Huawei Device Co., Ltd.
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include "tooling/dynamic/client/domain/debugger_client.h"

#include "tooling/dynamic/client/session/session.h"

using PtJson = panda::ecmascript::tooling::PtJson;
namespace OHOS::ArkCompiler::Toolchain {
bool DebuggerClient::DispatcherCmd(const std::string &cmd)
{
    std::map<std::string, std::function<int()>> dispatcherTable {
        { "break", std::bind(&DebuggerClient::BreakCommand, this)},
        { "backtrack", std::bind(&DebuggerClient::BacktrackCommand, this)},
        { "continue", std::bind(&DebuggerClient::ResumeCommand, this)},
        { "delete", std::bind(&DebuggerClient::DeleteCommand, this)},
        { "jump", std::bind(&DebuggerClient::JumpCommand, this)},
        { "disable", std::bind(&DebuggerClient::DisableCommand, this)},
        { "display", std::bind(&DebuggerClient::DisplayCommand, this)},
        { "enable", std::bind(&DebuggerClient::EnableCommand, this)},
        { "finish", std::bind(&DebuggerClient::FinishCommand, this)},
        { "frame", std::bind(&DebuggerClient::FrameCommand, this)},
        { "ignore", std::bind(&DebuggerClient::IgnoreCommand, this)},
        { "infobreakpoints", std::bind(&DebuggerClient::InfobreakpointsCommand, this)},
        { "infosource", std::bind(&DebuggerClient::InfosourceCommand, this)},
        { "list", std::bind(&DebuggerClient::ListCommand, this)},
        { "next", std::bind(&DebuggerClient::NextCommand, this)},
        { "ptype", std::bind(&DebuggerClient::PtypeCommand, this)},
        { "run", std::bind(&DebuggerClient::RunCommand, this)},
        { "setvar", std::bind(&DebuggerClient::SetvarCommand, this)},
        { "step", std::bind(&DebuggerClient::StepCommand, this)},
        { "undisplay", std::bind(&DebuggerClient::UndisplayCommand, this)},
        { "watch", std::bind(&DebuggerClient::WatchCommand, this)},
        { "setAsyncStackDepth", std::bind(&DebuggerClient::AsyncStackDepthCommand, this)},
        { "resume", std::bind(&DebuggerClient::ResumeCommand, this)},
        { "step-into", std::bind(&DebuggerClient::StepIntoCommand, this)},
        { "step-out", std::bind(&DebuggerClient::StepOutCommand, this)},
        { "step-over", std::bind(&DebuggerClient::StepOverCommand, this)},
        { "enable-launch-accelerate", std::bind(&DebuggerClient::EnableLaunchAccelerateCommand, this)},
        { "removeBreakpointsByUrl", std::bind(&DebuggerClient::RemoveBreakpointsByUrlCommand, this)},
        { "saveAllPossibleBreakpoints", std::bind(&DebuggerClient::SaveAllPossibleBreakpointsCommand, this)},
        { "setSymbolicBreakpoints", std::bind(&DebuggerClient::SetSymbolicBreakpointsCommand, this)},
        { "removeSymbolicBreakpoints", std::bind(&DebuggerClient::RemoveSymbolicBreakpointsCommand, this)},
    };

    auto entry = dispatcherTable.find(cmd);
    if (entry != dispatcherTable.end()) {
        entry->second();
        LOGI("DebuggerClient DispatcherCmd cmd: %{public}s", cmd.c_str());
        return true;
    }

    LOGI("unknown command: %{public}s", cmd.c_str());
    return false;
}

int DebuggerClient::BreakCommand()
{
    Session *session = SessionManager::getInstance().GetSessionById(sessionId_);
    if (session == nullptr) {
        LOGE("get session by id %{public}u failed", sessionId_);
        return -1;
    }
    uint32_t id = session->GetMessageId();

    std::unique_ptr<PtJson> request = PtJson::CreateObject();
    request->Add("id", id);
    request->Add("method", "Debugger.setBreakpointByUrl");

    std::unique_ptr<PtJson> params = PtJson::CreateObject();
    params->Add("columnNumber", breakPointInfoList_.back().columnNumber);
    params->Add("lineNumber", breakPointInfoList_.back().lineNumber);
    params->Add("url", breakPointInfoList_.back().url.c_str());
    request->Add("params", params);

    std::string message = request->Stringify();
    if (session->ClientSendReq(message)) {
        session->GetDomainManager().SetDomainById(id, "Debugger");
    }
    return 0;
}

int DebuggerClient::BacktrackCommand()
{
    return 0;
}

int DebuggerClient::DeleteCommand()
{
    Session *session = SessionManager::getInstance().GetSessionById(sessionId_);
    if (session == nullptr) {
        LOGE("get session by id %{public}u failed", sessionId_);
        return -1;
    }
    uint32_t id = session->GetMessageId();

    std::unique_ptr<PtJson> request = PtJson::CreateObject();
    request->Add("id", id);
    request->Add("method", "Debugger.removeBreakpoint");

    std::unique_ptr<PtJson> params = PtJson::CreateObject();
    std::string breakpointId = breakPointInfoList_.back().url;
    params->Add("breakpointId", breakpointId.c_str());
    request->Add("params", params);

    std::string message = request->Stringify();
    if (session->ClientSendReq(message)) {
        session->GetDomainManager().SetDomainById(id, "Debugger");
    }
    return 0;
}

int DebuggerClient::DisableCommand()
{
    Session *session = SessionManager::getInstance().GetSessionById(sessionId_);
    if (session == nullptr) {
        LOGE("get session by id %{public}u failed", sessionId_);
        return -1;
    }
    uint32_t id = session->GetMessageId();

    std::unique_ptr<PtJson> request = PtJson::CreateObject();
    request->Add("id", id);
    request->Add("method", "Debugger.disable");

    std::unique_ptr<PtJson> params = PtJson::CreateObject();
    request->Add("params", params);

    std::string message = request->Stringify();
    if (session->ClientSendReq(message)) {
        session->GetDomainManager().SetDomainById(id, "Debugger");
    }
    return 0;
}

int DebuggerClient::DisplayCommand()
{
    return 0;
}

int DebuggerClient::EnableCommand()
{
    Session *session = SessionManager::getInstance().GetSessionById(sessionId_);
    if (session == nullptr) {
        LOGE("get session by id %{public}u failed", sessionId_);
        return -1;
    }
    uint32_t id = session->GetMessageId();

    std::unique_ptr<PtJson> request = PtJson::CreateObject();
    request->Add("id", id);
    request->Add("method", "Debugger.enable");

    std::unique_ptr<PtJson> params = PtJson::CreateObject();
    request->Add("params", params);

    std::string message = request->Stringify();
    if (session->ClientSendReq(message)) {
        session->GetDomainManager().SetDomainById(id, "Debugger");
    }
    return 0;
}

int DebuggerClient::FinishCommand()
{
    return 0;
}

int DebuggerClient::FrameCommand()
{
    return 0;
}

int DebuggerClient::IgnoreCommand()
{
    return 0;
}

int DebuggerClient::InfobreakpointsCommand()
{
    return 0;
}

int DebuggerClient::InfosourceCommand()
{
    return 0;
}

int DebuggerClient::JumpCommand()
{
    return 0;
}

int DebuggerClient::NextCommand()
{
    return 0;
}

int DebuggerClient::ListCommand()
{
    return 0;
}

int DebuggerClient::PtypeCommand()
{
    return 0;
}

int DebuggerClient::RunCommand()
{
    return 0;
}

int DebuggerClient::SetvarCommand()
{
    return 0;
}

int DebuggerClient::StepCommand()
{
    return 0;
}

int DebuggerClient::UndisplayCommand()
{
    return 0;
}

int DebuggerClient::WatchCommand()
{
    return 0;
}

int DebuggerClient::AsyncStackDepthCommand()
{
    Session *session = SessionManager::getInstance().GetSessionById(sessionId_);
    if (session == nullptr) {
        LOGE("get session by id %{public}u failed", sessionId_);
        return -1;
    }
    uint32_t id = session->GetMessageId();

    std::unique_ptr<PtJson> request = PtJson::CreateObject();
    request->Add("id", id);
    request->Add("method", "Debugger.setAsyncCallStackDepth");

    std::unique_ptr<PtJson> params = PtJson::CreateObject();
    // 32 : maxAsyncCallChainDepth
    params->Add("maxDepth", 32);
    request->Add("params", params);

    std::string message = request->Stringify();
    if (session->ClientSendReq(message)) {
        session->GetDomainManager().SetDomainById(id, "Debugger");
    }
    return 0;
}

int DebuggerClient::ResumeCommand()
{
    Session *session = SessionManager::getInstance().GetSessionById(sessionId_);
    if (session == nullptr) {
        LOGE("get session by id %{public}u failed", sessionId_);
        return -1;
    }
    uint32_t id = session->GetMessageId();

    std::unique_ptr<PtJson> request = PtJson::CreateObject();
    request->Add("id", id);
    request->Add("method", "Debugger.resume");

    std::unique_ptr<PtJson> params = PtJson::CreateObject();
    request->Add("params", params);

    std::string message = request->Stringify();
    if (session->ClientSendReq(message)) {
        session->GetDomainManager().SetDomainById(id, "Debugger");
    }
    return 0;
}

int DebuggerClient::StepIntoCommand()
{
    Session *session = SessionManager::getInstance().GetSessionById(sessionId_);
    if (session == nullptr) {
        LOGE("get session by id %{public}u failed", sessionId_);
        return -1;
    }
    uint32_t id = session->GetMessageId();

    std::unique_ptr<PtJson> request = PtJson::CreateObject();
    request->Add("id", id);
    request->Add("method", "Debugger.stepInto");

    std::unique_ptr<PtJson> params = PtJson::CreateObject();
    request->Add("params", params);

    std::string message = request->Stringify();
    if (session->ClientSendReq(message)) {
        session->GetDomainManager().SetDomainById(id, "Debugger");
    }
    return 0;
}

int DebuggerClient::StepOutCommand()
{
    Session *session = SessionManager::getInstance().GetSessionById(sessionId_);
    if (session == nullptr) {
        LOGE("get session by id %{public}u failed", sessionId_);
        return -1;
    }
    uint32_t id = session->GetMessageId();

    std::unique_ptr<PtJson> request = PtJson::CreateObject();
    request->Add("id", id);
    request->Add("method", "Debugger.stepOut");

    std::unique_ptr<PtJson> params = PtJson::CreateObject();
    request->Add("params", params);

    std::string message = request->Stringify();
    if (session->ClientSendReq(message)) {
        session->GetDomainManager().SetDomainById(id, "Debugger");
    }
    return 0;
}

int DebuggerClient::StepOverCommand()
{
    Session *session = SessionManager::getInstance().GetSessionById(sessionId_);
    if (session == nullptr) {
        LOGE("get session by id %{public}u failed", sessionId_);
        return -1;
    }
    uint32_t id = session->GetMessageId();

    std::unique_ptr<PtJson> request = PtJson::CreateObject();
    request->Add("id", id);
    request->Add("method", "Debugger.stepOver");

    std::unique_ptr<PtJson> params = PtJson::CreateObject();
    request->Add("params", params);

    std::string message = request->Stringify();
    if (session->ClientSendReq(message)) {
        session->GetDomainManager().SetDomainById(id, "Debugger");
    }
    return 0;
}

void DebuggerClient::AddBreakPointInfo(const std::string& url, const int& lineNumber, const int& columnNumber)
{
    BreakPointInfo breakPointInfo;
    breakPointInfo.url = url;
    breakPointInfo.lineNumber = lineNumber - 1;
    breakPointInfo.columnNumber = columnNumber;
    breakPointInfoList_.emplace_back(breakPointInfo);
}

void DebuggerClient::AddSymbolicBreakpointInfo(const std::string& functionName)
{
    SymbolicBreakpointInfo symbolicBreakpointInfo;
    symbolicBreakpointInfo.functionName = functionName;
    symbolicBreakpointInfoList_.emplace_back(symbolicBreakpointInfo);
}

void DebuggerClient::AddUrl(const std::string& url)
{
    urlList_.push_back(url);
}

void DebuggerClient::RecvReply(std::unique_ptr<PtJson> json)
{
    if (json == nullptr) {
        LOGE("arkdb: json parse error");
        return;
    }

    if (!json->IsObject()) {
        LOGE("arkdb: json parse format error");
        json->ReleaseRoot();
        return;
    }

    std::string wholeMethod;
    std::string method;
    Result ret = json->GetString("method", &wholeMethod);
    if (ret != Result::SUCCESS) {
        LOGE("arkdb: find method error");
    }

    Session *session = SessionManager::getInstance().GetSessionById(sessionId_);
    if (session == nullptr) {
        LOGE("get session by id %{public}u failed", sessionId_);
        return;
    }
    SourceManager &sourceManager = session->GetSourceManager();
    WatchManager &watchManager = session->GetWatchManager();

    std::string::size_type length = wholeMethod.length();
    std::string::size_type indexPoint = 0;
    indexPoint = wholeMethod.find_first_of('.', 0);
    method = wholeMethod.substr(indexPoint + 1, length);
    if (method == "paused") {
        PausedReply(std::move(json));
        return;
    } else if (method == "scriptParsed") {
        sourceManager.EnableReply(std::move(json));
        return;
    } else if (method == "resumed") {
        watchManager.DebugFalseState();
        return;
    } else {
        LOGI("arkdb: Debugger reply is: %{public}s", json->Stringify().c_str());
    }
    HandleResponse(std::move(json));
}

void DebuggerClient::PausedReply(const std::unique_ptr<PtJson> json)
{
    if (json == nullptr) {
        LOGE("arkdb: json parse error");
        return;
    }

    if (!json->IsObject()) {
        LOGE("arkdb: json parse format error");
        json->ReleaseRoot();
        return;
    }

    std::unique_ptr<PtJson> params;
    Result ret = json->GetObject("params", &params);
    if (ret != Result::SUCCESS) {
        LOGE("arkdb: find params error");
        return;
    }

    std::unique_ptr<PtJson> callFrames;
    ret = params->GetArray("callFrames", &callFrames);
    if (ret != Result::SUCCESS) {
        LOGE("arkdb: find callFrames error");
        return;
    }

    std::map<int32_t, std::unique_ptr<CallFrame>> data;
    for (int32_t i = 0; i < callFrames->GetSize(); i++) {
        std::unique_ptr<CallFrame> callFrameInfo = CallFrame::Create(*(callFrames->Get(i)));
        if (callFrameInfo == nullptr) {
            LOGE("arkdb: get call frame info error");
            return;
        }
        data.emplace(i + 1, std::move(callFrameInfo));
    }

    Session *session = SessionManager::getInstance().GetSessionById(sessionId_);
    if (session == nullptr) {
        LOGE("get session by id %{public}u failed", sessionId_);
        return;
    }
    StackManager& stackManager = session->GetStackManager();
    SourceManager &sourceManager = session->GetSourceManager();
    WatchManager &watchManager = session->GetWatchManager();
    stackManager.ClearCallFrame();
    stackManager.SetCallFrames(std::move(data));
    sourceManager.GetDebugSources(callFrames->Get(0));
    watchManager.RequestWatchInfo(callFrames->Get(0));
    watchManager.DebugTrueState();
}

void DebuggerClient::HandleResponse(std::unique_ptr<PtJson> json)
{
    Session *session = SessionManager::getInstance().GetSessionById(sessionId_);
    if (session == nullptr) {
        LOGE("get session by id %{public}u failed", sessionId_);
        return;
    }
    SourceManager &sourceManager = session->GetSourceManager();
    WatchManager &watchManager = session->GetWatchManager();
    BreakPointManager& breakpoint = session->GetBreakPointManager();
    std::unique_ptr<PtJson> result;
    Result ret = json->GetObject("result", &result);
    if (ret != Result::SUCCESS) {
        LOGE("arkdb: find result error");
        return;
    }
    int32_t id;
    ret = json->GetInt("id", &id);
    if (ret == Result::SUCCESS) {
        std::string scriptSource;
        ret = result->GetString("scriptSource", &scriptSource);
        if (ret == Result::SUCCESS) {
            sourceManager.SetFileSource(id, scriptSource);
            return;
        }
    }
    std::string breakpointId;
    ret = result->GetString("breakpointId", &breakpointId);
    if (ret == Result::SUCCESS) {
        breakpoint.Createbreaklocation(std::move(json));
        return;
    }
    if (watchManager.HandleWatchResult(std::move(json), id)) {
        return;
    }
    return;
}

int DebuggerClient::SaveAllPossibleBreakpointsCommand()
{
    Session *session = SessionManager::getInstance().GetSessionById(sessionId_);
    if (session == nullptr) {
        LOGE("get session by id %{public}u failed", sessionId_);
        return -1;
    }
    uint32_t id = session->GetMessageId();

    std::unique_ptr<PtJson> request = PtJson::CreateObject();
    request->Add("id", id);
    request->Add("method", "Debugger.saveAllPossibleBreakpoints");

    std::unique_ptr<PtJson> params = PtJson::CreateObject();
    std::unique_ptr<PtJson> locations = PtJson::CreateObject();
    std::unique_ptr<PtJson> vector = PtJson::CreateArray();

    std::unique_ptr<PtJson> bp = PtJson::CreateObject();
    bp->Add("lineNumber", breakPointInfoList_.back().lineNumber);
    bp->Add("columnNumber", breakPointInfoList_.back().columnNumber);
    vector->Push(bp);
    locations->Add(breakPointInfoList_.back().url.c_str(), vector);
    params->Add("locations", locations);
    request->Add("params", params);

    std::string message = request->Stringify();
    if (session->ClientSendReq(message)) {
        session->GetDomainManager().SetDomainById(id, "Debugger");
    }

    return 0;
}

int DebuggerClient::SetSymbolicBreakpointsCommand()
{
    Session *session = SessionManager::getInstance().GetSessionById(sessionId_);
    if (session == nullptr) {
        LOGE("get session by id %{public}u failed", sessionId_);
        return -1;
    }
    uint32_t id = session->GetMessageId();

    std::unique_ptr<PtJson> request = PtJson::CreateObject();
    request->Add("id", id);
    request->Add("method", "Debugger.setSymbolicBreakpoints");

    std::unique_ptr<PtJson> params = PtJson::CreateObject();
    std::unique_ptr<PtJson> symbolicBreakpoints = PtJson::CreateArray();

    std::unique_ptr<PtJson> symbolicBreakpoint = PtJson::CreateObject();
    symbolicBreakpoint->Add("functionName", symbolicBreakpointInfoList_.back().functionName.c_str());
    symbolicBreakpoints->Push(symbolicBreakpoint);
    params->Add("symbolicBreakpoints", symbolicBreakpoints);
    request->Add("params", params);

    std::string message = request->Stringify();
    if (session->ClientSendReq(message)) {
        session->GetDomainManager().SetDomainById(id, "Debugger");
    }

    return 0;
}

int DebuggerClient::RemoveSymbolicBreakpointsCommand()
{
    Session *session = SessionManager::getInstance().GetSessionById(sessionId_);
    if (session == nullptr) {
        LOGE("get session by id %{public}u failed", sessionId_);
        return -1;
    }
    uint32_t id = session->GetMessageId();

    std::unique_ptr<PtJson> request = PtJson::CreateObject();
    request->Add("id", id);
    request->Add("method", "Debugger.removeSymbolicBreakpoints");

    std::unique_ptr<PtJson> params = PtJson::CreateObject();
    std::unique_ptr<PtJson> symbolicBreakpoints = PtJson::CreateArray();

    std::unique_ptr<PtJson> symbolicBreakpoint = PtJson::CreateObject();
    symbolicBreakpoint->Add("functionName", symbolicBreakpointInfoList_.back().functionName.c_str());
    symbolicBreakpoints->Push(symbolicBreakpoint);
    params->Add("symbolicBreakpoints", symbolicBreakpoints);
    request->Add("params", params);

    std::string message = request->Stringify();
    if (session->ClientSendReq(message)) {
        session->GetDomainManager().SetDomainById(id, "Debugger");
    }

    return 0;
}

int DebuggerClient::RemoveBreakpointsByUrlCommand()
{
    Session *session = SessionManager::getInstance().GetSessionById(sessionId_);
    if (session == nullptr) {
        LOGE("get session by id %{public}u failed", sessionId_);
        return -1;
    }
    uint32_t id = session->GetMessageId();

    std::unique_ptr<PtJson> request = PtJson::CreateObject();
    request->Add("id", id);
    request->Add("method", "Debugger.removeBreakpointsByUrl");

    std::unique_ptr<PtJson> params = PtJson::CreateObject();
    params->Add("url", urlList_.back().c_str());
    request->Add("params", params);

    std::string message = request->Stringify();
    if (session->ClientSendReq(message)) {
        session->GetDomainManager().SetDomainById(id, "Debugger");
    }

    return 0;
}

int DebuggerClient::EnableLaunchAccelerateCommand()
{
    Session *session = SessionManager::getInstance().GetSessionById(sessionId_);
    if (session == nullptr) {
        LOGE("get session by id %{public}u failed", sessionId_);
        return -1;
    }
    uint32_t id = session->GetMessageId();

    std::unique_ptr<PtJson> request = PtJson::CreateObject();
    request->Add("id", id);
    request->Add("method", "Debugger.enable");

    std::unique_ptr<PtJson> params = PtJson::CreateObject();
    std::unique_ptr<PtJson> options = PtJson::CreateArray();

    options->Push("enableLaunchAccelerate");
    params->Add("options", options);
    request->Add("params", params);

    std::string message = request->Stringify();
    if (session->ClientSendReq(message)) {
        session->GetDomainManager().SetDomainById(id, "Debugger");
    }

    return 0;
}
} // OHOS::ArkCompiler::Toolchain