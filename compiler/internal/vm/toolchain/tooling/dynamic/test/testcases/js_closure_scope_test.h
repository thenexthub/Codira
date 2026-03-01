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

#ifndef ECMASCRIPT_TOOLING_TEST_TESTCASES_JS_CLOSURE_SCOPE_TEST_H
#define ECMASCRIPT_TOOLING_TEST_TESTCASES_JS_CLOSURE_SCOPE_TEST_H

#include <map>
#include "tooling/dynamic/test/client_utils/test_util.h"

namespace panda::ecmascript::tooling::test {
class JsClosureScopeTest : public TestActions {
public:
    JsClosureScopeTest()
    {
        testAction = {
            {SocketAction::SEND, "enable"},
            {SocketAction::RECV, "", ActionRule::CUSTOM_RULE, MatchRule::replySuccess},
            {SocketAction::SEND, "runtime-enable"},
            {SocketAction::RECV, "", ActionRule::CUSTOM_RULE, MatchRule::replySuccess},
            {SocketAction::SEND, "run"},
            {SocketAction::RECV, "", ActionRule::CUSTOM_RULE, MatchRule::replySuccess},
            // load closure_scope.js
            {SocketAction::RECV, "Debugger.scriptParsed", ActionRule::STRING_CONTAIN},
            // break on start
            {SocketAction::RECV, "Debugger.paused", ActionRule::STRING_CONTAIN},
            // set first breakpoint
            {SocketAction::SEND, "b " DEBUGGER_JS_DIR "closure_scope.js 30"},
            {SocketAction::RECV, "", ActionRule::CUSTOM_RULE, MatchRule::replySuccess},
            // // set second breakpoint
            {SocketAction::SEND, "b " DEBUGGER_JS_DIR "closure_scope.js 53"},
            {SocketAction::RECV, "", ActionRule::CUSTOM_RULE, MatchRule::replySuccess},

            // hit breakpoint after resume first time
            {SocketAction::SEND, "resume"},
            {SocketAction::RECV, "Debugger.resumed", ActionRule::STRING_CONTAIN},
            {SocketAction::RECV, "", ActionRule::CUSTOM_RULE, MatchRule::replySuccess},
            {SocketAction::RECV, "Debugger.paused", ActionRule::CUSTOM_RULE, [] (auto recv, auto, auto) -> bool {
                std::unique_ptr<PtJson> json = PtJson::Parse(recv);
                Result ret;
                std::string method;
                ret = json->GetString("method", &method);
                if (ret != Result::SUCCESS || method != "Debugger.paused") {
                    return false;
                }

                DebuggerClient debuggerClient(0);
                debuggerClient.PausedReply(std::move(json));
                return true;
            }},
            {SocketAction::SEND, "print"},
            {SocketAction::RECV, "", ActionRule::CUSTOM_RULE, [] (auto recv, auto, auto) -> bool {
                std::unique_ptr<PtJson> json = PtJson::Parse(recv);
                int32_t id = 0;
                Result ret = json->GetInt("id", &id);
                if (ret != Result::SUCCESS) {
                    return false;
                }
                RuntimeClient runtimeClient(0);
                runtimeClient.HandleGetProperties(std::move(json), id);
                return true;
            }},
            {SocketAction::SEND, "print 2"},
            {SocketAction::RECV, "", ActionRule::CUSTOM_RULE, [] (auto recv, auto, auto) -> bool {
                std::unique_ptr<PtJson> json = PtJson::Parse(recv);
                Result ret;
                int32_t id = 0;
                ret = json->GetInt("id", &id);
                if (ret != Result::SUCCESS) {
                    return false;
                }

                std::unique_ptr<PtJson> result = nullptr;
                ret = json->GetObject("result", &result);
                if (ret != Result::SUCCESS) {
                    return false;
                }

                std::unique_ptr<PtJson> innerResult;
                ret = result->GetArray("result", &innerResult);
                if (ret != Result::SUCCESS) {
                    return false;
                }

                std::string name;
                ret = innerResult->Get(0)->GetString("name", &name);
                if (ret != Result::SUCCESS || name != "v3") {
                    return false;
                }

                std::unique_ptr<PtJson> value;
                ret = innerResult->Get(0)->GetObject("value", &value);
                if (ret != Result::SUCCESS) {
                    return false;
                }

                std::string valueDes;
                ret = value->GetString("description", &valueDes);
                if (ret != Result::SUCCESS || valueDes != "4") {
                    return false;
                }
                return true;
            }},
            {SocketAction::SEND, "print 3"},
            {SocketAction::RECV, "", ActionRule::CUSTOM_RULE, [] (auto recv, auto, auto) -> bool {
                std::unique_ptr<PtJson> json = PtJson::Parse(recv);
                Result ret;
                int32_t id = 0;
                ret = json->GetInt("id", &id);
                if (ret != Result::SUCCESS) {
                    return false;
                }

                std::unique_ptr<PtJson> result = nullptr;
                ret = json->GetObject("result", &result);
                if (ret != Result::SUCCESS) {
                    return false;
                }

                std::unique_ptr<PtJson> innerResult;
                ret = result->GetArray("result", &innerResult);
                if (ret != Result::SUCCESS) {
                    return false;
                }

                std::string name;
                ret = innerResult->Get(0)->GetString("name", &name);
                if (ret != Result::SUCCESS || name != "v2_2") {
                    return false;
                }

                std::unique_ptr<PtJson> value;
                ret = innerResult->Get(0)->GetObject("value", &value);
                if (ret != Result::SUCCESS) {
                    return false;
                }

                std::string valueDes;
                ret = value->GetString("description", &valueDes);
                if (ret != Result::SUCCESS || valueDes != "3") {
                    return false;
                }

                ret = innerResult->Get(1)->GetString("name", &name);
                if (ret != Result::SUCCESS || name != "v2_1") {
                    return false;
                }

                ret = innerResult->Get(1)->GetObject("value", &value);
                if (ret != Result::SUCCESS) {
                    return false;
                }

                ret = value->GetString("description", &valueDes);
                if (ret != Result::SUCCESS || valueDes != "2") {
                    return false;
                }
                return true;
            }},
            {SocketAction::SEND, "print 4"},
            {SocketAction::RECV, "", ActionRule::CUSTOM_RULE, [] (auto recv, auto, auto) -> bool {
                std::unique_ptr<PtJson> json = PtJson::Parse(recv);
                Result ret;
                int32_t id = 0;
                ret = json->GetInt("id", &id);
                if (ret != Result::SUCCESS) {
                    return false;
                }

                std::unique_ptr<PtJson> result = nullptr;
                ret = json->GetObject("result", &result);
                if (ret != Result::SUCCESS) {
                    return false;
                }

                std::unique_ptr<PtJson> innerResult;
                ret = result->GetArray("result", &innerResult);
                if (ret != Result::SUCCESS) {
                    return false;
                }

                std::string name;
                ret = innerResult->Get(0)->GetString("name", &name);
                if (ret != Result::SUCCESS || name != "v1") {
                    return false;
                }

                std::unique_ptr<PtJson> value;
                ret = innerResult->Get(0)->GetObject("value", &value);
                if (ret != Result::SUCCESS) {
                    return false;
                }

                std::string valueDes;
                ret = value->GetString("description", &valueDes);
                if (ret != Result::SUCCESS || valueDes != "1") {
                    return false;
                }
                return true;
            }},

            // hit breakpoint after resume second time
            {SocketAction::SEND, "resume"},
            {SocketAction::RECV, "Debugger.resumed", ActionRule::STRING_CONTAIN},
            {SocketAction::RECV, "", ActionRule::CUSTOM_RULE, MatchRule::replySuccess},
            {SocketAction::RECV, "Debugger.paused", ActionRule::CUSTOM_RULE, [] (auto recv, auto, auto) -> bool {
                std::unique_ptr<PtJson> json = PtJson::Parse(recv);
                Result ret;
                std::string method;
                ret = json->GetString("method", &method);
                if (ret != Result::SUCCESS || method != "Debugger.paused") {
                    return false;
                }
                DebuggerClient debuggerClient(0);
                debuggerClient.PausedReply(std::move(json));
                return true;
            }},
            {SocketAction::SEND, "print"},
            {SocketAction::RECV, "", ActionRule::CUSTOM_RULE, [] (auto recv, auto, auto) -> bool {
                std::unique_ptr<PtJson> json = PtJson::Parse(recv);
                int32_t id = 0;
                Result ret = json->GetInt("id", &id);
                if (ret != Result::SUCCESS) {
                    return false;
                }
                RuntimeClient runtimeClient(0);
                runtimeClient.HandleGetProperties(std::move(json), id);
                return true;
            }},
            {SocketAction::SEND, "print 2"},
            {SocketAction::RECV, "", ActionRule::CUSTOM_RULE, [] (auto recv, auto, auto) -> bool {
                std::unique_ptr<PtJson> json = PtJson::Parse(recv);
                Result ret;
                int32_t id = 0;
                ret = json->GetInt("id", &id);
                if (ret != Result::SUCCESS) {
                    return false;
                }

                std::unique_ptr<PtJson> result = nullptr;
                ret = json->GetObject("result", &result);
                if (ret != Result::SUCCESS) {
                    return false;
                }

                std::unique_ptr<PtJson> innerResult;
                ret = result->GetArray("result", &innerResult);
                if (ret != Result::SUCCESS) {
                    return false;
                }

                std::string name;
                ret = innerResult->Get(0)->GetString("name", &name);
                if (ret != Result::SUCCESS || name != "i") {
                    return false;
                }

                std::unique_ptr<PtJson> value;
                ret = innerResult->Get(0)->GetObject("value", &value);
                if (ret != Result::SUCCESS) {
                    return false;
                }

                std::string valueDes;
                ret = value->GetString("description", &valueDes);
                if (ret != Result::SUCCESS || valueDes != "5") {
                    return false;
                }
                return true;
            }},
            {SocketAction::SEND, "print 3"},
            {SocketAction::RECV, "", ActionRule::CUSTOM_RULE, [] (auto recv, auto, auto) -> bool {
                std::unique_ptr<PtJson> json = PtJson::Parse(recv);
                Result ret;
                int32_t id = 0;
                ret = json->GetInt("id", &id);
                if (ret != Result::SUCCESS) {
                    return false;
                }

                std::unique_ptr<PtJson> result = nullptr;
                ret = json->GetObject("result", &result);
                if (ret != Result::SUCCESS) {
                    return false;
                }

                std::unique_ptr<PtJson> innerResult;
                ret = result->GetArray("result", &innerResult);
                if (ret != Result::SUCCESS) {
                    return false;
                }

                std::string name;
                ret = innerResult->Get(0)->GetString("name", &name);
                if (ret != Result::SUCCESS || name != "a") {
                    return false;
                }

                std::unique_ptr<PtJson> value;
                ret = innerResult->Get(0)->GetObject("value", &value);
                if (ret != Result::SUCCESS) {
                    return false;
                }

                std::string valueDes;
                ret = value->GetString("description", &valueDes);
                if (ret != Result::SUCCESS || valueDes != "10") {
                    return false;
                }
                return true;
            }},
            // reply success and run
            {SocketAction::SEND, "success"},
            {SocketAction::SEND, "resume"},
            {SocketAction::RECV, "Debugger.resumed", ActionRule::STRING_CONTAIN},
            {SocketAction::RECV, "", ActionRule::CUSTOM_RULE, MatchRule::replySuccess},
        };
    }

    std::pair<std::string, std::string> GetEntryPoint() override
    {
        return {pandaFile_, entryPoint_};
    }
    ~JsClosureScopeTest() = default;

private:
    std::string pandaFile_ = DEBUGGER_ABC_DIR "closure_scope.abc";
    std::string sourceFile_ = DEBUGGER_JS_DIR "closure_scope.js";
    std::string entryPoint_ = "closure_scope";
};

std::unique_ptr<TestActions> GetJsClosureScopeTest()
{
    return std::make_unique<JsClosureScopeTest>();
}
}  // namespace panda::ecmascript::tooling::test

#endif  // ECMASCRIPT_TOOLING_TEST_TESTCASES_JS_CLOSURE_SCOPE_TEST_H
