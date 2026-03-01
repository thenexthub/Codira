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


#include "TaskQueue.h"

#include "CollectorProxy.h"
#ifdef COV_SIGNALHANDLE
extern "C" void __gcov_flush(void);
#endif
namespace MapleRuntime {

bool GCExecutor::Execute(void* owner)
{
    MRT_ASSERT(owner != nullptr, "task queue owner ptr should not be null!");
    CollectorProxy* collectorProxy = reinterpret_cast<CollectorProxy*>(owner);

    switch (taskType) {
        case GCTask::TaskType::TASK_TYPE_TERMINATE_GC: {
            return false;
        }
        case GCTask::TaskType::TASK_TYPE_TIMEOUT_GC: {
            uint64_t curTime = TimeUtil::NanoSeconds();
            if ((curTime - GCStats::GetPrevGCStartTime()) > CodiraRuntime::GetGCParam().backupGCInterval) {
                GCStats::SetPrevGCStartTime(curTime);
                collectorProxy->RunGarbageCollection(GCTask::ASYNC_TASK_INDEX, GC_REASON_BACKUP);
                GCStats::SetPrevGCFinishTime(TimeUtil::NanoSeconds());
            }
            break;
        }
        case GCTask::TaskType::TASK_TYPE_INVOKE_GC: {
            GCStats::SetPrevGCStartTime(TimeUtil::NanoSeconds());
            collectorProxy->RunGarbageCollection(taskIndex, gcReason);
            GCStats::SetPrevGCFinishTime(TimeUtil::NanoSeconds());
            break;
        }
        case GCTask::TaskType::TASK_TYPE_DUMP_HEAP: {
            CodeHeapData* codeHeapData = new CodeHeapData();
            if (codeHeapData != nullptr) {
                codeHeapData->DumpHeap();
                delete codeHeapData;
            } else {
                LOG(RTLOG_ERROR, "codeHeapData Init Failed");
            }
#ifdef COV_SIGNALHANDLE
            __gcov_flush();
#endif
            break;
        }
        case GCTask::TaskType::TASK_TYPE_DUMP_HEAP_IDE: {
#if defined(__OHOS__) && (__OHOS__ == 1)
            CodeHeapDataForIDE* heapSnapshotJSONSerializer = new CodeHeapDataForIDE();
            if (heapSnapshotJSONSerializer != nullptr) {
                heapSnapshotJSONSerializer->Serialize();
                delete heapSnapshotJSONSerializer;
            } else {
                LOG(RTLOG_ERROR, "heapSnapshotJSONSerializer Init Failed");
            }
            break;
#endif
        }

        case GCTask::TaskType::TASK_TYPE_DUMP_HEAP_OOM: {
            CodeHeapData* codeHeapData = new CodeHeapData(true);
            if (codeHeapData != nullptr) {
                codeHeapData->DumpHeap();
                delete codeHeapData;
            } else {
                LOG(RTLOG_ERROR, "codeHeapData Init Failed");
            }
#ifdef COV_SIGNALHANDLE
            __gcov_flush();
#endif
            break;
        }
        default:
            LOG(RTLOG_ERROR, "[GC] Error task type: %u ignored!", static_cast<uint32_t>(taskType));
            break;
    }
    return true;
}
} // namespace MapleRuntime
