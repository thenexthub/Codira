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

#include "samples_record.h"
#include "libarkfile/debug_helpers.h"
#include "libarkfile/class_data_accessor.h"

namespace ark::tooling::sampler {
static constexpr size_t ROOT_NODE_ID = 1;
static constexpr size_t PROGRAM_NODE_ID = 2;
static constexpr size_t IDLE_NODE_ID = 3;
static const uintptr_t ROOT_NODE_PTR = (reinterpret_cast<uintptr_t>(reinterpret_cast<void *>(INT_MAX - ROOT_NODE_ID)));
static const uintptr_t PROGRAM_NODE_PTR =
    (reinterpret_cast<uintptr_t>(reinterpret_cast<void *>(INT_MAX - PROGRAM_NODE_ID)));
static const uintptr_t IDLE_NODE_PTR = (reinterpret_cast<uintptr_t>(reinterpret_cast<void *>(INT_MAX - IDLE_NODE_ID)));
void SamplesRecord::NodeInit(ProfileInfo &profileInfo)
{
    NodeKey nodeKey;
    CpuProfileNode methodNode;

    nodeKey.methodKey.frameId.pandaFilePtr = ROOT_NODE_PTR;
    nodesMap_.emplace(nodeKey, nodesMap_.size() + 1);
    methodNode.parentId = 0;
    methodNode.codeEntry.functionName = "(root)";
    methodNode.id = ROOT_NODE_ID;
    profileInfo.nodes[profileInfo.nodeCount++] = methodNode;

    nodeKey.methodKey.frameId.pandaFilePtr = PROGRAM_NODE_PTR;
    nodesMap_.emplace(nodeKey, nodesMap_.size() + 1);
    methodNode.codeEntry.functionName = "(program)";
    methodNode.id = PROGRAM_NODE_ID;
    profileInfo.nodes[profileInfo.nodeCount++] = methodNode;
    profileInfo.nodes[0].children.push_back(methodNode.id);

    nodeKey.methodKey.frameId.pandaFilePtr = IDLE_NODE_PTR;
    nodesMap_.emplace(nodeKey, nodesMap_.size() + 1);
    methodNode.codeEntry.functionName = "(idle)";
    methodNode.id = IDLE_NODE_ID;
    profileInfo.nodes[profileInfo.nodeCount++] = methodNode;
    profileInfo.nodes[0].children.push_back(methodNode.id);
}

/**
 * Builds a stack information map (stackInfoMap_) by parsing the stack information
 * from the provided sampleInfo. Each stack frame (frameId) is processed to generate
 * a corresponding FrameInfo object, which is then stored in stackInfoMap_.
 * @param sampleInfo A SampleInfo object containing the stack information.
 */
void SamplesRecord::BuildStackInfoMap(const SampleInfo &sampleInfo)
{
    FrameInfo frameInfo;
    // Iterate over the managedStack array in sampleInfo.
    for (size_t index = 0; index < sampleInfo.stackInfo.managedStackSize; ++index) {
        const auto &frameId = sampleInfo.stackInfo.managedStack[index];

        // Check if the frameId already exists in stackInfoMap_. If it does, skip processing.
        if (stackInfoMap_.find(frameId) != stackInfoMap_.end()) {
            continue;
        }
        // If frameId's pandaFilePtr is null, the frame is invalid, so skip processing.
        if (frameId.pandaFilePtr == 0) {
            LOG(DEBUG, PROFILER) << "Profiler collected an invalid pandFilePtr";
            continue;
        }

        auto pfId = reinterpret_cast<panda_file::File *>(frameId.pandaFilePtr);
        panda_file::File::EntityId fileId(frameId.fileId);
        // Use MethodDataAccessor to access method data and retrieve the module name and function name.
        panda_file::MethodDataAccessor mda(*pfId, fileId);

        // Use ClassDataAccessor to get url.
        panda_file::ClassDataAccessor cda(*pfId, mda.GetClassId());
        auto sourceFileId = cda.GetSourceFileId();
        if (!sourceFileId) {
            frameInfo.url = "<unknown>";
        } else {
            frameInfo.url = reinterpret_cast<const char *>(pfId->GetStringData(sourceFileId.value()).data);
        }

        // Check if the URL already exists in scriptIdMap_. If not, assign a new scriptId.
        auto iter = scriptIdMap_.find(frameInfo.url);
        if (iter == scriptIdMap_.end()) {
            auto scriptId = scriptIdMap_.size() + 1;
            scriptIdMap_.emplace(frameInfo.url, scriptId);
            frameInfo.scriptId = static_cast<int>(scriptId);
        } else {
            frameInfo.scriptId = static_cast<int>(iter->second);
        }
        frameInfo.moduleName = mda.GetClassName();
        frameInfo.functionName = mda.GetFullName();
        frameInfo.lineNumber =
            static_cast<int>(panda_file::debug_helpers::GetLineNumber(mda, frameId.bcOffset, pfId)) - 1;
        stackInfoMap_.emplace(frameId, frameInfo);
    }
}

/**
 * Retrieves the FrameInfo associated with the given frameId.
 * @param frameId The unique identifier of the stack frame.
 * @return A pointer to the corresponding FrameInfo, or nullptr if not found.
 *         Caller must check for nullptr before dereferencing.
 */
struct FrameInfo *SamplesRecord::GetFrameInfoByFrameId(const SampleInfo::ManagedStackFrameId &frameId)
{
    auto itor = stackInfoMap_.find(frameId);
    if (itor != stackInfoMap_.end()) {
        return &itor->second;
    }
    return nullptr;
}

/**
 * Processes a single call stack data from the sampleInfo and populates the profileInfo.
 * This function iterates over the managed stack frames in reverse order, builds nodes for each frame,
 * and updates the profileInfo object. It also calculates time deltas between samples and updates timestamps.
 * @param sampleInfo The SampleInfo object containing the stack information and timestamp.
 * @param profileInfo The ProfileInfo object to be updated with processed data.
 * @param prevTimeStamp Reference to the previous timestamp for calculating time deltas.
 */
void SamplesRecord::ProcessSingleCallStackData(const SampleInfo &sampleInfo, ProfileInfo &profileInfo,
                                               uint64_t &prevTimeStamp)
{
    int nodeId = 1;
    // Iterate over the managed stack frames in reverse order.
    for (auto index = static_cast<int>(sampleInfo.stackInfo.managedStackSize - 1); index >= 0; --index) {
        const auto &currentFrame = &sampleInfo.stackInfo.managedStack[index];
        auto frameInfo = GetFrameInfoByFrameId(*currentFrame);
        // Skip processing if FrameInfo is invalid (e.g., null pointer or missing pandaFilePtr).
        if (frameInfo == nullptr) {
            LOG(DEBUG, PROFILER) << "Profiler encountered an invalid pandaFilePtr";
            continue;
        }

        // Generate a unique NodeKey for the current frame.
        NodeKey nodeKey;
        nodeKey.parentId = nodeId;
        nodeKey.methodKey.lineNumber = frameInfo->lineNumber;
        nodeKey.methodKey.frameId = *currentFrame;

        // Check if the node already exists in the nodesMap_.
        if (nodesMap_.find(nodeKey) == nodesMap_.end()) {
            // Create a new CpuProfileNode if it doesn't exist.
            CpuProfileNode methodNode;
            methodNode.id = nodeId = static_cast<int>(nodesMap_.size() + 1);
            nodesMap_.emplace(nodeKey, nodeId);
            methodNode.codeEntry = *frameInfo;
            profileInfo.nodes[profileInfo.nodeCount++] = methodNode;
        } else {
            // If the node already exists, retrieve its ID.
            nodeId = nodesMap_.at(nodeKey);
        }

        // Process the topmost frame (index == 0) to update hit count and add a sample.
        if (index == 0) {
            profileInfo.nodes[nodeId - 1].hitCount++;
            profileInfo.samples.push_back(nodeId);
        }

        // Update the parent node's children list.
        auto &children = profileInfo.nodes[nodeKey.parentId - 1].children;
        if (std::find(children.begin(), children.end(), nodeId) == children.end()) {
            children.push_back(nodeId);
        }
    }

    // Calculate the time delta between the current sample and the previous one.
    int timeDelta;
    if (prevTimeStamp == 0) {
        profileInfo.tid = sampleInfo.threadInfo.threadId;
        profileInfo.startTime = threadStartTime_;
        timeDelta = static_cast<int>(sampleInfo.timeStamp - threadStartTime_);
    } else {
        timeDelta = static_cast<int>(sampleInfo.timeStamp - prevTimeStamp);
    }

    // Update the timeDeltas array and the previous timestamp.
    profileInfo.timeDeltas.push_back(timeDelta);
    prevTimeStamp = sampleInfo.timeStamp;

    // Update the stop time for the profileInfo.
    profileInfo.stopTime = sampleInfo.timeStamp;
}

/**
 * Generates a ProfileInfo object for a single thread by processing a collection of SampleInfo objects.
 * @param sampleInfos A vector of unique pointers to `SampleInfo` objects representing profiling data for a
 * single thread.
 * @return A unique pointer to a `ProfileInfo` object containing the processed profiling data for the thread.
 *         Returns `nullptr` if the input `sampleInfos` vector is empty.
 */
std::unique_ptr<ProfileInfo> SamplesRecord::GetSingleThreadProfileInfo(const SampleInfoVector &sampleInfos)
{
    if (sampleInfos.empty()) {
        return nullptr;
    }
    auto singleThreadProfileInfo = std::make_unique<ProfileInfo>();
    NodeInit(*singleThreadProfileInfo);
    uint64_t prevTimeStamp = 0;
    for (const auto &sampleInfo : sampleInfos) {
        BuildStackInfoMap(*sampleInfo);
        ProcessSingleCallStackData(*sampleInfo, *singleThreadProfileInfo, prevTimeStamp);
    }
    return singleThreadProfileInfo;
}

/**
 * Generates a collection of ProfileInfo objects for all threads profiling data.
 * @return A unique pointer to a vector of unique pointers to `ProfileInfo` objects, containing profiling data
 *         for all threads. Returns `nullptr` if the `tidToSampleInfosMap_` is empty.
 */
std::unique_ptr<std::vector<std::unique_ptr<ProfileInfo>>> SamplesRecord::GetAllThreadsProfileInfos()
{
    if (tidToSampleInfosMap_.empty()) {
        return nullptr;
    }
    auto allThreadsProfileInfos = std::make_unique<std::vector<std::unique_ptr<ProfileInfo>>>();
    for (const auto &pair : tidToSampleInfosMap_) {
        auto profileInfo = GetSingleThreadProfileInfo(pair.second);
        if (profileInfo) {
            allThreadsProfileInfos->emplace_back(std::move(profileInfo));
        }
    }
    return allThreadsProfileInfos;
}
}  // namespace ark::tooling::sampler