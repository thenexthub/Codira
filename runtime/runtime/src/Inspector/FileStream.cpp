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


#include <cerrno>
#include <climits>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <ios>
#include <ostream>
#include <unistd.h>
#include "Base/LogFile.h"
#include "FileStream.h"

namespace MapleRuntime {
void HeapProfilerStream::SetHandler(SendMsgCB sendMsgInit)
{
    if (sendMsg != nullptr) {
        return;
    }
    sendMsg = sendMsgInit;
}
bool HeapProfilerStream::SetMessageID(const std::string &message)
{
    std::string key = "\"id\":";
    size_t startPos = message.find(key);
    if (startPos == std::string::npos) {
        return false;
    }
    startPos += key.length();

    while (startPos < message.length() && std::isspace(message[startPos])) {
        ++startPos;
    }

    if (message[startPos] == '"') {
        size_t endPos = message.find('"', startPos + 1);
        if (endPos == std::string::npos) {
            return false;
        }
        messageId = CString(message.substr(startPos + 1, endPos - startPos - 1).c_str());
    } else {
        size_t endPos = message.find(',', startPos);
        if (endPos == std::string::npos) {
            endPos = message.find('}', startPos);
        }
        if (endPos == std::string::npos) {
            return false;
        }
        messageId = CString(message.substr(startPos, endPos - startPos).c_str());
    }
        return true;
}

CString HeapProfilerStream::GetMessageID()
{
    return messageId;
}

void HeapProfilerStream::SetContext(MsgType type)
{
    if (type == DUMPHEAPSNAPSHOT || type == ALLOCATION) {
        start = "{\"method\":\"HeapProfiler.addHeapSnapshotChunk\",\"params\":{\"chunk\":\"";
        end = "\"},\"profiler\":\"cangjie profiler\"}";
        return;
    } else if (type == STATSUPDATE) {
        start = "{\"method\":\"HeapProfiler.heapStatsUpdate\",\"params\":{\"statsUpdate\":";
        end = "},\"profiler\":\"cangjie profiler\"}";
        return;
    } else {
        start = "";
    }
    end = ",\"profiler\":\"cangjie profiler\"}";
}
int HeapProfilerStream::GetSize()
{
    static const int heapProfilerChunkSise = static_cast<int>(20 * 1024);
    return heapProfilerChunkSise;
}
bool HeapProfilerStream::WriteChunk(char* data, int32_t size)
{
    std::string chunk(data, size);
    std::string msg = start + chunk + end;
    sendMsg(msg);
    return true;
}
}
