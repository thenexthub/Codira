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

#ifndef ECMASCRIPT_TOOLING_TEST_TESTCASES_JS_WATCH_VARIABLE_TEST_H
#define ECMASCRIPT_TOOLING_TEST_TESTCASES_JS_WATCH_VARIABLE_TEST_H

#include "tooling/dynamic/test/client_utils/test_util.h"

namespace panda::ecmascript::tooling::test {
class JsWatchVariableTest : public TestActions {
public:
    JsWatchVariableTest()
    {
        testAction = {
            {SocketAction::SEND, "enable"},
            {SocketAction::RECV, "", ActionRule::CUSTOM_RULE, MatchRule::replySuccess},
            {SocketAction::SEND, "runtime-enable"},
            {SocketAction::RECV, "", ActionRule::CUSTOM_RULE, MatchRule::replySuccess},
            {SocketAction::SEND, "run"},
            {SocketAction::RECV, "", ActionRule::CUSTOM_RULE, MatchRule::replySuccess},
            // load step.js
            {SocketAction::RECV, "Debugger.scriptParsed", ActionRule::STRING_CONTAIN},
            // break on start
            {SocketAction::RECV, "Debugger.paused", ActionRule::CUSTOM_RULE,
                [](auto recv, auto, auto) -> bool {
                    std::unique_ptr<PtJson> json = PtJson::Parse(recv);
                    DebuggerClient debuggerClient(0);
                    debuggerClient.RecvReply(std::move(json));
                    return true;
                }},
            // set breakpoint
            {SocketAction::SEND, "b " DEBUGGER_JS_DIR "step.js 26"},
            {SocketAction::RECV, "", ActionRule::CUSTOM_RULE, MatchRule::replySuccess},
            {SocketAction::SEND, "b " DEBUGGER_JS_DIR "step.js 31"},
            {SocketAction::RECV, "", ActionRule::CUSTOM_RULE, MatchRule::replySuccess},
            {SocketAction::SEND, "b " DEBUGGER_JS_DIR "step.js 38"},
            {SocketAction::RECV, "", ActionRule::CUSTOM_RULE, MatchRule::replySuccess},
            {SocketAction::SEND, "b " DEBUGGER_JS_DIR "step.js 52"},
            {SocketAction::RECV, "", ActionRule::CUSTOM_RULE, MatchRule::replySuccess},
            {SocketAction::SEND, "b " DEBUGGER_JS_DIR "step.js 62"},
            {SocketAction::RECV, "", ActionRule::CUSTOM_RULE, MatchRule::replySuccess},
            {SocketAction::SEND, "b " DEBUGGER_JS_DIR "step.js 64"},
            {SocketAction::RECV, "", ActionRule::CUSTOM_RULE, MatchRule::replySuccess},
            {SocketAction::SEND, "b " DEBUGGER_JS_DIR "step.js 77"},
            {SocketAction::RECV, "", ActionRule::CUSTOM_RULE, MatchRule::replySuccess},
            {SocketAction::SEND, "b " DEBUGGER_JS_DIR "step.js 87"},
            {SocketAction::RECV, "", ActionRule::CUSTOM_RULE, MatchRule::replySuccess},
            {SocketAction::SEND, "b " DEBUGGER_JS_DIR "step.js 98"},
            {SocketAction::RECV, "", ActionRule::CUSTOM_RULE, MatchRule::replySuccess},
            // hit breakpoint after resume first time
            {SocketAction::SEND, "resume"},
            {SocketAction::RECV, "Debugger.resumed", ActionRule::STRING_CONTAIN},
            {SocketAction::RECV, "", ActionRule::CUSTOM_RULE, MatchRule::replySuccess},
            {SocketAction::RECV, "Debugger.paused", ActionRule::STRING_CONTAIN},
            {SocketAction::SEND, "watch b"}, // b = 3
            {SocketAction::RECV, "", ActionRule::CUSTOM_RULE,
                [this](auto recv, auto, auto) -> bool { return RecvWatchVariableInfo(recv, "3"); }},
            // hit breakpoint after resume second time
            {SocketAction::SEND, "resume"},
            {SocketAction::RECV, "Debugger.resumed", ActionRule::STRING_CONTAIN},
            {SocketAction::RECV, "", ActionRule::CUSTOM_RULE, MatchRule::replySuccess},
            {SocketAction::RECV, "Debugger.paused", ActionRule::STRING_CONTAIN},
            {SocketAction::SEND, "watch a"}, // a = 2
            {SocketAction::RECV, "", ActionRule::CUSTOM_RULE,
                [this](auto recv, auto, auto) -> bool { return RecvWatchVariableInfo(recv, "2"); }},

            {SocketAction::SEND, "resume"},
            {SocketAction::RECV, "Debugger.resumed", ActionRule::STRING_CONTAIN},
            {SocketAction::RECV, "", ActionRule::CUSTOM_RULE, MatchRule::replySuccess},
            {SocketAction::RECV, "Debugger.paused", ActionRule::STRING_CONTAIN},
            {SocketAction::SEND, "watch c"}, // c = 2
            {SocketAction::RECV, "", ActionRule::CUSTOM_RULE,
                [this](auto recv, auto, auto) -> bool { return RecvWatchVariableInfo(recv, "2"); }},

            {SocketAction::SEND, "resume"},
            {SocketAction::RECV, "Debugger.resumed", ActionRule::STRING_CONTAIN},
            {SocketAction::RECV, "", ActionRule::CUSTOM_RULE, MatchRule::replySuccess},
            {SocketAction::RECV, "Debugger.paused", ActionRule::STRING_CONTAIN},
            {SocketAction::SEND, "watch a"}, // a = 6
            {SocketAction::RECV, "", ActionRule::CUSTOM_RULE,
                [this](auto recv, auto, auto) -> bool { return RecvWatchVariableInfo(recv, "6"); }},

            {SocketAction::SEND, "resume"},
            {SocketAction::RECV, "Debugger.resumed", ActionRule::STRING_CONTAIN},
            {SocketAction::RECV, "", ActionRule::CUSTOM_RULE, MatchRule::replySuccess},
            {SocketAction::RECV, "Debugger.paused", ActionRule::STRING_CONTAIN},
            {SocketAction::SEND, "watch c"}, // c = 1
            {SocketAction::RECV, "", ActionRule::CUSTOM_RULE,
                [this](auto recv, auto, auto) -> bool { return RecvWatchVariableInfo(recv, "1"); }},

            {SocketAction::SEND, "resume"},
            {SocketAction::RECV, "Debugger.resumed", ActionRule::STRING_CONTAIN},
            {SocketAction::RECV, "", ActionRule::CUSTOM_RULE, MatchRule::replySuccess},
            {SocketAction::RECV, "Debugger.paused", ActionRule::STRING_CONTAIN},
            {SocketAction::SEND, "watch d"}, // d = 1
            {SocketAction::RECV, "", ActionRule::CUSTOM_RULE,
                [this](auto recv, auto, auto) -> bool { return RecvWatchVariableInfo(recv, "1"); }},

            {SocketAction::SEND, "resume"},
            {SocketAction::RECV, "Debugger.resumed", ActionRule::STRING_CONTAIN},
            {SocketAction::RECV, "", ActionRule::CUSTOM_RULE, MatchRule::replySuccess},
            {SocketAction::RECV, "Debugger.paused", ActionRule::STRING_CONTAIN},
            {SocketAction::SEND, "watch a"}, // a = 23
            {SocketAction::RECV, "", ActionRule::CUSTOM_RULE,
                [this](auto recv, auto, auto) -> bool { return RecvWatchVariableInfo(recv, "23"); }},

            {SocketAction::SEND, "resume"},
            {SocketAction::RECV, "Debugger.resumed", ActionRule::STRING_CONTAIN},
            {SocketAction::RECV, "", ActionRule::CUSTOM_RULE, MatchRule::replySuccess},
            {SocketAction::RECV, "Debugger.paused", ActionRule::STRING_CONTAIN},
            {SocketAction::SEND, "watch a"}, // a = 100
            {SocketAction::RECV, "", ActionRule::CUSTOM_RULE,
                [this](auto recv, auto, auto) -> bool { return RecvWatchVariableInfo(recv, "100"); }},

            {SocketAction::SEND, "resume"},
            {SocketAction::RECV, "Debugger.resumed", ActionRule::STRING_CONTAIN},
            {SocketAction::RECV, "", ActionRule::CUSTOM_RULE, MatchRule::replySuccess},
            {SocketAction::RECV, "Debugger.paused", ActionRule::STRING_CONTAIN},
            {SocketAction::SEND, "watch b"}, // b = 13
            {SocketAction::RECV, "", ActionRule::CUSTOM_RULE,
                [this](auto recv, auto, auto) -> bool { return RecvWatchVariableInfo(recv, "13"); }},

            // reply success and run
            {SocketAction::SEND, "success"},
            {SocketAction::SEND, "resume"},
            {SocketAction::RECV, "Debugger.resumed", ActionRule::STRING_CONTAIN},
            {SocketAction::RECV, "", ActionRule::CUSTOM_RULE, MatchRule::replySuccess},
        };
    }

    bool RecvWatchVariableInfo(std::string recv, std::string var_value)
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
    ~JsWatchVariableTest() = default;

private:
    std::string pandaFile_ = DEBUGGER_ABC_DIR "step.abc";
    std::string sourceFile_ = DEBUGGER_JS_DIR "step.js";
    std::string entryPoint_ = "step";
};

std::unique_ptr<TestActions> GetJsWatchVariableTest()
{
    return std::make_unique<JsWatchVariableTest>();
}
}  // namespace panda::ecmascript::tooling::test

#endif // ECMASCRIPT_TOOLING_TEST_TESTCASES_JS_WATCH_VARIABLE_TEST_H
