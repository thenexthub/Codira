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

#include "agent/heapprofiler_impl.h"

namespace panda::ecmascript::tooling {
static constexpr int32_t MILLI_TO_MICRO = 1000;
static constexpr double INTERVAL = 0.2;
// Whenever adding a new protocol which is not a standard CDP protocol,
// must add its methodName to the heapProfilerProtocolList
void HeapProfilerImpl::InitializeExtendedProtocolsList()
{
    std::vector<std::string> heapProfilerProtocolList {};
    heapProfilerExtendedProtocols_ = std::move(heapProfilerProtocolList);
}

std::optional<std::string> HeapProfilerImpl::DispatcherImpl::Dispatch(const DispatchRequest &request,
    [[maybe_unused]] bool crossLanguageDebug)
{
    Method method = GetMethodEnum(request.GetMethod());
    LOG_DEBUGGER(DEBUG) << "dispatch [" << request.GetMethod() << "] to HeapProfilerImpl";
    switch (method) {
        case Method::ADD_INSPECTED_HEAP_OBJECT:
            AddInspectedHeapObject(request);
            break;
        case Method::COLLECT_GARBAGE:
            CollectGarbage(request);
            break;
        case Method::ENABLE:
            Enable(request);
            break;
        case Method::DISABLE:
            Disable(request);
            break;
        case Method::GET_HEAP_OBJECT_ID:
            GetHeapObjectId(request);
            break;
        case Method::GET_OBJECT_BY_HEAP_OBJECT_ID:
            GetObjectByHeapObjectId(request);
            break;
        case Method::GET_SAMPLING_PROFILE:
            GetSamplingProfile(request);
            break;
        case Method::START_SAMPLING:
            StartSampling(request);
            break;
        case Method::START_TRACKING_HEAP_OBJECTS:
            StartTrackingHeapObjects(request);
            break;
        case Method::STOP_SAMPLING:
            StopSampling(request);
            break;
        case Method::STOP_TRACKING_HEAP_OBJECTS:
            StopTrackingHeapObjects(request);
            break;
        case Method::TAKE_HEAP_SNAPSHOT:
            TakeHeapSnapshot(request);
            break;
        default:
            SendResponse(request, DispatchResponse::Fail("Unknown method: " + request.GetMethod()));
            break;
    }
    return std::nullopt;
}

HeapProfilerImpl::DispatcherImpl::Method HeapProfilerImpl::DispatcherImpl::GetMethodEnum(const std::string& method)
{
    if (method == "addInspectedHeapObject") {
        return Method::ADD_INSPECTED_HEAP_OBJECT;
    } else if (method == "collectGarbage") {
        return Method::COLLECT_GARBAGE;
    } else if (method == "enable") {
        return Method::ENABLE;
    } else if (method == "disable") {
        return Method::DISABLE;
    } else if (method == "getHeapObjectId") {
        return Method::GET_HEAP_OBJECT_ID;
    } else if (method == "getObjectByHeapObjectId") {
        return Method::GET_OBJECT_BY_HEAP_OBJECT_ID;
    } else if (method == "getSamplingProfile") {
        return Method::GET_SAMPLING_PROFILE;
    } else if (method == "startSampling") {
        return Method::START_SAMPLING;
    } else if (method == "startTrackingHeapObjects") {
        return Method::START_TRACKING_HEAP_OBJECTS;
    } else if (method == "stopSampling") {
        return Method::STOP_SAMPLING;
    } else if (method == "stopTrackingHeapObjects") {
        return Method::STOP_TRACKING_HEAP_OBJECTS;
    } else if (method == "takeHeapSnapshot") {
        return Method::TAKE_HEAP_SNAPSHOT;
    } else {
        return Method::UNKNOWN;
    }
}

void HeapProfilerImpl::DispatcherImpl::AddInspectedHeapObject(const DispatchRequest &request)
{
    std::unique_ptr<AddInspectedHeapObjectParams> params = AddInspectedHeapObjectParams::Create(request.GetParams());
    if (params == nullptr) {
        SendResponse(request, DispatchResponse::Fail("wrong params"));
        return;
    }
    DispatchResponse response = heapprofiler_->AddInspectedHeapObject(*params);
    SendResponse(request, response);
}

void HeapProfilerImpl::DispatcherImpl::CollectGarbage(const DispatchRequest &request)
{
    DispatchResponse response = heapprofiler_->CollectGarbage();
    SendResponse(request, response);
}

void HeapProfilerImpl::DispatcherImpl::Enable(const DispatchRequest &request)
{
    DispatchResponse response = heapprofiler_->Enable();
    heapprofiler_->InitializeExtendedProtocolsList();
    EnableReturns result(heapprofiler_->heapProfilerExtendedProtocols_);
    SendResponse(request, response, result);
}

void HeapProfilerImpl::DispatcherImpl::Disable(const DispatchRequest &request)
{
    DispatchResponse response = heapprofiler_->Disable();
    SendResponse(request, response);
}

void HeapProfilerImpl::DispatcherImpl::GetHeapObjectId(const DispatchRequest &request)
{
    std::unique_ptr<GetHeapObjectIdParams> params = GetHeapObjectIdParams::Create(request.GetParams());
    if (params == nullptr) {
        SendResponse(request, DispatchResponse::Fail("wrong params"));
        return;
    }

    HeapSnapshotObjectId objectId;
    DispatchResponse response = heapprofiler_->GetHeapObjectId(*params, &objectId);
    GetHeapObjectIdReturns result(std::move(objectId));
    SendResponse(request, response, result);
}

void HeapProfilerImpl::DispatcherImpl::GetObjectByHeapObjectId(const DispatchRequest &request)
{
    std::unique_ptr<GetObjectByHeapObjectIdParams> params = GetObjectByHeapObjectIdParams::Create(request.GetParams());
    if (params == nullptr) {
        SendResponse(request, DispatchResponse::Fail("wrong params"));
        return;
    }

    std::unique_ptr<RemoteObject> remoteObjectResult;
    DispatchResponse response = heapprofiler_->GetObjectByHeapObjectId(*params, &remoteObjectResult);
    if (remoteObjectResult == nullptr) {
        SendResponse(request, response);
        return;
    }

    GetObjectByHeapObjectIdReturns result(std::move(remoteObjectResult));
    SendResponse(request, response, result);
}

void HeapProfilerImpl::DispatcherImpl::GetSamplingProfile(const DispatchRequest &request)
{
    std::unique_ptr<SamplingHeapProfile> profile;
    DispatchResponse response = heapprofiler_->GetSamplingProfile(&profile);
    if (profile == nullptr) {
        SendResponse(request, response);
        return;
    }

    // The return value type of GetSamplingProfile is the same as of StopSampling.
    StopSamplingReturns result(std::move(profile));
    SendResponse(request, response, result);
}

void HeapProfilerImpl::DispatcherImpl::StartSampling(const DispatchRequest &request)
{
    std::unique_ptr<StartSamplingParams> params = StartSamplingParams::Create(request.GetParams());
    if (params == nullptr) {
        SendResponse(request, DispatchResponse::Fail("wrong params"));
        return;
    }
    DispatchResponse response = heapprofiler_->StartSampling(*params);
    SendResponse(request, response);
}

void HeapProfilerImpl::DispatcherImpl::StopSampling(const DispatchRequest &request)
{
    std::unique_ptr<SamplingHeapProfile> profile;
    DispatchResponse response = heapprofiler_->StopSampling(&profile);
    if (profile == nullptr) {
        SendResponse(request, response);
        return;
    }

    StopSamplingReturns result(std::move(profile));
    SendResponse(request, response, result);
}

void HeapProfilerImpl::DispatcherImpl::StartTrackingHeapObjects(const DispatchRequest &request)
{
    std::unique_ptr<StartTrackingHeapObjectsParams> params =
        StartTrackingHeapObjectsParams::Create(request.GetParams());
    if (params == nullptr) {
        SendResponse(request, DispatchResponse::Fail("wrong params"));
        return;
    }
    DispatchResponse response = heapprofiler_->StartTrackingHeapObjects(*params);
    SendResponse(request, response);
}

void HeapProfilerImpl::DispatcherImpl::StopTrackingHeapObjects(const DispatchRequest &request)
{
    std::unique_ptr<StopTrackingHeapObjectsParams> params = StopTrackingHeapObjectsParams::Create(request.GetParams());
    if (params == nullptr) {
        SendResponse(request, DispatchResponse::Fail("wrong params"));
        return;
    }
    DispatchResponse response = heapprofiler_->StopTrackingHeapObjects(*params);
    SendResponse(request, response);
}

void HeapProfilerImpl::DispatcherImpl::TakeHeapSnapshot(const DispatchRequest &request)
{
    std::unique_ptr<StopTrackingHeapObjectsParams> params = StopTrackingHeapObjectsParams::Create(request.GetParams());
    if (params == nullptr) {
        SendResponse(request, DispatchResponse::Fail("wrong params"));
        return;
    }
    DispatchResponse response = heapprofiler_->TakeHeapSnapshot(*params);
    SendResponse(request, response);
}

bool HeapProfilerImpl::Frontend::AllowNotify() const
{
    return channel_ != nullptr;
}

void HeapProfilerImpl::Frontend::AddHeapSnapshotChunk(char *data, int32_t size)
{
    if (!AllowNotify()) {
        return;
    }

    tooling::AddHeapSnapshotChunk addHeapSnapshotChunk;
    addHeapSnapshotChunk.GetChunk().resize(size);
    for (int32_t i = 0; i < size; ++i) {
        addHeapSnapshotChunk.GetChunk()[i] = data[i];
    }

    channel_->SendNotification(addHeapSnapshotChunk);
}

void HeapProfilerImpl::Frontend::ReportHeapSnapshotProgress(int32_t done, int32_t total)
{
    if (!AllowNotify()) {
        return;
    }

    tooling::ReportHeapSnapshotProgress reportHeapSnapshotProgress;
    reportHeapSnapshotProgress.SetDone(done).SetTotal(total);
    if (done >= total) {
        reportHeapSnapshotProgress.SetFinished(true);
    }
    channel_->SendNotification(reportHeapSnapshotProgress);
}

void HeapProfilerImpl::Frontend::HeapStatsUpdate(HeapStat* updateData, int32_t count)
{
    if (!AllowNotify()) {
        return;
    }
    std::vector<int32_t> statsDiff;
    for (int32_t i = 0; i < count; ++i) {
        statsDiff.emplace_back(updateData[i].index_);
        statsDiff.emplace_back(updateData[i].count_);
        statsDiff.emplace_back(updateData[i].size_);
    }
    tooling::HeapStatsUpdate heapStatsUpdate;
    heapStatsUpdate.SetStatsUpdate(std::move(statsDiff));
    channel_->SendNotification(heapStatsUpdate);
}

void HeapProfilerImpl::Frontend::LastSeenObjectId(int32_t lastSeenObjectId, int64_t timeStampUs)
{
    if (!AllowNotify()) {
        return;
    }

    tooling::LastSeenObjectId lastSeenObjectIdEvent;
    lastSeenObjectIdEvent.SetLastSeenObjectId(lastSeenObjectId);
    const int THOUSAND = 1000;
    double timestampMS = static_cast<double>(timeStampUs) / THOUSAND;
    lastSeenObjectIdEvent.SetTimestamp(timestampMS);
    channel_->SendNotification(lastSeenObjectIdEvent);
}

void HeapProfilerImpl::Frontend::ResetProfiles()
{
    if (!AllowNotify()) {
        return;
    }
}

HeapProfilerImpl::~HeapProfilerImpl()
{
    uv_loop_t *loop = reinterpret_cast<uv_loop_t *>(vm_->GetLoop());
    if (handle_!= nullptr && loop != nullptr) {
        uv_close(reinterpret_cast<uv_handle_t*>(handle_), [](uv_handle_t* handle) {
            delete reinterpret_cast<uv_timer_t*>(handle);
            handle = nullptr;
        });
    }
}

DispatchResponse HeapProfilerImpl::AddInspectedHeapObject([[maybe_unused]] const AddInspectedHeapObjectParams &params)
{
    return DispatchResponse::Fail("AddInspectedHeapObject not support now");
}

DispatchResponse HeapProfilerImpl::CollectGarbage()
{
    panda::JSNApi::TriggerGC(vm_, panda::JSNApi::TRIGGER_GC_TYPE::FULL_GC);
    panda::JSNApi::TriggerGC(vm_, panda::JSNApi::TRIGGER_GC_TYPE::SHARED_FULL_GC);
    return DispatchResponse::Ok();
}

DispatchResponse HeapProfilerImpl::Enable()
{
    return DispatchResponse::Ok();
}

DispatchResponse HeapProfilerImpl::Disable()
{
    panda::DFXJSNApi::DestroyHeapProfiler(vm_);
    return DispatchResponse::Ok();
}

DispatchResponse HeapProfilerImpl::GetHeapObjectId([[maybe_unused]] const GetHeapObjectIdParams &params,
    HeapSnapshotObjectId *objectId)
{
    ASSERT(objectId != nullptr);
    *objectId = 0;
    return DispatchResponse::Fail("GetHeapObjectId not support now");
}

DispatchResponse HeapProfilerImpl::GetObjectByHeapObjectId([[maybe_unused]] const GetObjectByHeapObjectIdParams &params,
    [[maybe_unused]] std::unique_ptr<RemoteObject> *remoteObjectResult)
{
    return DispatchResponse::Fail("GetObjectByHeapObjectId not support now");
}

DispatchResponse HeapProfilerImpl::GetSamplingProfile([[maybe_unused]] std::unique_ptr<SamplingHeapProfile> *profile)
{
    auto samplingInfo = panda::DFXJSNApi::GetAllocationProfile(vm_);
    if (samplingInfo == nullptr) {
        return DispatchResponse::Fail("GetSamplingProfile fail");
    }
    *profile = SamplingHeapProfile::FromSamplingInfo(std::move(samplingInfo));
    return DispatchResponse::Ok();
}

DispatchResponse HeapProfilerImpl::StartSampling([[maybe_unused]] const StartSamplingParams &params)
{
    panda::JSNApi::SetProfilerState(vm_, true);
    uint64_t samplingInterval = static_cast<uint64_t>(params.GetSamplingInterval());
    bool result = panda::DFXJSNApi::StartSampling(vm_, samplingInterval);
    if (result) {
        return DispatchResponse::Ok();
    }
    return DispatchResponse::Fail("StartSampling fail");
}

DispatchResponse HeapProfilerImpl::StopSampling([[maybe_unused]] std::unique_ptr<SamplingHeapProfile> *profile)
{
    DispatchResponse samplingProfile = GetSamplingProfile(profile);
    if (samplingProfile.IsOk()) {
        panda::DFXJSNApi::StopSampling(vm_);
        panda::JSNApi::SetProfilerState(vm_, false);
        return DispatchResponse::Ok();
    }
    return DispatchResponse::Fail("StopSampling fail");
}

DispatchResponse HeapProfilerImpl::StartTrackingHeapObjects(const StartTrackingHeapObjectsParams &params)
{
    panda::JSNApi::SetProfilerState(vm_, true);
    if (handle_ != nullptr && uv_is_active(reinterpret_cast<uv_handle_t*>(handle_))) {
        return DispatchResponse::Ok();
    }
    bool traceAllocation = params.GetTrackAllocations();
    bool result = panda::DFXJSNApi::StartHeapTracking(vm_, INTERVAL, true, &stream_, traceAllocation, false);

    uv_loop_t *loop = reinterpret_cast<uv_loop_t *>(vm_->GetLoop());
    if (loop == nullptr) {
        return DispatchResponse::Fail("Loop is nullptr");
    }
    if (handle_ == nullptr) {
        handle_ = new uv_timer_t;
    }
    uv_timer_init(loop, handle_);
    handle_->data = this;
    uv_timer_start(handle_, HeapTrackingCallback, 0, INTERVAL * MILLI_TO_MICRO);
    if (DebuggerApi::IsMainThread()) {
        uv_async_send(&loop->wq_async);
    } else {
        uv_work_t *work = new uv_work_t;
        uv_queue_work(loop, work, [](uv_work_t *) { }, [](uv_work_t *work, int32_t) { delete work; });
    }

    if (result) {
        return DispatchResponse::Ok();
    } else {
        return DispatchResponse::Fail("StartHeapTracking fail");
    }
}

void HeapProfilerImpl::HeapTrackingCallback(uv_timer_t* handle)
{
    HeapProfilerImpl *heapProfilerImpl = static_cast<HeapProfilerImpl *>(handle->data);
    if (heapProfilerImpl == nullptr) {
        return;
    }
    panda::DFXJSNApi::UpdateHeapTracking(heapProfilerImpl->vm_, &(heapProfilerImpl->stream_));
}

DispatchResponse HeapProfilerImpl::StopTrackingHeapObjects(const StopTrackingHeapObjectsParams &params)
{
    if (handle_ != nullptr) {
        uv_timer_stop(handle_);
    }
    bool result = false;
    if (params.GetReportProgress()) {
        HeapProfilerProgress progress(&frontend_);
        result = panda::DFXJSNApi::StopHeapTracking(vm_, &stream_, &progress, false);
    } else {
        result = panda::DFXJSNApi::StopHeapTracking(vm_, &stream_, nullptr, false);
    }
    if (result) {
        panda::JSNApi::SetProfilerState(vm_, false);
        return DispatchResponse::Ok();
    } else {
        return DispatchResponse::Fail("StopHeapTracking fail");
    }
}

DispatchResponse HeapProfilerImpl::TakeHeapSnapshot(const StopTrackingHeapObjectsParams &params)
{
    bool captureNumericValue = params.GetCaptureNumericValue();
    DumpSnapShotOption dumpOption;
    dumpOption.dumpFormat = DumpFormat::JSON;
    dumpOption.isVmMode = true;
    dumpOption.isPrivate = false;
    dumpOption.captureNumericValue = captureNumericValue;
    if (params.GetReportProgress()) {
        HeapProfilerProgress progress(&frontend_);
        panda::DFXJSNApi::DumpHeapSnapshot(vm_, &stream_, dumpOption, &progress);
    } else {
        panda::DFXJSNApi::DumpHeapSnapshot(vm_, &stream_, dumpOption);
    }
    return DispatchResponse::Ok();
}
}  // namespace panda::ecmascript::tooling
