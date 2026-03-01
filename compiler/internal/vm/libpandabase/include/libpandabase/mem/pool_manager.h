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

#ifndef LIBPANDABASE_MEM_POOL_MANAGER_H
#define LIBPANDABASE_MEM_POOL_MANAGER_H

#include "malloc_mem_pool.h"
#include "mmap_mem_pool.h"
#include "arena-inl.h"

namespace panda {
enum class PoolType { MALLOC, MMAP };

class PoolManager {
public:
    ~PoolManager() = default;
    PoolManager() = default;
    DEFAULT_NOEXCEPT_MOVE_SEMANTIC(PoolManager);
    DEFAULT_COPY_SEMANTIC(PoolManager);
    static void Initialize(PoolType type = PoolType::MMAP);
    static Arena *AllocArena(size_t size, SpaceType space_type, AllocatorType allocator_type,
                             const void *allocator_addr = nullptr);
    static void FreeArena(Arena *arena);
    static MmapMemPool *GetMmapMemPool();
    static MallocMemPool *GetMallocMemPool();

    static void Finalize();

private:
    static bool is_initialized;
    static PoolType pool_type;
    static MallocMemPool *malloc_mem_pool;
    static MmapMemPool *mmap_mem_pool;
};

}  // namespace panda

#endif  // LIBPANDABASE_MEM_POOL_MANAGER_H
