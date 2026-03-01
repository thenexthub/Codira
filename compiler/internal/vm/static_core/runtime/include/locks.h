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
#ifndef PANDA_RUNTIME_LOCKS_H_
#define PANDA_RUNTIME_LOCKS_H_

#include "libarkbase/os/mutex.h"

#include <memory>

namespace ark {

#ifndef ARK_HYBRID
class PANDA_PUBLIC_API MutatorLock : public os::memory::RWLock {
    using LockT = os::memory::RWLock;
#else
class PANDA_PUBLIC_API MutatorLock : public os::memory::DummyLock {
    using LockT = os::memory::DummyLock;
#endif
#ifndef NDEBUG
public:
    enum MutatorLockState {UNLOCKED, RDLOCK, WRLOCK};

    void ReadLock() ACQUIRE_SHARED();

    void WriteLock() ACQUIRE();

    bool TryReadLock() TRY_ACQUIRE_SHARED(true);

    bool TryWriteLock() TRY_ACQUIRE(true);

    void Unlock() RELEASE_GENERIC();

    MutatorLockState GetState() const;

    bool HasLock() const;
#endif  // !NDEBUG
};

class PANDA_PUBLIC_API Locks {
public:
    static void Initialize();

    static MutatorLock *NewMutatorLock();

    /// Lock used for preventing custom_tls_cache_ modifications
    static os::memory::Mutex *customTlsLock_;  // NOLINT(misc-non-private-member-variables-in-classes)

    /**
     * The lock is a specific lock for exclusive suspension process,
     * It is static for access from JVMTI interface
     */
    static os::memory::Mutex *userSuspensionLock_;
};

}  // namespace ark

#endif  // PANDA_RUNTIME_LOCKS_H_
