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
#ifndef PANDA_RUNTIME_MEM_MALLOC_PROXY_ALLOCATOR_H
#define PANDA_RUNTIME_MEM_MALLOC_PROXY_ALLOCATOR_H

#include <functional>
#include <map>
#include <memory>
#include <unordered_map>
#include <type_traits>

#include "libarkbase/mem/mem.h"
#include "libarkbase/mem/space.h"
#include "libarkbase/os/mutex.h"

namespace ark::mem {

using ark::Alignment;
class EmptyMemoryConfig;

/// @brief Class-proxy to the malloc, do some logging.
template <typename AllocConfigT>
class MallocProxyAllocator {
public:
    NO_COPY_SEMANTIC(MallocProxyAllocator);
    NO_MOVE_SEMANTIC(MallocProxyAllocator);

    explicit MallocProxyAllocator(MemStatsType *memStats, SpaceType typeAllocation = SpaceType::SPACE_TYPE_INTERNAL)
        : typeAllocation_(typeAllocation), memStats_ {memStats}
    {
    }

    ~MallocProxyAllocator();

    [[nodiscard]] void *Alloc(size_t size, Alignment align = DEFAULT_ALIGNMENT);

    template <class T>
    T *AllocArray(size_t size)
    {
        return static_cast<T *>(this->Alloc(sizeof(T) * size));
    }

    void Free(void *mem);

private:
    static constexpr bool DUMMY_ALLOC_CONFIG = std::is_same<AllocConfigT, EmptyMemoryConfig>::value;
    std::unordered_map<void *, size_t> allocatedMemory_;
    SpaceType typeAllocation_;
    MemStatsType *memStats_;
    os::memory::Mutex lock_;
};

}  // namespace ark::mem

#endif  // PANDA_RUNTIME_MEM_MALLOC_PROXY_ALLOCATOR_H
