/*
 * Copyright (c) NeXTHub Corporation. All Rights Reserved.
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Author: Tunjay Akbarli
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at:
 *
 * http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 *
 * Please contact NeXTHub Corporation, 651 N Broad St, Suite 201,
 * Middletown, DE 19709, New Castle County, USA.
 */

#ifndef ECMASCRIPT_TOOLING_CLIENT_ARK_CLI_CLI_COMMAND_H
#define ECMASCRIPT_TOOLING_CLIENT_ARK_CLI_CLI_COMMAND_H

#include <cstdlib>
#include <functional>
#include <map>
#include <vector>

#include "common/log_wrapper.h"
#include "tooling/dynamic/client/domain/heapprofiler_client.h"
#include "tooling/dynamic/client/domain/profiler_client.h"
#include "tooling/dynamic/client/manager/domain_manager.h"
#include "tooling/dynamic/utils/utils.h"
#include "tooling/dynamic/client/session/session.h"

namespace OHOS::ArkCompiler::Toolchain {
using StrPair = std::pair<std::string, std::string>;
using VecStr = std::vector<std::string>;

enum class ErrCode : uint8_t {
    ERR_OK   = 0,
    ERR_FAIL = 1
};

class CliCommand {
public:
    CliCommand(std::vector<std::string> cliCmdStr, uint32_t sessionId)
        : cmd_(cliCmdStr[0]), sessionId_(sessionId)
    {
        for (size_t i = 1u; i < cliCmdStr.size(); i++) {
            argList_.push_back(cliCmdStr[i]);
        }
    }

    ~CliCommand() = default;

    ErrCode OnCommand();
    ErrCode ExecCommand();
    void CreateCommandMap();
    void CreateOtherCommandMap();
    ErrCode HeapProfilerCommand(const std::string &cmd);
    ErrCode DebuggerCommand(const std::string &cmd);
    ErrCode CpuProfileCommand(const std::string &cmd);
    ErrCode RuntimeCommand(const std::string &cmd);
    ErrCode BreakCommand(const std::string &cmd);
    ErrCode DeleteCommand(const std::string &cmd);
    ErrCode StepCommand(const std::string &cmd);
    ErrCode ShowstackCommand(const std::string &cmd);
    ErrCode DisplayCommand(const std::string &cmd);
    ErrCode InfosourceCommand(const std::string &cmd);
    ErrCode ListCommand(const std::string &cmd);
    ErrCode PrintCommand(const std::string &cmd);
    ErrCode WatchCommand(const std::string &cmd);
    ErrCode SessionAddCommand(const std::string &cmd);
    ErrCode SessionDelCommand(const std::string &cmd);
    ErrCode SessionListCommand(const std::string &cmd);
    ErrCode SessionSwitchCommand(const std::string &cmd);
    ErrCode TestCommand(const std::string &cmd);
    ErrCode ExecHelpCommand();
    ErrCode SaveAllPossibleBreakpointsCommand(const std::string &cmd);
    ErrCode SetSymbolicBreakpointsCommand(const std::string &cmd);
    ErrCode RemoveSymbolicBreakpointsCommand(const std::string &cmd);
    ErrCode RemoveBreakpointsByUrlCommand(const std::string &cmd);
    void OutputCommand(const std::string &cmd, bool flag);

    VecStr GetArgList()
    {
        return argList_;
    }

private:
    std::string cmd_ ;
    VecStr argList_ {};
    std::map<StrPair, std::function<ErrCode()>> commandMap_;
    std::string resultReceiver_ = "";
    uint32_t sessionId_;
};
} // namespace OHOS::ArkCompiler::Toolchain

#endif // ECMASCRIPT_TOOLING_CLIENT_ARK_CLI_CLI_COMMAND_H