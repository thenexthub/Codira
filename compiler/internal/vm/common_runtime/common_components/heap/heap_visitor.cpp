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

#include "common_interfaces/heap/heap_visitor.h"

#include "common_components/common_runtime/hooks.h"
#include "common_components/mutator/mutator.h"
namespace common {
UnmarkAllXRefsHookFunc g_unmarkAllXRefsHook = nullptr;
SweepUnmarkedXRefsHookFunc g_sweepUnmarkedXRefsHook = nullptr;
AddXRefToStaticRootsHookFunc g_addXRefToStaticRootsHook = nullptr;
RemoveXRefFromStaticRootsHookFunc g_removeXRefFromStaticRootsHook = nullptr;

VisitStaticRootsHookFunc g_visitStaticRootsHook = nullptr;
UpdateStaticRootsHookFunc g_updateStaticRootsHook = nullptr;
SweepStaticRootsHookFunc g_sweepStaticRootsHook = nullptr;

void RegisterVisitStaticRootsHook(VisitStaticRootsHookFunc func)
{
    g_visitStaticRootsHook = func;
}

void RegisterUpdateStaticRootsHook(UpdateStaticRootsHookFunc func)
{
    g_updateStaticRootsHook = func;
}

void RegisterSweepStaticRootsHook(SweepStaticRootsHookFunc func)
{
    g_sweepStaticRootsHook = func;
}


void VisitRoots(const RefFieldVisitor &visitor)
{
    VisitDynamicGlobalRoots(visitor);
    VisitDynamicLocalRoots(visitor);
    VisitDynamicConcurrentRoots(visitor);
    VisitBaseRoots(visitor);
}

void VisitSTWRoots(const RefFieldVisitor &visitor)
{
    VisitDynamicGlobalRoots(visitor);
    VisitDynamicLocalRoots(visitor);
    VisitBaseRoots(visitor);
}

void VisitConcurrentRoots(const RefFieldVisitor &visitor)
{
    VisitDynamicConcurrentRoots(visitor);
}

void VisitWeakRoots(const WeakRefFieldVisitor &visitor)
{
    VisitDynamicWeakGlobalRoots(visitor);
    VisitDynamicWeakGlobalRootsOld(visitor);
    VisitDynamicWeakLocalRoots(visitor);
}

void VisitGlobalRoots(const RefFieldVisitor &visitor)
{
    VisitDynamicGlobalRoots(visitor);
    VisitBaseRoots(visitor);
}

void VisitWeakGlobalRoots(const WeakRefFieldVisitor &visitor)
{
    VisitDynamicWeakGlobalRoots(visitor);
    VisitDynamicWeakGlobalRootsOld(visitor);
}

void VisitPreforwardRoots(const RefFieldVisitor &visitor)
{
    VisitDynamicPreforwardRoots(visitor);
}

// Visit specific mutator's root.
void VisitMutatorRoot(const RefFieldVisitor &visitor, Mutator &mutator)
{
    if (mutator.GetEcmaVMPtr()) {
        VisitDynamicThreadRoot(visitor, mutator.GetEcmaVMPtr());
    }
}

void VisitWeakMutatorRoot(const WeakRefFieldVisitor &visitor, Mutator &mutator)
{
    if (mutator.GetEcmaVMPtr()) {
        VisitDynamicWeakThreadRoot(visitor, mutator.GetEcmaVMPtr());
    }
}

void VisitMutatorPreforwardRoot(const RefFieldVisitor &visitor, Mutator &mutator)
{
    if (mutator.GetEcmaVMPtr()) {
        VisitDynamicThreadPreforwardRoot(visitor, mutator.GetEcmaVMPtr());
    }
}

void RegisterUnmarkAllXRefsHook(UnmarkAllXRefsHookFunc func)
{
    g_unmarkAllXRefsHook = func;
}

void RegisterSweepUnmarkedXRefsHook(SweepUnmarkedXRefsHookFunc func)
{
    g_sweepUnmarkedXRefsHook = func;
}

void RegisterAddXRefToStaticRootsHook(AddXRefToStaticRootsHookFunc func)
{
    g_addXRefToStaticRootsHook = func;
}

void RegisterRemoveXRefFromStaticRootsHook(RemoveXRefFromStaticRootsHookFunc func)
{
    g_removeXRefFromStaticRootsHook = func;
}

void UnmarkAllXRefs()
{
    g_unmarkAllXRefsHook();
}

void SweepUnmarkedXRefs()
{
    g_sweepUnmarkedXRefsHook();
}

void AddXRefToRoots()
{
    AddXRefToDynamicRoots();
    g_addXRefToStaticRootsHook();
}

void RemoveXRefFromRoots()
{
    RemoveXRefFromDynamicRoots();
    g_removeXRefFromStaticRootsHook();
}
}  // namespace common
