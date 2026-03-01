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

#include "mutex.h"
#include "utils/logger.h"

namespace panda::os::unix::memory::futex {

// Avoid repeatedly calling GetCurrentThreadId by storing tid locally
thread_local thread::ThreadId current_tid {0};

void PostFork()
{
    current_tid = os::thread::GetCurrentThreadId();
}

// Spin for small arguments and yield for longer ones.
static void BackOff(uint32_t i)
{
    static constexpr uint32_t SPIN_MAX = 10;
    if (i <= SPIN_MAX) {
        volatile uint32_t x = 0;  // Volatile to make sure loop is not optimized out.
        const uint32_t spin_count = 10 * i;
        for (uint32_t spin = 0; spin < spin_count; spin++) {
            ++x;
        }
    } else {
        thread::ThreadYield();
    }
}

// Wait until pred is true, or until timeout is reached.
// Return true if the predicate test succeeded, false if we timed out.
template <typename Pred>
static inline bool WaitBrieflyFor(std::atomic_int *addr, Pred pred)
{
    // We probably don't want to do syscall (switch context) when we use WaitBrieflyFor
    static constexpr uint32_t MAX_BACK_OFF = 10;
    static constexpr uint32_t MAX_ITER = 50;
    for (uint32_t i = 1; i <= MAX_ITER; i++) {
        BackOff(std::min(i, MAX_BACK_OFF));
        // Atomic with relaxed order reason: mutex synchronization
        if (pred(addr->load(std::memory_order_relaxed))) {
            return true;
        }
    }
    return false;
}

// NOLINTNEXTLINE(cppcoreguidelines-pro-type-member-init)
Mutex::Mutex()
{
    MutexInit(&mutex_);
}

Mutex::~Mutex()
{
    MutexDestroy(&mutex_);
}

void Mutex::Lock()
{
    MutexLock(&mutex_, false);
}

bool Mutex::TryLock()
{
    return MutexLock(&mutex_, true);
}

bool Mutex::TryLockWithSpinning()
{
    return MutexTryLockWithSpinning(&mutex_);
}

void Mutex::Unlock()
{
    MutexUnlock(&mutex_);
}

void Mutex::LockForOther(thread::ThreadId thread)
{
    MutexLockForOther(&mutex_, thread);
}

void Mutex::UnlockForOther(thread::ThreadId thread)
{
    MutexUnlockForOther(&mutex_, thread);
}

RWLock::~RWLock()
{
#ifndef PANDA_TARGET_MOBILE
    if (!Mutex::DoNotCheckOnDeadlock()) {
#endif  // PANDA_TARGET_MOBILE
        // Atomic with relaxed order reason: mutex synchronization
        if (state_.load(std::memory_order_relaxed) != 0) {
            LOG(FATAL, COMMON) << "RWLock destruction failed; state_ is non zero!";
            // Atomic with relaxed order reason: mutex synchronization
        } else if (exclusive_owner_.load(std::memory_order_relaxed) != 0) {
            LOG(FATAL, COMMON) << "RWLock destruction failed; RWLock has an owner!";
            // Atomic with relaxed order reason: mutex synchronization
        } else if (waiters_.load(std::memory_order_relaxed) != 0) {
            LOG(FATAL, COMMON) << "RWLock destruction failed; RWLock has waiters!";
        }
#ifndef PANDA_TARGET_MOBILE
    } else {
        LOG(WARNING, COMMON) << "Deadlock detected, ignoring RWLock";
    }
#endif  // PANDA_TARGET_MOBILE
}

void RWLock::WriteLock()
{
    if (current_tid == 0) {
        current_tid = os::thread::GetCurrentThreadId();
    }
    bool done = false;
    while (!done) {
        // Atomic with relaxed order reason: mutex synchronization
        auto cur_state = state_.load(std::memory_order_relaxed);
        if (LIKELY(cur_state == UNLOCKED)) {
            // Unlocked, can acquire writelock
            // Do CAS in case other thread beats us and acquires readlock first
            done = state_.compare_exchange_weak(cur_state, WRITE_LOCKED, std::memory_order_acquire);
        } else {
            // Wait until RWLock is unlocked
            if (!WaitBrieflyFor(&state_, [](int32_t state) { return state == UNLOCKED; })) {
                // WaitBrieflyFor failed, go to futex wait
                // Increment waiters count.
                IncrementWaiters();
                // Retry wait until lock not held. If we have more than one reader, cur_state check fail
                // doesn't mean this lock is unlocked.
                while (cur_state != UNLOCKED) {
                    // NOLINTNEXTLINE(hicpp-signed-bitwise)
                    if (futex(GetStateAddr(), FUTEX_WAIT_PRIVATE, cur_state, nullptr, nullptr, 0) != 0) {
                        if ((errno != EAGAIN) && (errno != EINTR)) {
                            LOG(FATAL, COMMON) << "Futex wait failed!";
                        }
                    }
                    // Atomic with relaxed order reason: mutex synchronization
                    cur_state = state_.load(std::memory_order_relaxed);
                }
                DecrementWaiters();
            }
        }
    }
    // RWLock is held now
    // Atomic with relaxed order reason: mutex synchronization
    ASSERT(state_.load(std::memory_order_relaxed) == WRITE_LOCKED);
    // Atomic with relaxed order reason: mutex synchronization
    ASSERT(exclusive_owner_.load(std::memory_order_relaxed) == 0);
    // Atomic with relaxed order reason: mutex synchronization
    exclusive_owner_.store(current_tid, std::memory_order_relaxed);
}

void RWLock::HandleReadLockWait(int32_t cur_state)
{
    // Wait until RWLock WriteLock is unlocked
    if (!WaitBrieflyFor(&state_, [](int32_t state) { return state >= UNLOCKED; })) {
        // WaitBrieflyFor failed, go to futex wait
        IncrementWaiters();
        // Retry wait until WriteLock not held.
        while (cur_state == WRITE_LOCKED) {
            // NOLINTNEXTLINE(hicpp-signed-bitwise)
            if (futex(GetStateAddr(), FUTEX_WAIT_PRIVATE, cur_state, nullptr, nullptr, 0) != 0) {
                if ((errno != EAGAIN) && (errno != EINTR)) {
                    LOG(FATAL, COMMON) << "Futex wait failed!";
                }
            }
            // Atomic with relaxed order reason: mutex synchronization
            cur_state = state_.load(std::memory_order_relaxed);
        }
        DecrementWaiters();
    }
}

bool RWLock::TryReadLock()
{
    bool done = false;
    // Atomic with relaxed order reason: mutex synchronization
    auto cur_state = state_.load(std::memory_order_relaxed);
    while (!done) {
        if (cur_state >= UNLOCKED) {
            auto new_state = cur_state + READ_INCREMENT;
            // cur_state should be updated with fetched value on fail
            done = state_.compare_exchange_weak(cur_state, new_state, std::memory_order_acquire);
        } else {
            // RWLock is Write held, trylock failed.
            return false;
        }
    }
    ASSERT(!HasExclusiveHolder());
    return true;
}

bool RWLock::TryWriteLock()
{
    if (current_tid == 0) {
        current_tid = os::thread::GetCurrentThreadId();
    }
    bool done = false;
    // Atomic with relaxed order reason: mutex synchronization
    auto cur_state = state_.load(std::memory_order_relaxed);
    while (!done) {
        if (LIKELY(cur_state == UNLOCKED)) {
            // Unlocked, can acquire writelock
            // Do CAS in case other thread beats us and acquires readlock first
            // cur_state should be updated with fetched value on fail
            done = state_.compare_exchange_weak(cur_state, WRITE_LOCKED, std::memory_order_acquire);
        } else {
            // RWLock is held, trylock failed.
            return false;
        }
    }
    // RWLock is held now
    // Atomic with relaxed order reason: mutex synchronization
    ASSERT(state_.load(std::memory_order_relaxed) == WRITE_LOCKED);
    // Atomic with relaxed order reason: mutex synchronization
    ASSERT(exclusive_owner_.load(std::memory_order_relaxed) == 0);
    // Atomic with relaxed order reason: mutex synchronization
    exclusive_owner_.store(current_tid, std::memory_order_relaxed);
    return true;
}

void RWLock::WriteUnlock()
{
    if (current_tid == 0) {
        current_tid = os::thread::GetCurrentThreadId();
    }
    ASSERT(IsExclusiveHeld(current_tid));

    bool done = false;
    // Atomic with relaxed order reason: mutex synchronization
    int32_t cur_state = state_.load(std::memory_order_relaxed);
    // CAS is weak and might fail, do in loop
    while (!done) {
        if (LIKELY(cur_state == WRITE_LOCKED)) {
            // Reset exclusive owner before changing state to avoid check failures if other thread sees UNLOCKED
            // Atomic with relaxed order reason: mutex synchronization
            exclusive_owner_.store(0, std::memory_order_relaxed);
            // Change state to unlocked and do release store.
            // waiters_ load should not be reordered before state_, so it's done with seq cst.
            // cur_state should be updated with fetched value on fail
            done = state_.compare_exchange_weak(cur_state, UNLOCKED, std::memory_order_seq_cst);
            if (LIKELY(done)) {
                // We are doing write unlock, all waiters could be ReadLocks so we need to wake all.
                // Atomic with seq_cst order reason: mutex synchronization
                if (waiters_.load(std::memory_order_seq_cst) > 0) {
                    // NOLINTNEXTLINE(hicpp-signed-bitwise)
                    futex(GetStateAddr(), FUTEX_WAKE_PRIVATE, WAKE_ALL, nullptr, nullptr, 0);
                }
            }
        } else {
            LOG(FATAL, COMMON) << "RWLock WriteUnlock got unexpected state, RWLock is not writelocked?";
        }
    }
}

ConditionVariable::~ConditionVariable()
{
#ifndef PANDA_TARGET_MOBILE
    if (!Mutex::DoNotCheckOnDeadlock()) {
#endif  // PANDA_TARGET_MOBILE
        // Atomic with relaxed order reason: mutex synchronization
        if (waiters_.load(std::memory_order_relaxed) != 0) {
            LOG(FATAL, COMMON) << "CondVar destruction failed; waiters_ is non zero!";
        }
#ifndef PANDA_TARGET_MOBILE
    } else {
        LOG(WARNING, COMMON) << "Deadlock detected, ignoring CondVar";
    }
#endif  // PANDA_TARGET_MOBILE
}

void ConditionVariable::Wait(Mutex *mutex)
{
    if (current_tid == 0) {
        current_tid = os::thread::GetCurrentThreadId();
    }
    if (!mutex->IsHeld(current_tid)) {
        LOG(FATAL, COMMON) << "CondVar Wait failed; provided mutex is not held by current thread";
    }

    // It's undefined behavior to call Wait with different mutexes on the same condvar
    Mutex *old_mutex = nullptr;
    // Atomic with relaxed order reason: mutex synchronization
    while (!mutex_ptr_.compare_exchange_weak(old_mutex, mutex, std::memory_order_relaxed)) {
        // CAS failed, either it was spurious fail and old val is nullptr, or make sure mutex ptr equals to current
        if (old_mutex != mutex && old_mutex != nullptr) {
            LOG(FATAL, COMMON) << "CondVar Wait failed; mutex_ptr_ doesn't equal to provided mutex";
        }
    }

    // Atomic with relaxed order reason: mutex synchronization
    waiters_.fetch_add(1, std::memory_order_relaxed);
    mutex->IncrementWaiters();
    auto old_count = mutex->GetRecursiveCount();
    mutex->SetRecursiveCount(1);
    // Atomic with relaxed order reason: mutex synchronization
    auto cur_cond = cond_.load(std::memory_order_relaxed);
    mutex->Unlock();
    // NOLINTNEXTLINE(hicpp-signed-bitwise)
    if (futex(GetCondAddr(), FUTEX_WAIT_PRIVATE, cur_cond, nullptr, nullptr, 0) != 0) {
        if ((errno != EAGAIN) && (errno != EINTR)) {
            LOG(FATAL, COMMON) << "Futex wait failed!";
        }
    }
    mutex->Lock();
    mutex->SetRecursiveCount(old_count);
    mutex->DecrementWaiters();
    // Atomic with relaxed order reason: mutex synchronization
    waiters_.fetch_sub(1, std::memory_order_relaxed);
}

struct timespec ConvertTime(uint64_t ms, uint64_t ns)
{
    struct timespec time = {0, 0};
    const int64_t MILLISECONDS_PER_SEC = 1000;
    const int64_t NANOSECONDS_PER_MILLISEC = 1000000;
    const int64_t NANOSECONDS_PER_SEC = 1000000000;
    auto seconds = static_cast<time_t>(ms / MILLISECONDS_PER_SEC);
    auto nanoseconds = static_cast<time_t>((ms % MILLISECONDS_PER_SEC) * NANOSECONDS_PER_MILLISEC + ns);
    time.tv_sec += seconds;
    time.tv_nsec += nanoseconds;
    if (time.tv_nsec >= NANOSECONDS_PER_SEC) {
        time.tv_nsec -= NANOSECONDS_PER_SEC;
        time.tv_sec++;
    }
    return time;
}

bool ConditionVariable::TimedWait(Mutex *mutex, uint64_t ms, uint64_t ns, bool is_absolute)
{
    if (current_tid == 0) {
        current_tid = os::thread::GetCurrentThreadId();
    }
    if (!mutex->IsHeld(current_tid)) {
        LOG(FATAL, COMMON) << "CondVar Wait failed; provided mutex is not held by current thread";
    }

    // It's undefined behavior to call Wait with different mutexes on the same condvar
    Mutex *old_mutex = nullptr;
    // Atomic with relaxed order reason: mutex synchronization
    while (!mutex_ptr_.compare_exchange_weak(old_mutex, mutex, std::memory_order_relaxed)) {
        // CAS failed, either it was spurious fail and old val is nullptr, or make sure mutex ptr equals to current
        if (old_mutex != mutex && old_mutex != nullptr) {
            LOG(FATAL, COMMON) << "CondVar Wait failed; mutex_ptr_ doesn't equal to provided mutex";
        }
    }

    bool timeout = false;
    struct timespec time = ConvertTime(ms, ns);
    // Atomic with relaxed order reason: mutex synchronization
    waiters_.fetch_add(1, std::memory_order_relaxed);
    mutex->IncrementWaiters();
    auto old_count = mutex->GetRecursiveCount();
    mutex->SetRecursiveCount(1);
    // Atomic with relaxed order reason: mutex synchronization
    auto cur_cond = cond_.load(std::memory_order_relaxed);
    mutex->Unlock();
    int futex_call_res = 0;
    if (is_absolute) {
        // FUTEX_WAIT_BITSET uses absolute time
        // NOLINTNEXTLINE(hicpp-signed-bitwise)
        static constexpr int WAIT_BITSET = FUTEX_WAIT_BITSET_PRIVATE;
        // NOLINTNEXTLINE(hicpp-signed-bitwise)
        static constexpr int MATCH_ANY = FUTEX_BITSET_MATCH_ANY;
        futex_call_res = futex(GetCondAddr(), WAIT_BITSET, cur_cond, &time, nullptr, MATCH_ANY);
    } else {
        // FUTEX_WAIT uses relative time
        // NOLINTNEXTLINE(hicpp-signed-bitwise)
        futex_call_res = futex(GetCondAddr(), FUTEX_WAIT_PRIVATE, cur_cond, &time, nullptr, 0);
    }
    if (futex_call_res != 0) {
        if (errno == ETIMEDOUT) {
            timeout = true;
        } else if ((errno != EAGAIN) && (errno != EINTR)) {
            LOG(FATAL, COMMON) << "Futex wait failed!";
        }
    }
    mutex->Lock();
    mutex->SetRecursiveCount(old_count);
    mutex->DecrementWaiters();
    // Atomic with relaxed order reason: mutex synchronization
    waiters_.fetch_sub(1, std::memory_order_relaxed);
    return timeout;
}

void ConditionVariable::SignalCount(int32_t to_wake)
{
    // Atomic with relaxed order reason: mutex synchronization
    if (waiters_.load(std::memory_order_relaxed) == 0) {
        // No waiters, do nothing
        return;
    }

    if (current_tid == 0) {
        current_tid = os::thread::GetCurrentThreadId();
    }
    // Atomic with relaxed order reason: mutex synchronization
    auto mutex = mutex_ptr_.load(std::memory_order_relaxed);
    // If this condvar has waiters, mutex_ptr_ should be set
    ASSERT(mutex != nullptr);
    // Atomic with relaxed order reason: mutex synchronization
    cond_.fetch_add(1, std::memory_order_relaxed);
    if (mutex->IsHeld(current_tid)) {
        // This thread is owner of current mutex, do requeue to mutex waitqueue
        // NOLINTNEXTLINE(hicpp-signed-bitwise)
        bool success = futex(GetCondAddr(), FUTEX_REQUEUE_PRIVATE, 0, reinterpret_cast<const timespec *>(to_wake),
                             mutex->GetStateAddr(), 0) != -1;
        if (!success) {
            LOG(FATAL, COMMON) << "Futex requeue failed!";
        }
    } else {
        // Mutex is not held by this thread, do wake
        // NOLINTNEXTLINE(hicpp-signed-bitwise)
        futex(GetCondAddr(), FUTEX_WAKE_PRIVATE, to_wake, nullptr, nullptr, 0);
    }
}

}  // namespace panda::os::unix::memory::futex
