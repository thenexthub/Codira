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

#ifndef COMMON_COMPONENTS_BASE_ATOMIC_SPINLOCK_H
#define COMMON_COMPONENTS_BASE_ATOMIC_SPINLOCK_H

#include <atomic>

#include "common_interfaces/base/common.h"

namespace common {
class AtomicSpinLock {
public:
    AtomicSpinLock() {}
    ~AtomicSpinLock() = default;

    void Lock()
    {
        while (state_.test_and_set(std::memory_order_acquire)) {}
    }

    void Unlock() { state_.clear(std::memory_order_release); }

    bool TryLock() { return (state_.test_and_set(std::memory_order_acquire) == false); }

private:
    std::atomic_flag state_ = ATOMIC_FLAG_INIT;

    NO_COPY_SEMANTIC_CC(AtomicSpinLock);
};
} // namespace common

#endif // COMMON_COMPONENTS_BASE_ATOMIC_SPINLOCK_H
