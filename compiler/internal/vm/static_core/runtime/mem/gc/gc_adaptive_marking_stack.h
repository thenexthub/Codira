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
#ifndef PANDA_RUNTIME_MEM_GC_GC_ADAPTIVE_MARKING_STACK_H
#define PANDA_RUNTIME_MEM_GC_GC_ADAPTIVE_MARKING_STACK_H

#include "runtime/mem/gc/gc_root_type.h"
#include "runtime/mem/gc/workers/gc_workers_tasks.h"
#include "runtime/include/mem/panda_containers.h"
#include "runtime/mem/gc/gc_adaptive_stack.h"

namespace ark::mem {

class GC;

/*
 * Adaptive stack with GC workers support.
 * It will try to pop objects from the source stack and push
 * it to the destination stack. if the destination stack reaches the limit,
 * we will create a new task for worker.
 * If stack limit is equal to zero, it means that the destination stack is unlimited.
 */
class GCAdaptiveMarkingStack : public GCAdaptiveStack<ObjectHeader *> {
public:
    using ObjectVisitor = std::function<void(const ObjectHeader *)>;
    using MarkedObjects = PandaVector<PandaDeque<ObjectHeader *> *>;

    using GCAdaptiveStack<ObjectHeader *>::GCAdaptiveStack;

    NO_COPY_SEMANTIC(GCAdaptiveMarkingStack);
    NO_MOVE_SEMANTIC(GCAdaptiveMarkingStack);

    ~GCAdaptiveMarkingStack() override;

    /**
     * This method should be used when we find new object by field from another object.
     * @param from_object from which object we found object by reference, nullptr for roots
     * @param object object which will be added to the stack
     */
    void PushToStack(const ObjectHeader *fromObject, ObjectHeader *object);

    /**
     * This method should be used when we find new object as a root
     * @param root_type type of the root which we found
     * @param object object which will be added to the stack
     */
    void PushToStack(RootType rootType, ObjectHeader *object);

    /**
     * @brief Travers objects in stack via visitor and return marked objects.
     * Visitor can push in stack, but mustn't pop from it.
     */
    MarkedObjects MarkObjects(const ObjectVisitor &visitor);

    void *GetAdditionalMarkingInfo() const
    {
        return additionalMarkingInfo_;
    }

    void SetAdditionalMarkingInfo(void *infoPtr)
    {
        additionalMarkingInfo_ = infoPtr;
    }

protected:
    GCAdaptiveMarkingStack *CreateStack() override;
    GCWorkersTask CreateTask(GCAdaptiveStack<ObjectHeader *> *stack) override;

private:
    void *additionalMarkingInfo_ {nullptr};
};

}  // namespace ark::mem

#endif  // PANDA_RUNTIME_MEM_GC_GC_ADAPTIVE_MARKING_STACK_H
