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

#ifndef PANDA_RUNTIME_MEM_GC_GC_ADAPTIVE_STACK_INL_H
#define PANDA_RUNTIME_MEM_GC_GC_ADAPTIVE_STACK_INL_H

#include "runtime/mem/gc/gc_adaptive_stack.h"
#include "runtime/mem/gc/gc.h"
#include "runtime/mem/gc/workers/gc_workers_task_pool.h"

namespace ark::mem {

template <typename Ref>
GCAdaptiveStack<Ref>::GCAdaptiveStack(GC *gc, size_t stackSizeLimit, size_t newTaskStackSizeLimit,
                                      GCWorkersTaskTypes task, uint64_t timeLimitForNewTaskCreation,
                                      PandaDeque<Ref> *stackSrc)
    : stackSizeLimit_(stackSizeLimit),
      newTaskStackSizeLimit_(newTaskStackSizeLimit),
      timeLimitForNewTaskCreation_(timeLimitForNewTaskCreation),
      taskType_(task),
      gc_(gc)
{
    initialStackSizeLimit_ = stackSizeLimit_;
    auto allocator = gc_->GetInternalAllocator();
    ASSERT((stackSizeLimit == 0) || (gc->GetWorkersTaskPool() != nullptr));
    if (stackSrc != nullptr) {
        stackSrc_ = stackSrc;
    } else {
        stackSrc_ = allocator->template New<PandaDeque<Ref>>(allocator->Adapter());
    }
    stackDst_ = allocator->template New<PandaDeque<Ref>>(allocator->Adapter());
}

template <typename Ref>
GCAdaptiveStack<Ref>::~GCAdaptiveStack()
{
    gc_->GetInternalAllocator()->Delete(stackSrc_);
    gc_->GetInternalAllocator()->Delete(stackDst_);
}

template <typename Ref>
bool GCAdaptiveStack<Ref>::Empty() const
{
    return stackSrc_->empty() && stackDst_->empty();
}

template <typename Ref>
size_t GCAdaptiveStack<Ref>::Size() const
{
    return stackSrc_->size() + stackDst_->size();
}

template <typename Ref>
PandaDeque<Ref> *GCAdaptiveStack<Ref>::MoveStacksPointers()
{
    ASSERT(stackSrc_ != nullptr);
    auto *returnValue = stackSrc_;
    stackSrc_ = nullptr;
    return returnValue;
}

template <typename Ref>
void GCAdaptiveStack<Ref>::PushToStack(Ref element)
{
    ASSERT_PRINT(IsAddressInObjectsHeap(element), element);
    if ((stackSizeLimit_ > 0) && ((stackDst_->size() + 1) == stackSizeLimit_)) {
        // Try to create a new task only once
        // Create a new stack and send a new task to GC
        LOG(DEBUG, GC) << "GCAdaptiveStack: Try to add new task " << GCWorkersTaskTypesToString(taskType_)
                       << " for worker";
        ASSERT(gc_->GetWorkersTaskPool() != nullptr);
        ASSERT(taskType_ != GCWorkersTaskTypes::TASK_EMPTY);
        auto allocator = gc_->GetInternalAllocator();
        auto *newStack = CreateStack();
        ASSERT(newStack != nullptr);
        if (gc_->GetWorkersTaskPool()->AddTask(CreateTask(newStack))) {
            LOG(DEBUG, GC) << "GCAdaptiveStack: Successfully add new task " << GCWorkersTaskTypesToString(taskType_)
                           << " for worker";
            stackDst_ = allocator->template New<PandaDeque<Ref>>(allocator->Adapter());
        } else {
            // We will try to create a new task later
            stackSizeLimit_ += stackSizeLimit_;
            LOG(DEBUG, GC) << "GCAdaptiveStack: Failed to add new task " << GCWorkersTaskTypesToString(taskType_)
                           << " for worker";
            [[maybe_unused]] auto srcStack = newStack->MoveStacksPointers();
            ASSERT(srcStack == stackDst_);
            allocator->Delete(newStack);
        }
        if (IsHighTaskCreationRate()) {
            stackSizeLimit_ += stackSizeLimit_;
        }
    }
    stackDst_->push_back(element);
}

template <typename Ref>
bool GCAdaptiveStack<Ref>::IsHighTaskCreationRate()
{
    if (createdTasks_ == 0) {
        startTime_ = ark::os::time::GetClockTimeInThreadCpuTime();
    }
    createdTasks_++;
    if (tasksForTimeCheck_ == createdTasks_) {
        uint64_t curTime = ark::os::time::GetClockTimeInThreadCpuTime();
        ASSERT(curTime >= startTime_);
        uint64_t totalTimeConsumed = curTime - startTime_;
        uint64_t oneTaskConsumed = totalTimeConsumed / createdTasks_;
        LOG(DEBUG, GC) << "Created " << createdTasks_ << " tasks in " << Timing::PrettyTimeNs(totalTimeConsumed);
        if (oneTaskConsumed < timeLimitForNewTaskCreation_) {
            createdTasks_ = 0;
            startTime_ = 0;
            return true;
        }
    }
    return false;
}

template <typename Ref>
Ref GCAdaptiveStack<Ref>::PopFromStack()
{
    if (stackSrc_->empty()) {
        ASSERT(!stackDst_->empty());
        auto temp = stackSrc_;
        stackSrc_ = stackDst_;
        stackDst_ = temp;
        if (stackSizeLimit_ != 0) {
            // We may increase the current limit, so return it to the default value
            stackSizeLimit_ = initialStackSizeLimit_;
        }
    }
    ASSERT(!stackSrc_->empty());
    auto *element = stackSrc_->back();
    stackSrc_->pop_back();
    return element;
}

template <typename Ref>
template <typename Handler>
void GCAdaptiveStack<Ref>::TraverseObjects(Handler &handler)
{
    if (stackSrc_->empty()) {
        std::swap(stackSrc_, stackDst_);
    }
    while (!Empty()) {
        [[maybe_unused]] auto stackSrcSize = stackSrc_->size();
        for (auto ref : *stackSrc_) {
            handler.ProcessRef(ref);
            // visitor mustn't pop from stack
            ASSERT(stackSrcSize == stackSrc_->size());
        }
        stackSrc_->clear();
        std::swap(stackSrc_, stackDst_);
    }
}

template <typename Ref>
bool GCAdaptiveStack<Ref>::IsWorkersTaskSupported() const
{
    return taskType_ != GCWorkersTaskTypes::TASK_EMPTY;
}

template <typename Ref>
void GCAdaptiveStack<Ref>::Clear()
{
    *stackSrc_ = PandaDeque<Ref>();
    *stackDst_ = PandaDeque<Ref>();
}
}  // namespace ark::mem
#endif  // PANDA_RUNTIME_MEM_GC_GC_ADAPTIVE_STACK_INL_H
