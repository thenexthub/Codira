/*
 * Copyright (c) 2021-2025 Huawei Device Co., Ltd.
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 * http://www.apache.org/licenses/LICENSE-2.0
 *
 *nless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */
#include "libarkbase/mem/mem.h"
#include "libarkbase/mem/mem_pool.h"
#include "libarkbase/mem/mmap_mem_pool-inl.h"

#include "gtest/gtest.h"

namespace ark {

class MMapMemPoolTest : public testing::Test {
public:
    MMapMemPoolTest()
    {
        instance_ = nullptr;
    }

    ~MMapMemPoolTest() override
    {
        delete instance_;
        FinalizeMemConfig();
    }

    NO_MOVE_SEMANTIC(MMapMemPoolTest);
    NO_COPY_SEMANTIC(MMapMemPoolTest);

protected:
    // CC-OFFNXT(G.FUN.01-CPP) solid logic
    MmapMemPool *CreateMMapMemPool(size_t objectPoolSize = 0, size_t internalSize = 0, size_t compilerSize = 0,
                                   size_t codeSize = 0U, size_t framesSize = 0U, size_t stacksSize = 0U)
    {
        ASSERT(instance_ == nullptr);
        SetupMemConfig(objectPoolSize, internalSize, compilerSize, codeSize, framesSize, stacksSize);
        instance_ = new MmapMemPool();
        return instance_;
    }

    void FreePoolWithPolicy(MmapMemPool *memPool, OSPagesPolicy policy, Pool &pool)
    {
        if (policy == OSPagesPolicy::NO_RETURN) {
            memPool->template FreePool<OSPagesPolicy::NO_RETURN>(pool.GetMem(), pool.GetSize());
        } else {
            memPool->template FreePool<OSPagesPolicy::IMMEDIATE_RETURN>(pool.GetMem(), pool.GetSize());
        }
    }

    void ReturnedToOsTest(OSPagesPolicy firstPoolPolicy, OSPagesPolicy secondPoolPolicy, OSPagesPolicy thirdPoolPolicy,
                          bool needFourthPool, bool bigPoolRealloc)
    {
        static constexpr size_t POOL_COUNT = 3U;
        static constexpr size_t MMAP_MEMORY_SIZE = 16_MB;
        static constexpr size_t BIG_POOL_ALLOC_SIZE = 12_MB;
        MmapMemPool *memPool = CreateMMapMemPool(MMAP_MEMORY_SIZE);
        std::array<Pool, POOL_COUNT> pools {{NULLPOOL, NULLPOOL, NULLPOOL}};
        Pool fourthPool = NULLPOOL;

        for (size_t i = 0; i < POOL_COUNT; i++) {
            pools[i] = memPool->AllocPool(4_MB, SpaceType::SPACE_TYPE_OBJECT, AllocatorType::HUMONGOUS_ALLOCATOR);
            ASSERT_TRUE(pools[i].GetMem() != nullptr);
            FillMemory(pools[i].GetMem(), pools[i].GetSize());
        }
        if (needFourthPool) {
            fourthPool = memPool->AllocPool(4_MB, SpaceType::SPACE_TYPE_OBJECT, AllocatorType::HUMONGOUS_ALLOCATOR);
            ASSERT_TRUE(fourthPool.GetMem() != nullptr);
            FillMemory(fourthPool.GetMem(), fourthPool.GetSize());
        }

        FreePoolWithPolicy(memPool, firstPoolPolicy, pools[0U]);
        FreePoolWithPolicy(memPool, thirdPoolPolicy, pools[2U]);
        FreePoolWithPolicy(memPool, secondPoolPolicy, pools[1U]);

        if (bigPoolRealloc) {
            auto finalPool = memPool->template AllocPool<OSPagesAllocPolicy::ZEROED_MEMORY>(
                BIG_POOL_ALLOC_SIZE, SpaceType::SPACE_TYPE_OBJECT, AllocatorType::HUMONGOUS_ALLOCATOR);
            ASSERT_TRUE(finalPool.GetMem() != nullptr);
            IsZeroMemory(finalPool.GetMem(), finalPool.GetSize());
            memPool->FreePool(finalPool.GetMem(), finalPool.GetSize());
        } else {
            // reallocate again
            for (size_t i = 0; i < POOL_COUNT; i++) {
                pools[i] = memPool->template AllocPool<OSPagesAllocPolicy::ZEROED_MEMORY>(
                    4_MB, SpaceType::SPACE_TYPE_OBJECT, AllocatorType::HUMONGOUS_ALLOCATOR);
                ASSERT_TRUE(pools[i].GetMem() != nullptr);
                IsZeroMemory(pools[i].GetMem(), pools[i].GetSize());
            }
            for (size_t i = 0; i < POOL_COUNT; i++) {
                memPool->FreePool(pools[i].GetMem(), pools[i].GetSize());
            }
        }
        if (needFourthPool) {
            memPool->FreePool(fourthPool.GetMem(), fourthPool.GetSize());
        }
        DeleteMMapMemPool(memPool);
    }

private:
    // CC-OFFNXT(G.FUN.01-CPP) solid logic
    void SetupMemConfig(size_t objectPoolSize, size_t internalSize, size_t compilerSize, size_t codeSize,
                        size_t framesSize, size_t stacksSize)
    {
        mem::MemConfig::Initialize(objectPoolSize, internalSize, compilerSize, codeSize, framesSize, stacksSize);
    }

    void FinalizeMemConfig()
    {
        mem::MemConfig::Finalize();
    }

    void DeleteMMapMemPool(MmapMemPool *memPool)
    {
        ASSERT(instance_ == memPool);
        delete memPool;
        FinalizeMemConfig();
        instance_ = nullptr;
    }

    void FillMemory(void *start, size_t size)
    {
        size_t itEnd = size / sizeof(uint64_t);
        auto *pointer = static_cast<uint64_t *>(start);
        for (size_t i = 0; i < itEnd; i++) {
            // NOLINTNEXTLINE(cppcoreguidelines-pro-bounds-pointer-arithmetic)
            pointer[i] = MAGIC_VALUE;
        }
    }

    bool IsZeroMemory(void *start, size_t size)
    {
        size_t itEnd = size / sizeof(uint64_t);
        auto *pointer = static_cast<uint64_t *>(start);
        for (size_t i = 0; i < itEnd; i++) {
            // NOLINTNEXTLINE(cppcoreguidelines-pro-bounds-pointer-arithmetic)
            if (pointer[i] != 0U) {
                return false;
            }
        }
        return true;
    }

    static constexpr uint64_t MAGIC_VALUE = 0xDEADBEEF;

    MmapMemPool *instance_;
};

TEST_F(MMapMemPoolTest, HeapOOMTest)
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

TEST_F(MMapMemPoolTest, HeapOOMAndAllocInOtherSpacesTest)
{
    MmapMemPool *memPool = CreateMMapMemPool(4_MB, 1_MB, 1_MB, 1_MB, 1_MB, 1_MB);

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
    auto pool6 = memPool->AllocPool(1_MB, SpaceType::SPACE_TYPE_FRAMES, AllocatorType::BUMP_ALLOCATOR);
    ASSERT_TRUE(pool6.GetMem() != nullptr);
    auto pool7 = memPool->AllocPool(1_MB, SpaceType::SPACE_TYPE_NATIVE_STACKS, AllocatorType::BUMP_ALLOCATOR);
    ASSERT_TRUE(pool7.GetMem() != nullptr);

    // cleaning
    memPool->FreePool(pool1.GetMem(), pool1.GetSize());
    memPool->FreePool(pool3.GetMem(), pool3.GetSize());
    memPool->FreePool(pool4.GetMem(), pool4.GetSize());
    memPool->FreePool(pool5.GetMem(), pool5.GetSize());
    memPool->FreePool(pool6.GetMem(), pool6.GetSize());
    memPool->FreePool(pool7.GetMem(), pool7.GetSize());
}

TEST_F(MMapMemPoolTest, GetAllocatorInfoTest)
{
    static constexpr AllocatorType ALLOC_TYPE = AllocatorType::BUMP_ALLOCATOR;
    static constexpr size_t POOL_SIZE = 4_MB;
    static constexpr size_t POINTER_POOL_OFFSET = 1_MB;
    ASSERT_TRUE(POINTER_POOL_OFFSET < POOL_SIZE);
    MmapMemPool *memPool = CreateMMapMemPool(POOL_SIZE * 2U, 0U, 0U, 0U);
    int *allocatorAddr = new int();
    Pool poolWithAllocAddr = memPool->AllocPool(POOL_SIZE, SpaceType::SPACE_TYPE_OBJECT, ALLOC_TYPE, allocatorAddr);
    Pool poolWithoutAllocAddr = memPool->AllocPool(POOL_SIZE, SpaceType::SPACE_TYPE_OBJECT, ALLOC_TYPE);
    ASSERT_TRUE(poolWithAllocAddr.GetMem() != nullptr);
    ASSERT_TRUE(poolWithoutAllocAddr.GetMem() != nullptr);

    void *firstPoolPointer = ToVoidPtr(ToUintPtr(poolWithAllocAddr.GetMem()) + POINTER_POOL_OFFSET);
    ASSERT_TRUE(ToUintPtr(memPool->GetAllocatorInfoForAddr(firstPoolPointer).GetAllocatorHeaderAddr()) ==
                ToUintPtr(allocatorAddr));
    ASSERT_TRUE(memPool->GetAllocatorInfoForAddr(firstPoolPointer).GetType() == ALLOC_TYPE);
    ASSERT_TRUE(ToUintPtr(memPool->GetStartAddrPoolForAddr(firstPoolPointer)) == ToUintPtr(poolWithAllocAddr.GetMem()));

    void *secondPoolPointer = ToVoidPtr(ToUintPtr(poolWithoutAllocAddr.GetMem()) + POINTER_POOL_OFFSET);
    ASSERT_TRUE(ToUintPtr(memPool->GetAllocatorInfoForAddr(secondPoolPointer).GetAllocatorHeaderAddr()) ==
                ToUintPtr(poolWithoutAllocAddr.GetMem()));
    ASSERT_TRUE(memPool->GetAllocatorInfoForAddr(secondPoolPointer).GetType() == ALLOC_TYPE);
    ASSERT_TRUE(ToUintPtr(memPool->GetStartAddrPoolForAddr(secondPoolPointer)) ==
                ToUintPtr(poolWithoutAllocAddr.GetMem()));

    // cleaning
    memPool->FreePool(poolWithAllocAddr.GetMem(), poolWithAllocAddr.GetSize());
    memPool->FreePool(poolWithoutAllocAddr.GetMem(), poolWithoutAllocAddr.GetSize());

    delete allocatorAddr;
}

TEST_F(MMapMemPoolTest, CheckLimitsForInternalSpacesTest)
{
#ifndef PANDA_TARGET_32
    MmapMemPool *memPool = CreateMMapMemPool(1_GB, 1_GB, 1_GB, 1_GB, 1_GB, 1_GB);
    Pool objectPool = memPool->AllocPool(1_GB, SpaceType::SPACE_TYPE_OBJECT, AllocatorType::BUMP_ALLOCATOR);
    Pool internalPool = memPool->AllocPool(1_GB, SpaceType::SPACE_TYPE_COMPILER, AllocatorType::BUMP_ALLOCATOR);
    Pool codePool = memPool->AllocPool(1_GB, SpaceType::SPACE_TYPE_CODE, AllocatorType::BUMP_ALLOCATOR);
    Pool compilerPool = memPool->AllocPool(1_GB, SpaceType::SPACE_TYPE_INTERNAL, AllocatorType::BUMP_ALLOCATOR);
    Pool framesPool = memPool->AllocPool(1_GB, SpaceType::SPACE_TYPE_FRAMES, AllocatorType::BUMP_ALLOCATOR);
    Pool stacksPool = memPool->AllocPool(1_GB, SpaceType::SPACE_TYPE_NATIVE_STACKS, AllocatorType::BUMP_ALLOCATOR);
    // Check that these pools has been created successfully
    ASSERT_TRUE(objectPool.GetMem() != nullptr);
    ASSERT_TRUE(internalPool.GetMem() != nullptr);
    ASSERT_TRUE(codePool.GetMem() != nullptr);
    ASSERT_TRUE(compilerPool.GetMem() != nullptr);
    ASSERT_TRUE(framesPool.GetMem() != nullptr);
    ASSERT_TRUE(stacksPool.GetMem() != nullptr);
    // Check that part of internal pools located in 64 bits address space
    bool internal =
        (ToUintPtr(internalPool.GetMem()) + internalPool.GetSize() - 1L) > std::numeric_limits<uint32_t>::max();
    bool code = (ToUintPtr(codePool.GetMem()) + codePool.GetSize() - 1L) > std::numeric_limits<uint32_t>::max();
    bool compiler =
        (ToUintPtr(compilerPool.GetMem()) + compilerPool.GetSize() - 1L) > std::numeric_limits<uint32_t>::max();
    bool frame = (ToUintPtr(framesPool.GetMem()) + framesPool.GetSize() - 1L) > std::numeric_limits<uint32_t>::max();
    bool stacks = (ToUintPtr(stacksPool.GetMem()) + stacksPool.GetSize() - 1L) > std::numeric_limits<uint32_t>::max();
    ASSERT_TRUE(internal || code || compiler || frame || stacks);

    // cleaning
    memPool->FreePool(objectPool.GetMem(), objectPool.GetSize());
    memPool->FreePool(internalPool.GetMem(), internalPool.GetSize());
    memPool->FreePool(codePool.GetMem(), codePool.GetSize());
    memPool->FreePool(compilerPool.GetMem(), compilerPool.GetSize());
    memPool->FreePool(framesPool.GetMem(), framesPool.GetSize());
    memPool->FreePool(stacksPool.GetMem(), stacksPool.GetSize());
#endif
}

TEST_F(MMapMemPoolTest, PoolReturnTest)
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

TEST_F(MMapMemPoolTest, CheckEnoughPoolsTest)
{
    static constexpr size_t MMAP_MEM_POOL_SIZE = 10_MB;
    static constexpr size_t POOL_SIZE = 4_MB;
    MmapMemPool *memPool = CreateMMapMemPool(MMAP_MEM_POOL_SIZE);

    ASSERT_TRUE(memPool->HaveEnoughPoolsInObjectSpace(2U, POOL_SIZE));
    ASSERT_FALSE(memPool->HaveEnoughPoolsInObjectSpace(3U, POOL_SIZE));
    auto pool1 = memPool->AllocPool(3_MB, SpaceType::SPACE_TYPE_OBJECT, AllocatorType::HUMONGOUS_ALLOCATOR);
    ASSERT_TRUE(memPool->HaveEnoughPoolsInObjectSpace(1U, POOL_SIZE));
    ASSERT_FALSE(memPool->HaveEnoughPoolsInObjectSpace(2U, POOL_SIZE));
    auto pool2 = memPool->AllocPool(1_MB, SpaceType::SPACE_TYPE_OBJECT, AllocatorType::HUMONGOUS_ALLOCATOR);
    memPool->FreePool(pool1.GetMem(), pool1.GetSize());
    ASSERT_TRUE(memPool->HaveEnoughPoolsInObjectSpace(1U, POOL_SIZE));
    ASSERT_FALSE(memPool->HaveEnoughPoolsInObjectSpace(2U, POOL_SIZE));
    memPool->FreePool(pool2.GetMem(), pool2.GetSize());
    ASSERT_TRUE(memPool->HaveEnoughPoolsInObjectSpace(2U, POOL_SIZE));
    ASSERT_FALSE(memPool->HaveEnoughPoolsInObjectSpace(3U, POOL_SIZE));

    auto pool3 = memPool->AllocPool(4_MB, SpaceType::SPACE_TYPE_OBJECT, AllocatorType::HUMONGOUS_ALLOCATOR);
    auto pool4 = memPool->AllocPool(1_MB, SpaceType::SPACE_TYPE_OBJECT, AllocatorType::HUMONGOUS_ALLOCATOR);
    ASSERT_TRUE(memPool->HaveEnoughPoolsInObjectSpace(1U, POOL_SIZE));
    ASSERT_FALSE(memPool->HaveEnoughPoolsInObjectSpace(2U, POOL_SIZE));
    memPool->FreePool(pool3.GetMem(), pool3.GetSize());
    ASSERT_TRUE(memPool->HaveEnoughPoolsInObjectSpace(2U, POOL_SIZE));
    ASSERT_FALSE(memPool->HaveEnoughPoolsInObjectSpace(3U, POOL_SIZE));
    auto pool5 = memPool->AllocPool(5_MB, SpaceType::SPACE_TYPE_OBJECT, AllocatorType::HUMONGOUS_ALLOCATOR);
    ASSERT_TRUE(memPool->HaveEnoughPoolsInObjectSpace(1U, POOL_SIZE));
    ASSERT_FALSE(memPool->HaveEnoughPoolsInObjectSpace(2U, POOL_SIZE));
    memPool->FreePool(pool5.GetMem(), pool5.GetSize());
    ASSERT_TRUE(memPool->HaveEnoughPoolsInObjectSpace(2U, POOL_SIZE));
    ASSERT_FALSE(memPool->HaveEnoughPoolsInObjectSpace(3U, POOL_SIZE));
    memPool->FreePool(pool4.GetMem(), pool4.GetSize());
    ASSERT_TRUE(memPool->HaveEnoughPoolsInObjectSpace(2U, POOL_SIZE));
    ASSERT_FALSE(memPool->HaveEnoughPoolsInObjectSpace(3U, POOL_SIZE));
}

TEST_F(MMapMemPoolTest, TestMergeWithFreeSpace)
{
    static constexpr size_t MMAP_MEM_POOL_SIZE = 10_MB;
    MmapMemPool *memPool = CreateMMapMemPool(MMAP_MEM_POOL_SIZE);
    auto pool = memPool->AllocPool(7_MB, SpaceType::SPACE_TYPE_OBJECT, AllocatorType::HUMONGOUS_ALLOCATOR);
    memPool->FreePool(pool.GetMem(), pool.GetSize());
    pool = memPool->AllocPool(8_MB, SpaceType::SPACE_TYPE_OBJECT, AllocatorType::HUMONGOUS_ALLOCATOR);
    ASSERT_TRUE(pool.GetMem() != nullptr);
    ASSERT_EQ(8_MB, pool.GetSize());
    memPool->FreePool(pool.GetMem(), pool.GetSize());
}

TEST_F(MMapMemPoolTest, ReturnedToOsPlusZeroingMemoryTest)
{
    for (OSPagesPolicy a : {OSPagesPolicy::NO_RETURN, OSPagesPolicy::IMMEDIATE_RETURN}) {
        for (OSPagesPolicy b : {OSPagesPolicy::NO_RETURN, OSPagesPolicy::IMMEDIATE_RETURN}) {
            for (OSPagesPolicy c : {OSPagesPolicy::NO_RETURN, OSPagesPolicy::IMMEDIATE_RETURN}) {
                for (bool d : {false, true}) {
                    for (bool e : {false, true}) {
                        ReturnedToOsTest(a, b, c, d, e);
                    }
                }
            }
        }
    }
}

}  // namespace ark
