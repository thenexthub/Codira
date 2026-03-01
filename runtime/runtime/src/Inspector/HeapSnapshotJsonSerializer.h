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


#ifndef MRT_CODE_HEAP_DATA_IDE_H
#define MRT_CODE_HEAP_DATA_IDE_H
#include <fstream>
#include <climits>
#include <sstream>
#include "FileStream.h"
#include "Base/CString.h"
#include "Inspector/CodeHeapData.h"
#include "securec.h"

namespace MapleRuntime {
class CodeHeapDataForIDE : public CodeHeapData {
public:
    using CodeHeapDataIDForIDE = u4;
    bool Serialize();
    void SerializeFixedHeader();
    void SerializeString();
    void SerializeRecordHeader(const u1 tag, const u4 time);
    void SerializeAllClassLoad();
    void SerializeAllStructClassLoad();
    void SerializeAllClass();
    void SerializeAllStructClass();
    void SerializeStackTrace();
    void SerializeStartThread();
    void SerializeHeapDump();
    void SerializeStackFrame(FrameInfo& frame, uint32_t frameIdx);
    void SerializeClassLoad(TypeInfo* klass, CodeHeapDataStringId klassId, const u1 tag);
    void SerializeAllObjects();
    void SerializeGlobalRoot(BaseObject*& obj, const u1 tag);
    void SerializeUnknownRoot(BaseObject*& obj, const u1 tag);
    void SerializeLocalRoot(BaseObject*& obj, const u1 tag, const u4 tid, const u1 depth);
    void SerializeThreadObjectRoot(BaseObject*& obj, const u1 tag, const u4 tid, const u4 stackTraceIdx);
    void SerializeObjectArray(BaseObject*& obj, const u1 tag);
    void SerializeStructArray(BaseObject*& obj, const u1 tag);
    void SerializePrimitiveArray(BaseObject*& obj, const u1 tag);
    void SerializeInstance(BaseObject*& obj, const u1 tag);
    void SerializeClass(TypeInfo* klass, CodeHeapDataStringId klassId, const u1 tag);
    void SerializeStructClass(TypeInfo* klass, CodeHeapDataStringId klassId, const u1 tag);
    u4 GetId(CodeHeapDataStringId klassId);
    I8 GetObjType(BaseObject* obj);
private:
    StreamWriter* writer = nullptr;
    std::unordered_map<CodeHeapDataStringId, u4> stringIdxMap;
    u4 stringIdx = 0;
};
}
#endif
