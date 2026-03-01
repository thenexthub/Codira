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

#include <array>
#include <atomic>
#include <chrono>
#include <condition_variable>
#include <thread>

#include "gtest/gtest.h"
#include "iostream"
#include "runtime/include/coretypes/string.h"
#include "runtime/include/runtime.h"
#include "runtime/include/panda_vm.h"
#include "runtime/mem/malloc-proxy-allocator-inl.h"
#include "runtime/mem/mem_stats.h"
#include "runtime/mem/mem_stats_default.h"
#include "runtime/mem/runslots_allocator-inl.h"

namespace ark::mem::test {

#ifndef PANDA_NIGHTLY_TEST_ON
constexpr uint64_t ITERATION = 256;
constexpr size_t NUM_THREADS = 2;
#else
constexpr uint64_t ITERATION = 1 << 17;
constexpr size_t NUM_THREADS = 8;
#endif

using NonObjectAllocator = RunSlotsAllocator<RawMemoryConfig>;

class MemStatsTest : public testing::Test {
public:
    MemStatsTest()
    {
        // we need runtime for creating objects
        RuntimeOptions options;
        options.SetShouldLoadBootPandaFiles(false);
        options.SetShouldInitializeIntrinsics(false);
        options.SetGcType("stw");
        options.SetRunGcInPlace(true);
        Runtime::Create(options);
        thread_ = ark::MTManagedThread::GetCurrent();
        thread_->ManagedCodeBegin();
    }

    ~MemStatsTest() override
    {
        thread_->ManagedCodeEnd();
        Runtime::Destroy();
    }

    NO_COPY_SEMANTIC(MemStatsTest);
    NO_MOVE_SEMANTIC(MemStatsTest);

protected:
    // NOLINTNEXTLINE(misc-non-private-member-variables-in-classes)
    ark::MTManagedThread *thread_ {};
};

using MallocProxyNonObjectAllocator = MallocProxyAllocator<RawMemoryConfig>;

class RawStatsBeforeTest {
    size_t rawBytesAllocatedBeforeTest_;
    size_t rawBytesFreedBeforeTest_;
    size_t rawBytesFootprintBeforeRest_;

public:
    explicit RawStatsBeforeTest(MemStatsType *stats)
        : rawBytesAllocatedBeforeTest_(stats->GetAllocated(SpaceType::SPACE_TYPE_INTERNAL)),
          rawBytesFreedBeforeTest_(stats->GetFreed(SpaceType::SPACE_TYPE_INTERNAL)),
          rawBytesFootprintBeforeRest_(stats->GetFootprint(SpaceType::SPACE_TYPE_INTERNAL))
    {
    }

    [[nodiscard]] size_t GetRawBytesAllocatedBeforeTest() const
    {
        return rawBytesAllocatedBeforeTest_;
    }

    [[nodiscard]] size_t GetRawBytesFreedBeforeTest() const
    {
        return rawBytesFreedBeforeTest_;
    }

    [[nodiscard]] size_t GetRawBytesFootprintBeforeTest() const
    {
        return rawBytesFootprintBeforeRest_;
    }
};

void AssertHeapStats(MemStatsType *stats, size_t bytesInHeap, size_t heapBytesAllocated, size_t heapBytesFreed)
{
    ASSERT_EQ(heapBytesAllocated, stats->GetAllocated(SpaceType::SPACE_TYPE_OBJECT));
    ASSERT_EQ(heapBytesFreed, stats->GetFreed(SpaceType::SPACE_TYPE_OBJECT));
    ASSERT_EQ(bytesInHeap, stats->GetFootprint(SpaceType::SPACE_TYPE_OBJECT));
}

void AssertHeapHumongousStats(MemStatsType *stats, size_t bytesInHeap, size_t heapBytesAllocated, size_t heapBytesFreed)
{
    ASSERT_EQ(heapBytesAllocated, stats->GetAllocated(SpaceType::SPACE_TYPE_HUMONGOUS_OBJECT));
    ASSERT_EQ(heapBytesFreed, stats->GetFreed(SpaceType::SPACE_TYPE_HUMONGOUS_OBJECT));
    ASSERT_EQ(bytesInHeap, stats->GetFootprint(SpaceType::SPACE_TYPE_HUMONGOUS_OBJECT));
}

void AssertHeapObjectsStats(MemStatsType *stats, size_t heapObjectsAllocated, size_t heapObjectsFreed,
                            size_t heapHumungousObjectsAllocated, size_t heapHumungousObjectsFreed)
{
    ASSERT_EQ(heapObjectsAllocated, stats->GetTotalObjectsAllocated());
    ASSERT_EQ(heapObjectsFreed, stats->GetTotalObjectsFreed());

    // On arm-32 platform, we should cast the uint64_t(-1) to size_t(-1)
    ASSERT_EQ(heapObjectsAllocated - heapHumungousObjectsAllocated,
              static_cast<size_t>(stats->GetTotalRegularObjectsAllocated()));
    ASSERT_EQ(heapObjectsFreed - heapHumungousObjectsFreed, static_cast<size_t>(stats->GetTotalRegularObjectsFreed()));

    ASSERT_EQ(heapHumungousObjectsAllocated, stats->GetTotalHumongousObjectsAllocated());
    ASSERT_EQ(heapHumungousObjectsFreed, stats->GetTotalHumongousObjectsFreed());

    ASSERT_EQ(heapObjectsAllocated - heapObjectsFreed, stats->GetObjectsCountAlive());
    ASSERT_EQ(heapObjectsAllocated - heapObjectsFreed + heapHumungousObjectsAllocated - heapHumungousObjectsFreed,
              stats->GetRegularObjectsCountAlive());
    ASSERT_EQ(heapHumungousObjectsAllocated - heapHumungousObjectsFreed, stats->GetHumonguousObjectsCountAlive());
}

/**
 * We add bytes which we allocated before tests for our internal structures, but we don't add it to `freed` because
 * destructors haven't be called yet.
 */
void AssertRawStats(MemStatsType *stats, size_t rawBytesAllocated, size_t rawBytesFreed, size_t rawBytesFootprint,
                    RawStatsBeforeTest &statsBeforeTest)
{
    ASSERT_EQ(rawBytesAllocated + statsBeforeTest.GetRawBytesAllocatedBeforeTest(),
              stats->GetAllocated(SpaceType::SPACE_TYPE_INTERNAL));
    ASSERT_EQ(rawBytesFreed + statsBeforeTest.GetRawBytesFreedBeforeTest(),
              stats->GetFreed(SpaceType::SPACE_TYPE_INTERNAL));
    ASSERT_EQ(rawBytesFootprint + statsBeforeTest.GetRawBytesFootprintBeforeTest(),
              stats->GetFootprint(SpaceType::SPACE_TYPE_INTERNAL));
}

TEST_F(MemStatsTest, SimpleTest)
{
    static constexpr size_t BYTES_OBJECT1 = 10;
    static constexpr size_t BYTES_OBJECT2 = 12;
    static constexpr size_t BYTES_RAW_MEMORY_ALLOC1 = 20;
    static constexpr size_t BYTES_RAW_MEMORY_ALLOC2 = 30002;
    static constexpr size_t RAW_MEMORY_FREED = 5;

    auto *stats = thread_->GetVM()->GetMemStats();
    size_t initHeapBytes = stats->GetAllocated(SpaceType::SPACE_TYPE_OBJECT);
    size_t initHeapObjects = stats->GetTotalObjectsAllocated();
    RawStatsBeforeTest rawStatsBeforeTest(stats);
    stats->RecordAllocateObject(BYTES_OBJECT1, SpaceType::SPACE_TYPE_OBJECT);
    stats->RecordAllocateObject(BYTES_OBJECT2, SpaceType::SPACE_TYPE_OBJECT);
    stats->RecordAllocateRaw(BYTES_RAW_MEMORY_ALLOC1, SpaceType::SPACE_TYPE_INTERNAL);
    stats->RecordAllocateRaw(BYTES_RAW_MEMORY_ALLOC2, SpaceType::SPACE_TYPE_INTERNAL);
    stats->RecordFreeRaw(RAW_MEMORY_FREED, SpaceType::SPACE_TYPE_INTERNAL);

    AssertHeapStats(stats, initHeapBytes + BYTES_OBJECT1 + BYTES_OBJECT2, initHeapBytes + BYTES_OBJECT1 + BYTES_OBJECT2,
                    0);
    AssertHeapObjectsStats(stats, initHeapObjects + 2U, 0, 0, 0);
    ASSERT_EQ(initHeapBytes + BYTES_OBJECT1 + BYTES_OBJECT2, stats->GetFootprint(SpaceType::SPACE_TYPE_OBJECT));
    AssertRawStats(stats, BYTES_RAW_MEMORY_ALLOC1 + BYTES_RAW_MEMORY_ALLOC2, RAW_MEMORY_FREED,
                   BYTES_RAW_MEMORY_ALLOC1 + BYTES_RAW_MEMORY_ALLOC2 - RAW_MEMORY_FREED, rawStatsBeforeTest);
    stats->RecordFreeRaw(BYTES_RAW_MEMORY_ALLOC1 + BYTES_RAW_MEMORY_ALLOC2 - RAW_MEMORY_FREED,
                         SpaceType::SPACE_TYPE_INTERNAL);
}

// testing MemStats via allocators.
TEST_F(MemStatsTest, NonObjectTestViaMallocAllocator)
{
    static constexpr size_t BYTES_ALLOC1 = 23;
    static constexpr size_t BYTES_ALLOC2 = 42;

    mem::MemStatsType *stats = thread_->GetVM()->GetMemStats();
    RawStatsBeforeTest rawStatsBeforeTest(stats);
    size_t initHeapBytes = stats->GetAllocated(SpaceType::SPACE_TYPE_OBJECT);
    size_t initHeapObjects = stats->GetTotalObjectsAllocated();
    MallocProxyNonObjectAllocator allocator(stats, SpaceType::SPACE_TYPE_INTERNAL);

    void *a1 = allocator.Alloc(BYTES_ALLOC1);
    allocator.Free(a1);
    void *a2 = allocator.Alloc(BYTES_ALLOC2);

    AssertHeapStats(stats, initHeapBytes, initHeapBytes, 0);
    AssertHeapObjectsStats(stats, initHeapObjects, 0, 0, 0);
    AssertRawStats(stats, BYTES_ALLOC1 + BYTES_ALLOC2, BYTES_ALLOC1, BYTES_ALLOC2, rawStatsBeforeTest);
    allocator.Free(a2);
}

// testing MemStats via allocators.
TEST_F(MemStatsTest, NonObjectTestViaSlotsAllocator)
{
    static constexpr uint64_t POOL_SIZE = 4_MB;
    static constexpr size_t REAL_BYTES_ALLOC1 = 23;
    // RunSlotsAllocator uses 32 bytes for allocation 23 bytes
    static constexpr size_t BYTES_IN_ALLOCATOR_ALLOC1 = 32;
    static constexpr size_t REAL_BYTES_ALLOC2 = 42;
    static constexpr size_t BYTES_IN_ALLOCATOR_ALLOC2 = 64;

    mem::MemStatsType *stats = thread_->GetVM()->GetMemStats();
    size_t initHeapBytes = stats->GetAllocated(SpaceType::SPACE_TYPE_OBJECT);
    size_t initHeapObjects = stats->GetTotalObjectsAllocated();
    RawStatsBeforeTest rawStatsBeforeTest(stats);

    auto *allocator = new NonObjectAllocator(stats, SpaceType::SPACE_TYPE_INTERNAL);
    void *mem = aligned_alloc(RUNSLOTS_ALIGNMENT_IN_BYTES, POOL_SIZE);
    allocator->AddMemoryPool(mem, POOL_SIZE);

    void *a1 = allocator->Alloc(REAL_BYTES_ALLOC1);
    allocator->Free(a1);
    void *a2 = allocator->Alloc(REAL_BYTES_ALLOC2);

    AssertHeapStats(stats, initHeapBytes, initHeapBytes, 0);
    AssertHeapObjectsStats(stats, initHeapObjects, 0, 0, 0);
    AssertRawStats(stats, BYTES_IN_ALLOCATOR_ALLOC1 + BYTES_IN_ALLOCATOR_ALLOC2, BYTES_IN_ALLOCATOR_ALLOC1,
                   BYTES_IN_ALLOCATOR_ALLOC2, rawStatsBeforeTest);
    allocator->Free(a2);
    delete allocator;
    // NOLINTNEXTLINE(cppcoreguidelines-no-malloc)
    std::free(mem);
}

// allocate and free small object in the heap
TEST_F(MemStatsTest, SmallObject)
{
    mem::MemStatsType *stats = thread_->GetVM()->GetMemStats();
    size_t initHeapBytes = stats->GetAllocated(SpaceType::SPACE_TYPE_OBJECT);
    size_t initHeapObjects = stats->GetTotalObjectsAllocated();
    RawStatsBeforeTest rawStatsBeforeTest(stats);
    std::string simpleString = "abcdef12345";
    LanguageContext ctx = Runtime::GetCurrent()->GetLanguageContext(panda_file::SourceLang::PANDA_ASSEMBLY);
    [[maybe_unused]] coretypes::String *stringObject =
        coretypes::String::CreateFromMUtf8(reinterpret_cast<const uint8_t *>(&simpleString[0]), simpleString.length(),
                                           ctx, Runtime::GetCurrent()->GetPandaVM());
    ASSERT_TRUE(stringObject != nullptr);
    thread_->GetVM()->GetGC()->WaitForGCInManaged(GCTask(GCTaskCause::EXPLICIT_CAUSE));
    size_t allocSize = simpleString.size() + sizeof(coretypes::String);
    size_t aligmentSize = 1UL << RunSlots<>::ConvertToPowerOfTwoUnsafe(allocSize);
    AssertHeapStats(stats, initHeapBytes, initHeapBytes + aligmentSize, aligmentSize);
    AssertHeapObjectsStats(stats, initHeapObjects + 1, 1, 0, 0);
    ASSERT_EQ(rawStatsBeforeTest.GetRawBytesFootprintBeforeTest(), stats->GetFootprint(SpaceType::SPACE_TYPE_INTERNAL));
}

// allocate and free big object in the heap
TEST_F(MemStatsTest, BigObject)
{
    mem::MemStatsType *stats = thread_->GetVM()->GetMemStats();
    RawStatsBeforeTest rawStatsBeforeTest(stats);
    size_t initHeapBytes = stats->GetAllocated(SpaceType::SPACE_TYPE_OBJECT);
    size_t initHeapObjects = stats->GetTotalObjectsAllocated();
    std::string simpleString;
    auto objectAllocator = thread_->GetVM()->GetGC()->GetObjectAllocator();
    size_t allocSize = objectAllocator->GetRegularObjectMaxSize() + 1;
    for (size_t j = 0; j < allocSize; j++) {
        simpleString.append("x");
    }
    LanguageContext ctx = Runtime::GetCurrent()->GetLanguageContext(panda_file::SourceLang::PANDA_ASSEMBLY);
    [[maybe_unused]] coretypes::String *stringObject =
        coretypes::String::CreateFromMUtf8(reinterpret_cast<const uint8_t *>(&simpleString[0]), simpleString.length(),
                                           ctx, Runtime::GetCurrent()->GetPandaVM());
    ASSERT_TRUE(stringObject != nullptr);
    thread_->GetVM()->GetGC()->WaitForGCInManaged(GCTask(GCTaskCause::EXPLICIT_CAUSE));
    allocSize += sizeof(coretypes::String);
    size_t aligmentSize = AlignUp(allocSize, GetAlignmentInBytes(FREELIST_DEFAULT_ALIGNMENT));

    AssertHeapStats(stats, initHeapBytes, initHeapBytes + aligmentSize, aligmentSize);
    AssertHeapObjectsStats(stats, initHeapObjects + 1, 1, 0, 0);
    ASSERT_EQ(rawStatsBeforeTest.GetRawBytesFootprintBeforeTest(), stats->GetFootprint(SpaceType::SPACE_TYPE_INTERNAL));
}

// allocate and free humongous object in the heap
TEST_F(MemStatsTest, HumongousObject)
{
    mem::MemStatsType *stats = thread_->GetVM()->GetMemStats();
    RawStatsBeforeTest rawStatsBeforeTest(stats);
    size_t initHeapBytes = stats->GetAllocated(SpaceType::SPACE_TYPE_HUMONGOUS_OBJECT);
    size_t initHeapObjects = stats->GetTotalObjectsAllocated();
    std::string simpleString;
    auto objectAllocator = thread_->GetVM()->GetGC()->GetObjectAllocator();
    size_t allocSize = objectAllocator->GetLargeObjectMaxSize() + 1;
    for (size_t j = 0; j < allocSize; j++) {
        simpleString.append("x");
    }
    LanguageContext ctx = Runtime::GetCurrent()->GetLanguageContext(panda_file::SourceLang::PANDA_ASSEMBLY);
    [[maybe_unused]] coretypes::String *stringObject =
        coretypes::String::CreateFromMUtf8(reinterpret_cast<const uint8_t *>(&simpleString[0]), simpleString.length(),
                                           ctx, Runtime::GetCurrent()->GetPandaVM());
    ASSERT_TRUE(stringObject != nullptr);
    thread_->GetVM()->GetGC()->WaitForGCInManaged(GCTask(GCTaskCause::EXPLICIT_CAUSE));
    // NOLINTNEXTLINE(readability-magic-numbers)
    AssertHeapHumongousStats(stats, initHeapBytes, initHeapBytes + 2359296U, 2359296U);
    AssertHeapObjectsStats(stats, initHeapObjects, 0, 1, 1);
    ASSERT_EQ(rawStatsBeforeTest.GetRawBytesFootprintBeforeTest(), stats->GetFootprint(SpaceType::SPACE_TYPE_INTERNAL));

    ASSERT_EQ(2359296UL, stats->GetAllocated(SpaceType::SPACE_TYPE_HUMONGOUS_OBJECT));
    ASSERT_EQ(2359296UL, stats->GetFreed(SpaceType::SPACE_TYPE_HUMONGOUS_OBJECT));
    ASSERT_EQ(0, stats->GetFootprint(SpaceType::SPACE_TYPE_HUMONGOUS_OBJECT));
}

TEST_F(MemStatsTest, TotalFootprint)
{
    static constexpr size_t BYTES_ALLOC1 = 2;
    static constexpr size_t BYTES_ALLOC2 = 5;
    static constexpr size_t RAW_ALLOC1 = 15;
    static constexpr size_t RAW_ALLOC2 = 30;

    MemStatsDefault stats;
    stats.RecordAllocateObject(BYTES_ALLOC1, SpaceType::SPACE_TYPE_OBJECT);
    stats.RecordAllocateObject(BYTES_ALLOC2, SpaceType::SPACE_TYPE_OBJECT);
    stats.RecordAllocateRaw(RAW_ALLOC1, SpaceType::SPACE_TYPE_INTERNAL);
    stats.RecordAllocateRaw(RAW_ALLOC2, SpaceType::SPACE_TYPE_INTERNAL);

    ASSERT_EQ(BYTES_ALLOC1 + BYTES_ALLOC2, stats.GetFootprint(SpaceType::SPACE_TYPE_OBJECT));
    ASSERT_EQ(BYTES_ALLOC1 + BYTES_ALLOC2 + RAW_ALLOC1 + RAW_ALLOC2, stats.GetTotalFootprint());
    ASSERT_EQ(RAW_ALLOC1 + RAW_ALLOC2, stats.GetFootprint(SpaceType::SPACE_TYPE_INTERNAL));

    stats.RecordFreeRaw(RAW_ALLOC1, SpaceType::SPACE_TYPE_INTERNAL);

    ASSERT_EQ(BYTES_ALLOC1 + BYTES_ALLOC2, stats.GetFootprint(SpaceType::SPACE_TYPE_OBJECT));
    ASSERT_EQ(BYTES_ALLOC1 + BYTES_ALLOC2 + RAW_ALLOC2, stats.GetTotalFootprint());
    ASSERT_EQ(RAW_ALLOC2, stats.GetFootprint(SpaceType::SPACE_TYPE_INTERNAL));
}

TEST_F(MemStatsTest, Statistics)
{
    static constexpr size_t BYTES_OBJECT = 10;
    static constexpr size_t BYTES_ALLOC1 = 23;
    static constexpr size_t BYTES_ALLOC2 = 42;

    MemStatsDefault stats;
    stats.RecordAllocateObject(BYTES_OBJECT, SpaceType::SPACE_TYPE_OBJECT);
    stats.RecordAllocateRaw(BYTES_ALLOC1, SpaceType::SPACE_TYPE_INTERNAL);
    stats.RecordAllocateRaw(BYTES_ALLOC2, SpaceType::SPACE_TYPE_INTERNAL);

    auto statistics = stats.GetStatistics();
    ASSERT_TRUE(statistics.find(std::to_string(BYTES_OBJECT)) != std::string::npos);
    ASSERT_TRUE(statistics.find(std::to_string(BYTES_ALLOC1 + BYTES_ALLOC2)) != std::string::npos);
    stats.RecordFreeRaw(BYTES_ALLOC1 + BYTES_ALLOC2, SpaceType::SPACE_TYPE_INTERNAL);
}

void FillMemStatsForConcurrency(MemStatsDefault &stats, std::condition_variable &readyToStart, std::mutex &cvMutex,
                                std::atomic_size_t &threadsReady, coretypes::String *stringObject)
{
    {
        std::unique_lock<std::mutex> lockForReadyToStart(cvMutex);
        // Atomic with seq_cst order reason: data race with threads_ready with requirement for sequentially consistent
        // order where threads observe all modifications in the same order
        threadsReady.fetch_add(1, std::memory_order_seq_cst);
        // Atomic with seq_cst order reason: data race with threads_ready with requirement for sequentially consistent
        // order where threads observe all modifications in the same order
        if (threadsReady.load(std::memory_order_seq_cst) == NUM_THREADS) {
            // Unlock all threads
            readyToStart.notify_all();
        } else {
            // Atomic with seq_cst order reason: data race with threads_ready with requirement for sequentially
            // consistent order where threads observe all modifications in the same order
            readyToStart.wait(lockForReadyToStart,
                              [&threadsReady] { return threadsReady.load(std::memory_order_seq_cst) == NUM_THREADS; });
        }
    }
    for (size_t i = 1; i <= ITERATION; i++) {
        for (size_t index = 0; index < SPACE_TYPE_SIZE; index++) {
            SpaceType type = ToSpaceType(index);
            if (IsHeapSpace(type)) {
                stats.RecordAllocateObject(stringObject->ObjectSize(), type);
            } else {
                stats.RecordAllocateRaw(i * (index + 1), type);
            }
        }
    }

    for (size_t index = 0; index < SPACE_TYPE_SIZE; index++) {
        SpaceType type = ToSpaceType(index);
        if (IsHeapSpace(type)) {
            stats.RecordFreeObject(stringObject->ObjectSize(), type);
        } else {
            stats.RecordFreeRaw(ITERATION * (index + 1), type);
        }
    }
}

TEST_F(MemStatsTest, TestThreadSafety)
{
    std::string simpleString = "smallData";
    LanguageContext ctx = Runtime::GetCurrent()->GetLanguageContext(panda_file::SourceLang::PANDA_ASSEMBLY);
    coretypes::String *stringObject =
        coretypes::String::CreateFromMUtf8(reinterpret_cast<const uint8_t *>(&simpleString[0]), simpleString.length(),
                                           ctx, Runtime::GetCurrent()->GetPandaVM());

    MemStatsDefault stats;

    std::array<std::thread, NUM_THREADS> threads;

    std::atomic_size_t threadsReady = 0;
    std::mutex cvMutex;
    std::condition_variable readyToStart;
    for (size_t i = 0; i < NUM_THREADS; i++) {
        threads[i] = std::thread(FillMemStatsForConcurrency, std::ref(stats), std::ref(readyToStart), std::ref(cvMutex),
                                 std::ref(threadsReady), stringObject);
    }

    for (size_t i = 0; i < NUM_THREADS; i++) {
        threads[i].join();
    }

    constexpr uint64_t SUM = (ITERATION + 1) * ITERATION / 2U;
    constexpr uint64_t TOTAL_ITERATION_COUNT = NUM_THREADS * ITERATION;

    for (size_t index = 0; index < SPACE_TYPE_SIZE; index++) {
        SpaceType type = ToSpaceType(index);
        if (IsHeapSpace(type)) {
            ASSERT_EQ(stats.GetAllocated(type), TOTAL_ITERATION_COUNT * stringObject->ObjectSize());
            ASSERT_EQ(stats.GetFreed(type), NUM_THREADS * stringObject->ObjectSize());
            ASSERT_EQ(stats.GetFootprint(type), (TOTAL_ITERATION_COUNT - NUM_THREADS) * stringObject->ObjectSize());
        } else {
            ASSERT_EQ(stats.GetAllocated(type), SUM * NUM_THREADS * (index + 1));
            ASSERT_EQ(stats.GetFreed(type), TOTAL_ITERATION_COUNT * (index + 1));
            ASSERT_EQ(stats.GetFootprint(type), (SUM - ITERATION) * NUM_THREADS * (index + 1));
        }
    }
}

}  // namespace ark::mem::test
