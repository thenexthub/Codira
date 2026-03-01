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


#include "CodeAllocData.h"
#include <iostream>
#include <chrono>
#include <Common/ScopedObjectAccess.h>
#include <chrono>
#include "Common/StackType.h"
#include "UnwindStack/PrintStackInfo.h"
#include "UnwindStack/StackInfo.h"
#include "UnwindStack/StackMetadataHelper.h"
#include "FileStream.h"

namespace MapleRuntime {
static CodeAllocData* g_allocInfo;
CodeAllocData* CodeAllocData::GetCodeAllocData()
{
    if (g_allocInfo == nullptr) {
        g_allocInfo = new CodeAllocData();
    }
    return g_allocInfo;
}

void CodeAllocData::SetCodeAllocData()
{
    g_allocInfo -> InitAllocParam();
}

TraceNodeField* CodeAllocData::FindNode(const FrameAddress* ptr, const char* str)
{
    if (str == nullptr) {
        return nullptr;
    };
    size_t key = FindKey(ptr, str);
    auto iter = traceNodeMap.find(key);
    if (iter != traceNodeMap.end()) {
        return iter->second;
    } else {
        return nullptr;
    }
}

int32_t CodeAllocData::FindKey(const FrameAddress* ptr, const char* str)
{
    // The identifier of a stack frame depends on the FA of the frame and the name of the frame.
    std::size_t hash1 = std::hash<const FrameAddress* >{}(ptr);
    std::size_t hash2 = std::hash<std::string>{}(str);
    return hash1 ^ hash2;
}

void CodeAllocData::DeleteCodeAllocData()
{
    std::unique_lock<std::mutex> lock(sharedMtx);
    SetRecording(false);
    HeapProfilerStream* stream = &MapleRuntime::HeapProfilerStream::GetInstance();
    stream->SetContext(ALLOCATION);
    g_allocInfo->SerializeCodeAllocData();
    for (auto sample:g_allocInfo->samples) {
        delete sample;
        sample = nullptr;
    }
    samples.clear();
    for (auto traceInfo:g_allocInfo->traceFunctionInfo) {
        delete traceInfo;
        traceInfo = nullptr;
    }
    traceFunctionInfo.clear();
    traceNodeMap.clear();
    g_allocInfo->DeleteAllNode(g_allocInfo->traceNodeHead);
    delete g_allocInfo->writer;
}

void CodeAllocData::DeleteAllNode(TraceNodeField* node)
{
    for (size_t i = 0; i < node->children.size(); i++) {
        DeleteAllNode(node->children[i]);
    }
    delete node;
}

void CodeAllocData::SerializeCodeAllocData()
{
    writer->WriteString("{\\\"head\\\":");
    // 1. dump callFrame
    SerializeCallFrames();
    writer->WriteString(",");
    // 2. dump samples
    SerializeSamples();
    writer->WriteChar('}');
    writer->End();
}

void CodeAllocData::SerializeCallFrames()
{
    SerializeEachFrame(traceNodeHead);
}

void CodeAllocData::SerializeFunctionInfo(int32_t idx)
{
    writer->WriteString("{\\\"functionName\\\":\\\"");
    writer->WriteString(traceFunctionInfo[idx]->functionName);
    writer->WriteString("\\\",\\\"scriptName\\\":\\\"");
    writer->WriteString(traceFunctionInfo[idx]->scriptName);
    writer->WriteString("\\\",\\\"lineNumber\\\":");
    writer->WriteNumber(traceFunctionInfo[idx]->line);
    writer->WriteString(",\\\"columnNumber\\\":");
    writer->WriteNumber(traceFunctionInfo[idx]->column);
    writer->WriteChar('}');
}

void CodeAllocData::SerializeEachFrame(TraceNodeField* node)
{
    writer->WriteString("{\\\"callFrame\\\":");
    SerializeFunctionInfo(node->functionInfoIndex);
    writer->WriteChar(',');
    writer->WriteString("\\\"selfSize\\\":");
    writer->WriteNumber(node->selfSize);
    writer->WriteString(",\\\"id\\\":");
    writer->WriteNumber(node->id);
    writer->WriteString(",\\\"nodeType\\\":");
    writer->WriteNumber(node->type);

    writer->WriteString(",\\\"children\\\":[");
    if (node->children.size() != 0) {
        for (size_t i = 0; i < node->children.size(); i++) {
            SerializeEachFrame(node->children[i]);
            if (i < node->children.size() - 1) {
                writer->WriteChar(',');
            }
        }
    }
    writer->WriteChar(']');
    writer->WriteChar('}');
}

void CodeAllocData::SerializeSamples()
{
    writer->WriteString("\\\"samples\\\":[");
    // for each sample
    for (size_t i = 0; i < samples.size(); i++) {
        if (i != 0) {
            writer->WriteChar(',');
        }
        writer->WriteString("{\\\"size\\\":");
        writer->WriteNumber(samples[i]->size);
        writer->WriteChar(',');
        writer->WriteString("\\\"nodeId\\\":");
        writer->WriteNumber(samples[i]->nodeId);
        writer->WriteChar(',');
        writer->WriteString("\\\"ordinal\\\":");
        writer->WriteNumber(samples[i]->orinal);
        writer->WriteChar('}');
    }
    writer->WriteChar(']');
}
void CodeAllocData::InitRoot() {
    TraceNodeField* traceNode = new TraceNodeField();
    if (traceNode == nullptr) {
        LOG(RTLOG_ERROR, "init traceNode failed");
        return;
    }
    traceNode->id = 0;
    traceNode->functionInfoIndex = 0;
    traceNode->selfSize = 0;
    traceNodeHead = traceNode;
    TraceFunctionInfo* traceFunction = new TraceFunctionInfo();
    if (traceFunction == nullptr) {
        LOG(RTLOG_ERROR, "init traceFunction failed");
        return;
    }
    traceFunction->functionName = "{root}";
    traceFunction->scriptName = "";
    traceFunction->line = -1;
    CodeAllocData::GetCodeAllocData()->traceFunctionInfo.push_back(traceFunction);
}

void CodeAllocData::InitAllocParam() {
    sampSize = 1 * 1024 ; // default 1 * 1024 b
    InitRoot();
    HeapProfilerStream* stream = &MapleRuntime::HeapProfilerStream::GetInstance();
    writer = new StreamWriter(stream);
}

void CodeAllocData::SerializeStats()
{
    HeapProfilerStream* stream = &MapleRuntime::HeapProfilerStream::GetInstance();
    stream->SetContext(STATSUPDATE);
    writer->WriteString("[");
    writer->WriteNumber(traceNodeID);
    writer->WriteChar(',');
    writer->WriteNumber(0);
    writer->WriteChar(',');
    ssize_t allocatedSize = MapleRuntime::Heap::GetHeap().GetAllocatedSize();
    writer->WriteNumber(allocatedSize);
    writer->WriteString("]");
    writer->End();
}

void CodeAllocData::RecordAllocNodes(const TypeInfo* ti, MSize size)
{
    std::unique_lock<std::mutex> lock(sharedMtx);
    if (!IsRecording()) { // avoid delete func was called at this time
        return;
    }
    allocSize += size;
    if (allocSize < sampSize) {
        return;
    } else {
        AllocStackInfo* allocStackInfo  = new AllocStackInfo();
        allocStackInfo->ProcessStackTrace(ti, size);
        delete allocStackInfo;
        allocSize = 0;
        SerializeStats();
    }
}

int32_t AllocStackInfo::ProcessTraceInfo(FrameInfo &frame)
{
    if (frame.GetFrameType() == FrameType::NATIVE) {
        return -1;
    }
    TraceFunctionInfo* traceFunction = new TraceFunctionInfo();
    if (frame.GetFrameType() == FrameType::MANAGED) {
        StackMetadataHelper stackMetadataHelper(frame);
        traceFunction->scriptName = frame.GetFileName();
        traceFunction->scriptName.ReplaceAll("/", "\\");
        traceFunction->functionName = frame.GetFuncName();
        traceFunction->line = stackMetadataHelper.GetLineNumber();
    } else {
        delete traceFunction;
        return -1;
    }

    CodeAllocData::GetCodeAllocData()->traceFunctionInfo.push_back(traceFunction);
    int32_t functionInfoIndex = CodeAllocData::GetCodeAllocData()->traceFunctionInfo.size() - 1;
    return functionInfoIndex;
}

void AllocStackInfo::ProcessTraceNode(TraceNodeField* head, const TypeInfo* ti, MSize allocSize)
{
    // Initialize nodes and fill nodes in head in sequence.
    if (frames.empty()) {
        // This node is the root node.
        head->selfSize += allocSize;
        return;
    }
    while (!frames.empty()) {
        FrameInfo* f = frames.top();
        frames.pop();
        TraceNodeField* traceNode = new TraceNodeField();
        traceNode->id = CodeAllocData::GetCodeAllocData()->SetNodeID();
        traceNode->functionInfoIndex = ProcessTraceInfo(*f);
        traceNode->type = ti->GetType();
        if (traceNode->functionInfoIndex == -1) {
            delete traceNode;
            continue;
        }
        traceNode->selfSize = 0;
        // Add the children node of the upper-level node.
        head->children.push_back(traceNode);
        // In this case, head becomes a child node. Used to insert a child node next time.
        head = traceNode;
        // Add to the map for next search.
        FrameAddress* FA = f->mFrame.GetFA();
        size_t key = CodeAllocData::GetCodeAllocData()->FindKey(FA, f->GetFuncName().Str());
        CodeAllocData::GetCodeAllocData()->traceNodeMap.insert(std::pair<size_t, TraceNodeField* >(key, traceNode));
        delete f;
        f = nullptr;
    }
    head->selfSize += allocSize;
}
void AllocStackInfo::ProcessStackTrace(const TypeInfo* ti, MSize size)
{
    UnwindContext uwContext;
    // Top unwind context can only be runtime or Codira context.
    CheckTopUnwindContextAndInit(uwContext);
    while (!uwContext.frameInfo.mFrame.IsAnchorFrame(anchorFA)) {
        AnalyseAndSetFrameType(uwContext);
        FrameInfo* f = new FrameInfo(uwContext.frameInfo);
        // 1. If the node has been recorded, add the content in the stack to the end of the node.
        FrameAddress* FA = f->mFrame.GetFA();
        TraceNodeField* node = CodeAllocData::GetCodeAllocData()->FindNode(FA, f->GetFuncName().Str());
        if (node != nullptr) {
            delete f;
            ProcessTraceNode(node, ti, size);
            return;
        }
        frames.push(f);
        UnwindContext caller;
        lastFrameType = uwContext.frameInfo.GetFrameType();
#ifndef _WIN64
        if (uwContext.UnwindToCallerContext(caller) == false) {
#else
        if (uwContext.UnwindToCallerContext(caller, uwCtxStatus) == false) {
#endif
            return;
        }
        uwContext = caller;
    }
    // 2. If the stack back is not recorded, the call chain is a new call chain and the root node is the root node.
    ProcessTraceNode(CodeAllocData::GetCodeAllocData()->traceNodeHead, ti, size);

    // 3. Record the samples at this time.
    Sample* sample = new Sample();
    sample->size = size;
    sample->nodeId = CodeAllocData::GetCodeAllocData()->traceFunctionInfo.size();
    auto now = std::chrono::system_clock::now();
    auto timestamp = std::chrono::duration_cast<std::chrono::seconds>(now.time_since_epoch()).count();
    sample->orinal = static_cast<int32_t>(timestamp);
    CodeAllocData::GetCodeAllocData()->samples.push_back(sample);
}
}
