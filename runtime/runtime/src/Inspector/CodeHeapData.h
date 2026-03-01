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


#ifndef MRT_CODE_HEAP_DATA_H
#define MRT_CODE_HEAP_DATA_H

#include <Common/BaseObject.h>
#include <Common/StackType.h>
#include <map>
#include <set>
#include <stack>
#include <sys/time.h>
#include "Base/CString.h"
#include "UnwindStack/GcStackInfo.h"
namespace MapleRuntime {
class CodeHeapData {
public:
    CodeHeapData() = default;
    explicit CodeHeapData(bool fromOOM) : dumpAfterOOM(fromOOM) {}

    ~CodeHeapData()
    {
        auto iter = stacktraces.begin();
        while (iter != stacktraces.end()) {
            auto recordStackInfo = iter->first;
            iter++;
            delete recordStackInfo;
            recordStackInfo = nullptr;
        }
    }

    using u1 = uint8_t;
    using u2 = uint16_t;
    using u4 = uint32_t;
    using u8 = uint64_t;
    using CodeHeapDataID = u8;
    using CodeHeapDataStringId = CodeHeapDataID;
    using CodeHeapDataStackFrameId = CodeHeapDataID;
    using CodeHeapDataStackTraceSerialNumber = CodeHeapDataID;
    static constexpr CodeHeapDataStackTraceSerialNumber kCodeHeapDataNullStackTrace = 0;
    const static size_t alignment = 8;

    enum CodeHeapDataTag {
        TAG_STRING_IN_UTF8 = 0x01,
        TAG_CLASS_LOAD = 0x02,
        TAG_STACK_FRAME = 0x04,
        TAG_STACK_TRACE = 0x05,
        TAG_HEAP_DUMP = 0x0c,
        TAG_START_THREAD = 0x0A,
    };

    enum DumpTag {
        TAG_ROOT_UNKNOWN = 0xFF,
        TAG_ROOT_GLOBAL = 0x01,
        TAG_ROOT_LOCAL = 0x02,
        TAG_ROOT_THREAD_OBJECT = 0x08,
        TAG_CLASS_DUMP = 0x20,
        TAG_INSTANCE_DUMP = 0x21,
        TAG_OBJECT_ARRAY_DUMP = 0x22,
        TAG_PRIMITIVE_ARRAY_DUMP = 0x23,
        TAG_STRUCT_ARRAY_DUMP = 0x24
    };

    enum BasicType {
        OBJECT = 2,
        BOOLEAN = 4,
        CHAR = 5,
        FLOAT = 6,
        DOUBLE = 7,
        BYTE = 8,
        SHORT = 9,
        INT = 10,
        LONG = 11,
    };

    struct DumpObject {
        BaseObject* obj;
        u1 tag;
        u4 threadId;
        u4 frameNum;
        CodeHeapDataStringId classId;
    };

    struct DumpClass {
        TypeInfo* klass;
        CodeHeapDataStringId klassId;
    };

    std::list<DumpObject> dumpObjects;
    std::map<TypeInfo*, CodeHeapDataStringId> dumpClassMap;
    std::map<TypeInfo*, CodeHeapDataStringId> dumpStructClassMap;

    uint32_t kCodeHeapDataTime = 0;

    CString methodName;
    CString fileName;
    uint32_t lineNumber = 0;

    std::map<CString, CodeHeapDataStringId> strings;

    void DumpHeap();
    bool DumpHeap(int fd);
    void WriteHeap();
    void ProcessHeap();

    void WriteFixedHeader();
    void WriteString();
    void WriteAllClassLoad();
    void WriteAllStructClassLoad();
    void WriteStackFrame(FrameInfo& frame, uint32_t frameIdx);
    void WriteStackTrace();
    void WriteRecordHeader(const u1 tag, const u4 time);
    void WriteAllObjects();
    void WriteAllClass();
    void WriteAllStructClass();
    void WriteHeapDump();
    void WriteStartThread();
    void WriteGCtibFileds(GCTib tib, bool isObject, int fieldNum);
    void WriteBitmapWordFileds(GCTib tib, bool isObject, int fieldNum);
    void WriteGCTibType(GCTib tib);
    void ProcessStacktrace(RecordStackInfo* recordStackInfo);
    void AddU1(const u1 value); // add len = 1
    void AddU2(const u2 value); // add len = 1
    void AddU4(const u4 value); // add len = 1
    void AddU8(const u8 value); // add len = 1
    void AddID(const u8 value); // add len = 1

    void ModifyLength();

    void AddU1List(const u1* value, uint8_t count);
    void AddU2List(const u2* value, uint8_t count);
    void AddU4List(const u4* value, uint8_t count);
    void AddU8List(const u8* value, uint8_t count);

    void HandleAddU1(const u1* value, uint8_t count);
    void HandleAddU2(const u2* value, uint8_t count);
    void HandleAddU4(const u4* value, uint8_t count);
    void HandleAddU8(const u8* value, uint8_t count);

    void AddStringId(CodeHeapDataStringId value);

    void ProcessRootGlobal();
    void ProcessRootConcurrencyModel();
    void ProcessRootLocal();
    void ProcessRootThreadObject();
    void ProcessRootFinalizer();
    void ProcessHeapObject(BaseObject* roots);
    void ProcessRootClass(TypeInfo* klass);
    void ProcessStructClass(TypeInfo* klass);

    void WriteGlobalRoot(BaseObject*& obj, const u1 tag);
    void WriteUnknownRoot(BaseObject*& obj, const u1 tag);
    void WriteLocalRoot(BaseObject*& obj, const u1 tag, const u4 tid, const u1 depth);
    void WriteThreadObjectRoot(BaseObject*& obj, const u1 tag, const u4 tid, const u4 stackTraceIdx);
    void WriteObjectArray(BaseObject*& obj, const u1 tag);
    void WriteStructArray(BaseObject*& obj, const u1 tag);
    void WritePrimitiveArray(BaseObject*& obj, const u1 tag);
    void WriteInstance(BaseObject*& obj, const u1 tag);
    void WriteClass(TypeInfo* klass, CodeHeapDataStringId klassId, const u1 tag);
    void WriteStructClass(TypeInfo* klass, CodeHeapDataStringId klassId, const u1 tag);

    void WriteClassLoad(TypeInfo* klass, CodeHeapDataStringId klassId, const u1 tag);
    void GetFrameInfo(FrameInfo frame, const u1 tag);
    void EndRecord();

    std::vector<uint8_t> buffer; // buffer 8byte vector
    uint64_t length = 0;

    CodeHeapDataStringId LookupStringId(const CString& string);
    CodeHeapData::CodeHeapDataStringId stringId = 0x40000000;
    CodeHeapData::CodeHeapDataStringId threadObjectId = 0x80000000;
    std::set<TypeInfo*> roots;

    bool dumpAfterOOM = false;
    CString threadName;
    std::unordered_map<FrameInfo*, CodeHeapDataStackFrameId> frames;
    std::unordered_map<RecordStackInfo*, CodeHeapDataStackTraceSerialNumber> stacktraces;
    FILE* fp;
    CodeHeapDataStackFrameId frameId = 0;
    CodeHeapDataStackFrameId threadNameId = 0;
    u4 threadId = 0;
    CodeHeapDataStackTraceSerialNumber traceSerialNum = kCodeHeapDataNullStackTrace + 1;
};
} // namespace MapleRuntime
#endif
