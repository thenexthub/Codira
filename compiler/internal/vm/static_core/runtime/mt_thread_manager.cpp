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

#include "libarkbase/os/native_stack.h"
#include "libarkbase/os/thread.h"
#include "libarkbase/utils/logger.h"
#include "libarkbase/utils/utf.h"
#include "runtime/include/mem/allocator.h"
#include "runtime/include/panda_vm.h"
#include "runtime/include/runtime.h"
#include "runtime/include/thread-inl.h"
#include "runtime/include/thread_scopes.h"
#include "runtime/lock_order_graph.h"
#include "runtime/thread_manager.h"

namespace ark {
MTThreadManager::MTThreadManager(mem::InternalAllocatorPtr allocator) : threads_(allocator->Adapter())
{
    lastId_ = 0;
    pendingThreads_ = 0;
}

MTThreadManager::~MTThreadManager()
{
    threads_.clear();
}

uint32_t MTThreadManager::GetInternalThreadId()
{
    os::memory::LockHolder lock(idsLock_);
    for (size_t i = 0; i < internalThreadIds_.size(); i++) {
        lastId_ = (lastId_ + 1) % internalThreadIds_.size();
        if (!internalThreadIds_[lastId_]) {
            internalThreadIds_.set(lastId_);
            return lastId_ + 1;  // 0 is reserved as uninitialized value.
        }
    }
    LOG(FATAL, RUNTIME) << "Out of internal thread ids";
    UNREACHABLE();
}

void MTThreadManager::RemoveInternalThreadId(uint32_t id)
{
    id--;  // 0 is reserved as uninitialized value.
    os::memory::LockHolder lock(idsLock_);
    ASSERT(internalThreadIds_[id]);
    internalThreadIds_.reset(id);
}

MTManagedThread *MTThreadManager::GetThreadByInternalThreadIdWithLockHeld(uint32_t threadId)
{
    // Do not optimize with std::find_if - sometimes there are problems with incorrect memory accesses
    for (auto thread : threads_) {
        if (thread->GetInternalId() == threadId) {
            return thread;
        }
    }
    return nullptr;
}

bool MTThreadManager::DeregisterSuspendedThreads()
{
    auto current = MTManagedThread::GetCurrent();
    auto i = threads_.begin();
    bool isPotentiallyBlockedThreadPresent = false;
    bool isNonblockedThreadPresent = false;
    while (i != threads_.end()) {
        MTManagedThread *thread = *i;
        auto status = thread->GetStatus();
        // Do not deregister current thread (which should be in status NATIVE) as HasNoActiveThreads
        // assumes it stays registered.
        if (thread == current) {
            i++;
            continue;
        }
        // Only threads in IS_TERMINATED_LOOP status can be deregistered.
        if (CanDeregister(status)) {
            DecreaseCountersForThread(thread);
            i = threads_.erase(i);
            continue;
        }
        if (status == ThreadStatus::NATIVE || status == ThreadStatus::IS_BLOCKED) {
            // We have a blocked thread - there is a potential termination loop
            isPotentiallyBlockedThreadPresent = true;
        } else {
            // We have at least one non-blocked thread - termination loop is impossible
            isNonblockedThreadPresent = true;
        }
        LOG(DEBUG, RUNTIME) << "Daemon thread " << thread->GetId()
                            << " remains in DeregisterSuspendedThreads, status = "
                            << ManagedThread::ThreadStatusAsString(status);
        i++;
    }
    if (isPotentiallyBlockedThreadPresent && !isNonblockedThreadPresent) {
        // All threads except current are blocked (have BLOCKED or NATIVE status)
        LOG(DEBUG, RUNTIME) << "Potential termination loop with daemon threads is detected";
        return StopThreadsOnTerminationLoops(current);
    }
    // Sanity check, we should get at least current thread in that list.
    ASSERT(!threads_.empty());
    return threads_.size() == 1;
}

void MTThreadManager::DecreaseCountersForThread(MTManagedThread *thread)
{
    if (thread->IsDaemon()) {
        daemonThreadsCount_--;
        // Do not delete this thread structure as it may be used by suspended thread
        daemonThreads_.push_back(thread);
    }
    threadsCount_--;
}

bool MTThreadManager::StopThreadsOnTerminationLoops(MTManagedThread *current)
{
    if (!LockOrderGraph::CheckForTerminationLoops(threads_, daemonThreads_, current)) {
        LOG(DEBUG, RUNTIME) << "Termination loop with daemon threads was not confirmed";
        return false;
    }

    os::memory::Mutex::IgnoreChecksOnTerminationLoop();
    auto i = threads_.begin();
    while (i != threads_.end()) {
        MTManagedThread *thread = *i;
        if (thread != current) {
            DecreaseCountersForThread(thread);
            i = threads_.erase(i);
            continue;
        }
        i++;
    }
    return true;
}

void MTThreadManager::WaitForDeregistration()
{
    trace::ScopedTrace scopedTrace(__FUNCTION__);
    {
        os::memory::LockHolder lock(threadLock_);

        // First wait for non-daemon threads to finish
        while (!HasNoActiveThreads()) {
            stopVar_.TimedWait(&threadLock_, WAIT_INTERVAL);
        }

        // Then stop daemon threads
        StopDaemonThreads();

        // Finally wait until all threads are suspended
        while (true) {
            if (pendingThreads_ != 0) {
                // There are threads, which are not completely registered
                // We can not destroy other threads, as they may use shared data (waiting mutexes)
                stopVar_.TimedWait(&threadLock_, WAIT_INTERVAL);
                continue;
            }
            if (DeregisterSuspendedThreads()) {
                break;
            }
            stopVar_.TimedWait(&threadLock_, WAIT_INTERVAL);
        }
    }
    for (const auto &thread : daemonThreads_) {
        thread->FreeInternalMemory();
    }
    auto threshold = Runtime::GetOptions().GetIgnoreDaemonMemoryLeaksThreshold();
    Runtime::GetCurrent()->SetDaemonMemoryLeakThreshold(daemonThreads_.size() * threshold);
    Runtime::GetCurrent()->SetDaemonThreadsCount(daemonThreads_.size());
}

void MTThreadManager::StopDaemonThreads() REQUIRES(threadLock_)
{
    trace::ScopedTrace scopedTrace(__FUNCTION__);
    for (auto thread : threads_) {
        if (thread->IsDaemon()) {
            LOG(DEBUG, RUNTIME) << "Stopping daemon thread " << thread->GetId();
            /* @sync 1
             * @description The thread manager will request the daemon thread to go into the termination loop after this
             * point.
             * */
            thread->StopDaemonThread();
        }
    }
    // Suspend any future new threads
    suspendNewCount_++;
}

int MTThreadManager::GetThreadsCount()
{
    return threadsCount_;
}

#ifndef NDEBUG
uint32_t MTThreadManager::GetAllRegisteredThreadsCount()
{
    return registeredThreadsCount_;
}
#endif  // NDEBUG

void MTThreadManager::SuspendAllThreads()
{
    trace::ScopedTrace scopedTrace("Suspending mutator threads");
    auto curThread = MTManagedThread::GetCurrent();
    os::memory::LockHolder lock(threadLock_);
    EnumerateThreadsWithLockheld([curThread](ManagedThread *thread) {
        if (thread != curThread) {
            thread->SuspendImpl(true);
        }
        return true;
    });
    suspendNewCount_++;
}

bool MTThreadManager::IsRunningThreadExist()
{
    auto curThread = MTManagedThread::GetCurrent();
    os::memory::LockHolder lock(threadLock_);
    bool isExists = false;
    EnumerateThreadsWithLockheld([curThread, &isExists](ManagedThread *thread) {
        if (thread != curThread) {
            if (thread->GetStatus() == ThreadStatus::RUNNING) {
                isExists = true;
                return false;
            };
        }
        return true;
    });
    return isExists;
}

void MTThreadManager::ResumeAllThreads()
{
    trace::ScopedTrace scopedTrace("Resuming mutator threads");
    auto curThread = MTManagedThread::GetCurrent();
    os::memory::LockHolder lock(threadLock_);
    if (suspendNewCount_ > 0) {
        suspendNewCount_--;
    }
    EnumerateThreadsWithLockheld([curThread](ManagedThread *thread) {
        if (thread != curThread) {
            thread->ResumeImpl(true);
        }
        return true;
    });
}

void MTThreadManager::RegisterThread(MTManagedThread *thread)
{
    os::memory::LockHolder lock(threadLock_);
    thread->GetVM()->GetGC()->OnThreadCreate(thread);
    threadsCount_++;
#ifndef NDEBUG
    registeredThreadsCount_++;
#endif  // NDEBUG
    threads_.emplace_back(thread);
    for (uint32_t i = suspendNewCount_; i > 0; i--) {
        thread->SuspendImpl(true);
    }
}

bool MTThreadManager::UnregisterExitedThread(MTManagedThread *thread)
{
    ASSERT(MTManagedThread::GetCurrent() == thread);
    {
        thread->NativeCodeEnd();

        os::memory::LockHolder lock(threadLock_);
        // While this thread is suspended, do not delete it as other thread can be accessing it.
        // TestAllFlags is required because termination request can be sent while thread_lock_ is unlocked
        while (thread->TestAllFlags()) {
            threadLock_.Unlock();
            thread->SafepointPoll();
            threadLock_.Lock();
        }

        thread->CollectTLABMetrics();
        thread->ClearTLAB();
        thread->DestroyInternalResources();

        LOG(DEBUG, RUNTIME) << "Stopping thread " << thread->GetId();
        thread->UpdateStatus(ThreadStatus::FINISHED);
        // Do not delete main thread, Runtime::GetMainThread is expected to always return valid object
        if (thread == GetMainThread()) {
            return false;
        }

        // This code should happen after thread has been resumed: Both WaitSuspension and ResumeImps requires locking
        // suspend_lock_, so it acts as a memory barrier; flag clean should be visible in this thread after exit from
        // WaitSuspenion
        thread->MakeTSANHappyForThreadState();

        threads_.remove(thread);
        if (thread->IsDaemon()) {
            daemonThreadsCount_--;
        }
        threadsCount_--;

        // If managed_thread, its nativePeer should be 0 before
        delete thread;
        stopVar_.Signal();
        return true;
    }
}

void MTThreadManager::RegisterSensitiveThread() const
{
    LOG(INFO, RUNTIME) << __func__ << " is an empty implementation now.";
}

void MTThreadManager::DumpUnattachedThreads(std::ostream &os)
{
    os::native_stack::DumpUnattachedThread dump;
    dump.InitKernelTidLists();
    os::memory::LockHolder lock(threadLock_);
    for (const auto &thread : threads_) {
        dump.AddTid(static_cast<pid_t>(thread->GetId()));
    }
    dump.Dump(os, Runtime::GetCurrent()->IsDumpNativeCrash(), Runtime::GetCurrent()->GetUnwindStackFn());
}

MTManagedThread *MTThreadManager::SuspendAndWaitThreadByInternalThreadId(uint32_t threadId)
{
    static constexpr uint32_t YIELD_ITERS = 500;
    // NB! Expected to be called in registered thread, change implementation if this function used elsewhere
    MTManagedThread *current = MTManagedThread::GetCurrent();
    MTManagedThread *suspended = nullptr;
    ASSERT(current != nullptr);
    ASSERT(current->GetStatus() != ThreadStatus::RUNNING);

    // Extract target thread
    while (true) {
        // If two threads call SuspendAndWaitThreadByInternalThreadId concurrently, one has to get suspended
        // while other waits for thread to be suspended, so thread_lock_ is required to be held until
        // SuspendImpl is called
        current->SafepointPoll();
        {
            os::memory::LockHolder lock(threadLock_);

            suspended = GetThreadByInternalThreadIdWithLockHeld(threadId);
            if (UNLIKELY(suspended == nullptr)) {
                // no thread found, exit
                return nullptr;
            }
            ASSERT(current != suspended);
            if (LIKELY(!current->IsSuspended())) {
                suspended->SuspendImpl(true);
                break;
            }
            // Unsafe to suspend as other thread may be waiting for this thread to suspend;
            // Should get suspended on Safepoint()
        }
    }

    // Now wait until target thread is really suspended
    for (uint32_t loopIter = 0;; loopIter++) {
        if (suspended->GetStatus() != ThreadStatus::RUNNING) {
            // Thread is suspended now
            return suspended;
        }
        if (loopIter < YIELD_ITERS) {
            MTManagedThread::Yield();
        } else {
            static constexpr uint32_t SHORT_SLEEP_MS = 1;
            os::thread::NativeSleep(SHORT_SLEEP_MS);
        }
    }
    UNREACHABLE();
}

}  // namespace ark
