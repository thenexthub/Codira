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


#include "CollectorResources.h"

#include <thread>

#include "Base/SysCall.h"
#include "CollectorProxy.h"
#include "Common/RunType.h"
#include "Common/ScopedObjectAccess.h"
#include "LoaderManager.h"
#include "Mutator/MutatorManager.h"

namespace MapleRuntime {
extern "C" uintptr_t MRT_StopGCWork()
{
    Heap::GetHeap().StopGCWork();
    return 0;
}

void* CollectorResources::GCMainThreadEntry(void* arg)
{
#ifdef __APPLE__
    int ret = pthread_setname_np("gc-main-thread");
    CHECK_E(UNLIKELY(ret != 0), "pthread setname in CollectorResources::StartGCThreads() return %d rather than 0",
            ret);
#elif defined(__linux__) || defined(hongmeng)
    int ret = prctl(PR_SET_NAME, "gc-main-thread");
    CHECK_E(UNLIKELY(ret != 0), "pthread setname in CollectorResources::StartGCThreads() return %d rather than 0",
            ret);
#endif
    
    MRT_ASSERT(arg != nullptr, "GCMainThreadEntry arg=nullptr");
    // set current thread as a gc thread.
    ThreadLocal::SetThreadType(ThreadType::GC_THREAD);

    LOG(RTLOG_INFO, "[GC] CollectorResources Thread begin.");

#if defined(__linux__) || defined(hongmeng)
    // set thread priority.
    GCPoolThread::SetThreadPriority(MapleRuntime::GetTid(), GCPoolThread::GC_THREAD_PRIORITY);
#endif

    // run event loop in this thread.
    CollectorResources* collectorResources = reinterpret_cast<CollectorResources*>(arg);
    collectorResources->RunTaskLoop();

    LOG(RTLOG_INFO, "[GC] CollectorResources Thread end.");
    return nullptr;
}

void CollectorResources::Init()
{
    taskQueue = new TaskQueue<GCExecutor>;
    taskQueue->Init();
    finishedGcIndex = GCTask::SYNC_TASK_MIN_INDEX;
    StartGCThreads();
    finalizerProcessor.Start();
    gcStats.Init();
}

void CollectorResources::Fini()
{
    MRT_ASSERT(!finalizerProcessor.IsRunning(), "Invalid finalizerProcessor status");
    MRT_ASSERT(!gcThreadRunning.load(std::memory_order_relaxed), "Invalid GC thread status");
    taskQueue->Fini();
    delete taskQueue;
    taskQueue = nullptr;
}

void CollectorResources::StopGCWork()
{
    finalizerProcessor.Stop();
    TerminateGCTask();
    StopGCThreads();
}

// Send terminate task to gc thread.
void CollectorResources::TerminateGCTask()
{
    if (gcThreadRunning.load(std::memory_order_acquire) == false) {
        return;
    }

    TaskQueue<GCExecutor>::TaskFilter filter = [](GCExecutor&, GCExecutor&) { return false; };
    GCExecutor task(GCTask::TaskType::TASK_TYPE_TERMINATE_GC);
    (void)taskQueue->EnqueueSync(task, filter); // enqueue to sync queue
}

// Usually called from main thread, wait for collector thread to exit.
void CollectorResources::StopGCThreads()
{
    if (gcThreadRunning.load(std::memory_order_acquire) == false) {
        return;
    }
    int ret = ::pthread_join(gcMainThread, nullptr);
    CHECK_E(UNLIKELY(ret != 0), "::pthread_join() in StopGCThreads() return %d", ret);
    // wait the thread pool stopped.
    if (gcThreadPool != nullptr) {
        gcThreadPool->Exit();
        delete gcThreadPool;
        gcThreadPool = nullptr;
    }
    gcThreadRunning.store(false, std::memory_order_release);
}

void CollectorResources::RunTaskLoop()
{
    gcTid.store(MapleRuntime::GetTid(), std::memory_order_release);
    taskQueue->DrainTaskQueue(&collectorProxy);
    NotifyGCFinished(GCTask::TASK_INDEX_FOR_EXIT);
}

// For the ignored gc request, check whether need to wait for current gc finish
void CollectorResources::PostIgnoredGcRequest(GCReason reason)
{
    GCRequest& request = g_gcRequests[reason];
    if (request.IsSyncGC() && isGcStarted.load(std::memory_order_seq_cst)) {
        ScopedEnterSaferegion safeRegion(false);
        WaitForGCFinish();
    }
}

void CollectorResources::RequestAsyncGC(GCReason reason)
{
    // The gc request must be none blocked
    MRT_ASSERT(!g_gcRequests[reason].IsSyncGC(), "trigger from unsafe context must be none blocked");
    GCExecutor gcTask(GCTask::TaskType::TASK_TYPE_INVOKE_GC, reason);
    // we use async enqueue because this doesn't have locks, lowering the risk
    // of timeouts when entering safe region due to thread scheduling
    taskQueue->EnqueueAsync(gcTask);
}

void CollectorResources::RequestGCAndWait(GCReason reason)
{
    // Enter saferegion since current thread may blocked by locks.
    ScopedEnterSaferegion enterSaferegion(false);
    GCExecutor gcTask(GCTask::TaskType::TASK_TYPE_INVOKE_GC, reason);

    TaskQueue<GCExecutor>::TaskFilter filter = [](GCExecutor& oldTask, GCExecutor& newTask) {
        return oldTask.GetGCReason() == newTask.GetGCReason();
    };

    GCRequest& request = g_gcRequests[reason];
    // If this gcTask need not to block, just add to async queue
    if (!request.IsSyncGC()) {
        taskQueue->EnqueueAsync(gcTask);
        return;
    }

    // If this gcTask need to block,
    // add gcTask to syncTaskQueue of gcTaskQueue and wait until this gcTask finished
    std::unique_lock<std::mutex> lock(gcFinishedCondMutex);
    uint64_t curThreadSyncIndex = taskQueue->EnqueueSync(gcTask, filter);
    // wait until GC finished
    std::function<bool()> pred = [this, curThreadSyncIndex] {
        return ((finishedGcIndex >= curThreadSyncIndex) || (finishedGcIndex == GCTask::TASK_INDEX_FOR_EXIT));
    };
    gcFinishedCondVar.wait(lock, pred);
}

void CollectorResources::RequestGC(GCReason reason, bool async)
{
    if (!IsGCActive()) {
        return;
    }

    GCRequest& request = g_gcRequests[reason];
    uint64_t curTime = TimeUtil::NanoSeconds();
    request.SetPrevRequestTime(curTime);
    if (collectorProxy.ShouldIgnoreRequest(request)) {
        DLOG(ALLOC, "ignore gc request");
        PostIgnoredGcRequest(reason);
    } else if (async) {
        RequestAsyncGC(reason);
    } else {
        RequestGCAndWait(reason);
    }
}

void CollectorResources::NotifyGCFinished(uint64_t gcIndex)
{
    std::unique_lock<std::mutex> lock(gcFinishedCondMutex);
    isGcStarted.store(false, std::memory_order_relaxed);
    if (gcIndex != GCTask::ASYNC_TASK_INDEX) { // sync gc, need set taskIndex
        finishedGcIndex.store(gcIndex);
    }
    gcFinishedCondVar.notify_all();
    BroadcastGCCompletion();
}

void CollectorResources::WaitForGCFinish()
{
    uint64_t startTime = TimeUtil::MicroSeconds();
    std::unique_lock<std::mutex> lock(gcFinishedCondMutex);
    uint64_t curWaitGcIndex = finishedGcIndex.load();
    std::function<bool()> pred = [this, curWaitGcIndex] {
        return (!IsGcStarted() || (curWaitGcIndex != finishedGcIndex) ||
                (finishedGcIndex == GCTask::TASK_INDEX_FOR_EXIT));
    };
#ifdef __OHOS__
    std::chrono::seconds waitTime(2); // 2 seconds
    gcFinishedCondVar.wait_for(lock, waitTime, pred);
#else
    gcFinishedCondVar.wait(lock, pred);
#endif
    uint64_t stopTime = TimeUtil::MicroSeconds();
    uint64_t diffTime = stopTime - startTime;
    VLOG(REPORT, "WaitForGCFinish cost %zu us", diffTime);
}

void CollectorResources::StartGCThreads()
{
    bool expected = false;
    if (gcThreadRunning.compare_exchange_strong(expected, true, std::memory_order_acquire) == false) {
        return;
    }
    // starts the thread pool.
    if (gcThreadPool == nullptr) {
        int32_t helperThreads = 1;
        gcThreadCount = helperThreads + 1;
        VLOG(REPORT, "total gc thread count %d, helper thread count %d", gcThreadCount, helperThreads);
        gcThreadPool = new (std::nothrow) GCThreadPool("gc", helperThreads, GCPoolThread::GC_THREAD_PRIORITY);
        CHECK_DETAIL(gcThreadPool != nullptr, "new GCThreadPool failed");
    }

    // create the collector thread.
    if (::pthread_create(&gcMainThread, nullptr, CollectorResources::GCMainThreadEntry, this) != 0) {
        MRT_ASSERT(0, "pthread_create failed!");
    }
    // set thread name.
#ifdef __WIN64
    int ret = pthread_setname_np(gcMainThread, "gc-main-thread");
    CHECK_E(UNLIKELY(ret != 0), "pthread_setname_np() in CollectorResources::StartGCThreads() return %d rather than 0",
            ret);
#endif
}

int32_t CollectorResources::GetGCThreadCount(const bool isConcurrent) const
{
    if (GetThreadPool() == nullptr) {
        return 1;
    }
    if (isConcurrent) {
        return gcThreadCount;
    }
    // default to 2
    return 2;
}

void CollectorResources::BroadcastGCCompletion()
{
    gcWorking = 0;
#if defined(_WIN64) || defined(__APPLE__)
    WakeWhenGCDone();
#else
    (void)Futex(&gcWorking, FUTEX_WAKE_PRIVATE, INT_MAX);
#endif
}

void CollectorResources::RequestHeapDump(GCTask::TaskType gcTask)
{
    TaskQueue<GCExecutor>::TaskFilter filter = [](GCExecutor&, GCExecutor&) { return false; };
    GCExecutor dumpTask = GCExecutor(gcTask);
    taskQueue->EnqueueSync(dumpTask, filter);
}

} // namespace MapleRuntime
