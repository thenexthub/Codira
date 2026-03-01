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

#ifndef PANDA_LIBPANDABASE_PBASE_OS_UNIX_FUTEX_FMUTEX_H
#define PANDA_LIBPANDABASE_PBASE_OS_UNIX_FUTEX_FMUTEX_H

#ifdef MC_ON
#include <assert.h>
#include <limits.h>
#include <pthread.h>
#include <stdatomic.h>
#define THREAD_ID pthread_t
#define GET_CURRENT_THREAD pthread_self()
#define ATOMIC(type) _Atomic type
#define ATOMIC_INT atomic_int
#define ATOMIC_STORE(addr, val, mem) atomic_store_explicit(addr, val, mem)
#define ATOMIC_LOAD(addr, mem) atomic_load_explicit(addr, mem)
#define ATOMIC_FETCH_ADD(addr, val, mem) atomic_fetch_add_explicit(addr, val, mem)
#define ATOMIC_FETCH_SUB(addr, val, mem) atomic_fetch_sub_explicit(addr, val, mem)
#define ATOMIC_CAS_WEAK(addr, old_val, new_val, mem1, mem2) \
    atomic_compare_exchange_weak_explicit((addr), &(old_val), (new_val), (mem1), (mem2))
#define ASSERT(a) assert(a)
#define LIKELY(a) a
#define UNLIKELY(a) a
#define MIN(a, b) (((a) < (b)) ? (a) : (b))
#define MAX(a, b) (((a) > (b)) ? (a) : (b))
#else
#include <unistd.h>
#include <limits>
#include <sys/param.h>
#include <atomic>
#include <libarkbase/os/thread.h>
#include <sys/syscall.h>
#include <linux/futex.h>
namespace ark::os::unix::memory::futex {
#define THREAD_ID ark::os::thread::ThreadId                                // NOLINT(cppcoreguidelines-macro-usage)
#define GET_CURRENT_THREAD ark::os::thread::GetCurrentThreadId()           // NOLINT(cppcoreguidelines-macro-usage)
#define ATOMIC(type) std::atomic<type>                                     // NOLINT(cppcoreguidelines-macro-usage)
#define ATOMIC_INT ATOMIC(int)                                             // NOLINT(cppcoreguidelines-macro-usage)
// CC-OFFNXT(G.PRE.02) namespace member
#define ATOMIC_STORE(addr, val, mem) (addr)->store(val, std::mem)          // NOLINT(cppcoreguidelines-macro-usage)
// CC-OFFNXT(G.PRE.02) namespace member
#define ATOMIC_LOAD(addr, mem) (addr)->load(std::mem)                      // NOLINT(cppcoreguidelines-macro-usage)
// CC-OFFNXT(G.PRE.02) namespace member
#define ATOMIC_FETCH_ADD(addr, val, mem) (addr)->fetch_add(val, std::mem)  // NOLINT(cppcoreguidelines-macro-usage)
// CC-OFFNXT(G.PRE.02) namespace member
#define ATOMIC_FETCH_SUB(addr, val, mem) (addr)->fetch_sub(val, std::mem)  // NOLINT(cppcoreguidelines-macro-usage)
// CC-OFFNXT(G.PRE.02) namespace member
// NOLINTNEXTLINE(cppcoreguidelines-macro-usage)
#define ATOMIC_CAS_WEAK(addr, old_val, new_val, mem1, mem2) \
    (addr)->compare_exchange_weak(old_val, new_val, std::mem1, std::mem2)
#endif

// Copy of mutex storage, after complete implementation should totally replace mutex::current_tid
// NOLINTNEXTLINE(readability-identifier-naming)
extern thread_local THREAD_ID current_tid;

void MutexInit(struct fmutex *const m);
void MutexDestroy(struct fmutex *const m);
bool MutexLock(struct fmutex *const m, bool trylock);
bool MutexTryLockWithSpinning(struct fmutex *const m);
void MutexUnlock(struct fmutex *const m);
void MutexLockForOther(struct fmutex *const m, THREAD_ID thread);
void MutexUnlockForOther(struct fmutex *const m, THREAD_ID thread);

#ifdef MC_ON
// GenMC does not support syscalls(futex)
// Models are defined in .c file to avoid code style warnings
#else
// NOLINTNEXTLINE(readability-identifier-naming)
inline int futex(volatile int *uaddr, int op, int val, const struct timespec *timeout, volatile int *uaddr2, int val3)
{
    // NOLINTNEXTLINE
    return syscall(SYS_futex, uaddr, op, val, timeout, uaddr2, val3);
}
#endif

static constexpr int WAKE_ONE = 1;
static constexpr int WAKE_ALL = INT_MAX;
static constexpr int32_t HELD_MASK = 1;
static constexpr int32_t WAITER_SHIFT = 1;
// NOLINTNEXTLINE(hicpp-signed-bitwise, cppcoreguidelines-narrowing-conversions, bugprone-narrowing-conversions)
static constexpr int32_t WAITER_INCREMENT = 1 << WAITER_SHIFT;

struct fmutex {
    // Lowest bit: 0 - unlocked, 1 - locked.
    // Other bits: Number of waiters.
    // Unified lock state and waiters count to avoid requirement of double seq_cst memory order on mutex unlock
    // as it's done in RWLock::WriteUnlock
    ATOMIC_INT stateAndWaiters;
    ATOMIC(THREAD_ID) exclusiveOwner;
    int recursiveCount;
    bool recursiveMutex;
};

int *GetStateAddr(struct fmutex *const m);
void IncrementWaiters(struct fmutex *const m);
void DecrementWaiters(struct fmutex *const m);
int32_t GetWaiters(struct fmutex *const m);
bool IsHeld(struct fmutex *const m, THREAD_ID thread);
__attribute__((visibility("default"))) bool MutexDoNotCheckOnTerminationLoop();
__attribute__((visibility("default"))) void MutexIgnoreChecksOnTerminationLoop();

struct CondVar {
#ifdef MC_ON
    alignas(alignof(uint64_t)) struct fmutex *ATOMIC(mutexPtr);
#else
    alignas(alignof(uint64_t)) ATOMIC(struct fmutex *) mutexPtr;
#endif
    // The value itself is not important, detected only its change
    ATOMIC(int32_t) cond;
    ATOMIC(int32_t) waiters;
};

__attribute__((visibility("default"))) void ConditionVariableInit(struct CondVar *const cond);
__attribute__((visibility("default"))) void ConditionVariableDestroy(struct CondVar *const cond);
__attribute__((visibility("default"))) void SignalCount(struct CondVar *const cond, int32_t toWake);
__attribute__((visibility("default"))) void Wait(struct CondVar *const cond, struct fmutex *const m);
__attribute__((visibility("default"))) bool TimedWait(struct CondVar *cond, struct fmutex *m, uint64_t ms, uint64_t ns,
                                                      bool isAbsolute);

#ifndef MC_ON
}  // namespace ark::os::unix::memory::futex
#endif

#endif  // PANDA_LIBPANDABASE_PBASE_OS_UNIX_FUTEX_FMUTEX_H
