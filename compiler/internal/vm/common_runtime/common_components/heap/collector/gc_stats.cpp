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
#include "common_components/heap/collector/gc_stats.h"

#include "common_interfaces/base_runtime.h"
#include "common_components/base/time_utils.h"
#include "common_components/heap/heap.h"
#include "common_components/log/log.h"

namespace common {
size_t g_gcCount = 0;
uint64_t g_gcTotalTimeUs = 0;
size_t g_gcCollectedTotalBytes = 0;

size_t g_fullGCCount = 0;
double g_fullGCMeanRate = 0.0;

uint64_t GCStats::prevGcStartTime = TimeUtil::NanoSeconds() - LONG_MIN_HEU_GC_INTERVAL_NS;
uint64_t GCStats::prevGcFinishTime = TimeUtil::NanoSeconds() - LONG_MIN_HEU_GC_INTERVAL_NS;

void GCStats::Init()
{
    isConcurrentMark = false;
    async = false;
    gcStartTime = TimeUtil::NanoSeconds();
    gcEndTime = TimeUtil::NanoSeconds();

    totalSTWTime = 0;
    maxSTWTime = 0;

    collectedObjects = 0;
    collectedBytes = 0;

    fromSpaceSize = 0;
    smallGarbageSize = 0;

    nonMovableSpaceSize = 0;
    nonMovableGarbageSize = 0;

    largeSpaceSize = 0;
    largeGarbageSize = 0;

    liveBytesBeforeGC = 0;
    liveBytesAfterGC = 0;

    garbageRatio = 0.0;
    collectionRate = 0.0;

    // 20 MB:set 20 MB as intial value
    heapThreshold = std::min(BaseRuntime::GetInstance()->GetGCParam().gcThreshold, 20 * MB);
    // 0.2:set 20% heap size as intial value
    heapThreshold = std::min(static_cast<size_t>(Heap::GetHeap().GetMaxCapacity() * 0.2), heapThreshold);

    targetFootprint = heapThreshold;
    shouldRequestYoung = false;
}

void GCStats::Dump() const
{
    // Print a summary of the last GC.
    size_t liveSize = Heap::GetHeap().GetAllocatedSize();
    size_t heapSize = Heap::GetHeap().GetUsedPageSize();
    double utilization = (heapSize == 0) ? 0 : (static_cast<double>(liveSize) / heapSize);
    // Do not change this GC log format.
    // Output one line statistic info after each gc task,
    // include the gc type, collected objects and current heap utilization, etc.
    // display to std-output. take care to modify.
    std::string maxSTWTime = PrettyOrderMathNano(MaxSTWTime(), "s");
    std::string totalSTWTime = PrettyOrderMathNano(TotalSTWTime(), "s");
    std::string totalGCTime = PrettyOrderMathNano(gcEndTime - gcStartTime, "s");
    std::ostringstream oss;
    oss <<
        "GC for " << g_gcRequests[reason].name << ": " << (async ? "async:" : "sync: ") <<
        "gcType: " << GCTypeToString(gcType) << ", collected bytes: " <<
        collectedBytes << "->" << PrettyOrderInfo(collectedBytes, "B") << ", " <<
        "->" << PrettyOrderInfo(liveSize, "B") << "/" << heapSize << "->" <<
        PrettyOrderInfo(heapSize, "B") << ", max pause: " << MaxSTWTime() <<
        "->" << maxSTWTime << ", total pause: " << TotalSTWTime() << "->" << totalSTWTime <<
        ", total GC time: " << (gcEndTime - gcStartTime) << "->" << totalGCTime;
    VLOG(INFO, oss.str().c_str());
    VLOG(DEBUG, "allocated size: %s, heap size: %s, heap utilization: %.2f%%", Pretty(liveSize).c_str(),
         Pretty(heapSize).c_str(), utilization * 100); // 100 for percentage.

    OHOS_HITRACE(HITRACE_LEVEL_COMMERCIAL, "CMCGC::GCStatsDump", (
                    "collectedObjects:" + std::to_string(collectedObjects) +
                    ";collectedBytes:" + std::to_string(collectedBytes) +
                    ";liveSize:" + std::to_string(liveSize) +
                    ";heapSize:" + std::to_string(heapSize) +
                    ";max pause:" + maxSTWTime +
                    ";total pause:" + totalSTWTime +
                    ";total GC time:" + totalGCTime
                ).c_str());
}
} // namespace common
