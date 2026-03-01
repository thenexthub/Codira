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

#ifndef ECMASCRIPT_TOOLING_TEST_TESTCASES_JS_SYMBOLICBREAKPOINT_LOOP_TEST_H
#define ECMASCRIPT_TOOLING_TEST_TESTCASES_JS_SYMBOLICBREAKPOINT_LOOP_TEST_H

#include "tooling/dynamic/test/client_utils/test_util.h"

namespace panda::ecmascript::tooling::test {
class JsSymbolicBreakpointTest : public TestActions {
public:
    JsSymbolicBreakpointTest()
    {
        testAction = {
            {SocketAction::SEND, "enable"},
            {SocketAction::RECV, "", ActionRule::CUSTOM_RULE, MatchRule::replySuccess},
            {SocketAction::SEND, "runtime-enable"},
            {SocketAction::RECV, "", ActionRule::CUSTOM_RULE, MatchRule::replySuccess},
            {SocketAction::SEND, "run"},
            {SocketAction::RECV, "", ActionRule::CUSTOM_RULE, MatchRule::replySuccess},
            // load common_func.js
            {SocketAction::RECV, "Debugger.scriptParsed", ActionRule::STRING_CONTAIN},
            // break on start
            {SocketAction::RECV, "Debugger.paused", ActionRule::STRING_CONTAIN},
            // set symbol breakpoints
            {SocketAction::SEND, "setSymbolicBreakpoints " "common_func"},
            {SocketAction::RECV, "", ActionRule::CUSTOM_RULE, MatchRule::replySuccess},
            {SocketAction::SEND, "setSymbolicBreakpoints " "for_loop"},
            {SocketAction::RECV, "", ActionRule::CUSTOM_RULE, MatchRule::replySuccess},

            // hit the common_func function
            {SocketAction::SEND, "resume"},
            {SocketAction::RECV, "Debugger.resumed", ActionRule::STRING_CONTAIN},
            {SocketAction::RECV, "", ActionRule::CUSTOM_RULE, MatchRule::replySuccess},
            {SocketAction::RECV, "Debugger.paused", ActionRule::CUSTOM_RULE,
                [this](auto recv, auto, auto) -> bool { return RecvBreakInfo(recv, "common_func", 16); }},

            // set symbol breakpoints
            {SocketAction::SEND, "setSymbolicBreakpoints " "while_loop"},
            {SocketAction::RECV, "", ActionRule::CUSTOM_RULE, MatchRule::replySuccess},

            // hit the for_loop function
            {SocketAction::SEND, "resume"},
            {SocketAction::RECV, "Debugger.resumed", ActionRule::STRING_CONTAIN},
            {SocketAction::RECV, "", ActionRule::CUSTOM_RULE, MatchRule::replySuccess},
            {SocketAction::RECV, "Debugger.paused", ActionRule::CUSTOM_RULE,
                [this](auto recv, auto, auto) -> bool { return RecvBreakInfo(recv, "for_loop", 26); }},

            // set symbol breakpoints
            {SocketAction::SEND, "setSymbolicBreakpoints " "while_loop"},
            {SocketAction::RECV, "", ActionRule::CUSTOM_RULE, MatchRule::replySuccess},

            // hit the while_loop function
            {SocketAction::SEND, "resume"},
            {SocketAction::RECV, "Debugger.resumed", ActionRule::STRING_CONTAIN},
            {SocketAction::RECV, "", ActionRule::CUSTOM_RULE, MatchRule::replySuccess},
            {SocketAction::RECV, "Debugger.paused", ActionRule::CUSTOM_RULE,
                [this](auto recv, auto, auto) -> bool { return RecvBreakInfo(recv, "while_loop", 34); }},
            
            // set symbol breakpoints
            {SocketAction::SEND, "setSymbolicBreakpoints " "loop_switch"},
            {SocketAction::RECV, "", ActionRule::CUSTOM_RULE, MatchRule::replySuccess},

            // hit the loop_switch function
            {SocketAction::SEND, "resume"},
            {SocketAction::RECV, "Debugger.resumed", ActionRule::STRING_CONTAIN},
            {SocketAction::RECV, "", ActionRule::CUSTOM_RULE, MatchRule::replySuccess},
            {SocketAction::RECV, "Debugger.paused", ActionRule::STRING_CONTAIN},

            // set symbol breakpoints
            {SocketAction::SEND, "setSymbolicBreakpoints " "factorial"},
            {SocketAction::RECV, "", ActionRule::CUSTOM_RULE, MatchRule::replySuccess},

            // firstly hit the factorial function
            {SocketAction::SEND, "resume"},
            {SocketAction::RECV, "Debugger.resumed", ActionRule::STRING_CONTAIN},
            {SocketAction::RECV, "", ActionRule::CUSTOM_RULE, MatchRule::replySuccess},
            {SocketAction::RECV, "Debugger.paused", ActionRule::CUSTOM_RULE,
                [this](auto recv, auto, auto) -> bool { return RecvBreakInfo(recv, "factorial", 57); }},

            // remove symbol breakpoints
            {SocketAction::SEND, "removeSymbolicBreakpoints " "loop_switch"},
            {SocketAction::RECV, "", ActionRule::CUSTOM_RULE, MatchRule::replySuccess},

            // secondly hit the factorial function
            {SocketAction::SEND, "resume"},
            {SocketAction::RECV, "Debugger.resumed", ActionRule::STRING_CONTAIN},
            {SocketAction::RECV, "", ActionRule::CUSTOM_RULE, MatchRule::replySuccess},
            {SocketAction::RECV, "Debugger.paused", ActionRule::CUSTOM_RULE,
                [this](auto recv, auto, auto) -> bool { return RecvBreakInfo(recv, "factorial", 57); }},

            // remove symbol breakpoints
            {SocketAction::SEND, "removeSymbolicBreakpoints " "factorial"},
            {SocketAction::RECV, "", ActionRule::CUSTOM_RULE, MatchRule::replySuccess},

            // reply success and run
            {SocketAction::SEND, "success"},
            {SocketAction::SEND, "resume"},
            {SocketAction::RECV, "Debugger.resumed", ActionRule::STRING_CONTAIN},
            {SocketAction::RECV, "", ActionRule::CUSTOM_RULE, MatchRule::replySuccess},
        };
    }

    bool RecvBreakInfo(std::string recv, std::string funcName, int lineNumber)
    {
        std::unique_ptr<PtJson> json = PtJson::Parse(recv);
        Result ret;
        std::string method = "";
        ret = json->GetString("method", &method);
        if (ret != Result::SUCCESS || method != "Debugger.paused") {
            return false;
        }

        std::unique_ptr<PtJson> params = nullptr;
        ret = json->GetObject("params", &params);
        if (ret != Result::SUCCESS) {
            return false;
        }

        std::unique_ptr<PtJson> callFrames = nullptr;
        ret = params->GetArray("callFrames", &callFrames);
        if (ret != Result::SUCCESS) {
            return false;
        }

        std::string functionName = "";
        ret = callFrames->Get(0)->GetString("functionName", &functionName);
        if (ret != Result::SUCCESS || functionName != funcName) {
            return false;
        }

        std::unique_ptr<PtJson> location = nullptr;
        ret = callFrames->Get(0)->GetObject("location", &location);
        if (ret != Result::SUCCESS) {
            return false;
        }

        int lineNum = 0;
        ret = location->GetInt("lineNumber", &lineNum);
        if (ret != Result::SUCCESS || lineNum != lineNumber) {
            return false;
        }

        DebuggerClient debuggerClient(0);
        debuggerClient.PausedReply(std::move(json));
        return true;
    }

    std::pair<std::string, std::string> GetEntryPoint() override
    {
        return {pandaFile_, entryPoint_};
    }
    ~JsSymbolicBreakpointTest() = default;

private:
    std::string pandaFile_ = DEBUGGER_ABC_DIR "common_func.abc";
    std::string sourceFile_ = DEBUGGER_JS_DIR "common_func.js";
    std::string entryPoint_ = "common_func";
};

std::unique_ptr<TestActions> GetJsSymbolicBreakpointTest()
{
    return std::make_unique<JsSymbolicBreakpointTest>();
}
}  // namespace panda::ecmascript::tooling::test

#endif // ECMASCRIPT_TOOLING_TEST_TESTCASES_JS_SYMBOLICBREAKPOINT_LOOP_TEST_H