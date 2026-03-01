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
#include "common_components/heap/collector/gc_request.h"

#include "common_components/base/time_utils.h"
#include "common_components/heap/collector/gc_stats.h"
namespace common {
namespace {
// Set a safe initial value so that the first GC is able to trigger.
uint64_t g_initHeuTriggerTimestamp = TimeUtil::NanoSeconds() - LONG_MIN_HEU_GC_INTERVAL_NS;
uint64_t g_initNativeTriggerTimestamp = TimeUtil::NanoSeconds() - MIN_ASYNC_GC_INTERVAL_NS;
} // namespace

inline bool GCRequest::IsFrequentGC() const
{
    if (minIntervelNs == 0) {
        return false;
    }
    int64_t now = static_cast<int64_t>(TimeUtil::NanoSeconds());
    return (now - prevRequestTime < minIntervelNs);
}

inline bool GCRequest::IsFrequentAsyncGC() const
{
    uint64_t now = TimeUtil::NanoSeconds();
    return (now - GCStats::GetPrevGCFinishTime()) < minIntervelNs;
}

// heuristic gc is triggered by object allocation,
// the heap stats should take into consideration.
inline bool GCRequest::IsFrequentHeuristicGC() const { return IsFrequentAsyncGC(); }

bool GCRequest::ShouldBeIgnored() const
{
    switch (reason) {
        case GC_REASON_HEU:
        case GC_REASON_YOUNG:
        case GC_REASON_BACKGROUND:
        case GC_REASON_HINT:
            return IsFrequentHeuristicGC();
        case GC_REASON_NATIVE:
            return IsFrequentAsyncGC();
        case GC_REASON_OOM:
        case GC_REASON_FORCE:
            return IsFrequentGC();
        default:
            return false;
    }
}

GCRequest g_gcRequests[] = {
    { GC_REASON_USER, "user", false, true, 0, 0 },
    { GC_REASON_OOM, "oom", true, false, 0, 0 },
    { GC_REASON_BACKUP, "backup", true, false, 0, 0 },
    { GC_REASON_HEU, "heuristic", false, true, LONG_MIN_HEU_GC_INTERVAL_NS, g_initHeuTriggerTimestamp },
    { GC_REASON_YOUNG, "young", false, true, LONG_MIN_HEU_GC_INTERVAL_NS, g_initHeuTriggerTimestamp },
    { GC_REASON_NATIVE, "native_alloc", false, true, MIN_ASYNC_GC_INTERVAL_NS, g_initNativeTriggerTimestamp },
    { GC_REASON_HEU_SYNC, "heuristic_sync", true, true, 0, 0 },
    { GC_REASON_NATIVE_SYNC, "native_alloc_sync", true, true, 0, 0 },
    { GC_REASON_FORCE, "force", true, false, 0, 0 },
    { GC_REASON_APPSPAWN, "appspawn", true, false, 0, 0 },
    { GC_REASON_XREF, "force_xref", true, false, 0, 0 },
    { GC_REASON_BACKGROUND, "backgound", false, true, LONG_MIN_HEU_GC_INTERVAL_NS, g_initHeuTriggerTimestamp },
    { GC_REASON_HINT, "hint", false, true, LONG_MIN_HEU_GC_INTERVAL_NS, g_initHeuTriggerTimestamp },
    { GC_REASON_IDLE, "idle", false, true, LONG_MIN_HEU_GC_INTERVAL_NS, g_initHeuTriggerTimestamp }
};
} // namespace common
