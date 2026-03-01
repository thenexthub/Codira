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


#ifndef MRT_GCREQUEST_H
#define MRT_GCREQUEST_H

#include <cstdint>
#include <limits>

#include "Base/Globals.h"

namespace MapleRuntime {
// Minimum time between async GC (heuristic, native).
constexpr uint64_t MIN_ASYNC_GC_INTERVAL_NS = MapleRuntime::SECOND_TO_NANO_SECOND;
constexpr uint64_t LONG_MIN_HEU_GC_INTERVAL_NS = 200 * MapleRuntime::MILLI_SECOND_TO_NANO_SECOND;

// Used by Collector::RequestGC.
// It tells why GC is triggered.
//
// sync: Caller of Collector::RequestGC will wait until GC completes.
// async: Collector::RequestGC returns immediately and caller continues to run.
enum GCReason : uint32_t {
    GC_REASON_USER = 0,    // Triggered by user explicitly.
    GC_REASON_OOM,         // Out of memory. Failed to allocate object.
    GC_REASON_BACKUP,      // backup gc is triggered if no other reason triggers gc for a long time.
    GC_REASON_HEU,         // Statistics show it is worth doing GC. Does not have to be immediate.
    GC_REASON_NATIVE,      // Native-Allocation-Registry shows it's worth doing GC.
    GC_REASON_HEU_SYNC,    // Just wait one gc request to reduce heap fragmentation.
    GC_REASON_NATIVE_SYNC, // Just wait one gc request to reduce native heap consumption.
    GC_REASON_FORCE,       // force gc is triggered when runtime triggers gc actively.
    GC_REASON_MAX,
    GC_REASON_INVALID = std::numeric_limits<uint32_t>::max(),
};

struct GCRequest {
    const GCReason reason;
    const char* name; // Human-readable names of GC reasons.

    const bool isSync;
    const bool isConcurrent;

    uint64_t minIntervelNs;
    uint64_t prevRequestTime;

    inline bool IsFrequentGC() const;
    inline bool IsFrequentAsyncGC() const;
    inline bool IsFrequentHeuristicGC() const;
    bool ShouldBeIgnored() const;

    bool IsSyncGC() const { return isSync; }

    void SetMinInterval(const uint64_t intervalNs) { minIntervelNs = intervalNs; }
    void SetPrevRequestTime(uint64_t timestamp) { prevRequestTime = timestamp; }
};

// Defined in gcRequest.cpp
extern GCRequest g_gcRequests[GC_REASON_MAX];
} // namespace MapleRuntime
#endif // MRT_GCREQUEST_H
