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
#ifndef PANDA_RUNTIME_COMPILER_WORKER_H
#define PANDA_RUNTIME_COMPILER_WORKER_H

#include "libarkbase/macros.h"
#include "libarkbase/os/mutex.h"
#include "runtime/include/mem/allocator.h"
#include "runtime/include/compiler_interface.h"

namespace ark {

class Compiler;

/// @brief Compiler worker interface
class CompilerWorker {
public:
    CompilerWorker(mem::InternalAllocatorPtr internalAllocator, Compiler *compiler)
        : internalAllocator_(internalAllocator), compiler_(compiler)
    {
    }

    NO_COPY_SEMANTIC(CompilerWorker);
    NO_MOVE_SEMANTIC(CompilerWorker);

    virtual ~CompilerWorker() = default;

    /**
     * @brief Initializes the compiler worker.
     * After that, you can start to add tasks to the compiler worker
     */
    virtual void InitializeWorker() = 0;

    /**
     * @brief Finalizes the compiler worker.
     * After that, task queue is empty and you must not call the AddTask method
     */
    virtual void FinalizeWorker() = 0;

    /**
     * @brief Atomically closes the worker task queue
     * and waiting for all tasks from the queue to be processed.
     * This method is thread-safe with respect to the AddTask method
     */
    virtual void JoinWorker() = 0;

    /**
     * @brief Returns state of the compiler worker
     * @return true if task queue is closed or
     * the worker is not initialized, false - otherwise
     */
    virtual bool IsWorkerJoined() = 0;

    /**
     * @brief Adds task in the compiler worker task queue.
     * This method should be used after InitializeWorker and
     * it has no effect after JoinWorker.
     */
    virtual void AddTask(CompilerTask &&task) = 0;

protected:
    mem::InternalAllocatorPtr internalAllocator_;  // NOLINT(misc-non-private-member-variables-in-classes)
    Compiler *compiler_;                           // NOLINT(misc-non-private-member-variables-in-classes)
};

}  // namespace ark

#endif  // PANDA_RUNTIME_COMPILER_WORKER_H
