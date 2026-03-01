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

#ifndef COMMON_COMPONENTS_BASE_SPINLOCK_H
#define COMMON_COMPONENTS_BASE_SPINLOCK_H

#include <pthread.h>

#include "common_interfaces/base/common.h"

namespace common {
class SpinLock {
public:
    // Create a Mutex that is not held by anybody.
    SpinLock() { pthread_spin_init(&spinlock_, 0); }

    ~SpinLock() { pthread_spin_destroy(&spinlock_); }

    void Lock() { pthread_spin_lock(&spinlock_); }

    void Unlock() { pthread_spin_unlock(&spinlock_); }

    bool TryLock() { return pthread_spin_trylock(&spinlock_) == 0; }

private:
    pthread_spinlock_t spinlock_;
    NO_COPY_SEMANTIC_CC(SpinLock);
};
} // namespace common

#endif // COMMON_COMPONENTS_BASE_SPINLOCK_H
