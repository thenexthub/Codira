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
#ifndef PANDA_RUNTIME_MEM_GC_G1_GC_EVACUATE_REGIONS_TASK_STACK_H
#define PANDA_RUNTIME_MEM_GC_G1_GC_EVACUATE_REGIONS_TASK_STACK_H

#include "runtime/mem/gc/gc_adaptive_stack.h"
#include "runtime/include/mem/allocator.h"
#include "runtime/mem/gc/g1/g1-evacuate-regions-task.h"

namespace ark::mem {
template <typename Ref>
class GCEvacuateRegionsTaskStack : public GCAdaptiveStack<Ref> {
public:
    using GCAdaptiveStack<Ref>::GCAdaptiveStack;

    NO_COPY_SEMANTIC(GCEvacuateRegionsTaskStack);
    NO_MOVE_SEMANTIC(GCEvacuateRegionsTaskStack);

    ~GCEvacuateRegionsTaskStack() override = default;

protected:
    using Base = GCAdaptiveStack<Ref>;

    GCEvacuateRegionsTaskStack<Ref> *CreateStack() override
    {
        auto *gc = this->GetGC();
        auto allocator = gc->GetInternalAllocator();
        return allocator->template New<GCEvacuateRegionsTaskStack<Ref>>(
            gc, this->GetNewTaskStackSizeLimit(), this->GetNewTaskStackSizeLimit(), this->GetTaskType(),
            this->GetTimeLimitForNewTaskCreation(), this->GetStackDst());
    }

    GCWorkersTask CreateTask(GCAdaptiveStack<Ref> *stack) override
    {
        return G1EvacuateRegionsTask<Ref>(static_cast<GCEvacuateRegionsTaskStack<Ref> *>(stack));
    }
};

}  // namespace ark::mem
#endif
