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

#ifndef COMMON_COMPONENTS_MUTATOR_MUTATOR_MANAGER_INL_H
#define COMMON_COMPONENTS_MUTATOR_MUTATOR_MANAGER_INL_H

#include "common_components/mutator/mutator_manager.h"

namespace common {
template<class STWFunction>
void MutatorManager::FlipMutators(STWParam& param, STWFunction&& stwFunction, FlipFunction *flipFunction)
{
    std::list<Mutator*> undoneMutators;
    {
        OHOS_HITRACE(HITRACE_LEVEL_COMMERCIAL, "Waiting-STW", "");
        ScopedStopTheWorld stw(param);
        OHOS_HITRACE(HITRACE_LEVEL_COMMERCIAL, param.stwReason, "");

        stwFunction();
        bool ignoreFinalizer = true;
        // Hope process ui thread's flipFunction at last, so add the ui mutator at the end of undoeMuators list.
        VisitAllMutators([&undoneMutators, flipFunction](Mutator& mutator) {
                mutator.SetFlipFunction(flipFunction);
                mutator.SetSuspensionFlag(MutatorBase::SuspensionType::SUSPENSION_FOR_PENDING_CALLBACK);
                undoneMutators.push_front(&mutator);
            }, ignoreFinalizer);
    }
    // Resume all mutator threads.
    AcquireMutatorManagementWLock();
    for (auto it = undoneMutators.begin(); it != undoneMutators.end();) {
        bool hasDead = std::find(allMutatorList_.begin(), allMutatorList_.end(), *it) == allMutatorList_.end();
        if (UNLIKELY_CC(hasDead)) {
            it = undoneMutators.erase(it);
            continue;
        }
        Mutator* mutator = *it;
        if (mutator->TryRunFlipFunction()) {
            it = undoneMutators.erase(it);
            continue;
        }
        it++;
    }
    // Wait for flipFunction running finish.
    for (auto it = undoneMutators.begin(); it != undoneMutators.end(); it++) {
        Mutator* mutator = *it;
        mutator->WaitFlipFunctionFinish();
    }
    MutatorManagementWUnlock();
}
} // namespace common

#endif  // COMMON_COMPONENTS_MUTATOR_MUTATOR_MANAGER_INL_H
