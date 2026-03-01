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
#ifndef PANDA_RUNTIME_TESTS_ALLOCATOR_TEST_BASE_H_
#define PANDA_RUNTIME_TESTS_ALLOCATOR_TEST_BASE_H_

#include <gtest/gtest.h>

#include <algorithm>
#include <array>
#include <cstdlib>
#include <cstring>
#include <ctime>
#include <tuple>
#include <unordered_set>

#include "libarkbase/mem/mem.h"
#include "libarkbase/os/thread.h"
#include "libarkbase/utils/utils.h"
#include "runtime/mem/bump-allocator.h"
#include "runtime/mem/mem_stats_additional_info.h"
#include "runtime/mem/mem_stats_default.h"
#include "runtime/include/object_header.h"

namespace ark::mem {

template <class Allocator>
class AllocatorTest : public testing::Test {
public:
    explicit AllocatorTest()
    {
#ifdef PANDA_NIGHTLY_TEST_ON
        seed_ = std::time(NULL);
#else
        static constexpr unsigned int FIXED_SEED = 0xDEADBEEF;
        seed_ = FIXED_SEED;
#endif
        srand(seed_);
        InitByteArray();
    }

protected:
    static constexpr size_t BYTE_ARRAY_SIZE = 1000;

    unsigned int seed_;                                  // NOLINT(misc-non-private-member-variables-in-classes)
    std::array<uint8_t, BYTE_ARRAY_SIZE> byteArray_ {};  // NOLINT(misc-non-private-member-variables-in-classes)

    /// Byte array initialization of random bytes
    void InitByteArray()
    {
        for (size_t i = 0; i < BYTE_ARRAY_SIZE; ++i) {
            byteArray_[i] = RandFromRange(0, std::numeric_limits<uint8_t>::max());
        }
    }

    /**
     * @brief Add pool to allocator (maybe empty for some allocators)
     * @param allocator - allocator for pool memory adding
     */
    virtual void AddMemoryPoolToAllocator([[maybe_unused]] Allocator &allocator) = 0;

    /**
     * @brief Add pool to allocator and protect (maybe empty for some allocators)
     * @param allocator - allocator for pool memory addition and protection
     */
    virtual void AddMemoryPoolToAllocatorProtected([[maybe_unused]] Allocator &allocator) = 0;

    /**
     * @brief Check to allocated by this allocator
     * @param allocator - allocator
     * @param mem - allocated memory
     */
    virtual bool AllocatedByThisAllocator([[maybe_unused]] Allocator &allocator, [[maybe_unused]] void *mem) = 0;

    /**
     * @brief Generate random value from [min_value, max_value]
     * @param min_value - minimum size_t value in range
     * @param max_value - maximum size_t value in range
     * @return random size_t value [min_value, max_value]
     */
    size_t RandFromRange(size_t minValue, size_t maxValue)
    {
        // rand() is not thread-safe method.
        // So do it under the lock
        static os::memory::Mutex randLock;
        os::memory::LockHolder lock(randLock);
        // NOLINTNEXTLINE(cert-msc50-cpp)
        return minValue + rand() % (maxValue - minValue + 1);
    }

    /**
     * @brief Write value in memory for death test
     * @param mem - memory for writing
     *
     * Write value in memory for address sanitizer test
     */
    void DeathWriteUint64(void *mem)
    {
        static constexpr uint64_t INVALID_ADDR = 0xDEADBEEF;
        *(static_cast<uint64_t *>(mem)) = INVALID_ADDR;
    }

    /**
     * @brief Set random bytes in memory from byte array
     * @param mem - memory for random bytes from byte array writing
     * @param size - size memory in bytes
     * @return start index in byte_array
     */
    size_t SetBytesFromByteArray(void *mem, size_t size)
    {
        size_t startIndex = RandFromRange(0, BYTE_ARRAY_SIZE - 1);
        size_t copied = 0;
        size_t firstCopySize = std::min(size, BYTE_ARRAY_SIZE - startIndex);
        // Set head of memory
        MemcpyUnsafe(mem, &byteArray_[startIndex], firstCopySize);
        size -= firstCopySize;
        copied += firstCopySize;
        // Set middle part of memory
        while (size > BYTE_ARRAY_SIZE) {
            MemcpyUnsafe(ToVoidPtr(ToUintPtr(mem) + copied), byteArray_.data(), BYTE_ARRAY_SIZE);
            size -= BYTE_ARRAY_SIZE;
            copied += BYTE_ARRAY_SIZE;
        }
        // Set tail of memory
        MemcpyUnsafe(ToVoidPtr(ToUintPtr(mem) + copied), byteArray_.data(), size);

        return startIndex;
    }

    /**
     * @brief Compare bytes in memory with byte array
     * @param mem - memory for random bytes from byte array writing
     * @param size - size memory in bytes
     * @param start_index_in_byte_array - start index in byte array for comaration with memory
     * @return boolean value: true if bytes are equal and fasle if not equal
     */
    bool CompareBytesWithByteArray(void *mem, size_t size, size_t startIndexInByteArray)
    {
        size_t compared = 0;
        size_t firstCompareSize = std::min(size, BYTE_ARRAY_SIZE - startIndexInByteArray);
        // Compare head of memory
        if (memcmp(mem, &byteArray_[startIndexInByteArray], firstCompareSize) != 0) {
            return false;
        }
        compared += firstCompareSize;
        size -= firstCompareSize;
        // Compare middle part of memory
        while (size >= BYTE_ARRAY_SIZE) {
            if (memcmp(ToVoidPtr(ToUintPtr(mem) + compared), byteArray_.data(), BYTE_ARRAY_SIZE) != 0) {
                return false;
            }
            size -= BYTE_ARRAY_SIZE;
            compared += BYTE_ARRAY_SIZE;
        }
        // Compare tail of memory
        return memcmp(ToVoidPtr(ToUintPtr(mem) + compared), byteArray_.data(), size) == 0;
    }

    /**
     * @brief Allocate with one alignment
     * @tparam MIN_ALLOC_SIZE - minimum possible size for one allocation
     * @tparam MAX_ALLOC_SIZE - maximum possible size for one allocation
     * @tparam ALIGNMENT - enum Alignment value for allocations
     * @tparam AllocatorArgs - arguments types for allocor creation
     * @param pools_count - count of pools needed by allocation
     * @param allocator_args - arguments for allocator creation
     *
     * Allocate all possible sizes from [MIN_ALLOC_SIZE, MAX_ALLOC_SIZE] with ALIGNMENT alignment
     */
    template <size_t MIN_ALLOC_SIZE, size_t MAX_ALLOC_SIZE, Alignment ALIGNMENT, class... AllocatorArgs>
    void OneAlignedAllocFreeTest(size_t poolsCount, AllocatorArgs &&...allocatorArgs)
    {
        static constexpr size_t ALLOCATIONS_COUNT = MAX_ALLOC_SIZE - MIN_ALLOC_SIZE + 1;

        auto *memStats = new mem::MemStatsType();
        Allocator allocator(memStats, std::forward<AllocatorArgs>(allocatorArgs)...);
        for (size_t i = 0; i < poolsCount; ++i) {
            AddMemoryPoolToAllocator(allocator);
        }
        std::array<std::pair<void *, size_t>, ALLOCATIONS_COUNT> allocatedElements;

        // Allocations
        for (size_t size = MIN_ALLOC_SIZE; size <= MAX_ALLOC_SIZE; ++size) {
            void *mem = allocator.Alloc(size, Alignment(ALIGNMENT));
            ASSERT_TRUE(mem != nullptr) << "Didn't allocate " << size << " bytes with  "
                                        << static_cast<size_t>(ALIGNMENT) << " log alignment, seed: " << seed_;
            ASSERT_EQ(reinterpret_cast<uintptr_t>(mem) & (GetAlignmentInBytes(Alignment(ALIGNMENT)) - 1), 0UL)
                << size << " bytes, " << static_cast<size_t>(ALIGNMENT) << " log alignment, seed: " << seed_;
            allocatedElements[size - MIN_ALLOC_SIZE] = {mem, SetBytesFromByteArray(mem, size)};
        }
        // Check and Free
        for (size_t size = MIN_ALLOC_SIZE; size <= MAX_ALLOC_SIZE; size++) {
            size_t k = size - MIN_ALLOC_SIZE;
            ASSERT_TRUE(CompareBytesWithByteArray(allocatedElements[k].first, size, allocatedElements[k].second))
                << "address: " << std::hex << allocatedElements[k].first << ", size: " << size
                << ", alignment: " << static_cast<size_t>(ALIGNMENT) << ", seed: " << seed_;
            allocator.Free(allocatedElements[k].first);
        }
        delete memStats;
    }
    /**
     * @brief Allocate with all alignment
     * @tparam MIN_ALLOC_SIZE - minimum possible size for one allocation
     * @tparam MAX_ALLOC_SIZE - maximum possible size for one allocation
     * @tparam LOG_ALIGN_MIN_VALUE - minimum possible alignment for one allocation
     * @tparam LOG_ALIGN_MAX_VALUE - maximum possible alignment for one allocation
     * @param pools_count - count of pools needed by allocation
     *
     * Allocate all possible sizes from [MIN_ALLOC_SIZE, MAX_ALLOC_SIZE] with all possible alignment from
     * [LOG_ALIGN_MIN, LOG_ALIGN_MAX]
     */
    template <size_t MIN_ALLOC_SIZE, size_t MAX_ALLOC_SIZE, Alignment LOG_ALIGN_MIN_VALUE = LOG_ALIGN_MIN,
              Alignment LOG_ALIGN_MAX_VALUE = LOG_ALIGN_MAX>
    void AlignedAllocFreeTest(size_t poolsCount = 1)
    {
        static_assert(MIN_ALLOC_SIZE <= MAX_ALLOC_SIZE);
        static_assert(LOG_ALIGN_MIN_VALUE <= LOG_ALIGN_MAX_VALUE);
        static constexpr size_t ALLOCATIONS_COUNT =
            (MAX_ALLOC_SIZE - MIN_ALLOC_SIZE + 1) * (LOG_ALIGN_MAX_VALUE - LOG_ALIGN_MIN_VALUE + 1);

        std::array<std::pair<void *, size_t>, ALLOCATIONS_COUNT> allocatedElements;
        auto *memStats = new mem::MemStatsType();
        Allocator allocator(memStats);
        for (size_t i = 0; i < poolsCount; i++) {
            AddMemoryPoolToAllocator(allocator);
        }

        // Allocations with alignment
        size_t k = 0;
        for (size_t size = MIN_ALLOC_SIZE; size <= MAX_ALLOC_SIZE; ++size) {
            for (size_t align = LOG_ALIGN_MIN_VALUE; align <= LOG_ALIGN_MAX_VALUE; ++align, ++k) {
                void *mem = allocator.Alloc(size, Alignment(align));
                ASSERT_TRUE(mem != nullptr)
                    << "Didn't allocate " << size << " bytes with  " << align << " log alignment, seed: " << seed_;
                ASSERT_EQ(reinterpret_cast<uintptr_t>(mem) & (GetAlignmentInBytes(Alignment(align)) - 1), 0UL)
                    << size << " bytes, " << align << " log alignment, seed: " << seed_;
                allocatedElements[k] = {mem, SetBytesFromByteArray(mem, size)};
            }
        }
        // Check and free
        k = 0;
        for (size_t size = MIN_ALLOC_SIZE; size <= MAX_ALLOC_SIZE; ++size) {
            for (size_t align = LOG_ALIGN_MIN_VALUE; align <= LOG_ALIGN_MAX_VALUE; ++align, ++k) {
                ASSERT_TRUE(CompareBytesWithByteArray(allocatedElements[k].first, size, allocatedElements[k].second))
                    << "address: " << std::hex << allocatedElements[k].first << ", size: " << size
                    << ", alignment: " << align << ", seed: " << seed_;
                allocator.Free(allocatedElements[k].first);
            }
        }
        delete memStats;
    }

    /**
     * @brief Simple test for allocate and free
     * @param alloc_size - size in bytes for each allocation
     * @param elements_count - count of elements for allocation
     * @param pools_count - count of pools needed by allocation
     *
     * Allocate elements with random values setting, check and free memory
     */
    void AllocateAndFree(size_t allocSize, size_t elementsCount, size_t poolsCount = 1)
    {
        auto *memStats = new mem::MemStatsType();
        Allocator allocator(memStats);
        for (size_t i = 0; i < poolsCount; i++) {
            AddMemoryPoolToAllocator(allocator);
        }
        std::vector<std::pair<void *, size_t>> allocatedElements(elementsCount);

        // Allocations
        for (size_t i = 0; i < elementsCount; ++i) {
            void *mem = allocator.Alloc(allocSize);
            ASSERT_TRUE(mem != nullptr) << "Didn't allocate " << allocSize << " bytes in " << i
                                        << " iteration, seed: " << seed_;
            size_t index = SetBytesFromByteArray(mem, allocSize);
            allocatedElements[i] = {mem, index};
        }
        // Free
        for (auto &element : allocatedElements) {
            ASSERT_TRUE(CompareBytesWithByteArray(element.first, allocSize, element.second))
                << "address: " << std::hex << element.first << ", size: " << allocSize << ", seed: " << seed_;
            allocator.Free(element.first);
        }
        delete memStats;
    }

    /**
     * @brief Simple test for checking iteration over free pools method.
     * @tparam pools_count - count of pools needed by allocation, must be bigger than 3
     * @param alloc_size - size in bytes for each allocation
     *
     * Allocate and use memory pools; free all elements from first, last
     * and one in the middle; call iteration over free pools
     * and allocate smth again.
     */
    template <size_t POOLS_COUNT = 5>
    void VisitAndRemoveFreePools(size_t allocSize)
    {
        static constexpr size_t POOLS_TO_FREE = 3;
        static_assert(POOLS_COUNT > POOLS_TO_FREE);
        std::array<std::vector<void *>, POOLS_COUNT> allocatedElements;
        auto *memStats = new mem::MemStatsType();
        Allocator allocator(memStats);

        for (size_t i = 0; i < POOLS_COUNT; i++) {
            AddMemoryPoolToAllocator(allocator);
            while (true) {
                void *mem = allocator.Alloc(allocSize);
                if (mem == nullptr) {
                    break;
                }
                allocatedElements[i].push_back(mem);
            }
        }
        std::array<size_t, POOLS_TO_FREE> freedPoolsIndexes = {0, POOLS_COUNT / 2, POOLS_COUNT - 1};
        // free all elements in pools
        for (auto i : freedPoolsIndexes) {
            FreeAllocatedElements(allocatedElements, allocator, i);
        }
        size_t freedPools = 0;
        allocator.VisitAndRemoveFreePools([&freedPools](void *mem, size_t size) {
            (void)mem;
            (void)size;
            freedPools++;
        });
        ASSERT_TRUE(freedPools == POOLS_TO_FREE) << ", seed: " << seed_;
        ASSERT_TRUE(allocator.Alloc(allocSize) == nullptr) << ", seed: " << seed_;
        // allocate again
        for (auto i : freedPoolsIndexes) {
            AddMemoryPoolToAllocator(allocator);
            while (true) {
                void *mem = allocator.Alloc(allocSize);
                if (mem == nullptr) {
                    break;
                }
                allocatedElements[i].push_back(mem);
            }
        }
        // free everything:
        for (size_t i = 0; i < POOLS_COUNT; i++) {
            FreeAllocatedElements(allocatedElements, allocator, i);
        }
        freedPools = 0;
        allocator.VisitAndRemoveFreePools([&freedPools](void *mem, size_t size) {
            (void)mem;
            (void)size;
            freedPools++;
        });
        delete memStats;
        ASSERT_TRUE(freedPools == POOLS_COUNT) << ", seed: " << seed_;
    }

    template <size_t POOLS_COUNT = 5>
    static void FreeAllocatedElements(std::array<std::vector<void *>, POOLS_COUNT> &allocatedElements,
                                      Allocator &allocator, size_t i)
    {
        for (auto j : allocatedElements[i]) {
            allocator.Free(j);
        }
        allocatedElements[i].clear();
    }

    /**
     * @brief Allocate with different sizes and free in random order
     * @tparam MIN_ALLOC_SIZE - minimum possible size for one allocation
     * @tparam MAX_ALLOC_SIZE - maximum possible size for one allocation
     * @tparam AllocatorArgs - arguments types for allocor creation
     * @param elements_count - count of elements for allocation
     * @param pools_count - count of pools needed by allocation
     * @param allocator_args - arguments for allocator creation
     * Allocate elements with random size and random values setting in random order, check and free memory in random
     * order too
     */
    template <size_t MIN_ALLOC_SIZE, size_t MAX_ALLOC_SIZE, class... AllocatorArgs>
    void AllocateFreeDifferentSizesTest(size_t elementsCount, size_t poolsCount, AllocatorArgs &&...allocatorArgs)
    {
        std::unordered_set<size_t> usedIndexes;
        // {memory, size, start_index_in_byte_array}
        std::vector<std::tuple<void *, size_t, size_t>> allocatedElements(elementsCount);
        auto *memStats = new mem::MemStatsType();
        Allocator allocator(memStats, std::forward<AllocatorArgs>(allocatorArgs)...);
        for (size_t i = 0; i < poolsCount; i++) {
            AddMemoryPoolToAllocator(allocator);
        }

        size_t fullSizeAllocated = 0;
        for (size_t i = 0; i < elementsCount; ++i) {
            size_t size = RandFromRange(MIN_ALLOC_SIZE, MAX_ALLOC_SIZE);
            // Allocation
            void *mem = allocator.Alloc(size);
            ASSERT_TRUE(mem != nullptr) << "Didn't allocate " << size << " bytes, full allocated: " << fullSizeAllocated
                                        << ", seed: " << seed_;
            fullSizeAllocated += size;
            // Write random bytes
            allocatedElements[i] = {mem, size, SetBytesFromByteArray(mem, size)};
            usedIndexes.insert(i);
        }
        // Compare and free
        while (!usedIndexes.empty()) {
            size_t i = RandFromRange(0, elementsCount - 1);
            auto it = usedIndexes.find(i);
            if (it != usedIndexes.end()) {
                usedIndexes.erase(it);
            } else {
                i = *usedIndexes.begin();
                usedIndexes.erase(usedIndexes.begin());
            }
            // Compare
            ASSERT_TRUE(CompareBytesWithByteArray(std::get<0>(allocatedElements[i]), std::get<1>(allocatedElements[i]),
                                                  std::get<2U>(allocatedElements[i])))
                << "Address: " << std::hex << std::get<0>(allocatedElements[i])
                << ", size: " << std::get<1>(allocatedElements[i])
                << ", start index in byte array: " << std::get<2U>(allocatedElements[i]) << ", seed: " << seed_;
            allocator.Free(std::get<0>(allocatedElements[i]));
        }
        delete memStats;
    }

    /**
     * @brief Try to allocate too big object, must not allocate memory
     * @tparam MAX_ALLOC_SIZE - maximum possible size for allocation by this allocator
     */
    template <size_t MAX_ALLOC_SIZE>
    void AllocateTooBigObjectTest()
    {
        auto *memStats = new mem::MemStatsType();
        Allocator allocator(memStats);
        AddMemoryPoolToAllocator(allocator);

        size_t sizeObj = MAX_ALLOC_SIZE + 1;
        void *mem = allocator.Alloc(sizeObj);
        ASSERT_TRUE(mem == nullptr) << "Allocate too big object with " << sizeObj << " size at address " << std::hex
                                    << mem;
        delete memStats;
    }

    /**
     * @brief Try to allocate too many objects, must not allocate all objects
     * @param alloc_size - size in bytes for one allocation
     * @param elements_count - count of elements for allocation
     *
     * Allocate too many elements, so must not allocate all objects
     */
    void AllocateTooMuchTest(size_t allocSize, size_t elementsCount)
    {
        auto *memStats = new mem::MemStatsType();
        Allocator allocator(memStats);
        AddMemoryPoolToAllocatorProtected(allocator);

        bool isNotAll = false;
        for (size_t i = 0; i < elementsCount; i++) {
            void *mem = allocator.Alloc(allocSize);
            if (mem == nullptr) {
                isNotAll = true;
                break;
            }
            SetBytesFromByteArray(mem, allocSize);
        }
        ASSERT_TRUE(isNotAll) << "elements count: " << elementsCount << ", element size: " << allocSize
                              << ", seed: " << seed_;
        delete memStats;
    }

    /**
     * @brief Use allocator in std::vector
     * @param elements_count - count of elements for allocation
     *
     * Check working of adapter of this allocator on example std::vector
     */
    // NOLINTNEXTLINE(readability-magic-numbers)
    void AllocateVectorTest(size_t elementsCount = 32)
    {
        using ElementType = size_t;
        static constexpr size_t MAGIC_CONST = 3;
        auto *memStats = new mem::MemStatsType();
        Allocator allocator(memStats);
        AddMemoryPoolToAllocatorProtected(allocator);
        using AdapterType = typename decltype(allocator.Adapter())::template Rebind<ElementType>::other;
        std::vector<ElementType, AdapterType> vec(allocator.Adapter());

        for (size_t i = 0; i < elementsCount; i++) {
            vec.push_back(i * MAGIC_CONST);
        }
        for (size_t i = 0; i < elementsCount; i++) {
            ASSERT_EQ(vec[i], i * MAGIC_CONST) << "iteration: " << i;
        }

        vec.clear();

        for (size_t i = 0; i < elementsCount; i++) {
            vec.push_back(i * (MAGIC_CONST + 1));
        }
        for (size_t i = 0; i < elementsCount; i++) {
            ASSERT_EQ(vec[i], i * (MAGIC_CONST + 1)) << "iteration: " << i;
        }
        delete memStats;
    }

    /**
     * @brief Allocate and reuse
     * @tparam element_type - type of elements for allocations
     * @param alignment_mask - mask for alignment of two addresses
     * @param elements_count - count of elements for allocation
     *
     * Allocate and free memory and later reuse. Checking for two start addresses
     */
    template <class ElementType = uint64_t>
    void AllocateReuseTest(size_t alignmentMask, size_t elementsCount = 100)  // NOLINT(readability-magic-numbers)
    {
        static constexpr size_t SIZE_1 = sizeof(ElementType);
        static constexpr size_t SIZE_2 = SIZE_1 * 3;

        auto *memStats = new mem::MemStatsType();
        Allocator allocator(memStats);
        AddMemoryPoolToAllocator(allocator);
        std::vector<std::pair<void *, size_t>> allocatedElements(elementsCount);

        // First allocations
        for (size_t i = 0; i < elementsCount; ++i) {
            void *mem = allocator.Alloc(SIZE_1);
            ASSERT_TRUE(mem != nullptr) << "Didn't allocate " << SIZE_1 << " bytes in " << i << " iteration";
            size_t index = SetBytesFromByteArray(mem, SIZE_1);
            allocatedElements[i] = {mem, index};
        }
        auto firstAllocatedMem = reinterpret_cast<uintptr_t>(allocatedElements[0].first);
        // Free
        for (size_t i = 0; i < elementsCount; i++) {
            ASSERT_TRUE(CompareBytesWithByteArray(allocatedElements[i].first, SIZE_1, allocatedElements[i].second))
                << "address: " << std::hex << allocatedElements[i].first << ", size: " << SIZE_1 << ", seed: " << seed_;
            allocator.Free(allocatedElements[i].first);
        }
        // Second allocations
        for (size_t i = 0; i < elementsCount; ++i) {
            void *mem = allocator.Alloc(SIZE_2);
            ASSERT_TRUE(mem != nullptr) << "Didn't allocate " << SIZE_2 << " bytes in " << i << " iteration";
            size_t index = SetBytesFromByteArray(mem, SIZE_2);
            allocatedElements[i] = {mem, index};
        }
        auto secondAllocatedMem = reinterpret_cast<uintptr_t>(allocatedElements[0].first);
        // Free
        for (size_t i = 0; i < elementsCount; i++) {
            ASSERT_TRUE(CompareBytesWithByteArray(allocatedElements[i].first, SIZE_2, allocatedElements[i].second))
                << "address: " << std::hex << allocatedElements[i].first << ", size: " << SIZE_2 << ", seed: " << seed_;
            allocator.Free(allocatedElements[i].first);
        }
        delete memStats;
        ASSERT_EQ(firstAllocatedMem & ~alignmentMask, secondAllocatedMem & ~alignmentMask)
            << "first address = " << std::hex << firstAllocatedMem << ", second address = " << std::hex
            << secondAllocatedMem << std::endl
            << "alignment mask: " << alignmentMask << ", seed: " << seed_;
    }
    /**
     * @brief Allocate and free objects, collect via allocator method
     * @tparam MIN_ALLOC_SIZE - minimum possible size for one allocation
     * @tparam MAX_ALLOC_SIZE - maximum possible size for one allocation
     * @tparam LOG_ALIGN_MIN_VALUE - minimum possible alignment for one allocation
     * @tparam LOG_ALIGN_MAX_VALUE - maximum possible alignment for one allocation
     * @tparam ELEMENTS_COUNT_FOR_NOT_POOL_ALLOCATOR - 0 if allocator use pools, count of elements for allocation if
     * don't use pools
     * @param free_granularity - granularity for objects free before collection
     * @param pools_count - count of pools needed by allocation
     *
     * Allocate objects, free part of objects and collect via allocator method with free calls during the collection.
     * Check of collection.
     */
    template <size_t MIN_ALLOC_SIZE, size_t MAX_ALLOC_SIZE, Alignment LOG_ALIGN_MIN_VALUE = LOG_ALIGN_MIN,
              Alignment LOG_ALIGN_MAX_VALUE = LOG_ALIGN_MAX, size_t ELEMENTS_COUNT_FOR_NOT_POOL_ALLOCATOR = 0>
    void ObjectCollectionTest(size_t freeGranularity = 4, size_t poolsCount = 2)
    {
        size_t elementsCount = 0;
        std::vector<void *> allocatedElements;
        auto *memStats = new mem::MemStatsType();
        Allocator allocator(memStats);
        std::unordered_set<size_t> usedIndexes;
        ObjectIteratingSetUp<MIN_ALLOC_SIZE, MAX_ALLOC_SIZE, LOG_ALIGN_MIN_VALUE, LOG_ALIGN_MAX_VALUE,
                             ELEMENTS_COUNT_FOR_NOT_POOL_ALLOCATOR>(freeGranularity, poolsCount, allocator,
                                                                    elementsCount, allocatedElements, usedIndexes);

        // Collect all objects into unordered_set via allocator's method
        allocator.Collect(&AllocatorTest<Allocator>::ReturnDeadAndPutInSet);
        // Check in unordered_set
        for (size_t i = 0; i < elementsCount; i++) {
            auto it = usedIndexes.find(i);
            if (it != usedIndexes.end()) {
                void *mem = allocatedElements[i];
                ASSERT_TRUE(EraseFromSet(mem))
                    << "Object at address " << std::hex << mem << " isn't in collected objects, seed: " << seed_;
            }
        }

        delete memStats;
        ASSERT_TRUE(IsEmptySet()) << "seed: " << seed_;
    }

    /**
     * @brief Allocate and free objects, collect via allocator method
     * @tparam MIN_ALLOC_SIZE - minimum possible size for one allocation
     * @tparam MAX_ALLOC_SIZE - maximum possible size for one allocation
     * @tparam LOG_ALIGN_MIN_VALUE - minimum possible alignment for one allocation
     * @tparam LOG_ALIGN_MAX_VALUE - maximum possible alignment for one allocation
     * @tparam ELEMENTS_COUNT_FOR_NOT_POOL_ALLOCATOR - 0 if allocator use pools, count of elements for allocation if
     * don't use pools
     * @param free_granularity - granularity for objects free before collection
     * @param pools_count - count of pools needed by allocation
     *
     * Allocate objects, free part of objects and iterate via allocator method.
     * Check the iterated elements and free later.
     */
    template <size_t MIN_ALLOC_SIZE, size_t MAX_ALLOC_SIZE, Alignment LOG_ALIGN_MIN_VALUE = LOG_ALIGN_MIN,
              Alignment LOG_ALIGN_MAX_VALUE = LOG_ALIGN_MAX, size_t ELEMENTS_COUNT_FOR_NOT_POOL_ALLOCATOR = 0>
    void ObjectIteratorTest(size_t freeGranularity = 4, size_t poolsCount = 2)
    {
        size_t elementsCount = 0;
        std::vector<void *> allocatedElements;
        auto *memStats = new mem::MemStatsType();
        Allocator allocator(memStats);
        std::unordered_set<size_t> usedIndexes;
        ObjectIteratingSetUp<MIN_ALLOC_SIZE, MAX_ALLOC_SIZE, LOG_ALIGN_MIN_VALUE, LOG_ALIGN_MAX_VALUE,
                             ELEMENTS_COUNT_FOR_NOT_POOL_ALLOCATOR>(freeGranularity, poolsCount, allocator,
                                                                    elementsCount, allocatedElements, usedIndexes);

        // Collect all objects into unordered_set via allocator's method
        allocator.IterateOverObjects(&AllocatorTest<Allocator>::VisitAndPutInSet);
        // Free all and check in unordered_set
        for (size_t i = 0; i < elementsCount; i++) {
            auto it = usedIndexes.find(i);
            if (it != usedIndexes.end()) {
                void *mem = allocatedElements[i];
                allocator.Free(mem);
                ASSERT_TRUE(EraseFromSet(mem))
                    << "Object at address " << std::hex << mem << " isn't in collected objects, seed: " << seed_;
            }
        }

        delete memStats;
        ASSERT_TRUE(IsEmptySet()) << "seed: " << seed_;
    }

    /**
     * @brief Allocate and free objects, iterate via allocator method iterating in range
     * @tparam MIN_ALLOC_SIZE - minimum possible size for one allocation
     * @tparam MAX_ALLOC_SIZE - maximum possible size for one allocation
     * @tparam LOG_ALIGN_MIN_VALUE - minimum possible alignment for one allocation
     * @tparam LOG_ALIGN_MAX_VALUE - maximum possible alignment for one allocation
     * @tparam ELEMENTS_COUNT_FOR_NOT_POOL_ALLOCATOR - 0 if allocator use pools, count of elements for allocation if
     * don't use pools
     * @param range_iteration_size - size of a iteration range during test. Must be a power of two
     * @param free_granularity - granularity for objects free before collection
     * @param pools_count - count of pools needed by allocation
     *
     * Allocate objects, free part of objects and iterate via allocator method iterating in range. Check of iteration
     * and free later.
     */
    template <size_t MIN_ALLOC_SIZE, size_t MAX_ALLOC_SIZE, Alignment LOG_ALIGN_MIN_VALUE = LOG_ALIGN_MIN,
              Alignment LOG_ALIGN_MAX_VALUE = LOG_ALIGN_MAX, size_t ELEMENTS_COUNT_FOR_NOT_POOL_ALLOCATOR = 0>
    void ObjectIteratorInRangeTest(size_t rangeIterationSize, size_t freeGranularity = 4, size_t poolsCount = 2)
    {
        ASSERT((rangeIterationSize & (rangeIterationSize - 1U)) == 0U);
        size_t elementsCount = 0;
        std::vector<void *> allocatedElements;
        std::unordered_set<size_t> usedIndexes;
        auto *memStats = new mem::MemStatsType();
        Allocator allocator(memStats);
        ObjectIteratingSetUp<MIN_ALLOC_SIZE, MAX_ALLOC_SIZE, LOG_ALIGN_MIN_VALUE, LOG_ALIGN_MAX_VALUE,
                             ELEMENTS_COUNT_FOR_NOT_POOL_ALLOCATOR>(freeGranularity, poolsCount, allocator,
                                                                    elementsCount, allocatedElements, usedIndexes);

        void *minObjPointer = *std::min_element(allocatedElements.begin(), allocatedElements.end());
        void *maxObjPointer = *std::max_element(allocatedElements.begin(), allocatedElements.end());
        // Collect all objects into unordered_set via allocator's method
        uintptr_t curPointer = ToUintPtr(minObjPointer);
        curPointer = curPointer & (~(rangeIterationSize - 1));
        while (curPointer <= ToUintPtr(maxObjPointer)) {
            allocator.IterateOverObjectsInRange(&AllocatorTest<Allocator>::VisitAndPutInSet, ToVoidPtr(curPointer),
                                                ToVoidPtr(curPointer + rangeIterationSize - 1U));
            curPointer = curPointer + rangeIterationSize;
        }

        // Free all and check in unordered_set
        for (size_t i = 0; i < elementsCount; i++) {
            auto it = usedIndexes.find(i);
            if (it != usedIndexes.end()) {
                void *mem = allocatedElements[i];
                allocator.Free(mem);
                ASSERT_TRUE(EraseFromSet(mem))
                    << "Object at address " << std::hex << mem << " isn't in collected objects, seed: " << seed_;
            }
        }
        delete memStats;
        ASSERT_TRUE(IsEmptySet()) << "seed: " << seed_;
    }

    /**
     * @brief Address sanitizer test for allocator
     * @tparam elements_count - count of elements for allocation
     * @param free_granularity - granularity for freed elements
     * @param pools_count - count of pools needed by allocation
     *
     * Test for address sanitizer. Free some elements and try to write value in freed elements.
     */
    // NOLINTNEXTLINE(readability-magic-numbers)
    template <size_t ELEMENTS_COUNT = 100>
    void AsanTest(size_t freeGranularity = 3, size_t poolsCount = 1)  // NOLINT(readability-magic-numbers)
    {
        using ElementType = uint64_t;
        static constexpr size_t ALLOC_SIZE = sizeof(ElementType);
        static constexpr size_t ALLOCATIONS_COUNT = ELEMENTS_COUNT;

        if (freeGranularity == 0) {
            freeGranularity = 1;
        }

        auto *memStats = new mem::MemStatsType();
        Allocator allocator(memStats);
        for (size_t i = 0; i < poolsCount; i++) {
            AddMemoryPoolToAllocatorProtected(allocator);
        }
        std::array<void *, ALLOCATIONS_COUNT> allocatedElements {};
        // Allocations
        for (size_t i = 0; i < ALLOCATIONS_COUNT; ++i) {
            void *mem = allocator.Alloc(ALLOC_SIZE);
            ASSERT_TRUE(mem != nullptr) << "Didn't allocate " << ALLOC_SIZE << " bytes on " << i << " iteration";
            allocatedElements[i] = mem;
        }
        // Free some elements
        for (size_t i = 0; i < ALLOCATIONS_COUNT; i += freeGranularity) {
            allocator.Free(allocatedElements[i]);
        }
        // Asan check
        for (size_t i = 0; i < ALLOCATIONS_COUNT; ++i) {
            if (i % freeGranularity == 0) {
#ifdef PANDA_ASAN_ON
                EXPECT_DEATH(DeathWriteUint64(allocatedElements[i]), "")
                    << "Write " << sizeof(ElementType) << " bytes at address " << std::hex << allocatedElements[i];
#else
                continue;
#endif  // PANDA_ASAN_ON
            } else {
                allocator.Free(allocatedElements[i]);
            }
        }
        delete memStats;
    }
    /**
     * @brief Test to allocated by this allocator
     *
     * Test for allocator function which check memory on allocaion by this allocator
     */
    void AllocatedByThisAllocatorTest()
    {
        mem::MemStatsType memStats;
        Allocator allocator(&memStats);
        AllocatedByThisAllocatorTest(allocator);
    }

    /**
     * @brief Test to allocated by this allocator
     *
     * Test for allocator function which check memory on allocaion by this allocator
     */
    void AllocatedByThisAllocatorTest(Allocator &allocator)
    {
        static constexpr size_t ALLOC_SIZE = sizeof(uint64_t);
        AddMemoryPoolToAllocatorProtected(allocator);
        void *allocatedByThis = allocator.Alloc(ALLOC_SIZE);
        // NOLINTNEXTLINE(cppcoreguidelines-no-malloc)
        std::unique_ptr<void, void (*)(void *)> allocatedByMalloc(std::malloc(ALLOC_SIZE), free);
        std::array<uint8_t, ALLOC_SIZE> allocatedOnStack {};
        void *allocatedByMallocAddr = allocatedByMalloc.get();

        ASSERT_TRUE(AllocatedByThisAllocator(allocator, allocatedByThis)) << "address: " << std::hex << allocatedByThis;
        ASSERT_FALSE(AllocatedByThisAllocator(allocator, allocatedByMallocAddr))
            << "address: " << allocatedByMallocAddr;
        ASSERT_FALSE(AllocatedByThisAllocator(allocator, static_cast<void *>(allocatedOnStack.data())))
            << "address on stack: " << std::hex << static_cast<void *>(allocatedOnStack.data());

        allocator.Free(allocatedByThis);
        allocatedByMalloc.reset();

        // NOLINTNEXTLINE(clang-analyzer-unix.Malloc)
        ASSERT_FALSE(AllocatedByThisAllocator(allocator, allocatedByMallocAddr))
            << "after free, address: " << allocatedByMallocAddr;
    }

    /**
     * @brief Simultaneously allocate/free objects in different threads
     * @tparam allocator - target allocator for test
     * @tparam MIN_ALLOC_SIZE - minimum possible size for one allocation
     * @tparam MAX_ALLOC_SIZE - maximum possible size for one allocation
     * @tparam THREADS_COUNT - the number of threads used in this test
     * @param min_elements_count - minimum elements which will be allocated during test for each thread
     * @param max_elements_count - maximum elements which will be allocated during test for each thread
     */
    template <size_t MIN_ALLOC_SIZE, size_t MAX_ALLOC_SIZE, size_t THREADS_COUNT>
    void MtAllocTest(Allocator *allocator, size_t minElementsCount, size_t maxElementsCount)
    {
#if defined(PANDA_TARGET_ARM64) || defined(PANDA_TARGET_32)
        // We have an issue with QEMU during MT tests. Issue 2852
        static_assert(THREADS_COUNT == 1);
#endif
        std::atomic<size_t> numFinished = 0;
        for (size_t i = 0; i < THREADS_COUNT; i++) {
            auto tid = os::thread::ThreadStart(&MtAllocRun, this, allocator, &numFinished, MIN_ALLOC_SIZE,
                                               MAX_ALLOC_SIZE, minElementsCount, maxElementsCount);
            os::thread::ThreadDetach(tid);
        }

        while (true) {
            // Atomic with seq_cst order reason: data race with num_finished with requirement for sequentially
            // consistent order where threads observe all modifications in the same order
            if (numFinished.load(std::memory_order_seq_cst) == THREADS_COUNT) {
                break;
            }
            os::thread::Yield();
        }
    }

    /**
     * @brief Simultaneously allocate/free objects in different threads
     * @tparam MIN_ALLOC_SIZE - minimum possible size for one allocation
     * @tparam MAX_ALLOC_SIZE - maximum possible size for one allocation
     * @tparam THREADS_COUNT - the number of threads used in this test
     * @param min_elements_count - minimum elements which will be allocated during test for each thread
     * @param max_elements_count - maximum elements which will be allocated during test for each thread
     * @param free_granularity - granularity for objects free before total free
     */
    template <size_t MIN_ALLOC_SIZE, size_t MAX_ALLOC_SIZE, size_t THREADS_COUNT>
    void MtAllocFreeTest(size_t minElementsCount, size_t maxElementsCount, size_t freeGranularity = 4)
    {
#if defined(PANDA_TARGET_ARM64) || defined(PANDA_TARGET_32)
        // We have an issue with QEMU during MT tests. Issue 2852
        static_assert(THREADS_COUNT == 1);
#endif
        auto *memStats = new mem::MemStatsType();
        Allocator allocator(memStats);
        std::atomic<size_t> numFinished = 0;

        // Prepare an allocator
        MTTestPrologue(allocator, RandFromRange(MIN_ALLOC_SIZE, MAX_ALLOC_SIZE));

        for (size_t i = 0; i < THREADS_COUNT; i++) {
            (void)freeGranularity;
            auto tid = os::thread::ThreadStart(&MtAllocFreeRun, this, &allocator, &numFinished, freeGranularity,
                                               MIN_ALLOC_SIZE, MAX_ALLOC_SIZE, minElementsCount, maxElementsCount);
            os::thread::ThreadDetach(tid);
        }

        while (true) {
            // Atomic with seq_cst order reason: data race with num_finished with requirement for sequentially
            // consistent order where threads observe all modifications in the same order
            if (numFinished.load(std::memory_order_seq_cst) == THREADS_COUNT) {
                break;
            }
            os::thread::Yield();
        }
        delete memStats;
    }

    /**
     * @brief Simultaneously allocate objects and iterate over objects (in range too) in different threads
     * @tparam MIN_ALLOC_SIZE - minimum possible size for one allocation
     * @tparam MAX_ALLOC_SIZE - maximum possible size for one allocation
     * @tparam THREADS_COUNT - the number of threads used in this test
     * @param min_elements_count - minimum elements which will be allocated during test for each thread
     * @param max_elements_count - maximum elements which will be allocated during test for each thread
     * @param range_iteration_size - size of a iteration range during test. Must be a power of two
     */
    template <size_t MIN_ALLOC_SIZE, size_t MAX_ALLOC_SIZE, size_t THREADS_COUNT>
    void MtAllocIterateTest(size_t minElementsCount, size_t maxElementsCount, size_t rangeIterationSize)
    {
#if defined(PANDA_TARGET_ARM64) || defined(PANDA_TARGET_32)
        // We have an issue with QEMU during MT tests. Issue 2852
        static_assert(THREADS_COUNT == 1);
#endif
        auto *memStats = new mem::MemStatsType();
        Allocator allocator(memStats);
        std::atomic<size_t> numFinished = 0;

        // Prepare an allocator
        MTTestPrologue(allocator, RandFromRange(MIN_ALLOC_SIZE, MAX_ALLOC_SIZE));

        for (size_t i = 0; i < THREADS_COUNT; i++) {
            (void)rangeIterationSize;
            auto tid = os::thread::ThreadStart(&MtAllocIterateRun, this, &allocator, &numFinished, rangeIterationSize,
                                               MIN_ALLOC_SIZE, MAX_ALLOC_SIZE, minElementsCount, maxElementsCount);
            os::thread::ThreadDetach(tid);
        }

        while (true) {
            // Atomic with seq_cst order reason: data race with num_finished with requirement for sequentially
            // consistent order where threads observe all modifications in the same order
            if (numFinished.load(std::memory_order_seq_cst) == THREADS_COUNT) {
                break;
            }
            os::thread::Yield();
        }

        // Delete all objects in allocator
        allocator.Collect([&](ObjectHeader *object) {
            (void)object;
            return ObjectStatus::DEAD_OBJECT;
        });
        delete memStats;
    }

    /**
     * @brief Simultaneously allocate and collect objects in different threads
     * @tparam MIN_ALLOC_SIZE - minimum possible size for one allocation
     * @tparam MAX_ALLOC_SIZE - maximum possible size for one allocation
     * @tparam THREADS_COUNT - the number of threads used in this test
     * @param min_elements_count - minimum elements which will be allocated during test for each thread
     * @param max_elements_count - maximum elements which will be allocated during test for each thread
     * @param max_thread_with_collect - maximum threads which will call collect simultaneously
     */
    template <size_t MIN_ALLOC_SIZE, size_t MAX_ALLOC_SIZE, size_t THREADS_COUNT>
    void MtAllocCollectTest(size_t minElementsCount, size_t maxElementsCount, size_t maxThreadWithCollect = 1)
    {
#if defined(PANDA_TARGET_ARM64) || defined(PANDA_TARGET_32)
        // We have an issue with QEMU during MT tests. Issue 2852
        static_assert(THREADS_COUNT == 1);
#endif
        auto *memStats = new mem::MemStatsType();
        Allocator allocator(memStats);
        std::atomic<size_t> numFinished = 0;
        std::atomic<uint32_t> threadWithCollect {0U};

        // Prepare an allocator
        MTTestPrologue(allocator, RandFromRange(MIN_ALLOC_SIZE, MAX_ALLOC_SIZE));

        for (size_t i = 0; i < THREADS_COUNT; i++) {
            auto tid = os::thread::ThreadStart(&MtAllocCollectRun, this, &allocator, &numFinished, MIN_ALLOC_SIZE,
                                               MAX_ALLOC_SIZE, minElementsCount, maxElementsCount, maxThreadWithCollect,
                                               &threadWithCollect);
            os::thread::ThreadDetach(tid);
        }

        while (true) {
            // Atomic with seq_cst order reason: data race with num_finished with requirement for sequentially
            // consistent order where threads observe all modifications in the same order
            if (numFinished.load(std::memory_order_seq_cst) == THREADS_COUNT) {
                break;
            }
            os::thread::Yield();
        }

        // Delete all objects in allocator
        allocator.Collect([&](ObjectHeader *object) {
            (void)object;
            return ObjectStatus::DEAD_OBJECT;
        });
        delete memStats;
    }

private:
    /**
     * @brief Allocate and free objects in allocator for future collecting/iterating checks
     * @tparam MIN_ALLOC_SIZE - minimum possible size for one allocation
     * @tparam MAX_ALLOC_SIZE - maximum possible size for one allocation
     * @tparam LOG_ALIGN_MIN_VALUE - minimum possible alignment for one allocation
     * @tparam LOG_ALIGN_MAX_VALUE - maximum possible alignment for one allocation
     * @tparam ELEMENTS_COUNT_FOR_NOT_POOL_ALLOCATOR - 0 if allocator use pools, count of elements for allocation if
     * don't use pools
     * @param free_granularity - granularity for objects free before collection
     * @param pools_count - count of pools needed by allocation
     *
     * Allocate objects and free part of objects.
     */
    template <size_t MIN_ALLOC_SIZE, size_t MAX_ALLOC_SIZE, Alignment LOG_ALIGN_MIN_VALUE,
              Alignment LOG_ALIGN_MAX_VALUE, size_t ELEMENTS_COUNT_FOR_NOT_POOL_ALLOCATOR>
    void ObjectIteratingSetUp(size_t freeGranularity, size_t poolsCount, Allocator &allocator, size_t &elementsCount,
                              std::vector<void *> &allocatedElements, std::unordered_set<size_t> &usedIndexes)
    {
        AddMemoryPoolToAllocator(allocator);
        size_t allocatedPools = 1;
        auto doAllocations = [poolsCount]([[maybe_unused]] size_t allocatedPoolsCount,
                                          [[maybe_unused]] size_t count) -> bool {
            if constexpr (ELEMENTS_COUNT_FOR_NOT_POOL_ALLOCATOR == 0) {
                return allocatedPoolsCount < poolsCount;
            } else {
                (void)poolsCount;
                return count < ELEMENTS_COUNT_FOR_NOT_POOL_ALLOCATOR;
            }
        };

        // Allocations
        while (doAllocations(allocatedPools, elementsCount)) {
            size_t size = RandFromRange(MIN_ALLOC_SIZE, MAX_ALLOC_SIZE);
            size_t align = RandFromRange(LOG_ALIGN_MIN_VALUE, LOG_ALIGN_MAX_VALUE);
            void *mem = allocator.Alloc(size, Alignment(align));
            if constexpr (ELEMENTS_COUNT_FOR_NOT_POOL_ALLOCATOR == 0) {
                if (mem == nullptr) {
                    AddMemoryPoolToAllocator(allocator);
                    allocatedPools++;
                    mem = allocator.Alloc(size);
                }
            }
            ASSERT_TRUE(mem != nullptr) << "Didn't allocate " << size << " bytes in " << elementsCount
                                        << " iteration, seed : " << seed_;
            allocatedElements.push_back(mem);
            usedIndexes.insert(elementsCount++);
        }
        // Free some elements
        for (size_t i = 0; i < elementsCount; i += freeGranularity) {
            size_t index = RandFromRange(0, elementsCount - 1);
            auto it = usedIndexes.find(index);
            if (it == usedIndexes.end()) {
                it = usedIndexes.begin();
                index = *it;
            }
            allocator.Free(allocatedElements[index]);
            usedIndexes.erase(it);
        }
    }

    /**
     * @brief Prepare Allocator for the MT work. Allocate and free everything except one element
     * It will generate a common allocator state before specific tests.
     */
    void MTTestPrologue(Allocator &allocator, size_t allocSize);

    static void MtAllocRun(AllocatorTest<Allocator> *allocatorTestInstance, Allocator *allocator,
                           std::atomic<size_t> *numFinished, size_t minAllocSize, size_t maxAllocSize,
                           size_t minElementsCount, size_t maxElementsCount);

    static void MtAllocFreeRun(AllocatorTest<Allocator> *allocatorTestInstance, Allocator *allocator,
                               std::atomic<size_t> *numFinished, size_t freeGranularity, size_t minAllocSize,
                               size_t maxAllocSize, size_t minElementsCount, size_t maxElementsCount);

    static void MtAllocIterateRun(AllocatorTest<Allocator> *allocatorTestInstance, Allocator *allocator,
                                  std::atomic<size_t> *numFinished, size_t rangeIterationSize, size_t minAllocSize,
                                  size_t maxAllocSize, size_t minElementsCount, size_t maxElementsCount);

    static void MtAllocCollectRun(AllocatorTest<Allocator> *allocatorTestInstance, Allocator *allocator,
                                  std::atomic<size_t> *numFinished, size_t minAllocSize, size_t maxAllocSize,
                                  size_t minElementsCount, size_t maxElementsCount, uint32_t maxThreadWithCollect,
                                  std::atomic<uint32_t> *threadWithCollect);

    static std::unordered_set<void *> objectsSet_;

    static void VisitAndPutInSet(void *objMem)
    {
        objectsSet_.insert(objMem);
    }

    static ObjectStatus ReturnDeadAndPutInSet(ObjectHeader *objMem)
    {
        objectsSet_.insert(objMem);
        return ObjectStatus::DEAD_OBJECT;
    }

    static bool EraseFromSet(void *objMem)
    {
        auto it = objectsSet_.find(objMem);
        if (it != objectsSet_.end()) {
            objectsSet_.erase(it);
            return true;
        }
        return false;
    }

    static bool IsEmptySet() noexcept
    {
        return objectsSet_.empty();
    }

    static std::string BuildInfoForMtAllocTests(std::tuple<void *, size_t, size_t> allocatedElement, unsigned int seed)
    {
        std::stringstream stream;
        stream << "Address: " << std::hex << std::get<0>(allocatedElement)
               << ", size: " << std::get<1>(allocatedElement)
               << ", start index in byte array: " << std::get<2U>(allocatedElement) << ", seed: " << seed;
        return stream.str();
    }
};

// NOLINTBEGIN(fuchsia-statically-constructed-objects)
template <class Allocator>
std::unordered_set<void *> AllocatorTest<Allocator>::objectsSet_;
// NOLINTEND(fuchsia-statically-constructed-objects)

template <class Allocator>
void AllocatorTest<Allocator>::MtAllocRun(AllocatorTest<Allocator> *allocatorTestInstance, Allocator *allocator,
                                          std::atomic<size_t> *numFinished, size_t minAllocSize, size_t maxAllocSize,
                                          size_t minElementsCount, size_t maxElementsCount)
{
    size_t elementsCount = allocatorTestInstance->RandFromRange(minElementsCount, maxElementsCount);
    std::unordered_set<size_t> usedIndexes;
    // {memory, size, start_index_in_byte_array}
    std::vector<std::tuple<void *, size_t, size_t>> allocatedElements(elementsCount);

    for (size_t i = 0; i < elementsCount; ++i) {
        size_t size = allocatorTestInstance->RandFromRange(minAllocSize, maxAllocSize);
        // Allocation
        void *mem = allocator->Alloc(size);
        // Do while because other threads can use the whole pool before we try to allocate smth in it
        while (mem == nullptr) {
            allocatorTestInstance->AddMemoryPoolToAllocator(*allocator);
            mem = allocator->Alloc(size);
        }
        ASSERT_TRUE(mem != nullptr);
        // Write random bytes
        allocatedElements[i] = {mem, size, allocatorTestInstance->SetBytesFromByteArray(mem, size)};
        usedIndexes.insert(i);
    }

    // Compare
    while (!usedIndexes.empty()) {
        size_t i = allocatorTestInstance->RandFromRange(0, elementsCount - 1);
        auto it = usedIndexes.find(i);
        if (it != usedIndexes.end()) {
            usedIndexes.erase(it);
        } else {
            i = *usedIndexes.begin();
            usedIndexes.erase(usedIndexes.begin());
        }
        ASSERT_TRUE(allocatorTestInstance->AllocatedByThisAllocator(*allocator, std::get<0>(allocatedElements[i])));
        ASSERT_TRUE(allocatorTestInstance->CompareBytesWithByteArray(
            std::get<0>(allocatedElements[i]), std::get<1>(allocatedElements[i]), std::get<2U>(allocatedElements[i])))
            << BuildInfoForMtAllocTests(allocatedElements[i], allocatorTestInstance->seed_);
    }
    // Atomic with seq_cst order reason: data race with num_finished with requirement for sequentially consistent order
    // where threads observe all modifications in the same order
    numFinished->fetch_add(1, std::memory_order_seq_cst);
}

template <class Allocator>
void AllocatorTest<Allocator>::MtAllocFreeRun(AllocatorTest<Allocator> *allocatorTestInstance, Allocator *allocator,
                                              std::atomic<size_t> *numFinished, size_t freeGranularity,
                                              size_t minAllocSize, size_t maxAllocSize, size_t minElementsCount,
                                              size_t maxElementsCount)
{
    size_t elementsCount = allocatorTestInstance->RandFromRange(minElementsCount, maxElementsCount);
    std::unordered_set<size_t> usedIndexes;
    // {memory, size, start_index_in_byte_array}
    std::vector<std::tuple<void *, size_t, size_t>> allocatedElements(elementsCount);

    for (size_t i = 0; i < elementsCount; ++i) {
        size_t size = allocatorTestInstance->RandFromRange(minAllocSize, maxAllocSize);
        // Allocation
        void *mem = allocator->Alloc(size);
        // Do while because other threads can use the whole pool before we try to allocate smth in it
        while (mem == nullptr) {
            allocatorTestInstance->AddMemoryPoolToAllocator(*allocator);
            mem = allocator->Alloc(size);
        }
        ASSERT_TRUE(mem != nullptr);
        // Write random bytes
        allocatedElements[i] = {mem, size, allocatorTestInstance->SetBytesFromByteArray(mem, size)};
        usedIndexes.insert(i);
    }

    // Free some elements
    for (size_t i = 0; i < elementsCount; i += freeGranularity) {
        size_t index = allocatorTestInstance->RandFromRange(0, elementsCount - 1);
        auto it = usedIndexes.find(index);
        if (it != usedIndexes.end()) {
            usedIndexes.erase(it);
        } else {
            index = *usedIndexes.begin();
            usedIndexes.erase(usedIndexes.begin());
        }
        ASSERT_TRUE(allocatorTestInstance->AllocatedByThisAllocator(*allocator, std::get<0>(allocatedElements[index])));
        // Compare
        ASSERT_TRUE(allocatorTestInstance->CompareBytesWithByteArray(std::get<0>(allocatedElements[index]),
                                                                     std::get<1>(allocatedElements[index]),
                                                                     std::get<2U>(allocatedElements[index])))
            << BuildInfoForMtAllocTests(allocatedElements[i], allocatorTestInstance->seed_);
        allocator->Free(std::get<0>(allocatedElements[index]));
    }

    // Compare and free
    while (!usedIndexes.empty()) {
        size_t i = allocatorTestInstance->RandFromRange(0, elementsCount - 1);
        auto it = usedIndexes.find(i);
        if (it != usedIndexes.end()) {
            usedIndexes.erase(it);
        } else {
            i = *usedIndexes.begin();
            usedIndexes.erase(usedIndexes.begin());
        }
        // Compare
        ASSERT_TRUE(allocatorTestInstance->CompareBytesWithByteArray(
            std::get<0>(allocatedElements[i]), std::get<1>(allocatedElements[i]), std::get<2U>(allocatedElements[i])))
            << BuildInfoForMtAllocTests(allocatedElements[i], allocatorTestInstance->seed_);
        allocator->Free(std::get<0>(allocatedElements[i]));
    }
    // Atomic with seq_cst order reason: data race with num_finished with requirement for sequentially consistent order
    // where threads observe all modifications in the same order
    numFinished->fetch_add(1, std::memory_order_seq_cst);
}

template <class Allocator>
void AllocatorTest<Allocator>::MtAllocIterateRun(AllocatorTest<Allocator> *allocatorTestInstance, Allocator *allocator,
                                                 std::atomic<size_t> *numFinished, size_t rangeIterationSize,
                                                 size_t minAllocSize, size_t maxAllocSize, size_t minElementsCount,
                                                 size_t maxElementsCount)
{
    static constexpr size_t ITERATION_IN_RANGE_COUNT = 100;
    size_t elementsCount = allocatorTestInstance->RandFromRange(minElementsCount, maxElementsCount);
    // {memory, size, start_index_in_byte_array}
    std::vector<std::tuple<void *, size_t, size_t>> allocatedElements(elementsCount);

    // Iterate over all object
    allocator->IterateOverObjects([&](void *mem) { (void)mem; });

    // Allocate objects
    for (size_t i = 0; i < elementsCount; ++i) {
        size_t size = allocatorTestInstance->RandFromRange(minAllocSize, maxAllocSize);
        // Allocation
        void *mem = allocator->Alloc(size);
        // Do while because other threads can use the whole pool before we try to allocate smth in it
        while (mem == nullptr) {
            allocatorTestInstance->AddMemoryPoolToAllocator(*allocator);
            mem = allocator->Alloc(size);
        }
        ASSERT_TRUE(mem != nullptr);
        // Write random bytes
        allocatedElements[i] = {mem, size, allocatorTestInstance->SetBytesFromByteArray(mem, size)};
    }

    // Iterate over all object
    allocator->IterateOverObjects([&](void *mem) { (void)mem; });

    size_t iteratedOverObjects = 0;
    // Compare values inside the objects
    for (size_t i = 0; i < elementsCount; ++i) {
        // do a lot of iterate over range calls to check possible races
        if (iteratedOverObjects < ITERATION_IN_RANGE_COUNT) {
            void *leftBorder = ToVoidPtr(ToUintPtr(std::get<0>(allocatedElements[i])) & ~(rangeIterationSize - 1U));
            void *rightBorder = ToVoidPtr(ToUintPtr(leftBorder) + rangeIterationSize - 1U);
            allocator->IterateOverObjectsInRange([&](void *mem) { (void)mem; }, leftBorder, rightBorder);
            iteratedOverObjects++;
        }
        ASSERT_TRUE(allocatorTestInstance->AllocatedByThisAllocator(*allocator, std::get<0>(allocatedElements[i])));
        // Compare
        ASSERT_TRUE(allocatorTestInstance->CompareBytesWithByteArray(
            std::get<0>(allocatedElements[i]), std::get<1>(allocatedElements[i]), std::get<2U>(allocatedElements[i])))
            << BuildInfoForMtAllocTests(allocatedElements[i], allocatorTestInstance->seed_);
    }
    // Atomic with seq_cst order reason: data race with num_finished with requirement for sequentially consistent order
    // where threads observe all modifications in the same order
    numFinished->fetch_add(1, std::memory_order_seq_cst);
}

template <class Allocator>
void AllocatorTest<Allocator>::MtAllocCollectRun(AllocatorTest<Allocator> *allocatorTestInstance, Allocator *allocator,
                                                 std::atomic<size_t> *numFinished, size_t minAllocSize,
                                                 size_t maxAllocSize, size_t minElementsCount, size_t maxElementsCount,
                                                 uint32_t maxThreadWithCollect,
                                                 std::atomic<uint32_t> *threadWithCollect)
{
    size_t elementsCount = allocatorTestInstance->RandFromRange(minElementsCount, maxElementsCount);

    // Allocate objects
    for (size_t i = 0; i < elementsCount; ++i) {
        size_t size = allocatorTestInstance->RandFromRange(minAllocSize, maxAllocSize);
        // Allocation
        void *mem = allocator->Alloc(size);
        // Do while because other threads can use the whole pool before we try to allocate smth in it
        while (mem == nullptr) {
            allocatorTestInstance->AddMemoryPoolToAllocator(*allocator);
            mem = allocator->Alloc(size);
        }
        ASSERT_TRUE(mem != nullptr);
        auto object = static_cast<ObjectHeader *>(mem);
        object->SetMarkedForGC();
    }

    // Collect objects
    // Atomic with seq_cst order reason: data race with num_finished with requirement for sequentially consistent order
    // where threads observe all modifications in the same order
    if (threadWithCollect->fetch_add(1U, std::memory_order_seq_cst) < maxThreadWithCollect) {
        allocator->Collect([&](ObjectHeader *object) {
            ObjectStatus objectStatus =
                object->IsMarkedForGC() ? ObjectStatus::DEAD_OBJECT : ObjectStatus::ALIVE_OBJECT;
            return objectStatus;
        });
    }
    // Atomic with seq_cst order reason: data race with num_finished with requirement for sequentially consistent order
    // where threads observe all modifications in the same order
    numFinished->fetch_add(1, std::memory_order_seq_cst);
}

template <class Allocator>
void AllocatorTest<Allocator>::MTTestPrologue(Allocator &allocator, size_t allocSize)
{
    // Allocator preparing:
    std::vector<void *> allocatedElements;
    AddMemoryPoolToAllocator(allocator);
    // Allocate objects
    while (true) {
        // Allocation
        void *mem = allocator.Alloc(allocSize);
        if (mem == nullptr) {
            break;
        }
        allocatedElements.push_back(mem);
    }
    // Free everything except one element:
    for (size_t i = 1; i < allocatedElements.size(); ++i) {
        allocator.Free(allocatedElements[i]);
    }

    allocator.VisitAndRemoveFreePools([&](void *mem, size_t size) {
        (void)mem;
        (void)size;
    });
}

}  // namespace ark::mem

#endif  // PANDA_RUNTIME_TESTS_ALLOCATOR_TEST_BASE_H_
