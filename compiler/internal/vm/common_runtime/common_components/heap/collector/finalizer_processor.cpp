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
#include "common_components/heap/collector/finalizer_processor.h"

#include "common_components/common/scoped_object_access.h"
#include "common_components/mutator/mutator.h"

namespace common {
constexpr uint32_t DEFAULT_FINALIZER_TIMEOUT_MS = 2000;

// Note: can only be called by FinalizerProcessor thread
extern "C" PUBLIC_API void* ArkProcessFinalizers(void* arg)
{
#ifdef __APPLE__
    CHECK_CALL(pthread_setname_np, ("gc-helper"), "finalizer-processor thread setname");
#elif defined(__linux__) || defined(PANDA_TARGET_OHOS)
    CHECK_CALL(prctl, (PR_SET_NAME, "gc-helper"), "finalizer-processor thread setname");
#endif
    reinterpret_cast<FinalizerProcessor*>(arg)->Run();
    return nullptr;
}

void FinalizerProcessor::Start()
{
    pthread_t thread;
    pthread_attr_t attr;
    // size_t stackSize = ArkCommonRuntime::GetConcurrencyParam().thStackSize * KB; // default 1MB stacksize
    // FinalizerProcessor needs to be revied
    size_t stackSize = 1024 * KB;
#if defined(__linux__) || defined(PANDA_TARGET_OHOS) || defined(__APPLE__)
    // PTHREAD_STACK_MIN is not supported in Windows.
    if (stackSize < static_cast<size_t>(PTHREAD_STACK_MIN)) {
        stackSize = static_cast<size_t>(PTHREAD_STACK_MIN);
    }
#endif
    CHECK_CALL(pthread_attr_init, (&attr), "init pthread attr");
    CHECK_CALL(pthread_attr_setdetachstate, (&attr, PTHREAD_CREATE_JOINABLE), "set pthread joinable");
    CHECK_CALL(pthread_attr_setstacksize, (&attr, stackSize), "set pthread stacksize");
    CHECK_CALL(pthread_create, (&thread, &attr, ArkProcessFinalizers, this),
               "create finalizer-process thread");
#ifdef __WIN64
    CHECK_CALL(pthread_setname_np, (thread, "gc-helper"), "finalizer-processor thread setname");
#endif
    CHECK_CALL(pthread_attr_destroy, (&attr), "destroy pthread attr");
    threadHandle_ = thread;

    WaitStarted();
}

// Stop FinalizerProcessor is only invoked at Fork or Runtime finliazaiton
// Should only invoke once.
void FinalizerProcessor::Stop()
{
	// This will only occur in the prefork scenario.
    if (running_ == false) {
        return;
    }
    running_ = false;
    Notify();
    WaitStop();
}

FinalizerProcessor::FinalizerProcessor()
{
    started_ = false;
    running_ = false;
    iterationWaitTime_ = DEFAULT_FINALIZER_TIMEOUT_MS;
    timeProcessorBegin_ = 0;
    timeProcessUsed_ = 0;
    timeCurrentProcessBegin_ = 0;
    hasFinalizableJob_.store(false, std::memory_order_relaxed);
    shouldReclaimHeapGarbage_.store(false, std::memory_order_relaxed);
    shouldFeedHungryBuffers_.store(false, std::memory_order_relaxed);
}

void FinalizerProcessor::Run()
{
    Init();
    NotifyStarted();
    while (running_) {
        if (hasFinalizableJob_.load(std::memory_order_relaxed)) {
            // In theory we currently won't have this
            ProcessFinalizables();
#if defined(GCINFO_DEBUG) && GCINFO_DEBUG
            LogAfterProcess();
#endif
        }

        if (shouldFeedHungryBuffers_.load(std::memory_order_relaxed)) {
            FeedHungryBuffers();
        }

        if (shouldReclaimHeapGarbage_.load(std::memory_order_relaxed)) {
            ReclaimHeapGarbage();
        }

        {
            COMMON_PHASE_TIMER("finalizerProcessor waitting time");
            while (running_) {
                Wait(iterationWaitTime_);
                if (hasFinalizableJob_.load(std::memory_order_relaxed) ||
                    shouldReclaimHeapGarbage_.load(std::memory_order_relaxed) ||
                    shouldFeedHungryBuffers_.load(std::memory_order_relaxed)) {
                    break;
                }
            }
        }
    }
    Fini();
}

void FinalizerProcessor::Init()
{
    Mutator* mutator = MutatorManager::Instance().CreateRuntimeMutator(ThreadType::FP_THREAD);
    (void)mutator->EnterSaferegion(true);
    tid_ = mutator->GetTid();
    ThreadLocal::SetProtectAddr(reinterpret_cast<uint8_t*>(0));
    running_ = true;
    timeProcessorBegin_ = TimeUtil::MicroSeconds();
    timeProcessUsed_ = 0;
    MutatorManager::Instance().MutatorManagementRLock();
    fpMutator_ = mutator;
    MutatorManager::Instance().MutatorManagementRUnlock();
    VLOG(INFO, "FinalizerProcessor thread started");
}

void FinalizerProcessor::Fini()
{
    MutatorManager::Instance().MutatorManagementRLock();
    fpMutator_ = nullptr;
    MutatorManager::Instance().MutatorManagementRUnlock();
    MutatorManager::Instance().DestroyRuntimeMutator(ThreadType::FP_THREAD);
    VLOG(INFO, "FinalizerProcessor thread stopped");
}

void FinalizerProcessor::WaitStop()
{
    pthread_t thread = threadHandle_;
    int tmpResult = ::pthread_join(thread, nullptr);
    LOGF_CHECK(tmpResult == 0) << "::pthread_join() in FinalizerProcessor::WaitStop() return " <<
    tmpResult << " rather than 0.";
    started_ = false;
    threadHandle_ = 0;
}

void FinalizerProcessor::Notify() { wakeCondition_.notify_one(); }

void FinalizerProcessor::Wait(uint32_t timeoutMilliSeconds)
{
    std::unique_lock<std::mutex> lock(wakeLock_);
    std::chrono::milliseconds epoch(timeoutMilliSeconds);
    wakeCondition_.wait_for(lock, epoch);
}

void FinalizerProcessor::NotifyStarted()
{
    {
        std::unique_lock<std::mutex> lock(startedLock_);
        LOGF_CHECK(started_ != true) << "unpexcted true, FinalizerProcessor might not wait stopped";
        started_ = true;
    }
    startedCondition_.notify_all();
}

void FinalizerProcessor::WaitStarted()
{
    std::unique_lock<std::mutex> lock(startedLock_);
    if (started_) {
        return;
    }
    startedCondition_.wait(lock, [this] { return started_; });
}

void FinalizerProcessor::EnqueueFinalizables(const std::function<bool(BaseObject*)>& finalizable, uint32_t countLimit)
{
    std::lock_guard<std::mutex> l(listLock_);
    auto it = finalizers_.begin();
    while (it != finalizers_.end() && countLimit != 0) {
        RefField<> tmpField(reinterpret_cast<HeapAddress>(*it));
        BaseObject* obj = tmpField.GetTargetObject();
        --countLimit;
        if (finalizable(obj)) {
            finalizables_.push_back(reinterpret_cast<BaseObject*>(tmpField.GetFieldValue()));
            it = finalizers_.erase(it);
        } else {
            ++it;
        }
    }

    if (!finalizables_.empty()) {
        hasFinalizableJob_.store(true, std::memory_order_relaxed);
    }
}

// Process finalizable list
// 1. always process list head
// 2. Leave safe region (calling in finalizerProcessor thread)
// 3. Invoke finalize method
// 4. remove processed finalizables
void FinalizerProcessor::ProcessFinalizableList()
{
    LOG_COMMON(FATAL) << "Unresolved fatal";
    UNREACHABLE_CC();
}

void FinalizerProcessor::ProcessFinalizables()
{
    COMMON_PHASE_TIMER("Finalizer");
    {
        // we leave saferegion to avoid GC visit those changing queues.
        ScopedObjectAccess soa;
        std::lock_guard<std::mutex> l(listLock_);
        // FP will not come here before cleaning up workingFinalizables
        // workingFinalizables is expected empty, thus we could use std::swap here
        workingFinalizables_.swap(finalizables_);
    }
    DLOG(FINALIZE, "finalizer: working size %zu", workingFinalizables_.size());
    ProcessFinalizableList();
    if (finalizables_.empty()) {
        hasFinalizableJob_.store(false, std::memory_order_relaxed);
    }
}

#if defined(GCINFO_DEBUG) && GCINFO_DEBUG
void FinalizerProcessor::LogAfterProcess()
{
    if (!ENABLE_LOG(FINALIZE)) {
        return;
    }
    uint64_t timeNow = TimeUtil::MicroSeconds();
    uint64_t timeConsumed = timeNow - timeCurrentProcessBegin_;
    uint64_t totalTimePassed = timeNow - timeProcessorBegin_;
    timeProcessUsed_ += timeConsumed;
    constexpr float percentageDivend = 100.0f;
    float percentage = (static_cast<float>(TIME_FACTOR * timeProcessUsed_) / totalTimePassed) / percentageDivend;
    DLOG(FINALIZE, "[FinalizerProcessor] End (%luus [%luus] [%.2f%%])", timeConsumed, timeProcessUsed_, percentage);
}
#endif

void FinalizerProcessor::RegisterFinalizer(BaseObject* obj)
{
    RefField<> tmpField(nullptr);
    Heap::GetHeap().GetBarrier().WriteStaticRef(tmpField, obj);
    std::lock_guard<std::mutex> l(listLock_);
    finalizers_.push_back(reinterpret_cast<BaseObject*>(tmpField.GetFieldValue()));
}

void FinalizerProcessor::ReclaimHeapGarbage()
{
    OHOS_HITRACE(HITRACE_LEVEL_COMMERCIAL, "ARK_RT_GC_RECLAIM", "");
    Heap::GetHeap().GetAllocator().ReclaimGarbageMemory(false);
    shouldReclaimHeapGarbage_.store(false, std::memory_order_relaxed);
}

void FinalizerProcessor::FeedHungryBuffers()
{
    Heap::GetHeap().GetAllocator().FeedHungryBuffers();
    shouldFeedHungryBuffers_.store(false, std::memory_order_relaxed);
}
} // namespace common
