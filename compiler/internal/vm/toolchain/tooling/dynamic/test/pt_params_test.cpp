/*
 * Copyright (c) 2022-2025 Huawei Device Co., Ltd.
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

#include "tooling/dynamic/base/pt_params.h"
#include "ecmascript/tests/test_helper.h"
#include "protocol_handler.h"

using namespace panda::ecmascript;
using namespace panda::ecmascript::tooling;

namespace panda::test {
class PtParamsTest : public testing::Test {
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

HWTEST_F_L0(PtParamsTest, EnableParamsCreateTest)
{
    std::unique_ptr<PtJson> ptJson = PtJson::CreateObject();
    std::string attribute = "test";
    ptJson->Add("maxScriptsCacheSize", attribute.c_str());
    std::unique_ptr<EnableParams> result = EnableParams::Create(*ptJson);
    ASSERT_TRUE(result == nullptr);
}

HWTEST_F_L0(PtParamsTest, EvaluateOnCallFrameParamsCreateTest)
{
    std::unique_ptr<PtJson> ptJson = PtJson::CreateObject();
    std::string attribute = "test";
    ptJson->Add("objectGroup", 0);
    ptJson->Add("includeCommandLineAPI", attribute.c_str());
    ptJson->Add("silent", attribute.c_str());
    ptJson->Add("returnByValue", attribute.c_str());
    ptJson->Add("generatePreview", attribute.c_str());
    ptJson->Add("throwOnSideEffect", attribute.c_str());
    std::unique_ptr<EvaluateOnCallFrameParams> result = EvaluateOnCallFrameParams::Create(*ptJson);
    ASSERT_TRUE(result == nullptr);
}

HWTEST_F_L0(PtParamsTest, GetPossibleBreakpointsParamsCreateTest)
{
    std::unique_ptr<PtJson> ptJson = PtJson::CreateObject();
    std::unique_ptr<PtJson>  start = PtJson::CreateObject();
    std::unique_ptr<PtJson> ptJson1 = PtJson::CreateObject();
    std::unique_ptr<PtJson>  end = PtJson::CreateObject();
    std::string attribute = "test";
    ptJson->Add("start", start);
    ptJson->Add("end", end);
    std::unique_ptr<GetPossibleBreakpointsParams> result = GetPossibleBreakpointsParams::Create(*ptJson);
    ASSERT_TRUE(result == nullptr);
    ptJson1->Add("end", attribute.c_str());
    ptJson1->Add("restrictToFunction", attribute.c_str());
    std::unique_ptr<GetPossibleBreakpointsParams> result1 = GetPossibleBreakpointsParams::Create(*ptJson1);
    ASSERT_TRUE(result1 == nullptr);
}

HWTEST_F_L0(PtParamsTest, ResumeParamsCreateTest)
{
    std::unique_ptr<PtJson> ptJson = PtJson::CreateObject();
    std::unique_ptr<PtJson> ptJson1 = PtJson::CreateObject();
    std::string attribute = "test";
    ptJson->Add("terminateOnResume", attribute.c_str());
    std::unique_ptr<ResumeParams> result = ResumeParams::Create(*ptJson);
    ASSERT_TRUE(result == nullptr);
    std::unique_ptr<ResumeParams> result1 = ResumeParams::Create(*ptJson1);
    ASSERT_TRUE(result == nullptr);
}

HWTEST_F_L0(PtParamsTest, SetBlackboxPatternsParamsCreateTest)
{
    std::unique_ptr<PtJson> ptJson = PtJson::CreateObject();
    std::unique_ptr<PtJson> array = PtJson::CreateArray();
    std::string attribute = "test";
    array->Push(128);
    array->Push(true);
    array->Push(attribute.c_str());
    ptJson->Add("patterns", array);
    std::unique_ptr<SetBlackboxPatternsParams> result = SetBlackboxPatternsParams::Create(*ptJson);
    ASSERT_TRUE(result == nullptr);
}

HWTEST_F_L0(PtParamsTest, StepIntoParamsCreateTest)
{
    std::unique_ptr<PtJson> ptJson = PtJson::CreateObject();
    std::unique_ptr<PtJson> ptJson1 = PtJson::CreateObject();
    std::unique_ptr<PtJson> array = PtJson::CreateArray();
    std::unique_ptr<PtJson> scriptId = PtJson::CreateObject();
    std::unique_ptr<PtJson> end = PtJson::CreateObject();
    std::string attribute = "test";
    scriptId->Add("scriptId", 123);
    end->Add("end", 123);
    array->Push(scriptId);
    array->Push(end);
    ptJson->Add("skipList", array);
    std::unique_ptr<StepIntoParams> result = StepIntoParams::Create(*ptJson);
    ASSERT_TRUE(result == nullptr);
    std::unique_ptr<StepIntoParams> result1 = StepIntoParams::Create(*ptJson1);
    ASSERT_TRUE(result1 != nullptr);
}

HWTEST_F_L0(PtParamsTest, StepOverParamsCreateTest)
{
    std::unique_ptr<PtJson> ptJson = PtJson::CreateObject();
    std::unique_ptr<PtJson> ptJson1 = PtJson::CreateObject();
    std::unique_ptr<PtJson> array = PtJson::CreateArray();
    std::unique_ptr<PtJson> scriptId = PtJson::CreateObject();
    std::unique_ptr<PtJson> end = PtJson::CreateObject();
    std::string attribute = "test";
    scriptId->Add("scriptId", 123);
    end->Add("end", 123);
    array->Push(scriptId);
    array->Push(end);
    ptJson->Add("skipList", array);
    std::unique_ptr<StepOverParams> result = StepOverParams::Create(*ptJson);
    ASSERT_TRUE(result == nullptr);
    std::unique_ptr<StepOverParams> result1 = StepOverParams::Create(*ptJson1);
    ASSERT_TRUE(result1 != nullptr);
}

HWTEST_F_L0(PtParamsTest, SetMixedDebugParamsCreateTest)
{
    std::unique_ptr<PtJson> ptJson = PtJson::CreateObject();
    std::unique_ptr<PtJson> ptJson1 = PtJson::CreateObject();
    std::string attribute = "test";
    ptJson->Add("enabled", attribute.c_str());
    std::unique_ptr<SetMixedDebugParams> result = SetMixedDebugParams::Create(*ptJson);
    ASSERT_TRUE(result == nullptr);
    std::unique_ptr<SetMixedDebugParams> result1 = SetMixedDebugParams::Create(*ptJson1);
    ASSERT_TRUE(result == nullptr);
}

HWTEST_F_L0(PtParamsTest, ReplyNativeCallingParamsCreateTest)
{
    std::unique_ptr<PtJson> ptJson = PtJson::CreateObject();
    std::unique_ptr<PtJson> ptJson1 = PtJson::CreateObject();
    std::string attribute = "test";
    ptJson->Add("userCode", attribute.c_str());
    std::unique_ptr<ReplyNativeCallingParams> result = ReplyNativeCallingParams::Create(*ptJson);
    ASSERT_TRUE(result == nullptr);
    std::unique_ptr<ReplyNativeCallingParams> result1 = ReplyNativeCallingParams::Create(*ptJson1);
    ASSERT_TRUE(result == nullptr);
}

HWTEST_F_L0(PtParamsTest, CallFunctionOnParamsCreateTest)
{
    std::unique_ptr<PtJson> ptJson = PtJson::CreateObject();
    std::string attribute = "test";
    ptJson->Add("arguments", attribute.c_str());
    std::unique_ptr<CallFunctionOnParams> result = CallFunctionOnParams::Create(*ptJson);
    ASSERT_TRUE(result == nullptr);
}

HWTEST_F_L0(PtParamsTest, GetHeapObjectIdParamsCreateTest)
{
    std::unique_ptr<PtJson> ptJson = PtJson::CreateObject();
    std::string attribute = "test";
    ptJson->Add("arguments", attribute.c_str());
    std::unique_ptr<GetHeapObjectIdParams> result = GetHeapObjectIdParams::Create(*ptJson);
    ASSERT_TRUE(result != nullptr);
}

HWTEST_F_L0(PtParamsTest, GetObjectByHeapObjectIdParamsCreateTest)
{
    std::unique_ptr<PtJson> ptJson = PtJson::CreateObject();
    std::string attribute = "test";
    ptJson->Add("arguments", attribute.c_str());
    std::unique_ptr<GetObjectByHeapObjectIdParams> result = GetObjectByHeapObjectIdParams::Create(*ptJson);
    ASSERT_TRUE(result != nullptr);
}

HWTEST_F_L0(PtParamsTest, RequestMemoryDumpParamsCreateTest)
{
    std::unique_ptr<PtJson> ptJson = PtJson::CreateObject();
    std::unique_ptr<PtJson> ptJson1 = PtJson::CreateObject();
    std::string attribute = "test";
    ptJson->Add("deterministic", attribute.c_str());
    ptJson->Add("levelOfDetail", attribute.c_str());
    std::unique_ptr<RequestMemoryDumpParams> result = RequestMemoryDumpParams::Create(*ptJson);
    ASSERT_TRUE(result == nullptr);
    ptJson1->Add("levelOfDetail", 0);
    std::unique_ptr<RequestMemoryDumpParams> result1 = RequestMemoryDumpParams::Create(*ptJson1);
    ASSERT_TRUE(result1 == nullptr);
}

HWTEST_F_L0(PtParamsTest, StartParamsCreateTest)
{
    std::unique_ptr<PtJson> ptJson = PtJson::CreateObject();
    std::unique_ptr<PtJson> ptJson1 = PtJson::CreateObject();
    std::unique_ptr<PtJson> traceConfig = PtJson::CreateObject();
    std::string attribute = "test";
    ptJson->Add("categories", 0);
    ptJson->Add("options", 0);
    ptJson->Add("bufferUsageReportingInterval", attribute.c_str());
    ptJson->Add("transferMode", 0);
    ptJson->Add("streamFormat", 0);
    ptJson->Add("streamCompression", 0);
    ptJson->Add("traceConfig", 0);
    ptJson->Add("perfettoConfig", 0);
    ptJson->Add("tracingBackend", 0);
    std::unique_ptr<StartParams> result = StartParams::Create(*ptJson);
    ASSERT_TRUE(result == nullptr);
    ptJson1->Add("transferMode", attribute.c_str());
    ptJson1->Add("streamFormat", attribute.c_str());
    ptJson1->Add("streamCompression", attribute.c_str());
    traceConfig->Add("recordMode", 0);
    ptJson1->Add("traceConfig", traceConfig);
    ptJson1->Add("tracingBackend", attribute.c_str());

    std::unique_ptr<StartParams> result1 = StartParams::Create(*ptJson1);
    ASSERT_TRUE(result1 == nullptr);
}

HWTEST_F_L0(PtParamsTest, AddRequireParamsTest)
{
    std::string msg;
    std::unique_ptr<SmartStepIntoParams> smartStepIntoParams;

    msg = std::string() + R"({})";
    smartStepIntoParams = SmartStepIntoParams::Create(DispatchRequest(msg).GetParams());
    EXPECT_EQ(smartStepIntoParams, nullptr);
}

HWTEST_F_L0(PtParamsTest, SeriliazationTimeoutCheckEnableParamsCreateTest)
{
    std::string msg;
    std::unique_ptr<SeriliazationTimeoutCheckEnableParams> seriliazationParams;

    msg = std::string() + R"({})";
    seriliazationParams = SeriliazationTimeoutCheckEnableParams::Create(DispatchRequest(msg).GetParams());
    EXPECT_EQ(seriliazationParams, nullptr);

    msg = std::string() + R"({"id":0,"method":"PtParams.Test","params":{"threshold":2}})";
    seriliazationParams = SeriliazationTimeoutCheckEnableParams::Create(DispatchRequest(msg).GetParams());
    ASSERT_TRUE(seriliazationParams != nullptr);
}

HWTEST_F_L0(PtParamsTest, RuntimeGetPropertiesParamTest)
{
    std::unique_ptr<PtJson> requestWithStartAndCount = PtJson::CreateObject();
    std::unique_ptr<PtJson> requestWithoutStartAndCount = PtJson::CreateObject();
    requestWithStartAndCount->Add("objectId", "0");
    requestWithStartAndCount->Add("accessorPropertiesOnly", false);
    requestWithStartAndCount->Add("generatePreview", true);
    requestWithStartAndCount->Add("ownProperties", true);
    requestWithStartAndCount->Add("start", 0);
    requestWithStartAndCount->Add("count", 100);
    
    std::unique_ptr<GetPropertiesParams> result = GetPropertiesParams::Create(*requestWithStartAndCount);
    // Assert creating request paramater success
    ASSERT_TRUE(result != nullptr);
    // Asset this parameter is valid for requesting with range
    ASSERT_TRUE(result->IsValidRequestUsingRange());
 
    requestWithoutStartAndCount->Add("objectId", "0");
    requestWithoutStartAndCount->Add("accessorPropertiesOnly", true);
    requestWithoutStartAndCount->Add("generatePreview", true);
    requestWithoutStartAndCount->Add("ownProperties", false);
 
    result = GetPropertiesParams::Create(*requestWithoutStartAndCount);
    // Assert creating request paramater success
    ASSERT_TRUE(result != nullptr);
    // Asset this parameter is NOT valid for requesting with range
    ASSERT_TRUE(!result->IsValidRequestUsingRange());
}
}  // namespace panda::test