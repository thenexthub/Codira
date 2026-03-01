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
#ifndef COMMON_COMPONENTS_HEAP_COLLECTOR_GCREQUEST_H
#define COMMON_COMPONENTS_HEAP_COLLECTOR_GCREQUEST_H

#include <cstdint>
#include <limits>

#include "common_components/base/globals.h"
#include "common_interfaces/base_runtime.h"

namespace common {
// Minimum time between async GC (heuristic, native).
constexpr uint64_t MIN_ASYNC_GC_INTERVAL_NS = SECOND_TO_NANO_SECOND;
constexpr uint64_t LONG_MIN_HEU_GC_INTERVAL_NS = 200 * MILLI_SECOND_TO_NANO_SECOND;

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
extern GCRequest g_gcRequests[];
} // namespace common

#endif  // COMMON_COMPONENTS_HEAP_COLLECTOR_GC_DEBUGGER_H
