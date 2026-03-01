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

#include "libarkbase/mem/arena.h"
#include "libarkbase/mem/pool_manager.h"
#include "libarkbase/mem/mem.h"
#include "libarkbase/mem/mem_config.h"
#include "libarkbase/mem/mmap_mem_pool-inl.h"

#include "gtest/gtest.h"
#include "libarkbase/utils/logger.h"

namespace ark {

class ArenaTest : public testing::Test {
public:
    ArenaTest()
    {
        static constexpr size_t INTERNAL_SIZE = 16_MB;
        ark::mem::MemConfig::Initialize(0U, INTERNAL_SIZE, 0U, 0U, 0U, 0U);
        PoolManager::Initialize();
    }

    ~ArenaTest() override
    {
        PoolManager::Finalize();
        ark::mem::MemConfig::Finalize();
    }

    NO_MOVE_SEMANTIC(ArenaTest);
    NO_COPY_SEMANTIC(ArenaTest);

protected:
    static constexpr size_t ARENA_SIZE = 1_MB;

    template <class ArenaT>
    ArenaT *CreateArena(size_t size)
    {
        return PoolManager::GetMmapMemPool()->AllocArena<ArenaT>(size, SpaceType::SPACE_TYPE_INTERNAL,
                                                                 AllocatorType::ARENA_ALLOCATOR);
    }

    template <class ArenaT>
    void GetOccupiedAndFreeSizeTestImplementation(size_t arenaSize, size_t allocSize)
    {
        ASSERT_TRUE(arenaSize != 0U);
        ASSERT_TRUE(allocSize != 0U);
        auto *arena = CreateArena<ArenaT>(arenaSize);
        size_t oldFreeSize = arena->GetFreeSize();
        ASSERT_TRUE(arena->Alloc(allocSize) != nullptr);
        ASSERT_TRUE(arena->GetOccupiedSize() == allocSize);
        ASSERT_TRUE(oldFreeSize - allocSize == arena->GetFreeSize());
    }

    template <class ArenaT>
    void ResizeAndResetTestImplementation(size_t arenaSize, size_t allocSize)
    {
        constexpr const int64_t IMM_TWO = 2;
        ASSERT_TRUE(arenaSize != 0U);
        ASSERT_TRUE(allocSize != 0U);
        auto *arena = CreateArena<ArenaT>(arenaSize);
        ASSERT_TRUE(allocSize * IMM_TWO <= arena->GetFreeSize());
        void *firstAllocation = arena->Alloc(allocSize);
        void *secondAllocation = arena->Alloc(allocSize);
        ASSERT_TRUE(firstAllocation != nullptr);
        ASSERT_TRUE(firstAllocation != nullptr);
        ASSERT_TRUE(arena->GetOccupiedSize() == IMM_TWO * allocSize);
        arena->Resize(allocSize);
        ASSERT_TRUE(arena->GetOccupiedSize() == allocSize);
        void *thirdAllocation = arena->Alloc(allocSize);
        // we expect that we get the same address
        ASSERT_TRUE(ToUintPtr(secondAllocation) == ToUintPtr(thirdAllocation));
        ASSERT_TRUE(arena->GetOccupiedSize() == IMM_TWO * allocSize);
        arena->Reset();
        ASSERT_TRUE(arena->GetOccupiedSize() == 0U);
    }
};

TEST_F(ArenaTest, GetOccupiedAndFreeSizeTest)
{
    static constexpr size_t ALLOC_SIZE = AlignUp(ARENA_SIZE / 2U, GetAlignmentInBytes(ARENA_DEFAULT_ALIGNMENT));
    static constexpr Alignment ARENA_ALIGNMENT = LOG_ALIGN_4;
    static constexpr size_t ALIGNED_ALLOC_SIZE = AlignUp(ALLOC_SIZE, GetAlignmentInBytes(ARENA_ALIGNMENT));
    GetOccupiedAndFreeSizeTestImplementation<Arena>(ARENA_SIZE, ALLOC_SIZE);
    GetOccupiedAndFreeSizeTestImplementation<AlignedArena<ARENA_ALIGNMENT>>(ARENA_SIZE, ALIGNED_ALLOC_SIZE);
    GetOccupiedAndFreeSizeTestImplementation<DoubleLinkedAlignedArena<ARENA_ALIGNMENT>>(ARENA_SIZE, ALIGNED_ALLOC_SIZE);
}

TEST_F(ArenaTest, ResizeAndResetTest)
{
    static constexpr size_t ALLOC_SIZE = AlignUp(ARENA_SIZE / 3U, GetAlignmentInBytes(ARENA_DEFAULT_ALIGNMENT));
    static constexpr Alignment ARENA_ALIGNMENT = LOG_ALIGN_4;
    static constexpr size_t ALIGNED_ALLOC_SIZE = AlignUp(ALLOC_SIZE, GetAlignmentInBytes(ARENA_ALIGNMENT));
    ResizeAndResetTestImplementation<Arena>(ARENA_SIZE, ALLOC_SIZE);
    ResizeAndResetTestImplementation<AlignedArena<ARENA_ALIGNMENT>>(ARENA_SIZE, ALIGNED_ALLOC_SIZE);
    ResizeAndResetTestImplementation<DoubleLinkedAlignedArena<ARENA_ALIGNMENT>>(ARENA_SIZE, ALIGNED_ALLOC_SIZE);
}

}  // namespace ark
