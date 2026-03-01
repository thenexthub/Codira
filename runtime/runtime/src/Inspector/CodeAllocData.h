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


#ifndef MRT_CODE_ALLOC_DATA_H
#define MRT_CODE_ALLOC_DATA_H
#include "UnwindStack/GcStackInfo.h"
#include "CodeHeapData.h"
#include "HeapSnapshotJsonSerializer.h"
namespace MapleRuntime {
struct TraceFunctionInfo {
    CString functionName;
    CString scriptName;
    CString url = "";
    int32_t line;
    int32_t column = -1;
};

struct TraceNodeField {
    int32_t id = 0; // Unique ID
    int32_t functionInfoIndex;
    int32_t selfSize;
    I8 type = 0;
    std::vector<TraceNodeField*> children;
};

struct Sample {
    int32_t size;
    int32_t nodeId;
    int32_t orinal;
};

class CodeAllocData {
public:
    static CodeAllocData* GetCodeAllocData();
    static void SetCodeAllocData();
    TraceNodeField* FindNode(const FrameAddress*, const char*);
    int32_t FindKey(const FrameAddress*, const char*);
    void DeleteCodeAllocData();
    bool IsRecording() { return recording.load();};
    void SetRecording(bool isRecording) { recording.store(isRecording, std::memory_order_release);};
    void DeleteAllNode(TraceNodeField* node);
    void SerializeCodeAllocData();
    void SerializeSamples();
    void SerializeCallFrames();
    void SerializeStats();
    void SerializeFunctionInfo(int32_t idx);
    void SerializeEachFrame(TraceNodeField* node);
    void InitAllocParam();
    void InitRoot();
    void RecordAllocNodes(const TypeInfo* ti, MSize size);
    int32_t SetNodeID() { return ++traceNodeID;};
    friend class AllocStackInfo;
private:
    std::unordered_map<int32_t, TraceNodeField*> traceNodeMap;
    TraceNodeField* traceNodeHead; // ROOT node
    std::vector<Sample*> samples;
    std::vector<TraceFunctionInfo*> traceFunctionInfo;
    int32_t sampSize;
    int32_t allocSize;
    int32_t traceNodeID = 0;
    StreamWriter* writer = nullptr;
    std::atomic<bool> recording{false};
    std::mutex sharedMtx;
};

class AllocStackInfo : public GCStackInfo {
public:
    int32_t ProcessTraceInfo(FrameInfo &frame);
    void ProcessTraceNode(TraceNodeField* head, const TypeInfo* ti, MSize allocSize);
    void ProcessStackTrace(const TypeInfo* ti, MSize size);
private:
    std::stack<FrameInfo* > frames;
};
 
} // namespace MapleRuntime
#endif
