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


#include "SamplesRecord.h"
#include <algorithm>
#include "ObjectModel/MFuncdesc.inline.h"
#include "UnwindStack/MangleNameHelper.h"
#include "Base/TimeUtils.h"

namespace MapleRuntime {
constexpr int TIME_USEC_PER_SEC = 1000 * 1000;
constexpr int TIME_NSEC_PER_USEC = 1000;
uint64_t SamplesRecord::GetMicrosecondsTimeStamp()
{
    struct timespec time;
    clock_gettime(CLOCK_MONOTONIC, &time);
    return time.tv_sec * TIME_USEC_PER_SEC + time.tv_nsec / TIME_NSEC_PER_USEC;
}

SamplesRecord::~SamplesRecord()
{
    ReleaseProfileInfo();
}

void SamplesRecord::InitProfileInfo()
{
    profileInfo = new ProfileInfo();
    CHECK_DETAIL(profileInfo != nullptr, "new profileInfo fail");
    NodeInit();
}

void SamplesRecord::ReleaseProfileInfo()
{
    if (profileInfo != nullptr) {
        delete profileInfo;
        profileInfo = nullptr;
    }
}

void SamplesRecord::NodeInit()
{
    CpuProfileNode methodNode;

    /*
     * Root node is the start node of the caller tree.
     * The caller tree is usually as follows:
     *                root
     *             /   |   \
     *         program idle funcA
     *                      / |  \
     *                 funcB  ... ...
     */
    methodNode.id = ROOT_NODE_ID;
    methodNode.parentId = UNKNOWN_NODE_ID;
    methodNode.codeEntry.functionName = "(root)";
    profileInfo->nodes[profileInfo->nodeCount++] = methodNode;
    profileInfo->nodeSet.insert(methodNode);

    /*
     * When the sampling interval is greater than the threshold, the sampling result is invalid.
     * Then the method name will be set to (program).
     */
    methodNode.id = PROGRAM_NODE_ID;
    methodNode.parentId = ROOT_NODE_ID;
    methodNode.codeEntry.functionName = "(program)";
    profileInfo->nodes[profileInfo->nodeCount++] = methodNode;
    profileInfo->nodes[ROOT_NODE_ID - 1].children.emplace_back(methodNode.id);
    profileInfo->nodeSet.insert(methodNode);

    /*
     * When a sample is empty, the number of hit idle nodes increases.
     */
    methodNode.id = IDLE_NODE_ID;
    methodNode.parentId = ROOT_NODE_ID;
    methodNode.codeEntry.functionName = "(idle)";
    profileInfo->nodes[profileInfo->nodeCount++] = methodNode;
    profileInfo->nodes[ROOT_NODE_ID - 1].children.emplace_back(methodNode.id);
    profileInfo->nodeSet.insert(methodNode);
}

uint64_t SamplesRecord::UpdateScriptIdMap(CString& url)
{
    auto iter = scriptIdMap.find(url);
    if (iter == scriptIdMap.end()) {
        scriptIdMap.emplace(url, scriptIdMap.size() + 1);
        return static_cast<uint64_t>(scriptIdMap.size());
    } else {
        return iter->second;
    }
}

bool SamplesRecord::UpdateNodeSet(ProfileInfo* info, CpuProfileNode& methodNode)
{
    auto& nodSet = info->nodeSet;
    auto result = nodSet.find(methodNode);
    if (result == nodSet.end()) {
        info->previousId = methodNode.id = nodSet.size() + 1;
        nodSet.insert(methodNode);
        return true;
    } else {
        info->previousId = methodNode.id = result->id;
        return false;
    }
}

void SamplesRecord::AddNodes(ProfileInfo* info, CpuProfileNode& methodNode)
{
    info->nodes[info->nodeCount++] = methodNode;
    if (methodNode.parentId > 0 && methodNode.parentId <= info->nodeCount) {
        info->nodes[methodNode.parentId - 1].children.emplace_back(methodNode.id);
    }
}

int SamplesRecord::GetSampleNodeId(uint64_t previousId, uint64_t topFrameNodeId)
{
    return previousId == 0 ? ROOT_NODE_ID : topFrameNodeId;
}

uint64_t SamplesRecord::GetPreviousTimeStamp(uint64_t previousTimeStamp, uint64_t startTime)
{
    return previousTimeStamp == 0 ? startTime : previousTimeStamp;
}

void SamplesRecord::DeleteAbnormalSample(ProfileInfo* info, int timeDelta)
{
    if (timeDelta > static_cast<int>(timeDeltaThreshold)) {
        uint32_t size = info->samples.size();
        if (size > 0) {
            info->samples[size - 1] = PROGRAM_NODE_ID;
        }
        info->previousState = FrameType::UNKNOWN;
    }
}

void SamplesRecord::SetPreviousState(ProfileInfo* info, FrameType frameType)
{
    info->previousState = frameType;
}

void SamplesRecord::IncreaseNodeHitCount(ProfileInfo* info, int sampleNodeId)
{
    info->nodes[sampleNodeId - 1].hitCount++;
}

void SamplesRecord::AddSampleNodeId(ProfileInfo* info, int sampleNodeId)
{
    info->samples.emplace_back(sampleNodeId);
}

void SamplesRecord::AddTimeDelta(ProfileInfo* info, int timeDelta)
{
    info->timeDeltas.emplace_back(timeDelta);
}

void SamplesRecord::SetPreviousTimeStamp(ProfileInfo* info, uint64_t timeStamp)
{
    info->previousTimeStamp = timeStamp;
}

ProfileInfo* SamplesRecord::GetProfileInfo(uint64_t mutatorId)
{
    if (profileInfo->mutatorId != mutatorId) {
        profileInfo->mutatorId = mutatorId;
    }
    return profileInfo;
}

void SamplesRecord::AddSample(SampleTask& task)
{
    ProfileInfo* info = GetProfileInfo(task.mutatorId);
    if (info->nodeCount >= MAX_NODE_COUNT) {
        LOG(RTLOG_WARNING, "CpuProfileNode counts over 20000.");
        return;
    }
    // Construct the sampled data into an array of codeInfo.
    std::vector<CodeInfo> codeInfos = GetCodeInfos(task);
    int frameSize = codeInfos.size();
    CpuProfileNode methodNode;
    methodNode.id = 1;
    for (; frameSize >= 1; frameSize--) {
        methodNode.codeEntry = codeInfos[frameSize - 1];
        methodNode.parentId =  methodNode.id;
        if (UpdateNodeSet(info, methodNode)) {
            AddNodes(info, methodNode);
        }
    }
    int sampleNodeId = GetSampleNodeId(info->previousId, methodNode.id);
    uint64_t previousTimeStamp = GetPreviousTimeStamp(info->previousTimeStamp, info->startTime);
    int timeDelta = static_cast<int>(task.timeStamp - previousTimeStamp);

    DeleteAbnormalSample(info, timeDelta);

    StatisticStateTime(info, timeDelta);

    SetPreviousState(info, codeInfos.front().frameType);

    IncreaseNodeHitCount(info, sampleNodeId);

    AddSampleNodeId(info, sampleNodeId);

    AddTimeDelta(info, timeDelta);

    SetPreviousTimeStamp(info, task.timeStamp);
}

void SamplesRecord::AddEmptySample(SampleTask& task)
{
    ProfileInfo* info = GetProfileInfo(task.mutatorId);
    uint64_t previousTimeStamp = GetPreviousTimeStamp(info->previousTimeStamp, info->startTime);
    int timeDelta = static_cast<int>(task.timeStamp - previousTimeStamp);

    DeleteAbnormalSample(info, timeDelta);

    StatisticStateTime(info, timeDelta);

    SetPreviousState(info, FrameType::UNKNOWN);

    IncreaseNodeHitCount(info, ROOT_NODE_ID);

    AddSampleNodeId(info, IDLE_NODE_ID);

    AddTimeDelta(info, timeDelta);

    SetPreviousTimeStamp(info, task.timeStamp);
}

void SamplesRecord::StatisticStateTime(ProfileInfo* info, int timeDelta)
{
    FrameType state = info->previousState;
    switch (state) {
        case FrameType::MANAGED: {
            info->managedTime += static_cast<uint64_t>(timeDelta);
            break;
        }
        default: {
            info->unknownTime += static_cast<uint64_t>(timeDelta);
        }
    }
}

void SamplesRecord::StringifySampleData(ProfileInfo* info)
{
    sampleData = "";
    sampleData.Append(CString::FormatString("{\"mutatorId\":%d,", info->mutatorId));
    sampleData.Append(CString::FormatString("\"startTime\":%llu,", info->startTime));
    sampleData.Append(CString::FormatString("\"endTime\":%llu,", info->stopTime));

    StringifyStateTimeStatistic(info);
    StringifyNodes(info);
    StringifySamples(info);
}

void SamplesRecord::StringifyStateTimeStatistic(ProfileInfo* info)
{
    sampleData.Append(CString::FormatString("\"managedTime\":%llu,", info->managedTime));
    sampleData.Append(CString::FormatString("\"unknownTime\":%llu,", info->unknownTime));
}

void SamplesRecord::StringifyNodes(ProfileInfo* info)
{
    sampleData.Append("\"nodes\":[");
    size_t nodeCount = static_cast<size_t>(info->nodeCount);

    for (size_t i = 0; i < nodeCount; i++) {
        struct CpuProfileNode& node = info->nodes[i];
        struct CodeInfo& codeEntry = node.codeEntry;
        sampleData.Append(CString::FormatString("{\"id\":%d,", node.id));
        sampleData.Append("\"callFrame\":{\"functionName\":\"");
        sampleData.Append(codeEntry.functionName);
        sampleData.Append(CString::FormatString("\",\"scriptId\":\"%d\",", codeEntry.scriptId));
        sampleData.Append("\"url\":\"");
        sampleData.Append(codeEntry.url);
        sampleData.Append(CString::FormatString("\",\"lineNumber\":%d},", codeEntry.lineNumber));
        sampleData.Append(CString::FormatString("\"hitCount\":%d,\"children\":[", node.hitCount));

        std::vector<uint64_t> children = node.children;
        size_t childrenCount = children.size();
        for (size_t j = 0; j < childrenCount; j++) {
            sampleData.Append(CString::FormatString("%d,", children[j]));
        }
        if (childrenCount > 0) {
            sampleData = sampleData.SubStr(0, sampleData.Length() - 1);
        }
        sampleData += "]},";
    }
    sampleData = sampleData.SubStr(0, sampleData.Length() - 1);
    sampleData += "],";
}

void SamplesRecord::StringifySamples(ProfileInfo* info)
{
    std::vector<int>& samples = info->samples;
    std::vector<int>& timeDeltas = info->timeDeltas;

    size_t samplesCount = samples.size();
    if (samplesCount == 0) {
        sampleData.Append("\"samples\":[],\"timeDeltas\":[]}");
        return;
    }
    CString samplesIdStr = "";
    CString timeDeltasStr = "";
    for (size_t i = 0; i < samplesCount; i++) {
        samplesIdStr.Append(CString::FormatString("%d,", samples[i]));
        timeDeltasStr.Append(CString::FormatString("%d,", timeDeltas[i]));
    }

    samplesIdStr = samplesIdStr.SubStr(0, samplesIdStr.Length() - 1);
    timeDeltasStr = timeDeltasStr.SubStr(0, timeDeltasStr.Length() - 1);

    sampleData.Append("\"samples\":[");
    sampleData.Append(samplesIdStr);
    sampleData.Append("],\"timeDeltas\":[");
    sampleData.Append(timeDeltasStr);
    sampleData.Append("]}");
}

void SamplesRecord::DumpProfileInfo()
{
    StringifySampleData(profileInfo);
    WriteFile();
}

bool SamplesRecord::OpenFile(int fd)
{
    if (fd == -1) {
        return false;
    }

    if (ftruncate(fd, 0) == -1) {
        return false;
    }
    fileDesc = fd;
    return true;
}

void SamplesRecord::WriteFile()
{
    int err = write(fileDesc, sampleData.Str(), sampleData.Length());
    if (err == -1) {
        LOG(RTLOG_ERROR, "Write file failed. msg: %s", strerror(errno));
    }
}

void SamplesRecord::RunTaskLoop()
{
    while (!taskQueue.empty()) {
        auto task = taskQueue.front();
        taskQueue.pop_front();
        if (task.frameCnt == 0) {
            AddEmptySample(task);
        } else {
            AddSample(task);
        }
    }
}

void SamplesRecord::DoSingleTask(uint64_t previousTimeStemp)
{
    if (IsTimeout(previousTimeStemp)) {
        return;
    }
    if (taskQueue.empty()) {
        return;
    }
    auto task = taskQueue.front();
    if (!task.finishParsed) {
        return;
    }
    taskQueue.pop_front();
    if (task.frameCnt == 0) {
        AddEmptySample(task);
    } else {
        AddSample(task);
    }
}

void SamplesRecord::ParseSampleData(uint64_t previousTimeStemp)
{
    if (taskQueue.empty()) {
        return;
    }
    for (auto& task : taskQueue) {
        if (task.finishParsed) {
            continue;
        }
        for (int i = task.checkPoint; i < task.frameCnt; ++i) {
            GetDemangleName(task.funcDescRefs[i]);
            GetUrl(task.funcDescRefs[i]);
            // Avoid taking too long time to parse symbol.
            if (IsTimeout(previousTimeStemp)) {
                task.checkPoint = i + 1;
                return;
            }
        }
        task.finishParsed = true;
    }
}

bool SamplesRecord::IsTimeout(uint64_t previousTimeStemp)
{
    uint64_t currentTimeStamp = SamplesRecord::GetMicrosecondsTimeStamp();
    int64_t ts = static_cast<int64_t>(interval) -
        static_cast<int64_t>(currentTimeStamp - previousTimeStemp);
    if (ts < 0) {
        return true;
    }
    return false;
}

void SamplesRecord::Post(uint64_t mutatorId, std::vector<uint64_t>& FuncDescRefs,
                         std::vector<FrameType>& FrameTypes, std::vector<uint32_t>& LineNumbers)
{
    uint64_t timeStamp = SamplesRecord::GetMicrosecondsTimeStamp();
    SampleTask task(timeStamp, mutatorId, FuncDescRefs, FrameTypes, LineNumbers);
    taskQueue.push_back(task);
}

std::vector<CodeInfo> SamplesRecord::GetCodeInfos(SampleTask& task)
{
    std::vector<CodeInfo> codeInfos;
    for (int i = 0; i < task.frameCnt; ++i) {
        CodeInfo codeInfo;
        codeInfo.lineNumber = task.lineNumbers[i];
        codeInfo.frameType = task.frameTypes[i];
        codeInfo.funcIdentifier = task.funcDescRefs[i];
        codeInfo.functionName = GetDemangleName(codeInfo.funcIdentifier);
        codeInfo.url = GetUrl(codeInfo.funcIdentifier);
        codeInfo.scriptId = UpdateScriptIdMap(codeInfo.url);
        codeInfos.emplace_back(codeInfo);
    }
    return codeInfos;
}

CString SamplesRecord::GetUrl(uint64_t funcIdentifier)
{
    if (identifierUrlMap.find(funcIdentifier) != identifierUrlMap.end()) {
        return identifierUrlMap[funcIdentifier];
    }
    return ParseUrl(funcIdentifier);
}

CString SamplesRecord::ParseUrl(uint64_t funcIdentifier)
{
    FuncDescRef funcDescRef = reinterpret_cast<FuncDescRef>(funcIdentifier);
    CString path = funcDescRef->GetFuncDir();
    CString fileName = funcDescRef->GetFuncFilename();
#ifdef _WIN64
    CString slash = "\\";
#else
    CString slash = "/";
#endif
    CString url =  path.IsEmpty() ? fileName : path + slash + fileName;
    identifierUrlMap.emplace(funcIdentifier, url);
    return fileName;
}

CString SamplesRecord::GetDemangleName(uint64_t funcIdentifier)
{
    if (identifierFuncnameMap.find(funcIdentifier) != identifierFuncnameMap.end()) {
        return identifierFuncnameMap[funcIdentifier];
    }
    return ParseDemangleName(funcIdentifier);
}

CString SamplesRecord::ParseDemangleName(uint64_t funcIdentifier)
{
    FuncDescRef funcDescRef = reinterpret_cast<FuncDescRef>(funcIdentifier);
    MangleNameHelper mangleNameHelper(funcDescRef->GetFuncName(),
        StackTraceFormatFlag(funcDescRef->GetStackTraceFormat()));
    mangleNameHelper.Demangle();
    CString funcName = mangleNameHelper.GetDemangleName();
    identifierFuncnameMap.emplace(funcIdentifier, funcName);
    return funcName;
}
}
