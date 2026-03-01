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
#ifndef PANDA_RUNTIME_COMPILER_QUEUE_COUNTER_PRIORITY_H_
#define PANDA_RUNTIME_COMPILER_QUEUE_COUNTER_PRIORITY_H_

#include <algorithm>
#include <cstring>

#include "libarkbase/utils/time.h"
#include "runtime/compiler_queue_interface.h"
#include "runtime/include/mem/panda_containers.h"
#include "runtime/include/method-inl.h"

namespace ark {

class CompilationQueueElement {
public:
    explicit CompilationQueueElement(CompilerTask &&task) : context_(std::move(task))
    {
        timestamp_ = time::GetCurrentTimeInMillis();
        counter_ = context_.GetMethod()->GetHotnessCounter();
    }

    int64_t GetCounter() const
    {
        // Do not return method->counter as it can changed during sorting
        return counter_;
    }

    const CompilerTask &GetContext() const
    {
        return context_;
    }

    CompilerTask &GetContext()
    {
        return context_;
    }

    uint64_t GetTimestamp() const
    {
        return timestamp_;
    }

    void UpdateCounter(int64_t newCounter)
    {
        counter_ = newCounter;
    }

private:
    uint64_t timestamp_;
    int64_t counter_;
    CompilerTask context_;
};

/**
 * The counter priority queue sorts all tasks (methods) by its hotness counters.
 * It extracts the most hot method for compilation (the lesser counter).
 * It also has a limit of tasks for compilation. If the task is too old, it is expired and removed from the queue.
 * Note, in case of skip the method due to length limit, its hotness counter is reset.
 * Max length and life time is configured.
 * This queue is thread unsafe (should be used under lock).
 */
class CompilerPriorityCounterQueue : public CompilerQueueInterface {
public:
    explicit CompilerPriorityCounterQueue(mem::InternalAllocatorPtr allocator, uint64_t maxLength,
                                          uint64_t taskLifeSpan)
        : allocator_(allocator), queue_(allocator->Adapter())
    {
        maxLength_ = maxLength;
        taskLifeSpan_ = taskLifeSpan;
        queueName_ = "";
        SetQueueName("priority counter compilation queue");
    }

    CompilerTask GetTask() override
    {
        UpdateQueue();
        if (queue_.empty()) {
            LOG(DEBUG, COMPILATION_QUEUE) << "Empty " << queueName_ << ", return nothing";
            return CompilerTask();
        }
        sort(queue_.begin(), queue_.end(), comparator_);
        auto element = queue_.back();
        auto task = std::move(element->GetContext());
        queue_.pop_back();
        allocator_->Delete(element);
        LOG(DEBUG, COMPILATION_QUEUE) << "Extract a task from a " << queueName_ << ": " << GetTaskDescription(task);
        return task;
    }

    // NOLINTNEXTLINE(google-default-arguments)
    void AddTask(CompilerTask &&ctx, [[maybe_unused]] size_t priority = 0) override
    {
        UpdateQueue();
        if (queue_.size() >= maxLength_) {
            // Not sure if it is possible to exceed the size more than one
            // Reset the counter of the rejected method;
            ctx.GetMethod()->ResetHotnessCounter();
            ctx.GetMethod()->AtomicSetCompilationStatus(Method::WAITING,
                                                        ctx.IsOsr() ? Method::COMPILED : Method::NOT_COMPILED);
            // Maybe, replace the other method in queue?
            LOG(DEBUG, COMPILATION_QUEUE) << "Skip adding the task " << GetTaskDescription(ctx)
                                          << " due to limit of tasks (" << maxLength_ << ") in a " << queueName_;
            return;
        }
        LOG(DEBUG, COMPILATION_QUEUE) << "Add an element to a " << queueName_ << ": " << GetTaskDescription(ctx);
        auto element = allocator_->New<CompilationQueueElement>(std::move(ctx));
        // Sorting will be in Get function
        queue_.push_back(element);
    }

    void Finalize() override
    {
        for (auto e : queue_) {
            allocator_->Delete(e);
        }
        queue_.clear();
        LOG(DEBUG, COMPILATION_QUEUE) << "Clear a " << queueName_;
    }

protected:
    size_t GetQueueSize() override
    {
        return queue_.size();
    }

    virtual bool UpdateCounterAndCheck(CompilationQueueElement *element)
    {
        // The only way to update counter
        element->UpdateCounter(element->GetContext().GetMethod()->GetHotnessCounter());
        uint64_t curStamp = time::GetCurrentTimeInMillis();
        return (curStamp - element->GetTimestamp() >= taskLifeSpan_);
    }

    void SetQueueName(char const *name)
    {
        queueName_ = name;
    }

private:
    class Comparator {
    public:
        bool operator()(CompilationQueueElement *a, CompilationQueueElement *b) const
        {
            if (a->GetCounter() == b->GetCounter()) {
                // Use method name just in case?
                if (a->GetTimestamp() == b->GetTimestamp()) {
                    // The only way is a name
                    // Again, as we pull from the end return reversed compare
                    return strcmp(reinterpret_cast<const char *>(a->GetContext().GetMethod()->GetName().data),
                                  reinterpret_cast<const char *>(b->GetContext().GetMethod()->GetName().data)) > 0;
                }
                // Low time is high priority (pull from the end)
                return a->GetTimestamp() > b->GetTimestamp();
            }
            // First, handle a method with higher hotness (less counter)
            return (a->GetCounter() > b->GetCounter());
        }
    };

    void UpdateQueue()
    {
        // Remove expired tasks
        for (auto it = queue_.begin(); it != queue_.end();) {
            auto element = *it;
            // We should update the counter inside as the queue choose the semantic of the counter by itself
            if (UpdateCounterAndCheck(element)) {
                LOG(DEBUG, COMPILATION_QUEUE) << "Remove an expired element from a " << queueName_ << ": "
                                              << GetTaskDescription(element->GetContext());
                auto ctx = std::move(element->GetContext());
                ASSERT(ctx.GetMethod() != nullptr);
                ctx.GetMethod()->AtomicSetCompilationStatus(Method::WAITING,
                                                            ctx.IsOsr() ? Method::COMPILED : Method::NOT_COMPILED);
                allocator_->Delete(element);
                it = queue_.erase(it);
            } else {
                ++it;
            }
        }
    }
    mem::InternalAllocatorPtr allocator_;
    PandaVector<CompilationQueueElement *> queue_;
    Comparator comparator_;
    uint64_t maxLength_;
    // In milliseconds
    uint64_t taskLifeSpan_;
    const char *queueName_;
};

}  // namespace ark

#endif  // PANDA_RUNTIME_COMPILER_QUEUE_COUNTER_PRIORITY_H_
