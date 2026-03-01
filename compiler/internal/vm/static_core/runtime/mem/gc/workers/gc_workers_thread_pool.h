/**
 * Copyright (c) 2021-2024 Huawei Device Co., Ltd.
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 * http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#ifndef PANDA_RUNTIME_MEM_GC_GC_WORKERS_THREAD_POOL_H
#define PANDA_RUNTIME_MEM_GC_GC_WORKERS_THREAD_POOL_H

#include "runtime/include/thread.h"
#include "runtime/thread_pool.h"
#include "runtime/mem/gc/workers/gc_workers_task_pool.h"

namespace ark::mem {
// Forward declaration for GCWorkersProcessor
class GCWorkersThreadPool;

class GCWorkersProcessor : public ProcessorInterface<GCWorkersTask, GCWorkersThreadPool *> {
public:
    explicit GCWorkersProcessor(GCWorkersThreadPool *gcThreadsPools) : gcThreadsPools_(gcThreadsPools) {}

    ~GCWorkersProcessor() override = default;
    NO_COPY_SEMANTIC(GCWorkersProcessor);
    NO_MOVE_SEMANTIC(GCWorkersProcessor);

    bool Process(GCWorkersTask &&task) override;
    bool Init() override;
    bool Destroy() override;

private:
    GCWorkersThreadPool *gcThreadsPools_;
    void *workerData_ {nullptr};
};

class GCWorkersQueueSimple : public TaskQueueInterface<GCWorkersTask> {
public:
    explicit GCWorkersQueueSimple(mem::InternalAllocatorPtr allocator, size_t queueLimit)
        : TaskQueueInterface<GCWorkersTask>(queueLimit), queue_(allocator->Adapter())
    {
    }

    ~GCWorkersQueueSimple() override = default;
    NO_COPY_SEMANTIC(GCWorkersQueueSimple);
    NO_MOVE_SEMANTIC(GCWorkersQueueSimple);

    GCWorkersTask GetTask() override
    {
        if (queue_.empty()) {
            LOG(DEBUG, GC) << "Empty " << queueName_ << ", return nothing";
            return GCWorkersTask();
        }
        auto task = queue_.front();
        queue_.pop_front();
        LOG(DEBUG, GC) << "Extract a task from a " << queueName_ << ": " << GetTaskDescription(task);
        return task;
    }

    // NOLINTNEXTLINE(google-default-arguments)
    void AddTask(GCWorkersTask &&ctx, [[maybe_unused]] size_t priority = 0) override
    {
        LOG(DEBUG, GC) << "Add task to a " << queueName_ << ": " << GetTaskDescription(ctx);
        queue_.push_front(ctx);
    }

    void Finalize() override
    {
        // Nothing to deallocate
        LOG(DEBUG, GC) << "Clear a " << queueName_;
        queue_.clear();
    }

protected:
    PandaString GetTaskDescription(const GCWorkersTask &ctx)
    {
        PandaOStringStream stream;
        stream << GCWorkersTaskTypesToString(ctx.GetType());
        return stream.str();
    }

    size_t GetQueueSize() override
    {
        return queue_.size();
    }

private:
    PandaList<GCWorkersTask> queue_;
    const char *queueName_ = "simple gc workers task queue";
};

class GCWorkersCreationInterface : public WorkerCreationInterface {
public:
    explicit GCWorkersCreationInterface(PandaVM *vm) : gcThread_(vm, Thread::ThreadType::THREAD_TYPE_GC)
    {
        ASSERT(vm != nullptr);
    }

    ~GCWorkersCreationInterface() override = default;
    NO_COPY_SEMANTIC(GCWorkersCreationInterface);
    NO_MOVE_SEMANTIC(GCWorkersCreationInterface);

    void AttachWorker(bool helperThread) override
    {
        if (!helperThread) {
            Thread::SetCurrent(&gcThread_);
        }
    }
    void DetachWorker(bool helperThread) override
    {
        if (!helperThread) {
            Thread::SetCurrent(nullptr);
        }
    }

private:
    Thread gcThread_;
};

/// @brief GC workers task pool based on internal thread pool
class GCWorkersThreadPool final : public GCWorkersTaskPool {
public:
    NO_COPY_SEMANTIC(GCWorkersThreadPool);
    NO_MOVE_SEMANTIC(GCWorkersThreadPool);
    explicit GCWorkersThreadPool(GC *gc, size_t threadsCount = 0);
    ~GCWorkersThreadPool() final;

    void SetAffinityForGCWorkers();

    void UnsetAffinityForGCWorkers();

private:
    /**
     * @brief Try to add new gc workers task to thread pool
     * @param task gc workers task
     * @return true if task successfully added to thread pool, false - otherwise
     */
    bool TryAddTask(GCWorkersTask &&task) final;

    /**
     * @brief Try to get gc task from gc workers task queue and
     * run it in the current thread to help gc workers from thread pool.
     * Do nothing if couldn't get task
     * @see GCWorkersQueueSimple
     */
    void RunInCurrentThread() final;

    /* GC thread pool specific variables */
    ThreadPool<GCWorkersTask, GCWorkersProcessor, GCWorkersThreadPool *> *threadPool_;
    GCWorkersQueueSimple *queue_;
    GCWorkersCreationInterface *workerIface_;
    mem::InternalAllocatorPtr internalAllocator_;
    const size_t threadsCount_;

    friend class GCWorkersProcessor;
};

}  // namespace ark::mem

#endif  // PANDA_RUNTIME_MEM_GC_GC_WORKERS_THREAD_POOL_H
