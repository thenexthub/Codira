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

#include <gtest/gtest.h>

#include "libarkbase/utils/logger.h"
#include "libarkbase/mem/mem_config.h"
#include "libarkbase/mem/pool_manager.h"
#include "runtime/mem/heap_space.h"

namespace ark::mem::test {

class HeapSpaceTest : public testing::Test {
public:
    explicit HeapSpaceTest() = default;

    NO_COPY_SEMANTIC(HeapSpaceTest);
    NO_MOVE_SEMANTIC(HeapSpaceTest);

    ~HeapSpaceTest() override = default;

protected:
    static constexpr size_t DEFAULT_TEST_HEAP_SIZE = 64_MB;
    static constexpr size_t DEFAULT_TEST_YOUNG_SIZE = 4_MB;
    static constexpr size_t DEFAULT_MIN_PERCENTAGE = 30U;
    static constexpr size_t DEFAULT_MAX_PERCENTAGE = 70U;

    size_t GetCurrentMaxSize() const
    {
        return heapSpace_->GetCurrentSize();
    }

    size_t GetCurrentYoungMaxSize() const
    {
        return genSpaces_->GetCurrentYoungSize();
    }

    size_t GetCurrentTenuredMaxSize() const
    {
        return genSpaces_->GetCurrentSize();
    }

    struct HeapSpaceHolder {
        NO_COPY_SEMANTIC(HeapSpaceHolder);
        NO_MOVE_SEMANTIC(HeapSpaceHolder);

        explicit HeapSpaceHolder(size_t initialHeapSize = DEFAULT_TEST_HEAP_SIZE,
                                 size_t maxHeapSize = DEFAULT_TEST_HEAP_SIZE,
                                 uint32_t minPercentage = DEFAULT_MIN_PERCENTAGE,
                                 uint32_t maxPercentage = DEFAULT_MAX_PERCENTAGE)
        {
            MemConfig::Initialize(maxHeapSize, 0_MB, 0_MB, 0_MB, 0_MB, 0_MB, initialHeapSize);
            PoolManager::Initialize();
            HeapSpaceTest::heapSpace_ = new HeapSpace();
            HeapSpaceTest::heapSpace_->Initialize(MemConfig::GetInitialHeapSizeLimit(), MemConfig::GetHeapSizeLimit(),
                                                  minPercentage, maxPercentage);
        }

        ~HeapSpaceHolder() noexcept
        {
            delete HeapSpaceTest::heapSpace_;
            HeapSpaceTest::heapSpace_ = nullptr;
            PoolManager::Finalize();
            MemConfig::Finalize();
        }
    };

    struct GenerationalSpacesHolder {
        NO_COPY_SEMANTIC(GenerationalSpacesHolder);
        NO_MOVE_SEMANTIC(GenerationalSpacesHolder);

        // CC-OFFNXT(G.FUN.01) solid logic
        explicit GenerationalSpacesHolder(size_t initYoungSize = DEFAULT_TEST_YOUNG_SIZE,
                                          size_t youngSize = DEFAULT_TEST_YOUNG_SIZE,
                                          size_t initialHeapSize = DEFAULT_TEST_HEAP_SIZE,
                                          size_t maxHeapSize = DEFAULT_TEST_HEAP_SIZE, uint32_t minPercentage = 0U,
                                          uint32_t maxPercentage = PERCENT_100_U32)
        {
            MemConfig::Initialize(maxHeapSize, 0_MB, 0_MB, 0_MB, 0_MB, 0_MB, initialHeapSize);
            PoolManager::Initialize();
            HeapSpaceTest::genSpaces_ = new GenerationalSpaces();
            HeapSpaceTest::genSpaces_->Initialize(initYoungSize, true, youngSize, true,
                                                  MemConfig::GetInitialHeapSizeLimit(), MemConfig::GetHeapSizeLimit(),
                                                  minPercentage, maxPercentage);
        }

        ~GenerationalSpacesHolder() noexcept
        {
            delete HeapSpaceTest::genSpaces_;
            HeapSpaceTest::genSpaces_ = nullptr;
            PoolManager::Finalize();
            MemConfig::Finalize();
        }
    };

    static HeapSpace *heapSpace_;
    static GenerationalSpaces *genSpaces_;
};

HeapSpace *HeapSpaceTest::heapSpace_ = nullptr;
GenerationalSpaces *HeapSpaceTest::genSpaces_ = nullptr;

TEST_F(HeapSpaceTest, AllocFreeAndCheckSizesTest)
{
    HeapSpaceHolder hsh;

    auto pool1 =
        heapSpace_->TryAllocPool(4_MB, SpaceType::SPACE_TYPE_OBJECT, AllocatorType::FREELIST_ALLOCATOR, nullptr);
    ASSERT_NE(pool1, NULLPOOL);
    auto pool2 =
        heapSpace_->TryAllocPool(6_MB, SpaceType::SPACE_TYPE_OBJECT, AllocatorType::FREELIST_ALLOCATOR, nullptr);
    ASSERT_NE(pool2, NULLPOOL);

    ASSERT_EQ(heapSpace_->GetHeapSize(), 10_MB);

    heapSpace_->FreePool(pool1.GetMem(), pool1.GetSize());
    ASSERT_EQ(heapSpace_->GetHeapSize(), 6_MB);
    auto *arena1 =
        heapSpace_->TryAllocArena(6_MB, SpaceType::SPACE_TYPE_OBJECT, AllocatorType::FREELIST_ALLOCATOR, nullptr);
    ASSERT_NE(arena1, nullptr);
    ASSERT_EQ(heapSpace_->GetHeapSize(), 12_MB);
    heapSpace_->FreePool(pool2.GetMem(), pool2.GetSize());
    ASSERT_EQ(heapSpace_->GetHeapSize(), 6_MB);
    heapSpace_->FreeArena(arena1);
    ASSERT_EQ(heapSpace_->GetHeapSize(), 0_MB);
}

TEST_F(HeapSpaceTest, EmulateAllocBeforeAndAfterSTWGCTest)
{
    static constexpr size_t INIT_HEAP_SIZE = 4_MB;
    static constexpr size_t FIRST_POOL_SIZE = INIT_HEAP_SIZE / 2U;
    static constexpr size_t SECOND_POOL_SIZE = INIT_HEAP_SIZE * 3U / 4U;
    HeapSpaceHolder hsh(INIT_HEAP_SIZE, 2U * INIT_HEAP_SIZE, 0U, PERCENT_100_U32);

    ASSERT_EQ(GetCurrentMaxSize(), INIT_HEAP_SIZE) << "Current heap limit must be equal initial heap size";
    auto pool1 = heapSpace_->TryAllocPool(FIRST_POOL_SIZE, SpaceType::SPACE_TYPE_OBJECT,
                                          AllocatorType::FREELIST_ALLOCATOR, nullptr);
    ASSERT_NE(pool1, NULLPOOL);
    // -- Emulate simple GC
    heapSpace_->ComputeNewSize();
    ASSERT_EQ(GetCurrentMaxSize(), INIT_HEAP_SIZE);
    // --
    auto pool2 = heapSpace_->TryAllocPool(SECOND_POOL_SIZE, SpaceType::SPACE_TYPE_OBJECT,
                                          AllocatorType::FREELIST_ALLOCATOR, nullptr);
    ASSERT_EQ(pool2, NULLPOOL) << "We now can't allocate pool";
    // -- Emulate simple GC
    heapSpace_->ComputeNewSize();
    ASSERT_EQ(GetCurrentMaxSize(), FIRST_POOL_SIZE + SECOND_POOL_SIZE);
    // --
    pool2 = heapSpace_->TryAllocPool(SECOND_POOL_SIZE, SpaceType::SPACE_TYPE_OBJECT, AllocatorType::FREELIST_ALLOCATOR,
                                     nullptr);
    ASSERT_NE(pool2, NULLPOOL);
    heapSpace_->FreePool(pool1.GetMem(), pool1.GetSize());
    // -- Emulate simple GC
    heapSpace_->ComputeNewSize();
    ASSERT_EQ(GetCurrentMaxSize(), FIRST_POOL_SIZE + SECOND_POOL_SIZE);
    // --
    heapSpace_->FreePool(pool2.GetMem(), pool2.GetSize());
}

TEST_F(HeapSpaceTest, EmulateAllocBeforeAndDuringGenGCTest)
{
    static constexpr size_t INIT_HEAP_SIZE = 16_MB;
    static constexpr size_t FIRST_POOL_SIZE = 4_MB;
    static constexpr size_t SECOND_POOL_SIZE = 10_MB;
    GenerationalSpacesHolder gsh(DEFAULT_TEST_YOUNG_SIZE, DEFAULT_TEST_YOUNG_SIZE, INIT_HEAP_SIZE, INIT_HEAP_SIZE * 2U);
    auto youngPool = genSpaces_->AllocAlonePoolForYoung(SpaceType::SPACE_TYPE_OBJECT,
                                                        AllocatorType::BUMP_ALLOCATOR_WITH_TLABS, nullptr);
    // Check young pool allocation
    ASSERT_NE(youngPool, NULLPOOL);
    ASSERT_EQ(youngPool.GetSize(), DEFAULT_TEST_YOUNG_SIZE);
    // Check current heap space sizes before "runtime work"
    ASSERT_EQ(GetCurrentYoungMaxSize(), DEFAULT_TEST_YOUNG_SIZE);
    ASSERT_EQ(GetCurrentTenuredMaxSize(), INIT_HEAP_SIZE - DEFAULT_TEST_YOUNG_SIZE);

    auto pool1 = genSpaces_->TryAllocPoolForYoung(FIRST_POOL_SIZE, SpaceType::SPACE_TYPE_OBJECT,
                                                  AllocatorType::FREELIST_ALLOCATOR, nullptr);
    ASSERT_EQ(pool1, NULLPOOL);
    // -- Emulate simple GC
    genSpaces_->SetIsWorkGC(true);
    pool1 = genSpaces_->TryAllocPoolForTenured(FIRST_POOL_SIZE, SpaceType::SPACE_TYPE_OBJECT,
                                               AllocatorType::RUNSLOTS_ALLOCATOR, nullptr);
    ASSERT_EQ(pool1.GetSize(), FIRST_POOL_SIZE);
    genSpaces_->ComputeNewSize();
    ASSERT_EQ(GetCurrentYoungMaxSize(), DEFAULT_TEST_YOUNG_SIZE);
    ASSERT_EQ(GetCurrentTenuredMaxSize(), INIT_HEAP_SIZE - DEFAULT_TEST_YOUNG_SIZE);
    // --
    // Try too big pool, now no space for such pool
    auto pool2 = genSpaces_->TryAllocPoolForTenured(SECOND_POOL_SIZE, SpaceType::SPACE_TYPE_OBJECT,
                                                    AllocatorType::FREELIST_ALLOCATOR, nullptr);
    ASSERT_EQ(pool2, NULLPOOL) << "Now no space for such pool, so we must have NULLPOOL";
    // -- Emulate simple GC
    genSpaces_->SetIsWorkGC(true);
    ASSERT_TRUE(genSpaces_->CanAllocInSpace(false, SECOND_POOL_SIZE)) << "We can allocate pool during GC";
    pool2 = genSpaces_->TryAllocPoolForTenured(SECOND_POOL_SIZE, SpaceType::SPACE_TYPE_OBJECT,
                                               AllocatorType::FREELIST_ALLOCATOR, nullptr);
    ASSERT_EQ(pool2.GetSize(), SECOND_POOL_SIZE) << "We can allocate pool during GC";
    ASSERT_EQ(GetCurrentTenuredMaxSize(), FIRST_POOL_SIZE + SECOND_POOL_SIZE);
    genSpaces_->ComputeNewSize();
    ASSERT_EQ(GetCurrentTenuredMaxSize(), FIRST_POOL_SIZE + 2U * SECOND_POOL_SIZE);
    // --
    genSpaces_->FreePool(pool2.GetMem(), pool2.GetSize());
    genSpaces_->FreePool(pool1.GetMem(), pool1.GetSize());
    genSpaces_->FreeYoungPool(youngPool.GetMem(), youngPool.GetSize());
}

TEST_F(HeapSpaceTest, SharedPoolTest)
{
    static constexpr size_t INIT_HEAP_SIZE = 32_MB;
    static constexpr size_t INIT_YOUNG_SIZE = 2_MB;
    static constexpr size_t MAX_YOUNG_SIZE = 8_MB;
    static constexpr size_t SHARED_POOL_SIZE = 8_MB;
    static constexpr size_t REGION_SIZE = 1_MB;
    static constexpr bool IS_YOUNG = true;
    GenerationalSpacesHolder gsh(INIT_YOUNG_SIZE, MAX_YOUNG_SIZE, INIT_HEAP_SIZE);
    auto sharedPool = genSpaces_->AllocSharedPool(SHARED_POOL_SIZE, SpaceType::SPACE_TYPE_OBJECT,
                                                  AllocatorType::REGION_ALLOCATOR, nullptr);
    ASSERT_EQ(sharedPool.GetSize(), SHARED_POOL_SIZE);
    ASSERT_EQ(GetCurrentYoungMaxSize(), INIT_YOUNG_SIZE);
    ASSERT_EQ(GetCurrentTenuredMaxSize(), INIT_HEAP_SIZE - INIT_YOUNG_SIZE);
    ASSERT_TRUE(genSpaces_->CanAllocInSpace(IS_YOUNG, INIT_YOUNG_SIZE));
    ASSERT_TRUE(genSpaces_->CanAllocInSpace(!IS_YOUNG, INIT_YOUNG_SIZE));
    genSpaces_->IncreaseYoungOccupiedInSharedPool(REGION_SIZE);
    genSpaces_->IncreaseTenuredOccupiedInSharedPool(REGION_SIZE);
    ASSERT_FALSE(genSpaces_->CanAllocInSpace(IS_YOUNG, INIT_YOUNG_SIZE));
    ASSERT_TRUE(genSpaces_->CanAllocInSpace(!IS_YOUNG, INIT_YOUNG_SIZE));
    auto pool1 = genSpaces_->TryAllocPoolForTenured(REGION_SIZE, SpaceType::SPACE_TYPE_OBJECT,
                                                    AllocatorType::REGION_ALLOCATOR, nullptr);
    ASSERT_EQ(pool1.GetSize(), REGION_SIZE);
    genSpaces_->ReduceYoungOccupiedInSharedPool(REGION_SIZE);
    ASSERT_TRUE(genSpaces_->CanAllocInSpace(IS_YOUNG, INIT_YOUNG_SIZE));
    genSpaces_->ReduceTenuredOccupiedInSharedPool(REGION_SIZE);
    ASSERT_TRUE(genSpaces_->CanAllocInSpace(IS_YOUNG, INIT_YOUNG_SIZE));
    ASSERT_TRUE(genSpaces_->CanAllocInSpace(!IS_YOUNG, INIT_YOUNG_SIZE));
    genSpaces_->FreeTenuredPool(pool1.GetMem(), pool1.GetSize());
    genSpaces_->FreeSharedPool(sharedPool.GetMem(), sharedPool.GetSize());
}

TEST_F(HeapSpaceTest, ClampNewMaxSizeTest)
{
    // NOLINTNEXTLINE(readability-magic-numbers)
    HeapSpaceHolder hsh(10_MB, DEFAULT_TEST_HEAP_SIZE, 0U, PERCENT_100_U32);

    auto pool1 =
        heapSpace_->TryAllocPool(4_MB, SpaceType::SPACE_TYPE_OBJECT, AllocatorType::FREELIST_ALLOCATOR, nullptr);
    ASSERT_NE(pool1, NULLPOOL);
    auto pool2 =
        heapSpace_->TryAllocPool(6_MB, SpaceType::SPACE_TYPE_OBJECT, AllocatorType::FREELIST_ALLOCATOR, nullptr);
    ASSERT_NE(pool2, NULLPOOL);

    ASSERT_EQ(heapSpace_->GetHeapSize(), 10_MB);

    heapSpace_->ClampCurrentMaxHeapSize();
    heapSpace_->SetIsWorkGC(true);

    auto pool3 =
        heapSpace_->TryAllocPool(6_MB, SpaceType::SPACE_TYPE_OBJECT, AllocatorType::FREELIST_ALLOCATOR, nullptr);
    ASSERT_EQ(pool3, NULLPOOL);

    pool3 = heapSpace_->TryAllocPool(PANDA_DEFAULT_POOL_SIZE, SpaceType::SPACE_TYPE_OBJECT,
                                     AllocatorType::FREELIST_ALLOCATOR, nullptr);
    ASSERT_NE(pool3, NULLPOOL);

    heapSpace_->FreePool(pool1.GetMem(), pool1.GetSize());
    heapSpace_->FreePool(pool2.GetMem(), pool2.GetSize());
    heapSpace_->FreePool(pool3.GetMem(), pool3.GetSize());
}

}  // namespace ark::mem::test
