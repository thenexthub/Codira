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

#ifndef ECMASCRIPT_TOOLING_AGENT_TRACING_IMPL_H
#define ECMASCRIPT_TOOLING_AGENT_TRACING_IMPL_H

#if defined(ECMASCRIPT_SUPPORT_TRACING)
#include <uv.h>
#endif

#include "tooling/dynamic/base/pt_params.h"
#include "tooling/dynamic/base/pt_returns.h"
#include "dispatcher.h"

#include "ecmascript/dfx/cpu_profiler/samples_record.h"
#include "ecmascript/dfx/tracing/tracing.h"
#include "libpandabase/macros.h"

namespace panda::ecmascript::tooling {
class TracingImpl final {
public:
    explicit TracingImpl(const EcmaVM *vm, ProtocolChannel *channel) : vm_(vm), frontend_(channel) {}
    ~TracingImpl();

    std::unique_ptr<std::vector<TraceEvent>> End();
    DispatchResponse GetCategories(std::vector<std::string> categories);
    DispatchResponse RecordClockSyncMarker(std::string syncId);
    DispatchResponse RequestMemoryDump(std::unique_ptr<RequestMemoryDumpParams> params,
                                       std::string dumpGuid,  bool success);
    DispatchResponse Start(std::unique_ptr<StartParams> params);
#if defined(ECMASCRIPT_SUPPORT_TRACING)
    static void TracingBufferUsageReport(uv_timer_t* handle);
#endif
    static std::string GetCategoriesFromStartParams(const std::unique_ptr<StartParams>& params);

    class DispatcherImpl final : public DispatcherBase {
    public:
        DispatcherImpl(ProtocolChannel *channel, std::unique_ptr<TracingImpl> tracing)
            : DispatcherBase(channel), tracing_(std::move(tracing)) {}
        ~DispatcherImpl() override = default;
        std::optional<std::string> Dispatch(const DispatchRequest &request, bool crossLanguageDebug = false) override;
        void End(const DispatchRequest &request);
        void GetCategories(const DispatchRequest &request);
        void RecordClockSyncMarker(const DispatchRequest &request);
        void RequestMemoryDump(const DispatchRequest &request);
        void Start(const DispatchRequest &request);

        enum class Method {
            END,
            GET_CATEGORIES,
            RECORD_CLOCK_SYNC_MARKER,
            REQUEST_MEMORY_DUMP,
            START,
            UNKNOWN
        };
        Method GetMethodEnum(const std::string& method);

    private:
        NO_COPY_SEMANTIC(DispatcherImpl);
        NO_MOVE_SEMANTIC(DispatcherImpl);

        std::unique_ptr<TracingImpl> tracing_ {};
    };

    class Frontend {
    public:
        explicit Frontend(ProtocolChannel *channel) : channel_(channel) {}
        ~Frontend() = default;

        void BufferUsage(double percentFull, int32_t eventCount, double value);
        void DataCollected(std::unique_ptr<std::vector<TraceEvent>> traceEvents);
        void TracingComplete();

    private:
        bool AllowNotify() const;
        ProtocolChannel *channel_ {nullptr};
    };

private:
    NO_COPY_SEMANTIC(TracingImpl);
    NO_MOVE_SEMANTIC(TracingImpl);

    const EcmaVM *vm_ {nullptr};
    Frontend frontend_;
#if defined(ECMASCRIPT_SUPPORT_TRACING)
    uv_timer_t *handle_ {nullptr};
#endif
};
}  // namespace panda::ecmascript::tooling
#endif