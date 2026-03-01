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
#ifndef PANDA_RUNTIME_COMPILER_QUEUE_SIMPLE_H_
#define PANDA_RUNTIME_COMPILER_QUEUE_SIMPLE_H_

#include "runtime/compiler_queue_interface.h"
#include "runtime/include/mem/panda_containers.h"

namespace ark {

/**
 * The simple queue works as FIFO without any other logic: it stores all tasks to a list and extracts the first one.
 * There is no length limit or expiring condition. Mostly useful for debugging, as is more deterministic.
 * This queue is thread unsafe (should be used under lock).
 */
class CompilerQueueSimple : public CompilerQueueInterface {
public:
    explicit CompilerQueueSimple(mem::InternalAllocatorPtr allocator) : queue_(allocator->Adapter()) {}

    CompilerTask GetTask() override
    {
        if (queue_.empty()) {
            LOG(DEBUG, COMPILATION_QUEUE) << "Empty " << queueName_ << ", return nothing";
            return CompilerTask();
        }
        auto task = std::move(queue_.front());
        queue_.pop_front();
        LOG(DEBUG, COMPILATION_QUEUE) << "Extract a task from a " << queueName_ << ": " << GetTaskDescription(task);
        return task;
    }

    // NOLINTNEXTLINE(google-default-arguments)
    void AddTask(CompilerTask &&ctx, [[maybe_unused]] size_t priority = 0) override
    {
        LOG(DEBUG, COMPILATION_QUEUE) << "Add task to a " << queueName_ << ": " << GetTaskDescription(ctx);
        queue_.push_front(std::move(ctx));
    }

    void Finalize() override
    {
        // Nothing to deallocate
        LOG(DEBUG, COMPILATION_QUEUE) << "Clear a " << queueName_;
        queue_.clear();
    }

protected:
    size_t GetQueueSize() override
    {
        return queue_.size();
    }

private:
    PandaList<CompilerTask> queue_;
    const char *queueName_ = "simple compilation queue";
};

}  // namespace ark

#endif  // PANDA_RUNTIME_COMPILER_QUEUE_H_
