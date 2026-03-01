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
#ifndef RUNTIME_MEM_PANDA_PYGOTE_SPACE_ALLOCATOR_H
#define RUNTIME_MEM_PANDA_PYGOTE_SPACE_ALLOCATOR_H

#include <functional>
#include <memory>
#include <vector>

#include "libarkbase/macros.h"
#include "libarkbase/mem/arena-inl.h"
#include "libarkbase/mem/mem.h"
#include "libarkbase/mem/mem_range.h"
#include "runtime/mem/runslots_allocator.h"
#include "runtime/mem/gc/bitmap.h"
#include "runtime/mem/heap_space.h"

namespace ark {
class ObjectHeader;
}  // namespace ark

namespace ark::mem {

enum PygoteSpaceState {
    STATE_PYGOTE_INIT,     // before pygote fork, used for small non-movable objects
    STATE_PYGOTE_FORKING,  // at first pygote fork, allocate for copied objects
    STATE_PYGOTE_FORKED,   // after fork, can't allocate/free objects in it
};

using BitmapList = std::vector<MarkBitmap *>;
template <typename AllocConfigT>
class PygoteSpaceAllocator final {
public:
    DEFAULT_NOEXCEPT_MOVE_SEMANTIC(PygoteSpaceAllocator);
    NO_COPY_SEMANTIC(PygoteSpaceAllocator);
    explicit PygoteSpaceAllocator(MemStatsType *memStats);
    ~PygoteSpaceAllocator();
    void *Alloc(size_t size, Alignment alignment = ark::DEFAULT_ALIGNMENT);

    void Free(void *mem);

    void SetState(PygoteSpaceState newState);

    PygoteSpaceState GetState() const
    {
        return state_;
    }

    static constexpr size_t GetMaxSize()
    {
        return RunSlotsAllocator<AllocConfigT>::GetMaxSize();
    }

    bool CanAllocNonMovable(size_t size, Alignment align)
    {
        return state_ == STATE_PYGOTE_INIT && AlignUp(size, GetAlignmentInBytes(align)) <= GetMaxSize();
    }

    bool ContainObject(const ObjectHeader *object);

    bool IsLive(const ObjectHeader *object);

    void ClearLiveBitmaps();

    BitmapList &GetLiveBitmaps()
    {
        // return live_bitmaps as mark bitmap for gc,
        // gc will update it at end of gc process
        return liveBitmaps_;
    }

    template <typename Visitor>
    void IterateOverObjectsInRange(const Visitor &visitor, void *start, void *end);

    void IterateOverObjects(const ObjectVisitor &objectVisitor);

    void VisitAndRemoveAllPools(const MemVisitor &memVisitor);

    void VisitAndRemoveFreePools(const MemVisitor &memVisitor);

    void Collect(const GCObjectVisitor &gcVisitor);

    void SetHeapSpace(HeapSpace *heapSpace)
    {
        heapSpace_ = heapSpace;
    }

private:
    void CreateLiveBitmap(void *heapBegin, size_t heapSize);
    RunSlotsAllocator<AllocConfigT> runslotsAlloc_;
    Arena *arena_ = nullptr;
    SpaceType spaceType_ = SpaceType::SPACE_TYPE_OBJECT;
    PygoteSpaceState state_ = STATE_PYGOTE_INIT;
    BitmapList liveBitmaps_;
    MemStatsType *memStats_;
    HeapSpace *heapSpace_ {nullptr};
};

}  // namespace ark::mem

#endif  // RUNTIME_MEM_PANDA_PYGOTE_SPACE_ALLOCATOR_H
