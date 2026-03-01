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

#include "runtime/mem/gc/gc_adaptive_marking_stack.h"
#include "runtime/mem/gc/gc.h"
#include "runtime/mem/gc/workers/gc_workers_task_pool.h"
#include "runtime/mem/gc/gc_adaptive_stack_inl.h"

namespace ark::mem {

GCAdaptiveMarkingStack::~GCAdaptiveMarkingStack()
{
    auto allocator = GetGC()->GetInternalAllocator();
    allocator->Delete(additionalMarkingInfo_);
}

void GCAdaptiveMarkingStack::PushToStack(const ObjectHeader *fromObject, ObjectHeader *object)
{
    LOG(DEBUG, GC) << "Add object to stack: " << GetDebugInfoAboutObject(object)
                   << " accessed from object: " << fromObject;
    ValidateObject(fromObject, object);
    GCAdaptiveStack<ObjectHeader *>::PushToStack(object);
}

void GCAdaptiveMarkingStack::PushToStack(RootType rootType, ObjectHeader *object)
{
    LOG(DEBUG, GC) << "Add object to stack: " << GetDebugInfoAboutObject(object) << " accessed as a root: " << rootType;
    ValidateObject(rootType, object);
    GCAdaptiveStack<ObjectHeader *>::PushToStack(object);
}

GCAdaptiveMarkingStack *GCAdaptiveMarkingStack::CreateStack()
{
    auto *gc = GetGC();
    auto allocator = gc->GetInternalAllocator();
    // New tasks will be created with the same new_task_stack_size_limit_ and stack_size_limit_
    return allocator->New<GCAdaptiveMarkingStack>(gc, GetNewTaskStackSizeLimit(), GetNewTaskStackSizeLimit(),
                                                  GetTaskType(), GetTimeLimitForNewTaskCreation(), GetStackDst());
}

GCWorkersTask GCAdaptiveMarkingStack::CreateTask(GCAdaptiveStack<ObjectHeader *> *stack)
{
    return GCMarkWorkersTask(GetTaskType(), static_cast<GCAdaptiveMarkingStack *>(stack));
}

GCAdaptiveMarkingStack::MarkedObjects GCAdaptiveMarkingStack::MarkObjects(
    const GCAdaptiveMarkingStack::ObjectVisitor &visitor)
{
    // We need this to avoid allocation of new stack and fragmentation
    static constexpr size_t BUCKET_SIZE = 16;
    auto *gc = GetGC();
    auto allocator = gc->GetInternalAllocator();
    MarkedObjects markedObjects;
    PandaDeque<ObjectHeader *> *tailMarkedObjects =
        allocator->template New<PandaDeque<ObjectHeader *>>(allocator->Adapter());
    if (GetStackSrc()->empty()) {
        std::swap(GetStackSrc(), GetStackDst());
    }
    while (!Empty()) {
        [[maybe_unused]] auto stackSrcSize = GetStackSrc()->size();
        for (auto *object : *GetStackSrc()) {
            visitor(object);
            // visitor mustn't pop from stack
            ASSERT(stackSrcSize == GetStackSrc()->size());
        }
        if (LIKELY(GetStackSrc()->size() > BUCKET_SIZE)) {
            markedObjects.push_back(GetStackSrc());
            GetStackSrc() = allocator->template New<PandaDeque<ObjectHeader *>>(allocator->Adapter());
        } else {
            tailMarkedObjects->insert(tailMarkedObjects->end(), GetStackSrc()->begin(), GetStackSrc()->end());
            GetStackSrc()->clear();
        }
        std::swap(GetStackSrc(), GetStackDst());
    }
    if (!tailMarkedObjects->empty()) {
        markedObjects.push_back(tailMarkedObjects);
    } else {
        allocator->Delete(tailMarkedObjects);
    }
    return markedObjects;
}
}  // namespace ark::mem
