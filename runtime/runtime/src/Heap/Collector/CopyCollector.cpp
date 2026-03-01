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


#include "CopyCollector.h"

#include "Allocator/RegionSpace.h"
#include "Common/Runtime.h"
#include "Mutator/MutatorManager.h"
#include "Mutator/SatbBuffer.h"
#include "ObjectModel/RefField.inline.h"
#include "schedule.h"
#if defined(CODIRA_TSAN_SUPPORT)
#include "Sanitizer/SanitizerInterface.h"
#endif

namespace MapleRuntime {
void CopyCollector::PostGarbageCollection(uint64_t gcIndex)
{
    TracingCollector::PostGarbageCollection(gcIndex);
    MutatorManager::Instance().DestroyExpiredMutators();
}

void CopyCollector::CopyObject(const BaseObject& fromObj, BaseObject& toObj, size_t size) const
{
    uintptr_t from = reinterpret_cast<uintptr_t>(&fromObj);
    uintptr_t to = reinterpret_cast<uintptr_t>(&toObj);
    CHECK_E(memmove_s(reinterpret_cast<void*>(to), size, reinterpret_cast<void*>(from), size) != EOK, "memmove_s fail");
#if defined(CODIRA_TSAN_SUPPORT)
    Sanitizer::TsanFixShadow(reinterpret_cast<void*>(from), reinterpret_cast<void*>(to), size);
#endif
}

void CopyCollector::RunGarbageCollection(uint64_t gcIndex, GCReason reason)
{
    ScopedEntryTrace trace("CODERT_GC_START");
    // prevent other threads stop-the-world during GC.
    // this may be removed in the future.
    ScopedSTWLock stwLock;
    // ScopedStopTheWorld stw;

    gcReason = reason;
    PreGarbageCollection(true);
    ScheduleTraceEvent(TRACE_EV_GC_START, -1, nullptr, 0);
    VLOG(REPORT, "[GC] Start %s %s gcIndex= %lu", GetCollectorName(), g_gcRequests[gcReason].name, gcIndex);
    GCStats& gcStats = GetGCStats();
    gcStats.collectedBytes = 0;
    gcStats.gcStartTime = TimeUtil::NanoSeconds();

    DoGarbageCollection();

    if (reason == GC_REASON_OOM) {
        Heap::GetHeap().GetAllocator().ReclaimGarbageMemory(true);
    }

    PostGarbageCollection(gcIndex);
    gcStats.gcEndTime = TimeUtil::NanoSeconds();
    UpdateGCStats();
    uint64_t gcTimeNs = gcStats.gcEndTime - gcStats.gcStartTime;
    ScheduleTraceEvent(TRACE_EV_GC_DONE, -1, nullptr, 0);
    double rate = (static_cast<double>(gcStats.collectedBytes) / gcTimeNs) * (static_cast<double>(NS_PER_S) / MB);
    VLOG(REPORT, "total gc time: %s us, collection rate %.3lf MB/s\n", Pretty(gcTimeNs / NS_PER_US).Str(), rate);
    g_gcCount++;
    g_gcTotalTimeUs += (gcTimeNs / NS_PER_US);
    g_gcCollectedTotalBytes += gcStats.collectedBytes;
    gcStats.collectionRate = rate;
}

void CopyCollector::ForwardFromSpace()
{
    ScopedEntryTrace trace("CODERT_GC_FORWARD");
    TransitionToGCPhase(GCPhase::GC_PHASE_FORWARD, true);

    RegionSpace& space = reinterpret_cast<RegionSpace&>(theAllocator);
    GCStats& stats = GetGCStats();
    stats.liveBytesBeforeGC = space.AllocatedBytes();
    stats.fromSpaceSize = space.FromSpaceSize();
    space.ForwardFromSpace(GetThreadPool());
}

void CopyCollector::RefineFromSpace()
{
    GCStats& stats = GetGCStats();
    RegionSpace& space = reinterpret_cast<RegionSpace&>(theAllocator);
    stats.smallGarbageSize = space.RefineFromSpace();
}
} // namespace MapleRuntime
