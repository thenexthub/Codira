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

#include <sys/mman.h>

#include "libarkbase/mem/mem.h"
#include "libarkbase/os/mem.h"
#include "libarkbase/utils/asan_interface.h"
#include "libarkbase/utils/logger.h"
#include "libarkbase/utils/math_helpers.h"
#include "runtime/include/runtime.h"
#include "runtime/mem/alloc_config.h"
#include "runtime/mem/freelist_allocator-inl.h"
#include "runtime/tests/allocator_test_base.h"

namespace ark::mem {

using NonObjectFreeListAllocator = FreeListAllocator<EmptyAllocConfigWithCrossingMap>;

class FreeListAllocatorTest : public AllocatorTest<NonObjectFreeListAllocator> {
public:
    NO_COPY_SEMANTIC(FreeListAllocatorTest);
    NO_MOVE_SEMANTIC(FreeListAllocatorTest);

    FreeListAllocatorTest()
    {
        // Logger::InitializeStdLogging(Logger::Level::DEBUG, Logger::Component::ALL);
        // We need to create a runtime instance to be able to use CrossingMap.
        options_.SetShouldLoadBootPandaFiles(false);
        options_.SetShouldInitializeIntrinsics(false);
        Runtime::Create(options_);
        thread_ = ark::MTManagedThread::GetCurrent();
        thread_->ManagedCodeBegin();
        if (!CrossingMapSingleton::IsCreated()) {
            CrossingMapSingleton::Create();
            crossingmapManualHandling_ = true;
        }
    }

    ~FreeListAllocatorTest() override
    {
        thread_->ManagedCodeEnd();
        ClearPoolManager();
        if (crossingmapManualHandling_) {
            CrossingMapSingleton::Destroy();
        }
        Runtime::Destroy();
        // Logger::Destroy();
    }

protected:
    static constexpr size_t DEFAULT_POOL_SIZE_FOR_ALLOC = NonObjectFreeListAllocator::GetMinPoolSize();
    static constexpr size_t DEFAULT_POOL_ALIGNMENT_FOR_ALLOC = FREELIST_DEFAULT_ALIGNMENT;
    static constexpr size_t POOL_HEADER_SIZE = sizeof(NonObjectFreeListAllocator::MemoryPoolHeader);
    static constexpr size_t MAX_ALLOC_SIZE = NonObjectFreeListAllocator::GetMaxSize();

    void AddMemoryPoolToAllocator(NonObjectFreeListAllocator &alloc) override
    {
        os::memory::LockHolder lock(poolLock_);
        Pool pool = PoolManager::GetMmapMemPool()->AllocPool(DEFAULT_POOL_SIZE_FOR_ALLOC, SpaceType::SPACE_TYPE_OBJECT,
                                                             AllocatorType::FREELIST_ALLOCATOR, &alloc);
        ASSERT(pool.GetSize() == DEFAULT_POOL_SIZE_FOR_ALLOC);
        if (pool.GetMem() == nullptr) {
            ASSERT_TRUE(0 && "Can't get a new pool from PoolManager");
        }
        allocatedPoolsByPoolManager_.push_back(pool);
        if (!alloc.AddMemoryPool(pool.GetMem(), pool.GetSize())) {
            ASSERT_TRUE(0 && "Can't add mem pool to allocator");
        }
    }

    void AddMemoryPoolToAllocatorProtected(NonObjectFreeListAllocator &alloc) override
    {
        // We use common PoolManager from Runtime. Therefore, we have the same pool allocation for both cases.
        AddMemoryPoolToAllocator(alloc);
    }

    bool AllocatedByThisAllocator(NonObjectFreeListAllocator &allocator, void *mem) override
    {
        return allocator.AllocatedByFreeListAllocator(mem);
    }

    void ClearPoolManager(bool clearCrossingMap = false)
    {
        for (auto i : allocatedPoolsByPoolManager_) {
            PoolManager::GetMmapMemPool()->FreePool(i.GetMem(), i.GetSize());
            if (clearCrossingMap) {
                // We need to remove corresponding Pools from the CrossingMap
                CrossingMapSingleton::RemoveCrossingMapForMemory(i.GetMem(), i.GetSize());
            }
        }
        allocatedPoolsByPoolManager_.clear();
    }

private:
    ark::MTManagedThread *thread_ {};
    std::vector<Pool> allocatedPoolsByPoolManager_;
    RuntimeOptions options_;
    bool crossingmapManualHandling_ {false};
    // Mutex, which allows only one thread to add pool to the pool vector
    os::memory::Mutex poolLock_;
};

TEST_F(FreeListAllocatorTest, SimpleAllocateDifferentObjSizeTest)
{
    LOG(DEBUG, ALLOC) << "SimpleAllocateDifferentObjSizeTest";
    auto *memStats = new mem::MemStatsType();
    NonObjectFreeListAllocator allocator(memStats);
    AddMemoryPoolToAllocator(allocator);
    // NOLINTNEXTLINE(readability-magic-numbers)
    for (size_t i = 23U; i < 300U; i++) {
        void *mem = allocator.Alloc(i);
        (void)mem;
        LOG(DEBUG, ALLOC) << "Allocate obj with size " << i << " at " << std::hex << mem;
    }
    delete memStats;
}

TEST_F(FreeListAllocatorTest, AllocateWriteFreeTest)
{
    // NOLINTNEXTLINE(readability-magic-numbers)
    AllocateAndFree(FREELIST_ALLOCATOR_MIN_SIZE, 512U);
}

TEST_F(FreeListAllocatorTest, AllocateRandomFreeTest)
{
    static constexpr size_t ALLOC_SIZE = FREELIST_ALLOCATOR_MIN_SIZE;
    static constexpr size_t ELEMENTS_COUNT = 512;
    static constexpr size_t POOLS_COUNT = 1;
    AllocateFreeDifferentSizesTest<ALLOC_SIZE, 2U * ALLOC_SIZE>(ELEMENTS_COUNT, POOLS_COUNT);
}

TEST_F(FreeListAllocatorTest, AllocateTooBigObjTest)
{
    AllocateTooBigObjectTest<MAX_ALLOC_SIZE + 1>();
}

TEST_F(FreeListAllocatorTest, AlignmentAllocTest)
{
    static constexpr size_t POOLS_COUNT = 2;
    // NOLINTNEXTLINE(readability-magic-numbers)
    AlignedAllocFreeTest<FREELIST_ALLOCATOR_MIN_SIZE, MAX_ALLOC_SIZE / 4096U>(POOLS_COUNT);
}

TEST_F(FreeListAllocatorTest, AllocateTooMuchTest)
{
    static constexpr size_t ALLOC_SIZE = FREELIST_ALLOCATOR_MIN_SIZE;
    AllocateTooMuchTest(ALLOC_SIZE, DEFAULT_POOL_SIZE_FOR_ALLOC / ALLOC_SIZE);
}

TEST_F(FreeListAllocatorTest, ObjectIteratorTest)
{
    ObjectIteratorTest<FREELIST_ALLOCATOR_MIN_SIZE, MAX_ALLOC_SIZE>();
}

TEST_F(FreeListAllocatorTest, ObjectCollectionTest)
{
    ObjectCollectionTest<FREELIST_ALLOCATOR_MIN_SIZE, MAX_ALLOC_SIZE>();
}

TEST_F(FreeListAllocatorTest, ObjectIteratorInRangeTest)
{
    ObjectIteratorInRangeTest<FREELIST_ALLOCATOR_MIN_SIZE, MAX_ALLOC_SIZE>(
        CrossingMapSingleton::GetCrossingMapGranularity());
}

TEST_F(FreeListAllocatorTest, AsanTest)
{
    AsanTest();
}

TEST_F(FreeListAllocatorTest, VisitAndRemoveFreePoolsTest)
{
    static constexpr size_t POOLS_COUNT = 5;
    VisitAndRemoveFreePools<POOLS_COUNT>(MAX_ALLOC_SIZE);
}

TEST_F(FreeListAllocatorTest, AllocatedByFreeListAllocatorTest)
{
    AllocatedByThisAllocatorTest();
}

TEST_F(FreeListAllocatorTest, FailedLinksTest)
{
    static constexpr size_t MIN_ALLOC_SIZE = FREELIST_ALLOCATOR_MIN_SIZE;
    auto *memStats = new mem::MemStatsType();
    NonObjectFreeListAllocator allocator(memStats);
    AddMemoryPoolToAllocator(allocator);
    std::pair<void *, size_t> pair;

    std::array<std::pair<void *, size_t>, 3U> memoryElements;
    for (size_t i = 0; i < 3U; i++) {
        void *mem = allocator.Alloc(MIN_ALLOC_SIZE);
        ASSERT_TRUE(mem != nullptr);
        size_t index = SetBytesFromByteArray(mem, MIN_ALLOC_SIZE);
        memoryElements.at(i) = std::pair<void *, size_t> {mem, index};
    }

    pair = memoryElements[1];
    ASSERT_TRUE(CompareBytesWithByteArray(std::get<0>(pair), MIN_ALLOC_SIZE, std::get<1>(pair)));
    allocator.Free(std::get<0>(pair));

    pair = memoryElements[0];
    ASSERT_TRUE(CompareBytesWithByteArray(std::get<0>(pair), MIN_ALLOC_SIZE, std::get<1>(pair)));
    allocator.Free(std::get<0>(pair));

    {
        void *mem = allocator.Alloc(MIN_ALLOC_SIZE * 2U);
        ASSERT_TRUE(mem != nullptr);
        size_t index = SetBytesFromByteArray(mem, MIN_ALLOC_SIZE * 2U);
        memoryElements.at(0) = std::pair<void *, size_t> {mem, index};
    }

    {
        void *mem = allocator.Alloc(MIN_ALLOC_SIZE);
        ASSERT_TRUE(mem != nullptr);
        size_t index = SetBytesFromByteArray(mem, MIN_ALLOC_SIZE);
        memoryElements.at(1) = std::pair<void *, size_t> {mem, index};
    }

    {
        pair = memoryElements[0];
        ASSERT_TRUE(CompareBytesWithByteArray(std::get<0>(pair), MIN_ALLOC_SIZE * 2U, std::get<1>(pair)));
        allocator.Free(std::get<0>(pair));
    }

    {
        pair = memoryElements[1];
        ASSERT_TRUE(CompareBytesWithByteArray(std::get<0>(pair), MIN_ALLOC_SIZE, std::get<1>(pair)));
        allocator.Free(std::get<0>(pair));
    }

    {
        pair = memoryElements[2U];
        ASSERT_TRUE(CompareBytesWithByteArray(std::get<0>(pair), MIN_ALLOC_SIZE, std::get<1>(pair)));
        allocator.Free(std::get<0>(pair));
    }
    delete memStats;
}

TEST_F(FreeListAllocatorTest, MaxAllocationSizeTest)
{
    static constexpr size_t ALLOC_SIZE = MAX_ALLOC_SIZE;
    static constexpr size_t ALLOC_COUNT = 2;
    auto *memStats = new mem::MemStatsType();
    NonObjectFreeListAllocator allocator(memStats);
    AddMemoryPoolToAllocator(allocator);
    std::array<void *, ALLOC_COUNT> memoryElements {};
    for (size_t i = 0; i < ALLOC_COUNT; i++) {
        void *mem = allocator.Alloc(ALLOC_SIZE);
        ASSERT_TRUE(mem != nullptr);
        memoryElements.at(i) = mem;
    }
    for (size_t i = 0; i < ALLOC_COUNT; i++) {
        allocator.Free(memoryElements.at(i));
    }
    delete memStats;
}

TEST_F(FreeListAllocatorTest, AllocateTheWholePoolFreeAndAllocateAgainTest)
{
    size_t minSizePowerOfTwo;
    if ((FREELIST_ALLOCATOR_MIN_SIZE & (FREELIST_ALLOCATOR_MIN_SIZE - 1)) == 0U) {
        minSizePowerOfTwo = ark::helpers::math::GetIntLog2(FREELIST_ALLOCATOR_MIN_SIZE);
    } else {
        // NOLINTNEXTLINE(readability-magic-numbers)
        minSizePowerOfTwo = ceil(std::log(FREELIST_ALLOCATOR_MIN_SIZE) / std::log(2.0F));
    }
    if (((1U << minSizePowerOfTwo) - sizeof(freelist::MemoryBlockHeader)) < FREELIST_ALLOCATOR_MIN_SIZE) {
        minSizePowerOfTwo++;
    }
    size_t allocSize = (1U << minSizePowerOfTwo) - sizeof(freelist::MemoryBlockHeader);
    // To cover all memory we need to consider pool header size at first bytes of pool memory.
    size_t firstAllocSize = (1U << minSizePowerOfTwo) - sizeof(freelist::MemoryBlockHeader) - POOL_HEADER_SIZE;
    if (firstAllocSize < FREELIST_ALLOCATOR_MIN_SIZE) {
        firstAllocSize = (1U << (minSizePowerOfTwo + 1)) - sizeof(freelist::MemoryBlockHeader) - POOL_HEADER_SIZE;
    }
    auto *memStats = new mem::MemStatsType();
    NonObjectFreeListAllocator allocator(memStats);
    AddMemoryPoolToAllocator(allocator);
    std::vector<void *> memoryElements;
    size_t allocCount = 0;

    // Allocate first element
    void *firstAllocMem = allocator.Alloc(firstAllocSize);
    ASSERT_TRUE(firstAllocMem != nullptr);

    // Allocate and use the whole alloc pool
    while (true) {
        void *mem = allocator.Alloc(allocSize);
        if (mem == nullptr) {
            break;
        }
        allocCount++;
        memoryElements.push_back(mem);
    }

    // Free all elements
    allocator.Free(firstAllocMem);
    for (size_t i = 0; i < allocCount; i++) {
        allocator.Free(memoryElements.back());
        memoryElements.pop_back();
    }

    // Allocate first element again
    firstAllocMem = allocator.Alloc(firstAllocSize);
    ASSERT_TRUE(firstAllocMem != nullptr);

    // Allocate again
    for (size_t i = 0; i < allocCount; i++) {
        void *mem = allocator.Alloc(allocSize);
        ASSERT_TRUE(mem != nullptr);
        memoryElements.push_back(mem);
    }

    // Free all elements again
    allocator.Free(firstAllocMem);
    for (size_t i = 0; i < allocCount; i++) {
        allocator.Free(memoryElements.back());
        memoryElements.pop_back();
    }
    delete memStats;
}

TEST_F(FreeListAllocatorTest, MTAllocFreeTest)
{
    static constexpr size_t MIN_ELEMENTS_COUNT = 500;
    static constexpr size_t MAX_ELEMENTS_COUNT = 1000;
#if defined(PANDA_TARGET_ARM64) || defined(PANDA_TARGET_32)
    // We have an issue with QEMU during MT tests. Issue 2852
    static constexpr size_t THREADS_COUNT = 1;
#else
    static constexpr size_t THREADS_COUNT = 10;
#endif
    static constexpr size_t MAX_MT_ALLOC_SIZE = MAX_ALLOC_SIZE / 128;
    static constexpr size_t MT_TEST_RUN_COUNT = 5;
    // Threads can concurrently add Pools to the allocator, therefore, we must make it into account
    // And also we must take fragmentation into account
    ASSERT_TRUE(mem::MemConfig::GetHeapSizeLimit() >
                2U * (AlignUp(MAX_ELEMENTS_COUNT * MAX_MT_ALLOC_SIZE, DEFAULT_POOL_SIZE_FOR_ALLOC)) +
                    THREADS_COUNT * DEFAULT_POOL_SIZE_FOR_ALLOC);
    for (size_t i = 0; i < MT_TEST_RUN_COUNT; i++) {
        MtAllocFreeTest<FREELIST_ALLOCATOR_MIN_SIZE, MAX_MT_ALLOC_SIZE, THREADS_COUNT>(MIN_ELEMENTS_COUNT,
                                                                                       MAX_ELEMENTS_COUNT);
        ClearPoolManager(true);
    }
}

TEST_F(FreeListAllocatorTest, MTAllocIterateTest)
{
    static constexpr size_t MIN_ELEMENTS_COUNT = 500;
    static constexpr size_t MAX_ELEMENTS_COUNT = 1000;
#if defined(PANDA_TARGET_ARM64) || defined(PANDA_TARGET_32)
    // We have an issue with QEMU during MT tests. Issue 2852
    static constexpr size_t THREADS_COUNT = 1;
#else
    static constexpr size_t THREADS_COUNT = 10;
#endif
    static constexpr size_t MAX_MT_ALLOC_SIZE = MAX_ALLOC_SIZE / 128;
    static constexpr size_t MT_TEST_RUN_COUNT = 5;
    // Threads can concurrently add Pools to the allocator, therefore, we must make it into account
    // And also we must take fragmentation into account
    ASSERT_TRUE(mem::MemConfig::GetHeapSizeLimit() >
                2U * (AlignUp(MAX_ELEMENTS_COUNT * MAX_MT_ALLOC_SIZE, DEFAULT_POOL_SIZE_FOR_ALLOC)) +
                    THREADS_COUNT * DEFAULT_POOL_SIZE_FOR_ALLOC);
    for (size_t i = 0; i < MT_TEST_RUN_COUNT; i++) {
        MtAllocIterateTest<FREELIST_ALLOCATOR_MIN_SIZE, MAX_MT_ALLOC_SIZE, THREADS_COUNT>(
            MIN_ELEMENTS_COUNT, MAX_ELEMENTS_COUNT, CrossingMapSingleton::GetCrossingMapGranularity());
        ClearPoolManager(true);
    }
}

TEST_F(FreeListAllocatorTest, MTAllocCollectTest)
{
    static constexpr size_t MIN_ELEMENTS_COUNT = 500;
    static constexpr size_t MAX_ELEMENTS_COUNT = 1000;
#if defined(PANDA_TARGET_ARM64) || defined(PANDA_TARGET_32)
    // We have an issue with QEMU during MT tests. Issue 2852
    static constexpr size_t THREADS_COUNT = 1;
#else
    static constexpr size_t THREADS_COUNT = 10;
#endif
    static constexpr size_t MAX_MT_ALLOC_SIZE = MAX_ALLOC_SIZE / 128;
    static constexpr size_t MT_TEST_RUN_COUNT = 5;
    // Threads can concurrently add Pools to the allocator, therefore, we must make it into account
    // And also we must take fragmentation into account
    ASSERT_TRUE(mem::MemConfig::GetHeapSizeLimit() >
                2U * (AlignUp(MAX_ELEMENTS_COUNT * MAX_MT_ALLOC_SIZE, DEFAULT_POOL_SIZE_FOR_ALLOC)) +
                    THREADS_COUNT * DEFAULT_POOL_SIZE_FOR_ALLOC);
    for (size_t i = 0; i < MT_TEST_RUN_COUNT; i++) {
        MtAllocCollectTest<FREELIST_ALLOCATOR_MIN_SIZE, MAX_MT_ALLOC_SIZE, THREADS_COUNT>(MIN_ELEMENTS_COUNT,
                                                                                          MAX_ELEMENTS_COUNT);
        ClearPoolManager(true);
    }
}

/*
 * This test checks that `Free` clears padding status bits in MemoryBlockHeader.
 * We allocate 4 consecutive blocks and free the 3rd and 1st ones. After it we
 * allocate block that needs alignment that leads to padding size is saved after
 * block header and PADDING_STATUS_COMMON_HEADER_WITH_PADDING_SIZE bit is set.
 * We free this block and allocator overwrites padding size by pointer to the next
 * free block. Allocator should clear padding status otherwise incorrect padding
 * will be used and we got heap corruption.
 */
TEST_F(FreeListAllocatorTest, AlignTest)
{
    auto memStats = std::unique_ptr<mem::MemStatsType>();
    NonObjectFreeListAllocator allocator(memStats.get());
    AddMemoryPoolToAllocator(allocator);

    // NOLINTNEXTLINE(readability-magic-numbers)
    auto *ptr1 = allocator.Alloc(1678U);
    // NOLINTNEXTLINE(readability-magic-numbers)
    auto *ptr2 = allocator.Alloc(1678U);
    // NOLINTNEXTLINE(readability-magic-numbers)
    auto *ptr3 = allocator.Alloc(1678U);
    // NOLINTNEXTLINE(readability-magic-numbers)
    auto *ptr4 = allocator.Alloc(1678U);

    allocator.Free(ptr3);
    allocator.Free(ptr1);

    // NOLINTNEXTLINE(readability-magic-numbers)
    auto *ptr5 = allocator.Alloc(1536U, GetLogAlignment(128U));
    ASSERT_FALSE(IsAligned(reinterpret_cast<uintptr_t>(ptr3), 128U));
    ASSERT_TRUE(IsAligned(reinterpret_cast<uintptr_t>(ptr5), 128U));
    allocator.Free(ptr5);

    // NOLINTNEXTLINE(readability-magic-numbers)
    auto *ptr6 = allocator.Alloc(1272U);
    // allocations 5 and 6 should use the same memory block. But after free padding bits
    // should be cleared so pointers will be not equal.
    ASSERT_TRUE(ptr5 != ptr6);
    allocator.Free(ptr6);

    allocator.Free(ptr2);
    allocator.Free(ptr4);
}

}  // namespace ark::mem
