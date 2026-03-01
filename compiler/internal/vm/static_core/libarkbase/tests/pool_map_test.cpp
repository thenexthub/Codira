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

#include "gtest/gtest.h"
#include "libarkbase/mem/pool_map.h"
#include "libarkbase/mem/mem_pool.h"
#include "libarkbase/mem/mem.h"

#include <ctime>
#include <algorithm>

namespace ark {

class PoolMapTest : public testing::Test {
public:
    PoolMapTest()
    {
#ifdef PANDA_NIGHTLY_TEST_ON
        seed_ = std::time(NULL);
#else
        // NOLINTNEXTLINE(readability-magic-numbers)
        seed_ = 0xDEADBEEF;
#endif
        srand(seed_);
    }

    ~PoolMapTest() override
    {
        ResetPoolMap();
    }

    NO_COPY_SEMANTIC(PoolMapTest);
    NO_MOVE_SEMANTIC(PoolMapTest);

protected:
    static constexpr size_t MINIMAL_POOL_SIZE = PANDA_POOL_ALIGNMENT_IN_BYTES;

    void AddToPoolMap(Pool pool, SpaceType spaceType, AllocatorType allocatorType, void *allocatorAddr = nullptr)
    {
        if (allocatorAddr == nullptr) {
            allocatorAddr = pool.GetMem();
        }
        pools_.push_back(pool);
        poolMap_.AddPoolToMap(pool.GetMem(), pool.GetSize(), spaceType, allocatorType, allocatorAddr);
    }

    void RemovePoolFromMap(Pool pool)
    {
        auto items = std::remove(pools_.begin(), pools_.end(), pool);
        ASSERT(items != pools_.end());
        pools_.erase(items, pools_.end());
        poolMap_.RemovePoolFromMap(pool.GetMem(), pool.GetSize());
    }

    void ResetPoolMap()
    {
        for (auto i : pools_) {
            poolMap_.RemovePoolFromMap(i.GetMem(), i.GetSize());
        }
        pools_.clear();
    }

    bool IsEmptyPoolMap() const
    {
        return poolMap_.IsEmpty();
    }

    SpaceType GetRandSpaceType() const
    {
        // NOLINTNEXTLINE(cert-msc50-cpp)
        int randIndex = rand() % ALL_SPACE_TYPES.size();
        return ALL_SPACE_TYPES[randIndex];
    }

    AllocatorType GetRandAllocatorType() const
    {
        // NOLINTNEXTLINE(cert-msc50-cpp)
        int randIndex = rand() % ALL_ALLOCATOR_TYPES.size();
        return ALL_ALLOCATOR_TYPES[randIndex];
    }

    size_t RandHeapAddr() const
    {
        // NOLINTNEXTLINE(cert-msc50-cpp)
        return AlignUp(rand() % PANDA_MAX_HEAP_SIZE, DEFAULT_ALIGNMENT_IN_BYTES);
    }

    void CheckRandomPoolAddress(Pool pool, SpaceType spaceType, AllocatorType allocatorType, uintptr_t allocatorAddr)
    {
        void *poolAddr = RandAddrFromPool(pool);
        ASSERT_EQ(GetSpaceTypeForAddr(poolAddr), spaceType);
        ASSERT_EQ(GetAllocatorInfoForAddr(poolAddr).GetType(), allocatorType);
        ASSERT_EQ(ToUintPtr(GetAllocatorInfoForAddr(poolAddr).GetAllocatorHeaderAddr()), allocatorAddr);
    }

private:
    void *RandAddrFromPool(Pool pool) const
    {
        // NOLINTNEXTLINE(cert-msc50-cpp)
        return ToVoidPtr(ToUintPtr(pool.GetMem()) + rand() % pool.GetSize());
    }

    AllocatorInfo GetAllocatorInfoForAddr(const void *addr) const
    {
        return poolMap_.GetAllocatorInfo(addr);
    }

    SpaceType GetSpaceTypeForAddr(const void *addr) const
    {
        return poolMap_.GetSpaceType(addr);
    }

    static constexpr std::array<SpaceType, 6U> ALL_SPACE_TYPES = {SpaceType::SPACE_TYPE_OBJECT,
                                                                  SpaceType::SPACE_TYPE_HUMONGOUS_OBJECT,
                                                                  SpaceType::SPACE_TYPE_NON_MOVABLE_OBJECT,
                                                                  SpaceType::SPACE_TYPE_INTERNAL,
                                                                  SpaceType::SPACE_TYPE_CODE,
                                                                  SpaceType::SPACE_TYPE_COMPILER};

    static constexpr std::array<AllocatorType, 8U> ALL_ALLOCATOR_TYPES = {
        AllocatorType::RUNSLOTS_ALLOCATOR, AllocatorType::FREELIST_ALLOCATOR, AllocatorType::HUMONGOUS_ALLOCATOR,
        AllocatorType::ARENA_ALLOCATOR,    AllocatorType::BUMP_ALLOCATOR,     AllocatorType::TLAB_ALLOCATOR,
        AllocatorType::REGION_ALLOCATOR,   AllocatorType::FRAME_ALLOCATOR};

    unsigned int seed_;
    std::vector<Pool> pools_;
    PoolMap poolMap_;
};

TEST_F(PoolMapTest, TwoConsistentPoolsTest)
{
    static constexpr size_t FIRST_POOL_SIZE = 4U * MINIMAL_POOL_SIZE;
    static constexpr size_t SECOND_POOL_SIZE = 10U * MINIMAL_POOL_SIZE;
    static constexpr uintptr_t FIRST_POOL_ADDR = 0;
    static constexpr uintptr_t SECOND_POOL_ADDR = FIRST_POOL_ADDR + FIRST_POOL_SIZE;
    static constexpr SpaceType FIRST_SPACE_TYPE = SpaceType::SPACE_TYPE_INTERNAL;
    static constexpr SpaceType SECOND_SPACE_TYPE = SpaceType::SPACE_TYPE_OBJECT;
    static constexpr AllocatorType FIRST_ALLOCATOR_TYPE = AllocatorType::RUNSLOTS_ALLOCATOR;
    static constexpr AllocatorType SECOND_ALLOCATOR_TYPE = AllocatorType::FREELIST_ALLOCATOR;

    uintptr_t firstPoolAllocatorHeaderAddr = RandHeapAddr();

    Pool firstPool(FIRST_POOL_SIZE, ToVoidPtr(FIRST_POOL_ADDR));
    Pool secondPool(SECOND_POOL_SIZE, ToVoidPtr(SECOND_POOL_ADDR));

    AddToPoolMap(firstPool, FIRST_SPACE_TYPE, FIRST_ALLOCATOR_TYPE, ToVoidPtr(firstPoolAllocatorHeaderAddr));
    AddToPoolMap(secondPool, SECOND_SPACE_TYPE, SECOND_ALLOCATOR_TYPE);

    CheckRandomPoolAddress(firstPool, FIRST_SPACE_TYPE, FIRST_ALLOCATOR_TYPE, firstPoolAllocatorHeaderAddr);
    // We haven't initialized second allocator header address.
    // Therefore it must return a pointer to the first pool byte.
    CheckRandomPoolAddress(secondPool, SECOND_SPACE_TYPE, SECOND_ALLOCATOR_TYPE, SECOND_POOL_ADDR);

    // Check that we remove elements from pool map correctly
    RemovePoolFromMap(firstPool);
    RemovePoolFromMap(secondPool);

    ASSERT_TRUE(IsEmptyPoolMap());
    ResetPoolMap();
}

TEST_F(PoolMapTest, AddRemoveDifferentPoolsTest)
{
    static constexpr size_t MAX_POOL_SIZE = 256U * MINIMAL_POOL_SIZE;
    static constexpr size_t ITERATIONS = 200;
    static constexpr uintptr_t POOL_START_ADDR = PANDA_POOL_ALIGNMENT_IN_BYTES;
    for (size_t i = 0; i < ITERATIONS; i++) {
        // NOLINTNEXTLINE(cert-msc50-cpp)
        size_t poolSize = AlignUp(rand() % MAX_POOL_SIZE, PANDA_POOL_ALIGNMENT_IN_BYTES);
        SpaceType space = GetRandSpaceType();
        AllocatorType allocator = GetRandAllocatorType();
        Pool pool(poolSize, ToVoidPtr(POOL_START_ADDR));

        AddToPoolMap(pool, space, allocator);
        CheckRandomPoolAddress(pool, space, allocator, POOL_START_ADDR);
        RemovePoolFromMap(pool);
    }

    ASSERT_TRUE(IsEmptyPoolMap());
    ResetPoolMap();
}

}  // namespace ark
