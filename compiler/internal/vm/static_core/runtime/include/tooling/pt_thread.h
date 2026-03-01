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
#ifndef PANDA_RUNTIME_INCLUDE_TOOLING_PT_THREAD_H
#define PANDA_RUNTIME_INCLUDE_TOOLING_PT_THREAD_H

#include <cstdint>
#include "libarkbase/macros.h"
#include "runtime/coroutines/coroutine.h"
#include "runtime/include/managed_thread.h"

namespace ark::tooling {
class PtThread {
public:
    explicit PtThread(ManagedThread *managedThread) : managedThread_(managedThread) {}

    // Deprecated API
    explicit PtThread(uint32_t /* unused */) {}

    bool operator==(const PtThread &other) const
    {
        return managedThread_ == other.managedThread_;
    }

    bool operator!=(const PtThread &other) const
    {
        return !(*this == other);
    }

    bool operator<(const PtThread &other) const
    {
        return managedThread_ < other.managedThread_;
    }

    uint32_t GetId() const
    {
        constexpr uint32_t PT_THREAD_NONE_ID = 0xffffffff;
        if (managedThread_ == nullptr) {
            return PT_THREAD_NONE_ID;
        }
        if (Coroutine::ThreadIsCoroutine(managedThread_)) {
            return Coroutine::CastFromThread(managedThread_)->GetCoroutineId();
        }
        return managedThread_->GetId();
    }

    ManagedThread *GetManagedThread() const
    {
        return managedThread_;
    }

    PANDA_PUBLIC_API static const PtThread NONE;

    ~PtThread() = default;

    DEFAULT_COPY_SEMANTIC(PtThread);
    DEFAULT_MOVE_SEMANTIC(PtThread);

private:
    ManagedThread *managedThread_ {nullptr};
};
}  // namespace ark::tooling

#endif  // PANDA_RUNTIME_INCLUDE_TOOLING_PT_THREAD_H
