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

#ifndef PANDA_RUNTIME_THREAD_POOL_H_
#define PANDA_RUNTIME_THREAD_POOL_H_

#include <atomic>
#include "libarkbase/os/mutex.h"
#include "libarkbase/os/thread.h"
#include "runtime/include/mem/allocator.h"
#include "runtime/include/mem/panda_containers.h"
#include "runtime/thread_pool_queue.h"

static constexpr uint64_t TASK_WAIT_TIMEOUT = 500U;

namespace ark {

template <typename Task, typename ProcArg>
class ProcessorInterface {
public:
    NO_COPY_SEMANTIC(ProcessorInterface);
    NO_MOVE_SEMANTIC(ProcessorInterface);

    ProcessorInterface() = default;
    virtual ~ProcessorInterface() = default;

    explicit ProcessorInterface(ProcArg args);
    virtual bool Process(Task &&) = 0;
    // before main loop
    virtual bool Init()
    {
        return true;
    }
    // before thread exit
    virtual bool Destroy()
    {
        return true;
    }

    void SetTid(int tid)
    {
        // Atomic with release order reason: threads should see correct initialization
        tid_.store(tid, std::memory_order_release);
    }

    bool IsStarted() const
    {
        return tid_ != -1;
    }

    int GetTid() const
    {
        // Atomic with acquire order reason: load should get the correct value
        return tid_.load(std::memory_order_acquire);
    }

private:
    std::atomic_int tid_ {-1};
};

class WorkerCreationInterface {
public:
    NO_COPY_SEMANTIC(WorkerCreationInterface);
    NO_MOVE_SEMANTIC(WorkerCreationInterface);
    WorkerCreationInterface() = default;
    virtual ~WorkerCreationInterface() = default;
    virtual void AttachWorker([[maybe_unused]] bool helperThread)
    {
        // do nothing here
    }
    virtual void DetachWorker([[maybe_unused]] bool helperThread)
    {
        // do nothing here
    }
};

template <typename Task, typename Proc, typename ProcArg>
class ThreadPool {
public:
    NO_COPY_SEMANTIC(ThreadPool);
    NO_MOVE_SEMANTIC(ThreadPool);

    explicit ThreadPool(mem::InternalAllocatorPtr allocator, TaskQueueInterface<Task> *queue, ProcArg args,
                        size_t nThreads = 1, const char *threadName = nullptr,
                        WorkerCreationInterface *workerCreationInterface = nullptr)
        : allocator_(allocator),
          queue_(queue),
          workers_(allocator_->Adapter()),
          procs_(allocator_->Adapter()),
          args_(args),
          isThreadActive_(allocator_->Adapter()),
          workerCreationInterface_(workerCreationInterface)
    {
        isActive_ = true;
        threadName_ = threadName;
        Scale(nThreads);
    }

    ~ThreadPool()
    {
        os::memory::LockHolder lock(scaleLock_);
        DeactivateWorkers();
        WaitForWorkers();
    }

    void Scale(size_t newNThreads)
    {
        os::memory::LockHolder scaleLock(scaleLock_);
        if (!IsActive()) {
            return;
        }
        LOG(DEBUG, RUNTIME) << "Scale thread pool for " << newNThreads << " new threads";
        if (newNThreads <= 0) {
            LOG(ERROR, RUNTIME) << "Incorrect number of threads " << newNThreads << " for thread pool";
            return;
        }
        if (newNThreads > threadsCounter_) {
            // Need to add new threads.
            {
                os::memory::LockHolder queueLock(queueLock_);
                isThreadActive_.resize(newNThreads);
            }
            for (size_t i = threadsCounter_; i < newNThreads; i++) {
                CreateNewThread(i);
            }
        } else if (newNThreads < threadsCounter_) {
            // Need to remove threads.
            for (size_t i = threadsCounter_ - 1; i >= newNThreads; i--) {
                StopWorker(workers_.back(), i);
                workers_.pop_back();
                allocator_->Delete(procs_.back());
                procs_.pop_back();
            }
            {
                os::memory::LockHolder queueLock(queueLock_);
                isThreadActive_.resize(newNThreads);
            }
        } else {
            // Same number of threads - do nothing.
        }
        threadsCounter_ = newNThreads;
        LOG(DEBUG, RUNTIME) << "Scale has been completed";
    }

    void Help()
    {
        // Disallow scaling while the main thread processes the queue
        os::memory::LockHolder scaleLock(scaleLock_);
        if (!IsActive()) {
            return;
        }
        auto *proc = allocator_->New<Proc>(args_);
        ASSERT(proc != nullptr);
        WorkerCreationInterface *iface = GetWorkerCreationInterface();
        if (iface != nullptr) {
            iface->AttachWorker(true);
        }
        if (!proc->Init()) {
            LOG(FATAL, RUNTIME) << "Cannot initialize worker thread";
        }
        proc->SetTid(os::thread::GetCurrentThreadId());
        while (true) {
            Task task;
            {
                os::memory::LockHolder lock(queueLock_);
                task = std::move(queue_->GetTask());
            }
            if (task.IsEmpty()) {
                break;
            }
            SignalTask();
            proc->Process(std::move(task));
        }
        if (!proc->Destroy()) {
            LOG(FATAL, RUNTIME) << "Cannot destroy worker thread";
        }
        if (iface != nullptr) {
            iface->DetachWorker(true);
        }
        allocator_->Delete(proc);
    }

    bool TryPutTask(Task &&task)
    {
        bool res = false;
        {
            os::memory::LockHolder lock(queueLock_);
            if (!isActive_) {
                return false;
            }
            res = queue_->TryAddTask(std::move(task));
        }
        if (res) {
            // Task was added.
            SignalTask();
        }
        return res;
    }

    bool PutTask(Task &&task)
    {
        {
            os::memory::LockHolder lock(queueLock_);
            if (!isActive_) {
                return false;
            }
            while (queue_->IsFull()) {
                WaitTask();
            }
            queue_->AddTask(std::move(task));
        }
        SignalTask();
        return true;
    }

    bool IsActive()
    {
        os::memory::LockHolder lock(queueLock_);
        return isActive_;
    }

    void Shutdown(bool force = false)
    {
        os::memory::LockHolder lock(scaleLock_);
        DeactivateWorkers();
        if (force) {
            // Sync.
            WaitForWorkers();
        }
    }

    void WaitTask()
    {
        condVar_.TimedWait(&queueLock_, TASK_WAIT_TIMEOUT);
    }

    static void WorkerEntry(ThreadPool<Task, Proc, ProcArg> *threadPool, Proc *proc, int i)
    {
        WorkerCreationInterface *iface = threadPool->GetWorkerCreationInterface();
        if (iface != nullptr) {
            iface->AttachWorker(false);
        }
        if (!proc->Init()) {
            LOG(FATAL, RUNTIME) << "Cannot initialize worker thread";
        }
        proc->SetTid(os::thread::GetCurrentThreadId());
        while (true) {
            Task task;
            {
                os::memory::LockHolder lock(threadPool->queueLock_);
                if (!threadPool->IsActive(i)) {
                    break;
                }
                task = std::move(threadPool->queue_->GetTask());
                if (task.IsEmpty()) {
                    threadPool->WaitTask();
                    continue;
                }
            }
            threadPool->SignalTask();
            LOG(DEBUG, RUNTIME) << "Worker " << i << " started to process task";
            proc->Process(std::move(task));
        }
        if (!proc->Destroy()) {
            LOG(FATAL, RUNTIME) << "Cannot destroy worker thread";
        }
        if (iface != nullptr) {
            iface->DetachWorker(false);
        }
        LOG(DEBUG, RUNTIME) << "Worker " << i << " is finished";
    }

    using ProcVisitor = void (*)(Proc *p, size_t data);

    void EnumerateProcs(ProcVisitor visitor, size_t data)
    {
        os::memory::LockHolder lock(scaleLock_);
        for (auto it : procs_) {
            visitor(it, data);
        }
    }

private:
    void SignalTask()
    {
        condVar_.Signal();
    }

    void SignalAllTasks()
    {
        condVar_.SignalAll();
    }

    void DeactivateWorkers()
    {
        os::memory::LockHolder lock(queueLock_);
        isActive_ = false;
        queue_->Finalize();
        SignalAllTasks();
        // NOLINTNEXTLINE(modernize-loop-convert)
        for (size_t i = 0; i < isThreadActive_.size(); i++) {
            isThreadActive_.at(i) = false;
        }
    }

    bool IsActive(int i) REQUIRES(queueLock_)
    {
        return isThreadActive_.at(i);
    }

    void WaitForWorkers() REQUIRES(scaleLock_)
    {
        for (auto worker : workers_) {
            StopWorker(worker);
        }
        {
            os::memory::LockHolder lock(queueLock_);
            isThreadActive_.clear();
        }
        workers_.clear();
        for (auto proc : procs_) {
            allocator_->Delete(proc);
        }
        procs_.clear();
    }

    void StopWorker(std::thread *worker, size_t threadId = 0) REQUIRES(scaleLock_)
    {
        if (worker != nullptr) {
            if (threadId != 0) {
                os::memory::LockHolder lock(queueLock_);
                isThreadActive_.at(threadId) = false;
            }
            SignalAllTasks();
            worker->join();
            allocator_->Delete(worker);
            worker = nullptr;
        }
    }

    void CreateNewThread(int i) REQUIRES(scaleLock_)
    {
        {
            os::memory::LockHolder lock(queueLock_);
            isThreadActive_.at(i) = true;
        }
        auto proc = allocator_->New<Proc>(args_);
        auto worker = allocator_->New<std::thread>(WorkerEntry, this, proc, i);
        if (worker == nullptr) {
            LOG(FATAL, RUNTIME) << "Cannot create a worker thread";
        }
        if (threadName_ != nullptr) {
            int res = os::thread::SetThreadName(worker->native_handle(), threadName_);
            if (res != 0) {
                LOG(ERROR, RUNTIME) << "Failed to set a name for the worker thread";
            }
        }
        workers_.emplace_back(worker);
        procs_.emplace_back(proc);
    }

    WorkerCreationInterface *GetWorkerCreationInterface()
    {
        return workerCreationInterface_;
    }

    mem::InternalAllocatorPtr allocator_;
    os::memory::ConditionVariable condVar_;
    TaskQueueInterface<Task> *queue_ GUARDED_BY(queueLock_);
    PandaList<std::thread *> workers_ GUARDED_BY(scaleLock_);
    size_t threadsCounter_ GUARDED_BY(scaleLock_) = 0;
    PandaList<Proc *> procs_ GUARDED_BY(scaleLock_);
    ProcArg args_;
    bool isActive_ GUARDED_BY(queueLock_) = false;
    os::memory::Mutex queueLock_;
    os::memory::Mutex scaleLock_;
    PandaVector<bool> isThreadActive_ GUARDED_BY(queueLock_);
    WorkerCreationInterface *workerCreationInterface_;
    const char *threadName_;
};

}  // namespace ark

#endif  // PANDA_RUNTIME_THREAD_POOL_H_
