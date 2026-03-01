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


#include "CodeHeapData.h"
#include <cerrno>
#include <Common/BaseObject.h>
#include <Common/Runtime.h>
#include <Common/ScopedObjectAccess.h>
#include <Heap/Collector/TaskQueue.h>
#include <Heap/Collector/TracingCollector.h>
#include <sys/time.h>

#include "ObjectModel/MArray.inline.h"
#include "Common/BaseObject.h"
#include "Common/StackType.h"
#include "Concurrency/ConcurrencyModel.h"
#include "Heap/Heap.h"
#include "ObjectModel/RefField.h"
#include "Sync/Sync.h"
#include "UnwindStack/PrintStackInfo.h"
#include "UnwindStack/StackInfo.h"
#include "UnwindStack/StackMetadataHelper.h"
namespace MapleRuntime {
static bool g_oomIsTrigged = false;

void CodeHeapData::WriteHeap()
{
    WriteFixedHeader();
    WriteString();
    WriteAllClassLoad();
    WriteAllStructClassLoad();
    WriteStackTrace();
    WriteStartThread();
    WriteHeapDump();
}

void CodeHeapData::ProcessHeap()
{
    ProcessRootGlobal();
    ProcessRootLocal();
    ProcessRootConcurrencyModel();
    ProcessRootFinalizer();
    (void)LookupStringId("reserved");
    (void)LookupStringId("RefFields");
    (void)LookupStringId("ValueField");
    //  dump object contents
    auto dumpVisitor = [this](BaseObject* obj) { ProcessHeapObject(obj); };
    bool ret = Heap::GetHeap().ForEachObj(dumpVisitor, false);
    CHECK_E(UNLIKELY(!ret), "theAllocator.ForEachObj() in DumpHeap() return false.");
}

void CodeHeapData::DumpHeap()
{
    // step1 - open file
    CString specifiedPath;
    if (dumpAfterOOM && !g_oomIsTrigged) {
        g_oomIsTrigged = true;
        Logger::GetLogger().GetLogPath("codeHeapDumpLog", specifiedPath);
        auto pid = MapleRuntime::GetPid();
        CString dumpFile = CString("code_OOM_pid") + CString(pid) + CString(".dat");
#if defined(_WIN64)
        const char* separator = "\\";
#else
        const char* separator = "/";
#endif
        if (specifiedPath.IsEmpty()) {
            // dump to current path
            fp = fopen(dumpFile.Str(), "wb");
            PRINT_INFO("Heap dump log is writing into .%s%s ...\n", separator, dumpFile.Str());
        } else {
            // dump to specified path
            dumpFile = specifiedPath + separator + dumpFile;
            fp = fopen(dumpFile.Str(), "wb");
            PRINT_INFO("Heap dump log is writing into %s ...\n", dumpFile.Str());
        }
    } else {
        // dump for codeprof
        fp = fopen("item_data.dat.cache", "wb");
    }

    if (!fp) {
        LOG(RTLOG_ERROR, "Failed to open heap dump file, stop dumping heap info, %s", strerror(errno));
        return;
    }
    // step2 - write file
    ScopedStopTheWorld scopedStopTheWorld("dump heap to file");
    ProcessHeap();
    WriteHeap();

    // step3 - close file
    int ret = fclose(fp);
    fp = nullptr;
    if (ret) {
        LOG(RTLOG_ERROR, "Fail to close file when dump heap data finished, %s", strerror(errno));
    }
    if (!dumpAfterOOM) {
        const int strLen = 20;
        char* str = static_cast<char*>(NativeAllocator::NativeAlloc(strLen * sizeof(char)));
        sprintf_s(str, strLen, "%s", "item_data.dat.cache");
        ret = rename(str, "item_data.dat");
        NativeAllocator::NativeFree(str, strLen * sizeof(char));
        if (ret) {
            LOG(RTLOG_ERROR, "Fail to rename file when dump heap data finished, %s", strerror(errno));
        }
    }
}

bool CodeHeapData::DumpHeap(int fd)
{
    int copyfd = dup(fd);
    if (copyfd == -1) {
        LOG(RTLOG_ERROR, "Failed to open heap dump file, stop dumping heap info, %s", strerror(errno));
    }
    fp = fdopen(copyfd, "wb");
    if (fp == nullptr) {
        close(copyfd);
        LOG(RTLOG_ERROR, "Failed to open heap dump file, stop dumping heap info, %s", strerror(errno));
        return false;
    }

    ScopedStopTheWorld scopedStopTheWorld("dump heap to fd");
    ProcessHeap();
    WriteHeap();

    // fclose will close fd
    // if fclose success, no need to close fd.
    // if fclose failed, close fd to prevent resource leakage.
    int ret = fclose(fp);
    fp = nullptr;
    if (ret) {
        close(copyfd);
        LOG(RTLOG_ERROR, "Fail to close file when dump heap data finished, %s", strerror(errno));
        return false;
    }
    return true;
}

void CodeHeapData::ProcessHeapObject(BaseObject* obj)
{
    if (obj == nullptr) {
        return;
    }

    if (obj->IsRawArray()) {
        MArray* mArray = reinterpret_cast<MArray*>(obj);
        TypeInfo* componentTypeInfo = mArray->GetComponentTypeInfo();
        if (componentTypeInfo->IsPrimitiveType()) {
            DumpObject dumpObject = { obj, TAG_PRIMITIVE_ARRAY_DUMP, 0, 0,
                                      LookupStringId(obj->GetTypeInfo()->GetName() == nullptr ?
                                                         "anonymousPrimitiveArray" :
                                                         obj->GetTypeInfo()->GetName()) };
            dumpObjects.push_back(dumpObject);
        } else if (componentTypeInfo->IsStructType()) {
            DumpObject dumpObject = { obj, TAG_STRUCT_ARRAY_DUMP, 0, 0,
                                      LookupStringId(obj->GetTypeInfo()->GetName() == nullptr ?
                                                         "anonymousStructArray" :
                                                         obj->GetTypeInfo()->GetName()) };
            dumpObjects.push_back(dumpObject);
            ProcessStructClass(obj->GetTypeInfo());
            return;
        } else if (componentTypeInfo->IsObjectType() ||
                   componentTypeInfo->IsArrayType() ||
                   componentTypeInfo->IsInterface()) {
            DumpObject dumpObject = { obj, TAG_OBJECT_ARRAY_DUMP, 0, 0,
                                      LookupStringId(obj->GetTypeInfo()->GetName() == nullptr ?
                                                         "anonymousObjectArray" :
                                                         obj->GetTypeInfo()->GetName()) };
            dumpObjects.push_back(dumpObject);
        } else {
            LOG(RTLOG_ERROR, "array object %p has wrong component type", mArray);
        }
    } else if (obj->GetTypeInfo()->IsVaildType()) {
        DumpObject dumpObject = {
            obj, TAG_INSTANCE_DUMP, 0, 0,
            LookupStringId(obj->GetTypeInfo()->GetName() == nullptr ? "defaultLambda" : obj->GetTypeInfo()->GetName())
        };
        dumpObjects.push_back(dumpObject);
    } else {
        LOG(RTLOG_ERROR, "object %p has wrong component type", obj);
        return;
    }
    ProcessRootClass(obj->GetTypeInfo());
}

void CodeHeapData::ProcessRootClass(TypeInfo* klass)
{
    if (dumpClassMap.find(klass) == dumpClassMap.end()) {
        dumpClassMap.insert(std::pair<TypeInfo*, CodeHeapDataStringId>(
            klass,
            LookupStringId(klass->GetName() == nullptr ? "defaultLambda" :
                                                         klass->GetName()))); // lamda obj has null name
    }
}

void CodeHeapData::ProcessStructClass(TypeInfo* klass)
{
    if (dumpStructClassMap.find(klass) == dumpStructClassMap.end()) {
        dumpStructClassMap.insert(std::pair<TypeInfo*, CodeHeapDataStringId>(
            klass,
            LookupStringId(klass->GetName() == nullptr ? "defaultStructLamda" :
                                                         klass->GetName()))); // lamda obj has null name
    }
}
void CodeHeapData::ProcessStacktrace(RecordStackInfo* recordStackInfo)
{
    std::vector<FrameInfo*> framesInStack = recordStackInfo->stacks;
    if (stacktraces.find(recordStackInfo) == stacktraces.end()) {
        stacktraces.insert(
            std::pair<RecordStackInfo*, CodeHeapDataStackTraceSerialNumber>(recordStackInfo, traceSerialNum++));
        CString threadIdx = CString(threadId);
        LookupStringId(threadName);
        for (size_t i = 0; i < framesInStack.size(); ++i) {
            FrameInfo* frame = framesInStack[i];
            if (frame->GetFrameType() == FrameType::MANAGED) {
                LookupStringId(frame->GetFuncName().Str());
                LookupStringId(frame->GetFileName().Str());
            } else {
                Os::Loader::BinaryInfo binInfo;
                (void)Os::Loader::GetBinaryInfoFromAddress(frame->mFrame.GetIP(), &binInfo);
                LookupStringId(CString(binInfo.filePathName).Str());
                LookupStringId(CString(binInfo.symbolName).Str());
            }
            frames.insert(std::pair<FrameInfo*, CodeHeapDataStackFrameId>(frame, frameId++));
        }
    }
}

void CodeHeapData::ProcessRootLocal()
{
    MutatorManager::Instance().VisitAllMutators([this](Mutator &mutator) {
        // skip finalizer
        if (!mutator.IsVaildCODEThread()) {
            return;
        }
        threadId = static_cast<uint32_t>(mutator.GetCODEThreadId());
        if (mutator.GetCODEThreadName() == nullptr) {
            threadName = CString("defaultName") + CString(threadId);
        } else {
            threadName = CString(mutator.GetCODEThreadName());
        }
        RecordStackInfo* recordStackInfo = new RecordStackInfo(&(mutator.GetUnwindContext()), threadId, threadName);
        recordStackInfo->FillInStackTrace();

        ProcessStacktrace(recordStackInfo);
        RootVisitor rootVisitor = [this, recordStackInfo](ObjectRef &objRef) {
            BaseObject* obj = objRef.object;
            if (obj == nullptr || !Heap::IsHeapAddress(obj)) {
                return;
            }
            DumpObject dumpObject = {obj,
                TAG_ROOT_LOCAL,
                threadId,
                recordStackInfo->GetCurrentFrame(),
                LookupStringId(obj->GetTypeInfo()->GetName() == nullptr ? "anonymous" : obj->GetTypeInfo()->GetName())
            };
            dumpObjects.push_back(dumpObject);
        };
        recordStackInfo->VisitStackRoots(rootVisitor, mutator);
    });
}

void CodeHeapData::ProcessRootGlobal()
{
    RefFieldVisitor visitor = [this](RefField<>& refField) {
        BaseObject* obj = Heap::GetBarrier().ReadStaticRef(refField);
        if (obj == nullptr || !Heap::IsHeapAddress(obj)) {
            return;
        }
        DumpObject dumpObject = {
            obj, TAG_ROOT_GLOBAL, 0, 0,
            LookupStringId(obj->GetTypeInfo()->GetName() == nullptr ? "anonymous" : obj->GetTypeInfo()->GetName())
        };
        dumpObjects.push_back(dumpObject);
    };
    Heap::GetHeap().VisitStaticRoots(visitor);
}

void CodeHeapData::ProcessRootConcurrencyModel()
{
    RootVisitor visitor = [this](ObjectRef& objRef) {
        BaseObject* obj = objRef.object;
        if (obj == nullptr || !Heap::IsHeapAddress(obj)) {
            return;
        }
        DumpObject dumpObject = {
            obj, TAG_ROOT_UNKNOWN, 0, 0,
            LookupStringId(obj->GetTypeInfo()->GetName() == nullptr ? "anonymous" : obj->GetTypeInfo()->GetName())
        };
        dumpObjects.push_back(dumpObject);
    };
    Runtime::Current().GetConcurrencyModel().VisitGCRoots(&visitor);
}

void CodeHeapData::ProcessRootFinalizer()
{
    RootVisitor visitor = [this](ObjectRef& objRef) {
        BaseObject* obj = objRef.object;
        if (obj == nullptr || !Heap::IsHeapAddress(obj)) {
            return;
        }
        DumpObject dumpObject = {
            obj, TAG_ROOT_UNKNOWN, 0, 0,
            LookupStringId(obj->GetTypeInfo()->GetName() == nullptr ? "anonymous" : obj->GetTypeInfo()->GetName())
        };
        dumpObjects.push_back(dumpObject);
    };
    Heap::GetHeap().GetFinalizerProcessor().VisitGCRoots(visitor);
}

void CodeHeapData::WriteHeapDump()
{
    WriteRecordHeader(TAG_HEAP_DUMP, kCodeHeapDataTime);
    WriteAllClass();
    WriteAllStructClass();
    WriteAllObjects();
    ModifyLength();
    EndRecord();
}
/*
 * Record thread info:
 *     RecordHeader header;
 *     u4 thread serial number
 *     ID thread object ID
 *     u4 stack trace serial number
 *     ID thread name string ID
 */
void CodeHeapData::WriteStartThread()
{
    for (auto trace = stacktraces.begin(); trace != stacktraces.end(); trace++) {
        WriteRecordHeader(TAG_START_THREAD, kCodeHeapDataTime);
        AddU4(trace->first->GetStackTid());
        AddID(threadObjectId++);
        AddU4(trace->second);
        CString threadNameAll = trace->first->GetThreadName();
        AddID(LookupStringId(threadNameAll));
        ModifyLength();
        EndRecord();
    }
}

void CodeHeapData::WriteAllClass()
{
    for (auto klassInfo : dumpClassMap) {
        WriteClass(klassInfo.first, klassInfo.second, TAG_CLASS_DUMP);
    }
}

void CodeHeapData::WriteAllStructClass()
{
    for (auto klassInfo : dumpStructClassMap) {
        WriteStructClass(klassInfo.first, klassInfo.second, TAG_CLASS_DUMP);
    }
}

void CodeHeapData::WriteAllObjects()
{
    for (auto objectInfo : dumpObjects) {
        switch (objectInfo.tag) {
            case TAG_ROOT_THREAD_OBJECT:
                WriteThreadObjectRoot(objectInfo.obj, objectInfo.tag, objectInfo.threadId, 0);
                break;
            case TAG_ROOT_LOCAL:
                WriteLocalRoot(objectInfo.obj, objectInfo.tag, objectInfo.threadId, objectInfo.frameNum);
                break;
            case TAG_ROOT_GLOBAL:
                WriteGlobalRoot(objectInfo.obj, objectInfo.tag);
                break;
            case TAG_ROOT_UNKNOWN:
                WriteUnknownRoot(objectInfo.obj, objectInfo.tag);
                break;
            case TAG_OBJECT_ARRAY_DUMP:
                WriteObjectArray(objectInfo.obj, objectInfo.tag);
                break;
            case TAG_STRUCT_ARRAY_DUMP:
                WriteStructArray(objectInfo.obj, objectInfo.tag);
                break;
            case TAG_PRIMITIVE_ARRAY_DUMP:
                WritePrimitiveArray(objectInfo.obj, objectInfo.tag);
                break;
            case TAG_INSTANCE_DUMP:
                WriteInstance(objectInfo.obj, objectInfo.tag);
                break;
            default:
                break;
        }
    }
}
/*
 * Record Global Root Info:
 *     u1 tag;     //denoting the type of this sub-record
 *     ID objId;   // object ID
 */
void CodeHeapData::WriteGlobalRoot(BaseObject*& obj, const u1 tag)
{
    AddU1(tag);
    CodeHeapDataID objId = (reinterpret_cast<CodeHeapDataID>(obj));
    CString name = obj->GetTypeInfo()->GetName();
    AddStringId(objId);
}

/*
 * Record Unknown Root Info:
 *     u1 tag;     // denoting the type of this sub-record
 *     ID objId;   // object ID
 */
void CodeHeapData::WriteUnknownRoot(BaseObject*& obj, const u1 tag)
{
    AddU1(tag);
    CodeHeapDataID objId = (reinterpret_cast<CodeHeapDataID>(obj));
    AddStringId(objId);
}

/*
 * Record Local Root Info:
 *     u1 tag;         // denoting the type of this sub-record
 *     ID objId;       // object ID
 *     u4 threadIdx;   // thread serial number
 *     u4 frame;       // frame number in stack trace (-1 for empty)
 */
void CodeHeapData::WriteLocalRoot(BaseObject*& obj, const u1 tag, const u4 tid, const u1 depth)
{
    AddU1(tag);
    CodeHeapDataID objId = (reinterpret_cast<CodeHeapDataID>(obj));
    AddStringId(objId);
    AddU4(tid);
    AddU4(depth);
}

/*
 * Record Thread Object Root Info:
 *     u1 tag;             // denoting the type of this sub-record
 *     ID threadObjId;     // thread object ID
 *     u4 threadIdx;       // thread serial number
 *     u4 stackTraceIdx;   // stack trace serial number
 *
 */
void CodeHeapData::WriteThreadObjectRoot(BaseObject*& obj, const u1 tag, const u4 tid, const u4 stackTraceIdx)
{
    AddU1(tag);
    CodeHeapDataID objId = (reinterpret_cast<CodeHeapDataID>(obj));
    AddStringId(objId);
    AddU4(tid);
    AddU4(stackTraceIdx);
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
void CodeHeapData::WriteObjectArray(BaseObject*& obj, const u1 tag)
{
    AddU1(tag);
    CodeHeapDataID objId = (reinterpret_cast<CodeHeapDataID>(obj));
    AddStringId(objId);
    u4 num = 0;
    std::stack<u8> VAL;
    RefFieldVisitor visitor = [&VAL, &num](RefField<>& arrayContent) {
        VAL.push(reinterpret_cast<CodeHeapDataID>(arrayContent.GetTargetObject()));
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
    AddU4(num);
    AddStringId(reinterpret_cast<CodeHeapDataID>(obj->GetTypeInfo()));
    while (!VAL.empty()) {
        u8 val = VAL.top();
        VAL.pop();
        AddU8(val);
    }
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

 void CodeHeapData::WriteStructArray(BaseObject*& obj, const u1 tag)
{
    AddU1(tag);
    CodeHeapDataID objId = (reinterpret_cast<CodeHeapDataID>(obj));
    AddStringId(objId);
    u4 num = 0;
    std::stack<u8> VAL;
    RefFieldVisitor visitor = [&VAL, &num](RefField<>& arrayContent) {
        VAL.push(reinterpret_cast<CodeHeapDataID>(arrayContent.GetTargetObject()));
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
    AddU4(arrayLengthVal);
    AddU4(num);
    AddStringId(reinterpret_cast<CodeHeapDataID>(obj->GetTypeInfo()));
    while (!VAL.empty()) {
        u8 val = VAL.top();
        VAL.pop();
        AddU8(val);
    }
}

/*
 * Record Primitive Array Info:
 *     u1 tag;          // denoting the type of this sub-record
 *     ID arrObjId;     // array object
 *     u4 num;          // number of elements
 *     u1 type;         // element type
 */

void CodeHeapData::WritePrimitiveArray(BaseObject*& obj, const u1 tag)
{
    MSize componentSize = obj->GetTypeInfo()->GetComponentSize();
    if (componentSize == 0) {
        return;
    }
    AddU1(tag);
    CodeHeapDataID objId = (reinterpret_cast<CodeHeapDataID>(obj));
    AddStringId(objId);
    MArray* mArray = reinterpret_cast<MArray*>(obj);
    AddU4(mArray->GetLength());
    switch (componentSize) {
        // bool:1 bytes
        case 1:
            AddU1(BOOLEAN);
            break;
        // short:2 bytes
        case 2:
            AddU1(SHORT);
            break;
        // int:4 bytes
        case 4:
            AddU1(INT);
            break;
        // long:8 bytes
        case 8:
            AddU1(LONG);
            break;
        default:
            break;
    }
}

/*
 * Record Struct Class Info:
 *     u1 tag;          // denoting the type of this sub-record
 *     ID classObjId;   // class object ID
 *     u4 size;         // instance size (in bytes)
 */
void CodeHeapData::WriteStructClass(TypeInfo* klass, CodeHeapDataStringId klassId, const u1 tag)
{
    AddU1(tag);
    AddStringId(reinterpret_cast<u8>(klass));
    TypeInfo* componentKlass = klass->GetComponentTypeInfo();
    // No alignment required
    u4 size = componentKlass->GetInstanceSize();
    AddU4(size);
}

/*
 * Record Class Info:
 *     u1 tag;          // denoting the type of this sub-record
 *     ID classObjId;   // class object ID
 *     u4 size;         // instance size (in bytes)
 */
void CodeHeapData::WriteClass(TypeInfo* klass, CodeHeapDataStringId klassId, const u1 tag)
{
    AddU1(tag);
    AddStringId(reinterpret_cast<u8>(klass));
    // 8-byte alignment
    if (!klass->IsObjectType()) {
        AddU4(0);
        return;
    }
    u4 size = AlignUp<u4>((klass->GetInstanceSize() + TYPEINFO_PTR_SIZE), alignment);
    // 8 bytes for each field
    AddU4(size);
}

/*
 * Record Instance Info:
 *     u1 tag;           // denoting the type of this sub-record
 *     ID objId;         // object ID
 *     ID classObjId;    // class object ID
 *     u4 num;           // number of ref fields
 *     VAL entry[];      // ref contents in instance field values (this class, followed by super class, etc)
 */
void CodeHeapData::WriteInstance(BaseObject*& obj, const u1 tag)
{
    AddU1(tag);
    CodeHeapDataID objId = (reinterpret_cast<CodeHeapDataID>(obj));
    AddStringId(objId);
    AddStringId(reinterpret_cast<CodeHeapDataID>(obj->GetTypeInfo()));
    u4 num = 0;
    std::stack<u8> VAL;
    RefFieldVisitor visitor = [&VAL, &num](RefField<>& fieldAddr) {
        VAL.push(reinterpret_cast<u8>(fieldAddr.GetTargetObject()));
        num++;
    };
    TypeInfo* currentClass = obj->GetTypeInfo();
    if (obj->HasRefField()) {
        GCTib gcTib = currentClass->GetGCTib();
        MAddress objAddr = reinterpret_cast<MAddress>(obj) + TYPEINFO_PTR_SIZE;
        gcTib.ForEachBitmapWord(objAddr, visitor);
    }
    AddU4(num);
    while (!VAL.empty()) {
        u8 val = VAL.top();
        VAL.pop();
        AddU8(val);
    }
}

/*
 * Record String Info:
 *     RecordHeader header;
 *     ID id;      // ID for this string
 *     u1 str[];   // UTF8 characters for string (NOT NULL terminated)
 */
void CodeHeapData::WriteString()
{
    for (auto string : strings) {
        WriteRecordHeader(TAG_STRING_IN_UTF8, kCodeHeapDataTime);
        CodeHeapDataStringId id = string.second;
        AddStringId(id);
        const CString str = string.first;
        AddU1List(reinterpret_cast<const uint8_t*>(str.Str()), str.Length());
        ModifyLength();
        EndRecord();
    }
}

/*
 * Record Stack Frame Info:
 *     RecordHeader header;
 *     ID frameId;      // stack frame ID
 *     ID methodNameId; // method name string ID
 *     ID srcFileNameId;   // source file name string ID
 *     u4 line;         // line number(>0: line number, 0: no line information available, -1: unknown
 *     location)
 */
void CodeHeapData::WriteStackFrame(FrameInfo& frame, uint32_t frameIdx)
{
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
    WriteRecordHeader(TAG_STACK_FRAME, kCodeHeapDataTime);
    AddStringId(frames[&frame]);
    AddStringId(LookupStringId(methodName.Str()));
    AddStringId(LookupStringId(fileName.Str()));
    AddU4(reinterpret_cast<u4>(lineNumber));
    ModifyLength();
    EndRecord();
}

/*
 * Record Stack Trace Info:
 *     RecordHeader header;
 *     u4 stackTraceIdx;   // stack trace serial number
 *     u4 threadIdx;    // thread serial number
 *     u4 frameNum;     // number of frames
 *     ID frames[];     // series of stack frame ID's
 */
void CodeHeapData::WriteStackTrace()
{
    for (auto trace = stacktraces.begin(); trace != stacktraces.end(); trace++) {
        auto env = std::getenv("DumpStackDepth");
        // 10: Default Limit Max Dump Depth as 10 frames
        size_t size = CString::ParseNumFromEnv(env) == 0 ? 10 : CString::ParseNumFromEnv(env);
        size_t depth = trace->first->stacks.size() > size ? size : trace->first->stacks.size();
        std::vector<FrameInfo*> stack = trace->first->stacks;
        for (size_t i = 0; i < depth; ++i) {
            WriteStackFrame(*stack[i], i);
        }
        WriteRecordHeader(TAG_STACK_TRACE, kCodeHeapDataTime);
        AddU4(trace->second);
        AddU4(trace->first->GetStackTid());
        AddU4(depth);
        for (size_t i = 0; i < depth; ++i) {
            AddStringId(frames[stack[i]]);
        }
        ModifyLength();
        EndRecord();
    }
}

/*
 * Record Record Header Info:
 *     u1 tag;    // denoting the type of the record
 *     u4 time;   // number of microseconds since the time stamp in the header
 *     u4 length; // number of bytes that follow this u4 field and belong to this record
 */
void CodeHeapData::WriteRecordHeader(const u1 tag, const u4 time)
{
    AddU1(tag);
    // DEADDEADEADDEAD: placeholder, the actual length is filled in Func modifyLength.
    const u8 tmpLens = 0xDEADDEADEADDEAD;
    AddU8(tmpLens);
}

void CodeHeapData::AddU1(const u1 value) { AddU1List(&value, 1); }

void CodeHeapData::AddU2(const u2 value) { AddU2List(&value, 1); }

void CodeHeapData::AddU4(const u4 value) { AddU4List(&value, 1); }

void CodeHeapData::AddU8(const u8 value) { AddU8List(&value, 1); }

void CodeHeapData::AddID(const u8 value) { AddU8List(&value, 1); }

void CodeHeapData::AddU1List(const u1* value, uint8_t count)
{
    HandleAddU1(value, count);
    length += count;
}

void CodeHeapData::AddU2List(const u2* value, uint8_t count)
{
    HandleAddU2(value, count);
    length += count * sizeof(u2);
    ;
}

void CodeHeapData::AddU4List(const u4* value, uint8_t count)
{
    HandleAddU4(value, count);
    length += count * sizeof(u4);
}
void CodeHeapData::AddU8List(const u8* value, uint8_t count)
{
    HandleAddU8(value, count);
    length += count * sizeof(u8);
}

enum ByteOffset {
    FIRST_BYTE = 0 * 8,
    SECOND_BYTE = 1 * 8,
    THIRD_BYTE = 2 * 8,
    FOURTH_BYTE = 3 * 8,
    FIFTH_BYTE = 4 * 8,
    SIXTH_BYTE = 5 * 8,
    SEVENTH_BYTE = 6 * 8,
    EIGHTH_BYTE = 7 * 8
};

void CodeHeapData::HandleAddU1(const u1* value, uint8_t count) { buffer.insert(buffer.end(), value, value + count); }

void CodeHeapData::HandleAddU2(const u2* value, uint8_t count)
{
    for (int i = 0; i < count; i++) {
        buffer.push_back(static_cast<uint8_t>((*value >> SECOND_BYTE) & 0xFF));
        buffer.push_back(static_cast<uint8_t>((*value >> FIRST_BYTE) & 0xFF));
        value++;
    }
}

void CodeHeapData::HandleAddU4(const u4* value, uint8_t count)
{
    for (int i = 0; i < count; i++) {
        buffer.push_back(static_cast<uint8_t>((*value >> FOURTH_BYTE) & 0xFF));
        buffer.push_back(static_cast<uint8_t>((*value >> THIRD_BYTE) & 0xFF));
        buffer.push_back(static_cast<uint8_t>((*value >> SECOND_BYTE) & 0xFF));
        buffer.push_back(static_cast<uint8_t>((*value >> FIRST_BYTE) & 0xFF));
        value++;
    }
}

void CodeHeapData::HandleAddU8(const u8* value, uint8_t count)
{
    // 0: offset for 1st byte
    // 8: offset for 2nd byte
    // 16: offset for 3st byte
    // 24: offset for 4st byte
    // 32: offset for 5st byte
    // 40: offset for 6st byte
    // 48: offset for 7st byte
    // 56: offset for 8st byte
    for (int i = 0; i < count; i++) {
        buffer.push_back(static_cast<uint8_t>((*value >> EIGHTH_BYTE) & 0xFF));
        buffer.push_back(static_cast<uint8_t>((*value >> SEVENTH_BYTE) & 0xFF));
        buffer.push_back(static_cast<uint8_t>((*value >> SIXTH_BYTE) & 0xFF));
        buffer.push_back(static_cast<uint8_t>((*value >> FIFTH_BYTE) & 0xFF));
        buffer.push_back(static_cast<uint8_t>((*value >> FOURTH_BYTE) & 0xFF));
        buffer.push_back(static_cast<uint8_t>((*value >> THIRD_BYTE) & 0xFF));
        buffer.push_back(static_cast<uint8_t>((*value >> SECOND_BYTE) & 0xFF));
        buffer.push_back(static_cast<uint8_t>((*value >> FIRST_BYTE) & 0xFF));
        value++;
    }
}

void CodeHeapData::AddStringId(CodeHeapData::CodeHeapDataStringId value) { AddID(static_cast<const u8>(value)); }

void CodeHeapData::EndRecord()
{
    const char* ptr = reinterpret_cast<const char*>(buffer.data());
    fwrite(ptr, length, 1, fp);
    length = 0;
    std::vector<uint8_t>().swap(buffer);
}

void CodeHeapData::WriteFixedHeader()
{
    const char ident[] = "CODIRA PROFILE 1.0.1";
    AddU1List(reinterpret_cast<const uint8_t*>(ident), sizeof(ident));
    const u4 idSize = sizeof(CodeHeapDataID);
    AddU4(idSize);
    struct timeval timeNow {};
    gettimeofday(&timeNow, nullptr);
    const uint64_t msecsTime = (timeNow.tv_sec * 1000) + (timeNow.tv_usec / 1000);
    const uint32_t timeHigh = static_cast<uint32_t>(msecsTime >> 32);
    const uint32_t timeLow = static_cast<uint32_t>(msecsTime & 0xFFFFFFFF);
    AddU4(timeHigh);
    AddU4(timeLow);
    EndRecord();
}
void CodeHeapData::WriteAllClassLoad()
{
    for (auto klassInfo : dumpClassMap) {
        WriteClassLoad(klassInfo.first, klassInfo.second, TAG_CLASS_LOAD);
    }
}

void CodeHeapData::WriteAllStructClassLoad()
{
    for (auto klassInfo : dumpStructClassMap) {
        WriteClassLoad(klassInfo.first, klassInfo.second, TAG_CLASS_LOAD);
    }
}

/*
 * Record Class Load Info:
 *     ID class object ID
 *     ID  class name string ID
 */
void CodeHeapData::WriteClassLoad(TypeInfo* klass, CodeHeapDataStringId klassId, const u1 tag)
{
    WriteRecordHeader(tag, kCodeHeapDataTime);
    AddID(reinterpret_cast<u8>(klass));
    AddStringId(klassId);
    ModifyLength();
    EndRecord();
}

void CodeHeapData::ModifyLength()
{
    // 9: Subtract the length of the record header
    constexpr uint8_t recordHeaderLength = 9;
    uint64_t value = length - recordHeaderLength;
    // 1,2,3,4,5,6,7,8: Stores 64 bits for length
    buffer[1] = (static_cast<uint8_t>((value >> EIGHTH_BYTE) & 0xFF));
    buffer[2] = (static_cast<uint8_t>((value >> SEVENTH_BYTE) & 0xFF));
    buffer[3] = (static_cast<uint8_t>((value >> SIXTH_BYTE) & 0xFF));
    buffer[4] = (static_cast<uint8_t>((value >> FIFTH_BYTE) & 0xFF));
    buffer[5] = (static_cast<uint8_t>((value >> FOURTH_BYTE) & 0xFF));
    buffer[6] = (static_cast<uint8_t>((value >> THIRD_BYTE) & 0xFF));
    buffer[7] = (static_cast<uint8_t>((value >> SECOND_BYTE) & 0xFF));
    buffer[8] = (static_cast<uint8_t>((value >> FIRST_BYTE) & 0xFF));
}

CodeHeapData::CodeHeapDataStringId CodeHeapData::LookupStringId(const CString& string)
{
    auto it = strings.find(string);
    if (it != strings.end()) {
        return it->second;
    }
    CodeHeapData::CodeHeapDataStringId id = stringId++;
    strings.insert(std::pair<CString, CodeHeapDataStringId>(string, id));
    return id;
}
} // namespace MapleRuntime
