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
#ifndef PANDA_RUNTIME_MONITOR_H_
#define PANDA_RUNTIME_MONITOR_H_

#include <atomic>
#include <optional>

#include "libarkbase/os/mutex.h"
#include "libarkbase/utils/list.h"
#include "libarkbase/utils/logger.h"
#include "runtime/include/thread_status.h"

namespace ark {

class MTManagedThread;
class ObjectHeader;
template <class T>
class VMHandle;
class MarkWord;
class MonitorPool;

/// To avoid inheritance in the `Thread` class we don't use `List` (it forces list element to inherit `ListNode`).
template <typename T>
class ThreadList {
public:
    bool Empty() const
    {
        return head_ == nullptr;
    }

    T &Front()
    {
        return *head_;
    }

    void PopFront();

    void PushFront(T &thread);

    void EraseAfter(T *prev, T *current);

    void Swap(ThreadList &other)
    {
        std::swap(head_, other.head_);
    }

    void Splice(ThreadList &other);

    void Clear()
    {
        head_ = nullptr;
    }

    template <typename Predicate>
    bool RemoveIf(Predicate pred);

private:
    T *head_ {nullptr};
};

// NOTE(aWX851037): open questions for implementation:
// 1. Should we reset the state to unlocked from heavyweight lock?
// Potential benefit: less memory consumption and usage of lightweight locks
// Potential drawback: infrustructure to detect, when the monitor is not acquired by any thread and time for repeated
// inflation
// 2. If the state should be reseted, when it should be done?
// Potential targets: after monitor release check the owners of monitors,
// special request, for instance, from GC.
// 3. Do we really need try locks?
// 4. Is it useful to return ObjectHeader from monitorenter/exit? Right now it is enough to return bool value.

class Monitor {
public:
    using MonitorId = uintptr_t;

    enum State {
        OK,
        INTERRUPTED,
        ILLEGAL,
    };

    MonitorId GetId() const
    {
        return id_;
    }

    PANDA_PUBLIC_API static Monitor::State MonitorEnter(ObjectHeader *obj, bool trylock = false);

    static void InflateThinLock(MTManagedThread *thread, const VMHandle<ObjectHeader> &obj);

    PANDA_PUBLIC_API static Monitor::State MonitorExit(ObjectHeader *obj);

    /**
     * Static call which attempts to wait until timeout, interrupt, or notification.
     *
     * @param  obj  an object header of corresponding object
     * @param  status  status to be set up during wait
     * @param  timeout  waiting time in milliseconds
     * @param  nanos  additional time in nanoseconds
     * @param  ignore_interruption  ignore interruption event or not
     * @return true if it was interrupted; false otherwise
     */
    PANDA_PUBLIC_API static State Wait(ObjectHeader *obj, ThreadStatus status, uint64_t timeout, uint64_t nanos,
                                       bool ignoreInterruption = false);

    static State Notify(ObjectHeader *obj);

    static State NotifyAll(ObjectHeader *obj);

    /**
     * Static call which attempts to inflate object lock (lightweight/unlocked) and acquires its lock if it's
     * successful. Provides no guarantees on object having heavy lock unless it returns true.
     *
     * @param  obj  an object header of corresponding object
     * @param  thread pointer to thread which will acquire the monitor.
     * @tparam for_other_thread include logic for inflation of monitor owned by other thread. Should be used
     * only in futex build.
     * @return true if new monitor was successfuly created and object's markword updated with monitor's ID;
     * false otherwise
     */
    template <bool FOR_OTHER_THREAD = false>
    static bool Inflate(ObjectHeader *obj, MTManagedThread *thread);

    /**
     * Static call which attempts to deflate object's heavy lock if it's present and unlocked.
     * Ignores object if it doesn't have heavy lock.
     *
     * @param  obj  an object header of corresponding object
     * @return true if object's monitor was found, acquired and freed; false otherwise
     */
    static bool Deflate(ObjectHeader *obj);

    PANDA_PUBLIC_API static uint8_t HoldsLock(ObjectHeader *obj);

    static uint32_t GetLockOwnerOsThreadID(ObjectHeader *obj);

    static Monitor *GetMonitorFromObject(ObjectHeader *obj);

    static void TraceMonitorLock(ObjectHeader *obj, bool isWait);

    static void TraceMonitorUnLock();

    // NO_THREAD_SAFETY_ANALYSIS for monitor->lock_
    // Some more information in the issue #1662
    bool Release(MTManagedThread *thread) NO_THREAD_SAFETY_ANALYSIS;

    uint32_t GetHashCode();

    bool HasHashCode() const;

    void SetHashCode(uint32_t hash);

    void SetObject(ObjectHeader *object)
    {
        obj_ = object;
    }

    ObjectHeader *GetObject()
    {
        return obj_;
    }

    // Public constructor is needed for allocator
    explicit Monitor(MonitorId id) : id_(id), owner_(), hashCode_(0), waitersCounter_(0)
    {
        // Atomic with relaxed order reason: memory access in monitor
        owner_.store(nullptr, std::memory_order_relaxed);
    }

private:
    MonitorId id_;
    ObjectHeader *obj_ {nullptr};  // Used for GC deflation
    std::atomic<MTManagedThread *> owner_;
    // These are two lists, which are linked with nextThread
    // Be careful when changing these two lists to other types, or changing List implementation,
    // current Monitor::Notify implementation relies on the fact that reference to MTManagedThread is still valid
    // when PopFront is called.
    ThreadList<MTManagedThread> waiters_;
    ThreadList<MTManagedThread> toWakeup_;
    uint64_t recursiveCounter_ {0};
    os::memory::Mutex lock_;
    std::atomic<uint32_t> hashCode_;
    std::atomic<uint32_t> waitersCounter_;

    // NO_THREAD_SAFETY_ANALYSIS for monitor->lock_
    // Some more information in the issue #1662
    bool Acquire(MTManagedThread *thread, const VMHandle<ObjectHeader> &objHandle,
                 bool trylock) NO_THREAD_SAFETY_ANALYSIS;

    void Acquire(MTManagedThread *thread, const VMHandle<ObjectHeader> &objHandle) NO_THREAD_SAFETY_ANALYSIS;

    void InitWithOwner(MTManagedThread *thread, ObjectHeader *obj) NO_THREAD_SAFETY_ANALYSIS;

    void ReleaseOnFailedInflate(MTManagedThread *thread) NO_THREAD_SAFETY_ANALYSIS;

    bool SetOwner(MTManagedThread *expected, MTManagedThread *thread)
    {
        return owner_.compare_exchange_strong(expected, thread);
    }

    static std::optional<Monitor::State> HandleLightLockedState(MarkWord &mark, MTManagedThread *thread,
                                                                VMHandle<ObjectHeader> &objHandle,
                                                                uint32_t &lightlockRetryCount,
                                                                [[maybe_unused]] bool &shouldInflate, bool trylock);

    static std::optional<Monitor::State> HandleUnlockedState(MarkWord &mark, MTManagedThread *thread,
                                                             VMHandle<ObjectHeader> &objHandle, bool &shouldInflate,
                                                             bool trylock);

    static Monitor::State HandleHeavyLockedState(Monitor *monitor, MTManagedThread *thread,
                                                 VMHandle<ObjectHeader> &objHandle, bool trylock);

    MTManagedThread *GetOwner()
    {
        // Atomic with relaxed order reason: memory access in monitor
        return owner_.load(std::memory_order_relaxed);
    }

    bool DeflateInternal();

    static bool TryAddMonitor(Monitor *monitor, MarkWord &oldMark, MTManagedThread *thread, ObjectHeader *obj,
                              MonitorPool *monitorPool);

    static State WaitWithHeavyLockedState(MTManagedThread *thread, VMHandle<ObjectHeader> &objHandle, MarkWord &mark,
                                          ThreadStatus status, uint64_t timeout, uint64_t nanos,
                                          bool ignoreInterruption);

    friend class MonitorPool;
};

}  // namespace ark

#endif  // PANDA_RUNTIME_MONITOR_H_
