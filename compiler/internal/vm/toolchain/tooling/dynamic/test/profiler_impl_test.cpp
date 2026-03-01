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

#include "agent/profiler_impl.h"
#include "ecmascript/tests/test_helper.h"
#include "protocol_handler.h"

using namespace panda::ecmascript;
using namespace panda::ecmascript::tooling;

namespace panda::test {
class ProfilerImplTest : public testing::Test {
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

HWTEST_F_L0(ProfilerImplTest, Disable)
{
    ProtocolChannel *channel = nullptr;
    auto profiler = std::make_unique<ProfilerImpl>(ecmaVm, channel);
    DispatchResponse response = profiler->Disable();
    ASSERT_TRUE(response.GetMessage() == "");
    ASSERT_TRUE(response.IsOk());
}

HWTEST_F_L0(ProfilerImplTest, Enable)
{
    ProtocolChannel *channel = nullptr;
    auto profiler = std::make_unique<ProfilerImpl>(ecmaVm, channel);
    DispatchResponse response = profiler->Enable();
    ASSERT_TRUE(response.GetMessage() == "");
    ASSERT_TRUE(response.IsOk());
}


HWTEST_F_L0(ProfilerImplTest, Start)
{
    ProtocolChannel *channel = nullptr;
    auto profiler = std::make_unique<ProfilerImpl>(ecmaVm, channel);
    DispatchResponse response = profiler->Start();
    std::unique_ptr<Profile> profile;
    DispatchResponse response1 = profiler->Stop(&profile);
    ASSERT_TRUE(response.IsOk());
    ASSERT_TRUE(response1.IsOk());
    ASSERT_TRUE(profile);
}

HWTEST_F_L0(ProfilerImplTest, Stop)
{
    ProtocolChannel *channel = nullptr;
    auto profiler = std::make_unique<ProfilerImpl>(ecmaVm, channel);
    std::unique_ptr<Profile> profile;
    DispatchResponse response = profiler->Stop(&profile);
    ASSERT_TRUE(response.GetMessage().find("Stop is failure") != std::string::npos);
    ASSERT_TRUE(!response.IsOk());
}

HWTEST_F_L0(ProfilerImplTest, SetSamplingInterval)
{
    ProtocolChannel *channel = nullptr;
    SetSamplingIntervalParams params;
    auto profiler = std::make_unique<ProfilerImpl>(ecmaVm, channel);
    DispatchResponse response = profiler->SetSamplingInterval(params);
    ASSERT_TRUE(response.GetMessage() == "");
    ASSERT_TRUE(response.IsOk());
}

HWTEST_F_L0(ProfilerImplTest, EnableSerializationTimeoutCheck)
{
    ProtocolChannel *channel = nullptr;
    SeriliazationTimeoutCheckEnableParams params;
    auto profiler = std::make_unique<ProfilerImpl>(ecmaVm, channel);
    DispatchResponse response = profiler->EnableSerializationTimeoutCheck(params);
    ASSERT_TRUE(response.GetMessage() == "");
    ASSERT_TRUE(response.IsOk());
}

HWTEST_F_L0(ProfilerImplTest, DisableSerializationTimeoutCheck)
{
    ProtocolChannel *channel = nullptr;
    auto profiler = std::make_unique<ProfilerImpl>(ecmaVm, channel);
    DispatchResponse response = profiler->DisableSerializationTimeoutCheck();
    ASSERT_TRUE(response.GetMessage() == "");
    ASSERT_TRUE(response.IsOk());
}

HWTEST_F_L0(ProfilerImplTest, GetBestEffortCoverage)
{
    ProtocolChannel *channel = nullptr;
    auto profiler = std::make_unique<ProfilerImpl>(ecmaVm, channel);
    DispatchResponse response = profiler->GetBestEffortCoverage();
    ASSERT_TRUE(response.GetMessage() == "GetBestEffortCoverage not support now");
}

HWTEST_F_L0(ProfilerImplTest, StopPreciseCoverage)
{
    ProtocolChannel *channel = nullptr;
    auto profiler = std::make_unique<ProfilerImpl>(ecmaVm, channel);
    DispatchResponse response = profiler->StopPreciseCoverage();
    ASSERT_TRUE(response.GetMessage() == "StopPreciseCoverage not support now");
}

HWTEST_F_L0(ProfilerImplTest, TakePreciseCoverage)
{
    ProtocolChannel *channel = nullptr;
    auto profiler = std::make_unique<ProfilerImpl>(ecmaVm, channel);
    DispatchResponse response = profiler->TakePreciseCoverage();
    ASSERT_TRUE(response.GetMessage() == "TakePreciseCoverage not support now");
}

HWTEST_F_L0(ProfilerImplTest, StartPreciseCoverage)
{
    ProtocolChannel *channel = nullptr;
    auto profiler = std::make_unique<ProfilerImpl>(ecmaVm, channel);
    std::string msg = std::string() + R"({"id":0,"method":"Debugger.requestMemoryDump","params":{}})";
    DispatchRequest request = DispatchRequest(msg);
    std::unique_ptr<StartPreciseCoverageParams> params = StartPreciseCoverageParams::Create(request.GetParams());
    DispatchResponse response = profiler->StartPreciseCoverage(*params);
    ASSERT_TRUE(response.GetMessage() == "StartPreciseCoverage not support now");
}

HWTEST_F_L0(ProfilerImplTest, StartTypeProfile)
{
    ProtocolChannel *channel = nullptr;
    auto profiler = std::make_unique<ProfilerImpl>(ecmaVm, channel);
    DispatchResponse response = profiler->StartTypeProfile();
    ASSERT_TRUE(response.GetMessage() == "StartTypeProfile not support now");
}

HWTEST_F_L0(ProfilerImplTest, StopTypeProfile)
{
    ProtocolChannel *channel = nullptr;
    auto profiler = std::make_unique<ProfilerImpl>(ecmaVm, channel);
    DispatchResponse response = profiler->StopTypeProfile();
    ASSERT_TRUE(response.GetMessage() == "StopTypeProfile not support now");
}

HWTEST_F_L0(ProfilerImplTest, TakeTypeProfile)
{
    ProtocolChannel *channel = nullptr;
    auto profiler = std::make_unique<ProfilerImpl>(ecmaVm, channel);
    DispatchResponse response = profiler->TakeTypeProfile();
    ASSERT_TRUE(response.GetMessage() == "TakeTypeProfile not support now");
}

HWTEST_F_L0(ProfilerImplTest, DispatcherImplDispatch)
{
    std::string result = "";
    std::function<void(const void*, const std::string &)> callback =
        [&result]([[maybe_unused]] const void *ptr, const std::string &temp) { result = temp; };
    ProtocolChannel *channel = new ProtocolHandler(callback, ecmaVm);
    auto tracing = std::make_unique<ProfilerImpl>(ecmaVm, channel);
    auto dispatcherImpl = std::make_unique<ProfilerImpl::DispatcherImpl>(channel, std::move(tracing));
    std::string msg = std::string() + R"({"id":0,"method":"Profiler.Test","params":{}})";
    DispatchRequest request(msg);
    dispatcherImpl->Dispatch(request);
    ASSERT_TRUE(result.find("Unknown method: Test") != std::string::npos);
    msg = std::string() + R"({"id":0,"method":"Profiler.disable","params":{}})";
    DispatchRequest request1 = DispatchRequest(msg);
    dispatcherImpl->Dispatch(request1);
    if (channel) {
        delete channel;
        channel = nullptr;
    }
    ASSERT_TRUE(result == "{\"id\":0,\"result\":{}}");
}

HWTEST_F_L0(ProfilerImplTest, DispatcherImplEnable)
{
    std::string result = "";
    std::function<void(const void*, const std::string &)> callback =
        [&result]([[maybe_unused]] const void *ptr, const std::string &temp) { result = temp; };
    ProtocolChannel *channel = new ProtocolHandler(callback, ecmaVm);
    auto tracing = std::make_unique<ProfilerImpl>(ecmaVm, channel);
    auto dispatcherImpl = std::make_unique<ProfilerImpl::DispatcherImpl>(channel, std::move(tracing));
    std::string msg = std::string() + R"({"id":0,"method":"Profiler.enable","params":{}})";
    DispatchRequest request(msg);
    dispatcherImpl->Dispatch(request);
    if (channel) {
        delete channel;
        channel = nullptr;
    }
    ASSERT_TRUE(result.find("protocols") != std::string::npos);
}

HWTEST_F_L0(ProfilerImplTest, DispatcherImplDisable)
{
    std::string result = "";
    std::function<void(const void*, const std::string &)> callback =
        [&result]([[maybe_unused]] const void *ptr, const std::string &temp) { result = temp; };
    ProtocolChannel *channel = new ProtocolHandler(callback, ecmaVm);
    auto tracing = std::make_unique<ProfilerImpl>(ecmaVm, channel);
    auto dispatcherImpl = std::make_unique<ProfilerImpl::DispatcherImpl>(channel, std::move(tracing));
    std::string msg = std::string() + R"({"id":0,"method":"Profiler.disable","params":{}})";
    DispatchRequest request(msg);
    dispatcherImpl->Dispatch(request);
    if (channel) {
        delete channel;
        channel = nullptr;
    }
    ASSERT_TRUE(result == "{\"id\":0,\"result\":{}}");
}

HWTEST_F_L0(ProfilerImplTest, DispatcherImplStart)
{
    std::string result = "";
    std::function<void(const void*, const std::string &)> callback =
        [&result]([[maybe_unused]] const void *ptr, const std::string &temp) { result = temp; };
    ProtocolChannel *channel = new ProtocolHandler(callback, ecmaVm);
    auto tracing = std::make_unique<ProfilerImpl>(ecmaVm, channel);
    auto dispatcherImpl = std::make_unique<ProfilerImpl::DispatcherImpl>(channel, std::move(tracing));
    std::string msg = std::string() + R"({"id":0,"method":"Profiler.start","params":{}})";
    DispatchRequest request(msg);
    dispatcherImpl->Dispatch(request);
    ASSERT_TRUE(result == "{\"id\":0,\"result\":{}}");
    msg = std::string() + R"({"id":0,"method":"Profiler.stop","params":{}})";
    DispatchRequest request1 = DispatchRequest(msg);
    dispatcherImpl->Dispatch(request1);
    if (channel) {
        delete channel;
        channel = nullptr;
    }
    ASSERT_TRUE(result.find("\"id\":0") != std::string::npos);
    ASSERT_TRUE(result.find("\"profile\"") != std::string::npos);
}

HWTEST_F_L0(ProfilerImplTest, DispatcherImplStop)
{
    std::string result = "";
    std::function<void(const void*, const std::string &)> callback =
        [&result]([[maybe_unused]] const void *ptr, const std::string &temp) { result = temp; };
    ProtocolChannel *channel = new ProtocolHandler(callback, ecmaVm);
    auto tracing = std::make_unique<ProfilerImpl>(ecmaVm, channel);
    auto dispatcherImpl = std::make_unique<ProfilerImpl::DispatcherImpl>(channel, std::move(tracing));
    std::string msg = std::string() + R"({"id":0,"method":"Profiler.stop","params":{}})";
    DispatchRequest request(msg);
    dispatcherImpl->Dispatch(request);
    if (channel) {
        delete channel;
        channel = nullptr;
    }
    ASSERT_TRUE(result.find("Stop is failure") != std::string::npos);
}

HWTEST_F_L0(ProfilerImplTest, DispatcherImplSetSamplingInterval)
{
    std::string result = "";
    std::function<void(const void*, const std::string &)> callback =
        [&result]([[maybe_unused]] const void *ptr, const std::string &temp) { result = temp; };
    ProtocolChannel *channel = new ProtocolHandler(callback, ecmaVm);
    auto tracing = std::make_unique<ProfilerImpl>(ecmaVm, channel);
    auto dispatcherImpl = std::make_unique<ProfilerImpl::DispatcherImpl>(channel, std::move(tracing));
    std::string msg = std::string() + R"({"id":0,"method":"Profiler.setSamplingInterval","params":{}})";
    DispatchRequest request(msg);
    dispatcherImpl->Dispatch(request);
    ASSERT_TRUE(result.find("wrong params") != std::string::npos);
    msg = std::string() + R"({"id":0,"method":"Profiler.setSamplingInterval","params":{"interval":24}})";
    DispatchRequest request1(msg);
    dispatcherImpl->Dispatch(request1);
    if (channel) {
        delete channel;
        channel = nullptr;
    }
    ASSERT_TRUE(result == "{\"id\":0,\"result\":{}}");
}

HWTEST_F_L0(ProfilerImplTest, DispatcherImplGetBestEffortCoverage)
{
    std::string result = "";
    std::function<void(const void*, const std::string &)> callback =
        [&result]([[maybe_unused]] const void *ptr, const std::string &temp) { result = temp; };
    ProtocolChannel *channel = new ProtocolHandler(callback, ecmaVm);
    auto tracing = std::make_unique<ProfilerImpl>(ecmaVm, channel);
    auto dispatcherImpl = std::make_unique<ProfilerImpl::DispatcherImpl>(channel, std::move(tracing));
    std::string msg = std::string() + R"({"id":0,"method":"Profiler.getBestEffortCoverage","params":{}})";
    DispatchRequest request(msg);
    dispatcherImpl->Dispatch(request);
    if (channel) {
        delete channel;
        channel = nullptr;
    }
    ASSERT_TRUE(result.find("GetBestEffortCoverage not support now") != std::string::npos);
}

HWTEST_F_L0(ProfilerImplTest, DispatcherImplStopPreciseCoverage)
{
    std::string result = "";
    std::function<void(const void*, const std::string &)> callback =
        [&result]([[maybe_unused]] const void *ptr, const std::string &temp) { result = temp; };
    ProtocolChannel *channel = new ProtocolHandler(callback, ecmaVm);
    auto tracing = std::make_unique<ProfilerImpl>(ecmaVm, channel);
    auto dispatcherImpl = std::make_unique<ProfilerImpl::DispatcherImpl>(channel, std::move(tracing));
    std::string msg = std::string() + R"({"id":0,"method":"Profiler.stopPreciseCoverage","params":{}})";
    DispatchRequest request(msg);
    dispatcherImpl->Dispatch(request);
    if (channel) {
        delete channel;
        channel = nullptr;
    }
    ASSERT_TRUE(result.find("StopPreciseCoverage not support now") != std::string::npos);
}

HWTEST_F_L0(ProfilerImplTest, DispatcherImplTakePreciseCoverage)
{
    std::string result = "";
    std::function<void(const void*, const std::string &)> callback =
        [&result]([[maybe_unused]] const void *ptr, const std::string &temp) { result = temp; };
    ProtocolChannel *channel = new ProtocolHandler(callback, ecmaVm);
    auto tracing = std::make_unique<ProfilerImpl>(ecmaVm, channel);
    auto dispatcherImpl = std::make_unique<ProfilerImpl::DispatcherImpl>(channel, std::move(tracing));
    std::string msg = std::string() + R"({"id":0,"method":"Profiler.takePreciseCoverage","params":{}})";
    DispatchRequest request(msg);
    dispatcherImpl->Dispatch(request);
    if (channel) {
        delete channel;
        channel = nullptr;
    }
    ASSERT_TRUE(result.find("TakePreciseCoverage not support now") != std::string::npos);
}

HWTEST_F_L0(ProfilerImplTest, DispatcherImplStartPreciseCoverage)
{
    std::string result = "";
    std::function<void(const void*, const std::string &)> callback =
        [&result]([[maybe_unused]] const void *ptr, const std::string &temp) { result = temp; };
    ProtocolChannel *channel = new ProtocolHandler(callback, ecmaVm);
    auto tracing = std::make_unique<ProfilerImpl>(ecmaVm, channel);
    auto dispatcherImpl = std::make_unique<ProfilerImpl::DispatcherImpl>(channel, std::move(tracing));
    std::string msg = std::string() + R"({"id":0,"method":"Profiler.startPreciseCoverage","params":{}})";
    DispatchRequest request(msg);
    dispatcherImpl->Dispatch(request);
    if (channel) {
        delete channel;
        channel = nullptr;
    }
    ASSERT_TRUE(result.find("StartPreciseCoverage not support now") != std::string::npos);
}

HWTEST_F_L0(ProfilerImplTest, DispatcherImplStartPreciseCoverage_WrongParams)
{
    std::string result = "";
    std::function<void(const void*, const std::string &)> callback =
        [&result]([[maybe_unused]] const void *ptr, const std::string &temp) { result = temp; };
    ProtocolChannel *channel = new ProtocolHandler(callback, ecmaVm);
    auto tracing = std::make_unique<ProfilerImpl>(ecmaVm, channel);
    auto dispatcherImpl = std::make_unique<ProfilerImpl::DispatcherImpl>(channel, std::move(tracing));
    std::string msg = "";
    msg += R"({"id":0,"method":"Profiler.startPreciseCoverage","params":{"callCount":"not_a_boolean"}})";
    DispatchRequest request(msg);
    dispatcherImpl->Dispatch(request);
    if (channel) {
        delete channel;
        channel = nullptr;
    }
    ASSERT_TRUE(result.find("wrong params") != std::string::npos);
}

HWTEST_F_L0(ProfilerImplTest, DispatcherImplStartTypeProfile)
{
    std::string result = "";
    std::function<void(const void*, const std::string &)> callback =
        [&result]([[maybe_unused]] const void *ptr, const std::string &temp) { result = temp; };
    ProtocolChannel *channel = new ProtocolHandler(callback, ecmaVm);
    auto tracing = std::make_unique<ProfilerImpl>(ecmaVm, channel);
    auto dispatcherImpl = std::make_unique<ProfilerImpl::DispatcherImpl>(channel, std::move(tracing));
    std::string msg = std::string() + R"({"id":0,"method":"Profiler.startTypeProfile","params":{}})";
    DispatchRequest request(msg);
    dispatcherImpl->Dispatch(request);
    if (channel) {
        delete channel;
        channel = nullptr;
    }
    ASSERT_TRUE(result.find("StartTypeProfile not support now") != std::string::npos);
}

HWTEST_F_L0(ProfilerImplTest, DispatcherImplStopTypeProfile)
{
    std::string result = "";
    std::function<void(const void*, const std::string &)> callback =
        [&result]([[maybe_unused]] const void *ptr, const std::string &temp) { result = temp; };
    ProtocolChannel *channel = new ProtocolHandler(callback, ecmaVm);
    auto tracing = std::make_unique<ProfilerImpl>(ecmaVm, channel);
    auto dispatcherImpl = std::make_unique<ProfilerImpl::DispatcherImpl>(channel, std::move(tracing));
    std::string msg = std::string() + R"({"id":0,"method":"Profiler.stopTypeProfile","params":{}})";
    DispatchRequest request(msg);
    dispatcherImpl->Dispatch(request);
    if (channel) {
        delete channel;
        channel = nullptr;
    }
    ASSERT_TRUE(result.find("StopTypeProfile not support now") != std::string::npos);
}

HWTEST_F_L0(ProfilerImplTest, DispatcherImplTakeTypeProfile)
{
    std::string result = "";
    std::function<void(const void*, const std::string &)> callback =
        [&result]([[maybe_unused]] const void *ptr, const std::string &temp) { result = temp; };
    ProtocolChannel *channel = new ProtocolHandler(callback, ecmaVm);
    auto tracing = std::make_unique<ProfilerImpl>(ecmaVm, channel);
    auto dispatcherImpl = std::make_unique<ProfilerImpl::DispatcherImpl>(channel, std::move(tracing));
    std::string msg = std::string() + R"({"id":0,"method":"Profiler.takeTypeProfile","params":{}})";
    DispatchRequest request(msg);
    dispatcherImpl->Dispatch(request);
    if (channel) {
        delete channel;
        channel = nullptr;
    }
    ASSERT_TRUE(result.find("TakeTypeProfile not support now") != std::string::npos);
}

HWTEST_F_L0(ProfilerImplTest, DispatcherImplDisableSerializationTimeoutCheck)
{
    std::string result = "";
    std::function<void(const void*, const std::string &)> callback =
        [&result]([[maybe_unused]] const void *ptr, const std::string &temp) { result = temp; };
    ProtocolChannel *channel = new ProtocolHandler(callback, ecmaVm);
    auto tracing = std::make_unique<ProfilerImpl>(ecmaVm, channel);
    auto dispatcherImpl = std::make_unique<ProfilerImpl::DispatcherImpl>(channel, std::move(tracing));
    std::string msg = std::string() + R"({"id":0,"method":"Profiler.disableSerializationTimeoutCheck","params":{}})";
    DispatchRequest request(msg);
    dispatcherImpl->Dispatch(request);
    if (channel) {
        delete channel;
        channel = nullptr;
    }
    ASSERT_TRUE(result == "{\"id\":0,\"result\":{}}");
}

HWTEST_F_L0(ProfilerImplTest, DispatcherImplEnableSerializationTimeoutCheck)
{
    std::string result = "";
    std::function<void(const void*, const std::string &)> callback =
        [&result]([[maybe_unused]] const void *ptr, const std::string &temp) { result = temp; };
    ProtocolChannel *channel = new ProtocolHandler(callback, ecmaVm);
    auto tracing = std::make_unique<ProfilerImpl>(ecmaVm, channel);
    auto dispatcherImpl = std::make_unique<ProfilerImpl::DispatcherImpl>(channel, std::move(tracing));
    std::string msg = std::string() + R"({"id":0,"method":"Profiler.enableSerializationTimeoutCheck","params":{}})";
    DispatchRequest request(msg);
    dispatcherImpl->Dispatch(request);
    ASSERT_TRUE(result.find("wrong params") != std::string::npos);
    msg = std::string() + R"({"id":0,"method":"Profiler.enableSerializationTimeoutCheck","params":{"threshold": 5}})";
    DispatchRequest request1(msg);
    dispatcherImpl->Dispatch(request1);
    if (channel) {
        delete channel;
        channel = nullptr;
    }
    ASSERT_TRUE(result == "{\"id\":0,\"result\":{}}");
}

HWTEST_F_L0(ProfilerImplTest, FrontendPreciseCoverageDeltaUpdate)
{
    std::string result = "";
    std::function<void(const void*, const std::string &)> callback =
        [&result]([[maybe_unused]] const void *ptr, const std::string &temp) { result = temp; };
    ProtocolChannel *channel = nullptr;
    auto frontend = std::make_unique<ProfilerImpl::Frontend>(channel);
    frontend->PreciseCoverageDeltaUpdate();
    ASSERT_TRUE(result == "");
    if (!channel) {
        channel = new ProtocolHandler(callback, ecmaVm);
    }
    auto frontend1 = std::make_unique<ProfilerImpl::Frontend>(channel);
    frontend1->PreciseCoverageDeltaUpdate();
    if (channel) {
        delete channel;
        channel = nullptr;
    }
    ASSERT_TRUE(result.find("Profile.PreciseCoverageDeltaUpdate") != std::string::npos);
}
}  // namespace panda::test
