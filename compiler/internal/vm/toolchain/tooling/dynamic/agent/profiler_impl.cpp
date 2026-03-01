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

#include "agent/profiler_impl.h"

#include "protocol_channel.h"

namespace panda::ecmascript::tooling {
// Whenever adding a new protocol which is not a standard CDP protocol,
// must add its methodName to the profilerProtocolList
void ProfilerImpl::InitializeExtendedProtocolsList()
{
    std::vector<std::string> profilerProtocolList {
        "startTypeProfile",
        "stopTypeProfile",
        "takeTypeProfile",
        "enableSerializationTimeoutCheck",
        "disableSerializationTimeoutCheck"
    };
    profilerExtendedProtocols_ = std::move(profilerProtocolList);
}

std::optional<std::string> ProfilerImpl::DispatcherImpl::Dispatch(const DispatchRequest &request,
    [[maybe_unused]] bool crossLanguageDebug)
{
    Method method = GetMethodEnum(request.GetMethod());
    LOG_DEBUGGER(DEBUG) << "dispatch [" << request.GetMethod() << "] to ProfilerImpl";
    switch (method) {
        case Method::DISABLE:
            Disable(request);
            break;
        case Method::ENABLE:
            Enable(request);
            break;
        case Method::START:
            Start(request);
            break;
        case Method::STOP:
            Stop(request);
            break;
        case Method::SET_SAMPLING_INTERVAL:
            SetSamplingInterval(request);
            break;
        case Method::GET_BEST_EFFORT_COVERAGE:
            GetBestEffortCoverage(request);
            break;
        case Method::STOP_PRECISE_COVERAGE:
            StopPreciseCoverage(request);
            break;
        case Method::TAKE_PRECISE_COVERAGE:
            TakePreciseCoverage(request);
            break;
        case Method::START_PRECISE_COVERAGE:
            StartPreciseCoverage(request);
            break;
        case Method::START_TYPE_PROFILE:
            StartTypeProfile(request);
            break;
        case Method::STOP_TYPE_PROFILE:
            StopTypeProfile(request);
            break;
        case Method::TAKE_TYPE_PROFILE:
            TakeTypeProfile(request);
            break;
        case Method::ENABLE_SERIALIZATION_TIMEOUT_CHECK:
            EnableSerializationTimeoutCheck(request);
            break;
        case Method::DISABLE_SERIALIZATION_TIMEOUT_CHECK:
            DisableSerializationTimeoutCheck(request);
            break;
        default:
            SendResponse(request, DispatchResponse::Fail("Unknown method: " + request.GetMethod()));
            break;
    }
    return std::nullopt;
}

ProfilerImpl::DispatcherImpl::Method ProfilerImpl::DispatcherImpl::GetMethodEnum(const std::string& method)
{
    if (method == "disable") {
        return Method::DISABLE;
    } else if (method == "enable") {
        return Method::ENABLE;
    } else if (method == "start") {
        return Method::START;
    } else if (method == "stop") {
        return Method::STOP;
    } else if (method == "setSamplingInterval") {
        return Method::SET_SAMPLING_INTERVAL;
    } else if (method == "getBestEffortCoverage") {
        return Method::GET_BEST_EFFORT_COVERAGE;
    } else if (method == "stopPreciseCoverage") {
        return Method::STOP_PRECISE_COVERAGE;
    } else if (method == "takePreciseCoverage") {
        return Method::TAKE_PRECISE_COVERAGE;
    } else if (method == "startPreciseCoverage") {
        return Method::START_PRECISE_COVERAGE;
    } else if (method == "startTypeProfile") {
        return Method::START_TYPE_PROFILE;
    } else if (method == "stopTypeProfile") {
        return Method::STOP_TYPE_PROFILE;
    } else if (method == "takeTypeProfile") {
        return Method::TAKE_TYPE_PROFILE;
    } else if (method == "enableSerializationTimeoutCheck") {
        return Method::ENABLE_SERIALIZATION_TIMEOUT_CHECK;
    } else if (method == "disableSerializationTimeoutCheck") {
        return Method::DISABLE_SERIALIZATION_TIMEOUT_CHECK;
    } else {
        return Method::UNKNOWN;
    }
}

void ProfilerImpl::DispatcherImpl::Disable(const DispatchRequest &request)
{
    DispatchResponse response = profiler_->Disable();
    SendResponse(request, response);
}

void ProfilerImpl::DispatcherImpl::Enable(const DispatchRequest &request)
{
    DispatchResponse response = profiler_->Enable();
    profiler_->InitializeExtendedProtocolsList();
    EnableReturns result(profiler_->profilerExtendedProtocols_);
    SendResponse(request, response, result);
}

void ProfilerImpl::DispatcherImpl::Start(const DispatchRequest &request)
{
    DispatchResponse response = profiler_->Start();
    SendResponse(request, response);
}

void ProfilerImpl::DispatcherImpl::Stop(const DispatchRequest &request)
{
    std::unique_ptr<Profile> profile;
    DispatchResponse response = profiler_->Stop(&profile);
    if (profile == nullptr) {
        SendResponse(request, response);
        return;
    }

    StopReturns result(std::move(profile));
    SendResponse(request, response, result);
}

void ProfilerImpl::DispatcherImpl::EnableSerializationTimeoutCheck(const DispatchRequest &request)
{
    std::unique_ptr<SeriliazationTimeoutCheckEnableParams> params =
        SeriliazationTimeoutCheckEnableParams::Create(request.GetParams());
    if (params == nullptr) {
        SendResponse(request, DispatchResponse::Fail("wrong params"));
        return;
    }
    DispatchResponse response = profiler_->EnableSerializationTimeoutCheck(*params);
    SendResponse(request, response);
}

void ProfilerImpl::DispatcherImpl::DisableSerializationTimeoutCheck(const DispatchRequest &request)
{
    DispatchResponse response = profiler_->DisableSerializationTimeoutCheck();
    SendResponse(request, response);
}

void ProfilerImpl::DispatcherImpl::SetSamplingInterval(const DispatchRequest &request)
{
    std::unique_ptr<SetSamplingIntervalParams> params = SetSamplingIntervalParams::Create(request.GetParams());
    if (params == nullptr) {
        SendResponse(request, DispatchResponse::Fail("wrong params"));
        return;
    }
    DispatchResponse response = profiler_->SetSamplingInterval(*params);
    SendResponse(request, response);
}

void ProfilerImpl::DispatcherImpl::GetBestEffortCoverage(const DispatchRequest &request)
{
    DispatchResponse response = profiler_->GetBestEffortCoverage();
    SendResponse(request, response);
}

void ProfilerImpl::DispatcherImpl::StopPreciseCoverage(const DispatchRequest &request)
{
    DispatchResponse response = profiler_->StopPreciseCoverage();
    SendResponse(request, response);
}

void ProfilerImpl::DispatcherImpl::TakePreciseCoverage(const DispatchRequest &request)
{
    DispatchResponse response = profiler_->TakePreciseCoverage();
    SendResponse(request, response);
}

void ProfilerImpl::DispatcherImpl::StartPreciseCoverage(const DispatchRequest &request)
{
    std::unique_ptr<StartPreciseCoverageParams> params = StartPreciseCoverageParams::Create(request.GetParams());
    if (params == nullptr) {
        SendResponse(request, DispatchResponse::Fail("wrong params"));
        return;
    }
    DispatchResponse response = profiler_->StartPreciseCoverage(*params);
    SendResponse(request, response);
}

void ProfilerImpl::DispatcherImpl::StartTypeProfile(const DispatchRequest &request)
{
    DispatchResponse response = profiler_->StartTypeProfile();
    SendResponse(request, response);
}

void ProfilerImpl::DispatcherImpl::StopTypeProfile(const DispatchRequest &request)
{
    DispatchResponse response = profiler_->StopTypeProfile();
    SendResponse(request, response);
}

void ProfilerImpl::DispatcherImpl::TakeTypeProfile(const DispatchRequest &request)
{
    DispatchResponse response = profiler_->TakeTypeProfile();
    SendResponse(request, response);
}

bool ProfilerImpl::Frontend::AllowNotify() const
{
    return channel_ != nullptr;
}

void ProfilerImpl::Frontend::PreciseCoverageDeltaUpdate()
{
    if (!AllowNotify()) {
        return;
    }

    tooling::PreciseCoverageDeltaUpdate preciseCoverageDeltaUpdate;
    channel_->SendNotification(preciseCoverageDeltaUpdate);
}

DispatchResponse ProfilerImpl::Disable()
{
    return DispatchResponse::Ok();
}

DispatchResponse ProfilerImpl::Enable()
{
    return DispatchResponse::Ok();
}

DispatchResponse ProfilerImpl::Start()
{
    panda::JSNApi::SetProfilerState(vm_, true);
    bool result = panda::DFXJSNApi::StartCpuProfilerForInfo(vm_);
    if (!result) {
        LOG_DEBUGGER(ERROR) << "ProfilerImpl::Start failed";
        return DispatchResponse::Fail("Start is failure");
    }
    return DispatchResponse::Ok();
}

DispatchResponse ProfilerImpl::Stop(std::unique_ptr<Profile> *profile)
{
    auto profileInfo = panda::DFXJSNApi::StopCpuProfilerForInfo(vm_);
    if (profileInfo == nullptr) {
        LOG_DEBUGGER(ERROR) << "Transfer DFXJSNApi::StopCpuProfilerImpl is failure";
        return DispatchResponse::Fail("Stop is failure");
    }
    *profile = Profile::FromProfileInfo(*profileInfo);
    panda::JSNApi::SetProfilerState(vm_, false);
    return DispatchResponse::Ok();
}

DispatchResponse ProfilerImpl::EnableSerializationTimeoutCheck(const SeriliazationTimeoutCheckEnableParams &params)
{
    int32_t threshhold = params.GetThreshold();
    panda::DFXJSNApi::EnableSeriliazationTimeoutCheck(vm_, threshhold);
    LOG_DEBUGGER(DEBUG) << "Profiler Serialization timeout check is enabled with threshhold: " << threshhold;
    return DispatchResponse::Ok();
}

DispatchResponse ProfilerImpl::DisableSerializationTimeoutCheck()
{
    panda::DFXJSNApi::DisableSeriliazationTimeoutCheck(vm_);
    LOG_DEBUGGER(DEBUG) << "Profiler Serialization check is disabled";
    return DispatchResponse::Ok();
}

DispatchResponse ProfilerImpl::SetSamplingInterval(const SetSamplingIntervalParams &params)
{
    panda::DFXJSNApi::SetCpuSamplingInterval(vm_, params.GetInterval());
    return DispatchResponse::Ok();
}

DispatchResponse ProfilerImpl::GetBestEffortCoverage()
{
    return DispatchResponse::Fail("GetBestEffortCoverage not support now");
}

DispatchResponse ProfilerImpl::StopPreciseCoverage()
{
    return DispatchResponse::Fail("StopPreciseCoverage not support now");
}

DispatchResponse ProfilerImpl::TakePreciseCoverage()
{
    return DispatchResponse::Fail("TakePreciseCoverage not support now");
}

DispatchResponse ProfilerImpl::StartPreciseCoverage([[maybe_unused]] const StartPreciseCoverageParams &params)
{
    return DispatchResponse::Fail("StartPreciseCoverage not support now");
}

DispatchResponse ProfilerImpl::StartTypeProfile()
{
    return DispatchResponse::Fail("StartTypeProfile not support now");
}

DispatchResponse ProfilerImpl::StopTypeProfile()
{
    return DispatchResponse::Fail("StopTypeProfile not support now");
}

DispatchResponse ProfilerImpl::TakeTypeProfile()
{
    return DispatchResponse::Fail("TakeTypeProfile not support now");
}
}  // namespace panda::ecmascript::tooling
