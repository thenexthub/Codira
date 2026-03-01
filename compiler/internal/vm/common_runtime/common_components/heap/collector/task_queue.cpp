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

#include "common_components/heap/collector/task_queue.h"

#include "common_components/heap/collector/collector_proxy.h"

namespace common {
bool GCRunner::Execute(void* owner)
{
    ASSERT_LOGF(owner != nullptr, "task queue owner ptr should not be null!");
    CollectorProxy* collectorProxy = reinterpret_cast<CollectorProxy*>(owner);

    switch (taskType_) {
        case GCTask::GCTaskType::GC_TASK_TERMINATE_GC: {
            return false;
        }
        case GCTask::GCTaskType::GC_TASK_INVOKE_GC: {
            GCStats::SetPrevGCStartTime(TimeUtil::NanoSeconds());
            collectorProxy->RunGarbageCollection(taskIndex_, gcReason_, gcType_);
            GCStats::SetPrevGCFinishTime(TimeUtil::NanoSeconds());
            break;
        }
        case GCTask::GCTaskType::GC_TASK_DUMP_HEAP: { //LCOV_EXCL_BR_LINE
            LOG_COMMON(FATAL) << "Don't know how to dump heap";
            UNREACHABLE_CC();
            break;
        }
        case GCTask::GCTaskType::GC_TASK_DUMP_HEAP_IDE: { //LCOV_EXCL_BR_LINE
            LOG_COMMON(FATAL) << "Don't know how to dump heap OOM";
            UNREACHABLE_CC();
            break;
        }

        case GCTask::GCTaskType::GC_TASK_DUMP_HEAP_OOM: { //LCOV_EXCL_BR_LINE
            LOG_COMMON(FATAL) << "Don't know how to dump heap OOM";
            UNREACHABLE_CC();
            break;
        }
        default:
            LOG_COMMON(ERROR) << "[GC] Error task type: " << static_cast<uint32_t>(taskType_) << " ignored!";
            break;
    }
    return true;
}
} // namespace common
