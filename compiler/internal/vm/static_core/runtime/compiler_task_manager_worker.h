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
#ifndef RUTNIME_COMPILER_TASK_MANAGER_WORKER_H
#define RUTNIME_COMPILER_TASK_MANAGER_WORKER_H

#include "runtime/compiler_worker.h"
#include "runtime/include/runtime.h"
#include "runtime/include/mem/panda_containers.h"
#include "libarkbase/taskmanager/task.h"
#include "libarkbase/taskmanager/task_queue.h"
#include "libarkbase/taskmanager/task_manager.h"

namespace ark {

/// @brief Compiler worker task pool based on common TaskManager (TaskQueue)
class CompilerTaskManagerWorker : public CompilerWorker {
public:
    CompilerTaskManagerWorker(mem::InternalAllocatorPtr internalAllocator, Compiler *compiler);

    NO_COPY_SEMANTIC(CompilerTaskManagerWorker);
    NO_MOVE_SEMANTIC(CompilerTaskManagerWorker);

    void InitializeWorker() override
    {
        compilerWorkerJoined_ = false;
    }

    void FinalizeWorker() override
    {
        JoinWorker();
    }

    void JoinWorker() override;

    bool IsWorkerJoined() override
    {
        return compilerWorkerJoined_;
    }

    void AddTask(CompilerTask &&task) override;

    ~CompilerTaskManagerWorker() override
    {
        taskmanager::TaskManager::DestroyTaskQueue<decltype(internalAllocator_->Adapter())>(compilerTaskManagerQueue_);
    }

private:
    void BackgroundCompileMethod(CompilerTask &&ctx);

    taskmanager::TaskQueueInterface *compilerTaskManagerQueue_ {nullptr};
    os::memory::Mutex taskQueueLock_;
    // This queue is used for methods need to be compiled inside TaskScheduler without compilation_lock_.
    PandaDeque<CompilerTask> compilerTaskDeque_ GUARDED_BY(taskQueueLock_);
    std::atomic<bool> compilerWorkerJoined_ {true};
};

}  // namespace ark

#endif  // RUTNIME_COMPILER_TASK_MANAGER_WORKER_H
