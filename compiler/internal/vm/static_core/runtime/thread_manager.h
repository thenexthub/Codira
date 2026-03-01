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
#ifndef PANDA_RUNTIME_THREAD_MANAGER_H_
#define PANDA_RUNTIME_THREAD_MANAGER_H_

#include <bitset>

#include "libarkbase/os/mutex.h"
#include "libarkbase/utils/time.h"
#include "libarkbase/os/time.h"
#include "runtime/include/coretypes/array-inl.h"
#include "runtime/include/mem/panda_containers.h"
#include "runtime/include/mem/panda_smart_pointers.h"
#include "runtime/include/mtmanaged_thread.h"
#include "runtime/include/thread_status.h"
#include "runtime/include/locks.h"

namespace ark {

// This interval is required for waiting for threads to stop.
static const int WAIT_INTERVAL = 10;
static constexpr int64_t K_MAX_DUMP_TIME_NS = UINT64_C(6 * 1000 * 1000 * 1000);   // 6s
static constexpr int64_t K_MAX_SINGLE_DUMP_TIME_NS = UINT64_C(50 * 1000 * 1000);  // 50ms

enum class EnumerationFlag {
    NONE = 0,                 // Nothing
    NON_CORE_THREAD = 1,      // Plugin type thread
    MANAGED_CODE_THREAD = 2,  // Thread which can execute managed code
    VM_THREAD = 4,            // Includes VM threads
    ALL = 8,                  // See the comment in the function SatisfyTheMask below
};

class ThreadManager {
public:
    NO_COPY_SEMANTIC(ThreadManager);
    NO_MOVE_SEMANTIC(ThreadManager);

    using Callback = std::function<bool(ManagedThread *)>;

    explicit ThreadManager() = default;
    virtual ~ThreadManager() = default;

    /**
     * @brief thread enumeration and applying @param cb to them
     * @return true if for every thread @param cb was successful (returned true) and false otherwise
     */
    bool EnumerateThreads(const Callback &cb, unsigned int incMask = static_cast<unsigned int>(EnumerationFlag::ALL),
                          unsigned int xorMask = static_cast<unsigned int>(EnumerationFlag::NONE)) const
    {
        return EnumerateThreadsImpl(cb, incMask, xorMask);
    }

    virtual void WaitForDeregistration() = 0;

    virtual void SuspendAllThreads() = 0;
    virtual void ResumeAllThreads() = 0;

    virtual bool IsRunningThreadExist() = 0;

    ManagedThread *GetMainThread() const
    {
        return mainThread_;
    }

    void SetMainThread(ManagedThread *thread)
    {
        mainThread_ = thread;
    }

protected:
    bool SatisfyTheMask(ManagedThread *t, unsigned int mask) const
    {
        if ((mask & static_cast<unsigned int>(EnumerationFlag::ALL)) != 0) {
            // Some uninitialized threads may not have attached flag,
            // So, they are not included as MANAGED_CODE_THREAD.
            // Newly created threads are using flag suspend new count.
            // The case leads to deadlocks, when the thread can not be resumed.
            // To deal with it, just add a specific ALL case
            return true;
        }

        // For NONE mask
        bool target = false;
        if ((mask & static_cast<unsigned int>(EnumerationFlag::MANAGED_CODE_THREAD)) != 0) {
            target = t->IsAttached();
            if ((mask & static_cast<unsigned int>(EnumerationFlag::NON_CORE_THREAD)) != 0) {
                // Due to hyerarhical structure, we need to conjunct types
                bool nonCoreThread = t->GetThreadLang() != ark::panda_file::SourceLang::PANDA_ASSEMBLY;
                target = target && nonCoreThread;
            }
        }

        if ((mask & static_cast<unsigned int>(EnumerationFlag::VM_THREAD)) != 0) {
            target = target || t->IsVMThread();
        }

        return target;
    }

    bool ApplyCallbackToThread(const Callback &cb, ManagedThread *t, unsigned int incMask, unsigned int xorMask) const
    {
        bool incTarget = SatisfyTheMask(t, incMask);
        bool xorTarget = SatisfyTheMask(t, xorMask);
        if (incTarget != xorTarget) {
            if (!cb(t)) {
                return false;
            }
        }
        return true;
    }

    virtual bool EnumerateThreadsImpl(const Callback &cb, unsigned int incMask, unsigned int xorMask) const = 0;

private:
    ManagedThread *mainThread_ {nullptr};
};

class MTThreadManager : public ThreadManager {
public:
    NO_COPY_SEMANTIC(MTThreadManager);
    NO_MOVE_SEMANTIC(MTThreadManager);

    // For performance reasons don't exceed specified amount of bits.
    static constexpr size_t MAX_INTERNAL_THREAD_ID = std::min(0xffffU, ManagedThread::MAX_INTERNAL_THREAD_ID);

    explicit MTThreadManager(mem::InternalAllocatorPtr allocator);

    ~MTThreadManager() override;

    bool EnumerateThreadsWithLockheld(const Callback &cb,
                                      unsigned int incMask = static_cast<unsigned int>(EnumerationFlag::ALL),
                                      unsigned int xorMask = static_cast<unsigned int>(EnumerationFlag::NONE)) const
    // REQUIRES(*GetThreadsLock())
    // Cannot enable the annotation, as the function is also called with thread_lock directly
    {
        for (auto t : GetThreadsList()) {
            if (!ApplyCallbackToThread(cb, t, incMask, xorMask)) {
                return false;
            }
        }
        return true;
    }

    template <class Callback>
    void EnumerateThreadsForDump(const Callback &cb, std::ostream &os)
    {
        // NOTE(00510180 & 00537420) can not get WriteLock() when other thread run code "while {}"
        // issue #3085
        SuspendAllThreads();
        MTManagedThread *self = MTManagedThread::GetCurrent();
        ASSERT(self != nullptr);
        self->GetMutatorLock()->WriteLock();
        {
            os << "ARK THREADS (" << threadsCount_ << "):\n";
        }
        os::memory::LockHolder lock(threadLock_);
        int64_t start = ark::os::time::GetClockTimeInThreadCpuTime();
        int64_t end;
        int64_t lastTime = start;
        cb(self, os);
        for (const auto &thread : threads_) {
            if (thread == self) {
                continue;
            }
            cb(thread, os);
            end = ark::os::time::GetClockTimeInThreadCpuTime();
            if ((end - lastTime) > K_MAX_SINGLE_DUMP_TIME_NS) {
                LOG(ERROR, RUNTIME) << "signal catcher: thread_list_dump thread : " << thread->GetId()
                                    << "timeout : " << (end - lastTime);
            }
            lastTime = end;
            if ((end - start) > K_MAX_DUMP_TIME_NS) {
                LOG(ERROR, RUNTIME) << "signal catcher: thread_list_dump timeout : " << end - start << "\n";
                break;
            }
        }
        DumpUnattachedThreads(os);
        self->GetMutatorLock()->Unlock();
        ResumeAllThreads();
    }

    void DumpUnattachedThreads(std::ostream &os);

    void RegisterThread(MTManagedThread *thread);

    void IncPendingThreads()
    {
        os::memory::LockHolder lock(threadLock_);
        pendingThreads_++;
    }

    void DecPendingThreads()
    {
        os::memory::LockHolder lock(threadLock_);
        pendingThreads_--;
    }

    void AddDaemonThread()
    {
        daemonThreadsCount_++;
    }

    int GetThreadsCount();

#ifndef NDEBUG
    uint32_t GetAllRegisteredThreadsCount();
#endif  // NDEBUG

    void WaitForDeregistration() override;

    void SuspendAllThreads() override;
    void ResumeAllThreads() override;

    uint32_t GetInternalThreadId();

    void RemoveInternalThreadId(uint32_t id);

    bool IsRunningThreadExist() override;

    // Returns true if unregistration succeeded; for now it can fail when we are trying to unregister main thread
    bool UnregisterExitedThread(MTManagedThread *thread);

    MTManagedThread *SuspendAndWaitThreadByInternalThreadId(uint32_t threadId);

    void RegisterSensitiveThread() const;

    os::memory::Mutex *GetThreadsLock()
    {
        return &threadLock_;
    }

protected:
    bool EnumerateThreadsImpl(const Callback &cb, unsigned int incMask, unsigned int xorMask) const override
    {
        os::memory::LockHolder lock(*GetThreadsLock());
        return EnumerateThreadsWithLockheld(cb, incMask, xorMask);
    }

    // The methods are used only in EnumerateThreads in mt mode
    const PandaList<MTManagedThread *> &GetThreadsList() const
    {
        return threads_;
    }
    os::memory::Mutex *GetThreadsLock() const
    {
        return &threadLock_;
    }

private:
    bool HasNoActiveThreads() const REQUIRES(threadLock_)
    {
        ASSERT(threadsCount_ >= daemonThreadsCount_);
        auto thread = static_cast<uint32_t>(threadsCount_ - daemonThreadsCount_);
        return thread < 2 && pendingThreads_ == 0;
    }

    bool StopThreadsOnTerminationLoops(MTManagedThread *current) REQUIRES(threadLock_);

    /**
     * Tries to stop all daemon threads in case there are no active basic threads
     * returns false if we need to wait
     */
    void StopDaemonThreads() REQUIRES(threadLock_);

    /**
     * Deregister all suspended threads including daemon threads.
     * Returns true on success and false otherwise.
     */
    bool DeregisterSuspendedThreads() REQUIRES(threadLock_);

    void DecreaseCountersForThread(MTManagedThread *thread) REQUIRES(threadLock_);

    MTManagedThread *GetThreadByInternalThreadIdWithLockHeld(uint32_t threadId) REQUIRES(threadLock_);

    bool CanDeregister(enum ThreadStatus status)
    {
        // Deregister thread only for IS_TERMINATED_LOOP.
        // In all other statuses we should wait:
        // * CREATED - wait until threads finish initializing which requires communication with ThreadManager;
        // * BLOCKED - it means we are trying to acquire lock in Monitor, which was created in internalAllocator;
        // * TERMINATING - threads which requires communication with Runtime;
        // * FINISHED threads should be deleted itself;
        // * NATIVE threads are either go to FINISHED status or considered a part of a deadlock;
        // * other statuses - should eventually go to IS_TERMINATED_LOOP or FINISHED status.
        return status == ThreadStatus::IS_TERMINATED_LOOP;
    }

    mutable os::memory::Mutex threadLock_;
    // Counter used to suspend newly created threads after SuspendAllThreads/SuspendDaemonThreads
    uint32_t suspendNewCount_ GUARDED_BY(threadLock_) = 0;
    // We should delete only finished thread structures, so call delete explicitly on finished threads
    // and don't touch other pointers
    PandaList<MTManagedThread *> threads_ GUARDED_BY(threadLock_);
    os::memory::Mutex idsLock_;
    std::bitset<MAX_INTERNAL_THREAD_ID> internalThreadIds_ GUARDED_BY(idsLock_);
    uint32_t lastId_ GUARDED_BY(idsLock_);
    PandaList<MTManagedThread *> daemonThreads_;

    os::memory::ConditionVariable stopVar_;
    std::atomic_uint32_t threadsCount_ = 0;
#ifndef NDEBUG
    // This field is required for counting all registered threads (including finished daemons)
    // in AttachThreadTest. It is not needed in production mode.
    std::atomic_uint32_t registeredThreadsCount_ = 0;
#endif  // NDEBUG
    std::atomic_uint32_t daemonThreadsCount_ = 0;
    // A specific counter of threads, which are not completely created
    // When the counter != 0, operations with thread set are permitted to avoid destruction of shared data (mutexes)
    // Synchronized with lock (not atomic) for mutual exclusion with thread operations
    int pendingThreads_ GUARDED_BY(threadLock_);
};

}  // namespace ark

#endif  // PANDA_RUNTIME_THREAD_MANAGER_H_
