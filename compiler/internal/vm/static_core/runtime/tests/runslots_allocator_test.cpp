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

#include "libarkbase/os/mem.h"
#include "libarkbase/utils/logger.h"
#include "runtime/mem/runslots_allocator-inl.h"
#include "runtime/mem/runslots_allocator_stl_adapter.h"
#include "runtime/tests/allocator_test_base.h"

namespace ark::mem {

using NonObjectAllocator = RunSlotsAllocator<EmptyAllocConfigWithCrossingMap>;
using RunSlotsType = RunSlots<>;

class RunSlotsAllocatorTest : public AllocatorTest<NonObjectAllocator> {
public:
    NO_COPY_SEMANTIC(RunSlotsAllocatorTest);
    NO_MOVE_SEMANTIC(RunSlotsAllocatorTest);

    // NOLINTNEXTLINE(modernize-use-equals-default)
    RunSlotsAllocatorTest()
    {
        // Logger::InitializeStdLogging(Logger::Level::DEBUG, Logger::Component::ALL);
    }

    ~RunSlotsAllocatorTest() override
    {
        for (auto i : allocatedMemMmap_) {
            ark::os::mem::UnmapRaw(std::get<0>(i), std::get<1>(i));
        }
        // Logger::Destroy();
    }

protected:
    static constexpr size_t DEFAULT_POOL_SIZE_FOR_ALLOC = NonObjectAllocator::GetMinPoolSize();
    static constexpr size_t DEFAULT_POOL_ALIGNMENT_FOR_ALLOC = RUNSLOTS_ALIGNMENT_IN_BYTES;
    static constexpr Alignment RUNSLOTS_LOG_MAX_ALIGN = LOG_ALIGN_8;

    void AddMemoryPoolToAllocator(NonObjectAllocator &alloc) override
    {
        os::memory::LockHolder lock(poolLock_);
        void *mem = ark::os::mem::MapRWAnonymousRaw(DEFAULT_POOL_SIZE_FOR_ALLOC);
        std::pair<void *, size_t> newPair {mem, DEFAULT_POOL_SIZE_FOR_ALLOC};
        allocatedMemMmap_.push_back(newPair);
        if (!alloc.AddMemoryPool(mem, DEFAULT_POOL_SIZE_FOR_ALLOC)) {
            ASSERT_TRUE(0 && "Can't add mem pool to allocator");
        }
    }

    void AddMemoryPoolToAllocatorProtected(NonObjectAllocator &alloc) override
    {
        os::memory::LockHolder lock(poolLock_);
        void *mem = ark::os::mem::MapRWAnonymousRaw(DEFAULT_POOL_SIZE_FOR_ALLOC + PANDA_PAGE_SIZE);
        mprotect(reinterpret_cast<void *>(reinterpret_cast<uintptr_t>(mem) + DEFAULT_POOL_SIZE_FOR_ALLOC),
                 PANDA_PAGE_SIZE, PROT_NONE);
        std::pair<void *, size_t> newPair {mem, DEFAULT_POOL_SIZE_FOR_ALLOC + PANDA_PAGE_SIZE};
        allocatedMemMmap_.push_back(newPair);
        if (!alloc.AddMemoryPool(mem, DEFAULT_POOL_SIZE_FOR_ALLOC)) {
            ASSERT_TRUE(0 && "Can't add mem pool to allocator");
        }
    }

    void ReleasePages(NonObjectAllocator &alloc)
    {
        alloc.ReleaseEmptyRunSlotsPagesUnsafe();
    }

    bool AllocatedByThisAllocator(NonObjectAllocator &allocator, void *mem) override
    {
        return allocator.AllocatedByRunSlotsAllocator(mem);
    }

    void TestRunSlots(size_t slotsSize)
    {
        LOG(DEBUG, ALLOC) << "Test RunSlots with size " << slotsSize;
        void *mem = aligned_alloc(RUNSLOTS_ALIGNMENT_IN_BYTES, RUNSLOTS_SIZE);
        auto runslots = reinterpret_cast<RunSlotsType *>(mem);
        runslots->Initialize(slotsSize, ToUintPtr(mem), true);
        int i = 0;
        while (runslots->PopFreeSlot() != nullptr) {
            i++;
        }
        // NOLINTNEXTLINE(cppcoreguidelines-no-malloc)
        free(mem);
        LOG(DEBUG, ALLOC) << "Iteration = " << i;
    }

private:
    std::vector<std::pair<void *, size_t>> allocatedMemMmap_;
    // Mutex, which allows only one thread to add pool to the pool vector
    os::memory::Mutex poolLock_;
};

TEST_F(RunSlotsAllocatorTest, SimpleRunSlotsTest)
{
    for (size_t i = RunSlotsType::ConvertToPowerOfTwoUnsafe(RunSlotsType::MinSlotSize());
         i <= RunSlotsType::ConvertToPowerOfTwoUnsafe(RunSlotsType::MaxSlotSize()); i++) {
        TestRunSlots(1U << i);
    }
}

TEST_F(RunSlotsAllocatorTest, SimpleAllocateDifferentObjSizeTest)
{
    LOG(DEBUG, ALLOC) << "SimpleAllocateDifferentObjSizeTest";
    mem::MemStatsType memStats;
    NonObjectAllocator allocator(&memStats);
    AddMemoryPoolToAllocator(allocator);
    // NOLINTNEXTLINE(readability-magic-numbers)
    for (size_t i = 23UL; i < 300UL; i++) {
        void *mem = allocator.Alloc(i);
        (void)mem;
        LOG(DEBUG, ALLOC) << "Allocate obj with size " << i << " at " << std::hex << mem;
    }
}

TEST_F(RunSlotsAllocatorTest, TestReleaseRunSlotsPagesTest)
{
    static constexpr size_t ALLOC_SIZE = RunSlotsType::ConvertToPowerOfTwoUnsafe(RunSlotsType::MinSlotSize());
    LOG(DEBUG, ALLOC) << "TestRunSlotsReusageTestTest";
    mem::MemStatsType memStats;
    NonObjectAllocator allocator(&memStats);
    AddMemoryPoolToAllocator(allocator);
    std::vector<void *> elements;
    // Fill the whole pool
    while (true) {
        void *mem = allocator.Alloc(ALLOC_SIZE);
        if (mem == nullptr) {
            break;
        }
        elements.push_back(mem);
        LOG(DEBUG, ALLOC) << "Allocate obj with size " << ALLOC_SIZE << " at " << std::hex << mem;
    }
    // Free everything except the last element
    ASSERT(elements.size() > 1);
    size_t elementToFreeCount = elements.size() - 1;
    for (size_t i = 0; i < elementToFreeCount; i++) {
        allocator.Free(elements.back());
        elements.pop_back();
    }

    // ReleaseRunSlotsPages
    ReleasePages(allocator);

    // Try to allocate everything again
    for (size_t i = 0; i < elementToFreeCount; i++) {
        void *mem = allocator.Alloc(ALLOC_SIZE);
        ASSERT_TRUE(mem != nullptr);
        elements.push_back(mem);
        LOG(DEBUG, ALLOC) << "Allocate obj with size " << ALLOC_SIZE << " at " << std::hex << mem;
    }

    // Free everything
    for (auto i : elements) {
        allocator.Free(i);
    }
}

TEST_F(RunSlotsAllocatorTest, AllocateAllPossibleSizesFreeTest)
{
    for (size_t i = 1; i <= RunSlotsType::MaxSlotSize(); i++) {
        AllocateAndFree(i, RUNSLOTS_SIZE / i);
    }
}

TEST_F(RunSlotsAllocatorTest, AllocateWriteFreeTest)
{
    // NOLINTNEXTLINE(readability-magic-numbers)
    AllocateAndFree(sizeof(uint64_t), 512UL);
}

TEST_F(RunSlotsAllocatorTest, AllocateRandomFreeTest)
{
    static constexpr size_t ALLOC_SIZE = sizeof(uint64_t);
    static constexpr size_t ELEMENTS_COUNT = 512;
    static constexpr size_t POOLS_COUNT = 1;
    AllocateFreeDifferentSizesTest<ALLOC_SIZE / 2UL, 2UL * ALLOC_SIZE>(ELEMENTS_COUNT, POOLS_COUNT);
}

TEST_F(RunSlotsAllocatorTest, CheckReuseOfRunSlotsTest)
{
    AllocateReuseTest(RUNSLOTS_ALIGNMENT_MASK);
}

TEST_F(RunSlotsAllocatorTest, AllocateTooBigObjTest)
{
    AllocateTooBigObjectTest<RunSlotsType::MaxSlotSize()>();
}

TEST_F(RunSlotsAllocatorTest, AlignmentAllocTest)
{
    AlignedAllocFreeTest<1, RunSlotsType::MaxSlotSize(), LOG_ALIGN_MIN, RUNSLOTS_LOG_MAX_ALIGN>();
}

TEST_F(RunSlotsAllocatorTest, AllocateTooMuchTest)
{
    static constexpr size_t ALLOC_SIZE = sizeof(uint64_t);
    AllocateTooMuchTest(ALLOC_SIZE, DEFAULT_POOL_SIZE_FOR_ALLOC / ALLOC_SIZE);
}

TEST_F(RunSlotsAllocatorTest, AllocateVectorTest)
{
    AllocateVectorTest();
}

TEST_F(RunSlotsAllocatorTest, AllocateReuse2)
{
    // It's regression test
    auto *memStats = new mem::MemStatsType();
    NonObjectAllocator allocator(memStats);
    static constexpr size_t SIZE1 = 60;
    static constexpr size_t SIZE2 = 204;
    constexpr char CHAR1 = 'a';
    constexpr char CHAR2 = 'b';
    constexpr char CHAR3 = 'c';
    constexpr char CHAR4 = 'd';
    constexpr char CHAR5 = 'e';
    constexpr char CHAR6 = 'f';
    AddMemoryPoolToAllocatorProtected(allocator);
    auto fillStr = [](char *str, char c, size_t size) {
        for (size_t i = 0; i < size - 1; i++) {
            // NOLINTNEXTLINE(cppcoreguidelines-pro-bounds-pointer-arithmetic)
            str[i] = c;
        }
        // NOLINTNEXTLINE(cppcoreguidelines-pro-bounds-pointer-arithmetic)
        str[size - 1] = 0;
    };
    auto checkStr = [](const char *str, char c, size_t size) {
        for (size_t i = 0; i < size - 1; i++) {
            // NOLINTNEXTLINE(cppcoreguidelines-pro-bounds-pointer-arithmetic)
            if (str[i] != c) {
                return false;
            }
        }
        return true;
    };
    char *strA = reinterpret_cast<char *>(allocator.Alloc(SIZE1));
    char *strB = reinterpret_cast<char *>(allocator.Alloc(SIZE1));
    char *strC = reinterpret_cast<char *>(allocator.Alloc(SIZE1));
    fillStr(strA, CHAR1, SIZE1);
    fillStr(strB, CHAR2, SIZE1);
    fillStr(strC, CHAR3, SIZE1);
    ASSERT_TRUE(checkStr(strA, CHAR1, SIZE1));
    ASSERT_TRUE(checkStr(strB, CHAR2, SIZE1));
    ASSERT_TRUE(checkStr(strC, CHAR3, SIZE1));
    allocator.Free(static_cast<void *>(strA));
    allocator.Free(static_cast<void *>(strB));
    allocator.Free(static_cast<void *>(strC));
    char *strD = reinterpret_cast<char *>(allocator.Alloc(SIZE2));
    char *strE = reinterpret_cast<char *>(allocator.Alloc(SIZE2));
    char *strF = reinterpret_cast<char *>(allocator.Alloc(SIZE2));
    fillStr(strD, CHAR4, SIZE2);
    fillStr(strE, CHAR5, SIZE2);
    fillStr(strF, CHAR6, SIZE2);
    ASSERT_TRUE(checkStr(strD, CHAR4, SIZE2));
    ASSERT_TRUE(checkStr(strE, CHAR5, SIZE2));
    ASSERT_TRUE(checkStr(strF, CHAR6, SIZE2));
    delete memStats;
}

TEST_F(RunSlotsAllocatorTest, ObjectIteratorTest)
{
    ObjectIteratorTest<1, RunSlotsType::MaxSlotSize(), LOG_ALIGN_MIN, RUNSLOTS_LOG_MAX_ALIGN>();
}

TEST_F(RunSlotsAllocatorTest, ObjectCollectionTest)
{
    ObjectCollectionTest<1, RunSlotsType::MaxSlotSize(), LOG_ALIGN_MIN, RUNSLOTS_LOG_MAX_ALIGN>();
}

TEST_F(RunSlotsAllocatorTest, ObjectIteratorInRangeTest)
{
    ObjectIteratorInRangeTest<1, RunSlotsType::MaxSlotSize(), LOG_ALIGN_MIN, RUNSLOTS_LOG_MAX_ALIGN>(
        CrossingMapSingleton::GetCrossingMapGranularity());
}

TEST_F(RunSlotsAllocatorTest, AsanTest)
{
    AsanTest();
}

TEST_F(RunSlotsAllocatorTest, VisitAndRemoveFreePoolsTest)
{
    static constexpr size_t POOLS_COUNT = 5;
    VisitAndRemoveFreePools<POOLS_COUNT>(RunSlotsType::MaxSlotSize());
}

TEST_F(RunSlotsAllocatorTest, AllocatedByRunSlotsAllocatorTest)
{
    AllocatedByThisAllocatorTest();
}

TEST_F(RunSlotsAllocatorTest, RunSlotsReusingTest)
{
    static constexpr size_t SMALL_OBJ_SIZE = sizeof(uint32_t);
    static constexpr size_t BIG_OBJ_SIZE = 128;
    auto *memStats = new mem::MemStatsType();
    NonObjectAllocator allocator(memStats);
    AddMemoryPoolToAllocatorProtected(allocator);
    // Alloc one big object. this must cause runslots init with it size
    void *mem = allocator.Alloc(BIG_OBJ_SIZE);
    // Free this object
    allocator.Free(mem);

    // Alloc small object. We must reuse already allocated and freed RunSlots
    void *smallObjMem = allocator.Alloc(SMALL_OBJ_SIZE);
    size_t smallObjIndex = SetBytesFromByteArray(smallObjMem, SMALL_OBJ_SIZE);

    // Alloc big obj again.
    void *bigObjMem = allocator.Alloc(BIG_OBJ_SIZE);
    size_t bigObjIndex = SetBytesFromByteArray(bigObjMem, BIG_OBJ_SIZE);

    // Alloc one more small object.
    void *secondSmallObjMem = allocator.Alloc(SMALL_OBJ_SIZE);
    size_t secondSmallObjIndex = SetBytesFromByteArray(secondSmallObjMem, SMALL_OBJ_SIZE);

    ASSERT_TRUE(CompareBytesWithByteArray(bigObjMem, BIG_OBJ_SIZE, bigObjIndex));
    ASSERT_TRUE(CompareBytesWithByteArray(smallObjMem, SMALL_OBJ_SIZE, smallObjIndex));
    ASSERT_TRUE(CompareBytesWithByteArray(secondSmallObjMem, SMALL_OBJ_SIZE, secondSmallObjIndex));
    delete memStats;
}

TEST_F(RunSlotsAllocatorTest, MTAllocFreeTest)
{
    static constexpr size_t MIN_ELEMENTS_COUNT = 1500;
    static constexpr size_t MAX_ELEMENTS_COUNT = 3000;
#if defined(PANDA_TARGET_ARM64) || defined(PANDA_TARGET_32)
    // We have an issue with QEMU during MT tests. Issue 2852
    static constexpr size_t THREADS_COUNT = 1;
#else
    static constexpr size_t THREADS_COUNT = 10;
#endif
    static constexpr size_t MT_TEST_RUN_COUNT = 5;
    for (size_t i = 0; i < MT_TEST_RUN_COUNT; i++) {
        MtAllocFreeTest<1, RunSlotsType::MaxSlotSize(), THREADS_COUNT>(MIN_ELEMENTS_COUNT, MAX_ELEMENTS_COUNT);
    }
}

TEST_F(RunSlotsAllocatorTest, MTAllocIterateTest)
{
    static constexpr size_t MIN_ELEMENTS_COUNT = 1500;
    static constexpr size_t MAX_ELEMENTS_COUNT = 3000;
#if defined(PANDA_TARGET_ARM64) || defined(PANDA_TARGET_32)
    // We have an issue with QEMU during MT tests. Issue 2852
    static constexpr size_t THREADS_COUNT = 1;
#else
    static constexpr size_t THREADS_COUNT = 10;
#endif
    static constexpr size_t MT_TEST_RUN_COUNT = 5;
    for (size_t i = 0; i < MT_TEST_RUN_COUNT; i++) {
        MtAllocIterateTest<1, RunSlotsType::MaxSlotSize(), THREADS_COUNT>(
            MIN_ELEMENTS_COUNT, MAX_ELEMENTS_COUNT, CrossingMapSingleton::GetCrossingMapGranularity());
    }
}

TEST_F(RunSlotsAllocatorTest, MTAllocCollectTest)
{
    static constexpr size_t MIN_ELEMENTS_COUNT = 1500;
    static constexpr size_t MAX_ELEMENTS_COUNT = 3000;
#if defined(PANDA_TARGET_ARM64) || defined(PANDA_TARGET_32)
    // We have an issue with QEMU during MT tests. Issue 2852
    static constexpr size_t THREADS_COUNT = 1;
#else
    static constexpr size_t THREADS_COUNT = 10;
#endif
    static constexpr size_t MT_TEST_RUN_COUNT = 5;
    for (size_t i = 0; i < MT_TEST_RUN_COUNT; i++) {
        MtAllocCollectTest<1, RunSlotsType::MaxSlotSize(), THREADS_COUNT>(MIN_ELEMENTS_COUNT, MAX_ELEMENTS_COUNT);
    }
}

}  // namespace ark::mem
