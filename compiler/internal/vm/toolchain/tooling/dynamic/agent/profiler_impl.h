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

#ifndef ECMASCRIPT_TOOLING_AGENT_PROFILER_IMPL_H
#define ECMASCRIPT_TOOLING_AGENT_PROFILER_IMPL_H

#include "tooling/dynamic/base/pt_params.h"
#include "tooling/dynamic/base/pt_returns.h"
#include "dispatcher.h"

#include "ecmascript/dfx/cpu_profiler/samples_record.h"
#include "libpandabase/macros.h"

namespace panda::ecmascript::tooling {
class ProfilerImpl final {
public:
    ProfilerImpl(const EcmaVM *vm, ProtocolChannel *channel) : vm_(vm), frontend_(channel) {}
    ~ProfilerImpl() = default;

    DispatchResponse Disable();
    DispatchResponse Enable();
    DispatchResponse Start();
    DispatchResponse Stop(std::unique_ptr<Profile> *profile);
    DispatchResponse SetSamplingInterval(const SetSamplingIntervalParams &params);
    DispatchResponse GetBestEffortCoverage();
    DispatchResponse StopPreciseCoverage();
    DispatchResponse TakePreciseCoverage();
    DispatchResponse StartPreciseCoverage(const StartPreciseCoverageParams &params);
    DispatchResponse StartTypeProfile();
    DispatchResponse StopTypeProfile();
    DispatchResponse TakeTypeProfile();
    DispatchResponse EnableSerializationTimeoutCheck(const SeriliazationTimeoutCheckEnableParams &params);
    DispatchResponse DisableSerializationTimeoutCheck();

    class DispatcherImpl final : public DispatcherBase {
    public:
        DispatcherImpl(ProtocolChannel *channel, std::unique_ptr<ProfilerImpl> profiler)
            : DispatcherBase(channel), profiler_(std::move(profiler)) {}
        ~DispatcherImpl() override = default;

        std::optional<std::string> Dispatch(const DispatchRequest &request, bool crossLanguageDebug = false) override;
        void Enable(const DispatchRequest &request);
        void Disable(const DispatchRequest &request);
        void Start(const DispatchRequest &request);
        void Stop(const DispatchRequest &request);
        void SetSamplingInterval(const DispatchRequest &request);
        void GetBestEffortCoverage(const DispatchRequest &request);
        void StopPreciseCoverage(const DispatchRequest &request);
        void TakePreciseCoverage(const DispatchRequest &request);
        void StartPreciseCoverage(const DispatchRequest &request);
        void StartTypeProfile(const DispatchRequest &request);
        void StopTypeProfile(const DispatchRequest &request);
        void TakeTypeProfile(const DispatchRequest &request);
        void EnableSerializationTimeoutCheck(const DispatchRequest &request);
        void DisableSerializationTimeoutCheck(const DispatchRequest &request);

        enum class Method {
            DISABLE,
            ENABLE,
            START,
            STOP,
            SET_SAMPLING_INTERVAL,
            GET_BEST_EFFORT_COVERAGE,
            STOP_PRECISE_COVERAGE,
            TAKE_PRECISE_COVERAGE,
            START_PRECISE_COVERAGE,
            START_TYPE_PROFILE,
            STOP_TYPE_PROFILE,
            TAKE_TYPE_PROFILE,
            ENABLE_SERIALIZATION_TIMEOUT_CHECK,
            DISABLE_SERIALIZATION_TIMEOUT_CHECK,
            UNKNOWN
        };
        Method GetMethodEnum(const std::string& method);

    private:
        NO_COPY_SEMANTIC(DispatcherImpl);
        NO_MOVE_SEMANTIC(DispatcherImpl);

        std::unique_ptr<ProfilerImpl> profiler_ {};
    };

    class Frontend {
    public:
        explicit Frontend(ProtocolChannel *channel) : channel_(channel) {}
        ~Frontend() = default;

        void PreciseCoverageDeltaUpdate();

    private:
        bool AllowNotify() const;

        ProtocolChannel *channel_ {nullptr};
    };

private:
    NO_COPY_SEMANTIC(ProfilerImpl);
    NO_MOVE_SEMANTIC(ProfilerImpl);

    void InitializeExtendedProtocolsList();

    const EcmaVM *vm_ {nullptr};
    [[maybe_unused]] Frontend frontend_;
    std::vector<std::string> profilerExtendedProtocols_ {};
};
}  // namespace panda::ecmascript::tooling
#endif
