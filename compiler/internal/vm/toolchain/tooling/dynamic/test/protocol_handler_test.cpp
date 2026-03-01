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

#include "ecmascript/tests/test_helper.h"
#include "protocol_handler.h"

using namespace panda::ecmascript;
using namespace panda::ecmascript::tooling;

namespace panda::test {
class ProtocolHandlerTest : public testing::Test {
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

HWTEST_F_L0(ProtocolHandlerTest, DispatchCommandTest)
{
    std::string result = "";
    std::function<void(const void*, const std::string &)> callback =
        [&result]([[maybe_unused]] const void *ptr, const std::string &temp) { result = temp; };
    auto protocol = std::make_unique<ProtocolHandler>(callback, ecmaVm);
    std::string msg = std::string() + R"({"id":0,"method":"Tracing.Test","params":{}})";
    protocol->DispatchCommand(msg + "");
    protocol->ProcessCommand();
    ASSERT_TRUE(result.find("Unknown method: Test") != std::string::npos);
}

HWTEST_F_L0(ProtocolHandlerTest, GetDispatchStatusTest)
{
    std::string result = "";
    std::function<void(const void*, const std::string &)> callback =
        [&result]([[maybe_unused]] const void *ptr, const std::string &temp) { result = temp; };
    auto protocol = std::make_unique<ProtocolHandler>(callback, ecmaVm);
    std::string msg = std::string() + R"({"id":0,"method":"Tracing.Test","params":{}})";
    ProtocolHandler::DispatchStatus status = ProtocolHandler::DispatchStatus(protocol->GetDispatchStatus());
    ASSERT_TRUE(status == ProtocolHandler::DispatchStatus::DISPATCHED);
    protocol->DispatchCommand(msg + "");
    status = ProtocolHandler::DispatchStatus(protocol->GetDispatchStatus());
    ASSERT_TRUE(status == ProtocolHandler::DispatchStatus::UNKNOWN);
}

HWTEST_F_L0(ProtocolHandlerTest, ProcessCommandTest)
{
    std::string result = "";
    std::function<void(const void*, const std::string &)> callback =
        [&result]([[maybe_unused]] const void *ptr, const std::string &temp) { result = temp; };
    auto protocol = std::make_unique<ProtocolHandler>(callback, ecmaVm);
    std::string msg = std::string() + R"({"id":0,"method":"Tracing.Test","params":{}})";
    protocol->ProcessCommand();
    ASSERT_TRUE(result == "");
    protocol->DispatchCommand(msg + "");
    protocol->ProcessCommand();
    ASSERT_TRUE(result.find("Unknown method: Test") != std::string::npos);
}

HWTEST_F_L0(ProtocolHandlerTest, SendResponseTest)
{
    std::string result = "";
    std::function<void(const void*, const std::string &)> callback =
        [&result]([[maybe_unused]] const void *ptr, const std::string &temp) { result = temp; };
    auto protocol = std::make_unique<ProtocolHandler>(callback, ecmaVm);
    std::string msg = std::string() + R"({"id":0,"method":"Tracing.Test","params":{}})";
    DispatchRequest request(msg);
    ResponseCode code = ResponseCode::NOK;
    msg = "error";
    DispatchResponse response = DispatchResponse::Create(code, msg);
    PtBaseReturns returns;
    protocol->SendResponse(request, response, returns);
    ASSERT_TRUE(result == "{\"id\":0,\"result\":{\"code\":1,\"message\":\"error\"}}");
    code = ResponseCode::OK;
    DispatchResponse response1 = DispatchResponse::Create(code, msg);
    protocol->SendResponse(request, response1, returns);
    ASSERT_TRUE(result == "{\"id\":0,\"result\":{}}");
}
}  // namespace panda::test