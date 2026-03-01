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

#ifndef ECMASCRIPT_TOOLING_TEST_TESTCASES_JS_STEPOUT_ARROW_TEST_H
#define ECMASCRIPT_TOOLING_TEST_TESTCASES_JS_STEPOUT_ARROW_TEST_H

#include "tooling/dynamic/test/client_utils/test_util.h"

namespace panda::ecmascript::tooling::test {
class JsStepoutArrowTest : public TestActions {
public:
    JsStepoutArrowTest()
    {
        testAction = {
            {SocketAction::SEND, "enable"},
            {SocketAction::RECV, "", ActionRule::CUSTOM_RULE, MatchRule::replySuccess},
            {SocketAction::SEND, "runtime-enable"},
            {SocketAction::RECV, "", ActionRule::CUSTOM_RULE, MatchRule::replySuccess},
            {SocketAction::SEND, "run"},
            {SocketAction::RECV, "", ActionRule::CUSTOM_RULE, MatchRule::replySuccess},
            // load arrow_func.js
            {SocketAction::RECV, "Debugger.scriptParsed", ActionRule::STRING_CONTAIN},
            // break on start
            {SocketAction::RECV, "Debugger.paused", ActionRule::CUSTOM_RULE,
                [](auto recv, auto, auto) -> bool {
                    std::unique_ptr<PtJson> json = PtJson::Parse(recv);
                    DebuggerClient debuggerClient(0);
                    debuggerClient.RecvReply(std::move(json));
                    return true;
                }},
            // set first breakpoint
            {SocketAction::SEND, "b " DEBUGGER_JS_DIR "arrow_func.js 19"},
            {SocketAction::RECV, "", ActionRule::CUSTOM_RULE, MatchRule::replySuccess},

            // hit breakpoint after resume first time
            {SocketAction::SEND, "resume"},
            {SocketAction::RECV, "Debugger.resumed", ActionRule::STRING_CONTAIN},
            {SocketAction::RECV, "", ActionRule::CUSTOM_RULE, MatchRule::replySuccess},
            {SocketAction::RECV, "Debugger.paused", ActionRule::CUSTOM_RULE,
                [this](auto recv, auto, auto) -> bool { return RecvBreakInfo(recv); }},

            {SocketAction::SEND, "watch num1"},
            {SocketAction::RECV, "Debugger.paused", ActionRule::CUSTOM_RULE,
                [this](auto recv, auto, auto) -> bool { return RecvWatchInfo(recv, "2"); }},
            {SocketAction::SEND, "watch num2"},
            {SocketAction::RECV, "Debugger.paused", ActionRule::CUSTOM_RULE,
                [this](auto recv, auto, auto) -> bool { return RecvWatchInfo(recv, "2"); }},

            {SocketAction::SEND, "so"},
            {SocketAction::RECV, "Debugger.resumed", ActionRule::STRING_CONTAIN},
            {SocketAction::RECV, "", ActionRule::CUSTOM_RULE, MatchRule::replySuccess},
            {SocketAction::RECV, "Debugger.paused", ActionRule::CUSTOM_RULE,
                [this](auto recv, auto, auto) -> bool { return RecvStepoutInfo(recv, "func_main_0", 22); }},
            {SocketAction::RECV, "", ActionRule::CUSTOM_RULE, MatchRule::replySuccess},

            {SocketAction::SEND, "resume"},
            {SocketAction::RECV, "", ActionRule::CUSTOM_RULE, MatchRule::replySuccess},

            {SocketAction::RECV, "Debugger.resumed", ActionRule::STRING_CONTAIN},
            {SocketAction::RECV, "", ActionRule::CUSTOM_RULE, MatchRule::replySuccess},
            {SocketAction::RECV, "Debugger.paused", ActionRule::STRING_CONTAIN},

            // reply success and run
            {SocketAction::SEND, "success"},
            {SocketAction::SEND, "resume"},
            {SocketAction::RECV, "Debugger.resumed", ActionRule::STRING_CONTAIN},
            {SocketAction::RECV, "", ActionRule::CUSTOM_RULE, MatchRule::replySuccess},
        };
    }

    bool RecvBreakInfo(std::string recv)
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

        std::unique_ptr<PtJson> hitBreakpoints = nullptr;
        ret = params->GetArray("hitBreakpoints", &hitBreakpoints);
        if (ret != Result::SUCCESS) {
            return false;
        }

        std::string breakpoint = "";
        breakpoint = hitBreakpoints->Get(0)->GetString();
        if (ret != Result::SUCCESS || breakpoint.find(sourceFile_) == std::string::npos ||
            breakpoint.find("18") == std::string::npos) {
            return false;
        }

        DebuggerClient debuggerClient(0);
        debuggerClient.PausedReply(std::move(json));
        return true;
    }

    bool RecvStepoutInfo(std::string recv, std::string funcName, int lineNumber)
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

    bool RecvWatchInfo(std::string recv, std::string var_value)
    {
        std::unique_ptr<PtJson> json = PtJson::Parse(recv);
        Result ret;
        int id = 0;
        ret = json->GetInt("id", &id);
        if (ret != Result::SUCCESS) {
            return false;
        }

        std::unique_ptr<PtJson> result = nullptr;
        ret = json->GetObject("result", &result);
        if (ret != Result::SUCCESS) {
            return false;
        }

        std::unique_ptr<PtJson> watchResult = nullptr;
        ret = result->GetObject("result", &watchResult);
        if (ret != Result::SUCCESS) {
            return false;
        }

        std::string type = "";
        ret = watchResult->GetString("type", &type);
        if (ret != Result::SUCCESS || type != "number") {
            return false;
        }

        std::string value = "";
        ret = watchResult->GetString("unserializableValue", &value);
        if (ret != Result::SUCCESS || value != var_value) {
            return false;
        }
        return true;
    }

    std::pair<std::string, std::string> GetEntryPoint() override
    {
        return {pandaFile_, entryPoint_};
    }
    ~JsStepoutArrowTest() = default;

private:
    std::string pandaFile_ = DEBUGGER_ABC_DIR "arrow_func.abc";
    std::string sourceFile_ = DEBUGGER_JS_DIR "arrow_func.js";
    std::string entryPoint_ = "arrow_func";
};

std::unique_ptr<TestActions> GetJsStepoutArrowTest()
{
    return std::make_unique<JsStepoutArrowTest>();
}
}  // namespace panda::ecmascript::tooling::test

#endif // ECMASCRIPT_TOOLING_TEST_TESTCASES_JS_STEPOUT_ARROW_TEST_H
