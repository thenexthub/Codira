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

#include "runtime/mem/gc/epsilon/epsilon.h"

#include "libarkbase/utils/logger.h"
#include "runtime/include/runtime.h"
#include "runtime/include/panda_vm.h"

namespace ark::mem {
template <class LanguageConfig>
EpsilonGC<LanguageConfig>::EpsilonGC(ObjectAllocatorBase *objectAllocator, const GCSettings &settings)
    : GCLang<LanguageConfig>(objectAllocator, settings)
{
    this->SetType(GCType::EPSILON_GC);
}

template <class LanguageConfig>
void EpsilonGC<LanguageConfig>::InitializeImpl()
{
    InternalAllocatorPtr allocator = this->GetInternalAllocator();
    auto barrierSet = allocator->New<GCDummyBarrierSet>(allocator);
    ASSERT(barrierSet != nullptr);
    this->SetGCBarrierSet(barrierSet);
    LOG(DEBUG, GC) << "Epsilon GC initialized...";
}

template <class LanguageConfig>
void EpsilonGC<LanguageConfig>::RunPhasesImpl([[maybe_unused]] GCTask &task)
{
    LOG(DEBUG, GC) << "Epsilon GC RunPhases...";
    GCScopedPauseStats scopedPauseStats(this->GetPandaVm()->GetGCStats());
}

// NOLINTNEXTLINE(misc-unused-parameters)
template <class LanguageConfig>
bool EpsilonGC<LanguageConfig>::WaitForGC([[maybe_unused]] GCTask task)
{
    return false;
}

template <class LanguageConfig>
void EpsilonGC<LanguageConfig>::InitGCBits([[maybe_unused]] ark::ObjectHeader *objHeader)
{
}

template <class LanguageConfig>
void EpsilonGC<LanguageConfig>::InitGCBitsForAllocationInTLAB([[maybe_unused]] ark::ObjectHeader *objHeader)
{
    LOG(FATAL, GC) << "TLABs are not supported by this GC";
}

template <class LanguageConfig>
bool EpsilonGC<LanguageConfig>::Trigger([[maybe_unused]] PandaUniquePtr<GCTask> task)
{
    return false;
}

template <class LanguageConfig>
bool EpsilonGC<LanguageConfig>::IsPostponeGCSupported() const
{
    return true;
}

template <class LanguageConfig>
void EpsilonGC<LanguageConfig>::MarkObject(ObjectHeader *object)
{
    object->SetMarkedForGC<true>();
}

template <class LanguageConfig>
bool EpsilonGC<LanguageConfig>::IsMarked(const ObjectHeader *object) const
{
    return object->IsMarkedForGC<true>();
}

template <class LanguageConfig>
void EpsilonGC<LanguageConfig>::MarkReferences([[maybe_unused]] GCMarkingStackType *references,
                                               [[maybe_unused]] GCPhase gcPhase)
{
}

TEMPLATE_CLASS_LANGUAGE_CONFIG(EpsilonGC);

}  // namespace ark::mem
