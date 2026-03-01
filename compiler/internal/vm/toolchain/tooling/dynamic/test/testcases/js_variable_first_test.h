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

#ifndef ECMASCRIPT_TOOLING_TEST_UTILS_TESTCASES_JS_VARIABLE_TEST_H
#define ECMASCRIPT_TOOLING_TEST_UTILS_TESTCASES_JS_VARIABLE_TEST_H

#include "test/utils/test_util.h"

namespace panda::ecmascript::tooling::test {
class JsVariableFirstTest : public TestEvents {
public:
    JsVariableFirstTest()
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
            std::string pandaFile = DEBUGGER_ABC_DIR "variable_first.abc";
            std::string sourceFile = DEBUGGER_JS_DIR "variable_first.js";
            static_cast<JsVariableFirstTestChannel *>(channel_)->Initial(vm_, runtime_);
            runtime_->Enable();
            // 306: breakpointer line
            int32_t lineNumber = 314;
            location_ = TestUtil::GetLocation(sourceFile.c_str(), lineNumber, 0, pandaFile.c_str());
            ASSERT_TRUE(location_.GetMethodId().IsValid());
            TestUtil::SuspendUntilContinue(DebugEvent::LOAD_MODULE);
            ASSERT_EQ(moduleName, pandaFile);
            ASSERT_TRUE(debugger_->NotifyScriptParsed(0, pandaFile));
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

        channel_ = new JsVariableFirstTestChannel();
    }

    std::pair<std::string, std::string> GetEntryPoint() override
    {
        std::string pandaFile = DEBUGGER_ABC_DIR "variable_first.abc";
        return {pandaFile, entryPoint_};
    }
    ~JsVariableFirstTest()
    {
        delete channel_;
        channel_ = nullptr;
    }

private:
    class JsVariableFirstTestChannel : public TestChannel {
    public:
        JsVariableFirstTestChannel() = default;
        ~JsVariableFirstTestChannel() = default;
        void Initial(const EcmaVM *vm, RuntimeImpl *runtime)
        {
            vm_ = vm;
            runtime_ = runtime;
        }

        void SendNotification(const PtBaseEvents &events) override
        {
            const static std::vector<std::function<bool(const PtBaseEvents &events)>> eventList = {
                [](const PtBaseEvents &events) -> bool {
                    std::string sourceFile = DEBUGGER_JS_DIR "variable_first.js";
                    auto parsed = static_cast<const ScriptParsed *>(&events);
                    std::string str = parsed->ToJson()->Stringify();
                    std::cout << "JsVariableFirstTestChannel: SendNotification 0:\n" << str << std::endl;

                    ASSERT_EQ(parsed->GetName(), "Debugger.scriptParsed");
                    ASSERT_EQ(parsed->GetUrl(), sourceFile);
                    return true;
                },
                [this](const PtBaseEvents &events) -> bool {
                    auto paused = static_cast<const Paused *>(&events);
                    std::string str = paused->ToJson()->Stringify();
                    std::cout << "JsVariableFirstTestChannel: SendNotification 1:\n" << str << std::endl;

                    ASSERT_EQ(paused->GetName(), "Debugger.paused");
                    auto frame = paused->GetCallFrames()->at(0).get();
                    ASSERT_EQ(frame->GetFunctionName(), "resolveHandler");
                    auto scopes = frame->GetScopeChain();
                    ASSERT_EQ(scopes->size(), 3U);  // 2: contain local, global and closure
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
        NO_COPY_SEMANTIC(JsVariableFirstTestChannel);
        NO_MOVE_SEMANTIC(JsVariableFirstTestChannel);

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
            { "set1", { "object", "set", "Set", "Set(1) {1}", "[object Set]", "size", "number", "1", "1",
                        "[[Entries]]", "object", "array", "Array", "Array(1)", "1" } },
            { "set2", { "object", "set", "Set", "Set(7) {'h', 'e', 'l', 'o', 'w', ...}", "[object Set]", "size",
                        "number", "7", "7", "[[Entries]]", "object", "array", "Array", "Array(7)",
                        "h,e,l,o,w,r,d" } },
            { "set3", { "object", "set", "Set", "Set(1) {Object}", "[object Set]", "size", "number", "1", "1",
                        "[[Entries]]", "object", "array", "Array", "Array(1)", "[object Object]" } },
            { "set4", { "object", "set", "Set", "Set(0)", "[object Set]", "size", "number", "0", "0", "[[Entries]]",
                        "object", "array", "Array", "Array(0)", "" } },
            { "set5", { "object", "set", "Set", "Set(2) {'Apple', 'Banana'}", "[object Set]", "size", "number", "2",
                        "2", "[[Entries]]", "object", "array", "Array", "Array(2)", "Apple,Banana" } },
            { "set6", { "object", "set", "Set", "Set(0)", "[object Set]", "size", "number", "0", "0", "[[Entries]]",
                        "object", "array", "Array", "Array(0)", "" } },
            { "set7", { "object", "set", "Set", "Set(0)", "[object Set]", "size", "number", "0", "0", "[[Entries]]",
                        "object", "array", "Array", "Array(0)", "" } },
            { "set8", { "object", "set", "Set", "Set(1) {Object}", "[object Set]", "size", "number", "1", "1",
                        "[[Entries]]", "object", "array", "Array", "Array(1)", "[object Object]" } },
            { "set9", { "object", "set", "Set", "Set(1) {Object}", "[object Set]", "size", "number", "1", "1",
                        "[[Entries]]", "object", "array", "Array", "Array(1)", "[object Object]" } },
            { "set10", { "object", "set", "Set", "Set(1) {Object}", "[object Set]", "size", "number", "1", "1",
                         "[[Entries]]", "object", "array", "Array", "Array(1)", "[object Object]" } },
            { "set11", { "object", "set", "Set", "Set(1) {Object}", "[object Set]", "size", "number", "1", "1",
                         "[[Entries]]", "object", "array", "Array", "Array(1)", "[object Object]" } },
            { "set12", { "object", "set", "Set", "Set(1) {Object}", "[object Set]", "size", "number", "1", "1",
                         "[[Entries]]", "object", "array", "Array", "Array(1)", "[object Object]" } },
            { "set13", { "object", "set", "Set", "Set(1) {Object}", "[object Set]", "size", "number", "1", "1",
                         "[[Entries]]", "object", "array", "Array", "Array(1)", "[object Object]" } },
            { "set14", { "object", "set", "Set", "Set(0)", "[object Set]", "size", "number", "0", "0", "[[Entries]]",
                         "object", "array", "Array", "Array(0)", "" } },
            { "set15", { "object", "set", "Set", "Set(4) {0, 'hello', Object, 1}", "[object Set]", "size",
                         "number", "4", "4", "[[Entries]]", "object", "array", "Array", "Array(4)",
                         "0,hello,[object Object],1" } },
            { "set16", { "object", "set", "Set", "Set(1) {Object}", "[object Set]", "size", "number", "1", "1",
                         "[[Entries]]", "object", "array", "Array", "Array(1)", "[object Object]" } },
            { "set17", { "object", "set", "Set", "Set(1) {999}", "[object Set]", "size", "number", "1", "1",
                         "[[Entries]]", "object", "array", "Array", "Array(1)", "999" } },
            { "set18", { "object", "set", "Set", "Set(1) {Object}", "[object Set]", "size", "number", "1", "1",
                         "[[Entries]]", "object", "array", "Array", "Array(1)", "[object Object]" } },
            { "set19", { "object", "set", "Set", "Set(0)", "[object Set]", "size", "number", "0", "0",
                         "[[Entries]]", "object", "array", "Array", "Array(0)", "" } },
            { "number1", { "number", "65535", "65535" } },
            { "number2", { "number", "5e-324", "5e-324" } },
            { "number3", { "number", "10000000000", "10000000000" } },
            { "number4", { "number", "2199023255551", "2199023255551" } },
            { "number5", { "number", "16383", "16383" } },
            { "number6", { "object", "Object", "Number{[[PrimitiveValue]]: 999}", "999", "[[PrimitiveValue]]",
                           "number", "999", "999" } },
            { "number7", { "number", "1.23e+47", "1.23e+47" } },
            { "number8", { "number", "1", "1" } },
            { "number9", { "number", "65536", "65536" } },
            { "number10", { "number", "-65534", "-65534" } },
            { "number11", { "number", "65535", "65535" } },
            { "number12", { "number", "0.000015259021896696422", "0.000015259021896696422" } },
            { "number13", { "number", "1", "1" } },
            { "number14", { "object", "Object", "Number{[[PrimitiveValue]]: 0}", "0", "[[PrimitiveValue]]", "number",
                            "0", "0" } },
            { "number15", { "object", "Object", "Number{[[PrimitiveValue]]: 1.7976931348623157e+308}",
                            "1.7976931348623157e+308", "[[PrimitiveValue]]", "number", "1.7976931348623157e+308",
                            "1.7976931348623157e+308" } },
            { "number16", { "object", "Object", "Number{[[PrimitiveValue]]: 5e-324}", "5e-324", "[[PrimitiveValue]]",
                            "number", "5e-324", "5e-324" } },
            { "number17", { "object", "Object", "Number{[[PrimitiveValue]]: 10000000000}", "10000000000",
                            "[[PrimitiveValue]]", "number", "10000000000", "10000000000" } },
            { "number18", { "object", "Object", "Number{[[PrimitiveValue]]: 2199023255551}", "2199023255551",
                            "[[PrimitiveValue]]", "number", "2199023255551", "2199023255551" } },
            { "number19", { "object", "Object", "Number{[[PrimitiveValue]]: 16383}", "16383", "[[PrimitiveValue]]",
                            "number", "16383", "16383" } },
            { "number20", { "object", "Object", "Number{[[PrimitiveValue]]: 1.23e+47}", "1.23e+47",
                            "[[PrimitiveValue]]", "number", "1.23e+47", "1.23e+47" } },
            { "number21", { "object", "Object", "Number{[[PrimitiveValue]]: 1}", "1", "[[PrimitiveValue]]", "number",
                            "1", "1" } },
            { "number22", { "object", "Object", "Number{[[PrimitiveValue]]: 65536}", "65536", "[[PrimitiveValue]]",
                            "number", "65536", "65536" } },
            { "number23", { "object", "Object", "Number{[[PrimitiveValue]]: -65534}", "-65534", "[[PrimitiveValue]]",
                            "number", "-65534", "-65534" } },
            { "number24", { "object", "Object", "Number{[[PrimitiveValue]]: 65535}", "65535", "[[PrimitiveValue]]",
                            "number", "65535", "65535" } },
            { "number25", { "object", "Object", "Number{[[PrimitiveValue]]: 0.000015259021896696422}",
                            "0.000015259021896696422", "[[PrimitiveValue]]", "number", "0.000015259021896696422",
                            "0.000015259021896696422" } },
            { "number26", { "object", "Object", "Number{[[PrimitiveValue]]: 1}", "1", "[[PrimitiveValue]]", "number",
                            "1", "1" } },
            { "number27", { "number", "1.7976931348623157e+308", "1.7976931348623157e+308" } },
            { "string1", { "string", "", "" } },
            { "string2", { "string", "", "" } },
            { "string3", { "string", "world", "world" } },
            { "string4", { "string", "helloworld", "helloworld" } },
            { "string5", { "string", "e", "e" } },
            { "string6", { "string", "1", "1" } },
            { "string7", { "object", "Object", "String{[[PrimitiveValue]]: }", "", "[[PrimitiveValue]]", "string", "",
                           "", "length", "number", "0", "0" } },
            { "string8", { "object", "Object", "String{[[PrimitiveValue]]: [object Set]}", "[object Set]",
                           "[[PrimitiveValue]]", "string", "[object Set]", "[object Set]", "0", "string", "[", "[",
                           "1", "string", "o", "o", "2", "string", "b", "b", "3", "string", "j", "j", "4", "string",
                           "e", "e", "5", "string", "c", "c", "6", "string", "t", "t", "7", "string", " ", " ", "8",
                           "string", "S", "S", "9", "string", "e", "e", "10", "string", "t", "t", "11", "string", "]",
                           "]", "length", "number", "12", "12" } },
            { "string9", { "object", "Object", "String{[[PrimitiveValue]]: 1}", "1", "[[PrimitiveValue]]", "string",
                           "1", "1", "0", "string", "1", "1", "length", "number", "1", "1" } },
            { "string10", { "object", "Object", "String{[[PrimitiveValue]]: helloworld}", "helloworld",
                            "[[PrimitiveValue]]", "string", "helloworld", "helloworld", "0", "string", "h", "h", "1",
                            "string", "e", "e", "2", "string", "l", "l", "3", "string", "l", "l", "4", "string", "o",
                            "o", "5", "string", "w", "w", "6", "string", "o", "o", "7", "string", "r", "r", "8",
                            "string", "l", "l", "9", "string", "d", "d", "length", "number", "10", "10" } },
            { "string11", { "object", "Object", "String{[[PrimitiveValue]]: [object Object]}", "[object Object]",
                            "[[PrimitiveValue]]", "string", "[object Object]", "[object Object]", "0", "string", "[",
                            "[", "1", "string", "o", "o", "2", "string", "b", "b", "3", "string", "j", "j", "4",
                            "string", "e", "e", "5", "string", "c", "c", "6", "string", "t", "t", "7", "string", " ",
                            " ", "8", "string", "O", "O", "9", "string", "b", "b", "10", "string", "j", "j", "11",
                            "string", "e", "e", "12", "string", "c", "c", "13", "string", "t", "t", "14", "string",
                            "]", "]", "length", "number", "15", "15" } },
            { "string12", { "object", "Object", "String{[[PrimitiveValue]]: undefined}", "undefined",
                            "[[PrimitiveValue]]", "string", "undefined", "undefined", "0", "string", "u", "u", "1",
                            "string", "n", "n", "2", "string", "d", "d", "3", "string", "e", "e", "4", "string", "f",
                            "f", "5", "string", "i", "i", "6", "string", "n", "n", "7", "string", "e", "e", "8",
                            "string", "d", "d", "length", "number", "9", "9" } },
            { "string13", { "object", "Object", "String{[[PrimitiveValue]]: Apple,Banana}", "Apple,Banana",
                            "[[PrimitiveValue]]", "string", "Apple,Banana", "Apple,Banana", "0", "string", "A", "A",
                            "1", "string", "p", "p", "2", "string", "p", "p", "3", "string", "l", "l", "4", "string",
                            "e", "e", "5", "string", ",", ",", "6", "string", "B", "B", "7", "string", "a", "a", "8",
                            "string", "n", "n", "9", "string", "a", "a", "10", "string", "n", "n", "11", "string",
                            "a", "a", "length", "number", "12", "12" } },
            { "string14", { "object", "Object", "String{[[PrimitiveValue]]: 999}", "999", "[[PrimitiveValue]]",
                            "string", "999", "999", "0", "string", "9", "9", "1", "string", "9", "9", "2", "string",
                            "9", "9", "length", "number", "3", "3" } },
            { "string15", { "object", "Object", "String{[[PrimitiveValue]]: Cannot get source code of function}",
                            "Cannot get source code of function", "[[PrimitiveValue]]", "string",
                            "Cannot get source code of function", "Cannot get source code of function", "0", "string",
                            "C", "C", "1", "string", "a", "a", "2", "string", "n", "n", "3", "string", "n", "n", "4",
                            "string", "o", "o", "5", "string", "t", "t", "6", "string", " ", " ", "7", "string", "g",
                            "g", "8", "string", "e", "e", "9", "string", "t", "t", "10", "string", " ", " ", "11",
                            "string", "s", "s", "12", "string", "o", "o", "13", "string", "u", "u", "14", "string",
                            "r", "r", "15", "string", "c", "c", "16", "string", "e", "e", "17", "string", " ", " ",
                            "18", "string", "c", "c", "19", "string", "o", "o", "20", "string", "d", "d", "21",
                            "string", "e", "e", "22", "string", " ", " ", "23", "string", "o", "o", "24", "string",
                            "f", "f", "25", "string", " ", " ", "26", "string", "f", "f", "27", "string", "u", "u",
                            "28", "string", "n", "n", "29", "string", "t", "t", "30", "string", "i", "i", "31",
                            "string", "o", "o", "32", "string", "n", "n", "length", "number", "33", "33" } },
            { "string16", { "object", "Object", "String{[[PrimitiveValue]]: /^\\d+\\.\\d+$/i}", "/^\\d+\\.\\d+$/i",
                            "[[PrimitiveValue]]", "string", "/^\\d+\\.\\d+$/i", "/^\\d+\\.\\d+$/i", "0", "string", "/",
                            "/", "1", "string", "^", "^", "2", "string", "\\", "\\", "3", "string", "d", "d", "4",
                            "string", "+", "+", "5", "string", "\\", "\\", "6", "string", ".", ".", "7", "string",
                            "\\", "\\", "8", "string", "d", "d", "9", "string", "+", "+", "10", "string", "$", "$",
                            "11", "string", "/", "/", "12", "string", "i", "i", "length", "number", "13", "13"} },
            { "string17", { "object", "Object", "String{[[PrimitiveValue]]: [object ArrayBuffer]}",
                            "[object ArrayBuffer]", "[[PrimitiveValue]]", "string", "[object ArrayBuffer]",
                            "[object ArrayBuffer]", "0", "string", "[", "[", "1", "string", "o", "o", "2", "string",
                            "b", "b", "3", "string", "j", "j", "4", "string", "e", "e", "5", "string", "c", "c", "6",
                            "string", "t", "t", "7", "string", " ", " ", "8", "string", "A", "A", "9", "string", "r",
                            "r", "10", "string", "r", "r", "11", "string", "a", "a", "12", "string", "y", "y", "13",
                            "string", "B", "B", "14", "string", "u", "u", "15", "string", "f", "f", "16", "string",
                            "f", "f", "17", "string", "e", "e", "18", "string", "r", "r", "19", "string", "]", "]",
                            "length", "number", "20", "20" } },
            { "string18", { "object", "Object",
                            "String{[[PrimitiveValue]]: 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0}",
                            "0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0", "[[PrimitiveValue]]", "string",
                            "0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0",
                            "0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0", "0", "string", "0", "0", "1",
                            "string", ",", ",", "2", "string", "0", "0", "3", "string", ",", ",", "4", "string", "0",
                            "0", "5", "string", ",", ",", "6", "string", "0", "0", "7", "string", ",", ",", "8",
                            "string", "0", "0", "9", "string", ",", ",", "10", "string", "0", "0", "11", "string",
                            ",", ",", "12", "string", "0", "0", "13", "string", ",", ",", "14", "string", "0", "0",
                            "15", "string", ",", ",", "16", "string", "0", "0", "17", "string", ",", ",", "18",
                            "string", "0", "0", "19", "string", ",", ",", "20", "string", "0", "0", "21", "string",
                            ",", ",", "22", "string", "0", "0", "23", "string", ",", ",", "24", "string", "0", "0",
                            "25", "string", ",", ",", "26", "string", "0", "0", "27", "string", ",", ",", "28",
                            "string", "0", "0", "29", "string", ",", ",", "30", "string", "0", "0", "31", "string",
                            ",", ",", "32", "string", "0", "0", "33", "string", ",", ",", "34", "string", "0", "0",
                            "35", "string", ",", ",", "36", "string", "0", "0", "37", "string", ",", ",", "38",
                            "string", "0", "0", "39", "string", ",", ",", "40", "string", "0", "0", "41", "string",
                            ",", ",", "42", "string", "0", "0", "43", "string", ",", ",", "44", "string", "0", "0",
                            "45", "string", ",", ",", "46", "string", "0", "0", "length", "number", "47", "47" } },
            { "string19", { "object", "Object", "String{[[PrimitiveValue]]: [object DataView]}", "[object DataView]",
                            "[[PrimitiveValue]]", "string", "[object DataView]", "[object DataView]",
                            "0", "string", "[", "[", "1", "string", "o", "o", "2", "string", "b", "b", "3", "string",
                            "j", "j", "4", "string", "e", "e", "5", "string", "c", "c", "6", "string", "t", "t", "7",
                            "string", " ", " ", "8", "string", "D", "D", "9", "string", "a", "a", "10", "string", "t",
                            "t", "11", "string", "a", "a", "12", "string", "V", "V", "13", "string", "i", "i", "14",
                            "string", "e", "e", "15", "string", "w", "w", "16", "string", "]", "]", "length", "number",
                            "17", "17" } },
            { "string20", { "object", "Object", "String{[[PrimitiveValue]]: [object Map]}", "[object Map]",
                            "[[PrimitiveValue]]", "string", "[object Map]", "[object Map]", "0", "string", "[", "[",
                            "1", "string", "o", "o", "2", "string", "b", "b", "3", "string", "j", "j", "4", "string",
                            "e", "e", "5", "string", "c", "c", "6", "string", "t", "t", "7", "string", " ", " ", "8",
                            "string", "M", "M", "9", "string", "a", "a", "10", "string", "p", "p", "11", "string",
                            "]", "]", "length", "number", "12", "12" } },
            { "string21", { "object", "Object", "String{[[PrimitiveValue]]: Cannot get source code of function}",
                            "Cannot get source code of function", "[[PrimitiveValue]]", "string",
                            "Cannot get source code of function", "Cannot get source code of function", "0", "string",
                            "C", "C", "1", "string", "a", "a", "2", "string", "n", "n", "3", "string", "n", "n", "4",
                            "string", "o", "o", "5", "string", "t", "t", "6", "string", " ", " ", "7", "string", "g",
                            "g", "8", "string", "e", "e", "9", "string", "t", "t", "10", "string", " ", " ", "11",
                            "string", "s", "s", "12", "string", "o", "o", "13", "string", "u", "u", "14", "string",
                            "r", "r", "15", "string", "c", "c", "16", "string", "e", "e", "17", "string", " ", " ",
                            "18", "string", "c", "c", "19", "string", "o", "o", "20", "string", "d", "d", "21",
                            "string", "e", "e", "22", "string", " ", " ", "23", "string", "o", "o", "24", "string",
                            "f", "f", "25", "string", " ", " ", "26", "string", "f", "f", "27", "string", "u", "u",
                            "28", "string", "n", "n", "29", "string", "t", "t", "30", "string", "i", "i", "31",
                            "string", "o", "o", "32", "string", "n", "n", "length", "number", "33", "33" } },
            { "string22", { "string", "�", "�" } },
            { "string23", { "string", "🙂", "🙂" } },
            { "string24", { "string", "�", "�" } },
            { "string25", { "string", "👨‍👨‍👧‍👦", "👨‍👨‍👧‍👦" } },
            { "bigint1", { "bigint", "9007199254740991n", "9007199254740991" } },
            { "bigint2", { "bigint", "9007199254740991n", "9007199254740991" } },
            { "bigint3", { "bigint", "9007199254740991n", "9007199254740991" } },
            { "bigint4", { "bigint", "9007199254740991n", "9007199254740991" } },
            { "bigint5", { "bigint", "9007199254740991n", "9007199254740991" } },
            { "bigint6", { "bigint", "9007199254740991n", "9007199254740991" } },
            { "bigint7", { "bigint", "999n", "999" } },
            { "bigint8", { "bigint", "9007199254741990n", "9007199254741990" } },
            { "bigint9", { "bigint", "-9007199254739992n", "-9007199254739992" } },
            { "bigint10", { "bigint", "8998192055486250009n", "8998192055486250009" } },
            { "bigint11", { "bigint", "0n", "0" } },
            { "bigint12", { "bigint", "999n", "999" } },
            { "bigint13", { "bigint", "10000000000n", "10000000000" } },
            { "bigint14", { "bigint", "888888888888888888888888888888888888888888888n",
                            "888888888888888888888888888888888888888888888" } },
            { "bigint15", { "bigint", "16383n", "16383" } },
            { "bigint16", { "bigint", "0n", "0" } },
            { "bigint17", { "bigint", "0n", "0" } },
            { "bigint18", { "bigint", "122999999999999994846185700645503654167417192448n",
                            "122999999999999994846185700645503654167417192448" } },
            { "bigint19", { "bigint", "1234567n", "1234567" } },
            { "bigint20", { "bigint", "65535n", "65535" } },
            { "boolean1", { "object", "Object", "Boolean{[[PrimitiveValue]]: true}", "true", "[[PrimitiveValue]]",
                            "boolean", "true", "true" } },
            { "boolean2", { "object", "Object", "Boolean{[[PrimitiveValue]]: true}", "true", "[[PrimitiveValue]]",
                            "boolean", "true", "true" } },
            { "boolean3", { "object", "Object", "Boolean{[[PrimitiveValue]]: true}", "true", "[[PrimitiveValue]]",
                            "boolean", "true", "true" } },
            { "boolean4", { "object", "Object", "Boolean{[[PrimitiveValue]]: false}", "false", "[[PrimitiveValue]]",
                            "boolean", "false", "false" } },
            { "boolean5", { "object", "Object", "Boolean{[[PrimitiveValue]]: true}", "true", "[[PrimitiveValue]]",
                            "boolean", "true", "true" } },
            { "boolean6", { "object", "Object", "Boolean{[[PrimitiveValue]]: true}", "true", "[[PrimitiveValue]]",
                            "boolean", "true", "true" } },
            { "boolean7", { "object", "Object", "Boolean{[[PrimitiveValue]]: true}", "true", "[[PrimitiveValue]]",
                            "boolean", "true", "true" } },
            { "boolean8", { "object", "Object", "Boolean{[[PrimitiveValue]]: false}", "false", "[[PrimitiveValue]]",
                            "boolean", "false", "false" } },
            { "boolean9", { "boolean", "true", "true" } },
            { "boolean10", { "boolean", "false", "false" } },
            { "boolean11", { "object", "Object", "Boolean{[[PrimitiveValue]]: false}", "false", "[[PrimitiveValue]]",
                             "boolean", "false", "false" } },
            { "boolean12", { "boolean", "false", "false" } },
            { "boolean13", { "object", "Object", "Boolean{[[PrimitiveValue]]: false}", "false", "[[PrimitiveValue]]",
                             "boolean", "false", "false" } },
            { "boolean14", { "boolean", "true", "true" } },
            { "boolean15", { "object", "Object", "Boolean{[[PrimitiveValue]]: true}", "true", "[[PrimitiveValue]]",
                             "boolean", "true", "true" } },
            { "boolean16", { "object", "Object", "Boolean{[[PrimitiveValue]]: false}", "false", "[[PrimitiveValue]]",
                             "boolean", "false", "false" } },
            { "boolean17", { "object", "Object", "Boolean{[[PrimitiveValue]]: true}", "true", "[[PrimitiveValue]]",
                             "boolean", "true", "true" } },
            { "boolean18", { "object", "Object", "Boolean{[[PrimitiveValue]]: true}", "true", "[[PrimitiveValue]]",
                             "boolean", "true", "true" } },
            { "boolean19", { "object", "Object", "Boolean{[[PrimitiveValue]]: true}", "true", "[[PrimitiveValue]]",
                             "boolean", "true", "true" } },
            { "boolean20", { "object", "Object", "Boolean{[[PrimitiveValue]]: true}", "true", "[[PrimitiveValue]]",
                             "boolean", "true", "true" } },
            { "boolean21", { "object", "Object", "Boolean{[[PrimitiveValue]]: true}", "true", "[[PrimitiveValue]]",
                             "boolean", "true", "true" } },
            { "boolean22", { "object", "Object", "Boolean{[[PrimitiveValue]]: true}", "true", "[[PrimitiveValue]]",
                             "boolean", "true", "true" } },
            { "boolean23", { "object", "Object", "Boolean{[[PrimitiveValue]]: false}", "false", "[[PrimitiveValue]]",
                             "boolean", "false", "false" } },
            { "boolean24", { "object", "Object", "Boolean{[[PrimitiveValue]]: true}", "true", "[[PrimitiveValue]]",
                             "boolean", "true", "true" } },
            { "boolean25", { "object", "Object", "Boolean{[[PrimitiveValue]]: true}", "true", "[[PrimitiveValue]]",
                             "boolean", "true", "true" } },
            { "boolean26", { "object", "Object", "Boolean{[[PrimitiveValue]]: true}", "true", "[[PrimitiveValue]]",
                             "boolean", "true", "true" } },
            { "boolean27", { "object", "Object", "Boolean{[[PrimitiveValue]]: true}", "true", "[[PrimitiveValue]]",
                             "boolean", "true", "true" } },
            { "boolean28", { "object", "Object", "Boolean{[[PrimitiveValue]]: true}", "true", "[[PrimitiveValue]]",
                             "boolean", "true", "true" } },
            { "boolean29", { "object", "Object", "Boolean{[[PrimitiveValue]]: true}", "true", "[[PrimitiveValue]]",
                             "boolean", "true", "true" } },
            { "map1", { "object", "map", "Map", "Map(0)", "[object Map]", "size", "number", "0", "0", "[[Entries]]",
                        "object", "array", "Array", "Array(0)", "" } },
            { "map2", { "object", "map", "Map", "Map(2) {1 => 'hello', 2 => 'world'}", "[object Map]", "size",
                        "number", "2", "2", "[[Entries]]", "object", "array", "Array", "Array(2)",
                        "[object Object],[object Object]" } },
            { "map3", { "object", "map", "Map", "Map(1) {NaN => 'NaN'}", "[object Map]", "size", "number", "1", "1",
                        "[[Entries]]", "object", "array", "Array", "Array(1)", "[object Object]" } },
            { "map4", { "object", "map", "Map", "Map(0)", "[object Map]", "size", "number", "0", "0", "[[Entries]]",
                        "object", "array", "Array", "Array(0)", "", "0", "string", "hello", "hello" } },
            { "map5", { "object", "map", "Map", "Map(4) {0 => 'zero', 1 => 'one', 2 => 'two', 3 => 'three'}",
                        "[object Map]", "size", "number", "4", "4", "[[Entries]]", "object", "array", "Array",
                        "Array(4)", "[object Object],[object Object],[object Object],[object Object]" } },
            { "map6", { "object", "map", "Map", "Map(1) {Object => 'set0'}", "[object Map]", "size", "number", "1",
                        "1", "[[Entries]]", "object", "array", "Array", "Array(1)", "[object Object]" } },
            { "map7", { "object", "map", "Map", "Map(1) {1 => 'number0'}", "[object Map]", "size", "number", "1", "1",
                        "[[Entries]]", "object", "array", "Array", "Array(1)", "[object Object]" } },
            { "map8", { "object", "map", "Map", "Map(1) {'helloworld' => 'string0'}", "[object Map]", "size",
                        "number", "1", "1", "[[Entries]]", "object", "array", "Array", "Array(1)",
                        "[object Object]" } },
            { "map9", { "object", "map", "Map", "Map(1) {Object => 'object0'}", "[object Map]", "size", "number", "1",
                        "1", "[[Entries]]", "object", "array", "Array", "Array(1)", "[object Object]" } },
            { "map10", { "object", "map", "Map", "Map(1) {undefined => 'undefined0'}", "[object Map]", "size",
                         "number", "1", "1", "[[Entries]]", "object", "array", "Array", "Array(1)",
                         "[object Object]" } },
            { "map11", { "object", "map", "Map", "Map(1) {Object => 'array0'}", "[object Map]", "size", "number", "1",
                         "1", "[[Entries]]", "object", "array", "Array", "Array(1)", "[object Object]" } },
            { "map12", { "object", "map", "Map", "Map(1) {Object => 'map3'}", "[object Map]", "size", "number", "1",
                         "1", "[[Entries]]", "object", "array", "Array", "Array(1)", "[object Object]" } },
            { "map13", { "object", "map", "Map", "Map(1) {Object => 'generator0'}", "[object Map]", "size", "number",
                         "1", "1", "[[Entries]]", "object", "array", "Array", "Array(1)", "[object Object]" } },
            { "map14", { "object", "map", "Map", "Map(1) {Object => 'regexp0'}", "[object Map]", "size", "number",
                         "1", "1", "[[Entries]]", "object", "array", "Array", "Array(1)", "[object Object]" } },
            { "map15", { "object", "map", "Map", "Map(1) {Object => 'arraybuffer0'}", "[object Map]", "size",
                         "number", "1", "1", "[[Entries]]", "object", "array", "Array", "Array(1)",
                         "[object Object]" } },
            { "map16", { "object", "map", "Map", "Map(1) {Object => 'uint8array0'}", "[object Map]", "size", "number",
                         "1", "1", "[[Entries]]", "object", "array", "Array", "Array(1)", "[object Object]" } },
            { "map17", { "object", "map", "Map", "Map(1) {Object => 'dataview0'}", "[object Map]", "size", "number",
                         "1", "1", "[[Entries]]", "object", "array", "Array", "Array(1)", "[object Object]" } },
            { "map18", { "object", "map", "Map", "Map(1) {8998192055486250009 => 'bigint10'}", "[object Map]", "size",
                         "number", "1", "1", "[[Entries]]", "object", "array", "Array", "Array(1)",
                         "[object Object]" } },
            { "map19", { "object", "map", "Map", "Map(1) {Object => 'function0'}", "[object Map]", "size", "number",
                         "1", "1", "[[Entries]]", "object", "array", "Array", "Array(1)", "[object Object]" } },
            { "object1", { "object", "Object", "Object", "[object Object]", "0", "string", "zero", "zero", "1",
                           "string", "one", "one", "2", "string", "two", "two", "3", "string", "three", "three", "4",
                           "string", "four", "four", "5", "string", "five", "five" } },
            { "object2", { "object", "Object", "Object", "[object Object]", "0", "string", "zero", "zero", "1",
                           "string", "one", "one", "2", "string", "two", "two", "3", "string", "three", "three", "4",
                           "string", "four", "four", "5", "string", "five", "five" } },
            { "object3", { "object", "Object", "Object", "[object Object]", "0", "string", "zero", "zero", "1",
                           "string", "one", "one", "2", "string", "two", "two", "3", "string", "three", "three", "4",
                           "string", "four", "four", "5", "string", "five", "five" } },
            { "object4", { "object", "set", "Set", "Set(0)", "[object Set]", "size", "number", "0", "0",
                           "[[Entries]]", "object", "array", "Array", "Array(0)", "" } },
            { "object5", { "object", "Object", "String{[[PrimitiveValue]]: helloworld}", "helloworld",
                           "[[PrimitiveValue]]", "string", "helloworld", "helloworld", "0", "string", "h", "h", "1",
                           "string", "e", "e", "2", "string", "l", "l", "3", "string", "l", "l", "4", "string", "o",
                           "o", "5", "string", "w", "w", "6", "string", "o", "o", "7", "string", "r", "r", "8",
                           "string", "l", "l", "9", "string", "d", "d", "length", "number", "10", "10" } },
            { "object6", { "object", "map", "Map", "Map(0)", "[object Map]", "size", "number", "0", "0", "[[Entries]]",
                           "object", "array", "Array", "Array(0)", "" } },
            { "object7", { "object", "Object", "Number{[[PrimitiveValue]]: 1}", "1", "[[PrimitiveValue]]", "number",
                           "1", "1" } },
            { "object8", { "object", "Object", "Object", "[object Object]", "key0", "string", "value0", "value0",
                           "key1", "number", "100", "100" } },
            { "object9", { "object", "Object", "Object", "[object Object]", "none" } },
            { "object10", { "object", "array", "Array", "Array(2)", "Apple,Banana", "0", "string", "Apple", "Apple",
                            "1", "string", "Banana", "Banana", "length", "number", "2", "2" } },
            { "object11", { "object", "Object", "BigInt", "8998192055486250009", "none" } },
            { "object12", { "function", "Generator", "function* generator0( { [js code] }",
                            "Cannot get source code of function" } },
            { "object13", { "object", "regexp", "RegExp", "/^\\d+\\.\\d+$/i", "/^\\d+\\.\\d+$/i", "global", "boolean",
                            "false", "false", "ignoreCase", "boolean", "true", "true", "multiline", "boolean", "false",
                            "false", "dotAll", "boolean", "false", "false", "hasIndices", "boolean", "false", "false",
                            "unicode", "boolean", "false", "false", "sticky", "boolean", "false", "false", "flags",
                            "string", "i", "i", "source", "string", "^\\d+\\.\\d+$", "^\\d+\\.\\d+$", "lastIndex",
                            "number", "0", "0" } },
            { "object14", { "object", "Object", "BigInt", "999", "none" } },
            { "object15", { "object", "arraybuffer", "Arraybuffer", "Arraybuffer(24)", "[object ArrayBuffer]",
                            "[[Int8Array]]", "object", "Object", "Int8Array(24)",
                            "0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0", "[[Uint8Array]]", "object", "Object",
                            "Uint8Array(24)", "0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0",
                            "[[Uint8ClampedArray]]", "object", "Object", "Uint8ClampedArray",
                            "0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0", "[[Int16Array]]", "object", "Object",
                            "Int16Array(12)", "0,0,0,0,0,0,0,0,0,0,0,0", "[[Uint16Array]]", "object", "Object",
                            "Uint16Array", "0,0,0,0,0,0,0,0,0,0,0,0", "[[Int32Array]]", "object", "Object",
                            "Int32Array(6)", "0,0,0,0,0,0", "[[Uint32Array]]", "object", "Object", "Uint32Array",
                            "0,0,0,0,0,0", "[[Float32Array]]", "object", "Object", "Float32Array", "0,0,0,0,0,0",
                            "[[Float64Array]]", "object", "Object",
                            "Float64Array", "0,0,0", "[[BigInt64Array]]", "object",
                            "Object", "BigInt64Array", "0,0,0", "[[BigUint64Array]]",
                            "object", "Object", "BigUint64Array", "0,0,0" } },
            { "object16", { "object", "Object", "Uint8Array(24)", "0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0",
                            "0", "number", "0", "0", "1", "number", "0", "0", "2", "number", "0", "0", "3", "number",
                            "0", "0", "4", "number", "0", "0", "5", "number", "0", "0", "6", "number", "0", "0", "7",
                            "number", "0", "0", "8", "number", "0", "0", "9", "number", "0", "0", "10", "number", "0",
                            "0", "11", "number", "0", "0", "12", "number", "0", "0", "13", "number", "0", "0", "14",
                            "number", "0", "0", "15", "number", "0", "0", "16", "number", "0", "0", "17", "number",
                            "0", "0", "18", "number", "0", "0", "19", "number", "0", "0", "20", "number", "0", "0",
                            "21", "number", "0", "0", "22", "number", "0", "0", "23", "number", "0", "0" } },
            { "object17", { "object", "dataview", "Dataview", "DataView(24)", "[object DataView]", "buffer",
                            "object", "arraybuffer", "Arraybuffer", "Arraybuffer(24)", "[object ArrayBuffer]",
                            "byteLength", "number", "24", "24", "byteOffset", "number", "0", "0" } },
            { "object18", { "object", "Object", "Boolean{[[PrimitiveValue]]: false}", "false", "[[PrimitiveValue]]",
                            "boolean", "false", "false" } },
            { "object19", { "function", "Function", "function function0( { [js code] }",
                            "Cannot get source code of function" } },
            { "regExp1", { "object", "regexp", "RegExp", "/^a/g", "/^a/g", "global", "boolean", "true", "true",
                           "ignoreCase", "boolean", "false", "false", "multiline", "boolean", "false", "false",
                           "dotAll", "boolean", "false", "false", "hasIndices", "boolean", "false", "false", "unicode",
                           "boolean", "false", "false", "sticky", "boolean", "false", "false", "flags", "string", "g",
                           "g", "source", "string", "^a", "^a", "lastIndex", "number", "0", "0" } },
            { "regExp2", { "object", "regexp", "RegExp", "/^ab+c/g", "/^ab+c/g", "global", "boolean", "true", "true",
                           "ignoreCase", "boolean", "false", "false", "multiline", "boolean", "false", "false",
                           "dotAll", "boolean", "false", "false", "hasIndices", "boolean", "false", "false", "unicode",
                           "boolean", "false", "false", "sticky", "boolean", "false", "false", "flags", "string", "g",
                           "g", "source", "string", "^ab+c", "^ab+c", "lastIndex", "number", "0", "0" } },
            { "regExp3", { "object", "regexp", "RegExp", "/123$/", "/123$/", "global", "boolean", "false", "false",
                           "ignoreCase", "boolean", "false", "false", "multiline", "boolean", "false", "false",
                           "dotAll", "boolean", "false", "false", "hasIndices", "boolean", "false", "false", "unicode",
                           "boolean", "false", "false", "sticky", "boolean", "false", "false", "flags", "string", "",
                           "", "source", "string", "123$", "123$", "lastIndex", "number", "0", "0" } },
            { "regExp4", { "object", "regexp", "RegExp", "/\\d/i", "/\\d/i", "global", "boolean", "false", "false",
                           "ignoreCase", "boolean", "true", "true", "multiline", "boolean", "false", "false", "dotAll",
                           "boolean", "false", "false", "hasIndices", "boolean", "false", "false", "unicode",
                           "boolean", "false", "false", "sticky", "boolean", "false", "false", "flags", "string", "i",
                           "i", "source", "string", "\\d", "\\d", "lastIndex", "number", "0", "0" } },
            { "regExp5", { "object", "regexp", "RegExp", "/^[a-zA-Z]/w{5,17}$/iu", "/^[a-zA-Z]\\/w{5,17}$/iu",
                           "global", "boolean", "false", "false", "ignoreCase", "boolean", "true", "true", "multiline",
                           "boolean", "false", "false", "dotAll", "boolean", "false", "false", "hasIndices", "boolean",
                           "false", "false", "unicode", "boolean", "true", "true", "sticky", "boolean", "false",
                           "false", "flags", "string", "iu", "iu", "source", "string",
                           "^[a-zA-Z]/w{5,17}$", "^[a-zA-Z]/w{5,17}$", "lastIndex", "number", "0", "0" } },
            { "regExp6", { "object", "regexp", "RegExp", "/[A-Z]/m", "/[A-Z]/m", "global", "boolean", "false", "false",
                           "ignoreCase", "boolean", "false", "false", "multiline", "boolean", "true", "true", "dotAll",
                           "boolean", "false", "false", "hasIndices", "boolean", "false", "false", "unicode",
                           "boolean", "false", "false", "sticky", "boolean", "false", "false", "flags", "string", "m",
                           "m", "source", "string", "[A-Z]", "[A-Z]", "lastIndex", "number", "0", "0" } },
            { "regExp7", { "object", "regexp", "RegExp", "/(/d{3}-|/d{4}-)?(/d{8}|/d{7})?/gm",
                           "/(\\/d{3}-|\\/d{4}-)?(\\/d{8}|\\/d{7})?/gm", "global", "boolean", "true", "true",
                           "ignoreCase", "boolean", "false", "false", "multiline", "boolean", "true", "true", "dotAll",
                           "boolean", "false", "false", "hasIndices", "boolean", "false", "false", "unicode",
                           "boolean", "false", "false", "sticky", "boolean", "false", "false", "flags", "string", "gm",
                           "gm", "source", "string", "(/d{3}-|/d{4}-)?(/d{8}|/d{7})?",
                           "(/d{3}-|/d{4}-)?(/d{8}|/d{7})?", "lastIndex", "number", "0", "0" } },
            { "regExp8", { "object", "regexp", "RegExp", "/[a-z]/y", "/[a-z]/y", "global", "boolean", "false", "false",
                           "ignoreCase", "boolean", "false", "false", "multiline", "boolean", "false", "false",
                           "dotAll", "boolean", "false", "false", "hasIndices", "boolean", "false", "false", "unicode",
                           "boolean", "false", "false", "sticky", "boolean", "true", "true", "flags", "string", "y",
                           "y", "source", "string", "[a-z]", "[a-z]", "lastIndex", "number", "0", "0" } },
            { "regExp9", { "object", "regexp", "RegExp", "/\\s/u", "/\\s/u", "global", "boolean", "false", "false",
                           "ignoreCase", "boolean", "false", "false", "multiline", "boolean", "false", "false",
                           "dotAll", "boolean", "false", "false", "hasIndices", "boolean", "false", "false",
                           "unicode", "boolean", "true", "true", "sticky", "boolean", "false", "false", "flags",
                           "string", "u", "u", "source", "string", "\\s", "\\s", "lastIndex", "number", "0", "0" } },
            { "regExp10", { "object", "regexp", "RegExp", "/a+/s", "/a+/s", "global", "boolean", "false", "false",
                            "ignoreCase", "boolean", "false", "false", "multiline", "boolean", "false", "false",
                            "dotAll", "boolean", "true", "true", "hasIndices", "boolean", "true", "true", "unicode",
                            "boolean", "false", "false", "sticky", "boolean", "false", "false", "flags", "string",
                            "s", "s", "source", "string", "a+", "a+", "lastIndex", "number", "0", "0" } },
            { "regExp11", { "object", "regexp", "RegExp", "/(d+-)?(d{4}-?d{7}|d{3}-?d{8}|^d{7,8})(-d+)?/s",
                            "/(d+-)?(d{4}-?d{7}|d{3}-?d{8}|^d{7,8})(-d+)?/s", "global", "boolean", "false", "false",
                            "ignoreCase", "boolean", "false", "false", "multiline", "boolean", "false", "false",
                            "dotAll", "boolean", "true", "true", "hasIndices", "boolean", "true", "true", "unicode",
                            "boolean", "false", "false", "sticky", "boolean", "false", "false", "flags", "string",
                            "s", "s", "source", "string", "(d+-)?(d{4}-?d{7}|d{3}-?d{8}|^d{7,8})(-d+)?",
                            "(d+-)?(d{4}-?d{7}|d{3}-?d{8}|^d{7,8})(-d+)?", "lastIndex", "number", "0", "0" } },
            { "regExp12", { "object", "regexp", "RegExp", "/a?/gy", "/a?/gy", "global", "boolean", "true", "true",
                            "ignoreCase", "boolean", "false", "false", "multiline", "boolean", "false", "false",
                            "dotAll", "boolean", "false", "false", "hasIndices", "boolean", "false", "false",
                            "unicode", "boolean", "false", "false", "sticky", "boolean", "true", "true", "flags",
                            "string", "gy", "gy", "source", "string", "a?", "a?", "lastIndex", "number", "0", "0" } },
            { "regExp13", { "object", "regexp", "RegExp",
                            "//^((0([1-9]{1}))|(1[1|2]))/(([0-2]([1-9]{1}))|(3[0|1]))/(d{2}|d{4})$//",
                            "/\\/^((0([1-9]{1}))|(1[1|2]))\\/(([0-2]([1-9]{1}))|(3[0|1]))\\/(d{2}|d{4})$\\//",
                            "global", "boolean", "false", "false", "ignoreCase", "boolean", "false", "false",
                            "multiline", "boolean", "false", "false", "dotAll", "boolean", "false", "false",
                            "hasIndices", "boolean", "false", "false", "unicode", "boolean", "false", "false",
                            "sticky", "boolean", "false", "false", "flags", "string", "", "", "source", "string",
                            "/^((0([1-9]{1}))|(1[1|2]))/(([0-2]([1-9]{1}))|(3[0|1]))/(d{2}|d{4})$/",
                            "/^((0([1-9]{1}))|(1[1|2]))/(([0-2]([1-9]{1}))|(3[0|1]))/(d{2}|d{4})$/", "lastIndex",
                            "number", "0", "0" } },
            { "regExp14", { "object", "regexp", "RegExp", "/a*/gimy", "/a*/gimy", "global", "boolean", "true", "true",
                            "ignoreCase", "boolean", "true", "true", "multiline", "boolean", "true", "true", "dotAll",
                            "boolean", "false", "false", "hasIndices", "boolean", "false", "false", "unicode",
                            "boolean", "false", "false", "sticky", "boolean", "true", "true", "flags", "string",
                            "gimy", "gimy", "source", "string", "a*", "a*", "lastIndex", "number", "0", "0" } },
            { "regExp15", { "object", "regexp", "RegExp", "/^[/w-]+(/.[/w-]+)*@[/w-]+(/.[/w-]+)+$/gi",
                            "/^[\\/w-]+(\\/.[\\/w-]+)*@[\\/w-]+(\\/.[\\/w-]+)+$/gi", "global", "boolean", "true",
                            "true", "ignoreCase", "boolean", "true", "true", "multiline", "boolean", "false",
                            "false", "dotAll", "boolean", "false", "false", "hasIndices", "boolean", "false", "false",
                            "unicode", "boolean", "false", "false", "sticky", "boolean", "false", "false", "flags",
                            "string", "gi", "gi", "source", "string", "^[/w-]+(/.[/w-]+)*@[/w-]+(/.[/w-]+)+$",
                            "^[/w-]+(/.[/w-]+)*@[/w-]+(/.[/w-]+)+$", "lastIndex", "number", "0", "0" } },
            { "regExp16", { "object", "regexp", "RegExp", "/a|b/gimsy", "/a|b/gimsy", "global", "boolean", "true",
                            "true", "ignoreCase", "boolean", "true", "true", "multiline", "boolean", "true", "true",
                            "dotAll", "boolean", "true", "true", "hasIndices", "boolean", "true", "true", "unicode",
                            "boolean", "false", "false", "sticky", "boolean", "true", "true", "flags", "string",
                            "gimsy", "gimsy", "source", "string", "a|b", "a|b", "lastIndex", "number", "0", "0" } },
            { "regExp17", { "object", "regexp", "RegExp",
                            "/^((0([1-9]{1}))|(1[1|2]))/(([0-2]([1-9]{1}))|(3[0|1]))/(d{2}|d{4})$/",
                            "/^((0([1-9]{1}))|(1[1|2]))\\/(([0-2]([1-9]{1}))|(3[0|1]))\\/(d{2}|d{4})$/", "global",
                            "boolean", "false", "false", "ignoreCase", "boolean", "false", "false", "multiline",
                            "boolean", "false", "false", "dotAll", "boolean", "false", "false", "hasIndices",
                            "boolean", "false", "false", "unicode", "boolean", "false", "false", "sticky", "boolean",
                            "false", "false", "flags", "string", "", "", "source", "string",
                            "^((0([1-9]{1}))|(1[1|2]))/(([0-2]([1-9]{1}))|(3[0|1]))/(d{2}|d{4})$",
                            "^((0([1-9]{1}))|(1[1|2]))/(([0-2]([1-9]{1}))|(3[0|1]))/(d{2}|d{4})$", "lastIndex",
                            "number", "0", "0" } },
            { "regExp18", { "object", "regexp", "RegExp", "/\\/<(.*)>.*<\\/\\/\\/1>|<(.*) \\/\\/>/i",
                            "/\\\\/<(.*)>.*<\\\\/\\\\/\\\\/1>|<(.*) \\\\/\\\\/>/i", "global", "boolean", "false",
                            "false", "ignoreCase", "boolean", "true", "true", "multiline", "boolean", "false",
                            "false", "dotAll", "boolean", "false", "false", "hasIndices", "boolean", "false", "false",
                            "unicode", "boolean", "false", "false", "sticky", "boolean", "false", "false", "flags",
                            "string", "i", "i", "source", "string",
                            "\\/<(.*)>.*<\\/\\/\\/1>|<(.*) \\/\\/>", "\\/<(.*)>.*<\\/\\/\\/1>|<(.*) \\/\\/>",
                            "lastIndex", "number", "0", "0" } },
            { "regExp19", { "object", "regexp", "RegExp", "/^[1-9]*[1-9][0-9]*$/m", "/^[1-9]*[1-9][0-9]*$/m",
                            "global", "boolean", "false", "false", "ignoreCase", "boolean", "false", "false",
                            "multiline", "boolean", "true", "true", "dotAll", "boolean", "false", "false",
                            "hasIndices", "boolean", "false", "false", "unicode", "boolean", "false", "false",
                            "sticky", "boolean", "false", "false", "flags", "string", "m", "m", "source",
                            "string", "^[1-9]*[1-9][0-9]*$", "^[1-9]*[1-9][0-9]*$", "lastIndex", "number",
                            "0", "0" } },
            { "regExp20", { "object", "regexp", "RegExp", "/^[a-zA-Z]\\/w{5,17}$/", "/^[a-zA-Z]\\\\/w{5,17}$/",
                            "global", "boolean", "false", "false", "ignoreCase", "boolean", "false", "false",
                            "multiline", "boolean", "false", "false", "dotAll", "boolean", "false", "false",
                            "hasIndices", "boolean", "false", "false", "unicode", "boolean", "false", "false",
                            "sticky", "boolean", "false", "false", "flags", "string", "", "", "source", "string",
                            "^[a-zA-Z]\\/w{5,17}$", "^[a-zA-Z]\\/w{5,17}$", "lastIndex", "number", "0", "0" } },
            { "regExp21", { "object", "regexp", "RegExp", "/^[0-9a-zA-Z_]{1,}$/u", "/^[0-9a-zA-Z_]{1,}$/u", "global",
                            "boolean", "false", "false", "ignoreCase", "boolean", "false", "false", "multiline",
                            "boolean", "false", "false", "dotAll", "boolean", "false", "false", "hasIndices",
                            "boolean", "false", "false", "unicode", "boolean", "true", "true", "sticky", "boolean",
                            "false", "false", "flags", "string", "u", "u", "source", "string", "^[0-9a-zA-Z_]{1,}$",
                            "^[0-9a-zA-Z_]{1,}$", "lastIndex", "number", "0", "0" } },
            { "proxy", { "object", "proxy", "Object", "Proxy", "[object Object]",
                         "[[Target]]", "object", "Object", "Object", "[object Object]",
                         "[[Handler]]", "object", "Object", "Object", "[object Object]",
                         "[[IsRevoked]]", "boolean", "false", "false" } },
            { "proxy1", { "object", "proxy", "Object", "Proxy", "[object Object]",
                          "[[Target]]", "object", "Object", "Object", "[object Object]",
                          "[[Handler]]", "object", "Object", "Object", "[object Object]",
                          "[[IsRevoked]]", "boolean", "false", "false" } },
            { "proxy2", { "object", "Object", "Object", "[object Object]",
                          "proxy", "object", "proxy", "Object", "Proxy", "[object Object]",
                          "revoke", "function", "Function", "function ( { [native code] }",
                          "function () { [native code] }" } },
            { "revoke", { "function", "Function", "function ( { [native code] }", "function () { [native code] }" } },
            { "target", { "object", "Object", "Object", "[object Object]",
                          "name", "string", "openharmony", "openharmony",
                          "age", "number", "3", "3" } },
            { "handler", { "object", "Object", "Object", "[object Object]",
                           "get", "function", "Function", "function get( { [js code] }",
                           "Cannot get source code of function",
                           "set", "function", "Function", "function set( { [js code] }",
                           "Cannot get source code of function" } },
            { "resolveHandler", { "function", "Function", "function resolveHandler( { [js code] }",
                                  "Cannot get source code of function"} },
            { "result", { "object", "Object", "Object", "[object Object]", "flag", "boolean",
                          "true", "true"} },
            { "a", { "object", "promise", "Promise", "Promise", "[object Promise]",
                     "[[PromiseState]]", "string", "Fulfilled", "Fulfilled", "[[PromiseResult]]",
                     "object", "Object", "Object", "[object Object]"} },
        };

        int32_t index_ {0};
        const EcmaVM *vm_ {nullptr};
        RuntimeImpl *runtime_ {nullptr};
    };

    std::string entryPoint_ = "variable_first";
    JSPtLocation location_ {nullptr, JSPtLocation::EntityId(0), 0};
    size_t breakpointCounter_ = 0;
};

std::unique_ptr<TestEvents> GetJsVariableFirstTest()
{
    return std::make_unique<JsVariableFirstTest>();
}
}  // namespace panda::ecmascript::tooling::test

#endif  // ECMASCRIPT_TOOLING_TEST_UTILS_TESTCASES_JS_VARIABLE_TEST_H
