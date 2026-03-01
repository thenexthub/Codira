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

#ifndef COMMON_COMPONENTS_COMMON_SCOPED_SAFEREGION_H
#define COMMON_COMPONENTS_COMMON_SCOPED_SAFEREGION_H

#include "common_components/mutator/mutator.inline.h"

namespace common {
// Scoped guard for saferegion.
class ScopedEnterSaferegion {
public:
    ScopedEnterSaferegion() = delete;
    explicit ScopedEnterSaferegion(bool onlyForMutator = false)
    {
        Mutator* mutator = Mutator::GetMutator();
        ThreadType threadType = ThreadLocal::GetThreadType();
        if (onlyForMutator &&
            (threadType == ThreadType::FP_THREAD || threadType == ThreadType::GC_THREAD)) {
            stateChanged = false;
        } else {
            stateChanged = (mutator != nullptr) ? mutator->EnterSaferegion(true) : false;
        }
    }

    ~ScopedEnterSaferegion()
    {
        if (LIKELY_CC(stateChanged)) {
            Mutator* mutator = Mutator::GetMutator(); // state changed, mutator must be not null
            (void)mutator->LeaveSaferegion();
        }
    }

private:
    bool stateChanged;
};

class ScopedObjectAccess {
public:
    ScopedObjectAccess() : mutator(*Mutator::GetMutator()), leavedSafeRegion(mutator.LeaveSaferegion()) {}

    ~ScopedObjectAccess()
    {
        if (LIKELY_CC(leavedSafeRegion)) {
            // qemu use c++ thread local, it has issue with some cases for ZRT annotation, if reload again
            // fail on O3 and pass on O0 if load mutator again, not figureout why yet
            (void)mutator.EnterSaferegion(false);
        }
    }

private:
    Mutator& mutator;
    bool leavedSafeRegion;
};
} // namespace common

#endif // COMMON_COMPONENTS_COMMON_SCOPED_SAFEREGION_H
