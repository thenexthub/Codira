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

#include <ctime>

#include "gtest/gtest.h"
#include "runtime/mem/gc/heap-space-misc/crossing_map.h"
#include "runtime/mem/internal_allocator-inl.h"

namespace ark::mem {

class CrossingMapTest : public testing::Test {
public:
    CrossingMapTest()
    {
        // Logger::InitializeStdLogging(Logger::Level::DEBUG, Logger::Component::ALL);
#ifdef PANDA_NIGHTLY_TEST_ON
        seed_ = std::time(NULL);
#else
        // NOLINTNEXTLINE(readability-magic-numbers)
        seed_ = 0xDEADBEEF;
#endif
        srand(seed_);
        ark::mem::MemConfig::Initialize(MEMORY_POOL_SIZE, MEMORY_POOL_SIZE, 0, 0, 0, 0);
        PoolManager::Initialize();
        startAddr_ = GetPoolMinAddress();
        memStats_ = new mem::MemStatsType();
        internalAllocator_ = new InternalAllocatorT<InternalAllocatorConfig::PANDA_ALLOCATORS>(memStats_);
        crossingMap_ = new CrossingMap(internalAllocator_, startAddr_, GetPoolSize());
        crossingMap_->Initialize();
        crossingMap_->InitializeCrossingMapForMemory(ToVoidPtr(startAddr_), GetPoolSize());
    }

    ~CrossingMapTest() override
    {
        crossingMap_->RemoveCrossingMapForMemory(ToVoidPtr(startAddr_), GetPoolSize());
        crossingMap_->Destroy();
        delete crossingMap_;
        delete static_cast<Allocator *>(internalAllocator_);
        PoolManager::Finalize();
        ark::mem::MemConfig::Finalize();
        // Logger::Destroy();
        delete memStats_;
    }

    NO_COPY_SEMANTIC(CrossingMapTest);
    NO_MOVE_SEMANTIC(CrossingMapTest);

protected:
    CrossingMap *GetCrossingMap()
    {
        return crossingMap_;
    }

    void *GetRandomObjAddr(size_t size)
    {
        ASSERT(size < GetPoolSize());
        // NOLINTNEXTLINE(cert-msc50-cpp)
        uintptr_t randOffset = rand() % (GetPoolSize() - size);
        randOffset = (randOffset >> CrossingMap::CROSSING_MAP_OBJ_ALIGNMENT) << CrossingMap::CROSSING_MAP_OBJ_ALIGNMENT;
        return ToVoidPtr(startAddr_ + randOffset);
    }

    void *AddPage(void *addr)
    {
        return ToVoidPtr(ToUintPtr(addr) + PANDA_PAGE_SIZE);
    }

    void *IncreaseAddr(void *addr, size_t value)
    {
        return ToVoidPtr(ToUintPtr(addr) + value);
    }

    void *DecreaseAddr(void *addr, size_t value)
    {
        return ToVoidPtr(ToUintPtr(addr) - value);
    }

    size_t GetMapNumFromAddr(void *addr)
    {
        return crossingMap_->GetMapNumFromAddr(addr);
    }

    static constexpr size_t MIN_GAP_BETWEEN_OBJECTS = 1U << CrossingMap::CROSSING_MAP_OBJ_ALIGNMENT;

    static constexpr size_t MEMORY_POOL_SIZE = 64_MB;

    static constexpr size_t POOLS_SIZE = CrossingMap::CROSSING_MAP_STATIC_ARRAY_GRANULARITY;

    uintptr_t GetPoolMinAddress()
    {
        return PoolManager::GetMmapMemPool()->GetMinObjectAddress();
    }

    size_t GetPoolSize()
    {
        return PoolManager::GetMmapMemPool()->GetMaxObjectAddress() -
               PoolManager::GetMmapMemPool()->GetMinObjectAddress();
    }

    void *GetLastObjectByte(void *objAddr, size_t objSize)
    {
        ASSERT(objSize != 0);
        return ToVoidPtr(ToUintPtr(objAddr) + objSize - 1U);
    }

    unsigned int GetSeed()
    {
        return seed_;
    }

private:
    unsigned int seed_;
    InternalAllocatorPtr internalAllocator_;
    uintptr_t startAddr_;
    CrossingMap *crossingMap_;
    mem::MemStatsType *memStats_;
};

TEST_F(CrossingMapTest, OneSmallObjTest)
{
    static constexpr size_t OBJ_SIZE = 1;
    // Use OBJ_SIZE + PAGE_SIZE here or we can get an overflow during AddPage(obj_addr)
    void *objAddr = GetRandomObjAddr(OBJ_SIZE + PANDA_PAGE_SIZE);
    GetCrossingMap()->AddObject(objAddr, OBJ_SIZE);
    ASSERT_TRUE(GetCrossingMap()->FindFirstObject(objAddr, objAddr) == objAddr) << " seed = " << GetSeed();
    ASSERT_TRUE(GetCrossingMap()->FindFirstObject(AddPage(objAddr), AddPage(objAddr)) == nullptr)
        << " seed = " << GetSeed();
}

TEST_F(CrossingMapTest, BigSmallObjTest)
{
    static constexpr size_t OBJ_SIZE = PANDA_PAGE_SIZE * 2U;
    void *objAddr = GetRandomObjAddr(OBJ_SIZE);
    GetCrossingMap()->AddObject(objAddr, OBJ_SIZE);
    ASSERT_TRUE(GetCrossingMap()->FindFirstObject(objAddr, ToVoidPtr(ToUintPtr(objAddr) + OBJ_SIZE)) == objAddr)
        << " seed = " << GetSeed();
    if (PANDA_CROSSING_MAP_MANAGE_CROSSED_BORDER) {
        ASSERT_TRUE(GetCrossingMap()->FindFirstObject(AddPage(objAddr), ToVoidPtr(ToUintPtr(objAddr) + OBJ_SIZE)) ==
                    objAddr)
            << " seed = " << GetSeed();
    }
    GetCrossingMap()->RemoveObject(objAddr, OBJ_SIZE);
    ASSERT_TRUE(GetCrossingMap()->FindFirstObject(objAddr, ToVoidPtr(ToUintPtr(objAddr) + OBJ_SIZE)) == nullptr)
        << " seed = " << GetSeed();
    if (PANDA_CROSSING_MAP_MANAGE_CROSSED_BORDER) {
        ASSERT_TRUE(GetCrossingMap()->FindFirstObject(AddPage(objAddr), ToVoidPtr(ToUintPtr(objAddr) + OBJ_SIZE)) ==
                    nullptr)
            << " seed = " << GetSeed();
    }
}

TEST_F(CrossingMapTest, HugeObjTest)
{
    static constexpr size_t OBJ_SIZE = MEMORY_POOL_SIZE >> 1U;
    void *objAddr = GetRandomObjAddr(OBJ_SIZE);
    GetCrossingMap()->AddObject(objAddr, OBJ_SIZE);
    ASSERT_TRUE(GetCrossingMap()->FindFirstObject(objAddr, objAddr) == objAddr) << " seed = " << GetSeed();
    if (PANDA_CROSSING_MAP_MANAGE_CROSSED_BORDER) {
        for (size_t i = 1_MB; i < OBJ_SIZE; i += 1_MB) {
            void *addr = ToVoidPtr(ToUintPtr(objAddr) + i);
            ASSERT_TRUE(GetCrossingMap()->FindFirstObject(addr, addr) == objAddr) << " seed = " << GetSeed();
        }
    }
    GetCrossingMap()->RemoveObject(objAddr, OBJ_SIZE);
    ASSERT_TRUE(GetCrossingMap()->FindFirstObject(objAddr, objAddr) == nullptr) << " seed = " << GetSeed();
    if (PANDA_CROSSING_MAP_MANAGE_CROSSED_BORDER) {
        for (size_t i = 1_MB; i < OBJ_SIZE; i += 1_MB) {
            void *addr = ToVoidPtr(ToUintPtr(objAddr) + i);
            ASSERT_TRUE(GetCrossingMap()->FindFirstObject(addr, addr) == nullptr) << " seed = " << GetSeed();
        }
    }
}

TEST_F(CrossingMapTest, TwoSequentialObjectsTest)
{
    static constexpr size_t FIRST_OBJ_SIZE = MIN_GAP_BETWEEN_OBJECTS;
    static constexpr size_t SECOND_OBJ_SIZE = 1_KB;
    // Add some extra memory for possible shifts
    void *firstObjAddr = GetRandomObjAddr(FIRST_OBJ_SIZE + SECOND_OBJ_SIZE + FIRST_OBJ_SIZE);
    void *secondObjAddr = IncreaseAddr(firstObjAddr, FIRST_OBJ_SIZE);
    // We must be sure that these objects will be saved in the same locations
    if (GetMapNumFromAddr(firstObjAddr) != GetMapNumFromAddr(secondObjAddr)) {
        firstObjAddr = IncreaseAddr(firstObjAddr, FIRST_OBJ_SIZE);
        secondObjAddr = IncreaseAddr(firstObjAddr, FIRST_OBJ_SIZE);
        ASSERT_TRUE(GetMapNumFromAddr(firstObjAddr) == GetMapNumFromAddr(secondObjAddr)) << " seed = " << GetSeed();
    }
    GetCrossingMap()->AddObject(firstObjAddr, FIRST_OBJ_SIZE);
    GetCrossingMap()->AddObject(secondObjAddr, SECOND_OBJ_SIZE);

    ASSERT_TRUE(GetCrossingMap()->FindFirstObject(firstObjAddr, firstObjAddr) == firstObjAddr)
        << " seed = " << GetSeed();

    GetCrossingMap()->RemoveObject(firstObjAddr, FIRST_OBJ_SIZE, secondObjAddr);
    ASSERT_TRUE(GetCrossingMap()->FindFirstObject(firstObjAddr, firstObjAddr) == secondObjAddr)
        << " seed = " << GetSeed();

    GetCrossingMap()->RemoveObject(secondObjAddr, SECOND_OBJ_SIZE);
    ASSERT_TRUE(GetCrossingMap()->FindFirstObject(firstObjAddr, firstObjAddr) == nullptr) << " seed = " << GetSeed();
}

TEST_F(CrossingMapTest, TwoNonSequentialObjectsTest)
{
    static constexpr size_t FIRST_OBJ_SIZE = MIN_GAP_BETWEEN_OBJECTS;
    static constexpr size_t GAP_BETWEEN_OBJECTS = 1_MB;
    static constexpr size_t SECOND_OBJ_SIZE = 1_KB;
    // Add some extra memory for possible shifts
    void *firstObjAddr = GetRandomObjAddr(FIRST_OBJ_SIZE + SECOND_OBJ_SIZE + GAP_BETWEEN_OBJECTS);
    void *secondObjAddr = IncreaseAddr(firstObjAddr, FIRST_OBJ_SIZE + GAP_BETWEEN_OBJECTS);

    GetCrossingMap()->AddObject(firstObjAddr, FIRST_OBJ_SIZE);
    GetCrossingMap()->AddObject(secondObjAddr, SECOND_OBJ_SIZE);

    ASSERT_TRUE(GetCrossingMap()->FindFirstObject(firstObjAddr, secondObjAddr) == firstObjAddr)
        << " seed = " << GetSeed();

    GetCrossingMap()->RemoveObject(firstObjAddr, FIRST_OBJ_SIZE, secondObjAddr);
    ASSERT_TRUE(GetCrossingMap()->FindFirstObject(firstObjAddr, firstObjAddr) == nullptr) << " seed = " << GetSeed();
    ASSERT_TRUE(GetCrossingMap()->FindFirstObject(firstObjAddr, secondObjAddr) == secondObjAddr)
        << " seed = " << GetSeed();

    GetCrossingMap()->RemoveObject(secondObjAddr, SECOND_OBJ_SIZE);
    ASSERT_TRUE(GetCrossingMap()->FindFirstObject(firstObjAddr, secondObjAddr) == nullptr) << " seed = " << GetSeed();
}

TEST_F(CrossingMapTest, ThreeSequentialObjectsTest)
{
    static constexpr size_t FIRST_OBJ_SIZE = 4_MB;
    static constexpr size_t SECOND_OBJ_SIZE = MIN_GAP_BETWEEN_OBJECTS;
    static constexpr size_t THIRD_OBJ_SIZE = 1_KB;
    auto seedInfo = " seed = " + std::to_string(GetSeed());

    // Add some extra memory for possible shifts
    void *firstObjAddr = GetRandomObjAddr(FIRST_OBJ_SIZE + SECOND_OBJ_SIZE + THIRD_OBJ_SIZE + 3U * SECOND_OBJ_SIZE);
    void *secondObjAddr = IncreaseAddr(firstObjAddr, FIRST_OBJ_SIZE);
    void *thirdObjAddr = IncreaseAddr(secondObjAddr, SECOND_OBJ_SIZE);

    // We must be sure that the first object will cross the borders for the second one
    if (GetMapNumFromAddr(GetLastObjectByte(firstObjAddr, FIRST_OBJ_SIZE)) != GetMapNumFromAddr(secondObjAddr)) {
        firstObjAddr = IncreaseAddr(firstObjAddr, SECOND_OBJ_SIZE);
        secondObjAddr = IncreaseAddr(firstObjAddr, FIRST_OBJ_SIZE);
        thirdObjAddr = IncreaseAddr(secondObjAddr, SECOND_OBJ_SIZE);
        ASSERT_TRUE(GetMapNumFromAddr(GetLastObjectByte(firstObjAddr, FIRST_OBJ_SIZE)) ==
                    GetMapNumFromAddr(secondObjAddr))
            << seedInfo;
    }

    // We must be sure that the second and the third object will be saved in the same locations
    if (GetMapNumFromAddr(secondObjAddr) != GetMapNumFromAddr(thirdObjAddr)) {
        firstObjAddr = IncreaseAddr(firstObjAddr, 2U * SECOND_OBJ_SIZE);
        secondObjAddr = IncreaseAddr(firstObjAddr, FIRST_OBJ_SIZE);
        thirdObjAddr = IncreaseAddr(secondObjAddr, SECOND_OBJ_SIZE);
        ASSERT_TRUE(GetMapNumFromAddr(GetLastObjectByte(firstObjAddr, FIRST_OBJ_SIZE)) ==
                    GetMapNumFromAddr(secondObjAddr))
            << seedInfo;
        ASSERT_TRUE(GetMapNumFromAddr(secondObjAddr) == GetMapNumFromAddr(thirdObjAddr)) << seedInfo;
    }

    GetCrossingMap()->AddObject(firstObjAddr, FIRST_OBJ_SIZE);
    GetCrossingMap()->AddObject(secondObjAddr, SECOND_OBJ_SIZE);
    GetCrossingMap()->AddObject(thirdObjAddr, THIRD_OBJ_SIZE);

    if (PANDA_CROSSING_MAP_MANAGE_CROSSED_BORDER) {
        ASSERT_TRUE(GetCrossingMap()->FindFirstObject(secondObjAddr, secondObjAddr) == firstObjAddr) << seedInfo;
    } else {
        ASSERT_TRUE(GetCrossingMap()->FindFirstObject(secondObjAddr, secondObjAddr) == secondObjAddr) << seedInfo;
    }

    GetCrossingMap()->RemoveObject(secondObjAddr, SECOND_OBJ_SIZE, thirdObjAddr, firstObjAddr, FIRST_OBJ_SIZE);
    if (PANDA_CROSSING_MAP_MANAGE_CROSSED_BORDER) {
        ASSERT_TRUE(GetCrossingMap()->FindFirstObject(secondObjAddr, secondObjAddr) == firstObjAddr) << seedInfo;
    } else {
        ASSERT_TRUE(GetCrossingMap()->FindFirstObject(secondObjAddr, secondObjAddr) == thirdObjAddr) << seedInfo;
    }
    GetCrossingMap()->RemoveObject(thirdObjAddr, THIRD_OBJ_SIZE, nullptr, firstObjAddr, FIRST_OBJ_SIZE);
    if (PANDA_CROSSING_MAP_MANAGE_CROSSED_BORDER) {
        ASSERT_TRUE(GetCrossingMap()->FindFirstObject(secondObjAddr, secondObjAddr) == firstObjAddr) << seedInfo;
    } else {
        ASSERT_TRUE(GetCrossingMap()->FindFirstObject(secondObjAddr, secondObjAddr) == nullptr) << seedInfo;
    }

    GetCrossingMap()->RemoveObject(firstObjAddr, FIRST_OBJ_SIZE);
    ASSERT_TRUE(GetCrossingMap()->FindFirstObject(secondObjAddr, secondObjAddr) == nullptr) << seedInfo;
}

TEST_F(CrossingMapTest, InitializeCrosingMapForMemoryTest)
{
    static constexpr size_t POOL_COUNT = 6;
    static constexpr size_t GRANULARITY = 2;
    GetCrossingMap()->RemoveCrossingMapForMemory(ToVoidPtr(GetPoolMinAddress()), GetPoolSize());
    void *startAddr = GetRandomObjAddr((POOLS_SIZE * 2U + PANDA_POOL_ALIGNMENT_IN_BYTES) * POOL_COUNT +
                                       PANDA_POOL_ALIGNMENT_IN_BYTES);
    uintptr_t alignedStartAddr = AlignUp(ToUintPtr(startAddr), PANDA_POOL_ALIGNMENT_IN_BYTES);

    std::array<bool, POOL_COUNT> deletedPools {};
    for (size_t i = 0; i < POOL_COUNT; i++) {
        void *poolAddr = ToVoidPtr(alignedStartAddr + i * (POOLS_SIZE * 2U + PANDA_POOL_ALIGNMENT_IN_BYTES));
        GetCrossingMap()->InitializeCrossingMapForMemory(poolAddr, POOLS_SIZE * 2U);
        deletedPools[i] = false;
    }

    for (size_t i = 0; i < POOL_COUNT; i += GRANULARITY) {
        void *poolAddr = ToVoidPtr(alignedStartAddr + i * (POOLS_SIZE * 2U + PANDA_POOL_ALIGNMENT_IN_BYTES));
        GetCrossingMap()->RemoveCrossingMapForMemory(poolAddr, POOLS_SIZE * 2U);
        deletedPools[i] = true;
    }

    for (size_t i = 0; i < POOL_COUNT; i++) {
        if (deletedPools[i]) {
            continue;
        }
        void *poolAddr = ToVoidPtr(alignedStartAddr + i * (POOLS_SIZE * 2U + PANDA_POOL_ALIGNMENT_IN_BYTES));
        GetCrossingMap()->RemoveCrossingMapForMemory(poolAddr, POOLS_SIZE * 2U);
    }

    GetCrossingMap()->InitializeCrossingMapForMemory(ToVoidPtr(GetPoolMinAddress()), GetPoolSize());
}

}  // namespace ark::mem
