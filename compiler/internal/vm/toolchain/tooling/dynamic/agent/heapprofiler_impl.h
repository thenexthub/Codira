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

#ifndef ECMASCRIPT_TOOLING_AGENT_HEAPPROFILER_IMPL_H
#define ECMASCRIPT_TOOLING_AGENT_HEAPPROFILER_IMPL_H

#if defined(ECMASCRIPT_SUPPORT_HEAPPROFILER)
#include <uv.h>
#endif

#include "tooling/dynamic/base/pt_params.h"
#include "tooling/dynamic/base/pt_events.h"
#include "tooling/dynamic/base/pt_returns.h"
#include "dispatcher.h"
#include "protocol_handler.h"
#include "protocol_channel.h"

#include "ecmascript/dfx/hprof/progress.h"
#include "ecmascript/dfx/hprof/stream.h"
#include "ecmascript/napi/include/dfx_jsnapi.h"
#include "libpandabase/macros.h"

#include <sys/time.h>

namespace panda::ecmascript::tooling {
class HeapProfilerImpl final {
public:
    explicit HeapProfilerImpl(const EcmaVM *vm, ProtocolChannel *channel)
        : vm_(vm), frontend_(channel), stream_(&frontend_) {}
    ~HeapProfilerImpl();

    DispatchResponse AddInspectedHeapObject(const AddInspectedHeapObjectParams &params);
    DispatchResponse CollectGarbage();
    DispatchResponse Enable();
    DispatchResponse Disable();
    DispatchResponse GetHeapObjectId(const GetHeapObjectIdParams &params, HeapSnapshotObjectId *objectId);
    DispatchResponse GetObjectByHeapObjectId(const GetObjectByHeapObjectIdParams &params,
                                             std::unique_ptr<RemoteObject> *remoteObjectResult);
    DispatchResponse GetSamplingProfile(std::unique_ptr<SamplingHeapProfile> *profile);
    DispatchResponse StartSampling(const StartSamplingParams &params);
    DispatchResponse StartTrackingHeapObjects(const StartTrackingHeapObjectsParams &params);
#if defined(ECMASCRIPT_SUPPORT_HEAPPROFILER)
    static void HeapTrackingCallback(uv_timer_t* handle);
#endif
    DispatchResponse StopSampling(std::unique_ptr<SamplingHeapProfile> *profile);
    DispatchResponse StopTrackingHeapObjects(const StopTrackingHeapObjectsParams &params);
    // The params type of TakeHeapSnapshot is the same as of StopTrackingHeapObjects.
    DispatchResponse TakeHeapSnapshot(const StopTrackingHeapObjectsParams &params);

    class DispatcherImpl final : public DispatcherBase {
    public:
        DispatcherImpl(ProtocolChannel *channel, std::unique_ptr<HeapProfilerImpl> heapprofiler)
            : DispatcherBase(channel), heapprofiler_(std::move(heapprofiler)) {}
        ~DispatcherImpl() override = default;

        std::optional<std::string> Dispatch(const DispatchRequest &request, bool crossLanguageDebug = false) override;
        void AddInspectedHeapObject(const DispatchRequest &request);
        void CollectGarbage(const DispatchRequest &request);
        void Enable(const DispatchRequest &request);
        void Disable(const DispatchRequest &request);
        void GetHeapObjectId(const DispatchRequest &request);
        void GetObjectByHeapObjectId(const DispatchRequest &request);
        void GetSamplingProfile(const DispatchRequest &request);
        void StartSampling(const DispatchRequest &request);
        void StartTrackingHeapObjects(const DispatchRequest &request);
        void StopSampling(const DispatchRequest &request);
        void StopTrackingHeapObjects(const DispatchRequest &request);
        void TakeHeapSnapshot(const DispatchRequest &request);

        enum class Method {
            ADD_INSPECTED_HEAP_OBJECT,
            COLLECT_GARBAGE,
            ENABLE,
            DISABLE,
            GET_HEAP_OBJECT_ID,
            GET_OBJECT_BY_HEAP_OBJECT_ID,
            GET_SAMPLING_PROFILE,
            START_SAMPLING,
            START_TRACKING_HEAP_OBJECTS,
            STOP_SAMPLING,
            STOP_TRACKING_HEAP_OBJECTS,
            TAKE_HEAP_SNAPSHOT,
            UNKNOWN
        };
        Method GetMethodEnum(const std::string& method);

    private:
        NO_COPY_SEMANTIC(DispatcherImpl);
        NO_MOVE_SEMANTIC(DispatcherImpl);

        std::unique_ptr<HeapProfilerImpl> heapprofiler_ {};
    };

private:
    NO_COPY_SEMANTIC(HeapProfilerImpl);
    NO_MOVE_SEMANTIC(HeapProfilerImpl);

    class Frontend {
    public:
        explicit Frontend(ProtocolChannel *channel) : channel_(channel) {}
        ~Frontend() = default;

        void AddHeapSnapshotChunk(char *data, int32_t size);
        void ReportHeapSnapshotProgress(int32_t done, int32_t total);
        void HeapStatsUpdate(HeapStat* updateData, int32_t count);
        void LastSeenObjectId(int32_t lastSeenObjectId, int64_t timeStampUs);
        void ResetProfiles();

    private:
        bool AllowNotify() const;

        ProtocolChannel *channel_ {nullptr};
    };

    class HeapProfilerStream final : public Stream {
    public:
        explicit HeapProfilerStream(Frontend *frontend)
            : frontend_(frontend) {}
        
        void EndOfStream() override {}
        int GetSize() override
        {
            static const int heapProfilerChunkSise = static_cast<int>(100_KB);
            return heapProfilerChunkSise;
        }
        bool WriteChunk(char *data, int32_t size) override
        {
            if (!Good()) {
                return false;
            }
            frontend_->AddHeapSnapshotChunk(data, size);
            return true;
        }
        bool WriteBinBlock(char *data, int32_t size) override
        {
            return WriteChunk(data, size);
        }
        bool Good() override
        {
            return frontend_ != nullptr;
        }

        void UpdateHeapStats(HeapStat* updateData, int32_t count) override
        {
            if (!Good()) {
                return;
            }
            frontend_->HeapStatsUpdate(updateData, count);
        }

        void UpdateLastSeenObjectId(int32_t lastSeenObjectId, int64_t timeStampUs) override
        {
            if (!Good()) {
                return;
            }
            frontend_->LastSeenObjectId(lastSeenObjectId, timeStampUs);
        }

    private:
        NO_COPY_SEMANTIC(HeapProfilerStream);
        NO_MOVE_SEMANTIC(HeapProfilerStream);

        Frontend *frontend_ {nullptr};
    };

    class HeapProfilerProgress final : public Progress {
    public:
        explicit HeapProfilerProgress(Frontend *frontend)
            : frontend_(frontend) {}

        void ReportProgress(int32_t done, int32_t total) override
        {
            frontend_->ReportHeapSnapshotProgress(done, total);
        }

    private:
        NO_COPY_SEMANTIC(HeapProfilerProgress);
        NO_MOVE_SEMANTIC(HeapProfilerProgress);

        Frontend *frontend_ {nullptr};
    };

    void InitializeExtendedProtocolsList();

    const EcmaVM *vm_ {nullptr};
    Frontend frontend_;
    HeapProfilerStream stream_;
    std::vector<std::string> heapProfilerExtendedProtocols_ {};
#if defined(ECMASCRIPT_SUPPORT_HEAPPROFILER)
    uv_timer_t *handle_ {nullptr};
#endif

    friend class HeapProfilerImplFriendTest;
};
}  // namespace panda::ecmascript::tooling
#endif
