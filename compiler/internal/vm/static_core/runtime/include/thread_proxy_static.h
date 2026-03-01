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
#ifndef PANDA_RUNTIME_THREAD_PROXY_STATIC_H
#define PANDA_RUNTIME_THREAD_PROXY_STATIC_H

#include "runtime/include/thread_interface.h"
#include "libarkbase/macros.h"

#include <atomic>

namespace ark {

class MutatorLock;

class ThreadProxyStatic : public ThreadInterface {
public:
    explicit ThreadProxyStatic(MutatorLock *mutatorLock) : ThreadInterface(mutatorLock) {}

    // NO_THREAD_SANITIZE for invalid TSAN data race report
    NO_THREAD_SANITIZE bool TestAllFlags() const;

    PANDA_PUBLIC_API enum ThreadStatus GetStatus() const;

    void InitializeThreadFlag();

    void CleanUpThreadStatus();

    PANDA_PUBLIC_API void UpdateStatus(enum ThreadStatus status);

    inline bool IsSuspended() const
    {
        return ReadFlag(SUSPEND_REQUEST);
    }

    inline bool IsRuntimeTerminated() const
    {
        return ReadFlag(RUNTIME_TERMINATION_REQUEST);
    }

    inline void SetRuntimeTerminated()
    {
        SetFlag(RUNTIME_TERMINATION_REQUEST);
    }

    bool IsThreadAlive() const
    {
        return GetStatus() != ThreadStatus::FINISHED;
    }

    static constexpr uint32_t GetFlagOffset()
    {
        return MEMBER_OFFSET(ThreadProxyStatic, fts_);
    }

    static void InitializeInitThreadFlag();

    // NO_THREAD_SAFETY_ANALYSIS due to TSAN not being able to determine lock status
    /// Transition to suspended and back to runnable, re-acquire share on mutator_lock_
    PANDA_PUBLIC_API void SuspendCheck() NO_THREAD_SAFETY_ANALYSIS;

    PANDA_PUBLIC_API void SuspendImpl(bool internalSuspend = false);

    PANDA_PUBLIC_API void ResumeImpl(bool internalResume = false);

    PANDA_PUBLIC_API void SafepointPoll();

    PANDA_PUBLIC_API bool IsUserSuspended() const;

    PANDA_PUBLIC_API void WaitSuspension();

    void MakeTSANHappyForThreadState();

    /* @sync 1
     * @description This synchronization point can be used to insert a new attribute or method
     * into ManagedThread class.
     */
private:
    // NO_THREAD_SAFETY_ANALYSIS due to TSAN not being able to determine lock status
    void TransitionFromRunningToSuspended(enum ThreadStatus status) NO_THREAD_SAFETY_ANALYSIS;

    // Separate functions for NO_THREAD_SANITIZE to suppress TSAN data race report
    NO_THREAD_SANITIZE uint32_t ReadFlagsAndThreadStatusUnsafe();

    // NO_THREAD_SANITIZE for invalid TSAN data race report
    NO_THREAD_SANITIZE bool ReadFlag(ThreadFlag flag) const
    {
        // NOLINTNEXTLINE(cppcoreguidelines-pro-type-union-access)
        return (fts_.asStruct.flags & static_cast<uint16_t>(flag)) != 0;
    }

    // Separate functions for NO_THREAD_SANITIZE to suppress TSAN data race report
    NO_THREAD_SANITIZE uint32_t ReadFlagsUnsafe() const;

    void SetFlag(ThreadFlag flag);

    void ClearFlag(ThreadFlag flag);

    enum SafepointFlag : bool { DONT_CHECK_SAFEPOINT = false, CHECK_SAFEPOINT = true };
    enum ReadlockFlag : bool { NO_READLOCK = false, READLOCK = true };

    // NO_THREAD_SAFETY_ANALYSIS due to TSAN not being able to determine lock status
    template <SafepointFlag SAFEPOINT = DONT_CHECK_SAFEPOINT, ReadlockFlag READLOCK_FLAG = NO_READLOCK>
    void StoreStatus(ThreadStatus status) NO_THREAD_SAFETY_ANALYSIS
    {
        // NOLINTBEGIN(cppcoreguidelines-pro-type-union-access)
        while (true) {
            union FlagsAndThreadStatus oldFts {
            };
            union FlagsAndThreadStatus newFts {
            };
            oldFts.asInt = ReadFlagsAndThreadStatusUnsafe();

            // NOLINTNEXTLINE(readability-braces-around-statements, hicpp-braces-around-statements)
            if constexpr (SAFEPOINT == CHECK_SAFEPOINT) {  // NOLINT(bugprone-suspicious-semicolon)
                if (oldFts.asStructNonvolatile.flags != initialThreadFlag_) {
                    // someone requires a safepoint
                    SafepointPoll();
                    continue;
                }
            }

            // mutator lock should be acquired before change status
            // to avoid blocking in running state
            // NOLINTNEXTLINE(readability-braces-around-statements, hicpp-braces-around-statements)
            if constexpr (READLOCK_FLAG == READLOCK) {  // NOLINT(bugprone-suspicious-semicolon)
                GetMutatorLock()->ReadLock();
            }

            // clang-format conflicts with CodeCheckAgent, so disable it here
            // clang-format off
            // If it's not required to check safepoint, CAS would behave the same as
            // regular STORE, just will do extra attempts, in this case it makes sense
            // to use STORE for just thread status 16-bit word.
            if constexpr (SAFEPOINT == DONT_CHECK_SAFEPOINT) {
                auto newStatus = static_cast<uint16_t>(status);
                // Atomic with release order reason: data race with other mutators
                fts_.asAtomic.status.store(newStatus, std::memory_order_release);
                break;
            }

            // if READLOCK, there's chance, someone changed the flags
            // in parallel, let's check before the CAS.
            if constexpr (READLOCK_FLAG == READLOCK) {
                if (ReadFlagsUnsafe() != oldFts.asStructNonvolatile.flags) {
                    GetMutatorLock()->Unlock();
                    continue;
                }
            }

            newFts.asStructNonvolatile.flags = oldFts.asStructNonvolatile.flags;
            newFts.asStructNonvolatile.status = status;
            // Atomic with release order reason: data race with other mutators
            if (fts_.asAtomicInt.compare_exchange_weak(
                oldFts.asNonvolatileInt, newFts.asNonvolatileInt, std::memory_order_release)) {
                // If CAS succeeded, we set new status and no request occurred here, safe to proceed.
                break;
            }
            // Release mutator lock to acquire it on the next loop iteration
            // clang-format on
            // NOLINTNEXTLINE(readability-braces-around-statements, hicpp-braces-around-statements)
            if constexpr (READLOCK_FLAG == READLOCK) {  // NOLINT(bugprone-suspicious-semicolon)
                GetMutatorLock()->Unlock();
            }
        }
        // NOLINTEND(cppcoreguidelines-pro-type-union-access)
    }

    union __attribute__((__aligned__(4))) FlagsAndThreadStatus {
        FlagsAndThreadStatus() = default;
        ~FlagsAndThreadStatus() = default;

        volatile uint32_t asInt;
        uint32_t asNonvolatileInt;
        std::atomic_uint32_t asAtomicInt;

        struct __attribute__((packed)) {
            volatile uint16_t flags;
            volatile enum ThreadStatus status;
        } asStruct;

        struct __attribute__((packed)) {
            uint16_t flags;
            enum ThreadStatus status;
        } asStructNonvolatile;

        struct {
            std::atomic_uint16_t flags;
            std::atomic_uint16_t status;
        } asAtomic;

        NO_COPY_SEMANTIC(FlagsAndThreadStatus);
        NO_MOVE_SEMANTIC(FlagsAndThreadStatus);
    };

    // NOLINTBEGIN(misc-non-private-member-variables-in-classes)
    FlagsAndThreadStatus fts_ {};
    PANDA_PUBLIC_API static ThreadFlag initialThreadFlag_;
    // NOLINTEND(misc-non-private-member-variables-in-classes)

    static constexpr uint32_t THREAD_STATUS_OFFSET = 16;
    static_assert(sizeof(fts_) == sizeof(uint32_t), "Wrong fts_ size");

    os::memory::ConditionVariable suspendVar_ GUARDED_BY(suspendLock_);
    os::memory::Mutex suspendLock_;
    uint32_t suspendCount_ GUARDED_BY(suspendLock_) = 0;
    std::atomic_uint32_t userCodeSuspendCount_ {0};
};

}  // namespace ark

#endif  // PANDA_RUNTIME_THREAD_PROXY_STATIC_H
