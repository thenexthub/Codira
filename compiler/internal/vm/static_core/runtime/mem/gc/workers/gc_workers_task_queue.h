/**
 * Copyright (c) 2023-2025 Huawei Device Co., Ltd.
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

#ifndef PANDA_RUNTIME_MEM_GC_WORKERS_GC_WORKERS_TASK_QUEUE_H
#define PANDA_RUNTIME_MEM_GC_WORKERS_GC_WORKERS_TASK_QUEUE_H

#include "libarkbase/taskmanager/task.h"
#include "runtime/mem/gc/workers/gc_workers_task_pool.h"

namespace ark::mem {

/// @brief GC workers task pool based on common TaskManager (TaskQueue)
class GCWorkersTaskQueue final : public GCWorkersTaskPool {
public:
    explicit GCWorkersTaskQueue(GC *gc);
    NO_COPY_SEMANTIC(GCWorkersTaskQueue);
    NO_MOVE_SEMANTIC(GCWorkersTaskQueue);
    ~GCWorkersTaskQueue() final;

private:
    /**
     * @brief Try to add new gc workers task to gc task queue
     * @param task new gc workers task for gc task queue
     * @return true if task successfully added to task queue, false - otherwise
     */
    bool TryAddTask(GCWorkersTask &&task) final;

    /**
     * @brief Try to get gc task from gc task queue and
     * run it in the current thread to help workers from task manager.
     * Do nothing if couldn't get task
     */
    void RunInCurrentThread() final;
};

}  // namespace ark::mem

#endif  // PANDA_RUNTIME_MEM_GC_WORKERS_GC_WORKERS_TASK_QUEUE_H
