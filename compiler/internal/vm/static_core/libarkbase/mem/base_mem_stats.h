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

#ifndef PANDA_BASE_MEM_STATS_H
#define PANDA_BASE_MEM_STATS_H

#include "libarkbase/os/mutex.h"
#include <cstdio>
#include "libarkbase/macros.h"
#include "space.h"

#include <array>
#include <atomic>

WEAK_FOR_LTO_START

namespace ark {

class BaseMemStats {
public:
    NO_COPY_SEMANTIC(BaseMemStats);
    NO_MOVE_SEMANTIC(BaseMemStats);

    BaseMemStats() = default;
    virtual ~BaseMemStats() = default;

    PANDA_PUBLIC_API void RecordAllocateRaw(size_t size, SpaceType typeMem);

    // NOTE(aemelenko): call RecordFreeRaw when CodeAllocator supports deallocate
    // NOTE(aemelenko): call RecordFreeRaw when ArenaAllocator supports deallocate
    PANDA_PUBLIC_API void RecordFreeRaw(size_t size, SpaceType typeMem);

    // getters
    [[nodiscard]] PANDA_PUBLIC_API uint64_t GetAllocated(SpaceType typeMem) const;
    [[nodiscard]] PANDA_PUBLIC_API uint64_t GetFreed(SpaceType typeMem) const;
    [[nodiscard]] PANDA_PUBLIC_API uint64_t GetFootprint(SpaceType typeMem) const;

    [[nodiscard]] PANDA_PUBLIC_API uint64_t GetAllocatedHeap() const;
    [[nodiscard]] PANDA_PUBLIC_API uint64_t GetFreedHeap() const;
    [[nodiscard]] PANDA_PUBLIC_API uint64_t GetFootprintHeap() const;
    [[nodiscard]] PANDA_PUBLIC_API uint64_t GetTotalFootprint() const;

protected:
    PANDA_PUBLIC_API void RecordAllocate(size_t size, SpaceType typeMem);
    PANDA_PUBLIC_API void RecordMoved(size_t size, SpaceType typeMem);
    PANDA_PUBLIC_API void RecordFree(size_t size, SpaceType typeMem);

private:
    std::array<std::atomic_uint64_t, SPACE_TYPE_SIZE> allocated_ {0};
    std::array<std::atomic_uint64_t, SPACE_TYPE_SIZE> freed_ {0};
};

}  // namespace ark

WEAK_FOR_LTO_END

#endif  // PANDA_BASE_MEM_STATS_H
