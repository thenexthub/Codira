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


#include <iostream>
#include <unistd.h>
#include <cstring>
#include <sstream>
#include <pthread.h>
#include "Mutator/MutatorManager.h"
#include "UnwindStack/StackMetadataHelper.h"
#include "ObjectModel/MArray.inline.h"
#include "FileStream.h"
#include "CodeHeapData.h"
#include "HeapSnapshotJsonSerializer.h"

namespace MapleRuntime {

bool CodeHeapDataForIDE::Serialize()
{
    ScopedStopTheWorld scopedStopTheWorld("serialize heap data");
    ProcessHeap();
    HeapProfilerStream* stream = &MapleRuntime::HeapProfilerStream::GetInstance();
    stream->SetContext(DUMPHEAPSNAPSHOT);
    writer = new StreamWriter(stream);
    writer->WriteChar('{');
    SerializeFixedHeader();
    SerializeString();
    SerializeAllClassLoad();
    SerializeAllStructClassLoad();
    SerializeStackTrace();
    SerializeStartThread();
    SerializeHeapDump();
    writer->WriteChar('}');
    writer->End();
    stream->SetContext(END);
    writer->WriteString("{\"id\":");
    writer->WriteString(stream->GetMessageID());
    writer->WriteString(",\"result\":{}");
    writer->End();
    delete writer;
    writer = nullptr;
    return true;
}

void CodeHeapDataForIDE::SerializeFixedHeader()
{
    const u4 idSize = sizeof(u4);
    struct timeval timeNow {};
    gettimeofday(&timeNow, nullptr);
    const uint64_t msecsTime = (timeNow.tv_sec * 1000) + (timeNow.tv_usec / 1000);
    const uint32_t timeHigh = static_cast<uint32_t>(msecsTime >> 32);
    const uint32_t timeLow = static_cast<uint32_t>(msecsTime & 0xFFFFFFFF);
    writer->WriteString("\\\"HEADER\\\":[");
    writer->WriteNumber(idSize);
    writer->WriteChar(',');
    writer->WriteNumber(timeHigh);
    writer->WriteChar(',');
    writer->WriteNumber(timeLow);
    writer->WriteChar(']');
    writer->WriteChar(',');
}

void CodeHeapDataForIDE::SerializeString()
{
    writer->WriteString("\\\"STRING\\\":[");
    bool isFirstElement = true;
    for (auto string : strings) {
        if (!isFirstElement) {
            writer->WriteChar(',');
        } else {
            isFirstElement = false;
        }
        writer->WriteChar('[');
        CodeHeapDataStringId id = string.second;
        writer->WriteNumber(GetId(id));
        writer->WriteChar(',');
        CString str = string.first;
        str.ReplaceAll("/", "\\");
        writer->WriteContinueString("\\\"");
        writer->WriteString(str);
        writer->WriteContinueString("\\\"");
        writer->WriteChar(']');
    }
    writer->WriteChar(']');
    writer->WriteChar(',');
}

void CodeHeapDataForIDE::SerializeAllClassLoad()
{
    writer->WriteContinueString("\\\"CLASSLOAD\\\":[");
    bool isFirstElement = true;
    for (auto klassInfo : dumpClassMap) {
        if (!isFirstElement) {
            writer->WriteChar(',');
        } else {
            isFirstElement = false;
        }
        SerializeClassLoad(klassInfo.first, klassInfo.second, TAG_CLASS_LOAD);
    }
    writer->WriteChar(']');
    writer->WriteChar(',');
}

void CodeHeapDataForIDE::SerializeAllClass()
{
    writer->WriteContinueString("\\\"CLASS\\\":[");
    bool isFirstElement = true;
    for (auto klassInfo : dumpClassMap) {
        if (!isFirstElement) {
            writer->WriteChar(',');
        } else {
            isFirstElement = false;
        }
        SerializeClass(klassInfo.first, klassInfo.second, TAG_CLASS_DUMP);
    }
    writer->WriteChar(']');
    writer->WriteChar(',');
}

void CodeHeapDataForIDE::SerializeAllStructClass()
{
    writer->WriteContinueString("\\\"STRUCTCLASS\\\":[");
    bool isFirstElement = true;
    for (auto klassInfo : dumpStructClassMap) {
        if (!isFirstElement) {
            writer->WriteChar(',');
        } else {
            isFirstElement = false;
        }
        SerializeStructClass(klassInfo.first, klassInfo.second, TAG_CLASS_DUMP);
    }
    writer->WriteChar(']');
    writer->WriteChar(',');
}

void CodeHeapDataForIDE::SerializeClassLoad(TypeInfo* ti, CodeHeapDataStringId klassId, const u1 tag)
{
    writer->WriteChar('[');
    writer->WriteNumber(GetId(reinterpret_cast<CodeHeapDataStringId>(ti)));
    writer->WriteChar(',');
    writer->WriteNumber(GetId(reinterpret_cast<CodeHeapDataStringId>(klassId)));
    writer->WriteChar(']');
}

void CodeHeapDataForIDE::SerializeAllStructClassLoad()
{
    writer->WriteContinueString("\\\"STRUCTCLASSLOAD\\\":[");
    bool isFirstElement = true;
    for (auto klassInfo : dumpStructClassMap) {
        if (!isFirstElement) {
            writer->WriteChar(',');
        } else {
            isFirstElement = false;
        }
        SerializeClassLoad(klassInfo.first, klassInfo.second, TAG_CLASS_LOAD);
    }
    writer->WriteChar(']');
    writer->WriteChar(',');
}

void CodeHeapDataForIDE::SerializeStackFrame(FrameInfo& frame, uint32_t frameIdx)
{
    writer->WriteChar('[');
    if (frameIdx > 0 && frame.GetFrameType() == FrameType::NATIVE) {
        return;
    }
    if (frame.GetFrameType() == FrameType::MANAGED) {
        StackMetadataHelper stackMetadataHelper(frame);
        methodName = frame.GetFuncName();
        fileName = frame.GetFileName();
        lineNumber = stackMetadataHelper.GetLineNumber();
    } else {
        Os::Loader::BinaryInfo binInfo;
        (void)Os::Loader::GetBinaryInfoFromAddress(frame.mFrame.GetIP(), &binInfo);
        fileName = CString(binInfo.filePathName);
        methodName = CString(binInfo.symbolName);
    }
    writer->WriteNumber(frames[&frame]);
    writer->WriteChar(',');
    writer->WriteNumber(GetId(LookupStringId(methodName.Str())));
    writer->WriteChar(',');
    writer->WriteNumber(GetId(LookupStringId(fileName.Str())));
    writer->WriteChar(',');
    writer->WriteNumber(reinterpret_cast<u4>(lineNumber));
    writer->WriteChar(']');
}


void CodeHeapDataForIDE::SerializeStackTrace()
{
    bool isFirstElement = true;
    for (auto trace = stacktraces.begin(); trace != stacktraces.end(); trace++) {
        if (!isFirstElement) {
            writer->WriteChar(',');
        } else {
            isFirstElement = false;
        }
        // 10: Default Limit Max Dump Depth as 10 frames
        size_t size = 10;
        size_t depth = trace->first->stacks.size() > size ? size : trace->first->stacks.size();
        std::vector<FrameInfo*> stack = trace->first->stacks;
        writer->WriteContinueString("\\\"STACKFRAME\\\":[");
        for (size_t i = 0; i < depth; ++i) {
            if (i != 0) {
                writer->WriteChar(',');
            }
            SerializeStackFrame(*stack[i], i);
        }
        writer->WriteChar(']');
        writer->WriteChar(',');
        writer->WriteContinueString("\\\"STACKTRACE\\\":[");
        writer->WriteNumber(trace->second);
        writer->WriteChar(',');
        writer->WriteNumber(trace->first->GetStackTid());
        writer->WriteChar(',');
        writer->WriteNumber(depth);
        writer->WriteChar(',');
        writer->WriteChar('[');
        for (size_t i = 0; i < depth; ++i) {
            if (i != 0) {
                writer->WriteChar(',');
            }
            writer->WriteNumber(frames[stack[i]]);
        }
        writer->WriteChar(']');
        writer->WriteChar(']');
    }
    writer->WriteChar(',');
}

void CodeHeapDataForIDE::SerializeStartThread()
{
    bool isFirstElement = true;
    for (auto trace = stacktraces.begin(); trace != stacktraces.end(); trace++) {
        if (!isFirstElement) {
            writer->WriteChar(',');
        } else {
            isFirstElement = false;
        }
        writer->WriteContinueString("\\\"STARTTHREAD\\\":[");
        writer->WriteNumber(trace->first->GetStackTid());
        writer->WriteChar(',');
        writer->WriteNumber(GetId(threadObjectId++));
        writer->WriteChar(',');
        writer->WriteNumber(trace->second);
        writer->WriteChar(',');
        CString threadNameAll = trace->first->GetThreadName();
        writer->WriteNumber(GetId(LookupStringId(threadNameAll)));
        writer->WriteChar(']');
    }
    writer->WriteChar(',');
}

void CodeHeapDataForIDE::SerializeHeapDump()
{
    SerializeAllClass();
    SerializeAllStructClass();
    SerializeAllObjects();
}

void CodeHeapDataForIDE::SerializeAllObjects()
{
    writer->WriteContinueString("\\\"OBJECTS\\\":[");
    bool isFirstElement = true;
    for (auto objectInfo : dumpObjects) {
        if (!isFirstElement) {
            writer->WriteChar(',');
        } else {
            isFirstElement = false;
        }
        switch (objectInfo.tag) {
            case TAG_ROOT_THREAD_OBJECT:
                SerializeThreadObjectRoot(objectInfo.obj, objectInfo.tag, objectInfo.threadId, 0);
                break;
            case TAG_ROOT_LOCAL:
                SerializeLocalRoot(objectInfo.obj, objectInfo.tag, objectInfo.threadId, objectInfo.frameNum);
                break;
            case TAG_ROOT_GLOBAL:
                SerializeGlobalRoot(objectInfo.obj, objectInfo.tag);
                break;
            case TAG_ROOT_UNKNOWN:
                SerializeUnknownRoot(objectInfo.obj, objectInfo.tag);
                break;
            case TAG_OBJECT_ARRAY_DUMP:
                SerializeObjectArray(objectInfo.obj, objectInfo.tag);
                break;
            case  TAG_STRUCT_ARRAY_DUMP:
                SerializeStructArray(objectInfo.obj, objectInfo.tag);
                break;
            case TAG_PRIMITIVE_ARRAY_DUMP:
                SerializePrimitiveArray(objectInfo.obj, objectInfo.tag);
                break;
            case TAG_INSTANCE_DUMP:
                SerializeInstance(objectInfo.obj, objectInfo.tag);
                break;
            default:
                break;
        }
    }
    writer->WriteChar(']');
}

/*
 * Record Global Root Info:
 *     u1 tag;     //denoting the type of this sub-record
 *     ID objId;   // object ID
 */
void CodeHeapDataForIDE::SerializeGlobalRoot(BaseObject*& obj, const u1 tag)
{
    writer->WriteChar('[');
    writer->WriteNumber(tag);
    writer->WriteChar(',');
    writer->WriteNumber(GetObjType(obj));
    writer->WriteChar(',');
    CodeHeapDataID objAddr = (reinterpret_cast<CodeHeapDataID>(obj));
    u4 id = GetId(objAddr);
    CString name = obj->GetTypeInfo()->GetName();
    writer->WriteNumber(id);
    writer->WriteChar(']');
}

/*
 * Record Unknown Root Info:
 *     u1 tag;     // denoting the type of this sub-record
 *     ID objId;   // object ID
 */
void CodeHeapDataForIDE::SerializeUnknownRoot(BaseObject*& obj, const u1 tag)
{
    writer->WriteChar('[');
    writer->WriteNumber(tag);
    writer->WriteChar(',');
    writer->WriteNumber(GetObjType(obj));
    writer->WriteChar(',');
    CodeHeapDataID objAddr = (reinterpret_cast<CodeHeapDataID>(obj));
    u4 id = GetId(objAddr);
    writer->WriteNumber(id);
    writer->WriteChar(']');
}

/*
 * Record Local Root Info:
 *     u1 tag;         // denoting the type of this sub-record
 *     ID objId;       // object ID
 *     u4 threadIdx;   // thread serial number
 *     u4 frame;       // frame number in stack trace (-1 for empty)
 */
void CodeHeapDataForIDE::SerializeLocalRoot(BaseObject*& obj, const u1 tag, const u4 tid, const u1 depth)
{
    writer->WriteChar('[');
    writer->WriteNumber(tag);
    writer->WriteChar(',');
    writer->WriteNumber(GetObjType(obj));
    writer->WriteChar(',');
    CodeHeapDataID objAddr = (reinterpret_cast<CodeHeapDataID>(obj));
    u4 id = GetId(objAddr);
    writer->WriteNumber(id);
    writer->WriteChar(',');
    writer->WriteNumber(tid);
    writer->WriteChar(',');
    writer->WriteNumber(depth);
    writer->WriteChar(']');
}

/*
 * Record Thread Object Root Info:
 *     u1 tag;             // denoting the type of this sub-record
 *     ID threadObjId;     // thread object ID
 *     u4 threadIdx;       // thread serial number
 *     u4 stackTraceIdx;   // stack trace serial number
 *
 */
void CodeHeapDataForIDE::SerializeThreadObjectRoot(BaseObject*& obj, const u1 tag, const u4 tid, const u4 stackTraceIdx)
{
    writer->WriteChar('[');
    writer->WriteNumber(tag);
    writer->WriteChar(',');
    writer->WriteNumber(GetObjType(obj));
    writer->WriteChar(',');
    CodeHeapDataID objAddr = (reinterpret_cast<CodeHeapDataID>(obj));
    u4 id = GetId(objAddr);
    writer->WriteNumber(id);
    writer->WriteChar(',');
    writer->WriteNumber(tid);
    writer->WriteChar(',');
    writer->WriteNumber(stackTraceIdx);
    writer->WriteChar(']');
}

/*
 * Record Class Info:
 *     u1 tag;          // denoting the type of this sub-record
 *     ID classObjId;   // class object ID
 *     u4 size;         // instance size (in bytes)
 */
void CodeHeapDataForIDE::SerializeClass(TypeInfo* ti, CodeHeapDataStringId klassId, const u1 tag)
{
    writer->WriteChar('[');
    writer->WriteNumber(tag);
    writer->WriteChar(',');
    writer->WriteNumber(GetId(reinterpret_cast<CodeHeapDataStringId>(ti)));
    writer->WriteChar(',');
    // 8-byte alignment
    if (!ti->IsObjectType()) {
        writer->WriteNumber(0);
        writer->WriteChar(']');
        return;
    }
    u4 size = AlignUp<u4>((ti->GetInstanceSize() + TYPEINFO_PTR_SIZE), alignment);
    // 8 bytes for each field
    writer->WriteNumber(size);
    writer->WriteChar(']');
}

/*
 * Record Object Array Info:
 *     u1 tag;             // denoting the type of this sub-record
 *     ID arrObjId;        // array object ID
 *     u4 num;             // number of elements
 *     ID arrClassObjId;   // array class object ID
 *     ID elements[num];   // elements
 *
 */
void CodeHeapDataForIDE::SerializeObjectArray(BaseObject*& obj, const u1 tag)
{
    writer->WriteChar('[');
    writer->WriteNumber(tag);
    writer->WriteChar(',');
    writer->WriteNumber(GetObjType(obj));
    writer->WriteChar(',');
    CodeHeapDataID objAddr = (reinterpret_cast<CodeHeapDataID>(obj));
    u4 id = GetId(objAddr);
    writer->WriteNumber(id);
    writer->WriteChar(',');
    u4 num = 0;
    std::stack<u4> VAL;
    RefFieldVisitor visitor = [&VAL, &num, this](RefField<>& arrayContent) {
        VAL.push(GetId(reinterpret_cast<CodeHeapDataID>(arrayContent.GetTargetObject())));
        num++;
    };
    // take array length and content.
    MArray* mArray = reinterpret_cast<MArray*>(obj);
    MIndex arrayLengthVal = mArray->GetLength();
    RefField<>* arrayContent = reinterpret_cast<RefField<>*>(mArray->ConvertToCArray());
    // for each object in array.
    for (MIndex i = 0; i < arrayLengthVal; ++i) {
        visitor(arrayContent[i]);
    }
    writer->WriteNumber(num);
    writer->WriteChar(',');
    writer->WriteNumber(GetId(reinterpret_cast<CodeHeapDataID>(obj->GetTypeInfo())));
    writer->WriteChar(',');
    writer->WriteChar('[');
    bool isFirstElement = true;
    while (!VAL.empty()) {
        if (!isFirstElement) {
            writer->WriteChar(',');
        } else {
            isFirstElement = false;
        }
        u4 val = VAL.top();
        VAL.pop();
        writer->WriteNumber(val);
    }
    writer->WriteChar(']');
    writer->WriteChar(']');
}

/*
 * Record struct Array Info:
 *     u1 tag;             // denoting the type of this sub-record
 *     ID arrObjId;        // array object ID
 *     u4 componentNum;        // component Num
 *     u4 num;             // number of ref fields
 *     ID arrClassObjId;   // array class object ID
 *     ID elements[num];   // elements
 *
 */

void CodeHeapDataForIDE::SerializeStructArray(BaseObject*& obj, const u1 tag)
{
    writer->WriteChar('[');
    writer->WriteNumber(tag);
    writer->WriteChar(',');
    writer->WriteNumber(GetObjType(obj));
    writer->WriteChar(',');
    CodeHeapDataID objAddr = (reinterpret_cast<CodeHeapDataID>(obj));
    u4 id = GetId(objAddr);
    writer->WriteNumber(id);
    writer->WriteChar(',');
    u4 num = 0;
    std::stack<u4> VAL;
    RefFieldVisitor visitor = [&VAL, &num, this](RefField<>& arrayContent) {
        VAL.push(GetId(reinterpret_cast<CodeHeapDataID>(arrayContent.GetTargetObject())));
        num++;
    };
    // take array length and content.
    MArray* mArray = reinterpret_cast<MArray*>(obj);
    MIndex arrayLengthVal = mArray->GetLength();
    TypeInfo* componentTypeInfo = mArray->GetComponentTypeInfo();
    GCTib gcTib = componentTypeInfo->GetGCTib();
    MAddress contentAddr = reinterpret_cast<Uptr>(mArray) + MArray::GetContentOffset();
    if (componentTypeInfo->HasRefField()) {
        for (MIndex i = 0; i < arrayLengthVal; ++i) {
            gcTib.ForEachBitmapWord(contentAddr, visitor);
            contentAddr += mArray->GetElementSize();
        }
    }

    writer->WriteNumber(arrayLengthVal);
    writer->WriteChar(',');
    writer->WriteNumber(num);
    writer->WriteChar(',');
    writer->WriteNumber(GetId(reinterpret_cast<CodeHeapDataID>(obj->GetTypeInfo())));
    writer->WriteChar(',');
    writer->WriteChar('[');
    bool isFirstElement = true;
    while (!VAL.empty()) {
        if (!isFirstElement) {
            writer->WriteChar(',');
        } else {
            isFirstElement = false;
        }
        u4 val = VAL.top();
        VAL.pop();
        writer->WriteNumber(val);
    }
    writer->WriteChar(']');
    writer->WriteChar(']');
}

/*
 * Record Primitive Array Info:
 *     u1 tag;          // denoting the type of this sub-record
 *     ID arrObjId;     // array object
 *     u4 num;          // number of elements
 *     u1 type;         // element type
 */

void CodeHeapDataForIDE::SerializePrimitiveArray(BaseObject*& obj, const u1 tag)
{
    writer->WriteChar('[');
    writer->WriteNumber(tag);
    writer->WriteChar(',');
    writer->WriteNumber(GetObjType(obj));
    writer->WriteChar(',');
    CodeHeapDataID objAddr = (reinterpret_cast<CodeHeapDataID>(obj));
    u4 id = GetId(objAddr);
    writer->WriteNumber(id);
    writer->WriteChar(',');
    MArray* mArray = reinterpret_cast<MArray*>(obj);
    writer->WriteNumber(mArray->GetLength());
    writer->WriteChar(',');
    MSize componentSize = obj->GetTypeInfo()->GetComponentSize();
    switch (componentSize) {
        // bool:1 bytes
        case 1:
            writer->WriteNumber(BOOLEAN);
            break;
        // short:2 bytes
        case 2:
            writer->WriteNumber(SHORT);
            break;
        // int:4 bytes
        case 4:
            writer->WriteNumber(INT);
            break;
        // long:8 bytes
        case 8:
            writer->WriteNumber(LONG);
            break;
        default:
            break;
    }
    writer->WriteChar(']');
}

/*
 * Record Struct Class Info:
 *     u1 tag;          // denoting the type of this sub-record
 *     ID classObjId;   // class object ID
 *     u4 size;         // instance size (in bytes)
 */
void CodeHeapDataForIDE::SerializeStructClass(TypeInfo* ti, CodeHeapDataStringId klassId, const u1 tag)
{
    writer->WriteChar('[');
    writer->WriteNumber(tag);
    writer->WriteChar(',');
    writer->WriteNumber(GetId(reinterpret_cast<CodeHeapDataStringId>(ti)));
    TypeInfo* componentKlass = ti->GetComponentTypeInfo();
    // No alignment required
    u4 size = componentKlass->GetInstanceSize();
    writer->WriteChar(',');
    writer->WriteNumber(size);
    writer->WriteChar(']');
}

/*
 * Record Instance Info:
 *     u1 tag;           // denoting the type of this sub-record
 *     ID objId;         // object ID
 *     ID classObjId;    // class object ID
 *     u4 num;           // number of ref fields
 *     VAL entry[];      // ref contents in instance field values (this class, followed by super class, etc)
 */
void CodeHeapDataForIDE::SerializeInstance(BaseObject*& obj, const u1 tag)
{
    writer->WriteChar('[');
    writer->WriteNumber(tag);
    writer->WriteChar(',');
    writer->WriteNumber(GetObjType(obj));
    writer->WriteChar(',');
    u4 id = GetId(reinterpret_cast<CodeHeapDataID>(obj));
    writer->WriteNumber(id);
    writer->WriteChar(',');
    writer->WriteNumber(GetId(reinterpret_cast<CodeHeapDataID>(obj->GetTypeInfo())));
    writer->WriteChar(',');
    u4 num = 0;
    std::stack<u4> VAL;
    RefFieldVisitor visitor = [&VAL, &num, this](RefField<>& fieldAddr) {
        VAL.push(GetId(reinterpret_cast<CodeHeapDataID>(fieldAddr.GetTargetObject())));
        num++;
    };
    TypeInfo* currentClass = obj->GetTypeInfo();
    if (obj->HasRefField()) {
        GCTib gcTib = currentClass->GetGCTib();
        MAddress objAddr = reinterpret_cast<MAddress>(obj) + TYPEINFO_PTR_SIZE;
        gcTib.ForEachBitmapWord(objAddr, visitor);
    }
    writer->WriteNumber(num);
    writer->WriteChar(',');
    writer->WriteChar('[');
    bool isFirstElement = true;
    while (!VAL.empty()) {
        if (!isFirstElement) {
            writer->WriteChar(',');
        } else {
            isFirstElement = false;
        }
        u4 val = VAL.top();
        VAL.pop();
        writer->WriteNumber(val);
    }
    writer->WriteChar(']');
    writer->WriteChar(']');
}

CodeHeapData::u4 CodeHeapDataForIDE::GetId(CodeHeapDataStringId klassId)
{
    if (stringIdxMap.find(klassId) == stringIdxMap.end()) {
        stringIdxMap[klassId] = ++stringIdx;
    }
    return stringIdxMap[klassId];
}

I8 CodeHeapDataForIDE::GetObjType(BaseObject* obj)
{
    I8 objType = TypeKind::TYPE_KIND_NOTHING;
    if (obj != nullptr) {
        TypeInfo* typeInfo = obj->GetTypeInfo();
        if (typeInfo != nullptr) {
            objType = typeInfo->GetType();
        }
    }
    return objType;
}
}
