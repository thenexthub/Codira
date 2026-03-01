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

#include "tooling/dynamic/base/pt_returns.h"
#include "ecmascript/tests/test_helper.h"
#include "tooling/dynamic/protocol_handler.h"

using namespace panda::ecmascript;
using namespace panda::ecmascript::tooling;


namespace panda::test {
class PtReturnsTest : public testing::Test {
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

HWTEST_F_L0(PtReturnsTest, EvaluateOnCallFrameReturns)
{
    std::unique_ptr<RemoteObject> result = std::make_unique<RemoteObject>();
    result->SetType("idle");
    std::unique_ptr<EvaluateOnCallFrameReturns> evaluateOnCallFrameReturns
                     = std::make_unique<EvaluateOnCallFrameReturns>(std::move(result));
    ASSERT_NE(evaluateOnCallFrameReturns, nullptr);

    std::unique_ptr<PtJson> tmpJson;
    ASSERT_EQ(evaluateOnCallFrameReturns->ToJson()->GetObject("exceptionDetails", &tmpJson), Result::NOT_EXIST);
}

HWTEST_F_L0(PtReturnsTest, GetScriptSourceReturns)
{
    std::unique_ptr<GetScriptSourceReturns> getScriptSourceReturns = std::make_unique
                                                                    <GetScriptSourceReturns>("source_1");
    ASSERT_NE(getScriptSourceReturns, nullptr);

    std::unique_ptr<PtJson> tmpJson;
    ASSERT_EQ(getScriptSourceReturns->ToJson()->GetObject("bytecode", &tmpJson), Result::NOT_EXIST);
}

HWTEST_F_L0(PtReturnsTest, SearchInContentReturns)
{
    auto result = std::vector<std::unique_ptr<SearchMatch>>();
    std::unique_ptr<SearchMatch> tmpResult = std::make_unique<SearchMatch>();
    tmpResult->SetLine(13);
    tmpResult->SetLineContent("line-10");
    result.emplace_back(std::move(tmpResult));
    std::unique_ptr<SearchInContentReturns> searchInContentReturns
                    = std::make_unique<SearchInContentReturns>(std::move(result));
    ASSERT_NE(searchInContentReturns, nullptr);

    std::unique_ptr<PtJson> json;
    ASSERT_EQ(searchInContentReturns->ToJson()->GetArray("result", &json), Result::SUCCESS);
    ASSERT_NE(json, nullptr);
    EXPECT_EQ(json->GetSize(), 1);
}

HWTEST_F_L0(PtReturnsTest, SetScriptSourceReturns)
{
    auto callFrames = std::vector<std::unique_ptr<CallFrame>>();
    std::unique_ptr<CallFrame> callFrame = std::make_unique<CallFrame>();
    std::unique_ptr<Location> location = std::make_unique<Location>();
    location->SetScriptId(3).SetLine(36);
    std::unique_ptr<RemoteObject> res = std::make_unique<RemoteObject>();
    res->SetType("idle5");

    callFrame->SetCallFrameId(33);
    callFrame->SetLocation(std::move(location));
    callFrame->SetThis(std::move(res));
    callFrames.emplace_back(std::move(callFrame));

    std::unique_ptr<ExceptionDetails> exceptionDetails = std::make_unique<ExceptionDetails>();
    exceptionDetails->SetExceptionId(12);
    std::unique_ptr<SetScriptSourceReturns> setScriptSourceReturns
                   = std::make_unique <SetScriptSourceReturns>(std::move(callFrames),
                                    true, std::move(exceptionDetails));
    ASSERT_NE(setScriptSourceReturns, nullptr);

    std::unique_ptr<PtJson> callFramesJson;
    ASSERT_EQ(setScriptSourceReturns->ToJson()->GetArray("callFrames", &callFramesJson), Result::SUCCESS);
    ASSERT_NE(callFramesJson, nullptr);
    EXPECT_EQ(callFramesJson->GetSize(), 1);
    bool tmpStackChanged;
    ASSERT_EQ(setScriptSourceReturns->ToJson()->GetBool("stackChanged", &tmpStackChanged), Result::SUCCESS);
    ASSERT_TRUE(tmpStackChanged);
    std::unique_ptr<PtJson> exceptionDetailJson;
    ASSERT_EQ(setScriptSourceReturns->ToJson()->GetObject("exceptionDetails", &exceptionDetailJson), Result::SUCCESS);

    std::unique_ptr<SetScriptSourceReturns> setScriptSource = std::make_unique<SetScriptSourceReturns>(std::nullopt);
    ASSERT_NE(setScriptSource, nullptr);

    std::unique_ptr<PtJson> callJson;
    ASSERT_EQ(setScriptSource->ToJson()->GetArray("callFrames", &callJson), Result::NOT_EXIST);
    bool stack;
    ASSERT_EQ(setScriptSource->ToJson()->GetBool("stackChanged", &stack), Result::NOT_EXIST);
    std::unique_ptr<PtJson> exceptionJson;
    ASSERT_EQ(setScriptSource->ToJson()->GetObject("exceptionDetails", &exceptionJson), Result::NOT_EXIST);
}

HWTEST_F_L0(PtReturnsTest, GetPropertiesReturns)
{
    auto descriptor = std::vector<std::unique_ptr<panda::ecmascript::tooling::PropertyDescriptor>>();
    std::unique_ptr<panda::ecmascript::tooling::PropertyDescriptor> propertyDescriptor
                    = std::make_unique<panda::ecmascript::tooling::PropertyDescriptor>();
    propertyDescriptor->SetName("filename1").SetConfigurable(true).SetEnumerable(true);
    descriptor.emplace_back(std::move(propertyDescriptor));

    auto internalPropertyDescripties = std::vector<std::unique_ptr<InternalPropertyDescriptor>>();
    std::unique_ptr<InternalPropertyDescriptor> internalPropertyDescriptor
                            = std::make_unique<InternalPropertyDescriptor>();
    internalPropertyDescriptor->SetName("filename2");
    internalPropertyDescripties.emplace_back(std::move(internalPropertyDescriptor));

    auto privateProperties = std::vector<std::unique_ptr<PrivatePropertyDescriptor>>();
    std::unique_ptr<PrivatePropertyDescriptor> privatePropertyDescriptor
                            = std::make_unique<PrivatePropertyDescriptor>();
    privatePropertyDescriptor->SetName("filename3");
    privateProperties.emplace_back(std::move(privatePropertyDescriptor));

    std::unique_ptr<ExceptionDetails> exceptionDetails = std::make_unique<ExceptionDetails>();
    exceptionDetails->SetExceptionId(12);
    std::unique_ptr<GetPropertiesReturns> getPropertiesReturns
                    = std::make_unique<GetPropertiesReturns>(std::move(descriptor),
                      std::move(internalPropertyDescripties),
                      std::move(privateProperties),
                      std::move(exceptionDetails));
    ASSERT_NE(getPropertiesReturns, nullptr);

    std::unique_ptr<PtJson> resultJson;
    ASSERT_EQ(getPropertiesReturns->ToJson()->GetArray("result", &resultJson), Result::SUCCESS);
    ASSERT_NE(resultJson, nullptr);
    EXPECT_EQ(resultJson->GetSize(), 1);

    std::unique_ptr<PtJson> internalJson;
    ASSERT_EQ(getPropertiesReturns->ToJson()->GetArray("internalProperties", &internalJson), Result::SUCCESS);
    ASSERT_NE(internalJson, nullptr);
    EXPECT_EQ(internalJson->GetSize(), 1);

    std::unique_ptr<PtJson> privateJson;
    ASSERT_EQ(getPropertiesReturns->ToJson()->GetArray("privateProperties", &privateJson), Result::SUCCESS);
    ASSERT_NE(privateJson, nullptr);
    EXPECT_EQ(privateJson->GetSize(), 1);

    std::unique_ptr<PtJson> tmpJson;
    ASSERT_EQ(getPropertiesReturns->ToJson()->GetObject("exceptionDetails", &tmpJson), Result::SUCCESS);
}

HWTEST_F_L0(PtReturnsTest, GetBestEffortCoverageReturns)
{
    auto coverages = std::vector<std::unique_ptr<Coverage>>();
    std::unique_ptr<Coverage> coverage = std::make_unique<Coverage>();
    coverage->SetStartOffset(0);
    coverage->SetEndOffset(10);
    coverage->SetCount(5);
    coverages.emplace_back(std::move(coverage));

    auto functions = std::vector<std::unique_ptr<FunctionCoverage>>();
    std::unique_ptr<FunctionCoverage> functionCoverage = std::make_unique<FunctionCoverage>();
    functionCoverage->SetFunctionName("functionName1");
    functionCoverage->SetisBlockCoverage(true);
    functionCoverage->SetFunctions(std::move(coverages));

    auto result = std::vector<std::unique_ptr<ScriptCoverage>>();
    std::unique_ptr<ScriptCoverage> scriptCoverage = std::make_unique<ScriptCoverage>();
    scriptCoverage->SetScriptId("13");
    scriptCoverage->SetUrl("url13");
    scriptCoverage->SetFunctions(std::move(functions));
    result.emplace_back(std::move(scriptCoverage));

    std::unique_ptr<GetBestEffortCoverageReturns> getBestEffortCoverageReturns
                    = std::make_unique<GetBestEffortCoverageReturns>(std::move(result));

    std::unique_ptr<PtJson> json;
    ASSERT_EQ(getBestEffortCoverageReturns->ToJson()->GetArray("result", &json), Result::SUCCESS);
    ASSERT_NE(json, nullptr);
    EXPECT_EQ(json->GetSize(), 1);
}

HWTEST_F_L0(PtReturnsTest, TakePreciseCoverageReturns)
{
    auto coverages = std::vector<std::unique_ptr<Coverage>>();
    std::unique_ptr<Coverage> coverage = std::make_unique<Coverage>();
    coverage->SetStartOffset(0);
    coverage->SetEndOffset(10);
    coverage->SetCount(5);
    coverages.emplace_back(std::move(coverage));

    auto functions = std::vector<std::unique_ptr<FunctionCoverage>>();
    std::unique_ptr<FunctionCoverage> functionCoverage = std::make_unique<FunctionCoverage>();
    functionCoverage->SetFunctionName("functionName1");
    functionCoverage->SetisBlockCoverage(true);
    functionCoverage->SetFunctions(std::move(coverages));

    auto result = std::vector<std::unique_ptr<ScriptCoverage>>();
    std::unique_ptr<ScriptCoverage> scriptCoverage = std::make_unique<ScriptCoverage>();
    scriptCoverage->SetScriptId("13");
    scriptCoverage->SetUrl("url13");
    scriptCoverage->SetFunctions(std::move(functions));
    result.emplace_back(std::move(scriptCoverage));

    std::unique_ptr<TakePreciseCoverageReturns> takePreciseCoverageReturns
                        = std::make_unique<TakePreciseCoverageReturns>(std::move(result), 1001);
    ASSERT_NE(takePreciseCoverageReturns, nullptr);

    std::unique_ptr<PtJson> json;
    ASSERT_EQ(takePreciseCoverageReturns->ToJson()->GetArray("result", &json), Result::SUCCESS);
    ASSERT_NE(json, nullptr);
    EXPECT_EQ(json->GetSize(), 1);
}

HWTEST_F_L0(PtReturnsTest, TakeTypeProfileReturns)
{
    auto types = std::vector<std::unique_ptr<TypeObject>>();
    std::unique_ptr<TypeObject> typeObject = std::make_unique<TypeObject>();
    typeObject->SetName("object1");

    auto entries = std::vector<std::unique_ptr<TypeProfileEntry>>();
    std::unique_ptr<TypeProfileEntry> typeProfileEntry = std::make_unique<TypeProfileEntry>();
    typeProfileEntry->SetOffset(4);
    typeProfileEntry->SetTypes(std::move(types));

    auto result = std::vector<std::unique_ptr<ScriptTypeProfile>>();
    std::unique_ptr<ScriptTypeProfile> scriptTypeProfile = std::make_unique<ScriptTypeProfile>();
    scriptTypeProfile->SetUrl("url5");
    scriptTypeProfile->SetScriptId("id5");
    scriptTypeProfile->SetEntries(std::move(entries));
    result.emplace_back(std::move(scriptTypeProfile));

    std::unique_ptr<TakeTypeProfileReturns> takeTypeProfileturns
                    = std::make_unique<TakeTypeProfileReturns>(std::move(result));

    std::unique_ptr<PtJson> json;
    ASSERT_EQ(takeTypeProfileturns->ToJson()->GetArray("result", &json), Result::SUCCESS);
    ASSERT_NE(json, nullptr);
    EXPECT_EQ(json->GetSize(), 1);
}

HWTEST_F_L0(PtReturnsTest, GetCategoriesReturns)
{
    auto result = std::vector<std::string>();
    std::string str = "getCategories";
    result.emplace_back(str);
    std::unique_ptr<GetCategoriesReturns> getCategoriesReturns = std::make_unique
                                                    <GetCategoriesReturns>(std::move(result));

    std::unique_ptr<PtJson> json;
    ASSERT_EQ(getCategoriesReturns->ToJson()->GetArray("categories", &json), Result::SUCCESS);
    ASSERT_NE(json, nullptr);
    EXPECT_EQ(json->GetSize(), 1);
}

HWTEST_F_L0(PtReturnsTest, GetHeapUsageReturnsCreateTest)
{
    std::string msg;
    std::unique_ptr<GetHeapUsageReturns> heapUsageReturns;

    msg = std::string() + R"({})";
    heapUsageReturns = GetHeapUsageReturns::Create(DispatchRequest(msg).GetParams());
    EXPECT_EQ(heapUsageReturns, nullptr);

    msg = std::string() + R"({"id":0,"method":"PtReturns.Test","params":{"usedSize":20,"totalSize":20}})";
    heapUsageReturns = GetHeapUsageReturns::Create(DispatchRequest(msg).GetParams());
    ASSERT_TRUE(heapUsageReturns != nullptr);
}
}