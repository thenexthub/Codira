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

#ifndef ECMASCRIPT_TOOLING_TEST_TESTCASES_JS_CONTAINER_TEST_H
#define ECMASCRIPT_TOOLING_TEST_TESTCASES_JS_CONTAINER_TEST_H

#include "tooling/dynamic/test/client_utils/test_util.h"

namespace panda::ecmascript::tooling::test {
class JsContainerTest : public TestActions {
public:
    JsContainerTest()
    {
        testAction = {
            {SocketAction::SEND, "enable"},
            {SocketAction::RECV, "", ActionRule::CUSTOM_RULE, MatchRule::replySuccess},
            {SocketAction::SEND, "runtime-enable"},
            {SocketAction::RECV, "", ActionRule::CUSTOM_RULE, MatchRule::replySuccess},
            {SocketAction::SEND, "run"},
            {SocketAction::RECV, "", ActionRule::CUSTOM_RULE, MatchRule::replySuccess},
            // load sample.js
            {SocketAction::RECV, "Debugger.scriptParsed", ActionRule::STRING_CONTAIN},
            // break on start
            {SocketAction::RECV, "Debugger.paused", ActionRule::STRING_CONTAIN},
            // set breakpoint
            {SocketAction::SEND, "b " DEBUGGER_JS_DIR "container.js 368"},
            {SocketAction::RECV, "", ActionRule::CUSTOM_RULE, MatchRule::replySuccess},
            // hit breakpoint after resume first time
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
                    breakpoint.find("367") == std::string::npos) {
                    return false;
                }

                DebuggerClient debuggerClient(0);
                debuggerClient.PausedReply(std::move(json));
                return true;
            }},

            {SocketAction::SEND, "print"},
            {SocketAction::RECV, "", ActionRule::CUSTOM_RULE, [this] (auto recv, auto, auto) -> bool {
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
                std::unique_ptr<PtJson> value;
                std::string type;
                std::string className;
                std::string unserializableValue;
                std::string description;
                for (int32_t i = 0; i < innerResult->GetSize(); i++) {
                    std::vector<std::string> infos;
                    ret = innerResult->Get(i)->GetString("name", &name);
                    if (ret != Result::SUCCESS) {
                        return false;
                    }

                    ret = innerResult->Get(i)->GetObject("value", &value);
                    if (ret != Result::SUCCESS) {
                        return false;
                    }

                    ret = value->GetString("type", &type);
                    if (ret == Result::SUCCESS) {
                        infos.push_back(type);
                    }

                    ret = value->GetString("className", &className);
                    if (ret == Result::SUCCESS) {
                        infos.push_back(className);
                    }

                    ret = value->GetString("unserializableValue", &unserializableValue);
                    if (ret == Result::SUCCESS) {
                        infos.push_back(unserializableValue);
                    }

                    ret = value->GetString("description", &description);
                    if (ret == Result::SUCCESS) {
                        infos.push_back(description);
                    }

                    for (uint32_t j = 0; j < infos.size(); j++) {
                        if (infos[j] != this->variableMap_.at(name)[j]) {
                            return false;
                        }
                    }
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
    ~JsContainerTest() = default;

private:
    std::string pandaFile_ = DEBUGGER_ABC_DIR "container.abc";
    std::string sourceFile_ = DEBUGGER_JS_DIR "container.js";
    std::string entryPoint_ = "container";

    const std::map<std::string, std::vector<std::string>> variableMap_ = {
        { "nop", { "undefined" } },
        { "foo", { "function", "Function", "function foo( { [js code] }",
            "function foo( { [js code] }" } },
        { "ArrayList", { "function", "Function", "function ArrayList( { [native code] }",
            "function ArrayList( { [native code] }" } },
        { "Deque", { "function", "Function", "function Deque( { [native code] }",
            "function Deque( { [native code] }" } },
        { "HashMap", { "function", "Function", "function HashMap( { [native code] }",
            "function HashMap( { [native code] }" } },
        { "HashSet", { "function", "Function", "function HashSet( { [native code] }",
            "function HashSet( { [native code] }" } },
        { "LightWeightMap", { "function", "Function", "function LightWeightMap( { [native code] }",
            "function LightWeightMap( { [native code] }" } },
        { "LightWeightSet", { "function", "Function", "function LightWeightSet( { [native code] }",
            "function LightWeightSet( { [native code] }" } },
        { "LinkedList", { "function", "Function", "function LinkedList( { [native code] }",
            "function LinkedList( { [native code] }" } },
        { "List", { "function", "Function", "function List( { [native code] }",
            "function List( { [native code] }" } },
        { "PlainArray", { "function", "Function", "function PlainArray( { [native code] }",
            "function PlainArray( { [native code] }" } },
        { "Queue", { "function", "Function", "function Queue( { [native code] }",
            "function Queue( { [native code] }" } },
        { "Stack", { "function", "Function", "function Stack( { [native code] }",
            "function Stack( { [native code] }" } },
        { "TreeMap", { "function", "Function", "function TreeMap( { [native code] }",
            "function TreeMap( { [native code] }" } },
        { "TreeSet", { "function", "Function", "function TreeSet( { [native code] }",
            "function TreeSet( { [native code] }" } },
        { "Vector", { "function", "Function", "function Vector( { [native code] }",
            "function Vector( { [native code] }" } },
        { "arrayList", { "object", "Object", "ArrayList", "ArrayList" } },
        { "deque", { "object", "Object", "Deque", "Deque" } },
        { "hashMap", { "object", "Object", "HashMap", "HashMap" } },
        { "hashSet", { "object", "Object", "HashSet", "HashSet" } },
        { "lightWeightMap", { "object", "Object", "LightWeightMap", "LightWeightMap" } },
        { "lightWeightSet", { "object", "Object", "LightWeightSet", "LightWeightSet" } },
        { "linkedList", { "object", "Object", "LinkedList", "LinkedList" } },
        { "list", { "object", "Object", "List", "List" } },
        { "plainArray", { "object", "Object", "PlainArray", "PlainArray" } },
        { "queue", { "object", "Object", "Queue", "Queue" } },
        { "stack", { "object", "Object", "Stack", "Stack" } },
        { "treeMap", { "object", "Object", "TreeMap", "TreeMap" } },
        { "treeSet", { "object", "Object", "TreeSet", "TreeSet" } },
        { "vector", { "object", "Object", "Vector", "Vector" } },
        { "boolean0", { "boolean", "false", "false" } },
        { "boolean1", { "boolean", "true", "true" } },
        { "boolean2", { "boolean", "false", "false" } },
        { "boolean3", { "boolean", "true", "true" } },
        { "number0", { "number", "1888", "1888" } },
        { "number1", { "number", "177", "177" } },
        { "number2", { "number", "-1", "-1" } },
        { "number3", { "number", "-1", "-1" } },
        { "number4", { "number", "222", "222" } },
        { "number5", { "number", "-2", "-2" } },
        { "number6", { "number", "566", "566" } },
        { "number7", { "number", "588", "588" } },
        { "number8", { "number", "588", "588" } },
        { "number9", { "number", "0", "0" } },
        { "number10", { "number", "-1", "-1" } },
        { "number11", { "number", "88", "88" } },
        { "number12", { "number", "18", "18" } },
        { "number13", { "number", "322", "322" } },
        { "number14", { "number", "8", "8" } },
        { "number15", { "number", "4", "4" } },
        { "number16", { "number", "8", "8" } },
        { "number17", { "number", "5", "5" } },
        { "number18", { "number", "188", "188" } },
        { "number19", { "number", "8", "8" } },
        { "number20", { "number", "388", "388" } },
        { "number21", { "number", "0", "0" } },
        { "number22", { "number", "5", "5" } },
        { "number23", { "number", "18", "18" } },
        { "number24", { "number", "1388", "1388" } },
        { "number25", { "number", "6", "6" } },
        { "number26", { "number", "0", "0" } },
        { "number27", { "number", "857", "857" } },
        { "string0", { "string", "two", "two" } },
        { "string1", { "string", "change", "change" } },
        { "string2", { "string", "three", "three" } },
        { "string3", { "string", "array", "array" } },
        { "string4", { "string", "male", "male" } },
        { "string5", { "string", "one", "one" } },
        { "sendableArray", { "object", "Object", "SendableArray [Sendable]", "SendableArray [Sendable]" } },
        { "sendableArray0", { "number", "1", "1" } },
        { "sendableArray1", { "number", "2", "2" } },
        { "sendableArraySize", { "number", "2", "2" } },
        { "sendableMap", { "object", "Object", "SendableMap [Sendable]", "SendableMap [Sendable]" } },
        { "sendableMap0", { "number", "1", "1" } },
        { "sendableMap1", { "number", "2", "2" } },
        { "sendableMapSize", { "number", "2", "2" } },
        { "sendableSet", { "object", "Object", "SendableSet [Sendable]", "SendableSet [Sendable]" } },
        { "sendableSet0", { "boolean", "true", "true" } },
        { "sendableSet1", { "boolean", "true", "true" } },
        { "sendableSetSize", { "number", "2", "2" } },
        };
};

std::unique_ptr<TestActions> GetJsContainerTest()
{
    return std::make_unique<JsContainerTest>();
}
}  // namespace panda::ecmascript::tooling::test

#endif  // ECMASCRIPT_TOOLING_TEST_TESTCASES_JS_CONTAINER_TEST_H
