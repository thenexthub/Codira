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

#ifndef ECMASCRIPT_TOOLING_CLIENT_DOMAIN_HEAPPROFILER_CLIENT_H
#define ECMASCRIPT_TOOLING_CLIENT_DOMAIN_HEAPPROFILER_CLIENT_H

#include <iostream>
#include <fstream>
#include <map>

#include "tooling/dynamic/base/pt_json.h"

using PtJson = panda::ecmascript::tooling::PtJson;

namespace OHOS::ArkCompiler::Toolchain {
enum HeapProfilerEvent {
    DAFAULT_VALUE = 0,
    ALLOCATION = 1,
    ALLOCATION_STOP,
    HEAPDUMP,
    ENABLE,
    DISABLE,
    SAMPLING,
    SAMPLING_STOP,
    COLLECT_GARBAGE
};
class HeapProfilerClient final {
public:
    HeapProfilerClient(uint32_t sessionId) : sessionId_(sessionId) {}
    ~HeapProfilerClient() = default;

    bool DispatcherCmd(const std::string &cmd, const std::string &arg);
    int HeapDumpCommand();
    int AllocationTrackCommand();
    int AllocationTrackStopCommand();
    int Enable();
    int Disable();
    int Samping();
    int SampingStop();
    int CollectGarbage();
    void RecvReply(std::unique_ptr<PtJson> json);
    void SaveHeapSnapshotAndAllocationTrackData(const std::string &chunk);
    void SaveHeapsamplingData(std::unique_ptr<PtJson> result);
    bool WriteHeapProfilerForFile(const std::string &fileName, const std::string &data);

private:
    std::string fileName_;
    std::map<uint32_t, HeapProfilerEvent> idEventMap_;
    std::string path_;
    bool isAllocationMsg_ {false};
    uint32_t sessionId_;
};
} // OHOS::ArkCompiler::Toolchain
#endif