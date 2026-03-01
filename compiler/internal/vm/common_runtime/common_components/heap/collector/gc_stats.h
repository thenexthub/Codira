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

#ifndef COMMON_COMPONENTS_HEAP_COLLECTOR_STATS_H
#define COMMON_COMPONENTS_HEAP_COLLECTOR_STATS_H

#include <algorithm>
#include <atomic>
#include <list>
#include <memory>
#include <mutex>

#include "common_components/base/immortal_wrapper.h"
#include "common_components/heap/collector/gc_request.h"
#include "common_components/log/log.h"
#include "common_interfaces/base_runtime.h"

namespace common {
// statistics for previous gc.
class GCStats {
public:
    GCStats() = default;
    ~GCStats() = default;

    void Init();

    size_t GetThreshold() const { return heapThreshold; }

    inline uint64_t TotalSTWTime() const { return totalSTWTime; }

    inline uint64_t MaxSTWTime() const { return maxSTWTime; }

    void recordSTWTime(uint64_t time)
    {
        totalSTWTime += time;
        maxSTWTime = std::max(maxSTWTime, time);
    }

    void Dump() const;

    static uint64_t GetPrevGCStartTime() { return prevGcStartTime; }

    static void SetPrevGCStartTime(uint64_t timestamp) { prevGcStartTime = timestamp; }

    static uint64_t GetPrevGCFinishTime() { return prevGcFinishTime; }

    static void SetPrevGCFinishTime(uint64_t timestamp) { prevGcFinishTime = timestamp; }

    size_t GetAccumulatedFreeSize() const { return accumulatedFreeSize; }

    void IncreaseAccumulatedFreeSize(size_t size) { accumulatedFreeSize += size; }

    static uint64_t prevGcStartTime;
    static uint64_t prevGcFinishTime;

    GCReason reason;
    GCType gcType;
    bool isConcurrentMark;
    bool async;

    uint64_t gcStartTime;
    uint64_t gcEndTime;

    uint64_t totalSTWTime; // total stw time(microseconds)
    uint64_t maxSTWTime; // max stw time(microseconds)

    size_t liveBytesBeforeGC;
    size_t liveBytesAfterGC;

    size_t fromSpaceSize;
    size_t smallGarbageSize;

    size_t nonMovableSpaceSize;
    size_t nonMovableGarbageSize;

    size_t largeSpaceSize;
    size_t largeGarbageSize;

    size_t collectedBytes;
    size_t collectedObjects;

    size_t accumulatedFreeSize;

    double garbageRatio;
    double collectionRate; // bytes per nano-second

    size_t heapThreshold;
    size_t targetFootprint;

    // Use for heuristic request, set by last gc status.
    bool shouldRequestYoung;

    bool isYoungGC()
    {
        return reason == GCReason::GC_REASON_YOUNG;
    }
};
extern size_t g_gcCount;
extern uint64_t g_gcTotalTimeUs;
extern size_t g_gcCollectedTotalBytes;

extern size_t g_fullGCCount;
extern double g_fullGCMeanRate;
} // namespace common

#endif  // COMMON_COMPONENTS_HEAP_COLLECTOR_STATS_H
