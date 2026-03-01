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
#ifndef PANDA_RUNTIME_MEM_GC_GC_ADAPTIVE_STACK_H
#define PANDA_RUNTIME_MEM_GC_GC_ADAPTIVE_STACK_H

#include "runtime/mem/gc/gc_root_type.h"
#include "runtime/mem/gc/workers/gc_workers_tasks.h"
#include "runtime/include/mem/panda_containers.h"
#include "runtime/mem/gc/g1/object_ref.h"

namespace ark::mem {

class GC;

/*
 * Adaptive stack with GC workers support.
 * It will try to pop objects from the source stack and push
 * it to the destination stack. if the destination stack reaches the limit,
 * we will create a new task for worker.
 * If stack limit is equal to zero, it means that the destination stack is unlimited.
 */
template <typename Ref>
class GCAdaptiveStack {
public:
    explicit GCAdaptiveStack(GC *gc, size_t stackSizeLimit = 0, size_t newTaskStackSizeLimit = 0,
                             GCWorkersTaskTypes task = GCWorkersTaskTypes::TASK_EMPTY,
                             uint64_t timeLimitForNewTaskCreation = 0, PandaDeque<Ref> *stackSrc = nullptr);

    virtual ~GCAdaptiveStack();
    NO_COPY_SEMANTIC(GCAdaptiveStack);
    NO_MOVE_SEMANTIC(GCAdaptiveStack);

    /**
     * @brief Add new object into destination stack.
     * If we set the limit for stack, we will create a new task for
     * GC workers with it.
     */
    void PushToStack(Ref element);

    /**
     * @brief Pop an object from source stack.
     * If the source stack is empty, we will swap it with destination stack
     * and return an object from it.
     */
    Ref PopFromStack();

    /**
     * @brief Travers objects in stack via visitor and return marked objects.
     * Visitor can push in stack, but mustn't pop from it.
     */
    template <typename Handler>
    void TraverseObjects(Handler &handler);

    /// @brief Check that destination or source stack has at least one object.
    bool Empty() const;
    /// @brief Returns the sum of destination and source stacks sizes.
    size_t Size() const;

    /**
     * @brief Set the src stack pointer to nullptr.
     * Should be used if we decide to free it not on destructor of this instance.
     */
    PandaDeque<Ref> *MoveStacksPointers();

    /// @brief Returns true if stack supports parallel workers, false otherwise
    bool IsWorkersTaskSupported() const;

    /// @brief Remove all elements from stack
    void Clear();

    void SetTaskType(GCWorkersTaskTypes taskType)
    {
        taskType_ = taskType;
    }

    GCWorkersTaskTypes GetTaskType() const
    {
        return taskType_;
    }

protected:
    virtual GCAdaptiveStack<Ref> *CreateStack() = 0;
    virtual GCWorkersTask CreateTask(GCAdaptiveStack<Ref> *stack) = 0;

    PandaDeque<Ref> *&GetStackSrc()
    {
        return stackSrc_;
    }

    PandaDeque<Ref> *&GetStackDst()
    {
        return stackDst_;
    }

    GC *GetGC()
    {
        return gc_;
    }

    size_t GetNewTaskStackSizeLimit() const
    {
        return newTaskStackSizeLimit_;
    }

    uint64_t GetTimeLimitForNewTaskCreation() const
    {
        return timeLimitForNewTaskCreation_;
    }

private:
    static constexpr size_t DEFAULT_TASKS_FOR_TIME_CHECK = 10;

    /**
     * @brief Check tasks creation rate.
     * If we create tasks too frequently, return true.
     */
    bool IsHighTaskCreationRate();

private:
    PandaDeque<Ref> *stackSrc_;
    PandaDeque<Ref> *stackDst_;
    size_t stackSizeLimit_ {0};
    size_t initialStackSizeLimit_ {0};
    size_t newTaskStackSizeLimit_ {0};
    uint64_t startTime_ {0};
    uint64_t timeLimitForNewTaskCreation_ {0};
    size_t tasksForTimeCheck_ {DEFAULT_TASKS_FOR_TIME_CHECK};
    size_t createdTasks_ {0};
    GCWorkersTaskTypes taskType_;
    GC *gc_;
};
}  // namespace ark::mem

#endif  // PANDA_RUNTIME_MEM_GC_GC_ADAPTIVE_STACK_H
