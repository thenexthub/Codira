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


#include "Heap/GcThreadPool.h"

#if defined(__linux__) || defined(hongmeng) || defined(__APPLE__)
#include <sys/resource.h>
#endif
#include <sched.h>

#include "Base/Log.h"
#include "Base/Panic.h"
#include "Base/SysCall.h"
#include "Mutator/MutatorManager.h"
#include "securec.h"

// thread pool implementation
namespace MapleRuntime {
GCPoolThread::GCPoolThread(GCThreadPool* threadPool, const char* threadName, size_t threadId, size_t stackSize)
    : id(threadId), tid(-1), name(threadName), pool(threadPool)
{
    pthread_attr_t attr;
    CHECK_PTHREAD_CALL(pthread_attr_init, (&attr), "");
    CHECK_PTHREAD_CALL(pthread_attr_setstacksize, (&attr, stackSize), stackSize);
    CHECK_PTHREAD_CALL(pthread_create, (&pthread, nullptr, &WorkerFunc, this), "GCPoolThread init");
#ifdef __WIN64
    CHECK_PTHREAD_CALL(pthread_setname_np, (pthread, threadName), "GCPoolThread SetName");
#endif
    CHECK_PTHREAD_CALL(pthread_attr_destroy, (&attr), "GCPoolThread init");
}

GCPoolThread::~GCPoolThread()
{
    CHECK_PTHREAD_CALL(pthread_join, (pthread, nullptr), "thread deinit");
    pool = nullptr;
}

#if defined(__linux__) || defined(hongmeng)
void GCPoolThread::SetPriority(int32_t priority) const { SetThreadPriority(tid, priority); }

void GCPoolThread::SetThreadPriority(pid_t tid, int32_t priority)
{
    errno = 0;
    int ret = ::setpriority(static_cast<int>(PRIO_PROCESS), static_cast<uint32_t>(tid), priority);
    // strerror_r does not work as expected: outputs nothing for __amd64__
    CHECK_E(UNLIKELY(ret != 0 && errno != 0), "::setpriority(tid %d, priority %d) failed: %s", tid, priority,
            ::strerror(errno));
}
#endif

void* GCPoolThread::WorkerFunc(void* param)
{
    // set current thread as a gc thread.
    ThreadLocal::SetThreadType(ThreadType::GC_THREAD);

    GCPoolThread* thread = reinterpret_cast<GCPoolThread*>(param);
    GCThreadPool* pool = thread->pool;

#ifdef __APPLE__
    CHECK_PTHREAD_CALL(pthread_setname_np, (thread->name.Str()), "GCPoolThread SetName");
#elif defined(__linux__) || defined(hongmeng)
    CHECK_PTHREAD_CALL(prctl, (PR_SET_NAME, thread->name.Str()), "GCPoolThread SetName");
#endif

#if defined(__linux__) || defined(hongmeng)
    thread->tid = MapleRuntime::GetTid();
    SetThreadPriority(thread->tid, pool->priority);
#endif

    while (!pool->IsExited()) {
        HeapWork* task = nullptr;
        {
            std::unique_lock<std::mutex> poolLock(pool->poolMutex);
            // hang up in threadSleepingCondVar when pool stopped or to many active thread
            while (((pool->currActiveThreadNum > pool->maxActiveThreadNum) || !pool->IsRunning()) &&
                   !pool->IsExited()) {
                // currActiveThreadNum start at maxThreadNum, dec before thread hangup in sleeping state
                --(pool->currActiveThreadNum);
                if (pool->currActiveThreadNum == 0) {
                    // all thread sleeping, pool in stop state, notify wait stop thread
                    pool->allThreadStopped.notify_all();
                }
                pool->threadSleepingCondVar.wait(poolLock);
                ++(pool->currActiveThreadNum);
            }
            // if no task available thread hung up in taskEmptyCondVar
            while (pool->workQueue.empty() && pool->IsRunning() && !pool->IsExited()) {
                // currExecuteThreadNum start at 0, inc before thread wait for task
                ++(pool->waitingThreadNum);
                if (pool->waitingThreadNum == pool->maxActiveThreadNum) {
                    // all task is done, notify wait finish thread
                    pool->allWorkDoneCondVar.notify_all();
                }
                pool->taskEmptyCondVar.wait(poolLock);
                --(pool->waitingThreadNum);
            }
            if (!pool->workQueue.empty() && pool->IsRunning() && !pool->IsExited()) {
                task = pool->workQueue.front();
                pool->workQueue.pop();
            }
        }
        if (task != nullptr) {
            task->Execute(thread->id);
            delete task;
        }
    }
    {
        std::unique_lock<std::mutex> poolLock(pool->poolMutex);
        --(pool->currActiveThreadNum);
        if (pool->currActiveThreadNum == 0) {
            // all thread sleeping, pool in stop state, notify wait stop thread
            pool->allThreadStopped.notify_all();
        }
    }
    return nullptr;
}

const int MAX_NAME_LEN = 256;

GCThreadPool::GCThreadPool(const char* poolName, int32_t threadNum, int32_t prior)
    : priority(prior), name(poolName), running(false), exit(false), maxThreadNum(threadNum),
      maxActiveThreadNum(threadNum), currActiveThreadNum(maxThreadNum), waitingThreadNum(0)
{
    // init and start thread
    char threadName[MAX_NAME_LEN];
    for (int32_t i = 0; i < maxThreadNum; ++i) {
        // threadID 0 is main thread, sub threadID start at 1
        errno_t ret = snprintf_s(threadName, MAX_NAME_LEN, (MAX_NAME_LEN - 1), "%s-pool-t%d", poolName, (i + 1));
        CHECK_E(ret < 0, "snprintf_s name = %s threadId%d in GCThreadPool::GCThreadPool return %d rather than 0.",
                name.Str(), (i + 1), ret);
        // default Sleeping
        GCPoolThread* threadItem = new (std::nothrow) GCPoolThread(this, threadName, (i + 1), DEFAULT_STACK_SIZE);
        CHECK_DETAIL(threadItem != nullptr, "new GCPoolThread failed");
        threads.push_back(threadItem);
    }
    // pool init in stop state
    Stop();
    LOG(RTLOG_DEBUG, "GCThreadPool init");
}

void GCThreadPool::Exit()
{
    std::unique_lock<std::mutex> poolLock(poolMutex);
    // set pool exit flag
    exit.store(true, std::memory_order_relaxed);

    // notify all waiting thread exit
    taskEmptyCondVar.notify_all();
    // notify all stopped thread exit
    threadSleepingCondVar.notify_all();

    // notify all WaitFinish thread return
    allWorkDoneCondVar.notify_all();
    // notify all WaitStop thread return
    allThreadStopped.notify_all();
    LOG(RTLOG_DEBUG, "GCThreadPool Exit");
}

GCThreadPool::~GCThreadPool()
{
    // wait until threads exit
    for (auto thread : threads) {
        delete thread;
    }
    threads.clear();
    ClearAllWork();
}

#if defined(__linux__) || defined(hongmeng)
void GCThreadPool::SetPriority(int32_t prior) const
{
    for (auto thread : threads) {
        thread->SetPriority(prior);
    }
}
#endif

void GCThreadPool::SetMaxActiveThreadNum(int32_t num)
{
    std::unique_lock<std::mutex> poolLock(poolMutex);
    int32_t oldNum = maxActiveThreadNum;
    if (num >= maxThreadNum) {
        maxActiveThreadNum = maxThreadNum;
    } else if (num > 0) {
        maxActiveThreadNum = num;
    } else {
        LOG(RTLOG_ERROR, "SetMaxActiveThreadNum invalid input val");
        return;
    }
    // active more thread get to work when pool is running
    if ((maxActiveThreadNum > oldNum) && (waitingThreadNum > 0) && IsRunning()) {
        threadSleepingCondVar.notify_all();
    }
}

void GCThreadPool::AddWork(HeapWork* task)
{
    CHECK_DETAIL(task != nullptr, "failed to add a null task");
    std::unique_lock<std::mutex> poolLock(poolMutex);
    workQueue.push(task);
    // do not notify when pool isn't running, notify_all in start
    // notify if there is active thread waiting for task
    if (IsRunning() && (waitingThreadNum > 0)) {
        taskEmptyCondVar.notify_one();
    }
}

void GCThreadPool::Start()
{
    // notify all sleeping threads get to work
    std::unique_lock<std::mutex> poolLock(poolMutex);
    running.store(true, std::memory_order_relaxed);
    threadSleepingCondVar.notify_all();
}

void GCThreadPool::DrainWorkQueue()
{
    MRT_ASSERT(!IsRunning(), "thread pool is running");
    HeapWork* task = nullptr;
    do {
        task = nullptr;
        poolMutex.lock();
        if (!workQueue.empty()) {
            task = workQueue.front();
            workQueue.pop();
        }
        poolMutex.unlock();
        if (task != nullptr) {
            task->Execute(0);
            delete task;
        }
    } while (task != nullptr);
}

void GCThreadPool::WaitFinish()
{
    HeapWork* task = nullptr;
    do {
        task = nullptr;
        poolMutex.lock();
        if (!workQueue.empty() && IsRunning() && !IsExited()) {
            task = workQueue.front();
            workQueue.pop();
        }
        poolMutex.unlock();
        if (task != nullptr) {
            task->Execute(0);
            delete task;
        }
    } while (task != nullptr);

    // wait all task execute finish
    // waitingThreadNum == maxActiveThreadNum indicate all work done
    // no need to wait when pool stopped or exited
    {
        std::unique_lock<std::mutex> poolLock(poolMutex);
        while ((waitingThreadNum != maxActiveThreadNum) && IsRunning() && !IsExited()) {
            allWorkDoneCondVar.wait(poolLock);
        }
    }
    // let all thread sleeping for next start
    // if threads not in sleeping mode, next start signal may be missed
    Stop();
    // clean up task in GC thread, thread pool might receive "exit" in stop thread
    DrainWorkQueue();
}

void GCThreadPool::Stop()
{
    // notify & wait all thread enter stopped state
    std::unique_lock<std::mutex> poolLock(poolMutex);
    running.store(false, std::memory_order_relaxed);
    taskEmptyCondVar.notify_all();
    while (currActiveThreadNum != 0) {
        allThreadStopped.wait(poolLock);
    }
}

void GCThreadPool::ClearAllWork()
{
    std::unique_lock<std::mutex> poolLock(poolMutex);
    while (!workQueue.empty()) {
        HeapWork* task = workQueue.front();
        workQueue.pop();
        delete task;
    }
}

#if defined(MRT_DEBUG) && (MRT_DEBUG == 1)
ScopedCpuTime::ScopedCpuTime(GCThreadPool& threadPool, LogType type) : logType(type)
{
    if (ENABLE_LOG(logType)) {
#if defined(__linux__) || defined(hongmeng)
        threadCount = threadPool.GetMaxThreadNum() + 1;
        cid.resize(threadCount);
        workerStart.resize(threadCount);

        pthread_getcpuclockid(pthread_self(), &cid[0]);
        clock_gettime(cid[0], &workerStart[0]);
        size_t index = 1;
        for (auto worker : threadPool.GetThreads()) {
            pthread_t thread = worker->GetThread();
            pthread_getcpuclockid(thread, &cid[index]);
            clock_gettime(cid[index], &workerStart[index]);
            ++index;
        }
#else
        VLOG(logType, "ScopedCpuTime is not supported in windows yet.");
#endif
    }
}

ScopedCpuTime::~ScopedCpuTime()
{
    if (ENABLE_LOG(logType)) {
#if defined(__linux__) || defined(hongmeng)
        struct timespec workerEnd[threadCount];
        uint64_t workerCpuTime[threadCount];
        for (size_t i = 0; i < threadCount; ++i) {
            clock_gettime(cid[i], &workerEnd[i]);
            workerCpuTime[i] = static_cast<uint64_t>(workerEnd[i].tv_sec - workerStart[i].tv_sec) *
                    MapleRuntime::SECOND_TO_NANO_SECOND +
                static_cast<uint64_t>((workerEnd[i].tv_nsec - workerStart[i].tv_nsec));
        }
        for (size_t i = 0; i < threadCount; ++i) {
            VLOG(logType, "worker %zu cputime: %lu,\t", i, workerCpuTime[i]);
        }
#endif
    }
}
#endif // DEBUG
} // namespace MapleRuntime
