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

#ifndef PANDA_LIBPANDABASE_PBASE_OS_MACROS_H_
#define PANDA_LIBPANDABASE_PBASE_OS_MACROS_H_

#if defined(PANDA_USE_FUTEX)
#include "platforms/unix/libarkbase/futex/mutex.h"
#elif !defined(PANDA_TARGET_UNIX) && !defined(PANDA_TARGET_WINDOWS)
#error "Unsupported platform"
#endif

#include "libarkbase/clang.h"
#include "libarkbase/macros.h"
#include "libarkbase/os/thread.h"

#include <atomic>
#include <pthread.h>
#include <tuple>

namespace ark::os::memory {

// Dummy lock which locks nothing
// but has the same methods as RWLock and Mutex.
// Can be used in Locks Holders.
class CAPABILITY("mutex") DummyLock {
public:
    void Lock() const ACQUIRE() {}
    bool TryLock() const TRY_ACQUIRE(true)
    {
        return true;
    }
    bool TryReadLock() const TRY_ACQUIRE_SHARED(true)
    {
        return true;
    }
    bool TryWriteLock() const TRY_ACQUIRE(true)
    {
        return true;
    }
    void Unlock() const RELEASE() {}
    void ReadLock() const ACQUIRE_SHARED() {}
    void WriteLock() const ACQUIRE() {}
};

#if defined(PANDA_USE_FUTEX)
using Mutex = ark::os::unix::memory::futex::Mutex;
using RecursiveMutex = ark::os::unix::memory::futex::RecursiveMutex;
using RWLock = ark::os::unix::memory::futex::RWLock;
using ConditionVariable = ark::os::unix::memory::futex::ConditionVariable;
#else
class ConditionVariable;

class CAPABILITY("mutex") Mutex {
public:
    PANDA_PUBLIC_API explicit Mutex(bool is_init = true);

    PANDA_PUBLIC_API ~Mutex();

    PANDA_PUBLIC_API void Lock() ACQUIRE();

    PANDA_PUBLIC_API bool TryLock() TRY_ACQUIRE(true);

    PANDA_PUBLIC_API void Unlock() RELEASE();

    // NOTE(mwx851039): Extract common part as an interface.
    static bool DoNotCheckOnTerminationLoop()
    {
        return no_check_for_deadlock_;
    }

    static void IgnoreChecksOnTerminationLoop()
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
    PANDA_PUBLIC_API static std::atomic_bool no_check_for_deadlock_;

    NO_COPY_SEMANTIC(Mutex);
    NO_MOVE_SEMANTIC(Mutex);

    friend ConditionVariable;
};

class CAPABILITY("mutex") RecursiveMutex : public Mutex {
public:
    PANDA_PUBLIC_API RecursiveMutex();

    ~RecursiveMutex() = default;

    NO_COPY_SEMANTIC(RecursiveMutex);
    NO_MOVE_SEMANTIC(RecursiveMutex);
};

class CAPABILITY("mutex") RWLock {
public:
    PANDA_PUBLIC_API RWLock();

    PANDA_PUBLIC_API ~RWLock();

    PANDA_PUBLIC_API void ReadLock() ACQUIRE_SHARED();

    PANDA_PUBLIC_API void WriteLock() ACQUIRE();

    PANDA_PUBLIC_API bool TryReadLock() TRY_ACQUIRE_SHARED(true);

    PANDA_PUBLIC_API bool TryWriteLock() TRY_ACQUIRE(true);

    PANDA_PUBLIC_API void Unlock() RELEASE_GENERIC();

private:
    pthread_rwlock_t rwlock_;

    NO_COPY_SEMANTIC(RWLock);
    NO_MOVE_SEMANTIC(RWLock);
};

// Some RTOS could not have support for condition variables, so this primitive should be used carefully
class ConditionVariable {
public:
    PANDA_PUBLIC_API ConditionVariable();

    PANDA_PUBLIC_API ~ConditionVariable();

    PANDA_PUBLIC_API void Signal();

    PANDA_PUBLIC_API void SignalAll();

    PANDA_PUBLIC_API void Wait(Mutex *mutex);

    PANDA_PUBLIC_API bool TimedWait(Mutex *mutex, uint64_t ms, uint64_t ns = 0, bool is_absolute = false);

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

template <class T, bool NEED_LOCK = true>
class SCOPED_CAPABILITY LockHolder {
public:
    explicit LockHolder(T &lock) ACQUIRE(lock) : lock_(lock)
    {
        if constexpr (NEED_LOCK) {  // NOLINTNEXTLINE(readability-braces-around-statements)
            lock_.Lock();
        }
    }

    ~LockHolder() RELEASE()
    {
        if constexpr (NEED_LOCK) {  // NOLINTNEXTLINE(readability-braces-around-statements)
            lock_.Unlock();
        }
    }

private:
    T &lock_;

    NO_COPY_SEMANTIC(LockHolder);
    NO_MOVE_SEMANTIC(LockHolder);
};

template <class T, bool NEED_LOCK = true>
class SCOPED_CAPABILITY ReadLockHolder {
public:
    explicit ReadLockHolder(T &lock) ACQUIRE_SHARED(lock) : lock_(lock)
    {
        if constexpr (NEED_LOCK) {  // NOLINTNEXTLINE(readability-braces-around-statements)
            lock_.ReadLock();
        }
    }

    ~ReadLockHolder() RELEASE()
    {
        if constexpr (NEED_LOCK) {  // NOLINTNEXTLINE(readability-braces-around-statements)
            lock_.Unlock();
        }
    }

private:
    T &lock_;

    NO_COPY_SEMANTIC(ReadLockHolder);
    NO_MOVE_SEMANTIC(ReadLockHolder);
};

template <class T, bool NEED_LOCK = true>
class SCOPED_CAPABILITY WriteLockHolder {
public:
    explicit WriteLockHolder(T &lock) ACQUIRE(lock) : lock_(lock)
    {
        if constexpr (NEED_LOCK) {  // NOLINTNEXTLINE(readability-braces-around-statements)
            lock_.WriteLock();
        }
    }

    ~WriteLockHolder() RELEASE()
    {
        if constexpr (NEED_LOCK) {  // NOLINTNEXTLINE(readability-braces-around-statements)
            lock_.Unlock();
        }
    }

private:
    T &lock_;

    NO_COPY_SEMANTIC(WriteLockHolder);
    NO_MOVE_SEMANTIC(WriteLockHolder);
};

enum class LockType : uint8_t {
    LOCK,
    READ_LOCK,
    WRITE_LOCK,
};

namespace detail {

template <LockType TYPE, class L>
inline bool TryLockMutex(L &lock) NO_THREAD_SAFETY_ANALYSIS
{
    if constexpr (TYPE == LockType::LOCK) {
        return lock.TryLock();
    } else if constexpr (TYPE == LockType::READ_LOCK) {
        return lock.TryReadLock();
    } else if constexpr (TYPE == LockType::WRITE_LOCK) {
        return lock.TryWriteLock();
    }
    return true;
}

template <LockType TYPE, class L>
inline int TryLock(L &lock)
{
    if (TryLockMutex<TYPE>(lock)) {
        return -1;
    }
    return 0;
}

template <LockType TYPE, class L0, class... L1>
inline int TryLock(L0 &lock0, L1 &...rest) NO_THREAD_SAFETY_ANALYSIS
{
    if (TryLockMutex<TYPE>(lock0)) {
        int failedIndex = TryLock<TYPE>(rest...);
        if (failedIndex == -1) {
            return -1;
        }
        lock0.Unlock();
        return failedIndex + 1;
    }
    return 0;
}

template <LockType TYPE, class L>
inline void LockMutex(L &lock) NO_THREAD_SAFETY_ANALYSIS
{
    if constexpr (TYPE == LockType::LOCK) {
        lock.Lock();
    } else if constexpr (TYPE == LockType::READ_LOCK) {
        lock.ReadLock();
    } else if constexpr (TYPE == LockType::WRITE_LOCK) {
        lock.WriteLock();
    }
}

}  // namespace detail

template <LockType TYPE, class L0>
inline void Lock(L0 &lock)
{
    detail::LockMutex<TYPE>(lock);
}

template <LockType TYPE, class L0, class... L1>
void Lock(L0 &lock0, L1 &...rest) NO_THREAD_SAFETY_ANALYSIS
{
    detail::LockMutex<TYPE>(lock0);
    int failedIndex = detail::TryLock<TYPE>(rest...);
    if (failedIndex == -1) {
        return;
    }

    lock0.Unlock();
    thread::Yield();
    Lock<TYPE>(rest..., lock0);
}

/**
 * @brief RAII handle for locking an arbitrary number of mutexes using a deadlock avoidance algorithm
 * @warning: Does not support thread safety analysis
 * @tparam locks - locks to be locked for the scope duration
 * @tparam NEED_LOCK - specifies whether the locks are actually aquired
 */
template <bool NEED_LOCK = true, class... T>
class ScopedLock {
public:
    explicit ScopedLock(T &...locks) : locks_(std::tie(locks...))
    {
        static_assert(sizeof...(T) > 0, "ScopedLock argument list can not be empty");

        if constexpr (NEED_LOCK) {  // NOLINTNEXTLINE(readability-braces-around-statements)
            Lock<LockType::LOCK>(locks...);
        }
    }

    ~ScopedLock()
    {
        if constexpr (NEED_LOCK) {  // NOLINTNEXTLINE(readability-braces-around-statements)
            auto unlock = [](auto &...lock) NO_THREAD_SAFETY_ANALYSIS { (lock.Unlock(), ...); };
            std::apply(unlock, locks_);
        }
    }

private:
    std::tuple<T &...> locks_;

    NO_COPY_SEMANTIC(ScopedLock);
    NO_MOVE_SEMANTIC(ScopedLock);
};

/**
 * @brief RAII handle for read locking an arbitrary number of mutexes using a deadlock avoidance algorithm
 * @warning: Does not support thread safety analysis
 * @tparam locks - locks to be locked for the scope duration
 * @tparam NEED_LOCK - specifies whether the locks are actually aquired
 */
template <bool NEED_LOCK = true, class... T>
class ReadScopedLock {
public:
    explicit ReadScopedLock(T &...locks) : locks_(std::tie(locks...))
    {
        static_assert(sizeof...(T) > 0, "ReadScopedLock argument list can not be empty");

        if constexpr (NEED_LOCK) {  // NOLINTNEXTLINE(readability-braces-around-statements)
            Lock<LockType::READ_LOCK>(locks...);
        }
    }

    ~ReadScopedLock()
    {
        if constexpr (NEED_LOCK) {  // NOLINTNEXTLINE(readability-braces-around-statements)
            auto unlock = [](auto &...lock) NO_THREAD_SAFETY_ANALYSIS { (lock.Unlock(), ...); };
            std::apply(unlock, locks_);
        }
    }

private:
    std::tuple<T &...> locks_;

    NO_COPY_SEMANTIC(ReadScopedLock);
    NO_MOVE_SEMANTIC(ReadScopedLock);
};

/**
 * @brief RAII handle for write locking an arbitrary number of mutexes using a deadlock avoidance algorithm
 * @warning: Does not support thread safety analysis
 * @tparam locks - locks to be locked for the scope duration
 * @tparam NEED_LOCK - specifies whether the locks are actually aquired
 */
template <bool NEED_LOCK = true, class... T>
class WriteScopedLock {
public:
    explicit WriteScopedLock(T &...locks) : locks_(std::tie(locks...))
    {
        static_assert(sizeof...(T) > 0, "WriteLockHolder argument list can not be empty");

        if constexpr (NEED_LOCK) {  // NOLINTNEXTLINE(readability-braces-around-statements)
            Lock<LockType::WRITE_LOCK>(locks...);
        }
    }

    ~WriteScopedLock()
    {
        if constexpr (NEED_LOCK) {  // NOLINTNEXTLINE(readability-braces-around-statements)
            auto unlock = [](auto &...lock) NO_THREAD_SAFETY_ANALYSIS { (lock.Unlock(), ...); };
            std::apply(unlock, locks_);
        }
    }

private:
    std::tuple<T &...> locks_;

    NO_COPY_SEMANTIC(WriteScopedLock);
    NO_MOVE_SEMANTIC(WriteScopedLock);
};

class Event {
public:
    Event() = default;
    ~Event() = default;
    NO_COPY_SEMANTIC(Event);
    NO_MOVE_SEMANTIC(Event);

    /**
     * Blocks current thread until another thread will fire the same event.
     * It can be used concurrently with other wait/fire.
     * It has no effect in case fire has already been caused, but guarantees happens-before.
     */
    void Wait()
    {
        os::memory::LockHolder lh(lock_);
        while (!happened_) {
            cv_.Wait(&lock_);
        }
    }

    /**
     * Unblocks all threads that are waiting the same event.
     * Can be used concurrently with other wait/fire but multiply calls have no effect.
     */
    void Fire()
    {
        os::memory::LockHolder lh(lock_);
        happened_ = true;
        cv_.SignalAll();
    }

private:
    bool happened_ = false;
    Mutex lock_;
    ConditionVariable cv_;
};

}  // namespace ark::os::memory

#endif  // PAND_LIBPANDABASE_PBASE_OS_MACROS_H_
