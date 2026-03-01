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

#include "agent/runtime_impl.h"
#include "ecmascript/tests/test_helper.h"
#include "tooling/dynamic/protocol_handler.h"

using namespace panda::ecmascript;
using namespace panda::ecmascript::tooling;


namespace panda::test {
class RuntimeImplTest : public testing::Test {
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

HWTEST_F_L0(RuntimeImplTest, DispatcherImplDispatch)
{
    std::string result = "";
    std::function<void(const void*, const std::string &)> callback =
        [&result]([[maybe_unused]] const void *ptr, const std::string &temp) {result = temp;};
    ProtocolChannel *channel = new ProtocolHandler(callback, ecmaVm);
    auto runtimeImpl = std::make_unique<RuntimeImpl>(ecmaVm, channel);
    auto dispatcherImpl = std::make_unique<RuntimeImpl::DispatcherImpl>(channel, std::move(runtimeImpl));
    std::string msg = std::string() + R"({"id":0,"method":"Rumtime.test","params":{}})";
    DispatchRequest request(msg);
    dispatcherImpl->Dispatch(request);
    ASSERT_TRUE(result.find("unknown method: test") != std::string::npos);

    msg = std::string() + R"({"id":0,"method":"Rumtime.enable","params":{}})";
    DispatchRequest request1(msg);
    dispatcherImpl->Dispatch(request1);
    ASSERT_TRUE(result.find("protocols") != std::string::npos);
    if (channel != nullptr) {
        delete channel;
        channel = nullptr;
    }
}

HWTEST_F_L0(RuntimeImplTest, DispatcherImplEnable)
{
    std::string result = "";
    std::function<void(const void*, const std::string &)> callback =
        [&result]([[maybe_unused]] const void *ptr, const std::string &temp) {result = temp;};
    ProtocolChannel *channel = new ProtocolHandler(callback, ecmaVm);
    auto runtimeImpl = std::make_unique<RuntimeImpl>(ecmaVm, channel);
    auto dispatcherImpl = std::make_unique<RuntimeImpl::DispatcherImpl>(channel, std::move(runtimeImpl));

    std::string msg = std::string() + R"({"id":0,"method":"Rumtime.enable","params":{}})";
    std::unique_ptr<PtBaseReturns> res = nullptr;
    DispatchRequest request1(msg);
    DispatchResponse response = dispatcherImpl->Enable(request1, res);
    channel->SendResponse(request1, response, *res);
    ASSERT_TRUE(result.find("protocols") != std::string::npos);
    if (channel != nullptr) {
        delete channel;
        channel = nullptr;
    }
}

HWTEST_F_L0(RuntimeImplTest, DispatcherImplDisable)
{
    std::string result = "";
    std::function<void(const void*, const std::string &)> callback =
        [&result]([[maybe_unused]] const void *ptr, const std::string &temp) {result = temp;};
    ProtocolChannel *channel = new ProtocolHandler(callback, ecmaVm);
    auto runtimeImpl = std::make_unique<RuntimeImpl>(ecmaVm, channel);
    auto dispatcherImpl = std::make_unique<RuntimeImpl::DispatcherImpl>(channel, std::move(runtimeImpl));

    std::string msg = std::string() + R"({"id":0,"method":"Rumtime.disable","params":{}})";
    DispatchRequest request1(msg);
    DispatchResponse response = dispatcherImpl->Disable(request1);
    channel->SendResponse(request1, response, PtBaseReturns());
    ASSERT_TRUE(result == "{\"id\":0,\"result\":{}}");
    if (channel != nullptr) {
        delete channel;
        channel = nullptr;
    }
}

HWTEST_F_L0(RuntimeImplTest, DispatcherImplRunIfWaitingForDebugger)
{
    std::string result = "";
    std::function<void(const void*, const std::string &)> callback =
        [&result]([[maybe_unused]] const void *ptr, const std::string &temp) {result = temp;};
    ProtocolChannel *channel = new ProtocolHandler(callback, ecmaVm);
    auto runtimeImpl = std::make_unique<RuntimeImpl>(ecmaVm, channel);
    auto dispatcherImpl = std::make_unique<RuntimeImpl::DispatcherImpl>(channel, std::move(runtimeImpl));

    std::string msg = std::string() + R"({"id":0,"method":"Rumtime.runIfWaitingForDebugger","params":{}})";
    DispatchRequest request1(msg);
    DispatchResponse response = dispatcherImpl->RunIfWaitingForDebugger(request1);
    channel->SendResponse(request1, response, PtBaseReturns());
    ASSERT_TRUE(result == "{\"id\":0,\"result\":{}}");
    if (channel != nullptr) {
        delete channel;
        channel = nullptr;
    }
}

HWTEST_F_L0(RuntimeImplTest, DispatcherImplGetProperties__001)
{
    std::string result = "";
    std::function<void(const void*, const std::string &)> callback =
        [&result]([[maybe_unused]] const void *ptr, const std::string &temp) {result = temp;};
    ProtocolChannel *channel = new ProtocolHandler(callback, ecmaVm);
    auto runtimeImpl = std::make_unique<RuntimeImpl>(ecmaVm, channel);
    auto dispatcherImpl = std::make_unique<RuntimeImpl::DispatcherImpl>(channel, std::move(runtimeImpl));

    std::string msg = std::string() + R"({"id":0,"method":"Rumtime.getProperties","params":{"objectId":0}})";
    DispatchRequest request(msg);
    std::unique_ptr<PtBaseReturns> res = nullptr;
    DispatchResponse response = dispatcherImpl->GetProperties(request, res);
    channel->SendResponse(request, response, *res);
    ASSERT_TRUE(result.find("wrong params") != std::string::npos);

    msg = std::string() + R"({"id":0,"method":"Rumtime.getProperties","params":{"objectId":"0"}})";
    DispatchRequest request1(msg);
    response = dispatcherImpl->GetProperties(request1, res);
    channel->SendResponse(request1, response, *res);
    ASSERT_TRUE(result.find("Unknown object id") != std::string::npos);
    if (channel != nullptr) {
        delete channel;
        channel = nullptr;
    }
}

HWTEST_F_L0(RuntimeImplTest, DispatcherImplGetProperties__002)
{
    ProtocolChannel *channel = new ProtocolHandler(nullptr, ecmaVm);
    auto runtimeImpl = std::make_unique<RuntimeImpl>(ecmaVm, channel);
    auto dispatcherImpl = std::make_unique<RuntimeImpl::DispatcherImpl>(channel, std::move(runtimeImpl));
    std::string msg = std::string() + R"({"id":0,"method":"Rumtime.getProperties","params":{"objectId":"0"}})";
    DispatchRequest request(msg);
    std::unique_ptr<PtBaseReturns> resultPtr;
    DispatchResponse response = dispatcherImpl->GetProperties(request, resultPtr);
    EXPECT_STREQ(response.GetMessage().c_str(), "Unknown object id");
    if (channel != nullptr) {
        delete channel;
        channel = nullptr;
    }
}

HWTEST_F_L0(RuntimeImplTest, DispatcherImplGetHeapUsage)
{
    std::string result = "";
    std::function<void(const void*, const std::string &)> callback =
        [&result]([[maybe_unused]] const void *ptr, const std::string &temp) {result = temp;};
    ProtocolChannel *channel = new ProtocolHandler(callback, ecmaVm);
    auto runtimeImpl = std::make_unique<RuntimeImpl>(ecmaVm, channel);
    auto dispatcherImpl = std::make_unique<RuntimeImpl::DispatcherImpl>(channel, std::move(runtimeImpl));

    std::string msg = std::string() + R"({"id":0,"method":"Rumtime.getHeapUsage ","params":{}})";
    DispatchRequest request(msg);
    std::unique_ptr<PtBaseReturns> res = nullptr;
    DispatchResponse response = dispatcherImpl->GetHeapUsage(request, res);
    channel->SendResponse(request, response, *res);
    ASSERT_TRUE(result.find("usedSize") != std::string::npos);
    ASSERT_TRUE(result.find("totalSize") != std::string::npos);
    if (channel != nullptr) {
        delete channel;
        channel = nullptr;
    }
}

HWTEST_F_L0(RuntimeImplTest, DispatcherImplDispatchDisable)
{
    std::string result = "";
    std::function<void(const void*, const std::string &)> callback =
        [&result]([[maybe_unused]] const void *ptr, const std::string &temp) {result = temp;};
    ProtocolChannel *channel = new ProtocolHandler(callback, ecmaVm);
    auto runtimeImpl = std::make_unique<RuntimeImpl>(ecmaVm, channel);
    auto dispatcherImpl = std::make_unique<RuntimeImpl::DispatcherImpl>(channel, std::move(runtimeImpl));
    std::string msg = std::string() + R"({"id":0,"method":"Rumtime.disable","params":{}})";
    DispatchRequest request1(msg);
    dispatcherImpl->Dispatch(request1);
    ASSERT_TRUE(result == "{\"id\":0,\"result\":{}}");
    if (channel != nullptr) {
        delete channel;
        channel = nullptr;
    }
}

HWTEST_F_L0(RuntimeImplTest, DispatcherImplDispatchRunIfWaitingForDebugger)
{
    std::string result = "";
    std::function<void(const void*, const std::string &)> callback =
        [&result]([[maybe_unused]] const void *ptr, const std::string &temp) {result = temp;};
    ProtocolChannel *channel = new ProtocolHandler(callback, ecmaVm);
    auto runtimeImpl = std::make_unique<RuntimeImpl>(ecmaVm, channel);
    auto dispatcherImpl = std::make_unique<RuntimeImpl::DispatcherImpl>(channel, std::move(runtimeImpl));
    std::string msg = std::string() + R"({"id":0,"method":"Rumtime.runIfWaitingForDebugger","params":{}})";
    DispatchRequest request1(msg);
    dispatcherImpl->Dispatch(request1);
    ASSERT_TRUE(result == "{\"id\":0,\"result\":{}}");
    if (channel != nullptr) {
        delete channel;
        channel = nullptr;
    }
}

HWTEST_F_L0(RuntimeImplTest, DispatcherImplDispatchGetHeapUsage)
{
    std::string result = "";
    std::function<void(const void*, const std::string &)> callback =
        [&result]([[maybe_unused]] const void *ptr, const std::string &temp) {result = temp;};
    ProtocolChannel *channel = new ProtocolHandler(callback, ecmaVm);
    auto runtimeImpl = std::make_unique<RuntimeImpl>(ecmaVm, channel);
    auto dispatcherImpl = std::make_unique<RuntimeImpl::DispatcherImpl>(channel, std::move(runtimeImpl));
    std::string msg = std::string() + R"({"id":0,"method":"Rumtime.getHeapUsage","params":{}})";
    DispatchRequest request1(msg);
    dispatcherImpl->Dispatch(request1);
    ASSERT_FALSE(result.empty());
    ASSERT_TRUE(result.find("usedSize") != std::string::npos);
    if (channel != nullptr) {
        delete channel;
        channel = nullptr;
    }
}

}  // namespace panda::test
