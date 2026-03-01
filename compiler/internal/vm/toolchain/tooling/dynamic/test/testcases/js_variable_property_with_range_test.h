/**
 * Copyright (c) 2026 Huawei Device Co., Ltd.
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
 
#ifndef ECMASCRIPT_TOOLING_TEST_TESTCASES_JS_VARIABLE_PROPERTY_WITH_RANGE_H
#define ECMASCRIPT_TOOLING_TEST_TESTCASES_JS_VARIABLE_PROPERTY_WITH_RANGE_H
 
#include "tooling/dynamic/test/client_utils/test_util.h"
 
namespace panda::ecmascript::tooling::test {
class JsVariablePropertyWithRangeTest : public TestActions {
public:
    JsVariablePropertyWithRangeTest()
    {
        testAction = {
            {SocketAction::SEND, "enable"},
            {SocketAction::RECV, "", ActionRule::CUSTOM_RULE, MatchRule::replySuccess},
            {SocketAction::SEND, "runtime-enable"},
            {SocketAction::RECV, "", ActionRule::CUSTOM_RULE, MatchRule::replySuccess},
            {SocketAction::SEND, "run"},
            {SocketAction::RECV, "", ActionRule::CUSTOM_RULE, MatchRule::replySuccess},
            // load variable_properties_with_range.js
            {SocketAction::RECV, "Debugger.scriptParsed", ActionRule::STRING_CONTAIN},
            {SocketAction::RECV, "Debugger.paused", ActionRule::STRING_CONTAIN},
            // set 2 breakpoints
            {SocketAction::SEND, "b " DEBUGGER_JS_DIR "variable_properties_with_range.js 138"},
            {SocketAction::RECV, "", ActionRule::CUSTOM_RULE, MatchRule::replySuccess},
            {SocketAction::SEND, "b " DEBUGGER_JS_DIR "variable_properties_with_range.js 203"},
            {SocketAction::RECV, "", ActionRule::CUSTOM_RULE, MatchRule::replySuccess},
            // resume to run
            {SocketAction::SEND, "resume"},
            {SocketAction::RECV, "Debugger.resumed", ActionRule::STRING_CONTAIN},
            {SocketAction::RECV, "", ActionRule::CUSTOM_RULE, MatchRule::replySuccess},
            {SocketAction::RECV, "Debugger.paused", ActionRule::CUSTOM_RULE, [this] (auto recv, auto, auto) -> bool {
                
                std::unique_ptr<PtJson> json = PtJson::Parse(recv);
                Result ret;
                std::string method;
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
 
                std::string breakpoint;
                breakpoint = hitBreakpoints->Get(0)->GetString();
                if (ret != Result::SUCCESS || breakpoint.find(sourceFile_) == std::string::npos ||
                    breakpoint.find("137") == std::string::npos) {
                    return false;
                }
 
                DebuggerClient debuggerClient(0);
                debuggerClient.PausedReply(std::move(json));
                return true;
            }},
 
            {SocketAction::SEND, "p"},
            {SocketAction::RECV, "", ActionRule::CUSTOM_RULE, MatchRule::replySuccess},
 
            {SocketAction::SEND, "p 6 0 100"},
            {SocketAction::RECV, "", ActionRule::CUSTOM_RULE, [this] (auto recv, auto, auto) -> bool {
                return CompareExpected(recv, truthGround[expectDesIndex]);
            }},
            // Test start and end are in the range of object property index
            {SocketAction::SEND, "p 43 0 10"},
            {SocketAction::RECV, "", ActionRule::CUSTOM_RULE, [this] (auto recv, auto, auto) -> bool {
                return CompareExpected(recv, truthGround[expectDesIndex]);
            }},
 
            {SocketAction::SEND, "p 9 0 100"},
            {SocketAction::RECV, "", ActionRule::CUSTOM_RULE, [this] (auto recv, auto, auto) -> bool {
                return CompareExpected(recv, truthGround[expectDesIndex]);
            }},
            // Test start and end are in the range of object property index,
            // and not from the beginning
            {SocketAction::SEND, "p 55 10 5"},
            {SocketAction::RECV, "", ActionRule::CUSTOM_RULE, [this] (auto recv, auto, auto) -> bool {
                return CompareExpected(recv, truthGround[expectDesIndex]);
            }},
 
            {SocketAction::SEND, "p 21 0 100"},
            {SocketAction::RECV, "", ActionRule::CUSTOM_RULE, [this] (auto recv, auto, auto) -> bool {
                return CompareExpected(recv, truthGround[expectDesIndex]);
            }},
            // Test end are out of the range, only return startIndex to LastIndex
            {SocketAction::SEND, "p 57 140 100"},
            {SocketAction::RECV, "", ActionRule::CUSTOM_RULE, [this] (auto recv, auto, auto) -> bool {
                return CompareExpected(recv, truthGround[expectDesIndex]);
            }},
 
            {SocketAction::SEND, "resume"},
            {SocketAction::RECV, "Debugger.resumed", ActionRule::STRING_CONTAIN},
            {SocketAction::RECV, "", ActionRule::CUSTOM_RULE, MatchRule::replySuccess},
 
            {SocketAction::RECV, "Debugger.paused", ActionRule::CUSTOM_RULE, [this] (auto recv, auto, auto) -> bool {
                
                std::unique_ptr<PtJson> json = PtJson::Parse(recv);
                Result ret;
                std::string method;
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
 
                std::string breakpoint;
                breakpoint = hitBreakpoints->Get(0)->GetString();
                if (ret != Result::SUCCESS || breakpoint.find(sourceFile_) == std::string::npos ||
                    breakpoint.find("202") == std::string::npos) {
                    return false;
                }
 
                DebuggerClient debuggerClient(0);
                debuggerClient.PausedReply(std::move(json));
                return true;
            }},
 
            {SocketAction::SEND, "p"},
            {SocketAction::RECV, "", ActionRule::CUSTOM_RULE, MatchRule::replySuccess},
            // Test property range request for arrays
            {SocketAction::SEND, "p 7 0 10"},
            {SocketAction::RECV, "", ActionRule::CUSTOM_RULE, [this] (auto recv, auto, auto) -> bool {
                return CompareExpected(recv, truthGround[expectDesIndex]);
            }},
 
            {SocketAction::SEND, "p 11 100 6"},
            {SocketAction::RECV, "", ActionRule::CUSTOM_RULE, [this] (auto recv, auto, auto) -> bool {
                return CompareExpected(recv, truthGround[expectDesIndex]);
            }},
 
            {SocketAction::SEND, "p 15 145 100"},
            {SocketAction::RECV, "", ActionRule::CUSTOM_RULE, [this] (auto recv, auto, auto) -> bool {
                return CompareExpected(recv, truthGround[expectDesIndex]);
            }},
 
            {SocketAction::SEND, "p 17 99 1"},
            {SocketAction::RECV, "", ActionRule::CUSTOM_RULE, [this] (auto recv, auto, auto) -> bool {
                return CompareExpected(recv, truthGround[expectDesIndex]);
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
    ~JsVariablePropertyWithRangeTest() = default;
 
private:
    std::string pandaFile_ = DEBUGGER_ABC_DIR "variable_properties_with_range.abc";
    std::string sourceFile_ = DEBUGGER_JS_DIR "variable_properties_with_range.js";
    std::string entryPoint_ = "variable_properties_with_range";
    size_t expectDesIndex = 0;
 
    bool CompareExpected(std::string recv, const std::string &expect)
    {
        expectDesIndex++;
        bool result = (recv == expect);
        if (!result) {
            std::cout << "recv: " << recv << std::endl;
            std::cout << "expect: " << expect << std::endl;
        }
        return result;
    }
 
    const std::vector<std::string> truthGround = {
        "{\"id\":8,\"result\":{\"result\":[{\"name\":\"size\",\"value\":{\"type\":\"number\","
        "\"unserializableValue\":\"150\",""\"description\":\"150\",\"objectId\":\"42\"},"
        "\"writable\":false,\"configurable\":false,\"enumerable\":false,\"isOwn\":false},"
        "{\"name\":\"[[PlainArray]]\",\"value\":{\"type\":\"object\",\"subtype\":\"array\","
        "\"className\":\"Array\",\"unserializableValue\":\"Array(100)\",\"description\":"
        "\"Array(150)\",\"objectId\":\"43\",\"arrayOrContainer\":\"Array\"},\"writable\":false,"
        "\"configurable\":false,\"enumerable\":false,\"isOwn\":false}]}}",
 
        "{\"id\":9,\"result\":{\"result\":[{\"name\":\"0\",\"value\":{\"type\":\"object\","
        "\"className\":\"Object\",\"unserializableValue\":\"Object\",\"description\":\"Object\","
        "\"objectId\":\"44\"},\"writable\":true,\"configurable\":true,\"enumerable\":true,"
        "\"isOwn\":true},{\"name\":\"1\",\"value\":{\"type\":\"object\",\"className\":\"Object\","
        "\"unserializableValue\":\"Object\",\"description\":\"Object\",\"objectId\":\"45\"},"
        "\"writable\":true,\"configurable\":true,\"enumerable\":true,\"isOwn\":true},"
        "{\"name\":\"2\",\"value\":{\"type\":\"object\",\"className\":\"Object\","
        "\"unserializableValue\":\"Object\",\"description\":\"Object\",\"objectId\":\"46\"},"
        "\"writable\":true,\"configurable\":true,\"enumerable\":true,\"isOwn\":true},{\"name\":\"3\","
        "\"value\":{\"type\":\"object\",\"className\":\"Object\",\"unserializableValue\":\"Object\","
        "\"description\":\"Object\",\"objectId\":\"47\"},\"writable\":true,\"configurable\":true,"
        "\"enumerable\":true,\"isOwn\":true},{\"name\":\"4\",\"value\":{\"type\":\"object\","
        "\"className\":\"Object\",\"unserializableValue\":\"Object\",\"description\":\"Object\","
        "\"objectId\":\"48\"},\"writable\":true,\"configurable\":true,\"enumerable\":true,\"isOwn\":true},"
        "{\"name\":\"5\",\"value\":{\"type\":\"object\",\"className\":\"Object\","
        "\"unserializableValue\":\"Object\",\"description\":\"Object\",\"objectId\":\"49\"},"
        "\"writable\":true,\"configurable\":true,\"enumerable\":true,\"isOwn\":true},"
        "{\"name\":\"6\",\"value\":{\"type\":\"object\",\"className\":\"Object\","
        "\"unserializableValue\":\"Object\",\"description\":\"Object\",\"objectId\":\"50\"},"
        "\"writable\":true,\"configurable\":true,\"enumerable\":true,\"isOwn\":true},"
        "{\"name\":\"7\",\"value\":{\"type\":\"object\",\"className\":\"Object\","
        "\"unserializableValue\":\"Object\",\"description\":\"Object\",\"objectId\":\"51\"},"
        "\"writable\":true,\"configurable\":true,\"enumerable\":true,\"isOwn\":true},"
        "{\"name\":\"8\",\"value\":{\"type\":\"object\",\"className\":\"Object\","
        "\"unserializableValue\":\"Object\",\"description\":\"Object\",\"objectId\":\"52\"},"
        "\"writable\":true,\"configurable\":true,\"enumerable\":true,\"isOwn\":true},"
        "{\"name\":\"9\",\"value\":{\"type\":\"object\",\"className\":\"Object\","
        "\"unserializableValue\":\"Object\",\"description\":\"Object\",\"objectId\":\"53\"},"
        "\"writable\":true,\"configurable\":true,\"enumerable\":true,\"isOwn\":true}]}}",
 
        "{\"id\":10,\"result\":{\"result\":[{\"name\":\"size\",\"value\":{\"type\":\"number\","
        "\"unserializableValue\":\"150\",\"description\":\"150\",\"objectId\":\"54\"},"
        "\"writable\":false,\"configurable\":false,\"enumerable\":false,\"isOwn\":false},"
        "{\"name\":\"[[ArrayList]]\",\"value\":{\"type\":\"object\",\"subtype\":\"array\","
        "\"className\":\"Array\",\"unserializableValue\":\"Array(100)\","
        "\"description\":\"Array(150)\",\"objectId\":\"55\",\"arrayOrContainer\":\"Array\"},"
        "\"writable\":false,\"configurable\":false,\"enumerable\":false,\"isOwn\":false}]}}",
 
        "{\"id\":11,\"result\":{\"result\":[{\"name\":\"10\",\"value\":{\"type\":\"number\","
        "\"unserializableValue\":\"10\",\"description\":\"10\"},\"writable\":true,"
        "\"configurable\":true,\"enumerable\":true,\"isOwn\":true},{\"name\":\"11\","
        "\"value\":{\"type\":\"number\",\"unserializableValue\":\"11\",\"description\":\"11\"},"
        "\"writable\":true,\"configurable\":true,\"enumerable\":true,\"isOwn\":true},"
        "{\"name\":\"12\",\"value\":{\"type\":\"number\",\"unserializableValue\":\"12\","
        "\"description\":\"12\"},\"writable\":true,\"configurable\":true,\"enumerable\":true,"
        "\"isOwn\":true},{\"name\":\"13\",\"value\":{\"type\":\"number\","
        "\"unserializableValue\":\"13\",\"description\":\"13\"},\"writable\":true,"
        "\"configurable\":true,\"enumerable\":true,\"isOwn\":true},{\"name\":\"14\","
        "\"value\":{\"type\":\"number\",\"unserializableValue\":\"14\",\"description\":\"14\"},"
        "\"writable\":true,\"configurable\":true,\"enumerable\":true,\"isOwn\":true}]}}",
 
        "{\"id\":12,\"result\":{\"result\":[{\"name\":\"size\",\"value\":{\"type\":\"number\","
        "\"unserializableValue\":\"150\",\"description\":\"150\",\"objectId\":\"56\"},"
        "\"writable\":false,\"configurable\":false,\"enumerable\":false,\"isOwn\":false},"
        "{\"name\":\"[[Entries]]\",\"value\":{\"type\":\"object\",\"subtype\":\"array\","
        "\"className\":\"Array\",\"unserializableValue\":\"Array(100)\","
        "\"description\":\"Array(150)\",\"objectId\":\"57\",\"arrayOrContainer\":\"Array\"},"
        "\"writable\":false,\"configurable\":false,\"enumerable\":false,\"isOwn\":false}]}}",
 
        "{\"id\":13,\"result\":{\"result\":[{\"name\":\"0\",\"value\":{\"type\":\"object\",\"className\":\"Object\","
        "\"unserializableValue\":\"Object\",\"description\":\"Object\",\"objectId\":\"58\"},\"writable\":true,"
        "\"configurable\":true,\"enumerable\":true,\"isOwn\":true},{\"name\":\"1\",\"value\":{\"type\":\"object\","
        "\"className\":\"Object\",\"unserializableValue\":\"Object\",\"description\":\"Object\",\"objectId\":\"59\"},"
        "\"writable\":true,\"configurable\":true,\"enumerable\":true,\"isOwn\":true},{\"name\":\"2\","
        "\"value\":{\"type\":\"object\",\"className\":\"Object\",\"unserializableValue\":\"Object\","
        "\"description\":\"Object\",\"objectId\":\"60\"},\"writable\":true,\"configurable\":true,\"enumerable\":true,"
        "\"isOwn\":true},{\"name\":\"3\",\"value\":{\"type\":\"object\",\"className\":\"Object\","
        "\"unserializableValue\":\"Object\",\"description\":\"Object\",\"objectId\":\"61\"},\"writable\":true,"
        "\"configurable\":true,\"enumerable\":true,\"isOwn\":true},{\"name\":\"4\",\"value\":{\"type\":\"object\","
        "\"className\":\"Object\",\"unserializableValue\":\"Object\",\"description\":\"Object\",\"objectId\":\"62\"},"
        "\"writable\":true,\"configurable\":true,\"enumerable\":true,\"isOwn\":true},{\"name\":\"5\","
        "\"value\":{\"type\":\"object\",\"className\":\"Object\",\"unserializableValue\":\"Object\","
        "\"description\":\"Object\",\"objectId\":\"63\"},\"writable\":true,\"configurable\":true,\"enumerable\":true,"
        "\"isOwn\":true},{\"name\":\"6\",\"value\":{\"type\":\"object\",\"className\":\"Object\","
        "\"unserializableValue\":\"Object\",\"description\":\"Object\",\"objectId\":\"64\"},\"writable\":true,"
        "\"configurable\":true,\"enumerable\":true,\"isOwn\":true},{\"name\":\"7\",\"value\":{\"type\":\"object\","
        "\"className\":\"Object\",\"unserializableValue\":\"Object\",\"description\":\"Object\",\"objectId\":\"65\"},"
        "\"writable\":true,\"configurable\":true,\"enumerable\":true,\"isOwn\":true},{\"name\":\"8\","
        "\"value\":{\"type\":\"object\",\"className\":\"Object\",\"unserializableValue\":\"Object\","
        "\"description\":\"Object\",\"objectId\":\"66\"},\"writable\":true,\"configurable\":true,\"enumerable\":true,"
        "\"isOwn\":true},{\"name\":\"9\",\"value\":{\"type\":\"object\",\"className\":\"Object\","
        "\"unserializableValue\":\"Object\",\"description\":\"Object\",\"objectId\":\"67\"},\"writable\":true,"
        "\"configurable\":true,\"enumerable\":true,\"isOwn\":true},{\"name\":\"10\",\"value\":{\"type\":\"object\","
        "\"className\":\"Object\",\"unserializableValue\":\"Object\",\"description\":\"Object\",\"objectId\":\"68\"},"
        "\"writable\":true,\"configurable\":true,\"enumerable\":true,\"isOwn\":true},{\"name\":\"11\","
        "\"value\":{\"type\":\"object\",\"className\":\"Object\",\"unserializableValue\":\"Object\","
        "\"description\":\"Object\",\"objectId\":\"69\"},\"writable\":true,\"configurable\":true,\"enumerable\":true,"
        "\"isOwn\":true},{\"name\":\"12\",\"value\":{\"type\":\"object\",\"className\":\"Object\","
        "\"unserializableValue\":\"Object\",\"description\":\"Object\",\"objectId\":\"70\"},\"writable\":true,"
        "\"configurable\":true,\"enumerable\":true,\"isOwn\":true},{\"name\":\"13\",\"value\":{\"type\":\"object\","
        "\"className\":\"Object\",\"unserializableValue\":\"Object\",\"description\":\"Object\",\"objectId\":\"71\"},"
        "\"writable\":true,\"configurable\":true,\"enumerable\":true,\"isOwn\":true},{\"name\":\"14\","
        "\"value\":{\"type\":\"object\",\"className\":\"Object\",\"unserializableValue\":\"Object\","
        "\"description\":\"Object\",\"objectId\":\"72\"},\"writable\":true,\"configurable\":true,\"enumerable\":true,"
        "\"isOwn\":true},{\"name\":\"15\",\"value\":{\"type\":\"object\",\"className\":\"Object\","
        "\"unserializableValue\":\"Object\",\"description\":\"Object\",\"objectId\":\"73\"},\"writable\":true,"
        "\"configurable\":true,\"enumerable\":true,\"isOwn\":true},{\"name\":\"16\",\"value\":{\"type\":\"object\","
        "\"className\":\"Object\",\"unserializableValue\":\"Object\",\"description\":\"Object\",\"objectId\":\"74\"},"
        "\"writable\":true,\"configurable\":true,\"enumerable\":true,\"isOwn\":true},{\"name\":\"17\","
        "\"value\":{\"type\":\"object\",\"className\":\"Object\",\"unserializableValue\":\"Object\","
        "\"description\":\"Object\",\"objectId\":\"75\"},\"writable\":true,\"configurable\":true,\"enumerable\":true,"
        "\"isOwn\":true},{\"name\":\"18\",\"value\":{\"type\":\"object\",\"className\":\"Object\","
        "\"unserializableValue\":\"Object\",\"description\":\"Object\",\"objectId\":\"76\"},\"writable\":true,"
        "\"configurable\":true,\"enumerable\":true,\"isOwn\":true},{\"name\":\"19\",\"value\":{\"type\":\"object\","
        "\"className\":\"Object\",\"unserializableValue\":\"Object\",\"description\":\"Object\",\"objectId\":\"77\"},"
        "\"writable\":true,\"configurable\":true,\"enumerable\":true,\"isOwn\":true},{\"name\":\"20\","
        "\"value\":{\"type\":\"object\",\"className\":\"Object\",\"unserializableValue\":\"Object\","
        "\"description\":\"Object\",\"objectId\":\"78\"},\"writable\":true,\"configurable\":true,\"enumerable\":true,"
        "\"isOwn\":true},{\"name\":\"21\",\"value\":{\"type\":\"object\",\"className\":\"Object\","
        "\"unserializableValue\":\"Object\",\"description\":\"Object\",\"objectId\":\"79\"},\"writable\":true,"
        "\"configurable\":true,\"enumerable\":true,\"isOwn\":true},{\"name\":\"22\",\"value\":{\"type\":\"object\","
        "\"className\":\"Object\",\"unserializableValue\":\"Object\",\"description\":\"Object\",\"objectId\":\"80\"},"
        "\"writable\":true,\"configurable\":true,\"enumerable\":true,\"isOwn\":true},{\"name\":\"23\","
        "\"value\":{\"type\":\"object\",\"className\":\"Object\",\"unserializableValue\":\"Object\","
        "\"description\":\"Object\",\"objectId\":\"81\"},\"writable\":true,\"configurable\":true,\"enumerable\":true,"
        "\"isOwn\":true},{\"name\":\"24\",\"value\":{\"type\":\"object\",\"className\":\"Object\","
        "\"unserializableValue\":\"Object\",\"description\":\"Object\",\"objectId\":\"82\"},\"writable\":true,"
        "\"configurable\":true,\"enumerable\":true,\"isOwn\":true},{\"name\":\"25\",\"value\":{\"type\":\"object\","
        "\"className\":\"Object\",\"unserializableValue\":\"Object\",\"description\":\"Object\",\"objectId\":\"83\"},"
        "\"writable\":true,\"configurable\":true,\"enumerable\":true,\"isOwn\":true},{\"name\":\"26\","
        "\"value\":{\"type\":\"object\",\"className\":\"Object\",\"unserializableValue\":\"Object\","
        "\"description\":\"Object\",\"objectId\":\"84\"},\"writable\":true,\"configurable\":true,"
        "\"enumerable\":true,\"isOwn\":true},{\"name\":\"27\",\"value\":{\"type\":\"object\","
        "\"className\":\"Object\",\"unserializableValue\":\"Object\",\"description\":\"Object\",\"objectId\":\"85\"},"
        "\"writable\":true,\"configurable\":true,\"enumerable\":true,\"isOwn\":true},{\"name\":\"28\","
        "\"value\":{\"type\":\"object\",\"className\":\"Object\",\"unserializableValue\":\"Object\","
        "\"description\":\"Object\",\"objectId\":\"86\"},\"writable\":true,\"configurable\":true,\"enumerable\":true,"
        "\"isOwn\":true},{\"name\":\"29\",\"value\":{\"type\":\"object\",\"className\":\"Object\","
        "\"unserializableValue\":\"Object\",\"description\":\"Object\",\"objectId\":\"87\"},\"writable\":true,"
        "\"configurable\":true,\"enumerable\":true,\"isOwn\":true},{\"name\":\"30\",\"value\":{\"type\":\"object\","
        "\"className\":\"Object\",\"unserializableValue\":\"Object\",\"description\":\"Object\",\"objectId\":\"88\"},"
        "\"writable\":true,\"configurable\":true,\"enumerable\":true,\"isOwn\":true},{\"name\":\"31\","
        "\"value\":{\"type\":\"object\",\"className\":\"Object\",\"unserializableValue\":\"Object\","
        "\"description\":\"Object\",\"objectId\":\"89\"},\"writable\":true,\"configurable\":true,\"enumerable\":true,"
        "\"isOwn\":true},{\"name\":\"32\",\"value\":{\"type\":\"object\",\"className\":\"Object\","
        "\"unserializableValue\":\"Object\",\"description\":\"Object\",\"objectId\":\"90\"},\"writable\":true,"
        "\"configurable\":true,\"enumerable\":true,\"isOwn\":true},{\"name\":\"33\",\"value\":{\"type\":\"object\","
        "\"className\":\"Object\",\"unserializableValue\":\"Object\",\"description\":\"Object\",\"objectId\":\"91\"},"
        "\"writable\":true,\"configurable\":true,\"enumerable\":true,\"isOwn\":true},{\"name\":\"34\","
        "\"value\":{\"type\":\"object\",\"className\":\"Object\",\"unserializableValue\":\"Object\","
        "\"description\":\"Object\",\"objectId\":\"92\"},\"writable\":true,\"configurable\":true,\"enumerable\":true,"
        "\"isOwn\":true},{\"name\":\"35\",\"value\":{\"type\":\"object\",\"className\":\"Object\","
        "\"unserializableValue\":\"Object\",\"description\":\"Object\",\"objectId\":\"93\"},\"writable\":true,"
        "\"configurable\":true,\"enumerable\":true,\"isOwn\":true},{\"name\":\"36\",\"value\":{\"type\":\"object\","
        "\"className\":\"Object\",\"unserializableValue\":\"Object\",\"description\":\"Object\",\"objectId\":\"94\"},"
        "\"writable\":true,\"configurable\":true,\"enumerable\":true,\"isOwn\":true},{\"name\":\"37\","
        "\"value\":{\"type\":\"object\",\"className\":\"Object\",\"unserializableValue\":\"Object\","
        "\"description\":\"Object\",\"objectId\":\"95\"},\"writable\":true,\"configurable\":true,\"enumerable\":true,"
        "\"isOwn\":true},{\"name\":\"38\",\"value\":{\"type\":\"object\",\"className\":\"Object\","
        "\"unserializableValue\":\"Object\",\"description\":\"Object\",\"objectId\":\"96\"},\"writable\":true,"
        "\"configurable\":true,\"enumerable\":true,\"isOwn\":true},{\"name\":\"39\",\"value\":{\"type\":\"object\","
        "\"className\":\"Object\",\"unserializableValue\":\"Object\",\"description\":\"Object\",\"objectId\":\"97\"},"
        "\"writable\":true,\"configurable\":true,\"enumerable\":true,\"isOwn\":true},{\"name\":\"40\","
        "\"value\":{\"type\":\"object\",\"className\":\"Object\",\"unserializableValue\":\"Object\","
        "\"description\":\"Object\",\"objectId\":\"98\"},\"writable\":true,\"configurable\":true,\"enumerable\":true,"
        "\"isOwn\":true},{\"name\":\"41\",\"value\":{\"type\":\"object\",\"className\":\"Object\","
        "\"unserializableValue\":\"Object\",\"description\":\"Object\",\"objectId\":\"99\"},\"writable\":true,"
        "\"configurable\":true,\"enumerable\":true,\"isOwn\":true},{\"name\":\"42\",\"value\":{\"type\":\"object\","
        "\"className\":\"Object\",\"unserializableValue\":\"Object\",\"description\":\"Object\",\"objectId\":\"100\"},"
        "\"writable\":true,\"configurable\":true,\"enumerable\":true,\"isOwn\":true},{\"name\":\"43\","
        "\"value\":{\"type\":\"object\",\"className\":\"Object\",\"unserializableValue\":\"Object\","
        "\"description\":\"Object\",\"objectId\":\"101\"},\"writable\":true,\"configurable\":true,\"enumerable\":true,"
        "\"isOwn\":true},{\"name\":\"44\",\"value\":{\"type\":\"object\",\"className\":\"Object\","
        "\"unserializableValue\":\"Object\",\"description\":\"Object\",\"objectId\":\"102\"},\"writable\":true,"
        "\"configurable\":true,\"enumerable\":true,\"isOwn\":true},{\"name\":\"45\",\"value\":{\"type\":\"object\","
        "\"className\":\"Object\",\"unserializableValue\":\"Object\",\"description\":\"Object\",\"objectId\":\"103\"},"
        "\"writable\":true,\"configurable\":true,\"enumerable\":true,\"isOwn\":true},{\"name\":\"46\","
        "\"value\":{\"type\":\"object\",\"className\":\"Object\",\"unserializableValue\":\"Object\","
        "\"description\":\"Object\",\"objectId\":\"104\"},\"writable\":true,\"configurable\":true,\"enumerable\":true,"
        "\"isOwn\":true},{\"name\":\"47\",\"value\":{\"type\":\"object\",\"className\":\"Object\","
        "\"unserializableValue\":\"Object\",\"description\":\"Object\",\"objectId\":\"105\"},\"writable\":true,"
        "\"configurable\":true,\"enumerable\":true,\"isOwn\":true},{\"name\":\"48\",\"value\":{\"type\":\"object\","
        "\"className\":\"Object\",\"unserializableValue\":\"Object\",\"description\":\"Object\",\"objectId\":\"106\"},"
        "\"writable\":true,\"configurable\":true,\"enumerable\":true,\"isOwn\":true},{\"name\":\"49\","
        "\"value\":{\"type\":\"object\",\"className\":\"Object\",\"unserializableValue\":\"Object\","
        "\"description\":\"Object\",\"objectId\":\"107\"},\"writable\":true,\"configurable\":true,\"enumerable\":true,"
        "\"isOwn\":true},{\"name\":\"50\",\"value\":{\"type\":\"object\",\"className\":\"Object\","
        "\"unserializableValue\":\"Object\",\"description\":\"Object\",\"objectId\":\"108\"},\"writable\":true,"
        "\"configurable\":true,\"enumerable\":true,\"isOwn\":true},{\"name\":\"51\",\"value\":{\"type\":\"object\","
        "\"className\":\"Object\",\"unserializableValue\":\"Object\",\"description\":\"Object\",\"objectId\":\"109\"},"
        "\"writable\":true,\"configurable\":true,\"enumerable\":true,\"isOwn\":true},{\"name\":\"52\","
        "\"value\":{\"type\":\"object\",\"className\":\"Object\",\"unserializableValue\":\"Object\","
        "\"description\":\"Object\",\"objectId\":\"110\"},\"writable\":true,\"configurable\":true,\"enumerable\":true,"
        "\"isOwn\":true},{\"name\":\"53\",\"value\":{\"type\":\"object\",\"className\":\"Object\","
        "\"unserializableValue\":\"Object\",\"description\":\"Object\",\"objectId\":\"111\"},\"writable\":true,"
        "\"configurable\":true,\"enumerable\":true,\"isOwn\":true},{\"name\":\"54\",\"value\":{\"type\":\"object\","
        "\"className\":\"Object\",\"unserializableValue\":\"Object\",\"description\":\"Object\",\"objectId\":\"112\"},"
        "\"writable\":true,\"configurable\":true,\"enumerable\":true,\"isOwn\":true},{\"name\":\"55\","
        "\"value\":{\"type\":\"object\",\"className\":\"Object\",\"unserializableValue\":\"Object\","
        "\"description\":\"Object\",\"objectId\":\"113\"},\"writable\":true,\"configurable\":true,\"enumerable\":true,"
        "\"isOwn\":true},{\"name\":\"56\",\"value\":{\"type\":\"object\",\"className\":\"Object\","
        "\"unserializableValue\":\"Object\",\"description\":\"Object\",\"objectId\":\"114\"},\"writable\":true,"
        "\"configurable\":true,\"enumerable\":true,\"isOwn\":true},{\"name\":\"57\",\"value\":{\"type\":\"object\","
        "\"className\":\"Object\",\"unserializableValue\":\"Object\",\"description\":\"Object\",\"objectId\":\"115\"},"
        "\"writable\":true,\"configurable\":true,\"enumerable\":true,\"isOwn\":true},{\"name\":\"58\","
        "\"value\":{\"type\":\"object\",\"className\":\"Object\",\"unserializableValue\":\"Object\","
        "\"description\":\"Object\",\"objectId\":\"116\"},\"writable\":true,\"configurable\":true,\"enumerable\":true,"
        "\"isOwn\":true},{\"name\":\"59\",\"value\":{\"type\":\"object\",\"className\":\"Object\","
        "\"unserializableValue\":\"Object\",\"description\":\"Object\",\"objectId\":\"117\"},\"writable\":true,"
        "\"configurable\":true,\"enumerable\":true,\"isOwn\":true},{\"name\":\"60\",\"value\":{\"type\":\"object\","
        "\"className\":\"Object\",\"unserializableValue\":\"Object\",\"description\":\"Object\",\"objectId\":\"118\"},"
        "\"writable\":true,\"configurable\":true,\"enumerable\":true,\"isOwn\":true},{\"name\":\"61\","
        "\"value\":{\"type\":\"object\",\"className\":\"Object\",\"unserializableValue\":\"Object\","
        "\"description\":\"Object\",\"objectId\":\"119\"},\"writable\":true,\"configurable\":true,"
        "\"enumerable\":true,\"isOwn\":true},{\"name\":\"62\",\"value\":{\"type\":\"object\","
        "\"className\":\"Object\",\"unserializableValue\":\"Object\",\"description\":\"Object\",\"objectId\":\"120\"},"
        "\"writable\":true,\"configurable\":true,\"enumerable\":true,\"isOwn\":true},{\"name\":\"63\","
        "\"value\":{\"type\":\"object\",\"className\":\"Object\",\"unserializableValue\":\"Object\","
        "\"description\":\"Object\",\"objectId\":\"121\"},\"writable\":true,\"configurable\":true,"
        "\"enumerable\":true,\"isOwn\":true},{\"name\":\"64\",\"value\":{\"type\":\"object\","
        "\"className\":\"Object\",\"unserializableValue\":\"Object\",\"description\":\"Object\","
        "\"objectId\":\"122\"},\"writable\":true,\"configurable\":true,\"enumerable\":true,\"isOwn\":true},"
        "{\"name\":\"65\",\"value\":{\"type\":\"object\",\"className\":\"Object\",\"unserializableValue\":\"Object\","
        "\"description\":\"Object\",\"objectId\":\"123\"},\"writable\":true,\"configurable\":true,"
        "\"enumerable\":true,\"isOwn\":true},{\"name\":\"66\",\"value\":{\"type\":\"object\",\"className\":\"Object\","
        "\"unserializableValue\":\"Object\",\"description\":\"Object\",\"objectId\":\"124\"},\"writable\":true,"
        "\"configurable\":true,\"enumerable\":true,\"isOwn\":true},{\"name\":\"67\",\"value\":{\"type\":\"object\","
        "\"className\":\"Object\",\"unserializableValue\":\"Object\",\"description\":\"Object\",\"objectId\":\"125\"},"
        "\"writable\":true,\"configurable\":true,\"enumerable\":true,\"isOwn\":true},{\"name\":\"68\","
        "\"value\":{\"type\":\"object\",\"className\":\"Object\",\"unserializableValue\":\"Object\","
        "\"description\":\"Object\",\"objectId\":\"126\"},\"writable\":true,\"configurable\":true,"
        "\"enumerable\":true,\"isOwn\":true},{\"name\":\"69\",\"value\":{\"type\":\"object\","
        "\"className\":\"Object\",\"unserializableValue\":\"Object\",\"description\":\"Object\",\"objectId\":\"127\"},"
        "\"writable\":true,\"configurable\":true,\"enumerable\":true,\"isOwn\":true},{\"name\":\"70\","
        "\"value\":{\"type\":\"object\",\"className\":\"Object\",\"unserializableValue\":\"Object\","
        "\"description\":\"Object\",\"objectId\":\"128\"},\"writable\":true,\"configurable\":true,"
        "\"enumerable\":true,\"isOwn\":true},{\"name\":\"71\",\"value\":{\"type\":\"object\","
        "\"className\":\"Object\",\"unserializableValue\":\"Object\",\"description\":\"Object\","
        "\"objectId\":\"129\"},\"writable\":true,\"configurable\":true,\"enumerable\":true,\"isOwn\":true},"
        "{\"name\":\"72\",\"value\":{\"type\":\"object\",\"className\":\"Object\","
        "\"unserializableValue\":\"Object\",\"description\":\"Object\",\"objectId\":\"130\"},\"writable\":true,"
        "\"configurable\":true,\"enumerable\":true,\"isOwn\":true},{\"name\":\"73\","
        "\"value\":{\"type\":\"object\",\"className\":\"Object\",\"unserializableValue\":\"Object\","
        "\"description\":\"Object\",\"objectId\":\"131\"},\"writable\":true,\"configurable\":true,"
        "\"enumerable\":true,\"isOwn\":true},{\"name\":\"74\",\"value\":{\"type\":\"object\","
        "\"className\":\"Object\",\"unserializableValue\":\"Object\",\"description\":\"Object\","
        "\"objectId\":\"132\"},\"writable\":true,\"configurable\":true,\"enumerable\":true,\"isOwn\":true},"
        "{\"name\":\"75\",\"value\":{\"type\":\"object\",\"className\":\"Object\","
        "\"unserializableValue\":\"Object\",\"description\":\"Object\",\"objectId\":\"133\"},"
        "\"writable\":true,\"configurable\":true,\"enumerable\":true,\"isOwn\":true},{\"name\":\"76\","
        "\"value\":{\"type\":\"object\",\"className\":\"Object\",\"unserializableValue\":\"Object\","
        "\"description\":\"Object\",\"objectId\":\"134\"},\"writable\":true,\"configurable\":true,"\
        "\"enumerable\":true,\"isOwn\":true},{\"name\":\"77\",\"value\":{\"type\":\"object\","
        "\"className\":\"Object\",\"unserializableValue\":\"Object\",\"description\":\"Object\","
        "\"objectId\":\"135\"},\"writable\":true,\"configurable\":true,\"enumerable\":true,\"isOwn\":true},"
        "{\"name\":\"78\",\"value\":{\"type\":\"object\",\"className\":\"Object\","
        "\"unserializableValue\":\"Object\",\"description\":\"Object\",\"objectId\":\"136\"},"
        "\"writable\":true,\"configurable\":true,\"enumerable\":true,\"isOwn\":true},{\"name\":\"79\","
        "\"value\":{\"type\":\"object\",\"className\":\"Object\",\"unserializableValue\":\"Object\","
        "\"description\":\"Object\",\"objectId\":\"137\"},\"writable\":true,\"configurable\":true,"
        "\"enumerable\":true,\"isOwn\":true},{\"name\":\"80\",\"value\":{\"type\":\"object\","
        "\"className\":\"Object\",\"unserializableValue\":\"Object\",\"description\":\"Object\","
        "\"objectId\":\"138\"},\"writable\":true,\"configurable\":true,\"enumerable\":true,\"isOwn\":true},"
        "{\"name\":\"81\",\"value\":{\"type\":\"object\",\"className\":\"Object\","
        "\"unserializableValue\":\"Object\",\"description\":\"Object\",\"objectId\":\"139\"},"
        "\"writable\":true,\"configurable\":true,\"enumerable\":true,\"isOwn\":true},{\"name\":\"82\","
        "\"value\":{\"type\":\"object\",\"className\":\"Object\",\"unserializableValue\":\"Object\","
        "\"description\":\"Object\",\"objectId\":\"140\"},\"writable\":true,\"configurable\":true,"
        "\"enumerable\":true,\"isOwn\":true},{\"name\":\"83\",\"value\":{\"type\":\"object\","
        "\"className\":\"Object\",\"unserializableValue\":\"Object\",\"description\":\"Object\","
        "\"objectId\":\"141\"},\"writable\":true,\"configurable\":true,\"enumerable\":true,"
        "\"isOwn\":true},{\"name\":\"84\",\"value\":{\"type\":\"object\",\"className\":\"Object\","
        "\"unserializableValue\":\"Object\",\"description\":\"Object\",\"objectId\":\"142\"},"
        "\"writable\":true,\"configurable\":true,\"enumerable\":true,\"isOwn\":true},"
        "{\"name\":\"85\",\"value\":{\"type\":\"object\",\"className\":\"Object\","
        "\"unserializableValue\":\"Object\",\"description\":\"Object\",\"objectId\":\"143\"},"
        "\"writable\":true,\"configurable\":true,\"enumerable\":true,\"isOwn\":true},{\"name\":\"86\","
        "\"value\":{\"type\":\"object\",\"className\":\"Object\",\"unserializableValue\":\"Object\","
        "\"description\":\"Object\",\"objectId\":\"144\"},\"writable\":true,\"configurable\":true,"
        "\"enumerable\":true,\"isOwn\":true},{\"name\":\"87\",\"value\":{\"type\":\"object\","
        "\"className\":\"Object\",\"unserializableValue\":\"Object\",\"description\":\"Object\","
        "\"objectId\":\"145\"},\"writable\":true,\"configurable\":true,\"enumerable\":true,"
        "\"isOwn\":true},{\"name\":\"88\",\"value\":{\"type\":\"object\",\"className\":\"Object\","
        "\"unserializableValue\":\"Object\",\"description\":\"Object\",\"objectId\":\"146\"},"
        "\"writable\":true,\"configurable\":true,\"enumerable\":true,\"isOwn\":true},{\"name\":\"89\","
        "\"value\":{\"type\":\"object\",\"className\":\"Object\",\"unserializableValue\":\"Object\","
        "\"description\":\"Object\",\"objectId\":\"147\"},\"writable\":true,\"configurable\":true,"
        "\"enumerable\":true,\"isOwn\":true},{\"name\":\"90\",\"value\":{\"type\":\"object\","
        "\"className\":\"Object\",\"unserializableValue\":\"Object\",\"description\":\"Object\","
        "\"objectId\":\"148\"},\"writable\":true,\"configurable\":true,\"enumerable\":true,\"isOwn\":true},"
        "{\"name\":\"91\",\"value\":{\"type\":\"object\",\"className\":\"Object\","
        "\"unserializableValue\":\"Object\",\"description\":\"Object\",\"objectId\":\"149\"},"
        "\"writable\":true,\"configurable\":true,\"enumerable\":true,\"isOwn\":true},{\"name\":\"92\","
        "\"value\":{\"type\":\"object\",\"className\":\"Object\",\"unserializableValue\":\"Object\","
        "\"description\":\"Object\",\"objectId\":\"150\"},\"writable\":true,\"configurable\":true,"
        "\"enumerable\":true,\"isOwn\":true},{\"name\":\"93\",\"value\":{\"type\":\"object\","
        "\"className\":\"Object\",\"unserializableValue\":\"Object\",\"description\":\"Object\","
        "\"objectId\":\"151\"},\"writable\":true,\"configurable\":true,\"enumerable\":true,\"isOwn\":true},"
        "{\"name\":\"94\",\"value\":{\"type\":\"object\",\"className\":\"Object\","
        "\"unserializableValue\":\"Object\",\"description\":\"Object\",\"objectId\":\"152\"},"
        "\"writable\":true,\"configurable\":true,\"enumerable\":true,\"isOwn\":true},{\"name\":\"95\","
        "\"value\":{\"type\":\"object\",\"className\":\"Object\",\"unserializableValue\":\"Object\","
        "\"description\":\"Object\",\"objectId\":\"153\"},\"writable\":true,\"configurable\":true,"
        "\"enumerable\":true,\"isOwn\":true},{\"name\":\"96\",\"value\":{\"type\":\"object\","
        "\"className\":\"Object\",\"unserializableValue\":\"Object\",\"description\":\"Object\","
        "\"objectId\":\"154\"},\"writable\":true,\"configurable\":true,\"enumerable\":true,"
        "\"isOwn\":true},{\"name\":\"97\",\"value\":{\"type\":\"object\",\"className\":\"Object\","
        "\"unserializableValue\":\"Object\",\"description\":\"Object\",\"objectId\":\"155\"},"
        "\"writable\":true,\"configurable\":true,\"enumerable\":true,\"isOwn\":true},{\"name\":\"98\","
        "\"value\":{\"type\":\"object\",\"className\":\"Object\",\"unserializableValue\":\"Object\","
        "\"description\":\"Object\",\"objectId\":\"156\"},\"writable\":true,\"configurable\":true,"
        "\"enumerable\":true,\"isOwn\":true},{\"name\":\"99\",\"value\":{\"type\":\"object\","
        "\"className\":\"Object\",\"unserializableValue\":\"Object\",\"description\":\"Object\","
        "\"objectId\":\"157\"},\"writable\":true,\"configurable\":true,\"enumerable\":true,"
        "\"isOwn\":true},{\"name\":\"length\",\"value\":{\"type\":\"number\","
        "\"unserializableValue\":\"100\",\"description\":\"100\"},\"writable\":true,"
        "\"configurable\":false,\"enumerable\":false,\"isOwn\":true}]}}",

        "{\"id\":16,\"result\":{\"result\":[{\"name\":\"0\",\"value\":{\"type\":\"number\","
        "\"unserializableValue\":\"0\",\"description\":\"0\"},\"writable\":true,\"configurable\":true,"
        "\"enumerable\":true,\"isOwn\":true},{\"name\":\"1\",\"value\":{\"type\":\"number\","
        "\"unserializableValue\":\"1\",\"description\":\"1\"},\"writable\":true,\"configurable\":true,"
        "\"enumerable\":true,\"isOwn\":true},{\"name\":\"2\",\"value\":{\"type\":\"number\","
        "\"unserializableValue\":\"2\",\"description\":\"2\"},\"writable\":true,\"configurable\":true,"
        "\"enumerable\":true,\"isOwn\":true},{\"name\":\"3\",\"value\":{\"type\":\"number\","
        "\"unserializableValue\":\"3\",\"description\":\"3\"},\"writable\":true,\"configurable\":true,"
        "\"enumerable\":true,\"isOwn\":true},{\"name\":\"4\",\"value\":{\"type\":\"number\","
        "\"unserializableValue\":\"4\",\"description\":\"4\"},\"writable\":true,\"configurable\":true,"
        "\"enumerable\":true,\"isOwn\":true},{\"name\":\"5\",\"value\":{\"type\":\"number\","
        "\"unserializableValue\":\"5\",\"description\":\"5\"},\"writable\":true,\"configurable\":true,"
        "\"enumerable\":true,\"isOwn\":true},{\"name\":\"6\",\"value\":{\"type\":\"number\","
        "\"unserializableValue\":\"6\",\"description\":\"6\"},\"writable\":true,\"configurable\":true,"
        "\"enumerable\":true,\"isOwn\":true},{\"name\":\"7\",\"value\":{\"type\":\"number\","
        "\"unserializableValue\":\"7\",\"description\":\"7\"},\"writable\":true,\"configurable\":true,"
        "\"enumerable\":true,\"isOwn\":true},{\"name\":\"8\",\"value\":{\"type\":\"number\","
        "\"unserializableValue\":\"8\",\"description\":\"8\"},\"writable\":true,\"configurable\":true,"
        "\"enumerable\":true,\"isOwn\":true},{\"name\":\"9\",\"value\":{\"type\":\"number\","
        "\"unserializableValue\":\"9\",\"description\":\"9\"},\"writable\":true,\"configurable\":true,"
        "\"enumerable\":true,\"isOwn\":true}]}}",
 
        "{\"id\":17,\"result\":{\"result\":[{\"name\":\"100\",\"value\":{\"type\":\"number\","
        "\"unserializableValue\":\"100\",\"description\":\"100\"},\"writable\":true,\"configurable\":true,"
        "\"enumerable\":true,\"isOwn\":true},{\"name\":\"101\",\"value\":{\"type\":\"number\","
        "\"unserializableValue\":\"101\",\"description\":\"101\"},\"writable\":true,\"configurable\":true,"
        "\"enumerable\":true,\"isOwn\":true},{\"name\":\"102\",\"value\":{\"type\":\"number\","
        "\"unserializableValue\":\"102\",\"description\":\"102\"},\"writable\":true,\"configurable\":true,"
        "\"enumerable\":true,\"isOwn\":true},{\"name\":\"103\",\"value\":{\"type\":\"number\","
        "\"unserializableValue\":\"103\",\"description\":\"103\"},\"writable\":true,\"configurable\":true,"
        "\"enumerable\":true,\"isOwn\":true},{\"name\":\"104\",\"value\":{\"type\":\"number\","
        "\"unserializableValue\":\"104\",\"description\":\"104\"},\"writable\":true,\"configurable\":true,"
        "\"enumerable\":true,\"isOwn\":true},{\"name\":\"105\",\"value\":{\"type\":\"number\","
        "\"unserializableValue\":\"105\",\"description\":\"105\"},\"writable\":true,\"configurable\":true,"
        "\"enumerable\":true,\"isOwn\":true}]}}",
 
        "{\"id\":18,\"result\":{\"result\":[{\"name\":\"145\",\"value\":{\"type\":\"number\","
        "\"unserializableValue\":\"1.100000023841858\",\"description\":\"1.100000023841858\"},"
        "\"writable\":true,\"configurable\":true,\"enumerable\":true,\"isOwn\":true},{\"name\":\"146\","
        "\"value\":{\"type\":\"number\",\"unserializableValue\":\"1.100000023841858\","
        "\"description\":\"1.100000023841858\"},\"writable\":true,\"configurable\":true,\"enumerable\":true,"
        "\"isOwn\":true},{\"name\":\"147\",\"value\":{\"type\":\"number\","
        "\"unserializableValue\":\"1.100000023841858\","
        "\"description\":\"1.100000023841858\"},\"writable\":true,\"configurable\":true,\"enumerable\":true,"
        "\"isOwn\":true},{\"name\":\"148\",\"value\":{\"type\":\"number\","
        "\"unserializableValue\":\"1.100000023841858\",\"description\":\"1.100000023841858\"},\"writable\":true,"
        "\"configurable\":true,\"enumerable\":true,\"isOwn\":true},{\"name\":\"149\",\"value\":{\"type\":\"number\","
        "\"unserializableValue\":\"1.100000023841858\",\"description\":\"1.100000023841858\"},\"writable\":true,"
        "\"configurable\":true,\"enumerable\":true,\"isOwn\":true}]}}",
 
        "{\"id\":19,\"result\":{\"result\":[{\"name\":\"99\",\"value\":{\"type\":\"bigint\","
        "\"unserializableValue\":\"10000000n\",\"description\":\"10000000n\"},\"writable\":true,\"configurable\":true,"
        "\"enumerable\":true,\"isOwn\":true}]}}"
    };
};
 
std::unique_ptr<TestActions> GetJsVariablePropertyWithRangeTest()
{
    return std::make_unique<JsVariablePropertyWithRangeTest>();
}
}  // namespace panda::ecmascript::tooling::test
 
#endif  // ECMASCRIPT_TOOLING_TEST_TESTCASES_JS_VARIABLE_PROPERTY_WITH_RANGE_TEST_H