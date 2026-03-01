/**
 * Copyright (c) 2023-2024 Huawei Device Co., Ltd.
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

#ifndef PANDA_RUNTIME_MEM_GC_WORKERS_GC_WORKERS_TASK_POOL_H
#define PANDA_RUNTIME_MEM_GC_WORKERS_GC_WORKERS_TASK_POOL_H

#include "runtime/mem/gc/gc.h"
#include "runtime/mem/gc/workers/gc_workers_tasks.h"

namespace ark::mem {

/// @brief Base abstract class for GC workers task pool
class GCWorkersTaskPool {
public:
    explicit GCWorkersTaskPool(GC *gc) : gc_(gc) {}
    NO_COPY_SEMANTIC(GCWorkersTaskPool);
    NO_MOVE_SEMANTIC(GCWorkersTaskPool);
    virtual ~GCWorkersTaskPool() = default;

    /**
     * @brief Add new GCWorkerTask to gc workers task pool
     * @param task new gc worker task for pool
     * @see GCWorkerTask
     *
     * @return true if task successfully added to pool, false - otherwise
     */
    bool AddTask(GCWorkersTask &&task);

    /**
     * @brief Add new GCWorkerTask to gc workers task pool by gc task type
     * @param type type of new gc worker task for pool
     * @see GCWorkerTaskTypes
     *
     * @return true if task successfully added to pool, false - otherwise
     */
    ALWAYS_INLINE bool AddTask(GCWorkersTaskTypes type)
    {
        return AddTask(GCWorkersTask(type));
    }

    /// @return pointer to used GC
    ALWAYS_INLINE GC *GetGC() const
    {
        return gc_;
    }

    /// @brief Wait until all GC sended tasks will be solved
    void WaitUntilTasksEnd();

protected:
    /**
     * @brief If gc task pool is not empty then try to get gc workers task from gc task pool
     * and run it in the current thread.
     * It helps to gc workers
     */
    virtual void RunInCurrentThread() = 0;

    /**
     * @brief Try to add new gc workers task based on specific implementation
     * @param task gc worker task for pool
     *
     * @return true if task successfully added to pool, false - otherwise
     */
    virtual bool TryAddTask(GCWorkersTask &&task) = 0;

    /**
     * @brief Run sended gc workers task from gc task pool on a worker
     * @param task sended gc workers task
     * @param worker_data specific data for one worker if needed
     */
    void RunGCWorkersTask(GCWorkersTask *task, void *workerData = nullptr);

private:
    // Wait for all sended tasks, time in ms
    static constexpr uint64_t ALL_GC_TASKS_FINISH_WAIT_TIMEOUT = 1U;

    void IncreaseSolvedTasks();

    ALWAYS_INLINE void ResetTasks() REQUIRES(allSolvedTasksCondVarLock_)
    {
        solvedTasks_ = 0U;
        sendedTasks_ = 0U;
        solvedTasksSnapshot_ = 0U;
    }

    GC *gc_ {nullptr};

    os::memory::Mutex allSolvedTasksCondVarLock_;
    /**
     * @brief Conditional varible is used for waiting for all gc tasks at some point
     * @see WaitUntilTasksEnd
     * @see IncreaseSolvedTasks
     */
    os::memory::ConditionVariable allSolvedTasksCondVar_ GUARDED_BY(allSolvedTasksCondVarLock_);
    // CC-OFFNXT(G.FMT.03) project code style
    size_t solvedTasksSnapshot_ GUARDED_BY(allSolvedTasksCondVarLock_) {0U};
    std::atomic_size_t solvedTasks_ {0U};
    std::atomic_size_t sendedTasks_ {0U};
};

}  // namespace ark::mem

#endif  // PANDA_RUNTIME_MEM_GC_WORKERS_GC_WORKERS_TASK_POOL_H
