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

#include "runtime/include/runtime.h"
#include "runtime/include/thread_proxy.h"
#include "runtime/interpreter/runtime_interface.h"

#include "libarkbase/macros.h"

namespace ark {

CONSTEXPR_IN_RELEASE ThreadFlag GetInitialThreadFlag()
{
#ifndef NDEBUG
    ThreadFlag initialFlag = Runtime::GetOptions().IsRunGcEverySafepoint() ? SAFEPOINT_REQUEST : NO_FLAGS;
    return initialFlag;
#else
    return NO_FLAGS;
#endif
}

ThreadFlag ThreadProxyStatic::initialThreadFlag_ = NO_FLAGS;

/* static */
void ThreadProxyStatic::InitializeInitThreadFlag()
{
    initialThreadFlag_ = GetInitialThreadFlag();
}

bool ThreadProxyStatic::TestAllFlags() const
{
    return (fts_.asStruct.flags) != initialThreadFlag_;  // NOLINT(cppcoreguidelines-pro-type-union-access)
}

void ThreadProxyStatic::SetFlag(ThreadFlag flag)
{
    // Atomic with seq_cst order reason: data race with flags with requirement for sequentially consistent order
    // where threads observe all modifications in the same order
    // NOLINTNEXTLINE(cppcoreguidelines-pro-type-union-access)
    fts_.asAtomic.flags.fetch_or(flag, std::memory_order_seq_cst);
}

void ThreadProxyStatic::ClearFlag(ThreadFlag flag)
{
    // Atomic with seq_cst order reason: data race with flags with requirement for sequentially consistent order
    // where threads observe all modifications in the same order
    // NOLINTNEXTLINE(cppcoreguidelines-pro-type-union-access)
    fts_.asAtomic.flags.fetch_and(UINT16_MAX ^ flag, std::memory_order_seq_cst);
}

uint32_t ThreadProxyStatic::ReadFlagsAndThreadStatusUnsafe()
{
    // NOLINTNEXTLINE(cppcoreguidelines-pro-type-union-access)
    return fts_.asInt;
}

enum ThreadStatus ThreadProxyStatic::GetStatus() const
{
    // Atomic with acquire order reason: data race with flags with dependecies on reads after
    // the load which should become visible
    // NOLINTNEXTLINE(cppcoreguidelines-pro-type-union-access)
    return static_cast<enum ThreadStatus>(fts_.asAtomic.status.load(std::memory_order_acquire));
}

uint32_t ThreadProxyStatic::ReadFlagsUnsafe() const
{
    // NOLINTNEXTLINE(cppcoreguidelines-pro-type-union-access)
    return fts_.asStruct.flags;
}

void ThreadProxyStatic::InitializeThreadFlag()
{
    // NOLINTNEXTLINE(cppcoreguidelines-pro-type-union-access)
    fts_.asInt = initialThreadFlag_;
}

void ThreadProxyStatic::CleanUpThreadStatus()
{
    InitializeThreadFlag();
    StoreStatus<DONT_CHECK_SAFEPOINT, NO_READLOCK>(ThreadStatus::CREATED);
}

void ThreadProxyStatic::UpdateStatus(enum ThreadStatus status)
{
    ThreadStatus oldStatus = GetStatus();
    if (oldStatus == ThreadStatus::RUNNING && status != ThreadStatus::RUNNING) {
        TransitionFromRunningToSuspended(status);
    } else if (oldStatus != ThreadStatus::RUNNING && status == ThreadStatus::RUNNING) {
        // NB! This thread is treated as suspended so when we transition from suspended state to
        // running we need to check suspension flag and counter so SafepointPoll has to be done before
        // acquiring mutator_lock.
        // StoreStatus acquires lock here
        StoreStatus<CHECK_SAFEPOINT, READLOCK>(ThreadStatus::RUNNING);
    } else if (oldStatus == ThreadStatus::NATIVE && status != ThreadStatus::IS_TERMINATED_LOOP &&
               IsRuntimeTerminated()) {
        // If a daemon thread with NATIVE status was deregistered, it should not access any managed object,
        // i.e. change its status from NATIVE, because such object may already be deleted by the runtime.
        // In case its status is changed, we must call a Safepoint to terminate this thread.
        // For example, if a daemon thread calls ManagedCodeBegin (which changes status from NATIVE to
        // RUNNING), it may be interrupted by a GC thread, which changes status to IS_SUSPENDED.
        StoreStatus<CHECK_SAFEPOINT>(status);
    } else {
        // NB! Status is not a simple bit, without atomics it can produce faulty GetStatus.
        StoreStatus(status);
    }
}

void ThreadProxyStatic::TransitionFromRunningToSuspended(enum ThreadStatus status)
{
    // Do Unlock after StoreStatus, because the thread requesting a suspension should see an updated status
    StoreStatus(status);
    GetMutatorLock()->Unlock();
}

void ThreadProxyStatic::SuspendCheck()
{
    // We should use internal suspension to avoid missing call of IncSuspend
    SuspendImpl(true);
    GetMutatorLock()->Unlock();
    GetMutatorLock()->ReadLock();
    ResumeImpl(true);
}

void ThreadProxyStatic::SuspendImpl(bool internalSuspend)
{
    os::memory::LockHolder lock(suspendLock_);
    LOG(DEBUG, RUNTIME) << "Suspending thread " << GetId();
    if (!internalSuspend) {
        if (IsUserSuspended()) {
            LOG(DEBUG, RUNTIME) << "thread " << GetId() << " is already suspended";
            return;
        }
        userCodeSuspendCount_++;
    }
    auto oldCount = suspendCount_++;
    if (oldCount == 0) {
        SetFlag(SUSPEND_REQUEST);
    }
}

void ThreadProxyStatic::ResumeImpl(bool internalResume)
{
    os::memory::LockHolder lock(suspendLock_);
    LOG(DEBUG, RUNTIME) << "Resuming thread " << GetId();
    if (!internalResume) {
        if (!IsUserSuspended()) {
            LOG(DEBUG, RUNTIME) << "thread " << GetId() << " is already resumed";
            return;
        }
        ASSERT(userCodeSuspendCount_ != 0);
        userCodeSuspendCount_--;
    }
    if (suspendCount_ > 0) {
        suspendCount_--;
        if (suspendCount_ == 0) {
            ClearFlag(SUSPEND_REQUEST);
        }
    }
    // Help for UnregisterExitedThread
    TSAN_ANNOTATE_HAPPENS_BEFORE(&fts_);
    suspendVar_.Signal();
}

void ThreadProxyStatic::SafepointPoll()
{
    if (this->TestAllFlags()) {
        trace::ScopedTrace scopedTrace("RunSafepoint");
        ark::interpreter::RuntimeInterface::Safepoint();
    }
}

bool ThreadProxyStatic::IsUserSuspended() const
{
    return userCodeSuspendCount_ > 0;
}

void ThreadProxyStatic::WaitSuspension()
{
    constexpr int TIMEOUT = 100;
    auto oldStatus = GetStatus();
    PrintSuspensionStackIfNeeded();
    UpdateStatus(ThreadStatus::IS_SUSPENDED);
    {
        /* @sync 1
         * @description Right after the thread updates its status to IS_SUSPENDED and right before beginning to wait
         * for actual suspension
         */
        os::memory::LockHolder lock(suspendLock_);
        while (suspendCount_ > 0) {
            suspendVar_.TimedWait(&suspendLock_, TIMEOUT);
            // In case runtime is being terminated, we should abort suspension and release monitors
            if (UNLIKELY(IsRuntimeTerminated())) {
                suspendLock_.Unlock();
                OnRuntimeTerminated();
                UNREACHABLE();
            }
        }
        ASSERT(!IsSuspended());
    }
    UpdateStatus(oldStatus);
}

void ThreadProxyStatic::MakeTSANHappyForThreadState()
{
    TSAN_ANNOTATE_HAPPENS_AFTER(&fts_);
}

#ifdef ARK_HYBRID

using namespace common;

static thread_local ThreadHolder *g_SharedExternalHolder = nullptr;

enum ThreadStatus ThreadProxyHybrid::GetStatus() const
{
    if (GetThreadHolder()->IsInRunningState()) {
        return ThreadStatus::RUNNING;
    }
    return ThreadStatus::NATIVE;
}

void ThreadProxyHybrid::UpdateStatus([[maybe_unused]] enum ThreadStatus status)
{
    ThreadStatus oldStatus = GetStatus();
    if (oldStatus == ThreadStatus::RUNNING && status != ThreadStatus::RUNNING) {
        GetThreadHolder()->TransferToNativeIfInRunning();
        GetMutatorLock()->Unlock();
    } else if (oldStatus != ThreadStatus::RUNNING && status == ThreadStatus::RUNNING) {
        GetThreadHolder()->TransferToRunningIfInNative();
        GetMutatorLock()->ReadLock();
    }
}

bool ThreadProxyHybrid::TestAllFlags() const
{
    return GetThreadHolder()->HasSuspendRequest();
}

bool ThreadProxyHybrid::IsSuspended() const
{
    return GetThreadHolder()->HasSuspendRequest();
}

void ThreadProxyHybrid::SafepointPoll()
{
    GetThreadHolder()->CheckSafepointIfSuspended();
}

void ThreadProxyHybrid::WaitSuspension()
{
    GetThreadHolder()->WaitSuspension();
}

void ThreadProxyHybrid::InitializeThreadFlag() {}

void ThreadProxyHybrid::CleanUpThreadStatus() {}

bool ThreadProxyHybrid::IsRuntimeTerminated() const
{
    return false;
}

void ThreadProxyHybrid::SetRuntimeTerminated()
{
    UNREACHABLE();
}

void ThreadProxyHybrid::SuspendCheck()
{
    UNREACHABLE();
}

void ThreadProxyHybrid::SuspendImpl([[maybe_unused]] bool internalSuspend)
{
    UNREACHABLE();
}

void ThreadProxyHybrid::ResumeImpl([[maybe_unused]] bool internalResume)
{
    UNREACHABLE();
}

bool ThreadProxyHybrid::IsUserSuspended()
{
    UNREACHABLE();
    return false;
}

void ThreadProxyHybrid::MakeTSANHappyForThreadState()
{
    UNREACHABLE();
}

void ThreadProxyHybrid::BindMutator()
{
    GetThreadHolder()->BindMutator();
}

void ThreadProxyHybrid::UnbindMutator()
{
    GetThreadHolder()->UnbindMutator();
}

bool ThreadProxyHybrid::CreateExternalHolderIfNeeded(bool useSharedHolder)
{
    if (threadHolder_ != nullptr) {
        return false;
    }

    if (useSharedHolder) {
        if (GetSharedExternalHolder() != nullptr) {
            threadHolder_ = GetSharedExternalHolder();
            return true;
        }
        // NOTE(panferovi): replace ThreadHolder::GerCurrent by new interface
        // that obtain ThreadHolder from JS, when it is implemented
        threadHolder_ = ThreadHolder::GetCurrent() != nullptr ? ThreadHolder::GetCurrent()
                                                              : ThreadHolder::CreateAndRegisterNewThreadHolder(nullptr);
        ASSERT(threadHolder_ != nullptr);
        SetSharedExternalHolder(threadHolder_);
        return true;
    }

    threadHolder_ = ThreadHolder::CreateAndRegisterNewThreadHolder(nullptr);
    return true;
}

/* static */
void ThreadProxyHybrid::SetSharedExternalHolder(ThreadHolder *externalHolder)
{
    g_SharedExternalHolder = externalHolder;
}

/* static */
ThreadHolder *ThreadProxyHybrid::GetSharedExternalHolder()
{
    return g_SharedExternalHolder;
}

#endif

}  // namespace ark
