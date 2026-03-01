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

#include "libarkbase/mem/mem_pool.h"
#include "malloc_mem_pool-inl.h"
#include "mmap_mem_pool-inl.h"
#include "pool_manager.h"
#include "libarkbase/utils/logger.h"

namespace ark {

// default is mmap_mem_pool_
PoolType PoolManager::poolType_ = PoolType::MMAP;
bool PoolManager::isInitialized_ = false;
MallocMemPool *PoolManager::mallocMemPool_ = nullptr;
MmapMemPool *PoolManager::mmapMemPool_ = nullptr;

Arena *PoolManager::AllocArena(size_t size, SpaceType spaceType, AllocatorType allocatorType, const void *allocatorAddr)
{
    if (poolType_ == PoolType::MMAP) {
        return mmapMemPool_->template AllocArenaImpl<Arena, OSPagesAllocPolicy::NO_POLICY>(
            size, spaceType, allocatorType, allocatorAddr);
    }
    return mallocMemPool_->template AllocArenaImpl<Arena, OSPagesAllocPolicy::NO_POLICY>(size, spaceType, allocatorType,
                                                                                         allocatorAddr);
}

void PoolManager::FreeArena(Arena *arena)
{
    if (poolType_ == PoolType::MMAP) {
        return mmapMemPool_->template FreeArenaImpl<Arena, OSPagesPolicy::IMMEDIATE_RETURN>(arena);
    }
    return mallocMemPool_->template FreeArenaImpl<Arena, OSPagesPolicy::IMMEDIATE_RETURN>(arena);
}

void PoolManager::Initialize(PoolType type)
{
    ASSERT(!isInitialized_);
    isInitialized_ = true;
    poolType_ = type;
    if (poolType_ == PoolType::MMAP) {
        mmapMemPool_ = new MmapMemPool();
    } else {
        mallocMemPool_ = new MallocMemPool();
    }
    LOG(DEBUG, ALLOC) << "PoolManager Initialized";
}

MmapMemPool *PoolManager::GetMmapMemPool()
{
    ASSERT(isInitialized_);
    ASSERT(poolType_ == PoolType::MMAP);
    return mmapMemPool_;
}

MallocMemPool *PoolManager::GetMallocMemPool()
{
    ASSERT(isInitialized_);
    ASSERT(poolType_ == PoolType::MALLOC);
    return mallocMemPool_;
}

void PoolManager::Finalize()
{
    ASSERT(isInitialized_);
    isInitialized_ = false;
    if (poolType_ == PoolType::MMAP) {
        delete mmapMemPool_;
        mmapMemPool_ = nullptr;
    } else {
        delete mallocMemPool_;
        mallocMemPool_ = nullptr;
    }
}

}  // namespace ark
