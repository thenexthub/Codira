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

#include "runtime/monitor.h"

#include "libarkbase/os/thread.h"
#include "runtime/include/object_header.h"
#include "runtime/include/runtime.h"
#include "runtime/include/runtime_notification.h"
#include "runtime/include/thread_scopes.h"
#include "runtime/include/thread-inl.h"
#include "runtime/interpreter/runtime_interface.h"
#include "runtime/mark_word.h"
#include "runtime/monitor_pool.h"
#include "runtime/handle_base-inl.h"
#include "runtime/mem/vm_handle.h"

#include <cinttypes>
#include <string>
#include <sched.h>

namespace ark {
template <typename T>
template <typename Predicate>
bool ThreadList<T>::RemoveIf(Predicate pred)
{
    bool found = false;
    auto prev = head_;
    for (auto current = head_; current != nullptr; current = current->GetNextWait()) {
        if (pred(*current)) {
            found = true;
            EraseAfter(prev, current);
            current = prev;
        } else {
            prev = current;
        }
    }
    return found;
}

template <typename T>
void ThreadList<T>::Splice(ThreadList &other)
{
    if (Empty()) {
        head_ = other.head_;
    } else {
        T *last = head_;
        for (; last->GetNextWait() != nullptr; last = last->GetNextWait()) {
        }
        last->SetWaitNext(other.head_);
    }
    other.Clear();
}

template <typename T>
void ThreadList<T>::EraseAfter(T *prev, T *current)
{
    if (current == head_) {
        head_ = current->GetNextWait();
    } else {
        prev->SetWaitNext(current->GetNextWait());
    }
}

template <typename T>
void ThreadList<T>::PopFront()
{
    head_ = head_->GetNextWait();
}

template <typename T>
void ThreadList<T>::PushFront(T &thread)
{
    thread.SetWaitNext(head_);
    head_ = &thread;
}

void Monitor::InflateThinLock(MTManagedThread *thread, [[maybe_unused]] const VMHandle<ObjectHeader> &objHandle)
{
#if defined(PANDA_USE_FUTEX)
    // Futex inflation policy: suspend target thread, wait until it actually gets suspended
    // and try inflating light monitor (`Inflate` expects lock to still be acquired by target;
    // otherwise markword CAS fails). If it fails (i.e. thread got suspended when this monitor is
    // no longer taken), we restart lightlock acquisition policy again.
    // Compared to forced inflation (actively retry inflation once MAX_TRYLOCK_RETRY is reached
    // or inflate monitor once this thread acquires light lock), this policy yields much better
    // performance for short running synchronized blocks or functions, and is still expected to
    // succeeed on longer blocks which should have safepoints and suspend successfully with
    // monitor still acquired.
    // We are trying to inflate light lock acquired by other thread, suspend it first
    MTManagedThread *owner = nullptr;
    ASSERT(objHandle.GetPtr() != nullptr);
    MarkWord mark = objHandle.GetPtr()->AtomicGetMark();
    auto threadManager = reinterpret_cast<MTThreadManager *>(thread->GetVM()->GetThreadManager());
    os::thread::ThreadId ownerThreadId = mark.GetThreadId();
    {
        ScopedChangeThreadStatus ets(thread, ThreadStatus::IS_WAITING_INFLATION);
        owner = threadManager->SuspendAndWaitThreadByInternalThreadId(ownerThreadId);
    }
    thread->SetEnterMonitorObject(nullptr);
    thread->SetWaitingMonitorOldStatus(ThreadStatus::FINISHED);
    // Thread could have finished by the time we tried stopping it
    if (owner != nullptr) {
        // NB! Inflate can do nothing if monitor is already unlocked or acquired by other thread.
        Inflate<true>(objHandle.GetPtr(), owner);
        {
            // ResumeImpl can be called during thread termination, which leads to destruction of locked suspend_lock_
            // inside ResumeImpl.
            os::memory::LockHolder threadLock(*threadManager->GetThreadsLock());
            owner->ResumeImpl(true);
        }
    }
#else
    // Non-futex inflation policy: Wait until light lock is released, acquire it and inflate
    // to heavy monitor
    {
        static constexpr uint64_t SLEEP_MS = 10;
        thread->TimedWait(ThreadStatus::IS_WAITING_INFLATION, SLEEP_MS, 0);
    }
    thread->SetEnterMonitorObject(nullptr);
    thread->SetWaitingMonitorOldStatus(ThreadStatus::FINISHED);
#endif
}

std::optional<Monitor::State> Monitor::HandleLightLockedState(MarkWord &mark, MTManagedThread *thread,
                                                              VMHandle<ObjectHeader> &objHandle,
                                                              uint32_t &lightlockRetryCount,
                                                              [[maybe_unused]] bool &shouldInflate, bool trylock)
{
    os::thread::ThreadId ownerThreadId = mark.GetThreadId();
    if (ownerThreadId == thread->GetInternalId()) {
        uint32_t newCount = mark.GetLockCount() + 1;
        if (newCount < MarkWord::LIGHT_LOCK_LOCK_MAX_COUNT) {
            auto newMark = mark.DecodeFromLightLock(thread->GetInternalId(), newCount);
            // Strong CAS as the loop iteration is large
            ASSERT(objHandle.GetPtr() != nullptr);
            bool ret = objHandle.GetPtr()->AtomicSetMark(mark, newMark);
            if (ret) {
                LOG(DEBUG, RUNTIME) << "The lightweight monitor was successfully recursively acquired";
                Monitor::TraceMonitorLock(objHandle.GetPtr(), false);
                thread->PushLocalObjectLocked(objHandle.GetPtr());
                return Monitor::State::OK;
            }
        } else {
            Monitor::Inflate(objHandle.GetPtr(), thread);
            // Inflate set up recursive counter to just current amount, loop again.
        }
    } else {
        // Lock acquired by other thread.
        if (trylock) {
            return Monitor::State::ILLEGAL;
        }

        // Retry acquiring light lock in loop first to avoid excessive inflation
        static constexpr uint32_t MAX_TRYLOCK_RETRY = 100;
        static constexpr uint32_t YIELD_AFTER = 50;

        lightlockRetryCount++;
        if (lightlockRetryCount < MAX_TRYLOCK_RETRY) {
            if (lightlockRetryCount > YIELD_AFTER) {
                MTManagedThread::Yield();
            }
        } else {
            // Retried acquiring light lock for too long, do inflation

            thread->SetEnterMonitorObject(objHandle.GetPtr());
            thread->SetWaitingMonitorOldStatus(ThreadStatus::IS_WAITING_INFLATION);
            Monitor::InflateThinLock(thread, objHandle);
#if defined(PANDA_USE_FUTEX)
            lightlockRetryCount = 0;
#else
            shouldInflate = true;
#endif
        }
    }
    // Couldn't update mark.
    if (trylock) {
        return Monitor::State::ILLEGAL;
    }

    return std::nullopt;
}

std::optional<Monitor::State> Monitor::HandleUnlockedState(MarkWord &mark, MTManagedThread *thread,
                                                           VMHandle<ObjectHeader> &objHandle, bool &shouldInflate,
                                                           bool trylock)
{
    if (shouldInflate) {
        if (Monitor::Inflate(objHandle.GetPtr(), thread)) {
            thread->PushLocalObjectLocked(objHandle.GetPtr());
            return Monitor::State::OK;
        }
        // Couldn't inflate.
        if (trylock) {
            return Monitor::State::ILLEGAL;
        }

        return std::nullopt;
    }

    ASSERT(thread->GetInternalId() <= MarkWord::LIGHT_LOCK_THREADID_MAX_COUNT);
    auto newMark = mark.DecodeFromLightLock(thread->GetInternalId(), 1);
    // Strong CAS as the loop iteration is large
    ASSERT(objHandle.GetPtr() != nullptr);
    auto ret = objHandle.GetPtr()->AtomicSetMark(mark, newMark);
    if (ret) {
        LOG(DEBUG, RUNTIME) << "The lightweight monitor was successfully acquired for the first time";
        Monitor::TraceMonitorLock(objHandle.GetPtr(), false);
        thread->PushLocalObjectLocked(objHandle.GetPtr());
        return Monitor::State::OK;
    }
    // Couldn't update mark.
    if (trylock) {
        return Monitor::State::ILLEGAL;
    }

    return std::nullopt;
}

Monitor::State Monitor::HandleHeavyLockedState(Monitor *monitor, MTManagedThread *thread,
                                               VMHandle<ObjectHeader> &objHandle, bool trylock)
{
    if (monitor == nullptr) {
        // Not sure if it is possible
        return Monitor::State::ILLEGAL;
    }
    bool ret = monitor->Acquire(thread, objHandle, trylock);
    if (ret) {
        thread->PushLocalObjectLocked(objHandle.GetPtr());
    }
    return ret ? Monitor::State::OK : Monitor::State::ILLEGAL;
}

/**
 * Static call, which implements the basic functionality of monitors:
 * heavyweight, lightweight and so on.
 *
 * @param  obj  an object header of corresponding object
 * @param  trylock is true if the function should fail in case of lock was already acquired by other thread
 * @return      state of function execution (ok, illegal)
 */
Monitor::State Monitor::MonitorEnter(ObjectHeader *obj, bool trylock)
{
    auto *thread = MTManagedThread::GetCurrent();
    ASSERT(thread != nullptr);
    // This function can unlock MutatorLock, so GC can run during lock acquire waiting
    // so we need to use handle to get updated header pointer
    [[maybe_unused]] HandleScope<ObjectHeader *> scope(thread);
    VMHandle<ObjectHeader> objHandle(thread, obj);
    bool shouldInflate = false;
    uint32_t lightlockRetryCount = 0;

    while (true) {
        MarkWord mark = objHandle.GetPtr()->AtomicGetMark();
        MarkWord::ObjectState state = mark.GetState();

        LOG(DEBUG, RUNTIME) << "Try to enter monitor " << std::hex << obj << "  with state " << std::dec << state;

        switch (state) {
            case MarkWord::STATE_HEAVY_LOCKED: {
                auto monitor = thread->GetMonitorPool()->LookupMonitor(mark.GetMonitorId());
                return HandleHeavyLockedState(monitor, thread, objHandle, trylock);
            }
            case MarkWord::STATE_LIGHT_LOCKED: {
                auto retState =
                    HandleLightLockedState(mark, thread, objHandle, lightlockRetryCount, shouldInflate, trylock);
                if (retState.has_value()) {
                    return retState.value();
                }
                // Go to the next iteration
                continue;
            }
            case MarkWord::STATE_HASHED:
                if (Inflate(objHandle.GetPtr(), thread)) {
                    thread->PushLocalObjectLocked(objHandle.GetPtr());
                    return State::OK;
                }
                // Couldn't inflate.
                if (trylock) {
                    return State::ILLEGAL;
                }
                // Go to the next iteration
                continue;
            case MarkWord::STATE_UNLOCKED: {
                auto retState = HandleUnlockedState(mark, thread, objHandle, shouldInflate, trylock);
                if (retState.has_value()) {
                    return retState.value();
                }
                // Go to the next iteration
                continue;
            }
            case MarkWord::STATE_GC:
                LOG(FATAL, RUNTIME) << "Not yet implemented";
                return State::ILLEGAL;
            default:
                LOG(FATAL, RUNTIME) << "Undefined object state";
                return State::ILLEGAL;
        }
    }
}

Monitor::State Monitor::MonitorExit(ObjectHeader *obj)
{
    ASSERT(obj != nullptr);
    auto thread = MTManagedThread::GetCurrent();
    bool ret = false;

    while (true) {
        MarkWord mark = obj->AtomicGetMark();
        MarkWord newMark = mark;
        MarkWord::ObjectState state = mark.GetState();
        LOG(DEBUG, RUNTIME) << "Try to exit monitor " << std::hex << obj << "  with state " << std::dec << state;
        switch (state) {
            case MarkWord::STATE_HEAVY_LOCKED: {
                auto monitor = thread->GetMonitorPool()->LookupMonitor(mark.GetMonitorId());
                ret = monitor->Release(thread);
                if (ret) {
                    thread->PopLocalObjectLocked(obj);
                }
                return ret ? State::OK : State::ILLEGAL;
            }
            case MarkWord::STATE_LIGHT_LOCKED: {
                if (mark.GetThreadId() != thread->GetInternalId()) {
                    LOG(DEBUG, RUNTIME) << "Caling MonitorEnter on object which isn't owned by this thread";
                    return State::ILLEGAL;
                }
                uint32_t newCount = mark.GetLockCount() - 1;
                if (newCount != 0) {
                    newMark = mark.DecodeFromLightLock(thread->GetInternalId(), newCount);
                } else {
                    newMark = mark.DecodeFromUnlocked();
                }
                // Strong CAS as the loop iteration is large
                if (obj->AtomicSetMark(mark, newMark)) {
                    LOG(DEBUG, RUNTIME) << "Exited lightweight lock";
                    TraceMonitorUnLock();
                    thread->PopLocalObjectLocked(obj);
                    return State::OK;
                }
                // CAS failed, must have been heavy locked by other thread. Retry unlock.
                continue;
            }
            case MarkWord::STATE_HASHED:
            case MarkWord::STATE_UNLOCKED:
                LOG(ERROR, RUNTIME) << "Try to perform monitor exit from unlocked state";
                return State::ILLEGAL;
            case MarkWord::STATE_GC:
                LOG(FATAL, RUNTIME) << "Not yet implemented";
                return State::ILLEGAL;
            default:
                LOG(FATAL, RUNTIME) << "Undefined object state";
                return State::ILLEGAL;
        }
    }
}

static inline bool DoWaitInternal(MTManagedThread *thread, ThreadStatus status, uint64_t timeout, uint64_t nanos)
    REQUIRES(*(thread->GetWaitingMutex()))
{
    bool isTimeout = false;
    if (timeout == 0 && nanos == 0) {
        // Normal wait
        thread->WaitWithLockHeld(status);
    } else {
        isTimeout = thread->TimedWaitWithLockHeld(status, timeout, nanos, false);
    }
    return isTimeout;
}

Monitor::State Monitor::WaitWithHeavyLockedState(MTManagedThread *thread, VMHandle<ObjectHeader> &objHandle,
                                                 MarkWord &mark, ThreadStatus status, uint64_t timeout, uint64_t nanos,
                                                 bool ignoreInterruption)
{
    State resultState = State::OK;
    auto monitor = thread->GetMonitorPool()->LookupMonitor(mark.GetMonitorId());
    ASSERT(monitor != nullptr);
    if (monitor->GetOwner() != thread) {
        // The monitor is acquired by other thread
        // throw an internal exception?
        LOG(ERROR, RUNTIME) << "Illegal monitor state: try to wait with monitor acquired by other thread";
        return State::ILLEGAL;
    }

    thread->GetWaitingMutex()->Lock();

    // Use LockHolder inside scope
    uint64_t counter = monitor->recursiveCounter_;
    // Wait should be called under the monitor. We checked it in the previous if.
    // Thus, the operation with queues are thread-safe
    monitor->waiters_.PushFront(*thread);
    thread->SetWaitingMonitor(monitor);
    thread->SetWaitingMonitorOldStatus(status);

    monitor->recursiveCounter_ = 1;
    // Atomic with relaxed order reason: memory access in monitor
    monitor->waitersCounter_.fetch_add(1, std::memory_order_relaxed);
    monitor->Release(thread);

    bool isTimeout = false;
    if (thread->IsInterruptedWithLockHeld() && !ignoreInterruption) {
        resultState = State::INTERRUPTED;
        thread->GetWaitingMutex()->Unlock();
        // Calling Safepoing to let GC start if needed
        // in the else branch GC can start during Wait
        ark::interpreter::RuntimeInterface::Safepoint();
    } else {
        TraceMonitorLock(objHandle.GetPtr(), true);
        isTimeout = DoWaitInternal(thread, status, timeout, nanos);
        TraceMonitorUnLock();  // End Wait().
        thread->GetWaitingMutex()->Unlock();
    }
    // Unlock WaitingMutex before to avoid deadlock
    // Nothing happen, if the thread is rescheduled between,
    // As the monitor was already released for external users
    [[maybe_unused]] bool ret = monitor->Acquire(thread, objHandle, false);
    ASSERT(ret);
    // Atomic with relaxed order reason: memory access in monitor
    monitor->waitersCounter_.fetch_sub(1, std::memory_order_relaxed);
    monitor->recursiveCounter_ = counter;

    if (thread->IsInterrupted()) {
        // NOTE(dtrubenkov): call ark::ThrowException when it will be imlemented
        resultState = State::INTERRUPTED;
    }

    // problems with equality of MTManagedThread's
    bool found = monitor->waiters_.RemoveIf(
        [thread](MTManagedThread &t) { return thread->GetInternalId() == t.GetInternalId(); });
    // If no matching thread found in waiters_, it should have been moved to to_wakeup_
    // but this thread timed out or got interrupted
    if (!found) {
        monitor->toWakeup_.RemoveIf(
            [thread](MTManagedThread &t) { return thread->GetInternalId() == t.GetInternalId(); });
    }

    thread->SetWaitingMonitor(nullptr);
    thread->SetWaitingMonitorOldStatus(ThreadStatus::FINISHED);
    Runtime::GetCurrent()->GetNotificationManager()->MonitorWaitedEvent(objHandle.GetPtr(), isTimeout);

    return resultState;
}

/** Zero timeout is used as infinite wait (see docs)
 */
Monitor::State Monitor::Wait(ObjectHeader *obj, ThreadStatus status, uint64_t timeout, uint64_t nanos,
                             bool ignoreInterruption)
{
    ASSERT(obj != nullptr);
    auto *thread = MTManagedThread::GetCurrent();
    ASSERT(thread != nullptr);
    State resultState = State::OK;

    // This function can unlock MutatorLock, so GC can run during wait
    // so we need to use handle to get updated header pointer
    [[maybe_unused]] HandleScope<ObjectHeader *> scope(thread);
    VMHandle<ObjectHeader> objHandle(thread, obj);

    Runtime::GetCurrent()->GetNotificationManager()->MonitorWaitEvent(obj, timeout);

    while (true) {
        MarkWord mark = objHandle->AtomicGetMark();
        MarkWord::ObjectState state = mark.GetState();
        LOG(DEBUG, RUNTIME) << "Try to wait with state " << state;
        switch (state) {
            case MarkWord::STATE_HEAVY_LOCKED: {
                return WaitWithHeavyLockedState(thread, objHandle, mark, status, timeout, nanos, ignoreInterruption);
            }
            case MarkWord::STATE_LIGHT_LOCKED:
                if (mark.GetThreadId() != thread->GetInternalId()) {
                    LOG(FATAL, RUNTIME) << "Illegal monitor state: try to wait with monitor acquired by other thread";
                    return resultState;
                }
                Inflate(objHandle.GetPtr(), thread);
                // Go to the next iteration.
                continue;
            case MarkWord::STATE_UNLOCKED:
            case MarkWord::STATE_HASHED:
            case MarkWord::STATE_GC:
                LOG(ERROR, RUNTIME) << "Try to perform Wait from unsupported state";
                return State::ILLEGAL;
            default:
                LOG(FATAL, RUNTIME) << "Undefined object state";
                UNREACHABLE();
        }
    }
}

Monitor::State Monitor::Notify(ObjectHeader *obj)
{
    ASSERT(obj != nullptr);
    MarkWord mark = obj->AtomicGetMark();
    MarkWord::ObjectState state = mark.GetState();
    auto thread = MTManagedThread::GetCurrent();
    LOG(DEBUG, RUNTIME) << "Try to notify with state " << state;

    switch (state) {
        case MarkWord::STATE_HEAVY_LOCKED: {
            auto monitor = thread->GetMonitorPool()->LookupMonitor(mark.GetMonitorId());
            ASSERT(monitor != nullptr);
            if (monitor->GetOwner() != thread) {
                // The monitor is acquired by other thread
                // throw an internal exception?
                LOG(ERROR, RUNTIME) << "Illegal monitor state: try to notify with monitor acquired by other thread";
                return State::ILLEGAL;
            }

            // Notify should be called under the monitor. We checked it in the previous if.
            // Thus, the operation with queues are thread-safe

            // Move one thread from waiters to wake_up
            if (!monitor->waiters_.Empty()) {
                // With current ark::List implementation this reference is valid.
                // This can be broken with future changes.
                auto &waiter = monitor->waiters_.Front();
                monitor->waiters_.PopFront();
                monitor->toWakeup_.PushFront(waiter);
            }
            return State::OK;  // Success
        }
        case MarkWord::STATE_LIGHT_LOCKED:
            if (mark.GetThreadId() != thread->GetInternalId()) {
                LOG(ERROR, RUNTIME) << "Illegal monitor state: try to notify with monitor acquired by other thread";
                return State::ILLEGAL;
            }
            return State::OK;  // Success
        case MarkWord::STATE_UNLOCKED:
        case MarkWord::STATE_HASHED:
        case MarkWord::STATE_GC:
            LOG(ERROR, RUNTIME) << "Try to perform Notify from unsupported state";
            return State::ILLEGAL;
        default:
            LOG(FATAL, RUNTIME) << "Undefined object state";
            UNREACHABLE();
    }
}

Monitor::State Monitor::NotifyAll(ObjectHeader *obj)
{
    ASSERT(obj != nullptr);
    MarkWord mark = obj->AtomicGetMark();
    MarkWord::ObjectState state = mark.GetState();
    auto thread = MTManagedThread::GetCurrent();
    LOG(DEBUG, RUNTIME) << "Try to notify all with state " << state;

    switch (state) {
        case MarkWord::STATE_HEAVY_LOCKED: {
            auto monitor = thread->GetMonitorPool()->LookupMonitor(mark.GetMonitorId());
            if (monitor->GetOwner() != thread) {
                // The monitor is acquired by other thread
                // throw an internal exception?
                LOG(ERROR, RUNTIME) << "Illegal monitor state: try to notify with monitor acquired by other thread";
                return State::ILLEGAL;
            }

            // NotifyAll should be called under the monitor. We checked it in the previous if.
            // Thus, the operation with queues are thread-safe
            if (monitor->toWakeup_.Empty()) {
                monitor->toWakeup_.Swap(monitor->waiters_);
                return State::OK;
            }

            // Concatenate two queues
            if (!monitor->waiters_.Empty()) {
                monitor->toWakeup_.Splice(monitor->waiters_);
                monitor->waiters_.Clear();
            }
            return State::OK;  // Success
        }
        case MarkWord::STATE_LIGHT_LOCKED:
            if (mark.GetThreadId() != thread->GetInternalId()) {
                LOG(ERROR, RUNTIME) << "Illegal monitor state: try to notify with monitor acquired by other thread";
                return State::ILLEGAL;
            }
            return State::OK;  // Success
        case MarkWord::STATE_UNLOCKED:
        case MarkWord::STATE_HASHED:
        case MarkWord::STATE_GC:
            LOG(ERROR, RUNTIME) << "Try to perform NotifyAll from unsupported state";
            return State::ILLEGAL;
        default:
            LOG(FATAL, RUNTIME) << "Undefined object state";
            UNREACHABLE();
    }
}

void Monitor::Acquire(MTManagedThread *thread, const VMHandle<ObjectHeader> &objHandle)
{
    Runtime::GetCurrent()->GetNotificationManager()->MonitorContendedEnterEvent(objHandle.GetPtr());
    // If not trylock...
    // Do atomic add out of scope to prevent GC getting old waiters_counter_
    // Atomic with relaxed order reason: memory access in monitor
    waitersCounter_.fetch_add(1, std::memory_order_relaxed);
    thread->SetEnterMonitorObject(objHandle.GetPtr());
    thread->SetWaitingMonitorOldStatus(ThreadStatus::IS_BLOCKED);
    {
        ScopedChangeThreadStatus ets(thread, ThreadStatus::IS_BLOCKED);
        // Save current monitor, on which the given thread is blocked.
        // It can be used to detect potential deadlock with daemon threds.
        thread->SetEnteringMonitor(this);
        lock_.Lock();
        // Deadlock is no longer possible with the given thread.
        thread->SetEnteringMonitor(nullptr);
        // Do this inside scope for thread to release this monitor during runtime destroy
        if (!this->SetOwner(nullptr, thread)) {
            LOG(FATAL, RUNTIME) << "Set monitor owner failed in Acquire";
        }
        thread->AddMonitor(this);
        this->recursiveCounter_++;
    }
    thread->SetEnterMonitorObject(nullptr);
    thread->SetWaitingMonitorOldStatus(ThreadStatus::FINISHED);
    // Atomic with relaxed order reason: memory access in monitor
    waitersCounter_.fetch_sub(1, std::memory_order_relaxed);
    // Even thout these 2 warnings are valid, We suppress them. Reason is to have consistent logging
    // Otherwise we would see that lock was done on one monitor address,
    // and unlock (after GC) - ona different one
    // SUPPRESS_CSA_NEXTLINE(alpha.core.WasteObjHeader)
    Runtime::GetCurrent()->GetNotificationManager()->MonitorContendedEnteredEvent(objHandle.GetPtr());
    LOG(DEBUG, RUNTIME) << "The fat monitor was successfully acquired for the first time";
    // SUPPRESS_CSA_NEXTLINE(alpha.core.WasteObjHeader)
    TraceMonitorLock(objHandle.GetPtr(), false);
}

bool Monitor::Acquire(MTManagedThread *thread, const VMHandle<ObjectHeader> &objHandle, bool trylock)
{
    ASSERT_MANAGED_CODE();

    MTManagedThread *owner = this->GetOwner();
    if (owner == thread) {
        // Do we need to hold a lock here?
        this->recursiveCounter_++;
        LOG(DEBUG, RUNTIME) << "The fat monitor was successfully recursively acquired";
        TraceMonitorLock(objHandle.GetPtr(), false);
        return true;
    }

    // Use trylock first
    if (trylock) {
        if (!lock_.TryLock()) {
            return false;
        }
    } else {
#ifdef PANDA_USE_FUTEX
        if (!lock_.TryLockWithSpinning()) {
#else
        if (!lock_.TryLock()) {
#endif  // PANDA_USE_FUTEX
            Acquire(thread, objHandle);
            return true;
        }
    }

    if (!this->SetOwner(nullptr, thread)) {
        LOG(FATAL, RUNTIME) << "Set monitor owner failed in Acquire";
    }
    thread->AddMonitor(this);
    this->recursiveCounter_++;
    LOG(DEBUG, RUNTIME) << "The fat monitor was successfully acquired for the first time";
    TraceMonitorLock(objHandle.GetPtr(), false);
    return true;
}

void Monitor::InitWithOwner(MTManagedThread *thread, ObjectHeader *obj)
{
    ASSERT(this->GetOwner() == nullptr);

#ifdef PANDA_USE_FUTEX
    ASSERT(thread == MTManagedThread::GetCurrent() || thread->GetStatus() != ThreadStatus::RUNNING);
    lock_.LockForOther(thread->GetId());
#else
    ASSERT(thread == MTManagedThread::GetCurrent());
    [[maybe_unused]] bool res = lock_.TryLock();
    ASSERT(res);
#endif  // PANDA_USE_FUTEX

    if (!this->SetOwner(nullptr, thread)) {
        LOG(FATAL, RUNTIME) << "Set monitor owner failed in InitWithOwner";
    }
    this->recursiveCounter_++;
    LOG(DEBUG, RUNTIME) << "The fat monitor was successfully initialized for the first time";
    TraceMonitorLock(obj, false);
}

void Monitor::ReleaseOnFailedInflate(MTManagedThread *thread)
{
    if (thread != this->GetOwner()) {
        LOG(FATAL, RUNTIME) << "Releasing lock which isn't owned by this thread";
    }
    TraceMonitorUnLock();
    this->recursiveCounter_--;
    ASSERT(this->recursiveCounter_ == 0);
    // This should never fail
    [[maybe_unused]] bool success = this->SetOwner(thread, nullptr);
    ASSERT(success);
#ifdef PANDA_USE_FUTEX
    ASSERT(thread == MTManagedThread::GetCurrent() || thread->GetStatus() != ThreadStatus::RUNNING);
    lock_.UnlockForOther(thread->GetId());
#else
    ASSERT(thread == MTManagedThread::GetCurrent());
    lock_.Unlock();
#endif  // PANDA_USE_FUTEX
    LOG(DEBUG, RUNTIME) << "The fat monitor was successfully released after failed inflation";
}

bool Monitor::Release(MTManagedThread *thread)
{
    if (thread != this->GetOwner()) {
        LOG(FATAL, RUNTIME) << "Releasing lock which isn't owned by this thread";
        return false;
    }
    TraceMonitorUnLock();
    this->recursiveCounter_--;
    if (this->recursiveCounter_ == 0) {
        if (!this->SetOwner(thread, nullptr)) {
            LOG(FATAL, RUNTIME) << "Set monitor owner failed in Release";
        }
        // Signal the only waiter (the other one will be signaled after the next release)
        MTManagedThread *waiter = nullptr;
        Monitor *waitingMon = nullptr;
        if (!this->toWakeup_.Empty()) {
            // NB! Current list implementation leaves this pointer valid after PopFront, change this
            // if List implementation is changed.
            waiter = &(this->toWakeup_.Front());
            waitingMon = waiter->GetWaitingMonitor();
            this->toWakeup_.PopFront();
        }
        thread->RemoveMonitor(this);
        // Signal waiter after mutex unlock so that signalled thread doesn't get stuck on lock_
        if (waiter != nullptr && waitingMon == this) {
            waiter->Signal();
            LOG(DEBUG, RUNTIME) << "Send the notifing signal to " << waiter->GetId();
        }
        lock_.Unlock();
    }
    LOG(DEBUG, RUNTIME) << "The fat monitor was successfully released";
    return true;
}

bool Monitor::TryAddMonitor(Monitor *monitor, MarkWord &oldMark, MTManagedThread *thread, ObjectHeader *obj,
                            MonitorPool *monitorPool)
{
    MarkWord newMark = oldMark.DecodeFromMonitor(monitor->GetId());
    bool ret = obj->AtomicSetMark(oldMark, newMark);
    if (!ret) {
        // Means, someone changed the mark
        monitor->recursiveCounter_ = 1;
        monitor->ReleaseOnFailedInflate(thread);
        monitorPool->FreeMonitor(monitor->GetId());
    } else {
        // Unlike normal Acquire, AddMonitor should be done not in InitWithOwner but after successful inflation to avoid
        // data race
        thread->AddMonitor(monitor);
    }

    return ret;
}

template <bool FOR_OTHER_THREAD>
bool Monitor::Inflate(ObjectHeader *obj, MTManagedThread *thread)
{
    ASSERT(obj != nullptr);
    MarkWord oldMark = obj->AtomicGetMark();
    MarkWord::ObjectState state = oldMark.GetState();

    // Dont inflate if someone already inflated the lock.
    if (state == MarkWord::STATE_HEAVY_LOCKED) {
        return false;
    }
    // NOLINTNEXTLINE(readability-braces-around-statements, hicpp-braces-around-statements)
    if constexpr (FOR_OTHER_THREAD) {  // NOLINT(bugprone-suspicious-semicolon)
        // Dont inflate if monitor got unlocked or acquired by other thread.
        if (state != MarkWord::STATE_LIGHT_LOCKED || oldMark.GetThreadId() != thread->GetInternalId()) {
            return false;
        }
    }

    auto *monitorPool = thread->GetMonitorPool();
    auto *monitor = monitorPool->CreateMonitor(obj);
    if (monitor == nullptr) {
        LOG(FATAL, RUNTIME) << "Couldn't create new monitor. Out of memory?";
        return false;
    }
    monitor->InitWithOwner(thread, obj);

    switch (state) {
        case MarkWord::STATE_LIGHT_LOCKED:
            if (oldMark.GetThreadId() != thread->GetInternalId()) {
                monitor->ReleaseOnFailedInflate(thread);
                monitorPool->FreeMonitor(monitor->GetId());
                return false;
            }
            monitor->recursiveCounter_ = oldMark.GetLockCount();
            break;
        case MarkWord::STATE_HASHED:
            monitor->SetHashCode(oldMark.GetHash());
            /* fallthrough */
            [[fallthrough]];
        case MarkWord::STATE_UNLOCKED:
            // NOLINTNEXTLINE(readability-braces-around-statements, hicpp-braces-around-statements)
            if constexpr (FOR_OTHER_THREAD) {  // NOLINT(bugprone-suspicious-semicolon)
                // We did check above, has to be unreachable
                UNREACHABLE();
            } else {  // NOLINT(readability-misleading-indentation)
                break;
            }
        case MarkWord::STATE_HEAVY_LOCKED:
            // Has to be unreachable
            UNREACHABLE();
        case MarkWord::STATE_GC:
            LOG(FATAL, RUNTIME) << "Trying to inflate object in GC state";
            return false;
        default:
            LOG(FATAL, RUNTIME) << "Undefined object state";
            return false;
    }

    return TryAddMonitor(monitor, oldMark, thread, obj, monitorPool);
}

bool Monitor::Deflate(ObjectHeader *obj)
{
    Monitor *monitor = nullptr;
    MarkWord oldMark = obj->AtomicGetMark();
    MarkWord::ObjectState state = oldMark.GetState();
    bool ret = false;

    if (state != MarkWord::STATE_HEAVY_LOCKED) {
        LOG(DEBUG, RUNTIME) << "Trying to deflate non-heavy locked object";
        return false;
    }
    auto *thread = MTManagedThread::GetCurrent();
    ASSERT(thread != nullptr);
    auto *monitorPool = thread->GetMonitorPool();
    monitor = monitorPool->LookupMonitor(oldMark.GetMonitorId());
    if (monitor == nullptr) {
        LOG(DEBUG, RUNTIME) << "Monitor was already destroyed by someone else.";
        return false;
    }

    ret = monitor->DeflateInternal();
    if (ret) {
        monitorPool->FreeMonitor(monitor->GetId());
    }
    return ret;
}

bool Monitor::DeflateInternal()
{
    if (GetOwner() != nullptr) {
        LOG(DEBUG, RUNTIME) << "Trying to deflate monitor which already has owner";
        return false;
    }
    // Atomic with relaxed order reason: memory access in monitor
    if (waitersCounter_.load(std::memory_order_relaxed) > 0) {
        LOG(DEBUG, RUNTIME) << "Trying to deflate monitor which is trying to be acquired by other threads";
        return false;
    }
    if (!lock_.TryLock()) {
        LOG(DEBUG, RUNTIME) << "Couldn't TryLock monitor for deflation";
        return false;
    }
    ASSERT(obj_ != nullptr);
    ASSERT(recursiveCounter_ == 0);
    ASSERT(waiters_.Empty());
    ASSERT(toWakeup_.Empty());
    ASSERT(GetOwner() == static_cast<MTManagedThread *>(nullptr));
    MarkWord oldMark = obj_->AtomicGetMark();
    MarkWord newMark = oldMark;
    if (HasHashCode()) {
        newMark = oldMark.DecodeFromHash(GetHashCode());
        LOG(DEBUG, RUNTIME) << "Deflating monitor to hash";
    } else {
        newMark = oldMark.DecodeFromUnlocked();
        LOG(DEBUG, RUNTIME) << "Deflating monitor to unlocked";
    }

    // Warning: AtomicSetMark is weak, retry
    while (!obj_->AtomicSetMark<false>(oldMark, newMark)) {
        newMark = HasHashCode() ? oldMark.DecodeFromHash(GetHashCode()) : oldMark.DecodeFromUnlocked();
    }
    lock_.Unlock();
    return true;
}

uint8_t Monitor::HoldsLock(ObjectHeader *obj)
{
    MarkWord mark = obj->AtomicGetMark();
    MarkWord::ObjectState state = mark.GetState();
    MTManagedThread *thread = MTManagedThread::GetCurrent();
    ASSERT(thread != nullptr);

    switch (state) {
        case MarkWord::STATE_HEAVY_LOCKED: {
            Monitor *monitor = thread->GetMonitorPool()->LookupMonitor(mark.GetMonitorId());
            ASSERT(monitor != nullptr);
            // asm has no boolean type
            return (monitor->GetOwner() == thread) ? 1 : 0;
        }
        case MarkWord::STATE_LIGHT_LOCKED:
            return (mark.GetThreadId() == thread->GetInternalId()) ? 1 : 0;
        case MarkWord::STATE_UNLOCKED:
        case MarkWord::STATE_HASHED:
        case MarkWord::STATE_GC:
            return 0;
        default:
            LOG(FATAL, RUNTIME) << "Undefined object state";
            return 0;
    }
}

uint32_t Monitor::GetLockOwnerOsThreadID(ObjectHeader *obj)
{
    if (obj == nullptr) {
        return MTManagedThread::NON_INITIALIZED_THREAD_ID;
    }
    MarkWord mark = obj->AtomicGetMark();
    MarkWord::ObjectState state = mark.GetState();

    switch (state) {
        case MarkWord::STATE_HEAVY_LOCKED: {
            auto *thread = MTManagedThread::GetCurrent();
            ASSERT(thread != nullptr);
            Monitor *monitor = thread->GetMonitorPool()->LookupMonitor(mark.GetMonitorId());
            ASSERT(monitor != nullptr);
            MTManagedThread *owner = monitor->GetOwner();
            if (owner == nullptr) {
                return MTManagedThread::NON_INITIALIZED_THREAD_ID;
            }
            return owner->GetId();
        }
        case MarkWord::STATE_LIGHT_LOCKED: {
            return mark.GetThreadId();
        }
        case MarkWord::STATE_UNLOCKED:
        case MarkWord::STATE_HASHED:
        case MarkWord::STATE_GC:
            return 0;
        default:
            LOG(FATAL, RUNTIME) << "Undefined object state";
            return 0;
    }
}

Monitor *Monitor::GetMonitorFromObject(ObjectHeader *obj)
{
    if (obj != nullptr) {
        MarkWord mark = obj->AtomicGetMark();
        MarkWord::ObjectState state = mark.GetState();
        auto *thread = MTManagedThread::GetCurrent();
        ASSERT(thread != nullptr);
        switch (state) {
            case MarkWord::STATE_HEAVY_LOCKED:
                return thread->GetMonitorPool()->LookupMonitor(mark.GetMonitorId());
            case MarkWord::STATE_LIGHT_LOCKED:
                return nullptr;
            default:
                // Shouldn't happen, return nullptr
                LOG(WARNING, RUNTIME) << "obj:" << obj << " not locked by heavy or light locked";
        }
    }
    return nullptr;
}

inline void Monitor::TraceMonitorLock([[maybe_unused]] ObjectHeader *obj, bool isWait)
{
    if (UNLIKELY(ark::trace::IsEnabled())) {
        // Use stack memory to avoid "Too many allocations" error.
        constexpr int BUF_SIZE = 32;
        std::array<char, BUF_SIZE> buf = {};
// For Security reasons, optionally hide address based on build target
#if !defined(PANDA_TARGET_OHOS) || !defined(NDEBUG)
        // NOLINTNEXTLINE(cppcoreguidelines-pro-type-vararg)
        int ret = snprintf_s(buf.data(), BUF_SIZE, BUF_SIZE - 1,
                             (isWait ? "Waiting on 0x%" PRIxPTR : "Locking 0x%" PRIxPTR), ToUintPtr(obj));
#else
        // NOLINTNEXTLINE(cppcoreguidelines-pro-type-vararg)
        int ret = snprintf_s(buf.data(), BUF_SIZE, BUF_SIZE - 1, (isWait ? "Waiting on object" : "Locking object"));
#endif
        if (ret < 0) {
            UNREACHABLE();
        }
        trace::BeginTracePoint(buf.data());
    }
}

inline void Monitor::TraceMonitorUnLock()
{
    if (UNLIKELY(ark::trace::IsEnabled())) {
        trace::EndTracePoint();
    }
}

uint32_t Monitor::GetHashCode()
{
    while (!HasHashCode()) {
        uint32_t expected = 0;
        uint32_t newHash = ObjectHeader::GenerateHashCode();
        if (hashCode_.compare_exchange_weak(expected, newHash)) {
            return newHash;
        }
    }
    ASSERT(HasHashCode());
    // Atomic with relaxed order reason: memory access in monitor
    return hashCode_.load(std::memory_order_relaxed);
}

bool Monitor::HasHashCode() const
{
    // Atomic with relaxed order reason: memory access in monitor
    return hashCode_.load(std::memory_order_relaxed) != 0;
}

void Monitor::SetHashCode(uint32_t hash)
{
    ASSERT(GetOwner() == MTManagedThread::GetCurrent());
    if (!HasHashCode()) {
        // Atomic with relaxed order reason: memory access in monitor
        hashCode_.store(hash, std::memory_order_relaxed);
    } else {
        LOG(FATAL, RUNTIME) << "Attempt to rewrite hash in monitor";
    }
}
}  // namespace ark
