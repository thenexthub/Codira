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

#ifndef ECMASCRIPT_TOOLING_TEST_TESTCASES_JS_WATCH_OTHER_TYPE_TEST_H
#define ECMASCRIPT_TOOLING_TEST_TESTCASES_JS_WATCH_OTHER_TYPE_TEST_H

#include "tooling/dynamic/test/client_utils/test_util.h"

namespace panda::ecmascript::tooling::test {
class JsWatchOtherTypeTest : public TestActions {
public:
    JsWatchOtherTypeTest()
    {
        testAction = {
            {SocketAction::SEND, "enable"},
            {SocketAction::RECV, "", ActionRule::CUSTOM_RULE, MatchRule::replySuccess},
            {SocketAction::SEND, "runtime-enable"},
            {SocketAction::RECV, "", ActionRule::CUSTOM_RULE, MatchRule::replySuccess},
            {SocketAction::SEND, "run"},
            {SocketAction::RECV, "", ActionRule::CUSTOM_RULE, MatchRule::replySuccess},
            // load watch_variable.js
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
            {SocketAction::SEND, "b " DEBUGGER_JS_DIR "watch_variable.js 327"},
            {SocketAction::RECV, "", ActionRule::CUSTOM_RULE, MatchRule::replySuccess},

            // hit breakpoint after resume first time
            {SocketAction::SEND, "resume"},
            {SocketAction::RECV, "Debugger.resumed", ActionRule::STRING_CONTAIN},
            {SocketAction::RECV, "", ActionRule::CUSTOM_RULE, MatchRule::replySuccess},
            {SocketAction::RECV, "Debugger.paused", ActionRule::STRING_CONTAIN},

            // watch
            {SocketAction::SEND, "watch generator0"},
            {SocketAction::RECV, "", ActionRule::CUSTOM_RULE,
                [this](auto recv, auto, auto) -> bool {
                    return RecvWatchFunctionInfo(recv, "Generator", "function* generator0( { [js code] }");
                }},
            {SocketAction::SEND, "watch regexp0"},
            {SocketAction::RECV, "", ActionRule::CUSTOM_RULE,
                [this](auto recv, auto, auto) -> bool {
                    return RecvWatchSubtypeInfo(recv, "regexp", "RegExp", "/^\\d+\\.\\d+$/i");
                }},
            {SocketAction::RECV, "", ActionRule::CUSTOM_RULE, MatchRule::replySuccess},
            {SocketAction::SEND, "watch iterator0"},
            {SocketAction::RECV, "", ActionRule::CUSTOM_RULE,
                [this](auto recv, auto, auto) -> bool {
                    return RecvWatchFunctionInfo(recv, "Function",
                                                "function [Symbol.iterator]( { [native code] }");
                }},

            {SocketAction::SEND, "resume"},
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

    bool RecvWatchSubtypeInfo(std::string recv, std::string var_type, std::string var_class,
                              std::string var_info)
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
        if (ret != Result::SUCCESS || type != "object") {
            return false;
        }

        std::string subtype = "";
        ret = watchResult->GetString("subtype", &subtype);
        if (ret != Result::SUCCESS || subtype != var_type) {
            return false;
        }

        std::string className = "";
        ret = watchResult->GetString("className", &className);
        if (ret != Result::SUCCESS || className != var_class) {
            return false;
        }

        std::string value = "";
        ret = watchResult->GetString("unserializableValue", &value);
        if (ret != Result::SUCCESS || value != var_info) {
            return false;
        }

        std::string description = "";
        ret = watchResult->GetString("description", &description);
        if (ret != Result::SUCCESS || description != var_info) {
            return false;
        }

        DebuggerClient debuggerClient(0);
        debuggerClient.RecvReply(std::move(json));
        return true;
    }

    bool RecvWatchFunctionInfo(std::string recv, std::string func_class, std::string func_info)
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
        if (ret != Result::SUCCESS || type != "function") {
            return false;
        }

        std::string className = "";
        ret = watchResult->GetString("className", &className);
        if (ret != Result::SUCCESS || className != func_class) {
            return false;
        }

        std::string value = "";
        ret = watchResult->GetString("unserializableValue", &value);
        if (ret != Result::SUCCESS || value != func_info) {
            return false;
        }

        std::string description = "";
        ret = watchResult->GetString("description", &description);
        if (ret != Result::SUCCESS || description != func_info) {
            return false;
        }

        DebuggerClient debuggerClient(0);
        debuggerClient.RecvReply(std::move(json));
        return true;
    }

    std::pair<std::string, std::string> GetEntryPoint() override
    {
        return {pandaFile_, entryPoint_};
    }
    ~JsWatchOtherTypeTest() = default;

private:
    std::string pandaFile_ = DEBUGGER_ABC_DIR "watch_variable.abc";
    std::string sourceFile_ = DEBUGGER_JS_DIR "watch_variable.js";
    std::string entryPoint_ = "watch_variable";
};

std::unique_ptr<TestActions> GetJsWatchOtherTypeTest()
{
    return std::make_unique<JsWatchOtherTypeTest>();
}
}  // namespace panda::ecmascript::tooling::test

#endif // ECMASCRIPT_TOOLING_TEST_TESTCASES_JS_WATCH_OTHER_TYPE_TEST_H
