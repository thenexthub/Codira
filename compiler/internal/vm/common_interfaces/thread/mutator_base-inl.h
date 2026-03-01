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

#ifndef COMMON_INTERFACES_THREAD_MUTATOR_BASE_INL_H
#define COMMON_INTERFACES_THREAD_MUTATOR_BASE_INL_H

#include "thread/mutator_base.h"

#include "base/common.h"

namespace common {
inline void MutatorBase::DoEnterSaferegion()
{
    // set current mutator in saferegion.
    SetInSaferegion(SAFE_REGION_TRUE);
}

inline bool MutatorBase::EnterSaferegion([[maybe_unused]] bool updateUnwindContext) noexcept
{
    if (LIKELY_CC(!InSaferegion())) {
        DoEnterSaferegion();
        return true;
    }
    return false;
}

inline bool MutatorBase::LeaveSaferegion() noexcept
{
    if (LIKELY_CC(InSaferegion())) {
        DoLeaveSaferegion();
        return true;
    }
    return false;
}

// Ensure that mutator is changed only once by mutator itself or Profile
bool MutatorBase::TransitionToCpuProfile(bool bySelf)
{
    do {
        CpuProfileState state = cpuProfileState_.load();
        // If this mutator profile has finished, just return
        if (state == FINISH_CPUPROFILE) {
            return true;
        }
        // If this mutator is executing profile by other thread, mutator should wait but profile just return
        if (state == IN_CPUPROFILING) {
            if (bySelf) {
                WaitForCpuProfiling();
                return true;
            } else {
                return false;
            }
        }
        if (!bySelf && state == NO_CPUPROFILE) {
            return true;
        }
        // Current thread set atomic variable to ensure atomicity of phase transition
        CHECK_CC(state == NEED_CPUPROFILE);
        if (cpuProfileState_.compare_exchange_weak(state, IN_CPUPROFILING)) {
            TransitionToCpuProfileExclusive();
            cpuProfileState_.store(FINISH_CPUPROFILE, std::memory_order_release);
            return true;
        }
    } while (true);
}
}  // namespace common
#endif  // COMMON_INTERFACES_THREAD_MUTATOR_BASE_INL_H