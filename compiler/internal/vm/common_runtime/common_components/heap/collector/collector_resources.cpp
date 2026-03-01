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
#include "common_components/heap/collector/collector_resources.h"

#include <thread>

#include "common_components/base/sys_call.h"
#include "common_components/heap/collector/collector_proxy.h"
#include "common_components/common/run_type.h"
#include "common_components/common/scoped_object_access.h"
#include "common_components/mutator/mutator_manager.h"
#ifdef ENABLE_QOS
#include "qos.h"
#endif

namespace common {

void* CollectorResources::GCMainThreadEntry(void* arg)
{
#ifdef __APPLE__
    int ret = pthread_setname_np("OS_GC_Thread");
    LOGE_IF(UNLIKELY_CC(ret != 0)) << "pthread setname in CollectorResources::StartGCThreads() return " <<
        ret << " rather than 0";
#elif defined(__linux__) || defined(PANDA_TARGET_OHOS)
    int ret = prctl(PR_SET_NAME, "OS_GC_Thread");
    LOGE_IF(UNLIKELY_CC(ret != 0)) << "pthread setname in CollectorResources::StartGCThreads() return " <<
        ret << " rather than 0";
#endif

    ASSERT_LOGF(arg != nullptr, "GCMainThreadEntry arg=nullptr");
    // set current thread as a gc thread.
    ThreadLocal::SetThreadType(ThreadType::GC_THREAD);

    VLOG(INFO, "CollectorResources Thread begin.");

#ifdef ENABLE_QOS
    OHOS::QOS::SetQosForOtherThread(OHOS::QOS::QosLevel::QOS_USER_INITIATED, GetTid());
#endif

    // run event loop in this thread.
    CollectorResources* collectorResources = reinterpret_cast<CollectorResources*>(arg);
    collectorResources->RunTaskLoop();

    VLOG(INFO, "CollectorResources Thread end.");
    return nullptr;
}

void CollectorResources::Init()
{
    taskQueue_ = new GCTaskQueue<GCRunner>;
    taskQueue_->Init();
    finishedGcIndex_ = 0;
    StartGCThreads();
    finalizerProcessor_.Start();
    gcStats_.Init();
    hasRelease = false;
}

void CollectorResources::Fini()
{
    if (hasRelease == false) {
        StopGCWork();
        ASSERT_LOGF(!finalizerProcessor_.IsRunning(), "Invalid finalizerProcessor status");
        ASSERT_LOGF(!gcThreadRunning_.load(std::memory_order_relaxed), "Invalid GC thread status");
        taskQueue_->Finish();
        delete taskQueue_;
        taskQueue_ = nullptr;
        hasRelease = true;
    }
}

void CollectorResources::StopGCWork()
{
    finalizerProcessor_.Stop();
    TerminateGCTask();
    StopGCThreads();
}

void CollectorResources::StartRuntimeThreads()
{
    // For Postfork.
    Init();
}

void CollectorResources::StopRuntimeThreads()
{
    // For Prefork.
    Fini();
}

// Send terminate task to gc thread.
void CollectorResources::TerminateGCTask()
{
    if (gcThreadRunning_.load(std::memory_order_acquire) == false) {
        return;
    }

    GCTaskQueue<GCRunner>::GCTaskFilter filter = [](GCRunner&, GCRunner&) { return false; };
    GCRunner task(GCTask::GCTaskType::GC_TASK_TERMINATE_GC);
    (void)taskQueue_->EnqueueSync(task, filter); // enqueue to sync queue
}

// Usually called from main thread, wait for collector thread to exit.
void CollectorResources::StopGCThreads()
{
    if (gcThreadRunning_.load(std::memory_order_acquire) == false) {
        LOG_COMMON(FATAL) << "[GC] CollectorResources Thread not begin.";
        UNREACHABLE_CC();
    }
    {
        // Enter saferegion to avoid blocking gc stw
        ScopedEnterSaferegion enterSaferegion(true);
        int ret = ::pthread_join(gcMainThread_, nullptr);
        LOGE_IF(UNLIKELY_CC(ret != 0)) << "::pthread_join() in StopGCThreads() return " << ret;
    }
    // wait the thread pool stopped.
    if (gcThreadPool_ != nullptr) {
        gcThreadPool_->Destroy(0);
        gcThreadPool_ = nullptr;
    }
    gcThreadRunning_.store(false, std::memory_order_release);
}

void CollectorResources::RunTaskLoop()
{
    gcTid_.store(GetTid(), std::memory_order_release);
    taskQueue_->DrainTaskQueue(&collectorProxy_);
    NotifyGCFinished(GCTask::TASK_INDEX_GC_EXIT);
}

// For the ignored gc request, check whether need to wait for current gc finish
void CollectorResources::PostIgnoredGcRequest(GCReason reason)
{
    GCRequest& request = g_gcRequests[reason];
    if (request.IsSyncGC() && isGcStarted_.load(std::memory_order_seq_cst)) {
        ScopedEnterSaferegion safeRegion(false);
        WaitForGCFinish();
    }
}

void CollectorResources::RequestAsyncGC(GCReason reason, GCType gcType)
{
    // The gc request must be none blocked
    ASSERT_LOGF(!g_gcRequests[reason].IsSyncGC(), "trigger from unsafe context must be none blocked");
    GCRunner gcTask(GCTask::GCTaskType::GC_TASK_INVOKE_GC, reason, gcType);
    // we use async enqueue because this doesn't have locks, lowering the risk
    // of timeouts when entering safe region due to thread scheduling
    taskQueue_->EnqueueAsync(gcTask);
}

void CollectorResources::RequestGCAndWait(GCReason reason, GCType gcType)
{
    // Enter saferegion since current thread may blocked by locks.
    ScopedEnterSaferegion enterSaferegion(false);
    GCRunner gcTask(GCTask::GCTaskType::GC_TASK_INVOKE_GC, reason, gcType);

    GCTaskQueue<GCRunner>::GCTaskFilter filter = [](GCRunner& oldTask, GCRunner& newTask) {
        return oldTask.GetGCReason() == newTask.GetGCReason();
    };

    GCRequest& request = g_gcRequests[reason];
    // If this gcTask need not to block, just add to async queue
    if (!request.IsSyncGC()) {
        taskQueue_->EnqueueAsync(gcTask);
        return;
    }

    // If this gcTask need to block,
    // add gcTask to syncTaskQueue of gcTaskQueue and wait until this gcTask finished
    std::unique_lock<std::mutex> lock(gcFinishedCondMutex_);
    uint64_t curThreadSyncIndex = taskQueue_->EnqueueSync(gcTask, filter);
    // wait until GC finished
    std::function<bool()> pred = [this, curThreadSyncIndex] {
        return ((finishedGcIndex_ >= curThreadSyncIndex) || (finishedGcIndex_ == GCTask::TASK_INDEX_GC_EXIT));
    };
    gcFinishedCondVar_.wait(lock, pred);
}

void CollectorResources::RequestGC(GCReason reason, bool async, GCType gcType)
{
    if (!IsGCActive()) {
        return;
    }

    GCRequest& request = g_gcRequests[reason];
    uint64_t curTime = TimeUtil::NanoSeconds();
    request.SetPrevRequestTime(curTime);
    if (collectorProxy_.ShouldIgnoreRequest(request)
        || (reason == GCReason::GC_REASON_NATIVE && IsNativeGCInvoked())) {
        DLOG(ALLOC, "ignore gc request");
        PostIgnoredGcRequest(reason);
    } else if (async) {
        if (reason == GCReason::GC_REASON_NATIVE) {
            SetIsNativeGCInvoked(true);
        }
        RequestAsyncGC(reason, gcType);
    } else {
        RequestGCAndWait(reason, gcType);
    }
}

void CollectorResources::NotifyGCFinished(uint64_t gcIndex)
{
    std::unique_lock<std::mutex> lock(gcFinishedCondMutex_);
    isGcStarted_.store(false, std::memory_order_relaxed);
    if (gcIndex >= GCTask::TASK_INDEX_SYNC_GC_MIN) { // sync gc, need set taskIndex
        finishedGcIndex_.store(gcIndex);
    }
    gcFinishedCondVar_.notify_all();
    BroadcastGCFinished();
}

void CollectorResources::MarkGCStart()
{
    std::unique_lock<std::mutex> lock(gcFinishedCondMutex_);

    // Wait for any existing GC to finish - inline the wait logic
    std::function<bool()> pred = [this] {
        return !IsGcStarted();
    };
    gcFinishedCondVar_.wait(lock, pred);

    // Now claim GC ownership
    SetGcStarted(true);
}

void CollectorResources::MarkGCFinish(uint64_t gcIndex)
{
    NotifyGCFinished(gcIndex);
}

void CollectorResources::WaitForGCFinish()
{
    uint64_t startTime = TimeUtil::MicroSeconds();
    std::unique_lock<std::mutex> lock(gcFinishedCondMutex_);
    uint64_t curWaitGcIndex = finishedGcIndex_.load();
    std::function<bool()> pred = [this, curWaitGcIndex] {
        return (!IsGcStarted() || (curWaitGcIndex != finishedGcIndex_) ||
                (finishedGcIndex_ == GCTask::TASK_INDEX_GC_EXIT));
    };
    gcFinishedCondVar_.wait(lock, pred);
    uint64_t stopTime = TimeUtil::MicroSeconds();
    uint64_t diffTime = stopTime - startTime;
    VLOG(DEBUG, "WaitForGCFinish cost %zu us", diffTime);
}

void CollectorResources::StartGCThreads()
{
    bool expected = false;
    if (gcThreadRunning_.compare_exchange_strong(expected, true, std::memory_order_acquire) == false) {
        LOG_COMMON(FATAL) << "[GC] CollectorResources Thread already begin.";
        UNREACHABLE_CC();
    }
    DCHECK_CC(gcThreadPool_ == nullptr);
    gcThreadPool_ = Taskpool::GetCurrentTaskpool();
    gcThreadPool_->Initialize();
    LOGF_CHECK(gcThreadPool_ != nullptr) << "new GCThreadPool failed";
    uint32_t helperThreads = gcThreadPool_->GetTotalThreadNum();
    if (helperThreads > 0) {
        --helperThreads;    // gc task is exclusive, so keep one thread left
    }
    // 1 is for gc main thread.
    gcThreadCount_ = helperThreads + 1;
    VLOG(DEBUG, "total gc thread count %d, helper thread count %d", gcThreadCount_, helperThreads);

    // create the collector thread.
    if (::pthread_create(&gcMainThread_, nullptr, CollectorResources::GCMainThreadEntry, this) != 0) {
        ASSERT_LOGF(0, "pthread_create failed!");
    }
    // set thread name.
#ifdef __WIN64
    int ret = pthread_setname_np(gcMainThread_, "OS_GC_Thread");
    LOGE_IF(UNLIKELY_CC(ret != 0)) << "pthread_setname_np() in CollectorResources::StartGCThreads() return " <<
        ret << " rather than 0";
#endif
}

uint32_t CollectorResources::GetGCThreadCount(const bool isConcurrent) const
{
    if (GetThreadPool() == nullptr) {
        return 1;
    } else if (isConcurrent) {
        return gcThreadCount_;
    }
    // default to 2
    return 2;
}

void CollectorResources::BroadcastGCFinished()
{
    gcWorking_ = 0;
#if defined(_WIN64) || defined(__APPLE__)
    WakeWhenGCDone();
#else
    (void)Futex(&gcWorking_, FUTEX_WAKE_PRIVATE, INT_MAX);
#endif
}

void CollectorResources::RequestHeapDump(GCTask::GCTaskType gcTask)
{
    GCTaskQueue<GCRunner>::GCTaskFilter filter = [](GCRunner&, GCRunner&) { return false; };
    GCRunner dumpTask = GCRunner(gcTask);
    taskQueue_->EnqueueSync(dumpTask, filter);
}

} // namespace common
