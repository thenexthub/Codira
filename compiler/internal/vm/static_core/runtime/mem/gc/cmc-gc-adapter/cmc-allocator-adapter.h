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

#ifndef RUNTIME_MEM_GC_CMCGCADAPTER_CMC_ALLOCATOR_ADAPTER_H
#define RUNTIME_MEM_GC_CMCGCADAPTER_CMC_ALLOCATOR_ADAPTER_H

#include "runtime/include/mem/allocator.h"

namespace ark::mem {
class ObjectAllocConfigWithCrossingMap;
class ObjectAllocConfig;
class TLAB;

template <MTModeT MT_MODE>
class CMCObjectAllocatorAdapter final : public ObjectAllocatorNoGen<MT_MODE> {
public:
    NO_MOVE_SEMANTIC(CMCObjectAllocatorAdapter);
    NO_COPY_SEMANTIC(CMCObjectAllocatorAdapter);

    explicit CMCObjectAllocatorAdapter(MemStatsType *memStats, bool createPygoteSpaceAllocator);

    ~CMCObjectAllocatorAdapter() final = default;

    [[nodiscard]] void *Allocate(size_t size, Alignment align, [[maybe_unused]] ark::ManagedThread *thread,
                                 ObjectAllocatorBase::ObjMemInitPolicy objInit, bool pinned) override;

    [[nodiscard]] void *AllocateNonMovable(size_t size, Alignment align, ark::ManagedThread *thread,
                                           ObjectAllocatorBase::ObjMemInitPolicy objInit) override;

    void IterateOverObjectsSafe([[maybe_unused]] const ObjectVisitor &objectVisitor) override;

    bool IsNonMovable([[maybe_unused]] const ObjectHeader *obj) override
    {
        return false;
    }
};

}  // namespace ark::mem
#endif  // RUNTIME_MEM_GC_CMCGCADAPTER_CMC_ALLOCATOR_ADAPTER_H
