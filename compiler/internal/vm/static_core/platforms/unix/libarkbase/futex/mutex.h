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

#ifndef PANDA_LIBPANDABASE_PBASE_OS_UNIX__FUTEX_MUTEX_H_
#define PANDA_LIBPANDABASE_PBASE_OS_UNIX__FUTEX_MUTEX_H_

#include "libarkbase/clang.h"
#include "libarkbase/os/thread.h"
#include "libarkbase/macros.h"
#include "fmutex.h"

#include <array>
#include <atomic>
#include <limits>
#include <iostream>

#include <unistd.h>
#include <sys/syscall.h>
#include <linux/futex.h>

namespace ark::os::unix::memory::futex {
// We need to update TLS current tid after fork
PANDA_PUBLIC_API void PostFork();

class ConditionVariable;

class CAPABILITY("mutex") Mutex {
public:
    PANDA_PUBLIC_API Mutex();

    PANDA_PUBLIC_API ~Mutex();

    PANDA_PUBLIC_API void Lock() ACQUIRE();

    PANDA_PUBLIC_API bool TryLock() TRY_ACQUIRE(true);

    PANDA_PUBLIC_API bool TryLockWithSpinning() TRY_ACQUIRE(true);

    PANDA_PUBLIC_API void Unlock() RELEASE();

    // Should be used only in monitor. Intended to be used with just created mutexes which aren't in use yet
    // Registers `thread` as mutex's owner and locks it
    PANDA_PUBLIC_API void LockForOther(ark::os::thread::ThreadId thread);

    // Should be used only in monitor. Intended to be used with just created mutexes which aren't in use yet
    // Unegisters `thread` as mutex's owner and unlocks it
    PANDA_PUBLIC_API void UnlockForOther(ark::os::thread::ThreadId thread);

    static bool DoNotCheckOnTerminationLoop()
    {
        return MutexDoNotCheckOnTerminationLoop();
    }

    static void IgnoreChecksOnTerminationLoop()
    {
        MutexIgnoreChecksOnTerminationLoop();
    }

private:
    struct fmutex mutex_;

    int *GetStateAddr()
    {
        return futex::GetStateAddr(&mutex_);
    }

    void IncrementWaiters()
    {
        futex::IncrementWaiters(&mutex_);
    }

    void DecrementWaiters()
    {
        futex::DecrementWaiters(&mutex_);
    }

    int32_t GetWaiters()
    {
        // Atomic with relaxed order reason: mutex synchronization
        // NOLINTNEXTLINE(hicpp-signed-bitwise)
        return futex::GetWaiters(&mutex_);
    }

    bool IsHeld(ark::os::thread::ThreadId thread)
    {
        return futex::IsHeld(&mutex_, thread);
    }

    int GetRecursiveCount()
    {
        return mutex_.recursiveCount;
    }

    void SetRecursiveCount(int count)
    {
        mutex_.recursiveCount = count;
    }

    static_assert(std::atomic<ark::os::thread::ThreadId>::is_always_lock_free);

    NO_COPY_SEMANTIC(Mutex);
    NO_MOVE_SEMANTIC(Mutex);

    friend ConditionVariable;

protected:
    explicit Mutex(bool recursive) : Mutex()
    {
        mutex_.recursiveMutex = recursive;
    };
};

class CAPABILITY("mutex") RecursiveMutex : public Mutex {
public:
    RecursiveMutex() : Mutex(true) {}

    ~RecursiveMutex() = default;

private:
    NO_COPY_SEMANTIC(RecursiveMutex);
    NO_MOVE_SEMANTIC(RecursiveMutex);
};

class SHARED_CAPABILITY("mutex") RWLock {
public:
    RWLock() = default;

    PANDA_PUBLIC_API ~RWLock();

    // ReadLock and ReadUnlock are used in mutator lock often, prefer inlining over call to libpandabase
    ALWAYS_INLINE void ReadLock() ACQUIRE_SHARED()
    {
        bool done = false;
        while (!done) {
            // Atomic with relaxed order reason: mutex synchronization
            auto curState = state_.load(std::memory_order_relaxed);
            if (LIKELY(curState >= UNLOCKED)) {
                auto newState = curState + READ_INCREMENT;
                done = state_.compare_exchange_weak(curState, newState, std::memory_order_acquire);
            } else {
                HandleReadLockWait(curState);
            }
        }
        ASSERT(!HasExclusiveHolder());
    }

    ALWAYS_INLINE void Unlock() RELEASE_GENERIC()
    {
        if (HasExclusiveHolder()) {
            WriteUnlock();
        } else {
            ReadUnlock();
        }
    }

    void FutexWait(int32_t curState) ACQUIRE();
    PANDA_PUBLIC_API void WriteLock() ACQUIRE();

    PANDA_PUBLIC_API bool TryReadLock() TRY_ACQUIRE_SHARED(true);

    PANDA_PUBLIC_API bool TryWriteLock() TRY_ACQUIRE(true);

private:
    ALWAYS_INLINE void ReadUnlock() RELEASE_SHARED()
    {
        ASSERT(!HasExclusiveHolder());
        bool done = false;
        // Atomic with relaxed order reason: mutex synchronization
        auto curState = state_.load(std::memory_order_relaxed);
        while (!done) {
            if (LIKELY(curState > 0)) {
                // Reduce state by 1 and do release store.
                // waiters_ load should not be reordered before state_, so it's done with seq cst.
                auto newState = curState - READ_INCREMENT;
                // cur_state should be updated with fetched value on fail
                // Atomic with seq_cst order reason: mutex synchronization
                done = state_.compare_exchange_weak(curState, newState, std::memory_order_seq_cst);
                if (done && newState == UNLOCKED) {
                    WakeAllWaiters();
                }
            } else {
                // Cannot use logger in header
                std::cout << "RWLock ReadUnlock got unexpected state, RWLock is unlocked?" << std::endl;
                std::abort();  // CC-OFF(G.FUU.08) fatal error
            }
        }
    }

    PANDA_PUBLIC_API void WriteUnlock() RELEASE();

    // Non-inline path for handling waiting.
    PANDA_PUBLIC_API void HandleReadLockWait(int32_t curState);

    static constexpr int32_t WRITE_LOCKED = -1;
    static constexpr int32_t UNLOCKED = 0;
    static constexpr int32_t READ_INCREMENT = 1;
    // -1 - write locked; 0 - unlocked; > 0 - read locked by state_ owners.
    std::atomic_int32_t state_ {0};

    int *GetStateAddr()
    {
        return reinterpret_cast<int *>(&state_);
    }

    // Exclusive owner.
    alignas(alignof(uint32_t)) std::atomic<ark::os::thread::ThreadId> exclusiveOwner_ {0};
    static_assert(std::atomic<ark::os::thread::ThreadId>::is_always_lock_free);

    bool HasExclusiveHolder()
    {
        // Atomic with relaxed order reason: mutex synchronization
        return exclusiveOwner_.load(std::memory_order_relaxed) != 0;
    }
    bool IsExclusiveHeld(ark::os::thread::ThreadId thread)
    {
        // Atomic with relaxed order reason: mutex synchronization
        return exclusiveOwner_.load(std::memory_order_relaxed) == thread;
    }

    // Number of waiters both for read and write locks.
    std::atomic_uint32_t waiters_ {0};

    void IncrementWaiters()
    {
        // Atomic with relaxed order reason: mutex synchronization
        waiters_.fetch_add(1, std::memory_order_relaxed);
    }
    void DecrementWaiters()
    {
        // Atomic with relaxed order reason: mutex synchronization
        waiters_.fetch_sub(1, std::memory_order_relaxed);
    }

    inline void WakeAllWaiters()
    {
        // We are doing write unlock, all waiters could be ReadLocks so we need to wake all.
        // Atomic with seq_cst order reason: mutex synchronization
        if (waiters_.load(std::memory_order_seq_cst) > 0) {
            // NOLINTNEXTLINE(hicpp-signed-bitwise)
            futex(GetStateAddr(), FUTEX_WAKE_PRIVATE, WAKE_ALL, nullptr, nullptr, 0);
        }
    }

    inline void WaitForFutex(int32_t curState);

    // Extra padding to make RWLock 16 bytes long
    static constexpr size_t PADDING_SIZE = 1;
    std::array<uint32_t, PADDING_SIZE> padding_ = {0};
    // [[maybe_unused]] causes issues, dummy accessor for `padding_` as workaround
    uint32_t DummyAccessPadding()
    {
        return padding_[0];
    }

    NO_COPY_SEMANTIC(RWLock);
    NO_MOVE_SEMANTIC(RWLock);
};

class ConditionVariable {
public:
    ConditionVariable() : cond_()
    {
        futex::ConditionVariableInit(&cond_);
    }

    PANDA_PUBLIC_API ~ConditionVariable()
    {
        futex::ConditionVariableDestroy(&cond_);
    }

    void Signal()
    {
        futex::SignalCount(&cond_, WAKE_ONE);
    }

    void SignalAll()
    {
        futex::SignalCount(&cond_, WAKE_ALL);
    }

    PANDA_PUBLIC_API void Wait(Mutex *mutex)
    {
        futex::Wait(&cond_, &mutex->mutex_);
    }

    PANDA_PUBLIC_API bool TimedWait(Mutex *mutex, uint64_t ms, uint64_t ns = 0, bool isAbsolute = false)
    {
        return futex::TimedWait(&cond_, &mutex->mutex_, ms, ns, isAbsolute);
    }

private:
    struct CondVar cond_;
    static_assert(std::atomic<Mutex *>::is_always_lock_free);

    NO_COPY_SEMANTIC(ConditionVariable);
    NO_MOVE_SEMANTIC(ConditionVariable);
};

static constexpr size_t ALL_STRUCTURES_SIZE = 16U;
static_assert(sizeof(ConditionVariable) == ALL_STRUCTURES_SIZE);
static_assert(sizeof(RWLock) == ALL_STRUCTURES_SIZE);
}  // namespace ark::os::unix::memory::futex

#endif  // PANDA_LIBPANDABASE_PBASE_OS_UNIX__FUTEX_MUTEX_H_
