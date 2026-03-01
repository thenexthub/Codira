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
#include "dispatcher.h"
#include "ecmascript/tests/test_helper.h"
#include "protocol_handler.h"

using namespace panda::ecmascript;
using namespace panda::ecmascript::tooling;

namespace panda::test {
class DispatcherTest : public testing::Test {
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

HWTEST_F_L0(DispatcherTest, DispatchRequestTest)
{
    std::string msg = "";
    auto dispatchRequest = std::make_unique<DispatchRequest>(msg);
    bool result = dispatchRequest->IsValid();
    ASSERT_TRUE(!result);
    msg = std::string() + R"({"id":0,"method":"Tracing.xxx","params":{}})";
    auto dispatchRequest1 = std::make_unique<DispatchRequest>(msg);
    result  = dispatchRequest1->IsValid();
    ASSERT_TRUE(result);
    msg = std::string() + R"("":0)";
    auto dispatchRequest2 = std::make_unique<DispatchRequest>(msg);
    result = dispatchRequest2->IsValid();
    ASSERT_TRUE(!result);
    msg = std::string() + R"({"method":"Tracing","params":{}})";
    auto dispatchRequest3 = std::make_unique<DispatchRequest>(msg);
    result = dispatchRequest3->IsValid();
    msg = std::string() + R"({"id":0,"params":{}})";
    auto dispatchRequest9 = std::make_unique<DispatchRequest>(msg);
    result = dispatchRequest9->IsValid();
    ASSERT_TRUE(!result);
    msg = std::string() + R"({"id":0,"method":"Tracing","params":{}})";
    auto dispatchRequest4 = std::make_unique<DispatchRequest>(msg);
    result = dispatchRequest4->IsValid();
    ASSERT_TRUE(!result);
    msg = std::string() + R"({"id":0,"method":".Tracing","params":{}})";
    auto dispatchRequest5 = std::make_unique<DispatchRequest>(msg);
    result = dispatchRequest5->IsValid();
    ASSERT_TRUE(!result);
    msg = std::string() + R"({"id":0,"method":"Tracing.","params":{}})";
    auto dispatchRequest6 = std::make_unique<DispatchRequest>(msg);
    result = dispatchRequest6->IsValid();
    ASSERT_TRUE(!result);
    msg = std::string() + R"({"id":0,"method":"Tracing.end", "params":1})";
    auto dispatchRequest7 = std::make_unique<DispatchRequest>(msg);
    result = dispatchRequest7->IsValid();
    ASSERT_TRUE(!result);
    msg = std::string() + R"({"id":0,"method":"Tracing.end"})";
    auto dispatchRequest8 = std::make_unique<DispatchRequest>(msg);
    result = dispatchRequest8->IsValid();
    ASSERT_TRUE(result);
}

HWTEST_F_L0(DispatcherTest, IsValidTest)
{
    std::string msg = std::string() + R"({"id":0,"method":"Tracing.end","params":{}})";
    auto dispatchRequest = std::make_unique<DispatchRequest>(msg);
    bool result = dispatchRequest->IsValid();
    ASSERT_TRUE(result);
}

HWTEST_F_L0(DispatcherTest, GetDomainTest)
{
    std::string msg = std::string() + R"({"id":0,"method":"Tracing.end","params":{}})";
    auto dispatchRequest = std::make_unique<DispatchRequest>(msg);
    std::string result = dispatchRequest->GetDomain();
    ASSERT_TRUE(result == "Tracing");
}

HWTEST_F_L0(DispatcherTest, DispatchResponseCreateTest)
{
    ResponseCode code = ResponseCode::OK;
    std::string msg = "";
    DispatchResponse response = DispatchResponse::Create(code, msg);
    ASSERT_TRUE(response.IsOk());
    ASSERT_TRUE(response.GetMessage() == "");

    msg = "msg";
    response = DispatchResponse::Create(msg);
    ASSERT_TRUE(!response.IsOk());
    ASSERT_TRUE(response.GetMessage() == "msg");

    std::optional<std::string> error;
    response = DispatchResponse::Create(error);
    ASSERT_TRUE(response.IsOk());
    ASSERT_TRUE(response.GetMessage() == "");
}

HWTEST_F_L0(DispatcherTest, DispatcherDispatchTest)
{
    std::string msg = "";
    DispatchRequest dispatchRequest(msg);
    msg = std::string() + R"({"id":0,"method":"Tracing.end","params":{}})";
    DispatchRequest dispatchRequest1(msg);
    msg = std::string() + R"({"id":0,"method":"Test.end","params":{}})";
    DispatchRequest dispatchRequest2(msg);
    std::string result = "";
    std::function<void(const void*, const std::string &)> callback =
        [&result]([[maybe_unused]] const void *ptr, const std::string &temp) { result = temp; };
    ProtocolChannel *channel =  new ProtocolHandler(callback, ecmaVm);
    auto dispatcher = std::make_unique<Dispatcher>(ecmaVm, channel);
    dispatcher->Dispatch(dispatchRequest);
    ASSERT_TRUE(result == "");
    result = "";
    dispatcher->Dispatch(dispatchRequest1);
    ASSERT_TRUE(result.find("\"id\":0,") != std::string::npos);
    result = "";
    dispatcher->Dispatch(dispatchRequest2);
    ASSERT_TRUE(result == "");
}

HWTEST_F_L0(DispatcherTest, SaveAllBreakpointsTest)
{
    ProtocolChannel *channel =  new ProtocolHandler(nullptr, ecmaVm);
    auto dispatcher = std::make_unique<Dispatcher>(ecmaVm, channel);

    std::string msg = std::string() +
        R"({
            "id": 0,
            "method": "Debugger.saveAllPossibleBreakpoints",
            "params": {
                "locations": {
                    "entry|entry|1.0.0|src/main/ets/pages/Index.ts": [{
                        "lineNumber": 59,
                        "columnNumber": 16
                    }]
                }
            }
        })";
    DispatchRequest request(msg);
    std::optional<std::string> result = dispatcher->Dispatch(request, true);
    EXPECT_STREQ(result.value().c_str(),
        R"({"id":0,"result":{"code":1,"message":"SaveAllPossibleBreakpoints: debugger agent is not enabled"}})");
    if (channel) {
        delete channel;
        channel = nullptr;
    }
}

HWTEST_F_L0(DispatcherTest, RemoveBreakpointTest)
{
    ProtocolChannel *channel =  new ProtocolHandler(nullptr, ecmaVm);
    auto dispatcher = std::make_unique<Dispatcher>(ecmaVm, channel);

    std::string msg = std::string() +
        R"({
            "id": 0,
            "method": "Debugger.removeBreakpointsByUrl",
            "params": {
                "url": "entry|entry|1.0.0|src/main/ets/pages/Index.ts"
            }
        })";
    DispatchRequest request(msg);
    std::optional<std::string> result = dispatcher->Dispatch(request, true);
    EXPECT_STREQ(result.value().c_str(),
        R"({"id":0,"result":{"code":1,"message":"Unknown url"}})");
    if (channel) {
        delete channel;
        channel = nullptr;
    }
}

HWTEST_F_L0(DispatcherTest, SetBreakpointTest)
{
    ProtocolChannel *channel =  new ProtocolHandler(nullptr, ecmaVm);
    auto dispatcher = std::make_unique<Dispatcher>(ecmaVm, channel);

    std::string msg = std::string() +
        R"({
            "id": 0,
            "method": "Debugger.getPossibleAndSetBreakpointByUrl",
            "params": {
                "locations": [{
                    "url": "entry|entry|1.0.0|src/main/ets/pages/Index.ts",
                    "lineNumber": 59,
                    "columnNumber": 16
                }]
            }
        })";
    DispatchRequest request(msg);
    std::optional<std::string> result = dispatcher->Dispatch(request, true);
    EXPECT_STREQ(result.value().c_str(),
        R"({"id":0,"result":{"code":1,"message":"GetPossibleAndSetBreakpointByUrl: debugger agent is not enabled"}})");
    if (channel) {
        delete channel;
        channel = nullptr;
    }
}

HWTEST_F_L0(DispatcherTest, GetPropertiesTest)
{
    ProtocolChannel *channel = new ProtocolHandler(nullptr, ecmaVm);
    auto dispatcher = std::make_unique<Dispatcher>(ecmaVm, channel);
    std::optional<std::string> result;
    std::string msg = R"({"id":0,"method":"Runtime.getProperties"})";
    DispatchRequest request(msg);
    result = dispatcher->Dispatch(request, true);
    ASSERT_TRUE(result.has_value());
    ASSERT_TRUE(result.value().find("wrong params") != std::string::npos);

    msg = R"({"id":0,"method":"Runtime.getProperties","params":{"objectId":"1"}})";
    DispatchRequest request2(msg);
    result = dispatcher->Dispatch(request2, true);
    ASSERT_TRUE(result.has_value());
    ASSERT_TRUE(result.value().find("Unknown object id") != std::string::npos);
    if (channel) {
        delete channel;
        channel = nullptr;
    }
}

HWTEST_F_L0(DispatcherTest, CallFunctionOnTest)
{
    ProtocolChannel *channel = new ProtocolHandler(nullptr, ecmaVm);
    auto dispatcher = std::make_unique<Dispatcher>(ecmaVm, channel);
    std::optional<std::string> result;
    std::string msg = R"({"id":0,"method":"Debugger.callFunctionOn"})";
    DispatchRequest request3(msg);
    result = dispatcher->Dispatch(request3, true);
    ASSERT_TRUE(result.has_value());
    ASSERT_TRUE(result.value().find("wrong params") != std::string::npos);

    msg = std::string() + R"({"id":0,"method":"Debugger.callFunctionOn","params":{
        "callFrameId":"0", "functionDeclaration":"test"}})";
    DispatchRequest request4(msg);
    result = dispatcher->Dispatch(request4, true);
    ASSERT_TRUE(result.has_value());
    EXPECT_STREQ(result.value().c_str(), R"({"id":0,"result":{"code":1,"message":"Invalid callFrameId."}})");
    if (channel) {
        delete channel;
        channel = nullptr;
    }
}

HWTEST_F_L0(DispatcherTest, EvaluateOnCallFrameTest)
{
    ProtocolChannel *channel = new ProtocolHandler(nullptr, ecmaVm);
    auto dispatcher = std::make_unique<Dispatcher>(ecmaVm, channel);
    std::optional<std::string> result;
    std::string msg = R"({"id":0,"method":"Debugger.evaluateOnCallFrame"})";
    DispatchRequest request5(msg);
    result = dispatcher->Dispatch(request5, true);
    ASSERT_TRUE(result.has_value());
    ASSERT_TRUE(result.value().find("wrong params") != std::string::npos);

    msg = std::string() +
        R"({
            "id":0,
            "method":"Debugger.evaluateOnCallFrame",
            "params":{
                "callFrameId":"-1",
                "expression":"the expression"
            }
        })";
    DispatchRequest request6(msg);
    result = dispatcher->Dispatch(request6, true);
    ASSERT_TRUE(result.has_value());
    EXPECT_STREQ(result.value().c_str(), R"({"id":0,"result":{"code":1,"message":"Invalid callFrameId."}})");
    if (channel) {
        delete channel;
        channel = nullptr;
    }
}
}  // namespace panda::test