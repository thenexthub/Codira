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

#include "agent/debugger_impl.h"
#ifdef ECMASCRIPT_SUPPORT_HEAPPROFILER
#include "agent/heapprofiler_impl.h"
#endif
#ifdef ECMASCRIPT_SUPPORT_CPUPROFILER
#include "agent/profiler_impl.h"
#endif
#include "agent/animation_impl.h"
#include "agent/css_impl.h"
#include "agent/dom_impl.h"
#include "agent/overlay_impl.h"
#include "agent/page_impl.h"
#include "agent/target_impl.h"
#include "agent/tracing_impl.h"
#include "protocol_channel.h"

namespace panda::ecmascript::tooling {
DispatchRequest::DispatchRequest(const std::string &message)
{
    std::unique_ptr<PtJson> json = PtJson::Parse(message);
    if (json == nullptr) {
        JsonParseError();
        return;
    }
    if (!json->IsObject()) {
        JsonFormatError(json);
        return;
    }

    Result ret;
    int32_t callId;
    ret = json->GetInt("id", &callId);
    if (ret != Result::SUCCESS) {
        code_ = RequestCode::PARSE_ID_ERROR;
        LOG_DEBUGGER(ERROR) << "parse id error";
        return;
    }
    callId_ = callId;

    std::string wholeMethod;
    ret = json->GetString("method", &wholeMethod);
    if (ret != Result::SUCCESS || wholeMethod.empty()) {
        code_ = RequestCode::PARSE_METHOD_ERROR;
        LOG_DEBUGGER(ERROR) << "parse method error";
        return;
    }
    std::string::size_type length = wholeMethod.length();
    std::string::size_type indexPoint = wholeMethod.find_first_of('.', 0);
    if (indexPoint == std::string::npos || indexPoint == 0 || indexPoint == length - 1) {
        code_ = RequestCode::METHOD_FORMAT_ERROR;
        LOG_DEBUGGER(ERROR) << "method format error: " << wholeMethod;
        return;
    }
    domain_ = wholeMethod.substr(0, indexPoint);
    method_ = wholeMethod.substr(indexPoint + 1, length);

    LOG_DEBUGGER(DEBUG) << "id: " << callId_ << ", domain: " << domain_ << ", method: " << method_;

    std::unique_ptr<PtJson> params;
    ret = json->GetObject("params", &params);
    if (ret == Result::NOT_EXIST) {
        return;
    }
    if (ret == Result::TYPE_ERROR) {
        code_ = RequestCode::PARAMS_FORMAT_ERROR;
        LOG_DEBUGGER(ERROR) << "params format error";
        return;
    }
    params_ = std::move(params);
}

DispatchRequest::~DispatchRequest()
{
    params_->ReleaseRoot();
}

DispatchResponse DispatchResponse::Create(ResponseCode code, const std::string &msg)
{
    DispatchResponse response;
    response.code_ = code;
    response.errorMsg_ = msg;
    return response;
}

DispatchResponse DispatchResponse::Create(std::optional<std::string> error)
{
    DispatchResponse response;
    if (error.has_value()) {
        response.code_ = ResponseCode::NOK;
        response.errorMsg_ = error.value();
    }
    return response;
}

DispatchResponse DispatchResponse::Ok()
{
    return DispatchResponse();
}

DispatchResponse DispatchResponse::Fail(const std::string &message)
{
    DispatchResponse response;
    response.code_ = ResponseCode::NOK;
    response.errorMsg_ = message;
    return response;
}

void DispatcherBase::SendResponse(const DispatchRequest &request, const DispatchResponse &response,
                                  const PtBaseReturns &result)
{
    if (channel_ != nullptr) {
        channel_->SendResponse(request, response, result);
    }
}

std::string DispatcherBase::ReturnsValueToString(const int32_t callId, const DispatchResponse& response,
    std::unique_ptr<PtBaseReturns> result)
{
    std::unique_ptr<PtJson> ptJson = PtJson::CreateObject();
    std::unique_ptr<PtJson> resultObj;
    if (result != nullptr && response.IsOk()) {
        resultObj = result->ToJson();
    } else {
        resultObj = DispatchResponseToJson(response);
    }
    ptJson->Add("id", callId);
    ptJson->Add("result", resultObj);
    std::string str = ptJson->Stringify();
    if (str.empty()) {
        LOG_DEBUGGER(ERROR) << "Dispatcher::ReturnsValueToString: json stringify error";
        return "";
    }
    ptJson->ReleaseRoot();
    return str;
}

std::unique_ptr<PtJson> DispatcherBase::DispatchResponseToJson(const DispatchResponse &response) const
{
    std::unique_ptr<PtJson> result = PtJson::CreateObject();

    if (!response.IsOk()) {
        result->Add("code", static_cast<int32_t>(response.GetError()));
        result->Add("message", response.GetMessage().c_str());
    }

    return result;
}

Dispatcher::Dispatcher(const EcmaVM *vm, ProtocolChannel *channel)
{
    // profiler
#ifdef ECMASCRIPT_SUPPORT_CPUPROFILER
    auto profiler = std::make_unique<ProfilerImpl>(vm, channel);
    dispatchers_["Profiler"] =
        std::make_unique<ProfilerImpl::DispatcherImpl>(channel, std::move(profiler));
#endif
#ifdef ECMASCRIPT_SUPPORT_HEAPPROFILER
    auto heapProfiler = std::make_unique<HeapProfilerImpl>(vm, channel);
    dispatchers_["HeapProfiler"] =
        std::make_unique<HeapProfilerImpl::DispatcherImpl>(channel, std::move(heapProfiler));
#endif
#ifdef ECMASCRIPT_SUPPORT_TRACING
    auto tracing = std::make_unique<TracingImpl>(vm, channel);
    dispatchers_["Tracing"] =
        std::make_unique<TracingImpl::DispatcherImpl>(channel, std::move(tracing));
#endif

    // debugger
    auto runtime = std::make_unique<RuntimeImpl>(vm, channel);
    auto debugger = std::make_unique<DebuggerImpl>(vm, channel, runtime.get());
    dispatchers_["Runtime"] =
        std::make_unique<RuntimeImpl::DispatcherImpl>(channel, std::move(runtime));
    dispatchers_["Debugger"] =
        std::make_unique<DebuggerImpl::DispatcherImpl>(channel, std::move(debugger));

    auto dom = std::make_unique<DomImpl>();
    dispatchers_["DOM"] =
        std::make_unique<DomImpl::DispatcherImpl>(channel, std::move(dom));

    auto css = std::make_unique<CssImpl>();
    dispatchers_["CSS"] =
        std::make_unique<CssImpl::DispatcherImpl>(channel, std::move(css));

    auto overlay = std::make_unique<OverlayImpl>();
    dispatchers_["Overlay"] =
        std::make_unique<OverlayImpl::DispatcherImpl>(channel, std::move(overlay));

    auto target = std::make_unique<TargetImpl>();
    dispatchers_["Target"] =
        std::make_unique<TargetImpl::DispatcherImpl>(channel, std::move(target));

    auto page = std::make_unique<PageImpl>();
    dispatchers_["Page"] =
        std::make_unique<PageImpl::DispatcherImpl>(channel, std::move(page));

    auto animation = std::make_unique<AnimationImpl>();
    dispatchers_["Animation"] =
        std::make_unique<AnimationImpl::DispatcherImpl>(channel, std::move(animation));
}

std::optional<std::string> Dispatcher::Dispatch(const DispatchRequest &request, bool crossLanguageDebug) const
{
    if (!request.IsValid()) {
        LOG_DEBUGGER(ERROR) << "Unknown request";
        return std::nullopt;
    }
    const std::string &domain = request.GetDomain();
    auto dispatcher = dispatchers_.find(domain);
    if (dispatcher != dispatchers_.end()) {
        return dispatcher->second->Dispatch(request, crossLanguageDebug);
    } else {
        if (domain == "Test") {
            if (request.GetMethod() == "fail") {
                LOG_DEBUGGER(FATAL) << "Test fail";
                UNREACHABLE();
            }
            LOG_DEBUGGER(INFO) << "Test success";
        } else {
            LOG_DEBUGGER(ERROR) << "unknown domain: " << domain;
        }
    }
    return std::nullopt;
}


std::string Dispatcher::GetJsFrames() const
{
    auto dispatcher = dispatchers_.find("Debugger");
    if (dispatcher != dispatchers_.end()) {
        auto debuggerImpl = reinterpret_cast<DebuggerImpl::DispatcherImpl*>(dispatcher->second.get());
        return debuggerImpl->GetJsFrames();
    }
    return "";
}
}  // namespace panda::ecmascript::tooling
