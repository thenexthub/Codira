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
#include "runtime/mem/gc/gc_scoped_phase.h"

#include "runtime/mem/gc/gc.h"

namespace ark::mem {

GCScopedPhase::GCScopedPhase(GC *gc, GCPhase newPhase)
{
    ASSERT(gc != nullptr);
    gc_ = gc;
    gc_->BeginTracePoint(GetPhaseName(newPhase));
    phase_ = newPhase;
    oldPhase_ = gc_->GetGCPhase();
    gc_->SetGCPhase(phase_);
    LOG(DEBUG, GC) << "== " << GetGCName() << "::" << GetPhaseName(phase_) << " started ==";
    gc_->FireGCPhaseStarted(newPhase);
}

GCScopedPhase::~GCScopedPhase()
{
    gc_->FireGCPhaseFinished(phase_);
    gc_->SetGCPhase(oldPhase_);
    gc_->EndTracePoint();
    LOG(DEBUG, GC) << "== " << GetGCName() << "::" << GetPhaseName(phase_) << " finished ==";
}

PandaString GCScopedPhase::GetGCName()
{
    GCType type = gc_->GetType();
    switch (type) {
        case GCType::EPSILON_GC:
            return "EpsilonGC";
        case GCType::STW_GC:
            return "StwGC";
        default:
            return "GC";
    }
}

}  // namespace ark::mem
