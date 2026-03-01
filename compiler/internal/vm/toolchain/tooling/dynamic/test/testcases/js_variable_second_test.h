/**
 * Copyright (c) 2022-2026 Huawei Device Co., Ltd.
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

#ifndef ECMASCRIPT_TOOLING_TEST_UTILS_TESTCASES_JS_VARIABLE_SECOND_TEST_H
#define ECMASCRIPT_TOOLING_TEST_UTILS_TESTCASES_JS_VARIABLE_SECOND_TEST_H

#include "test/utils/test_util.h"

namespace panda::ecmascript::tooling::test {
class JsVariableSecondTest : public TestEvents {
public:
    JsVariableSecondTest()
    {
        breakpoint = [this](const JSPtLocation &location) {
            ASSERT_TRUE(location.GetMethodId().IsValid());
            ASSERT_LOCATION_EQ(location, location_);
            ++breakpointCounter_;
            debugger_->NotifyPaused(location, PauseReason::INSTRUMENTATION);
            TestUtil::SuspendUntilContinue(DebugEvent::BREAKPOINT, location);
            return true;
        };

        loadModule = [this](std::string_view moduleName) {
            std::string panfaFile = DEBUGGER_ABC_DIR "variable_second.abc";
            std::string sourceFile = DEBUGGER_JS_DIR "variable_second.js";
            static_cast<JsVariableSecondTestChannel *>(channel_)->Initial(vm_, runtime_);
            runtime_->Enable();
            // 166: breakpointer line
            int32_t line = 166;
            location_ = TestUtil::GetLocation(sourceFile.c_str(), line, 0, panfaFile.c_str());
            ASSERT_TRUE(location_.GetMethodId().IsValid());
            TestUtil::SuspendUntilContinue(DebugEvent::LOAD_MODULE);
            ASSERT_EQ(moduleName, panfaFile);
            ASSERT_TRUE(debugger_->NotifyScriptParsed(0, panfaFile));
            auto condFuncRef = FunctionRef::Undefined(vm_);
            auto ret = debugInterface_->SetBreakpoint(location_, condFuncRef);
            ASSERT_TRUE(ret);
            return true;
        };

        scenario = [this]() {
            TestUtil::WaitForLoadModule();
            TestUtil::Continue();
            TestUtil::WaitForBreakpoint(location_);
            TestUtil::Continue();
            auto ret = debugInterface_->RemoveBreakpoint(location_);
            ASSERT_TRUE(ret);
            ASSERT_EXITED();
            return true;
        };

        vmDeath = [this]() {
            ASSERT_EQ(breakpointCounter_, 1U);  // 1: break point counter
            return true;
        };

        channel_ = new JsVariableSecondTestChannel();
    }

    std::pair<std::string, std::string> GetEntryPoint() override
    {
        std::string panfaFile = DEBUGGER_ABC_DIR "variable_second.abc";
        return {panfaFile, entryPoint_};
    }
    ~JsVariableSecondTest()
    {
        delete channel_;
        channel_ = nullptr;
    }

private:
    class JsVariableSecondTestChannel : public TestChannel {
    public:
        JsVariableSecondTestChannel() = default;
        ~JsVariableSecondTestChannel() = default;
        void Initial(const EcmaVM *vm, RuntimeImpl *runtime)
        {
            vm_ = vm;
            runtime_ = runtime;
        }

        void SendNotification(const PtBaseEvents &events) override
        {
            const static std::vector<std::function<bool(const PtBaseEvents &events)>> eventList = {
                [](const PtBaseEvents &events) -> bool {
                    std::string sourceFile = DEBUGGER_JS_DIR "variable_second.js";
                    auto parsed = static_cast<const ScriptParsed *>(&events);
                    std::string str = parsed->ToJson()->Stringify();
                    std::cout << "JsVariableSecondTestChannel: SendNotification 0:\n" << str << std::endl;

                    ASSERT_EQ(parsed->GetName(), "Debugger.scriptParsed");
                    ASSERT_EQ(parsed->GetUrl(), sourceFile);
                    return true;
                },
                [this](const PtBaseEvents &events) -> bool {
                    auto paused = static_cast<const Paused *>(&events);
                    std::string str = paused->ToJson()->Stringify();
                    std::cout << "JsVariableSecondTestChannel: SendNotification 1:\n" << str << std::endl;

                    ASSERT_EQ(paused->GetName(), "Debugger.paused");
                    auto frame = paused->GetCallFrames()->at(0).get();
                    ASSERT_EQ(frame->GetFunctionName(), "foo");
                    auto scopes = frame->GetScopeChain();
                    ASSERT_EQ(scopes->size(), 2U);  // 2: contain local and global
                    for (uint32_t i = 0; i < scopes->size(); i++) {
                        auto scope = scopes->at(i).get();
                        if (scope->GetType() != Scope::Type::Local()) {
                            continue;
                        }
                        auto localId = scope->GetObject()->GetObjectId();
                        GetPropertiesParams params;
                        params.SetObjectId(localId).SetOwnProperties(true);
                        std::vector<std::unique_ptr<PropertyDescriptor>> outPropertyDesc;
                        runtime_->GetProperties(params, &outPropertyDesc, {}, {}, {});
                        for (const auto &property : outPropertyDesc) {
                            std::cout << "=====================================" << std::endl;
                            std::cout << property->GetName() << std::endl;
                            auto value = property->GetValue();
                            std::vector<std::string> infos;
                            PushValueInfo(value, infos);
                            if (value->GetType() == RemoteObject::TypeName::Object) {
                                std::vector<std::unique_ptr<PropertyDescriptor>> outPropertyDescInner;
                                ASSERT_TRUE(value->HasObjectId());
                                params.SetObjectId(value->GetObjectId()).SetOwnProperties(true);
                                runtime_->GetProperties(params, &outPropertyDescInner, {}, {}, {});
                                if (outPropertyDescInner.size() == 0) {
                                    infos.push_back("none");
                                }
                                for (const auto &propertyInner : outPropertyDescInner) {
                                    std::cout << "###########################################" << std::endl;
                                    std::cout << propertyInner->GetName() << std::endl;
                                    infos.push_back(propertyInner->GetName());
                                    auto innerValue = propertyInner->GetValue();
                                    PushValueInfo(innerValue, infos);
                                }
                            }
                            ASSERT_EQ(infos.size(), variableMap_.at(property->GetName()).size());
                            for (uint32_t j = 0; j < infos.size(); j++) {
                                ASSERT_EQ(infos[j], variableMap_.at(property->GetName())[j]);
                            }
                        }
                    }
                    return true;
                }
            };

            ASSERT_TRUE(eventList[index_](events));
            index_++;
        }

    private:
        NO_COPY_SEMANTIC(JsVariableSecondTestChannel);
        NO_MOVE_SEMANTIC(JsVariableSecondTestChannel);

        void PushValueInfo(RemoteObject *value, std::vector<std::string> &infos)
        {
            std::cout << "type: " << value->GetType() << std::endl;
            infos.push_back(value->GetType());
            if (value->HasObjectId()) {
                std::cout << "id: " << value->GetObjectId() << std::endl;
            }
            if (value->HasSubType()) {
                std::cout << "sub type: " << value->GetSubType() << std::endl;
                infos.push_back(value->GetSubType());
            }
            if (value->HasClassName()) {
                std::cout << "class name: " << value->GetClassName() << std::endl;
                infos.push_back(value->GetClassName());
            }
            if (value->HasDescription()) {
                std::cout << "desc: " << value->GetDescription() << std::endl;
                infos.push_back(value->GetDescription());
            }
            if (value->HasValue()) {
                std::cout << "type: " <<
                    value->GetValue()->Typeof(vm_)->ToString(vm_) << std::endl;
                std::cout << "tostring: " <<
                    value->GetValue()->ToString(vm_)->ToString(vm_) << std::endl;
                infos.push_back(value->GetValue()->ToString(vm_)->ToString(vm_));
            }
        }

        /*
        * Expect map type: map<name, value list>
        * value list (optional):
        *    type
        *    subType
        *    className
        *    description
        *    value tostring
        *
        * if is object value, will push back key and value.
        *
        * for example:
        * var abc = 1
        *     { "abc", { "number", "1", "1" } }
        * var obj = { "key": "2" }
        *     { "obj0", { "object", "Object", "Object", "[object Object]", "key", "string", "2", "2" } }
        */
        const std::map<std::string, std::vector<std::string>> variableMap_ = {
            { "nop", { "undefined" } },
            { "foo", { "function", "Function", "function foo( { [js code] }",
                       "Cannot get source code of function"} },
            { "string0", { "string", "helloworld", "helloworld" } },
            { "boolean0", { "object", "Object", "Boolean{[[PrimitiveValue]]: false}", "false", "[[PrimitiveValue]]",
                            "boolean", "false", "false" } },
            { "number0", { "number", "1", "1" } },
            { "obj0", { "object", "Object", "Object", "[object Object]",
                        "key0", "string", "value0", "value0",
                        "key1", "number", "100", "100" } },
            { "arraybuffer0", { "object", "arraybuffer", "Arraybuffer", "Arraybuffer(24)", "[object ArrayBuffer]",
                                "[[Int8Array]]", "object", "Object", "Int8Array(24)",
                                "0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0", "[[Uint8Array]]", "object",
                                "Object", "Uint8Array(24)", "0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0",
                                "[[Uint8ClampedArray]]", "object", "Object", "Uint8ClampedArray",
                                "0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0", "[[Int16Array]]", "object",
                                "Object", "Int16Array(12)", "0,0,0,0,0,0,0,0,0,0,0,0", "[[Uint16Array]]", "object",
                                "Object", "Uint16Array", "0,0,0,0,0,0,0,0,0,0,0,0",
                                "[[Int32Array]]", "object", "Object",
                                "Int32Array(6)", "0,0,0,0,0,0", "[[Uint32Array]]", "object", "Object", "Uint32Array",
                                "0,0,0,0,0,0", "[[Float32Array]]", "object", "Object", "Float32Array", "0,0,0,0,0,0",
                                "[[Float64Array]]", "object", "Object", "Float64Array", "0,0,0", "[[BigInt64Array]]",
                                "object", "Object", "BigInt64Array", "0,0,0", "[[BigUint64Array]]", "object", "Object",
                                "BigUint64Array", "0,0,0" } },
            { "function0", { "function", "Function", "function function0( { [js code] }",
                             "Cannot get source code of function" } },
            { "generator0", { "function", "Generator", "function* generator0( { [js code] }",
                              "Cannot get source code of function" } },
            { "map0", { "object", "map", "Map", "Map(0)", "[object Map]", "size", "number", "0", "0", "[[Entries]]",
                        "object", "array", "Array", "Array(0)", "" } },
            { "set0", { "object", "set", "Set", "Set(0)", "[object Set]", "size", "number", "0", "0", "[[Entries]]",
                        "object", "array", "Array", "Array(0)", "" } },
            { "undefined0", { "undefined" } },
            { "array0", { "object", "array", "Array", "Array(2)", "Apple,Banana", "0", "string", "Apple", "Apple",
                          "1", "string", "Banana", "Banana", "length", "number", "2", "2" } },
            { "regexp0", { "object", "regexp", "RegExp", "/^\\d+\\.\\d+$/i", "/^\\d+\\.\\d+$/i", "global", "boolean",
                           "false", "false", "ignoreCase", "boolean", "true", "true", "multiline", "boolean", "false",
                           "false", "dotAll", "boolean", "false", "false", "hasIndices", "boolean", "false", "false",
                           "unicode", "boolean", "false", "false", "sticky", "boolean", "false", "false", "flags",
                           "string", "i", "i", "source", "string", "^\\d+\\.\\d+$", "^\\d+\\.\\d+$", "lastIndex",
                           "number", "0", "0" } },
            { "uint8array0", { "object", "Object", "Uint8Array(24)",
                               "0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0", "0", "number", "0", "0",
                               "1", "number", "0", "0", "2", "number", "0", "0", "3", "number", "0", "0", "4",
                               "number", "0", "0", "5", "number", "0", "0", "6", "number", "0", "0", "7",
                               "number", "0", "0", "8", "number", "0", "0", "9", "number", "0", "0", "10",
                               "number", "0", "0", "11", "number", "0", "0", "12", "number", "0", "0", "13",
                               "number", "0", "0", "14", "number", "0", "0", "15", "number", "0", "0", "16",
                               "number", "0", "0", "17", "number", "0", "0", "18", "number", "0", "0", "19",
                               "number", "0", "0", "20", "number", "0", "0", "21", "number", "0", "0", "22",
                               "number", "0", "0", "23", "number", "0", "0" } },
            { "dataview0", { "object", "dataview", "Dataview", "DataView(24)", "[object DataView]", "buffer",
                             "object", "arraybuffer", "Arraybuffer", "Arraybuffer(24)", "[object ArrayBuffer]",
                             "byteLength", "number", "24", "24", "byteOffset", "number", "0", "0" } },
            { "bigint0", { "bigint", "999n", "999" } },
            { "typedarray0", { "object", "Object", "Uint8Array(0)", "", "none" } },
            { "sharedarraybuffer0", { "object", "Object", "SharedArrayBuffer(32)", "[object SharedArrayBuffer]",
                                      "[[Int8Array]]", "object", "Object", "Int8Array(32)",
                                      "0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0",
                                      "[[Uint8Array]]", "object", "Object", "Uint8Array(32)",
                                      "0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0",
                                      "[[Int16Array]]", "object", "Object", "Int16Array(16)",
                                      "0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0", "[[Int32Array]]", "object", "Object",
                                      "Int32Array(8)", "0,0,0,0,0,0,0,0", "[[ArrayBufferByteLength]]", "number", "32",
                                      "32", "byteLength", "number", "32", "32" } },
            { "weakref0", { "object", "Object", "WeakRef {}", "[object WeakRef]", "none" } },
            { "iterator0", { "function", "Function", "function [Symbol.iterator]( { [native code] }",
                             "function [Symbol.iterator]() { [native code] }" } },
            { "array1", { "object", "array", "Array", "Array(3)", "banana,apple,peach", "0", "string", "banana",
                          "banana", "1", "string", "apple", "apple", "2", "string", "peach", "peach", "length",
                          "number", "3", "3" } },
            { "array2", { "object", "array", "Array", "Array(3)", "banana,apple,peach", "0", "string", "banana",
                          "banana", "1", "string", "apple", "apple", "2", "string", "peach", "peach", "length",
                          "number", "3", "3" } },
            { "array3", { "object", "array", "Array", "Array(1)", "apple", "0", "string", "apple", "apple",
                          "length", "number", "1", "1" } },
            { "array4", { "string", "banana", "banana" } },
            { "array5", { "object", "array", "Array", "Array(1)", "", "length", "number", "1", "1" } },
            { "array6", { "object", "array", "Array", "Array(1)", "helloworld", "0", "string", "helloworld",
                          "helloworld", "length", "number", "1", "1" } },
            { "array7", { "object", "array", "Array", "Array(1)", "false", "0", "object", "Object",
                          "Boolean{[[PrimitiveValue]]: false}", "false", "length", "number", "1", "1" } },
            { "array8", { "object", "array", "Array", "Array(1)", "[object Object]", "0", "object", "Object",
                          "Object", "[object Object]", "length", "number", "1", "1" } },
            { "array9", { "object", "array", "Array", "Array(1)", "Cannot get source code of function", "0", "function",
                          "Function", "function function0( { [js code] }", "Cannot get source code of function",
                          "length", "number", "1", "1" } },
            { "array10", { "object", "array", "Array", "Array(1)", "[object Map]", "0", "object", "map", "Map",
                           "Map(0)", "[object Map]", "length", "number", "1", "1" } },
            { "array11", { "object", "array", "Array", "Array(1)", "[object Set]", "0", "object", "set", "Set",
                           "Set(0)", "[object Set]", "length", "number", "1", "1" } },
            { "array12", { "object", "array", "Array", "Array(1)", "", "0", "undefined", "length", "number",
                           "1", "1" } },
            { "array13", { "object", "array", "Array", "Array(1)", "Apple,Banana", "0", "object", "array", "Array",
                           "Array(2)", "Apple,Banana", "length", "number", "1", "1" } },
            { "array14", { "object", "array", "Array", "Array(1)", "Cannot get source code of function", "0",
                           "function", "Generator", "function* generator0( { [js code] }",
                           "Cannot get source code of function", "length", "number", "1", "1" } },
            { "array15", { "object", "array", "Array", "Array(1)", "/^\\d+\\.\\d+$/i", "0", "object", "regexp",
                           "RegExp", "/^\\d+\\.\\d+$/i", "/^\\d+\\.\\d+$/i", "length", "number", "1", "1" } },
            { "array16", { "object", "array", "Array", "Array(1)", "[object ArrayBuffer]", "0", "object",
                           "arraybuffer", "Arraybuffer", "Arraybuffer(24)", "[object ArrayBuffer]",
                           "length", "number", "1", "1" } },
            { "array17", { "object", "array", "Array", "Array(1)",
                           "0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0", "0", "object", "Object",
                           "Uint8Array(24)", "0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0",
                           "length", "number", "1", "1" } },
            { "array18", { "object", "array", "Array", "Array(1)", "[object DataView]", "0", "object", "dataview",
                           "Dataview", "DataView(24)", "[object DataView]", "length", "number", "1", "1" } },
            { "array19", { "object", "array", "Array", "Array(1)", "999", "0", "bigint", "999n", "999", "length",
                           "number", "1", "1" } },
            { "array20", { "object", "array", "Array", "Array(3)", "banana,apple,peach", "0", "string", "banana",
                           "banana", "1", "string", "apple", "apple", "2", "string", "peach", "peach", "length",
                           "number", "3", "3" } },
            { "array21", { "string", "banana", "banana" } },
            { "typedarray1", { "object", "Object", "Int8Array(0)", "", "none" } },
            { "typedarray2", { "object", "Object", "Uint8ClampedArray", "", "none" } },
            { "typedarray3", { "object", "Object", "Int16Array(0)", "", "none" } },
            { "typedarray4", { "object", "Object", "Uint16Array", "", "none" } },
            { "typedarray5", { "object", "Object", "Int32Array(0)", "", "none" } },
            { "typedarray6", { "object", "Object", "Uint32Array", "", "none" } },
            { "typedarray7", { "object", "Object", "Float32Array", "", "none" } },
            { "typedarray8", { "object", "Object", "Float64Array", "", "none" } },
            { "typedarray9", { "object", "Object", "BigInt64Array", "", "none" } },
            { "typedarray10", { "object", "Object", "BigUint64Array", "", "none" } },
            { "typedarray11", { "object", "Object", "Uint8Array(1)", "0", "0", "number", "0", "0" } },
            { "iterator1", { "function", "Function", "function values( { [native code] }",
                             "function values() { [native code] }" } },
            { "iterator3", { "function", "Function", "function values( { [native code] }",
                             "function values() { [native code] }" } },
            { "iterator2", { "function", "Function", "function entries( { [native code] }",
                             "function entries() { [native code] }" } },
            { "iterator4", { "function", "Function", "function values( { [native code] }",
                             "function values() { [native code] }" } },
            { "iterator5", { "function", "Function", "function [Symbol.iterator]( { [native code] }",
                             "function [Symbol.iterator]() { [native code] }" } },
            { "iterator6", { "function", "Function", "function values( { [native code] }",
                             "function values() { [native code] }" } },
            { "iterator7", { "function", "Function", "function values( { [native code] }",
                             "function values() { [native code] }" } },
            { "iterator8", { "function", "Function", "function values( { [native code] }",
                             "function values() { [native code] }" } },
            { "iterator9", { "function", "Function", "function values( { [native code] }",
                             "function values() { [native code] }" } },
            { "iterator10", { "function", "Function", "function values( { [native code] }",
                              "function values() { [native code] }" } },
            { "iterator11", { "function", "Function", "function values( { [native code] }",
                              "function values() { [native code] }" } },
            { "iterator12", { "function", "Function", "function values( { [native code] }",
                              "function values() { [native code] }" } },
            { "iterator13", { "function", "Function", "function values( { [native code] }",
                              "function values() { [native code] }" } },
            { "iterator14", { "function", "Function", "function values( { [native code] }",
                              "function values() { [native code] }" } },
            { "iterator15", { "function", "Function", "function values( { [native code] }",
                              "function values() { [native code] }" } },
            { "iterator16", { "function", "Function", "function values( { [native code] }",
                              "function values() { [native code] }" } },
            { "iterator17", { "undefined" } },
            { "iterator18", { "undefined" } },
            { "iterator19", { "undefined" } },
            { "weakMap0", { "object", "weakmap", "Weakmap", "WeakMap(0)", "[object WeakMap]",
                            "[[Entries]]", "object", "array", "Array", "Array(0)", "" } },
            { "p1", { "object", "Object", "Number{[[PrimitiveValue]]: 1}", "1", "[[PrimitiveValue]]", "number",
                      "1", "1" } },
            { "p2", { "object", "Object", "Number{[[PrimitiveValue]]: 2}", "2", "[[PrimitiveValue]]", "number",
                      "2", "2" } },
            { "weakMap1", { "object", "weakmap", "Weakmap", "WeakMap(2) {Object => 'hello', Object => 'world'}",
                            "[object WeakMap]", "[[Entries]]", "object", "array",
                            "Array", "Array(2)", "[object Object],[object Object]" } },
            { "weakMap2", { "object", "weakmap", "Weakmap", "WeakMap(0)", "[object WeakMap]", "[[Entries]]",
                            "object", "array", "Array", "Array(0)", "", "0", "string", "hello", "hello" } },
            { "weakMap3", { "object", "weakmap", "Weakmap", "WeakMap(1) {Object => 'weakMap0'}", "[object WeakMap]",
                            "[[Entries]]", "object", "array", "Array", "Array(1)", "[object Object]" } },
            { "weakMap4", { "object", "weakmap", "Weakmap", "WeakMap(2) {Object => 37, Object => 'azerty'}",
                            "[object WeakMap]", "[[Entries]]", "object", "array",
                            "Array", "Array(2)", "[object Object],[object Object]" } },
            { "weakMap5", { "object", "weakmap", "Weakmap", "WeakMap(1) {Object => undefined}", "[object WeakMap]",
                            "[[Entries]]", "object", "array", "Array", "Array(1)", "[object Object]" } },
            { "weakSet0", { "object", "weakset", "Weakset", "WeakSet(0)", "[object WeakSet]",
                            "[[Entries]]", "object", "array", "Array", "Array(0)", "" } },
            { "weakSet1", { "object", "weakset", "Weakset", "WeakSet(1) {Object}", "[object WeakSet]",
                            "[[Entries]]", "object", "array", "Array", "Array(1)", "[object Object]" } },
            { "weakSet2", { "object", "weakset", "Weakset", "WeakSet(2) {Object, Object}", "[object WeakSet]",
                            "[[Entries]]", "object", "array", "Array", "Array(2)",
                            "[object Object],[object Object]" } },
            { "Parent", { "function", "Function", "function Parent( { [js code] }",
                          "Cannot get source code of function" } },
            { "class1", { "object", "Object", "customClass", "[object Object]", "_a", "number", "1", "1",
                          "_b", "number", "2", "2", "child", "object", "Object", "Child", "[object Object]", "print",
                          "function", "Function", "function ( { [js code] }", "Cannot get source code of function" } },
            { "parent", { "object", "Object", "Parent", "[object Object]", "name", "string", "parent", "parent",
                          "age", "number", "50", "50" } },
            { "customClass", { "function", "Function", "function customClass( { [js code] }",
                               "Cannot get source code of function" } },
            { "child", { "object", "Object", "Child", "[object Object]", "name", "string", "child", "child",
                         "age", "number", "15", "15", "idNumber", "string", "1234", "1234" } },
            { "class2", { "object", "Object", "Object", "[object Object]", "name", "string", "class2", "class2" } },
            { "Child", { "function", "Function", "function Child( { [js code] }",
                         "Cannot get source code of function" } },
        };

        int32_t index_ {0};
        const EcmaVM *vm_ {nullptr};
        RuntimeImpl *runtime_ {nullptr};
    };

    std::string entryPoint_ = "variable_second";
    JSPtLocation location_ {nullptr, JSPtLocation::EntityId(0), 0};
    size_t breakpointCounter_ = 0;
};

std::unique_ptr<TestEvents> GetJsVariableSecondTest()
{
    return std::make_unique<JsVariableSecondTest>();
}
}  // namespace panda::ecmascript::tooling::test

#endif  // ECMASCRIPT_TOOLING_TEST_UTILS_TESTCASES_JS_VARIABLE_TEST_H
