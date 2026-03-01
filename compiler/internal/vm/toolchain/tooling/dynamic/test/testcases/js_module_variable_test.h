/**
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

#ifndef ECMASCRIPT_TOOLING_TEST_TESTCASES_JS_MODULE_VARIABLE_TEST_H
#define ECMASCRIPT_TOOLING_TEST_TESTCASES_JS_MODULE_VARIABLE_TEST_H

#include "tooling/dynamic/test/client_utils/test_util.h"

namespace panda::ecmascript::tooling::test {
class JsModuleVariableTest : public TestActions {
public:
    JsModuleVariableTest()
    {
        testAction = {
            {SocketAction::SEND, "enable"},
            {SocketAction::RECV, "", ActionRule::CUSTOM_RULE, MatchRule::replySuccess},
            {SocketAction::SEND, "runtime-enable"},
            {SocketAction::RECV, "", ActionRule::CUSTOM_RULE, MatchRule::replySuccess},
            {SocketAction::SEND, "run"},
            {SocketAction::RECV, "", ActionRule::CUSTOM_RULE, MatchRule::replySuccess},
            // load export_variable_second.js
            {SocketAction::RECV, "Debugger.scriptParsed", ActionRule::STRING_CONTAIN},
            {SocketAction::RECV, "Debugger.paused", ActionRule::STRING_CONTAIN},
            {SocketAction::SEND, "resume"},
            {SocketAction::RECV, "Debugger.resumed", ActionRule::STRING_CONTAIN},
            {SocketAction::RECV, "", ActionRule::CUSTOM_RULE, MatchRule::replySuccess},
            // load export_variable_first.js
            {SocketAction::RECV, "Debugger.scriptParsed", ActionRule::STRING_CONTAIN},
            {SocketAction::RECV, "Debugger.paused", ActionRule::STRING_CONTAIN},
            {SocketAction::SEND, "resume"},
            {SocketAction::RECV, "Debugger.resumed", ActionRule::STRING_CONTAIN},
            {SocketAction::RECV, "", ActionRule::CUSTOM_RULE, MatchRule::replySuccess},
            // load module_variable.js
            {SocketAction::RECV, "Debugger.scriptParsed", ActionRule::STRING_CONTAIN},
            // break on start
            {SocketAction::RECV, "Debugger.paused", ActionRule::STRING_CONTAIN},
            // set breakpoint
            {SocketAction::SEND, "b " DEBUGGER_JS_DIR"module_variable.js 247"},
            {SocketAction::RECV, "", ActionRule::CUSTOM_RULE, MatchRule::replySuccess},

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
                    breakpoint.find("246") == std::string::npos) {
                    return false;
                }

                DebuggerClient debuggerClient(0);
                debuggerClient.PausedReply(std::move(json));
                return true;
            }},

            {SocketAction::SEND, "p"},
            {SocketAction::RECV, "", ActionRule::CUSTOM_RULE, [] (auto recv, auto, auto) -> bool {
                std::unique_ptr<PtJson> json = PtJson::Parse(recv);
                std::unique_ptr<PtJson> result;

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

                RuntimeClient runtimeClient(0);
                runtimeClient.HandleGetProperties(std::move(json), id);
                return true;
            }},

            {SocketAction::SEND, "p 3"},
            {SocketAction::RECV, "", ActionRule::CUSTOM_RULE, [this] (auto recv, auto, auto) -> bool {
                std::unique_ptr<PtJson> json = PtJson::Parse(recv);
                std::unique_ptr<PtJson> result;

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

                std::vector<std::unique_ptr<PropertyDescriptor>> outPropertyDesc;
                for (int32_t i = 0; i < innerResult->GetSize(); i++) {
                    auto variableInfo = PropertyDescriptor::Create(*(innerResult->Get(i)));
                    outPropertyDesc.emplace_back(std::move(variableInfo));
                }

                std::map<std::string, std::vector<std::string>> actualProperties;
                for (const auto &property : outPropertyDesc) {
                    std::vector<std::string> attributes;
                    RemoteObject *value = property->GetValue();
                    std::cout << " ============================== " << std::endl;
                    std::cout << "variableName: " << property->GetName() << std::endl;
                    PushValueInfo(value, attributes);
                    actualProperties[property->GetName()] = attributes;
                }

                // compare size
                if (actualProperties.size() != moduleVariableMap_.size()) {
                    std::cout << "actualProperties.size() != moduleVariableMap_.size()" << std::endl;
                    std::cout << "actualProperties.size() is: " << actualProperties.size() << std::endl;
                    std::cout << "moduleVariableMap_.size() is: " << moduleVariableMap_.size() << std::endl;
                    return false;
                }

                // compare the properties of each variable
                for (const auto &actualProperty : actualProperties) {
                    const std::string &variableName = actualProperty.first;
                    const std::vector<std::string> &actualAttributes = actualProperty.second;

                    if (moduleVariableMap_.find(variableName) != moduleVariableMap_.end()) {
                        const std::vector<std::string> &expectedAttributes = moduleVariableMap_.at(variableName);
                        if (actualAttributes == expectedAttributes) {
                            continue;
                        } else {
                            std::cout << "Property mismatch: " << variableName << std::endl;
                            return false;
                        }
                    } else {
                        std::cout << "variable name not found: " << variableName << std::endl;
                        return false;
                    }
                }

                RuntimeClient runtimeClient(0);
                runtimeClient.HandleGetProperties(std::move(json), id);
                return true;
            }},

            {SocketAction::SEND, "p 5"},
            {SocketAction::RECV, "", ActionRule::CUSTOM_RULE, [this] (auto recv, auto, auto) -> bool {
                return CompareExpected(recv, innerDes[expectDesIndex]);
            }},

            {SocketAction::SEND, "p 6"},
            {SocketAction::RECV, "", ActionRule::CUSTOM_RULE, [this] (auto recv, auto, auto) -> bool {
                return CompareExpected(recv, innerDes[expectDesIndex]);
            }},

            {SocketAction::SEND, "p 7"},
            {SocketAction::RECV, "", ActionRule::CUSTOM_RULE, [this] (auto recv, auto, auto) -> bool {
                return CompareExpected(recv, innerDes[expectDesIndex]);
            }},

            {SocketAction::SEND, "p 10"},
            {SocketAction::RECV, "", ActionRule::CUSTOM_RULE, [this] (auto recv, auto, auto) -> bool {
                return CompareExpected(recv, innerDes[expectDesIndex]);
            }},

            {SocketAction::SEND, "p 13"},
            {SocketAction::RECV, "", ActionRule::CUSTOM_RULE, [this] (auto recv, auto, auto) -> bool {
                return CompareExpected(recv, innerDes[expectDesIndex]);
            }},

            {SocketAction::SEND, "p 16"},
            {SocketAction::RECV, "", ActionRule::CUSTOM_RULE, [this] (auto recv, auto, auto) -> bool {
                return CompareExpected(recv, innerDes[expectDesIndex]);
            }},

            {SocketAction::SEND, "p 17"},
            {SocketAction::RECV, "", ActionRule::CUSTOM_RULE, [this] (auto recv, auto, auto) -> bool {
                return CompareExpected(recv, innerDes[expectDesIndex]);
            }},

            {SocketAction::SEND, "p 20"},
            {SocketAction::RECV, "", ActionRule::CUSTOM_RULE, [this] (auto recv, auto, auto) -> bool {
                return CompareExpected(recv, innerDes[expectDesIndex]);
            }},

            {SocketAction::SEND, "p 23"},
            {SocketAction::RECV, "", ActionRule::CUSTOM_RULE, [this] (auto recv, auto, auto) -> bool {
                return CompareExpected(recv, innerDes[expectDesIndex]);
            }},

            {SocketAction::SEND, "p 26"},
            {SocketAction::RECV, "", ActionRule::CUSTOM_RULE, [this] (auto recv, auto, auto) -> bool {
                return CompareExpected(recv, innerDes[expectDesIndex]);
            }},

            {SocketAction::SEND, "p 29"},
            {SocketAction::RECV, "", ActionRule::CUSTOM_RULE, [this] (auto recv, auto, auto) -> bool {
                return CompareExpected(recv, innerDes[expectDesIndex]);
            }},

            {SocketAction::SEND, "p 32"},
            {SocketAction::RECV, "", ActionRule::CUSTOM_RULE, [this] (auto recv, auto, auto) -> bool {
                return CompareExpected(recv, innerDes[expectDesIndex]);
            }},

            {SocketAction::SEND, "p 35"},
            {SocketAction::RECV, "", ActionRule::CUSTOM_RULE, [this] (auto recv, auto, auto) -> bool {
                return CompareExpected(recv, innerDes[expectDesIndex]);
            }},

            {SocketAction::SEND, "p 38"},
            {SocketAction::RECV, "", ActionRule::CUSTOM_RULE, [this] (auto recv, auto, auto) -> bool {
                return CompareExpected(recv, innerDes[expectDesIndex]);
            }},

            {SocketAction::SEND, "p 44"},
            {SocketAction::RECV, "", ActionRule::CUSTOM_RULE, [this] (auto recv, auto, auto) -> bool {
                return CompareExpected(recv, innerDes[expectDesIndex]);
            }},

            {SocketAction::SEND, "p 50"},
            {SocketAction::RECV, "", ActionRule::CUSTOM_RULE, [this] (auto recv, auto, auto) -> bool {
                return CompareExpected(recv, innerDes[expectDesIndex]);
            }},

            {SocketAction::SEND, "p 91"},
            {SocketAction::RECV, "", ActionRule::CUSTOM_RULE, [this] (auto recv, auto, auto) -> bool {
                return CompareExpected(recv, innerDes[expectDesIndex]);
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
    ~JsModuleVariableTest() = default;

private:
    std::string pandaFile_ = DEBUGGER_ABC_DIR "module_variable.abc";
    std::string sourceFile_ = DEBUGGER_JS_DIR "module_variable.js";
    std::string entryPoint_ = "module_variable";
    size_t expectDesIndex = 0;

    void PushValueInfo(RemoteObject *value, std::vector<std::string> &infos)
    {
        std::cout << "type: " << value->GetType() << std::endl;
        infos.push_back(value->GetType());
        if (value->HasSubType()) {
            std::cout << "sub type: " << value->GetSubType() << std::endl;
            infos.push_back(value->GetSubType());
        }
        if (value->HasClassName()) {
            std::cout << "class name: " << value->GetClassName() << std::endl;
            infos.push_back(value->GetClassName());
        }
        if (value->HasUnserializableValue()) {
            std::cout << "unserializableValue: " << value->GetUnserializableValue() << std::endl;
            infos.push_back(value->GetUnserializableValue());
        }
        if (value->HasDescription()) {
            std::cout << "desc: " << value->GetDescription() << std::endl;
            infos.push_back(value->GetDescription());
        }
    }

    bool CompareExpected(std::string recv, const std::string &expect)
    {
        expectDesIndex++;
        bool result = (recv == expect);
        if (!result) {
            std::cout << "recv: " << std::endl;
            std::cout << recv << std::endl;
            std::cout << "expect: " << std::endl;
            std::cout << expect << std::endl;
        }
        return result;
    }

    const std::map<std::string, std::vector<std::string>> moduleVariableMap_ = {
        { "string0", { "string", "helloworld", "helloworld" } },
        { "boolean0", { "object", "Object", "Boolean{[[PrimitiveValue]]: false}",
            "Boolean{[[PrimitiveValue]]: false}"} },
        { "number0", { "number", "1", "1" } },
        { "obj0", { "object", "Object", "Object", "Object"} },
        { "arraybuffer0", { "object", "arraybuffer", "Arraybuffer", "Arraybuffer(24)", "Arraybuffer(24)"} },
        { "function0", { "function", "Function", "function function0( { [js code] }",
            "function function0( { [js code] }" } },
        { "generator0", { "function", "Generator", "function* generator0( { [js code] }",
            "function* generator0( { [js code] }" } },
        { "map0", { "object", "map", "Map", "Map(0)", "Map(0)"} },
        { "set0", { "object", "set", "Set", "Set(0)", "Set(0)"} },
        { "undefined0", { "undefined" } },
        { "array0", { "object", "array", "Array", "Array(2)", "Array(2)"} },
        { "regexp0", { "object", "regexp", "RegExp", "/^\\d+\\.\\d+$/i", "/^\\d+\\.\\d+$/i"} },
        { "uint8array0", { "object", "Object", "Uint8Array(24)", "Uint8Array(24)"} },
        { "dataview0", { "object", "dataview", "Dataview", "DataView(24)", "DataView(24)"} },
        { "bigint0", { "bigint", "999n", "999n" } },
        { "typedarray0", { "object", "Object", "Uint8Array(0)", "Uint8Array(0)"} },
        { "sharedarraybuffer0", { "object", "Object", "SharedArrayBuffer(32)", "SharedArrayBuffer(32)"} },
        { "weakref0", { "object", "Object", "WeakRef {}", "WeakRef {}"} },
        { "iterator0", { "function", "Function", "function [Symbol.iterator]( { [native code] }",
            "function [Symbol.iterator]( { [native code] }" } },
        { "string1", { "string", "helloworld1", "helloworld1" } },
        { "boolean1", { "object", "Object", "Boolean{[[PrimitiveValue]]: false}",
            "Boolean{[[PrimitiveValue]]: false}"} },
        { "number1", { "number", "11", "11" } },
        { "obj1", { "object", "Object", "Object", "Object"} },
        { "arraybuffer1", { "object", "arraybuffer", "Arraybuffer", "Arraybuffer(24)", "Arraybuffer(24)"} },
        { "function1", { "function", "Function", "function function1( { [js code] }",
            "function function1( { [js code] }" } },
        { "generator1", { "function", "Generator", "function* generator1( { [js code] }",
            "function* generator1( { [js code] }" } },
        { "map1", { "object", "map", "Map", "Map(0)", "Map(0)"} },
        { "set1", { "object", "set", "Set", "Set(0)", "Set(0)"} },
        { "undefined1", { "undefined" } },
        { "regexp1", { "object", "regexp", "RegExp", "/^\\d+\\.\\d+$/i", "/^\\d+\\.\\d+$/i"} },
        { "uint8array1", { "object", "Object", "Uint8Array(24)", "Uint8Array(24)"} },
        { "dataview1", { "object", "dataview", "Dataview", "DataView(24)", "DataView(24)"} },
        { "bigint1", { "bigint", "9999n", "9999n" } },
        { "typedarray1", { "object", "Object", "Uint8Array(0)", "Uint8Array(0)"} },
        { "sharedarraybuffer1", { "object", "Object", "SharedArrayBuffer(32)", "SharedArrayBuffer(32)"} },
        { "weakref1", { "object", "Object", "WeakRef {}", "WeakRef {}"} },
        { "iterator1", { "function", "Function", "function [Symbol.iterator]( { [native code] }",
            "function [Symbol.iterator]( { [native code] }" } },
        { "string2", { "string", "helloworld11", "helloworld11" } },
        { "boolean2", { "object", "Object", "Boolean{[[PrimitiveValue]]: false}",
            "Boolean{[[PrimitiveValue]]: false}"} },
        { "number2", { "number", "111", "111" } },
        { "obj2", { "object", "Object", "Object", "Object"} },
        { "arraybuffer2", { "object", "arraybuffer", "Arraybuffer", "Arraybuffer(24)", "Arraybuffer(24)"} },
        { "function2", { "function", "Function", "function function2( { [js code] }",
            "function function2( { [js code] }" } },
        { "generator2", { "function", "Generator", "function* generator2( { [js code] }",
            "function* generator2( { [js code] }" } },
        { "map2", { "object", "map", "Map", "Map(0)", "Map(0)"} },
        { "set2", { "object", "set", "Set", "Set(0)", "Set(0)"} },
        { "undefined2", { "undefined" } },
        { "regexp2", { "object", "regexp", "RegExp", "/^\\d+\\.\\d+$/i", "/^\\d+\\.\\d+$/i"} },
        { "uint8array2", { "object", "Object", "Uint8Array(24)", "Uint8Array(24)"} },
        { "dataview2", { "object", "dataview", "Dataview", "DataView(24)", "DataView(24)"} },
        { "bigint2", { "bigint", "9999n", "9999n" } },
        { "typedarray2", { "object", "Object", "Uint8Array(0)", "Uint8Array(0)"} },
        { "sharedarraybuffer2", { "object", "Object", "SharedArrayBuffer(32)", "SharedArrayBuffer(32)"} },
        { "weakref2", { "object", "Object", "WeakRef {}", "WeakRef {}"} },
        { "iterator2", { "function", "Function", "function [Symbol.iterator]( { [native code] }",
            "function [Symbol.iterator]( { [native code] }" } },
        { "string4", { "string", "helloworld123", "helloworld123" } },
        { "boolean4", { "object", "Object", "Boolean{[[PrimitiveValue]]: true}", "Boolean{[[PrimitiveValue]]: true}"} },
        { "number4", { "number", "4", "4" } },
        { "foo", { "function", "Function", "function foo( { [js code] }", "function foo( { [js code] }"} },
        { "string6", { "string", "helloworld12345", "helloworld12345" } },
        { "boolean6", { "object", "Object", "Boolean{[[PrimitiveValue]]: true}", "Boolean{[[PrimitiveValue]]: true}"} },
        { "number6", { "number", "6", "6" } },
        { "obj6", { "object", "Object", "Object", "Object"} },
        { "arraybuffer6", { "object", "arraybuffer", "Arraybuffer", "Arraybuffer(24)", "Arraybuffer(24)"} },
        { "function6", { "function", "Function", "function function6( { [js code] }",
            "function function6( { [js code] }" } },
        { "generator6", { "function", "Generator", "function* generator6( { [js code] }",
            "function* generator6( { [js code] }" } },
        { "map6", { "object", "map", "Map", "Map(0)", "Map(0)"} },
        { "set6", { "object", "set", "Set", "Set(0)", "Set(0)"} },
        { "undefined6", { "undefined" } },
        { "array6", { "object", "array", "Array", "Array(2)", "Array(2)"} },
        { "regexp6", { "object", "regexp", "RegExp", "/^\\d+\\.\\d+$/i", "/^\\d+\\.\\d+$/i"} },
        { "uint8array6", { "object", "Object", "Uint8Array(24)", "Uint8Array(24)"} },
        { "dataview6", { "object", "dataview", "Dataview", "DataView(23)", "DataView(23)"} },
        { "bigint6", { "bigint", "99999n", "99999n" } },
        { "string7", { "string", "helloworld12345", "helloworld12345" } },
        { "boolean7", { "object", "Object", "Boolean{[[PrimitiveValue]]: true}", "Boolean{[[PrimitiveValue]]: true}"} },
        { "number7", { "number", "6", "6" } },
        { "obj7", { "object", "Object", "Object", "Object"} },
        { "arraybuffer7", { "object", "arraybuffer", "Arraybuffer", "Arraybuffer(24)", "Arraybuffer(24)"} },
        { "function7", { "function", "Function", "function function6( { [js code] }",
            "function function6( { [js code] }" } },
        { "generator7", { "function", "Generator", "function* generator6( { [js code] }",
            "function* generator6( { [js code] }" } },
        { "map7", { "object", "map", "Map", "Map(0)", "Map(0)"} },
        { "set7", { "object", "set", "Set", "Set(0)", "Set(0)"} },
        { "undefined7", { "undefined" } },
        { "array7", { "object", "array", "Array", "Array(2)", "Array(2)"} },
        { "regexp7", { "object", "regexp", "RegExp", "/^\\d+\\.\\d+$/i", "/^\\d+\\.\\d+$/i"} },
        { "uint8array7", { "object", "Object", "Uint8Array(24)", "Uint8Array(24)"} },
        { "dataview7", { "object", "dataview", "Dataview", "DataView(23)", "DataView(23)"} },
        { "bigint7", { "bigint", "99999n", "99999n" } },
        { "string9", { "string", "helloworld11", "helloworld11" } },
        { "boolean9", { "object", "Object", "Boolean{[[PrimitiveValue]]: false}",
            "Boolean{[[PrimitiveValue]]: false}"} },
        { "number9", { "number", "111", "111" } },
        { "obj9", { "object", "Object", "Object", "Object"} },
        { "arraybuffer9", { "object", "arraybuffer", "Arraybuffer", "Arraybuffer(24)", "Arraybuffer(24)"} },
        { "function9", { "function", "Function", "function function8( { [js code] }",
            "function function8( { [js code] }" } },
        { "generator9", { "function", "Generator", "function* generator8( { [js code] }",
            "function* generator8( { [js code] }" } },
        { "map9", { "object", "map", "Map", "Map(0)", "Map(0)"} },
        { "set9", { "object", "set", "Set", "Set(0)", "Set(0)"} },
        { "undefined9", { "undefined" } },
        { "array9", { "object", "array", "Array", "Array(2)", "Array(2)"} },
        { "regexp9", { "object", "regexp", "RegExp", "/^\\d+\\.\\d+$/i", "/^\\d+\\.\\d+$/i"} },
        { "uint8array9", { "object", "Object", "Uint8Array(24)", "Uint8Array(24)"} },
        { "dataview9", { "object", "dataview", "Dataview", "DataView(24)", "DataView(24)"} },
        { "bigint9", { "bigint", "9999n", "9999n" } },
        { "string2", { "string", "helloworld11", "helloworld11" } },
        { "boolean10", { "object", "Object", "Boolean{[[PrimitiveValue]]: false}",
            "Boolean{[[PrimitiveValue]]: false}"} },
        { "string10", { "string", "helloworld11", "helloworld11" } },
        { "number10", { "number", "111", "111" } },
        { "obj10", { "object", "Object", "Object", "Object"} },
        { "arraybuffer10", { "object", "arraybuffer", "Arraybuffer", "Arraybuffer(24)", "Arraybuffer(24)"} },
        { "function10", { "function", "Function", "function function8( { [js code] }",
            "function function8( { [js code] }" } },
        { "generator10", { "function", "Generator", "function* generator8( { [js code] }",
            "function* generator8( { [js code] }" } },
        { "map10", { "object", "map", "Map", "Map(0)", "Map(0)"} },
        { "set10", { "object", "set", "Set", "Set(0)", "Set(0)"} },
        { "undefined10", { "undefined" } },
        { "array10", { "object", "array", "Array", "Array(2)", "Array(2)"} },
        { "regexp10", { "object", "regexp", "RegExp", "/^\\d+\\.\\d+$/i", "/^\\d+\\.\\d+$/i"} },
        { "uint8array10", { "object", "Object", "Uint8Array(24)", "Uint8Array(24)"} },
        { "dataview10", { "object", "dataview", "Dataview", "DataView(24)", "DataView(24)"} },
        { "bigint10", { "bigint", "9999n", "9999n" } },
        { "transit1", { "number", "1", "1" } },
        { "transit2", { "number", "2", "2" } },
        { "transit4", { "number", "3", "3" } },
        { "transit5", { "number", "1", "1" } },
        { "transit6", { "number", "2", "2" } },
        { "transit7", { "number", "3", "3" } },
        { "multipleOut1", { "number", "1", "1" } },
        { "multipleOut2", { "number", "2", "2" } },
        { "multipleOut3", { "number", "3", "3" } },
        { "multipleOut4", { "number", "4", "4" } },
        { "multipleOut5", { "number", "5", "5" } },
        { "multipleOut6", { "number", "6", "6" } },
        { "multipleOut10", { "number", "10", "10" } },
        { "multipleOut11", { "number", "11", "11" } },
        { "multipleOut12", { "number", "12", "12" } },
        { "*default*", { "function", "Function", "function default( { [js code] }",
            "function default( { [js code] }" } },
        {"obj", { "object", "Object", "Object",  "Object"}}
    };

    const std::vector<std::string> innerDes = {
        "{\"id\":10,\"result\":{\"result\":[{\"name\":\"length\",\"value\":{\"type\":\"number\",\"unserializableValue\""
        ":\"0\",\"description\":\"0\"},\"writable\":false,\"configurable\":true,\"enumerable\":false,\"isOwn\":true},{"
        "\"name\":\"name\",\"value\":{\"type\":\"string\",\"unserializableValue\":\"default\",\"description\":"
        "\"default\"},\"writable\":false,\"configurable\":true,\"enumerable\":false,\"isOwn\":true},{\"name\":"
        "\"prototype\",\"value\":{\"type\":\"object\",\"className\":\"Object\",\"unserializableValue\":\"Object\","
        "\"description\":\"Object\",\"objectId\":\"100\"},\"writable\":true,\"configurable\":false,\"enumerable\":"
        "false,\"isOwn\":true}]}}",

        "{\"id\":11,\"result\":{\"result\":[{\"name\":\"0\",\"value\":{\"type\":\"string\",\"unserializableValue\":"
        "\"Apple\",\"description\":\"Apple\"},\"writable\":true,\"configurable\":true,\"enumerable\":true,\"isOwn\":"
        "true},{\"name\":\"1\",\"value\":{\"type\":\"string\",\"unserializableValue\":\"Banana\",\"description\":"
        "\"Banana\"},\"writable\":true,\"configurable\":true,\"enumerable\":true,\"isOwn\":true},{\"name\":\"length\","
        "\"value\":{\"type\":\"number\",\"unserializableValue\":\"2\",\"description\":\"2\"},\"writable\":true,"
        "\"configurable\":false,\"enumerable\":false,\"isOwn\":true}]}}",

        "{\"id\":12,\"result\":{\"result\":[{\"name\":\"[[Int8Array]]\",\"value\":{\"type\":\"object\",\"className\":"
        "\"Object\",\"unserializableValue\":\"Int8Array(24)\",\"description\":\"Int8Array(24)\",\"objectId\":\"101\","
        "\"arrayOrContainer\":\"Array\"},"
        "\"writable\":true,\"configurable\":true,\"enumerable\":false,\"isOwn\":true},{\"name\":\"[[Uint8Array]]\","
        "\"value\":{\"type\":\"object\",\"className\":\"Object\",\"unserializableValue\":\"Uint8Array(24)\","
        "\"description\":\"Uint8Array(24)\",\"objectId\":\"102\",\"arrayOrContainer\":\"Array\"},"
        "\"writable\":true,\"configurable\":true,"
        "\"enumerable\":false,\"isOwn\":true},{\"name\":\"[[Uint8ClampedArray]]\",\"value\":{\"type\":\"object\","
        "\"className\":\"Object\",\"unserializableValue\":"
        "\"Uint8ClampedArray(24)\",\"description\":\"Uint8ClampedArray(24)\",\"objectId\":\"103\","
        "\"arrayOrContainer\":\"Array\"},"
        "\"writable\":true,\"configurable\":true,\"enumerable\":false,\"isOwn\":true},{\"name\":\"[[Int16Array]]\","
        "\"value\":{\"type\":\"object\",\"className\":\"Object\",\"unserializableValue\":\"Int16Array(12)\","
        "\"description\":\"Int16Array(12)\",\"objectId\":\"104\","
        "\"arrayOrContainer\":\"Array\"},"
        "\"writable\":true,\"configurable\":true,"
        "\"enumerable\":false,\"isOwn\":true},{\"name\":\"[[Uint16Array]]\",\"value\":{\"type\":\"object\","
        "\"className\":\"Object\",\"unserializableValue\":\"Uint16Array(12)\",\"description\":\"Uint16Array(12)\","
        "\"objectId\":\"105\",\"arrayOrContainer\":\"Array\"},"
        "\"writable\":true,\"configurable\":true,\"enumerable\":false,\"isOwn\":true},{\"name\":"
        "\"[[Int32Array]]\",\"value\":{\"type\":\"object\",\"className\":\"Object\",\"unserializableValue\":"
        "\"Int32Array(6)\",\"description\":\"Int32Array(6)\",\"objectId\":\"106\",\"arrayOrContainer\":\"Array\"},"
        "\"writable\":true,\"configurable\":"
        "true,\"enumerable\":false,\"isOwn\":true},{\"name\":\"[[Uint32Array]]\",\"value\":{\"type\":\"object\","
        "\"className\":\"Object\",\"unserializableValue\":\"Uint32Array(6)\","
        "\"description\":\"Uint32Array(6)\",\"objectId\":\"107\",\"arrayOrContainer\":\"Array\"},"
        "\"writable\":true,\"configurable\":true,\"enumerable\":false,\"isOwn\":true},{\"name\":\"[[Float32Array]]\","
        "\"value\":{\"type\":\"object\",\"className\":\"Object\","
        "\"unserializableValue\":\"Float32Array(6)\",\"description\":"
        "\"Float32Array(6)\",\"objectId\":\"108\",\"arrayOrContainer\":\"Array\"},\"writable\":"
        "true,\"configurable\":true,\"enumerable\":false,\"isOwn\":true},"
        "{\"name\":\"[[Float64Array]]\",\"value\":{\"type\":\"object\",\"className\":\"Object\","
        "\"unserializableValue\":\"Float64Array(3)\",\"description\":"
        "\"Float64Array(3)\",\"objectId\":\"109\",\"arrayOrContainer\":\"Array\"},\"writable\":true,"
        "\"configurable\":true,\"enumerable\":false,\"isOwn\":true},{\"name\":\"[[BigInt64Array]]\",\"value\":"
        "{\"type\":\"object\",\"className\":\"Object\",\"unserializableValue\":"
        "\"BigInt64Array(3)\",\"description\":\"BigInt64Array(3)\","
        "\"objectId\":\"110\",\"arrayOrContainer\":\"Array\"},"
        "\"writable\":true,\"configurable\":true,\"enumerable\":false,\"isOwn\":true},{\"name\":"
        "\"[[BigUint64Array]]\",\"value\":{\"type\":\"object\",\"className\":\"Object\",\"unserializableValue\":"
        "\"BigUint64Array(3)\",\"description\":\"BigUint64Array(3)\","
        "\"objectId\":\"111\",\"arrayOrContainer\":\"Array\"},\"writable\":true,\"configurable\":true,"
        "\"enumerable\":false,\"isOwn\":true}]}}",

        "{\"id\":13,\"result\":{\"result\":[{\"name\":\"[[PrimitiveValue]]\",\"value\":{\"type\":\"boolean\","
        "\"unserializableValue\":\"false\",\"description\":\"false\",\"objectId\":\"112\"},\"writable\":false,"
        "\"configurable\":false,\"enumerable\":false,\"isOwn\":false}]}}",

        "{\"id\":14,\"result\":{\"result\":[{\"name\":\"buffer\",\"value\":{\"type\":\"object\",\"subtype\":"
        "\"arraybuffer\",\"className\":\"Arraybuffer\",\"unserializableValue\":\"Arraybuffer(24)\",\"description\":"
        "\"Arraybuffer(24)\",\"objectId\":\"113\"},\"writable\":false,\"configurable\":false,\"enumerable\":false,"
        "\"isOwn\":false},{\"name\":\"byteLength\",\"value\":{\"type\":\"number\",\"unserializableValue\":\"24\","
        "\"description\":\"24\",\"objectId\":\"114\"},\"writable\":false,\"configurable\":false,\"enumerable\":false,"
        "\"isOwn\":false},{\"name\":\"byteOffset\",\"value\":{\"type\":\"number\",\"unserializableValue\":\"0\","
        "\"description\":\"0\",\"objectId\":\"115\"},\"writable\":false,\"configurable\":false,\"enumerable\":false,"
        "\"isOwn\":false}]}}",

        "{\"id\":15,\"result\":{\"result\":[{\"name\":\"length\",\"value\":{\"type\":\"number\","
        "\"unserializableValue\":\"0\",\"description\":\"0\"},\"writable\":false,\"configurable\":true,\"enumerable\":"
        "false,\"isOwn\":true},{\"name\":\"name\",\"value\":{\"type\":\"string\",\"unserializableValue\":\"foo\","
        "\"description\":\"foo\"},\"writable\":false,\"configurable\":true,\"enumerable\":false,\"isOwn\":true},"
        "{\"name\":\"prototype\",\"value\":{\"type\":\"object\",\"className\":\"Object\",\"unserializableValue\":"
        "\"Object\",\"description\":\"Object\",\"objectId\":\"116\"},\"writable\":true,\"configurable\":false,"
        "\"enumerable\":false,\"isOwn\":true}]}}",

        "{\"id\":16,\"result\":{\"result\":[{\"name\":\"length\",\"value\":{\"type\":\"number\","
        "\"unserializableValue\":\"0\",\"description\":\"0\"},\"writable\":false,\"configurable\":true,\"enumerable\":"
        "false,\"isOwn\":true},{\"name\":\"name\",\"value\":{\"type\":\"string\",\"unserializableValue\":\"function0\","
        "\"description\":\"function0\"},\"writable\":false,\"configurable\":true,\"enumerable\":false,\"isOwn\":true},"
        "{\"name\":\"prototype\",\"value\":{\"type\":\"object\",\"className\":\"Object\",\"unserializableValue\":"
        "\"Object\",\"description\":\"Object\",\"objectId\":\"117\"},\"writable\":true,\"configurable\":false,"
        "\"enumerable\":false,\"isOwn\":true}]}}",

        "{\"id\":17,\"result\":{\"result\":[{\"name\":\"[[IsGenerator]]\",\"value\":{\"type\":\"boolean\","
        "\"unserializableValue\":\"true\",\"description\":\"true\",\"objectId\":\"118\"},\"writable\":false,"
        "\"configurable\":false,\"enumerable\":false,\"isOwn\":false},{\"name\":\"length\",\"value\":{\"type\":"
        "\"number\",\"unserializableValue\":\"0\",\"description\":\"0\"},\"writable\":false,\"configurable\":true,"
        "\"enumerable\":false,\"isOwn\":true},{\"name\":\"name\",\"value\":{\"type\":\"string\","
        "\"unserializableValue\":\"generator0\",\"description\":\"generator0\"},\"writable\":false,\"configurable\":"
        "true,\"enumerable\":false,\"isOwn\":true},{\"name\":\"prototype\",\"value\":{\"type\":\"object\","
        "\"className\":\"Object\",\"unserializableValue\":\"Object\",\"description\":\"Object\",\"objectId\":\"119\"},"
        "\"writable\":true,\"configurable\":false,\"enumerable\":false,\"isOwn\":true}]}}",

        "{\"id\":18,\"result\":{\"result\":[{\"name\":\"length\",\"value\":{\"type\":\"number\","
        "\"unserializableValue\":\"0\",\"description\":\"0\"},\"writable\":false,\"configurable\":true,\"enumerable\":"
        "false,\"isOwn\":true},{\"name\":\"name\",\"value\":{\"type\":\"string\",\"unserializableValue\":"
        "\"[Symbol.iterator]\",\"description\":\"[Symbol.iterator]\"},\"writable\":false,\"configurable\":true,"
        "\"enumerable\":false,\"isOwn\":true}]}}",

        "{\"id\":19,\"result\":{\"result\":[{\"name\":\"size\",\"value\":{\"type\":\"number\",\"unserializableValue\":"
        "\"0\",\"description\":\"0\",\"objectId\":\"120\"},\"writable\":false,\"configurable\":false,\"enumerable\":"
        "false,\"isOwn\":false},{\"name\":\"[[Entries]]\",\"value\":{\"type\":\"object\",\"subtype\":\"array\","
        "\"className\":\"Array\",\"unserializableValue\":\"Array(0)\",\"description\":\"Array(0)\",\"objectId\":"
        "\"121\",\"arrayOrContainer\":\"Array\"},"
        "\"writable\":false,\"configurable\":false,\"enumerable\":false,\"isOwn\":false}]}}",

        "{\"id\":20,\"result\":{\"result\":[{\"name\":\"key0\",\"value\":{\"type\":\"string\",\"unserializableValue\":"
        "\"value0\",\"description\":\"value0\"},\"writable\":true,\"configurable\":true,\"enumerable\":true,\"isOwn\":"
        "true},{\"name\":\"key1\",\"value\":{\"type\":\"number\",\"unserializableValue\":\"100\",\"description\":"
        "\"100\"},\"writable\":true,\"configurable\":true,\"enumerable\":true,\"isOwn\":true}]}}",

        "{\"id\":21,\"result\":{\"result\":[{\"name\":\"global\",\"value\":{\"type\":\"boolean\","
        "\"unserializableValue\":\"false\",\"description\":\"false\",\"objectId\":\"122\"},\"writable\":false,"
        "\"configurable\":false,\"enumerable\":false,\"isOwn\":false},{\"name\":\"ignoreCase\",\"value\":{\"type\":"
        "\"boolean\",\"unserializableValue\":\"true\",\"description\":\"true\",\"objectId\":\"123\"},\"writable\":"
        "false,\"configurable\":false,\"enumerable\":false,\"isOwn\":false},{\"name\":\"multiline\",\"value\":"
        "{\"type\":\"boolean\",\"unserializableValue\":\"false\",\"description\":\"false\",\"objectId\":\"124\"},"
        "\"writable\":false,\"configurable\":false,\"enumerable\":false,\"isOwn\":false},{\"name\":\"dotAll\","
        "\"value\":{\"type\":\"boolean\",\"unserializableValue\":\"false\",\"description\":\"false\",\"objectId\":"
        "\"125\"},\"writable\":false,\"configurable\":false,\"enumerable\":false,\"isOwn\":false},{\"name\":"
        "\"hasIndices\",\"value\":{\"type\":\"boolean\",\"unserializableValue\":\"false\",\"description\":\"false\","
        "\"objectId\":\"126\"},\"writable\":false,\"configurable\":false,\"enumerable\":false,\"isOwn\":false},"
        "{\"name\":\"unicode\",\"value\":{\"type\":\"boolean\",\"unserializableValue\":\"false\",\"description\":"
        "\"false\",\"objectId\":\"127\"},\"writable\":false,\"configurable\":false,\"enumerable\":false,\"isOwn\":"
        "false},{\"name\":\"sticky\",\"value\":{\"type\":\"boolean\",\"unserializableValue\":\"false\",\"description\":"
        "\"false\",\"objectId\":\"128\"},\"writable\":false,\"configurable\":false,\"enumerable\":false,\"isOwn\":"
        "false},{\"name\":\"flags\",\"value\":{\"type\":\"string\",\"unserializableValue\":\"i\",\"description\":"
        "\"i\",\"objectId\":\"129\"},\"writable\":false,\"configurable\":false,\"enumerable\":false,\"isOwn\":false},"
        "{\"name\":\"source\",\"value\":{\"type\":\"string\",\"unserializableValue\":\"^\\\\d+\\\\.\\\\d+$\","
        "\"description\":\"^\\\\d+\\\\.\\\\d+$\",\"objectId\":\"130\"},\"writable\":false,\"configurable\":false,"
        "\"enumerable\":false,\"isOwn\":false},{\"name\":\"lastIndex\",\"value\":{\"type\":\"number\","
        "\"unserializableValue\":\"0\",\"description\":\"0\"},\"writable\":true,\"configurable\":false,\"enumerable\":"
        "false,\"isOwn\":true}]}}",

        "{\"id\":22,\"result\":{\"result\":[{\"name\":\"size\",\"value\":{\"type\":\"number\",\"unserializableValue\":"
        "\"0\",\"description\":\"0\",\"objectId\":\"131\"},\"writable\":false,\"configurable\":false,\"enumerable\":"
        "false,\"isOwn\":false},{\"name\":\"[[Entries]]\",\"value\":{\"type\":\"object\",\"subtype\":\"array\","
        "\"className\":\"Array\",\"unserializableValue\":\"Array(0)\",\"description\":\"Array(0)\",\"objectId\":"
        "\"132\",\"arrayOrContainer\":\"Array\"},"
        "\"writable\":false,\"configurable\":false,\"enumerable\":false,\"isOwn\":false}]}}",

        "{\"id\":23,\"result\":{\"result\":[{\"name\":\"[[Int8Array]]\",\"value\":{\"type\":\"object\",\"className\":"
        "\"Object\",\"unserializableValue\":\"Int8Array(32)\",\"description\":\"Int8Array(32)\",\"objectId\":\"133\","
        "\"arrayOrContainer\":\"Array\"},"
        "\"writable\":true,\"configurable\":true,\"enumerable\":false,\"isOwn\":true},{\"name\":\"[[Uint8Array]]\","
        "\"value\":{\"type\":\"object\",\"className\":\"Object\",\"unserializableValue\":\"Uint8Array(32)\","
        "\"description\":\"Uint8Array(32)\",\"objectId\":\"134\",\"arrayOrContainer\":\"Array\"},"
        "\"writable\":true,\"configurable\":true,"
        "\"enumerable\":false,\"isOwn\":true},{\"name\":\"[[Int16Array]]\",\"value\":{\"type\":\"object\","
        "\"className\":\"Object\",\"unserializableValue\":\"Int16Array(16)\",\"description\":\"Int16Array(16)\","
        "\"objectId\":\"135\",\"arrayOrContainer\":\"Array\"},"
        "\"writable\":true,\"configurable\":true,\"enumerable\":false,\"isOwn\":true},{\"name\":"
        "\"[[Int32Array]]\",\"value\":{\"type\":\"object\",\"className\":\"Object\",\"unserializableValue\":"
        "\"Int32Array(8)\",\"description\":\"Int32Array(8)\",\"objectId\":\"136\",\"arrayOrContainer\":\"Array\"},"
        "\"writable\":true,\"configurable\":"
        "true,\"enumerable\":false,\"isOwn\":true},{\"name\":\"[[ArrayBufferByteLength]]\",\"value\":{\"type\":"
        "\"number\",\"unserializableValue\":\"32\",\"description\":\"32\",\"objectId\":\"137\"},\"writable\":false,"
        "\"configurable\":false,\"enumerable\":false,\"isOwn\":false},{\"name\":\"byteLength\",\"value\":{\"type\":"
        "\"number\",\"unserializableValue\":\"32\",\"description\":\"32\",\"objectId\":\"138\"},\"writable\":false,"
        "\"configurable\":false,\"enumerable\":false,\"isOwn\":false}]}}",

        "{\"id\":24,\"result\":{\"result\":[{\"name\":\"0\",\"value\":{\"type\":\"number\",\"unserializableValue\":"
        "\"0\",\"description\":\"0\"},\"writable\":true,\"configurable\":true,\"enumerable\":true,\"isOwn\":true},"
        "{\"name\":\"1\",\"value\":{\"type\":\"number\",\"unserializableValue\":\"0\",\"description\":\"0\"},"
        "\"writable\":true,\"configurable\":true,\"enumerable\":true,\"isOwn\":true},{\"name\":\"2\",\"value\":"
        "{\"type\":\"number\",\"unserializableValue\":\"0\",\"description\":\"0\"},\"writable\":true,\"configurable\":"
        "true,\"enumerable\":true,\"isOwn\":true},{\"name\":\"3\",\"value\":{\"type\":\"number\","
        "\"unserializableValue\":\"0\",\"description\":\"0\"},\"writable\":true,\"configurable\":true,\"enumerable\":"
        "true,\"isOwn\":true},{\"name\":\"4\",\"value\":{\"type\":\"number\",\"unserializableValue\":\"0\","
        "\"description\":\"0\"},\"writable\":true,\"configurable\":true,\"enumerable\":true,\"isOwn\":true},{\"name\":"
        "\"5\",\"value\":{\"type\":\"number\",\"unserializableValue\":\"0\",\"description\":\"0\"},\"writable\":true,"
        "\"configurable\":true,\"enumerable\":true,\"isOwn\":true},{\"name\":\"6\",\"value\":{\"type\":\"number\","
        "\"unserializableValue\":\"0\",\"description\":\"0\"},\"writable\":true,\"configurable\":true,\"enumerable\":"
        "true,\"isOwn\":true},{\"name\":\"7\",\"value\":{\"type\":\"number\",\"unserializableValue\":\"0\","
        "\"description\":\"0\"},\"writable\":true,\"configurable\":true,\"enumerable\":true,\"isOwn\":true},{\"name\":"
        "\"8\",\"value\":{\"type\":\"number\",\"unserializableValue\":\"0\",\"description\":\"0\"},\"writable\":true,"
        "\"configurable\":true,\"enumerable\":true,\"isOwn\":true},{\"name\":\"9\",\"value\":{\"type\":\"number\","
        "\"unserializableValue\":\"0\",\"description\":\"0\"},\"writable\":true,\"configurable\":true,\"enumerable\":"
        "true,\"isOwn\":true},{\"name\":\"10\",\"value\":{\"type\":\"number\",\"unserializableValue\":\"0\","
        "\"description\":\"0\"},\"writable\":true,\"configurable\":true,\"enumerable\":true,\"isOwn\":true},{\"name\":"
        "\"11\",\"value\":{\"type\":\"number\",\"unserializableValue\":\"0\",\"description\":\"0\"},\"writable\":true,"
        "\"configurable\":true,\"enumerable\":true,\"isOwn\":true},{\"name\":\"12\",\"value\":{\"type\":\"number\","
        "\"unserializableValue\":\"0\",\"description\":\"0\"},\"writable\":true,\"configurable\":true,\"enumerable\":"
        "true,\"isOwn\":true},{\"name\":\"13\",\"value\":{\"type\":\"number\",\"unserializableValue\":\"0\","
        "\"description\":\"0\"},\"writable\":true,\"configurable\":true,\"enumerable\":true,\"isOwn\":true},{\"name\":"
        "\"14\",\"value\":{\"type\":\"number\",\"unserializableValue\":\"0\",\"description\":\"0\"},\"writable\":true,"
        "\"configurable\":true,\"enumerable\":true,\"isOwn\":true},{\"name\":\"15\",\"value\":{\"type\":\"number\","
        "\"unserializableValue\":\"0\",\"description\":\"0\"},\"writable\":true,\"configurable\":true,\"enumerable\":"
        "true,\"isOwn\":true},{\"name\":\"16\",\"value\":{\"type\":\"number\",\"unserializableValue\":\"0\","
        "\"description\":\"0\"},\"writable\":true,\"configurable\":true,\"enumerable\":true,\"isOwn\":true},{\"name\":"
        "\"17\",\"value\":{\"type\":\"number\",\"unserializableValue\":\"0\",\"description\":\"0\"},\"writable\":true,"
        "\"configurable\":true,\"enumerable\":true,\"isOwn\":true},{\"name\":\"18\",\"value\":{\"type\":\"number\","
        "\"unserializableValue\":\"0\",\"description\":\"0\"},\"writable\":true,\"configurable\":true,\"enumerable\":"
        "true,\"isOwn\":true},{\"name\":\"19\",\"value\":{\"type\":\"number\",\"unserializableValue\":\"0\","
        "\"description\":\"0\"},\"writable\":true,\"configurable\":true,\"enumerable\":true,\"isOwn\":true},{\"name\":"
        "\"20\",\"value\":{\"type\":\"number\",\"unserializableValue\":\"0\",\"description\":\"0\"},\"writable\":true,"
        "\"configurable\":true,\"enumerable\":true,\"isOwn\":true},{\"name\":\"21\",\"value\":{\"type\":\"number\","
        "\"unserializableValue\":\"0\",\"description\":\"0\"},\"writable\":true,\"configurable\":true,\"enumerable\":"
        "true,\"isOwn\":true},{\"name\":\"22\",\"value\":{\"type\":\"number\",\"unserializableValue\":\"0\","
        "\"description\":\"0\"},\"writable\":true,\"configurable\":true,\"enumerable\":true,\"isOwn\":true},{\"name\":"
        "\"23\",\"value\":{\"type\":\"number\",\"unserializableValue\":\"0\",\"description\":\"0\"},\"writable\":true,"
        "\"configurable\":true,\"enumerable\":true,\"isOwn\":true}]}}",

        "{\"id\":25,\"result\":{\"result\":[{\"name\":\"[[PrimitiveValue]]\",\"value\":{\"type\":\"boolean\","
        "\"unserializableValue\":\"true\",\"description\":\"true\",\"objectId\":\"139\"},\"writable\":false,"
        "\"configurable\":false,\"enumerable\":false,\"isOwn\":false}]}}",

        "{\"id\":26,\"result\":{\"result\":[{\"name\":\"0\",\"value\":{\"type\":\"number\",\"unserializableValue\":"
        "\"0\",\"description\":\"0\"},\"writable\":true,\"configurable\":true,\"enumerable\":true,\"isOwn\":true},"
        "{\"name\":\"1\",\"value\":{\"type\":\"number\",\"unserializableValue\":\"0\",\"description\":\"0\"},"
        "\"writable\":true,\"configurable\":true,\"enumerable\":true,\"isOwn\":true},{\"name\":\"2\",\"value\":"
        "{\"type\":\"number\",\"unserializableValue\":\"0\",\"description\":\"0\"},\"writable\":true,\"configurable\":"
        "true,\"enumerable\":true,\"isOwn\":true},{\"name\":\"3\",\"value\":{\"type\":\"number\","
        "\"unserializableValue\":\"0\",\"description\":\"0\"},\"writable\":true,\"configurable\":true,\"enumerable\":"
        "true,\"isOwn\":true},{\"name\":\"4\",\"value\":{\"type\":\"number\",\"unserializableValue\":\"0\","
        "\"description\":\"0\"},\"writable\":true,\"configurable\":true,\"enumerable\":true,\"isOwn\":true},{\"name\":"
        "\"5\",\"value\":{\"type\":\"number\",\"unserializableValue\":\"0\",\"description\":\"0\"},\"writable\":true,"
        "\"configurable\":true,\"enumerable\":true,\"isOwn\":true},{\"name\":\"6\",\"value\":{\"type\":\"number\","
        "\"unserializableValue\":\"0\",\"description\":\"0\"},\"writable\":true,\"configurable\":true,\"enumerable\":"
        "true,\"isOwn\":true},{\"name\":\"7\",\"value\":{\"type\":\"number\",\"unserializableValue\":\"0\","
        "\"description\":\"0\"},\"writable\":true,\"configurable\":true,\"enumerable\":true,\"isOwn\":true},{\"name\":"
        "\"8\",\"value\":{\"type\":\"number\",\"unserializableValue\":\"0\",\"description\":\"0\"},\"writable\":true,"
        "\"configurable\":true,\"enumerable\":true,\"isOwn\":true},{\"name\":\"9\",\"value\":{\"type\":\"number\","
        "\"unserializableValue\":\"0\",\"description\":\"0\"},\"writable\":true,\"configurable\":true,\"enumerable\":"
        "true,\"isOwn\":true},{\"name\":\"10\",\"value\":{\"type\":\"number\",\"unserializableValue\":\"0\","
        "\"description\":\"0\"},\"writable\":true,\"configurable\":true,\"enumerable\":true,\"isOwn\":true},{\"name\":"
        "\"11\",\"value\":{\"type\":\"number\",\"unserializableValue\":\"0\",\"description\":\"0\"},\"writable\":true,"
        "\"configurable\":true,\"enumerable\":true,\"isOwn\":true},{\"name\":\"12\",\"value\":{\"type\":\"number\","
        "\"unserializableValue\":\"0\",\"description\":\"0\"},\"writable\":true,\"configurable\":true,\"enumerable\":"
        "true,\"isOwn\":true},{\"name\":\"13\",\"value\":{\"type\":\"number\",\"unserializableValue\":\"0\","
        "\"description\":\"0\"},\"writable\":true,\"configurable\":true,\"enumerable\":true,\"isOwn\":true},"
        "{\"name\":\"14\",\"value\":{\"type\":\"number\",\"unserializableValue\":\"0\",\"description\":\"0\"},"
        "\"writable\":true,\"configurable\":true,\"enumerable\":true,\"isOwn\":true},{\"name\":\"15\",\"value\":"
        "{\"type\":\"number\",\"unserializableValue\":\"0\",\"description\":\"0\"},\"writable\":true,\"configurable\":"
        "true,\"enumerable\":true,\"isOwn\":true},{\"name\":\"16\",\"value\":{\"type\":\"number\","
        "\"unserializableValue\":\"0\",\"description\":\"0\"},\"writable\":true,\"configurable\":true,\"enumerable\":"
        "true,\"isOwn\":true},{\"name\":\"17\",\"value\":{\"type\":\"number\",\"unserializableValue\":\"0\","
        "\"description\":\"0\"},\"writable\":true,\"configurable\":true,\"enumerable\":true,\"isOwn\":true},{\"name\":"
        "\"18\",\"value\":{\"type\":\"number\",\"unserializableValue\":\"0\",\"description\":\"0\"},\"writable\":true,"
        "\"configurable\":true,\"enumerable\":true,\"isOwn\":true},{\"name\":\"19\",\"value\":{\"type\":\"number\","
        "\"unserializableValue\":\"0\",\"description\":\"0\"},\"writable\":true,\"configurable\":true,\"enumerable\":"
        "true,\"isOwn\":true},{\"name\":\"20\",\"value\":{\"type\":\"number\",\"unserializableValue\":\"0\","
        "\"description\":\"0\"},\"writable\":true,\"configurable\":true,\"enumerable\":true,\"isOwn\":true},{\"name\":"
        "\"21\",\"value\":{\"type\":\"number\",\"unserializableValue\":\"0\",\"description\":\"0\"},\"writable\":true,"
        "\"configurable\":true,\"enumerable\":true,\"isOwn\":true},{\"name\":\"22\",\"value\":{\"type\":\"number\","
        "\"unserializableValue\":\"0\",\"description\":\"0\"},\"writable\":true,\"configurable\":true,\"enumerable\":"
        "true,\"isOwn\":true},{\"name\":\"23\",\"value\":{\"type\":\"number\",\"unserializableValue\":\"0\","
        "\"description\":\"0\"},\"writable\":true,\"configurable\":true,\"enumerable\":true,\"isOwn\":true}]}}"
    };
};

std::unique_ptr<TestActions> GetJsModuleVariableTest()
{
    return std::make_unique<JsModuleVariableTest>();
}
}  // namespace panda::ecmascript::tooling::test

#endif  // ECMASCRIPT_TOOLING_TEST_TESTCASES_JS_MODULE_VARIABLE_TEST_H
