/**
 * Copyright (c) 2025-2026 Huawei Device Co., Ltd.
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

#ifndef ECMASCRIPT_TOOLING_TEST_TESTCASES_JS_WATCH_MODULE_TEST_H
#define ECMASCRIPT_TOOLING_TEST_TESTCASES_JS_WATCH_MODULE_TEST_H

#include "tooling/dynamic/test/client_utils/test_util.h"
#include "ecmascript/tests/test_helper.h"

namespace panda::ecmascript::tooling::test {
class JsWatchModuleTest : public TestActions {
public:
    JsWatchModuleTest()
    {
        testAction = {
            {SocketAction::SEND, "enable"},
            {SocketAction::RECV, "", ActionRule::CUSTOM_RULE, MatchRule::replySuccess},
            {SocketAction::SEND, "runtime-enable"},
            {SocketAction::RECV, "", ActionRule::CUSTOM_RULE, MatchRule::replySuccess},
            {SocketAction::SEND, "run"},
            {SocketAction::RECV, "", ActionRule::CUSTOM_RULE, MatchRule::replySuccess},
            // load local_export.js
            {SocketAction::RECV, "Debugger.scriptParsed", ActionRule::STRING_CONTAIN},
            {SocketAction::RECV, "Debugger.paused", ActionRule::STRING_CONTAIN},
            {SocketAction::SEND, "resume"},
            {SocketAction::RECV, "Debugger.resumed", ActionRule::STRING_CONTAIN},
            {SocketAction::RECV, "", ActionRule::CUSTOM_RULE, MatchRule::replySuccess},
            // load module_watch.js
            {SocketAction::RECV, "Debugger.scriptParsed", ActionRule::STRING_CONTAIN},
            // break on start
            {SocketAction::RECV, "Debugger.paused", ActionRule::STRING_CONTAIN},
            
            // set breakpoint
            {SocketAction::SEND, "b " DEBUGGER_JS_DIR "module_watch.js 26"},
            {SocketAction::RECV, "", ActionRule::CUSTOM_RULE, MatchRule::replySuccess},

            // hit breakpoint after resume first time
            {SocketAction::SEND, "resume"},
            {SocketAction::RECV, "Debugger.resumed", ActionRule::STRING_CONTAIN},
            {SocketAction::RECV, "", ActionRule::CUSTOM_RULE, MatchRule::replySuccess},
            {SocketAction::RECV, "Debugger.paused", ActionRule::CUSTOM_RULE,
                [this](auto recv, auto, auto) -> bool {
                    return RecvHitBreakInfo(recv, 25);
                }
            },
            {SocketAction::SEND, "watch allTest"},
            {SocketAction::RECV, "", ActionRule::CUSTOM_RULE,
                [this](auto recv, auto, auto) -> bool { return RecvWatchVariableInfo(recv, "Object"); }},
            {SocketAction::SEND, "p o 1"},
            {SocketAction::RECV, "", ActionRule::CUSTOM_RULE,
                [this] (auto recv, auto, auto) -> bool {
                    return GetPropertiesInfo(recv);
                }
            },
            {SocketAction::SEND, "p 5"},
            {SocketAction::RECV, "", ActionRule::CUSTOM_RULE,
                [this] (auto recv, auto, auto) -> bool {
                    return GetPropertiesInfo(recv);  //check elements in allTest
                }
            },
            {SocketAction::SEND, "success"},
            {SocketAction::SEND, "resume"},
            {SocketAction::RECV, "Debugger.resumed", ActionRule::STRING_CONTAIN},
            {SocketAction::RECV, "", ActionRule::CUSTOM_RULE, MatchRule::replySuccess},
        };
    }
    bool GetPropertiesInfo(std::string recv)
    {
        std::unique_ptr<PtJson> json = PtJson::Parse(recv);
        
        std::unique_ptr<PtJson> result;
        GTEST_LOG_(INFO) << "JsWatchModuleTestChannel: SendNotification 3:\n" << json->Stringify();
        int id;
        Result ret = json->GetInt("id", &id);
        if (ret != Result::SUCCESS) {
            return false;
        }

        ret = json->GetObject("result", &result);
        if (ret != Result::SUCCESS) {
            return false;
        }

        std::unique_ptr<PtJson> innerResult;
        ret = result->GetArray("result", &innerResult);
        if (ret != Result::SUCCESS) {
            return false;
        }
        std::unique_ptr<PtJson> value;
        GTEST_LOG_(INFO) << "JsWatchModuleTestChannel: object:\n" << innerResult->Get(1)->Stringify();
        ret = innerResult->Get(1)->GetObject("value", &value);
        if (ret != Result::SUCCESS) {
            return false;
        }
        std::string type;
        ret = value->GetString("type", &type);
        if (ret != Result::SUCCESS) {
            return false;
        }
        GTEST_LOG_(INFO) << "JsWatchModuleTestChannel: type:\n" << type.c_str();
        if (type == "undefined") {
            return false;
        }
        RuntimeClient runtimeClient(0);
        runtimeClient.HandleGetProperties(std::move(json), id);
        return true;
    }
    bool RecvHitBreakInfo(std::string recv, int line)
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
            breakpoint.find(std::to_string(line)) == std::string::npos) {
            return false;
        }
        DebuggerClient debuggerClient(0);
        debuggerClient.PausedReply(std::move(json));
        return true;
    }
    bool RecvWatchVariableInfo(std::string recv, std::string var_value)
    {
        std::unique_ptr<PtJson> json = PtJson::Parse(recv);
        GTEST_LOG_(INFO) << "JsWatchModuleTestChannel: SendNotification 4:\n" << json->Stringify();
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
    ~JsWatchModuleTest() = default;

private:
    std::string pandaFile_ = DEBUGGER_ABC_DIR "module_watch.abc";
    std::string sourceFile_ = DEBUGGER_JS_DIR "module_watch.js";
    std::string entryPoint_ = "module_watch";
};

std::unique_ptr<TestActions> GetJsWatchModuleTest()
{
    return std::make_unique<JsWatchModuleTest>();
}
} // namespace panda::ecmascript::tooling::test

#endif // ECMASCRIPT_TOOLING_TEST_TESTCASES_JS_WATCH_MODULE_TEST_H
