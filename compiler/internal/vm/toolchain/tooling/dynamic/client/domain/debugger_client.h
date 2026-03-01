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

#ifndef ECMASCRIPT_TOOLING_CLIENT_DOMAIN_DEBUGGER_CLIENT_H
#define ECMASCRIPT_TOOLING_CLIENT_DOMAIN_DEBUGGER_CLIENT_H

#include <iostream>
#include <vector>

#include "tooling/dynamic/base/pt_json.h"

using PtJson = panda::ecmascript::tooling::PtJson;
using Result = panda::ecmascript::tooling::Result;
namespace OHOS::ArkCompiler::Toolchain {
struct BreakPointInfo {
    int lineNumber;
    int columnNumber;
    std::string url;
};

struct SymbolicBreakpointInfo {
    std::string functionName;
};

class DebuggerClient final {
public:
    DebuggerClient(int32_t sessionId) : sessionId_(sessionId) {}
    ~DebuggerClient() = default;

    bool DispatcherCmd(const std::string &cmd);
    int BreakCommand();
    int BacktrackCommand();
    int DeleteCommand();
    int DisableCommand();
    int DisplayCommand();
    int EnableCommand();
    int FinishCommand();
    int FrameCommand();
    int IgnoreCommand();
    int InfobreakpointsCommand();
    int InfosourceCommand();
    int JumpCommand();
    int NextCommand();
    int ListCommand();
    int PtypeCommand();
    int RunCommand();
    int SetvarCommand();
    int StepCommand();
    int UndisplayCommand();
    int WatchCommand();
    int ResumeCommand();
    int StepIntoCommand();
    int StepOutCommand();
    int StepOverCommand();
    int EnableLaunchAccelerateCommand();
    int SaveAllPossibleBreakpointsCommand();
    int AsyncStackDepthCommand();
    int SetSymbolicBreakpointsCommand();
    int RemoveSymbolicBreakpointsCommand();
    int RemoveBreakpointsByUrlCommand();
    void AddBreakPointInfo(const std::string& url, const int& lineNumber, const int& columnNumber = 0);
    void AddSymbolicBreakpointInfo(const std::string& functionName);
    void AddUrl(const std::string& url);
    void RecvReply(std::unique_ptr<PtJson> json);
    void PausedReply(const std::unique_ptr<PtJson> json);
    void HandleResponse(std::unique_ptr<PtJson> json);

private:
    std::vector<std::string> urlList_ {};
    std::vector<BreakPointInfo> breakPointInfoList_ {};
    std::vector<SymbolicBreakpointInfo> symbolicBreakpointInfoList_ {};
    int32_t sessionId_;
};
} // OHOS::ArkCompiler::Toolchain
#endif