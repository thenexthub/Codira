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
#include "mem/mem.h"
#include "mem/mmap_mem_pool-inl.h"

#include "gtest/gtest.h"

namespace panda {

class MMapMemPoolTest : public testing::Test {
public:
    MMapMemPoolTest()
    {
        instance_ = nullptr;
    }

    ~MMapMemPoolTest()
    {
        if (instance_ != nullptr) {
            delete instance_;
        }
        FinalizeMemConfig();
    }

protected:
    MmapMemPool *CreateMMapMemPool(size_t object_pool_size = 0, size_t internal_size = 0, size_t compiler_size = 0,
                                   size_t code_size = 0)
    {
        ASSERT(instance_ == nullptr);
        SetupMemConfig(object_pool_size, internal_size, compiler_size, code_size);
        instance_ = new MmapMemPool();
        return instance_;
    }

private:
    void SetupMemConfig(size_t object_pool_size, size_t internal_size, size_t compiler_size, size_t code_size)
    {
        mem::MemConfig::Initialize(object_pool_size, internal_size, compiler_size, code_size);
    }

    void FinalizeMemConfig()
    {
        mem::MemConfig::Finalize();
    }

    MmapMemPool *instance_;
};

HWTEST_F(MMapMemPoolTest, HeapOOMTest, testing::ext::TestSize.Level0)
{
    MmapMemPool *memPool = CreateMMapMemPool(4_MB);

    auto pool1 = memPool->AllocPool(4_MB, SpaceType::SPACE_TYPE_HUMONGOUS_OBJECT, AllocatorType::HUMONGOUS_ALLOCATOR);
    ASSERT_TRUE(pool1.GetMem() != nullptr);
    auto pool2 = memPool->AllocPool(4_MB, SpaceType::SPACE_TYPE_HUMONGOUS_OBJECT, AllocatorType::HUMONGOUS_ALLOCATOR);
    ASSERT_TRUE(pool2.GetMem() == nullptr);
    auto pool3 = memPool->AllocPool(4_MB, SpaceType::SPACE_TYPE_NON_MOVABLE_OBJECT, AllocatorType::HUMONGOUS_ALLOCATOR);
    ASSERT_TRUE(pool3.GetMem() == nullptr);
    auto pool4 = memPool->AllocPool(4_MB, SpaceType::SPACE_TYPE_OBJECT, AllocatorType::HUMONGOUS_ALLOCATOR);
    ASSERT_TRUE(pool4.GetMem() == nullptr);

    memPool->FreePool(pool1.GetMem(), pool1.GetSize());
}

HWTEST_F(MMapMemPoolTest, HeapOOMAndAllocInOtherSpacesTest, testing::ext::TestSize.Level0)
{
    MmapMemPool *memPool = CreateMMapMemPool(4_MB, 1_MB, 1_MB, 1_MB);

    auto pool1 = memPool->AllocPool(4_MB, SpaceType::SPACE_TYPE_OBJECT, AllocatorType::BUMP_ALLOCATOR);
    ASSERT_TRUE(pool1.GetMem() != nullptr);
    auto pool2 = memPool->AllocPool(4_MB, SpaceType::SPACE_TYPE_OBJECT, AllocatorType::BUMP_ALLOCATOR);
    ASSERT_TRUE(pool2.GetMem() == nullptr);
    auto pool3 = memPool->AllocPool(1_MB, SpaceType::SPACE_TYPE_COMPILER, AllocatorType::BUMP_ALLOCATOR);
    ASSERT_TRUE(pool3.GetMem() != nullptr);
    auto pool4 = memPool->AllocPool(1_MB, SpaceType::SPACE_TYPE_CODE, AllocatorType::BUMP_ALLOCATOR);
    ASSERT_TRUE(pool4.GetMem() != nullptr);
    auto pool5 = memPool->AllocPool(1_MB, SpaceType::SPACE_TYPE_INTERNAL, AllocatorType::BUMP_ALLOCATOR);
    ASSERT_TRUE(pool5.GetMem() != nullptr);

    // cleaning
    memPool->FreePool(pool1.GetMem(), pool1.GetSize());
    memPool->FreePool(pool3.GetMem(), pool3.GetSize());
    memPool->FreePool(pool4.GetMem(), pool4.GetSize());
    memPool->FreePool(pool5.GetMem(), pool5.GetSize());
}

HWTEST_F(MMapMemPoolTest, GetAllocatorInfoTest, testing::ext::TestSize.Level0)
{
    static constexpr AllocatorType ALLOC_TYPE = AllocatorType::BUMP_ALLOCATOR;
    static constexpr size_t POOL_SIZE = 4_MB;
    static constexpr size_t POINTER_POOL_OFFSET = 1_MB;
    ASSERT_TRUE(POINTER_POOL_OFFSET < POOL_SIZE);
    MmapMemPool *memPool = CreateMMapMemPool(POOL_SIZE * 2, 0, 0, 0);
    int *allocator_addr = new int();
    Pool pool_with_alloc_addr = memPool->AllocPool(POOL_SIZE, SpaceType::SPACE_TYPE_OBJECT, ALLOC_TYPE, allocator_addr);
    Pool pool_without_alloc_addr = memPool->AllocPool(POOL_SIZE, SpaceType::SPACE_TYPE_OBJECT, ALLOC_TYPE);
    ASSERT_TRUE(pool_with_alloc_addr.GetMem() != nullptr);
    ASSERT_TRUE(pool_without_alloc_addr.GetMem() != nullptr);

    void *first_pool_pointer = ToVoidPtr(ToUintPtr(pool_with_alloc_addr.GetMem()) + POINTER_POOL_OFFSET);
    ASSERT_TRUE(ToUintPtr(memPool->GetAllocatorInfoForAddr(first_pool_pointer).GetAllocatorHeaderAddr()) ==
                ToUintPtr(allocator_addr));
    ASSERT_TRUE(memPool->GetAllocatorInfoForAddr(first_pool_pointer).GetType() == ALLOC_TYPE);
    ASSERT_TRUE(ToUintPtr(memPool->GetStartAddrPoolForAddr(first_pool_pointer)) ==
                ToUintPtr(pool_with_alloc_addr.GetMem()));

    void *second_pool_pointer = ToVoidPtr(ToUintPtr(pool_without_alloc_addr.GetMem()) + POINTER_POOL_OFFSET);
    ASSERT_TRUE(ToUintPtr(memPool->GetAllocatorInfoForAddr(second_pool_pointer).GetAllocatorHeaderAddr()) ==
                ToUintPtr(pool_without_alloc_addr.GetMem()));
    ASSERT_TRUE(memPool->GetAllocatorInfoForAddr(second_pool_pointer).GetType() == ALLOC_TYPE);
    ASSERT_TRUE(ToUintPtr(memPool->GetStartAddrPoolForAddr(second_pool_pointer)) ==
                ToUintPtr(pool_without_alloc_addr.GetMem()));

    // cleaning
    memPool->FreePool(pool_with_alloc_addr.GetMem(), pool_with_alloc_addr.GetSize());
    memPool->FreePool(pool_without_alloc_addr.GetMem(), pool_without_alloc_addr.GetSize());

    delete allocator_addr;
}

HWTEST_F(MMapMemPoolTest, CheckLimitsForInternalSpacesTest, testing::ext::TestSize.Level0)
{
#ifndef PANDA_TARGET_32
    MmapMemPool *mem_pool = CreateMMapMemPool(1_MB, 2_MB, 2_MB, 2_MB);
    Pool object_pool = mem_pool->AllocPool(1_MB, SpaceType::SPACE_TYPE_OBJECT, AllocatorType::BUMP_ALLOCATOR);
    Pool internal_pool = mem_pool->AllocPool(2_MB, SpaceType::SPACE_TYPE_COMPILER, AllocatorType::BUMP_ALLOCATOR);
    Pool code_pool = mem_pool->AllocPool(2_MB, SpaceType::SPACE_TYPE_CODE, AllocatorType::BUMP_ALLOCATOR);
    Pool compiler_pool = mem_pool->AllocPool(2_MB, SpaceType::SPACE_TYPE_INTERNAL, AllocatorType::BUMP_ALLOCATOR);
    // Check that these pools has been created successfully
    ASSERT_TRUE(object_pool.GetMem() != nullptr);
    ASSERT_TRUE(internal_pool.GetMem() != nullptr);
    ASSERT_TRUE(code_pool.GetMem() != nullptr);
    ASSERT_TRUE(compiler_pool.GetMem() != nullptr);

    // cleaning
    mem_pool->FreePool(object_pool.GetMem(), object_pool.GetSize());
    mem_pool->FreePool(internal_pool.GetMem(), internal_pool.GetSize());
    mem_pool->FreePool(code_pool.GetMem(), code_pool.GetSize());
    mem_pool->FreePool(compiler_pool.GetMem(), compiler_pool.GetSize());
#endif
}

HWTEST_F(MMapMemPoolTest, PoolReturnTest, testing::ext::TestSize.Level0)
{
    MmapMemPool *memPool = CreateMMapMemPool(8_MB);

    auto pool1 = memPool->AllocPool(4_MB, SpaceType::SPACE_TYPE_OBJECT, AllocatorType::HUMONGOUS_ALLOCATOR);
    ASSERT_TRUE(pool1.GetMem() != nullptr);
    auto pool2 = memPool->AllocPool(4_MB, SpaceType::SPACE_TYPE_OBJECT, AllocatorType::HUMONGOUS_ALLOCATOR);
    ASSERT_TRUE(pool2.GetMem() != nullptr);
    auto pool3 = memPool->AllocPool(4_MB, SpaceType::SPACE_TYPE_OBJECT, AllocatorType::HUMONGOUS_ALLOCATOR);
    ASSERT_TRUE(pool3.GetMem() == nullptr);
    memPool->FreePool(pool1.GetMem(), pool1.GetSize());
    memPool->FreePool(pool2.GetMem(), pool2.GetSize());

    auto pool4 = memPool->AllocPool(6_MB, SpaceType::SPACE_TYPE_OBJECT, AllocatorType::HUMONGOUS_ALLOCATOR);
    ASSERT_TRUE(pool4.GetMem() != nullptr);
    auto pool5 = memPool->AllocPool(1_MB, SpaceType::SPACE_TYPE_OBJECT, AllocatorType::HUMONGOUS_ALLOCATOR);
    ASSERT_TRUE(pool5.GetMem() != nullptr);
    auto pool6 = memPool->AllocPool(1_MB, SpaceType::SPACE_TYPE_OBJECT, AllocatorType::HUMONGOUS_ALLOCATOR);
    ASSERT_TRUE(pool6.GetMem() != nullptr);
    memPool->FreePool(pool6.GetMem(), pool6.GetSize());
    memPool->FreePool(pool4.GetMem(), pool4.GetSize());
    memPool->FreePool(pool5.GetMem(), pool5.GetSize());

    auto pool7 = memPool->AllocPool(8_MB, SpaceType::SPACE_TYPE_OBJECT, AllocatorType::HUMONGOUS_ALLOCATOR);
    ASSERT_TRUE(pool7.GetMem() != nullptr);
    memPool->FreePool(pool7.GetMem(), pool7.GetSize());
}

HWTEST_F(MMapMemPoolTest, CheckEnoughPoolsTest, testing::ext::TestSize.Level0)
{
    static constexpr size_t POOL_SIZE = 4_MB;
    MmapMemPool *memPool = CreateMMapMemPool(10_MB);

    ASSERT_TRUE(memPool->HaveEnoughPoolsInObjectSpace(2, POOL_SIZE));
    ASSERT_FALSE(memPool->HaveEnoughPoolsInObjectSpace(3, POOL_SIZE));
    auto pool1 = memPool->AllocPool(3_MB, SpaceType::SPACE_TYPE_OBJECT, AllocatorType::HUMONGOUS_ALLOCATOR);
    ASSERT_TRUE(memPool->HaveEnoughPoolsInObjectSpace(1, POOL_SIZE));
    ASSERT_FALSE(memPool->HaveEnoughPoolsInObjectSpace(2, POOL_SIZE));
    auto pool2 = memPool->AllocPool(1_MB, SpaceType::SPACE_TYPE_OBJECT, AllocatorType::HUMONGOUS_ALLOCATOR);
    memPool->FreePool(pool1.GetMem(), pool1.GetSize());
    ASSERT_TRUE(memPool->HaveEnoughPoolsInObjectSpace(1, POOL_SIZE));
    ASSERT_FALSE(memPool->HaveEnoughPoolsInObjectSpace(2, POOL_SIZE));
    memPool->FreePool(pool2.GetMem(), pool2.GetSize());
    ASSERT_TRUE(memPool->HaveEnoughPoolsInObjectSpace(2, POOL_SIZE));
    ASSERT_FALSE(memPool->HaveEnoughPoolsInObjectSpace(3, POOL_SIZE));

    auto pool3 = memPool->AllocPool(4_MB, SpaceType::SPACE_TYPE_OBJECT, AllocatorType::HUMONGOUS_ALLOCATOR);
    auto pool4 = memPool->AllocPool(1_MB, SpaceType::SPACE_TYPE_OBJECT, AllocatorType::HUMONGOUS_ALLOCATOR);
    ASSERT_TRUE(memPool->HaveEnoughPoolsInObjectSpace(1, POOL_SIZE));
    ASSERT_FALSE(memPool->HaveEnoughPoolsInObjectSpace(2, POOL_SIZE));
    memPool->FreePool(pool3.GetMem(), pool3.GetSize());
    ASSERT_TRUE(memPool->HaveEnoughPoolsInObjectSpace(2, POOL_SIZE));
    ASSERT_FALSE(memPool->HaveEnoughPoolsInObjectSpace(3, POOL_SIZE));
    auto pool5 = memPool->AllocPool(5_MB, SpaceType::SPACE_TYPE_OBJECT, AllocatorType::HUMONGOUS_ALLOCATOR);
    ASSERT_TRUE(memPool->HaveEnoughPoolsInObjectSpace(1, POOL_SIZE));
    ASSERT_FALSE(memPool->HaveEnoughPoolsInObjectSpace(2, POOL_SIZE));
    memPool->FreePool(pool5.GetMem(), pool5.GetSize());
    ASSERT_TRUE(memPool->HaveEnoughPoolsInObjectSpace(2, POOL_SIZE));
    ASSERT_FALSE(memPool->HaveEnoughPoolsInObjectSpace(3, POOL_SIZE));
    memPool->FreePool(pool4.GetMem(), pool4.GetSize());
    ASSERT_TRUE(memPool->HaveEnoughPoolsInObjectSpace(2, POOL_SIZE));
    ASSERT_FALSE(memPool->HaveEnoughPoolsInObjectSpace(3, POOL_SIZE));
}

/**
 * @tc.name: UpdateCompilerMemoryLimitTest
 * @tc.desc: check UpdateCompilerMemoryLimit function
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(MMapMemPoolTest, UpdateCompilerMemoryLimitTest, testing::ext::TestSize.Level0) {
    MmapMemPool *memPool = CreateMMapMemPool(0, 0, 1_MB, 0);
    auto arena1 = memPool->AllocArena(4_MB, SpaceType::SPACE_TYPE_COMPILER, AllocatorType::HUMONGOUS_ALLOCATOR);
    ASSERT_NE(arena1, nullptr);
    memPool->SetUseCompilerSpaceSizeLimit(true);
    auto arena2 = memPool->AllocArena(4_MB, SpaceType::SPACE_TYPE_COMPILER, AllocatorType::HUMONGOUS_ALLOCATOR);
    ASSERT_EQ(arena2, nullptr);
    memPool->SetUseCompilerSpaceSizeLimit(false);
    auto arena3 = memPool->AllocArena(4_MB, SpaceType::SPACE_TYPE_COMPILER, AllocatorType::HUMONGOUS_ALLOCATOR);
    ASSERT_NE(arena3, nullptr);
}

}  // namespace panda
