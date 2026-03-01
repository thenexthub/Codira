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

#ifndef PANDA_RUNTIME_TOOLING_SAMPLER_SAMPLE_RECORD_H
#define PANDA_RUNTIME_TOOLING_SAMPLER_SAMPLE_RECORD_H

#include <map>
#include <string>
#include <unordered_map>

#include "libarkbase/os/mutex.h"
#include "sample_info.h"

namespace ark::tooling::sampler {
const int MAX_NODE_COUNT = 20000;  // 20000:the maximum size of the array

struct FrameInfo {
    int scriptId = 0;
    int lineNumber = -1;
    int columnNumber = -1;
    std::string functionName;
    std::string moduleName;
    std::string url;
};

struct CpuProfileNode {
    int id = 0;
    int parentId = 0;
    int hitCount = 0;
    FrameInfo codeEntry;
    std::vector<int> children;
};

struct ProfileInfo {
    uint64_t tid = 0;
    uint64_t startTime = 0;
    uint64_t stopTime = 0;
    std::array<CpuProfileNode, MAX_NODE_COUNT> nodes;
    int nodeCount = 0;
    std::vector<int> samples;
    std::vector<int> timeDeltas;
    // state time statistic
    uint64_t gcTime = 0;
    uint64_t cInterpreterTime = 0;
    uint64_t asmInterpreterTime = 0;
    uint64_t aotTime = 0;
    uint64_t builtinTime = 0;
    uint64_t napiTime = 0;
    uint64_t arkuiEngineTime = 0;
    uint64_t runtimeTime = 0;
    uint64_t otherTime = 0;
};

struct MethodKey {
    // NOLINTBEGIN(misc-non-private-member-variables-in-classes,-warnings-as-errors)
    SampleInfo::ManagedStackFrameId frameId;
    int lineNumber = -1;
    // NOLINTEND(misc-non-private-member-variables-in-classes,-warnings-as-errors)
    bool operator<(const MethodKey &methodKey) const
    {
        return std::tie(lineNumber, frameId) < std::tie(methodKey.lineNumber, methodKey.frameId);
    }
};

struct NodeKey {
    // NOLINTBEGIN(misc-non-private-member-variables-in-classes,-warnings-as-errors)
    MethodKey methodKey;
    int parentId = 0;
    // NOLINTEND(misc-non-private-member-variables-in-classes,-warnings-as-errors)
    bool operator<(const NodeKey &nodeKey) const
    {
        return std::tie(parentId, methodKey) < std::tie(nodeKey.parentId, nodeKey.methodKey);
    }
};

class SamplesRecord {
public:
    SamplesRecord() = default;
    PANDA_PUBLIC_API std::unique_ptr<std::vector<std::unique_ptr<ProfileInfo>>> GetAllThreadsProfileInfos();
    void SetThreadStartTime(uint64_t threadStartTime)
    {
        threadStartTime_ = threadStartTime;
    };
    void AddSampleInfo(uint32_t threadId, std::unique_ptr<SampleInfo> sampleInfo)
    {
        os::memory::LockHolder holder(addSamplInfoLock_);
        tidToSampleInfosMap_[threadId].emplace_back(std::move(sampleInfo));
    };

private:
    void NodeInit(ProfileInfo &profileInfo);
    using SampleInfoVector = std::vector<std::unique_ptr<SampleInfo>>;
    std::unique_ptr<ProfileInfo> GetSingleThreadProfileInfo(const SampleInfoVector &sampleInfos);
    void BuildStackInfoMap(const SampleInfo &sampleInfo);
    FrameInfo *GetFrameInfoByFrameId(const SampleInfo::ManagedStackFrameId &frameId);
    void ProcessSingleCallStackData(const SampleInfo &sampleInfo, ProfileInfo &profileInfo, uint64_t &prevTimeStamp);

    uint64_t threadStartTime_ = 0;
    os::memory::Mutex addSamplInfoLock_;
    std::map<NodeKey, int> nodesMap_ = {};
    std::map<std::string, size_t> scriptIdMap_ = {};
    std::map<SampleInfo::ManagedStackFrameId, FrameInfo> stackInfoMap_;
    std::unordered_map<uint32_t, SampleInfoVector> tidToSampleInfosMap_ = {};
};
}  // namespace ark::tooling::sampler

#endif  // PANDA_RUNTIME_TOOLING_SAMPLER_SAMPLE_RECORD_H
