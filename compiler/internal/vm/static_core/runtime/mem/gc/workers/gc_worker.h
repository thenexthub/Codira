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

#ifndef PANDA_RUNTIME_MEM_GC_WORKERS_GC_WORKER_H
#define PANDA_RUNTIME_MEM_GC_WORKERS_GC_WORKER_H

#include "libarkbase/taskmanager/task.h"
#include "runtime/mem/gc/gc_queue.h"

namespace ark {
// Forward declaration for GCWorker
class Thread;
}  // namespace ark

namespace ark::mem {
// Forward declaration for GCWorker
class GC;

/**
 * @class GCWorker represents GC-worker which runs GC tasks on a non-managed thread
 * @brief The class implements interaction between GC and GC-worker. GC-worker can be separate internal GC-thread or
 * a thread from TaskManager
 */
class GCWorker {
public:
    /**
     * @brief Create local gc tasks queue and structure for gc panda thread.
     * Don't start gc worker
     * @see ark::Thread
     * @see CreateAndStartWorker
     * @see FinalizeAndDestroyWorker
     *
     * @param gc poiner to used GC
     */
    explicit GCWorker(GC *gc);
    NO_COPY_SEMANTIC(GCWorker);
    NO_MOVE_SEMANTIC(GCWorker);
    ~GCWorker();

    /// @brief Create worker and start execution
    void CreateAndStartWorker();
    /// @brief Join (wait for) all worker tasks and destroy worker
    void FinalizeAndDestroyWorker();

    /**
     * @brief Add new gc task to GC worker. Try to add the task to local queue and then run it on a worker
     * @param task gc task for running on a worker
     * @return true if task was added to local task queue, false - otherwise
     */
    bool AddTask(PandaUniquePtr<GCTask> task);

    /// @brief Add all postponed tasks to gc queue
    void OnPostponeGCEnd();

private:
    PandaUniquePtr<GCTask> GetTask();
    void RunGC(PandaUniquePtr<GCTask> task);

    /* Internal thread specific functions */

    static void GCThreadLoop(GCWorker *gcWorker);

    /* TaskManager specific functions */

    void CreateAndAddTaskToTaskManager();
    void GCTaskRunner();

    GC *gc_ {nullptr};
    GCQueueInterface *gcTaskQueue_ {nullptr};
    Thread *gcThread_ {nullptr};
    uint32_t collectNumberMod_ {1U};
    /* Internal thread specific variables */
    std::thread *gcInternalThread_ {nullptr};
    /* TaskManager specific variables */
    taskmanager::RunnerCallback gcRunner_ {nullptr};
    std::atomic_bool needToFinish_ {false};
    os::memory::Mutex gcTaskRunMutex_;
    os::memory::Mutex postponedTasksMutex_;
    PandaQueue<PandaUniquePtr<GCTask>> postponedTasks_ GUARDED_BY(postponedTasksMutex_);
};

}  // namespace ark::mem

#endif  // PANDA_RUNTIME_MEM_GC_WORKERS_GC_WORKER_H
