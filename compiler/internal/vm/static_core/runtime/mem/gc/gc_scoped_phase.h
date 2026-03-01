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
#ifndef GC_SCOPED_PHASE_H
#define GC_SCOPED_PHASE_H

#include "libarkbase/macros.h"
#include "libarkbase/utils/logger.h"
#include "runtime/include/mem/panda_string.h"
#include "runtime/mem/gc/gc_phase.h"
#include "runtime/mem/mem_stats_additional_info.h"
#include "runtime/mem/mem_stats_default.h"

namespace ark::mem {

// forward declarations:
class GC;

class GCScopedPhase {
public:
    GCScopedPhase(GC *gc, GCPhase newPhase);
    NO_COPY_SEMANTIC(GCScopedPhase);
    NO_MOVE_SEMANTIC(GCScopedPhase);

    ~GCScopedPhase();

    static PandaString GetPhaseName(GCPhase phase)
    {
        switch (phase) {
            case GCPhase::GC_PHASE_IDLE:
                return "Idle";
            case GCPhase::GC_PHASE_RUNNING:
                return "RunPhases()";
            case GCPhase::GC_PHASE_COLLECT_ROOTS:
                return "CollectRoots()";
            case GCPhase::GC_PHASE_INITIAL_MARK:
                return "InitialMark";
            case GCPhase::GC_PHASE_MARK:
                return "MarkAll()";
            case GCPhase::GC_PHASE_MARK_YOUNG:
                return "Mark()";
            case GCPhase::GC_PHASE_REMARK:
                return "YoungRemark()";
            case GCPhase::GC_PHASE_COLLECT_YOUNG_AND_MOVE:
                return "CollectAndMove()";
            case GCPhase::GC_PHASE_SWEEP:
                return "Sweep()";
            case GCPhase::GC_PHASE_CLEANUP:
                return "Cleanup()";
            default:
                return "UnknownPhase";
        }
    }

    static const char *GetPhaseAbbr(GCPhase phase)
    {
        switch (phase) {
            case GCPhase::GC_PHASE_IDLE:
                return "Idle";
            case GCPhase::GC_PHASE_RUNNING:
                return "RunPhases";
            case GCPhase::GC_PHASE_COLLECT_ROOTS:
                return "ColRoots";
            case GCPhase::GC_PHASE_INITIAL_MARK:
                return "InitMark";
            case GCPhase::GC_PHASE_MARK:
                return "Mark";
            case GCPhase::GC_PHASE_MARK_YOUNG:
                return "MarkY";
            case GCPhase::GC_PHASE_REMARK:
                return "YRemark";
            case GCPhase::GC_PHASE_COLLECT_YOUNG_AND_MOVE:
                return "ColYAndMove";
            case GCPhase::GC_PHASE_SWEEP:
                return "Sweep";
            case GCPhase::GC_PHASE_CLEANUP:
                return "Cleanup";
            default:
                return "UnknownPhase";
        }
    }

private:
    GCPhase phase_;
    GCPhase oldPhase_;
    GC *gc_;

    PandaString GetGCName();
};

}  // namespace ark::mem

#endif  // GC_SCOPED_PHASE_H
