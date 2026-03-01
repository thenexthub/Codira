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

#include "agent/dom_impl.h"
#include "ecmascript/tests/test_helper.h"
#include "protocol_handler.h"

using namespace panda::ecmascript;
using namespace panda::ecmascript::tooling;

namespace panda::test {
class DomImplTest : public testing::Test {
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

HWTEST_F_L0(DomImplTest, DispatchTest_001)
{
    std::string outStrForCallbackCheck = "";
    std::function<void(const void*, const std::string &)> callback =
        [&outStrForCallbackCheck]([[maybe_unused]] const void *ptr, const std::string &inStrOfReply) {
            outStrForCallbackCheck = inStrOfReply;};
    ProtocolChannel *protocolChannel = new ProtocolHandler(callback, ecmaVm);

    auto domImpl = std::make_unique<DomImpl>();
    auto dispatcherImpl = std::make_unique<DomImpl::DispatcherImpl>(protocolChannel, std::move(domImpl));
    std::string msg = std::string() + R"({"id":0,"method":"Debugger.disable","params":{}})";
    DispatchRequest request(msg);
    dispatcherImpl->Dispatch(request);
    EXPECT_STREQ(outStrForCallbackCheck.c_str(), R"({"id":0,"result":{}})");
}

HWTEST_F_L0(DomImplTest, DispatchTest_002)
{
    std::string outStrForCallbackCheck = "";
    std::function<void(const void*, const std::string &)> callback =
        [&outStrForCallbackCheck]([[maybe_unused]] const void *ptr, const std::string &inStrOfReply) {
            outStrForCallbackCheck = inStrOfReply;};
    ProtocolChannel *protocolChannel = new ProtocolHandler(callback, ecmaVm);

    auto domImpl = std::make_unique<DomImpl>();
    auto dispatcherImpl = std::make_unique<DomImpl::DispatcherImpl>(protocolChannel, std::move(domImpl));
    std::string msg = "DomImplTestCommand";
    DispatchRequest request(msg);
    dispatcherImpl->Dispatch(request);
    EXPECT_STREQ(outStrForCallbackCheck.c_str(), R"({"id":-1,"result":{"code":1,"message":"Unknown method: "}})");
}
}  // namespace panda::test