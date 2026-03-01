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
#ifndef PANDA_RUNTIME_MEM_GC_G1_G1_EVACUATE_REGIONS_TASK_H
#define PANDA_RUNTIME_MEM_GC_G1_G1_EVACUATE_REGIONS_TASK_H

#include "runtime/mem/gc/workers/gc_workers_tasks.h"

namespace ark::mem {

class GcThreadScope {
public:
    explicit GcThreadScope(Thread *gcThread) : previous_(Thread::GetCurrent())
    {
        Thread::SetCurrent(gcThread);
    }

    ~GcThreadScope()
    {
        Thread::SetCurrent(previous_);
    }

    DEFAULT_COPY_SEMANTIC(GcThreadScope);
    DEFAULT_MOVE_SEMANTIC(GcThreadScope);

private:
    Thread *previous_;
};

template <typename Ref>
class GCEvacuateRegionsTaskStack;

template <typename Ref>
class G1EvacuateRegionsTask : public GCWorkersTask {
public:
    using StackType = GCEvacuateRegionsTaskStack<Ref>;
    explicit G1EvacuateRegionsTask(StackType *markingStack)
        : GCWorkersTask(GCWorkersTaskTypes::TASK_EVACUATE_REGIONS, markingStack)
    {
    }

    DEFAULT_COPY_SEMANTIC(G1EvacuateRegionsTask);
    DEFAULT_MOVE_SEMANTIC(G1EvacuateRegionsTask);
    ~G1EvacuateRegionsTask() = default;

    StackType *GetMarkingStack() const
    {
        return static_cast<StackType *>(storage_);
    }
};
}  // namespace ark::mem
#endif
