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

#include "agent/heapprofiler_impl.h"
#include "ecmascript/tests/test_helper.h"
#include <uv.h>

using namespace panda::ecmascript;
using namespace panda::ecmascript::tooling;

namespace panda::ecmascript::tooling {
class HeapProfilerImplFriendTest {
public:
    explicit HeapProfilerImplFriendTest(std::unique_ptr<HeapProfilerImpl> &heapprofilerImpl)
    {
        heapprofilerImpl_ = std::move(heapprofilerImpl);
    }

    void ResetProfiles()
    {
        heapprofilerImpl_->frontend_.ResetProfiles();
    }

    void LastSeenObjectId(int32_t lastSeenObjectId, int64_t timeStampUs)
    {
        heapprofilerImpl_->frontend_.LastSeenObjectId(lastSeenObjectId, timeStampUs);
    }

    void HeapStatsUpdate(HeapStat* updateData, int32_t count)
    {
        heapprofilerImpl_->frontend_.HeapStatsUpdate(updateData, count);
    }

    void AddHeapSnapshotChunk(char *data, int32_t size)
    {
        heapprofilerImpl_->frontend_.AddHeapSnapshotChunk(data, size);
    }

    void ReportHeapSnapshotProgress(int32_t done, int32_t total)
    {
        heapprofilerImpl_->frontend_.ReportHeapSnapshotProgress(done, total);
    }

    static void TestHeapTrackingCallback(uv_timer_t* handle)
    {
#if defined(ECMASCRIPT_SUPPORT_HEAPPROFILER)
        HeapProfilerImpl::HeapTrackingCallback(handle);
#endif
    }

private:
    std::unique_ptr<HeapProfilerImpl> heapprofilerImpl_;
};
}

namespace panda::test {
class HeapProfilerImplTest : public testing::Test {
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

HWTEST_F_L0(HeapProfilerImplTest, AddInspectedHeapObject)
{
    ProtocolChannel *channel = nullptr;
    AddInspectedHeapObjectParams param;
    auto heapProfiler = std::make_unique<HeapProfilerImpl>(ecmaVm, channel);
    DispatchResponse response = heapProfiler->AddInspectedHeapObject(param);
    ASSERT_TRUE(response.GetMessage() == "AddInspectedHeapObject not support now");
    ASSERT_TRUE(!response.IsOk());
}

HWTEST_F_L0(HeapProfilerImplTest, CollectGarbage)
{
    ProtocolChannel *channel = nullptr;
    auto heapProfiler = std::make_unique<HeapProfilerImpl>(ecmaVm, channel);
    DispatchResponse response = heapProfiler->CollectGarbage();
    ASSERT_TRUE(response.GetMessage() == "");
    ASSERT_TRUE(response.IsOk());
}

HWTEST_F_L0(HeapProfilerImplTest, Enable)
{
    ProtocolChannel *channel = nullptr;
    auto heapProfiler = std::make_unique<HeapProfilerImpl>(ecmaVm, channel);
    DispatchResponse response = heapProfiler->Enable();
    ASSERT_TRUE(response.GetMessage() == "");
    ASSERT_TRUE(response.IsOk());
}

HWTEST_F_L0(HeapProfilerImplTest, Disable)
{
    ProtocolChannel *channel = nullptr;
    auto heapProfiler = std::make_unique<HeapProfilerImpl>(ecmaVm, channel);
    DispatchResponse response = heapProfiler->Disable();
    ASSERT_TRUE(response.GetMessage() == "");
    ASSERT_TRUE(response.IsOk());
}

HWTEST_F_L0(HeapProfilerImplTest, GetHeapObjectId)
{
    ProtocolChannel *channel = nullptr;
    auto heapProfiler = std::make_unique<HeapProfilerImpl>(ecmaVm, channel);
    GetHeapObjectIdParams params;
    HeapSnapshotObjectId objectId;
    DispatchResponse response = heapProfiler->GetHeapObjectId(params, &objectId);
    ASSERT_TRUE(response.GetMessage() == "GetHeapObjectId not support now");
    ASSERT_TRUE(!response.IsOk());
}

HWTEST_F_L0(HeapProfilerImplTest, GetObjectByHeapObjectId)
{
    ProtocolChannel *channel = nullptr;
    auto heapProfiler = std::make_unique<HeapProfilerImpl>(ecmaVm, channel);
    std::unique_ptr<GetObjectByHeapObjectIdParams> params;
    std::unique_ptr<RemoteObject> remoteObjectResult;
    DispatchResponse response = heapProfiler->GetObjectByHeapObjectId(*params, &remoteObjectResult);
    ASSERT_TRUE(response.GetMessage() == "GetObjectByHeapObjectId not support now");
    ASSERT_TRUE(!response.IsOk());
}

HWTEST_F_L0(HeapProfilerImplTest, GetSamplingProfile)
{
    ProtocolChannel *channel = nullptr;
    auto heapProfiler = std::make_unique<HeapProfilerImpl>(ecmaVm, channel);
    std::unique_ptr<SamplingHeapProfile> profile;
    DispatchResponse response = heapProfiler->GetSamplingProfile(&profile);
    ASSERT_TRUE(response.GetMessage() == "GetSamplingProfile fail");
    ASSERT_TRUE(!response.IsOk());
}

HWTEST_F_L0(HeapProfilerImplTest, StartSampling)
{
    ProtocolChannel *channel = nullptr;
    auto heapProfiler = std::make_unique<HeapProfilerImpl>(ecmaVm, channel);
    StartSamplingParams params;
    DispatchResponse response = heapProfiler->StartSampling(params);
    ASSERT_TRUE(response.IsOk());
}

HWTEST_F_L0(HeapProfilerImplTest, StopSampling)
{
    ProtocolChannel *channel = nullptr;
    auto heapProfiler = std::make_unique<HeapProfilerImpl>(ecmaVm, channel);
    std::unique_ptr<SamplingHeapProfile> profile;
    DispatchResponse response = heapProfiler->StopSampling(&profile);
    ASSERT_TRUE(response.GetMessage() == "StopSampling fail");
    ASSERT_TRUE(!response.IsOk());
}

HWTEST_F_L0(HeapProfilerImplTest, TakeHeapSnapshot)
{
    ProtocolChannel *channel = nullptr;
    auto heapProfiler = std::make_unique<HeapProfilerImpl>(ecmaVm, channel);
    StopTrackingHeapObjectsParams params;
    DispatchResponse response = heapProfiler->TakeHeapSnapshot(params);
    ASSERT_TRUE(response.GetMessage() == "");
    ASSERT_TRUE(response.IsOk());
}

HWTEST_F_L0(HeapProfilerImplTest, DispatcherImplDispatch)
{
    std::string result = "";
    std::function<void(const void*, const std::string &)> callback =
        [&result]([[maybe_unused]] const void *ptr, const std::string &temp) {result = temp;};
    ProtocolChannel *channel = new ProtocolHandler(callback, ecmaVm);
    auto tracing = std::make_unique<HeapProfilerImpl>(ecmaVm, channel);
    auto dispatcherImpl = std::make_unique<HeapProfilerImpl::DispatcherImpl>(channel, std::move(tracing));
    std::string msg = std::string() + R"({"id":0,"method":"Debugger.Test","params":{}})";
    DispatchRequest request(msg);
    dispatcherImpl->Dispatch(request);
    ASSERT_TRUE(result.find("Unknown method: Test") != std::string::npos);
    msg = std::string() + R"({"id":0,"method":"Debugger.disable","params":{}})";
    DispatchRequest request1 = DispatchRequest(msg);
    dispatcherImpl->Dispatch(request1);
    if (channel) {
        delete channel;
        channel = nullptr;
    }
    ASSERT_TRUE(result == "{\"id\":0,\"result\":{}}");
}

HWTEST_F_L0(HeapProfilerImplTest, DispatcherImplAddInspectedHeapObject)
{
    std::string result = "";
    std::function<void(const void*, const std::string &)> callback =
        [&result]([[maybe_unused]] const void *ptr, const std::string &temp) {result = temp;};
    ProtocolChannel *channel = new ProtocolHandler(callback, ecmaVm);
    auto tracing = std::make_unique<HeapProfilerImpl>(ecmaVm, channel);
    auto dispatcherImpl = std::make_unique<HeapProfilerImpl::DispatcherImpl>(channel, std::move(tracing));
    std::string msg = std::string() + R"({"id":0,"method":"HeapProfiler.addInspectedHeapObject","params":{}})";
    DispatchRequest request(msg);
    dispatcherImpl->Dispatch(request);
    ASSERT_TRUE(result.find("wrong params") != std::string::npos);
    msg = std::string() + R"({"id":0,"method":"HeapProfiler.addInspectedHeapObject","params":{"heapObjectId":"0"}})";
    DispatchRequest request1 = DispatchRequest(msg);
    dispatcherImpl->Dispatch(request1);
    if (channel) {
        delete channel;
        channel = nullptr;
    }
    ASSERT_TRUE(result.find("AddInspectedHeapObject not support now") != std::string::npos);
}

HWTEST_F_L0(HeapProfilerImplTest, DispatcherImplCollectGarbage)
{
    std::string result = "";
    std::function<void(const void*, const std::string &)> callback =
        [&result]([[maybe_unused]] const void *ptr, const std::string &temp) {result = temp;};
    ProtocolChannel *channel = new ProtocolHandler(callback, ecmaVm);
    auto tracing = std::make_unique<HeapProfilerImpl>(ecmaVm, channel);
    auto dispatcherImpl = std::make_unique<HeapProfilerImpl::DispatcherImpl>(channel, std::move(tracing));
    std::string msg = std::string() + R"({"id":0,"method":"HeapProfiler.collectGarbage","params":{}})";
    DispatchRequest request(msg);
    dispatcherImpl->Dispatch(request);
    ASSERT_TRUE(result == "{\"id\":0,\"result\":{}}");
    if (channel) {
        delete channel;
        channel = nullptr;
    }
}

HWTEST_F_L0(HeapProfilerImplTest, DispatcherImplEnable)
{
    std::string result = "";
    std::function<void(const void*, const std::string &)> callback =
        [&result]([[maybe_unused]] const void *ptr, const std::string &temp) {result = temp;};
    ProtocolChannel *channel = new ProtocolHandler(callback, ecmaVm);
    auto tracing = std::make_unique<HeapProfilerImpl>(ecmaVm, channel);
    auto dispatcherImpl = std::make_unique<HeapProfilerImpl::DispatcherImpl>(channel, std::move(tracing));
    std::string msg = std::string() + R"({"id":0,"method":"HeapProfiler.enable","params":{}})";
    DispatchRequest request(msg);
    dispatcherImpl->Dispatch(request);
    ASSERT_TRUE(result.find("protocols") != std::string::npos);
    if (channel) {
        delete channel;
        channel = nullptr;
    }
}

HWTEST_F_L0(HeapProfilerImplTest, DispatcherImplDisable)
{
    std::string result = "";
    std::function<void(const void*, const std::string &)> callback =
        [&result]([[maybe_unused]] const void *ptr, const std::string &temp) {result = temp;};
    ProtocolChannel *channel = new ProtocolHandler(callback, ecmaVm);
    auto tracing = std::make_unique<HeapProfilerImpl>(ecmaVm, channel);
    auto dispatcherImpl = std::make_unique<HeapProfilerImpl::DispatcherImpl>(channel, std::move(tracing));
    std::string msg = std::string() + R"({"id":0,"method":"HeapProfiler.disable","params":{}})";
    DispatchRequest request(msg);
    dispatcherImpl->Dispatch(request);
    ASSERT_TRUE(result == "{\"id\":0,\"result\":{}}");
    if (channel) {
        delete channel;
        channel = nullptr;
    }
}

HWTEST_F_L0(HeapProfilerImplTest, DispatcherImplGetHeapObjectId)
{
    std::string result = "";
    std::function<void(const void*, const std::string &)> callback =
        [&result]([[maybe_unused]] const void *ptr, const std::string &temp) {result = temp;};
    ProtocolChannel *channel = new ProtocolHandler(callback, ecmaVm);
    auto tracing = std::make_unique<HeapProfilerImpl>(ecmaVm, channel);
    auto dispatcherImpl = std::make_unique<HeapProfilerImpl::DispatcherImpl>(channel, std::move(tracing));
    std::string msg = std::string() + R"({"id":0,"method":"HeapProfiler.getHeapObjectId","params":{"objectId":true}})";
    DispatchRequest request(msg);
    dispatcherImpl->Dispatch(request);
    ASSERT_TRUE(result.find("wrong params") != std::string::npos);
    msg = std::string() + R"({"id":0,"method":"HeapProfiler.getHeapObjectId","params":{"objectId":"0"}})";
    DispatchRequest request1(msg);
    dispatcherImpl->Dispatch(request1);
    ASSERT_TRUE(result.find("GetHeapObjectId not support now") != std::string::npos);
    if (channel) {
        delete channel;
        channel = nullptr;
    }
}

HWTEST_F_L0(HeapProfilerImplTest, DispatcherImplGetObjectByHeapObjectId)
{
    std::string result = "";
    std::function<void(const void*, const std::string &)> callback =
        [&result]([[maybe_unused]] const void *ptr, const std::string &temp) {result = temp;};
    ProtocolChannel *channel = new ProtocolHandler(callback, ecmaVm);
    auto tracing = std::make_unique<HeapProfilerImpl>(ecmaVm, channel);
    auto dispatcherImpl = std::make_unique<HeapProfilerImpl::DispatcherImpl>(channel, std::move(tracing));
    std::string msg = std::string() + R"({"id":0,"method":"HeapProfiler.getObjectByHeapObjectId","params":{
        "objectId":001}})";
    DispatchRequest request(msg);
    dispatcherImpl->Dispatch(request);
    ASSERT_TRUE(result.find("wrong params") != std::string::npos);
    msg = std::string() + R"({"id":0,"method":"HeapProfiler.getObjectByHeapObjectId","params":{"objectId":"001",
        "objectGroup":"000"}})";
    DispatchRequest request1(msg);
    dispatcherImpl->Dispatch(request1);
    ASSERT_TRUE(result.find("GetObjectByHeapObjectId not support now") != std::string::npos);
    if (channel) {
        delete channel;
        channel = nullptr;
    }
}

HWTEST_F_L0(HeapProfilerImplTest, DispatcherImplGetSamplingProfile)
{
    std::string result = "";
    std::function<void(const void*, const std::string &)> callback =
        [&result]([[maybe_unused]] const void *ptr, const std::string &temp) {result = temp;};
    ProtocolChannel *channel = new ProtocolHandler(callback, ecmaVm);
    auto tracing = std::make_unique<HeapProfilerImpl>(ecmaVm, channel);
    auto dispatcherImpl = std::make_unique<HeapProfilerImpl::DispatcherImpl>(channel, std::move(tracing));
    std::string msg = std::string() + R"({"id":0,"method":"HeapProfiler.getSamplingProfile","params":{}})";
    DispatchRequest request(msg);
    dispatcherImpl->GetSamplingProfile(request);
    ASSERT_TRUE(result.find("GetSamplingProfile fail") != std::string::npos);
    if (channel) {
        delete channel;
        channel = nullptr;
    }
}

HWTEST_F_L0(HeapProfilerImplTest, DispatcherImplStartSampling)
{
    std::string result = "";
    std::function<void(const void*, const std::string &)> callback =
        [&result]([[maybe_unused]] const void *ptr, const std::string &temp) {result = temp;};
    ProtocolChannel *channel = new ProtocolHandler(callback, ecmaVm);
    auto tracing = std::make_unique<HeapProfilerImpl>(ecmaVm, channel);
    auto dispatcherImpl = std::make_unique<HeapProfilerImpl::DispatcherImpl>(channel, std::move(tracing));
    std::string msg = std::string() + R"({"id":0,"method":"HeapProfiler.startSampling","params":{
        "samplingInterval":"Test"}})";
    DispatchRequest request(msg);
    dispatcherImpl->StartSampling(request);
    ASSERT_TRUE(result.find("wrong params") != std::string::npos);
    msg = std::string() + R"({"id":0,"method":"HeapProfiler.startSampling","params":{"samplingInterval":1000}})";
    DispatchRequest request1(msg);
    dispatcherImpl->StartSampling(request1);
    ASSERT_TRUE(result == "{\"id\":0,\"result\":{}}");
    if (channel) {
        delete channel;
        channel = nullptr;
    }
}

HWTEST_F_L0(HeapProfilerImplTest, DispatcherImplStopSampling)
{
    std::string result = "";
    std::function<void(const void*, const std::string &)> callback =
        [&result]([[maybe_unused]] const void *ptr, const std::string &temp) {result = temp;};
    ProtocolChannel *channel = new ProtocolHandler(callback, ecmaVm);
    auto tracing = std::make_unique<HeapProfilerImpl>(ecmaVm, channel);
    auto dispatcherImpl = std::make_unique<HeapProfilerImpl::DispatcherImpl>(channel, std::move(tracing));
    std::string msg = std::string() + R"({"id":0,"method":"HeapProfiler.stopSampling","params":{}})";
    DispatchRequest request(msg);
    dispatcherImpl->StopSampling(request);
    ASSERT_TRUE(result.find("StopSampling fail") != std::string::npos);
    if (channel) {
        delete channel;
        channel = nullptr;
    }
}

HWTEST_F_L0(HeapProfilerImplTest, DispatcherImplTakeHeapSnapshot)
{
    std::string result = "";
    std::function<void(const void*, const std::string &)> callback =
        [&result]([[maybe_unused]] const void *ptr, const std::string &temp) {result = temp;};
    ProtocolChannel *channel = new ProtocolHandler(callback, ecmaVm);
    auto tracing = std::make_unique<HeapProfilerImpl>(ecmaVm, channel);
    auto dispatcherImpl = std::make_unique<HeapProfilerImpl::DispatcherImpl>(channel, std::move(tracing));
    std::string msg = std::string() + R"({"id":0,"method":"HeapProfiler.takeHeapSnapshot","params":{
        "reportProgress":10,
        "treatGlobalObjectsAsRoots":10,
        "captureNumericValue":10}})";
    DispatchRequest request(msg);
    dispatcherImpl->TakeHeapSnapshot(request);
    ASSERT_TRUE(result.find("wrong params") != std::string::npos);
    msg = std::string() + R"({"id":0,"method":"HeapProfiler.takeHeapSnapshot","params":{
        "reportProgress":true,
        "treatGlobalObjectsAsRoots":true,
        "captureNumericValue":true}})";
    DispatchRequest request1(msg);
    dispatcherImpl->TakeHeapSnapshot(request1);
    ASSERT_TRUE(result == "{\"id\":0,\"result\":{}}");
    if (channel) {
        delete channel;
        channel = nullptr;
    }
}

HWTEST_F_L0(HeapProfilerImplTest, GetSamplingProfileSuccessful)
{
    StartSamplingParams params;
    ProtocolChannel *channel = nullptr;
    auto heapProfiler = std::make_unique<HeapProfilerImpl>(ecmaVm, channel);
    DispatchResponse response = heapProfiler->StartSampling(params);
    std::unique_ptr<SamplingHeapProfile> samplingHeapProfile = std::make_unique<SamplingHeapProfile>();
    DispatchResponse result = heapProfiler->GetSamplingProfile(&samplingHeapProfile);
    ASSERT_TRUE(result.IsOk());
}

HWTEST_F_L0(HeapProfilerImplTest, StartSamplingFail)
{
    StartSamplingParams params;
    ProtocolChannel *channel = nullptr;
    auto heapProfiler = std::make_unique<HeapProfilerImpl>(ecmaVm, channel);
    DispatchResponse response = heapProfiler->StartSampling(params);
    DispatchResponse result = heapProfiler->StartSampling(params);
    ASSERT_TRUE(!result.IsOk());
}

HWTEST_F_L0(HeapProfilerImplTest, StopSamplingSuccessful)
{
    StartSamplingParams params;
    ProtocolChannel *channel = nullptr;
    std::unique_ptr<SamplingHeapProfile> samplingHeapProfile = std::make_unique<SamplingHeapProfile>();
    auto heapProfiler = std::make_unique<HeapProfilerImpl>(ecmaVm, channel);
    DispatchResponse response = heapProfiler->StartSampling(params);
    DispatchResponse result = heapProfiler->StopSampling(&samplingHeapProfile);
    ASSERT_TRUE(result.IsOk());
}

HWTEST_F_L0(HeapProfilerImplTest, StartTrackingHeapObjects)
{
    std::string result = "";
    std::function<void(const void*, const std::string &)> callback =
        [&result]([[maybe_unused]] const void *ptr, const std::string &temp) {result = temp;};
    ProtocolChannel *channel = new ProtocolHandler(callback, ecmaVm);
    auto tracing = std::make_unique<HeapProfilerImpl>(ecmaVm, channel);
    auto dispatcherImpl = std::make_unique<HeapProfilerImpl::DispatcherImpl>(channel, std::move(tracing));
    std::string msg = "";
    msg += R"({"id":0,"method":"HeapProfiler.StartTrackingHeapObjects","params":{"trackAllocations":0}})";
    DispatchRequest request1(msg);
    dispatcherImpl->StartTrackingHeapObjects(request1);
    ASSERT_TRUE(result == "{\"id\":0,\"result\":{\"code\":1,\"message\":\"wrong params\"}}");
    if (channel) {
        delete channel;
        channel = nullptr;
    }
}

HWTEST_F_L0(HeapProfilerImplTest, StopTrackingHeapObjects)
{
    std::string result = "";
    std::function<void(const void*, const std::string &)> callback =
        [&result]([[maybe_unused]] const void *ptr, const std::string &temp) {result = temp;};
    ProtocolChannel *channel = new ProtocolHandler(callback, ecmaVm);
    auto tracing = std::make_unique<HeapProfilerImpl>(ecmaVm, channel);
    auto dispatcherImpl = std::make_unique<HeapProfilerImpl::DispatcherImpl>(channel, std::move(tracing));
    std::string msg = "";
    msg += R"({"id":0,"method":"HeapProfiler.StopTrackingHeapObjects","params":{"reportProgress":0}})";
    DispatchRequest request1(msg);
    dispatcherImpl->StopTrackingHeapObjects(request1);
    ASSERT_TRUE(result == "{\"id\":0,\"result\":{\"code\":1,\"message\":\"wrong params\"}}");
    if (channel) {
        delete channel;
        channel = nullptr;
    }
}

HWTEST_F_L0(HeapProfilerImplTest, RepeateStartTrackingHeapObjects)
{
    std::string result = "";
    std::function<void(const void*, const std::string &)> callback =
        [&result]([[maybe_unused]] const void *ptr, const std::string &temp) {result = temp;};
    ProtocolChannel *channel = new ProtocolHandler(callback, ecmaVm);
    auto tracing = std::make_unique<HeapProfilerImpl>(ecmaVm, channel);
    auto dispatcherImpl = std::make_unique<HeapProfilerImpl::DispatcherImpl>(channel, std::move(tracing));
    ecmaVm->SetLoop(uv_default_loop());
    std::string msg = "";
    msg += R"({"id":0,"method":"HeapProfiler.StartTrackingHeapObjects","params":{"trackAllocations":false}})";
    DispatchRequest request1(msg);
    dispatcherImpl->StartTrackingHeapObjects(request1);
    ASSERT_TRUE(result == "{\"id\":0,\"result\":{}}");
    std::string msg1 = "";
    msg1 += R"({"id":0,"method":"HeapProfiler.StartTrackingHeapObjects","params":{"trackAllocations":false}})";
    DispatchRequest request2(msg1);
    dispatcherImpl->StartTrackingHeapObjects(request2);
    ASSERT_TRUE(result == "{\"id\":0,\"result\":{}}");
    if (channel) {
        delete channel;
        channel = nullptr;
    }
}

HWTEST_F_L0(HeapProfilerImplTest, StartAndStopTrackingHeapObjects)
{
    std::string result = "";
    std::function<void(const void*, const std::string &)> callback =
        [&result]([[maybe_unused]] const void *ptr, const std::string &temp) {result = temp;};
    ProtocolChannel *channel = new ProtocolHandler(callback, ecmaVm);
    auto tracing = std::make_unique<HeapProfilerImpl>(ecmaVm, channel);
    auto dispatcherImpl = std::make_unique<HeapProfilerImpl::DispatcherImpl>(channel, std::move(tracing));
    ecmaVm->SetLoop(uv_default_loop());
    std::string msg = "";
    msg += R"({"id":0,"method":"HeapProfiler.StartTrackingHeapObjects","params":{"trackAllocations":false}})";
    DispatchRequest request1(msg);
    dispatcherImpl->StartTrackingHeapObjects(request1);
    ASSERT_TRUE(result == "{\"id\":0,\"result\":{}}");
    std::string msg1 = "";
    msg1 += R"({"id":0,"method":"HeapProfiler.StopTrackingHeapObjects","params":{"reportProgress":false}})";
    DispatchRequest request2(msg1);
    dispatcherImpl->StopTrackingHeapObjects(request2);
    ASSERT_TRUE(result == "{\"id\":0,\"result\":{}}");
    if (channel) {
        delete channel;
        channel = nullptr;
    }
}

HWTEST_F_L0(HeapProfilerImplTest, ResetProfiles)
{
    std::string result = "";
    std::function<void(const void*, const std::string &)> callback =
        [&result]([[maybe_unused]] const void *ptr, const std::string &temp) {result = temp;};
    ProtocolChannel *channel = new ProtocolHandler(callback, ecmaVm);
    auto tracing = std::make_unique<HeapProfilerImpl>(ecmaVm, nullptr);
    auto heapprofiler = std::make_unique<HeapProfilerImplFriendTest>(tracing);
    heapprofiler->ResetProfiles();
    ASSERT_TRUE(result == "");
    tracing = std::make_unique<HeapProfilerImpl>(ecmaVm, channel);
    heapprofiler = std::make_unique<HeapProfilerImplFriendTest>(tracing);
    heapprofiler->ResetProfiles();
    ASSERT_TRUE(result == "");
    if (channel) {
        delete channel;
        channel = nullptr;
    }
}

HWTEST_F_L0(HeapProfilerImplTest, LastSeenObjectId)
{
    std::string result = "";
    std::function<void(const void*, const std::string &)> callback =
        [&result]([[maybe_unused]] const void *ptr, const std::string &temp) {result = temp;};
    ProtocolChannel *channel = new ProtocolHandler(callback, ecmaVm);
    auto tracing = std::make_unique<HeapProfilerImpl>(ecmaVm, nullptr);
    auto heapprofiler = std::make_unique<HeapProfilerImplFriendTest>(tracing);
    heapprofiler->LastSeenObjectId(0, 0);
    ASSERT_TRUE(result == "");
    tracing = std::make_unique<HeapProfilerImpl>(ecmaVm, channel);
    heapprofiler = std::make_unique<HeapProfilerImplFriendTest>(tracing);
    heapprofiler->LastSeenObjectId(0, 0);
    std::string msg = "{\"method\":\"HeapProfiler.lastSeenObjectId\",";
    msg += "\"params\":{\"lastSeenObjectId\":0,";
    msg += "\"timestamp\":0}}";
    ASSERT_TRUE(result == msg);
    if (channel) {
        delete channel;
        channel = nullptr;
    }
}

HWTEST_F_L0(HeapProfilerImplTest, HeapStatsUpdate)
{
    std::string result = "";
    std::function<void(const void*, const std::string &)> callback =
        [&result]([[maybe_unused]] const void *ptr, const std::string &temp) {result = temp;};
    ProtocolChannel *channel = new ProtocolHandler(callback, ecmaVm);
    auto tracing = std::make_unique<HeapProfilerImpl>(ecmaVm, nullptr);
    auto heapprofiler = std::make_unique<HeapProfilerImplFriendTest>(tracing);
    heapprofiler->HeapStatsUpdate(nullptr, 0);
    ASSERT_TRUE(result == "");
    tracing = std::make_unique<HeapProfilerImpl>(ecmaVm, channel);
    heapprofiler = std::make_unique<HeapProfilerImplFriendTest>(tracing);
    heapprofiler->HeapStatsUpdate(nullptr, 0);
    std::string msg = "{\"method\":\"HeapProfiler.heapStatsUpdate\",";
    msg += "\"params\":{\"statsUpdate\":[]}}";
    ASSERT_TRUE(result == msg);
    if (channel) {
        delete channel;
        channel = nullptr;
    }
}

HWTEST_F_L0(HeapProfilerImplTest, AddHeapSnapshotChunk)
{
    std::string result = "";
    std::function<void(const void*, const std::string &)> callback =
        [&result]([[maybe_unused]] const void *ptr, const std::string &temp) {result = temp;};
    auto tracing = std::make_unique<HeapProfilerImpl>(ecmaVm, nullptr);
    auto heapprofiler = std::make_unique<HeapProfilerImplFriendTest>(tracing);
    heapprofiler->AddHeapSnapshotChunk(nullptr, 0);
    ASSERT_TRUE(result == "");
}

HWTEST_F_L0(HeapProfilerImplTest, ReportHeapSnapshotProgress)
{
    std::string result = "";
    std::function<void(const void*, const std::string &)> callback =
        [&result]([[maybe_unused]] const void *ptr, const std::string &temp) {result = temp;};
    auto tracing = std::make_unique<HeapProfilerImpl>(ecmaVm, nullptr);
    auto heapprofiler = std::make_unique<HeapProfilerImplFriendTest>(tracing);
    heapprofiler->ReportHeapSnapshotProgress(0, 0);
    ASSERT_TRUE(result == "");
}

HWTEST_F_L0(HeapProfilerImplTest, DispatcherImplDispatchGetSamplingProfile)
{
    std::string result = "";
    std::function<void(const void*, const std::string &)> callback =
        [&result]([[maybe_unused]] const void *ptr, const std::string &temp) {result = temp;};
    ProtocolChannel *channel = new ProtocolHandler(callback, ecmaVm);
    auto heapProfiler = std::make_unique<HeapProfilerImpl>(ecmaVm, channel);
    auto dispatcherImpl = std::make_unique<HeapProfilerImpl::DispatcherImpl>(channel, std::move(heapProfiler));
    std::string msg = "";
    msg += R"({"id":0,"method":"HeapProfiler.getSamplingProfile","params":{}})";
    DispatchRequest request(msg);
    dispatcherImpl->Dispatch(request);
    ASSERT_TRUE(result.find("GetSamplingProfile fail") != std::string::npos);
    if (channel) {
        delete channel;
        channel = nullptr;
    }
}

HWTEST_F_L0(HeapProfilerImplTest, DispatcherImplDispatchGetSamplingProfileSuccess)
{
    std::string result = "";
    std::function<void(const void*, const std::string &)> callback =
        [&result]([[maybe_unused]] const void *ptr, const std::string &temp) {result = temp;};
    ProtocolChannel *channel = new ProtocolHandler(callback, ecmaVm);
    auto heapProfiler = std::make_unique<HeapProfilerImpl>(ecmaVm, channel);
    auto dispatcherImpl = std::make_unique<HeapProfilerImpl::DispatcherImpl>(channel, std::move(heapProfiler));
    std::string msg1 = "";
    msg1 += R"({"id":0,"method":"HeapProfiler.startSampling","params":{"samplingInterval":1000}})";
    DispatchRequest request1(msg1);
    dispatcherImpl->Dispatch(request1);
    result.clear();
    std::string msg2 = "";
    msg2 += R"({"id":0,"method":"HeapProfiler.getSamplingProfile","params":{}})";
    DispatchRequest request2(msg2);
    dispatcherImpl->Dispatch(request2);
    ASSERT_TRUE(result.find("\"result\"") != std::string::npos);
    ASSERT_TRUE(result.find("\"error\"") == std::string::npos);
    if (channel) {
        delete channel;
        channel = nullptr;
    }
}

HWTEST_F_L0(HeapProfilerImplTest, DispatcherImplDispatchStartSampling)
{
    std::string result = "";
    std::function<void(const void*, const std::string &)> callback =
        [&result]([[maybe_unused]] const void *ptr, const std::string &temp) {result = temp;};
    ProtocolChannel *channel = new ProtocolHandler(callback, ecmaVm);
    auto heapProfiler = std::make_unique<HeapProfilerImpl>(ecmaVm, channel);
    auto dispatcherImpl = std::make_unique<HeapProfilerImpl::DispatcherImpl>(channel, std::move(heapProfiler));
    std::string msg1 = "";
    msg1 += R"({"id":0,"method":"HeapProfiler.startSampling","params":{"samplingInterval":1000}})";
    DispatchRequest request1(msg1);
    dispatcherImpl->Dispatch(request1);
    ASSERT_TRUE(result == "{\"id\":0,\"result\":{}}");
    result.clear();
    std::string msg2 = "";
    msg2 += R"({"id":0,"method":"HeapProfiler.startSampling","params":{"samplingInterval":"Test"}})";
    DispatchRequest request2(msg2);
    dispatcherImpl->Dispatch(request2);
    ASSERT_TRUE(result.find("wrong params") != std::string::npos);
    if (channel) {
        delete channel;
        channel = nullptr;
    }
}

HWTEST_F_L0(HeapProfilerImplTest, DispatcherImplDispatchStartTrackingHeapObjects)
{
    std::string result = "";
    std::function<void(const void*, const std::string &)> callback =
        [&result]([[maybe_unused]] const void *ptr, const std::string &temp) {result = temp;};
    ProtocolChannel *channel = new ProtocolHandler(callback, ecmaVm);
    auto heapProfiler = std::make_unique<HeapProfilerImpl>(ecmaVm, channel);
    auto dispatcherImpl = std::make_unique<HeapProfilerImpl::DispatcherImpl>(channel, std::move(heapProfiler));
    std::string msg1 = "";
    msg1 += R"({"id":0,"method":"HeapProfiler.startTrackingHeapObjects","params":{"trackAllocations":false}})";
    DispatchRequest request1(msg1);
    dispatcherImpl->Dispatch(request1);
    ASSERT_TRUE(result.find("Loop is nullptr") != std::string::npos);
    result.clear();
    std::string msg2 = "";
    msg2 += R"({"id":0,"method":"HeapProfiler.startTrackingHeapObjects","params":{"trackAllocations":0}})";
    DispatchRequest request2(msg2);
    dispatcherImpl->Dispatch(request2);
    ASSERT_TRUE(result == "{\"id\":0,\"result\":{\"code\":1,\"message\":\"wrong params\"}}");
    if (channel) {
        delete channel;
        channel = nullptr;
    }
}

HWTEST_F_L0(HeapProfilerImplTest, DispatcherImplDispatchStopSampling)
{
    std::string result = "";
    std::function<void(const void*, const std::string &)> callback =
        [&result]([[maybe_unused]] const void *ptr, const std::string &temp) {result = temp;};
    ProtocolChannel *channel = new ProtocolHandler(callback, ecmaVm);
    auto heapProfiler = std::make_unique<HeapProfilerImpl>(ecmaVm, channel);
    auto dispatcherImpl = std::make_unique<HeapProfilerImpl::DispatcherImpl>(channel, std::move(heapProfiler));
    std::string msg = "";
    msg += R"({"id":0,"method":"HeapProfiler.stopSampling","params":{}})";
    DispatchRequest request(msg);
    dispatcherImpl->Dispatch(request);
    ASSERT_TRUE(result.find("StopSampling fail") != std::string::npos);
    if (channel) {
        delete channel;
        channel = nullptr;
    }
}

HWTEST_F_L0(HeapProfilerImplTest, DispatcherImplDispatchStopSamplingSuccess)
{
    std::string result = "";
    std::function<void(const void*, const std::string &)> callback =
        [&result]([[maybe_unused]] const void *ptr, const std::string &temp) {result = temp;};
    ProtocolChannel *channel = new ProtocolHandler(callback, ecmaVm);
    auto heapProfiler = std::make_unique<HeapProfilerImpl>(ecmaVm, channel);
    auto dispatcherImpl = std::make_unique<HeapProfilerImpl::DispatcherImpl>(channel, std::move(heapProfiler));
    std::string msg1 = "";
    msg1 += R"({"id":0,"method":"HeapProfiler.startSampling","params":{"samplingInterval":1000}})";
    DispatchRequest request1(msg1);
    dispatcherImpl->Dispatch(request1);
    result.clear();
    std::string msg2 = "";
    msg2 += R"({"id":0,"method":"HeapProfiler.stopSampling","params":{}})";
    DispatchRequest request2(msg2);
    dispatcherImpl->Dispatch(request2);
    ASSERT_TRUE(result.find("\"result\"") != std::string::npos);
    ASSERT_TRUE(result.find("\"error\"") == std::string::npos);
    if (channel) {
        delete channel;
        channel = nullptr;
    }
}

HWTEST_F_L0(HeapProfilerImplTest, DispatcherImplDispatchStopTrackingHeapObjects_01)
{
    std::string result = "";
    std::function<void(const void*, const std::string &)> callback =
        [&result]([[maybe_unused]] const void *ptr, const std::string &temp) {result = temp;};
    ProtocolChannel *channel = new ProtocolHandler(callback, ecmaVm);
    auto heapProfiler = std::make_unique<HeapProfilerImpl>(ecmaVm, channel);
    auto dispatcherImpl = std::make_unique<HeapProfilerImpl::DispatcherImpl>(channel, std::move(heapProfiler));
    std::string msg1 = "";
    msg1 += R"({"id":0,"method":"HeapProfiler.stopTrackingHeapObjects","params":{"reportProgress":false}})";
    DispatchRequest request1(msg1);
    dispatcherImpl->Dispatch(request1);
    ASSERT_TRUE(result.find("StopHeapTracking fail") != std::string::npos);
    result.clear();
    std::string msg2 = "";
    msg2 += R"({"id":0,"method":"HeapProfiler.stopTrackingHeapObjects","params":{"reportProgress":0}})";
    DispatchRequest request2(msg2);
    dispatcherImpl->Dispatch(request2);
    ASSERT_TRUE(result == "{\"id\":0,\"result\":{\"code\":1,\"message\":\"wrong params\"}}");
    if (channel) {
        delete channel;
        channel = nullptr;
    }
}

HWTEST_F_L0(HeapProfilerImplTest, DispatcherImplDispatchStopTrackingHeapObjects_02)
{
    std::string result = "";
    std::function<void(const void*, const std::string &)> callback =
        [&result]([[maybe_unused]] const void *ptr, const std::string &temp) {result = temp;};
    ProtocolChannel *channel = new ProtocolHandler(callback, ecmaVm);
    auto tracing = std::make_unique<HeapProfilerImpl>(ecmaVm, channel);
    auto dispatcherImpl = std::make_unique<HeapProfilerImpl::DispatcherImpl>(channel, std::move(tracing));
    std::string msg = std::string() + R"({"id":0,"method":"HeapProfiler.stopTrackingHeapObjects","params":{
        "reportProgress":10,
        "treatGlobalObjectsAsRoots":10,
        "captureNumericValue":10}})";
    DispatchRequest request(msg);
    dispatcherImpl->Dispatch(request);
    ASSERT_TRUE(result.find("wrong params") != std::string::npos);
    msg = std::string() + R"({"id":0,"method":"HeapProfiler.stopTrackingHeapObjects","params":{
        "reportProgress":true,
        "treatGlobalObjectsAsRoots":true,
        "captureNumericValue":true}})";
    DispatchRequest request1(msg);
    dispatcherImpl->Dispatch(request1);
    ASSERT_TRUE(result.find("StopHeapTracking fail") != std::string::npos);
    if (channel) {
        delete channel;
        channel = nullptr;
    }
}

HWTEST_F_L0(HeapProfilerImplTest, DispatcherImplDispatchTakeHeapSnapshot)
{
    std::string result = "";
    std::function<void(const void*, const std::string &)> callback =
        [&result]([[maybe_unused]] const void *ptr, const std::string &temp) {result = temp;};
    ProtocolChannel *channel = new ProtocolHandler(callback, ecmaVm);
    auto heapProfiler = std::make_unique<HeapProfilerImpl>(ecmaVm, channel);
    auto dispatcherImpl = std::make_unique<HeapProfilerImpl::DispatcherImpl>(channel, std::move(heapProfiler));
    std::string msg1 = "";
    msg1 += R"({"id":0,"method":"HeapProfiler.takeHeapSnapshot","params":{
        "reportProgress":10,
        "treatGlobalObjectsAsRoots":10,
        "captureNumericValue":10}})";
    DispatchRequest request1(msg1);
    dispatcherImpl->Dispatch(request1);
    ASSERT_TRUE(result.find("wrong params") != std::string::npos);
    result.clear();
    std::string msg2 = "";
    msg2 += R"({"id":0,"method":"HeapProfiler.takeHeapSnapshot","params":{
        "reportProgress":true,
        "treatGlobalObjectsAsRoots":true,
        "captureNumericValue":true}})";
    DispatchRequest request2(msg2);
    dispatcherImpl->Dispatch(request2);
    ASSERT_TRUE(result == "{\"id\":0,\"result\":{}}");
    if (channel) {
        delete channel;
        channel = nullptr;
    }
}

HWTEST_F_L0(HeapProfilerImplTest, HeapTrackingCallbackTest)
{
#if defined(ECMASCRIPT_SUPPORT_HEAPPROFILER)
    std::string result = "";
    uv_timer_t handle1;
    handle1.data = nullptr;
    HeapProfilerImplFriendTest::TestHeapTrackingCallback(&handle1);
    ASSERT_TRUE(result == "");
    std::function<void(const void*, const std::string &)> callback =
        [&result]([[maybe_unused]] const void *ptr, const std::string &temp) {result = temp;};
    ProtocolChannel *channel = new ProtocolHandler(callback, ecmaVm);
    auto heapProfiler = std::make_unique<HeapProfilerImpl>(ecmaVm, channel);
    uv_timer_t handle2;
    handle2.data = heapProfiler.get();
    HeapProfilerImplFriendTest::TestHeapTrackingCallback(&handle2);
    ASSERT_TRUE(result == "");
    if (channel) {
        delete channel;
        channel = nullptr;
    }
#endif
}
}  // namespace panda::test
