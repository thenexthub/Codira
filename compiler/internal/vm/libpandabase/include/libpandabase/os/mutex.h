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

#ifndef LIBPANDABASE_OS_MUTEX_H
#define LIBPANDABASE_OS_MUTEX_H

#if defined(PANDA_USE_FUTEX)
#include "unix/libpandabase/futex/mutex.h"
#elif !defined(PANDA_TARGET_UNIX) && !defined(PANDA_TARGET_WINDOWS)
#error "Unsupported platform"
#endif

#include "clang.h"
#include "macros.h"

#include <atomic>
#include <pthread.h>

namespace panda::os::memory {

// Dummy lock which locks nothing
// but has the same methods as RWLock and Mutex.
// Can be used in Locks Holders.
class DummyLock {
public:
    void Lock() const {}
    void Unlock() const {}
    void ReadLock() const {}
    void WriteLock() const {}
};

#if defined(PANDA_USE_FUTEX)
using Mutex = panda::os::unix::memory::futex::Mutex;
using RecursiveMutex = panda::os::unix::memory::futex::RecursiveMutex;
using RWLock = panda::os::unix::memory::futex::RWLock;
using ConditionVariable = panda::os::unix::memory::futex::ConditionVariable;
#else
class ConditionVariable;

class CAPABILITY("mutex") Mutex {
public:
    explicit Mutex(bool is_init = true);

    ~Mutex();

    void Lock() ACQUIRE();

    bool TryLock() TRY_ACQUIRE(true);

    void Unlock() RELEASE();

    // TODO: Extract common part as an interface.
    static bool DoNotCheckOnDeadlock()
    {
        return no_check_for_deadlock_;
    }

    static void IgnoreChecksOnDeadlock()
    {
        no_check_for_deadlock_ = true;
    }

protected:
    void Init(pthread_mutexattr_t *attrs);

private:
    pthread_mutex_t mutex_;

    // This field is set to false in case of deadlock with daemon threads (only daemon threads
    // are not finished and they have state IS_BLOCKED). In this case we should terminate
    // those threads ignoring failures on lock structures destructors.
    static std::atomic_bool no_check_for_deadlock_;

    NO_COPY_SEMANTIC(Mutex);
    NO_MOVE_SEMANTIC(Mutex);

    friend ConditionVariable;
};

class CAPABILITY("mutex") RecursiveMutex : public Mutex {
public:
    RecursiveMutex();

    ~RecursiveMutex() = default;

    NO_COPY_SEMANTIC(RecursiveMutex);
    NO_MOVE_SEMANTIC(RecursiveMutex);
};

class CAPABILITY("mutex") RWLock {
public:
    RWLock();

    ~RWLock();

    void ReadLock() ACQUIRE_SHARED();

    void WriteLock() ACQUIRE();

    bool TryReadLock() TRY_ACQUIRE_SHARED(true);

    bool TryWriteLock() TRY_ACQUIRE(true);

    void Unlock() RELEASE_GENERIC();

private:
    pthread_rwlock_t rwlock_;

    NO_COPY_SEMANTIC(RWLock);
    NO_MOVE_SEMANTIC(RWLock);
};

// Some RTOS could not have support for condition variables, so this primitive should be used carefully
class ConditionVariable {
public:
    ConditionVariable();

    ~ConditionVariable();

    void Signal();

    void SignalAll();

    void Wait(Mutex *mutex);

    bool TimedWait(Mutex *mutex, uint64_t ms, uint64_t ns = 0, bool is_absolute = false);

private:
    pthread_cond_t cond_;

    NO_COPY_SEMANTIC(ConditionVariable);
    NO_MOVE_SEMANTIC(ConditionVariable);
};
#endif  // PANDA_USE_FUTEX

using PandaThreadKey = pthread_key_t;
const auto PandaGetspecific = pthread_getspecific;     // NOLINT(readability-identifier-naming)
const auto PandaSetspecific = pthread_setspecific;     // NOLINT(readability-identifier-naming)
const auto PandaThreadKeyCreate = pthread_key_create;  // NOLINT(readability-identifier-naming)

template <class T, bool need_lock = true>
class SCOPED_CAPABILITY LockHolder {
public:
    explicit LockHolder(T &lock) ACQUIRE(lock) : lock_(lock)
    {
        if constexpr (need_lock) {  // NOLINTNEXTLINE(readability-braces-around-statements)
            lock_.Lock();
        }
    }

    ~LockHolder() RELEASE()
    {
        if constexpr (need_lock) {  // NOLINTNEXTLINE(readability-braces-around-statements)
            lock_.Unlock();
        }
    }

private:
    T &lock_;

    NO_COPY_SEMANTIC(LockHolder);
    NO_MOVE_SEMANTIC(LockHolder);
};

template <class T, bool need_lock = true>
class SCOPED_CAPABILITY ReadLockHolder {
public:
    explicit ReadLockHolder(T &lock) ACQUIRE_SHARED(lock) : lock_(lock)
    {
        if constexpr (need_lock) {  // NOLINTNEXTLINE(readability-braces-around-statements)
            lock_.ReadLock();
        }
    }

    ~ReadLockHolder() RELEASE()
    {
        if constexpr (need_lock) {  // NOLINTNEXTLINE(readability-braces-around-statements)
            lock_.Unlock();
        }
    }

private:
    T &lock_;

    NO_COPY_SEMANTIC(ReadLockHolder);
    NO_MOVE_SEMANTIC(ReadLockHolder);
};

template <class T, bool need_lock = true>
class SCOPED_CAPABILITY WriteLockHolder {
public:
    explicit WriteLockHolder(T &lock) ACQUIRE(lock) : lock_(lock)
    {
        if constexpr (need_lock) {  // NOLINTNEXTLINE(readability-braces-around-statements)
            lock_.WriteLock();
        }
    }

    ~WriteLockHolder() RELEASE()
    {
        if constexpr (need_lock) {  // NOLINTNEXTLINE(readability-braces-around-statements)
            lock_.Unlock();
        }
    }

private:
    T &lock_;

    NO_COPY_SEMANTIC(WriteLockHolder);
    NO_MOVE_SEMANTIC(WriteLockHolder);
};

}  // namespace panda::os::memory

#endif  // LIBPANDABASE_OS_MUTEX_H
