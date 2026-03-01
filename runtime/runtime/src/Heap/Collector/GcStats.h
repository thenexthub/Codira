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


#ifndef MRT_STATS_H
#define MRT_STATS_H

#include <algorithm>
#include <atomic>
#include <list>
#include <memory>
#include <mutex>

#include "Base/ImmortalWrapper.h"
#include "Base/Panic.h"
#include "GcRequest.h"

namespace MapleRuntime {
// statistics for previous gc.
class GCStats {
public:
    GCStats() = default;
    ~GCStats() = default;

    void Init();

    size_t GetThreshold() const { return heapThreshold; }

    void Dump() const;

    static uint64_t GetPrevGCStartTime() { return prevGcStartTime; }

    static void SetPrevGCStartTime(uint64_t timestamp) { prevGcStartTime = timestamp; }

    static uint64_t GetPrevGCFinishTime() { return prevGcFinishTime; }

    static void SetPrevGCFinishTime(uint64_t timestamp) { prevGcFinishTime = timestamp; }

    static uint64_t prevGcStartTime;
    static uint64_t prevGcFinishTime;

    GCReason reason;
    bool isConcurrentMark;
    bool async;

    uint64_t gcStartTime;
    uint64_t gcEndTime;

    size_t liveBytesBeforeGC;
    size_t liveBytesAfterGC;

    size_t fromSpaceSize;
    size_t smallGarbageSize;

    size_t pinnedSpaceSize;
    size_t pinnedGarbageSize;

    size_t largeSpaceSize;
    size_t largeGarbageSize;

    size_t collectedBytes;
    size_t collectedObjects;

    double garbageRatio;
    double collectionRate; // bytes per nano-second

    size_t heapThreshold;
};
extern size_t g_gcCount;
extern uint64_t g_gcTotalTimeUs;
extern size_t g_gcCollectedTotalBytes;
} // namespace MapleRuntime
#endif // MRT_STATS_H
