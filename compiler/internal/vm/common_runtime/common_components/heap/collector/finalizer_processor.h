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

#ifndef COMMON_COMPONENTS_HEAP_COLLECTOR_FINALIZER_PROCESSOR_H
#define COMMON_COMPONENTS_HEAP_COLLECTOR_FINALIZER_PROCESSOR_H

#include <climits>
#include <condition_variable>
#include <list>
#include <mutex>

#include "common_components/common/page_allocator.h"
#include "common_components/common/type_def.h"
#include "common_components/heap/collector/collector.h"
#include "common_components/log/log.h"

namespace common {
template<typename T>
using ManagedList = std::list<T, StdContainerAllocator<T, FINALIZER_PROCESSOR>>;

class FinalizerProcessor {
public:
    FinalizerProcessor();
    ~FinalizerProcessor() = default;

    // mainly for resurrection.
    uint32_t VisitFinalizers(const RootVisitor& visitor)
    {
        uint32_t count = 0;
        std::lock_guard<std::mutex> l(listLock_);
        for (BaseObject*& obj : finalizers_) {
            visitor(reinterpret_cast<ObjectRef&>(obj));
            ++count;
        }
        return count;
    }

    // for : *finalizers* are not proper gc roots.
    void VisitGCRoots(const RootVisitor& visitor)
    {
        std::lock_guard<std::mutex> l(listLock_);
        for (BaseObject*& obj : finalizables_) {
            visitor(reinterpret_cast<ObjectRef&>(obj));
        }
        for (BaseObject*& obj : workingFinalizables_) {
            visitor(reinterpret_cast<ObjectRef&>(obj));
        }
    }

    // mainly for fixing old pointers
    void VisitRawPointers(const RootVisitor& visitor)
    {
        std::lock_guard<std::mutex> l(listLock_);
        for (BaseObject*& obj : finalizables_) {
            visitor(reinterpret_cast<ObjectRef&>(obj));
        }
        for (BaseObject*& obj : workingFinalizables_) {
            visitor(reinterpret_cast<ObjectRef&>(obj));
        }
        for (BaseObject*& obj : finalizers_) {
            visitor(reinterpret_cast<ObjectRef&>(obj));
        }
    }

    // notify for finalizer processing loop, invoked after GC
    void Notify();
    // wait started flag set, call after create finalizerProcessor thread
    void WaitStarted();

    void Start();
    void Stop();
    void Run();
    void Init();
    void Fini();
    void WaitStop();

    void EnqueueFinalizables(const std::function<bool(BaseObject*)>& finalizable, uint32_t countLimit = UINT_MAX);
    void RegisterFinalizer(BaseObject* obj);
    bool IsRunning() const { return running_; }
    uint32_t GetTid() const { return tid_; }

    Mutator* GetMutator() const { return fpMutator_; }

    void NotifyToReclaimGarbage() { shouldReclaimHeapGarbage_.store(true); }
    void NotifyToFeedAllocBuffers()
    {
        shouldFeedHungryBuffers_.store(true, std::memory_order_release);
        Notify();
    }

private:
    void NotifyStarted();
    void Wait(uint32_t timeoutMilliSeconds);
    void ProcessFinalizables();
    void ProcessFinalizableList();
    void ReclaimHeapGarbage();
    void FeedHungryBuffers();

    std::mutex wakeLock_;
    std::condition_variable wakeCondition_; // notify finalizer processing continue

    std::mutex startedLock_;
    std::condition_variable startedCondition_; // notify finalizerProcessor thread is started
    volatile bool started_;

    volatile bool running_; // Initially false and set true after finalizerProcessor thread start, set false when stop

    uint32_t iterationWaitTime_;

    // finalization
    std::mutex listLock_;                 // lock for finalizers & finalizables & workingFinalizables
    ManagedList<BaseObject*> finalizers_; // created finalizer record, accessed by mutator & GC

    // a dead finalizer is moved into finalizable by GC, then run finalize method by FP thread
    ManagedList<BaseObject*> finalizables_;

    ManagedList<BaseObject*> workingFinalizables_; // FP working list, swap from finalizables

    std::atomic<bool> hasFinalizableJob_;
    std::atomic<bool> shouldReclaimHeapGarbage_;
    std::atomic<bool> shouldFeedHungryBuffers_;
    #if defined(GCINFO_DEBUG) && GCINFO_DEBUG
    // stats
    void LogAfterProcess();
#endif
    uint64_t timeProcessorBegin_;
    uint64_t timeProcessUsed_;
    uint64_t timeCurrentProcessBegin_;
    uint32_t tid_ = 0;
    pthread_t threadHandle_ = 0; // thread handle to thread
    Mutator* fpMutator_ = nullptr;
};
} // namespace common

#endif // COMMON_COMPONENTS_HEAP_COLLECTOR_FINALIZER_PROCESSOR_H
