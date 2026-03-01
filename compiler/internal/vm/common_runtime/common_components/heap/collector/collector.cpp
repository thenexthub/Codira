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
#include "common_components/heap/collector/collector.h"

#include "common_components/heap/heap.h"
#include "common_components/mutator/mutator.h"

namespace common {
namespace {
const char* const COLLECTOR_NAME[] = { "No Collector", "Proxy Collector", "Regional-Copying Collector",
                                       "Smooth Collector" };
}

const char* Collector::GetGCPhaseName(GCPhase phase)
{
    static const char* phaseNames[] = {
        "undefined phase",      // GC_PHASE_UNDEF
        "idle phase",           // GC_PHASE_IDLE
        "stub phase",           // reserved
        "stub phase",           // reserved
        "stub phase",           // reserved
        "stub phase",           // reserved
        "stub phase",           // reserved
        "stub phase",           // reserved
        "start phase",          // GC_PHASE_START
        "enum phase",           // GC_PHASE_ENUM
        "marking phase",          // GC_PHASE_MARK
        "remark-satb phase",    // GC_PHASE_REMARK_SATB
        "final-mark phase",     // GC_PHASE_FINAL_MARK
        "post-marking phase",     // GC_PHASE_POST_MARK
        "pre-copy phase",    // GC_PHASE_PRECOPY
        "copy phase",        // GC_PHASE_COPY
        "fix phase",            // GC_PHASE_FIX
    };
    return phaseNames[phase];
}

Collector::Collector() {}

const char* Collector::GetCollectorName() const { return COLLECTOR_NAME[collectorType_]; }

void Collector::RequestGC(GCReason reason, bool async, GCType gcType)
{
    RequestGCInternal(reason, async, gcType);
    return;
}
} // namespace common.
