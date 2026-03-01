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

#include "mem/pool_manager.h"

#include "mem/malloc_mem_pool-inl.h"
#include "mem/mmap_mem_pool-inl.h"

namespace panda {

// default is mmap_mem_pool
PoolType PoolManager::pool_type = PoolType::MMAP;
bool PoolManager::is_initialized = false;
MallocMemPool *PoolManager::malloc_mem_pool = nullptr;
MmapMemPool *PoolManager::mmap_mem_pool = nullptr;

Arena *PoolManager::AllocArena(size_t size, SpaceType space_type, AllocatorType allocator_type,
                               const void *allocator_addr)
{
    if (pool_type == PoolType::MMAP) {
        return mmap_mem_pool->AllocArenaImpl(size, space_type, allocator_type, allocator_addr);
    }
    return malloc_mem_pool->AllocArenaImpl(size, space_type, allocator_type, allocator_addr);
}

void PoolManager::FreeArena(Arena *arena)
{
    if (pool_type == PoolType::MMAP) {
        return mmap_mem_pool->FreeArenaImpl(arena);
    }
    return malloc_mem_pool->FreeArenaImpl(arena);
}

void PoolManager::Initialize(PoolType type)
{
    ASSERT(!is_initialized);
    is_initialized = true;
    pool_type = type;
    if (pool_type == PoolType::MMAP) {
        mmap_mem_pool = new MmapMemPool();
    } else {
        malloc_mem_pool = new MallocMemPool();
    }
    LOG(DEBUG, ALLOC) << "PoolManager Initialized";
}

MmapMemPool *PoolManager::GetMmapMemPool()
{
    ASSERT(is_initialized);
    ASSERT(pool_type == PoolType::MMAP);
    return mmap_mem_pool;
}

MallocMemPool *PoolManager::GetMallocMemPool()
{
    ASSERT(is_initialized);
    ASSERT(pool_type == PoolType::MALLOC);
    return malloc_mem_pool;
}

void PoolManager::Finalize()
{
    ASSERT(is_initialized);
    is_initialized = false;
    if (pool_type == PoolType::MMAP) {
        delete mmap_mem_pool;
        mmap_mem_pool = nullptr;
    } else {
        delete malloc_mem_pool;
        malloc_mem_pool = nullptr;
    }
}

}  // namespace panda
