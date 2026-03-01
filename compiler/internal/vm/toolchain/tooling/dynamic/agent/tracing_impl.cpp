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

#include "agent/tracing_impl.h"

#include "protocol_channel.h"

namespace panda::ecmascript::tooling {
std::optional<std::string> TracingImpl::DispatcherImpl::Dispatch(const DispatchRequest &request,
    [[maybe_unused]] bool crossLanguageDebug)
{
    Method method = GetMethodEnum(request.GetMethod());
    LOG_DEBUGGER(DEBUG) << "dispatch [" << request.GetMethod() << "] to TracingImpl";
    switch (method) {
        case Method::END:
            End(request);
            break;
        case Method::GET_CATEGORIES:
            GetCategories(request);
            break;
        case Method::RECORD_CLOCK_SYNC_MARKER:
            RecordClockSyncMarker(request);
            break;
        case Method::REQUEST_MEMORY_DUMP:
            RequestMemoryDump(request);
            break;
        case Method::START:
            Start(request);
            break;
        default:
            SendResponse(request, DispatchResponse::Fail("Unknown method: " + request.GetMethod()));
            break;
    }
    return std::nullopt;
}

TracingImpl::DispatcherImpl::Method TracingImpl::DispatcherImpl::GetMethodEnum(const std::string& method)
{
    if (method == "end") {
        return Method::END;
    } else if (method == "getCategories") {
        return Method::GET_CATEGORIES;
    } else if (method == "recordClockSyncMarker") {
        return Method::RECORD_CLOCK_SYNC_MARKER;
    } else if (method == "requestMemoryDump") {
        return Method::REQUEST_MEMORY_DUMP;
    } else if (method == "start") {
        return Method::START;
    } else {
        return Method::UNKNOWN;
    }
}

void TracingImpl::DispatcherImpl::End(const DispatchRequest &request)
{
    auto traceEvents = tracing_->End();
    if (traceEvents == nullptr) {
        LOG_DEBUGGER(ERROR) << "Transfer DFXJSNApi::StopTracing is failure";
        SendResponse(request, DispatchResponse::Fail("Stop is failure"));
        return;
    }
    SendResponse(request, DispatchResponse::Ok());

    tracing_->frontend_.DataCollected(std::move(traceEvents));
    tracing_->frontend_.TracingComplete();
}

void TracingImpl::DispatcherImpl::GetCategories(const DispatchRequest &request)
{
    std::vector<std::string> categories;
    DispatchResponse response = tracing_->GetCategories(categories);
    SendResponse(request, response);
}

void TracingImpl::DispatcherImpl::RecordClockSyncMarker(const DispatchRequest &request)
{
    std::string syncId;
    DispatchResponse response = tracing_->RecordClockSyncMarker(syncId);
    SendResponse(request, response);
}

void TracingImpl::DispatcherImpl::RequestMemoryDump(const DispatchRequest &request)
{
    std::unique_ptr<RequestMemoryDumpParams> params =
        RequestMemoryDumpParams::Create(request.GetParams());
    if (params == nullptr) {
        SendResponse(request, DispatchResponse::Fail("wrong params"));
        return;
    }
    std::string dumpGuid;
    bool success = false;
    DispatchResponse response = tracing_->RequestMemoryDump(std::move(params), dumpGuid, success);
    SendResponse(request, response);
}

void TracingImpl::DispatcherImpl::Start(const DispatchRequest &request)
{
    std::unique_ptr<StartParams> params =
        StartParams::Create(request.GetParams());
    if (params == nullptr) {
        SendResponse(request, DispatchResponse::Fail("wrong params"));
        return;
    }
    DispatchResponse response = tracing_->Start(std::move(params));
    SendResponse(request, response);
}

bool TracingImpl::Frontend::AllowNotify() const
{
    return channel_ != nullptr;
}

void TracingImpl::Frontend::BufferUsage(double percentFull, int32_t eventCount, double value)
{
    if (!AllowNotify()) {
        return;
    }

    tooling::BufferUsage bufferUsage;
    bufferUsage.SetPercentFull(percentFull).SetEventCount(eventCount).SetValue(value);
    channel_->SendNotification(bufferUsage);
}

void TracingImpl::Frontend::DataCollected(std::unique_ptr<std::vector<TraceEvent>> traceEvents)
{
    if (!AllowNotify()) {
        return;
    }

    tooling::DataCollected dataCollected;
    dataCollected.SetTraceEvents(std::move(traceEvents));

    channel_->SendNotification(dataCollected);
}

void TracingImpl::Frontend::TracingComplete()
{
    if (!AllowNotify()) {
        return;
    }

    tooling::TracingComplete tracingComplete;
    channel_->SendNotification(tracingComplete);
}

std::unique_ptr<std::vector<TraceEvent>> TracingImpl::End()
{
#if defined(ECMASCRIPT_SUPPORT_TRACING)
    if (handle_ != nullptr) {
        uv_timer_stop(handle_);
    }
#endif
    auto traceEvents = panda::DFXJSNApi::StopTracing(vm_);
    return traceEvents;
}

DispatchResponse TracingImpl::GetCategories([[maybe_unused]] std::vector<std::string> categories)
{
    return DispatchResponse::Fail("GetCategories not support now.");
}

DispatchResponse TracingImpl::RecordClockSyncMarker([[maybe_unused]] std::string syncId)
{
    return DispatchResponse::Fail("RecordClockSyncMarker not support now.");
}

DispatchResponse TracingImpl::RequestMemoryDump([[maybe_unused]] std::unique_ptr<RequestMemoryDumpParams> params,
                                                [[maybe_unused]] std::string dumpGuid, [[maybe_unused]] bool success)
{
    return DispatchResponse::Fail("RequestMemoryDump not support now.");
}

DispatchResponse TracingImpl::Start(std::unique_ptr<StartParams> params)
{
    std::string categories = GetCategoriesFromStartParams(params);
    if (!panda::DFXJSNApi::StartTracing(vm_, categories)) {
        return DispatchResponse::Fail("Start tracing failed");
    }
#if defined(ECMASCRIPT_SUPPORT_TRACING)
    if (params->HasBufferUsageReportingInterval()) {
        LOG_DEBUGGER(ERROR) << "HasBufferUsageReportingInterval " << params->GetBufferUsageReportingInterval();
        if (handle_ != nullptr && uv_is_active(reinterpret_cast<uv_handle_t*>(handle_))) {
            LOG_DEBUGGER(ERROR) << "uv_is_active!!!";
            return DispatchResponse::Ok();
        }

        uv_loop_t *loop = reinterpret_cast<uv_loop_t *>(vm_->GetLoop());
        if (loop == nullptr) {
            return DispatchResponse::Fail("Loop is nullptr");
        }
        if (handle_ == nullptr) {
            handle_ = new uv_timer_t;
        }
        uv_timer_init(loop, handle_);
        handle_->data = this;
        uv_timer_start(handle_, TracingBufferUsageReport, 0, params->GetBufferUsageReportingInterval());
        if (DebuggerApi::IsMainThread()) {
            uv_async_send(&loop->wq_async);
        } else {
            uv_work_t *work = new uv_work_t;
            uv_queue_work(loop, work, [](uv_work_t *) { }, [](uv_work_t *work, int32_t) { delete work; });
        }
    }
#endif
    return DispatchResponse::Ok();
}

std::string TracingImpl::GetCategoriesFromStartParams(const std::unique_ptr<StartParams>& params)
{
    if (!params) {
        return {};
    }

    std::string categories = params->GetCategories();
    if (!categories.empty()) {
        return categories;
    }

    const TraceConfig* traceConfig = params->GetTraceConfig();
    if (!traceConfig) {
        return {};
    }

    const auto* includeCategories = traceConfig->GetIncludedCategories();
    if (!includeCategories || includeCategories->empty()) {
        return {};
    }

    std::ostringstream oss;
    for (size_t i = 0; i < includeCategories->size(); ++i) {
        if (i > 0) {
            oss << ",";
        }
        oss << (*includeCategories)[i];
    }
    return oss.str();
}

TracingImpl::~TracingImpl()
{
#if defined(ECMASCRIPT_SUPPORT_TRACING)
    uv_loop_t *loop = reinterpret_cast<uv_loop_t *>(vm_->GetLoop());
    if (handle_!= nullptr && loop != nullptr) {
        uv_close(reinterpret_cast<uv_handle_t*>(handle_), [](uv_handle_t* handle) {
            delete reinterpret_cast<uv_timer_t*>(handle);
            handle = nullptr;
        });
    }
#endif
}

#if defined(ECMASCRIPT_SUPPORT_TRACING)
void TracingImpl::TracingBufferUsageReport(uv_timer_t* handle)
{
    TracingImpl *tracing = static_cast<TracingImpl *>(handle->data);
    if (tracing == nullptr) {
        LOG_DEBUGGER(ERROR) << "tracing == nullptr";
        return;
    }

    double percentFull = 0.0;
    uint32_t eventCount = 0;
    double value = 0.0;
    panda::DFXJSNApi::GetTracingBufferUseage(tracing->vm_, percentFull, eventCount, value);
    tracing->frontend_.BufferUsage(percentFull, eventCount, value);
}
#endif
}  // namespace panda::ecmascript::tooling