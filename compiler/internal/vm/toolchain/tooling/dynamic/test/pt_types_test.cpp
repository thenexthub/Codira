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

#include "tooling/dynamic/base/pt_types.h"
#include "common_components/base/utf_helper.h"
#include "ecmascript/js_regexp.h"
#include "ecmascript/object_factory.h"

#include "ecmascript/tests/test_helper.h"
#include "protocol_handler.h"

using namespace panda::ecmascript;
using namespace panda::ecmascript::tooling;

namespace panda::test {
class PtTypesTest : public testing::Test {
public:
    static void SetUpTestCase()
    {
        GTEST_LOG_(INFO) << "SetUpTestCase";
    }

    static void TearDownTestCase()
    {
        GTEST_LOG_(INFO) << "TearDownCase";
    }

    void SetUp() override
    {
        TestHelper::CreateEcmaVMWithScope(ecmaVm, thread, scope);
    }

    void TearDown() override
    {
        TestHelper::DestroyEcmaVMWithScope(ecmaVm, scope);
    }

protected:
    EcmaVM *ecmaVm {nullptr};
    EcmaHandleScope *scope {nullptr};
    JSThread *thread {nullptr};
};

HWTEST_F_L0(PtTypesTest, BreakpointDetailsToString)
{
    BreakpointDetails input;
    BreakpointId result = BreakpointDetails::ToString(input);
    ASSERT_TRUE(result == "id:0:0:");
}

HWTEST_F_L0(PtTypesTest, BreakpointDetailsParseBreakpointId)
{
    BreakpointId input = "";
    BreakpointDetails detail;
    bool result = BreakpointDetails::ParseBreakpointId(input, &detail);
    ASSERT_TRUE(!result);
    input = "id:0";
    result = BreakpointDetails::ParseBreakpointId(input, &detail);
    ASSERT_TRUE(!result);
    input = "id:0:0";
    result = BreakpointDetails::ParseBreakpointId(input, &detail);
    ASSERT_TRUE(!result);
    input = "id:0:0:url";
    result = BreakpointDetails::ParseBreakpointId(input, &detail);
    ASSERT_TRUE(result);
}

HWTEST_F_L0(PtTypesTest, TypeNameValid)
{
    std::string type = "object";
    bool result = RemoteObject::TypeName::Valid(type);
    ASSERT_TRUE(result);
    type = "function";
    result = RemoteObject::TypeName::Valid(type);
    ASSERT_TRUE(result);
    type = "undefined";
    result = RemoteObject::TypeName::Valid(type);
    ASSERT_TRUE(result);
    type = "string";
    result = RemoteObject::TypeName::Valid(type);
    ASSERT_TRUE(result);
    type = "number";
    result = RemoteObject::TypeName::Valid(type);
    ASSERT_TRUE(result);
    type = "boolean";
    result = RemoteObject::TypeName::Valid(type);
    ASSERT_TRUE(result);
    type = "symbol";
    result = RemoteObject::TypeName::Valid(type);
    ASSERT_TRUE(result);
    type = "bigint";
    result = RemoteObject::TypeName::Valid(type);
    ASSERT_TRUE(result);
    type = "wasm";
    result = RemoteObject::TypeName::Valid(type);
    ASSERT_TRUE(result);
    type = "int";
    result = RemoteObject::TypeName::Valid(type);
    ASSERT_TRUE(!result);
}

HWTEST_F_L0(PtTypesTest, SubTypeNameValid)
{
    std::string type = "array";
    bool result = RemoteObject::SubTypeName::Valid(type);
    ASSERT_TRUE(result);
    type = "null";
    result = RemoteObject::SubTypeName::Valid(type);
    ASSERT_TRUE(result);
    type = "node";
    result = RemoteObject::SubTypeName::Valid(type);
    ASSERT_TRUE(result);
    type = "regexp";
    result = RemoteObject::SubTypeName::Valid(type);
    ASSERT_TRUE(result);
    type = "map";
    result = RemoteObject::SubTypeName::Valid(type);
    ASSERT_TRUE(result);
    type = "set";
    result = RemoteObject::SubTypeName::Valid(type);
    ASSERT_TRUE(result);
    type = "weakmap";
    result = RemoteObject::SubTypeName::Valid(type);
    ASSERT_TRUE(result);
    type = "iterator";
    result = RemoteObject::SubTypeName::Valid(type);
    ASSERT_TRUE(result);
    type = "generator";
    result = RemoteObject::SubTypeName::Valid(type);
    ASSERT_TRUE(result);
    type = "error";
    result = RemoteObject::SubTypeName::Valid(type);
    ASSERT_TRUE(result);
    type = "proxy";
    result = RemoteObject::SubTypeName::Valid(type);
    ASSERT_TRUE(result);
    type = "promise";
    result = RemoteObject::SubTypeName::Valid(type);
    ASSERT_TRUE(result);
    type = "typedarray";
    result = RemoteObject::SubTypeName::Valid(type);
    ASSERT_TRUE(result);
    type = "arraybuffer";
    result = RemoteObject::SubTypeName::Valid(type);
    ASSERT_TRUE(result);
    type = "dataview";
    result = RemoteObject::SubTypeName::Valid(type);
    ASSERT_TRUE(result);
    type = "i32";
    result = RemoteObject::SubTypeName::Valid(type);
    ASSERT_TRUE(result);
    type = "i64";
    result = RemoteObject::SubTypeName::Valid(type);
    ASSERT_TRUE(result);
    type = "f32";
    result = RemoteObject::SubTypeName::Valid(type);
    ASSERT_TRUE(result);
    type = "f64";
    result = RemoteObject::SubTypeName::Valid(type);
    ASSERT_TRUE(result);
    type = "v128";
    result = RemoteObject::SubTypeName::Valid(type);
    ASSERT_TRUE(result);
    type = "externref";
    result = RemoteObject::SubTypeName::Valid(type);
    ASSERT_TRUE(result);
    type = "int";
    result = RemoteObject::SubTypeName::Valid(type);
    ASSERT_TRUE(!result);
}

HWTEST_F_L0(PtTypesTest, ExceptionDetailsGetException)
{
    ExceptionDetails exception;
    RemoteObject* result = exception.GetException();
    ASSERT_TRUE(result == nullptr);
}

HWTEST_F_L0(PtTypesTest, InternalPropertyDescriptorGetValue)
{
    InternalPropertyDescriptor descriptor;
    RemoteObject* result = descriptor.GetValue();
    ASSERT_TRUE(result == nullptr);
}

HWTEST_F_L0(PtTypesTest, PrivatePropertyDescriptor)
{
    PrivatePropertyDescriptor descriptor;
    ASSERT_TRUE(descriptor.GetName() == "");
}

HWTEST_F_L0(PtTypesTest, PropertyDescriptorGetValue)
{
    panda::ecmascript::tooling::PropertyDescriptor descriptor;
    RemoteObject* result = descriptor.GetValue();
    ASSERT_TRUE(result == nullptr);
    result = descriptor.GetGet();
    ASSERT_TRUE(result == nullptr);
    result = descriptor.GetSet();
    ASSERT_TRUE(result == nullptr);
    result = descriptor.GetSymbol();
    ASSERT_TRUE(result == nullptr);
    bool res = descriptor.HasSymbol();
    ASSERT_TRUE(!res);
}

HWTEST_F_L0(PtTypesTest, BreakLocationTypeValid)
{
    BreakLocation::Type type;
    bool result = type.Valid("debuggerStatement");
    ASSERT_TRUE(result);
    result = type.Valid("call");
    ASSERT_TRUE(result);
    result = type.Valid("return");
    ASSERT_TRUE(result);
    result = type.Valid("test");
    ASSERT_TRUE(!result);
}

HWTEST_F_L0(PtTypesTest, ScopeTypeValid)
{
    Scope::Type type;
    bool result = type.Valid("global");
    ASSERT_TRUE(result);
    result = type.Valid("local");
    ASSERT_TRUE(result);
    result = type.Valid("with");
    ASSERT_TRUE(result);
    result = type.Valid("closure");
    ASSERT_TRUE(result);
    result = type.Valid("catch");
    ASSERT_TRUE(result);
    result = type.Valid("block");
    ASSERT_TRUE(result);
    result = type.Valid("script");
    ASSERT_TRUE(result);
    result = type.Valid("eval");
    ASSERT_TRUE(result);
    result = type.Valid("module");
    ASSERT_TRUE(result);
    result = type.Valid("wasm-expression-stack");
    ASSERT_TRUE(result);
    result = type.Valid("test");
    ASSERT_TRUE(!result);
}

HWTEST_F_L0(PtTypesTest, CallFrameGetFunctionLocation)
{
    CallFrame callFrame;
    Location *location = callFrame.GetFunctionLocation();
    ASSERT_TRUE(location == nullptr);
    RemoteObject *result = callFrame.GetReturnValue();
    ASSERT_TRUE(result == nullptr);
}

HWTEST_F_L0(PtTypesTest, MemoryDumpLevelOfDetailValuesValid)
{
    bool result = MemoryDumpLevelOfDetailValues::Valid("background");
    ASSERT_TRUE(result);
    result = MemoryDumpLevelOfDetailValues::Valid("light");
    ASSERT_TRUE(result);
    result = MemoryDumpLevelOfDetailValues::Valid("detailed");
    ASSERT_TRUE(result);
    result = MemoryDumpLevelOfDetailValues::Valid("test");
    ASSERT_TRUE(!result);
}

HWTEST_F_L0(PtTypesTest, TraceConfigRecordModeValuesValid)
{
    bool result = TraceConfig::RecordModeValues::Valid("recordUntilFull");
    ASSERT_TRUE(result);
    result = TraceConfig::RecordModeValues::Valid("recordContinuously");
    ASSERT_TRUE(result);
    result = TraceConfig::RecordModeValues::Valid("recordAsMuchAsPossible");
    ASSERT_TRUE(result);
    result = TraceConfig::RecordModeValues::Valid("echoToConsole");
    ASSERT_TRUE(result);
    result = TraceConfig::RecordModeValues::Valid("test");
    ASSERT_TRUE(!result);
}

HWTEST_F_L0(PtTypesTest, TracingBackendValues)
{
    bool result = TracingBackendValues::Valid("auto");
    ASSERT_TRUE(result);
    result = TracingBackendValues::Valid("chrome");
    ASSERT_TRUE(result);
    result = TracingBackendValues::Valid("system");
    ASSERT_TRUE(result);
    result = TracingBackendValues::Valid("test");
    ASSERT_TRUE(!result);
}

HWTEST_F_L0(PtTypesTest, TraceConfigToJsonTest)
{
    TraceConfig treceConfig;
    std::vector<std::string> vct {"one", "two"};
    treceConfig.SetIncludedCategories(vct);
    treceConfig.SetExcludedCategories(vct);
    treceConfig.SetSyntheticDelays(vct);
    std::unique_ptr<PtJson> result = treceConfig.ToJson();
    ASSERT_TRUE(result);
}

HWTEST_F_L0(PtTypesTest, TraceConfigCreateTest)
{
    std::string attribute = "test";
    std::unique_ptr<PtJson> ptJson = PtJson::CreateObject();
    ptJson->Add("recordMode", 0);
    ptJson->Add("enableSampling", attribute.c_str());
    ptJson->Add("enableSystrace", attribute.c_str());
    ptJson->Add("enableArgumentFilter", attribute.c_str());
    ptJson->Add("includedCategories", attribute.c_str());
    ptJson->Add("excludedCategories", attribute.c_str());
    ptJson->Add("syntheticDelays", attribute.c_str());
    ptJson->Add("memoryDumpConfig", attribute.c_str());
    std::unique_ptr<TraceConfig> traceConfig = TraceConfig::Create(*ptJson);
    ASSERT_TRUE(!traceConfig);

    std::unique_ptr<PtJson> ptJson1 = PtJson::CreateObject();
    ptJson1->Add("recordMode", "test");
    std::unique_ptr<TraceConfig> traceConfig1 = TraceConfig::Create(*ptJson1);
    ASSERT_TRUE(!traceConfig1);

    std::unique_ptr<PtJson> ptJson2 = PtJson::CreateObject();
    std::unique_ptr<TraceConfig> traceConfig2 = TraceConfig::Create(*ptJson2);
    ASSERT_TRUE(traceConfig2);

    std::unique_ptr<PtJson> includedCategoriesArray = PtJson::CreateArray();
    includedCategoriesArray->Push("includedCategory");
    std::unique_ptr<PtJson> excludedCategoriesArray = PtJson::CreateArray();
    excludedCategoriesArray->Push("excludedCategory1");
    excludedCategoriesArray->Push("excludedCategory2");
    std::unique_ptr<PtJson> syntheticDelaysArray = PtJson::CreateArray();
    syntheticDelaysArray->Push("syntheticDelay1");
    syntheticDelaysArray->Push("syntheticDelay2");
    syntheticDelaysArray->Push("syntheticDelay3");
    std::unique_ptr<PtJson> ptJson3 = PtJson::CreateObject();
    ptJson3->Add("recordMode", "recordUntilFull");
    ptJson3->Add("enableSampling", true);
    ptJson3->Add("enableSystrace", true);
    ptJson3->Add("enableArgumentFilter", true);
    ptJson3->Add("includedCategories", includedCategoriesArray);
    ptJson3->Add("excludedCategories", excludedCategoriesArray);
    ptJson3->Add("syntheticDelays", syntheticDelaysArray);
    ptJson3->Add("memoryDumpConfig", PtJson::CreateObject());
    std::unique_ptr<TraceConfig> traceConfig3 = TraceConfig::Create(*ptJson3);
    ASSERT_TRUE(traceConfig3);
    ASSERT_TRUE(traceConfig3->GetEnableSampling());
    ASSERT_TRUE(traceConfig3->GetEnableSystrace());
    ASSERT_TRUE(traceConfig3->GetEnableArgumentFilter());
    ASSERT_EQ(traceConfig3->GetIncludedCategories()->size(), 1);
    ASSERT_EQ(traceConfig3->GetExcludedCategories()->size(), 2);
    ASSERT_EQ(traceConfig3->GetSyntheticDelays()->size(), 3);
}

HWTEST_F_L0(PtTypesTest, ScriptTypeProfileCreateTest)
{
    std::unique_ptr<PtJson> ptJson = PtJson::CreateObject();
    std::unique_ptr<PtJson> array = PtJson::CreateArray();
    std::unique_ptr<PtJson> object = PtJson::CreateObject();
    object->Add("offset", "test");
    array->Push(object);
    ptJson->Add("entries", array);
    std::unique_ptr<ScriptTypeProfile> profile = ScriptTypeProfile::Create(*ptJson);
    ASSERT_TRUE(!profile);
}

HWTEST_F_L0(PtTypesTest, TypeProfileEntryCreateTest)
{
    std::unique_ptr<PtJson> ptJson = PtJson::CreateObject();
    std::unique_ptr<PtJson> array = PtJson::CreateArray();
    std::unique_ptr<PtJson> object = PtJson::CreateObject();
    object->Add("name", 0);
    array->Push(object);
    ptJson->Add("types", array);
    std::unique_ptr<TypeProfileEntry> entry = TypeProfileEntry::Create(*ptJson);
    ASSERT_TRUE(!entry);
}

HWTEST_F_L0(PtTypesTest, ScriptCoverageCreateTest)
{
    std::unique_ptr<PtJson> ptJson = PtJson::CreateObject();
    std::unique_ptr<PtJson> array = PtJson::CreateArray();
    std::unique_ptr<PtJson> object = PtJson::CreateObject();
    object->Add("functionName", 0);
    array->Push(object);
    ptJson->Add("functions", array);
    std::unique_ptr<ScriptCoverage> coverage = ScriptCoverage::Create(*ptJson);
    ASSERT_TRUE(!coverage);
}

HWTEST_F_L0(PtTypesTest, FunctionCoverageCreateTest)
{
    std::unique_ptr<PtJson> ptJson = PtJson::CreateObject();
    std::unique_ptr<PtJson> array = PtJson::CreateArray();
    std::unique_ptr<PtJson> object = PtJson::CreateObject();
    object->Add("functionName", 0);
    array->Push(object);
    ptJson->Add("ranges", array);
    std::unique_ptr<FunctionCoverage> coverage = FunctionCoverage::Create(*ptJson);
    ASSERT_TRUE(!coverage);
}

HWTEST_F_L0(PtTypesTest, SymbolRemoteObjectTest)
{
    size_t size = 10;
    const char *data = "SymbolRemoteObjectTest";
    SymbolRemoteObject obj(ecmaVm, SymbolRef::New(ecmaVm, StringRef::NewFromUtf8(ecmaVm, (const char*)data, size)));
}

HWTEST_F_L0(PtTypesTest, DescriptionForObjectForDate)
{
    double input = 123456789.0;
    Local<DateRef> date = DateRef::New(ecmaVm, input);
    std::string arrayOrContainer;
    std::string description = ObjectRemoteObject::DescriptionForObject(ecmaVm, date, arrayOrContainer);
    ASSERT_TRUE(arrayOrContainer.empty());
    ASSERT_TRUE(description.find("GMT") != std::string::npos);
}

HWTEST_F_L0(PtTypesTest, DescriptionForObjectForPromise)
{
    Local<PromiseCapabilityRef> capability = PromiseCapabilityRef::New(ecmaVm);
    Local<PromiseRef> promise = capability->GetPromise(ecmaVm);
    std::string arrayOrContainer;
    std::string description = ObjectRemoteObject::DescriptionForObject(ecmaVm, promise, arrayOrContainer);
    ASSERT_TRUE(arrayOrContainer.empty());
    ASSERT_TRUE(description == "Promise");
}

HWTEST_F_L0(PtTypesTest, PrivatePropertyDescriptorCreate)
{
    std::unique_ptr<PtJson> ptJson = PtJson::CreateObject();
    std::unique_ptr<PtJson> ptJson1 = PtJson::CreateObject();
    bool value = true;
    ptJson->Add("userCode", value);
    std::unique_ptr<PrivatePropertyDescriptor> result = PrivatePropertyDescriptor::Create(*ptJson);
    ASSERT_TRUE(result == nullptr);
    value = false;
    ptJson->Add("userCode1", value);
    result = PrivatePropertyDescriptor::Create(*ptJson);
    ASSERT_TRUE(result == nullptr);
    int32_t value1 = 100;
    ptJson1->Add("test", value1);
    std::unique_ptr<PrivatePropertyDescriptor> result1 = PrivatePropertyDescriptor::Create(*ptJson1);
    ASSERT_TRUE(result == nullptr);
    int64_t value2 = 0;
    ptJson1->Add("", value2);
    result1 = PrivatePropertyDescriptor::Create(*ptJson1);
    ASSERT_TRUE(result == nullptr);
    std::unique_ptr<PtJson> ptJson2 = PtJson::CreateObject();
    ptJson2->Add("name", value);
    std::unique_ptr<PrivatePropertyDescriptor> result2 = PrivatePropertyDescriptor::Create(*ptJson2);
    ASSERT_TRUE(result == nullptr);
    ptJson2->Add("value", value1);
    result2 = PrivatePropertyDescriptor::Create(*ptJson2);
    ASSERT_TRUE(result == nullptr);
    ptJson2->Add("get", value2);
    result2 = PrivatePropertyDescriptor::Create(*ptJson2);
    ASSERT_TRUE(result == nullptr);
    value2 = 10;
    ptJson2->Add("set", value2);
    result2 = PrivatePropertyDescriptor::Create(*ptJson2);
    ASSERT_TRUE(result == nullptr);
    std::unique_ptr<PtJson> ptJson3 = PtJson::CreateObject();
    ptJson3->Add("name", "test");
    ptJson3->Add("value", ptJson);
    ptJson3->Add("get", ptJson1);
    ptJson3->Add("set", ptJson2);
    std::unique_ptr<PrivatePropertyDescriptor> result3 = PrivatePropertyDescriptor::Create(*ptJson3);
    ASSERT_TRUE(result == nullptr);
}

HWTEST_F_L0(PtTypesTest, CallArgumentToJson)
{
    CallArgument callArgument;
    Local<JSValueRef> name = StringRef::NewFromUtf8(ecmaVm, "name");
    callArgument.SetValue(name);
    RemoteObjectId objectId(10);
    callArgument.SetObjectId(objectId);
    UnserializableValue value("test");
    callArgument.SetUnserializableValue(value);
    std::unique_ptr<PtJson> ret = callArgument.ToJson();
    ASSERT_TRUE(ret != nullptr);
    CallArgument callArgument1;
    RemoteObjectId objectId1(20);
    callArgument1.SetObjectId(objectId1);
    ret = callArgument1.ToJson();
    ASSERT_TRUE(ret != nullptr);
    CallArgument callArgument2;
    UnserializableValue value1("CallArgumentToJson");
    callArgument2.ToJson();
    ASSERT_TRUE(ret != nullptr);
}

HWTEST_F_L0(PtTypesTest, ScriptPositionToJson)
{
    ScriptPosition position;
    int32_t line = 10;
    position.SetLine(line);
    int32_t column = 8;
    position.SetColumn(column);
    std::unique_ptr<PtJson> ret = position.ToJson();
    ASSERT_TRUE(ret != nullptr);
}

HWTEST_F_L0(PtTypesTest, SearchMatchCreate)
{
    std::unique_ptr<PtJson> ptJson = PtJson::CreateObject();
    int32_t lineNumber = 10;
    ptJson->Add("lineNumber", lineNumber);
    std::string lineContent = "12";
    ptJson->Add("lineContent", lineContent.c_str());
    std::unique_ptr<SearchMatch> ret = SearchMatch::Create(*ptJson);
    ASSERT_TRUE(ret != nullptr);

    std::unique_ptr<PtJson> ptJson1 = PtJson::CreateObject();
    std::string attribute = "32";
    ptJson1->Add("lineNumber", attribute.c_str());
    std::unique_ptr<SearchMatch> ret1 = SearchMatch::Create(*ptJson1);
    ASSERT_TRUE(ret1 == nullptr);

    std::unique_ptr<PtJson> ptJson2 = PtJson::CreateObject();
    int32_t lineNumber2 = 14;
    std::string lineContent2 = "12";
    ptJson1->Add("lineNumber1", lineNumber2);
    ptJson1->Add("lineContent2", lineContent2.c_str());
    std::unique_ptr<SearchMatch> ret2 = SearchMatch::Create(*ptJson1);
    ASSERT_TRUE(ret2 == nullptr);
}

HWTEST_F_L0(PtTypesTest, LocationRangeCreate)
{
    std::unique_ptr<PtJson> ptJson = PtJson::CreateObject();
    std::unique_ptr<PtJson> start = PtJson::CreateObject();
    std::unique_ptr<PtJson> end = PtJson::CreateObject();
    std::string scriptId = "12";
    int32_t lineNumber = 1;
    int32_t columnNumber = 2;
    start->Add("lineNumber", lineNumber);
    start->Add("columnNumber", columnNumber);
    end->Add("lineNumber", lineNumber + 1);
    end->Add("columnNumber", columnNumber + 2);
    ptJson->Add("scriptId", scriptId.c_str());
    ptJson->Add("start", start);
    ptJson->Add("end", end);
    std::unique_ptr<LocationRange> ret = LocationRange::Create(*ptJson);
    ASSERT_TRUE(ret != nullptr);
}

HWTEST_F_L0(PtTypesTest, LocationRangeCreateSwitch)
{
    std::unique_ptr<PtJson> ptJson = PtJson::CreateObject();
    std::unique_ptr<PtJson> start = PtJson::CreateObject();
    std::unique_ptr<PtJson> end = PtJson::CreateObject();
    std::string scriptId = "12";
    int32_t lineNumber = 1;
    int32_t columnNumber = 2;
    start->Add("lineNumber1", lineNumber);
    start->Add("columnNumber1", columnNumber);
    end->Add("lineNumber1", lineNumber + 1);
    end->Add("columnNumber1", columnNumber + 2);
    ptJson->Add("scriptId1", scriptId.c_str());
    ptJson->Add("start", start);
    ptJson->Add("end", end);
    std::unique_ptr<LocationRange> ret = LocationRange::Create(*ptJson);
    ASSERT_TRUE(ret == nullptr);
}

HWTEST_F_L0(PtTypesTest, LocationRangeCreateSwitchFail)
{
    std::unique_ptr<PtJson> ptJson = PtJson::CreateObject();
    std::unique_ptr<PtJson> start = PtJson::CreateObject();
    std::unique_ptr<PtJson> end = PtJson::CreateObject();
    std::string scriptId = "12";
    int32_t lineNumber = 1;
    int32_t columnNumber = 2;
    start->Add("lineNumber1", lineNumber);
    start->Add("columnNumber1", columnNumber);
    end->Add("lineNumber1", lineNumber + 1);
    end->Add("columnNumber1", columnNumber + 2);
    ptJson->Add("scriptId1", scriptId.c_str());
    ptJson->Add("start1", start);
    ptJson->Add("end1", end);
    std::unique_ptr<LocationRange> ret = LocationRange::Create(*ptJson);
    ASSERT_TRUE(ret == nullptr);
}

HWTEST_F_L0(PtTypesTest, RuntimeCallFrameCreate)
{
    std::unique_ptr<PtJson> ptJson = PtJson::CreateObject();
    std::string functionName = "RuntimeCallFrameCreate";
    std::string scriptId = "scriptId";
    std::string url = "url";
    int32_t lineNumber = 2;
    int32_t columnNumber = 10;
    ptJson->Add("functionName", functionName.c_str());
    ptJson->Add("moduleName", functionName.c_str());
    ptJson->Add("scriptId", scriptId.c_str());
    ptJson->Add("url", url.c_str());
    ptJson->Add("lineNumber", lineNumber);
    ptJson->Add("columnNumber", columnNumber);
    std::unique_ptr<RuntimeCallFrame> ret = RuntimeCallFrame::Create(*ptJson);
    ASSERT_TRUE(ret != nullptr);
}

HWTEST_F_L0(PtTypesTest, RuntimeCallFrameCreateFailSwitch)
{
    std::unique_ptr<PtJson> ptJson = PtJson::CreateObject();
    std::string functionName = "RuntimeCallFrameCreate";
    std::string scriptId = "scriptId";
    std::string url = "url";
    int32_t lineNumber = 2;
    int32_t columnNumber = 10;
    ptJson->Add("functionName1", functionName.c_str());
    ptJson->Add("moduleName1", functionName.c_str());
    ptJson->Add("scriptId1", scriptId.c_str());
    ptJson->Add("url1", url.c_str());
    ptJson->Add("lineNumber1", lineNumber);
    ptJson->Add("columnNumber1", columnNumber);
    std::unique_ptr<RuntimeCallFrame> ret = RuntimeCallFrame::Create(*ptJson);
    ASSERT_TRUE(ret == nullptr);
}

HWTEST_F_L0(PtTypesTest, SamplingHeapProfileNodeCreate)
{
    std::unique_ptr<PtJson> ptJson = PtJson::CreateObject();
    std::unique_ptr<PtJson> callFrame = PtJson::CreateObject();
    std::unique_ptr<PtJson> array = PtJson::CreateArray();
    std::unique_ptr<PtJson> object = PtJson::CreateObject();
    callFrame->Add("callFrame", "test");
    ptJson->Add("callFrame", callFrame);
    object->Add("test", 0);
    array->Push(object);
    ptJson->Add("children", array);
    std::unique_ptr<SamplingHeapProfileNode> ret = SamplingHeapProfileNode::Create(*ptJson);
    ASSERT_TRUE(ret == nullptr);
}

HWTEST_F_L0(PtTypesTest, SamplingHeapProfileCreate)
{
    std::unique_ptr<PtJson> ptJson = PtJson::CreateObject();
    std::unique_ptr<PtJson> head = PtJson::CreateObject();
    std::unique_ptr<PtJson> array = PtJson::CreateArray();
    std::unique_ptr<PtJson> object = PtJson::CreateObject();
    head->Add("head", "headTest");
    ptJson->Add("head", head);
    object->Add("test", 0);
    array->Push(object);
    ptJson->Add("samples", array);
    std::unique_ptr<SamplingHeapProfile> ret = SamplingHeapProfile::Create(*ptJson);
    ASSERT_TRUE(ret == nullptr);
}

HWTEST_F_L0(PtTypesTest, ExceptionDetailsCreate)
{
    std::unique_ptr<PtJson> ptJson = PtJson::CreateObject();
    std::unique_ptr<PtJson> exception = PtJson::CreateObject();
    exception->Add("exception", "test");
    ptJson->Add("exception", exception);
    std::unique_ptr<ExceptionDetails> ret = ExceptionDetails::Create(*ptJson);
    ASSERT_TRUE(ret == nullptr);
}


HWTEST_F_L0(PtTypesTest, ProfileToJsonTest)
{
    Profile profile;
    std::vector<int32_t> samples {1, 2, 3}; // 1:element 2:element 3:element
    std::vector<int32_t> timeDeltas {1, 2, 3}; // 1:element 2:element 3:element
    profile.SetSamples(samples);
    profile.SetTimeDeltas(timeDeltas);
    std::unique_ptr<PtJson> result = profile.ToJson();
    ASSERT_TRUE(result);
}

HWTEST_F_L0(PtTypesTest, ProfileFromProfileInfoTest)
{
    ProfileInfo info;
    info.samples.push_back(1);
    info.samples.push_back(2);
    info.timeDeltas.push_back(1);
    info.timeDeltas.push_back(2);
    std::unique_ptr<Profile> result = Profile::FromProfileInfo(info);
    ASSERT_TRUE(result);
}

HWTEST_F_L0(PtTypesTest, ProfileNodeToJsonTest)
{
    std::unique_ptr<PtJson> ptJson = PtJson::CreateObject();
    ProfileNode node;
    std::vector<int32_t> children {0, 1, 2, 3}; // 1: element 2:element, 3:element
    std::unique_ptr<PositionTickInfo> positionTickInfo = std::make_unique<PositionTickInfo>();
    std::unique_ptr<RuntimeCallFrame> callFrame = std::make_unique<RuntimeCallFrame>();
    positionTickInfo->SetLine(123);
    positionTickInfo->SetTicks(123);
    std::vector<std::unique_ptr<PositionTickInfo>> positionTicks;
    positionTicks.push_back(std::move(positionTickInfo));
    node.SetChildren(children);
    node.SetPositionTicks(std::move(positionTicks));
    node.SetCallFrame(std::move(callFrame));
    std::unique_ptr<PtJson> result = node.ToJson();
    ASSERT_TRUE(result);
    ProfileNode node1;
    std::unique_ptr<RuntimeCallFrame> callFrame1 = std::make_unique<RuntimeCallFrame>();
    node1.SetCallFrame(std::move(callFrame1));
    std::unique_ptr<PtJson> result1 = node1.ToJson();
    ASSERT_TRUE(result1);
}

HWTEST_F_L0(PtTypesTest, ProfileNodeFromCpuProfileNodeTest)
{
    CpuProfileNode cpuProfileNode;
    cpuProfileNode.children.push_back(0);
    cpuProfileNode.children.push_back(1); // 1: element
    std::unique_ptr<ProfileNode> result = ProfileNode::FromCpuProfileNode(cpuProfileNode);
    ASSERT_TRUE(result);
}

HWTEST_F_L0(PtTypesTest, PrivatePropertyDescriptorToJsonTest)
{
    PrivatePropertyDescriptor descriptor;
    std::unique_ptr<RemoteObject> value = std::make_unique<RemoteObject>();
    std::unique_ptr<RemoteObject> get = std::make_unique<RemoteObject>();
    std::unique_ptr<RemoteObject> set = std::make_unique<RemoteObject>();
    descriptor.SetValue(std::move(value));
    descriptor.SetGet(std::move(get));
    descriptor.SetSet(std::move(set));
    std::unique_ptr<PtJson> result = descriptor.ToJson();
    ASSERT_TRUE(result);
}

HWTEST_F_L0(PtTypesTest, PropertyDescriptorFromPropertyTest)
{
    Local<SymbolRef> symbol = SymbolRef::New(ecmaVm, StringRef::NewFromUtf8(ecmaVm, "test"));
    PropertyAttribute property;
    property.SetGetter(StringRef::NewFromUtf8(ecmaVm, "test"));
    property.SetSetter(StringRef::NewFromUtf8(ecmaVm, "test"));
    std::unique_ptr<panda::ecmascript::tooling::PropertyDescriptor> result =
        panda::ecmascript::tooling::PropertyDescriptor::FromProperty(ecmaVm, symbol, property);
    ASSERT_TRUE(result);
}

HWTEST_F_L0(PtTypesTest, AnonymousFunctionPropertyNameTest)
{
    Local<FunctionRef> func = FunctionRef::New(ecmaVm, nullptr);
    Local<JSValueRef> funcName = JSValueRef::Hole(ecmaVm);
    PropertyAttribute property = PropertyAttribute::Default();
    property.SetValue(func);
    std::unique_ptr<panda::ecmascript::tooling::PropertyDescriptor> result =
        panda::ecmascript::tooling::PropertyDescriptor::FromProperty(ecmaVm, funcName, property);
    ASSERT_TRUE(result->GetName() ==
        panda::ecmascript::tooling::PropertyDescriptor::ANONYMOUS_FUNCTION_NAME_PLACEHOLDER);
}

HWTEST_F_L0(PtTypesTest, PtEventsGetReasonString)
{
    PauseReason reason = static_cast<PauseReason>(50);
    std::string ret = Paused::GetReasonString(reason);
    ASSERT_TRUE(ret.empty());
}

HWTEST_F_L0(PtTypesTest, LocationRangeToJsonTest)
{
    LocationRange range;
    std::unique_ptr<ScriptPosition> start = std::make_unique<ScriptPosition>();
    std::unique_ptr<ScriptPosition> end = std::make_unique<ScriptPosition>();
    range.SetStart(std::move(start));
    range.SetEnd(std::move(end));
    std::unique_ptr<PtJson> result = range.ToJson();
    ASSERT_TRUE(result);
}

HWTEST_F_L0(PtTypesTest, BreakLocationToJsonTest)
{
    BreakLocation location;
    location.SetColumn(0);
    location.SetType("test");
    std::unique_ptr<PtJson> result = location.ToJson();
    ASSERT_TRUE(result);
}

HWTEST_F_L0(PtTypesTest, CallFrameCreateTest)
{
    std::unique_ptr<PtJson> ptJson = PtJson::CreateObject();
    std::unique_ptr<PtJson> array = PtJson::CreateArray();
    std::unique_ptr<PtJson> object = PtJson::CreateObject();
    object->Add("type", 0);
    array->Push(object);
    ptJson->Add("scopeChain", array);
    std::unique_ptr<CallFrame> result = CallFrame::Create(*ptJson);
    ASSERT_TRUE(!result);
}

HWTEST_F_L0(PtTypesTest, ProfileCreate)
{
    std::unique_ptr<PtJson> ptJson = PtJson::CreateObject();
    std::unique_ptr<PtJson> array = PtJson::CreateArray();
    std::unique_ptr<PtJson> object = PtJson::CreateObject();
    std::unique_ptr<PtJson> object1 = PtJson::CreateObject();
    object->Add("type", 0);
    array->Push(object);
    ptJson->Add("nodes", array);
    std::unique_ptr<Profile> result = Profile::Create(*ptJson);
    ASSERT_TRUE(!result);
    std::unique_ptr<PtJson> ptJson1 = PtJson::CreateObject();
    ptJson1->Add("samples", 0);
    ptJson1->Add("timeDeltas", 0);
    std::unique_ptr<Profile> result1 = Profile::Create(*ptJson1);
    ASSERT_TRUE(!result1);
}

HWTEST_F_L0(PtTypesTest, ProfileNodeCreate)
{
    std::unique_ptr<PtJson> ptJson = PtJson::CreateObject();
    std::string attribute = "test";
    std::unique_ptr<PtJson> object = PtJson::CreateObject();
    object->Add("functionName", 0);
    ptJson->Add("callFrame", object);
    ptJson->Add("hitCount", attribute.c_str());
    ptJson->Add("children", attribute.c_str());
    ptJson->Add("positionTicks", attribute.c_str());
    ptJson->Add("deoptReason", 0);
    std::unique_ptr<ProfileNode> result = ProfileNode::Create(*ptJson);
    ASSERT_TRUE(!result);
    std::unique_ptr<PtJson> ptJson1 = PtJson::CreateObject();
    std::unique_ptr<PtJson> object1 = PtJson::CreateObject();
    std::unique_ptr<PtJson> array = PtJson::CreateArray();
    object1->Add("attribute", attribute.c_str());
    array->Push(object1);
    ptJson1->Add("positionTicks", array);
    std::unique_ptr<ProfileNode> result1 = ProfileNode::Create(*ptJson1);
    ASSERT_TRUE(!result1);
}

HWTEST_F_L0(PtTypesTest, LocationCreate)
{
    std::unique_ptr<PtJson> ptJson = PtJson::CreateObject();
    std::string attribute = "test";
    ptJson->Add("columnNumber", attribute.c_str());
    std::unique_ptr<Location> result = Location::Create(*ptJson);
    ASSERT_TRUE(!result);
}

HWTEST_F_L0(PtTypesTest, PropertyDescriptorCreate)
{
    std::unique_ptr<PtJson> ptJson = PtJson::CreateObject();
    std::unique_ptr<PtJson> get = PtJson::CreateObject();
    std::unique_ptr<PtJson> set = PtJson::CreateObject();
    std::unique_ptr<PtJson> symbol = PtJson::CreateObject();
    std::string attribute = "test";
    get->Add("type", attribute.c_str());
    set->Add("type", attribute.c_str());
    symbol->Add("type", attribute.c_str());
    ptJson->Add("get", get);
    ptJson->Add("set", set);
    ptJson->Add("symbol", symbol);
    std::unique_ptr<tooling::PropertyDescriptor> result = tooling::PropertyDescriptor::Create(*ptJson);
    ASSERT_TRUE(!result);
}

HWTEST_F_L0(PtTypesTest, SamplingHeapProfileFromSamplingInfo)
{
    SamplingInfo samplingInfo;
    samplingInfo.head_.selfSize_ = 0;
    samplingInfo.head_.id_ = 0;
    samplingInfo.head_.callFrameInfo_.codeType_ = "codetype";
    samplingInfo.head_.callFrameInfo_.columnNumber_ = 21;
    samplingInfo.head_.callFrameInfo_.functionName_ = "TestSampling";
    samplingInfo.head_.callFrameInfo_.lineNumber_ = 221;
    samplingInfo.head_.callFrameInfo_.scriptId_ = 1;
    samplingInfo.head_.callFrameInfo_.url_ = "url";
    Sample sampleInfo(12024, 19, 1, 2);
    samplingInfo.samples_.push_back(sampleInfo);
    std::unique_ptr<SamplingHeapProfile> profile = SamplingHeapProfile::FromSamplingInfo(&samplingInfo);
    ASSERT_TRUE(profile != nullptr);
    SamplingHeapProfileNode *nodeInfo = profile->GetHead();
    int32_t selfSize = nodeInfo->GetSelfSize();
    ASSERT_TRUE(selfSize == samplingInfo.head_.selfSize_);
    int32_t id = nodeInfo->GetId();
    ASSERT_TRUE(id == samplingInfo.head_.id_);
    RuntimeCallFrame *callFrame = nodeInfo->GetCallFrame();
    const std::string functionName = callFrame->GetFunctionName();
    ASSERT_TRUE(functionName == samplingInfo.head_.callFrameInfo_.functionName_);
    const std::string url = callFrame->GetUrl();
    ASSERT_TRUE(url == samplingInfo.head_.callFrameInfo_.url_);
    int32_t lineNumber = callFrame->GetLineNumber();
    ASSERT_TRUE(lineNumber == samplingInfo.head_.callFrameInfo_.lineNumber_);
    int32_t columnNumber = callFrame->GetColumnNumber();
    ASSERT_TRUE(columnNumber == samplingInfo.head_.callFrameInfo_.columnNumber_);
    const std::vector<std::unique_ptr<SamplingHeapProfileSample>> *sampData = profile->GetSamples();
    int32_t nodeId = sampData->data()->get()->GetNodeId();
    ASSERT_TRUE(nodeId == sampleInfo.nodeId_);
    int32_t size = sampData->data()->get()->GetSize();
    ASSERT_TRUE(size == (sampleInfo.size_ * sampleInfo.count_));
    int64_t ordinal = sampData->data()->get()->GetOrdinal();
    ASSERT_TRUE(ordinal == sampleInfo.ordinal_);
}

HWTEST_F_L0(PtTypesTest, SamplingHeapProfileTransferHead)
{
    SamplingNode allocationNode;
    allocationNode.selfSize_ = 0;
    allocationNode.id_ = 0;
    allocationNode.callFrameInfo_.codeType_ = "codetype";
    allocationNode.callFrameInfo_.columnNumber_ = 21;
    allocationNode.callFrameInfo_.functionName_ = "TestSampling";
    allocationNode.callFrameInfo_.lineNumber_ = 221;
    allocationNode.callFrameInfo_.scriptId_ = 1;
    allocationNode.callFrameInfo_.url_ = "url";
    std::unique_ptr<SamplingHeapProfileNode> headInfo = SamplingHeapProfile::TransferHead(&allocationNode);
    ASSERT_TRUE(headInfo != nullptr);
    int32_t selfSize = headInfo->GetSelfSize();
    ASSERT_TRUE(selfSize == allocationNode.selfSize_);
    int32_t id = headInfo->GetId();
    ASSERT_TRUE(id == allocationNode.id_);
    RuntimeCallFrame *callFrame = headInfo->GetCallFrame();
    const std::string functionName = callFrame->GetFunctionName();
    ASSERT_TRUE(functionName == allocationNode.callFrameInfo_.functionName_);
    const std::string url = callFrame->GetUrl();
    ASSERT_TRUE(url == allocationNode.callFrameInfo_.url_);
    int32_t lineNumber = callFrame->GetLineNumber();
    ASSERT_TRUE(lineNumber == allocationNode.callFrameInfo_.lineNumber_);
    int32_t columnNumber = callFrame->GetColumnNumber();
    ASSERT_TRUE(columnNumber == allocationNode.callFrameInfo_.columnNumber_);
}

HWTEST_F_L0(PtTypesTest, DescriptionForObjectTest)
{
    Local<WeakMapRef> weakmap = WeakMapRef::New(ecmaVm);
    std::string arrayOrContainer;
    std::string description = ObjectRemoteObject::DescriptionForObject(ecmaVm, weakmap, arrayOrContainer);
    ASSERT_TRUE(!arrayOrContainer.empty());
    ASSERT_TRUE(description.find("WeakMap") != std::string::npos);

    Local<MapRef> map = MapRef::New(ecmaVm);
    Local<MapIteratorRef> mapiterator = MapIteratorRef::New(ecmaVm, map);
    description = ObjectRemoteObject::DescriptionForObject(ecmaVm, mapiterator, arrayOrContainer);
    ASSERT_TRUE(description.find("MapIterator") != std::string::npos);

    Local<SetRef> set = SetRef::New(ecmaVm);
    Local<SetIteratorRef> setiterator = SetIteratorRef::New(ecmaVm, set);
    description = ObjectRemoteObject::DescriptionForObject(ecmaVm, setiterator, arrayOrContainer);
    ASSERT_TRUE(description.find("SetIterator") != std::string::npos);

    size_t size = 10;
    Local<NativePointerRef> nativepointer = NativePointerRef::New(ecmaVm, ecmaVm, size);
    description = ObjectRemoteObject::DescriptionForObject(ecmaVm, nativepointer, arrayOrContainer);
    std::cout << description << std::endl;
    ASSERT_TRUE(description.find("External") != std::string::npos);
}

HWTEST_F_L0(PtTypesTest, FromTaggedTest)
{
    double input = 123456789.0;
    Local<DateRef> date = DateRef::New(ecmaVm, input);
    std::unique_ptr<RemoteObject> remoteObject = RemoteObject::FromTagged(ecmaVm, date);
    std::string description = remoteObject->GetDescription();
    ASSERT_TRUE(description.find("GMT") != std::string::npos);

    Local<WeakMapRef> weakmap = WeakMapRef::New(ecmaVm);
    remoteObject = RemoteObject::FromTagged(ecmaVm, weakmap);
    description = remoteObject->GetDescription();
    ASSERT_TRUE(description.find("WeakMap") != std::string::npos);

    Local<MapRef> map = MapRef::New(ecmaVm);
    Local<MapIteratorRef> mapiterator = MapIteratorRef::New(ecmaVm, map);
    remoteObject = RemoteObject::FromTagged(ecmaVm, mapiterator);
    description = remoteObject->GetDescription();
    ASSERT_TRUE(description.find("MapIterator") != std::string::npos);

    Local<SetRef> set = SetRef::New(ecmaVm);
    Local<SetIteratorRef> setiterator = SetIteratorRef::New(ecmaVm, set);
    remoteObject = RemoteObject::FromTagged(ecmaVm, setiterator);
    description = remoteObject->GetDescription();
    ASSERT_TRUE(description.find("SetIterator") != std::string::npos);

    Local<PromiseCapabilityRef> capability = PromiseCapabilityRef::New(ecmaVm);
    Local<PromiseRef> promise = capability->GetPromise(ecmaVm);
    remoteObject = RemoteObject::FromTagged(ecmaVm, promise);
    description = remoteObject->GetDescription();
    ASSERT_TRUE(description.find("Promise") != std::string::npos);

    size_t size = 10;
    Local<NativePointerRef> nativepointer = NativePointerRef::New(ecmaVm, ecmaVm, size);
    remoteObject = RemoteObject::FromTagged(ecmaVm, nativepointer);
    description = remoteObject->GetDescription();
    ASSERT_TRUE(description.find("External") != std::string::npos);
}

HWTEST_F_L0(PtTypesTest, FromTaggedPrimitiveTypes)
{
    // Test number
    int32_t input = 12345;
    Local<NumberRef> numberRef = NumberRef::New(ecmaVm, input);
    std::unique_ptr<RemoteObject> remoteObject2 = RemoteObject::FromTagged(ecmaVm, numberRef);
    std::string description = remoteObject2->GetType();
    ASSERT_EQ(description, "number");

    // Test boolean - true
    Local<BooleanRef> trueRef = BooleanRef::New(ecmaVm, true);
    std::unique_ptr<RemoteObject> trueRemoteObj = RemoteObject::FromTagged(ecmaVm, trueRef);
    ASSERT_TRUE(trueRemoteObj != nullptr);
    ASSERT_EQ(trueRemoteObj->GetType(), RemoteObject::TypeName::Boolean);
    ASSERT_TRUE(trueRemoteObj->HasDescription());
    ASSERT_EQ(trueRemoteObj->GetDescription(), "true");

    // Test boolean - false
    Local<BooleanRef> falseRef = BooleanRef::New(ecmaVm, false);
    std::unique_ptr<RemoteObject> falseRemoteObj = RemoteObject::FromTagged(ecmaVm, falseRef);
    ASSERT_TRUE(falseRemoteObj != nullptr);
    ASSERT_EQ(falseRemoteObj->GetType(), RemoteObject::TypeName::Boolean);
    ASSERT_TRUE(falseRemoteObj->HasDescription());
    ASSERT_EQ(falseRemoteObj->GetDescription(), "false");

    // Test number
    Local<NumberRef> numberRef2 = NumberRef::New(ecmaVm, 123.45);
    std::unique_ptr<RemoteObject> numberRemoteObj = RemoteObject::FromTagged(ecmaVm, numberRef2);
    ASSERT_TRUE(numberRemoteObj != nullptr);
    ASSERT_EQ(numberRemoteObj->GetType(), RemoteObject::TypeName::Number);
    ASSERT_TRUE(numberRemoteObj->HasDescription());

    // Test bigint
    int64_t num = 456;
    Local<BigIntRef> bigIntRef = BigIntRef::New(ecmaVm, num);
    std::unique_ptr<RemoteObject> bigIntRemoteObj = RemoteObject::FromTagged(ecmaVm, bigIntRef);
    ASSERT_TRUE(bigIntRemoteObj != nullptr);
    ASSERT_EQ(bigIntRemoteObj->GetType(), RemoteObject::TypeName::Bigint);
    ASSERT_TRUE(bigIntRemoteObj->HasDescription());
    ASSERT_TRUE(bigIntRemoteObj->GetDescription().find("456n") != std::string::npos);

    Local<WeakSetRef> weakSetRef = WeakSetRef::New(ecmaVm);
    std::unique_ptr<RemoteObject> weakSetObj = RemoteObject::FromTagged(ecmaVm, weakSetRef);
    ASSERT_TRUE(weakSetObj != nullptr);
    description = weakSetObj->GetDescription();
    ASSERT_TRUE(description.find("WeakSet") != std::string::npos);
}

HWTEST_F_L0(PtTypesTest, NativeRangeCreateTest)
{
    std::string msg;
    std::unique_ptr<NativeRange> nativeRange;

    msg = std::string() + R"({})";
    nativeRange = NativeRange::Create(DispatchRequest(msg).GetParams());
    EXPECT_EQ(nativeRange, nullptr);

    msg = std::string() + R"({"id":0,"method":"Debugger.Test","params":{"start":20,"end":40}})";
    nativeRange = NativeRange::Create(DispatchRequest(msg).GetParams());
    ASSERT_TRUE(nativeRange != nullptr);
}

HWTEST_F_L0(PtTypesTest, BreakpointInfoCreateTest)
{
    std::string msg;
    std::unique_ptr<BreakpointInfo> breakpointInfo;

    msg = std::string() + R"({})";
    breakpointInfo = BreakpointInfo::Create(DispatchRequest(msg).GetParams());
    EXPECT_EQ(breakpointInfo, nullptr);

    msg = std::string() + R"({"id":0,"method":"Debugger.Test","params":{"condition":"Test","urlRegex":"Test"}})";
    breakpointInfo = BreakpointInfo::Create(DispatchRequest(msg).GetParams());
    EXPECT_EQ(breakpointInfo, nullptr);

    msg = std::string() + R"({"id":0,"method":"Debugger.Test",
        "params":{"scriptHash":"Test", "restrictToFunction": true}})";
    breakpointInfo = BreakpointInfo::Create(DispatchRequest(msg).GetParams());
    EXPECT_EQ(breakpointInfo, nullptr);
    
    msg = std::string() + R"({"id":0,"method":"Debugger.Test","params":{"lineNumber":1,"columnNumber":2,"url":"Test",
        "condition":"Test", "urlRegex":"Test", "scriptHash":"Test", "restrictToFunction": true}})";
    breakpointInfo = BreakpointInfo::Create(DispatchRequest(msg).GetParams());
    std::unique_ptr<PtJson> resultJson = breakpointInfo->ToJson();
    ASSERT_TRUE(resultJson != nullptr);

    msg = std::string() + R"({"id":0,"method":"Debugger.Test",
        "params":{"lineNumber":1, "columnNumber":2, "url":"Test"}})";
    breakpointInfo = BreakpointInfo::Create(DispatchRequest(msg).GetParams());
    resultJson = breakpointInfo->ToJson();
    ASSERT_TRUE(resultJson != nullptr);
}

HWTEST_F_L0(PtTypesTest, BreakpointReturnInfoCreateTest)
{
    std::string msg;
    std::unique_ptr<BreakpointReturnInfo> breakpointReturnInfo;

    msg = std::string() + R"({})";
    breakpointReturnInfo = BreakpointReturnInfo::Create(DispatchRequest(msg).GetParams());
    EXPECT_EQ(breakpointReturnInfo, nullptr);

    msg = std::string() + R"({"id":0,"method":"Debugger.Test","params":{"lineNumber":20,"columnNumber":40}})";
    breakpointReturnInfo = BreakpointReturnInfo::Create(DispatchRequest(msg).GetParams());
    ASSERT_TRUE(breakpointReturnInfo != nullptr);

    msg = std::string() + R"({"id":0,"method":"Debugger.Test","params":{"id":"Test","scriptId":5}})";
    breakpointReturnInfo = BreakpointReturnInfo::Create(DispatchRequest(msg).GetParams());
    EXPECT_EQ(breakpointReturnInfo, nullptr);
}

HWTEST_F_L0(PtTypesTest, InvalidUtf16Test)
{
    const char16_t invalidUtf16[] = { 0xD800 };
    Local<StringRef> invalidStrRef = StringRef::NewFromUtf16(ecmaVm, invalidUtf16);
    std::string invalidStrInDebugger = invalidStrRef->DebuggerToString(ecmaVm);

    auto utf8 = reinterpret_cast<const uint8_t*>(invalidStrInDebugger.c_str());
    size_t utf8Len = invalidStrInDebugger.size();
    for (size_t i = 0; i < utf8Len;) {
        std::pair<int32_t, size_t> result =
            common::utf_helper::ConvertUtf8ToUnicodeChar(utf8 + i, utf8Len - i);
        // -1: invalid utf8
        ASSERT_TRUE(result.first != -1);
        i += result.second;
    }
}

HWTEST_F_L0(PtTypesTest, InvalidUtf16DescriptionTest)
{
    const char16_t invalidUtf16[] = { 0xD800 };
    Local<StringRef> invalidStrRef = StringRef::NewFromUtf16(ecmaVm, invalidUtf16);
    JSHandle<JSTaggedValue> invalidStrHandle(thread, JSNApiHelper::ToJSTaggedValue(*invalidStrRef));
    std::string invalidStrInDebugger = invalidStrRef->DebuggerToString(ecmaVm);
    std::string invalidStr = invalidStrRef->ToString(ecmaVm);
    ASSERT_TRUE(invalidStrInDebugger != invalidStr);

    // String
    StringRemoteObject strObj(ecmaVm, invalidStrRef);
    EXPECT_EQ(strObj.GetDescription(), invalidStrInDebugger);

    // Primitive String
    ObjectFactory *factory = ecmaVm->GetFactory();
    JSHandle<JSTaggedValue> primitiveStrHandle =
        JSHandle<JSTaggedValue>::Cast(factory->NewJSPrimitiveRef(PrimitiveType::PRIMITIVE_STRING, invalidStrHandle));
    Local<JSValueRef> primitiveStr = JSNApiHelper::ToLocal<JSValueRef>(primitiveStrHandle);
    std::string arrayOrContainer;
    std::string primitiveStrDesc = ObjectRemoteObject::DescriptionForObject(ecmaVm, primitiveStr, arrayOrContainer);
    ASSERT_TRUE(primitiveStrDesc.find(invalidStrInDebugger) != std::string::npos);
    ASSERT_TRUE(arrayOrContainer.empty());

    // RegExp
    JSHandle<JSHClass> jsRegExpClass = factory->NewEcmaHClass(JSRegExp::SIZE, JSType::JS_REG_EXP);
    JSHandle<JSRegExp> jsRegExp = JSHandle<JSRegExp>::Cast(factory->NewJSObject(jsRegExpClass));
    jsRegExp->SetOriginalSource(thread, invalidStrHandle.GetTaggedValue());
    JSHandle<JSTaggedValue> jsRegExpHandle = JSHandle<JSTaggedValue>::Cast(jsRegExp);
    Local<RegExpRef> regExpRef = JSNApiHelper::ToLocal<RegExpRef>(jsRegExpHandle);
    std::string regExpDesc = ObjectRemoteObject::DescriptionForObject(ecmaVm, regExpRef, arrayOrContainer);
    ASSERT_TRUE(regExpDesc.find(invalidStrInDebugger) != std::string::npos);

    // Map
    Local<MapRef> mapRef = MapRef::New(ecmaVm);
    mapRef->Set(ecmaVm, invalidStrRef, invalidStrRef);
    std::string mapDesc = ObjectRemoteObject::DescriptionForObject(ecmaVm, mapRef, arrayOrContainer);
    ASSERT_TRUE(!arrayOrContainer.empty());
    ASSERT_TRUE(mapDesc.find(invalidStrInDebugger) != std::string::npos);

    // WeakMap
    Local<WeakMapRef> weakMapRef = WeakMapRef::New(ecmaVm);
    weakMapRef->Set(ecmaVm, primitiveStr, invalidStrRef);
    std::string weakMapDesc = ObjectRemoteObject::DescriptionForObject(ecmaVm, weakMapRef, arrayOrContainer);
    ASSERT_TRUE(!arrayOrContainer.empty());
    ASSERT_TRUE(weakMapDesc.find(invalidStrInDebugger) != std::string::npos);

    // Set
    Local<SetRef> setRef = SetRef::New(ecmaVm);
    setRef->Add(ecmaVm, invalidStrRef);
    std::string setDesc = ObjectRemoteObject::DescriptionForObject(ecmaVm, setRef, arrayOrContainer);
    ASSERT_TRUE(!arrayOrContainer.empty());
    ASSERT_TRUE(setDesc.find(invalidStrInDebugger) != std::string::npos);

    // WeakSet
    Local<WeakSetRef> weakSetRef = WeakSetRef::New(ecmaVm);
    weakSetRef->Add(ecmaVm, invalidStrRef);
    std::string weakSetDesc = ObjectRemoteObject::DescriptionForObject(ecmaVm, weakSetRef, arrayOrContainer);
    ASSERT_TRUE(!arrayOrContainer.empty());
    ASSERT_TRUE(weakSetDesc.find(invalidStrInDebugger) != std::string::npos);

    // Error
    Local<JSValueRef> error = Exception::Error(ecmaVm, invalidStrRef);
    std::string errorDesc = ObjectRemoteObject::DescriptionForObject(ecmaVm, error, arrayOrContainer);
    ASSERT_TRUE(errorDesc.find(invalidStrInDebugger) != std::string::npos);

    // Symbol
    Local<SymbolRef> symbol = SymbolRef::New(ecmaVm, invalidStrRef);
    SymbolRemoteObject symbolObj(ecmaVm, symbol);
    std::string symbolDesc = symbolObj.GetDescription();
    ASSERT_TRUE(symbolDesc.find(invalidStrInDebugger) != std::string::npos);
}
}  // namespace panda::test
