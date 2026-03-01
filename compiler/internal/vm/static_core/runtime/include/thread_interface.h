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
#ifndef PANDA_RUNTIME_THREAD_INTERFACE_H
#define PANDA_RUNTIME_THREAD_INTERFACE_H

#include "runtime/include/locks.h"
#include "runtime/include/thread_status.h"
#include "libarkbase/macros.h"

namespace ark {

class PandaVM;

class ThreadInterface {
public:
    explicit ThreadInterface(MutatorLock *mutatorLock) : mutatorLock_(mutatorLock) {}

    MutatorLock *GetMutatorLock()
    {
        return mutatorLock_;
    }

    const MutatorLock *GetMutatorLock() const
    {
        return mutatorLock_;
    }

#ifndef NDEBUG
    MutatorLock::MutatorLockState GetLockState() const
    {
        return lockState_;
    }

    void SetLockState(MutatorLock::MutatorLockState state)
    {
        lockState_ = state;
    }
#endif

    virtual void OnRuntimeTerminated() {}

    virtual void PrintSuspensionStackIfNeeded() {}

    virtual uint32_t GetId() const
    {
        return 0;
    }

private:
    MutatorLock *mutatorLock_;
#ifndef NDEBUG
    MutatorLock::MutatorLockState lockState_ = MutatorLock::UNLOCKED;
#endif
};

}  // namespace ark

#endif  // PANDA_RUNTIME_THREAD_INTERFACE_H
